/********************************************************************************
** Form generated from reading UI file 'profileeditor.ui'
**
** Created by: Qt User Interface Compiler version 5.1.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PROFILEEDITOR_H
#define UI_PROFILEEDITOR_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHeaderView>

QT_BEGIN_NAMESPACE

class Ui_ProfileEditor
{
public:
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *ProfileEditor)
    {
        if (ProfileEditor->objectName().isEmpty())
            ProfileEditor->setObjectName(QStringLiteral("ProfileEditor"));
        ProfileEditor->resize(400, 300);
        buttonBox = new QDialogButtonBox(ProfileEditor);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setGeometry(QRect(30, 240, 341, 32));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        retranslateUi(ProfileEditor);
        QObject::connect(buttonBox, SIGNAL(accepted()), ProfileEditor, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), ProfileEditor, SLOT(reject()));

        QMetaObject::connectSlotsByName(ProfileEditor);
    } // setupUi

    void retranslateUi(QDialog *ProfileEditor)
    {
        ProfileEditor->setWindowTitle(QApplication::translate("ProfileEditor", "Dialog", 0));
    } // retranslateUi

};

namespace Ui {
    class ProfileEditor: public Ui_ProfileEditor {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PROFILEEDITOR_H
