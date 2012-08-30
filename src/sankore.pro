#-------------------------------------------------
#
# Project created by QtCreator 2012-07-24T18:08:17
#
#-------------------------------------------------

QT       += core gui

TARGET = sankore
TEMPLATE = app


SOURCES += main.cpp\
        cvaux.cpp\
        cvblob.cpp\
        cvcolor.cpp\
        cvcontour.cpp\
        cvlabel.cpp\
        cvtrack.cpp
        
HEADERS  += cvblob.h

LIBS += -lopencv_highgui -lopencv_core
