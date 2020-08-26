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
#ifndef IMPORTCDJOB_H
#define IMPORTCDJOB_H

#include <tjob.h>
#include "trackinfo.h"

struct ImportCdJobPrivate;
class ImportCdJob : public tJob {
        Q_OBJECT
    public:
        explicit ImportCdJob(QString blockDevice, QList<TrackInfoPtr> trackInfo, QString outputDir, int readSpeed, QObject* parent = nullptr);
        ~ImportCdJob();

        QString description();
        bool canCancel();
        void cancel();


    signals:
        void descriptionChanged(QString description);
        void canCancelChanged(bool canCancel);

    private:
        ImportCdJobPrivate* d;

        void performNextAction();
        void fail(QString description);

        // tJob interface
    public:
        quint64 progress();
        quint64 totalProgress();
        QWidget* makeProgressWidget();
        State state();
};

#endif // IMPORTCDJOB_H
