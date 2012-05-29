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
#include <string>
#include "FrmToolBar.h"
#include "FrmTagEditor.h"
#include "IPlaylistView.h"
#include "DlgConvertTask.h"

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
    void InitMyUi();
    void InitMousCore();
    void ClearMousCore();
    void InitQtSlots();

    void FormatTime(QString& str, int ms);

private:
    void SlotPlayerStopped();
private slots:
    void SlotUiPlayerStopped();

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

    void SlotPlayMediaItem(IPlaylistView* view, const mous::MediaItem* item);
    void SlotConvertMediaItem(const mous::MediaItem *item);
    void SlotConvertMediaItems(QList<const mous::MediaItem*> items);

private:
    Ui::MainWindow *ui;
    FrmToolBar m_FrmToolBar;
    FrmTagEditor m_FrmTagEditor;
    sqt::MidClickTabBar* m_TabBarPlaylist;
    sqt::CustomHeadTabWidget* m_TabWidgetPlaylist;

    QIcon m_IconPlaying;
    QIcon m_IconPaused;

    QToolButton* m_BtnPreference;

    QTimer* m_TimerUpdateUi;
    const int m_UpdateInterval;

    mous::IPluginManager* m_PluginManager;
    mous::IMediaLoader* m_MediaLoader;
    mous::IPlayer* m_Player;
    mous::IConvTaskFactory* m_ConvFactory;
    mous::ITagParserFactory* m_ParserFactory;

    IPlaylistView* m_UsedPlaylistView;
    const mous::MediaItem* m_UsedMediaItem;

    bool m_SliderPlayingPreempted;

    DlgConvertTask m_DlgConvertTask;
};

#endif // MAINWINDOW_H
