//////////////////////////////////////////////////////////////////////
// import_pcm_magnachip_column.cpp: Convert a .dat PCM_MAGNACHIP (ETEST)
// file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#include <time.h>
#include <qmath.h>

#ifdef _WIN32
#include "windows.h"    // For 'GetWindowsDirectoryA' declaration
#endif

#include <qfileinfo.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>

#include "engine.h"
#include "import_pcm_magnachip_column.h"
#include "import_constants.h"
#include "gqtl_global.h"

// File format: (size delimited)
//
//#FILE_NAME    : 2013-jan-20-su48909.dat
//#Starttime    : 2013-JAN-20 04:28
//#LotNumber    : SU48909
//#CustomerCode :  CL1583B0
//#TotalWafers  : 25
//#SitePerWafer : 5
//#TP_Name&Rev  : Rev0.0_
//#PCM_Spec&Rev : CIRRUS_HL18GF_
//#PARMs        : 59
//#Operator     : c2asic
//#DeviceName   : S08D6CS18417B-SM
//#Process      : C2product
//#FAB          : F-4
//#Testname     : cirrus
//#LimitsFile   : CIRRUS_HL18GF_
//#StationNum   : 1
//
//ParaName                 VTl0N2                   VTlN2
//Description              1.8V N                   1.8V N
//Size                     [10/10]                  [10/0.18]
//SpecLow                  0.280                    0.360
//Target                   0.350                    0.430
//SpecHigh                 0.410                    0.500
//Unit                     V                        V
//WF,Site                  ======                   ======
//01,B                     0.343                    0.402
//01,C                     0.339                    0.406
//01,L                     0.341                    0.414
//01,R                     0.340                    0.400
//01,T                     0.338                    0.404
//==,====                  ======                   ======
//02,B                     0.344                    0.399


// main.cpp
extern QLabel           *GexScriptStatusLabel;  // Handle to script status text in status bar
extern QProgressBar     *GexProgressBar;        // Handle to progress bar in status bar

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGPCM_MAGNACHIP_COLUMNtoSTDF::CGPCM_MAGNACHIP_COLUMNtoSTDF()
{
    // Default: PCM_MAGNACHIP_COLUMN parameter list on disk includes all known PCM_MAGNACHIP_COLUMN parameters...
    m_bNewPcmMagnachipParameterFound = false;

    m_pCGPcmMagnachipParameter = NULL;
    m_lStartTime = 0;
    m_iStation = 1;
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CGPCM_MAGNACHIP_COLUMNtoSTDF::~CGPCM_MAGNACHIP_COLUMNtoSTDF()
{
    // Destroy list of Parameters tables.
    if(m_pCGPcmMagnachipParameter!=NULL)
        delete [] m_pCGPcmMagnachipParameter;
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGPCM_MAGNACHIP_COLUMNtoSTDF::GetLastError()
{
    m_strLastError = "Import PCM (Magnachip): ";

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
        m_strLastError += "Invalid file format, Specification Limits not found";
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
// Load PCM_MAGNACHIP_COLUMN Parameter table from DISK
//////////////////////////////////////////////////////////////////////
void CGPCM_MAGNACHIP_COLUMNtoSTDF::LoadParameterIndexTable(void)
{
    QString strPcmMagnachipTableFile;
    QString strString;

    strPcmMagnachipTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
    strPcmMagnachipTableFile += GEX_PCM_PARAMETERS;

    // Open PCM_MAGNACHIP_COLUMN Parameter table file
    QFile f( strPcmMagnachipTableFile );
    if(!f.open( QIODevice::ReadOnly ))
        return;

    // Assign file I/O stream
    QTextStream hPcmMagnachipTableFile(&f);

    // Skip comment lines
    do
    {
        strString = hPcmMagnachipTableFile.readLine();
    }
    while((strString.indexOf("----------------------") < 0) && (hPcmMagnachipTableFile.atEnd() == false));

    // Read lines
    m_pFullPcmMagnachipParametersList.clear();
    strString = hPcmMagnachipTableFile.readLine();
    while (strString.isNull() == false)
    {
        // Save Parameter name in list
        m_pFullPcmMagnachipParametersList.append(strString);
        // Read next line
        strString = hPcmMagnachipTableFile.readLine();
    };

    // Close file
    f.close();
}

//////////////////////////////////////////////////////////////////////
// Save PCM_MAGNACHIP_COLUMN Parameter table to DISK
//////////////////////////////////////////////////////////////////////
void CGPCM_MAGNACHIP_COLUMNtoSTDF::DumpParameterIndexTable(void)
{
    QString     strPcmMagnachipTableFile;

    strPcmMagnachipTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
    strPcmMagnachipTableFile += GEX_PCM_PARAMETERS;

    // Open PCM_MAGNACHIP_COLUMN Parameter table file
    QFile f( strPcmMagnachipTableFile );
    if(!f.open( QIODevice::WriteOnly ))
        return;

    // Assign file I/O stream
    QTextStream hPcmMagnachipTableFile(&f);

    // First few lines are comments:
    hPcmMagnachipTableFile << "############################################################" << endl;
    hPcmMagnachipTableFile << "# DO NOT EDIT THIS FILE!" << endl;
    hPcmMagnachipTableFile << "# Quantix Examinator: PCM Parameters detected" << endl;
    hPcmMagnachipTableFile << "# www.mentor.com" << endl;
    hPcmMagnachipTableFile << "# Quantix Examinator reads and writes into this file..." << endl;
    hPcmMagnachipTableFile << "-----------------------------------------------------------" << endl;

    // Write lines
    // m_pFullPcmMagnachipParametersList.sort();
    for(int nIndex = 0; nIndex < m_pFullPcmMagnachipParametersList.count(); nIndex++)
    {
        // Write line
        hPcmMagnachipTableFile << m_pFullPcmMagnachipParametersList[nIndex] << endl;
    };

    // Close file
    f.close();
}

//////////////////////////////////////////////////////////////////////
// If Examinator doesn't have this PCM_MAGNACHIP_COLUMN parameter in his dictionnary, have it added.
//////////////////////////////////////////////////////////////////////
void CGPCM_MAGNACHIP_COLUMNtoSTDF::UpdateParameterIndexTable(QString strParamName)
{
    // Check if the table is empty...if so, load it from disk first!
    if(m_pFullPcmMagnachipParametersList.isEmpty() == true)
    {
        // Load PCM_MAGNACHIP_COLUMN parameter table from disk...
        LoadParameterIndexTable();
    }

    // Check if Parameter name already in table...if not, add it to the list
    // the new full list will be dumped to the disk at the end.
    if(m_pFullPcmMagnachipParametersList.indexOf(strParamName) < 0)
    {
        // Update list
        m_pFullPcmMagnachipParametersList.append(strParamName);

        // Set flag to force the current PCM_MAGNACHIP_COLUMN table to be updated on disk
        m_bNewPcmMagnachipParameterFound = true;
    }
}

//////////////////////////////////////////////////////////////////////
// Normalize test limits when writing into PTR.
//////////////////////////////////////////////////////////////////////
void CGPCM_MAGNACHIP_COLUMNtoSTDF::NormalizeLimits(int iIndex)
{
    int value_scale=0;
    QString strUnits = m_pCGPcmMagnachipParameter[iIndex].strUnits;
    if((strUnits.length() <= 1) || (strUnits.contains("/")))
    {
        // units too short to include a prefix, then keep it 'as-is'
        m_pCGPcmMagnachipParameter[iIndex].scale = 0;
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
    m_pCGPcmMagnachipParameter[iIndex].fLowLimit *= GS_POW(10.0,value_scale);
    m_pCGPcmMagnachipParameter[iIndex].fHighLimit *= GS_POW(10.0,value_scale);
    m_pCGPcmMagnachipParameter[iIndex].scale = value_scale;
    if(value_scale)
        strUnits = strUnits.mid(1); // Take all characters after the prefix.
    m_pCGPcmMagnachipParameter[iIndex].strUnits = strUnits;
}


//////////////////////////////////////////////////////////////////////
// Check if File is compatible with PCM_MAGNACHIP_COLUMN format
//////////////////////////////////////////////////////////////////////
bool CGPCM_MAGNACHIP_COLUMNtoSTDF::IsCompatible(const char *szFileName)
{
    QString strString;
    QString strSection;
    bool    bIsCompatible = false;

    // Open hCsmFile file
    QFile f( szFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening ASL1000 file
        return false;
    }
    // Assign file I/O stream
    QTextStream hPcmMagnachipFile(&f);

    // extract PCM_WAT information
    //#FILE_NAME    : 2013-jan-20-su48909.dat
    //#Starttime    : 2013-JAN-20 04:28
    //#LotNumber    : SU48909
    //#CustomerCode :  CL1583B0
    //#TotalWafers  : 25
    //#SitePerWafer : 5
    //#TP_Name&Rev  : Rev0.0_
    //#PCM_Spec&Rev : CIRRUS_HL18GF_
    //#PARMs        : 59
    //#Operator     : c2asic
    //#DeviceName   : S08D6CS18417B-SM
    //#Process      : C2product
    //#FAB          : F-4
    //#Testname     : cirrus
    //#LimitsFile   : CIRRUS_HL18GF_
    //#StationNum   : 1
    //
    //ParaName                 VTl0N2                   VTlN2
    //Description              1.8V N                   1.8V N
    //Size                     [10/10]                  [10/0.18]
    //SpecLow                  0.280                    0.360
    //Target                   0.350                    0.430
    //SpecHigh                 0.410                    0.500
    //Unit                     V                        V
    //WF,Site                  ======                   ======

    while(hPcmMagnachipFile.atEnd() == false)
    {
        strString = hPcmMagnachipFile.readLine();
        if(strString.simplified().isEmpty())
            continue;

        if(strString.startsWith("#"))
            continue;
        else
            strSection = strString.mid(0,25).simplified().toUpper();

        //ParaName                 VTl0N2                   VTlN2
        //Description              1.8V N                   1.8V N
        //Size                     [10/10]                  [10/0.18]
        if(strSection=="PARANAME")
        {
            strString = hPcmMagnachipFile.readLine();
            strSection = strString.mid(0,25).simplified().toUpper();
            if(strSection=="DESCRIPTION")
            {
                strString = hPcmMagnachipFile.readLine();
                strSection = strString.mid(0,25).simplified().toUpper();
                if(strSection=="SIZE")
                {
                    bIsCompatible = true;
                    break;
                }
            }
        }

        // Incorrect header...this is not a PCM_MAGNACHIP_COLUMN file!
        // Close file
        bIsCompatible = false;
        break;

    }

    // Close file
    f.close();

    return bIsCompatible;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the PCM_MAGNACHIP_COLUMN file
//////////////////////////////////////////////////////////////////////
bool CGPCM_MAGNACHIP_COLUMNtoSTDF::ReadPcmMagnachipFile(const char *PcmMagnachipFileName,const char *strFileNameSTDF)
{
    QString strString;
    QString strSection;
    QString strValue;
    bool    bStatus;
    int     iIndex;     // Loop index

    // Open CSV file
    QFile f( PcmMagnachipFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening PCM_MAGNACHIP_COLUMN file
        m_iLastError = errOpenFail;

        // Convertion failed.
        return false;
    }

    // Assign file I/O stream
    QTextStream hPcmMagnachipFile(&f);

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    iProgressStep = 0;
    iNextFilePos = 0;
    iFileSize = f.size() + 1;
    QCoreApplication::processEvents();

    // extract PCM_WAT information
    //#FILE_NAME    : 2013-jan-20-su48909.dat
    //#Starttime    : 2013-JAN-20 04:28
    //#LotNumber    : SU48909
    //#CustomerCode :  CL1583B0
    //#TotalWafers  : 25
    //#SitePerWafer : 5
    //#TP_Name&Rev  : Rev0.0_
    //#PCM_Spec&Rev : CIRRUS_HL18GF_
    //#PARMs        : 59
    //#Operator     : c2asic
    //#DeviceName   : S08D6CS18417B-SM
    //#Process      : C2product
    //#FAB          : F-4
    //#Testname     : cirrus
    //#LimitsFile   : CIRRUS_HL18GF_
    //#StationNum   : 1
    while(hPcmMagnachipFile.atEnd() == false)
    {
        strString = ReadLine(hPcmMagnachipFile);
        if(strString.simplified().remove(" ").isEmpty())
            continue;

        if(strString.startsWith("#"))
        {
            strString = strString.mid(1);
            strSection = strString.section(":",0,0).simplified().toUpper();
            strValue = strString.section(":",1).simplified();
        }
        else
            break;

        if(strValue == ".")
            strValue = "";


        if(strSection=="LOTNUMBER")
        {
            m_strLotId = strValue.simplified();
        }
        else if(strSection=="TP_NAME&REV")
        {
            m_strProgramName = strValue.simplified();
        }
        else if(strSection=="PCM_SPEC&REV")
        {
            m_strSpecificationName = strValue.simplified();
        }
        else if(strSection=="OPERATOR")
        {
            m_strOperatorName = strValue.simplified();
        }
        else if(strSection=="DEVICENAME")
        {
            m_strProductId = strValue.simplified();
        }
        else if(strSection=="PROCESS")
        {
            m_strProcessId = strValue;
        }
        else if(strSection=="FAB")
        {
            m_strFacilityId = strValue.simplified();
        }
        else if(strSection=="TESTNAME")
        {
            m_strTesterName = strValue.simplified();
        }
        else if(strSection=="LIMITSFILE")
        {
            m_strAuxFile = strValue.simplified();
        }
        else if(strSection=="STATIONNUM")
        {
            bool bNum;
            m_iStation = strValue.simplified().mid(0,1).toInt(&bNum);
            if(!bNum)
                m_iStation = 1;
        }
        else if(strSection=="STARTTIME")
        {
            //#Starttime    : 2013-JAN-20 04:28
            if(strValue.count(":")==1)
                strValue = strValue.simplified() + ":00";
            QString lDate = strValue.section(' ',0,0).simplified();
            QString lTime = strValue.section(' ',1).simplified();
            int lMonth;
            for(lMonth=1; lMonth<=12; ++lMonth)
            {
                if(QDate::shortMonthName(lMonth).mid(0,3).toUpper()
                        == lDate.section("-",1,1).simplified().toUpper())
                    break;
            }

            int lYear = lDate.section("-",0,0).simplified().toInt();
            int lDay = lDate.section("-",2,2).simplified().toInt();
            QDate clDate(lYear,lMonth,lDay);
            QTime clTime = QTime::fromString(lTime);
            QDateTime clDateTime(clDate,clTime);
            clDateTime.setTimeSpec(Qt::UTC);
            if(!clDateTime.isValid())
                clDateTime = QDateTime::fromString(strValue);
            if(!clDateTime.isValid())
                clDateTime = QDateTime::fromString(strValue,Qt::ISODate);
            if(clDateTime.isValid())
                m_lStartTime = clDateTime.toTime_t();
        }
    }

    //ParaName                 VTl0N2                   VTlN2
    //Description              1.8V N                   1.8V N
    //Size                     [10/10]                  [10/0.18]
    //SpecLow                  0.280                    0.360
    //Target                   0.350                    0.430
    //SpecHigh                 0.410                    0.500
    //Unit                     V                        V
    //WF,Site                  ======                   ======

    // Goto Parameter description
    if(strString.isEmpty())
        strString = ReadLine(hPcmMagnachipFile);
    if(!strString.startsWith("ParaName",Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a valid PCM_MAGNACHIP_COLUMN file!
        m_iLastError = errInvalidFormat;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    // have found the parameter description
    // Extract the N column names
    QString strName,strDescription,strSize;
    QStringList lstNames, lstDescritions,lstSizes;

    lstNames = SplitLine(strString);
    // Count the number of parameters specified in the line
    m_iTotalParameters = lstNames.count();

    // If no parameter specified...ignore!
    if(m_iTotalParameters <= 0)
    {
        // Incorrect header...this is not a valid PCM_MAGNACHIP_COLUMN file!
        m_iLastError = errInvalidFormat;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    // Allocate the buffer to hold the N parameters & results.
    m_pCGPcmMagnachipParameter = new CGPcmMagnachipParameter[m_iTotalParameters];   // List of parameters

    lstDescritions = SplitLine(ReadLine(hPcmMagnachipFile));
    lstSizes = SplitLine(ReadLine(hPcmMagnachipFile));

    // Ignore the first column
    for(iIndex=1;iIndex<m_iTotalParameters;iIndex++)
    {
        strName = lstNames[iIndex].trimmed();
        // Add the Description
        strDescription = "";
        if(lstDescritions.count() > iIndex)
            strDescription = lstDescritions[iIndex].trimmed();
        if((!strDescription.isEmpty()) && (strDescription != ".") && (strName != strDescription))
            strName += " - " + strDescription;
        // Add the Size
        strSize = "";
        if(lstSizes.count() > iIndex)
            strSize = lstSizes[iIndex].trimmed();
        if((!strSize.isEmpty()) && (strSize != "."))
            strName += " - " + strSize;

        m_pCGPcmMagnachipParameter[iIndex].strName = strName;
        UpdateParameterIndexTable(strName);     // Update Parameter master list if needed.
        m_pCGPcmMagnachipParameter[iIndex].bStaticHeaderWritten = false;
    }

    //SpecLow                  0.280                    0.360
    //Target                   0.350                    0.430
    //SpecHigh                 0.410                    0.500
    //Unit                     V                        V
    int iLimits =0;
    QStringList lstSections;
    while(hPcmMagnachipFile.atEnd() == false)
    {
        strSection = "";
        lstSections = SplitLine(ReadLine(hPcmMagnachipFile));
        if(!lstSections.isEmpty())
            strSection = lstSections.first();
        if(strSection.startsWith("Unit", Qt::CaseInsensitive))
        {
            // found the UNIT limits

            // Extract the N column toUpper Limits
            for(iIndex=1;iIndex<m_iTotalParameters;iIndex++)
            {
                strSection = "";
                if(lstSections.count() > iIndex)
                    strSection = lstSections[iIndex].simplified();
                m_pCGPcmMagnachipParameter[iIndex].strUnits = strSection;
            }

        }
        else if(strSection.startsWith("SPECHIGH", Qt::CaseInsensitive))
        {
            // found the HIGH limits
            iLimits |= 1;

            // Extract the N column toUpper Limits
            for(iIndex=1;iIndex<m_iTotalParameters;iIndex++)
            {
                strSection = "";
                if(lstSections.count() > iIndex)
                    strSection = lstSections[iIndex];
                m_pCGPcmMagnachipParameter[iIndex].fHighLimit = strSection.simplified().toFloat(&bStatus);
                m_pCGPcmMagnachipParameter[iIndex].bValidHighLimit = bStatus;
            }
        }
        else if(strSection.startsWith("SPECLOW", Qt::CaseInsensitive))
        {
            // found the Low limits
            iLimits |= 2;

            // Extract the N column Lower Limits
            for(iIndex=1;iIndex<m_iTotalParameters;iIndex++)
            {
                strSection = "";
                if(lstSections.count() > iIndex)
                    strSection = lstSections[iIndex];
                m_pCGPcmMagnachipParameter[iIndex].fLowLimit = strSection.simplified().toFloat(&bStatus);
                m_pCGPcmMagnachipParameter[iIndex].bValidLowLimit = bStatus;
            }
        }
        else if(strSection.startsWith("WF,Site", Qt::CaseInsensitive))
            break;
        else
            continue;
    }

    if(iLimits != 3)
    {
        // Incorrect header...this is not a valid PCM_MAGNACHIP_COLUMN file!: we didn't find the limits!
        m_iLastError = errNoLimitsFound;

        // Convertion failed.
        return false;
    }

    // Normalize all Limits
    for(iIndex=1;iIndex<m_iTotalParameters;iIndex++)
        NormalizeLimits(iIndex);

    // Loop reading file until end is reached & generate STDF file dynamically.
    bStatus = WriteStdfFile(&hPcmMagnachipFile,strFileNameSTDF);
    if(!bStatus)
        QFile::remove(strFileNameSTDF);

    // All PCM_MAGNACHIP_COLUMN file read...check if need to update the PCM_MAGNACHIP_COLUMN Parameter list on disk?
    if(bStatus && (m_bNewPcmMagnachipParameterFound == true))
        DumpParameterIndexTable();

    // Close file
    f.close();

    // Success parsing PCM_MAGNACHIP_COLUMN file
    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from PCM_MAGNACHIP_COLUMN data parsed
//////////////////////////////////////////////////////////////////////
bool CGPCM_MAGNACHIP_COLUMNtoSTDF::WriteStdfFile(QTextStream *hPcmMagnachipFile, const char *strFileNameSTDF)
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
    StdfFile.WriteByte(1);                  // SUN CPU type
    StdfFile.WriteByte(4);                  // STDF V4
    StdfFile.WriteRecord();

    if(m_lStartTime <= 0)
        m_lStartTime = QDateTime::currentDateTime().toTime_t();

    // Write MIR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 10;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteDword(m_lStartTime);      // Setup time
    StdfFile.WriteDword(m_lStartTime);      // Start time
    StdfFile.WriteByte(m_iStation);         // Station
    StdfFile.WriteByte((BYTE) 'P');         // Test Mode = PRODUCTION
    StdfFile.WriteByte((BYTE) ' ');         // rtst_cod
    StdfFile.WriteByte((BYTE) ' ');         // prot_cod
    StdfFile.WriteWord(65535);              // burn_tim
    StdfFile.WriteByte((BYTE) ' ');         // cmod_cod
    StdfFile.WriteString(m_strLotId.toLatin1().constData());        // Lot ID
    StdfFile.WriteString(m_strProductId.toLatin1().constData());    // Part Type / Product ID
    StdfFile.WriteString(m_strTesterName.toLatin1().constData());   // Node name
    StdfFile.WriteString("");               // Tester Type
    StdfFile.WriteString(m_strProgramName.toLatin1().constData());  // Job name
    StdfFile.WriteString("");               // Job rev
    StdfFile.WriteString("");               // sublot-id
    StdfFile.WriteString(m_strOperatorName.toLatin1().constData()); // operator
    StdfFile.WriteString("");               // exec-type
    StdfFile.WriteString("");               // exe-ver
    StdfFile.WriteString("WAFER");          // test-cod
    StdfFile.WriteString("");               // test-temperature
    // Construct custom Galaxy USER_TXT
    QString strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ETEST;
    strUserTxt += ":PCM MAGNACHIP";
    StdfFile.WriteString((char *)strUserTxt.toLatin1().constData());    // user-txt
    StdfFile.WriteString(m_strAuxFile.toLatin1().constData());          // aux-file
    StdfFile.WriteString("");               // package-type
    StdfFile.WriteString(m_strProductId.toLatin1().constData());        // familyID
    StdfFile.WriteString("");               // Date-code
    StdfFile.WriteString(m_strFacilityId.toLatin1().constData());       // Facility-ID
    StdfFile.WriteString("");               // FloorID
    StdfFile.WriteString(m_strProcessId.toLatin1().constData());        // ProcessID
    StdfFile.WriteString("");               // OperFrq
    StdfFile.WriteString(m_strSpecificationName.toLatin1().constData());// SpecName
    StdfFile.WriteRecord();

    // Write Test results for each line read.
    QString strString;
    QString strSection;
    QString strWaferID;
    float   fValue;             // Used for readng floating point numbers.
    int     value_scale;        // Scale factor for limots & results.
    int     iIndex;             // Loop index
    int     iSiteNumber;
    QString strSiteNumber;
    BYTE    bData;
    WORD    wSoftBin,wHardBin;
    long    iTotalGoodBin,iTotalFailBin;
    long    iTestNumber,iTotalTests,iPartNumber;
    bool    bStatus,bPassStatus;

    QStringList lstSections;
    // Reset counters
    iTotalGoodBin=iTotalFailBin=0;
    iPartNumber=0;

    // Write all Parameters read on this wafer.: WIR.PIR,PTR....PTR, PRR, PIR, PTR,PTR...PRR,   ... WRR
    while(hPcmMagnachipFile->atEnd() == false)
    {

        // Part number
        iPartNumber++;

        // Read line
        lstSections = SplitLine(ReadLine(*hPcmMagnachipFile));
        if(lstSections.isEmpty())
            break;

        // Extract WaferID
        strSection = lstSections.first().simplified();

        //==,====                  ======
        if(strSection.startsWith("=="))
            continue;
        if(strSection.startsWith("WF,Site",Qt::CaseInsensitive))
            continue;

        if(strSection.section(",",0,0) != strWaferID)
        {
            // Write WRR in case we have finished to write wafer records.
            if(strWaferID.isEmpty() == false)
            {
                // WRR
                RecordReadInfo.iRecordType = 2;
                RecordReadInfo.iRecordSubType = 20;
                StdfFile.WriteHeader(&RecordReadInfo);
                StdfFile.WriteByte(1);                      // Test head
                StdfFile.WriteByte(255);                    // Tester site (all)
                StdfFile.WriteDword(m_lStartTime);          // Time of last part tested
                StdfFile.WriteDword(iTotalGoodBin+iTotalFailBin);// Parts tested: always 5
                StdfFile.WriteDword(0);                     // Parts retested
                StdfFile.WriteDword(0);                     // Parts Aborted
                StdfFile.WriteDword(iTotalGoodBin);         // Good Parts
                StdfFile.WriteDword((DWORD)-1)  ;           // Functionnal Parts
                StdfFile.WriteString(strWaferID.toLatin1().constData());// WaferID
                StdfFile.WriteRecord();
            }

            iTotalGoodBin=iTotalFailBin=0;

            // For each wafer, have to write limit in the first PTR
            for(iIndex=1;iIndex<m_iTotalParameters;iIndex++)
                m_pCGPcmMagnachipParameter[iIndex].bStaticHeaderWritten = false;

            // Write WIR of new Wafer.
            strWaferID = strSection.section(",",0,0);
            RecordReadInfo.iRecordType = 2;
            RecordReadInfo.iRecordSubType = 10;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(1);                              // Test head
            StdfFile.WriteByte(255);                            // Tester site (all)
            StdfFile.WriteDword(m_lStartTime);                  // Start time
            StdfFile.WriteString(strWaferID.toLatin1().constData());// WaferID
            StdfFile.WriteRecord();
        }

        // Reset Pass/Fail flag.
        bPassStatus = true;

        // Reset counters
        iTotalTests = 0;

        // Extract Site
        strSiteNumber = strSection.section(",",1,1).simplified();
        if(strSiteNumber.startsWith("C", Qt::CaseInsensitive))
            iSiteNumber = 1;        // Center
        else if(strSiteNumber.startsWith("B", Qt::CaseInsensitive))
            iSiteNumber = 2;        // Bottom
        else if(strSiteNumber.startsWith("L", Qt::CaseInsensitive))
            iSiteNumber = 3;        // Left
        else if(strSiteNumber.startsWith("T", Qt::CaseInsensitive))
            iSiteNumber = 4;        // Top
        else
            iSiteNumber = 5;        // Right, or other position.

        // Write PIR for parts in this Wafer site
        RecordReadInfo.iRecordType = 5;
        RecordReadInfo.iRecordSubType = 10;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);                              // Test head
        StdfFile.WriteByte(iSiteNumber);                    // Tester site
        StdfFile.WriteRecord();

        //01,B                     0.343                    0.402                    596.1
        // Read Parameter results for this record
        for(iIndex=1;iIndex<m_iTotalParameters;iIndex++)
        {
            strSection = "";
            if(lstSections.count() > iIndex)
                strSection = lstSections[iIndex];
            fValue = strSection.simplified().toFloat(&bStatus);

            // Normalize result
            value_scale = m_pCGPcmMagnachipParameter[iIndex].scale;
            fValue*= GS_POW(10.0,value_scale);

            if(bStatus == true)
            {
                // Valid test result...write the PTR
                iTotalTests++;

                RecordReadInfo.iRecordType = 15;
                RecordReadInfo.iRecordSubType = 10;
                StdfFile.WriteHeader(&RecordReadInfo);
                // Compute Test# (add user-defined offset)
                iTestNumber = (long) m_pFullPcmMagnachipParametersList.indexOf(m_pCGPcmMagnachipParameter[iIndex].strName);
                iTestNumber += GEX_TESTNBR_OFFSET_PCM;      // Test# offset
                StdfFile.WriteDword(iTestNumber);           // Test Number
                StdfFile.WriteByte(1);                      // Test head
                StdfFile.WriteByte(iSiteNumber);            // Tester site#
                if(((m_pCGPcmMagnachipParameter[iIndex].bValidLowLimit==true) && (fValue < m_pCGPcmMagnachipParameter[iIndex].fLowLimit)) ||
                        ((m_pCGPcmMagnachipParameter[iIndex].bValidHighLimit==true) && (fValue > m_pCGPcmMagnachipParameter[iIndex].fHighLimit)))
                {
                    bData = 0200;   // Test Failed
                    bPassStatus = false;
                }
                else
                {
                    bData = 0;      // Test passed
                }
                StdfFile.WriteByte(bData);                  // TEST_FLG
                bData = 0;
                if(m_pCGPcmMagnachipParameter[iIndex].bValidLowLimit==true)
                    bData = 0x40;       // not strict limit
                if(m_pCGPcmMagnachipParameter[iIndex].bValidHighLimit==true)
                    bData |= 0x80;      // not strict limit
                StdfFile.WriteByte(bData);                          // PARAM_FLG
                StdfFile.WriteFloat(fValue);                        // Test result
                if(m_pCGPcmMagnachipParameter[iIndex].bStaticHeaderWritten == false)
                {
                    StdfFile.WriteString(m_pCGPcmMagnachipParameter[iIndex].strName.toLatin1().constData());    // TEST_TXT
                    StdfFile.WriteString("");                       // ALARM_ID
                    bData = 2;      // Valid data.
                    if(m_pCGPcmMagnachipParameter[iIndex].bValidLowLimit==false)
                        bData |=0x40;
                    if(m_pCGPcmMagnachipParameter[iIndex].bValidHighLimit==false)
                        bData |=0x80;
                    StdfFile.WriteByte(bData);                      // OPT_FLAG
                    StdfFile.WriteByte(0);                          // RES_SCALE
                    StdfFile.WriteByte(-value_scale);               // LLM_SCALE
                    StdfFile.WriteByte(-value_scale);               // HLM_SCALE
                    StdfFile.WriteFloat(m_pCGPcmMagnachipParameter[iIndex].fLowLimit);      // LOW Limit
                    StdfFile.WriteFloat(m_pCGPcmMagnachipParameter[iIndex].fHighLimit);     // HIGH Limit
                    StdfFile.WriteString(m_pCGPcmMagnachipParameter[iIndex].strUnits.toLatin1().constData());   // No Units
                    m_pCGPcmMagnachipParameter[iIndex].bStaticHeaderWritten = true;
                }
                StdfFile.WriteRecord();
            }   // Valid test result
        }       // Read all results on line

        // Write PRR
        RecordReadInfo.iRecordType = 5;
        RecordReadInfo.iRecordSubType = 20;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);          // Test head
        StdfFile.WriteByte(iSiteNumber);// Tester site#:1
        if(bPassStatus == true)
        {
            StdfFile.WriteByte(0);              // PART_FLG : PASSED
            wSoftBin = wHardBin = 1;
            iTotalGoodBin++;
        }
        else
        {
            StdfFile.WriteByte(8);              // PART_FLG : FAILED
            wSoftBin = wHardBin = 0;
            iTotalFailBin++;
        }
        StdfFile.WriteWord((WORD)iTotalTests);  // NUM_TEST
        StdfFile.WriteWord(wHardBin);           // HARD_BIN
        StdfFile.WriteWord(wSoftBin);           // SOFT_BIN
        switch(iSiteNumber)
        {
        case 1: // Center
            StdfFile.WriteWord(1);          // X_COORD
            StdfFile.WriteWord(1);          // Y_COORD
            break;
        case 2: // Down
            StdfFile.WriteWord(1);          // X_COORD
            StdfFile.WriteWord(2);          // Y_COORD
            break;
        case 3: // Left
            StdfFile.WriteWord(0);          // X_COORD
            StdfFile.WriteWord(1);          // Y_COORD
            break;
        case 4: // Top
            StdfFile.WriteWord(1);          // X_COORD
            StdfFile.WriteWord(0);          // Y_COORD
            break;
        case 5: // Right
            StdfFile.WriteWord(2);          // X_COORD
            StdfFile.WriteWord(1);          // Y_COORD
            break;
        default: // More than 5 sites?....give 0,0 coordonates
            StdfFile.WriteWord(0);          // X_COORD
            StdfFile.WriteWord(0);          // Y_COORD
            break;
        }
        StdfFile.WriteDword(0);             // No testing time known...
        StdfFile.WriteString(QString::number(iPartNumber).toLatin1().constData());      // PART_ID
        StdfFile.WriteString("");           // PART_TXT
        StdfFile.WriteString("");           // PART_FIX
        StdfFile.WriteRecord();
    };          // Read all lines with valid data records in file

    // Write WRR for last wafer inserted
    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);                  // Test head
    StdfFile.WriteByte(255);                // Tester site (all)
    StdfFile.WriteDword(m_lStartTime);      // Time of last part tested
    StdfFile.WriteDword(iTotalGoodBin+iTotalFailBin);// Parts tested: always 5
    StdfFile.WriteDword(0);                 // Parts retested
    StdfFile.WriteDword(0);                 // Parts Aborted
    StdfFile.WriteDword(iTotalGoodBin);     // Good Parts
    StdfFile.WriteDword((DWORD)-1);         // Functionnal Parts
    StdfFile.WriteString(strWaferID.toLatin1().constData());    // WaferID
    StdfFile.WriteRecord();

    // Write MRR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteDword(m_lStartTime);      // File finish-time.
    StdfFile.WriteRecord();

    // Close STDF file.
    StdfFile.Close();

    // Success
    return true;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' PCM_MAGNACHIP_COLUMN file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGPCM_MAGNACHIP_COLUMNtoSTDF::Convert(const char *PcmMagnachipFileName, const char *strFileNameSTDF)
{
    // No erro (default)
    m_iLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(PcmMagnachipFileName);
    QFileInfo fOutput(strFileNameSTDF);

    QFile f( strFileNameSTDF );
    if((f.exists() == true) && (fInput.lastModified() < fOutput.lastModified()))
        return true;

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    bool bHideProgressAfter=true;
    bool bHideLabelAfter=false;
    if(GexScriptStatusLabel != NULL)
    {
        if(GexScriptStatusLabel->isHidden())
        {
            bHideLabelAfter = true;
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(PcmMagnachipFileName).fileName()+"...");
            GexScriptStatusLabel->show();
        }
    }

    if(GexProgressBar != NULL)
    {
        bHideProgressAfter = GexProgressBar->isHidden();
        GexProgressBar->setMaximum(100);
        GexProgressBar->setTextVisible(true);
        GexProgressBar->setValue(0);
        GexProgressBar->show();
    }
    QCoreApplication::processEvents();

    if(ReadPcmMagnachipFile(PcmMagnachipFileName,strFileNameSTDF) != true)
    {
        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if((GexProgressBar != NULL)
                && bHideProgressAfter)
            GexProgressBar->hide();

        if((GexScriptStatusLabel != NULL)
                && bHideLabelAfter)
            GexScriptStatusLabel->hide();
        return false;   // Error reading PCM_MAGNACHIP_COLUMN file
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
// Split the line into N column of 25 char
//////////////////////////////////////////////////////////////////////
QStringList CGPCM_MAGNACHIP_COLUMNtoSTDF::SplitLine(QString strLine)
{
    QStringList lstColumns;
    for(int iIndex=0; iIndex<strLine.length(); iIndex+=25)
    {
        lstColumns += strLine.mid(iIndex,25).simplified();
    }
    return lstColumns;
}

//////////////////////////////////////////////////////////////////////
// Read line : skip empty line
//////////////////////////////////////////////////////////////////////
QString CGPCM_MAGNACHIP_COLUMNtoSTDF::ReadLine(QTextStream& hFile)
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
        if(hFile.atEnd())
            break;
        strString = hFile.readLine();
        if(QString(strString).remove('\t').isEmpty())
            strString = "";
    }
    while(!strString.isNull() && strString.isEmpty());

    return strString;

}
