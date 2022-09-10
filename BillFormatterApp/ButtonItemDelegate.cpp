#include <QtGui>
#include <qdialog.h>
#include <qapplication.h>
#include "ButtonItemDelegate.h"
#include "BillFormatterApp.h"

ButtonItemDelegate::ButtonItemDelegate(QObject* parent)
    : QItemDelegate(parent)
{
}

void ButtonItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (0 < index.column())
    {
        return QItemDelegate::paint(painter, option, index);
    }

    QStyleOptionButton button;
    QRect r = option.rect;//getting the rect of the cell
    int x, y, w, h;
    x = r.left() + r.width() - 30;//the X coordinate
    y = r.top();//the Y coordinate
    w = 30;//button width
    h = 30;//button height
    button.rect = QRect(x, y, w, h);
    button.text = "=^.^=";
    button.state = QStyle::State_Enabled;

    QApplication::style()->drawControl(QStyle::CE_PushButton, &button, painter);
}

bool ButtonItemDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if (0 < index.column())
    {
        return QItemDelegate::editorEvent(event, model, option, index);
    }

    if (event->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent* e = (QMouseEvent*)event;
        int clickX = e->x();
        int clickY = e->y();

        QRect r = option.rect;//getting the rect of the cell
        int x, y, w, h;
        x = r.left() + r.width() - 30;//the X coordinate
        y = r.top();//the Y coordinate
        w = 30;//button width
        h = 30;//button height

        if (clickX > x && clickX < x + w)
        {
            if (clickY > y && clickY < y + h)
            {
                //QDialog* d = new QDialog();
                //d->setGeometry(0, 120, 100, 100);
                //d->show();
                convert_document(index);
            }
        }
    }

    return true;
}

void ButtonItemDelegate::convert_document(const QModelIndex& index)
{
    BillFormatterApp::instance()->convertRecord(index.row());
}
