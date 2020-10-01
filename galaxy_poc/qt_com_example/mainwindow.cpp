#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->mQueryButton->setEnabled(false);

    QObject::connect(ui->mQueryButton, SIGNAL(clicked()), this, SLOT(onClicked()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onStart()
{
    emit sInitCom();
    show();
}


void MainWindow::onComInitialized(const QString & doc)
{
    ui->mDocCom->setText(doc);
    ui->mQueryButton->setEnabled(true);
}

void MainWindow::onSubObjectInitialized(const QString & doc)
{
    ui->mQueryButton->setEnabled(false);
    ui->mDocSubObject->setText(doc);
}

void MainWindow::onStatus(const QString & status)
{
    ui->mStatus->setText(status);
}

void MainWindow::onClicked()
{
    emit sQuery();
}
