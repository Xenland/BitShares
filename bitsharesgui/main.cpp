//#include "bitsharesmainwindow.h"
#include "bshare_gui.h"
#include <QApplication>
#include <QDebug>
#include <fc/thread/thread.hpp>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    //BitSharesMainWindow mainWindow;
    //mainWindow.show();
    bshare_gui bshare_window;
    bshare_window.setWindowTitle("Bitshare | Invictus Innovations");
    bshare_window.show();
    qDebug() << "RUNNING";
	fc::usleep( fc::microseconds(1000) );
    
    return application.exec();
}
