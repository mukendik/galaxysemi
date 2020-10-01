#include <QFileInfo>
#include <import_vishay_ase_qa.h>

#include "gqtl_global.h"
#include "gqtl_log.h"

#include "import_vishay_ase_qa.h"
#include "promis_interpreter_factory.h"
#include "promis_item_base.h"
#include "bin_map_store_factory.h"
#include "bin_map_item_base.h"
#include "converter_external_file.h"

const QString   cSystem = "System";
const QString   cStation = "Station";
const QString   cFacility = "Facility";
const QString   cHandler = "Handler";
const QString   cLogin = "Login";
const QString   cProduct = "Product";
const QString   cJobName = "Job_Name";
const QString   cJobRev = "Job Rev.";
const QString   cDvcName = "Dvc_Name";
const QString   cDvcType = "Dvc_Type";
const QString   cLotId = "Lot_Id.";
const QString   cLotNo = "Lot_No.";
const QString   cLotQty = "Lot_Q'ty";
const QString   cDatalogRate = "Datalog_rate";
const QString   cNumberOfTests = "Number_of_tests";
const QString   cQtyLogged = "Quantity_logged";
const QString   cQtyToLog = "Quantity_to_log";
const QString   cReport = "Report";
const QString   cFinalTest = "final_tests";
const QString   cVishayASEQA(":VISHAY_ASE_QA");
const QChar     cDoubleDotSeparator(':');
const QString   cPassBin = "PASS";
const int       cTestDefinitionKeyCol = 4;
const int       cFirstParameterCol = 5;

namespace GS
{
namespace Parser
{

QVector<QString> VishayASEQAtoSTDF::mCompatibleKeys;

//////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////
VishayASEQAtoSTDF::VishayASEQAtoSTDF()
    : ParserBase(typeVishayASEQA, "typeVishayASEQA"), mMinTestColumn(5), mMaxTestColumn(0), mStationNumber(0)
{
    ParserBinning lParserBin;

    lParserBin.SetBinNumber(1);
    lParserBin.SetBinName(cPassBin);
    lParserBin.SetPassFail(true);

    mBinning.insert(1, lParserBin);
}

bool VishayASEQAtoSTDF::IsCompatible(const QString &aFileName)
{
    bool    lIsCompatible = false;
    QFile   lFile(aFileName);

    if(lFile.open(QIODevice::ReadOnly))
    {
        // Assign file I/O stream
        QTextStream lTxtStream(&lFile);

        QMap<QString, QString> lHeaders;
        QString lErrorMessqge;
        if (ReadHeader(lHeaders, lTxtStream, lErrorMessqge))
        {
            lIsCompatible = true;
        }
    }

    return lIsCompatible;
}

bool VishayASEQAtoSTDF::ReadHeader(QMap<QString, QString>& aHeaders, QTextStream &aTxtSTream, QString& aErrorMessage)
{
    bool lHeaderCompleted = false;
    bool lAbort = false;
    bool lValidHeader = false;

    QRegExp lRegExpEqual("=+");
    QRegExp lRegExpReport("DATALOG REPORT");
    QList<QRegExp> lBeginHeaderPatterns;
    lBeginHeaderPatterns << lRegExpEqual << lRegExpReport << lRegExpEqual;

    InitCompatibleKeys();

    QString lLine;
    do
    {
        lLine = aTxtSTream.readLine().simplified();

        if (lLine.isNull() == false && lLine.isEmpty() == false)
        {
            if (lBeginHeaderPatterns.isEmpty() == false)
            {
                if (lBeginHeaderPatterns.first().exactMatch(lLine))
                    lBeginHeaderPatterns.takeFirst();
                else
                    lAbort = true;
            }
            else
            {
                if (lRegExpEqual.exactMatch(lLine))
                    lHeaderCompleted = true;
                else
                {
                    QString lKey = lLine.section(':', 0, 0).simplified();
                    QString lValue = lLine.section(':', 1).simplified();

                    if (mCompatibleKeys.contains(lKey.toLower()) && aHeaders.contains(lKey) == false)
                    {
                        aHeaders.insert(lKey, lValue);
                    }
                    else
                    {
                        aErrorMessage = "Incompatible header key found [" + lKey + "]";
                        lAbort = true;
                    }
                }
            }
        }
    }
    while(!lAbort &&!lHeaderCompleted && !aTxtSTream.atEnd());

    if (!lAbort && lHeaderCompleted && aHeaders.count() == mCompatibleKeys.count())
        lValidHeader = true;

    return lValidHeader;
}

void VishayASEQAtoSTDF::InitCompatibleKeys()
{
    if (mCompatibleKeys.isEmpty())
    {
        mCompatibleKeys.push_back(cSystem.toLower());
        mCompatibleKeys.push_back(cStation.toLower());
        mCompatibleKeys.push_back(cFacility.toLower());
        mCompatibleKeys.push_back(cHandler.toLower());
        mCompatibleKeys.push_back(cLogin.toLower());
        mCompatibleKeys.push_back(cProduct.toLower());
        mCompatibleKeys.push_back(cJobName.toLower());
        mCompatibleKeys.push_back(cJobRev.toLower());
        mCompatibleKeys.push_back(cDvcName.toLower());
        mCompatibleKeys.push_back(cDvcType.toLower());
        mCompatibleKeys.push_back(cLotId.toLower());
        mCompatibleKeys.push_back(cLotNo.toLower());
        mCompatibleKeys.push_back(cLotQty.toLower());
        mCompatibleKeys.push_back(cDatalogRate.toLower());
        mCompatibleKeys.push_back(cNumberOfTests.toLower());
        mCompatibleKeys.push_back(cQtyLogged.toLower());
        mCompatibleKeys.push_back(cQtyToLog.toLower());
        mCompatibleKeys.push_back(cReport.toLower());
    }
}

bool VishayASEQAtoSTDF::ConvertoStdf(const QString &aInputFileName, QString &aFileNameSTDF)
{
    // Open log2 file
    QFile lFile(aInputFileName);
    if(!lFile.open( QIODevice::ReadOnly ))
    {
        // Failed Opening CSV file
        mLastError = errOpenFail;
        mLastErrorMessage = "Unable to open file " + aInputFileName;

        // Convertion failed.
        return false;
    }

    QFileInfo lFileInfo(aInputFileName);
    mDataFilePath = lFileInfo.absolutePath();
    mDataFileInfo = lFileInfo;

    // Check if converter_external_file exists
    bool lExternalFile = ConverterExternalFile::Exists(mDataFilePath);

    if (!lExternalFile)
    {
        mLastError = errConverterExternalFile;
        mLastErrorMessage = "File " + mDataFilePath + "/converter_external_file.xml" + " does not exist";

        return false;
    }

    if (getOptDefaultBinValuesfromExternalFile(mDataFilePath + "/", mLastErrorMessage) == false)
    {
        return false;
    }
    else
    {
        ParserBinning lParserBin;
        lParserBin.SetBinNumber(mDefaultBinNumber);
        lParserBin.SetBinName(mDefaultBinName);
        mBinning.insert(mDefaultBinNumber, lParserBin);
     }


    if (ReadBinMappingFile() == false)
    {
        return false;
    }

    // Assign file I/O stream
    QTextStream aTxtStream(&lFile);

    // Process Header section
    if (ProcessHeader(aTxtStream) == false)
    {
        return false;
    }

    // Read Promis data file using sublotId as key
    if (ReadPromisDataFile() == false)
    {
        return false;
    }

    // Process Test Definition section
    if (ProcessTestsDefinitions(aTxtStream) == false)
    {
        return false;
    }

    if (WriteOutputStdf(aTxtStream, aFileNameSTDF) == false)
    {
        QFile::remove(aFileNameSTDF);
        return false;
    }

    return true;
}

bool VishayASEQAtoSTDF::ProcessTestsDefinitions(QTextStream& aTxtStream)
{
    bool lAbort = false;
    bool lValidSection = false;

    QRegExp lRxSeparator("\\-+");
    QRegExp lRxTest("^\\s+'\\s+'\\s+'\\s+'\\s+Test.+");
    QRegExp lRxItem("^\\s+'\\s+'\\s+'\\s+'\\s+Item.+");
    QRegExp lRxLL("^\\s+'\\s+'\\s+'\\s+'\\s+LL.+");
    QRegExp lRxHL("^\\s+'\\s+'\\s+'\\s+'\\s+HL.+");
    QRegExp lRxBias1("^\\s+'\\s+'\\s+'\\s+'\\s+Bias1.+");
    QRegExp lRxBias2("^\\s+'\\s+'\\s+'\\s+'\\s+Bias2.+");
    QRegExp lRxTime("^\\s+'\\s+'\\s+'\\s+'\\s+Time.+");
    QRegExp lRxUnit("^\\s+No\\.\\s+Bin\\s+Time\\[s\\]\\s+Result\\s+Unit.+");

    QList<QPair<QRegExp, bool> > lPattern;
    lPattern << qMakePair(lRxSeparator, false) << qMakePair(lRxTest, true)<< qMakePair(lRxItem, true)
             << qMakePair(lRxLL, true) << qMakePair(lRxHL, true) << qMakePair(lRxBias1, true)
             << qMakePair(lRxBias2, true) << qMakePair(lRxTime, true) << qMakePair(lRxSeparator, false)
             << qMakePair(lRxUnit, true) << qMakePair(lRxSeparator, false);

    QString lLine;
    do
    {
        lLine = ReadLine(aTxtStream);

        if (lLine.isNull() == false && lLine.isEmpty() == false)
        {
            if (lPattern.isEmpty() == false)
            {
                if (lPattern.first().first.exactMatch(lLine))
                {
                    if (lPattern.first().second)
                    {
                        lAbort = !ParseTestDefinitionLine(lLine);
                    }

                    lPattern.takeFirst();
                }
                else
                    lAbort = true;
            }
        }
    }
    while(!lAbort && !lPattern.isEmpty() && !aTxtStream.atEnd());

    if (!lAbort)
    {
        if (lPattern.isEmpty())
            lValidSection = true;
        else
        {
            mLastError = errMissingData;
            mLastErrorMessage = "Test Definition section is incomplete";
        }
    }

    return lValidSection;
}

bool VishayASEQAtoSTDF::ProcessParts(QTextStream &aTxtStream, GQTL_STDF::StdfParse &aStdfParse)
{
    bool    lValidSection = true;
    QString lLine;
    do
    {
        lLine = ReadLine(aTxtStream);

        if (lLine.isNull() == false && lLine.isEmpty() == false)
        {
            lValidSection = ParsePartLine(lLine, aStdfParse);
        }
    }
    while(lValidSection && !aTxtStream.atEnd());


    return lValidSection;
}

bool VishayASEQAtoSTDF::ParseTestDefinitionLine(const QString &aLine)
{
    QStringList lItems = aLine.split(" ", QString::SkipEmptyParts);

    // First test definition line defines what is the max columns on a test row
    if (mMaxTestColumn == 0)
        mMaxTestColumn = lItems.count();

    if (lItems.count() < mMinTestColumn)
    {
        mLastError = errInvalidFormatParameter;
        mLastErrorMessage = "There are too few columns in Test Definition section";
        return false;
    }

    QString lKey = lItems[cTestDefinitionKeyCol];

    if (lItems.count() > mMaxTestColumn)
    {
        mLastError = errInvalidFormatParameter;
        mLastErrorMessage = "There are too many columns for row " + lKey + " in Test Definition section";
        return false;
    }

    // Items count must be an odd number as there is 5 header columns then 2 columns per test
    if ((lItems.count() % 2) == 0)
    {
        mLastError = errInvalidFormatParameter;
        mLastErrorMessage = "Number of columns in incorrect for row " + lKey + " in Test Definition section";
        return false;
    }

    int     lNbTests = (lItems.count() - cFirstParameterCol) / 2;

    QString lValue;
    int     lTestIndex = -1;
    int     lItemCol = 0;

    if (lKey.compare("Item", Qt::CaseInsensitive) == 0)
    {
        // ([0-9]+)_(.*)
        QRegExp lRxTest("([0-9]+)_(.*)");

        for (int lIdx = 0; lIdx < lNbTests; ++lIdx)
        {
            lItemCol = cFirstParameterCol + (lIdx * 2);
            lValue = lItems[lItemCol];
            int lTestNumber = 0;

            if (lRxTest.exactMatch(lValue))
                lTestNumber = lRxTest.cap(1).toInt();

            if (lTestNumber != 0)
            {
                try
                {
                    const Qx::BinMapping::BinMapItemBase &lBinMapItem =
                            mBinMapFinalTestStore->GetBinMapItemByTestNumber( lTestNumber );

                    // Keep track of test definition for enabled test
                    if (lBinMapItem.GetEnabled())
                    {
                        // Keep track of bin to map for this test column
                        mTestColumnToBinNumber.insert(lIdx, static_cast<stdf_type_u2>(lBinMapItem.GetBinNumber()));

                        // Create Binning information (number and name)
                        if (mBinning.contains(lBinMapItem.GetBinNumber()) == false)
                        {
                            ParserBinning lParserBin;

                            lParserBin.SetBinNumber(static_cast<stdf_type_u2>(lBinMapItem.GetBinNumber()));
                            lParserBin.SetBinName(QString::fromStdString(lBinMapItem.GetBinName()));

                            mBinning.insert(lBinMapItem.GetBinNumber(), lParserBin);
                        }

                        if (mTestNumberToTestIndex.contains(lTestNumber))
                        {
                            lTestIndex = mTestNumberToTestIndex.value(lTestNumber);
                        }
                        else
                        {
                            ParserParameter lParameter;
                            lTestIndex = mParameterList.count();

                            mParameterList.append(ParserParameter());
                            mTestNumberToTestIndex.insert(lTestNumber, lTestIndex);
                        }

                        mTestColumnToTestIndex.insert(lIdx, lTestIndex);

                        mParameterList[lTestIndex].SetTestNumber(lTestNumber);
                        mParameterList[lTestIndex].SetTestName(QString::fromStdString(lBinMapItem.GetTestName()));
                        mParameterList[lTestIndex].SetIsParamatricTest(true);
                    }
                    else
                        mTestColumnToBinNumber.insert(lIdx, mDefaultBinNumber);
                }
                catch (const Qx::BinMapping::BinMappingExceptionForUserBase &)
                {
                    // DONT DO ANYTHING HERE WHEN TEST DOES NOT HAVE A MAPPED ENTRY IN BINMAP FILE
                }
            }
            else
                mTestColumnToBinNumber.insert(lIdx, mDefaultBinNumber);
        }
    }
    else if (lKey.compare("LL", Qt::CaseInsensitive) == 0)
    {
        // (>?)([-+]?[0-9]+\.?[0-9]*)([munpfKMGT]?)
        QRegExp lRxLowLimit("(>?)([-+]?[0-9]+\\.?[0-9]*)([munpfKMGT]?)");

        for (int lIdx = 0; lIdx < lNbTests; ++lIdx)
        {
            lItemCol = cFirstParameterCol + (lIdx * 2);

            if (mTestColumnToTestIndex.contains(lIdx))
            {
                lTestIndex = mTestColumnToTestIndex.value((lIdx));
                lValue = lItems[lItemCol];

                if (lRxLowLimit.exactMatch(lValue))
                {
                    if (lRxLowLimit.cap(1).isEmpty() == false)
                    {
                        int lHLScaleFactor = 0;

                        if (lRxLowLimit.cap(3).isEmpty() == false)
                        {
                            lHLScaleFactor = GS::Core::STDFUnitPrefixToScale(lRxLowLimit.cap(3).at(0).toLatin1());
                        }

                        mParameterList[lTestIndex].SetLowLimit(GS::Core::NormalizeValue(lRxLowLimit.cap(2).toDouble(),
                                                                                   lHLScaleFactor));
                        mParameterList[lTestIndex].SetValidLowLimit(true);
                    }
                }
                else if (lValue.compare("-") != 0)
                {
                    mLastError = errInvalidFormatParameter;
                    mLastErrorMessage = "Unknown format detected at column " + QString::number(lItemCol) + " for LL row";
                    return false;
                }
            }
        }
    }
    else if (lKey.compare("HL", Qt::CaseInsensitive) == 0)
    {
        // (<?)([-+]?[0-9]+\.?[0-9]*)([munpfKMGT]?)
        QRegExp lRxHighLimit("(<?)([-+]?[0-9]+\\.?[0-9]*)([munpfKMGT]?)");

        for (int lIdx = 0; lIdx < lNbTests; ++lIdx)
        {
            lItemCol = cFirstParameterCol + (lIdx * 2);

            if (mTestColumnToTestIndex.contains(lIdx))
            {
                lTestIndex = mTestColumnToTestIndex.value((lIdx));
                lValue = lItems[lItemCol];

                if (lRxHighLimit.exactMatch(lValue))
                {
                    if (lRxHighLimit.cap(1).isEmpty() == false)
                    {
                        int lHLScaleFactor = 0;

                        if (lRxHighLimit.cap(3).isEmpty() == false)
                        {
                            lHLScaleFactor = GS::Core::STDFUnitPrefixToScale(lRxHighLimit.cap(3).at(0).toLatin1());
                        }

                        mParameterList[lTestIndex].SetHighLimit(GS::Core::NormalizeValue(lRxHighLimit.cap(2).toDouble(),
                                                                                   lHLScaleFactor));
                        mParameterList[lTestIndex].SetValidHighLimit(true);
                    }
                }
                else if (lValue.compare("-") != 0)
                {
                    mLastError = errInvalidFormatParameter;
                    mLastErrorMessage = "Unknown format detected at column " + QString::number(lItemCol) + " for HL row";
                    return false;
                }
            }
        }
    }
    else if (lKey.compare("Time", Qt::CaseInsensitive) == 0)
    {
        // ([0-9]+\.?[0-9]*)([munpfKMGT]?).*
        QRegExp lRxTime("([0-9]+\\.?[0-9]*)([munpfKMGT]?)(.*)");

        for (int lIdx = 0; lIdx < lNbTests; ++lIdx)
        {
            lItemCol = cFirstParameterCol + (lIdx * 2);

            if (mTestColumnToTestIndex.contains(lIdx))
            {
                lTestIndex = mTestColumnToTestIndex.value((lIdx));
                lValue = lItems[lItemCol];

                if (lRxTime.exactMatch(lValue))
                {
                    int lTimeScaleFactor = 0;

                    if (lRxTime.cap(3).isEmpty() == false)
                    {
                        lTimeScaleFactor = GS::Core::STDFUnitPrefixToScale(lRxTime.cap(2).at(0).toLatin1());
                    }

                    mParameterList[lTestIndex].SetExecTime(static_cast<float>(GS::Core::NormalizeValue(lRxTime.cap(1).toDouble(),
                                                                                                 lTimeScaleFactor)));
                }
                else if (lValue.compare("-") != 0)
                {
                    mLastError = errInvalidFormatParameter;
                    mLastErrorMessage = "Unknown format detected at column " + QString::number(lItemCol) +
                                        " for Time row";
                    return false;
                }
            }
        }
    }
    else if (lKey.compare("Unit", Qt::CaseInsensitive) == 0)
    {
        // ([munpfKMGT]?)(.*)
        QRegExp lRxUnit("([munpfKMGT]?)(.*)");

        for (int lIdx = 0; lIdx < lNbTests; ++lIdx)
        {
            lItemCol = cFirstParameterCol + (lIdx * 2);

            if (mTestColumnToTestIndex.contains(lIdx))
            {
                lTestIndex = mTestColumnToTestIndex.value((lIdx));
                lValue = lItems[lItemCol];

                if (lRxUnit.exactMatch(lValue))
                {
                    if (lRxUnit.cap(1).isEmpty() == false)
                    {
                        mParameterList[lTestIndex].SetResultScale(GS::Core::STDFUnitPrefixToScale(lRxUnit.cap(1)
                                                                                            .at(0).toLatin1()));
                    }

                    if (lRxUnit.cap(2).isEmpty() == false && lValue.compare("-") != 0)
                    {
                        mParameterList[lTestIndex].SetTestUnit(lRxUnit.cap(2));
                    }
                }
                else
                {
                    mLastError = errInvalidFormatParameter;
                    mLastErrorMessage = "Unknown format detected at column " + QString::number(lItemCol) +
                                        " for Unit row";
                    return false;
                }
            }
        }
    }

    return true;
}

bool VishayASEQAtoSTDF::ParsePartLine(const QString &aLine, GQTL_STDF::StdfParse& aStdfParse)
{
    QStringList lItems = aLine.split(" ", QString::SkipEmptyParts);
    QRegExp lRxPartId("^(.+)\\[.*\\]");
    QString lValue;
    bool    lPass;
    stdf_type_b1 lPartFlag = 0;
    stdf_type_u2 lBin = 1;
    stdf_type_u4 lTestTime = 0;
    stdf_type_cn lPartId;

    if (lItems.count() < mMinTestColumn)
    {
        mLastError = errInvalidFormatParameter;
        mLastErrorMessage = "There are too few columns in Test results section";
        return false;
    }

    // Extract Part ID
    lPartId = lItems[0].section("[", 0, 0);

    if (lItems.count() > mMaxTestColumn)
    {
        mLastError = errInvalidFormatParameter;
        mLastErrorMessage = "There are too many columns for part id " + lPartId + " in Test results section";
        return false;
    }

    // Items count must be an odd number as there is 5 header columns then 2 columns per test
    if ((lItems.count() % 2) == 0)
    {
        mLastError = errInvalidFormatParameter;
        mLastErrorMessage = "Number of columns in incorrect for part id " + lPartId + " in Test results section";
        return false;
    }

    int     lNbTests = (lItems.count() - cFirstParameterCol) / 2;
    int     lTestIndex = -1;

    QMap<int, QPair<QString, bool> > lExecutedParameterList;

    // Extract Test Time
    if (lItems[2].compare("'") != 0)
    {
        bool    lOk = false;
        double  lTT = lItems[2].toDouble(&lOk);

        if (lOk == false)
        {
            {
                mLastError = errInvalidFormatParameter;
                mLastErrorMessage = "Test time is not a numerical value for Part ID " + lPartId;
                return false;
            }
        }

        lTestTime = static_cast<stdf_type_u4>(lTT * 1000);
    }

    if (WritePIR(aStdfParse) == false)
    {
        mLastError = errWriteSTDF;
        mLastErrorMessage = "Failed while writing PIR Record";

        return false;
    }

    int lFailingTestColumn = -1;
    int lItemCol = -1;

    for (int lIdx = 0; lIdx < lNbTests; ++lIdx)
    {
        lItemCol = cFirstParameterCol + (lIdx * 2);
        lPass = lItems[lItemCol + 1].contains("'");

        if (mTestColumnToTestIndex.contains(lIdx))
        {
            lTestIndex = mTestColumnToTestIndex.value((lIdx));
            lValue = lItems[lItemCol];

            // When parsing the first part, check whether the test is a Functional Test (value are
            if (mParameterList[lTestIndex].GetStaticHeaderWritten() == false)
            {
                if (lValue.compare("PASS", Qt::CaseInsensitive) == 0 ||
                    lValue.compare("FAIL", Qt::CaseInsensitive) == 0)
                {
                    mParameterList[lTestIndex].SetIsParamatricTest(false);
                }
            }

            if (lExecutedParameterList.contains(lTestIndex))
            {
                lExecutedParameterList[lTestIndex].second &= lPass;
            }
            else
            {
                lExecutedParameterList.insert(lTestIndex, qMakePair(lValue, lPass));
            }

            if (lPass == false)
            {
                lFailingTestColumn = lIdx;
            }
        }
        else if (mTestColumnToBinNumber.contains(lIdx) == false)
        {
            // This test was not found in the mapping file, we keep track of the failing test
            if (lPass == false)
            {
                lFailingTestColumn = lIdx;
            }
        }
    }

    // Extract Part Status
    if (lItems[3].compare("PASS", Qt::CaseInsensitive) != 0)
    {
        lPartFlag = 0x08;

        if (lFailingTestColumn >= 0)
        {
            if (mTestColumnToBinNumber.contains(lFailingTestColumn) == false)
            {
                mLastError = errNonMappedFailingTest;
                mLastErrorMessage = "Test at column " + QString::number(cFirstParameterCol + (lFailingTestColumn * 2)) +
                                    " is failing a part but cannot be mapped";
                return false;
            }
            else
            {
                lBin = mTestColumnToBinNumber.value(lFailingTestColumn);
            }
        }
        else if (mDefaultBinSet == true)
        {
            lBin = mDefaultBinNumber;
        }
        else
        {
            mLastError = errUndefinedBinDefault;
            mLastErrorMessage = "Could not process Part ID: " + lPartId;
            return false;
        }
    }

    mBinning[lBin].IncrementBinCount(1);

    if (WriteTestResults(aStdfParse, lExecutedParameterList) == false)
    {
        return false;
    }

    if (WritePRR(aStdfParse, lPartFlag, static_cast<stdf_type_u2>(lExecutedParameterList.count()),
                 lBin, lTestTime, lPartId) == false)
    {
        mLastError = errWriteSTDF;
        mLastErrorMessage = "Failed while writing PRR Record";

        return false;
    }

    return true;
}

bool VishayASEQAtoSTDF::ProcessHeader(QTextStream &aTxtStream)
{
    QMap<QString, QString>  lHeaders;
    QRegExp lRxStation("STATION([01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])$", Qt::CaseInsensitive);

    if (ReadHeader(lHeaders, aTxtStream, mLastErrorMessage) == false)
    {
        mLastError = errMissingData;
        return false;
    }

    mTesterType = lHeaders.value(cSystem).section(" ", 0, 0);
    mNodeName = "STATEC";
    mHandlerId = lHeaders.value(cHandler);
    mOperatorName = lHeaders.value(cLogin);
    mJobName = lHeaders.value(cJobName);
    mJobRev = lHeaders.value(cJobRev);
    mLotId = lHeaders.value(cLotId).section(".", 0, 0);
    mSublotId = lHeaders.value(cLotId);

    QDateTime lReportTime = QDateTime::fromString(lHeaders.value(cReport).simplified(),
                                                  "yyyy/MM/dd hh:mm:ss");

    lReportTime.setTimeSpec(Qt::UTC);
    mStartTime = lReportTime.toTime_t();

    if (lRxStation.exactMatch(lHeaders.value(cStation)))
    {
        mStationNumber = static_cast<stdf_type_u1>(lRxStation.cap(1).toUShort());
    }

    return true;
}

bool VishayASEQAtoSTDF::ReadPromisDataFile()
{
    // Check if converter_external_file exists
    QString lExternalFileName;
    QString lExternalFileFormat;
    QString lExternalFileError;

    ConverterExternalFile::GetPromisFile(mDataFilePath, "final", "prod", lExternalFileName,
                                         lExternalFileFormat, lExternalFileError);
    mPromisFilePath = lExternalFileName;
    if(lExternalFileName.isEmpty())
    {
        if(lExternalFileError.isEmpty())
            lExternalFileError = "No 'PROMIS_DATA_FT' file defined";
        mLastError = errReadPromisFile;
        mLastErrorMessage = lExternalFileError;
        return false;
    }

    if(mSublotId.isEmpty())
    {
        mLastError = errReadPromisFile;
        mLastErrorMessage = "No Promis key defined";

        // Convertion failed.
        return false;
    }

    // CLear/Set variables to default
    mFacilityId = "";
    mSiteLocation = "";
    mDateCode = "";
    mPackage = "";
    mProcId = "";
    mPackageType = "";
    mProductId = "";

    try
    {
        mPromisInterpreter.reset( Qx::BinMapping::PromisInterpreterFactory< Qx::BinMapping::promis_lvm_ft >
                                  ::MakePromisInterpreter( mSublotId.toStdString(),
                                                           lExternalFileName.toStdString(),
                                                           ConverterExternalFile::GetExternalFileName( mDataFilePath )
                                                           .toStdString() ) );

        mDateCode           = mPromisInterpreter->GetPromisItem().GetDateCode().c_str();
        mPackage            = mPromisInterpreter->GetPromisItem().GetPackage().c_str();
        mProcId             = mPromisInterpreter->GetPromisItem().GetProductId().c_str();
        mFacilityId = mSiteLocation = mPromisInterpreter->GetPromisItem().GetSiteId().c_str();
        mProductId          = mPromisInterpreter->GetPromisItem().GetGeometryName().c_str();
        mPackageType        = mPromisInterpreter->GetPromisItem().GetPackageType().c_str();

        if (mPromisInterpreter->GetPromisItem().GetEquipmentID().empty() == false)
        {
            mTesterType = mNodeName = mPromisInterpreter->GetPromisItem().GetEquipmentID().c_str();
        }
    }
    catch( const std::exception &lException )
    {
        mLastError = errReadPromisFile;
        mLastErrorMessage = lException.what();
        return false;
    }

    return true;
}

bool VishayASEQAtoSTDF::WriteHBR(GQTL_STDF::StdfParse &aStdfParse)
{
    GQTL_STDF::Stdf_HBR_V4 lHBRRecord;
    lHBRRecord.SetHEAD_NUM(1);
    lHBRRecord.SetSITE_NUM(1);

    QMap<int,ParserBinning>::Iterator it;
    for(it = mBinning.begin(); it != mBinning.end(); it++)
    {
        if ((*it).GetBinCount() > 0)
        {
            lHBRRecord.SetHBIN_NUM((*it).GetBinNumber());
            lHBRRecord.SetHBIN_CNT(static_cast<stdf_type_u4>((*it).GetBinCount()));
            lHBRRecord.SetHBIN_NAM((*it).GetBinName());
            lHBRRecord.SetHBIN_PF((*it).GetPassFail() ? 'P' : 'F');

            if (aStdfParse.WriteRecord(&lHBRRecord) == false)
                return false;
        }
    }

    return true;
}

bool VishayASEQAtoSTDF::WriteMIR(GQTL_STDF::StdfParse &aStdfParse)
{
    GQTL_STDF::Stdf_MIR_V4 lMIRRecord;

    lMIRRecord.SetSETUP_T(static_cast<stdf_type_u4>(mStartTime));
    lMIRRecord.SetSTART_T(static_cast<stdf_type_u4>(mStartTime));
    lMIRRecord.SetSTAT_NUM(mStationNumber);
    lMIRRecord.SetMODE_COD(static_cast<BYTE>('P'));
    lMIRRecord.SetRTST_COD(static_cast<BYTE>(' '));
    lMIRRecord.SetPROT_COD(static_cast<BYTE>(' '));
    lMIRRecord.SetBURN_TIM(65535);
    lMIRRecord.SetCMOD_COD(static_cast<BYTE>(' '));
    lMIRRecord.SetLOT_ID(mLotId.toLatin1().constData());
    if(!mProductId.isEmpty())
        lMIRRecord.SetPART_TYP(mProductId.toLatin1().constData());
    if(!mNodeName.isEmpty())
        lMIRRecord.SetNODE_NAM(mNodeName.toLatin1().constData());
    if(!mTesterType.isEmpty())
        lMIRRecord.SetTSTR_TYP(mTesterType.toLatin1().constData());
    lMIRRecord.SetJOB_NAM(mJobName.toLatin1().constData());
    lMIRRecord.SetJOB_REV(mJobRev.toLatin1().constData());
    lMIRRecord.SetSBLOT_ID(mSublotId.toLatin1().constData());
    lMIRRecord.SetOPER_NAM(mOperatorName.toLatin1().constData());
    if(!mSiteLocation.isEmpty())
        lMIRRecord.SetTEST_COD(mSiteLocation.toLatin1().constData());

    // Construct custom Galaxy USER_TXT
    QString lUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    lUserTxt += cDoubleDotSeparator;
    lUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    lUserTxt += cVishayASEQA;
    lMIRRecord.SetUSER_TXT(lUserTxt.toLatin1().constData());

    lMIRRecord.SetAUX_FILE("");
    if(!mPackageType.isEmpty())
        lMIRRecord.SetPKG_TYP(mPackageType.toLatin1().constData());
    lMIRRecord.SetFAMLY_ID("");
    if(!mDateCode.isEmpty())
        lMIRRecord.SetDATE_COD(mDateCode.toLatin1().constData());
    if(!mFacilityId.isEmpty())
        lMIRRecord.SetFACIL_ID(mFacilityId.toLatin1().constData());
    lMIRRecord.SetFLOOR_ID("");
    if(!mProcId.isEmpty())
     lMIRRecord.SetPROC_ID(mProcId.toLatin1().constData());
    if(!mPackage.isEmpty())
        lMIRRecord.SetROM_COD(mPackage.toLatin1().constData());

    return aStdfParse.WriteRecord(&lMIRRecord);
}

bool VishayASEQAtoSTDF::WriteMRR(GQTL_STDF::StdfParse &aStdfParse)
{
    GQTL_STDF::Stdf_MRR_V4 lMRRRecord;
    lMRRRecord.SetFINISH_T(static_cast<stdf_type_u4>(mStartTime));

    return aStdfParse.WriteRecord(&lMRRRecord);
}

bool VishayASEQAtoSTDF::WritePIR(GQTL_STDF::StdfParse &aStdfParse)
{
    GQTL_STDF::Stdf_PIR_V4 lPIRRecord;
    lPIRRecord.SetHEAD_NUM(1);
    lPIRRecord.SetSITE_NUM(1);

    return aStdfParse.WriteRecord(&lPIRRecord);
}

bool VishayASEQAtoSTDF::WritePRR(GQTL_STDF::StdfParse &aStdfParse, stdf_type_b1 aFlag, stdf_type_u2 aTestExecuted,
                                 stdf_type_u2 aBin, stdf_type_u4 aTestTime, stdf_type_cn aPartId)
{
    GQTL_STDF::Stdf_PRR_V4 lPRRRecord;
    lPRRRecord.SetHEAD_NUM(1);
    lPRRRecord.SetSITE_NUM(1);
    lPRRRecord.SetPART_FLG(aFlag);
    lPRRRecord.SetHARD_BIN(aBin);
    lPRRRecord.SetSOFT_BIN(aBin);
    lPRRRecord.SetTEST_T(aTestTime);
    lPRRRecord.SetNUM_TEST(aTestExecuted);
    lPRRRecord.SetPART_ID(aPartId);

    return aStdfParse.WriteRecord(&lPRRRecord);
}

bool VishayASEQAtoSTDF::WriteTestResults(GQTL_STDF::StdfParse &aStdfParse, QMap<int, QPair<QString, bool> > aExecutedTest)
{
    for (int lIdx = 0; lIdx < mParameterList.count(); ++lIdx)
    {
        ParserParameter& lParameter = mParameterList[lIdx];
        if (aExecutedTest.contains(lIdx))
        {
            const QPair<QString, bool>& lTest = aExecutedTest[lIdx];
            if (lParameter.IsParametricTest())
            {
                GQTL_STDF::Stdf_PTR_V4 lPTRRecord;
                bool lOk = false;
                float lResult = lTest.first.toFloat(&lOk);
                stdf_type_b1 lTestFlag = lTest.second ? 0 : static_cast<stdf_type_b1>(-128);

                lPTRRecord.SetHEAD_NUM(1);
                lPTRRecord.SetSITE_NUM(1);
                lPTRRecord.SetTEST_NUM(static_cast<stdf_type_u4>(lParameter.GetTestNumber()));

                if (lOk)
                    lPTRRecord.SetRESULT(lResult);
                else
                    lTestFlag |= 0x02;

                lPTRRecord.SetTEST_FLG(lTestFlag);
                lPTRRecord.SetPARM_FLG(0);

                if (lParameter.GetStaticHeaderWritten() == false)
                {
                    stdf_type_b1 lOptFlag = 0;

                    lPTRRecord.SetRES_SCAL(static_cast<stdf_type_i1>(lParameter.GetResultScale()));
                    if (lParameter.GetValidLowLimit())
                        lPTRRecord.SetLO_LIMIT(static_cast<stdf_type_r4>(lParameter.GetLowLimit()));
                    else
                        lOptFlag |= 0x40;

                    if (lParameter.GetValidHighLimit())
                        lPTRRecord.SetHI_LIMIT(static_cast<stdf_type_r4>(lParameter.GetHighLimit()));
                    else
                        lOptFlag |= 0x80;

                    lPTRRecord.SetOPT_FLAG(lOptFlag);
                    lPTRRecord.SetUNITS(lParameter.GetTestUnits());
                    lParameter.SetStaticHeaderWritten(true);
                }

                if (aStdfParse.WriteRecord(&lPTRRecord) == false)
                {
                    mLastError = errWriteSTDF;
                    mLastErrorMessage = "Failed while writing PTR Record";

                    return false;
                }
            }
            else
            {
                GQTL_STDF::Stdf_FTR_V4 lFTRRecord;

                lFTRRecord.SetHEAD_NUM(1);
                lFTRRecord.SetSITE_NUM(1);
                lFTRRecord.SetTEST_NUM(static_cast<stdf_type_u4>(lParameter.GetTestNumber()));

                stdf_type_b1 lData = lTest.second ? 0 : static_cast<stdf_type_b1>(-128);

                lFTRRecord.SetTEST_FLG(lData);
                lFTRRecord.SetOPT_FLAG(static_cast<stdf_type_b1>(-1));

                if (lParameter.GetStaticHeaderWritten() == false)
                    lParameter.SetStaticHeaderWritten(true);

                if (aStdfParse.WriteRecord(&lFTRRecord) == false)
                {
                    mLastError = errWriteSTDF;
                    mLastErrorMessage = "Failed while writing FTR Record";

                    return false;
                }
            }

            lParameter.IncrementExecTest();

            if (lTest.second == false)
                lParameter.IncrementFailTest();
        }
    }

    return true;
}

bool VishayASEQAtoSTDF::WriteSBR(GQTL_STDF::StdfParse &aStdfParse)
{
    GQTL_STDF::Stdf_SBR_V4 lSBRRecord;
    lSBRRecord.SetHEAD_NUM(1);
    lSBRRecord.SetSITE_NUM(1);

    QMap<int,ParserBinning>::Iterator it;
    for(it = mBinning.begin(); it != mBinning.end(); it++)
    {
        if ((*it).GetBinCount() > 0)
        {
            lSBRRecord.SetSBIN_NUM((*it).GetBinNumber());
            lSBRRecord.SetSBIN_CNT(static_cast<stdf_type_u4>((*it).GetBinCount()));
            lSBRRecord.SetSBIN_NAM((*it).GetBinName());
            lSBRRecord.SetSBIN_PF((*it).GetPassFail() ? 'P' : 'F');

            if (aStdfParse.WriteRecord(&lSBRRecord) == false)
                return false;
        }
    }

    return true;
}

bool VishayASEQAtoSTDF::WriteSDR(GQTL_STDF::StdfParse &aStdfParse)
{
    GQTL_STDF::Stdf_SDR_V4 lSDRRecord;
    lSDRRecord.SetHEAD_NUM(255);
    lSDRRecord.SetSITE_GRP(1);
    lSDRRecord.SetSITE_CNT(1);
    lSDRRecord.SetSITE_NUM(0, 1);
    lSDRRecord.SetHAND_ID(mHandlerId);

    return aStdfParse.WriteRecord(&lSDRRecord);
}

bool VishayASEQAtoSTDF::WriteTSR(GQTL_STDF::StdfParse &aStdfParse)
{
    GQTL_STDF::Stdf_TSR_V4 lTSRRecord;

    QList<ParserParameter>::Iterator it;
    for(it = mParameterList.begin(); it != mParameterList.end(); it++)
    {
        lTSRRecord.SetHEAD_NUM(255);
        lTSRRecord.SetSITE_NUM(1);
        lTSRRecord.SetTEST_TYP((*it).IsParametricTest() ? 'P' : 'F');
        lTSRRecord.SetTEST_NUM(static_cast<stdf_type_u4>((*it).GetTestNumber()));
        lTSRRecord.SetEXEC_CNT((*it).GetExecCount());
        lTSRRecord.SetFAIL_CNT((*it).GetExecFail());
        lTSRRecord.SetTEST_NAM((*it).GetTestName());
        lTSRRecord.SetTEST_TIM((*it).GetExecTime());

        if (aStdfParse.WriteRecord(&lTSRRecord) == false)
            return false;
    }


    return true;
}

bool VishayASEQAtoSTDF::WriteOutputStdf(QTextStream& aTxtStream, const QString &aFileNameSTDF)
{
    GQTL_STDF::StdfParse lStdfParser;

    if(lStdfParser.Open(aFileNameSTDF.toLatin1().constData(), STDF_WRITE) == false)
    {
        mLastError = errWriteSTDF;
        mLastErrorMessage = QString("Can't write to output file %1").arg(aFileNameSTDF).toLatin1().constData();
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());

        return false;
    }

    if(mStartTime <= 0)
        mStartTime = QDateTime::currentDateTime().toTime_t();

    if (WriteMIR(lStdfParser) == false)
    {
        mLastError = errWriteSTDF;
        mLastErrorMessage = "Failed while writing MIR Record";
        lStdfParser.Close();
        return false;
    }

    if (WriteSDR(lStdfParser) == false)
    {
        mLastError = errWriteSTDF;
        mLastErrorMessage = "Failed while writing SDR Record";
        lStdfParser.Close();
        return false;
    }

    // Process Parts and Test results section
    if (ProcessParts(aTxtStream, lStdfParser) == false)
    {
        lStdfParser.Close();
        return false;
    }

    if (WriteTSR(lStdfParser) == false)
    {
        mLastError = errWriteSTDF;
        mLastErrorMessage = "Failed while writing TSR Records";
        lStdfParser.Close();
        return false;
    }

    if (WriteHBR(lStdfParser) == false)
    {
        mLastError = errWriteSTDF;
        mLastErrorMessage = "Failed while writing HBR Records";
        lStdfParser.Close();
        return false;
    }

    if (WriteSBR(lStdfParser) == false)
    {
        mLastError = errWriteSTDF;
        mLastErrorMessage = "Failed while writing SBR Records";
        lStdfParser.Close();
        return false;
    }

    if (WriteMRR(lStdfParser) == false)
    {
        mLastError = errWriteSTDF;
        mLastErrorMessage = "Failed while writing MRR Record";
        lStdfParser.Close();
        return false;
    }

    return true;
}

bool VishayASEQAtoSTDF::ReadBinMappingFile()
{
    QString lExternalFileFormat;
    QString lExternalFileError;
    QString lBinMapFileName;

    // Check if Test->Binning mapping file to overload softbin
    ConverterExternalFile::GetBinmapFile(mDataFilePath, "final", "prod", cFinalTest,
                                         lBinMapFileName, lExternalFileFormat, lExternalFileError);
    mExternalFilePath = ConverterExternalFile::GetExternalFileName(mDataFilePath);
    mBinMapFilePath = lBinMapFileName;
    if(lBinMapFileName.isEmpty())
    {
        if(lExternalFileError.isEmpty())
            lExternalFileError = "No 'BINMAP_FILE' file defined";

        if(!cFinalTest.isEmpty())
            lExternalFileError = lExternalFileError + " for " + cFinalTest + " category";

        mLastError = errReadBinMapFile;
        mLastErrorMessage = lExternalFileError;

        return false;
    }

    try
    {
        mBinMapFinalTestStore.reset(Qx::BinMapping::BinMapStoreFactory<Qx::BinMapping::lvm_ft_ft>
                                                ::MakeBinMapStore(lBinMapFileName.toStdString(),
                                                                  mExternalFilePath.toStdString() ) );

    }
    catch( const std::exception &lException )
    {
        mLastError = errReadBinMapFile;
        mLastErrorMessage = lException.what() + appendBinMappingExceptionInfo();
        return false;
    }

    return true;
}

std::string VishayASEQAtoSTDF::GetErrorMessage(const int ErrorCode) const
{
    QString lError;

    switch(ErrorCode)
    {
        case errNonMappedFailingTest:
            lError =
                QString( "typeVishayASEQA - Invalid file format: - Failed processing input LOG2 [%1] - Unknown test number 379 in the bin map file %2 specified in %3" )
                    .arg( mDataFileInfo.absoluteFilePath() )
                    .arg( mBinMapFilePath )
                    .arg( mExternalFilePath );
            break;

        case errReadBinDefault:
            lError += "Failing to read default bin information: " + mLastErrorMessage;
            break;

        case errUndefinedBinDefault:
            lError += "No default bin was defined in converter external file " + mExternalFilePath
                      + " : " + mLastErrorMessage;
            break;

        default:
            lError += QString::fromStdString(ParserBase::GetErrorMessage(ErrorCode));
            break;
    }

    return lError.toStdString();
}

}
}
