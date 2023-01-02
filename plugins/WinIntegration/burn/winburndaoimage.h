#ifndef WINBURNDAOIMAGE_H
#define WINBURNDAOIMAGE_H

#include <QObject>

#include <imapi2.h>
#include <winrt/Windows.Foundation.h>
#include <tpromise.h>

struct WinBurnDaoImagePrivate;
class WinBurnDaoImage : public QObject {
        Q_OBJECT
    public:
        explicit WinBurnDaoImage(QObject* parent = nullptr);
        ~WinBurnDaoImage();

        tPromise<void>* createImageFromFiles(QStringList files);
        winrt::com_ptr<IRawCDImageCreator> daoImage();
        int trackNumberFromLba(qint64 lba);
        qint64 leadoutLba();

        void setAlbumName(QString albumName);

    signals:

    private:
        WinBurnDaoImagePrivate* d;
};

typedef QSharedPointer<WinBurnDaoImage> WinBurnDaoImagePtr;

#endif // WINBURNDAOIMAGE_H
