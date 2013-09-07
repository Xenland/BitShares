#include "chatwidget.h"
#include "ui_chatwidget.h"

ChatWidget::ChatWidget(QWidget *parent) :
    SelfSizingWidget(parent),
    ui(new Ui::ChatWidget)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
}

void ChatWidget::setContact(QString contactName)
{
    setWindowTitle(contactName);
    readSettings();
}

ChatWidget::~ChatWidget()
{
    delete ui;
}


