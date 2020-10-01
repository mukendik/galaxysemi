///////////////////////////////////////////////////////////
// This class reads PC & Unix computer info
// used to know the exact computer details
///////////////////////////////////////////////////////////
#include <QMessageBox>
#include <QFile>

#include "read_system_info.h"

#ifdef _WIN32
#include "iptypes.h"
#endif

#if defined(__unix__)
#include <sys/types.h>
#include <pwd.h>
#endif


// in main.cpp
extern void				WriteDebugMessageFile(const QString & strMessage);

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
ReadSystemInfo::ReadSystemInfo()
{
    // Write debug info (if Debug mode enabled)
    WriteDebugMessageFile("ReadSystemInfo::constructor");

    // Clears all variables
    lNumberOfProcessors = 0;	// PC: Number of processors
    lProcessorType		= 0;	// PC: Processor type
    lProcessorLevel		= 0;	// PC: Processor design level
    lProcessorRevision	= 0;	// PC: Processor design revision
    strHostID			= "?";	// Unix: HostID
    strHostName			= "?";	// Computer name
    strAccountName		= "?";	// Login user name.
    strDiskID			= "?";	// PC: Inludes the list of disk ID strings (serial#s)
    strPlatform			= "?";	// [PC-Windows], [Sun-Solaris], [HP-UX] or [Linux]
    strNetworkBoardID	= "?";	// Includes 1st Ethernet boardID detected.
    strNetworkBoardsIDs = "?";	// Includes the list of Ethernet boardIDs detected

    m_nErrorCode = READ_SYS_INFO_NOERROR;
}

///////////////////////////////////////////////////////////
// Load system information
///////////////////////////////////////////////////////////
void ReadSystemInfo::load(const QString & strUserHome)
{
    m_strUserHome = strUserHome;

    // Read basic system info...host name, user, processor type, etc...
    m_nErrorCode = ReadBasicSystemInfo();
    if(m_nErrorCode != READ_SYS_INFO_NOERROR)
        return;

    // Get PC-Host ID : built from Serial#s of computer disks...if no disk #, value remains 0.
    m_nErrorCode = ReadPcHostID();

#ifndef unix
#if 0
    // Under Windows, a HostID is enough info!
    if(m_nErrorCode == READ_SYS_INFO_NOERROR)
        return;	// We have a valid HostID...this is enough, don't check the EthernetBoard!
#endif
#endif

    // Read Network board IDs (NAC)...if no board detected, value remains set to '?'
    m_nErrorCode = ReadNetworkBoardInfo();
}

#ifdef _WIN32
//extern int  GetAdaptersInfo(PIP_ADAPTER_INFO ,unsigned long*);
typedef DWORD (WINAPI* GetAdaptersInfoFunc)(PIP_ADAPTER_INFO pAdapterInfo,  PULONG pOutBufLen);
#endif

///////////////////////////////////////////////////////////
// Tells if failed/passed reading system info
///////////////////////////////////////////////////////////
int	ReadSystemInfo::isSystemInfoAvailable(void)
{
    return m_nErrorCode;
}

#ifdef _WIN32
///////////////////////////////////////////////////////////
// Return the windows version associated with the WinVersion value
///////////////////////////////////////////////////////////
QString ReadSystemInfo::winVersion(QSysInfo::WinVersion version)
{
    QString strWinVersion = "WV_unkown";
    switch(version)
    {
        // MS-DOS-based versions
        case QSysInfo::WV_32s:
                strWinVersion = "WV_32s";
                break;
        case QSysInfo::WV_95:
                strWinVersion = "WV_95";
                break;
        case QSysInfo::WV_98:
                strWinVersion = "WV_98";
                break;
        case QSysInfo::WV_Me:
                strWinVersion = "WV_Me";
                break;
        // NT-based versions
        case QSysInfo::WV_NT:
                strWinVersion = "WV_NT";
                break;
        case QSysInfo::WV_2000:
                strWinVersion = "WV_2000";
                break;
        case QSysInfo::WV_XP:
                strWinVersion = "WV_XP";
                break;
        case QSysInfo::WV_2003:
                strWinVersion = "WV_2003";
                break;
        case QSysInfo::WV_VISTA:
                strWinVersion = "WV_VISTA";
                break;
        case QSysInfo::WV_WINDOWS7:
                strWinVersion = "WV_WINDOWS7";
                break;
        case QSysInfo::WV_WINDOWS8:
                strWinVersion = "WV_WINDOWS8";
                break;
        // CE-based versions
        case QSysInfo::WV_CE:
                strWinVersion = "WV_CE";
                break;
        case QSysInfo::WV_CENET:
                strWinVersion = "WV_CENET";
                break;
        case QSysInfo::WV_CE_5:
                strWinVersion = "WV_CE_5";
                break;
        case QSysInfo::WV_CE_6:
                strWinVersion = "WV_CE_6";
                break;
        default:
                break;
    }
    return strWinVersion;
}
#endif
///////////////////////////////////////////////////////////
// Read basic system info (hostanme, ID, diskID, etc....)
///////////////////////////////////////////////////////////
int	ReadSystemInfo::ReadBasicSystemInfo(void)
{
    #define STRING_SIZE 2047
    char	szString[STRING_SIZE+1];

    // Write debug info (if Debug mode enabled)
    WriteDebugMessageFile("ReadSystemInfo::ReadBasicSystemInfo (enter)");

#ifdef unix
    // UNIX Platform
    #ifdef __sun__
        strPlatform = "[Sun-Solaris]";
    #endif
    #ifdef hpux
        strPlatform = "[HP-UX]";
    #endif
    #ifdef __linux__
        QFile file("/etc/issue");
        if (!file.open(QIODevice::ReadOnly))
            strPlatform = "[Linux]";
        else
        {
            QString strDistribution = QString(file.readLine()).section("\\", 0, 0).trimmed();
            strPlatform = "[Linux: " + strDistribution + "]";
        }
        file.close();
    #endif

    // Get Unix workstation hostname
    if(gethostname(szString,2047) != 0)
        return READ_SYS_INFO_ERROR_HOSTNAME;	// ERROR: Failed reading hostname
    strHostName = szString;

    // Get current user name.
    struct passwd *pstPassWd = getpwuid(geteuid());
    strAccountName = pstPassWd->pw_name;
    // strAccountName = getlogin();

    // Get Unix workstation hostID
    long lHostId = gethostid();
    sprintf(szString,"%ld",lHostId);
    strHostID = szString;
#endif
#ifdef _WIN32
    SYSTEM_INFO si;
    HKEY hKey;
    DWORD dwBufLen = STRING_SIZE;
    DWORD lSize = STRING_SIZE;	// Maximum buffer size to get user name/computer name

    // PC Platform
    strPlatform = "[PC-Windows: " + winVersion(QSysInfo::windowsVersion()) + "]";
//	strPlatform = "[PC-Windows]";

    // Get PC computer processor info
    GetSystemInfo(&si);
    lNumberOfProcessors = si.dwNumberOfProcessors;	// eg: 1
    lProcessorType = si.dwProcessorType;			// eg: 586
    lProcessorLevel = si.wProcessorLevel;			// eg: 6
    lProcessorRevision = si.wProcessorRevision;		// eg: 2954

    // Get PC computer name.
    int lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        REGSTR_PATH_COMPUTRNAME,
        0,
        KEY_QUERY_VALUE,
        &hKey);
    if(lRes == ERROR_SUCCESS)
    {
        lRes = RegQueryValueEx(hKey,
                    REGSTR_VAL_COMPUTRNAME,
                    NULL,
                    NULL,
                    (LPBYTE)szString,
                    &dwBufLen);

        RegCloseKey(hKey);
    }
    if(lRes != ERROR_SUCCESS)
    {
        WriteDebugMessageFile("ReadSystemInfo::ReadBasicSystemInfo: failed to read hostname with RegOpenKey !");
        return READ_SYS_INFO_ERROR_HOSTNAME;	// ERROR: Failed reading hostname
        // To Do : try the Qt solution :
        //QString hn=QHostInfo::localHostName();
        //WriteDebugMessageFile(QString("ReadSystemInfo::ReadBasicSystemInfo: hostname according Qt : %1").arg(hn));
        //strcpy(szString, hn.toLatin1().data());
    }

    // Convert computer name string from Unicode to ASCII (1 byte/char)
    if(dwBufLen>=2 && szString[1]==0)
    {
        for(lRes=2;lRes<(int)dwBufLen;lRes+=2)
            szString[lRes/2] = szString[lRes];
    }
    szString[lRes/2] = 0;
    // If didn't get any name from registry, try another command!
    if(*szString == 0)
      GetComputerNameA(szString,&lSize);
    strHostName = szString;

    // Get current user name.
    GetUserNameA(szString,&lSize);
    strAccountName = szString;
#endif

    if(strHostName.isEmpty() == true)
        strHostName="?";
    if(strAccountName.isEmpty() == true)
        strAccountName="?";

    // Write debug info (if Debug mode enabled)
    WriteDebugMessageFile("ReadSystemInfo::ReadBasicSystemInfo (exit)");

    return READ_SYS_INFO_NOERROR;
}

///////////////////////////////////////////////////////////
// Read system Ethernet network board ID
///////////////////////////////////////////////////////////
int	ReadSystemInfo::ReadNetworkBoardInfo(void)
{
    QString		strTraceMessage;
    QString		strBoardFound;

    // Write debug info (if Debug mode enabled)
    WriteDebugMessageFile("ReadSystemInfo::ReadNetworkBoardInfo (enter)");

    // Clear buffers.
    strNetworkBoardsIDs = "";
    strNetworkBoardID = "";

#ifdef _WIN32
    char	szString[2048];

    // Get Host ID (Ethernet Board MAC address)
    MibII	m;
    int iCount;
    m.Init();

    iCount = m.GetNICCount(false, false);	// No dialup nor loop back entries: only real network boards.
    if(iCount <= 0 || iCount > 7)
        goto check_net_adaptors;	// Try a different technic to read network adaptors (if more than 7, as otherwise it crashes with this old technique).

    // Trace message
    strTraceMessage = "ReadSystemInfo::ReadNetworkBoardInfo - " + QString::number(iCount) + " adapters detected";
    WriteDebugMessageFile(strTraceMessage);

    // Get board descriptions from Windows.
    tSTRUCTNICINFO * m_pNICInfo;
    tSTRUCTNICINFO * pNetInfo;
    int iChecksum;
    bool bValidMAC;;
    int iIndex;

    // Set variables
    iChecksum = 0;
    bValidMAC = false;
    m_pNICInfo = new tSTRUCTNICINFO [iCount+1];

    m.GetNICInfo(m_pNICInfo);

    // Loop for all boards found.
    for(iIndex= 0; iIndex < iCount ; iIndex++)
    {
        // Trace message
        strTraceMessage = "ReadSystemInfo::ReadNetworkBoardInfo - Adapter#" + QString::number(iIndex) + "...";
        WriteDebugMessageFile(strTraceMessage);

        // Address of structure to process
        pNetInfo = &m_pNICInfo[iIndex];

        strBoardFound = "";
        if(pNetInfo->type == 6) //6 = ETHERNET adaptor found
        {
            iChecksum = 0;
            for (UINT i = 0; i < pNetInfo->MACLength; i++)
            {
                // Build ID as string
                sprintf(szString, "%02X-", pNetInfo->MAC[i]);
                // Check if MAC is not only FF bytes!
                if(pNetInfo->MAC[i] != 0xff)
                    bValidMAC = true;
                // update checksum...in case the sum is 0...meaning MAC is only 0s!
                iChecksum += pNetInfo->MAC[i];
                // Build Network Adaptor MAC address
                strBoardFound += szString;
            }
            // IF MAC address is only FF bytes, or only 0 bytes...ignore this dummy address!
            if(bValidMAC == false)
                iChecksum = 0;

            // Trace message
            strTraceMessage = "ReadSystemInfo::ReadNetworkBoardInfo - Adapter#" + QString::number(iIndex) + "...decoded: " + strBoardFound;
            WriteDebugMessageFile(strTraceMessage);
        }

        // If we have found a valid board ID, then we save it!...unless alreay in our list
        if(iChecksum && (strNetworkBoardsIDs.indexOf(strBoardFound) < 0))
        {
            if(strNetworkBoardID.isEmpty() == true)
                strNetworkBoardID = strBoardFound;	// This is the 1st Ethernet controller.

            // Keep track of ALL Ethernet boards present...needed for notebook hooked to dosking a station
            strNetworkBoardsIDs += strBoardFound;
            strNetworkBoardsIDs += ";";	// Keep list of IDs with ';' separator.
        }
    };

    // Trace message
    strTraceMessage = "ReadSystemInfo::ReadNetworkBoardInfo - Adapters decoded:" + strNetworkBoardsIDs;
    WriteDebugMessageFile(strTraceMessage);

    delete m_pNICInfo;
//////////////////////////////////////////////////

    // If Windows NT or higher....second chance to find network boards!
check_net_adaptors:
    PIP_ADAPTER_INFO pAdapterInfo;
    PIP_ADAPTER_INFO pAdapter = NULL;
    DWORD dwRetVal = 0;

    // Trace message
    strTraceMessage = "ReadSystemInfo::ReadNetworkBoardInfo - calling iphlpapi.dll...";
    WriteDebugMessageFile(strTraceMessage);

    HINSTANCE dllLibrary  = LoadLibraryA("iphlpapi.dll") ;
    if (dllLibrary  < (HINSTANCE) HINSTANCE_ERROR)
        goto exit_check_net_adaptors;

   GetAdaptersInfoFunc GetAdaptersInfo;
   GetAdaptersInfo = NULL; /* pointer to function */

    GetAdaptersInfo = (GetAdaptersInfoFunc) GetProcAddress(dllLibrary, "GetAdaptersInfo");
    if (!GetAdaptersInfo)
        goto exit_check_net_adaptors;

    pAdapterInfo = (IP_ADAPTER_INFO *) malloc( sizeof(IP_ADAPTER_INFO) );
    unsigned long	ulOutBufLen;
    ulOutBufLen = sizeof(IP_ADAPTER_INFO);

    // Make an initial call to GetAdaptersInfo to get
    // the necessary size into the ulOutBufLen variable
    if (GetAdaptersInfo( pAdapterInfo, &ulOutBufLen) != ERROR_SUCCESS)
    {
        // BG: plantage en mode Debug...
        //GlobalFree (pAdapterInfo);
        free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO *) malloc (ulOutBufLen);
    }

    if ((dwRetVal = GetAdaptersInfo( pAdapterInfo, &ulOutBufLen)) == NO_ERROR)
    {
        // Trace message
        strTraceMessage = "ReadSystemInfo::ReadNetworkBoardInfo - iphlpapi.dll detecting adaptors...";
        WriteDebugMessageFile(strTraceMessage);

      pAdapter = pAdapterInfo;
      iIndex = 0;
      while (pAdapter)
      {
        // Trace message
        strTraceMessage = "ReadSystemInfo::ReadNetworkBoardInfo - iphlpapi.dll reading adaptor#" + QString::number(iIndex);
        WriteDebugMessageFile(strTraceMessage);

        //fprintf(hFile,"\tAdapter Name: \t%s\n", pAdapter->AdapterName);
        //fprintf(hFile,"\tAdapter Desc: \t%s\n", pAdapter->Description);
        //fprintf(hFile,"\tAdapter Addr: \t");

          // Only read boards with address of 6 bytes or more ( less bytes are invalid adaptors)
         if(pAdapter->AddressLength >= 6)
         {
            // Clear variables.
            iChecksum = 0;
            strBoardFound = "";

            bool bIsValidMacAddress = true;

            // Ignore NULL addresses returned by other network interfaces
            if ((pAdapter->Address[0] == 0x00 &&
                     pAdapter->Address[1] == 0x00 &&
                     pAdapter->Address[2] == 0x00 &&
                     pAdapter->Address[3] == 0x00 &&
                     pAdapter->Address[4] == 0x00 &&
                     pAdapter->Address[5] == 0x00) ||
            // Ignore Microsoft Loopback Adapter
                    (pAdapter->Address[0] == 0x02 &&
                     pAdapter->Address[1] == 0x00 &&
                     pAdapter->Address[2] == 0x4c &&
                     pAdapter->Address[3] == 0x4f &&
                     pAdapter->Address[4] == 0x4f &&
                     pAdapter->Address[5] == 0x50))
            {
                bIsValidMacAddress = false;
            }

            for (UINT i = 0; i < pAdapter->AddressLength; i++)
            {
                // Build ID as string
                sprintf(szString, "%02X-", pAdapter->Address[i]);

                // update checksum...in case the sum is 0...meaning MAC is only 0s!
                iChecksum += pAdapter->Address[i];

                // Build Network Adaptor MAC address
                strBoardFound += szString;

                // Trace message
                strTraceMessage = "ReadSystemInfo::ReadNetworkBoardInfo - iphlpapi.dll Adapter#" + QString::number(iIndex) + "...decoded: " + strBoardFound;
                WriteDebugMessageFile(strTraceMessage);
            }

            // If we have found a valid board ID, then we save it!...unless alreay in our list
            if(iChecksum && (strNetworkBoardsIDs.indexOf(strBoardFound) < 0))
            {
                if(strNetworkBoardID.isEmpty() && bIsValidMacAddress)
                    strNetworkBoardID = strBoardFound;	// This is the 1st Ethernet controller.

                // Keep track of ALL Ethernet boards present...needed for notebook hooked to dosking a station
                strNetworkBoardsIDs += strBoardFound;
                strNetworkBoardsIDs += ";";	// Keep list of IDs with ';' separator.
            }
         }

        pAdapter = pAdapter->Next;
        iIndex++;
      }

    // BG: free allocated memory
    free(pAdapterInfo);

    // Trace message
    strTraceMessage = "ReadSystemInfo::ReadNetworkBoardInfo - iphlpapi.dll Adapters decoded:" + strNetworkBoardsIDs;
    WriteDebugMessageFile(strTraceMessage);

    }
exit_check_net_adaptors:

    // Write debug info (if Debug mode enabled)
    WriteDebugMessageFile("ReadSystemInfo::ReadNetworkBoardInfo (exit)");

    ////////////////////////////////////////////////////
    // Check if we have found at least one valid entry
  if (strNetworkBoardID.isEmpty() == true) {
    WriteDebugMessageFile("READ_SYS_INFO_ERROR_NETWORK 1");
        return READ_SYS_INFO_ERROR_NETWORK;
  }
#endif
#ifdef unix
    char	szString[2048];
    char	szLine[2048];
    char	*ptChar;
    FILE	*hFile;
    int		iTotalBoards;
    int		iIndex,iLength;
        int             iErrorCode;
    time_t	lStartTime=time(NULL);
    QString strString;

#ifdef __linux__
#if 0
        bool bCommandExists=false;
        hFile = fopen("/sbin/ifconfig", "r");
        if(hFile)
        {
        bCommandExists = true;
        fclose(hFile);
        }
        hFile=fopen("/root/gexlm_debug.txt", "w");
        if(hFile)
        {
        fprintf(hFile, "/sbin/ifconfig command status = %d\n", (int)bCommandExists);
        fclose(hFile);
        }
#endif // 0

    // Under linux, do not use 'arp' command but 'ifconfig' instead
    strString = "/sbin/ifconfig -a > ";

#else
    // Get list of Ethernet adaptors detected for this computer.
    strString = "/usr/sbin/arp ";
    strString += strHostName;
    strString += " > ";
#endif

    // HOME must exist!
    ptChar = getenv("HOME");
  if (ptChar == NULL) {
    WriteDebugMessageFile("READ_SYS_INFO_ERROR_NETWORK 2");
        return READ_SYS_INFO_ERROR_NETWORK;
  }

    sprintf(szString,"%s/gex_list_adaptors.txt",ptChar);
    unlink(szString);	// Remove file.

    // Under Solaris, HP-UX: Makes string '/usr/sbin/arp -a | grep <hostname> > <$HOME>/gex_list_adaptors.txt'
    // Under linux: Makes string '/sbin/ifconfig  > <$HOME>/gex_list_adaptors.txt'
    strString += szString;
    iErrorCode = system(strString.toLatin1().constData());
  if (iErrorCode == -1) {
    WriteDebugMessageFile("READ_SYS_INFO_ERROR_NETWORK 3");
        return READ_SYS_INFO_ERROR_NETWORK;
  }

    // Wait until file with list file of boards created.
  if (WEXITSTATUS(iErrorCode)) {
    // Nothing to do
  }
    // Read file and see how boards found (must have at least one!)
read_file:
    // Check if timeout...it took over 4 seconds to check processes...and still not okay!
  if (time(NULL) - lStartTime > 4) {
    WriteDebugMessageFile("READ_SYS_INFO_ERROR_NETWORK 4");
        return READ_SYS_INFO_ERROR_NETWORK;
  }

    iTotalBoards = 0;
    hFile = fopen(szString,"r");
    while(!feof(hFile))
    {
        *szLine = 0;
        // Read one line of the Boards file. File include a string like
        // <Hostname> <Ip address> at <Physical address> <Flags>
        // pluton (193.1.1.3) at 8:00:46:65:34 permanent physical
    if (fgets(szLine, 2047, hFile) == NULL) {
      if (ferror(hFile)) {
        WriteDebugMessageFile("READ_SYS_INFO_ERROR_NETWORK 5");
        return READ_SYS_INFO_ERROR_NETWORK;
      } else {
        break;
      }
    }
        strBoardFound = szLine;

        // Find first digit of PhysicalAddress
#ifdef __linux__
        // Parsing line from 'ifconfig' output...so first see if we are on the line holding the Ethernet board address
        iIndex = strBoardFound.indexOf("HWaddr", 0, Qt::CaseInsensitive);
        if(iIndex >= 0)
        {
            // Remove string priod to the 'HWAddr' string
            strBoardFound = strBoardFound.mid(iIndex);

            // Point to first digit of the board address
            iIndex = strBoardFound.indexOf(':')-2;

        }
#else
        // Parsing line from 'arp' output: Point to first digit of the board address
        iIndex = strBoardFound.indexOf(':')-2;
#endif

                if(iIndex > 0 && strBoardFound.isNull() == false && strBoardFound.isEmpty() == false)
        {
            // Extract board address + rest of string
            strBoardFound = strBoardFound.mid(iIndex);
            // Extract board address from the string (remove all what is AFTER)
            iLength = strBoardFound.lastIndexOf(':');
            iLength = iLength+3;	// Compute string length to extract.
            strBoardFound = strBoardFound.mid(0,iLength);

            // Get Board address, without any white characters.
            strBoardFound = strBoardFound.trimmed();
            // Save this board address
            if(strNetworkBoardID.isEmpty() == true)
                strNetworkBoardID = strBoardFound;	// This is the 1st Ethernet controller.
            // Keep track of ALL Ethernet boards present...needed for notebook hooked to dosking a station
            strNetworkBoardsIDs += strBoardFound;
            strNetworkBoardsIDs += ";";	// Keep list of IDs with ';' separator.
            if(strBoardFound.isEmpty() == false)
                iTotalBoards++;
        }
    };
    fclose(hFile);
    // If we don't have any board...loop until found, or timeout!
    if(iTotalBoards == 0)
        goto read_file;
    unlink(szString);	// Remove file.

    // Write debug info (if Debug mode enabled)
    WriteDebugMessageFile("ReadSystemInfo::ReadNetworkBoardInfo (exit)");

    // Check if we have found at least one valid entry
  if (strNetworkBoardID.isEmpty() == true) {
    WriteDebugMessageFile("READ_SYS_INFO_ERROR_NETWORK 6");
        return READ_SYS_INFO_ERROR_NETWORK;
  }
#endif
    return READ_SYS_INFO_NOERROR;
}

///////////////////////////////////////////////////////////
// Read serial#s of PC disks
///////////////////////////////////////////////////////////
int	ReadSystemInfo::ReadPcHostID(void)
{
    int	iStatus = READ_SYS_INFO_NOERROR;

    // Write debug info (if Debug mode enabled)
    WriteDebugMessageFile("ReadSystemInfo::ReadPcHostID (enter)");

#ifdef _WIN32
    // Get an HostID computer from a mix of diskSerialNumbers!
   iStatus = getHardDriveComputerID();
#endif

    // Write debug info (if Debug mode enabled)
    WriteDebugMessageFile("ReadSystemInfo::ReadPcHostID (exit)");

    return iStatus;
}

#define  TITLE   "DiskId32"

///////////////////////////////////////////////////////////


#ifdef _WIN32

//  Required to ensure correct PhysicalDrive IOCTL structure setup
#pragma pack(1)
// All functions to get an HostID computer from a mix of diskSerialNumbers!
///////////////////////////////////////////////////////////
// Read NT drive info
///////////////////////////////////////////////////////////
int ReadSystemInfo::ReadPhysicalDriveInNT (void)
{
   int done = FALSE;
   int drive = 0;

   for (drive = 0; drive < MAX_IDE_DRIVES; drive++)
   {
      HANDLE hPhysicalDriveIOCTL = 0;

         //  Try to get a handle to PhysicalDrive IOCTL, report failure
         //  and exit if can't.
      char driveName [256];

      sprintf (driveName, "\\\\.\\PhysicalDrive%d", drive);

         //  Windows NT, Windows 2000, must have admin rights
      hPhysicalDriveIOCTL = CreateFileA (driveName,
                               GENERIC_READ | GENERIC_WRITE,
                               FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                               OPEN_EXISTING, 0, NULL);
      // if (hPhysicalDriveIOCTL == INVALID_HANDLE_VALUE)
      //    printf ("Unable to open physical drive %d, error code: 0x%lX\n",
      //            drive, GetLastError ());

      if (hPhysicalDriveIOCTL != INVALID_HANDLE_VALUE)
      {
         GETVERSIONOUTPARAMS VersionParams;
         DWORD               cbBytesReturned = 0;

            // Get the version, etc of PhysicalDrive IOCTL
         memset ((void*) &VersionParams, 0, sizeof(VersionParams));

         if ( ! DeviceIoControl (hPhysicalDriveIOCTL, DFP_GET_VERSION,
                   NULL,
                   0,
                   &VersionParams,
                   sizeof(VersionParams),
                   &cbBytesReturned, NULL) )
         {
            // printf ("DFP_GET_VERSION failed for drive %d\n", i);
            // continue;
         }

            // If there is a IDE device at number "i" issue commands
            // to the device
         if (VersionParams.bIDEDeviceMap > 0)
         {
            BYTE             bIDCmd = 0;   // IDE or ATAPI IDENTIFY cmd
            SENDCMDINPARAMS  scip;
            //SENDCMDOUTPARAMS OutCmd;

            // Now, get the ID sector for all IDE devices in the system.
               // If the device is ATAPI use the IDE_ATAPI_IDENTIFY command,
               // otherwise use the IDE_ATA_IDENTIFY command
            bIDCmd = (VersionParams.bIDEDeviceMap >> drive & 0x10) ? \
                      IDE_ATAPI_IDENTIFY : IDE_ATA_IDENTIFY;

            memset (&scip, 0, sizeof(scip));
            memset (IdOutCmd, 0, sizeof(IdOutCmd));

            if ( DoIDENTIFY (hPhysicalDriveIOCTL,
                       &scip,
                       (PSENDCMDOUTPARAMS)&IdOutCmd,
                       (BYTE) bIDCmd,
                       (BYTE) drive,
                       &cbBytesReturned))
            {
               DWORD diskdata [256];
               int ijk = 0;
            //USHORT *pIdSector = (USHORT *)
            //  ((PSENDCMDOUTPARAMS) IdOutCmd) -> bBuffer;
                           PSENDCMDOUTPARAMS pSendCmdOutParams = (PSENDCMDOUTPARAMS) IdOutCmd;
                           USHORT *pIdSector = (USHORT *) pSendCmdOutParams->bBuffer;

               for (ijk = 0; ijk < 256; ijk++)
                  diskdata [ijk] = pIdSector [ijk];

               PrintIdeInfo (drive, diskdata);

               done = TRUE;
            }
        }

         CloseHandle (hPhysicalDriveIOCTL);
      }
   }

   return done;
}


///////////////////////////////////////////////////////////
// Query Drive...
// FUNCTION: Send an IDENTIFY command to the drive
// bDriveNum = 0-3
// bIDCmd = IDE_ATA_IDENTIFY or IDE_ATAPI_IDENTIFY
///////////////////////////////////////////////////////////
BOOL ReadSystemInfo::DoIDENTIFY (HANDLE hPhysicalDriveIOCTL, PSENDCMDINPARAMS pSCIP,
                 PSENDCMDOUTPARAMS pSCOP, BYTE bIDCmd, BYTE bDriveNum,
                 PDWORD lpcbBytesReturned)
{
      // Set up data structures for IDENTIFY command.
   pSCIP -> cBufferSize = IDENTIFY_BUFFER_SIZE;
   pSCIP -> irDriveRegs.bFeaturesReg = 0;
   pSCIP -> irDriveRegs.bSectorCountReg = 1;
   pSCIP -> irDriveRegs.bSectorNumberReg = 1;
   pSCIP -> irDriveRegs.bCylLowReg = 0;
   pSCIP -> irDriveRegs.bCylHighReg = 0;

      // Compute the drive number.
   pSCIP -> irDriveRegs.bDriveHeadReg = 0xA0 | ((bDriveNum & 1) << 4);

      // The command can either be IDE identify or ATAPI identify.
   pSCIP -> irDriveRegs.bCommandReg = bIDCmd;
   pSCIP -> bDriveNumber = bDriveNum;
   pSCIP -> cBufferSize = IDENTIFY_BUFFER_SIZE;

   return ( DeviceIoControl (hPhysicalDriveIOCTL, DFP_RECEIVE_DRIVE_DATA,
               (LPVOID) pSCIP,
               sizeof(SENDCMDINPARAMS) - 1,
               (LPVOID) pSCOP,
               sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1,
               lpcbBytesReturned, NULL) );
}


///////////////////////////////////////////////////////////
// Returns the path of the given path+file name.
///////////////////////////////////////////////////////////
char *ReadSystemInfo::FindPath(char	*szString)
{
    int		iIndex;

    iIndex = strlen(szString);
    if(iIndex < 1)
        return NULL;

    iIndex--;
    do
    {
        if(szString[iIndex] == '\\' || szString[iIndex] == ':' || szString[iIndex] == '/')
            return &szString[iIndex];
        iIndex--;
    }
    while(iIndex >= 0);
    return NULL;
}

///////////////////////////////////////////////////////////
// Read Windows9X drive
///////////////////////////////////////////////////////////
int ReadSystemInfo::ReadDrivePortsInWin9X (void)
{
    int done = FALSE;

   HANDLE VxDHandle = 0;
   pt_IdeDInfo pOutBufVxD = 0;
   DWORD lpBytesReturned = 0;

    //  set the thread priority high so that we get exclusive access to the disk
   SetPriorityClass (GetCurrentProcess (), REALTIME_PRIORITY_CLASS);

      // 1. Make an output buffer for the VxD
   rt_IdeDInfo info;
   pOutBufVxD = &info;

      // *****************
      // KLUDGE WARNING!!!
      // HAVE to zero out the buffer space for the IDE information!
      // If this is NOT done then garbage could be in the memory
      // locations indicating if a disk exists or not.
   ZeroMemory (&info, sizeof(info));

   // 1. Try to load the VxD
   //  must use the short file name path to open a VXD file
   VxDHandle = CreateFileA ("\\\\.\\IDE21201.VXD", 0, 0, 0,
                            0, FILE_FLAG_DELETE_ON_CLOSE, 0);

   if (VxDHandle != INVALID_HANDLE_VALUE)
   {
         // 2. Run VxD function
      DeviceIoControl (VxDHandle, m_cVxDFunctionIdesDInfo,
                    0, 0, pOutBufVxD, sizeof(pt_IdeDInfo), &lpBytesReturned, 0);

         // 3. Unload VxD
      CloseHandle (VxDHandle);
   }
   else
   {
       QMessageBox::information(NULL,"Error","ERROR: Could not open IDE21201.VXD file");
       exit(0);
   }

      // 4. Translate and store data
   unsigned long int i = 0;
   for (i=0; i<8; i++)
   {
      if((pOutBufVxD->DiskExists[i]) && (pOutBufVxD->IDEExists[i/2]))
      {
            DWORD diskinfo [256];
            for (int j = 0; j < 256; j++)
                diskinfo [j] = pOutBufVxD -> DisksRawInfo [i * 256 + j];

            // process the information for this buffer
           PrintIdeInfo (i, diskinfo);
            done = TRUE;
      }
   }

   //  reset the thread priority back to normal
   SetPriorityClass (GetCurrentProcess (), NORMAL_PRIORITY_CLASS);

   return done;
}


///////////////////////////////////////////////////////////
// Read NT SCSI drive
///////////////////////////////////////////////////////////
int ReadSystemInfo::ReadIdeDriveAsScsiDriveInNT (void)
{
   int done = FALSE;
   int controller = 0;

   for (controller = 0; controller < 2; controller++)
   {
      HANDLE hScsiDriveIOCTL = 0;
      char   driveName [256];

         //  Try to get a handle to PhysicalDrive IOCTL, report failure
         //  and exit if can't.
      sprintf (driveName, "\\\\.\\Scsi%d:", controller);

         //  Windows NT, Windows 2000, any rights should do
      hScsiDriveIOCTL = CreateFileA (driveName,
                               GENERIC_READ | GENERIC_WRITE,
                               FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                               OPEN_EXISTING, 0, NULL);
      // if (hScsiDriveIOCTL == INVALID_HANDLE_VALUE)
      //    printf ("Unable to open SCSI controller %d, error code: 0x%lX\n",
      //            controller, GetLastError ());

      if (hScsiDriveIOCTL != INVALID_HANDLE_VALUE)
      {
         int drive = 0;

         for (drive = 0; drive < 2; drive++)
         {
            char buffer [sizeof (SRB_IO_CONTROL) + SENDIDLENGTH];
            SRB_IO_CONTROL *p = (SRB_IO_CONTROL *) buffer;
            SENDCMDINPARAMS *pin =
                   (SENDCMDINPARAMS *) (buffer + sizeof (SRB_IO_CONTROL));
            DWORD dummy;

            memset (buffer, 0, sizeof (buffer));
            p -> HeaderLength = sizeof (SRB_IO_CONTROL);
            p -> Timeout = 10000;
            p -> Length = SENDIDLENGTH;
            p -> ControlCode = IOCTL_SCSI_MINIPORT_IDENTIFY;
            strncpy ((char *) p -> Signature, "SCSIDISK", 8);

            pin -> irDriveRegs.bCommandReg = IDE_ATA_IDENTIFY;
            pin -> bDriveNumber = drive;

            if (DeviceIoControl (hScsiDriveIOCTL, IOCTL_SCSI_MINIPORT,
                                 buffer,
                                 sizeof (SRB_IO_CONTROL) +
                                         sizeof (SENDCMDINPARAMS) - 1,
                                 buffer,
                                 sizeof (SRB_IO_CONTROL) + SENDIDLENGTH,
                                 &dummy, NULL))
            {
               SENDCMDOUTPARAMS *pOut =
                    (SENDCMDOUTPARAMS *) (buffer + sizeof (SRB_IO_CONTROL));
               IDSECTOR *pId = (IDSECTOR *) (pOut -> bBuffer);
               if (pId -> sModelNumber [0])
               {
                  DWORD diskdata [256];
                  int ijk = 0;
                  USHORT *pIdSector = (USHORT *) pId;

                  for (ijk = 0; ijk < 256; ijk++)
                     diskdata [ijk] = pIdSector [ijk];

                  PrintIdeInfo (controller * 2 + drive, diskdata);

                  done = TRUE;
               }
            }
         }
         CloseHandle (hScsiDriveIOCTL);
      }
   }

   return done;
}

///////////////////////////////////////////////////////////
// Print info
///////////////////////////////////////////////////////////
void ReadSystemInfo::PrintIdeInfo(int /*drive*/, DWORD diskdata[256])
{
   char string1 [1024];

   //  copy the hard drive serial number to the buffer
   strcpy (string1, ConvertToString (diskdata, 10, 19));
   if (0 == HardDriveSerialNumber [0] &&
            //  serial number must be alphanumeric
            //  (but there can be leading spaces on IBM drives)
       (isalnum (string1 [0]) || isalnum (string1 [19])))
      strcpy (HardDriveSerialNumber, string1);
}


///////////////////////////////////////////////////////////
// Convert buffer to string
///////////////////////////////////////////////////////////
char *ReadSystemInfo::ConvertToString (DWORD diskdata [256], int firstIndex, int lastIndex)
{
   static char string [1024];
   int index = 0;
   int position = 0;

      //  each integer has two characters stored in it backwards
   for (index = firstIndex; index <= lastIndex; index++)
   {
         //  get high byte for 1st character
      string [position] = (char) (diskdata [index] / 256);
      position++;

         //  get low byte for 2nd character
      string [position] = (char) (diskdata [index] % 256);
      position++;
   }

      //  end the string
   string [position] = '\0';

      //  cut off the trailing blanks
   for (index = position - 1; index > 0 && ' ' == string [index]; index--)
      string [index] = '\0';

   return string;
}


///////////////////////////////////////////////////////////
// Computes a PC_hostID
///////////////////////////////////////////////////////////
int ReadSystemInfo::getHardDriveComputerID ()
{
   int done = FALSE;
   __int64 id = 0;
   OSVERSIONINFOEX osvi;

   strcpy (HardDriveSerialNumber, "");

   // Try calling GetVersionEx using the OSVERSIONINFOEX structure.
   // If that fails, try using the OSVERSIONINFO structure.
   memset (&osvi, 0, sizeof (OSVERSIONINFOEX));
   osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

   if( !GetVersionEx ((OSVERSIONINFO *) &osvi))
   {
      // If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.
      osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
      if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) )
      {
             // Failed reading the Windows version...force it to Win95!
            osvi.dwPlatformId = VER_PLATFORM_WIN32_WINDOWS;
            osvi.dwMajorVersion = 4;
            osvi.dwMinorVersion = 0;
      }
   }

    if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
    {
        //  this works under WinNT4 or Win2K if you have admin rights
        done = ReadPhysicalDriveInNT ();

        //  this should work in WinNT or Win2K if previous did not work
        //  this is kind of a backdoor via the SCSI mini port driver into
        //     the IDE drives
        done = ReadIdeDriveAsScsiDriveInNT ();
    }
    else
    {
         //  this works under Win9X and calls a VXD
      int attempt = 0;

         //  try this up to 10 times to get a hard drive serial number
      for (attempt = 0;
           attempt < 10 && ! done && 0 == HardDriveSerialNumber [0];
           attempt++)
         done = ReadDrivePortsInWin9X ();
    }

   if (done)
   {
      char *p = HardDriveSerialNumber;


      //  ignore first 5 characters from western digital hard drives if
      //  the first four characters are WD-W
      if ( ! strncmp (HardDriveSerialNumber, "WD-W", 4))
          p += 5;
      for ( ; p && *p; p++)
      {
         if ('-' == *p) continue;
         id *= 10;
         switch (*p)
         {
            case '0': id += 0; break;
            case '1': id += 1; break;
            case '2': id += 2; break;
            case '3': id += 3; break;
            case '4': id += 4; break;
            case '5': id += 5; break;
            case '6': id += 6; break;
            case '7': id += 7; break;
            case '8': id += 8; break;
            case '9': id += 9; break;
            case 'a': case 'A': id += 10; break;
            case 'b': case 'B': id += 11; break;
            case 'c': case 'C': id += 12; break;
            case 'd': case 'D': id += 13; break;
            case 'e': case 'E': id += 14; break;
            case 'f': case 'F': id += 15; break;
            case 'g': case 'G': id += 16; break;
            case 'h': case 'H': id += 17; break;
            case 'i': case 'I': id += 18; break;
            case 'j': case 'J': id += 19; break;
            case 'k': case 'K': id += 20; break;
            case 'l': case 'L': id += 21; break;
            case 'm': case 'M': id += 22; break;
            case 'n': case 'N': id += 23; break;
            case 'o': case 'O': id += 24; break;
            case 'p': case 'P': id += 25; break;
            case 'q': case 'Q': id += 26; break;
            case 'r': case 'R': id += 27; break;
            case 's': case 'S': id += 28; break;
            case 't': case 'T': id += 29; break;
            case 'u': case 'U': id += 30; break;
            case 'v': case 'V': id += 31; break;
            case 'w': case 'W': id += 32; break;
            case 'x': case 'X': id += 33; break;
            case 'y': case 'Y': id += 34; break;
            case 'z': case 'Z': id += 35; break;
         }
      }

       // Build 64bits long integer string.
       unsigned long uMsb,uLsb;
       uMsb = id >> 32;
       uLsb = id & 0xffffffff;
       char	szString[20];
       sprintf(szString,"%lu%lu",uMsb,uLsb);
       strHostID = szString;
       return READ_SYS_INFO_NOERROR;
   }
   WriteDebugMessageFile("READ_SYS_INFO_ERROR_NETWORK 7");
   return READ_SYS_INFO_ERROR_NETWORK;
}

#endif	// _WIN32: Get hostID functions.
