#include "winburnprovider.h"

#include <comutil.h>
#include <statemanager.h>
#include <tlogger.h>
#include <burnmanager.h>
#include <tpopover.h>
#include <imapi2.h>
#include <winrt/base.h>
#include "winburnpopover.h"
#include "daoformatlocker.h"
#include "winburndaoimage.h"
#include <imapi2error.h>
#include <Dbt.h>
#include "winburnjob.h"
#include <tjobmanager.h>

using namespace Qt::Literals;

struct WinBurnProviderPrivate {
    _bstr_t driveId;

    QString burnerName;
    QString burnerVendor;
    QStringList driveLetters;

    QString albumName;
    WinBurnDaoImagePtr burnImage;
    QString admonition;
    bool admonitionIsError;
    bool imageReady;

    winrt::com_ptr<IDiscMaster2> discMaster;
    winrt::com_ptr<IDiscRecorder2> discRecorder;

    bool visible;
};

WinBurnProvider::WinBurnProvider(_bstr_t driveId, QObject* parent) : BurnBackend(parent) {
    d = new WinBurnProviderPrivate();
    d->driveId = driveId;

    auto discMaster = winrt::create_instance<IDiscMaster2>(CLSID_MsftDiscMaster2);
    auto discRecorder = winrt::create_instance<IDiscRecorder2>(CLSID_MsftDiscRecorder2);
    winrt::check_hresult(discRecorder->InitializeDiscRecorder(d->driveId.GetBSTR()));

    BSTR productId;
    winrt::check_hresult(discRecorder->get_ProductId(&productId));
    d->burnerName = QString::fromWCharArray(productId).trimmed();
    SysFreeString(productId);

    BSTR vendorId;
    winrt::check_hresult(discRecorder->get_VendorId(&vendorId));
    d->burnerVendor = QString::fromWCharArray(vendorId).trimmed();
    SysFreeString(vendorId);

    SAFEARRAY* driveLetters;
    winrt::check_hresult(discRecorder->get_VolumePathNames(&driveLetters));

    Q_ASSERT(driveLetters->cDims == 1);
    LONG lBound;
    LONG uBound;
    winrt::check_hresult(SafeArrayGetLBound(driveLetters, 1, &lBound));
    winrt::check_hresult(SafeArrayGetUBound(driveLetters, 1, &uBound));

    for (LONG i = lBound; i < uBound; i++) {
        VARIANT variant;
        VariantInit(&variant);
        winrt::check_hresult(SafeArrayGetElement(driveLetters, &i, &variant));
        _variant_t variantWrap;
        variantWrap.Attach(variant);
        auto str = static_cast<_bstr_t>(variantWrap);

        d->driveLetters.append(QString::fromWCharArray(str));
    }

    SafeArrayDestroy(driveLetters);

    StateManager::instance()->burn()->registerBackend(this);

    qApp->installNativeEventFilter(this);
}

WinBurnProvider::~WinBurnProvider() {
    if (StateManager::instance()->burn()->availableBackends().contains(this)) StateManager::instance()->burn()->deregisterBackend(this);
    delete d;
}

bool WinBurnProvider::visible() {
    return d->visible;
}

void WinBurnProvider::close() {
    d->visible = false;
    emit visibleChanged();
}

QString WinBurnProvider::albumName() {
    return d->albumName;
}

void WinBurnProvider::setAlbumName(QString albumName) {
    d->albumName = albumName;
    emit albumNameChanged();
}

QString WinBurnProvider::admonition() {
    return d->admonition;
}

bool WinBurnProvider::admonitionIsError() {
    return d->admonitionIsError;
}

bool WinBurnProvider::imageReady() {
    return d->imageReady;
}

void WinBurnProvider::startBurn() {
    auto burnJob = new WinBurnJob(d->burnImage, d->driveId, d->albumName);
    tJobManager::trackJob(burnJob);

    d->visible = false;
    emit visibleChanged();
}

void WinBurnProvider::updateCd() {
    try {
        auto discFormatDAO = winrt::create_instance<IDiscFormat2RawCD>(CLSID_MsftDiscFormat2RawCD);
        discFormatDAO->put_ClientName(_bstr_t("theBeat"));
        winrt::check_hresult(discFormatDAO->put_Recorder(d->discRecorder.get()));

        DaoFormatLocker locker(discFormatDAO);

        LONG lastPossibleStartOfLeadout;
        winrt::check_hresult(discFormatDAO->get_LastPossibleStartOfLeadout(&lastPossibleStartOfLeadout));
        if (d->burnImage->leadoutLba() > lastPossibleStartOfLeadout || d->burnImage->leadoutLba() == 0) {
            d->admonition = tr("This playlist is too long to fit on the CD");
            d->admonitionIsError = true;
            emit admonitionChanged();
            return;
        }

        VARIANT_BOOL isBlank;
        winrt::check_hresult(!discFormatDAO->get_MediaPhysicallyBlank(&isBlank));
        if (isBlank == VARIANT_FALSE) {
            d->admonition = tr("The CD is not blank");
            d->admonitionIsError = false;
            emit admonitionChanged();
            return;
        }

        d->admonition = {};
        d->admonitionIsError = false;
        emit admonitionChanged();
    } catch (...) {
        winrt::hresult hr = winrt::to_hresult();
        switch (hr) {
            case E_IMAPI_RECORDER_MEDIA_NO_MEDIA:
            case E_IMAPI_DF2RAW_MEDIA_IS_NOT_SUPPORTED:
                d->admonition = tr("Insert a CD-R or CD-RW into the drive.");
                d->admonitionIsError = true;
                emit admonitionChanged();
                break;
            case E_IMAPI_DF2RAW_RECORDER_NOT_SUPPORTED:
            case E_IMAPI_RECORDER_MEDIA_BUSY:
            case E_IMAPI_RECORDER_MEDIA_FORMAT_IN_PROGRESS:
                d->admonition = tr("The drive is busy. Please wait for other disc operations to complete.");
                d->admonitionIsError = true;
                emit admonitionChanged();
                break;
            case E_IMAPI_DF2RAW_MEDIA_IS_NOT_BLANK:
                d->admonition = tr("The CD is not blank");
                d->admonitionIsError = true;
                emit admonitionChanged();
                break;
            default:
                d->admonition = tr("Can't burn to this disc. Try again with a different disc.").append(QStringLiteral(" HRESULT: 0x%1").arg(static_cast<ulong>(hr), 16, 16, QChar('0')));
                d->admonitionIsError = true;
                emit admonitionChanged();
        }
    }
}

void WinBurnProvider::burn(QStringList files, QString albumName, QWidget* parentWindow) {
    WinBurnPopover* jp = new WinBurnPopover(files, d->driveId, albumName);
    tPopover* popover = new tPopover(jp);
    popover->setPopoverWidth(SC_DPI(-200));
    popover->setPopoverSide(tPopover::Bottom);
    connect(jp, &WinBurnPopover::done, popover, &tPopover::dismiss);
    connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
    connect(popover, &tPopover::dismissed, jp, &WinBurnPopover::deleteLater);
    popover->show(parentWindow->window());
}

QString WinBurnProvider::displayName() {
    return QStringLiteral("%1 %2 (%3)").arg(d->burnerVendor, d->burnerName, d->driveLetters.join(", "));
}

QUrl WinBurnProvider::qmlFile() {
    return QUrl(u"qrc:/qt/qml/com/vicr123/thebeat/plugin/winintegration/WinBurnPopover.qml"_s);
}

QString WinBurnProvider::burn(QStringList files, QString albumName, QQuickWindow* parentWindow) {
    setAlbumName(albumName);

    d->burnImage = WinBurnDaoImagePtr(new WinBurnDaoImage());

    d->discMaster = winrt::create_instance<IDiscMaster2>(CLSID_MsftDiscMaster2);
    d->discRecorder = winrt::create_instance<IDiscRecorder2>(CLSID_MsftDiscRecorder2);
    winrt::check_hresult(d->discRecorder->InitializeDiscRecorder(d->driveId.GetBSTR()));

    d->imageReady = false;
    emit imageReadyChanged();
    d->burnImage->createImageFromFiles(files).then([ this ] {
        updateCd();
        d->imageReady = true;
        emit imageReadyChanged();
    });

    d->visible = true;
    emit visibleChanged();
    return {};
}


bool WinBurnProvider::nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) {
    Q_UNUSED(eventType)
    Q_UNUSED(result)

    MSG* msg = reinterpret_cast<MSG*>(message);
    if (msg->message == WM_DEVICECHANGE) {
        switch (msg->wParam) {
            case DBT_DEVICEARRIVAL:
            case DBT_DEVICEREMOVECOMPLETE:
            case DBT_DEVICETYPESPECIFIC:
                if (d->visible) {
                    QTimer::singleShot(0, this, &WinBurnProvider::updateCd);
                }
                *result = TRUE;
                return true;
        }
    }

    return false;
}
