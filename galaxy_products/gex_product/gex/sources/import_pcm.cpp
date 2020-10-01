//////////////////////////////////////////////////////////////////////
// import_pcm.cpp: Convert a PCM (Process control Monitor).PCM (UMC)
// file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#include "windows.h"		// For 'GetWindowsDirectoryA' declaration
#endif

#include <qfileinfo.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>

#include "engine.h"
#include "import_pcm.h"
#include "time.h"
#include "import_constants.h"

// File format: (CSV delimited)
// TYPE NO:JUPITER A3,PCM SPEC:HAJ02A_P4,LOT ID:D0FAA.1,DATE:2003-04-07 02:37:11
// ITEM :44,TOTAL:25 PCS,PASS:25 PCS,TRANSFER:25 PCS
// LOT,WF#,S#,BVCOXP1P0,BVDDN10/.5,BVDDP10/.5 ... Parameter names
// D0FAA.1,01,R,12.6,9.9,9.6,
// D0FAA.1,01,L,12.6,9.9,9.6,
// D0FAA.1,01,B,12.6,9.9,9.6,
// D0FAA.1,01,T,12.6,9.9,9.6,
// D0FAA.1,01,C,12.6,9.9,9.6,
// .....
// <MAX>,,,14.4,1
// <MIN>,,,12.4,9.9,
// <AVG>,,,13.072,9.92,
// <STD>,,,0.560040
// <SPEC HIGH>,,,NONE,NONE,NONE,.84,.67
// <SPEC LOW>,,,7,7,NONE,9,9

// File format from Lantiq
//TYPE NO:M4762A00011,PCM SPEC:C65LP-M4762A00011U06-V1E,LOT ID:19G52001,DATE:2011-03-11
//CUST_PRODUCT:M4762A00015
//ITEM:,TOTAL:,PASS:,TRANSFER:
//LOT,WF#,BKID,S#,BEOL_LB_RS/UM,BEOL_M12M12_SHORT,...
//19G52001,1,PFRRS037MXG5,P8-19,.010872,.000000000...
//19G52001,1,PFRRS037MXG5,P12-9,.010941,.000000000...


//TYPE NO:R164A-99,PCM SPEC:R164IY-2,LOT ID:UMSQQS5,DATE:2011-05-05
//CUST_PRODUCT:M4606M00031C11N_MQ
//ITEM:295,TOTAL:25,PASS:25,TRANSFER:25
//LOT,WF#,S#,BETA_PNP_IB_1IE_IFX,BREAK_NN_FW_IFX,BR...
//UMSQQS5,01,P7-2,1.5582,10.54,9.104,1.06338,1.3159...
//UMSQQS5,01,P7-4,1.54809,10.564,9.105,1.05107,1.29...

// File format from case 5388
//#NAME?,PCM SPEC:C65LP-M4762A00011U06-V1E,LOT ID:19H04031,DATE:2011-04-05,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
//#NAME?,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
//-<SPEC HIGH>,,,,,,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,216,1224,NONE,NONE,146.4,912,0.489,0.371,NONE,NONE,2.30E-15,1.69E-15,1.14E-15
//-<SPEC LOW>,,,,,,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,181.2,968,NONE,NONE,120,720,0.43,0.301,NONE,NONE,1.70E-15,1.23E-15,8.08E-16
//-ITEM:,TOTAL:,PASS:,TRANSFER:,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
//#NAME?,WF#,BKID,S#,SiteX,SiteY,EFUSE_DG_PRE_ION,EFUSE_POST_RES,EFUSE_PRE_RES,HVT_RINGO_IFX_0P8V_FREQ,HVT_RINGO_IFX_0P8V_IDDA,HVT_RINGO_IFX_0P8V_IDDQ,HVT_RINGO_IFX_1P1V_FREQ,HVT_RINGO_IFX_1P1V_IDDA,HVT_RINGO_IFX_1P1V_IDDQ,HVT_RINGO_IFX_1V_FREQ,HVT_RINGO_IFX_1V_IDDA,HVT_RINGO_IFX_1V_IDDQ,RVT_RINGO_IFX_0P8V_FREQ,RVT_RINGO_IFX_0P8V_IDDA,RVT_RINGO_IFX_0P8V_IDDQ,RVT_RINGO_IFX_1P1V_FREQ,RVT_RINGO_IFX_1P1V_IDDA,RVT_RINGO_IFX_1P1V_IDDQ,RVT_RINGO_IFX_1V_FREQ,RVT_RINGO_IFX_1V_IDDA,RVT_RINGO_IFX_1V_IDDQ,RVT_RINGO_IFX_N_IDLIN,RVT_RINGO_IFX_N_ION,RVT_RINGO_IFX_N_VTLIN,RVT_RINGO_IFX_N_VTSAT,RVT_RINGO_IFX_P_IDLIN,RVT_RINGO_IFX_P_ION,RVT_RINGO_IFX_P_VTLIN,RVT_RINGO_IFX_P_VTSAT,VPNP_IFX_BETA_0P7V,VPNP_IFX_VBE_10UA,VPP_IFXRFM1M12_P11P1_CAP,VPP_IFXRFM2M12_P11P1_CAP,VPP_IFXRFM3M12_P11P1_CAP
//19H04031,1,PFSSS123MXD7,P8-19,8,19,18625,102300,130.01,21879,0.000077945,1.24E-07,57300,0.00027785,4.35E-07,45707,0.00020335,2.96E-07,35818,0.0001288,2.39E-07,75015,0.0003595,7.43E-07,62748,0.0002732,5.20E-07,120.6,1295.5,0.4657,0.3006,73.15,845.45,0.4491,0.3022,1.2567,0.7691,1.10E-15,1.34E-15,9.56E-16


// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGPCMtoSTDF::CGPCMtoSTDF()
{
	// Default: PCM parameter list on disk includes all known PCM parameters...
	m_bNewPcmParameterFound = false;
	m_lStartTime = 0;
	m_iParametersOffset = 3;
	m_iSitePos = 2;
	m_iBackSidePos = -1;

	m_pCGPcmParameter = NULL;
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CGPCMtoSTDF::~CGPCMtoSTDF()
{
	// Destroy list of Parameters tables.
	if(m_pCGPcmParameter!=NULL)
		delete [] m_pCGPcmParameter;
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGPCMtoSTDF::GetLastError()
{
	m_strLastError = "Import PCM: ";

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
		case errNoLimitsFound:
			m_strLastError += "Invalid file format: Specification Limits not found";
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
// Load PCM Parameter table from DISK
//////////////////////////////////////////////////////////////////////
void CGPCMtoSTDF::LoadParameterIndexTable(void)
{
	QString	strPcmTableFile;
	QString	strString;

    strPcmTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strPcmTableFile += GEX_PCM_PARAMETERS;

	// Open PCM Parameter table file
    QFile f( strPcmTableFile );
    if(!f.open( QIODevice::ReadOnly ))
		return;

	// Assign file I/O stream
	QTextStream hPcmTableFile(&f);

	// Skip comment lines
	do
	{
	  strString = hPcmTableFile.readLine();
	}
    while((strString.indexOf("----------------------") < 0) && (hPcmTableFile.atEnd() == false));

	// Read lines
	m_pFullPcmParametersList.clear();
	strString = hPcmTableFile.readLine();
	while (strString.isNull() == false)
	{
		// Save Parameter name in list
		m_pFullPcmParametersList.append(strString);
		// Read next line
		strString = hPcmTableFile.readLine();
	};

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// Save PCM Parameter table to DISK
//////////////////////////////////////////////////////////////////////
void CGPCMtoSTDF::DumpParameterIndexTable(void)
{
	QString		strPcmTableFile;

    strPcmTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strPcmTableFile += GEX_PCM_PARAMETERS;

	// Open PCM Parameter table file
    QFile f( strPcmTableFile );
    if(!f.open( QIODevice::WriteOnly ))
		return;

	// Assign file I/O stream
	QTextStream hPcmTableFile(&f);

	// First few lines are comments:
	hPcmTableFile << "############################################################" << endl;
	hPcmTableFile << "# DO NOT EDIT THIS FILE!" << endl;
    hPcmTableFile << "# Quantix Examinator: PCM Parameters detected" << endl;
	hPcmTableFile << "# www.mentor.com" << endl;
    hPcmTableFile << "# Quantix Examinator reads and writes into this file..." << endl;
	hPcmTableFile << "-----------------------------------------------------------" << endl;

	// Write lines
	// m_pFullPcmParametersList.sort();
	for (QStringList::const_iterator
		 iter  = m_pFullPcmParametersList.begin();
		 iter != m_pFullPcmParametersList.end(); ++iter) {
		// Write line
		hPcmTableFile << *iter << endl;
	}

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// If Examinator doesn't have this PCM parameter in his dictionnary, have it added.
//////////////////////////////////////////////////////////////////////
void CGPCMtoSTDF::UpdateParameterIndexTable(QString strParamName)
{
	// Check if the table is empty...if so, load it from disk first!
	if(m_pFullPcmParametersList.isEmpty() == true)
	{
		// Load PCM parameter table from disk...
		LoadParameterIndexTable();
	}
	
	// Check if Parameter name already in table...if not, add it to the list
	// the new full list will be dumped to the disk at the end.
    if(m_pFullPcmParametersList.indexOf(strParamName) < 0)
	{
		// Update list
		m_pFullPcmParametersList.append(strParamName);

		// Set flag to force the current PCM table to be updated on disk
		m_bNewPcmParameterFound = true;
	}
}


//////////////////////////////////////////////////////////////////////
// Check if File is compatible with PCM format
//////////////////////////////////////////////////////////////////////
bool CGPCMtoSTDF::IsCompatible(const char *szFileName)
{
	QString strString;
	QString strSection;


	// Open hCsmFile file
    QFile f( szFileName );
    if(!f.open( QIODevice::ReadOnly ))
	{
		// Failed Opening ASL1000 file
		return false;
	}
	// Assign file I/O stream
	QTextStream hPcmFile(&f);

	// Check if first line is the correct PCM header...
	do
		strString = hPcmFile.readLine();
	while(!strString.isNull() && strString.isEmpty());
	
	strString = strString.trimmed();	// remove leading spaces.
	strSection = strString.section(',',0,0);
	if(strSection.startsWith("TYPE NO:") == false)
	{
		// case 5388
		// Accept any first keyword

		// Incorrect header...this is not a PCM file!
		// Close file
		// f.close();
		// return false;
	}

	strSection = strString.section(',',1,1);
	if(strSection.startsWith("PCM SPEC:") == false)
	{
		// Incorrect header...this is not a PCM file!
		// Close file
		f.close();
		return false;
	}
	strSection = strString.section(',',2,2);
	if(strSection.startsWith("LOT ID:") == false)
	{
		// Incorrect header...this is not a PCM file!
		// Close file
		f.close();
		return false;
	}
	
	// Check if found the header parameters line
	// LOT,WF#,BKID,S#,BEOL_LB_RS/UM
	// LOT,WF#,S#,BETA_PNP_IB_1IE_IFX
	int nLine = 7;
	while(!strString.contains(",WF#,",Qt::CaseInsensitive))
	{
		if((nLine <= 0) || hPcmFile.atEnd())
			break;
		strString = hPcmFile.readLine().simplified().remove(" ");
		nLine--;
	}

	// Close file
	f.close();

	if(strString.startsWith("LOT,WF#,BKID,S#,",Qt::CaseInsensitive))
		return true;

	if(strString.startsWith("LOT,WF#,S#,",Qt::CaseInsensitive))
		return true;

	if(strString.contains(",WF#,BKID,S#,",Qt::CaseInsensitive))
		return true;

	if(strString.contains(",WF#,S#,",Qt::CaseInsensitive))
		return true;

	return false;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the PCM file
//////////////////////////////////////////////////////////////////////
bool CGPCMtoSTDF::ReadPcmFile(const char *PcmFileName,const char *strFileNameSTDF)
{
	QString strString;
	QString strSection;
	bool	bStatus;
	int		iIndex;				// Loop index

	// Open CSV file
    QFile f( PcmFileName );
    if(!f.open( QIODevice::ReadOnly ))
	{
		// Failed Opening PCM file
		m_iLastError = errOpenFail;
		
		// Convertion failed.
		return false;
	}

 	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	iNextFilePos = 0;
	iProgressStep = 0;
	iFileSize = f.size() + 1;
	

   QFile f_limits( PcmFileName );
    if(!f_limits.open( QIODevice::ReadOnly ))
	{
		// Failed Opening PCM file
		m_iLastError = errOpenFail;
		
		// Convertion failed.
		// Close file
		f.close();
		return false;
	}
	// Assign file I/O stream
	QTextStream hPcmFile(&f);
	QTextStream hPcmFileLimits(&f_limits);	// Handle to scan file only to read its limits (located at the end)

	// Check if first line is the correct PCM header...
	strString = ReadLine(hPcmFile);
	strString = strString.trimmed();	// remove leading spaces.
	strSection = strString.section(',',0,0);
	if(strSection.startsWith("TYPE NO:") == false)
	{
		// case 5388
		//  Accept any first keyword

		// Incorrect header...this is not a PCM file!
		// m_iLastError = errInvalidFormat;
		
		// Convertion failed.
		// Close file
		// f_limits.close();
		// f.close();
		// return false;
	}
	strSection = strString.section(',',1,1);
	if(strSection.startsWith("PCM SPEC:") == false)
	{
		// Incorrect header...this is not a PCM file!
		m_iLastError = errInvalidFormat;
		
		// Convertion failed.
		// Close file
		f_limits.close();
		f.close();
		return false;
	}
	strSection = strString.section(',',2,2);
	if(strSection.startsWith("LOT ID:") == false)
	{
		// Incorrect header...this is not a PCM file!
		m_iLastError = errInvalidFormat;
		
		// Convertion failed.
		// Close file
		f_limits.close();
		f.close();
		return false;
	}
	
	// Extract ProductID
	strSection = strString.section(',',0,0);
	m_strProductID = strSection.section(':',1,1);
	m_strProductID = m_strProductID.trimmed();	// remove leading spaces.
	
	// Extract ProcessID
	strSection = strString.section(',',1,1);
	m_strProcessID = strSection.section(':',1,1);
	m_strProcessID = m_strProcessID.trimmed();	// remove leading spaces.
	
	// Extract LotID
	strSection = strString.section(',',2,2);
	m_strLotID = strSection.section(':',1,1);
	m_strLotID = m_strLotID.trimmed();	// remove leading spaces.

	strSection = strString.section(',',3,3);
	if(strSection.startsWith("DATE:") == false)
	{
		// Incorrect header...this is not a PCM file!
		m_iLastError = errInvalidFormat;
		
		// Convertion failed.
		// Close file
		f_limits.close();
		f.close();
		return false;
	}
	strSection = strSection.section(':',1); // Date format: YYYY-MM-DD HH:MM:SS
	// Extract TimeStamp, convert to UINT (nbr of seconds since 1-1-1970)
	int iDay,iMonth,iYear; 
	int	iHours,iMinutes,iSeconds;
	QString	strDate,strTime;
	strDate = strSection.section(' ',0,0);
	strTime = strSection.section(' ',1,1);
	strSection = strDate.section('-',0,0);	// First field is Year
	iYear = strSection.toInt();
	strSection = strDate.section('-',1,1);	// Second field is Month
	iMonth = strSection.toInt();
	strSection = strDate.section('-',2,2);	// Third field is Day
	iDay = strSection.toInt();

	strSection = strTime.section(':',0,0);	// First field is Hours
	iHours = strSection.toInt();
	strSection = strTime.section(':',1,1);	// Second field is Minutes
	iMinutes = strSection.toInt();
	strSection = strTime.section(':',2,2);	// Third field is Seconds
	iSeconds = strSection.toInt();

	QDate PcmDate(iYear,iMonth,iDay);
	QTime PcmTime(iHours,iMinutes,iSeconds);
	QDateTime PcmeDateTime(PcmDate);
	PcmeDateTime.setTimeSpec(Qt::UTC);
	PcmeDateTime.setTime(PcmTime);
	m_lStartTime = PcmeDateTime.toTime_t();

	// Goto line: LOT,WF#,S#,BVCOXP1P0,BVDDN10/.5,BVDDP10/.5 ... Parameter names
	int nLine = 7;
	while(!strString.contains(",WF#,",Qt::CaseInsensitive))
	{
		if((nLine <= 0) || hPcmFile.atEnd())
		{
			// Incorrect header...this is not a valid PCM file!
			m_iLastError = errInvalidFormat;

			// Convertion failed.
			// Close file
			f_limits.close();
			f.close();
			return false;
		}

		strString = ReadLine(hPcmFile);
		nLine--;
	}

	// Check the current format
	m_iParametersOffset = 3;
	m_iSitePos = 2;
	m_iBackSidePos = -1;
	if(strString.contains(",WF#,BKID,S#,",Qt::CaseInsensitive))
	{
		m_iParametersOffset = 4;
		m_iSitePos = 3;
		m_iBackSidePos = 2;
	}

	// Ignore SiteX,SiteY
	if(strString.contains(",S#,SiteX,SiteY",Qt::CaseInsensitive))
		m_iParametersOffset+=2;

	// Count the number of parameters specified in the line
	m_iTotalParameters=strString.count(",");

	// Do not count first N fields that are LotID and WaferID.
	m_iTotalParameters -= m_iParametersOffset;

	// If no parameter specified...ignore!
	if(m_iTotalParameters <= 0)
	{
		// Incorrect header...this is not a valid PCM file!
		m_iLastError = errInvalidFormatLowInRows;

		// Convertion failed.
		// Close file
		f_limits.close();
		f.close();
		return false;
	}

	m_iTotalParameters++;
	// Allocate the buffer to hold the N parameters & results.
	m_pCGPcmParameter = new CGPcmParameter[m_iTotalParameters];	// List of parameters

	// Extract the N column names
	for(iIndex=0;iIndex<m_iTotalParameters;iIndex++)
	{
		strSection = strString.section(',',iIndex+m_iParametersOffset,iIndex+m_iParametersOffset);
		strSection = strSection.trimmed();	// Remove spaces
		m_pCGPcmParameter[iIndex].strName = strSection;
		UpdateParameterIndexTable(strSection);		// Update Parameter master list if needed.
		m_pCGPcmParameter[iIndex].bStaticHeaderWritten = false;
	}

	// With 2nd file handle: seek to the end of the file to read the Spec Limits.
	int	iLimits =0;
	while(hPcmFileLimits.atEnd() == false)
	{
		strString = ReadLine(hPcmFileLimits);
		if(strString.contains("<SPEC HIGH>") == true)
		{
			// found the HIGH limits
			iLimits |= 1;

			// Extract the N column Upper Limits
			for(iIndex=0;iIndex<m_iTotalParameters;iIndex++)
			{
				strSection = strString.section(',',iIndex+m_iParametersOffset,iIndex+m_iParametersOffset);
				m_pCGPcmParameter[iIndex].fHighLimit = strSection.toFloat(&bStatus);
				m_pCGPcmParameter[iIndex].bValidHighLimit = bStatus;
			}
		}
		else
		if(strString.contains("<SPEC LOW>") == true)
		{
			// found the Low limits
			iLimits |= 2;

			// Extract the N column Lower Limits
			for(iIndex=0;iIndex<m_iTotalParameters;iIndex++)
			{
				strSection = strString.section(',',iIndex+m_iParametersOffset,iIndex+m_iParametersOffset);
				m_pCGPcmParameter[iIndex].fLowLimit = strSection.toFloat(&bStatus);
				m_pCGPcmParameter[iIndex].bValidLowLimit = bStatus;
			}
		}
	};
	if(iLimits != 3)
	{
		// Incorrect header...this is not a valid PCM file!: we didn't find the limits!
		m_iLastError = errNoLimitsFound;

		// Convertion failed.
		bStatus = false;
	}
	else
	{
		// Loop reading file until end is reached & generate STDF file dynamically.
		bStatus = WriteStdfFile(&hPcmFile,strFileNameSTDF);
		if(!bStatus)
			QFile::remove(strFileNameSTDF);
	}

	// Close file
	f_limits.close();
	f.close();
	
	// All PCM file read...check if need to update the PCM Parameter list on disk?
	if(bStatus && (m_bNewPcmParameterFound == true))
		DumpParameterIndexTable();

	// Success parsing PCM file
	return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from PCM data parsed
//////////////////////////////////////////////////////////////////////
bool CGPCMtoSTDF::WriteStdfFile(QTextStream *hPcmFile, const char *strFileNameSTDF)
{
	// now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open((char*)strFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
	{
		// Failed importing CSV file into STDF database
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
	strUserTxt += ":UMC";
	StdfFile.WriteString(strUserTxt.toLatin1().constData());	// user-txt
	StdfFile.WriteString("");							// aux-file
	StdfFile.WriteString("");							// package-type
	StdfFile.WriteString(m_strProductID.toLatin1().constData());// familyID
	StdfFile.WriteString("");							// Date-code
	StdfFile.WriteString("");							// Facility-ID
	StdfFile.WriteString("");							// FloorID
	StdfFile.WriteString(m_strProcessID.toLatin1().constData());// ProcessID	StdfFile.WriteRecord();
	StdfFile.WriteRecord();

	// Write Test results for each line read.
	QString strString;
	char	szString[257];
	QString strSection;
	QString	strWaferID;
	float	fValue;				// Used for readng floating point numbers.
	int		iIndex;				// Loop index
	int		iSiteNumber;
	int		iPosX;
	int		iPosY;
	BYTE		bData;
	WORD		wSoftBin,wHardBin;
	long		iTotalGoodBin,iTotalFailBin;
	long		iTestNumber,iTotalTests,iPartNumber;
	bool		bStatus,bPassStatus;
	QStringList	lstSections;
	// Reset counters
	iTotalGoodBin=iTotalFailBin=0;
	iPartNumber=0;

	QString strBackSideID;

	// Write all Parameters read on this wafer.: WIR.PIR,PTR....PTR, PRR, PIR, PTR,PTR...PRR,   ... WRR
	while(hPcmFile->atEnd() == false)
	{

		// Read line
		strString = ReadLine(*hPcmFile);
        lstSections = strString.split(",",QString::KeepEmptyParts);
		
		if(strString.startsWith("<"))
			continue;	// Reach the end of valid data records...
		if(strString.startsWith("<SPEC HIGH>"))
			break;	// Reach the end of valid data records...
		if(strString.startsWith("<SPEC LOW>"))
			break;	// Reach the end of valid data records...

		// Part number
		iPartNumber++;

		// Check if have the good count
		if(lstSections.count() < m_iTotalParameters+m_iParametersOffset)
		{
			m_iLastError = errInvalidFormatLowInRows;
			StdfFile.Close();
			// Convertion failed.
			return false;
		}
		
		// Extract WaferID
		strSection = lstSections[1];
		if(strSection != strWaferID)
		{
			// Write WRR in case we have finished to write wafer records.
			if(strWaferID.isEmpty() == false)
			{
				// WRR
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
				StdfFile.WriteString(strWaferID.toLatin1().constData());	// WaferID
				if(!strBackSideID.isEmpty())
					StdfFile.WriteString(strBackSideID.toLatin1().constData());	// Fab wafer ID

				StdfFile.WriteRecord();

			}

			iTotalGoodBin=iTotalFailBin=0;
			strBackSideID = "";

			// For each wafer, have to write limit in the first PTR
			for(iIndex=0;iIndex<m_iTotalParameters;iIndex++)
				m_pCGPcmParameter[iIndex].bStaticHeaderWritten = false;

			// Write WIR of new Wafer.
			strWaferID = strSection;
			RecordReadInfo.iRecordType = 2;
			RecordReadInfo.iRecordSubType = 10;
			StdfFile.WriteHeader(&RecordReadInfo);
			StdfFile.WriteByte(1);								// Test head
			StdfFile.WriteByte(255);							// Tester site (all)
			StdfFile.WriteDword(m_lStartTime);					// Start time
			StdfFile.WriteString(strWaferID.toLatin1().constData());	// WaferID
			StdfFile.WriteRecord();
		}

		// Reset Pass/Fail flag.
		bPassStatus = true;

		// Reset counters
		iTotalTests = 0;

		// Extract the BackSideId if any
		if(m_iBackSidePos > -1)
			strBackSideID = lstSections[m_iBackSidePos];

		// Extract Site
		strSection = lstSections[m_iSitePos];
		strSection = strSection.toUpper();
		if(strSection.startsWith("P"))
		{
			// Have pos X Y
			iSiteNumber = 1;
			iPosX = strSection.section("P",1).section("-",0,0).toInt();
			iPosY = strSection.section("P",1).section("-",1).toInt();
		}
		else
		{
			iPosX = iPosY = -32768;
			if(strSection == "C")
				iSiteNumber = 1;	// Center
			else
			if(strSection == "B")
				iSiteNumber = 2;	// Bottom (Down)
			else
			if(strSection == "L")
				iSiteNumber = 3;	// Left
			else
			if(strSection == "T")
				iSiteNumber = 4;	// Top
			else
			if(strSection == "R")
				iSiteNumber = 5;	// Right
			else
				iSiteNumber = 6;	// Other site...

			switch(iSiteNumber)
			{
				case 1:	// Center
					iPosX = 1;
					iPosY = 1;
					break;
				case 2:	// Down
					iPosX = 1;
					iPosY = 2;
					break;
				case 3:	// Left
					iPosX = 0;
					iPosY = 1;
					break;
				case 4:	// Top
					iPosX = 1;
					iPosY = 0;
					break;
				case 5:	// Right
					iPosX = 2;
					iPosY = 1;
					break;
				default: // More than 5 sites?....give 0,0 coordonates
					iPosX = 0;
					iPosY = 0;
					break;
			}
		}

		// Write PIR for parts in this Wafer site
		RecordReadInfo.iRecordType = 5;
		RecordReadInfo.iRecordSubType = 10;
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(1);								// Test head
		StdfFile.WriteByte(iSiteNumber);					// Tester site
		StdfFile.WriteRecord();

		// Read Parameter results for this record
		for(iIndex=0;iIndex<m_iTotalParameters;iIndex++)
		{
			strSection = lstSections[iIndex+m_iParametersOffset];
			fValue = strSection.toFloat(&bStatus);
			if(bStatus == true)
			{
				// Valid test result...write the PTR
				iTotalTests++;

				RecordReadInfo.iRecordType = 15;
				RecordReadInfo.iRecordSubType = 10;
				StdfFile.WriteHeader(&RecordReadInfo);
				// Compute Test# (add user-defined offset)
                iTestNumber = (long) m_pFullPcmParametersList.indexOf(m_pCGPcmParameter[iIndex].strName);
				iTestNumber += GEX_TESTNBR_OFFSET_PCM;		// Test# offset
				StdfFile.WriteDword(iTestNumber);			// Test Number
				StdfFile.WriteByte(1);						// Test head
				StdfFile.WriteByte(iSiteNumber);			// Tester site#
				if(((m_pCGPcmParameter[iIndex].bValidLowLimit==true) && (fValue < m_pCGPcmParameter[iIndex].fLowLimit)) ||
				   ((m_pCGPcmParameter[iIndex].bValidHighLimit==true) && (fValue > m_pCGPcmParameter[iIndex].fHighLimit)))
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
				StdfFile.WriteFloat(fValue);						// Test result
				if(m_pCGPcmParameter[iIndex].bStaticHeaderWritten == false)
				{
					StdfFile.WriteString(m_pCGPcmParameter[iIndex].strName.toLatin1().constData());	// TEST_TXT
					StdfFile.WriteString("");							// ALARM_ID
					bData = 2;	// Valid data.
					if(m_pCGPcmParameter[iIndex].bValidLowLimit==false)
						bData |=0x40;
					if(m_pCGPcmParameter[iIndex].bValidHighLimit==false)
						bData |=0x80;
					StdfFile.WriteByte(bData);							// OPT_FLAG
					StdfFile.WriteByte(0);								// RES_SCALE
					StdfFile.WriteByte(0);								// LLM_SCALE
					StdfFile.WriteByte(0);								// HLM_SCALE
					StdfFile.WriteFloat(m_pCGPcmParameter[iIndex].fLowLimit);			// LOW Limit
					StdfFile.WriteFloat(m_pCGPcmParameter[iIndex].fHighLimit);		// HIGH Limit
					StdfFile.WriteString("");							// No Units
					m_pCGPcmParameter[iIndex].bStaticHeaderWritten = true;
				}
				StdfFile.WriteRecord();
			}	// Valid test result
		}		// Read all results on line

		// Write PRR
		RecordReadInfo.iRecordType = 5;
		RecordReadInfo.iRecordSubType = 20;
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(1);			// Test head
		StdfFile.WriteByte(iSiteNumber);// Tester site#:1
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
		StdfFile.WriteWord((WORD)iTotalTests);	// NUM_TEST
		StdfFile.WriteWord(wHardBin);           // HARD_BIN
		StdfFile.WriteWord(wSoftBin);           // SOFT_BIN
		StdfFile.WriteWord(iPosX);				// X_COORD
		StdfFile.WriteWord(iPosY);				// Y_COORD
		StdfFile.WriteDword(0);				// No testing time known...
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
	StdfFile.WriteDword(m_lStartTime);			// Time of last part tested
	StdfFile.WriteDword(iTotalGoodBin+iTotalFailBin);	// Parts tested: always 5
	StdfFile.WriteDword(0);						// Parts retested
	StdfFile.WriteDword(0);						// Parts Aborted
	StdfFile.WriteDword(iTotalGoodBin);			// Good Parts
	StdfFile.WriteDword((DWORD)-1);				// Functionnal Parts
	StdfFile.WriteString(strWaferID.toLatin1().constData());	// WaferID
	if(!strBackSideID.isEmpty())
		StdfFile.WriteString(strBackSideID.toLatin1().constData());	// Fab wafer ID
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
// Convert 'FileName' PCM file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGPCMtoSTDF::Convert(const char *PcmFileName, const char *strFileNameSTDF)
{
	// No erro (default)
	m_iLastError = errNoError;

	// If STDF file already exists...do not rebuild it...unless dates not matching!
	QFileInfo fInput(PcmFileName);
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
			GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(PcmFileName).fileName()+"...");
			GexScriptStatusLabel->show();
		}
	}
    QCoreApplication::processEvents();

    if(ReadPcmFile(PcmFileName,strFileNameSTDF) != true)
	{
		//////////////////////////////////////////////////////////////////////
		// For ProgressBar
		if((GexProgressBar != NULL)
		&& bHideProgressAfter)
			GexProgressBar->hide();
		
		if((GexScriptStatusLabel != NULL)
		&& bHideLabelAfter)
			GexScriptStatusLabel->hide();
		return false;	// Error reading PCM file
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
QString CGPCMtoSTDF::ReadLine(QTextStream& hFile)
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
