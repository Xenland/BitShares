#include "bitsharesmainwindow.h"
#include "ui_bitsharesmainwindow.h"
#include <QtCore>
#include <QtGui>
#include <qmessagebox.h>
#include "chatwidget.h"

const int VERSIONNUM = 1;

BitSharesMainWindow::BitSharesMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::BitSharesMainWindow)
{
    ui->setupUi(this);
    ui->bitSharesTreeView->setUniformRowHeights(true);
    ui->bitSharesTreeView->header()->hide();
    ui->bitSharesTreeView->setModel(&_bitSharesTreeModel);
    readSettings();
    //ui->stackedWidget

}

BitSharesMainWindow::~BitSharesMainWindow()
{
    delete ui;
}


void BitSharesMainWindow::readSettings()
{
    QSettings settings("Invictus Innovations","BitSharesGUI");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("state").toByteArray(),VERSIONNUM);
    ui->treeStackSplitter->restoreState(settings.value("treeStackSplitter").toByteArray());
    //read recent profiles
}

void BitSharesMainWindow::writeSettings()
{
    QSettings settings("Invictus Innovations","BitSharesGUI");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState(VERSIONNUM));
    settings.setValue("treeStackSplitter", ui->treeStackSplitter->saveState());
    //settings.setValue("recentProfiles", recentProfiles);
}

void BitSharesMainWindow::closeEvent(QCloseEvent *event)
{
    if (okToContinue()) 
        {
        writeSettings();
        event->accept();
        } 
    else
        {
        event->ignore();
        }
}


void BitSharesMainWindow::on_actionExit_triggered()
{
    close();
}

void BitSharesMainWindow::on_actionCreateMail_triggered()
{
    //QMessageBox::information(this,"Create Mail Message","Widget not implemented yet");
    //Allow multiple createMailWidgets to be created
    //QCreateMailWidget* createMailWidget = new QCreateMailWidget(this);
    //createMailWidget->show();
    ChatWidget* chatWidget = new ChatWidget(nullptr); //parent is nullptr to make modeless floating window
    chatWidget->setWindowTitle("Message Session");
    chatWidget->show();

}
