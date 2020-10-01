#include <QtGui/QApplication>
#include "db_keys_editor.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DbKeysEditor w;
    w.show();
    
    return a.exec();
}
