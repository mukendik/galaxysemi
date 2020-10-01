#include "mainwindow.h"
#include <QApplication>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    QTimer timer;
    w.connect(&timer, SIGNAL(timeout()), SLOT(close()));
    timer.start(2000);

    return a.exec();
}
