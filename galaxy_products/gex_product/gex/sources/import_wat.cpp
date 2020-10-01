//////////////////////////////////////////////////////////////////////
// import_wat.cpp: Convert a .WAT (TSMC) file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#include "windows.h"		// For 'GetWindowsDirectoryA' declaration
#endif

#include <qfileinfo.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>

#include "engine.h"
#include "import_wat.h"
#include "time.h"
#include "import_constants.h"

// File format:
//                                                  W.A.T. DATA ATTACHED
// TYPE NO :TME391                        PROCESS  :018TW30LF1                    PCM SPEC:T.B.D         QTY: 1 pcs
// LOT ID  :D62748.05                     DATE     :10/01/2001
// WAF SITE    VT_N4 N1u   Isat_N4     BV_N4       VT_P4 N1u   Isat_P4     BV_P4       VT_N43 N1u  Isat_N43    BV_N43      VT_P43 N1u
// ID  ID      V 10/.18    mA 10/.18   V 10/.18    V 10/.18    mA 10/.18   V 10/.18    V 10/.35    mA 10/.35   V 10/.35    V 10/.3
// 7  -1        0.421       6.512       4.047      -0.469      -3.066      -5.168       0.678       6.663       7.000      -0.667
// 7  -2        0.425       6.497       4.020      -0.474      -3.023      -5.195       0.681       6.588       7.000      -0.673
// 7  -3        0.424       6.477       4.047      -0.479      -2.984      -5.168       0.671       6.686       7.000      -0.668
// 7  -4        0.426       6.478       4.020      -0.459      -3.141      -5.250       0.669       6.595       7.000      -0.665
// 7  -5        0.427       6.420       4.047      -0.471      -3.027      -5.250       0.669       6.560       7.000      -0.667
// .....
// -------------------------------------------------------------------------------------------------------------------------------
// AVERAGE      0.424       6.477       4.036      -0.470      -3.048      -5.206       0.674       6.618       7.000      -0.668
// STD DEV     2.347E-03    0.035       0.015      7.493E-03    0.059       0.041      5.560E-03    0.053       0.000      3.029E-03
// SPEC HI      0.520       6.900       12.000     -0.400      -2.210      -3.600       0.820       6.900       20.000     -0.640
// SPEC LO      0.320       5.100       3.600      -0.600      -2.990      -12.000      0.620       5.100       6.000      -0.840
// -------------------------------------------------------------------------------------------------------------------------------
// WAF SITE    Isat_P43    BV_P43      VTFpo_N+    VTFpo_P+    Rs_NW(STI)  Rs_N+       Rs_P+       Rs_N+Po     Rs_P+Po     Rc_N+
// ID  ID      mA 10/.3    V 10/.3     V .28/128   V .28/128   Ohm/Sq 20   Ohm/Sq .22  Ohm/Sq .22  Ohm/Sq .18  Ohm/Sq .18  Ohm/Se .22
// ....

// main.cpp
extern void				WriteDebugMessageFile(const QString & strMessage);
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGWatWafer::~CGWatWafer()
{
	while (!pParameterList.isEmpty())
		delete pParameterList.takeFirst();
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGWATtoSTDF::CGWATtoSTDF()
{
	// Default: WAT parameter list on disk includes all known WAT parameters...
	m_bNewWatParameterFound = false;
	m_lStartTime = 0;
}

CGWATtoSTDF::~CGWATtoSTDF()
{
	while(!m_pWaferList.isEmpty())
		delete m_pWaferList.takeFirst();
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGWATtoSTDF::GetLastError()
{
	m_strLastError = "Import WAT: ";

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
// Load WAT Parameter table from DISK
//////////////////////////////////////////////////////////////////////
void CGWATtoSTDF::LoadParameterIndexTable(void)
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
// Save WAT Parameter table to DISK
//////////////////////////////////////////////////////////////////////
void CGWATtoSTDF::DumpParameterIndexTable(void)
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
    hWatTableFile << "# Quantix Examinator: WAT Parameters detected" << endl;
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
// If Examinator doesn't have this WAT parameter in his dictionnary, have it added.
//////////////////////////////////////////////////////////////////////
void CGWATtoSTDF::UpdateParameterIndexTable(QString strParamName)
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

		// Set flag to force the current WAT table to be updated on disk
		m_bNewWatParameterFound = true;
	}
}

//////////////////////////////////////////////////////////////////////
// Find or create Parameter entry, return handle to it.
//////////////////////////////////////////////////////////////////////
CGWatParameter *CGWATtoSTDF::FindParameterEntry(int iWaferID,int iSiteID,QString strParamName,QString strUnits)
{
	CGWatWafer *						ptWafers	= NULL;		// List of Wafers in WAT file
	CGWatParameter *					ptParam		= NULL;		// List of parameters
	QList<CGWatWafer*>::iterator		itWafer		= m_pWaferList.begin();
	QList<CGWatParameter*>::iterator	itParam;

	while(itWafer != m_pWaferList.end())
	{
		ptWafers = (*itWafer);

		if(ptWafers->iWaferID == iWaferID)
		{
			// Update Min/Max SiteID
			if(iSiteID > 0)
			{
				ptWafers->iLowestSiteID=gex_min(ptWafers->iLowestSiteID,iSiteID);				// Lowest WaferID found in WAT file
				ptWafers->iHighestSiteID=gex_max(ptWafers->iHighestSiteID,iSiteID);				// Highest WaferID found in WAT file
			}

			// Found WaferID entry...now find Parameter entry
			itParam = ptWafers->pParameterList.begin();
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

		// Not the WaferID we need...see next one.
		itWafer++;
	};

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
void CGWATtoSTDF::SaveParameterResult(int iWaferID,int iSiteID,QString strName,QString strUnits,QString strValue)
{
	if(strName.isNull() == true)
		return;	// Ignore empty entry!

	// Find Pointer to the Parameter cell
	CGWatParameter *ptParam = FindParameterEntry(iWaferID,iSiteID,strName,strUnits);

	// Save the Test value in it
	strValue.remove ('x');	// Remove any 'x' character if found (sometimes present to flag entry out of specs)
	strValue.remove ('?');	// Remove any '?' character if found (sometimes present to flag entry out of specs)
	strValue = strValue.trimmed();
	ptParam->fValue[iSiteID] = strValue.toFloat();
}

//////////////////////////////////////////////////////////////////////
// Save WAT parameter High or Low Limit result...
//////////////////////////////////////////////////////////////////////
void CGWATtoSTDF::SaveParameterLimit(QString strName,QString strValue,int iLimit)
{
	if(strName.isNull() == true)
		return;	// Ignore empty entry!

	CGWatParameter *				ptParam		= NULL;					// List of parameters
	QList<CGWatWafer*>::iterator	itWafer		= m_pWaferList.begin(); 

	while(itWafer != m_pWaferList.end())
	{
		// Get Parameter pointer...save limits in ALL WaferEntry.
        ptParam = FindParameterEntry((*itWafer)->iWaferID,0,strName);
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
		itWafer++;
	};
}


//////////////////////////////////////////////////////////////////////
// Check if File is compatible with WAT format
//////////////////////////////////////////////////////////////////////
bool CGWATtoSTDF::IsCompatible(const char *szFileName)
{
	QString strString;

	// Open hCsmFile file
    QFile f( szFileName );
    if(!f.open( QIODevice::ReadOnly ))
	{
		// Failed Opening ASL1000 file
		return false;
	}
	QTextStream hWatFile(&f);

	// Find the correct WAT header in the 10 first lines ...
	for(int i=0; i!=10; i++)
	{
		do
			strString = hWatFile.readLine();
		while(!strString.isNull() && strString.isEmpty());

		strString = strString.trimmed();	// remove leading spaces.
		if(strString.startsWith("W.A.T. DATA ATTACHED", Qt::CaseInsensitive))
		{
			f.close();
			return true;
		}
	}

	// Incorrect header...this is not a WAT file!
	f.close();
	return false;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the WAT file
//////////////////////////////////////////////////////////////////////
bool CGWATtoSTDF::ReadWatFile(const char *WatFileName)
{
	QString strMessage, strString;
	QString strSection;
	QString	strSite;
	QString	strParameters[10];	// Holds the 10 Parameters name (WAT file is organized by sets of 10 parameters columns)
	QString	strUnits[10];		// Holds the 10 Parameters Units 
	int		iWaferID;			// WaferID processed
	int		iSiteID;			// SiteID processed
	int		iIndex;				// Loop index

	// Debug trace
	strMessage = "---- CGWATtoSTDF::ReadWatFile(): reading wat file (";
	strMessage += WatFileName;
	strMessage += ")";
	WriteDebugMessageFile(strMessage);

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

	// Find the correct WAT header in the 10 first lines ...
	for(int i=0; i!=10; i++)
	{
		do
			strString = hWatFile.readLine();
		while(!strString.isNull() && strString.isEmpty());

		strString = strString.trimmed();	// remove leading spaces.
		if(strString.startsWith("W.A.T. DATA ATTACHED", Qt::CaseInsensitive))
			break;
	}

	if(!strString.startsWith("W.A.T. DATA ATTACHED", Qt::CaseInsensitive))
	{
		// Incorrect header...this is not a WAT file!
		m_iLastError = errInvalidFormat;
		
		// Convertion failed.
		// Close file
		f.close();
		return false;
	}

	// Read line : " TYPE NO :TMF671                        PROCESS  :0184W30LF1                    PCM SPEC:TBD           QTY:25 pcs"
	// KEY WORD : VALUE_WITHOUT_SPACE  (only for 25 pcs)
	// Read line until "WAF SITE"
	QString strKeyWord, strValue;
	strString = "";
	while (hWatFile.atEnd() == false)
	{
		if(strString.isEmpty())
			strString = ReadLine(hWatFile);					// Read ProductID, ProcessID
        if(strString.toUpper().indexOf("WAF SITE") >= 0)
			break;

		strKeyWord = strString.section(":",0,0).trimmed();
		strString = strString.section(":",1).trimmed();
		strValue = strString.section(" ",0,0);
        if((strString.indexOf(":") > 0)
		&& (strValue.section(":",0,0).trimmed() == strString.section(":",0,0).trimmed()))
		{
			// empty value
			strValue = "";
		}
		else
			strString = strString.section(" ",1).trimmed();
		
		if(strKeyWord.toUpper() == "TYPE NO")
			m_strProductID = strValue;
		else
		if(strKeyWord.toUpper() == "PROCESS")
			m_strProcessID = strValue;
		else
		if(strKeyWord.toUpper() == "PCM SPEC")
			m_strSpecID = strValue;
		else
        if(strKeyWord.endsWith("QTY",Qt::CaseInsensitive))
			strString = "";
		else
        if(strKeyWord.endsWith("PASS",Qt::CaseInsensitive))
			strString = "";
		else
		if(strKeyWord.toUpper() == "LOT ID")
			m_strLotID = strValue;
		else
		if(strKeyWord.toUpper() == "DATE")
		{
			// Extract TimeStamp, convert to UINT (nbr of seconds since 1-1-1970)
			int iDay,iMonth,iYear;
			strSection = strValue.section('/',0,0);	// First field is Month
			iMonth = strSection.toInt();
			strSection = strValue.section('/',1,1);	// Second field is Day
			iDay = strSection.toInt();
			strSection = strValue.section('/',2,2);	// Last field is Year
			iYear = strSection.toInt();
			QDate WatDate(iYear,iMonth,iDay);
			QDateTime WateDateTime(WatDate);
			WateDateTime.setTimeSpec(Qt::UTC);
			m_lStartTime = WateDateTime.toTime_t();

			strString = "";
		}
	}


	// ALREADY READ - Read line with list of Parameters names
	// eg: " WAF SITE    VT_N4 N1u   Isat_N4     BV_N4       VT_P4 N1u   Isat_P4     BV_P4       VT_N43 N1u  Isat_N43    BV_N43      VT_P43 N1u"
	//
	// For some specific WAT, have an additional offset of 3 spaces between each test line
	// Have to found -------- line, and if exist have to add offset
	//WAF SITE   BV_N2 A        BV_N2 S        BV_N3          BV_P2 A        BV_P2 S        BV_P4          CONTI_M1       CONTI_M2       
	//ID  ID     V @1uA         V @1uA         V @0.1uA       V @1uA         V @1uA         V @-0.1uA      OHM  0.6um     OHM  0.7um     
	//--- ----   ------------   ------------   ------------   ------------   ------------   ------------   ------------   ------------   


	// Loop reading file until end is reached
	QString strLineParam1, strLineParam2;
	int nOffset = 0;
	int nStart = 12;
	do
	{	
		strLineParam1 = strString;
		strString = ReadLine(hWatFile);
		strLineParam2 = strString;
		strString = ReadLine(hWatFile);
		// Check if have some offset
        if(strString.indexOf("------------") > 0)
		{

            nStart = strString.indexOf("------------");
			strString = strString.section("------------",1);
			nOffset = strString.section("------------",0,0).length();
			strString = ReadLine(hWatFile);
		}

		// Extract the 10 column names (or less if not all are filled)
		for(iIndex=0;iIndex<10;iIndex++)
		{
			strSection = strLineParam1.mid(nStart+iIndex*(12+nOffset),12);
			strSection = strSection.trimmed();	// Remove spaces
            if (!strSection.isEmpty())
                strParameters[iIndex] = strSection;
		}
		
		// Extract the 10 column Parameters second half of the name (usually contains units)
		for(iIndex=0;iIndex<10;iIndex++)
		{
			strSection = strLineParam2.mid(nStart+iIndex*(12+nOffset),12);
			strSection = strSection.trimmed();	// Remove spaces
            if (!strSection.isEmpty())
            {
                strParameters[iIndex] += " " + strSection;
                strSection.replace("(","").replace(")", ""); // replace ( and ) by empty char
                QStringList lListUnits = strSection.split(" ");
                if (lListUnits.size() > 0)
                    strUnits[iIndex] = lListUnits[0].trimmed();  // the unit is the part before the space
                else
                    strUnits[iIndex] = " ";
            }
		}

		
        while(strString.indexOf("----------------------") < 0)
		{
			// Read line of Parameter data unless end of data reached (reaching Param. Mean, Sigma and limits)
			// eg: " 7  -1        0.421       6.512       4.047      -0.469      -3.066      -5.168       0.678       6.663       7.000      -0.667"
			// or may be end of data, in which case the line is full of '------'
	
			// Extract WaferID, and save it to the parameter list as a parameter itself!
			strSection = strString.mid(0,3);
			strSection.remove ('-');	// Remove leading '-' if exists
			iWaferID = strSection.toInt();

			// Extract SiteID
			strSite = strString.mid(5,4);
			strSite.remove ('-');	// Remove leading '-' if exists
			strSite.remove ('#');	// Remove leading '#' if exists...sometimes used to flag a site with failing parameters
			iSiteID = strSite.toInt();

            if(strSection.isEmpty() != true)
				SaveParameterResult(iWaferID,iSiteID,"WAF ID","",strSection);

			// For each column, extract parameter value and save it
			for(iIndex=0;iIndex<10;iIndex++)
			{
				strSection = strString.mid(nStart+iIndex*(12+nOffset),12);
					// Save parameter result in buffer
				if(strSection.isNull() != true)
                    SaveParameterResult(iWaferID,iSiteID,strParameters[iIndex],strUnits[iIndex],strSection);
			}

			// Check if last line processed (should normally never happen as statistcs should follow...)
			if(hWatFile.atEnd())
				break;

			// Read next line in file...
			strString = ReadLine(hWatFile);
		};

		// Have reached the statistics section:
		// -------------------------------------------------------------------------------------------------------------------------------
		// AVERAGE      0.424       6.477       4.036      -0.470      -3.048      -5.206       0.674       6.618       7.000      -0.668
		// STD DEV     2.347E-03    0.035       0.015      7.493E-03    0.059       0.041      5.560E-03    0.053       0.000      3.029E-03
		// SPEC HI      0.520       6.900       12.000     -0.400      -2.210      -3.600       0.820       6.900       20.000     -0.640
		// SPEC LO      0.320       5.100       3.600      -0.600      -2.990      -12.000      0.620       5.100       6.000      -0.840
		// -------------------------------------------------------------------------------------------------------------------------------

		// Skip line starting with 'AVERAGE'
		ReadLine(hWatFile);
		// Skip line starting with 'STD DEV'
		ReadLine(hWatFile);

		// Read Parameters Spec HIGH limits.
		strString = ReadLine(hWatFile);
		for(iIndex=0;iIndex<10;iIndex++)
		{
			strSection = strString.mid(nStart+iIndex*(12+nOffset),12);
			if(strSection.isNull() != true)
			{
				// Remove spaces
				strSection = strSection.trimmed();
				// Save parameter result in buffer
				SaveParameterLimit(strParameters[iIndex],strSection,eHighLimit);
			}
		}

		// Read Parameters Spec LOW limits.
		strString = ReadLine(hWatFile);
		for(iIndex=0;iIndex<10;iIndex++)
		{
			strSection = strString.mid(nStart+iIndex*(12+nOffset),12);
			if(strSection.isNull() != true)
			{
				// Remove spaces
				strSection = strSection.trimmed();
				// Save parameter result in buffer
				SaveParameterLimit(strParameters[iIndex],strSection,eLowLimit);
			}
		}

		// Skip line starting with '---------'
		ReadLine(hWatFile);

		// Should point to next bloc of parameters..unless end of file!
		do
		{
		  strString = ReadLine(hWatFile);
		}
        while((strString.indexOf("WAF SITE") < 0) && (hWatFile.atEnd() == false));

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
bool CGWATtoSTDF::WriteStdfFile(const char *strFileNameSTDF)
{
	QString strMessage;

	// Debug trace
	strMessage = "---- CGWATtoSTDF::WriteStdfFile(): writing STDF file (";
	strMessage += strFileNameSTDF;
	strMessage += ")";
	WriteDebugMessageFile(strMessage);

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
    strUserTxt += ":VIS";
	StdfFile.WriteString(strUserTxt.toLatin1().constData());	// user-txt
	StdfFile.WriteString("");							// aux-file
	StdfFile.WriteString("");							// package-type
	StdfFile.WriteString(m_strProductID.toLatin1().constData());	// familyID
	StdfFile.WriteString("");							// Date-code
	StdfFile.WriteString("");							// Facility-ID
	StdfFile.WriteString("");							// FloorID
	StdfFile.WriteString(m_strProcessID.toLatin1().constData());	// ProcessID
	StdfFile.WriteString("");							// Operation
    StdfFile.WriteString("");  // spec_nam
    StdfFile.WriteString(m_strSpecID.toLatin1().constData());  // spec_ver

	StdfFile.WriteRecord();

	// Write Test results for each waferID
	char		szString[257];
	BYTE		iSiteNumber,bData;
	WORD		wSoftBin,wHardBin;
	long		iTotalGoodBin,iTotalFailBin;
	long		iTestNumber,iPartNumber=0;
	bool		bPassStatus;

	CGWatParameter *					ptParam		= NULL;
	CGWatWafer *						ptWafers	= NULL;		// List of Wafers in WAT file
	QList<CGWatWafer*>::iterator		itWafer		= m_pWaferList.begin();
	QList<CGWatParameter*>::iterator	itParam; 
	
	while(itWafer != m_pWaferList.end())
	{
		ptWafers = (*itWafer);

		// Debug trace
		strMessage = "---- CGWATtoSTDF::WriteStdfFile(): writing wafer " + QString::number(ptWafers->iWaferID);
		strMessage += " (lowest site=" + QString::number(ptWafers->iLowestSiteID);
		strMessage += ", highest site=" + QString::number(ptWafers->iHighestSiteID) + ")";
		WriteDebugMessageFile(strMessage);

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
		for(iSiteNumber = (BYTE)ptWafers->iLowestSiteID; iSiteNumber <= (BYTE)ptWafers->iHighestSiteID; iSiteNumber++)
		{
#if 0
			// Debug trace
			strMessage = "---- CGWATtoSTDF::WriteStdfFile(): writing PIR ";
			strMessage += " (site " + QString::number(iSiteNumber) + ")";
			WriteDebugMessageFile(strMessage);
#endif

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

			itParam = ptWafers->pParameterList.begin();	// First test in list
			
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

#if 0
					// Debug trace
					strMessage = "---- CGWATtoSTDF::WriteStdfFile(): writing PTR for test " + QString::number(iTestNumber);
					strMessage += " (site " + QString::number(iSiteNumber) + ")";
					WriteDebugMessageFile(strMessage);
#endif

					StdfFile.WriteDword(iTestNumber);			// Test Number
					StdfFile.WriteByte(1);						// Test head
					StdfFile.WriteByte(iSiteNumber);			// Tester site:1,2,3,4 or 5, etc.
					if((ptParam->bValidLowLimit) && (ptParam->fValue[iSiteNumber] < ptParam->fLowLimit))
					{
						bData = 0200;	// Test Failed
						bPassStatus = false;
					}
					else if((ptParam->bValidHighLimit) && (ptParam->fValue[iSiteNumber] > ptParam->fHighLimit))
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

#if 0
			// Debug trace
			strMessage = "---- CGWATtoSTDF::WriteStdfFile(): writing PRR ";
			strMessage += " (site " + QString::number(iSiteNumber) + ")";
			WriteDebugMessageFile(strMessage);
#endif

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
				case 8:	// toUpper-Left corner
					StdfFile.WriteWord(0);			// X_COORD
					StdfFile.WriteWord(0);			// Y_COORD
					break;
				case 9:	// toUpper-Right corner
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
		StdfFile.WriteByte(255);					// Tester site (all)
		StdfFile.WriteDword(m_lStartTime);			// Time of last part tested
		StdfFile.WriteDword(iTotalGoodBin+iTotalFailBin);	// Parts tested: always 5
		StdfFile.WriteDword(0);						// Parts retested
		StdfFile.WriteDword(0);						// Parts Aborted
		StdfFile.WriteDword(iTotalGoodBin);			// Good Parts
		StdfFile.WriteDword((DWORD)-1);				// Functionnal Parts

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
		// Not the WaferID we need...see next one.
		itWafer++;
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
bool CGWATtoSTDF::Convert(const char *WatFileName, const char *strFileNameSTDF)
{
	QString strMessage;

	// No erro (default)
	m_iLastError = errNoError;

	// Debug trace
	strMessage = "---- CGWATtoSTDF::Convert(): converting wat file (";
	strMessage += WatFileName;
	strMessage += " -> ";
	strMessage += strFileNameSTDF;
	strMessage += ")";
	WriteDebugMessageFile(strMessage);

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
QString CGWATtoSTDF::ReadLine(QTextStream& hFile)
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
#if 0
		// Debug trace
		strMessage = "---- CGWATtoSTDF::ReadLine(): line read (" + strString + ")";
		WriteDebugMessageFile(strMessage);
#endif
	}
	while(!strString.isNull() && strString.isEmpty());

	return strString;

}
