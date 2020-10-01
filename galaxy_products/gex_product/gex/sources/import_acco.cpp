//////////////////////////////////////////////////////////////////////
// import_acco.cpp: Convert a Acco .csv
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
#include "import_acco.h"
#include "time.h"
#include "import_constants.h"

//
//STS8107 StationA
//Date: 2010-02-24                        ShiftNo: A3
//LotID: TH60284_5_FP1002-0915            TesterNo: 3151BDS
//MarkingID:                              TestHead:
//D/C:                                    TestType
//Program: C:\AccoTest\TESTFILE\CP\3151BDSx2\2K060DFH.prg
//Program REV:
//MatchCode:
//Operator: op                            Temperature:
//Average Test Time(ms): 304              Idle time: 0 day 0:33:54
//Beginning Time: 2010-02-24 ¤U¤È 07:50:26Ending Time: 2010-02-24 ¤U¤È 08:45:42
//Total Testing time: 0 day 0:21:20
//Description:
//WaferLotID:
//WaferID:
//
//Total: 8304
//Pass: 8147    98.11%
//Fail: 157    1.89%
//Bin[1] Pass default: 8147    98.11%
//Bin[2] Fail default: 0    0.00%
//...
//
//SITE_NO,DUT_NO,DUT_PASS,SW_BIN,T_TIME,DATA_NUM,Isink1 contact,...
//Unit,,,,ms,,V,V,V,V,V,V,V,V,V,V,V,uA,uA,uA,uA,uA,uA,mA,mA,mA,...
//LimitL,,,,,,-0.9000,-0.9000,-0.9000,-0.9000,-0.9000,-0.9000,...
//LimitU,,,,,,-0.3000,-0.3000,-0.3000,-0.3000,-0.3000,-0.3000,...
//
//1,16,True,1,318,43,-0.6203,-0.6212,-0.6206,-0.6212,-0.6205,...
//2,16,False,4,296,16,-0.6214,-0.6220,-0.6214,-0.6214,-0.6222,...

// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGAccotoSTDF::CGAccotoSTDF()
{
	// Default: Acco parameter list on disk includes all known Acco parameters...
	m_bNewAccoParameterFound = false;
	m_lStartTime = m_lStopTime = 0;
	m_strTesterType = "S8107";
	m_nTotalParts = 0;
	m_nPassParts = 0;
	m_nFailParts = 0;

	m_pAccoParameter = NULL;
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CGAccotoSTDF::~CGAccotoSTDF()
{
	// Destroy list of Parameters tables.
	if(m_pAccoParameter!=NULL)
		delete [] m_pAccoParameter;
	// Destroy list of Bin tables.
	QMap<int,CGAccoBinning *>::Iterator it;
	for(it = m_mapAccoSoftBinning.begin(); it!= m_mapAccoSoftBinning.end(); it++)
		delete m_mapAccoSoftBinning[it.key()];
	m_mapAccoSoftBinning.clear();
	for(it = m_mapAccoHardBinning.begin(); it!= m_mapAccoHardBinning.end(); it++)
		delete m_mapAccoHardBinning[it.key()];
	m_mapAccoHardBinning.clear();
}


//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGAccotoSTDF::GetLastError()
{
	strLastError = "Import Acco: ";

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
// Load Acco Parameter table from DISK
//////////////////////////////////////////////////////////////////////
void CGAccotoSTDF::LoadParameterIndexTable(void)
{
	QString	strAccoTableFile;
	QString	strString;

    strAccoTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strAccoTableFile += GEX_ACCO_PARAMETERS;

	// Open Acco Parameter table file
	QFile f( strAccoTableFile );
    if(!f.open( QIODevice::ReadOnly ))
		return;

	// Assign file I/O stream
	QTextStream hAccoTableFile(&f);

	// Skip comment lines
	do
	{
	  strString = hAccoTableFile.readLine();
	}
    while((strString.indexOf("----------------------") < 0) && (hAccoTableFile.atEnd() == false));

	// Read lines
	m_pFullAccoParametersList.clear();
	strString = hAccoTableFile.readLine();
	while (strString.isNull() == false)
	{
		// Save Parameter name in list
		m_pFullAccoParametersList.append(strString);
		// Read next line
		strString = hAccoTableFile.readLine();
	};

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// Save Acco Parameter table to DISK
//////////////////////////////////////////////////////////////////////
void CGAccotoSTDF::DumpParameterIndexTable(void)
{
	QString		strAccoTableFile;
	QString		strString;
	int	uIndex;

    strAccoTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strAccoTableFile += GEX_ACCO_PARAMETERS;

	// Open Acco Parameter table file
	QFile f( strAccoTableFile );
    if(!f.open( QIODevice::WriteOnly ))
		return;

	// Assign file I/O stream
	QTextStream hAccoTableFile(&f);

	// First few lines are comments:
	hAccoTableFile << "############################################################" << endl;
	hAccoTableFile << "# DO NOT EDIT THIS FILE!" << endl;
    hAccoTableFile << "# Quantix Examinator: Acco Parameters detected" << endl;
	hAccoTableFile << "# www.mentor.com" << endl;
    hAccoTableFile << "# Quantix Examinator reads and writes into this file..." << endl;
	hAccoTableFile << "-----------------------------------------------------------" << endl;

	// Write lines
	// m_pFullAccoParametersList.sort();
	for(uIndex=0;uIndex<m_pFullAccoParametersList.count();uIndex++)
	{
		// Write line
		hAccoTableFile << m_pFullAccoParametersList[uIndex] << endl;
	};

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// If Examinator doesn't have this Acco parameter in his dictionnary, have it added.
//////////////////////////////////////////////////////////////////////
void CGAccotoSTDF::UpdateParameterIndexTable(QString strParamName)
{
	// Check if the table is empty...if so, load it from disk first!
	if(m_pFullAccoParametersList.isEmpty() == true)
	{
		// Load Acco parameter table from disk...
		LoadParameterIndexTable();
	}

	// Check if Parameter name already in table...if not, add it to the list
	// the new full list will be dumped to the disk at the end.
    if(m_pFullAccoParametersList.indexOf(strParamName) < 0)
	{
		// Update list
		m_pFullAccoParametersList.append(strParamName);

		// Set flag to force the current Acco table to be updated on disk
		m_bNewAccoParameterFound = true;
	}
}


//////////////////////////////////////////////////////////////////////
// Check if File is compatible with Acco format
//////////////////////////////////////////////////////////////////////
bool CGAccotoSTDF::IsCompatible(const char *szFileName)
{
	QString strString;
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
	QTextStream hAccoFile(&f);

	// Check if found the data line
	//SITE_NO,DUT_NO,DUT_PASS,SW_BIN
	int nLine = 0;
	while(!hAccoFile.atEnd())
	{
		strString = hAccoFile.readLine().remove(" ").toUpper();
		if(strString.startsWith("SITE_NO,DUT_NO,DUT_PASS,SW_BIN"))
		{
			f.close();
			return true;
		}
		if(strString.startsWith("SITE_NUM,PART_TXT,PASSFG,SOFT_BIN"))
		{
			f.close();
			return true;
		}
		if(strString.startsWith("[DEVICESTATISTICS]"))
		{
			f.close();
			return true;
		}
		if(strString.startsWith("[STATISTICSINFORMATION]"))
		{
			f.close();
			return true;
		}
		if(strString.startsWith("[SOFTWAREBININFORMATION]"))
		{
			f.close();
			return true;
		}
		if(strString.startsWith("[HARDWAREBININFORMATION]"))
		{
			f.close();
			return true;
		}
		nLine++;
		if(nLine > 60)
			break;
	}

	// Incorrect header...this is not a Acco file!
	f.close();
	return false;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the Acco file
//////////////////////////////////////////////////////////////////////
bool CGAccotoSTDF::ReadAccoFile(const char *AccoFileName,const char *strFileNameSTDF)
{
	QString strString;
	QString strSection;
	QString strValue;
	bool	bStatus;
	int		iIndex;				// Loop index

	// Open CSV file
	QFile f( AccoFileName );
    if(!f.open( QIODevice::ReadOnly ))
	{
		// Failed Opening Acco file
		iLastError = errOpenFail;

		// Convertion failed.
		return false;
	}

 	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	iNextFilePos = 0;
	iProgressStep = 0;
	iFileSize = f.size() + 1;

	// Assign file I/O stream
	QTextStream hAccoFile(&f);
	QDate	clDate;
	QTime	clTime;

	strString = ReadLine(hAccoFile).simplified();
	if(strString.startsWith("STS",Qt::CaseInsensitive))
	{
		m_strTesterType =strString.section(" ",0,0);
		strString = "";
	}

	// Read Acco information
	while(!hAccoFile.atEnd())
	{
		if(strString.isEmpty())
			strString = ReadLine(hAccoFile).replace(":,",":");

		strSection = strString.section(":",0,0).remove(' ').toLower();

		if((strSection == "[softwarebininformation]")
		||  (strSection == "[hardwarebininformation]"))
		{
			m_strTesterType = "STS8200";
			//Site,Hardware Bin#,Bin Type,Bin description,Bin Count,Binning Percentage
			//All Sites,Bin[2],P,2,0,0.00

			QMap<int,CGAccoBinning *>*pBinTable;
			CGAccoBinning *pBinning;
			int		nBinNum;
			int		nSiteNum;
			bool	bIsValidNum;
			QString strValue;
			QStringList lstValues;

			if(strSection == "[hardwarebininformation]")
				pBinTable = &m_mapAccoHardBinning;
			else
				pBinTable = &m_mapAccoSoftBinning;

			while(strString.indexOf("Bin[") < 0)
			{
				strString = ReadLine(hAccoFile);
				if(strString.startsWith("["))
					break;
				if(strString.startsWith("SITE_N"))
					break;
			}
			while(strString.indexOf("Bin[") >= 0)
			{
                // All Sites,Bin[2],P,2,0,0.00
				lstValues = strString.split(",",QString::KeepEmptyParts);
				strValue = lstValues[0].toUpper();
				strValue = strValue.remove("SITE");
				nSiteNum = strValue.toInt(&bIsValidNum);
				if(!bIsValidNum)
					nSiteNum = 255;

				strSection = lstValues[1];
				if(!strSection.startsWith("bin[",Qt::CaseInsensitive))
				{
					// Error
					break;
				}

				nBinNum = strSection.section("[",1).section("]",0,0).simplified().toInt();
				if((*pBinTable).contains(nBinNum))
					pBinning = (*pBinTable)[nBinNum];
				else
					pBinning = new CGAccoBinning();

				pBinning->nNumber = nBinNum;
				if(lstValues.count() >= 2)
					pBinning->bPass = (lstValues[2].toUpper() == "P");
				if(lstValues.count() >= 3)
					pBinning->strName = lstValues[3].simplified();
				if((lstValues.count() >= 5)
				&& (lstValues[4] != "0")
				&& (lstValues[5] != "0.0"))
					pBinning->mapSumCount[nSiteNum] = lstValues[4].toInt();

				(*pBinTable)[pBinning->nNumber] = pBinning;

				strString = ReadLine(hAccoFile);

			}
		}
		else if(strSection.startsWith("["))
		{
			// Summary section
			// Skip
			while(strString.indexOf("Bin[") < 0)
			{
				strString = ReadLine(hAccoFile);
				if(strString.startsWith("["))
					break;
				if(strString.startsWith("SITE_N",Qt::CaseInsensitive))
					break;
			}
		}
		else if(strSection.startsWith("bin[", Qt::CaseInsensitive))
		{
			// Bins
			CGAccoBinning *pBinning;
			int	nBinNum;
			while(strString.startsWith("bin[", Qt::CaseInsensitive))
			{

				//Bin[1] Pass default: 8057    97.03%
				//Bin[2] Fail default: 0    0.00%
				//Bin[32] Alarm Bin: 0    0.00%
				//Bin[2] Isink1 contact_FAIL: 8    0.10%

				if(strString.startsWith("SITE_N",Qt::CaseInsensitive))
				{
					// Data
					break;
				}


				strSection = strString.section(" ",0,0).toLower();
				strString = strString.section(" ",1);

				if(!strSection.startsWith("Bin[",Qt::CaseInsensitive))
				{
					// Error
					break;
				}

				nBinNum = strSection.section("[",1).section("]",0,0).simplified().toInt();
				if(m_mapAccoSoftBinning.contains(nBinNum))
					pBinning = m_mapAccoSoftBinning[nBinNum];
				else
					pBinning = new CGAccoBinning();

				pBinning->nNumber = nBinNum;
				pBinning->strName = strString.section(":",0,0).simplified();
				pBinning->bPass = pBinning->strName.toUpper().indexOf("FAIL") == -1;
				pBinning->mapSumCount[255] = strString.section(":",1).simplified().section(" ",0,0).toInt();

				m_mapAccoSoftBinning[pBinning->nNumber] = pBinning;

				strString = ReadLine(hAccoFile);

			}
		}
		else if(strSection.startsWith("site_no"))
		{
			// Data
			break;
		}
		else if(strSection.startsWith("site_num"))
		{
			// Data
			break;
		}
		else if(strSection == "date")
		{
			strValue = strString.left(40).section(":",1).simplified();
			strString = "";

			//Date: 2010-02-24
			QString	strDate = strValue;
			clDate = QDate(strDate.section("-",0,0).toInt(),strDate.section("-",1,1).toInt(),strDate.section("-",2,2).toInt());
			clTime = QTime::fromString("00:00:00");

			QDateTime clDateTime(clDate,clTime);
			clDateTime.setTimeSpec(Qt::UTC);
			m_lStartTime = clDateTime.toTime_t();
		}
		else if(strSection == "lotid")
		{
			strValue = strString.left(40).section(":",1).simplified();
			strString = strString.mid(40);

			//LotID: TH60284_9_FP1002-091
			m_strLotID = strValue;
		}
		else if(strSection == "testerno")
		{
			//TesterNo: 3151BDS
			m_strTesterID = strString.section(":",1).simplified();
			strString = "";
		}
		else if(strSection == "program")
		{
			//Program: C:\AccoTest\TESTFILE\CP\3151BDSx2\2K060DFH_RT.prg
			m_strJobName = strString.section(":",1).simplified();
			strString = "";
		}
		else if(strSection == "programrev")
		{
			//Program REV:
			m_strJobRev = strString.section(":",1).simplified();
			strString = "";
		}
		else if((strSection == "operator")||(strSection == "user"))
		{
			strValue = strString.left(40).section(":",1).simplified();
			strString = "";

			//~Operator ID,JRG
			m_strOperatorID = strValue;
		}
		else if(strSection == "beginningtime")
		{
			strValue = strString.left(40).section(":",1).simplified();
			strString = strString.mid(40);

			//Beginning Time: 2010-02-24 ¤U¤È 05:53:01
			QString	strDate = strValue.section(" ",0,0);
			QString strTime = strValue.section(" ",2);
			clDate = QDate(strDate.section("-",0,0).toInt(),strDate.section("-",1,1).toInt(),strDate.section("-",2,2).toInt());
			clTime = QTime::fromString(strTime);

			if(clTime.hour() == 12)
				clTime = clTime.addSecs(-60*60*12);

			if(strValue.toUpper().indexOf("U") > 0)
				clTime = clTime.addSecs(60*60*12);

			QDateTime clDateTime(clDate,clTime);
			clDateTime.setTimeSpec(Qt::UTC);
			m_lStartTime = clDateTime.toTime_t();
		}
		else if(strSection == "endingtime")
		{
			strValue = strString.section(":",1).simplified();
			strString = "";

			//Ending Time: 2010-02-24 ¤U¤È 05:58:52
			QString	strDate = strValue.section(" ",0,0);
			QString strTime = strValue.section(" ",2);
			clDate = QDate(strDate.section("-",0,0).toInt(),strDate.section("-",1,1).toInt(),strDate.section("-",2,2).toInt());
			clTime = QTime::fromString(strTime);

			if(clTime.hour() == 12)
				clTime = clTime.addSecs(-60*60*12);

			if(strValue.toUpper().indexOf("U") > 0)
				clTime = clTime.addSecs(60*60*12);

			QDateTime clDateTime(clDate,clTime);
			clDateTime.setTimeSpec(Qt::UTC);
			m_lStopTime = clDateTime.toTime_t();
		}
		else if(strSection == "waferid")
		{
			//WaferID:
			m_strWaferID = strString.section(":",1).simplified();
			strString = "";
		}
		else if(strSection == "total")
		{
			m_nTotalParts = strString.simplified().section(" ",1,1).toInt();
			strString = "";
		}
		else if(strSection == "pass")
		{
			m_nPassParts = strString.simplified().section(" ",1,1).toInt();
			strString = "";
		}
		else if(strSection == "fail")
		{
			m_nFailParts = strString.simplified().section(" ",1,1).toInt();
			strString = "";
		}
		else
		{
			strString = "";
		}



	}

	while(strString.simplified().isEmpty() && !hAccoFile.atEnd())
		strString = ReadLine(hAccoFile);

	//SITE_NO,DUT_NO,DUT_PASS,SW_BIN,T_TIME,DATA_NUM,Isink1 contact,Isink2 contact,Isink3 contact,Isink4 contact,ENSET contact,IN contact,OUTCP contact,INTA contact,INTB contact,INTC contact,INTD contact,Icc_1x,IDsense1,IDsense2,IDsense3,ID_sense4,EN_lkg,Pre_io1,Pre_io1_err,Pre_io2,Pre_io2_err,Pre_io3,Pre_io3_err,Pre_io4,Pre_io4_err,Trim_Code,INTA check,INTB check,INTC check,INTD check,Post_io1,Post_io2,Post_io3,Post_io4,fs_avg,fs_avg_err,io_match,io1_code14,io2_code14,io3_code14,io4_code14,vout_no_load,startup,
	//Unit,,,,ms,,V,V,V,V,V,V,V,V,V,V,V,uA,uA,uA,uA,uA,uA,mA,mA,mA,mA,mA,mA,mA,mA,,V,V,V,V,mA,mA,mA,mA,mA,%,%,mA,mA,mA,mA,V,mA,
	//LimitL,,,,,,-0.9000,-0.9000,-0.9000,-0.9000,-0.9000,-0.9000,-0.9000,,,,,90.0000,95.0000,95.0000,95.0000,95.0000,-0.0900,,-10.0000,,-10.0000,,-10.0000,,-10.0000,-8.0000,,,,,,,,,,-8.0000,-2.8000,4.0000,4.0000,4.0000,4.0000,4.0000,100.0000,
	//LimitU,,,,,,-0.3000,-0.3000,-0.3000,-0.3000,-0.3000,-0.3000,-0.3000,0.5000,0.5000,0.5000,0.5000,350.0000,225.0000,225.0000,225.0000,225.0000,0.0900,,10.0000,,10.0000,,10.0000,,10.0000,8.0000,,,,,,,,,,8.0000,2.8000,5.6000,5.6000,5.6000,5.6000,,,


	if((strString.startsWith("SITE_NO,DUT_NO,DUT_PASS,SW_BIN", Qt::CaseInsensitive) == false)
	&& (strString.startsWith("SITE_NUM,PART_TXT,PASSFG,SOFT_BIN", Qt::CaseInsensitive) == false))
	{
		// Incorrect header...this is not a Acco file!
		f.close();
		return false;
	}
	m_iDataOffset = 6;
	m_iXYCoordPos = -1;
	if((strString.indexOf("X_COORD,Y_COORD", Qt::CaseInsensitive) > 0))
	{
		m_iDataOffset += 2;
		m_iXYCoordPos = strString.left(strString.indexOf("X_COORD,Y_COORD", Qt::CaseInsensitive)).count(",");
	}

    QStringList lstSections = strString.split(",",QString::KeepEmptyParts);
	// Count the number of parameters specified in the line
	m_iTotalParameters=lstSections.count();
	// If no parameter specified...ignore!
	if(m_iTotalParameters <= 0)
	{
		// Incorrect header...this is not a valid Acco file!
		iLastError = errInvalidFormat;

		// Convertion failed.
		f.close();
		return false;
	}

	// Allocate the buffer to hold the N parameters & results.
	m_pAccoParameter = new CGAccoParameter[m_iTotalParameters];	// List of parameters

	strString = ReadLine(hAccoFile);
	if(strString.startsWith("Unit", Qt::CaseInsensitive) == false)
	{
		// Incorrect header...this is not a Acco file!
		f.close();
		return false;
	}
    QStringList lstUnits = strString.split(",",QString::KeepEmptyParts);
	strString = ReadLine(hAccoFile);
	if(strString.startsWith("LimitL", Qt::CaseInsensitive) == false)
	{
		// Incorrect header...this is not a Acco file!
		f.close();
		return false;
	}
    QStringList lstLowLimits = strString.split(",",QString::KeepEmptyParts);
	strString = ReadLine(hAccoFile);
	if(strString.startsWith("LimitU", Qt::CaseInsensitive) == false)
	{
		// Incorrect header...this is not a Acco file!
		f.close();
		return false;
	}
    QStringList lstHighLimits = strString.split(",",QString::KeepEmptyParts);

	// Extract the N column names
	// Do not count first 4 fields
	QString strUnit;
	QString strLowLimit;
	QString strHighLimit;
	int nScale;
	for(iIndex=m_iDataOffset;iIndex<m_iTotalParameters;iIndex++)
	{
		strSection = lstSections[iIndex];

		if(strSection.isEmpty())
			break;

		strUnit = lstUnits[iIndex];
		strLowLimit = lstLowLimits[iIndex];
		strHighLimit = lstHighLimits[iIndex];
		nScale = 0;

		NormalizeLimits(strUnit, nScale);

		UpdateParameterIndexTable(strSection);		// Update Parameter master list if needed.

		m_pAccoParameter[iIndex].nNumber = m_pFullAccoParametersList.indexOf(strSection);
		m_pAccoParameter[iIndex].strName = strSection;
		m_pAccoParameter[iIndex].strUnits = strUnit;
		m_pAccoParameter[iIndex].nScale = nScale;
		m_pAccoParameter[iIndex].fHighLimit = strHighLimit.toFloat(&(m_pAccoParameter[iIndex].bValidHighLimit));
		m_pAccoParameter[iIndex].fLowLimit = strLowLimit.toFloat(&(m_pAccoParameter[iIndex].bValidLowLimit));
		m_pAccoParameter[iIndex].bStaticHeaderWritten = false;
	}

	m_iTotalParameters = iIndex;

	// Loop reading file until end is reached & generate STDF file dynamically.
	bStatus = WriteStdfFile(&hAccoFile,strFileNameSTDF);
	if(!bStatus)
		QFile::remove(strFileNameSTDF);

	// All Acco file read...check if need to update the Acco Parameter list on disk?
	if(bStatus && (m_bNewAccoParameterFound == true))
		DumpParameterIndexTable();

	// Success parsing Acco file
	f.close();
	return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from Acco data parsed
//////////////////////////////////////////////////////////////////////
bool CGAccotoSTDF::WriteStdfFile(QTextStream *hAccoFile, const char *strFileNameSTDF)
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
	StdfFile.WriteString(m_strTesterID.toLatin1().constData());		// Node name
	StdfFile.WriteString(m_strTesterType.toLatin1().constData());	// Tester Type
	StdfFile.WriteString(m_strJobName.toLatin1().constData());		// Job name
	StdfFile.WriteString(m_strJobRev.toLatin1().constData());		// Job rev
	StdfFile.WriteString("");					// sublot-id
	StdfFile.WriteString(m_strOperatorID.toLatin1().constData());	// operator
	StdfFile.WriteString("");					// exec-type
	StdfFile.WriteString("");					// exe-ver
	StdfFile.WriteString("WAFER");				// test-cod
	StdfFile.WriteString("");					// test-temperature
    // Construct custom Galaxy USER_TXT
	QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
	strUserTxt += ":";
	strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
	strUserTxt += ":ACCO_"+m_strTesterType.toUpper();
	StdfFile.WriteString((char *)strUserTxt.toLatin1().constData());	// user-txt
	StdfFile.WriteString("");							// aux-file
	StdfFile.WriteString("");							// package-type
	StdfFile.WriteString(m_strProductID.toLatin1().constData());	// familyID
	StdfFile.WriteRecord();

	// Write Test results for each line read.
	QString strString;
	char	szString[257];
	QString strSection;
	float	fValue;				// Used for readng floating point numbers.
	int		iIndex;				// Loop index
	int		iSite;
	int		iXpos, iYpos;
	int		iBin;
	int		iTime;
	bool	bPassStatus;
	long		iTotalGoodBin,iTotalFailBin;
	long		iTotalTests,iPartNumber;
	bool		bStatus;
	QStringList	lstSections;
	BYTE		bData;
	CGAccoBinning* pBinning;

	// Reset counters
	iTotalGoodBin=iTotalFailBin=0;
	iPartNumber=0;

	// Write all Parameters read on this wafer.: WIR.PIR,PTR....PTR, PRR, PIR, PTR,PTR...PRR,   ... WRR

	if(m_strWaferID.isEmpty())
		m_strWaferID = "1";

	// Write WIR of new Wafer.
	RecordReadInfo.iRecordType = 2;
	RecordReadInfo.iRecordSubType = 10;
	StdfFile.WriteHeader(&RecordReadInfo);
	StdfFile.WriteByte(1);								// Test head
	StdfFile.WriteByte(255);							// Tester site (all)
	StdfFile.WriteDword(m_lStartTime);					// Start time
	StdfFile.WriteString(m_strWaferID.toLatin1().constData());	// WaferID
	StdfFile.WriteRecord();

	//SITE_NO,DUT_NO,DUT_PASS,SW_BIN,T_TIME,DATA_NUM,Isink1 contact,Isink2 contact,Isink...
	// ...
	//1,16,True,1,288,43,-0.6182,-0.6189,-0.6183,-0.6183,-0.6190,-0.5645,-0.5931,0.0020,...
	//2,16,True,1,299,43,-0.6190,-0.6196,-0.6190,-0.6190,-0.6200,-0.5651,-0.5953,0.0023,...

	while(hAccoFile->atEnd() == false)
	{

		// Read line
		strString = ReadLine(*hAccoFile).simplified();

		if(strString.isEmpty())
			continue;

        lstSections = strString.split(",",QString::KeepEmptyParts);

		iIndex = 0;
		iSite = lstSections[iIndex++].toInt(&bStatus);

		// Check if start with a valid site num
		if(!bStatus)
			continue;

		// Check if have the good count
		if(lstSections.count() < m_iDataOffset)
		{
			iLastError = errInvalidFormatLowInRows;

			StdfFile.Close();
			// Convertion failed.
			return false;
		}
		iXpos = iYpos = -32768;
		iPartNumber = lstSections[iIndex++].toInt();
		bPassStatus = lstSections[iIndex++].startsWith("True",Qt::CaseInsensitive);
		iBin = lstSections[iIndex++].toInt();
		if(m_iXYCoordPos == iIndex)
		{
			iXpos = lstSections[iIndex++].toInt();
			iYpos = lstSections[iIndex++].toInt();
		}
		iTime = lstSections[iIndex++].toInt();
		if(m_iXYCoordPos == iIndex)
		{
			iXpos = lstSections[iIndex++].toInt();
			iYpos = lstSections[iIndex++].toInt();
		}
		lstSections[iIndex++].toInt();

		if(!m_mapAccoSoftBinning.contains(iBin))
		{
			pBinning = new CGAccoBinning();

			pBinning->nNumber = iBin;
			pBinning->strName = "";
			m_mapAccoSoftBinning[iBin] = pBinning;
		}

		pBinning = m_mapAccoSoftBinning[iBin];

		pBinning->bPass = bPassStatus;
		if(!pBinning->mapSiteCount.contains(iSite))
			pBinning->mapSiteCount[iSite] = 0;

		pBinning->mapSiteCount[iSite]++;

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
				break;

			fValue = strSection.toFloat();

			// Valid test result...write the PTR
			iTotalTests++;

			RecordReadInfo.iRecordType = 15;
			RecordReadInfo.iRecordSubType = 10;
			StdfFile.WriteHeader(&RecordReadInfo);
			// Compute Test# (add user-defined offset)
			StdfFile.WriteDword(m_pAccoParameter[iIndex].nNumber + GEX_TESTNBR_OFFSET_ACCO);			// Test Number
			StdfFile.WriteByte(1);						// Test head
			StdfFile.WriteByte(iSite);			// Tester site#

			if(!m_pAccoParameter[iIndex].bValidLowLimit && !m_pAccoParameter[iIndex].bValidHighLimit)
			{
				// No pass/fail information
				bData = 0x40;
			}
			else if(((m_pAccoParameter[iIndex].bValidLowLimit==true) && (fValue < m_pAccoParameter[iIndex].fLowLimit)) ||
			   ((m_pAccoParameter[iIndex].bValidHighLimit==true) && (fValue > m_pAccoParameter[iIndex].fHighLimit)))
			{
				bData = 0200;	// Test Failed
			}
			else
			{
				bData = 0;		// Test passed
			}
			StdfFile.WriteByte(bData);							// TEST_FLG
			StdfFile.WriteByte(0x40|0x80);						// PARAM_FLG
			StdfFile.WriteFloat(fValue * GS_POW(10.0,m_pAccoParameter[iIndex].nScale));						// Test result
			if(m_pAccoParameter[iIndex].bStaticHeaderWritten == false)
			{
				StdfFile.WriteString(m_pAccoParameter[iIndex].strName.toLatin1().constData());	// TEST_TXT
				StdfFile.WriteString("");							// ALARM_ID
				bData = 2;	// Valid data.
				if(m_pAccoParameter[iIndex].bValidLowLimit==false)
					bData |=0x40;
				if(m_pAccoParameter[iIndex].bValidHighLimit==false)
					bData |=0x80;
				StdfFile.WriteByte(bData);							// OPT_FLAG
				StdfFile.WriteByte(-m_pAccoParameter[iIndex].nScale);			// RES_SCALE
				StdfFile.WriteByte(-m_pAccoParameter[iIndex].nScale);			// LLM_SCALE
				StdfFile.WriteByte(-m_pAccoParameter[iIndex].nScale);			// HLM_SCALE
				StdfFile.WriteFloat(m_pAccoParameter[iIndex].fLowLimit * GS_POW(10.0,m_pAccoParameter[iIndex].nScale));		// LOW Limit
				StdfFile.WriteFloat(m_pAccoParameter[iIndex].fHighLimit * GS_POW(10.0,m_pAccoParameter[iIndex].nScale));		// HIGH Limit
				StdfFile.WriteString(m_pAccoParameter[iIndex].strUnits.toLatin1().constData());	// Units
				m_pAccoParameter[iIndex].bStaticHeaderWritten = true;
			}
			StdfFile.WriteRecord();
		}		// Read all results on line

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
		}
		else
		{
			StdfFile.WriteByte(8);				// PART_FLG : FAILED
			iTotalFailBin++;
		}
		StdfFile.WriteWord((WORD)iTotalTests);	// NUM_TEST
		StdfFile.WriteWord(iBin);				// HARD_BIN
		StdfFile.WriteWord(iBin);				// SOFT_BIN
		StdfFile.WriteWord(iXpos);				// X_COORD
		StdfFile.WriteWord(iYpos);				// Y_COORD
		StdfFile.WriteDword(iTime);				// testing time
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
	StdfFile.WriteString(QString("SIDE["+m_strWaferID+"]").toLatin1().constData());	// UserDesc
	StdfFile.WriteRecord();


	QMap<int,CGAccoBinning *>::Iterator it;
	QMap<int,int>::Iterator itSite;

	RecordReadInfo.iRecordType = 1;
	RecordReadInfo.iRecordSubType = 40;
	for(it = m_mapAccoHardBinning.begin(); it != m_mapAccoHardBinning.end(); it++)
	{
		pBinning = *it;

		for(itSite = pBinning->mapSumCount.begin(); itSite != pBinning->mapSumCount.end(); itSite++)
		{
			// Write HBR/site
			StdfFile.WriteHeader(&RecordReadInfo);
			StdfFile.WriteByte(1);						// Test Head
			StdfFile.WriteByte(itSite.key());			// Test sites
			StdfFile.WriteWord(pBinning->nNumber);		// HBIN
            StdfFile.WriteDword(itSite.value());			// Total Bins
			if(pBinning->bPass)
				StdfFile.WriteByte('P');
			else
				StdfFile.WriteByte('F');
			StdfFile.WriteString( pBinning->strName.toLatin1().constData());
			StdfFile.WriteRecord();

			if(pBinning->mapSiteCount.contains(itSite.key()))
				pBinning->mapSiteCount.remove(itSite.key());
		}

		for(itSite = pBinning->mapSiteCount.begin(); itSite != pBinning->mapSiteCount.end(); itSite++)
		{
			// Write HBR/site
			StdfFile.WriteHeader(&RecordReadInfo);
			StdfFile.WriteByte(1);						// Test Head
			StdfFile.WriteByte(itSite.key());			// Test sites
			StdfFile.WriteWord(pBinning->nNumber);		// HBIN
            StdfFile.WriteDword(itSite.value());			// Total Bins
			if(pBinning->bPass)
				StdfFile.WriteByte('P');
			else
				StdfFile.WriteByte('F');
			StdfFile.WriteString( pBinning->strName.toLatin1().constData());
			StdfFile.WriteRecord();
		}

	}

	RecordReadInfo.iRecordType = 1;
	RecordReadInfo.iRecordSubType = 50;
	for(it = m_mapAccoSoftBinning.begin(); it != m_mapAccoSoftBinning.end(); it++)
	{
		pBinning = *it;

		for(itSite = pBinning->mapSumCount.begin(); itSite != pBinning->mapSumCount.end(); itSite++)
		{
			// Write SBR/site
			StdfFile.WriteHeader(&RecordReadInfo);
			StdfFile.WriteByte(1);						// Test Head
			StdfFile.WriteByte(itSite.key());			// Test sites
			StdfFile.WriteWord(pBinning->nNumber);		// SBIN
            StdfFile.WriteDword(itSite.value());			// Total Bins
			if(pBinning->bPass)
				StdfFile.WriteByte('P');
			else
				StdfFile.WriteByte('F');
			StdfFile.WriteString( pBinning->strName.toLatin1().constData());
			StdfFile.WriteRecord();

			if(pBinning->mapSiteCount.contains(itSite.key()))
				pBinning->mapSiteCount.remove(itSite.key());
		}

		for(itSite = pBinning->mapSiteCount.begin(); itSite != pBinning->mapSiteCount.end(); itSite++)
		{
			// Write SBR/site
			StdfFile.WriteHeader(&RecordReadInfo);
			StdfFile.WriteByte(1);						// Test Head
			StdfFile.WriteByte(itSite.key());			// Test sites
			StdfFile.WriteWord(pBinning->nNumber);		// SBIN
            StdfFile.WriteDword(itSite.value());			// Total Bins
			if(pBinning->bPass)
				StdfFile.WriteByte('P');
			else
				StdfFile.WriteByte('F');
			StdfFile.WriteString( pBinning->strName.toLatin1().constData());
			StdfFile.WriteRecord();
		}
	}


	if(m_nTotalParts == 0)
	{
		m_nTotalParts = iTotalGoodBin + iTotalFailBin;
		m_nPassParts = iTotalGoodBin;
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
// Convert 'FileName' Acco file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGAccotoSTDF::Convert(const char *AccoFileName, const char *strFileNameSTDF)
{
	// No erro (default)
	iLastError = errNoError;

	// If STDF file already exists...do not rebuild it...unless dates not matching!
	QFileInfo fInput(AccoFileName);
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
			GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(AccoFileName).fileName()+"...");
			GexScriptStatusLabel->show();
		}
		GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(AccoFileName).fileName()+"...");
		GexScriptStatusLabel->show();
	}
    QCoreApplication::processEvents();

    if(ReadAccoFile(AccoFileName,strFileNameSTDF) != true)
	{
		//////////////////////////////////////////////////////////////////////
		// For ProgressBar
		if((GexProgressBar != NULL)
		&& bHideProgressAfter)
			GexProgressBar->hide();

		if((GexScriptStatusLabel != NULL)
		&& bHideLabelAfter)
			GexScriptStatusLabel->hide();
		return false;	// Error reading Acco file
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
void CGAccotoSTDF::NormalizeLimits(QString &strUnit, int &nScale)
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
QString CGAccotoSTDF::ReadLine(QTextStream& hFile)
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
