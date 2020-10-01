//////////////////////////////////////////////////////////////////////
// import_eagle_datalog.cpp: Convert a EAGLE_DATALOG file to STDF V4.0
//////////////////////////////////////////////////////////////////////

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
#include "import_eagle_datalog.h"
#include "import_constants.h"
#include "gqtl_global.h"

// File format:
//120,1,"ETS300",2,9,"0641001pxaa","W022","Allegro MicroSystems Inc.","Eagle23","4.04f","2.03",
//125,W,"","","Tokyo Electron Limited P8 Prober","",
//140,1,"c:\ETS\WORK\PR3980PEA.DLL","PR3980PEA - Eagle Test Systems Application","1.0","05/14/2001","11/21/2006",
//140,2,"c:\ETS\WORK\PR3980PEA.pds","PR3980PEA","1.00","05/14/2001","11/21/2006",
//140,3,"c:\ETS\WORK\PR3980PEA.vec","<not available>","1.0","05/14/2001","04/16/2004",
//140,4,"PR3980PEA","AllegroÄMotorDrivers","1.0","05/14/2001","11/21/2006",
//10,1.0,0,ì,-10,"S/N","Hardware_Eagle_ID",
//10,1.1,0,ì,-10,"S/N","Hardware_DibBoard_ID",
//...
//10,16.30,0,1,1,"P/F","Timing_Out2A_MinSetupHold",
//10,16.31,0,1,1,"P/F","Timing_Out2B_MinSetupHold",
//100,1.0,"    ","P",23,
//100,1.1,"    ","P",11,
//100,1.2,"    ","P",0,
//100,1.3,"    ","P",1,
//100,1.4,"    ","P",-0.625,
//...
//100,16.31,"    ","P",1,
//130,2,"11/29/2006  08:51:59","1","P",23,1,1,1,
//100,1.0,"    ","P",23,
//...
//100,16.31,"    ","P",1,
//130,1,"11/29/2006  09:53:13","1275","P",13,30,1,1,
//999,
//50,1,"P",1152,"Good DUT",
//50,2,"F",0,"Ibb currents",
//50,3,"F",5,"Idd currents",
//...
//50,32,"A",0,"Alarm Bin",
//2,1.-1,"Continuity",1264,11,
//1,1.0,"Hardware_Eagle_ID",1275,0,
//1,1.1,"Hardware_DibBoard_ID",1275,0,
//...
//1,5.118,"VCP_PWM50kHz",1227,0,
//2,6.-1,"InputLogicCurrent",1266,0,
//1,6.0,"ILogic_STEP_VDD3V_L",1227,0,
//...
//1,16.31,"Timing_Out2B_MinSetupHold",1152,0,
//2,17.-1,"Compbuf",1252,0,
//2,18.-1,"PWM",1252,0,
//3,1275,1152,1168889,4033214,
//250,1,1,581,
//250,1,2,0,
//250,1,3,4,
//...
//250,1,32,0,
//202,1,1.-1,642,4,
//201,1,1.0,646,0,
//201,1,1.1,646,0,
//201,1,1.2,646,0,
//...
//202,1,18.-1,635,0,
//203,1,646,581,
//250,2,1,571,
//250,2,2,0,
//201,2,16.31,571,0,
//202,2,17.-1,617,0,
//202,2,18.-1,617,0,
//203,2,629,571,
//300,-1,1.0,1275,23,0,23,23,
//301,-1,1.0,12,0,1,1275,
//300,-1,1.1,1275,11,0,11,11,
//301,-1,1.1,12,0,1,1275,
//...
//300,-1,16.31,1152,1,0,1,1,
//301,-1,16.31,12,0,11,1152,
//300,1,1.0,646,23,0,23,23,
//301,1,1.0,12,0,1,646,
//300,1,1.1,646,11,0,11,11,
//...
//301,2,16.31,12,0,11,571,
//30,"11/29/2006  08:51:59","11/29/2006  09:53:13",



// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar


#define TEST_PASSFLAG_UNKNOWN	0
#define TEST_PASSFLAG_PASS		1
#define TEST_PASSFLAG_FAIL		2
#define TEST_PASSFLAG_ALARM		3


#define BIT0			0x01
#define BIT1			0x02
#define BIT2			0x04
#define BIT3			0x08
#define BIT4			0x10
#define BIT5			0x20
#define BIT6			0x40
#define BIT7			0x80


CGEagleDatalogParameter::CGEagleDatalogParameter()
{
	m_uiTestNumLeft=m_uiTestNumRight=0;
	m_nScale=0;
	m_fLowLimit=m_fHighLimit=0;
	m_bValidLowLimit=m_bValidHighLimit=m_bStaticHeaderWritten=false;	
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGEAGLE_DATALOGtoSTDF::CGEAGLE_DATALOGtoSTDF()
{
	// Default: EAGLE_DATALOG parameter list on disk includes all known EAGLE_DATALOG parameters...
	m_bNewEagleDatalogParameterFound = false;
	m_lStartTime=-1;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGEAGLE_DATALOGtoSTDF::~CGEAGLE_DATALOGtoSTDF()
{
	QMap<QString,CGEagleDatalogParameter*>::Iterator itMapParam;
	for ( itMapParam = m_qMapParameterList.begin(); itMapParam != m_qMapParameterList.end(); ++itMapParam ) 
	{
        delete itMapParam.value();
	}
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGEAGLE_DATALOGtoSTDF::GetLastError()
{
	m_strLastError = "Import EAGLE_DATALOG: ";

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
		case errTestNumberOverflow:
            m_strLastError += "Invalid file format: Quantix generated test number is too big";
			break;	
		case errLicenceExpired:
			m_strLastError += "License has expired or Data file out of date...";
			break;	
	}
	// Return Error Message
	return m_strLastError;
}

//////////////////////////////////////////////////////////////////////
// Load EAGLE_DATALOG Parameter table from DISK
//////////////////////////////////////////////////////////////////////
void CGEAGLE_DATALOGtoSTDF::LoadParameterIndexTable(void)
{
	QString	strEagleDatalogTableFile;
	QString	strString;

    strEagleDatalogTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strEagleDatalogTableFile += GEX_EAGLE_DATALOG_PARAMETERS;

	// Open EAGLE_DATALOG Parameter table file
    QFile f( strEagleDatalogTableFile );
    if(!f.open( QIODevice::ReadOnly ))
		return;

	// Assign file I/O stream
	QTextStream hEagleDatalogTableFile(&f);

	// Skip comment or empty lines
	do
	{
	  strString = hEagleDatalogTableFile.readLine();
	}
    while((strString.indexOf("----------------------") < 0) && (!hEagleDatalogTableFile.atEnd()));

	// Read lines
	m_pFullEagleDatalogParametersList.clear();
	strString = hEagleDatalogTableFile.readLine();
	while (strString.isNull() == false)
	{
		// Save Parameter name in list
		m_pFullEagleDatalogParametersList.append(strString);
		// Read next line
		strString = hEagleDatalogTableFile.readLine();
	};

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// Save EAGLE_DATALOG Parameter table to DISK
//////////////////////////////////////////////////////////////////////
void CGEAGLE_DATALOGtoSTDF::DumpParameterIndexTable(void)
{
	QString		strEagleDatalogTableFile;

    strEagleDatalogTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strEagleDatalogTableFile += GEX_EAGLE_DATALOG_PARAMETERS;

	// Open EAGLE_DATALOG Parameter table file
    QFile f( strEagleDatalogTableFile );
    if(!f.open( QIODevice::WriteOnly ))
		return;

	// Assign file I/O stream
	QTextStream hEagleDatalogTableFile(&f);

	// First few lines are comments:
	hEagleDatalogTableFile << "############################################################" << endl;
	hEagleDatalogTableFile << "# DO NOT EDIT THIS FILE!" << endl;
    hEagleDatalogTableFile << "# Quantix Examinator: EAGLE_DATALOG Parameters detected" << endl;
	hEagleDatalogTableFile << "# www.mentor.com" << endl;
    hEagleDatalogTableFile << "# Quantix Examinator reads and writes into this file..." << endl;
	hEagleDatalogTableFile << "-----------------------------------------------------------" << endl;

	// Write lines
	// m_pFullEagleDatalogParametersList.sort();
	for (QStringList::const_iterator
		 iter  = m_pFullEagleDatalogParametersList.begin();
		 iter != m_pFullEagleDatalogParametersList.end(); ++iter) {
		// Write line
		hEagleDatalogTableFile << *iter << endl;
	}

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// If Examinator doesn't have this EAGLE_DATALOG parameter in his dictionnary, have it added.
//////////////////////////////////////////////////////////////////////
void CGEAGLE_DATALOGtoSTDF::UpdateParameterIndexTable(QString strParamName)
{
	// Check if the table is empty...if so, load it from disk first!
	if(m_pFullEagleDatalogParametersList.isEmpty() == true)
	{
		// Load EAGLE_DATALOG parameter table from disk...
		LoadParameterIndexTable();
	}
	
	// Check if Parameter name already in table...if not, add it to the list
	// the new full list will be dumped to the disk at the end.
    if(m_pFullEagleDatalogParametersList.indexOf(strParamName) < 0)
	{
		// Update list
		m_pFullEagleDatalogParametersList.append(strParamName);

		// Set flag to force the current EAGLE_DATALOG table to be updated on disk
		m_bNewEagleDatalogParameterFound = true;
	}
}


//////////////////////////////////////////////////////////////////////
// Normalize test limits when writing into PTR.
//////////////////////////////////////////////////////////////////////
void CGEAGLE_DATALOGtoSTDF::NormalizeLimits(QString &strUnit, int &nScale)
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
// Check if File is compatible with EAGLE_DATALOG format
//////////////////////////////////////////////////////////////////////
bool CGEAGLE_DATALOGtoSTDF::IsCompatible(const char *szFileName)
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
	QTextStream hEagleDatalogFile(&f);

	// Check if first line is the correct EAGLE_DATALOG header...
	//120,1,"ETS300",2,9,"0641001pxaa","W022","Allegro MicroSystems Inc.","Eagle23","4.04f","2.03",
	//125,W,"","","Tokyo Electron Limited P8 Prober","",
	//140,1,"c:\ETS\WORK\PR3980PEA.DLL","PR3980PEA - Eagle Test Systems Application","1.0","05/14/2001","11/21/2006",
	
	do
		strString = hEagleDatalogFile.readLine();
	while(!strString.isNull() && strString.isEmpty());

	// Close file
	f.close();

    QStringList lstSections = strString.split(",",QString::KeepEmptyParts);
	// Check if have the good count
	if(lstSections.count() <= 2)
	{
		return false;
	}

	if(( lstSections[0] != "120")
	|| !(lstSections[2].remove('"').simplified().startsWith("ETS")))
	{
		// Incorrect header...this is not a EAGLE_DATALOG file!
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the EAGLE_DATALOG file
//////////////////////////////////////////////////////////////////////
bool CGEAGLE_DATALOGtoSTDF::ReadEagleDatalogFile(const char *EagleDatalogFileName,const char *strFileNameSTDF)
{
	bool		bIsNum;
	QStringList	lstSections;
    QString strString, strRecord;

	// Init some variables
	m_uiMaxTestNumLeft = m_uiMaxTestNumRight = 0;

	// Open EAGLE_DATALOG file
    QFile f( EagleDatalogFileName );
    if(!f.open( QIODevice::ReadOnly ))
	{
		// Failed Opening EAGLE_DATALOG file
		m_iLastError = errOpenFail;
		
		// Convertion failed.
		return false;
	}

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	iProgressStep = 0;
	iNextFilePos = 0;
	iFileSize = f.size() + 1;
    QCoreApplication::processEvents();
	
	
	// Assign file I/O stream
	QTextStream hEagleDatalogFile(&f);

	// Check if first line is the correct EAGLE_DATALOG header...
	//120,1,"ETS300",2,9,"0641001pxaa","W022","Allegro MicroSystems Inc.","Eagle23","4.04f","2.03",
	//125,W,"","","Tokyo Electron Limited P8 Prober","",
	//140,1,"c:\ETS\WORK\PR3980PEA.DLL","PR3980PEA - Eagle Test Systems Application","1.0","05/14/2001","11/21/2006",
	
    m_strLine = ReadLine(hEagleDatalogFile);

    lstSections = m_strLine.split(",", QString::KeepEmptyParts);
	// Check if have the good count
	if(lstSections.count() <= 2)
	{
		m_iLastError = errInvalidFormatLowInRows;
		
		// Convertion failed.
		// Close file
		f.close();
		return false;
	}

	if(( lstSections[0] != "120")
	|| !(lstSections[2].remove('"').simplified().startsWith("ETS")))
	{
		// Incorrect header...this is not a EAGLE_DATALOG file!
		m_iLastError = errInvalidFormat;
		
		// Convertion failed.
		// Close file
		f.close();
		return false;
	}

	m_bIsAWafer = false;
	// Read EAGLE_DATALOG information
	while(!hEagleDatalogFile.atEnd())
	{
		if(m_strLine.isEmpty())
		{
			hEagleDatalogFile.skipWhiteSpace();
			m_strLine = ReadLine(hEagleDatalogFile);
            // Split comma but ignore comma between "
            //lstSections = m_strLine.split(",", QString::KeepEmptyParts);
            // 1,8401.3888,"All Chans, All Alarms:  num",48,0,
            // 1
            // 8401.3888
            // "All Chans, All Alarms:  num"
            // 48
            // 0
            //
            strString = m_strLine;
            lstSections.clear();
            while(!strString.isEmpty())
            {
                // If starts with "
                // and have comma between "
                if((strString.left(1) == "\"")
                        && (strString.section("\"",0,1).count(",")>0))
                {
                    strRecord = strString.section("\"",1,1);
                    strString = strString.section(",",strRecord.count(",")+1);
                }
                else
                {
                    strRecord = strString.section(",",0,0);
                    strString = strString.section(",",1);
                }
                lstSections.append(strRecord);
            }
		}


		if(lstSections[0] == "120")
		{
			//120,1,"ETS300",2,9,"0641001pxaa","W022","Allegro MicroSystems Inc.","Eagle23","4.04f","2.03",
			if(lstSections.count() > 2)
				m_strTesterId = lstSections[2].remove('"').simplified();
			if(lstSections.count() > 5)
				m_strLotId = lstSections[5].remove('"').simplified();
			if(lstSections.count() > 6)
				m_strSubLotId = lstSections[6].remove('"').simplified();
			if(lstSections.count() > 7)
				m_strOperName = lstSections[7].remove('"').simplified();
			if(lstSections.count() > 8)
				m_strNodeName = lstSections[8].remove('"').simplified();
			if(lstSections.count() > 9)
				m_strExecVer = lstSections[9].remove('"').simplified();
		}
		else if(lstSections[0] == "125")
		{
			//125,W,"","","Tokyo Electron Limited P8 Prober","",
			// Check if have the good count
			if(lstSections.count() <= 4)
			{
				m_iLastError = errInvalidFormatLowInRows;
				
				// Convertion failed.
				// Close file
				f.close();
				return false;
			}

			m_strHandId = lstSections[4].remove('"').simplified();
			if(lstSections[1].startsWith("W", Qt::CaseInsensitive))
				m_bIsAWafer = true;
			else
				m_bIsAWafer = false;
		}
		else if(lstSections[0] == "140")
		{
			// Check if have the good count
			if(lstSections.count() <= 5)
			{
				m_iLastError = errInvalidFormatLowInRows;
				
				// Convertion failed.
				// Close file
				f.close();
				return false;
			}

			if(lstSections[1] == "1")
			{
				//140,1,"c:\ETS\WORK\PR3980PEA.DLL","PR3980PEA - Eagle Test Systems Application","1.0","05/14/2001","11/21/2006",
				m_strPartType = lstSections[3].remove('"').simplified();
			}
			else
			if(lstSections[1] == "2")
			{
				//140,2,"c:\ETS\WORK\PR3980PEA.pds","PR3980PEA","1.00","05/14/2001","11/21/2006",

				m_strAuxFile = lstSections[2].remove('"').simplified();
				m_strSpecName = m_strJobName = lstSections[3].remove('"').simplified();
				m_strSpecRev = m_strJobRev = lstSections[4].remove('"').simplified();
			}
			else
			if(lstSections[1] == "4")
			{
				//140,4,"PR3980PEA","AllegroÄMotorDrivers","1.0","05/14/2001","11/21/2006",

				m_strSpecName = m_strJobName = lstSections[2].remove('"').simplified();
				m_strSpecRev = m_strJobRev = lstSections[4].remove('"').simplified();
				m_strDateCode = lstSections[5].remove('"').simplified();
			}
		}
		else if(lstSections[0] == "10")
		{
			//10,1.0,0,ì,-10,"S/N","Hardware_Eagle_ID",
			// test parameter description
			// Check if have the good count
			if(lstSections.count() <= 6)
			{
				m_iLastError = errInvalidFormatLowInRows;
				
				// Convertion failed.
				// Close file
				f.close();
				return false;
			}

			unsigned int	uiTestNumLeft, uiTestNumRight;
            QString			strTestNum;

			strTestNum = lstSections[1];
			uiTestNumLeft = strTestNum.section(".",0,0).simplified().toUInt();
			uiTestNumRight = strTestNum.section(".",1,1).simplified().toUInt();
			if(uiTestNumLeft > m_uiMaxTestNumLeft)
				m_uiMaxTestNumLeft = uiTestNumLeft;
			if(uiTestNumRight > m_uiMaxTestNumRight)
				m_uiMaxTestNumRight = uiTestNumRight;

			CGEagleDatalogParameter *pParameter;
			if(m_qMapParameterList.contains(strTestNum))
				pParameter = m_qMapParameterList[strTestNum];
			else
			{
				pParameter = new CGEagleDatalogParameter();
				pParameter->m_uiTestNumLeft = uiTestNumLeft;
				pParameter->m_uiTestNumRight = uiTestNumRight;
				m_qMapParameterList[strTestNum] = pParameter;
			}
			pParameter->m_strName = lstSections[6].remove('"').simplified();
			pParameter->m_strUnits = lstSections[5].remove('"').simplified();
			pParameter->m_fLowLimit = lstSections[4].simplified().toFloat(&bIsNum);
			if(bIsNum)
				pParameter->m_bValidLowLimit = true;
			pParameter->m_fHighLimit = lstSections[3].simplified().toFloat(&bIsNum);
			if(bIsNum)
				pParameter->m_bValidHighLimit = true;
			pParameter->m_nFormatRes = lstSections[3].simplified().toInt(&bIsNum);
			NormalizeLimits(pParameter->m_strUnits, pParameter->m_nScale);
		}
		else if(lstSections[0] == "100")
			break;
		else if(lstSections[0] == "130")
			break;
		else
		{
			// Ignore this record
		}
		m_strLine = "";
	}

	QString strNextLine;

	while(!hEagleDatalogFile.atEnd())
	{
		strNextLine = ReadLine(hEagleDatalogFile);
		// Part line (130) after Part results (100)
		if((strNextLine.left(4) == "130,")
		&& ((m_strLine.left(4) == "100,")))
			m_lstPartResult.append(strNextLine);
		if(strNextLine.left(4) == "999,")
			break;
		m_strLine = strNextLine;
	}

	//Restart at the beggining of the file
	hEagleDatalogFile.seek(0);
	m_strLine.clear();

	QString strLine;
	if(!m_lstPartResult.isEmpty())
		strLine = m_lstPartResult.first();

	if(strLine.left(4) == "130,")
	{
		// PRR-like line

		// Check if Valid X,Y wafer coordinates specified...because sometimes the header doesn't specify 'W' for wafersort but 'P'!
        if(m_bIsAWafer == false)
        {
            bool	bDieX,bDieY;
            QStringList lstSections = strLine.split(",", QString::KeepEmptyParts);
            // Check if have the good count
            if(lstSections.count() <= 6)
            {
                m_iLastError = errInvalidFormatLowInRows;

                // Convertion failed.
                // Close file
                f.close();
                return false;
            }

            lstSections[5].toInt(&bDieX);
            lstSections[6].toInt(&bDieY);
            if(bDieX && bDieY)
                m_bIsAWafer = true;
        }

        // Extract date info
        strLine = strLine.section(",",2,2).remove('"').simplified();
    }
	else
		strLine = "";
	if(!strLine.isEmpty())
	{
		QDate clDate(strLine.section(' ',0,0).section("/",2,2).simplified().toInt(),
					 strLine.section(' ',0,0).section("/",0,0).simplified().toInt(),
					 strLine.section(' ',0,0).section("/",1,1).simplified().toInt());
		QTime clTime = QTime::fromString(strLine.section(' ',1).simplified());
		QDateTime clDateTime(clDate,clTime);
		clDateTime.setTimeSpec(Qt::UTC);
		m_lStartTime = clDateTime.toTime_t();

	}

	iNextFilePos = 0;

	if(!WriteStdfFile(&hEagleDatalogFile,strFileNameSTDF))
	{
		// Close file
		f.close();
		QFile::remove(strFileNameSTDF);
		return false;
	}
	
	f.close();

	// All EAGLE_DATALOG file read...check if need to update the EAGLE_DATALOG Parameter list on disk?
	if(m_bNewEagleDatalogParameterFound == true)
		DumpParameterIndexTable();

	// Success parsing EAGLE_DATALOG file
	return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from EAGLE_DATALOG data parsed
//////////////////////////////////////////////////////////////////////
bool CGEAGLE_DATALOGtoSTDF::WriteStdfFile(QTextStream *hEagleDatalogFile,const char *strFileNameSTDF)
{
	// Choose rule for test number generation (the goal is to make sure we generate unique test numbers): 
    // Quantix TestNumber = Eagle TestNumber_Left * Multiplier + Eagle TestNumber_Right
	// 1. Define the right multiplier (for compatibility, try to use 100000)
	unsigned int uiMultiplier=100000;
	if((0xffffffff-m_uiMaxTestNumRight)/uiMultiplier < m_uiMaxTestNumLeft)
	{
		uiMultiplier = 1;
		while(((double)m_uiMaxTestNumRight/(double)uiMultiplier) >= 1)
			uiMultiplier *= 10;
	}
	// 2. Check if multiplier will not generate too big numbers
	if((0xffffffff-m_uiMaxTestNumRight)/uiMultiplier < m_uiMaxTestNumLeft)
	{
		m_iLastError = errTestNumberOverflow;
		return false;
	}

	// now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open((char*)strFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
	{
		// Failed importing EAGLE_DATALOG file into STDF database
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
	StdfFile.WriteString(m_strPartType.toLatin1().constData());	// Part Type / Product ID
	StdfFile.WriteString(m_strNodeName.toLatin1().constData());	// Node name
	StdfFile.WriteString(m_strTesterId.toLatin1().constData());	// Tester Type
	StdfFile.WriteString(m_strJobName.toLatin1().constData());		// Job name
	StdfFile.WriteString(m_strJobRev.toLatin1().constData());		// Job rev
	StdfFile.WriteString(m_strSubLotId.toLatin1().constData());	// sublot-id
	StdfFile.WriteString(m_strOperName.toLatin1().constData());	// operator
	StdfFile.WriteString("ETS Test Executive");				// exec-type
	StdfFile.WriteString(m_strExecVer.toLatin1().constData());		// exe-ver
	if(m_bIsAWafer)											// test-cod
		StdfFile.WriteString("WAFER");
	else
		StdfFile.WriteString("");
	StdfFile.WriteString("");								// test-temperature
	// Construct custom Galaxy USER_TXT 
	QString	strUserTxt;
	strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;	
	strUserTxt += ":";
	strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
	strUserTxt += ":EAGLE_DATALOG";
	StdfFile.WriteString(strUserTxt.toLatin1().constData());		// user-txt
	StdfFile.WriteString(m_strAuxFile.toLatin1().constData());		// aux-file
	StdfFile.WriteString("WAFER");							// package-type
	StdfFile.WriteString(m_strOperName.section(' ',0,0).toLatin1().constData());	// familyID
	StdfFile.WriteString(m_strDateCode.toLatin1().constData());	// Date-code
	StdfFile.WriteString("");								// Facility-ID
	StdfFile.WriteString("");								// FloorID
	StdfFile.WriteString(m_strProcessId.toLatin1().constData());	// ProcessID
	StdfFile.WriteString("");								// oper_nam
	StdfFile.WriteString(m_strSpecName.toLatin1().constData());	// spec_name
	StdfFile.WriteString(m_strSpecRev.toLatin1().constData());		// spec_ver

	StdfFile.WriteRecord();

	if(m_bIsAWafer)
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
	QString		strRecord;
	QString		strString;
	QString		strPartNum;
	int			iTotalGoodBin,iTotalFailBin;
	int			iTotalTests, iPartNumber;
	int			iSiteNumber=0;
	bool		bIsNum;
	bool		bPassStatus=false;
	BYTE		bData;
	QStringList lstSections;

	QString		strBinName;
    int			lSBin=0, lHBin=0, lBin;
	int			iXWafer=-32768;
	int			iYWafer=-32768;
	bool		bTestPass;
    float		fValue;

	unsigned int	uiTestNumLeft, uiTestNumRight, uiGalaxyTestNum;
    QString			strTestNum;

	// Reset counters
  //FIXME: not used ?
  //bStdfHeaderWritten = false;
	iTotalGoodBin=iTotalFailBin=0;
	iTotalTests=iPartNumber=0;

	// Write all Parameters read on this file : PIR,PTR...,PRR, PIR,PTR..., PRR

	//100,1.3,"    ","P",1,
	//100,1.4,"    ","P",-0.625,
	//100,16.30,"    ","P",1,
	//100,16.31,"    ","P",1,
	//130,2,"11/29/2006  08:51:59","1","P",23,1,1,1,
	
	// Read EAGLE_DATALOG information
	while(!hEagleDatalogFile->atEnd())
	{
		if(m_strLine.isEmpty())
		{
			hEagleDatalogFile->skipWhiteSpace();
			m_strLine = ReadLine(*hEagleDatalogFile);

            // Split comma but ignore comma between "
            //lstSections = m_strLine.split(",", QString::KeepEmptyParts);
            // 1,8401.3888,"All Chans, All Alarms:  num",48,0,
            // 1
            // 8401.3888
            // "All Chans, All Alarms:  num"
            // 48
            // 0
            //
            strString = m_strLine;
            lstSections.clear();
            while(!strString.isEmpty())
            {
                // If starts with "
                // and have comma between "
                if((strString.left(1) == "\"")
                        && (strString.section("\"",0,1).count(",")>0))
                {
                    strRecord = strString.section("\"",1,1);
                    strString = strString.section(",",strRecord.count(",")+1);
                }
                else
                {
                    strRecord = strString.section(",",0,0);
                    strString = strString.section(",",1);
                }
                lstSections.append(strRecord);
            }
        }

		if(lstSections[0] == "100")
		{// Parameter result
			//100,4.27,"    ","P",1,

			// Check if have the good count
			if(lstSections.count() <= 4)
			{
				m_iLastError = errInvalidFormatLowInRows;
				
				StdfFile.Close();
				// Convertion failed.
				return false;
			}

			strTestNum = lstSections[1];
			uiTestNumLeft = strTestNum.section(".",0,0).simplified().toUInt();
			uiTestNumRight = strTestNum.section(".",1,1).simplified().toUInt();

			// Check if multiplier will not generate too big numbers
			if((0xffffffff-uiTestNumRight)/uiMultiplier < uiTestNumLeft)
			{
				StdfFile.Close();
				m_iLastError = errTestNumberOverflow;
				return false;
			}

			CGEagleDatalogParameter *pParameter;
			if(m_qMapParameterList.contains(strTestNum))
				pParameter = m_qMapParameterList[strTestNum];
			else
			{
				pParameter = new CGEagleDatalogParameter();
				pParameter->m_uiTestNumLeft = uiTestNumLeft;
				pParameter->m_uiTestNumRight = uiTestNumRight;
				m_qMapParameterList[strTestNum] = pParameter;
			}

			// if the first result
			if(iTotalTests == 0)
			{
				// Read record = 130

				QString strLine;
				// first and delete
                if(m_lstPartResult.isEmpty())
                {
                    StdfFile.Close();
                    m_iLastError = errInvalidFormat;
                    return false;
                }

                strLine = m_lstPartResult.takeFirst();

				//130,1,"11/29/2006  09:53:13","1275","P",13,30,1,1,
				iSiteNumber = strLine.section(",",1,1).toInt();
				
				// Write PIR
				// Write PIR for parts in this Wafer site
				RecordReadInfo.iRecordType = 5;
				RecordReadInfo.iRecordSubType = 10;
				StdfFile.WriteHeader(&RecordReadInfo);
				StdfFile.WriteByte(1);					// Test head
				StdfFile.WriteByte(iSiteNumber);					// Tester site
				StdfFile.WriteRecord();

				// Reset counters
				iTotalTests = 0;

			}

			bTestPass = lstSections[3].remove('"').simplified() == "P";
            fValue = lstSections[4].simplified().toFloat(&bIsNum);

			// Write PTR
			RecordReadInfo.iRecordType = 15;
			RecordReadInfo.iRecordSubType = 10;
			
			StdfFile.WriteHeader(&RecordReadInfo);
			uiGalaxyTestNum = pParameter->m_uiTestNumLeft*uiMultiplier+pParameter->m_uiTestNumRight;
			StdfFile.WriteDword(uiGalaxyTestNum);				// Test Number
			StdfFile.WriteByte(1);								// Test head
			StdfFile.WriteByte(iSiteNumber);					// Tester site:1,2,3,4 or 5, etc.
			if(bTestPass)
				bData = 0;		// Test passed
			else
				bData = BIT7;	// Test Failed
			StdfFile.WriteByte(bData);							// TEST_FLG
			bData = BIT6|BIT7;
            StdfFile.WriteByte(bData);							// PARAM_FLG
            StdfFile.WriteFloat(fValue * GS_POW(10.0,pParameter->m_nScale));		// Test result
			if(!pParameter->m_bStaticHeaderWritten)
			{
				pParameter->m_bStaticHeaderWritten = true;
				// save Parameter name without unit information
				StdfFile.WriteString(pParameter->m_strName.toLatin1().constData());	// TEST_TXT
				StdfFile.WriteString("");							// ALARM_ID

				bData = 2;	// Valid data.
				if(!pParameter->m_bValidLowLimit)
					bData |= BIT6;
				if(!pParameter->m_bValidHighLimit)
					bData |= BIT7;
				StdfFile.WriteByte(bData);							// OPT_FLAG

				StdfFile.WriteByte(-pParameter->m_nScale);							// RES_SCALE
				StdfFile.WriteByte(-pParameter->m_nScale);							// LLM_SCALE
				StdfFile.WriteByte(-pParameter->m_nScale);							// HLM_SCALE
				StdfFile.WriteFloat(pParameter->m_fLowLimit * GS_POW(10.0,pParameter->m_nScale));	// LOW Limit
				StdfFile.WriteFloat(pParameter->m_fHighLimit * GS_POW(10.0,pParameter->m_nScale));	// HIGH Limit
				StdfFile.WriteString(pParameter->m_strUnits.toLatin1().constData());		// Units
				
				strRecord = "%9." + QString::number(pParameter->m_nFormatRes);
				StdfFile.WriteString(strRecord.toLatin1().constData());					// c_resfmt
				StdfFile.WriteString(strRecord.toLatin1().constData());					// c_llmfmt
				StdfFile.WriteString(strRecord.toLatin1().constData());					// c_hlmfmt
			}
			StdfFile.WriteRecord();

			iTotalTests++;
		}
		else if(lstSections[0] == "130")
		{//End of part

			//130,1,"11/29/2006  09:53:13","1275","P",13,30,1,1,
			// Check if have the good count
            if(lstSections.count() <= 8)
			{
				m_iLastError = errInvalidFormatLowInRows;
				
				StdfFile.Close();
				// Convertion failed.
				return false;
			}
			
			QString strDateTime = lstSections[2].remove('"').simplified();
			QDate clDate(strDateTime.section(' ',0,0).section("/",2,2).toInt(),
						 strDateTime.section(' ',0,0).section("/",0,0).toInt(),
						 strDateTime.section(' ',0,0).section("/",1,1).toInt());
			QTime clTime = QTime::fromString(strDateTime.section(' ',1).simplified());
			QDateTime clDateTime(clDate,clTime);
			clDateTime.setTimeSpec(Qt::UTC);
			m_lStartTime = clDateTime.toTime_t();

			iSiteNumber = lstSections[1].toInt();
			strPartNum = lstSections[3].remove('"').simplified();
			bPassStatus = lstSections[4].remove('"').simplified() == "P";
			if(m_bIsAWafer)
			{
				iXWafer = lstSections[5].toInt();
				iYWafer = lstSections[6].toInt();
			}
            lSBin = lstSections[7].toInt();
            lHBin = lstSections[8].toInt();

			// if the no result
			if(iTotalTests == 0)
			{
				// Write PIR
				// Write PIR for parts in this Wafer site
				RecordReadInfo.iRecordType = 5;
				RecordReadInfo.iRecordSubType = 10;
				StdfFile.WriteHeader(&RecordReadInfo);
				StdfFile.WriteByte(1);					// Test head
				StdfFile.WriteByte(iSiteNumber);					// Tester site
				StdfFile.WriteRecord();
			}
			
			// Write PRR
			RecordReadInfo.iRecordType = 5;
			RecordReadInfo.iRecordSubType = 20;
			StdfFile.WriteHeader(&RecordReadInfo);
			StdfFile.WriteByte(1);						// Test head
			StdfFile.WriteByte(iSiteNumber);		// Tester site:1,2,3,4 or 5
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
			StdfFile.WriteWord((WORD)iTotalTests);	// NUM_TEST
            StdfFile.WriteWord((WORD)lHBin);           // HARD_BIN
            StdfFile.WriteWord((WORD)lSBin);           // SOFT_BIN
			StdfFile.WriteWord(iXWafer);			// X_COORD
			StdfFile.WriteWord(iYWafer);			// Y_COORD
			StdfFile.WriteDword(0);					// No testing time known...
			StdfFile.WriteString(strPartNum.toLatin1().constData());// PART_ID
			StdfFile.WriteString("");				// PART_TXT
			StdfFile.WriteString("");				// PART_FIX
			StdfFile.WriteRecord();

			// Reset counters
			iTotalTests = 0;

			iPartNumber++;
		}
		else
		if(lstSections[0] == "50")
		{
			//50,32,"A",0,"Alarm Bin",

			// Check if have the good count
			if(lstSections.count() <= 4)
			{
				m_iLastError = errInvalidFormatLowInRows;
				
				StdfFile.Close();
				// Convertion failed.
				return false;
			}
			
            lBin = lstSections[1].simplified().toInt();
			bPassStatus = lstSections[2].remove('"').simplified() == "P";
			iTotalTests = lstSections[3].simplified().toInt();
			strBinName = lstSections[4].remove('"').simplified();
			
			// Write HBR
			RecordReadInfo.iRecordType = 1;
			RecordReadInfo.iRecordSubType = 40;
			StdfFile.WriteHeader(&RecordReadInfo);
			StdfFile.WriteByte(255);						// Test Head = ALL
			StdfFile.WriteByte(255);						// Test sites = ALL		
            StdfFile.WriteWord(lBin);	// HBIN = 0
			StdfFile.WriteDword(iTotalTests);	// Total Bins
			if(bPassStatus)
				StdfFile.WriteByte('P');
			else
				StdfFile.WriteByte('F');
			StdfFile.WriteString(strBinName.toLatin1().constData());
			StdfFile.WriteRecord();

			// Write SBR
			RecordReadInfo.iRecordType = 1;
			RecordReadInfo.iRecordSubType = 50;
			StdfFile.WriteHeader(&RecordReadInfo);
			StdfFile.WriteByte(255);						// Test Head = ALL
			StdfFile.WriteByte(255);						// Test sites = ALL		
            StdfFile.WriteWord(lBin);	// HBIN = 0
			StdfFile.WriteDword(iTotalTests);	// Total Bins
			if(bPassStatus)
				StdfFile.WriteByte('P');
			else
				StdfFile.WriteByte('F');
			StdfFile.WriteString(strBinName.toLatin1().constData());
			StdfFile.WriteRecord();
		}
		else
		if (lstSections[0] == "30")
		{
			//30,"11/29/2006  08:51:59","11/29/2006  09:53:13",

			// Check if have the good count
			if(lstSections.count() <= 2)
			{
				m_iLastError = errInvalidFormatLowInRows;
				
				StdfFile.Close();
				// Convertion failed.
				return false;
			}
			
			QString strDateTime = lstSections[2].remove('"').simplified();
			QDate clDate(strDateTime.section(' ',0,0).section("/",2,2).simplified().toInt(),
						 strDateTime.section(' ',0,0).section("/",0,0).simplified().toInt(),
						 strDateTime.section(' ',0,0).section("/",1,1).simplified().toInt());
			QTime clTime = QTime::fromString(strDateTime.section(' ',1).simplified());
			QDateTime clDateTime(clDate,clTime);
			clDateTime.setTimeSpec(Qt::UTC);
			m_lStartTime = clDateTime.toTime_t();
		}
		else
		if (lstSections[0] == "1")
		{
			//1,1.0,"Hardware_Eagle_ID",1275,0,
			// Check if have the good count
			if(lstSections.count() <= 4)
			{
				m_iLastError = errInvalidFormatLowInRows;
				
				StdfFile.Close();
				// Convertion failed.
				return false;
			}
			
			int		nNum;
			QString strName;
			

			// create TSR for test information
			// Write TSR for last wafer inserted
			RecordReadInfo.iRecordType = 10;
			RecordReadInfo.iRecordSubType = 30;
			StdfFile.WriteHeader(&RecordReadInfo);
			StdfFile.WriteByte(255);						// Test head
			StdfFile.WriteByte(255);						// Tester site (all)
			StdfFile.WriteByte('P');

			strTestNum = lstSections[1];
			uiTestNumLeft = strTestNum.section(".",0,0).simplified().toUInt();
			uiTestNumRight = strTestNum.section(".",1,1).simplified().toUInt();
			uiGalaxyTestNum = uiTestNumLeft*uiMultiplier+uiTestNumRight;
			StdfFile.WriteDword(uiGalaxyTestNum);						// Test Number

            nNum = lstSections[3].toInt() + lstSections[4].toInt();
			StdfFile.WriteDword(nNum);						// Number of test executions
			
			nNum = lstSections[4].toInt();
			StdfFile.WriteDword(nNum);						// Number of test failures
			StdfFile.WriteDword(4294967295UL);				// Number of alarmed tests
			
			strName = lstSections[2].remove('"').simplified();
			StdfFile.WriteString(strName.toLatin1().constData());// TEST_NAM

			StdfFile.WriteRecord();
		}
		else
		if (lstSections[0] == "201")
		{
			//201,1,1.0,646,0,
			// Check if have the good count
			if(lstSections.count() <= 4)
			{
				m_iLastError = errInvalidFormatLowInRows;
				
				StdfFile.Close();
				// Convertion failed.
				return false;
			}
			
            int		nNum;

			// create TSR for test information
			// Write TSR for last wafer inserted
			RecordReadInfo.iRecordType = 10;
			RecordReadInfo.iRecordSubType = 30;
			StdfFile.WriteHeader(&RecordReadInfo);
			StdfFile.WriteByte(1);						// Test head
			nNum = lstSections[1].toInt();
			StdfFile.WriteByte(nNum);						// Tester site (all)
			StdfFile.WriteByte('P');

			strTestNum = lstSections[2];
			uiTestNumLeft = strTestNum.section(".",0,0).simplified().toUInt();
			uiTestNumRight = strTestNum.section(".",1,1).simplified().toUInt();
			uiGalaxyTestNum = uiTestNumLeft*uiMultiplier+uiTestNumRight;
			StdfFile.WriteDword(uiGalaxyTestNum);			// Test Number

            nNum = lstSections[3].toInt() + lstSections[4].toInt();
			StdfFile.WriteDword(nNum);						// Number of test executions
			
			nNum = lstSections[4].toInt();
			StdfFile.WriteDword(nNum);						// Number of test failures
			StdfFile.WriteDword(4294967295UL);				// Number of alarmed tests

			StdfFile.WriteRecord();
		}
		else
		{
			// Ignore
		}
		m_strLine = "";
	}


	if(m_bIsAWafer)
	{
		// Write WRR for last wafer inserted
		RecordReadInfo.iRecordType = 2;
		RecordReadInfo.iRecordSubType = 20;
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(1);						// Test head
		StdfFile.WriteByte(255);					// Tester site (all)
		StdfFile.WriteDword(m_lStartTime);						// Time of last part tested
		StdfFile.WriteDword(iPartNumber);			// Parts tested: always 5
		StdfFile.WriteDword(0);						// Parts retested
		StdfFile.WriteDword(0);						// Parts Aborted
		StdfFile.WriteDword(iTotalGoodBin);			// Good Parts
		StdfFile.WriteDword(4294967295UL);			// Functionnal Parts
		StdfFile.WriteString(m_strWaferId.toLatin1().constData());	// WaferID
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
// Convert 'FileName' EAGLE_DATALOG file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGEAGLE_DATALOGtoSTDF::Convert(const char *EagleDatalogFileName, const char *strFileNameSTDF)
{
	// No erro (default)
	m_iLastError = errNoError;

	// If STDF file already exists...do not rebuild it...unless dates not matching!
	QFileInfo fInput(EagleDatalogFileName);
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
		GexProgressBar->setMaximum(200);
		GexProgressBar->setTextVisible(true);
		GexProgressBar->setValue(0);
		GexProgressBar->show();
	}

	if(GexScriptStatusLabel != NULL)
	{
		if(GexScriptStatusLabel->isHidden())
		{
			bHideLabelAfter = true;
			GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(EagleDatalogFileName).fileName()+"...");
			GexScriptStatusLabel->show();
		}
	}
    QCoreApplication::processEvents();

    if(ReadEagleDatalogFile(EagleDatalogFileName,strFileNameSTDF) != true)
	{
		//////////////////////////////////////////////////////////////////////
		// For ProgressBar
		if((GexProgressBar != NULL)
		&& bHideProgressAfter)
			GexProgressBar->hide();
		
		if((GexScriptStatusLabel != NULL)
		&& bHideLabelAfter)
			GexScriptStatusLabel->hide();

		return false;	// Error reading EAGLE_DATALOG file
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
QString CGEAGLE_DATALOGtoSTDF::ReadLine(QTextStream& hFile)
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
	do
    {
		strString = hFile.readLine();
        // Ignore invalid start line
        // GCORE-16976
        // This is the list of sections which starts with specific number
        // not ignored by the algo
        // in lstSections[0] or (m_)strLine.left(4) or strNextLine.left(4)
        // If a new section has to be analyse => add the new number in this list
        if((strString.left(2) == "1,")
                || (strString.left(3) == "10,")
                || (strString.left(3) == "30,")
                || (strString.left(3) == "50,")
                || (strString.left(4) == "100,")
                || (strString.left(4) == "120,")
                || (strString.left(4) == "125,")
                || (strString.left(4) == "130,")
                || (strString.left(4) == "140,")
                || (strString.left(4) == "201,")
                || (strString.left(4) == "999,"))
        {
            break;
        }
        else
        {
            strString = "";
        }
    }
    while(!strString.isNull() && strString.isEmpty()
          && !hFile.atEnd());

	return strString;

}
