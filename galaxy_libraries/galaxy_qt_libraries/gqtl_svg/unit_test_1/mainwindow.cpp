#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <gqtl_svgrenderer.h>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mSVGW.load(QString("11-zone_os.svg") );
    setCentralWidget(&mSVGW);
}

MainWindow::~MainWindow()
{
    delete ui;
}
