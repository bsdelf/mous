#ifndef MAINUI_H
#define MAINUI_H

class Client;

class MainUi
{
public:
    MainUi();
    ~MainUi();

    int Exec();

private:
    Client* m_Client;
};

#endif
