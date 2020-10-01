#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <libgexpb.h>
#include <QScriptEngine>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_pb(0),
    m_se(0),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_se=new GexScriptEngine(this);

    m_pb=CGexPB::CreatePropertyBrowser(m_se, this, 20);
    setCentralWidget(m_pb);
}

MainWindow::~MainWindow()
{
    delete ui;
}
