#ifndef WEBVIEW_H
#define WEBVIEW_H

#include <QWebView>

class WebView: public QWebView
{
    Q_OBJECT

public:
    WebView():QWebView()
    {
        QObject::connect(this, SIGNAL(loadFinished(bool)), this, SLOT(OnLoadFinished(bool)));
        QObject::connect(this, SIGNAL(statusBarMessage(QString)), this, SLOT(OnStatusBarMessage(QString)));
    }
public slots:
    void OnLoadFinished(bool ok)
    {
        qDebug("Load finished : ok = %s", ok?"true":"false");
    }
    void OnStatusBarMessage(const QString &text)
    {
        qDebug("New message : %s", text.toLatin1().data());
    }
};

#endif // WEBVIEW_H
