#ifndef APPENV_H
#define APPENV_H

#include <QString>
#include <scx/Singleton.h>

struct AppEnv
{
public:
    bool Init();
    void Save();

public:
    // path
    QString configDir;
    QString pluginDir;
    QString resourceDir;

    QString configFile;
    QString translationFile;

    // config
    QString ifNotUtf8;

    // ui && status
    QByteArray windowGeometry;
    QByteArray windowState;
    QByteArray tagEditorSplitterState;
    int tabCount;
    int tabIndex;
    int volume;

private:
    void InitFilePath();
    bool LoadConfig();
    bool CheckDefaultConfig();
};

typedef scx::Singleton<AppEnv> GlobalAppEnv;

#endif // APPENV_H
