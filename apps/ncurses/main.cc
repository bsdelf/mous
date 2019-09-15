#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include <fstream>
#include <locale>

#include "AppEnv.h"
#include "MainUi.h"
#include "Server.h"

using namespace std;

pid_t FetchPid() {
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

void StorePid() {
  const auto env = GlobalAppEnv::Instance();
  fstream stream;
  stream.open(env->pidFile.c_str(), ios::out);
  stream << ::getpid();
  stream.close();
}

void ClearPid() {
  const auto env = GlobalAppEnv::Instance();
  ::unlink(env->pidFile.c_str());
}

void Daemonize() {
  const int destfd = ::open("/dev/null", O_RDWR);
  if (destfd < 0) {
    ::exit(EXIT_FAILURE);
  }
  ::dup2(destfd, STDIN_FILENO);
  ::dup2(destfd, STDOUT_FILENO);
  ::dup2(destfd, STDERR_FILENO);
}

int main(int argc, char** argv) {
  std::locale::global(std::locale(""));

  if (!GlobalAppEnv::Instance()->Init()) {
    return EXIT_FAILURE;
  }

  pid_t pid = FetchPid();
  if (pid == 0 || (::kill(pid, 0) != 0 && errno == ESRCH)) {
    pid = ::fork();
  }

  if (pid == 0) {
    Daemonize();
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
