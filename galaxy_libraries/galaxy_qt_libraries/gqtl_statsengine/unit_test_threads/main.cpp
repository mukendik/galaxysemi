
#include <unistd.h>

#include "statsthread.h"


#ifdef _WIN32
    #include <windows.h>

    void cSleep(unsigned milliseconds)
    {
        Sleep(milliseconds);
    }
#else
    #include <unistd.h>

    void cSleep(unsigned milliseconds)
    {
        usleep(milliseconds * 1000); // takes microseconds
    }
#endif

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QString lAppDir = QCoreApplication::applicationDirPath();

    printf("Main thread ID: %p\n", QThread::currentThreadId());

    QList<StatsThread*> lThreadList;
    int lIdealCount = QThread::idealThreadCount();
    printf("Ideal thread count: %d\n", lIdealCount);

    for(int i = 0; i< lIdealCount; ++i)
    {
        StatsThread* lThread = new StatsThread(lAppDir, i);
        printf("Starting thread %d...\n", i);
        lThread->start();
        lThreadList.append(lThread);
    }

    cSleep(10000);

    printf("Stop threads if still running!\n");

    for (int i = 0; i < lThreadList.size() ;++i)
    {
        if (!lThreadList[i]->isThreadOk())
        {
            printf("Error when running thread %d! EXIT...\n", i);
            return EXIT_FAILURE;
        }

        delete lThreadList[i];
        lThreadList[i] = NULL;
    }

    printf("Test succesfully done!\n");
    return EXIT_SUCCESS;
}
