#include "Server.h"

struct ServerPrivate
{
};

Server::Server()
{
    d = new ServerPrivate;
}

Server::~Server()
{
    delete d;
}

int Server::Exec()
{
    return 0;
}
