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

    QList<QAction*> actionList;
    QAction* action = NULL;
    QMenu* menu = NULL;

    // Action append
    action = new QAction(tr("Append"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(slotAppend()));
    actionList << action;

    // Action remove
    action = new QAction(tr("Remove"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(slotRemove()));
    actionList << action;

    // Action cut
    action = new QAction(tr("Cut"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(slotCut()));
    actionList << action;

    // Action paste
    action = new QAction(tr("Paste"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(slotPaste()));
    actionList << action;

    actionList << new QAction(this);

    // Action tagging
    action = new QAction(tr("Tagging"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(slotTagging()));
    actionList << action;

    // Action convert
    action = new QAction(tr("Convert"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(slotConvert()));
    actionList << action;

    // Action properties
    action = new QAction(tr("Properties"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(slotProperties()));
    actionList << action;

    actionList << new QAction(this);

    // Action playlist menu
    action = new QAction(tr("Playlist"), this);
    actionList << action;
    menu = new QMenu(this);
    action->setMenu(menu);

    action = new QAction(tr("Load"), this);
    menu->addAction(action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotPlaylistLoad()));

    action = new QAction(tr("Rename"), this);
    menu->addAction(action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotPlaylistRename()));

    action = new QAction(tr("Save As"), this);
    menu->addAction(action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotPlaylistSaveAs()));

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

/* IPlayListView interfaces */
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

/* Action menus */
void SimplePlayListView::slotAppend()
{

}

void SimplePlayListView::slotRemove()
{

}

void SimplePlayListView::slotCut()
{

}

void SimplePlayListView::slotPaste()
{

}

void SimplePlayListView::slotTagging()
{

}

void SimplePlayListView::slotConvert()
{

}

void SimplePlayListView::slotProperties()
{

}

void SimplePlayListView::slotPlaylistLoad()
{

}

void SimplePlayListView::slotPlaylistRename()
{

}

void SimplePlayListView::slotPlayListSaveAs()
{

}
