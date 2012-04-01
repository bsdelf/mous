#ifndef SIMPLEPLAYLISTVIEW_H
#define SIMPLEPLAYLISTVIEW_H

#include <QtCore>
#include <QtGui>
#include <deque>
#include "IPlayListView.h"
#include "DlgLoadingMedia.h"
#include <util/Playlist.h>
#include <scx/Thread.hpp>

namespace mous {
    struct MediaItem;
}


class SimplePlayListView : public QTreeView, public IPlayListView
{
    Q_OBJECT
public:
    explicit SimplePlayListView(QWidget *parent = 0);

public:
    virtual ~SimplePlayListView();

    virtual void setMediaLoader(const mous::IMediaLoader* loader);

    virtual const mous::MediaItem* getNextItem() const;
    virtual const mous::MediaItem* getPreviousItem() const;
    virtual size_t getItemCount() const;

signals:
    void sigPlayMediaItem(IPlayListView *view, const mous::MediaItem* item);
    void sigConvertMediaItem(const mous::MediaItem* item);
    void sigConvertMediaItems(QList<const mous::MediaItem*> items);

private:
    struct MediaRow
    {
        mous::MediaItem* item;
        QList<QStandardItem *> row;
    };

private:
    void mouseDoubleClickEvent(QMouseEvent * event);

private slots:
    void slotAppend();
    void slotRemove();

    void slotCopy();
    void slotCut();
    void slotPaste();

    void slotTagging();
    void slotConvert();
    void slotProperties();

    void slotPlaylistLoad();
    void slotPlaylistRename();
    void slotPlaylistSaveAs();

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
