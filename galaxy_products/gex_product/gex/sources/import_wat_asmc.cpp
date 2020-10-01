//////////////////////////////////////////////////////////////////////
// import_wat_asmc.cpp: Convert a .WAT_ASMC (TSMC) file to STDF V4.0
//////////////////////////////////////////////////////////////////////
#include "gqtl_global.h"
#include <qmath.h>
#include <time.h>
#ifdef _WIN32
#include "windows.h"		// For 'GetWindowsDirectoryA' declaration
#endif

#include <qfileinfo.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>

#include "engine.h"
#include "import_wat_asmc.h"
#include "import_constants.h"


// MIR RECORD CORRESPONDENCE
// MIR.PART_TYP      MIR.PROC_ID       MIR.TEST_COD      MOR.LOT_ID   MIR.SETUP_T              MIR.TSTR_TYP
// File format:
//Typename:6M04471   Process:QUBIC1F   Stepcode:PQB512   Lot:6E2640   TestDate:   2006/09/07   Equip:7k07   Pg:  1
// NAME1  | PGateBV@ N-EPIShe DN_SubsS BN_OxShe N+SDShee P+SDShee SNGLX_2u 250+Shee SNGLX_1. Cnt_2.8_ Br_2.8_M PTSIN+Sh 1kP-RESS
//     2  |     349n     etRh     heet     etRh     tRho     tRho     mP+S     tRho     6uP+     M2/M     2/M1     eetR     heet
//  UNIT  |    Volts    KOhms    Ohms/    Ohms/    Ohms/    Ohms/    Mirco    Ohms/    Mirco     Ohms    nAmps    Ohms/    KOhms
//PARCODE |   QB0010   QB0021   QB0030   QB0040   QB0070   QB0080   QB0090   QB0200   QB0210   QB0220   QB0230   QB0240   QB0290
// L.S.L. |   12.000    3.400   31.000   25.000   40.000  115.000    1.600  210.000    1.300   26.000    0.000    5.800    0.800
// U.S.L. |   25.000    5.100   51.000   41.000  160.000  335.000    2.400  290.000    1.800   49.000   10.000   11.800    1.200
//-------------Key------Key------Key------Key------Key------Key------Key------Key------Key------Key------Key------Key------Key---
//WAF   9 |   19.390    4.270   40.000   35.200   70.100  268.800C   2.071  272.000    1.587   42.100    0.155    8.000    0.927 
//        |   19.390    4.163   39.300   34.200   68.900  265.200C   2.040  266.000    1.576   40.500    0.152    8.200    0.903 
//

// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGWatAsmcWafer::~CGWatAsmcWafer()
{
	while (!m_pParameterList.isEmpty())
		delete m_pParameterList.takeFirst();
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGWAT_ASMCtoSTDF::CGWAT_ASMCtoSTDF()
{
	// Default: WAT_ASMC parameter list on disk includes all known WAT_ASMC parameters...
	m_bNewWatAsmcParameterFound = false;
	m_lStartTime = 0;
}

CGWAT_ASMCtoSTDF::~CGWAT_ASMCtoSTDF()
{
	while (!m_pWaferList.isEmpty())
		delete m_pWaferList.takeFirst();
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGWAT_ASMCtoSTDF::GetLastError()
{
	m_strLastError = "Import WAT_ASMC: ";

	switch(m_iLastError)
	{
		default:
		case errNoError:
			m_strLastError += "No Error";
			break;
		case errOpenFail:
			m_strLastError += "Failed to open file";
			break;
		case errInvalidFormat:
			m_strLastError += "Invalid file format";
			break;
		case errWriteSTDF:
			m_strLastError += "Failed creating temporary file. Folder permission issue?";
			break;	
		case errLicenceExpired:
			m_strLastError += "License has expired or Data file out of date...";
			break;	
	}
	// Return Error Message
	return m_strLastError;
}

//////////////////////////////////////////////////////////////////////
// Check if File date is older than license expiration date!
//////////////////////////////////////////////////////////////////////
bool CGWAT_ASMCtoSTDF::CheckValidityDate(QDate *pExpirationDate)
{
	// If no expiration date, or m_lStartTime not set ...ignore !
	if((pExpirationDate == NULL) || (m_lStartTime <= 0))
		return true;

	// Check date found in data file to convert...and see if not to recent!
	// Check if STDF file is too recent
	QDateTime expirationDateTime(*pExpirationDate);
	QDateTime FileDateTime;
	FileDateTime.setTime_t(m_lStartTime);
	FileDateTime.setTimeSpec(Qt::UTC);
	expirationDateTime.setTimeSpec(Qt::UTC);
	if(FileDateTime > expirationDateTime)
		return false;	// Invalid file date! refuse to convert the file.
	return true;
}

//////////////////////////////////////////////////////////////////////
// Load WAT_ASMC Parameter table from DISK
//////////////////////////////////////////////////////////////////////
void CGWAT_ASMCtoSTDF::LoadParameterIndexTable(void)
{
	QString	strWatAsmcTableFile;
	QString	strString;

    strWatAsmcTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strWatAsmcTableFile += GEX_WAT_ASMC_PARAMETERS;

	// Open WAT_ASMC Parameter table file
    QFile f( strWatAsmcTableFile );
    if(!f.open( QIODevice::ReadOnly ))
		return;

	// Assign file I/O stream
	QTextStream hWatAsmcTableFile(&f);

	// Skip comment lines
	do
	{
	  strString = hWatAsmcTableFile.readLine();
	}
    while((strString.indexOf("----------------------") < 0) && (hWatAsmcTableFile.atEnd() == false));

	// Read lines
	m_pFullWatAsmcParametersList.clear();
	strString = hWatAsmcTableFile.readLine();
	while (strString.isNull() == false)
	{
		// Save Parameter name in list
		m_pFullWatAsmcParametersList.append(strString);
		// Read next line
		strString = hWatAsmcTableFile.readLine();
	};

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// Save WAT_ASMC Parameter table to DISK
//////////////////////////////////////////////////////////////////////
void CGWAT_ASMCtoSTDF::DumpParameterIndexTable(void)
{
	QString		strWatAsmcTableFile;

    strWatAsmcTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strWatAsmcTableFile += GEX_WAT_ASMC_PARAMETERS;

	// Open WAT_ASMC Parameter table file
    QFile f( strWatAsmcTableFile );
    if(!f.open( QIODevice::WriteOnly ))
		return;

	// Assign file I/O stream
	QTextStream hWatAsmcTableFile(&f);

	// First few lines are comments:
	hWatAsmcTableFile << "############################################################" << endl;
	hWatAsmcTableFile << "# DO NOT EDIT THIS FILE!" << endl;
	hWatAsmcTableFile << "# Quantix Examinator: WAT_ASMC Parameters detected" << endl;
	hWatAsmcTableFile << "# www.mentor.com" << endl;
	hWatAsmcTableFile << "# Quantix Examinator reads and writes into this file..." << endl;
	hWatAsmcTableFile << "-----------------------------------------------------------" << endl;

	// Write lines
	// m_pFullWatAsmcParametersList.sort();
	for(int nIndex = 0; nIndex < m_pFullWatAsmcParametersList.count(); nIndex++)
	{
		// Write line
		hWatAsmcTableFile << m_pFullWatAsmcParametersList[nIndex] << endl;
	};

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// If Examinator doesn't have this WAT_ASMC parameter in his dictionnary, have it added.
//////////////////////////////////////////////////////////////////////
void CGWAT_ASMCtoSTDF::UpdateParameterIndexTable(QString strParamName)
{
	// Check if the table is empty...if so, load it from disk first!
	if(m_pFullWatAsmcParametersList.isEmpty() == true)
	{
		// Load WAT_ASMC parameter table from disk...
		LoadParameterIndexTable();
	}
	
	// Check if Parameter name already in table...if not, add it to the list
	// the new full list will be dumped to the disk at the end.
    if(m_pFullWatAsmcParametersList.indexOf(strParamName) < 0)
	{
		// Update list
		m_pFullWatAsmcParametersList.append(strParamName);

		// Set flag to force the current WAT_ASMC table to be updated on disk
		m_bNewWatAsmcParameterFound = true;
	}
}

//////////////////////////////////////////////////////////////////////
// Find or create Parameter entry, return handle to it.
//////////////////////////////////////////////////////////////////////
CGWatAsmcParameter *CGWAT_ASMCtoSTDF::FindParameterEntry(int iWaferID,int iSiteID,QString strParamName,QString strUnits)
{
	CGWatAsmcWafer *						ptWafers	= NULL;					// List of Wafers in WAT_ASMC file
	CGWatAsmcParameter *					ptParam		= NULL;					// List of parameters
	QList<CGWatAsmcWafer*>::iterator		itWafer		= m_pWaferList.begin();
	QList<CGWatAsmcParameter*>::iterator	itParam;

	// find or create the ptWafers
	while(itWafer != m_pWaferList.end())
	{
		if((*itWafer)->m_iWaferID == iWaferID)
		{
			ptWafers = (*itWafer);
			break;
		}

		// Not the WaferID we need...see next one.
		itWafer++;
	};

	if(ptWafers == NULL)
	{
		// WaferID entry doesn't exist yet...so create it now!
		ptWafers = new CGWatAsmcWafer;
		ptWafers->m_iLowestSiteID=ptWafers->m_iHighestSiteID=iSiteID;
		ptWafers->m_iWaferID = iWaferID;
		
		// Add Wafer entry to existing list.
		m_pWaferList.append(ptWafers);
	}

	// Update Min/Max SiteID
	if(iSiteID > 0)
	{
		ptWafers->m_iLowestSiteID=gex_min(ptWafers->m_iLowestSiteID,iSiteID);				// Lowest WaferID found in WAT_ASMC file
		ptWafers->m_iHighestSiteID=gex_max(ptWafers->m_iHighestSiteID,iSiteID);				// Highest WaferID found in WAT_ASMC file
	}
	
	// Found WaferID entry...now find Parameter entry
	itParam = ptWafers->m_pParameterList.begin();
	while(itParam != ptWafers->m_pParameterList.end())
	{
		if((*itParam)->m_strName == strParamName)
			return (*itParam);	// Found the Parameter!

		// Not the Parameter we need...see next one.
		itParam++;
	};
	
	// Parameter not in list...create entry!
	// Create & add new Parameter entry in Wafer list
	ptParam = new CGWatAsmcParameter;
	ptParam->m_strName = strParamName;
	ptParam->m_strUnits = strUnits;
	ptParam->m_fHighLimit = ptParam->m_fLowLimit = 0;
	NormalizeLimits(ptParam);
	ptParam->m_bValidLowLimit = false;		// Low limit defined
	ptParam->m_bValidHighLimit = false;	// High limit defined

	ptParam->m_bStaticHeaderWritten = false;
	ptWafers->m_pParameterList.append(ptParam);
	iTotalParameters++;
	// If Examinator doesn't have this WAT_ASMC parameter in his dictionnary, have it added.
	UpdateParameterIndexTable(strParamName);
	
	// Return pointer to the Parameter 
	return ptParam;
}

//////////////////////////////////////////////////////////////////////
// Save WAT_ASMC parameter result...
//////////////////////////////////////////////////////////////////////
void CGWAT_ASMCtoSTDF::SaveParameterResult(int iWaferID,int iSiteID,QString strName,QString strUnits,QString strValue)
{
	if(strName.isNull() == true)
		return;	// Ignore empty entry!

	// Find Pointer to the Parameter cell
	float	fValue;
	CGWatAsmcParameter *ptParam = FindParameterEntry(iWaferID,iSiteID,strName,strUnits);

	// Save the Test value in it
	strValue = strValue.trimmed();
	fValue = strValue.toFloat() * GS_POW(10.0,ptParam->m_nScale);
	ptParam->m_fValue[iSiteID] = fValue;
}

//////////////////////////////////////////////////////////////////////
// Save WAT_ASMC parameter High or Low Limit result...
//////////////////////////////////////////////////////////////////////
void CGWAT_ASMCtoSTDF::SaveParameterLimit(QString strName,QString strUnit,QString strValue,int iLimit)
{
	if(strName.isNull() == true)
		return;	// Ignore empty entry!

	CGWatAsmcParameter *				ptParam	= NULL;	// List of parameters
	QList<CGWatAsmcWafer*>::iterator	itWafer	= m_pWaferList.begin();

	while(itWafer != m_pWaferList.end())
	{
		// Get Parameter pointer...save limits in ALL WaferEntry.
		ptParam = FindParameterEntry((*itWafer)->m_iWaferID,0,strName,strUnit);
		switch(iLimit)
		{
			case eHighLimit:
				ptParam->m_fHighLimit		= strValue.toFloat() * GS_POW(10.0,ptParam->m_nScale);
				ptParam->m_bValidHighLimit	= true;		// High limit defined

				break;
			case eLowLimit:
				ptParam->m_fLowLimit		= strValue.toFloat()  * GS_POW(10.0,ptParam->m_nScale);
				ptParam->m_bValidLowLimit	= true;		// Low limit defined
				break;
		}

		// Move to next wafer entry
		itWafer++;
	};
}

//////////////////////////////////////////////////////////////////////
// Normalize test limits when writing into PTR.
//////////////////////////////////////////////////////////////////////
void CGWAT_ASMCtoSTDF::NormalizeLimits(CGWatAsmcParameter *pParameter)
{
	int	value_scale=0;
	if(pParameter->m_strUnits.length() <= 1)
	{
		// units too short to include a prefix, then keep it 'as-is'
		pParameter->m_nScale = 0;
		return;
	}
	if(pParameter->m_strUnits.startsWith("MIRCO", Qt::CaseInsensitive))
	{
		// exception
		pParameter->m_nScale = 0;
		return;
	}

	QChar cPrefix = pParameter->m_strUnits[0];
	switch(cPrefix.toLatin1())
	{
		case 'm': // Milli
			value_scale = -3;
			break;
		case 'u': // Micro
			value_scale = -6;
			break;
		case 'n': // Nano
			value_scale = -9;
			break;
		case 'p': // Pico
			value_scale = -12;
			break;
		case 'f': // Fento
			value_scale = -15;
			break;
		case 'K': // Kilo
			value_scale = 3;
			break;
		case 'M': // Mega
			value_scale = 6;
			break;
		case 'G': // Giga
			value_scale = 9;
			break;
		case 'T': // Tera
			value_scale = 12;
			break;
	}
	pParameter->m_fLowLimit *= GS_POW(10.0,value_scale);
	pParameter->m_fHighLimit *= GS_POW(10.0,value_scale);
	pParameter->m_nScale = value_scale;
	if(value_scale)
		pParameter->m_strUnits = pParameter->m_strUnits.mid(1);	// Take all characters after the prefix.
}



//////////////////////////////////////////////////////////////////////
// Check if File is compatible with WAT_ASMC format
//////////////////////////////////////////////////////////////////////
bool CGWAT_ASMCtoSTDF::IsCompatible(const char *szFileName)
{
	QString strString;
	QString strSection;
	QString strValue;
	QString	strSite;

	// Open hCsmFile file
    QFile f( szFileName );
    if(!f.open( QIODevice::ReadOnly ))
	{
		// Failed Opening ASL1000 file
		return false;
	}
	// Assign file I/O stream
	QTextStream hWatAsmcFile(&f);

	//Typename:6M04471   Process:QUBIC1F   Stepcode:PQB512   Lot:6E2640   TestDate:   2006/09/07   Equip:7k07   Pg:  1
	// Check if first line is the correct WAT_ASMC header...
	// Read WAT_ASMC information
	QString strDate;
	
	do
		strString = hWatAsmcFile.readLine();
	while(!strString.isNull() && strString.isEmpty());

	f.close();

	if(!strString.section(":",0,0).simplified().toUpper().startsWith("TYPENAME", Qt::CaseInsensitive))
	{
		// Incorrect header...this is not a WAT_ASMC file!
		return false;
	}
	
	return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the WAT_ASMC file
//////////////////////////////////////////////////////////////////////
bool CGWAT_ASMCtoSTDF::ReadWatAsmcFile(const char *WatAsmcFileName,QDate *pExpirationDate)
{
	QString strString;
	QString strSection;
	QString	strSite;
	QString	strParameters[13];	// Holds the 13 Parameters name (WAT_ASMC file is organized by sets of 13 parameters columns)
	QString	strUnits[13];		// Holds the 13 Parameters Units 
	QString	strLowLimits[13];	// Holds the 13 Parameters Low Limits
	QString	strHighLimits[13];	// Holds the 13 Parameters High Limits 
	int		iWaferID=0;			// WaferID processed
	int		iSiteID=0;			// SiteID processed
	int		iIndex;				// Loop index

	// Open WAT_ASMC file
    QFile f( WatAsmcFileName );
    if(!f.open( QIODevice::ReadOnly ))
	{
		// Failed Opening WAT_ASMC file
		m_iLastError = errOpenFail;
		
		// Convertion failed.
		return false;
	}
	// Assign file I/O stream
	QTextStream hWatAsmcFile(&f);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	iFileSize = f.size() + 1;

	//Typename:6M04471   Process:QUBIC1F   Stepcode:PQB512   Lot:6E2640   TestDate:   2006/09/07   Equip:7k07   Pg:  1
	// Check if first line is the correct WAT_ASMC header...
	// Read WAT_ASMC information
	QString strDate;
	
	strString = ReadLine(hWatAsmcFile);

	if(!strString.section(":",0,0).simplified().toUpper().startsWith("TYPENAME", Qt::CaseInsensitive))
	{
		// Incorrect header...this is not a WAT_ASMC file!
		m_iLastError = errInvalidFormat;
		
		// Convertion failed.
		// Close file
		f.close();
		return false;
	}
	
	while(!strString.isEmpty())
	{
		strSection = strString.section(":",0,0).simplified().toUpper();
		strString = strString.section(":",1);

		//Typename:6M04471   Process:QUBIC1F   Stepcode:PQB512   Lot:6E2640   TestDate:   2006/09/07   Equip:7k07   Pg:  1
		if(strSection.startsWith("TYPENAME", Qt::CaseInsensitive))
		{
			m_strProductId = strString.left(9).simplified();
			strString = strString.mid(9);
		}
		else
		if(strSection.startsWith("PROCESS", Qt::CaseInsensitive))
		{
			m_strProcessId = strString.left(9).simplified();
			strString = strString.mid(9);
		}
		else
		if(strSection.startsWith("STEPCODE", Qt::CaseInsensitive))
		{
			m_strTestCode = strString.left(8).simplified();
			strString = strString.mid(8);
		}
		else
		if(strSection.startsWith("LOT", Qt::CaseInsensitive))
		{
			m_strLotId = strString.left(8).simplified();
			strString = strString.mid(8);
		}
		else
		if(strSection.startsWith("TESTDATE", Qt::CaseInsensitive))
		{
			strDate = strString.left(15).simplified();
			strString = strString.mid(15);
		}
		else
		if(strSection.startsWith("EQUIP", Qt::CaseInsensitive))
		{
			m_strTesterType = strString.left(6).simplified();
			strString = strString.mid(6);
		}
		else
		if(strSection.startsWith("PG", Qt::CaseInsensitive))
		{
			// page number !
			strString = "";
		}
		else
		{
			// Incorrect header...this is not a WAT_ASMC file!
			m_iLastError = errInvalidFormat;
			
			// Convertion failed.
			// Close file
			f.close();
			return false;
		}

	}
	
	if(!strDate.isEmpty())
	{
		QDate qDate(strDate.section("/",0,0).toInt(),strDate.section("/",1,1).toInt(),strDate.section("/",2,2).toInt());
		QDateTime clDateTime(qDate);
		clDateTime.setTimeSpec(Qt::UTC);
		m_lStartTime = clDateTime.toTime_t();
	}

	// Check if file date is not more recent than license expiration date!
	if(CheckValidityDate(pExpirationDate) == false)
	{
		// Incorrect file date (greater than license expiration date)...refuse to convert file!
		m_iLastError = errLicenceExpired;
		
		// Convertion failed.
		// Close file
		f.close();
		return false;
	}

	// Read line with list of Parameters names

	// Loop reading file until end is reached
	while(!hWatAsmcFile.atEnd())
	{	
		strString = ReadLine(hWatAsmcFile);
		if(strString.isEmpty() || strString.startsWith("Typename:", Qt::CaseInsensitive) || strString.startsWith("Typename:", Qt::CaseInsensitive))
			continue;
		
		if(strString.simplified().startsWith("NAME1", Qt::CaseInsensitive))
		{
			// NAME1  | PGateBV@ N-EPIShe DN_SubsS BN_OxShe N+SDShee P+SDShee SNGLX_2u 250+Shee SNGLX_1. Cnt_2.8_ Br_2.8_M PTSIN+Sh 1kP-RESS
			//     2  |     349n     etRh     heet     etRh     tRho     tRho     mP+S     tRho     6uP+     M2/M     2/M1     eetR     heet
			//  UNIT  |    Volts    KOhms    Ohms/    Ohms/    Ohms/    Ohms/    Mirco    Ohms/    Mirco     Ohms    nAmps    Ohms/    KOhms
			//PARCODE |   QB0010   QB0021   QB0030   QB0040   QB0070   QB0080   QB0090   QB0200   QB0210   QB0220   QB0230   QB0240   QB0290
			// L.S.L. |   12.000    3.400   31.000   25.000   40.000  115.000    1.600  210.000    1.300   26.000    0.000    5.800    0.800
			// U.S.L. |   25.000    5.100   51.000   41.000  160.000  335.000    2.400  290.000    1.800   49.000   10.000   11.800    1.200

			// Extract the 13 column names (or less if not all are filled)
			for(iIndex=0;iIndex<13;iIndex++)
			{
				strSection = strString.mid(9+iIndex*9,9).trimmed();	// Remove spaces
				strParameters[iIndex] = strSection;
				strUnits[iIndex]="";
				strLowLimits[iIndex]="";
				strHighLimits[iIndex]="";
			}
			
			// Extract the 13 column Parameters second half of the name
			strString = ReadLine(hWatAsmcFile);
			for(iIndex=0;iIndex<13;iIndex++)
			{
				strSection = strString.mid(9+iIndex*9,9).trimmed();	// Remove spaces
				if(!strSection.isEmpty())
					strParameters[iIndex] += strSection;
			}

			// Extract the 13 column Units
			strString = ReadLine(hWatAsmcFile);
			for(iIndex=0;iIndex<13;iIndex++)
			{
				strSection = strString.mid(9+iIndex*9,9).remove("/").remove("-").trimmed();	// Remove spaces
				if(!strSection.isEmpty())
					strUnits[iIndex] = strSection;
			}

			// skip PARCODE
			strString = ReadLine(hWatAsmcFile);

			// Extract the 13 column Low Limits
			strString = ReadLine(hWatAsmcFile);
			for(iIndex=0;iIndex<13;iIndex++)
			{
				strSection = strString.mid(9+iIndex*9,9).trimmed();	// Remove spaces
				if(!strSection.isEmpty())
					strLowLimits[iIndex] = strSection;
			}

			// Extract the 13 column High Limits
			strString = ReadLine(hWatAsmcFile);
			for(iIndex=0;iIndex<13;iIndex++)
			{
				strSection = strString.mid(9+iIndex*9,9).trimmed();	// Remove spaces
				if(!strSection.isEmpty())
					strHighLimits[iIndex] = strSection;
			}

			// skip the line just after : -------------Key------Key------Key
			strString = ReadLine(hWatAsmcFile);
			continue;
		}

		if(strString.startsWith("WAF", Qt::CaseInsensitive))
		{
			// Extract WaferID, and save it to the parameter list as a parameter itself!
			iWaferID = strString.mid(3,4).toInt();
			iSiteID = 0;
		}
		
		iSiteID++;

		// Read line of Parameter data 
		//WAF   9 |   19.390    4.270   40.000   35.200   70.100  268.800C   2.071  272.000    1.587   42.100    0.155    8.000    0.927 
		//        |   19.390    4.163   39.300   34.200   68.900  265.200C   2.040  266.000    1.576   40.500    0.152    8.200    0.903 
	
		// For each column, extract parameter value and save it
		for(iIndex=0;iIndex<13;iIndex++)
		{
			strSection = strString.mid(9+iIndex*9,9).remove("C ");
			// Save parameter result in buffer
			if(iSiteID == 1)
			{
				// save limits
				SaveParameterLimit(strParameters[iIndex],strUnits[iIndex],strLowLimits[iIndex],eLowLimit);
				SaveParameterLimit(strParameters[iIndex],strUnits[iIndex],strHighLimits[iIndex],eHighLimit);
			}
			if(!strSection.isEmpty())
				SaveParameterResult(iWaferID,iSiteID,strParameters[iIndex],strUnits[iIndex],strSection);
		}
	};

	// Close file
	f.close();

	// All WAT_ASMC file read...check if need to update the WAT_ASMC Parameter list on disk?
	if(m_bNewWatAsmcParameterFound == true)
		DumpParameterIndexTable();

	// Success parsing WAT_ASMC file
	return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from WAT_ASMC data parsed
//////////////////////////////////////////////////////////////////////
bool CGWAT_ASMCtoSTDF::WriteStdfFile(const char *strFileNameSTDF)
{
	// now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open((char*)strFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
	{
		// Failed importing WAT_ASMC file into STDF database
		m_iLastError = errWriteSTDF;
		
		// Convertion failed.
		return false;
	}

	// Write FAR
	RecordReadInfo.iRecordType = 0;
	RecordReadInfo.iRecordSubType = 10;
	StdfFile.WriteHeader(&RecordReadInfo);
	StdfFile.WriteByte(1);					// SUN CPU type
	StdfFile.WriteByte(4);					// STDF V4
	StdfFile.WriteRecord();

	if(m_lStartTime <= 0)
		m_lStartTime = QDateTime::currentDateTime().toTime_t();

	// Write MIR
	RecordReadInfo.iRecordType = 1;
	RecordReadInfo.iRecordSubType = 10;
	StdfFile.WriteHeader(&RecordReadInfo);
	StdfFile.WriteDword(m_lStartTime);			// Setup time
	StdfFile.WriteDword(m_lStartTime);			// Start time
	StdfFile.WriteByte(1);						// Station
	StdfFile.WriteByte((BYTE) 'P');				// Test Mode = PRODUCTION
	StdfFile.WriteByte((BYTE) ' ');				// rtst_cod
	StdfFile.WriteByte((BYTE) ' ');				// prot_cod
	StdfFile.WriteWord(65535);					// burn_tim
	StdfFile.WriteByte((BYTE) ' ');				// cmod_cod
	StdfFile.WriteString(m_strLotId.toLatin1().constData());		// Lot ID
	StdfFile.WriteString(m_strProductId.toLatin1().constData());	// Part Type / Product ID
	StdfFile.WriteString("");					// Node name
	StdfFile.WriteString(m_strTesterType.toLatin1().constData());	// Tester Type
	StdfFile.WriteString("");					// Job name
	StdfFile.WriteString("");					// Job rev
	StdfFile.WriteString("");					// sublot-id
	StdfFile.WriteString("");					// operator
	StdfFile.WriteString("");					// exec-type
	StdfFile.WriteString("");					// exe-ver
	StdfFile.WriteString(m_strTestCode.toLatin1().constData());	// test-cod
	StdfFile.WriteString("");					// test-temperature
	// Construct custom Galaxy USER_TXT 
	QString	strUserTxt;
	strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;	
	strUserTxt += ":";
	strUserTxt += GEX_IMPORT_DATAORIGIN_ETEST;
	strUserTxt += ":WAT_ASMC";
	StdfFile.WriteString(strUserTxt.toLatin1().constData());	// user-txt
	StdfFile.WriteString("");							// aux-file
	StdfFile.WriteString("");							// package-type
	StdfFile.WriteString(m_strProductId.toLatin1().constData());	// familyID
	StdfFile.WriteString("");							// Date-code
	StdfFile.WriteString("");							// Facility-ID
	StdfFile.WriteString("");							// FloorID
	StdfFile.WriteString(m_strProcessId.toLatin1().constData());// ProcessID

	StdfFile.WriteRecord();

	// Write Test results for each waferID
	char		szString[257];
	BYTE		iSiteNumber,bData;
	WORD		wSoftBin,wHardBin;
	long		iTotalGoodBin,iTotalFailBin;
	long		iTestNumber,iPartNumber=0;
	float		fValue;
	bool		bPassStatus;

	
	CGWatAsmcParameter *					ptParam		= NULL;
	CGWatAsmcWafer *						ptWafers	= NULL;		// List of Wafers in WAT_ASMC file
	QList<CGWatAsmcWafer*>::iterator		itWafer		= m_pWaferList.begin();
	QList<CGWatAsmcParameter*>::iterator	itParam;
	
	while(itWafer != m_pWaferList.end())
	{
		ptWafers = (*itWafer);

		// Write WIR
		RecordReadInfo.iRecordType = 2;
		RecordReadInfo.iRecordSubType = 10;
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(1);						// Test head
		StdfFile.WriteByte(255);					// Tester site (all)
		StdfFile.WriteDword(m_lStartTime);			// Start time
		sprintf(szString,"%d",ptWafers->m_iWaferID);
		StdfFile.WriteString(szString);				// WaferID
		StdfFile.WriteRecord();

		// Write all Parameters read on this wafer.: PTR....PTR, PRR
		iTotalGoodBin=iTotalFailBin=0;

		// Write PTRs for EACH of the X sites
		for(iSiteNumber = ptWafers->m_iLowestSiteID; iSiteNumber <= ptWafers->m_iHighestSiteID; iSiteNumber++)
		{
			// before write PIR, PTRs, PRR
			// verify if we have some test executed from this site
			bPassStatus = false;

			itParam = ptWafers->m_pParameterList.begin();	// First test in list
			while(itParam  != ptWafers->m_pParameterList.end())
			{
				if((*itParam)->m_fValue.contains (iSiteNumber) == true)
					bPassStatus = true;
				
				// Next Parameter!
				itParam++;
			};

			if(!bPassStatus)
				continue;
			
			// Write PIR for parts in this Wafer site
			RecordReadInfo.iRecordType = 5;
			RecordReadInfo.iRecordSubType = 10;
			StdfFile.WriteHeader(&RecordReadInfo);
			StdfFile.WriteByte(1);								// Test head
			StdfFile.WriteByte(iSiteNumber);					// Tester site
			StdfFile.WriteRecord();

			// Part number
			iPartNumber++;
			bPassStatus = true;

			itParam = ptWafers->m_pParameterList.begin();	// First test in list
			while(itParam != ptWafers->m_pParameterList.end())
			{
				ptParam = (*itParam);
				
				// Write the PTR if it exists for this site...
				//////////////////////////////////////////////////////////////////////
				// For ProgressBar
				if(GexProgressBar != NULL)
				{
					iParameterCount++;
					while(iParameterCount > iNextParameter)
					{
						iProgressStep += 100/iTotalParameters + 1;
						iNextParameter  += iTotalParameters/100 + 1;
						GexProgressBar->setValue(iProgressStep);
					}
				}
                QCoreApplication::processEvents();

				if(ptParam->m_fValue.contains (iSiteNumber) == true)
				{
					RecordReadInfo.iRecordType = 15;
					RecordReadInfo.iRecordSubType = 10;
					StdfFile.WriteHeader(&RecordReadInfo);
					// Compute Test# (add user-defined offset)
                    iTestNumber = (long) m_pFullWatAsmcParametersList.indexOf(ptParam->m_strName);
					iTestNumber += GEX_TESTNBR_OFFSET_WAT_ASMC;		// Test# offset
					StdfFile.WriteDword(iTestNumber);			// Test Number
					StdfFile.WriteByte(1);						// Test head
					StdfFile.WriteByte(iSiteNumber);			// Tester site:1,2,3,4 or 5, etc.
					fValue = ptParam->m_fValue[iSiteNumber];
					if((fValue < ptParam->m_fLowLimit) || (fValue > ptParam->m_fHighLimit))
					{
						bData = 0200;	// Test Failed
						bPassStatus = false;
					}
					else
					{
						bData = 0;		// Test passed
					}
					StdfFile.WriteByte(bData);							// TEST_FLG
					StdfFile.WriteByte(0x40|0x80);						// PARAM_FLG
					StdfFile.WriteFloat(fValue);	// Test result
					if(ptParam->m_bStaticHeaderWritten == false)
					{
						// save Parameter name 
						StdfFile.WriteString(ptParam->m_strName.toLatin1().constData());	// TEST_TXT
						StdfFile.WriteString("");							// ALARM_ID

						bData = 2;	// Valid data.
						if(ptParam->m_bValidLowLimit==false)
							bData |=0x40;
						if(ptParam->m_bValidHighLimit==false)
							bData |=0x80;
						StdfFile.WriteByte(bData);							// OPT_FLAG

						StdfFile.WriteByte(-ptParam->m_nScale);				// RES_SCALE
						StdfFile.WriteByte(-ptParam->m_nScale);				// LLM_SCALE
						StdfFile.WriteByte(-ptParam->m_nScale);				// HLM_SCALE
						StdfFile.WriteFloat(ptParam->m_bValidLowLimit ? ptParam->m_fLowLimit : 0);			// LOW Limit
						StdfFile.WriteFloat(ptParam->m_bValidHighLimit ? ptParam->m_fHighLimit : 0);			// HIGH Limit
						StdfFile.WriteString(ptParam->m_strUnits.toLatin1().constData());	// Units
						ptParam->m_bStaticHeaderWritten = true;
					}
					StdfFile.WriteRecord();
				}

				// Next Parameter!
				itParam++;
			};

			// Write PRR
			RecordReadInfo.iRecordType = 5;
			RecordReadInfo.iRecordSubType = 20;
			StdfFile.WriteHeader(&RecordReadInfo);
			StdfFile.WriteByte(1);						// Test head
			StdfFile.WriteByte(iSiteNumber);			// Tester site:1,2,3,4 or 5
			if(bPassStatus == true)
			{
				StdfFile.WriteByte(0);				// PART_FLG : PASSED
				wSoftBin = wHardBin = 1;
				iTotalGoodBin++;
			}
			else
			{
				StdfFile.WriteByte(8);				// PART_FLG : FAILED
				wSoftBin = wHardBin = 0;
				iTotalFailBin++;
			}
			StdfFile.WriteWord((WORD)ptWafers->m_pParameterList.count());		// NUM_TEST
			StdfFile.WriteWord(wHardBin);            // HARD_BIN
			StdfFile.WriteWord(wSoftBin);            // SOFT_BIN
			// wafers usually have 5 sites
			switch(iSiteNumber)
			{
				case 1:	// Center
					StdfFile.WriteWord(1);			// X_COORD
					StdfFile.WriteWord(1);			// Y_COORD
					break;
				case 2:	// Left
					StdfFile.WriteWord(1);			// X_COORD
					StdfFile.WriteWord(2);			// Y_COORD
					break;
				case 3:	// Right
					StdfFile.WriteWord(0);			// X_COORD
					StdfFile.WriteWord(1);			// Y_COORD
					break;
				case 4:	// Lower-Left corner
					StdfFile.WriteWord(1);			// X_COORD
					StdfFile.WriteWord(0);			// Y_COORD
					break;
				case 5:	// toUpper-Right corner
					StdfFile.WriteWord(2);			// X_COORD
					StdfFile.WriteWord(1);			// Y_COORD
					break;
				default: // More than 5 sites?....give 0,0 coordonates
					StdfFile.WriteWord(0);			// X_COORD
					StdfFile.WriteWord(0);			// Y_COORD
					break;
			}
			StdfFile.WriteDword(0);				// No testing time known...
			sprintf(szString,"%ld",iPartNumber);
			StdfFile.WriteString(szString);		// PART_ID
			StdfFile.WriteString("");			// PART_TXT
			StdfFile.WriteString("");			// PART_FIX
			StdfFile.WriteRecord();
		}

			
		// Write WRR
		RecordReadInfo.iRecordType = 2;
		RecordReadInfo.iRecordSubType = 20;
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(1);						// Test head
		StdfFile.WriteByte(255);					// Tester site (all)
		StdfFile.WriteDword(m_lStartTime);			// Time of last part tested
		StdfFile.WriteDword(iTotalGoodBin+iTotalFailBin);// Parts tested: always 5
		StdfFile.WriteDword(0);						// Parts retested
		StdfFile.WriteDword(0);						// Parts Aborted
		StdfFile.WriteDword(iTotalGoodBin);			// Good Parts
		StdfFile.WriteDword((DWORD)-1);				// Functionnal Parts

		sprintf(szString,"%d",ptWafers->m_iWaferID);
		StdfFile.WriteString(szString);				// WaferID
		StdfFile.WriteRecord();

#if 0
		// NO SUMMARY FOR ETEST
		// Write SBR Bin0 (FAIL)
		RecordReadInfo.iRecordType = 1;
		RecordReadInfo.iRecordSubType = 50;
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(255);					// Test Head = ALL
		StdfFile.WriteByte(255);					// Test sites = ALL		
		StdfFile.WriteWord(0);						// SBIN = 0
		StdfFile.WriteDword(iTotalFailBin);			// Total Bins
		StdfFile.WriteByte('F');
		StdfFile.WriteRecord();

		// Write SBR Bin1 (PASS)
		RecordReadInfo.iRecordType = 1;
		RecordReadInfo.iRecordSubType = 50;
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(255);					// Test Head = ALL
		StdfFile.WriteByte(255);					// Test sites = ALL		
		StdfFile.WriteWord(1);						// SBIN = 1
		StdfFile.WriteDword(iTotalGoodBin);			// Total Bins
		StdfFile.WriteByte('P');
		StdfFile.WriteRecord();

		// Write HBR Bin0 (FAIL)
		RecordReadInfo.iRecordType = 1;
		RecordReadInfo.iRecordSubType = 40;
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(255);					// Test Head = ALL
		StdfFile.WriteByte(255);					// Test sites = ALL		
		StdfFile.WriteWord(0);						// HBIN = 0
		StdfFile.WriteDword(iTotalFailBin);			// Total Bins
		StdfFile.WriteByte('F');
		StdfFile.WriteRecord();

		// Write HBR Bin1 (PASS)
		RecordReadInfo.iRecordType = 1;
		RecordReadInfo.iRecordSubType = 40;
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(255);					// Test Head = ALL
		StdfFile.WriteByte(255);					// Test sites = ALL		
		StdfFile.WriteWord(1);						// HBIN = 1
		StdfFile.WriteDword(iTotalGoodBin);			// Total Bins
		StdfFile.WriteByte('P');
		StdfFile.WriteRecord();

#endif

		// Not the WaferID we need...see next one.
		itWafer++;
	};

	// Write MRR
	RecordReadInfo.iRecordType = 1; 
	RecordReadInfo.iRecordSubType = 20;
	StdfFile.WriteHeader(&RecordReadInfo);
	StdfFile.WriteDword(0);			// File finish-time.
	StdfFile.WriteRecord();

	// Close STDF file.
	StdfFile.Close();

	// Success
	return true;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' WAT_ASMC file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGWAT_ASMCtoSTDF::Convert(const char *WatAsmcFileName, const char *strFileNameSTDF,QDate *pExpirationDate)
{
	// No erro (default)
	m_iLastError = errNoError;

	// If STDF file already exists...do not rebuild it...unless dates not matching!
	QFileInfo fInput(WatAsmcFileName);
	QFileInfo fOutput(strFileNameSTDF);

    QFile f( strFileNameSTDF );
    if((f.exists() == true) && (fInput.lastModified() < fOutput.lastModified()))
		return true;

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	bool bHideProgressAfter=true;
	bool bHideLabelAfter=false;

	iProgressStep = 0;
	iNextFilePos = 0;
	iNextParameter = 0;
	iTotalParameters = 0;
	iParameterCount = 0;

	if(GexScriptStatusLabel != NULL)
	{
		if(GexScriptStatusLabel->isHidden())
		{
			bHideLabelAfter = true;
			GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(WatAsmcFileName).fileName()+"...");
			GexScriptStatusLabel->show();
		}
	}

	if(GexProgressBar != NULL)
	{
		bHideProgressAfter = GexProgressBar->isHidden();
		GexProgressBar->setMaximum(200);
		GexProgressBar->setTextVisible(true);
		GexProgressBar->setValue(0);
		GexProgressBar->show();
	}
    QCoreApplication::processEvents();
	
	if(ReadWatAsmcFile(WatAsmcFileName,pExpirationDate) != true)
	{
		//////////////////////////////////////////////////////////////////////
		// For ProgressBar
		if((GexProgressBar != NULL)
		&& bHideProgressAfter)
			GexProgressBar->hide();
		
		if((GexScriptStatusLabel != NULL)
		&& bHideLabelAfter)
			GexScriptStatusLabel->hide();
		return false;	// Error reading WAT_ASMC file
	}

	if(WriteStdfFile(strFileNameSTDF) != true)
	{
		//////////////////////////////////////////////////////////////////////
		// For ProgressBar
		if((GexProgressBar != NULL)
		&& bHideProgressAfter)
			GexProgressBar->hide();
		
		if((GexScriptStatusLabel != NULL)
		&& bHideLabelAfter)
			GexScriptStatusLabel->hide();
		QFile::remove(strFileNameSTDF);
		return false;
	}
	
	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	if((GexProgressBar != NULL)
	&& bHideProgressAfter)
		GexProgressBar->hide();
	
	if((GexScriptStatusLabel != NULL)
	&& bHideLabelAfter)
		GexScriptStatusLabel->hide();
	
	// Convertion successful
	return true;
}

//////////////////////////////////////////////////////////////////////
// Read line : skip empty line
//////////////////////////////////////////////////////////////////////
QString CGWAT_ASMCtoSTDF::ReadLine(QTextStream& hFile)
{
	QString strString;
	
	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	if(GexProgressBar != NULL)
	{
        while((int) hFile.device()->pos() > iNextFilePos)
		{
			iProgressStep += 100/iFileSize + 1;
			iNextFilePos  += iFileSize/100 + 1;
			GexProgressBar->setValue(iProgressStep);
		}
	}
    QCoreApplication::processEvents();
			
	do
		strString = hFile.readLine();
	while(!strString.isNull() && strString.isEmpty());

	return strString;

}
