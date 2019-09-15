#pragma once

#include <QtWidgets>

namespace sqt {

class CustomHeadTabWidget : public QTabWidget {
  Q_OBJECT

 public:
  CustomHeadTabWidget(QWidget* parent = 0);
  void SetTabBar(QTabBar* tb);

 signals:
  void SigDoubleClick();

 private:
  void mouseDoubleClickEvent(QMouseEvent* event);
};

}  // namespace sqt
