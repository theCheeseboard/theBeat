/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2022 Victor Tran
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
#ifndef ABSTRACTBURNJOB_H
#define ABSTRACTBURNJOB_H

#include <QCoroTask>
#include <tjob.h>

class AbstractBurnJob : public tJob {
        Q_OBJECT
    public:
        explicit AbstractBurnJob(QObject* parent = nullptr);

        virtual QCoro::Task<> start() = 0;
        virtual QString description() = 0;
        virtual bool canCancel() = 0;
        virtual void cancel() = 0;

        QWidget* makeProgressWidget();

    signals:
        void descriptionChanged(QString description);
        void canCancelChanged(bool canCancel);
};

#endif // ABSTRACTBURNJOB_H
