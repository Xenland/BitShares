#include "profileeditor.h"
#include "ui_profileeditor.h"

ProfileEditor::ProfileEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProfileEditor)
{
    ui->setupUi(this);
}

ProfileEditor::~ProfileEditor()
{
    delete ui;
}
