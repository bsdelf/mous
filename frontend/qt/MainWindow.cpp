#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "AppEnv.h"
#include "DlgListSelect.h"
#include "DlgConvertOption.h"
#include "SimplePlaylistView.h"

#include "MidClickTabBar.hpp"
#include "CustomHeadTabWidget.hpp"
using namespace sqt;

#include <scx/Signal.hpp>
using namespace scx;

#include <util/MediaItem.h>
using namespace mous;

using namespace std;

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

    m_FrmTagEditor.RestoreUiStatus();
    const AppEnv* env = GlobalAppEnv::Instance();
    restoreGeometry(env->windowGeometry);
    restoreState(env->windowState);
}

MainWindow::~MainWindow()
{
    m_FrmTagEditor.WaitForLoadFinished();

    m_Player->SigFinished()->DisconnectReceiver(this);

    if (m_Player->Status() == PlayerStatus::Playing) {
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

void MainWindow::closeEvent(QCloseEvent*)
{
    AppEnv* env = GlobalAppEnv::Instance();
    env->tagEditorSplitterState = m_FrmTagEditor.saveGeometry();
    env->windowGeometry = saveGeometry();
    env->windowState = saveState();
    env->tabCount = m_TabWidgetPlaylist->count();
    env->tabIndex = m_TabWidgetPlaylist->currentIndex();
    env->volume = m_Player->Volume();
    m_FrmTagEditor.SaveUiStatus();

    for (int i = 0 ; i < m_TabWidgetPlaylist->count(); ++i) {
        SimplePlaylistView* view =  qobject_cast<SimplePlaylistView*>(m_TabWidgetPlaylist->widget(i));
        QString filePath = env->configDir + QString("/playlist%1.dat").arg(i);
        view->Save(filePath.toAscii());
    }
}

void MainWindow::InitMousCore()
{
    m_PluginManager = IPluginManager::Create();
    m_MediaLoader = IMediaLoader::Create();
    m_Player = IPlayer::Create();
    m_ConvFactory = IConvTaskFactory::Create();
    m_ParserFactory = ITagParserFactory::Create();

    m_PluginManager->LoadPluginDir(GlobalAppEnv::Instance()->pluginDir.toLocal8Bit().data());
    vector<string> pathList;
    m_PluginManager->DumpPluginPath(pathList);

    vector<const IPluginAgent*> packAgentList;
    vector<const IPluginAgent*> tagAgentList;
    m_PluginManager->DumpPluginAgent(packAgentList, PluginType::MediaPack);
    m_PluginManager->DumpPluginAgent(tagAgentList, PluginType::TagParser);

    m_MediaLoader->RegisterMediaPackPlugin(packAgentList);
    m_MediaLoader->RegisterTagParserPlugin(tagAgentList);

    vector<const IPluginAgent*> decoderAgentList;
    vector<const IPluginAgent*> encoderAgentList;
    vector<const IPluginAgent*> rendererAgentList;
    m_PluginManager->DumpPluginAgent(decoderAgentList, PluginType::Decoder);
    m_PluginManager->DumpPluginAgent(encoderAgentList, PluginType::Encoder);
    m_PluginManager->DumpPluginAgent(rendererAgentList, PluginType::Renderer);

    m_Player->RegisterRendererPlugin(rendererAgentList[0]);
    m_Player->RegisterDecoderPlugin(decoderAgentList);
    m_Player->SigFinished()->Connect(&MainWindow::SlotPlayerFinished, this);

    m_ConvFactory->RegisterDecoderPlugin(decoderAgentList);
    m_ConvFactory->RegisterEncoderPlugin(encoderAgentList);

    m_ParserFactory->RegisterTagParserPlugin(tagAgentList);

    m_FrmTagEditor.SetPlayer(m_Player);
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
    m_FrmTagEditor.SetPlayer(NULL);
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
    AppEnv* env = GlobalAppEnv::Instance();

    // Playing & Paused icon
    m_IconPlaying.addFile(QString::fromUtf8(":/img/resource/play.png"), QSize(), QIcon::Normal, QIcon::On);
    m_IconPaused.addFile(QString::fromUtf8(":/img/resource/pause.png"), QSize(), QIcon::Normal, QIcon::On);

    // Volume
    if (env->volume < 0) {
        env->volume = m_Player->Volume();
    } else {
        m_Player->SetVolume(env->volume);
    }
    m_FrmToolBar.SliderVolume()->setValue(env->volume);

    // PlayList View
    m_TabBarPlaylist = new MidClickTabBar(this);
    m_TabWidgetPlaylist = new CustomHeadTabWidget(this);
    m_TabWidgetPlaylist->SetTabBar(m_TabBarPlaylist);
    m_TabWidgetPlaylist->setMovable(true);
    ui->layoutPlaylist->addWidget(m_TabWidgetPlaylist);

    // Status bar buttons
    m_BtnPreference = new QToolButton(ui->statusBar);
    m_BtnPreference->setAutoRaise(true);
    m_BtnPreference->setText(QChar((int)0x263A));
    m_BtnPreference->setToolTip(tr("Preference"));

    ui->statusBar->addPermanentWidget(m_BtnPreference, 0);

    // Recover previous playlist
    for (int i = 0; i < env->tabCount; ++i) {
        SlotWidgetPlayListDoubleClick();
        SimplePlaylistView* view =  qobject_cast<SimplePlaylistView*>(m_TabWidgetPlaylist->widget(i));
        QString filePath = env->configDir + QString("/playlist%1.dat").arg(i);
        if (QFileInfo(filePath).isFile())
            view->Load(filePath.toAscii());
        else
            qDebug() << filePath << "not exist!";
    }
    m_TabWidgetPlaylist->setCurrentIndex(env->tabIndex);

    // Show left-side Dock
    m_Dock = new QDockWidget(tr("Metadata"));
    m_Dock->setObjectName("Dock");
    m_Dock->setWidget(&m_FrmTagEditor);
    addDockWidget(Qt::LeftDockWidgetArea, m_Dock);
    m_Dock->setFeatures(QDockWidget::NoDockWidgetFeatures | QDockWidget::DockWidgetMovable);

    // Show top slider
    ui->toolBar->addWidget(&m_FrmToolBar);
    ui->toolBar->setMovable(false);
    setContextMenuPolicy(Qt::NoContextMenu);
}

void MainWindow::InitQtSlots()
{
    connect(m_TimerUpdateUi, SIGNAL(timeout()), this, SLOT(SlotUpdateUi()));

    connect(m_FrmToolBar.BtnPlay(), SIGNAL(clicked()), this, SLOT(SlotBtnPlay()));
    connect(m_FrmToolBar.BtnPrev(), SIGNAL(clicked()), this, SLOT(SlotBtnPrev()));
    connect(m_FrmToolBar.BtnNext(), SIGNAL(clicked()), this, SLOT(SlotBtnNext()));

    connect(m_FrmToolBar.SliderVolume(), SIGNAL(valueChanged(int)), this, SLOT(SlotSliderVolumeValueChanged(int)));

    connect(m_FrmToolBar.SliderPlaying(), SIGNAL(sliderPressed()), this, SLOT(SlotSliderPlayingPressed()));
    connect(m_FrmToolBar.SliderPlaying(), SIGNAL(sliderReleased()), this, SLOT(SlotSliderPlayingReleased()));
    connect(m_FrmToolBar.SliderPlaying(), SIGNAL(valueChanged(int)), this, SLOT(SlotSliderPlayingValueChanged(int)));

    connect(m_TabBarPlaylist, SIGNAL(SigMidClick(int)), this, SLOT(SlotBarPlayListMidClick(int)));
    connect(m_TabWidgetPlaylist, SIGNAL(SigDoubleClick()), this, SLOT(SlotWidgetPlayListDoubleClick()));

    connect(&m_FrmTagEditor, SIGNAL(SigMediaItemChanged(const MediaItem&)), this, SLOT(SlotTagUpdated(const MediaItem&)));
}

/* MousCore slots */
void MainWindow::SlotPlayerFinished()
{
    QMetaObject::invokeMethod(this, "SlotUiPlayerFinished", Qt::QueuedConnection);
}

void MainWindow::SlotUiPlayerFinished()
{
    qDebug() << "SlotUiPlayerFinished()";

    if (m_UsedPlaylistView != NULL) {
        const MediaItem* item = m_UsedPlaylistView->NextItem();
        if (item != NULL) {
            SlotPlayMediaItem(m_UsedPlaylistView, *item);
        } else {

        }
    }
}

/* Qt slots */
void MainWindow::SlotUpdateUi()
{
    int total = -1;
    int ms = -1;
    int hz = -1;
    int kbps = -1;

    //==== Update statusbar.
    if (m_Player->Status() != PlayerStatus::Closed) {
        total = m_Player->RangeDuration();
        ms = m_Player->OffsetMs();
        hz = m_Player->SamleRate();
        kbps = m_Player->BitRate();
    } else {
        ui->statusBar->showMessage("");
        m_FrmToolBar.SliderPlaying()->setSliderPosition(0);
        return;
    }

    const QString& status = QString("%1 Hz | %2 Kbps | %3:%4/%5:%6").arg(hz).arg(kbps, 4).
            arg(ms/1000/60, 2, 10, QChar('0')).arg(ms/1000%60, 2, 10, QChar('0')).
            arg(total/1000/60, 2, 10, QChar('0')).arg(total/1000%60, 2, 10, QChar('0'));

    ui->statusBar->showMessage(status);

    //==== Update slider.
    if (!m_SliderPlayingPreempted) {
        int val = (double)ms / total * m_FrmToolBar.SliderPlaying()->maximum();
        m_FrmToolBar.SliderPlaying()->setSliderPosition(val);
    }
}

void MainWindow::SlotBtnPlay()
{
    qDebug() << m_Player->Status();

    switch (m_Player->Status()) {
    case PlayerStatus::Closed:
        if (m_UsedMediaItem != NULL) {
            if (m_Player->Open(m_UsedMediaItem->url) == ErrorCode::Ok)
                SlotBtnPlay();
        }
        break;

    case PlayerStatus::Playing:
        m_Player->Pause();
        m_TimerUpdateUi->stop();
        m_FrmToolBar.BtnPlay()->setIcon(m_IconPlaying);
        break;

    case PlayerStatus::Paused:
        m_TimerUpdateUi->start(m_UpdateInterval);
        m_Player->Resume();
        m_FrmToolBar.BtnPlay()->setIcon(m_IconPaused);
        break;

    case PlayerStatus::Stopped:
        m_TimerUpdateUi->start(m_UpdateInterval);
        if (m_UsedMediaItem->hasRange)
            m_Player->Play(m_UsedMediaItem->msBeg, m_UsedMediaItem->msEnd);
        else
            m_Player->Play();
        m_FrmToolBar.BtnPlay()->setIcon(m_IconPaused);
        break;
    }
}

void MainWindow::SlotBtnPrev()
{
    if (m_UsedPlaylistView == NULL)
        return;

    const mous::MediaItem* item = m_UsedPlaylistView->PrevItem();
    if (item != NULL)
        SlotPlayMediaItem(m_UsedPlaylistView, *item);
}

void MainWindow::SlotBtnNext()
{
    if (m_UsedPlaylistView == NULL)
        return;

    const mous::MediaItem* item = m_UsedPlaylistView->NextItem();
    if (item != NULL)
        SlotPlayMediaItem(m_UsedPlaylistView, *item);
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

    const double& percent = (double)val / m_FrmToolBar.SliderPlaying()->maximum();
    m_Player->SeekPercent(percent);
}

void MainWindow::SlotBarPlayListMidClick(int index)
{
    if (m_TabWidgetPlaylist->count() <= 1)
        return;

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
    view->SetClipboard(&m_Clipboard);

    connect(view, SIGNAL(SigPlayMediaItem(IPlaylistView*, const MediaItem&)),
            this, SLOT(SlotPlayMediaItem(IPlaylistView*, const MediaItem&)));
    connect(view, SIGNAL(SigConvertMediaItem(const MediaItem&)),
            this, SLOT(SlotConvertMediaItem(const MediaItem&)));
    connect(view, SIGNAL(SigConvertMediaItems(const QList<MediaItem>&)),
            this, SLOT(SlotConvertMediaItems(const QList<MediaItem>&)));

    m_TabWidgetPlaylist->addTab(view, tr("List") + " " + QString::number(m_TabWidgetPlaylist->count()));
    m_TabWidgetPlaylist->setCurrentIndex(m_TabWidgetPlaylist->count()-1);
}

void MainWindow::SlotPlayMediaItem(IPlaylistView *view, const MediaItem& item)
{
    if (m_Player->Status() == PlayerStatus::Playing) {
        m_Player->Close();
    }
    if (m_Player->Status() != PlayerStatus::Closed) {
        m_Player->Close();
        m_TimerUpdateUi->stop();
    }

    m_UsedMediaItem = &item;

    if (m_Player->Open(item.url) != ErrorCode::Ok) {
        setWindowTitle("Mous ( " + tr("Failed to open!") + " )");
        usleep(100*1000);
        SlotBtnNext();
        return;
    }

    m_TimerUpdateUi->start(m_UpdateInterval);
    if (item.hasRange)
        m_Player->Play(item.msBeg, item.msEnd);
    else
        m_Player->Play();
    m_FrmToolBar.BtnPlay()->setIcon(m_IconPaused);

    setWindowTitle("Mous ( " + QString::fromUtf8(item.tag.title.c_str()) + " )");

    m_UsedPlaylistView = view;

    m_FrmTagEditor.LoadMediaItem(item);
}

void MainWindow::SlotConvertMediaItem(const MediaItem& item)
{
    //==== show encoders
    vector<string> encoderNames = m_ConvFactory->EncoderNames();
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
    newTask->EncoderOptions(opts);

    QString fileName =
            QString::fromUtf8((item.tag.artist + " - " + item.tag.title + "." + newTask->EncoderFileSuffix()).c_str());
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
    QString filePath = QFileInfo(dlgOption.Dir(), dlgOption.FileName()).absoluteFilePath();
    m_DlgConvertTask.show();
    m_DlgConvertTask.AddTask(newTask, filePath);
}

void MainWindow::SlotConvertMediaItems(const QList<MediaItem>& items)
{

}

void MainWindow::SlotTagUpdated(const MediaItem& item)
{
    if (m_UsedPlaylistView != NULL)
        m_UsedPlaylistView->OnMediaItemUpdated(item);
}
