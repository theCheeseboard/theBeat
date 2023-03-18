#include "podcastmodel.h"

#include "podcast.h"
#include "podcastitem.h"
#include <QPainter>
#include <QStaticText>
#include <tapplication.h>
#include <thebeatcommon.h>

struct PodcastModelPrivate {
        QPointer<Podcast> podcast;
};

PodcastModel::PodcastModel(QObject* parent) :
    QAbstractListModel(parent) {
    d = new PodcastModelPrivate();
}

PodcastModel::~PodcastModel() {
    delete d;
}

void PodcastModel::setPodcast(Podcast* podcast) {
    this->beginResetModel();
    if (d->podcast) {
        d->podcast->disconnect(this);
    }
    d->podcast = podcast;
    this->endResetModel();
}

int PodcastModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }

    if (!d->podcast) return 0;

    return d->podcast->items().count();
}

QVariant PodcastModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid())
        return QVariant();

    auto item = d->podcast->items().at(index.row());
    switch (role) {
        case Qt::DisplayRole:
            return item->title();
        case PodcastItem:
            return QVariant::fromValue(item);
        case Description:
            return item->description();
        case PlainDescription:
            return item->plainDescription();
        case PublishDate:
            return item->published();
        case Duration:
            return item->duration();
    }

    return QVariant();
}

PodcastItemDelegate::PodcastItemDelegate(QObject* parent) :
    QStyledItemDelegate{parent} {
}

tPaintCalculator PodcastItemDelegate::paintCalculator(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    tPaintCalculator calculator;
    calculator.setPainter(painter);
    calculator.setDrawBounds(option.rect);
    calculator.setLayoutDirection(option.direction);

    QPen transientColor = option.palette.color(QPalette::Disabled, QPalette::WindowText);

    QStringList information;
    information.append(QLocale().toString(index.data(PodcastModel::PublishDate).toDateTime().date(), "dd MMM yyyy"));
    information.append(TheBeatCommon::durationToString(index.data(PodcastModel::Duration).toULongLong() * 1000));
    QString informationString = libContemporaryCommon::humanReadablePartJoinString().append(information.join(libContemporaryCommon::humanReadablePartJoinString()));

    auto title = index.data(Qt::DisplayRole).toString();

    auto preferredTitleWidth = option.fontMetrics.horizontalAdvance(title);
    auto preferredInfoWidth = option.fontMetrics.horizontalAdvance(informationString);
    const auto maxWidth = option.rect.width() - 18;

    auto titleWidth = preferredTitleWidth;
    auto infoWidth = preferredInfoWidth;

    if (titleWidth + infoWidth > maxWidth) {
        titleWidth = maxWidth - infoWidth;
        if (titleWidth < 200) {
            titleWidth = 200;
            infoWidth = maxWidth - titleWidth;
        }
    }

    QRect textRect;
    textRect.setWidth(titleWidth + 1);
    textRect.setHeight(option.fontMetrics.height());
    textRect.moveTopLeft(option.rect.topLeft() + QPoint(9, 9));
    calculator.addRect(textRect, [painter, index, option, title](QRectF drawBounds) {
        painter->setPen(option.palette.color(QPalette::WindowText));
        painter->setFont(option.font);
        painter->drawText(drawBounds, Qt::AlignLeft, option.fontMetrics.elidedText(title, Qt::ElideRight, drawBounds.width()));
    });

    QRect infoRect = textRect;
    infoRect.setWidth(infoWidth + 1);
    infoRect.moveLeft(textRect.right());
    calculator.addRect(infoRect, [painter, index, option, transientColor, informationString](QRectF drawBounds) {
        painter->setPen(transientColor);
        painter->drawText(drawBounds, Qt::AlignLeft, option.fontMetrics.elidedText(informationString, Qt::ElideRight, drawBounds.width()));
    });

    auto description = index.data(PodcastModel::PlainDescription).toString().replace("\n", " ");
    QRect summaryRect = textRect;
    summaryRect.moveTop(textRect.bottom() + 3);
    summaryRect.setHeight(option.fontMetrics.height() * 3);
    summaryRect.setWidth(maxWidth);
    summaryRect = option.fontMetrics.boundingRect(summaryRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, description);
    if (summaryRect.height() > option.fontMetrics.height() * 3) summaryRect.setHeight(option.fontMetrics.height() * 3);
    calculator.addRect(summaryRect, [painter, index, option, description, transientColor](QRectF drawBounds) {
        painter->setPen(transientColor);
        painter->drawText(drawBounds, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, description);
    });

    return calculator;
}

void PodcastItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    tApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter);

    painter->save();
    auto paintCalculator = this->paintCalculator(painter, option, index);
    paintCalculator.performPaint();
    painter->restore();
}

QSize PodcastItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
    return paintCalculator(nullptr, option, index).sizeWithMargins().toSize();
}
