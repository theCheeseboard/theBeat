#include "winburnprovider.h"

#include <comutil.h>
#include <statemanager.h>
#include <tlogger.h>
#include <burnmanager.h>
#include <tpopover.h>
#include <imapi2.h>
#include <winrt/base.h>
#include "winburnpopover.h"

struct WinBurnProviderPrivate {
    _bstr_t driveId;

    QString burnerName;
    QString burnerVendor;
    QStringList driveLetters;
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

    tDebug("WinBurnProvider") << "lbound: " << (int) lBound;
    tDebug("WinBurnProvider") << "hbound: " << (int) uBound;

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

    tDebug("WinBurnProvider") << "Created burn provider: " << static_cast<const char*>(driveId);
    StateManager::instance()->burn()->registerBackend(this);
}

WinBurnProvider::~WinBurnProvider() {
    delete d;
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
