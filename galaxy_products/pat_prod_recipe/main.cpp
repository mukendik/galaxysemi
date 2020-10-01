#include <QApplication>

#include "patprodrecipe.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    PatProdRecipe widget;
    widget.show();
    return app.exec(); //
}

