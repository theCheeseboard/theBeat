#include "headerbackgroundcontroller.h"

#include <QGraphicsBlurEffect>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QPainter>
#include <tpaintcalculator.h>

struct HeaderBackgroundControllerPrivate {
        QWidget* parent;
        QImage image;
        int topPadding;

        QImage cachedBlur;
        QRect cachedBlurSize;
};

HeaderBackgroundController::HeaderBackgroundController(QWidget* parent) :
    QObject{parent} {
    d = new HeaderBackgroundControllerPrivate;
    parent->installEventFilter(this);
    d->parent = parent;
}

HeaderBackgroundController::~HeaderBackgroundController() {
    delete d;
}

void HeaderBackgroundController::setImage(QImage image) {
    d->image = image;
    d->cachedBlurSize = QRect();
    d->parent->update();
}

void HeaderBackgroundController::setTopPadding(int topPadding) {
    d->topPadding = topPadding;
}

bool HeaderBackgroundController::eventFilter(QObject* watched, QEvent* event) {
    if (watched == d->parent && event->type() == QEvent::Paint) {
        auto painter = new QPainter(d->parent);

        tPaintCalculator paintCalculator;
        paintCalculator.setPainter(painter);
        paintCalculator.setDrawBounds(d->parent->size());

        QColor backgroundCol = d->parent->palette().color(QPalette::Window);
        if ((backgroundCol.red() + backgroundCol.green() + backgroundCol.blue()) / 3 < 127) {
            backgroundCol = QColor(0, 0, 0, 150);
        } else {
            backgroundCol = QColor(255, 255, 255, 150);
        }

        if (d->image.isNull()) {
            // TODO
            //            ui->buttonSpacer->setFixedWidth(0);
        } else {
            QRect rect;
            rect.setSize(d->image.size().scaled(d->parent->width(), d->parent->height(), Qt::KeepAspectRatioByExpanding));
            rect.moveLeft(d->parent->width() / 2 - rect.width() / 2);
            rect.moveTop(d->parent->height() / 2 - rect.height() / 2);

            const int radius = 30;
            if (d->cachedBlurSize != rect || d->cachedBlurSize.isNull()) {
                auto blur = new QGraphicsBlurEffect();
                blur->setBlurRadius(radius);

                QGraphicsScene scene;
                QGraphicsPixmapItem item;
                item.setPixmap(QPixmap::fromImage(d->image));
                item.setGraphicsEffect(blur);
                scene.addItem(&item);

                QRect anchoredRect = rect;
                anchoredRect.moveTopLeft(QPoint(0, 0));
                d->cachedBlur = QImage(anchoredRect.size(), QImage::Format_ARGB32);
                anchoredRect.adjust(-radius, -radius, radius, radius);

                QPainter blurPainter(&d->cachedBlur);
                scene.render(&blurPainter, anchoredRect, QRectF(-radius, -radius, d->image.width() + radius, d->image.height() + radius));
                d->cachedBlurSize = rect;
            }

            // Blur the background
            paintCalculator.addRect(rect, [this, painter, radius](QRectF paintBounds) {
                painter->drawImage(paintBounds.adjusted(-radius, -radius, radius, radius), d->cachedBlur);
            });

            QRect rightRect;
            rightRect.setSize(d->image.size().scaled(0, d->parent->height() - d->topPadding, Qt::KeepAspectRatioByExpanding));
            rightRect.moveRight(d->parent->width());
            rightRect.moveTop(d->topPadding + (d->parent->height() - d->topPadding) / 2 - rightRect.height() / 2);

            paintCalculator.addRect(rightRect, [this, painter, backgroundCol](QRectF paintBounds) {
                painter->setBrush(backgroundCol);
                painter->setPen(Qt::transparent);
                painter->drawRect(0, 0, d->parent->width(), d->parent->height());
                painter->drawImage(paintBounds, d->image.scaled(paintBounds.size().toSize(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            });

            //            ui->buttonSpacer->setFixedWidth(rightRect.width());
        }

        paintCalculator.performPaint();

        delete painter;
    }
    return false;
}
