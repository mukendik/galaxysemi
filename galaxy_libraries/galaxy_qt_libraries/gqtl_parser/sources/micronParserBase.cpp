#include "micronParserBase.h"

namespace GS
{
namespace Parser
{

MicronParserBase::MicronParserBase(ParserType lType, const QString &lName) : ParserBase(lType, lName)
{
}

MicronParserBase::~MicronParserBase()
{
    mPatBins.clear();
}

bool MicronParserBase::ReadHeaderSection(QTextStream &lInputFileStream)
{
    QString lStrString("");
    short lReadLine(0);
    while (lStrString.compare("$$_END_HEADER") != 0
           && !lInputFileStream.atEnd()
           && lReadLine < 200)
    {
        if (lStrString.startsWith("DESIGN_ID", Qt::CaseInsensitive))
            mMIRRecord.SetDSGN_REV(lStrString.section(": ", 1));
        else if (lStrString.startsWith("ENG_CONTACT", Qt::CaseInsensitive))
            mMIRRecord.SetENG_ID(lStrString.section(": ", 1));
        else if (lStrString.startsWith("FAB", Qt::CaseInsensitive))
            mMIRRecord.SetFLOOR_ID(lStrString.section(": ", 1));
        else if (lStrString.startsWith("FINISH_ETIME", Qt::CaseInsensitive))
            mMRRRecord.SetFINISH_T(lStrString.section(": ", 1).toDouble());
        else if (lStrString.startsWith("LOG_REV", Qt::CaseInsensitive))
            mMIRRecord.SetUSER_TXT(lStrString.section(": ", 1));
        else if (lStrString.startsWith("LOT_ID", Qt::CaseInsensitive))
            mMIRRecord.SetLOT_ID(lStrString.section(": ", 1));
        else if (lStrString.startsWith("OPERATOR", Qt::CaseInsensitive))
            mMIRRecord.SetOPER_NAM(lStrString.section(": ", 1));
//        else if (lStrString.startsWith("PARTNER", Qt::CaseInsensitive))
//            mMIRRecord.SetEXTRA_ID(lStrString.section(": ", 1));
        else if (lStrString.startsWith("PART_TYPE", Qt::CaseInsensitive))
            mMIRRecord.SetPART_TYP(lStrString.section(": ", 1));
        else if (lStrString.startsWith("PRB_CARD", Qt::CaseInsensitive))
            mSDRRecord.SetCARD_ID(lStrString.section(": ", 1));
        else if (lStrString.startsWith("PRB_FACILITY", Qt::CaseInsensitive))
            mMIRRecord.SetFACIL_ID(lStrString.section(": ", 1));
        else if (lStrString.startsWith("PROCESS_ID", Qt::CaseInsensitive))
            mMIRRecord.SetPROC_ID(lStrString.section(": ", 1));
        else if (lStrString.startsWith("PROGRAM", Qt::CaseInsensitive))
            mMIRRecord.SetJOB_NAM(lStrString.section(": ", 1));
        else if (lStrString.startsWith("START_ETIME", Qt::CaseInsensitive))
        {
//            QStringList lDateList = lFields[15].split('/');
//            QDate lDate(1900, 1, 1);
//            if (lDateList.count() == 3)
//            {
//                int lYear = lDateList[2].toInt();
//                if (lYear < 70)
//                    lYear += 2000;
//                else if (lYear < 99)
//                    lYear += 1900;

//                lDate.setDate(lYear, lDateList[0].toInt(), lDateList[1].toInt());
//            }
//            QTime lTime = QTime::fromString(lFields[16], "hh:mm:ss");
//            mStartTime = QDateTime(lDate, lTime,Qt::UTC).toTime_t();
            mMIRRecord.SetSTART_T(lStrString.section(": ", 1).toULong());
            mMIRRecord.SetSETUP_T(lStrString.section(": ", 1).toULong());
            mWIRRecord.SetSTART_T(lStrString.section(": ", 1).toULong());
        }
        else if (lStrString.startsWith("STATION_NAME", Qt::CaseInsensitive))
            mMIRRecord.SetNODE_NAM(lStrString.section(": ", 1));
        else if (lStrString.startsWith("TEMP", Qt::CaseInsensitive))
            mMIRRecord.SetTST_TEMP(lStrString.section(": ", 1));
        else if (lStrString.startsWith("TESTER", Qt::CaseInsensitive))
        {
            mMIRRecord.SetTSTR_TYP(lStrString.section(": ", 1));
//            mMIRRecord.SetSTAT_NUM(lStrString.section(": ", 1));
        }
        else if (lStrString.startsWith("WAF_SIZE", Qt::CaseInsensitive))
            mWCRRecord.SetWAFR_SIZ(lStrString.section(": ", 1).toDouble());
        else if (lStrString.startsWith("WAFER_ID", Qt::CaseInsensitive)
                 || lStrString.startsWith("WAFER:", Qt::CaseInsensitive))
        {
            mWIRRecord.SetWAFER_ID(lStrString.section(": ", 1));
            mWRRRecord.SetWAFER_ID(lStrString.section(": ", 1));
        }
        else if (lStrString.startsWith("FINISH_DATETIME", Qt::CaseInsensitive))
        {
            QStringList lDateList = lStrString.section(": ", 1).section(" ", 0).split('/');
            QDate lDate(1900, 1, 1);
            if (lDateList.count() == 3)
            {
                int lYear = lDateList[2].toInt();
                if (lYear < 70)
                    lYear += 2000;
                else if (lYear < 99)
                    lYear += 1900;

                lDate.setDate(lYear, lDateList[0].toInt(), lDateList[1].toInt());
            }
            QTime lTime = QTime::fromString(lStrString.section(": ", 1).section(" ", 1), "hh:mm:ss");
            time_t lFinishTime = QDateTime(lDate, lTime,Qt::UTC).toTime_t();
            mMRRRecord.SetFINISH_T(lFinishTime);
            mWRRRecord.SetFINISH_T(lFinishTime);
        }
        else if (lStrString.startsWith("GOOD_DIE", Qt::CaseInsensitive))
            mWRRRecord.SetGOOD_CNT(lStrString.section(": ", 1).toDouble());
        else if (lStrString.startsWith("TOTAL_DIE", Qt::CaseInsensitive))
            mWRRRecord.SetPART_CNT(lStrString.section(": ", 1).toDouble());

        lStrString = ReadLine(lInputFileStream);
    }
    return true;
}


bool MicronParserBase::ReadTestsDefinitionSection( QTextStream &lInputFile,
                                                   ParameterDictionary &parameterDirectory,
                                                   QList< QPair<unsigned int, QString> >& testList,
                                                   const QString keyWord)
{
    int lTestNumber;
    QString lName;
    QString lLine(keyWord);
    while (!lInputFile.atEnd()
           && !lLine.startsWith("^"))
    {
        lLine = ReadLine(lInputFile);

        // Test definitions must start with "#define" keyword
        if (lLine.startsWith("#define", Qt::CaseInsensitive) &&
            lLine.contains(keyWord, Qt::CaseInsensitive))
        {
            lName = lLine.section(" ", 3, 3);
            lTestNumber = parameterDirectory.UpdateParameterIndexTable(lName);
            testList.append(QPair<unsigned int, QString>(lTestNumber, lName));
        }
    }
    return true;
}

bool MicronParserBase::ReadSummarySection(QTextStream &lInputFileStream,
                                          QList<GQTL_STDF::Stdf_SBR_V4*>& lSBRRecordList,
                                          QMap<unsigned short, GQTL_STDF::Stdf_HBR_V4*>& lHBRRecordList)
{
    QString lLine("");
    bool lHardSummaryFound(false), lSoftSummaryFound(false);
    bool lAllSummaryFound(false);
    QMap<unsigned short, ParserBinning> lParseBins;

    while(!lInputFileStream.atEnd()
          && !lAllSummaryFound
          && !lLine.startsWith("$$_END_SUMMARY"))
    {
        lLine = ReadLine(lInputFileStream).trimmed();

        // #define FM_BIN_DEF H FAIL BIN_H 00 (REG# 100) Opens, Inputs
        if (lLine.contains("FM_BIN_DEF", Qt::CaseInsensitive)
            || lLine.contains("EC_BIN_DEF", Qt::CaseInsensitive))
        {
            QStringList lElt = lLine.split(" ");
            ParserBinning lParseBin;
            bool lOk;
            unsigned short lBinNum;
            // if the line is for a PAT failure, get the test number from the REG field
            // #define FM_BIN_DEF DPAT FAIL BIN_DPAT 00 (REG# 141) Dynamic PPAT fail
            if (lLine.contains("PAT FAIL"))
            {
                // convert the bin from the base 10 to the base 16
                QString lRegField = lElt[7].remove(")");
                lBinNum = lRegField.toUShort();
                lRegField = QString::number(lBinNum, 16);
                lBinNum = lRegField.toUShort(&lOk, 16);
                mPatBins.insert(lElt[2], lBinNum);
            }
            else
            {
                lBinNum = lElt[2].toLatin1().toHex().toUShort(&lOk,16);
            }
            if (lElt[3].compare("PASS",Qt::CaseInsensitive) == 0)
            {
                lParseBin.SetBinName("BIN_1");
                lParseBin.SetPassFail(true);
                lBinNum = 1;
            }
            else
            {
                lParseBin.SetBinName(lElt[4]);
                lParseBin.SetPassFail(false);
            }
            lParseBin.SetBinNumber(lBinNum);

            lParseBins.insert(lBinNum, lParseBin);
            lLine = lLine.section(" ", 2);
        }


        // read the hard bin
        // a hard bin can be used for multiple soft bins
        // '.:*',2583,
        if (lLine.startsWith("%thiswaf_binbit", Qt::CaseInsensitive))
        {
            QStringList lFields;
            bool lOk;
            while (!lLine.contains(");"))
            {
                lFields = lLine.split(",");
                if (lFields.size() >= 2)
                {
                    QString lBinString = lFields[0].replace("'", "").section(":",0,0);
                    unsigned short lBinNum;
                    if (lBinString.compare(".") == 0 || lBinString.compare("*") == 0)
                    {
                        lBinNum = 1;
                    }
                    else if (lBinString.contains("PAT"))
                    {
                        lBinNum = mPatBins.value(lBinString);
                    }
                    else
                    {
                        lBinNum = lBinString.toLatin1().toHex().toUShort(&lOk,16);
                    }
                    if (!lHBRRecordList.contains(lBinNum))
                    {
                        GQTL_STDF::Stdf_HBR_V4* lHBRRecord = new GQTL_STDF::Stdf_HBR_V4();
                        lHBRRecord->SetHEAD_NUM(1);
                        lHBRRecord->SetSITE_NUM(1);
                        lHBRRecord->SetHBIN_NUM(lBinNum);
                        lHBRRecord->SetHBIN_CNT((lFields[1]).toULong());
                        if (lParseBins.contains(lBinNum))
                        {
                            lHBRRecord->SetHBIN_NAM(lParseBins.value(lBinNum).GetBinName());
                            if (lParseBins.value(lBinNum).GetPassFail())
                                lHBRRecord->SetHBIN_PF('P');
                            else
                                lHBRRecord->SetHBIN_PF('F');
                        }
                        else
                        {
                            lHBRRecord->SetHBIN_PF(' ');
                        }
                        lHBRRecordList.insert(lBinNum, lHBRRecord);
                    }
                    else
                    {
                        GQTL_STDF::Stdf_HBR_V4* lHBRRecord = lHBRRecordList.value(lBinNum);
                        lHBRRecord->SetHBIN_CNT(lHBRRecord->m_u4HBIN_CNT + lFields[1].toULong());
                    }
                }
                lLine = ReadLine(lInputFileStream);
            }
            lHardSummaryFound = true;
        }

        // read the soft bin
        // %thiswaf_bin = (
        // '#',0,
        // '*',2583,
        if (lLine.startsWith("%thiswaf_bin =", Qt::CaseInsensitive))
        {
            QStringList lFields;
            bool lOk;
            unsigned short lBinNum;
            bool lBinPassFound(false);
            while (!lLine.contains(");"))
            {
                lFields = lLine.split(",");
                if (lFields.size() >= 2)
                {
                    QString lBinString = lFields[0].replace("'", "");
                    if (lBinString.compare(".") == 0 || lBinString.compare("*") == 0)
                    {
                        lBinNum = 1;
                    }
                    else if (lBinString.contains("PAT"))
                    {
                        lBinNum = mPatBins.value(lBinString);
                    }
                    else
                    {
                        lBinNum = lBinString.toLatin1().toHex().toUShort(&lOk,16);
                    }
                    // if we found the pass bin (. or *) we don't write it twice
                    if (1 != lBinNum || false == lBinPassFound)
                    {
                        GQTL_STDF::Stdf_SBR_V4* lSBRRecord = new GQTL_STDF::Stdf_SBR_V4();
                        lSBRRecord->SetHEAD_NUM(1);
                        lSBRRecord->SetSITE_NUM(1);
                        lSBRRecord->SetSBIN_NUM(lBinNum);
                        lSBRRecord->SetSBIN_CNT((lFields[1]).toULong());
                        if (lParseBins.contains(lBinNum))
                        {
                            lSBRRecord->SetSBIN_NAM(lParseBins.value(lBinNum).GetBinName());
//                            if (lParseBins.value(lBinNum).GetPassFail())
//                                lSBRRecord->SetSBIN_PF('P');
//                            else
//                                lSBRRecord->SetSBIN_PF('F');
                        }
//                        else
//                        {
//                            lSBRRecord->SetSBIN_PF(' ');
//                        }
                        if (lBinNum == 1)
                            lSBRRecord->SetSBIN_PF('P');
                        else
                            lSBRRecord->SetSBIN_PF('F');
                        lSBRRecordList.append(lSBRRecord);
                        if (lBinNum == 1)
                            lBinPassFound = true;
                    }
                }
                lLine = ReadLine(lInputFileStream);
            }
            lSoftSummaryFound = true;
        }

        lAllSummaryFound = lHardSummaryFound & lSoftSummaryFound;

    }
    if (lInputFileStream.atEnd())
        return false;
    else
        return true;

}



bool MicronParserBase::ReadDieDataSection(QTextStream &lInputFile,
                                          QHash< QPair<qint16, qint16>, QStringList > &dieData,
                                          bool writeRecord,
                                          const QString keyWord,
                                          GS::StdLib::Stdf& lStdfFile,
                                          QList< QPair<unsigned int, QString> >& testList)
{
    QString lLine;

    StartProgressStatus(100, "Read Die Section");


    while (!lInputFile.atEnd())
    {

        lLine = ReadLine(lInputFile);
        if (lLine.startsWith(keyWord, Qt::CaseInsensitive))
        {
            QStringList lFields = lLine.split(" ");
            // Need to have at least 5 items in the line
            if (lFields.size() > 4)
            {
                QString lPartId = lFields[1];
                QString lXString = lPartId.section(":", 2, 2);
                qint16 lX;
                if(lXString.contains("N"))
                    lX = 0 - (lXString.remove("N")).toDouble();
                else
                    lX = (lXString.remove("P")).toDouble();
                qint16 lY = lPartId.section(":", 3, 3).toDouble();
                if (!writeRecord)
                {
                    QStringList lValues;
                    for (int i=5; i<lFields.size(); ++i)
                    {
                        lValues.append(lFields[i]);
                    }
                    QPair<qint16, qint16> lCoord = QPair<qint16, qint16>(lX, lY);
                    // If the die has been added in the previous iteration, add the results to the existing ones.
                    if (dieData.contains(lCoord))
                    {
                        QStringList lAllValues = dieData.value(lCoord);
                        lAllValues.append(lValues);
                        dieData.remove(lCoord);
                        dieData.insert(lCoord, lAllValues);
                    }
                    else
                    {
                        dieData.insert(QPair<qint16, qint16>(lX, lY), lValues);
                    }
                }
                else
                {
                    // Write PIR
                    GQTL_STDF::Stdf_PIR_V4 lPIRRecord;
                    QString lBinString = lFields[2].section("H",1).section("S",0,0);
                    unsigned short lHead;
                    if (lBinString == "." || lBinString == "*")
                        lHead = 1;
                    else
                        lHead = lBinString.toUShort();
                    lPIRRecord.SetHEAD_NUM(lHead);
                    // for the first version, set site num to 1
                    lPIRRecord.SetSITE_NUM(1);
                    lPIRRecord.Write(lStdfFile);


                    // Write PTR
                    QStringList lCurrentDieResults = dieData.value(QPair<qint16, qint16>(lX, lY));
                    unsigned int lTotalTests = lCurrentDieResults.size() + lFields[4].toUShort();
                    GQTL_STDF::Stdf_PTR_V4 lPTRRecord;
                    for (int i=0; i<lCurrentDieResults.size(); ++i)
                    {
                        if(lCurrentDieResults[i].compare("X", Qt::CaseInsensitive) != 0)
                        {
                            lPTRRecord.SetTEST_NUM(testList[i].first);
                            lPTRRecord.SetHEAD_NUM(lHead);
                            lPTRRecord.SetSITE_NUM(1);          // for the first version
                            lPTRRecord.SetTEST_FLG(0x40);
                            lPTRRecord.SetPARM_FLG(0);
                            lPTRRecord.SetRESULT(lCurrentDieResults[i].toFloat());
                            lPTRRecord.SetTEST_TXT(testList[i].second);
    //                        lPTRRecord.setALARM_ID();
    //                        lPTRRecord.setOPT_FLAG();
                            lPTRRecord.Write(lStdfFile);
                        }
                    }
                    for (unsigned short i=0; i<lFields[4].toUShort(); ++i)
                    {
                        if(lFields.size() >= (i+6)
                           && lFields[i+5].compare("X", Qt::CaseInsensitive) != 0)
                        {
                            lPTRRecord.SetTEST_NUM(testList[i+lCurrentDieResults.size()].first);
                            lPTRRecord.SetHEAD_NUM(lHead);
                            lPTRRecord.SetSITE_NUM(1);          // for the first version
                            lPTRRecord.SetTEST_FLG(0x40);
                            lPTRRecord.SetPARM_FLG(0);
                            lPTRRecord.SetRESULT(lFields[i+5].toFloat());
                            lPTRRecord.SetTEST_TXT(testList[i+lCurrentDieResults.size()].second);
    //                        lPTRRecord.setALARM_ID();
    //                        lPTRRecord.setOPT_FLAG();
                            lPTRRecord.Write(lStdfFile);
                        }
                    }



                    // Write PRR
                    GQTL_STDF::Stdf_PRR_V4 lPRRRecord;
                    lPRRRecord.SetHEAD_NUM(lHead);
                    lPRRRecord.SetSITE_NUM(1);
                    // I:L => the first part is the HBIN, the second one is the soft bin.
                    // We write the hexa code of the soft and the hard bin except the . and the *
                    QString lHbinString = lFields[3].section(":", 0, 0), lSbinString = lFields[3].section(":", 1);
                    bool lOk;
                    unsigned short lHBin, lSBin;
                    // Find the HBIN
                    if (lHbinString.compare(".") == 0 || lHbinString.compare("*") == 0)
                    {
                        lHBin = 1;
                    }
                    else if (lHbinString.contains("PAT"))
                    {
                        lPRRRecord.SetPART_FLG(0x08);
                        lHBin = mPatBins.value(lHbinString);
                    }
                    else
                    {
                        lPRRRecord.SetPART_FLG(0x08);
                        lHBin = lHbinString.toLatin1().toHex().toUShort(&lOk,16);
                    }

                    // Find the SBIN
                    if (lSbinString.compare(".") == 0 || lSbinString.compare("*") == 0)
                    {
                        lSBin = 1;
                    }
                    else if (lSbinString.contains("PAT"))
                    {
                        lSBin = mPatBins.value(lSbinString);
                    }
                    else
                    {
                        lSBin = lSbinString.toLatin1().toHex().toUShort(&lOk,16);
                    }


                    lPRRRecord.SetNUM_TEST(lTotalTests);
                    lPRRRecord.SetX_COORD(lX);
                    lPRRRecord.SetY_COORD(lY);
                    lPRRRecord.SetHARD_BIN(lHBin);
                    lPRRRecord.SetSOFT_BIN(lSBin);
                    lPRRRecord.SetPART_ID(lFields[1]);
//                    lPRRRecord.SetPART_TXT();
                    lPRRRecord.Write(lStdfFile);

                }
            }
        }
    }
    return true;

}

}
}
