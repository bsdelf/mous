#include "SimplePlaylistView.h"
#include <QtCore>
#include <QtGui>

#include <util/MediaItem.h>
#include <core/IMediaLoader.h>
using namespace mous;

#include <scx/Thread.hpp>
#include <scx/IconvHelper.hpp>
#include <scx/CharsetHelper.hpp>
using namespace scx;
using namespace std;

#include "UiHelper.hpp"
using namespace sqt;

SimplePlaylistView::SimplePlaylistView(QWidget *parent) :
    QTreeView(parent),
    m_MediaLoader(NULL)
{
    setContextMenuPolicy(Qt::ActionsContextMenu);

    QList<QAction*> actionList;
    QAction* action = NULL;
    QActionGroup* group = NULL;
    QMenu* menu = NULL;

    // Action append
    action = new QAction(tr("Append"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(SlotAppend()));
    actionList << action;

    // Action remove
    action = new QAction(tr("Remove"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(SlotRemove()));
    actionList << action;

    // Action copy
    action = new QAction(tr("Copy"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(SlotCopy()));
    actionList << action;

    // Action cut
    action = new QAction(tr("Cut"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(SlotCut()));
    actionList << action;

    // Action paste
    action = new QAction(tr("Paste"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(SlotPaste()));
    actionList << action;

    actionList << new QAction(this);

    // Action tagging
    action = new QAction(tr("Tagging"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(SlotTagging()));
    actionList << action;

    // Action convert
    action = new QAction(tr("Convert"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(SlotConvert()));
    actionList << action;

    // Action properties
    action = new QAction(tr("Properties"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(SlotProperties()));
    actionList << action;

    actionList << new QAction(this);

    // Action playlist menu
    action = new QAction(tr("Playlist"), this);
    actionList << action;
    menu = new QMenu(this);
    action->setMenu(menu);

    action = new QAction(tr("Load"), this);
    menu->addAction(action);
    connect(action, SIGNAL(triggered()), this, SLOT(SlotPlaylistLoad()));

    action = new QAction(tr("Rename"), this);
    menu->addAction(action);
    connect(action, SIGNAL(triggered()), this, SLOT(SlotPlaylistRename()));

    action = new QAction(tr("Save As"), this);
    menu->addAction(action);
    connect(action, SIGNAL(triggered()), this, SLOT(SlotPlaylistSaveAs()));

    // Action play mode menu
    action = new QAction(tr("Play Mode"), this);
    actionList << action;
    menu = new QMenu(this);
    action->setMenu(menu);
    group = new QActionGroup(this);

    action = new QAction(tr("Normal"), this);
    action->setCheckable(true);
    group->addAction(action);
    action = new QAction(tr("Repeat"), this);
    action->setCheckable(true);
    group->addAction(action);
    action->setChecked(true);
    action = new QAction(tr("Shuffle"), this);
    action->setCheckable(true);
    group->addAction(action);
    action = new QAction(tr("Shuffle Repeat"), this);
    action->setCheckable(true);
    group->addAction(action);
    action = new QAction(tr("Repeat One"), this);
    action->setCheckable(true);
    group->addAction(action);
    menu->addActions(group->actions());
    //group->setExclusive(true);

    // Style
    setActionSeparator(actionList);
    addActions(actionList);

    setAcceptDrops(true);
    setDragEnabled(true);

    setRootIsDecorated(false);
    setItemsExpandable(false);
    setAlternatingRowColors(true);
    setUniformRowHeights(true);

    setModel(&m_StModel);
    header()->setResizeMode(QHeaderView::Stretch);

    // Header
    QStringList headList;
    headList << tr("Artist") << tr("Album") << tr("Title") << tr("Track") << tr("Duration");
    m_StModel.setHorizontalHeaderLabels(headList);
    m_StModel.setColumnCount(headList.size());

    // Test
    /*
    m_StModel.setRowCount(0);
    for (int row = 0; row < m_StModel.rowCount(); ++row) {
        for (int column = 0; column < m_StModel.columnCount(); ++column) {
             QStandardItem *item = new QStandardItem(QString("row %0, column %1").arg(row).arg(column));
             item->setEditable(false);
             item->setSizeHint(QSize(-1, 25));
             m_StModel.setItem(row, column, item);
         }
    }
    */

    m_Playlist.SetMode(PlaylistMode::Repeat);

    // connect
    connect(this, SIGNAL(SigReadyToLoad()), this, SLOT(SlotReadyToLoad()), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(SigLoadFinished()), this, SLOT(SlotLoadFinished()), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(SigMediaRowGot(MediaRow*)), this, SLOT(SlotMediaRowGot(MediaRow*)), Qt::BlockingQueuedConnection);

    //connect(&m_PickMediaItemTimer, SIGNAL(timeout()), this, SLOT(SlotPickMediaItem()));
}

SimplePlaylistView::~SimplePlaylistView()
{
    QList<QAction*> actionList = actions();
    while (!actionList.isEmpty()) {
        QAction* action = actionList.takeFirst();
        removeAction(action);
        if (action->menu() != NULL)
            delete action->menu();
        delete action;
    }

    while (m_StModel.rowCount() > 0) {
        QList<QStandardItem*> list = m_StModel.takeRow(0);
        foreach(QStandardItem* item, list)
            delete item;
    }

    for (int i = 0; i < m_Playlist.Count(); ++i) {
        delete m_Playlist[i];
    }
    m_Playlist.Clear();

    //m_LoadThread.jooin();
}

/* IPlayListView interfaces */
void SimplePlaylistView::SetMediaLoader(const IMediaLoader* loader)
{
    m_MediaLoader = loader;
}

const MediaItem* SimplePlaylistView::GetNextItem() const
{
    return m_Playlist.SeqHasOffset(1) ? m_Playlist.SeqItemAtOffset(1, true) : NULL;

}

const MediaItem* SimplePlaylistView::GetPreviousItem() const
{
    return m_Playlist.SeqHasOffset(-1) ? m_Playlist.SeqItemAtOffset(-1, true) : NULL;
}

size_t SimplePlaylistView::GetItemCount() const
{
    return m_Playlist.Count();
}

/* Override qt methods */
void SimplePlaylistView::dragEnterEvent(QDragEnterEvent *event)
{
    qDebug() << "+drag" << event->mimeData()->text();
    event->acceptProposedAction();
}

void SimplePlaylistView::dragMoveEvent(QDragMoveEvent *event)
{
    qDebug() << "~drag" << event->mimeData()->text();
}

void SimplePlaylistView::dragLeaveEvent(QDragLeaveEvent *event)
{
    qDebug() << "-drag";
}

void SimplePlaylistView::dropEvent(QDropEvent *event)
{
    qDebug() << "!drop" << event->mimeData()->text();
    event->acceptProposedAction();
}

void SimplePlaylistView::mouseDoubleClickEvent(QMouseEvent * event)
{
    QTreeView::mouseDoubleClickEvent(event);

    if (m_Playlist.Empty())
        return;

    QModelIndex index(selectedIndexes()[0]);
    qDebug() << index.row();

    m_Playlist.SeqJumpTo(index.row());
    MediaItem* item = m_Playlist.SeqItemAtOffset(0, false);

    emit SigPlayMediaItem(this, item);
}

/* Action menus */
void SimplePlaylistView::SlotAppend()
{
    // Get media path
    QString oldPath("~");
    if (!m_OldMediaPath.isEmpty()) {
        QFileInfo info(m_OldMediaPath);
        oldPath = info.dir().dirName();
    }
    QStringList pathList = QFileDialog::getOpenFileNames(
                this, tr("Open Media"), oldPath, tr("*"));
    if (pathList.isEmpty())
        return;

    m_OldMediaPath = pathList.first();
    scx::Function<void (const QStringList&)> fn(&SimplePlaylistView::LoadMediaItem, this);
    m_LoadMediaThread.Run(fn, pathList);
}

void SimplePlaylistView::SlotRemove()
{

}

void SimplePlaylistView::SlotCopy()
{

}

void SimplePlaylistView::SlotCut()
{

}

void SimplePlaylistView::SlotPaste()
{

}

void SimplePlaylistView::SlotTagging()
{

}

void SimplePlaylistView::SlotConvert()
{
    if (m_Playlist.Empty())
        return;

    QModelIndex index(selectedIndexes()[0]);
    qDebug() << index.row();

    MediaItem* item = m_Playlist[index.row()];

    emit SigConvertMediaItem(item);
}

void SimplePlaylistView::SlotProperties()
{

}

void SimplePlaylistView::SlotPlaylistLoad()
{

}

void SimplePlaylistView::SlotPlaylistRename()
{

}

void SimplePlaylistView::SlotPlaylistSaveAs()
{

}

void SimplePlaylistView::SlotReadyToLoad()
{
    setUpdatesEnabled(false);

    m_DlgLoadingMedia.setWindowTitle(tr("Loading"));
    m_DlgLoadingMedia.show();
    //m_PickMediaItemTimer.start(1);
}


void SimplePlaylistView::SlotLoadFinished()
{
    setUpdatesEnabled(true);
    m_DlgLoadingMedia.hide();
}

void SimplePlaylistView::SlotMediaRowGot(MediaRow* mediaRow)
{
    /*
    if (m_TmpLoadList.isEmpty()) {
        m_DlgLoadingMedia.SetFileName("");
        if (m_LoadFinished) {
            m_PickMediaItemTimer.stop();
            m_DlgLoadingMedia.hide();
            setUpdatesEnabled(true);
        }
        return;
    }
    m_TmpLoadMutex.lock();
    MediaRow mediaRow = m_TmpLoadList.takeFirst();
    m_TmpLoadMutex.unlock();
    */

    QFileInfo info(QString::fromUtf8(mediaRow->item->url.c_str()));
    QString fileName(info.fileName());

    m_DlgLoadingMedia.SetFileName(fileName);

    m_Playlist.Append(mediaRow->item);
    m_StModel.appendRow(mediaRow->row);
}

void SimplePlaylistView::LoadMediaItem(const QStringList& pathList)
{
    m_LoadFinished = false;
    m_TmpLoadList.clear();

    emit SigReadyToLoad();

    std::deque<mous::MediaItem*> mediaItemList;

    for (int i = 0; i < pathList.size(); ++i) {
        m_MediaLoader->LoadMedia(pathList.at(i).toUtf8().data(), mediaItemList);

        for(size_t j = 0; j < mediaItemList.size(); ++j) {
            MediaItem* item = mediaItemList[j];

            MediaRow mediaRow;
            mediaRow.item = item;

            // Check sec duration
            int secDuration = 0;
            if (item->hasRange) {
                if (item->msEnd != (uint64_t)-1)
                    secDuration = (item->msEnd - item->msBeg)/1000;
                else
                    secDuration = (item->duration - item->msBeg)/1000;
            } else {
                secDuration = item->duration/1000;
            }
            QString strDuration;
            strDuration.sprintf("%.2d:%.2d", secDuration/60, secDuration%60);

            string tmp;
            if (!CharsetHelper::IsUtf8(item->tag.artist.c_str())
                    && IconvHelper::ConvFromTo("GBK", "UTF-8", item->tag.artist.data(), item->tag.artist.size(), tmp))
                item->tag.artist = tmp;
            else
                qDebug() << "no touch:" << QString::fromUtf8(item->tag.artist.c_str());

            if (!CharsetHelper::IsUtf8(item->tag.album.c_str())
                    && IconvHelper::ConvFromTo("GBK", "UTF-8", item->tag.album.data(), item->tag.album.size(), tmp))
                item->tag.album = tmp;
            else
                qDebug() << "no touch:" << QString::fromUtf8(item->tag.album.c_str());

            if (!CharsetHelper::IsUtf8(item->tag.title.c_str())
                    && IconvHelper::ConvFromTo("GBK", "UTF-8", item->tag.title.data(), item->tag.title.size(), tmp))
                item->tag.title = tmp;
            else
                qDebug() << "no touch:" << QString::fromUtf8(item->tag.title.c_str());

            // Build row
            mediaRow.row << new QStandardItem(QString::fromUtf8(item->tag.artist.c_str()));
            mediaRow.row << new QStandardItem(QString::fromUtf8(item->tag.album.c_str()));
            mediaRow.row << new QStandardItem(QString::fromUtf8(item->tag.title.c_str()));
            mediaRow.row << new QStandardItem(QString::number(item->tag.track));
            mediaRow.row << new QStandardItem(strDuration);
            for (int i = 0; i < mediaRow.row.size(); ++i) {
                QStandardItem* item = mediaRow.row[i];
                item->setEditable(false);
                item->setSizeHint(QSize(-1, 22));
            }

            emit SigMediaRowGot(&mediaRow);
            /*
            m_TmpLoadMutex.lock();
            m_TmpLoadList.append(mediaRow);
            m_TmpLoadMutex.unlock();
            */
        }
    }

    emit SigLoadFinished();
    m_LoadFinished = true;
}
