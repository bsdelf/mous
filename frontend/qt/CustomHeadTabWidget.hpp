#ifndef CUSTOMHEADTABWIDGET_H
#define CUSTOMHEADTABWIDGET_H

#include <QTabWidget>

namespace sqt {

class CustomHeadTabWidget: public QTabWidget
{
    Q_OBJECT

public:
    CustomHeadTabWidget(QWidget * parent = 0);
    void SetTabBar(QTabBar* tb);

signals:
    void SigDoubleClick();

private:
    void mouseDoubleClickEvent(QMouseEvent* event);
};

}

#endif // CUSTOMHEADTABWIDGET_H
