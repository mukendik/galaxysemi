#include <QFile>
#include <cmath>
#include <QFileInfo>
#include <QDir>


#include "importAquantiaPCM.h"
#include "gqtl_log.h"
#include "gqtl_global.h"


namespace GS
{
namespace Parser {

AquantiaPCMtoSTDF::AquantiaPCMtoSTDF(): ParserBase(typeAquantiaPCM, "AquantiaPCM")
{

}


//////////////////////////////////////////////////////////////////////
// Check if File is compatible with AquantiaPCMtoSTDF format
// We have to check if the file name finish with .wacdata.dis or .waclimit.dis.
// After taht, check if the other file exists.
//////////////////////////////////////////////////////////////////////
bool AquantiaPCMtoSTDF::IsCompatible(const QString &FileName)
{
    bool	lIsCompatible = false;

    QString lSuffix(".wacdata.dis");
    QString lSuffixToFind(".waclimit.dis");
    QString lFileToFind;
    if (!FileName.endsWith(lSuffix))
    {
        lSuffix = ".waclimit.dis";
        lSuffixToFind = ".wacdata.dis";
        if (!FileName.endsWith(lSuffix))
            return false;
    }
    lFileToFind = FileName.section(lSuffix, 0, 0) + lSuffixToFind;

    // check if the second file exits
    if (!QFile::exists(lFileToFind))
        return false;

    QString lDataFileName, lLimitFileName;
    if (FileName.endsWith(".wacdata.dis"))
    {
        lDataFileName  = FileName;
        lLimitFileName = lFileToFind;
    }
    else
    {
        lDataFileName  = lFileToFind;
        lLimitFileName = FileName;
    }

    QFile lDataFile(lDataFileName), lLimitFile(lLimitFileName);

    // Check that the data file start with "START SECTION WET_LOT"
    if (!lDataFile.open(QIODevice::ReadOnly))
    {
        return false;
    }
    // Assign file I/O stream
    QTextStream lDatastreamFile(&lDataFile);
    // Check if first N line is the correct header
    int     lLine = 0;
    QString lString;
    while(!lDatastreamFile.atEnd() && lLine < 20)
    {
        lString = lDatastreamFile.readLine().simplified();

        if(lString.startsWith("START SECTION WET_LOT", Qt::CaseInsensitive) == false)
        {
            lIsCompatible = true;
            break;
        }
        ++lLine;
    }
    // Close file
    lDataFile.close();

    if (lIsCompatible == false)
        return false;

    // Check that the data file start with "START SECTION WET_LIMIT_HEADER"
    if (!lDataFile.open(QIODevice::ReadOnly))
    {
        return false;
    }
    // Assign file I/O stream
    QTextStream lLimitStreamFile(&lLimitFile);
    // Check if first N line is the correct header
    lLine = 0;
    while(!lLimitStreamFile.atEnd() && lLine < 20)
    {
        lString = lLimitStreamFile.readLine().simplified();

        if(lString.startsWith("START SECTION WET_LIMIT_HEADER", Qt::CaseInsensitive) == false)
        {
            lIsCompatible = true;
            break;
        }
        ++lLine;
    }
    // Close file
    lDataFile.close();

    return lIsCompatible;
}

bool AquantiaPCMtoSTDF::ConvertoStdf(const QString &inputFileName, QString &StdfFileName)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString(" file %1").arg( inputFileName).toLatin1().constData());

    QString lLimitFileName, lDataFileName;
    if (inputFileName.endsWith(".waclimit.dis"))
    {
        lLimitFileName = inputFileName;
        lDataFileName  = inputFileName.section(".waclimit.dis", 0, 0) + ".wacdata.dis";
    }
    else
    {
        lDataFileName  = inputFileName;
        lLimitFileName = inputFileName.section(".wacdata.dis", 0, 0) + ".waclimit.dis";
    }

    ////////////////////////////////////////////////////////////////////////
    /// Read limit file and fill the MIR and PTR
    QFile lLimitFile(lLimitFileName);
    // Open the limit file
    if(!lLimitFile.open( QIODevice::ReadOnly ))
    {
        // Failed Opening SkyworksIFF file
        mLastError = errOpenFail;

        // Convertion failed.
        return false;
    }
    // Assign file I/O stream
    QTextStream lStreamFile(&lLimitFile);

    QString lStrString;
    while (lStrString.compare("START SECTION WET_LIMIT_HEADER") != 0 && !lStreamFile.atEnd())
    {
        lStrString = lStreamFile.readLine().simplified();
    }
    if (!ReadLimitHeaderSection(lStreamFile))
    {
        mLastError = errInvalidFormatParameter;
        QFile::remove(StdfFileName);
        // Close file
        lLimitFile.close();
        return false;
    }

    while (lStrString.compare("START SECTION WET_LIMIT") != 0 && !lStreamFile.atEnd())
    {
        lStrString = lStreamFile.readLine().simplified();
    }

    if (!ReadLimitSection(lStreamFile))
    {
        mLastError = errInvalidFormatParameter;
        QFile::remove(StdfFileName);
        // Close file
        lLimitFile.close();
        return false;
    }

    ////////////////////////////////////////////////////////////////////////
    /// Read data file and fill STDF records
    QFile lDatatFile(lDataFileName);
    // Open the data file
    if(!lDatatFile.open( QIODevice::ReadOnly ))
    {
        // Failed Opening SkyworksIFF file
        mLastError = errOpenFail;

        // Convertion failed.
        return false;
    }
    // Assign file I/O stream
    lStreamFile.setDevice(&lDatatFile);

    // Read WET_LOT section
    while (lStrString.compare("START SECTION WET_LOT") != 0 && !lStreamFile.atEnd())
    {
        lStrString = lStreamFile.readLine().simplified();
    }
    if (!ReadWetLotSection(lStreamFile))
    {
        mLastError = errInvalidFormatParameter;
        QFile::remove(StdfFileName);
        // Close file
        lDatatFile.close();
        return false;
    }


    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ETEST;
    strUserTxt += ":PCM_AQUANTIA";
    mMIRRecord.SetUSER_TXT(strUserTxt.toLatin1().constData());	// user-txt

    // Nothing to do wth these sections: COORDINATE_DEFINITION and LOADER_OPTION
    // Read WET_EQUIPMENT section
    while (lStrString.compare("START SECTION WET_EQUIPMENT") != 0 && !lStreamFile.atEnd())
    {
        lStrString = lStreamFile.readLine().simplified();
    }
    lStrString = lStreamFile.readLine().simplified();
    if (lStrString.split("|").size() < 2)
    {
        mLastError = errInvalidFormatParameter;
        QFile::remove(StdfFileName);
        // Close file
        lDatatFile.close();
        return false;
    }
    mMIRRecord.SetNODE_NAM(lStrString.section("|", 0, 0));
    mMIRRecord.SetTSTR_TYP(lStrString.section("|", 1, 1));

    // Read WET_COMPONENT section
    while (lStrString.compare("START SECTION WET_COMPONENT") != 0 && !lStreamFile.atEnd())
    {
        lStrString = lStreamFile.readLine().simplified();
    }
    lStrString = lStreamFile.readLine().simplified();
    if (lStrString.split("|").size() < 2)
    {
        mLastError = errInvalidFormatParameter;
        QFile::remove(StdfFileName);
        // Close file
        lDatatFile.close();
        return false;
    }
    mSDRRecord.SetLOAD_ID(lStrString.section("|", 0, 0));

    // Read WET_WAFER section
    while (!lStreamFile.atEnd())
    {
        while (lStrString.compare("START SECTION WET_WAFER") != 0 && !lStreamFile.atEnd())
        {
            lStrString = lStreamFile.readLine().simplified();
        }
        if (lStreamFile.atEnd())
            return true;
        lStrString = lStreamFile.readLine().simplified();
        if (lStrString.split("|").size() < 4)
        {
            mLastError = errInvalidFormatParameter;
            QFile::remove(StdfFileName);
            // Close file
            RemoveOutputFiles();
            lDatatFile.close();
            return false;
        }
        QString lLastWafer = lStrString.section("|", 0, 0);
        mMIRRecord.SetSBLOT_ID(lLastWafer);
        mWIRRecord.SetWAFER_ID(lStrString.section("|", 3, 3));

        // Now start the writing of the stdf file
        GQTL_STDF::StdfParse lStdfParser;

        QString lOutputFile = QFileInfo(StdfFileName).absolutePath() + QDir::separator();
        lOutputFile += QFileInfo(StdfFileName).baseName() + "_" + lLastWafer;
        lOutputFile += "." + QFileInfo(StdfFileName).completeSuffix();
        mOutputFiles.append(lOutputFile);
        if(lStdfParser.Open(lOutputFile.toLatin1().constData(), STDF_WRITE) == false)
        {
            GSLOG(SYSLOG_SEV_ERROR, QString("Can't open the input output file' %1")
                                    .arg(StdfFileName).toLatin1().constData());
            mLastError = errWriteSTDF;
            RemoveOutputFiles();
            return false;
        }

        lStdfParser.WriteRecord(&mMIRRecord);

        mWIRRecord.SetHEAD_NUM(1);							// Test head
        mWIRRecord.SetSITE_GRP(255);						// Tester site (all)
        lStdfParser.WriteRecord(&mWIRRecord);
        lStdfParser.WriteRecord(&mWCRRecord);

        unsigned int lPartCount(0);
        int lPassBin(0), lFailBin(0);

        while (lStrString.compare("FINISH SECTION WET_WAFER") != 0 && !lStreamFile.atEnd())
        {
            // Read WET_SITE section
            while (lStrString.compare("START SECTION WET_SITE") != 0 && !lStreamFile.atEnd())
            {
                lStrString = lStreamFile.readLine().simplified();
            }
            lStrString = lStreamFile.readLine().simplified();
            if (lStrString.compare("FINISH SECTION WET_WAFER") == 0 || lStreamFile.atEnd())
                break;
            QStringList lSiteList = lStrString.split("|");
            if (lSiteList.size() < 3)
            {
                mLastError = errInvalidFormatParameter;
                QFile::remove(StdfFileName);
                // Close file
                RemoveOutputFiles();
                lDatatFile.close();
                return false;
            }
            mPRRRecord.SetX_COORD(lSiteList[0].toDouble());
            mPRRRecord.SetY_COORD(lSiteList[1].toDouble());
            int lSiteNum = lSiteList[2].toShort();
            mPRRRecord.SetSITE_NUM(lSiteNum);
            mPIRRecord.SetSITE_NUM(lSiteNum);
            mPIRRecord.SetHEAD_NUM(1);
            lStdfParser.WriteRecord(&mPIRRecord);

            // Read WET_RESULT section
            while (lStrString.compare("START SECTION WET_RESULT") != 0 && !lStreamFile.atEnd())
            {
                lStrString = lStreamFile.readLine().simplified();
            }
            lStrString = lStreamFile.readLine().simplified();
            bool lFail(false);
            while (lStrString.compare("FINISH SECTION WET_RESULT") != 0 && !lStreamFile.atEnd())
            {
                if (lStrString.split("|").size() < 2)
                {
                    mLastError = errInvalidFormatParameter;
                    QFile::remove(StdfFileName);
                    // Close file
                    RemoveOutputFiles();
                    lDatatFile.close();
                    return false;
                }
                GQTL_STDF::Stdf_PTR_V4 lPTRRecord;
                ParserParameter& lTest = mTestList[lStrString.section("|", 0, 0).toLong()];
                lPTRRecord.SetTEST_NUM(lTest.GetTestNumber());
                lPTRRecord.SetHEAD_NUM(1);
                lPTRRecord.SetSITE_NUM(lSiteNum);
                lPTRRecord.SetPARM_FLG(0);
                lPTRRecord.SetTEST_FLG(0);


                gsfloat64 lValue = GS::Core::NormalizeValue(lStrString.section("|", 1, 1).toDouble(), lTest.GetResultScale());
                lPTRRecord.SetRESULT(lValue);
                if ((lTest.GetValidLowLimit() && ((lTest.GetLowLimit() - lValue) > 0.00001))
                    || (lTest.GetValidHighLimit() && ((lValue - lTest.GetHighLimit()) > 0.00001)))
                {
                    lFail = true;
                    lPTRRecord.SetTEST_FLG(0x80);
                }      

                lTest.IncrementExecTest();
                if (!lTest.GetStaticHeaderWritten())
                {
                    lPTRRecord.SetTEST_TXT(lTest.GetTestName());
                    lPTRRecord.SetALARM_ID("");
                    gsuchar lOptFlg = 0x02;
                    lPTRRecord.SetRES_SCAL(lTest.GetResultScale());
                    lPTRRecord.SetLLM_SCAL(lTest.GetResultScale());
                    lPTRRecord.SetHLM_SCAL(lTest.GetResultScale());
                    if (lTest.GetValidLowLimit())
                        lPTRRecord.SetLO_LIMIT(lTest.GetLowLimit());// R*4 Low test limit value OPT_FLAGbit 4 or 6 = 1
                    else
                        lOptFlg |= 0x50;
                    if (lTest.GetValidHighLimit())
                        lPTRRecord.SetHI_LIMIT(lTest.GetHighLimit());// R*4 High test limit value OPT_FLAGbit 5 or 7 = 1
                    else
                        lOptFlg |= 0xA0;

                    lPTRRecord.SetUNITS(lTest.GetTestUnits());
                    if (lTest.GetValidLowSpecLimit())
                    {
                        lPTRRecord.SetLO_SPEC(lTest.GetLowSpecLimit());// R*4 Low specification limit value OPT_FLAGbit 2 = 1
                    }
                    else
                        lOptFlg |= 0x04;
                    if (lTest.GetValidHighSpecLimit())
                    {
                        lPTRRecord.SetHI_SPEC(lTest.GetHighSpecLimit());// R*4 High specification limit value OPT_FLAGbit 3 = 1
                    }
                    else
                        lOptFlg |= 0x08;
                    lPTRRecord.SetOPT_FLAG(lOptFlg);
                    lTest.SetStaticHeaderWritten(true);
                }
                lStdfParser.WriteRecord(&lPTRRecord);
                lStrString = lStreamFile.readLine().simplified();
            }
            if (lFail == true)
            {
                mPRRRecord.SetSOFT_BIN(2);
                mPRRRecord.SetHARD_BIN(2);
                ++lFailBin;
            }
            else
            {
                mPRRRecord.SetSOFT_BIN(1);
                mPRRRecord.SetHARD_BIN(1);
                ++lPassBin;
            }
            ++lPartCount;
            lStdfParser.WriteRecord(&mPRRRecord);
        }

        GQTL_STDF::Stdf_SBR_V4 lSBRRecord;
        lSBRRecord.SetHEAD_NUM(255);
        lSBRRecord.SetSITE_NUM(1);
        lSBRRecord.SetSBIN_NUM(1);
        lSBRRecord.SetSBIN_CNT(lPassBin);
        lSBRRecord.SetSBIN_PF('P');
        lStdfParser.WriteRecord(&lSBRRecord);

        lSBRRecord.SetSBIN_NUM(2);
        lSBRRecord.SetSBIN_CNT(lFailBin);
        lSBRRecord.SetSBIN_PF('F');
        lStdfParser.WriteRecord(&lSBRRecord);

        GQTL_STDF::Stdf_HBR_V4 lHBRRecord;
        lHBRRecord.SetHEAD_NUM(255);
        lHBRRecord.SetSITE_NUM(1);
        lHBRRecord.SetHBIN_NUM(1);
        lHBRRecord.SetHBIN_CNT(lPassBin);
        lHBRRecord.SetHBIN_PF('P');
        lStdfParser.WriteRecord(&lHBRRecord);

        lHBRRecord.SetHBIN_NUM(2);
        lHBRRecord.SetHBIN_CNT(lFailBin);
        lHBRRecord.SetHBIN_PF('F');
        lStdfParser.WriteRecord(&lHBRRecord);

        mWRRRecord.SetHEAD_NUM(1);
        mWRRRecord.SetSITE_GRP(255);
        mWRRRecord.SetPART_CNT(lPartCount);
        lStdfParser.WriteRecord(&mWRRRecord);
        lStdfParser.WriteRecord(&mMRRRecord);
        lStdfParser.Close();
    }
    return true;
}

gsbool AquantiaPCMtoSTDF::ReadLimitSection(QTextStream &lInputFileStream)
{
    QString     lStrString;
    QStringList lItemsList;
    lStrString = lInputFileStream.readLine().trimmed();
    while (lStrString.compare("FINISH SECTION WET_LIMIT") != 0 && !lInputFileStream.atEnd())
    {
        QString lLowLimit, lHighLimit;
        int lScalingFactor = 0;
        ParserParameter lTest;
        lItemsList = lStrString.split("|");
        if (lItemsList.size() < 10)
            return false;
        lTest.SetTestNumber(lItemsList[0].toLong());
        lTest.SetTestName(lItemsList[1]);
        lTest.SetTestTxt(lItemsList[2]);
        if (!lItemsList[3].isEmpty())
        {
            lLowLimit = lItemsList[3];

        }
        if(!lItemsList[4].isEmpty())
        {
            lHighLimit = lItemsList[4];
        }

        // Some units contains "/", for example um/mA, we don't have to scale them
        if(lItemsList[9].contains("/"))
            lTest.SetTestUnit(lItemsList[9]);
        else
            lTest.SetTestUnit(GS::Core::NormalizeUnit(lItemsList[9], lScalingFactor));
        lTest.SetResultScale(lScalingFactor);

        if (lLowLimit != "")
        {
            lTest.SetLowLimit(GS::Core::NormalizeValue(lLowLimit.toDouble(), lScalingFactor));
            lTest.SetValidLowLimit(true);
        }
        if (lHighLimit != "")
        {
            lTest.SetHighLimit(GS::Core::NormalizeValue(lHighLimit.toDouble(), lScalingFactor));
            lTest.SetValidHighLimit(true);
        }
        mTestList.insert(lItemsList[0].toLong(), lTest);
        lStrString = lInputFileStream.readLine().trimmed();
    }

    return true;
}


gsbool AquantiaPCMtoSTDF::ReadLimitHeaderSection(QTextStream &lInputFileStream)
{
    QString lStrString("");
    lStrString = lInputFileStream.readLine().trimmed();
    int lIndex(0);

    while (lStrString.compare("FINISH SECTION WET_LIMIT_HEADER") != 0 && !lInputFileStream.atEnd())
    {
        switch(lIndex)
        {
        case 0:
            mMIRRecord.SetFACIL_ID(lStrString);
            break;
        case 1:
            mMIRRecord.SetSPEC_NAM(lStrString);
            break;
        case 2:
            mMIRRecord.SetSPEC_VER(lStrString);
            break;
        case 3:
            break;
        default:
            break;
        }
        lStrString = lInputFileStream.readLine().trimmed();
        ++lIndex;
    }

    if (lStrString.compare("FINISH SECTION WET_LIMIT_HEADER") != 0)
        return false;
    else
        return true;
}

gsbool AquantiaPCMtoSTDF::ReadWetLotSection(QTextStream &lInputFileStream)
{
    QString     lStrString;
    const QString cEndWetLot("FINISH SECTION WET_LOT");

    // Read lot_id
    if (!ReadLine(lInputFileStream, lStrString, cEndWetLot))
        return false;
    mMIRRecord.SetLOT_ID(lStrString);

    // Read mfg_area_name
    if (!ReadLine(lInputFileStream, lStrString, cEndWetLot))
        return false;
    mMIRRecord.SetFACIL_ID(lStrString);

    // Read mfg_step_name
    if (!ReadLine(lInputFileStream, lStrString, cEndWetLot))
        return false;
    mMIRRecord.SetTEST_COD(lStrString);

    // Read insertion_name
    if (!ReadLine(lInputFileStream, lStrString, cEndWetLot))
        return false;
    if (!ReadLine(lInputFileStream, lStrString, cEndWetLot))
        return false;
    mMIRRecord.SetFLOW_ID(lStrString);

    // Read test_lim_set_name
    if (!ReadLine(lInputFileStream, lStrString, cEndWetLot))
        return false;
    mMIRRecord.SetSPEC_NAM(lStrString);

    // Read test_lim_set_rev
    if (!ReadLine(lInputFileStream, lStrString, cEndWetLot))
        return false;
    if (lStrString != mMIRRecord.m_cnSPEC_VER)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("The Spec_ver is not the same in the limit file and the data one, %1 vs %2")
              .arg(mMIRRecord.m_cnSPEC_VER)
              .arg(lStrString).toLatin1().constData());
        return false;
    }
    mMIRRecord.SetSPEC_VER(lStrString);

    // Read program_name
    if (!ReadLine(lInputFileStream, lStrString, cEndWetLot))
        return false;
    mMIRRecord.SetJOB_REV(lStrString);

    // Read program_rev
    if (!ReadLine(lInputFileStream, lStrString, cEndWetLot))
        return false;
    if (!ReadLine(lInputFileStream, lStrString, cEndWetLot))
        return false;
    if (!ReadLine(lInputFileStream, lStrString, cEndWetLot))
        return false;
    mMIRRecord.SetJOB_REV(lStrString);

    // Read date_test_started
    if (!ReadLine(lInputFileStream, lStrString, cEndWetLot))
        return false;
    if (!ReadLine(lInputFileStream, lStrString, cEndWetLot))
        return false;
    if (!ReadLine(lInputFileStream, lStrString, cEndWetLot))
        return false;
    if (!ReadLine(lInputFileStream, lStrString, cEndWetLot))
        return false;
    if (!ReadLine(lInputFileStream, lStrString, cEndWetLot))
        return false;
    QDateTime lDataTime;
    GetDateFromString(lStrString, lDataTime);
    mMIRRecord.SetSTART_T(lDataTime.toTime_t());
    mWIRRecord.SetSTART_T(lDataTime.toTime_t());

    // Read date_test_finished
    if (!ReadLine(lInputFileStream, lStrString, cEndWetLot))
        return false;
    GetDateFromString(lStrString, lDataTime);
    mMRRRecord.SetFINISH_T(lDataTime.toTime_t());
    mWRRRecord.SetFINISH_T(lDataTime.toTime_t());

    // Read theta
    if (!ReadLine(lInputFileStream, lStrString, cEndWetLot))
        return false;
    char lC;
    switch(lStrString.toInt())
    {
    case 0:
        lC = 'U';
        break;
    case 90:
        lC = 'R';
        break;
    case 180:
        lC = 'D';
        break;
    case 270:
        lC = 'L';
        break;
    default:
        lC = 'U';
        break;
    }
    mWCRRecord.SetWF_FLAT(lC);

    // Read operator_id
    if (!ReadLine(lInputFileStream, lStrString, cEndWetLot))
        return false;
    if (!ReadLine(lInputFileStream, lStrString, cEndWetLot))
        return false;
    if (!ReadLine(lInputFileStream, lStrString, cEndWetLot))
        return false;
    if (!ReadLine(lInputFileStream, lStrString, cEndWetLot))
        return false;
    if (!ReadLine(lInputFileStream, lStrString, cEndWetLot))
        return false;
    if (!ReadLine(lInputFileStream, lStrString, cEndWetLot))
        return false;
    mMIRRecord.SetOPER_NAM(lStrString);

    // Read mes_product_id
    if (!ReadLine(lInputFileStream, lStrString, cEndWetLot))
        return false;
    mMIRRecord.SetPART_TYP(lStrString);

    return true;
}

gsbool AquantiaPCMtoSTDF::ReadLine(QTextStream& inputFile, QString& line, const QString& tag)
{
    if (!inputFile.atEnd())
        line = inputFile.readLine().trimmed();
    else
        return false;
    if (line == tag)
        return false;
    return true;
}

void AquantiaPCMtoSTDF::GetDateFromString(QString dateString, QDateTime& dataTime)
{
    // The possible formats are:
    // Dates can be in any of the following formats (Pivot year is 80, which implies the years between 1981 and 2080):
//    MM/DD/YY
//    MM/DD/YYYY
//    DD.MM.YY
//    DD.MM.YYYY
//    MM-DD-YYYY
//    YYYY-MM-DD
//    DD-MON-YY
//    DD-MON-YYYY
    // The most common one is MM-DD-YYYY
    QString timeString = dateString.section(" ", 1, 1);
    QString lDateString = dateString.section(" ", 0, 0);
    QDate lDate;
    if (lDateString.contains("-"))
    {
        if (lDateString.size() == 10)
        {
            if (lDateString.section("-", 0, 0).size() == 2)
                lDate = QDate::fromString(lDateString, "MM-dd-yyyy");
            else
                lDate = QDate::fromString(lDateString, "yyyy-MM-dd");
        }
        else
            lDate = QDate::fromString(lDateString, "dd-MM-yy");
    }
    else if (lDateString.contains("/"))
    {
        if (lDateString.size() == 10)
            lDate = QDate::fromString(lDateString, "MM/dd/yyyy");
        else
            lDate = QDate::fromString(lDateString, "MM/dd/yy");
    }
    else
        if (lDateString.contains("."))
        {
            if (lDateString.size() == 10)
                lDate = QDate::fromString(lDateString, "dd.MM.yyyy");
            else
                lDate = QDate::fromString(lDateString, "dd.MM.yy");
        }

    QTime lTime = QTime::fromString(timeString, "hh:mm:ss");
    dataTime = QDateTime(lDate, lTime,Qt::UTC);
}

}
}
