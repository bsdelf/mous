#ifndef SIMPLEPLAYLISTVIEW_H
#define SIMPLEPLAYLISTVIEW_H

#include <QtCore>
#include <QtGui>
#include "IPlayListView.h"

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
};

#endif // SIMPLEPLAYLISTVIEW_H
