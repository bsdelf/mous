#ifndef APPENV_H
#define APPENV_H

#include <QString>
#include <scx/Singleton.hpp>

struct AppEnv
{
public:
    bool Init();

public:
    // path
    QString pluginDir;
    QString resourceDir;

    QString translationFile;

    // config
    QString ifNotUtf8;

    // ui
    int width;
    int height;

private:
    void ChooseTranslation();

};

typedef scx::Singleton<AppEnv> GlobalAppEnv;

#endif // APPENV_H
