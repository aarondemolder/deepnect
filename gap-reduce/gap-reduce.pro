TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt


SOURCES += \
        main.cpp \
        ../tinyexr.cpp

HEADERS += ../include/tinyexr.h

INCLUDEPATH+= include
