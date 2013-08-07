#include <QApplication>
#include <fc/thread/thread.hpp>
#include <fc/log/logger.hpp>

fc::future<void> app_loop;

void process_qt_events()
{
   while( !app_loop.canceled() )
   {
      QCoreApplication::instance()->sendPostedEvents();
      QCoreApplication::instance()->processEvents();
      fc::usleep( fc::microseconds( 1000*20 ) );
   }
}


int main( int argc, char** argv )
{
   try
   {
      QApplication app(argc, argv);      
      app.setOrganizationDomain( "invictus-innovations.com" );
      app.setOrganizationName( "Invictus Innovations, Inc" );
      app.setApplicationName( "Secure Exchange Client - SEC" );

      app_loop = fc::async( [=]{ process_qt_events(); } );
      app.connect( &app, &QCoreApplication::aboutToQuit, [](){ app_loop.cancel(); } );
      app_loop.wait();
      return 0;
   } 
   catch ( const fc::exception& e )
   {
      elog( "${e}", ("e",e.to_detail_string() ) );
   }
   return -1;
}
