#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#include "Server.h"
#include "MainUi.h"

int main(int argc, char** argv)
{
    pid_t pid = fork();

    if (pid == 0) {
        Server server;
        return server.Exec();
    } else if (pid > 0) {
        MainUi ui;
        return ui.Exec();
    } else {
        perror("mous-ncurses: Failed to start");
        return -1;
    }
}
