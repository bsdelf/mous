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

const QString ITEM_MIME = "index:";
const QString FILE_MIME = "file://";

typedef PlaylistActionHistory<MediaItem> ActionHistory;

SimplePlaylistView::SimplePlaylistView(QWidget *parent) :
    QTreeView(parent),
    m_MediaLoader(NULL),
    m_ShortcutCopy(qobject_cast<QWidget*>(this)),
    m_ShortcutCut(qobject_cast<QWidget*>(this)),
    m_ShortcutPaste(qobject_cast<QWidget*>(this)),
    m_ShortcutDelete(qobject_cast<QWidget*>(this)),
    m_ShortcutUndo(qobject_cast<QWidget*>(this)),
    m_ShortcutRedo(qobject_cast<QWidget*>(this))
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

    actionList << new QAction(this);

    // Action remove
    action = new QAction(tr("Remove"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(SlotShortcutDelete()));
    actionList << action;

    // Action copy
    action = new QAction(tr("Copy"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(SlotShortcutCopy()));
    actionList << action;

    // Action cut
    action = new QAction(tr("Cut"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(SlotShortcutCut()));
    actionList << action;

    // Action paste
    action = new QAction(tr("Paste"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(SlotShortcutPaste()));
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
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::DragDrop);

    setRootIsDecorated(false);
    setItemsExpandable(false);
    setAlternatingRowColors(true);
    setUniformRowHeights(true);

    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);

    setModel(&m_ItemModel);
    header()->setResizeMode(QHeaderView::Stretch);

    // Header
    QStringList headList;
    headList << tr("Artist") << tr("Album") << tr("Title") << tr("Track") << tr("Duration");
    m_ItemModel.setHorizontalHeaderLabels(headList);
    m_ItemModel.setColumnCount(headList.size());

    // Test
    /*
    m_ItemModel.setRowCount(0);
    for (int row = 0; row < m_ItemModel.rowCount(); ++row) {
        for (int column = 0; column < m_ItemModel.columnCount(); ++column) {
             QStandardItem *item = new QStandardItem(QString("row %0, column %1").arg(row).arg(column));
             item.setEditable(false);
             item.setSizeHint(QSize(-1, 25));
             m_ItemModel.setItem(row, column, item);
         }
    }
    */

    m_Playlist.SetMode(PlaylistMode::Repeat);

    // connect
    connect(this, SIGNAL(SigReadyToLoad()),
            this, SLOT(SlotReadyToLoad()), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(SigLoadFinished()),
            this, SLOT(SlotLoadFinished()), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(SigListRowGot(const ListRow&)),
            this, SLOT(SlotListRowGot(const ListRow&)), Qt::BlockingQueuedConnection);

    //connect(&m_ScrollTimer, SIGNAL(timeout()), this, SLOT(SlotCheckForScroll));
    SetupShortcuts();
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

    while (m_ItemModel.rowCount() > 0) {
        QList<QStandardItem*> list = m_ItemModel.takeRow(0);
        foreach(QStandardItem* item, list)
            delete item;
    }

    m_Playlist.Clear();

}

/* IPlayListView interfaces */
void SimplePlaylistView::SetMediaLoader(const IMediaLoader* loader)
{
    m_MediaLoader = loader;
}

const MediaItem* SimplePlaylistView::NextItem() const
{
    return m_Playlist.SeqHasOffset(1) ? &m_Playlist.SeqItemAtOffset(1, true) : NULL;

}

const MediaItem* SimplePlaylistView::PrevItem() const
{
    return m_Playlist.SeqHasOffset(-1) ? &m_Playlist.SeqItemAtOffset(-1, true) : NULL;
}

int SimplePlaylistView::ItemCount() const
{
    return m_Playlist.Count();
}

void SimplePlaylistView::SetupShortcuts()
{
    m_ShortcutCopy.setKey(QKeySequence::Copy);
    m_ShortcutCut.setKey(QKeySequence::Cut);
    m_ShortcutPaste.setKey(QKeySequence::Paste);
    m_ShortcutDelete.setKey(QKeySequence::Delete);
    m_ShortcutUndo.setKey(QKeySequence::Undo);
    m_ShortcutRedo.setKey(QKeySequence::Redo);

    connect(&m_ShortcutCopy, SIGNAL(activated()), this, SLOT(SlotShortcutCopy()));
    connect(&m_ShortcutCut, SIGNAL(activated()), this, SLOT(SlotShortcutCut()));
    connect(&m_ShortcutPaste, SIGNAL(activated()), this, SLOT(SlotShortcutPaste()));
    connect(&m_ShortcutDelete, SIGNAL(activated()), this, SLOT(SlotShortcutDelete()));
    connect(&m_ShortcutUndo, SIGNAL(activated()), this, SLOT(SlotShortcutUndo()));
    connect(&m_ShortcutRedo, SIGNAL(activated()), this, SLOT(SlotShortcutRedo()));
}

/* Override qt methods */
void SimplePlaylistView::mousePressEvent(QMouseEvent *event)
{
    QTreeView::mousePressEvent(event);

    if (event->button() == Qt::LeftButton && !selectedIndexes().empty())
        m_DragStartPos = event->pos();
}

void SimplePlaylistView::mouseMoveEvent(QMouseEvent *event)
{
    // Prepare for drag
    if (!(event->buttons() & Qt::LeftButton))
        return;
    if (m_DragStartPos.isNull() ||
            (event->pos() - m_DragStartPos).manhattanLength() < QApplication::startDragDistance())
        return;

    // Pick selected indexes
    QModelIndexList list = selectedIndexes();
    if (list.empty() || !list.contains(indexAt(event->pos())))
        return;
    QSet<int> indexes;
    QString strIndexes = ITEM_MIME;
    for (int i = 0; i < list.size(); ++i) {
        int row = list[i].row();
        if (!indexes.contains(row)) {
            indexes.insert(row);
            strIndexes.append(QString::number(row)).append(":");
        }
    }

    QDrag* drag = new QDrag(this);
    QMimeData* mimeData = new QMimeData;
    mimeData->setText(strIndexes);
    drag->setMimeData(mimeData);

    Qt::DropAction dropAction = drag->exec(Qt::MoveAction);

    if (!m_DragStartPos.isNull()) {
        m_DragStartPos.setX(0);
        m_DragStartPos.setY(0);
    }

    qDebug() << "done";
}

void SimplePlaylistView::mouseDoubleClickEvent(QMouseEvent * event)
{
    QTreeView::mouseDoubleClickEvent(event);

    if (m_Playlist.Empty())
        return;

    QModelIndex index(selectedIndexes()[0]);
    qDebug() << index.row();

    m_Playlist.SeqJumpTo(index.row());
    const MediaItem& item = m_Playlist.SeqItemAtOffset(0, false);

    emit SigPlayMediaItem(this, item);
}

void SimplePlaylistView::dragEnterEvent(QDragEnterEvent *event)
{
    qDebug() << "+drag" << event->mimeData()->text();
    event->acceptProposedAction();
}

void SimplePlaylistView::dragMoveEvent(QDragMoveEvent *event)
{
    QString head = "file://";
    QString text = event->mimeData()->text();
    qDebug() << "~drag" << event->mimeData()->text();

    event->accept();

    QRect contentRect = viewport()->contentsRect();
    if (event->pos().y() >= contentRect.bottom() - 20) {
        //m_ScrollTimer.is
        //m_ScrollTimer.start(100);
    } else if (event->pos().y() <= contentRect.top() + 20) {
        //m_ScrollTimer.start(100);
    }

    QModelIndex indexUnderCursor = indexAt(event->pos());
    if (indexUnderCursor.isValid()) {
        QModelIndex nextIndex = indexBelow(indexUnderCursor);
        //if (nextIndex.isValid() && !visualRect(nextIndex).isValid()) {
            scrollTo(nextIndex);
        //}
    }
}

void SimplePlaylistView::dragLeaveEvent(QDragLeaveEvent *event)
{
    qDebug() << "-drag";

    // Clear for drag
    if (!m_DragStartPos.isNull()) {
        m_DragStartPos.setX(0);
        m_DragStartPos.setY(0);
    }
}

void SimplePlaylistView::dropEvent(QDropEvent *event)
{
    const QString& text = event->mimeData()->text();

    if (text.startsWith(ITEM_MIME)) {
        qDebug() << "!drop:movement";

        QStringList strIndexes = text.split(":", QString::SkipEmptyParts);
        if (strIndexes.size() <= 1)
            return;
        QList<int> indexes;
        for (int i = 1; i < strIndexes.size(); ++i) {
            indexes << strIndexes[i].toInt();
        }
    } else if (text.startsWith(FILE_MIME)) {
        qDebug() << "!drop:append file";

        QStringList files = text.split(FILE_MIME, QString::SkipEmptyParts);
        for (int i = 0; i < files.size(); ++i) {
            files[i] = files[i].trimmed();
        }

        scx::Function<void (const QStringList&)> fn(&SimplePlaylistView::LoadMediaItem, this);
        m_LoadMediaThread.Run(fn, files);
        m_LoadMediaThread.Detach();
    } else {
        return;
    }
    // Clear for drag
    if (!m_DragStartPos.isNull()) {
        m_DragStartPos.setX(0);
        m_DragStartPos.setY(0);
    }
}

/* Action menus */
void SimplePlaylistView::SlotAppend()
{
    // Pick media files
    QString oldPath("~");
    if (!m_PrevMediaFilePath.isEmpty()) {
        QFileInfo info(m_PrevMediaFilePath);
        oldPath = info.dir().dirName();
    }
    QStringList pathList = QFileDialog::getOpenFileNames(
                this, tr("Open Media"), oldPath, tr("*"));
    if (pathList.isEmpty())
        return;

    m_PrevMediaFilePath = pathList.first();

    // Async load
    scx::Function<void (const QStringList&)> fn(&SimplePlaylistView::LoadMediaItem, this);
    m_LoadMediaThread.Run(fn, pathList);
    m_LoadMediaThread.Detach();
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

    MediaItem item = m_Playlist[index.row()];

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
}


void SimplePlaylistView::SlotLoadFinished()
{
    setUpdatesEnabled(true);
    m_DlgLoadingMedia.hide();
}

void SimplePlaylistView::SlotListRowGot(const ListRow& listRow)
{
    QFileInfo info(QString::fromUtf8(listRow.item.url.c_str()));
    QString fileName(info.fileName());

    m_DlgLoadingMedia.SetFileName(fileName);

    m_Playlist.Append(listRow.item);
    m_ItemModel.appendRow(listRow.fields);
}

void SimplePlaylistView::SlotShortcutCopy()
{
    qDebug() << "copy";
}

void SimplePlaylistView::SlotShortcutCut()
{
    qDebug() << "cut";
}

void SimplePlaylistView::SlotShortcutPaste()
{
    qDebug() << "paste";
}

void SimplePlaylistView::SlotShortcutDelete()
{
    qDebug() << "delete";
}

void SimplePlaylistView::SlotShortcutUndo()
{
    qDebug() << "undo";

    if (m_History.HasUndoAction()) {
        ActionHistory::Action action = m_History.PopUndoAction();
        switch (action.type) {
        case ActionHistory::Insert:
        {
            // append?
            if (action.insertPos == -1) {
                assert(!m_Playlist.Empty() && m_ItemModel.rowCount() != 0);
                int n = action.srcItemList.size();
                int begin = m_Playlist.Count() - n;
                vector<int> indexes(n);
                for (int i = 0; i < n; ++i) {
                    indexes[i] = begin + i;
                }
                m_Playlist.Remove(indexes);
                m_ItemModel.removeRows(begin, n);
            } else {

            }
        }
            break;

        case ActionHistory::Remove:
        {

        }
            break;

        case ActionHistory::Move:
        {
        }
            break;

        default:
            break;
        }
    }
}

void SimplePlaylistView::SlotShortcutRedo()
{
    qDebug() << "redo";

    if (m_History.HasRedoAction()) {
        ActionHistory::Action action = m_History.TakeRedoAction();
        switch (action.type) {
        case ActionHistory::Insert:
        {
            // append?
            if (action.insertPos == -1) {
                SlotReadyToLoad();
                for (size_t i = 0; i < action.srcItemList.size(); ++i) {
                    ListRow listRow = BuildListRow(action.srcItemList[i].second);
                    SlotListRowGot(listRow);
                }
                SlotLoadFinished();
            } else {

            }
        }
            break;

        case ActionHistory::Remove:
            break;

        case ActionHistory::Move:
            break;

        default:
            break;
        }
    }
}

void SimplePlaylistView::LoadMediaItem(const QStringList& pathList)
{
    if (pathList.empty())
        return;

    emit SigReadyToLoad();

    // Prepare for history
    ActionHistory::Action action;
    action.type = ActionHistory::Insert;
    action.insertPos = -1;

    for (int i = 0; i < pathList.size(); ++i) {
        if (pathList.at(i).isEmpty())
            continue;

        // Although load ok,
        // the item may still invaild(player won't be able to play it)
        deque<MediaItem> mediaItemList;
        const char* filePath = pathList.at(i).toUtf8().data();
        if (m_MediaLoader->LoadMedia(filePath, mediaItemList) != ErrorCode::Ok)
            continue;

        for(size_t j = 0; j < mediaItemList.size(); ++j) {
            MediaItem& item = mediaItemList[j];
            ListRow listRow = BuildListRow(item);
            action.srcItemList.push_back(std::pair<int, MediaItem>(-1, item));
            emit SigListRowGot(listRow);
        }
    }

    // Record operation
    if (!action.srcItemList.empty())
        m_History.PushUndoAction(action);

    emit SigLoadFinished();
}

SimplePlaylistView::ListRow SimplePlaylistView::BuildListRow(MediaItem& item) const
{
    ListRow listRow;
    listRow.item = item;

    // Check sec duration
    int secDuration = 0;
    if (item.hasRange) {
        if (item.msEnd != (uint64_t)-1)
            secDuration = (item.msEnd - item.msBeg)/1000;
        else
            secDuration = (item.duration - item.msBeg)/1000;
    } else {
        secDuration = item.duration/1000;
    }
    QString strDuration;
    strDuration.sprintf("%.2d:%.2d", secDuration/60, secDuration%60);

    string tmp;
    if (!CharsetHelper::IsUtf8(item.tag.artist.c_str())
            && IconvHelper::ConvFromTo("GBK", "UTF-8", item.tag.artist.data(), item.tag.artist.size(), tmp))
        item.tag.artist = tmp;
    else
        qDebug() << "no touch:" << QString::fromUtf8(item.tag.artist.c_str());

    if (!CharsetHelper::IsUtf8(item.tag.album.c_str())
            && IconvHelper::ConvFromTo("GBK", "UTF-8", item.tag.album.data(), item.tag.album.size(), tmp))
        item.tag.album = tmp;
    else
        qDebug() << "no touch:" << QString::fromUtf8(item.tag.album.c_str());

    if (!CharsetHelper::IsUtf8(item.tag.title.c_str())
            && IconvHelper::ConvFromTo("GBK", "UTF-8", item.tag.title.data(), item.tag.title.size(), tmp))
        item.tag.title = tmp;
    else
        qDebug() << "no touch:" << QString::fromUtf8(item.tag.title.c_str());

    // Build fields
    listRow.fields << new QStandardItem(QString::fromUtf8(item.tag.artist.c_str()));
    listRow.fields << new QStandardItem(QString::fromUtf8(item.tag.album.c_str()));
    listRow.fields << new QStandardItem(QString::fromUtf8(item.tag.title.c_str()));
    listRow.fields << new QStandardItem(QString::number(item.tag.track));
    listRow.fields << new QStandardItem(strDuration);
    for (int i = 0; i < listRow.fields.size(); ++i) {
        QStandardItem* item = listRow.fields[i];
        item->setEditable(false);
        item->setSizeHint(QSize(-1, 22));
    }

    return listRow;
}

void SimplePlaylistView::SlotCheckForScroll()
{

}
