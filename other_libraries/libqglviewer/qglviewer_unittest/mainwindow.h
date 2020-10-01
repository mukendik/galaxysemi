#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qglviewer.h>
#include "drill_3d_viewer.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    /*
    class glviewer : public QGLViewer
    {
        Q_OBJECT

        public:
            glviewer(QWidget * pParent = NULL) { }
            virtual ~glviewer() {}

    };
    glviewer mGLViewer;
    */
    GexGLViewer mGLV;
public slots:
    void OnGrab();
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
