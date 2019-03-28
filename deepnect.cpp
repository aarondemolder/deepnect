//Base gl viewer taken from the libefreenect cpp wrapper example

#include <cstdlib>
#include <iostream>

#include <fstream>
#include <QImage>
#include <QImageWriter>

#include <vector>
#include <libfreenect.hpp>

#include <class_container.h>

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
}

void saveDepth()
{
    static std::vector<uint16_t> depth(640*480);
    device->getDepth(depth);

    QImage imageOut(640, 480, QImage::Format_RGB32);
    QRgb value;

//    for (int i = 0; i < 480*640; ++i)
//    {
//        value = qRgb(depth[i], 0, 0);
//        imageOut.setPixel(x, y, value);
//    }
//    myfile.close();

//    for(int x=0; x < 640; x++)
//        {
//            value = qRgb(depth[x], depth[x], depth[x]);
//            imageOut.setPixel(x, 0, value);
//            for(int y=0; y < 480; y++)
//            {
//                    value = qRgb(depth[y], depth[y], depth[y]);
//                    imageOut.setPixel(x, y, value);
//            }
//        }

    for (int x=0; x<640; ++x)
    {
        //value = qRgb(depth[i], 0, 0);
        //        imageOut.setPixel(x, y, value);
    }

    QImageWriter writerQ("outimage.bmp", "bmp");
    writerQ.write(imageOut);

}


void saveColour()
{
    static std::vector<uint8_t> rgb(640*480*3);
    device->getRGB(rgb);

    QImage imageOut(640, 480, QImage::Format_RGB16);
    QRgb value;

    //012345678
    //rgbrgbrgbr

    int scanlineOffset = 0;
    for (int y = 0; y < 480; ++y)
    {
        for (int x = 0; x < 640; ++x)
        {
            value = qRgb(rgb[3*x+0+scanlineOffset],rgb[3*x+1+scanlineOffset],rgb[3*x+2+scanlineOffset]);
            imageOut.setPixel(x, y, value);
        }
        scanlineOffset+=640;
    }


//    for(int x=0; x < 1; x++)
//        {
//            for(int y=0; y < 3; y++)
//            {
//                    value = qRgb(rgb[]);
//                    imageOut.setPixel(x, y, value);
//            }
//        }

    //static_cast<int>(rgb[3*(i)+0])


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
