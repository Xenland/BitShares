#ifndef BSHARE_VIEWPORT_MAIL_HPP
#define BSHARE_VIEWPORT_MAIL_HPP

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QHBoxLayout>
#include <QBoxLayout>

class bshare_viewport_mail : public QWidget
{
    Q_OBJECT
public:
    explicit bshare_viewport_mail(QWidget *parent = 0);
private:
    QGridLayout * layout;

    //To
    QLabel * to_label;
        //HBoxLayout (Hints the input to stretch horizontally)
        QWidget * to_input_container_widget;
        QHBoxLayout * to_input_container;

            QLineEdit * to_input;

    //From
    QLabel * from_label;
    QLineEdit * from_input;

    //Subject
    QLabel * subject_label;
    QLineEdit * subject_input;

    //Body (Rich text editor functionality)
    QTextEdit * message_input;


signals:
    
public slots:
    
};

#endif // BSHARE_VIEWPORT_MAIL_HPP
