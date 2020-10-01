#include <QCoreApplication>
#include <QDir>
#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
    #include <windows.h> // DWORD
    // Kernel32.lib on Windows 7 and Windows Server 2008 R2,
    // Psapi.lib on Windows Server 2008, Windows Vista, Windows Server 2003, and Windows XP/2000
    #include <psapi.h>
    #include <TlHelp32.h> // for CreateToolhelp32Snapshot
#endif

#ifdef __unix
    int GetProcesses()
    {
        printf("GetProcesses\n");
        QDir lProc("/proc");
        QFileInfoList lFIL=lProc.entryInfoList(QStringList("*"), QDir::Dirs);
        foreach(QFileInfo lF, lFIL)
        {
            if (!QFile::exists("/proc/"+lF.fileName()+"/cmdline"))
                continue;
            QFile lCmdLine("/proc/"+lF.fileName()+"/cmdline");
            if (!lCmdLine.open(QIODevice::ReadOnly))
                continue;
            QByteArray lBA=lCmdLine.readAll();
            //if (lF.fileName()=="4039") //(lBA.endsWith("gex"))
                printf("%s %s\n", lF.fileName().toLatin1().data(), lBA.data());
            lCmdLine.close();
        }
        return 0;
    }
#endif

#ifdef WIN32
    int GetProcesses()
    {
        // Get the list of process identifiers.

        HANDLE hProcessSnap;
        //HANDLE hProcess;
        PROCESSENTRY32 pe32;
        //DWORD dwPriorityClass;

        // Take a snapshot of all processes in the system.
        hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
        if( hProcessSnap == INVALID_HANDLE_VALUE )
        {
            printf( "CreateToolhelp32Snapshot failed\n");
            return -1;
        }
        // Set the size of the structure before using it.
        pe32.dwSize = sizeof( PROCESSENTRY32 );


        // Retrieve information about the first process, and exit if unsuccessful. Dont know why....
        if( !Process32First( hProcessSnap, &pe32 ) )
        {
          printf( "Process32First failed"); // show cause of failure
          CloseHandle( hProcessSnap );          // clean the snapshot object
          return  -1;
        }

        // Now walk the snapshot of processes, and
        // display information about each process in turn
        do
        {
            printf("%ld\t%ls\n", pe32.th32ProcessID, pe32.szExeFile);
        }
        while( Process32Next( hProcessSnap, &pe32 ) );
        CloseHandle( hProcessSnap );

        return 0;
    }
#endif

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    int r=GetProcesses();

    return r; //a.exec();
}
