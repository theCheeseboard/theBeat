#ifndef MAINWINDOWTOUCHBAR_H
#define MAINWINDOWTOUCHBAR_H

#include <QObject>

struct MainWindowTouchBarPrivate;
class MainWindowTouchBar : public QObject
{
    Q_OBJECT
    public:
        explicit MainWindowTouchBar(QWidget *parent = nullptr);
        ~MainWindowTouchBar();

    signals:

    private:
        MainWindowTouchBarPrivate* d;

        void setupTouchBar();

};

#endif // MAINWINDOWTOUCHBAR_H
