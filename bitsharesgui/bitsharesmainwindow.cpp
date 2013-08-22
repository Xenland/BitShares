#include "bitsharesmainwindow.h"
#include "ui_bitsharesmainwindow.h"

BitSharesMainWindow::BitSharesMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::BitSharesMainWindow)
{
    ui->setupUi(this);
    ui->bitSharesTreeView->setModel(&_bitSharesTreeModel);
}

BitSharesMainWindow::~BitSharesMainWindow()
{
    delete ui;
}

void BitSharesMainWindow::on_actionExit_triggered()
{
    close();
}
