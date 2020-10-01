//////////////////////////////////////////////////////////////////////
// importVishayATM.cpp: Convert a Vishay ATM (Amkor) file to STDF V4.0
//////////////////////////////////////////////////////////////////////
#include "importVishayATM.h"
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
//Lot Id : EJ23V002.2    Device:SiRA10BDP
//============================================================================
// PowerTech Tester        Update Time:2/9/2018 5:50:48 PM      Auto-Clear Counter When then window be closed
// Station <A>             Counter file name:       Log data file name:
// FileName: <C:\Users\PowerTECH\Desktop\vishay\FR32B-0003.01.ptf>
// Comment :
// Pass    :          159  25.32 %
// Fail    :          469  74.68 %
// Total   :          628

//============================================================================
// M#  TID   ItemName                                       Fail     Percent
//----------------------------------------------------------------------------
//  1  19    KEL                                              0       0.00 %
//  2  7     IGSS                                             0       0.00 %
//  3  27    IDSS                                             0       0.00 %
//  4  79    BVDSS                                            0       0.00 %
//  5  80    VDSP                                             0       0.00 %
//.
//.
//.
//============================================================================
// S# Bin# SortComment     Pass/Fail         Counter    Percent
//----------------------------------------------------------------------------
//  1    7 KELVIN             Fail                 0      0.00 %
//  2    1 S-OPEN             Fail                 0      0.00 %
//  3    1 G-OPEN             Fail                 0      0.00 %
//.
//.


const QString cLotID = "Lot Id";
const QString cDevice = "Device";
const QString cStation = "Station";
const QString cMSection = "M#";
const QString cSSection = "Sbin";
const QString cUpdateTime = "Update Time";
const QString cSpace = " ";
const QString cPoint = ".";
const QString cDoublePoint = ":";
const QString cLotName = "LotId";
const QString cSortEntries = "sort_entries";
const QString cFinalTest = "final_tests";
const QString cFail = "Fail";
const QString cPass = "Pass";
const QString cQPlus = "Q+";

namespace GS
{
namespace Parser
{


//////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////
VishayATMtoSTDF::VishayATMtoSTDF(): ParserBase(typeVishayATM, "typeVishayATM")
{
}

//////////////////////////////////////////////////////////////////////
// Check if File is compatible with VISHAY ATM format
//////////////////////////////////////////////////////////////////////
bool VishayATMtoSTDF::IsCompatible(const QString &aFileName)
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
    QString lStrString = lCsvFile.readLine();
    if (!lStrString.contains(cLotID) || !lStrString.contains(cDevice) )
    {
        // Close file
        lFile.close();
        return false;
    }

    // Check if we can found the string Update Time in the first 10 lines
    short lNbreLine(0);
    while (!lStrString.contains(cUpdateTime) && !lCsvFile.atEnd() && (lNbreLine < 10) )
    {
        lStrString = lCsvFile.readLine();
        ++lNbreLine;
    }

    // if we are at the end of the file or we have reached more than 10 lines, exit
    if (lCsvFile.atEnd() || lNbreLine >= 10)
    {
        // Close file
        lFile.close();
        return false;
    }

    // This line has to start with "Station"
    lStrString = lCsvFile.readLine();
    if (!lStrString.contains(cStation))
    {
        // Close file
        lFile.close();
        return false;
    }


    // Check if we can found the M# section in the next 15 lines
    lNbreLine = 0;
    while (!lStrString.contains(cMSection) && !lCsvFile.atEnd() && (lNbreLine < 15) )
    {
        lStrString = lCsvFile.readLine();
        ++lNbreLine;
    }


    // if we are at the end of the file or we have reached more than 15 lines, exit
    if (lCsvFile.atEnd() || lNbreLine >= 15)
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
// Read and Parse the CSV file
//////////////////////////////////////////////////////////////////////
bool VishayATMtoSTDF::ConvertoStdf(const QString& aInputFileName, QString &aFileNameSTDF)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString(" file %1").arg(aInputFileName).toLatin1().constData());

    if(WriteStdfFile(aInputFileName, aFileNameSTDF) != true)
    {
        QFile::remove(aFileNameSTDF);
        return false;
    }

    mLastError = errNoError;
    // Success parsing CSV file
    return true;
}


//////////////////////////////////////////////////////////////////////
// Create STDF file from CSV data parsed
//////////////////////////////////////////////////////////////////////
bool VishayATMtoSTDF::WriteStdfFile(const QString &aFileName, const QString& aFileNameSTDF)
{
    // Open CSV file
    QFile lFile(aFileName);
    if(!lFile.open( QIODevice::ReadOnly ))
    {
        // Failed Opening CSV file
        mLastError = errOpenFail;

        // Convertion failed.
        return false;
    }
    QFileInfo lFileInfo(aFileName);
    QString lDataFilePath = lFileInfo.absolutePath();

    // Assign file I/O stream
    QTextStream aInputFile(&lFile);

    // now generate the STDF file...
    GQTL_STDF::StdfParse lStdfParser;

    if(lStdfParser.Open(aFileNameSTDF.toLatin1().constData(), STDF_WRITE) == false)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Can't open the input output file' %1")
                                        .arg(aFileNameSTDF).toLatin1().constData());
        mLastError = errWriteSTDF;
        // Close file
        lFile.close();
        return false;
    }

    // Write MIR
    GQTL_STDF::Stdf_MIR_V4 lMIRRecord;
    QString lStrString = ReadLine(aInputFile);
    QString lLotID, lSubLotID;
    //Lot Id : EJ23V002.2    Device:SiRA10BDP
    // lotId = EJ23V002; SubLotId = EJ23V002.2; MIR.PROC_ID = SiRA10BDP
    if (lStrString.contains(cLotID) && lStrString.contains(cDevice))
    {
        QString lString = lStrString.section(cDevice, 0, 0);
        lString.remove(cSpace);
        lString.remove(cLotName);
        lString.remove(cDoublePoint);
        QStringList lLotIdList = lString.split(cPoint);
        if (lLotIdList.size() != 2)
        {
            mLastError = errInvalidFormatParameter;
            mLastErrorMessage = "The line " + lStrString + " is not correct. It has contain the lot.subLot";
            // Close file
            lFile.close();
            return false;
        }
        lLotID = lLotIdList[0];
        lSubLotID = lString;
        lMIRRecord.SetLOT_ID(lLotID);
        lMIRRecord.SetSBLOT_ID(lSubLotID);

        // Get device info
        lString = lStrString.section(cDevice, 1, 1).section(' ', 0, 0);
        lString.remove(cDoublePoint);
        lMIRRecord.SetPROC_ID(lString.trimmed());
    }


    // Check if we can found the string Update Time in the first 10 lines
    short lNbreLine(0);
    while (!lStrString.contains(cUpdateTime) && !aInputFile.atEnd() && (lNbreLine < 10) )
    {
        lStrString = ReadLine(aInputFile);
        ++lNbreLine;
    }

    // if we are at the end of the file or we have reached more than 10 lines, exit
    if (aInputFile.atEnd() || lNbreLine >= 10)
    {
        mLastError = errInvalidFormatParameter;
        mLastErrorMessage = "can not find the updateTime line";
        // Close file
        lFile.close();
        return false;
    }

    // PowerTech Tester        Update Time:2/9/2018 5:50:48 PM      Auto-Clear Counter When then window be closed
    lStrString = lStrString.trimmed().simplified();
    QString lString = lStrString.section("Tester", 0, 0);
    lMIRRecord.SetNODE_NAM(lString);
    lMIRRecord.SetTSTR_TYP(lString);

    // extract the date "MM/DD/YY hh:mm:ss"
    QString lDateString = lStrString.section(" ", 2, 5);    // lDateString = "Update Time:2/9/2018 5:50:48 PM"
    lDateString = lDateString.section(":", 1);              // lDateString = "2/9/2018 5:50:48 PM"

    if (lDateString.split(" ", QString::SkipEmptyParts).size() < 2)
    {
        // Close STDF file.
        lStdfParser.Close();
        GSLOG(SYSLOG_SEV_ERROR, QString("Error in the date format %1, it has to be like MM/DD/YY hh:mm:ss")
                                        .arg(lDateString).toLatin1().constData());
        mLastError = errWriteSTDF;
        // Close file
        lFile.close();
        return false;
    }
    QStringList lDateList = (lDateString.section(" ", 0, 0)).split("/");
    QDate lDate;
    if (lDateList.count() == 3)
    {
        int lYear = lDateList[2].toInt();
        if (lYear < 70)
            lYear += 2000;
        else if (lYear < 99)
            lYear += 1900;

        if (!lDate.setDate(lYear, lDateList[0].toInt(), lDateList[1].toInt()))
        {
            GSLOG(SYSLOG_SEV_WARNING, QString("Invalid date %1").arg(lDateString).toUtf8().constData());
        }
    }
    QString lFormat("hh:mm:ss AP");
    QString lTimeString = lDateString.section(' ', 1);  //  lTimeString = "5:50:48 PM"
    // the format of the hours can be only on 1 degit and not on 2: h:mm:ss or hh:mm:ss
    if (lTimeString.section(":", 0, 0).size() == 1)
    {
        lFormat = "h:mm:ss AP";
    }
    QTime lTime = QTime::fromString(lTimeString, lFormat);
    mStartTime = QDateTime(lDate, lTime,Qt::UTC).toTime_t();
    lMIRRecord.SetSTART_T(mStartTime);
    lMIRRecord.SetSETUP_T(mStartTime);

    // This line has to start with "Station"
    lString = ReadLine(aInputFile).trimmed().simplified();
    if (lString.contains(cStation) && lString.split(" ", QString::SkipEmptyParts).size() >= 2)
    {
        QString lFirstItem = lString.section(" ", 1, 1).remove ("<").remove (">");
        char lStation = (lFirstItem.toStdString())[0]; // We are sure that the item 1 has at list one character
        stdf_type_u1 lStationNumber;
        // In some case, we have a letter instead of a number. We have to convert it like A=1, B=2, ...,Z=26
        if (lStation >= 'A' && lStation <='Z')
        {
            lStationNumber = lStation - 'A' + 1;
        }
        else
        {
            lStationNumber = lStation;
        }
        lMIRRecord.SetSTAT_NUM(lStationNumber);
    }

    // Construct custom Galaxy USER_TXT
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    strUserTxt += ":AMKOR";
    lMIRRecord.SetUSER_TXT(strUserTxt.toLatin1().constData());

    // Check if converter_external_file exists
    bool lExternalFile = ConverterExternalFile::Exists(lDataFilePath);

    if(lExternalFile)
    {
        QString lPromisFileName;
        QString lExternalFileFormat;
        QString lExternalFileError;

        if(!ConverterExternalFile::GetPromisFile(lDataFilePath,
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
        if (!ReadPromisFile(lPromisFileName, lSubLotID, ConverterExternalFile::GetExternalFileName( lDataFilePath )))
        {
            // Close file
            lFile.close();
            return false;
        }


        // Add all promis' fields (GCORE-17179: only if they are present in promis file)
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
        if(!mTesterType.isEmpty())
        {
            lMIRRecord.SetTSTR_TYP(mTesterType);
            lMIRRecord.SetNODE_NAM(mTesterType);
        }
        if(!mPackageType.isEmpty())
            lMIRRecord.SetPKG_TYP(mPackageType);
    }
    else
    {
        mLastError = errReadBinMapFile;
        mLastErrorMessage = "can't find the external file";
        // Close file
        lFile.close();
        return false;
    }


    // Write MIR
    lStdfParser.WriteRecord(&lMIRRecord);

    // Read until the Bin section
    do
    {
        lString = ReadLine(aInputFile).trimmed();
    }
    while(aInputFile.atEnd() == false && !lString.startsWith(cSSection));

    if(lExternalFile)
    {
        // Read bin mapping
        try
        {
            if(ReadBinMappingFile(QFileInfo(lFile).absolutePath()) == false)
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
    }

    // Read binning
    do
    {
        // Read lines while empty
        lString = ReadLine(aInputFile).trimmed().simplified();
        while (lString.trimmed().isEmpty() && !aInputFile.atEnd())
        {
            lString = ReadLine(aInputFile).trimmed().simplified();
        }

        // 14    3 IGSS_4             Fail                45      7.17 %
        // We don't have a separator in the line. We have only the Pass or Fail to separate the first part and the second one
        if (!lString.isEmpty())
        {
            QString lSeparator = cFail;
            // Split line if not empty
            if (lString.contains(cPass, Qt::CaseInsensitive))
            {
                lSeparator = cPass;
                QStringList lFields = lString.split(lSeparator);
                if (lFields.size() < 2)
                {
                    continue;
                }

                // lFields[lFields.size() - 1] will contain  45 7.17 %
                int lBinCount = (lFields[lFields.size() - 1]).trimmed().section(" ", 0, 0).toInt();
                if (lBinCount == 0)
                {
                    continue;
                }

                VishayATMBinning lBinMap;
                lBinMap.mPass = true;
                // Bin 1 Parts will have the SortComment field of 'DC3 PASS'.  Other Pass bins should be ignored.
                // Example: 'DC3 PASS', Sort Name = SDC3 for Bin 1(PASS)
                // force the bin name to "PASS"
                lBinMap.mName = cPass.toUpper();
                lBinMap.mSoftBinNum = 1;
                lBinMap.mCount = lBinCount;
                mVishayATMBinPerBinNumber.insert(1, lBinMap);
                continue;
            }

            // Here, the separator is Fail
            if (!lString.contains(cFail, Qt::CaseInsensitive))
            {
                continue;
            }

            // the first party is like "15 2 DC2 D-S SHORT" or "17 3 BVDSS_41"
            // If a sort name field contains a underscore and a number '_##', the CSVFinalTestsFile (or database table) should be used to perform the lookup
            // If a sort name field does not contain a number, the CSVCSVSortEntries (or database table) column C should be used to perform the lookup
            QString lFirstPart = lString.section(lSeparator, 0, 0).simplified();

            // The second party is like "275 43.79 %"
            QString lSecondPart = lString.section(lSeparator, 1).simplified();
            int lBinCount = lSecondPart.section(" ", 0, 0).toInt();
            // Any bin with a part quantity of 0 should be ignored
            if (lBinCount == 0)
            {
                continue;
            }

            QString lBinName = lFirstPart.section(" ", 2).simplified();
            // Ignore 'SortComment' elements which contain '_Q+' or 'Q+'.  These are QA tests and should not be included in the part totals.
            if (lBinName.endsWith(cQPlus, Qt::CaseInsensitive))
            {
                continue;
            }


            if(lExternalFile)
            {
                // check if the last 3 letters in the name has the format: "_XX" or "_X": X is a number
                QString lLastThreeChar = lBinName.section("_", -1); // start from the right
                bool lIsInt(false);
                int lTestNumber = lLastThreeChar.toInt(&lIsInt);

                // ignore the test if is  not a number or the test# = 0
                if (!lIsInt || lTestNumber == 0)
                {
                    continue;
                }
                if (lLastThreeChar.size() <= 3)
                {
                    try
                    {
                        if(lTestNumber > 100 && lTestNumber <= 200)
                            lTestNumber -= 100;

                        const Qx::BinMapping::BinMapItemBase &lBinMapItem =
                                mBinMapFinalTestStore->GetBinMapItemByTestNumber( lTestNumber );

                        if(lBinMapItem.GetEnabled() == true)
                            ProcessBinMappingItemByTestNumber(lBinMapItem, lBinCount);
                    }
                    catch (const Qx::BinMapping::BinMappingExceptionForUserBase &)
                    {
                        try
                        {
                            if(lTestNumber > 100 && lTestNumber <= 200)
                                lTestNumber -= 100;

                            const Qx::BinMapping::BinMapItemBase &lBinMapItem =
                                    mBinMapFinalTestStore->GetBinMapItemByTestNumber( lTestNumber );

                            if(lBinMapItem.GetEnabled() == true)
                                ProcessBinMappingItemByTestNumber(lBinMapItem, lBinCount);
                        }
                        catch (const Qx::BinMapping::BinMappingExceptionForUserBase & )
                        {
                            mLastError = errInvalidFormatParameter;
                            mLastErrorMessage = "Found unknown test number " +
                                                QString::number(lTestNumber) +
                                                " in the bin map file.";
                            lFile.close();
                            return false;
                        }
                    }
                }
                // Get bin mapping from the sort entries file
                else
                {
                    try
                    {
                        const Qx::BinMapping::BinMapItemBase &lBinMapItem =
                                mBinMapSortEntriesStore->GetBinMapItemByTestNumber(lTestNumber);

                        if(lBinMapItem.GetEnabled() == true)
                            ProcessBinMappingItemByTestName(lBinMapItem, lBinCount);
                    }
                    catch (const Qx::BinMapping::BinMappingExceptionForUserBase &)
                    {
                        mLastError = errInvalidFormatParameter;
                        mLastErrorMessage = "Found unknown test number " +
                                            QString::number(lTestNumber) +
                                            " in the bin map file.";
                        lFile.close();
                        return false;
                    }
                }
            }
        }
    }
    while(aInputFile.atEnd() == false); // Read all lines in file

    QMap<int,VishayATMBinning>::Iterator itTestNumber;
    for(itTestNumber = mVishayATMBinPerBinNumber.begin(); itTestNumber != mVishayATMBinPerBinNumber.end(); itTestNumber++)
    {
        WriteSBR(*itTestNumber, lStdfParser);
    }

    QMap<std::string,VishayATMBinning>::Iterator itTestName;
    for(itTestName = mVishayATMBinPerTestName.begin(); itTestName != mVishayATMBinPerTestName.end(); itTestName++)
    {
        WriteSBR(*itTestName, lStdfParser);
    }

    // Bin mapping file is present OR BinNumber can be used for HBin and SBin
    for(itTestNumber = mVishayATMBinPerBinNumber.begin(); itTestNumber != mVishayATMBinPerBinNumber.end(); itTestNumber++)
    {
        WriteHBR(*itTestNumber, lStdfParser);
    }

    for(itTestName = mVishayATMBinPerTestName.begin(); itTestName != mVishayATMBinPerTestName.end(); itTestName++)
    {
        WriteHBR(*itTestName, lStdfParser);
    }


    // Write MRR
    GQTL_STDF::Stdf_MRR_V4 lMRRRecord;
    // use the last date in the csv file
    lMRRRecord.SetFINISH_T(mStartTime);
    lMRRRecord.SetDISP_COD(' ');
    lStdfParser.WriteRecord(&lMRRRecord);

    // Close STDF file.
    lStdfParser.Close();

    // Success
    return true;
}

void VishayATMtoSTDF::ProcessBinMappingItemByTestName(const Qx::BinMapping::BinMapItemBase &aBinMapItem, int aBinCount)
{
    std::string lTestName = aBinMapItem.GetTestName();

    if(mVishayATMBinPerTestName.find(lTestName) == mVishayATMBinPerTestName.end())
    {
        VishayATMBinning lBinMap;
        lBinMap.mPass = (aBinMapItem.GetBinNumber() == 1);
        lBinMap.mName = aBinMapItem.GetBinName().c_str();
        lBinMap.mSoftBinNum = aBinMapItem.GetBinNumber();
        lBinMap.mCount = 0;

        mVishayATMBinPerTestName.insert(lTestName, lBinMap);
    }

    mVishayATMBinPerTestName[lTestName].mCount += aBinCount;
}


void VishayATMtoSTDF::ProcessBinMappingItemByTestNumber(const Qx::BinMapping::BinMapItemBase &aBinMapItem, int aBinCount)
{
    int lBinNumber = aBinMapItem.GetBinNumber();

    if(mVishayATMBinPerBinNumber.find(lBinNumber) == mVishayATMBinPerBinNumber.end())
    {
        VishayATMBinning lBinMap;
        lBinMap.mPass = (aBinMapItem.GetBinNumber() == 1);
        lBinMap.mName = aBinMapItem.GetBinName().c_str();
        lBinMap.mSoftBinNum = aBinMapItem.GetBinNumber();
        lBinMap.mCount = 0;

        mVishayATMBinPerBinNumber.insert(lBinNumber, lBinMap);
    }

    mVishayATMBinPerBinNumber[lBinNumber].mCount += aBinCount;
}


bool VishayATMtoSTDF::GetInputFiles(const QString& aInputFileName, QString &aPromisFilePath, QString &aBinMapFilePath)
{
    aPromisFilePath = aBinMapFilePath = "";

    // Get Promis File
    QString lExternalFileName, lExternalFileFormat, lExternalFileError;
    ConverterExternalFile::GetPromisFile(aInputFileName,
                                         "final",
                                         "prod",
                                         lExternalFileName,
                                         lExternalFileFormat,
                                         lExternalFileError);

    if(lExternalFileName.isEmpty())
    {
        if(lExternalFileError.isEmpty())
        {
            lExternalFileError = "No 'GEX_PROMIS_DATA' file defined";
        }
        mLastError = errMissingData;
        mLastErrorMessage = lExternalFileError;
        return false;
    }
    if (!QFile::exists(lExternalFileName))
    {
        mLastError = errReadPromisFile;
        mLastErrorMessage = "Unable to find promis file: " + lExternalFileName;
        return false;
    }
    aPromisFilePath = lExternalFileName;

    // Get Bin Map File
    lExternalFileName = lExternalFileFormat = lExternalFileError = "";
    ConverterExternalFile::GetBinmapFile(aInputFileName,
                                         "final",
                                         "prod",
                                         lExternalFileName,
                                         lExternalFileFormat,
                                         lExternalFileError);

    if(lExternalFileName.isEmpty())
    {
        if(lExternalFileError.isEmpty())
        {
            lExternalFileError = "No 'GEX_BINMAP_FILE' file defined";
        }
        mLastError = errMissingData;
        mLastErrorMessage = lExternalFileError;
        return false;
    }
    if (!QFile::exists(lExternalFileName))
    {
        mLastError = errReadPromisFile;
        mLastErrorMessage = "Unable to find Bin Map file: " + lExternalFileName;
        return false;
    }
    aBinMapFilePath = lExternalFileName;

    return true;
}

bool VishayATMtoSTDF::ReadPromisFile(const QString &aPromisFile, const QString &aPromisKey, const QString &aConverterExternalFilePath )
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
        mLastErrorMessage = lException.what();
        return false;
    }

    return true;
}

bool VishayATMtoSTDF::ReadBinMappingFile(const QString& aInputFilePath)
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
                                                                  mExternalFilePath.toStdString() ) );

    }
    catch( const std::exception &lException )
    {
        mLastErrorMessage = lException.what() + appendBinMappingExceptionInfo();
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
        mBinMapSortEntriesStore.reset(Qx::BinMapping::BinMapStoreFactory<Qx::BinMapping::lvm_ft_se_new>
                                                ::MakeBinMapStore(lExternalFileName.toStdString(),
                                                                  mExternalFilePath.toStdString() ) );


    }
    catch( const std::exception &lException )
    {
        mLastErrorMessage = lException.what();
        return false;
    }

    return true;
}

bool VishayATMtoSTDF::GetBinMappingFileName(const QString& aExternalFilePath,
                                            const QString &aCategory,
                                            QString& lBinMapFileName)
{
    QString lExternalFileFormat;
    QString lExternalFileError;
    // Check if Test->Binning mapping file to overload softbin
    mExternalFilePath = ConverterExternalFile::GetExternalFileName(aExternalFilePath);
    ConverterExternalFile::GetBinmapFile(aExternalFilePath, "final","prod",aCategory,lBinMapFileName,lExternalFileFormat,lExternalFileError);
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


bool VishayATMtoSTDF::IsInTheFinalTestBinMapStore(int aNumber)
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


bool VishayATMtoSTDF::IsInTheSortEntriesBinMapStore(int aNumber)
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


void VishayATMtoSTDF::WriteHBR(VishayATMBinning& lItem, GQTL_STDF::StdfParse& aStdfParser)
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

    mHBRRecord.SetHBIN_NAM(lItem.mName.toLatin1().constData());

    aStdfParser.WriteRecord(&mHBRRecord);
}

void VishayATMtoSTDF::WriteSBR(VishayATMBinning& lItem, GQTL_STDF::StdfParse& aStdfParser)
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

    mSBRRecord.SetSBIN_NAM(lItem.mName.toLatin1().constData());

    aStdfParser.WriteRecord(&mSBRRecord);
}


//////////////////////////////////////////////////////////////////////
// Skip empty line
//////////////////////////////////////////////////////////////////////
void  VishayATMtoSTDF::SpecificReadLine (QString &aString)
{
    if(aString.left(3) == ",,," && (aString.simplified().count(",")==aString.simplified().length()))
        aString = "";
}

//////////////////////////////////////////////////////////////////////
// Check if the line is empty or contains only ','
//////////////////////////////////////////////////////////////////////
bool VishayATMtoSTDF::EmptyLine(const QString& aLine)
{
    bool lEmpty(true);
    if (!aLine.isEmpty())
    {
        QStringList lCells = aLine.split(",", QString::KeepEmptyParts);
        for(int lIndexCell=0; lIndexCell<lCells.count(); ++lIndexCell)
        {
            if (!lCells[lIndexCell].isEmpty())
            {
                lEmpty = false;
                break;
            }
        }
    }
    return lEmpty;
}

}
}
