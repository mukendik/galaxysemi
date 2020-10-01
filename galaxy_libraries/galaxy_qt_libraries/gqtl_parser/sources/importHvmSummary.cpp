#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <QStringBuilder>
#include "stdfrecords_v4.h"
#include "converter_external_file.h"
#include "importHvmSummary.h"
#include "bin_map_store_factory.h"
#include "bin_map_item_base.h"
#include "promis_interpreter_factory.h"
#include "promis_item_base.h"

namespace GS
{
namespace Parser
{

static const QChar cComaSeparator(',');
static const QChar cDotSeparator('.');
static const QChar cDoubleDotSeparator(':');
static const QChar cSpaceSeparator(' ');
static const QChar cEmpty(' ');
static const QChar cLineFeed('\n');

//////////////////////////////////////////////////////////////////////
// Check if File is compatible with HvmSummary format
//////////////////////////////////////////////////////////////////////

static const QString CNTFILENAME("CNTFILENAME");
static const QString CNTFILECREATEDDATETIME("CNTFILECREATEDDATETIME");
static const QString CSVFILECREATEDDATETIME("CSVFILECREATEDDATETIME");
static const QString TSTFILENAME("TSTFILENAME");
static const QString BIN("BIN");
static const QString SORTNAME("SORTNAME");
static const QString HVM_LOT_SUMMARY(":HVM_LOT_SUMMARY");
const QString cPass("PASS");

bool ImportHvmSummary::IsCompatible(const QString &aFileName)
{
    // Open hCsmFile file
    QFile lFile( aFileName );
    if(!lFile.open( QIODevice::ReadOnly ))
    {
        // Failed Opening ASL1000 file
        return false;
    }

    // Assign file I/O stream
    QTextStream lFileIO(&lFile);

    //HVM Data file : HVM Summary Format
    //CNT File Name,A26K345.3.cnt,,,,
    //CNT File Created Date Time ,01 01 1970 08:00:00,,,,
    while(!lFileIO.atEnd())
    {
        QString lString = lFileIO.readLine().remove(cEmpty).toUpper();
        if(lString.isEmpty())
        {
            continue;
        }
        else if(lString.startsWith(CNTFILENAME))
        {
            continue;
        }
        else if(lString.startsWith(CNTFILECREATEDDATETIME))
        {
            lFile.close();
            return true;
        }
        else
        {
            break;
        }
    }

    // Incorrect header...this is not a HvmSummary file!
    lFile.close();

    return false;
}

bool ImportHvmSummary::ComputeBinning(int aBinNum, const QString &aBinName, int aBinCount)
{
    QString lBinKey = (mHaveBinmapFile) ? aBinName : QString::number(aBinNum);

    // Special case for bin 1 001 <product> /S/L
    if(aBinNum == 1)
        lBinKey = "001 /S/L";

    if (mHaveBinmapFile)
    {
        try
        {
            const Qx::BinMapping::BinMapItemBase &lItem = mBinMapStore->GetBinMapItemByBinName( lBinKey.toStdString() );

            CreateOrUpdateBinning(mSoftBinning, lBinKey, lItem.GetSoftBinNumber(), lItem.GetBinName().c_str(),
                                  aBinCount, lItem.GetBinCategory() == 'P');


            CreateOrUpdateBinning(mHardBinning, lBinKey, lItem.GetHardBinNumber(), lItem.GetBinName().c_str(),
                                  aBinCount, lItem.GetBinCategory() == 'P');
        }
        catch( const std::exception &lException )
        {
            mLastErrorMessage = lException.what();
            return false;
        }
    }
    else
    {
        CreateOrUpdateBinning(mSoftBinning, lBinKey, aBinNum, aBinName, aBinCount, aBinNum == 1);

        CreateOrUpdateBinning(mHardBinning, lBinKey, aBinNum, aBinName, aBinCount, aBinNum == 1);
    }

    return true;
}

ParserBinning *ImportHvmSummary::MakeBinning(int aBinNum, const QString& aBinName, int aBinCount, bool aPass)
{
    ParserBinning* lBinning = new ParserBinning();

    lBinning->SetBinNumber(aBinNum);
    lBinning->SetBinName(aBinName);
    lBinning->SetPassFail(aPass);
    lBinning->SetBinCount(aBinCount);

    return lBinning;
}

void ImportHvmSummary::CreateOrUpdateBinning(QMap<QString, ParserBinning *> &aBinnings, const QString& aBinKey,
                                             int aBinNum, const QString& aBinName, int aBinCount, bool aPass)
{
    if (aBinnings.contains(aBinKey) == false)
    {
        aBinnings.insert(aBinKey, MakeBinning(aBinNum, aBinName, aBinCount, aPass));
    }
    else
    {
        aBinnings[aBinKey]->IncrementBinCount(aBinCount);
    }
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the PromisHvmSummary file
//////////////////////////////////////////////////////////////////////

static const QString cFinal("final");
static const QString cProd("prod");

bool ImportHvmSummary::ReadPromisHvmDataFile()
{
    // Check if converter_external_file exists
    QString lExternalFileName;
    QString lExternalFileFormat;
    QString lExternalFileError;
    ConverterExternalFile::GetPromisFile(mDataFilePath,
                                         cFinal,
                                         cProd,
                                         lExternalFileName,
                                         lExternalFileFormat,
                                         lExternalFileError);
    if(lExternalFileName.isEmpty())
    {
        if(lExternalFileError.isEmpty())
            lExternalFileError = QStringLiteral("No 'GEX_PROMIS_HVM_DATA' file defined");
        mLastError = errInvalidFormatParameter;
        mLastErrorMessage = lExternalFileError;
        return false;
    }

    if(mSubLotID.isEmpty())
    {
        // Failed Opening HvmSummary file
        mLastError = errInvalidFormatParameter;
        mLastErrorMessage = QStringLiteral("No Promis Lot Id defined");

        // Convertion failed.
        return false;
    }

    try
    {
        mPromisInterpreter.reset( Qx::BinMapping::PromisInterpreterFactory< Qx::BinMapping::promis_hvm_ft >
                                  ::MakePromisInterpreter( mSubLotID.toStdString(),
                                                           lExternalFileName.toStdString(),
                                                           ConverterExternalFile::GetExternalFileName( mDataFilePath )
                                                           .toStdString() ) );

        // We've found the right line
        mDateCod = mPromisInterpreter->GetPromisItem().GetDateCode().c_str();
        mPkgTyp = mPromisInterpreter->GetPromisItem().GetPackageType().c_str();
        mNodeNam = mPromisInterpreter->GetPromisItem().GetEquipmentID().c_str();
        mProcId = mPromisInterpreter->GetPromisItem().GetPartNumber().c_str();
        mFacilId = mPromisInterpreter->GetPromisItem().GetSiteLocation().c_str();
        mPartTyp = mPromisInterpreter->GetPromisItem().GetGeometryName().c_str();
        mExtraId = mPromisInterpreter->GetPromisItem().GetDiePart().c_str();
        mTestCod = mPromisInterpreter->GetPromisItem().GetDivision().c_str();
    }
    catch( const std::exception &lException )
    {
        mLastErrorMessage = lException.what();
        return false;
    }

    return true;
}


//////////////////////////////////////////////////////////////////////
// Read and Parse the HvmBinmap file
//////////////////////////////////////////////////////////////////////
bool ImportHvmSummary::ReadHvmBinmapFile()
{
    QString lExternalFileName;
    QString lExternalFileFormat;
    QString lExternalFileError;
    // Check if Test->Binning mapping file to overload softbin
    ConverterExternalFile::GetBinmapFile(mDataFilePath,"final","prod",lExternalFileName,lExternalFileFormat,lExternalFileError);
    mExternalFilePath = mDataFilePath;
    mBinMapFilePath = lExternalFileName;
    if(lExternalFileName.isEmpty())
    {
        if(lExternalFileError.isEmpty())
            lExternalFileError = QStringLiteral("No 'GEX_HVM_BINMAP_FILE' file defined");
        mLastError = errInvalidFormatParameter;;
        mLastErrorMessage = lExternalFileError;
        return false;
    }

    try
    {
        const QString &lConverterExternalFilePath = ConverterExternalFile::GetExternalFileName( mDataFilePath );
        mBinMapStore.reset( Qx::BinMapping::BinMapStoreFactory< Qx::BinMapping::hvm_ft >
                            ::MakeBinMapStore( lExternalFileName.toStdString(),
                                               lConverterExternalFilePath.toStdString() ) );
    }
    catch( const std::exception &lException )
    {
        mLastErrorMessage = lException.what() + appendBinMappingExceptionInfo();
        return false;
    }

    mHaveBinmapFile = true;

    return true;
}


//////////////////////////////////////////////////////////////////////
// Read and Parse the HvmSummary file
//////////////////////////////////////////////////////////////////////

bool ImportHvmSummary::ReadHvmSummaryFile(const QString &aFileName)
{

    // Open CSV file
    QFile lQFile( aFileName );
    if(!lQFile.open( QIODevice::ReadOnly ))
    {
        // Failed Opening HvmSummary file
        mLastError = errOpenFail;
        mLastErrorMessage = lQFile.errorString();

        // Convertion failed.
        return false;
    }

    // Assign file I/O stream
    QTextStream lFile(&lQFile);
    QDate lDate;
    QTime lTime;

    //CNT File Name,A26K345.3.cnt,,,,
    //CNT File Created Date Time ,01 01 1970 08:00:00,,,,
    //TST File Name,IRF830~1.TST,,,,
    //CSV File Name,A26K345.3.CSV,,,,
    //CSV File Created Date Time ,19 10 2011 13:53:48,,,,
    //Polarity,NPN,,,,
    //Comment,,,,,
    //Bin Sub and Lot Counters,,,,,
    //Bin,SortName, Sub, Percent,,
    //1,001 IRF830A/S/L,496,2585,,


    // Read HvmSummary information
    QString lStrString;
    QString lStrSection;
    while(!lFile.atEnd())
    {
        lStrString = ReadLine(lFile);
        lStrSection = lStrString.section(cComaSeparator,0,0).remove(cSpaceSeparator).toUpper();
        lStrString = lStrString.section(cComaSeparator,1,1).simplified();

        if(lStrSection == CNTFILENAME)
        {
            mSubLotID = lStrString.section(cDotSeparator,0,lStrString.count(cDotSeparator)-1).simplified();
            mLotID = lStrString.section(cDotSeparator,0,0);
        }
        else if(lStrSection == CNTFILECREATEDDATETIME)
        {
            QString lStrDate = lStrString.section(cSpaceSeparator,0,2);
            QString lStrTime = lStrString.section(cSpaceSeparator,2);
            int iDay = lStrDate.section(cSpaceSeparator,0,0).toInt();
            int iMonth = lStrDate.section(cSpaceSeparator,1,1).toInt();
            int iYear = lStrDate.section(cSpaceSeparator,2,2).toInt();
            int iHour = lStrTime.section(cDoubleDotSeparator,0,0).toInt();
            int iMin = lStrTime.section(cDoubleDotSeparator,1,1).toInt();
            int iSec = lStrTime.section(cDoubleDotSeparator,2,2).toInt();

            lDate = QDate(iYear,iMonth,iDay);
            lTime = QTime(iHour,iMin,iSec);
        }
        else if(lStrSection == CSVFILECREATEDDATETIME)
        {
            if(lDate.isValid() && (lDate.year()>1970))
                continue;

            QString lStrDate, lStrTime;
            lStrDate = lStrString.section(cSpaceSeparator,0,2);
            lStrTime = lStrString.section(cSpaceSeparator,2);
            int iDay = lStrDate.section(cSpaceSeparator,0,0).toInt();
            int iMonth = lStrDate.section(cSpaceSeparator,1,1).toInt();
            int iYear = lStrDate.section(cSpaceSeparator,2,2).toInt();
            int iHour = lStrTime.section(cDoubleDotSeparator,0,0).toInt();
            int iMin = lStrTime.section(cDoubleDotSeparator,1,1).toInt();
            int iSec = lStrTime.section(cDoubleDotSeparator,2,2).toInt();

            lDate = QDate(iYear,iMonth,iDay);
            lTime = QTime(iHour,iMin,iSec);
        }
        else if(lStrSection == TSTFILENAME)
        {
            mNodeNam = lStrString.section(cDotSeparator,0,lStrString.count(cDotSeparator)-1).simplified();
        }
        else if(lStrSection == BIN)
        {
            break;
        }
    }

    mStartTime = mStopTime = QDateTime(lDate,lTime).toTime_t();

    if((lStrSection != BIN) ||
            (lStrString.toUpper().remove(cSpaceSeparator) != SORTNAME))
    {
        mLastError = errInvalidFormatParameter;
        mLastErrorMessage = QStringLiteral("Line 'Bin,SortName,Sub,Percent' not found");

        lQFile.close();

        return false;
    }

    //Bin,SortName, Sub, Percent,,
    //1,001 IRF830A/S/L,496,2585,,
    QString lBinCode;

    while(!lFile.atEnd())
    {
        QString lStrString = ReadLine(lFile).simplified().toUpper();

        if(lStrString.isEmpty())
            continue;

        bool lIsValidNum = false;
        int lBinNum  = lStrString.section(cComaSeparator,0,0).toInt(&lIsValidNum);

        if(!lIsValidNum)
            break;

        if(lStrString.count(cComaSeparator) < 3)
        {
            mLastError = errInvalidFormatParameter;
            mLastErrorMessage = QStringLiteral("Bin Sub line is truncated");

            lQFile.close();
            return false;
        }

        QString lBinName = lBinCode = lStrString.section(cComaSeparator,1,1);
        int lBinCount = lStrString.section(cComaSeparator,2,2).toInt();

        if(lBinCount <= 0) continue;

        if (ComputeBinning(lBinNum, lBinName, lBinCount) == false)
        {
            lQFile.close();
            return false;
        }
    }

    // Success parsing HvmSummary file
    lQFile.close();
    return true;
}


//////////////////////////////////////////////////////////////////////
// Create STDF file from HvmSummary data parsed
//////////////////////////////////////////////////////////////////////
bool ImportHvmSummary::WriteStdfFile(const QString &aFileNameSTDF)
{
    // now generate the STDF file...
    GS::StdLib::Stdf lStdfFile;
    if(lStdfFile.Open((char*)aFileNameSTDF.toLatin1().constData(), STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        // Failed importing CSV file into STDF database
        mLastError = errWriteSTDF;

        // Convertion failed.
        return false;
    }

    // Write FAR
    GQTL_STDF::Stdf_FAR_V4 lStdf_FAR_V4;
    lStdf_FAR_V4.SetCPU_TYPE(1);// SUN CPU type
    lStdf_FAR_V4.SetSTDF_VER(4);// STDF V4
    lStdf_FAR_V4.Write(lStdfFile);


    if(mStartTime <= 0)
        mStartTime = QDateTime::currentDateTime().toTime_t();

    // Write MIR
    GQTL_STDF::Stdf_MIR_V4 lStdf_MIR_V4;
    lStdf_MIR_V4.SetSETUP_T(mStartTime);
    lStdf_MIR_V4.SetSTART_T(mStartTime);
    lStdf_MIR_V4.SetSTAT_NUM(1);
    lStdf_MIR_V4.SetMODE_COD((BYTE) 'P');
    lStdf_MIR_V4.SetRTST_COD((BYTE) ' ');
    lStdf_MIR_V4.SetPROT_COD((BYTE) ' ');
    lStdf_MIR_V4.SetBURN_TIM(65535);
    lStdf_MIR_V4.SetCMOD_COD((BYTE) ' ');
    lStdf_MIR_V4.SetLOT_ID(mLotID.toLatin1().constData());
    lStdf_MIR_V4.SetPART_TYP(mPartTyp.toLatin1().constData());
    lStdf_MIR_V4.SetNODE_NAM(mNodeNam.toLatin1().constData());
    lStdf_MIR_V4.SetTSTR_TYP("");
    lStdf_MIR_V4.SetJOB_NAM("");
    lStdf_MIR_V4.SetJOB_REV("");
    lStdf_MIR_V4.SetSBLOT_ID(mSubLotID.toLatin1().constData());
    lStdf_MIR_V4.SetOPER_NAM("");
    lStdf_MIR_V4.SetEXEC_TYP("");
    lStdf_MIR_V4.SetEXEC_VER("");
    lStdf_MIR_V4.SetTEST_COD(mTestCod.toLatin1().constData());
    lStdf_MIR_V4.SetTST_TEMP("");

    // Construct custom Galaxy USER_TXT
    QString lUserTxt = GEX_IMPORT_DATAORIGIN_LABEL %
            cDoubleDotSeparator %
            GEX_IMPORT_DATAORIGIN_ATETEST %
            HVM_LOT_SUMMARY;
    lStdf_MIR_V4.SetUSER_TXT(lUserTxt.toLatin1().constData());

    lStdf_MIR_V4.SetAUX_FILE("");
    lStdf_MIR_V4.SetPKG_TYP(mPkgTyp.toLatin1().constData());
    lStdf_MIR_V4.SetFAMLY_ID("");
    lStdf_MIR_V4.SetDATE_COD(mDateCod.toLatin1().constData());
    lStdf_MIR_V4.SetFACIL_ID(mFacilId.toLatin1().constData());
    lStdf_MIR_V4.SetFLOOR_ID("");
    lStdf_MIR_V4.SetPROC_ID(mProcId.toLatin1().constData());
    lStdf_MIR_V4.Write(lStdfFile);


    if(!mExtraId.isEmpty())
    {
        // Write SDR
        GQTL_STDF::Stdf_SDR_V4 lStdf_SDR_V4;
        lStdf_SDR_V4.SetHEAD_NUM((BYTE)255);
        lStdf_SDR_V4.SetSITE_GRP((BYTE)1);
        lStdf_SDR_V4.SetSITE_CNT((BYTE)1);
        lStdf_SDR_V4.SetSITE_NUM(0, 1);
        lStdf_SDR_V4.SetEXTR_ID(mExtraId.toLatin1().constData());
        lStdf_SDR_V4.Write(lStdfFile);
    }

    QMap<QString, ParserBinning*>::Iterator it;
    int lTotalParts = 0;
    int lPassParts = 0;

    GQTL_STDF::Stdf_SBR_V4 lStdf_SBR_V4;
    for(it = mSoftBinning.begin(); it != mSoftBinning.end(); it++)
    {
        if((*it)->GetBinCount() > 0)
        {
            // Write SBR/site
            lStdf_SBR_V4.SetHEAD_NUM(255);
            lStdf_SBR_V4.SetSITE_NUM(0);
            lStdf_SBR_V4.SetSBIN_NUM((*it)->GetBinNumber());
            lStdf_SBR_V4.SetSBIN_CNT((*it)->GetBinCount());
            if((*it)->GetPassFail())
            {
                lStdf_SBR_V4.SetSBIN_PF('P');
                lPassParts += (*it)->GetBinCount();
                if (mHaveBinmapFile)
                {
                    lStdf_SBR_V4.SetSBIN_NAM(cPass);
                }
                else
                {
                    lStdf_SBR_V4.SetSBIN_NAM((*it)->GetBinName());
                }
            }
            else
            {
                lStdf_SBR_V4.SetSBIN_PF('F');
                lStdf_SBR_V4.SetSBIN_NAM((*it)->GetBinName());
            }
            lStdf_SBR_V4.Write(lStdfFile);

            lTotalParts += (*it)->GetBinCount();
        }
    }

    // Bin mapping file is present OR BinNumber can be used for HBin and SBin
    // SBR and HBR are identicals
    GQTL_STDF::Stdf_HBR_V4 lStdf_HBR_V4;
    for(it = mHardBinning.begin(); it != mHardBinning.end(); it++)
    {
        if((*it)->GetBinCount() > 0)
        {
            // Write SBR/site
            lStdf_HBR_V4.SetHEAD_NUM(255);
            lStdf_HBR_V4.SetSITE_NUM(0);
            lStdf_HBR_V4.SetHBIN_NUM((*it)->GetBinNumber());
            lStdf_HBR_V4.SetHBIN_CNT((*it)->GetBinCount());
            if((*it)->GetPassFail())
            {
                lStdf_HBR_V4.SetHBIN_PF('P');
                if (mHaveBinmapFile)
                {
                    lStdf_HBR_V4.SetHBIN_NAM(cPass);
                }
                else
                {
                    lStdf_HBR_V4.SetHBIN_NAM((*it)->GetBinName());
                }
            }
            else
            {
                lStdf_HBR_V4.SetHBIN_PF('F');
                lStdf_HBR_V4.SetHBIN_NAM((*it)->GetBinName());
            }
            lStdf_HBR_V4.Write(lStdfFile);
        }
    }

    // Write PCR
    GQTL_STDF::Stdf_PCR_V4 lStdf_PCR_V4;
    lStdf_PCR_V4.SetHEAD_NUM(255);
    lStdf_PCR_V4.SetSITE_NUM(255);
    lStdf_PCR_V4.SetPART_CNT(lTotalParts);
    lStdf_PCR_V4.SetRTST_CNT(0);
    lStdf_PCR_V4.SetABRT_CNT(0);
    lStdf_PCR_V4.SetGOOD_CNT(lPassParts);
    lStdf_PCR_V4.Write(lStdfFile);


    // Write MRR
    GQTL_STDF::Stdf_MRR_V4 lStdf_MRR_V4;
    lStdf_MRR_V4.SetFINISH_T(mStopTime);
    lStdf_MRR_V4.Write(lStdfFile);


    // Close STDF file.
    lStdfFile.Close();

    // Success
    return true;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' HvmSummary file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool ImportHvmSummary::ConvertoStdf(const QString &aOriginalFileName, QString &aStdfFileName)
{
    // No erro (default)
    mLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFile lQFile( aStdfFileName );
    if((lQFile.exists() == true)
            && (QFileInfo(aOriginalFileName).lastModified() < QFileInfo(aStdfFileName).lastModified()))
    {
        return true;
    }

    QFileInfo clFile(aOriginalFileName);
    mDataFilePath = clFile.absolutePath();

    bool lConvertionWithExternalFile = ConverterExternalFile::Exists(mDataFilePath);

    bool lStatus = true;
    if(lConvertionWithExternalFile)
    {
        if(!ReadHvmBinmapFile())
        {
            mLastErrorMessage += QStringLiteral(". \nHVM FT Bin mapping cannot be retrieved");
            lStatus = false;
        }
    }

    lStatus = ReadHvmSummaryFile(aOriginalFileName);

    if(lConvertionWithExternalFile)
    {
        if(lStatus && !ReadPromisHvmDataFile())
        {
            mLastErrorMessage += QStringLiteral(". \nHVM Promis data cannot be retrieved");
            lStatus = false;
        }
    }

    // Generate STDF file dynamically.
    if(lStatus)
    {
        lStatus = WriteStdfFile(aStdfFileName);
    }

    if(!lStatus)
    {
        QFile::remove(aStdfFileName);
    }

    return lStatus;
}




} //namespace GS
} //namespace Parser
