#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>

namespace Ui {
class ChatWidget;
}

class ChatWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit ChatWidget(QWidget *parent = 0);
    ~ChatWidget();
    
private:
    Ui::ChatWidget *ui;
};

#endif // CHATWIDGET_H
