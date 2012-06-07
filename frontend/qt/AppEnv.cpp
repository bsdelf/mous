#include "AppEnv.h"
#include <QtCore>

namespace Path {
    const QString Config = "/.config/mous";
    const QString Plugin = "/lib/mous";
    const QString Resource = "/share/mous";
}

namespace Key {
    const QString IfNotUtf8 = "IfNotUtf8";
    const QString TagEditorSplitterState = "TagEditorSplitterState";
    const QString WindowGeometry = "WindowGeometry";
    const QString WindowState = "WindowState";
}

bool AppEnv::Init()
{
#ifdef CMAKE_INSTALL_PREFIX
    pluginDir = QString(CMAKE_INSTALL_PREFIX) + Path::Plugin;
    resourceDir = QString(CMAKE_INSTALL_PREFIX) + Path::Resource;
#else
    QStringList libPathes =
            (QStringList() << ("/usr/local" + Path::Plugin) << ("/usr" + Path::Plugin));
    foreach (QString path, libPathes) {
        if (QFileInfo(path).isDir()) {
            pluginDir = path;
            break;
        }
    }
    QStringList resPathes =
            (QStringList() << ("/usr/local" + Path::Resource) << ("/usr" + Path::Resource));
    foreach (QString path, resPathes) {
        if (QFileInfo(path).isDir()) {
            resourceDir = path;
            break;
        }
    }
#endif
    if (!pluginDir.isEmpty() && !resourceDir.isEmpty()) {
        InitFilePath();
        return LoadConfig();
    } else {
        return false;
    }
}

void AppEnv::Save()
{
    QSettings settings(configFile, QSettings::IniFormat);
    settings.setValue(Key::IfNotUtf8, ifNotUtf8);    
    settings.setValue(Key::WindowGeometry, windowGeometry);
    settings.setValue(Key::WindowState, windowState);
    settings.setValue(Key::TagEditorSplitterState, tagEditorSplitterState);
    settings.sync();
}

void AppEnv::InitFilePath()
{
    configFile = QDir::homePath() + Path::Config + "/qt/config";
    qDebug() << "configFile" << configFile;

    QString locale = QLocale::system().name();
    translationFile = resourceDir + "/qt/mous-qt_" + locale;
    qDebug() << "locale:" << locale;
    qDebug() << "translationFile:" << translationFile;
}

bool AppEnv::LoadConfig()
{
    if (!CheckDefaultConfig())
        return false;

    QSettings settings(configFile, QSettings::IniFormat);
    ifNotUtf8 = settings.value(Key::IfNotUtf8).toString();    
    windowGeometry = settings.value(Key::WindowGeometry).toByteArray();
    windowState = settings.value(Key::WindowState).toByteArray();
    tagEditorSplitterState = settings.value(Key::TagEditorSplitterState).toByteArray();
    return true;
}

bool AppEnv::CheckDefaultConfig()
{
    QString configRoot = QDir::homePath() + Path::Config + "/qt";
    QFileInfo configRootInfo(configRoot);
    if (configRootInfo.exists()) {
        if (!configRootInfo.isDir() || !configRootInfo.isWritable())
            return false;
    } else {
        QDir().mkpath(configRoot);
    }

    QFileInfo configFileInfo(configFile);
    if (configFileInfo.exists()) {
        if (!configFileInfo.isFile() || !configFileInfo.isWritable())
            return false;
        else
            return true;
    } else {
        QFile(configFile).open(QIODevice::WriteOnly);
    }

    QSettings settings(configFile, QSettings::IniFormat);
    settings.setValue(Key::IfNotUtf8, "GBK");
    settings.setValue(Key::WindowGeometry, QByteArray());
    settings.setValue(Key::WindowState, QByteArray());
    settings.setValue(Key::TagEditorSplitterState, QByteArray());
    settings.sync();

    return true;
}
