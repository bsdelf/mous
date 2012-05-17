#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <scx/Singleton.hpp>

struct Config
{
    bool Init();

    // environment
    std::string pluginDir;
    std::string resourceDir;

    std::string configFile;
    std::string pidFile;
    std::string pyMapFile;

    // config content
    std::string serverIp;
    int serverPort;
    std::string ifNotUtf8;

private:
    bool SaveDefault();
    bool LoadContent();
};

typedef scx::Singleton<Config> GlobalConfig;

#endif
