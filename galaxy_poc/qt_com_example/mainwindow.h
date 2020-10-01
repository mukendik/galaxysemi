#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#ifdef WIN32
#include <QAxObject>
#endif

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void    sInitCom();
    void    sQuery();

public slots:
    void    onStart();
    void    onComInitialized(const QString & doc);
    void    onSubObjectInitialized(const QString & doc);
    void    onStatus(const QString & status);
    void    onClicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
