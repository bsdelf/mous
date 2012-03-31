#include "SimplePlayListView.h"
#include <QtCore>
#include <QtGui>
#include <util/MediaItem.h>
#include <core/IMediaLoader.h>
#include "UiHelper.hpp"
using namespace std;
using namespace sqt;
using namespace mous;

SimplePlayListView::SimplePlayListView(QWidget *parent) :
    QTreeView(parent),
    mMediaLoader(NULL)
{
    setContextMenuPolicy(Qt::ActionsContextMenu);

    QList<QAction*> actionList;
    QAction* action = NULL;
    QActionGroup* group = NULL;
    QMenu* menu = NULL;

    // Action append
    action = new QAction(tr("Append"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(slotAppend()));
    actionList << action;

    // Action remove
    action = new QAction(tr("Remove"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(slotRemove()));
    actionList << action;

    // Action copy
    action = new QAction(tr("Copy"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(slotCopy()));
    actionList << action;

    // Action cut
    action = new QAction(tr("Cut"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(slotCut()));
    actionList << action;

    // Action paste
    action = new QAction(tr("Paste"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(slotPaste()));
    actionList << action;

    actionList << new QAction(this);

    // Action tagging
    action = new QAction(tr("Tagging"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(slotTagging()));
    actionList << action;

    // Action convert
    action = new QAction(tr("Convert"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(slotConvert()));
    actionList << action;

    // Action properties
    action = new QAction(tr("Properties"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(slotProperties()));
    actionList << action;

    actionList << new QAction(this);

    // Action playlist menu
    action = new QAction(tr("Playlist"), this);
    actionList << action;
    menu = new QMenu(this);
    action->setMenu(menu);

    action = new QAction(tr("Load"), this);
    menu->addAction(action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotPlaylistLoad()));

    action = new QAction(tr("Rename"), this);
    menu->addAction(action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotPlaylistRename()));

    action = new QAction(tr("Save As"), this);
    menu->addAction(action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotPlaylistSaveAs()));

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
    setModel(&mModel);
    header()->setResizeMode(QHeaderView::Stretch);

    // Header
    QStringList headList;
    headList << tr("Artist") << tr("Album") << tr("Title") << tr("Track") << tr("Duration");
    mModel.setHorizontalHeaderLabels(headList);

    // Test
    mModel.setRowCount(0);
    for (int row = 0; row < mModel.rowCount(); ++row) {
        for (int column = 0; column < mModel.columnCount(); ++column) {
             QStandardItem *item = new QStandardItem(QString("row %0, column %1").arg(row).arg(column));
             item->setEditable(false);
             item->setSizeHint(QSize(-1, 25));
             mModel.setItem(row, column, item);
         }
     }

    mMediaList.SetMode(PlaylistMode::Repeat);
}

SimplePlayListView::~SimplePlayListView()
{
    QList<QAction*> actionList = actions();
    while (!actionList.isEmpty()) {
        QAction* action = actionList.takeFirst();
        removeAction(action);
        if (action->menu() != NULL)
            delete action->menu();
        delete action;
    }
}

/* IPlayListView interfaces */
void SimplePlayListView::setMediaLoader(const IMediaLoader* loader)
{
    mMediaLoader = loader;
}

const MediaItem* SimplePlayListView::getNextItem()
{
    MediaItem* item = NULL;
    if (mMediaList.SeqCurrent(item, 1)) {
        mMediaList.SeqMoveNext();
        mMediaList.SeqCurrent(item);
    }
    return item;
}

const MediaItem* SimplePlayListView::getPreviousItem()
{
    MediaItem* item = NULL;
    if (mMediaList.SeqCurrent(item, -1)) {
        mMediaList.SeqMoveNext(-1);
        mMediaList.SeqCurrent(item);
    }
    return NULL;
}

size_t SimplePlayListView::getItemCount() const
{
    return mMediaList.GetItemCount();
}

void SimplePlayListView::mouseDoubleClickEvent(QMouseEvent * event)
{
    QTreeView::mouseDoubleClickEvent(event);

    //if (selectedIndexes().size() != 1)
    //    return;

    QModelIndex index(selectedIndexes()[0]);
    qDebug() << index.row();

    mMediaList.SeqJumpTo(index.row());
    MediaItem* item = NULL;
    mMediaList.SeqCurrent(item);

    emit sigPlayMediaItem(this, item);
}

/* Action menus */
void SimplePlayListView::slotAppend()
{
    // Get media path
    QString oldPath("~");
    if (!mOldMediaPath.isEmpty()) {
        QFileInfo info(mOldMediaPath);
        oldPath = info.dir().dirName();
    }
    QStringList pathList = QFileDialog::getOpenFileNames(this,
         tr("Open Media"), oldPath, tr("*"));
    if (pathList.isEmpty())
        return;

    mOldMediaPath = pathList.first();

    deque<MediaItem*> itemList;
    deque<MediaItem*> tmpItemList;
    for (int i = 0; i < pathList.size(); ++i) {
        mMediaLoader->LoadMedia(pathList.at(i).toUtf8().data(), tmpItemList);
        itemList.insert(itemList.end(), tmpItemList.begin(), tmpItemList.end());
    }

    for(size_t i = 0; i < itemList.size(); ++i) {
        MediaItem* item = itemList[i];

        mMediaList.AppendItem(item);

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

        // Build row
        QList<QStandardItem *> row;
        row << new QStandardItem(QString::fromUtf8(item->artist.c_str()));
        row << new QStandardItem(QString::fromUtf8(item->album.c_str()));
        row << new QStandardItem(QString::fromUtf8(item->title.c_str()));
        row << new QStandardItem(QString::number(item->track));
        row << new QStandardItem(strDuration);
        for (int i = 0; i < row.size(); ++i) {
            QStandardItem* item = row[i];
            item->setEditable(false);
            item->setSizeHint(QSize(-1, 22));
        }
        mModel.appendRow(row);
        //qDebug() << QString::fromUtf8(itemList[i]->url.c_str());
    }

    // Update view

}

void SimplePlayListView::slotRemove()
{

}

void SimplePlayListView::slotCopy()
{

}

void SimplePlayListView::slotCut()
{

}

void SimplePlayListView::slotPaste()
{

}

void SimplePlayListView::slotTagging()
{

}

void SimplePlayListView::slotConvert()
{

}

void SimplePlayListView::slotProperties()
{

}

void SimplePlayListView::slotPlaylistLoad()
{

}

void SimplePlayListView::slotPlaylistRename()
{

}

void SimplePlayListView::slotPlaylistSaveAs()
{

}
