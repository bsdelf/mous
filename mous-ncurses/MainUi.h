#ifndef MAINUI_H
#define MAINUI_H

struct PrivateMainUi;

class MainUi
{
public:
    MainUi();
    ~MainUi();

    int Exec();

private:
    bool StartClient();
    void StopClient();

    void BeginNcurses();
    void EndNcurses();

private:
    PrivateMainUi* d;
};

#endif
