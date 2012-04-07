#include <QtGui/QApplication>
#include <QTranslator>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator trans;
    trans.load("QMous");
    a.installTranslator(&trans);

    MainWindow w;
    w.show();

    return a.exec();
}
