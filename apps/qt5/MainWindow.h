#pragma once

#include <QtWidgets>

#include <core/MediaLoader.h>
#include <core/Player.h>
#include <core/ConvTask.h>
#include <core/ConvTaskFactory.h>
#include <core/TagParserFactory.h>
#include <util/MediaItem.h>
using namespace mous;

#include <string>
using namespace std;

#include "FrmToolBar.h"
#include "FrmTagEditor.h"
#include "IPlaylistView.h"
#include "DlgConvertTask.h"
#include "PlaylistClipboard.h"

namespace Ui {
    class MainWindow;
}

namespace sqt {
    class MidClickTabBar;
    class CustomHeadTabWidget;
}

namespace mous {
    struct MediaItem;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void closeEvent(QCloseEvent *);

private:
    void InitMyUi();
    void InitMousCore();
    void ClearMousCore();
    void InitQtSlots();

private:
    void SlotPlayerFinished();
private slots:
    void SlotUiPlayerFinished();

private slots:
    void SlotUpdateUi();

    void SlotBtnPlay();
    void SlotBtnPrev();
    void SlotBtnNext();

    void SlotSliderVolumeValueChanged(int);

    void SlotSliderPlayingPressed();
    void SlotSliderPlayingReleased();
    void SlotSliderPlayingValueChanged(int);

    void SlotBarPlayListMidClick(int index);
    void SlotWidgetPlayListDoubleClick();

    void SlotPlayMediaItem(IPlaylistView* view, const MediaItem& item);
    void SlotConvertMediaItem(const MediaItem& item);
    void SlotConvertMediaItems(const QList<MediaItem>& items);

    void SlotTagUpdated(const MediaItem& item);

private:
    Ui::MainWindow *ui = nullptr;
    QDockWidget* m_Dock = nullptr;
    FrmToolBar m_FrmToolBar;
    FrmTagEditor m_FrmTagEditor;
    sqt::MidClickTabBar* m_TabBarPlaylist = nullptr;
    sqt::CustomHeadTabWidget* m_TabWidgetPlaylist = nullptr;

    QIcon m_IconPlaying;
    QIcon m_IconPaused;

    QToolButton* m_BtnPreference = nullptr;

    QTimer m_TimerUpdateUi;
    const int m_UpdateInterval = 500;

    MediaLoader* m_MediaLoader = nullptr;
    Player* m_Player = nullptr;
    ConvTaskFactory* m_ConvFactory = nullptr;
    TagParserFactory* m_ParserFactory = nullptr;
    QMutex m_PlayerMutex { QMutex::Recursive };

    IPlaylistView* m_UsedPlaylistView = nullptr;
    const MediaItem* m_UsedMediaItem = nullptr;
    PlaylistClipboard<MediaItem> m_Clipboard;

    bool m_SliderPlayingPreempted = false;

    DlgConvertTask m_DlgConvertTask;
};

