//////////////////////////////////////////////////////////////////////
// import_csm_type_2.cpp: Convert a .TXT (CSM) Type 2 ASCII  file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#include "import_csm_type_2.h"

#include <math.h>
#include <time.h>

#ifdef _WIN32
#include "windows.h"		// For 'GetWindowsDirectoryA' declaration
#endif

#include <qdir.h>
#include <qfileinfo.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>
#include <qregexp.h>

#include "db_transactions.h"
#include "engine.h"
#include "gex_database_entry.h"
#include "import_constants.h"
#include "temporary_files_manager.h"

// File format:
// PRODUCT,LOT,WAFER,SPLIT,SITE,X_ADDR,Y_ADDR,PW_RES,NW_RES, .....
// LSL,,,,,,,100,200,150,.....
// HSL,,,,,,,900,860,1200,...
// TARGET,,,,,,,500,530,675,...
// LTL,,,,,,,150,230,180,...
// HTL,,,,,,,900,860,1060,...
// TARGET,,,,,,,500,530,675,...
// CS90700,7SWD23277.000,13,4.3/5.0/3.5,1,2,7,1146,439.2,0.2238,8.161,....

// in main.cpp
extern QLabel *			GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar

//////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////
CGCSM2toSTDF::CGCSM2toSTDF()
{
    // Default: CSM parameter list on disk includes all known CSM parameters...
    m_bNewCsmParameterFound = false;
    m_lStartTime = 0;

    m_pCGCsm2Parameter = NULL;
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CGCSM2toSTDF::~CGCSM2toSTDF()
{
    // Destroy list of Parameters tables.
    if(m_pCGCsm2Parameter!=NULL)
        delete [] m_pCGCsm2Parameter;
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGCSM2toSTDF::GetLastError()
{
    m_strLastError = "Import CSM: ";

    switch(m_iLastError)
    {
        default:
        case errNoError:
            m_strLastError += "No Error";
            break;
        case errOpenFail:
            m_strLastError += "Failed to open file";
            break;
        case errInvalidFormatParameter:
            m_strLastError += "Invalid file format: Didn't find 'PRODUCT' header line";
            break;
        case errInvalidFormatLowInRows:
            m_strLastError += "Invalid file format: 'Parameter' line too short, missing rows";
            break;
        case errInvalidFormatMissingUnit:
            m_strLastError += "Invalid file format: 'Units' line missing";
            break;
        case errInvalidFormatMissingUSL:
            m_strLastError += "Invalid file format: 'USL' line missing";
            break;
        case errInvalidFormatMissingLSL:
            m_strLastError += "Invalid file format: 'LSL' line missing";
            break;
        case errInvalidFormatMissingHTL:
            m_strLastError += "Invalid file format: 'HTL' line missing";
            break;
        case errInvalidFormatMissingLTL:
            m_strLastError += "Invalid file format: 'LTL' line missing";
            break;
        case errInvalidFormatMissingTarget:
            m_strLastError += "Invalid file format: 'TARGET' line missing";
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
// Check if File is compatible with GDF format
//////////////////////////////////////////////////////////////////////
bool CGCSM2toSTDF::IsCompatible(QString szFileName)
{
    // Open CSM file
    QFile f( szFileName );
    if(!f.open( QIODevice::ReadOnly ))
        return false;

    // Assign file I/O stream
    QTextStream hCsmFile(&f);

    // Check if valid CSM header...extract header data.
    QString strString;
    strString = hCsmFile.readLine();

    // Close file
    f.close();

    if(strString.startsWith("PRODUCT,LOT,WAFER,SPLIT,SITE,X_ADDR,Y_ADDR,", Qt::CaseInsensitive) == false)
        return false;

    return true;
}

//////////////////////////////////////////////////////////////////////
// Load CSM Parameter table from DISK
//////////////////////////////////////////////////////////////////////
void CGCSM2toSTDF::LoadParameterIndexTable(void)
{
    QString	strCsvTableFile;
    QString	strString;

    strCsvTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
    strCsvTableFile += GEX_CSM_TYPE_2_PARAMETERS;

    // Open CSM Parameter table file
    QFile f( strCsvTableFile );
    if(!f.open( QIODevice::ReadOnly ))
        return;

    // Assign file I/O stream
    QTextStream hCsvTableFile(&f);

    // Skip comment lines
    do
    {
      strString = hCsvTableFile.readLine();
    }
    while((strString.indexOf("----------------------") < 0) && (hCsvTableFile.atEnd() == false));

    // Read lines
    m_pFullCsmParametersList.clear();
    strString = hCsvTableFile.readLine();
    while (strString.isNull() == false)
    {
        // Save Parameter name in list
        m_pFullCsmParametersList.append(strString);
        // Read next line
        strString = hCsvTableFile.readLine();
    };

    // Close file
    f.close();
}

//////////////////////////////////////////////////////////////////////
// Save CSM Parameter table to DISK
//////////////////////////////////////////////////////////////////////
void CGCSM2toSTDF::DumpParameterIndexTable(void)
{
    QString		strCsvTableFile;
    QString		strString;

    strCsvTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
    strCsvTableFile += GEX_CSM_TYPE_2_PARAMETERS;

    // Open CSM Parameter table file
    QFile f( strCsvTableFile );
    if(!f.open( QIODevice::WriteOnly ))
        return;

    // Assign file I/O stream
    QTextStream hCsvTableFile(&f);

    // First few lines are comments:
    hCsvTableFile << "############################################################" << endl;
    hCsvTableFile << "# DO NOT EDIT THIS FILE!" << endl;
    hCsvTableFile << "# Quantix Examinator: CSM Parameters detected" << endl;
    hCsvTableFile << "# www.mentor.com" << endl;
    hCsvTableFile << "# Quantix Examinator reads and writes into this file..." << endl;
    hCsvTableFile << "-----------------------------------------------------------" << endl;

    // Write lines
    // m_pFullCsmParametersList.sort();
    for (QStringList::const_iterator
         iter  = m_pFullCsmParametersList.begin();
         iter != m_pFullCsmParametersList.end(); ++iter) {
        // Write line
        hCsvTableFile << *iter << endl;
    }

    // Close file
    f.close();
}

//////////////////////////////////////////////////////////////////////
// If Examinator doesn't have this CSM parameter in his dictionnary, have it added.
//////////////////////////////////////////////////////////////////////
void CGCSM2toSTDF::UpdateParameterIndexTable(QString strParamName)
{
    // Check if the table is empty...if so, load it from disk first!
    if(m_pFullCsmParametersList.isEmpty() == true)
    {
        // Load CSM parameter table from disk...
        LoadParameterIndexTable();
    }

    // Check if Parameter name already in table...if not, add it to the list
    // the new full list will be dumped to the disk at the end.
    if(m_pFullCsmParametersList.indexOf(strParamName) < 0)
    {
        // Update list
        m_pFullCsmParametersList.append(strParamName);

        // Set flag to force the current CSM table to be updated on disk
        m_bNewCsmParameterFound = true;
    }
}

//////////////////////////////////////////////////////////////////////
// Clear variables
//////////////////////////////////////////////////////////////////////
void CGCSM2toSTDF::clear(void)
{
    m_lfWaferDiameter=0.0;			// Wafer diameter in mm
    m_lfWaferDieWidth=0.0;			// Die X size
    m_lfWaferDieHeight=0.0;			// Die Y size
    m_cWaferFlat=' ';				// Flat orientation: U, D, R or L
    m_iWaferCenterDieX = m_iWaferCenterDieY = -32768;	// coordinates of center die on wafer
    m_cWaferPosX = m_cWaferPosY = ' ';	// Positive X & Y direction of wafer
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the CSM type 2 file
//////////////////////////////////////////////////////////////////////
bool CGCSM2toSTDF::ReadCsmFile(const char *CsvFileName,const char *strFileNameSTDF)
{
    QString strString;
    QString strSection;
    bool	bStatus;
    unsigned	uIndex;				// Loop index

    // Clear variables
    clear();

    // Open CSM file
    QFile f( CsvFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening CSM file
        m_iLastError = errOpenFail;

        // Convertion failed.
        return false;
    }
    // Assign file I/O stream
    QTextStream hCsmFile(&f);

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    iFileSize = f.size()+1;

    // Current time
    m_lStartTime = time(NULL);
    m_strTesterName = m_strTesterType = m_strProgramName= "CSMC";				// Job name
    m_strProgramRev = m_strOperator = m_strExecType = m_strExecRev = "CSMc";
    m_strTestCode = m_strFacilityID = m_strFloorID = m_strProcessID = "CSMC";
    m_iBurninTime = 65535;

    // Check if valid CSM header...extract header data.
    strString = ReadLine(hCsmFile);
    if(strString.startsWith("PRODUCT,LOT,WAFER,SPLIT,SITE,X_ADDR,Y_ADDR,", Qt::CaseInsensitive) == false)
    {
        // Incorrect header...this is not a valid CSM file!
        m_iLastError = errInvalidFormatParameter;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    // Count the number of parameters specified in the line
    QStringList strCells;
    QString strTestNumber,strTestPinmap;
    m_uTotalParameters=0;

    strCells = strString.split(",",QString::KeepEmptyParts);
    m_uTotalParameters = strCells.count();
    // If no parameter specified...ignore!
    if(m_uTotalParameters <= 0)
    {
        m_iLastError = errInvalidFormatLowInRows;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    m_lStartParsingOffset = CSM_RAW_Y_ADDR;

    // Allocate the buffer to hold the N parameters & results.
    m_pCGCsm2Parameter = new CGCsm2Parameter[m_uTotalParameters];	// List of parameters

    // Extract the N column names
    for(uIndex=0;uIndex<m_uTotalParameters;uIndex++)
    {
        m_pCGCsm2Parameter[uIndex].lTestNumber = (unsigned long)-1;
        m_pCGCsm2Parameter[uIndex].lPinmapIndex = -1;

        if (uIndex >= (unsigned int) strCells.count())
            strSection = "";
        else
            strSection = strCells[uIndex+1];
        strSection = strSection.trimmed();	// Remove spaces
        if(strSection.isEmpty())
            strSection = "-?-";	// Default name for any missing label!
        m_pCGCsm2Parameter[uIndex].strName = strSection;
        UpdateParameterIndexTable(strSection);		// Update Parameter master list if needed.
        m_pCGCsm2Parameter[uIndex].bStaticHeaderWritten = false;
    }

    // Read next line: LSL (Low Spec Limits)
    strString = ReadLine(hCsmFile);
    strCells = strString.split(",",QString::KeepEmptyParts);
    // Check if have the good count
    if ((unsigned int) strCells.count() < m_uTotalParameters)
    {
        // Incorrect header...this is not a valid CSM file!
        m_iLastError = errInvalidFormatLowInRows;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }


    strSection = strCells[0];
    if(strSection.startsWith("LSL", Qt::CaseInsensitive) == false && strSection.startsWith("SCL", Qt::CaseInsensitive) == false)
    {
        // Incorrect header...this is not a valid CSM file!
        m_iLastError = errInvalidFormatMissingLSL;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    // Extract the N column LSL Limits
    for(uIndex=0;uIndex<m_uTotalParameters;uIndex++)
    {
        strSection = strCells[uIndex+1].trimmed();
        m_pCGCsm2Parameter[uIndex].fLowSpecLimit = strSection.toFloat(&bStatus);
        if(uIndex >= (unsigned int) strCells.count())
            bStatus = false;
        m_pCGCsm2Parameter[uIndex].bValidLowSpecLimit = bStatus;
    }

    // Read next line: HSL (High Spec Limits)
    strString = ReadLine(hCsmFile);
    strCells = strString.split(",",QString::KeepEmptyParts);
    // Check if have the good count
    if ((unsigned int) strCells.count() < m_uTotalParameters)
    {
        m_iLastError = errInvalidFormatLowInRows;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    strSection = strCells[0];
    if(strSection.startsWith("HSL", Qt::CaseInsensitive) == false && strSection.startsWith("SCH", Qt::CaseInsensitive) == false)
    {
        // Incorrect header...this is not a valid CSM file!
        m_iLastError = errInvalidFormatMissingUSL;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    // Extract the N column HSL Limits
    for(uIndex=0;uIndex<m_uTotalParameters;uIndex++)
    {
        strSection = strCells[uIndex+1].trimmed();
        m_pCGCsm2Parameter[uIndex].fHighSpecLimit = strSection.toFloat(&bStatus);
        if (uIndex >= (unsigned int) strCells.count())
            bStatus = false;
        m_pCGCsm2Parameter[uIndex].bValidHighSpecLimit = bStatus;
    }

        // Read next line: TARGET (Spec target value)
    strString = ReadLine(hCsmFile);
    strCells = strString.split(",",QString::KeepEmptyParts);
    // Check if have the good count
    if ((unsigned int) strCells.count() < m_uTotalParameters)
    {
        m_iLastError = errInvalidFormatLowInRows;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    strSection = strCells[0];
    if(strSection.startsWith("TARGET", Qt::CaseInsensitive) == false)
    {
        // Incorrect header...this is not a valid CSM file!
        m_iLastError = errInvalidFormatMissingTarget;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    // Extract the N column TARGET values
    for(uIndex=0;uIndex<m_uTotalParameters;uIndex++)
    {
        strSection = strCells[uIndex+1].trimmed();
        m_pCGCsm2Parameter[uIndex].fTargetValue = strSection.toFloat(&bStatus);
        if (uIndex >= (unsigned int) strCells.count())
            bStatus = false;
        m_pCGCsm2Parameter[uIndex].bValidTarget = bStatus;
    }


    // Read line with list of Parameters Lower Test Limit
    strString = ReadLine(hCsmFile);
    strCells = strString.split(",",QString::KeepEmptyParts);
    // Check if have the good count
    if ((unsigned int) strCells.count() < m_uTotalParameters)
    {
        m_iLastError = errInvalidFormatLowInRows;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    strSection = strCells[0];
    if(strSection.startsWith("LTL", Qt::CaseInsensitive) == false && strSection.startsWith("SPL", Qt::CaseInsensitive) == false)
    {
        // Incorrect header...this is not a valid CSM file!
        m_iLastError = errInvalidFormatMissingLTL;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    // Extract the N column Lower Test Limits
    for(uIndex=0;uIndex<m_uTotalParameters;uIndex++)
    {
        strSection = strCells[uIndex+1].trimmed();
        m_pCGCsm2Parameter[uIndex].fLowLimit = strSection.toFloat(&bStatus);
        if (uIndex >= (unsigned int) strCells.count())
            bStatus = false;
        m_pCGCsm2Parameter[uIndex].bValidLowLimit = bStatus;
    }


    // Read line with list of Parameters High Test Limit
    strString = ReadLine(hCsmFile);
    strCells = strString.split(",",QString::KeepEmptyParts);
    // Check if have the good count
    if ((unsigned int) strCells.count() < m_uTotalParameters)
    {
        m_iLastError = errInvalidFormatLowInRows;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    strSection = strCells[0];
    if(strSection.startsWith("HTL", Qt::CaseInsensitive) == false && strSection.startsWith("SPH", Qt::CaseInsensitive) == false)
    {
        // Incorrect header...this is not a valid CSM file!
        m_iLastError = errInvalidFormatMissingHTL;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    // Extract the N column Upper Test Limits
    for(uIndex=0;uIndex<m_uTotalParameters;uIndex++)
    {
        strSection = strCells[uIndex+1].trimmed();
        m_pCGCsm2Parameter[uIndex].fHighLimit = strSection.toFloat(&bStatus);
        if (uIndex >= (unsigned int) strCells.count())
            bStatus = false;
        m_pCGCsm2Parameter[uIndex].bValidHighLimit = bStatus;
    }


    // Read line with list of Parameters UNITS
    strString = ReadLine(hCsmFile);
    strCells = strString.split(",",QString::KeepEmptyParts);
    // Check if have the good count
    if ((unsigned int) strCells.count() < m_uTotalParameters)
    {
        m_iLastError = errInvalidFormatLowInRows;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    strSection = strCells[0];
    if(strSection.startsWith("UNITS", Qt::CaseInsensitive) == false)
    {
        // Incorrect header...this is not a valid CSM file!
        m_iLastError = errInvalidFormatMissingUnit;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    // Extract the N column Units
    for(uIndex=0;uIndex<m_uTotalParameters;uIndex++)
    {
        strSection = strCells[uIndex+1];
        strSection = strSection.trimmed();	// Remove spaces
        if (uIndex < (unsigned int) strCells.count())
            m_pCGCsm2Parameter[uIndex].strUnits = strSection;
    }

    // Loop reading file until end is reached & generate STDF file dynamically.
    bStatus = WriteStdfFile(&hCsmFile,strFileNameSTDF);
    if(!bStatus)
        QFile::remove(strFileNameSTDF);

    // Close file
    f.close();

    // All CSM file read...check if need to update the CSM Parameter list on disk?
    if(m_bNewCsmParameterFound == true)
        DumpParameterIndexTable();

    // Success parsing CSM file
    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from CSM data parsed
//////////////////////////////////////////////////////////////////////
int	CGCSM2toSTDF::ResultDisplayScaleFactor(unsigned uIndex)
{
    // If no units defined, do not rescale.
    if(m_pCGCsm2Parameter[uIndex].strUnits.isEmpty())
        return 0;

    // If no smart-scaling enabled, return!
    //if(!ReportOptions.bStatsSmartScaling)
    //	return 0;

    // Do not customize scaling if units are not normalized.
    const char *ptChar = m_pCGCsm2Parameter[uIndex].strUnits.toLatin1().constData();
    switch(*ptChar)
    {
        case 'm':
        case 'M':
        case 'K':
        case 'G':
        case 'T':
        case 'u':
        case 'n':
        case 'p':
        case 'f':
            return 0;
    }

    // Check the limits
    float	fLowLimit=0;
    float	fHighLimit=0;

    if(m_pCGCsm2Parameter[uIndex].bValidLowLimit)
        fLowLimit = fabs(m_pCGCsm2Parameter[uIndex].fLowLimit);

    if(m_pCGCsm2Parameter[uIndex].bValidHighLimit)
        fHighLimit = fabs(m_pCGCsm2Parameter[uIndex].fHighLimit);

    float fMiddle = (fHighLimit + fLowLimit)/2;
    if(!fMiddle)
        return	0;

    int	iExponent= (int) log10(fMiddle);	// Get power of 10 for the middle point of the limits in absolute value
    if(iExponent <= -13)
        return -15;	// Fento
    if(iExponent >= 9)
        return 9;	// Tera

    switch(iExponent)
    {
        case -12:
        case -11:
        case -10:
            return 12;	// 'n'
        case -9:
        case -8:
        case -7:
            return 9;	// 'p'
        case -6:
        case -5:
        case -4:
            return 6;	// 'u'
        case -1:
        case -2:
        case -3:
            return 3;	// 'm'

        default:
        case 0:
        case 1:
        case 2:
            return 0;

        case 3:
        case 4:
        case 5:
            return -3;	// K
        case 6:
        case 7:
        case 8:
            return -6;	// M
    }
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from CSM data parsed
//////////////////////////////////////////////////////////////////////
bool CGCSM2toSTDF::WriteStdfFile(QTextStream *hCsmFile, const char *strFileNameSTDF)
{
    // now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open((char*)strFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        // Failed importing CSM file into STDF database
        m_iLastError = errWriteSTDF;

        // Convertion failed.
        return false;
    }

    // Clear variables
    m_strWaferID = "";	// clar wafer ID

    // Read first data line
    QString strString;
    QStringList	strCells;
    strString = ReadLine(*hCsmFile);
    strCells = strString.split(",",QString::KeepEmptyParts);
    // Check if have the good count
    if(strCells.count() <= 6)
    {
        m_iLastError = errInvalidFormatLowInRows;

        // Convertion failed.
        return false;
    }

    m_strProductID = strCells[0];
    m_strLotID = strCells[1];
    m_strSubLotID = strCells[3];	// Split lot

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
    StdfFile.WriteWord(m_iBurninTime);			// burn_tim
    StdfFile.WriteByte((BYTE) ' ');				// cmod_cod
    StdfFile.WriteString(m_strLotID.toLatin1().constData());			// Lot ID
    StdfFile.WriteString(m_strProductID.toLatin1().constData());		// Part Type / Product ID
    StdfFile.WriteString(m_strTesterName.toLatin1().constData());	// Node name
    StdfFile.WriteString(m_strTesterType.toLatin1().constData());	// Tester Type
    StdfFile.WriteString(m_strProgramName.toLatin1().constData());	// Job name
    StdfFile.WriteString(m_strProgramRev.toLatin1().constData());	// Job rev
    StdfFile.WriteString(m_strSubLotID.toLatin1().constData());		// sublot-id
    StdfFile.WriteString(m_strOperator.toLatin1().constData());		// operator
    StdfFile.WriteString(m_strExecType.toLatin1().constData());		// exec-type
    StdfFile.WriteString(m_strExecRev.toLatin1().constData());		// exe-ver
    StdfFile.WriteString(m_strTestCode.toLatin1().constData());		// test-cod
    StdfFile.WriteString(m_strTemperature.toLatin1().constData());	// test-temperature
    // Construct custom Galaxy USER_TXT
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ETEST;
    strUserTxt += ":CSMC";
    StdfFile.WriteString(strUserTxt.toLatin1().constData());		// user-txt
    StdfFile.WriteString("");								// aux-file
    StdfFile.WriteString(m_strPackageType.toLatin1().constData());	// package-type
    StdfFile.WriteString(m_strFamilyID.toLatin1().constData());		// familyID
    StdfFile.WriteString("");								// Date-code
    StdfFile.WriteString(m_strFacilityID.toLatin1().constData());	// Facility-ID
    StdfFile.WriteString(m_strFloorID.toLatin1().constData());		// FloorID
    StdfFile.WriteString(m_strProcessID.toLatin1().constData());		// ProcessID
    StdfFile.WriteString(m_strFrequencyStep.toLatin1().constData());	// Frequency/Step
    StdfFile.WriteRecord();

    // Write Test results for each line read.
    QString			strSection;
    float			fValue;				// Used for readng floating point numbers.
    unsigned		uIndex;				// Loop index
    BYTE			bData;
    WORD			wSoftBin,wHardBin;
    int				iDieX,iDieY,iSite,iExecTime;
    long			iTotalGoodBin,iTotalGoodBinInWafer,iTotalFailBin;
    long			iTotalTests,iPartNumber=0;
    unsigned long	lTestNumber;
    bool			bStatus,bPassStatus;
    int				iRes_scale;

    // Reset counters
    iTotalGoodBin=iTotalGoodBinInWafer=iTotalFailBin=0;

    // Write all Parameters read on this wafer.: PTR....PTR, PRR
    do
    {
        // Ignore empty lines
        if(strString.isEmpty() == false)
        {
            // If new wafer#, close previous WIR/WRR
            if(m_strWaferID != strCells[2])
            {
                // Check if FIRST WIR to write (in which case no WRR to write!)
                if(m_strWaferID.isEmpty() == false)
                {
                    RecordReadInfo.iRecordType = 2;
                    RecordReadInfo.iRecordSubType = 20;
                    StdfFile.WriteHeader(&RecordReadInfo);
                    StdfFile.WriteByte(1);						// Test Head
                    StdfFile.WriteByte(255);					// Test sites = ALL
                    StdfFile.WriteDword(time(NULL));			// File finish-time.
                    StdfFile.WriteDword(iPartNumber);			// Total Parts tested
                    StdfFile.WriteDword(0);						// Total Parts re-tested
                    StdfFile.WriteDword(0);						// Total Parts aborted
                    StdfFile.WriteDword(iTotalGoodBinInWafer);	// Total GOOD Parts
                    StdfFile.WriteRecord();
                }

                // WIR
                m_strWaferID = strCells[2];
                RecordReadInfo.iRecordType = 2;
                RecordReadInfo.iRecordSubType = 10;
                StdfFile.WriteHeader(&RecordReadInfo);
                StdfFile.WriteByte(1);				// head#
                StdfFile.WriteByte(255);			// group# (all groups)
                StdfFile.WriteDword(m_lStartTime);	// Setup time
                StdfFile.WriteString(m_strWaferID.toLatin1().constData());	// WaferID.
                StdfFile.WriteRecord();

                // Reset counters
                iPartNumber=0;
                iTotalGoodBinInWafer=0;

            }
            // Part number
            iPartNumber++;

            // Pass/Fail flag.
            bPassStatus = true;

            // Reset counters
            iTotalTests = 0;

            // If CSM file include binning,Die location data, extract them...
            // If CSM file doesn't include Binning,Die location info...use defaults.
            iSite = strCells[4].toInt();
            iDieX = strCells[5].toInt();
            iDieY = strCells[6].toInt();
            iExecTime = 0;
            wSoftBin = 65535;
            wHardBin = 0;
            uIndex = m_lStartParsingOffset;


            // Write PIR
            RecordReadInfo.iRecordType = 5;
            RecordReadInfo.iRecordSubType = 10;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(1);			// Test head
            StdfFile.WriteByte(iSite);		// Tester site#
            StdfFile.WriteRecord();

            // Read Parameter results for this record
            while (uIndex < m_uTotalParameters &&
                   uIndex + 1 < (unsigned int) strCells.count()) {
                fValue = strCells[uIndex+1].trimmed().toFloat(&bStatus);
                if(bStatus == true)
                {
                    // Valid test result...write the PTR
                    iTotalTests++;

                    RecordReadInfo.iRecordType = 15;
                    RecordReadInfo.iRecordSubType = 10;
                    StdfFile.WriteHeader(&RecordReadInfo);
                    // Compute Test# (add user-defined offset)
                    if((long)m_pCGCsm2Parameter[uIndex].lTestNumber < 0)
                    {
                        lTestNumber = (long) m_pFullCsmParametersList.indexOf(m_pCGCsm2Parameter[uIndex].strName);
                        lTestNumber += GEX_TESTNBR_OFFSET_CSMC;		// Test# offset
                        m_pCGCsm2Parameter[uIndex].lTestNumber = lTestNumber;	// Save Test number for future runs.
                    }
                    else
                        lTestNumber = m_pCGCsm2Parameter[uIndex].lTestNumber;

                    StdfFile.WriteDword(lTestNumber);			// Test Number
                    StdfFile.WriteByte(1);						// Test head

                    // Write Site# (if known, or 1 otherwise)
                    StdfFile.WriteByte(iSite);				// Tester site#

                    if(((m_pCGCsm2Parameter[uIndex].bValidLowLimit==true) && (fValue < m_pCGCsm2Parameter[uIndex].fLowLimit)) ||
                       ((m_pCGCsm2Parameter[uIndex].bValidHighLimit==true) && (fValue > m_pCGCsm2Parameter[uIndex].fHighLimit)))
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

                    // Write static part (limits + test name) ?
                    if(m_pCGCsm2Parameter[uIndex].bStaticHeaderWritten == false)
                    {
                        StdfFile.WriteString(m_pCGCsm2Parameter[uIndex].strName.toLatin1().constData());	// TEST_TXT
                        StdfFile.WriteString("");							// ALARM_ID
                        bData = 2;	// Valid data.
                        if(m_pCGCsm2Parameter[uIndex].bValidLowLimit==false)
                            bData |=0x40;
                        if(m_pCGCsm2Parameter[uIndex].bValidHighLimit==false)
                            bData |=0x80;
                        if(m_pCGCsm2Parameter[uIndex].bValidLowSpecLimit==false)
                            bData |=0x4;
                        if(m_pCGCsm2Parameter[uIndex].bValidHighSpecLimit==false)
                            bData |=0x8;
                        StdfFile.WriteByte(bData);							// OPT_FLAG
                        iRes_scale = ResultDisplayScaleFactor(uIndex);
                        StdfFile.WriteByte(iRes_scale);						// RES_SCALE
                        StdfFile.WriteByte(0);								// LLM_SCALE
                        StdfFile.WriteByte(0);								// HLM_SCALE
                        StdfFile.WriteFloat(m_pCGCsm2Parameter[uIndex].fLowLimit);		// LOW Test Limit
                        StdfFile.WriteFloat(m_pCGCsm2Parameter[uIndex].fHighLimit);		// HIGH Test Limit
                        StdfFile.WriteString(m_pCGCsm2Parameter[uIndex].strUnits.toLatin1().constData());	// Units
                        m_pCGCsm2Parameter[uIndex].bStaticHeaderWritten = true;
                        StdfFile.WriteString("");	// C_RESFMT	: ANSI C result format string
                        StdfFile.WriteString("");	// C_LLMFMT	: ANSI C low limit format string
                        StdfFile.WriteString("");	// C_HLMFMT	: ANSI C high limit format string
                        StdfFile.WriteFloat(m_pCGCsm2Parameter[uIndex].fLowSpecLimit);		// LOW Spec Limit
                        StdfFile.WriteFloat(m_pCGCsm2Parameter[uIndex].fHighSpecLimit);		// HIGH Spec Limit
                    }
                    StdfFile.WriteRecord();
                }	// Valid test result

                // Move to next parameter column to read
                uIndex++;
            };	// Read all results on line

            // Write PRR
            RecordReadInfo.iRecordType = 5;
            RecordReadInfo.iRecordSubType = 20;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(1);			// Test head
            // Tester site#
            StdfFile.WriteByte(iSite);	// Tester site#
            if(bPassStatus == true)
            {
                StdfFile.WriteByte(0);				// PART_FLG : PASSED
                // Create a binning result unless it is already in the CSM file...
                wSoftBin = wHardBin = 1;
                iTotalGoodBinInWafer++;
                iTotalGoodBin++;
            }
            else
            {
                StdfFile.WriteByte(8);				// PART_FLG : FAILED
                // Create a binning result unless it is already in the CSM file...
                wSoftBin = wHardBin = 0;
                iTotalFailBin++;
            }

            // BG 23/07/2004 : should we force a nb of tests for the wafermap to be correct????
            StdfFile.WriteWord((WORD)iTotalTests);	// NUM_TEST
            StdfFile.WriteWord(wHardBin);           // HARD_BIN
            StdfFile.WriteWord(wSoftBin);           // SOFT_BIN
            StdfFile.WriteWord(iDieX);				// X_COORD
            StdfFile.WriteWord(iDieY);				// Y_COORD
            StdfFile.WriteDword(iExecTime);			// Testing time (if known, otherwise: 0)
            strSection = QString::number(iPartNumber);	// Build PartID
            StdfFile.WriteString(strSection.toLatin1().constData());	// PART_ID
            StdfFile.WriteRecord();
        }

        // Read line
        strString = ReadLine(*hCsmFile);
        // Split line
        strCells = strString.split(",",QString::KeepEmptyParts);
        // Check if have the good count
        if(strCells.count() <= 6)
        {
            m_iLastError = errInvalidFormatLowInRows;

            // Close STDF file.
            StdfFile.Close();

            // Convertion failed.
            return false;
        }

    }
    while(!strString.isNull());	// Read all lines in file

    // WRR (if WaferID was defined)
    if(m_strWaferID.isEmpty() == false)
    {
        RecordReadInfo.iRecordType = 2;
        RecordReadInfo.iRecordSubType = 20;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);						// Test Head
        StdfFile.WriteByte(255);					// Test sites = ALL
        StdfFile.WriteDword(time(NULL));			// File finish-time.
        StdfFile.WriteDword(iPartNumber);			// Total Parts tested
        StdfFile.WriteDword(0);						// Total Parts re-tested
        StdfFile.WriteDword(0);						// Total Parts aborted
        StdfFile.WriteDword(iTotalGoodBin);			// Total GOOD Parts
        StdfFile.WriteRecord();
    }

    // If no binning data given in the CSM file, write our extrapolation.
    // Write SBR Bin0 (FAIL)
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 50;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(255);					// Test Head = ALL
    StdfFile.WriteByte(255);					// Test sites = ALL
    StdfFile.WriteWord(0);						// SBIN = 0
    StdfFile.WriteDword(iTotalFailBin);			// Total Bins
    StdfFile.WriteByte('F');
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
    StdfFile.WriteRecord();

    // Write PCR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 30;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);						// Test Head
    StdfFile.WriteByte(255);					// Test sites = ALL
    StdfFile.WriteDword(iPartNumber);			// Total Parts tested
    StdfFile.WriteDword(0);						// Total Parts re-tested
    StdfFile.WriteDword(0);						// Total Parts aborted
    StdfFile.WriteDword(iTotalGoodBin);			// Total GOOD Parts
    StdfFile.WriteRecord();

    // Write MRR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteDword(time(NULL));			// File finish-time.
    StdfFile.WriteRecord();

    // Close STDF file.
    StdfFile.Close();

    // Success
    return true;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' CSM type 2 file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGCSM2toSTDF::Convert(const char *CsmFileName, const char *strFileNameSTDF)
{
    // No erro (default)
    m_iLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(CsmFileName);
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
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(CsmFileName).fileName()+"...");
            GexScriptStatusLabel->show();
        }
    }
    QCoreApplication::processEvents();


    if(ReadCsmFile(CsmFileName,strFileNameSTDF) == true)
    {
        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if((GexProgressBar != NULL)
        && bHideProgressAfter)
            GexProgressBar->hide();

        if((GexScriptStatusLabel != NULL)
        && bHideLabelAfter)
            GexScriptStatusLabel->hide();

        return true;	// Convertion successful
    }

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    if((GexProgressBar != NULL)
    && bHideProgressAfter)
        GexProgressBar->hide();

    if((GexScriptStatusLabel != NULL)
    && bHideLabelAfter)
        GexScriptStatusLabel->hide();

    return true;
}

//////////////////////////////////////////////////////////////////////
// Read line : skip empty line
//////////////////////////////////////////////////////////////////////
QString CGCSM2toSTDF::ReadLine(QTextStream& hFile)
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

