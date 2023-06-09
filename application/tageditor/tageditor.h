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
#ifndef TAGEDITOR_H
#define TAGEDITOR_H

#include <QWidget>

namespace Ui {
    class TagEditor;
}

struct TagEditorPrivate;
class TagEditor : public QWidget {
        Q_OBJECT

    public:
        explicit TagEditor(QString track, QWidget* parent = nullptr);
        ~TagEditor();

    signals:
        void done();

    private slots:
        void on_titleLabel_backButtonClicked();

        void on_titleEdit_textEdited(const QString& arg1);

        void on_artistEdit_textEdited(const QString& arg1);

        void on_albumEdit_textEdited(const QString& arg1);

        void on_trackBox_valueChanged(int arg1);

        void on_discBox_valueChanged(int arg1);

        void on_yearBox_valueChanged(int arg1);

        void on_revertButton_clicked();

        void on_saveButton_clicked();

        void on_genreEdit_textEdited(const QString& arg1);

    private:
        Ui::TagEditor* ui;
        TagEditorPrivate* d;

        void loadTag();
        void markDirty();
        void revert();
};

#endif // TAGEDITOR_H
