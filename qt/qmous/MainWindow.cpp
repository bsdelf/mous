#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "MidClickTabBar.hpp"
#include "CustomHeadTabWidget.hpp"
#include <scx/Signal.hpp>
#include <util/MediaItem.h>
#include "SimplePlayListView.h"
using namespace std;
using namespace sqt;
using namespace mous;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mTimerUpdateUi(new QTimer),
    mUpdateInterval(500),
    mPluginMgr(IPluginManager::Create()),
    mMediaLoader(IMediaLoader::Create()),
    mPlayer(IPlayer::Create()),
    mMediaItem(NULL),
    mSliderPlayingPreempted(false)
{
    ui->setupUi(this);    
    initMousCore();
    initMyUi();
    initQtSlots();
}

MainWindow::~MainWindow()
{
    if (mPlayer->GetStatus() == PlayerStatus::Playing) {
        mPlayer->Close();
    }
    if (mTimerUpdateUi != NULL) {
        if (mTimerUpdateUi->isActive())
            mTimerUpdateUi->stop();
        delete mTimerUpdateUi;
    }

    delete ui;

    mPlayer->UnregisterAll();
    mMediaLoader->UnregisterAll();
    mPluginMgr->UnloadAll();

    IPluginManager::Free(mPluginMgr);
    IMediaLoader::Free(mMediaLoader);
    IPlayer::Free(mPlayer);
}

void MainWindow::initMousCore()
{
    mPluginMgr->LoadPluginDir("./plugins");
    vector<string> pathList;
    mPluginMgr->GetPluginPath(pathList);

    vector<const IPluginAgent*> packAgentList;
    vector<const IPluginAgent*> tagAgentList;
    mPluginMgr->GetPlugins(packAgentList, PluginType::MediaPack);
    mPluginMgr->GetPlugins(tagAgentList, PluginType::TagParser);

    mMediaLoader->RegisterMediaPackPlugin(packAgentList);
    mMediaLoader->RegisterTagParserPlugin(tagAgentList);

    vector<const IPluginAgent*> decoderAgentList;
    vector<const IPluginAgent*> rendererAgentList;
    mPluginMgr->GetPlugins(decoderAgentList, PluginType::Decoder);
    mPluginMgr->GetPlugins(rendererAgentList, PluginType::Renderer);

    mPlayer->RegisterRendererPlugin(rendererAgentList[0]);
    mPlayer->RegisterDecoderPlugin(decoderAgentList);

    mPlayer->SigFinished()->Connect(&MainWindow::slotPlayerStopped, this);

    qDebug() << ">> MediaPack count:" << packAgentList.size();
    qDebug() << ">> TagParser count:" << tagAgentList.size();
    qDebug() << ">> Decoder count:" << decoderAgentList.size();
    qDebug() << ">> Renderer count:" << rendererAgentList.size();
}

void MainWindow::initMyUi()
{
    // Playing & Paused icon
    mIconPlaying.addFile(QString::fromUtf8(":/img/resource/play.png"), QSize(), QIcon::Normal, QIcon::On);
    mIconPaused.addFile(QString::fromUtf8(":/img/resource/pause.png"), QSize(), QIcon::Normal, QIcon::On);

    // Play mode button

    // Volume
    ui->sliderVolume->setValue(mPlayer->GetVolume());

    // PlayList View
    mBarPlayList = new MidClickTabBar(this);
    mWidgetPlayList = new CustomHeadTabWidget(this);
    mWidgetPlayList->setTabBar(mBarPlayList);
    mWidgetPlayList->setMovable(true);
    ui->layoutPlayList->addWidget(mWidgetPlayList);

    // Status bar buttons
    mBtnPreference = new QToolButton(ui->barStatus);
    mBtnPreference->setAutoRaise(true);
    mBtnPreference->setText("P");
    mBtnPreference->setToolTip(tr("Preference"));

    ui->barStatus->addPermanentWidget(mBtnPreference, 0);

    // Default playlist
    slotWidgetPlayListDoubleClick();
}

void MainWindow::initQtSlots()
{
    connect(mTimerUpdateUi, SIGNAL(timeout()), this, SLOT(slotUpdateUi()));

    connect(ui->btnPlay, SIGNAL(clicked()), this, SLOT(slotBtnPlay()));
    connect(ui->btnStop, SIGNAL(clicked()), this, SLOT(slotBtnStop()));

    connect(ui->sliderVolume, SIGNAL(valueChanged(int)), this, SLOT(slotSliderVolumeValueChanged(int)));

    connect(ui->sliderPlaying, SIGNAL(sliderPressed()), this, SLOT(slotSliderPlayingPressed()));
    connect(ui->sliderPlaying, SIGNAL(sliderReleased()), this, SLOT(slotSliderPlayingReleased()));
    connect(ui->sliderPlaying, SIGNAL(valueChanged(int)), this, SLOT(slotSliderPlayingValueChanged(int)));

    connect(mBarPlayList, SIGNAL(sigMidClick(int)), this, SLOT(slotBarPlayListMidClick(int)));
    connect(mWidgetPlayList, SIGNAL(sigDoubleClick()), this, SLOT(slotWidgetPlayListDoubleClick()));
}

void MainWindow::formatTime(QString& str, int ms)
{
    int sec = ms/1000;
    str.sprintf("%.2d:%.2d", (int)(sec/60), (int)(sec%60));
}

/* MousCore slots */
void MainWindow::slotPlayerStopped()
{
    qDebug() << "Stopped!";
    if (mPlaylistView != NULL) {
        const MediaItem* item = mPlaylistView->getNextItem();
        slotPlayMediaItem(mPlaylistView, item);
    }
}

/* Qt slots */
void MainWindow::slotUpdateUi()
{
    // Update statusbar.
    int total = mPlayer->GetRangeDuration();
    int ms = mPlayer->GetOffsetMs();
    int kbps = mPlayer->GetBitRate();

    mStatusMsg.sprintf("%d kbps | %.2d:%.2d/%.2d:%.2d",
                 kbps,
                 ms/1000/60, ms/1000%60, total/1000/60, total/1000%60);

    ui->barStatus->showMessage(mStatusMsg);

    // Update slider.
    if (!mSliderPlayingPreempted) {
        int percent = (double)ms / total * ui->sliderPlaying->maximum();
        ui->sliderPlaying->setSliderPosition(percent);
    }
}

void MainWindow::slotBtnPlay()
{
    qDebug() << mPlayer->GetStatus();

    switch (mPlayer->GetStatus()) {
    case PlayerStatus::Closed:
        if (mMediaItem != NULL) {
            mPlayer->Open(mMediaItem->url);
            slotBtnPlay();
        }
        break;

    case PlayerStatus::Playing:
        mPlayer->Pause();
        mTimerUpdateUi->stop();
        ui->btnPlay->setIcon(mIconPlaying);
        break;

    case PlayerStatus::Paused:
        mTimerUpdateUi->start(mUpdateInterval);
        mPlayer->Resume();
        ui->btnPlay->setIcon(mIconPaused);
        break;

    case PlayerStatus::Stopped:
        mTimerUpdateUi->start(mUpdateInterval);
        if (mMediaItem->hasRange)
            mPlayer->Play(mMediaItem->msBeg, mMediaItem->msEnd);
        else
            mPlayer->Play();
        ui->btnPlay->setIcon(mIconPaused);
        break;
    }
}

void MainWindow::slotBtnStop()
{
    qDebug() << mPlayer->GetStatus();

    mPlayer->Pause();
    mTimerUpdateUi->stop();
}

void MainWindow::slotSliderVolumeValueChanged(int val)
{
    mPlayer->SetVolume(val);
}

void MainWindow::slotSliderPlayingPressed()
{
    mSliderPlayingPreempted = true;
}

void MainWindow::slotSliderPlayingReleased()
{
    mSliderPlayingPreempted = false;
}

void MainWindow::slotSliderPlayingValueChanged(int val)
{
    if (!mSliderPlayingPreempted)
        return;

    uint64_t ms = (double)val / ui->sliderPlaying->maximum() * mPlayer->GetRangeDuration();
    mPlayer->Seek(mPlayer->GetRangeBegin() + ms);
}

void MainWindow::slotBarPlayListMidClick(int index)
{
    SimplePlayListView* view = (SimplePlayListView*)mWidgetPlayList->widget(index);
    mWidgetPlayList->removeTab(index);
    disconnect(view, SIGNAL(sigPlayMediaItem(const mous::MediaItem*)),
            this, SLOT(slotPlayMediaItem(const mous::MediaItem*)));
    delete view;

    mBarPlayList->setFocus();
}

void MainWindow::slotWidgetPlayListDoubleClick()
{
    SimplePlayListView* view = new SimplePlayListView(this);
    view->setMediaLoader(mMediaLoader);
    connect(view, SIGNAL(sigPlayMediaItem(IPlayListView*, const mous::MediaItem*)),
            this, SLOT(slotPlayMediaItem(IPlayListView*, const mous::MediaItem*)));

    mWidgetPlayList->addTab(view, QString::number(mWidgetPlayList->count()));
    mWidgetPlayList->setCurrentIndex(mWidgetPlayList->count()-1);
}

void MainWindow::slotPlayMediaItem(IPlayListView *view, const MediaItem *item)
{
    if (mPlayer->GetStatus() == PlayerStatus::Playing) {
        mPlayer->Close();
    }
    if (mPlayer->GetStatus() != PlayerStatus::Closed) {
        mPlayer->Close();
        mTimerUpdateUi->stop();
    }

    mMediaItem = item;
    mPlayer->Open(mMediaItem->url);

    mTimerUpdateUi->start(mUpdateInterval);
    if (mMediaItem->hasRange)
        mPlayer->Play(mMediaItem->msBeg, mMediaItem->msEnd);
    else
        mPlayer->Play();
    ui->btnPlay->setIcon(mIconPaused);

    mPlaylistView = view;
}
