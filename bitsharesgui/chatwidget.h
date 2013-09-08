#ifndef CHATWIDGET_H
#define CHATWIDGET_H

//DLN only include one of these
#include "selfsizingwidget.h"
//#include "selfsizingmainwindow.h"
#include <qstringlistmodel.h>

class StringListModel : public QStringListModel
{
public:
              StringListModel(QObject* parent) : QStringListModel(parent) {}
         void append(const QString& string)
                {
                insertRows(rowCount(), 1);
                setData(index(rowCount()-1), string);
                }

  StringListModel& operator<<(const QString& string)
                {
                append(string);
                return *this;
                }
};


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
    bool eventFilter(QObject *watched, QEvent *e);

private slots:
    void sendCurrentChatMessage();
    void sendChatMessage(QString messageToSend);
    void messageReceived(QString receivedMessage);
private:


    Ui::ChatWidget *ui;
    StringListModel* chatViewModel;
};

#endif // CHATWIDGET_H
