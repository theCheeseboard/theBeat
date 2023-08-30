#ifndef LASTFMLOGINPOPOVER_H
#define LASTFMLOGINPOPOVER_H

#include <QCoroTask>
#include <QWidget>

namespace Ui {
    class LastFmLoginPopover;
}

struct LastFmLoginPopoverPrivate;
class LastFmLoginPopover : public QWidget {
        Q_OBJECT

    public:
        explicit LastFmLoginPopover(QWidget* parent = nullptr);
        ~LastFmLoginPopover();

    signals:
        void done();

    private slots:
        void on_errorOkButton_clicked();

        void on_titleLabel_2_backButtonClicked();

        void on_titleLabel_backButtonClicked();

        void on_loginButton_clicked();

        void on_loginLabel_linkActivated(const QString& link);

    private:
        Ui::LastFmLoginPopover* ui;
        LastFmLoginPopoverPrivate* d;

        QCoro::Task<> acquireToken();
        QCoro::Task<> checkAuth();
};

#endif // LASTFMLOGINPOPOVER_H
