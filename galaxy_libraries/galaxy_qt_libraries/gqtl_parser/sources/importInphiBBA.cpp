//////////////////////////////////////////////////////////////////////
// importInphiBBA.cpp: Convert a .CSV / Excel file to STDF V4.0
//////////////////////////////////////////////////////////////////////
#include "importInphiBBA.h"
#include "importConstants.h"

#include "gqtl_log.h"
#include "gqtl_global.h"
#include <math.h>

#include <qfileinfo.h>

/// In some cases, we have an optional field Source_Lot just after the wafer_id
//Column 1	Column 2	Column 3	Column 4	Column 5	Column 6	Column 7	Column 8	Column 9	Column 10...
//--- Global Info:
//LOC	WIN
//StartTime	20170526 14:34:41
//FinishTime	20170526 14:34:45
//Product	DP042A
//Lot_ID	DP042P005
//Wafer_ID	DP042P005-02
//TestProc	WS1
//Temperature	25C
//Operator	OPID01
//SoftBinName	1	PASS
//SoftBinName	2	Reserved for post-probe gain binning
//SoftBinName	3000	SPI Functional Test Fail (Register Read-Write, SPI Check, SPI specific screen)
//SoftBinName	3005	SPI Register Calibration Fail
//SoftBinName	3333	Reserved for post-probe failures (slicing sigma outliers, slicing gain tails)
//SoftBinName	3999	SPI: Other
//SoftBinName	4005	OTP_Burn Fail
//SoftBinName	4999	OTP: Other
//SoftBinName	5000	Func Fail - Digital Scan
//SoftBinName	5005	Check Register Fused Data Fail / OTP Readback
//SoftBinName	9031	WIN BIN -- Bond Pad(by DC test)
//SoftBinName	9033	WIN BIN -- Air-Bridge
//SoftBinName	9036	WIN BIN -- Probe mark shift(by DC test)
//HardBinName	1	PASS CATEGORY 1
//HardBinName	2	PASS CATEGORY 2
//HardBinName	3	FAIL - POST PROBE AND SPI
//HardBinName	4	FAIL - OTP
//HardBinName	5	FAIL - FUNCTIONAL
//HardBinName	6	FAIL - POWER
//HardBinName	7	FAIL - AC PARAM
//HardBinName	8	FAIL - LEAK SHORT
//HardBinName	9	FAIL - VISUAL
//--- Test Software:
//ProgramName	DP042_A3KS
//ProgramRev	TBD
//TestSpecRevision	Embedded
//--- Test Hardware:
//TesterName	AMIDA
//Handler_ID	EG-4090
//Load_board	SMU board
//--- Parametric Data:
//Parameter	SBIN	HBIN	P/F	REF_X	REF_Y	DIE_X	DIE_Y	X	Y	SITE	TIME	101000_VCC1_INN_DC	101020_VCC2_INP_DC ...
//Tests#												101000	101020	101030
//Unit												ohm	ohm	ohm
//HighL												60	60	60
//LowL												45	45	45
//PID-1	1	1	PASS	-2	-3	6	0	20	12	1	0	53.68092	55.87479	55.92019
//PID-2	8070	8	FAIL	-2	-3	5	1	19	13	1	0	98.4325	55.87479	55.85835
//PID-3	1	1	PASS	-2	-3	6	1	20	13	1	0	53.60494	55.81312	55.87895
//PID-4	1	1	PASS	-2	-3	4	2	18	14	1	0	53.89099	56.08135	55.98216
//PID-5	1	1	PASS	-2	-3	5	2	19	14	1	0	53.69995	55.95723	55.83777
//PID-6	1	1	PASS	-2	-3	6	2	20	14	1	0	53.47249	55.50679	55.61237
//PID-7	1	1	PASS	-2	-3	3	3	17	15	1	0	53.91016	56.10209	55.94083
//PID-8	1	1	PASS	-2	-3	4	3	18	15	1	0	53.73805	55.95723	55.98216
//PID-9	1	1	PASS	-2	-3	5	3	19	15	1	0	53.5481	55.85422	55.79665
//PID-10	1	1	PASS	-2	-3	6	3	20	15	1	0	53.34069	55.60852	55.61237
//PID-11	1	1	PASS	-2	-3	3	4	17	16	1	0	53.7762	56.03992	55.94083
//PID-12	1	1	PASS	-2	-3	4	4	18	16	1	0	53.64291	55.69018	55.73509
//PID-13	1	1	PASS	-2	-3	5	4	19	16	1	0	53.41592	55.58815	55.65321
//PID-14	9020	9	FAIL	-2	-3	6	4	20	16	1	0	53.19086	55.4662	55.32812
//PID-15	1	1	PASS	-2	-3	2	5	16	17	1	0	53.7953	56.06063	56.04427
//PID-16	1	1	PASS	-2	-3	3	5	17	17	1	0	53.71899	55.85422	55.7146
//PID-17	1	1	PASS	-2	-3	4	5	18	17	1	0	53.49137	55.69018	55.61237
//PID-18	1	1	PASS	-2	-3	5	5	19	17	1	0	53.2844	55.62892	55.51051
//PID-19	1	1	PASS	-2	-3	6	5	20	17	1	0	53.09764	55.3852	55.4293
//PID-20	1	1	PASS	-2	-3	1	6	15	18	1	0	53.81441	56.16441	56.00285
//PID-21	1	1	PASS	-2	-3	2	6	16	18	1	0	53.68092	56.01922	55.92019
//PID-22	9020	9	FAIL	-2	-3	3	6	17	18	1	0	53.5481	55.81312	55.8172
//PID-23	1	1	PASS	-2	-3	4	6	18	18	1	0	53.41592	55.52711	55.34832

namespace GS
{
namespace Parser
{

//////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////
InphiBBASTDF::InphiBBASTDF() : ParserBase(typeInphiBBA, "InphiBBA")
{
    mTestList = NULL;
    mParameterDirectory.SetFileName(GEX_TRI_INPHIBBA_PARAMETERS);
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
InphiBBASTDF::~InphiBBASTDF()
{
    // Destroy list of Parameters tables.
    if(mTestList!=NULL)
        delete [] mTestList;

    // Destroy list of Bin tables.
    qDeleteAll(mSoftBinning.values());
    mSoftBinning.clear();
    qDeleteAll(mHardBinning.values());
    mHardBinning.clear();
}



//////////////////////////////////////////////////////////////////////
// Check if File is compatible with EGL Skywork format
//////////////////////////////////////////////////////////////////////
bool InphiBBASTDF::IsCompatible(const QString &FileName)
{
    QFile lFile(FileName);
    if(!lFile.open( QIODevice::ReadOnly ))
    {
        // Convertion failed.
        return false;
    }
    // Assign file I/O stream
    QTextStream lCsvFile(&lFile);

    // Check the compatibility
    //Column 1	Column 2	Column 3	Column 4	Column 5	Column 6	Column 7	Column 8	Column 9	Column 10...
    //--- Global Info:
    //LOC	WIN
    //StartTime	20170526 14:34:41
    //FinishTime	20170526 14:34:45
    //Product	DP042A
    //Lot_ID	DP042P005
    //Wafer_ID	DP042P005-02
    //TestProc	WS1
    //Temperature	25C
    //Operator	OPID01
    //SoftBinName	1	PASS
    QString     lReadLine;
    QStringList lKeywords = QStringList("Column") << "--- Global Info:" << "LOC" << "StartTime" <<
                                                     "FinishTime" << "Product" << "Lot_ID" << "Wafer_ID" <<
                                                     "TestProc" << "Temperature" << "Operator";

    // Check format matches with expected keywords
    while(lKeywords.isEmpty() == false)
    {
        lReadLine = lCsvFile.readLine();

        // If keyword is not the expected one, then format is not compatible with Inphi BBA parser
        if (lReadLine.startsWith(lKeywords.takeFirst()) == false)
        {
            lFile.close();
            return false;
        }
    }

    // Close file
    lFile.close();

    return true;
}


//////////////////////////////////////////////////////////////////////
bool InphiBBASTDF::ConvertoStdf(const QString &CsvFileName,  QString &StdfFileName)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString(" file %1").arg( CsvFileName).toLatin1().constData());

    QString lStrString;

    // Open CSV file
    QFile lFile( CsvFileName );
    if(!lFile.open( QIODevice::ReadOnly ))
    {
        // Failed Opening CSV file
        mLastError = errOpenFail;
        mLastErrorMessage = lFile.errorString();
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());

        // Convertion failed.
        return false;
    }
    // Assign file I/O stream
    QTextStream lCsvFile(&lFile);

    lStrString = ReadLine(lCsvFile);
    // Check the compatibility
    //Column 1	Column 2	Column 3	Column 4	Column 5	Column 6	Column 7	Column 8	Column 9	Column 10...
    //--- Global Info:
    //LOC	WIN
    //StartTime	20170526 14:34:41
    //FinishTime	20170526 14:34:45
    //Product	DP042A
    //Lot_ID	DP042P005
    //Wafer_ID	DP042P005-02
    //TestProc	WS1
    //Temperature	25C
    //Operator	OPID01
    //SoftBinName	1	PASS
    // Extract the headers

    if(!lStrString.startsWith("Column", Qt::CaseInsensitive))
    {
        mLastError = InvalidKeyword;
        mLastErrorMessage = QString("Parser expects the first line to start with 'column' keyword.");
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());

        lFile.close();
        return false;
    }
    lStrString = ReadLine(lCsvFile);
    if (!lStrString.contains("Global Info", Qt::CaseInsensitive))
    {
        mLastError = InvalidKeyword;
        mLastErrorMessage = QString("Parser expects to find '--- Global Info' section on the second line.");
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
        lFile.close();
        return false;
    }

    // LOC, WIN
    lStrString = ReadLine(lCsvFile);
    if (!lStrString.startsWith("LOC", Qt::CaseInsensitive) || lStrString.split(",").size() <= 1)
    {
        mLastError = InvalidKeyword;
        mLastErrorMessage = QString("Parser could not find 'LOC' keyword at the expected position.");
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());

        lFile.close();
        return false;
    }
    mMIRRecord.SetFACIL_ID(lStrString.section(",", 1, 1));

    // StartTime,20170526 14:34:41
    lStrString = ReadLine(lCsvFile);
    if (!lStrString.startsWith("StartTime", Qt::CaseInsensitive)
        || lStrString.split(",").size() <= 1
        || lStrString.section(",", 1, 1).split(" ").size() <= 1)
    {
        mLastError = InvalidKeyword;
        mLastErrorMessage = QString("Parser could not find 'StartTime' keyword at the expected position.");
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());

        lFile.close();
        return false;
    }
    mStartTime = GetDate(lStrString.section(",", 1, 1).section(" ", 0, 0),
                         lStrString.section(",", 1, 1).section(" ", 1, 1));
    if(mStartTime == 0)
    {
        lFile.close();
        mLastError = errInvalidFormatParameter;
        mLastErrorMessage = QString("Invalid date/time format %1").arg(lStrString);
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
        return false;
    }
    mMIRRecord.SetSTART_T(mStartTime);
    mMIRRecord.SetSETUP_T(mStartTime);
    mWIRRecord.SetSTART_T(mStartTime);

    // FInishTime,20170526 14:28:42
    lStrString = ReadLine(lCsvFile);
    if (!lStrString.startsWith("FinishTime", Qt::CaseInsensitive)
        || lStrString.split(",").size() <= 1
        || lStrString.section(",", 1, 1).split(" ").size() <= 1)
    {
        mLastError = InvalidKeyword;
        mLastErrorMessage = QString("Parser could not find 'FinishTime' keyword at the expected position.");
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());

        lFile.close();
        return false;
    }
    gstime lFinishTime = GetDate(lStrString.section(",", 1, 1).section(" ", 0, 0),
                                  lStrString.section(",", 1, 1).section(" ", 1, 1));
    if(lFinishTime == 0)
    {
        lFile.close();
        mLastError = errInvalidFormatParameter;
        mLastErrorMessage = QString("Invalid date/time format %1").arg(lStrString);
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
        return false;
    }
    mMRRRecord.SetFINISH_T(lFinishTime);
    mWRRRecord.SetFINISH_T(lFinishTime);

    // Product,DP042A
    lStrString = ReadLine(lCsvFile);
    if (!lStrString.startsWith("Product", Qt::CaseInsensitive) || lStrString.split(",").size() <= 1)
    {
        mLastError = InvalidKeyword;
        mLastErrorMessage = QString("Parser could not find 'Product' keyword at the expected position.");
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());

        lFile.close();
        return false;
    }
    mMIRRecord.SetPART_TYP(lStrString.section(",", 1, 1));

    // Lot_ID,DP042P005
    lStrString = ReadLine(lCsvFile);
    if (!lStrString.startsWith("Lot_ID", Qt::CaseInsensitive) || lStrString.split(",").size() <= 1)
    {
        mLastError = InvalidKeyword;
        mLastErrorMessage = QString("Parser could not find 'Lot_ID' keyword at the expected position.");
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());

        lFile.close();
        return false;
    }
    mMIRRecord.SetLOT_ID(lStrString.section(",", 1, 1));

    // Wafer_ID,DP042P005-02
    lStrString = ReadLine(lCsvFile);
    if (!lStrString.startsWith("Wafer_ID", Qt::CaseInsensitive) || lStrString.split(",").size() <= 1)
    {
        mLastError = InvalidKeyword;
        mLastErrorMessage = QString("Parser could not find 'Wafer_ID' keyword at the expected position.");
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());

        lFile.close();
        return false;
    }
    mWIRRecord.SetWAFER_ID(lStrString.section(",", 1, 1));
    mWRRRecord.SetWAFER_ID(lStrString.section(",", 1, 1));

    // TestProc,WS1
    lStrString = ReadLine(lCsvFile);
    if (!lStrString.startsWith("TestProc", Qt::CaseInsensitive) || lStrString.split(",").size() <= 1)
    {
        mLastError = InvalidKeyword;
        mLastErrorMessage = QString("Parser could not find 'TestProc' keyword at the expected position.");
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());

        lFile.close();
        return false;
    }
    //mMIRRecord.SetRTST_COD(lStrString.section(",", 1, 1));

    // Temperature,25C
    lStrString = ReadLine(lCsvFile);
    if (!lStrString.startsWith("Temperature", Qt::CaseInsensitive) || lStrString.split(",").size() <= 1)
    {
        mLastError = InvalidKeyword;
        mLastErrorMessage = QString("Parser could not find 'Temperature' keyword at the expected position.");
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());

        lFile.close();
        return false;
    }
    mMIRRecord.SetTST_TEMP(lStrString.section(",", 1, 1));

    // Operator,OPID01
    lStrString = ReadLine(lCsvFile);
    if (!lStrString.startsWith("Operator", Qt::CaseInsensitive) || lStrString.split(",").size() <= 1)
    {
        mLastError = InvalidKeyword;
        mLastErrorMessage = QString("Parser could not find 'Operator' keyword at the expected position.");
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());

        lFile.close();
        return false;
    }
    mMIRRecord.SetOPER_NAM(lStrString.section(",", 1, 1));

    // Read binning section
    if (!FillBinning(lCsvFile))
    {
        lFile.close();
        return false;
    }

    // Construct custom Galaxy USER_TXT
    QString	lUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    lUserTxt += ":";
    lUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    lUserTxt += ":INPHI_BBA";
    mMIRRecord.SetUSER_TXT(lUserTxt.toLatin1().constData());

    // ProgramName,DP042_A3KS
    lStrString = ReadLine(lCsvFile);
    if (!lStrString.startsWith("ProgramName", Qt::CaseInsensitive) || lStrString.split(",").size() <= 1)
    {
        mLastError = InvalidKeyword;
        mLastErrorMessage = QString("Parser could not find 'ProgramName' keyword at the expected position.");
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());

        lFile.close();
        return false;
    }
    mMIRRecord.SetJOB_NAM(lStrString.section(",", 1, 1));

    // ProgramRev,TBD
    lStrString = ReadLine(lCsvFile);
    if (!lStrString.startsWith("ProgramRev", Qt::CaseInsensitive) || lStrString.split(",").size() <= 1)
    {
        mLastError = InvalidKeyword;
        mLastErrorMessage = QString("Parser could not find 'ProgramRev' keyword at the expected position.");
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());

        lFile.close();
        return false;
    }
    mMIRRecord.SetJOB_REV(lStrString.section(",", 1, 1));

    // TestSpecRevision,Embedded
    lStrString = ReadLine(lCsvFile);
    if (!lStrString.startsWith("TestSpecRevision", Qt::CaseInsensitive) || lStrString.split(",").size() <= 1)
    {
        mLastError = InvalidKeyword;
        mLastErrorMessage = QString("Parser could not find 'TestSpecRevision' keyword at the expected position.");
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());

        lFile.close();
        return false;
    }
    mMIRRecord.SetSPEC_VER(lStrString.section(",", 1, 1));

    // --- Test Hardware:
    lStrString = ReadLine(lCsvFile);
    // TesterName,AMIDA
    lStrString = ReadLine(lCsvFile);
    if (!lStrString.startsWith("TesterName", Qt::CaseInsensitive) || lStrString.split(",").size() <= 1)
    {
        mLastError = InvalidKeyword;
        mLastErrorMessage = QString("Parser could not find 'TesterName' keyword at the expected position.");
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());

        lFile.close();
        return false;
    }
    mMIRRecord.SetTSTR_TYP(lStrString.section(",", 1, 1));

    // Handler_ID,EG-4090
    lStrString = ReadLine(lCsvFile);
    if (!lStrString.startsWith("Handler_ID", Qt::CaseInsensitive) || lStrString.split(",").size() <= 1)
    {
        mLastError = InvalidKeyword;
        mLastErrorMessage = QString("Parser could not find 'Handler_ID' keyword at the expected position.");
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());

        lFile.close();
        return false;
    }
    mSDRRecord.SetHAND_ID(lStrString.section(",", 1, 1));

    // Load_board,SMU board
    lStrString = ReadLine(lCsvFile);
    if (!lStrString.startsWith("Load_board", Qt::CaseInsensitive) || lStrString.split(",").size() <= 1)
    {
        mLastError = InvalidKeyword;
        mLastErrorMessage = QString("Parser could not find 'Load_board' keyword at the expected position.");
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());

        lFile.close();
        return false;
    }
    mSDRRecord.SetLOAD_ID(lStrString.section(",", 1, 1));

    // Finish the reading of the rest of the file: the results
    if(WriteStdfFile(lCsvFile, StdfFileName) != true)
    {
        QFile::remove(StdfFileName);
        // Close file
        lFile.close();
        return false;
    }

    // Close file
    lFile.close();

    mLastError = errNoError;
    mLastErrorMessage = "";
    // Success parsing Inphi BBA file
    return true;
}

gsbool InphiBBASTDF::IsNotAtEndOfFile(const QTextStream& lCsvFile)
{
    if(lCsvFile.atEnd() == true)
    {
        // Incorrect header...this is not a valid CSV file!
        mLastError = errInvalidFormatParameter;
        mLastErrorMessage = "Unexpected end of file";
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
        // Convertion failed. Close file
        return false;
    }
    return true;
}


gsbool InphiBBASTDF::FillBinning(QTextStream& lCsvFile)
{
    QString lLine = ReadLine(lCsvFile);
    QStringList lFields;
    while (lLine.startsWith("SoftBinName", Qt::CaseInsensitive) || lLine.startsWith("HardBinName", Qt::CaseInsensitive))
    {
        lFields = lLine.split(",");
        if (lFields.size() < 3)
        {
            return false;
        }

        gsbool lPassFail(false);
        if (lFields[2].contains("pass", Qt::CaseInsensitive))
        {
            lPassFail = true;
        }
        ParserBinning* lBinning = new ParserBinning();
        if (lFields[0].compare("SoftBinName", Qt::CaseInsensitive) == 0)
        {
            lBinning->SetBinNumber(lFields[1].toInt());
            lBinning->SetPassFail(lPassFail);
            lBinning->SetBinName(lFields[2]);
            mSoftBinning.insert(lFields[1].toInt(), lBinning);
        }

        if (lFields[0].compare("HardBinName", Qt::CaseInsensitive) == 0)
        {
            lBinning->SetBinNumber(lFields[1].toInt());
            lBinning->SetPassFail(lPassFail);
            lBinning->SetBinName(lFields[2]);
            mHardBinning.insert(lFields[1].toInt(), lBinning);
        }
        lLine = ReadLine(lCsvFile);
    }

    if (mHardBinning.isEmpty())
    {
        mLastError = MissingHardBinDefinition;
        mLastErrorMessage = "Parser could not find any definition for Hard Binnings";
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());

        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from CSV data parsed
//////////////////////////////////////////////////////////////////////
bool InphiBBASTDF::WriteStdfFile(QTextStream &aCsvFile, const QString &StdfFileName)
{
    // now generate the STDF file...
    GQTL_STDF::Stdf_WCR_V4 lWCRRecord;
    GQTL_STDF::StdfParse lStdfParser;

    if(lStdfParser.Open(StdfFileName.toLatin1().constData(),STDF_WRITE) == false)
    {
        // Failed importing CSV file into STDF database
        mLastError = errWriteSTDF;
        mLastErrorMessage = "Enable to open the STDF file";
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
        // Convertion failed.
        return false;
    }

    QString lStrString = ReadLine(aCsvFile);
    while (!lStrString.startsWith("Parameter", Qt::CaseInsensitive))
    {
        lStrString = ReadLine(aCsvFile);
    }

    // Check if have all mandatory value
    QStringList lMandatoryValue;
    lMandatoryValue << "Parameter" << "SBIN" << "HBIN" << "P/F" << "REF_X" << "REF_Y" << "DIE_X" << "DIE_Y" << "X" << "Y" << "SITE" << "TIME";

    QStringList lStrCells = lStrString.split(",", QString::KeepEmptyParts);
    for (int i=0; i< lMandatoryValue.size(); ++i)
    {
        if(!lStrCells[i].contains(lMandatoryValue[i], Qt::CaseInsensitive))
        {
            mLastError = errInvalidFormatParameter;
            mLastErrorMessage = "Column '"+lStrCells[i]+"' must have a valid value";
            GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
            //lFile.close();
            return false;
        }
    }

    short lFirstTestRaw = 12;   // Contains the index of the first test raw
    short lNumberOfTests = lStrCells.size() - lFirstTestRaw;   // Contains the index of the first test raw

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("%1 Total Parameters").arg(lNumberOfTests).toLatin1().constData());
    // Allocate the buffer to hold the N parameters & results.
    try
    {
        mTestList = new ParserParameter[lNumberOfTests];	// List of parameters
    }
    catch(const std::bad_alloc& )
    {
        GSLOG(SYSLOG_SEV_ERROR, "Memory allocation exception caught ");
        //lFile.close();
        return false;
    }
    catch(...)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Exception caught ");
        //lFile.close();
        return false;
    }

    // Line 1 - Test name
    // Set test name in the csv parameters
    for (int i = 0; i < lNumberOfTests; ++i)
    {
        mTestList[i].SetTestName(lStrCells[i+lFirstTestRaw]);
    }

    // Line 2 - Test number
    if(!IsNotAtEndOfFile(aCsvFile))
    {
        //lFile.close();
        return false;
    }
    lStrString = ReadLine(aCsvFile);
    lStrCells = lStrString.split(",",QString::KeepEmptyParts);
    if (lStrCells.size() < lNumberOfTests)
    {
        mLastError = errInvalidFormatParameter;
        mLastErrorMessage = "The test number line is wrongly formated";
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
        //lFile.close();
        return false;
    }
    long lTestNumber;
    for (int i = 0; i < lNumberOfTests; ++i)
    {
        lTestNumber = lStrCells[lFirstTestRaw + i].toLong();
        mTestList[i].SetTestNumber(lTestNumber);
    }


    // read units
    int lScalingFactor = 0;
    // Line 3 - Unit
    if(!IsNotAtEndOfFile(aCsvFile))
    {
        //lFile.close();
        return false;
    }
    lStrString = ReadLine(aCsvFile);
    lStrCells = lStrString.split("," ,QString::KeepEmptyParts);
    if (lStrCells.size() < lNumberOfTests)
    {
        mLastError = errInvalidFormatParameter;
        mLastErrorMessage = "The units line is wrongly formated";
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
        //lFile.close();
        return false;
    }
    lStrCells = lStrString.split(",");
    QString lUnitString;
    for(int i = 0; i < lNumberOfTests; ++i)
    {
        lScalingFactor = 0;
        lUnitString = lStrCells[lFirstTestRaw + i];
        if (!GetNotNAString(lUnitString).isEmpty())
        {
            mTestList[i].SetTestUnit(GS::Core::NormalizeUnit(lUnitString, lScalingFactor));
            mTestList[i].SetResultScale(lScalingFactor);
        }
    }


    // Line 4 - High limit
    if(!IsNotAtEndOfFile(aCsvFile))
    {
        //lFile.close();
        return false;
    }
    lStrString = ReadLine(aCsvFile);
    lStrCells = lStrString.split("," ,QString::KeepEmptyParts);
    if (lStrCells.size() < lFirstTestRaw+lNumberOfTests)
    {
        mLastError = errInvalidFormatParameter;
        mLastErrorMessage = "The line for high limit is wrongly formated";
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
        //lFile.close();
        return false;
    }
    QString lHighLimit;
    for (int i = 0; i < lNumberOfTests; ++i)
    {
        lHighLimit = lStrCells[lFirstTestRaw + i];
        if (!GetNotNAString(lHighLimit).isEmpty())
        {
            mTestList[i].SetHighLimit(GS::Core::NormalizeValue(lHighLimit.toDouble(), mTestList[i].GetResultScale()));
            mTestList[i].SetValidHighLimit(true);
        }
    }

    // Line 5 - Low limit
    if(!IsNotAtEndOfFile(aCsvFile))
    {
        //lFile.close();
        return false;
    }
    lStrString = ReadLine(aCsvFile);
    lStrCells = lStrString.split("," ,QString::KeepEmptyParts);
    if (lStrCells.size() < lFirstTestRaw+lNumberOfTests)
    {
        mLastError = errInvalidFormatParameter;
        mLastErrorMessage = "The line for low limit is wrongly formated";
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
        //lFile.close();
        return false;
    }
    QString lLowLimit;
    for (int i = 0; i < lNumberOfTests; ++i)
    {
        lLowLimit = lStrCells[lFirstTestRaw + i];
        if (!GetNotNAString(lLowLimit).isEmpty())
        {
            mTestList[i].SetLowLimit(GS::Core::NormalizeValue(lLowLimit.toDouble(), mTestList[i].GetResultScale()));
            mTestList[i].SetValidLowLimit(true);
        }
    }

    // Beginning of test results
    if(!IsNotAtEndOfFile(aCsvFile))
    {
        //lFile.close();
        return false;
    }

    lStdfParser.WriteRecord(&mMIRRecord);

    // write SDR
    mSDRRecord.SetHEAD_NUM(1);
    mSDRRecord.SetSITE_GRP(1);
    mSDRRecord.SetSITE_CNT(1);
    mSDRRecord.SetSITE_NUM(1, 1);
    lStdfParser.WriteRecord(&mSDRRecord);

    // Write WIR
    mWIRRecord.SetHEAD_NUM(1);
    mWIRRecord.SetSITE_GRP(255);
    lStdfParser.WriteRecord(&mWIRRecord);

    QStringList lFields;
    QString lLine;

    lLine = ReadLine(aCsvFile);
    lFields = lLine.split(",");

    // Write Test results for each line read.
    QString lLineString;
    BYTE	lValidLimit;
    int lSiteNum;
    int lPartsPerWafer = 0, lGoodPartsPerWafer = 0;
    // Write all Parameters read on this wafer.: PTR....PTR, PRR
    while(aCsvFile.atEnd() == false) // Read all lines in file
    {
        if (lFields.size() == 0)
        {
            // Read line
            lLineString = ReadLine(aCsvFile);
            // Split line
            lFields = lLineString.split(",",QString::KeepEmptyParts);
            // ignore empty lines
            if(lLineString.isEmpty())
                continue;
        }

        if (lFields.size() < (lFirstTestRaw+lNumberOfTests))
        {
            //lStdfParser.Close();
            mLastError = errInvalidFormatParameter;
            mLastErrorMessage = QString("The number of parameter in this line is less than the expected one = %1")
                  .arg(lLineString);
            GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());

            return false;
        }

        lSiteNum = lFields[10].toInt();
        ++lPartsPerWafer;
        // Write PIR
        GQTL_STDF::Stdf_PIR_V4 lPIRRecord;
        lPIRRecord.SetHEAD_NUM(1);
        lPIRRecord.SetSITE_NUM(lSiteNum);
        lStdfParser.WriteRecord(&lPIRRecord);

        // Read Parameter results for this record
        for(int lIndex=0; lIndex<lNumberOfTests; ++lIndex)
        {
            // If it's a PTR (pattern name empty)
            GQTL_STDF::Stdf_PTR_V4 lPTRRecord;
            lPTRRecord.SetTEST_NUM(mTestList[lIndex].GetTestNumber());
            lPTRRecord.SetHEAD_NUM(1);
            lPTRRecord.SetSITE_NUM(lSiteNum);
            lPTRRecord.SetPARM_FLG(0);
            bool ok;
            float lValue = GS::Core::NormalizeValue(lFields[lFirstTestRaw + lIndex].toDouble(&ok),
                                                    mTestList[lIndex].GetResultScale());
            if (ok)
            {
                lPTRRecord.SetTEST_FLG(0x40);       // Result, no P/F indication
                lPTRRecord.SetRESULT(lValue);
            }
            else
            {
                lPTRRecord.SetTEST_FLG(0x52);       // No result, no Pass/fail indication
                lPTRRecord.SetRESULT(0);
            }

            if (mTestList[lIndex].GetStaticHeaderWritten() == false)
            {
                mTestList[lIndex].SetStaticHeaderWritten(true);
                lPTRRecord.SetTEST_TXT(mTestList[lIndex].GetTestName());
                lPTRRecord.SetRES_SCAL(mTestList[lIndex].GetResultScale());
                lPTRRecord.SetLLM_SCAL(mTestList[lIndex].GetResultScale());
                lPTRRecord.SetHLM_SCAL(mTestList[lIndex].GetResultScale());
                lValidLimit = 0x0e;
                if(mTestList[lIndex].GetValidLowLimit()==false)
                    lValidLimit |=0x40;
                if(mTestList[lIndex].GetValidHighLimit()==false)
                    lValidLimit |=0x80;
                lPTRRecord.SetOPT_FLAG(lValidLimit);
                lPTRRecord.SetLO_LIMIT(mTestList[lIndex].GetLowLimit());
                lPTRRecord.SetHI_LIMIT(mTestList[lIndex].GetHighLimit());
                lPTRRecord.SetUNITS(mTestList[lIndex].GetTestUnits());
            }
            lStdfParser.WriteRecord(&lPTRRecord);
        };

        // Write PRR
        GQTL_STDF::Stdf_PRR_V4 lPRRRecord;
        lPRRRecord.SetTEST_T(lFields[11].toLong()*1000);
        lPRRRecord.SetSITE_NUM(lSiteNum);
        int lSoftBin = lFields[1].toInt();
        int lHardBin = lFields[2].toInt();
        bool lBinFlag;
        lBinFlag = (lFields[3].toUpper() == "PASS");

        lPRRRecord.SetSOFT_BIN(lSoftBin);
        lPRRRecord.SetHARD_BIN(lHardBin);

        BYTE lPartFlg = 0;
        if (!lBinFlag)
            lPartFlg |= 0x08;
        else
            ++lGoodPartsPerWafer;
        lPRRRecord.SetPART_FLG(lPartFlg);
        lPRRRecord.SetX_COORD(lFields[8].toInt());
        lPRRRecord.SetY_COORD(lFields[9].toInt());
        lPRRRecord.SetPART_ID(lFields[0]);
        lStdfParser.WriteRecord(&lPRRRecord);

        if (WriteReticleDTR(lFields, lStdfParser) == false)
        {
            mLastError = errInvalidFormatParameter;
            mLastErrorMessage = QString("Can't write the reticle information");
            GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
            return false;
        }

        // Fill software map
        if(mSoftBinning.contains(lSoftBin))
        {
            ParserBinning *lBinning = mSoftBinning[lSoftBin];
            lBinning->SetBinCount((lBinning->GetBinCount()) + 1);
        }

        // Fill hardware map
        if(mHardBinning.contains(lHardBin))
        {
            ParserBinning *lBinning = mHardBinning[lHardBin];
            lBinning->SetBinCount((lBinning->GetBinCount()) + 1);
        }
        lFields.clear();
    };


    // Write WCR
    lStdfParser.WriteRecord(&lWCRRecord);

    // Write WRR
    mWRRRecord.SetHEAD_NUM(1);
    mWRRRecord.SetSITE_GRP(255);
    mWRRRecord.SetPART_CNT(lPartsPerWafer);
    mWRRRecord.SetGOOD_CNT(lGoodPartsPerWafer);
    lStdfParser.WriteRecord(&mWRRRecord);

    // Write HBR
    QMap<int,ParserBinning *>::Iterator it;
    GQTL_STDF::Stdf_HBR_V4 lHBRRecord;
    for(it = mHardBinning.begin(); it != mHardBinning.end(); it++)
    {
        ParserBinning *lBinning;
        lBinning = *it;
        lHBRRecord.SetHEAD_NUM(255);
        lHBRRecord.SetSITE_NUM(255);
        lHBRRecord.SetHBIN_NUM(lBinning->GetBinNumber());
        lHBRRecord.SetHBIN_CNT(lBinning->GetBinCount());
        if(lBinning->GetPassFail())
            lHBRRecord.SetHBIN_PF('P');
        else
            lHBRRecord.SetHBIN_PF('F');
        lHBRRecord.SetHBIN_NAM(lBinning->GetBinName().toLatin1().constData());
        lStdfParser.WriteRecord(&lHBRRecord);
    }

    // Write SBR
    GQTL_STDF::Stdf_SBR_V4 lSBRRecord;
    for(it = mSoftBinning.begin(); it != mSoftBinning.end(); it++)
    {
        ParserBinning *lBinning;
        lBinning = *it;
        lSBRRecord.SetHEAD_NUM(255);
        lSBRRecord.SetSITE_NUM(255);
        lSBRRecord.SetSBIN_NUM(lBinning->GetBinNumber());
        lSBRRecord.SetSBIN_CNT(lBinning->GetBinCount());
        if(lBinning->GetPassFail())
            lSBRRecord.SetSBIN_PF('P');
        else
            lSBRRecord.SetSBIN_PF('F');
        lSBRRecord.SetSBIN_NAM(lBinning->GetBinName().toLatin1().constData());
        lStdfParser.WriteRecord(&lSBRRecord);
    }

    // Write MRR
    // use the last date in the csv file
    lStdfParser.WriteRecord(&mMRRRecord);

    // Close STDF file.
    lStdfParser.Close();

    // Success
    return true;
}

gstime_t InphiBBASTDF::GetDate(const QString date, const QString time, const int daysToAdd)
{
    // Try to compute date
    QDate lDate = QDate::fromString(date, "yyyyMMdd");
    if(lDate.isNull() || !lDate.isValid())
        return 0;

    // Add days if required
    if(daysToAdd != 0)
        lDate = lDate.addDays(daysToAdd);

    // Try to compute time
    QTime lTime = QTime::fromString(time, "hh:mm:ss");
    if(lTime.isNull() || !lTime.isValid())
        return 0;

    // Return time_t from computed date/time
    return QDateTime(lDate, lTime, Qt::UTC).toTime_t();
}

QString InphiBBASTDF::GetNotNAString(const QString string)
{
    QString lNew = string.trimmed();
    if ((lNew.compare("NA", Qt::CaseInsensitive) == 0)
            || (lNew.compare("None", Qt::CaseInsensitive) == 0))
        lNew = "";
    return lNew;
}


    //////////////////////////////////////////////////////////////////////
// Skip empty line
        //////////////////////////////////////////////////////////////////////
void  InphiBBASTDF::SpecificReadLine (QString &line)
{
    if(line.left(3) == ",,," && (line.simplified().count(",")==line.simplified().length()))
        line = "";
    }

//////////////////////////////////////////////////////////////////////
// Check if the line is empty or contains only ','
//////////////////////////////////////////////////////////////////////
bool InphiBBASTDF::EmptyLine(const QString& line)
{
    bool lEmpty(true);
    if (!line.isEmpty())
    {
        QStringList lStrCells = line.split(",", QString::KeepEmptyParts);
        for(int lIndexCell=0; lIndexCell<lStrCells.count(); ++lIndexCell)
        {
            if (!lStrCells[lIndexCell].isEmpty())
            {
                lEmpty = false;
                break;
            }
        }
    }
    return lEmpty;
}

// The Json syntaxe
//{
//"TYPE":"RETICLE",
//"DIEX":integer,      // Die X coordinate on the map
//"DIEY":integer,      // Die Y coordinate on the map
//"RETX":integer,     // X coordinate within the reticle
//"RETY": integer     // Y coordinate within the reticle
//}
gsbool InphiBBASTDF::WriteReticleDTR(const QStringList& fields,
                                     GQTL_STDF::StdfParse& lStdfFile)
{
    gsbool lOk(false);

    // Extract Pos X and PosY of the reticle on the map from column X and Y
    gsint8 lPosX = fields.at(4).toInt(&lOk);
    if (!lOk)
    {
        mLastError = InvalidReticlePosX;
        return false;
    }

    gsint8 lPosY = fields.at(5).toInt(&lOk);
    if (!lOk)
    {
        mLastError = InvalidReticlePosY;
        return false;
    }

    // Extract X and Y coordinate inside the reticle
    gsint8 lRetX = fields.at(6).toInt(&lOk);
    if (!lOk)
    {
        mLastError = InvalidPositionInReticle;
        return false;
    }
    gsint8 lRetY = fields.at(7).toInt(&lOk);
    if (!lOk)
    {
        mLastError = InvalidPositionInReticle;
        return false;
    }


    GQTL_STDF::Stdf_DTR_V4 lDTRRecord;
    QJsonObject            lReticleField;
    lReticleField.insert("TYPE", QJsonValue(QString("reticle")));
    lReticleField.insert("DIEX", QJsonValue(fields.at(8).toInt()));
    lReticleField.insert("DIEY", QJsonValue(fields.at(9).toInt()));
    lReticleField.insert("RETX", QJsonValue(lRetX));
    lReticleField.insert("RETY", QJsonValue(lRetY));
    lReticleField.insert("RETPOSX", QJsonValue(lPosX));
    lReticleField.insert("RETPOSY", QJsonValue(lPosY));
    lDTRRecord.SetTEXT_DAT(lReticleField);
    lStdfFile.WriteRecord(&lDTRRecord);
    return true;
}


}
}
