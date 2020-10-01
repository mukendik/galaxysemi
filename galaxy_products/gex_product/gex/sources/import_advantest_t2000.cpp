//////////////////////////////////////////////////////////////////////
// import_advantest_t2000.cpp: Convert a advantest_t2000 file to STDF V4.0
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

#include "import_advantest_t2000.h"
#include "import_constants.h"
#include "engine.h"
#include "gqtl_global.h"

// File format: Data log
//***** Test Plan [qc_sc1x] Start at 12/07/2007 04:06:16 *****
//  Device Type        : qc_sc1x_bga                       Performance Board ID: -----
//  Site Controller ID : 1
//  Lot Number         :                                   Sample Number       : 22
//
//
//***** ID: 300 [AnPLL_set13_Pr1V384Po2_mf_pllout_PD] *****
//  DUT  Result     Range    Item      Value     Judge Hi  Judge Lo  Pin Name
//    2  PASS       M20MHz   Tmin      51.87nS   52.50nS   50.50nS   GPIO_30
//    2  PASS       M20MHz   Tmean     51.97nS   52.50nS   50.50nS   GPIO_30
//    2  PASS       M20MHz   Tmax      52.09nS   52.50nS   50.50nS   GPIO_30
//    2  PASS       M20MHz   StdDev    46.69pS   500.0pS   1.000pS   GPIO_30
//    2  PASS       M20MHz   Tpkpk     215.5pS   1.000nS   1.000pS   GPIO_30
//    2  PASS       M20MHz   Freq      19.24MHz  500.0MHz  1.000MHz  GPIO_30
//    3  PASS       M20MHz   Tmin      51.82nS   52.50nS   50.50nS   GPIO_30
//    3  PASS       M20MHz   Tmean     51.97nS   52.50nS   50.50nS   GPIO_30
//    3  PASS       M20MHz   Tmax      52.14nS   52.50nS   50.50nS   GPIO_30
//    3  PASS       M20MHz   StdDev    76.65pS   500.0pS   1.000pS   GPIO_30
//    3  PASS       M20MHz   Tpkpk     324.8pS   1.000nS   1.000pS   GPIO_30
//    3  PASS       M20MHz   Freq      19.24MHz  500.0MHz  1.000MHz  GPIO_30
//
//***** ID: 300 [AnPLL19p2Pr1V384Po1D01MN18_chipx16_gpio30_PD] *****
//  DUT  Result     Range    Item      Value     Judge Hi  Judge Lo  Pin Name
//    2  PASS       M20MHz   Tmin      50.50nS   52.85nS   50.35nS   GPIO_30
//    2  PASS       M20MHz   Tmean     50.71nS   52.85nS   50.35nS   GPIO_30
//    2  PASS       M20MHz   Tmax      51.94nS   52.85nS   50.35nS   GPIO_30
//    2  PASS       M20MHz   StdDev    301.7pS   500.0pS   1.000pS   GPIO_30
//    2  PASS       M20MHz   Tpkpk     1.438nS   2.000nS   1.000pS   GPIO_30
//    2  PASS       M20MHz   Freq      19.72MHz  500.0MHz  1.000MHz  GPIO_30
//    3  PASS       M20MHz   Tmin      50.50nS   52.85nS   50.35nS   GPIO_30
//    3  PASS       M20MHz   Tmean     50.75nS   52.85nS   50.35nS   GPIO_30
//    3  PASS       M20MHz   Tmax      52.09nS   52.85nS   50.35nS   GPIO_30
//    3  PASS       M20MHz   StdDev    322.4pS   500.0pS   1.000pS   GPIO_30
//    3  PASS       M20MHz   Tpkpk     1.595nS   2.000nS   1.000pS   GPIO_30
//    3  PASS       M20MHz   Freq      19.71MHz  500.0MHz  1.000MHz  GPIO_30
//
//
//***** Test Plan [qc_sc1x] End at 12/07/2007 04:06:17 *****
//Total Result
//  DUT 1    DUT 2    DUT 3    DUT 4
//  ----     PASS     PASS     ----
//
// File format: Data std
//***** Test Plan [qc_sc1x] Start at 12/07/2007 04:06:16 *****
//  Device Type        : qc_sc1x_bga                       Performance Board ID: -----
//  Site Controller ID : 1
//  Lot Number         :                                   Sample Number       : 22
//
//
//Started at: 20070915 000915
//
//testflow: G92_SnglTP_FLOW
//
//test_program: g92_a02_ws_T2KMe_eng_00_1
//
//DutId : 1
//SiteCId : 1
//AT_PROBER_X_ADDRESS : -1
//AT_PROBER_Y_ADDRESS : 14
//device : g92
//device_revision : A02
//test_revision : 00.1
//TEST_STAGE : CP
//log_filename_prefix : g92.a02.discrete
//log_directory : C:/temp/u337user
//Test ID    Test Description                   Index  Result    Value     JudgeHigh JudgeLow  DUT  Pin
//1190001    Vdd_iddq_test                           1 PASS         3.168A    30.00A   1.000uA    1 VDD
//_param_ Vdd_iddq_test_plist_g92_a01_jtag_iddq@1.05_ 3168.00000 : DUT 1
//
//1290001    Fbvddq_iddq_test                        1 PASS        329.6mA    4.000A   1.000uA    1 FBVDDQ
//_param_ Fbvddq_iddq_test_plist_g92_a01_jtag_iddq@1.8_ 329.60000 : DUT 1
//
//1290002    iddq_others_test                        1 PASS        23.00mA   200.0mA   1.000uA    1 PEX_IOVDD
//1290002    iddq_others_test                        1 PASS        14.88mA   200.0mA   1.000uA    1 VDD33
//_param_ iddq_others_test_plist_g92_a01_jtag_iddq@1.10_ 23.0 : DUT 1
//
//_param_ iddq_others_test_plist_g92_a01_jtag_iddq@3.30_ 14.88000 : DUT 1
//DutId : 1
//SiteCId : 1
//Ended at: 20070915 000946
//
//***** Test Plan [g92_a02_ws_T2KMe_eng_00_1] End at 09/15/2007 00:09:46 *****
//Total Result
//  DUT 1
//  PASS
//
//***** Test Plan [g92_a02_ws_T2KMe_eng_00_1] Start at 09/15/2007 00:11:19 *****

// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar

#define BIT0			0x01
#define BIT1			0x02
#define BIT2			0x04
#define BIT3			0x08
#define BIT4			0x10
#define BIT5			0x20
#define BIT6			0x40
#define BIT7			0x80

#define	C_INFINITE	(float) 1e32

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGADVANTEST_T2000toSTDF::CGADVANTEST_T2000toSTDF()
{
    m_bFlagGenerateMpr = false;
    m_lStartTime = 0;
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGADVANTEST_T2000toSTDF::GetLastError()
{
    m_strLastError = "Import ADVANTEST_T2000: ";

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
// Normalize test limits when writing into PTR.
//////////////////////////////////////////////////////////////////////
void CGADVANTEST_T2000toSTDF::NormalizeValues(QString &strUnits,float &fValue, int &nScale, bool &bIsNumber)
{
    int i;
    QString strValue = strUnits;

    // In strValue, the current value with unit
    if(strValue.startsWith("-"))
        i = 6;
    else
        i = 5;
    if(strValue.length() <= i)
    {
        // no unit
        strUnits = "";
    }
    else
    {
        strUnits = strValue.right(strValue.length()-i);
        strValue = strValue.left(i);
    }

    fValue = strValue.toFloat(&bIsNumber);
    nScale=0;

    if(strUnits.length() <= 1)
    {
        // units too short to include a prefix, then keep it 'as-is'
        return;
    }

    QChar cPrefix = strUnits[0];
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
    fValue *= GS_POW(10.0,nScale);
    if(nScale)
        strUnits = strUnits.mid(1);	// Take all characters after the prefix.
}

//////////////////////////////////////////////////////////////////////
// Check if File is compatible with ADVANTEST_T2000 format
//////////////////////////////////////////////////////////////////////
bool CGADVANTEST_T2000toSTDF::IsCompatible(const char *szFileName)
{
    QString strString;
    QString strSection;
    QString strValue;

    // Open hAdvantestT2000File file
    QFile f( szFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening AdvantestT2000 file
        return false;
    }
    // Assign file I/O stream
    QTextStream hAdvantestT2000File(&f);

    // Check if first line is the correct ADVANTEST_T2000 header...
    //***** Test Plan [qc_sc1x] Start at 12/07/2007 04:06:16 *****
    //  Device Type        : qc_sc1x_bga                       Performance Board ID: -----
    //  Site Controller ID : 1
    //  Lot Number         :                                   Sample Number       : 22
    //
    //
    //***** ID: 300 [AnPLL_set13_Pr1V384Po2_mf_pllout_PD] *****

    do
        strString = hAdvantestT2000File.readLine();
    while(!strString.isNull() && strString.isEmpty());

    // Close file
    f.close();

    if(strString.startsWith("***** Test Plan", Qt::CaseInsensitive))
        return true;
    else
        return false;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the ADVANTEST_T2000 file
//////////////////////////////////////////////////////////////////////
bool CGADVANTEST_T2000toSTDF::ReadAdvantestT2000File(const char *AdvantestT2000FileName,const char *strFileNameSTDF)
{
    QString strString;
    QString strSection;
    bool	bIsNumber = false;

    // Open ADVANTEST_T2000 file
    QFile f( AdvantestT2000FileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening ADVANTEST_T2000 file
        m_iLastError = errOpenFail;

        // Convertion failed.
        return false;
    }

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    iFileSize = f.size() + 1;

    // Assign file I/O stream
    QTextStream hAdvantestT2000File(&f);

    // Check if first line is the correct ADVANTEST_T2000 header...
    //***** Test Plan [qc_sc1x] Start at 12/07/2007 04:06:16 *****

    m_iLastError = errInvalidFormat;
    // Read first non-empty line
    strString = ReadLine(hAdvantestT2000File);
    if(strString.isNull())
    {
        // Close file
        f.close();
        return false;
    }

    // Incorrect header...this is not a ADVANTEST_T2000 file!
    // Convertion failed.
    if(!strString.startsWith("***** Test Plan [", Qt::CaseInsensitive))
    {
        // Close file
        f.close();
        return false;
    }

    strString = strString.toLower().section("start at ",1).remove('*').simplified();
    QString strDate = strString.mid(6,4) + "/" + strString.left(5) + "T" + strString.right(8);
    QDateTime clDateTime;
    clDateTime.setTimeSpec(Qt::UTC);
    m_lStartTime = clDateTime.fromString(strDate, Qt::ISODate).toTime_t();

    strString = ReadLine(hAdvantestT2000File).trimmed();
    // Read ADVANTEST_T2000 information

    while(!hAdvantestT2000File.atEnd())
    {
        if(strString.isEmpty())
            strString = ReadLine(hAdvantestT2000File).trimmed();

        // Read header
        if(strString.startsWith("***** Test Plan [", Qt::CaseInsensitive))
        {
            //***** Test Plan [qc_sc1x] Start at 12/07/2007 04:06:16 *****
            //***** Test Plan [qc_sc1x] End at 12/07/2007 04:09:44 *****

            strString = "";
            continue;
        }
        else if(strString.startsWith("TEST ID", Qt::CaseInsensitive))
        {
            // Read the first run to save all pin name for MPR parameter
            // only if m_bFlagGenerateMpr
            // else break;
            if(!m_bFlagGenerateMpr)
                break;

            strString = "";
            bIsNumber = false;
            while(!hAdvantestT2000File.atEnd())
            {
                if(strString.isEmpty())
                {
                    hAdvantestT2000File.skipWhiteSpace();
                    strString = ReadLine(hAdvantestT2000File);
                }

                strString.mid(0,11).simplified().toInt(&bIsNumber);

                // Read header
                if(bIsNumber)
                {
                    // Test line result
                    strSection = "TEST ID";
                }
                else if(strString.startsWith("_param_", Qt::CaseInsensitive))
                {
                    strString = "";
                    continue;
                }
                else
                    break;


                if(strSection == "TEST ID")
                {
                    // Test Result
                    //Test ID    Test Description                   Index  Result    Value     JudgeHigh JudgeLow  DUT  Pin
                    //1190001    Vdd_iddq_test                           1 PASS         3.168A    30.00A   1.000uA    1 VDD
                    //                                                   1 PASS         3.168A    30.00A   1.000uA    2 VDD
                    //6590001    mbistbypass_vmin_srh                    1 PASS        706.0mV      None      None    1

                    if(!strString.mid(98).simplified().isEmpty())
                    {
                        // MPR
                        if(!m_qMapPinIndex.contains(strString.mid(98).simplified()))
                        {
                            m_qMapPinIndex[strString.mid(98).simplified()] = m_qMapPinIndex.count()+1;
                        }
                    }
                    // SKIP OTHER SITE RESULT
                    while(!hAdvantestT2000File.atEnd())
                    {
                        strString = ReadLine(hAdvantestT2000File);
                        if(!strString.mid(0,11).simplified().isEmpty())
                            break;
                    }
                }
                else
                    strString = "";
            }
            break;
        }
        else
        {
            strSection = strString.section(":",0,0).simplified().toUpper();
            strString = strString.section(":",1);
        }

        if(strSection == "LOT NUMBER")
        {
            m_strLotId = strString.left(30).simplified();
            strString = strString.mid(30);
        }
        else if(strSection == "SAMPLE NUMBER")
        {// ignore
            strString = "";
        }
        else if(strSection == "DEVICE TYPE")
        {
            m_strDeviceId = strString.left(30).simplified();
            strString = strString.mid(30);
        }
        else if(strSection == "TEST_PROGRAM")
        {
            m_strProgramId = strString.left(30).simplified();
            strString = strString.mid(30);
        }
        else if(strSection == "STATION")
        {
            m_strStationId = strString.left(4).simplified();
            strString = strString.mid(4);
        }
        else if(strSection == "SITE CONTROLLER ID")
        {// ignore
            strString = "";
        }
        else if(strSection == "AT_PROBER_X_ADDRESS")
        {
            m_strWaferId = "1";
            strString = "";
        }
        else if(strSection == "***** ID")
        {
            // Check if it is stats info
            //***** ID: 300 [AnPLL_set13_Pr1V384Po2_mf_pllout_PD] *****
            // NO PASS FAIL in this line
            if((strString.contains("PASS",Qt::CaseInsensitive)) || (strString.contains("FAIL",Qt::CaseInsensitive)))
            {
                strString = "";
                continue;
            }

            break;
        }
        else
        {
            // ignore
            strString = "";
        }
    }

    // Convertion failed.
    if(hAdvantestT2000File.atEnd())
    {
        // Close file
        f.close();
        return false;
    }

    //Restart at the beggining of the file
    hAdvantestT2000File.seek(0);

    if(WriteStdfFile(hAdvantestT2000File,strFileNameSTDF) != true)
    {
        QFile::remove(strFileNameSTDF);
        // Close file
        f.close();
        return false;
    }

    // Success parsing ADVANTEST_T2000 file
    m_iLastError = errNoError;
    return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from ADVANTEST_T2000 data parsed
//////////////////////////////////////////////////////////////////////
bool CGADVANTEST_T2000toSTDF::WriteStdfFile(QTextStream &hAdvantestT2000File,const char *strFileNameSTDF)
{
    // now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open((char*)strFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        // Failed importing ADVANTEST_T2000 file into STDF database
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
    StdfFile.WriteString(m_strDeviceId.toLatin1().constData());	// Part Type / Product ID
    StdfFile.WriteString("");					// Node name
    StdfFile.WriteString("");					// Tester Type
    StdfFile.WriteString(m_strProgramId.toLatin1().constData());		// Job name
    StdfFile.WriteString("");					// Job rev
    StdfFile.WriteString("");					// sublot-id
    StdfFile.WriteString("");					// operator
    StdfFile.WriteString("");					// exec-type
    StdfFile.WriteString("");					// exe-ver
    StdfFile.WriteString("");				// test-cod
    StdfFile.WriteString("");					// test-temperature
    // Construct custom Galaxy USER_TXT
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    strUserTxt += ":ADVANTEST_T2000";
    StdfFile.WriteString(strUserTxt.toLatin1().constData());		// user-txt
    StdfFile.WriteString("");					// aux-file
    StdfFile.WriteString("");					// package-type
    StdfFile.WriteString(m_strDeviceId.toLatin1().constData());	// familyID
    StdfFile.WriteString("");					// Date-code
    StdfFile.WriteString("");					// Facility-ID
    StdfFile.WriteString("");					// FloorID
    StdfFile.WriteString("");					// ProcessID

    StdfFile.WriteRecord();

    QString strString, strValue, strSection;
    QMap<QString,int>::Iterator itStringInt;
    QMap<QString, float>::Iterator itStringFloat;
    QString			strTestName;
    QString			strTestDescription;
    int				nTestNum;
    float			fValue;
    int				nScale;
    QString			strUnit;
    QString			strPinName;
    int				iPinIndex;
    int				nSiteId;
    QStringList		lstSites;
    QStringList::Iterator itSite;
    CGAdvantestT2000Parameter *ptTest;
    BYTE			bData;
    bool			bPassStatus;
    bool			bIsNumber;
    int				iBin, iTotalTests, iPartNumber;
    int				iTotalGoodBin, iTotalFailBin, iTotalPart;

    ptTest = NULL;
    bData = 0;
    bPassStatus = bIsNumber = false;
    iPinIndex = iBin = nSiteId = nTestNum = iPartNumber = 0;
    iTotalTests = iTotalPart = iTotalFailBin = iTotalGoodBin = 0;

    // Write PMR record if any
    if(m_qMapPinIndex.count() > 0)
    {

        for(itStringInt=m_qMapPinIndex.begin(); itStringInt!=m_qMapPinIndex.end(); itStringInt++)
        {
            strPinName = itStringInt.key();
            iPinIndex = itStringInt.value();

            // Write PMR pin info.
            RecordReadInfo.iRecordType = 1;
            RecordReadInfo.iRecordSubType = 60;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteWord(iPinIndex);						// Pin Index
            StdfFile.WriteWord(0);								// Channel type
            StdfFile.WriteString(strPinName.toLatin1().constData());	// Channel name
            StdfFile.WriteString("");							// Physical name
            StdfFile.WriteString("");							// Logical name
            StdfFile.WriteByte(1);								// Test head
            StdfFile.WriteByte(255);							// Tester site (all)
            StdfFile.WriteRecord();
        }

    }

    if(!m_strWaferId.isEmpty())
    {
        // Write WIR of new Wafer.
        RecordReadInfo.iRecordType = 2;
        RecordReadInfo.iRecordSubType = 10;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);									// Test head
        StdfFile.WriteByte(255);								// Tester site (all)
        StdfFile.WriteDword(m_lStartTime);						// Start time
        StdfFile.WriteString(m_strWaferId.toLatin1().constData());	// WaferID
        StdfFile.WriteRecord();
    }

    // Write Test results for each PartID
    while(!hAdvantestT2000File.atEnd())
    {
        if(strString.isEmpty())
            strString = ReadLine(hAdvantestT2000File).trimmed();

        // Read header
        if(strString.startsWith("***** Test Plan [", Qt::CaseInsensitive))
        {
            //***** Test Plan [qc_sc1x] Start at 12/07/2007 04:06:16 *****
            //***** Test Plan [qc_sc1x] End at 12/07/2007 04:09:44 *****

            strString = strString.toLower().section("start at ",1).remove('*').simplified();
            if(strString.isEmpty())
            {
                //***** Test Plan [qc_sc1x] End at 12/07/2007 04:09:44 *****
                //Total Result
                //  DUT 1    DUT 2    DUT 3    DUT 4
                //  ----     PASS     PASS     ----

                // Read 3 next line for pass status
                strString = ReadLine(hAdvantestT2000File);
                strString = ReadLine(hAdvantestT2000File);
                strString = ReadLine(hAdvantestT2000File);


                // end of PartId tested
                // write PIR PTR PTR ... PRR for each Part/Site

                for(itSite=lstSites.begin(); itSite!=lstSites.end(); itSite++)
                {
                    nSiteId = (*itSite).toInt();

                    // In strString, the pass status for all sites
                    m_clPartsInfo.m_bPass[nSiteId] = strString.mid(((nSiteId - 1)*10), 9).trimmed().startsWith("PASS", Qt::CaseInsensitive);

                    // Write PIR for parts in this Wafer site
                    RecordReadInfo.iRecordType = 5;
                    RecordReadInfo.iRecordSubType = 10;
                    StdfFile.WriteHeader(&RecordReadInfo);
                    StdfFile.WriteByte(1);								// Test head
                    StdfFile.WriteByte(nSiteId);					// Tester site
                    StdfFile.WriteRecord();


                    // Reset counters
                    iTotalTests = 0;

                    QListIterator<CGAdvantestT2000Parameter*> lstIteratorParameterFlow(m_plstParameterFlow);
                    lstIteratorParameterFlow.toFront();

                    while(lstIteratorParameterFlow.hasNext())
                    {
                        ptTest = lstIteratorParameterFlow.next();

                        // Check if have result for this site
                        if(!ptTest->m_mapPass.contains(nSiteId))
                            continue;

                        iTotalTests ++;

                        if(!m_lstParametersStaticHeaderWritten.contains(ptTest->m_strName))
                            m_lstParametersStaticHeaderWritten.append(ptTest->m_strName);

                        // Check if it is FTR (0 result), PTR (1 result without pin name), MPR (results with pin name)
                        if( ptTest->m_mapValue.count() == 0)
                        {
                            // Write FTR
                            RecordReadInfo.iRecordType = 15;
                            RecordReadInfo.iRecordSubType = 20;

                            StdfFile.WriteHeader(&RecordReadInfo);
                            StdfFile.WriteDword(ptTest->m_nNumber);			// Test Number
                            StdfFile.WriteByte(1);							// Test head
                            StdfFile.WriteByte(nSiteId);					// Tester site:1,2,3,4 or 5, etc.
                            if(ptTest->m_mapPass[nSiteId])
                                bData = 0;		// Test passed
                            else
                                bData = BIT7;	// Test Failed

                            StdfFile.WriteByte(bData);						// TEST_FLG
                            bData = 0;
                            StdfFile.WriteByte(bData);						// OPT_FLG
                            StdfFile.WriteDword(0);		// cycl_cnt
                            StdfFile.WriteDword(0);		// rel_vadr
                            StdfFile.WriteDword(0);		// rept_cnt
                            StdfFile.WriteDword(0);		// num_fail
                            StdfFile.WriteDword(0);		// xfail_ad
                            StdfFile.WriteDword(0);		// yfail_ad
                            StdfFile.WriteWord(0);		// vect_off
                            StdfFile.WriteWord(0);		// rtn_icnt
                            StdfFile.WriteWord(0);
                            StdfFile.WriteWord(0);
                            StdfFile.WriteString("");	// vect_name
                            StdfFile.WriteString("");	// time_set
                            StdfFile.WriteString("");	// op_code
                            StdfFile.WriteString(ptTest->m_strName.toLatin1().constData());	// test_txt: test name
                            StdfFile.WriteString("");	// alarm_id
                            StdfFile.WriteString("");	// prog_txt
                            StdfFile.WriteString("");	// rslt_txt
                            StdfFile.WriteByte(0);		// patg_num
                            StdfFile.WriteString("");	// spin_map
                            StdfFile.WriteRecord();
                        }
                        else
                        if( ptTest->m_mapValue[nSiteId].contains("NO-PIN-DATA"))
                        {
                            // Write PTR
                            RecordReadInfo.iRecordType = 15;
                            RecordReadInfo.iRecordSubType = 10;

                            StdfFile.WriteHeader(&RecordReadInfo);
                            StdfFile.WriteDword(ptTest->m_nNumber);			// Test Number
                            StdfFile.WriteByte(1);							// Test head
                            StdfFile.WriteByte(nSiteId);					// Tester site:1,2,3,4 or 5, etc.
                            if(ptTest->m_mapPass[nSiteId])
                                bData = 0;		// Test passed
                            else
                            {
                                bData = BIT7;	// Test Failed
                                bPassStatus = false;
                            }
                            if(ptTest->m_mapValue[nSiteId]["NO-PIN-DATA"] == C_INFINITE)
                                bData |= BIT1;

                            StdfFile.WriteByte(bData);						// TEST_FLG
                            bData = 0;
                            StdfFile.WriteByte(bData);						// PARAM_FLG
                            StdfFile.WriteFloat(ptTest->m_mapValue[nSiteId]["NO-PIN-DATA"]);							// Test result
                            if(!ptTest->m_bStaticHeaderWritten)
                            {
                                // save Parameter name without unit information
                                StdfFile.WriteString(ptTest->m_strName.toLatin1().constData());	// TEST_TXT
                                StdfFile.WriteString("");					// ALARM_ID

                                bData = BIT1;	// Valid data.
                                if(!ptTest->m_bValidLowLimit)
                                    bData |= BIT6;
                                if(!ptTest->m_bValidHighLimit)
                                    bData |= BIT7;
                                StdfFile.WriteByte(bData);					// OPT_FLAG

                                StdfFile.WriteByte(-ptTest->m_nScale);		// RES_SCALE
                                StdfFile.WriteByte(-ptTest->m_nScale);		// LLM_SCALE
                                StdfFile.WriteByte(-ptTest->m_nScale);		// HLM_SCALE
                                StdfFile.WriteFloat(ptTest->m_fLowLimit);	// LOW Limit
                                StdfFile.WriteFloat(ptTest->m_fHighLimit);	// HIGH Limit
                                StdfFile.WriteString(ptTest->m_strUnits.toLatin1().constData());		// Units
                            }
                            StdfFile.WriteRecord();
                        }
                        else
                        {
                            // only if m_bFlagGenerateMpr
                            // else error
                            if(!m_bFlagGenerateMpr)
                                return false;

                            // Write MPR
                            RecordReadInfo.iRecordType = 15;
                            RecordReadInfo.iRecordSubType = 15;

                            StdfFile.WriteHeader(&RecordReadInfo);
                            StdfFile.WriteDword(ptTest->m_nNumber);		// Test Number
                            StdfFile.WriteByte(1);						// Test head
                            StdfFile.WriteByte(nSiteId);				// Tester site:1,2,3,4 or 5, etc.
                            bData = 0;
                            if(!ptTest->m_mapPass[nSiteId])
                                bData |= BIT7;	// Test Failed

                            StdfFile.WriteByte(bData);					// TEST_FLG
                            bData = BIT6|BIT7;
                            StdfFile.WriteByte(bData);					// PARAM_FLG
                            StdfFile.WriteWord(ptTest->m_mapValue[nSiteId].count());	// RTN_ICNT
                            StdfFile.WriteWord(ptTest->m_mapValue[nSiteId].count());	// RSLT_CNT
                            bData = 0;
                            for (int i = 0; i != (ptTest->m_mapValue[nSiteId].count()+1)/2; i++)
                            {
                                StdfFile.WriteByte(0);					// RTN_STAT
                            }
                            for(itStringFloat=ptTest->m_mapValue[nSiteId].begin(); itStringFloat!=ptTest->m_mapValue[nSiteId].end(); itStringFloat++)
                            {
                                StdfFile.WriteFloat(itStringFloat.value());			// Test result
                            }
                            StdfFile.WriteString(ptTest->m_strName.toLatin1().constData());	// TEST_TXT
                            StdfFile.WriteString("");					// ALARM_ID

                            if(!ptTest->m_bStaticHeaderWritten)
                            {
                                // No Scale result
                                bData = 0;
                                if(ptTest->m_nScale == 0)
                                    bData |= BIT0;
                                // No LowLimit
                                if(!ptTest->m_bValidLowLimit)
                                {
                                    bData |= BIT2;
                                    bData |= BIT6;
                                }
                                // No HighLimit
                                if(!ptTest->m_bValidHighLimit)
                                {
                                    bData |= BIT3;
                                    bData |= BIT7;
                                }
                                StdfFile.WriteByte(bData);					// OPT_FLAG
                                StdfFile.WriteByte(-ptTest->m_nScale);		// RES_SCALE
                                StdfFile.WriteByte(-ptTest->m_nScale);		// LLM_SCALE
                                StdfFile.WriteByte(-ptTest->m_nScale);		// HLM_SCALE
                                StdfFile.WriteFloat(ptTest->m_fLowLimit);	// LOW Limit
                                StdfFile.WriteFloat(ptTest->m_fHighLimit);	// HIGH Limit
                                StdfFile.WriteFloat(0);						// StartIn
                                StdfFile.WriteFloat(0);						// IncrIn
                                for(itStringFloat=ptTest->m_mapValue[nSiteId].begin(); itStringFloat!=ptTest->m_mapValue[nSiteId].end(); itStringFloat++)
                                {
                                    StdfFile.WriteWord(m_qMapPinIndex[itStringFloat.key()]);		// RTN_INDX
                                }
                                StdfFile.WriteString(ptTest->m_strUnits.toLatin1().constData());	// Units
                                StdfFile.WriteString("");				// units_in
                                StdfFile.WriteString("");				// c_resfmt
                                StdfFile.WriteString("");				// c_llmfmt
                                StdfFile.WriteString("");				// c_hlmfmt
                                StdfFile.WriteFloat(0);					// lo_spec
                                StdfFile.WriteFloat(0);					// hi_spec
                            }
                            StdfFile.WriteRecord();
                        }
                    }

                    bPassStatus = m_clPartsInfo.m_bPass[nSiteId];
                    if(bPassStatus)
                        iBin = 1;
                    else
                        iBin = 0;


                    if(!m_mapBinsCount.contains(iBin))
                        m_mapBinsCount[iBin].m_nCount[nSiteId] = 0;
                    if(!m_mapBinsCount[iBin].m_nCount.contains(nSiteId))
                        m_mapBinsCount[iBin].m_nCount[nSiteId] = 0;

                    m_mapBinsCount[iBin].m_nCount[nSiteId]++;
                    iTotalPart++;

                    // Write PRR
                    RecordReadInfo.iRecordType = 5;
                    RecordReadInfo.iRecordSubType = 20;
                    StdfFile.WriteHeader(&RecordReadInfo);
                    StdfFile.WriteByte(1);						// Test head
                    StdfFile.WriteByte(nSiteId);			// Tester site:1,2,3,4 or 5
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
                    StdfFile.WriteWord(m_clPartsInfo.m_nXLoc[nSiteId]);	// X_COORD
                    StdfFile.WriteWord(m_clPartsInfo.m_nYLoc[nSiteId]);	// Y_COORD
                    StdfFile.WriteDword(0);					// No testing time known...
                    StdfFile.WriteString(QString::number(iPartNumber ++).toLatin1().constData());// PART_ID
                    StdfFile.WriteString("");				// PART_TXT
                    StdfFile.WriteString("");				// PART_FIX
                    StdfFile.WriteRecord();
                }
            }
            else
            {
                // start time

                QString strDate = strString.mid(6,4) + "/" + strString.left(5) + "T" + strString.right(8);
                QDateTime clDateTime;
                clDateTime.setTimeSpec(Qt::UTC);
                m_lStartTime = clDateTime.fromString(strDate, Qt::ISODate).toTime_t();
            }

            // Clean all the flow results
            while(!m_plstParameterFlow.isEmpty())
            {
                ptTest = m_plstParameterFlow.takeFirst();

                ptTest->m_mapPass.clear();
                ptTest->m_mapValue.clear();
                delete ptTest;
            }

            lstSites.clear();
            ptTest=NULL;
            strString = "";
            continue;
        }
        else if(strString.startsWith("TEST ID", Qt::CaseInsensitive))
        {
            // PTR OR MPR if PinName
            //Test ID    Test Description                   Index  Result    Value     JudgeHigh JudgeLow  DUT  Pin
            //1290001    IDD, not VDD IFP_PLLVDD                 1 PASS        66.40mA    2.000A  -5.000uA    1 FBVDDQ
            //                                                   1 PASS        67.20mA    2.000A  -5.000uA    2 FBVDDQ
            //1290001    IDD, not VDD IFP_PLLVDD                 1 PASS        41.30mA    1.000A  -5.000mA    1 PEX_IOVDD
            //                                                   1 PASS        42.04mA    1.000A  -5.000mA    2 PEX_IOVDD

            strString = "";


            bIsNumber = false;
            bPassStatus = false;

            ptTest = NULL;

            while(!hAdvantestT2000File.atEnd())
            {
                strString = ReadLine(hAdvantestT2000File);

                if(strString.startsWith("_param_", Qt::CaseInsensitive))
                {
                    nTestNum = -1;
                    strString = "";
                    continue;
                }

                strValue = strString.left(11).simplified();
                if(!strValue.isEmpty())
                    nTestNum = strValue.toInt(&bIsNumber);
                else
                    bIsNumber = true;

                if(!bIsNumber)
                    break;

                strValue = strString.mid(94,4).simplified();
                nSiteId = strValue.toInt();
                if(!lstSites.contains(strValue))
                    lstSites.append(strValue);

                strValue = strString.mid(11,35).simplified();
                if(!strValue.isEmpty())
                    strTestName = strTestDescription = strValue;

                strPinName = strString.mid(98).simplified();
                if(!strPinName.isEmpty() && !m_bFlagGenerateMpr)
                {
                    strTestName = strTestDescription + " - " + strPinName;
                    strPinName = "NO-PIN-DATA";
                }

                if(strPinName.isEmpty())
                    strPinName = "NO-PIN-DATA";

                if(!(ptTest && (ptTest->m_nNumber == nTestNum) && (ptTest->m_strName == strTestName)))
                {
                    ptTest = new CGAdvantestT2000Parameter();
                    ptTest->m_strName = strTestName;
                    ptTest->m_nNumber = nTestNum;
                    ptTest->m_bValidHighLimit = ptTest->m_bValidLowLimit = false;
                    ptTest->m_fHighLimit = ptTest->m_fLowLimit = 0;
                    ptTest->m_nScale = 0;

                    if(!m_bFlagGenerateMpr)
                        ptTest->m_bStaticHeaderWritten = false; // always write test name in PTR record
                    else
                        ptTest->m_bStaticHeaderWritten = m_lstParametersStaticHeaderWritten.contains(ptTest->m_strName);

                    m_plstParameterFlow.append(ptTest);
                }



                // Result
                if(!ptTest->m_mapPass.contains(nSiteId))
                    ptTest->m_mapPass[nSiteId] = true;
                ptTest->m_mapPass[nSiteId] &= (bool)strString.contains("PASS",Qt::CaseInsensitive);

                strValue = strString.mid(64,9).simplified();
                if(strValue.contains("None",Qt::CaseInsensitive))
                {
                    fValue = C_INFINITE;
                }
                else
                {
                    strUnit = strValue;
                    NormalizeValues(strUnit, fValue, nScale, bIsNumber);
                    if(!bIsNumber)
                        fValue = C_INFINITE;
                }

                ptTest->m_mapValue[nSiteId][strPinName] = fValue;

                if(!ptTest->m_bStaticHeaderWritten)
                {
                    ptTest->m_strUnits = strUnit;
                    ptTest->m_nScale = nScale;

                    // Update test information limits
                    // Low limit
                    strUnit = strString.mid(84,9).simplified();
                    if(!ptTest->m_bValidLowLimit)
                        NormalizeValues(strUnit, ptTest->m_fLowLimit, nScale, ptTest->m_bValidLowLimit);

                    // High limit
                    strUnit = strString.mid(74,9).simplified();
                    if(!ptTest->m_bValidHighLimit)
                        NormalizeValues(strUnit, ptTest->m_fHighLimit, nScale, ptTest->m_bValidHighLimit);
                }

            }
            continue;

        }
        else
        {
            strSection = strString.section(":",0,0).simplified().toUpper();
            strString = strString.section(":",1);
        }


        if(strSection == "LOT NUMBER")
        {
            strString = strString.mid(30);
        }
        else if(strSection == "SAMPLE NUMBER")
        {
            iPartNumber = strString.simplified().toInt();
            strString = "";
        }
        else if(strSection == "DEVICE TYPE")
        {// ignore
            strString = strString.mid(30);
        }
        else if(strSection == "SITE CONTROLLER ID")
        {// ignore
            strString = "";
        }
        else if(strSection == "DUTID")
        {
            strValue = strString.simplified();
            nSiteId = strValue.toInt();
            if(!lstSites.contains(strValue))
                lstSites.append(strValue);

            strString = "";
        }
        else if(strSection == "SITECID")
        {// ignore
            strString = "";
        }
        else if(strSection == "AT_PROBER_X_ADDRESS")
        {
            m_clPartsInfo.m_nXLoc[nSiteId] = strString.simplified().toInt();
            strString = "";
        }
        else if(strSection == "AT_PROBER_Y_ADDRESS")
        {
            m_clPartsInfo.m_nYLoc[nSiteId] = strString.simplified().toInt();
            strString = "";
        }
        else if(strSection == "LOT NUMBER")
        {
            strString = strString.mid(30);
        }
        else if(strSection == "SAMPLE NUMBER")
        {
            iPartNumber = strString.simplified().toInt();
            strString = "";
        }
        else if(strSection == "***** ID")
        {
            // CAS 1 (one FTR)
            //***** ID: 10009 [JTAG_vih_vil_Vddmin] PASS *****
            //  Pattern List Name: plist_g98_a02_jtag_vih_vil
            //     ...
            //DUT ID: 1
            //       Result: PASS
            //DUT ID: 2
            //       Result: PASS

            // CAS 2 (one PTR per line)
            //***** ID: 1000 [continuity] FAIL *****
            //     Test Condition: tc_lvl_openshort
            // ...
            //  DUT  Result     Status    Value      Judge Hi   Judge Lo   Pin Name
            //     1 LOWFAIL    Valid       -974.5mV   -40.00mV   -850.0mV FBA_CMD23

            // CAS 3 (statistical results) (one PTR per line)
            //***** ID: 300 [AnPLL19p2Pr1V384Po1D08_usb_gpio30_PD] *****
            //  DUT  Result     Range    Item      Value     Judge Hi  Judge Lo  Pin Name
            //    2  PASS       M50MHz   Tmin      20.54nS   21.39nS   20.20nS   GPIO_30


            bIsNumber = false;
            bPassStatus = false;

            nTestNum = strString.section('[',0,0).simplified().toInt(&bIsNumber);
            if(!bIsNumber)
                return false;

            strTestName = strString.section('[',1).section(']',0,0).simplified();
            strString = strString.section(']',1).simplified();
            bPassStatus = strString.startsWith("PASS", Qt::CaseInsensitive);

            // Test results here
            // goto DUT
            while(!hAdvantestT2000File.atEnd())
            {
                strString = ReadLine(hAdvantestT2000File);
                if(strString.simplified().startsWith("DUT", Qt::CaseInsensitive))
                    break;
                if(strString.startsWith("*****", Qt::CaseInsensitive))
                    break;
            }

            if(strString.startsWith("*****", Qt::CaseInsensitive))
                    continue;

            if(strString.simplified().startsWith("DUT ID", Qt::CaseInsensitive))
            {
                //DUT ID: 1
                //       Result: PASS
                // FTR
                ptTest = new CGAdvantestT2000Parameter();
                ptTest->m_strName = strTestName;
                ptTest->m_nNumber = nTestNum;
                ptTest->m_bStaticHeaderWritten = m_lstParametersStaticHeaderWritten.contains(ptTest->m_strName);
                ptTest->m_bValidHighLimit = ptTest->m_bValidLowLimit = false;
                ptTest->m_fHighLimit = ptTest->m_fLowLimit = 0;
                ptTest->m_nScale = 0;

                m_plstParameterFlow.append(ptTest);

                while(!hAdvantestT2000File.atEnd())
                {
                    strValue = strString.mid(7).simplified();
                    nSiteId = strValue.toInt(&bIsNumber);
                    if(!bIsNumber)
                        return false;
                    if(!lstSites.contains(strValue))
                        lstSites.append(strValue);

                    strString = ReadLine(hAdvantestT2000File).simplified();
                    if(!strString.startsWith("Result:", Qt::CaseInsensitive))
                        return false;

                    ptTest->m_mapPass[nSiteId] = strString.contains("PASS", Qt::CaseInsensitive);

                    strString = ReadLine(hAdvantestT2000File).simplified();
                    if(!strString.startsWith("DUT ID", Qt::CaseInsensitive))
                        break;
                }
            }
            else
            {
                // PTR
                //  DUT  Result     Status    Value      Judge Hi   Judge Lo   Pin Name
                //     1 LOWFAIL    Valid       -974.5mV   -40.00mV   -850.0mV FBA_CMD23
                //  DUT  Result     Range    Item      Value     Judge Hi  Judge Lo  Pin Name
                //    2  PASS       M50MHz   Tmin      20.54nS   21.39nS   20.20nS   GPIO_30

                int iDutPos = strString.indexOf("Dut",0, Qt::CaseInsensitive);
                int iItemPos = strString.indexOf("Item",0, Qt::CaseInsensitive);
                int iPinNamePos = strString.indexOf("Pin Name",0, Qt::CaseInsensitive);
                int iResultPos = strString.indexOf("Result",0, Qt::CaseInsensitive);
                int iValuePos = strString.indexOf("Value",0, Qt::CaseInsensitive);
                int iJudgeHiPos = strString.indexOf("Judge Hi",0, Qt::CaseInsensitive);
                int iJudgeLoPos = strString.indexOf("Judge Lo",0, Qt::CaseInsensitive);

                if((iDutPos == -1) && (iResultPos == -1) && (iValuePos == -1))
                    return false;

                while(!hAdvantestT2000File.atEnd())
                {
                    strString = ReadLine(hAdvantestT2000File);
                    strValue = strString.mid(iDutPos,4).simplified();
                    nSiteId = strValue.toInt(&bIsNumber);
                    if(!bIsNumber)
                        break;
                    if(!lstSites.contains(strValue))
                        lstSites.append(strValue);

                    ptTest = new CGAdvantestT2000Parameter();
                    ptTest->m_strName = strTestName;
                    ptTest->m_nNumber = nTestNum;
                    ptTest->m_bValidHighLimit = ptTest->m_bValidLowLimit = false;
                    ptTest->m_fHighLimit = ptTest->m_fLowLimit = 0;
                    ptTest->m_nScale = 0;

                    m_plstParameterFlow.append(ptTest);

                    if(iItemPos > 0)
                        ptTest->m_strName += " - " + strString.mid(iItemPos,9).simplified();
                    if(iPinNamePos > 0)
                        ptTest->m_strName += " " + strString.mid(iPinNamePos).simplified();

                    ptTest->m_mapPass[nSiteId] = strString.mid(iResultPos,10).contains("PASS", Qt::CaseInsensitive);
                    strValue = strString.mid(iValuePos,10).simplified();
                    if(strValue.contains("None", Qt::CaseInsensitive))
                    {
                        fValue = C_INFINITE;
                    }
                    else
                    {
                        strUnit = strValue;
                        NormalizeValues(strUnit, fValue, nScale, bIsNumber);
                        if(!bIsNumber)
                            fValue = C_INFINITE;
                    }

                    ptTest->m_mapValue[nSiteId]["NO-PIN-DATA"] = fValue;

                    ptTest->m_bStaticHeaderWritten = false; // m_lstParametersStaticHeaderWritten.contains(ptTest->m_strName);
                    if(!ptTest->m_bStaticHeaderWritten)
                    {
                        ptTest->m_nScale = nScale;
                        ptTest->m_strUnits = strUnit;

                        if(iJudgeLoPos > 0)
                        {
                            strUnit = strString.mid(iJudgeLoPos,10).simplified();
                            NormalizeValues(strUnit, ptTest->m_fLowLimit, nScale, ptTest->m_bValidLowLimit);
                        }

                        if(iJudgeHiPos > 0)
                        {
                            strUnit = strString.mid(iJudgeHiPos,10).simplified();
                            NormalizeValues(strUnit, ptTest->m_fHighLimit, nScale, ptTest->m_bValidHighLimit);
                        }

                    }
                }
            }
        }
        else
        {
            // ignore
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
        StdfFile.WriteDword(iTotalPart);			// Parts tested: always 5
        StdfFile.WriteDword(0);						// Parts retested
        StdfFile.WriteDword(0);						// Parts Aborted
        StdfFile.WriteDword(iTotalGoodBin);			// Good Parts
        StdfFile.WriteDword(4294967295UL);			// Functionnal Parts
        StdfFile.WriteString(m_strWaferId.toLatin1().constData());	// WaferID
        StdfFile.WriteRecord();
    }


    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 40;
    QMap<int,CAdvantest_T2000BinInfo>::Iterator itMapBin;
    QMap<int, int>::Iterator itBinCount;
    for ( itMapBin = m_mapBinsCount.begin(); itMapBin != m_mapBinsCount.end(); ++itMapBin )
    {
        // Write HBR for each site
        for ( itBinCount = itMapBin.value().m_nCount.begin(); itBinCount != itMapBin.value().m_nCount.end(); itBinCount++)
        {
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(1);							// Test Head = ALL
            StdfFile.WriteByte(itBinCount.key());			// Test sites = ALL
            StdfFile.WriteWord(itMapBin.key());				// HBIN
            StdfFile.WriteDword(itBinCount.value());			// Total Bins
            if(itMapBin.key() == 1)
                StdfFile.WriteByte('P');
            else
                StdfFile.WriteByte('F');
            StdfFile.WriteString("");
            StdfFile.WriteRecord();
        }
    }

    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 50;

    for ( itMapBin = m_mapBinsCount.begin(); itMapBin != m_mapBinsCount.end(); ++itMapBin )
    {
        // Write SBRfor each site
        for ( itBinCount = itMapBin.value().m_nCount.begin(); itBinCount != itMapBin.value().m_nCount.end(); itBinCount++)
        {

            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(1);						// Test Head
            StdfFile.WriteByte(itBinCount.key());		// Test sites
            StdfFile.WriteWord(itMapBin.key());			// SBIN = 0
            StdfFile.WriteDword(itBinCount.value());	// Total Bins
            if(itMapBin.key() == 1)
                StdfFile.WriteByte('P');
            else
                StdfFile.WriteByte('F');
            StdfFile.WriteString("");
            StdfFile.WriteRecord();
        }
    }

    // Write MRR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteDword(0);			// File finish-time.
    StdfFile.WriteRecord();

    // Close STDF file.
    StdfFile.Close();

    // Success
    return true;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' ADVANTEST_T2000 file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGADVANTEST_T2000toSTDF::Convert(const char *AdvantestT2000FileName, const char *strFileNameSTDF)
{
    // No erro (default)
    m_iLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(AdvantestT2000FileName);
    QFileInfo fOutput(strFileNameSTDF);

    QFile f( strFileNameSTDF );
    if((f.exists() == true) && (fInput.lastModified() < fOutput.lastModified()))
        return true;

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    bool bHideProgressAfter=true;
    bool bHideLabelAfter=false;

    iProgressStep = 0;
    iNextFilePos = 0;

    if(GexScriptStatusLabel != NULL)
    {
        if(GexScriptStatusLabel->isHidden())
        {
            bHideLabelAfter = true;
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+
                                                             QFileInfo(AdvantestT2000FileName).fileName()+
                                                             "...");
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

    if(ReadAdvantestT2000File(AdvantestT2000FileName,strFileNameSTDF) != true)
    {
        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if((GexProgressBar != NULL)
        && bHideProgressAfter)
            GexProgressBar->hide();

        if((GexScriptStatusLabel != NULL)
        && bHideLabelAfter)
            GexScriptStatusLabel->hide();

        return false;	// Error reading ADVANTEST_T2000 file
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
QString CGADVANTEST_T2000toSTDF::ReadLine(QTextStream& hFile)
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
