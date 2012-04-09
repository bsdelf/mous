#ifndef SIMPLEPLAYLISTVIEW_H
#define SIMPLEPLAYLISTVIEW_H

#include <QtCore>
#include <QtGui>
#include <deque>
#include "IPlaylistView.h"
#include "DlgLoadingMedia.h"
#include <util/Playlist.h>
#include <scx/Thread.hpp>

namespace mous {
    struct MediaItem;
}

class SimplePlaylistView : public QTreeView, public IPlaylistView
{
    Q_OBJECT
public:
    explicit SimplePlaylistView(QWidget *parent = 0);

public:
    virtual ~SimplePlaylistView();

    virtual void SetMediaLoader(const mous::IMediaLoader* loader);

    virtual const mous::MediaItem* GetNextItem() const;
    virtual const mous::MediaItem* GetPreviousItem() const;
    virtual size_t GetItemCount() const;

signals:
    void SigPlayMediaItem(IPlaylistView *view, const mous::MediaItem* item);
    void SigConvertMediaItem(const mous::MediaItem* item);
    void SigConvertMediaItems(QList<const mous::MediaItem*> items);

private:
    struct MediaRow
    {
        mous::MediaItem* item;
        QList<QStandardItem *> row;
    };

private:
    void mouseDoubleClickEvent(QMouseEvent * event);

private slots:
    void SlotAppend();
    void SlotRemove();

    void SlotCopy();
    void SlotCut();
    void SlotPaste();

    void SlotTagging();
    void SlotConvert();
    void SlotProperties();

    void SlotPlaylistLoad();
    void SlotPlaylistRename();
    void SlotPlaylistSaveAs();

    void SlotReadyToLoad();
    void SlotLoadFinished();
    void SlotMediaRowGot(MediaRow *mediaRow);

signals:
    void SigReadyToLoad();
    void SigMediaRowGot(MediaRow* mediaRow);
    void SigLoadFinished();

private:
    void LoadMediaItem(const QStringList& pathList);

private:
    const mous::IMediaLoader* m_MediaLoader;

    QString m_OldMediaPath;

    QStandardItemModel m_StModel;
    mous::Playlist<mous::MediaItem*> m_Playlist;

    QTimer m_PickMediaItemTimer;
    DlgLoadingMedia m_DlgLoadingMedia;
    scx::Thread m_LoadMediaThread;
    QMutex m_TmpLoadMutex;
    QList<MediaRow> m_TmpLoadList;
    bool m_LoadFinished;
};

#endif // SIMPLEPLAYLISTVIEW_H
