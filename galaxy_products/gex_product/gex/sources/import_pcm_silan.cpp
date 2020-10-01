//////////////////////////////////////////////////////////////////////
// import_pcm_silan.cpp: Convert a PcmSilan file to STDF V4.0
//////////////////////////////////////////////////////////////////////

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
#include "import_pcm_silan.h"
#include "import_constants.h"
#include "gqtl_global.h"

// File format:
//Production,Lot NO.,wafer NO.,point,VT5N50/1.8,VT5N50/2,VTSAT5N50/2,VTSAT5N50/2-B,VT5N50/3,VT5N50/5,VT5N50/50,VT5N2/50,VT5N5000/2,
//,,,Unit,V,V,V,V,V,V,V,V,V,V,uA,uA,uA,uA,uA,uA,uA,uA,uA,uA,um,um,uA,uA,uA,uA,V,V,V,V,uA,V,V,V,V,V,V,V,V,V,uA,uA,uA,uA,uA,uA,V,V,V,
//,,,C-,0,0.78,0,0,0,0,0.73,0,0,0,0,5500,0,0,0,0,0,5,0,0,0,0,0,0,0,0,0,12,0,12,0,0,0,18,0,0.72,0,0,0,0,0,17000,0,0,0,0,0,15,15,0,0,
//A,,,B-,0,0.78,0,0,0,0,0.73,0,0,0,0,5500,0,0,0,0,0,5,0,0,0,0,0,0,0,0,0,12,0,12,0,0,0,18,0,0.72,0,0,0,0,0,17000,0,0,0,0,0,15,15,0,0
//,,,B+,0,1.1,0,0,0,0,1.15,0,0,0,0,7700,0,0,0,0,0,15,0,0,0,0,0,0.001,0,0,0,18,0,18,0,0,0,35,0,1.04,0,0,0,0,0,23000,0,0,0.001,0,0,30
//,,,C+,0,1.1,0,0,0,0,1.15,0,0,0,0,7700,0,0,0,0,0,15,0,0,0,0,0,0.001,0,0,0,18,0,18,0,0,0,35,0,1.04,0,0,0,0,0,23000,0,0,0.001,0,0,30
//MS1627SB1,S5827427PE-32,1,1,0.928805,0.932447,0.748557,1.913986,0.928678,0.926981,0.912531,0.90014,0.91429,2.510099,7072.188,6499
//MS1627SB1,S5827427PE-32,1,2,0.929446,0.925867,0.741677,1.897683,0.926559,0.921277,0.9075,0.891705,0.909582,2.496358,7102.5,6560.6
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

CGPcmSilanParameter::CGPcmSilanParameter()
{	
	m_nNumber = -1;
	m_nScale = 0;
	m_fLowLimit = m_fHighLimit = 0;
	m_bValidLowLimit = m_bValidHighLimit = m_bStaticHeaderWritten = false;
	m_fSpecLowLimit = m_fSpecHighLimit = 0;
	m_bValidSpecLowLimit = m_bValidSpecHighLimit = false;
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGPcmSilantoSTDF::CGPcmSilantoSTDF()
{
	// Default: PcmSilan parameter list on disk includes all known PcmSilan parameters...
	m_bNewPcmSilanParameterFound = false;
	m_lStartTime = 0;
	m_pParameterList = NULL;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGPcmSilantoSTDF::~CGPcmSilantoSTDF()
{
	if(m_pParameterList)
		delete [] m_pParameterList;
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGPcmSilantoSTDF::GetLastError()
{
	m_strLastError = "Import Silan - PCM data : ";

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
// Load PcmSilan Parameter table from DISK
//////////////////////////////////////////////////////////////////////
void CGPcmSilantoSTDF::LoadParameterIndexTable(void)
{
	QString	strPcmSilanTableFile;
	QString	strString;

    strPcmSilanTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strPcmSilanTableFile += GEX_PCM_PARAMETERS;

	// Open PcmSilan Parameter table file
	QFile f( strPcmSilanTableFile );
    if(!f.open( QIODevice::ReadOnly ))
		return;

	// Assign file I/O stream
	QTextStream hPcmSilanTableFile(&f);

	// Skip comment or empty lines
	do
	{
	  strString = hPcmSilanTableFile.readLine();
	}
    while((strString.indexOf("----------------------") < 0) && (!hPcmSilanTableFile.atEnd()));

	// Read lines
	m_pFullPcmSilanParametersList.clear();
	strString = hPcmSilanTableFile.readLine();
	while (strString.isNull() == false)
	{
		// Save Parameter name in list
		m_pFullPcmSilanParametersList.append(strString);
		// Read next line
		strString = hPcmSilanTableFile.readLine();
	};

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// Save PcmSilan Parameter table to DISK
//////////////////////////////////////////////////////////////////////
void CGPcmSilantoSTDF::DumpParameterIndexTable(void)
{
	QString		strPcmSilanTableFile;

    strPcmSilanTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strPcmSilanTableFile += GEX_PCM_PARAMETERS;

	// Open PcmSilan Parameter table file
	QFile f( strPcmSilanTableFile );
    if(!f.open( QIODevice::WriteOnly ))
		return;

	// Assign file I/O stream
	QTextStream hPcmSilanTableFile(&f);

	// First few lines are comments:
	hPcmSilanTableFile << "############################################################" << endl;
	hPcmSilanTableFile << "# DO NOT EDIT THIS FILE!" << endl;
    hPcmSilanTableFile << "# Quantix Examinator: PCM Parameters detected" << endl;
	hPcmSilanTableFile << "# www.mentor.com" << endl;
    hPcmSilanTableFile << "# Quantix Examinator reads and writes into this file..." << endl;
	hPcmSilanTableFile << "-----------------------------------------------------------" << endl;

	// Write lines
	// m_pFullPcmSilanParametersList.sort();
	for(int nIndex = 0; nIndex < m_pFullPcmSilanParametersList.count(); nIndex++)
	{
		// Write line
		hPcmSilanTableFile << m_pFullPcmSilanParametersList[nIndex] << endl;
	};

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// If Examinator doesn't have this PcmSilan parameter in his dictionnary, have it added.
//////////////////////////////////////////////////////////////////////
int CGPcmSilantoSTDF::UpdateParameterIndexTable(QString strParamName)
{
	int iTestNumber;

	// Check if the table is empty...if so, load it from disk first!
	if(m_pFullPcmSilanParametersList.isEmpty() == true)
	{
		// Load PcmSilan parameter table from disk...
		LoadParameterIndexTable();
	}
	
	// Check if Parameter name already in table...if not, add it to the list
	// the new full list will be dumped to the disk at the end.

    iTestNumber = m_pFullPcmSilanParametersList.indexOf(strParamName);
	if(iTestNumber < 0)
	{
		// Update list
		m_pFullPcmSilanParametersList.append(strParamName);
        iTestNumber = m_pFullPcmSilanParametersList.indexOf(strParamName);

		// Set flag to force the current PcmSilan table to be updated on disk
		m_bNewPcmSilanParameterFound = true;
	}

	return iTestNumber;
}


//////////////////////////////////////////////////////////////////////
// Save PCM parameter High or Low Limit result...
//////////////////////////////////////////////////////////////////////
void CGPcmSilantoSTDF::SaveParameter(int iIndex,QString strName,QString strUnit,QString strLowLimit, QString strHighLimit,QString strSpecLowLimit, QString strSpecHighLimit)
{
	if(iIndex > m_iTotalParameters)
		return;

	if(strName.isEmpty())
		return;	// Ignore empty entry!

	CGPcmSilanParameter *ptParam = &m_pParameterList[iIndex];

	ptParam->m_strUnit = strUnit;
	NormalizeLimits(ptParam->m_strUnit, ptParam->m_nScale);

	ptParam->m_bValidLowLimit = false;
	ptParam->m_bValidHighLimit = false;
	ptParam->m_bValidSpecLowLimit = false;
	ptParam->m_bValidSpecHighLimit = false;
	
	// If all is "0" then no limit
	if((strLowLimit == "0")
	&& (strHighLimit == "0")
	&& (strSpecLowLimit == "0")
	&& (strSpecHighLimit == "0"))
		return;

	if((strSpecLowLimit != "0")
	|| (strSpecHighLimit != "0"))
	{
		// if have one or two Spec limits	
		// Save Spec limit and save Control limit as Spec Limit
		ptParam->m_fSpecLowLimit = strSpecLowLimit.toFloat(&ptParam->m_bValidSpecLowLimit) * GS_POW(10.0,ptParam->m_nScale);
		ptParam->m_fSpecHighLimit = strSpecHighLimit.toFloat(&ptParam->m_bValidSpecHighLimit) * GS_POW(10.0,ptParam->m_nScale);
		ptParam->m_fLowLimit = strSpecLowLimit.toFloat(&ptParam->m_bValidLowLimit) * GS_POW(10.0,ptParam->m_nScale);
		ptParam->m_fHighLimit = strSpecHighLimit.toFloat(&ptParam->m_bValidHighLimit) * GS_POW(10.0,ptParam->m_nScale);
	}



	if((strLowLimit != "0")
	|| (strHighLimit != "0"))
	{
		// if have one or two Control limits	
		// Save Control limit
		ptParam->m_fLowLimit = strLowLimit.toFloat(&ptParam->m_bValidLowLimit) * GS_POW(10.0,ptParam->m_nScale);
		ptParam->m_fHighLimit = strHighLimit.toFloat(&ptParam->m_bValidHighLimit) * GS_POW(10.0,ptParam->m_nScale);
	}


}

//////////////////////////////////////////////////////////////////////
// Normalize test limits when writing into PTR.
//////////////////////////////////////////////////////////////////////
void CGPcmSilantoSTDF::NormalizeLimits(QString &strUnit, int &nScale)
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
// Check if File is compatible with PcmSilan format
//////////////////////////////////////////////////////////////////////
bool CGPcmSilantoSTDF::IsCompatible(const char *szFileName)
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
	QTextStream hPcmSilanFile(&f);
	QStringList lstSections;

	// Check if first line is the correct PcmSilan header...
	//Production,Lot NO.,wafer NO.,point,VT5N50/1.8,VT5N50/2,VTSAT5N50/2,VTSAT5N50/2-B,VT5N50/3,VT5N50/5,VT5N50/50,VT5N2/50,VT5N5000/2,

	do
		strString = hPcmSilanFile.readLine().simplified();
	while(!strString.isNull() && strString.isEmpty());

	if(	!strString.startsWith("Production,Lot NO.,wafer NO.,point,", Qt::CaseInsensitive))
	{
		// Incorrect header...this is not a PcmSilan file!
		// Close file
		f.close();
		return false;
	}
	// Close file
	f.close();

	return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the PcmSilan file
//////////////////////////////////////////////////////////////////////
bool CGPcmSilantoSTDF::ReadPcmSilanFile(const char *PcmSilanFileName,const char *strFileNameSTDF)
{
	QString strString;
	QString strSection;
	QString strValue;
	QString	strSite;

	// Open PcmSilan file
	QFile f( PcmSilanFileName );
    if(!f.open( QIODevice::ReadOnly ))
	{
		// Failed Opening PcmSilan file
		m_iLastError = errOpenFail;
		
		// Convertion failed.
		return false;
	}
	// Assign file I/O stream
	QTextStream hPcmSilanFile(&f);
	QStringList lstSections;

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	iFileSize = f.size() + 1;

	// Check if first line is the correct PcmSilan header...
	//Production,Lot NO.,wafer NO.,point,VT5N50/1.8,VT5N50/2,VTSAT5N50/2,VTSAT5N50/2-B,VT5N50/3,VT5N50/5,VT5N50/50,VT5N2/50,VT5N5000/2,
	//,,,Unit,V,V,V,V,V,V,V,V,V,V,uA,uA,uA,uA,uA,uA,uA,uA,uA,uA,um,um,uA,uA,uA,uA,V,V,V,V,uA,V,V,V,V,V,V,V,V,V,uA,uA,uA,uA,uA,uA,V,V,V,

	strString = ReadLine(hPcmSilanFile).simplified();
	if(	!strString.startsWith("Production,Lot NO.,wafer NO.,point,", Qt::CaseInsensitive))
	{
		// Incorrect header...this is not a PcmSilan file!
		m_iLastError = errInvalidFormat;
		
		// Convertion failed.
		// Close file
		f.close();
		return false;
	}

	// Good header

	//Production,Lot NO.,wafer NO.,point,VT5N50/1.8,VT5N50/2,VTSAT5N50/2,VTSAT5N50/2-B,VT5N50/3,VT5N50/5,VT5N50/50,VT5N2/50,VT5N5000/2,

	// Read PcmSilan information
    lstSections = strString.split(",",QString::KeepEmptyParts);

	// Count the number of parameters specified in the line
	// Do not count first 4 fields
	m_iTotalParameters=lstSections.count() - 4;
	// If no parameter specified...ignore!
	if(m_iTotalParameters <= 0)
	{
		// Incorrect header...this is not a valid Silan - PCM file!
		m_iLastError = errInvalidFormat;

		// Convertion failed.
		// Close file
		f.close();
		return false;
	}

	// Allocate the buffer to hold the N parameters & results.
	m_pParameterList = new CGPcmSilanParameter[m_iTotalParameters];	// List of parameters

	// Extract the N column names
	int iIndex;
	for(iIndex=0;iIndex<m_iTotalParameters;iIndex++)
	{
		strSection = lstSections[iIndex+4].trimmed();	// Remove spaces
		m_pParameterList[iIndex].m_strName = strSection;
		m_pParameterList[iIndex].m_nNumber = UpdateParameterIndexTable(strSection);		// Update Parameter master list if needed.
	}


	// Read the next 5 lines
	//,,,Unit,V,V,V,V,V,V,V,V,V,V,uA,uA,uA,uA,uA,uA,uA,uA,uA,uA,um,um,uA,uA,uA,uA,V,V,V,V,uA,V,V,V,V,V,V,V,V,V,uA,uA,uA,uA,uA,uA,V,V,V,
	//,,,C-,0,0.78,0,0,0,0,0.73,0,0,0,0,5500,0,0,0,0,0,5,0,0,0,0,0,0,0,0,0,12,0,12,0,0,0,18,0,0.72,0,0,0,0,0,17000,0,0,0,0,0,15,15,0,0,
	//A,,,B-,0,0.78,0,0,0,0,0.73,0,0,0,0,5500,0,0,0,0,0,5,0,0,0,0,0,0,0,0,0,12,0,12,0,0,0,18,0,0.72,0,0,0,0,0,17000,0,0,0,0,0,15,15,0,0
	//,,,B+,0,1.1,0,0,0,0,1.15,0,0,0,0,7700,0,0,0,0,0,15,0,0,0,0,0,0.001,0,0,0,18,0,18,0,0,0,35,0,1.04,0,0,0,0,0,23000,0,0,0.001,0,0,30
	//,,,C+,0,1.1,0,0,0,0,1.15,0,0,0,0,7700,0,0,0,0,0,15,0,0,0,0,0,0.001,0,0,0,18,0,18,0,0,0,35,0,1.04,0,0,0,0,0,23000,0,0,0.001,0,0,30

	// Read all limits
	QStringList	lstUnit;
	QStringList lstLowLimit;
	QStringList lstSpecLowLimit;
	QStringList lstSpecHighLimit;
	QStringList lstHighLimit;

	strString = ReadLine(hPcmSilanFile);
    lstUnit = strString.split(",",QString::KeepEmptyParts);
	strString = ReadLine(hPcmSilanFile);
    lstLowLimit = strString.split(",",QString::KeepEmptyParts);
	strString = ReadLine(hPcmSilanFile);
    lstSpecLowLimit = strString.split(",",QString::KeepEmptyParts);
	strString = ReadLine(hPcmSilanFile);
    lstSpecHighLimit = strString.split(",",QString::KeepEmptyParts);
	strString = ReadLine(hPcmSilanFile);
    lstHighLimit = strString.split(",",QString::KeepEmptyParts);

	if((lstUnit.count() < m_iTotalParameters) || (lstUnit[3].toUpper() != "UNIT")
	|| (lstLowLimit.count()	< m_iTotalParameters) || (lstLowLimit[3].toUpper()	!= "C-")
	|| (lstSpecLowLimit.count()		< m_iTotalParameters) || (lstSpecLowLimit[3].toUpper()		!= "B-")
	|| (lstSpecHighLimit.count()	< m_iTotalParameters) || (lstSpecHighLimit[3].toUpper()		!= "B+")
	|| (lstHighLimit.count() < m_iTotalParameters) || (lstHighLimit[3].toUpper()	!= "C+"))
	{
		// Incorrect header...this is not a valid Silan - PCM file!
		m_iLastError = errInvalidFormat;

		// Convertion failed.
		// Close file
		f.close();
		return false;
	}

	// Extract the N columns
	for(iIndex=0;iIndex<m_iTotalParameters;iIndex++)
	{
		SaveParameter(iIndex,m_pParameterList[iIndex].m_strName,lstUnit[iIndex+4],lstLowLimit[iIndex+4],lstHighLimit[iIndex+4],lstSpecLowLimit[iIndex+4],lstSpecHighLimit[iIndex+4]);
	}


	if(!WriteStdfFile(&hPcmSilanFile,strFileNameSTDF))
	{
		QFile::remove(strFileNameSTDF);
		// Close file
		f.close();
		return false;
	}
	
	// Close file
	f.close();
	
	// All PcmSilan file read...check if need to update the PcmSilan Parameter list on disk?
	if(m_bNewPcmSilanParameterFound == true)
		DumpParameterIndexTable();

	// Success parsing PcmSilan file
	return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from PcmSilan data parsed
//////////////////////////////////////////////////////////////////////
bool CGPcmSilantoSTDF::WriteStdfFile(QTextStream *hPcmSilanFile,const char *strFileNameSTDF)
{
	// now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open((char*)strFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
	{
		// Failed importing PcmSilan file into STDF database
		m_iLastError = errWriteSTDF;
		
		// Convertion failed.
		return false;
	}

	// Read the first line result to have Lot and Product information
	QString		strString;
	strString = ReadLine(*hPcmSilanFile);

	m_strProductId = strString.section(",",0,0).trimmed();
	m_strLotId = strString.section(",",1,1).trimmed();


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
	StdfFile.WriteString("Pcm Silan");			// Tester Type
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
	strUserTxt += ":PCM_SILAN";
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
	WORD		wSoftBin,wHardBin;
	int			iGoodParts;
	int			iTotalGoodBin,iTotalFailBin;
	int			iTotalTests,iPartNumber;
	bool		bPassStatus;
	BYTE		bData;

	int			iIndex;
	int			iWaferNumber;
	int			iSiteNumber;
	bool		bTestPass;
	float		fValue;
	bool		bValue;

	// Reset counters
	iGoodParts=iTotalGoodBin=iTotalFailBin=0;
	iPartNumber=0;
	m_nWaferId = -1;

	// Write all Parameters read on this file : WIR,PIR,PTR...,PRR,WRR WIR,PIR,PTR..., PRR
	// Read PcmSilan result
	while(!hPcmSilanFile->atEnd())
	{
		if(strString.isEmpty())
			strString = ReadLine(*hPcmSilanFile);


		iWaferNumber = strString.section(",",2,2).trimmed().toInt();
		iSiteNumber = strString.section(",",3,3).trimmed().toInt();

		if(m_nWaferId != iWaferNumber)
		{
			// New wafer
			// Close the last
			if(m_nWaferId > -1)
			{
				// Write WRR for last wafer inserted
				RecordReadInfo.iRecordType = 2;
				RecordReadInfo.iRecordSubType = 20;
				StdfFile.WriteHeader(&RecordReadInfo);
				StdfFile.WriteByte(1);						// Test head
				StdfFile.WriteByte(255);					// Tester site (all)
				StdfFile.WriteDword(m_lStartTime);			// Time of last part tested
				StdfFile.WriteDword(iTotalGoodBin+iTotalFailBin);	// Parts tested
				StdfFile.WriteDword(0);						// Parts retested
				StdfFile.WriteDword(0);						// Parts Aborted
				StdfFile.WriteDword(iTotalGoodBin);			// Good Parts
				StdfFile.WriteDword((DWORD)-1);				// Functionnal Parts
				StdfFile.WriteString(QString::number(m_nWaferId).toLatin1().constData());	// WaferID
				StdfFile.WriteString("");					// FabId
				StdfFile.WriteString("");					// FrameId
				StdfFile.WriteString("");					// MaskId
				StdfFile.WriteString("");					// UserDesc
				StdfFile.WriteRecord();

			}

			m_nWaferId = iWaferNumber;

			// Write WIR of new Wafer.
			RecordReadInfo.iRecordType = 2;
			RecordReadInfo.iRecordSubType = 10;
			StdfFile.WriteHeader(&RecordReadInfo);
			StdfFile.WriteByte(1);							// Test head
			StdfFile.WriteByte(255);						// Tester site (all)
			StdfFile.WriteDword(m_lStartTime);				// Start time
			StdfFile.WriteString(QString::number(m_nWaferId).toLatin1().constData());	// WaferID
			StdfFile.WriteRecord();

			// For each wafer, have to write limit in the first PTR
			for(iIndex=0;iIndex<m_iTotalParameters;iIndex++)
			{
				m_pParameterList[iIndex].m_bStaticHeaderWritten = false;
			}

			iTotalGoodBin = iTotalFailBin = iTotalTests = 0;
			iPartNumber = 0;

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

		strString = strString.trimmed().remove(" ");

		for(iIndex=0;iIndex<m_iTotalParameters;iIndex++)
		{
			fValue = strString.section(",",iIndex+4,iIndex+4).toFloat(&bValue);

			// Check if have valid value
			if(!bValue)
				continue;

			iTotalTests++;
			bTestPass = true;
			// Compute Test# (add user-defined offset)

			if(m_pParameterList[iIndex].m_bValidLowLimit)
				bTestPass &= (m_pParameterList[iIndex].m_fLowLimit < fValue * GS_POW(10.0,m_pParameterList[iIndex].m_nScale));
			if(m_pParameterList[iIndex].m_bValidHighLimit)
				bTestPass &= (m_pParameterList[iIndex].m_fHighLimit > fValue * GS_POW(10.0,m_pParameterList[iIndex].m_nScale));
			bPassStatus &= bTestPass;
			

			// Write PTR
			RecordReadInfo.iRecordType = 15;
			RecordReadInfo.iRecordSubType = 10;
			
			StdfFile.WriteHeader(&RecordReadInfo);
			StdfFile.WriteDword(m_pParameterList[iIndex].m_nNumber + GEX_TESTNBR_OFFSET_PCM);			// Test Number
			StdfFile.WriteByte(1);						// Test head
			StdfFile.WriteByte(iSiteNumber);			// Tester site:1,2,3,4 or 5, etc.
			if(bTestPass)
				bData = 0;		// Test passed
			else
				bData = BIT7;	// Test Failed
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
				if(!m_pParameterList[iIndex].m_bValidSpecLowLimit)
					bData |= BIT2;
				if(!m_pParameterList[iIndex].m_bValidSpecHighLimit)
					bData |= BIT3;
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
				StdfFile.WriteString("");											//
				StdfFile.WriteString("");											//
				StdfFile.WriteString("");											//
				StdfFile.WriteFloat(m_pParameterList[iIndex].m_fSpecLowLimit);	// LOW Spec Limit
				StdfFile.WriteFloat(m_pParameterList[iIndex].m_fSpecHighLimit);	// HIGH Spec Limit
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
			default: // More than 5 sites?....give 0,0 coordonates
				StdfFile.WriteWord(0);			// X_COORD
				StdfFile.WriteWord(0);			// Y_COORD
				break;
		}
		StdfFile.WriteDword(0);					// No testing time known...
		StdfFile.WriteString(QString::number(iPartNumber).toLatin1().constData());// PART_ID
		StdfFile.WriteString("");				// PART_TXT
		StdfFile.WriteString("");				// PART_FIX
		StdfFile.WriteRecord();

		strString = "";

	}

	// Close the last
	if(m_nWaferId > -1)
	{
		// Write WRR for last wafer inserted
		RecordReadInfo.iRecordType = 2;
		RecordReadInfo.iRecordSubType = 20;
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(1);						// Test head
		StdfFile.WriteByte(255);					// Tester site (all)
		StdfFile.WriteDword(m_lStartTime);			// Time of last part tested
		StdfFile.WriteDword(iTotalGoodBin+iTotalFailBin);	// Parts tested
		StdfFile.WriteDword(0);						// Parts retested
		StdfFile.WriteDword(0);						// Parts Aborted
		StdfFile.WriteDword(iTotalGoodBin);			// Good Parts
		StdfFile.WriteDword((DWORD)-1);				// Functionnal Parts
		StdfFile.WriteString(QString::number(m_nWaferId).toLatin1().constData());	// WaferID
		StdfFile.WriteString("");					// FabId
		StdfFile.WriteString("");					// FrameId
		StdfFile.WriteString("");					// MaskId
		StdfFile.WriteString("");					// UserDesc
		StdfFile.WriteRecord();

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
// Convert 'FileName' PcmSilan file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGPcmSilantoSTDF::Convert(const char *PcmSilanFileName, const char *strFileNameSTDF)
{
	// No erro (default)
	m_iLastError = errNoError;

	// If STDF file already exists...do not rebuild it...unless dates not matching!
	QFileInfo fInput(PcmSilanFileName);
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
			GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(PcmSilanFileName).fileName()+"...");
			GexScriptStatusLabel->show();
		}
		GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(PcmSilanFileName).fileName()+"...");
		GexScriptStatusLabel->show();
	}
    QCoreApplication::processEvents();


    if(ReadPcmSilanFile(PcmSilanFileName,strFileNameSTDF) != true)
	{
		//////////////////////////////////////////////////////////////////////
		// For ProgressBar
		if((GexProgressBar != NULL)
		&& bHideProgressAfter)
			GexProgressBar->hide();
		
		if((GexScriptStatusLabel != NULL)
		&& bHideLabelAfter)
			GexScriptStatusLabel->hide();
		return false;	// Error reading PcmSilan file
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
QString CGPcmSilantoSTDF::ReadLine(QTextStream& hFile)
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
