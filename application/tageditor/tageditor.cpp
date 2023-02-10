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
#include "tageditor.h"
#include "ui_tageditor.h"

#include "common.h"
#include "library/librarymanager.h"
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <terrorflash.h>
#include <tpopover.h>
#include <tscrim.h>

struct TagEditorPrivate {
        QString track;
        TagLib::FileRef file;
        bool clean = true;

        QMetaObject::Connection scrimClickConnection;
};

TagEditor::TagEditor(QString track, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::TagEditor) {
    ui->setupUi(this);

    d = new TagEditorPrivate();
    d->track = track;

    loadTag();

    ui->titleLabel->setBackButtonShown(true);
}

TagEditor::~TagEditor() {
    delete d;
    delete ui;
}

void TagEditor::on_titleLabel_backButtonClicked() {
    if (!d->clean) {
        tErrorFlash::flashError(ui->dirtyWidget);
        return;
    }
    emit done();
}

void TagEditor::on_titleEdit_textEdited(const QString& arg1) {
    d->file.tag()->setTitle(TagLib::String(arg1.toUtf8().data(), TagLib::String::UTF8));
    markDirty();
}

void TagEditor::on_artistEdit_textEdited(const QString& arg1) {
    d->file.tag()->setArtist(TagLib::String(arg1.toUtf8().data(), TagLib::String::UTF8));
    markDirty();
}

void TagEditor::on_albumEdit_textEdited(const QString& arg1) {
    d->file.tag()->setAlbum(TagLib::String(arg1.toUtf8().data(), TagLib::String::UTF8));
    markDirty();
}

void TagEditor::loadTag() {
#ifdef Q_OS_WIN
    d->file = TagLib::FileRef(reinterpret_cast<const wchar_t*>(d->track.constData()));
#else
    d->file = TagLib::FileRef(d->track.toUtf8());
#endif

    QSignalBlocker blocker1(ui->trackBox);
    QSignalBlocker blocker2(ui->discBox);
    QSignalBlocker blocker3(ui->yearBox);

    ui->titleEdit->setText(QString::fromWCharArray(d->file.tag()->title().toCWString()));
    ui->artistEdit->setText(QString::fromWCharArray(d->file.tag()->artist().toCWString()));
    ui->albumEdit->setText(QString::fromWCharArray(d->file.tag()->album().toCWString()));
    ui->genreEdit->setText(QString::fromWCharArray(d->file.tag()->genre().toCWString()));
    ui->trackBox->setValue(d->file.tag()->track());
    ui->yearBox->setValue(d->file.tag()->year());

    ui->lengthLabel->setText(Common::durationToString(d->file.audioProperties()->lengthInMilliseconds()));
}

void TagEditor::markDirty() {
    ui->dirtyWidget->expand();

    tPopover* popover = tPopover::popoverForPopoverWidget(this);
    popover->setDismissable(false);

    if (popover->parentWidget()) {
        d->scrimClickConnection =
            connect(tScrim::scrimForWidget(popover->parentWidget()),
                &tScrim::scrimClicked, this,
                [this] {
            on_titleLabel_backButtonClicked();
            });
    }
    d->clean = false;
}

void TagEditor::revert() {
    // Close the file
    d->file = TagLib::FileRef();

    loadTag();
    ui->dirtyWidget->collapse();
    tPopover::popoverForPopoverWidget(this)->setDismissable(true);
    disconnect(d->scrimClickConnection);
    d->clean = true;
}

void TagEditor::on_trackBox_valueChanged(int arg1) {
    d->file.tag()->setTrack(arg1);
    markDirty();
}

void TagEditor::on_discBox_valueChanged(int arg1) {
    markDirty();
}

void TagEditor::on_yearBox_valueChanged(int arg1) {
    d->file.tag()->setYear(arg1);
    markDirty();
}

void TagEditor::on_revertButton_clicked() {
    revert();
}

void TagEditor::on_saveButton_clicked() {
    d->file.save();

    // Update the database
    LibraryManager::instance()->addTrack(d->track, true);
    revert();
}

void TagEditor::on_genreEdit_textEdited(const QString& arg1) {
    d->file.tag()->setGenre(arg1.toStdString());
    markDirty();
}
