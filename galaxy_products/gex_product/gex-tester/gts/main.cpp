#include <qapplication.h>
#include "gts_mainwindow.h"


int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    Gts_MainWindow w;
    w.show();
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    return a.exec();
}
