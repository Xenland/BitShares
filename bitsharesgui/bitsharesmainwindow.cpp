#include "bitsharesmainwindow.h"
#include "ui_bitsharesmainwindow.h"
#include <QtCore>
#include <QtGui>
#include <qmessagebox.h>

BitSharesMainWindow::BitSharesMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::BitSharesMainWindow)
{
    ui->setupUi(this);
    ui->bitSharesTreeView->setUniformRowHeights(true);
    ui->bitSharesTreeView->header()->hide();
    ui->bitSharesTreeView->setModel(&_bitSharesTreeModel);
    //ui->stackedWidget

}

BitSharesMainWindow::~BitSharesMainWindow()
{
    delete ui;
}

void BitSharesMainWindow::on_actionExit_triggered()
{
    close();
}

void BitSharesMainWindow::on_actionCreateMail_triggered()
{
    QMessageBox::information(this,"Create Mail Message","Widget not implemented yet");
    //Allow multiple createMailWidgets to be created
    //QCreateMailWidget* createMailWidget = new QCreateMailWidget(this);
    //createMailWidget->show();

}
