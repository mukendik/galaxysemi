// ginibuffer.h: interface for the CGCrypto class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _GALAXY_SYSTEMINFO_HEADER_
#define _GALAXY_SYSTEMINFO_HEADER_

#include <string>

#include <gstdl_errormgr.h>
/*
#if defined(_WIN32)
#	if defined(_GALAXY_SYSTEMINFO_EXPORTS_)
#		define GSYSTEMINFO_API __declspec(dllexport)
#	elif defined _GALAXY_STDUTILS_DLL_MODULE
#		define GSYSTEMINFO_API
#	else	// _GALAXY_SYSTEMINFO_EXPORTS_
#		define GSYSTEMINFO_API __declspec(dllimport)
#	endif // _GALAXY_SYSTEMINFO_EXPORTS_
#else
#	define GSYSTEMINFO_API
#endif // _MSC_VER
*/

#define GSYSTEMINFO_API

#define	clString std::string

#ifdef _WIN32
    #include <windows.h>
	#include <regstr.h>
	#include "gstdl_hostid_mibaccess.h"
#endif
#if defined __unix__ || __APPLE__&__MACH__
	#include <unistd.h> 
	#include <pwd.h>
#endif

#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#if 0 // Not needed (generates errors) on Solaris 7 / g++ 3.4.6
// Under Solaris (with Sun's C++ compiler and GNU g++ compiler), need to declare these externals!
#if (defined unix && defined sun)
extern "C"
{
extern int gethostname(char *name, int namelen); 
extern long gethostid(void); 
extern char *getlogin(void);
extern struct passwd *getpwuid_r(uid_t uid, struct passwd *pwd, char *buffer, int buflen);
}
#endif
#endif

#define	READ_SYS_INFO_NOERROR			0	// Success!...no error
#define	READ_SYS_INFO_ERROR_HOSTNAME	1	// Failed reading computer name
#define	READ_SYS_INFO_ERROR_NETWORK		2	// Failed reading Ethernet board info.
#define	READ_SYS_INFO_ERROR_HOSTIP		3	// Failed reading computer IP address
#define READ_SYS_INFO_ERROR_OS			4	// Failed reading OS
#define READ_SYS_INFO_ERROR_USER		5	// Failed reading user name

#if _WIN32
 // For PC platform only: compute HostID
#define  SENDIDLENGTH  sizeof (SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE


//  ---------------------------------------------------
// (* Output Bbuffer for the VxD (rt_IdeDinfo record) *)
typedef struct _rt_IdeDInfo_
{
    BYTE IDEExists[4];
    BYTE DiskExists[8];
    WORD DisksRawInfo[8*256];
} rt_IdeDInfo, *pt_IdeDInfo;


// (* IdeDinfo "data fields" *)
typedef struct _rt_DiskInfo_
{
   BOOL DiskExists;
   BOOL ATAdevice;
   BOOL RemovableDevice;
   WORD TotLogCyl;
   WORD TotLogHeads;
   WORD TotLogSPT;
   char SerialNumber[20];
   char FirmwareRevision[8];
   char ModelNumber[40];
   WORD CurLogCyl;
   WORD CurLogHeads;
   WORD CurLogSPT;
} rt_DiskInfo;

#define  m_cVxDFunctionIdesDInfo  1

//  Max number of drives assuming primary/secondary, master/slave topology
#define  MAX_IDE_DRIVES  4
#define  IDENTIFY_BUFFER_SIZE  512


   //  IOCTL commands
#define  DFP_GET_VERSION          0x00074080
#define  DFP_SEND_DRIVE_COMMAND   0x0007c084
#define  DFP_RECEIVE_DRIVE_DATA   0x0007c088

#define  FILE_DEVICE_SCSI              0x0000001b
#define  IOCTL_SCSI_MINIPORT_IDENTIFY  ((FILE_DEVICE_SCSI << 16) + 0x0501)
#define  IOCTL_SCSI_MINIPORT 0x0004D008  //  see NTDDSCSI.H for definition



   //  GETVERSIONOUTPARAMS contains the data returned from the 
   //  Get Driver Version function.
typedef struct _GETVERSIONOUTPARAMS
{
   BYTE bVersion;      // Binary driver version.
   BYTE bRevision;     // Binary driver revision.
   BYTE bReserved;     // Not used.
   BYTE bIDEDeviceMap; // Bit map of IDE devices.
   DWORD fCapabilities; // Bit mask of driver capabilities.
   DWORD dwReserved[4]; // For future use.
} GETVERSIONOUTPARAMS, *PGETVERSIONOUTPARAMS, *LPGETVERSIONOUTPARAMS;


   //  Bits returned in the fCapabilities member of GETVERSIONOUTPARAMS 
#define  CAP_IDE_ID_FUNCTION             1  // ATA ID command supported
#define  CAP_IDE_ATAPI_ID                2  // ATAPI ID command supported
#define  CAP_IDE_EXECUTE_SMART_FUNCTION  4  // SMART commannds supported

#ifndef _WINIOCTL_
   //  IDE registers
typedef struct _IDEREGS
{
   BYTE bFeaturesReg;       // Used for specifying SMART "commands".
   BYTE bSectorCountReg;    // IDE sector count register
   BYTE bSectorNumberReg;   // IDE sector number register
   BYTE bCylLowReg;         // IDE low order cylinder value
   BYTE bCylHighReg;        // IDE high order cylinder value
   BYTE bDriveHeadReg;      // IDE drive/head register
   BYTE bCommandReg;        // Actual IDE command.
   BYTE bReserved;          // reserved for future use.  Must be zero.
} IDEREGS, *PIDEREGS, *LPIDEREGS;
#endif

#ifndef _WINIOCTL_
   //  SENDCMDINPARAMS contains the input parameters for the 
   //  Send Command to Drive function.
typedef struct _SENDCMDINPARAMS
{
   DWORD     cBufferSize;   //  Buffer size in bytes
   IDEREGS   irDriveRegs;   //  Structure with drive register values.
   BYTE bDriveNumber;       //  Physical drive number to send 
                            //  command to (0,1,2,3).
   BYTE bReserved[3];       //  Reserved for future expansion.
   DWORD     dwReserved[4]; //  For future use.
   BYTE      bBuffer[1];    //  Input buffer.
} SENDCMDINPARAMS, *PSENDCMDINPARAMS, *LPSENDCMDINPARAMS;
#endif

   //  Valid values for the bCommandReg member of IDEREGS.
#define  IDE_ATAPI_IDENTIFY  0xA1  //  Returns ID sector for ATAPI.
#define  IDE_ATA_IDENTIFY    0xEC  //  Returns ID sector for ATA.

#ifndef _WINIOCTL_
   // Status returned from driver
typedef struct _DRIVERSTATUS
{
   BYTE  bDriverError;  //  Error code from driver, or 0 if no error.
   BYTE  bIDEStatus;    //  Contents of IDE Error register.
                        //  Only valid when bDriverError is SMART_IDE_ERROR.
   BYTE  bReserved[2];  //  Reserved for future expansion.
   DWORD  dwReserved[2];  //  Reserved for future expansion.
} DRIVERSTATUS, *PDRIVERSTATUS, *LPDRIVERSTATUS;
#endif
#ifndef _WINIOCTL_

   // Structure returned by PhysicalDrive IOCTL for several commands
typedef struct _SENDCMDOUTPARAMS
{
   DWORD         cBufferSize;   //  Size of bBuffer in bytes
   DRIVERSTATUS  DriverStatus;  //  Driver status structure.
   BYTE          bBuffer[1];    //  Buffer of arbitrary length in which to store the data read from the                                                       // drive.
} SENDCMDOUTPARAMS, *PSENDCMDOUTPARAMS, *LPSENDCMDOUTPARAMS;
#endif

   // The following struct defines the interesting part of the IDENTIFY
   // buffer:
typedef struct _IDSECTOR
{
   USHORT  wGenConfig;
   USHORT  wNumCyls;
   USHORT  wReserved;
   USHORT  wNumHeads;
   USHORT  wBytesPerTrack;
   USHORT  wBytesPerSector;
   USHORT  wSectorsPerTrack;
   USHORT  wVendorUnique[3];
   CHAR    sSerialNumber[20];
   USHORT  wBufferType;
   USHORT  wBufferSize;
   USHORT  wECCSize;
   CHAR    sFirmwareRev[8];
   CHAR    sModelNumber[40];
   USHORT  wMoreVendorUnique;
   USHORT  wDoubleWordIO;
   USHORT  wCapabilities;
   USHORT  wReserved1;
   USHORT  wPIOTiming;
   USHORT  wDMATiming;
   USHORT  wBS;
   USHORT  wNumCurrentCyls;
   USHORT  wNumCurrentHeads;
   USHORT  wNumCurrentSectorsPerTrack;
   ULONG   ulCurrentSectorCapacity;
   USHORT  wMultSectorStuff;
   ULONG   ulTotalAddressableSectors;
   USHORT  wSingleWordDMA;
   USHORT  wMultiWordDMA;
   BYTE    bReserved[128];
} IDSECTOR, *PIDSECTOR;


typedef struct _SRB_IO_CONTROL
{
   ULONG HeaderLength;
   UCHAR Signature[8];
   ULONG Timeout;
   ULONG ControlCode;
   ULONG ReturnCode;
   ULONG Length;
} SRB_IO_CONTROL, *PSRB_IO_CONTROL;

#endif	// #if _WIN32: Get hostID for PC platform only.

class GSYSTEMINFO_API CGSystemInfo
{
public:
	GDECLARE_ERROR_MAP(CGSystemInfo)
	{
		eReadBasicSystemInfo,			// Error reading basic system information
		eReadNetworkBoardInfo,			// Error reading network board information
		eReadDiskID,					// Error reading Disk ID (PC)
		eReadNetworkBoardIDAndDiskID	// Error reading Disk ID and network board information
	}
	GDECLARE_END_ERROR_MAP(CGSystemInfo)

	CGSystemInfo(void);
    BOOL ReadSystemInfo(void);
    char* GetGalaxySemiTempFolder();

    //! \brief Estimate the CPU/Core speed in mghz
    //! \return Return the freq in mghz or a negative number if failure
    //! \attention It is just an estimate, not the real value read from the hardware/motherboard/OS
    int EstimateCPUFreq();

	clString	m_strPlatform;			// Platform type: [PC-Windows], [Sun-Solaris] or [HP-UX]
    //! \brief Operating system: Windows NT, SunOS 5.5.1
    clString	m_strOS;
	clString	m_strHostName;			// Computer name
	clString	m_strAccountName;			// Login name
	clString	m_strNetworkBoardID;		// Network Adaptor MAC address
	clString	m_strNetworkBoardsIDs;	// List of Ethernet MAC addresses with ';' separator
	clString	m_strDiskID;				// PC: String list of Disk serial numbers
	clString	m_strHostID;				// Unix: Host ID, PC: computed from disksSerialNumbers.
	clString	m_strHostIP;				// IP address of host
	long		m_lNumberOfProcessors;	// PC, eg: 1
	long		m_lProcessorType;			// PC, eg: 586
	long		m_lProcessorLevel;		// PC, eg: 6
	long		m_lProcessorRevision;		// PC, eg: 2954
private:
    //! \brief The OS temp folder. Empty first, then seached at first Get.
    clString mOSTempFolder;
	int		ReadBasicSystemInfo(void);
	int		ReadNetworkBoardInfo(void);
	int		ReadPcHostID(void);
	BOOL	ReadHostIP(const std::string& strHostName, std::string& strHostIP);
	BOOL	ReadOS(std::string& strOS);
#if _WIN32
	// Functions/buffers used to compute the PC_HostID
	USHORT IdOutCmd [(sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE) >> 1];
	char HardDriveSerialNumber [1024];
	int getHardDriveComputerID ();
    int ReadPhysicalDriveInNT (void);
    int ReadDrivePortsInWin9X (void);
    int ReadIdeDriveAsScsiDriveInNT (void);
	char *FindPath(char	*szString);
	char *ConvertToString (DWORD diskdata [256], int firstIndex, int lastIndex);
	void PrintIdeInfo (int drive, DWORD diskdata [256]);
	BOOL DoIDENTIFY (HANDLE, PSENDCMDINPARAMS, PSENDCMDOUTPARAMS, BYTE, BYTE,PDWORD);
#endif
};

#endif // _GALAXY_SYSTEMINFO_HEADER_
