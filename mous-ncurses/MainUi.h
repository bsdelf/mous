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

    bool HandleTopKey(int, bool&);

    void OnResize();
    void UpdateTopLayout();
    void ShowOrHideExplorer();
    void ShowOrHideHelp();
    void SwitchFocus();
    void SwitchPlaylist(int);

private:
    PrivateMainUi* d;
};

#endif
