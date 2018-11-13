#ifndef SECONDARYINFORMATIONLISTDELEGATE_H
#define SECONDARYINFORMATIONLISTDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>

class SecondaryInformationListDelegate : public QStyledItemDelegate
{
        Q_OBJECT
    public:
        explicit SecondaryInformationListDelegate(QObject *parent = nullptr);

        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // SECONDARYINFORMATIONLISTDELEGATE_H
