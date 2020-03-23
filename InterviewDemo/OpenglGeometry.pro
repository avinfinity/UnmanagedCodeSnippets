#-------------------------------------------------
#
# Project created by QtCreator 2016-08-08T19:54:52
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OpenglGeometry
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    geometryengine.cpp \
    mainwidget.cpp

HEADERS  += mainwindow.h \
    geometryengine.h \
    mainwidget.h

FORMS    += mainwindow.ui

RESOURCES += \
    shaders.qrc \
    textures.qrc
