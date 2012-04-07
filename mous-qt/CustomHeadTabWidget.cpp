#include "CustomHeadTabWidget.hpp"
using namespace sqt;
#include <QtGui>
#include <QtCore>

CustomHeadTabWidget::CustomHeadTabWidget(QWidget *parent):
    QTabWidget(parent)
{

}

void CustomHeadTabWidget::SetTabBar(QTabBar *tb)
{
    QTabWidget::setTabBar(tb);
}

void CustomHeadTabWidget::mouseDoubleClickEvent(QMouseEvent* evt)
{
    QTabWidget::mouseDoubleClickEvent(evt);

    if (!tabBar()->underMouse())
        emit SigDoubleClick();
}
