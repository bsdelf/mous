#ifndef SIMPLEPLAYLISTVIEW_H
#define SIMPLEPLAYLISTVIEW_H

#include <QtCore>
#include <QtGui>
#include <deque>
#include "IPlayListView.h"
#include <util/Playlist.h>

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

private:
    const mous::IMediaLoader* mMediaLoader;

    QString mOldMediaPath;

    QStandardItemModel mModel;

    mous::Playlist<mous::MediaItem*> mMediaList;
};

#endif // SIMPLEPLAYLISTVIEW_H
