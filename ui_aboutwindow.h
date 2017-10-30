/********************************************************************************
** Form generated from reading UI file 'aboutwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.9.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ABOUTWINDOW_H
#define UI_ABOUTWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_AboutWindow
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QLabel *label_2;
    QSpacerItem *verticalSpacer;
    QLabel *label_3;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *okButton;

    void setupUi(QDialog *AboutWindow)
    {
        if (AboutWindow->objectName().isEmpty())
            AboutWindow->setObjectName(QStringLiteral("AboutWindow"));
        AboutWindow->resize(400, 300);
        QIcon icon;
        icon.addFile(QStringLiteral(":/icons/icon.svg"), QSize(), QIcon::Normal, QIcon::Off);
        AboutWindow->setWindowIcon(icon);
        verticalLayout = new QVBoxLayout(AboutWindow);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        label = new QLabel(AboutWindow);
        label->setObjectName(QStringLiteral("label"));
        QFont font;
        font.setPointSize(20);
        label->setFont(font);

        verticalLayout->addWidget(label);

        label_2 = new QLabel(AboutWindow);
        label_2->setObjectName(QStringLiteral("label_2"));

        verticalLayout->addWidget(label_2);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        label_3 = new QLabel(AboutWindow);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setWordWrap(true);

        verticalLayout->addWidget(label_3);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        okButton = new QPushButton(AboutWindow);
        okButton->setObjectName(QStringLiteral("okButton"));
        QIcon icon1;
        QString iconThemeName = QStringLiteral("dialog-ok");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon1 = QIcon::fromTheme(iconThemeName);
        } else {
            icon1.addFile(QStringLiteral("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        okButton->setIcon(icon1);

        horizontalLayout->addWidget(okButton);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(AboutWindow);

        QMetaObject::connectSlotsByName(AboutWindow);
    } // setupUi

    void retranslateUi(QDialog *AboutWindow)
    {
        AboutWindow->setWindowTitle(QApplication::translate("AboutWindow", "About theBeat", Q_NULLPTR));
        label->setText(QApplication::translate("AboutWindow", "theBeat 1.0", Q_NULLPTR));
        label_2->setText(QApplication::translate("AboutWindow", "Audio Player", Q_NULLPTR));
        label_3->setText(QApplication::translate("AboutWindow", "Copyright (c) Victor Tran 2017. Licensed under the terms of the GNU General Public License, version 3 or later.", Q_NULLPTR));
        okButton->setText(QApplication::translate("AboutWindow", "OK", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class AboutWindow: public Ui_AboutWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ABOUTWINDOW_H
