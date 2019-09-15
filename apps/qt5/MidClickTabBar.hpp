#ifndef MIDCLICKTABBAR_H
#define MIDCLICKTABBAR_H

#include <QTabBar>

namespace sqt {

class MidClickTabBar : public QTabBar {
  Q_OBJECT

 public:
  MidClickTabBar(QWidget* parent = 0);

 signals:
  void SigMidClick(int id);

 private:
  virtual void mouseReleaseEvent(QMouseEvent* event);
};

}  // namespace sqt

#endif  // DCLICKTABBAR_H
