#include "chatwidget.h"
#include "ui_chatwidget.h"


ChatWidget::ChatWidget(QWidget *parent) :
    SelfSizingWidget(parent),
    ui(new Ui::ChatWidget)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    QAction* sendCurrentChatMessageAction = new QAction(tr("Send Chat Message"),this);
    sendCurrentChatMessageAction->setShortcut(tr("Ctrl+Enter"));
    sendCurrentChatMessageAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(sendCurrentChatMessageAction, SIGNAL(activated()),
           this, SLOT(sendCurrentChatMessage()));
    addAction(sendCurrentChatMessageAction);


    chatViewModel = new StringListModel(ui->chatView);
    QStringList stringList;
    stringList << "Ni hao" << "Hao. Ni hao ma";
    chatViewModel->setStringList(stringList);
    ui->chatView->setModel(chatViewModel);

    ui->chatTextEdit->installEventFilter(this);

}

ChatWidget::~ChatWidget()
{
    delete ui;
}

void ChatWidget::setContact(QString contactName)
{
    setWindowTitle(contactName);
    readSettings();
}

bool ChatWidget::eventFilter(QObject *watched, QEvent* qevent)
{
    if (watched == ui->chatTextEdit && qevent->type() == QEvent::KeyPress) 
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(qevent);
        if (keyEvent->key() == Qt::Key_Enter ||
            keyEvent->key() == Qt::Key_Return) 

        {
            if (keyEvent->modifiers() & Qt::ControlModifier) 
            {
                sendCurrentChatMessage();
                return true;
            }
        }
    }
    return QWidget::eventFilter(watched, qevent);
}

void ChatWidget::sendCurrentChatMessage()
{
    QString messageToSend = ui->chatTextEdit->toPlainText();
    sendChatMessage(messageToSend);
}

void ChatWidget::sendChatMessage(QString messageToSend)
{
    chatViewModel->append(messageToSend);
    ui->chatTextEdit->setPlainText("");
}

void ChatWidget::messageReceived(QString receivedMessage)
{
    chatViewModel->append(receivedMessage);
}


#if 0
//unused code for intercepting Ctrl-Enter and Enter in chatTextEditor
void chatTextEditor::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) 
    {
    case Qt::Key_Enter:
        if (event->modifiers() & Qt::ControlModifier) 
        {
        }
        else
        {
        }
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}
#endif