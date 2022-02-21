#include "winburnpopover.h"
#include "ui_winburnpopover.h"

#include "winburnjob.h"
#include <comutil.h>
#include <tjobmanager.h>

struct WinBurnPopoverPrivate {
    QStringList files;
    _bstr_t driveId;
    quint64 playlistLength = 0;
};

WinBurnPopover::WinBurnPopover(QStringList files, _bstr_t driveId, QString albumName, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::WinBurnPopover) {
    ui->setupUi(this);

    d = new WinBurnPopoverPrivate();
    d->files = files;
    d->driveId = driveId;

    ui->titleLabel->setText(tr("Burn %1").arg(QLocale().quoteString(albumName)));
    ui->titleLabel->setBackButtonShown(true);
    ui->burnOptionsWidget->setFixedWidth(SC_DPI(600));

    ui->albumNameEdit->setText(albumName);

    QPalette pal = ui->warningFrame->palette();
    pal.setColor(QPalette::Window, QColor(255, 100, 0));
    pal.setColor(QPalette::WindowText, Qt::white);
    ui->warningFrame->setPalette(pal);

//    for (QString file : files) {
//        TagLib::FileRef tagFile(file.toStdString().data());
//        if (tagFile.audioProperties()) {
//            d->playlistLength += tagFile.audioProperties()->length() * 1000;
//        }
//    }
}

WinBurnPopover::~WinBurnPopover() {
    delete ui;
}

void WinBurnPopover::on_burnButton_clicked() {
    WinBurnJob* burnJob = new WinBurnJob(d->files, d->driveId, ui->albumNameEdit->text());
    tJobManager::trackJob(burnJob);
    emit done();
}

void WinBurnPopover::on_titleLabel_backButtonClicked() {
    emit done();
}
