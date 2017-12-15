#include <QApplication>
#include <QTranslator>

#include "AppEnv.h"
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    auto env = GlobalAppEnv::Instance();
    auto flag = env->Init();
    if (!flag) {
        qDebug() << "bad";
    }


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
