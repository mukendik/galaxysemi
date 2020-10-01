//////////////////////////////////////////////////////////////////////
// import_semi_g85.cpp: Convert a SEMI_G85 file to STDF V4.0
//////////////////////////////////////////////////////////////////////
#include "importTriQuintRF.h"
#include "importConstants.h"
#include <gqtl_log.h>
#include <QFileInfo>

//* PROCESS LINE TQT DATE 00:01 24 Oct 2013   WAFER 132421101 OPER:BD
//* INPUT PROBE SN NONE	OUTPUT PROBE SN NONE	OTHER PROBE SN NONE	PROBER ID RF018
//* ET=Elapsed Time (min),  Key =
//* EG6717A			START SITE 1	STOP SITE 3401
//* FREQ MS11 AS11 MS21 AS21 MS12 AS12 MS22 AS22
//* GHZ MAG ANG MAG ANG MAG ANG MAG ANG
//*R 10 C 29 KEY A ET 00:03:20 V1= 0.425215 I1= -0.000004 V3= 8.999035 I3= 0.155793
//500.000000E-3	129.939631E-3	171.784607E+0	2.673316E+0	157.833237E+0	175.461531E-3	-34.794273E+0	238.316983E-3	135.962418E+0
//2.500000E+0	388.804108E-3	160.341507E+0	2.370290E+0	54.593201E+0	153.473467E-3	-81.099312E+0	75.728513E-3	-100.786171E+0
//3.250000E+0	375.746965E-3	139.543488E+0	2.487765E+0	17.536854E+0	129.228264E-3	-125.096832E+0	243.669733E-3	-102.026993E+0
//4.000000E+0	354.509801E-3	91.949066E+0	2.579771E+0	-22.441406E+0	100.887850E-3	-155.231293E+0	348.086834E-3	-124.148323E+0
//4.750000E+0	416.331023E-3	19.889091E+0	2.655458E+0	-66.867180E+0	62.088169E-3	-163.016220E+0	434.461236E-3	-147.894562E+0
//5.500000E+0	545.691252E-3	-51.742634E+0	2.512162E+0	-111.950302E+0	120.489478E-3	-171.207275E+0	375.782907E-3	-175.392929E+0
//6.250000E+0	584.112465E-3	-104.674683E+0	2.555477E+0	-159.137207E+0	176.791415E-3	151.002060E+0	338.286310E-3	154.058395E+0
//7.000000E+0	128.694192E-3	-179.934006E+0	2.796523E+0	137.523102E+0	226.065904E-3	83.515793E+0	367.869288E-3	78.933319E+0
//7.750000E+0	712.328911E-3	-52.542946E+0	1.597682E+0	55.444920E+0	140.819237E-3	6.711033E+0	560.204148E-3	-32.438187E+0
//*R 10 C 28 KEY A ET 00:03:42 V1= 0.402400 I1= -0.000005 V3= 8.999301 I3= 0.155218
//500.000000E-3	132.849246E-3	171.590454E+0	2.685135E+0	157.767639E+0	174.865574E-3	-34.423733E+0	240.152478E-3	136.571548E+0
//2.500000E+0	390.471727E-3	159.577713E+0	2.372077E+0	54.185928E+0	153.700069E-3	-80.638443E+0	77.494599E-3	-107.481476E+0
//3.250000E+0	374.873102E-3	138.530212E+0	2.487527E+0	17.031912E+0	130.771324E-3	-123.710884E+0	242.321610E-3	-104.768776E+0
//4.000000E+0	352.378577E-3	90.584846E+0	2.578894E+0	-23.065254E+0	102.456033E-3	-153.186234E+0	346.150517E-3	-126.656296E+0
//4.750000E+0	415.363312E-3	18.339264E+0	2.650302E+0	-67.577240E+0	67.164771E-3	-162.050415E+0	429.104775E-3	-150.326859E+0
//5.500000E+0	546.182930E-3	-52.982635E+0	2.504704E+0	-112.788589E+0	125.657871E-3	-172.724167E+0	369.693756E-3	-178.159073E+0
//6.250000E+0	582.445562E-3	-106.089966E+0	2.546276E+0	-160.182159E+0	180.790722E-3	149.448120E+0	332.582623E-3	149.939819E+0
//7.000000E+0	118.533455E-3	172.849396E+0	2.762929E+0	135.899826E+0	230.039522E-3	81.525070E+0	376.009434E-3	72.934807E+0
//7.750000E+0	718.624651E-3	-52.996220E+0	1.550188E+0	54.399529E+0	140.958950E-3	5.107509E+0	578.600466E-3	-34.593548E+0

namespace GS
{
namespace Parser
{

//////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////
TriQuintRFToSTDF::TriQuintRFToSTDF() : ParserBase(typeTriQuintRF, "typeTriQuintRF")
{
    mProbeId = "";
    mParameterDirectory.SetFileName(GEX_TRI_QUINT_RF_PARAMETERS);
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
TriQuintRFToSTDF::~TriQuintRFToSTDF()
{
}


//////////////////////////////////////////////////////////////////////
// Check if File is compatible with EGL Skywork format
//////////////////////////////////////////////////////////////////////
bool TriQuintRFToSTDF::IsCompatible(const QString &FileName)
{
    bool	lIsCompatible(false);

    QFile lFile(FileName);
    if(!lFile.open( QIODevice::ReadOnly ))
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Can not open file %1").arg(FileName).toLatin1().constData());
        // Convertion failed.
        return false;
    }
    // Assign file I/O stream
    QTextStream lInputFile(&lFile);


    // Check the compatibility
    QString lStrString("");
    while (lStrString.isEmpty() && !lInputFile.atEnd())
    {
        lStrString = lInputFile.readLine().trimmed();
    }

    if (lStrString.startsWith("* PROCESS LINE", Qt::CaseInsensitive)
        && lStrString.contains("DATE", Qt::CaseInsensitive)
        && lStrString.contains("WAFER", Qt::CaseInsensitive)
        && lStrString.contains("OPER", Qt::CaseInsensitive))
    {
        lIsCompatible = true;
    }
    else
    {
        lIsCompatible =false;
    }

    // Read the next line to confirm that we are reading the TriQuint RF
    if (lIsCompatible)
    {
        lStrString = lInputFile.readLine().trimmed();
        if (!lStrString.contains("INPUT PROBE"))
        {
            lIsCompatible = false;
        }
    }

    // Close file
    lFile.close();

    return lIsCompatible;
}


//////////////////////////////////////////////////////////////////////
/// Read the file twice. The first pass is to find the MIR fields
/// The second one is the fill all others records
/////////////////////////////////////////////////////////////////////
bool TriQuintRFToSTDF::ConvertoStdf(const QString &triQuintRFFileName, QString &stdfFileName)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString(" file %1").arg( triQuintRFFileName).toLatin1().constData());
    QString lStrString("");

    // Open TriQuintRF file
    QFile lFile(triQuintRFFileName);
    if(!lFile.open( QIODevice::ReadOnly ))
    {
        // Failed Opening TriQuintRF file
        mLastError = errOpenFail;

        // Convertion failed.
        return false;
    }
    // Assign file I/O stream
    QTextStream lInputFile(&lFile);

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    //mFileSize = lFile.size()+1;

    // Read the first non-empty line
    while (lStrString.isEmpty() && !lInputFile.atEnd())
    {
        lStrString = lInputFile.readLine().trimmed();
    }

    // Just recheck that the file is compatible.
    if (!lStrString.startsWith("* PROCESS LINE", Qt::CaseInsensitive)
        || !lStrString.contains("DATE", Qt::CaseInsensitive)
        || !lStrString.contains("WAFER", Qt::CaseInsensitive)
        || !lStrString.contains("OPER", Qt::CaseInsensitive))
    {
        // Close file
        lFile.close();
        mLastError = errInvalidFormatParameter;
        GSLOG(SYSLOG_SEV_ERROR, "Invalid header format");
        // Fail parsing TriQuintRF file
        return false;
    }

    // * PROCESS LINE TQT DATE 00:01 24 Oct 2013   WAFER 132421101 OPER:BD
    // The date is beween date and wafer
    QString lDateString = (lStrString.section("DATE", 1, 1, QString::SectionCaseInsensitiveSeps))
            .section("WAFER", 0, 0, QString::SectionCaseInsensitiveSeps).trimmed();
    QStringList lDateList = lDateString.split(' ');
    mDate = QDate(1900, 1, 1);
    if (lDateList.count() == 4)
    {
        int lYear = lDateList[3].toInt();
        if (lYear < 70)
            lYear += 2000;
        else if (lYear < 99)
            lYear += 1900;

        QMap<QString, int> lMonth;
        lMonth.insert("Jan", 1);
        lMonth.insert("Feb", 2);
        lMonth.insert("Mar", 3);
        lMonth.insert("Apr", 4);
        lMonth.insert("May", 5);
        lMonth.insert("Jun", 6);
        lMonth.insert("Jul", 7);
        lMonth.insert("Aug", 8);
        lMonth.insert("Sep", 9);
        lMonth.insert("Oct", 10);
        lMonth.insert("Nov", 11);
        lMonth.insert("Dec", 12);
        mDate.setDate(lYear, lMonth.value(lDateList[2]), lDateList[1].toInt());
    }
    else
    {
        // Close file
        lFile.close();
        mLastError = errInvalidFormatParameter;
        GSLOG(SYSLOG_SEV_ERROR, QString("Invalid date format %1").arg(lDateString).toLatin1().constData());
        // Fail parsing TriQuintRF file
        return false;
    }
    QTime lTime = QTime::fromString(lDateList[0], "hh:mm");
    QDateTime lDateTime = QDateTime(mDate, lTime,Qt::UTC);
    mStartTime = lDateTime.toTime_t();

    mMIRRecord.SetSTART_T(mStartTime);
    mMIRRecord.SetSETUP_T(mStartTime);
    mWIRRecord.SetSTART_T(mStartTime);

    // Get the wafer lot and the wafer id
    // * PROCESS LINE TQT DATE 00:01 24 Oct 2013   WAFER 132421101 OPER:BD
    // WAFER 132421101 => wafer_id = 01 (two last degit) and the rest is a lot_id
    QString lWafer = (lStrString.section("WAFER", 1, 1, QString::SectionCaseInsensitiveSeps))
            .section("OPER", 0, 0, QString::SectionCaseInsensitiveSeps).trimmed();
    mWIRRecord.SetWAFER_ID(lWafer.right(2));
    mWRRRecord.SetWAFER_ID(lWafer.right(2));
    mMIRRecord.SetLOT_ID(lWafer.left(lWafer.size() - 2));

    // Add the operator
    mMIRRecord.SetOPER_NAM(lStrString.section("OPER:", 1, 1, QString::SectionCaseInsensitiveSeps));

    // Read PROB ID
    lStrString = ReadLine(lInputFile);
    if (lStrString.contains("PROBER ID", Qt::CaseInsensitive))
    {
        mProbeId = lStrString.section("PROBER ID", 1, 1, QString::SectionCaseInsensitiveSeps);
    }

    lStrString = ReadLine(lInputFile);
    // Read the product ID
    if (lStrString.startsWith("* ET", Qt::CaseInsensitive))
    {
        lStrString = ReadLine(lInputFile);
        if (lStrString.contains("START SITE", Qt::CaseInsensitive))
        {
            mMIRRecord.SetPART_TYP(lStrString.section(" ", 1, 1));
        }
    }

    // Read the rest of the file until we found 2 last lines
    // * SHELL_LV_MPVS REV 111, DEFAULTS EG6717ASP1_MS113MS1 REV NE
    // * ADT 7.72 TESTED 3401 DATE 07:18 24 Oct 2013
    while (!lStrString.contains("REV", Qt::CaseInsensitive)
           && !lStrString.contains("DEFAULTS", Qt::CaseInsensitive)
           && !lInputFile.atEnd())
    {
        lStrString = ReadLine(lInputFile);
    }
    mMIRRecord.SetJOB_NAM(lStrString.section(" ", 1, 1));
    mMIRRecord.SetJOB_REV(lStrString.section(" ", 2, 3).remove(","));
    lStrString = ReadLine(lInputFile);
    if (lStrString.contains("ADT", Qt::CaseInsensitive)
        && lStrString.contains("TESTED", Qt::CaseInsensitive))
    {
        mMRRRecord.SetEXC_DESC(lStrString.section(" ", 2, 2));
    }

    lInputFile.seek(0);

    // Read the FREQ line
    // * FREQ MS11 AS11 MS21 AS21 MS12 AS12 MS22 AS22
    while (!lStrString.startsWith("* FREQ", Qt::CaseInsensitive)
           && !lInputFile.atEnd())
    {
        lStrString = ReadLine(lInputFile);
    }
    if (!lInputFile.atEnd())
    {
        QStringList lTestNames = lStrString.split(" ");
        for (unsigned short i=2; i<lTestNames.size(); ++i)
        {
            mTestNames.append(lTestNames[i]);
        }
    }
    else
    {
        // Close file
        lFile.close();
        mLastError = errInvalidFormatParameter;
        GSLOG(SYSLOG_SEV_ERROR, "Unexpected end of file");
        // Fail parsing TriQuintRF file
        return false;
    }

    // Read the FREQ line
    // * GHZ MAG ANG MAG ANG MAG ANG MAG ANG
    while (!lStrString.startsWith("* GHZ", Qt::CaseInsensitive)
           && !lInputFile.atEnd())
    {
        lStrString = ReadLine(lInputFile);
    }
    if (!lInputFile.atEnd())
    {
        QStringList lTestUnits = lStrString.split(" ");
        for (unsigned short i=1; i<lTestUnits.size(); ++i)
        {
            mTestUnits.append(lTestUnits[i]);
        }
    }
    else
    {
        // Close file
        lFile.close();
        mLastError = errInvalidFormatParameter;
        GSLOG(SYSLOG_SEV_ERROR, "Unexpected end of file");
        // Fail parsing TriQuintRF file
        return false;
    }

    if(WriteStdfFile(lInputFile, stdfFileName) != true)
    {
        QFile::remove(stdfFileName);
        // Close file
        lFile.close();
        return false;
    }

    // Close file
    lFile.close();

    mLastError = errNoError;
    // Success parsing TriQuintRF file
    return true;
}

void  TriQuintRFToSTDF::SpecificReadLine (QString &line)
{
    line = line.simplified();
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from TriQuintRF data parsed
//////////////////////////////////////////////////////////////////////
bool TriQuintRFToSTDF::WriteStdfFile(QTextStream &inputFile, const QString &StdfFileName)
{
    // now generate the STDF file...
    GS::StdLib::Stdf lStdfFile;
    if(lStdfFile.Open(StdfFileName.toLatin1().constData(),STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        // Failed importing TriQuintRF file into STDF database
        mLastError = errWriteSTDF;

        // Convertion failed.
        return false;
    }

    // Ensure CPU type (for encoding STDF records) is the one of the computer platform running the codes.
    lStdfFile.SetStdfCpuType(lStdfFile.GetComputerCpuType());

    // Write FAR
    GQTL_STDF::Stdf_FAR_V4 lFARrecord;
    lFARrecord.SetCPU_TYPE(lStdfFile.GetComputerCpuType());	 // Force CPU type to current computer platform.
    lFARrecord.SetSTDF_VER(4);                               // STDF V4
    lFARrecord.Write(lStdfFile);

    // Write MIR
    // Add undefined values in the TriQuintRF file and mandatory fields to the STDF
    mMIRRecord.SetSTAT_NUM(1);
    mMIRRecord.SetMODE_COD(' ');
    mMIRRecord.SetRTST_COD(' ');
    mMIRRecord.SetPROT_COD(' ');
    mMIRRecord.SetBURN_TIM(65535);
    mMIRRecord.SetCMOD_COD(' ');
    mMIRRecord.SetTSTR_TYP("");
    mMIRRecord.SetNODE_NAM("");
    mMIRRecord.SetTSTR_TYP("");
    // Construct custom Galaxy USER_TXT
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    strUserTxt += ":TRIQUINTRF";
    mMIRRecord.SetUSER_TXT(strUserTxt.toLatin1().constData());
    mMIRRecord.Write(lStdfFile);

    // Write SDR
    GQTL_STDF::Stdf_SDR_V4 lSDRRecord;
    // Set undefined values
    lSDRRecord.SetHEAD_NUM(1);
    lSDRRecord.SetSITE_GRP(1);
    lSDRRecord.SetSITE_CNT(1);
    lSDRRecord.SetSITE_NUM(1, 1);
    lSDRRecord.SetHAND_ID(mProbeId);
    lSDRRecord.Write(lStdfFile);

    // Write WIR
    mWIRRecord.SetHEAD_NUM(1);
    mWIRRecord.SetSITE_GRP(255);
    mWIRRecord.Write(lStdfFile);

    // Write PIR, PTR and PRR
    GQTL_STDF::Stdf_PRR_V4 lPRRRecord;
    GQTL_STDF::Stdf_PIR_V4 lPIRRecord;
    GQTL_STDF::Stdf_PTR_V4 lPTRRecord;
    // By default the PIR doesn't have any information
    lPIRRecord.SetHEAD_NUM(1);
    lPIRRecord.SetSITE_NUM(1);

    // lFailedParts is for the die that didn't pass the limits and lBadParts is the number of die with no result
    unsigned int lGoodParts(0), lBadParts(0), lFailedParts(0);
    QStringList lFields;
    QString lLine("");
    QStringList lTestNames;

    bool lWritePRR(false);

    QList< QPair<QString, QString> > lDCLimits;
    QMap<QString, QList< QPair<QString, QString> > > lLimits; // will contain the (Freq, <ll,hl>)

    // Read the rest of the file and fill the PRR, PTR and PIR
    while (!inputFile.atEnd() && !lLine.contains("SHELL_LV_MPVS", Qt::CaseInsensitive))
    {
        // * DC_Limits (0.1,0.2) (0.25,0.5) (-2.5 8.2) (,)
        if (lLine.startsWith("* DC_Limits", Qt::CaseInsensitive))
        {
            if (lLine.count("(") != lLine.count(")"))
            {
                GSLOG(SYSLOG_SEV_ERROR, QString("Invalid limit line %1")
                      .arg(lLine).toLatin1().constData());
                return false;
            }

            lDCLimits.clear();
            QStringList lLimitsFields = lLine.split(" ");
            for (int i=2; i<lLimitsFields.size(); ++i)
            {
                lLimitsFields[i] = lLimitsFields[i].remove("(").remove(")");
                // By default, section function doesn't skip empty field
                lDCLimits.append(QPair<QString, QString>(lLimitsFields[i].section(",", 0, 0),
                                                         lLimitsFields[i].section(",", 1, 1)));
            }

            // Read all others limits values
            while (!lLine.startsWith("*R", Qt::CaseInsensitive))
            {
                QString lFreq = lLine.section(" ", 1, 1);
                QList< QPair<QString, QString> > lLimitsList;
                if (lLine.count("(") != lLine.count(")"))
                {
                    GSLOG(SYSLOG_SEV_ERROR, QString("Invalid limit line %1")
                          .arg(lLine).toLatin1().constData());
                    return false;
                }
                QStringList lLimitsFields = lLine.split(" ");
                for (int i=2; i<lLimitsFields.size(); ++i)
                {
                    lLimitsFields[i] = lLimitsFields[i].remove("(").remove(")");
                    // By default, section function doesn't skip empty field
                    lLimitsList.append(QPair<QString, QString>(lLimitsFields[i].section(",", 0, 0),
                                                               lLimitsFields[i].section(",", 1, 1)));
                }
                lLimits.insert(lFreq, lLimitsList);
                lLine = ReadLine(inputFile);
            }
        }
        // *R 10 C 29 KEY A ET 00:03:20 V1= 0.425215 I1= -0.000004 V3= 8.999035 I3= 0.155793
        if (lLine.startsWith("*R", Qt::CaseInsensitive))
        {
            // Write PIR
            lPIRRecord.Write(lStdfFile);
            lWritePRR = true;

            lFields.clear();
            lFields = lLine.split(" ", QString::SkipEmptyParts);

            // Fill x and y coordinates
            lPRRRecord.SetX_COORD(lFields[3].toInt());
            lPRRRecord.SetY_COORD(lFields[1].toInt());
            lPRRRecord.SetPART_ID(lFields[3] + "." + lFields[1]);

            // Fill the Elapsed test time in milliseconds
            // 05:03:20 format hh:mm:ss
            unsigned long lElaspedTime(0);
            if (lFields[7].split(":").size() == 3)
            {
                QTime lTime = QTime::fromString(lFields[7], "hh:mm:ss");
                QDateTime lDateTime = QDateTime(mDate, lTime,Qt::UTC);
                lElaspedTime = (lDateTime.toTime_t() - mStartTime) * 1000;
                mStartTime = lDateTime.toTime_t();
            }
            else
            {
                GSLOG(SYSLOG_SEV_ERROR, QString("Invalid elapsed test time date %1")
                      .arg(lFields[7]).toLatin1().constData());
            }
            lPRRRecord.SetTEST_T(lElaspedTime);

            bool lDieFail(false);
            // Read tests in the end of the line
            // V1= 0.425215 I1= -0.000004 V3= 8.999035 I3= 0.155793
            // Remove the "=" and we will have test name, test result
            QStringList lTests = lLine.section(" ", 8).remove("=").split(" ");
            for(unsigned short i=0; i<(lTests.size() / 2); ++i )
            {
                QString lTestName = lTests[2*i];
                if(!lTestNames.contains(lTestName))
                {
                    lTestNames.append(lTestName);
                }
                unsigned long lTestNumber = mParameterDirectory.UpdateParameterIndexTable(lTestName);
                lPTRRecord.SetTEST_NUM(lTestNumber);
                lPTRRecord.SetHEAD_NUM(1);
                lPTRRecord.SetSITE_NUM(1);
//                lPTRRecord.SetTEST_FLG(0x40); // Test completed with no pass/fail indication
                lPTRRecord.SetPARM_FLG(0);
                float lResult = lTests[2*i+1].toFloat();
                lPTRRecord.SetRESULT(lResult);
                lPTRRecord.SetTEST_TXT(lTestName);
                lPTRRecord.SetUNITS("");

                if (mTestParams.contains(lTestName))
                {
                    unsigned long lExec = (mTestParams.value(lTestName)->GetExecCount());
                    (mTestParams.value(lTestName))->SetExecCount(++lExec);
                }
                else
                {
                    ParserParameter* lParmer = new ParserParameter();
                    lParmer->SetExecCount(1);
                    lParmer->SetTestName(lTestName);
                    lParmer->SetTestNumber(lTestNumber);
                    lParmer->SetTestValue(lResult);
                    mTestParams.insert(lTestName, lParmer);
                }


                if (lDCLimits.size() >= (lTests.size()/2))
                {
                    bool lTestFail(false);
                    BYTE lValidLimit = 0x0e;
                    QString lLowLimit = lDCLimits[i].first;
                    QString lHighLimit = lDCLimits[i].second;
                    if (!lLowLimit.isEmpty())
                    {
                        lPTRRecord.SetLO_LIMIT(lLowLimit.toFloat());
                        if (lResult < lLowLimit.toFloat())
                        {
                            lTestFail = true;
                        }
                    }
                    else
                    {
                        lValidLimit |=0x40;
                    }
                    if (!lHighLimit.isEmpty())
                    {
                        lPTRRecord.SetHI_LIMIT(lHighLimit.toFloat());
                        if (lResult > lHighLimit.toFloat())
                        {
                            lTestFail = true;
                        }
                    }
                    else
                    {
                        lValidLimit |=0x80;
                    }
                    if (lTestFail == true)
                    {
                        unsigned long lExecFail = (mTestParams.value(lTestName)->GetExecFail());
                        (mTestParams.value(lTestName))->SetExecFail(++lExecFail);
                        lPTRRecord.SetTEST_FLG(0x80);
                        lDieFail = true;
                        ++lFailedParts;
                    }
                    else
                    {
                        ++lGoodParts;
                        lPTRRecord.SetTEST_FLG(0x0);
                    }
                    lPTRRecord.SetOPT_FLAG(lValidLimit);
                }
                else
                {
                    ++lGoodParts;
                    lPTRRecord.SetTEST_FLG(0x40); // Test completed with no pass/fail indication
                }

                lPTRRecord.Write(lStdfFile);
            }

            // Read result lines
            lLine = ReadLine(inputFile);

            // For failure: * DC Bias Failure - Drain & Gate Limits Exceeded
            if (lLine.contains("DC Bias Failure", Qt::CaseInsensitive))
            {
                ++lBadParts;
                lPRRRecord.SetPART_FLG(0x08);
                lPRRRecord.SetHARD_BIN(2);
                lPRRRecord.SetSOFT_BIN(2);
                lPRRRecord.SetNUM_TEST(lTests.size() / 2);
                lPRRRecord.SetPART_TXT(lLine);
            }
            // 500.000000E-3	111.396648E-3	178.287567E+0	2.564586E+0	156.544052E+0	176.122740E-3	-35.556717E+0	228.883699E-3	135.976639E+0
            else
            {
                lPRRRecord.SetPART_FLG(0);
                lPRRRecord.SetHARD_BIN(1);
                lPRRRecord.SetSOFT_BIN(1);
                lPRRRecord.SetPART_TXT("");

                while (!lLine.startsWith("*R", Qt::CaseInsensitive)
                       && !lLine.isEmpty()
                       && !inputFile.atEnd()
                       && !lLine.contains("SHELL_LV_MPVS", Qt::CaseInsensitive))
                {
                    QStringList lResults = lLine.split(" ");
                    if (lResults.size() != (mTestNames.size()+1)
                        || lResults.size() != mTestUnits.size())
                    {
                        GSLOG(SYSLOG_SEV_ERROR, QString("Invalid measurement: %1")
                              .arg(mStartTime).toLatin1().constData());
                        // Close STDF file.
                        lStdfFile.Close();
                        return false;

                    }
                    QList < QPair<QString, QString> > lLineLimits = lLimits.value(lResults[0]);
                    for (unsigned short lResultIndex=1; lResultIndex<lResults.size(); ++lResultIndex)
                    {
                        QString lTestName = mTestNames[lResultIndex-1]
                                            + " " + lResults[0]
                                            + " " + mTestUnits[0];
                        if(!lTestNames.contains(lTestName))
                        {
                            lTestNames.append(lTestName);
                        }
                        unsigned long lTestNumber = mParameterDirectory.UpdateParameterIndexTable(lTestName);

                        if (mTestParams.contains(lTestName))
                        {
                            unsigned long lExec = (mTestParams.value(lTestName)->GetExecCount());
                            (mTestParams.value(lTestName))->SetExecCount(++lExec);
                        }
                        else
                        {
                            ParserParameter* lParmer = new ParserParameter();
                            lParmer->SetExecCount(1);
                            lParmer->SetExecFail(0);
                            lParmer->SetTestName(lTestName);
                            lParmer->SetTestNumber(lTestNumber);
                            lParmer->SetTestUnit(mTestUnits[lResultIndex-1]);
                            lParmer->SetTestValue(lResults[lResultIndex].toFloat());
                            mTestParams.insert(lTestName, lParmer);
                        }

                        lPTRRecord.SetTEST_NUM(lTestNumber);
                        lPTRRecord.SetHEAD_NUM(1);
                        lPTRRecord.SetSITE_NUM(1);



                        if (lLineLimits.size() > lResultIndex-1)
                        {
                            bool lTestFail(false);
                            BYTE lValidLimit = 0x0e;
                            QString lLowLimit = lLineLimits[lResultIndex-1].first;
                            QString lHighLimit = lLineLimits[lResultIndex-1].second;
                            if (!lLowLimit.isEmpty())
                            {
                                lPTRRecord.SetLO_LIMIT(lLowLimit.toDouble());
                                if (lResults[lResultIndex].toDouble() < lLowLimit.toDouble())
                                {
                                    lTestFail = true;
                                }
                            }
                            else
                            {
                                lValidLimit |=0x40;
                            }
                            if (!lHighLimit.isEmpty())
                            {
                                lPTRRecord.SetHI_LIMIT(lHighLimit.toDouble());
                                if (lResults[lResultIndex].toDouble() > lHighLimit.toDouble())
                                {
                                    lTestFail = true;
                                }
                            }
                            else
                            {
                                lValidLimit |=0x80;
                            }
                            if (lTestFail == true)
                            {
                                unsigned long lExecFail = (mTestParams.value(lTestName)->GetExecFail());
                                (mTestParams.value(lTestName))->SetExecFail(++lExecFail);
                                lPTRRecord.SetTEST_FLG(0x80);
                                lDieFail = true;
                                ++lFailedParts;
                            }
                            else
                            {
                                ++lGoodParts;
                                lPTRRecord.SetTEST_FLG(0x0);
                            }
                            lPTRRecord.SetOPT_FLAG(lValidLimit);
                        }
                        else
                        {
                            ++lGoodParts;
                            lPTRRecord.SetTEST_FLG(0x40); // Test completed with no pass/fail indication
                        }
                        lPTRRecord.SetPARM_FLG(0);
                        lPTRRecord.SetRESULT(lResults[lResultIndex].toFloat());
                        lPTRRecord.SetTEST_TXT(lTestName);
                        lPTRRecord.SetUNITS(mTestUnits[lResultIndex]);
                        lPTRRecord.Write(lStdfFile);
                    }

                    lLine = ReadLine(inputFile);
                }
            }
            if (true == lWritePRR)
            {
                if (lDieFail == true)
                {
                    lPRRRecord.SetHARD_BIN(3);
                    lPRRRecord.SetSOFT_BIN(3);
                    lPRRRecord.SetPART_FLG(0x08);
                }
                else
                    lPRRRecord.SetPART_FLG(0x0);
                lPRRRecord.Write(lStdfFile);
                lWritePRR = false;
            }
        }
        else
        {
            lLine = ReadLine(inputFile);
        }
    }

    // Write the last PRR if it hasn't been writen
    if (true == lWritePRR)
    {
        lPRRRecord.Write(lStdfFile);
        lWritePRR = false;
    }



    // All tests names read...check if need to update the TriQuintRF Parameter list on disk?
    if(mParameterDirectory.GetNewParameterFound() == true)
        mParameterDirectory.DumpParameterIndexTable();

    // Write TSR
    GQTL_STDF::Stdf_TSR_V4 lTSRRecord;
    lTSRRecord.SetHEAD_NUM(255);
    QStringList lTestParams = mTestParams.keys();
    // To have the same order of TSRs in the STDF
    qSort(lTestParams);
    for (unsigned short lIndex=0; lIndex<lTestParams.size(); ++lIndex)
    {
        ParserParameter* lParmer = mTestParams.value(lTestParams[lIndex]);
        lTSRRecord.SetHEAD_NUM(255);
        lTSRRecord.SetSITE_NUM(1);
        lTSRRecord.SetTEST_TYP('P');
        lTSRRecord.SetTEST_NUM(lParmer->GetTestNumber());
        lTSRRecord.SetEXEC_CNT(lParmer->GetExecCount());
        lTSRRecord.SetFAIL_CNT(lParmer->GetExecFail());
        lTSRRecord.SetTEST_NAM(lParmer->GetTestName());
        lTSRRecord.Write(lStdfFile);
    }

    qDeleteAll(mTestParams.values());
    mTestParams.clear();

    // Write SBR
    // Write good one
    GQTL_STDF::Stdf_SBR_V4 lSBRRecord;
    lSBRRecord.SetHEAD_NUM(255);
    lSBRRecord.SetSITE_NUM(1);
    lSBRRecord.SetSBIN_NUM(1);
    lSBRRecord.SetSBIN_CNT(lGoodParts);
    lSBRRecord.SetSBIN_PF('P');
    lSBRRecord.Write(lStdfFile);
    // Write bad one
    lSBRRecord.SetSBIN_NUM(2);
    lSBRRecord.SetSBIN_CNT(lBadParts);
    lSBRRecord.SetSBIN_PF('F');
    lSBRRecord.Write(lStdfFile);
    // Write Failure one
    lSBRRecord.SetSBIN_NUM(3);
    lSBRRecord.SetSBIN_CNT(lFailedParts);
    lSBRRecord.SetSBIN_PF('F');
    lSBRRecord.Write(lStdfFile);

    // Write HBR
     // Write good one
    GQTL_STDF::Stdf_HBR_V4 lHBRRecord;
    lHBRRecord.SetHEAD_NUM(255);
    lHBRRecord.SetSITE_NUM(1);
    lHBRRecord.SetHBIN_NUM(1);
    lHBRRecord.SetHBIN_CNT(lGoodParts);
    lHBRRecord.SetHBIN_PF('P');
    lHBRRecord.Write(lStdfFile);
    // Write the bad one
    lHBRRecord.SetHBIN_NUM(2);
    lHBRRecord.SetHBIN_CNT(lBadParts);
    lHBRRecord.SetHBIN_PF('F');
    lHBRRecord.Write(lStdfFile);
    // Write the failure one
    lHBRRecord.SetHBIN_NUM(3);
    lHBRRecord.SetHBIN_CNT(lFailedParts);
    lHBRRecord.SetHBIN_PF('F');
    lHBRRecord.Write(lStdfFile);

    // Write WRR
    mWRRRecord.SetHEAD_NUM(1);
    mWRRRecord.SetFINISH_T(mStartTime);
    mWRRRecord.SetSITE_GRP(255);
    mWRRRecord.SetPART_CNT(lGoodParts + lBadParts + lFailedParts);
    mWRRRecord.SetGOOD_CNT(lGoodParts);
    mWRRRecord.SetMASK_ID(mMIRRecord.m_cnPART_TYP.mid(0, mMIRRecord.m_cnPART_TYP.size() - 1));
    mWRRRecord.Write(lStdfFile);

    // Write MRR
    mMRRRecord.SetFINISH_T(mStartTime);
    mMRRRecord.Write(lStdfFile);

    // Close STDF file.
    lStdfFile.Close();

    // Success
    return true;
}

}
}
