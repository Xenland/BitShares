#ifndef BSHARE_GUI_H
#define BSHARE_GUI_H

//Qt libs
#include <QWidget>
#include <QList>
#include <QStringList>
#include <QStandardItem>
#include <QStandardItemModel>

//bshare libs
#include "bshare_viewport.hpp"

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

    /** Private Variables **/
    QStandardItemModel * bshare_menu_treeview_model;
    bshare_viewport * viewport_controller;

    /** Private Functions **/
};

#endif // BSHARE_GUI_H
