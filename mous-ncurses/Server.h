#ifndef SERVER_H
#define SERVER_H

struct ServerPrivate;

class Server
{
public:
    Server();
    ~Server();

    int Exec();

private:
    ServerPrivate* d;
};

#endif
