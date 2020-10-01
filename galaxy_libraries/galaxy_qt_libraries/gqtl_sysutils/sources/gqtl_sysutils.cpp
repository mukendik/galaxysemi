/******************************************************************************!
 * \file gqtl_sysutils.cpp
 * \brief GEX utilities classes
 *        see galaxy_poc/memsize
 ******************************************************************************/

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
    // overloading the WINVER/_WIN32_WINNT to access new EX API
    # define WINVER 0x0500
    # include <windows.h>
    # include <Psapi.h>
    #include <TlHelp32.h> // for CreateToolhelp32Snapshot...
    // We already have a CopyFile function in our
    // interface but Microsoft has a CopyFile macro
    # undef CopyFile
#endif

#if defined(linux) || defined(__linux)
// in order to use getrusage
# include <sys/resource.h>
// for sysinfo
# include <sys/sysinfo.h>
// for sysconf
# include <unistd.h>
// for malloc_stats
# include <malloc.h>
// For chmod
# include <sys/stat.h>
// For open
# include <fcntl.h>
#endif

#if defined(__APPLE__) && defined(__MACH__)
#include <malloc/malloc.h>
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/sysctl.h>
#include <unistd.h>
#endif

#include <ctype.h>
#include "gqtl_sysutils.h"
#include <gqtl_log.h>
#include <QObject>
#include <QFileInfo>
#include <QApplication>
#include <QDir>
#include <QRegExp>
#include <QDateTime>
#include <QMap>
#include <QVariant>
#include <QProcess>
#include <QStyleFactory>
#include <QWidget>

#ifdef _WIN32
// Prototype of Windows GetUserProfileDirectory() function
typedef BOOL (STDMETHODCALLTYPE FAR * LPFNGETUSERPROFILEDIR)(
    HANDLE hToken,
    LPTSTR lpProfileDir,
    LPDWORD lpcchSize
    );
#endif

/******************************************************************************!
 * \fn CGexSystemUtils
 * \brief Constructor
 ******************************************************************************/
CGexSystemUtils::CGexSystemUtils()
{
}

/******************************************************************************!
 * \fn ~CGexSystemUtils
 * \brief Destructor
 ******************************************************************************/
CGexSystemUtils::~CGexSystemUtils()
{
    GSLOG(7, "CGexSystemUtils::~CGexSystemUtils");
}

/******************************************************************************!
 * \fn GetPlatformDynLibExtension
 ******************************************************************************/
QString CGexSystemUtils::GetPlatformDynLibExtension()
{
#ifdef Q_OS_WIN
    return "dll";
#elif defined(__MACH__)
    return "dylib";
#elif __sun__
    return "so";
#elif defined(__linux__)
    return "so";
#endif
    return "";
}

/******************************************************************************!
 * \fn GetQObjectsCount
 ******************************************************************************/
QString CGexSystemUtils::GetQObjectsCount(
    QObject& root, QMap<QString, QVariant>& lCounts, const bool lRecursive)
{
    //QMap<QString, QVariant> lCounts;
    //if (!QCoreApplication::instance())
    //  return "error";
    QObjectList lOL = root.children();
    QObject* lO = 0;
    foreach(lO, lOL)
    {
        if (! lO)
        {
            continue;
        }
        #ifdef QT_DEBUG
        //qDebug("%s : %s",
        //       lO->metaObject()->className(),
        //       lO->objectName().toLatin1().data());
        #endif
        const char* lCN = lO->metaObject()->className();
        if (lCN == 0)
        {
            continue;
        }
        if (lCounts.contains(lCN))
        {
            lCounts[lCN] = lCounts[lCN].toInt() + 1;
        }
        else
        {
            lCounts[lCN] = QVariant(1);
        }

        if (lRecursive)
        {
            GetQObjectsCount(*lO, lCounts, lRecursive);
        }
    }
    return "ok";
}

/******************************************************************************!
 * \fn GetPlatform
 ******************************************************************************/
QString CGexSystemUtils::GetPlatform()
{
    QString os;
#ifdef Q_OS_WIN
    os = "win";
#elif defined(__MACH__)
    os = "mac";
#elif __sun__
    os = "solaris";
#elif defined(__linux__)
    os = "linux";
#endif
    os.append(QString::number(QSysInfo::WordSize));
    return os;
}

/******************************************************************************!
 * \fn CheckGexEnvironment
 * \brief Checks if GEX environment correctly setup
 * \return TRUE if GEX environment correctly setup, FALSE else
 ******************************************************************************/
bool CGexSystemUtils::CheckGexEnvironment(QString& strUserHome,
                                          QString& strApplicationDir,
                                          QString& strError)
{
    // Get user's home directory
    if (GetUserHomeDirectory(strUserHome) != NoError)
    {
        strError = "ERROR: Unable to retrieve user's home directory.\n";
        return false;
    }

    // Get application directory
    if (GetApplicationDirectory(strApplicationDir) != NoError)
    {
#if defined __unix__ || __APPLE__ & __MACH__
        strError =
            "ERROR: The environment variable GEX_PATH is not set...\nyou must"
            " set it to the path of the Quantix Examinator application.\n";
#else
        strError =
            "ERROR: Unable to retrieve Examinator application directory.\n";
#endif
        return false;
    }

    return true;
}

/******************************************************************************!
 * \fn GetUserHomeDirectory
 * \brief Retrieves the user's home directory
 * \return : NoError or error code
 ******************************************************************************/
int CGexSystemUtils::GetUserHomeDirectory(QString& strDirectory)
{

#ifdef _WIN32
    char szString[2048];

    // WinNT/Win2000: use SDK functions to get user's profile directory

    // Get Current Process handle
    HANDLE hProcess = GetCurrentProcess();

    // Get an access token
    HANDLE hAccessToken;
    if (OpenProcessToken(hProcess, TOKEN_QUERY, &hAccessToken) == true)
    {
        // Load userenv DLL
        HMODULE hLibrary = LoadLibraryA("userenv.dll");
        if (hLibrary != NULL)
        {
            // Get address of GetUserProfileDirectory() function
            LPFNGETUSERPROFILEDIR GetUserProfileDirectory = NULL;
            GetUserProfileDirectory = (LPFNGETUSERPROFILEDIR)
                GetProcAddress(hLibrary, "GetUserProfileDirectoryA");
            if (GetUserProfileDirectory != NULL)
            {
                // Get User's profile
                char szUserProfileDirectory[1024];
                unsigned long ulSize = 1024;

                if (GetUserProfileDirectory(hAccessToken,
                                            (LPTSTR) szUserProfileDirectory,
                                            &ulSize) != 0)
                {
                    strDirectory = szUserProfileDirectory;
                    NormalizePath(strDirectory);

                    return NoError;
                }
            }
        }
    }

    // Windows95 doesn't support Profilefolder, keep using Windows folder
    GetWindowsDirectoryA(szString, 2047);
    strDirectory = szString;
    NormalizePath(strDirectory);

    return NoError;

#endif

#if defined __unix__ || __APPLE__ & __MACH__
    // Unix: get HOME environment variable
    char* ptChar;

    ptChar = getenv("HOME");
    if (ptChar != NULL)
    {
        strDirectory = ptChar;  // the HOME variable is valid
        NormalizePath(strDirectory);
        return NoError;
    }

// We miss the HOME variable. Then assume file is in the current directory
//  if (getcwd(szString, 2047) == NULL)
//  {
//    //TODO
//  }
//  strDirectory  = szString;
//  NormalizePath(strDirectory);

    return NotSupported;
#endif
}

/******************************************************************************!
 * \fn GetApplicationDirectory
 * \brief Retrieves the application's absolute path
 * \return : NoError or error code
 ******************************************************************************/
int CGexSystemUtils::GetApplicationDirectory(QString& strDirectory)
{
#ifdef _WIN32
    // Windows: use SDK functions to get application path
    char szApplicationDirectory[1024];

    // Get application full name
    if (GetModuleFileNameA(NULL, szApplicationDirectory, 1024) == 0)
    {
        return NotSupported;
    }

    // Retrieve Path information
    strDirectory = szApplicationDirectory;
    QFileInfo clFileInfo(strDirectory);
    strDirectory = clFileInfo.absolutePath();
    NormalizePath(strDirectory);

    return NoError;
#endif

#if defined __unix__ || __APPLE__ & __MACH__
    // unix: get GEX_PATH environment variable,
    // path to where the STDF Examinator is installed
    char* ptChar;

    ptChar = getenv("GEX_PATH");
    if (ptChar == NULL)
    {
        return NotSupported;
    }

    strDirectory = ptChar;
    NormalizePath(strDirectory);

    return NoError;
#endif
}

/******************************************************************************!
 * \fn NormalizePath
 * \brief Normalize the specified path
 ******************************************************************************/
void CGexSystemUtils::NormalizePath(QString& strPath)
{
    QString strNewPath = strPath;

    // Empty ?
    if (strNewPath.isEmpty())
    {
        return;
    }

    // Replace path separators
#if defined __unix__ || __APPLE__ & __MACH__
    strNewPath.replace("\\", "/");
#else
    strNewPath.replace("/", "\\");
#endif

    // Also remove all space character(s) at the end of the path
    strPath = strNewPath.trimmed();
}

/******************************************************************************!
 * \fn SetGuiStyle
 * \brief Set GUI look and feel
 ******************************************************************************/
void CGexSystemUtils::SetGuiStyle()
{
    // Setup rendering settings
    QString lStyle = "fusion";
#if defined __unix__ || __APPLE__ & __MACH__
#ifdef __sun__
    // Under Solaris-unix, the default windows font (MS Sans Serif 8pixels)
    // doesn't give good result
    QGuiApplication::setFont(QFont("helvetica", 9));
    // Use Plastique style
    QApplication::setStyle(QStyleFactory::create(lStyle));
#endif
#if defined __linux || __APPLE__ & __MACH__
    // Under Linux, the default windows font (MS Sans Serif 8pixels)
    // doesn't give good result
    QGuiApplication::setFont(QFont("helvetica", 9));
    // Use Plastique style
    QApplication::setStyle(QStyleFactory::create(lStyle));
#endif
#endif
#ifdef _WIN32
    switch (QSysInfo::WindowsVersion)
    {
    case QSysInfo::WV_XP:
    case QSysInfo::WV_2003:
        QGuiApplication::setFont(QFont("helvetica", 9));
        QApplication::setStyle(QStyleFactory::create(lStyle));
        break;
    default:
        QApplication::setStyle(QStyleFactory::create(lStyle));
        break;
    }
#endif
}

/******************************************************************************!
 * \fn NormalizePath
 * \brief Normalize the specified path
 ******************************************************************************/
void CGexSystemUtils::NormalizePath(const QString& strPathSrc,
                                    QString& strPathDest)
{
    strPathDest = strPathSrc;
    NormalizePath(strPathDest);
}

/******************************************************************************!
 * \fn NormalizeString
 * \brief Clean the specified string for use in a path
 *        (replace special characters by '_')
 ******************************************************************************/
void CGexSystemUtils::NormalizeString(QString& strString)
{
    QRegExp regExp("[^A-Za-z0-9_]");
    strString.replace(regExp, "_");
}

/******************************************************************************!
 * \fn NormalizeString
 * \brief Clean the specified string for use in a path
 *        (replace special characters by '_')
 ******************************************************************************/
void CGexSystemUtils::NormalizeString(const QString& strStringSrc,
                                      QString& strStringDest)
{
    strStringDest = strStringSrc;
    NormalizeString(strStringDest);
}

/******************************************************************************!
 * \fn GetLocalTime
 * \brief Returns local time for a time_t value
 ******************************************************************************/
QString CGexSystemUtils::GetLocalTime(time_t tTime)
{
    QDateTime clDateTime;

    clDateTime.setTimeSpec(Qt::LocalTime);
    clDateTime.setTime_t(tTime);

    return clDateTime.toString("ddd MMM dd hh:mm:ss yyyy\n");
}

/******************************************************************************!
 * \fn GetUtcTime
 * \brief Returns UTC time for a time_t value
 ******************************************************************************/
QString CGexSystemUtils::GetUtcTime(time_t tTime)
{
    return GetUtcTime(tTime, QString("ddd MMM dd hh:mm:ss yyyy\n"));
}

/******************************************************************************!
 * \fn GetUtcTime
 * \brief Returns UTC time for a time_t value and the given format
 ******************************************************************************/
QString CGexSystemUtils::GetUtcTime(const time_t tTime, const QString & tFormat)
{
    QDateTime clDateTime;

    clDateTime.setTimeSpec(Qt::UTC);
    clDateTime.setTime_t(tTime);

    return clDateTime.toString(tFormat);
}

/******************************************************************************!
 * \fn NormalizeFileName
 * \brief Clean the specified string for use in a file name
 *        (replace special characters by '_')
 ******************************************************************************/
void CGexSystemUtils::NormalizeFileName(QString& strString)
{
    // 1. Remove any 'white space' characeters from start and end,
    // and replace each internal whitespaces with a single space
    strString = strString.simplified();

    // 2. Remove spaces
    strString.remove(' ');

    // 3. Replace unwanted characters with '_'
    QRegExp regExp("[^A-Za-z0-9_\\-\\.#@]");
    strString.replace(regExp, "_");
}

/******************************************************************************!
 * \fn NormalizeFileName
 * \brief Clean the specified string for use in a file name
 *        (replace special characters by '_')
 ******************************************************************************/
void CGexSystemUtils::NormalizeFileName(const QString& strStringSrc,
                                        QString& strStringDest)
{
    strStringDest = strStringSrc;
    NormalizeString(strStringDest);
}


/******************************************************************************!
 * \fn CopyFile
 ******************************************************************************/
bool CGexSystemUtils::CopyFile(const QString& strFrom, const QString& strTo)
{
    //GSLOG(7, "Copy %s to %s...",
    //      strFrom.toLatin1().data(), strTo.toLatin1().data());
#ifdef unix
    size_t lReadSize, lWriteSize;
    char* ptBuffer = 0;
    QFile hTo(strTo);
    FILE* hFromFile, * hToFile;

    // Erase destination if it exists
    if (hTo.exists() == true)
    {
        // Make it writable. So we can erase it
        chmod(strTo.toLatin1().constData(), 0777);
        if (hTo.remove(strTo) != true)
        {
            return false;  // Failed to erase destination file
        }
    }

    // Allocate buffer to perform the file copy
    ptBuffer = new char[1000000];
    if (ptBuffer == NULL)
    {
        return false;  // Memory allocation failure

    }
    hFromFile = fopen(strFrom.toLatin1().constData(), "rb");
    if (hFromFile == NULL)
    {
        delete[] ptBuffer;
        return false;
    }

    hToFile = fopen(strTo.toLatin1().constData(), "wb");
    if (hToFile == NULL)
    {
        delete[] ptBuffer;
        fclose(hFromFile);
        return false;
    }

    while (! feof(hFromFile))
    {
        lReadSize = fread(ptBuffer, sizeof(char), 1000000, hFromFile);
        if (lReadSize > 0)
        {
            lWriteSize = fwrite(ptBuffer, sizeof(char), lReadSize, hToFile);
            if (lWriteSize != lReadSize)
            {
                delete[] ptBuffer;
                fclose(hFromFile);
                fclose(hToFile);
                return false;
            }
        }
        else if (ferror(hFromFile))
        {
            delete[] ptBuffer;
            fclose(hFromFile);
            fclose(hToFile);
            return false;
        }
    }
    delete[] ptBuffer;
    fclose(hFromFile);
    fclose(hToFile);
    return true;
#else
    // Windows Copy (make destination writable first)
    bool bResult;
    QDir d;
    QFile cFile;
    cFile.setPermissions(strTo,
                         QFile::ReadOwner | QFile::WriteOwner |
                         QFile::ReadUser | QFile::WriteUser);
    // Erase file if exists
    d.remove(strTo);
    // Copy file
    bResult = cFile.copy(strFrom, strTo);
    return bResult;
#endif
}

#if defined(linux) || defined(__linux)
/******************************************************************************!
 * \fn GetVmSizeValue
 * \brief get VmRSS, not VmSize
 *        Resident Set Size: number of pages the process has in real memory.
 *        This is just the pages which count toward text, data, or stack space.
 *        This does not include pages which have not been demand-loaded in, or
 *        which are swapped out.
 ******************************************************************************/
int GetVmSizeValue()
{
    char lBuff[4096];
    char* lP;
    int lFd;
    int lLen;

    lFd = ::open("/proc/self/statm", O_RDONLY);
    lLen = ::read(lFd, lBuff, sizeof(lBuff) - 1);
    ::close(lFd);
    lBuff[lLen] = '\0';
    lP = ::strtok(lBuff, " ");
    lP = ::strtok(NULL, " ");

    return (::atoll(lP) * ::getpagesize()) >> 10;  // Bytes ==> KB
}
#endif

#if defined __MACH__
// Found on https://raw.githubusercontent.com/binchewer/power_fixer/master/power_fixer/main.c
static int GetBSDProcessList( kinfo_proc **procList, size_t *procCount)
// Returns a list of all BSD processes on the system.  This routine
// allocates the list and puts it in *procList and a count of the
// number of entries in *procCount.  You are responsible for freeing
// this list (use "free" from System framework).
// On success, the function returns 0.
// On error, the function returns a BSD errno value.
{
    int                 err;
    kinfo_proc *        result;
    bool                 done;
    static const int     name[] = { CTL_KERN , KERN_PROC , KERN_PROC_ALL , 0 };
    // Declaring name as const requires us to cast it when passing it to
    // sysctl because the prototype doesn't include the const modifier.
    size_t               length;

    assert ( procList != NULL );
    assert (*procList == NULL );
    assert (procCount != NULL );

    *procCount = 0 ;

    // We start by calling sysctl with result == NULL and length == 0.
    // That will succeed, and set length to the appropriate length.
    // We then allocate a buffer of that size and call sysctl again
    // with that buffer.  If that succeeds, we're done.  If that fails
    // with ENOMEM, we have to throw away our buffer and loop.  Note
    // that the loop causes use to call sysctl with NULL again; this
    // is necessary because the ENOMEM failure case sets length to
    // the amount of data returned, not the amount of data that
    // could have been returned.

    result = NULL ;
    done = false ;
    do {
        assert (result == NULL );

        // Call sysctl with a NULL buffer.

        length = 0 ;
        err = sysctl ( ( int *) name, ( sizeof (name) / sizeof (*name)) - 1 ,
NULL , &length,
NULL , 0 );
        if (err == - 1 ) {
            err = errno ;
        }

        // Allocate an appropriately sized buffer based on the results
        // from the previous call.

        if (err == 0 ) {
            result = (kinfo_proc *) malloc (length);
            if (result == NULL ) {
                err = ENOMEM ;
            }
        }

        // Call sysctl again with the new buffer.  If we get an ENOMEM
        // error, toss away our buffer and start again.

        if (err == 0 ) {
            err = sysctl ( ( int *) name, ( sizeof (name) / sizeof (*name)) - 1 ,
result, &length,
NULL , 0 );
            if (err == - 1 ) {
                err = errno ;
            }
            if (err == 0 ) {
                done = true ;
            } else if (err == ENOMEM ) {
                assert (result != NULL );
                free (result);
                result = NULL ;
                err = 0 ;
            }
        }
    } while (err == 0 && ! done);

    // Clean up and establish post conditions.

    if (err != 0 && result != NULL ) {
        free (result);
        result = NULL ;
    }
    *procList = result;
    if (err == 0 ) {
        *procCount = length / sizeof ( kinfo_proc );
    }

    assert ( (err == 0 ) == (*procList != NULL ) );

    return err;
}
#endif

QMap<unsigned, QString> CGexSystemUtils::GetProcesses()
{
    QMap<unsigned, QString> lOutput;
    #ifdef WIN32
        // Get the list of process identifiers
        HANDLE hProcessSnap;
        //HANDLE hProcess;
        PROCESSENTRY32 pe32;
        //DWORD dwPriorityClass;

        // Take a snapshot of all processes in the system.
        hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
        if( hProcessSnap == INVALID_HANDLE_VALUE )
        {
            lOutput.insert(0, "CreateToolhelp32Snapshot failed");
            //printf( "CreateToolhelp32Snapshot failed\n");
            return lOutput;
        }
        // Set the size of the structure before using it.
        pe32.dwSize = sizeof( PROCESSENTRY32 );

        // Retrieve information about the first process, and exit if unsuccessful. Dont know why....
        if( !Process32First( hProcessSnap, &pe32 ) )
        {
          //printf( "Process32First failed"); // show cause of failure
          lOutput.insert(0, "Process32First failed");
          CloseHandle( hProcessSnap );          // clean the snapshot object
          return  lOutput;
        }

        // Now walk the snapshot of processes, and
        // display information about each process in turn
        do
        {
            //printf("%ld\t%ls\n", pe32.th32ProcessID, pe32.szExeFile);
            lOutput.insert(pe32.th32ProcessID, QString::fromWCharArray(pe32.szExeFile) );
        }
        while( Process32Next( hProcessSnap, &pe32 ) );
        CloseHandle( hProcessSnap );

        return lOutput;
    #endif

    #if defined __unix
        QDir lProc("/proc");
        QFileInfoList lFIL=lProc.entryInfoList(QStringList("*"), QDir::Dirs);
        foreach(QFileInfo lF, lFIL)
        {
            bool ok=false;
            unsigned pid=lF.fileName().toUInt(&ok);
            if (!ok)
                continue;
            if (!QFile::exists("/proc/"+lF.fileName()+"/cmdline"))
                continue;
            QFile lCmdLine("/proc/"+lF.fileName()+"/cmdline");
            if (!lCmdLine.open(QIODevice::ReadOnly))
                continue;
            QByteArray lBA=lCmdLine.readAll();
            //if (lF.fileName()=="4039") //(lBA.endsWith("gex"))
                //printf("%s %s\n", lF.fileName().toLatin1().data(), lBA.data());
            lOutput.insert(pid, QString(lBA) );
            lCmdLine.close();
        }
        return lOutput;
    #endif

#ifdef __MACH__
    kinfo_proc * result = NULL;
    size_t count = 0;
    if(GetBSDProcessList(&result,&count) == 0)
    {
        for (size_t i = 0; i < count; i++)
        {
            kinfo_proc *proc = NULL;
            proc = &result[i];

            lOutput.insert(proc->kp_proc.p_pid, proc->kp_proc.p_comm);
        }
    }
    free(result);
#endif

    lOutput.insert(0, "Not implemented on this platform");
    return lOutput;
}

/******************************************************************************!
 * \fn GetMemoryInfo
 ******************************************************************************/
QMap<QString, QVariant> CGexSystemUtils::GetMemoryInfo(const bool lRawTest,
                                                       const bool lDetailed)
{
    QMap<QString, QVariant> r;
    static const long u = 1024 * 1024;
    static const QString unit = QString("Mo");

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
    MEMORYSTATUSEX memory_status;
    ZeroMemory(&memory_status, sizeof(MEMORYSTATUSEX));
    memory_status.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memory_status))
    {
        // 6G on my 6G laptop
        r.insert("TotalPhysMem",
                 QString::number((unsigned int)
                                 (memory_status.ullTotalPhys / u)) + "Mo");

        if (lDetailed)
        {
            /*
               MemoryLoad : A number between 0 and 100 that specifies the
               approximate percentage of physical memory that is in use (0
               indicates no memory use and 100 indicates full memory use).

               TotalPhys : The amount of actual physical memory. (returns 6 on
               my a 6G RAM computer).

               AvailPhys : The amount of physical memory currently available.
               Exple : on my 6Go computer running MySql, Firefox, QtCreator, it
               usually returns around 3000Mo.  This is the amount of physical
               memory that can be immediately reused without having to write its
               contents to disk first.  It is the sum of the size of the
               standby, free, and zero lists.

               TotalPageFile : The current committed memory limit for the system
               or the current process, whichever is smaller.  Returns 12Go on my
               6G laptop.

               TotalVirtual : The size of the user-mode portion of the virtual
               address space of the calling process.  This value depends on the
               type of process, the type of processor, and the configuration of
               the operating system.  For example, this value is approximately 2
               GB for most 32-bit processes on an x86 processor and
               approximately 3 GB for 32-bit processes that are large address
               aware running on a system with 4-gigabyte tuning enabled.

               PageFaultCount : The number of page faults.
               PeakWorkingSetSize : The peak working set size.
               WorkingSetSize : The current working set size.
               QuotaPeakPagedPoolUsage : The peak paged pool usage.
               QuotaPagedPoolUsage : The current paged pool usage.
               QuotaPeakNonPagedPoolUsage : The peak nonpaged pool usage.
               QuotaNonPagedPoolUsage : The current nonpaged pool usage.
               PagefileUsage, PrivateUsage : The Commit Charge value for this
               process.  Commit Charge is the total amount of memory that the
               memory manager has committed for a running process.  Windows 7
               and Windows Server 2008 R2 and earlier: PagefileUsage should be
               always zero. Check PrivateUsage instead.
             */
            r.insert("TotalPhys",  // 6G on my 6G laptop
                     (unsigned int) (memory_status.ullTotalPhys / u));
            r.insert("MemoryLoad", (int) memory_status.dwMemoryLoad);
            r.insert("AvailPhys",
                     QString::number((unsigned int)
                                     (memory_status.ullAvailPhys / u)) + "Mo");
            r.insert("TotalPageFile",
                     (unsigned int) (memory_status.ullTotalPageFile / u));
            r.insert("AvailPageFile",
                     (unsigned int) (memory_status.ullAvailPageFile / u));
            r.insert("TotalVirtual",
                     (unsigned int) (memory_status.ullTotalVirtual / u));
            r.insert("AvailVirtual",
                     (unsigned int) (memory_status.ullAvailVirtual / u));
            r.insert("AvailExtendedVirtual",
                     (unsigned int) (memory_status.ullAvailExtendedVirtual /
                                     u));
        }
    }

    PROCESS_MEMORY_COUNTERS pmc;
    ZeroMemory(&pmc, sizeof(PROCESS_MEMORY_COUNTERS));
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)) != 0)
    {
        r.insert("MemUsedByProcess",
                 QString::number((int) (pmc.WorkingSetSize / u)) + "Mo");
        r.insert("MemUsedByProcessInKo",
                 QString::number((int) (pmc.WorkingSetSize/1024)));
        if (lDetailed)
        {
            r.insert("WorkingSetSize", (int) (pmc.WorkingSetSize / u));
            r.insert("PageFaultCount", (int) (pmc.PageFaultCount / u));
            r.insert("PeakWorkingSetSize", (int) (pmc.PeakWorkingSetSize / u));
            r.insert("QuotaPeakPagedPoolUsage",
                     (int) (pmc.QuotaPeakPagedPoolUsage / u));
            r.insert("QuotaPagedPoolUsage",
                     (int) (pmc.QuotaPagedPoolUsage / u));
            r.insert("PagefileUsage", (int) (pmc.PagefileUsage / u));
            r.insert("PeakPagefileUsage", (int) (pmc.PeakPagefileUsage/u) );
            // only in latest version of PSAPI
            //r.insert("PrivateUsage",(int)pmc.PrivateUsage/u);
        }
    }
    else
    {
        r.insert("GetProcessMemoryInfoError", (int) GetLastError());
    }

    PROCESS_MEMORY_COUNTERS_EX pmc_ex; // extended version (no joke)
    ZeroMemory(&pmc_ex, sizeof(PROCESS_MEMORY_COUNTERS_EX));
    if (GetProcessMemoryInfo( GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc_ex, sizeof(pmc_ex))!=0 )
    {
        r.insert("PrivateUsage", (int) (pmc_ex.PrivateUsage/u) );
        r.insert("MemUsedByProcess", (int) (pmc_ex.PrivateUsage/u) ); // Let s use this one vs WorkingSet
        r.insert("MemUsedByProcessInKo", (int) (pmc_ex.PrivateUsage/1024) );
    }
    else
    {
        r.insert("PrivateUsage", "?" );
        //qDebug("GetProcessMemoryInfo failed");
    }


    return r;
#endif

#if defined(linux) || defined(__linux)
    {
        char lBuff[4096];
        char* lP;
        int lFd;
        int lLen;
        unsigned long lMemtotal = 0;
        unsigned long lMemfree = 0;

        lFd = ::open("/proc/meminfo", O_RDONLY);
        lLen = ::read(lFd, lBuff, sizeof(lBuff) - 1);
        ::close(lFd);
        lBuff[lLen] = '\0';
        lP = lBuff;
        lP = ::strstr(lP, "MemTotal:") + sizeof("MemTotal:");
        ::sscanf(lP, "%lu", &lMemtotal);
        lP = ::strstr(lP, "MemFree:") + sizeof("MemFree:");
        ::sscanf(lP, "%lu", &lMemfree);

        r.insert("TotalPhysMem",
                 QString::number((int) (lMemtotal >> 10)) + "Mo");
        r.insert("AvailPhys",
                 QString::number((int) (lMemfree >> 10)) + "Mo");
    }

/* Committed_AS:
    An estimate of how much RAM you would need to make a 99.99% guarantee that
    there never is OOM (out of memory) for this workload. Normally the kernel
    will overcommit memory. That means, say you do a 1GB malloc, nothing
    happens, really. Only when you start USING that malloc memory you will get
    real memory on demand, and just as much as you use. So you sort of take a
    mortgage and hope the bank doesn't go bust. Other cases might include when
    you mmap a file that's shared only when you write to it and you get a
    private copy of that data. While it normally is shared between
    processes. The Committed_AS is a guesstimate of how much RAM/swap you would
    need worst-case.  The amount of memory required to almost never run out of
    memory with the current workload.  Normally, the kernel hands out more
    memory than it actually has with the hope that applications overallocate.
    If all the applications were to use what they allocate, this is the amount
    of physical memory you would need.
 */
    if (lDetailed)
    {
        QProcess p;
        p.start("awk",
                QStringList() << "/Commited_AS/ { print $2 }" <<
                "/proc/meminfo");
        p.waitForFinished();
        QString memory = p.readAllStandardOutput();
        r.insert("Commited_AS", QString::number(
                     (int) (memory.toLong() / 1024)) + "Mo");  // Mo ?
        p.close();
    }

    // Get VmSize
    int lResult = GetVmSizeValue();
    if (lResult > 0)
    {
        r.insert("MemUsedByProcess",
                 QString::number((int) (lResult >> 10)) + "Mo");
    }
    // use pmap -x 23654 ?
    // use more /proc/23654/smaps ?
    // use more /proc/23654/status ? look at VmSize and VmPeak
    // use ps
    // is getrusage() available on all linux ?
    rusage lRU;
    lResult = getrusage(RUSAGE_THREAD, &lRU);
    if (lResult == 0)
    {
        // This is the maximum resident set size used (in kilobytes).
        // For RUSAGE_CHILDREN, this is the resident set size of the largest
        // child, not the maximum resident set size of the process tree.
        //r.insert("MemUsedByProcess", (int) lRU.ru_maxrss/1024);
        if (lDetailed)
        {
            r.insert("maxrss", QString::number(
                         (int) lRU.ru_maxrss / 1024) + unit);
        }
    }

    rlimit lRL;
    // Taille maximum de la mmoire virtuelle du processus en octets.
    // Cette limite affecte les appels  brk(2), mmap(2) et mremap(2),
    // qui chouent avec l'erreur ENOMEM en cas de dpassement de cette limite.
    // De meme, l'extension de la pile automatique chouera (et generera un
    // SIGSEGV qui tuera le processus si aucune pile alternative n'a ete
    // definie par un appel a sigaltstack(2)).
    // Depuis que cette valeur est de type long, sur les machines ou le type
    // long est sur 32 bits, soit cette limite est au plus 2 GiB,
    // soit cette ressource est illimite.
    lResult = getrlimit(RLIMIT_AS, &lRL);
    if (lResult == 0)
    {
        r.insert("RLIMIT_AS_cur",
                 (lRL.rlim_cur ==
                  RLIM_INFINITY) ? "Inf" :
                 (QString::number((int) (lRL.rlim_cur / u)) + unit));
        r.insert("RLIMIT_AS_max",
                 (lRL.rlim_max ==
                  RLIM_INFINITY) ? "Inf" :
                 (QString::number((int) (lRL.rlim_max / u)) + unit));
    }

    // The maximum size of the process's data segment
    // (initialized data, uninitialized data, and heap).
    // This limit affects calls to brk(2) and sbrk(2), which fail with the
    // error ENOMEM upon encountering the soft limit of this resource.
    lResult = getrlimit(RLIMIT_DATA, &lRL);
    if (lResult == 0)
    {
        r.insert("RLIMIT_DATA_cur",
                 (lRL.rlim_cur ==
                  RLIM_INFINITY) ? "Inf" :
                 (QString::number((int) (lRL.rlim_cur / u)) + unit));
        r.insert("RLIMIT_DATA_max",
                 (lRL.rlim_max ==
                  RLIM_INFINITY) ? "Inf" :
                 (QString::number((int) (lRL.rlim_max / u)) + unit));
    }

    // Taille maximale d'un fichier que le processus peut creer.
    // Les tentatives d'extension d'un fichier au-dela de cette limite
    // resultent en un signal SIGXFSZ.
    // Par defaut ce signal termine le processus, mais il peut etre capture,
    // et dans ce cas l'appel systeme concerne (par exemple write(2),
    // truncate(2)) echoue avec l'erreur EFBIG.
    lResult = getrlimit(RLIMIT_FSIZE, &lRL);
    if (lResult == 0)
    {
        r.insert("RLIMIT_FSIZE_cur",
                 (lRL.rlim_cur ==
                  RLIM_INFINITY) ? "Inf" :
                 (QString::number((int) (lRL.rlim_cur / (rlim_t) u)) + unit));
        r.insert("RLIMIT_FSIZE_max",
                 (lRL.rlim_max ==
                  RLIM_INFINITY) ? "Inf" :
                 (QString::number((int) (lRL.rlim_max / (rlim_t) u)) + unit));
    }

    /*
       Le nombre maximal d'octets de memoire que le processus peut verrouiller
       en RAM.  Cette limite est arrondie vers le bas au plus proche multiple de
       la taille de page systeme.  Cette limite affecte les appels mlock(2) et
       mlockall(2) et l'operation MAP_LOCKED de mmap(2).  Depuis Linux 2.6.9,
       cela affecte egalement l'operation SHM_LOCK de shmctl(2), ou est
       configure un maximum sur le nombre total d'octets dans les segments de
       memoire partagee (voir shmget(2)) qui peut etre verrouille par l'UID reel
       du processus appelant.  Les verrous SHM_LOCK de shmctl(2) sont
       comptabilises de maniere separe des verrous memoire par processus
       effectues par mlock(2), mlockall(2) et l'operation MAP_LOCKED de mmap(2)
       ; un processus peut verrouiller autant d'octets jusqu'a sa limite dans
       chacune de ces deux categories.  Dans les noyaux Linux anterieurs la
       version 2.6.9, cette limite controlait la quantite de memoire qu'un
       processus privilgie pouvait verrouiller.  Depuis Linux 2.6.9, il n'y a
       aucune limite sur la quantite de mmoire qu'un processus privilegie peut
       verrouiller, et cette limite concerne plutot les processus non
       privilgies.
     */
    lResult = getrlimit(RLIMIT_MEMLOCK, &lRL);
    if (lResult == 0)
    {
        r.insert("RLIMIT_MEMLOCK_cur",
                 QString::number((int) (lRL.rlim_cur / (rlim_t) u)) + unit);
        r.insert("RLIMIT_MEMLOCK_max",
                 QString::number((int) (lRL.rlim_max / (rlim_t) u)) + unit);
    }

    // Le nombre maximum de processus (ou plus precisement sous Linux,
    // de threads) qui peuvent etre crees pour l'UID reel du processus appelant.
    // Une fois cette limite atteinte, fork(2) echoue avec l'erreur EAGAIN.
    lResult = getrlimit(RLIMIT_NPROC, &lRL);
    if (lResult == 0)
    {
        r.insert("RLIMIT_NPROC_cur", (int) (lRL.rlim_cur));
        r.insert("RLIMIT_NPROC_max", (int) (lRL.rlim_max));
    }

    // Indique la limite (en pages) pour la taille de l'ensemble resident du
    // processus (le nombre de pages de mmoire virtuelle en RAM).
    // Cette limite n'a d'effet que sous Linux 2.4.x o x < 30, et n'affecte
    // que les appels madvise(2) indiquant MADV_WILLNEED.
    lResult = getrlimit(RLIMIT_RSS, &lRL);
    if (lResult == 0)
    {
        r.insert("RLIMIT_RSS_cur", QString::number((int) (lRL.rlim_cur)) + "p");
        r.insert("RLIMIT_RSS_max", QString::number((int) (lRL.rlim_max)) + "p");
    }

    // Taille d'une page en octets. Ne doit pas etre inferieure a 1.
    // (Certains systmes utilisent PAGE_SIZE a la place)
    // 4096o on my 32bit ubuntu 11
    long lSysconfResult = sysconf(_SC_PAGESIZE);  // or PAGE_SIZE
    if (lSysconfResult != -1)
    {
        r.insert("PAGESIZE", QString::number((int) (lSysconfResult)) + "o");
    }

#ifdef _SC_PHYS_PAGES
    lSysconfResult = sysconf(_SC_PHYS_PAGES);
    if (lSysconfResult != -1)
    {
        r.insert("_SC_PHYS_PAGES", QString::number((int) (lSysconfResult)) +
                 "p");
    }
#endif

#ifdef _SC_AVPHYS_PAGES
    // _SC_AVPHYS_PAGES : Le nombre de pages de mmoire physique actuellement
    // disponibles. 1 million on my 32bit 6G RAM Ubuntu 11
    lSysconfResult = sysconf(_SC_AVPHYS_PAGES);
    if (lSysconfResult != -1)
    {
        r.insert("_SC_AVPHYS_PAGES", QString::number(
                     (int) (lSysconfResult)) + "p");
    }
#endif

    struct sysinfo lSysinfo;
    lResult = sysinfo(&lSysinfo);
    if (lResult == 0)
    {
        r.insert("totalram",  // Total usable main memory size
                 (int) ((lSysinfo.totalram * lSysinfo.mem_unit) / u));
        r.insert("freeram",  // Available memory size
                 (int) ((lSysinfo.freeram * lSysinfo.mem_unit) / u));
        r.insert("sharedram",  // Amount of shared memory
                 (int) ((lSysinfo.sharedram * lSysinfo.mem_unit) / u));
        r.insert("totalswap",  // Total swap space size
                 (int) ((lSysinfo.totalswap * lSysinfo.mem_unit) / u));
        r.insert("freeswap",  // swap space still available
                 (int) ((lSysinfo.freeswap * lSysinfo.mem_unit) / u));
        r.insert("totalhigh",  // Total high memory size
                 (int) ((lSysinfo.totalhigh * lSysinfo.mem_unit) / u));
        r.insert("freehigh",  // Available high memory size
                 (int) ((lSysinfo.freehigh * lSysinfo.mem_unit) / u));
        r.insert("Load", (int) lSysinfo.loads[0]);
        // 1, 5, and 15 minute load averages. Seems to be a percent * 10000 ?
    }

    if (lDetailed)
    {
        struct mallinfo lMallinfo = mallinfo();
        // Dynamically allocated address space used by the program, including
        // book-keeping overhead per allocation of sizeof(void *) bytes
        r.insert("uordblks", lMallinfo.uordblks);
    }

    // todo : get stats/info from mmap
    //r.insert("MMAP_THRESHOLD", MMAP_THRESHOLD);
#endif // linux

#if defined(__APPLE__) && defined(__MACH__)
    Q_UNUSED(lDetailed);
    {
        unsigned long lMaxmem = 0;
        size_t lSize;
        kern_return_t lStatus;
        vm_statistics_data_t lVmStats;

        lSize = sizeof(lMaxmem);
        sysctlbyname("hw.memsize", &lMaxmem, &lSize, NULL, 0);

        unsigned int lCount = HOST_VM_INFO_COUNT;
        lStatus = host_statistics(mach_host_self(), HOST_VM_INFO,
                                  (host_info_t) &lVmStats, &lCount);
        if (lStatus == KERN_SUCCESS) {
            r.insert("TotalPhysMem",
                     QString::number((int) (lMaxmem >> 20)) + "Mo");
            r.insert("AvailPhys",
                     QString::number((int) (lVmStats.free_count *
                                            getpagesize() >> 20)) + "Mo");
        }

        kern_return_t lKr;
        struct task_basic_info_64 lTi;
        mach_msg_type_number_t lCount2 = TASK_BASIC_INFO_64_COUNT;

        mach_port_t lTask = mach_task_self();

        lKr = task_info(lTask, TASK_BASIC_INFO_64,
                        (task_info_t) &lTi, &lCount2);
        if (lKr == KERN_SUCCESS) {
            r.insert("MemUsedByProcess",
                     QString::number((int) (lTi.resident_size >> 20)) + "Mo");
        }
    }
#endif

    if (lRawTest)
    {
        void* p = 0;
        void* lastp = 0;
        size_t size = 1024 * 1024;
        p = lastp = calloc(1, size);
        while (p)
        {
            size += 20 * 1024 * 1024;  // Can be more to speed up things
            p = realloc(p, size);
            if (p)
            {
                lastp = p;
            }
        }
        if (lastp)
        {
            free(lastp);
        }
        r.insert("AvailContinuousMem", QString::number(size / u) + unit);
        //GSLOG(7, "Available continuous memory : %d Mo", (size/(1024))/1024);

        int step = 10 * 1024 * 1024;
        QVector<void*> s;
        do
        {
            p = malloc(step);
            if (p)
            {
                s.push_back(p);
            }
        }
        while (p);
        double available_mo =
            ((double) ((double) step * s.size()) / (1024.f)) / 1024.f;
        foreach(p, s)
        free(p);

        r.insert("AvailFragmentedMem", QString::number(available_mo) + unit);
        //GSLOG(7, "Available fragmented memory : %d Mo", (int)available_mo);
    }

    return r;
}
