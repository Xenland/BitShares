#ifndef BSHARE_GUI_H
#define BSHARE_GUI_H

#include <QWidget>
#include <QList>
#include <QStringList>
#include <QStandardItem>
#include <QStandardItemModel>

namespace Ui {
class bshare_gui;
}

class bshare_gui : public QWidget
{
    Q_OBJECT
    
public:
    explicit bshare_gui(QWidget *parent = 0);
    ~bshare_gui();
    
private:
    Ui::bshare_gui *ui;

    QStandardItemModel * bshare_menu_treeview_model;
};

#endif // BSHARE_GUI_H
