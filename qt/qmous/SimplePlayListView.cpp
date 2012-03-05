#include "SimplePlayListView.h"
#include <QtCore>
#include <QtGui>
#include <mous/MediaItem.h>
#include "UiHelper.hpp"
using namespace sqt;
using namespace mous;

SimplePlayListView::SimplePlayListView(QWidget *parent) :
    QTreeView(parent)
{
    setContextMenuPolicy(Qt::ActionsContextMenu);

    //QAction* action = NULL;

    QAction* actionPlaylist = new QAction(tr("Playlist"), this);
    QMenu* menuPlaylist = new QMenu(this);
    menuPlaylist->addAction(new QAction(tr("Load"), this));
    menuPlaylist->addAction(new QAction(tr("Rename"), this));
    menuPlaylist->addAction(new QAction(tr("Save As"), this));
    actionPlaylist->setMenu(menuPlaylist);

    /*
    QAction* actionPlayMode = new QAction(tr("Play Mode"), this);
    QActionGroup* groupPlayMode = new QActionGroup(this);
    QMenu* menuPlayMode = new QMenu(this);

    action = new QAction(tr("Normal"), this);
    action->setCheckable(true);
    groupPlayMode->addAction(action);
    menuPlayMode->addAction(action);

    menuPlayMode->addAction(new QAction(tr("Repeat Playlist"), this));
    menuPlayMode->addAction(new QAction(tr("Repeat Track"), this));
    menuPlayMode->addAction(new QAction(tr("Shuffle Track"), this));
    menuPlayMode->addAction(new QAction(tr("Shuffle Playlist"), this));
    actionPlayMode->setMenu(menuPlayMode);
    */

    QList<QAction*> actionList;
    actionList << new QAction(tr("Append"), this)
               << new QAction(tr("Remove"), this)
               << new QAction(tr("Cut"), this)
               << new QAction(tr("Paste"), this)
               << new QAction(this)
               << new QAction(tr("Tagging"), this)
               << new QAction(tr("Convert"), this)
               << new QAction(tr("Properties"), this)
               << new QAction(this)
               << actionPlaylist;

    setActionSeparator(actionList);
    addActions(actionList);

    setAcceptDrops(true);
    setDragEnabled(true);
}

SimplePlayListView::~SimplePlayListView()
{
    QList<QAction*> actionList = actions();
    while (!actionList.isEmpty()) {
        QAction* action = actionList.takeFirst();
        removeAction(action);
        if (action->menu() != NULL)
            delete action->menu();
        delete action;
    }

}

const MediaItem* SimplePlayListView::getNextItem()
{
    return NULL;
}

const MediaItem* SimplePlayListView::getPreviousItem()
{
    return NULL;
}

const size_t SimplePlayListView::getItemCount()
{
    return 0;
}
