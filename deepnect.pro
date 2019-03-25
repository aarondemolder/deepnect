TARGET=deepnect
TEMPLATE = app
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -std=c++11 -pthread


SOURCES += \
        deepnect.cpp


INCLUDEPATH+= include \
                /usr/include/libusb-1.0

LIBS += -lfreenect \
        -lfreenect_sync \
        -lglut \
        -lGL \
        -lGLU \
        -pthread

OBJECTS_DIR=$$PWD/obj

HEADERS += \
    class_container.h
