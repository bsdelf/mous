#include "BrowserStyleTabWidget.h"
using namespace sqt;

/**
 * BrowserStyleTabBar
 */
BrowserStyleTabBar::BrowserStyleTabBar(QWidget* parent):
        QTabBar(parent),
        mLastPressed(false)
{
    addTab("+");
}

BrowserStyleTabBar::~BrowserStyleTabBar()
{

}

void BrowserStyleTabBar::mousePressEvent(QMouseEvent *event)
{
    const QPoint& p = event->pos();
    if (tabRect(count()-1).contains(p)) {
        mLastPressed = true;
        return;
    } else {
        mLastPressed = false;
    }

    QTabBar::mousePressEvent(event);

    mMouseTabOffset = tabRect(currentIndex()).right() - p.x();
}

void BrowserStyleTabBar::mouseReleaseEvent(QMouseEvent* event)
{
    const QPoint& p = event->pos();
    if (event->button() == Qt::LeftButton) {
        if (mLastPressed && tabRect(count()-1).contains(p)){
            insertTab(count()-1, "foo" + QString::number(count()-1));
            setCurrentIndex(count()-2);
            mLastPressed = false;
        } else {
            QTabBar::mousePressEvent(event);
        }
    } else if (event->button() == Qt::MidButton) {
        QTabBar::mousePressEvent(event);
        int tab = tabAt(p);
        if (tab >= 0 && tab < count()-1) {
            removeTab(tab);
            if (tab == currentIndex() && tab > 0)
                setCurrentIndex(tab-1);
        }
    }
}

void BrowserStyleTabBar::mouseMoveEvent(QMouseEvent* event)
{
    if (mLastPressed)
        return;

    const QPoint& p = event->pos();
    if (p.x()+mMouseTabOffset < tabRect(count()-1).left())
        QTabBar::mouseMoveEvent(event);
}

void BrowserStyleTabBar::moveTab(int from, int to)
{

    QTabBar::moveTab(from, to);
}

/**
 * BrowserStyleTabWidget
 */
BrowserStyleTabWidget::BrowserStyleTabWidget(QWidget* parent, Qt::WindowFlags f):
        QWidget(parent, f)
{
    mLayout = new QVBoxLayout();
    mTab = new BrowserStyleTabBar(this);
    mStack = new QStackedWidget(this);

    mLayout->addWidget(mTab);
    mLayout->addWidget(mStack);
    this->setLayout(mLayout);

    mTab->setExpanding(false);
    mTab->setMovable(true);
    //mTab->addTab("new");


}

BrowserStyleTabWidget::~BrowserStyleTabWidget()
{

}
