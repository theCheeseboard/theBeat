/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2022 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/
#include "printpopover.h"
#include "ui_printpopover.h"

#include "common.h"
#include <QPainter>
#include <QPrintPreviewWidget>
#include <QPrinterInfo>
#include <tpaintcalculator.h>

struct PrintPopoverPrivate {
        QPrinter* printer;
        QPrintPreviewWidget* printPreview;
        AbstractLibraryBrowser::ListInformation listInformation;
};

PrintPopover::PrintPopover(AbstractLibraryBrowser::ListInformation listInformation, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::PrintPopover) {
    ui->setupUi(this);
    d = new PrintPopoverPrivate();
    d->listInformation = listInformation;

    d->printer = new QPrinter(QPrinter::HighResolution);
    d->printer->setPageMargins(QMarginsF(1, 1, 1, 1), QPageLayout::Inch);
    d->printer->setDocName(listInformation.name);

    d->printPreview = new QPrintPreviewWidget(d->printer);
    connect(d->printPreview, &QPrintPreviewWidget::paintRequested, this, &PrintPopover::paintPrinter);
    ui->printerLayout->addWidget(d->printPreview);

    ui->containerWidget->setFixedWidth(SC_DPI(600));

    ui->titleLabel->setText(tr("Print %1").arg(QLocale().quoteString(listInformation.name)));
    ui->titleLabel->setBackButtonShown(true);

    ui->printerBox->addItems(QPrinterInfo::availablePrinterNames());
    ui->printerBox->setCurrentIndex(QPrinterInfo::availablePrinterNames().indexOf(QPrinterInfo::defaultPrinterName()));
    updatePageSizes();
}

PrintPopover::~PrintPopover() {
    delete d;
    delete ui;
}

void PrintPopover::on_titleLabel_backButtonClicked() {
    emit done();
}

void PrintPopover::paintPrinter(QPrinter* printer) {
    if (ui->trackListingButton->isChecked()) {
        paintTrackListing(printer);
    } else if (ui->jewelCaseButton->isChecked()) {
    }
}

void PrintPopover::paintTrackListing(QPrinter* printer) {
    QPainter painter(printer);

    QRectF pageBounds = printer->pageLayout().paintRectPixels(printer->resolution());
    pageBounds.moveTo(0, 0);
    double scale = printer->resolution() / 96.0;

    tPaintCalculator calculator;
    calculator.setPainter(&painter);
    calculator.setLayoutDirection(this->layoutDirection());
    calculator.setDrawBounds(pageBounds);

    QFont titleFont(this->font(), printer);
    titleFont.setPointSize(25);
    QFontMetrics titleFontMetrics(titleFont);

    QFont bodyFont(this->font(), printer);
    bodyFont.setPointSize(11);
    QFontMetrics bodyFontMetrics(bodyFont);

    QFont trackFont(bodyFont, printer);
    trackFont.setPointSize(bodyFont.pointSize() * 2);
    QFontMetrics trackFontMetrics(trackFont);

    QRectF imageRect;
    imageRect.setSize(QSizeF(150, 150) * scale);
    imageRect.moveTopLeft(pageBounds.topLeft());
    calculator.addRect(imageRect, [&](QRectF drawBounds) {
        if (d->listInformation.graphic.isNull()) {
            painter.fillRect(drawBounds, QColor(200, 200, 200));
        } else {
            painter.drawImage(drawBounds, d->listInformation.graphic);
        }
    });

    QRectF titleRect;
    titleRect.setHeight(titleFontMetrics.height());
    titleRect.setWidth(titleFontMetrics.horizontalAdvance(d->listInformation.name));
    titleRect.moveTopLeft(imageRect.topRight() + QPointF(20, 0) * scale);
    calculator.addRect(titleRect, [&](QRectF drawBounds) {
        painter.setFont(titleFont);
        painter.drawText(drawBounds, Qt::AlignLeft, d->listInformation.name);
    });

    QStringList subtitleText;
    subtitleText.append(tr("%n tracks", nullptr, d->listInformation.tracks.length()));

    quint64 totalDuration = 0;
    for (AbstractLibraryBrowser::TrackInformation trackInformation : d->listInformation.tracks) {
        totalDuration += trackInformation.duration;
    }
    subtitleText.append(Common::durationToString(totalDuration));

    QRectF subtitleRect;
    subtitleRect.setHeight(bodyFontMetrics.height());
    subtitleRect.setWidth(bodyFontMetrics.horizontalAdvance(subtitleText.join(" · ")));
    subtitleRect.moveTopLeft(titleRect.bottomLeft());
    calculator.addRect(subtitleRect, [&](QRectF drawBounds) {
        painter.setFont(bodyFont);
        painter.drawText(drawBounds, Qt::AlignLeft, subtitleText.join(" · "));
    });

    double y = imageRect.bottom() + 10 * scale;
    bool shaded = true;

    for (AbstractLibraryBrowser::TrackInformation trackInformation : d->listInformation.tracks) {
        int rowHeight = trackFontMetrics.height() * 1.5;

        if (y + rowHeight > pageBounds.bottom()) {
            y = 0;
            calculator.addRect(QRect(), [=](QRectF drawBounds) {
                printer->newPage();
            });
        }

        QRectF shadeRect;
        shadeRect.setWidth(pageBounds.width());
        shadeRect.setHeight(rowHeight);
        shadeRect.moveTop(y);
        shadeRect.moveLeft(imageRect.left());
        if (shaded) {
            calculator.addRect(shadeRect, [&](QRectF drawBounds) {
                painter.setBrush(QColor(200, 200, 200));
                painter.setPen(Qt::transparent);
                painter.drawRect(drawBounds);
            });
        }
        y += shadeRect.height();

        QRectF trackRect;
        trackRect.setWidth(trackFontMetrics.horizontalAdvance("99"));
        trackRect.setHeight(trackFontMetrics.height());
        trackRect.moveTop(shadeRect.top() + trackRect.height() / 4);
        trackRect.moveLeft(shadeRect.left() + trackRect.height() / 4);
        calculator.addRect(trackRect, [=, &painter](QRectF drawBounds) {
            QString text = "-";
            if (trackInformation.trackNumber != 0) text = QLocale().toString(trackInformation.trackNumber);

            painter.setPen(Qt::black);
            painter.setFont(trackFont);
            painter.drawText(drawBounds, Qt::AlignCenter, text);
        });

        QStringList trackTitleText;
        trackTitleText.append(trackInformation.title);
        trackTitleText.append(Common::durationToString(trackInformation.duration));

        QRectF trackTitleRect;
        trackTitleRect.setWidth(bodyFontMetrics.horizontalAdvance(trackTitleText.join(" · ")));
        trackTitleRect.setHeight(bodyFontMetrics.height());
        trackTitleRect.moveTopLeft(trackRect.topRight() + QPointF(trackRect.height() / 4 + 10 * scale, 0));
        calculator.addRect(trackTitleRect, [=, &painter](QRectF drawBounds) {
            painter.setFont(bodyFont);
            painter.drawText(drawBounds, Qt::AlignCenter, trackTitleText.join(" · "));
        });

        QStringList trackInfoText;
        if (!trackInformation.artist.isEmpty()) trackInfoText.append(tr("by %1").arg(trackInformation.artist));
        if (!trackInformation.album.isEmpty()) trackInfoText.append(tr("on %1").arg(trackInformation.album));

        QRectF trackInfoRect;
        trackInfoRect.setWidth(bodyFontMetrics.horizontalAdvance(trackInfoText.join(" · ")) + 1);
        trackInfoRect.setHeight(bodyFontMetrics.height());
        trackInfoRect.moveTopLeft(trackTitleRect.bottomLeft());
        if (trackInfoRect.right() > shadeRect.right() - 10 * scale) trackInfoRect.setRight(shadeRect.right() - 10 * scale);
        calculator.addRect(trackInfoRect, [=, &painter](QRectF drawBounds) {
            painter.setFont(bodyFont);
            painter.drawText(drawBounds, Qt::AlignCenter, bodyFontMetrics.elidedText(trackInfoText.join(" · "), Qt::ElideRight, trackInfoRect.width()));
        });

        shaded = !shaded;
    }

    calculator.performPaint();
    painter.end();
}

void PrintPopover::on_trackListingButton_toggled(bool checked) {
    if (checked) d->printPreview->updatePreview();
}

void PrintPopover::on_jewelCaseButton_toggled(bool checked) {
    if (checked) d->printPreview->updatePreview();
}

void PrintPopover::on_printerBox_currentIndexChanged(int index) {
    d->printer->setPrinterName(ui->printerBox->itemText(index));
    updatePageSizes();
    d->printPreview->updatePreview();
}

void PrintPopover::on_printButton_clicked() {
    paintPrinter(d->printer);
    emit done();
}

void PrintPopover::on_pageSizeBox_currentIndexChanged(int index) {
    d->printer->setPageSize(QPrinterInfo::printerInfo(d->printer->printerName()).supportedPageSizes().at(index));
    d->printPreview->updatePreview();
}

void PrintPopover::updatePageSizes() {
    QSignalBlocker blocker(ui->pageSizeBox);
    ui->pageSizeBox->clear();

    QPrinterInfo printerInfo = QPrinterInfo::printerInfo(d->printer->printerName());
    for (QPageSize pageSize : printerInfo.supportedPageSizes()) {
        ui->pageSizeBox->addItem(pageSize.name());
        if (printerInfo.defaultPageSize() == pageSize) {
            d->printer->setPageSize(pageSize);
            ui->pageSizeBox->setCurrentIndex(ui->pageSizeBox->count() - 1);
        }
    }
}

void PrintPopover::on_grayscaleBox_toggled(bool checked) {
    d->printer->setColorMode(checked ? QPrinter::GrayScale : QPrinter::Color);
    d->printPreview->updatePreview();
}

void PrintPopover::on_duplexBox_toggled(bool checked) {
    d->printer->setDuplex(checked ? QPrinter::DuplexLongSide : QPrinter::DuplexNone);
    d->printPreview->updatePreview();
}
