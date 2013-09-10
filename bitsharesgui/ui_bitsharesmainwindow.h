/********************************************************************************
** Form generated from reading UI file 'bitsharesmainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.1.1
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
    QAction *actionAdd_Contact;
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout_3;
    QSplitter *treeStackSplitter;
    QTreeView *bitSharesTreeView;
    QStackedWidget *stackedWidget;
    QWidget *chatPage;
    QVBoxLayout *verticalLayout;
    QSplitter *chatSplitter;
    QListView *chatView;
    QPlainTextEdit *chatTextEdit;
    QWidget *mailPage;
    QVBoxLayout *verticalLayout_2;
    QSplitter *mailSplitter;
    QTableView *mailHeadersTable;
    QTextBrowser *mailPreview;
    QMenuBar *menuBar;
    QMenu *menuBitShares;
    QMenu *menuEdit;
    QMenu *menuMail;
    QMenu *menuHelp;
    QMenu *menuContacts;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *BitSharesMainWindow)
    {
        if (BitSharesMainWindow->objectName().isEmpty())
            BitSharesMainWindow->setObjectName(QStringLiteral("BitSharesMainWindow"));
        BitSharesMainWindow->resize(832, 708);
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
        actionAdd_Contact = new QAction(BitSharesMainWindow);
        actionAdd_Contact->setObjectName(QStringLiteral("actionAdd_Contact"));
        centralWidget = new QWidget(BitSharesMainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(centralWidget->sizePolicy().hasHeightForWidth());
        centralWidget->setSizePolicy(sizePolicy);
        verticalLayout_3 = new QVBoxLayout(centralWidget);
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        treeStackSplitter = new QSplitter(centralWidget);
        treeStackSplitter->setObjectName(QStringLiteral("treeStackSplitter"));
        treeStackSplitter->setOrientation(Qt::Horizontal);
        bitSharesTreeView = new QTreeView(treeStackSplitter);
        bitSharesTreeView->setObjectName(QStringLiteral("bitSharesTreeView"));
        QSizePolicy sizePolicy1(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(bitSharesTreeView->sizePolicy().hasHeightForWidth());
        bitSharesTreeView->setSizePolicy(sizePolicy1);
        treeStackSplitter->addWidget(bitSharesTreeView);
        stackedWidget = new QStackedWidget(treeStackSplitter);
        stackedWidget->setObjectName(QStringLiteral("stackedWidget"));
        sizePolicy.setHeightForWidth(stackedWidget->sizePolicy().hasHeightForWidth());
        stackedWidget->setSizePolicy(sizePolicy);
        stackedWidget->setMinimumSize(QSize(50, 0));
        chatPage = new QWidget();
        chatPage->setObjectName(QStringLiteral("chatPage"));
        sizePolicy.setHeightForWidth(chatPage->sizePolicy().hasHeightForWidth());
        chatPage->setSizePolicy(sizePolicy);
        verticalLayout = new QVBoxLayout(chatPage);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        chatSplitter = new QSplitter(chatPage);
        chatSplitter->setObjectName(QStringLiteral("chatSplitter"));
        sizePolicy.setHeightForWidth(chatSplitter->sizePolicy().hasHeightForWidth());
        chatSplitter->setSizePolicy(sizePolicy);
        chatSplitter->setOrientation(Qt::Vertical);
        chatView = new QListView(chatSplitter);
        chatView->setObjectName(QStringLiteral("chatView"));
        chatSplitter->addWidget(chatView);
        chatTextEdit = new QPlainTextEdit(chatSplitter);
        chatTextEdit->setObjectName(QStringLiteral("chatTextEdit"));
        chatSplitter->addWidget(chatTextEdit);

        verticalLayout->addWidget(chatSplitter);

        stackedWidget->addWidget(chatPage);
        mailPage = new QWidget();
        mailPage->setObjectName(QStringLiteral("mailPage"));
        sizePolicy.setHeightForWidth(mailPage->sizePolicy().hasHeightForWidth());
        mailPage->setSizePolicy(sizePolicy);
        verticalLayout_2 = new QVBoxLayout(mailPage);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        mailSplitter = new QSplitter(mailPage);
        mailSplitter->setObjectName(QStringLiteral("mailSplitter"));
        sizePolicy.setHeightForWidth(mailSplitter->sizePolicy().hasHeightForWidth());
        mailSplitter->setSizePolicy(sizePolicy);
        mailSplitter->setOrientation(Qt::Vertical);
        mailHeadersTable = new QTableView(mailSplitter);
        mailHeadersTable->setObjectName(QStringLiteral("mailHeadersTable"));
        mailSplitter->addWidget(mailHeadersTable);
        mailPreview = new QTextBrowser(mailSplitter);
        mailPreview->setObjectName(QStringLiteral("mailPreview"));
        mailSplitter->addWidget(mailPreview);

        verticalLayout_2->addWidget(mailSplitter);

        stackedWidget->addWidget(mailPage);
        treeStackSplitter->addWidget(stackedWidget);

        verticalLayout_3->addWidget(treeStackSplitter);

        BitSharesMainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(BitSharesMainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 832, 22));
        menuBitShares = new QMenu(menuBar);
        menuBitShares->setObjectName(QStringLiteral("menuBitShares"));
        menuEdit = new QMenu(menuBar);
        menuEdit->setObjectName(QStringLiteral("menuEdit"));
        menuMail = new QMenu(menuBar);
        menuMail->setObjectName(QStringLiteral("menuMail"));
        menuHelp = new QMenu(menuBar);
        menuHelp->setObjectName(QStringLiteral("menuHelp"));
        menuContacts = new QMenu(menuBar);
        menuContacts->setObjectName(QStringLiteral("menuContacts"));
        BitSharesMainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(BitSharesMainWindow);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        BitSharesMainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(BitSharesMainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        BitSharesMainWindow->setStatusBar(statusBar);

        menuBar->addAction(menuBitShares->menuAction());
        menuBar->addAction(menuEdit->menuAction());
        menuBar->addAction(menuContacts->menuAction());
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
        menuContacts->addAction(actionAdd_Contact);
        mainToolBar->addAction(actionCreateMail);

        retranslateUi(BitSharesMainWindow);

        stackedWidget->setCurrentIndex(1);


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
        actionAdd_Contact->setText(QApplication::translate("BitSharesMainWindow", "Add Contact", 0));
        menuBitShares->setTitle(QApplication::translate("BitSharesMainWindow", "File", 0));
        menuEdit->setTitle(QApplication::translate("BitSharesMainWindow", "Edit", 0));
        menuMail->setTitle(QApplication::translate("BitSharesMainWindow", "Mail", 0));
        menuHelp->setTitle(QApplication::translate("BitSharesMainWindow", "Help", 0));
        menuContacts->setTitle(QApplication::translate("BitSharesMainWindow", "Contacts", 0));
    } // retranslateUi

};

namespace Ui {
    class BitSharesMainWindow: public Ui_BitSharesMainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_BITSHARESMAINWINDOW_H
