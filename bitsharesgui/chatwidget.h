#ifndef CHATWIDGET_H
#define CHATWIDGET_H

//DLN only include one of these
#include "selfsizingwidget.h"
//#include "selfsizingmainwindow.h"

namespace Ui {
class ChatWidget;
}

class ChatWidget : public SelfSizingWidget
{
    Q_OBJECT
    //identity
    //contact
    //communication data
    //session message history (list model)
public:
    explicit ChatWidget(QWidget *parent = 0);
    ~ChatWidget();
    
    void setContact(QString contactName);

private:
    Ui::ChatWidget *ui;
};

#endif // CHATWIDGET_H
