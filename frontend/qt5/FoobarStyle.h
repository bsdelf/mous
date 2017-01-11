#pragma once

#include <QtWidgets>

class FoobarStyle: public QProxyStyle
{
public:
    FoobarStyle(QStyle *baseStyle = 0):
        QProxyStyle(baseStyle)
    {
    }

    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
    {
        if (element == QStyle::PE_IndicatorItemViewItemDrop) {
            qDebug() << option->rect;
            int y = 0;
            if (!option->rect.isNull()) {
                y = option->rect.y();
            } else {
                const QAbstractItemView* view = qobject_cast<const QAbstractItemView*>(widget);
                if (view == nullptr)
                    return;
                int rows = view->model()->rowCount();
                QModelIndex last = view->model()->index(rows-1, 0);
                QRect lastRect = view->visualRect(last);
                y = lastRect.bottom();
            }

            if (!option->rect.isNull()) {
                m_BelowIndicator.setX(widget->rect().width()/2);
                m_BelowIndicator.setY(y + option->rect.height()/2);
            } else {
                m_BelowIndicator.setX(-1);
                m_BelowIndicator.setY(-1);
            }

            painter->setRenderHint(QPainter::Antialiasing, true);
            QColor c(Qt::black);
            QPen pen(c);
            pen.setWidth(2);
            QBrush brush(c);
            painter->setPen(pen);
            painter->setBrush(brush);

            QPoint a(0, y);
            QPoint b(widget->rect().width(), y);
            painter->drawLine(a, b);

            painter->drawPoint(m_BelowIndicator);

        } else {
            m_BelowIndicator.setX(-1);
            m_BelowIndicator.setY(-1);

            QProxyStyle::drawPrimitive(element, option, painter, widget);
        }
    }

    QPoint BelowIndicator() const
    {
        return m_BelowIndicator;
    }

private:
    mutable QPoint m_BelowIndicator;
};

