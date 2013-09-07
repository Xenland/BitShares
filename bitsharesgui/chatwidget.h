#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>

namespace Ui {
class ChatWidget;
}

class ChatWidget : public QWidget
{
    Q_OBJECT
    //identity
    //contact
    //communication data
    //session message history (list model)
public:
    explicit ChatWidget(QWidget *parent = 0);
    ~ChatWidget();
    
private:
    Ui::ChatWidget *ui;
};

#endif // CHATWIDGET_H
