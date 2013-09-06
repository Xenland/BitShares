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
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableView>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QTreeView>
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
    QAction *actionCreateMail;
    QAction *actionReply;
    QAction *actionReply_To_all;
    QAction *actionForward;
    QAction *actionUndo;
    QAction *actionRedo;
    QAction *actionCut;
    QAction *actionCopy;
    QAction *actionPaste;
    QAction *actionDelete_Message;
    QAction *actionFind;
    QWidget *centralWidget;
    QSplitter *treeStackSplitter;
    QTreeView *bitSharesTreeView;
    QStackedWidget *stackedWidget;
    QWidget *chatPage;
    QSplitter *chatSplitter;
    QListView *chatView;
    QPlainTextEdit *chatTextEdit;
    QWidget *mailPage;
    QSplitter *mailSplitter;
    QTableView *mailHeadersTable;
    QTextBrowser *mailPreview;
    QSplitter *splitter;
    QMenuBar *menuBar;
    QMenu *menuBitShares;
    QMenu *menuEdit;
    QMenu *menuMail;
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
        actionCreateMail = new QAction(BitSharesMainWindow);
        actionCreateMail->setObjectName(QStringLiteral("actionCreateMail"));
        QIcon icon;
        icon.addFile(QStringLiteral(":/images/images/pencil_small.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionCreateMail->setIcon(icon);
        actionReply = new QAction(BitSharesMainWindow);
        actionReply->setObjectName(QStringLiteral("actionReply"));
        actionReply_To_all = new QAction(BitSharesMainWindow);
        actionReply_To_all->setObjectName(QStringLiteral("actionReply_To_all"));
        actionForward = new QAction(BitSharesMainWindow);
        actionForward->setObjectName(QStringLiteral("actionForward"));
        actionUndo = new QAction(BitSharesMainWindow);
        actionUndo->setObjectName(QStringLiteral("actionUndo"));
        actionRedo = new QAction(BitSharesMainWindow);
        actionRedo->setObjectName(QStringLiteral("actionRedo"));
        actionCut = new QAction(BitSharesMainWindow);
        actionCut->setObjectName(QStringLiteral("actionCut"));
        actionCopy = new QAction(BitSharesMainWindow);
        actionCopy->setObjectName(QStringLiteral("actionCopy"));
        actionPaste = new QAction(BitSharesMainWindow);
        actionPaste->setObjectName(QStringLiteral("actionPaste"));
        actionDelete_Message = new QAction(BitSharesMainWindow);
        actionDelete_Message->setObjectName(QStringLiteral("actionDelete_Message"));
        actionFind = new QAction(BitSharesMainWindow);
        actionFind->setObjectName(QStringLiteral("actionFind"));
        centralWidget = new QWidget(BitSharesMainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        treeStackSplitter = new QSplitter(centralWidget);
        treeStackSplitter->setObjectName(QStringLiteral("treeStackSplitter"));
        treeStackSplitter->setGeometry(QRect(9, 9, 611, 511));
        treeStackSplitter->setOrientation(Qt::Horizontal);
        bitSharesTreeView = new QTreeView(treeStackSplitter);
        bitSharesTreeView->setObjectName(QStringLiteral("bitSharesTreeView"));
        treeStackSplitter->addWidget(bitSharesTreeView);
        stackedWidget = new QStackedWidget(treeStackSplitter);
        stackedWidget->setObjectName(QStringLiteral("stackedWidget"));
        QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(stackedWidget->sizePolicy().hasHeightForWidth());
        stackedWidget->setSizePolicy(sizePolicy);
        stackedWidget->setMinimumSize(QSize(100, 0));
        chatPage = new QWidget();
        chatPage->setObjectName(QStringLiteral("chatPage"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(chatPage->sizePolicy().hasHeightForWidth());
        chatPage->setSizePolicy(sizePolicy1);
        chatSplitter = new QSplitter(chatPage);
        chatSplitter->setObjectName(QStringLiteral("chatSplitter"));
        chatSplitter->setGeometry(QRect(-30, 20, 256, 384));
        sizePolicy1.setHeightForWidth(chatSplitter->sizePolicy().hasHeightForWidth());
        chatSplitter->setSizePolicy(sizePolicy1);
        chatSplitter->setOrientation(Qt::Vertical);
        chatView = new QListView(chatSplitter);
        chatView->setObjectName(QStringLiteral("chatView"));
        chatSplitter->addWidget(chatView);
        chatTextEdit = new QPlainTextEdit(chatSplitter);
        chatTextEdit->setObjectName(QStringLiteral("chatTextEdit"));
        chatSplitter->addWidget(chatTextEdit);
        stackedWidget->addWidget(chatPage);
        mailPage = new QWidget();
        mailPage->setObjectName(QStringLiteral("mailPage"));
        sizePolicy1.setHeightForWidth(mailPage->sizePolicy().hasHeightForWidth());
        mailPage->setSizePolicy(sizePolicy1);
        mailSplitter = new QSplitter(mailPage);
        mailSplitter->setObjectName(QStringLiteral("mailSplitter"));
        mailSplitter->setGeometry(QRect(10, 20, 225, 511));
        sizePolicy1.setHeightForWidth(mailSplitter->sizePolicy().hasHeightForWidth());
        mailSplitter->setSizePolicy(sizePolicy1);
        mailSplitter->setOrientation(Qt::Vertical);
        mailHeadersTable = new QTableView(mailSplitter);
        mailHeadersTable->setObjectName(QStringLiteral("mailHeadersTable"));
        mailSplitter->addWidget(mailHeadersTable);
        mailPreview = new QTextBrowser(mailSplitter);
        mailPreview->setObjectName(QStringLiteral("mailPreview"));
        mailSplitter->addWidget(mailPreview);
        stackedWidget->addWidget(mailPage);
        treeStackSplitter->addWidget(stackedWidget);
        splitter = new QSplitter(centralWidget);
        splitter->setObjectName(QStringLiteral("splitter"));
        splitter->setGeometry(QRect(0, 0, 0, 0));
        splitter->setOrientation(Qt::Vertical);
        BitSharesMainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(BitSharesMainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 667, 21));
        menuBitShares = new QMenu(menuBar);
        menuBitShares->setObjectName(QStringLiteral("menuBitShares"));
        menuEdit = new QMenu(menuBar);
        menuEdit->setObjectName(QStringLiteral("menuEdit"));
        menuMail = new QMenu(menuBar);
        menuMail->setObjectName(QStringLiteral("menuMail"));
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
        menuBar->addAction(menuEdit->menuAction());
        menuBar->addAction(menuMail->menuAction());
        menuBar->addAction(menuHelp->menuAction());
        menuBitShares->addAction(actionNew_Profile);
        menuBitShares->addAction(actionOpen_Profile);
        menuBitShares->addAction(actionExit);
        menuBitShares->addAction(actionRecent_Profiles);
        menuEdit->addAction(actionUndo);
        menuEdit->addAction(actionRedo);
        menuEdit->addSeparator();
        menuEdit->addAction(actionCut);
        menuEdit->addAction(actionCopy);
        menuEdit->addAction(actionPaste);
        menuEdit->addAction(actionDelete_Message);
        menuEdit->addSeparator();
        menuEdit->addAction(actionFind);
        menuMail->addAction(actionCreateMail);
        menuMail->addAction(actionReply);
        menuMail->addAction(actionReply_To_all);
        menuMail->addAction(actionForward);
        menuHelp->addAction(actionAbout);
        mainToolBar->addAction(actionCreateMail);

        retranslateUi(BitSharesMainWindow);

        stackedWidget->setCurrentIndex(0);


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
        actionCreateMail->setText(QApplication::translate("BitSharesMainWindow", "New Mail Message", 0));
        actionCreateMail->setShortcut(QApplication::translate("BitSharesMainWindow", "Ctrl+N", 0));
        actionReply->setText(QApplication::translate("BitSharesMainWindow", "Reply", 0));
        actionReply->setShortcut(QApplication::translate("BitSharesMainWindow", "Ctrl+R", 0));
        actionReply_To_all->setText(QApplication::translate("BitSharesMainWindow", "Reply to all", 0));
        actionReply_To_all->setShortcut(QApplication::translate("BitSharesMainWindow", "Ctrl+Shift+R", 0));
        actionForward->setText(QApplication::translate("BitSharesMainWindow", "Forward", 0));
        actionUndo->setText(QApplication::translate("BitSharesMainWindow", "Undo", 0));
        actionUndo->setShortcut(QApplication::translate("BitSharesMainWindow", "Ctrl+Z", 0));
        actionRedo->setText(QApplication::translate("BitSharesMainWindow", "Redo", 0));
        actionRedo->setShortcut(QApplication::translate("BitSharesMainWindow", "Ctrl+Y", 0));
        actionCut->setText(QApplication::translate("BitSharesMainWindow", "Cut", 0));
        actionCut->setShortcut(QApplication::translate("BitSharesMainWindow", "Ctrl+X", 0));
        actionCopy->setText(QApplication::translate("BitSharesMainWindow", "Copy", 0));
        actionCopy->setShortcut(QApplication::translate("BitSharesMainWindow", "Ctrl+C", 0));
        actionPaste->setText(QApplication::translate("BitSharesMainWindow", "Paste", 0));
        actionPaste->setShortcut(QApplication::translate("BitSharesMainWindow", "Ctrl+V", 0));
        actionDelete_Message->setText(QApplication::translate("BitSharesMainWindow", "Delete Message", 0));
        actionDelete_Message->setShortcut(QApplication::translate("BitSharesMainWindow", "Del", 0));
        actionFind->setText(QApplication::translate("BitSharesMainWindow", "Find", 0));
        actionFind->setShortcut(QApplication::translate("BitSharesMainWindow", "Ctrl+F", 0));
        menuBitShares->setTitle(QApplication::translate("BitSharesMainWindow", "File", 0));
        menuEdit->setTitle(QApplication::translate("BitSharesMainWindow", "Edit", 0));
        menuMail->setTitle(QApplication::translate("BitSharesMainWindow", "Mail", 0));
        menuHelp->setTitle(QApplication::translate("BitSharesMainWindow", "Help", 0));
    } // retranslateUi

};

namespace Ui {
    class BitSharesMainWindow: public Ui_BitSharesMainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_BITSHARESMAINWINDOW_H
