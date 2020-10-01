//////////////////////////////////////////////////////////////////////
// import_sdi.cpp: Convert a Sdi file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#include <math.h>
#include <qmath.h>
#include <time.h>

#ifdef _WIN32
#include "windows.h"		// For 'GetWindowsDirectoryA' declaration
#endif

#include <qfileinfo.h>
#include <qmap.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>
#include <QDir>

#include "engine.h"
#include "import_sdi.h"
#include "import_constants.h"

// File format:
//miin-how.lee@avagotech.com
//die_test 2CK7_DESC
//circuit_id lot_no wafer_no xy_location bin tester_id fixture_id man_site test_site operator_id test_date test_time retest deg_c tester_os_rev str ac_cal iddq_min iddq_max
//free_format
//2CK7T1B GBG463 GBG463-08 13,1 AA T9313 00 TSMC UNK 100976 20110714 013016 0 -9999.0 5.2.2 "UNKNOWN:AA:Passed" 272 3.214474 3.364459
//

// File format:
//LOT_NO DP_DESC_ID CIRCUIT_ID WAFER_NO XY_LOCATION RETEST BIN TEST_DATE TEST_TIME TESTER_ID OPERATOR_ID DEG_C TEST_REV TESTER_OS_REV FIXTURE_ID TEST_NAME TEST_SITE MAN_SITE WAFER_TM RETICLE_X RETICLE_Y Q2_LPM Q2_HPM Q1_LPM Q1_HPM VBLIN_LPM Q1_LEAKAGE Q2_LEAKAGE TERMCODE I_TOT_HPM I_TOT_LPM Q1_LKG Q2_LKG STR  CHIP_ID
//NTHY1A7904 2HY1_DC_MMIC HY1A D5972-031FEC3 100,100 2 DF 20120102 235529 APT09 prod 30  NA NA 2 NA FTC FTC +1.455800000000e+02 +6.000000000000e+00 +5.000000000000e+00 +2.704000000000e-02 +7.940000000000e-02 +4.450000000000e-04 +1.794000000000e-02 NA NA NA +9.000000000000e+02 +9.734000000000e-02 +2.748500000000e-02 +3.084500000000e-06 +1.315500000000e-06 "Q1_LPM" NA
//NTHY1A7904 2HY1_DC_MMIC HY1A D5972-031FEC3 100,101 2 DD 20120102 235529 APT09 prod 30  NA NA 2 NA FTC FTC +1.455800000000e+02 +6.000000000000e+00 +6.000000000000e+00 +3.537500000000e-06 +1.623500000000e-05 +1.206500000000e-02 +1.000050000000e-01 NA NA NA +9.000000000000e+02 +1.000212000000e-01 +1.206854000000e-02 +7.310000000000e-06 +2.263500000000e-06 "Q2_LPM" NA
//NTHY1A7904 2HY1_DC_MMIC HY1A D5972-031FEC3 100,102 2 DD 20120102 235529 APT09 prod 30  NA NA 2 NA FTC FTC +1.455800000000e+02 +6.000000000000e+00 +6.000000000000e+00 +4.515000000000e-06 +1.627000000000e-05 +1.226000000000e-02 +1.000050000000e-01 NA NA NA +9.000000000000e+02 +1.000213000000e-01 +1.226452000000e-02 +3.270000000000e-06 +1.435500000000e-06 "Q2_LPM" NA




// main.cpp
extern QLabel           *GexScriptStatusLabel;  // Handle to script status text in status bar
extern QProgressBar	*   GexProgressBar;         // Handle to progress bar in status bar

#define TEST_PASSFLAG_UNKNOWN 0
#define TEST_PASSFLAG_PASS 1
#define TEST_PASSFLAG_FAIL 2


#define BIT0            0x01
#define BIT1            0x02
#define BIT2            0x04
#define BIT3            0x08
#define BIT4            0x10
#define BIT5            0x20
#define BIT6            0x40
#define BIT7            0x80

CGSdiBin::CGSdiBin(QString strValue)
{
    bool bIsNumber;
    m_nNumber = strValue.toInt(&bIsNumber);
    m_bPassFlag = false;
    m_strName = strValue;
    m_nCount = 0;

    if(!bIsNumber)
    {
        // Convert to number
        int nIndex;
        QString strNumber;
        for(nIndex=0; nIndex<strValue.length();nIndex++)
        {
            strNumber += QString::number((int) QByteArray(QString(
                       strValue.mid(nIndex,1).toUpper()).toLatin1().constData(),1).at(0));
        }
        if(strNumber.startsWith("0"))
            strNumber = strNumber.right(strNumber.length()-1);

        m_nNumber = strNumber.toInt();

        if(strValue.toUpper() == "AA")
            m_nNumber = 1;
    }

    if(m_nNumber == 1)
        m_bPassFlag = true;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGSditoSTDF::CGSditoSTDF()
{
    // Default: Sdi parameter list on disk includes all known Sdi parameters...
    m_bNewSdiParameterFound = false;
    m_lStartTime = 0;
    m_lStopTime = 0;
    m_strRtstCod = " ";

    m_nPosDate = -1;
    m_nPosTime = -1;
    m_nPosWaferId = -1;
    m_nPosPartId = -1;
    m_nPosBin = -1;
    m_nPosBinDesc = -1;
    m_nPosXY = -1;
    m_nNbItems = -1;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGSditoSTDF::~CGSditoSTDF()
{
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGSditoSTDF::GetLastError()
{
    QString strLastError = "Import SDI data : ";

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
// Load Sdi Parameter table from DISK
//////////////////////////////////////////////////////////////////////
void CGSditoSTDF::LoadParameterIndexTable(void)
{
    QString	strSdiTableFile;
    QString	strString;

    strSdiTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
    strSdiTableFile += GEX_SDI_PARAMETERS;

    // Open Sdi Parameter table file
    QFile f( strSdiTableFile );
    if(!f.open( QIODevice::ReadOnly ))
        return;

    // Assign file I/O stream
    QTextStream hSdiTableFile(&f);

    // Skip comment or empty lines
    do
    {
        strString = hSdiTableFile.readLine();
    }
    while((strString.indexOf("----------------------") < 0) && (!hSdiTableFile.atEnd()));

    // Read lines
    m_pFullSdiParametersList.clear();
    strString = hSdiTableFile.readLine();
    while (strString.isNull() == false)
    {
        // Save Parameter name in list
        m_pFullSdiParametersList.append(strString);
        // Read next line
        strString = hSdiTableFile.readLine();
    };

    // Close file
    f.close();
}

//////////////////////////////////////////////////////////////////////
// Save Sdi Parameter table to DISK
//////////////////////////////////////////////////////////////////////
void CGSditoSTDF::DumpParameterIndexTable(void)
{
    QString		strSdiTableFile;

    strSdiTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
    strSdiTableFile += GEX_SDI_PARAMETERS;

    // Open Sdi Parameter table file
    QFile f( strSdiTableFile );
    if(!f.open( QIODevice::WriteOnly ))
        return;

    // Assign file I/O stream
    QTextStream hSdiTableFile(&f);

    // First few lines are comments:
    hSdiTableFile << "############################################################" << endl;
    hSdiTableFile << "# DO NOT EDIT THIS FILE!" << endl;
    hSdiTableFile << "# Quantix Examinator: SDI Parameters detected" << endl;
    hSdiTableFile << "# www.mentor.com" << endl;
    hSdiTableFile << "# Quantix Examinator reads and writes into this file..." << endl;
    hSdiTableFile << "-----------------------------------------------------------" << endl;

    // Write lines
    // m_pFullSdiParametersList.sort();
    for(int nIndex = 0; nIndex < m_pFullSdiParametersList.count(); nIndex++)
    {
        // Write line
        hSdiTableFile << m_pFullSdiParametersList[nIndex] << endl;
    };

    // Close file
    f.close();
}

//////////////////////////////////////////////////////////////////////
// If Examinator doesn't have this Sdi parameter in his dictionnary, have it added.
//////////////////////////////////////////////////////////////////////
int CGSditoSTDF::UpdateParameterIndexTable(QString strParamName)
{
    int iTestNumber;

    // Check if the table is empty...if so, load it from disk first!
    if(m_pFullSdiParametersList.isEmpty() == true)
    {
        // Load Sdi parameter table from disk...
        LoadParameterIndexTable();
    }

    // Check if Parameter name already in table...if not, add it to the list
    // the new full list will be dumped to the disk at the end.

    iTestNumber = m_pFullSdiParametersList.indexOf(strParamName);
    if(iTestNumber < 0)
    {
        // Update list
        m_pFullSdiParametersList.append(strParamName);
        iTestNumber = m_pFullSdiParametersList.indexOf(strParamName);

        // Set flag to force the current Sdi table to be updated on disk
        m_bNewSdiParameterFound = true;
    }

    return iTestNumber;
}



//////////////////////////////////////////////////////////////////////
// Check if File is compatible with Sdi format
//////////////////////////////////////////////////////////////////////
bool CGSditoSTDF::IsCompatible(const char *szFileName)
{
    QString strString;

    // Open hCsmFile file
    QFile f( szFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening ASL1000 file
        return false;
    }
    QTextStream hSdiFile(&f);
    bool bCompatible = false;
    int nLine;
    nLine = 0;

    // Check if 4 first lines are the correct Sdi header...
    //miin-how.lee@avagotech.com
    //die_test 2CK7_DESC
    //circuit_id lot_no wafer_no xy_location bin tester_id fixture_id man_site test_site operator_id test_date test_time retest deg_c tester_os_rev str ac_cal iddq_min iddq_max
    //free_format

    while(!hSdiFile.atEnd())
    {
        strString = hSdiFile.readLine().toLower();
        if(strString.startsWith("free_format"))
            break;
        if(strString.contains("circuit_id") && strString.contains("lot_no") && strString.contains("wafer_no") && strString.contains("xy_location") && strString.contains("fixture_id") && strString.contains("man_site"))
            bCompatible = true;
        nLine++;
        if(nLine>8)
            break;
    }

    f.close();
    return bCompatible;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the Sdi file
//////////////////////////////////////////////////////////////////////
bool CGSditoSTDF::ReadSdiFile(const char *SdiFileName)
{
    QString strLine;
    QString strSection;
    QString strValue;

    // Open Sdi file
    QFile f( SdiFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening Sdi file
        m_iLastError = errOpenFail;

        // Convertion failed.
        return false;
    }

    QFileInfo clFile(SdiFileName);
    m_strDataFilePath = clFile.absolutePath();

    // Assign file I/O stream
    QTextStream hSdiFile(&f);

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    iFileSize = f.size() + 1;

    bool bCheckFormat = false;
    // Check if first line is the correct Sdi header...
    strLine = ReadLine(hSdiFile).simplified();
    // Can directly start with data
    if(!((strLine.toLower().contains("circuit_id") && strLine.toLower().contains("man_site"))))
    {
        bCheckFormat = true;
        //miin-how.lee@avagotech.com
        m_strSupervisorName = strLine;
        //die_test 2CK7_DESC
        strLine = ReadLine(hSdiFile).simplified();
        m_strTestCod = strLine.section(" ",0,0);
        m_strJobName = strLine.section(" ",1);
        strLine = ReadLine(hSdiFile).simplified();
    }
    //circuit_id lot_no wafer_no xy_location bin tester_id fixture_id man_site test_site operator_id test_date test_time retest deg_c tester_os_rev str ac_cal iddq_min iddq_max
    if(!((strLine.toLower().contains("circuit_id") && strLine.toLower().contains("man_site"))))
    {
        // Incorrect header...this is not a valid SDI file!
        m_iLastError = errInvalidFormat;
        m_strLastError = "One or more Mandatory keywords (circuit_id,man_site,...) are missing.\n";

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    // Good header

    //circuit_id lot_no wafer_no xy_location bin tester_id fixture_id man_site test_site operator_id test_date test_time retest deg_c tester_os_rev str power_clk iddq_min iddq_max
    //free_format
    //2EW9T1A N3X609 N3X609-06 8,1 AB TZH01 00 TSMC UNK 113196 20110707 013703 0 -9999.0 5.2.2 "UNKNOWN:AB:Passed" -999.9 150.742477 197.737839

    QStringList lstSections;
    QDate   clDate;
    QTime   clTime;
    int     nIndex;
    int     nQuoteOffset;

    // Read Sdi information
    lstSections = strLine.split(" ", QString::SkipEmptyParts);
    if(bCheckFormat)
    {
        //free_format
        strLine = ReadLine(hSdiFile).simplified();
        if(!strLine.contains("free_format", Qt::CaseInsensitive))
        {
            // Incorrect header...this is not a valid SDI file!
            m_iLastError = errInvalidFormat;
            m_strLastError = "Mandatory keyword 'free_format' is missing or not at the good line.\n";

            // Convertion failed.
            // Close file
            f.close();
            return false;
        }
    }

    // Read Values
    strLine = ReadLine(hSdiFile).simplified();

    //Extract global info
    nQuoteOffset = 0;
    m_nNbItems = lstSections.count();

    for(nIndex=0;nIndex<lstSections.count();nIndex++)
    {
        strSection = lstSections.at(nIndex).toUpper();
        strValue = strLine.section(" ",nIndex,nIndex+nQuoteOffset);
        if(strValue.toUpper() == "NA")
            strValue = "";

        // Extract value
        if(strSection == "LOT_NO")
            m_strLotId = strValue;
        else if(strSection == "WAFER_NO")
            m_nPosWaferId = nIndex;
        else if(strSection == "CIRCUIT_ID")
            m_strFamilyId = strValue.left(4);
        else if(strSection == "DP_DESC_ID")
            m_strProductId = strValue.left(4);
        else if(strSection == "TEST_DATE")
        {// 20110707 013703
            clDate = QDate(strValue.left(4).toInt(),strValue.mid(4,2).toInt(),strValue.right(2).toInt());
            m_nPosDate = nIndex;
        }
        else if(strSection == "TEST_TIME")
        {// 20110707 013703
            clTime = QTime(strValue.left(2).toInt(),strValue.mid(2,2).toInt(),strValue.right(2).toInt());
            m_nPosTime = nIndex;
        }
        else if(strSection == "OPERATOR_ID")
            m_strOperator = strValue;
        else if(strSection == "MAN_SITE")
            m_strContractorId = strValue;
        else if(strSection == "TEST_SITE")
            m_strFacilId = strValue;
        else if(strSection == "TESTER_ID")
            m_strTesterName = strValue;
        else if(strSection == "TESTER_OS_REV")
            m_strExecVer = strValue;
        else if(strSection == "DEG_C")
            m_strTemp = strValue;
        else if(strSection == "BIN")
            m_nPosBin = nIndex;
        else if(strSection == "CHIP_ID")
            m_nPosPartId = nIndex;
        else if(strSection == "STR")
            m_nPosBinDesc = nIndex;
        else if(strSection == "XY_LOCATION")
            m_nPosXY = nIndex;
        else if(strSection == "RETEST")
            m_strRtstCod = strValue;
        else if(strSection == "TEST_REV")
            m_strJobRev = strValue;
        else if(strSection == "FIXTURE_ID")
            m_strDibId = strValue;
        else if((strSection == "TEST_SITE")
                || (strSection == "MAN_SITE")
                || (strSection == "WAFER_TM")
                || (strSection == "RETICLE_X")
                || (strSection == "RETICLE_Y"))
        {
            // ignore
        }
        else
        {
            m_mapIndexParameters[nIndex]=strSection.toUpper();
        }

    }

    if(m_strProductId.isEmpty())
        m_strProductId = m_strFamilyId;

    if((m_nPosBin <= 0) || (m_nPosXY <= 0) || (m_nPosWaferId <= 0))
    {
        // Incorrect header...this is not a valid SDI file!
        m_iLastError = errInvalidFormat;
        m_strLastError = "Mandatory keyword '";
        if(m_nPosBin <= 0)
            m_strLastError+= "bin";
        else if(m_nPosWaferId <= 0)
            m_strLastError+= "wafer_no";
        else
            m_strLastError+= "xy_location";
        m_strLastError+="' is missing.\n";

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    if(clDate.isValid())
    {
        QDateTime clDateTime(clDate,clTime);
        clDateTime.setTimeSpec(Qt::UTC);
        m_lStartTime = clDateTime.toTime_t();
    }

    // Count the number of parameters specified in the line
    m_iTotalParameters=m_mapIndexParameters.count();
    // If no parameter specified...ignore!
    if((m_iTotalParameters <= 0) )
    {
        // Incorrect header...this is not a valid SDI file!
        m_iLastError = errInvalidFormat;
        m_strLastError = "No parameter found.\n";

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    // Restart to the beginning
    hSdiFile.seek(0);

    // Write STDF file
    if(!WriteStdfFile(&hSdiFile))
    {
        // Close file
        f.close();
        return false;
    }

    // Close file
    f.close();

    // All Sdi file read...check if need to update the Sdi Parameter list on disk?
    if(m_bNewSdiParameterFound == true)
        DumpParameterIndexTable();

    // Success parsing Sdi file
    return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from Sdi data parsed
//////////////////////////////////////////////////////////////////////
bool CGSditoSTDF::WriteStdfFile(QTextStream *hSdiFile)
{
    // now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;

    // Write Test results for each line read.
    int         nSoftBin,nHardBin;
    int         nXPos,nYPos;
    int         iGoodParts;
    int         iTotalGoodBin,iTotalFailBin;
    int         iTotalTests,iPartNumber;
    bool        bPassStatus;
    BYTE        bData;

    QString     strTestName;
    int         nTestNum;

    float       fValue;
    bool        bValue;

    QMap<QString,CGSdiBin*> mapBinInfo;

    QStringList lstValues;
    QDate       clDate;
    QTime       clTime;
    QDateTime clDateTime(clDate,clTime);
    clDateTime.setTimeSpec(Qt::UTC);

    // Read the first line result to have Lot and Product information
    QString     strValue;
    QString     strString;
    int         iLine;
    iLine = 0;
    // Goto result
    while(!hSdiFile->atEnd())
    {
        strString = ReadLine(*hSdiFile); iLine++;
        if((strString.toLower().contains("circuit_id") && strString.toLower().contains("man_site")))
            break;
    }

    // Reset counters
    iGoodParts=iTotalGoodBin=iTotalFailBin=0;
    iPartNumber=0;

    //circuit_id lot_no wafer_no xy_location bin tester_id fixture_id man_site test_site operator_id test_date test_time retest deg_c tester_os_rev str power_clk iddq_min iddq_max
    //free_format
    //2EW9T1A N3X609 N3X609-06 8,1 AB TZH01 00 TSMC UNK 113196 20110707 013703 0 -9999.0 5.2.2 "UNKNOWN:AB:Passed" -999.9 150.742477 197.737839
    //2EW9T1A N3X609 N3X609-06 9,1 AB TZH01 00 TSMC UNK 113196 20110707 013712 0 -9999.0 5.2.2 "UNKNOWN:AB:Passed" -999.9 158.016907 204.668884

    // Write all Parameters read on this file : PIR,PTR...,PRR
    QMap<int,QString>::iterator itParameters;
    // Read Sdi result
    m_strWaferId = "";
    while(!hSdiFile->atEnd())
    {
        strString = ReadLine(*hSdiFile).simplified(); iLine++;
        if(strString.isEmpty())
            continue;

        if(strString.startsWith("free_format",Qt::CaseInsensitive))
            continue;

        if(strString.startsWith("END",Qt::CaseInsensitive))
            break;

        lstValues = strString.split(" ", QString::SkipEmptyParts);
        if(lstValues.count() < m_nNbItems)
        {
            // Incorrect header...this is not a valid SDI file!
            m_iLastError = errInvalidFormat;
            m_strLastError = "Missing some values at line "+QString::number(iLine) + ".\n";

            // Close STDF file.
            StdfFile.Close();
            return false;
        }

        if(m_nPosDate > 0)
        {
            strValue = lstValues.at(m_nPosDate);
            clDate = QDate(strValue.left(4).toInt(),strValue.mid(4,2).toInt(),strValue.right(2).toInt());
            if(m_nPosTime > 0)
            {
                strValue = lstValues.at(m_nPosTime);
                clTime = QTime(strValue.left(2).toInt(),strValue.mid(2,2).toInt(),strValue.right(2).toInt());
            }
            if(clDate.isValid())
            {
                clDateTime.setDate(clDate);
                clDateTime.setTime(clTime);
                m_lStopTime = clDateTime.toTime_t();
            }
        }

        // Wafer id
        if(m_strWaferId != lstValues.at(m_nPosWaferId))
        {
            // Close Stdf file if open
            if(!m_strWaferId.isEmpty())
            {
                strValue = m_strWaferId;
                if(m_strWaferId.section("-",1).length() == 2)
                    strValue = m_strWaferId.right(2);
                // Write WRR for last wafer inserted
                RecordReadInfo.iRecordType = 2;
                RecordReadInfo.iRecordSubType = 20;
                StdfFile.WriteHeader(&RecordReadInfo);
                StdfFile.WriteByte(1);                      // Test head
                StdfFile.WriteByte(255);                    // Tester site (all)
                StdfFile.WriteDword(m_lStartTime);           // Time of last part tested
                StdfFile.WriteDword(iTotalGoodBin+iTotalFailBin);   // Parts tested
                StdfFile.WriteDword(0);                     // Parts retested
                StdfFile.WriteDword(0);                     // Parts Aborted
                StdfFile.WriteDword(iTotalGoodBin);         // Good Parts
                StdfFile.WriteDword((DWORD)-1);             // Functionnal Parts
                StdfFile.WriteString(strValue.toLatin1().constData());  // WaferID
                StdfFile.WriteString("");                   // FabId
                StdfFile.WriteString("");                   // FrameId
                StdfFile.WriteString("");                   // MaskId
                StdfFile.WriteString("");                   // UserDesc
                StdfFile.WriteRecord();

                RecordReadInfo.iRecordType = 1;

                RecordReadInfo.iRecordSubType = 40;
                QMap<QString,CGSdiBin*>::Iterator itMapBin;
                for ( itMapBin = mapBinInfo.begin(); itMapBin != mapBinInfo.end(); ++itMapBin )
                {
                    // Write HBR
                    StdfFile.WriteHeader(&RecordReadInfo);
                    StdfFile.WriteByte(255);                        // Test Head = ALL
                    StdfFile.WriteByte(1);                          // Test sites = ALL
                    StdfFile.WriteWord(itMapBin.value()->m_nNumber); // HBIN = 0
                    StdfFile.WriteDword(itMapBin.value()->m_nCount); // Total Bins
                    if(itMapBin.value()->m_bPassFlag)
                        StdfFile.WriteByte('P');
                    else
                        StdfFile.WriteByte('F');
                    StdfFile.WriteString(QString(itMapBin.value()->m_strName).toLatin1().constData());
                    StdfFile.WriteRecord();
                }

                RecordReadInfo.iRecordType = 1;
                RecordReadInfo.iRecordSubType = 50;

                for ( itMapBin = mapBinInfo.begin(); itMapBin != mapBinInfo.end(); ++itMapBin )
                {
                    // Write SBR
                    StdfFile.WriteHeader(&RecordReadInfo);
                    StdfFile.WriteByte(255);                        // Test Head = ALL
                    StdfFile.WriteByte(1);                          // Test sites = ALL
                    StdfFile.WriteWord(itMapBin.value()->m_nNumber); // HBIN = 0
                    StdfFile.WriteDword(itMapBin.value()->m_nCount); // Total Bins
                    if(itMapBin.value()->m_bPassFlag)
                        StdfFile.WriteByte('P');
                    else
                        StdfFile.WriteByte('F');
                    StdfFile.WriteString(itMapBin.value()->m_strName.toLatin1().constData());
                    StdfFile.WriteRecord();
                }

                RecordReadInfo.iRecordType = 1;
                RecordReadInfo.iRecordSubType = 30;
                StdfFile.WriteHeader(&RecordReadInfo);
                StdfFile.WriteByte(255);                    // Test Head = ALL
                StdfFile.WriteByte(255);                    // Test sites = ALL
                StdfFile.WriteDword(iTotalGoodBin+iTotalFailBin);// Total Parts tested
                StdfFile.WriteDword(0);                     // Total Parts re-tested
                StdfFile.WriteDword(0);                     // Total Parts aborted
                StdfFile.WriteDword(iTotalGoodBin);         // Total GOOD Parts
                StdfFile.WriteRecord();

                // Write MRR
                RecordReadInfo.iRecordType = 1;
                RecordReadInfo.iRecordSubType = 20;
                StdfFile.WriteHeader(&RecordReadInfo);
                StdfFile.WriteDword(m_lStartTime);          // File finish-time.
                StdfFile.WriteRecord();

                // Close STDF file.
                StdfFile.Close();

                iGoodParts=iTotalGoodBin=iTotalFailBin=0;
                iPartNumber=0;

                for ( itMapBin = mapBinInfo.begin(); itMapBin != mapBinInfo.end(); ++itMapBin )
                    delete itMapBin.value();
                mapBinInfo.clear();
            }

            if(m_lStartTime < m_lStopTime)
                m_lStartTime = m_lStopTime;
            m_strWaferId = lstValues.at(m_nPosWaferId);
            // Open a new Stdf file
            // now generate the STDF file...
            QFileInfo clFile(m_strStdfFileName);
            // Generate a specific file name
            // CSV: lot.splitlot_<restofilename>.csv
            // STDF: lot.splitlot_W#_<restoffilename>.csv.gextb.std
            QString strStdfFileName = m_strDataFilePath+QDir::separator();
            strStdfFileName+=clFile.baseName()+"_"+m_strWaferId;
            strStdfFileName += "."+clFile.completeSuffix();

            if(StdfFile.Open((char*)strStdfFileName.toLatin1().constData(),STDF_WRITE) != GS::StdLib::Stdf::NoError)
            {
                // Failed importing SpektraLotSummary file into STDF database
                m_iLastError = errWriteSTDF;

                // Convertion failed.
                return false;
            }

            m_lstStdfFileName += strStdfFileName;

            // Write FAR
            RecordReadInfo.iRecordType = 0;
            RecordReadInfo.iRecordSubType = 10;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(1);                  // SUN CPU type
            StdfFile.WriteByte(4);                  // STDF V4
            StdfFile.WriteRecord();

            // Write MIR
            RecordReadInfo.iRecordType = 1;
            RecordReadInfo.iRecordSubType = 10;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteDword(m_lStartTime);          // Setup time
            StdfFile.WriteDword(m_lStartTime);          // Start time
            StdfFile.WriteByte(1);                      // Station
            StdfFile.WriteByte((BYTE) 'P');             // Test Mode = PRODUCTION
            StdfFile.WriteByte((BYTE) m_strRtstCod[0].toLatin1()); // rtst_cod
            StdfFile.WriteByte((BYTE) ' ');             // prot_cod
            StdfFile.WriteWord(65535);                  // burn_tim
            StdfFile.WriteByte((BYTE) ' ');             // cmod_cod
            StdfFile.WriteString(m_strLotId.toLatin1().constData());        // Lot ID
            StdfFile.WriteString(m_strProductId.toLatin1().constData());    // Part Type / Product ID
            StdfFile.WriteString(m_strTesterName.toLatin1().constData());   // Node name
            StdfFile.WriteString("");                   // Tester Type
            StdfFile.WriteString(m_strJobName.toLatin1().constData());      // Job name
            StdfFile.WriteString(m_strJobRev.toLatin1().constData());       // Job rev
            StdfFile.WriteString("");                   // sublot-id
            StdfFile.WriteString(m_strOperator.toLatin1().constData());     // operator
            StdfFile.WriteString("");                   // exec-type
            StdfFile.WriteString(m_strExecVer.toLatin1().constData());      // exe-ver
            StdfFile.WriteString(m_strTestCod.toLatin1().constData());      // test-cod
            if(m_strTemp == "-9999.0")
                StdfFile.WriteString("");               // test-temperature
            else
                StdfFile.WriteString(m_strTemp.toLatin1().constData());     // test-temperature
            // Construct custom Galaxy USER_TXT
            QString	strUserTxt;
            strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
            strUserTxt += ":";
            strUserTxt += GEX_IMPORT_DATAORIGIN_ETEST;
            strUserTxt += ":SDI";
            StdfFile.WriteString((char *)strUserTxt.toLatin1().constData());// user-txt
            StdfFile.WriteString("");                   // aux-file
            StdfFile.WriteString("");                   // package-type
            StdfFile.WriteString(m_strFamilyId.toLatin1().constData());     // familyID
            StdfFile.WriteString("");                   // Date-code
            StdfFile.WriteString(m_strFacilId.toLatin1().constData());      // Facility-ID
            StdfFile.WriteString("");                   // FloorID
            StdfFile.WriteString("");                   // ProcessID
            StdfFile.WriteString("");                   //OPER_FRQ	Operation frequency or step
            StdfFile.WriteString("");                   //SPEC_NAM	Test specification name
            StdfFile.WriteString("");                   //SPEC_VER	Test specification version number
            StdfFile.WriteString("");                   //FLOW_ID	Test flow ID
            StdfFile.WriteString("");                   //SETUP_ID	Test setup ID
            StdfFile.WriteString("");                   //DSGN_REV	Device design revision
            StdfFile.WriteString("");                   //ENG_ID	Engineering lot ID
            StdfFile.WriteString("");                   //ROM_COD	ROM code ID
            StdfFile.WriteString("");                   //SERL_NUM	Tester serial number
            StdfFile.WriteString(m_strSupervisorName.toLatin1().constData());//SUPR_NAM	Supervisor name or ID
            StdfFile.WriteRecord();

            // Write SDR for last wafer inserted
            RecordReadInfo.iRecordType = 1;
            RecordReadInfo.iRecordSubType = 80;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(255);                // Test Head = ALL
            StdfFile.WriteByte((BYTE)1);            // head#
            StdfFile.WriteByte((BYTE)1);            // Group#
            StdfFile.WriteByte((BYTE)0);            // site_count
            //StdfFile.WriteByte((BYTE)1);          // array of test site#
            StdfFile.WriteString("");               // HAND_TYP: Handler/prober type
            StdfFile.WriteString("");               // HAND_ID: Handler/prober name
            StdfFile.WriteString("");               // CARD_TYP: Probe card type
            StdfFile.WriteString("");               // CARD_ID: Probe card name
            StdfFile.WriteString("");               //LOAD_TYP : Load board type
            StdfFile.WriteString("");               //LOAD_ID : Load board ID
            StdfFile.WriteString("");               //DIB_TYP : DIB board type
            StdfFile.WriteString(m_strDibId.toLatin1().constData());        //DIB_ID : DIB board ID
            StdfFile.WriteString("");               //CABL_TYP : Interface cable type
            StdfFile.WriteString("");               //CABL_ID : Interface cable ID
            StdfFile.WriteString("");               //CONT_TYP : Handler contactor type
            StdfFile.WriteString(m_strContractorId.toLatin1().constData()); //CONT_ID : Handler contactor ID
            StdfFile.WriteString("");               //LASR_TYP : Laser type
            StdfFile.WriteString("");               //LASR_ID : Laser ID
            StdfFile.WriteString("");               //EXTR_TYP : Extra equipment type field
            StdfFile.WriteString("");               //EXTR_ID : Extra equipment ID
            StdfFile.WriteRecord();

            strValue = m_strWaferId;
            if(m_strWaferId.section("-",1).length() == 2)
                strValue = m_strWaferId.right(2);

            // Write WIR of new Wafer.
            RecordReadInfo.iRecordType = 2;
            RecordReadInfo.iRecordSubType = 10;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(1);                  // Test head
            StdfFile.WriteByte(255);                // Tester site (all)
            StdfFile.WriteDword(m_lStartTime);      // Start time
            StdfFile.WriteString(strValue.toLatin1().constData());  // WaferID
            StdfFile.WriteRecord();
        }

        iPartNumber++;

        // Write PIR
        // Write PIR for parts in this Wafer site
        RecordReadInfo.iRecordType = 5;
        RecordReadInfo.iRecordSubType = 10;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);                  // Test head
        StdfFile.WriteByte(1);                  // Tester site
        StdfFile.WriteRecord();

        // Reset Pass/Fail flag.
        bPassStatus = true;

        // Bin name
        strValue = lstValues.at(m_nPosBin);
        if(!mapBinInfo.contains(strValue))
            mapBinInfo[strValue] = new CGSdiBin(strValue);

        nHardBin = nSoftBin = mapBinInfo[strValue]->m_nNumber;
        mapBinInfo[strValue]->m_nCount++;

        strValue = lstValues.at(m_nPosXY);
        nXPos = strValue.section(",",0,0).toInt();
        nYPos = strValue.section(",",1,1).toInt();

        // Reset counters
        iTotalTests = 0;

        for(itParameters=m_mapIndexParameters.begin(); itParameters!=m_mapIndexParameters.end();itParameters++)
        {
            fValue = lstValues.at(itParameters.key()).toFloat(&bValue);

            // Check if have valid value
            if(!bValue)
                continue;

            strTestName = itParameters.value();
            nTestNum = UpdateParameterIndexTable(strTestName);

            iTotalTests++;

            // Write PTR
            RecordReadInfo.iRecordType = 15;
            RecordReadInfo.iRecordSubType = 10;

            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteDword(nTestNum + GEX_TESTNBR_OFFSET_SDI); // Test Number
            StdfFile.WriteByte(1);                                  // Test head
            StdfFile.WriteByte(1);                                  // Tester site:1,2,3,4 or 5, etc.
            bData = BIT6;                                           // Test without P/F indication
            StdfFile.WriteByte(bData);                              // TEST_FLG
            bData = BIT6|BIT7;
            StdfFile.WriteByte(bData);                              // PARAM_FLG
            StdfFile.WriteFloat(fValue);                            // Test result
            StdfFile.WriteString(strTestName.toLatin1().constData());// TEST_TXT
            StdfFile.WriteRecord();
        }

        if(m_nPosPartId > 0)
        {
            lstValues.at(m_nPosPartId).toInt(&bValue);
            if(bValue)
                iPartNumber = lstValues.at(m_nPosPartId).toInt();
        }
        // Write PRR
        RecordReadInfo.iRecordType = 5;
        RecordReadInfo.iRecordSubType = 20;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);                  // Test head
        StdfFile.WriteByte(1);                  // Tester site:1,2,3,4 or 5
        if(bPassStatus == true)
        {
            StdfFile.WriteByte(0);              // PART_FLG : PASSED
            iTotalGoodBin++;
            iGoodParts++;
        }
        else
        {
            StdfFile.WriteByte(8);              // PART_FLG : FAILED
            iTotalFailBin++;
        }
        StdfFile.WriteWord((WORD)iTotalTests);  // NUM_TEST
        StdfFile.WriteWord(nHardBin);           // HARD_BIN
        StdfFile.WriteWord(nSoftBin);           // SOFT_BIN
        StdfFile.WriteWord(nXPos);              // X_COORD
        StdfFile.WriteWord(nYPos);              // Y_COORD
        if(m_lStartTime < m_lStopTime)
            StdfFile.WriteDword(m_lStopTime-m_lStartTime);
        else
            StdfFile.WriteDword(0);             // No testing time known...
        StdfFile.WriteString(QString::number(iPartNumber).toLatin1().constData());// PART_ID
        StdfFile.WriteString("");               // PART_TXT
        StdfFile.WriteString("");               // PART_FIX
        StdfFile.WriteRecord();

        if(m_lStartTime < m_lStopTime)
            m_lStartTime = m_lStopTime;
    }

    strValue = m_strWaferId;
    if(m_strWaferId.section("-",1).length() == 2)
        strValue = m_strWaferId.right(2);

    // Write WRR for last wafer inserted
    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);                      // Test head
    StdfFile.WriteByte(255);                    // Tester site (all)
    StdfFile.WriteDword(m_lStartTime);           // Time of last part tested
    StdfFile.WriteDword(iTotalGoodBin+iTotalFailBin);   // Parts tested
    StdfFile.WriteDword(0);                     // Parts retested
    StdfFile.WriteDword(0);                     // Parts Aborted
    StdfFile.WriteDword(iTotalGoodBin);         // Good Parts
    StdfFile.WriteDword((DWORD)-1);             // Functionnal Parts
    StdfFile.WriteString(strValue.toLatin1().constData());  // WaferID
    StdfFile.WriteString("");                   // FabId
    StdfFile.WriteString("");                   // FrameId
    StdfFile.WriteString("");                   // MaskId
    StdfFile.WriteString("");                   // UserDesc
    StdfFile.WriteRecord();


    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 40;
    QMap<QString,CGSdiBin*>::Iterator itMapBin;
    for ( itMapBin = mapBinInfo.begin(); itMapBin != mapBinInfo.end(); ++itMapBin )
    {
        // Write HBR
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(255);                        // Test Head = ALL
        StdfFile.WriteByte(1);                          // Test sites = ALL
        StdfFile.WriteWord(itMapBin.value()->m_nNumber); // HBIN = 0
        StdfFile.WriteDword(itMapBin.value()->m_nCount); // Total Bins
        if(itMapBin.value()->m_bPassFlag)
            StdfFile.WriteByte('P');
        else
            StdfFile.WriteByte('F');
        StdfFile.WriteString(itMapBin.value()->m_strName.toLatin1().constData());
        StdfFile.WriteRecord();
    }

    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 50;

    for ( itMapBin = mapBinInfo.begin(); itMapBin != mapBinInfo.end(); ++itMapBin )
    {
        // Write SBR
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(255);                        // Test Head = ALL
        StdfFile.WriteByte(1);                          // Test sites = ALL
        StdfFile.WriteWord(itMapBin.value()->m_nNumber); // HBIN = 0
        StdfFile.WriteDword(itMapBin.value()->m_nCount); // Total Bins
        if(itMapBin.value()->m_bPassFlag)
            StdfFile.WriteByte('P');
        else
            StdfFile.WriteByte('F');
        StdfFile.WriteString(itMapBin.value()->m_strName.toLatin1().constData());
        StdfFile.WriteRecord();
    }

    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 30;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(255);                    // Test Head = ALL
    StdfFile.WriteByte(255);                    // Test sites = ALL
    StdfFile.WriteDword(iTotalGoodBin+iTotalFailBin);// Total Parts tested
    StdfFile.WriteDword(0);                     // Total Parts re-tested
    StdfFile.WriteDword(0);                     // Total Parts aborted
    StdfFile.WriteDword(iTotalGoodBin);         // Total GOOD Parts
    StdfFile.WriteRecord();

    // Write MRR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteDword(m_lStartTime);          // File finish-time.
    StdfFile.WriteRecord();

    // Close STDF file.
    StdfFile.Close();

    for ( itMapBin = mapBinInfo.begin(); itMapBin != mapBinInfo.end(); ++itMapBin )
        delete itMapBin.value();
    mapBinInfo.clear();

    // Success
    return true;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' Sdi file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGSditoSTDF::Convert(const char *SdiFileName, QStringList &lstFileNameSTDF)
{
    // No erro (default)
    m_iLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(SdiFileName);
    QFileInfo fOutput(lstFileNameSTDF.first());

    QFile f( lstFileNameSTDF.first() );
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
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(SdiFileName).fileName()+"...");
            GexScriptStatusLabel->show();
        }
        GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(SdiFileName).fileName()+"...");
        GexScriptStatusLabel->show();
    }
    QCoreApplication::processEvents();

    m_strStdfFileName = lstFileNameSTDF.first();
    lstFileNameSTDF.clear();
    if(ReadSdiFile(SdiFileName) != true)
    {
        while(!m_lstStdfFileName.isEmpty())
            QFile::remove(m_lstStdfFileName.takeFirst());
        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if((GexProgressBar != NULL)
                && bHideProgressAfter)
            GexProgressBar->hide();

        if((GexScriptStatusLabel != NULL)
                && bHideLabelAfter)
            GexScriptStatusLabel->hide();
        return false;	// Error reading Sdi file
    }

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    if((GexProgressBar != NULL)
            && bHideProgressAfter)
        GexProgressBar->hide();

    if((GexScriptStatusLabel != NULL)
            && bHideLabelAfter)
        GexScriptStatusLabel->hide();

    lstFileNameSTDF = m_lstStdfFileName;
    // Convertion successful
    return true;
}

//////////////////////////////////////////////////////////////////////
// Read line : skip empty line
//////////////////////////////////////////////////////////////////////
QString CGSditoSTDF::ReadLine(QTextStream& hFile)
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

    // Remove spaces under " "
    int iIndex = strString.indexOf(" \"");
    if(iIndex >= 0)
    {
        QString strValue = strString.section(" \"",1);
        strString = strString.section(" \"",0,0);
        while(!strValue.isEmpty())
        {
            strString += " " + strValue.section("\" ",0,0).replace(" ","_");
            strValue = strValue.section("\" ",1);

            iIndex = strValue.indexOf(" \"");
            if(iIndex >= 0)
            {
                strString += " " + strValue.section(" \"",0,0);
                strValue = strValue.section(" \"",1);
            }
            else
            {
                strString += " " + strValue;
                strValue = "";
            }
        }
    }

    return strString;

}
