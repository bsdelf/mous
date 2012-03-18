#-------------------------------------------------
#
# Project created by QtCreator 2012-02-20T22:12:47
#
#-------------------------------------------------

QT       += core gui

TARGET = qmous
TEMPLATE = app

INCLUDEPATH += ../../sdk
LIBS += -L./ -Wl,-rpath,./ -lMousCore

SOURCES += main.cpp\
        MainWindow.cpp \
        SimplePlayListView.cpp \
    CustomHeadTabWidget.cpp \
    MidClickTabBar.cpp

HEADERS  += MainWindow.h \
    SimplePlayListView.h \
    IPlayListView.h \
    CustomHeadTabWidget.hpp \
    MidClickTabBar.hpp \
    UiHelper.hpp

FORMS    += MainWindow.ui

TRANSLATIONS = QMous.ts

RESOURCES += \
    AllRes.qrc
