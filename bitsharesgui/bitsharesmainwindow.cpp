#include "bitsharesmainwindow.h"
#include "ui_bitsharesmainwindow.h"
#include <qmessagebox.h>
#include "chatwidget.h"
#include <QSettings>


BitSharesMainWindow::BitSharesMainWindow(QWidget *parent) :
    SelfSizingMainWindow(parent),
    ui(new Ui::BitSharesMainWindow)
{
    ui->setupUi(this);
    ui->bitSharesTreeView->setUniformRowHeights(true);
    ui->bitSharesTreeView->header()->hide();
    ui->bitSharesTreeView->setModel(&_bitSharesTreeModel);
    setWindowTitle("BitShare GUI");
    readSettings();
    //ui->stackedWidget

}

BitSharesMainWindow::~BitSharesMainWindow()
{
    delete ui;
}


void BitSharesMainWindow::readSettings()
{
    SelfSizingMainWindow::readSettings();
    QSettings settings("Invictus Innovations","BitSharesGUI");
    ui->treeStackSplitter->restoreState(settings.value("treeStackSplitter").toByteArray());
    //read recent profiles
}

void BitSharesMainWindow::writeSettings()
{
    SelfSizingMainWindow::writeSettings();
    QSettings settings("Invictus Innovations","BitSharesGUI");
    settings.beginGroup(windowTitle());
    settings.setValue("treeStackSplitter", ui->treeStackSplitter->saveState());
    settings.endGroup();
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
    qApp->closeAllWindows();
}

void BitSharesMainWindow::on_actionCreateMail_triggered()
{
    //QMessageBox::information(this,"Create Mail Message","Widget not implemented yet");
    //Allow multiple createMailWidgets to be created
    //QCreateMailWidget* createMailWidget = new QCreateMailWidget(this);
    //createMailWidget->show();
    ChatWidget* chatWidget = new ChatWidget(nullptr); //parent is nullptr to make modeless floating window
    static int person = 0;
    ++person;
    QString contactName = QString("Person %1").arg(person);
    chatWidget->setContact(contactName);
    chatWidget->show();

}
