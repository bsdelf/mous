#pragma once

#include <string>
#include <scx/Singleton.h>

struct AppEnv
{
    bool Init();

    // environment
    std::string configDir;
    std::string pluginDir;

    std::string configFile;
    std::string pidFile;
    std::string contextFile;
    std::string playlistFile;

    // config content
    std::string serverIp;
    int serverPort;
    std::string ifNotUtf8;

private:
    bool SaveDefault();
    bool LoadContent();
};

typedef scx::Singleton<AppEnv> GlobalAppEnv;

