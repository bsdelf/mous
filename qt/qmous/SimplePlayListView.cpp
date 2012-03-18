#include "SimplePlayListView.h"
#include <QtCore>
#include <QtGui>
#include <common/MediaItem.h>
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
    return NULL;
}

const MediaItem* SimplePlayListView::getPreviousItem()
{
    return NULL;
}

size_t SimplePlayListView::getItemCount() const
{
    return 0;
}

void SimplePlayListView::mouseDoubleClickEvent(QMouseEvent * event)
{
    QTreeView::mouseDoubleClickEvent(event);

    //if (selectedIndexes().size() != 1)
    //    return;

    QModelIndex index(selectedIndexes()[0]);
    qDebug() << index.row();

    emit sigPlayMediaItem(mMediaItemList[index.row()]);
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
    mOldMediaPath = QFileDialog::getOpenFileName(this,
         tr("Open Media"), oldPath, tr("*"));

    deque<MediaItem*> itemList;
    mMediaLoader->LoadMedia(mOldMediaPath.toUtf8().data(), itemList);

    for(size_t i = 0; i < itemList.size(); ++i) {
        MediaItem* item = itemList[i];

        mMediaItemList.push_back(item);

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
