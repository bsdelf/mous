#include "CustomHeadTabWidget.h"
using namespace sqt;
#include <QtGui>
#include <QtCore>

CustomHeadTabWidget::CustomHeadTabWidget(QWidget *parent):
    QTabWidget(parent)
{

}

void CustomHeadTabWidget::setTabBar(QTabBar *tb)
{
    QTabWidget::setTabBar(tb);
}

void CustomHeadTabWidget::mouseDoubleClickEvent(QMouseEvent* evt)
{
    QTabWidget::mouseDoubleClickEvent(evt);

    if (!tabBar()->underMouse())
        emit sigDoubleClick();
}
