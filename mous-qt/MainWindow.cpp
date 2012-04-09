#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "MidClickTabBar.hpp"
#include "CustomHeadTabWidget.hpp"
#include "DlgListSelect.h"
#include <scx/Signal.hpp>
#include <util/MediaItem.h>
#include "SimplePlaylistView.h"
#include "DlgConvertOption.h"
using namespace std;
using namespace sqt;
using namespace mous;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_TimerUpdateUi(new QTimer),
    m_UpdateInterval(500),
    m_UsedPlaylistView(NULL),
    m_UsedMediaItem(NULL),
    m_SliderPlayingPreempted(false)
{
    ui->setupUi(this);    
    InitMousCore();
    InitMyUi();
    InitQtSlots();
}

MainWindow::~MainWindow()
{
    m_FrmTagEditor.WaitForLoadFinished();

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

    ClearMousCore();
}

void MainWindow::InitMousCore()
{
    m_PluginManager = IPluginManager::Create();
    m_MediaLoader = IMediaLoader::Create();
    m_Player = IPlayer::Create();
    m_ConvFactory = IConvTaskFactory::Create();
    m_ParserFactory = ITagParserFactory::Create();

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

    m_ParserFactory->RegisterTagParserPlugin(tagAgentList);
    m_FrmTagEditor.SetTagParserFactory(m_ParserFactory);

    qDebug() << ">> MediaPack count:" << packAgentList.size();
    qDebug() << ">> TagParser count:" << tagAgentList.size();
    qDebug() << ">> Decoder count:" << decoderAgentList.size();
    qDebug() << ">> Encoder count:" << encoderAgentList.size();
    qDebug() << ">> Renderer count:" << rendererAgentList.size();
}

void MainWindow::ClearMousCore()
{
    m_Player->SigFinished()->DisconnectReceiver(this);
    m_FrmTagEditor.SetTagParserFactory(NULL);

    m_Player->UnregisterAll();
    m_MediaLoader->UnregisterAll();
    m_ConvFactory->UnregisterAll();
    m_ParserFactory->UnregisterAll();
    m_PluginManager->UnloadAll();

    IPluginManager::Free(m_PluginManager);
    IMediaLoader::Free(m_MediaLoader);
    IPlayer::Free(m_Player);
    IConvTaskFactory::Free(m_ConvFactory);
    ITagParserFactory::Free(m_ParserFactory);
}

void MainWindow::InitMyUi()
{
    // Playing & Paused icon
    m_IconPlaying.addFile(QString::fromUtf8(":/img/resource/play.png"), QSize(), QIcon::Normal, QIcon::On);
    m_IconPaused.addFile(QString::fromUtf8(":/img/resource/pause.png"), QSize(), QIcon::Normal, QIcon::On);

    // Volume
    m_FrmToolBar.GetSliderVolume()->setValue(m_Player->GetVolume());

    // PlayList View
    m_TabBarPlaylist = new MidClickTabBar(this);
    m_TabWidgetPlaylist = new CustomHeadTabWidget(this);
    m_TabWidgetPlaylist->SetTabBar(m_TabBarPlaylist);
    m_TabWidgetPlaylist->setMovable(true);
    ui->layoutPlaylist->addWidget(m_TabWidgetPlaylist);

    // Status bar buttons
    m_BtnPreference = new QToolButton(ui->statusBar);
    m_BtnPreference->setAutoRaise(true);
    m_BtnPreference->setText("P");
    m_BtnPreference->setToolTip(tr("Preference"));

    ui->statusBar->addPermanentWidget(m_BtnPreference, 0);

    // Show default playlist
    SlotWidgetPlayListDoubleClick();

    QDockWidget* dock = new QDockWidget("Metadata");
    dock->setWidget(&m_FrmTagEditor);
    addDockWidget(Qt::LeftDockWidgetArea, dock);
    dock->setFeatures(QDockWidget::NoDockWidgetFeatures | QDockWidget::DockWidgetMovable);

    ui->toolBar->addWidget(&m_FrmToolBar);
    ui->toolBar->setMovable(false);
    setContextMenuPolicy(Qt::NoContextMenu);
}

void MainWindow::InitQtSlots()
{
    connect(m_TimerUpdateUi, SIGNAL(timeout()), this, SLOT(SlotUpdateUi()));

    connect(this, SIGNAL(SigLoadFileTag(QString)),
            &m_FrmTagEditor, SLOT(SlotLoadFileTag(QString)), Qt::QueuedConnection);

    connect(m_FrmToolBar.GetBtnPlay(), SIGNAL(clicked()), this, SLOT(SlotBtnPlay()));

    connect(m_FrmToolBar.GetSliderVolume(), SIGNAL(valueChanged(int)), this, SLOT(SlotSliderVolumeValueChanged(int)));

    connect(m_FrmToolBar.GetSliderPlaying(), SIGNAL(sliderPressed()), this, SLOT(SlotSliderPlayingPressed()));
    connect(m_FrmToolBar.GetSliderPlaying(), SIGNAL(sliderReleased()), this, SLOT(SlotSliderPlayingReleased()));
    connect(m_FrmToolBar.GetSliderPlaying(), SIGNAL(valueChanged(int)), this, SLOT(SlotSliderPlayingValueChanged(int)));

    connect(m_TabBarPlaylist, SIGNAL(SigMidClick(int)), this, SLOT(SlotBarPlayListMidClick(int)));
    connect(m_TabWidgetPlaylist, SIGNAL(SigDoubleClick()), this, SLOT(SlotWidgetPlayListDoubleClick()));
}

void MainWindow::FormatTime(QString& str, int ms)
{
    int sec = ms/1000;
    str.sprintf("%.2d:%.2d", (int)(sec/60), (int)(sec%60));
}

/* MousCore slots */
void MainWindow::SlotPlayerStopped()
{
    QMetaObject::invokeMethod(this, "SlotUiPlayerStopped", Qt::QueuedConnection);
}

void MainWindow::SlotUiPlayerStopped()
{
    qDebug() << "Stopped!";
    if (m_UsedPlaylistView != NULL) {
        const MediaItem* item = m_UsedPlaylistView->GetNextItem();
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

    const QString& status = QString("%1 kbps | %2:%3/%4:%5").arg(kbps).
            arg(ms/1000/60, 2, 10, QChar('0')).arg(ms/1000%60, 2, 10, QChar('0')).
            arg(total/1000/60, 2, 10, QChar('0')).arg(total/1000%60, 2, 10, QChar('0'));

    ui->statusBar->showMessage(status);

    //==== Update slider.
    if (!m_SliderPlayingPreempted) {
        int percent = (double)ms / total * m_FrmToolBar.GetSliderPlaying()->maximum();
        m_FrmToolBar.GetSliderPlaying()->setSliderPosition(percent);
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
        m_FrmToolBar.GetBtnPlay()->setIcon(m_IconPlaying);
        break;

    case PlayerStatus::Paused:
        m_TimerUpdateUi->start(m_UpdateInterval);
        m_Player->Resume();
        m_FrmToolBar.GetBtnPlay()->setIcon(m_IconPaused);
        break;

    case PlayerStatus::Stopped:
        m_TimerUpdateUi->start(m_UpdateInterval);
        if (m_UsedMediaItem->hasRange)
            m_Player->Play(m_UsedMediaItem->msBeg, m_UsedMediaItem->msEnd);
        else
            m_Player->Play();
        m_FrmToolBar.GetBtnPlay()->setIcon(m_IconPaused);
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
    m_SliderPlayingPreempted = true;
}

void MainWindow::SlotSliderPlayingReleased()
{
    m_SliderPlayingPreempted = false;
}

void MainWindow::SlotSliderPlayingValueChanged(int val)
{
    if (!m_SliderPlayingPreempted)
        return;

    const double& percent = (double)val / m_FrmToolBar.GetSliderPlaying()->maximum();
    m_Player->SeekPercent(percent);
}

void MainWindow::SlotBarPlayListMidClick(int index)
{
    SimplePlaylistView* view = (SimplePlaylistView*)m_TabWidgetPlaylist->widget(index);
    m_TabWidgetPlaylist->removeTab(index);

    disconnect(view, 0, this, 0);

    delete view;

    m_TabBarPlaylist->setFocus();
}

void MainWindow::SlotWidgetPlayListDoubleClick()
{
    SimplePlaylistView* view = new SimplePlaylistView(this);
    view->SetMediaLoader(m_MediaLoader);

    connect(view, SIGNAL(SigPlayMediaItem(IPlaylistView*, const mous::MediaItem*)),
            this, SLOT(SlotPlayMediaItem(IPlaylistView*, const mous::MediaItem*)));
    connect(view, SIGNAL(SigConvertMediaItem(const mous::MediaItem*)),
            this, SLOT(SlotConvertMediaItem(const mous::MediaItem*)));
    connect(view, SIGNAL(SigConvertMediaItems(QList<const mous::MediaItem*>)),
            this, SLOT(SlotConvertMediaItems(QList<const mous::MediaItem*>)));

    m_TabWidgetPlaylist->addTab(view, QString::number(m_TabWidgetPlaylist->count()));
    m_TabWidgetPlaylist->setCurrentIndex(m_TabWidgetPlaylist->count()-1);
}

void MainWindow::SlotPlayMediaItem(IPlaylistView *view, const MediaItem *item)
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
    m_FrmToolBar.GetBtnPlay()->setIcon(m_IconPaused);

    setWindowTitle(QString::fromUtf8(item->tag.title.c_str()));

    m_UsedPlaylistView = view;

    emit SigLoadFileTag(QString::fromUtf8(item->url.c_str()));
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

    QString fileName =
            QString::fromUtf8((item->tag.artist + " - " + item->tag.title + "." + newTask->GetEncoderFileSuffix()).c_str());
    DlgConvertOption dlgOption(this);
    dlgOption.SetDir(QDir::homePath());
    dlgOption.SetFileName(fileName);
    dlgOption.BindWidgetAndOption(opts);
    dlgOption.setWindowTitle(tr("Config"));
    dlgOption.exec();

    if (dlgOption.result() != QDialog::Accepted) {
        IConvTask::Free(newTask);
        return;
    }

    //==== do work
    m_DlgConvertTask.show();
    m_DlgConvertTask.AddTask(newTask, fileName);
}

void MainWindow::SlotConvertMediaItems(QList<const MediaItem*> items)
{

}
