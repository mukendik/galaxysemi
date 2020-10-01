#include <QtCore/QCoreApplication>
#include <QtWebKit>
#include <QWebView>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    qDebug("main: hello world!");
    //QWebView v; // impossible
    //QWebPage p; // impossible
    QGraphicsWebView wv; // ok
    //QWebFrame f; // impossible : needs a QWebPage and destructor private

    return a.exec();
}
