#ifndef HEADERBACKGROUNDCONTROLLER_H
#define HEADERBACKGROUNDCONTROLLER_H

#include <QWidget>

struct HeaderBackgroundControllerPrivate;
class HeaderBackgroundController : public QObject {
        Q_OBJECT
    public:
        explicit HeaderBackgroundController(QWidget* parent = nullptr);
        ~HeaderBackgroundController();

        void setImage(QImage image);
        void setTopPadding(int topPadding);

    signals:

    private:
        HeaderBackgroundControllerPrivate* d;

        void updateMargins();

        // QObject interface
    public:
        bool eventFilter(QObject* watched, QEvent* event);
};

#endif // HEADERBACKGROUNDCONTROLLER_H
