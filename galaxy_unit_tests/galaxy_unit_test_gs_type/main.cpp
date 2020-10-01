#include "gs_types.h"
#include <stdio.h>
#include <QString>

int main(int argc, char** argv)
{
    printf("main: %d args: %s\n", argc, argv[0]?argv[0]:"?");

    // Read a correct recipe
    QString ErrorMsg("");

    if(sizeof(float) !=4 )
        ErrorMsg += "Float has a wrong size. It size is" + QString::number(sizeof(float)) + " != 4 \n";
    if(sizeof(double) !=8 )
        ErrorMsg += "double has a wrong size. It size is" + QString::number(sizeof(double)) + " != 8 \n";
    if(sizeof(gsint8) != 1)
        ErrorMsg += "gsint8 has a wrong size. It size is" + QString::number(sizeof(gsint8)) + " != 1 \n";
    if(sizeof(gsint16) != 2)
        ErrorMsg += "gsint16 has a wrong size. It size is" + QString::number(sizeof(gsint16)) + " != 2 \n";
    if(sizeof(gsint32) != 4)
        ErrorMsg += "gsint32 has a wrong size. It size is" + QString::number(sizeof(gsint32)) + " != 4 \n";
    if(sizeof(gsint64) != 8)
        ErrorMsg += "gsint64 has a wrong size. It size is" + QString::number(sizeof(gsint64)) + " != 8 \n";
    if(sizeof(gsuint8) != 1)
        ErrorMsg += "gsuint8 has a wrong size. It size is" + QString::number(sizeof(gsuint8)) + " != 1 \n";
    if(sizeof(gsuint16) != 2)
        ErrorMsg += "gsuint16 has a wrong size. It size is" + QString::number(sizeof(gsuint16)) + " != 2 \n";
    if(sizeof(gsuint32) != 4)
        ErrorMsg += "gsuint32 has a wrong size. It size is" + QString::number(sizeof(gsuint32)) + " != 4 \n";
    if(sizeof(gsuint64) != 8)
        ErrorMsg += "gsuint64 has a wrong size. It size is" + QString::number(sizeof(gsuint64)) + " != 8 \n";
    if(sizeof(gschar) != 1)
        ErrorMsg += "gschar has a wrong size. It size is" + QString::number(sizeof(gschar)) + " != 1 \n";
    if(sizeof(gsuchar) != 1)
        ErrorMsg += "gsuchar has a wrong size. It size is" + QString::number(sizeof(gsuchar)) + " != 1 \n";

    int lSizeOfGstime;

    // If windows
    #ifdef _WIN32
    #ifdef _WIN64
    lSizeOfGstime = 8;
    #else
    lSizeOfGstime = 4;
    #endif
    #endif
    // if linux
    #if __GNUC__
    #if __x86_64__ || __ppc64__
    lSizeOfGstime = 8;
    #else
    lSizeOfGstime = 4;
    #endif
    #endif

    if(sizeof(gstime_t) != lSizeOfGstime)
        ErrorMsg += "gstime_t has a wrong size. It size is" + QString::number(sizeof(gstime_t)) + " != "+ lSizeOfGstime + " \n";

    if (ErrorMsg != "")
    {
        printf("These variable have wrong size %s \n", ErrorMsg.toLatin1().constData());
        return EXIT_FAILURE;
    }
    else
    {
        printf("All variables have correct size \n");
    }

    printf("Success\n");
    return EXIT_SUCCESS;
}
