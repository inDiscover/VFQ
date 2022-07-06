#pragma once

#include <qitemdelegate.h>

class ButtonItemDelegate :
    public QItemDelegate
{
    Q_OBJECT

public:
    ButtonItemDelegate(QObject* parent = 0);
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index);
};
