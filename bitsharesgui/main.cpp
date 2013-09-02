#ifdef WIN32
#include "bitsharesmainwindow.h"
#else
#include "bshare_gui.h"
#endif
#include <QApplication>
#include <QDebug>
#include <fc/thread/thread.hpp>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
#ifdef WIN32
    BitSharesMainWindow mainWindow;
    mainWindow.show();
#else
    bshare_gui bshare_window;
    bshare_window.setWindowTitle("Bitshares | Invictus Innovations");
    bshare_window.show();
    qDebug() << "RUNNING";
	fc::usleep( fc::microseconds(1000) );
#endif    
    return application.exec();
}
