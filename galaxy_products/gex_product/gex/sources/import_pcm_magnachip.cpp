//////////////////////////////////////////////////////////////////////
// import_pcm_magnachip.cpp: Convert a .txt PCM MAgnachip summary KDF file to STDF V4.0
//////////////////////////////////////////////////////////////////////
#include <time.h>

#ifdef _WIN32
#include "windows.h"		// For 'GetWindowsDirectoryA' declaration
#endif

#include <qfileinfo.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>
#include <QListIterator>

#include "engine.h"
#include "import_wat.h"
#include "import_pcm_magnachip.h"
#include "import_constants.h"

// File format:
//										 Lot Summary Report KDF V1.1											Page: 1
//																								
//Fab ID	: Gumi Fab3																							
//Date	: 27-Jul-2005																							
//Time	: 15:19:31																							
//Device	: HF353362																							
//Lot ID	: ZBE416-R1																							
//Process	: 2P3M, 5/12V Process, Rev0.																							
//																								
//	Para	Vth_NC1		IDoff_NC1	IDsat_NC1	BVdss_NC1	Vth_NC2		IDoff_NC2	IDsat_NC2	Isub_NC2	BVdss_NC2	Vth_NC5		Vth_NC5B	IDsat_NC5
//	Min	0.67 		0		400		9		0.67 		0		400		0.00		9		0.67 		1.27		19 
//	Typ	0.75 		1		460		12		0.75 		1		460		3		12		0.75		1.35		22
//	Max	0.83 		100		520		15		0.83 		100		520		100 		15		0.67		1.43		25 
//	Unit	V		pA		uA/um		V		V		pA		uA/um		pA		V		V		V		uA/um
//Site	Wafer																							
//B	4	0.422		2.637E+07	628.110		0.000		0.576		1.100E+04	555.330		5.202		0.000		0.730		1.318		23.451
//T	4	0.428		1.976E+07	620.780		0.000		0.595		1.230E+03	543.280		4.761		0.000		0.730		1.312		23.521
// ...
//R	9	0.650		5.849E+01	516.580		7.083		0.721		7.520E-01	475.840		3.188		12.083		0.738		1.328		23.209
//L	9	0.646		2.653E+01	519.450		7.500		0.716		1.021E+00	478.440		3.206		12.083		0.732		1.316		23.636
//	Average	0.515		1.039E+07	528.618		1.208		0.632		2.364E+04	517.033		4.130		6.292		-6.164		-142.907	37.197
//	Stdev	0.090		1.741E+07	161.120		2.793		0.075		9.882E+04	28.077		0.743		4.871		37.673		472.537		41.974
//																								
//																								
//										 Lot Summary Report KDF V1.1											Page: 2
//																								
//Fab ID	: Gumi Fab3																							
//Date	: 27-Jul-2005																							
//Time	: 15:19:31																							
//Device	: HF353362																							
//Lot ID	: ZBE416-R1																							
//Process	: 2P3M, 5/12V Process, Rev0.																							
//																								
//	Para	Vth_PC1		IDoff_PC1	IDsat_PC1	BVdss_PC1	Vth_PC2		IDoff_PC2	IDsat_PC2	BVdss_PC2	Vth_PC5		Vth_PC5B	IDsat_PC5	Vth_NCO7
//	Min	-0.83		-100		-300		-13		-0.83		-100		-280		-13		-0.98 		-1.43		-8 		0.67
// ....

// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGPCM_KDFtoSTDF::CGPCM_KDFtoSTDF()
{
	// Default: WAT parameter list on disk includes all known WAT parameters...
	m_bNewWatParameterFound = false;
	m_lStartTime = 0;
}

CGPCM_KDFtoSTDF::~CGPCM_KDFtoSTDF()
{
	while (!m_pWaferList.isEmpty())
		delete m_pWaferList.takeFirst();
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGPCM_KDFtoSTDF::GetLastError()
{
	m_strLastError = "Import PCM (Magnachip): ";

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
// Load PCM Parameter table from DISK
//////////////////////////////////////////////////////////////////////
void CGPCM_KDFtoSTDF::LoadParameterIndexTable(void)
{
	QString	strWatTableFile;
	QString	strString;

    strWatTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strWatTableFile += GEX_WAT_PARAMETERS;

	// Open WAT Parameter table file
    QFile f( strWatTableFile );
    if(!f.open( QIODevice::ReadOnly ))
		return;

	// Assign file I/O stream
	QTextStream hWatTableFile(&f);

	// Skip comment lines
	do
	{
	  strString = hWatTableFile.readLine();
	}
    while((strString.indexOf("----------------------") < 0) && (hWatTableFile.atEnd() == false));

	// Read lines
	m_pFullWatParametersList.clear();
	strString = hWatTableFile.readLine();
	while (strString.isNull() == false)
	{
		// Save Parameter name in list
		m_pFullWatParametersList.append(strString);
		// Read next line
		strString = hWatTableFile.readLine();
	};

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// Save PCM Parameter table to DISK
//////////////////////////////////////////////////////////////////////
void CGPCM_KDFtoSTDF::DumpParameterIndexTable(void)
{
	QString		strWatTableFile;

    strWatTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strWatTableFile += GEX_WAT_PARAMETERS;

	// Open WAT Parameter table file
    QFile f( strWatTableFile );
    if(!f.open( QIODevice::WriteOnly ))
		return;

	// Assign file I/O stream
	QTextStream hWatTableFile(&f);

	// First few lines are comments:
	hWatTableFile << "############################################################" << endl;
	hWatTableFile << "# DO NOT EDIT THIS FILE!" << endl;
    hWatTableFile << "# Quantix Examinator: WAT/PCM Parameters detected" << endl;
	hWatTableFile << "# www.mentor.com" << endl;
    hWatTableFile << "# Quantix Examinator reads and writes into this file..." << endl;
	hWatTableFile << "-----------------------------------------------------------" << endl;

	// Write lines
	// m_pFullWatParametersList.sort();
	for(int nIndex=0; nIndex < m_pFullWatParametersList.count(); nIndex++)
	{
		// Write line
		hWatTableFile << m_pFullWatParametersList[nIndex] << endl;
	};

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// If Examinator doesn't have this PCM parameter in his dictionnary, have it added.
//////////////////////////////////////////////////////////////////////
void CGPCM_KDFtoSTDF::UpdateParameterIndexTable(QString strParamName)
{
	// Check if the table is empty...if so, load it from disk first!
	if(m_pFullWatParametersList.isEmpty() == true)
	{
		// Load WAT parameter table from disk...
		LoadParameterIndexTable();
	}
	
	// Check if Parameter name already in table...if not, add it to the list
	// the new full list will be dumped to the disk at the end.
    if(m_pFullWatParametersList.indexOf(strParamName) < 0)
	{
		// Update list
		m_pFullWatParametersList.append(strParamName);

		// Set flag to force the current PCM table to be updated on disk
		m_bNewWatParameterFound = true;
	}
}

//////////////////////////////////////////////////////////////////////
// Find or create Parameter entry, return handle to it.
//////////////////////////////////////////////////////////////////////
CGWatParameter *CGPCM_KDFtoSTDF::FindParameterEntry(int iWaferID,int iSiteID,QString strParamName,QString strUnits)
{
	CGWatWafer	*ptWafers;		// List of Wafers in PCM file
	CGWatParameter *ptParam;	// List of parameters

	if(m_pWaferList.count() > 0)
	{
		ptWafers = m_pWaferList.first();
		
		QListIterator<CGWatWafer*> lstIteratorWafer(m_pWaferList);

		lstIteratorWafer.toFront();
		while(lstIteratorWafer.hasNext())
		{
			ptWafers = lstIteratorWafer.next();

			if(ptWafers->iWaferID == iWaferID)
			{
				// Update Min/Max SiteID
				if(iSiteID > 0)
				{
					ptWafers->iLowestSiteID=gex_min(ptWafers->iLowestSiteID,iSiteID);				// Lowest WaferID found in WAT file
					ptWafers->iHighestSiteID=gex_max(ptWafers->iHighestSiteID,iSiteID);				// Highest WaferID found in WAT file
				}

				// Found WaferID entry...now find Parameter entry
				QList<CGWatParameter*>::iterator itParam = ptWafers->pParameterList.begin();
				
				while(itParam != ptWafers->pParameterList.end())
				{
					if((*itParam)->strName == strParamName)
						return (*itParam);	// Found the Parameter!

					// Not the Parameter we need...see next one.
					itParam++;
				};

				// Parameter not in list...create entry!
				// Create & add new Parameter entry in Wafer list
				ptParam = new CGWatParameter;
				ptParam->strName = strParamName;
				ptParam->strUnits = strUnits;
				ptParam->bValidLowLimit = false;		// Low limit defined
				ptParam->bValidHighLimit = false;	// High limit defined

				ptParam->bStaticHeaderWritten = false;
				ptWafers->pParameterList.append(ptParam);
				iTotalParameters++;
				// If Examinator doesn't have this WAT parameter in his dictionnary, have it added.
				UpdateParameterIndexTable(strParamName);
				
				// Return pointer to the Parameter 
				return ptParam;
			}
		};
	}

	// WaferID entry doesn't exist yet...so create it now!
	ptWafers = new CGWatWafer;
	ptWafers->iWaferID = iWaferID;

	// Update Min/Max SiteID
	if(iSiteID > 0)
	{
		ptWafers->iLowestSiteID=iSiteID;				// Lowest WaferID found in WAT file
		ptWafers->iHighestSiteID=iSiteID;				// Highest WaferID found in WAT file
	}

	// Create & add new Parameter entry in Wafer list
	ptParam = new CGWatParameter;
	ptParam->strName = strParamName;
	ptParam->strUnits = strUnits;
	ptParam->bValidLowLimit = false;		// Low limit defined
	ptParam->bValidHighLimit = false;	// High limit defined
	ptParam->bStaticHeaderWritten = false;
	ptWafers->pParameterList.append(ptParam);
	iTotalParameters++;

	// If Examinator doesn't have this WAT parameter in his dictionnary, have it added.
	UpdateParameterIndexTable(strParamName);
	
	// Add Wafer entry to existing list.
	m_pWaferList.append(ptWafers);

	// Return pointer to the Parameter 
	return ptParam;
}

//////////////////////////////////////////////////////////////////////
// Save WAT parameter result...
//////////////////////////////////////////////////////////////////////
void CGPCM_KDFtoSTDF::SaveParameterResult(int iWaferID,int iSiteID,QString strName,QString strUnits,QString strValue)
{
	if(strName.isNull() == true)
		return;	// Ignore empty entry!

	// Find Pointer to the Parameter cell
	CGWatParameter *ptParam = FindParameterEntry(iWaferID,iSiteID,strName,strUnits);

	// Save the Test value in it
	strValue.remove ('x');	// Remove any 'x' character if found (sometimes present to flag entry out of specs)
	strValue = strValue.trimmed();
	ptParam->fValue[iSiteID] = strValue.toFloat();
}

//////////////////////////////////////////////////////////////////////
// Save WAT parameter High or Low Limit result...
//////////////////////////////////////////////////////////////////////
void CGPCM_KDFtoSTDF::SaveParameterLimit(QString strName,QString strValue,int iLimit)
{
	if(strName.isNull() == true)
		return;	// Ignore empty entry!

	CGWatParameter *				ptParam;	// List of parameters
	QList<CGWatWafer*>::iterator	itBegin	= m_pWaferList.begin();
	QList<CGWatWafer*>::iterator	itEnd	= m_pWaferList.end();
	
	while(itBegin != itEnd)
	{
		// Get Parameter pointer...save limits in ALL WaferEntry.
		ptParam = FindParameterEntry((*itBegin)->iWaferID,0,strName);
		switch(iLimit)
		{
			case eHighLimit:
				ptParam->fHighLimit = strValue.toFloat();
				ptParam->bValidHighLimit = true;		// High limit defined

				break;
			case eLowLimit:
				ptParam->fLowLimit = strValue.toFloat();
				ptParam->bValidLowLimit = true;		// Low limit defined
				break;
		}

		// Move to next wafer entry
		itBegin++;
	};
}


//////////////////////////////////////////////////////////////////////
// Check if File is compatible with PCM_KDF format
//////////////////////////////////////////////////////////////////////
bool CGPCM_KDFtoSTDF::IsCompatible(const char *szFileName)
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
	QTextStream hWatFile(&f);

	// Loop reading file until end is reached
	do
		strString = hWatFile.readLine();
	while(!strString.isNull() && strString.isEmpty());

	// Check if first line is the correct WAT header...
	strString = strString.trimmed();	// remove leading spaces.
	if(!strString.startsWith("Lot Summary Report KDF"))
	{
		// Incorrect header...this is not a PCM file!
		// Close file
		f.close();
		return false;
	}

	// Read line : "Fab ID	: Gumi Fab3
	do
		strString = hWatFile.readLine();
	while(!strString.isNull() && strString.isEmpty());

	strSection = strString.section(':',0,0); // Fab ID
	strSection = strSection.trimmed();	// remove leading spaces.
	if(strSection.startsWith("Fab ID") == false)
	{
		// Incorrect header...this is not a PCM file!
		// Close file
		f.close();
		return false;
	}

	// Close file
	f.close();

	return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the WAT file
//////////////////////////////////////////////////////////////////////
bool CGPCM_KDFtoSTDF::ReadWatFile(const char *WatFileName)
{
	QString strString;
	QString strSection;
	QString	strSite;
	QStringList strCells;
	QStringList strHighLimitsCells;
	QStringList strLowLimitsCells;
	QString	strParameters[12];	// Holds the 12 Parameters name (PCM file is organized by sets of 12 parameters columns)
	QString	strUnits[12];		// Holds the 12 Parameters Units 
	int		iWaferID;			// WaferID processed
	int		iSiteID;			// SiteID processed
	int		iIndex;				// Loop index
	int		iTotalParam;	// Parameter results on a line

	// Open WAT file
    QFile f( WatFileName );
    if(!f.open( QIODevice::ReadOnly ))
	{
		// Failed Opening WAT file
		m_iLastError = errOpenFail;
		
		// Convertion failed.
		return false;
	}
	// Assign file I/O stream
	QTextStream hWatFile(&f);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	iFileSize = f.size() + 1;

	// Loop reading file until end is reached
	strString = ReadLine(hWatFile);
	do
	{	
		// Check if first line is the correct WAT header...
		strString = strString.trimmed();	// remove leading spaces.
		if(!strString.startsWith("Lot Summary Report KDF"))
		{
			// Incorrect header...this is not a PCM file!
			m_iLastError = errInvalidFormat;
			
			// Convertion failed.
			// Close file
			f.close();
			return false;
		}

		// Skip empty line
		//hWatFile.readLine();

		// Read line : "Fab ID	: Gumi Fab3
		strString = ReadLine(hWatFile);
		strSection = strString.section(':',0,0); // Fab ID
		strSection = strSection.trimmed();	// remove leading spaces.
		if(strSection.startsWith("Fab ID") == false)
		{
			// Incorrect header...this is not a PCM file!
			m_iLastError = errInvalidFormat;
			
			// Convertion failed.
			// Close file
			f.close();
			return false;
		}
		m_strProcessID = strString.section(':',1); // Fab ID
		m_strProcessID = m_strProcessID.trimmed();	// remove leading spaces.

		// Read line : "Date	: 27-Jul-2005
		strString = ReadLine(hWatFile);
		strSection = strString.section(':',0,0); // Fab ID
		strSection = strSection.trimmed();	// remove leading spaces.
		if(strSection.startsWith("Date") == false)
		{
			// Incorrect header...this is not a PCM file!
			m_iLastError = errInvalidFormat;
			
			// Convertion failed.
			// Close file
			f.close();
			return false;
		}

		// Extract TimeStamp, convert to UINT (nbr of seconds since 1-1-1970)
		int iDay=1,iMonth=1,iYear=1970;
		strString = strString.section(':',1,1); // Date format: DD-Month-YYYY
		strSection = strString.section('-',0,0);	// First field is Day
		iDay = strSection.toInt();
		strSection = strString.section('-',1,1);	// Second field is Month
		if(strSection.startsWith("Jan", Qt::CaseInsensitive))
			iMonth=1;
		if(strSection.startsWith("Fev", Qt::CaseInsensitive))
			iMonth=2;
		if(strSection.startsWith("Mar", Qt::CaseInsensitive))
			iMonth=3;
		if(strSection.startsWith("Apr", Qt::CaseInsensitive))
			iMonth=4;
		if(strSection.startsWith("May", Qt::CaseInsensitive))
			iMonth=5;
		if(strSection.startsWith("Jun", Qt::CaseInsensitive))
			iMonth=6;
		if(strSection.startsWith("Jul", Qt::CaseInsensitive))
			iMonth=7;
		if(strSection.startsWith("Aug", Qt::CaseInsensitive))
			iMonth=8;
		if(strSection.startsWith("Sep", Qt::CaseInsensitive))
			iMonth=9;
		if(strSection.startsWith("Oct", Qt::CaseInsensitive))
			iMonth=10;
		if(strSection.startsWith("Nov", Qt::CaseInsensitive))
			iMonth=11;
		if(strSection.startsWith("Dec", Qt::CaseInsensitive))
			iMonth=12;

		strSection = strString.section('-',2,2);	// Last field is Year
		iYear = strSection.toInt();
		QDate WatDate(iYear,iMonth,iDay);

		// Read line : "Time	:  15:19:31
		strString = ReadLine(hWatFile);
		strSection = strString.section(':',0,0); // Time
		strSection = strSection.trimmed();	// remove leading spaces.
		if(strSection.startsWith("Time") == false)
		{
			// Incorrect header...this is not a PCM file!
			m_iLastError = errInvalidFormat;
			
			// Convertion failed.
			// Close file
			f.close();
			return false;
		}
		int iHour,iMin,iSec;
		strString = strString.section(':',1);		// Time format: HH:MM:SS
		strSection = strString.section(':',0,0);	// First field is HH
		iHour = strSection.toInt();
		strSection = strString.section(':',1,1);	// First field is MM
		iMin = strSection.toInt();
		strSection = strString.section(':',2,2);	// First field is SS
		iSec = strSection.toInt();

		QTime WatTime(iHour,iMin,iSec);
		QDateTime WateDateTime(WatDate,WatTime);
		WateDateTime.setTimeSpec(Qt::UTC);
		m_lStartTime = WateDateTime.toTime_t();

		// Read line : "Device	:  xxxxxx
		strString = ReadLine(hWatFile);
		strSection = strString.section(':',0,0);	// Product ID (Product)
		strSection = strSection.trimmed();	// remove leading spaces.
		if(strSection.startsWith("Device") == false)
		{
			// Incorrect header...this is not a PCM file!
			m_iLastError = errInvalidFormat;
			
			// Convertion failed.
			// Close file
			f.close();
			return false;
		}
		m_strProductID = strString.section(':',1);		// Product ID
		m_strProductID = m_strProductID.trimmed();	// remove leading spaces.

		// Read line : "Lot ID	:  xxxxxx
		strString = ReadLine(hWatFile);
		strSection = strString.section(':',0,0);	// Lot ID
		strSection = strSection.trimmed();	// remove leading spaces.
		if(strSection.startsWith("Lot ID") == false)
		{
			// Incorrect header...this is not a PCM file!
			m_iLastError = errInvalidFormat;
			
			// Convertion failed.
			// Close file
			f.close();
			return false;
		}
		m_strLotID = strString.section(':',1);		// Lot ID
		m_strLotID = m_strLotID.trimmed();	// remove leading spaces.


		// Read line : "Process	:  xxxxxx
		strString = ReadLine(hWatFile);
		strSection = strString.section(':',0,0);	// Process ID
		strSection = strSection.trimmed();	// remove leading spaces.
		if(strSection.startsWith("Process") == false)
		{
			// Incorrect header...this is not a PCM file!
			m_iLastError = errInvalidFormat;
			
			// Convertion failed.
			// Close file
			f.close();
			return false;
		}
		m_strComment = strString.section(':',1);		// Process info
		m_strComment = m_strComment.trimmed();	// remove leading spaces.
		
		// Skip \t line
		//ReadLine(hWatFile);

		// Read line with list of Parameters names (Maximum of 12 names per line)
		// eg: " 	Para	Vth_NC1		IDoff_NC1	IDsat_NC1	BVdss_NC1	Vth_NC2		IDoff_NC2	IDsat_NC2	Isub_NC2	BVdss_NC2	Vth_NC5		Vth_NC5B	IDsat_NC5"
		strString = ReadLine(hWatFile);
        strCells = strString.split('\t');
		iTotalParam = strCells.count();
		// If no parameter specified...ignore!
		if(iTotalParam <= 0)
		{
			// Incorrect header...this is not a valid PCM file!
			m_iLastError = errInvalidFormat;
			
			// Convertion failed.
			// Close file
			f.close();
			return false;
		}

		if(strCells[0].startsWith("Para") == false)
		{
			// Incorrect header...this is not a PCM file!
			m_iLastError = errInvalidFormat;
			
			// Convertion failed.
			// Close file
			f.close();
			return false;
		}

		// Extract the 12 column names (or less if not all are filled)
		for(iIndex=0;iIndex<12;iIndex++)
		{
			strSection = "";
			if(strCells.count() > 1+iIndex)
				strSection = strCells[1+iIndex];
			strSection = strSection.trimmed();	// Remove spaces
			strParameters[iIndex] = strSection;
		}
		
		// Read Parameters Spec LOW limits.
		strString = ReadLine(hWatFile);
        strLowLimitsCells = strString.split('\t');
		if(strLowLimitsCells[0].startsWith("Min") == false)
		{
			// Incorrect header...this is not a PCM file!
			m_iLastError = errInvalidFormat;
			
			// Convertion failed.
			// Close file
			f.close();
			return false;
		}

		// Skip line hoding 'typical value' result
		strString = ReadLine(hWatFile);

		// Read Parameters Spec HIGH limits.
		strString = ReadLine(hWatFile);
        strHighLimitsCells = strString.split('\t');
		if(strHighLimitsCells[0].startsWith("Max") == false)
		{
			// Incorrect header...this is not a PCM file!
			m_iLastError = errInvalidFormat;
			
			// Convertion failed.
			// Close file
			f.close();
			return false;
		}

		// Read Parameters Units
		strString = ReadLine(hWatFile);
		strString = strString.replace("\t\t","\t.");	// For tests with no units, inesrt a '.' character so to avoid mis-alignement in parsing units & results.
        strCells = strString.split('\t');
		if(strCells[0].startsWith("Unit") == false)
		{
			// Incorrect header...this is not a PCM file!
			m_iLastError = errInvalidFormat;
			
			// Convertion failed.
			// Close file
			f.close();
			return false;
		}
		for(iIndex=0;iIndex<12;iIndex++)
		{
			strSection = "";
			if(strCells.count() > 1+iIndex)
				strSection = strCells[1+iIndex];
			strSection = strSection.trimmed();	// Remove spaces
			strUnits[iIndex] = strSection;
			// If unit doesn't start with a '.', force one. This avoids conflicting units with prefix. e.g: pA becomes .pA
			if(strSection[0] != '.')
				strUnits[iIndex] += ".";
		}

		// Skip line: 'Site	Wafer'
		strString = ReadLine(hWatFile);

		// Read 1st test result line
		strString = ReadLine(hWatFile);

		// Read test results until end of data block reached (ends with statistics: Average and StdDev)
        while(strString.indexOf("Average") < 0)
		{
			// Read line of Parameter data unless end of data reached (reaching Param. Average and StdDev)
			// eg: "B	4	0.422		2.637E+07	628.110		0.000		0.576		1.100E+04	555.330		5.202		0.000		0.730		1.318		23.451"
	
			// Split line into individual cells
            strCells = strString.split('\t');

			// Extract Site#
			if(strCells[0].startsWith("C", Qt::CaseInsensitive))
				iSiteID = 1;		// Center
			else
			if(strCells[0].startsWith("B", Qt::CaseInsensitive))
				iSiteID = 2;		// Bottom
			else
			if(strCells[0].startsWith("L", Qt::CaseInsensitive))
				iSiteID = 3;		// Left
			else
			if(strCells[0].startsWith("T", Qt::CaseInsensitive))
				iSiteID = 4;		// Top
			else
				iSiteID = 5;		// Right, or other position.

			// Extract WaferID, and save it to the parameter list as a parameter itself!
			strSection = "";
			if(strCells.count() > 1)
				strSection = strCells[1];
			iWaferID = strSection.toInt();

			SaveParameterResult(iWaferID,iSiteID,"WAF ID","",strSection);

			// For each column, extract parameter value and save it
			for(iIndex=0;iIndex<12;iIndex++)
			{
				strSection = "";
				if(strCells.count() > 2+iIndex)
					strSection = strCells[2+iIndex];
				// Save parameter result in buffer
				if(strSection.isEmpty() != true)
					SaveParameterResult(iWaferID,iSiteID,strParameters[iIndex],strUnits[iIndex],strSection);
			}

			// Check if last line processed (should normally never happen as statistcs should follow...)
			if(hWatFile.atEnd())
				break;

			// Read next line in file...
			strString = ReadLine(hWatFile);
		};

		// Have reached the statistics section:
		// Average	0.515		1.039E+07	528.618		1.208		0.632		2.364E+04	517.033		4.130		6.292		-6.164		-142.907	37.197
		// Stdev	0.090		1.741E+07	161.120		2.793		0.075		9.882E+04	28.077		0.743		4.871		37.673		472.537		41.974

		// Should point to next bloc of parameters..unless end of file!
		do
		{
			strString = ReadLine(hWatFile);
		}
        while((strString.indexOf("Lot Summary Report KDF") < 0) && (hWatFile.atEnd() == false));

		// Save static parameter info: Low limit, High limit, Units.
		for(iIndex=0;iIndex<12;iIndex++)
		{

			strSection = "";
			if(strLowLimitsCells.count() > 1+iIndex)
				strSection = strLowLimitsCells[1+iIndex];
			if(strSection.isEmpty() != true)
			{
				// Remove spaces
				strSection = strSection.trimmed();
				// Save parameter result in buffer
				SaveParameterLimit(strParameters[iIndex],strSection,eLowLimit);

				strSection = "";
				if(strHighLimitsCells.count() > 1+iIndex)
					strSection = strHighLimitsCells[1+iIndex];
				// Remove spaces
				strSection = strSection.trimmed();
				// Save parameter result in buffer
				SaveParameterLimit(strParameters[iIndex],strSection,eHighLimit);
			}
		}
	}
	while (hWatFile.atEnd() == false);

	// Close file
	f.close();

	// All WAT file read...check if need to update the WAT Parameter list on disk?
	if(m_bNewWatParameterFound == true)
		DumpParameterIndexTable();

	// Success parsing WAT file
	return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from WAT data parsed
//////////////////////////////////////////////////////////////////////
bool CGPCM_KDFtoSTDF::WriteStdfFile(const char *strFileNameSTDF)
{
	// now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open((char*)strFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
	{
		// Failed importing WAT file into STDF database
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
	StdfFile.WriteString("");					// Node name
	StdfFile.WriteString("");					// Tester Type
	StdfFile.WriteString("");					// Job name
	StdfFile.WriteString("");					// Job rev
	StdfFile.WriteString("");					// sublot-id
	StdfFile.WriteString("");					// operator
	StdfFile.WriteString("");					// exec-type
	StdfFile.WriteString("");					// exe-ver
	StdfFile.WriteString("WAFER");				// test-cod
	StdfFile.WriteString("");					// test-temperature
	// Construct custom Galaxy USER_TXT 
	QString	strUserTxt;
	strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;	
	strUserTxt += ":";
	strUserTxt += GEX_IMPORT_DATAORIGIN_ETEST;
	strUserTxt += ":HYNIX MAGNACHIP";
	StdfFile.WriteString((char *)strUserTxt.toLatin1().constData());	// user-txt
	StdfFile.WriteString("");							// aux-file
	StdfFile.WriteString("");							// package-type
	StdfFile.WriteString(m_strProductID.toLatin1().constData());	// familyID
	StdfFile.WriteString("");							// Date-code
	StdfFile.WriteString("");							// Facility-ID
	StdfFile.WriteString("");							// FloorID
	StdfFile.WriteString(m_strProcessID.toLatin1().constData());	// ProcessID

	StdfFile.WriteRecord();

	// Write Test results for each waferID
	CGWatWafer	*ptWafers;		// List of Wafers in WAT file
	char		szString[257];
	BYTE		iSiteNumber,bData;
	WORD		wSoftBin,wHardBin;
	long		iTotalGoodBin,iTotalFailBin;
	long		iTestNumber,iPartNumber=0;
	bool		bPassStatus;
	CGWatParameter *ptParam;

	QListIterator<CGWatWafer*> lstIteratorWafer(m_pWaferList);
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
		sprintf(szString,"%d",ptWafers->iWaferID);
		StdfFile.WriteString(szString);				// WaferID
		StdfFile.WriteRecord();

		// Write all Parameters read on this wafer.: PTR....PTR, PRR
		iTotalGoodBin=iTotalFailBin=0;

		// Write PTRs for EACH of the X sites
		for(iSiteNumber = ptWafers->iLowestSiteID; iSiteNumber <= ptWafers->iHighestSiteID; iSiteNumber++)
		{
			// Write PIR for parts in this Wafer site
			RecordReadInfo.iRecordType = 5;
			RecordReadInfo.iRecordSubType = 10;
			StdfFile.WriteHeader(&RecordReadInfo);
			StdfFile.WriteByte(1);								// Test head
			StdfFile.WriteByte(iSiteNumber);					// Tester site
			StdfFile.WriteRecord();

			// Part number
			iPartNumber++;

			QList<CGWatParameter*>::iterator itParam = ptWafers->pParameterList.begin();
			
			bPassStatus = true;
			while(itParam != ptWafers->pParameterList.end())
			{
				ptParam = (*itParam);
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

				// Write the PTR if it exists for this site...
				if(ptParam->fValue.contains (iSiteNumber) == true)
				{
					RecordReadInfo.iRecordType = 15;
					RecordReadInfo.iRecordSubType = 10;
					StdfFile.WriteHeader(&RecordReadInfo);
					// Compute Test# (add user-defined offset)
                    iTestNumber = (long) m_pFullWatParametersList.indexOf(ptParam->strName);
					iTestNumber += GEX_TESTNBR_OFFSET_WAT;		// Test# offset
					StdfFile.WriteDword(iTestNumber);			// Test Number
					StdfFile.WriteByte(1);						// Test head
					StdfFile.WriteByte(iSiteNumber);			// Tester site:1,2,3,4 or 5, etc.
					if((ptParam->fValue[iSiteNumber] < ptParam->fLowLimit) || (ptParam->fValue[iSiteNumber] > ptParam->fHighLimit))
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
					StdfFile.WriteFloat(ptParam->fValue[iSiteNumber]);	// Test result
					if(ptParam->bStaticHeaderWritten == false)
					{
						StdfFile.WriteString(ptParam->strName.toLatin1().constData());	// TEST_TXT
						StdfFile.WriteString("");							// ALARM_ID

						bData = 2;	// Valid data.
						if(ptParam->bValidLowLimit==false)
							bData |=0x40;
						if(ptParam->bValidHighLimit==false)
							bData |=0x80;
						StdfFile.WriteByte(bData);							// OPT_FLAG

						StdfFile.WriteByte(0);								// RES_SCALE
						StdfFile.WriteByte(0);								// LLM_SCALE
						StdfFile.WriteByte(0);								// HLM_SCALE
						StdfFile.WriteFloat(ptParam->fLowLimit);			// LOW Limit
						StdfFile.WriteFloat(ptParam->fHighLimit);			// HIGH Limit
						StdfFile.WriteString(ptParam->strUnits.toLatin1().constData());	// Units
						ptParam->bStaticHeaderWritten = true;
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
			StdfFile.WriteWord((WORD)ptWafers->pParameterList.count());		// NUM_TEST
			StdfFile.WriteWord(wHardBin);            // HARD_BIN
			StdfFile.WriteWord(wSoftBin);            // SOFT_BIN
			// 200mm wafers, usually have 5 sites, 300mm wafers usually have 9 sites
			switch(iSiteNumber)
			{
				case 1:	// Center
					StdfFile.WriteWord(1);			// X_COORD
					StdfFile.WriteWord(1);			// Y_COORD
					break;
				case 2:	// Down
					StdfFile.WriteWord(1);			// X_COORD
					StdfFile.WriteWord(2);			// Y_COORD
					break;
				case 3:	// Left
					StdfFile.WriteWord(0);			// X_COORD
					StdfFile.WriteWord(1);			// Y_COORD
					break;
				case 4:	// Top
					StdfFile.WriteWord(1);			// X_COORD
					StdfFile.WriteWord(0);			// Y_COORD
					break;
				case 5:	// Right
					StdfFile.WriteWord(2);			// X_COORD
					StdfFile.WriteWord(1);			// Y_COORD
					break;
				case 6:	// Lower-Right corner
					StdfFile.WriteWord(2);			// X_COORD
					StdfFile.WriteWord(2);			// Y_COORD
					break;
				case 7:	// Lower-Left corner
					StdfFile.WriteWord(0);			// X_COORD
					StdfFile.WriteWord(2);			// Y_COORD
					break;
				case 8:	// Upper-Left corner
					StdfFile.WriteWord(0);			// X_COORD
					StdfFile.WriteWord(0);			// Y_COORD
					break;
				case 9:	// Upper-Right corner
					StdfFile.WriteWord(2);			// X_COORD
					StdfFile.WriteWord(0);			// Y_COORD
					break;
				default: // More than 9 sites?....give 0,0 coordonates
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
		StdfFile.WriteByte(255);						// Tester site (all)
		StdfFile.WriteDword(m_lStartTime);			// Time of last part tested
		StdfFile.WriteDword(iTotalGoodBin+iTotalFailBin);	// Parts tested: always 5
		StdfFile.WriteDword(0);						// Parts retested
		StdfFile.WriteDword(0);						// Parts Aborted
		StdfFile.WriteDword(iTotalGoodBin);			// Good Parts
		StdfFile.WriteDword((DWORD)-1)	;			// Functionnal Parts

		sprintf(szString,"%d",ptWafers->iWaferID);
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
// Convert 'FileName' WAT file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGPCM_KDFtoSTDF::Convert(const char *WatFileName, const char *strFileNameSTDF)
{
	// No erro (default)
	m_iLastError = errNoError;

	// If STDF file already exists...do not rebuild it...unless dates not matching!
	QFileInfo fInput(WatFileName);
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
			GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(WatFileName).fileName()+"...");
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
	
    if(ReadWatFile(WatFileName) != true)
	{
		//////////////////////////////////////////////////////////////////////
		// For ProgressBar
		if((GexProgressBar != NULL)
		&& bHideProgressAfter)
			GexProgressBar->hide();
		
		if((GexScriptStatusLabel != NULL)
		&& bHideLabelAfter)
			GexScriptStatusLabel->hide();
		return false;	// Error reading WAT file
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
QString CGPCM_KDFtoSTDF::ReadLine(QTextStream& hFile)
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
	{
		strString = hFile.readLine();
		if(!strString.isNull() && QString(strString).remove('\t').isEmpty())
			strString = "";
	}
	while(!strString.isNull() && strString.isEmpty());

	return strString;

}
