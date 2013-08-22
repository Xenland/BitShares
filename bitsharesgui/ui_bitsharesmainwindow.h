/********************************************************************************
** Form generated from reading UI file 'bitsharesmainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.1.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_BITSHARESMAINWINDOW_H
#define UI_BITSHARESMAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QListView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_BitSharesMainWindow
{
public:
    QAction *actionOpen_Profile;
    QAction *actionExit;
    QAction *actionNew_Profile;
    QAction *actionRecent_Profiles;
    QAction *actionAbout;
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout;
    QSplitter *mainWindowHorizontalSplitter;
    QTreeView *bitSharesTreeView;
    QSplitter *conversationSendMessageSplitter;
    QListView *conversationView;
    QPlainTextEdit *sendMessageTextEdit;
    QMenuBar *menuBar;
    QMenu *menuBitShares;
    QMenu *menuHelp;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *BitSharesMainWindow)
    {
        if (BitSharesMainWindow->objectName().isEmpty())
            BitSharesMainWindow->setObjectName(QStringLiteral("BitSharesMainWindow"));
        BitSharesMainWindow->resize(667, 692);
        actionOpen_Profile = new QAction(BitSharesMainWindow);
        actionOpen_Profile->setObjectName(QStringLiteral("actionOpen_Profile"));
        actionExit = new QAction(BitSharesMainWindow);
        actionExit->setObjectName(QStringLiteral("actionExit"));
        actionNew_Profile = new QAction(BitSharesMainWindow);
        actionNew_Profile->setObjectName(QStringLiteral("actionNew_Profile"));
        actionRecent_Profiles = new QAction(BitSharesMainWindow);
        actionRecent_Profiles->setObjectName(QStringLiteral("actionRecent_Profiles"));
        actionAbout = new QAction(BitSharesMainWindow);
        actionAbout->setObjectName(QStringLiteral("actionAbout"));
        centralWidget = new QWidget(BitSharesMainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        mainWindowHorizontalSplitter = new QSplitter(centralWidget);
        mainWindowHorizontalSplitter->setObjectName(QStringLiteral("mainWindowHorizontalSplitter"));
        mainWindowHorizontalSplitter->setOrientation(Qt::Horizontal);
        bitSharesTreeView = new QTreeView(mainWindowHorizontalSplitter);
        bitSharesTreeView->setObjectName(QStringLiteral("bitSharesTreeView"));
        mainWindowHorizontalSplitter->addWidget(bitSharesTreeView);
        conversationSendMessageSplitter = new QSplitter(mainWindowHorizontalSplitter);
        conversationSendMessageSplitter->setObjectName(QStringLiteral("conversationSendMessageSplitter"));
        conversationSendMessageSplitter->setOrientation(Qt::Vertical);
        conversationView = new QListView(conversationSendMessageSplitter);
        conversationView->setObjectName(QStringLiteral("conversationView"));
        conversationSendMessageSplitter->addWidget(conversationView);
        sendMessageTextEdit = new QPlainTextEdit(conversationSendMessageSplitter);
        sendMessageTextEdit->setObjectName(QStringLiteral("sendMessageTextEdit"));
        conversationSendMessageSplitter->addWidget(sendMessageTextEdit);
        mainWindowHorizontalSplitter->addWidget(conversationSendMessageSplitter);

        verticalLayout->addWidget(mainWindowHorizontalSplitter);

        BitSharesMainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(BitSharesMainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 667, 21));
        menuBitShares = new QMenu(menuBar);
        menuBitShares->setObjectName(QStringLiteral("menuBitShares"));
        menuHelp = new QMenu(menuBar);
        menuHelp->setObjectName(QStringLiteral("menuHelp"));
        BitSharesMainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(BitSharesMainWindow);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        BitSharesMainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(BitSharesMainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        BitSharesMainWindow->setStatusBar(statusBar);

        menuBar->addAction(menuBitShares->menuAction());
        menuBar->addAction(menuHelp->menuAction());
        menuBitShares->addAction(actionNew_Profile);
        menuBitShares->addAction(actionOpen_Profile);
        menuBitShares->addAction(actionExit);
        menuBitShares->addAction(actionRecent_Profiles);
        menuHelp->addAction(actionAbout);

        retranslateUi(BitSharesMainWindow);

        QMetaObject::connectSlotsByName(BitSharesMainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *BitSharesMainWindow)
    {
        BitSharesMainWindow->setWindowTitle(QApplication::translate("BitSharesMainWindow", "MainWindow", 0));
        actionOpen_Profile->setText(QApplication::translate("BitSharesMainWindow", "Open Profile...", 0));
        actionExit->setText(QApplication::translate("BitSharesMainWindow", "Exit", 0));
        actionNew_Profile->setText(QApplication::translate("BitSharesMainWindow", "New Profile...", 0));
        actionRecent_Profiles->setText(QApplication::translate("BitSharesMainWindow", "Recent Profiles", 0));
        actionAbout->setText(QApplication::translate("BitSharesMainWindow", "About", 0));
        menuBitShares->setTitle(QApplication::translate("BitSharesMainWindow", "File", 0));
        menuHelp->setTitle(QApplication::translate("BitSharesMainWindow", "Help", 0));
    } // retranslateUi

};

namespace Ui {
    class BitSharesMainWindow: public Ui_BitSharesMainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_BITSHARESMAINWINDOW_H
