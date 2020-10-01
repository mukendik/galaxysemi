#include <QApplication>
#include <QTimer>
#include "mainwindow.h"
#if defined(linux) || defined(__linux)
# include <dlfcn.h>
#endif

/*

See that in my console (Vostro 3750):
getProcAddress: Unable to resolve 'glProgramParameteri'
getProcAddress: Unable to resolve 'glProgramParameteriEXT'
getProcAddress: Unable to resolve 'glProgramParameteri'
getProcAddress: Unable to resolve 'glProgramParameteriEXT'
getProcAddress: Unable to resolve 'glProgramParameteri'
getProcAddress: Unable to resolve 'glProgramParameteriEXT'

*/

int main(int argc, char *argv[])
{
#if defined(linux) || defined(__linux)
    if (dlopen("libX11-xcb.so", RTLD_LAZY | RTLD_NOLOAD) == NULL) {
        printf("%s\n", dlerror());
        return EXIT_SUCCESS;
    }
#endif

    qDebug("QT_VERSION_STR = %s", QT_VERSION_STR);
    QApplication a(argc, argv);
    qDebug("main: creating a MainWindow...");
    MainWindow w;
    w.show();
    
    #ifdef QT_DEBUG
        //QApplication::beep();
        //QApplication::aboutQt();
    #else

    #endif

    QTimer lGrabTimer;
    QObject::connect(&lGrabTimer, SIGNAL(timeout()), &w, SLOT(OnGrab()) );
    lGrabTimer.start(1000);

    QTimer lTimer;
    lTimer.start(2000);
    QObject::connect(&lTimer, SIGNAL(timeout()), &w, SLOT(close()) );

    int r=EXIT_SUCCESS;
    r=a.exec();
    return r;
}
