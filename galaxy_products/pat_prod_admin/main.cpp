#include <QApplication>

#include "patprodrecipe_admin.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
	PatProdRecipe_Admin widget;
    widget.show();
    return app.exec(); //
}

