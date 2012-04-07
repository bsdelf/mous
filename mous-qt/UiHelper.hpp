#ifndef SQT_UIHELPER_HPP
#define SQT_UIHELPER_HPP

#include <QtGui>
#include <QtCore>

namespace sqt {

static inline void  setActionSeparator(QList<QAction*> list)
{
    for (int i = 0; i < list.size(); ++i) {
        QAction* action = list[i];
        if (action->text().isEmpty())
            action->setSeparator(true);
    }
}

static inline void adjustAllStackPages(QStackedWidget* stack, QSizePolicy plicy)
{
    for (int i = 0; i < stack->count(); ++i)
    {
        stack->widget(i)->setSizePolicy(plicy);
    }
}

static inline void switchStackPage(QStackedWidget* stack, int index)
{
    const int pageCount = stack->count();
    if (pageCount != 0)
    {
        QSizePolicy policyMin(QSizePolicy::Ignored, QSizePolicy::Ignored);
        QSizePolicy policyMax(QSizePolicy::Expanding, QSizePolicy::Expanding);

        // minimal previous page
        if (stack->currentIndex() != index)
        {
            stack->currentWidget()->setSizePolicy(policyMin);
        }
        else
        {
            // minimal all page
            adjustAllStackPages(stack, policyMin);
        }
        // show and maximal new page
        stack->setCurrentIndex(index);
        stack->currentWidget()->setSizePolicy(policyMax);
        stack->adjustSize();
    }
}

}

#endif
