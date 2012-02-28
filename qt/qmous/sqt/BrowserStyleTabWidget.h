#ifndef BROWSERSTYLETABWIDGET_H
#define BROWSERSTYLETABWIDGET_H

#include <QtCore>
#include <QtGui>

namespace sqt {

class TabWidgetDelegate
{
public:
    virtual ~TabWidgetDelegate() { }
    virtual QWidget* createWidget() = 0;
    virtual void releaseWidget(QWidget* widget) = 0;
};

class BrowserStyleTabBar: public QTabBar
{
    Q_OBJECT

public:
    BrowserStyleTabBar(QWidget* parent);
    ~BrowserStyleTabBar();

    void setDelegate(TabWidgetDelegate* dg);

signals:
    void sigNewTab(QWidget* newWidget);
    void sigTabClosed(QWidget* closedWidget);
    void sigTabChanged(QWidget* relatedWidget);

private:
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);

private:
    int mMouseTabOffset;
    bool mLastPressed;
    TabWidgetDelegate* mDelegate;
};

class BrowserStyleTabWidget: public QWidget
{
    Q_OBJECT

public:
    BrowserStyleTabWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~BrowserStyleTabWidget();

    void setDelegate(TabWidgetDelegate* dg);

signals:
    void sigRequestCloseTab(int index);

private:
    QVBoxLayout* mLayout;
    BrowserStyleTabBar* mTab;
    QStackedWidget* mStack;

    TabWidgetDelegate* mDelegate;
};

}

#endif // BROWSERSTYLETABWIDGET_H
