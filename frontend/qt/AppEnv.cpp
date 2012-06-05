#include "AppEnv.h"
#include <QFileInfo>
#include <QStringList>
#include <QLocale>
#include <QtDebug>

const QString LIB_PATH = "/lib/mous";
const QString RES_PATH = "/share/mous";

bool AppEnv::Init()
{
#ifdef CMAKE_INSTALL_PREFIX
    pluginDir = QString(CMAKE_INSTALL_PREFIX) + LIB_PATH;
    resourceDir = QString(CMAKE_INSTALL_PREFIX) + RES_PATH;
#else
    QStringList libPathes =
            (QStringList() << ("/usr/local" + LIB_PATH) << ("/usr" + LIB_PATH));
    foreach (QString path, libPathes) {
        if (QFileInfo(path).isDir()) {
            pluginDir = path;
            break;
        }
    }
    QStringList resPathes =
            (QStringList() << ("/usr/local" + RES_PATH) << ("/usr" + RES_PATH));
    foreach (QString path, resPathes) {
        if (QFileInfo(path).isDir()) {
            resourceDir = path;
            break;
        }
    }
#endif
    if (!pluginDir.isEmpty() && !resourceDir.isEmpty()) {
        ChooseTranslation();
        return true;
    } else {
        return false;
    }
}

void AppEnv::ChooseTranslation()
{
    QString locale = QLocale::system().name();
    translationFile = resourceDir + "/qt/mous-qt_" + locale;

    qDebug() << "locale:" << locale;
    qDebug() << "translationFile:" << translationFile;
}
