#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore>
#include <QtGui>
#include <PluginManager.h>
#include <MediaLoader.h>
#include <Player.h>

namespace Ui {
    class MainWindow;
}

namespace sqt {
    class BrowserStyleTabWidget;
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
    void setupQtSlots();
    void formatTime(QString& str, int ms);

private:
    void slotPlayerStopped();

private slots:
    void slotUpdateUi();
    void slotBtnPlay();
    void slotBtnStop();
    void slotSliderPlayingPressed();
    void slotSliderPlayingReleased();
    void slotSliderPlayingValueChanged(int);

private:
    Ui::MainWindow *ui;

    sqt::BrowserStyleTabWidget* mWidgetPlayList;

    QIcon mIconPlaying;
    QIcon mIconPaused;

    QString mStatusMsg;

    QTimer* mTimerUpdateUi;
    const int mUpdateInterval;

    mous::PluginManager mPluginMgr;
    mous::MediaLoader mLoader;
    mous::Player mPlayer;
    mous::MediaItem* mMediaItem;

    bool mSliderPlayingPreempted;
};

#endif // MAINWINDOW_H
