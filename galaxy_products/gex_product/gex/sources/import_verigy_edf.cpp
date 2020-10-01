//////////////////////////////////////////////////////////////////////
// import_verigy_edf.cpp: Convert a .VERIGY_EDF (TSMC) file to STDF V4.0
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
#include <QTextStream>

#include "engine.h"
#include "import_verigy_edf.h"
#include "import_constants.h"

// File format:
//######## Started Testprogram Testflow ##########################################
//Level die started on 05/14/06 at 20:12:41
//Testflow started on 05/14/06 at 20:14:52
//Site 1: W2,23
//Site 2: W2,22
//
//======== Started Testsuite Pre_Iddq_fs_off_27 ==================================
//Executed Testmethod IDDQ,Production_Iddq on Site 1: PASSED
//Equation Set 26; Spec Set 1; Timing Set 1
//Equation Set 4; Spec Set 2; Level Set 1
//Startlabel: iddq_fs_off_27
//
//-------- IDDQ_CURRENT@VDD_DIG[1]: PASSED ---------------------------------------
//IDDQ_CURRENT@VDD_DIG[1]: PASSED
//Pass/Fail Limits: [ 0.100000  ,   545.000000  ]
//-------- Pin Results -----------------------------------------------------------
//         VDD_DIG PASSED 144.749000
//
//Executed Testmethod IDDQ,Production_Iddq on Site 2: PASSED
//Equation Set 26; Spec Set 1; Timing Set 1
//Equation Set 4; Spec Set 2; Level Set 1
//Startlabel: iddq_fs_off_27
//
//-------- IDDQ_CURRENT@VDD_DIG[1]: PASSED ---------------------------------------
//IDDQ_CURRENT@VDD_DIG[1]: PASSED
//Pass/Fail Limits: [ 0.100000  ,   545.000000  ]
//-------- Pin Results -----------------------------------------------------------
//         VDD_DIG PASSED 367.816000


// ADD NEW SYNTAX FOR PASSED FAILED TRANSITION
//
//Executed Testfunction spec_search on Site 1: PASSED
//Equation Set 4; Spec Set 2; Timing Set 1
//Equation Set 1; Spec Set 1; Level Set 1
//Startlabel: vid_funcre
//
//Spec Name: per
//Spec Type: timing
//
//Search Method: binary
//Search Range:  [   50.000 ns,  1000.000 ns]
//Step Width:        25.000 ns
//Resolution:         0.100 ns
//-------- Results ---------------------------------------------------------------
//Passed/Failed transition:    P = 416.500 ns    F = 416.600 ns
//
//
//======== Ended Testsuite Pre_Iddq_fs_off_27 ====================================
//
//Site 2 has PASSED. Binned to bin 1(1)

// Format 2: case 6591
//######## Started Testprogram Testflow ##########################################
//Test program Testflow of device /fme/users/mvelum/PROJECTS/EVEREST_40nm_93k/FT/B0/In_Development/8af2030_ft_0100_201204030/8af2030_ft_0100 started on 05/03/2012 at 08:51:31 AM on verigy.fme.fujitsu.com by mvelum
//Level device started on 05/03/2012 at 08:51:31 AM
//Site 1: P1
//Testflow started on 05/03/2012 at 08:51:31 AM
//PrintToDatalog on Site 1: DIB_ID:SY114294
//PrintToDatalog on Site 1: DEVICE_TYPE:
//PrintToDatalog on Site 1: DEVICE_ID:
//Executed Testmethod 8af2010_tml.src.DC_Tests.SupplyShort_Testtest_number : 190


//Site  TNum      Test                  TestSuite         p/f      Pin  LoLim              Measure        HiLim             TestFunc  VectLabel   T_Equ  T_Spec  T_Set  L_Equ  L_Spec  L_Set
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//1     100       Idd_AVDRF[1]          SupplyShort_Test  passed   .    -1.000000 mA <=     0.140740 mA    <=  5.000000 mA  .         AdcPowerUp  1      1       1      2      1       1
//1     110       Idd_AVDBB[1]          SupplyShort_Test  passed   .    -1.000000 mA <=     0.856853 mA    <=  5.000000 mA  .         AdcPowerUp  1      1       1      2      1       1

//1     250       Idd_VDDSFI[1]         SupplyShort_Test  passed   .    -1.000000 mA <=     0.704705 mA    <=  5.000000 mA  .         AdcPowerUp  1      1       1      2      1       1
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//TestSuite:  SupplyShort_Test  failed


// main.cpp
extern QLabel           *GexScriptStatusLabel;  // Handle to script status text in status bar
extern QProgressBar	*   GexProgressBar;         // Handle to progress bar in status bar

#define TEST_PASSFLAG_UNKNOWN   0
#define TEST_PASSFLAG_PASS      1
#define TEST_PASSFLAG_FAIL      2


#define BIT0            0x01
#define BIT1            0x02
#define BIT2            0x04
#define BIT3            0x08
#define BIT4            0x10
#define BIT5            0x20
#define BIT6            0x40
#define BIT7            0x80

CVerigyEdfBinInfo::CVerigyEdfBinInfo()
{
    nPassFlag=TEST_PASSFLAG_UNKNOWN;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGVERIGY_EDFtoSTDF::CGVERIGY_EDFtoSTDF()
{
    // Default: VERIGY_EDF parameter list on disk includes all known VERIGY_EDF parameters...
    m_bNewVerigyEdfParameterFound = false;
    m_lStartTime = 0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGVERIGY_EDFtoSTDF::~CGVERIGY_EDFtoSTDF()
{
    QMap<int,CVerigyEdfBinInfo*>::Iterator itMapBin;
    for ( itMapBin = m_mapBins.begin(); itMapBin != m_mapBins.end(); ++itMapBin )
    {
        delete itMapBin.value();
    }
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGVERIGY_EDFtoSTDF::GetLastError()
{
    QString strLastError = "Import VERIGY_EDF: ";

    switch(m_iLastError)
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
    case errWriteSTDF:
        strLastError += "Failed creating temporary file. Folder permission issue?";
        break;
    case errLicenceExpired:
        strLastError += "License has expired or Data file out of date...";
        break;
    }
    if(!m_strLastError.isEmpty())
        strLastError += " - "+m_strLastError;

    // Return Error Message
    return strLastError;
}

//////////////////////////////////////////////////////////////////////
// Load VERIGY_EDF Parameter table from DISK
//////////////////////////////////////////////////////////////////////
void CGVERIGY_EDFtoSTDF::LoadParameterIndexTable(void)
{
    QString strVerigyEdfTableFile;
    QString strString;

    strVerigyEdfTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
    strVerigyEdfTableFile += GEX_VERIGY_EDL_PARAMETERS;

    // Open VERIGY_EDF Parameter table file
    QFile f( strVerigyEdfTableFile );
    if(!f.open( QIODevice::ReadOnly ))
        return;

    // Assign file I/O stream
    QTextStream hVerigyEdfTableFile(&f);

    // Skip comment or empty lines
    do
    {
        strString = hVerigyEdfTableFile.readLine();
    }
    while((strString.indexOf("----------------------") < 0) && (!hVerigyEdfTableFile.atEnd()));

    // Read lines
    m_pFullVerigyEdfParametersList.clear();
    strString = hVerigyEdfTableFile.readLine();
    while (strString.isNull() == false)
    {
        // Save Parameter name in list
        m_pFullVerigyEdfParametersList.append(strString);
        // Read next line
        strString = hVerigyEdfTableFile.readLine();
    };

    // Close file
    f.close();
}

//////////////////////////////////////////////////////////////////////
// Save VERIGY_EDF Parameter table to DISK
//////////////////////////////////////////////////////////////////////
void CGVERIGY_EDFtoSTDF::DumpParameterIndexTable(void)
{
    QString strVerigyEdfTableFile;
    QString strString;

    strVerigyEdfTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
    strVerigyEdfTableFile += GEX_VERIGY_EDL_PARAMETERS;

    // Open VERIGY_EDF Parameter table file
    QFile f( strVerigyEdfTableFile );
    if(!f.open( QIODevice::WriteOnly ))
        return;

    // Assign file I/O stream
    QTextStream hVerigyEdfTableFile(&f);

    // First few lines are comments:
    hVerigyEdfTableFile << "############################################################" << endl;
    hVerigyEdfTableFile << "# DO NOT EDIT THIS FILE!" << endl;
    hVerigyEdfTableFile << "# Quantix Examinator: VERIGY_EDF Parameters detected" << endl;
    hVerigyEdfTableFile << "# www.mentor.com" << endl;
    hVerigyEdfTableFile << "# Quantix Examinator reads and writes into this file..." << endl;
    hVerigyEdfTableFile << "-----------------------------------------------------------" << endl;

    // Write lines
    // m_pFullVerigyEdfParametersList.sort();
    for(int nIndex = 0; nIndex < m_pFullVerigyEdfParametersList.count(); nIndex++)
    {
        // Write line
        hVerigyEdfTableFile << m_pFullVerigyEdfParametersList[nIndex] << endl;
    };

    // Close file
    f.close();
}

//////////////////////////////////////////////////////////////////////
// If Examinator doesn't have this VERIGY_EDF parameter in his dictionnary, have it added.
//////////////////////////////////////////////////////////////////////
void CGVERIGY_EDFtoSTDF::UpdateParameterIndexTable(QString strParamName)
{
    // Check if the table is empty...if so, load it from disk first!
    if(m_pFullVerigyEdfParametersList.isEmpty() == true)
    {
        // Load VERIGY_EDF parameter table from disk...
        LoadParameterIndexTable();
    }

    // Check if Parameter name already in table...if not, add it to the list
    // the new full list will be dumped to the disk at the end.
    if(m_pFullVerigyEdfParametersList.indexOf(strParamName) < 0)
    {
        // Update list
        m_pFullVerigyEdfParametersList.append(strParamName);

        // Set flag to force the current VERIGY_EDF table to be updated on disk
        m_bNewVerigyEdfParameterFound = true;
    }
}


//////////////////////////////////////////////////////////////////////
// Normalize test limits when writing into PTR.
//////////////////////////////////////////////////////////////////////
void CGVERIGY_EDFtoSTDF::NormalizeLimits(QString &strUnit, int &nScale)
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
// Read and Parse the VERIGY_EDF file
//////////////////////////////////////////////////////////////////////
bool CGVERIGY_EDFtoSTDF::ReadVerigyEdfFile(const char *VerigyEdfFileName,const char *strFileNameSTDF)
{
    QString strString;

    // Open VERIGY_EDF file
    FILE *fp = fopen(VerigyEdfFileName,"r");
    char buff[2048+1];
    if(!fp)
    {
        // Failed Opening VERIGY_EDF file
        m_iLastError = errOpenFail;

        // Convertion failed.
        return false;
    }


    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    iProgressStep = 0;
    iNextFilePos = 0;
    fseek(fp, 0, SEEK_END);
    iFileSize = ftell(fp) + 1;
    rewind(fp);
    QCoreApplication::processEvents();

    // Check if first line is the correct VERIGY_EDF header...
    //Level die started on 05/14/06 at 20:12:41

    // Goto the first line Testflow started
    // ignore all other line before
    while(ReadLine(fp,buff))
    {
        strString = buff;
        if(strString.contains(" started on ", Qt::CaseInsensitive))
            break;
    }

    //Level die started on 05/14/06 at 20:12:41
    //Testflow started on 11/13/06 at 23:15:22
    if(!strString.contains(" started on ", Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a VERIGY_EDF file!
        m_iLastError = errInvalidFormat;
        fclose(fp);

        // Convertion failed.
        return false;
    }

    // Read VERIGY_EDF information
    // Goto date
    // format: started on 07/06/09 at 17:51:02
    // format: started on 05/03/2012 at 08:51:31 AM
    strString = strString.section(" started on ",1).section(" on ",0,0);
    m_lStartTime = GetDateTimeFromString(strString);

    // It's a Verigy EDF file

    if(!WriteStdfFile(fp,strFileNameSTDF))
    {
        QFile::remove(strFileNameSTDF);
        fclose(fp);
        return false;
    }
    fclose(fp);

    // All VERIGY_EDF file read...check if need to update the VERIGY_EDF Parameter list on disk?
    if(m_bNewVerigyEdfParameterFound == true)
        DumpParameterIndexTable();

    // Success parsing VERIGY_EDF file
    return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from VERIGY_EDF data parsed
//////////////////////////////////////////////////////////////////////
bool CGVERIGY_EDFtoSTDF::WriteStdfFile(FILE *fp,const char *strFileNameSTDF)
{
    // now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    char buff[2048+1];
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open((char*)strFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        // Failed importing VERIGY_EDF file into STDF database
        m_iLastError = errWriteSTDF;

        // Convertion failed.
        return false;
    }

    // Write FAR
    RecordReadInfo.iRecordType = 0;
    RecordReadInfo.iRecordSubType = 10;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);                      // SUN CPU type
    StdfFile.WriteByte(4);                      // STDF V4
    StdfFile.WriteRecord();

    if(m_lStartTime <= 0)
        m_lStartTime = QDateTime::currentDateTime().toTime_t();

    // Write MIR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 10;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteDword(m_lStartTime);          // Setup time
    StdfFile.WriteDword(m_lStartTime);          // Start time
    StdfFile.WriteByte(1);                      // Station
    StdfFile.WriteByte((BYTE) 'P');             // Test Mode = PRODUCTION
    StdfFile.WriteByte((BYTE) ' ');             // rtst_cod
    StdfFile.WriteByte((BYTE) ' ');             // prot_cod
    StdfFile.WriteWord(65535);                  // burn_tim
    StdfFile.WriteByte((BYTE) ' ');             // cmod_cod
    StdfFile.WriteString(m_strLotId.toLatin1().constData());// Lot ID
    StdfFile.WriteString("");                   // Part Type / Product ID
    StdfFile.WriteString("");                   // Node name
    StdfFile.WriteString("VERIGY");             // Tester Type
    StdfFile.WriteString("");                   // Job name
    StdfFile.WriteString("");                   // Job rev
    StdfFile.WriteString("");                   // sublot-id
    StdfFile.WriteString("");                   // operator
    StdfFile.WriteString("");                   // exec-type
    StdfFile.WriteString("");                   // exe-ver
    StdfFile.WriteString("");                   // test-cod
    StdfFile.WriteString("");                   // test-temperature
    // Construct custom Galaxy USER_TXT
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    strUserTxt += ":VERIGY_EDF";
    StdfFile.WriteString((char *)strUserTxt.toLatin1().constData());	// user-txt
    StdfFile.WriteString("");                   // aux-file
    StdfFile.WriteString("");                   // package-type
    StdfFile.WriteString("");                   // familyID
    StdfFile.WriteString("");                   // Date-code
    StdfFile.WriteString("");                   // Facility-ID
    StdfFile.WriteString("");                   // FloorID
    StdfFile.WriteString(m_strProcessId.toLatin1().constData());// ProcessID

    StdfFile.WriteRecord();

    bool        bWirWritten = false;
    // Write Test results for each line read.
    QString     strString;
    WORD        wSoftBin,wHardBin;
    long        iTotalGoodBin,iTotalFailBin;
    QString     strPartNumber;
    long        iTotalPartsRetested;
    BYTE        bData;
    CVerigyEdfPartInfo *pPartInfo=NULL;
    QMap<int, CVerigyEdfPartInfo*>::Iterator iSiteInfo;

    int         iSiteNumber = 0;
    int         iTestNumber = 0;
    bool        bTestPass = false;
    bool        bTestShmoo = false;
    bool        bTestFunctional;
    QString     strStartTestSuite;
    QString     strStartLabel;
    QString     strTestName;
    QString     strPinName;
    QString     strResult;
    QString     strLeft="[";
    QString     strRight="]";
    QString     strUnit;
    float       fLowLimit=0.0F, fHighLimit=0.0F;
    float       fResult;
    int         nScale;
    bool        bStrictLowLimit, bStrictHighLimit;
    bool        bHaveLowLimit, bHaveHighLimit;
    bool        bTestAlreadySaved;
    QStringList lstTestSavedWithLimit;
    int         iIndex = 0;

    QStringList lstPinNameIndex;


    // Reset counters
    bTestFunctional = false;
    bHaveLowLimit = bHaveHighLimit = false;
    bStrictLowLimit = bStrictHighLimit = false;
    iTotalGoodBin=iTotalFailBin=0;
    iTotalPartsRetested = 0;

    // Write all Parameters read on this file : PIR,PTR...,PRR, PIR,PTR..., PRR
    //Testflow started on 05/14/06 at 20:14:52
    //Site 1: W2,23
    //Site 2: W2,22

    // Read VERIGY_EDF information
    while(ReadLine(fp,buff))
    {
        if(strlen(buff) == 0)
            continue;

        if(strncmp(buff,"Executed Test: Functional",25) == 0)
        {
            // Write FTR
            // extract test name
            strString = buff;
            strString = strString.section(":",1).simplified();

            //======== Started Testsuite IDDQiddq23_0_flex ===================================
            strTestName = strStartTestSuite + " - " + strString;

            if(m_mapSitePartsInfo.contains(iSiteNumber))
                pPartInfo = m_mapSitePartsInfo[iSiteNumber];
            else
            {
                pPartInfo = new CVerigyEdfPartInfo();
                m_mapSitePartsInfo[iSiteNumber] = pPartInfo;
            }
            pPartInfo->nTotalTests++;

            bTestAlreadySaved = false;
            if(lstTestSavedWithLimit.contains(strTestName))
                bTestAlreadySaved = true;
            else
            {
                lstTestSavedWithLimit.append(strTestName);
                UpdateParameterIndexTable(strTestName);
            }


            // Compute Test# (add user-defined offset)
            iTestNumber = (long) lstTestSavedWithLimit.indexOf(strTestName);
            iTestNumber += GEX_TESTNBR_OFFSET_VERIGY_EDL;// Test# offset

            // Write FTR
            RecordReadInfo.iRecordType = 15;
            RecordReadInfo.iRecordSubType = 20;

            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteDword(iTestNumber);   // Test Number
            StdfFile.WriteByte(1);              // Test head
            StdfFile.WriteByte(iSiteNumber);    // Tester site:1,2,3,4 or 5, etc.
            if(bTestPass)
                bData = 0;                  // Test passed
            else
                bData = BIT7;               // Test Failed
            StdfFile.WriteByte(bData);      // TEST_FLG

            // save empty field for report_readfile.cpp
            StdfFile.WriteByte(255);    // opt_flg
            StdfFile.WriteDword(0);     // cycl_cnt
            StdfFile.WriteDword(0);     // rel_vadr
            StdfFile.WriteDword(0);     // rept_cnt
            StdfFile.WriteDword(0);     // num_fail
            StdfFile.WriteDword(0);     // xfail_ad
            StdfFile.WriteDword(0);     // yfail_ad
            StdfFile.WriteWord(0);      // vect_off
            StdfFile.WriteWord(0);      // rtn_icnt
            StdfFile.WriteWord(0);
            StdfFile.WriteWord(0);
            StdfFile.WriteString("");   // vect_name
            StdfFile.WriteString("");   // time_set
            StdfFile.WriteString("");   // op_code
            StdfFile.WriteString(strTestName.toLatin1().constData());	// test_txt: test name
            StdfFile.WriteString("");   // alarm_id
            StdfFile.WriteString("");   // prog_txt
            StdfFile.WriteString("");   // rslt_txt

            StdfFile.WriteRecord();
            bTestFunctional = false;
        }
        else if((strncmp(buff,"----",4) == 0)
                || (strncmp(buff,"Pass/Fail Limit",15) == 0))
        {
            // Check if it is a test result
            bool bSearchResults = false;
            bool bHavePassInfo = false;

            strString = "";

            if((strncmp(buff,"Pass/Fail Limit",15) == 0))
                goto labelReadLimits;

            // ending with FAILED or PASSED
            strString = buff;
            strString = strString.section(":",1).simplified();
            if(!strString.startsWith("PASSED", Qt::CaseInsensitive) && !strString.startsWith("FAILED", Qt::CaseInsensitive))
            {
                if(strncmp(buff,"-------- Results",16) == 0)
                {
                    // Here we can save some search MTR result
                    bSearchResults = true;
                }
                else if(strncmp(buff,"-------- Shmoo Spec",19) == 0)
                    continue;
                else if(strncmp(buff,"-------- Shmoo",14) == 0)
                {
                    // Here we can save some search MTR result
                    bTestShmoo = true;
                }
                else
                    continue;
            }
            else
            {
                bTestPass = strString.startsWith("PASSED", Qt::CaseInsensitive);
                bHavePassInfo = true;

                // -------- Functional: FAILED ----------------------------------------------------
                bTestFunctional = (strncmp(buff,"-------- Functional:",20) == 0);
            }

            //-------- IDDQ_DELTA@VDIG[1]: PASSED --------------------------------------------
            //IDDQ_DELTA@VDIG[1]: PASSED
            //Pass/Fail Limits: [-100.000000  ,   100.000000  ]
            //-------- Pin Results -----------------------------------------------------------
            //			VDIG PASSED -0.000023

            //-------- Stop at Vector 144: PASSED --------------------------------------------
            //Functional Pre-Test: PASSED
            //Sum of Currents:        36.162000 uA
            //Pass/Fail Limits: [-0.500000 mA,    0.500000 mA]
            //Measured Range:   [ 0.000303 mA ..  0.035859 mA]
            //-------- Pin Results -----------------------------------------------------------
            //             vdd PASSED  0.000303 mA
            //          vdd_io PASSED  0.035859 mA

            //-------- Stop at Vector 48914: FAILED ------------------------------------------
            //Functional Pre-Test: FAILED
            //Sum of Currents:     FAILED   241.072000 uA
            //Pass/Fail Limits: [ 0.000000 mA,    0.200000 mA]
            //Measured Range:   [ 0.045443 mA ..  0.195629 mA]
            //          vdd_io  0.045443 mA
            //             vdd  0.195629 mA

            //-------- Results ---------------------------------------------------------------
            //Passed transition:    P = 1.800000 V
            //Pass/Fail Limits: [ 0.000000 V,    4.500000 V]
            //Measured Value:     1.800000 V
            //Failed transition:    F = 1.900000 V
            //Pass/Fail Limits: [ 0.000000 V,    4.500000 V]
            //Measured Value:     1.900000 V

            // extract test name
            strString = buff;
            if(strncmp(buff,"-------- Results",16) == 0)
                strString = "";
            else if(strncmp(buff,"-------- Shmoo",14) == 0)
                strString = strString.remove("-").section("Shmoo",1).simplified();
            else
                strString = strString.remove("--").section(":",0,0).simplified();

            if(!ReadLine(fp,buff))
            {
                m_strLastError = "Unexpected end of file";
                m_iLastError = errInvalidFormat;
                return false;
            }
labelReadLimits:

            // Reset limits
            bHaveLowLimit = bHaveHighLimit = false;
            bStrictLowLimit = bStrictHighLimit = false;
            fLowLimit = fHighLimit=0.0F;
            nScale = 0;
            strUnit = "";

            //======== Started Testsuite IDDQiddq23_0_flex ===================================
            //-------- IDDQ_DELTA@VDIG[1]: PASSED --------------------------------------------
            strTestName = strStartTestSuite;
            if(!strString.isEmpty())
                strTestName += " - " + strString;

            if(m_mapSitePartsInfo.contains(iSiteNumber))
                pPartInfo = m_mapSitePartsInfo[iSiteNumber];
            else
            {
                pPartInfo = new CVerigyEdfPartInfo();
                m_mapSitePartsInfo[iSiteNumber] = pPartInfo;
            }
            pPartInfo->nTotalTests++;

            bTestAlreadySaved = false;
            if(lstTestSavedWithLimit.contains(strTestName))
                bTestAlreadySaved = true;
            else
            {
                lstTestSavedWithLimit.append(strTestName);
                UpdateParameterIndexTable(strTestName);
            }


            // Compute Test# (add user-defined offset)
            iTestNumber = (long) lstTestSavedWithLimit.indexOf(strTestName);
            iTestNumber += GEX_TESTNBR_OFFSET_VERIGY_EDL;// Test# offset

            // Goto Limit definition if any
            while(strncmp(buff,"Pass/Fail Limit",15) != 0)
            {
                // Try to search Limit Def
                // if any, have to go to the next step
                if(strncmp(buff,"======== Ended Testsuite",24)==0)
                    break;
                else if(   (strncmp(buff,"Executed Test",13)==0)
                           && (strncmp(buff,"Executed Test:",14)!=0))
                    break;
                else if(strncmp(buff,"-------- ",9) == 0)
                    break;
                else if(strncmp(buff,"Measured Value",14) == 0)
                    break;
                else if(strncmp(buff,"Passed/Failed transition",24) == 0)
                    break;
                else if(!bTestShmoo && ((strlen(buff) != 0) && (strncmp(buff,"   ",3) == 0)))
                    break;
                if(!ReadLine(fp,buff))
                {
                    m_strLastError = "Unexpected end of file";
                    m_iLastError = errInvalidFormat;
                    return false;
                }
            }

            strString = buff;
            //Pass/Fail Limits: [-100.000000  ,   100.000000  ]
            //-------- Pin Results -----------------------------------------------------------
            //			VDIG PASSED -0.000023
            // if already saved, jump to test result
            if((strncmp(buff,"Pass/Fail Limit",15) == 0)
                    && (!bTestAlreadySaved))
            {
                // read test limits
                //Pass/Fail Limits: [-100.000000  ,   100.000000  ]
                if(strncmp(buff,"Pass/Fail Limit:",16)==0)
                {
                    //Pass/Fail Limit:    0.000000    (Equal means passed)
                    bHaveLowLimit = bHaveHighLimit = false;
                    bStrictLowLimit = bStrictHighLimit = false;
                }
                else if(strncmp(buff,"Pass/Fail Limits:",17)==0)
                {
                    // have test limits
                    //Pass/Fail Limits: [ 0.100000  ,   545.000000  ]
                    //Pass/Fail Limits: ( 0.100000  ,   545.000000  )
                    //Pass/Fail Limits: [-0.001000 mA,    0.001000 mA]
                    strLeft="[";
                    strRight="]";
                    if(strString.indexOf("(")>=0 || strString.indexOf(")")>=0)
                    {
                        strLeft="(";
                        strRight=")";
                    }

                    strString = strString.section(":",1).simplified();
                    //[ -0.001000 mA , 0.001000 mA ]
                    if(strString.indexOf(strLeft)==0)
                        bStrictLowLimit = false;
                    else
                        bStrictLowLimit = true;

                    if(strString.indexOf(strRight) > 1)
                        bStrictHighLimit = false;
                    else
                        bStrictHighLimit = true;

                    strString = strString.remove(strLeft).remove(strRight);
                    // -0.001000 mA , 0.001000 mA
                    strLeft = strString.section(",",0,0).simplified();
                    strRight= strString.section(",",1).simplified();
                    //-0.001000 mA
                    fLowLimit = strLeft.section(" ",0,0).toFloat(&bHaveLowLimit);
                    fHighLimit = strRight.section(" ",0,0).toFloat(&bHaveHighLimit);
                }
            }

            bool bHaveFloatResult = false;
            QStringList lstResults;
            int nPos;

            //-------- Pin Results -----------------------------------------------------------
            while(strncmp(buff,"-------- ",9) != 0)
            {
                if(strncmp(buff,"Measured Value",14) == 0)
                {
                    // try to save the first value
                    strString = buff;
                    lstResults.append(strString.section(":",1).simplified());
                }
                if(strncmp(buff,"Passed/Failed transition",24) == 0)
                {
                    // try to save the 2 values
                    //-------- Results ---------------------------------------------------------------
                    //Passed/Failed transition:    P = 416.500 ns    F = 416.600 ns
                    strString = buff;
                    if(strString.contains("P = ") && strString.contains("F = "))
                    {
                        strString = strString.section("P =",1).simplified();
                        lstResults.append(strString.section("F =",0,0).simplified());
                        lstResults.append(strString.section("F =",1).simplified());
                    }
                }
                // Try to search Pin Results
                // if any, have to go to the next step
                if(strncmp(buff,"======== Ended Testsuite",24)==0)
                    break;
                else if(	(strncmp(buff,"Executed Test",13)==0)
                            &&	(strncmp(buff,"Executed Test:",14)!=0))
                    break;
                else if((strlen(buff) != 0) && (strncmp(buff,"   ",3) == 0))
                    break;
                if(!ReadLine(fp,buff))
                {
                    m_strLastError = "Unexpected end of file";
                    m_iLastError = errInvalidFormat;
                    return false;
                }
            }

            if( (strncmp(buff,"-------- Pin Results",20) != 0)
                    && !((strlen(buff) != 0) && (strncmp(buff,"   ",3) == 0)))

            {
                // No result found
            }
            else
            {
                //-------- Pin Results -----------------------------------------------------------
                //			VDIG PASSED -0.000023 mA
                //			     PASSED -0.000023 mA
                //			VDIG        -0.000023 mA
                //			VDIG        -0.000023
                if(strncmp(buff,"-------- Pin Results",20) == 0)
                {
                    lstResults.clear();
                    ReadLine(fp,buff);
                }

                while(strlen(buff) != 0)
                {
                    strString = QString(buff).simplified();
                    if(!bHavePassInfo)
                    {
                        if((strString.indexOf("PASSED",0,Qt::CaseInsensitive)>0)
                                || (strString.indexOf("FAILED",0,Qt::CaseInsensitive)>0))
                        {
                            bTestPass = bTestPass && (strString.indexOf("PASSED",0,Qt::CaseInsensitive)>0);
                        }
                    }
                    lstResults.append(strString);

                    if(!ReadLine(fp,buff))
                    {
                        m_strLastError = "Unexpected end of file";
                        m_iLastError = errInvalidFormat;
                        return false;
                    }
                }
            }


            if(!bSearchResults && (lstResults.isEmpty() || bTestFunctional || bTestShmoo))
            {
                int i;

                // Write FTR
                RecordReadInfo.iRecordType = 15;
                RecordReadInfo.iRecordSubType = 20;

                StdfFile.WriteHeader(&RecordReadInfo);
                StdfFile.WriteDword(iTestNumber);   // Test Number
                StdfFile.WriteByte(1);              // Test head
                StdfFile.WriteByte(iSiteNumber);    // Tester site:1,2,3,4 or 5, etc.
                if(bTestPass)
                    bData = 0;                  // Test passed
                else
                    bData = BIT7;               // Test Failed
                StdfFile.WriteByte(bData);      // TEST_FLG

                // save empty field for report_readfile.cpp
                StdfFile.WriteByte(255);    // opt_flg
                StdfFile.WriteDword(0);     // cycl_cnt
                StdfFile.WriteDword(0);     // rel_vadr
                StdfFile.WriteDword(0);     // rept_cnt
                StdfFile.WriteDword(0);     // num_fail
                StdfFile.WriteDword(0);     // xfail_ad
                StdfFile.WriteDword(0);     // yfail_ad
                StdfFile.WriteWord(0);      // vect_off

                StdfFile.WriteWord(lstResults.count()); // RTN_ICNT
                StdfFile.WriteWord(0);                  // PGM_ICNT
                for(i=0; i<lstResults.count(); i++)
                {
                    StdfFile.WriteWord(i);              // rtn_indx
                }
                for(i=0; i!=(lstResults.count()+1)/2; i++)
                    StdfFile.WriteByte(0);              // RTN_STAT

                StdfFile.WriteWord(0);
                StdfFile.WriteString("");   // vect_name
                StdfFile.WriteString("");   // time_set
                StdfFile.WriteString("");   // op_code
                StdfFile.WriteString(strTestName.toLatin1().constData());// test_txt: test name
                StdfFile.WriteString("");   // alarm_id
                StdfFile.WriteString("");   // prog_txt
                StdfFile.WriteString("");   // rslt_txt

                StdfFile.WriteRecord();
            }
            else if(!bSearchResults && (lstResults.count() == 1))
            {
                strString = lstResults.first();

                // found the good place for the float result
                for(nPos=3; nPos>=0; nPos--)
                {
                    strString.section(" ",nPos,nPos).toFloat(&bHaveFloatResult);
                    if(bHaveFloatResult)
                        break;
                }
                fResult = strString.section(" ",nPos,nPos).toFloat(&bHaveFloatResult);
                if(bHaveFloatResult)
                    strUnit = strString.section(" ",nPos+1,nPos+1);
                else
                    strUnit = "";

                if(!strUnit.isEmpty())
                    NormalizeLimits(strUnit, nScale);

                // Write PTR
                RecordReadInfo.iRecordType = 15;
                RecordReadInfo.iRecordSubType = 10;

                StdfFile.WriteHeader(&RecordReadInfo);
                StdfFile.WriteDword(iTestNumber);   // Test Number
                StdfFile.WriteByte(1);              // Test head
                StdfFile.WriteByte(iSiteNumber);    // Tester site:1,2,3,4 or 5, etc.
                if(bTestPass)
                    bData = 0;                  // Test passed
                else
                    bData = BIT7;               // Test Failed

                if(!bHaveFloatResult)
                    bData |= ~BIT1;

                StdfFile.WriteByte(bData);      // TEST_FLG
                bData = 0;
                if(bHaveLowLimit && !bStrictLowLimit)
                    bData |= BIT6;
                if(bHaveHighLimit && !bStrictHighLimit)
                    bData |= BIT7;
                StdfFile.WriteByte(bData);      // PARAM_FLG
                StdfFile.WriteFloat(fResult * GS_POW(10.0,nScale));// Test result
                if(!bTestAlreadySaved)
                {
                    // save Parameter name without unit information
                    StdfFile.WriteString(strTestName.toLatin1().constData());// TEST_TXT
                    StdfFile.WriteString("");                           // ALARM_ID

                    bData = 2;	// Valid data.
                    if(!bHaveLowLimit)
                        bData |= BIT6;
                    if(!bHaveHighLimit)
                        bData |= BIT7;
                    StdfFile.WriteByte(bData);                          // OPT_FLAG

                    StdfFile.WriteByte(-nScale);                        // RES_SCALE
                    StdfFile.WriteByte(-nScale);                        // LLM_SCALE
                    StdfFile.WriteByte(-nScale);                        // HLM_SCALE
                    StdfFile.WriteFloat(fLowLimit * GS_POW(10.0,nScale)); // LOW Limit
                    StdfFile.WriteFloat(fHighLimit * GS_POW(10.0,nScale));// HIGH Limit
                    StdfFile.WriteString(strUnit.toLatin1().constData());// Units
                }
                StdfFile.WriteRecord();
            }
            else if(!lstResults.isEmpty())
            {
                strString = lstResults.first();

                // found the good place for the float result
                for(nPos=3; nPos>=0; nPos--)
                {
                    strString.section(" ",nPos,nPos).toFloat(&bHaveFloatResult);
                    if(bHaveFloatResult)
                        break;
                }
                fResult = strString.section(" ",nPos,nPos).toFloat(&bHaveFloatResult);
                if(bHaveFloatResult)
                    strUnit = strString.section(" ",nPos+1,nPos+1);
                else
                    strUnit = "";

                if(!strUnit.isEmpty())
                    NormalizeLimits(strUnit, nScale);

                // Write MPR
                RecordReadInfo.iRecordType = 15;
                RecordReadInfo.iRecordSubType = 15;

                int	i;
                int	nPos;

                StdfFile.WriteHeader(&RecordReadInfo);
                StdfFile.WriteDword(iTestNumber);   // Test Number
                StdfFile.WriteByte(1);              // Test head
                StdfFile.WriteByte(iSiteNumber);    // Tester site

                if(bTestPass)
                    StdfFile.WriteByte(0);      // passed // TEST_FLG
                else
                    StdfFile.WriteByte(BIT7);   // failed // TEST_FLG

                bData = 0;
                if(bHaveLowLimit && !bStrictLowLimit)
                    bData |= BIT6;
                if(bHaveHighLimit && !bStrictHighLimit)
                    bData |= BIT7;
                StdfFile.WriteByte(bData);                  // PARAM_FLG
                StdfFile.WriteWord(lstResults.count());     // RTN_ICNT
                StdfFile.WriteWord(lstResults.count());     // RSLT_CNT
                for(i=0; i!=(lstResults.count()+1)/2; i++)
                    StdfFile.WriteByte(0);                  // RTN_STAT

                for(i=0; i<lstResults.count(); i++)
                {
                    strString = lstResults[i];
                    // found the good place for the float result
                    for(nPos=3; nPos>=0; nPos--)
                    {
                        strString.section(" ",nPos,nPos).toFloat(&bHaveFloatResult);
                        if(bHaveFloatResult)
                            break;
                    }
                    fResult = strString.section(" ",nPos,nPos).toFloat(&bHaveFloatResult);
                    if(strUnit.isEmpty() && bHaveFloatResult)
                        strUnit = strString.section(" ",nPos+1,nPos+1);

                    StdfFile.WriteFloat(fResult * GS_POW(10.0,nScale));   // Test result
                }

                StdfFile.WriteString(strTestName.toLatin1().constData());// TEST_TXT
                StdfFile.WriteString("");                               // ALARM_ID
                if(!bTestAlreadySaved)
                {
                    bData = BIT1|BIT2|BIT3;
                    if(!bHaveLowLimit)
                        bData |= BIT6;
                    if(!bHaveHighLimit)
                        bData |= BIT7;
                    StdfFile.WriteByte(bData);          // OPT_FLAG
                    StdfFile.WriteByte(-nScale);        // SCAL
                    StdfFile.WriteByte(-nScale);        // SCAL
                    StdfFile.WriteByte(-nScale);        // SCAL
                    StdfFile.WriteFloat(fLowLimit * GS_POW(10.0,nScale)); // LIMIT
                    StdfFile.WriteFloat(fHighLimit * GS_POW(10.0,nScale));// LIMIT


                    StdfFile.WriteFloat(0);             // StartIn
                    StdfFile.WriteFloat(0);             // IncrIn

                    for(i=0; i<lstResults.count(); i++)
                    {
                        StdfFile.WriteWord(i);          // rtn_indx
                    }
                    StdfFile.WriteString(strUnit.toLatin1().constData());// TEST_UNIT
                    StdfFile.WriteString("");           //
                    StdfFile.WriteString("");           //
                    StdfFile.WriteString("");           //
                    StdfFile.WriteString("");           //
                    StdfFile.WriteFloat(0);             // LIMIT
                    StdfFile.WriteFloat(0);             // LIMIT
                }

                StdfFile.WriteRecord();
            }
            lstResults.clear();
            bTestShmoo = false;
            bTestFunctional = false;
            strTestName = strUnit = "";
            iTestNumber = 0;

        }
        else if(strncmp(buff,"Site  TNum",10) == 0)
        {
            // Start with Site
            //Site  TNum      Test            TestSuite        p/f      Pin          LoLim              Measure         HiLim
            // Check if it is a test result
            bool bHavePassInfo = false;

            strString = buff;
            if(!strString.simplified().remove(" ").startsWith("SiteTNumTestTestSuitep/fPinLoLimMeasureHiLim",Qt::CaseInsensitive))
            {
                m_strLastError = buff;
                m_iLastError = errInvalidFormat;
                return false;
            }

            // IGNORE EMPTY FUNCTION TEST
            //Site  TNum      Test  TestSuite  p/f      Pin  LoLim  Measure  HiLim  TestFunc  VectLabel  T_Equ  T_Spec  T_Set  L_Equ  L_Spec  L_Set
            //---------------------------------------------------------------------------------------------------------------------------------------
            //---------------------------------------------------------------------------------------------------------------------------------------
            //TestSuite:  download   passed

            // SAVE AS PTR (no pin info)
            //Site  TNum      Test                  TestSuite         p/f      Pin  LoLim              Measure        HiLim             TestFunc  VectLabel   T_Equ  T_Spec  T_Set  L_Equ  L_Spec  L_Set
            //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
            //1     100       Idd_AVDRF[1]          SupplyShort_Test  passed   .    -1.000000 mA <=     0.140740 mA    <=  5.000000 mA  .         AdcPowerUp  1      1       1      2      1       1
            //1     250       Idd_VDDSFI[1]         SupplyShort_Test  passed   .    -1.000000 mA <=     0.704705 mA    <=  5.000000 mA  .         AdcPowerUp  1      1       1      2      1       1
            //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
            //TestSuite:  SupplyShort_Test  failed

            // SAVE AS FTR (no unit and same LoLim/HiLim)
            //Site  TNum      Test          TestSuite   p/f      Pin  LoLim           Measure      HiLim           TestFunc  VectLabel     T_Equ  T_Spec  T_Set  L_Equ  L_Spec  L_Set
            //-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
            //1     810       Pattern[1]    AdcPowerUp  passed   .     1.000000   ==   1.000000     == 1.000000    .         AdcPowerUpB0  1      1       1      1      1       1
            //1     811       DPS_State[1]  AdcPowerUp  passed   .     1.000000   ==   1.000000     == 1.000000    .         AdcPowerUpB0  1      1       1      1      1       1
            //-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
            //TestSuite:  AdcPowerUp  passed

            // SAVE AS MPR (pin info)
            //Site  TNum      Test            TestSuite        p/f      Pin          LoLim              Measure         HiLim              TestFunc  VectLabel  T_Equ  T_Spec  T_Set  L_Equ  L_Spec  L_Set
            //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
            //1     1100      passVolt_mV[1]  Continuity_Test  passed   ADC_HTEST1N  -800.000000 mV <   -471.771000 mV   < -200.000000 mV  .         error      1      1       1      99     1       1
            //1     1100      passVolt_mV[1]  Continuity_Test  passed   TXREFDIV1    -800.000000 mV <   -364.610000 mV   < -200.000000 mV  .         error      1      1       1      99     1       1
            //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
            //TestSuite:  Continuity_Test  passed

            // Goto results
            if(!ReadLine(fp,buff) || (strncmp(buff,"---------",9) != 0))
            {
                m_strLastError = buff;
                m_iLastError = errInvalidFormat;
                return false;
            }
            if(!ReadLine(fp,buff))
            {
                m_strLastError = "Unexpected end of file";
                m_iLastError = errInvalidFormat;
                return false;
            }
            if(strncmp(buff,"---------",9) == 0)
            {
                // No result
                // Read next line and exit
                ReadLine(fp,buff);
                continue;
            }
            // Read the line info
            bool bHaveFloatResult = false;
            QStringList lstResults;
            QStringList lstPinName;
            // Reset limits
            bHaveLowLimit = bHaveHighLimit = false;
            bStrictLowLimit = bStrictHighLimit = false;
            fLowLimit = fHighLimit=0.0F;
            nScale = 0;
            strUnit = "";

            while(strncmp(buff,"---------",9) != 0)
            {
                iIndex = 0;

                strString = buff;
                strString = strString.simplified();

                if(iTestNumber != strString.section(" ",1,1).toInt())
                {
                    iSiteNumber = strString.section(" ",iIndex,iIndex).toInt();iIndex++;
                    iTestNumber = strString.section(" ",iIndex,iIndex).toInt();iIndex++;
                    strTestName = strString.section(" ",iIndex,iIndex);iIndex++;
                    strStartTestSuite = strString.section(" ",iIndex,iIndex);iIndex++;
                }
                else
                    iIndex = 4;
                bTestPass = bHavePassInfo = (strString.section(" ",iIndex,iIndex).toLower()=="passed");iIndex++;
                strPinName = strString.section(" ",iIndex,iIndex);iIndex++;
                fLowLimit = strString.section(" ",iIndex,iIndex).toFloat(&bHaveLowLimit);iIndex++;
                strUnit = strString.section(" ",iIndex,iIndex);iIndex++;
                if((strUnit == "==") || (strUnit == "<") || (strUnit == "<="))
                {
                    // No Unit
                    bStrictLowLimit = !strUnit.contains("=");
                    strUnit = "";
                }
                else
                {
                    bStrictLowLimit = !strString.section(" ",iIndex,iIndex).contains("=");iIndex++;
                }
                strResult = strString.section(" ",iIndex,iIndex);iIndex++;
                strUnit = strString.section(" ",iIndex,iIndex);iIndex++;
                if((strUnit == "==") || (strUnit == "<") || (strUnit == "<="))
                {
                    // No Unit
                    bStrictHighLimit = !strUnit.contains("=");
                    strUnit = "";
                }
                else
                {
                    bStrictHighLimit = !strString.section(" ",iIndex,iIndex).contains("=");iIndex++;
                }
                fHighLimit = strString.section(" ",iIndex,iIndex).toFloat(&bHaveHighLimit);iIndex++;

                // Then
                if(strPinName == ".")
                    strPinName = "";
                if((fLowLimit == fHighLimit)
                        && !bStrictLowLimit && !bStrictHighLimit)
                    bTestFunctional = true;
                else
                {
                    lstPinName.append(strPinName);
                    lstResults.append(strResult);
                    fResult = strResult.toFloat(&bHaveFloatResult);
                }

                // Pin results
                if(!strPinName.isEmpty())
                {
                    // goto next results
                    if(!ReadLine(fp,buff))
                    {
                        m_strLastError = "Unexpected end of file";
                        m_iLastError = errInvalidFormat;
                        return false;
                    }
                    if(strncmp(buff,"---------",9) != 0)
                        continue;
                }


                if(m_mapSitePartsInfo.contains(iSiteNumber))
                    pPartInfo = m_mapSitePartsInfo[iSiteNumber];
                else
                {
                    pPartInfo = new CVerigyEdfPartInfo();
                    m_mapSitePartsInfo[iSiteNumber] = pPartInfo;
                }
                pPartInfo->nTotalTests++;

                //strTestName = strStartTestSuite + " - " + strTestName;
                strTestName += " <> " + strStartTestSuite;

                bTestAlreadySaved = false;
                if(lstTestSavedWithLimit.contains(strTestName))
                    bTestAlreadySaved = true;
                else
                {
                    lstTestSavedWithLimit.append(strTestName);
                    UpdateParameterIndexTable(strTestName);
                }

                if(lstResults.isEmpty() || bTestFunctional)
                {
                    int i;

                    // Write FTR
                    RecordReadInfo.iRecordType = 15;
                    RecordReadInfo.iRecordSubType = 20;

                    StdfFile.WriteHeader(&RecordReadInfo);
                    StdfFile.WriteDword(iTestNumber);   // Test Number
                    StdfFile.WriteByte(1);              // Test head
                    StdfFile.WriteByte(iSiteNumber);    // Tester site:1,2,3,4 or 5, etc.
                    if(bTestPass)
                        bData = 0;                  // Test passed
                    else
                        bData = BIT7;               // Test Failed
                    StdfFile.WriteByte(bData);      // TEST_FLG

                    // save empty field for report_readfile.cpp
                    StdfFile.WriteByte(255);    // opt_flg
                    StdfFile.WriteDword(0);     // cycl_cnt
                    StdfFile.WriteDword(0);     // rel_vadr
                    StdfFile.WriteDword(0);     // rept_cnt
                    StdfFile.WriteDword(0);     // num_fail
                    StdfFile.WriteDword(0);     // xfail_ad
                    StdfFile.WriteDword(0);     // yfail_ad
                    StdfFile.WriteWord(0);      // vect_off

                    StdfFile.WriteWord(lstResults.count()); // RTN_ICNT
                    StdfFile.WriteWord(0);                  // PGM_ICNT
                    for(i=0; i<lstResults.count(); i++)
                    {
                        StdfFile.WriteWord(i);              // rtn_indx
                    }
                    for(i=0; i!=(lstResults.count()+1)/2; i++)
                        StdfFile.WriteByte(0);              // RTN_STAT

                    StdfFile.WriteWord(0);
                    StdfFile.WriteString("");   // vect_name
                    StdfFile.WriteString("");   // time_set
                    StdfFile.WriteString("");   // op_code
                    StdfFile.WriteString(strTestName.toLatin1().constData());// test_txt: test name
                    StdfFile.WriteString("");   // alarm_id
                    StdfFile.WriteString("");   // prog_txt
                    StdfFile.WriteString("");   // rslt_txt

                    StdfFile.WriteRecord();
                }
                else if(bHaveFloatResult && strPinName.isEmpty() && (lstResults.count() == 1))
                {
                    strResult = lstResults.first();
                    fResult = strResult.toFloat(&bHaveFloatResult);
                    if(!strUnit.isEmpty())
                        NormalizeLimits(strUnit, nScale);

                    // Write PTR
                    RecordReadInfo.iRecordType = 15;
                    RecordReadInfo.iRecordSubType = 10;

                    StdfFile.WriteHeader(&RecordReadInfo);
                    StdfFile.WriteDword(iTestNumber);   // Test Number
                    StdfFile.WriteByte(1);              // Test head
                    StdfFile.WriteByte(iSiteNumber);    // Tester site:1,2,3,4 or 5, etc.
                    if(bTestPass)
                        bData = 0;                  // Test passed
                    else
                        bData = BIT7;               // Test Failed

                    if(!bHaveFloatResult)
                        bData |= ~BIT1;

                    StdfFile.WriteByte(bData);      // TEST_FLG
                    bData = 0;
                    if(bHaveLowLimit && !bStrictLowLimit)
                        bData |= BIT6;
                    if(bHaveHighLimit && !bStrictHighLimit)
                        bData |= BIT7;
                    StdfFile.WriteByte(bData);      // PARAM_FLG
                    StdfFile.WriteFloat(fResult * GS_POW(10.0,nScale));// Test result
                    if(!bTestAlreadySaved)
                    {
                        // save Parameter name without unit information
                        StdfFile.WriteString(strTestName.toLatin1().constData());// TEST_TXT
                        StdfFile.WriteString("");                           // ALARM_ID

                        bData = 2;	// Valid data.
                        if(!bHaveLowLimit)
                            bData |= BIT6;
                        if(!bHaveHighLimit)
                            bData |= BIT7;
                        StdfFile.WriteByte(bData);                          // OPT_FLAG

                        StdfFile.WriteByte(-nScale);                        // RES_SCALE
                        StdfFile.WriteByte(-nScale);                        // LLM_SCALE
                        StdfFile.WriteByte(-nScale);                        // HLM_SCALE
                        StdfFile.WriteFloat(fLowLimit * GS_POW(10.0,nScale)); // LOW Limit
                        StdfFile.WriteFloat(fHighLimit * GS_POW(10.0,nScale));// HIGH Limit
                        StdfFile.WriteString(strUnit.toLatin1().constData());// Units
                    }
                    StdfFile.WriteRecord();
                }
                else if(!lstResults.isEmpty())
                {
                    // Before to write MPR
                    // Write the PMR record
                    if(!bTestAlreadySaved)
                    {
                        RecordReadInfo.iRecordType = 1;
                        RecordReadInfo.iRecordSubType = 60;

                        foreach ( strPinName, lstPinName )
                        {
                            if(!lstPinNameIndex.contains(strPinName))
                            {
                                lstPinNameIndex.append(strPinName);

                                StdfFile.WriteHeader(&RecordReadInfo);
                                StdfFile.WriteWord(lstPinNameIndex.indexOf(strPinName));    // Pin Index
                                StdfFile.WriteWord(0);                                      // Channel type
                                StdfFile.WriteString(strPinName.toLatin1().constData());    // Channel name
                                StdfFile.WriteString(strPinName.toLatin1().constData());    // Physical name
                                StdfFile.WriteString(strPinName.toLatin1().constData());    // Logical name
                                StdfFile.WriteByte(1);                              // Test head
                                StdfFile.WriteByte(255);                            // Tester site (all)
                                StdfFile.WriteRecord();
                            }
                        }
                    }

                    if(!strUnit.isEmpty())
                        NormalizeLimits(strUnit, nScale);

                    // Write MPR
                    RecordReadInfo.iRecordType = 15;
                    RecordReadInfo.iRecordSubType = 15;

                    int i;
                    StdfFile.WriteHeader(&RecordReadInfo);
                    StdfFile.WriteDword(iTestNumber);   // Test Number
                    StdfFile.WriteByte(1);              // Test head
                    StdfFile.WriteByte(iSiteNumber);    // Tester site

                    if(bTestPass)
                        StdfFile.WriteByte(0);      // passed // TEST_FLG
                    else
                        StdfFile.WriteByte(BIT7);   // failed // TEST_FLG

                    bData = 0;
                    if(bHaveLowLimit && !bStrictLowLimit)
                        bData |= BIT6;
                    if(bHaveHighLimit && !bStrictHighLimit)
                        bData |= BIT7;
                    StdfFile.WriteByte(bData);                  // PARAM_FLG
                    StdfFile.WriteWord(lstResults.count());     // RTN_ICNT
                    StdfFile.WriteWord(lstResults.count());     // RSLT_CNT
                    for(i=0; i!=(lstResults.count()+1)/2; i++)
                        StdfFile.WriteByte(0);                  // RTN_STAT

                    for(i=0; i<lstResults.count(); i++)
                    {
                        strResult = lstResults[i];
                        fResult = strResult.toFloat(&bHaveFloatResult);
                        StdfFile.WriteFloat(fResult * GS_POW(10.0,nScale));   // Test result
                    }

                    StdfFile.WriteString(strTestName.toLatin1().constData());// TEST_TXT
                    StdfFile.WriteString("");                               // ALARM_ID
                    if(!bTestAlreadySaved)
                    {
                        bData = BIT1|BIT2|BIT3;
                        if(!bHaveLowLimit)
                            bData |= BIT6;
                        if(!bHaveHighLimit)
                            bData |= BIT7;
                        StdfFile.WriteByte(bData);          // OPT_FLAG
                        StdfFile.WriteByte(-nScale);        // SCAL
                        StdfFile.WriteByte(-nScale);        // SCAL
                        StdfFile.WriteByte(-nScale);        // SCAL
                        StdfFile.WriteFloat(fLowLimit * GS_POW(10.0,nScale)); // LIMIT
                        StdfFile.WriteFloat(fHighLimit * GS_POW(10.0,nScale));// LIMIT


                        StdfFile.WriteFloat(0);             // StartIn
                        StdfFile.WriteFloat(0);             // IncrIn

                        for(i=0; i<lstPinName.count(); i++)
                        {
                            strPinName = lstPinName[i];
                            StdfFile.WriteWord(lstPinNameIndex.indexOf(strPinName));// rtn_indx
                        }
                        StdfFile.WriteString(strUnit.toLatin1().constData());// TEST_UNIT
                        StdfFile.WriteString("");           //
                        StdfFile.WriteString("");           //
                        StdfFile.WriteString("");           //
                        StdfFile.WriteString("");           //
                        StdfFile.WriteFloat(0);             // LIMIT
                        StdfFile.WriteFloat(0);             // LIMIT
                    }

                    StdfFile.WriteRecord();
                }

                // Reset limits
                lstResults.clear();
                lstPinName.clear();
                bTestFunctional = false;
                bHaveLowLimit = bHaveHighLimit = false;
                bStrictLowLimit = bStrictHighLimit = false;
                fLowLimit = fHighLimit=0.0F;
                strTestName = strUnit = "";
                iTestNumber = 0;
                nScale = 0;

                if(strncmp(buff,"---------",9) == 0)
                    break;

                if(!ReadLine(fp,buff))
                {
                    m_strLastError = "Unexpected end of file";
                    m_iLastError = errInvalidFormat;
                    return false;
                }
            }
            //////////////////////////////////////////////////////////////////////
            // For ProgressBar
            if(GexProgressBar != NULL)
            {
                while((int) ftell(fp) > iNextFilePos)
                {
                    iProgressStep += 100/iFileSize + 1;
                    iNextFilePos  += iFileSize/100 + 1;
                    GexProgressBar->setValue(iProgressStep);
                }
            }
        }


        if(strncmp(buff,"PrintToDatalog",14) == 0)
        {
            //PrintToDatalog
        }
        else if(strncmp(buff,"Testflow started",16)==0)
        {
            strString = buff;

            //Testflow started on 06/11/07 at 00:44:52
            strString = strString.section(" started on ",1).section(" on ",0,0);
            m_lStartTime = GetDateTimeFromString(strString);
        }
        else if(strncmp(buff,"Testflow ended",14)==0)
        {
            strString = buff;

            // Testflow ended after 9.315 seconds on 06/11/07 at 00:44:51
            strString = strString.section(" on ",1).section(" on ",0,0);
            m_lStartTime = GetDateTimeFromString(strString);

            for(iSiteInfo = m_mapSitePartsInfo.begin(); iSiteInfo != m_mapSitePartsInfo.end(); iSiteInfo++)
            {
                pPartInfo = iSiteInfo.value();

                if(pPartInfo->nTotalTests == -1)
                    continue;

                if(pPartInfo->nXWafer == -32768)
                {
                    if(m_mapPartsIdExec.contains(pPartInfo->strPartId))
                        iTotalPartsRetested++;
                    else
                        m_mapPartsIdExec[pPartInfo->strPartId] = 0;
                    strPartNumber = pPartInfo->strPartId;
                }
                else
                {
                    if(m_mapPartsIdExec.contains(QString::number(pPartInfo->nXWafer) + " " + QString::number(pPartInfo->nYWafer)))
                        iTotalPartsRetested++;
                    else
                        m_mapPartsIdExec[QString::number(pPartInfo->nXWafer) + " " + QString::number(pPartInfo->nYWafer)] = m_mapPartsIdExec.count()+1;

                    //iPartNumber++;
                    strPartNumber = QString::number(m_mapPartsIdExec[QString::number(pPartInfo->nXWafer) + " " + QString::number(pPartInfo->nYWafer)]);
                    iSiteNumber = iSiteInfo.key();
                }

                // Write PRR
                RecordReadInfo.iRecordType = 5;
                RecordReadInfo.iRecordSubType = 20;
                StdfFile.WriteHeader(&RecordReadInfo);
                StdfFile.WriteByte(1);                  // Test head
                StdfFile.WriteByte(iSiteNumber);        // Tester site:1,2,3,4 or 5
                if(pPartInfo->bPassStatus == true)
                {
                    StdfFile.WriteByte(0);              // PART_FLG : PASSED
                    iTotalGoodBin++;
                }
                else
                {
                    StdfFile.WriteByte(8);              // PART_FLG : FAILED
                    iTotalFailBin++;
                }
                wSoftBin = wHardBin = pPartInfo->nBinning;
                StdfFile.WriteWord((WORD)pPartInfo->nTotalTests);// NUM_TEST
                StdfFile.WriteWord(wHardBin);           // HARD_BIN
                StdfFile.WriteWord(wSoftBin);           // SOFT_BIN
                StdfFile.WriteWord(pPartInfo->nXWafer); // X_COORD
                StdfFile.WriteWord(pPartInfo->nYWafer); // Y_COORD
                StdfFile.WriteDword(0);                 // No testing time known...
                StdfFile.WriteString(strPartNumber.toLatin1().constData());// PART_ID
                StdfFile.WriteString("");               // PART_TXT
                StdfFile.WriteString("");               // PART_FIX
                StdfFile.WriteRecord();

                // reset values
                pPartInfo->nTotalTests = -1;
                pPartInfo->bPassStatusValid = false;
                pPartInfo->nXWafer = -32768;
                pPartInfo->nYWafer = -32768;
                pPartInfo->strPartId = "";
                pPartInfo->nBinning = 0;
                pPartInfo->bPassStatus = false;
            }
        }
        else if(strncmp(buff,"Site",4)==0)
        {
            strString = buff;

            //Site 1 has PASSED. Binned to bin 1(1)
            strString = strString.toUpper();
            if(strString.indexOf("BINNED TO BIN")>0)
            {
                //Site 1 has PASSED. Binned to bin 1(1)
                iSiteNumber = strString.section(" ",1,1).toInt();
                if(m_mapSitePartsInfo.contains(iSiteNumber))
                    pPartInfo = m_mapSitePartsInfo[iSiteNumber];
                else
                {
                    pPartInfo = new CVerigyEdfPartInfo();
                    m_mapSitePartsInfo[iSiteNumber] = pPartInfo;
                }

                // Save only the first binning
                if(pPartInfo->bPassStatusValid)
                    continue;

                pPartInfo->bPassStatus = strString.section(" ",3,3).startsWith("PASSED", Qt::CaseInsensitive);
                pPartInfo->bPassStatusValid = true;
                pPartInfo->nBinning = strString.section('(',1).section(')',0,0).toInt();

                if(!m_mapBins.contains(pPartInfo->nBinning))
                {
                    CVerigyEdfBinInfo *pBin = new CVerigyEdfBinInfo();

                    m_mapBins[pPartInfo->nBinning] = pBin;
                    if(!pPartInfo->bPassStatus)
                        m_mapBins[pPartInfo->nBinning]->nPassFlag = TEST_PASSFLAG_FAIL;
                    else
                        m_mapBins[pPartInfo->nBinning]->nPassFlag = TEST_PASSFLAG_PASS;
                }
                if(!m_mapBins[pPartInfo->nBinning]->mapSiteNbCnt.contains(255))
                    m_mapBins[pPartInfo->nBinning]->mapSiteNbCnt[255]=0;
                m_mapBins[pPartInfo->nBinning]->mapSiteNbCnt[255]++;

                if(!m_mapBins[pPartInfo->nBinning]->mapSiteNbCnt.contains(iSiteNumber))
                    m_mapBins[pPartInfo->nBinning]->mapSiteNbCnt[iSiteNumber]=0;
                m_mapBins[pPartInfo->nBinning]->mapSiteNbCnt[iSiteNumber]++;

            }
            else
            {
                //Site 1: W2,23
                //Site 2: W2,22

                iSiteNumber = strString.section(":",0,0).section(" ",1).toInt();
                if(m_mapSitePartsInfo.contains(iSiteNumber))
                    pPartInfo = m_mapSitePartsInfo[iSiteNumber];
                else
                {
                    pPartInfo = new CVerigyEdfPartInfo();
                    m_mapSitePartsInfo[iSiteNumber] = pPartInfo;
                }

                if(strString.section(":",1).simplified().left(1).toUpper() == "W")
                {
                    pPartInfo->nXWafer = strString.section(":",1).remove("W").section(",",0,0).toInt();
                    pPartInfo->nYWafer = strString.section(":",1).remove("W").section(",",1).toInt();
                    if(!bWirWritten)
                    {
                        // Write WIR of new Wafer.
                        RecordReadInfo.iRecordType = 2;
                        RecordReadInfo.iRecordSubType = 10;
                        StdfFile.WriteHeader(&RecordReadInfo);
                        StdfFile.WriteByte(1);                          // Test head
                        StdfFile.WriteByte(255);                        // Tester site (all)
                        StdfFile.WriteDword(m_lStartTime);              // Start time
                        StdfFile.WriteString(m_strWaferId.toLatin1().constData());// WaferID
                        StdfFile.WriteRecord();
                        bWirWritten = true;
                    }
                }
                else if(strString.section(":",1).simplified().left(1).toUpper() == "P")
                {
                    pPartInfo->strPartId = strString.section("P",1).simplified();
                }

                // Write PIR
                // Write PIR for parts in this Wafer site

                RecordReadInfo.iRecordType = 5;
                RecordReadInfo.iRecordSubType = 10;
                StdfFile.WriteHeader(&RecordReadInfo);
                StdfFile.WriteByte(1);                      // Test head
                StdfFile.WriteByte(iSiteNumber);            // Tester site
                StdfFile.WriteRecord();

                pPartInfo->nTotalTests = 0;
            }

        }
        else if(strncmp(buff,"======== Started Testsuite",26)==0)
        {
            strString = buff;

            //======== Started Testsuite IDDQiddq23_0_flex ===================================
            strStartTestSuite = strString.section(" ",3,3);

            //////////////////////////////////////////////////////////////////////
            // For ProgressBar
            if(GexProgressBar != NULL)
            {
                while((int) ftell(fp) > iNextFilePos)
                {
                    iProgressStep += 100/iFileSize + 1;
                    iNextFilePos  += iFileSize/100 + 1;
                    GexProgressBar->setValue(iProgressStep);
                }
            }
        }
        else if(strncmp(buff,"======== Ended Testsuite",24)==0)
        {
            //======== End TMFuse_Blow ===========================================

        }
        else if(	(strncmp(buff,"Executed Test",13)==0)
                    &&	(strncmp(buff,"Executed Test:",14)!=0))
        {
            strString = buff;

            //Executed Test IDDQ,Production_Iddq on Site 1: PASSED
            // Check if it is the good format
            if(strString.contains("on Site",Qt::CaseInsensitive))
            {
                strString = strString.section("on Site ",1);
                iSiteNumber = strString.section(":",0,0).toInt();
                bTestPass = strString.endsWith("PASSED",Qt::CaseInsensitive);
            }

        }
        else if(strncmp(buff,"Equation Set",12)==0)
        {
            //Equation Set 26; Spec Set 1; Timing Set 1
            //Equation Set 4; Spec Set 2; Level Set 1

        }
        else if(strncmp(buff,"Startlabel",10)==0)
        {
            strString = buff;

            //Startlabel: iddq_fs_off_40
            strStartLabel = strString.section(":",1).simplified();
        }
        else
        {
            // Ignore
            strString = "";
        }
    }



    for(iSiteInfo = m_mapSitePartsInfo.begin(); iSiteInfo != m_mapSitePartsInfo.end(); iSiteInfo++)
        delete iSiteInfo.value();
    m_mapSitePartsInfo.clear();

    if(bWirWritten)
    {
        // Write WRR for last wafer inserted
        RecordReadInfo.iRecordType = 2;
        RecordReadInfo.iRecordSubType = 20;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);                      // Test head
        StdfFile.WriteByte(255);                    // Tester site (all)
        StdfFile.WriteDword(0);                     // Time of last part tested
        StdfFile.WriteDword(m_mapPartsIdExec.count() + iTotalPartsRetested);// Parts tested: always 5
        StdfFile.WriteDword(iTotalPartsRetested);   // Parts retested
        StdfFile.WriteDword(0);                     // Parts Aborted
        StdfFile.WriteDword(iTotalGoodBin);         // Good Parts
        StdfFile.WriteDword(4294967295UL);          // Functionnal Parts
        StdfFile.WriteString(m_strWaferId.toLatin1().constData());// WaferID
        StdfFile.WriteRecord();
    }

    m_mapPartsIdExec.clear();

    int nTotalParts = 0;
    int nPassParts = 0;

    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 40;
    QMap<int,CVerigyEdfBinInfo*>::Iterator itMapBin;
    QMap<int,int>::Iterator itMapSite;
    for ( itMapBin = m_mapBins.begin(); itMapBin != m_mapBins.end(); ++itMapBin )
    {
        for(itMapSite = itMapBin.value()->mapSiteNbCnt.begin(); itMapSite != itMapBin.value()->mapSiteNbCnt.end(); itMapSite++)
        {
            // Write HBR
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(itMapSite.key()==255 ? 255 : 1); // Test Head = ALL
            StdfFile.WriteByte(itMapSite.key());                // Test sites = ALL
            StdfFile.WriteWord(itMapBin.key());                 // HBIN = 0
            StdfFile.WriteDword(itMapSite.value());              // Total Bins

            if(itMapSite.key()==255)
            {
                nTotalParts += itMapSite.value();
                if(itMapBin.value()->nPassFlag == TEST_PASSFLAG_PASS)
                    nPassParts += itMapSite.value();
            }

            if(itMapBin.value()->nPassFlag == TEST_PASSFLAG_UNKNOWN)
                StdfFile.WriteByte(' ');
            else if(itMapBin.value()->nPassFlag == TEST_PASSFLAG_PASS)
                StdfFile.WriteByte('P');
            else
                StdfFile.WriteByte('F');
            StdfFile.WriteString("");
            StdfFile.WriteRecord();
        }
    }

    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 50;

    for ( itMapBin = m_mapBins.begin(); itMapBin != m_mapBins.end(); ++itMapBin )
    {
        for(itMapSite = itMapBin.value()->mapSiteNbCnt.begin(); itMapSite != itMapBin.value()->mapSiteNbCnt.end(); itMapSite++)
        {
            // Write SBR
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(itMapSite.key()==255 ? 255 : 1); // Test Head = ALL
            StdfFile.WriteByte(itMapSite.key());                // Test sites = ALL
            StdfFile.WriteWord(itMapBin.key());                 // SBIN = 0
            StdfFile.WriteDword(itMapSite.value());              // Total Bins
            if(itMapBin.value()->nPassFlag == TEST_PASSFLAG_UNKNOWN)
                StdfFile.WriteByte(' ');
            else if(itMapBin.value()->nPassFlag == TEST_PASSFLAG_PASS)
                StdfFile.WriteByte('P');
            else
                StdfFile.WriteByte('F');
            StdfFile.WriteString("");
            StdfFile.WriteRecord();
        }
    }

    // Write PCR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 30;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(255);					// Test Head = ALL
    StdfFile.WriteByte(255);					// Test sites = ALL
    StdfFile.WriteDword(nTotalParts);			// Total Parts tested
    StdfFile.WriteDword(0);						// Total Parts re-tested
    StdfFile.WriteDword(0);						// Total Parts aborted
    StdfFile.WriteDword(nPassParts);			// Total GOOD Parts
    StdfFile.WriteRecord();

    // Write MRR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteDword(m_lStartTime);          // File finish-time.
    StdfFile.WriteRecord();

    // Close STDF file.
    StdfFile.Close();

    // Success
    return true;
}

//////////////////////////////////////////////////////////////////////
// Check if File is compatible with VERIGY_EDF format
//////////////////////////////////////////////////////////////////////
bool CGVERIGY_EDFtoSTDF::IsCompatible(const char *szFileName)
{
    QString strString;

    // Open hCsmFile file
    QFile f( szFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening Edf file
        return false;
    }
    // Assign file I/O stream
    QTextStream hEdfFile(&f);

    // Check the first 20 lines
    int nLine = 0;
    while(!hEdfFile.atEnd())
    {
        strString = hEdfFile.readLine();
        if(strString.startsWith("Testflow started on", Qt::CaseInsensitive))
        {
            f.close();
            return true;
        }
        if(strString.startsWith("======== Started", Qt::CaseInsensitive))
        {
            f.close();
            return true;
        }
        if(nLine > 20)
        {
            // Incorrect header...this is not a Edf file!
            f.close();
            return false;
        }
        // Ignore empty lines
        if(strString.isEmpty())
            continue;

        // Ignore line PrintToDatalog
        if(strString.startsWith("PrintToDatalog", Qt::CaseInsensitive))
            continue;

        nLine ++;
    }

    f.close();

    return false;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' VERIGY_EDF file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGVERIGY_EDFtoSTDF::Convert(const char *VerigyEdfFileName, const char *strFileNameSTDF)
{
    // No erro (default)
    m_iLastError = errNoError;
    m_strLastError = "";

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(VerigyEdfFileName);
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
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(VerigyEdfFileName).fileName()+"...");
            GexScriptStatusLabel->show();
        }
    }
    QCoreApplication::processEvents();

    if(ReadVerigyEdfFile(VerigyEdfFileName,strFileNameSTDF) != true)
    {
        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if((GexProgressBar != NULL)
                && bHideProgressAfter)
            GexProgressBar->hide();

        if((GexScriptStatusLabel != NULL)
                && bHideLabelAfter)
            GexScriptStatusLabel->hide();
        return false;	// Error reading VERIGY_EDF file
    }

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
//return lDateTime from string strDateTime
// format: 07/06/09 at 17:51:02
// format: 05/03/2012 at 08:51:31 AM
//////////////////////////////////////////////////////////////////////
long CGVERIGY_EDFtoSTDF::GetDateTimeFromString(QString strDateTime)
{
    int     nYear, nMonth, nDay;
    int     nHour, nMin, nSec;
    long    lDateTime;

    if(strDateTime.length()<20)
        return 0;

    QString strDate = strDateTime.section(" at ",0,0);
    QString strTime = strDateTime.section(" at ",1);

    nMonth = strDate.mid(0,2).toInt();
    nDay = strDate.mid(3,2).toInt();
    nYear = strDate.mid(6,4).toInt();
    if(nYear < 100)
        nYear += 2000;
    nHour = strTime.mid(0,2).toInt();
    nMin= strTime.mid(3,2).toInt();
    nSec = strTime.mid(6,2).toInt();

    if(strTime.endsWith("PM",Qt::CaseInsensitive))
        nHour += 12;

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
int CGVERIGY_EDFtoSTDF::ReadLine(FILE *fp, char *bp)
{
    char c = '\0';
    int i = 0;

    if(feof(fp))
    {
        bp[i] = '\0';
        return 0;
    }

    // Read one line from the source file
    while( (c = getc(fp)) != '\n' )
    {
        if( c == EOF )         /* return false on unexpected EOF */
        {
            bp[i] = '\0';
            return 0;
        }
        if(i < 2048)
            bp[i++] = c;
    }
    bp[i] = '\0';

    QCoreApplication::processEvents();
    return 1;
}
