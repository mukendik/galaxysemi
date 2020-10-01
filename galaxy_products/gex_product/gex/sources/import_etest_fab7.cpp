//////////////////////////////////////////////////////////////////////
// import_etest_fab7.cpp: Convert a Etest Fab7 .csv
// file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#include "gqtl_global.h"
#include <qmath.h>

#ifdef _WIN32
#include "windows.h"		// For 'GetWindowsDirectoryA' declaration
#endif

#include <qfileinfo.h>
#include <qregexp.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>
#include "engine.h"
#include "import_etest_fab7.h"
#include "time.h"
#include "import_constants.h"

//
//Lot ID,7IYG29523.000,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,, [...]
//Timestamp (Start),9/8/2010 6:49,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,, [...]
//Timestamp (End),9/8/2010 17:19,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,, [...]
//Fab,7,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,, [...]
//Technology,cms10sf,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,, [...]
//Product,VX185.01,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,, [...]
//Test Program,1950C,1992C,1995C,1997C,1998C,7918A,7918D,1999B,1932C, [...]
//Equipment Id,yets729,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,, [...]
//Parameter Count,143,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,, [...]
//Temperature,25,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,, [...]
//Flat Orientation,RIGHT,RIGHT,RIGHT,RIGHT,RIGHT,RIGHT,RIGHT,RIGHT,RI [...]
//Wafer Count,8,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,, [...]
//Test Level (Metal Type),LT,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,, [...]
//,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
//,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
//,,,PNP,1950C,,,,,,,,,,,,,,,,,,,,,,,,,,,,PNP,1992C,,,PNP,1998C,,,,,, [...]
//,,,SiteCount,88,,,,,,,,,,,,,,,,,,,,,,,,,,,,SiteCount,88,,,SiteCount [...]
//,,,ParameterCount,27,,,,,,,,,,,,,,,,,,,,,,,,,,,,ParameterCount,2,,, [...]
//WaferID, Pass/Fail, SiteID,Site_X, Site_Y,WAC_BB_LT_N+MAZE_Rs/Sq,WA [...]
//,,,Unit,,Ohm/sq,Ohm/sq,Ohm/sq,Ohm/sq,Ohm/Sq,Ohms/sq,Ohm/sq,ohm/um,o [...]
//,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
//5,Fail,1,5,21,9.393,8.009,9.785,8.399,643.6,513.4,115.2,2.085,1.523 [...]
//,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
//MAX,,,,,9.77,8.332,10.24,9.933,666.7,521.2,118.1,2.615,1.89,1.832,2 [...]
//MIN,,,,,8.885,7.364,8.655,6.859,619,499.6,110.5,1.716,1.273,1.265,1 [...]
//AVG,,,,,9.231920455,7.946045455,9.430193182,7.825090909,640.4284091 [...]
//MEDIAN,,,,,9.227,7.977,9.429,7.632,639.05,511.75,114.6,2.1125,1.580 [...]
//STDEV,,,,,0.165006668,0.21013985,0.359007279,0.636843358,10.6344263 [...]
//SPEC HIGH,,,,,1.50E+01,1.50E+01,1.30E+01,1.30E+01,8.45E+02,6.31E+02 [...]
//SPEC LOW,,,,,4.00E+00,4.00E+00,4.00E+00,4.00E+00,4.55E+02,4.21E+02, [...]


// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGEtestFab7toSTDF::CGEtestFab7toSTDF()
{
	// Default: EtestFab7 parameter list on disk includes all known EtestFab7 parameters...
	m_bNewEtestFab7ParameterFound = false;
	m_lStartTime = m_lStopTime = 0;
	m_nTotalParts = 0;
	m_nPassParts = 0;
	m_nFailParts = 0;

	m_pEtestFab7Parameter = NULL;
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CGEtestFab7toSTDF::~CGEtestFab7toSTDF()
{
	// Destroy list of Parameters tables.
	if(m_pEtestFab7Parameter!=NULL)
		delete [] m_pEtestFab7Parameter;
}


//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGEtestFab7toSTDF::GetLastError()
{
	strLastError = "Import Etest Fab7: ";

	switch(iLastError)
	{
		default:
		case errNoError:
			strLastError += "No Error";
			break;
		case errOpenFail:
			strLastError += "Failed to open file";
			break;
		case errInvalidFormat:
			strLastError += "Invalid file format";
			break;
		case errInvalidFormatLowInRows:
			strLastError += "Invalid file format: 'Parameter' line too short, missing rows";
			break;
		case errNoLimitsFound:
			strLastError += "Invalid file format, Specification Limits not found";
			break;
		case errWriteSTDF:
			strLastError += "Failed creating temporary file. Folder permission issue?";
			break;	
		case errLicenceExpired:
			strLastError += "License has expired or Data file out of date...";
			break;	
	}
	// Return Error Message
	return strLastError;
}

//////////////////////////////////////////////////////////////////////
// Load EtestFab7 Parameter table from DISK
//////////////////////////////////////////////////////////////////////
void CGEtestFab7toSTDF::LoadParameterIndexTable(void)
{
	QString	strEtestFab7TableFile;
	QString	strString;

    strEtestFab7TableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strEtestFab7TableFile += GEX_ETEST_FAB7_PARAMETERS;

	// Open EtestFab7 Parameter table file
	QFile f( strEtestFab7TableFile );
    if(!f.open( QIODevice::ReadOnly ))
		return;

	// Assign file I/O stream
	QTextStream hEtestFab7TableFile(&f);

	// Skip comment lines
	do
	{
	  strString = hEtestFab7TableFile.readLine();
	}
    while((strString.indexOf("----------------------") < 0) && (hEtestFab7TableFile.atEnd() == false));

	// Read lines
	m_pFullEtestFab7ParametersList.clear();
	strString = hEtestFab7TableFile.readLine();
	while (strString.isNull() == false)
	{
		// Save Parameter name in list
		m_pFullEtestFab7ParametersList.append(strString);
		// Read next line
		strString = hEtestFab7TableFile.readLine();
	};

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// Save EtestFab7 Parameter table to DISK
//////////////////////////////////////////////////////////////////////
void CGEtestFab7toSTDF::DumpParameterIndexTable(void)
{
	QString		strEtestFab7TableFile;
	int	uIndex;

    strEtestFab7TableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strEtestFab7TableFile += GEX_ETEST_FAB7_PARAMETERS;

	// Open EtestFab7 Parameter table file
	QFile f( strEtestFab7TableFile );
    if(!f.open( QIODevice::WriteOnly ))
		return;

	// Assign file I/O stream
	QTextStream hEtestFab7TableFile(&f);

	// First few lines are comments:
	hEtestFab7TableFile << "############################################################" << endl;
	hEtestFab7TableFile << "# DO NOT EDIT THIS FILE!" << endl;
    hEtestFab7TableFile << "# Quantix Examinator: Etest Fab7 Parameters detected" << endl;
	hEtestFab7TableFile << "# www.mentor.com" << endl;
    hEtestFab7TableFile << "# Quantix Examinator reads and writes into this file..." << endl;
	hEtestFab7TableFile << "-----------------------------------------------------------" << endl;

	// Write lines
	// m_pFullEtestFab7ParametersList.sort();
	for(uIndex=0;uIndex<m_pFullEtestFab7ParametersList.count();uIndex++)
	{
		// Write line
		hEtestFab7TableFile << m_pFullEtestFab7ParametersList[uIndex] << endl;
	};

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// If Examinator doesn't have this EtestFab7 parameter in his dictionnary, have it added.
//////////////////////////////////////////////////////////////////////
void CGEtestFab7toSTDF::UpdateParameterIndexTable(QString strParamName)
{
	// Check if the table is empty...if so, load it from disk first!
	if(m_pFullEtestFab7ParametersList.isEmpty() == true)
	{
		// Load EtestFab7 parameter table from disk...
		LoadParameterIndexTable();
	}
	
	// Check if Parameter name already in table...if not, add it to the list
	// the new full list will be dumped to the disk at the end.
    if(m_pFullEtestFab7ParametersList.indexOf(strParamName) < 0)
	{
		// Update list
		m_pFullEtestFab7ParametersList.append(strParamName);

		// Set flag to force the current EtestFab7 table to be updated on disk
		m_bNewEtestFab7ParameterFound = true;
	}
}


//////////////////////////////////////////////////////////////////////
// Check if File is compatible with EtestFab7 format
//////////////////////////////////////////////////////////////////////
bool CGEtestFab7toSTDF::IsCompatible(const char *szFileName)
{
	QString strString;

	// Open hCsmFile file
    QFile f( szFileName );
    if(!f.open( QIODevice::ReadOnly ))
	{
		// Failed Opening ASL1000 file
		return false;
	}
	// Assign file I/O stream
	QTextStream hEtestFab7File(&f);

	// Check if found the data line
	//WaferID, Pass/Fail, SiteID,Site_X, Site_Y,
	int nLine = 0;
	while(!hEtestFab7File.atEnd())
	{
		strString = hEtestFab7File.readLine().remove(" ").toUpper();
		if(strString.startsWith("WAFERID,PASS/FAIL,SITEID,SITE_X,SITE_Y"))
		{
			f.close();
			return true;
		}
		nLine++;
		if(nLine > 30)
			break;
	}

	// Incorrect header...this is not a EtestFab7 file!
	f.close();
	return false;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the EtestFab7 file
//////////////////////////////////////////////////////////////////////
bool CGEtestFab7toSTDF::ReadEtestFab7File(const char *EtestFab7FileName,const char *strFileNameSTDF)
{
	QString strString;
	QString strSection;
	QString strValue;
	bool	bStatus;
	int		iIndex;				// Loop index

	// Open CSV file
	QFile f( EtestFab7FileName );
    if(!f.open( QIODevice::ReadOnly ))
	{
		// Failed Opening EtestFab7 file
		iLastError = errOpenFail;
		
		// Convertion failed.
		return false;
	}

 	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	iNextFilePos = 0;
	iProgressStep = 0;
	iFileSize = f.size()*2 + 1;
	
	// Assign file I/O stream
	QTextStream hEtestFab7File(&f);
	QDate	clDate;
	QTime	clTime;

	// Read EtestFab7 information

	//Lot ID,7IYG29523.000,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,, [...]
	//Timestamp (Start),9/8/2010 6:49,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,, [...]
	//Timestamp (End),9/8/2010 17:19,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,, [...]
	//Fab,7,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,, [...]
	//Technology,cms10sf,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,, [...]
	//Product,VX185.01,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,, [...]
	//Test Program,1950C,1992C,1995C,1997C,1998C,7918A,7918D,1999B,1932C, [...]
	//Equipment Id,yets729,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,, [...]
	//Parameter Count,143,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,, [...]
	//Temperature,25,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,, [...]
	//Flat Orientation,RIGHT,RIGHT,RIGHT,RIGHT,RIGHT,RIGHT,RIGHT,RIGHT,RI [...]
	//Wafer Count,8,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,, [...]
	//Test Level (Metal Type),LT,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,, [...]

	while(!hEtestFab7File.atEnd())
	{
		strString = ReadLine(hEtestFab7File);

		strSection = strString.section(",",0,0).remove(' ').toLower();

		if(strSection.isEmpty())
			break;

		if(strSection == "lotid")
		{
			//Lot ID,7IYG29523.000
			strValue = strString.section(",",1,1).simplified();
			m_strLotID = strValue;
		}
		else if(strSection == "timestamp(start)")
		{
			//Timestamp (Start),9/8/2010 6:49
			strValue = strString.section(",",1,1).simplified();
			QString	strDate = strValue.section(" ",0,0);
			QString strTime = strValue.section(" ",1);
			clDate = QDate(strDate.section("/",2,2).toInt(),strDate.section("/",1,1).toInt(),strDate.section("/",0,0).toInt());
			clTime = QTime::fromString(strTime);

			QDateTime clDateTime(clDate,clTime);
			clDateTime.setTimeSpec(Qt::UTC);
			m_lStartTime = clDateTime.toTime_t();
		}
		else if(strSection == "timestamp(end)")
		{
			//Timestamp (End),9/8/2010 17:19
			strValue = strString.section(",",1,1).simplified();
			QString	strDate = strValue.section(" ",0,0);
			QString strTime = strValue.section(" ",1);
			clDate = QDate(strDate.section("/",2,2).toInt(),strDate.section("/",1,1).toInt(),strDate.section("/",0,0).toInt());
			clTime = QTime::fromString(strTime);

			QDateTime clDateTime(clDate,clTime);
			clDateTime.setTimeSpec(Qt::UTC);
			m_lStopTime = clDateTime.toTime_t();
		}
		else if(strSection == "fab")
		{
			//Fab,7
			m_strFabId = strString.section(",",1,1).simplified();
			strString = "";
		}
		else if(strSection == "technology")
		{
			//Technology,cms10sf
			m_strProcId = strString.section(",",1,1).simplified();
			strString = "";
		}
		else if(strSection == "product")
		{
			//Product,VX185.01
			m_strProductID = strString.section(",",1,1).simplified();
			strString = "";
		}
		else if(strSection == "testprogram")
		{
			//Test Program,1950C,1992C,1995C,1997C,1998C,7918A,7918D,1999B,1932C
			m_strJobName = strString.section(",",1).remove(",,").simplified();
			strString = "";
		}
		else if(strSection == "equipmentid")
		{
			//Equipment Id,yets729
			m_strTesterType = strString.section(",",1,1).simplified();
			strString = "";
		}
		else if(strSection == "temperature")
		{
			//Temperature,25
			m_strTemperature = strString.section(",",1,1).simplified();
			strString = "";
		}
		else if(strSection == "testlevel(metaltype)")
		{
			//Test Level (Metal Type),LT
			m_strTestCod = strString.section(",",1,1).simplified();
			strString = "";
		}
	}

	//,,,PNP,1950C,,,,,,,,,,,,,,,,,,,,,,,,,,,,PNP,1992C,,,PNP,1998C,,,,,, [...]
	//,,,SiteCount,88,,,,,,,,,,,,,,,,,,,,,,,,,,,,SiteCount,88,,,SiteCount [...]
	//,,,ParameterCount,27,,,,,,,,,,,,,,,,,,,,,,,,,,,,ParameterCount,2,,, [...]
	//WaferID, Pass/Fail, SiteID,Site_X, Site_Y,WAC_BB_LT_N+MAZE_Rs/Sq,WA [...]
	//,,,Unit,,Ohm/sq,Ohm/sq,Ohm/sq,Ohm/sq,Ohm/Sq,Ohms/sq,Ohm/sq,ohm/um,o [...]
	//,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
	//5,Fail,1,5,21,9.393,8.009,9.785,8.399,643.6,513.4,115.2,2.085,1.523 [...]
	//,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
	//MAX,,,,,9.77,8.332,10.24,9.933,666.7,521.2,118.1,2.615,1.89,1.832,2 [...]
	//MIN,,,,,8.885,7.364,8.655,6.859,619,499.6,110.5,1.716,1.273,1.265,1 [...]
	//AVG,,,,,9.231920455,7.946045455,9.430193182,7.825090909,640.4284091 [...]
	//MEDIAN,,,,,9.227,7.977,9.429,7.632,639.05,511.75,114.6,2.1125,1.580 [...]
	//STDEV,,,,,0.165006668,0.21013985,0.359007279,0.636843358,10.6344263 [...]
	//SPEC HIGH,,,,,1.50E+01,1.50E+01,1.30E+01,1.30E+01,8.45E+02,6.31E+02 [...]
	//SPEC LOW,,,,,4.00E+00,4.00E+00,4.00E+00,4.00E+00,4.55E+02,4.21E+02, [...]

	
	// Goto parameters def
	while(!hEtestFab7File.atEnd())
	{
		strString = ReadLine(hEtestFab7File).remove(" ");
		if(strString.startsWith("WaferID,Pass/Fail,SiteID,Site_X,Site_Y", Qt::CaseInsensitive))
			break;
	}

	if(strString.startsWith("WaferID,Pass/Fail,SiteID,Site_X,Site_Y", Qt::CaseInsensitive) == false)
	{
		// Incorrect header...this is not a EtestFab7 file!
		iLastError = errInvalidFormat;
		f.close();
		return false;
	}
	m_iDataOffset = 5;

    QStringList lstSections = strString.split(",",QString::KeepEmptyParts);
	// Count the number of parameters specified in the line
	m_iTotalParameters=lstSections.count();
	// If no parameter specified...ignore!
	if(m_iTotalParameters <= m_iDataOffset)
	{
		// Incorrect header...this is not a valid EtestFab7 file!
		iLastError = errInvalidFormat;
		
		// Convertion failed.
		f.close();
		return false;
	}

	// Allocate the buffer to hold the N parameters & results.
	m_pEtestFab7Parameter = new CGEtestFab7Parameter[m_iTotalParameters];	// List of parameters

	// Unit line
	strString = ReadLine(hEtestFab7File);
	if(strString.contains("Unit", Qt::CaseInsensitive) == false)
	{
		// Incorrect header...this is not a EtestFab7 file!
		iLastError = errNoLimitsFound;
		f.close();
		return false;
	}
    QStringList lstUnits = strString.split(",",QString::KeepEmptyParts);

	// Goto limits def
	while(!hEtestFab7File.atEnd())
	{
		strString = ReadLine(hEtestFab7File);
		if(strString.left(9) == "SPEC HIGH")
			break;
	}

	if(strString.startsWith("SPEC HIGH", Qt::CaseInsensitive) == false)
	{
		// Incorrect header...this is not a EtestFab7 file!
		iLastError = errNoLimitsFound;
		f.close();
		return false;
	}
    QStringList lstHighLimits = strString.split(",",QString::KeepEmptyParts);

	strString = ReadLine(hEtestFab7File);
	if(strString.startsWith("SPEC LOW", Qt::CaseInsensitive) == false)
	{
		// Incorrect header...this is not a EtestFab7 file!
		iLastError = errNoLimitsFound;
		f.close();
		return false;
	}
    QStringList lstLowLimits = strString.split(",",QString::KeepEmptyParts);

	// Extract the N column names
	// Do not count first 4 fields
	QString strUnit;
	QString strLowLimit;
	QString strHighLimit;
	int nScale;

	for(iIndex=m_iDataOffset;iIndex<m_iTotalParameters;iIndex++)
	{
		strSection = lstSections[iIndex].simplified();

		if(strSection.isEmpty())
			break;

		if(strSection.simplified().toUpper() == "SITE_X")
		{
			m_pEtestFab7Parameter[iIndex].strName = strSection.simplified().toUpper();
			continue;
		}

		if(strSection.simplified().toUpper() == "SITE_Y")
		{
			m_pEtestFab7Parameter[iIndex].strName = strSection.simplified().toUpper();
			continue;
		}

		strUnit = lstUnits[iIndex];
		strLowLimit = lstLowLimits[iIndex];
		strHighLimit = lstHighLimits[iIndex];
		nScale = 0;

		NormalizeLimits(strUnit, nScale);

		UpdateParameterIndexTable(strSection);		// Update Parameter master list if needed.

		m_pEtestFab7Parameter[iIndex].nNumber = m_pFullEtestFab7ParametersList.indexOf(strSection);
		m_pEtestFab7Parameter[iIndex].strName = strSection;
		m_pEtestFab7Parameter[iIndex].strUnits = strUnit;
		m_pEtestFab7Parameter[iIndex].nScale = nScale;
		m_pEtestFab7Parameter[iIndex].fHighLimit = strHighLimit.toFloat(&(m_pEtestFab7Parameter[iIndex].bValidHighLimit));
		m_pEtestFab7Parameter[iIndex].fLowLimit = strLowLimit.toFloat(&(m_pEtestFab7Parameter[iIndex].bValidLowLimit));
		m_pEtestFab7Parameter[iIndex].bStaticHeaderWritten = false;
	}

	m_iTotalParameters = iIndex;

	// It's a Etest Fab7 file
	// Reset file position and start to write STDF file
	hEtestFab7File.seek(0);
	iNextFilePos = 0;

	// Loop reading file until end is reached & generate STDF file dynamically.
	bStatus = WriteStdfFile(&hEtestFab7File,strFileNameSTDF);
	if(!bStatus)
		QFile::remove(strFileNameSTDF);

	// All EtestFab7 file read...check if need to update the EtestFab7 Parameter list on disk?
	if(bStatus && (m_bNewEtestFab7ParameterFound == true))
		DumpParameterIndexTable();

	// Success parsing EtestFab7 file
	f.close();
	return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from EtestFab7 data parsed
//////////////////////////////////////////////////////////////////////
bool CGEtestFab7toSTDF::WriteStdfFile(QTextStream *hEtestFab7File, const char *strFileNameSTDF)
{
	// now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open((char*)strFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
	{
		// Failed importing CSV file into STDF database
		iLastError = errWriteSTDF;
		
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
	StdfFile.WriteString(m_strTesterType.toLatin1().constData());	// Tester Type
	StdfFile.WriteString(m_strJobName.toLatin1().constData());		// Job name
	StdfFile.WriteString("");					// Job rev
	StdfFile.WriteString("");					// sublot-id
	StdfFile.WriteString("");					// operator
	StdfFile.WriteString("");					// exec-type
	StdfFile.WriteString("");					// exe-ver
    StdfFile.WriteString(m_strTestCod.toLatin1().constData());			// test-cod
	StdfFile.WriteString(m_strTemperature.toLatin1().constData());	// test-temperature
	// Construct custom Galaxy USER_TXT 
	QString	strUserTxt;
	strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;	
	strUserTxt += ":";
	strUserTxt += GEX_IMPORT_DATAORIGIN_ETEST;
	strUserTxt += ":ETEST_FAB7";
	StdfFile.WriteString((char *)strUserTxt.toLatin1().constData());	// user-txt
	StdfFile.WriteString("");							// aux-file
	StdfFile.WriteString("");							// package-type
	StdfFile.WriteString(m_strProductID.toLatin1().constData());	// familyID
	StdfFile.WriteString("");							// date_cod
	StdfFile.WriteString(m_strFabId.toLatin1().constData());		// facil_id
	StdfFile.WriteString("");							// floor_id
	StdfFile.WriteString(m_strProcId.toLatin1().constData());		// proc_id
	StdfFile.WriteRecord();

	// Write Test results for each line read.
	QString strString;
	QString strWaferID;
	char	szString[257];
	QString strSection;
	float	fValue;				// Used for readng floating point numbers.
	int		iIndex;				// Loop index
	int		iSite;
	int		iXpos, iYpos;
	int		iBin;
	bool	bPassStatus;
	long		iTotalGoodBin,iTotalFailBin;
	long		iTotalTests,iPartNumber;
	bool		bStatus;
	QStringList	lstSections;
	BYTE		bData;
  int lStartTime;
  //FIXME: not used ?
  //int lStopTime;

	lStartTime = m_lStartTime;
  //FIXME: not used ?
  //lStopTime = m_lStopTime;
	
	// Reset counters
	iTotalGoodBin=iTotalFailBin=0;
	iPartNumber=0;

	// Write all Parameters read on this wafer.: WIR.PIR,PTR....PTR, PRR, PIR, PTR,PTR...PRR,   ... WRR

	// Goto data
	while(!hEtestFab7File->atEnd())
	{
		strString = ReadLine(*hEtestFab7File);
		if(strString.left(8) == "WaferID,")
			break;
	}

	m_strWaferID = "";

	//WaferID, Pass/Fail, SiteID,Site_X, Site_Y,WAC_BB_LT_N+MAZE_Rs/Sq,WA [...]
	//,,,Unit,,Ohm/sq,Ohm/sq,Ohm/sq,Ohm/sq,Ohm/Sq,Ohms/sq,Ohm/sq,ohm/um,o [...]
	//,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
	//5,Fail,1,5,21,9.393,8.009,9.785,8.399,643.6,513.4,115.2,2.085,1.523 [...]

	while(hEtestFab7File->atEnd() == false)
	{

		// Read line
		strString = ReadLine(*hEtestFab7File).simplified();

		if(strString.isEmpty())
			continue;

		if(strString.left(1) == ",")
			continue;
		if(strString.left(4) == "MAX,")
			break;

        lstSections = strString.split(",",QString::KeepEmptyParts);


		// Check if have the good count
		if(lstSections.count() < m_iDataOffset)
		{
			iLastError = errInvalidFormatLowInRows;

			StdfFile.Close();
			// Convertion failed.
			return false;
		}

		iIndex = 0;
		strWaferID = lstSections[iIndex++];
		bPassStatus = (lstSections[iIndex++].toUpper() != "FAIL");
		iSite = lstSections[iIndex++].toInt(&bStatus);
		iXpos = lstSections[iIndex++].toInt();
		iYpos = lstSections[iIndex++].toInt();

		// Check if start with a valid site num
		if(!bStatus)
			continue;


		if(m_strWaferID != strWaferID)
		{
			if(!m_strWaferID.isEmpty())
			{
				lStartTime = 0;

				// Write WRR for last wafer inserted
				RecordReadInfo.iRecordType = 2;
				RecordReadInfo.iRecordSubType = 20;
				StdfFile.WriteHeader(&RecordReadInfo);
				StdfFile.WriteByte(1);						// Test head
				StdfFile.WriteByte(255);					// Tester site (all)
				StdfFile.WriteDword(0);					// Time of last part tested
				StdfFile.WriteDword(iTotalGoodBin+iTotalFailBin);	// Parts tested
				StdfFile.WriteDword(0);						// Parts retested
				StdfFile.WriteDword(0);						// Parts Aborted
				StdfFile.WriteDword(iTotalGoodBin);			// Good Parts
				StdfFile.WriteDword((DWORD)-1);				// Functionnal Parts
				StdfFile.WriteString(m_strWaferID.toLatin1().constData());	// WaferID
				StdfFile.WriteString(m_strFabId.toLatin1().constData());	// FabID
				StdfFile.WriteRecord();

				iPartNumber = iTotalFailBin = iTotalGoodBin = 0;
			}

			m_strWaferID = strWaferID;

			// Write WIR of new Wafer.
			RecordReadInfo.iRecordType = 2;
			RecordReadInfo.iRecordSubType = 10;
			StdfFile.WriteHeader(&RecordReadInfo);
			StdfFile.WriteByte(1);								// Test head
			StdfFile.WriteByte(255);							// Tester site (all)
			StdfFile.WriteDword(lStartTime);							// Start time
			StdfFile.WriteString(m_strWaferID.toLatin1().constData());	// WaferID
			StdfFile.WriteRecord();
		}

		// Reset counters
		iTotalTests = 0;

		// Write PIR for parts in this Wafer site
		RecordReadInfo.iRecordType = 5;
		RecordReadInfo.iRecordSubType = 10;
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(1);					// Test head
		StdfFile.WriteByte(iSite);				// Tester site
		StdfFile.WriteRecord();

		// Read Parameter results for this record
		for(;iIndex<lstSections.count();iIndex++)
		{
			if(iIndex >= m_iTotalParameters)
				break;

			strSection = lstSections[iIndex];
			if(strSection.isEmpty())
				continue;

			if(m_pEtestFab7Parameter[iIndex].strName == "SITE_X")
				continue;

			if(m_pEtestFab7Parameter[iIndex].strName == "SITE_Y")
				continue;

			fValue = strSection.toFloat();

			// Valid test result...write the PTR
			iTotalTests++;

			RecordReadInfo.iRecordType = 15;
			RecordReadInfo.iRecordSubType = 10;
			StdfFile.WriteHeader(&RecordReadInfo);
			// Compute Test# (add user-defined offset)
			StdfFile.WriteDword(m_pEtestFab7Parameter[iIndex].nNumber + GEX_TESTNBR_OFFSET_ETEST_FAB7);			// Test Number
			StdfFile.WriteByte(1);						// Test head
			StdfFile.WriteByte(iSite);			// Tester site#

			if(!m_pEtestFab7Parameter[iIndex].bValidLowLimit && !m_pEtestFab7Parameter[iIndex].bValidHighLimit)
			{
				// No pass/fail information
				bData = 0x40;
			}
			else if(((m_pEtestFab7Parameter[iIndex].bValidLowLimit==true) && (fValue < m_pEtestFab7Parameter[iIndex].fLowLimit)) ||
			   ((m_pEtestFab7Parameter[iIndex].bValidHighLimit==true) && (fValue > m_pEtestFab7Parameter[iIndex].fHighLimit)))
			{
				bData = 0200;	// Test Failed
			}
			else
			{
				bData = 0;		// Test passed
			}
			StdfFile.WriteByte(bData);							// TEST_FLG
			StdfFile.WriteByte(0x40|0x80);						// PARAM_FLG
			StdfFile.WriteFloat(fValue * GS_POW(10.0,m_pEtestFab7Parameter[iIndex].nScale));						// Test result
			StdfFile.WriteString(m_pEtestFab7Parameter[iIndex].strName.toLatin1().constData());	// TEST_TXT
			if(m_pEtestFab7Parameter[iIndex].bStaticHeaderWritten == false)
			{
				StdfFile.WriteString("");							// ALARM_ID
				bData = 2;	// Valid data.
				if(m_pEtestFab7Parameter[iIndex].bValidLowLimit==false)
					bData |=0x40;
				if(m_pEtestFab7Parameter[iIndex].bValidHighLimit==false)
					bData |=0x80;
				StdfFile.WriteByte(bData);							// OPT_FLAG
				StdfFile.WriteByte(-m_pEtestFab7Parameter[iIndex].nScale);			// RES_SCALE
				StdfFile.WriteByte(-m_pEtestFab7Parameter[iIndex].nScale);			// LLM_SCALE
				StdfFile.WriteByte(-m_pEtestFab7Parameter[iIndex].nScale);			// HLM_SCALE
				StdfFile.WriteFloat(m_pEtestFab7Parameter[iIndex].fLowLimit * GS_POW(10.0,m_pEtestFab7Parameter[iIndex].nScale));		// LOW Limit
				StdfFile.WriteFloat(m_pEtestFab7Parameter[iIndex].fHighLimit * GS_POW(10.0,m_pEtestFab7Parameter[iIndex].nScale));		// HIGH Limit
				StdfFile.WriteString(m_pEtestFab7Parameter[iIndex].strUnits.toLatin1().constData());	// Units
				m_pEtestFab7Parameter[iIndex].bStaticHeaderWritten = true;
			}
			StdfFile.WriteRecord();
		}		// Read all results on line

		m_nTotalParts++;
		iPartNumber++;

		// Write PRR
		RecordReadInfo.iRecordType = 5;
		RecordReadInfo.iRecordSubType = 20;
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(1);			// Test head
		StdfFile.WriteByte(iSite);// Tester site#:1
		if(bPassStatus == true)
		{
			StdfFile.WriteByte(0);				// PART_FLG : PASSED
			iTotalGoodBin++;
			iBin = 1;

			m_nPassParts++;
		}
		else
		{
			StdfFile.WriteByte(8);				// PART_FLG : FAILED
			iTotalFailBin++;
			iBin = 0;

			m_nFailParts++;
		}
		StdfFile.WriteWord((WORD)iTotalTests);	// NUM_TEST
		StdfFile.WriteWord(iBin);				// HARD_BIN
		StdfFile.WriteWord(iBin);				// SOFT_BIN
		StdfFile.WriteWord(iXpos);				// X_COORD
		StdfFile.WriteWord(iYpos);				// Y_COORD
		StdfFile.WriteDword(0);				// testing time
		sprintf(szString,"%ld",iPartNumber);
		StdfFile.WriteString(szString);		// PART_ID
		StdfFile.WriteString("");			// PART_TXT
		StdfFile.WriteString("");			// PART_FIX
		StdfFile.WriteRecord();
	};			// Read all lines with valid data records in file

	// Write WRR for last wafer inserted
	RecordReadInfo.iRecordType = 2;
	RecordReadInfo.iRecordSubType = 20;
	StdfFile.WriteHeader(&RecordReadInfo);
	StdfFile.WriteByte(1);						// Test head
	StdfFile.WriteByte(255);					// Tester site (all)
	StdfFile.WriteDword(m_lStopTime);			// Time of last part tested
	StdfFile.WriteDword(iTotalGoodBin+iTotalFailBin);	// Parts tested
	StdfFile.WriteDword(0);						// Parts retested
	StdfFile.WriteDword(0);						// Parts Aborted
	StdfFile.WriteDword(iTotalGoodBin);			// Good Parts
	StdfFile.WriteDword((DWORD)-1);				// Functionnal Parts
	StdfFile.WriteString(m_strWaferID.toLatin1().constData());	// WaferID
	StdfFile.WriteString("");					// FabId
	StdfFile.WriteString("");					// FrameId
	StdfFile.WriteString("");					// MaskId
	StdfFile.WriteString("");					// UserDesc
	StdfFile.WriteRecord();


	RecordReadInfo.iRecordType = 1;
	RecordReadInfo.iRecordSubType = 40;
	StdfFile.WriteHeader(&RecordReadInfo);
	StdfFile.WriteByte(255);					// Test Head
	StdfFile.WriteByte(255);					// Test sites
	StdfFile.WriteWord(1);						// HBIN
	StdfFile.WriteDword(m_nPassParts);			// Total Bins
	StdfFile.WriteByte('P');
	StdfFile.WriteString("");
	StdfFile.WriteRecord();

	StdfFile.WriteHeader(&RecordReadInfo);
	StdfFile.WriteByte(255);					// Test Head
	StdfFile.WriteByte(255);					// Test sites
	StdfFile.WriteWord(0);						// HBIN
	StdfFile.WriteDword(m_nFailParts);			// Total Bins
	StdfFile.WriteByte('F');
	StdfFile.WriteString("");
	StdfFile.WriteRecord();


	RecordReadInfo.iRecordType = 1;
	RecordReadInfo.iRecordSubType = 50;
	// Write SBR/site
	StdfFile.WriteHeader(&RecordReadInfo);
	StdfFile.WriteByte(255);					// Test Head
	StdfFile.WriteByte(255);					// Test sites
	StdfFile.WriteWord(1);						// SBIN
	StdfFile.WriteDword(m_nPassParts);			// Total Bins
	StdfFile.WriteByte('P');
	StdfFile.WriteString("");
	StdfFile.WriteRecord();

	// Write SBR/site
	StdfFile.WriteHeader(&RecordReadInfo);
	StdfFile.WriteByte(255);					// Test Head
	StdfFile.WriteByte(255);					// Test sites
	StdfFile.WriteWord(0);						// SBIN
	StdfFile.WriteDword(m_nFailParts);			// Total Bins
	StdfFile.WriteByte('F');
	StdfFile.WriteString("");
	StdfFile.WriteRecord();


	// Write PCR
	RecordReadInfo.iRecordType = 1;
	RecordReadInfo.iRecordSubType = 30;
	StdfFile.WriteHeader(&RecordReadInfo);
	StdfFile.WriteByte(255);					// Test Head = ALL
	StdfFile.WriteByte(255);					// Test sites = ALL
	StdfFile.WriteDword(m_nTotalParts);			// Total Parts tested
	StdfFile.WriteDword(0);						// Total Parts re-tested
	StdfFile.WriteDword(0);						// Total Parts aborted
	StdfFile.WriteDword(m_nPassParts);			// Total GOOD Parts
	StdfFile.WriteRecord();

	// Write MRR
	RecordReadInfo.iRecordType = 1; 
	RecordReadInfo.iRecordSubType = 20;
	StdfFile.WriteHeader(&RecordReadInfo);
	StdfFile.WriteDword(m_lStopTime);			// File finish-time.
	StdfFile.WriteRecord();

	// Close STDF file.
	StdfFile.Close();

	// Success
	return true;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' EtestFab7 file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGEtestFab7toSTDF::Convert(const char *EtestFab7FileName, const char *strFileNameSTDF)
{
	// No erro (default)
	iLastError = errNoError;

	// If STDF file already exists...do not rebuild it...unless dates not matching!
	QFileInfo fInput(EtestFab7FileName);
	QFileInfo fOutput(strFileNameSTDF);

    QFile f( strFileNameSTDF );
    if((f.exists() == true) && (fInput.lastModified() < fOutput.lastModified()))
		return true;

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	bool bHideProgressAfter=true;
	bool bHideLabelAfter=false;
	if(GexProgressBar != NULL)
	{
		bHideProgressAfter = GexProgressBar->isHidden();
		GexProgressBar->setMaximum(100);
		GexProgressBar->setTextVisible(true);
		GexProgressBar->setValue(0);
		GexProgressBar->show();
	}

	if(GexScriptStatusLabel != NULL)
	{
		if(GexScriptStatusLabel->isHidden())
		{
			bHideLabelAfter = true;
			GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(EtestFab7FileName).fileName()+"...");
			GexScriptStatusLabel->show();
		}
		GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(EtestFab7FileName).fileName()+"...");
		GexScriptStatusLabel->show();
	}
    QCoreApplication::processEvents();

    if(ReadEtestFab7File(EtestFab7FileName,strFileNameSTDF) != true)
	{
		//////////////////////////////////////////////////////////////////////
		// For ProgressBar
		if((GexProgressBar != NULL)
		&& bHideProgressAfter)
			GexProgressBar->hide();
		
		if((GexScriptStatusLabel != NULL)
		&& bHideLabelAfter)
			GexScriptStatusLabel->hide();
		return false;	// Error reading EtestFab7 file
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
// Normalize test limits when writing into PTR.
//////////////////////////////////////////////////////////////////////
void CGEtestFab7toSTDF::NormalizeLimits(QString &strUnit, int &nScale)
{
	nScale = 0;
	if(strUnit.length() <= 1)
	{
		// units too short to include a prefix, then keep it 'as-is'
		return;
	}

	QChar cPrefix = strUnit[0];
	switch(cPrefix.toLatin1())
	{
		case 'm': // Milli
			nScale = -3;
			break;
		case 'u': // Micro
			nScale = -6;
			break;
		case 'n': // Nano
			nScale = -9;
			break;
		case 'p': // Pico
			nScale = -12;
			break;
		case 'f': // Fento
			nScale = -15;
			break;
		case 'K': // Kilo
			nScale = 3;
			break;
		case 'M': // Mega
			nScale = 6;
			break;
		case 'G': // Giga
			nScale = 9;
			break;
		case 'T': // Tera
			nScale = 12;
			break;
	}
	if(nScale)
		strUnit = strUnit.mid(1);	// Take all characters after the prefix.
}

//////////////////////////////////////////////////////////////////////
// Read line : skip empty line
//////////////////////////////////////////////////////////////////////
QString CGEtestFab7toSTDF::ReadLine(QTextStream& hFile)
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
