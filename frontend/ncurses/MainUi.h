#ifndef MAINUI_H
#define MAINUI_H

#include <string>
#include <deque>
using namespace std;

struct PrivateMainUi;

namespace mous {
    struct MediaItem;
}

class MainUi
{
    friend struct PrivateMainUi;

public:
    MainUi();
    ~MainUi();

    int Exec();

private:
    void SlotTryConnect();
    void SlotConnected();
    void SlotSwitchPlaylist(bool);
    void SlotTmpOpen(const std::string&);
    void SlotReqUserOpen(const std::string&);

private:
    bool StartClient();
    void StopClient();

    void BeginNcurses();
    void EndNcurses();

    bool HandleTopKey(int, bool&);
    void SyncRefresh();

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
