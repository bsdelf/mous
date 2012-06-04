#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include <iostream>
#include <fstream>
using namespace std;

#include "AppEnv.h"
#include "Server.h"
#include "MainUi.h"

pid_t FetchPid()
{
    pid_t pid = 0;
    const AppEnv* env = GlobalAppEnv::Instance();
    if (env != NULL) {
        fstream stream;
        stream.open(env->pidFile.c_str(), ios::in);
        if (stream.is_open()) {
            stream >> pid;
        }
        stream.close();
    }
    return pid;
}

void StorePid()
{
    const AppEnv* config = GlobalAppEnv::Instance();
    if (config != NULL) {
        fstream stream;
        stream.open(config->pidFile.c_str(), ios::out);
        stream << getpid();
        stream.close();
    }
}

void ClearPid()
{
    const AppEnv* env = GlobalAppEnv::Instance();
    if (env != NULL) {
        unlink(env->pidFile.c_str());
    }
}

int main(int argc, char** argv)
{
    if (!GlobalAppEnv::Instance()->Init())
        return 1;

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
