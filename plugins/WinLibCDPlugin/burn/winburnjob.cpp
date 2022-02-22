#include "winburnjob.h"

#include "winburnjobwidget.h"

#include <tpromise.h>
#include <imapi2.h>
#include <comutil.h>
#include <comdef.h>
#include <winrt/Windows.Foundation.h>
#include <tnotification.h>
#include <tlogger.h>
#include <Shlwapi.h>
#include <shcore.h>
#include <winrt/Windows.Storage.Streams.h>
#include <Windows.Storage.Streams.h>

#include <QAudioDecoder>
#include "winburndaoimage.h"

namespace winrt {
    using namespace winrt::Windows::Storage::Streams;
    using namespace winrt::Windows::Storage;

    template<>
    inline bool is_guid_of<DDiscFormat2RawCDEvents>(guid const& id) noexcept {
        return is_guid_of<DDiscFormat2RawCDEvents, IDispatch>(id);
    }
}

namespace abi {
    using namespace ABI::Windows::Storage::Streams;
}

struct DAOBurnEvents : winrt::implements<DAOBurnEvents, DDiscFormat2RawCDEvents, winrt::non_agile> {
    DAOBurnEvents(WinBurnJob* parent) {
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
                case DISPID_DDISCFORMAT2RAWCDEVENTS_UPDATE: {
                    Q_ASSERT(pDispParams->cArgs == 2);
                    Q_ASSERT(pDispParams->rgvarg[0].vt == VT_DISPATCH);
                    Q_ASSERT(pDispParams->rgvarg[1].vt == VT_DISPATCH);
                    Update(pDispParams->rgvarg[1].pdispVal, pDispParams->rgvarg[0].pdispVal);
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

    HRESULT __stdcall Update(
        IDispatch* object,
        IDispatch* progress) {
        Q_UNUSED(object);

        parent->notifyUpdate(progress);

        return S_OK;
    }

    WinBurnJob* parent;
};

struct WinBurnJobPrivate {
    WinBurnDaoImagePtr daoImage;
    _bstr_t driveId;
    QString albumTitle;

    QString title;
    QString description;
    quint64 progress = 0;
    quint64 totalProgress = 0;
    tJob::State state;

    QList<qint64> trackOffsets;
    qint64 leadout;

    winrt::com_ptr<IDiscMaster2> discMaster;

    winrt::com_ptr<IConnectionPoint> connectionPoint;
    winrt::agile_ref<DDiscFormat2RawCDEvents> burnEvents;
    DWORD eventToken;
};

WinBurnJob::WinBurnJob(WinBurnDaoImagePtr daoImage, _bstr_t driveId, QString albumTitle, QObject* parent) : tJob(parent) {
    d = new WinBurnJobPrivate();
    d->daoImage = daoImage;
    d->driveId = driveId;
    d->albumTitle = albumTitle;

    d->burnEvents = winrt::agile_ref<DDiscFormat2RawCDEvents> {winrt::make_self<DAOBurnEvents>(this)};

    d->title = tr("Burn %1").arg(QLocale().quoteString(albumTitle));
    emit titleChanged(d->title);

    d->description = tr("Preparing to burn...");
    emit descriptionChanged(d->description);

    run();
}

WinBurnJob::~WinBurnJob() {
    winrt::check_hresult(d->connectionPoint->Unadvise(d->eventToken));
    delete d;
}

void WinBurnJob::run() {
    TPROMISE_CREATE_NEW_THREAD(void, {
        try {
            auto discMaster = winrt::create_instance<IDiscMaster2>(CLSID_MsftDiscMaster2);
            auto discRecorder = winrt::create_instance<IDiscRecorder2>(CLSID_MsftDiscRecorder2);
            winrt::check_hresult(discRecorder->InitializeDiscRecorder(d->driveId.GetBSTR()));

            auto discFormatDAO = winrt::create_instance<IDiscFormat2RawCD>(CLSID_MsftDiscFormat2RawCD);
            discFormatDAO->put_ClientName(_bstr_t("theBeat"));
            winrt::check_hresult(discFormatDAO->put_Recorder(discRecorder.get()));

            auto container = discFormatDAO.as<IConnectionPointContainer>();
            winrt::check_hresult(container->FindConnectionPoint(winrt::guid_of<DDiscFormat2RawCDEvents>(), d->connectionPoint.put()));
            winrt::check_hresult(d->connectionPoint->Advise(d->burnEvents.get().get(), &d->eventToken));

            tDebug("WinBurnJob") << "Preparing media for burn";
            winrt::check_hresult(discFormatDAO->PrepareMedia());
            winrt::check_hresult(discFormatDAO->put_BufferUnderrunFreeDisabled(true));

            tDebug("WinBurnJob") << "Burning disc";
            winrt::com_ptr<IStream> daoImageStream;
            winrt::check_hresult(d->daoImage->daoImage()->CreateResultImage(daoImageStream.put()));
            winrt::check_hresult(discFormatDAO->WriteMedia(daoImageStream.get()));

            tDebug("WinBurnJob") << "Fixating media";
            winrt::check_hresult(discFormatDAO->ReleaseMedia());

            res();
        } catch (...) {
            _com_error err(winrt::to_hresult());
            LPCTSTR errMsg = err.ErrorMessage();
            rej(QString::fromWCharArray(errMsg));
        }
    })->then([ = ] {
        d->state = tJob::Finished;
        emit stateChanged(d->state);

        tDebug("WinBurnJob") << "Burn job complete";

        d->description = tr("Burn Successful");
        emit descriptionChanged(d->description);

        d->progress = 1;
        emit progressChanged(d->progress);
        d->totalProgress = 1;
        emit totalProgressChanged(d->totalProgress);

        tNotification* notification = new tNotification(tr("Burn Successful"), tr("Burned %1 to disc").arg(QLocale().quoteString(d->albumTitle)));
        notification->post();
    })->error([ = ](QString errorText) {
        d->state = tJob::Failed;
        emit stateChanged(d->state);

        tWarn("WinBurnJob") << "Burn job threw exception:";
        tWarn("WinBurnJob") << errorText;

        d->description = tr("Burn Failure: %1").arg(errorText);
        emit descriptionChanged(d->description);

        d->progress = 1;
        emit progressChanged(d->progress);
        d->totalProgress = 1;
        emit totalProgressChanged(d->totalProgress);

        tNotification* notification = new tNotification(tr("Burn Failure"), tr("Failed to burn %1 to disc").arg(QLocale().quoteString(d->albumTitle)));
        notification->post();
    });
}

QString WinBurnJob::title() {
    return d->title;
}

QString WinBurnJob::description() {
    return d->description;
}

void WinBurnJob::notifyUpdate(IDispatch* progress) {
    winrt::com_ptr<IDispatch> progressPtr;
    progressPtr.copy_from(progress);

    try {
        auto eventArgs = progressPtr.as<IDiscFormat2RawCDEventArgs>();

        IMAPI_FORMAT2_RAW_CD_WRITE_ACTION currentAction;
        winrt::check_hresult(eventArgs->get_CurrentAction(&currentAction));

        switch (currentAction) {
            case IMAPI_FORMAT2_RAW_CD_WRITE_ACTION_PREPARING:
                d->description = tr("Preparing to burn...");
                emit descriptionChanged(d->description);

                d->progress = 0;
                emit progressChanged(d->progress);
                d->totalProgress = 0;
                emit totalProgressChanged(d->totalProgress);
                break;
            case IMAPI_FORMAT2_RAW_CD_WRITE_ACTION_WRITING: {
                LONG startLba;
                eventArgs->get_StartLba(&startLba);

                LONG sectorCount;
                eventArgs->get_SectorCount(&sectorCount);

                LONG lastWriteLba;
                eventArgs->get_LastWrittenLba(&lastWriteLba);

                LONG remainingTime;
                eventArgs->get_RemainingTime(&remainingTime);

                QTime remainingDuration = QTime::fromMSecsSinceStartOfDay(0);
                remainingDuration.addSecs(remainingTime);

                d->progress = lastWriteLba - startLba;
                emit progressChanged(d->progress);

                d->totalProgress = sectorCount;
                emit totalProgressChanged(d->totalProgress);

                QStringList descriptionLines;

                //Figure out which track we are on
                int track = d->daoImage->trackNumberFromLba(lastWriteLba);
                if (track == -2) {
                    descriptionLines.append(tr("Finalising disc..."));
                } else if (track == -1) {
                    descriptionLines.append(tr("Preparing to burn..."));
                } else {
                    descriptionLines.append(tr("Burning track %1").arg(track));
                }

                descriptionLines.append(tr("About %1 remaining").arg(remainingDuration.toString("mm:ss")));

                d->description = descriptionLines.join("\n");
                emit descriptionChanged(d->description);
                break;
            }
            case IMAPI_FORMAT2_RAW_CD_WRITE_ACTION_FINISHING:
                d->description = tr("Finalising disc...");
                emit descriptionChanged(d->description);

                d->progress = 0;
                emit progressChanged(d->progress);

                d->totalProgress = 0;
                emit totalProgressChanged(d->totalProgress);
        }
    } catch (...) {
        tDebug("WinBurnJob") << "Caught exception in update handler";
        tDebug("WinBurnJob") << "HRESULT: " << winrt::to_hresult();
    }
}

quint64 WinBurnJob::progress() {
    return d->progress;
}

quint64 WinBurnJob::totalProgress() {
    return d->totalProgress;
}

tJob::State WinBurnJob::state() {
    return d->state;
}

QWidget* WinBurnJob::makeProgressWidget() {
    return new WinBurnJobWidget(this);
}
