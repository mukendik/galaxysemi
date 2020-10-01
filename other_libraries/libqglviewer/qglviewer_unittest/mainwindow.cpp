#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qglviewer.h"
#include "vec.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    mGLV(this),
    ui(new Ui::MainWindow)
{
    qDebug("Creator of MainWindow...");
    //mGLV.init(); // init() is private...
    mGLV.makeCurrent();
    // Set camera position
    qglviewer::Vec vecEyePoint;
    vecEyePoint.setValue(60.0, -350.0, 200.0);
    // Set scene center
    qglviewer::Vec vecCenterPoint;
    vecCenterPoint.setValue(50.0, -50.0, 50.0);
    mGLV.setHome(vecEyePoint, vecCenterPoint);
    mGLV.setFPSIsDisplayed(true);
    mGLV.setBackgroundColor(QColor(Qt::blue)); // does not work on my win7 vostro 3750

    ui->setupUi(this);
    setCentralWidget(&mGLV);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::OnGrab()
{
    mGLV.goHome();

    mGLV.repaint();
    mGLV.drawGrid();
    mGLV.draw();
    mGLV.drawWithNames();

    QApplication::processEvents();

    //if (!QPixmap::grabWidget(&mGLV).save("test.bmp"))
    // "QPixmap::grabWidget is deprecated, use QWidget::grab() instead"
    QPixmap lPixmap=QPixmap::fromImage(mGLV.grabFrameBuffer()); // grab the main window or the GL widget ?
    //QPixmap lPixmap=grab(); // wont call the grab from GLViewer but the grab from the QWidget

    if (lPixmap.isNull())
    {
        printf("Failed to grab GLViewer: Pixmap null\n");
        qApp->exit(EXIT_FAILURE);
    }

    /*
    if (!lPixmap.save("test.bmp"))
    {
        printf("Failed to save the pixmap of the GLViewer\n");
        qApp->exit(EXIT_FAILURE);
    }
    */

    printf("Current fps: %f\n", mGLV.currentFPS() ); // most of the time 0...

    // The background color should be dark blue, not black: from GLViewer init:
    //glClearColor(0.0f, 0.0588f, 0.2f, 0.0f);	// Let OpenGL clear background color to dark-blue.
}
