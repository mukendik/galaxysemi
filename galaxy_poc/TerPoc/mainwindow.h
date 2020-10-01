#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#ifdef WIN32
#include <QMap>
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

private:
    Ui::MainWindow*                                     ui;
    bool                                                getInputs(QString & inc);
#ifdef WIN32
    QMap<QString, QAxObject*>                           mIncrements;
    QAxObject*                                          checkOUT(const QString & inc, QString & msg);
    bool                                                checkIN(QAxObject *licInfo, QString & msg);
    void                                                refreshSummary(void);
    QAxObject                                           mTAGLMProxy;
#endif

private slots:
    void onCheckOUT();
    void onCheckIN();
    void onException(int, const QString &, const QString &, const QString &);
};

#endif // MAINWINDOW_H
