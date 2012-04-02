#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore>
#include <QtGui>
#include <core/IPluginManager.h>
#include <core/IMediaLoader.h>
#include <core/IPlayer.h>
#include <core/IConvTask.h>
#include <core/IConvTaskFactory.h>
#include "IPlayListView.h"
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
    void initMyUi();
    void initMousCore();
    void initQtSlots();

    void formatTime(QString& str, int ms);

private:
    void SlotPlayerStopped();

private slots:
    void SlotUpdateUi();

    void SlotBtnPlay();
    void SlotBtnStop();

    void SlotSliderVolumeValueChanged(int);

    void SlotSliderPlayingPressed();
    void SlotSliderPlayingReleased();
    void SlotSliderPlayingValueChanged(int);

    void SlotBarPlayListMidClick(int index);
    void SlotWidgetPlayListDoubleClick();

    void SlotPlayMediaItem(IPlayListView* view, const mous::MediaItem* item);
    void SlotConvertMediaItem(const mous::MediaItem *item);
    void SlotConvertMediaItems(QList<const mous::MediaItem*> items);

private:
    Ui::MainWindow *ui;

    sqt::MidClickTabBar* mBarPlayList;
    sqt::CustomHeadTabWidget* mWidgetPlayList;

    QIcon mIconPlaying;
    QIcon mIconPaused;

    QString mStatusMsg;
    QToolButton* mBtnPreference;

    QTimer* m_TimerUpdateUi;
    const int m_UpdateInterval;

    mous::IPluginManager* m_PluginManager;
    mous::IMediaLoader* m_MediaLoader;
    mous::IPlayer* m_Player;
    mous::IConvTaskFactory* m_ConvFactory;

    IPlayListView* m_UsedPlaylistView;
    const mous::MediaItem* m_UsedMediaItem;

    bool mSliderPlayingPreempted;

    DlgConvertTask m_DlgConvertTask;
};

#endif // MAINWINDOW_H
