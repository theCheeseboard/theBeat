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
#ifndef PRINTCONTROLLER_H
#define PRINTCONTROLLER_H

#include <QWidget>
#include <abstractlibrarybrowser.h>

class QPrinter;
struct PrintControllerPrivate;
class PrintController : public QObject {
        Q_OBJECT
    public:
        explicit PrintController(AbstractLibraryBrowser::ListInformation listInformation, QWidget* parent = nullptr);
        ~PrintController();

        static bool hasPrintersAvailable();

        void confirmAndPerformPrint();

    signals:

    private:
        PrintControllerPrivate* d;

        void paintPrinter(QPrinter* printer);
        void paintTrackListing(QPrinter* printer);
        void paintJewelCase(QPrinter* printer);
};

#endif // PRINTCONTROLLER_H
