#include <sys/types.h>
#include <unistd.h>

#include <iostream>
using namespace std;
#include <scx/ConfigFile.hpp>
using namespace scx;

#include "Config.h"
#include "Server.h"
#include "MainUi.h"

bool InitConfig()
{
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

int main(int argc, char** argv)
{
    if (!InitConfig())
        return -1;

    pid_t pid = fork();

    if (pid == 0) {
        Server server;
        return server.Exec();
    } else if (pid > 0) {
        MainUi ui;
        return ui.Exec();
    } else {
        cerr << "main(): Failed to fork()" << endl;
        return -1;
    }
}
