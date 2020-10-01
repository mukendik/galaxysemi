#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QScriptEngine>
#include <libgexpb.h>
#include <gex_scriptengine.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    CGexPB* m_pb;
    GexScriptEngine* m_se;
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
