#include "bshare_gui.h"
#include "ui_bshare_gui.h"

bshare_gui::bshare_gui(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::bshare_gui)
{
    //Setup this UI (read ui file basically and represent this class with it)
    ui->setupUi(this);

    /***
     ** Notes on structure
     **
     ** 1]treeview
     ** 2]viewport
     ***/

        /**
         * 1]treeview
         *  Populate treeview (leftside)
         **/

            /***
             ** Init treeview_modal
             ***/
                bshare_menu_treeview_model = new QStandardItemModel(this);
                //Set columns
                bshare_menu_treeview_model->setColumnCount(2);

                //Set column names
                QStringList ColumnNames;
                ColumnNames << "" << "Balance";
                bshare_menu_treeview_model->setHorizontalHeaderLabels(ColumnNames);

                /***
                 ** Account (toplevel)
                 ***/
                 QList<QStandardItem *> account_tree_info;
                 account_tree_info.append(new QStandardItem(QString("Account")));

                QStandardItem *account_tree = bshare_menu_treeview_model->invisibleRootItem();
                account_tree->appendRow(account_tree_info);


                /***
                 ** Trade (toplevel)
                 ***/
                 QList<QStandardItem *> trade_tree_info;
                 trade_tree_info.append(new QStandardItem(QString("Trade")));

                QStandardItem * trade_tree = bshare_menu_treeview_model->invisibleRootItem();
                trade_tree->appendRow(trade_tree_info);



                /***
                 ** Contacts (toplevel)
                 ***/
                 QList<QStandardItem *> contacts_tree_info;
                 contacts_tree_info.append(new QStandardItem(QString("Contacts")));

                QStandardItem * contacts_tree = bshare_menu_treeview_model->invisibleRootItem();
                contacts_tree->appendRow(contacts_tree_info);



                /***
                 ** Identites (toplevel)
                 ***/
                 QList<QStandardItem *> identities_tree_info;
                 identities_tree_info.append(new QStandardItem(QString("Identities")));

                QStandardItem * identities_tree = bshare_menu_treeview_model->invisibleRootItem();
                identities_tree->appendRow(identities_tree_info);


                /***
                 ** Mail (toplevel)
                 ***/
                 QList<QStandardItem *> mail_tree_info;
                 mail_tree_info.append(new QStandardItem(QString("Mail")));

                QStandardItem * mail_tree = bshare_menu_treeview_model->invisibleRootItem();
                mail_tree->appendRow(mail_tree_info);


                /***
                 ** Favorites (toplevel)
                 ***/
                 QList<QStandardItem *> favorites_tree_info;
                 favorites_tree_info.append(new QStandardItem(QString("Favorites")));

                QStandardItem * favorites_tree = bshare_menu_treeview_model->invisibleRootItem();
                favorites_tree->appendRow(favorites_tree_info);

        //Pair Model/View
        ui->bshare_menu_treeview->setModel(bshare_menu_treeview_model);

        //Set max width
        ui->bshare_menu_treeview->setMaximumWidth(300);

        //Auto-resize header for convience.
        ui->bshare_menu_treeview->resizeColumnToContents(0);


    /**
     * 2]viewport
     *  Viewport (rightside)
     **/
        viewport_controller = new bshare_viewport(this);
            //Attach to ui
#if 0
            ui->bshare_gui_gridlayout->addWidget(viewport_controller, 0,1, 1,1, Qt::AlignLeft | Qt::AlignTop);
#endif


}

bshare_gui::~bshare_gui()
{
    delete ui;
}
