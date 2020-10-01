#include <QFileInfo>
#include <QStringBuilder>

#include "import_fet_test_summary.h"
#include "converter_external_file.h"
#include "bin_map_store_factory.h"
#include "gqtl_log.h"
#include "bin_map_item_base.h"
#include "promis_interpreter_factory.h"
#include "promis_item_base.h"

const QString cFinalTest = "final_tests";
const QString cSortEntries = "sort_entries";
const QString cPassBin = "PASS";

namespace GS
{
namespace Parser
{

FetTestSummaryToStdf::FetTestSummaryToStdf() : ParserBase( typeFetTestSummary, "typeFetTestSummary" )
{
    mStartTime = mStopTime = 0;
    mHaveBinmapFile = false;
    mTotalGoodParts = mTotalParts = 0;
}

bool FetTestSummaryToStdf::IsCompatible(const QString &aFilePath)
{
    QFile linputFile( aFilePath );
    if(!linputFile.open( QIODevice::ReadOnly ))
    {
        // Failed Opening ASL1000 file
        return false;
    }

    // Assign file I/O stream
    QTextStream hFile(&linputFile);

    //Lot#,Tester,IF8/9,LCR,UIS,TR,Promis In,Promis Out,OTI In
    const QString lFirstLine = hFile.readLine().simplified().remove(' ').toUpper();
    if(!lFirstLine.startsWith("LOT#,TESTER,"))
    {
        linputFile.close();
        return false;
    }

    //...
    //Final Tests,,,,,,,Sort Entries,
    int lLine = 0;
    while(!hFile.atEnd())
    {
        const QString lCurrentLine = hFile.readLine().simplified().remove(' ').remove(',').toUpper();
        const QString lMatchingString("FINALTESTSSORTENTRIES");

        if(lCurrentLine.isEmpty())
            continue;
        else if(lCurrentLine.startsWith(lMatchingString))
        {
            linputFile.close();
            return true;
        }
        ++lLine;
        if(lLine >= 10)
            break;
    }

    // Incorrect header...this is not a FetTestSummary file!
    linputFile.close();

    return false;
}

QString FetTestSummaryToStdf::GetRealExternalFilePath( const QString &aFileName )
{
    if( ! QFile( aFileName ).open( QIODevice::ReadOnly ) )
    {
        const QString lPrefixedPath = mDataFilePath % '/' % aFileName;
        if( QFile( lPrefixedPath ).open( QIODevice::ReadOnly ) )
            return lPrefixedPath;
        return QString();
    }

    return aFileName;
}

bool FetTestSummaryToStdf::GetBinMappingFileName(const QString &aCategory, QString& lBinMapFileName)
{
    QString lExternalFileFormat;
    QString lExternalFileError;
    // Check if Test->Binning mapping file to overload softbin
    mExternalFilePath = ConverterExternalFile::GetExternalFileName(mDataFilePath);
    ConverterExternalFile::GetBinmapFile(mDataFilePath,"final","prod",aCategory,lBinMapFileName,lExternalFileFormat,lExternalFileError);
    mBinMapFilePath = lBinMapFileName;
    if(lBinMapFileName.isEmpty())
    {
        if(lExternalFileError.isEmpty())
            lExternalFileError = "No 'BINMAP_FILE' file defined";
        if(!aCategory.isEmpty())
            lExternalFileError = lExternalFileError % " for " % aCategory % " category";
        mLastError = errMissingData;
        mLastErrorMessage = lExternalFileError;
        return false;
    }
    return true;
}
bool FetTestSummaryToStdf::ReadBinmapFiles()
{
    // Read the final_test bin mapping
    QString lExternalFileName;
    if (!GetBinMappingFileName(cFinalTest, lExternalFileName))
    {
        return false;
    }
    try
    {
        mBinMapFinalTestStore.reset( Qx::BinMapping::BinMapStoreFactory< Qx::BinMapping::lvm_ft_ft >
                            ::MakeBinMapStore( lExternalFileName.toStdString(),
                                               mExternalFilePath.toStdString() ) );
    }
    catch(const std::exception &lException)
    {
        mLastErrorMessage = lException.what() + appendBinMappingExceptionInfo();
        mLastError = errReadBinMapFile;
        return false;
    }

    // Read the sort_entries bin mapping
    if (!GetBinMappingFileName(cSortEntries, lExternalFileName))
    {
        return false;
    }
    try
    {
        mBinMapSortEntriesStore.reset(Qx::BinMapping::BinMapStoreFactory<Qx::BinMapping::lvm_ft_se>
                                            ::MakeBinMapStore(lExternalFileName.toStdString(),
                                                              mExternalFilePath.toStdString() ) );
    }
    catch(const std::exception &lException)
    {
        mLastErrorMessage = lException.what();
        mLastError = errReadBinMapFile;
        return false;
    }

    mHaveBinmapFile = true;
    return true;
}

bool FetTestSummaryToStdf::ReadFetTestSummaryFile(const QString &aFilePath)
{
    // Open CSV file
    QFile f( aFilePath );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening FetTestSummary file
        mLastError = errOpenFail;
        mLastErrorMessage = f.errorString();

        // Convertion failed.
        return false;
    }

    // Assign file I/O stream
    QTextStream hFile(&f);


    //  Lot#,Tester,IF8/9,LCR,UIS,TR,Promis In,Promis Out,OTI In
    //  C14D213.6,1,11,11,20,0,34059,33817,34090
    //  ,,,,,,,,
    //  ,,,,,,,,
    //  ,,,,,,,,
    //  Final Tests,,,,,,,Sort Entries,
    //  Kelvin  19,17,,,,,,GDS sh,0

    QString lString;
    QString lHeaderSections;
    QString lHeaderValues;
    QString lSection;

    lHeaderSections = ReadLine(hFile);
    lHeaderValues = ReadLine(hFile);

    if(!lHeaderSections.startsWith("Lot#,Tester",Qt::CaseInsensitive))
    {
        mLastError = errInvalidFormatParameter;
        mLastErrorMessage = "Line 'Lot#,Tester' not found";

        f.close();

        return false;
    }
    // Read FetTestSummary information
    while(!lHeaderSections.isEmpty())
    {
        lSection = lHeaderSections.section(',',0,0).remove(' ').toUpper();
        lString = lHeaderValues.section(',',0,0).simplified();

        lHeaderSections = lHeaderSections.section(',',1);
        lHeaderValues = lHeaderValues.section(',',1);

        static const QString sLOTSHARP("LOT#");
        static const QString sTESTER("TESTER");
        static const QString sIF89("IF8/9");
        static const QString sLCR("LCR");
        static const QString sUIS("UIS");
        static const QString sTR("TR");
        static const QString sPROMISIN("PROMISIN");
        static const QString sPROMISOUT("PROMISOUT");
        static const QString sOTIIN("OTIIN");
        if(lSection == sLOTSHARP)
        {
            mPromisLotId = lString;
            mSubLotId = lString.section('.',1).simplified();
            mLotId = lString.section('.',0,0);
        }
        else if(lSection == sTESTER)
        {
            static const QString sMMINUS("M-");
            mNodeNam = mHandId = sMMINUS % lString.simplified();
        }
        else if(lSection == sIF89)
        {
            static const QString sIMINUS("I-");
            mCardId = sIMINUS % lString.simplified();
        }
        else if(lSection == sLCR)
        {
            static const QString sLMINUS("L-");
            mDibId = sLMINUS % lString.simplified();
        }
        else if(lSection == sUIS)
        {
            static const QString sUMINUS("U-");
            mLoadId = sUMINUS % lString.simplified();
        }
        else if(lSection == sTR)
        {
            static const QString sTRMINUS("TR-");
            mCableId = sTRMINUS % lString.simplified();
        }
        else if(lSection == sPROMISIN)
        {
            if(mTotalParts == 0)
                mTotalParts = lString.simplified().toInt();
        }
        else if(lSection == sPROMISOUT)
        {
            mTotalGoodParts = lString.simplified().toInt();

            CGFetTestSummaryBinning clBinMap;
            clBinMap.Pass = true;
            clBinMap.Name = cPassBin;
            clBinMap.HardBinNum = 1;
            clBinMap.SoftBinNum = 1;
            clBinMap.Count = mTotalGoodParts;

            mFetTestSummaryBinPerTestName.insert(clBinMap.Name.toStdString(), clBinMap);

        }
        else if(lSection == sOTIIN)
        {
            bool bIsNum;
            lString.simplified().toInt(&bIsNum);
            if(bIsNum)
                mTotalParts = lString.simplified().toInt(&bIsNum);
        }
    }

    mStartTime = QFileInfo(aFilePath).created().toTime_t();
    mStopTime = QFileInfo(aFilePath).lastModified().toTime_t();

    while(!hFile.atEnd())
    {
        lString = ReadLine(hFile);

        lSection = lString.section(',',0,0).remove(' ').toUpper();
        lString = lString.section(',',1).remove(',').remove(' ').toUpper().simplified();

        if(lString.isEmpty())
            continue;

        break;
    }

    if((lSection != "FINALTESTS") || (lString != "SORTENTRIES"))
    {
        mLastError = errInvalidFormatParameter;
        mLastErrorMessage = "Line 'Final Tests,,,,,,,Sort Entries' not found";

        f.close();

        return false;
    }

    //  Final Tests,,,,,,,Sort Entries,
    //  Kelvin  19,17,,,,,,GDS sh,0

    int       lBinCount;
    bool      lIsValidNum;

    while(!hFile.atEnd())
    {
        lString = ReadLine(hFile).simplified().toUpper();

        if(lString.isEmpty())
            continue;

        // Extract Finat Test binning
        lBinCount  = lString.section(',',1,1).simplified().toInt(&lIsValidNum);

        if(lIsValidNum)
        {
            // Extract test number Ex: Kelvin 19/119  =>  1 test number - 19
            int lTestNumber;
            if(ExtractTestNumber(lTestNumber, lString) == false)
            {
                f.close();
                return false;
            }

            //GCORE-16590: B_FT_[L|H]VM mapped parsers must ignore 0
            if(lTestNumber == 0)
            {
                continue;
            }

            // -- Search if data to match with final test bin mapping
            try
            {
                const Qx::BinMapping::BinMapItemBase &lBinMapItem =
                        mBinMapFinalTestStore->GetBinMapItemByTestNumber( lTestNumber );

                ProcessBinMappingItemByTestNumber(lBinMapItem, lBinCount);
            }
            catch (const Qx::BinMapping::BinMappingExceptionForUserBase &)
            {
                mLastError = errInvalidFormatParameter;
                mLastErrorMessage = QString("Unable to find test# %1 in Final Tests bin mapping file").arg(lTestNumber);
                return false;
            }
        }

        // -- Search if data to match with sort entry
        QString lTestName = lString.section(',',7,7).simplified();
        lBinCount  = lString.section(',',8,8).simplified().toInt(&lIsValidNum);

        /// GCORE-15113 (Eli comment)
        /// 1) Ignore 0 Sort Entry counts, meaning we should not add them to the converter file or insert them
        /// 2) If Sort Entry is valid but doesn't have a valid lookup, we should also ignore this entry (do not add to output .std)
        /// and display a warning during the insertion that this entry was not added to the file.
        /// Do not reject for Sort Entries.  Only Invalid final tests should cause a rejection
        if(lIsValidNum && lBinCount > 0 )
        {
            try
            {
                const Qx::BinMapping::BinMapItemBase &lBinMapItem =
                        mBinMapSortEntriesStore->GetBinMapItemByTestName( lTestName.toStdString() );

                ProcessBinMappingItemByTestName(lBinMapItem, lBinCount);
            }
            catch (const Qx::BinMapping::BinMappingExceptionForUserBase &)
            {
                GSLOG(SYSLOG_SEV_WARNING,
                      qPrintable(QString("The Sort Entry %1 doens't have a valid lookup.").arg(lTestName)) );
                continue;
            }
        }
    }

    // Success parsing FetTestSummary file
    f.close();
    return true;
}


bool FetTestSummaryToStdf::ExtractTestNumber(int& aOutTestNumber, QString& aStringToExtract)
{
    QString lTestNumber = aStringToExtract.section(',',0,0).section(' ',1,1).simplified();

    if(lTestNumber.contains('/'))
    {
        QStringList lOutTestNumbers = lTestNumber.split('/');

        bool lConvert = false;
        int lFirstTestNumber = lOutTestNumbers[0].toInt(&lConvert);
        if(lConvert == false)
        {
            mLastError = errInvalidFormatParameter;
            mLastErrorMessage = QStringLiteral("Conversion of %1 to integer failed").arg(lOutTestNumbers[0]);
            return false;
        }

        int lSecondTestNumber = lOutTestNumbers[1].toInt(&lConvert);
        if(lConvert == false)
        {
            mLastError = errInvalidFormatParameter;
            mLastErrorMessage = QStringLiteral("Conversion of %1 to integer failed").arg(lOutTestNumbers[1]);
            return false;
        }

        //-- When two test number, need to check some spec rules
        // -- rule 1
        if(lFirstTestNumber > 100)
        {
             mLastError = errInvalidFormatParameter;
             mLastErrorMessage = "First test number must be less or equal to 100";

             return false;
        }

        //--rule 2
        if( (lFirstTestNumber + 100) != lSecondTestNumber)
        {
            mLastError = errInvalidFormatParameter;
            mLastErrorMessage = QStringLiteral("Final Test Entry %1 map to different bin numbers and names. "
                                "Please make sure that each entry maps to a single bin number and name").arg(lTestNumber);

            return false;
        }

        aOutTestNumber = lFirstTestNumber;
    }
    else
    {
        bool lInteger = false;
        aOutTestNumber = lTestNumber.toInt(&lInteger);

        if (lInteger == false)
        {
            mLastError = errInvalidFormatParameter;
            mLastErrorMessage = QStringLiteral("Conversion of %1 to integer failed").arg(lTestNumber);
            return false;
        }

        if(aOutTestNumber > 100 && aOutTestNumber <= 200)
        {
             mLastError = errInvalidFormatParameter;
             mLastErrorMessage = "Single test number must be lesser than 101 and greater than 200 ";

             return false;
        }
    }

    return true;

}

void FetTestSummaryToStdf::ProcessBinMappingItemByTestName(const Qx::BinMapping::BinMapItemBase &aBinMapItem, int aBinCount)
{
    std::string lTestName = aBinMapItem.GetTestName();

    if(mFetTestSummaryBinPerTestName.find(lTestName) == mFetTestSummaryBinPerTestName.end())
    {
        CGFetTestSummaryBinning clBinMap;
        clBinMap.Pass = (aBinMapItem.GetBinNumber() == 1);
        clBinMap.Name = aBinMapItem.GetBinName().c_str();
        clBinMap.HardBinNum = aBinMapItem.GetBinNumber();
        clBinMap.SoftBinNum = aBinMapItem.GetBinNumber();
        clBinMap.Count = 0;

        mFetTestSummaryBinPerTestName.insert(lTestName, clBinMap);
    }

    mFetTestSummaryBinPerTestName[lTestName].Count += aBinCount;
}


void FetTestSummaryToStdf::ProcessBinMappingItemByTestNumber(const Qx::BinMapping::BinMapItemBase &aBinMapItem, int aBinCount)
{
    int lTestNumber = aBinMapItem.GetTestNumber();

    if(mFetTestSummaryBinPerTestNumber.find(lTestNumber) == mFetTestSummaryBinPerTestNumber.end())
    {
        CGFetTestSummaryBinning clBinMap;
        clBinMap.Pass = (aBinMapItem.GetBinNumber() == 1);
        clBinMap.Name = aBinMapItem.GetBinName().c_str();
        clBinMap.HardBinNum = aBinMapItem.GetBinNumber();
        clBinMap.SoftBinNum = aBinMapItem.GetBinNumber();
        clBinMap.Count = 0;

        mFetTestSummaryBinPerTestNumber.insert(lTestNumber, clBinMap);
    }

    mFetTestSummaryBinPerTestNumber[lTestNumber].Count += aBinCount;
}


bool FetTestSummaryToStdf::ReadPromisDataFile()
{
    // Check if converter_external_file exists
    QString lExternalFileName;
    QString lExternalFileFormat;
    QString lExternalFileError;
    ConverterExternalFile::GetPromisFile(mDataFilePath,"final","prod",lExternalFileName,lExternalFileFormat,lExternalFileError);
    mPromisFilePath = lExternalFileName;
    if(lExternalFileName.isEmpty())
    {
        if(lExternalFileError.isEmpty())
            lExternalFileError = appendPromisExceptionInfo();
        mLastError = errMissingData;
        mLastErrorMessage = lExternalFileError;
        return false;
    }

    if(mPromisLotId.isEmpty())
    {
        // Failed Opening FetTestSummary file
        mLastError = errMissingData;
        mLastErrorMessage = "No Promis Lot Id defined";

        // Convertion failed.
        return false;
    }

    // CLear/Set variables to default
    mFacilityId = "";
    mSiteLocation = "";
    mTesterType = "";
    mDateCode = "";
    mPackage = "";
    mProcId = "";
    mPackageType = "";
    mPromisLotId_D2 = "";
    mGeometryName_D2 = "";
    mPromisLotId_D3 = "";
    mGeometryName_D3 = "";
    mPromisLotId_D4 = "";
    mGeometryName_D4 = "";

    try
    {
        mPromisInterpreter.reset( Qx::BinMapping::PromisInterpreterFactory< Qx::BinMapping::promis_lvm_ft >
                                  ::MakePromisInterpreter( mPromisLotId.toStdString(),
                                                           lExternalFileName.toStdString(),
                                                           ConverterExternalFile::GetExternalFileName( mDataFilePath )
                                                           .toStdString() ) );

        mDateCode           = mPromisInterpreter->GetPromisItem().GetDateCode().c_str();
        mPackage            = mPromisInterpreter->GetPromisItem().GetPackage().c_str();
        mTesterType         = mPromisInterpreter->GetPromisItem().GetEquipmentID().c_str();
        mProcId             = mPromisInterpreter->GetPromisItem().GetProductId().c_str();
        mFacilityId = mSiteLocation = mPromisInterpreter->GetPromisItem().GetSiteId().c_str();
        mProductId          = mPromisInterpreter->GetPromisItem().GetGeometryName().c_str();
        mPackageType        = mPromisInterpreter->GetPromisItem().GetPackageType().c_str();
        mPromisLotId_D2     = mPromisInterpreter->GetPromisItem().GetGeometryNameP2().c_str();
        mGeometryName_D2    = mPromisInterpreter->GetPromisItem().GetLotIdP2().c_str();
        mPromisLotId_D3     = mPromisInterpreter->GetPromisItem().GetGeometryNameP3().c_str();
        mGeometryName_D3    = mPromisInterpreter->GetPromisItem().GetLotIdP3().c_str();
        mPromisLotId_D4     = mPromisInterpreter->GetPromisItem().GetGeometryNameP4().c_str();
        mGeometryName_D4    = mPromisInterpreter->GetPromisItem().GetLotIdP4().c_str();
    }
    catch( const std::exception &lException )
    {
        mLastErrorMessage = lException.what();
        return false;
    }

    return true;
}

bool FetTestSummaryToStdf::ConvertoStdf(const QString &aInputFilePath, QString &aOutputFilePath)
{
    // No erro (default)
    mLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(aInputFilePath);
    QFileInfo fOutput(aOutputFilePath);

    if((fOutput.exists() == true) && (fInput.lastModified() < fOutput.lastModified()))
        return true;

    bool bStatus = true;

    QFileInfo cFile(aInputFilePath);
    mDataFilePath = cFile.absolutePath();

    if(!ReadBinmapFiles())
    {
        bStatus = false;    // Not set... return false to indicate no data was read
        mLastErrorMessage = mLastErrorMessage % ". \nFetTest Summary FT Bin mapping cannot be retrieved";

    }

    if(bStatus)
        bStatus = ReadFetTestSummaryFile(aInputFilePath);

    if(bStatus && !ReadPromisDataFile())
    {
        bStatus = false;    // Not set... return false to indicate no data was read
        mLastErrorMessage = mLastErrorMessage % ". \nFetTest Summary Promis data cannot be retrieved";
    }

    // Generate STDF file dynamically.
    if(bStatus)
        bStatus = WriteStdfFile(aOutputFilePath);

    if(!bStatus)
        QFile::remove(aOutputFilePath);

    return bStatus;
}

bool FetTestSummaryToStdf::WriteStdfFile(const QString &aStdfFilePath)
{
    // now generate the STDF file...
    GS::StdLib::Stdf lStdfFile;
    if(lStdfFile.Open(aStdfFilePath.toLatin1().constData(),STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        // Failed importing CSV file into STDF database
        mLastError = errWriteSTDF;

        // Convertion failed.
        return false;
    }

    lStdfFile.SetStdfCpuType(lStdfFile.GetComputerCpuType());

    // Write FAR
    GQTL_STDF::Stdf_FAR_V4 lFARrecord;
    lFARrecord.SetCPU_TYPE(lStdfFile.GetComputerCpuType());	 // Force CPU type to current computer platform.
    lFARrecord.SetSTDF_VER(4);                               // STDF V4
    lFARrecord.Write(lStdfFile);

    if(mStartTime <= 0)
        mStartTime = QDateTime::currentDateTime().toTime_t();

    // Write MIR
    GQTL_STDF::Stdf_MIR_V4 lMIRRecord;
    lMIRRecord.SetSETUP_T(mStartTime);
    lMIRRecord.SetSTART_T(mStartTime);
    lMIRRecord.SetSTAT_NUM(1);
    lMIRRecord.SetMODE_COD('P');
    lMIRRecord.SetRTST_COD(' ');
    lMIRRecord.SetPROT_COD(' ');
    lMIRRecord.SetBURN_TIM(65535);
    lMIRRecord.SetCMOD_COD(' ');
    lMIRRecord.SetLOT_ID(mLotId.toLatin1().constData());
    if(!mProductId.isEmpty())
        lMIRRecord.SetPART_TYP(mProductId.toLatin1().constData());
    if(!mNodeNam.isEmpty())
        lMIRRecord.SetNODE_NAM(mNodeNam.toLatin1().constData());
    if(!mTesterType.isEmpty())
        lMIRRecord.SetTSTR_TYP(mTesterType.toLatin1().constData());
    lMIRRecord.SetJOB_NAM("");
    lMIRRecord.SetJOB_REV("");
    lMIRRecord.SetSBLOT_ID(mPromisLotId.toLatin1().constData());
    lMIRRecord.SetOPER_NAM(mOperator.toLatin1().constData());
    lMIRRecord.SetEXEC_TYP("");
    lMIRRecord.SetEXEC_VER("");
    if(!mSiteLocation.isEmpty())
        lMIRRecord.SetTEST_COD(mSiteLocation.toLatin1().constData());
    lMIRRecord.SetTST_TEMP("");

    // Construct custom Galaxy USER_TXT
    QString    lUserTxt;
    lUserTxt = lUserTxt % GEX_IMPORT_DATAORIGIN_LABEL %
    ':' %
    GEX_IMPORT_DATAORIGIN_ATETEST %
    ":fet_test_summary";

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
    lMIRRecord.SetOPER_FRQ("");
    lMIRRecord.SetSPEC_NAM("");
    lMIRRecord.SetSPEC_VER("");
    lMIRRecord.SetFLOW_ID("");
    lMIRRecord.SetSETUP_ID("");
    lMIRRecord.SetDSGN_REV("");
    lMIRRecord.SetENG_ID("");
    if(!mPackage.isEmpty())
        lMIRRecord.SetROM_COD(mPackage.toLatin1().constData());
    lMIRRecord.Write(lStdfFile);

    // Write die-tracking DTRs
    if(!mPackageType.isEmpty())
    {
        QString lString;
        lUserTxt = "<cmd> die-tracking die=1;wafer_product=" % mProductId %
        ";wafer_lot=" % mLotId %
        ";wafer_sublot=" % mPromisLotId;

        GQTL_STDF::Stdf_DTR_V4 lDTRRecord;
        lDTRRecord.SetTEXT_DAT(lUserTxt.toLatin1().constData());
        lDTRRecord.Write(lStdfFile);


        if(!mPromisLotId_D2.isEmpty() && !mGeometryName_D2.isEmpty())
        {
            lString = mPromisLotId_D2.section('.', 0, 0);
            lUserTxt = "<cmd> die-tracking die=2;wafer_product=" % mGeometryName_D2 %
            ";wafer_lot=" % lString %
            ";wafer_sublot=" % mPromisLotId_D2;

            lDTRRecord.Reset();
            lDTRRecord.SetTEXT_DAT(lUserTxt.toLatin1().constData());
            lDTRRecord.Write(lStdfFile);

            if(!mPromisLotId_D3.isEmpty() && !mGeometryName_D3.isEmpty())
            {
                lString = mPromisLotId_D3.section('.', 0, 0);
                lUserTxt = "<cmd> die-tracking die=3;wafer_product=" % mGeometryName_D3 %
                ";wafer_lot=" % lString %
                ";wafer_sublot=" % mPromisLotId_D3;

                lDTRRecord.Reset();
                lDTRRecord.SetTEXT_DAT(lUserTxt.toLatin1().constData());
                lDTRRecord.Write(lStdfFile);

                if(!mPromisLotId_D4.isEmpty() && !mGeometryName_D4.isEmpty())
                {
                    lString = mPromisLotId_D4.section('.', 0, 0);
                    lUserTxt = "<cmd> die-tracking die=4;wafer_product=" % mGeometryName_D4 %
                    ";wafer_lot=" % lString %
                    ";wafer_sublot=" % mPromisLotId_D4;

                    lDTRRecord.Reset();
                    lDTRRecord.SetTEXT_DAT(lUserTxt.toLatin1().constData());
                    lDTRRecord.Write(lStdfFile);
                }
            }
        }
    }

    // Write SDR
    GQTL_STDF::Stdf_SDR_V4 lSDRRecord;
    lSDRRecord.SetHEAD_NUM(1);
    lSDRRecord.SetSITE_GRP(1);
    lSDRRecord.SetSITE_CNT(1);
    lSDRRecord.SetSITE_NUM(0, 1);
    lSDRRecord.SetHAND_TYP(mProber.toLatin1().constData());
    lSDRRecord.SetHAND_ID(mHandId.toLatin1().constData());
    lSDRRecord.SetCARD_TYP("");
    lSDRRecord.SetCARD_ID(mCardId.toLatin1().constData());
    lSDRRecord.SetLOAD_TYP("");
    lSDRRecord.SetLOAD_ID(mLoadId.toLatin1().constData());
    lSDRRecord.SetDIB_TYP("");
    lSDRRecord.SetDIB_ID(mDibId.toLatin1().constData());
    lSDRRecord.SetCABL_TYP("");
    lSDRRecord.SetCABL_ID(mCableId.toLatin1().constData());
    lSDRRecord.SetCONT_TYP("");
    lSDRRecord.SetCONT_ID("");
    lSDRRecord.SetLASR_TYP("");
    lSDRRecord.SetLASR_ID("");
    lSDRRecord.SetEXTR_TYP("");
    lSDRRecord.SetEXTR_ID("");
    lSDRRecord.Write(lStdfFile);

    QMap<int,CGFetTestSummaryBinning>::Iterator itTestNumber;
    for(itTestNumber = mFetTestSummaryBinPerTestNumber.begin(); itTestNumber != mFetTestSummaryBinPerTestNumber.end(); itTestNumber++)
    {
        WriteSBR(*itTestNumber, lStdfFile);
    }

    QMap<std::string,CGFetTestSummaryBinning>::Iterator itTestName;
    for(itTestName = mFetTestSummaryBinPerTestName.begin(); itTestName != mFetTestSummaryBinPerTestName.end(); itTestName++)
    {
        WriteSBR(*itTestName, lStdfFile);
    }

    // Bin mapping file is present OR BinNumber can be used for HBin and SBin
    // SBR and HBR are identicals

    for(itTestNumber = mFetTestSummaryBinPerTestNumber.begin(); itTestNumber != mFetTestSummaryBinPerTestNumber.end(); itTestNumber++)
    {
        WriteHBR(*itTestNumber, lStdfFile);
    }

    for(itTestName = mFetTestSummaryBinPerTestName.begin(); itTestName != mFetTestSummaryBinPerTestName.end(); itTestName++)
    {
        WriteHBR(*itTestName, lStdfFile);
    }

    // Write PCR
    GQTL_STDF::Stdf_PCR_V4 lPCRRecord;

    lPCRRecord.SetHEAD_NUM(255);
    lPCRRecord.SetSITE_NUM(255);
    lPCRRecord.SetPART_CNT(mTotalParts);
    lPCRRecord.SetRTST_CNT(0);
    lPCRRecord.SetABRT_CNT(0);
    lPCRRecord.SetGOOD_CNT(mTotalGoodParts);

    lPCRRecord.Write(lStdfFile);

    // Write MRR
    GQTL_STDF::Stdf_MRR_V4 r;

    r.SetFINISH_T(mStopTime);

    r.Write(lStdfFile);

    // Close STDF file.
    lStdfFile.Close();

    // Success
    return true;
}


void FetTestSummaryToStdf::WriteHBR(CGFetTestSummaryBinning& lItem, GS::StdLib::Stdf &aStdfFile)
{
    mHBRRecord.Reset();

    if(lItem.Count <= 0) return;

    mHBRRecord.SetHEAD_NUM(255);
    mHBRRecord.SetSITE_NUM(0);
    mHBRRecord.SetHBIN_NUM(lItem.HardBinNum);
    mHBRRecord.SetHBIN_CNT(lItem.Count);

    if(lItem.Pass)
        mHBRRecord.SetHBIN_PF('P');
    else
        mHBRRecord.SetHBIN_PF('F');

    mHBRRecord.SetHBIN_NAM(lItem.Name.toLatin1().constData());

    mHBRRecord.Write(aStdfFile);
}

void FetTestSummaryToStdf::WriteSBR(CGFetTestSummaryBinning& lItem, GS::StdLib::Stdf &aStdfFile)
{
    mSBRRecord.Reset();

    if(lItem.Count <= 0) return;

    mSBRRecord.SetHEAD_NUM(255);
    mSBRRecord.SetSITE_NUM(0);
    mSBRRecord.SetSBIN_NUM(lItem.SoftBinNum);
    mSBRRecord.SetSBIN_CNT(lItem.Count);

    if(lItem.Pass)
        mSBRRecord.SetSBIN_PF('P');
    else
        mSBRRecord.SetSBIN_PF('F');

    mSBRRecord.SetSBIN_NAM(lItem.Name.toLatin1().constData());

    mSBRRecord.Write(aStdfFile);
}

}
}
