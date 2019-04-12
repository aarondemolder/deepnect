//Base gl viewer taken from the libefreenect cpp wrapper example
//Expanded with various saving functions for RGB and point cloud data sequences

#include <cstdlib>
#include <iostream>
#include <vector>
#include <fstream>
#include <future>

#include <libfreenect.hpp>
#include <deepnect.h>

#include <tinyexr.h>

#include <QImage>
#include <QImageWriter>

#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif



void fileWriter(int frameWrite)
{
    std::cout<<"Saving frame: " << frameWrite << " of "<< frameNum<<'\n';

    std::vector<uint8_t> rgb = rgbSeq[frameWrite];
    std::vector<uint16_t> depth = depthSeq[frameWrite];

    ///SAVES EXR WITH RGB + ZDEPTH (Uses TinyEXR), there's not much point saving in Deep with just depth per pixel and having to implement full OpenEXR
    if (exrToggle)
    {
        //init tinyEXR
        EXRHeader header;
        InitEXRHeader(&header);

        EXRImage image;
        InitEXRImage(&image);

        //set number of channels, and image sizes
        image.num_channels = 4;

        std::vector<float> images[4];
        images[0].resize(width * height);
        images[1].resize(width * height);
        images[2].resize(width * height);
        images[3].resize(width * height);

        //place rgb data from image loader into the EXR image array (also convert byte to float)
        for (int i = 0; i < width * height; i++)
        {
          images[0][i] = rgb[3*i+0]*(1.f/255.f);
          images[1][i] = rgb[3*i+1]*(1.f/255.f);
          images[2][i] = rgb[3*i+2]*(1.f/255.f);
          images[3][i] = depth[i]/1000.f; //we divide by 1000 to convert mm to m for the ZDepth
        }

        float* image_ptr[4];
        image_ptr[0] = &(images[2].at(0)); // B
        image_ptr[1] = &(images[1].at(0)); // G
        image_ptr[2] = &(images[0].at(0)); // R
        image_ptr[3] = &(images[3].at(0)); // Z

        //set EXR image data
        image.images = (unsigned char**)image_ptr;
        image.width = width;
        image.height = height;

        //set EXR header data
        header.num_channels = 4;
        header.channels = (EXRChannelInfo *)malloc(sizeof(EXRChannelInfo) * header.num_channels);
        // Must be BGR(A) order, since most of EXR viewers expect this channel order.
        strncpy(header.channels[0].name, "B", 255); header.channels[0].name[strlen("B")] = '\0';
        strncpy(header.channels[1].name, "G", 255); header.channels[1].name[strlen("G")] = '\0';
        strncpy(header.channels[2].name, "R", 255); header.channels[2].name[strlen("R")] = '\0';
        strncpy(header.channels[3].name, "Z", 255); header.channels[3].name[strlen("Z")] = '\0';

        header.pixel_types = (int *)malloc(sizeof(int) * header.num_channels);
        header.requested_pixel_types = (int *)malloc(sizeof(int) * header.num_channels);
        for (int i = 0; i < header.num_channels; i++)
        {
          header.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT; // pixel type of input image
          header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_HALF; // pixel type of output image to be stored in .EXR
        }

        //Save EXR plus prints
        const char* err;

        std::string text = "images/combine_";
        std::string extension = ".exr";
        text += std::to_string(frameWrite);
        text += extension;
        const char *cstr = text.c_str();

        int ret = SaveEXRImageToFile(&image, &header, cstr, &err);
        if (ret != TINYEXR_SUCCESS) {
          fprintf(stderr, "Save EXR err: %s\n", err);
        }
        //printf("Saved exr file. [ %d ] \n", frameNum);

        //free memory
        free(header.channels);
        free(header.pixel_types);
        free(header.requested_pixel_types);
    }



    ///SAVES RGB BMP (Uses QImage)
    /// Comment this if statement out if you wish to compile without Qt
    if (bmpToggle)
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

        QString s = QString::number(frameWrite);
        QImageWriter writerQ("images/rgb"+s+".bmp", "bmp");
        writerQ.write(imageOut);
    }


    ///SAVES .PLY COLOUR POINT CLOUD
    if (plyToggle)
    {
        std::ofstream myfile;

        std::string text2 = "points/cloud_";
        std::string extension2 = ".ply";
        text2 += std::to_string(frameWrite);
        text2 += extension2;

        myfile.open (text2);
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

        for (int i = 0; i < 480*640; ++i)
        {

            float f = 595.f;
            if (depth[i] != 0) //ensures that points where depth data is 0 are not saved to file
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
    }

}


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


    //start recording
    if (record)
    {
        // Draw the world coordinate but in red when recording
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        glColor3ub(255, 0, 0);  // X-axis
        glVertex3f(  0, 0, 0);
        glVertex3f( 50, 0, 0);
        glColor3ub(255, 0, 0);  // Y-axis
        glVertex3f(0,   0, 0);
        glVertex3f(0,  50, 0);
        glColor3ub(255, 0, 0);  // Z-axis
        glVertex3f(0, 0,   0);
        glVertex3f(0, 0,  50);
        glEnd();

        //ensures we only record on a new frame, instead of every opengl draw
        if (device->m_new_rgb_frame == true)
        {
            //the previous depth frame will be pushed alongside the new RGB frame, is this okay?
            //we should probably use the closest frame, but the example uses the previous - okay for things that don't move fast

            //push frames to vector
            rgbSeq.push_back(rgb);
            depthSeq.push_back(depth);

            frameNum++;
            bufferContent = true;
        }
    }

    //once recording is finished, and if bufferContent exists, write files
    if (!record && bufferContent)
    {
        std::cout<< "Saving Recording Buffer...\n";

        for (int i=0; i < frameNum; ++i)
        {
            //multithreaded file saving is faster, probably
            //fileWriter(i);
            std::async(fileWriter, i);
        }
        std::cout<< "Done!\n";

        //clear the recording buffer when we're done and reset the bufferContent flag
        bufferContent = !bufferContent;
        rgbSeq.clear();
        depthSeq.clear();

        frameNum =0;

        if (exrToggle) std::cout<< "Saved as EXR\n";
        if (bmpToggle) std::cout<< "Saved as BMP\n";
        if (plyToggle) std::cout<< "Saved as PLY\n";
    }

    if (!record)
    {
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
    }



    // Place the camera
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glScalef(zoom, zoom, 1);
    gluLookAt( -7*anglex, -7*angley, -1000.0,
                     0.0,       0.0,  2000.0,
                     0.0,      -1.0,     0.0 );

    glutSwapBuffers();

}




////saves single depth frame but as funky rgb (Uses QImage)
//void saveDepth()
//{
//    static std::vector<uint16_t> depth(640*480);
//    device->getDepth(depth);

//    QImage imageOut(640, 480, QImage::Format_RGB32);
//    QRgb value;

//    int scanlineOffset = 0;
//    for (int y = 0; y < 480; ++y)
//    {
//        for (int x = 0; x < 640; ++x)
//        {
//            value = qRgb(depth[x+scanlineOffset],depth[x+1+scanlineOffset],depth[x+2+scanlineOffset]);
//            imageOut.setPixel(x, y, value/100.f);
//        }
//        //std::cout<<"\n";
//        scanlineOffset+=640;
//    }

//    QImageWriter writerQ("outimage_depth.bmp", "bmp");
//    writerQ.write(imageOut);

//}

////save single RGB frame (Uses QImage)
//void saveColour()
//{
//    //create array for rgb channels for img and get true values from kinect
//    static std::vector<uint8_t> rgb(640*480*3);
//    device->getRGB(rgb);

//    //init qimage to put that data into
//    QImage imageOut(640, 480, QImage::Format_RGB16);
//    QRgb value;

//    //offset is for splitting the 1D pixel array into 640px widths
//    //then we put everything into the correct x,y position for the image writer
//    int scanlineOffset = 0;
//    for (int y = 0; y < 480; ++y)
//    {
//        for (int x = 0; x < 640; ++x)
//        {
//            value = qRgb(rgb[3*x+scanlineOffset],rgb[3*x+1+scanlineOffset],rgb[3*x+2+scanlineOffset]);
//            imageOut.setPixel(x, y, value);
//        }
//        scanlineOffset+=640*3;
//    }

//    QImageWriter writerQ("outimage.bmp", "bmp");
//    writerQ.write(imageOut);

//}

void recordToggle()
{
    ///this would cause issue with first recording, so whatever
//    if (!exrToggle && !bmpToggle && !plyToggle)
//    {
//        std::cout<< "No File Saving Specified. Cannot Record.\n";
//    }
//    else
//    {
        std::cout<< "Recording to buffer...\n";
        record = !record;
//    }
}

void exrToggleFunc()
{
    exrToggle = !exrToggle;

    if (exrToggle)
    {
        std::cout<< "EXR Saving Enabled\n";
    }
    else
    {
        std::cout<< "EXR Saving Disabled\n";
    }
}

void bmpToggleFunc()
{
    bmpToggle = !bmpToggle;

    if (bmpToggle)
    {
        std::cout<< "BMP Saving Enabled\n";
    }
    else
    {
        std::cout<< "BMP Saving Disabled\n";
    }
}

void plyToggleFunc()
{
    plyToggle = !plyToggle;

    if (plyToggle)
    {
        std::cout<< "PLY Saving Enabled\n";
    }
    else
    {
        std::cout<< "PLY Saving Disabled\n";
    }
}



void keyPressed(unsigned char key, int x, int y)
{
    switch (key)
    {
        case  'C':
        case  'c':
            color = !color;
        break;

        case 'R':
        case 'r':
            recordToggle();
        break;

        case 'E':
        case 'e':
            exrToggleFunc();
        break;

        case 'B':
        case 'b':
            bmpToggleFunc();
        break;

        case 'P':
        case 'p':
            plyToggleFunc();
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

//Update this
void printInfo()
{
    std::cout << "\nAvailable Controls:"              << std::endl;
    std::cout << "==================="                << std::endl;
    std::cout << "Rotate            :   Mouse Left Button" << std::endl;
    std::cout << "Zoom              :   Mouse Wheel"       << std::endl;
    std::cout << "Toggle Color      :   C"                 << std::endl;
    std::cout << "Record            :   R"                 << std::endl;
    std::cout << "Toggle EXR Saving :   E"                 << std::endl;
    std::cout << "Toggle BMP Saving :   B"                 << std::endl;
    std::cout << "Toggle PLY Saving :   P"                 << std::endl;
    std::cout << "Quit              :   Q or Esc\n"        << std::endl;
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
