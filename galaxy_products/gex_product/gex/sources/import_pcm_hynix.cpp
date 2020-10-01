//////////////////////////////////////////////////////////////////////
// import_pcm_hynix.cpp: Convert a .PCM MagnaChip / HYNIX Foundry file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#include "windows.h"		// For 'GetWindowsDirectoryA' declaration
#endif

#include <qfileinfo.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>

#include "engine.h"
#include "import_pcm_hynix.h"
#include "time.h"
#include "import_constants.h"

// File format:
//                                                  W.A.T. DATA ATTACHED
//   HYNIX  CHUNG-JU                                     PCM  DATA  SUMMARY                                       DATE : 04-07-30
//                                                      ======================                                     TIME : 21:12:41
//  DEVICE     : AATI12                LOT ID     : NS65643            DATE       : 29-JUN-04
//----------------------------------------------------------------------------------------------------------------------------------
//
// ID    TOT   PAS ! VTN-50/50  VTN-50/.60 IDN-50/.60 BVN-50/.60 VTP-50/50  VTP-50/.60 IDP-50/.60 BVP-50/.60 N+ RS      P+ RS     
// --------------- ! V          V          AMP        V          V          V          AMP        V          12*120/OHM 12*120/OHM
//                 !
//   1     5     5 !   0.754      0.719      0.213E-01   12.0      -.926      -.900      -.997E-02  -10.3       81.1       118.    
//   2     5     5 !   0.761      0.734      0.210E-01   12.0      -.950      -.927      -.966E-02  -10.3       81.0       118.    
//   3     5     5 !   0.756      0.734      0.208E-01   12.0      -.945      -.922      -.958E-02  -10.3       81.2       118.    
//...
//                 !
// LOT ID     AV       0.755      0.729      0.211E-01   12.0      -.944      -.916      -.971E-02  -10.3       82.4       120.    
// LOT ID     SD       0.914E-02  0.931E-02  0.254E-03  0.133      0.130E-01  0.182E-01  0.220E-03  0.122      0.996       2.27    
// SPEC  HIGH          0.850      0.850      0.260E-01   15.0     -0.850     -0.800     -0.850E-02  -8.00       95.0       135.    
// SPEC  LOW           0.650      0.600      0.170E-01   8.00      -1.05      -1.05     -0.145E-01  -15.0       55.0       95.0    
// ----------------------------------------------------------------------------------------------------------------------------------
// ....

// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar

#define	PCM_HYNIX_TOTAL_SITES			"Total_Sites_Per_Wafer"
#define	PCM_HYNIX_TOTAL_PASSING_SITES	"Total_Passing_Sites_Per_Wafer"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGPcmHynixWafer::~CGPcmHynixWafer()
{
	while (!pParameterList.isEmpty())
		delete pParameterList.takeFirst();
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGPCM_Hynix_toSTDF::CGPCM_Hynix_toSTDF()
{
	// Default: PCM parameter list on disk includes all known PCM parameters...
	m_bNewPcmParameterFound = false;
	m_lStartTime = 0;
}

CGPCM_Hynix_toSTDF::~CGPCM_Hynix_toSTDF()
{
	while (!m_pWaferList.isEmpty())
		delete m_pWaferList.takeFirst();
}
//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGPCM_Hynix_toSTDF::GetLastError()
{
	m_strLastError = "Import MagnaChip/Hynix PCM: ";

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
void CGPCM_Hynix_toSTDF::LoadParameterIndexTable(void)
{
	QString	strPcmTableFile;
	QString	strString;


    strPcmTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strPcmTableFile += GEX_PCM_PARAMETERS;

	// Open PCM Parameter table file
    QFile f( strPcmTableFile );
    if(!f.open( QIODevice::ReadOnly ))
		return;

	// Assign file I/O stream
	QTextStream hPcmTableFile(&f);

	// Skip comment lines
	do
	{
	  strString = hPcmTableFile.readLine();
	}
    while((strString.indexOf("----------------------") < 0) && (hPcmTableFile.atEnd() == false));

	// Read lines
	m_pFullPcmParametersList.clear();
	strString = hPcmTableFile.readLine();
	while (strString.isNull() == false)
	{
		// Save Parameter name in list
		m_pFullPcmParametersList.append(strString);
		// Read next line
		strString = hPcmTableFile.readLine();
	};

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// Save PCM Parameter table to DISK
//////////////////////////////////////////////////////////////////////
void CGPCM_Hynix_toSTDF::DumpParameterIndexTable(void)
{
	QString		strPcmTableFile;

    strPcmTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strPcmTableFile += GEX_PCM_PARAMETERS;

	// Open PCM Parameter table file
    QFile f( strPcmTableFile );
    if(!f.open( QIODevice::WriteOnly ))
		return;

	// Assign file I/O stream
	QTextStream hPcmTableFile(&f);

	// First few lines are comments:
	hPcmTableFile << "############################################################" << endl;
	hPcmTableFile << "# DO NOT EDIT THIS FILE!" << endl;
    hPcmTableFile << "# Quantix Examinator: PCM Parameters detected" << endl;
	hPcmTableFile << "# www.mentor.com" << endl;
    hPcmTableFile << "# Quantix Examinator reads and writes into this file..." << endl;
	hPcmTableFile << "-----------------------------------------------------------" << endl;

	// Write lines
	// m_pFullPcmParametersList.sort();
	//for(uIndex=0;uIndex<m_pFullPcmParametersList.count();uIndex++)
	for (QStringList::const_iterator
		 iter  = m_pFullPcmParametersList.begin();
		 iter != m_pFullPcmParametersList.end(); ++iter) {
		// Write line
		hPcmTableFile << *iter << endl;
	}

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// If Examinator doesn't have this PCM parameter in his dictionnary, have it added.
//////////////////////////////////////////////////////////////////////
void CGPCM_Hynix_toSTDF::UpdateParameterIndexTable(QString strParamName)
{
	// Check if the table is empty...if so, load it from disk first!
	if(m_pFullPcmParametersList.isEmpty() == true)
	{
		// Load PCM parameter table from disk...
		LoadParameterIndexTable();
	}
	
	// Check if Parameter name already in table...if not, add it to the list
	// the new full list will be dumped to the disk at the end.
    if(m_pFullPcmParametersList.indexOf(strParamName) < 0)
	{
		// Update list
		m_pFullPcmParametersList.append(strParamName);

		// Set flag to force the current PCM table to be updated on disk
		m_bNewPcmParameterFound = true;
	}
}

//////////////////////////////////////////////////////////////////////
// Update Wafer info (# od sites tested & # good parts)
//////////////////////////////////////////////////////////////////////
void CGPCM_Hynix_toSTDF::UpdateWaferInfo(int iWaferID,int iTotalSites,int iTotalGood)
{
	QList<CGPcmHynixWafer*>::iterator itWafer = m_pWaferList.begin();
	
	while(itWafer != m_pWaferList.end())
	{
		// Find relevant Wafer and save data.
		if((*itWafer)->iWaferID == iWaferID)
		{
			(*itWafer)->iNbParts	= iTotalSites;	// Nb parts for this wafer
			(*itWafer)->iNbGood		= iTotalGood;		// Nb good parts for this wafer
			return;
		}
		
		// Not the WaferID we need...see next one.
		itWafer++;
	}
}

//////////////////////////////////////////////////////////////////////
// Find or create Parameter entry, return handle to it.
//////////////////////////////////////////////////////////////////////
CGPcmHynixParameter *CGPCM_Hynix_toSTDF::FindParameterEntry(int iWaferID,int iSiteID,QString strParamName,QString strUnits)
{
	CGPcmHynixParameter *					ptParam = NULL;	// List of parameters
	CGPcmHynixWafer *						ptWafers = NULL;
	QList<CGPcmHynixWafer*>::iterator		itWafer = m_pWaferList.begin();
	QList<CGPcmHynixParameter*>::iterator	itParam;

	while(itWafer != m_pWaferList.end())
	{
		ptWafers = (*itWafer);

		if(ptWafers->iWaferID == iWaferID)
		{
			// Update Min/Max SiteID
			if(iSiteID > 0)
			{
				ptWafers->iLowestSiteID=gex_min(ptWafers->iLowestSiteID,iSiteID);				// Lowest WaferID found in PCM file
				ptWafers->iHighestSiteID=gex_max(ptWafers->iHighestSiteID,iSiteID);				// Highest WaferID found in PCM file
			}

			// Found WaferID entry...now find Parameter entry
			itParam = (*itWafer)->pParameterList.begin();
			while(itParam != ptWafers->pParameterList.end())
			{
				if((*itParam)->strName == strParamName)
					return (*itParam);	// Found the Parameter!

				// Not the Parameter we need...see next one.
				itParam++;
			};

			// Parameter not in list...create entry!
			// Create & add new Parameter entry in Wafer list
			ptParam = new CGPcmHynixParameter;
			ptParam->strName = strParamName;
			ptParam->strUnits = strUnits;
			ptParam->bHighLimit = ptParam->bLowLimit = false;
			ptParam->fHighLimit = ptParam->fLowLimit = 0;
			ptParam->bStaticHeaderWritten = false;
			ptWafers->pParameterList.append(ptParam);
			iTotalParameters++;
			// If Examinator doesn't have this PCM parameter in his dictionnary, have it added.
			UpdateParameterIndexTable(strParamName);
			
			// Return pointer to the Parameter 
			return ptParam;
		}

		// Not the WaferID we need...see next one.
		itWafer++;
	};

	// WaferID entry doesn't exist yet...so create it now!
	ptWafers = new CGPcmHynixWafer;
	ptWafers->iWaferID = iWaferID;

	// Update Min/Max SiteID
	if(iSiteID > 0)
	{
		ptWafers->iLowestSiteID=iSiteID;				// Lowest WaferID found in PCM file
		ptWafers->iHighestSiteID=iSiteID;				// Highest WaferID found in PCM file
	}

	// Create & add new Parameter entry in Wafer list
	ptParam = new CGPcmHynixParameter;
	ptParam->strName = strParamName;
	ptParam->strUnits = strUnits;
	ptParam->bHighLimit = ptParam->bLowLimit = false;
	ptParam->fHighLimit = ptParam->fLowLimit = 0;
	ptParam->bStaticHeaderWritten = false;
	ptWafers->pParameterList.append(ptParam);
	iTotalParameters++;
	
	// If Examinator doesn't have this PCM parameter in his dictionnary, have it added.
	UpdateParameterIndexTable(strParamName);
	
	// Add Wafer entry to existing list.
	m_pWaferList.append(ptWafers);

	// Return pointer to the Parameter 
	return ptParam;
}

//////////////////////////////////////////////////////////////////////
// Save PCM parameter result...
//////////////////////////////////////////////////////////////////////
void CGPCM_Hynix_toSTDF::SaveParameterResult(int iWaferID,int iSiteID,QString strName,QString strUnits,QString strValue)
{
	if(strName.isNull() == true)
		return;	// Ignore empty entry!

	// Find Pointer to the Parameter cell
	CGPcmHynixParameter *ptParam = FindParameterEntry(iWaferID,iSiteID,strName,strUnits);
	
	// Save the Test value in it
	ptParam->fValue[iSiteID] = strValue.toFloat();
}

//////////////////////////////////////////////////////////////////////
// Save PCM parameter High or Low Limit result...
//////////////////////////////////////////////////////////////////////
void CGPCM_Hynix_toSTDF::SaveParameterLimit(QString strName,QString strValue,int iLimit)
{
	if(strName.isNull() == true)
		return;	// Ignore empty entry!

	CGPcmHynixParameter *				ptParam	= NULL;					// List of parameters
	QList<CGPcmHynixWafer*>::iterator	itWafer = m_pWaferList.begin();

	while(itWafer != m_pWaferList.end())
	{
		// Get Parameter pointer...save limits in ALL WaferEntry.
		ptParam = FindParameterEntry((*itWafer)->iWaferID, 0, strName);
		switch(iLimit)
		{
			case eHighLimit:
				ptParam->bHighLimit = true;
				ptParam->fHighLimit = strValue.toFloat();
				break;
			case eLowLimit:
				ptParam->bLowLimit = true;
				ptParam->fLowLimit = strValue.toFloat();
				break;
		}

		// Move to next wafer entry
		itWafer++;
	};
}



//////////////////////////////////////////////////////////////////////
// Check if File is compatible with PCM_Hynix format
//////////////////////////////////////////////////////////////////////
bool CGPCM_Hynix_toSTDF::IsCompatible(const char *szFileName)
{
	QString strString;
	QString strSection;
	QString strValue;
	QString	strSite;
	int		iIndex;

	// Open hCsmFile file
    QFile f( szFileName );
    if(!f.open( QIODevice::ReadOnly ))
	{
		// Failed Opening ASL1000 file
		return false;
	}
	// Assign file I/O stream
	QTextStream hPcmFile(&f);

	// Check if first line is the correct PCM header...
	do
		strString = hPcmFile.readLine();
	while(!strString.isNull() && strString.isEmpty());

	f.close();

    iIndex = strString.indexOf("PCM  DATA  SUMMARY");
	if(iIndex < 0)
	{
		// Incorrect header...this is not a HYNIX PCM file!
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the PCM file
//////////////////////////////////////////////////////////////////////
bool CGPCM_Hynix_toSTDF::ReadPcmFile(const char *PcmFileName)
{
	QString strString;
	QString strSection;
	QString	strParameters[10];	// Holds the 10 Parameters name (PCM file is organized by sets of 10 parameters columns)
	QString	strUnits[10];		// Holds the 10 Parameters Units 
    QString	strTotalSites;
	QString	strPassingSites;
	int		iWaferID;			// WaferID processed
	int		iSiteID;			// SiteID processed
	int		iIndex;				
	int		iLoopIndex;			// Loop index

	// Open PCM file
    QFile f( PcmFileName );
    if(!f.open( QIODevice::ReadOnly ))
	{
		// Failed Opening PCM file
		m_iLastError = errOpenFail;
		
		// Convertion failed.
		return false;
	}
	// Assign file I/O stream
	QTextStream hPcmFile(&f);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	iFileSize = f.size() + 1;
	iTotalParameters = 0;

	// Check if first line is the correct PCM header...
	strString = ReadLine(hPcmFile);
    iIndex = strString.indexOf("PCM  DATA  SUMMARY");
	if(iIndex < 0)
	{
		// Incorrect header...this is not a HYNIX PCM file!
		m_iLastError = errInvalidFormat;
		
		// Convertion failed.
		// Close file
		f.close();
		return false;
	}

	// Extract Date & Time fields.
	QString	strDate,strTime;
	int iDay,iMonth,iYear; 
	int	iHours,iMinutes,iSeconds;
    iIndex = strString.indexOf("DATE :");
	if(iIndex < 0)
	{
		// Incorrect header...this is not a PCM file!
		m_iLastError = errInvalidFormat;
		
		// Convertion failed.
		// Close file
		f.close();
		return false;
	}

	// Offset to Data data
	iIndex += 6;
	strDate = strString.mid(iIndex); // Date format: YY-MM-DD

	// Read line#2:  ======================   TIME : 21:12:41
	strString = ReadLine(hPcmFile);
    iIndex = strString.indexOf("TIME :");
	if(iIndex < 0)
	{
		// Incorrect header...this is not a PCM file!
		m_iLastError = errInvalidFormat;
		
		// Convertion failed.
		// Close file
		f.close();
		return false;
	}
	// Offset to Time data
	iIndex += 6;
	strTime = strString.mid(iIndex); // Time format: HH:MM:SS


	// Extract TimeStamp, convert to UINT (nbr of seconds since 1-1-1970)
	strSection = strDate.section('-',0,0);	// First field is Year since 2000
	iYear = strSection.toInt() + 2000;
	strSection = strDate.section('-',1,1);	// Second field is Month
	iMonth = strSection.toInt();
	strSection = strDate.section('-',2,2);	// Third field is Day
	iDay = strSection.toInt();

	strSection = strTime.section(':',0,0);	// First field is Hours
	iHours = strSection.toInt();
	strSection = strTime.section(':',1,1);	// Second field is Minutes
	iMinutes = strSection.toInt();
	strSection = strTime.section(':',2,2);	// Third field is Seconds
	iSeconds = strSection.toInt();

	QDate PcmDate(iYear,iMonth,iDay);
	QTime PcmTime(iHours,iMinutes,iSeconds);
	QDateTime PcmeDateTime(PcmDate);
	PcmeDateTime.setTime(PcmTime);
	PcmeDateTime.setTimeSpec(Qt::UTC);
	m_lStartTime = PcmeDateTime.toTime_t();

	// Read line#3: DEVICE     : AATI12                LOT ID     : NS65643            DATE       : 29-JUN-04
	strString = ReadLine(hPcmFile);

	// Extract ProductID
    iIndex = strString.indexOf("DEVICE     :");
	if(iIndex < 0)
	{
		// Incorrect header...this is not a PCM file!
		m_iLastError = errInvalidFormat;
		
		// Convertion failed.
		// Close file
		f.close();
		return false;
	}
	// Offset to Product ID
	iIndex += 12;
	m_strProductID = strString.mid(iIndex,23);
	m_strProductID = m_strProductID.trimmed();	// remove leading spaces.

	// Extract Lot ID
    iIndex = strString.indexOf("LOT ID     :");
	if(iIndex < 0)
	{
		// Incorrect header...this is not a PCM file!
		m_iLastError = errInvalidFormat;
		
		// Convertion failed.
		// Close file
		f.close();
		return false;
	}
	// Offset to Lot ID
	iIndex += 12;
	m_strLotID = strString.mid(iIndex,18);
	m_strLotID = m_strLotID.trimmed();	// remove leading spaces.

	// Should point to next bloc of parameters..unless end of file!
	do
	{
	  strString = ReadLine(hPcmFile);
	}
    while((strString.indexOf("ID    TOT   PAS") < 0) && (hPcmFile.atEnd() == false));

	// If end of file: error
	if(hPcmFile.atEnd())
	{
		// Incorrect header...this is not a PCM file!
		m_iLastError = errInvalidFormat;
		
		// Convertion failed.
		// Close file
		f.close();
		return false;
	}

	// Pointing line with list of Parameters names
	// eg: "  ID    TOT   PAS ! VTN-50/50  VTN-50/.60 IDN-50/.60 BVN-50/.60 VTP-50/50  VTP-50/.60 IDP-50/.60 BVP-50/.60 N+ RS      P+ RS     

	// Loop reading file until end is reached
	int iHighRawIndex;
	do
	{	
		// Reset raw tracking counter
		iHighRawIndex=0;

		// Extract the 10 column names (or less if not all are filled)
		for(iLoopIndex=0;iLoopIndex<10;iLoopIndex++)
		{
			strSection = strString.mid(18+iLoopIndex*11,11);	// 1st half of the name
			strSection = strSection.trimmed();	// Remove spaces
			strParameters[iLoopIndex] = strSection + " ";
		}
		
		// Extract the 10 column ( second half of the name / or could be used as units too...)
		strString = ReadLine(hPcmFile);
		for(iLoopIndex=0;iLoopIndex<10;iLoopIndex++)
		{
			strSection = strString.mid(18+iLoopIndex*11,11);
			strSection = strSection.trimmed();	// Remove spaces
			strParameters[iLoopIndex] += strSection;
			strParameters[iLoopIndex] = strParameters[iLoopIndex].trimmed();	// Remove spaces

			// Keep track of total valid raws (can be upto 10)
			if(strParameters[iLoopIndex].isEmpty() != true)
				iHighRawIndex = iLoopIndex;
		}
		strString = ReadLine(hPcmFile);
        while((strString.indexOf("LOT ID     AV") < 0) && (hPcmFile.atEnd() == false))
		{
			// Read line of Parameter data unless end of data reached (reaching Param. Mean, Sigma and limits)
			// eg: "   4     5     5 !   0.759      0.732      0.211E-01   12.0      -.942      -.914      -.974E-02  -10.3       81.5       119.    
			// or may be end of data, in which case the line is of statitcis starts with 'LOT ID     AV'
	
			// Extract WaferID
			strSection = strString.mid(0,4);
			iWaferID = strSection.toInt();

			// Extract Total sites
			strTotalSites = strString.mid(5,6);
			strTotalSites = strTotalSites.trimmed();
			iSiteID = 1;	// As it is a summary PCM, we do not have each individual site, but all merged.

			// Extract Total Passing sites
			strPassingSites = strString.mid(10,6);
			strPassingSites = strPassingSites.trimmed();

			// If valid data line...
			if((iWaferID > 0) && (strTotalSites.length()>0) && (strPassingSites.length() > 0))
			{
				// For each column, extract parameter value and save it
				for(iLoopIndex=0;iLoopIndex<=iHighRawIndex;iLoopIndex++)
				{
					strSection = strString.mid(20+iLoopIndex*11,11);
					if(strSection.isNull() != true)
					{
						// Remove spaces
						strSection = strSection.trimmed();
						// Save parameter result in buffer
						SaveParameterResult(iWaferID,iSiteID,strParameters[iLoopIndex],strUnits[iLoopIndex],strSection);
					}
				}

				// Update Wafer details: total sites tested, total good
				UpdateWaferInfo(iWaferID,strTotalSites.toInt(),strPassingSites.toInt());
			}

			// Read next line in file...
			strString = ReadLine(hPcmFile);
		};

		// Have reached the statistics section:
		// LOT ID     AV       0.755      0.729      0.211E-01   12.0      -.944      -.916      -.971E-02  -10.3       82.4       120.    
		// LOT ID     SD       0.914E-02  0.931E-02  0.254E-03  0.133      0.130E-01  0.182E-01  0.220E-03  0.122      0.996       2.27    
		// SPEC  HIGH          0.850      0.850      0.260E-01   15.0     -0.850     -0.800     -0.850E-02  -8.00       95.0       135.    
		// SPEC  LOW           0.650      0.600      0.170E-01   8.00      -1.05      -1.05     -0.145E-01  -15.0       55.0       95.0    
		// -------------------------------------------------------------------------------------------------------------------------------

		// OR

		// Have reached the statistics section:
		// LOT ID     AV       0.755      0.729      0.211E-01   12.0      -.944      -.916      -.971E-02  -10.3       82.4       120.    
		// LOT ID     SD       0.914E-02  0.931E-02  0.254E-03  0.133      0.130E-01  0.182E-01  0.220E-03  0.122      0.996       2.27    
		// -------------------------------------------------------------------------------------------------------------------------------

		// If end of file: error
		if(hPcmFile.atEnd())
		{
			// Incorrect header...this is not a PCM file!
			m_iLastError = errInvalidFormat;
			
			// Convertion failed.
			// Close file
			f.close();
			return false;
		}

        while((strString.indexOf("---------------------") < 0) && (hPcmFile.atEnd() == false))
		{
			strString = ReadLine(hPcmFile);
            if(strString.indexOf("LOT ID     AV") > 0)
				continue;
            if(strString.indexOf("LOT ID     SD") > 0)
				continue;

            if(strString.indexOf("SPEC  HIGH") > 0)
			{
				// Read Parameters Spec HIGH limits.
				for(iLoopIndex=0;iLoopIndex<10;iLoopIndex++)
				{
					strSection = strString.mid(20+iLoopIndex*11,11);
					if(strSection.isNull() != true)
					{
						// Remove spaces
						strSection = strSection.trimmed();
						// Save parameter result in buffer
						SaveParameterLimit(strParameters[iLoopIndex],strSection,eHighLimit);
					}
				}
			}

            if(strString.indexOf("SPEC  LOW") > 0)
			{
				// Read Parameters Spec LOW limits.
				strString = ReadLine(hPcmFile);
				for(iLoopIndex=0;iLoopIndex<10;iLoopIndex++)
				{
					strSection = strString.mid(20+iLoopIndex*11,11);
					if(strSection.isNull() != true)
					{
						// Remove spaces
						strSection = strSection.trimmed();
						// Save parameter result in buffer
						SaveParameterLimit(strParameters[iLoopIndex],strSection,eLowLimit);
					}
				}
			}
		}

		// Should point to next bloc of parameters..unless end of file!
		do
		{
		  strString = ReadLine(hPcmFile);
		}
        while((strString.indexOf("ID    TOT   PAS") < 0) && (hPcmFile.atEnd() == false));
	}
	while (hPcmFile.atEnd() == false);

	// Close file
	f.close();

	// All PCM file read...check if need to update the PCM Parameter list on disk?
	if(m_bNewPcmParameterFound == true)
		DumpParameterIndexTable();

	// Success parsing PCM file
	return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from PCM data parsed
//////////////////////////////////////////////////////////////////////
bool CGPCM_Hynix_toSTDF::WriteStdfFile(const char *strFileNameSTDF)
{
	// now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open((char*)strFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
	{
		// Failed importing PCM file into STDF database
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
	strUserTxt += ":HYNIX";
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
	char		szString[257];
	int			iSiteNumber;
	BYTE		bData;
	WORD		wSoftBin,wHardBin;
	long		iTotalGoodParts,iTotalParts;
	long		iTestNumber;
	bool		bPassStatus;
	
	iTotalGoodParts=iTotalParts=0;
	
	CGPcmHynixParameter *					ptParam		= NULL;
	CGPcmHynixWafer	*						ptWafers	= NULL;		// List of Wafers in PCM file
	QList<CGPcmHynixWafer*>::iterator		itWafer		= m_pWaferList.begin();
	QList<CGPcmHynixParameter*>::iterator	itParam;

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
		sprintf(szString,"%d",ptWafers->iWaferID);
		StdfFile.WriteString(szString);				// WaferID
		StdfFile.WriteRecord();

		// Write all Parameters read on this wafer.: PTR....PTR, PRR

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

			itParam = ptWafers->pParameterList.begin();	// First test in list
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
                    iTestNumber = (long) m_pFullPcmParametersList.indexOf(ptParam->strName);
					iTestNumber += GEX_TESTNBR_OFFSET_PCM;		// Test# offset
					StdfFile.WriteDword(iTestNumber);			// Test Number
					StdfFile.WriteByte(1);						// Test head
					StdfFile.WriteByte(iSiteNumber);			// Tester site:1,2,3,4 or 5, etc.
					if((ptParam->bLowLimit && (ptParam->fValue[iSiteNumber] < ptParam->fLowLimit))
					|| (ptParam->bHighLimit && (ptParam->fValue[iSiteNumber] > ptParam->fHighLimit)))
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
						if(!ptParam->bLowLimit)
							bData |=0x40;
						if(!ptParam->bHighLimit)
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

			 // Refresh Site number value to be accurate
			iSiteNumber = ptWafers->iHighestSiteID;

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
			}
			else
			{
				StdfFile.WriteByte(8);				// PART_FLG : FAILED
				wSoftBin = wHardBin = 0;
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
			StdfFile.WriteString("");			// PART_ID
			StdfFile.WriteString("");			// PART_TXT
			StdfFile.WriteString("");			// PART_FIX
			StdfFile.WriteRecord();
		}

			
		iTotalGoodParts += ptWafers->iNbGood;
		iTotalParts += ptWafers->iNbParts;
		// Write WRR
		RecordReadInfo.iRecordType = 2;
		RecordReadInfo.iRecordSubType = 20;
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(1);						// Test head
		StdfFile.WriteByte(255);						// Tester site (all)
		StdfFile.WriteDword(m_lStartTime);			// Time of last part tested
		StdfFile.WriteDword(ptWafers->iNbParts);	// Parts tested: always 5
		StdfFile.WriteDword(0);						// Parts retested
		StdfFile.WriteDword(0);						// Parts Aborted
		StdfFile.WriteDword(ptWafers->iNbGood);		// Good Parts
		StdfFile.WriteDword((DWORD)-1);				// Functionnal Parts

		sprintf(szString,"%d",ptWafers->iWaferID);
		StdfFile.WriteString(szString);				// WaferID
		StdfFile.WriteRecord();
		
		// Not the WaferID we need...see next one.
		itWafer++;
	};

	// Write SBR Bin0 (FAIL)
	RecordReadInfo.iRecordType = 1;
	RecordReadInfo.iRecordSubType = 50;
	StdfFile.WriteHeader(&RecordReadInfo);
	StdfFile.WriteByte(255);					// Test Head = ALL
	StdfFile.WriteByte(255);					// Test sites = ALL		
	StdfFile.WriteWord(0);						// SBIN = 0
	StdfFile.WriteDword(iTotalParts-iTotalGoodParts);	// Total Bins
	StdfFile.WriteByte('F');
	StdfFile.WriteRecord();

	// Write SBR Bin1 (PASS)
	RecordReadInfo.iRecordType = 1;
	RecordReadInfo.iRecordSubType = 50;
	StdfFile.WriteHeader(&RecordReadInfo);
	StdfFile.WriteByte(255);					// Test Head = ALL
	StdfFile.WriteByte(255);					// Test sites = ALL		
	StdfFile.WriteWord(1);						// SBIN = 1
	StdfFile.WriteDword(iTotalGoodParts);		// Total Bins
	StdfFile.WriteByte('P');
	StdfFile.WriteRecord();

	// Write HBR Bin0 (FAIL)
	RecordReadInfo.iRecordType = 1;
	RecordReadInfo.iRecordSubType = 40;
	StdfFile.WriteHeader(&RecordReadInfo);
	StdfFile.WriteByte(255);					// Test Head = ALL
	StdfFile.WriteByte(255);					// Test sites = ALL		
	StdfFile.WriteWord(0);						// HBIN = 0
	StdfFile.WriteDword(iTotalParts-iTotalGoodParts);	// Total Bins
	StdfFile.WriteByte('F');
	StdfFile.WriteRecord();

	// Write HBR Bin1 (PASS)
	RecordReadInfo.iRecordType = 1;
	RecordReadInfo.iRecordSubType = 40;
	StdfFile.WriteHeader(&RecordReadInfo);
	StdfFile.WriteByte(255);					// Test Head = ALL
	StdfFile.WriteByte(255);					// Test sites = ALL		
	StdfFile.WriteWord(1);						// HBIN = 1
	StdfFile.WriteDword(iTotalGoodParts);		// Total Bins
	StdfFile.WriteByte('P');
	StdfFile.WriteRecord();

	// Write MRR
	RecordReadInfo.iRecordType = 1; 
	RecordReadInfo.iRecordSubType = 20;
	StdfFile.WriteHeader(&RecordReadInfo);
	StdfFile.WriteDword(m_lStartTime);			// finish-time=start time as no other info in file!
	StdfFile.WriteRecord();

	// Close STDF file.
	StdfFile.Close();

	// Success
	return true;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' PCM file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGPCM_Hynix_toSTDF::Convert(const char *PcmFileName, const char *strFileNameSTDF)
{
	// No erro (default)
	m_iLastError = errNoError;

	// If STDF file already exists...do not rebuild it...unless dates not matching!
	QFileInfo fInput(PcmFileName);
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
			GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(PcmFileName).fileName()+"...");
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
	
    if(ReadPcmFile(PcmFileName) != true)
	{
		//////////////////////////////////////////////////////////////////////
		// For ProgressBar
		if((GexProgressBar != NULL)
		&& bHideProgressAfter)
			GexProgressBar->hide();
		
		if((GexScriptStatusLabel != NULL)
		&& bHideLabelAfter)
			GexScriptStatusLabel->hide();
		return false;	// Error reading PCM file
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
QString CGPCM_Hynix_toSTDF::ReadLine(QTextStream& hFile)
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
