#-------------------------------------------------
#
# Project created by QtCreator 2012-02-20T22:12:47
#
#-------------------------------------------------

QT       += core gui

TARGET = qmous
TEMPLATE = app

INCLUDEPATH += ../../sdk ../../core
LIBS += -L./ -Wl,-rpath,./ -lMousCore

SOURCES += main.cpp\
        MainWindow.cpp \
    sqt/CustomHeadTabWidget.cpp \
    sqt/MidClickTabBar.cpp \
    SimplePlayListView.cpp

HEADERS  += MainWindow.h \
    sqt/CustomHeadTabWidget.h \
    sqt/MidClickTabBar.h \
    SimplePlayListView.h \
    sqt/UiHelper.hpp

FORMS    += MainWindow.ui

RESOURCES += \
    AllRes.qrc
