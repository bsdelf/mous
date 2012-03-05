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
    
public slots:
    
};

#endif // SIMPLEPLAYLISTVIEW_H
