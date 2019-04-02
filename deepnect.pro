TARGET=deepnect
#TEMPLATE = app
#CONFIG -= app_bundle
#CONFIG -= qt

#QT+=gui opengl core

QMAKE_CXXFLAGS += -std=c++11 -pthread

#CONFIG+=c++11

# where to put the .o files
OBJECTS_DIR=obj

# core Qt Libs to use add more here if needed.
QT+=gui opengl core

# as I want to support 4.8 and 5 this will set a flag for some of the mac stuff
# mainly in the types.h file for the setMacVisual which is native in Qt5
isEqual(QT_MAJOR_VERSION, 5) {
        cache()
        DEFINES +=QT5BUILD
}

SOURCES += \
        deepnect.cpp

HEADERS += include/class_container.h \
    include/vec3.h


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

CONFIG += console
