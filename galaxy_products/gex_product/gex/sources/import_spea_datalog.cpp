//////////////////////////////////////////////////////////////////////
// import_spea_datalog.cpp: Convert a .SpeaDatalog (TSMC) file to STDF V4.0
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

#include "import_spea_datalog.h"
#include "import_constants.h"
#include "engine.h"

// File format:
//#
//Datalogging Output (Extended Format)
//
//Lot Number:           0F933W04
//Wafer Number:         33
//Operator Name:        BG52204
//Lot Start Date/Time:  Wed Oct 08 2008 21:35:45
//Test Program Name:    2006ep04
//Test Program Release: 04
//Test Program Remark:  - 30dBm Gain New Rev 27/5/04
//Sub Test Plan:        SubTP1 - SubTP1 Description...
//Handler Connection:   Wafer Prober
//Workstation Name:     Workstation1
//Tester Name:          Comptest MX
//Tester Number:        7
//Software:             AtosC Prod. SW Rel. 1.50/U6
//Datalog:              All the tasks defined 'To Datalog' in the test plan map.
//SPC Sampling Type:    Die
//SPC Sampling Mode:    Manual
//SPC Sampling Rate:    11
//SPC Sampling Limit:   3000
//
//
//"Device: 1      Site: 2    X/Y Coord: 28 , 22    Date: Wed Oct 08 2008 21:35:46"
//Task: Opens
//     1 VOPRT Open       Dout         -5.9   < -5.5925  < -5.2   V
//     1 VOPRT Open       Doutb        -5.9   < -5.8575  < -5.2   V
//     2 VNWD Open        Vcc1         -1.    < -0.55937 < -0.45  V
//     2 VNWD Open        Vcc2         -1.    < -1.08734 < -0.45  V     F
//Task: Shorts
//     3 IPRT2 Short      PINA         -250.  < -197.187 < -80.   uA
//Task: VccLink
//     6 VccLink          Failing Pin: Vcc2
//Task: VccN_StaticTests
//    40 Supply Curre     Vcc1         27.5   < 11.46404 < 34.5   mA    F
//    41 Dout Bias Le     Dout         2.1    < 2.370625 < 2.6    V
//    42 Doutb Bias L     Doutb        2.1    < -0.0925  < 2.6    V     F
//    43 VPINK Voltag     PINK         3.9    < 4.18     < 4.5    V
//    44 VPINA Voltag     PINA         1.6    < 2.801875 < 2.     V     F
//    46 Vbias            PINA         2.15   < 1.378125 < 2.55   V     F
//Task: VccNDynTst30dBm
//   540 Pin_15_volta     CM_PIN_15    0.8    < 1.00125  < 1.2
//   541 CM_Pin15_Cur     CM_PIN_15    1.6    < 1.778419 < 2.4    uA
//   542 Half_DC_Offs     CM_PIN_15    0.2    < 0.422108 < 0.8    V
//   543 Calc_DC_Offs     CM_PIN_15    1.8    < 3.308404 < 2.6    V     F
//   544 Offset Dout      Dout         2.1    < 2.313125 < 2.6    V
//Bin: 4

// File format 2:
//Philips Semiconductors, SPEA AtosC Datalog
//
//Date                : 07.01.2009
//Time                : 11:49:10
//
//base_program_name   : 85176
//
//card_drw_nr         : na
//card_id             : na
//check_sum           : na
//
//...
//
//=======================================================================================
//Device:1                               Site:1   Date:Wed Jan 07 2009 11:49:22
//Test Temperature: 25
//Task: "Open"
//100    "OpenToVss"        Failing Pins: 0
//101    %-18 Failing Pins: 0
//Task: "Short"
//200    "ShortToVss"       Failing Pins: 0
//201    "ShortToVddVlcd"   Failing Pins: 0
//202    "ShortEvenPins"    Failing Pins: 0
//203    %-18 Failing Pins: 0
//
//Hardbin : 1
//Bin: 1
//=======================================================================================

// DATE FORMAT AND INCONSISTENCY
// EX N°1 (french date 05.08.2009 + PM time)
// ignore second date
//NXP, SPEA AtosC Datalog

//Date                : 05.08.2009
//Time                : 20:06:23
//Device:1     Wafer_no:0  x:-4   y:4    Site:5   Date:Wed Aug 05 2009 08:06:28

// EX N°2 (EN date + FR date + inconsistancy)
// ignore second date
//Datalogging Output (Extended Format)

//Lot Number:           A25B
//Wafer Number:         9
//Operator Name:        Sergio
//Lot Start Date/Time:  Wed Apr 15 1998 11:01:09

//Device: 1      Site: 1    X/Y Coord: 10,11      Date: 23/01/1998 14:03:21


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

CSpeaDatalogBinInfo::CSpeaDatalogBinInfo()
{
    nNbCnt=0;
    nPassFlag=TEST_PASSFLAG_UNKNOWN;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGSpeaDatalogtoSTDF::CGSpeaDatalogtoSTDF()
{
    // Default: SpeaDatalog parameter list on disk includes all known SpeaDatalog parameters...
    m_lStartTime = 0;
    m_lStopTime = 0;

    m_bQuoteFormat = false;
    m_iTestNamePos  = 999;
    m_iPinNamePos   = 999;
    m_iLowLimitPos  = 999;
    m_iResultPos    = 999;
    m_iHighLimitPos = 999;
    m_iUnitPos      = 999;
    m_iFlagPos      = 999;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGSpeaDatalogtoSTDF::~CGSpeaDatalogtoSTDF()
{
    QMap<int,CSpeaDatalogBinInfo*>::Iterator itMapBin;
    for ( itMapBin = m_qMapHBins.begin(); itMapBin != m_qMapHBins.end(); ++itMapBin )
    {
        if(m_qMapSBins.contains(itMapBin.key()) && (m_qMapSBins[itMapBin.key()] == m_qMapHBins[itMapBin.key()]))
            m_qMapSBins[itMapBin.key()] = NULL;

        if(itMapBin.value() != NULL)
            delete itMapBin.value();
    }
    for ( itMapBin = m_qMapSBins.begin(); itMapBin != m_qMapSBins.end(); ++itMapBin )
    {
        if(itMapBin.value() != NULL)
            delete itMapBin.value();
    }
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGSpeaDatalogtoSTDF::GetLastError()
{
    QString strErrorMessage = "Import SpeaDatalog: ";

    switch(m_iLastError)
    {
    default:
    case errNoError:
        strErrorMessage += "No Error";
        break;
    case errOpenFail:
        strErrorMessage += "Failed to open file";
        break;
    case errInvalidFormat:
        strErrorMessage += "Invalid file format";
        break;
    case errInvalidFormatParameter:
        strErrorMessage += "Invalid file format: Didn't find LowLimit < Result < HighLimit Unit positions";
        break;
    case errWriteSTDF:
        strErrorMessage += "Failed creating temporary file. Folder permission issue?";
        break;
    case errLicenceExpired:
        strErrorMessage += "License has expired or Data file out of date...";
        break;
    }

    if(!m_strLastError.isEmpty())
        strErrorMessage += ". "+m_strLastError;

    // Return Error Message
    return strErrorMessage;
}

//////////////////////////////////////////////////////////////////////
// Normalize test limits when writing into PTR.
//////////////////////////////////////////////////////////////////////
void CGSpeaDatalogtoSTDF::NormalizeLimits(QString &strUnit, int &nScale)
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
// Check if File is compatible with SpeaDatalog format
//////////////////////////////////////////////////////////////////////
bool CGSpeaDatalogtoSTDF::IsCompatible(const char *szFileName)
{
    QString strString;
    QString strSection;
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
    QTextStream hSpeaDatalogFile(&f);

    // Check if first line is the correct SpeaDatalog header...
    //#
    //Datalogging Output (Extended Format)
    // OR
    //Philips Semiconductors, SPEA AtosC Datalog

    do
    {
        strString = hSpeaDatalogFile.readLine();
        if(strString.startsWith("#"))
            strString = "";
        if(strString.startsWith("="))
            strString = "";
    }
    while(!strString.isNull() && strString.isEmpty());

    if((strString.simplified().toLower() != "datalogging output (extended format)")
            && (strString.indexOf("spea atosc datalog",0,Qt::CaseInsensitive) < 0))
    {
        // Incorrect header...this is not a SpeaDatalog file!
        // Close file
        f.close();
        return false;
    }

    // Close file
    f.close();

    return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the SpeaDatalog file
//////////////////////////////////////////////////////////////////////
bool CGSpeaDatalogtoSTDF::ReadSpeaDatalogFile(const char *SpeaDatalogFileName,const char *strFileNameSTDF)
{
    QString strString;
    QString strSection;
    QString strValue;

    // Open SpeaDatalog file
    QFile f( SpeaDatalogFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening SpeaDatalog file
        m_iLastError = errOpenFail;
        m_strLastError = f.errorString();

        // Convertion failed.
        return false;
    }
    // Assign file I/O stream
    QTextStream hSpeaDatalogFile(&f);

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    iProgressStep = 0;
    iNextFilePos = 0;
    iFileSize = f.size() + 1;
    QCoreApplication::processEvents();

    // Check if first line is the correct SpeaDatalog header...
    //#
    //Datalogging Output (Extended Format)
    // OR
    //Philips Semiconductors, SPEA AtosC Datalog

    strString = ReadLine(hSpeaDatalogFile).remove('"');

    if((strString.toLower() != "datalogging output (extended format)")
            && (strString.indexOf("spea atosc datalog",0,Qt::CaseInsensitive) < 0))
    {
        // Incorrect header...this is not a SpeaDatalog file!
        m_iLastError = errInvalidFormat;
        m_strLastError = "Invalid header";

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    QString strDate;
    QString strTime;

    // Read SpeaDatalog information
    //Lot Number:           0F933W04
    //Wafer Number:         33
    //Operator Name:        BG52204
    //Lot Start Date/Time:  Wed Oct 08 2008 21:35:45
    //Test Program Name:    2006ep04
    //Test Program Release: 04
    //Test Program Remark:  - 30dBm Gain New Rev 27/5/04
    //Sub Test Plan:        SubTP1 - SubTP1 Description...
    //Handler Connection:   Wafer Prober
    //Workstation Name:     Workstation1
    //Tester Name:          Comptest MX
    //Tester Number:        7
    //Software:             AtosC Prod. SW Rel. 1.50/U6
    //Datalog:              All the tasks defined 'To Datalog' in the test plan map.
    //SPC Sampling Type:    Die
    //SPC Sampling Mode:    Manual
    //SPC Sampling Rate:    11
    //SPC Sampling Limit:   3000
    while(!hSpeaDatalogFile.atEnd())
    {
        strString = ReadLine(hSpeaDatalogFile).remove('"');

        strSection = strString.section(":",0,0).simplified().toUpper();
        strString = strString.section(":",1);

        if(strString.simplified().toLower() == "na")
            continue;

        if((strSection == "LOT NUMBER") || (strSection == "LOT_ID"))
        {
            m_strLotId = strString.simplified();
        }
        else if(strSection == "SUBLOT_ID")
        {
            m_strSubLotId += " " + strString.simplified();
        }
        else if((strSection == "WAFER NUMBER") || (strSection == "WAFER_ID"))
        {
            m_strWaferId = strString.simplified();
            if(m_strWaferId.isEmpty())
                m_strWaferId = "1";
        }
        else if((strSection == "OPERATOR NAME") || (strSection == "OPERATOR"))
        {
            m_strOperatorName = strString.simplified();
        }
        else if(strSection == "LOT START DATE/TIME")
        {
            // ddd mmm dd yyyy
            // Wed Oct 08 2008 21:47:50
            m_lStartTime = GetDateTimeFromString(strString.simplified());
        }
        else if(strSection == "DATE")
        {
            // dd.mm.yyyy
            // 07.01.2009
            strDate = strString.simplified();
        }
        else if(strSection == "TIME")
        {
            // 11:49:10
            strTime = strString.simplified();
        }
        else if((strSection == "TEST PROGRAM NAME") || (strSection == "PROGRAM_NAME"))
        {
            m_strProgramId = strString.simplified();
        }
        else if((strSection == "TEST PROGRAM RELEASE") || (strSection == "PROGRAM_VARSION"))
        {
            m_strProgramRev = strString.simplified();
        }
        else if(strSection == "TEST PROGRAM REMARK")
        {
            m_strProgramId += " " + strString.simplified();
        }
        else if(strSection == "PROCESS_CODE")
        {
            m_strProcessId += " " + strString.simplified();
        }
        else if(strSection == "ROM_CODE")
        {
            m_strRomCode += " " + strString.simplified();
        }
        else if(strSection == "WORKSTATION NAME")
        {
            m_strNodeId = strString.simplified();
        }
        else if(strSection == "TESTER_NR")
        {
            m_strNodeId = strString.simplified();
        }
        else if(strSection == "TEST_TEMPERATURE")
        {
            m_strTestTemperature = strString.simplified();
        }
        else if(strSection == "DEVICE_NAME")
        {
            m_strPartType = strString.simplified();
        }
        else if(strSection == "PACKAGE")
        {
            m_strPackageType = strString.simplified();
        }
        else if(strSection == "HANDLER")
        {
            m_strHandType = strString.simplified();
        }
        else if(strSection == "HANDLER_ID")
        {
            m_strHandId = strString.simplified();
        }
        else if(strSection == "LOADBOARD_ID")
        {
            m_strLoadId = strString.simplified();
        }
        else if(strSection == "TESTER NAME")
        {
            m_strTestProgram = strString.simplified();
        }
        else if(strSection == "TESTER NUMBER")
        {
            m_strTestRev = strString.simplified();
        }
        else if(strSection == "SOFTWARE")
        {
            m_strSoftwareName = strString.simplified();
        }
        else if(strSection == "SPEA_RELEASE")
        {
            m_strSoftwareRev = strString.simplified();
        }
        else if(strSection == "DEVICE")
        {
            // Go throw the first run to collect all PinName and create PMR
            // until the first Bin 1
            //"Device: 1      Site: 2    X/Y Coord: 28 , 22    Date: Wed Oct 08 2008 21:35:46"
            //Task: Opens
            //     1 VOPRT Open       Dout         -5.9   < -5.5925  < -5.2   V
            //     1 VOPRT Open       Doutb        -5.9   < -5.8575  < -5.2   V
            //     2 VNWD Open        Vcc1         -1.    < -0.55937 < -0.45  V
            //     2 VNWD Open        Vcc2         -1.    < -1.08734 < -0.45  V     F
            //Task: Shorts
            //     3 IPRT2 Short      PINA         -250.  < -197.187 < -80.   uA
            //Task: VccLink
            //     6 VccLink          Failing Pin: Vcc2
            //Task: VccN_StaticTests
            //    40 Supply Curre     Vcc1         27.5   < 11.46404 < 34.5   mA    F
            //    41 Dout Bias Le     Dout         2.1    < 2.370625 < 2.6    V
            //    42 Doutb Bias L     Doutb        2.1    < -0.0925  < 2.6    V     F
            //    43 VPINK Voltag     PINK         3.9    < 4.18     < 4.5    V
            //    44 VPINA Voltag     PINA         1.6    < 2.801875 < 2.     V     F
            //    46 Vbias            PINA         2.15   < 1.378125 < 2.55   V     F
            //Task: VccNDynTst30dBm
            //   540 Pin_15_volta     CM_PIN_15    0.8    < 1.00125  < 1.2
            //   541 CM_Pin15_Cur     CM_PIN_15    1.6    < 1.778419 < 2.4    uA
            //   542 Half_DC_Offs     CM_PIN_15    0.2    < 0.422108 < 0.8    V
            //   543 Calc_DC_Offs     CM_PIN_15    1.8    < 3.308404 < 2.6    V     F
            //   544 Offset Dout      Dout         2.1    < 2.313125 < 2.6    V
            //Bin: 4

            bool bIsValue;
            int iTestNumber;
            int iLastTestNumber;
            QString strLastLine;

            iLastTestNumber = -1;
            while(!hSpeaDatalogFile.atEnd())
            {
                strString = ReadLine(hSpeaDatalogFile);

                if((strString.indexOf(":") > 0) && !strString.contains("Failing Pin"))
                {
                    strSection = strString.section(":",0,0).simplified().toUpper();
                    strString = strString.section(":",1);

                    if(strSection == "DEVICE")
                    {
                        if(strString.contains("DATE:",Qt::CaseInsensitive))
                        {
                            m_lStopTime = GetDateTimeFromString(strString.section("Date:",1).simplified());
                        }
                    }
                    else if(strSection == "BIN")
                    {
                        // Need to have all the flow
                        if(strString.simplified() == "1")
                            break;

                    }
                    else if(strSection == "TEST TEMPERATURE")
                    {
                        if(strString.simplified().toLower() != "na")
                            m_strTestTemperature = strString.simplified();
                    }
                    continue;
                }


                iTestNumber = GetTestNum(strString,bIsValue);
                if(bIsValue)
                {
                    // Save the position for LowLimit < Result < HighLimit Unit
                    //000000000011111111112222222222333333333344444444445555555555
                    //012345678901234567890123456789012345678901234567890123456789
                    //     10 High Level       Clock           -10.000 <      6.270 <     10.000 uA
                    // 2307   "IDDQ08"           "VDD"          -0.300    > 1.497       < 5.000     uA      p
                    //    540 Pin_15_volta     CM_PIN_15    0.8    < 1.04875  < 1.2
                    //7002   "Leakage_BP_SEG"   "S5"           -500.000  > 0.000       < 500.000   nA      p
                    // 1880   "IDD1_5_5V_6_5MHz" "VDD"          80.000    > 637.377     < 800.000   uA      p
                    // 13     "Open to VDD"      "S59"          -0.900    > -0.671      < -0.200     V      p
                    //7008   "Vbg7_2_8v"                        "A0"           0.500     > 1.001       < 2.000      V      p
                    //30001  "Initial Func"                                    "100Khz"                                    p

                    if(m_bQuoteFormat && strString.contains("%-18"))
                        strString = strString.replace("%-18","\" \"");

                    //if((m_iTestNamePos < 0) || (m_iPinNamePos < 0) || (m_iFlagPos < 0))
                    {
                        // TestName and PinName position
                        int nPos = strString.lastIndexOf('"');
                        if(nPos>0)
                        {
                            m_bQuoteFormat = true;
                            if(strString.count('"') == 2)
                            {
                                //2000   I Avdd disable           "Avdd"       100.00     < 538.46     < 900.00     uA      p
                                // Only one name is quoted
                                // Check if have a valid name between testNumber and quote
                                if(!strString.mid(7).section('"',0,0).remove(" ").isEmpty())
                                    m_iTestNamePos = 7;
                            }
                            if(m_iTestNamePos >= 999)
                                m_iTestNamePos = strString.indexOf('"');
                            nPos = strString.lastIndexOf('"',strString.lastIndexOf('"')-1);
                            if(nPos > m_iTestNamePos)
                            {
                                if((nPos>0) && (nPos<m_iPinNamePos))
                                    m_iPinNamePos = nPos;
                            }
                        }
                        else
                        {
                            if(m_bQuoteFormat == true)
                            {
                                // Mix definition
                                // Incorrect parameter line ...this is not a valid file!
                                m_iLastError = errInvalidFormat;
                                m_strLastError = "Test results definition with and without \"";
                                // Convertion failed.
                                // Close file
                                f.close();
                                return false;
                            }
                            if(m_iTestNamePos >= 999)   m_iTestNamePos  = 7;
                            if(m_iPinNamePos >= 999)    m_iPinNamePos   = 24;
                        }

                        // Initialization
                        QRegExp clSep("[<>]");
                        if(strString.count(clSep) == 2)
                        {
                            // Result position
                            nPos = strString.indexOf(clSep)+1;
                            if((nPos>0) && (nPos<m_iResultPos))
                                m_iResultPos = nPos;
                            // HighLimit position
                            nPos = strString.indexOf(clSep,m_iResultPos)+1;
                            if((nPos>0) && (nPos<m_iHighLimitPos))
                                m_iHighLimitPos = nPos;
                            // LowLimit position
                            if(m_iResultPos-12<m_iLowLimitPos)
                                m_iLowLimitPos = m_iResultPos-12;

                            // Find the first number
                            if(m_bQuoteFormat)
                            {
                                nPos = strString.lastIndexOf('"')+1;
                                strValue = "x"+strString.mid(nPos,(m_iResultPos-2)-nPos);
                                strValue = strValue.simplified().section(" ",1);
                            }
                            else
                            {
                                if(m_iLowLimitPos >= 999)
                                    nPos = m_iResultPos-12;
                                else
                                    nPos = m_iLowLimitPos-5;
                                strValue = "x"+strString.mid(nPos,(m_iResultPos-2)-nPos);
                                strValue = strValue.simplified().section(" ",1);
                            }
                            if(!strValue.isEmpty())
                            {
                                nPos = strString.indexOf(strValue,m_iPinNamePos+1)-1;
                                if((nPos>0) && (nPos<m_iLowLimitPos))
                                    m_iLowLimitPos = nPos;
                            }
                            // Find the last number
                            strValue = strString.mid(m_iHighLimitPos).simplified().section(" ",0,0);
                            if(!strValue.isEmpty())
                            {
                                // is it the HighLimit value ?
                                nPos = strString.indexOf(strValue,m_iHighLimitPos);
                                if((nPos<(m_iHighLimitPos+6)) && !strString.mid(m_iHighLimitPos,nPos).simplified().isEmpty())
                                    nPos += strValue.length()+1;
                                // Find the unit
                                strValue = strString.mid(nPos).simplified().section(" ",0,0);
                                if((strValue.toLower() != "p") && (strValue.toLower() != "f"))
                                {
                                    if(!strValue.isEmpty())
                                    {
                                        nPos = strString.indexOf(strValue,m_iHighLimitPos+1);
                                        if((nPos>0) && (nPos<m_iUnitPos))
                                            m_iUnitPos = nPos;
                                        // Find the Flag
                                        strValue = strString.mid(m_iUnitPos).simplified().section(" ",1);
                                        if((strValue.toLower() == "p") || (strValue.toLower() == "f"))
                                        {
                                            nPos = strString.indexOf(" "+strValue,m_iUnitPos)+1;
                                            if((nPos>0) && (nPos<m_iFlagPos))
                                                m_iFlagPos = nPos;
                                        }
                                    }
                                }
                                else
                                {
                                    nPos = strString.indexOf(" "+strValue,m_iHighLimitPos+1)+1;
                                    if((nPos>0) && (nPos<m_iFlagPos))
                                        m_iFlagPos = nPos;
                                }
                            }
                        }
                        if(strString.count(clSep) > 0)
                        {
                            // Find the Flag
                            strValue = strString.simplified().section(" ",-1);
                            if((strValue.toLower() == "p") || (strValue.toLower() == "f"))
                            {
                                nPos = strString.indexOf(" "+strValue)+1;
                                if((nPos>0) && (nPos<m_iFlagPos))
                                    m_iFlagPos = nPos;
                            }
                        }
                    }
                }
                if(iTestNumber == iLastTestNumber)
                {

                    // Same test number PMR
                    // Construct a unique PinName with the test number and the name of the Pin
                    if(!strLastLine.isEmpty())
                    {
                        strLastLine = "[" + QString::number(GetTestNum(strLastLine,bIsValue)) + "]" + GetPinName(strLastLine);
                        if(!m_qMapPinIndex.contains(strLastLine))
                            m_qMapPinIndex[strLastLine] = m_qMapPinIndex.count()+1;

                        strLastLine = "";
                    }

                    strString = "[" + QString::number(GetTestNum(strString,bIsValue)) + "]" + GetPinName(strString);
                    if(!m_qMapPinIndex.contains(strString))
                        m_qMapPinIndex[strString] = m_qMapPinIndex.count()+1;
                }
                else
                    strLastLine = strString;

                iLastTestNumber = iTestNumber;

            }

            break;
        }
    }

    //if(m_iLowLimitPos >= 999)   m_iLowLimitPos  = 39;
    //if(m_iResultPos >= 999)     m_iResultPos    = 52;
    //if(m_iHighLimitPos >= 999)  m_iHighLimitPos = 65;
    //if(m_iUnitPos >= 999)       m_iUnitPos      = 76;
    //if(m_iFlagPos >= 999)       m_iFlagPos      = 78;

    if((m_iLowLimitPos >= 999) || (m_iResultPos >= 999) || (m_iHighLimitPos >= 999) || (m_iUnitPos >= 999))
    {
        // Incorrect parameter line ...this is not a valid file!
        m_iLastError = errInvalidFormatParameter;
        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    if(m_lStartTime <= 0)
    {
        if(!strDate.isEmpty())
            m_lStartTime = GetDateTimeFromString(strDate+" "+strTime);
    }

    // Restart to the begining
    hSpeaDatalogFile.seek(0);

    if(!WriteStdfFile(&hSpeaDatalogFile, strFileNameSTDF))
    {
        QFile::remove(strFileNameSTDF);
        f.close();
        return false;
    }

    f.close();

    // Success parsing SpeaDatalog file
    return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from SpeaDatalog data parsed
//////////////////////////////////////////////////////////////////////
bool CGSpeaDatalogtoSTDF::WriteStdfFile(QTextStream *hSpeaDatalogFile,const char *strFileNameSTDF)
{
    // now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open((char*)strFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        // Failed importing SpeaDatalog file into STDF database
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
    {
        if(m_lStopTime > 0)
            m_lStartTime = m_lStopTime;
        else
            m_lStartTime = QDateTime::currentDateTime().toTime_t();
    }

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
    StdfFile.WriteString(m_strLotId.toLatin1().constData());        // Lot ID
    StdfFile.WriteString(m_strPartType.toLatin1().constData());     // Part Type / Product ID
    StdfFile.WriteString(m_strNodeId.toLatin1().constData());       // Node name
    StdfFile.WriteString(m_strTestProgram .toLatin1().constData()); // Tester Type
    StdfFile.WriteString(m_strProgramId.toLatin1().constData());    // Job name
    StdfFile.WriteString(m_strProgramRev.toLatin1().constData());   // Job rev
    StdfFile.WriteString(m_strSubLotId.toLatin1().constData());     // sublot-id
    StdfFile.WriteString(m_strOperatorName.toLatin1().constData()); // operator
    StdfFile.WriteString(m_strSoftwareName.toLatin1().constData()); // exec-type
    StdfFile.WriteString(m_strSoftwareRev.toLatin1().constData());  // exe-ver
    StdfFile.WriteString(m_strWaferId.isEmpty()?(char*)"":(char*)"WAFER");  // test-cod
    StdfFile.WriteString(m_strTestTemperature.toLatin1().constData());      // test-temperature
    // Construct custom Galaxy USER_TXT
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    strUserTxt += ":SPEA_DATALOG";
    StdfFile.WriteString((char *)strUserTxt.toLatin1().constData());    // user-txt
    StdfFile.WriteString("");                   // aux-file
    StdfFile.WriteString(m_strPackageType.toLatin1().constData());  // package-type
    StdfFile.WriteString("");                   // familyID
    StdfFile.WriteString("");                   // Date-code
    StdfFile.WriteString("");                   // Facility-ID
    StdfFile.WriteString("");                   // FloorID
    StdfFile.WriteString(m_strProcessId.toLatin1().constData());    // ProcessID
    StdfFile.WriteString("");                   // Oper-Freq
    StdfFile.WriteString("");                   // Spec-Nam
    StdfFile.WriteString("");                   // Spec-Ver
    StdfFile.WriteString("");                   // Flow-Id
    StdfFile.WriteString("");                   // Setup-Id
    StdfFile.WriteString("");                   // Dsgn-rev
    StdfFile.WriteString("");                   // Eng-id
    StdfFile.WriteString(m_strRomCode.toLatin1().constData());      // Rom-Id


    StdfFile.WriteRecord();

    if(!m_strHandType.isEmpty() || !m_strHandId.isEmpty() || !m_strLoadId.isEmpty())
    {
        // Write SDR
        RecordReadInfo.iRecordType = 1;
        RecordReadInfo.iRecordSubType = 80;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte((BYTE)1);            // head#
        StdfFile.WriteByte((BYTE)1);            // Group#
        StdfFile.WriteByte((BYTE)1);            // site_count
        StdfFile.WriteByte((BYTE)1);            // array of test site# (dummy!)
        StdfFile.WriteString(m_strHandType.toLatin1().constData()); // HAND_TYP: Handler/prober type
        StdfFile.WriteString(m_strHandId.toLatin1().constData());   // HAND_ID: Handler/prober name
        StdfFile.WriteString("");                               // CARD_TYP: Probe card type
        StdfFile.WriteString("");                               // CARD_ID: Probe card name
        StdfFile.WriteString("");                               // LOAD_TYP: Load board type
        StdfFile.WriteString(m_strLoadId.toLatin1().constData());   // LOAD_ID: Load board name
        StdfFile.WriteString("");                               // DIB_TYP: DIB board type
        StdfFile.WriteString("");                               // DIB_ID: DIB board name
        StdfFile.WriteString("");                               // CABL_TYP: Interface cable type
        StdfFile.WriteString("");                               // CABL_ID: Interface cable name
        StdfFile.WriteString("");                               // CONT_TYP: Handler contactor type
        StdfFile.WriteString("");                               // CONT_ID: Handler contactor name
        StdfFile.WriteString("");                               // LASR_TYP: Laser type
        StdfFile.WriteString("");                               // LASR_ID: Laser name
        StdfFile.WriteString("");                               // EXTR_TYP: Extra equipment type
        StdfFile.WriteString("");                               // EXTR_ID: Extra equipment name
        StdfFile.WriteRecord();
    }

    QString     strString;
    QString     strSection;
    WORD        wSoftBin,wHardBin;
    long        iTotalGoodBin,iTotalFailBin;
    long        iTotalTests,iPartNumber;
    bool        bIsValue;
    bool        bPassStatus;
    BYTE        bData;

    int         iIndex;
    int         iBin;
    int         iXWafer, iYWafer;
    int         iSiteNumber=0;
    int         iTestNumber;
    int         iLastTestNumber;
    int         iPinNumber;
    bool        bFunctionalTest;
    bool        bParametricTest;
    bool        bTestPass;;
    QString     strSequenceName;
    QString     strTestName;
    QString     strPinName;
    QString     strUnit;
    QString     strValue;
    int         nScale;
    float       fValue;
    float       fLowLimit=0;
    float       fHighLimit=0;
    bool        bHaveLowLimit, bHaveHighLimit;
    QMap<QString,int> mapTestScale;
    QMap<QString,QString> mapPinResult;
    QMap<QString,int>::Iterator itStringInt;
    QMap<QString,QString>::Iterator itPinResult;
    int         iPosX;
    int         iPosY;
    int         iPosSite;
    int         iPosWafer;
    int         iPosDate;

    bFunctionalTest = bParametricTest = false;
    iPosX = iPosY = iPosSite = iPosWafer = iPosDate = -1;

    iXWafer = iYWafer = -32768;
    // Write PMR record if any
    if(m_qMapPinIndex.count() > 0)
    {

        for(itStringInt=m_qMapPinIndex.begin(); itStringInt!=m_qMapPinIndex.end(); itStringInt++)
        {
            strPinName = itStringInt.key().section("]",1);
            iPinNumber = itStringInt.value();

            // Write PMR pin info.
            RecordReadInfo.iRecordType = 1;
            RecordReadInfo.iRecordSubType = 60;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteWord(iPinNumber);                     // Pin Index
            StdfFile.WriteWord(0);                              // Channel type
            StdfFile.WriteString(strPinName.toLatin1().constData());    // Channel name
            StdfFile.WriteString(strPinName.toLatin1().constData());    // Physical name
            StdfFile.WriteString(strPinName.toLatin1().constData());    // Logical name
            StdfFile.WriteByte(1);                              // Test head
            StdfFile.WriteByte(255);                            // Tester site (all)
            StdfFile.WriteRecord();
        }

    }

    if(!m_strWaferId.isEmpty())
    {
        // Write WIR of new Wafer.
        RecordReadInfo.iRecordType = 2;
        RecordReadInfo.iRecordSubType = 10;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);                          // Test head
        StdfFile.WriteByte(255);                        // Tester site (all)
        StdfFile.WriteDword(m_lStartTime);              // Start time
        StdfFile.WriteString(m_strWaferId.toLatin1().constData());  // WaferID
        StdfFile.WriteRecord();
    }

    // Write Test results for each line read.
    // Reset counters
    iTotalGoodBin=iTotalFailBin=0;
    iPartNumber=0;

    // Write all Parameters read on this file : PIR,PTR...,PRR, PIR,PTR..., PRR
    // Read SpeaDatalog information
    //"Device: 1      Site: 2    X/Y Coord: 28 , 22    Date: Wed Oct 08 2008 21:35:46"
    //Task: Opens
    //     1 VOPRT Open       Dout         -5.9   < -5.5925  < -5.2   V
    //     1 VOPRT Open       Doutb        -5.9   < -5.8575  < -5.2   V
    //     2 VNWD Open        Vcc1         -1.    < -0.55937 < -0.45  V
    //     2 VNWD Open        Vcc2         -1.    < -1.08734 < -0.45  V     F
    //Bin: 4
    while(!hSpeaDatalogFile->atEnd())
    {
        strString = ReadLine(*hSpeaDatalogFile).remove('"');

        strSection = strString.section(":",0,0).simplified().toUpper();
        strString = strString.section(":",1);

        if(strSection == "DEVICE")
        {
            //Device: 1      Site: 2    X/Y Coord: 28 , 22    Date: Wed Oct 08 2008 21:35:46
            //Device:2                               Site:2   Date:Wed Jan 07 2009 11:49:22

            // 2009 08 25
            // New format
            //Device:0     Wafer_no:0  x:-6   y:-21  Site:3   Date:Di Jun 16 2009 05:52:09


            iPartNumber = strString.left(7).simplified().toInt();

            if((iPosX < 0) && (iPosY < 0) && (iPosSite < 0) && (iPosWafer < 0) && (iPosDate < 0))
            {
                // Never computed
                iPosSite = strString.indexOf("Site",0,Qt::CaseInsensitive);
                if(iPosSite > 0)
                    iPosSite += QString("Site:").length();

                iPosWafer = strString.indexOf("Wafer_no",0,Qt::CaseInsensitive);
                if(iPosWafer > 0)
                    iPosWafer += QString("Wafer_no:").length();

                iPosX = strString.indexOf("X/Y Coord",0,Qt::CaseInsensitive);
                if(iPosX > 0)
                {
                    iPosX += QString("X/Y Coord:").length();
                    iPosY = -1;
                }
                else
                {
                    iPosX = strString.indexOf("x:",0,Qt::CaseInsensitive);
                    if(iPosX > 0)
                        iPosX += QString("x:").length();

                    iPosY = strString.indexOf("y:",0,Qt::CaseInsensitive);
                    if(iPosY > 0)
                        iPosY += QString("y:").length();

                }

                iPosDate = strString.indexOf("Date",0,Qt::CaseInsensitive);
                if(iPosDate > 0)
                    iPosDate += QString("Date:").length();

            }

            if(iPosSite > 0)
                iSiteNumber = strString.mid(iPosSite,4).simplified().toInt();

            if(iPosX > 0)
            {
                if(iPosY < 0)
                {
                    iXWafer = strString.mid(iPosX,12).section(",",0,0).simplified().toInt();
                    iYWafer = strString.mid(iPosX,12).section(",",1).simplified().toInt();
                }
                else
                {
                    iXWafer = strString.mid(iPosX,5).simplified().toInt();
                    iYWafer = strString.mid(iPosY,5).simplified().toInt();
                }
            }
            else
                iXWafer = iYWafer = -32768;

            if(iPosDate > 0)
            {
                strString = strString.mid(iPosDate);

                // Wed Oct 08 2008 21:47:50
                long lTime = GetDateTimeFromString(strString.simplified());
                if(lTime > m_lStopTime)
                    m_lStopTime = lTime;
            }


            // Write PIR
            // Write PIR for parts in this Wafer site
            RecordReadInfo.iRecordType = 5;
            RecordReadInfo.iRecordSubType = 10;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(1);                              // Test head
            StdfFile.WriteByte(iSiteNumber);                    // Tester site
            StdfFile.WriteRecord();

            // Reset Pass/Fail flag.
            bPassStatus = true;
            bTestPass = true;
            iBin = 0;

            // Reset counters
            iTotalTests = 0;
            iTestNumber = iLastTestNumber = -1;
            mapPinResult.clear();

            while(!hSpeaDatalogFile->atEnd())
            {
                strString = ReadLine(*hSpeaDatalogFile);

                strSection = strString.section(":",0,0).simplified().toUpper();
                //Task: Opens
                //     1 VOPRT Open       Dout         -5.9   < -5.5925  < -5.2   V
                //     1 VOPRT Open       Doutb        -5.9   < -5.8575  < -5.2   V

                iTestNumber = GetTestNum(strString,bIsValue);
                if(bIsValue)
                {
                    if(iLastTestNumber == -1)
                        iLastTestNumber = iTestNumber;
                }
                else
                    iTestNumber = -1;

                if(iLastTestNumber != iTestNumber)
                {
                    // Save the last results

                    // Check if it is a pattern or a value
                    strPinName = mapPinResult.begin().key();
                    strValue = mapPinResult.begin().value();
                    fValue = strValue.toFloat(&bIsValue);

                    // Check the number of Pin Result
                    if(mapPinResult.count() == 1)
                    {
                        // PTR OR FTR
                        if(bParametricTest)
                        {
                            // Parametric test

                            if(strPinName != "NOPIN")
                                strTestName += " - " + strPinName;

                            // Write PTR
                            RecordReadInfo.iRecordType = 15;
                            RecordReadInfo.iRecordSubType = 10;

                            StdfFile.WriteHeader(&RecordReadInfo);
                            StdfFile.WriteDword(iLastTestNumber);       // Test Number
                            StdfFile.WriteByte(1);                      // Test head
                            StdfFile.WriteByte(iSiteNumber);            // Tester site:1,2,3,4 or 5, etc.
                            if(bTestPass)
                                bData = 0;      // Test passed
                            else
                                bData = BIT7;   // Test Failed
                            StdfFile.WriteByte(bData);                  // TEST_FLG
                            bData = 0;
                            StdfFile.WriteByte(bData);                  // PARAM_FLG
                            StdfFile.WriteFloat(fValue * GS_POW(10.0,nScale));    // Test result
                            if(!mapTestScale.contains(QString::number(iLastTestNumber) + strTestName))
                            {
                                // save Parameter name without unit information
                                StdfFile.WriteString(strTestName.toLatin1().constData());   // TEST_TXT
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
                                StdfFile.WriteFloat(fHighLimit * GS_POW(10.0,nScale));    // HIGH Limit
                                StdfFile.WriteString(strUnit.toLatin1().constData());   // Units
                            }
                            StdfFile.WriteRecord();
                        }
                        else
                        {
                            // Functional test
                            int		nFailingPins;
                            QString strPattern;
                            BYTE	bOptFlag;

                            if(strPinName != "NOPIN")
                                strTestName += " - " + strPinName;

                            nFailingPins = 1;
                            if(bIsValue)
                                nFailingPins = (int)fValue;
                            else
                                strPattern = strValue;

                            bOptFlag = BIT0|BIT1|BIT2|BIT4|BIT5|BIT6|BIT7;

                            // Write FTR
                            RecordReadInfo.iRecordType = 15;
                            RecordReadInfo.iRecordSubType = 20;

                            StdfFile.WriteHeader(&RecordReadInfo);
                            StdfFile.WriteDword(iLastTestNumber);           // Test Number
                            StdfFile.WriteByte(1);                          // Test head
                            StdfFile.WriteByte(iSiteNumber);                // Tester site:1,2,3,4 or 5, etc.
                            if(bTestPass)
                                bData = 0;		// Test passed
                            else
                                bData = BIT7;	// Test Failed
                            StdfFile.WriteByte(bData);                      // TEST_FLG

                            // save the name of the test
                            StdfFile.WriteByte(bOptFlag);       // opt_flg
                            StdfFile.WriteDword(0);             // cycl_cnt
                            StdfFile.WriteDword(0);             // rel_vadr
                            StdfFile.WriteDword(0);             // rept_cnt
                            StdfFile.WriteDword(nFailingPins);  // num_fail
                            StdfFile.WriteDword(0);             // xfail_ad
                            StdfFile.WriteDword(0);             // yfail_ad
                            StdfFile.WriteWord(0);              // vect_off
                            StdfFile.WriteWord(0);              // rtn_icnt
                            StdfFile.WriteWord(0);
                            StdfFile.WriteWord(0);
                            StdfFile.WriteString(strPattern.toLatin1().constData());    // vect_name
                            StdfFile.WriteString("");           // time_set
                            StdfFile.WriteString("");           // op_code
                            StdfFile.WriteString(strTestName.toLatin1().constData());   // test_txt: test name
                            StdfFile.WriteString("");           // alarm_id
                            StdfFile.WriteString("");           // prog_txt
                            StdfFile.WriteString("");           // rslt_txt


                            StdfFile.WriteRecord();
                        }
                    }
                    else
                    {
                        // Multi results FTR or MPR

                        if(bParametricTest)
                        {
                            // Multi parametric results

                            // Write MPR
                            RecordReadInfo.iRecordType = 15;
                            RecordReadInfo.iRecordSubType = 15;

                            StdfFile.WriteHeader(&RecordReadInfo);
                            StdfFile.WriteDword(iLastTestNumber);   // Test Number
                            StdfFile.WriteByte(1);                  // Test head
                            StdfFile.WriteByte(iSiteNumber);        // Tester site:1,2,3,4 or 5, etc.
                            bData = 0;
                            if(!bTestPass)
                                bData |= BIT7;	// Test Failed

                            StdfFile.WriteByte(bData);                  // TEST_FLG
                            bData = BIT6|BIT7;
                            StdfFile.WriteByte(bData);                  // PARAM_FLG
                            StdfFile.WriteWord(mapPinResult.count());   // RTN_ICNT
                            StdfFile.WriteWord(mapPinResult.count());   // RSLT_CNT
                            bData = 0;
                            for(iIndex=0; iIndex!=(mapPinResult.count()+1)/2; iIndex++)
                            {
                                StdfFile.WriteByte(0);                  // RTN_STAT
                            }
                            for(itPinResult=mapPinResult.begin(); itPinResult!=mapPinResult.end(); itPinResult++)
                            {
                                StdfFile.WriteFloat(itPinResult.value().toFloat(&bIsValue) * GS_POW(10.0,nScale));			// Test result
                                if(!bIsValue)
                                {
                                    // Mix definition
                                    // Incorrect parameter line ...this is not a valid file!
                                    m_iLastError = errInvalidFormat;
                                    m_strLastError = "Test type redefinition (mix PTR/FTR) "+strTestName+"["+QString::number(iLastTestNumber)+"]";
                                    // Convertion failed.
                                    // Close file
                                    StdfFile.Close();
                                    return false;
                                }
                            }
                            StdfFile.WriteString(strTestName.toLatin1().constData());	// TEST_TXT
                            StdfFile.WriteString("");                   // ALARM_ID

                            if(!mapTestScale.contains(QString::number(iLastTestNumber) + strTestName))
                            {
                                // No Scale result
                                bData = 0;
                                if(nScale == 0)
                                    bData |= BIT0;
                                // No LowLimit
                                if(!bHaveLowLimit)
                                {
                                    bData |= BIT2;
                                    bData |= BIT6;
                                }
                                // No HighLimit
                                if(!bHaveHighLimit)
                                {
                                    bData |= BIT3;
                                    bData |= BIT7;
                                }
                                StdfFile.WriteByte(bData);                  // OPT_FLAG
                                StdfFile.WriteByte(-nScale);                // RES_SCALE
                                StdfFile.WriteByte(-nScale);                // LLM_SCALE
                                StdfFile.WriteByte(-nScale);                // HLM_SCALE
                                StdfFile.WriteFloat(fLowLimit * GS_POW(10.0,nScale));   // LOW Limit
                                StdfFile.WriteFloat(fHighLimit * GS_POW(10.0,nScale));  // HIGH Limit
                                StdfFile.WriteFloat(0);                     // StartIn
                                StdfFile.WriteFloat(0);                     // IncrIn
                                for(itPinResult=mapPinResult.begin(); itPinResult!=mapPinResult.end(); itPinResult++)
                                {
                                    StdfFile.WriteWord(m_qMapPinIndex["[" + QString::number(iLastTestNumber) + "]" + itPinResult.key()]);		// RTN_INDX
                                }
                                StdfFile.WriteString(strUnit.toLatin1().constData());	// Units
                                StdfFile.WriteString("");               // units_in
                                StdfFile.WriteString("");               // c_resfmt
                                StdfFile.WriteString("");               // c_llmfmt
                                StdfFile.WriteString("");               // c_hlmfmt
                                StdfFile.WriteFloat(0);                 // lo_spec
                                StdfFile.WriteFloat(0);                 // hi_spec
                            }
                            StdfFile.WriteRecord();
                        }
                        else
                        {
                            // Multi functional results

                            //FIXME: not used ?
                            //int nFailingPins;
                            QString strPattern;
                            BYTE	bOptFlag;
                            bOptFlag = BIT0|BIT1|BIT2|BIT4|BIT5|BIT6|BIT7;
                            //FIXME: not used ?
                            //nFailingPins = 0;

                            strPattern = strValue; //strPinName.simplified().section(" ",0,0);
                            if(strPinName != "NOPIN")
                                strTestName += " - " + strPinName.simplified().section(" ",1);

                            // Write FTR
                            RecordReadInfo.iRecordType = 15;
                            RecordReadInfo.iRecordSubType = 20;

                            StdfFile.WriteHeader(&RecordReadInfo);
                            StdfFile.WriteDword(iLastTestNumber);           // Test Number
                            StdfFile.WriteByte(1);                          // Test head
                            StdfFile.WriteByte(iSiteNumber);                // Tester site:1,2,3,4 or 5, etc.
                            if(bTestPass)
                                bData = 0;		// Test passed
                            else
                                bData = BIT7;	// Test Failed
                            StdfFile.WriteByte(bData);                      // TEST_FLG

                            // save the name of the test
                            StdfFile.WriteByte(bOptFlag);       // opt_flg
                            StdfFile.WriteDword(0);             // cycl_cnt
                            StdfFile.WriteDword(0);             // rel_vadr
                            StdfFile.WriteDword(0);             // rept_cnt
                            StdfFile.WriteDword(1);             // num_fail
                            StdfFile.WriteDword(0);             // xfail_ad
                            StdfFile.WriteDword(0);             // yfail_ad
                            StdfFile.WriteWord(0);              // vect_off
                            StdfFile.WriteWord(mapPinResult.count());	// RTN_ICNT
                            StdfFile.WriteWord(0);
                            for(itPinResult=mapPinResult.begin(); itPinResult!=mapPinResult.end(); itPinResult++)
                            {
                                StdfFile.WriteWord(m_qMapPinIndex["[" + QString::number(iLastTestNumber) + "]" + itPinResult.key()]);		// RTN_INDX

                                itPinResult.value().toFloat(&bIsValue);
                                if(bIsValue)
                                {
                                    // Mix definition
                                    // Incorrect parameter line ...this is not a valid file!
                                    m_iLastError = errInvalidFormat;
                                    m_strLastError = "Test type redefinition (mix PTR/FTR) "+strTestName+"["+QString::number(iLastTestNumber)+"]";
                                    // Convertion failed.
                                    // Close file
                                    StdfFile.Close();
                                    return false;
                                }
                            }
                            for(iIndex=0; iIndex!=(mapPinResult.count()+1)/2; iIndex++)
                            {
                                StdfFile.WriteByte(0);          // RTN_STAT
                            }
                            StdfFile.WriteWord(0);
                            StdfFile.WriteString(strPattern.toLatin1().constData());	// vect_name
                            StdfFile.WriteString("");       // time_set
                            StdfFile.WriteString("");       // op_code
                            StdfFile.WriteString(strTestName.toLatin1().constData());	// test_txt: test name
                            StdfFile.WriteString("");       // alarm_id
                            StdfFile.WriteString("");       // prog_txt
                            StdfFile.WriteString("");       // rslt_txt


                            StdfFile.WriteRecord();
                        }

                    }
                    if(!mapTestScale.contains(QString::number(iLastTestNumber) + strTestName))
                        mapTestScale[QString::number(iLastTestNumber) + strTestName] = nScale;

                    mapPinResult.clear();
                    bFunctionalTest = false;
                    bParametricTest = false;
                    iTotalTests++;

                    iLastTestNumber = iTestNumber;

                }

                if(strSection == "HARDBIN")
                {
                    // End for this Part
                    //Bin:  14
                    iBin = strString.section(":",1).remove('"').simplified().toInt();
                    if(!m_qMapHBins.contains(iBin))
                    {
                        CSpeaDatalogBinInfo *pBin = new CSpeaDatalogBinInfo();

                        pBin->nNbCnt = 0;
                        pBin->nPassFlag = TEST_PASSFLAG_FAIL;
                        if(iBin == 1)
                            pBin->nPassFlag = TEST_PASSFLAG_PASS;

                        m_qMapHBins[iBin] = pBin;
                    }
                    m_qMapHBins[iBin]->nNbCnt++;

                    // Read the next line

                    strString = ReadLine(*hSpeaDatalogFile).remove('"');
                    strSection = strString.section(":",0,0).simplified().toUpper();

                    iBin = strString.section(":",1).simplified().toInt();
                    if(!m_qMapSBins.contains(iBin))
                    {
                        CSpeaDatalogBinInfo *pBin = new CSpeaDatalogBinInfo();

                        pBin->nNbCnt = 0;
                        pBin->nPassFlag = TEST_PASSFLAG_FAIL;
                        if(iBin == 1)
                            pBin->nPassFlag = TEST_PASSFLAG_PASS;

                        m_qMapSBins[iBin] = pBin;
                    }
                    m_qMapSBins[iBin]->nNbCnt++;

                    break;
                }
                else if(strSection == "BIN")
                {
                    // End for this Part
                    //Bin:  14
                    iBin = strString.section(":",1).remove('"').simplified().toInt();
                    if(!m_qMapSBins.contains(iBin))
                    {
                        CSpeaDatalogBinInfo *pBin = new CSpeaDatalogBinInfo();

                        pBin->nNbCnt = 0;
                        pBin->nPassFlag = TEST_PASSFLAG_FAIL;
                        if(iBin == 1)
                        {
                            pBin->strName = "Pass";
                            pBin->nPassFlag = TEST_PASSFLAG_PASS;
                        }
                        else if(iBin == 2)
                            pBin->strName = "Open Fail";
                        else if(iBin == 3)
                            pBin->strName = "Short Fail";
                        else if(iBin == 4)
                            pBin->strName = "Leakage Fail";
                        else if(iBin == 5)
                            pBin->strName = "Idd Fail";
                        else if(iBin == 10)
                            pBin->strName = "Memory Fail";
                        else if(iBin == 11)
                            pBin->strName = "Interface Fail";

                        m_qMapSBins[iBin] = pBin;
                        m_qMapHBins[iBin] = pBin;
                    }
                    m_qMapSBins[iBin]->nNbCnt++;
                    break;
                }
                else if(strSection == "TASK")
                {
                    strSequenceName = strString.section(":",1).remove('"').simplified();
                    continue;
                }
                else if(strSection == "TEST TEMPERATURE")
                {
                    continue;
                }

                // Save information from this line

                // Construct the test name
                strTestName = GetTestName(strString);
                if(strTestName.startsWith("%-18"))
                {
                    // No name stored
                    strValue = " ";
                }

                strTestName = strSequenceName + " - " + strTestName;
                strPinName = GetPinName(strString);
                if(strPinName.isEmpty())
                    strPinName = "NOPIN";


                // Save the test result if any
                fValue = GetResult(strString,bIsValue);

                // We can have for the same test number
                // Functional reuslts and Parametric results
                // Ignore all Functional results for this test

                // Save the result of the current PinName
                if(bIsValue)
                {
                    if(bFunctionalTest && !bParametricTest)
                    {
                        // Reset all functional results existing in the mapPinResult
                        mapPinResult.clear();
                    }

                    bParametricTest = true;

                    mapPinResult[strPinName] = QString::number(fValue);

                    if(!mapTestScale.contains(QString::number(iTestNumber) + strTestName + " - " + strPinName))
                    {
                        // Save Low and High limits
                        fLowLimit = GetLowLimit(strString,bHaveLowLimit);
                        fHighLimit = GetHighLimit(strString,bHaveHighLimit);
                        // Save the Unit
                        strUnit = GetUnit(strString);
                        nScale = 0;
                        if(!strUnit.isEmpty())
                            NormalizeLimits(strUnit, nScale);
                    }
                    else
                        nScale = mapTestScale[QString::number(iTestNumber) + strTestName + " - " + strPinName];

                    // Save the Pass/Fail status
                    bTestPass = IsPassFlag(strString);
                }
                else
                {
                    if(bParametricTest)
                    {
                        // stored as parametric result
                        // ignore this line
                        continue;
                    }

                    bFunctionalTest = true;

                    strValue = GetPatternResult(strString);

                    // Save the Pass/Fail result if any
                    if(strValue.startsWith("Failing Pin",Qt::CaseInsensitive))
                    {
                        strValue = strString.section(":",1).simplified();
                        bTestPass = (strValue.toInt(&bIsValue) == 0);
                        if(!bIsValue)
                            bTestPass = false;

                        mapPinResult[strPinName] = strValue;
                    }
                    else if(strValue.endsWith("pass",Qt::CaseInsensitive))
                    {
                        bTestPass = true;
                        strValue = strValue.section(" ",0,0);
                        mapPinResult[strPinName] = strValue;
                    }
                    else if((strValue.endsWith("fail",Qt::CaseInsensitive))
                            ||(strValue.endsWith("fail_f",Qt::CaseInsensitive)))
                    {
                        bTestPass = false;
                        strValue = strValue.section(" ",0,0);
                        mapPinResult[strPinName] = strValue;
                    }
                    else
                    {
                        bTestPass = IsPassFlag(strString);
                        // Pattern
                        strValue = strValue.section(" ",0,0);
                        mapPinResult[strPinName] = strValue;
                    }
                }

                if(bPassStatus)
                    bPassStatus = bTestPass;
            }
            // Write PRR
            RecordReadInfo.iRecordType = 5;
            RecordReadInfo.iRecordSubType = 20;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(1);                  // Test head
            StdfFile.WriteByte(iSiteNumber);        // Tester site:1,2,3,4 or 5
            if(bPassStatus == true)
            {
                StdfFile.WriteByte(0);              // PART_FLG : PASSED
                iTotalGoodBin++;
            }
            else
            {
                StdfFile.WriteByte(8);              // PART_FLG : FAILED
                iTotalFailBin++;
            }
            wSoftBin = wHardBin = iBin;
            StdfFile.WriteWord((WORD)iTotalTests);  // NUM_TEST
            StdfFile.WriteWord(wHardBin);           // HARD_BIN
            StdfFile.WriteWord(wSoftBin);           // SOFT_BIN
            StdfFile.WriteWord(iXWafer);            // X_COORD
            StdfFile.WriteWord(iYWafer);            // Y_COORD
            StdfFile.WriteDword(0);                 // No testing time known...
            StdfFile.WriteString(QString::number(iPartNumber).toLatin1().constData());// PART_ID
            StdfFile.WriteString("");               // PART_TXT
            StdfFile.WriteString("");               // PART_FIX
            StdfFile.WriteRecord();
        }
        else if(strSection == "END DATE")
        {
            QString Date = strString.simplified();
            strString = ReadLine(*hSpeaDatalogFile).remove('"');

            strSection = strString.section(":",0,0).simplified().toUpper();
            strString = strString.section(":",1);
            if(strSection == "END TIME")
            {
                Date += " "+strString.simplified();
                int lTime = GetDateTimeFromString(Date);
                if(lTime > m_lStopTime)
                    m_lStopTime = lTime;
            }
        }
    }

    if(m_lStartTime > m_lStopTime)
        m_lStopTime = 0;

    if(!m_strWaferId.isEmpty())
    {
        // Write WRR for last wafer inserted
        RecordReadInfo.iRecordType = 2;
        RecordReadInfo.iRecordSubType = 20;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);                      // Test head
        StdfFile.WriteByte(255);                    // Tester site (all)
        StdfFile.WriteDword(m_lStopTime);           // Time of last part tested
        StdfFile.WriteDword(iPartNumber);           // Parts tested: always 5
        StdfFile.WriteDword(0);                     // Parts retested
        StdfFile.WriteDword(0);                     // Parts Aborted
        StdfFile.WriteDword(iTotalGoodBin);         // Good Parts
        StdfFile.WriteDword(4294967295UL);          // Functionnal Parts
        StdfFile.WriteString(m_strWaferId.toLatin1().constData());	// WaferID
        StdfFile.WriteRecord();
    }


    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 40;
    QMap<int,CSpeaDatalogBinInfo*>::Iterator itMapBin;
    for ( itMapBin = m_qMapHBins.begin(); itMapBin != m_qMapHBins.end(); ++itMapBin )
    {
        // Write HBR
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(255);                    // Test Head = ALL
        StdfFile.WriteByte(255);                    // Test sites = ALL
        StdfFile.WriteWord(itMapBin.key());	// HBIN = 0
        StdfFile.WriteDword(itMapBin.value()->nNbCnt);	// Total Bins
        if(itMapBin.value()->nPassFlag == TEST_PASSFLAG_PASS)
            StdfFile.WriteByte('P');
        else
            StdfFile.WriteByte('F');
        StdfFile.WriteString(itMapBin.value()->strName.toLatin1().constData());
        StdfFile.WriteRecord();
    }

    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 50;

    for ( itMapBin = m_qMapSBins.begin(); itMapBin != m_qMapSBins.end(); ++itMapBin )
    {
        // Write SBR
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(255);                        // Test Head = ALL
        StdfFile.WriteByte(255);                        // Test sites = ALL
        StdfFile.WriteWord(itMapBin.key());	// HBIN = 0
        StdfFile.WriteDword(itMapBin.value()->nNbCnt);   // Total Bins
        if(itMapBin.value()->nPassFlag == TEST_PASSFLAG_UNKNOWN)
            StdfFile.WriteByte(' ');
        else if(itMapBin.value()->nPassFlag == TEST_PASSFLAG_PASS)
            StdfFile.WriteByte('P');
        else
            StdfFile.WriteByte('F');
        StdfFile.WriteString(itMapBin.value()->strName.toLatin1().constData());
        StdfFile.WriteRecord();
    }

    // Write MRR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteDword(m_lStopTime);           // File finish-time.
    StdfFile.WriteRecord();

    // Close STDF file.
    StdfFile.Close();

    // Success
    return true;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' SpeaDatalog file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGSpeaDatalogtoSTDF::Convert(const char *SpeaDatalogFileName, const char *strFileNameSTDF)
{
    // No erro (default)
    m_iLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(SpeaDatalogFileName);
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
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(SpeaDatalogFileName).fileName()+"...");
            GexScriptStatusLabel->show();
        }
    }
    QCoreApplication::processEvents();

    if(ReadSpeaDatalogFile(SpeaDatalogFileName,strFileNameSTDF) != true)
    {
        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if((GexProgressBar != NULL)
                && bHideProgressAfter)
            GexProgressBar->hide();

        if((GexScriptStatusLabel != NULL)
                && bHideLabelAfter)
            GexScriptStatusLabel->hide();
        return false;	// Error reading SpeaDatalog file
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
//////////////////////////////////////////////////////////////////////
int CGSpeaDatalogtoSTDF::GetTestNum(QString strLine,bool &bIsValue)
{
    //     10 High Level       Clock           -10.000 <      6.270 <     10.000 uA
    //7008   "Vbg7_2_8v"                        "A0"           0.500     > 1.001       < 2.000      V      p
    return strLine.left(7).toInt(&bIsValue);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
QString CGSpeaDatalogtoSTDF::GetTestName(QString strLine)
{
    //    10 High Level       Clock           -10.000 <      6.270 <     10.000 uA
    //7008   "Vbg7_2_8v"                        "A0"           0.500     > 1.001       < 2.000      V      p
    if(m_bQuoteFormat)
    {
        int nStartPos = strLine.indexOf('"');
        int nEndPos = strLine.indexOf('"',nStartPos+1);
        if(nStartPos < m_iPinNamePos)
            return strLine.mid(nStartPos,nEndPos-nStartPos).remove('"').simplified();
    }

    if(strLine.contains("Failing Pin",Qt::CaseInsensitive))
        return strLine.mid(m_iTestNamePos,(strLine.indexOf("Failing Pin",0,Qt::CaseInsensitive)-m_iTestNamePos)).simplified();

    return strLine.mid(m_iTestNamePos,m_iPinNamePos-m_iTestNamePos).simplified();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
QString CGSpeaDatalogtoSTDF::GetPinName(QString strLine)
{
    //    10 High Level       Clock           -10.000 <      6.270 <     10.000 uA
    //7008   "Vbg7_2_8v"     "A0"           0.500     > 1.001       < 2.000      V      p
    //100    "FUNCMIN"       "SCLB"         "functhrs"                         18       f
    //101    "FUNCNOM"                      "functhrs"                                  p

    if(strLine.contains("Failing Pin",Qt::CaseInsensitive))
        return "";

    return strLine.mid(m_iPinNamePos,m_iLowLimitPos-m_iPinNamePos).remove('"').simplified();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
QString CGSpeaDatalogtoSTDF::GetPatternResult(QString strLine)
{
    //    10 High Level       Clock           -10.000 <      6.270 <     10.000 uA
    //7008   "Vbg7_2_8v"                        "A0"           0.500     > 1.001       < 2.000      V      p
    //30001  "Initial Func"                                    "100Khz"                                    p
    if(strLine.contains("Failing Pin",Qt::CaseInsensitive))
        return strLine.mid(strLine.indexOf("Failing Pin",0,Qt::CaseInsensitive)).simplified();

    return strLine.mid(m_iLowLimitPos).remove('"').trimmed();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
float CGSpeaDatalogtoSTDF::GetLowLimit(QString strLine,bool &bIsValue)
{
    //    10 High Level       Clock           -10.000 <      6.270 <     10.000 uA
    //7008   "Vbg7_2_8v"                        "A0"           0.500     > 1.001       < 2.000      V      p
    //30001  "Initial Func"                                    "100Khz"                                    p
    if(m_bQuoteFormat && (strLine.lastIndexOf('"')>m_iLowLimitPos))
    {
        // Functional parameter
        bIsValue = false;
        return 0.0;
    }
    return strLine.mid(m_iLowLimitPos,m_iResultPos-m_iLowLimitPos).remove('<').remove('>').toFloat(&bIsValue);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
float CGSpeaDatalogtoSTDF::GetResult(QString strLine,bool &bIsValue)
{
    //    10 High Level       Clock           -10.000 <      6.270 <     10.000 uA
    //7008   "Vbg7_2_8v"                        "A0"           0.500     > 1.001       < 2.000      V      p
    //30001  "Initial Func"                                    "100Khz"                                    p
    if(m_bQuoteFormat && (strLine.lastIndexOf('"')>m_iLowLimitPos))
    {
        // Functional parameter
        bIsValue = false;
        return 0.0;
    }
    return strLine.mid(m_iResultPos,m_iHighLimitPos-m_iResultPos).remove('<').remove('>').toFloat(&bIsValue);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
float CGSpeaDatalogtoSTDF::GetHighLimit(QString strLine,bool &bIsValue)
{
    //    10 High Level       Clock           -10.000 <      6.270 <     10.000 uA
    //7008   "Vbg7_2_8v"                        "A0"           0.500     > 1.001       < 2.000      V      p
    //30001  "Initial Func"                                    "100Khz"                                    p
    if(m_bQuoteFormat && (strLine.lastIndexOf('"')>m_iLowLimitPos))
    {
        // Functional parameter
        bIsValue = false;
        return 0.0;
    }
    return strLine.mid(m_iHighLimitPos,m_iUnitPos-m_iHighLimitPos).remove('<').remove('>').toFloat(&bIsValue);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
QString CGSpeaDatalogtoSTDF::GetUnit(QString strLine)
{
    //    10 High Level       Clock           -10.000 <      6.270 <     10.000 uA
    //7008   "Vbg7_2_8v"                        "A0"           0.500     > 1.001       < 2.000      V      p
    //30001  "Initial Func"                                    "100Khz"                                    p
    if(m_bQuoteFormat && (strLine.lastIndexOf('"')>m_iLowLimitPos))
    {
        // Functional parameter
        return "";
    }
    return strLine.mid(m_iUnitPos,m_iFlagPos-m_iUnitPos-1).simplified();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
bool CGSpeaDatalogtoSTDF::IsPassFlag(QString strLine)
{
    //    10 High Level       Clock           -10.000 <      6.270 <     10.000 uA
    //7008   "Vbg7_2_8v"                        "A0"           0.500     > 1.001       < 2.000      V      p
    //30001  "Initial Func"                                    "100Khz"                                    p
    // 'p' can be omitted
    // 'f' always if fail
    QString strValue = strLine.simplified().section(" ",-1);
    return strValue.toUpper() != "F";
}

//////////////////////////////////////////////////////////////////////
//return lDateTime from string strDateTime
// Wed Oct 08 2008 21:36:55
// 23/01/1998 14:03:21
// 21.07.2009 14:03:21
//////////////////////////////////////////////////////////////////////
long CGSpeaDatalogtoSTDF::GetDateTimeFromString(QString strDateTime)
{
    long lDateTime;

    if(strDateTime.length()<18)
        return 0;

    QString strDate;
    QDate clDate;
    QTime clTime;

    if(strDateTime.contains("/") || strDateTime.contains("."))
    {
        // dd/mm/yyyy
        // 23/01/1998 14:03:21
        QString strTime;
        strDate = strDateTime.left(10);
        strTime = strDateTime.section(" ",1);

        clDate = QDate::fromString(strDate,"dd/mm/yyyy");
        if(!clDate.isValid())
        {
            int nYear, nMonth, nDay;
            nDay = strDate.mid(0,2).toInt();
            nMonth = strDate.mid(3,2).toInt();
            nYear = strDate.mid(6,4).toInt();
            clDate = QDate(nYear,nMonth,nDay);
        }

        clTime = QTime::fromString(strTime);
    }
    else
    {
        // Wed Oct 08 2008 21:47:50
        // Di Jun 16 2009 05:52:09
        // Remove french word for the day
        strDate = "Wed " + strDateTime.section(" ",1).simplified();

        clDate = QDate::fromString(strDate.section(" ",0,3));
        clTime = QTime::fromString(strDate.section(" ",4));
    }

    if((m_lStartTime>0)
            && !clDate.isValid()
            && clTime.isValid())
    {
        // cannot retrieve date but have a time and a start_t
        if(QDateTime::fromTime_t(m_lStartTime).time() <= clTime)
            clDate = QDateTime::fromTime_t(m_lStartTime).date();
    }

    QDateTime clDateTime(clDate,clTime);

    clDateTime.setTimeSpec(Qt::UTC);
    lDateTime = clDateTime.toTime_t();
    return lDateTime;
}


//////////////////////////////////////////////////////////////////////
// Read line : skip empty line
//////////////////////////////////////////////////////////////////////
QString CGSpeaDatalogtoSTDF::ReadLine(QTextStream& hFile)
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
    while(!strString.isNull() && (strString.simplified().isEmpty() || strString.simplified().startsWith("#") || strString.simplified().startsWith("==")));


    return strString;

}
