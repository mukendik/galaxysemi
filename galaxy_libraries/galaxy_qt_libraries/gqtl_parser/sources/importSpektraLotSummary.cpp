#include "importSpektraLotSummary.h"
#include "bin_map_item_base.h"
#include "bin_map_store_factory.h"
#include "converter_external_file.h"
#include "bin_mapping_exceptions.h"
#include "promis_interpreter_factory.h"
#include "promis_item_base.h"

#include <QString>
#include <QFileInfo>

const QString cDtaFileName = "DTA File Name";
const QString cTstFileNameForDta = "TST FILE NAME FOR DTA";
const QString cCsvFileCreateDateTime = "CSV FILE CREATED DATE TIME";
const QString cOperatorName = "OPERATOR NAME";
const QString cStation = "STATION";
const QString cDeviceName = "DEVICE NAME";
const QString cLotName = "LOT NAME";
const QString cTester = "TESTER#";
const QString cHandler = "HANDLER#";
const QString cDieQtyPerWafer = "DIE QTY PER WAFER";
const QString cPass = "PASS";

namespace GS
{
namespace Parser
{

SpektraLotSummary::SpektraLotSummary() : ParserBase(typeSpektraLotSummary, "SpektraLotSummary")
{

}

SpektraLotSummary::~SpektraLotSummary()
{

}

bool SpektraLotSummary::ConvertoStdf(const QString &aInputFile, QString &aStdfFileName)
{
    // Open SpektraLotSummary file
    QFile lInputFile(aInputFile);
    if(!lInputFile.open(QIODevice::ReadOnly))
    {
        // Failed Opening CSV file
        mLastError = errOpenFail;

        // Convertion failed.
        return false;
    }

    QFileInfo lInputFileInfo(aInputFile);
    mDataFilePath = lInputFileInfo.absolutePath();

    // Assign file I/O stream
    QTextStream lInputTextStream(&lInputFile);

    if (ReadSpektraLotSummaryFile(lInputTextStream) == false)
    {
        return false;
    }

    if (WriteStdfFile(lInputTextStream, aStdfFileName) == false)
    {
        while(!mOutputFiles.isEmpty())
            QFile::remove(mOutputFiles.takeFirst());

        return false;
    }

    lInputFile.close();

    return true;
}

std::string SpektraLotSummary::GetErrorMessage(const int ErrorCode) const
{
    QString lError;

    switch(ErrorCode)
    {
        case errMissingData:
            lError += "Missing mandatory info";
            break;

        case errMultiLot:
            lError += "Invalid file format: This file contains more than one lot";
            break;

        default:
            lError += QString::fromStdString(ParserBase::GetErrorMessage(ErrorCode));
            break;
    }

    return lError.toStdString();
}

bool SpektraLotSummary::IsCompatible(const QString & aFileName)
{
    QString lLine;

    // Open SpektraLotSummary file
    QFile lInputFile(aFileName);

    if(!lInputFile.open(QIODevice::ReadOnly))
    {
        // Failed Opening SpektraLotSummary file
        return false;
    }

    // Assign file I/O stream
    QTextStream lInputStream(&lInputFile);

    // Check if first line is the correct SpektraLotSummary header...
    //DTA File Name,A04K041.1.Dta
    do
    {
        lLine = lInputStream.readLine();
    } while(!lLine.isNull() && lLine.isEmpty() && !lInputStream.atEnd());

    if(!lLine.simplified().startsWith("DTA File Name", Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a SpektraLotSummary file!
        return false;
    }

    // This is a Spektra file summary
    // Check if it is a Lot Summaty
    //DTA File Name,A04K041.1.Dta
    // ...
    //Die Qty Per Wafer,233
    //,,OSC            ,PASS           ,ISGS @ 5V      ,IDSS @ V       ,BVDSS          ,BV NEG RES     ,RDON           ,VP             ,VF             ,ISGS @ +/-V    ,CONT           ,NO SORT
    //Lot No,Wafer No,Bin24,Bin1,Bin2,Bin3,Bin4,Bin5,Bin6,Bin7,Bin8,Bin9,Bin10,Bin11,Total,Yield
    bool lFoundGrossDieLine = false;
    bool lFoundLotNoLine = false;
    int  lLineCount = 1;
    while(!lInputStream.atEnd() && lLineCount < 20 && !(lFoundGrossDieLine && lFoundLotNoLine))
    {
        lLine = lInputStream.readLine();

        if(lLine.startsWith("Die Qty Per Wafer,",Qt::CaseInsensitive))
            lFoundGrossDieLine = true;

        if(lLine.startsWith("Lot No,",Qt::CaseInsensitive))
            lFoundLotNoLine = true;

        lLineCount++;
    }

    lInputFile.close();

    if(lFoundGrossDieLine && lFoundLotNoLine)
        return true;

    return false;
}

bool SpektraLotSummary::ReadSpektraLotSummaryFile(QTextStream &aInputTextStream)
{
    // Check if first line is the correct SpektraLotSummary header...
    //DTA File Name,A04K041.1.Dta
    QString lLine = ReadLine(aInputTextStream);

    if(!lLine.simplified().startsWith(cDtaFileName, Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a SpektraLotSummary file!
        mLastError = errInvalidFormatParameter;

        return false;
    }

    // Read SpektraLotSummary information
    //DTA File Name,A04K041.1.Dta
    //DTA File Created Date Time,
    //TST File Name for DTA,DRSBA8583_02
    //CSV File Name,A04K041.1_Summary
    //CSV File Created Date Time,21 02 2010 23:34:00
    //Operator Name,12985
    //Station,A
    //Device Name,605810TS
    //Lot Name,A04K041.1
    //Comment,_
    //Tester#,SPEKTRA 02
    //Handler#,EG 04
    //Die Qty Per Wafer,233
    QDateTime   lDateTime;
    QString     lKey;
    QString     lValue;

    while(!aInputTextStream.atEnd())
    {
        lLine   = ReadLine(aInputTextStream);
        lKey    = lLine.section(",", 0, 0).toUpper();
        lValue  = lLine.section(",", 1);

        if(lKey.isEmpty())
            continue;

        if(lKey == cTstFileNameForDta)
        {
            //TST File Name for DTA,DRSBA8583_02
            mJobName = lValue;
        }
        else if(lKey == cCsvFileCreateDateTime)
        {
            //CSV File Created Date Time,21 02 2010 23:34:00
            QString lYear, lMonth, lDay;
            QString lHour, lMin, lSec;

            lValue  = lValue.simplified();
            lYear   = lValue.section(" ", 2, 2);
            lMonth  = lValue.section(" ", 1, 1);
            lDay    = lValue.section(" ", 0, 0);

            lValue  = lValue.section(" ", 3).simplified();
            lHour   = lValue.section(":", 0, 0);
            lMin    = lValue.section(":", 1, 1);
            lSec    = lValue.section(":", 2, 2);

            lDateTime = QDateTime(QDate(lYear.toInt(),lMonth.toInt(),lDay.toInt()),
                                  QTime(lHour.toInt(),lMin.toInt(),lSec.toInt()),
                                  Qt::UTC);
        }
        else if(lKey == cOperatorName)
        {
            //Operator Name,12985
            mOperatorId = lValue;
        }
        else if(lKey == cStation)
        {
            //Station,A
            mTesterName = lValue;
        }
        else if(lKey == cDeviceName)
        {
            //Device Name,605810TS
            mProcId = lValue;
        }
        else if(lKey == cLotName)
        {
            //Lot Name,A04K041.1
            mSubLotId = lValue;
            mLotId = mSubLotId.section(".", 0, 0);
        }
        else if(lKey == cTester)
        {
            //Tester#,SPEKTRA 02
            mTesterType = lValue;
        }
        else if(lKey == cHandler)
        {
            //Handler#,EG 04
            mHandlerId = mExtraId = lValue;
        }
        else if(lKey == cDieQtyPerWafer)
        {
            //Die Qty Per Wafer,233
            mGrossDie = lValue;
            break;
        }
    }

    if(lKey != cDieQtyPerWafer)
    {
        // Incorrect header...this is not a SpektraLotSummary file!
        mLastError = errInvalidFormatParameter;

        return false;
    }

    mStartTime = lDateTime.toTime_t();

    if(!ReadSpektraBinmapFile())
    {
        return false;
    }

    //Save Binning information
    QString lBinNames;
    QString lBinNumbers;
    int     lBinNumber;
    //Die Qty Per Wafer,233
    //,,OSC            ,PASS           ,ISGS @ 5V      ,IDSS @ V       ,BVDSS          ,BV NEG RES     ,RDON           ,VP             ,VF             ,ISGS @ +/-V    ,CONT           ,NO SORT
    //Lot No,Wafer No,Bin24,Bin1,Bin2,Bin3,Bin4,Bin5,Bin6,Bin7,Bin8,Bin9,Bin10,Bin11,Total,Yield

    lLine = ReadLine(aInputTextStream).simplified();

    if(!lLine.startsWith(",,"))
    {
        // Incorrect header...this is not a SpektraLotSummary file!
        mLastError = errInvalidFormatParameter;

        return false;
    }

    lBinNames = lLine;

    lLine = ReadLine(aInputTextStream).simplified();
    if(!lLine.startsWith("Lot No,Wafer No"))
    {
        // Incorrect header...this is not a SpektraLotSummary file!
        mLastError = errInvalidFormatParameter;

        return false;
    }

    lBinNumbers = lLine.toUpper();

    for(int lIndex = 2; lIndex < lBinNumbers.count(","); lIndex++)
    {
        lBinNumber = lBinNumbers.section(",", lIndex, lIndex).remove("BIN").simplified().toInt();

        mMapIndexToBinNumber[lIndex] = lBinNumber;

        if(!mMapSpektraSummaryBinning.contains(lBinNumber))
        {
            if(IsInTheBinMapStore(lBinNumber))
            {
                const Qx::BinMapping::BinMapItemBase& lItem = mBinMapStore->GetBinMapItemByTestNumber(lBinNumber);
                mMapSpektraSummaryBinning[lBinNumber].mName = lItem.GetBinName().c_str();
                mMapSpektraSummaryBinning[lBinNumber].mNumber = lBinNumber;
                mMapSpektraSummaryBinning[lBinNumber].mPass = lItem.IsPass();
                mMapSpektraSummaryBinning[lBinNumber].mCount = 0;

            }
            else
            {
                mMapSpektraSummaryBinning[lBinNumber].mName = lBinNames.section(",", lIndex, lIndex).simplified();
                mMapSpektraSummaryBinning[lBinNumber].mNumber = lBinNumber;
                mMapSpektraSummaryBinning[lBinNumber].mPass = (mMapSpektraSummaryBinning[lBinNumber].mName.toUpper() == cPass);
                mMapSpektraSummaryBinning[lBinNumber].mCount = 0;
            }

            // force the pass name to "PASS"
            if (mMapSpektraSummaryBinning[lBinNumber].mPass == true)
            {
                mMapSpektraSummaryBinning[lBinNumber].mName = cPass;
            }
        }
    }

    return true;
}

bool SpektraLotSummary::IsInTheBinMapStore(int aNumber)
{
    if(mBinMapStore.isNull()) return false;

    try
    {
        mBinMapStore->GetBinMapItemByTestNumber(aNumber);
        return true;
    }
    catch(const Qx::BinMapping::BinMappingExceptionForUserBase &)
    {
        return false;
    }
}

bool SpektraLotSummary::ReadPromisSpektraDataFile()
{
    mDiesUnit   = -1;
    mFacilId    = "";
    mPartType   = "";
    mDiesXSize  = 0.0;
    mDiesYSize  = 0.0;
    mFlatNotch  = 0;
    mTestCod    = "";

    QString lExternalFileName;
    QString lExternalFileFormat;
    QString lExternalFileError;

    // If converter external file is not there, continue the process without those informations
    if (ConverterExternalFile::Exists(mDataFilePath) == false)
        return true;

    ConverterExternalFile::GetPromisFile(mDataFilePath, "wafer", "prod",
                                         lExternalFileName, lExternalFileFormat, lExternalFileError);

    mPromisFilePath = lExternalFileName;
    if(lExternalFileName.isEmpty())
    {
        if(lExternalFileError.isEmpty())
            lExternalFileError = "No 'GEX_PROMIS_SPEKTRA_DATA' file defined";

        mLastError = errReadPromisFile;
        mLastErrorMessage = lExternalFileError;

        return false;
    }

    if(mSubLotId.isEmpty())
    {
        // Failed Opening Promis file
        mLastError = errReadPromisFile;
        mLastErrorMessage = "No Promis Lot Id defined";

        // Convertion failed.
        return false;
    }

    if(mWaferId.isEmpty())
    {
        // Failed Opening Promis file
        mLastError = errReadPromisFile;
        mLastErrorMessage = "No Wafer Id defined";

        // Convertion failed.
        return false;
    }

    try
    {
        QString     lPromisKey = mLotId + "." + mWaferId;     //<Lot_ID.Wafer_ID>

        mPromisInterpreter.reset( Qx::BinMapping::PromisInterpreterFactory< Qx::BinMapping::promis_hvm_wt >
                                  ::MakePromisInterpreter( lPromisKey.toStdString(),
                                                           lExternalFileName.toStdString(),
                                                           ConverterExternalFile::GetExternalFileName( mDataFilePath )
                                                           .toStdString() ) );

        mDiesUnit   = 3;
        mFacilId    = mPromisInterpreter->GetPromisItem().GetFabLocation().c_str();
        mProcId     = mPromisInterpreter->GetPromisItem().GetDiePart().c_str();
        mPartType   = mPromisInterpreter->GetPromisItem().GetGeometryName().c_str();
        mGrossDie   = mPromisInterpreter->GetPromisItem().GetGrossDiePerWafer().c_str();
        mDiesXSize  = QString::fromStdString(mPromisInterpreter->GetPromisItem().GetDieWidth()).toFloat();
        mDiesYSize  = QString::fromStdString(mPromisInterpreter->GetPromisItem().GetDieHeight()).toFloat();
        mFlatNotch  = QString::fromStdString(mPromisInterpreter->GetPromisItem().GetFlatOrientation()).toInt();
        mTestCod    = mPromisInterpreter->GetPromisItem().GetSiteLocation().c_str();
    }
    catch( const std::exception &lException )
    {
        mLastErrorMessage = lException.what();
        return false;
    }

    return true;
}

bool SpektraLotSummary::ReadSpektraBinmapFile()
{
    QString lExternalFileName;
    QString lExternalFileFormat;
    QString lExternalFileError;

    // If converter external file is not there, continue the process without those informations
    if (ConverterExternalFile::Exists(mDataFilePath) == false)
        return true;

    mExternalFilePath = ConverterExternalFile::GetExternalFileName(mDataFilePath);
    // Check if Test->Binning mapping file to overload softbin
    ConverterExternalFile::GetBinmapFile(mDataFilePath, "wafer", "prod",
                                         lExternalFileName, lExternalFileFormat, lExternalFileError);

    mBinMapFilePath = lExternalFileName;
    if(lExternalFileName.isEmpty())
    {
        if(lExternalFileError.isEmpty())
            lExternalFileError = "No 'GEX_SPEKTRA_BINMAP_FILE' file defined";

        mLastError = errReadBinMapFile;
        mLastErrorMessage = lExternalFileError;

        return false;
    }

    try
    {
        const QString &lConverterExternalFilePath = ConverterExternalFile::GetExternalFileName( mDataFilePath );
        mBinMapStore.reset( Qx::BinMapping::BinMapStoreFactory< Qx::BinMapping::hvm_ws_spektra >
                            ::MakeBinMapStore( lExternalFileName.toStdString(),
                                               lConverterExternalFilePath.toStdString() ) );
    }
    catch( const std::exception &lException )
    {
        mLastErrorMessage = lException.what() + appendBinMappingExceptionInfo();
        return false;
    }

    return true;
}

bool SpektraLotSummary::WriteStdfFile(QTextStream& aInputTextStream, const QString &aStdfFileName)
{
    // now generate the STDF file...
    GS::StdLib::Stdf lStdfFile;

    // Write Bin results for each line read.
    QString       lLine;
    QString       lLotId;
    int           lBinNumber;
    int           lTotalGoodBin;
    int           lTotalFailBin;

    // Write all binning read on this file : HBR, SBR
    //,,OSC            ,PASS           ,ISGS @ 5V      ,IDSS @ V       ,BVDSS          ,BV NEG RES     ,RDON           ,VP             ,VF             ,ISGS @ +/-V    ,CONT           ,NO SORT
    //Lot No,Wafer No,Bin24,Bin1,Bin2,Bin3,Bin4,Bin5,Bin6,Bin7,Bin8,Bin9,Bin10,Bin11,Total,Yield
    //A04K041.1,1,0,225,5,0,3,0,0,0,0,0,0,0,233,96.57%
    //A04K041.1,2,0,217,10,1,5,0,0,0,0,0,0,0,233,93.13%

    // For each new line, have to generate a new STDF file
    // Read SpektraLotSummary information
    while(!aInputTextStream.atEnd())
    {
        // read line and remove space
        //Lot No,Wafer No,Bin24,...
        //A04K041.1,1,0,...
        lLine = ReadLine(aInputTextStream);

        if(lLine.isEmpty())
            continue;

        if(lLotId.isEmpty())
            lLotId = lLine.section(",", 0, 0);

        if(lLotId.simplified().toUpper() != lLine.section(",", 0, 0).simplified().toUpper())
        {
            // Failed importing SpektraLotSummary file into STDF database
            mLastError = errMultiLot;

            // Convertion failed.
            return false;
        }

        mWaferId = lLine.section(",", 1, 1).toUpper();

        if(mWaferId == "TOTAL")
            break;

        if(!ReadPromisSpektraDataFile())
        {
            return false;
        }

        // Generate a specific file name
        // CSV: lot.splitlot_<restofilename>.csv
        // STDF: lot.splitlot_W#_<restoffilename>.csv.gextb.std
        QFileInfo   lStdfFileInfo(aStdfFileName);
        QString     lStdfFileName = lStdfFileInfo.path() + "/" + mSubLotId + "_W" + mWaferId;
        if(lStdfFileInfo.fileName().contains(mSubLotId))
            lStdfFileName += lStdfFileInfo.fileName().section(mSubLotId, 1);
        else
            lStdfFileName += "." + lStdfFileInfo.suffix();

        if(lStdfFile.Open(lStdfFileName.toLatin1().data(), STDF_WRITE) != GS::StdLib::Stdf::NoError)
        {
            // Failed importing SpektraLotSummary file into STDF database
            mLastError = errWriteSTDF;

            // Convertion failed.
            return false;
        }

        mOutputFiles += lStdfFileName;

        // Write FAR
        GQTL_STDF::Stdf_FAR_V4 lFARRecord;
        lFARRecord.SetCPU_TYPE(1);
        lFARRecord.SetSTDF_VER(4);
        lFARRecord.Write(lStdfFile);

        if(mStartTime <= 0)
            mStartTime = QDateTime::currentDateTime().toTime_t();

        // Write MIR
        QString lUserTxt;
        lUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
        lUserTxt += ":";
        lUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
        lUserTxt += ":SPEKTRA_LOT_SUMMARY";

        GQTL_STDF::Stdf_MIR_V4 lMIRRecord;
        lMIRRecord.SetSTART_T(mStartTime);
        lMIRRecord.SetSETUP_T(mStartTime);
        lMIRRecord.SetSTAT_NUM(1);
        lMIRRecord.SetMODE_COD('P');
        lMIRRecord.SetBURN_TIM(65535);
        lMIRRecord.SetLOT_ID(mLotId);
        lMIRRecord.SetPART_TYP(mPartType);
        lMIRRecord.SetNODE_NAM(mTesterName);
        lMIRRecord.SetTSTR_TYP(mTesterType);
        lMIRRecord.SetJOB_NAM(mJobName);
        lMIRRecord.SetSBLOT_ID(mSubLotId);
        lMIRRecord.SetOPER_NAM(mOperatorId);
        lMIRRecord.SetTEST_COD(mTestCod);
        lMIRRecord.SetUSER_TXT(lUserTxt);
        lMIRRecord.SetFACIL_ID(mFacilId);
        lMIRRecord.SetPROC_ID(mProcId);
        lMIRRecord.Write(lStdfFile);

        // Write SDR
        GQTL_STDF::Stdf_SDR_V4 lSDRRecord;
        lSDRRecord.SetHEAD_NUM(255);
        lSDRRecord.SetSITE_GRP(1);
        lSDRRecord.SetSITE_CNT(1);
        lSDRRecord.SetSITE_NUM(0, 1);
        lSDRRecord.SetHAND_ID(mHandlerId);
        lSDRRecord.SetEXTR_ID(mExtraId);
        lSDRRecord.Write(lStdfFile);

        // Write WIR
        GQTL_STDF::Stdf_WIR_V4 lWIRRecord;
        lWIRRecord.SetHEAD_NUM(1);
        lWIRRecord.SetSITE_GRP(255);
        lWIRRecord.SetSTART_T(mStartTime);
        lWIRRecord.SetWAFER_ID(mWaferId);
        lWIRRecord.Write(lStdfFile);

        // Write DTR
        QString lDatalog = "<cmd> gross_die=" + mGrossDie;
        GQTL_STDF::Stdf_DTR_V4 lDTRRecord;
        lDTRRecord.SetTEXT_DAT(lDatalog);
        lDTRRecord.Write(lStdfFile);

        lTotalGoodBin = lTotalFailBin = 0;

        for(int lIndex = 2; lIndex < mMapSpektraSummaryBinning.count(); lIndex++)
        {
            lBinNumber = mMapIndexToBinNumber[lIndex];

            mMapSpektraSummaryBinning[lBinNumber].mCount = lLine.section(",", lIndex, lIndex).simplified().toInt();
            if(mMapSpektraSummaryBinning[lBinNumber].mPass)
                lTotalGoodBin += mMapSpektraSummaryBinning[lBinNumber].mCount;
            else
                lTotalFailBin += mMapSpektraSummaryBinning[lBinNumber].mCount;
        }

        // Write WRR
        GQTL_STDF::Stdf_WRR_V4 lWRRRecord;
        lWRRRecord.SetHEAD_NUM(1);
        lWRRRecord.SetSITE_GRP(255);
        lWRRRecord.SetFINISH_T(mStartTime);
        lWRRRecord.SetPART_CNT(lTotalGoodBin + lTotalFailBin);
        lWRRRecord.SetGOOD_CNT(lTotalGoodBin);
        lWRRRecord.SetFUNC_CNT(0);
        lWRRRecord.SetABRT_CNT(0);
        lWRRRecord.SetRTST_CNT(0);
        lWRRRecord.SetWAFER_ID(mWaferId);
        lWRRRecord.Write(lStdfFile);

        // Write WCR
        if(mDiesUnit >= 0)
        {
            char lOrientation = ' ';
            if  (mFlatNotch==0)
                lOrientation = 'D';
            else if(mFlatNotch==90)
                lOrientation = 'L';
            else if(mFlatNotch==180)
                lOrientation = 'U';
            else if(mFlatNotch==270)
                lOrientation = 'R';

            GQTL_STDF::Stdf_WCR_V4 lWCRRecord;
            lWCRRecord.SetDIE_HT(mDiesXSize);
            lWCRRecord.SetDIE_WID(mDiesYSize);
            lWCRRecord.SetWF_UNITS(mDiesUnit);
            lWCRRecord.SetWF_FLAT(lOrientation);
            lWCRRecord.Write(lStdfFile);
        }

        GQTL_STDF::Stdf_HBR_V4 lHBRRecord;
        QMap<int, CGSpektraLotSummaryBinning>::Iterator it;
        for(it = mMapSpektraSummaryBinning.begin(); it != mMapSpektraSummaryBinning.end(); it++)
        {
            /// Do not write bin with count = 0 when using external file
            if(mBinMapStore.isNull() || mMapSpektraSummaryBinning[it.key()].mCount > 0)
            {
                // Write HBR
                lHBRRecord.SetHBIN_NUM(mMapSpektraSummaryBinning[it.key()].mNumber);
                lHBRRecord.SetHBIN_CNT(mMapSpektraSummaryBinning[it.key()].mCount);
                if(mMapSpektraSummaryBinning[it.key()].mPass)
                    lHBRRecord.SetHBIN_PF('P');
                else
                    lHBRRecord.SetHBIN_PF('F');
                lHBRRecord.SetHBIN_NAM(mMapSpektraSummaryBinning[it.key()].mName);
                lHBRRecord.Write(lStdfFile);
            }
        }

        GQTL_STDF::Stdf_SBR_V4 lSBRRecord;
        for(it = mMapSpektraSummaryBinning.begin(); it != mMapSpektraSummaryBinning.end(); it++)
        {
            /// Do not write bin with count = 0 when using external file
            if(mBinMapStore.isNull() || mMapSpektraSummaryBinning[it.key()].mCount > 0 )
            {
                // Write SBR
                lSBRRecord.SetSBIN_NUM(mMapSpektraSummaryBinning[it.key()].mNumber);
                lSBRRecord.SetSBIN_CNT(mMapSpektraSummaryBinning[it.key()].mCount);
                if(mMapSpektraSummaryBinning[it.key()].mPass)
                    lSBRRecord.SetSBIN_PF('P');
                else
                    lSBRRecord.SetSBIN_PF('F');
                lSBRRecord.SetSBIN_NAM(mMapSpektraSummaryBinning[it.key()].mName);
                lSBRRecord.Write(lStdfFile);
            }
        }

        // Write PCR
        GQTL_STDF::Stdf_PCR_V4 lPCRRecord;
        lPCRRecord.SetPART_CNT(lTotalGoodBin + lTotalFailBin);
        lPCRRecord.SetGOOD_CNT(lTotalGoodBin);
        lPCRRecord.SetABRT_CNT(0);
        lPCRRecord.SetRTST_CNT(0);
        lPCRRecord.Write(lStdfFile);

        // Write MRR
        GQTL_STDF::Stdf_MRR_V4 lMRRRecord;
        lMRRRecord.SetFINISH_T(mStartTime);
        lMRRRecord.Write(lStdfFile);

        // Close STDF file.
        lStdfFile.Close();
    }

    // Success
    return true;
}

}
}
