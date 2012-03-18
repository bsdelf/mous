#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore>
#include <QtGui>
#include <core/IPluginManager.h>
#include <core/IMediaLoader.h>
#include <core/IPlayer.h>

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
    void slotPlayerStopped();

private slots:
    void slotUpdateUi();

    void slotBtnPlay();
    void slotBtnStop();

    void slotSliderVolumeValueChanged(int);

    void slotSliderPlayingPressed();
    void slotSliderPlayingReleased();
    void slotSliderPlayingValueChanged(int);

    void slotBarPlayListMidClick(int index);
    void slotWidgetPlayListDoubleClick();

    void slotPlayMediaItem(const mous::MediaItem* item);

private:
    Ui::MainWindow *ui;

    sqt::MidClickTabBar* mBarPlayList;
    sqt::CustomHeadTabWidget* mWidgetPlayList;

    QIcon mIconPlaying;
    QIcon mIconPaused;

    QString mStatusMsg;
    QToolButton* mBtnPreference;

    QTimer* mTimerUpdateUi;
    const int mUpdateInterval;

    mous::IPluginManager* mPluginMgr;
    mous::IMediaLoader* mMediaLoader;
    mous::IPlayer* mPlayer;
    const mous::MediaItem* mMediaItem;

    bool mSliderPlayingPreempted;
};

#endif // MAINWINDOW_H
