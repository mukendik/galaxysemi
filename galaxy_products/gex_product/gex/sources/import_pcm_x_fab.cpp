//////////////////////////////////////////////////////////////////////
// import_pcm_x_fab.cpp: Convert a .PCM_X_FAB (TSMC) file to STDF V4.0
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
#include "import_pcm_x_fab.h"
#include "import_constants.h"

// File format:
//Lot,Wafer,Lot/Wf,Chip-Nbr,Lot/Wf/Ch,X-Coord.,Y-Coord.,M-Date,Device,VTO10X10N,VTO10X06N,
//
// OR
//
//Lot,Wafer,Lot/Wf,Chip-Nr,Lot/Wf/Ch,X-Coord.,Y-Coord.,M-Datum,Device,VTO10X10N,VTO10X06N,
//,,,,,,,,,V        *,V        *,uA/um    *,fF/um^2  *,um       *,um       *,uS/V     *,
//,,,,,,,,,7.50E-01,6.00E-01,4.20E+02,2.56E+00,4.50E-01,6.00E-01,1.00E+02,6.30E-01,0.00E+00,
//,,,,,,,,,9.50E-01,8.40E-01,5.80E+02,3.00E+00,6.50E-01,1.00E+00,1.40E+02,8.30E-01,3.00E+03,
//T05150,1,T05150/01,1,T05150/01/1,10,1,06.11.06 08:15:28,OMOZ964BFR,8.99E-01,7.85E-01,
//T05150,1,T05150/01,2,T05150/01/2,3,3,06.11.06 08:15:28,OMOZ964BFR,8.96E-01,8.01E-01,
//

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

CGPcmXFabParameter::CGPcmXFabParameter()
{	
	m_nScale = 0;
	m_fLowLimit = m_fHighLimit = 0;
	m_bValidLowLimit = m_bValidHighLimit = m_bStaticHeaderWritten = false;
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGPCM_X_FABtoSTDF::CGPCM_X_FABtoSTDF()
{
	// Default: PCM_X_FAB parameter list on disk includes all known PCM_X_FAB parameters...
	m_bNewPcmXFabParameterFound = false;
    m_lStartTime = 0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGPCM_X_FABtoSTDF::~CGPCM_X_FABtoSTDF()
{
	QMap<int,CGPcmXFabParameter*>::Iterator itParameter;
	for ( itParameter = m_pParameterList.begin(); itParameter != m_pParameterList.end(); ++itParameter ) 
	{
        delete itParameter.value();
	}
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGPCM_X_FABtoSTDF::GetLastError()
{
	m_strLastError = "Import X-FAB - PCM data : ";

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
// Load PCM_X_FAB Parameter table from DISK
//////////////////////////////////////////////////////////////////////
void CGPCM_X_FABtoSTDF::LoadParameterIndexTable(void)
{
	QString	strPcmXFabTableFile;
	QString	strString;

    strPcmXFabTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strPcmXFabTableFile += GEX_PCM_X_FAB_PARAMETERS;

	// Open PCM_X_FAB Parameter table file
    QFile f( strPcmXFabTableFile );
    if(!f.open( QIODevice::ReadOnly ))
		return;

	// Assign file I/O stream
	QTextStream hPcmXFabTableFile(&f);

	// Skip comment or empty lines
	do
	{
	  strString = hPcmXFabTableFile.readLine();
	}
    while((strString.indexOf("----------------------") < 0) && (!hPcmXFabTableFile.atEnd()));

	// Read lines
	m_pFullPcmXFabParametersList.clear();
	strString = hPcmXFabTableFile.readLine();
	while (strString.isNull() == false)
	{
		// Save Parameter name in list
		m_pFullPcmXFabParametersList.append(strString);
		// Read next line
		strString = hPcmXFabTableFile.readLine();
	};

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// Save PCM_X_FAB Parameter table to DISK
//////////////////////////////////////////////////////////////////////
void CGPCM_X_FABtoSTDF::DumpParameterIndexTable(void)
{
	QString		strPcmXFabTableFile;

    strPcmXFabTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strPcmXFabTableFile += GEX_PCM_X_FAB_PARAMETERS;

	// Open PCM_X_FAB Parameter table file
    QFile f( strPcmXFabTableFile );
    if(!f.open( QIODevice::WriteOnly ))
		return;

	// Assign file I/O stream
	QTextStream hPcmXFabTableFile(&f);

	// First few lines are comments:
	hPcmXFabTableFile << "############################################################" << endl;
	hPcmXFabTableFile << "# DO NOT EDIT THIS FILE!" << endl;
    hPcmXFabTableFile << "# Quantix Examinator: PCM_X_FAB Parameters detected" << endl;
	hPcmXFabTableFile << "# www.mentor.com" << endl;
    hPcmXFabTableFile << "# Quantix Examinator reads and writes into this file..." << endl;
	hPcmXFabTableFile << "-----------------------------------------------------------" << endl;

	// Write lines
	// m_pFullPcmXFabParametersList.sort();
	for(int nIndex = 0; nIndex < m_pFullPcmXFabParametersList.size(); nIndex++)
	{
		// Write line
		hPcmXFabTableFile << m_pFullPcmXFabParametersList[nIndex] << endl;
	};

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// If Examinator doesn't have this PCM_X_FAB parameter in his dictionnary, have it added.
//////////////////////////////////////////////////////////////////////
void CGPCM_X_FABtoSTDF::UpdateParameterIndexTable(QString strParamName)
{
	// Check if the table is empty...if so, load it from disk first!
	if(m_pFullPcmXFabParametersList.isEmpty() == true)
	{
		// Load PCM_X_FAB parameter table from disk...
		LoadParameterIndexTable();
	}
	
	// Check if Parameter name already in table...if not, add it to the list
	// the new full list will be dumped to the disk at the end.
    if(m_pFullPcmXFabParametersList.indexOf(strParamName) < 0)
	{
		// Update list
		m_pFullPcmXFabParametersList.append(strParamName);

		// Set flag to force the current PCM_X_FAB table to be updated on disk
		m_bNewPcmXFabParameterFound = true;
	}
}


//////////////////////////////////////////////////////////////////////
// Save PCM parameter High or Low Limit result...
//////////////////////////////////////////////////////////////////////
void CGPCM_X_FABtoSTDF::SaveParameter(int iIndex,
                                      QString strName,
                                      QString strUnit,
                                      const QString& fLowLimit,
                                      const QString& fHighLimit)
{
	if(strName.isEmpty())
		return;	// Ignore empty entry!

	CGPcmXFabParameter *ptParam;	// List of parameters

	// Get Parameter pointer...save limits in ALL WaferEntry.
	if(m_pParameterList.contains(iIndex))
	{
		ptParam = m_pParameterList[iIndex];
	}
	else
	{
		// save new parameter
		ptParam = new CGPcmXFabParameter();
		ptParam->m_strName = strName;
		m_pParameterList[iIndex] = ptParam;
	}

	ptParam->m_strUnit = strUnit;
	NormalizeLimits(ptParam->m_strUnit, ptParam->m_nScale);
    bool lOk;
    ptParam->m_fLowLimit = fLowLimit.toFloat(&lOk) * GS_POW(10.0,ptParam->m_nScale);
    if(lOk)
        ptParam->m_bValidLowLimit = true;		// High limit defined
    else
        ptParam->m_bValidLowLimit = false;
    ptParam->m_fHighLimit = fHighLimit.toFloat(&lOk)  * GS_POW(10.0,ptParam->m_nScale);
    if(lOk)
        ptParam->m_bValidHighLimit = true;		// Low limit defined
    else
        ptParam->m_bValidHighLimit = false;		// Low limit undefined

}

//////////////////////////////////////////////////////////////////////
//return lDateTime from string strDateTime "06.11.06 08:15:28".
//////////////////////////////////////////////////////////////////////
long CGPCM_X_FABtoSTDF::GetDateTimeFromString(QString strDateTime)
{
	int nYear, nMonth, nDay;
	int	nHour, nMin, nSec;
	long lDateTime;

	if(strDateTime.length()<17)
		return 0;
	
	nDay = strDateTime.mid(0,2).toInt();
	nMonth = strDateTime.mid(3,2).toInt();
	nYear = 2000 + strDateTime.mid(6,2).toInt();
	nHour = strDateTime.mid(9,2).toInt();
	nMin= strDateTime.mid(12,2).toInt();
	nSec = strDateTime.mid(15,2).toInt();

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
void CGPCM_X_FABtoSTDF::NormalizeLimits(QString &strUnit, int &nScale)
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
// Check if File is compatible with PCM_X_FAB format
//////////////////////////////////////////////////////////////////////
bool CGPCM_X_FABtoSTDF::IsCompatible(const char *szFileName)
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
	QTextStream hPcmXFabFile(&f);
	QStringList lstSections;

	// Check if first line is the correct PCM_X_FAB header...
	//Lot,Wafer,Lot/Wf,Chip-Nbr,Lot/Wf/Ch,X-Coord.,Y-Coord.,M-Date,Device,VTO10X10N,VTO10X06N,
	//
	// OR
	//
	//Lot,Wafer,Lot/Wf,Chip-Nr,Lot/Wf/Ch,X-Coord.,Y-Coord.,M-Datum,Device,VTO10X10N,VTO10X06N,
	//,,,,,,,,,V        *,V        *,uA/um    *,fF/um^2  *,um       *,um       *,uS/V     *,
	
	do
		strString = hPcmXFabFile.readLine().simplified();
	while(!strString.isNull() && strString.isEmpty());

    if(!strString.startsWith("Lot,Wafer,Lot/Wf,Chip-Nbr,Lot/Wf/Ch,X-Coord.,Y-Coord.,M-Date,Device", Qt::CaseInsensitive)
    && !strString.startsWith("Lot,Wafer,Lot/Wf,Chip-Nr,Lot/Wf/Ch,X-Coord.,Y-Coord.,M-Datum,Device", Qt::CaseInsensitive)
    && !strString.startsWith("Lot,Wafer,Lot/Wf,Chip-Nr,Lot/Wf/Ch,X-Coord.,Y-Coord.,M-Date,Device", Qt::CaseInsensitive)
    && !strString.startsWith("Lot,Wafer,Lot/Wf,Chip-Nbr,Lot/Wf/Ch,X-Coord.,Y-Coord.,M-Datum,Device", Qt::CaseInsensitive)
    )
	{
		// Incorrect header...this is not a PCM_X_FAB file!
		// Close file
		f.close();
		return false;
	}
	// Close file
	f.close();

	return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the PCM_X_FAB file
//////////////////////////////////////////////////////////////////////
bool CGPCM_X_FABtoSTDF::ReadPcmXFabFile(const char *PcmXFabFileName,const char *strFileNameSTDF)
{
	QString strString;
	QString strSection;
	QString strValue;
	QString	strSite;

	// Open PCM_X_FAB file
    QFile f( PcmXFabFileName );
    if(!f.open( QIODevice::ReadOnly ))
	{
		// Failed Opening PCM_X_FAB file
		m_iLastError = errOpenFail;
		
		// Convertion failed.
		return false;
	}
	// Assign file I/O stream
	QTextStream hPcmXFabFile(&f);
	QStringList lstSections;

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	iFileSize = f.size() + 1;

	// Check if first line is the correct PCM_X_FAB header...
	//Lot,Wafer,Lot/Wf,Chip-Nbr,Lot/Wf/Ch,X-Coord.,Y-Coord.,M-Date,Device,VTO10X10N,VTO10X06N,
	//
	// OR
	//
	//Lot,Wafer,Lot/Wf,Chip-Nr,Lot/Wf/Ch,X-Coord.,Y-Coord.,M-Datum,Device,VTO10X10N,VTO10X06N,
	//,,,,,,,,,V        *,V        *,uA/um    *,fF/um^2  *,um       *,um       *,uS/V     *,
	
	strString = ReadLine(hPcmXFabFile).simplified();
    if(!strString.startsWith("Lot,Wafer,Lot/Wf,Chip-Nbr,Lot/Wf/Ch,X-Coord.,Y-Coord.,M-Date,Device", Qt::CaseInsensitive)
    && !strString.startsWith("Lot,Wafer,Lot/Wf,Chip-Nr,Lot/Wf/Ch,X-Coord.,Y-Coord.,M-Datum,Device", Qt::CaseInsensitive)
    && !strString.startsWith("Lot,Wafer,Lot/Wf,Chip-Nr,Lot/Wf/Ch,X-Coord.,Y-Coord.,M-Date,Device", Qt::CaseInsensitive)
    && !strString.startsWith("Lot,Wafer,Lot/Wf,Chip-Nbr,Lot/Wf/Ch,X-Coord.,Y-Coord.,M-Datum,Device", Qt::CaseInsensitive)
    )
	{
		// Incorrect header...this is not a PCM_X_FAB file!
		m_iLastError = errInvalidFormat;
		
		// Convertion failed.
		// Close file
		f.close();
		return false;
	}
	// Good header
	// goto the first wafer information to find Lot_Id, Wafer ...
	// Skip 3 lines
	ReadLine(hPcmXFabFile);
	ReadLine(hPcmXFabFile);
	ReadLine(hPcmXFabFile);

	// Read PCM_X_FAB information
	strString = ReadLine(hPcmXFabFile);
    lstSections = strString.split(",", QString::KeepEmptyParts);

	// Lot Id
	m_strLotId = lstSections[0];
	// Wafer Id
	m_nWaferId = -1;
	// DateTime
	strString = "";
	if(lstSections.size() > 7)
		strString = lstSections[7];
    m_lStartTime = GetDateTimeFromString(strString.simplified().remove("\""));
	// Product Id
	if(lstSections.size() > 8)
	m_strProductId = lstSections[8];

	//Restart at the beggining of the file
	hPcmXFabFile.seek(0);

	if(!WriteStdfFile(&hPcmXFabFile,strFileNameSTDF))
	{
		QFile::remove(strFileNameSTDF);
		// Close file
		f.close();
		return false;
	}
	
	// Close file
	f.close();
	
	// All PCM_X_FAB file read...check if need to update the PCM_X_FAB Parameter list on disk?
	if(m_bNewPcmXFabParameterFound == true)
		DumpParameterIndexTable();

	// Success parsing PCM_X_FAB file
	return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from PCM_X_FAB data parsed
//////////////////////////////////////////////////////////////////////
bool CGPCM_X_FABtoSTDF::WriteStdfFile(QTextStream *hPcmXFabFile,const char *strFileNameSTDF)
{
	// now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open((char*)strFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
	{
		// Failed importing PCM_X_FAB file into STDF database
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

    // Write @
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
	strUserTxt += ":X-FAB";
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
	QString		strTestNames;
	QString		strUnits;
	QString		strLowLimits;
	QString		strHighLimits;
	float		fValue;

	// Reset counters
	iGoodParts=iTotalGoodBin=iTotalFailBin=0;
	iPartNumber=0;

	// Read parameters information
	strTestNames = ReadLine(*hPcmXFabFile);
    QStringList lstTestNames = strTestNames.split(",", QString::KeepEmptyParts);
	strUnits = ReadLine(*hPcmXFabFile);
    QStringList lstUnits = strUnits.split(",", QString::KeepEmptyParts);
	strLowLimits = ReadLine(*hPcmXFabFile);
    QStringList lstLowLimits = strLowLimits.split(",", QString::KeepEmptyParts);
	strHighLimits = ReadLine(*hPcmXFabFile);
    QStringList lstHighLimits = strHighLimits.split(",", QString::KeepEmptyParts);
	for(iIndex=9;iIndex<lstTestNames.size();iIndex++)
	{
		strUnits = "";
		if(lstUnits.size() > iIndex)
			strUnits = lstUnits[iIndex];
		strLowLimits = "";
		if(lstLowLimits.size() > iIndex)
			strLowLimits = lstLowLimits[iIndex];
		strHighLimits = "";
		if(lstHighLimits.size() > iIndex)
			strHighLimits = lstHighLimits[iIndex];

		SaveParameter(	iIndex,
						lstTestNames[iIndex].simplified(),
						strUnits.remove("*").remove("-").simplified(),
                        strLowLimits,
                        strHighLimits);
		
		UpdateParameterIndexTable(lstTestNames[iIndex].simplified());
	}

	// Write all Parameters read on this file : WIR,PIR,PTR...,PRR,WRR WIR,PIR,PTR..., PRR
	// Read PCM_X_FAB result
	strString = ReadLine(*hPcmXFabFile);
	QStringList lstSections;
	while(!strString.isEmpty())
	{
        lstSections = strString.split(",", QString::KeepEmptyParts);
		if(lstSections.size() <= 7)
		{
			// Failed importing PCM_X_FAB file into STDF database
			m_iLastError = errInvalidFormat;
			
			StdfFile.Close();
			// Convertion failed.
			return false;
		}


        m_lStartTime = GetDateTimeFromString(lstSections[7].remove("\""));
		iSiteNumber = lstSections[3].toInt();
		iXWafer = lstSections[5].toInt();
		iYWafer = lstSections[6].toInt();
        if(m_nWaferId != lstSections[1].toInt())
		{
			// For each wafer, have to write limit in the first PTR
			for(iIndex=9;iIndex<lstSections.size();iIndex++)
			{
				m_pParameterList[iIndex]->m_bStaticHeaderWritten = false;
			}
			
			m_nWaferId = lstSections[1].toInt();
			// Write WIR of new Wafer.
			RecordReadInfo.iRecordType = 2;
			RecordReadInfo.iRecordSubType = 10;
			StdfFile.WriteHeader(&RecordReadInfo);
			StdfFile.WriteByte(1);							// Test head
			StdfFile.WriteByte(255);						// Tester site (all)
			StdfFile.WriteDword(m_lStartTime);				// Start time
			StdfFile.WriteString(QString::number(m_nWaferId).toLatin1().constData());	// WaferID
			StdfFile.WriteRecord();
		}
		
		iPartNumber++;

		// Write PIR
		// Write PIR for parts in this Wafer site
		RecordReadInfo.iRecordType = 5;
		RecordReadInfo.iRecordSubType = 10;
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(1);								// Test head
		StdfFile.WriteByte(iSiteNumber);					// Tester site
		StdfFile.WriteRecord();

		// Reset Pass/Fail flag.
		bPassStatus = true;

		// Reset counters
		iTotalTests = 0;

		for(iIndex=9;iIndex<lstSections.size();iIndex++)
		{
			bTestPass = true;
			// Compute Test# (add user-defined offset)
			iTestNumber = GEX_TESTNBR_OFFSET_PCM_X_FAB + iIndex - 8;
			iTotalTests++;
			fValue = lstSections[iIndex].toFloat();
			if(m_pParameterList[iIndex]->m_bValidLowLimit)
				bTestPass &= (m_pParameterList[iIndex]->m_fLowLimit < fValue * GS_POW(10.0,m_pParameterList[iIndex]->m_nScale));
            if(m_pParameterList[iIndex]->m_bValidHighLimit)
				bTestPass &= (m_pParameterList[iIndex]->m_fHighLimit > fValue * GS_POW(10.0,m_pParameterList[iIndex]->m_nScale));
			bPassStatus &= bTestPass;
			

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
			StdfFile.WriteByte(bData);							// TEST_FLG
			bData = BIT6|BIT7;
			StdfFile.WriteByte(bData);							// PARAM_FLG
			StdfFile.WriteFloat(fValue * GS_POW(10.0,m_pParameterList[iIndex]->m_nScale));		// Test result
			if(!m_pParameterList[iIndex]->m_bStaticHeaderWritten)
			{
				m_pParameterList[iIndex]->m_bStaticHeaderWritten = true;

				// save Parameter name
				StdfFile.WriteString(m_pParameterList[iIndex]->m_strName.toLatin1().constData());	// TEST_TXT
				StdfFile.WriteString("");							// ALARM_ID

				bData = 2;	// Valid data.
				if(!m_pParameterList[iIndex]->m_bValidLowLimit)
					bData |= BIT6;
				if(!m_pParameterList[iIndex]->m_bValidHighLimit)
					bData |= BIT7;
				StdfFile.WriteByte(bData);							// OPT_FLAG

				StdfFile.WriteByte(-m_pParameterList[iIndex]->m_nScale);	// RES_SCALE
				StdfFile.WriteByte(-m_pParameterList[iIndex]->m_nScale);	// LLM_SCALE
				StdfFile.WriteByte(-m_pParameterList[iIndex]->m_nScale);	// HLM_SCALE
				StdfFile.WriteFloat(m_pParameterList[iIndex]->m_fLowLimit);	// LOW Limit
				StdfFile.WriteFloat(m_pParameterList[iIndex]->m_fHighLimit);// HIGH Limit
				StdfFile.WriteString(m_pParameterList[iIndex]->m_strUnit.toLatin1().constData());		// Units
			}
			StdfFile.WriteRecord();
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
			wSoftBin = wHardBin = 1;
		}
		else
		{
			StdfFile.WriteByte(8);				// PART_FLG : FAILED
			iTotalFailBin++;
			wSoftBin = wHardBin = 0;
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

		strString = ReadLine(*hPcmXFabFile);
		if(m_nWaferId != strString.section(",",1,1).toInt())
		{
			// Write WRR for last wafer inserted
			RecordReadInfo.iRecordType = 2;
			RecordReadInfo.iRecordSubType = 20;
			StdfFile.WriteHeader(&RecordReadInfo);
			StdfFile.WriteByte(1);						// Test head
			StdfFile.WriteByte(255);					// Tester site (all)
			StdfFile.WriteDword(m_lStartTime);			// Time of last part tested
			StdfFile.WriteDword(iPartNumber);			// Parts tested: always 5
			StdfFile.WriteDword(0);						// Parts retested
			StdfFile.WriteDword(0);						// Parts Aborted
			StdfFile.WriteDword(iGoodParts);			// Good Parts
			StdfFile.WriteDword((DWORD)-1);				// Functionnal Parts
			StdfFile.WriteString(QString::number(m_nWaferId).toLatin1().constData());	// WaferID
			StdfFile.WriteRecord();
			iPartNumber = 0;
			iGoodParts = 0;
			if(hPcmXFabFile->atEnd())
				break;
		}
	}

	

	RecordReadInfo.iRecordType = 1;
	RecordReadInfo.iRecordSubType = 40;

#if 0
	// NO SUMMARY FOR ETEST
	// Write HBR BAD BIN
	StdfFile.WriteHeader(&RecordReadInfo);
	StdfFile.WriteByte(255);						// Test Head = ALL
	StdfFile.WriteByte(255);						// Test sites = ALL		
	StdfFile.WriteWord(0);							// HBIN = 0
	StdfFile.WriteDword(iTotalFailBin);				// Total Bins
	StdfFile.WriteByte('F');
	StdfFile.WriteString("");
	StdfFile.WriteRecord();

	// Write HBR GOOD BIN
	StdfFile.WriteHeader(&RecordReadInfo);
	StdfFile.WriteByte(255);						// Test Head = ALL
	StdfFile.WriteByte(255);						// Test sites = ALL		
	StdfFile.WriteWord(1);							// HBIN = 1
	StdfFile.WriteDword(iTotalGoodBin);				// Total Bins
	StdfFile.WriteByte('P');
	StdfFile.WriteString("");
	StdfFile.WriteRecord();

	RecordReadInfo.iRecordType = 1;
	RecordReadInfo.iRecordSubType = 50;

	// Write SBR BAD BIN
	StdfFile.WriteHeader(&RecordReadInfo);
	StdfFile.WriteByte(255);						// Test Head = ALL
	StdfFile.WriteByte(255);						// Test sites = ALL		
	StdfFile.WriteWord(0);							// HBIN = 0
	StdfFile.WriteDword(iTotalFailBin);				// Total Bins
	StdfFile.WriteByte('F');
	StdfFile.WriteString("");
	StdfFile.WriteRecord();

	// Write SBR GOOD BIN
	StdfFile.WriteHeader(&RecordReadInfo);
	StdfFile.WriteByte(255);						// Test Head = ALL
	StdfFile.WriteByte(255);						// Test sites = ALL		
	StdfFile.WriteWord(1);							// HBIN = 1
	StdfFile.WriteDword(iTotalGoodBin);				// Total Bins
	StdfFile.WriteByte('P');
	StdfFile.WriteString("");
	StdfFile.WriteRecord();

#endif

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
// Convert 'FileName' PCM_X_FAB file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGPCM_X_FABtoSTDF::Convert(const char *PcmXFabFileName, const char *strFileNameSTDF)
{
	// No erro (default)
	m_iLastError = errNoError;

	// If STDF file already exists...do not rebuild it...unless dates not matching!
	QFileInfo fInput(PcmXFabFileName);
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
			GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(PcmXFabFileName).fileName()+"...");
			GexScriptStatusLabel->show();
		}
		GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(PcmXFabFileName).fileName()+"...");
		GexScriptStatusLabel->show();
	}
    QCoreApplication::processEvents();


    if(ReadPcmXFabFile(PcmXFabFileName,strFileNameSTDF) != true)
	{
		//////////////////////////////////////////////////////////////////////
		// For ProgressBar
		if((GexProgressBar != NULL)
		&& bHideProgressAfter)
			GexProgressBar->hide();
		
		if((GexScriptStatusLabel != NULL)
		&& bHideLabelAfter)
			GexScriptStatusLabel->hide();
		return false;	// Error reading PCM_X_FAB file
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
QString CGPCM_X_FABtoSTDF::ReadLine(QTextStream& hFile)
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
		strString = hFile.readLine().trimmed();
		while(strString.endsWith(','))
			strString = strString.left(strString.size()-1);
	}
	while(!strString.isNull() && strString.isEmpty());

	return strString;

}
