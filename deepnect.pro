TARGET=deepnect

QMAKE_CXXFLAGS += -std=c++14 -pthread

QT+=gui opengl core

SOURCES += deepnect.cpp \
            tinyexr.cpp

HEADERS += include/deepnect.h \
           include/tinyexr.h

INCLUDEPATH+= include \
                /usr/include/libusb-1.0

LIBS += -lfreenect \
        -lfreenect_sync \
        -lglut \
        -lGL \
        -lGLU \
        -pthread

OBJECTS_DIR=$$PWD/obj

CONFIG += console
