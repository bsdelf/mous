#-------------------------------------------------
#
# Project created by QtCreator 2012-02-20T22:12:47
#
#-------------------------------------------------

QT += widgets
CONFIG += c++14

TARGET = mous-qt
TEMPLATE = app

INCLUDEPATH += ../../sdk
LIBS += -liconv -L /Users/bsdelf/myapp/lib/ -lMousCore

macx {
    #QMAKE_CXXFLAGS_RELEASE += -fvisibility=hidden
    #QMAKE_CXXFLAGS_DEBUG += -fvisibility=hidden
    #LIBS += -framework
    CONFIG += x86_64
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.12
}


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
    SimplePlaylistView.cpp \
    AppEnv.cpp

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
    SimplePlaylistView.h \
    PlaylistActionHistory.h \
    PlaylistClipboard.h \
    FoobarStyle.h \
    AppEnv.h

FORMS    += MainWindow.ui \
    DlgListSelect.ui \
    DlgLoadingMedia.ui \
    FrmProgressBar.ui \
    DlgConvertTask.ui \
    DlgConvertOption.ui \
    FrmToolBar.ui \
    FrmTagEditor.ui

TRANSLATIONS = mous-qt_zh_CN.ts

RESOURCES += \
    AllRes.qrc
