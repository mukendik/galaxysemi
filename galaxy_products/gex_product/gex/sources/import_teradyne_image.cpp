//////////////////////////////////////////////////////////////////////
// import_teradyne_image.cpp: Convert a .TERADYNE_IMAGE (TSMC) file to STDF V4.0
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
#include "import_teradyne_image.h"
#include "import_constants.h"

// File format:
//Lot: 1                     Tester: sdcat07t          Program: ENGR_uN3010ft   
//Device: 1        Station: 1   Site: 0    Date: Sat Oct 14 13:50:46 2006
//
//CHARACTERIZE TESTING
//Sequencer:  VDD_CONTINUITY
//    10 CONTIN_BB_VDD      VDD_contin   -2.00 mA <     0.00 mA     <   2.00 mA
//    11 CONTIN_RF_VDD      VDD_contin   -2.00 mA <     0.12 mA     <   2.00 mA
//    12 CONTIN_CNTRL       VDD_contin   -2.00 mA <     0.00 mA     <   2.00 mA
//
//   107 cont+ VDD_GPS      Ana_contin     150 mV <       -0 mV (F) <   1000 mV
//Bin:  3 
//
//
//Lot: 1                     Tester: sdcat07t          Program: ENGR_uN3010ft   
//Device: 5        Station: 1   Site: 0    Date: Sat Oct 14 14:01:46 2006
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

CTeradyneImageBinInfo::CTeradyneImageBinInfo()
{
	nNbCnt=0;
	nPassFlag=TEST_PASSFLAG_UNKNOWN;
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGTERADYNE_IMAGEtoSTDF::CGTERADYNE_IMAGEtoSTDF()
{
	// Default: TERADYNE_IMAGE parameter list on disk includes all known TERADYNE_IMAGE parameters...
	m_bNewTeradyneImageParameterFound = false;
	m_lStartTime = 0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGTERADYNE_IMAGEtoSTDF::~CGTERADYNE_IMAGEtoSTDF()
{
	QMap<int,CTeradyneImageBinInfo*>::Iterator itMapBin;
	for ( itMapBin = m_qMapBins.begin(); itMapBin != m_qMapBins.end(); ++itMapBin ) 
	{
        delete itMapBin.value();
	}
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGTERADYNE_IMAGEtoSTDF::GetLastError()
{
	m_strLastError = "Import TERADYNE_IMAGE: ";

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
// Load TERADYNE_IMAGE Parameter table from DISK
//////////////////////////////////////////////////////////////////////
void CGTERADYNE_IMAGEtoSTDF::LoadParameterIndexTable(void)
{
	QString	strTeradyneImageTableFile;
	QString	strString;

    strTeradyneImageTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strTeradyneImageTableFile += GEX_TERADYNE_IMAGE_PARAMETERS;

	// Open TERADYNE_IMAGE Parameter table file
    QFile f( strTeradyneImageTableFile );
    if(!f.open( QIODevice::ReadOnly ))
		return;

	// Assign file I/O stream
	QTextStream hTeradyneImageTableFile(&f);

	// Skip comment or empty lines
	do
	{
	  strString = hTeradyneImageTableFile.readLine();
	}
    while((strString.indexOf("----------------------") < 0) && (!hTeradyneImageTableFile.atEnd()));

	// Read lines
	m_pFullTeradyneImageParametersList.clear();
	strString = hTeradyneImageTableFile.readLine();
	while (strString.isNull() == false)
	{
		// Save Parameter name in list
		m_pFullTeradyneImageParametersList.append(strString);
		// Read next line
		strString = hTeradyneImageTableFile.readLine();
	};

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// Save TERADYNE_IMAGE Parameter table to DISK
//////////////////////////////////////////////////////////////////////
void CGTERADYNE_IMAGEtoSTDF::DumpParameterIndexTable(void)
{
	QString		strTeradyneImageTableFile;
	QString		strString;

    strTeradyneImageTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strTeradyneImageTableFile += GEX_TERADYNE_IMAGE_PARAMETERS;

	// Open TERADYNE_IMAGE Parameter table file
    QFile f( strTeradyneImageTableFile );
    if(!f.open( QIODevice::WriteOnly ))
		return;

	// Assign file I/O stream
	QTextStream hTeradyneImageTableFile(&f);

	// First few lines are comments:
	hTeradyneImageTableFile << "############################################################" << endl;
	hTeradyneImageTableFile << "# DO NOT EDIT THIS FILE!" << endl;
    hTeradyneImageTableFile << "# Quantix Examinator: TERADYNE_IMAGE Parameters detected" << endl;
	hTeradyneImageTableFile << "# www.mentor.com" << endl;
    hTeradyneImageTableFile << "# Quantix Examinator reads and writes into this file..." << endl;
	hTeradyneImageTableFile << "-----------------------------------------------------------" << endl;

	// Write lines
	// m_pFullTeradyneImageParametersList.sort();
	for(int nIndex = 0; nIndex < m_pFullTeradyneImageParametersList.count(); nIndex++)
	{
		// Write line
		hTeradyneImageTableFile << m_pFullTeradyneImageParametersList[nIndex] << endl;
	};

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// If Examinator doesn't have this TERADYNE_IMAGE parameter in his dictionnary, have it added.
//////////////////////////////////////////////////////////////////////
void CGTERADYNE_IMAGEtoSTDF::UpdateParameterIndexTable(QString strParamName)
{
	// Check if the table is empty...if so, load it from disk first!
	if(m_pFullTeradyneImageParametersList.isEmpty() == true)
	{
		// Load TERADYNE_IMAGE parameter table from disk...
		LoadParameterIndexTable();
	}
	
	// Check if Parameter name already in table...if not, add it to the list
	// the new full list will be dumped to the disk at the end.
    if(m_pFullTeradyneImageParametersList.indexOf(strParamName) < 0)
	{
		// Update list
		m_pFullTeradyneImageParametersList.append(strParamName);

		// Set flag to force the current TERADYNE_IMAGE table to be updated on disk
		m_bNewTeradyneImageParameterFound = true;
	}
}


//////////////////////////////////////////////////////////////////////
// Normalize test limits when writing into PTR.
//////////////////////////////////////////////////////////////////////
void CGTERADYNE_IMAGEtoSTDF::NormalizeLimits(QString &strUnit, int &nScale)
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
// Check if File is compatible with TERADYNE_IMAGE format
//////////////////////////////////////////////////////////////////////
bool CGTERADYNE_IMAGEtoSTDF::IsCompatible(const char *szFileName)
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
	QTextStream hTeradyneImageFile(&f);

	// Check if first line is the correct TERADYNE_IMAGE header...
	//Wafer:    Start Time: Wed Jan  8 09:13:31 2003
	//Lot: 1                     Tester: sdcat07t          Program: ENGR_uN3010ft   
	//Device: 1        Station: 1   Site: 0    Date: Sat Oct 14 13:50:46 2006
	//
	//CHARACTERIZE TESTING
	
	do
		strString = hTeradyneImageFile.readLine();
	while(!strString.isNull() && strString.isEmpty());

	if((strString.section(":",0,0).simplified().toUpper() != "WAFER")
	&& (strString.section(":",0,0).simplified().toUpper() != "LOT"))
	{
		// Incorrect header...this is not a TERADYNE_IMAGE file!
		// Close file
		f.close();
		return false;
	}

	// Close file
	f.close();

	return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the TERADYNE_IMAGE file
//////////////////////////////////////////////////////////////////////
bool CGTERADYNE_IMAGEtoSTDF::ReadTeradyneImageFile(const char *TeradyneImageFileName,const char *strFileNameSTDF)
{
	QString strString;
	QString strSection;
	QString strValue;
	QString	strSite;

	// Open TERADYNE_IMAGE file
    QFile f( TeradyneImageFileName );
    if(!f.open( QIODevice::ReadOnly ))
	{
		// Failed Opening TERADYNE_IMAGE file
		m_iLastError = errOpenFail;
		
		// Convertion failed.
		return false;
	}
	// Assign file I/O stream
	QTextStream hTeradyneImageFile(&f);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	iProgressStep = 0;
	iNextFilePos = 0;
	iFileSize = f.size() + 1;
    QCoreApplication::processEvents();
	
	// Check if first line is the correct TERADYNE_IMAGE header...
	//Wafer:    Start Time: Wed Jan  8 09:13:31 2003
	//Lot: 1                     Tester: sdcat07t          Program: ENGR_uN3010ft   
	//Device: 1        Station: 1   Site: 0    Date: Sat Oct 14 13:50:46 2006
	//
	//CHARACTERIZE TESTING
	
	strString = ReadLine(hTeradyneImageFile);

	if((strString.section(":",0,0).simplified().toUpper() != "WAFER")
	&& (strString.section(":",0,0).simplified().toUpper() != "LOT"))
	{
		// Incorrect header...this is not a TERADYNE_IMAGE file!
		m_iLastError = errInvalidFormat;
		
		// Convertion failed.
		// Close file
		f.close();
		return false;
	}

	// Read TERADYNE_IMAGE information
	while(!hTeradyneImageFile.atEnd())
	{
		if(strString.isEmpty())
		{
			hTeradyneImageFile.skipWhiteSpace();
			strString = ReadLine(hTeradyneImageFile);
		}

		strSection = strString.section(":",0,0).simplified().toUpper();
		strString = strString.section(":",1);

		if(strSection == "WAFER")
		{
			m_strWaferId = strString.left(4).simplified();
			if(m_strWaferId.isEmpty())
				m_strWaferId = "1";
			strString = strString.mid(4);
		}
		else if(strSection == "START TIME")
		{
			QDateTime clDateTime;
			clDateTime.setTimeSpec(Qt::UTC);
			m_lStartTime = clDateTime.fromString(strString.simplified()).toTime_t();
			strString = "";
		}
		else if(strSection == "LOT")
		{
			m_strLotId = strString.left(21).simplified();
			strString = strString.mid(21);
		}
		else if(strSection == "TESTER")
		{
			m_strTesterId = strString.left(18).simplified();
			strString = strString.mid(18);
		}
		else if(strSection == "PROGRAM")
		{
			m_strProgramId = strString.simplified();
			strString = "";
		}
		else if(strSection == "DEVICE")
		{// ignore
			strString = strString.mid(9);
		}
		else if(strSection == "STATION")
		{
			m_strStationId = strString.left(4).simplified();
			strString = strString.mid(4);
		}
		else if(strSection == "SITE")
		{// ignore
			strString = strString.mid(5);
		}
		else if(strSection == "DATE")
		{
			QDateTime clDateTime;
			clDateTime.setTimeSpec(Qt::UTC);
			m_lStartTime = clDateTime.fromString(strString.simplified()).toTime_t();
			strString = "";
		}
		else if(strSection == "TEST PROGRAM")
		{
			m_strTestProgram = strString.left(13).simplified();
			strString = strString.mid(13);
		}
		else if(strSection == "TEST FLOW")
		{
			m_strTestFlow = strString.simplified();
			strString = "";
		}
		else if((strSection == "CHARACTERIZE TESTING")
			 || (strSection == "SEQUENCER"))
		{
			break;
		}
		else
		{
			// Ignore this incorrect key
			continue;
			
			// Incorrect header...this is not a TERADYNE_IMAGE file!
			m_iLastError = errInvalidFormat;
			
			// Convertion failed.
			// Close file
			f.close();
			return false;
		}
	}

	// It's a Teradyne Image file
	//Restart at the beggining of the file
	hTeradyneImageFile.seek(0);

	if(!WriteStdfFile(&hTeradyneImageFile,strFileNameSTDF))
	{
		QFile::remove(strFileNameSTDF);
		f.close();
		return false;
	}

	f.close();
	// All TERADYNE_IMAGE file read...check if need to update the TERADYNE_IMAGE Parameter list on disk?
	if(m_bNewTeradyneImageParameterFound == true)
		DumpParameterIndexTable();

	// Success parsing TERADYNE_IMAGE file
	return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from TERADYNE_IMAGE data parsed
//////////////////////////////////////////////////////////////////////
bool CGTERADYNE_IMAGEtoSTDF::WriteStdfFile(QTextStream *hTeradyneImageFile,const char *strFileNameSTDF)
{
	// now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open((char*)strFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
	{
		// Failed importing TERADYNE_IMAGE file into STDF database
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
	StdfFile.WriteByte(m_strStationId.toInt());	// Station
	StdfFile.WriteByte((BYTE) 'P');				// Test Mode = PRODUCTION
	StdfFile.WriteByte((BYTE) ' ');				// rtst_cod
	StdfFile.WriteByte((BYTE) ' ');				// prot_cod
	StdfFile.WriteWord(65535);					// burn_tim
	StdfFile.WriteByte((BYTE) ' ');				// cmod_cod
	StdfFile.WriteString(m_strLotId.toLatin1().constData());		// Lot ID
	StdfFile.WriteString("");					// Part Type / Product ID
	StdfFile.WriteString("");					// Node name
	StdfFile.WriteString(m_strTesterId.toLatin1().constData());	// Tester Type
	StdfFile.WriteString(m_strProgramId.toLatin1().constData());	// Job name
	StdfFile.WriteString("");					// Job rev
	StdfFile.WriteString("");					// sublot-id
	StdfFile.WriteString("");					// operator
	StdfFile.WriteString("");					// exec-type
	StdfFile.WriteString("");					// exe-ver
	StdfFile.WriteString(m_strWaferId.isEmpty()?(char*)"":(char*)"WAFER");// test-cod
	StdfFile.WriteString("");					// test-temperature
	// Construct custom Galaxy USER_TXT 
	QString	strUserTxt;
	strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;	
	strUserTxt += ":";
	strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
	strUserTxt += ":TERADYNE_IMAGE";
	StdfFile.WriteString((char *)strUserTxt.toLatin1().constData());	// user-txt
	StdfFile.WriteString("");					// aux-file
	StdfFile.WriteString("");					// package-type
	StdfFile.WriteString("");					// familyID
	StdfFile.WriteString("");					// Date-code
	StdfFile.WriteString("");					// Facility-ID
	StdfFile.WriteString("");					// FloorID
	StdfFile.WriteString(m_strProcessId.toLatin1().constData());	// ProcessID
	StdfFile.WriteString("");					// Oper-Freq
	StdfFile.WriteString("");					// Spec-Nam
	StdfFile.WriteString("");					// Spec-Ver
	StdfFile.WriteString(m_strTestFlow.toLatin1().constData());	// Flow-Id

	StdfFile.WriteRecord();

	if(!m_strWaferId.isEmpty())
	{
		// Write WIR of new Wafer.
		RecordReadInfo.iRecordType = 2;
		RecordReadInfo.iRecordSubType = 10;
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(1);							// Test head
		StdfFile.WriteByte(255);						// Tester site (all)
		StdfFile.WriteDword(m_lStartTime);				// Start time
		StdfFile.WriteString(m_strWaferId.toLatin1().constData());	// WaferID
		StdfFile.WriteRecord();
	}
	
	// Write Test results for each line read.
	QString		strString;
	QString		strSection;
	WORD		wSoftBin,wHardBin;
	long		iTotalGoodBin,iTotalFailBin;
	long		iTotalTests,iPartNumber;
	bool		bIsInteger;
	bool		bPassStatus;
	bool		bWarningStatus;
	BYTE		bData;
	int			iOffset;

	int			iBin;
	int			iXWafer=0, iYWafer=0;
	int			iSiteNumber=0;
	int			iTestNumber;
	bool		bTestPass;
	bool		bTestWarning;
	QString		strTestName;
	QString		strUnit;
	int			nScale;
	float		fValue;
	float		fLowLimit, fHighLimit;
	bool		bStrictLowLimit, bStrictHighLimit;
	bool		bHaveLowLimit, bHaveHighLimit;
	bool		bTestAlreadySaved;
	QStringList lstTestSavedWithLimit;

	// Reset counters
	iTotalGoodBin=iTotalFailBin=0;
	iPartNumber=0;
	iOffset=0;

	// Write all Parameters read on this file : PIR,PTR...,PRR, PIR,PTR..., PRR

	//Wafer:    Start Time: Wed Jan  8 09:13:31 2003 (optional)
	//Lot: 1                     Tester: sdcat07t          Program: ENGR_uN3010ft   
	//Device: 1        Station: 1   Site: 0    Date: Sat Oct 14 13:50:46 2006
	//
	//CHARACTERIZE TESTING
	
	// Read TERADYNE_IMAGE information
	while(!hTeradyneImageFile->atEnd())
	{
		
		if(strString.simplified().isEmpty())
			strString = ReadLine(*hTeradyneImageFile);
		if(strString.simplified().isEmpty())
			continue;

		strSection = strString.section(":",0,0).simplified().toUpper();
		strString = strString.section(":",1);

		if(strSection == "WAFER")
		{// ignore
			strString = "";
		}
		else if(strSection.left(12) == "END OF WAFER")
		{//End of Wafer  Part Count: 179   Finish Time: Wed Jan  8 09:21:16 2003
			strString = strString.mid(6);
		}
		else if(strSection == "FINISH TIME")
		{
			QDateTime clDateTime;
			clDateTime.setTimeSpec(Qt::UTC);
			m_lStartTime = clDateTime.fromString(strString.simplified()).toTime_t();
			strString = "";
		}
		else if(strSection == "LOT")
		{// ignore
			strString = "";
		}
		else if(strSection == "DEVICE")
		{
			iPartNumber = strString.left(9).toInt();
			strString = strString.mid(9);
		}
		else if(strSection == "STATION")
		{
			m_strStationId = strString.left(4).simplified();
			strString = strString.mid(4);
		}
		else if(strSection == "SITE")
		{
			iSiteNumber = strString.left(5).toInt();
			strString = strString.mid(5);
		}
		else if(strSection == "DATE")
		{
			QDateTime clDateTime;
			clDateTime.setTimeSpec(Qt::UTC);
			m_lStartTime = clDateTime.fromString(strString.simplified()).toTime_t();
			strString = "";
		}
		else if((strSection == "CHARACTERIZE TESTING")
			 || (strSection == "SEQUENCER"))
		{
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
			bWarningStatus = false;
			iBin = 0;

			// Reset counters
			iTotalTests = 0;

			while(!hTeradyneImageFile->atEnd())
			{
				strString = ReadLine(*hTeradyneImageFile);
				if(strString.simplified().isEmpty())
					continue;

				strSection = strString.section(":",0,0).simplified().toUpper();
				
				if(strSection == "BIN")
				{
					// End for this Part
					//Bin:  14 
					// or
					//Bin:  61     Wafer Coordinates: ( -1 , -6)

					iXWafer = iYWafer = -32768;
					iBin = strString.mid(4,8).toInt();
					if(!m_qMapBins.contains(iBin))
					{
						CTeradyneImageBinInfo *pBin = new CTeradyneImageBinInfo();
						
						pBin->nNbCnt = 0;
						m_qMapBins[iBin] = pBin;
							if(!bPassStatus)
								m_qMapBins[iBin]->nPassFlag = TEST_PASSFLAG_FAIL;
							else if(!bWarningStatus)
								m_qMapBins[iBin]->nPassFlag = TEST_PASSFLAG_PASS;
							else 
								m_qMapBins[iBin]->nPassFlag = TEST_PASSFLAG_UNKNOWN;
					}
					else
					{
						// update Pass Flag if more information
						if(m_qMapBins[iBin]->nPassFlag == TEST_PASSFLAG_UNKNOWN)
						{
							if(!bPassStatus)
								m_qMapBins[iBin]->nPassFlag = TEST_PASSFLAG_FAIL;
							else if(!bWarningStatus)
								m_qMapBins[iBin]->nPassFlag = TEST_PASSFLAG_PASS;
						}

					}
					m_qMapBins[iBin]->nNbCnt++;

                    if(strString.contains("Wafer", Qt::CaseInsensitive))
					{
						strString = strString.mid(33,8);
						iXWafer = strString.section(",",0,0).toInt();
						iYWafer = strString.section(",",1).toInt();
					}
					strString = "";
					break;
				}
				else if(strSection == "SEQUENCER")
					continue;

				strString.left(6).toInt(&bIsInteger);
				if(!bIsInteger)
					continue;

				//reset values
				iTestNumber = nScale = 0;
				strTestName = strUnit = "";
				fValue = fLowLimit = fHighLimit = 0;
				bStrictLowLimit = bStrictHighLimit = bHaveLowLimit = bHaveHighLimit = false;
				bTestAlreadySaved = false;
				bTestPass = true;
				bTestWarning = false;

				// Allow a possible additional num for test number
				if(strString.section(" ",0,0).length() > 6)
					iOffset = strString.section(" ",0,0).length() - 6;
				else
					iOffset = 0;

				iTestNumber = strString.left(6+iOffset).toInt();
				strTestName = strString.mid(6+iOffset,19).simplified();
				if(lstTestSavedWithLimit.contains(QString::number(iTestNumber)+" "+strTestName))
					bTestAlreadySaved = true;
				else
				{
					lstTestSavedWithLimit.append(QString::number(iTestNumber)+" "+strTestName);
					UpdateParameterIndexTable(QString::number(iTestNumber)+" "+strTestName);
				}

				// Compute Test# (add user-defined offset)
				//iTestNumber = (long) lstTestSavedWithLimit.findIndex(QString::number(iTestNumber)+" "+strTestName);
				//iTestNumber += GEX_TESTNBR_OFFSET_TERADYNE_IMAGE;// Test# offset
				
				if(strString.contains("(F)"))
				{
					bPassStatus = false;
					bTestPass = false;
					strString.replace("(F)","   ");
				}
				else
					bTestPass = true;

				if(strString.contains("(A)"))
				{
					bWarningStatus = true;
					bTestWarning = true;
					strString.replace("(A)","   ");
				}

				iTotalTests++;
				
                if(strString.contains("Failing Pins", Qt::CaseInsensitive))
				{
					// Functional parameter

					//000000000011111111112222222222333333333344444444445555555555666666666677777777778888888888
					//012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
					//415000 corr1               Failing Pins:  0    
					//       Pattern: corr1_v1                                                                    
					//434000 Vol_2p25mA_Check    Halt Vector:     32   Halt Cycle:    492   Failing Pins: 14 (F)
					//       Pattern: gpio_test                                                                       
 					//         Failed Pins:
					//                 a_19 : 15       d_3 : 17       d_9 : 21       d_2 : 25    
					//                  d_6 : 27       d_4 : 28       d_5 : 57      d_12 : 59    
					//                 a_16 : 136     d_10 : 146     d_11 : 149      d_1 : 158   
					//                  a_0 : 176     d_13 : 189   
					
					int		nHaltVector;
					int		nHaltCycle;
					int		nFailingPins;
					QString strPattern;
					BYTE	bOptFlag;
					bOptFlag = BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6|BIT7;
					nHaltVector = nHaltCycle = nFailingPins = 0;

                    if(strString.indexOf("Failing Pins") > 40)
					{
						// have fail information
						bool	bIsInt = false;
						nHaltVector = strString.section("Halt Vector:",1).left(8).toInt(&bIsInt);
						if(bIsInt)
							bOptFlag &= ~BIT5;
						nHaltCycle = strString.section("Halt Cycle:",1).left(8).toInt(&bIsInt);
						if(bIsInt)
							bOptFlag &= ~BIT0;
						nFailingPins = strString.section("Failing Pins:",1).toInt(&bIsInt);
						if(bIsInt)
							bOptFlag &= ~BIT3;
					}
					strString = ReadLine(*hTeradyneImageFile);
					strPattern = strString.section("Pattern:",1).simplified();

					// Write FTR
					RecordReadInfo.iRecordType = 15;
					RecordReadInfo.iRecordSubType = 20;

					StdfFile.WriteHeader(&RecordReadInfo);
					StdfFile.WriteDword(iTestNumber);				// Test Number
					StdfFile.WriteByte(1);							// Test head
					StdfFile.WriteByte(iSiteNumber);				// Tester site:1,2,3,4 or 5, etc.
					if(bTestPass)
						bData = 0;		// Test passed
					else
						bData = BIT7;	// Test Failed
					StdfFile.WriteByte(bData);							// TEST_FLG

					// save the name of the test
					StdfFile.WriteByte(bOptFlag);		// opt_flg
					StdfFile.WriteDword(nHaltCycle);	// cycl_cnt
					StdfFile.WriteDword(0);				// rel_vadr
					StdfFile.WriteDword(0);				// rept_cnt
					StdfFile.WriteDword(nFailingPins);	// num_fail
					StdfFile.WriteDword(0);				// xfail_ad
					StdfFile.WriteDword(0);				// yfail_ad
					StdfFile.WriteWord(nHaltVector);	// vect_off
					StdfFile.WriteWord(0);				// rtn_icnt
					StdfFile.WriteWord(0);
					StdfFile.WriteWord(0);
					StdfFile.WriteString(strPattern.toLatin1().constData());	// vect_name
					StdfFile.WriteString("");	// time_set
					StdfFile.WriteString("");	// op_code
					StdfFile.WriteString(strTestName.toLatin1().constData());	// test_txt: test name
					StdfFile.WriteString("");	// alarm_id
					StdfFile.WriteString("");	// prog_txt
					StdfFile.WriteString("");	// rslt_txt

						
					StdfFile.WriteRecord();
				}
				else
				{
					// Parametric test

					//000000000011111111112222222222333333333344444444445555555555666666666677777777778888888888
					//012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
					//    10 CONTIN_BB_VDD      VDD_contin   -2.00 mA <    -0.00 mA     <   2.00 mA
					// For each result

					// low limit
					strSection = strString.mid(38+iOffset,9).simplified();
					if(!strSection.isEmpty())
					{
						fLowLimit = strSection.section(" ",0,0).toFloat();
						strUnit = strSection.section(" ",1);
						strSection = strString.mid(47+iOffset,3).simplified();
						bStrictLowLimit = (strSection == "<");
						bHaveLowLimit = true;
					}


					// Test result
					strSection = strString.mid(50+iOffset,15).simplified();
					fValue = strSection.section(" ",0,0).toFloat();
					
					if(strUnit.length() < strSection.section(" ",1).simplified().length())
						strUnit = strSection.section(" ",1).simplified();

					// high limit
					strSection = strString.mid(65+iOffset,3).simplified();
					if(!strSection.isEmpty())
					{
						bStrictHighLimit = (strSection == "<");
						strSection = strString.mid(68+iOffset).simplified();
						fHighLimit = strSection.section(" ",0,0).toFloat();
						if(strUnit.length() < strSection.section(" ",1).simplified().length())
							strUnit = strSection.section(" ",1).simplified();
						bHaveHighLimit = true;
					}

					if(!strUnit.isEmpty())
						NormalizeLimits(strUnit, nScale);


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
					if(bTestWarning)
						bData |= BIT0;
					StdfFile.WriteByte(bData);							// TEST_FLG
					bData = 0;
					if(!bStrictLowLimit)
						bData |= BIT6;
					if(!bStrictHighLimit)
						bData |= BIT7;
					StdfFile.WriteByte(bData);								// PARAM_FLG
					StdfFile.WriteFloat(fValue * GS_POW(10.0,nScale));		// Test result
					if(!bTestAlreadySaved)
					{
						// save Parameter name without unit information
						StdfFile.WriteString(strTestName.toLatin1().constData());	// TEST_TXT
						StdfFile.WriteString("");							// ALARM_ID

						bData = 2;	// Valid data.
						if(!bHaveLowLimit)
							bData |= BIT6;
						if(!bHaveHighLimit)
							bData |= BIT7;
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
			}
			// Write PRR
			RecordReadInfo.iRecordType = 5;
			RecordReadInfo.iRecordSubType = 20;
			StdfFile.WriteHeader(&RecordReadInfo);
			StdfFile.WriteByte(1);						// Test head
			StdfFile.WriteByte(iSiteNumber);			// Tester site:1,2,3,4 or 5
			if(bPassStatus == true)
			{
				StdfFile.WriteByte(0);				// PART_FLG : PASSED
				iTotalGoodBin++;
			}
			else
			{
				StdfFile.WriteByte(8);				// PART_FLG : FAILED
				iTotalFailBin++;
			}
			wSoftBin = wHardBin = iBin;
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
			strString = "";
		}
		else
		{
			// Ignore
			strString = "";
		}
	}

	if(!m_strWaferId.isEmpty())
	{
		// Write WRR for last wafer inserted
		RecordReadInfo.iRecordType = 2;
		RecordReadInfo.iRecordSubType = 20;
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(1);						// Test head
		StdfFile.WriteByte(255);					// Tester site (all)
		StdfFile.WriteDword(0);						// Time of last part tested
		StdfFile.WriteDword(iPartNumber);			// Parts tested: always 5
		StdfFile.WriteDword(0);						// Parts retested
		StdfFile.WriteDword(0);						// Parts Aborted
		StdfFile.WriteDword(iTotalGoodBin);			// Good Parts
		StdfFile.WriteDword(4294967295UL);			// Functionnal Parts
		StdfFile.WriteString(m_strWaferId.toLatin1().constData());	// WaferID
		StdfFile.WriteRecord();
	}
	

	RecordReadInfo.iRecordType = 1;
	RecordReadInfo.iRecordSubType = 40;
	QMap<int,CTeradyneImageBinInfo*>::Iterator itMapBin;
	for ( itMapBin = m_qMapBins.begin(); itMapBin != m_qMapBins.end(); ++itMapBin ) 
	{
		// Write HBR
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(255);						// Test Head = ALL
		StdfFile.WriteByte(255);						// Test sites = ALL		
		StdfFile.WriteWord(itMapBin.key());	// HBIN = 0
        StdfFile.WriteDword(itMapBin.value()->nNbCnt);	// Total Bins
        if(itMapBin.value()->nPassFlag == TEST_PASSFLAG_UNKNOWN)
			StdfFile.WriteByte(' ');
        else if(itMapBin.value()->nPassFlag == TEST_PASSFLAG_PASS)
			StdfFile.WriteByte('P');
		else
			StdfFile.WriteByte('F');
		StdfFile.WriteString("");
		StdfFile.WriteRecord();
	}

	RecordReadInfo.iRecordType = 1;
	RecordReadInfo.iRecordSubType = 50;

	for ( itMapBin = m_qMapBins.begin(); itMapBin != m_qMapBins.end(); ++itMapBin ) 
	{
		// Write SBR
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(255);						// Test Head = ALL
		StdfFile.WriteByte(255);						// Test sites = ALL		
		StdfFile.WriteWord(itMapBin.key());	// HBIN = 0
        StdfFile.WriteDword(itMapBin.value()->nNbCnt);	// Total Bins
        if(itMapBin.value()->nPassFlag == TEST_PASSFLAG_UNKNOWN)
			StdfFile.WriteByte(' ');
        else if(itMapBin.value()->nPassFlag == TEST_PASSFLAG_PASS)
			StdfFile.WriteByte('P');
		else
			StdfFile.WriteByte('F');
		StdfFile.WriteString("");
		StdfFile.WriteRecord();
	}

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
// Convert 'FileName' TERADYNE_IMAGE file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGTERADYNE_IMAGEtoSTDF::Convert(const char *TeradyneImageFileName, const char *strFileNameSTDF)
{
	// No erro (default)
	m_iLastError = errNoError;

	// If STDF file already exists...do not rebuild it...unless dates not matching!
	QFileInfo fInput(TeradyneImageFileName);
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
			GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(TeradyneImageFileName).fileName()+"...");
			GexScriptStatusLabel->show();
		}
	}
    QCoreApplication::processEvents();

    if(ReadTeradyneImageFile(TeradyneImageFileName,strFileNameSTDF) != true)
	{
		//////////////////////////////////////////////////////////////////////
		// For ProgressBar
		if((GexProgressBar != NULL)
		&& bHideProgressAfter)
			GexProgressBar->hide();
		
		if((GexScriptStatusLabel != NULL)
		&& bHideLabelAfter)
			GexScriptStatusLabel->hide();
		return false;	// Error reading TERADYNE_IMAGE file
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
QString CGTERADYNE_IMAGEtoSTDF::ReadLine(QTextStream& hFile)
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
	while(!strString.isNull() && (strString.trimmed().isEmpty() || strString.trimmed().startsWith("*")));

	return strString;

}
