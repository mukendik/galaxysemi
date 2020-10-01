//////////////////////////////////////////////////////////////////////
// import_amida.cpp: Convert a .AMIDA file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#include "gqtl_global.h"
#include <qmath.h>
#include <time.h>

#ifdef _WIN32
#include "windows.h"		// For 'GetWindowsDirectoryA' declaration
#endif

#include <qfileinfo.h>
#include <qmap.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>

#include "engine.h"
#include "import_amida.h"
#include "import_constants.h"

//Test Program: 	(A555) On431.tps
//Device Name: 	TLV431ALP (FT)
//Operator: 	DemoOp
//LOT ID: 	AATI-GPIB-20100318
//Start Time: 	03/19/2010, 02:55:16 PM
//Fail #: 	0
//Pass #: 	100
//Total #: 	100
//
//Name	Bin#	X	Y	continuity1	continuity2	Ik_min	Vref	Iref	Ikoff	Zka
//Unit	N	N	N	V	V	uA	V	nA	nA	ohm
//Max	16	0	0	-0.2	-0.2	80	1.252	300	40	0.4
//Min	1	0	0	-1	-1	0	1.228	0	0	0
//1	1	0	0	-0.56000	-0.61000	32.00000	1.24800	80.20000	22.00000	0.31000
//2	1	0	0	-0.56000	-0.61000	32.00000	1.24800	80.20000	22.00000	0.31000
//

// OR
//Name	Site#	Bin#	X	Y	Isink1_os	Isink2_os	Isink3_os	Isink4_os	Isink5_os
//Unit	N	N	N	N	V	V	V	V	V	V	V	V	V	V	V	V	V	V	V	uA
//Max	4	16	0	0	-0.3	-0.3	-0.3	-0.3	-0.3	-0.3	-0.3	-0.3
//Min	1	1	0	0	-0.9	-0.9	-0.9	-0.9	-0.9	-0.9	-0.9	-0.9
//1	1	4	4	0	-0.53945	-0.53239	-0.53696	-0.54442	-0.54722	-0.53190
//2	2	13	5	0	-0.50365	-0.50458	-0.51175	-0.51074	-0.50190	-0.50486

// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar

#define TEST_PASSFLAG_UNKNOWN 0
#define TEST_PASSFLAG_PASS 1
#define TEST_PASSFLAG_FAIL 2


#define BIT0			0x01
#define BIT1			0x02
#define BIT2			0x04
#define BIT3			0x08
#define BIT4			0x10
#define BIT5			0x20
#define BIT6			0x40
#define BIT7			0x80

CGAmidaParameter::CGAmidaParameter()
{	
	m_nScale = 0;
	m_fLowLimit = m_fHighLimit = 0;
	m_bValidLowLimit = m_bValidHighLimit = m_bStaticHeaderWritten = false;
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGAmidatoSTDF::CGAmidatoSTDF()
{
	// Default: AMIDA parameter list on disk includes all known AMIDA parameters...
	m_bNewAmidaParameterFound = false;
	m_lStartTime = 0;
	m_bValidXYpos = false;
	m_nDataOffset = 2;
	m_nWaferId = 1;

	m_nGoodParts = 0;
	m_nFailParts = 0;
	m_nTotalParts = 0;

	m_pParameterList = NULL;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGAmidatoSTDF::~CGAmidatoSTDF()
{
	if(m_pParameterList)
		delete [] m_pParameterList;

	// Destroy list of Bin tables.
	QMap<int,CGAmidaBinning *>::Iterator it;
	for(it = m_mapAmidaBinning.begin(); it!= m_mapAmidaBinning.end(); it++)
		delete m_mapAmidaBinning[it.key()];
	m_mapAmidaBinning.clear();
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGAmidatoSTDF::GetLastError()
{
	m_strLastError = "Import Amida data : ";

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
			m_strLastError += "Invalid data file format";
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
// Load AMIDA Parameter table from DISK
//////////////////////////////////////////////////////////////////////
void CGAmidatoSTDF::LoadParameterIndexTable(void)
{
	QString	strAmidaTableFile;
	QString	strString;

    strAmidaTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strAmidaTableFile += GEX_AMIDA_PARAMETERS;

	// Open AMIDA Parameter table file
	QFile f( strAmidaTableFile );
    if(!f.open( QIODevice::ReadOnly ))
		return;

	// Assign file I/O stream
	QTextStream hAmidaTableFile(&f);

	// Skip comment or empty lines
	do
	{
	  strString = hAmidaTableFile.readLine();
	}
    while((strString.indexOf("----------------------") < 0) && (!hAmidaTableFile.atEnd()));

	// Read lines
	m_pFullAmidaParametersList.clear();
	strString = hAmidaTableFile.readLine();
	while (strString.isNull() == false)
	{
		// Save Parameter name in list
		m_pFullAmidaParametersList.append(strString);
		// Read next line
		strString = hAmidaTableFile.readLine();
	};

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// Save AMIDA Parameter table to DISK
//////////////////////////////////////////////////////////////////////
void CGAmidatoSTDF::DumpParameterIndexTable(void)
{
	QString		strAmidaTableFile;
	QString		strString;

    strAmidaTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strAmidaTableFile += GEX_AMIDA_PARAMETERS;

	// Open AMIDA Parameter table file
	QFile f( strAmidaTableFile );
    if(!f.open( QIODevice::WriteOnly ))
		return;

	// Assign file I/O stream
	QTextStream hAmidaTableFile(&f);

	// First few lines are comments:
	hAmidaTableFile << "############################################################" << endl;
	hAmidaTableFile << "# DO NOT EDIT THIS FILE!" << endl;
    hAmidaTableFile << "# Quantix Examinator: AMIDA Parameters detected" << endl;
	hAmidaTableFile << "# www.mentor.com" << endl;
    hAmidaTableFile << "# Quantix Examinator reads and writes into this file..." << endl;
	hAmidaTableFile << "-----------------------------------------------------------" << endl;

	// Write lines
	// m_pFullAmidaParametersList.sort();
	for(int nIndex = 0; nIndex < m_pFullAmidaParametersList.size(); nIndex++)
	{
		// Write line
		hAmidaTableFile << m_pFullAmidaParametersList[nIndex] << endl;
	};

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// If Examinator doesn't have this AMIDA parameter in his dictionnary, have it added.
//////////////////////////////////////////////////////////////////////
void CGAmidatoSTDF::UpdateParameterIndexTable(QString strParamName)
{
	// Check if the table is empty...if so, load it from disk first!
	if(m_pFullAmidaParametersList.isEmpty() == true)
	{
		// Load AMIDA parameter table from disk...
		LoadParameterIndexTable();
	}
	
	// Check if Parameter name already in table...if not, add it to the list
	// the new full list will be dumped to the disk at the end.
    if(m_pFullAmidaParametersList.indexOf(strParamName) < 0)
	{
		// Update list
		m_pFullAmidaParametersList.append(strParamName);

		// Set flag to force the current AMIDA table to be updated on disk
		m_bNewAmidaParameterFound = true;
	}
}


//////////////////////////////////////////////////////////////////////
// Save PCM parameter High or Low Limit result...
//////////////////////////////////////////////////////////////////////
void CGAmidatoSTDF::SaveParameter(int iIndex,QString &strName,QString &strUnit,QString &strLowLimit, QString &strHighLimit)
{
	if(strName.isEmpty())
		return;	// Ignore empty entry!

	if(iIndex > m_iTotalParameters)
		return;
	// Get Parameter pointer...save limits in ALL WaferEntry.

	CGAmidaParameter *ptParam = &m_pParameterList[iIndex];
	ptParam->m_strName = strName;
	ptParam->m_nTestNumber = m_pFullAmidaParametersList.indexOf(strName);
	ptParam->m_strUnit = strUnit;
	NormalizeLimits(ptParam->m_strUnit, ptParam->m_nScale);

	float fLimit;
	fLimit = strLowLimit.toFloat(&ptParam->m_bValidLowLimit);
	ptParam->m_fLowLimit = fLimit * GS_POW(10.0,ptParam->m_nScale);

	fLimit = strHighLimit.toFloat(&ptParam->m_bValidHighLimit);
	ptParam->m_fHighLimit = fLimit  * GS_POW(10.0,ptParam->m_nScale);

}

//////////////////////////////////////////////////////////////////////
//return lDateTime from string strDateTime "03/19/2010, 02:55:16 PM".
//////////////////////////////////////////////////////////////////////
long CGAmidatoSTDF::GetDateTimeFromString(QString strDateTime)
{
	int nYear, nMonth, nDay;
	int	nHour, nMin, nSec;
	long lDateTime;

	if(strDateTime.length()<20)
		return 0;
	
	QString strDate = strDateTime.left(10);
	QString strTime = strDateTime.section(" ",1);

	nMonth = strDate.mid(0,2).toInt();
	nDay = strDate.mid(3,2).toInt();
	nYear = strDate.mid(6,4).toInt();
	nHour = strTime.mid(0,2).toInt();
	nMin= strTime.mid(3,2).toInt();
	nSec = strTime.mid(6,2).toInt();

	if(strTime.endsWith("PM",Qt::CaseInsensitive))
		nHour += 12;

	QDate clDate(nYear,nMonth,nDay);
	QTime clTime(nHour,nMin,nSec);
	QDateTime clDateTime(clDate,clTime);

	clDateTime.setTimeSpec(Qt::UTC);
	lDateTime = clDateTime.toTime_t();
	return lDateTime;
}

//////////////////////////////////////////////////////////////////////
// Normalize test limits when writing into PTR.
//////////////////////////////////////////////////////////////////////
void CGAmidatoSTDF::NormalizeLimits(QString &strUnit, int &nScale)
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
// Check if File is compatible with AMIDA format
//////////////////////////////////////////////////////////////////////
bool CGAmidatoSTDF::IsCompatible(const char *szFileName)
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
	QTextStream hAmidaFile(&f);
	QStringList lstSections;

	// Goto data line
	//Test Program: 	(A555) On431.tps
	//Device Name: 	TLV431ALP (FT)
	//Operator: 	DemoOp
	//LOT ID: 	AATI-GPIB-20100318
	//Start Time: 	03/19/2010, 02:55:16 PM
	//Fail #: 	0
	//Pass #: 	100
	//Total #: 	100
	//
	//Name	Bin#	X	Y	continuity1	continuity2	Ik_min	Vref	Iref	Ikoff	Zka
	//Unit	N	N	N	V	V	uA	V	nA	nA	ohm
	//Max	16	0	0	-0.2	-0.2	80	1.252	300	40	0.4
	//Min	1	0	0	-1	-1	0	1.228	0	0	0
	//1	1	0	0	-0.56000	-0.61000	32.00000	1.24800	80.20000	22.00000	0.31000
	//2	1	0	0	-0.56000	-0.61000	32.00000	1.24800	80.20000	22.00000	0.31000

	int nLine = 0;
	while(true)
	{
		strString = hAmidaFile.readLine().simplified();
		if(strString.startsWith("Name Bin# ",Qt::CaseInsensitive))
			break;
		if(strString.startsWith("Name Site# Bin#",Qt::CaseInsensitive))
			break;
		nLine++;
		if(nLine > 15)
			break;
	}

	//Name	Bin#	X	Y	continuity1	continuity2	Ik_min	Vref	Iref	Ikoff	Zka
	if(!strString.startsWith("Name ",Qt::CaseInsensitive))
	{
		// Incorrect header...this is not a AMIDA file!
		// Close file
		f.close();
		return false;
	}

	//Unit	N	N	N	V	V	uA	V	nA	nA	ohm
	strString = hAmidaFile.readLine().simplified();
	if(!strString.startsWith("Unit N ",Qt::CaseInsensitive))
	{
		// Incorrect header...this is not a AMIDA file!
		// Close file
		f.close();
		return false;
	}

	//Max	16	0	0	-0.2	-0.2	80	1.252	300	40	0.4
	strString = hAmidaFile.readLine().simplified();
	if(!strString.startsWith("Max ",Qt::CaseInsensitive))
	{
		// Incorrect header...this is not a AMIDA file!
		// Close file
		f.close();
		return false;
	}

	//Min	1	0	0	-1	-1	0	1.228	0	0	0
	strString = hAmidaFile.readLine().simplified();
	if(!strString.startsWith("Min ",Qt::CaseInsensitive))
	{
		// Incorrect header...this is not a AMIDA file!
		// Close file
		f.close();
		return false;
	}

	// Close file
	f.close();

	return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the AMIDA file
//////////////////////////////////////////////////////////////////////
bool CGAmidatoSTDF::ReadAmidaFile(const char *AmidaFileName,const char *strFileNameSTDF)
{
	QString strString;
	QString strSection;
	QString strValue;
	QString	strSite;

	// Open AMIDA file
	QFile f( AmidaFileName );
    if(!f.open( QIODevice::ReadOnly ))
	{
		// Failed Opening AMIDA file
		m_iLastError = errOpenFail;
		
		// Convertion failed.
		return false;
	}
	// Assign file I/O stream
	QTextStream hAmidaFile(&f);
	QStringList lstSections;

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	iFileSize = f.size() + 1;

	//Test Program: 	(A555) On431.tps
	//Device Name: 	TLV431ALP (FT)
	//Operator: 	DemoOp
	//LOT ID: 	AATI-GPIB-20100318
	//Start Time: 	03/19/2010, 02:55:16 PM
	//Fail #: 	0
	//Pass #: 	100
	//Total #: 	100
	//
	//Name	Bin#	X	Y	continuity1	continuity2	Ik_min	Vref	Iref	Ikoff	Zka
	//Unit	N	N	N	V	V	uA	V	nA	nA	ohm
	//Max	16	0	0	-0.2	-0.2	80	1.252	300	40	0.4
	//Min	1	0	0	-1	-1	0	1.228	0	0	0
	//1	1	0	0	-0.56000	-0.61000	32.00000	1.24800	80.20000	22.00000	0.31000
	//2	1	0	0	-0.56000	-0.61000	32.00000	1.24800	80.20000	22.00000	0.31000

	while(hAmidaFile.atEnd() == false)
	{
		strString = ReadLine(hAmidaFile);
		if(strString.simplified().isEmpty())
			continue;
		if(strString.simplified().startsWith("Name Bin#",Qt::CaseInsensitive))
			break;
		if(strString.simplified().startsWith("Name Site# Bin#",Qt::CaseInsensitive))
			break;

		strSection = strString.section(":",0,0).simplified().toUpper();
		strValue = strString.section(":",1).simplified();

		if(strSection=="LOT ID")
		{
			m_strLotId = strValue.simplified();
		}
		else
		if(strSection=="DEVICE NAME")
		{
			m_strProductId = strValue.simplified();
		}
		else
		if(strSection=="OPERATOR")
		{
			m_strOperatorId = strValue.simplified();
		}
		else
		if(strSection=="TEST PROGRAM")
		{
			m_strProgramId = strValue.simplified();
		}
		else
		if(strSection=="FAIL #")
		{
			m_nFailParts = strValue.toInt();
		}
		else
		if(strSection=="PASS #")
		{
			m_nGoodParts = strValue.toInt();
		}
		else
		if(strSection=="TOTAL #")
		{
			m_nTotalParts = strValue.toInt();
		}
		else
		if(strSection=="START TIME")
		{				
			m_lStartTime = GetDateTimeFromString(strValue);
		}
		else
		{
			// Incorrect header...this is not a PCM_MAGNACHIP_TYPE_2 file!
			m_iLastError = errInvalidFormat;

			// Convertion failed.
			// Close file
			f.close();
			return false;
		}
	}


	if(!strString.simplified().startsWith("Name ",Qt::CaseInsensitive))
	{
		// Incorrect header...this is not a AMIDA file!
		m_iLastError = errInvalidFormat;
		// Close file
		f.close();
		return false;
	}

	//Name	Bin#	X	Y	continuity1	continuity2
	m_nDataOffset = 2;
	m_strTesterType = "TTL Handler";
	m_bValidSiteNum = (strString.simplified().toUpper().indexOf(" SITE# ") > 0);
	if(m_bValidSiteNum)
	{
		m_nDataOffset += 1;
	}
	m_bValidXYpos = (strString.simplified().toUpper().indexOf(" X Y ") > 0);
	if(m_bValidXYpos)
	{
		m_nDataOffset += 2;
		m_strTesterType = "GPIB Prober";
	}

	// Read parameters information
	QString strTestNames = strString;
    QStringList lstTestNames = strTestNames.split("\t", QString::KeepEmptyParts);
	QString strUnits = ReadLine(hAmidaFile);
    QStringList lstUnits = strUnits.split("\t", QString::KeepEmptyParts);
	QString strHighLimits = ReadLine(hAmidaFile);
    QStringList lstHighLimits = strHighLimits.split("\t", QString::KeepEmptyParts);
	QString strLowLimits = ReadLine(hAmidaFile);
    QStringList lstLowLimits = strLowLimits.split("\t", QString::KeepEmptyParts);

	// Allocate the buffer to hold the N parameters & results.
	m_iTotalParameters = lstTestNames.size();
	m_pParameterList = new CGAmidaParameter[m_iTotalParameters];	// List of parameters
	int iIndex;
	for(iIndex=m_nDataOffset;iIndex<lstTestNames.size();iIndex++)
	{
		strTestNames = lstTestNames[iIndex].simplified();
		if(strTestNames.isEmpty())
			break;

		strUnits = "";
		if(lstUnits.size() > iIndex)
			strUnits = lstUnits[iIndex].simplified();
		if(strUnits.toUpper() == "N")
			strUnits = "";

		strLowLimits = "";
		if(lstLowLimits.size() > iIndex)
			strLowLimits = lstLowLimits[iIndex].simplified();

		strHighLimits = "";
		if(lstHighLimits.size() > iIndex)
			strHighLimits = lstHighLimits[iIndex].simplified();

		UpdateParameterIndexTable(strTestNames);

		SaveParameter(	iIndex,
						strTestNames,
						strUnits,
						strLowLimits,
						strHighLimits);

	}
	m_iTotalParameters = iIndex;

	if(!WriteStdfFile(&hAmidaFile,strFileNameSTDF))
	{
		QFile::remove(strFileNameSTDF);
		// Close file
		f.close();
		return false;
	}
	
	// Close file
	f.close();
	
	// All AMIDA file read...check if need to update the AMIDA Parameter list on disk?
	if(m_bNewAmidaParameterFound == true)
		DumpParameterIndexTable();

	// Success parsing AMIDA file
	return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from AMIDA data parsed
//////////////////////////////////////////////////////////////////////
bool CGAmidatoSTDF::WriteStdfFile(QTextStream *hAmidaFile,const char *strFileNameSTDF)
{
	// now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open((char*)strFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
	{
		// Failed importing AMIDA file into STDF database
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
	StdfFile.WriteString(m_strProgramId.toLatin1().constData());	// Job name
	StdfFile.WriteString("");					// Job rev
	StdfFile.WriteString("");					// sublot-id
	StdfFile.WriteString(m_strOperatorId.toLatin1().constData());	// operator
	StdfFile.WriteString("");					// exec-type
	StdfFile.WriteString("");					// exe-ver
	StdfFile.WriteString("");					// test-cod
	StdfFile.WriteString("");					// test-temperature
	// Construct custom Galaxy USER_TXT 
	QString	strUserTxt;
	strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;	
	strUserTxt += ":";
	strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
	strUserTxt += ":AMIDA";
	StdfFile.WriteString((char *)strUserTxt.toLatin1().constData());	// user-txt
	StdfFile.WriteString("");					// aux-file
	StdfFile.WriteString("");					// package-type
	StdfFile.WriteString(m_strProductId.toLatin1().constData());	// familyID
	StdfFile.WriteString("");					// Date-code
	StdfFile.WriteString("");					// Facility-ID
	StdfFile.WriteString("");					// FloorID
	StdfFile.WriteString("");					// ProcessID

	StdfFile.WriteRecord();

	// Write Test results for each line read.
	QString		strString;
	WORD		wSoftBin,wHardBin;
	int			iGoodParts;
	int			iTotalGoodBin,iTotalFailBin;
	int			iTotalTests,iPartNumber;
	bool		bPassStatus;
	BYTE		bData;

	int			iIndex;
	int			iXWafer, iYWafer;
	int			iSiteNumber;
	int			iTestNumber;
	bool		bTestPass;
	bool		bStopOnFail;
	float		fValue;

	int			nIndexHeader = 0;

	CGAmidaBinning *pBinning;

	// Reset counters
	iGoodParts=iTotalGoodBin=iTotalFailBin=0;
	iPartNumber=0;
	iXWafer = iYWafer = -32768;

	// Write the WIR
	if(m_bValidXYpos)
	{
		// Write WIR
		RecordReadInfo.iRecordType = 2;
		RecordReadInfo.iRecordSubType = 10;
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(1);						// Test head
		StdfFile.WriteByte(255);					// Tester site (all)
		StdfFile.WriteDword(m_lStartTime);			// Start time
		StdfFile.WriteString(QString::number(m_nWaferId).toLatin1().constData());	// WaferID
		StdfFile.WriteRecord();

	}
	// Write all Parameters read on this file : WIR,PIR,PTR...,PRR,WRR
	// Read AMIDA result
	QStringList lstSections;
	while(!hAmidaFile->atEnd())
	{
		strString = ReadLine(*hAmidaFile);

        lstSections = strString.split("\t", QString::KeepEmptyParts);
		if(lstSections.size() <= 3)
			continue;

		nIndexHeader = 0;
		iPartNumber = lstSections[nIndexHeader++].toInt(&bPassStatus);
		if(!bPassStatus)
			continue;
		iSiteNumber = 1;
		if(m_bValidSiteNum)
			iSiteNumber = lstSections[nIndexHeader++].toInt();
		wSoftBin = wHardBin = lstSections[nIndexHeader++].toInt();
		if(m_bValidXYpos)
		{
			iXWafer = lstSections[nIndexHeader++].toInt();
			iYWafer = lstSections[nIndexHeader++].toInt();
		}


		if(m_mapAmidaBinning.contains(wSoftBin))
			pBinning = m_mapAmidaBinning[wSoftBin];
		else
		{
			pBinning = new CGAmidaBinning();

			pBinning->nNumber = wSoftBin;
			pBinning->bPass = (wHardBin == 1);
			if(pBinning->bPass)
				pBinning->strName = "PASS";
			pBinning->nCount = 0;
			m_mapAmidaBinning[pBinning->nNumber] = pBinning;
		}

		pBinning->nCount++;

		// Write PIR
		// Write PIR for parts in this Wafer site
		RecordReadInfo.iRecordType = 5;
		RecordReadInfo.iRecordSubType = 10;
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(1);								// Test head
		StdfFile.WriteByte(iSiteNumber);					// Tester site
		StdfFile.WriteRecord();

		// Reset Pass/Fail flag.
		bPassStatus = (wHardBin == 1);

		// Reset counters
		iTotalTests = 0;
		bTestPass = true;
		bStopOnFail = false;

		for(iIndex=m_nDataOffset;iIndex<lstSections.size();iIndex++)
		{
			if(iIndex >= m_iTotalParameters)
				break;

			// Compute Test# (add user-defined offset)
			iTestNumber = GEX_TESTNBR_OFFSET_AMIDA + m_pParameterList[iIndex].m_nTestNumber;

			fValue = lstSections[iIndex].toFloat();
			if(!bStopOnFail)
			{
				iTotalTests++;
				bTestPass = true;
				if(m_pParameterList[iIndex].m_bValidLowLimit)
					bTestPass &= (m_pParameterList[iIndex].m_fLowLimit < fValue * GS_POW(10.0,m_pParameterList[iIndex].m_nScale));
				if(m_pParameterList[iIndex].m_bValidHighLimit)
					bTestPass &= (m_pParameterList[iIndex].m_fHighLimit > fValue * GS_POW(10.0,m_pParameterList[iIndex].m_nScale));
			}
			else
			if(m_pParameterList[iIndex].m_bStaticHeaderWritten)
				continue;

			// Write PTR
			RecordReadInfo.iRecordType = 15;
			RecordReadInfo.iRecordSubType = 10;
			
			StdfFile.WriteHeader(&RecordReadInfo);
			StdfFile.WriteDword(iTestNumber);			// Test Number
			StdfFile.WriteByte(1);						// Test head
			StdfFile.WriteByte(iSiteNumber);			// Tester site:1,2,3,4 or 5, etc.
			if(bTestPass)
				bData = 0;		// Test passed
			else
				bData = BIT7;	// Test Failed
			if(bStopOnFail)
			{
				// Test not executed
				bData = BIT1|BIT4;
			}
			StdfFile.WriteByte(bData);							// TEST_FLG
			bData = BIT6|BIT7;
			StdfFile.WriteByte(bData);							// PARAM_FLG
			StdfFile.WriteFloat(fValue * GS_POW(10.0,m_pParameterList[iIndex].m_nScale));		// Test result
			if(!m_pParameterList[iIndex].m_bStaticHeaderWritten)
			{
				m_pParameterList[iIndex].m_bStaticHeaderWritten = true;

				// save Parameter name
				StdfFile.WriteString(m_pParameterList[iIndex].m_strName.toLatin1().constData());	// TEST_TXT
				StdfFile.WriteString("");							// ALARM_ID

				bData = 2;	// Valid data.
				if(!m_pParameterList[iIndex].m_bValidLowLimit)
					bData |= BIT6;
				if(!m_pParameterList[iIndex].m_bValidHighLimit)
					bData |= BIT7;
				StdfFile.WriteByte(bData);							// OPT_FLAG

				StdfFile.WriteByte(-m_pParameterList[iIndex].m_nScale);	// RES_SCALE
				StdfFile.WriteByte(-m_pParameterList[iIndex].m_nScale);	// LLM_SCALE
				StdfFile.WriteByte(-m_pParameterList[iIndex].m_nScale);	// HLM_SCALE
				StdfFile.WriteFloat(m_pParameterList[iIndex].m_fLowLimit);	// LOW Limit
				StdfFile.WriteFloat(m_pParameterList[iIndex].m_fHighLimit);// HIGH Limit
				StdfFile.WriteString(m_pParameterList[iIndex].m_strUnit.toLatin1().constData());		// Units
			}
			StdfFile.WriteRecord();

			bStopOnFail |= !bTestPass;
		}

		// Write PRR
		RecordReadInfo.iRecordType = 5;
		RecordReadInfo.iRecordSubType = 20;
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(1);					// Test head
		StdfFile.WriteByte(iSiteNumber);		// Tester site:1,2,3,4 or 5
		if(bPassStatus == true)
		{
			StdfFile.WriteByte(0);				// PART_FLG : PASSED
			iTotalGoodBin++;
			iGoodParts++;
		}
		else
		{
			StdfFile.WriteByte(8);				// PART_FLG : FAILED
			iTotalFailBin++;
		}
		StdfFile.WriteWord((WORD)iTotalTests);	// NUM_TEST
		StdfFile.WriteWord(wHardBin);           // HARD_BIN
		StdfFile.WriteWord(wSoftBin);           // SOFT_BIN
		StdfFile.WriteWord(iXWafer);			// X_COORD
		StdfFile.WriteWord(iYWafer);			// Y_COORD
		StdfFile.WriteDword(0);					// No testing time known...
		StdfFile.WriteString(QString::number(iPartNumber).toLatin1().constData());// PART_ID
		StdfFile.WriteString("");				// PART_TXT
		StdfFile.WriteString("");				// PART_FIX
		StdfFile.WriteRecord();
	}


	if(m_bValidXYpos)
	{
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
		StdfFile.WriteDword((DWORD)(-1));			// Functionnal Parts
		StdfFile.WriteString(QString::number(m_nWaferId).toLatin1().constData());// WaferID
		StdfFile.WriteRecord();

	}

	QMap<int,CGAmidaBinning *>::Iterator it;

	RecordReadInfo.iRecordType = 1;
	RecordReadInfo.iRecordSubType = 40;
	for(it = m_mapAmidaBinning.begin(); it != m_mapAmidaBinning.end(); it++)
	{
		pBinning = *it;

		// Write HBR
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(255);						// Test Head = ALL
		StdfFile.WriteByte(255);						// Test sites = ALL
		StdfFile.WriteWord(pBinning->nNumber);			// HBIN
		StdfFile.WriteDword(pBinning->nCount);			// Total Bins
		if(pBinning->bPass)
			StdfFile.WriteByte('P');
		else
			StdfFile.WriteByte('F');
		StdfFile.WriteString( pBinning->strName.toLatin1().constData());
		StdfFile.WriteRecord();
	}

	RecordReadInfo.iRecordType = 1;
	RecordReadInfo.iRecordSubType = 50;
	for(it = m_mapAmidaBinning.begin(); it != m_mapAmidaBinning.end(); it++)
	{
		pBinning = *it;

		// Write SBR
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(255);						// Test Head = ALL
		StdfFile.WriteByte(255);						// Test sites = ALL
		StdfFile.WriteWord(pBinning->nNumber);			// HBIN
		StdfFile.WriteDword(pBinning->nCount);			// Total Bins
		if(pBinning->bPass)
			StdfFile.WriteByte('P');
		else
			StdfFile.WriteByte('F');
		StdfFile.WriteString( pBinning->strName.toLatin1().constData());
		StdfFile.WriteRecord();
	}


	if(m_nTotalParts == 0)
	{
		m_nTotalParts = iTotalGoodBin + iTotalFailBin;
		m_nGoodParts = iTotalGoodBin;
		m_nFailParts = iTotalFailBin;
	}

	// Write PCR
	RecordReadInfo.iRecordType = 1;
	RecordReadInfo.iRecordSubType = 30;
	StdfFile.WriteHeader(&RecordReadInfo);
	StdfFile.WriteByte(255);					// Test Head = ALL
	StdfFile.WriteByte(255);					// Test sites = ALL
	StdfFile.WriteDword(m_nTotalParts);			// Total Parts tested
	StdfFile.WriteDword(0);						// Total Parts re-tested
	StdfFile.WriteDword(0);						// Total Parts aborted
	StdfFile.WriteDword(m_nGoodParts);			// Total GOOD Parts
	StdfFile.WriteRecord();

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
// Convert 'FileName' AMIDA file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGAmidatoSTDF::Convert(const char *AmidaFileName, const char *strFileNameSTDF)
{
	// No erro (default)
	m_iLastError = errNoError;

	// If STDF file already exists...do not rebuild it...unless dates not matching!
	QFileInfo fInput(AmidaFileName);
	QFileInfo fOutput(strFileNameSTDF);

    QFile f( strFileNameSTDF );
    if((f.exists() == true) && (fInput.lastModified() < fOutput.lastModified()))
		return true;

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	iProgressStep = 0;
	iNextFilePos = 0;

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
			GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(AmidaFileName).fileName()+"...");
			GexScriptStatusLabel->show();
		}
		GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(AmidaFileName).fileName()+"...");
		GexScriptStatusLabel->show();
	}
    QCoreApplication::processEvents();


    if(ReadAmidaFile(AmidaFileName,strFileNameSTDF) != true)
	{
		//////////////////////////////////////////////////////////////////////
		// For ProgressBar
		if((GexProgressBar != NULL)
		&& bHideProgressAfter)
			GexProgressBar->hide();
		
		if((GexScriptStatusLabel != NULL)
		&& bHideLabelAfter)
			GexScriptStatusLabel->hide();
		return false;	// Error reading AMIDA file
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
QString CGAmidatoSTDF::ReadLine(QTextStream& hFile)
{
	QString strString;
		
	if(hFile.atEnd())
		return "";

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
	}
	while(!strString.isNull() && strString.isEmpty());

	return strString;

}
