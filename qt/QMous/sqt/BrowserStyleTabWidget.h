#ifndef BROWSERSTYLETABWIDGET_H
#define BROWSERSTYLETABWIDGET_H

#include <QtCore>
#include <QtGui>

namespace sqt {

class BrowserStyleTabBar: public QTabBar
{
    Q_OBJECT

public:
    BrowserStyleTabBar(QWidget* parent);
    ~BrowserStyleTabBar();

private:
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);

    void moveTab(int from, int to);

private:
    int mMouseTabOffset;
    bool mLastPressed;
};

class BrowserStyleTabWidget: public QWidget
{
    Q_OBJECT

public:
    BrowserStyleTabWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~BrowserStyleTabWidget();

signals:
    void sigRequestCloseTab(int index);

private:
    QVBoxLayout* mLayout;
    BrowserStyleTabBar* mTab;
    QStackedWidget* mStack;
};

}

#endif // BROWSERSTYLETABWIDGET_H
