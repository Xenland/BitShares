#include "bitsharesmainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    BitSharesMainWindow mainWindow;
    mainWindow.show();
    
    return application.exec();
}
