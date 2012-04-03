#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "MidClickTabBar.hpp"
#include "CustomHeadTabWidget.hpp"
#include "DlgListSelect.h"
#include <scx/Signal.hpp>
#include <util/MediaItem.h>
#include "SimplePlayListView.h"
#include "DlgConvertOption.h"
using namespace std;
using namespace sqt;
using namespace mous;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_TimerUpdateUi(new QTimer),
    m_UpdateInterval(500),
    m_PluginManager(IPluginManager::Create()),
    m_MediaLoader(IMediaLoader::Create()),
    m_Player(IPlayer::Create()),
    m_ConvFactory(IConvTaskFactory::Create()),
    m_UsedPlaylistView(NULL),
    m_UsedMediaItem(NULL),
    mSliderPlayingPreempted(false)
{
    ui->setupUi(this);    
    initMousCore();
    initMyUi();
    initQtSlots();
}

MainWindow::~MainWindow()
{
    m_Player->SigFinished()->DisconnectReceiver(this);

    if (m_Player->GetStatus() == PlayerStatus::Playing) {
        m_Player->Close();
    }
    if (m_TimerUpdateUi != NULL) {
        if (m_TimerUpdateUi->isActive())
            m_TimerUpdateUi->stop();
        delete m_TimerUpdateUi;
    }

    delete ui;

    m_ConvFactory->UnregisterAll();
    m_Player->UnregisterAll();
    m_MediaLoader->UnregisterAll();
    m_PluginManager->UnloadAll();

    IPluginManager::Free(m_PluginManager);
    IMediaLoader::Free(m_MediaLoader);
    IPlayer::Free(m_Player);
    IConvTaskFactory::Free(m_ConvFactory);
}

void MainWindow::initMousCore()
{
    m_PluginManager->LoadPluginDir("./plugins");
    vector<string> pathList;
    m_PluginManager->GetPluginPath(pathList);

    vector<const IPluginAgent*> packAgentList;
    vector<const IPluginAgent*> tagAgentList;
    m_PluginManager->GetPlugins(packAgentList, PluginType::MediaPack);
    m_PluginManager->GetPlugins(tagAgentList, PluginType::TagParser);

    m_MediaLoader->RegisterMediaPackPlugin(packAgentList);
    m_MediaLoader->RegisterTagParserPlugin(tagAgentList);

    vector<const IPluginAgent*> decoderAgentList;
    vector<const IPluginAgent*> encoderAgentList;
    vector<const IPluginAgent*> rendererAgentList;
    m_PluginManager->GetPlugins(decoderAgentList, PluginType::Decoder);
    m_PluginManager->GetPlugins(encoderAgentList, PluginType::Encoder);
    m_PluginManager->GetPlugins(rendererAgentList, PluginType::Renderer);

    m_Player->RegisterRendererPlugin(rendererAgentList[0]);
    m_Player->RegisterDecoderPlugin(decoderAgentList);
    m_Player->SigFinished()->Connect(&MainWindow::SlotPlayerStopped, this);

    m_ConvFactory->RegisterDecoderPlugin(decoderAgentList);
    m_ConvFactory->RegisterEncoderPlugin(encoderAgentList);

    qDebug() << ">> MediaPack count:" << packAgentList.size();
    qDebug() << ">> TagParser count:" << tagAgentList.size();
    qDebug() << ">> Decoder count:" << decoderAgentList.size();
    qDebug() << ">> Encoder count:" << encoderAgentList.size();
    qDebug() << ">> Renderer count:" << rendererAgentList.size();
}

void MainWindow::initMyUi()
{
    // Playing & Paused icon
    mIconPlaying.addFile(QString::fromUtf8(":/img/resource/play.png"), QSize(), QIcon::Normal, QIcon::On);
    mIconPaused.addFile(QString::fromUtf8(":/img/resource/pause.png"), QSize(), QIcon::Normal, QIcon::On);

    // Play mode button

    // Volume
    ui->sliderVolume->setValue(m_Player->GetVolume());

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
    SlotWidgetPlayListDoubleClick();
}

void MainWindow::initQtSlots()
{
    connect(m_TimerUpdateUi, SIGNAL(timeout()), this, SLOT(SlotUpdateUi()));

    connect(ui->btnPlay, SIGNAL(clicked()), this, SLOT(SlotBtnPlay()));

    connect(ui->sliderVolume, SIGNAL(valueChanged(int)), this, SLOT(SlotSliderVolumeValueChanged(int)));

    connect(ui->sliderPlaying, SIGNAL(sliderPressed()), this, SLOT(SlotSliderPlayingPressed()));
    connect(ui->sliderPlaying, SIGNAL(sliderReleased()), this, SLOT(SlotSliderPlayingReleased()));
    connect(ui->sliderPlaying, SIGNAL(valueChanged(int)), this, SLOT(SlotSliderPlayingValueChanged(int)));

    connect(mBarPlayList, SIGNAL(sigMidClick(int)), this, SLOT(SlotBarPlayListMidClick(int)));
    connect(mWidgetPlayList, SIGNAL(sigDoubleClick()), this, SLOT(SlotWidgetPlayListDoubleClick()));
}

void MainWindow::formatTime(QString& str, int ms)
{
    int sec = ms/1000;
    str.sprintf("%.2d:%.2d", (int)(sec/60), (int)(sec%60));
}

/* MousCore slots */
void MainWindow::SlotPlayerStopped()
{
    qDebug() << "Stopped!";
    if (m_UsedPlaylistView != NULL) {
        const MediaItem* item = m_UsedPlaylistView->getNextItem();
        SlotPlayMediaItem(m_UsedPlaylistView, item);
    }
}

/* Qt slots */
void MainWindow::SlotUpdateUi()
{
    //==== Update statusbar.
    int total = m_Player->GetRangeDuration();
    int ms = m_Player->GetOffsetMs();
    int kbps = m_Player->GetBitRate();

    mStatusMsg.sprintf("%.3d kbps | %.2d:%.2d/%.2d:%.2d",
                 kbps,
                 ms/1000/60, ms/1000%60, total/1000/60, total/1000%60);

    ui->barStatus->showMessage(mStatusMsg);

    //==== Update slider.
    if (!mSliderPlayingPreempted) {
        int percent = (double)ms / total * ui->sliderPlaying->maximum();
        ui->sliderPlaying->setSliderPosition(percent);
    }
}

void MainWindow::SlotBtnPlay()
{
    qDebug() << m_Player->GetStatus();

    switch (m_Player->GetStatus()) {
    case PlayerStatus::Closed:
        if (m_UsedMediaItem != NULL) {
            m_Player->Open(m_UsedMediaItem->url);
            SlotBtnPlay();
        }
        break;

    case PlayerStatus::Playing:
        m_Player->Pause();
        m_TimerUpdateUi->stop();
        ui->btnPlay->setIcon(mIconPlaying);
        break;

    case PlayerStatus::Paused:
        m_TimerUpdateUi->start(m_UpdateInterval);
        m_Player->Resume();
        ui->btnPlay->setIcon(mIconPaused);
        break;

    case PlayerStatus::Stopped:
        m_TimerUpdateUi->start(m_UpdateInterval);
        if (m_UsedMediaItem->hasRange)
            m_Player->Play(m_UsedMediaItem->msBeg, m_UsedMediaItem->msEnd);
        else
            m_Player->Play();
        ui->btnPlay->setIcon(mIconPaused);
        break;
    }
}

void MainWindow::SlotBtnStop()
{
    qDebug() << m_Player->GetStatus();

    m_Player->Pause();
    m_TimerUpdateUi->stop();
}

void MainWindow::SlotSliderVolumeValueChanged(int val)
{
    m_Player->SetVolume(val);
}

void MainWindow::SlotSliderPlayingPressed()
{
    mSliderPlayingPreempted = true;
}

void MainWindow::SlotSliderPlayingReleased()
{
    mSliderPlayingPreempted = false;
}

void MainWindow::SlotSliderPlayingValueChanged(int val)
{
    if (!mSliderPlayingPreempted)
        return;

    uint64_t ms = (double)val / ui->sliderPlaying->maximum() * m_Player->GetRangeDuration();
    m_Player->Seek(m_Player->GetRangeBegin() + ms);
}

void MainWindow::SlotBarPlayListMidClick(int index)
{
    SimplePlayListView* view = (SimplePlayListView*)mWidgetPlayList->widget(index);
    mWidgetPlayList->removeTab(index);

    disconnect(view, 0, this, 0);

    delete view;

    mBarPlayList->setFocus();
}

void MainWindow::SlotWidgetPlayListDoubleClick()
{
    SimplePlayListView* view = new SimplePlayListView(this);
    view->setMediaLoader(m_MediaLoader);

    connect(view, SIGNAL(sigPlayMediaItem(IPlayListView*, const mous::MediaItem*)),
            this, SLOT(SlotPlayMediaItem(IPlayListView*, const mous::MediaItem*)));
    connect(view, SIGNAL(sigConvertMediaItem(const mous::MediaItem*)),
            this, SLOT(SlotConvertMediaItem(const mous::MediaItem*)));
    connect(view, SIGNAL(sigConvertMediaItems(QList<const mous::MediaItem*>)),
            this, SLOT(SlotConvertMediaItems(QList<const mous::MediaItem*>)));

    mWidgetPlayList->addTab(view, QString::number(mWidgetPlayList->count()));
    mWidgetPlayList->setCurrentIndex(mWidgetPlayList->count()-1);
}

void MainWindow::SlotPlayMediaItem(IPlayListView *view, const MediaItem *item)
{
    if (m_Player->GetStatus() == PlayerStatus::Playing) {
        m_Player->Close();
    }
    if (m_Player->GetStatus() != PlayerStatus::Closed) {
        m_Player->Close();
        m_TimerUpdateUi->stop();
    }

    m_UsedMediaItem = item;
    m_Player->Open(m_UsedMediaItem->url);

    m_TimerUpdateUi->start(m_UpdateInterval);
    if (m_UsedMediaItem->hasRange)
        m_Player->Play(m_UsedMediaItem->msBeg, m_UsedMediaItem->msEnd);
    else
        m_Player->Play();
    ui->btnPlay->setIcon(mIconPaused);

    setWindowTitle(QString::fromUtf8(item->title.c_str()));

    m_UsedPlaylistView = view;
}

void MainWindow::SlotConvertMediaItem(const MediaItem *item)
{
    //==== show encoders
    vector<string> encoderNames = m_ConvFactory->GetEncoderNames();
    if (encoderNames.empty())
        return;
    QStringList list;
    for (size_t i = 0; i < encoderNames.size(); ++i) {
        list << QString::fromUtf8(encoderNames[i].c_str());
        qDebug() << ">> encoder:" << i+1 << encoderNames[i].c_str();
    }

    DlgListSelect dlgEncoders(this);
    dlgEncoders.setWindowTitle(tr("Available Encoders"));
    dlgEncoders.SetItems(list);
    dlgEncoders.SetSelectedIndex(0);
    dlgEncoders.exec();

    if (dlgEncoders.result() != QDialog::Accepted)
        return;

    int encoderIndex = dlgEncoders.GetSelectedIndex();
    IConvTask* newTask = m_ConvFactory->CreateTask(item, encoderNames[encoderIndex]);
    if (newTask == NULL)
        return;

    //==== show options
    vector<const BaseOption*> opts;
    newTask->GetEncoderOptions(opts);

    std::string fileName(item->artist + " - " + item->title + "." + newTask->GetEncoderFileSuffix());
    DlgConvertOption dlgOption(this);
    dlgOption.SetDir(QDir::homePath());
    dlgOption.SetFileName(fileName.c_str());
    dlgOption.BindWidgetAndOption(opts);
    dlgOption.setWindowTitle(tr("Config"));
    dlgOption.exec();

    if (dlgOption.result() != QDialog::Accepted) {
        IConvTask::Free(newTask);
        return;
    }

    //==== do work
    m_DlgConvertTask.show();
    m_DlgConvertTask.AddTask(newTask, "/home/shen/output.wav");
}

void MainWindow::SlotConvertMediaItems(QList<const MediaItem*> items)
{

}
