#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore>
#include <QtGui>

#include <core/IPluginManager.h>
#include <core/IMediaLoader.h>
#include <core/IPlayer.h>
#include <core/IConvTask.h>
#include <core/IConvTaskFactory.h>
#include <core/ITagParserFactory.h>
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
    Ui::MainWindow *ui;
    QDockWidget* m_Dock;
    FrmToolBar m_FrmToolBar;
    FrmTagEditor m_FrmTagEditor;
    sqt::MidClickTabBar* m_TabBarPlaylist;
    sqt::CustomHeadTabWidget* m_TabWidgetPlaylist;

    QIcon m_IconPlaying;
    QIcon m_IconPaused;

    QToolButton* m_BtnPreference;

    QTimer* m_TimerUpdateUi;
    const int m_UpdateInterval;

    IPluginManager* m_PluginManager;
    IMediaLoader* m_MediaLoader;
    IPlayer* m_Player;
    IConvTaskFactory* m_ConvFactory;
    ITagParserFactory* m_ParserFactory;
    QMutex m_PlayerMutex;

    IPlaylistView* m_UsedPlaylistView;
    const MediaItem* m_UsedMediaItem;
    PlaylistClipboard<MediaItem> m_Clipboard;

    bool m_SliderPlayingPreempted;

    DlgConvertTask m_DlgConvertTask;
};

#endif // MAINWINDOW_H
