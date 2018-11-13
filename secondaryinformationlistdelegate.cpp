#include "secondaryinformationlistdelegate.h"

#include <QPainter>
#include <the-libs_global.h>

SecondaryInformationListDelegate::SecondaryInformationListDelegate(QObject *parent) : QStyledItemDelegate(parent)
{

}

void SecondaryInformationListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QPen transientColor = option.palette.color(QPalette::Disabled, QPalette::WindowText);

    painter->setPen(Qt::transparent);
    QPen textPen;
    if (option.state & QStyle::State_Selected) {
        painter->setBrush(option.palette.brush(QPalette::Highlight));
        textPen = option.palette.color(QPalette::HighlightedText);
        transientColor = textPen;
    } else if (option.state & QStyle::State_MouseOver) {
        QColor col = option.palette.color(QPalette::Highlight);
        col.setAlpha(127);
        painter->setBrush(col);
        textPen = option.palette.color(QPalette::HighlightedText);
    } else {
        painter->setBrush(option.palette.brush(QPalette::Window));
        textPen = option.palette.color(QPalette::WindowText);
    }
    painter->drawRect(option.rect);

    QRect textRect = option.rect;
    textRect.moveLeft(6 * theLibsGlobal::getDPIScaling());
    if (index.data(Qt::DecorationRole).canConvert<QIcon>()) {
        QRect iconRect;
        iconRect.setSize(QSize(16, 16) * theLibsGlobal::getDPIScaling());
        QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
        QImage iconImage = icon.pixmap(iconRect.size()).toImage();
        iconRect.moveLeft(option.rect.left() + 2 * theLibsGlobal::getDPIScaling());
        iconRect.moveTop(option.rect.top() + (option.rect.height() / 2) - (iconRect.height() / 2));
        painter->drawImage(iconRect, iconImage);
        textRect.setLeft(iconRect.right() + 6 * theLibsGlobal::getDPIScaling());
    }

    //Draw the primary information
    QRect nameRect = textRect;
    nameRect.setWidth(option.fontMetrics.width(index.data().toString()) + 1);
    textRect.setLeft(nameRect.right());

    painter->setPen(option.palette.color(QPalette::WindowText));
    painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, index.data().toString());

    //Draw the secondary information
    if (index.data(Qt::UserRole).toString() != "") {
        QString string = " - " + index.data(Qt::UserRole).toString();
        painter->setPen(transientColor);
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, string);
    }
}

QSize SecondaryInformationListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QSize s = QStyledItemDelegate::sizeHint(option, index);

    int width = 6 * theLibsGlobal::getDPIScaling() + option.fontMetrics.width(index.data().toString()) + option.fontMetrics.width(index.data(Qt::UserRole).toString());
    s.setWidth(width);

    return s;
}
