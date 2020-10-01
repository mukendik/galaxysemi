#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->pushButtonClose, SIGNAL(clicked()), this, SLOT(closeMe()));
}



MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeMe()
{
    exit(EXIT_SUCCESS);
}

