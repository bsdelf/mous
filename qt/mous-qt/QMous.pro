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
    CustomHeadTabWidget.cpp \
    MidClickTabBar.cpp \
    DlgListSelect.cpp \
    DlgLoadingMedia.cpp \
    FrmProgressBar.cpp \
    DlgConvertTask.cpp \
    DlgConvertOption.cpp \
    FrmToolBar.cpp \
    FrmTagEditor.cpp \
    SimplePlaylistView.cpp

HEADERS  += MainWindow.h \
    CustomHeadTabWidget.hpp \
    MidClickTabBar.hpp \
    UiHelper.hpp \
    DlgListSelect.h \
    DlgLoadingMedia.h \
    FrmProgressBar.h \
    DlgConvertTask.h \
    DlgConvertOption.h \
    FrmToolBar.h \
    FrmTagEditor.h \
    IPlaylistView.h \
    SimplePlaylistView.h

FORMS    += MainWindow.ui \
    DlgListSelect.ui \
    DlgLoadingMedia.ui \
    FrmProgressBar.ui \
    DlgConvertTask.ui \
    DlgConvertOption.ui \
    FrmToolBar.ui \
    FrmTagEditor.ui

TRANSLATIONS = QMous.ts

RESOURCES += \
    AllRes.qrc
