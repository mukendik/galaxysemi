#include <QApplication>
#include <QtWebKit>
#include <QWebView>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    qDebug("main:");
    QWebView v; // impossible
    v.show();
    v.load(QUrl("DieSelector.svg"));
    //QWebPage p; // impossible
    //QGraphicsWebView wv; // ok
    //QWebFrame f; // impossible : needs a QWebPage and destructor private

    return a.exec();
}
