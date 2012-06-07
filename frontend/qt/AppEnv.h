#ifndef APPENV_H
#define APPENV_H

#include <QString>
#include <scx/Singleton.hpp>

struct AppEnv
{
public:
    bool Init();
    void Save();

public:
    // path
    QString pluginDir;
    QString resourceDir;

    QString configFile;
    QString translationFile;

    // config
    QString ifNotUtf8;

    // ui    
    QByteArray windowGeometry;
    QByteArray windowState;
    QByteArray tagEditorSplitterState;

private:
    void InitFilePath();
    bool LoadConfig();
    bool CheckDefaultConfig();
};

typedef scx::Singleton<AppEnv> GlobalAppEnv;

#endif // APPENV_H
