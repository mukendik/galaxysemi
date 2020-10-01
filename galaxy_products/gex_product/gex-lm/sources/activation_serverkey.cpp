#include <stdio.h>
#ifdef __unix__
#include <stdlib.h>
#include <unistd.h> 
#endif
#include <time.h> 

#include <QtGui>
#include <QMessageBox>

#include <gqtl_sysutils.h>
#include <gqtl_skin.h>
#include <gex_shared.h>
#include "activation_serverkey.h"
#include "gexlm.h"
#include "cryptofile.h"			// File is in upper folder GEX


///////////////////////////////////////////////////////////
// Extern functions
///////////////////////////////////////////////////////////
// main.cpp
extern void		LogMessage(QString strMessage, bool bQWarning);
extern void		WriteDebugMessageFile(const QString & strMessage);
extern QString			strUserHome;				// User Home directory
///////////////////////////////////////////////////////////
// Extern variables
///////////////////////////////////////////////////////////
// from gexlm.cpp
extern char		*szAppFullName;
extern long		lPlatformRestriction;	// 0=Gex, 4=Gex-DB, etc...
// from gexlm_server.cpp
extern int		iMaximumLicenses;		// Maximum concurrent licenses allowed
extern int		iProductID;				// 0=Examinator, 1=Examinator for credence, ...,4=ExaminatorDB
extern int		iEditionType;			// Standard Edition
extern int		iMonitorProducts;		// Total Products allowed in Monitoring
extern int		iOptionalModules;		// Bit0=Plugin support, Bit1=PAT support,Bit2=Y123Web mode,Bit3=AllTimezones accepted
extern QString	strExpiration;			// License expiration date
extern QString	m_strMaintenanceExpiration;// License expiration date
extern QString	strLicenseID;			// License Key ID
extern int		iAllowedTimeShift;		// Allowed tile shift in minutes
extern int		iMaxVersion_Maj;		// Max allowed GEX version (major)
extern int		iMaxVersion_Min;		// Max allowed GEX version (minor)

// main.cpp
extern bool		bDisabledSupportPerpetualLicense;

///////////////////////////////////////////////////////////
// Constructor.
///////////////////////////////////////////////////////////
LicenseFile::LicenseFile(const QString & strUserHome, const QString & strApplicationDir)
{
	m_strUserHome = strUserHome;
	m_strApplicationDir = strApplicationDir;

	cSystemInfo.load(m_strUserHome);
}

///////////////////////////////////////////////////////////
// Destructor.
///////////////////////////////////////////////////////////
LicenseFile::~LicenseFile()
{
}

///////////////////////////////////////////////////////////
// Check if this is a perpetual license, if so, ensure it has
// active maintenance to allow running this release!
///////////////////////////////////////////////////////////
bool LicenseFile::CheckDisabledSupportPerpetualLicense(QString strCurrentLicense)
{
	QString strMessage = "LicenseFile::CheckDisabledSupportPerpetualLicense(" + strCurrentLicense + ")";
	WriteDebugMessageFile(strMessage);
	QDate clCompilationDate = QDate::fromString(QString(__DATE__).simplified(), "MMM d yyyy");
	
	if (!m_strMaintenanceExpiration.isEmpty())
	{
		if (clCompilationDate <= QDate::fromString(m_strMaintenanceExpiration, "yyyy M d"))
			return true;
		else
			return false;	// This is a hold license without maintenance contract: refuse to run it!
	}
		

	// Holds list of perpetual licenses:
	// o format is <Product Key>[|<Maintenance expiration date>]
	// o if no maintenance date, the corrsponding Product key has no maintenance and should be rejected
	// o if a maintenance date is specified, the key is rejected if this date is prior the compilation date
	const char* szPerpetualLicenses[] = {
		"GEX-GFC-139104885",				// PAT-Man: ADI / External Foundry
		"GEX-CMA-120877281",				// PAT-Man: ADI / Cambridge
		"GEX-DBM-126143538",				// PAT-Man: ADI / Philippines
		"GEX-NBA-120877116|Nov 30 2012",	// PAT-Man: ADI / Wilmington
		"GEX-OKL-114138061",				// PAT-Man: ADI / Analog Limerick (Main server)
		"GEX-CCD-143336257",				// PAT-Man: ADI / Analog Limerick (Backup server)
		"GEX-JBB-129753400|Dec 31 2010",	// Yield-Man: TSMC Taiwan
		"GEX-MCF-256083275|Jun 15 2011",	// GEX-PAT : SSSL, UK
		"GEX-EPI-247023891|Mar 01 2011",	// GEX-PAT : Advanced Silicon
		

		//"GEX-IIB-107897255",				// GEXP-1: HongKong Science & Technology parks corp.

		//"GEX-HGE-234980486|Nov 15 2009",	// Galaxy - GEXDBP (+plugin).
		//"GEX-KGD-110592905",				// Galaxy - GEXDBP (+plugin).
		//"GEX-DNF-63227378",				// Examinator: GEXDBP-SS5 (Plugin + MO + PAT).
		//"GEX-BME-186791136",				// PAT-Man: Micronas
		//"GEX-EPI-247023891|Mar 02 2011",	// Examinator-PAT: Advanced Silicons

		""	// End of list
	};

	QDate clMaintenanceExpirationDate;
	QString	strProductKey, strMaintenanceExpiration;

	const char* ptLicense;
	int	iIndex = 0;
	do
	{
		// Get license key from list
		ptLicense=szPerpetualLicenses[iIndex++];

		// Check if current license is in the exclusion list...
		if(*ptLicense)
		{
			strProductKey = QString(ptLicense).section("|", 0, 0);
			strMaintenanceExpiration = QString(ptLicense).section("|",1,1).simplified();
			if(strProductKey.toLower() == strCurrentLicense.toLower())
			{
				strMessage = "Perpetual license rejected because no valid maintenance contract";
				if(!strMaintenanceExpiration.isEmpty())
				{
					clMaintenanceExpirationDate = QDate::fromString(strMaintenanceExpiration, "MMM dd yyyy");
					if(!clMaintenanceExpirationDate.isValid())
						clMaintenanceExpirationDate = QDate::fromString(strMaintenanceExpiration, "MMM d yyyy");
					if(clMaintenanceExpirationDate.isValid())
					{
						if(clCompilationDate <= clMaintenanceExpirationDate)
							return true;
						strMessage += " (maintenance expired: " + clMaintenanceExpirationDate.toString("MMM dd yyyy") + " < ";
						strMessage += clCompilationDate.toString("MMM dd yyyy") + ")";
					}
					else
						strMessage += " (invalid maintenance expiration date: " + strMaintenanceExpiration + ")";
				}
				strMessage += ".";
				WriteDebugMessageFile(strMessage);

				return false;	// This is a hold license without maintenance contract: refuse to run it!
			}
		}

	}
	while(*ptLicense);
	return true;
}

///////////////////////////////////////////////////////////
// Encrypt then write string to file.
///////////////////////////////////////////////////////////
void LicenseFile::WriteCryptedFile(char *szString)
{
	CCryptoFile ascii;	// Buffer used for crypting data!
	char*		szAsciiKey;

	ascii.SetBufferFromBuffer(szString, (strlen(szString)+1)*sizeof(char));
	ascii.Encrypt_Buffer((unsigned char*)GEX_CRYPTO_KEY, GEX_KEY_BYTES);
	ascii.GetBufferToHexa(&szAsciiKey);
	fprintf(hCryptedFile,"%s\n",szAsciiKey);
    free(szAsciiKey);
}

///////////////////////////////////////////////////////////
// Encrypt then write data to file.
///////////////////////////////////////////////////////////
void LicenseFile::WriteCryptedFile(long lData)
{
	CCryptoFile ascii;	// Buffer used for crypting data!
	char	szString[2048];

	sprintf(szString,"%ld",lData);
	WriteCryptedFile(szString);
}

///////////////////////////////////////////////////////////
// Read one line from License file...ignore blank lines
///////////////////////////////////////////////////////////
bool LicenseFile::ReadFileLine(char *szLine,int iMaxChar,FILE *hFile)
{
  *szLine = 0;

  QString		strLine;
  QByteArray	pString;
  
  // Loop until non-empty line found...or end of file!
  while(!feof(hFile))
  {
	if(fgets(szLine,iMaxChar,hFile) == NULL)
		return false;	// failed reading line...probably end of file!

	// Remove all leading spaces
	strLine = QString(szLine).trimmed();
	if(strLine.isEmpty() == false)
	{
		pString = strLine.toLatin1();
		strcpy(szLine, pString.constData());
		return true;	// Found valid (non empty string)
	}
  };

  // end of file
  return false;
}

///////////////////////////////////////////////////////////
// Decrypt a string from license file.
///////////////////////////////////////////////////////////
void LicenseFile::ReadCryptedFile(char *szString)
{
	CCryptoFile ascii;	// Buffer used for de-crypting data!
	char*		szAsciiKey;

	*szString=0;
	if(ReadFileLine(szString,2047,hCryptedFile) == false)
		return;	// Error reading in file...
	ascii.SetBufferFromHexa(szString);
	ascii.Decrypt_Buffer((unsigned char*)GEX_CRYPTO_KEY,GEX_KEY_BYTES);
	ascii.GetBuffer(&szAsciiKey);
	strcpy(szString,szAsciiKey);

	// Update checksum (all characters without CR-LF marker)
    char* szAsciiKeyTemp = szAsciiKey;
	while(*szAsciiKey)
	{
		uChecksum += *szAsciiKey;
		szAsciiKey++;
	};
    free(szAsciiKeyTemp);
}

///////////////////////////////////////////////////////////
// Decrypt a (long int) from file.
///////////////////////////////////////////////////////////
void LicenseFile::ReadCryptedFile(long *lData)
{
	CCryptoFile ascii;	// Buffer used for de-crypting data!
	char	szString[2048];

	ReadCryptedFile(szString);
	*lData = 0;
	sscanf(szString,"%ld",lData);
}


///////////////////////////////////////////////////////////
// Check if License file proposed/found is correct...
///////////////////////////////////////////////////////////
bool LicenseFile::IsCorrectLicenseFile(void)
{
	if(LoadLicenseFile() == true)
	{
		WriteDebugMessageFile("IsCorrectLicenseFile() - License file loaded.");
		bool bAcceptLicense = CheckDisabledSupportPerpetualLicense(strLicenseID);
		bDisabledSupportPerpetualLicense = !bAcceptLicense;
		return bAcceptLicense;
	}

	WriteDebugMessageFile("IsCorrectLicenseFile() - Error loading license file.");
	return false;	
}

///////////////////////////////////////////////////////////
// Read Workstation info (hostanme, ID, diskID, etc....)
///////////////////////////////////////////////////////////
bool LicenseFile::LoadLicenseFile(void)
{
	// Check license file and see if valid!
	QString		strFileName, strString;
	QByteArray	pString;
	char		szString[2048];
	long		lData;
	QString		strMessage;
	int iFileVersion;

	WriteDebugMessageFile("LicenseFile::LoadLicenseFile (enter)");

	// Get GEX application path: $GEX_PATH/gex_license.bin
	strFileName = m_strApplicationDir + "/gex_license.txt";
	pString = strFileName.toLatin1();

	strMessage = "LicenseFile::LoadLicenseFile - opening license file ";
	strMessage += strFileName;
	WriteDebugMessageFile(strMessage);

	hCryptedFile = fopen(pString.constData(), "rb");


    if (hCryptedFile == NULL)
    {
        strFileName = m_strUserHome + "/gex_license.txt";
        pString = strFileName.toLatin1();

        strMessage = "LicenseFile::LoadLicenseFile - opening license file ";
        strMessage += strFileName;
        WriteDebugMessageFile(strMessage);

        hCryptedFile = fopen(pString.constData(), "rb");
    }

    if (hCryptedFile == NULL)
    {
        strFileName = m_strUserHome + "/GalaxySemi/gex_license.txt";
        pString = strFileName.toLatin1();

        strMessage = "LicenseFile::LoadLicenseFile - opening license file ";
        strMessage += strFileName;
        WriteDebugMessageFile(strMessage);

        hCryptedFile = fopen(pString.constData(), "rb");
    }

    if(hCryptedFile == NULL)
    {
        // Second chance: try to open gex_license.bin file...for backward compatibility
        strFileName = m_strApplicationDir + "/gex_license.bin";
        pString = strFileName.toLatin1();

        strMessage = "LicenseFile::LoadLicenseFile - opening license file ";
        strMessage += strFileName;
        WriteDebugMessageFile(strMessage);

        hCryptedFile = fopen(pString.constData(), "rb");
    }

	WriteDebugMessageFile("Checking if license file could be opened...");
	if(hCryptedFile == NULL)
	{
		LogMessage("ERROR: Failed to open gex_license.txt file...", true);
		return false;	// License file doesn't exist...so abort, and popup registration window!
	}

	strMessage = "License file opened " + strFileName;
	WriteDebugMessageFile(strMessage);
	switch(cSystemInfo.isSystemInfoAvailable())
	{
		case READ_SYS_INFO_ERROR_HOSTNAME:
			// Failed reading computer name
			LogMessage("ERROR: Failed reading hostname...", true);
			return false;
			break;

		case READ_SYS_INFO_ERROR_NETWORK:
			LogMessage("ERROR: Failed reading Ethernet board and disk info...", true);
			return false;
			break;
			
		case READ_SYS_INFO_NOERROR:
		default:
			break;

	}

	// Read lines from crypted file...
	WriteDebugMessageFile("LicenseFile::LoadLicenseFile - decoding license file");
	uChecksum = 0;	// Clear checksum
	ReadCryptedFile(szString);					// 'Galaxy Examinator - the STDF detective VX.Y Build Z'
	ReadCryptedFile(szString);					// License file version.
	iFileVersion=0;
	sscanf(szString,"%d",&iFileVersion);
	ReadCryptedFile(szString);					// Time and date license file was created (sec. ellapsed since Jan 1 1970)
	ReadCryptedFile(szString);					// License expiration date (YYYY MM DD)
	strExpiration = szString;
	strMessage = "LicenseFile::LoadLicenseFile - expiration date = ";
	strMessage += szString;
	strMessage += " (license file)";
	WriteDebugMessageFile(strMessage);

	ReadCryptedFile(szString);					// User name....(ignored in license filter)
	ReadCryptedFile(szString);					// Platform : Windows, Solaris, HP-UX
	strMessage = "LicenseFile::LoadLicenseFile - platform = ";
	strMessage += szString;
	strMessage += " (license file) / ";
	strMessage += cSystemInfo.strPlatform + " (system)";
	WriteDebugMessageFile(strMessage);
#if 0
	// Do not check the platform anymore to avoid license reject, as it has been enriched (under Windows for now) with
	// more details about the OS version
	if(cSystemInfo.strPlatform != szString)
	{
		LogMessage("ERROR: Current 'gex_license.txt' file not activated for this server OS platform!", true);
		return false;	// License file doesn't exist...so abort, and popup registration window!
	}
#endif
	ReadCryptedFile(szString);					// Product: 0= Examinator, 1=Examinator for Credence,2=Examinator for SZ, 4=ExaminatorDB
	iProductID=0;
	sscanf(szString,"%d",&iProductID);
	
	// <LicenseType>  <TotalLicensesOrdered> <Edition Type> <Products-for-Monitoring>
	ReadCryptedFile(szString);					// License type: 0=floating, 1=standalone
	int iLicenseType=-1;
	iEditionType=0;	// Standard Edition (1=Professional Edition)
	iMonitorProducts=0;	// Total Products allowed in Monitoring
	iOptionalModules=0;	// Bit0=Plugin support, Bit1=PAT support,Bit2=Y123Web mode,Bit3=AllTimezones accepted
	sscanf(szString,"%d %d %d %d %d %d %d %d",&iLicenseType,&iMaximumLicenses,&iEditionType,&iMonitorProducts,&iOptionalModules,&iAllowedTimeShift,&iMaxVersion_Maj,&iMaxVersion_Min);
	// If old license file: enable SYA option!
	if (iFileVersion < 101)
		iOptionalModules |= 0x10;
	strMessage = "LicenseFile::LoadLicenseFile - license type = " + QString::number(iLicenseType);
	strMessage += " (license file)";
	WriteDebugMessageFile(strMessage);

	if(iLicenseType != 0)
	{
		LogMessage("ERROR: Current 'gex_license.txt' file read is a standalone license, not a server license file!", true);
		return false;	// License file doesn't exist...so abort, and popup registration window!
	}
	ReadCryptedFile(szString);					// Computer name
	strMessage = "LicenseFile::LoadLicenseFile - hostname = ";
	strMessage += szString;
	strMessage += " (license file) / ";
	strMessage += cSystemInfo.strHostName + " (system)";
	WriteDebugMessageFile(strMessage);
	if(cSystemInfo.strHostName != szString)
	{
		LogMessage("ERROR: Current 'gex_license.txt' file not activated for this server!", true);
		return false;	// License file doesn't exist...so abort, and popup registration window!
	}
	ReadCryptedFile(szString);					// Account logged when GEX executed....(ignored in license filter)
	ReadCryptedFile(szString);					// ProductKey....(ignored in license filter)
	strLicenseID= szString;
	strMessage = "LicenseFile::LoadLicenseFile - id = ";
	strMessage += szString;
	strMessage += " (license file)";
	WriteDebugMessageFile(strMessage);
	ReadCryptedFile(szString);					// HostID
	strMessage = "LicenseFile::LoadLicenseFile - hostid = ";
	strMessage += szString;
	strMessage += " (license file) / ";
	strMessage += cSystemInfo.strHostID + " (system)";
	WriteDebugMessageFile(strMessage);
	if(cSystemInfo.strHostID != szString)
	{
		LogMessage("ERROR: Current 'gex_license.txt' file not activated for this server!", true);
		return false;	// License file doesn't exist...so abort, and popup registration window!
	}
	ReadCryptedFile(szString);					// DiskID (='?' if Unix)
	strMessage = "LicenseFile::LoadLicenseFile - diskid = ";
	strMessage += szString;
	strMessage += " (license file) / ";
	strMessage += cSystemInfo.strDiskID + " (system)";
	WriteDebugMessageFile(strMessage);
	if(cSystemInfo.strDiskID != szString)
	{
		LogMessage("ERROR: Current 'gex_license.txt' file not activated for this server!", true);
		return false;	// License file doesn't exist...so abort, and popup registration window!
	}
	ReadCryptedFile(szString);					// 1st Ethernet Network boardID (='?' if Unix)
	ReadCryptedFile(szString);					// List of ALL Ethernet Network boardIDs (='?' if Unix)

#ifdef _WIN32
	// Under windows: verify that this is the correct server.
	strMessage = "LicenseFile::LoadLicenseFile - netboards = ";
	strMessage += szString;
	strMessage += " (license file) / ";
	strMessage += cSystemInfo.strNetworkBoardID + " (system)";
	WriteDebugMessageFile(strMessage);

	bool bFoundNetBoard = false;
	strString = szString;
	QStringList	strNetBoards = strString.split(';');
	QStringList::Iterator it;
	for ( it = strNetBoards.begin(); it != strNetBoards.end(); ++it )
 	{
		if(cSystemInfo.strNetworkBoardID.indexOf(*it) >= 0)
			bFoundNetBoard = true;
	}
	if(!bFoundNetBoard)
	{
		LogMessage("ERROR: Current 'gex_license.txt' file not activated for this server!", true);
		return false;	// License file doesn't exist...so abort, and popup registration window!
	}
#endif	
	
	ReadCryptedFile(&lData);					// Processors (=0 if Unix)
	strMessage = "LicenseFile::LoadLicenseFile - nbprocessors = " + QString::number(lData);
	strMessage += " (license file) / ";
	strMessage += QString::number(cSystemInfo.lNumberOfProcessors) + " (system)";
	WriteDebugMessageFile(strMessage);
	if(lData != cSystemInfo.lNumberOfProcessors)
	{
		LogMessage("ERROR: Current 'gex_license.txt' file not activated for this server!", true);
		return false;	// License file doesn't exist...so abort, and popup registration window!
	}
	ReadCryptedFile(&lData);					// Processor type (=0 if Unix)
	strMessage = "LicenseFile::LoadLicenseFile - processortype = " + QString::number(lData);
	strMessage += " (license file) / ";
	strMessage += QString::number(cSystemInfo.lProcessorType) + " (system)";
	WriteDebugMessageFile(strMessage);
	if(lData != cSystemInfo.lProcessorType)
	{
		LogMessage("ERROR: Current 'gex_license.txt' file not activated for this server!", true);
		//return false;	// License file doesn't exist...so abort, and popup registration window!
	}
	ReadCryptedFile(&lData);					// Processor level (=0 if Unix)
	strMessage = "LicenseFile::LoadLicenseFile - processorlevel = " + QString::number(lData);
	strMessage += " (license file) / ";
	strMessage += QString::number(cSystemInfo.lProcessorLevel) + " (system)";
	WriteDebugMessageFile(strMessage);
	if(lData != cSystemInfo.lProcessorLevel)
	{
		LogMessage("ERROR: Current 'gex_license.txt' file not activated for this server!", true);
		return false;	// License file doesn't exist...so abort, and popup registration window!
	}
	ReadCryptedFile(&lData);					// Processor revision (=0 if Unix)
	strMessage = "LicenseFile::LoadLicenseFile - processorrevision = " + QString::number(lData);
	strMessage += " (license file) / ";
	strMessage += QString::number(cSystemInfo.lProcessorRevision) + " (system)";
	WriteDebugMessageFile(strMessage);
	if(lData != cSystemInfo.lProcessorRevision)
	{
		LogMessage("ERROR: Current 'gex_license.txt' file not activated for this server!", true);
		return false;	// License file doesn't exist...so abort, and popup registration window!
	}
	
	ReadCryptedFile(szString);					// Maintenance expiration date (YYYY MM DD)
	if (iFileVersion > 100)
		m_strMaintenanceExpiration = szString;

	ReadCryptedFile(szString);					// Spare field#1
	ReadCryptedFile(szString);					// Spare field#2
	ReadCryptedFile(szString);					// Spare field#3
	uChecksum &= 0xffff;
	unsigned long uComputedChecksum = uChecksum;	// Get checksum of file...without checksum field!
	ReadCryptedFile(szString);					// Spare field#5...or checksum
	if(strcmp(szString,"?") != 0)
	{
		// Verify checksum...
		unsigned long uFileChecksum=0;
		sscanf(szString,"%lu",&uFileChecksum);
		if(uFileChecksum != uComputedChecksum)
		{
			LogMessage("ERROR: Current 'gex_license.txt' file is corrupted!", true);
			return false;	// Invalid checksum...file corrupted
		}
	}

	// Close license file.
	fclose(hCryptedFile);

	WriteDebugMessageFile("LicenseFile::LoadLicenseFile (exit OK)");

	// License file okay....
	return true;
}

///////////////////////////////////////////////////////////
// No license file exist, so create license request file!
///////////////////////////////////////////////////////////
void LicenseFile::CreateLicenseRequestFile(int nPackageID, QString & strUserName, QString & strProductID)
{
	QString		strString;
	QString		strFile;
	QByteArray	pString;

	// Check if system info correctly read
	int nStatus = cSystemInfo.isSystemInfoAvailable();
	if(nStatus != READ_SYS_INFO_NOERROR)
	{
		switch(nStatus)
		{
			case READ_SYS_INFO_ERROR_HOSTNAME:
				// Failed reading computer name
				strString = "ERROR: Failed reading hostname...\n";
				break;
			case READ_SYS_INFO_ERROR_NETWORK:
				// Failed reading Ethernet board and disk info...
				strString = "ERROR: Failed reading Ethernet board and disk info...\n";
				break;
			default:
				strString = "ERROR: unknown error...\n";
				break;
		}
		strString += "\nGEX-LM will now exit!";
		QMessageBox::critical(NULL,szAppFullName,strString);
		exit(0);
	}

	// Build license request file path+name
	strFile = m_strUserHome + "/gex_request.txt";
	CGexSystemUtils::NormalizePath(strFile);

	pString = strFile.toLatin1();
	hCryptedFile = fopen(pString.constData(), "wb");
	if(hCryptedFile == NULL)
	{
		strString = "ERROR: Failed creating license request file:\n";
		strString += strFile;
		strString += "\nGEX-LM will now exit!";
		QMessageBox::critical(NULL,szAppFullName,strString);
		exit(0);
	}
	
	// Check type of License request to perform
	switch(nPackageID)
	{
		case 0:	// Examinator
		default:
			lPlatformRestriction = GEX_DATATYPE_ALLOWED_ANY;
			break;
		case 1:	// Examinator-Pro
		case 2:	// Yield-Man package
		case 3:	// PAT-Man package
			lPlatformRestriction = GEX_DATATYPE_ALLOWED_DATABASE;
			break;
	}

	// write into the file the information we need !
	WriteCryptedFile(szAppFullName);							// 'Galaxy Examinator - the STDF detective VX.Y Build Z'
	WriteCryptedFile(time(NULL));								// Current time/date
	WriteCryptedFile(const_cast<char*>("Server_Request"));		// License request is from a Server...so license type can only be of 'floating' type.
	pString = strUserName.toLatin1();
	WriteCryptedFile(pString.data());							// User name
	pString = cSystemInfo.strPlatform.toLatin1();
	WriteCryptedFile(pString.data());							// Platform : Windows, Solaris, HP-UX
	pString = cSystemInfo.strHostName.toLatin1();
	WriteCryptedFile(pString.data());							// Computer name
	pString = cSystemInfo.strAccountName.toLatin1();
	WriteCryptedFile(pString.data());							// Account logged when GEX executed.
	pString = strProductID.toLatin1();
	WriteCryptedFile(pString.data());							// ProductKey
	pString = cSystemInfo.strHostID.toLatin1();
	WriteCryptedFile(pString.data());							// HostID
	pString = cSystemInfo.strDiskID.toLatin1();
	WriteCryptedFile(pString.data());							// DiskID (='?' if Unix)
	pString = cSystemInfo.strNetworkBoardID.toLatin1();
	WriteCryptedFile(pString.data());							// NetworkBoardMAC
	pString = cSystemInfo.strNetworkBoardsIDs.toLatin1();
	WriteCryptedFile(pString.data());							// List of ALL Ethernet Network MAC addresses found on computer
	WriteCryptedFile(cSystemInfo.lNumberOfProcessors);			// Processors (=0 if Unix)
	WriteCryptedFile(cSystemInfo.lProcessorType);				// Processor type (=0 if Unix)
	WriteCryptedFile(cSystemInfo.lProcessorLevel);				// Processor level (=0 if Unix)
	WriteCryptedFile(cSystemInfo.lProcessorRevision);			// Processor revision (=0 if Unix)
	WriteCryptedFile(lPlatformRestriction);						// License type requested: GEX (0), Production (4), etc...
	WriteCryptedFile(const_cast<char*>("?"));					// Spare field#1
	WriteCryptedFile(const_cast<char*>("?"));					// Spare field#2
	WriteCryptedFile(const_cast<char*>("?"));					// Spare field#3
	WriteCryptedFile(const_cast<char*>("?"));					// Spare field#4
	WriteCryptedFile(const_cast<char*>("?"));					// Spare field#5

	fclose(hCryptedFile);

	strString = "Thanks for entering your identification information.\n";
	strString += "To receive your license file, do the following:\n";
	strString += "Email the file: ";
	strString += strFile;
	strString += "\nTo: "+QString(GEX_EMAIL_LICENSE)+"\n";
	strString += "\nWithin few minutes, an auto-reply message will\n";
	strString += "be returned to you with the license file and\n";
	strString += "the installation instructions.\n\nEnjoy GEX!";

	// Under windows, try to launch a Mail window!
#ifdef _WIN32
      OSVERSIONINFOEX osvi;
      BOOL bOsVersionInfoEx;
	  bool bWin95=true;	// If Win95, do not try to send the email 9would crash!).

      // Try calling GetVersionEx using the OSVERSIONINFOEX structure,
      // If that fails, try using the OSVERSIONINFO structure.
      memset(&osvi,0,sizeof(OSVERSIONINFOEX));
      osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

      bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO *)&osvi);
      if (!bOsVersionInfoEx) 
	  {
         // If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.
         osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
         GetVersionEx((OSVERSIONINFO *)&osvi);
      } // if

      switch (osvi.dwPlatformId) 
	  {
         case VER_PLATFORM_WIN32_NT:
			bWin95 = false;	// We have NT3, NT4, 2000, or XP Operating system!
            break;

         case VER_PLATFORM_WIN32_WINDOWS:
            if (osvi.dwMajorVersion > 4)
				bWin95 = false;	// Windows ME or higher
            else
               if (osvi.dwMajorVersion == 4) 
			   {
                  if (osvi.dwMinorVersion >= 90)
						bWin95 = false;	// Windows ME or higher
                  else
                     if (osvi.dwMinorVersion >= 10)
						bWin95 = false;	// Windows98
                     else
						bWin95 = true;	// Windows 95
               } // if
            break;
      } // switch


	  if(bWin95 == true)
	  {
		  // Do not send email, simply display a message box!
		  QMessageBox::information(NULL,"GEX License",strString);
	  }
	  else
	  {
		// Send the email!
		int iStatus = QMessageBox::information(NULL,"GEX License",
			strString,
			"Outlook users: Create Email Now!",
			"Other users: email manually", 0, 0, 1 );
		if(iStatus != 0)
		{
			QMessageBox::information(NULL,"GEX License",strString);
			exit(0);
		}

		QString strEmail;
		strEmail = "mailto:"+QString(GEX_EMAIL_LICENSE)+"?subject=Request for GEX license activation";
		strEmail += "&body=%0A====> LAST STEP to perform:%0A";
		strEmail += "   1. ATTACH the file ";
		strEmail += "'" + strFile + "' to this email%0A";
		strEmail += "   2. Send this message!%0A%0A";
		strEmail += "Within few minutes, an auto-reply message will%0A";
		strEmail += "be returned to you with the license file and%0A";
		strEmail += "the installation instructions.%0A%0AEnjoy GEX!";
		pString = strEmail.toLatin1();
		// Launch email shell
		ULONG_PTR ulpStatus;
		ulpStatus = (ULONG_PTR) ShellExecuteA(NULL,
				   "open",
				   pString.constData(),
				   NULL,
				   NULL,
				   SW_SHOWNORMAL);
		iStatus = (int )(ulpStatus);
		if(iStatus <= 32)
		{
			// Failed creating 
			strString = "*Failed* trying to create the email reply\nYou have to manually create it:\n\n" + strString;
			QMessageBox::warning(NULL,"GEX License",strString);
		}
	  }
#else
	// Under Unix, can't send email automatically: so simply display the instructions!
	QMessageBox::information(NULL,"GEX License",strString);

	// Also print this message to the console...so user still has the message after GEX closes!
	pString = strString.toLatin1();
	printf("\n\n\n\n");
	printf("\n************************************************\n");
	printf("%s", pString.constData());
	printf("\n************************************************\n");
	fflush(stdout);
#endif
	exit(0);
}

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
ActivationServerKeyDialog::ActivationServerKeyDialog(const QString & strUserHome, const QString & strApplicationDir, QWidget* parent)
	: QDialog(parent)
{
	// Init some members
	m_strUserHome = strUserHome;
	m_strApplicationDir = strApplicationDir;

	// Setu UI
	setupUi(this);

	// Setup the application buttons
	setWindowFlags(Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);

	// Set the Gex default palette
	CGexSkin gexSkin;
	gexSkin.applyPalette(this);
	
    // Set text in license text widget
	QString strLicenseText;

	strLicenseText  = "<html><head><meta name=\"qrichtext\" content=\"1\" /></head>";
	strLicenseText += "<body style=\" white-space: pre-wrap; font-family:MS Shell Dlg; font-size:8.25pt; font-weight:400; font-style:normal; text-decoration:none;\">";
	strLicenseText += "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;\">Welcome to Galaxy Examinator License Manager (GEX-LM) !</p>";
	strLicenseText += "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;\"></p>";
	strLicenseText += "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;\">You must be running this software on the CPU that will be used as the server to run GEX-LM license manager. If this is not the case, exit now and move the software to the correct server.</p>";
	strLicenseText += "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;\">======================================</p>";
	strLicenseText += "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;\">To complete the GEX-LM setup, do the following:</p>";
	strLicenseText += "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;\">     1) Select Software Package to activate</p>";
	strLicenseText += "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;\">     2) Enter your full name</p>";
	strLicenseText += "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;\">     3) Enter the Product Key (given to you when you placed your P.O.)</p>";
	strLicenseText += "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;\">     4) Click the 'Next' button</p>";
	strLicenseText += "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;\">     5) Email the file 'gex_request.txt' created to "+QString(GEX_EMAIL_LICENSE)+"</p>";
	strLicenseText += "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;\">     6) Within few minutes, you will receive your license file and activation instructions (email auto-reply)</p>";
	strLicenseText += "</body></html>";
	textLicense->setHtml(strLicenseText); 

	// Ensure edit fields are cleared
	editField1->clear();
	editField2->clear();
	editField1->setFocus();

	// Set the Package combo-box to the default value
	switch(lPlatformRestriction)
	{
		case GEX_DATATYPE_ALLOWED_ANY:
		default:
			comboBox->setCurrentIndex(0);	// Examinator
			break;
		case GEX_DATATYPE_ALLOWED_DATABASE:
			comboBox->setCurrentIndex(1);	// Examinator-Pro
			break;
	}

}

void ActivationServerKeyDialog::on_buttonNext_clicked()
{
	QString strUserName, strProductID;
	int		nPackageID;

	// Check user name entered
	strUserName = editField1->text();
	if(strUserName.isEmpty() == true)			// Licensee name
	{
		QMessageBox::critical(NULL,szAppFullName,
				GEX_T("A valid user name is required for the license activation"));
		editField1->setFocus();
		return;
	}

	// Check ProductKey entered
	bool bBadKey=false;
	strProductID = editField2->text();
	strProductID = strProductID.toUpper();		// uppercase
	if(strProductID.isEmpty() == true)
		bBadKey = true;
	else
	{
		if(strProductID.at(7) != '-')
			bBadKey = true;
		strProductID.truncate(4);
		if(strProductID != "GEX-")
			bBadKey = true;
	}
	if(bBadKey == true)
	{
		QMessageBox::critical(NULL,szAppFullName,
				GEX_T("A valid Product Key is required for the license activation (eg: GEX-XYZ-123456)"));
		editField2->setFocus();
		return;
	}
	strProductID = editField2->text();

	// Get Package ID
	nPackageID = comboBox->currentIndex();

	LicenseFile lic(m_strUserHome, m_strApplicationDir);
	lic.CreateLicenseRequestFile(nPackageID, strUserName, strProductID);
}

