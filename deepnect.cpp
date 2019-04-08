//Base gl viewer taken from the libefreenect cpp wrapper example
//Expanded with various saving functions for RGB and point cloud data sequences
//

#include <cstdlib>
#include <iostream>
#include <vector>
#include <fstream>
#include <libfreenect.hpp>

#include <class_container.h>
#include <vec6.h>

#include <QImage>
#include <QImageWriter>

#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif


Freenect::Freenect freenect;
MyFreenectDevice* device;

int window(0);                // Glut window identifier
int mx = -1, my = -1;         // Prevous mouse coordinates
float anglex = 0, angley = 0; // Panning angles
float zoom = 1;               // Zoom factor
bool color = true;            // Flag to indicate to use of color in the cloud
int frameNum = 0;              //framecounter for filename output
bool record = false;            //recordflag
int depthNum = 0;               //framecounter for depth output
bool recordDepth = false;       //recordflag for depth



void DrawGLScene()
{
    static std::vector<uint8_t> rgb(640*480*3);
    static std::vector<uint16_t> depth(640*480);

    device->getRGB(rgb);
    device->getDepth(depth);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPointSize(1.0f);

    glBegin(GL_POINTS);

    if (!color) glColor3ub(255, 255, 255);
    for (int i = 0; i < 480*640; ++i)
    {
        if (color)
            glColor3ub( rgb[3*i+0],    // R
                        rgb[3*i+1],    // G
                        rgb[3*i+2] );  // B

        float f = 595.f;
        // Convert from image plane coordinates to world coordinates
        glVertex3f( (i%640 - (640-1)/2.f) * depth[i] / f,  // X = (x - cx) * d / fx
                    (i/640 - (480-1)/2.f) * depth[i] / f,  // Y = (y - cy) * d / fy
                    depth[i] );                            // Z = d
    }

    glEnd();

    // Draw the world coordinate frame
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glColor3ub(255, 0, 0);  // X-axis
    glVertex3f(  0, 0, 0);
    glVertex3f( 50, 0, 0);
    glColor3ub(0, 255, 0);  // Y-axis
    glVertex3f(0,   0, 0);
    glVertex3f(0,  50, 0);
    glColor3ub(0, 0, 255);  // Z-axis
    glVertex3f(0, 0,   0);
    glVertex3f(0, 0,  50);
    glEnd();

    // Place the camera
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glScalef(zoom, zoom, 1);
    gluLookAt( -7*anglex, -7*angley, -1000.0,
                     0.0,       0.0,  2000.0,
                     0.0,      -1.0,     0.0 );

    glutSwapBuffers();

    //end opengl

    //below image saving - should probably limit fps to 30

    //if toggled, records rgb stream (but perhaps at opengl refresh rate?) into images folder
    if (record == true)
    {
        QImage imageOut(640, 480, QImage::Format_RGB16);
        QRgb value;

        int scanlineOffset = 0;
        for (int y = 0; y < 480; ++y)
        {
            for (int x = 0; x < 640; ++x)
            {
                value = qRgb(rgb[3*x+scanlineOffset],rgb[3*x+1+scanlineOffset],rgb[3*x+2+scanlineOffset]);
                imageOut.setPixel(x, y, value);
            }
            scanlineOffset+=640*3;
        }

        QString s = QString::number(frameNum);
        QImageWriter writerQ("images/rgb"+s+".bmp", "bmp");
        writerQ.write(imageOut);
        frameNum++;
    }

    //if toggled, records colour point clouds to folder
    if (recordDepth  == true)
    {
        ///saves depth as images instead of points
//        QImage imageOut(640, 480, QImage::Format_RGB16);
//        QRgb value;


//        int scanlineOffset = 0;
//        for (int y = 0; y < 480; ++y)
//        {
//            for (int x = 0; x < 640; ++x)
//            {
//                value = qRgb(depth[x+scanlineOffset],depth[x+1+scanlineOffset],depth[x+2+scanlineOffset]);
//                imageOut.setPixel(x, y, value/100.f);
//            }
//            scanlineOffset+=640;
//        }

//        QString s = QString::number(depthNum);
//        QImageWriter writerQ("images/depth"+s+".bmp", "bmp");
//        writerQ.write(imageOut);
//        depthNum++;


        ///this bit saves it as point clouds, we're going for .ply beacuse ascii
        /// pros - it works
        /// cons - it's slow, we could be write speed limited!

        std::ofstream myfile;

        std::string text = "cloud_";
        std::string extension = ".ply";
        text += std::to_string(frameNum);
        text += extension;

        myfile.open (text);
        myfile << "ply\n";
        myfile << "format ascii 1.0\n";
        myfile << "obj_info num_cols 640\n";
        myfile << "obj_info num_rows 480\n";
        myfile << "element vertex 307200\n";
        myfile << "property float x\n";
        myfile << "property float y\n";
        myfile << "property float z\n";
        myfile << "property uchar red\n";
        myfile << "property uchar green\n";
        myfile << "property uchar blue\n";
        myfile << "end_header\n";

        QImage imageOut(640, 480, QImage::Format_RGB16);
        QRgb value;

        int scanlineOffset = 0;
        for (int y = 0; y < 480; ++y)
        {
            for (int x = 0; x < 640; ++x)
            {
                value = qRgb(rgb[3*x+scanlineOffset],rgb[3*x+1+scanlineOffset],rgb[3*x+2+scanlineOffset]);
                imageOut.setPixel(x, y, value);
            }
            scanlineOffset+=640*3;
        }

        QString s = QString::number(frameNum);
        QImageWriter writerQ("images/rgb"+s+".bmp", "bmp");
        writerQ.write(imageOut);

        for (int i = 0; i < 480*640; ++i)
        {

            float f = 595.f;

            if (depth[i] != 0) //ensures that points where no depth data is recorded are not saved to file
            {
                myfile << -((i%640 - (640-1)/2.f) * (depth[i]/1000.f) / f) //x
                       << " "
                       << -((i/640 - (480-1)/2.f) * (depth[i]/1000.f) / f) //y (x and y are minused in order to flip the output)
                       << " "
                       << depth[i]/1000.f //z
                       << " "
                       << +rgb[3*i+0] //r
                       << " "
                       << +rgb[3*i+1] //g
                       << " "
                       << +rgb[3*i+2] //b
                       <<"\n";
            }

        }
        myfile.close();
        frameNum++;
    }


    //add if buffer record true:
    //use vec6 to dump data at 30 fps (requires timer) into lovely lil vec array
    //once complete require button press to run saveBuffer()


}


void saveBuffer()
{
    //pass rgbd data here from array
    //save as rgb and point cloud sequences
}


//saves single depth frame but as funky rgb
void saveDepth()
{
    static std::vector<uint16_t> depth(640*480);
    device->getDepth(depth);

    QImage imageOut(640, 480, QImage::Format_RGB32);
    QRgb value;

    int scanlineOffset = 0;
    for (int y = 0; y < 480; ++y)
    {
        for (int x = 0; x < 640; ++x)
        {
            value = qRgb(depth[x+scanlineOffset],depth[x+1+scanlineOffset],depth[x+2+scanlineOffset]);
            imageOut.setPixel(x, y, value/100.f);
        }
        //std::cout<<"\n";
        scanlineOffset+=640;
    }

    QImageWriter writerQ("outimage_depth.bmp", "bmp");
    writerQ.write(imageOut);

}

//save single RGB frame
void saveColour()
{
    //create array for rgb channels for img and get true values from kinect
    static std::vector<uint8_t> rgb(640*480*3);
    device->getRGB(rgb);

    //init qimage to put that data into
    QImage imageOut(640, 480, QImage::Format_RGB16);
    QRgb value;

    //offset is for splitting the 1D pixel array into 640px widths
    //then we put everything into the correct x,y position for the image writer
    int scanlineOffset = 0;
    for (int y = 0; y < 480; ++y)
    {
        for (int x = 0; x < 640; ++x)
        {
            value = qRgb(rgb[3*x+scanlineOffset],rgb[3*x+1+scanlineOffset],rgb[3*x+2+scanlineOffset]);
            imageOut.setPixel(x, y, value);
        }
        scanlineOffset+=640*3;
    }

    QImageWriter writerQ("outimage.bmp", "bmp");
    writerQ.write(imageOut);

}

void keyPressed(unsigned char key, int x, int y)
{
    switch (key)
    {
        case  'C':
        case  'c':
            color = !color;
        break;

    case 'S':
    case 's':
        saveColour();
    break;

    case 'D':
    case 'd':
        saveDepth();
    break;

    case 'R':
    case 'r':
        //toggle bool with flip
        record = !record;
    break;

    case 'P':
    case 'p':
        recordDepth = !recordDepth;
    break;

        case  'Q':
        case  'q':
        case 0x1B:  // ESC
            glutDestroyWindow(window);
            device->stopDepth();
            device->stopVideo();
            exit(0);


    }
}


void mouseMoved(int x, int y)
{
    if (mx >= 0 && my >= 0)
    {
        anglex += x - mx;
        angley += y - my;
    }

    mx = x;
    my = y;
}


void mouseButtonPressed(int button, int state, int x, int y)
{
    if (state == GLUT_DOWN)
    {
        switch (button)
        {
            case GLUT_LEFT_BUTTON:
                mx = x;
                my = y;
                break;

            case 3:
                zoom *= 1.2f;
                break;

            case 4:
                zoom /= 1.2f;
                break;
        }
    }
    else if (state == GLUT_UP && button == GLUT_LEFT_BUTTON)
    {
        mx = -1;
        my = -1;
    }
}


void resizeGLScene(int width, int height)
{
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(50.0, (float)width / height, 900.0, 11000.0);

    glMatrixMode(GL_MODELVIEW);
}


void idleGLScene()
{
    glutPostRedisplay();
}


void printInfo()
{
    std::cout << "\nAvailable Controls:"              << std::endl;
    std::cout << "==================="                << std::endl;
    std::cout << "Rotate       :   Mouse Left Button" << std::endl;
    std::cout << "Zoom         :   Mouse Wheel"       << std::endl;
    std::cout << "Toggle Color :   C"                 << std::endl;
    std::cout << "Quit         :   Q or Esc\n"        << std::endl;
}


int main(int argc, char **argv)
{
    device = &freenect.createDevice<MyFreenectDevice>(0);
    device->startVideo();
    device->startDepth();

    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutInitWindowPosition(0, 0);

    window = glutCreateWindow("deepnect");
    glClearColor(0.45f, 0.45f, 0.45f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.0f);

    glMatrixMode(GL_PROJECTION);
    gluPerspective(50.0, 1.0, 900.0, 11000.0);

    glutDisplayFunc(&DrawGLScene);
    glutIdleFunc(&idleGLScene);
    glutReshapeFunc(&resizeGLScene);
    glutKeyboardFunc(&keyPressed);
    glutMotionFunc(&mouseMoved);
    glutMouseFunc(&mouseButtonPressed);

    printInfo();

    glutMainLoop();

    return 0;
}
