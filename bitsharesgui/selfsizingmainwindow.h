#ifndef SELFSIZINGMAINWINDOW_H
#define SELFSIZINGMAINWINDOW_H

#include <QMainWindow>

class SelfSizingMainWindow : public QMainWindow
{
    Q_OBJECT

protected:
    virtual void readSettings();
    virtual void writeSettings();
    virtual void closeEvent(QCloseEvent *event);
    virtual bool okToContinue() { return true; }

public:
    explicit SelfSizingMainWindow(QWidget *parent = 0);
    
signals:
    
public slots:
    
};

#endif // SELFSIZINGMAINWINDOW_H
