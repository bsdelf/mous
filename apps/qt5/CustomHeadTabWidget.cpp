#include "CustomHeadTabWidget.hpp"
using namespace sqt;

CustomHeadTabWidget::CustomHeadTabWidget(QWidget* parent)
    : QTabWidget(parent) {
  setMouseTracking(true);
}

void CustomHeadTabWidget::SetTabBar(QTabBar* tb) {
  QTabWidget::setTabBar(tb);
}

void CustomHeadTabWidget::mouseDoubleClickEvent(QMouseEvent* event) {
  QTabWidget::mouseDoubleClickEvent(event);

  if (!tabBar()->underMouse())
    emit SigDoubleClick();
}
