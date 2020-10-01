#include "mainwindow.h"
#include <QApplication>
#include <QTimer>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    qDebug(QDir::currentPath().toLatin1().data());

    QTimer t;
    t.singleShot(100, &w, SLOT(LoadURL()) );

    //w.mWebView.setUrl(QUrl::fromLocalFile("advanced.htm")); // better ?
    //w.mWebView.load(QUrl::fromLocalFile("advanced.html"));
    //QApplication::processEvents();
    //QThread::currentThread()->sleep(5);
    w.mWebView.show();
    //while (!lWebView.showMaximized(); //loadFinished())
        //QCoreApplication::processEvents();
    //for(int i=0; i<100000; i++)
      //  QCoreApplication::processEvents();
    //mWebView.showMaximized();

    return a.exec();
}
