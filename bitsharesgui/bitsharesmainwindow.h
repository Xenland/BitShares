#ifndef BITSHARESMAINWINDOW_H
#define BITSHARESMAINWINDOW_H

#include <QMainWindow>
#include "bitsharestreemodel.h"

namespace Ui {
class BitSharesMainWindow;
}

class BitSharesMainWindow : public QMainWindow
{
    Q_OBJECT

    void readSettings();
    void writeSettings();
    void closeEvent(QCloseEvent *event);
    bool okToContinue() { return true; }

public:
    explicit BitSharesMainWindow(QWidget *parent = 0);
             ~BitSharesMainWindow();
    
private slots:

    void on_actionExit_triggered();

    void on_actionCreateMail_triggered();

private:
    Ui::BitSharesMainWindow *ui;
    BitSharesTreeModel _bitSharesTreeModel;

};

#endif // BITSHARESMAINWINDOW_H
