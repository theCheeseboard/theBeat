#include "winburnmanager.h"

#include <QMap>
#include "winburnprovider.h"

#include <imapi2.h>
#include <comutil.h>
#include <winrt/Windows.Foundation.h>
#include <tlogger.h>

#include <statemanager.h>
#include <burnmanager.h>

namespace winrt {
    template<>
    inline bool is_guid_of<DDiscMaster2Events>(guid const& id) noexcept {
        return is_guid_of<DDiscMaster2Events, IDispatch>(id);
    }
}


struct DiscMasterEvents : winrt::implements<DiscMasterEvents, DDiscMaster2Events, winrt::non_agile> {
    DiscMasterEvents(WinBurnManager* parent) {
        this->parent = parent;
    }

    HRESULT __stdcall Invoke(
        [[maybe_unused]] DISPID dispIdMember,
        [[maybe_unused]] REFIID riid,
        [[maybe_unused]] LCID lcid,
        [[maybe_unused]] WORD wFlags,
        [[maybe_unused]] DISPPARAMS* pDispParams,
        [[maybe_unused]] VARIANT* pVarResult,
        [[maybe_unused]] EXCEPINFO* pExcepInfo,
        [[maybe_unused]] UINT* puArgErr) noexcept final {

        if (!pDispParams)
            return E_POINTER;

        if (pDispParams->cNamedArgs != 0)
            return DISP_E_NONAMEDARGS;

        HRESULT hr = S_OK;

        try {
            switch (dispIdMember) {
                case DISPID_DDISCMASTER2EVENTS_DEVICEADDED: {
                    Q_ASSERT(pDispParams->cArgs == 2);
                    Q_ASSERT(pDispParams->rgvarg[0].vt == VT_BSTR);
                    Q_ASSERT(pDispParams->rgvarg[1].vt == VT_DISPATCH);
                    NotifyDeviceAdded(pDispParams->rgvarg[1].pdispVal, pDispParams->rgvarg[0].bstrVal);
                    break;
                }
                case DISPID_DDISCMASTER2EVENTS_DEVICEREMOVED: {
                    Q_ASSERT(pDispParams->cArgs == 2);
                    Q_ASSERT(pDispParams->rgvarg[0].vt == VT_BSTR);
                    Q_ASSERT(pDispParams->rgvarg[1].vt == VT_DISPATCH);
                    NotifyDeviceRemoved(pDispParams->rgvarg[1].pdispVal, pDispParams->rgvarg[0].bstrVal);
                    break;
                }
                default: {
                    hr = DISP_E_MEMBERNOTFOUND;
                    break;
                }
            }

            return hr;
        } catch (...) {
            return winrt::to_hresult();
        }

        return hr;
    }

    HRESULT __stdcall GetTypeInfoCount([[maybe_unused]] UINT* pctinfo) noexcept final {
        return E_NOTIMPL;
    }

    HRESULT __stdcall GetTypeInfo(
        [[maybe_unused]] UINT iTInfo,
        [[maybe_unused]] LCID lcid,
        [[maybe_unused]] ITypeInfo** ppTInfo) noexcept final {
        return E_NOTIMPL;
    }

    HRESULT __stdcall GetIDsOfNames(
        [[maybe_unused]] REFIID riid,
        [[maybe_unused]] LPOLESTR* rgszNames,
        [[maybe_unused]] UINT cNames,
        [[maybe_unused]] LCID lcid,
        [[maybe_unused]] DISPID* rgDispId) noexcept final {
        return E_NOTIMPL;
    }

    HRESULT __stdcall NotifyDeviceAdded(
        IDispatch* object,
        BSTR uniqueId
    ) noexcept final {
        tDebug("DiscMasterEvents") << "Device added: " << reinterpret_cast<char*>(uniqueId);
        parent->registerBurnDevice(_bstr_t(uniqueId, true));
        return S_OK;
    }

    HRESULT __stdcall NotifyDeviceRemoved(
        IDispatch* object,
        BSTR uniqueId
    ) noexcept final {
        tDebug("DiscMasterEvents") << "Device removed: " << reinterpret_cast<char*>(uniqueId);

        //Don't use deregisterBurnDevice because the unique ID that is sent in is invalid by now
        parent->updateBurnDevices();
        return S_OK;
    }

    WinBurnManager* parent;
};

struct BurnManagerPrivate {
    winrt::com_ptr<IDiscMaster2> discMaster;
    QMap<_bstr_t, WinBurnProvider*> burnProviders;

    winrt::com_ptr<IConnectionPoint> connectionPoint;
    winrt::com_ptr<DiscMasterEvents> discMasterEvents;
    DWORD eventToken;
};

WinBurnManager::WinBurnManager(QObject* parent) : QObject(parent) {
    d = new BurnManagerPrivate();
    d->discMaster = winrt::create_instance<IDiscMaster2>(CLSID_MsftDiscMaster2);

    auto container = d->discMaster.as<IConnectionPointContainer>();
    winrt::check_hresult(container->FindConnectionPoint(winrt::guid_of<DDiscMaster2Events>(), d->connectionPoint.put()));
    d->discMasterEvents = winrt::make_self<DiscMasterEvents>(this);

    winrt::check_hresult(d->connectionPoint->Advise(d->discMasterEvents.get(), &d->eventToken));

    tDebug("WinBurnManager") << "Burn manager ready";

    updateBurnDevices();
}

WinBurnManager::~WinBurnManager() {
    winrt::check_hresult(d->connectionPoint->Unadvise(d->eventToken));
    delete d;
}

void WinBurnManager::updateBurnDevices() {
    LONG count;
    winrt::check_hresult(d->discMaster->get_Count(&count));

    QList<_bstr_t> availableDriveIds;
    for (LONG i = 0; i < count; i++) {
        BSTR id;
        winrt::check_hresult(d->discMaster->get_Item(i, &id));
        _bstr_t wrappedId(id, false);

        registerBurnDevice(wrappedId);
        availableDriveIds.append(wrappedId);
    }

    QList<_bstr_t> driveIdsToRemove;
    for (_bstr_t driveId : d->burnProviders.keys()) {
        if (!availableDriveIds.contains(driveId)) driveIdsToRemove.append(driveId);
    }

    for (_bstr_t driveId : driveIdsToRemove) {
        deregisterBurnDevice(driveId);
    }
}

void WinBurnManager::registerBurnDevice(_bstr_t device) {
    if (!d->burnProviders.contains(device)) {
        WinBurnProvider* burnProvider = new WinBurnProvider(device);
        d->burnProviders.insert(device, burnProvider);
    }
}

void WinBurnManager::deregisterBurnDevice(_bstr_t device) {
    if (d->burnProviders.contains(device)) {
        WinBurnProvider* burnProvider = d->burnProviders.take(device);
        StateManager::instance()->burn()->deregisterBackend(burnProvider);
        burnProvider->deleteLater();
    }
}
