/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.9.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListView>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <phonon/seekslider.h>
#include "playlistlistwidget.h"
#include "visualisationframe.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionOpen;
    QAction *actionExit;
    QAction *actionScope;
    QAction *actionLines;
    QAction *actionCircle;
    QAction *actionAbout;
    QAction *actionManage_Library;
    QAction *actionClear_Playlist;
    QAction *actionSave_Playlist;
    QAction *actionAdd_to_existing_playlist;
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout;
    QFrame *contentFrame;
    QHBoxLayout *horizontalLayout;
    QListWidget *sourcesList;
    QFrame *sourcesDivider;
    QStackedWidget *sourcesStack;
    QWidget *sourcesStackPage1_2;
    QVBoxLayout *verticalLayout_5;
    VisualisationFrame *visualisationFrame;
    QWidget *sourcesStackPage2_2;
    QVBoxLayout *verticalLayout_7;
    QLabel *label_5;
    QTableView *library;
    QWidget *page;
    QVBoxLayout *verticalLayout_8;
    QLabel *label_7;
    QListView *PlaylistsView;
    QWidget *sourcesStackPage3_2;
    QVBoxLayout *verticalLayout_6;
    QLabel *label_3;
    QLabel *label_4;
    QHBoxLayout *horizontalLayout_5;
    QSpacerItem *horizontalSpacer_3;
    QPushButton *OpenFileButton;
    QLabel *label_6;
    QSpacerItem *verticalSpacer_2;
    QWidget *sourcesStackPage4_2;
    QVBoxLayout *verticalLayout_4;
    QLabel *label;
    QLabel *label_2;
    QLineEdit *networkStreamURL;
    QHBoxLayout *horizontalLayout_4;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *AddNetworkStreamButton;
    QSpacerItem *verticalSpacer;
    QWidget *page_2;
    QVBoxLayout *verticalLayout_9;
    QLabel *label_8;
    QLabel *label_9;
    QListView *listView;
    QFrame *line_2;
    QFrame *playlistContainerMainFrame;
    QVBoxLayout *playlistContainerMain;
    PlaylistListWidget *playlistWidget;
    QHBoxLayout *playlistOptions;
    QSpacerItem *horizontalSpacer;
    QToolButton *playlistMenuButton;
    QFrame *currentMediaFrame;
    QVBoxLayout *verticalLayout_3;
    QFrame *musicDivider;
    QHBoxLayout *horizontalLayout_3;
    QLabel *albumArtLabel;
    QFrame *frame;
    QVBoxLayout *verticalLayout_2;
    QLabel *currentTitleLabel;
    QLabel *currentMetadataLabel;
    QPushButton *repeatButton;
    QPushButton *backButton;
    QPushButton *playButton;
    QPushButton *nextButton;
    QHBoxLayout *horizontalLayout_2;
    Phonon::SeekSlider *seeker;
    QFrame *playlistContainerUnderFrame;
    QVBoxLayout *playlistContainerUnder;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuHelp;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(695, 455);
        QIcon icon;
        icon.addFile(QStringLiteral(":/icons/icon.svg"), QSize(), QIcon::Normal, QIcon::Off);
        MainWindow->setWindowIcon(icon);
        actionOpen = new QAction(MainWindow);
        actionOpen->setObjectName(QStringLiteral("actionOpen"));
        QIcon icon1;
        QString iconThemeName = QStringLiteral("document-open");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon1 = QIcon::fromTheme(iconThemeName);
        } else {
            icon1.addFile(QStringLiteral("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        actionOpen->setIcon(icon1);
        actionExit = new QAction(MainWindow);
        actionExit->setObjectName(QStringLiteral("actionExit"));
        QIcon icon2;
        iconThemeName = QStringLiteral("application-exit");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon2 = QIcon::fromTheme(iconThemeName);
        } else {
            icon2.addFile(QStringLiteral("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        actionExit->setIcon(icon2);
        actionScope = new QAction(MainWindow);
        actionScope->setObjectName(QStringLiteral("actionScope"));
        actionScope->setCheckable(true);
        actionScope->setChecked(true);
        actionLines = new QAction(MainWindow);
        actionLines->setObjectName(QStringLiteral("actionLines"));
        actionLines->setCheckable(true);
        actionCircle = new QAction(MainWindow);
        actionCircle->setObjectName(QStringLiteral("actionCircle"));
        actionCircle->setCheckable(true);
        actionAbout = new QAction(MainWindow);
        actionAbout->setObjectName(QStringLiteral("actionAbout"));
        QIcon icon3;
        iconThemeName = QStringLiteral("help-about");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon3 = QIcon::fromTheme(iconThemeName);
        } else {
            icon3.addFile(QStringLiteral("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        actionAbout->setIcon(icon3);
        actionManage_Library = new QAction(MainWindow);
        actionManage_Library->setObjectName(QStringLiteral("actionManage_Library"));
        actionClear_Playlist = new QAction(MainWindow);
        actionClear_Playlist->setObjectName(QStringLiteral("actionClear_Playlist"));
        QIcon icon4(QIcon::fromTheme(QStringLiteral("edit-delete")));
        actionClear_Playlist->setIcon(icon4);
        actionSave_Playlist = new QAction(MainWindow);
        actionSave_Playlist->setObjectName(QStringLiteral("actionSave_Playlist"));
        QIcon icon5(QIcon::fromTheme(QStringLiteral("document-save")));
        actionSave_Playlist->setIcon(icon5);
        actionAdd_to_existing_playlist = new QAction(MainWindow);
        actionAdd_to_existing_playlist->setObjectName(QStringLiteral("actionAdd_to_existing_playlist"));
        QIcon icon6(QIcon::fromTheme(QStringLiteral("list-add")));
        actionAdd_to_existing_playlist->setIcon(icon6);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        contentFrame = new QFrame(centralWidget);
        contentFrame->setObjectName(QStringLiteral("contentFrame"));
        horizontalLayout = new QHBoxLayout(contentFrame);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 9, 0, 0);
        sourcesList = new QListWidget(contentFrame);
        QIcon icon7;
        iconThemeName = QStringLiteral("view-media-visualisation");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon7 = QIcon::fromTheme(iconThemeName);
        } else {
            icon7.addFile(QStringLiteral("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        QListWidgetItem *__qlistwidgetitem = new QListWidgetItem(sourcesList);
        __qlistwidgetitem->setIcon(icon7);
        QIcon icon8;
        iconThemeName = QStringLiteral("media-playback-start");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon8 = QIcon::fromTheme(iconThemeName);
        } else {
            icon8.addFile(QStringLiteral("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        QListWidgetItem *__qlistwidgetitem1 = new QListWidgetItem(sourcesList);
        __qlistwidgetitem1->setIcon(icon8);
        QIcon icon9;
        iconThemeName = QStringLiteral("view-media-playlist");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon9 = QIcon::fromTheme(iconThemeName);
        } else {
            icon9.addFile(QStringLiteral("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        QListWidgetItem *__qlistwidgetitem2 = new QListWidgetItem(sourcesList);
        __qlistwidgetitem2->setIcon(icon9);
        QListWidgetItem *__qlistwidgetitem3 = new QListWidgetItem(sourcesList);
        __qlistwidgetitem3->setIcon(icon1);
        QIcon icon10;
        iconThemeName = QStringLiteral("online");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon10 = QIcon::fromTheme(iconThemeName);
        } else {
            icon10.addFile(QStringLiteral("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        QListWidgetItem *__qlistwidgetitem4 = new QListWidgetItem(sourcesList);
        __qlistwidgetitem4->setIcon(icon10);
        QIcon icon11;
        iconThemeName = QStringLiteral("media-optical-audio");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon11 = QIcon::fromTheme(iconThemeName);
        } else {
            icon11.addFile(QStringLiteral("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        QListWidgetItem *__qlistwidgetitem5 = new QListWidgetItem(sourcesList);
        __qlistwidgetitem5->setIcon(icon11);
        sourcesList->setObjectName(QStringLiteral("sourcesList"));
        sourcesList->setMaximumSize(QSize(300, 16777215));
        sourcesList->setFrameShape(QFrame::NoFrame);
        sourcesList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        sourcesList->setIconSize(QSize(32, 32));

        horizontalLayout->addWidget(sourcesList);

        sourcesDivider = new QFrame(contentFrame);
        sourcesDivider->setObjectName(QStringLiteral("sourcesDivider"));
        sourcesDivider->setFrameShape(QFrame::VLine);
        sourcesDivider->setFrameShadow(QFrame::Sunken);

        horizontalLayout->addWidget(sourcesDivider);

        sourcesStack = new QStackedWidget(contentFrame);
        sourcesStack->setObjectName(QStringLiteral("sourcesStack"));
        sourcesStackPage1_2 = new QWidget();
        sourcesStackPage1_2->setObjectName(QStringLiteral("sourcesStackPage1_2"));
        verticalLayout_5 = new QVBoxLayout(sourcesStackPage1_2);
        verticalLayout_5->setSpacing(6);
        verticalLayout_5->setContentsMargins(11, 11, 11, 11);
        verticalLayout_5->setObjectName(QStringLiteral("verticalLayout_5"));
        verticalLayout_5->setContentsMargins(0, 0, 0, 0);
        visualisationFrame = new VisualisationFrame(sourcesStackPage1_2);
        visualisationFrame->setObjectName(QStringLiteral("visualisationFrame"));
        visualisationFrame->setContextMenuPolicy(Qt::CustomContextMenu);
        visualisationFrame->setFrameShape(QFrame::StyledPanel);
        visualisationFrame->setFrameShadow(QFrame::Raised);

        verticalLayout_5->addWidget(visualisationFrame);

        sourcesStack->addWidget(sourcesStackPage1_2);
        sourcesStackPage2_2 = new QWidget();
        sourcesStackPage2_2->setObjectName(QStringLiteral("sourcesStackPage2_2"));
        verticalLayout_7 = new QVBoxLayout(sourcesStackPage2_2);
        verticalLayout_7->setSpacing(6);
        verticalLayout_7->setContentsMargins(11, 11, 11, 11);
        verticalLayout_7->setObjectName(QStringLiteral("verticalLayout_7"));
        verticalLayout_7->setContentsMargins(0, 0, 0, 0);
        label_5 = new QLabel(sourcesStackPage2_2);
        label_5->setObjectName(QStringLiteral("label_5"));
        QFont font;
        font.setPointSize(20);
        label_5->setFont(font);
        label_5->setIndent(9);

        verticalLayout_7->addWidget(label_5);

        library = new QTableView(sourcesStackPage2_2);
        library->setObjectName(QStringLiteral("library"));
        library->setFrameShape(QFrame::NoFrame);
        library->setEditTriggers(QAbstractItemView::NoEditTriggers);
        library->setDragEnabled(true);
        library->setSelectionBehavior(QAbstractItemView::SelectRows);
        library->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        library->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
        library->setShowGrid(false);
        library->setGridStyle(Qt::SolidLine);
        library->horizontalHeader()->setDefaultSectionSize(200);
        library->horizontalHeader()->setProperty("showSortIndicator", QVariant(true));
        library->verticalHeader()->setVisible(false);

        verticalLayout_7->addWidget(library);

        sourcesStack->addWidget(sourcesStackPage2_2);
        page = new QWidget();
        page->setObjectName(QStringLiteral("page"));
        verticalLayout_8 = new QVBoxLayout(page);
        verticalLayout_8->setSpacing(6);
        verticalLayout_8->setContentsMargins(11, 11, 11, 11);
        verticalLayout_8->setObjectName(QStringLiteral("verticalLayout_8"));
        verticalLayout_8->setContentsMargins(0, 0, 0, 0);
        label_7 = new QLabel(page);
        label_7->setObjectName(QStringLiteral("label_7"));
        label_7->setFont(font);
        label_7->setIndent(9);

        verticalLayout_8->addWidget(label_7);

        PlaylistsView = new QListView(page);
        PlaylistsView->setObjectName(QStringLiteral("PlaylistsView"));
        PlaylistsView->setFrameShape(QFrame::NoFrame);

        verticalLayout_8->addWidget(PlaylistsView);

        sourcesStack->addWidget(page);
        sourcesStackPage3_2 = new QWidget();
        sourcesStackPage3_2->setObjectName(QStringLiteral("sourcesStackPage3_2"));
        verticalLayout_6 = new QVBoxLayout(sourcesStackPage3_2);
        verticalLayout_6->setSpacing(6);
        verticalLayout_6->setContentsMargins(11, 11, 11, 11);
        verticalLayout_6->setObjectName(QStringLiteral("verticalLayout_6"));
        verticalLayout_6->setContentsMargins(-1, 0, -1, 0);
        label_3 = new QLabel(sourcesStackPage3_2);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setFont(font);

        verticalLayout_6->addWidget(label_3);

        label_4 = new QLabel(sourcesStackPage3_2);
        label_4->setObjectName(QStringLiteral("label_4"));

        verticalLayout_6->addWidget(label_4);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setSpacing(6);
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_3);

        OpenFileButton = new QPushButton(sourcesStackPage3_2);
        OpenFileButton->setObjectName(QStringLiteral("OpenFileButton"));
        OpenFileButton->setIcon(icon1);

        horizontalLayout_5->addWidget(OpenFileButton);


        verticalLayout_6->addLayout(horizontalLayout_5);

        label_6 = new QLabel(sourcesStackPage3_2);
        label_6->setObjectName(QStringLiteral("label_6"));
        label_6->setWordWrap(true);

        verticalLayout_6->addWidget(label_6);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_6->addItem(verticalSpacer_2);

        sourcesStack->addWidget(sourcesStackPage3_2);
        sourcesStackPage4_2 = new QWidget();
        sourcesStackPage4_2->setObjectName(QStringLiteral("sourcesStackPage4_2"));
        verticalLayout_4 = new QVBoxLayout(sourcesStackPage4_2);
        verticalLayout_4->setSpacing(6);
        verticalLayout_4->setContentsMargins(11, 11, 11, 11);
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        verticalLayout_4->setContentsMargins(-1, 0, -1, 0);
        label = new QLabel(sourcesStackPage4_2);
        label->setObjectName(QStringLiteral("label"));
        label->setFont(font);

        verticalLayout_4->addWidget(label);

        label_2 = new QLabel(sourcesStackPage4_2);
        label_2->setObjectName(QStringLiteral("label_2"));

        verticalLayout_4->addWidget(label_2);

        networkStreamURL = new QLineEdit(sourcesStackPage4_2);
        networkStreamURL->setObjectName(QStringLiteral("networkStreamURL"));

        verticalLayout_4->addWidget(networkStreamURL);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_2);

        AddNetworkStreamButton = new QPushButton(sourcesStackPage4_2);
        AddNetworkStreamButton->setObjectName(QStringLiteral("AddNetworkStreamButton"));
        QIcon icon12;
        iconThemeName = QStringLiteral("list-add");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon12 = QIcon::fromTheme(iconThemeName);
        } else {
            icon12.addFile(QStringLiteral("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        AddNetworkStreamButton->setIcon(icon12);

        horizontalLayout_4->addWidget(AddNetworkStreamButton);


        verticalLayout_4->addLayout(horizontalLayout_4);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_4->addItem(verticalSpacer);

        sourcesStack->addWidget(sourcesStackPage4_2);
        page_2 = new QWidget();
        page_2->setObjectName(QStringLiteral("page_2"));
        verticalLayout_9 = new QVBoxLayout(page_2);
        verticalLayout_9->setSpacing(6);
        verticalLayout_9->setContentsMargins(11, 11, 11, 11);
        verticalLayout_9->setObjectName(QStringLiteral("verticalLayout_9"));
        verticalLayout_9->setContentsMargins(0, 0, 0, 0);
        label_8 = new QLabel(page_2);
        label_8->setObjectName(QStringLiteral("label_8"));
        label_8->setFont(font);
        label_8->setIndent(9);

        verticalLayout_9->addWidget(label_8);

        label_9 = new QLabel(page_2);
        label_9->setObjectName(QStringLiteral("label_9"));
        label_9->setIndent(9);

        verticalLayout_9->addWidget(label_9);

        listView = new QListView(page_2);
        listView->setObjectName(QStringLiteral("listView"));
        listView->setFrameShape(QFrame::NoFrame);

        verticalLayout_9->addWidget(listView);

        sourcesStack->addWidget(page_2);

        horizontalLayout->addWidget(sourcesStack);

        line_2 = new QFrame(contentFrame);
        line_2->setObjectName(QStringLiteral("line_2"));
        line_2->setFrameShape(QFrame::VLine);
        line_2->setFrameShadow(QFrame::Sunken);

        horizontalLayout->addWidget(line_2);

        playlistContainerMainFrame = new QFrame(contentFrame);
        playlistContainerMainFrame->setObjectName(QStringLiteral("playlistContainerMainFrame"));
        playlistContainerMainFrame->setMaximumSize(QSize(300, 16777215));
        playlistContainerMainFrame->setFrameShape(QFrame::NoFrame);
        playlistContainerMainFrame->setFrameShadow(QFrame::Raised);
        playlistContainerMain = new QVBoxLayout(playlistContainerMainFrame);
        playlistContainerMain->setSpacing(6);
        playlistContainerMain->setContentsMargins(11, 11, 11, 11);
        playlistContainerMain->setObjectName(QStringLiteral("playlistContainerMain"));
        playlistContainerMain->setContentsMargins(0, 0, 0, 0);
        playlistWidget = new PlaylistListWidget(playlistContainerMainFrame);
        playlistWidget->setObjectName(QStringLiteral("playlistWidget"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(playlistWidget->sizePolicy().hasHeightForWidth());
        playlistWidget->setSizePolicy(sizePolicy);
        playlistWidget->setAcceptDrops(true);
        playlistWidget->setFrameShape(QFrame::NoFrame);
        playlistWidget->setDragEnabled(true);
        playlistWidget->setDragDropMode(QAbstractItemView::DropOnly);
        playlistWidget->setSelectionMode(QAbstractItemView::SingleSelection);

        playlistContainerMain->addWidget(playlistWidget);

        playlistOptions = new QHBoxLayout();
        playlistOptions->setSpacing(6);
        playlistOptions->setObjectName(QStringLiteral("playlistOptions"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        playlistOptions->addItem(horizontalSpacer);

        playlistMenuButton = new QToolButton(playlistContainerMainFrame);
        playlistMenuButton->setObjectName(QStringLiteral("playlistMenuButton"));
        QIcon icon13(QIcon::fromTheme(QStringLiteral("application-menu")));
        playlistMenuButton->setIcon(icon13);
        playlistMenuButton->setPopupMode(QToolButton::InstantPopup);
        playlistMenuButton->setAutoRaise(true);

        playlistOptions->addWidget(playlistMenuButton);


        playlistContainerMain->addLayout(playlistOptions);


        horizontalLayout->addWidget(playlistContainerMainFrame);


        verticalLayout->addWidget(contentFrame);

        currentMediaFrame = new QFrame(centralWidget);
        currentMediaFrame->setObjectName(QStringLiteral("currentMediaFrame"));
        currentMediaFrame->setFrameShape(QFrame::NoFrame);
        currentMediaFrame->setFrameShadow(QFrame::Raised);
        verticalLayout_3 = new QVBoxLayout(currentMediaFrame);
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        musicDivider = new QFrame(currentMediaFrame);
        musicDivider->setObjectName(QStringLiteral("musicDivider"));
        musicDivider->setFrameShape(QFrame::HLine);
        musicDivider->setFrameShadow(QFrame::Sunken);

        verticalLayout_3->addWidget(musicDivider);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        albumArtLabel = new QLabel(currentMediaFrame);
        albumArtLabel->setObjectName(QStringLiteral("albumArtLabel"));
        albumArtLabel->setText(QStringLiteral("AlbumArt"));

        horizontalLayout_3->addWidget(albumArtLabel);

        frame = new QFrame(currentMediaFrame);
        frame->setObjectName(QStringLiteral("frame"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(frame->sizePolicy().hasHeightForWidth());
        frame->setSizePolicy(sizePolicy1);
        verticalLayout_2 = new QVBoxLayout(frame);
        verticalLayout_2->setSpacing(0);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        currentTitleLabel = new QLabel(frame);
        currentTitleLabel->setObjectName(QStringLiteral("currentTitleLabel"));
        QSizePolicy sizePolicy2(QSizePolicy::Ignored, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(currentTitleLabel->sizePolicy().hasHeightForWidth());
        currentTitleLabel->setSizePolicy(sizePolicy2);
        QFont font1;
        font1.setPointSize(15);
        currentTitleLabel->setFont(font1);
        currentTitleLabel->setText(QStringLiteral("Bit Quest"));

        verticalLayout_2->addWidget(currentTitleLabel);

        currentMetadataLabel = new QLabel(frame);
        currentMetadataLabel->setObjectName(QStringLiteral("currentMetadataLabel"));
        currentMetadataLabel->setEnabled(false);
        sizePolicy2.setHeightForWidth(currentMetadataLabel->sizePolicy().hasHeightForWidth());
        currentMetadataLabel->setSizePolicy(sizePolicy2);
        currentMetadataLabel->setText(QStringLiteral("Kevin MacLeod"));

        verticalLayout_2->addWidget(currentMetadataLabel);


        horizontalLayout_3->addWidget(frame);

        repeatButton = new QPushButton(currentMediaFrame);
        repeatButton->setObjectName(QStringLiteral("repeatButton"));
        QIcon icon14;
        iconThemeName = QStringLiteral("media-repeat-all");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon14 = QIcon::fromTheme(iconThemeName);
        } else {
            icon14.addFile(QStringLiteral("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        repeatButton->setIcon(icon14);
        repeatButton->setCheckable(true);
        repeatButton->setFlat(true);

        horizontalLayout_3->addWidget(repeatButton);

        backButton = new QPushButton(currentMediaFrame);
        backButton->setObjectName(QStringLiteral("backButton"));
        QIcon icon15;
        iconThemeName = QStringLiteral("media-skip-backward");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon15 = QIcon::fromTheme(iconThemeName);
        } else {
            icon15.addFile(QStringLiteral("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        backButton->setIcon(icon15);
        backButton->setFlat(true);

        horizontalLayout_3->addWidget(backButton);

        playButton = new QPushButton(currentMediaFrame);
        playButton->setObjectName(QStringLiteral("playButton"));
        playButton->setIcon(icon8);
        playButton->setIconSize(QSize(32, 32));
        playButton->setFlat(true);

        horizontalLayout_3->addWidget(playButton);

        nextButton = new QPushButton(currentMediaFrame);
        nextButton->setObjectName(QStringLiteral("nextButton"));
        QIcon icon16;
        iconThemeName = QStringLiteral("media-skip-forward");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon16 = QIcon::fromTheme(iconThemeName);
        } else {
            icon16.addFile(QStringLiteral("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        nextButton->setIcon(icon16);
        nextButton->setFlat(true);

        horizontalLayout_3->addWidget(nextButton);


        verticalLayout_3->addLayout(horizontalLayout_3);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        seeker = new Phonon::SeekSlider(currentMediaFrame);
        seeker->setObjectName(QStringLiteral("seeker"));
        seeker->setIconVisible(false);
        seeker->setTracking(true);

        horizontalLayout_2->addWidget(seeker);


        verticalLayout_3->addLayout(horizontalLayout_2);

        playlistContainerUnderFrame = new QFrame(currentMediaFrame);
        playlistContainerUnderFrame->setObjectName(QStringLiteral("playlistContainerUnderFrame"));
        QSizePolicy sizePolicy3(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(playlistContainerUnderFrame->sizePolicy().hasHeightForWidth());
        playlistContainerUnderFrame->setSizePolicy(sizePolicy3);
        playlistContainerUnderFrame->setFrameShape(QFrame::NoFrame);
        playlistContainerUnderFrame->setFrameShadow(QFrame::Raised);
        playlistContainerUnder = new QVBoxLayout(playlistContainerUnderFrame);
        playlistContainerUnder->setSpacing(6);
        playlistContainerUnder->setContentsMargins(11, 11, 11, 11);
        playlistContainerUnder->setObjectName(QStringLiteral("playlistContainerUnder"));
        playlistContainerUnder->setContentsMargins(0, 0, 0, 0);

        verticalLayout_3->addWidget(playlistContainerUnderFrame);


        verticalLayout->addWidget(currentMediaFrame);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 695, 22));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QStringLiteral("menuFile"));
        menuHelp = new QMenu(menuBar);
        menuHelp->setObjectName(QStringLiteral("menuHelp"));
        MainWindow->setMenuBar(menuBar);
#ifndef QT_NO_SHORTCUT
#endif // QT_NO_SHORTCUT

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuHelp->menuAction());
        menuFile->addAction(actionOpen);
        menuFile->addAction(actionManage_Library);
        menuFile->addAction(actionExit);
        menuHelp->addAction(actionAbout);

        retranslateUi(MainWindow);

        sourcesStack->setCurrentIndex(5);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "theBeat", Q_NULLPTR));
        actionOpen->setText(QApplication::translate("MainWindow", "&Open", Q_NULLPTR));
#ifndef QT_NO_SHORTCUT
        actionOpen->setShortcut(QApplication::translate("MainWindow", "Ctrl+O", Q_NULLPTR));
#endif // QT_NO_SHORTCUT
        actionExit->setText(QApplication::translate("MainWindow", "&Exit", Q_NULLPTR));
#ifndef QT_NO_SHORTCUT
        actionExit->setShortcut(QApplication::translate("MainWindow", "Ctrl+Q", Q_NULLPTR));
#endif // QT_NO_SHORTCUT
        actionScope->setText(QApplication::translate("MainWindow", "Scope", Q_NULLPTR));
        actionLines->setText(QApplication::translate("MainWindow", "Lines", Q_NULLPTR));
        actionCircle->setText(QApplication::translate("MainWindow", "Circle", Q_NULLPTR));
        actionAbout->setText(QApplication::translate("MainWindow", "&About", Q_NULLPTR));
        actionManage_Library->setText(QApplication::translate("MainWindow", "&Manage Library", Q_NULLPTR));
        actionClear_Playlist->setText(QApplication::translate("MainWindow", "Clear Playlist", Q_NULLPTR));
        actionSave_Playlist->setText(QApplication::translate("MainWindow", "Save Playlist", Q_NULLPTR));
        actionAdd_to_existing_playlist->setText(QApplication::translate("MainWindow", "Add to existing playlist", Q_NULLPTR));

        const bool __sortingEnabled = sourcesList->isSortingEnabled();
        sourcesList->setSortingEnabled(false);
        QListWidgetItem *___qlistwidgetitem = sourcesList->item(0);
        ___qlistwidgetitem->setText(QApplication::translate("MainWindow", "Visualiser", Q_NULLPTR));
        QListWidgetItem *___qlistwidgetitem1 = sourcesList->item(1);
        ___qlistwidgetitem1->setText(QApplication::translate("MainWindow", "Music Library", Q_NULLPTR));
        QListWidgetItem *___qlistwidgetitem2 = sourcesList->item(2);
        ___qlistwidgetitem2->setText(QApplication::translate("MainWindow", "Playlists", Q_NULLPTR));
        QListWidgetItem *___qlistwidgetitem3 = sourcesList->item(3);
        ___qlistwidgetitem3->setText(QApplication::translate("MainWindow", "Open File", Q_NULLPTR));
        QListWidgetItem *___qlistwidgetitem4 = sourcesList->item(4);
        ___qlistwidgetitem4->setText(QApplication::translate("MainWindow", "Open Network Stream", Q_NULLPTR));
        QListWidgetItem *___qlistwidgetitem5 = sourcesList->item(5);
        ___qlistwidgetitem5->setText(QApplication::translate("MainWindow", "Play CD", Q_NULLPTR));
        sourcesList->setSortingEnabled(__sortingEnabled);

        label_5->setText(QApplication::translate("MainWindow", "Media Library", Q_NULLPTR));
        label_7->setText(QApplication::translate("MainWindow", "Playlists", Q_NULLPTR));
        label_3->setText(QApplication::translate("MainWindow", "Open File", Q_NULLPTR));
        label_4->setText(QApplication::translate("MainWindow", "Open an audio file from your computer", Q_NULLPTR));
        OpenFileButton->setText(QApplication::translate("MainWindow", "Open", Q_NULLPTR));
        label_6->setText(QApplication::translate("MainWindow", "Alternatively, drag in music from all around into the playlist box on the right to add it to the playlist.", Q_NULLPTR));
        label->setText(QApplication::translate("MainWindow", "Play Network Stream", Q_NULLPTR));
        label_2->setText(QApplication::translate("MainWindow", "Enter the URL of the network stream to play", Q_NULLPTR));
        networkStreamURL->setPlaceholderText(QApplication::translate("MainWindow", "Network Stream URL", Q_NULLPTR));
        AddNetworkStreamButton->setText(QApplication::translate("MainWindow", "Add", Q_NULLPTR));
        label_8->setText(QApplication::translate("MainWindow", "Play CD", Q_NULLPTR));
        label_9->setText(QApplication::translate("MainWindow", "CD Playback is coming soon. Stay tuned!", Q_NULLPTR));
        playlistMenuButton->setText(QString());
        repeatButton->setText(QString());
        backButton->setText(QString());
        playButton->setText(QString());
        nextButton->setText(QString());
        menuFile->setTitle(QApplication::translate("MainWindow", "Fi&le", Q_NULLPTR));
        menuHelp->setTitle(QApplication::translate("MainWindow", "Help", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
