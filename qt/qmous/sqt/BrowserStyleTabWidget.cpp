#include "BrowserStyleTabWidget.h"
using namespace sqt;

/**
 * BrowserStyleTabBar
 */
BrowserStyleTabBar::BrowserStyleTabBar(QWidget* parent):
        QTabBar(parent),
        mLastPressed(false),
        mDelegate(NULL)
{
    addTab("+");
}

BrowserStyleTabBar::~BrowserStyleTabBar()
{

}

void BrowserStyleTabBar::setDelegate(TabWidgetDelegate* dg)
{
    mDelegate = dg;
}

void BrowserStyleTabBar::mousePressEvent(QMouseEvent *event)
{
    const QPoint& p = event->pos();
    if (event->button() == Qt::LeftButton) {
        if (tabRect(count()-1).contains(p)) {
            mLastPressed = true;
            return;
        } else {
            mLastPressed = false;
        }
    }

    QTabBar::mousePressEvent(event);
    mMouseTabOffset = tabRect(currentIndex()).right() - p.x();
}

void BrowserStyleTabBar::mouseReleaseEvent(QMouseEvent* event)
{
    const QPoint& p = event->pos();
    if (event->button() == Qt::LeftButton) {
        if (mLastPressed && tabRect(count()-1).contains(p)){
            if (mDelegate != NULL) {
                QWidget* relatedWidget = mDelegate->createWidget();
                QVariant var((qlonglong)relatedWidget);
                int index = insertTab(count()-1, "foo" + QString::number(count()-1));
                setTabData(index, var);
                setCurrentIndex(count()-2);
            }
        } else {
            QTabBar::mousePressEvent(event);
        }
        mLastPressed = false;
    } else if (event->button() == Qt::MidButton) {
        QTabBar::mousePressEvent(event);
        int tab = tabAt(p);
        if (tab >= 0 && tab < count()-1) {
            removeTab(tab);
            // Keep the tab at the original index if possible.
            if (tab == currentIndex()) {
                if (tab == count()-1)
                    --tab;
                if (tab < 0)
                    tab = 0;
                setCurrentIndex(tab);
            }
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

/**
 * BrowserStyleTabWidget
 */
BrowserStyleTabWidget::BrowserStyleTabWidget(QWidget* parent, Qt::WindowFlags f):
        QWidget(parent, f),
        mLayout(NULL),
        mTab(NULL),
        mStack(NULL),
        mDelegate(NULL)
{
    mLayout = new QVBoxLayout(this);
    mTab = new BrowserStyleTabBar(this);
    mStack = new QStackedWidget(this);

    mLayout->addWidget(mTab);
    mLayout->addWidget(mStack);
    setLayout(mLayout);

    mTab->setExpanding(false);
    mTab->setMovable(true);

    mLayout->setContentsMargins(0, 0, 0, 0);
}

BrowserStyleTabWidget::~BrowserStyleTabWidget()
{
    delete mLayout;
    delete mTab;
    delete mStack;
}

void BrowserStyleTabWidget::setDelegate(TabWidgetDelegate* dg)
{
    mDelegate = dg;
}
