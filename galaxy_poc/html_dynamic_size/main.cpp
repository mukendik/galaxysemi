#include <QApplication>
#include <QGuiApplication>
#include <QPainter>
#include <QImage>
#include <QDir>
#include <QTextBrowser>
#include <QtWebKit>
#include <QWebView>
#include "webview.h"

int main(int argc, char **argv)
{
    //QApplication myApp(argc, argv, true);
    QApplication lApp(argc, argv);

    WebView lWV;
    lWV.setWindowTitle("QWebView");
    //lWV.load(QUrl::fromLocalFile("no_table.htm"));
    // the base QUrl must be the correct path finishing with a '/' !!!!!!!!!!!!!!!!!!!!!!!!!
    lWV.setHtml(
      QString("<html><body>  "
            " <img border=\"1\" style=\"height:90%%\" alt=\"error\" lowsrc=\"waf1-1-T1.png\" src=\"waf1-1-T786000.png\"> "
            "</body></html>"),
      QUrl::fromLocalFile("G:/galaxy_dev_master/galaxy_poc/html_dynamic_size/")
                );
    //lWV.load(QUrl::fromLocalFile("ppt_slide.htm"));
    lWV.show();
    //lWV.reload();

    QTextBrowser lTB;
    lTB.setWindowTitle("QTextBrowser");
    lTB.setSource(QUrl("no_table.htm"));
    lTB.show();
    lTB.move(10,10);

    return lApp.exec();
}

