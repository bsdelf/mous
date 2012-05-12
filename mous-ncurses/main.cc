#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include <iostream>
#include <fstream>
using namespace std;

#include <scx/ConfigFile.hpp>
using namespace scx;

#include "Config.h"
#include "Server.h"
#include "MainUi.h"

const char* const PID_FILE = "/home/shen/project/mous/build/server.pid";

bool InitConfig()
{
    return true;

    ConfigFile config;
    //config.Load("Config::ConfigPath");

    config.AppendComment("# server ip");
    config[Config::ServerIp] = "127.0.0.1";
    config.AppendComment("");

    config.AppendComment("# server port");
    config[Config::ServerPort] = "21027";
    config.AppendComment("");

    if (config.Save(Config::ConfigPath)) {
        cout << "Seems it's the first time you run me. >_<" << endl;
        cout << "Config file: " << Config::ConfigPath << endl;

        for (int i = 5; i > 0; --i) {
            cout << "." << i << flush;
            usleep(500*1000);
        }
        cout << endl;

        cout << "Enjoy it! ;-)" << endl;
        sleep(1);

        return true;
    } else {
        cerr << "InitConfig(): Failed to write config" << endl;
        return false;
    }
}

pid_t FetchPid()
{
    pid_t pid = 0;
    fstream stream;
    stream.open(PID_FILE, ios::in);
    if (stream.is_open()) {
        stream >> pid;
    }
    stream.close();
    return pid;
}

void StorePid()
{
    fstream stream;
    stream.open(PID_FILE, ios::out);
    stream << getpid();
    stream.close();
    cout << getpid() << endl;
}

void ClearPid()
{
    unlink(PID_FILE);
}

int main(int argc, char** argv)
{
    if (!InitConfig())
        return -1;

    pid_t pid = FetchPid();
    if (pid == 0 || (kill(pid, 0) != 0 && errno == ESRCH))
        pid = fork();

    if (pid == 0) {
        daemon(1, 0);
        StorePid();
        Server server;
        int ret = server.Exec();
        ClearPid();
        return ret;
    } else if (pid > 0) {
        MainUi ui;
        return ui.Exec();
    }
}
