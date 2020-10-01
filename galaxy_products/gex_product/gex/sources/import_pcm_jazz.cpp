//////////////////////////////////////////////////////////////////////
// import_spd.cpp: Convert a .SPD / TMT Credence file to STDF V4.0
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
#include "import_pcm_jazz.h"
#include "import_constants.h"

// File format:
//										 Lot Summary Report KDF V1.1											Page: 1
// PartID=XC3028	LotID=K41911.1	TestProgramName=SBC18HK_MP053PM_REV01	TesterId=4AGT02	Date=07/21/2005	Time=22:31:37	DCOP=4WN86.01	JazzPartID=A0325
// TestName		VT	IDSAT	VT	IDSAT	CR	CR	RHO	RHO	RHO	RHO	RHO	RHO	RHO	RHO	BVDSS	BVDSS	VFI	TDDB	VT	IDSAT	VT	IDSAT	RHO	RHO	CR	CR	CR	CR	CR	CR	CR	BVDSS	BVDSS	VFI	TDDB	BVCEO	BVCBO	BETA	IC	BVCEO	BVCBO	CR	CR	CR	RHO	CA	BVGO	RHO
// DeviceName		10XP18N	10XP18N	10XP36N_HV	10XP36N_HV	NDIFFUSION	NPOLY	NDIFFUSION	GATEPOLY	M1	M2	M3	M4	M5	M6	10XP18N	10XP36N	POLYN	GATEOX_HV	10XP18P	10XP18P	10XP30P_HV	10XP30P_HV	PDIFFUSION	NWELL	PDIFFUSION	PPOLY	VIA1	VIA2	VIA3	VIA4	VIA5	10XP18P	10XP30P	POLYP	GATEOX_LV	P2X10STD	P2X10STD	P2X10HV_M	P2X10HV	P2X10HV	P2X10HV	EP	BPOLY	SINKER	TR	MIM@0V	MIM	RESISTORLV
// UpperLimit		0.6	0.0069	0.72	0.007	40	25	12	12	0.095	0.095	0.095	0.086	0.022	0.012	8	12	15	-8	-0.36	-0.0021	-0.64	-0.0024	12	1070	40	25	20	20	20	8	8	8	10	15	-4	4.2	13.5	300	0.0000075	7.5	16	18	100	20	28	69	38	295
// LowerLimit		0.44	0.0051	0.52	0.005	4	2	2	2	0.069	0.069	0.069	0.046	0.014	0.009	3.2	6	4	-12	-0.52	-0.003	-0.84	-0.0034	2	710	4	0.1	0.1	0.1	0.1	0.1	0.1	3.2	5	4	-8	2.8	11.5	60	0.0000021	4.5	12	2	4	1	21	51	18	175
//
// Wafer#	Die#	Volt	Amp	Volt	Amp	Ohm	Ohm	Ohm/sq	Ohm/sq	Ohm/sq	Ohm/sq	Ohm/sq	Ohm/sq	Ohm/sq	Ohm/sq	Volt	Volt	Volt	Volt	Volt	Amp	Volt	Amp	Ohm	Ohm	Ohm	Ohm	Ohm	Ohm	Ohm	Ohm	Ohm	Volt	Volt	Volt	Volt	Volt	Volt	NONE	Amp	Volt	Volt	Ohm	Ohm	Ohm	Ohm/sq	pFarad	Volt	Ohm/sq
// 1	1	0.5423992	0.00579451	0.5900622	0.00609197	18.31684	9.329741	7.522559	4.710599	0.08290598	0.08208377	0.07714392	0.06959498	0.01878117	0.009140159	4.8	8.7	5.55	-9.3	-0.4326263	-0.00247116	-0.7043257	-0.00289325	7.520325	920	12.53887	9.024784	4.12043	3.40963	4.954868	2.066761	1.859375	6	7.95	6.5	-5.5	3.7	12.53624	87.69156	2.93E-06	6	13.75626	8.666667	16.81098	3.443732	24.84718	61.10949	30.4	240.8
// 1	2	0.5390368	0.00573716	0.5894479	0.00604493	19.39654	10.18103	7.964799	4.737743	0.08182969	0.08152778	0.07691239	0.07089887	0.01876508	0.009271372	4.8	8.7	5.55	-9	-0.4335597	-0.00246876	-0.7035286	-0.0028717	7.759314	926.6667	12.66065	10.01078	4.047312	3.343704	5.038685	2.050189	1.875	6	7.95	6.5	-5.8	3.4	12.43858	107.5397	3.26E-06	5.75	13.72442	9.022727	18.01931	3.472222	24.86595	60.67298	30.8	241.9

// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar

//////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////
CGPCM_JAZZtoSTDF::CGPCM_JAZZtoSTDF()
{
    m_pCGPcmParameter = NULL;
    m_lStartTime = 0;

    // Set default delimiter character
    m_cDelimiter=',';
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CGPCM_JAZZtoSTDF::~CGPCM_JAZZtoSTDF()
{
    // Destroy list of Parameters tables.
    if(m_pCGPcmParameter!=NULL)
        delete [] m_pCGPcmParameter;
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGPCM_JAZZtoSTDF::GetLastError()
{
    m_strLastError = "Import JAZZ: ";

    switch(m_iLastError)
    {
    default:
    case errNoError:
        m_strLastError += "No Error";
        break;
    case errOpenFail:
        m_strLastError += "Failed to open file";
        break;
    case errInvalidHeader:
        m_strLastError += "Invalid file format: Unexpected header format";
        break;
    case errInvalidFormatParameter:
        m_strLastError += "Invalid file format: Didn't find 'Parameter' line";
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
    case errInvalidFormatWaferId:
        m_strLastError += "Invalid file format: non-numeric WaferID";
        break;
    }
    // Return Error Message
    return m_strLastError;
}

//////////////////////////////////////////////////////////////////////
// Load PCM Parameter table from DISK
//////////////////////////////////////////////////////////////////////
void CGPCM_JAZZtoSTDF::LoadParameterIndexTable(void)
{
    QString	strWatTableFile;
    QString	strString;

    strWatTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
    strWatTableFile += GEX_WAT_PARAMETERS;

    // Open WAT Parameter table file
    QFile f( strWatTableFile );
    if(!f.open( QIODevice::ReadOnly ))
        return;

    // Assign file I/O stream
    QTextStream hWatTableFile(&f);

    // Skip comment lines
    do
    {
        strString = hWatTableFile.readLine();
    }
    while((strString.indexOf("----------------------") < 0) && (hWatTableFile.atEnd() == false));

    // Read lines
    m_pFullWatParametersList.clear();
    strString = hWatTableFile.readLine();
    while (strString.isNull() == false)
    {
        // Save Parameter name in list
        m_pFullWatParametersList.append(strString);
        // Read next line
        strString = hWatTableFile.readLine();
    };

    // Close file
    f.close();
}

//////////////////////////////////////////////////////////////////////
// Save PCM Parameter table to DISK
//////////////////////////////////////////////////////////////////////
void CGPCM_JAZZtoSTDF::DumpParameterIndexTable(void)
{
    QString		strWatTableFile;

    strWatTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
    strWatTableFile += GEX_WAT_PARAMETERS;

    // Open WAT Parameter table file
    QFile f( strWatTableFile );
    if(!f.open( QIODevice::WriteOnly ))
        return;

    // Assign file I/O stream
    QTextStream hWatTableFile(&f);

    // First few lines are comments:
    hWatTableFile << "############################################################" << endl;
    hWatTableFile << "# DO NOT EDIT THIS FILE!" << endl;
    hWatTableFile << "# Quantix Examinator: WAT/PCM Parameters detected" << endl;
    hWatTableFile << "# www.mentor.com" << endl;
    hWatTableFile << "# Quantix Examinator reads and writes into this file..." << endl;
    hWatTableFile << "-----------------------------------------------------------" << endl;

    // Write lines
    // m_pFullWatParametersList.sort();
    for(int nIndex = 0; nIndex < m_pFullWatParametersList.count(); nIndex++)
    {
        // Write line
        hWatTableFile << m_pFullWatParametersList[nIndex] << endl;
    };

    // Close file
    f.close();
}

//////////////////////////////////////////////////////////////////////
// If Examinator doesn't have this PCM parameter in his dictionnary, have it added.
//////////////////////////////////////////////////////////////////////
void CGPCM_JAZZtoSTDF::UpdateParameterIndexTable(QString strParamName)
{
    // Check if the table is empty...if so, load it from disk first!
    if(m_pFullWatParametersList.isEmpty() == true)
    {
        // Load WAT parameter table from disk...
        LoadParameterIndexTable();
    }

    // Check if Parameter name already in table...if not, add it to the list
    // the new full list will be dumped to the disk at the end.
    if(m_pFullWatParametersList.indexOf(strParamName) < 0)
    {
        // Update list
        m_pFullWatParametersList.append(strParamName);

        // Set flag to force the current PCM table to be updated on disk
        m_bNewWatParameterFound = true;
    }
}


//////////////////////////////////////////////////////////////////////
// Check if File is compatible with PCM_JAZZ format
//////////////////////////////////////////////////////////////////////
bool CGPCM_JAZZtoSTDF::IsCompatible(const char *szFileName)
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
    QTextStream hWatFile(&f);

    // Loop reading file until end is reached
    do
        strString = hWatFile.readLine();
    while(!strString.isNull() && strString.isEmpty());

    f.close();

    // Check if valid JAZZ PCM format
    if(strString.indexOf("JazzPartID", 0, Qt::CaseInsensitive) < 0)
    {
        // Incorrect header...this is not a Valid JAZZ PCM file!
        return false;
    }

    // 1st cell is 'PartID=xxx'
    strSection = strString.section(',',0,0); // PartID
    strSection = strSection.trimmed();	// remove leading spaces.
    if(strSection.startsWith("PartID") == false)
    {
        // Incorrect header...this is not a Jazz PCM file!
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the JAZZ file
//////////////////////////////////////////////////////////////////////
bool CGPCM_JAZZtoSTDF::ReadJazzFile(const char *JazzFileName,const char *strFileNameSTDF)
{
    QString strLine;
    QString strString;
    QString strSection;
    QStringList strCells;
    int		iIndex;				// Loop index
    bool	bStatus;

    // Delimiter character
    m_cDelimiter = ',';

    // Open E-Test JAZZ file
    QFile f( JazzFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening WAT file
        m_iLastError = errOpenFail;

        // Convertion failed.
        return false;
    }
    // Assign file I/O stream
    QTextStream hWatFile(&f);

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    iNextFilePos = 0;
    iProgressStep = 0;
    iFileSize = f.size() + 1;

    // Loop reading file until end is reached
    strLine = strString = ReadLine(hWatFile);

    // Check if valid JAZZ PCM format
    if(strString.indexOf("JazzPartID", 0, Qt::CaseSensitive) < 0)
    {
        // Incorrect header...this is not a Valid JAZZ PCM file!
        m_iLastError = errInvalidHeader;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    // 1st cell is 'PartID=xxx'
    strSection = strString.section(',',0,0); // PartID
    strSection = strSection.trimmed();	// remove leading spaces.
    if(strSection.startsWith("PartID") == false)
    {
        // Incorrect header...this is not a Jazz PCM file!
        m_iLastError = errInvalidHeader;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    m_strProductID = strSection.section('=',1);		// PartID ID
    m_strProductID = m_strProductID.trimmed();	// remove leading spaces.

    // 2nd cell is 'LotID=xxx'
    strSection = strLine.section(',',1,1);	// LotID
    strSection = strSection.trimmed();	// remove leading spaces.
    if(strSection.startsWith("LotID") == false)
    {
        // Incorrect header...this is not a Jazz PCM file!
        m_iLastError = errInvalidHeader;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    m_strLotID = strSection.section('=',1);	// Lot ID
    m_strLotID = m_strLotID.trimmed();	// remove leading spaces.

    // 3rd cell is 'TestProgramName=xxx'
    strSection = strLine.section(',',2,2);	// Test program name
    strSection = strSection.trimmed();	// remove leading spaces.
    if(strSection.startsWith("TestProgramName") == false)
    {
        // Incorrect header...this is not a Jazz PCM file!
        m_iLastError = errInvalidHeader;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    m_strProgramName = strSection.section('=',1);			// Program Name
    m_strProgramName = m_strProgramName.trimmed();	// remove leading spaces.

    // 4th cell is 'TesterId=xxx'
    strSection = strLine.section(',',3,3);	// Tester name
    strSection = strSection.trimmed();	// remove leading spaces.
    if(strSection.startsWith("TesterId") == false)
    {
        // Incorrect header...this is not a Jazz PCM file!
        m_iLastError = errInvalidHeader;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    m_strTesterName = strSection.section('=',1);			// Tester Name
    m_strTesterName = m_strTesterName.trimmed();	// remove leading spaces.

    // 5th cell is 'Date=MM/DD/YYYY'
    strSection = strLine.section(',',4,4);	// Date string
    strSection = strSection.trimmed();	// remove leading spaces.
    if(strSection.startsWith("Date") == false)
    {
        // Incorrect header...this is not a Jazz PCM file!
        m_iLastError = errInvalidHeader;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    int iDay=1,iMonth=1,iYear=1970;
    strString = strSection.section('=',1,1);		// Date format: MM-DD-YYYY
    strSection = strString.section('/',0,0);	// 1st field is Month
    iMonth = strSection.toInt();
    strSection = strString.section('/',1,1);	// 2nd field is Day
    iDay = strSection.toInt();
    strSection = strString.section('/',2,2);	// 3rd field is Year
    iYear = strSection.toInt();
    QDate WatDate(iYear,iMonth,iDay);

    // 6th cell is 'Time=HH:MM:SS'
    strSection = strLine.section(',',5,5);	// Time string
    strSection = strSection.trimmed();	// remove leading spaces.
    if(strSection.startsWith("Time") == false)
    {
        // Incorrect header...this is not a Jazz PCM file!
        m_iLastError = errInvalidHeader;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    int iHour,iMin,iSec;
    strString = strSection.section('=',1);		// Time format: HH:MM:SS
    strSection = strString.section(':',0,0);	// First field is HH
    iHour = strSection.toInt();
    strSection = strString.section(':',1,1);	// First field is MM
    iMin = strSection.toInt();
    strSection = strString.section(':',2,2);	// First field is SS
    iSec = strSection.toInt();
    QTime WatTime(iHour,iMin,iSec);
    QDateTime WateDateTime(WatDate,WatTime);
    WateDateTime.setTimeSpec(Qt::UTC);
    m_lStartTime = WateDateTime.toTime_t();

    // 7th cell: 'DCOP=xxx'
    strSection = strLine.section(',',6,6);	// DCOP
    strSection = strSection.trimmed();	// remove leading spaces.
    if(strSection.startsWith("DCOP") == false)
    {
        // Incorrect header...this is not a Jazz PCM file!
        m_iLastError = errInvalidHeader;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    // 8th cell: JazzPartID=xxx
    strSection = strLine.section(',',7,7);	// JazzPartID
    strSection = strSection.trimmed();	// remove leading spaces.
    if(strSection.startsWith("JazzPartID") == false)
    {
        // Incorrect header...this is not a Jazz PCM file!
        m_iLastError = errInvalidHeader;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    // Read line : "TestName,,VT,IDSAT,VT,IDSAT...'
    strString = ReadLine(hWatFile);
    QStringList strCellsTestName;
    m_iTotalParameters=0;

    strCellsTestName = strString.split(m_cDelimiter, QString::KeepEmptyParts);
    m_iTotalParameters = strCellsTestName.count();
    // If no parameter specified...ignore!
    if(m_iTotalParameters <= 0)
    {
        // Incorrect header...this is not a valid CSV file!
        m_iLastError = errInvalidFormatLowInRows;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }


    // Read line : "DeviceName,,10XP18N,10XP18N...'
    QStringList strCellsDeviceName;
    strString = ReadLine(hWatFile);
    strCellsDeviceName = strString.split(m_cDelimiter, QString::KeepEmptyParts);
    // Check if have the good count
    if(strCellsDeviceName.count() < m_iTotalParameters)
    {
        m_iLastError = errInvalidFormatLowInRows;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    // Allocate the buffer to hold the N parameters & results + init test names.
    m_pCGPcmParameter = new CGJazzParameter[m_iTotalParameters];	// List of parameters

    for(iIndex=2;iIndex<m_iTotalParameters;iIndex++)
    {
        // Remove space & '.' and " characters
        strSection = strCellsTestName[iIndex].trimmed();	// Get field and remove spaces
        strSection += " - ";
        strSection += strCellsDeviceName[iIndex].trimmed();	// Get field and remove spaces

        m_pCGPcmParameter[iIndex].strName = strSection;	// Test name.
        m_pCGPcmParameter[iIndex].bStaticHeaderWritten = false;

        // Update list of test names.
        UpdateParameterIndexTable(strSection);
    }

    // Read High Limits line.
    // Extract the N column Upper Limits
    strString = ReadLine(hWatFile);
    strCells = strString.split(m_cDelimiter, QString::KeepEmptyParts);
    // Check if have the good count
    if(strCells.count() < m_iTotalParameters)
    {
        m_iLastError = errInvalidFormatLowInRows;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    for(iIndex=2;iIndex<m_iTotalParameters;iIndex++)
    {
        strSection = strCells[iIndex].trimmed();
        m_pCGPcmParameter[iIndex].fHighLimit = strSection.toFloat(&bStatus);
        m_pCGPcmParameter[iIndex].bValidHighLimit = bStatus;
    }

    // Read Low Limits line.
    // Extract the N column Lower Limits
    strString = ReadLine(hWatFile);
    strCells = strString.split(m_cDelimiter, QString::KeepEmptyParts);
    // Check if have the good count
    if(strCells.count() < m_iTotalParameters)
    {
        m_iLastError = errInvalidFormatLowInRows;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    for(iIndex=2;iIndex<m_iTotalParameters;iIndex++)
    {
        strSection = strCells[iIndex].trimmed();
        m_pCGPcmParameter[iIndex].fLowLimit = strSection.toFloat(&bStatus);
        m_pCGPcmParameter[iIndex].bValidLowLimit = bStatus;
    }

    // Skip "," line
    //ReadLine(hWatFile);

    // Extract the N column Units
    strString = ReadLine(hWatFile);
    strCells = strString.split(m_cDelimiter, QString::KeepEmptyParts);
    // Check if have the good count
    if(strCells.count() < m_iTotalParameters)
    {
        m_iLastError = errInvalidFormatLowInRows;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    if(strCells[0] != "Wafer#")
    {
        // Incorrect header...this is not a valid CSV file!
        m_iLastError = errInvalidFormatLowInRows;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    // Extract units & compute scae factor (so to normalize limits & units)
    for(iIndex=2;iIndex<m_iTotalParameters;iIndex++)
    {
        strString = strCells[iIndex].trimmed();
        NormalizeLimits(iIndex,strString);
    }

    // Loop reading file until end is reached & generate STDF file dynamically.
    bStatus = WriteStdfFile(&hWatFile,strFileNameSTDF);
    if(!bStatus)
        QFile::remove(strFileNameSTDF);

    // Close file
    f.close();

    // All JAZZ file read...check if need to update the PCM Parameter list on disk?
    if(bStatus && (m_bNewWatParameterFound == true))
        DumpParameterIndexTable();

    // Success parsing CSV file
    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Normalize test limits when writing into PTR.
//////////////////////////////////////////////////////////////////////
void CGPCM_JAZZtoSTDF::NormalizeLimits(int iIndex,QString &strUnits)
{
    int	value_scale=0;
    if(strUnits.length() <= 1)
    {
        // units too short to include a prefix, then keep it 'as-is'
        m_pCGPcmParameter[iIndex].scale = 0;
        m_pCGPcmParameter[iIndex].strUnits = strUnits;
        return;
    }

    QChar cPrefix = strUnits[0];
    switch(cPrefix.toLatin1())
    {
    case 'm': // Milli
        value_scale = -3;
        break;
    case 'u': // Micro
        value_scale = -6;
        break;
    case 'n': // Nano
        value_scale = -9;
        break;
    case 'p': // Pico
        value_scale = -12;
        break;
    case 'f': // Fento
        value_scale = -15;
        break;
    case 'K': // Kilo
        value_scale = 3;
        break;
    case 'M': // Mega
        value_scale = 6;
        break;
    case 'G': // Giga
        value_scale = 9;
        break;
    case 'T': // Tera
        value_scale = 12;
        break;
    }
    m_pCGPcmParameter[iIndex].fLowLimit *= GS_POW(10.0,value_scale);
    m_pCGPcmParameter[iIndex].fHighLimit *= GS_POW(10.0,value_scale);
    m_pCGPcmParameter[iIndex].scale = value_scale;
    if(value_scale)
        strUnits = strUnits.mid(1);	// Take all characters after the prefix.
    m_pCGPcmParameter[iIndex].strUnits = strUnits;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from CSV data parsed
//////////////////////////////////////////////////////////////////////
bool CGPCM_JAZZtoSTDF::WriteStdfFile(QTextStream *hWatFile, const char *strFileNameSTDF)
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
    StdfFile.WriteString(m_strTesterName.toLatin1().constData());// Node name
    StdfFile.WriteString("JAZZ");				// Tester Type
    StdfFile.WriteString(m_strProgramName.toLatin1().constData());	// Job name
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
    strUserTxt += ":JAZZ";
    StdfFile.WriteString((char *)strUserTxt.toLatin1().constData());	// user-txt
    StdfFile.WriteString("");							// aux-file
    StdfFile.WriteString("");							// package-type
    StdfFile.WriteString("");							// familyID
    StdfFile.WriteString("");							// Date-code
    StdfFile.WriteString("");							// Facility-ID
    StdfFile.WriteString("");							// FloorID
    StdfFile.WriteString("");							// ProcessID
    StdfFile.WriteString("");							// Frequency/Step
    StdfFile.WriteRecord();

    // Write Test results for each line read.
    QStringList		strCells;
    QString			strWaferID;
    QString			strString;
    QString			strSection;
    float			fValue=0.0F;		// Used for readng floating point numbers.
    int				iIndex;				// Loop index
    BYTE			bData;
    WORD			wBin;
    long			iTotalGoodBin,iTotalFailBin;
    long			iTestNumber,iPartNumber;
    bool			bStatus,bPassStatus;
    int				value_scale;	// Scale factor for limots & results.
    int				iSiteID;

    // Reset counters
    iTotalGoodBin=iTotalFailBin=0;
    iPartNumber=0;
    iSiteID = 0;

    // Write all Parameters read on this wafer.: PTR....PTR, PRR
    while(hWatFile->atEnd() == false)
    {
        // Read line
        strString = ReadLine(*hWatFile);
        strString = strString.remove('"');

        // Check if we've reached the end of the file
        if (strString.isEmpty() ||
            strString.startsWith("Mean", Qt::CaseInsensitive))
        {
            break;
        }

        // Split line
        strCells = strString.split(m_cDelimiter, QString::KeepEmptyParts);
        // Check if have the good count
        if(strCells.count() < 2)
        {
            m_iLastError = errInvalidFormatLowInRows;

            // Convertion failed.
            return false;
        }

        // Write a WRR
        //1,7,50.005,7964.954,124.... '1' into strWaferID
        //2,1,50.005,7914.523,122.... '2' new WaferId

        if (!strWaferID.isEmpty()
                && (strWaferID != strCells[0]))
        {
            // WRR
            RecordReadInfo.iRecordType = 2;
            RecordReadInfo.iRecordSubType = 20;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(1);						// Test head
            StdfFile.WriteByte(255);					// Tester site (all)
            StdfFile.WriteDword(m_lStartTime);			// Time of last part tested
            StdfFile.WriteDword(iTotalGoodBin+iTotalFailBin);// Parts tested: always 7 in JAZZ file.
            StdfFile.WriteDword(0);						// Parts retested
            StdfFile.WriteDword(0);						// Parts Aborted
            StdfFile.WriteDword(iTotalGoodBin);			// Good Parts
            StdfFile.WriteDword((DWORD)-1);			// Functionnal Parts
            StdfFile.WriteString(strWaferID.toLatin1().constData());	// WaferID
            StdfFile.WriteRecord();
        }

        iTotalGoodBin=iTotalFailBin=0;

        // Write a WIR
        if (strWaferID != strCells[0])
        {
            // For each wafer, have to write limit in the first PTR
            for(iIndex=0;iIndex<m_iTotalParameters;iIndex++)
                m_pCGPcmParameter[iIndex].bStaticHeaderWritten = false;

            // New wafer sequence: write WIR
            RecordReadInfo.iRecordType = 2;
            RecordReadInfo.iRecordSubType = 10;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(1);								// Test head
            StdfFile.WriteByte(255);							// Tester site (all)
            StdfFile.WriteDword(m_lStartTime);					// Start time
            StdfFile.WriteString(strCells[0].toLatin1().constData());	// WaferID
            StdfFile.WriteRecord();
        }

        // Extract current waferID and SiteID
        strWaferID = strCells[0];
        iSiteID = strCells[1].toInt();

        // Make sure we have a numeric WaferID
        strWaferID.toUInt(&bStatus);
        if(!bStatus)
        {
            m_iLastError = errInvalidFormatWaferId;

            // Convertion failed.
            return false;
        }



        // Keep track of total Runs extracted
        iPartNumber++;

        // Clear Pass/Fail flag.
        bPassStatus = true;

        // Write PIR
        RecordReadInfo.iRecordType = 5;
        RecordReadInfo.iRecordSubType = 10;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);			// Test head
        StdfFile.WriteByte(1);			// Tester site#
        StdfFile.WriteRecord();

        // Read Parameter results for this record
        for(iIndex = 2; iIndex<m_iTotalParameters; iIndex++)
        {
            if(strCells.count() <= iIndex)
                break;

            // We have a result specified...convert it to a floating point value.
            strSection = strCells[iIndex].trimmed();
            fValue = strSection.toFloat(&bStatus);

            if(bStatus == true)
            {
                // Valid test result...write the PTR
                RecordReadInfo.iRecordType = 15;
                RecordReadInfo.iRecordSubType = 10;
                StdfFile.WriteHeader(&RecordReadInfo);

                // Compute Test# (add user-defined offset)
                iTestNumber = (long) m_pFullWatParametersList.indexOf(m_pCGPcmParameter[iIndex].strName);
                iTestNumber += GEX_TESTNBR_OFFSET_PCM;		// Test# offset

                StdfFile.WriteDword(iTestNumber);			// Test Number
                StdfFile.WriteByte(1);						// Test head
                StdfFile.WriteByte(1);						// Tester site#

                // Normalize result
                value_scale = m_pCGPcmParameter[iIndex].scale;
                fValue*= GS_POW(10.0,value_scale);

                // check If test fails limits
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
                    StdfFile.WriteByte(-value_scale);					// LLM_SCALE
                    StdfFile.WriteByte(-value_scale);					// HLM_SCALE
                    StdfFile.WriteFloat(m_pCGPcmParameter[iIndex].fLowLimit);		// LOW Limit
                    StdfFile.WriteFloat(m_pCGPcmParameter[iIndex].fHighLimit);	// HIGH Limit
                    StdfFile.WriteString(m_pCGPcmParameter[iIndex].strUnits.toLatin1().constData());	// Units
                    m_pCGPcmParameter[iIndex].bStaticHeaderWritten = true;
                }
                StdfFile.WriteRecord();
            }	// Valid test result
        };	// Read all results on line

        // Write PRR
        RecordReadInfo.iRecordType = 5;
        RecordReadInfo.iRecordSubType = 20;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);			// Test head
        // Tester site#
        StdfFile.WriteByte(1);	// Tester site#
        if(bPassStatus == true)
        {
            StdfFile.WriteByte(0);				// PART_FLG : PASSED
            iTotalGoodBin++;
            wBin = 1;	// PASS bin
        }
        else
        {
            StdfFile.WriteByte(8);				// PART_FLG : FAILED
            iTotalFailBin++;
            wBin = 0;	// FAIL bin
        }

        StdfFile.WriteWord((WORD)m_iTotalParameters-2);	// NUM_TEST
        StdfFile.WriteWord(wBin);		        // HARD_BIN
        StdfFile.WriteWord(wBin);	           // SOFT_BIN
        switch(iSiteID)
        {
        case 7:	// Top-Left
            StdfFile.WriteWord(0);			// X_COORD
            StdfFile.WriteWord(0);			// Y_COORD
            break;
        case 3:	// Top
            StdfFile.WriteWord(1);			// X_COORD
            StdfFile.WriteWord(0);			// Y_COORD
            break;
        case 2:	// Top-Right
            StdfFile.WriteWord(2);			// X_COORD
            StdfFile.WriteWord(0);			// Y_COORD
            break;
        case 4:	// Center
            StdfFile.WriteWord(1);			// X_COORD
            StdfFile.WriteWord(1);			// Y_COORD
            break;
        case 6:	// Down-Left
            StdfFile.WriteWord(0);			// X_COORD
            StdfFile.WriteWord(2);			// Y_COORD
            break;
        case 5:	// Down
            StdfFile.WriteWord(1);			// X_COORD
            StdfFile.WriteWord(2);			// Y_COORD
            break;
        case 1:	// Down-Right
            StdfFile.WriteWord(2);			// X_COORD
            StdfFile.WriteWord(2);			// Y_COORD
            break;
        }
        StdfFile.WriteDword(0);					// Testing time (if known, otherwise: 0)
        strSection = strString.section(m_cDelimiter,0,0);
        strSection += strString.section(m_cDelimiter,1,1);
        StdfFile.WriteString((char *)strSection.toLatin1().constData());	// PART_ID 'XY' X=Part#, Y=site#
        StdfFile.WriteRecord();
    }	// Read all lines in file

    // Last WRR
    if (iPartNumber)
    {
        RecordReadInfo.iRecordType = 2;
        RecordReadInfo.iRecordSubType = 20;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);  // Test head
        StdfFile.WriteByte(255);  // Tester site (all)
        StdfFile.WriteDword(m_lStartTime);  // Time of last part tested
        // Parts tested: always 7 in JAZZ file.
        StdfFile.WriteDword(iTotalGoodBin + iTotalFailBin);
        StdfFile.WriteDword(0);  // Parts retested
        StdfFile.WriteDword(0);  // Parts Aborted
        StdfFile.WriteDword(iTotalGoodBin);  // Good Parts
        StdfFile.WriteDword((DWORD) -1);  // Functionnal Parts
        StdfFile.WriteString(strWaferID.toLatin1().constData());  // WaferID
        StdfFile.WriteRecord();
    }

    // If no binning data given in the CSV file, write our extrapolation.

#if 0
    // NO SUMMARY FOR ETEST
    // Write SBR Bin0 (FAIL)
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 50;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(255);					// Test Head = ALL
    StdfFile.WriteByte(255);					// Test sites = ALL
    StdfFile.WriteWord(0);						// SBIN = 0
    StdfFile.WriteDword(iTotalFailBin);			// Total Bins
    StdfFile.WriteByte('F');
    StdfFile.WriteString("Failing sites");
    StdfFile.WriteRecord();

    // Write SBR Bin1 (PASS)
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 50;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(255);					// Test Head = ALL
    StdfFile.WriteByte(255);					// Test sites = ALL
    StdfFile.WriteWord(1);						// SBIN = 1
    StdfFile.WriteDword(iTotalGoodBin);			// Total Bins
    StdfFile.WriteByte('P');
    StdfFile.WriteString("Good sites");
    StdfFile.WriteRecord();

    // Write HBR Bin0 (FAIL)
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 40;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(255);					// Test Head = ALL
    StdfFile.WriteByte(255);					// Test sites = ALL
    StdfFile.WriteWord(0);						// HBIN = 0
    StdfFile.WriteDword(iTotalFailBin);			// Total Bins
    StdfFile.WriteByte('F');
    StdfFile.WriteString("Failing sites");
    StdfFile.WriteRecord();

    // Write HBR Bin1 (PASS)
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 40;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(255);					// Test Head = ALL
    StdfFile.WriteByte(255);					// Test sites = ALL
    StdfFile.WriteWord(1);						// HBIN = 1
    StdfFile.WriteDword(iTotalGoodBin);			// Total Bins
    StdfFile.WriteByte('P');
    StdfFile.WriteString("Good sites");
    StdfFile.WriteRecord();

    // Write PCR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 30;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(255);					// Test Head = ALL
    StdfFile.WriteByte(255);					// Test sites = ALL
    StdfFile.WriteDword(iPartNumber);			// Total Parts tested
    StdfFile.WriteDword(0);						// Total Parts re-tested
    StdfFile.WriteDword(0);						// Total Parts aborted
    StdfFile.WriteDword(iTotalGoodBin);			// Total GOOD Parts
    StdfFile.WriteRecord();

#endif

    // Write MRR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteDword(m_lStartTime);			// finish-time= start time as no info in file!
    StdfFile.WriteRecord();

    // Close STDF file.
    StdfFile.Close();

    // Success
    return true;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' JAZZ (Etest) file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGPCM_JAZZtoSTDF::Convert(const char *JazzFileName, const char *strFileNameSTDF)
{
    // No erro (default)
    m_iLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(JazzFileName);
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
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(JazzFileName).fileName()+"...");
            GexScriptStatusLabel->show();
        }
    }
    QCoreApplication::processEvents();

    if(ReadJazzFile(JazzFileName,strFileNameSTDF) != true)
    {
        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if((GexProgressBar != NULL)
                && bHideProgressAfter)
            GexProgressBar->hide();

        if((GexScriptStatusLabel != NULL)
                && bHideLabelAfter)
            GexScriptStatusLabel->hide();
        return false;	// Error reading SPD file
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
QString CGPCM_JAZZtoSTDF::ReadLine(QTextStream& hFile)
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

        // Test if we have only comma in the line
        if(strString.simplified().count(",") == strString.simplified().length())
        {
            strString = "";
        }
    }
    while(!strString.isNull() && strString.isEmpty() && !hFile.atEnd());

    return strString;
}
