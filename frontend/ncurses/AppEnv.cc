#include "AppEnv.h"

#include <iostream>
using namespace std;

#include <scx/Conv.hpp>
#include <scx/Env.hpp>
#include <scx/Dir.hpp>
#include <scx/FileInfo.hpp>
#include <scx/ConfigFile.hpp>
using namespace scx;

namespace Path {
    const char* const ConfigRoot = "/.config/mous/ncurses/";
    const char* const PluginRoot = "/lib/mous/";
    const char* const ResourceRoot = "/share/mous/";
    const char* const ResourcePinYin = "/pinyin/";

    const char* const PyMapFile = "unipy.map";
    const char* const ConfigFile = "config";
    const char* const PidFile = "server.pid";

    const char* const ContextFile = "context.dat";
    const char* const PlaylistFile = "playlist%d.dat";
}

namespace Field {
    const char* const ServerIp = "ServerIp";
    const char* const ServerPort = "ServerPort";

    const char* const IfNotUtf8 = "IfNotUtf8";
}

bool AppEnv::Init()
{
    // prepare installed dir
    FileInfo pluginDirInfo(string(CMAKE_INSTALL_PREFIX) + Path::PluginRoot);
    if (!pluginDirInfo.Exists() || pluginDirInfo.Type() != FileType::Directory)
        return false;
    pluginDir = pluginDirInfo.AbsFilePath();

    FileInfo resDirInfo(string(CMAKE_INSTALL_PREFIX) + Path::ResourceRoot);
    if (!resDirInfo.Exists() || resDirInfo.Type() != FileType::Directory)
        return false;
    resourceDir = resDirInfo.AbsFilePath();

    // prepare root dir
    configDir = Env::Env(Env::Home) + Path::ConfigRoot;

    pyMapFile = resourceDir + Path::ResourcePinYin + Path::PyMapFile;
    configFile = configDir + Path::ConfigFile;
    pidFile = configDir + Path::PidFile;
    contextFile = configDir + Path::ContextFile;
    playlistFile = configDir + Path::PlaylistFile;

    FileInfo configRootDirInfo(configDir);
    if (configRootDirInfo.Exists()) {
        if (configRootDirInfo.Type() != FileType::Directory)
            return false;
    } else {
        if (!Dir::MakeDir(configDir, 0777))
            return false;

        if (!SaveDefault())
            return false;
    }

    return LoadContent();
}

bool AppEnv::SaveDefault()
{
    ConfigFile config;

    config.AppendComment("# server ip");
    config[Field::ServerIp] = "127.0.0.1";
    config.AppendComment("");

    config.AppendComment("# server port");
    config[Field::ServerPort] = "21027";
    config.AppendComment("");

    config.AppendComment("# if tag is not utf8, use the following encoding");
    config[Field::IfNotUtf8] = "GBK";
    config.AppendComment("");

    if (config.Save(configFile)) {
        cout << "New config file was generated: " << configFile << endl;
        sleep(1);
        return true;
    } else {
        cerr << "InitConfig(): Failed to write config" << endl;
        return false;
    }
}

bool AppEnv::LoadContent()
{
    ConfigFile config;
    if (!config.Load(configFile, '#'))
        return false;

    serverIp = config[Field::ServerIp];
    serverPort = StrToNum<int>(config[Field::ServerPort]);
    ifNotUtf8 = config[Field::IfNotUtf8];

    return true;
}
