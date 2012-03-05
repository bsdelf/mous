#ifndef SIMPLEPLAYLISTVIEW_H
#define SIMPLEPLAYLISTVIEW_H

#include <QtCore>
#include <QtGui>
#include <deque>
#include "IPlayListView.h"

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

    virtual void setMediaLoader(const mous::MediaLoader* loader);

    virtual const mous::MediaItem* getNextItem();
    virtual const mous::MediaItem* getPreviousItem();
    virtual size_t getItemCount() const;

signals:
    void sigPlayMediaItem(const mous::MediaItem* item);

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
    const mous::MediaLoader* mMediaLoader;

    QString mOldMediaPath;

    QStandardItemModel mModel;

    std::deque<mous::MediaItem*> mMediaItemList;
};

#endif // SIMPLEPLAYLISTVIEW_H
