//////////////////////////////////////////////////////////////////////
// import_yokogawa.cpp: Convert a Yokogawa file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#include "gqtl_global.h"
#include <math.h>
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
#include "import_yokogawa.h"
#include "import_constants.h"

// File format:
//<< Test Result Data >>                                    2006/03/20 07:07:48
//$#HEADER#$
//Tester-ID               : ts6tsc2
//Station-Number          : 1
//Handler/Prober-Name     : 
//Test-Program-Name       : 1A8090W1B1
//Device-Name             : QA8090B
//
//Lot-Number              : MKC4523.1-15
//Wafer-Number            : 15 
//
//Customer                : sunplus
//$#DATE#$
//Test-Start-Date  2006/03/20_07:07:47
//Test-End-Date    2006/03/20_07:07:48
//$#TESTDATA#$
//Dut#     IC#   Waf#  IC(Waf)#   P/F    Bin     Cat    Xadr  Yadr
//   1   19108     15         1   FAIL     5       5      92   131
//$#DUT1TEST#$
//Test#  Pin  PF    Value  L-Limit  U-Limit Unit     DataName Comment/PinName
// 1010    +   P        -        -        - -        -        odd_iopin negative diode test
// 1010    1   P   -0.501   -1.200   -0.200 V        -        LEDPWM          
// 1010    3   P   -0.502   -1.200   -0.200 V        -        LEDC1           
// 1010    5   P   -0.503   -1.200   -0.200 V        -        LEDPWDN         


// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar

#define BIT0			0x01
#define BIT1			0x02
#define BIT2			0x04
#define BIT3			0x08
#define BIT4			0x10
#define BIT5			0x20
#define BIT6			0x40
#define BIT7			0x80


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGYOKOGAWAtoSTDF::CGYOKOGAWAtoSTDF()
{
	// Default: YOKOGAWA parameter list on disk includes all known YOKOGAWA parameters...
	m_bNewYokogawaParameterFound = false;
	m_lStartTime = 0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGYOKOGAWAtoSTDF::~CGYOKOGAWAtoSTDF()
{
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGYOKOGAWAtoSTDF::GetLastError()
{
	m_strLastError = "Import YOKOGAWA: ";

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
// Load YOKOGAWA Parameter table from DISK
//////////////////////////////////////////////////////////////////////
void CGYOKOGAWAtoSTDF::LoadParameterIndexTable(void)
{
	QString	strYokogawaTableFile;
	QString	strString;

    strYokogawaTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strYokogawaTableFile += GEX_YOKOGAWA_PARAMETERS;

	// Open YOKOGAWA Parameter table file
    QFile f( strYokogawaTableFile );
    if(!f.open( QIODevice::ReadOnly ))
		return;

	// Assign file I/O stream
	QTextStream hYokogawaTableFile(&f);

	// Skip comment or empty lines
	do
	{
	  strString = hYokogawaTableFile.readLine().simplified();
	}
    while((strString.indexOf("----------------------") < 0) && (!hYokogawaTableFile.atEnd()));

	// Read lines
	m_pFullYokogawaParametersList.clear();
	strString = hYokogawaTableFile.readLine().simplified();
	while (strString.isNull() == false)
	{
		// Save Parameter name in list
		m_pFullYokogawaParametersList.append(strString);
		// Read next line
		strString = hYokogawaTableFile.readLine().simplified();
	};

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// Save YOKOGAWA Parameter table to DISK
//////////////////////////////////////////////////////////////////////
void CGYOKOGAWAtoSTDF::DumpParameterIndexTable(void)
{
	QString		strYokogawaTableFile;

    strYokogawaTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strYokogawaTableFile += GEX_YOKOGAWA_PARAMETERS;

	// Open YOKOGAWA Parameter table file
    QFile f( strYokogawaTableFile );
    if(!f.open( QIODevice::WriteOnly ))
		return;

	// Assign file I/O stream
	QTextStream hYokogawaTableFile(&f);

	// First few lines are comments:
	hYokogawaTableFile << "############################################################" << endl;
	hYokogawaTableFile << "# DO NOT EDIT THIS FILE!" << endl;
    hYokogawaTableFile << "# Quantix Examinator: YOKOGAWA Parameters detected" << endl;
	hYokogawaTableFile << "# www.mentor.com" << endl;
    hYokogawaTableFile << "# Quantix Examinator reads and writes into this file..." << endl;
	hYokogawaTableFile << "-----------------------------------------------------------" << endl;

	// Write lines
	// m_pFullYokogawaParametersList.sort();
	for(int nIndex=0; nIndex < m_pFullYokogawaParametersList.count(); nIndex++)
	{
		// Write line
		hYokogawaTableFile << m_pFullYokogawaParametersList[nIndex] << endl;
	};

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// If Examinator doesn't have this YOKOGAWA parameter in his dictionnary, have it added.
//////////////////////////////////////////////////////////////////////
void CGYOKOGAWAtoSTDF::UpdateParameterIndexTable(QString strParamName)
{
	// Check if the table is empty...if so, load it from disk first!
	if(m_pFullYokogawaParametersList.isEmpty() == true)
	{
		// Load YOKOGAWA parameter table from disk...
		LoadParameterIndexTable();
	}
	
	// Check if Parameter name already in table...if not, add it to the list
	// the new full list will be dumped to the disk at the end.
    if(m_pFullYokogawaParametersList.indexOf(strParamName) < 0)
	{
		// Update list
		m_pFullYokogawaParametersList.append(strParamName);

		// Set flag to force the current YOKOGAWA table to be updated on disk
		m_bNewYokogawaParameterFound = true;
	}
}


//////////////////////////////////////////////////////////////////////
// Normalize test limits when writing into PTR.
//////////////////////////////////////////////////////////////////////
void CGYOKOGAWAtoSTDF::NormalizeLimits(QString &strUnit, int &nScale)
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
// Read and Parse the YOKOGAWA file
//////////////////////////////////////////////////////////////////////
bool CGYOKOGAWAtoSTDF::ReadYokogawaFile(const char *YokogawaFileName,const char *strFileNameSTDF)
{
	QString		strString;
	QString		strSection;


	// Open YOKOGAWA file
    QFile f( YokogawaFileName );
    if(!f.open( QIODevice::ReadOnly ))
	{
		// Failed Opening YOKOGAWA file
		m_iLastError = errOpenFail;
		
		// Convertion failed.
		return false;
	}
	// Assign file I/O stream
	QTextStream hYokogawaFile(&f);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	iProgressStep = 0;
	iNextFilePos = 0;
	iFileSize = f.size() + 1;
    QCoreApplication::processEvents();
	
	// Check if first line is the correct YOKOGAWA header...
	//TEXT REPORT
	strString = ReadLine(hYokogawaFile);

	//<< Test Result Data >>                                    2006/03/20 07:07:48
	//$#HEADER#$
	if(!strString.startsWith("<< Test Result Data >>", Qt::CaseInsensitive))
	{
		if(!strString.startsWith("$#HEADER#$", Qt::CaseInsensitive))
		{
			// Incorrect header...this is not a YOKOGAWA file!
			m_iLastError = errInvalidFormat;
			
			// Convertion failed.
			// Close file
			f.close();
			return false;
		}
	}
	else
	{
		strString = ReadLine(hYokogawaFile);
	}
	
	//$#HEADER#$
	if(!strString.startsWith("$#HEADER#$", Qt::CaseInsensitive))
	{
		// Incorrect header...this is not a YOKOGAWA file!
		m_iLastError = errInvalidFormat;
		
		// Convertion failed.
		// Close file
		f.close();
		return false;
	}

	strString = "";
	// Read YOKOGAWA information
	//Tester-ID               : ts6tsc2
	//Station-Number          : 1
	//Handler/Prober-Name     : 
	//Test-Program-Name       : 1A8090W1B1
	//Device-Name             : QA8090B
	//
	//Lot-Number              : MKC4523.1-15
	//Wafer-Number            : 15 
	//
	//Customer                : sunplus
	//$#DATE#$
	//Test-Start-Date  2006/03/20_07:07:47
	//Test-End-Date    2006/03/20_07:07:48
	//$#TESTDATA#$
	while(!hYokogawaFile.atEnd())
	{
		strString = ReadLine(hYokogawaFile);
		if(strString.isEmpty())
			continue;

		strSection = strString.section(":",0,0).simplified().toUpper();
		strString = strString.section(":",1);

		if(strSection == "TESTER-ID")
		{
			m_strTesterId = strString.simplified();
			strString = "";
		}
		else if(strSection == "STATION-NUMBER")
		{
			m_strStationNumber = strString.simplified();
			strString = "";
		}
		else if(strSection == "HANDLER/PROBER-NAME")
		{
			// SDR.HAND_ID
			m_strProberName = strString.simplified();
			strString = "";
		}
		else if(strSection == "TEST-PROGRAM-NAME")
		{
			m_strProgramName = strString.simplified();
			strString = "";
		}
		else if(strSection == "DEVICE-NAME")
		{
			m_strDeviceName = strString.simplified();
			strString = "";
		}
		else if(strSection == "LOT-NUMBER")
		{
			m_strLotId = strString.simplified();
			strString = "";
		}
		else if(strSection == "WAFER-NUMBER")
		{
			m_strWaferId = strString.simplified();
			strString = "";
		}
		else if(strSection == "CUSTOMER")
		{
			m_strOperatorName = strString.simplified();
			strString = "";
		}
		else if(strSection == "$#DATE#$")
		{
			//Test-Start-Date  2006/11/02_16:02:20
			strString = ReadLine(hYokogawaFile).simplified();
			strString = strString.section(" ",1);
			QDate	clDate = QDate::fromString(strString.left(10),Qt::ISODate);
			QTime	clTime = QTime::fromString(strString.right(8));
			QDateTime clDateTime(clDate,clTime);
			clDateTime.setTimeSpec(Qt::UTC);
			m_lStartTime = clDateTime.toTime_t();

			strString = ReadLine(hYokogawaFile).simplified();
			strString = strString.section(" ",1);
			clDate = QDate::fromString(strString.left(10),Qt::ISODate);
			clTime = QTime::fromString(strString.right(8));
			m_lEndTime = QDateTime(clDate,clTime).toTime_t();
			strString = "";
		}
		else if(strSection == "$#TESTDATA#$")
		{
			// begining of data
			break;
		}
		else
		{
			// Unknown header
			// ignore
		}
	}

	// It's a YOKOGAWA file
	//Restart at the beggining of the file
	hYokogawaFile.seek(0);

	if(!WriteStdfFile(&hYokogawaFile,strFileNameSTDF))
	{
		QFile::remove(strFileNameSTDF);
		// Close file
		f.close();
		return false;
	}
	
	// All YOKOGAWA file read...check if need to update the YOKOGAWA Parameter list on disk?
	if(m_bNewYokogawaParameterFound == true)
		DumpParameterIndexTable();

	// Success parsing YOKOGAWA file
	return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from YOKOGAWA data parsed
//////////////////////////////////////////////////////////////////////
bool CGYOKOGAWAtoSTDF::WriteStdfFile(QTextStream *hYokogawaFile,const char *strFileNameSTDF)
{
	// now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open((char*)strFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
	{
		// Failed importing YOKOGAWA file into STDF database
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
	StdfFile.WriteByte(m_strStationNumber.toInt());	// Station
	StdfFile.WriteByte((BYTE) 'P');				// Test Mode = PRODUCTION
	StdfFile.WriteByte((BYTE) ' ');				// rtst_cod
	StdfFile.WriteByte((BYTE) ' ');				// prot_cod
	StdfFile.WriteWord(65535);					// burn_tim
	StdfFile.WriteByte((BYTE) ' ');				// cmod_cod
	StdfFile.WriteString(m_strLotId.toLatin1().constData());		// Lot ID
	StdfFile.WriteString(m_strDeviceName.toLatin1().constData());	// Part Type / Product ID
	StdfFile.WriteString("");					// Node name
	StdfFile.WriteString(m_strTesterId.toLatin1().constData());	// Tester Type
	StdfFile.WriteString(m_strProgramName.toLatin1().constData());	// Job name
	StdfFile.WriteString("");					// Job rev
	StdfFile.WriteString("");					// sublot-id
	StdfFile.WriteString(m_strOperatorName.toLatin1().constData());	// operator
	StdfFile.WriteString("");					// exec-type
	StdfFile.WriteString("");					// exe-ver
	StdfFile.WriteString("");					// test-cod
	StdfFile.WriteString("");					// test-temperature
	// Construct custom Galaxy USER_TXT 
	QString	strUserTxt;
	strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;	
	strUserTxt += ":";
	strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
	strUserTxt += ":YOKOGAWA";
	StdfFile.WriteString(strUserTxt.toLatin1().constData());		// user-txt
	StdfFile.WriteString("");					// aux-file
	StdfFile.WriteString("");					// package-type
	StdfFile.WriteString("");					// familyID
	StdfFile.WriteString("");					// Date-code
	StdfFile.WriteString("");					// Facility-ID
	StdfFile.WriteString("");					// FloorID
	StdfFile.WriteString("");					// ProcessID

	StdfFile.WriteRecord();

	// Write WIR
	RecordReadInfo.iRecordType = 2;
	RecordReadInfo.iRecordSubType = 10;
	StdfFile.WriteHeader(&RecordReadInfo);
	StdfFile.WriteByte(1);						// Test head
	StdfFile.WriteByte(255);					// Tester site (all)
	StdfFile.WriteDword(m_lStartTime);			// Start time
	StdfFile.WriteString(m_strWaferId.toLatin1().constData());	// WaferID
	StdfFile.WriteRecord();
			
	// Write Test results for each line read.
	QString		strString;
	QString		strSection;
	QStringList	lstSections;
	long		iTotalGoodBin,iTotalFailBin;
        long iTotalTests,iSiteNumber;
        //FIXME: not used ?
        //long iPartNumber;
	BYTE		bData;

	QMap<int, CGYOKOGAWA_Part*>	mapPartResult;
	CGYOKOGAWA_Part*				pPartResult;

	QMap<int,int>	mapHBinCount;
	QMap<int,bool>	mapHBinGood;
	QMap<int,int>	mapSBinCount;
	QMap<int,bool>	mapSBinGood;

	int			iIndex;
	int			iTestNumber;
	int			iPinNumber;
	int			iNumber;
	bool		bTestPass;
	QString		strTestName;
	QString		strPinName;
	QString		strNotes;
	QString		strUnit;
	int			nScale;
	float		fValue;
	float		fLowLimit, fHighLimit;
	bool		bHaveLowLimit, bHaveHighLimit;
	bool		bTestAlreadySaved;
	bool		bPtrRecord;
	QStringList lstTestSavedWithLimit;

	// Reset counters
	iTotalGoodBin=iTotalFailBin=0;
	//FIXME: not used ?
        //iPartNumber=
        iTestNumber=iPinNumber=iNumber=0;

	// Write all Parameters read on this file : PIR,PTR...,PRR, PIR,PTR..., PRR
	//
	//$#SHEADER#$
	//S#:1 :Prg:1A9217W1A2                     :Waf#:  6 :
	//      Dev:ORISETECH:QA9217E-T            :Lot:C1102_QK6315-06C4              :
	//    Title:ST-07                                                              :
	//$#DATE#$
	//Test-Start-Date  2006/11/02_16:02:22
	//Test-End-Date    2006/11/02_16:02:24
	//$#TESTDATA#$
	//Dut#     IC#   Waf#  IC(Waf)#   P/F    Bin     Cat    Xadr  Yadr
	//   1       3      6         3   PASS     1       0      84     0
	//   2       4      6         4   FAIL     3       3      85     0
	//$#DUT1TEST#$
	//Test#  Pin  PF    Value  L-Limit  U-Limit Unit     DataName Comment/PinName
	// 1100    +   P        -        -        - -        -        VDD OPEN/SHORT
	// 1100  102   P   -0.386   -1.200   -0.200 V        -        VDD             
	
	// Read YOKOGAWA information
	while(!hYokogawaFile->atEnd())
	{
		
		if(strString.simplified().isEmpty())
			strString = ReadLine(*hYokogawaFile);
		if(strString.isEmpty())
			continue;

		strSection = strString.section(" ",0,0).toUpper();
		strString = strString.section(" ",1);

		if(strSection == "$#SHEADER#$")
		{
			// Ignore this marker
			// Goto next header
			while(!hYokogawaFile->atEnd())
			{
				strString = ReadLine(*hYokogawaFile);
				if(strString.startsWith("$#"))
				{
					// Found header
					break;
				}
			}
		}
		else if(strSection == "$#DATE#$")
		{
			//Test-Start-Date  2006/11/02_16:02:20
			strString = ReadLine(*hYokogawaFile).simplified();
			strString = strString.section(" ",1);
			QDate	clDate = QDate::fromString(strString.left(10),Qt::ISODate);
			QTime	clTime = QTime::fromString(strString.right(8));
			QDateTime clDateTime(clDate,clTime);
			clDateTime.setTimeSpec(Qt::UTC);
			m_lStartTime = clDateTime.toTime_t();

			strString = ReadLine(*hYokogawaFile).simplified();
			strString = strString.section(" ",1);
			clDate = QDate::fromString(strString.left(10),Qt::ISODate);
			clTime = QTime::fromString(strString.right(8));
			m_lEndTime = QDateTime(clDate,clTime).toTime_t();
			strString = "";
		}
		else if(strSection == "$#TESTDATA#$")
		{
			strString = ReadLine(*hYokogawaFile);
			// Verify if it's Dut line
			if(!strString.startsWith("Dut#", Qt::CaseInsensitive))
			{
				// Goto Next Marker
				while(!hYokogawaFile->atEnd())
				{
					strString = ReadLine(*hYokogawaFile);
					if(strString.startsWith("$#"))
					{
						// Found header
						break;
					}
				}
			}

			while(!hYokogawaFile->atEnd())
			{
				strString = ReadLine(*hYokogawaFile);
				if(strString.startsWith("$#DUT", Qt::CaseInsensitive))
				{
					// Have marker $#DUT1TEST#$
					break;
				}
				iIndex = 0;
                lstSections = strString.split(" ", QString::SkipEmptyParts);
				// Check if have the good count
				if(lstSections.count() < 9)
				{
					m_iLastError = errInvalidFormatLowInRows;
					
					// Convertion failed.
					return false;
				}

				iSiteNumber = lstSections[iIndex++].toInt();
				if(mapPartResult.contains(iSiteNumber))
					pPartResult = mapPartResult[iSiteNumber];
				else
				{
					pPartResult = new CGYOKOGAWA_Part();
					mapPartResult[iSiteNumber] = pPartResult;
				}
				pPartResult->bValid = false;
				pPartResult->bPass = false;
				pPartResult->nHBin = 0;
				pPartResult->nPartNb = 0;
				pPartResult->nSBin = 0;
				pPartResult->nX = 0;
				pPartResult->nY = 0;

				pPartResult->nPartNb = lstSections[iIndex++].toInt(&pPartResult->bValid);
				iIndex++;
				iIndex++;
				pPartResult->bPass = lstSections[iIndex++].startsWith("PASS", Qt::CaseInsensitive);
				pPartResult->nHBin = lstSections[iIndex++].toInt();
				pPartResult->nSBin = lstSections[iIndex++].toInt();
				pPartResult->nX = lstSections[iIndex++].toInt();
				pPartResult->nY = lstSections[iIndex++].toInt();

				// update Hbin and Sbin counter
				if(!mapHBinCount.contains(pPartResult->nHBin))
				{
					mapHBinCount[pPartResult->nHBin]=0;
					mapHBinGood[pPartResult->nHBin]=false;
				}
				if(!mapSBinCount.contains(pPartResult->nSBin))
				{
					mapSBinCount[pPartResult->nSBin]=0;
					mapSBinGood[pPartResult->nSBin]=false;
				}
				mapHBinCount[pPartResult->nHBin]++;
				mapHBinGood[pPartResult->nHBin]=pPartResult->bPass;
				mapSBinCount[pPartResult->nSBin]++;
				mapSBinGood[pPartResult->nSBin]=pPartResult->bPass;
			}
		}
		else if(strSection.startsWith("$#DUT", Qt::CaseInsensitive))
		{
			// $#DUT1TEST#$
			iSiteNumber = strSection.mid(5,1).toInt();
			strString = ReadLine(*hYokogawaFile);

			// Write PIR
			// Write PIR for parts in this Wafer site
			RecordReadInfo.iRecordType = 5;
			RecordReadInfo.iRecordSubType = 10;
			StdfFile.WriteHeader(&RecordReadInfo);
			StdfFile.WriteByte(1);					// Test head
			StdfFile.WriteByte(iSiteNumber);		// Tester site
			StdfFile.WriteRecord();

			pPartResult = mapPartResult[iSiteNumber];

			// Reset counters
			iTotalTests = 0;
			iTestNumber=iPinNumber=iNumber=0;
			strTestName=strPinName="";

			while(!hYokogawaFile->atEnd())
			{
				strString = ReadLine(*hYokogawaFile);
				if(strString.isEmpty())
					break;
				if(strString.startsWith("$#"))
					break;

                lstSections = strString.split(" ", QString::SkipEmptyParts);
				strSection = lstSections[0].toUpper();

				if(strSection == "TEST#")
				{
					strString = "";
					continue;
				}

				// test result
				//reset values
				iIndex = 0;
				nScale = 0;
				strUnit = "";
				fValue = fLowLimit = fHighLimit = 0;
				bHaveLowLimit = bHaveHighLimit = false;
				bTestAlreadySaved = false;
				bTestPass = true;

				// Check if have the good count
				if(lstSections.count() <= 2)
				{
					m_iLastError = errInvalidFormatLowInRows;
					
					// Convertion failed.
					return false;
				}
				
				iNumber = lstSections[iIndex++].toInt();
				if(iNumber != iTestNumber)
				{
					// New test
					strTestName = strPinName = "";
					iTestNumber = iNumber;
					// 2007/09/04 : Reset PinNumber
					iPinNumber = 0;
				}
				strSection = lstSections[iIndex++];
				if(strSection == "+")
				{
					// MPR as PTR
					bPtrRecord = true;
					// Do not reset iPinNumber = 0;
					// Have to use the same value for "^";
					strTestName = "";
					for(int i=8; i!=lstSections.count(); i++)
						strTestName += lstSections[i]+" ";
					strPinName = "";
					continue;
				}
				else if(strSection == "=")
				{
					// FTR Record
					bPtrRecord = false;
				}
				else if(strSection == "^")
				{
					// Standard PTR
					bPtrRecord = true;
					// same iPinNumber than the last
				}
				else if(strSection == "-")
				{
					// MPR as PTR
					bPtrRecord = true;
					// Have to increment iPinNumber
					// iPinNumber++;
					// case 5189
					iPinNumber = 0;
				}
				else
				{
					// MPR as PTR
					bPtrRecord = true;
					// Ignore the PinNumber : iPinNumber = strSection.toInt();
					// Have to increment iPinNumber
					// iPinNumber++;
					// case 5189
					bool bIsNumber;
					strSection = strSection.remove(QRegExp("\\D"));
					strSection.toInt(&bIsNumber);
					if(bIsNumber)
						iPinNumber = strSection.toInt();
					else
						iPinNumber++;
				}
				// Check if have the good count
				if(lstSections.count() <= 7)
				{
					m_iLastError = errInvalidFormatLowInRows;
					
					// Convertion failed.
					return false;
				}

				bTestPass = lstSections[iIndex++].startsWith("P", Qt::CaseInsensitive);
				fValue = lstSections[iIndex++].toFloat();
				fLowLimit = lstSections[iIndex++].toFloat(&bHaveLowLimit);
				fHighLimit = lstSections[iIndex++].toFloat(&bHaveHighLimit);
				strUnit = lstSections[iIndex++];
				if(strUnit.startsWith("-"))
					strUnit = "";
				iIndex++;
				strPinName = "";
				for(int i=8; i!=lstSections.count(); i++)
					strPinName += lstSections[i]+" ";
				strTestName = strTestName.simplified();
				strPinName = strPinName.simplified();
				if(strTestName.isEmpty())
					strTestName = strPinName;
				else if(strTestName == "-")
					strTestName = strPinName;
				else if(strPinName == "-")
					strPinName = strTestName;
				else
					strPinName = strTestName + " - " + strPinName;
				strTestName = strTestName.simplified();
				strPinName = strPinName.simplified();
				strPinName.replace(',','.').replace(';','.');

				if(lstTestSavedWithLimit.contains(strPinName + QString::number(iTestNumber*10000+iPinNumber)))
					bTestAlreadySaved = true;
				else
				{
					lstTestSavedWithLimit.append(strPinName + QString::number(iTestNumber*10000+iPinNumber));
					UpdateParameterIndexTable(strPinName + QString::number(iTestNumber*10000+iPinNumber));
				}

				
				iTotalTests++;

				if(!strUnit.isEmpty())
					NormalizeLimits(strUnit, nScale);


				if(bPtrRecord)
				{
					// Write PTR
					RecordReadInfo.iRecordType = 15;
					RecordReadInfo.iRecordSubType = 10;
					
					StdfFile.WriteHeader(&RecordReadInfo);
					StdfFile.WriteDword(iTestNumber*10000+iPinNumber);	// Test Number
					StdfFile.WriteByte(1);								// Test head
					StdfFile.WriteByte(iSiteNumber);					// Tester site:1,2,3,4 or 5, etc.
					if(bTestPass)
						bData = 0;		// Test passed
					else
						bData = BIT7;	// Test Failed
					StdfFile.WriteByte(bData);							// TEST_FLG
					bData = BIT6|BIT7;
					StdfFile.WriteByte(bData);							// PARAM_FLG
					StdfFile.WriteFloat(fValue * GS_POW(10.0,nScale));		// Test result
					if(!bTestAlreadySaved)
					{
						// save Parameter name without unit information
						StdfFile.WriteString(strPinName.toLatin1().constData());	// TEST_TXT
						StdfFile.WriteString("");							// ALARM_ID

						bData = 2;	// Valid data.
						if(bHaveLowLimit==false)
							bData |=0x40;
						if(bHaveHighLimit==false)
							bData |=0x80;
						StdfFile.WriteByte(bData);							// OPT_FLAG

						StdfFile.WriteByte(-nScale);						// RES_SCALE
						StdfFile.WriteByte(-nScale);						// LLM_SCALE
						StdfFile.WriteByte(-nScale);						// HLM_SCALE
						StdfFile.WriteFloat(fLowLimit * GS_POW(10.0,nScale));	// LOW Limit
						StdfFile.WriteFloat(fHighLimit * GS_POW(10.0,nScale));	// HIGH Limit
						StdfFile.WriteString(strUnit.toLatin1().constData());		// Units
					}
					StdfFile.WriteRecord();
				}
				else
				{
					// Ftr Record
					RecordReadInfo.iRecordType = 15;
					RecordReadInfo.iRecordSubType = 20;

					StdfFile.WriteHeader(&RecordReadInfo);
					StdfFile.WriteDword(iTestNumber*10000+iPinNumber);	// Test Number
					StdfFile.WriteByte(1);							// Test head
					StdfFile.WriteByte(iSiteNumber);				// Tester site:1,2,3,4 or 5, etc.
					if(bTestPass)
						bData = 0;		// Test passed
					else
						bData = BIT7;	// Test Failed
					StdfFile.WriteByte(bData);							// TEST_FLG

					// save the name of the test
					StdfFile.WriteByte(255);	// opt_flg
					StdfFile.WriteDword(0);		// cycl_cnt
					StdfFile.WriteDword(0);		// rel_vadr
					StdfFile.WriteDword(0);		// rept_cnt
					StdfFile.WriteDword(0);		// num_fail
					StdfFile.WriteDword(0);		// xfail_ad
					StdfFile.WriteDword(0);		// yfail_ad
					StdfFile.WriteWord(0);		// vect_off
					StdfFile.WriteWord(0);		// rtn_icnt
                    StdfFile.WriteWord(0);      // pgm_icnt
                    StdfFile.WriteWord(0);      // fail_pin
					StdfFile.WriteString("");	// vect_name
					StdfFile.WriteString("");	// time_set
					StdfFile.WriteString("");	// op_code
					StdfFile.WriteString(strPinName.toLatin1().constData());	// test_txt: test name
                    /*StdfFile.WriteString("");	// alarm_id
					StdfFile.WriteString("");	// prog_txt
					StdfFile.WriteString("");	// rslt_txt
					StdfFile.WriteByte(0);		// patg_num
                    StdfFile.WriteWord(0);   	// spin_map*/

					StdfFile.WriteRecord();
				}
			}

			// Write PRR
			RecordReadInfo.iRecordType = 5;
			RecordReadInfo.iRecordSubType = 20;
			StdfFile.WriteHeader(&RecordReadInfo);
			StdfFile.WriteByte(1);				// Test head
			StdfFile.WriteByte(iSiteNumber);				// Tester site:1,2,3,4 or 5
            if(pPartResult && pPartResult->bPass)
			{
				StdfFile.WriteByte(0);				// PART_FLG : PASSED
				iTotalGoodBin++;
			}
			else
			{
				StdfFile.WriteByte(8);				// PART_FLG : FAILED
				iTotalFailBin++;
			}
			StdfFile.WriteWord((WORD)iTotalTests);	// NUM_TEST
			StdfFile.WriteWord(pPartResult->nHBin);	// HARD_BIN
			StdfFile.WriteWord(pPartResult->nSBin);	// SOFT_BIN
			StdfFile.WriteWord(pPartResult->nX);	// X_COORD
			StdfFile.WriteWord(pPartResult->nY);	// Y_COORD
			StdfFile.WriteDword(0);					// No testing time known...
			StdfFile.WriteString(QString::number(pPartResult->nPartNb).toLatin1().constData());// PART_ID
			StdfFile.WriteString("");				// PART_TXT
			StdfFile.WriteString("");				// PART_FIX
			StdfFile.WriteRecord();
		}
		else if(strSection.startsWith("$#"))
		{
			// Goto next header
			while(!hYokogawaFile->atEnd())
			{
				strString = ReadLine(*hYokogawaFile).simplified();
				if(strString.startsWith("$#"))
				{
					// Found header
					break;
				}
			}
		}
		else
		{
			// Ignore
			strString = "";
		}
	}


	// Write WRR
	RecordReadInfo.iRecordType = 2;
	RecordReadInfo.iRecordSubType = 20;
	StdfFile.WriteHeader(&RecordReadInfo);
	StdfFile.WriteByte(1);						// Test head
	StdfFile.WriteByte(255);					// Tester site (all)
	StdfFile.WriteDword(m_lEndTime);			// Time of last part tested
	StdfFile.WriteDword(iTotalGoodBin+iTotalFailBin);	// Parts tested: always 5
	StdfFile.WriteDword(0);						// Parts retested
	StdfFile.WriteDword(0);						// Parts Aborted
	StdfFile.WriteDword(iTotalGoodBin);		// Good Parts
	StdfFile.WriteDword((DWORD)-1);				// Functionnal Parts
	StdfFile.WriteString(m_strWaferId.toLatin1().constData());// WaferID
	StdfFile.WriteRecord();

	RecordReadInfo.iRecordType = 1;
	RecordReadInfo.iRecordSubType = 40;

	QMap<int,int>::Iterator itBin;
	for ( itBin = mapHBinCount.begin(); itBin != mapHBinCount.end(); ++itBin ) 
	{
		// Write HBR
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(255);						// Test Head = ALL
		StdfFile.WriteByte(255);						// Test sites = ALL		
		StdfFile.WriteWord(itBin.key());				// HBIN
		StdfFile.WriteDword(mapHBinCount[itBin.key()]);	// Total Bins
		StdfFile.WriteByte(mapHBinGood[itBin.key()] ? 'P' : 'F');
		StdfFile.WriteString("");
		StdfFile.WriteRecord();

	}

	RecordReadInfo.iRecordType = 1;
	RecordReadInfo.iRecordSubType = 50;

	for ( itBin = mapSBinCount.begin(); itBin != mapSBinCount.end(); ++itBin ) 
	{
		// Write SBR
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(255);						// Test Head = ALL
		StdfFile.WriteByte(255);						// Test sites = ALL		
		StdfFile.WriteWord(itBin.key());				// HBIN
		StdfFile.WriteDword(mapSBinCount[itBin.key()]);	// Total Bins
		StdfFile.WriteByte(mapSBinGood[itBin.key()] ? 'P' : 'F');
		StdfFile.WriteString("");
		StdfFile.WriteRecord();

	}

	// Write MRR
	RecordReadInfo.iRecordType = 1; 
	RecordReadInfo.iRecordSubType = 20;
	StdfFile.WriteHeader(&RecordReadInfo);
	StdfFile.WriteDword(m_lEndTime);			// File finish-time.
	StdfFile.WriteRecord();

	// Close STDF file.
	StdfFile.Close();

	// Success
	return true;
}



//////////////////////////////////////////////////////////////////////
// Check if File is compatible with YOKOGAWA format
//////////////////////////////////////////////////////////////////////
bool CGYOKOGAWAtoSTDF::IsCompatible(const char *szFileName)
{
	QString strString;
	QString strSection;
	QString strValue;
	QString	strSite;

	// Open YOKOGAWA file
    QFile f( szFileName );
    if(!f.open( QIODevice::ReadOnly ))
	{
		// Convertion failed.
		return false;
	}
	// Assign file I/O stream
	QTextStream hYokogawaFile(&f);

	// Check if first line is the correct YOKOGAWA header...
	//TEXT REPORT
	
	do
		strString = hYokogawaFile.readLine().simplified();
	while(!strString.isNull() && strString.isEmpty());

	//<< Test Result Data >>                                    2006/03/20 07:07:48
	//$#HEADER#$
	if(!strString.startsWith("<< Test Result Data >>", Qt::CaseInsensitive))
	{
		if(strString.startsWith("$#HEADER#$", Qt::CaseInsensitive))
		{
			// Missing the first line but this is a YOKOGAWA file!
			f.close();
			return true;
		}
		// Incorrect header...this is not a YOKOGAWA file!
		f.close();
		return false;
	}
	
	//$#HEADER#$
	do
		strString = hYokogawaFile.readLine().simplified();
	while(!strString.isNull() && strString.isEmpty());

	if(!strString.startsWith("$#HEADER#$", Qt::CaseInsensitive))
	{
		// Incorrect header...this is not a YOKOGAWA file!
		f.close();
		return false;
	}


	// It's a YOKOGAWA file
	f.close();
	return true;
}


//////////////////////////////////////////////////////////////////////
// Convert 'FileName' YOKOGAWA file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGYOKOGAWAtoSTDF::Convert(const char *YokogawaFileName, const char *strFileNameSTDF)
{
	// No erro (default)
	m_iLastError = errNoError;

	// If STDF file already exists...do not rebuild it...unless dates not matching!
	QFileInfo fInput(YokogawaFileName);
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
			GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(YokogawaFileName).fileName()+"...");
			GexScriptStatusLabel->show();
		}
	}
    QCoreApplication::processEvents();

    if(ReadYokogawaFile(YokogawaFileName,strFileNameSTDF) != true)
	{
		//////////////////////////////////////////////////////////////////////
		// For ProgressBar
		if((GexProgressBar != NULL)
		&& bHideProgressAfter)
			GexProgressBar->hide();
		
		if((GexScriptStatusLabel != NULL)
		&& bHideLabelAfter)
			GexScriptStatusLabel->hide();
		return false;	// Error reading YOKOGAWA file
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
QString CGYOKOGAWAtoSTDF::ReadLine(QTextStream& hFile)
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
