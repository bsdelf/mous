#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include <locale>
#include <fstream>

#include "AppEnv.h"
#include "Server.h"
#include "MainUi.h"

using namespace std;

pid_t FetchPid()
{
    pid_t pid = 0;
    const auto env = GlobalAppEnv::Instance();
    fstream stream;
    stream.open(env->pidFile.c_str(), ios::in);
    if (stream) {
        stream >> pid;
    }
    stream.close();
    return pid;
}

void StorePid()
{
    const auto env = GlobalAppEnv::Instance();
    fstream stream;
    stream.open(env->pidFile.c_str(), ios::out);
    stream << ::getpid();
    stream.close();
}

void ClearPid()
{
    const auto env = GlobalAppEnv::Instance();
    ::unlink(env->pidFile.c_str());
}

int main(int argc, char** argv)
{
    std::locale::global(std::locale(""));
    
    if (!GlobalAppEnv::Instance()->Init()) {
        return 1;
    }

    pid_t pid = FetchPid();
    if (pid == 0 || (::kill(pid, 0) != 0 && errno == ESRCH)) {
        pid = ::fork();
    }

    if (pid == 0) {
        ::daemon(1, 0);
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
