//////////////////////////////////////////////////////////////////////
// import_pcm_tower.cpp: Convert a PCM_TOWER (Process control Monitor).csv
// file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#include <qfileinfo.h>
#include <qregexp.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>

#include "engine.h"
#include "import_pcm_tower.h"
#include "time.h"
#include "import_constants.h"

//Date , 2012_05_10 04:12:50
//SetupTime , 2012_05_10 04:12:50
//StartTime , 2012_05_10 04:12:50
//FinishTime , 2012_05_10 04:12:50
//Lot , T202487.1
//WaferID , 25
//WaferOrientation , Unknown
//Product , IKVOX16E
//TestCode , WAFER
//Family , IKVOX16E
//Proccess , T18C6CNO4M
//SpecName , F6_TS110CIK

//LOT_ID , WAFER_ID , SITE , Rc CS P+GC 0.22 LE Ohm/Cs , Rc CS P+AA 0.22 LE Ohm/Cs , VtPcc LV 10/.18 V , ...

//HL , , , 30.000000 , 30.000000 , -0.380000 , -0.002500 , 1.000000e-009 , -3.600000 , -0.580000 , -0.002550 , ...
//LL , , , 1.000000 , 1.000000 , -0.580000 , -0.003100 , -1.000000e-009 , -5.100000 , -0.780000 , -0.003450 , ...
//T202487.1,1,1,1.74587e+01,1.42845e+01,-4.10000e-01,-3.00300e-03,-4.21700e-10,-4.33633e+00,-6.17069e-01,...
//T202487.1,1,2,1.72749e+01,1.50383e+01,-4.16000e-01,-2.83100e-03,-2.76100e-10,-4.33633e+00,-6.29787e-01,...


// main.cpp
extern QLabel           *GexScriptStatusLabel;  // Handle to script status text in status bar
extern QProgressBar     *GexProgressBar;        // Handle to progress bar in status bar

int gOffset = 3;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGPCM_TOWERtoSTDF::CGPCM_TOWERtoSTDF()
{
    // Default: PCM_TOWER parameter list on disk includes all known PCM_TOWER parameters...
    m_bNewPcmTowerParameterFound = false;
    m_lStartTime = 0;

    m_pCGPcmTowerParameter = NULL;
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CGPCM_TOWERtoSTDF::~CGPCM_TOWERtoSTDF()
{
    // Destroy list of Parameters tables.
    if(m_pCGPcmTowerParameter!=NULL)
        delete [] m_pCGPcmTowerParameter;
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGPCM_TOWERtoSTDF::GetLastError()
{
    strLastError = "Import PCM_TOWER: ";

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
// Load PCM_TOWER Parameter table from DISK
//////////////////////////////////////////////////////////////////////
void CGPCM_TOWERtoSTDF::LoadParameterIndexTable(void)
{
    QString	strPcmTowerTableFile;
    QString	strString;

    strPcmTowerTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
    strPcmTowerTableFile += GEX_PCM_TOWER_PARAMETERS;

    // Open PCM_TOWER Parameter table file
    QFile f( strPcmTowerTableFile );
    if(!f.open( QIODevice::ReadOnly ))
        return;

    // Assign file I/O stream
    QTextStream hPcmTowerTableFile(&f);

    // Skip comment lines
    do
    {
        strString = hPcmTowerTableFile.readLine();
    }
    while((strString.indexOf("----------------------") < 0) && (hPcmTowerTableFile.atEnd() == false));

    // Read lines
    m_pFullPcmTowerParametersList.clear();
    strString = hPcmTowerTableFile.readLine();
    while (strString.isNull() == false)
    {
        // Save Parameter name in list
        m_pFullPcmTowerParametersList.append(strString);
        // Read next line
        strString = hPcmTowerTableFile.readLine();
    };

    // Close file
    f.close();
}

//////////////////////////////////////////////////////////////////////
// Save PCM_TOWER Parameter table to DISK
//////////////////////////////////////////////////////////////////////
void CGPCM_TOWERtoSTDF::DumpParameterIndexTable(void)
{
    QString     strPcmTowerTableFile;

    strPcmTowerTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
    strPcmTowerTableFile += GEX_PCM_TOWER_PARAMETERS;

    // Open PCM_TOWER Parameter table file
    QFile f( strPcmTowerTableFile );
    if(!f.open( QIODevice::WriteOnly ))
        return;

    // Assign file I/O stream
    QTextStream hPcmTowerTableFile(&f);

    // First few lines are comments:
    hPcmTowerTableFile << "############################################################" << endl;
    hPcmTowerTableFile << "# DO NOT EDIT THIS FILE!" << endl;
    hPcmTowerTableFile << "# Quantix Examinator: PCM_TOWER Parameters detected" << endl;
    hPcmTowerTableFile << "# www.mentor.com" << endl;
    hPcmTowerTableFile << "# Quantix Examinator reads and writes into this file..." << endl;
    hPcmTowerTableFile << "-----------------------------------------------------------" << endl;

    // Write lines
    // m_pFullPcmTowerParametersList.sort();
    for (QStringList::const_iterator
         iter  = m_pFullPcmTowerParametersList.begin();
         iter != m_pFullPcmTowerParametersList.end(); ++iter) {
        // Write line
        hPcmTowerTableFile << *iter << endl;
    }

    // Close file
    f.close();
}

//////////////////////////////////////////////////////////////////////
// If Examinator doesn't have this PCM_TOWER parameter in his dictionnary, have it added.
//////////////////////////////////////////////////////////////////////
void CGPCM_TOWERtoSTDF::UpdateParameterIndexTable(QString strParamName)
{
    // Check if the table is empty...if so, load it from disk first!
    if(m_pFullPcmTowerParametersList.isEmpty() == true)
    {
        // Load PCM_TOWER parameter table from disk...
        LoadParameterIndexTable();
    }

    // Check if Parameter name already in table...if not, add it to the list
    // the new full list will be dumped to the disk at the end.
    if(m_pFullPcmTowerParametersList.indexOf(strParamName) < 0)
    {
        // Update list
        m_pFullPcmTowerParametersList.append(strParamName);

        // Set flag to force the current PCM_TOWER table to be updated on disk
        m_bNewPcmTowerParameterFound = true;
    }
}


//////////////////////////////////////////////////////////////////////
// Check if File is compatible with PCM_TOWER format
//////////////////////////////////////////////////////////////////////
bool CGPCM_TOWERtoSTDF::IsCompatible(const char *szFileName)
{
    QString strString;
    QString strValue;

    // Open hCsmFile file
    QFile f( szFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening ASL1000 file
        return false;
    }
    // Assign file I/O stream
    QTextStream hPcmTowerFile(&f);

    // Check if first lines are the correct PCM_TOWER header...
    //Date , 2012_05_10 04:12:50
    //SetupTime , 2012_05_10 04:12:50
    //StartTime , 2012_05_10 04:12:50
    //FinishTime , 2012_05_10 04:12:50
    //Lot , T202487.1
    //WaferID , 25
    //WaferOrientation , Unknown
    //Product , IKVOX16E
    //TestCode , WAFER
    //Family , IKVOX16E
    //Proccess , T18C6CNO4M
    //SpecName , F6_TS110CIK

    //LOT_ID , WAFER_ID , SITE , Rc CS P+GC 0.22 LE Ohm/Cs , Rc CS P+AA 0.22 LE Ohm/Cs , VtPcc LV 10/.18 V , ...

    //HL , , , 30.000000 , 30.000000 , -0.380000 , -0.002500 , 1.000000e-009 , -3.600000 , -0.580000 , ...
    //LL , , , 1.000000 , 1.000000 , -0.580000 , -0.003100 , -1.000000e-009 , -5.100000 , -0.780000 , ...
    //T202487.1,1,1,1.74587e+01,1.42845e+01,-4.10000e-01,-3.00300e-03,-4.21700e-10,-4.33633e+00,...

    // Check the correct header
    int lLine = 0;
    bool lCompatible = false;
    do
    {
        strString = hPcmTowerFile.readLine().trimmed();
        if(strString.isEmpty())
            continue;
        strValue = strString.section(",",0,0).trimmed().toUpper();
        if((strValue == "DATE")
                || (strValue == "SETUPTIME")
                || (strValue == "STARTTIME")
                || (strValue == "FINISHTIME")
                || (strValue == "LOT")
                || (strValue == "WAFERID")
                || (strValue == "WAFERORIENTATION")
                || (strValue == "PRODUCT")
                || (strValue == "TESTCODE")
                || (strValue == "FAMILY")
                || (strValue == "PROCCESS")
                || (strValue == "SPECNAME"))
            continue;
        ++lLine;
        if(strString.section(",",0,2).remove(" ").toUpper() == "LOT_ID,WAFER_ID,SITE")
        {
            // Check if have limits
            strString = hPcmTowerFile.readLine().trimmed();
            if(strString.isEmpty())
                strString = hPcmTowerFile.readLine().trimmed();

            strValue = strString.section(",",0,0).trimmed().toUpper();
            if((strValue != "HL") && (strValue != "LL"))
                break;
            strString = hPcmTowerFile.readLine().trimmed();
            strValue = strString.section(",",0,0).trimmed().toUpper();
            if((strValue != "HL") && (strValue != "LL"))
                break;

            lCompatible = true;
            break;
        }
        if(strString.count(",") > 2)
            break;
        if(lLine > 10)
            break;
    }
    while(!hPcmTowerFile.atEnd());

    f.close();

    return lCompatible;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the PCM_TOWER file
//////////////////////////////////////////////////////////////////////
bool CGPCM_TOWERtoSTDF::ReadPcmTowerFile(const char *PcmTowerFileName,const char *strFileNameSTDF)
{
    QString strString;
    QString strSection;
    QString strValue;
    bool    bStatus;
    int     iIndex;             // Loop index

    // Open CSV file
    QFile f( PcmTowerFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening PCM_TOWER file
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
    QTextStream hPcmTowerFile(&f);
    // Check if first lines are the correct PCM_TOWER header...
    //Date , 2012_05_10 04:12:50
    //SetupTime , 2012_05_10 04:12:50
    //StartTime , 2012_05_10 04:12:50
    //FinishTime , 2012_05_10 04:12:50
    //Lot , T202487.1
    //WaferID , 25
    //WaferOrientation , Unknown
    //Product , IKVOX16E
    //TestCode , WAFER
    //Family , IKVOX16E
    //Proccess , T18C6CNO4M
    //SpecName , F6_TS110CIK

    //LOT_ID , WAFER_ID , SITE , Rc CS P+GC 0.22 LE Ohm/Cs , Rc CS P+AA 0.22 LE Ohm/Cs , VtPcc LV 10/.18 V , ...

    //HL , , , 30.000000 , 30.000000 , -0.380000 , -0.002500 , 1.000000e-009 , -3.600000 , -0.580000 , ...
    //LL , , , 1.000000 , 1.000000 , -0.580000 , -0.003100 , -1.000000e-009 , -5.100000 , -0.780000 , ...
    //T202487.1,1,1,1.74587e+01,1.42845e+01,-4.10000e-01,-3.00300e-03,-4.21700e-10,-4.33633e+00,...
    do
    {
        strString = ReadLine(hPcmTowerFile).trimmed();
        if(strString.isEmpty())
            continue;
        strSection = strString.section(",",0,0).trimmed().toUpper();
        strValue = strString.section(",",1).trimmed().toUpper();
        if(strSection == "DATE")
            m_lSetupTime = GetDateTimeFromString(strValue);
        else if(strSection == "SETUPTIME")
            m_lSetupTime = GetDateTimeFromString(strValue);
        else if(strSection == "STARTTIME")
            m_lStartTime = GetDateTimeFromString(strValue);
        else if(strSection == "FINISHTIME")
            m_lFinishTime = GetDateTimeFromString(strValue);
        else if(strSection == "LOT")
            m_strLotID = strValue;
        else if(strSection == "WAFERID")
            continue;
        else if(strSection == "WAFERORIENTATION")
            continue;
        else if(strSection == "PRODUCT")
            m_strProductID = strValue;
        else if(strSection == "TESTCODE")
            m_strTestCode = strValue;
        else if(strSection == "FAMILY")
            m_strFamilyID = strValue;
        else if(strSection == "PROCESS")
            m_strProcessID = strValue;
        else if(strSection == "SPECNAME")
            m_strSpecName = strValue;
        else if(strString.section(",",0,2).remove(" ").toUpper() == "LOT_ID,WAFER_ID,SITE")
            break;
    }
    while(!hPcmTowerFile.atEnd());

    if(strString.section(",",0,2).remove(" ").toUpper() != "LOT_ID,WAFER_ID,SITE")
    {
        // Incorrect file date (greater than license expiration date)...refuse to convert file!
        iLastError = errInvalidFormat;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    QStringList lstSections = strString.split(",", QString::KeepEmptyParts);

    // Count the number of parameters specified in the line
    // Do not count first 3 fields
    m_iTotalParameters=lstSections.count() - gOffset;
    // If no parameter specified...ignore!
    if(m_iTotalParameters <= 0)
    {
        // Incorrect header...this is not a valid PCM_TOWER file!
        iLastError = errInvalidFormat;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    // Allocate the buffer to hold the N parameters & results.
    m_pCGPcmTowerParameter = new CGPcmTowerParameter[m_iTotalParameters];	// List of parameters

    // Extract the N column names
    QString strUnit;
    for(iIndex=0;iIndex<m_iTotalParameters;iIndex++)
    {
        strSection = lstSections[iIndex+gOffset].trimmed();	// Remove spaces
        strUnit = "";
        // Retrieve the unit
        if(strSection.count(" ")>1)
        {
            strUnit = strSection.section(" ",strSection.count(" "),strSection.count(" "));
            // Check if it is a known unit
            if(strUnit.count("/") == 0)
                strSection = strSection.section(" ",0,strSection.count(" ")-1);
            else
                strUnit = "";
        }

        m_pCGPcmTowerParameter[iIndex].strName = strSection;
        UpdateParameterIndexTable(strSection);		// Update Parameter master list if needed.
        m_pCGPcmTowerParameter[iIndex].strUnits = strUnit;
        m_pCGPcmTowerParameter[iIndex].bStaticHeaderWritten = false;
    }

    int	iLimits =0;
    strString = ReadLine(hPcmTowerFile).trimmed();
    if(strString.isEmpty())
        strString = ReadLine(hPcmTowerFile).trimmed();

    if(!strString.startsWith("HL"))
    {
        // Incorrect header...this is not a valid PCM_TOWER file!
        iLastError = errInvalidFormat;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    // found the HIGH limits
    iLimits |= 1;
    lstSections = strString.split(",", QString::KeepEmptyParts);
    // Check if have the good count
    if(lstSections.count() < m_iTotalParameters+gOffset)
    {
        iLastError = errInvalidFormatLowInRows;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    // Extract the N column Upper Limits
    for(iIndex=0;iIndex<m_iTotalParameters;iIndex++)
    {
        strSection = lstSections[iIndex+gOffset].trimmed();
        m_pCGPcmTowerParameter[iIndex].fHighLimit = strSection.toFloat(&bStatus);
        m_pCGPcmTowerParameter[iIndex].bValidHighLimit = bStatus;
    }

    strString = ReadLine(hPcmTowerFile).trimmed();
    if(!strString.startsWith("LL"))
    {
        // Incorrect header...this is not a valid PCM_TOWER file!
        iLastError = errInvalidFormat;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    // found the Low limits
    iLimits |= 2;
    lstSections = strString.split(",", QString::KeepEmptyParts);
    // Check if have the good count
    if(lstSections.count() < m_iTotalParameters+gOffset)
    {
        iLastError = errInvalidFormatLowInRows;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    // Extract the N column Lower Limits
    for(iIndex=0;iIndex<m_iTotalParameters;iIndex++)
    {
        strSection = lstSections[iIndex+gOffset].trimmed();
        m_pCGPcmTowerParameter[iIndex].fLowLimit = strSection.toFloat(&bStatus);
        m_pCGPcmTowerParameter[iIndex].bValidLowLimit = bStatus;
    }

    if(iLimits != 3)
    {
        // Incorrect header...this is not a valid PCM_TOWER file!: we didn't find the limits!
        iLastError = errNoLimitsFound;

        // Convertion failed.
        bStatus = false;
    }
    else
    {
        // Loop reading file until end is reached & generate STDF file dynamically.
        bStatus = WriteStdfFile(&hPcmTowerFile,strFileNameSTDF);
        if(!bStatus)
            QFile::remove(strFileNameSTDF);
    }

    // Close file
    f.close();

    // All PCM_TOWER file read...check if need to update the PCM_TOWER Parameter list on disk?
    if(bStatus && (m_bNewPcmTowerParameterFound == true))
        DumpParameterIndexTable();

    // Success parsing PCM_TOWER file
    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from PCM_TOWER data parsed
//////////////////////////////////////////////////////////////////////
bool CGPCM_TOWERtoSTDF::WriteStdfFile(QTextStream *hPcmTowerFile, const char *strFileNameSTDF)
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
    StdfFile.WriteByte(1);                  // SUN CPU type
    StdfFile.WriteByte(4);                  // STDF V4
    StdfFile.WriteRecord();

    if(m_lStartTime <= 0)
        m_lStartTime = QDateTime::currentDateTime().toTime_t();

    // Write MIR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 10;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteDword(m_lSetupTime);          // Setup time
    StdfFile.WriteDword(m_lStartTime);          // Start time
    StdfFile.WriteByte(1);                      // Station
    StdfFile.WriteByte((BYTE) 'P');             // Test Mode = PRODUCTION
    StdfFile.WriteByte((BYTE) ' ');             // rtst_cod
    StdfFile.WriteByte((BYTE) ' ');             // prot_cod
    StdfFile.WriteWord(65535);                  // burn_tim
    StdfFile.WriteByte((BYTE) ' ');             // cmod_cod
    StdfFile.WriteString(m_strLotID.toLatin1().constData());        // Lot ID
    StdfFile.WriteString(m_strProductID.toLatin1().constData());    // Part Type / Product ID
    StdfFile.WriteString("");                   // Node name
    StdfFile.WriteString("");                   // Tester Type
    StdfFile.WriteString(m_strProgramID.toLatin1().constData());    // Job name
    StdfFile.WriteString("");                   // Job rev
    StdfFile.WriteString("");                   // sublot-id
    StdfFile.WriteString("");                   // operator
    StdfFile.WriteString("");                   // exec-type
    StdfFile.WriteString("");                   // exe-ver
    StdfFile.WriteString(m_strTestCode.toLatin1().constData());     // test-cod
    StdfFile.WriteString("");                   // test-temperature
    // Construct custom Galaxy USER_TXT
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ETEST;
    strUserTxt += ":PCM_TOWER";
    StdfFile.WriteString(strUserTxt.toLatin1().constData());// user-txt
    StdfFile.WriteString("");                           // aux-file
    StdfFile.WriteString("");                           // package-type
    StdfFile.WriteString(m_strFamilyID.toLatin1().constData()); // familyID
    StdfFile.WriteString("");                           // Date-code
    StdfFile.WriteString("");                           // Facility-ID
    StdfFile.WriteString("");                           // FloorID
    StdfFile.WriteString(m_strProcessID.toLatin1().constData());    // ProcessID
    StdfFile.WriteString("");                           // Opr freq
    StdfFile.WriteString(m_strSpecName.toLatin1().constData());// Spec Name
    StdfFile.WriteRecord();

    // Write Test results for each line read.
    QString strString;
    char    szString[257];
    QString strSection;
    QString strWaferID;
    float   fValue;             // Used for readng floating point numbers.
    int     iIndex;             // Loop index
    int     iSiteNumber;
    BYTE        bData;
    WORD        wSoftBin,wHardBin;
    long        iTotalGoodBin,iTotalFailBin;
    long        iAllGoodBin,iAllFailBin;
    long        iTestNumber,iTotalTests,iPartNumber;
    bool        bStatus,bPassStatus;
    QStringList lstSections;
    // Reset counters
    iTotalGoodBin=iTotalFailBin=0;
    iAllGoodBin=iAllFailBin=0;
    iPartNumber=0;

    // Write all Parameters read on this wafer.: WIR.PIR,PTR....PTR, PRR, PIR, PTR,PTR...PRR,   ... WRR
    while(!hPcmTowerFile->atEnd())
    {

        // Part number
        iPartNumber++;

        // Read line
        strString = ReadLine(*hPcmTowerFile);
        lstSections = strString.split(",", QString::KeepEmptyParts);
        // Check if have the good count
        if(lstSections.count() < m_iTotalParameters+gOffset)
        {
            iLastError = errInvalidFormatLowInRows;

            StdfFile.Close();
            // Convertion failed.
            return false;
        }

        // Extract WaferID
        strSection = lstSections[1].trimmed();
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
                StdfFile.WriteRecord();

                iAllGoodBin += iTotalGoodBin;
                iAllFailBin += iTotalFailBin;
            }

            iTotalGoodBin=iTotalFailBin=0;
            // For each wafer, have to write limit in the first PTR
            for(iIndex=0;iIndex<m_iTotalParameters;iIndex++)
                m_pCGPcmTowerParameter[iIndex].bStaticHeaderWritten = false;

            // Write WIR of new Wafer.
            strWaferID = strSection;
            RecordReadInfo.iRecordType = 2;
            RecordReadInfo.iRecordSubType = 10;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(1);                              // Test head
            StdfFile.WriteByte(255);                            // Tester site (all)
            StdfFile.WriteDword(m_lStartTime);                  // Start time
            StdfFile.WriteString(strWaferID.toLatin1().constData());    // WaferID
            StdfFile.WriteRecord();
        }

        // Reset Pass/Fail flag.
        bPassStatus = true;

        // Reset counters
        iTotalTests = 0;

        // Extract Site
        iSiteNumber = lstSections[2].trimmed().toInt();

        // Write PIR for parts in this Wafer site
        RecordReadInfo.iRecordType = 5;
        RecordReadInfo.iRecordSubType = 10;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);                              // Test head
        StdfFile.WriteByte(iSiteNumber);                    // Tester site
        StdfFile.WriteRecord();

        // Read Parameter results for this record
        for(iIndex=0;iIndex<m_iTotalParameters;iIndex++)
        {
            strSection = lstSections[iIndex+gOffset].trimmed();
            fValue = strSection.toFloat(&bStatus);
            if(bStatus == true)
            {
                // Valid test result...write the PTR
                iTotalTests++;

                RecordReadInfo.iRecordType = 15;
                RecordReadInfo.iRecordSubType = 10;
                StdfFile.WriteHeader(&RecordReadInfo);
                // Compute Test# (add user-defined offset)
                iTestNumber = (long) m_pFullPcmTowerParametersList.indexOf(m_pCGPcmTowerParameter[iIndex].strName);
                iTestNumber += GEX_TESTNBR_OFFSET_PCM_TOWER;// Test# offset
                StdfFile.WriteDword(iTestNumber);           // Test Number
                StdfFile.WriteByte(1);                      // Test head
                StdfFile.WriteByte(iSiteNumber);            // Tester site#
                if(((m_pCGPcmTowerParameter[iIndex].bValidLowLimit==true) && (fValue < m_pCGPcmTowerParameter[iIndex].fLowLimit)) ||
                        ((m_pCGPcmTowerParameter[iIndex].bValidHighLimit==true) && (fValue > m_pCGPcmTowerParameter[iIndex].fHighLimit)))
                {
                    bData = 0200;	// Test Failed
                    bPassStatus = false;
                }
                else
                {
                    bData = 0;		// Test passed
                }
                StdfFile.WriteByte(bData);                          // TEST_FLG
                StdfFile.WriteByte(0x40|0x80);                      // PARAM_FLG
                StdfFile.WriteFloat(fValue);                        // Test result
                if(m_pCGPcmTowerParameter[iIndex].bStaticHeaderWritten == false)
                {
                    StdfFile.WriteString(m_pCGPcmTowerParameter[iIndex].strName.toLatin1().constData());// TEST_TXT
                    StdfFile.WriteString("");                       // ALARM_ID
                    bData = 2;	// Valid data.
                    if(m_pCGPcmTowerParameter[iIndex].bValidLowLimit==false)
                        bData |=0x40;
                    if(m_pCGPcmTowerParameter[iIndex].bValidHighLimit==false)
                        bData |=0x80;
                    StdfFile.WriteByte(bData);                      // OPT_FLAG
                    StdfFile.WriteByte(0);                          // RES_SCALE
                    StdfFile.WriteByte(0);                          // LLM_SCALE
                    StdfFile.WriteByte(0);                          // HLM_SCALE
                    StdfFile.WriteFloat(m_pCGPcmTowerParameter[iIndex].fLowLimit);  // LOW Limit
                    StdfFile.WriteFloat(m_pCGPcmTowerParameter[iIndex].fHighLimit); // HIGH Limit
                    StdfFile.WriteString(m_pCGPcmTowerParameter[iIndex].strUnits.toLatin1().constData());// Units
                    m_pCGPcmTowerParameter[iIndex].bStaticHeaderWritten = true;
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
        case 1:	// Left
            StdfFile.WriteWord(1);          // X_COORD
            StdfFile.WriteWord(4);          // Y_COORD
            break;
        case 2:	// Down
            StdfFile.WriteWord(5);          // X_COORD
            StdfFile.WriteWord(6);          // Y_COORD
            break;
        case 3:	// Center
            StdfFile.WriteWord(5);          // X_COORD
            StdfFile.WriteWord(3);          // Y_COORD
            break;
        case 4:	// Top
            StdfFile.WriteWord(5);          // X_COORD
            StdfFile.WriteWord(1);          // Y_COORD
            break;
        case 5:	// Right
            StdfFile.WriteWord(8);          // X_COORD
            StdfFile.WriteWord(4);          // Y_COORD
            break;
        case 6:	// Upper-Right corner
            StdfFile.WriteWord(7);          // X_COORD
            StdfFile.WriteWord(2);          // Y_COORD
            break;
        case 7:	// Upper-Left corner
            StdfFile.WriteWord(3);          // X_COORD
            StdfFile.WriteWord(2);          // Y_COORD
            break;
        case 8:	// Lower-Left corner
            StdfFile.WriteWord(3);          // X_COORD
            StdfFile.WriteWord(5);          // Y_COORD
            break;
        case 9:	// Lower-Right corner
            StdfFile.WriteWord(7);          // X_COORD
            StdfFile.WriteWord(5);          // Y_COORD
            break;
        default: // More than 5 sites?....give 0,0 coordonates
            StdfFile.WriteWord(0);              // X_COORD
            StdfFile.WriteWord(0);              // Y_COORD
            break;
        }
        StdfFile.WriteDword(0);             // No testing time known...
        sprintf(szString,"%ld",iPartNumber);
        StdfFile.WriteString(szString);     // PART_ID
        StdfFile.WriteString("");           // PART_TXT
        StdfFile.WriteString("");           // PART_FIX
        StdfFile.WriteRecord();
    };			// Read all lines with valid data records in file

    // Write WRR for last wafer inserted
    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);                      // Test head
    StdfFile.WriteByte(255);                    // Tester site (all)
    StdfFile.WriteDword(m_lFinishTime);          // Time of last part tested
    StdfFile.WriteDword(iTotalGoodBin+iTotalFailBin);	// Parts tested: always 5
    StdfFile.WriteDword(0);                     // Parts retested
    StdfFile.WriteDword(0);                     // Parts Aborted
    StdfFile.WriteDword(iTotalGoodBin);         // Good Parts
    StdfFile.WriteDword((DWORD)-1);             // Functionnal Parts
    StdfFile.WriteString(strWaferID.toLatin1().constData());// WaferID
    StdfFile.WriteRecord();

    // Write HBR/site
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 40;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(255);                    // Test Head
    StdfFile.WriteByte(0);                      // Test sites
    StdfFile.WriteWord(0);                      // HBIN
    StdfFile.WriteDword(iAllFailBin);           // Total Bins
    StdfFile.WriteByte('F');
    StdfFile.WriteString("FAIL");
    StdfFile.WriteRecord();

    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(255);                    // Test Head
    StdfFile.WriteByte(0);                      // Test sites
    StdfFile.WriteWord(1);                      // HBIN
    StdfFile.WriteDword(iAllGoodBin);           // Total Bins
    StdfFile.WriteByte('P');
    StdfFile.WriteString("PASS");
    StdfFile.WriteRecord();

    // Write SBR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 50;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(255);                    // Test Head
    StdfFile.WriteByte(0);                      // Test sites
    StdfFile.WriteWord(0);                      // SBIN
    StdfFile.WriteDword(iAllFailBin);           // Total Bins
    StdfFile.WriteByte('F');
    StdfFile.WriteString("FAIL");
    StdfFile.WriteRecord();

    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(255);                    // Test Head
    StdfFile.WriteByte(0);                      // Test sites
    StdfFile.WriteWord(1);                      // SBIN
    StdfFile.WriteDword(iAllGoodBin);           // Total Bins
    StdfFile.WriteByte('P');
    StdfFile.WriteString("PASS");
    StdfFile.WriteRecord();

    // Write MRR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteDword(m_lFinishTime);          // File finish-time.
    StdfFile.WriteRecord();

    // Close STDF file.
    StdfFile.Close();

    // Success
    return true;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' PCM_TOWER file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGPCM_TOWERtoSTDF::Convert(const char *PcmTowerFileName, const char *strFileNameSTDF)
{
    // No erro (default)
    iLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(PcmTowerFileName);
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
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(PcmTowerFileName).fileName()+"...");
            GexScriptStatusLabel->show();
        }
        GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(PcmTowerFileName).fileName()+"...");
        GexScriptStatusLabel->show();
    }
    QCoreApplication::processEvents();

    if(ReadPcmTowerFile(PcmTowerFileName,strFileNameSTDF) != true)
    {
        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if((GexProgressBar != NULL)
                && bHideProgressAfter)
            GexProgressBar->hide();

        if((GexScriptStatusLabel != NULL)
                && bHideLabelAfter)
            GexScriptStatusLabel->hide();
        return false;   // Error reading PCM_TOWER file
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
//return lDateTime from string strDateTime "2012_05_10 04:12:50".
//////////////////////////////////////////////////////////////////////
long CGPCM_TOWERtoSTDF::GetDateTimeFromString(QString strDateTime)
{
    int nYear, nMonth, nDay;
    int	nHour, nMin, nSec;
    long lDateTime;

    if(strDateTime.length()<19)
        return 0;

    QString strDate = strDateTime.section(" ",0,0).trimmed();
    QString strTime = strDateTime.section(" ",1).trimmed();

    // Format yyyy_mm_dd hh:mn:ss
    nYear = strDate.section("_",0,0).toInt();
    nMonth = strDate.section("_",1,1).toInt();
    nDay = strDate.section("_",2,2).toInt();
    nHour = strTime.section(":",0,0).toInt();
    nMin= strTime.section(":",1,1).toInt();
    nSec = strTime.section(":",2,2).toInt();

    QDate clDate(nYear,nMonth,nDay);
    QTime clTime(nHour,nMin,nSec);
    QDateTime clDateTime(clDate,clTime);

    clDateTime.setTimeSpec(Qt::UTC);
    lDateTime = clDateTime.toTime_t();
    return lDateTime;
}

//////////////////////////////////////////////////////////////////////
// Read line : skip empty line
//////////////////////////////////////////////////////////////////////
QString CGPCM_TOWERtoSTDF::ReadLine(QTextStream& hFile)
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
