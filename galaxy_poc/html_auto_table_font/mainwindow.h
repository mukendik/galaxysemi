#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebView>
#include <QTimer>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QWebView mWebView;
    QTimer mTimer;
public slots:
    void OnLoadFinished(bool);
    void OnLoadStarted();
    void OnLoadProgress(int);
    void OnStatusBarchanged(QString);
    void LoadURL();
};

#endif // MAINWINDOW_H
