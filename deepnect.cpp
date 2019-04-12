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
#include <future>

#include <tinyexr.h>

#include <QImage>
#include <QImageWriter>

#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif


Freenect::Freenect freenect;
MyFreenectDevice* device;


//c style declarations should not be happening
int window(0);                // Glut window identifier
int mx = -1, my = -1;         // Prevous mouse coordinates
float anglex = 0, angley = 0; // Panning angles
float zoom = 1;               // Zoom factor
bool color = true;            // Flag to indicate to use of color in the cloud

int frameNum = 0;              //framecounter for filename output
bool record = false;            //recordflag
int depthNum = 0;               //framecounter for depth output
bool recordDepth = false;       //recordflag for depth

bool recordBuffer = false;
Vec6 rgbdImage[640][480];        //lets start making some sense with a 2D array for our image data
//std::vector<std::vector<Vec6>>(rgbSeq);
std::vector<std::vector<uint8_t>> rgbSeq;
std::vector<std::vector<uint16_t>> depthSeq;

int width = 640;
int height = 480;

int frameCount = 0;


//function that waits for new frame
//once both rgb and depth are available, store them
//assign new thread to write each frame to file
//could probably do with a cache or buffer of sorts



//

//move this into draw loop - need to detect if buffer flag has been triggered, only then write, and then reset flag
void frameBuffer(std::vector<uint8_t> rgb, std::vector<uint16_t> depth, int frameNum)
{
    rgbSeq.push_back(rgb);
    depthSeq.push_back(depth);

    //output test values from buffer - seems to work
//    std::vector<uint8_t> test = rgbSeq[frameNum];
//    std::cout<< +test[0] << '\n';


}


void recorder(std::vector<uint8_t> rgb, std::vector<uint16_t> depth, int frameNum)
{

    ///SAVES EXR WITH RGB + ZDEPTH (Uses TinyEXR), there's not much point saving in Deep and having to implement full OpenEXR
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
    for (int i = 0; i < width * height; i++) {
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
    for (int i = 0; i < header.num_channels; i++) {
      header.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT; // pixel type of input image
      header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_HALF; // pixel type of output image to be stored in .EXR
    }

    //Save EXR plus prints
    const char* err;

    std::string text = "images/combine_";
    std::string extension = ".exr";
    text += std::to_string(frameNum);
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


    ///SAVES RGB BMP (Uses QImage)
//    QImage imageOut(640, 480, QImage::Format_RGB16);
//    QRgb value;

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

//    QString s = QString::number(frameNum);
//    QImageWriter writerQ("images/rgb"+s+".bmp", "bmp");
//    writerQ.write(imageOut);


    ///SAVES .PLY COLOUR POINT CLOUD (very hardware limited)
    //can we save to ofstream without writing file?
    //see https://stackoverflow.com/questions/35890488/store-data-in-ofstream-without-opening-a-file

//    std::ofstream myfile;

//    std::string text = "points/cloud_";
//    std::string extension = ".ply";
//    text += std::to_string(frameNum);
//    text += extension;

//    myfile.open (text);
//    myfile << "ply\n";
//    myfile << "format ascii 1.0\n";
//    myfile << "obj_info num_cols 640\n";
//    myfile << "obj_info num_rows 480\n";
//    myfile << "element vertex 307200\n";
//    myfile << "property float x\n";
//    myfile << "property float y\n";
//    myfile << "property float z\n";
//    myfile << "property uchar red\n";
//    myfile << "property uchar green\n";
//    myfile << "property uchar blue\n";
//    myfile << "end_header\n";

//    for (int i = 0; i < 480*640; ++i)
//    {

//        float f = 595.f;

//        if (depth[i] != 0) //ensures that points where no depth data is recorded are not saved to file
//        {
//            myfile << -((i%640 - (640-1)/2.f) * (depth[i]/1000.f) / f) //x
//                   << " "
//                   << -((i/640 - (480-1)/2.f) * (depth[i]/1000.f) / f) //y (x and y are minused in order to flip the output)
//                   << " "
//                   << depth[i]/1000.f //z
//                   << " "
//                   << +rgb[3*i+0] //r
//                   << " "
//                   << +rgb[3*i+1] //g
//                   << " "
//                   << +rgb[3*i+2] //b
//                   <<"\n";
//        }

//    }
//    myfile.close();

}



void delay1()
{
    auto Start = std::chrono::high_resolution_clock::now();
    while (1)
    {
        auto End = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> Elapsed = End - Start;
        if (Elapsed.count() >= 1000.0)
            break;
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

//    if (device->m_new_rgb_frame == true)
//    {
//        std::cout<< frameCount<< "new frame!\n";
//        frameCount++;
//    }
//    std::cout<< frameNum << "draw\n";
//    frameNum++;



    if (record == true)
    {
        //ensures we only record on a new frame, instead of every opengl draw
        if (device->m_new_rgb_frame == true)
        {
            //the previous depth frame will be pushed alongside the new RGB frame, is this okay?

            //push to our magic buffer first to prevent losing slowdown
            //only then push buffer to writer in for loop with async

            //multithreaded writer
            //std::async(recorder, rgb, depth, frameNum);
            frameBuffer(rgb, depth, frameNum);
            frameNum++;
        }


        //the receiving function should probably add to the cache of frames and then when record is not true, save it once with a savebuffer flag
        //leaving the actual file saving to be multithreaded

        //but we should try not to do that here, as it's not fps limited
    }

}




//WIP
void saveBuffer()
{

    //init qimage to put that data into
    QImage imageOut(640, 480, QImage::Format_RGB16);
    QRgb value;

    int scanlineOffset = 0;
    for (int y = 0; y < 480; ++y)
    {
        for (int x = 0; x < 640; ++x)
        {
            value = qRgb(rgbSeq[0][3*x+scanlineOffset],rgbSeq[0][3*x+1+scanlineOffset],rgbSeq[0][3*x+2+scanlineOffset]);
            imageOut.setPixel(x, y, value);
        }
        scanlineOffset+=640*3;
    }

    QImageWriter writerQ("outimage.bmp", "bmp");
    writerQ.write(imageOut);
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

    case 'B':
    case 'b':
        recordBuffer = !recordBuffer;
    break;

    case 'V':
    case 'v':
        saveBuffer();
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
