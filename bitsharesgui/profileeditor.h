#ifndef PROFILEEDITOR_H
#define PROFILEEDITOR_H

#include <QDialog>

namespace Ui {
class ProfileEditor;
}

class ProfileEditor : public QDialog
{
    Q_OBJECT
    
public:
    explicit ProfileEditor(QWidget *parent = 0);
    ~ProfileEditor();
    
private:
    Ui::ProfileEditor *ui;
};

#endif // PROFILEEDITOR_H
