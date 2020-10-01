#include <QApplication>
#include <math.h>

#include "test.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    for (int i=0; true; i++)
    {
        printf("\r%d", i);
        try
        {
            Test* lT=new Test();
        }
        catch(const std::bad_alloc& e)
        {
            printf("\n%s\n", e.what());
            exit(EXIT_FAILURE);
        }
    }

    return a.exec();
}
