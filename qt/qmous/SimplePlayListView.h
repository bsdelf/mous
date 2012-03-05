#ifndef SIMPLEPLAYLISTVIEW_H
#define SIMPLEPLAYLISTVIEW_H

#include <QTreeView>
#include "IPlayListView.h"

class SimplePlayListView : public QTreeView, public IPlayListView
{
    Q_OBJECT
public:
    explicit SimplePlayListView(QWidget *parent = 0);

public:
    virtual ~SimplePlayListView();
    virtual const mous::MediaItem* getNextItem();
    virtual const mous::MediaItem* getPreviousItem();
    virtual const size_t getItemCount();

signals:
    
private slots:
    void slotAppend();
    void slotRemove();
    void slotCut();
    void slotPaste();

    void slotTagging();
    void slotConvert();
    void slotProperties();

    void slotPlaylistLoad();
    void slotPlaylistRename();
    void slotPlayListSaveAs();
};

#endif // SIMPLEPLAYLISTVIEW_H
