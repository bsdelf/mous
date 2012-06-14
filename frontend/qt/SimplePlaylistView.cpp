#include "SimplePlaylistView.h"
#include <QtCore>
#include <QtGui>

#include <util/MediaItem.h>
#include <util/PlaylistSerializer.h>
#include <core/IMediaLoader.h>
using namespace mous;

#include <scx/Thread.hpp>
#include <scx/IconvHelper.hpp>
#include <scx/CharsetHelper.hpp>
using namespace scx;
using namespace std;

#include "UiHelper.hpp"
using namespace sqt;

#include "AppEnv.h"
#include "FoobarStyle.h"

const QString FILE_MIME = "file://";

typedef PlaylistActionHistory<MediaItem> ActionHistory;

SimplePlaylistView::SimplePlaylistView(QWidget *parent) :
    QTreeView(parent),
    m_MediaLoader(NULL),
    m_Clipboard(NULL),
    m_PlayModeGroup(this),
    m_ShortcutCopy(qobject_cast<QWidget*>(this)),
    m_ShortcutCut(qobject_cast<QWidget*>(this)),
    m_ShortcutPaste(qobject_cast<QWidget*>(this)),
    m_ShortcutDelete(qobject_cast<QWidget*>(this)),
    m_ShortcutUndo(qobject_cast<QWidget*>(this)),
    m_ShortcutRedo(qobject_cast<QWidget*>(this))
{
    m_FoobarStyle = new FoobarStyle(style());
    setStyle(m_FoobarStyle);

    setContextMenuPolicy(Qt::ActionsContextMenu);

    QList<QAction*> actionList;
    QAction* action = NULL;
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
    /*
    action = new QAction(tr("Tagging"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(SlotTagging()));
    actionList << action;
    */

    // Action convert
    action = new QAction(tr("Convert"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(SlotConvert()));
    actionList << action;

    // Action properties
    /*
    action = new QAction(tr("Properties"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(SlotProperties()));
    actionList << action;
    */

    /*
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
    */

    // Action play mode menu
    action = new QAction(tr("Play Mode"), this);
    actionList << action;
    menu = new QMenu(this);
    action->setMenu(menu);

    action = new QAction(tr("Normal"), this);
    action->setCheckable(true);
    m_PlayModeGroup.addAction(action);
    action = new QAction(tr("Repeat"), this);
    action->setCheckable(true);
    m_PlayModeGroup.addAction(action);
    action->setChecked(true);
    action = new QAction(tr("Shuffle"), this);
    action->setCheckable(true);
    m_PlayModeGroup.addAction(action);
    action = new QAction(tr("Shuffle Repeat"), this);
    action->setCheckable(true);
    m_PlayModeGroup.addAction(action);
    action = new QAction(tr("Repeat One"), this);
    action->setCheckable(true);
    m_PlayModeGroup.addAction(action);
    m_PlayModeGroup.setExclusive(true);
    menu->addActions(m_PlayModeGroup.actions());

    // Style
    setActionSeparator(actionList);
    addActions(actionList);

    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDefaultDropAction(Qt::IgnoreAction);

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
    headList << tr("Album") << tr("Artist") << tr("Title") << tr("Track") << tr("Duration");
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

    connect(&m_PlayModeGroup, SIGNAL(triggered(QAction*)), this, SLOT(SlotPlayModeMenu(QAction*)));

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
        QList<QStandardItem*> rowList = m_ItemModel.takeRow(0);
        for(int i = 0; i < rowList.size(); ++i)
            delete rowList[i];
    }

    m_Playlist.Clear();

}

/* IPlayListView interfaces */
void SimplePlaylistView::SetMediaLoader(const IMediaLoader* loader)
{
    m_MediaLoader = loader;
}

void SimplePlaylistView::SetClipboard(PlaylistClipboard<mous::MediaItem>* clipboard)
{
    m_Clipboard = clipboard;
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

const char* SimplePlaylistView::PlayMode() const
{
    int mode = (int)m_Playlist.Mode();
    if (mode < 0 || mode > m_PlayModeGroup.actions().size())
        return "";
    else
        return m_PlayModeGroup.actions()[mode]->text().toLocal8Bit().data();
}

void SimplePlaylistView::SetPlayMode(int mode)
{
    m_PlayModeGroup.actions()[mode]->setChecked(true);
}

void SimplePlaylistView::Save(const char* filename) const
{
    typedef PlaylistSerializer<MediaItem> Serializer;
    Serializer::Store(m_Playlist, filename);
}

void SimplePlaylistView::Load(const char* filename)
{
    typedef PlaylistSerializer<MediaItem> Serializer;
    Serializer::Load(m_Playlist, filename);

    for (int i = 0; i < m_Playlist.Count(); ++i) {
        ListRow listRow = BuildListRow(m_Playlist[i]);
        m_ItemModel.appendRow(listRow.fields);
    }
}

void SimplePlaylistView::OnMediaItemUpdated(const mous::MediaItem& item)
{
    if (m_Playlist.Empty())
        return;

    // we SHOULD check all items here!!!
    for (int row = 0; row < m_Playlist.Count(); ++row) {
        MediaItem& destItem = m_Playlist[row];
        if (destItem.url == item.url &&
                destItem.duration == item.duration &&
                destItem.hasRange == item.hasRange &&
                destItem.msBeg == item.msBeg &&
                destItem.msEnd == item.msEnd) {
            destItem = item;
            m_ItemModel.item(row, 0)->setText(QString::fromUtf8(item.tag.album.c_str()));
            m_ItemModel.item(row, 1)->setText(QString::fromUtf8(item.tag.artist.c_str()));
            m_ItemModel.item(row, 2)->setText(QString::fromUtf8(item.tag.title.c_str()));
            m_ItemModel.item(row, 3)->setText(QString::number(item.tag.track));
        }
    }
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
    QTreeView::dragEnterEvent(event);
    event->accept();
}

void SimplePlaylistView::dragMoveEvent(QDragMoveEvent *event)
{
    QTreeView::dragMoveEvent(event);
    event->accept();
}

void SimplePlaylistView::dropEvent(QDropEvent *event)
{
    event->accept();

    const QString& text = event->mimeData()->text();

    if (text.startsWith(FILE_MIME)) {
        qDebug() << "!drop:append file";

        QStringList files = text.split(FILE_MIME, QString::SkipEmptyParts);
        for (int i = 0; i < files.size(); ++i) {
            QString file = QUrl(files[i].trimmed()).toLocalFile();
            // try parse
            if (!QFileInfo(file).isFile()) {
                file = QUrl::fromEncoded(file.toLocal8Bit()).toLocalFile();
            }
            files[i] = file;

            qDebug() << files[i];
        }

        scx::Function<void (const QStringList&)> fn(&SimplePlaylistView::LoadMediaItem, this);
        m_LoadMediaThread.Run(fn, files);
        m_LoadMediaThread.Detach();
    } else if (text.isEmpty()) {
        QList<int> rowList = PickSelectedRows();
        qSort(rowList);
        if (!rowList.empty()) {
            // calc insert pos
            int visualInsertPos = indexAt(m_FoobarStyle->BelowIndicator()).row();
            if (visualInsertPos == -1)
                visualInsertPos = m_Playlist.Count();

            int realInsertPos = visualInsertPos;
            for (int i = 0; i < rowList.size(); ++i) {
                if (rowList[i] < visualInsertPos)
                    --realInsertPos;
            }

            qDebug() << "visualInsertPos" << visualInsertPos;
            qDebug() << "realInsertPos" << realInsertPos;

            // copy & remove
            deque<MediaItem> content(rowList.count());
            for (int i = 0; i < rowList.size(); ++i) {
                content[i] = m_Playlist[rowList[i]];
            }
            for (int i = rowList.size()-1; i >= 0; --i) {
                int delPos = rowList[i];
                m_ItemModel.removeRow(delPos);
            }

            // insert or append(optimized?)
            ActionHistory::Action action;
            action.type = ActionHistory::Move;
            action.moveVisualPos = visualInsertPos;
            action.moveInsertPos = realInsertPos;

            for (size_t i = 0; i < content.size(); ++i) {
                const ListRow& listRow = BuildListRow(content[i]);
                m_ItemModel.insertRow(realInsertPos+i, listRow.fields);
                action.srcItemList.push_back(std::pair<int, MediaItem>(rowList[i], listRow.item));
            }

            // as for playlist, we already have "move"
            m_Playlist.Move(rowList.toVector().toStdVector(), visualInsertPos);

            // Record operation
            if (!action.srcItemList.empty())
                m_History.PushUndoAction(action);
        }
    }

    QTreeView::dropEvent(event);
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
                this, tr("Open Media"), oldPath, "*");
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

    QModelIndexList list = selectedIndexes();
    if (list.empty())
        return;

    QModelIndex index(list[0]);
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

void SimplePlaylistView::SlotPlayModeMenu(QAction* action)
{
    int index = m_PlayModeGroup.actions().indexOf(action);
    if (index < 0)
        return;
    m_Playlist.SetMode((EmPlaylistMode)index);
}

void SimplePlaylistView::SlotShortcutCopy()
{
    qDebug() << "copy";

    if (m_Clipboard == NULL)
        return;

    QList<int> selectedRows = PickSelectedRows();
    if (selectedRows.empty()) {
        m_Clipboard->Clear();
        return;
    }

    deque<MediaItem> contents;
    for (int i = 0; i < selectedRows.size(); ++i) {
        contents.push_back(m_Playlist[selectedRows[i]]);
    }
    m_Clipboard->SetContent(contents);
}

void SimplePlaylistView::SlotShortcutCut()
{
    qDebug() << "cut";

    SlotShortcutCopy();
    SlotShortcutDelete();
}

void SimplePlaylistView::SlotShortcutPaste()
{
    qDebug() << "paste";

    if (m_Clipboard == NULL || m_Clipboard->Empty())
        return;

    deque<MediaItem> content = m_Clipboard->Content();

    // insert or append(optimized?)
    QList<int> selectedRows = PickSelectedRows();
    int insertPos = selectedRows.count() == 1 ? selectedRows[0] : -1;

    ActionHistory::Action action;
    action.type = ActionHistory::Insert;
    action.insertPos = insertPos;

    for (size_t i = 0; i < content.size(); ++i) {
        const ListRow& listRow = BuildListRow(content[i]);
        if (insertPos != -1) {
            m_Playlist.Insert(insertPos+i, listRow.item);
            m_ItemModel.insertRow(insertPos+i, listRow.fields);
        } else {
            m_Playlist.Append(listRow.item);
            m_ItemModel.appendRow(listRow.fields);
        }
        action.srcItemList.push_back(std::pair<int, MediaItem>(-1, listRow.item));
    }

    // Record operation
    if (!action.srcItemList.empty())
        m_History.PushUndoAction(action);
}

void SimplePlaylistView::SlotShortcutDelete()
{
    qDebug() << "delete";

    QList<int> selectedRows = PickSelectedRows();
    if (selectedRows.empty())
        return;
    qSort(selectedRows);

    ActionHistory::Action action;
    action.type = ActionHistory::Remove;

    for (int i = selectedRows.size()-1; i >= 0; --i) {
        int delPos = selectedRows[i];

        // push at front to ensure asc seq
        action.srcItemList.push_front(std::pair<int, MediaItem>(delPos, m_Playlist[delPos]));

        m_Playlist.Remove(delPos);
        m_ItemModel.removeRow(delPos);
    }

    // Record operation
    if (!action.srcItemList.empty())
        m_History.PushUndoAction(action);
}

void SimplePlaylistView::SlotShortcutUndo()
{
    qDebug() << "undo";

    if (m_History.HasUndoAction()) {
        ActionHistory::Action action = m_History.PopUndoAction();
        const int n = action.srcItemList.size();
        switch (action.type) {
        case ActionHistory::Insert:
        {
            assert(!m_Playlist.Empty() && m_ItemModel.rowCount() != 0);

            // previously inserted or appended ?
            const int rmSince = action.insertPos != -1 ? action.insertPos : m_Playlist.Count() - n;
            vector<int> indexes(n);
            for (int i = 0; i < n; ++i) {
                indexes[i] = rmSince + i;
            }
            m_Playlist.Remove(indexes);
            m_ItemModel.removeRows(rmSince, n);
        }
            break;

        case ActionHistory::Remove:
        {
            // the seq should already be asc here
            for (int i = 0; i < n; ++i) {
                int insertPos = action.srcItemList[i].first;
                MediaItem& item = action.srcItemList[i].second;
                const ListRow& listRow = BuildListRow(item);
                m_Playlist.Insert(insertPos, listRow.item);
                m_ItemModel.insertRow(insertPos, listRow.fields);
            }
        }
            break;

        case ActionHistory::Move:
        {
            int moveInsertPos = action.moveInsertPos;
            for (int i = 0; i < n; ++i) {
                MediaItem& item = action.srcItemList[i].second;
                const ListRow& listRow = BuildListRow(item);
                m_ItemModel.removeRow(moveInsertPos + i);
                m_ItemModel.insertRow(action.srcItemList[i].first, listRow.fields);
                m_Playlist.Move(vector<int>(1, moveInsertPos + i), action.srcItemList[i].first);
            }
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
        const int n = action.srcItemList.size();
        switch (action.type) {
        case ActionHistory::Insert:
        {
            // insert or append
            for (int i = 0; i < n; ++i) {
                ListRow listRow = BuildListRow(action.srcItemList[i].second);
                if (action.insertPos != -1) {
                    m_Playlist.Insert(action.insertPos+i, listRow.item);
                    m_ItemModel.insertRow(action.insertPos+i, listRow.fields);
                } else {
                    m_Playlist.Append(listRow.item);
                    m_ItemModel.appendRow(listRow.fields);
                }
            }
        }
            break;

        case ActionHistory::Remove:
        {
            // original seq is asc
            for (int i = n-1; i >= 0; --i) {
                int delPos = action.srcItemList[i].first;
                m_Playlist.Remove(delPos);
                m_ItemModel.removeRow(delPos);
            }
        }
            break;

        case ActionHistory::Move:
        {
            // remove
            for (int i = n-1; i >= 0; --i) {
                int delPos = action.srcItemList[i].first;
                m_ItemModel.removeRow(delPos);
            }

            vector<int> oldPos(n);
            int begPos = action.moveInsertPos;
            for (int i = 0; i < n; ++i) {
                oldPos[i] = action.srcItemList[i].first;
                const ListRow& listRow = BuildListRow(action.srcItemList[i].second);
                m_ItemModel.insertRow(begPos+i, listRow.fields);
            }

            // as for playlist, we already have "move"
            m_Playlist.Move(oldPos, action.moveVisualPos);
        }
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

    //emit SigReadyToLoad();

    // Prepare for history
    ActionHistory::Action action;
    action.type = ActionHistory::Insert;
    action.insertPos = -1;

    string ifNotUtf8 = GlobalAppEnv::Instance()->ifNotUtf8.toStdString();

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
            CorrectTagCharset(item.tag, ifNotUtf8);
            ListRow listRow = BuildListRow(item);
            action.srcItemList.push_back(std::pair<int, MediaItem>(-1, item));
            emit SigListRowGot(listRow);
        }
    }

    // Record operation
    if (!action.srcItemList.empty())
        m_History.PushUndoAction(action);

    //emit SigLoadFinished();
}

QList<int> SimplePlaylistView::PickSelectedRows() const
{
    QList<int> rowList;
    QModelIndexList indexList = selectedIndexes();
    QSet<int> rowSet;
    for (int i = 0; i < indexList.size(); ++i) {
        int row = indexList[i].row();
        if (!rowSet.contains(row)) {
            rowSet.insert(row);
            rowList.append(row);
        }
    }
    return rowList;
}

// TODO: this function should be seprated in to a head file,
// then FrmTagEditor::LoadMediaItem() can be LoadMediaFile()
void SimplePlaylistView::CorrectTagCharset(MediaTag& tag, const string& ifNotUtf8) const
{
    string tmp;
    // artist
    if (!CharsetHelper::IsUtf8(tag.artist.c_str())
            && IconvHelper::ConvFromTo(ifNotUtf8, "UTF-8", tag.artist.data(), tag.artist.size(), tmp))
        tag.artist = tmp;
    //else
    //    qDebug() << "no touch:" << QString::fromUtf8(tag.artist.c_str());

    // album
    if (!CharsetHelper::IsUtf8(tag.album.c_str())
            && IconvHelper::ConvFromTo(ifNotUtf8, "UTF-8", tag.album.data(), tag.album.size(), tmp))
        tag.album = tmp;
    //else
    //    qDebug() << "no touch:" << QString::fromUtf8(tag.album.c_str());

    // title
    if (!CharsetHelper::IsUtf8(tag.title.c_str())
            && IconvHelper::ConvFromTo(ifNotUtf8, "UTF-8", tag.title.data(), tag.title.size(), tmp))
        tag.title = tmp;
    //else
    //    qDebug() << "no touch:" << QString::fromUtf8(tag.title.c_str());

    // comment
    if (!CharsetHelper::IsUtf8(tag.comment.c_str())
            && IconvHelper::ConvFromTo(ifNotUtf8, "UTF-8", tag.comment.data(), tag.comment.size(), tmp))
        tag.comment = tmp;
    //else
    //    qDebug() << "no touch:" << QString::fromUtf8(tag.comment.c_str());

    // genre
    if (!CharsetHelper::IsUtf8(tag.genre.c_str())
            && IconvHelper::ConvFromTo(ifNotUtf8, "UTF-8", tag.genre.data(), tag.genre.size(), tmp))
        tag.genre = tmp;
    //else
    //    qDebug() << "no touch:" << QString::fromUtf8(tag.genre.c_str());
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

    // Build fields
    listRow.fields << new QStandardItem(QString::fromUtf8(item.tag.album.c_str()));
    listRow.fields << new QStandardItem(QString::fromUtf8(item.tag.artist.c_str()));    
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
