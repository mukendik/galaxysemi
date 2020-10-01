///////////////////////////////////////////////////////////
// This class reads PC & Unix computer info
// used to know the exact computer details
///////////////////////////////////////////////////////////

#define _GALAXY_SYSTEMINFO_EXPORTS_

#include <sys/types.h>
#include <sys/stat.h> // also available in mingw 64 ? anyway no mkdir in the mingw verison...
#include <sys/time.h> // gettimeofday
#include <unistd.h> // usleep
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
# include <windows.h>
# include <direct.h>
#else
# include <sys/utsname.h>
# include <netdb.h>
# include <ifaddrs.h>
# include <sys/socket.h>
# if defined(__APPLE__) && defined(__MACH__)
#  include <net/if_dl.h>
# else
#  include <linux/if_arp.h>
# endif
#endif
#include "gstdl_utilsdll.h"
#include "gstdl_systeminfo.h"
#include "gstdl_systeminfo_debug.h"  // DEBUG MICROLINEAR

using namespace std;

/* --------------------------------------------------------------------------------------- */
/* PRIVATE TYPES AND DEFINES															   */
/* --------------------------------------------------------------------------------------- */
// Error map
GBEGIN_ERROR_MAP(CGSystemInfo)
    GMAP_ERROR(eReadBasicSystemInfo,"Error reading basic system information (Error code = %d)")
    GMAP_ERROR(eReadNetworkBoardInfo,"Error reading network board information")
    GMAP_ERROR(eReadDiskID,"Error reading disk information")
    GMAP_ERROR(eReadNetworkBoardIDAndDiskID,"Error reading disk and network board information")
GEND_ERROR_MAP(CGSystemInfo)

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CGSystemInfo::CGSystemInfo(void)
{
    // Clears all variables
    m_lNumberOfProcessors	= 0;	// PC: Number of processors
    m_lProcessorType		= 0;	// PC: Processor type
    m_lProcessorLevel		= 0;	// PC: Processor design level
    m_lProcessorRevision	= 0;	// PC: Processor design revision
    m_strHostID				= "?";	// Unix: HostID
    m_strHostIP				= "?";	// IP address
    m_strHostName			= "?";	// Computer name
    m_strAccountName		= "?";	// Login user name.
    m_strDiskID				= "?";	// PC: Inludes the list of disk ID strings (serial#s)
    m_strPlatform			= "?";	// [PC-Windows], [Sun-Solaris] or [HP-UX]
    m_strOS					= "?";	// Windozs NT, SunOS 5.5.1
    m_strNetworkBoardID		= "?";	// Includes 1st Ethernet boardID detected.
    m_strNetworkBoardsIDs	= "?";	// Includes the list of Ethernet boardIDs detected
}

char* CGSystemInfo::GetGalaxySemiTempFolder()
{
    // what about :
    // tmpfile() ?
    // tmpnam() ?
    // mkdtemp ?

    /*
      Try boost::filesystem's temp_directory_path():
        ISO/IEC 9945 (POSIX): The path supplied by the first environment variable found
            in the list TMPDIR, TMP, TEMP, TEMPDIR. If none of these are found, "/tmp".
        Windows: The path reported by the Windows GetTempPath API function.
            Interestingly, Window's GetTempPath uses similar logic to the POSIX version:
            the first environment variable in the list TMP, TEMP, USERPROFILE.
            If none of these are found, it returns the Windows directory.
    */

    static char lTempFolder[1024]="";
    // Just to be sure
    lTempFolder[0]='\0';

    char* lFolder=getenv("TMPDIR"); // seems to be POSIX...
    if (lFolder)
        strcpy(lTempFolder, lFolder);
    else
    {
        #ifdef WIN32
            // On windows XP often equal to : C:\Users\toto\AppData\Local\Temp
            lFolder=getenv("TMP");
            if (lFolder)
                strcpy(lTempFolder, lFolder);
            else
            {
                lFolder=getenv("TEMP");
                if (lFolder)
                    strcpy(lTempFolder, lFolder);
                // todo : try win32 API
                //else
                  //  GetTempPathA();
            }
        #else
            sprintf(lTempFolder, "/tmp");
        #endif
    }

    if (lTempFolder[0]=='\0')
      return (char*)"";

    strcat(lTempFolder, "/GalaxySemi");

    #if defined __unix__ || __APPLE__&__MACH__
        int lR=mkdir(lTempFolder, 0755);
    #else
        int lR=mkdir(lTempFolder);
    #endif

    // errno_t unknown on linux ?
    //errno_t
    int lErrno=errno;

    if (lR!=0 && lErrno!=EEXIST)
    {
        printf("mkdir '%s' failed: res=%d errno=%d \n", lTempFolder, lR, lErrno);
        return (char*)"";
    }
    return lTempFolder;
}

///////////////////////////////////////////////////////////
// Tells if failed/passed reading system info
BOOL CGSystemInfo::ReadSystemInfo(void)
{
    // DEBUG MICROLINEAR
    GSystemInfo_Debug_StartSession();

    // Read basic system info...host name, user, processor type, etc...
    int iBasicInfo = ReadBasicSystemInfo();

    // Read Network board IDs (NAC)...if no board detected, value remains set to '?'
    int iNetwork = ReadNetworkBoardInfo();

    // Get PC-Host ID : built from Serial#s of computer disks...if no disk #, value remains 0.
    int iDisks = ReadPcHostID();

    // Check Error codes
    if(iBasicInfo != READ_SYS_INFO_NOERROR)
    {
        GSET_ERROR1(CGSystemInfo,eReadBasicSystemInfo,NULL,iBasicInfo);
        return FALSE;
    }

#ifndef unix
    // Under Windows, a HostID is enough info!
    if(iDisks == READ_SYS_INFO_NOERROR)
        return TRUE;	// We have a valid HostID...this is enough, don't check the EthernetBoard!
#endif

    // If no Ethernet board + no disk serial numbers on PC computer...FAIL!
    if((iNetwork != READ_SYS_INFO_NOERROR) && (iDisks != READ_SYS_INFO_NOERROR))
    {
        GSET_ERROR0(CGSystemInfo,eReadNetworkBoardIDAndDiskID,NULL);
        return FALSE;
    }

    // We have at least Network board or HostID, that's enough

    // DEBUG MICROLINEAR
    GSystemInfo_Debug_StopSession();

    return TRUE;
}

///////////////////////////////////////////////////////////
// Read basic system info (hostanme, ID, diskID, etc....)
///////////////////////////////////////////////////////////
int	CGSystemInfo::ReadBasicSystemInfo(void)
{
    char	szString[2048];
    int		nReturnCode = READ_SYS_INFO_NOERROR;

    // DEBUG MICROLINEAR
    GSystemInfo_Debug_FunctionEnter("ReadBasicSystemInfo");

#if defined __APPLE__&__MACH__ || __unix__
    // UNIX Platform
    struct passwd	*pPasswd;

    struct passwd	stPasswd;
    char			szBuffer[1024];
#ifdef __sun__
        m_strPlatform = "[Sun-Solaris]";
#endif
#ifdef hpux
        m_strPlatform = "[HP-UX]";
#endif
#if defined __APPLE__&__MACH__
    m_strPlatform = "[Apple-MacOSX]";
#endif
#if defined __linux__
    m_strPlatform = "[Linux]";
#endif

    // Get Unix workstation hostname
    if(gethostname(szString,2047) != 0)
        nReturnCode =  READ_SYS_INFO_ERROR_HOSTNAME;	// ERROR: Failed reading hostname
    else
        m_strHostName = szString;

    // Get current user name.
#if 1 // On Solaris 7 / g++ 3.4.6, same syntax as on Linux
    if(getpwuid_r(getuid(), &stPasswd, szBuffer, 1024, &pPasswd) != 0)
        nReturnCode = READ_SYS_INFO_ERROR_USER;
    else
        m_strAccountName = stPasswd.pw_name;
#else
#if defined __sun__
    if(getpwuid_r(getuid(), &stPasswd, szBuffer, 1024) == NULL)
        nReturnCode = READ_SYS_INFO_ERROR_USER;
    else
        m_strAccountName = stPasswd.pw_name;
#elif defined __linux__ || __APPLE__&__MACH__
    if(getpwuid_r(getuid(), &stPasswd, szBuffer, 1024, &pPasswd) != 0)
        nReturnCode = READ_SYS_INFO_ERROR_USER;
    else
        m_strAccountName = stPasswd.pw_name;
#endif
#endif
    // Get Unix workstation hostID
    long lHostId = gethostid();
    sprintf(szString,"0x%08lx",lHostId);
    m_strHostID = szString;
#endif
#ifdef _WIN32
    SYSTEM_INFO si;
    HKEY hKey;
    DWORD dwBufLen = 2048;	// Maximum buffer size to get user name/computer name

    // PC Platform
    m_strPlatform = "[PC-Windows]";

    // Get PC computer processor info
    GetSystemInfo(&si);
    m_lNumberOfProcessors = si.dwNumberOfProcessors;	// eg: 1
    m_lProcessorType = si.dwProcessorType;			// eg: 586
    m_lProcessorLevel = si.wProcessorLevel;			// eg: 6
    m_lProcessorRevision = si.wProcessorRevision;		// eg: 2954

    // DEBUG MICROLINEAR
    GSystemInfo_Debug_WriteLongVar("NOP", m_lNumberOfProcessors);
    GSystemInfo_Debug_WriteLongVar("PT", m_lProcessorType);
    GSystemInfo_Debug_WriteLongVar("PL", m_lProcessorLevel);
    GSystemInfo_Debug_WriteLongVar("PR", m_lProcessorRevision);

    // Get PC computer name.
    strcpy(szString, "");
    LONG lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
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
    if((lRes != ERROR_SUCCESS) || (*szString == 0))
    {
        // If didn't get any name from registry, try another command!
        if(!GetComputerNameA(szString,&dwBufLen))
            nReturnCode = READ_SYS_INFO_ERROR_HOSTNAME;	// ERROR: Failed reading hostname
        else
            m_strHostName = szString;
    }
    else
        m_strHostName = szString;

    // DEBUG MICROLINEAR
    GSystemInfo_Debug_WriteStringVar("HN", m_strHostName.c_str());

    // Get current user name.
    GetUserNameA(szString,&dwBufLen);
    m_strAccountName = szString;

    // DEBUG MICROLINEAR
    GSystemInfo_Debug_WriteStringVar("AN", m_strAccountName.c_str());
#endif
    // Get IP address
    if(ReadHostIP(m_strHostName, m_strHostIP) == FALSE)
        nReturnCode = READ_SYS_INFO_ERROR_HOSTIP;

    // Get OS
    if(ReadOS(m_strOS) == FALSE)
        nReturnCode = READ_SYS_INFO_ERROR_OS;

    if(m_strHostName.empty() == true)
        m_strHostName="?";
    if(m_strAccountName.empty() == true)
        m_strAccountName="?";

    // DEBUG MICROLINEAR
    GSystemInfo_Debug_WriteStringVar("IP", m_strHostIP.c_str());
    GSystemInfo_Debug_WriteStringVar("OS", m_strOS.c_str());
    GSystemInfo_Debug_WriteStringVar("HN", m_strHostName.c_str());
    GSystemInfo_Debug_WriteStringVar("AN", m_strAccountName.c_str());

    // DEBUG MICROLINEAR
    GSystemInfo_Debug_FunctionExit("ReadBasicSystemInfo", nReturnCode);

    return nReturnCode;
}
///////////////////////////////////////////////////////////
// Read system Ethernet network board ID
///////////////////////////////////////////////////////////
int	CGSystemInfo::ReadNetworkBoardInfo(void)
{
    // DEBUG MICROLINEAR
    GSystemInfo_Debug_FunctionEnter("ReadNetworkBoardInfo");

    clString strBoardFound;

    // Clear buffers.
    m_strNetworkBoardsIDs = "";
    m_strNetworkBoardID = "";

#   if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
    char	szString[2048];

    // Get Host ID (Ethernet Board MAC address)
    GS::StdLib::MibII	m;
    int iCount = m.Init();

    // DEBUG MICROLINEAR
    GSystemInfo_Debug_WriteLongVar("NICCNT", iCount);

    // If no Ethernet board exist...error!
    if (iCount < 0)
    {
        // DEBUG MICROLINEAR
        GSystemInfo_Debug_FunctionExit("ReadNetworkBoardInfo (1)", READ_SYS_INFO_ERROR_NETWORK);

        return READ_SYS_INFO_ERROR_NETWORK;
    }

    iCount = m.GetNICCount(false, false);	// No dialup nor loop back entries: only real network boards.

    // DEBUG MICROLINEAR
    GSystemInfo_Debug_WriteLongVar("NICCNT", iCount);

    // Get board descriptions from Windows.
    GS::StdLib::tSTRUCTNICINFO * pNICInfo = new GS::StdLib::tSTRUCTNICINFO [iCount+1];
    GS::StdLib::tSTRUCTNICINFO * pCurrentNICInfo;
    int iChecksum =0;
    bool bValidMAC=false;

    // For now, until we merge the different ReadNetworkBoardInfo function from several codes,
    // let's use the same method as in int ReadSystemInfo::ReadNetworkBoardInfo(...)
    //iCount = m.GetNICInfo(pNICInfo);
    m.GetNICInfo(pNICInfo);

    // DEBUG MICROLINEAR
    GSystemInfo_Debug_WriteLongVar("NICCNT", iCount);

    // Loop for all boards found.
    for(int iIndex= 0; iIndex < iCount ; iIndex++)
    {
        // Address of structure to process
        pCurrentNICInfo = &pNICInfo[iIndex];

        strBoardFound = "";
        if(pCurrentNICInfo && pCurrentNICInfo->type == 6) //6 = ETHERNET adaptor found
        {
            iChecksum = 0;
            for (UINT i = 0; i < pCurrentNICInfo->MACLength; i++)
            {
                // Build ID as string
                if(i+1 < pCurrentNICInfo->MACLength)
                    sprintf(szString, "%02X-", pCurrentNICInfo->MAC[i]);
                else
                    sprintf(szString, "%02X", pCurrentNICInfo->MAC[i]);
                // Check if MAC is not only FF bytes!
                if(pCurrentNICInfo->MAC[i] != 0xff)
                    bValidMAC = true;
                // update checksum...in case the sum is 0...meaning MAC is only 0s!
                iChecksum += pCurrentNICInfo->MAC[i];
                // Build Network Adaptor MAC address
                strBoardFound += szString;
            }
            // IF MAC address is only FF bytes, or only 0 bytes...ignore this dummy address!
            if(bValidMAC == false)
                iChecksum = 0;
        }
        // If we have found a valid board ID, then we save it!
        if((iChecksum) && (m_strNetworkBoardID.empty() == true))
            m_strNetworkBoardID = strBoardFound;	// This is the 1st Ethernet controller.

        // Keep track of ALL Ethernet boards present...needed for notebook hooked to dosking a station
        m_strNetworkBoardsIDs += strBoardFound;
        if(iIndex+1 < iCount)
            m_strNetworkBoardsIDs += ";";	// Keep list of IDs with ';' separator.
    };
    delete[] pNICInfo; pNICInfo=0;

    // Check if we have found at least one valid entry
    if(m_strNetworkBoardID.empty() == true)
    {
        // DEBUG MICROLINEAR
        GSystemInfo_Debug_FunctionExit("ReadNetworkBoardInfo (2)", READ_SYS_INFO_ERROR_NETWORK);

        return READ_SYS_INFO_ERROR_NETWORK;
    }
#   else
    struct ifaddrs* ifap;
    struct ifaddrs* item;
    unsigned char*  ptr;
    char mac[18];

    if (::getifaddrs(&ifap) != 0)
    {
        return READ_SYS_INFO_ERROR_NETWORK;
    }

    for (item = ifap; item != NULL; item = item->ifa_next)
    {
#       if defined(__APPLE__) && defined(__MACH__)
        if (item->ifa_addr != NULL &&
            item->ifa_addr->sa_family == AF_LINK)
        {
            ptr = (unsigned char*)
                LLADDR((struct sockaddr_dl*) item->ifa_addr);
        }
        else
        {
            continue;
        }
#       else
        if (item->ifa_addr != NULL &&
            item->ifa_addr->sa_family == AF_PACKET)
        {
            ptr = (unsigned char*)
                ((struct sockaddr_ll*) item->ifa_addr)->sll_addr;
        }
        else
        {
            continue;
        }
#       endif

        if (ptr[0] == 0 && ptr[1] == 0 && ptr[2] == 0 &&
            ptr[3] == 0 && ptr[4] == 0 && ptr[5] == 0)
        {
            continue;
        }
        ::sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X",
                ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5]);

        if (m_strNetworkBoardID == "")
        {
            m_strNetworkBoardID = mac;
        }
        m_strNetworkBoardsIDs += mac;
        m_strNetworkBoardsIDs += ";";
    }

    freeifaddrs(ifap);
#   endif

    // DEBUG MICROLINEAR
    GSystemInfo_Debug_FunctionExit("ReadNetworkBoardInfo (3)", READ_SYS_INFO_NOERROR);

    return READ_SYS_INFO_NOERROR;
}

///////////////////////////////////////////////////////////
// Read serial#s of PC disks
///////////////////////////////////////////////////////////
int	CGSystemInfo::ReadPcHostID(void)
{
#ifdef _WIN32
    // Get an HostID computer from a mix of diskSerialNumbers!
   return getHardDriveComputerID();
#endif
    return READ_SYS_INFO_NOERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Description :
//
// Argument(s) :
//      CString& strOS : reference to string to receive OS of host
//
// Return type : BOOL
/////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGSystemInfo::ReadOS(string& strOS)
{
#if defined __unix__ || __APPLE__&__MACH__
    struct utsname name;

    if(uname(&name) == -1)
        return FALSE;

    strOS = name.sysname;
    strOS += " ";
    strOS += name.release;
    strOS += " ";
    strOS += name.version;
    return TRUE;
#endif

#ifdef _WIN32
    // DEBUG MICROLINEAR
    GSystemInfo_Debug_FunctionEnter("ReadOS");

    OSVERSIONINFO osvi;

    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if(GetVersionEx(&osvi) == 0)
    {
        // DEBUG MICROLINEAR
        GSystemInfo_Debug_FunctionExit("ReadOS", FALSE);

        return FALSE;
    }

    if((osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) &&
       (osvi.dwMajorVersion == 4) && (osvi.dwMinorVersion == 0))
    {
        strOS = "Windows 95";
    }
    if((osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) &&
       (osvi.dwMajorVersion == 4) && (osvi.dwMinorVersion == 10))
    {
        strOS = "Windows 98";
    }
    if((osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) &&
       (osvi.dwMajorVersion == 4) && (osvi.dwMinorVersion == 90))
    {
        strOS = "Windows Me";
    }
    if((osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
       (osvi.dwMajorVersion == 3) && (osvi.dwMinorVersion == 51))
    {
        strOS = "Windows NT 3.51";
    }
    if((osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
       (osvi.dwMajorVersion == 4) && (osvi.dwMinorVersion == 0))
    {
        strOS = "Windows NT 4.0";
    }
    if((osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
       (osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion == 0))
    {
        strOS = "Windows 2000";
    }
    if((osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
       (osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion == 1))
    {
        strOS = "Whistler";
    }
    if((osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
       (osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion == 2))
    {
        strOS = "Windows 2003";
    }

    if(strcmp(osvi.szCSDVersion, "") != 0)
    {
        strOS += " ";
        strOS += osvi.szCSDVersion;
    }

    // DEBUG MICROLINEAR
    GSystemInfo_Debug_WriteLongVar("OSPID", osvi.dwPlatformId);
    GSystemInfo_Debug_WriteLongVar("OSMAV", osvi.dwMajorVersion);
    GSystemInfo_Debug_WriteLongVar("OSMIV", osvi.dwMinorVersion);
    GSystemInfo_Debug_WriteStringVar("OSCSDV", osvi.szCSDVersion);

    // DEBUG MICROLINEAR
    GSystemInfo_Debug_FunctionExit("ReadOS", TRUE);

    return TRUE;
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Description :
//
// Argument(s) :
//      CString& strHostIP : reference to string to receive IP address of host
//
// Return type : BOOL
/////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGSystemInfo::ReadHostIP(const string& strHostName, string& strHostIP)
{
    char	szHostIP[256];

#ifdef _WIN32
    WSADATA 	stWinSockData;
    if(WSAStartup(MAKEWORD(1, 1), &stWinSockData) != 0)
        return FALSE;
#endif

    // Get hostent structure (using native unix version of gethostbyname)
    struct hostent *pstHostent = gethostbyname(strHostName.c_str());
    if(pstHostent != NULL)
    {
        sprintf(szHostIP, "%d.%d.%d.%d",(unsigned char)(pstHostent->h_addr_list[0][0]),
                                        (unsigned char)(pstHostent->h_addr_list[0][1]),
                                        (unsigned char)(pstHostent->h_addr_list[0][2]),
                                        (unsigned char)(pstHostent->h_addr_list[0][3]));
        strHostIP = szHostIP;
        return TRUE;
    }

    return FALSE;
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
int CGSystemInfo::ReadPhysicalDriveInNT (void)
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
//			   USHORT *pIdSector = (USHORT *)
//							 ((PSENDCMDOUTPARAMS) IdOutCmd) -> bBuffer;
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
BOOL CGSystemInfo::DoIDENTIFY (HANDLE hPhysicalDriveIOCTL, PSENDCMDINPARAMS pSCIP,
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
char *CGSystemInfo::FindPath(char	*szString)
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
int CGSystemInfo::ReadDrivePortsInWin9X (void)
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
       // Message:information(NULL, "Error",
       //                     "ERROR: Could not open IDE21201.VXD file");
       return done;
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
int CGSystemInfo::ReadIdeDriveAsScsiDriveInNT (void)
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
void CGSystemInfo::PrintIdeInfo (int /*drive*/, DWORD diskdata [256])
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
char *CGSystemInfo::ConvertToString (DWORD diskdata [256], int firstIndex, int lastIndex)
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
int CGSystemInfo::getHardDriveComputerID ()
{
    // DEBUG MICROLINEAR
    GSystemInfo_Debug_FunctionEnter("getHardDriveComputerID");

   char szDebug[512];
   int nStatus;
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
       // DEBUG MICROLINEAR
       nStatus = GetLastError();
       sprintf(szDebug, "Pb in GetVersionEx (1): %d", nStatus);
       GSystemInfo_Debug_WriteString(szDebug);

      // If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.
      osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
      if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) )
      {
           // DEBUG MICROLINEAR
           nStatus = GetLastError();
           sprintf(szDebug, "Pb in GetVersionEx (2): %d", nStatus);
           GSystemInfo_Debug_WriteString(szDebug);

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
       sprintf(szDebug, "ReadPhysicalDriveInNT: %d", (int)done);
       GSystemInfo_Debug_WriteString(szDebug);

        //  this should work in WinNT or Win2K if previous did not work
        //  this is kind of a backdoor via the SCSI mini port driver into
        //     the IDE drives
        done = ReadIdeDriveAsScsiDriveInNT ();
       sprintf(szDebug, "ReadIdeDriveAsScsiDriveInNT: %d", (int)done);
       GSystemInfo_Debug_WriteString(szDebug);
    }
    else
    {
         //  this works under Win9X and calls a VXD
      int attempt = 0;

         //  try this up to 10 times to get a hard drive serial number
      for (attempt = 0;
           attempt < 10 && ! done && 0 == HardDriveSerialNumber [0];
           attempt++)
           {
                done = ReadDrivePortsInWin9X ();
                sprintf(szDebug, "ReadDrivePortsInWin9X: %d", (int)done);
                GSystemInfo_Debug_WriteString(szDebug);
           }
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
       uMsb = (unsigned long)(id >> 32);
       uLsb = (unsigned long)(id & 0xffffffff);
       char	szString[20];
       sprintf(szString,"%lu%lu",uMsb,uLsb);
       m_strHostID = szString;

        // DEBUG MICROLINEAR
        GSystemInfo_Debug_WriteStringVar("HID", m_strHostID.c_str());
        GSystemInfo_Debug_FunctionExit("getHardDriveComputerID", READ_SYS_INFO_NOERROR);

       return READ_SYS_INFO_NOERROR;
   }

    // DEBUG MICROLINEAR
    GSystemInfo_Debug_FunctionExit("getHardDriveComputerID", READ_SYS_INFO_ERROR_NETWORK);

    return READ_SYS_INFO_ERROR_NETWORK;
}

#endif	// _WIN32: Get hostID functions.


uint64_t rdtsc_version2()
{
    uint64_t ret=0;
    # if __WORDSIZE == 64
        asm ("rdtsc; shl $32, %%rdx; or %%rdx, %%rax;"
            : "=A"(ret)
            : /* no input */
            : "%edx"
        );
    #else
        asm ("rdtsc"
            : "=A"(ret)
        );
    #endif
    return ret;
}

unsigned long long int rdtsc(void)
{
    unsigned long long int x;
    __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
    return x;
}

int CGSystemInfo::EstimateCPUFreq()
{
    struct timezone tz;
    struct timeval tvstart, tvstop;
    unsigned long long int cycles[2];
    unsigned long microseconds;
    int mhz=0;

    memset(&tz, 0, sizeof(tz));

    if (gettimeofday(&tvstart, &tz)!=0)
        return -1;
    cycles[0] = rdtsc();
    if (gettimeofday(&tvstart, &tz)!=0)
        return -1;

    if (usleep(250000)!=0)
        return -2;

    if (gettimeofday(&tvstop, &tz)!=0)
        return -1;
    cycles[1] = rdtsc();
    gettimeofday(&tvstop, &tz);

    microseconds = ((tvstop.tv_sec-tvstart.tv_sec)*1000000) + (tvstop.tv_usec-tvstart.tv_usec);

    if (microseconds==0)
        return -3;

    mhz = (int) (cycles[1]-cycles[0]) / microseconds;
    return mhz;
}
