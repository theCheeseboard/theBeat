#ifndef DAOFORMATLOCKER_H
#define DAOFORMATLOCKER_H

#include <imapi2.h>
#include <winrt/Windows.Foundation.h>

class DaoFormatLocker {
        winrt::com_ptr<IDiscFormat2RawCD> discFormatDAO;

    public:
        DaoFormatLocker(winrt::com_ptr<IDiscFormat2RawCD> discFormatDAO) {
            winrt::check_hresult(discFormatDAO->PrepareMedia());
            this->discFormatDAO = discFormatDAO;
        }

        ~DaoFormatLocker() {
            discFormatDAO->ReleaseMedia();
        }
};


#endif // DAOFORMATLOCKER_H
