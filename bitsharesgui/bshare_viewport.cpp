#include "bshare_viewport.hpp"

bshare_viewport::bshare_viewport(QWidget *parent) :
    QWidget(parent)
{
    /***
     ** Notes about structure
     **
     ** This class is a QWidget with a QStackedWidget attached
     ** allowing multiple widgets to be called in one area.
     ** Instead of calling the qstackedwidget, we will be calling
     ** bshare_viewport functions to control the stackedwidget/viewport.
     ** The benifits should be apparent.
     ***/

    layout = new QGridLayout(this);
    this->setLayout(layout);
        //layout settings
        //layout->setMargin(0);

    /**
     * Initialize viewport
     **/
    stacked_widget_viewport = new QStackedWidget(0);
    stacked_widget_viewport->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    stacked_widget_viewport->setStyleSheet("QStackedWidget{border:1px solid #000;}");
    layout->addWidget(stacked_widget_viewport, 0,0, 1,1, Qt::AlignLeft);

    mail_view = new bshare_viewport_mail(0);
        mail_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        //Add to (stacked widget)viewport
        stacked_widget_viewport->addWidget(mail_view);
}



/***
 * Group of Functions
 * Select which view should be showing
 ***/
void bshare_viewport::show_view(QString view){
    if(view == "mail"){
        stacked_widget_viewport->setCurrentIndex(0);
    }
}
