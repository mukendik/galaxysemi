//////////////////////////////////////////////////////////////////////
// import_tessera_inspection.cpp: Convert a TesseraInspection .csv
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
#include "import_tessera_inspection.h"
#include "time.h"
#include "import_constants.h"
//
//inspection1field-jgardner-WCXXX-U-17870-235DT-200901271100-FULL-MPTS.csv
//~Version,v1.1
//~Unit,micron
//~Product,{product-id}
//~Lot,944
//~Wafer,17870-235DT
//~Side,B
//~Slot ID,0
//~Start Date/Time,27/01/2009 10:53:46
//~End Date/Time,27/01/2009 11:00:41
//~Operator ID,JRG
//~Machine ID,Demo Rev2
//~Recipe,C:\ICOS\RECIPES\944B_8D_TESSERAMAP_FIELD_EME_1.RCP
//
//~Bins,PASS/FAIL (resulting code)
//~955,Pass,pass
//~700,Invalid,fail
//~799,Failed at Previous Test,fail
//~219,Accumulated Field,fail
//~219,Field Bubbles,fail
//~111,Field Scratches,fail
//~111,Field Debris,fail
//~119,Area Lens Fail,fail
//~199,Scratch Lens Fail,fail
//~299,Scratch Field Fail,fail
//~500,Unpopulated,fail
//~555,Reference,fail
//~855,Operator Accept,pass
//
//Column,Row,Pass,~Bin,~Defect Area Lens (um^2),~Scratch Length Lens (um),~Defect Area Field (um),~Scratch Length Field (um),~Die Size X (um),~Die Size Y (um),~Lens Position X (um),~Lens Position Y (um),~Parent Wafer,~Parent Row,~Parent Column
//0,0,0,500,,,,,,,,,,,
//0,1,0,500,,,,,,,,,,,

// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGTesseraInspectiontoSTDF::CGTesseraInspectiontoSTDF()
{
	// Default: TesseraInspection parameter list on disk includes all known TesseraInspection parameters...
	m_bNewTesseraInspectionParameterFound = false;
	m_lStartTime = 0;
	m_strTesterType = "Inspection";

	m_pTesseraInspectionParameter = NULL;
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CGTesseraInspectiontoSTDF::~CGTesseraInspectiontoSTDF()
{
	// Destroy list of Parameters tables.
	if(m_pTesseraInspectionParameter!=NULL)
		delete [] m_pTesseraInspectionParameter;
	// Destroy list of Bin tables.
	QMap<int,CGTesseraInspectionBinning *>::Iterator it;
	for(it = m_mapTesseraInspectionBinning.begin(); it!= m_mapTesseraInspectionBinning.end(); it++)
		delete m_mapTesseraInspectionBinning[it.key()];
	m_mapTesseraInspectionBinning.clear();
}


//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGTesseraInspectiontoSTDF::GetLastError()
{
	strLastError = "Import Tessera Inspection: ";

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
			strLastError += "Invalid file format: Specification Limits not found";
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
// Load TesseraInspection Parameter table from DISK
//////////////////////////////////////////////////////////////////////
void CGTesseraInspectiontoSTDF::LoadParameterIndexTable(void)
{
	QString	strTesseraInspectionTableFile;
	QString	strString;

    strTesseraInspectionTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strTesseraInspectionTableFile += GEX_TESSERA_PARAMETERS;

	// Open TesseraInspection Parameter table file
    QFile f( strTesseraInspectionTableFile );
    if(!f.open( QIODevice::ReadOnly ))
		return;

	// Assign file I/O stream
	QTextStream hTesseraInspectionTableFile(&f);

	// Skip comment lines
	do
	{
	  strString = hTesseraInspectionTableFile.readLine();
	}
    while((strString.indexOf("----------------------") < 0) && (hTesseraInspectionTableFile.atEnd() == false));

	// Read lines
	m_pFullTesseraInspectionParametersList.clear();
	strString = hTesseraInspectionTableFile.readLine();
	while (strString.isNull() == false)
	{
		// Save Parameter name in list
		m_pFullTesseraInspectionParametersList.append(strString);
		// Read next line
		strString = hTesseraInspectionTableFile.readLine();
	};

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// Save TesseraInspection Parameter table to DISK
//////////////////////////////////////////////////////////////////////
void CGTesseraInspectiontoSTDF::DumpParameterIndexTable(void)
{
	QString		strTesseraInspectionTableFile;
	QString		strString;
	int	uIndex;

    strTesseraInspectionTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strTesseraInspectionTableFile += GEX_TESSERA_PARAMETERS;

	// Open TesseraInspection Parameter table file
    QFile f( strTesseraInspectionTableFile );
    if(!f.open( QIODevice::WriteOnly ))
		return;

	// Assign file I/O stream
	QTextStream hTesseraInspectionTableFile(&f);

	// First few lines are comments:
	hTesseraInspectionTableFile << "############################################################" << endl;
	hTesseraInspectionTableFile << "# DO NOT EDIT THIS FILE!" << endl;
    hTesseraInspectionTableFile << "# Quantix Examinator: Tessera Parameters detected" << endl;
	hTesseraInspectionTableFile << "# www.mentor.com" << endl;
    hTesseraInspectionTableFile << "# Quantix Examinator reads and writes into this file..." << endl;
	hTesseraInspectionTableFile << "-----------------------------------------------------------" << endl;

	// Write lines
	// m_pFullTesseraInspectionParametersList.sort();
	for(uIndex=0;uIndex<m_pFullTesseraInspectionParametersList.count();uIndex++)
	{
		// Write line
		hTesseraInspectionTableFile << m_pFullTesseraInspectionParametersList[uIndex] << endl;
	};

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// If Examinator doesn't have this TesseraInspection parameter in his dictionnary, have it added.
//////////////////////////////////////////////////////////////////////
void CGTesseraInspectiontoSTDF::UpdateParameterIndexTable(QString strParamName)
{
	// Check if the table is empty...if so, load it from disk first!
	if(m_pFullTesseraInspectionParametersList.isEmpty() == true)
	{
		// Load TesseraInspection parameter table from disk...
		LoadParameterIndexTable();
	}
	
	// Check if Parameter name already in table...if not, add it to the list
	// the new full list will be dumped to the disk at the end.
    if(m_pFullTesseraInspectionParametersList.indexOf(strParamName) < 0)
	{
		// Update list
		m_pFullTesseraInspectionParametersList.append(strParamName);

		// Set flag to force the current TesseraInspection table to be updated on disk
		m_bNewTesseraInspectionParameterFound = true;
	}
}


//////////////////////////////////////////////////////////////////////
// Check if File is compatible with TesseraInspection format
//////////////////////////////////////////////////////////////////////
bool CGTesseraInspectiontoSTDF::IsCompatible(const char *szFileName)
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
	QTextStream hTesseraInspectionFile(&f);

	// Check if first line is the correct TesseraInspection header...
	//inspection1field-jgardner-WCXXX-U-17870-235DT-200901271100-FULL-MPTS.csv
	//~Version,v1.1
	do
		strString = hTesseraInspectionFile.readLine();
	while(!strString.isNull() && strString.isEmpty());

	// inspection
	if(strString.startsWith("inspection", Qt::CaseInsensitive) == false)
	{
		// Incorrect header...this is not a TesseraInspection file!
		f.close();
		return false;
	}

	do
		strString = hTesseraInspectionFile.readLine();
	while(!strString.isNull() && strString.isEmpty());

	strString = strString.remove(QRegExp("^~,?"));

	// Product Name
	if(strString.startsWith("Version", Qt::CaseInsensitive) == false)
	{
		// Incorrect header...this is not a TesseraInspection file!
		f.close();
		return false;
	}

	f.close();

	return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the TesseraInspection file
//////////////////////////////////////////////////////////////////////
bool CGTesseraInspectiontoSTDF::ReadTesseraInspectionFile(const char *TesseraInspectionFileName,const char *strFileNameSTDF)
{
	QString strString;
	QString strSection;
	bool	bStatus;
	int		iIndex;				// Loop index

	// Open CSV file
    QFile f( TesseraInspectionFileName );
    if(!f.open( QIODevice::ReadOnly ))
	{
		// Failed Opening TesseraInspection file
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
	QTextStream hTesseraInspectionFile(&f);

	// Check if first line is the correct TesseraInspection header...
	//inspection1field-jgardner-WCXXX-U-17870-235DT-200901271100-FULL-MPTS.csv

	strString = ReadLine(hTesseraInspectionFile);
	// inspection
	if(strString.startsWith("inspection", Qt::CaseInsensitive) == false)
	{
		// Incorrect header...this is not a TesseraInspection file!
		f.close();
		return false;
	}

	// Tessera Naming convention
	//
	//X1-ppenilla-diffrac2-S-18538-12UV-200906121554-Sample
	m_strJobName = strString.simplified();
	m_strSetupId="LensSettings="+m_strJobName;
	if(strString.count("-") >= 7)
	{
		QDate	clDate;
		QTime	clTime;
		// Good naming convention
		// extract info
		m_strProductID = strString.section("-",0,0);
		m_strOperatorID = strString.section("-",1,1);
		m_strTesterID = strString.section("-",2,2);
		m_strWaferID = strString.section("-",4,5);
		m_strJobRev = strString.section("-",7,7);

		//Date Time:200906121554
		QString	strDate = strString.section("-",6,6);
		clDate = QDate(strDate.left(4).toInt(),strDate.mid(4,2).toInt(),strDate.mid(6,2).toInt());
		clTime.setHMS(strDate.mid(8,2).toInt(), strDate.right(2).toInt(), 0);


		QDateTime clDateTime(clDate,clTime);
		clDateTime.setTimeSpec(Qt::UTC);
		m_lStartTime = clDateTime.toTime_t();
	}

	// NEW FORMAT
	// a comma "," was inserted after the "~"
		
	//~Version,v1.1
	//~Unit,micron
	//~Product,{product-id}
	//~Lot,944
	//~Wafer,17870-235DT
	//~Side,B
	//~Slot ID,0
	//~Start Date/Time,27/01/2009 10:53:46
	//~End Date/Time,27/01/2009 11:00:41
	//~Operator ID,JRG
	//~Machine ID,Demo Rev2
	//~Recipe,C:\ICOS\RECIPES\944B_8D_TESSERAMAP_FIELD_EME_1.RCP
	//
	QDate	clDate;
	QTime	clTime;

	strString = "";
	// Read Tessera information
	while(!hTesseraInspectionFile.atEnd())
	{
		if(strString.isEmpty())
			strString = ReadLine(hTesseraInspectionFile);

		// Delete "~," or "~" at the beginning of the line
		strString = strString.remove(QRegExp("^~,?"));

		strSection = strString.section(",",0,0);
		strString = strString.section(",",1,1);

		if(strSection.toLower() == "bins")
		{
			// Bin list
			break;
		}
		else if(strSection.toLower() == "column")
		{
			// Data
			break;
		}
		else if(strSection.toLower() == "version")
		{
			//~Version,v1.1
			m_strSoftVer = strString;
			strString = "";
		}
		else if(strSection.toLower() == "start date/time")
		{
			//~Start Date/Time,27/01/2009 10:53:46
			QString	strDate = strString.section(" ",0,0);
			QString strTime = strString.section(" ",1);
			clDate = QDate(strDate.section("/",2,2).toInt(),strDate.section("/",1,1).toInt(),strDate.section("/",0,0).toInt());
			clTime = QTime::fromString(strTime);
			
			QDateTime clDateTime(clDate,clTime);
			clDateTime.setTimeSpec(Qt::UTC);
			m_lStartTime = clDateTime.toTime_t();

			strString = "";
		}
		else if(strSection.toLower() == "end date/time")
		{
			//~End Date/Time,27/01/2009 10:53:46
			QString	strDate = strString.section(" ",0,0);
			QString strTime = strString.section(" ",1);
			clDate = QDate(strDate.section("/",2,2).toInt(),strDate.section("/",1,1).toInt(),strDate.section("/",0,0).toInt());
			clTime = QTime::fromString(strTime);
			
			QDateTime clDateTime(clDate,clTime);
			clDateTime.setTimeSpec(Qt::UTC);
			m_lStopTime = clDateTime.toTime_t();

			strString = "";
		}
		else if(strSection.toLower() == "product")
		{
			//~Product,{product-id}
			if(strString != "{product-id}")
				m_strProductID = strString;
			
			strString = "";
		}
		else if(strSection.toLower() == "lot")
		{
			//~Lot,944
			m_strLotID = strString;
			strString = "";
		}
		else if(strSection.toLower() == "wafer")
		{
			//~Wafer,17870-235DT
			m_strWaferID = strString;
			strString = "";
		}
		else if(strSection.toLower() == "operator id")
		{
			//~Operator ID,JRG
			m_strOperatorID = strString;
			strString = "";
		}
		else if(strSection.toLower() == "machine id")
		{
			//~Machine ID,Demo Rev2
			m_strTesterID = strString;
			strString = "";
		}
		else  
		{
			// Setup Configuration
			if(!m_strSetupId.isEmpty())
				m_strSetupId += ";";
			m_strSetupId += strSection.remove(' ')+"="+strString.simplified();

			strString = "";
		}

	}

	if(strSection.toLower() == "bins")
	{

		//~Bins,PASS/FAIL (resulting code)
		//~955,Pass,pass
		//~700,Invalid,fail
		//~799,Failed at Previous Test,fail
		//~219,Accumulated Field,fail
		//~219,Field Bubbles,fail
		//~111,Field Scratches,fail
		//~111,Field Debris,fail
		//~119,Area Lens Fail,fail
		//~199,Scratch Lens Fail,fail
		//~299,Scratch Field Fail,fail
		//~500,Unpopulated,fail
		//~555,Reference,fail
		//~855,Operator Accept,pass

		CGTesseraInspectionBinning *pBinning;
		while(!hTesseraInspectionFile.atEnd())
		{
			strString = ReadLine(hTesseraInspectionFile);

			strSection = strString.section(",",0,0).toLower();

			if(strSection == "column")
			{
				// Data
				break;
			}

			// Delete "~," or "~" at the beginning of the line
			strString = strString.remove(QRegExp("^~,?"));
			iIndex = 0;
			
			pBinning = new CGTesseraInspectionBinning();

			pBinning->nNumber = strString.section(",",iIndex,iIndex).toInt();
			iIndex++;
			pBinning->strName = strString.section(",",iIndex,iIndex);
			iIndex++;
			pBinning->bPass = (strString.section(",",iIndex,iIndex).toLower() == "pass");
			iIndex++;
			pBinning->nCount = 0;

			pBinning->bToIgnore = pBinning->strName.startsWith("Unpopulated", Qt::CaseInsensitive);
			
			m_mapTesseraInspectionBinning[pBinning->nNumber] = pBinning;

		}
	}


	//Column,Row,Pass,~Bin,~Defect Area Lens (um^2),~Scratch Length Lens (um),~Defect Area Field (um),~Scratch Length Field (um),~Die Size X (um),~Die Size Y (um),~Lens Position X (um),~Lens Position Y (um),~Parent Wafer,~Parent Row,~Parent Column
	
	// Delete all "~" into the line
	strString = strString.remove("~");
	
	if(strString.startsWith("Column,Row,Pass,Bin", Qt::CaseInsensitive) == false)
	{
		// Incorrect header...this is not a TesseraInspection file!
		f.close();
		return false;
	}

    QStringList lstSections = strString.split(",",QString::KeepEmptyParts);

	// Count the number of parameters specified in the line
	m_iTotalParameters=lstSections.count();
	// If no parameter specified...ignore!
	if(m_iTotalParameters <= 0)
	{
		// Incorrect header...this is not a valid TesseraInspection file!
		iLastError = errInvalidFormat;
		
		// Convertion failed.
		f.close();
		return false;
	}

	// Allocate the buffer to hold the N parameters & results.
	m_pTesseraInspectionParameter = new CGTesseraInspectionParameter[m_iTotalParameters];	// List of parameters

	// Extract the N column names
	// Do not count first 4 fields
	QString strUnit;
	int nScale;
	for(iIndex=4;iIndex<m_iTotalParameters;iIndex++)
	{
		strSection = lstSections[iIndex];
		strUnit = "";
		nScale = 0;

		// Check if have some unit
		if(strSection.indexOf("(") > 0)
		{
			// unit
			strUnit = strSection.section("(",1).remove(")");
			NormalizeLimits(strUnit, nScale);
			strSection = strSection.section("(",0,0).trimmed();
		}

		if(strSection.isEmpty())
			break;

		UpdateParameterIndexTable(strSection);		// Update Parameter master list if needed.

		m_pTesseraInspectionParameter[iIndex].nNumber = m_pFullTesseraInspectionParametersList.indexOf(strSection);
		m_pTesseraInspectionParameter[iIndex].strName = strSection;
		m_pTesseraInspectionParameter[iIndex].strUnits = strUnit;
		m_pTesseraInspectionParameter[iIndex].nScale = nScale;
		m_pTesseraInspectionParameter[iIndex].bValidHighLimit = false;
		m_pTesseraInspectionParameter[iIndex].bValidLowLimit = false;
		m_pTesseraInspectionParameter[iIndex].fHighLimit = 0;
		m_pTesseraInspectionParameter[iIndex].fLowLimit = 0;
		m_pTesseraInspectionParameter[iIndex].bStaticHeaderWritten = false;
	}

	m_iTotalParameters = iIndex;


	// Loop reading file until end is reached & generate STDF file dynamically.
	bStatus = WriteStdfFile(&hTesseraInspectionFile,strFileNameSTDF);
	if(!bStatus)
		QFile::remove(strFileNameSTDF);

	// All TesseraInspection file read...check if need to update the TesseraInspection Parameter list on disk?
	if(bStatus && (m_bNewTesseraInspectionParameterFound == true))
		DumpParameterIndexTable();

	// Success parsing TesseraInspection file
	f.close();
	return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from TesseraInspection data parsed
//////////////////////////////////////////////////////////////////////
bool CGTesseraInspectiontoSTDF::WriteStdfFile(QTextStream *hTesseraInspectionFile, const char *strFileNameSTDF)
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
	StdfFile.WriteString(m_strTesterID.toLatin1().constData());	// Node name
	StdfFile.WriteString(m_strTesterID.toLatin1().constData());					// Tester Type
	StdfFile.WriteString(m_strJobName.toLatin1().constData());	// Job name
	StdfFile.WriteString(m_strJobRev.toLatin1().constData());	// Job rev
	StdfFile.WriteString("");					// sublot-id
	StdfFile.WriteString(m_strOperatorID.toLatin1().constData());	// operator
	StdfFile.WriteString("");					// exec-type
	StdfFile.WriteString(m_strSoftVer.toLatin1().constData());// exe-ver
	StdfFile.WriteString("WAFER");				// test-cod
	StdfFile.WriteString("");					// test-temperature
	// Construct custom Galaxy USER_TXT 
	QString	strUserTxt;
	strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;	
	strUserTxt += ":";
	strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
	strUserTxt += ":TESSERA_"+m_strTesterType.toUpper();
	StdfFile.WriteString((char *)strUserTxt.toLatin1().constData());	// user-txt
	StdfFile.WriteString("");							// aux-file
	StdfFile.WriteString("");							// package-type
	StdfFile.WriteString(m_strProductID.toLatin1().constData());	// familyID
	StdfFile.WriteString("");							// Date-code
	StdfFile.WriteString("");							// Facility-ID
	StdfFile.WriteString("");							// FloorID
	StdfFile.WriteString("");							// ProcessID
	StdfFile.WriteString("");							// OperationFreq
	StdfFile.WriteString("");							// Spec-nam
	StdfFile.WriteString("");							// Spec-ver
	StdfFile.WriteString("");							// Flow-id
	StdfFile.WriteString((char *)m_strSetupId.left(255).toLatin1().constData());	// setup_id
	StdfFile.WriteRecord();

	// Write Test results for each line read.
	QString strString;
	char	szString[257];
	QString strSection;
	float	fValue;				// Used for readng floating point numbers.
	int		iIndex;				// Loop index
	int		iXpos, iYpos;
	int		iBin;
	bool	bPassStatus;
	long		iTotalGoodBin,iTotalFailBin;
	long		iTotalTests,iPartNumber;
	bool		bStatus;
	QStringList	lstSections;
	BYTE		bData;
	CGTesseraInspectionBinning* pBinning;
	
	// Reset counters
	iTotalGoodBin=iTotalFailBin=0;
	iPartNumber=0;

	// Write all Parameters read on this wafer.: WIR.PIR,PTR....PTR, PRR, PIR, PTR,PTR...PRR,   ... WRR

	// Write WIR of new Wafer.
	RecordReadInfo.iRecordType = 2;
	RecordReadInfo.iRecordSubType = 10;
	StdfFile.WriteHeader(&RecordReadInfo);
	StdfFile.WriteByte(1);								// Test head
	StdfFile.WriteByte(255);							// Tester site (all)
	StdfFile.WriteDword(m_lStartTime);					// Start time
	StdfFile.WriteString(m_strWaferID.toLatin1().constData());	// WaferID
	StdfFile.WriteRecord();
	
	//Column,Row,Pass,~Bin,~Defect Area Lens (um^2),~Scratch Length Lens (um),~Defect Area Field (um),~Scratch Length Field (um),~Die Size X (um),~Die Size Y (um),~Lens Position X (um),~Lens Position Y (um),~Parent Wafer,~Parent Row,~Parent Column
	//0,0,0,500,,,,,,,,,,,
	//0,1,0,500,,,,,,,,,,,
	while(hTesseraInspectionFile->atEnd() == false)
	{

		// Read line
		strString = ReadLine(*hTesseraInspectionFile);
        lstSections = strString.split(",",QString::KeepEmptyParts);
		// Check if have the good count
		if(lstSections.count() < m_iTotalParameters)
		{
			iLastError = errInvalidFormatLowInRows;
			
			StdfFile.Close();
			// Convertion failed.
			return false;
		}
		
		// Extract Column,Row,Pass
		iXpos = lstSections[0].toInt();
		iYpos = lstSections[1].toInt();
		iBin = lstSections[3].toInt();
		if(!m_mapTesseraInspectionBinning.contains(iBin))
		{
			pBinning = new CGTesseraInspectionBinning();

			pBinning->nNumber = iBin;
			pBinning->strName = "";
			pBinning->bPass = (lstSections[1].toInt() != 0);
			pBinning->nCount = 0;

			m_mapTesseraInspectionBinning[iBin] = pBinning;
		}
		
		pBinning = m_mapTesseraInspectionBinning[iBin];

		if(pBinning->bToIgnore)
		{
			// Unpopulated binning
			pBinning->nCount++;
			continue;
		}
		
		// Part number
		iPartNumber++;

		bPassStatus = pBinning->bPass;
		pBinning->nCount++;


		// Reset counters
		iTotalTests = 0;

		// Write PIR for parts in this Wafer site
		RecordReadInfo.iRecordType = 5;
		RecordReadInfo.iRecordSubType = 10;
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(1);					// Test head
		StdfFile.WriteByte(1);					// Tester site
		StdfFile.WriteRecord();

		// Read Parameter results for this record
		for(iIndex=4;iIndex<m_iTotalParameters;iIndex++)
		{
			strSection = lstSections[iIndex];
			fValue = strSection.toFloat(&bStatus);
			if(bStatus == true)
			{
				// Valid test result...write the PTR
				iTotalTests++;

				RecordReadInfo.iRecordType = 15;
				RecordReadInfo.iRecordSubType = 10;
				StdfFile.WriteHeader(&RecordReadInfo);
				// Compute Test# (add user-defined offset)
				StdfFile.WriteDword(m_pTesseraInspectionParameter[iIndex].nNumber + GEX_TESTNBR_OFFSET_TESSERA);			// Test Number
				StdfFile.WriteByte(1);						// Test head
				StdfFile.WriteByte(1);			// Tester site#
				
				if(!m_pTesseraInspectionParameter[iIndex].bValidLowLimit && !m_pTesseraInspectionParameter[iIndex].bValidHighLimit)
				{
					// No pass/fail information
					bData = 0x40;
				}
				else if(((m_pTesseraInspectionParameter[iIndex].bValidLowLimit==true) && (fValue < m_pTesseraInspectionParameter[iIndex].fLowLimit)) ||
				   ((m_pTesseraInspectionParameter[iIndex].bValidHighLimit==true) && (fValue > m_pTesseraInspectionParameter[iIndex].fHighLimit)))
				{
					bData = 0200;	// Test Failed
				}
				else
				{
					bData = 0;		// Test passed
				}
				StdfFile.WriteByte(bData);							// TEST_FLG
				StdfFile.WriteByte(0x40|0x80);						// PARAM_FLG
				StdfFile.WriteFloat(fValue * GS_POW(10.0,m_pTesseraInspectionParameter[iIndex].nScale));						// Test result
				if(m_pTesseraInspectionParameter[iIndex].bStaticHeaderWritten == false)
				{
					StdfFile.WriteString(m_pTesseraInspectionParameter[iIndex].strName.toLatin1().constData());	// TEST_TXT
					StdfFile.WriteString("");							// ALARM_ID
					bData = 2;	// Valid data.
					if(m_pTesseraInspectionParameter[iIndex].bValidLowLimit==false)
						bData |=0x40;
					if(m_pTesseraInspectionParameter[iIndex].bValidHighLimit==false)
						bData |=0x80;
					StdfFile.WriteByte(bData);							// OPT_FLAG
					StdfFile.WriteByte(-m_pTesseraInspectionParameter[iIndex].nScale);			// RES_SCALE
					StdfFile.WriteByte(-m_pTesseraInspectionParameter[iIndex].nScale);			// LLM_SCALE
					StdfFile.WriteByte(-m_pTesseraInspectionParameter[iIndex].nScale);			// HLM_SCALE
					StdfFile.WriteFloat(m_pTesseraInspectionParameter[iIndex].fLowLimit);		// LOW Limit
					StdfFile.WriteFloat(m_pTesseraInspectionParameter[iIndex].fHighLimit);		// HIGH Limit
					StdfFile.WriteString(m_pTesseraInspectionParameter[iIndex].strUnits.toLatin1().constData());	// Units
					m_pTesseraInspectionParameter[iIndex].bStaticHeaderWritten = true;
				}
				StdfFile.WriteRecord();
			}	// Valid test result
			else
			{
				if(m_pTesseraInspectionParameter[iIndex].bStaticHeaderWritten == false)
				{
					RecordReadInfo.iRecordType = 15;
					RecordReadInfo.iRecordSubType = 10;
					StdfFile.WriteHeader(&RecordReadInfo);
					// Compute Test# (add user-defined offset)
					StdfFile.WriteDword(m_pTesseraInspectionParameter[iIndex].nNumber + GEX_TESTNBR_OFFSET_TESSERA);			// Test Number
					StdfFile.WriteByte(1);						// Test head
					StdfFile.WriteByte(1);			// Tester site#
					
					// No pass/fail information
					bData = 0x52;
					StdfFile.WriteByte(bData);							// TEST_FLG
					StdfFile.WriteByte(0x40|0x80);						// PARAM_FLG
					StdfFile.WriteFloat(0);								// Test result
					StdfFile.WriteString(m_pTesseraInspectionParameter[iIndex].strName.toLatin1().constData());	// TEST_TXT
					StdfFile.WriteString("");							// ALARM_ID
					bData = 2;	// Valid data.
					if(m_pTesseraInspectionParameter[iIndex].bValidLowLimit==false)
						bData |=0x40;
					if(m_pTesseraInspectionParameter[iIndex].bValidHighLimit==false)
						bData |=0x80;
					StdfFile.WriteByte(bData);							// OPT_FLAG
					StdfFile.WriteByte(-m_pTesseraInspectionParameter[iIndex].nScale);			// RES_SCALE
					StdfFile.WriteByte(-m_pTesseraInspectionParameter[iIndex].nScale);			// LLM_SCALE
					StdfFile.WriteByte(-m_pTesseraInspectionParameter[iIndex].nScale);			// HLM_SCALE
					StdfFile.WriteFloat(m_pTesseraInspectionParameter[iIndex].fLowLimit);		// LOW Limit
					StdfFile.WriteFloat(m_pTesseraInspectionParameter[iIndex].fHighLimit);		// HIGH Limit
					StdfFile.WriteString(m_pTesseraInspectionParameter[iIndex].strUnits.toLatin1().constData());	// Units
					m_pTesseraInspectionParameter[iIndex].bStaticHeaderWritten = true;
					StdfFile.WriteRecord();

				}
			}
		}		// Read all results on line

		// Write PRR
		RecordReadInfo.iRecordType = 5;
		RecordReadInfo.iRecordSubType = 20;
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(1);			// Test head
		StdfFile.WriteByte(1);// Tester site#:1
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
		StdfFile.WriteDword(0);					// No testing time known...
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


	QMap<int,CGTesseraInspectionBinning *>::Iterator it;

	RecordReadInfo.iRecordType = 1;
	RecordReadInfo.iRecordSubType = 40;
	for(it = m_mapTesseraInspectionBinning.begin(); it != m_mapTesseraInspectionBinning.end(); it++)
	{
		pBinning = *it;

		if(pBinning->bToIgnore)
			continue;

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
	for(it = m_mapTesseraInspectionBinning.begin(); it != m_mapTesseraInspectionBinning.end(); it++)
	{
		pBinning = *it;

		if(pBinning->bToIgnore)
			continue;

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
// Convert 'FileName' TesseraInspection file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGTesseraInspectiontoSTDF::Convert(const char *TesseraInspectionFileName, const char *strFileNameSTDF)
{
	// No erro (default)
	iLastError = errNoError;

	// If STDF file already exists...do not rebuild it...unless dates not matching!
	QFileInfo fInput(TesseraInspectionFileName);
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
			GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(TesseraInspectionFileName).fileName()+"...");
			GexScriptStatusLabel->show();
		}
		GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(TesseraInspectionFileName).fileName()+"...");
		GexScriptStatusLabel->show();
	}
    QCoreApplication::processEvents();

    if(ReadTesseraInspectionFile(TesseraInspectionFileName,strFileNameSTDF) != true)
	{
		//////////////////////////////////////////////////////////////////////
		// For ProgressBar
		if((GexProgressBar != NULL)
		&& bHideProgressAfter)
			GexProgressBar->hide();
		
		if((GexScriptStatusLabel != NULL)
		&& bHideLabelAfter)
			GexScriptStatusLabel->hide();
		return false;	// Error reading TesseraInspection file
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
void CGTesseraInspectiontoSTDF::NormalizeLimits(QString &strUnit, int &nScale)
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
QString CGTesseraInspectiontoSTDF::ReadLine(QTextStream& hFile)
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
