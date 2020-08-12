/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2020 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/
#ifndef BURNJOB_H
#define BURNJOB_H

#include <tjob.h>

struct BurnJobPrivate;
class BurnJob : public tJob {
        Q_OBJECT
    public:
        explicit BurnJob(QStringList files, QString blockDevice, QString albumTitle, QObject* parent = nullptr);
        ~BurnJob();

        QString description();
        bool canCancel();

        void cancel();

    signals:
        void descriptionChanged(QString description);
        void canCancelChanged(bool canCancel);

    private:
        BurnJobPrivate* d;

        void performNextAction();

        // tJob interface
    public:
        quint64 progress();
        quint64 totalProgress();
        State state();
        QWidget* makeProgressWidget();
};

#endif // BURNJOB_H
