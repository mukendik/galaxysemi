#include <QPrinter>
#include <QWebView>
#include <QApplication>
#include <QThread>
#include <QDir>
#include <gstdl_systeminfo.h>
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), mWebView(parent)
{
    setCentralWidget(&mWebView);
    QObject::connect(&mWebView, SIGNAL(loadFinished(bool)), this, SLOT(OnLoadFinished(bool)) );
    QObject::connect(&mWebView, SIGNAL(loadStarted()), this, SLOT(OnLoadStarted()) );
    QObject::connect(&mWebView, SIGNAL(loadProgress(int)), this, SLOT(OnLoadProgress(int)) );
    QObject::connect(&mWebView, SIGNAL(statusBarMessage(QString)), this, SLOT(OnStatusBarchanged(QString)) );
    qDebug("MainWindow: end of ctor");
}

void MainWindow::OnStatusBarchanged(QString l)
{
    qDebug(l.toLatin1().data());
}

void MainWindow::OnLoadProgress(int p)
{
    qDebug("On load progress %d", p);
}

void MainWindow::LoadURL()
{
    mWebView.load(QUrl::fromLocalFile(QDir::currentPath()+"/advanced.html"));
    //mWebView.load(QUrl::fromLocalFile(QDir::currentPath()+"/test.html"));
    //mWebView.load(QUrl("http://www.ecosia.org"));
    //mWebView.setUrl(QUrl::fromLocalFile("test.html"));
}

void MainWindow::OnLoadFinished(bool f)
{
    CGSystemInfo lSI;
    qDebug("On load finished: ok: %s", f?"true":"false");
    if (f)
    {
        QPrinter lPrinter;
        lPrinter.setOutputFileName("portrait.pdf");
        lPrinter.setOrientation(QPrinter::Portrait);
        lPrinter.setPaperSize(QPrinter::A4); // 210 x 297 mm, 8.26 x 11.69 inches
        qreal l,r,t,b;
        lPrinter.getPageMargins(&l, &t, &r, &b, QPrinter::Millimeter);
        qDebug("Default Margin: l:%f t:%f r:%f b:%f mm", l, t, r, b); // default: 3mm ?
        lPrinter.setOutputFormat(QPrinter::PdfFormat);
        mWebView.print(&lPrinter);
        lPrinter.setOutputFileName("landscape.pdf");
        lPrinter.setOrientation(QPrinter::Landscape);
        mWebView.print(&lPrinter);
        /* // Will print into the default OS printer (webex, ...)
        lPrinter.setOutputFileName("native.ps");
        lPrinter.setOutputFormat(QPrinter::NativeFormat);
        mWebView.print(&lPrinter);
        */

        // Test with htmldoc
        QString lCP=QDir::current().absolutePath();
        qDebug(QDir::currentPath().toLatin1().data());
        QDir p=QDir::current();
        p.cdUp(); p.cdUp();
        p.cd("galaxy_products/gex_product/bin/htmldoc/");
        QDir::setCurrent( p.absolutePath() );
        qDebug(QDir::currentPath().toLatin1().data());
        // it seem htmldoc defaultly generate in pdf1.3..
        int lRet=system(QString("htmldoc%1 ../../../../galaxy_poc/html_auto_table_font/advanced.html "
                                "-t pdf14 -f %2/htmldoc.pdf")
                        .arg(QString(lSI.m_strOS.c_str()).startsWith("[PC-W")?".exe":"")
                        .arg(lCP)
                        .toLatin1().data() );
        qDebug("htmldoc returned: %d", lRet);

    }
    else
    {
        //mTimer.singleShot(2000, this, SLOT(LoadURL()) );
        //mWebView.reload();
        //mWebView.setHtml("<html><body>error</body></html>");
    }
}

void MainWindow::OnLoadStarted()
{
    qDebug("On load started");
}

MainWindow::~MainWindow()
{
    qDebug("MainWindow::~MainWindow");
}
