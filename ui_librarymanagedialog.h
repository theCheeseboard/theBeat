/********************************************************************************
** Form generated from reading UI file 'librarymanagedialog.ui'
**
** Created by: Qt User Interface Compiler version 5.9.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LIBRARYMANAGEDIALOG_H
#define UI_LIBRARYMANAGEDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_LibraryManageDialog
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label_3;
    QListWidget *folders;
    QHBoxLayout *horizontalLayout;
    QPushButton *addFolderButton;
    QPushButton *removeFolderButton;
    QSpacerItem *horizontalSpacer;
    QPushButton *okButton;

    void setupUi(QDialog *LibraryManageDialog)
    {
        if (LibraryManageDialog->objectName().isEmpty())
            LibraryManageDialog->setObjectName(QStringLiteral("LibraryManageDialog"));
        LibraryManageDialog->resize(400, 300);
        verticalLayout = new QVBoxLayout(LibraryManageDialog);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        label_3 = new QLabel(LibraryManageDialog);
        label_3->setObjectName(QStringLiteral("label_3"));
        QFont font;
        font.setPointSize(20);
        label_3->setFont(font);

        verticalLayout->addWidget(label_3);

        folders = new QListWidget(LibraryManageDialog);
        folders->setObjectName(QStringLiteral("folders"));

        verticalLayout->addWidget(folders);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        addFolderButton = new QPushButton(LibraryManageDialog);
        addFolderButton->setObjectName(QStringLiteral("addFolderButton"));
        QIcon icon;
        QString iconThemeName = QStringLiteral("list-add");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon = QIcon::fromTheme(iconThemeName);
        } else {
            icon.addFile(QStringLiteral("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        addFolderButton->setIcon(icon);

        horizontalLayout->addWidget(addFolderButton);

        removeFolderButton = new QPushButton(LibraryManageDialog);
        removeFolderButton->setObjectName(QStringLiteral("removeFolderButton"));
        QIcon icon1;
        iconThemeName = QStringLiteral("list-remove");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon1 = QIcon::fromTheme(iconThemeName);
        } else {
            icon1.addFile(QStringLiteral("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        removeFolderButton->setIcon(icon1);

        horizontalLayout->addWidget(removeFolderButton);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        okButton = new QPushButton(LibraryManageDialog);
        okButton->setObjectName(QStringLiteral("okButton"));
        QIcon icon2;
        iconThemeName = QStringLiteral("dialog-ok");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon2 = QIcon::fromTheme(iconThemeName);
        } else {
            icon2.addFile(QStringLiteral("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        okButton->setIcon(icon2);

        horizontalLayout->addWidget(okButton);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(LibraryManageDialog);

        QMetaObject::connectSlotsByName(LibraryManageDialog);
    } // setupUi

    void retranslateUi(QDialog *LibraryManageDialog)
    {
        LibraryManageDialog->setWindowTitle(QApplication::translate("LibraryManageDialog", "Manage Library", Q_NULLPTR));
        label_3->setText(QApplication::translate("LibraryManageDialog", "Library", Q_NULLPTR));
        addFolderButton->setText(QApplication::translate("LibraryManageDialog", "Add Folder", Q_NULLPTR));
        removeFolderButton->setText(QApplication::translate("LibraryManageDialog", "Remove Folder", Q_NULLPTR));
        okButton->setText(QApplication::translate("LibraryManageDialog", "OK", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class LibraryManageDialog: public Ui_LibraryManageDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LIBRARYMANAGEDIALOG_H
