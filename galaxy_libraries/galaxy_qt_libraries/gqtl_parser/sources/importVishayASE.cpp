//////////////////////////////////////////////////////////////////////
// importVishayASE.cpp: Convert a Vishay ASE file to STDF V4.0
//////////////////////////////////////////////////////////////////////
#include "importVishayASE.h"
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
//TESEC Spektra Test System
// 07/22/16 11:27:23
//Station	A
//Test by	Branch
//Polarity	NPN
//Test File	DC1_SUP85N10-10
//Lot Name	H21D166.1
//Device Name	SUP85N10
//Opeator
//Comment
//Bin	Sort Name	Count	Percent
//3	19 CONT        	0
//4	GDS SH         	0
//4	S-OPEN         	1
//4	G-OPEN         	0
//5	21 IDSS        	12




const QString cTestSystem = "Test System";
const QString cStation = "Station";
const QString cTestBy = "Test by";
const QString cPolarity = "Polarity";
const QString cLotName = "Lot Name";
const QString cDeviceName = "Device Name";
const QString cOperator = "Operator";
const QString cSortEntries = "sort_entries";
const QString cFinalTest = "final_tests";
const QString cTestFile = "Test File";
const QString cPass = "PASS";
const QString cSeparator = ",";

namespace GS
{
namespace Parser
{


//////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////
VishayASEtoSTDF::VishayASEtoSTDF(): ParserBase(typeVishayASE, "typeVishayASE")
{
}

//////////////////////////////////////////////////////////////////////
// Check if File is compatible with VISHAY ASE format
//////////////////////////////////////////////////////////////////////
bool VishayASEtoSTDF::IsCompatible(const QString &aFileName)
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
    if (!lStrString.contains(cTestSystem))
    {
        // Close file
        lFile.close();
        return false;
    }

    // This line has to contains the date, just ignore it.
    lCsvFile.readLine();

    // This line has to start with "Station"
    lStrString = lCsvFile.readLine();
    if (!lStrString.contains(cStation))
    {
        // Close file
        lFile.close();
        return false;
    }

    // This line has to start with "Test by"
    lStrString = lCsvFile.readLine();
    if (!lStrString.contains(cTestBy))
    {
        // Close file
        lFile.close();
        return false;
    }


    // This line has to start with "Polarity"
    lStrString = lCsvFile.readLine();
    if (!lStrString.contains(cPolarity))
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
bool VishayASEtoSTDF::ConvertoStdf(const QString& aInputFileName, QString &aFileNameSTDF)
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
bool VishayASEtoSTDF::WriteStdfFile(const QString &aFileName, const QString& aFileNameSTDF)
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

    // Extract the Test System
    QString lString = ReadLine(aInputFile);
    if (lString.contains(cTestSystem))
    {
        lString = lString.remove(cTestSystem).remove(",").trimmed();
        lMIRRecord.SetTSTR_TYP(lString);
    }

    // extract the date "MM/DD/YY hh:mm:ss"
    lString = ReadLine(aInputFile).trimmed();

    if (lString.split(' ', QString::SkipEmptyParts).size() < 2)
    {
        // Close STDF file.
        lStdfParser.Close();
        GSLOG(SYSLOG_SEV_ERROR, QString("Error in the date format %1, it has to be like MM/DD/YY hh:mm:ss")
                                        .arg(lString).toLatin1().constData());
        mLastError = errWriteSTDF;
        // Close file
        lFile.close();
        return false;
    }
    QStringList lDateList = (lString.section(' ', 0, 0)).split("/");
    QDate lDate;
    if (lDateList.count() == 3)
    {
        int lYear = lDateList[2].toInt();
        if (lYear < 70)
            lYear += 2000;
        else if (lYear < 99)
            lYear += 1900;

        if (! lDate.setDate(lYear, lDateList[0].toInt(), lDateList[1].toInt()))
        {
            GSLOG(SYSLOG_SEV_WARNING, QString("Invalid date %1").arg(lString).toUtf8().constData());
        }
    }

    QTime lTime = QTime::fromString(lString.section(" ", 1, 1).remove(","), "hh:mm:ss");
    mStartTime = QDateTime(lDate, lTime,Qt::UTC).toTime_t();
    lMIRRecord.SetSTART_T(mStartTime);
    lMIRRecord.SetSETUP_T(mStartTime);

    // This line has to start with "Station"
    lString = ReadLine(aInputFile);
    if (lString.contains(cStation) && lString.split(cSeparator, QString::SkipEmptyParts).size() == 2)
    {
        QString lFirstItem = lString.section(cSeparator, 1, 1);
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

    // ignore the line "Test by"
    lString = ReadLine(aInputFile);

    // ignore the line "Polarity"
    lString = ReadLine(aInputFile);

    // ignore the line "Test File" because the binning information for the pass bin will be catched in the bin section
    lString = ReadLine(aInputFile).trimmed().simplified();
    QString lPassBinName;
    if (lString.contains(cTestFile, Qt::CaseInsensitive))
    {
        lPassBinName = lString.section(",", 1, 1);
        if (!lPassBinName.contains("_"))
        {
            mLastError = errInvalidFormatParameter;
            mLastErrorMessage = "Unvalid test name " + lPassBinName + ". It doesn't contain _";
            lFile.close();
            return false;
        }
        lPassBinName = lPassBinName.section("_", 1);
    }
    else
    {
        mLastError = errInvalidFormatParameter;
        mLastErrorMessage = "No field Test File";
        lFile.close();
        return false;
    }

    // Read Lot line
    QString lSublotID, lLotID;
    lString = ReadLine(aInputFile);
    if (lString.contains(cLotName))
    {
        lString = lString.remove(cLotName).remove(",").trimmed();
        QStringList lLotIdList = lString.split(".");
        if (lLotIdList.size() != 2)
        {
            mLastError = errInvalidFormatParameter;
            mLastErrorMessage = "The line " + lString + " is not correct. It has to contain the lot.subLot";
            // Close file
            lFile.close();
            return false;
        }
        lLotID = lLotIdList[0];
        lSublotID = lString;
        lMIRRecord.SetLOT_ID(lLotID);
        lMIRRecord.SetSBLOT_ID(lSublotID);
    }
    else
    {
        mLastError = errInvalidFormatParameter;
        mLastErrorMessage = "The line " + lString + " is not correct. It has to contain the lot.subLot";
        // Close file
        lFile.close();
        return false;
    }

    // Read Device name
    lString = ReadLine(aInputFile);
    if (lString.contains(cDeviceName))
    {
        lString = lString.remove(cDeviceName).remove(",").trimmed();
        lMIRRecord.SetJOB_NAM(lString);
    }
    else
    {
        mLastError = errInvalidFormatParameter;
        mLastErrorMessage = "The line " + lString + " is not correct. It has to contain the Device Name";
        // Close file
        lFile.close();
        return false;
    }

    // Read Operator: This is an optional field
    lString = ReadLine(aInputFile);
    if (lString.contains(cOperator))
    {
        lString = lString.remove(cOperator).remove(",").trimmed();
        lMIRRecord.SetOPER_NAM(lString);
    }

    // Construct custom Galaxy USER_TXT
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    strUserTxt += ":ASE";
    lMIRRecord.SetUSER_TXT(strUserTxt.toLatin1().constData());

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
    if (!ReadPromisFile(lPromisFileName, lSublotID, ConverterExternalFile::GetExternalFileName( lAbsolutePath )))
    {
        // Close file
        lFile.close();
        return false;
    }

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
    if(!mPackageType.isEmpty())
        lMIRRecord.SetPKG_TYP(mPackageType);

    if(!mTesterType.isEmpty() )
    {
        lMIRRecord.SetNODE_NAM(mTesterType);
        lMIRRecord.SetTSTR_TYP(mTesterType);
    }

    // Write MIR
    lStdfParser.WriteRecord(&lMIRRecord);

    // Read until the Bin section
    do
    {
        lString = ReadLine(aInputFile).trimmed();
    }
    while(aInputFile.atEnd() == false && !lString.startsWith("Bin"));

    // Read bin mapping
    try
    {
        if(lExternalFile)
        {
            if(ReadBinMappingFile(QFileInfo(lFile).absolutePath()) == false)
            {
                mLastError = errReadBinMapFile;
                lFile.close();
                return false;
            }
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

    // Read binning
    do
    {
        // Read lines while empty
        lString = ReadLine(aInputFile);
        while (lString.trimmed().isEmpty() && !aInputFile.atEnd())
        {
            lString = ReadLine(aInputFile);
        }

        if (!lString.trimmed().isEmpty())
        {    // Split line if not empty
            QStringList lFields = lString.split(cSeparator);
            if (lFields.size() < 3)
                continue;

            int lBinCount = lFields[2].toInt();
            // if the line contains Total, just ignore it
            if (lFields[0].compare("Total", Qt::CaseInsensitive) == 0)
            {
                continue;
            }
            // If the second column contains the pass bin name, it is the pass bin
            if (lFields[0].toInt() == 1)
            {
                // if the count is 0, just continue, you will have an other bin1
                if (lBinCount == 0)
                {
                    continue;
                }
                else
                {
                    VishayASEBinning lBinMap;
                    lBinMap.mPass = true;
                    // Bin 1 Sort Name will defined using the text contained after the '_' in the Test File Field
                    // Example: 'DC1_SUP85N10-10', Sort Name = SUP85N10-10 for Bin 1(PASS)

                    lBinMap.mName = cPass;
                    lBinMap.mSoftBinNum = 1;
                    lBinMap.mCount = lBinCount;
                    mVishayASEBinPerTestNumber.insert(1, lBinMap);
                }
                continue;
            }

            // Get bin mapping from the final test file
            if(lExternalFile)
            {
                QStringList lTestNames = lFields[1].simplified().split(" ");
                if(lTestNames.size() == 0)
                {
                    mLastError = errInvalidFormatParameter;
                    mLastErrorMessage = "Incorrect sort name parameter found in input file within this line: " + lString;
                    lFile.close();
                    return false;
                }
                QStringList lTestNameSplit = lTestNames[0].split('_', QString::SkipEmptyParts);
                bool lIsInt(false);
                int lTestNumber = lTestNameSplit[0].toInt(&lIsInt);

                //ignore cases
                if(!lIsInt || lTestNumber == 0 || lTestNameSplit.size() == 1 || lTestNameSplit[1] == "REJ")
                    continue;

                if (lTestNumber < 1000) //Get bin mapping from the final test file
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
                    catch (const Qx::BinMapping::BinMappingExceptionForUserBase &lException)
                    {
                        //If no number is present for a 'finaltest' element, reject file
                        mLastError = errReadBinMapFile;//errInvalidFormatParameter
                        mLastErrorMessage = lException.what();
                        lFile.close();
                        return false;
                    }
                }
                else //lTestNumber >= 1000 -> Get bin mapping from the sort entries file
                {
                    try
                    {
                        const Qx::BinMapping::BinMapItemBase &lBinMapItem =
                                mBinMapSortEntiresStore->GetBinMapItemByTestNumber(lTestNumber);

                        if(lBinMapItem.GetEnabled() == true)
                                ProcessBinMappingItemByTestNumber(lBinMapItem, lBinCount);
                    }
                    catch (const Qx::BinMapping::BinMappingExceptionForUserBase &lException)
                    {
                        //If no number is present for a 'SortName' element, reject file
                        mLastError = errReadBinMapFile;//errInvalidFormatParameter
                        mLastErrorMessage = lException.what();
                        lFile.close();
                        return false;
                    }
                }
            }
        }
    }
    while(aInputFile.atEnd() == false); // Read all lines in file

    QMap<int,VishayASEBinning>::Iterator itTestNumber;
    for(itTestNumber = mVishayASEBinPerTestNumber.begin(); itTestNumber != mVishayASEBinPerTestNumber.end(); itTestNumber++)
    {
        WriteSBR(*itTestNumber, lStdfParser);
    }

    QMap<std::string,VishayASEBinning>::Iterator itTestName;
    for(itTestName = mVishayASEBinPerTestName.begin(); itTestName != mVishayASEBinPerTestName.end(); itTestName++)
    {
        WriteSBR(*itTestName, lStdfParser);
    }

    // Bin mapping file is present OR BinNumber can be used for HBin and SBin
    for(itTestNumber = mVishayASEBinPerTestNumber.begin(); itTestNumber != mVishayASEBinPerTestNumber.end(); itTestNumber++)
    {
        WriteHBR(*itTestNumber, lStdfParser);
    }

    for(itTestName = mVishayASEBinPerTestName.begin(); itTestName != mVishayASEBinPerTestName.end(); itTestName++)
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

void VishayASEtoSTDF::ProcessBinMappingItemByTestName(const Qx::BinMapping::BinMapItemBase &aBinMapItem, int aBinCount)
{
    std::string lTestName = aBinMapItem.GetTestName();

    if(mVishayASEBinPerTestName.find(lTestName) == mVishayASEBinPerTestName.end())
    {
        VishayASEBinning lBinMap;
        lBinMap.mPass = (aBinMapItem.GetBinNumber() == 1);
        lBinMap.mName = aBinMapItem.GetBinName().c_str();
        lBinMap.mSoftBinNum = aBinMapItem.GetBinNumber();
        lBinMap.mCount = 0;

        mVishayASEBinPerTestName.insert(lTestName, lBinMap);
    }

    mVishayASEBinPerTestName[lTestName].mCount += aBinCount;
}


void VishayASEtoSTDF::ProcessBinMappingItemByTestNumber(const Qx::BinMapping::BinMapItemBase &aBinMapItem, int aBinCount)
{
    int lTestNumber = aBinMapItem.GetTestNumber();

    if(mVishayASEBinPerTestNumber.find(lTestNumber) == mVishayASEBinPerTestNumber.end())
    {
        VishayASEBinning lBinMap;
        lBinMap.mPass = (aBinMapItem.GetBinNumber() == 1);
        lBinMap.mName = aBinMapItem.GetBinName().c_str();
        lBinMap.mSoftBinNum = aBinMapItem.GetBinNumber();
        lBinMap.mCount = 0;

        mVishayASEBinPerTestNumber.insert(lTestNumber, lBinMap);
    }

    mVishayASEBinPerTestNumber[lTestNumber].mCount += aBinCount;
}

bool VishayASEtoSTDF::ReadPromisFile(const QString &aPromisFile, const QString &aPromisKey, const QString &aConverterExternalFilePath )
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
        QScopedPointer< Qx::BinMapping::PromisInterpreterBase > lPromisInterpreter;

        mExternalFilePath = aConverterExternalFilePath;
        mPromisFilePath = aPromisFile;
        lPromisInterpreter.reset( Qx::BinMapping::PromisInterpreterFactory< Qx::BinMapping::promis_lvm_ft >
                                  ::MakePromisInterpreter( aPromisKey.toStdString(),
                                                           aPromisFile.toStdString(),
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
        mLastErrorMessage = /*lException.what() +*/ appendPromisExceptionInfo();
        return false;
    }

    return true;
}


bool VishayASEtoSTDF::ReadBinMappingFile(const QString& aInputFilePath)
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
        mBinMapSortEntiresStore.reset(Qx::BinMapping::BinMapStoreFactory<Qx::BinMapping::lvm_ft_se_new>
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

bool VishayASEtoSTDF::GetBinMappingFileName(const QString& aExternalFilePath,
                                            const QString &aCategory,
                                            QString& lBinMapFileName)
{
    QString lExternalFileFormat;
    QString lExternalFileError;
    // Check if Test->Binning mapping file to overload softbin
    ConverterExternalFile::GetBinmapFile(aExternalFilePath,"final","prod",aCategory,lBinMapFileName,lExternalFileFormat,lExternalFileError);
    mBinMapFilePath = lBinMapFileName;
    mExternalFilePath = ConverterExternalFile::GetExternalFileName(aExternalFilePath);
    if(lBinMapFileName.isEmpty())
    {
        if(lExternalFileError.isEmpty())
            lExternalFileError = appendBinMappingExceptionInfo();
        if(!aCategory.isEmpty())
            lExternalFileError = lExternalFileError + " for " + aCategory + " category";
        mLastError = errMissingData;
        mLastErrorMessage = lExternalFileError;
        return false;
    }
    return true;
}


bool VishayASEtoSTDF::IsInTheFinalTestBinMapStore(int aNumber)
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


bool VishayASEtoSTDF::IsInTheSortEntriesBinMapStore(int aNumber)
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


void VishayASEtoSTDF::WriteHBR(VishayASEBinning& lItem, GQTL_STDF::StdfParse& aStdfParser)
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

void VishayASEtoSTDF::WriteSBR(VishayASEBinning& lItem, GQTL_STDF::StdfParse& aStdfParser)
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
void  VishayASEtoSTDF::SpecificReadLine (QString &aString)
{
    if(aString.left(3) == ",,," && (aString.simplified().count(",")==aString.simplified().length()))
        aString = "";
}

//////////////////////////////////////////////////////////////////////
// Check if the line is empty or contains only ','
//////////////////////////////////////////////////////////////////////
bool VishayASEtoSTDF::EmptyLine(const QString& aLine)
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
