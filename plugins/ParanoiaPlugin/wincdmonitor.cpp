#include "wincdmonitor.h"

#include <QApplication>
#include <QTimer>
#include "paranoiacdcontroller.h"
#include <tlogger.h>

#include <Windows.h>

#include <Dbt.h>
#include <comutil.h>
#include <imapi2.h>
#include <winrt/base.h>
#include <atlsafe.h>

using namespace Qt::Literals;

struct WinCdMonitorPrivate {
    winrt::com_ptr<IDiscMaster2> discMaster;
    QMap<bstr_t, ParanoiaCdController*> controllers;
};

WinCdMonitor::WinCdMonitor(QObject* parent)
    : QObject{parent}, d{new WinCdMonitorPrivate()} {
    qApp->installNativeEventFilter(this);

    d->discMaster = winrt::create_instance<IDiscMaster2>(CLSID_MsftDiscMaster2);
    updateDisks();
}

WinCdMonitor::~WinCdMonitor() {
    delete d;
}

bool WinCdMonitor::nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) {
#ifdef Q_OS_WIN
    if (eventType == "windows_generic_MSG") {
        auto msg = static_cast<MSG*>(message);
        if (msg->message == WM_DEVICECHANGE) {
            auto lpdb = reinterpret_cast<PDEV_BROADCAST_HDR>(msg->lParam);
            switch (msg->wParam) {
                case DBT_DEVICEARRIVAL:
                    if (lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME) {
                        QTimer::singleShot(0, this, &WinCdMonitor::updateDisks);
                    }
                    break;
                case DBT_DEVICEREMOVECOMPLETE:
                    if (lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME) {
                        QTimer::singleShot(0, this, &WinCdMonitor::updateDisks);
                    }
                    break;
            }
        }
    }
#endif
    return false;
}

void WinCdMonitor::updateDisks() {
    LONG count;
    winrt::check_hresult(d->discMaster->get_Count(&count));

    QList<bstr_t> availableDriveIds;
    for (LONG i = 0; i < count; i++) {
        BSTR id;
        winrt::check_hresult(d->discMaster->get_Item(i, &id));
        bstr_t wrappedId(id, false);

        if (!d->controllers.contains(wrappedId)) {
            try {
                auto discRecorder = winrt::create_instance<IDiscRecorder2>(CLSID_MsftDiscRecorder2);

                winrt::check_hresult(discRecorder->InitializeDiscRecorder(wrappedId));
                SAFEARRAY* driveLetters;
                winrt::check_hresult(discRecorder->get_VolumePathNames(&driveLetters));

                ATL::CComSafeArray<VARIANT> driveLetterVariants;
                driveLetterVariants.Attach(driveLetters);

                if (driveLetterVariants.GetCount() > 0) {
                    variant_t driveLetterVariant = driveLetterVariants[driveLetterVariants.GetLowerBound()];
                    auto driveLetter = static_cast<bstr_t>(driveLetterVariant);
                    auto widget = new ParanoiaCdController(QString::fromWCharArray(driveLetter).remove("\\"));
                    d->controllers.insert(wrappedId, widget);
                } else {
                    tWarn("WinCdMonitor") << "Could not create burner because no drive letter is attached";
                }
            } catch (...) {
                // Do nothing
                tWarn("WinCdMonitor") << "Could not create burner. HRESULT " << QString::number(static_cast<ulong>(winrt::to_hresult()), 16);
            }
        }
        availableDriveIds.append(wrappedId);
    }

    QList<bstr_t> driveIdsToRemove;
    for (bstr_t driveId : d->controllers.keys()) {
        if (!availableDriveIds.contains(driveId)) driveIdsToRemove.append(driveId);
    }

    for (bstr_t driveId : driveIdsToRemove) {
        auto controller = d->controllers.take(driveId);
        controller->deleteLater();
    }
}
