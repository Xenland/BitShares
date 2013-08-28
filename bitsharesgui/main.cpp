#include "bitsharesmainwindow.h"
#include <QApplication>
#include <fc/thread/thread.hpp>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    BitSharesMainWindow mainWindow;
    mainWindow.show();
	fc::usleep( fc::microseconds(1000) );
    
    return application.exec();
}
