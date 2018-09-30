#include "AppEnv.h"

#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

#include <scx/Env.h>
#include <scx/Dir.h>
#include <scx/FileInfo.h>
#include <scx/Config.h>
using namespace scx;

namespace Path {
    const char* const ConfigRoot = "/.config/mous/ncurses/";
    const char* const PluginRoot = "/lib/mous/";

    const char* const ConfigFile = "config";
    const char* const PidFile = "server.pid";

    const char* const ContextFile = "context.dat";
    const char* const PlaylistFile = "playlist%d.dat";
}

namespace Key {
    const char* const ServerIp = "ServerIp";
    const char* const ServerPort = "ServerPort";
    const char* const IfNotUtf8 = "IfNotUtf8";
}

bool AppEnv::Init()
{
    // init path
    FileInfo pluginDirInfo(string(CMAKE_INSTALL_PREFIX) + Path::PluginRoot);
    if (!pluginDirInfo.Exists() || pluginDirInfo.Type() != FileType::Directory) {
        return false;
    }
    pluginDir = pluginDirInfo.AbsFilePath();
    configDir = Env::Get("HOME") + Path::ConfigRoot;
    configFile = configDir + Path::ConfigFile;
    pidFile = configDir + Path::PidFile;
    contextFile = configDir + Path::ContextFile;
    playlistFile = configDir + Path::PlaylistFile;

    // ensure root directory
    FileInfo configRootDirInfo(configDir);
    if (configRootDirInfo.Exists()) {
        if (configRootDirInfo.Type() != FileType::Directory) {
            return false;
        }
    } else {
        if (!Dir::MakeDir(configDir, 0777)) {
            return false;
        }
    }

    // ensure config file
    FileInfo configFileInfo(configFile);
    if (configFileInfo.Exists()) {
        if (configFileInfo.Type() != FileType::Regular) {
            return false;
        }
    } else {
        if (!SaveDefault()) {
            return false;
        }
    }

    return LoadContent();
}

bool AppEnv::SaveDefault()
{
    const auto& str = Config()
        .Append("server ip")
        .Append(Key::ServerIp, "127.0.0.1")
        .Append()
        .Append("server port")
        .Append(Key::ServerPort, "21027")
        .Append()
        .Append("fallback encoding")
        .Append(Key::IfNotUtf8, "GBK")
        .Append()
        .ToString();
    std::ofstream outfile;
    outfile.open(configFile.c_str(), std::ios::out);
    if (!outfile.is_open()) {
        cerr << "InitConfig(): Failed to write config" << endl;
        return false;
    }
    outfile.write(str.data(), str.size());
    outfile.close();
    cout << "New config file was generated: " << configFile << endl;
    sleep(1);
    return true;
}

bool AppEnv::LoadContent()
{
    std::ifstream infile;
    infile.open(configFile, std::ios::in);
    if (!infile.is_open()) {
        return false;
    }
    const auto& str = (std::stringstream() << infile.rdbuf()).str();
    infile.close();
    const Config config(str);
    serverIp = config[Key::ServerIp];
    serverPort = std::stoi(config[Key::ServerPort]);
    ifNotUtf8 = config[Key::IfNotUtf8];
    return true;
}
