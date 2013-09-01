#include "bshare_viewport_mail.hpp"

bshare_viewport_mail::bshare_viewport_mail(QWidget *parent) :
    QWidget(parent)
{
    layout = new QGridLayout(this);
    layout->setColumnStretch(0, 0);
    layout->setColumnStretch(1,1);
    this->setLayout(layout);

    /***
     ** Init Mail view
     ***/

    /**
     * To Recipient Input
     **/
        /** To Label **/
        to_label = new QLabel("To:", this);
            //set shrinking
            to_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            //add to layout
            layout->addWidget(to_label, 0,0, 1,1, Qt::AlignRight);

        /** To Input **/
        to_input_container_widget = new QWidget(this);
        to_input_container_widget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        to_input_container = new QHBoxLayout(to_input_container_widget);
        to_input_container->setMargin(0);
        to_input_container_widget->setLayout(to_input_container);

            //add content to container
            to_input = new QLineEdit(this);
                //set expanding policy
                to_input->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

                //add to container
                to_input_container->addWidget(to_input);

            //add to_input to layout
            layout->addWidget(to_input_container_widget, 0,1, 1,1, Qt::AlignLeft);
    /**
     * From Input
     **/
        /** From Label **/
            from_label = new QLabel("From:", this);

            //add to layout
            layout->addWidget(from_label, 1,0, 1,1, Qt::AlignRight);

        /** From Input **/
            from_input = new QLineEdit(this);

            //add to layout
            layout->addWidget(from_input, 1,1, 1,1, Qt::AlignLeft);

    /**
     * Subject Input
     **/
        /** Subject Label **/
            subject_label = new QLabel("Subject:", this);

            //add to layout
            layout->addWidget(subject_label, 2,0, 1,1, Qt::AlignRight);

        /** Subject Input **/
            subject_input = new QLineEdit(this);

            //add to layout
            layout->addWidget(subject_input, 2,1, 1,1, Qt::AlignLeft);


    /**
     * Body/Message Input
     **/
        /** Message Label **/
            message_input = new QTextEdit(this);
            message_input->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            //add to layout
            layout->addWidget(message_input, 3,0, 1,2, Qt::AlignLeft);

}
