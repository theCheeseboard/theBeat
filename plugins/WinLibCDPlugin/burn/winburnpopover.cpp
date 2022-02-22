#include "winburnpopover.h"
#include "ui_winburnpopover.h"

#include "winburnjob.h"
#include <comutil.h>
#include <tjobmanager.h>
#include <tlogger.h>

#include <Dbt.h>
#include <imapi2error.h>
#include "winburndaoimage.h"
#include "daoformatlocker.h"

struct WinBurnPopoverPrivate {
    QStringList files;
    _bstr_t driveId;
    quint64 playlistLength = 0;

    WinBurnDaoImagePtr burnImage;

    winrt::com_ptr<IDiscMaster2> discMaster;
    winrt::com_ptr<IDiscRecorder2> discRecorder;
};

WinBurnPopover::WinBurnPopover(QStringList files, _bstr_t driveId, QString albumName, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::WinBurnPopover) {
    ui->setupUi(this);

    d = new WinBurnPopoverPrivate();
    d->files = files;
    d->driveId = driveId;
    d->burnImage = WinBurnDaoImagePtr(new WinBurnDaoImage());

    d->discMaster = winrt::create_instance<IDiscMaster2>(CLSID_MsftDiscMaster2);
    d->discRecorder = winrt::create_instance<IDiscRecorder2>(CLSID_MsftDiscRecorder2);
    winrt::check_hresult(d->discRecorder->InitializeDiscRecorder(d->driveId.GetBSTR()));

    qApp->installNativeEventFilter(this);

    ui->titleLabel->setText(tr("Burn %1").arg(QLocale().quoteString(albumName)));
    ui->titleLabel->setBackButtonShown(true);
    ui->burnOptionsWidget->setFixedWidth(SC_DPI(600));

    ui->albumNameEdit->setText(albumName);

    QPalette pal = ui->warningFrame->palette();
    pal.setColor(QPalette::Window, QColor(255, 100, 0));
    pal.setColor(QPalette::WindowText, Qt::white);
    ui->warningFrame->setPalette(pal);

    d->burnImage->createImageFromFiles(files)->then([ = ] {
        updateCd();

        ui->stackedWidget->setCurrentAnimation(tStackedWidget::Fade);
        ui->stackedWidget->setCurrentWidget(ui->optionsPage);
    });

    ui->warningFrame->setTitle(tr("Heads up!").toUpper());
}

WinBurnPopover::~WinBurnPopover() {
    qApp->removeNativeEventFilter(this);
    delete ui;
}

void WinBurnPopover::on_burnButton_clicked() {
    WinBurnJob* burnJob = new WinBurnJob(d->burnImage, d->driveId, ui->albumNameEdit->text());
    tJobManager::trackJob(burnJob);
    emit done();
}

void WinBurnPopover::on_titleLabel_backButtonClicked() {
    emit done();
}

void WinBurnPopover::updateCd() {
    try {
        auto discFormatDAO = winrt::create_instance<IDiscFormat2RawCD>(CLSID_MsftDiscFormat2RawCD);
        discFormatDAO->put_ClientName(_bstr_t("theBeat"));
        winrt::check_hresult(discFormatDAO->put_Recorder(d->discRecorder.get()));

        DaoFormatLocker locker(discFormatDAO);

        LONG lastPossibleStartOfLeadout;
        winrt::check_hresult(discFormatDAO->get_LastPossibleStartOfLeadout(&lastPossibleStartOfLeadout));
        if (d->burnImage->leadoutLba() > lastPossibleStartOfLeadout || d->burnImage->leadoutLba() == 0) {
            setErrorState(tr("This playlist is too long to fit on the CD"));
            return;
        }

        VARIANT_BOOL isBlank;
        winrt::check_hresult(!discFormatDAO->get_MediaPhysicallyBlank(&isBlank));
        if (isBlank == VARIANT_FALSE) {
            setWarningState(tr("The CD is not blank."));
            return;
        }

        setOkState();
    } catch (...) {
        winrt::hresult hr = winrt::to_hresult();
        switch (hr) {
            case E_IMAPI_RECORDER_MEDIA_NO_MEDIA:
                setErrorState(tr("Insert a CD-R or CD-RW into the drive."));
                break;
            case E_IMAPI_DF2RAW_RECORDER_NOT_SUPPORTED:
            case E_IMAPI_RECORDER_MEDIA_BUSY:
            case E_IMAPI_RECORDER_MEDIA_FORMAT_IN_PROGRESS:
                setErrorState(tr("The drive is busy. Please wait for other disc operations to complete."));
                break;
            default:
                setErrorState(tr("Can't burn to this disc. Try again with a different disc.").append(QStringLiteral(" HRESULT: %1").arg(hr, 0, 16)));
        }
    }
}

void WinBurnPopover::setOkState() {
    ui->warningFrame->setVisible(false);
    ui->burnButton->setEnabled(true);
}

void WinBurnPopover::setWarningState(QString warning) {
    ui->warningFrame->setState(tStatusFrame::Warning);
    ui->warningFrame->setText(warning);
    ui->warningFrame->setVisible(true);
    ui->burnButton->setEnabled(true);
}

void WinBurnPopover::setErrorState(QString error) {
    ui->warningFrame->setState(tStatusFrame::Error);
    ui->warningFrame->setText(error);
    ui->warningFrame->setVisible(true);
    ui->burnButton->setEnabled(false);
}


bool WinBurnPopover::nativeEventFilter(const QByteArray& eventType, void* message, long* result) {
    Q_UNUSED(eventType)
    Q_UNUSED(result)

    MSG* msg = reinterpret_cast<MSG*>(message);
    if (msg->message == WM_DEVICECHANGE) {
        tDebug("WinBurnPopover") << "WINEVENT";
        switch (msg->wParam) {
            case DBT_DEVICEARRIVAL:
            case DBT_DEVICEREMOVECOMPLETE:
            case DBT_DEVICETYPESPECIFIC:
                QTimer::singleShot(0, this, &WinBurnPopover::updateCd);
                *result = TRUE;
                return true;
        }
    }

    return false;
}
