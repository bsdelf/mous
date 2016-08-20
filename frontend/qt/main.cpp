#include <QtGui/QApplication>
#include <QtCore/QTranslator>

#include "AppEnv.h"
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    auto env = GlobalAppEnv::Instance();
    env->Init();

    QApplication app(argc, argv);

    QTranslator translator;
    translator.load(env->translationFile);
    app.installTranslator(&translator);

    MainWindow win;
    win.show();

    int ret = app.exec();

    env->Save();

    return ret;
}
