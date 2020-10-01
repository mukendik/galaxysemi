//////////////////////////////////////////////////////////////////////
// importVishayASESummary.cpp: Convert a Vishay ASE file to STDF V4.0
//////////////////////////////////////////////////////////////////////
#include "importVishayASESummary.h"
#include <gqtl_log.h>
#include <math.h>
#include <qfileinfo.h>
#include "stdfparse.h"
#include "converter_external_file.h"
#include "bin_map_store_factory.h"
#include "bin_map_item_base.h"
#include "promis_interpreter_factory.h"
#include "promis_item_base.h"

/// The file example is below
//<< TEST SUMMARY REPORT >>
//===============================================
//System   :  STA2100 [ TDS014 ]
//Station  :  STATION1
//
//Facility :  STATEC
//Handler  :  HDS066
//
//Login    :  USER - OPERATOR,  MODE - OPERATOR
//Product  :  Product
//
//Job Name :  DC2_SUD50P06-15_08
//Job Rev. :
//Dvc Name :  SUD50P06-15-GE3
//
//Lot Id.  :  K15K261.1
//Lot No.  :  K15K261.1
//Lot Q'ty :  7812
//-----------------------------------------------
//Total Test Count  :           6453
//Total Good Count  :           6451
//Yield (TGC/TTC %) :          99.97
//-----------------------------------------------
//                      Date______   Time____
//Report      .......   2018/07/19   07:40:25
//Setup       .......   2018/07/19   05:35:32
//Test Start  .......   2018/07/19   05:36:19
//Test End    .......   2018/07/19   06:40:57
//-----------------------------------------------
//
//<< TEST BIN REPORT >>
//===============================================
//BIN    COUNT  PERCENT    BIN    COUNT  PERCENT
//-----------------------------------------------
//  1     6451   99.97       2        0    0.00
//  3        2    0.03       5        0    0.00
//
//<< SORT PLAN REPORT >>
//===============================================
//S#    NAME              BIN     COUNT  PERCENT
//-----------------------------------------------
//S1    19_KEL    (103)     3         2    0.03
//S2    1009_SPU  (300)     2         0    0.00
//...
//S6    2_IGSS    (307)     5         0    0.00
//S7    22_IDSX   (309)     5         0    0.00
//S8    SUD50P06-15         1      6451   99.97
//S9    REJ                 3         0    0.00
//
//<< TEST ITEM(S) REPORT >>
//===============================================
// No.   ITEM(S)              COUNT      PERCENT
//-----------------------------------------------
//   1   CONT                     2        0.03

const QString cSectionTestSummaryReport = "TEST SUMMARY REPORT";
const QString cSectionTestBinReport = "TEST BIN REPORT";
const QString cSectionSortPlanReport = "SORT PLAN REPORT";
const QString cSectionStart = "<<";
const QString cSectionEqualLine = "===";
const QString cSectionMinusLine = "---";

const QString cSystem = "System";
const QString cStation = "Station";
const QString cFacility = "Facility";
const QString cHandler = "Handler";
const QString cLogin = "Login";
const QString cProduct = "Product";
const QString cJobName = "Job Name";
const QString cJobRev = "Job Rev.";
const QString cDeviceName = "Dvc Name";
const QString cLotId = "Lot Id.";
const QString cLotNo = "Lot No.";
const QString cReport = "Report";
const QString cSetup = "Setup";
const QString cTestStart = "Test Start";
const QString cTestEnd = "Test End";

const QString cSortNum = "S#";
const QString cSortName = "NAME";
const QString cSortBin = "BIN";
const QString cSortCount = "COUNT";
const QString cSortPercent = "PERCENT";


const QString cSortEntries = "sort_entries";
const QString cFinalTest = "final_tests";
const QString cNodeName = "STATEC";
const QString cPass = "PASS";
const QString cSeparator = ":";
const QString cSpace = " ";

namespace GS
{
namespace Parser
{


//////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////
VishayASESummarytoSTDF::VishayASESummarytoSTDF(): ParserBase(typeVishayASESummary, "typeVishayASESummary")
{
}

//////////////////////////////////////////////////////////////////////
// Check if File is compatible with VISHAY ASE format
//////////////////////////////////////////////////////////////////////
bool VishayASESummarytoSTDF::IsCompatible(const QString &aFileName)
{
    QFile lFile(aFileName);
    if(!lFile.open(QIODevice::ReadOnly ))
    {
        // Convertion failed.
        return false;
    }
    // Assign file I/O stream
    QTextStream lCsvFile(&lFile);

    // Check the compatibility
    QString lStrString;

    // If the file starts with an empty line, just ignore it.
    lStrString = lCsvFile.readLine();
    if(lStrString.isEmpty())
    {
        lStrString = lCsvFile.readLine();
    }
    // The file has to start with the section << TEST SUMMARY REPORT >>
    if (!lStrString.contains(cSectionTestSummaryReport))
    {
        // Close file
        lFile.close();
        return false;
    }
    lStrString = lCsvFile.readLine();
    // If the line is ======, just ignore it.
    if(lStrString.contains("======"))
    {
        lStrString = lCsvFile.readLine();
    }
    // This line has to start with "System"
    if (!lStrString.contains(cSystem))
    {
        // Close file
        lFile.close();
        return false;
    }
    lStrString = lCsvFile.readLine();
    // This line has to start with "Station"
    if (!lStrString.contains(cStation))
    {
        // Close file
        lFile.close();
        return false;
    }

    // Close file
    lFile.close();

    return true;
}


//////////////////////////////////////////////////////////////////////
// Read and Parse the SUM file
//////////////////////////////////////////////////////////////////////
bool VishayASESummarytoSTDF::ConvertoStdf(const QString& aInputFileName, QString& aFileNameSTDF)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString(" file %1").arg(aInputFileName).toLatin1().constData());

    if(WriteStdfFile(aInputFileName, aFileNameSTDF) != true)
    {
        QFile::remove(aFileNameSTDF);
        return false;
    }

    mLastError = errNoError;
    // Success parsing SUM file
    return true;
}


//////////////////////////////////////////////////////////////////////
// Create STDF file from SUM data parsed
//////////////////////////////////////////////////////////////////////
bool VishayASESummarytoSTDF::WriteStdfFile(const QString &aFileName, const QString& aFileNameSTDF)
{
    // Open SUM file
    QFile lFile(aFileName);
    if(!lFile.open( QIODevice::ReadOnly ))
    {
        // Failed Opening SUM file
        mLastError = errOpenFail;
        mLastErrorMessage = QString("Cannot open input file '%1' ").arg(aFileName);
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());

        // Convertion failed.
        return false;
    }
    // Assign file I/O stream
    QTextStream aInputFile(&lFile);

    GQTL_STDF::Stdf_MIR_V4 lMIRRecord;
    GQTL_STDF::Stdf_MRR_V4 lMRRRecord;
    GQTL_STDF::Stdf_SDR_V4 lSDRRecord;

    QString lSublotID;
    QString lString;
    QString lSortName;
    QString lSortBin;
    QString lSortCount;
    QString lSortPercent;

    // Extract the info from the file name
    // .SUM Filename Mapping (Filename format = (Vishay|VIY)_<MIR.PROC_ID>_<LotID.Sublot>_YYYYMMDDhhmmss.sum)
    lString = QFileInfo(aFileName).fileName();
    if(lString.startsWith("Vishay") || lString.startsWith("VIY"))
    {
        QStringList lInfo = lString.split("_");
        if(lInfo.count() == 4)
        {
            lMIRRecord.SetPROC_ID(lInfo.at(1));
            lSublotID = lInfo.at(2);
            lMIRRecord.SetLOT_ID( lSublotID.section( '.', 0 ) );
            lMIRRecord.SetSBLOT_ID(lSublotID);

        }
    }
    // Section << TEST SUMMARY REPORT >>
    lString = ReadLine(aInputFile);
    if(!lString.contains(cSectionTestSummaryReport))
    {
        mLastErrorMessage = QString("Cannot find section '%1' ").arg(cSectionTestSummaryReport);
        mLastError = errMissingData;
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
        // Close file
        lFile.close();
        return false;
    }

    while(aInputFile.atEnd() == false)
    {
        lString = ReadLine(aInputFile);

        if (lString.contains(cSectionStart))
            break;

        // System   :  STA2100 [ TDS014 ]
        if (lString.contains(cSystem))
        {
            lString = lString.section(cSeparator,1).trimmed();
            lString = lString.section(cSpace,0,0);
            lMIRRecord.SetNODE_NAM(cNodeName);
            lMIRRecord.SetTSTR_TYP(lString);
        }
        // Station  :  STATION1
        else if (lString.contains(cStation))
        {
            lString = lString.section(cSeparator,1).trimmed();
            lString = lString.section(cStation,1,1,QString::SectionCaseInsensitiveSeps);
            lMIRRecord.SetSTAT_NUM(lString.toUInt());
        }
        // Handler  :  HDS066
        else if (lString.contains(cHandler))
        {
            lString = lString.section(cSeparator,1).trimmed();
            lSDRRecord.SetHAND_ID(lString);
        }
        // Login    :  USER - OPERATOR,  MODE - OPERATOR
        else if (lString.contains(cLogin))
        {
            lString = lString.section(cSeparator,1).trimmed();
            if(lString.contains("USER - OPERATOR,  MODE - OPERATOR"))
            {
                lMIRRecord.SetOPER_NAM(lString);
            }
        }
        // Product  :  Product
        else if (lString.contains(cProduct))
        {
            lString = lString.section(cSeparator,1).trimmed();
            if(!lString.contains(cProduct))
            {
                lMIRRecord.SetPROC_ID(lString);
            }
        }
        // Job Name :  DC2_SUD50P06-15_08
        else if (lString.contains(cJobName))
        {
            lString = lString.section(cSeparator,1).trimmed();
            lMIRRecord.SetJOB_NAM(lString);
        }
        // Job Rev. :
        else if (lString.contains(cJobRev))
        {
            lString = lString.section(cSeparator,1).trimmed();
            lMIRRecord.SetJOB_REV(lString);
        }
        // Dvc Name :  SUD50P06-15-GE3
        else if (lString.contains(cDeviceName))
        {
            lString = lString.section(cSeparator,1).trimmed();
            lMIRRecord.SetPROC_ID(lString);
        }
        //Lot Id.  :  K15K261.1
        else if (lString.contains(cLotId))
        {
            lSublotID = lString.section(cSeparator,1).trimmed();
            lMIRRecord.SetLOT_ID(lSublotID.section( '.', 0, 0 ) );
            lMIRRecord.SetSBLOT_ID(lSublotID);
        }
        // Setup       .......   2018/07/19   05:35:32
        else if (lString.contains(cSetup))
        {
            lString = lString.remove(cSetup).remove(".").simplified();
            QStringList lDateList = (lString.section(' ', 0, 0)).split("/");
            QDate lDate;
            if (lDateList.count() == 3)
            {
                int lYear = lDateList[0].toInt();
                if (lYear < 70)
                    lYear += 2000;
                else if (lYear < 99)
                    lYear += 1900;

                if (! lDate.setDate(lYear, lDateList[1].toInt(), lDateList[2].toInt()))
                {
                    GSLOG(SYSLOG_SEV_WARNING, QString("Invalid date %1").arg(lString).toUtf8().constData());
                }
            }

            QTime lTime = QTime::fromString(lString.section(" ", 1, 1), "hh:mm:ss");
            lMIRRecord.SetSETUP_T(QDateTime(lDate, lTime,Qt::UTC).toTime_t());
        }
        // Test Start  .......   2018/07/19   05:36:19
        else if (lString.contains(cTestStart))
        {
            lString = lString.remove(cTestStart).remove(".").simplified();
            QStringList lDateList = (lString.section(' ', 0, 0)).split("/");
            QDate lDate;
            if (lDateList.count() == 3)
            {
                int lYear = lDateList[0].toInt();
                if (lYear < 70)
                    lYear += 2000;
                else if (lYear < 99)
                    lYear += 1900;

                if (! lDate.setDate(lYear, lDateList[1].toInt(), lDateList[2].toInt()))
                {
                    GSLOG(SYSLOG_SEV_WARNING, QString("Invalid date %1").arg(lString).toUtf8().constData());
                }
            }

            QTime lTime = QTime::fromString(lString.section(" ", 1, 1), "hh:mm:ss");
            mStartTime = QDateTime(lDate, lTime,Qt::UTC).toTime_t();
            lMIRRecord.SetSTART_T(mStartTime);
        }
        // Test End    .......   2018/07/19   06:40:57
        else if (lString.contains(cTestEnd))
        {
            lString = lString.remove(cTestEnd).remove(".").simplified();
            QStringList lDateList = (lString.section(' ', 0, 0)).split("/");
            QDate lDate;
            if (lDateList.count() == 3)
            {
                int lYear = lDateList[0].toInt();
                if (lYear < 70)
                    lYear += 2000;
                else if (lYear < 99)
                    lYear += 1900;

                if (! lDate.setDate(lYear, lDateList[1].toInt(), lDateList[2].toInt()))
                {
                    GSLOG(SYSLOG_SEV_WARNING, QString("Invalid date %1").arg(lString).toUtf8().constData());
                }
            }

            QTime lTime = QTime::fromString(lString.section(" ", 1, 1).remove(","), "hh:mm:ss");
            lMRRRecord.SetFINISH_T(QDateTime(lDate, lTime,Qt::UTC).toTime_t());
        }
        // Total Test Count  :           6453
        // Total Good Count  :           6451
        // Facility :  STATEC
        // Lot No.  :  K15K261.1
        // Lot Q'ty :  7812
        // Yield (TGC/TTC %) :          99.97
        // Report      .......   2018/07/19   07:40:25
    }

    // Check the validity of the file
    if(lSublotID.isEmpty())
    {
        // missing Count section
        mLastErrorMessage = QString("Cannot find valid entry '%1' ").arg(cLotId);
        mLastError = errMissingData;
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());

        // Close file
        lFile.close();
        return false;
    }
    if(mStartTime == 0)
    {
        // missing Count section
        mLastErrorMessage = QString("Cannot find valid entry '%1' ").arg(cTestStart);
        mLastError = errMissingData;
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());

        // Close file
        lFile.close();
        return false;
    }

    // Section << TEST BIN REPORT >>
    if(!lString.contains(cSectionTestBinReport))
    {
        mLastErrorMessage = QString("Cannot find section '%1' ").arg(cSectionTestBinReport);
        mLastError = errMissingData;
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());

        // Close file
        lFile.close();
        return false;
    }
    while(aInputFile.atEnd() == false)
    {
        lString = ReadLine(aInputFile);

        if (lString.contains(cSectionStart))
            break;

    }
    // Section << SORT PLAN REPORT >>
    if(!lString.contains(cSectionSortPlanReport))
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Cannot find section '%1' ")
                                        .arg(cSectionSortPlanReport).toLatin1().constData());
        mLastError = errMissingData;
        // Close file
        lFile.close();
        return false;
    }
    // S#    NAME              BIN     COUNT  PERCENT
    lString = ReadLine(aInputFile);
    if (!lString.contains(cSortBin)
            || !lString.contains(cSortCount)
            || !lString.contains(cSortName)
            || !lString.contains(cSortNum)
            || !lString.contains(cSortPercent))
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Invalid section '%1' ")
                                        .arg(cSectionSortPlanReport).toLatin1().constData());
        mLastError = errMissingData;
        // Close file
        lFile.close();
        return false;
    }

    QString lAbsolutePath = QFileInfo(lFile).absolutePath();
    // Check if converter_external_file exists
    bool lExternalFile = ConverterExternalFile::Exists(lAbsolutePath);

    if (!lExternalFile)
    {
        mLastError = errConverterExternalFile;
        mLastErrorMessage = "File " + lAbsolutePath + "/converter_external_file.xml" + " does not exist";
        // Close file
        lFile.close();
        return false;
    }

    QString lPromisFileName;
    QString lExternalFileFormat;
    QString lExternalFileError;

    mExternalFilePath = ConverterExternalFile::GetExternalFileName(lAbsolutePath);
        if(!ConverterExternalFile::GetPromisFile(lAbsolutePath,
                                             "final",
                                             "prod",
                                             lPromisFileName,
                                             lExternalFileFormat,
                                             lExternalFileError))
    {
        // Debug message
        QString lErrorString;
        lErrorString.sprintf("Cannot found the promis file %s",lPromisFileName.toLatin1().constData());
        GSLOG(SYSLOG_SEV_DEBUG, lErrorString.toLatin1().constData());
    }

    // Read promis files
    mPromisFilePath = lPromisFileName;
    if (!ReadPromisFile(lPromisFileName, lSublotID, ConverterExternalFile::GetExternalFileName( lAbsolutePath )))
    {
        mLastErrorMessage = appendPromisExceptionInfo();
        lFile.close();
        return false;
    }

    // Read bin mapping
    try
    {
        if(ReadBinMappingFile(QFileInfo(lFile).absolutePath(), mExternalFilePath) == false)
        {
            mLastError = errReadBinMapFile;
            lFile.close();
            return false;
        }
    }
    catch (const Qx::BinMapping::BinMappingExceptionForUserBase &lException)
    {
        mLastError = errReadBinMapFile;
        mLastErrorMessage = lException.what();
        // Close file
        lFile.close();
        return false;
    }

    // S#    NAME              BIN     COUNT  PERCENT
    // S1    19_KEL    (103)     3         2    0.03
    do
    {
        // Read lines
        lString = ReadLine(aInputFile).trimmed().simplified();
        if (!lString.isEmpty())
        {
            if (lString.startsWith("<<") || lString.startsWith("==") || lString.startsWith("--") )
            {
                break;
            }
            // Extract Sort Info
            QStringList lElements = lString.split(" ");
            short lNumberOfElements = lElements.size();
            if (lNumberOfElements < 5)
            {
                mLastError = errInvalidFormatParameter;
                mLastErrorMessage = QString("The input line: %1, doesn't match to expected format").arg(lString);
                GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
                return false;
            }

            lSortPercent = lElements[--lNumberOfElements];                 // The last element
            lSortCount   = lElements.at(--lNumberOfElements);             // The element just before last element
            lSortBin     = lElements.at(--lNumberOfElements);             // The last element - 2
            lSortName    = lString.section(" ", 1, --lNumberOfElements);    // From the second element to the last - 3

            int lBinCount = lSortCount.toInt();
            // if the count is 0, just ignore
            if (lBinCount == 0)
            {
                continue;
            }

            // If the BIN column contains the pass bin 1, it is the pass bin
            if (lSortBin == "1")
            {
                VishayASESummaryBinning lBinMap;
                lBinMap.mPass = true;
                // Bin 1 Sort Name will defined using the text contained after the '_' in the Test File Field
                // Example: 'DC1_SUP85N10-10', Sort Name = SUP85N10-10 for Bin 1(PASS)

                lBinMap.mName = cPass;
                lBinMap.mSoftBinNum = 1;
                lBinMap.mCount = lBinCount;
                mVishayASESummaryBinPerTestNumber.insert(1, lBinMap);
                continue;
            }

            // Get bin mapping from the final test file
            // File format is :
            //      19_KEL    (103)
            //      19 is the TestNum
            //      19_KEL is the TestName
            lSortName = lSortName.section("(",0,0).trimmed();

            bool lIsInt(false);
            int lTestNumber = lSortName.section("_",0,0).toInt(&lIsInt);

            if(!lIsInt || (lTestNumber == 0))
            {
                continue;
            }
            // Get bin mapping from the sort entries file
            try
            {
                const Qx::BinMapping::BinMapItemBase &lBinMapItem =
                        mBinMapSortEntriesStore->GetBinMapItemByTestNumber(lTestNumber);

                // If the Sort is not enabled, ignore it
                if(lBinMapItem.GetEnabled())
                {
                    ProcessBinMappingItemByTestName(lBinMapItem, lBinCount);
                }
                // Found the Test, continue
                continue;
            }
            catch (const Qx::BinMapping::BinMappingExceptionForUserBase &)
            {
                //If no number is present for a 'SortName' element
                // try with final test entries
            }
            // Get bin mapping from the final test file
            try
            {
                if(lTestNumber > 100 && lTestNumber <= 200)
                    lTestNumber -= 100;
                const Qx::BinMapping::BinMapItemBase &lBinMapItem =
                        mBinMapFinalTestStore->GetBinMapItemByTestNumber( lTestNumber );

                // If the Sort is not enabled, ignore it
                if(lBinMapItem.GetEnabled())
                {
                    ProcessBinMappingItemByTestNumber(lBinMapItem, lBinCount);
                }
                // Found the test, continue
                continue;
            }
            catch (const Qx::BinMapping::BinMappingExceptionForUserBase &lException)
            {
                //If no number is present for a 'FinalTest' element
                mLastError = errInvalidFormatParameter;
                mLastErrorMessage = lException.what();
                lFile.close();
                return false;
            }
        }
    }
    while(aInputFile.atEnd() == false); // Read all lines in file

    // now generate the STDF file...
    GQTL_STDF::StdfParse lStdfParser;
    if(lStdfParser.Open(aFileNameSTDF.toLatin1().constData(), STDF_WRITE) == false)
    {
        mLastError = errWriteSTDF;
        mLastErrorMessage = QString("Can't open the input output file' %1").arg(aFileNameSTDF);
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
        // Close file
        lFile.close();
        return false;
    }

    // Write MIR
    // Construct custom Galaxy USER_TXT
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    strUserTxt += ":ASE_SUMMARY";
    lMIRRecord.SetUSER_TXT(strUserTxt.toLatin1().constData());

    // Add all promis' fields
    if(!mProcID.isEmpty())
        lMIRRecord.SetPROC_ID(mProcID);
    if(!mPackage.isEmpty())
        lMIRRecord.SetROM_COD(mPackage);
    if(!mProductID.isEmpty())
        lMIRRecord.SetPART_TYP(mProductID);
    if(!mDateCode.isEmpty())
        lMIRRecord.SetDATE_COD(mDateCode);
    if(!mSiteLocation.isEmpty())
    {
        lMIRRecord.SetFACIL_ID(mSiteLocation);
        lMIRRecord.SetTEST_COD(mSiteLocation);
    }
    if (!mTesterType.isEmpty())
    {
        lMIRRecord.SetNODE_NAM(mTesterType);
        lMIRRecord.SetTSTR_TYP(mTesterType);
    }
    if(!mPackageType.isEmpty())
        lMIRRecord.SetPKG_TYP(mPackageType);
    // Write MIR
    lStdfParser.WriteRecord(&lMIRRecord);

    // Write SDR
    lStdfParser.WriteRecord(&lSDRRecord);

    QMap<int,VishayASESummaryBinning>::Iterator itTestNumber;
    for(itTestNumber = mVishayASESummaryBinPerTestNumber.begin(); itTestNumber != mVishayASESummaryBinPerTestNumber.end(); itTestNumber++)
    {
        WriteSBR(*itTestNumber, lStdfParser);
    }

    QMap<std::string,VishayASESummaryBinning>::Iterator itTestName;
    for(itTestName = mVishayASESummaryBinPerTestName.begin(); itTestName != mVishayASESummaryBinPerTestName.end(); itTestName++)
    {
        WriteSBR(*itTestName, lStdfParser);
    }

    // Bin mapping file is present OR BinNumber can be used for HBin and SBin
    for(itTestNumber = mVishayASESummaryBinPerTestNumber.begin(); itTestNumber != mVishayASESummaryBinPerTestNumber.end(); itTestNumber++)
    {
        WriteHBR(*itTestNumber, lStdfParser);
    }

    for(itTestName = mVishayASESummaryBinPerTestName.begin(); itTestName != mVishayASESummaryBinPerTestName.end(); itTestName++)
    {
        WriteHBR(*itTestName, lStdfParser);
    }

    // Write MRR
    lMRRRecord.SetDISP_COD(' ');
    lStdfParser.WriteRecord(&lMRRRecord);

    // Close STDF file.
    lStdfParser.Close();

    // Success
    return true;
}

void VishayASESummarytoSTDF::ProcessBinMappingItemByTestName(const Qx::BinMapping::BinMapItemBase &aBinMapItem, int aBinCount)
{
    std::string lTestName = aBinMapItem.GetTestName();

    if(mVishayASESummaryBinPerTestName.find(lTestName) == mVishayASESummaryBinPerTestName.end())
    {
        VishayASESummaryBinning lBinMap;
        lBinMap.mPass = (aBinMapItem.GetBinNumber() == 1);
        lBinMap.mName = aBinMapItem.GetBinName().c_str();
        lBinMap.mSoftBinNum = aBinMapItem.GetBinNumber();
        lBinMap.mCount = 0;

        mVishayASESummaryBinPerTestName.insert(lTestName, lBinMap);
    }

    mVishayASESummaryBinPerTestName[lTestName].mCount += aBinCount;
}


void VishayASESummarytoSTDF::ProcessBinMappingItemByTestNumber(const Qx::BinMapping::BinMapItemBase &aBinMapItem, int aBinCount)
{
    int lTestNumber = aBinMapItem.GetTestNumber();

    if(mVishayASESummaryBinPerTestNumber.find(lTestNumber) == mVishayASESummaryBinPerTestNumber.end())
    {
        VishayASESummaryBinning lBinMap;
        lBinMap.mPass = (aBinMapItem.GetBinNumber() == 1);
        lBinMap.mName = aBinMapItem.GetBinName().c_str();
        lBinMap.mSoftBinNum = aBinMapItem.GetBinNumber();
        lBinMap.mCount = 0;

        mVishayASESummaryBinPerTestNumber.insert(lTestNumber, lBinMap);
    }

    mVishayASESummaryBinPerTestNumber[lTestNumber].mCount += aBinCount;
}

bool VishayASESummarytoSTDF::ReadPromisFile(const QString &aPromisFile, const QString &aPromisKey, const QString &aConverterExternalFilePath )
{
    if(aPromisKey.isEmpty())
    {
        // Failed Opening Promis file
        mLastError = errReadPromisFile;
        mLastErrorMessage = "No Promis key defined";

        // Convertion failed.
        return false;
    }

    try
    {
        QString lPromisFilePath = QFileInfo( aPromisFile ).absoluteFilePath();
        QScopedPointer< Qx::BinMapping::PromisInterpreterBase > lPromisInterpreter;

        lPromisInterpreter.reset( Qx::BinMapping::PromisInterpreterFactory< Qx::BinMapping::promis_lvm_ft >
                                  ::MakePromisInterpreter( aPromisKey.toStdString(),
                                                           lPromisFilePath.toStdString(),
                                                           aConverterExternalFilePath.toStdString() ) );
        const Qx::BinMapping::PromisItemBase &lPromisItem = lPromisInterpreter->GetPromisItem();

        mDateCode = lPromisItem.GetDateCode().c_str();
        mTesterType = lPromisItem.GetEquipmentID().c_str();
        mPackage = lPromisItem.GetPackage().c_str();
        mProcID = lPromisItem.GetProductId().c_str();
        mProductID = lPromisItem.GetGeometryName().c_str();
        mSiteLocation = lPromisItem.GetSiteId().c_str();
        mPackageType = lPromisItem.GetPackageType().c_str();
    }
    catch( const std::exception &lException )
    {
        mLastError = errReadPromisFile;
        mLastErrorMessage = lException.what() + appendPromisExceptionInfo();
        return false;
    }

    return true;
}


bool VishayASESummarytoSTDF::ReadBinMappingFile(const QString& aInputFilePath, const QString &aExternalFilePath)
{
    QString lExternalFileName;
    // Read the final_test bin mapping
    if (!GetBinMappingFileName(aInputFilePath, cFinalTest, lExternalFileName))
    {
        mLastError = errReadBinMapFile;
        mLastErrorMessage = appendBinMappingExceptionInfo();
        return false;
    }

    try
    {
        mBinMapFinalTestStore.reset(Qx::BinMapping::BinMapStoreFactory<Qx::BinMapping::lvm_ft_ft>
                                                ::MakeBinMapStore(lExternalFileName.toStdString(),
                                                                  aExternalFilePath.toStdString()));

    }
    catch( const std::exception &lException )
    {
        mLastErrorMessage = lException.what();
        return false;
    }

    // Read the sort_entries bin mapping
    if (!GetBinMappingFileName(aInputFilePath, cSortEntries, lExternalFileName))
    {
        mLastError = errReadBinMapFile;
        mLastErrorMessage = "cannot read the bin map file in the directory: " + aInputFilePath;
        return false;
    }

    try
    {
        const QString &lConverterExternalFilePath = ConverterExternalFile::GetExternalFileName( aInputFilePath );
        mBinMapSortEntriesStore.reset(Qx::BinMapping::BinMapStoreFactory<Qx::BinMapping::lvm_ft_se_new>
                                                ::MakeBinMapStore(lExternalFileName.toStdString(),
                                                                  lConverterExternalFilePath.toStdString() ) );


    }
    catch( const std::exception &lException )
    {
        mLastErrorMessage = lException.what();
        return false;
    }

    return true;
}

bool VishayASESummarytoSTDF::GetBinMappingFileName(const QString& aExternalFilePath,
                                            const QString &aCategory,
                                            QString& lBinMapFileName)
{
    QString lExternalFileFormat;
    QString lExternalFileError;
    // Check if Test->Binning mapping file to overload softbin
    ConverterExternalFile::GetBinmapFile(aExternalFilePath,"final","prod",aCategory,lBinMapFileName,lExternalFileFormat,lExternalFileError);
    mBinMapFilePath = lBinMapFileName;
    if(lBinMapFileName.isEmpty())
    {
        if(lExternalFileError.isEmpty())
            lExternalFileError = "No 'BINMAP_FILE' file defined";
        if(!aCategory.isEmpty())
            lExternalFileError = lExternalFileError + " for " + aCategory + " category";
        mLastError = errMissingData;
        mLastErrorMessage = lExternalFileError;
        return false;
    }
    return true;
}


bool VishayASESummarytoSTDF::IsInTheFinalTestBinMapStore(int aNumber)
{
    if(mBinMapFinalTestStore.isNull()) return false;

    try
    {
        mBinMapFinalTestStore->GetBinMapItemByTestNumber(aNumber);
        return true;
    }
    catch(const Qx::BinMapping::BinMappingExceptionForUserBase &)
    {
        return false;
    }
}


bool VishayASESummarytoSTDF::IsInTheSortEntriesBinMapStore(int aNumber)
{
    // TODO - SebL - refacto (GCORE-15312) demonstrates this function never worked as intended, it always raised an
    // exception. Fix it asap with the author.
    /**
    if(mBinMapSortEntiresStore.isNull()) return false;

    try
    {
        mBinMapSortEntiresStore->GetBinMapItemByTestNumber(aNumber);
        return true;
    }
    catch(Qx::BinMapping::BinMapItemNotFound&)
    {
        return false;
    }
    */

    ( void ) aNumber;
    return false;
}


void VishayASESummarytoSTDF::WriteHBR(VishayASESummaryBinning& lItem, GQTL_STDF::StdfParse& aStdfParser)
{
    mHBRRecord.Reset();

    if(lItem.mCount <= 0) return;

    mHBRRecord.SetHEAD_NUM(255);
    mHBRRecord.SetSITE_NUM(0);
    mHBRRecord.SetHBIN_NUM(lItem.mSoftBinNum);
    mHBRRecord.SetHBIN_CNT(lItem.mCount);

    if(lItem.mPass)
        mHBRRecord.SetHBIN_PF('P');
    else
        mHBRRecord.SetHBIN_PF('F');

    mHBRRecord.SetHBIN_NAM(lItem.mName);

    aStdfParser.WriteRecord(&mHBRRecord);
}

void VishayASESummarytoSTDF::WriteSBR(VishayASESummaryBinning& lItem, GQTL_STDF::StdfParse& aStdfParser)
{
    mSBRRecord.Reset();

    if(lItem.mCount <= 0) return;

    mSBRRecord.SetHEAD_NUM(255);
    mSBRRecord.SetSITE_NUM(0);
    mSBRRecord.SetSBIN_NUM(lItem.mSoftBinNum);
    mSBRRecord.SetSBIN_CNT(lItem.mCount);

    if(lItem.mPass)
        mSBRRecord.SetSBIN_PF('P');
    else
        mSBRRecord.SetSBIN_PF('F');

    mSBRRecord.SetSBIN_NAM(lItem.mName);

    aStdfParser.WriteRecord(&mSBRRecord);
}

//////////////////////////////////////////////////////////////////////
// Skip empty line
//////////////////////////////////////////////////////////////////////
void  VishayASESummarytoSTDF::SpecificReadLine (QString &aString)
{
    // Ignore line 	===============================================
    if(aString.contains(cSectionEqualLine))
    {
        aString = "";
    }
    // Ignore line -----------------------------------------------
    else if(aString.contains(cSectionMinusLine))
    {
        aString = "";
    }
}

}
}
