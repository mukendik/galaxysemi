//////////////////////////////////////////////////////////////////////
// import_csmc.cpp: Convert a CSMC file to STDF V4.0
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
#include <QTextStream>
#include <QListIterator>

#include "engine.h"
#include "import_csmc.h"
#include "import_constants.h"

// Old File format:
//CSMC06 WAFER SUMMARY FOR V1.0
//CODE: WM8716 LOT: A68KE41
//09/19/06 ON SYSTEM# AG40721 BY SYF
//
//HEADER CSMCC06 A68KE41 WM8716 09/19/06 22
//TEST#   TESTNAME             UNITS     (  1  )  (  2  )  (  3  )  (  4  )  (  5  )  Median  12345   Lcl     Ucl
//   1    N+ 225/25 RES        OHM/SQ    63.8222  63.9111  63.6889  63.9556  1.1E+03  63.9111  ....O  55.0000  75.0000
//   2    P+ 225/25 RES        OHM/SQ    115.867  115.333  115.511  116.044  1.1E+03  115.867  ....O   90.000  130.000
//   3    POLY1 225/25 RES     OHM/SQ    27.2889  27.6444  28.3111  28.6222  1.1E+03  28.3111  ....O  22.0000  32.0000
//...
// 352    M2 COMB              VOLT      34.5024  34.5024  34.5024  34.5024    .0008  34.5024  ....U  34.0000  35.0000
// 353    M2 MEANDER           mV         .04000   .04000  0.00000   .04000   .04000   .04000  .....  0.00000  1.00000
// 354    M1/M2 COMB           VOLT      34.5024  34.5024  34.5024  34.5024  34.5024  34.5024  .....  34.0000  35.0000
//******** EOW ********
//
//HEADER CSMCC06 A68KE41 WM8716 09/19/06 23
//TEST#   TESTNAME             UNITS     (  1  )  (  2  )  (  3  )  (  4  )  (  5  )  Median  12345   Lcl     Ucl
//   1    N+ 225/25 RES        OHM/SQ    64.0000  64.3111  64.0000  64.3555  63.8667  64.0000  .....  55.0000  75.0000
//   2    P+ 225/25 RES        OHM/SQ    116.044  115.956  115.911  116.400  115.422  115.956  .....   90.000  130.000
//...
// 352    M2 COMB              VOLT      34.5024  34.5032  34.5032  34.5024  34.5024  34.5024  .....  34.0000  35.0000
// 353    M2 MEANDER           mV         .04000  0.00000   .04000  0.00000  0.00000  0.00000  .....  0.00000  1.00000
// 354    M1/M2 COMB           VOLT      34.5024  34.5024  34.5024  34.5032  34.5024  34.5024  .....  34.0000  35.0000
//******** EOW ********
//******** EOL ********
//

// New File format:
//CSMC06 WAFER SUMMARY FOR V1.0
//CODE: WM8725 LOT: A79E251
//10/16/07 ON SYSTEM# AG40711 BY SM
//
//HEADER CSMC06 A79E251 WM8725 10/16/07 01
//   TESTNAME           UNITS     (  1  )  (  2  )  (  3  )  (  4  )  (  5  )  Median  12345   Lcl     Ucl
// N+ 225/25 RES        OHM/SQ    66.9556  66.3778  66.4889  67.5556  66.5556  66.5556  .....  55.0000  75.0000
// P+ 225/25 RES        OHM/SQ    118.467  117.411  117.722  119.700  117.711  117.722  .....   90.000  130.000
// POLY1 225/25 RES     OHM/SQ    27.5556  27.2444  28.1222  28.3444  28.2222  28.1222  .....  22.0000  32.0000

// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGCsmcWafer::~CGCsmcWafer()
{
	while (!m_pParameterList.isEmpty())
		delete m_pParameterList.takeFirst();
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGCSMCtoSTDF::CGCSMCtoSTDF()
{
	// Default: CSMC parameter list on disk includes all known CSMC parameters...
	m_bNewCsmcParameterFound = false;
	m_lStartTime = 0;
}

CGCSMCtoSTDF::~CGCSMCtoSTDF()
{
	while (!m_pWaferList.isEmpty())
		delete m_pWaferList.takeFirst();
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGCSMCtoSTDF::GetLastError()
{
	m_strLastError = "Import CSMC: ";

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
		case errInvalidFormatLowInRows:
			m_strLastError += "Invalid file format: 'Parameter' line too short, missing rows";
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
// Load CSMC Parameter table from DISK
//////////////////////////////////////////////////////////////////////
void CGCSMCtoSTDF::LoadParameterIndexTable(void)
{
	QString	strCsmcTableFile;
	QString	strString;

    strCsmcTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strCsmcTableFile += GEX_CSMC_PARAMETERS;

	// Open CSMC Parameter table file
    QFile f( strCsmcTableFile );
    if(!f.open( QIODevice::ReadOnly ))
		return;

	// Assign file I/O stream
	QTextStream hCsmcTableFile(&f);

	// Skip comment lines
	do
	{
	  strString = hCsmcTableFile.readLine();
	}
    while((strString.indexOf("----------------------") < 0) && (hCsmcTableFile.atEnd() == false));

	// Read lines
	m_pFullCsmcParametersList.clear();
	strString = hCsmcTableFile.readLine();
	while (strString.isNull() == false)
	{
		// Save Parameter name in list
		m_pFullCsmcParametersList.append(strString);
		// Read next line
		strString = hCsmcTableFile.readLine();
	};

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// Save CSMC Parameter table to DISK
//////////////////////////////////////////////////////////////////////
void CGCSMCtoSTDF::DumpParameterIndexTable(void)
{
	QString		strCsmcTableFile;
	QString		strString;

    strCsmcTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strCsmcTableFile += GEX_CSMC_PARAMETERS;

	// Open CSMC Parameter table file
    QFile f( strCsmcTableFile );
    if(!f.open( QIODevice::WriteOnly ))
		return;

	// Assign file I/O stream
	QTextStream hCsmcTableFile(&f);

	// First few lines are comments:
	hCsmcTableFile << "############################################################" << endl;
	hCsmcTableFile << "# DO NOT EDIT THIS FILE!" << endl;
    hCsmcTableFile << "# Quantix Examinator: CSMC Parameters detected" << endl;
	hCsmcTableFile << "# www.mentor.com" << endl;
    hCsmcTableFile << "# Quantix Examinator reads and writes into this file..." << endl;
	hCsmcTableFile << "-----------------------------------------------------------" << endl;

	// Write lines
	// m_pFullCsmcParametersList.sort();
	for (QStringList::const_iterator
		 iter  = m_pFullCsmcParametersList.begin();
		 iter != m_pFullCsmcParametersList.end(); ++iter) {
		// Write line
		hCsmcTableFile << *iter << endl;
	}

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// If Examinator doesn't have this CSMC parameter in his dictionnary, have it added.
//////////////////////////////////////////////////////////////////////
void CGCSMCtoSTDF::UpdateParameterIndexTable(QString strParamName)
{
	// Check if the table is empty...if so, load it from disk first!
	if(m_pFullCsmcParametersList.isEmpty() == true)
	{
		// Load CSMC parameter table from disk...
		LoadParameterIndexTable();
	}
	
	// Check if Parameter name already in table...if not, add it to the list
	// the new full list will be dumped to the disk at the end.
    if(m_pFullCsmcParametersList.indexOf(strParamName) < 0)
	{
		// Update list
		m_pFullCsmcParametersList.append(strParamName);

		// Set flag to force the current CSMC table to be updated on disk
		m_bNewCsmcParameterFound = true;
	}
}

//////////////////////////////////////////////////////////////////////
// Find or create Parameter entry, return handle to it.
//////////////////////////////////////////////////////////////////////
CGCsmcParameter *CGCSMCtoSTDF::FindParameterEntry(int iWaferID,int iSiteID,QString strParamName,QString strUnits)
{
	CGCsmcWafer	*		ptWafers;		// List of Wafers in CSMC file
	CGCsmcParameter *	ptParam;		// List of parameters
	
	QList<CGCsmcWafer*>::iterator itWaferBegin	= m_pWaferList.begin();
	QList<CGCsmcWafer*>::iterator itWaferEnd	= m_pWaferList.end();
	
	while(itWaferBegin != itWaferEnd)
	{
		if((*itWaferBegin)->m_iWaferID == iWaferID)
		{
			// Update Min/Max SiteID
			if(iSiteID > 0)
			{
				(*itWaferBegin)->m_iLowestSiteID=gex_min((*itWaferBegin)->m_iLowestSiteID,iSiteID);				// Lowest WaferID found in CSMC file
				(*itWaferBegin)->m_iHighestSiteID=gex_max((*itWaferBegin)->m_iHighestSiteID,iSiteID);			// Highest WaferID found in CSMC file
			}

			// Found WaferID entry...now find Parameter entry
			QList<CGCsmcParameter*>::iterator itParameterBegin	= (*itWaferBegin)->m_pParameterList.begin();
			QList<CGCsmcParameter*>::iterator itParameterEnd	= (*itWaferBegin)->m_pParameterList.end();
			
			while(itParameterBegin != itParameterEnd)
			{
				if((*itParameterBegin)->m_strName == strParamName)
					return (*itParameterBegin);	// Found the Parameter!

				// Not the Parameter we need...see next one.
				itParameterBegin++;
			};

			// Parameter not in list...create entry!
			// Create & add new Parameter entry in Wafer list
			ptParam = new CGCsmcParameter;
			ptParam->m_strName = strParamName;
			ptParam->m_strUnits = strUnits;
			ptParam->m_fHighLimit = ptParam->m_fLowLimit = 0;
			NormalizeLimits(ptParam);
			ptParam->m_bValidLowLimit = false;		// Low limit defined
			ptParam->m_bValidHighLimit = false;	// High limit defined

			ptParam->m_bStaticHeaderWritten = false;
			(*itWaferBegin)->m_pParameterList.append(ptParam);
			iTotalParameters++;
			// If Examinator doesn't have this CSMC parameter in his dictionnary, have it added.
			UpdateParameterIndexTable(strParamName);
			
			// Return pointer to the Parameter 
			return ptParam;
		}

		// Not the WaferID we need...see next one.
		itWaferBegin++;
	};

	// WaferID entry doesn't exist yet...so create it now!
	ptWafers = new CGCsmcWafer;
	ptWafers->m_iLowestSiteID=ptWafers->m_iHighestSiteID=0;
	ptWafers->m_iWaferID = iWaferID;

	// Update Min/Max SiteID
	if(iSiteID > 0)
	{
		ptWafers->m_iLowestSiteID=iSiteID;				// Lowest WaferID found in CSMC file
		ptWafers->m_iHighestSiteID=iSiteID;				// Highest WaferID found in CSMC file
	}

	// Create & add new Parameter entry in Wafer list
	ptParam = new CGCsmcParameter;
	ptParam->m_strName = strParamName;
	ptParam->m_strUnits = strUnits;
	ptParam->m_nScale = 0;
	ptParam->m_bValidLowLimit = false;		// Low limit defined
	ptParam->m_bValidHighLimit = false;	// High limit defined
	ptParam->m_bStaticHeaderWritten = false;
	ptWafers->m_pParameterList.append(ptParam);
	iTotalParameters++;
	
	// If Examinator doesn't have this CSMC parameter in his dictionnary, have it added.
	UpdateParameterIndexTable(strParamName);
	
	// Add Wafer entry to existing list.
	m_pWaferList.append(ptWafers);

	// Return pointer to the Parameter 
	return ptParam;
}

//////////////////////////////////////////////////////////////////////
// Save CSMC parameter result...
//////////////////////////////////////////////////////////////////////
void CGCSMCtoSTDF::SaveParameterResult(int iWaferID,int iSiteID,QString strName,QString strUnits,QString strValue)
{
	if(strName.isNull() == true)
		return;	// Ignore empty entry!

	// Find Pointer to the Parameter cell
	CGCsmcParameter *ptParam = FindParameterEntry(iWaferID,iSiteID,strName,strUnits);

	// Save the Test value in it
	strValue.remove ('x');	// Remove any 'x' character if found (sometimes present to flag entry out of specs)
	strValue = strValue.trimmed();
	ptParam->m_fValue[iSiteID] = strValue.toFloat() * GS_POW(10.0,ptParam->m_nScale);
}

//////////////////////////////////////////////////////////////////////
// Save CSMC parameter High or Low Limit result...
//////////////////////////////////////////////////////////////////////
void CGCSMCtoSTDF::SaveParameterLimit(QString strName,QString strValue,int iLimit)
{
	if(strName.isNull() == true)
		return;	// Ignore empty entry!
	if(strValue.isNull() == true)
		return;	// Ignore empty entry!

	bool		bIsFloat;
	CGCsmcParameter *ptParam;	// List of parameters
	QList<CGCsmcWafer*>::iterator itBegin	= m_pWaferList.begin();
	QList<CGCsmcWafer*>::iterator itEnd		= m_pWaferList.end();
	
	while(itBegin != itEnd)
	{
		// Get Parameter pointer...save limits in ALL WaferEntry.
		ptParam = FindParameterEntry((*itBegin)->m_iWaferID,0,strName);
		switch(iLimit)
		{
			case eHighLimit:
				ptParam->m_fHighLimit = strValue.toFloat(&bIsFloat) * GS_POW(10.0,ptParam->m_nScale);
				ptParam->m_bValidHighLimit = bIsFloat;		// High limit defined

				break;
			case eLowLimit:
				ptParam->m_fLowLimit = strValue.toFloat(&bIsFloat)  * GS_POW(10.0,ptParam->m_nScale);
				ptParam->m_bValidLowLimit = bIsFloat;		// Low limit defined
				break;
		}

		// Move to next wafer entry
		itBegin++;
	};
}

//////////////////////////////////////////////////////////////////////
// Normalize test limits when writing into PTR.
//////////////////////////////////////////////////////////////////////
void CGCSMCtoSTDF::NormalizeLimits(CGCsmcParameter *pParameter)
{
	int	value_scale=0;
	if(pParameter->m_strUnits.length() <= 1)
	{
		// units too short to include a prefix, then keep it 'as-is'
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
// Check if File is compatible with CSMC format
//////////////////////////////////////////////////////////////////////
bool CGCSMCtoSTDF::IsCompatible(const char *szFileName)
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
	QTextStream hCsmcFile(&f);

	// Check if first line is the correct CSMC header...
	// CSMC06 WAFER SUMMARY FOR V1.0
	do
		strString = hCsmcFile.readLine();
	while(!strString.isNull() && strString.isEmpty());

	strString = strString.trimmed();	// remove leading spaces.
	
	// Close file
	f.close();

	if(!strString.startsWith("CSMC06 WAFER SUMMARY FOR V1.0", Qt::CaseInsensitive))
	{
		// Incorrect header...this is not a CSMC file!
		return false;
	}


	return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the CSMC file
//////////////////////////////////////////////////////////////////////
bool CGCSMCtoSTDF::ReadCsmcFile(const char *CsmcFileName)
{
	QString		strString;
	QString		strSection;
	QString		strValue;
	QString		strSite;
	QString		strParameterNb;		// 
	QString		strParameter;		// 
	QString		strUnit;			// 
	int			iWaferID;			// WaferID processed
	int			iIndex;				// Loop index
	QStringList	lstSections;

	// Open CSMC file
    QFile f( CsmcFileName );
    if(!f.open( QIODevice::ReadOnly ))
	{
		// Failed Opening CSMC file
		m_iLastError = errOpenFail;
		
		// Convertion failed.
		return false;
	}
	// Assign file I/O stream
	QTextStream hCsmcFile(&f);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	iFileSize = f.size() + 1;

	// Check if first line is the correct CSMC header...
	// CSMC06 WAFER SUMMARY FOR V1.0
	strString = ReadLine(hCsmcFile);
	strString = strString.trimmed();	// remove leading spaces.
	if(!strString.startsWith("CSMC06 WAFER SUMMARY FOR V1.0", Qt::CaseInsensitive))
	{
		// Incorrect header...this is not a CSMC file!
		m_iLastError = errInvalidFormat;
		
		// Convertion failed.
		// Close file
		f.close();
		return false;
	}
	// Read CSMC information

	QString strDate;
	strString = ReadLine(hCsmcFile);
    lstSections = strString.split(" ");
	// Check if have the good count
	if(lstSections.count() <= 3)
	{
		m_iLastError = errInvalidFormatLowInRows;

		// Convertion failed.
		// Close file
		f.close();
		return false;
	}


	//CODE: WM8716 LOT: A68KE41
	strSection = lstSections[0];
	if(!strSection.startsWith("CODE", Qt::CaseInsensitive))
	{
		// Incorrect header...this is not a CSMC file!
		m_iLastError = errInvalidFormat;
		
		// Convertion failed.
		// Close file
		f.close();
		return false;
	}
	m_strProductID = lstSections[1];
	
	strSection = lstSections[2];
	if(!strSection.startsWith("LOT", Qt::CaseInsensitive))
	{
		// Incorrect header...this is not a CSMC file!
		m_iLastError = errInvalidFormat;
		
		// Convertion failed.
		// Close file
		f.close();
		return false;
	}
	m_strLotID = lstSections[3];

	strString = ReadLine(hCsmcFile).trimmed();
	//09/19/06 ON SYSTEM# AG40721 BY SYF
	strDate = strString.section(" ",0,0);
	QDate		qDate(strDate.section("/",2,2).toInt()+2000,strDate.section("/",0,0).toInt(),strDate.section("/",1,1).toInt());
	QDateTime clDateTime(qDate);
	clDateTime.setTimeSpec(Qt::UTC);
	m_lStartTime = clDateTime.toTime_t();
	strString = strString.section("SYSTEM#",1,1).trimmed();
	m_strNodeName = strString.section("BY",0,0).trimmed();
	m_strOperatorID = strString.section("BY",1,1).trimmed();

	// Read line with list of Parameters names

  //FIXME: not used ?
  //bool bHaveTestNumber;
  //bHaveTestNumber = false;
	int nStart, nSize;
	nStart = nSize = 0;
	// Loop reading file until end is reached
	do
	{	
		if(!strString.startsWith("HEADER CSMC06", Qt::CaseInsensitive))
		{
			strString = ReadLine(hCsmcFile);
			continue;
		}
		//HEADER CSMC06 A68KE41 WM8716 09/19/06 01
		// Extract WaferID, and save it to the parameter list as a parameter itself!
		strSection = strString.trimmed().right(2);
		iWaferID = strSection.toInt();
		
		strString = ReadLine(hCsmcFile).trimmed();
		// Old format
		//TEST#   TESTNAME             UNITS     (  1  )  (  2  )  (  3  )  (  4  )  (  5  )  Median  12345   Lcl     Ucl
		// New format
		//   TESTNAME             UNITS     (  1  )  (  2  )  (  3  )  (  4  )  (  5  )  Median  12345   Lcl     Ucl
		strSection = strString.section(" ",0,0);
		if(strSection.startsWith("TEST#", Qt::CaseInsensitive))
		{
			// Old format
			// Ignore TEST# column
			nStart = 7;
		}
		else
		if(strSection.startsWith("TESTNAME", Qt::CaseInsensitive))
		{
			// New format
			nStart = 0;
		}
		else
		{
			// Incorrect header...this is not a CSMC file!
			m_iLastError = errInvalidFormat;
			
			// Convertion failed.
			// Close file
			f.close();
			return false;
		}
		strString = ReadLine(hCsmcFile);

		do
		{
			// N+ 225/25 RES        OHM/SQ    66.9556  66.3778  66.4889  67.5556  66.5556  66.5556  .....  55.0000  75.0000
			//   1    N+ 225/25 RES        OHM/SQ    64.1778  64.5778  64.0889  64.3555  64.0889  64.1778  .....  55.0000  75.0000
			// Read data
			strParameter = strString.mid(nStart,20).trimmed();
			strUnit =  strString.mid(21+nStart,10).trimmed();
			strString = strString.mid(31+nStart,79).trimmed();//strString.right(78); // all other values
			

			// For each column, extract result value and save it
			for(iIndex=0;iIndex<5;iIndex++)
			{
				strSection = strString.mid(iIndex*9,9).trimmed();
					// Save parameter result in buffer
				if(!strSection.isEmpty())
					SaveParameterResult(iWaferID,iIndex+1,strParameter,strUnit,strSection);
			}

			SaveParameterLimit(strParameter,strString.mid(60,8).trimmed(),eLowLimit);
			SaveParameterLimit(strParameter,strString.mid(69).trimmed(),eHighLimit);
			
			// Read next line in file...
			strString = ReadLine(hCsmcFile);
		}
		while(!strString.startsWith("******** EOW ********") && !hCsmcFile.atEnd());

		// Check if last line processed (should normally never happen as statistcs should follow...)
		if(hCsmcFile.atEnd())
			break;;

	}
	while(!strString.startsWith("******** EOL ********") && !hCsmcFile.atEnd());


	// Close file
	f.close();

	// All CSMC file read...check if need to update the CSMC Parameter list on disk?
	if(m_bNewCsmcParameterFound == true)
		DumpParameterIndexTable();

	// Success parsing CSMC file
	return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from CSMC data parsed
//////////////////////////////////////////////////////////////////////
bool CGCSMCtoSTDF::WriteStdfFile(const char *strFileNameSTDF)
{
	// now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open((char*)strFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
	{
		// Failed importing CSMC file into STDF database
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
	StdfFile.WriteString(m_strLotID.toLatin1().constData());		// Lot ID
	StdfFile.WriteString(m_strProductID.toLatin1().constData());	// Part Type / Product ID
	StdfFile.WriteString(m_strNodeName.toLatin1().constData());		// Node name
	StdfFile.WriteString("");					// Tester Type
	StdfFile.WriteString("");					// Job name
	StdfFile.WriteString("");					// Job rev
	StdfFile.WriteString("");					// sublot-id
	StdfFile.WriteString(m_strOperatorID.toLatin1().constData());		// operator
	StdfFile.WriteString("");					// exec-type
	StdfFile.WriteString("");					// exe-ver
	StdfFile.WriteString("WAFER");				// test-cod
	StdfFile.WriteString("");					// test-temperature
	// Construct custom Galaxy USER_TXT 
	QString	strUserTxt;
	strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;	
	strUserTxt += ":";
	strUserTxt += GEX_IMPORT_DATAORIGIN_ETEST;
	strUserTxt += ":CSMC";
	StdfFile.WriteString(strUserTxt.toLatin1().constData());		// user-txt
	StdfFile.WriteString("");					// aux-file
	StdfFile.WriteString("");					// package-type
	StdfFile.WriteString(m_strProductID.toLatin1().constData());	// familyID
	StdfFile.WriteString("");					// Date-code
	StdfFile.WriteString("");					// Facility-ID
	StdfFile.WriteString("");					// FloorID
	StdfFile.WriteString("");					// ProcessID

	StdfFile.WriteRecord();

	// Write Test results for each waferID
	CGCsmcWafer	*ptWafers;		// List of Wafers in CSMC file
	char		szString[257];
	BYTE		iSiteNumber,bData;
	WORD		wSoftBin,wHardBin;
	long		iTotalGoodBin,iTotalFailBin;
	long		iTestNumber,iPartNumber=0;
	bool		bPassStatus;
	int			iParameterCount = 0;

	CGCsmcParameter *ptParam;
	
	QListIterator<CGCsmcWafer*> lstIteratorWafer(m_pWaferList);
	lstIteratorWafer.toFront();

	while(lstIteratorWafer.hasNext())
	{
		ptWafers = lstIteratorWafer.next();

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
			QList<CGCsmcParameter*>::iterator itParameterBegin	= ptWafers->m_pParameterList.begin();
			QList<CGCsmcParameter*>::iterator itParameterEnd	= ptWafers->m_pParameterList.end();
			
			bPassStatus = false;
			while(itParameterBegin != itParameterEnd)
			{
				if((*itParameterBegin)->m_fValue.contains (iSiteNumber) == true)
					bPassStatus = true;

				// Next Parameter!
				itParameterBegin++;
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

			bPassStatus			= true;
			itParameterBegin	= ptWafers->m_pParameterList.begin();
		
			while(itParameterBegin != itParameterEnd)
			{
				ptParam = (*itParameterBegin);

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

				// Write the PTR if it exists for this site...
				if(ptParam->m_fValue.contains (iSiteNumber) == true)
				{
					RecordReadInfo.iRecordType = 15;
					RecordReadInfo.iRecordSubType = 10;
					StdfFile.WriteHeader(&RecordReadInfo);
					// Compute Test# (add user-defined offset)
                    iTestNumber = (long) m_pFullCsmcParametersList.indexOf(ptParam->m_strName);
					iTestNumber += GEX_TESTNBR_OFFSET_CSMC;		// Test# offset
					StdfFile.WriteDword(iTestNumber);			// Test Number
					StdfFile.WriteByte(1);						// Test head
					StdfFile.WriteByte(iSiteNumber);			// Tester site:1,2,3,4 or 5, etc.
					if((ptParam->m_fValue[iSiteNumber] < ptParam->m_fLowLimit) || (ptParam->m_fValue[iSiteNumber] > ptParam->m_fHighLimit))
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
					StdfFile.WriteFloat(ptParam->m_fValue[iSiteNumber]);	// Test result
					if(ptParam->m_bStaticHeaderWritten == false)
					{
						// save Parameter name without unit information
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
						StdfFile.WriteFloat(ptParam->m_bValidHighLimit ? ptParam->m_fHighLimit : 0);		// HIGH Limit
						StdfFile.WriteString(ptParam->m_strUnits.toLatin1().constData());	// Units
						ptParam->m_bStaticHeaderWritten = true;
					}
					StdfFile.WriteRecord();
				}

				// Next Parameter!
				itParameterBegin++;
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
			// 200mm wafers, usually have 5 sites, 300mm wafers usually have 9 sites
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
				case 5:	// Upper-Right corner
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
		StdfFile.WriteDword(iTotalFailBin+iTotalGoodBin);	// Parts tested: always 5
		StdfFile.WriteDword(0);						// Parts retested
		StdfFile.WriteDword(0);						// Parts Aborted
		StdfFile.WriteDword(iTotalGoodBin);			// Good Parts
		StdfFile.WriteDword((DWORD)(-1));			// Functionnal Parts

		sprintf(szString,"%d",ptWafers->m_iWaferID);
		StdfFile.WriteString(szString);				// WaferID
		StdfFile.WriteRecord();

#if O
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
	};

	// Write MRR
	RecordReadInfo.iRecordType = 1; 
	RecordReadInfo.iRecordSubType = 20;
	StdfFile.WriteHeader(&RecordReadInfo);
	StdfFile.WriteDword(m_lStartTime);			// File finish-time.
	StdfFile.WriteRecord();

	// Close STDF file.
	StdfFile.Close();

	// Success
	return true;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' CSMC file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGCSMCtoSTDF::Convert(const char *CsmcFileName, const char *strFileNameSTDF)
{
	// No erro (default)
	m_iLastError = errNoError;

	// If STDF file already exists...do not rebuild it...unless dates not matching!
	QFileInfo fInput(CsmcFileName);
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

	if(GexScriptStatusLabel != NULL)
	{
		if(GexScriptStatusLabel->isHidden())
		{
			bHideLabelAfter = true;
			GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(CsmcFileName).fileName()+"...");
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

    if(ReadCsmcFile(CsmcFileName) != true)
	{
		//////////////////////////////////////////////////////////////////////
		// For ProgressBar
		if((GexProgressBar != NULL)
		&& bHideProgressAfter)
			GexProgressBar->hide();
		
		if((GexScriptStatusLabel != NULL)
		&& bHideLabelAfter)
			GexScriptStatusLabel->hide();

		return false;	// Error reading CSMC file
	}

	iNextFilePos = 0;

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
QString CGCSMCtoSTDF::ReadLine(QTextStream& hFile)
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
