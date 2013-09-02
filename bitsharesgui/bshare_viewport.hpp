#ifndef BSHARE_VIEWPORT_HPP
#define BSHARE_VIEWPORT_HPP

//qt libs
#include <QWidget>
#include <QStackedWidget>

//bshare libs
#include "bshare_viewports/bshare_viewport_mail.hpp"

class bshare_viewport : public QWidget
{
    Q_OBJECT
public:
    explicit bshare_viewport(QWidget *parent = 0);

private:
    /** Private Variables **/
        QGridLayout * layout;

        //Contains everything in one widget
        QStackedWidget * stacked_widget_viewport;

            //Account

            //Trade

            //Contacts

            //Identities

            //Mail
            bshare_viewport_mail * mail_view;

            //Favorites

    /** Private Functions **/
        void show_view(QString);


signals:
    
public slots:
    
};

#endif // BSHARE_VIEWPORT_HPP
