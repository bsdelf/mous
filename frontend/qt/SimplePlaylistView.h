#ifndef SIMPLEPLAYLISTVIEW_H
#define SIMPLEPLAYLISTVIEW_H

#include <QtCore>
#include <QtGui>

#include <deque>

#include <scx/Thread.hpp>

#include <util/MediaItem.h>
#include <util/Playlist.h>
using namespace mous;

#include "IPlaylistView.h"
#include "DlgLoadingMedia.h"
#include "PlaylistActionHistory.h"

class SimplePlaylistView : public QTreeView, public IPlaylistView
{
    Q_OBJECT

public:
    explicit SimplePlaylistView(QWidget *parent = 0);

public:
    virtual ~SimplePlaylistView();

    virtual void SetMediaLoader(const IMediaLoader* loader);

    virtual const MediaItem* NextItem() const;
    virtual const MediaItem* PrevItem() const;
    virtual int ItemCount() const;

signals:
    void SigPlayMediaItem(IPlaylistView *view, const MediaItem& item);
    void SigConvertMediaItem(const MediaItem& item);
    void SigConvertMediaItems(const QList<MediaItem>& items);

private:
    struct ListRow
    {
        MediaItem item;
        QList<QStandardItem*> fields;
    };

private:
    void SetupShortcuts();

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent * event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dropEvent(QDropEvent *event);    

private slots:
    void SlotAppend();

    void SlotTagging();
    void SlotConvert();
    void SlotProperties();

    void SlotPlaylistLoad();
    void SlotPlaylistRename();
    void SlotPlaylistSaveAs();

    void SlotReadyToLoad();
    void SlotLoadFinished();
    void SlotListRowGot(const ListRow& listRow);

    void SlotShortcutCopy();
    void SlotShortcutCut();
    void SlotShortcutPaste();
    void SlotShortcutDelete();
    void SlotShortcutUndo();
    void SlotShortcutRedo();

signals:
    void SigReadyToLoad();
    void SigListRowGot(const ListRow& listRow);
    void SigLoadFinished();

private:
    void LoadMediaItem(const QStringList& pathList);
    ListRow BuildListRow(MediaItem &item) const;

private slots:
    void SlotCheckForScroll();

private:
    const IMediaLoader* m_MediaLoader;

    QString m_PrevMediaFilePath;

    QStandardItemModel m_ItemModel;
    Playlist<MediaItem> m_Playlist;

    DlgLoadingMedia m_DlgLoadingMedia;
    scx::Thread m_LoadMediaThread;

    PlaylistActionHistory<MediaItem> m_History;

    QPoint m_DragStartPos;

    QTimer m_ScrollTimer;

    QShortcut m_ShortcutCopy;
    QShortcut m_ShortcutCut;
    QShortcut m_ShortcutPaste;
    QShortcut m_ShortcutDelete;
    QShortcut m_ShortcutUndo;
    QShortcut m_ShortcutRedo;
};

#endif // SIMPLEPLAYLISTVIEW_H
