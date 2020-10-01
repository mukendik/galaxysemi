//////////////////////////////////////////////////////////////////////
// import_semi_g85.cpp: Convert a SEMI_G85 file to STDF V4.0
//////////////////////////////////////////////////////////////////////
#include "importTriQuintDC.h"
#include "importConstants.h"
#include <gqtl_log.h>
#include <QFileInfo>
#include <QDir>

//Circuit type	Lot	Wafer	Rw	Cl	"Q1,BVGD10,V"	"Q1,VP,V"	"R7,RESISTOR,OHMS"	Anal dt
//EG6717A	1324211	4	41	27	-0.49013	-1.0682	98.529	9/21/2013 22:55
//EG6717A	1324211	4	32	41	-19.682	-1.0444	100.52	9/21/2013 22:55
//EG6717A	1324211	4	32	42	-19.609	-1.0444	100.73	9/21/2013 22:55
//EG6717A	1324211	4	32	43	-19.882	-1.0207	100.94	9/21/2013 22:55
//EG6717A	1324211	4	32	44	-19.664	-1.0444	101.12	9/21/2013 22:55
//EG6717A	1324211	3	33	6	-19.241	-1.0207	101.39	9/21/2013 22:55
//EG6717A	1324211	3	33	7	-19.492	-1.0207	101.12	9/21/2013 22:55

#define FIRST_TEST_COLUMN 5

namespace GS
{
namespace Parser
{

//////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////
TriQuintDCToSTDF::TriQuintDCToSTDF() : ParserBase(typeTriQuintDC, "typeTriQuintDC")
{
    mTestParams.clear();
    mParameterDirectory.SetFileName(GEX_TRI_QUINT_DC_PARAMETERS);
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
TriQuintDCToSTDF::~TriQuintDCToSTDF()
{
}

//////////////////////////////////////////////////////////////////////
// Check if File is compatible with EGL Skywork format
//////////////////////////////////////////////////////////////////////
bool TriQuintDCToSTDF::IsCompatible(const QString &FileName)
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

    //Circuit type	Lot	Wafer	Rw	Cl	"Q1,BVGD10,V"	"Q1,VP,V"	"R7,RESISTOR,OHMS"	Anal dt
    if (lStrString.startsWith("Circuit type", Qt::CaseInsensitive)
        && lStrString.contains("Lot", Qt::CaseInsensitive)
        && lStrString.contains("Wafer", Qt::CaseInsensitive)
        && lStrString.contains("Anal dt", Qt::CaseInsensitive))
    {
        lIsCompatible = true;
    }
    else
    {
        lIsCompatible =false;
    }

    // Close file
    lFile.close();

    return lIsCompatible;
}


//////////////////////////////////////////////////////////////////////
bool TriQuintDCToSTDF::ConvertoStdf(const QString &triQuintDCFileName, QString &stdfFileName)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString(" file %1").arg( triQuintDCFileName).toLatin1().constData());
    QString lStrString("");

    // Open TriQuintDC file
    QFile lFile(triQuintDCFileName);
    if(!lFile.open( QIODevice::ReadOnly ))
    {
        // Failed Opening TriQuintDC file
        mLastError = errOpenFail;

        // Convertion failed.
        return false;
    }
    // Assign file I/O stream
    QTextStream lInputFile(&lFile);

    // Read the first non-empty line
    while (lStrString.isEmpty() && !lInputFile.atEnd())
    {
        lStrString = ReadLine(lInputFile);
    }

    // Just recheck that the file is compatible.
    //Circuit type	Lot	Wafer	Rw	Cl	"Q1,BVGD10,V"	"Q1,VP,V"	"R7,RESISTOR,OHMS"	Anal dt
    if (!lStrString.startsWith("Circuit type", Qt::CaseInsensitive)
        || !lStrString.contains("Lot", Qt::CaseInsensitive)
        || !lStrString.contains("Wafer", Qt::CaseInsensitive)
        || !lStrString.contains("Anal dt", Qt::CaseInsensitive))
    {
        // Close file
        lFile.close();
        mLastError = errInvalidFormatParameter;
        GSLOG(SYSLOG_SEV_ERROR, "Invalid header format");
        // Fail parsing TriQuintDC file
        return false;
    }


    // Read the test names and units
    // Circuit type	Lot	Wafer	Rw	Cl	"Q1,BVGD10,V"	"Q1,VP,V"	"R7,RESISTOR,OHMS"	Anal dt
    QStringList lFields = lStrString.split("\t");
    ParserParameter lTestParam;
    for (int lIndex=FIRST_TEST_COLUMN; lIndex<lFields.size()-1; ++lIndex)
    {
        QString lTestName = (lFields[lIndex].remove("\"")).section(",", 0, 1);
        lTestParam.SetTestName(lTestName);
        lTestParam.SetTestUnit((lFields[lIndex].remove("\"")).section(",", 2));
        long lTestNumber = mParameterDirectory.UpdateParameterIndexTable(lTestName);
        lTestParam.SetTestNumber(lTestNumber);
        mTestParams.append(lTestParam);
    }


    if(WriteStdfFile(lInputFile, stdfFileName, mParameterDirectory) != true)
    {

        RemoveOutputFiles();
        // Close file
        lFile.close();
        return false;
    }

    // Close file
    lFile.close();

    mLastError = errNoError;
    // Success parsing TriQuintDC file
    return true;
}

//////////////////////////////////////////////////////////////////////
/// Create STDF file from TriQuintDC data parsed
/// The particularity of this format is that the date are inversed: the first one is in top bottom
/// and the last one is in the top
/// Ex: first line 12/05/2014 09:08
///     last line  12/05/2014 12:15
//////////////////////////////////////////////////////////////////////
bool TriQuintDCToSTDF::WriteStdfFile(QTextStream &inputFile,
                                     const QString &StdfFileName,
                                     ParameterDictionary& lParameterDirectory)
{
    // Loop on the test line, if the wafer id is different from the previous one, write a new stdf file
    unsigned short lLastWafer(0);
    // now generate the STDF file...
    GS::StdLib::Stdf lStdfFile;
    QString lOutputFile;
    QDateTime lDate;
    QString lLine("");
    // We have to read lines in inverse order. So, we have to store them in a list
    QStringList lAllLines;

    while (!inputFile.atEnd())
    {
        // EG6717A	1324211	4	41	27	-0.49013	-1.0682	98.529	9/21/2013 22:55
        lLine = ReadLine(inputFile);
        QStringList lFields = lLine.split("\t");
        // LL						-20.5	-1.1	99
        if (lLine.startsWith("LL"))
        {
            lFields = lLine.split("\t");
            for (int lLimitIndex=FIRST_TEST_COLUMN+1; lLimitIndex<lFields.size(); ++lLimitIndex)
            {
                if ((mTestParams.size() >= (lFields.size() - (FIRST_TEST_COLUMN+1)))
                    && !lFields[lLimitIndex].isEmpty())
                {
                    mTestParams[lLimitIndex - (FIRST_TEST_COLUMN+1)].SetLowLimit(lFields[lLimitIndex].toDouble());
                    mTestParams[lLimitIndex - (FIRST_TEST_COLUMN+1)].SetValidLowLimit(true);
                }
            }
            continue;
        }

        // HL						-20.5	-1.1	99
        if (lLine.startsWith("HL"))
        {
            lFields = lLine.split("\t");
            for (int lLimitIndex=FIRST_TEST_COLUMN+1; lLimitIndex<lFields.size(); ++lLimitIndex)
            {
                if ((mTestParams.size() >= (lFields.size() - (FIRST_TEST_COLUMN+1)))
                    && !lFields[lLimitIndex].isEmpty())
                {
                    mTestParams[lLimitIndex - (FIRST_TEST_COLUMN+1)].SetHighLimit(lFields[lLimitIndex].toDouble());
                    mTestParams[lLimitIndex - (FIRST_TEST_COLUMN+1)].SetValidHighLimit(true);
                }
            }
            continue;
        }

        if (lFields.size() <= FIRST_TEST_COLUMN)
        {
            GSLOG(SYSLOG_SEV_ERROR, QString("Invalid number of parameters in this line%1")
                  .arg(lLine).toLatin1().constData());
            return false;
        }

        // New wafer
        if (lLastWafer != lFields[2].toUShort())
        {
            if (!mOutputFiles.empty())
            {
                // read the time.
                QStringList lTests = lAllLines[lAllLines.size()-1].split("\t");
                if (!GetDateFromString(lTests[8], lDate))
                {
                    // Close file
                    lStdfFile.Close();
                    mLastError = errInvalidFormatParameter;
                    GSLOG(SYSLOG_SEV_ERROR, QString("Invalid date format %1")
                          .arg(lTests[8]).toLatin1().constData());
                    return false;
                }

                mStartTime = lDate.toTime_t();

                if (!WriteStdfFile(lStdfFile, lAllLines))
                {
                    return false;
                }

                lAllLines.clear();
                lAllLines.append(lLine);
            }
            // If not the first file, close the open one
            // else open it
            lLastWafer = lFields[2].toInt();
            lOutputFile = QFileInfo(StdfFileName).absolutePath() + QDir::separator();
            lOutputFile += QFileInfo(StdfFileName).baseName() + "_" + QString::number(lLastWafer);
            lOutputFile += "." + QFileInfo(StdfFileName).completeSuffix();
            mOutputFiles.append(lOutputFile);
            if(lStdfFile.Open(lOutputFile.toLatin1().constData(),STDF_WRITE) != GS::StdLib::Stdf::NoError)
            {
                // Failed importing TriQuintDC file into STDF database
                mLastError = errWriteSTDF;
                RemoveOutputFiles();
                // Convertion failed.
                return false;
            }
        }
        else
        {
            lAllLines.append(lLine);
        }
    }

    // Write the last stdf
    QStringList lTests = lAllLines[lAllLines.size()-1].split("\t");
    if (lTests.size() > 8 && !GetDateFromString(lTests[8], lDate))
    {
        // Close file
        lStdfFile.Close();
        mLastError = errInvalidFormatParameter;
        GSLOG(SYSLOG_SEV_ERROR, QString("Invalid date format %1")
              .arg(lTests[8]).toLatin1().constData());
        return false;
    }

    mStartTime = lDate.toTime_t();

    if (!WriteStdfFile(lStdfFile, lAllLines))
    {
        return false;
    }

    // All tests names read...check if need to update the TriQuintRF Parameter list on disk?
    if(lParameterDirectory.GetNewParameterFound() == true)
        lParameterDirectory.DumpParameterIndexTable();

    // Close STDF file.
    lStdfFile.Close();

    // Success
    return true;
}


bool TriQuintDCToSTDF::GetDateFromString(QString dateString, QDateTime& dataTime)
{
    QString timeString = dateString.section(" ", 1, 1);
    QStringList lDateList = dateString.section(" ", 0, 0).split('/');
    QDate lDate = QDate(1900, 1, 1);
    if (lDateList.count() == 3)
    {
        int lYear = lDateList[2].toInt();
        if (lYear < 70)
            lYear += 2000;
        else if (lYear < 99)
            lYear += 1900;
        lDate.setDate(lYear, lDateList[0].toInt(), lDateList[1].toInt());
    }
    else
    {
        // Fail parsing TriQuintDC file
        return false;
    }
    QTime lTime = QTime::fromString(timeString, "hh:mm");
    dataTime = QDateTime(lDate, lTime,Qt::UTC);
    return true;
}

bool TriQuintDCToSTDF::WriteStdfFile(GS::StdLib::Stdf& lStdfFile, QStringList& lAllLines)
{
    GQTL_STDF::Stdf_SBR_V4 lSBRRecord;
    GQTL_STDF::Stdf_HBR_V4 lHBRRecord;
    GQTL_STDF::Stdf_WRR_V4 lWRRRecord;
    GQTL_STDF::Stdf_WIR_V4 lWIRRecord;
    GQTL_STDF::Stdf_PIR_V4 lPIRRecord;
    GQTL_STDF::Stdf_PRR_V4 lPRRRecord;
    GQTL_STDF::Stdf_PTR_V4 lPTRRecord;
    GQTL_STDF::Stdf_MRR_V4 lMRRRecord;
    GQTL_STDF::Stdf_MIR_V4 lMIRRecord;
    unsigned int lLastTime(0), lGoodParts(0);
    QStringList lTests = lAllLines[lAllLines.size()-1].split("\t");

    // Create FAR
    GQTL_STDF::Stdf_FAR_V4 lFARrecord;

    // Add undefined values in the TriQuintDC file and mandatory fields to the STDF
    lMIRRecord.SetSTAT_NUM(1);
    lMIRRecord.SetMODE_COD(' ');
    lMIRRecord.SetRTST_COD(' ');
    lMIRRecord.SetPROT_COD(' ');
    lMIRRecord.SetBURN_TIM(65535);
    lMIRRecord.SetCMOD_COD(' ');
    lMIRRecord.SetTSTR_TYP("");
    lMIRRecord.SetNODE_NAM("");
    lMIRRecord.SetTSTR_TYP("");
    lMIRRecord.SetJOB_NAM("");
    lMIRRecord.SetJOB_REV("");
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    strUserTxt += ":TRIQUINTDC";
    lMIRRecord.SetUSER_TXT(strUserTxt.toLatin1().constData());


    // Define inchangeable information
    lWIRRecord.SetHEAD_NUM(1);
    lWIRRecord.SetSITE_GRP(255);

    lPIRRecord.SetHEAD_NUM(1);
    lPIRRecord.SetSITE_NUM(1);

    lPTRRecord.SetHEAD_NUM(1);
    lPTRRecord.SetSITE_NUM(1);
    lPTRRecord.SetTEST_FLG(0x40); // Test completed with no pass/fail indication
    lPTRRecord.SetPARM_FLG(0);

    lPRRRecord.SetHEAD_NUM(1);
    lPRRRecord.SetSITE_NUM(1);

    QDateTime lDate;

    // Write FAR
    // Ensure CPU type (for encoding STDF records) is the one of the computer platform running the codes.
    lStdfFile.SetStdfCpuType(lStdfFile.GetComputerCpuType());
    lFARrecord.SetCPU_TYPE(lStdfFile.GetComputerCpuType());  // Force CPU type to current computer platform.
    lFARrecord.SetSTDF_VER(4);                               // STDF V4
    lFARrecord.Write(lStdfFile);

    // Write MIR
    lMIRRecord.SetSTART_T(mStartTime);
    lMIRRecord.SetSETUP_T(mStartTime);
    lMIRRecord.SetPART_TYP(lTests[0]);
    lMIRRecord.SetLOT_ID(lTests[1]);
    lMIRRecord.Write(lStdfFile);

    // Write WIR
    lWIRRecord.SetSTART_T(mStartTime);
    QString lWaferID;
    if (lTests[2].size() == 1)
    {
        lWaferID = "0" + lTests[2];
    }
    else
    {
        lWaferID = lTests[2];
    }

//    bool lOk;
//    int lWaferInt = lTests[2].toInt(&lOk);
//    if(lOk && lWaferInt < 9)
//    {
//        lWaferID = "0" + lTests[2];
//    }
//    else
//    {
//        lWaferID = lTests[2];
//    }
    lWIRRecord.SetWAFER_ID(lWaferID);
    lWIRRecord.Write(lStdfFile);

    unsigned int lTotalTests(0);
    // Read the list in the inverse order
    for (int lIndex=lAllLines.size()-1; lIndex>=0; --lIndex)
    {
        lTests = lAllLines[lIndex].split("\t");
        // Write PIR
        lPIRRecord.Write(lStdfFile);
        bool lDieFail(false), lTestFail(false);

        // Write PTR
        ParserParameter lTestParam;
        for (int lTestIndex=FIRST_TEST_COLUMN; lTestIndex<lTests.size()-1; ++lTestIndex)
        {
            lTestFail = false;
            lTestParam = mTestParams[lTestIndex-FIRST_TEST_COLUMN];
            lPTRRecord.SetTEST_NUM(lTestParam.GetTestNumber());
            lPTRRecord.SetRESULT(lTests[lTestIndex].toFloat());
            lPTRRecord.SetTEST_TXT(lTestParam.GetTestName());
            lPTRRecord.SetUNITS(lTestParam.GetTestUnits());
            BYTE lValidLimit = 0x0e;
            if (lTestParam.GetValidLowLimit())
            {
                lPTRRecord.SetLO_LIMIT(lTestParam.GetLowLimit());
                if (lTests[lTestIndex].toDouble() < lTestParam.GetLowLimit())
                {
                    lTestFail = true;
                }
            }
            else
            {
                lValidLimit |=0x40;
            }
            if (lTestParam.GetValidHighLimit())
            {
                lPTRRecord.SetHI_LIMIT(lTestParam.GetHighLimit());
                if (lTests[lTestIndex].toDouble() > lTestParam.GetHighLimit())
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
                lPTRRecord.SetTEST_FLG(0x80);
                lDieFail = true;
            }
            else
            {
                lPTRRecord.SetTEST_FLG(0x0);
            }
            lPTRRecord.SetOPT_FLAG(lValidLimit);
            lPTRRecord.Write(lStdfFile);
            ++lGoodParts;
        }

        // Write PRR
        lPRRRecord.SetNUM_TEST(lTotalTests);
        if (lDieFail == true)
        {
            lPRRRecord.SetHARD_BIN(2);
            lPRRRecord.SetSOFT_BIN(2);
            //bit 3: 1=Partfailed
            lPRRRecord.SetPART_FLG(0x08);
        }
        else
        {
            lPRRRecord.SetHARD_BIN(1);
            lPRRRecord.SetSOFT_BIN(1);
            lPRRRecord.SetPART_FLG(0x0);
        }
        lPRRRecord.SetX_COORD(lTests[4].toInt());
        lPRRRecord.SetY_COORD(lTests[3].toInt());
        // Calculate the difference between the previous line and the current one
        if (lTests.size() > 8 && !GetDateFromString(lTests[8], lDate))
        {
            // Close file
            lStdfFile.Close();
            mLastError = errInvalidFormatParameter;
            GSLOG(SYSLOG_SEV_ERROR, QString("Invalid date format %1")
                  .arg(lTests[8]).toLatin1().constData());
            return false;
        }
        // For the last line, we don't have a reference.
        if (lIndex != lAllLines.size()-1)
        {
            lPRRRecord.SetTEST_T(lLastTime - lDate.toTime_t());
        }
        else
        {
            lPRRRecord.SetTEST_T(0);
        }
        lLastTime = lDate.toTime_t();
        lPRRRecord.SetPART_ID(lTests[4] + "." + lTests[3]);
        lPRRRecord.Write(lStdfFile);
    }

    // Write SBR
    lSBRRecord.SetHEAD_NUM(255);
    lSBRRecord.SetSITE_NUM(1);
    lSBRRecord.SetSBIN_NUM(1);
    lSBRRecord.SetSBIN_CNT(lGoodParts);
    lSBRRecord.SetSBIN_PF('P');
    lSBRRecord.Write(lStdfFile);

    // Write HBR
    lHBRRecord.SetHEAD_NUM(255);
    lHBRRecord.SetSITE_NUM(1);
    lHBRRecord.SetHBIN_NUM(1);
    lHBRRecord.SetHBIN_CNT(lGoodParts);
    lHBRRecord.SetHBIN_PF('P');
    lHBRRecord.Write(lStdfFile);

    // Write TSRs, SBRs and HBRs
    // Write TSR
    GQTL_STDF::Stdf_TSR_V4 lTSRRecord;
    lTSRRecord.SetHEAD_NUM(255);
    for (unsigned short lIndex=0; lIndex<mTestParams.size(); ++lIndex)
    {
        ParserParameter lParmer = mTestParams[lIndex];
        lTSRRecord.SetHEAD_NUM(255);
        lTSRRecord.SetSITE_NUM(1);
        lTSRRecord.SetTEST_TYP('P');
            lTSRRecord.SetTEST_NUM(lParmer.GetTestNumber());
            lTSRRecord.SetEXEC_CNT(lParmer.GetExecCount());
            lTSRRecord.SetFAIL_CNT(0);
            lTSRRecord.SetTEST_NAM(lParmer.GetTestName());
            lTSRRecord.Write(lStdfFile);
        }

        // Write WRR
        lWRRRecord.SetHEAD_NUM(1);
        lWRRRecord.SetSITE_GRP(255);
        lWRRRecord.SetFINISH_T(mStartTime);
        lWRRRecord.SetPART_CNT(lGoodParts);
        lWRRRecord.SetGOOD_CNT(lGoodParts);
        lWRRRecord.SetWAFER_ID(lWaferID);
        lWRRRecord.Write(lStdfFile);

        lMRRRecord.SetFINISH_T(mStartTime);
        lMRRRecord.SetDISP_COD(' ');
        lMRRRecord.Write(lStdfFile);
        lStdfFile.Close();

        return true;
    }


}
}
