#include <stdio.h>
#include <stdint.h>
#include <QCoreApplication>
#include <QThread>
#include <math.h>
#include <main.h>

bool TestPart191Test307()
{
    printf("Test Part191Test307: (sizeof float = %zd)\n", sizeof(float));
    float lFloat=(float)0;
    //memset(&lFloat, 0x7fa1f7be, sizeof(float));
    //memset(&lFloat, 0xbe, 1);
    //memset(()(&lFloat), 0xbe, 1);

    //unsigned char ptr[] = {0x7f, 0xa1, 0xf7, 0xbe};
    float lFloatMem;
    unsigned char* ptr = (unsigned char*) &lFloatMem;
    ptr[0] = 0x7f;
    ptr[1] = 0xa1;
    ptr[2] = 0xf7;
    ptr[3] = 0xbe;
    printf("todo: check little/big endianess...\n");
    //std::reverse(ptr, ptr + 4);
    lFloat = *reinterpret_cast<float*>(ptr);

    printf("Strangely, on our win32 QA server, 0x7fa1f7be becomes -4.841423e-001(0x7fe1f7be)\n");

    printf("percent f=%f : should be -0.483654\n", lFloat);
    printf("percent g=%e : should be -4.836540e-001\n", lFloat);
    //printf("percent A=%.10A : should be ???\n", lFloat);

    char lBuffer[256]="";
    sprintf(lBuffer, "%f", lFloat);
    if (strcmp(lBuffer, "-0.483654"))
    {
        printf("float diff from expected value\n");
        return false;
    }

    //printf("hexa= %x : should be ?????\n", *(unsigned int*)&lFloat);

    /*
    sprintf(lBuffer, "%x", *(unsigned int*)&lFloat);
    if (strcmp(lBuffer, ""))
    {
        printf("Hexa diff\n");
        return false;
    }
    */

    return true;
}

#define GETSTR(x) #x
#define GETSTRSTR(x) GETSTR(x)

bool TestPi()
{
    printf("\nTest Pi:\n");
    char lBuffer[256]="";
    double lPi=M_PI; // 3.14159265358979323846
    printf("M_PI percent lf =%.20lf \n", lPi); // 3.14159265358979323846
    sprintf(lBuffer, "%.20lf", lPi);
    if (strcmp(lBuffer, GETSTRSTR(M_PI) ))
    {
        printf("%s != %s\n", lBuffer, GETSTRSTR(M_PI));
        //return false;
    }

    float lPiFloat=(float)lPi;
    printf("PI percent f=%.22f \n", lPiFloat);
    printf("PI percent A=%.10A : should be 0XC.90FDB00000P-2\n", lPiFloat);

    sprintf(lBuffer, "%.10A", lPiFloat);
    if (strcmp(lBuffer, "0XC.90FDB00000P-2"))
    {
        printf("Hexa float diff\n");
        return false;
    }
    //printf("Pi hexa= %x : should be 40490fdb\n", *(unsigned int*)&lPiFloat);
    unsigned char* ptr = (unsigned char*) &lPiFloat;
    uint32_t lPiInt;
    lPiInt = (ptr[0] << 24) + (ptr[1] << 16) + (ptr[2] << 8) + ptr[8];
    printf("Pi hexa= %x : should be 40490fdb\n", lPiInt);
    //sprintf(lBuffer, "%x", *(unsigned int*)&lPiFloat);
    sprintf(lBuffer, "%x", lPiInt);
    if (strcmp(lBuffer, "40490fdb"))
    {
        printf("Hexa diff\n");
        return false;
    }
    return true;
}

void MyThread::run()
{
    printf("Test multithreaded:\n");
    /*
    if (!TestPi())
    {
        printf("Test Pi failed\n");
        exit(EXIT_FAILURE);
    }
    */
    TestPart191Test307();
    printf("End of threaded test...\n");
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    MyThread lT;
    lT.start();

    while (lT.isRunning())
        ;

    //if (!TestPi())
      //  return EXIT_FAILURE;

    printf("\nmain thread test:\n");
    if (!TestPart191Test307())
    {
        printf("Main thread test failed\n");
        return EXIT_FAILURE;
    }

    printf("\nsuccess\n");
    return EXIT_SUCCESS;
}
