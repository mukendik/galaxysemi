#include "import_stdf.h"
#include "gqtl_log.h"
#include "stdfrecord.h"
#include <QFile>
#include <QFileInfo>

#include "converter_external_file.h"

namespace GS
{
namespace Parser
{


#define READ_RECORD(reader,rec,msg) if(reader.ReadRecord(&rec)==false) \
                                    {msg="Error while reading "+rec.GetRecordShortName();\
                                    return false;}
#define WRITE_RECORD(writer,rec,msg) if(writer.WriteRecord(&rec)==false) \
                                    {msg="Error while writing "+rec.GetRecordShortName();\
                                    return false;}

#define PROCESS_RECORD(reader,writer,rec,msg)   READ_RECORD(reader,rec,msg)\
                                                WRITE_RECORD(writer,rec,msg);

STDFtoSTDF::STDFtoSTDF(): ParserBase(typeStdf, "typeStdf")
{

}

bool STDFtoSTDF::IsCompatible(const QString &aFileName)
{
    QFileInfo lFileInfo(aFileName);
    QString lExtension = lFileInfo.suffix().toLower();

    return (lExtension == "stdf") || (lExtension == "std");
}

bool STDFtoSTDF::ConvertoStdf(const QString &aInputFileName, QString &aFileNameSTDF)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString(" file %1").arg(aInputFileName).toLatin1().constData());

    QString lErrorMsg;
    bool lIsExternalFile = mVishayStdfRecordOverWrite.LoadExternalFiles(aInputFileName,
                                                                        Qx::BinMapping::queryable,
                                                                        lErrorMsg,
                                                                        mLastError);
    if(lIsExternalFile)
    {
        QFileInfo aFileInfo(aInputFileName);
        mExternalFilePath = ConverterExternalFile::GetExternalFileName(aFileInfo.path());
        if(!getOptDefaultBinValuesfromExternalFile(aInputFileName, lErrorMsg))
        {
            mLastErrorMessage = lErrorMsg;
            mLastError = errReadBinMapFile;
            return false;
            //GSLOG(SYSLOG_SEV_INFORMATIONAL, qPrintable("getOptDefaultBinValuesfromExternalFile return false: " + lErrorMsg));
        }
        if(mDefaultBinSet)
        {
            mVishayStdfRecordOverWrite.setDefaultBinValues(mDefaultBinName, mDefaultBinNumber);
        }
    }
    else
    {
        if(mLastError == errReadBinMapFile)
        {
            mLastErrorMessage = lErrorMsg;
            return false;
        }
        // This case happens when no external file is specified. It is not an error. We need to return without any error message
        else if(mLastError == errInvalidFormatParameter)
        {
            return false;
        }
    }

    Qx::BinMapping::BinMapStoreTypes lFormat = mVishayStdfRecordOverWrite.GetFormat();
    if (   (lFormat == Qx::BinMapping::hvm_ft)
        || (lFormat == Qx::BinMapping::hvm_ws_spektra)
        || (lFormat == Qx::BinMapping::hvm_ws_fet_test))
    {
        mLastErrorMessage = lErrorMsg;
        mLastError = errReadBinMapFile;
        return false;
    }

    mStdfDetectedType = VishayStdfRecordOverWrite::undefinedType;
    mLastFailedTest = -1;

    if (StdfValidityCheck(aInputFileName) == false)
        return false;

    if(WriteStdfFile(aInputFileName, aFileNameSTDF) == false)
    {
        QFile::remove(aFileNameSTDF);
        return false;
    }

    mLastError = errNoError;
    // Success parsing CSV file
    return true;
}

bool STDFtoSTDF::WriteStdfFile(const QString &aInputStdfFile, const QString &aOutputStdfFile)
{
    GQTL_STDF::StdfParse lStdfReader;	// STDF V4 reader
    GQTL_STDF::StdfParse lStdfWriter;	// STDF V4 writer
    QString lErrorMsg;
    int lStatus, lRecordType;

    // Open input
    if(lStdfReader.Open(qPrintable(aInputStdfFile)) == false)
    {
        lErrorMsg = " - Failed opening input STDF file:" + aInputStdfFile;
        mLastErrorMessage = lErrorMsg;
        mLastError = errOpenFail;
        GSLOG(SYSLOG_SEV_ERROR, qPrintable(lErrorMsg));
        return false;
    }
    // Open output
    if(lStdfWriter.Open(qPrintable(aOutputStdfFile), STDF_WRITE) == false)
    {
        lErrorMsg = " - Failed opening output STDF file:" + aInputStdfFile;
        mLastErrorMessage = lErrorMsg;
        mLastError = errWriteSTDF;
        GSLOG(SYSLOG_SEV_ERROR, qPrintable(lErrorMsg));
        return false;
    }

    // Read and rewrite record
    lStatus = lStdfReader.LoadNextRecord(&lRecordType);
    while((lStatus == GQTL_STDF::StdfParse::NoError) || (lStatus == GQTL_STDF::StdfParse::UnexpectedRecord))
    {
        bool lIsProcessingPass = false;
        // Process STDF record read.
        switch(lRecordType)
        {
            case GQTL_STDF::Stdf_Record::Rec_FAR:
                lIsProcessingPass = true;
//                lIsProcessingPass = ProcessFAR(lStdfReader, lStdfWriter, lErrorMsg);
                break;
            case GQTL_STDF::Stdf_Record::Rec_ATR:
                lIsProcessingPass = ProcessATR(lStdfReader, lStdfWriter, lErrorMsg);
                break;
            case GQTL_STDF::Stdf_Record::Rec_MIR:
                lIsProcessingPass = ProcessMIR(lStdfReader, lStdfWriter, lErrorMsg);
                break;
            case GQTL_STDF::Stdf_Record::Rec_MRR:
                lIsProcessingPass = ProcessMRR(lStdfReader, lStdfWriter, lErrorMsg);
                break;
            case GQTL_STDF::Stdf_Record::Rec_PCR:
                {
                    if (WriteMappedHbrSbrList(lStdfWriter, lErrorMsg) == false)
                    {
                        lErrorMsg.prepend(" - Failed updating SBR/HBR in output STDF file");
                        mLastErrorMessage = lErrorMsg;
                        mLastError = errInvalidFormatParameter;
                        GSLOG(SYSLOG_SEV_ERROR, qPrintable(lErrorMsg));
                        return false;
                    }

                }
                lIsProcessingPass = ProcessPCR(lStdfReader, lStdfWriter, lErrorMsg);
                break;
            case GQTL_STDF::Stdf_Record::Rec_HBR:
                lIsProcessingPass = ProcessHBR(lStdfReader, lStdfWriter, lErrorMsg);
                break;
            case GQTL_STDF::Stdf_Record::Rec_SBR:
                lIsProcessingPass = ProcessSBR(lStdfReader, lErrorMsg);
                break;
            case GQTL_STDF::Stdf_Record::Rec_PMR:
                lIsProcessingPass = ProcessPMR(lStdfReader, lStdfWriter, lErrorMsg);
                break;
            case GQTL_STDF::Stdf_Record::Rec_PGR:
                lIsProcessingPass = ProcessPGR(lStdfReader, lStdfWriter, lErrorMsg);
                break;
            case GQTL_STDF::Stdf_Record::Rec_PLR:
                lIsProcessingPass = ProcessPLR(lStdfReader, lStdfWriter, lErrorMsg);
                break;
            case GQTL_STDF::Stdf_Record::Rec_RDR:
                lIsProcessingPass = ProcessRDR(lStdfReader, lStdfWriter, lErrorMsg);
                break;
            case GQTL_STDF::Stdf_Record::Rec_SDR:
                lIsProcessingPass = ProcessSDR(lStdfReader, lStdfWriter, lErrorMsg);
                break;
            case GQTL_STDF::Stdf_Record::Rec_WIR:
                lIsProcessingPass = ProcessWIR(lStdfReader, lStdfWriter, lErrorMsg);
                break;
            case GQTL_STDF::Stdf_Record::Rec_WRR:
                lIsProcessingPass = ProcessWRR(lStdfReader, lStdfWriter, lErrorMsg);
                break;
            case GQTL_STDF::Stdf_Record::Rec_WCR:
                lIsProcessingPass = ProcessWCR(lStdfReader, lStdfWriter, lErrorMsg);
                break;
            case GQTL_STDF::Stdf_Record::Rec_PIR:
                lIsProcessingPass = ProcessPIR(lStdfReader, lStdfWriter, lErrorMsg);
                break;
            case GQTL_STDF::Stdf_Record::Rec_PRR:
                lIsProcessingPass = ProcessPRR(lStdfReader, lStdfWriter, lErrorMsg);
                break;
            case GQTL_STDF::Stdf_Record::Rec_TSR:
                lIsProcessingPass = ProcessTSR(lStdfReader, lStdfWriter, lErrorMsg);
                break;
            case GQTL_STDF::Stdf_Record::Rec_PTR:
                lIsProcessingPass = ProcessPTR(lStdfReader, lStdfWriter, lErrorMsg);
                break;
            case GQTL_STDF::Stdf_Record::Rec_MPR:
                lIsProcessingPass = ProcessMPR(lStdfReader, lStdfWriter, lErrorMsg);
                break;
            case GQTL_STDF::Stdf_Record::Rec_FTR:
                lIsProcessingPass = ProcessFTR(lStdfReader, lStdfWriter, lErrorMsg);
                break;
            case GQTL_STDF::Stdf_Record::Rec_BPS:
                lIsProcessingPass = ProcessBPS(lStdfReader, lStdfWriter, lErrorMsg);
                break;
            case GQTL_STDF::Stdf_Record::Rec_EPS:
                lIsProcessingPass = ProcessEPS(lStdfReader, lStdfWriter, lErrorMsg);
                break;
            case GQTL_STDF::Stdf_Record::Rec_GDR:
                lIsProcessingPass = ProcessGDR(lStdfReader, lStdfWriter, lErrorMsg);
                break;
            case GQTL_STDF::Stdf_Record::Rec_DTR:
                lIsProcessingPass = ProcessDTR(lStdfReader, lStdfWriter, lErrorMsg);
                break;
            case GQTL_STDF::Stdf_Record::Rec_RESERVED_IMAGE:
                lIsProcessingPass = ProcessRESERVED_IMAGE(lStdfReader, lStdfWriter, lErrorMsg);
                break;
            case GQTL_STDF::Stdf_Record::Rec_RESERVED_IG900:
                lIsProcessingPass = ProcessRESERVED_IG900(lStdfReader, lStdfWriter, lErrorMsg);
                break;
            case GQTL_STDF::Stdf_Record::Rec_UNKNOWN:
                lIsProcessingPass = ProcessUNKNOWN(lStdfReader, lStdfWriter, lErrorMsg);
                break;
            case GQTL_STDF::Stdf_Record::Rec_VUR :
                lIsProcessingPass = ProcessVUR(lStdfReader, lStdfWriter, lErrorMsg);
                break;
            case GQTL_STDF::Stdf_Record::Rec_PSR :
                lIsProcessingPass = ProcessPSR(lStdfReader, lStdfWriter, lErrorMsg);
                break;
            case GQTL_STDF::Stdf_Record::Rec_NMR :
                lIsProcessingPass = ProcessNMR(lStdfReader, lStdfWriter, lErrorMsg);
                break;
            case GQTL_STDF::Stdf_Record::Rec_CNR :
                lIsProcessingPass = ProcessCNR(lStdfReader, lStdfWriter, lErrorMsg);
                break;
            case GQTL_STDF::Stdf_Record::Rec_SSR :
                lIsProcessingPass = ProcessSSR(lStdfReader, lStdfWriter, lErrorMsg);
                break;
            case GQTL_STDF::Stdf_Record::Rec_CDR :
                lIsProcessingPass = ProcessCDR(lStdfReader, lStdfWriter, lErrorMsg);
                break;
            case GQTL_STDF::Stdf_Record::Rec_STR :
                lIsProcessingPass = ProcessSTR(lStdfReader, lStdfWriter, lErrorMsg);
                break;
            default:
                lIsProcessingPass = false;
                break;
        }

        if (lIsProcessingPass == false)
        {
            lErrorMsg.prepend(" - Failed processing input STDF file:" + aInputStdfFile + " - ");
            mLastErrorMessage = lErrorMsg;
            mLastError = errInvalidFormatParameter;
            GSLOG(SYSLOG_SEV_ERROR, qPrintable(lErrorMsg));
            return false;
        }

        // Read one record from STDF file.
        lStatus = lStdfReader.LoadNextRecord(&lRecordType);
    };

    lStdfReader.Close();
    lStdfWriter.Close();

    return true;
}

bool STDFtoSTDF::ProcessFAR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& lErrorMsg)
{
    GQTL_STDF::Stdf_FAR_V4 lRecord;
    PROCESS_RECORD(aReader, aWriter, lRecord, lErrorMsg);
    return true;
}

bool STDFtoSTDF::ProcessATR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& lErrorMsg)
{
    GQTL_STDF::Stdf_ATR_V4 lRecord;
    PROCESS_RECORD(aReader, aWriter, lRecord, lErrorMsg);
    return true;
}

bool STDFtoSTDF::WriteATR(GQTL_STDF::StdfParse& aWriter, QString& lErrorMsg)
{
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_FTEST;
    strUserTxt += ":STDFtoSTDF";

    GQTL_STDF::Stdf_ATR_V4 lATRRecord;
    lATRRecord.SetMOD_TIM(time(NULL));                          // MOD_TIME: current time & date.
    lATRRecord.SetCMD_LINE(strUserTxt.toLatin1().constData());	// ATR string.

    // Write record to disk
     WRITE_RECORD(aWriter,lATRRecord,lErrorMsg);
     return true;
}

bool STDFtoSTDF::ProcessMIR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& lErrorMsg)
{
    GQTL_STDF::Stdf_MIR_V4 lMIRRecord;
    READ_RECORD(aReader,lMIRRecord,lErrorMsg);

    // Write the ATR to explicitally indicate that the file has been changed and we did a bin mapping
    if (!WriteATR(aWriter, lErrorMsg))
    {
        return false;
    }

    QString lPromisKey;

    if (mStdfDetectedType == VishayStdfRecordOverWrite::waferType)
    {
        lPromisKey = QString(lMIRRecord.m_cnLOT_ID) + "." + QString::number(mStdfWaferId.toInt());
    }
    else if (mStdfDetectedType == VishayStdfRecordOverWrite::finalTestType)
    {
        lPromisKey = lMIRRecord.m_cnLOT_ID;
    }
    else
    {
        lErrorMsg = "Undefined stdf type. Cannot create lookup key for Promis data";
        mLastError = errReadPromisFile;
        return false;
    }

    if (mVishayStdfRecordOverWrite.UpdateMIRRecordWithPromis(lMIRRecord, lPromisKey, lErrorMsg) == false)

    {
        mLastError = errReadPromisFile;
        return false;
    }

    WRITE_RECORD(aWriter,lMIRRecord,lErrorMsg);

    GQTL_STDF::Stdf_DTR_V4 lDTRRecordDieCount;
    if( mVishayStdfRecordOverWrite.WritePromisGrossDieCount(lDTRRecordDieCount))
    {
        WRITE_RECORD(aWriter,lDTRRecordDieCount,lErrorMsg);
    }

    QList<GQTL_STDF::Stdf_DTR_V4*> lDTRRecordDieTracking;
    mVishayStdfRecordOverWrite.WritePromisDieTracking(lDTRRecordDieTracking);

    QList<GQTL_STDF::Stdf_DTR_V4*>::iterator lIter(lDTRRecordDieTracking.begin()),
                                                    lIterEnd(lDTRRecordDieTracking.end());
    for(; lIter != lIterEnd; ++lIter)
    {
        WRITE_RECORD(aWriter,(**lIter),lErrorMsg);
    }
    qDeleteAll(lDTRRecordDieTracking);

    return true;
}

bool STDFtoSTDF::ProcessMRR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg)
{
    GQTL_STDF::Stdf_MRR_V4 lRecord;
    PROCESS_RECORD(aReader, aWriter, lRecord, aErrorMsg);
    return true;
}

bool STDFtoSTDF::ProcessPCR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg)
{
    GQTL_STDF::Stdf_PCR_V4 lRecord;
    PROCESS_RECORD(aReader, aWriter, lRecord, aErrorMsg);
    return true;
}

bool STDFtoSTDF::ProcessHBR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg)
{
    GQTL_STDF::Stdf_HBR_V4 lRecord;
    READ_RECORD(aReader, lRecord, aErrorMsg);
    // Read record but do write it as it is
    if (mStdfDetectedType == VishayStdfRecordOverWrite::waferType ||
            mStdfDetectedType == VishayStdfRecordOverWrite::eTestType)
    {
        WRITE_RECORD(aWriter, lRecord, aErrorMsg);
    }
    return true;
}

bool STDFtoSTDF::ProcessSBR(GQTL_STDF::StdfParse& aReader, QString& aErrorMsg)
{
    GQTL_STDF::Stdf_SBR_V4 lRecord;
    // Read record but do write it as it is
    READ_RECORD(aReader,lRecord,aErrorMsg);
    // Extract bin num/nam to update map
    mVishayStdfRecordOverWrite.UpdateBin(lRecord);
    return true;
}

bool STDFtoSTDF::ProcessPMR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg)
{
    GQTL_STDF::Stdf_PMR_V4 lRecord;
    PROCESS_RECORD(aReader, aWriter, lRecord, aErrorMsg);
    return true;
}


bool STDFtoSTDF::ProcessPGR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg)
{
    GQTL_STDF::Stdf_PGR_V4 lRecord;
    PROCESS_RECORD(aReader, aWriter, lRecord, aErrorMsg);
    return true;
}

bool STDFtoSTDF::ProcessPLR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg)
{
    GQTL_STDF::Stdf_PLR_V4 lRecord;
    PROCESS_RECORD(aReader, aWriter, lRecord, aErrorMsg);
    return true;
}

bool STDFtoSTDF::ProcessRDR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg)
{
    GQTL_STDF::Stdf_RDR_V4 lRecord;
    PROCESS_RECORD(aReader, aWriter, lRecord, aErrorMsg);
    return true;
}

bool STDFtoSTDF::ProcessSDR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg)
{
    GQTL_STDF::Stdf_SDR_V4 lRecord;
    READ_RECORD(aReader,lRecord,aErrorMsg);

    mVishayStdfRecordOverWrite.UpdateSBRRecordWithPromis(lRecord);

    WRITE_RECORD(aWriter,lRecord,aErrorMsg);

    return true;
}

bool STDFtoSTDF::ProcessWIR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg)
{
    GQTL_STDF::Stdf_WIR_V4 lRecord;
    PROCESS_RECORD(aReader, aWriter, lRecord, aErrorMsg);
    return true;
}

bool STDFtoSTDF::ProcessWRR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg)
{
    GQTL_STDF::Stdf_WRR_V4 lRecord;
    PROCESS_RECORD(aReader, aWriter, lRecord, aErrorMsg);
    return true;
}

bool STDFtoSTDF::ProcessWCR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg)
{
    GQTL_STDF::Stdf_WCR_V4 lRecord;
    PROCESS_RECORD(aReader, aWriter, lRecord, aErrorMsg);
    return true;
}

bool STDFtoSTDF::ProcessPIR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg)
{
    GQTL_STDF::Stdf_PIR_V4 lRecord;
    mLastFailedTest = -1;
    PROCESS_RECORD(aReader, aWriter, lRecord, aErrorMsg);
    return true;
}

bool STDFtoSTDF::ProcessPRR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg)
{
    GQTL_STDF::Stdf_PRR_V4 lRecord;
    READ_RECORD(aReader,lRecord,aErrorMsg);

    if (mVishayStdfRecordOverWrite.UpdatePRRRecordWithBinMapping(lRecord, mLastFailedTest, aErrorMsg, appendBinMappingExceptionInfo()) == false)
    {
        mLastError = errInvalidFormatParameter;
        mLastErrorMessage = aErrorMsg;
        return false;
    }

    WRITE_RECORD(aWriter,lRecord,aErrorMsg);

    return true;
}

bool STDFtoSTDF::ProcessTSR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg)
{
    GQTL_STDF::Stdf_TSR_V4 lRecord;
    PROCESS_RECORD(aReader, aWriter, lRecord, aErrorMsg);
    return true;
}

bool STDFtoSTDF::ProcessPTR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg)
{
    GQTL_STDF::Stdf_PTR_V4 lRecord;
    READ_RECORD(aReader,lRecord,aErrorMsg);
    bool resProcessing = true;

    if (lRecord.IsTestFail())
    {
        resProcessing = mVishayStdfRecordOverWrite.readPTRRecordWithBinMapping(lRecord, mLastFailedTest, aErrorMsg);
    }
    if(resProcessing)
        WRITE_RECORD(aWriter,lRecord,aErrorMsg);

    return resProcessing;
}

bool STDFtoSTDF::ProcessMPR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg)
{
    GQTL_STDF::Stdf_MPR_V4 lRecord;
    PROCESS_RECORD(aReader, aWriter, lRecord, aErrorMsg);
    return true;
}

bool STDFtoSTDF::ProcessFTR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg)
{
    GQTL_STDF::Stdf_FTR_V4 lRecord;
    PROCESS_RECORD(aReader, aWriter, lRecord, aErrorMsg);
    return true;
}

bool STDFtoSTDF::ProcessBPS(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg)
{
    GQTL_STDF::Stdf_BPS_V4 lRecord;
    PROCESS_RECORD(aReader, aWriter, lRecord, aErrorMsg);
    return true;
}

bool STDFtoSTDF::ProcessEPS(GQTL_STDF::StdfParse& lReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg)
{
    GQTL_STDF::Stdf_EPS_V4 lRecord;
    PROCESS_RECORD(lReader, aWriter, lRecord, aErrorMsg);
    return true;
}

bool STDFtoSTDF::ProcessGDR(GQTL_STDF::StdfParse& lReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg)
{
    GQTL_STDF::Stdf_GDR_V4 lRecord;
    PROCESS_RECORD(lReader, aWriter, lRecord, aErrorMsg);
    return true;
}

bool STDFtoSTDF::ProcessDTR(GQTL_STDF::StdfParse& lReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg)
{
    GQTL_STDF::Stdf_DTR_V4 lRecord;
    PROCESS_RECORD(lReader, aWriter, lRecord, aErrorMsg);
    return true;
}

bool STDFtoSTDF::ProcessRESERVED_IMAGE(GQTL_STDF::StdfParse& lReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg)
{
    GQTL_STDF::Stdf_RESERVED_IMAGE_V4 lRecord;
    PROCESS_RECORD(lReader, aWriter, lRecord, aErrorMsg);
    return true;
}

bool STDFtoSTDF::ProcessRESERVED_IG900(GQTL_STDF::StdfParse& lReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg)
{
    GQTL_STDF::Stdf_RESERVED_IG900_V4 lRecord;
    PROCESS_RECORD(lReader, aWriter, lRecord, aErrorMsg);
    return true;
}

bool STDFtoSTDF::ProcessUNKNOWN(GQTL_STDF::StdfParse& lReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg)
{
    GQTL_STDF::Stdf_UNKNOWN_V4 lRecord;
    PROCESS_RECORD(lReader, aWriter, lRecord, aErrorMsg);
    return true;
}

bool STDFtoSTDF::ProcessVUR(GQTL_STDF::StdfParse& lReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg)
{
    GQTL_STDF::Stdf_VUR_V4 lRecord;
    PROCESS_RECORD(lReader, aWriter, lRecord, aErrorMsg);
    return true;
}

bool STDFtoSTDF::ProcessPSR(GQTL_STDF::StdfParse& lReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg)
{
    GQTL_STDF::Stdf_PSR_V4 lRecord;
    PROCESS_RECORD(lReader, aWriter, lRecord, aErrorMsg);
    return true;
}

bool STDFtoSTDF::ProcessNMR(GQTL_STDF::StdfParse& lReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg)
{
    GQTL_STDF::Stdf_NMR_V4 lRecord;
    PROCESS_RECORD(lReader, aWriter, lRecord, aErrorMsg);
    return true;
}

bool STDFtoSTDF::ProcessCNR(GQTL_STDF::StdfParse& lReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg)
{
    GQTL_STDF::Stdf_CNR_V4 lRecord;
    PROCESS_RECORD(lReader, aWriter, lRecord, aErrorMsg);
    return true;
}

bool STDFtoSTDF::ProcessSSR(GQTL_STDF::StdfParse& lReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg)
{
    GQTL_STDF::Stdf_SSR_V4 lRecord;
    PROCESS_RECORD(lReader, aWriter, lRecord, aErrorMsg);
    return true;
}

bool STDFtoSTDF::ProcessCDR(GQTL_STDF::StdfParse& lReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg)
{
    GQTL_STDF::Stdf_CDR_V4 lRecord;
    PROCESS_RECORD(lReader, aWriter, lRecord, aErrorMsg);
    return true;
}

bool STDFtoSTDF::ProcessSTR(GQTL_STDF::StdfParse& lReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg)
{
    GQTL_STDF::Stdf_STR_V4 lRecord;
    PROCESS_RECORD(lReader, aWriter, lRecord, aErrorMsg);
    return true;
}

bool STDFtoSTDF::WriteMappedHbrSbrList(GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg)
{
    QMap<int,ParserBinning>::Iterator lIter;

    // Rewrite SBR
    GQTL_STDF::Stdf_SBR_V4 lSBRRecord;
    for(lIter = mVishayStdfRecordOverWrite.mSoftBinning.begin();
        lIter != mVishayStdfRecordOverWrite.mSoftBinning.end(); ++lIter)
    {
        mVishayStdfRecordOverWrite.UpdateSBRRecord(lSBRRecord, *lIter);
        WRITE_RECORD(aWriter, lSBRRecord, aErrorMsg);
    }

    // Do not rewrite HBR with wafer type
    if (mStdfDetectedType == VishayStdfRecordOverWrite::finalTestType)
    {
        GQTL_STDF::Stdf_HBR_V4 lHBRRecord;
        for(lIter = mVishayStdfRecordOverWrite.mSoftBinning.begin();
            lIter != mVishayStdfRecordOverWrite.mSoftBinning.end(); ++lIter)
        {
            mVishayStdfRecordOverWrite.UdpateHBRRecord(lHBRRecord, *lIter);
            WRITE_RECORD(aWriter, lHBRRecord, aErrorMsg);
        }
    }

    return true;
}

bool STDFtoSTDF::StdfValidityCheck(const QString &aInputStdfFile)
{
    GQTL_STDF::StdfParse lStdfReader;	// STDF V4 reader
    QString lErrorMsg;
    bool    lIsProcessingPass = true;
    int     lStatus, lRecordType;
    int     lPIRCount = 0;

    // Open input
    if(lStdfReader.Open(qPrintable(aInputStdfFile)) == false)
    {
        lErrorMsg = " - Failed opening input STDF file:" + aInputStdfFile;
        mLastErrorMessage = lErrorMsg;
        mLastError = errOpenFail;
        GSLOG(SYSLOG_SEV_ERROR, qPrintable(lErrorMsg));
        return false;
    }

    // Read and rewrite record
    lStatus = lStdfReader.LoadNextRecord(&lRecordType);
    while(((lStatus == GQTL_STDF::StdfParse::NoError) || (lStatus == GQTL_STDF::StdfParse::UnexpectedRecord))
          && mStdfDetectedType == VishayStdfRecordOverWrite::undefinedType)
    {
        // Process STDF record read.
        switch(lRecordType)
        {
            case GQTL_STDF::Stdf_Record::Rec_WIR:
                lIsProcessingPass = StdfValidityCheckWIR(lStdfReader, lErrorMsg);
                break;
            case GQTL_STDF::Stdf_Record::Rec_PIR:
                if (++lPIRCount > 1)
                    lIsProcessingPass = StdfValidityCheckPIR(lErrorMsg);
                break;
            default:
                break;
        }

        if (lIsProcessingPass == false)
        {
            lErrorMsg.prepend(" - Failed processing input STDF file:" + aInputStdfFile + " - ");
            mLastErrorMessage = lErrorMsg;
            mLastError = errInvalidFormatParameter;
            GSLOG(SYSLOG_SEV_ERROR, qPrintable(lErrorMsg));
            return false;
        }

        // Read one record from STDF file.
        lStatus = lStdfReader.LoadNextRecord(&lRecordType);
    };

    if (mStdfDetectedType == VishayStdfRecordOverWrite::undefinedType)
    {
        mLastErrorMessage = " - Failed processing input STDF file:" + aInputStdfFile + " - Unable to detect stdf type";
        mLastError = errInvalidFormatParameter;
        return false;
    }

    return true;
}

bool STDFtoSTDF::StdfValidityCheckWIR(GQTL_STDF::StdfParse &aReader, QString &aErrorMsg)
{
    if (mVishayStdfRecordOverWrite.GetType() == VishayStdfRecordOverWrite::finalTestType)
    {
        aErrorMsg = "type mismatch between input file stdf file and external";
        GSLOG(SYSLOG_SEV_ERROR, qPrintable(aErrorMsg));
        return false;
    }

    GQTL_STDF::Stdf_WIR_V4 lRecord;
    if (aReader.ReadRecord(&lRecord) == false)
    {
        aErrorMsg = "Error while reading " + lRecord.GetRecordShortName();
        return false;
    }

    mStdfDetectedType = VishayStdfRecordOverWrite::waferType;
    mStdfWaferId = lRecord.m_cnWAFER_ID;

    return true;
}

bool STDFtoSTDF::StdfValidityCheckPIR(QString &aErrorMsg)
{
    if (mVishayStdfRecordOverWrite.GetType() == VishayStdfRecordOverWrite::waferType)
    {
        aErrorMsg = "type mismatch between input file stdf file and external";
        GSLOG(SYSLOG_SEV_ERROR, qPrintable(aErrorMsg));
        return false;
    }

    mStdfDetectedType = VishayStdfRecordOverWrite::finalTestType;

    return true;
}



} // Parser
} // GS
