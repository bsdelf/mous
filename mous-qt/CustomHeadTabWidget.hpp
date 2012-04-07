#ifndef CUSTOMHEADTABWIDGET_H
#define CUSTOMHEADTABWIDGET_H

#include <QTabWidget>

namespace sqt {

class CustomHeadTabWidget: public QTabWidget
{
    Q_OBJECT

public:
    CustomHeadTabWidget(QWidget * parent = 0);
    void setTabBar(QTabBar* tb);

signals:
    void sigDoubleClick();

private:
    void mouseDoubleClickEvent(QMouseEvent* evt);
};

}

#endif // CUSTOMHEADTABWIDGET_H
