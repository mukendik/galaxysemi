//////////////////////////////////////////////////////////////////////
/// import_teradyne_ascii.cpp: Convert a Teradyne Ascii file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#include "gqtl_global.h"
#include <math.h>
#include <qmath.h>
#include <time.h>

#ifdef _WIN32
#include "windows.h"		// For 'GetWindowsDirectoryA' declaration
#endif

#include <qfileinfo.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>
#include <QPair>

#include "import_teradyne_ascii.h"
#include "import_constants.h"
#include "engine.h"
#include "gex_algorithms.h"

// File format: Data log
//Datalog report
//03/22/2014 04:53:47
//      Prog Name:    Yosemite_v1.62.xls
//       Job Name:    Yosemite
//            Lot:    CDS641DCC02
//       Operator:    264849
//      Test Mode:    Production
//      Node Name:    J750-17
//      Part Type:    PCA9618
//    Channel map:    X1_HDVISProbeChannmap
//    Environment:    Room
//     FacilityID:    AMKOR_KOREA
//       FamilyID:    PC-IP
//        FloorID:    ATKI
//        PkgType:    000
//      ProcessID:    ICN8
//         SubLot:    CD
//       TestCode:    wr1

//    Site Number:
//         0
//         CardID:    1
//       LoadType:    Yosemite_v1.62.txt

//    Device#: 1


// Number     Site  Result   Test Name                 Pin    Channel   Low            Measured       High           Force          Loc
// 100000      0    PASS     Continuity_Opens          VCC    12        -500.0000 mV   -493.1250 mV   N/A            -100.0000 uA   0
// 100001      0    PASS     Continuity_Opens          DataA  42        -900.0000 mV   -503.2832 mV   N/A            -100.0000 uA   0
// 100002      0    PASS     Continuity_Opens          DataB  44        -900.0000 mV   -497.0594 mV   N/A            -100.0000 uA   0
// 105000      0    PASS     Continuity_Shorts         VCC    12        N/A            -493.1250 mV   -125.0000 mV   -100.0000 uA   0
// 105001      0    PASS     Continuity_Shorts         DataA  42        N/A            -503.2832 mV   -125.0000 mV   -100.0000 uA   0
// 105002      0    PASS     Continuity_Shorts         DataB  44        N/A            -497.0594 mV   -125.0000 mV   -100.0000 uA   0
// 110000      0    PASS     Vik_Test                  DataA  42        -1200.0000 mV  -890.5035 mV   -400.0000 mV   -18.0000 mA    0
// 110001      0    PASS     Vik_Test                  DataB  44        -1200.0000 mV  -895.7891 mV   -400.0000 mV   -18.0000 mA    0
// 120000      0    PASS     Lkg_VCC=0V_Test           DataA  42        -1.0000 uA     -0.0253 uA     1.0000 uA      200.0000 mV    0

// Number     Site  Result   Test Name                 Pattern
// 170000      0    PASS     Functional_Test           Functional_Input_Output
// 170001      0    PASS     Functional_Test           Functional_Output_Input
// NOTE:       Functional Test at Low Levels,          VDD = 2.2 Volts
// 171000      0    PASS     Functional_Test           Functional_Input_Output
// 171001      0    PASS     Functional_Test           Functional_Output_Input
// NOTE:       Functional Test at Nominal Levels,      VDD = 3.0 Volts
// 172000      0    PASS     Functional_Test           Functional_Input_Output
// 172001      0    PASS     Functional_Test           Functional_Output_Input
// NOTE:       Functional Test at High Levels,         VDD = 5.5 Volts

// Site Failed tests/Executed tests
//------------------------------------
//    0         0        51

// Site    Sort     Bin
//------------------------------------
//    0         1         1
//=========================================================================

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
CGTERADYNE_ASCIItoSTDF::CGTERADYNE_ASCIItoSTDF()
{
    mStartTime = 0;
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGTERADYNE_ASCIItoSTDF::GetLastError()
{
    mStrLastError = "Import TERADYNE_ASCII: ";

    switch(mIdLastError)
    {
        default:
        case errNoError:
            mStrLastError += "No Error";
            break;
        case errOpenFail:
            mStrLastError += "Failed to open file";
            break;
        case errInvalidFormat:
            mStrLastError += "Invalid file format";
            break;
        case errWriteSTDF:
            mStrLastError += "Failed creating temporary file. Folder permission issue?";
            break;
        case errLicenceExpired:
            mStrLastError += "License has expired or Data file out of date...";
            break;
    }
    // Return Error Message
    return mStrLastError;
}

//////////////////////////////////////////////////////////////////////
// Normalize test limits when writing into PTR.
//////////////////////////////////////////////////////////////////////
void CGTERADYNE_ASCIItoSTDF::NormalizeValues(QString &lStrUnits,float &lValue, int &lScale, bool &lIsNumber)
{
    int lLenth;
    QString lStrValue = lStrUnits;

    // In strValue, the current value with unit
    if(lStrValue.startsWith("-"))
        lLenth = 6;
    else
        lLenth = 5;
    if(lStrValue.length() <= lLenth)
    {
        // no unit
        lStrUnits = "";
    }
    else
    {
        lStrUnits = lStrValue.right(lStrValue.length()-lLenth);
        lStrValue = lStrValue.left(lLenth);
    }

    lValue = lStrValue.toFloat(&lIsNumber);
    lScale=0;

    if(lStrUnits.length() <= 1)
    {
        // units too short to include a prefix, then keep it 'as-is'
        return;
    }

    QChar cPrefix = lStrUnits[0];
    switch(cPrefix.toLatin1())
    {
        case 'm': // Milli
            lScale = -3;
            break;
        case 'u': // Micro
            lScale = -6;
            break;
        case 'n': // Nano
            lScale = -9;
            break;
        case 'p': // Pico
            lScale = -12;
            break;
        case 'f': // Fento
            lScale = -15;
            break;
        case 'K': // Kilo
            lScale = 3;
            break;
        case 'M': // Mega
            lScale = 6;
            break;
        case 'G': // Giga
            lScale = 9;
            break;
        case 'T': // Tera
            lScale = 12;
            break;
        default :
            lScale = 0;
    }
    lValue *= GS_POW(10.0,lScale);
    if(lScale)
        lStrUnits = lStrUnits.mid(1);	// Take all characters after the prefix.
}

//////////////////////////////////////////////////////////////////////
// Check if File is compatible with TERADYNE_ASCII format
//////////////////////////////////////////////////////////////////////
bool CGTERADYNE_ASCIItoSTDF::IsCompatible(const char *lFileName)
{
    QString lString;
    bool	lIsCompatible(false);

    // Open lTeradyneAsciiFile file
    QFile lFile( lFileName );
    if(!lFile.open( QIODevice::ReadOnly ))
    {
        // Failed Opening TeradyneAscii file
        return false;
    }
    // Assign file I/O stream
    QTextStream lTeradyneAsciiFile(&lFile);

    // Check if first line is the correct TERADYNE_ASCII header...
    //Datalog report
    //03/22/2014 04:53:47
    //      Prog Name:    Yosemite_v1.62.xls
    //       Job Name:    Yosemite
    //            Lot:    CDS641DCC02
    //       Operator:    264849
    //      Test Mode:    Production
    //      Node Name:    J750-17
    //      Part Type:    PCA9618

    do
        lString = lTeradyneAsciiFile.readLine();
    while(!lString.isNull() && lString.isEmpty());

    // Close file
    lFile.close();

    if(lString.startsWith("Datalog report", Qt::CaseInsensitive))
        lIsCompatible = true;

    lString = lTeradyneAsciiFile.readLine();
    lString = lTeradyneAsciiFile.readLine();
    if (lString.contains("Prog Name"))
        lIsCompatible = true;

    return lIsCompatible;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the TERADYNE_ASCII file
//////////////////////////////////////////////////////////////////////
bool CGTERADYNE_ASCIItoSTDF::ReadTeradyneAsciiFile(const char *lTeradyneAsciiFileName,
                                                   const char *lFileNameSTDF)
{
    QString lString;
    QString lDate;

    // Open TERADYNE_ASCII file
    QFile lFile( lTeradyneAsciiFileName );
    if(!lFile.open( QIODevice::ReadOnly ))
    {
        // Failed Opening TERADYNE_ASCII file
        mIdLastError = errOpenFail;

        // Convertion failed.
        return false;
    }

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    mFileSize = lFile.size() + 1;

    // Assign file I/O stream
    QTextStream lTeradyneAsciiFile(&lFile);

    // Check if first line is the correct TERADYNE_ASCII header...
    // Datalog report
    // 03/22/2014 04:53:47
    //  Prog Name:    Yosemite_v1.62.xls
    mIdLastError = errInvalidFormat;
    // Read first non-empty line
    lString = ReadLine(lTeradyneAsciiFile);
    if(lString.isNull())
    {
        // Close file
        lFile.close();
        return false;
    }

    // Incorrect header...this is not a TERADYNE_ASCII file!
    // Convertion failed.
    if(!lString.startsWith("Datalog report", Qt::CaseInsensitive))
    {
        // Close file
        lFile.close();
        return false;
    }

    lDate = ReadLine(lTeradyneAsciiFile);
    lString = ReadLine(lTeradyneAsciiFile);
    if (!lString.contains("Prog Name"))
    {
        // Close file
        lFile.close();
        return false;
    }

    // 03/22/2014 04:53:47
    QDateTime clDateTime;
    clDateTime.setTimeSpec(Qt::UTC);
    mStartTime = clDateTime.fromString(lDate, "MM/dd/yyyy HH:mm:ss").toTime_t();
    if(mStartTime <= 0)
        mStartTime = QDateTime::currentDateTime().toTime_t();
    mStdfMIRv4.SetSTART_T(mStartTime);
    mStdfMIRv4.SetSETUP_T(mStartTime);



    while (!lString.contains("Site Number"))
    {
        // Prog Name:    Yosemite_v1.62.xls
        if (lString.contains("Prog Name"))
        {
            lString = lString.section(":", 1).trimmed();
            mStdfMIRv4.SetJOB_REV(lString);
        }

        // Job Name:    Yosemite
        else if (lString.contains("Job Name"))
        {
            lString = lString.section(":", 1).trimmed();
            mStdfMIRv4.SetJOB_NAM(lString);
        }

        // Lot:    CDS641DCC02
        else if (lString.contains("Lot"))
        {
            lString = lString.section(":", 1).trimmed();
            mStdfMIRv4.SetLOT_ID(lString);
        }

        // Operator:    264849
        else if (lString.contains("Operator"))
        {
            lString = lString.section(":", 1).trimmed();
            mStdfMIRv4.SetOPER_NAM(lString);
        }

        // Test Mode:    Production
        else if (lString.contains("Test Mode"))
        {
            lString = lString.section(":", 1).trimmed();
            if (lString.contains("Production"))
                mStdfMIRv4.SetMODE_COD('P');
            else  if (lString.contains("Maintenance"))
                mStdfMIRv4.SetMODE_COD('M');
            else  if (lString.contains("Engineering"))
                mStdfMIRv4.SetMODE_COD('E');
            else  if (lString.contains("Development"))
                mStdfMIRv4.SetMODE_COD('D');
            else  if (lString.contains("Checker"))
                mStdfMIRv4.SetMODE_COD('C');
            else  if (lString.contains("AEL"))
                mStdfMIRv4.SetMODE_COD('A');
            else  if (lString.contains("Quality"))
                mStdfMIRv4.SetMODE_COD('Q');
            else
                mStdfMIRv4.SetMODE_COD(' ');
        }

        // Node Name:    J750-17
        else if (lString.contains("Node Name"))
        {
            lString = lString.section(":", 1).trimmed();
            mStdfMIRv4.SetNODE_NAM(lString);
        }

        // Part Type:    PCA9618
        else if (lString.contains("Part Type"))
        {
            lString = lString.section(":", 1).trimmed();
            mStdfMIRv4.SetPART_TYP(lString);
        }

        // Channel map:    X1_HDVISProbeChannmap
        // Do nothing

        // Environment: Room
        else if (lString.contains("Environment:"))
        {
            lString = lString.section(":", 1).trimmed();
            mStdfMIRv4.SetTST_TEMP(lString);
        }

        // FacilityID:    AMKOR_KOREA
        else if (lString.contains("FacilityID"))
        {
            lString = lString.section(":", 1).trimmed();
            mStdfMIRv4.SetFACIL_ID(lString);
        }

        // FamilyID:    PC-IP
        else if (lString.contains("FamilyID"))
        {
            lString = lString.section(":", 1).trimmed();
            mStdfMIRv4.SetFAMLY_ID(lString);
        }

        // FloorID:    ATKI
        else if (lString.contains("FloorID"))
        {
            lString = lString.section(":", 1).trimmed();
            mStdfMIRv4.SetFLOOR_ID(lString);
        }

        // PkgType:    000
        else if (lString.contains("PkgType"))
        {
            lString = lString.section(":", 1).trimmed();
            mStdfMIRv4.SetPKG_TYP(lString);
        }

        // ProcessID:    ICN8
        else if (lString.contains("ProcessID"))
        {
            lString = lString.section(":", 1).trimmed();
            mStdfMIRv4.SetPROC_ID(lString);
        }

        // SubLot:    CD
        else if (lString.contains("SubLot"))
        {
            lString = lString.section(":", 1).trimmed();
            mStdfMIRv4.SetSBLOT_ID(lString);
        }

        // TestCode:    wr1
        else if (lString.contains("TestCode"))
        {
            lString = lString.section(":", 1).trimmed();
            mStdfMIRv4.SetTEST_COD(lString);
        }
        lString = ReadLine(lTeradyneAsciiFile).trimmed();
    }
    // Construct custom Galaxy USER_TXT
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    strUserTxt += ":TERADYNE_ASCII";
    mStdfMIRv4.SetSTAT_NUM(0);
    mStdfMIRv4.SetUSER_TXT(strUserTxt);
    mStdfMIRv4.SetRTST_COD(' ');
    mStdfMIRv4.SetPROT_COD(' ');
    mStdfMIRv4.SetBURN_TIM(65535);
    mStdfMIRv4.SetCMOD_COD(' ');
    mStdfMIRv4.SetTSTR_TYP("");


    // Fill the SDR
    lString = ReadLine(lTeradyneAsciiFile).trimmed();
    QStringList lSiteList = lString.split(",");
    mStdfSDRv4.SetSITE_CNT(lSiteList.size());
    for(int lSite=0; lSite<lSiteList.size(); ++lSite)
        mStdfSDRv4.m_ku1SITE_NUM[lSite] = (lSiteList[lSite].trimmed()).toInt();
    mStdfSDRv4.SetSITE_NUM();
    while (!lString.contains("Device#"))
    {
        // CardID:    1
        if (lString.contains("CardID"))
        {
            lString = lString.section(":", 1).trimmed();
            mStdfSDRv4.SetCARD_ID(lString);
        }

        // LoadType:    Yosemite_v1.62.txt
        if (lString.contains("LoadType"))
        {
            lString = lString.section(":", 1).trimmed();
            mStdfSDRv4.SetLOAD_TYP(lString);
        }

        lString = ReadLine(lTeradyneAsciiFile).trimmed();
    }
    mStdfSDRv4.SetHEAD_NUM(1);
    mStdfSDRv4.SetSITE_GRP(1);

    // Convertion failed.
    if(lTeradyneAsciiFile.atEnd())
    {
        // Close file
        lFile.close();
        return false;
    }

    // Fill the list offset of the PTR and FTR fields
    // read PTR :
    //Number   Site    Result   Test Name     Pin    Channel   Low    Measured    High    Force    Loc
    while (!lString.startsWith("Number", Qt::CaseInsensitive)
           || !lString.contains("Measured", Qt::CaseInsensitive))
        lString = ReadLine(lTeradyneAsciiFile).trimmed();

    mPtrFields.mNumber = 0;
    mPtrFields.mSite = lString.indexOf("Site");
    mPtrFields.mResult = lString.indexOf("Result");
    mPtrFields.mTestName = lString.indexOf("Test Name");
    mPtrFields.mPin = lString.indexOf("Pin");
    mPtrFields.mChannel = lString.indexOf("Channel");
    mPtrFields.mLow = lString.indexOf("Low");
    mPtrFields.mMeasured = lString.indexOf("Measured");
    mPtrFields.mHigh = lString.indexOf("High");
    mPtrFields.mForce = lString.indexOf("Force");
    mPtrFields.mLoc = lString.indexOf("Loc");

    // Some files don't contain the word Pattern
    mPtrFields.mPattern = mPtrFields.mPin;

    //Restart at the beggining of the file
    lTeradyneAsciiFile.seek(0);

    if(WriteStdfFile(lTeradyneAsciiFile,lFileNameSTDF) != true)
    {
        QFile::remove(lFileNameSTDF);
        // Close file
        lFile.close();
        return false;
    }

    // Success parsing TERADYNE_ASCII file
    mIdLastError = errNoError;
    return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from TERADYNE_ASCII data parsed
//////////////////////////////////////////////////////////////////////
bool CGTERADYNE_ASCIItoSTDF::WriteStdfFile(QTextStream &lTeradyneAsciiFile, const char *lFileNameSTDF)
{
    // now generate the STDF file...
    GS::StdLib::Stdf lStdfFile;
    if(lStdfFile.Open((char*)lFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        // Failed importing TERADYNE_ASCII file into STDF database
        mIdLastError = errWriteSTDF;

        // Convertion failed.
        return false;
    }

    // Write FAR
    mStdfFARv4.SetCPU_TYPE(1);  // SUN CPU type
    mStdfFARv4.SetSTDF_VER(4);  // STDF V4
    mStdfFARv4.Write(lStdfFile);

    if(mStartTime <= 0)
        mStartTime = QDateTime::currentDateTime().toTime_t();

    // Write MIR
    mStdfMIRv4.Write(lStdfFile);

    // Write SDR
    mStdfSDRv4.Write(lStdfFile);

    QString lPartId = "";
    QStringList lStringList;


    QString lStrString;
    QList<stdf_type_u1> lSiteList;
    QStringList lPartIdList;

    // Read until first Device
    while (!lStrString.contains("Device#"))
    {
        lStrString = ReadLine(lTeradyneAsciiFile).trimmed();
    }

    // Write Test results for each PartID
    while(!lTeradyneAsciiFile.atEnd())
    {
        if(lStrString.isEmpty())
            lStrString = ReadLine(lTeradyneAsciiFile).trimmed();

        // Read the part_id
        if (lStrString.startsWith("Device#"))
        {
            lPartId = lStrString.section(":", 1).trimmed();
            lPartIdList = lPartId.split(",");
        }

        // Read the PRR' informations
        if (lStrString.startsWith("Site") && lStrString.contains("tests/Executed") )
        {
            // Clear the list of site id
            lSiteList.clear();

            QList< QPair<int, int> > lSiteExecution;  // Map of number of the site and the Executed tests
            // The next line contains only  ---
            lStrString = ReadLine(lTeradyneAsciiFile).trimmed();
            if (lStrString.startsWith("---"))
                lStrString = ReadLine(lTeradyneAsciiFile).trimmed();
            lStringList.clear();
            if (!lTeradyneAsciiFile.atEnd())
                lStringList = lStrString.split(" ", QString::SkipEmptyParts);
            bool ok;
            lStringList[0].toInt(&ok);
            if (ok)
            {
                QPair<int,int> lElt(lStringList[0].toInt(), lStringList[2].toInt());
                lSiteExecution.append(lElt);
                // We have to find a new line containning the soft bin and hard bin
                while (!lStrString.startsWith("Site") && !lStrString.isEmpty())
                {
                    lStrString = ReadLine(lTeradyneAsciiFile).trimmed();
                    if (!lTeradyneAsciiFile.atEnd())
                        lStringList = lStrString.split(" ", QString::SkipEmptyParts);
                    bool lInt;
                    lStringList[0].toInt(&lInt);
                    if (lInt)
                    {
                        QPair<int,int> lElt(lStringList[0].toInt(), lStringList[2].toInt());
                        lSiteExecution.append(lElt);
                    }
                }
                if (!lStrString.contains("Sort"))
                    return false;
                lStrString = ReadLine(lTeradyneAsciiFile).trimmed();
                if (lStrString.startsWith("---"))
                    lStrString = ReadLine(lTeradyneAsciiFile).trimmed();
                for (int lIndex=0; lIndex<lSiteExecution.size(); ++lIndex)
                {
                    lStringList.clear();
                    if (!lTeradyneAsciiFile.atEnd())
                        lStringList = lStrString.split(" ", QString::SkipEmptyParts);
                    bool ok;
                    lStringList[0].toInt(&ok);
                    if (!ok)
                        return false;

                    mStdfPRRv4.SetHEAD_NUM(1);
                    mStdfPRRv4.SetSITE_NUM((lSiteExecution[lIndex]).first);
                    char	bit_field=0;
                    if (lStringList[1].toInt() != 1 || lStringList[2].toInt() != 1)
                        bit_field |= BIT3;
                    mStdfPRRv4.SetPART_FLG(bit_field);
                    mStdfPRRv4.SetNUM_TEST(lSiteExecution[lIndex].second);
                    mStdfPRRv4.SetHARD_BIN(lStringList[1].toInt());
                    mStdfPRRv4.SetSOFT_BIN(lStringList[2].toInt());
                    mStdfPRRv4.SetPART_ID(lPartIdList[lIndex]);
                    mStdfPRRv4.Write(lStdfFile);
                    lStrString = ReadLine(lTeradyneAsciiFile).trimmed();
                }
            }
        }



        // Read and write PTR : 2 methods to find a PTR
        // Number     Site  Result   Test Name  Pin     Channel     Low     Measured       High   Force    Loc
        // Characterization setup 'Char_Vil_Low_IO', row 1, Measure: VDriveLo
        if((lStrString.startsWith("Number", Qt::CaseInsensitive)
                && lStrString.contains("Measured", Qt::CaseInsensitive))
            || lStrString.startsWith("Characterization setup"))
        {
            // Read until empty line
            bool lEndTable(false);
            lStrString = ReadLine(lTeradyneAsciiFile).trimmed();
            if (lStrString.startsWith("Site") && lStrString.contains("tests/Executed") )
               lEndTable = true;
            while (!lEndTable
                   && !lStrString.isEmpty()
                   && !lTeradyneAsciiFile.atEnd())
            {

                WritePTR(lStrString, lStdfFile, lSiteList);

                // check if the next line is a test result
                lStrString = ReadLine(lTeradyneAsciiFile).trimmed();
                if (lStrString.startsWith("Site") && lStrString.contains("tests/Executed") )
                   lEndTable = true;
                else
                {
                    QStringList lItems = lStrString.simplified().split(" ");
                    if (lItems.size() <= 1)
                        lEndTable = true;
                    else
                    {
                        bool ok;
                        lItems[0].toInt(&ok);
                        if (!ok)
                           lEndTable = true;
                    }
                }
            }
        }


        // Read and write FTR : 2 methods to find a FTR
        // Number     Site  Result   Test Name                 Pattern
        //    NOTE:       Functional Test at Low Levels,          VDD = 2.2 Volts
        if((lStrString.startsWith("Number", Qt::CaseInsensitive)
                && (lStrString.contains("Pattern", Qt::CaseInsensitive)
                   || (lStrString.size() < mPtrFields.mMeasured))))
        {
            QList<FTRFields> lFTRList;
            FTRFields lFTRElement;
            // Read until empty line
            bool lEndTable(false);
            lStrString = ReadLine(lTeradyneAsciiFile).trimmed();
            if (lStrString.startsWith("Site") && lStrString.contains("tests/Executed") )
               lEndTable = true;
            while (!lEndTable
                   && !lStrString.isEmpty()
                   && !lTeradyneAsciiFile.atEnd())
            {
                if (lStrString.startsWith("NOTE:", Qt::CaseInsensitive))
                {
                    QStringList lNoteItems = lStrString.simplified().split(",");
                    for (int i=0; i<lFTRList.size(); ++i)
                    {
                        lFTRElement = lFTRList[i];
                        if (lNoteItems.size() > 1)
                        {
                            lFTRElement.mTEST_TXT += " " + lNoteItems[1].trimmed();
                        }
                        mStdfFTRv4.SetTEST_NUM(lFTRElement.mTEST_NUM);
                        mStdfFTRv4.SetHEAD_NUM(1);
                        mStdfFTRv4.SetSITE_NUM(lFTRElement.mHEAD_NUM);
                        mStdfFTRv4.SetTEST_FLG(lFTRElement.mTEST_FLG);
                        mStdfFTRv4.SetTEST_TXT(lFTRElement.mTEST_TXT);
                        mStdfFTRv4.SetVECT_NAM(lFTRElement.mVECT_NAM);
                        mStdfFTRv4.Write(lStdfFile);
                    };
                    lFTRList.clear();
                }
                else
                {
                    lFTRElement.mTEST_NUM =
                            (lStrString.mid(mPtrFields.mNumber, mPtrFields.mSite - mPtrFields.mNumber).trimmed().toULong());
                    lFTRElement.mHEAD_NUM =
                            (lStrString.mid(mPtrFields.mSite, mPtrFields.mResult - mPtrFields.mSite).trimmed().toUShort());
                    QString lPassFailString =
                            lStrString.mid(mPtrFields.mResult, mPtrFields.mTestName - mPtrFields.mResult).trimmed();
                    stdf_type_b1 lPassFail = 0;
                    if (lPassFailString.isEmpty())
                        lPassFail |= 0x40;
                    else if (!lPassFailString.contains("PASS", Qt::CaseInsensitive))
                        lPassFail |= 0x80;
                    lFTRElement.mTEST_FLG = lPassFail;
                    if (mPtrFields.mPattern != 0)
                         lFTRElement.mTEST_TXT =
                                 lStrString.mid(mPtrFields.mTestName, mPtrFields.mPattern - mPtrFields.mTestName).trimmed();
                    else
                        lFTRElement.mTEST_TXT = lStrString.mid(mPtrFields.mTestName).trimmed();

                    if (mPtrFields.mPattern != 0)
                        lFTRElement.mVECT_NAM = lStrString.mid(mPtrFields.mPattern).trimmed();
                    else
                        lFTRElement.mVECT_NAM = "";
                    lFTRList.append(lFTRElement);
                }
                // check if the next line is a test result
                lStrString = ReadLine(lTeradyneAsciiFile).trimmed();
                QStringList lItems = lStrString.simplified().split(" ");
                if (!lStrString.startsWith("NOTE:", Qt::CaseInsensitive))
                {
                    if((lItems.size() <= 1) || (lItems.size() > 5))
                        lEndTable = true;
                    else
                    {
                        bool ok;
                        lItems[0].toInt(&ok);
                        if (!ok)
                            lEndTable = true;
                    }
                }
            }

            // If we don't have the bloc NOTE after the table
            if (!lFTRList.isEmpty())
            {
                for (int i=0; i<lFTRList.size(); ++i)
                {
                    lFTRElement = lFTRList[i];
                    mStdfFTRv4.SetTEST_NUM(lFTRElement.mTEST_NUM);
                    mStdfFTRv4.SetHEAD_NUM(1);
                    mStdfFTRv4.SetSITE_NUM(lFTRElement.mHEAD_NUM);
                    mStdfFTRv4.SetTEST_FLG(lFTRElement.mTEST_FLG);
                    mStdfFTRv4.SetTEST_TXT(lFTRElement.mTEST_TXT);
                    mStdfFTRv4.SetVECT_NAM(lFTRElement.mVECT_NAM);
                    mStdfFTRv4.Write(lStdfFile);
                };
                lFTRList.clear();
            }
        }

        // Read a FAR which begin with NOTE:
        if (lStrString.startsWith("NOTE:"))
        {
            FTRFields lFTRElement;
            QString lNameComplement = "";
            bool lEndTable(false);
            while (!lEndTable
                   && !lStrString.isEmpty()
                   && !lTeradyneAsciiFile.atEnd())
            {
                if (lStrString.startsWith("NOTE:", Qt::CaseInsensitive))
                {
                    QStringList lNoteItems = lStrString.simplified().split(",");
                    lNameComplement = lNoteItems[1].trimmed();
                }
                else
                {
                    lFTRElement.mTEST_NUM =
                            (lStrString.mid(mPtrFields.mNumber, mPtrFields.mSite - mPtrFields.mNumber).trimmed().toULong());
                    lFTRElement.mHEAD_NUM =
                            (lStrString.mid(mPtrFields.mSite, mPtrFields.mResult - mPtrFields.mSite).trimmed().toUShort());
                    QString lPassFailString =
                            lStrString.mid(mPtrFields.mResult, mPtrFields.mTestName - mPtrFields.mResult).trimmed();
                    stdf_type_b1 lPassFail = 0;
                    if (lPassFailString.isEmpty())
                        lPassFail |= 0x40;
                    else if (!lPassFailString.contains("PASS", Qt::CaseInsensitive))
                        lPassFail |= 0x80;
                    lFTRElement.mTEST_FLG = lPassFail;
                    if (mPtrFields.mPattern != 0)
                         lFTRElement.mTEST_TXT =
                                 lStrString.mid(mPtrFields.mTestName, mPtrFields.mPattern - mPtrFields.mTestName).trimmed();
                    else
                        lFTRElement.mTEST_TXT = lStrString.mid(mPtrFields.mTestName).trimmed();

                    if (mPtrFields.mPattern != 0)
                        lFTRElement.mVECT_NAM = lStrString.mid(mPtrFields.mPattern).trimmed();
                    else
                        lFTRElement.mVECT_NAM = "";
                    mStdfFTRv4.SetTEST_NUM(lFTRElement.mTEST_NUM);
                    mStdfFTRv4.SetHEAD_NUM(1);
                    mStdfFTRv4.SetSITE_NUM(lFTRElement.mHEAD_NUM);
                    mStdfFTRv4.SetTEST_FLG(lFTRElement.mTEST_FLG);
                    mStdfFTRv4.SetTEST_TXT(lFTRElement.mTEST_TXT + " " + lNameComplement);
                    mStdfFTRv4.SetVECT_NAM(lFTRElement.mVECT_NAM);
                    mStdfFTRv4.Write(lStdfFile);
                }
                // check if the next line is a test result
                lStrString = ReadLine(lTeradyneAsciiFile).trimmed();
                QStringList lItems = lStrString.simplified().split(" ");
                if (!lStrString.startsWith("NOTE:", Qt::CaseInsensitive))
                {
                    if((lItems.size() <= 1) || (lItems.size() > 5))
                        lEndTable = true;
                    else
                    {
                        bool ok;
                        lItems[0].toInt(&ok);
                        if (!ok)
                            lEndTable = true;
                    }
                }
            }
        }

        // Read a PTR if we have more than 9 element in the line and the first item is a number
        QStringList lItems = lStrString.simplified().split(" ");
        if(lItems.size() > 9)
        {
            bool ok;
            lItems[0].toInt(&ok);
            if (ok)
            {
                // Write a PTR
                WritePTR(lStrString, lStdfFile, lSiteList);
            }
        }
        if (!lStrString.startsWith("Site")
                && !lStrString.contains("tests/Executed")
                && !lStrString.startsWith("NOTE:")
                && !lStrString.startsWith("Characterization"))
            lStrString = ReadLine(lTeradyneAsciiFile).trimmed();
    }

    // Write MRR
    mStdfMRRv4.SetFINISH_T(0);
    mStdfMRRv4.Write(lStdfFile);

    // Close STDF file.
    lStdfFile.Close();

    // Success
    return true;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' TERADYNE_ASCII file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGTERADYNE_ASCIItoSTDF::Convert(const char *lTeradyneAsciiFileName,
                                     const char *lFileNameSTDF)
{
    // No erro (default)
    mIdLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo lInputFile(lTeradyneAsciiFileName);
    QFileInfo lOutputFile(lFileNameSTDF);

    QFile lFile( lFileNameSTDF );
    if((lFile.exists() == true) && (lInputFile.lastModified() < lOutputFile.lastModified()))
        return true;

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    bool bHideProgressAfter=true;
    bool bHideLabelAfter=false;

    mProgressStep = 0;
    mNextFilePos = 0;

    if(GexScriptStatusLabel != NULL)
    {
        if(GexScriptStatusLabel->isHidden())
        {
            bHideLabelAfter = true;
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+
                                                             QFileInfo(lTeradyneAsciiFileName).fileName()+
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

    if(ReadTeradyneAsciiFile(lTeradyneAsciiFileName,lFileNameSTDF) != true)
    {
        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if((GexProgressBar != NULL)
        && bHideProgressAfter)
            GexProgressBar->hide();

        if((GexScriptStatusLabel != NULL)
        && bHideLabelAfter)
            GexScriptStatusLabel->hide();

        return false;	// Error reading TERADYNE_ASCII file
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
QString CGTERADYNE_ASCIItoSTDF::ReadLine(QTextStream& lFile)
{
    QString lStrString;

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    if(GexProgressBar != NULL)
    {
        while((int) lFile.device()->pos() > mNextFilePos)
        {
            mProgressStep += 100/mFileSize + 1;
            mNextFilePos  += mFileSize/100 + 1;
            GexProgressBar->setValue(mProgressStep);
        }
    }
    QCoreApplication::processEvents();

    do
        lStrString = lFile.readLine();
    while(!lStrString.isNull() && lStrString.isEmpty());

    return lStrString;

}

//////////////////////////////////////////////////////////////////////
// Write PTR record
//////////////////////////////////////////////////////////////////////
bool CGTERADYNE_ASCIItoSTDF::WritePTR(QString lStrString, GS::StdLib::Stdf& lStdfFile, QList<stdf_type_u1>& lSiteList)
{
    mStdfPTRv4.SetTEST_NUM
            (lStrString.mid(mPtrFields.mNumber, mPtrFields.mSite - mPtrFields.mNumber).trimmed().toULong());
    mStdfPTRv4.SetHEAD_NUM(1);

    stdf_type_u1 lSiteNum = lStrString.mid(mPtrFields.mSite, mPtrFields.mResult - mPtrFields.mSite)
            .trimmed().toUShort();
    // write the PIR if the site_num doesn't exist
    if (!lSiteList.contains(lSiteNum))
    {
        lSiteList.append(lSiteNum);
        mStdfPIRv4.SetHEAD_NUM(1);
        mStdfPIRv4.SetSITE_NUM(lSiteNum);
        mStdfPIRv4.Write(lStdfFile);
    }


    mStdfPTRv4.SetSITE_NUM(lSiteNum);
    QString lPassFailString =
            lStrString.mid(mPtrFields.mResult, mPtrFields.mTestName - mPtrFields.mResult).trimmed();
    stdf_type_b1 lPassFail = 0;
    if (lPassFailString.isEmpty())
        lPassFail |= 0x40;
    else if (!lPassFailString.contains("PASS", Qt::CaseInsensitive))
        lPassFail |= 0x80;
    mStdfPTRv4.SetTEST_FLG(lPassFail);
    QString lTestName =
            lStrString.mid(mPtrFields.mTestName, mPtrFields.mPin - mPtrFields.mTestName).trimmed();
    QString lPin = lStrString.mid(mPtrFields.mPin, mPtrFields.mChannel - mPtrFields.mPin).trimmed();
    QString lForce = lStrString.mid(mPtrFields.mForce, mPtrFields.mLoc - mPtrFields.mForce).trimmed();
    lTestName += " " + lPin + " " + lForce;
    mStdfPTRv4.SetTEST_TXT(lTestName);
    mStdfPTRv4.SetPARM_FLG(0);
    QString lResult =
            lStrString.mid(mPtrFields.mMeasured, mPtrFields.mHigh - mPtrFields.mMeasured).simplified();
    QString lUnit = "";
    int nScale = 0;
    if (!lResult.isEmpty() && !lResult.contains("n/a", Qt::CaseInsensitive))
    {
        QStringList lResultList = lResult.split(" ", QString::SkipEmptyParts);
        if (lResultList.size() > 1)
            lUnit = lResult.split(" ", QString::SkipEmptyParts)[1];
        lResult = lResult.split(" ", QString::SkipEmptyParts)[0];

        float lValue = lResult.toFloat();
//                    NormalizeValues(lUnit, lValue, nScale, bIsNumber);
        mStdfPTRv4.SetRESULT(lValue);
        mStdfPTRv4.SetUNITS(lUnit);
//        if (bIsNumber)
            mStdfPTRv4.m_bRESULT_IsNAN = false;
//        else
//            mStdfPTRv4.m_bRESULT_IsNAN = true;
    }
    else
    {
        mStdfPTRv4.SetRESULT(0);
        mStdfPTRv4.m_bRESULT_IsNAN = true;
    }

    stdf_type_b1 lOptionalFlag = 0x0e;

    // Low Limits
    QString lLowLimit=lStrString.mid(mPtrFields.mLow, mPtrFields.mMeasured - mPtrFields.mLow).simplified();
    if (!lLowLimit.isEmpty() && !lLowLimit.contains("n/a", Qt::CaseInsensitive))
    {
        float lLowLimitFloat = (lLowLimit.split(" ", QString::SkipEmptyParts)[0]).toFloat();
//                    QString lUnit = lLowLimit.split(" ", QString::SkipEmptyParts)[1];
//                    bool lValidLowLimit;
//                    NormalizeValues(lUnit, lLowLimitFloat, nScale, lValidLowLimit);
        mStdfPTRv4.SetLO_LIMIT(lLowLimitFloat);
        mStdfPTRv4.SetLLM_SCAL(nScale);
    }
    else
    {
        mStdfPTRv4.SetLO_LIMIT(0);
        lOptionalFlag |= 0x50;      // bit 4 and 6 set to 1
    }

    // High Limits
    QString lHighLimit=lStrString.mid(mPtrFields.mHigh, mPtrFields.mForce - mPtrFields.mHigh).simplified();
    if (!lHighLimit.isEmpty() && !lHighLimit.contains("n/a", Qt::CaseInsensitive))
    {
        float lHighLimitFloat = (lHighLimit.split(" ", QString::SkipEmptyParts)[0]).toFloat();
//                    QString lUnit = lHighLimit.split(" ", QString::SkipEmptyParts)[1];
//                    bool lValidHighLimit;
//                    NormalizeValues(lUnit, lHighLimitFloat, nScale, lValidHighLimit);
        mStdfPTRv4.SetHLM_SCAL(nScale);
        mStdfPTRv4.SetHI_LIMIT(lHighLimitFloat);
    }
    else
    {
        mStdfPTRv4.SetHI_LIMIT(0);
        lOptionalFlag |= 0xA0;      // bit 5 and 7 set to 1
    }
    mStdfPTRv4.SetOPT_FLAG(lOptionalFlag);
    mStdfPTRv4.Write(lStdfFile);
    return true;
}
