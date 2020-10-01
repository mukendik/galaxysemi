#include "pat_gts_station.h"
#include "gtl_core.h"
#include "gstdl_systeminfo.h"
#include "gqtl_log.h"
#include "cbinning.h"
#include "gex_constants.h"
#include "import_all.h"
#include "temporary_files_manager.h"
#include "engine.h"
#include "product_info.h"
#include "pat_info.h"

#include <QDir>
#include <QFileInfo>
#include <QElapsedTimer>
#include <QSharedPointer>

template <typename T> void SharedPointerArrayDeleter(T array[])
{
    delete [] array;
}

namespace GS
{
namespace Gex
{

PATGtsStation::PATGtsStation(QObject *lParent)
    : QObject(lParent), mGenerateOutputTestData(false), mBinRecordsCreated(false),
      mPartCount(), mPartRead(0)
{
}

PATGtsStation::~PATGtsStation()
{
    qDeleteAll(mHardBinning);
    mHardBinning.clear();

    qDeleteAll(mSoftBinning);
    mSoftBinning.clear();
}

QString PATGtsStation::GetErrorMessage() const
{
    return mErrorMessage;
}

void PATGtsStation::SetTraceabilityFile(const QString &lTraceabilityFile)
{
    mTraceabilityFile = lTraceabilityFile;
}

void PATGtsStation::SetInputDataFile(const QString &lInputData)
{
    mInputDataFile = lInputData;
}

void PATGtsStation::SetTesterConf(const QString &lTesterConf)
{
    mTesterConf = lTesterConf;
}

void PATGtsStation::SetOutputDataFile(const QString &lOutputData)
{
    mOutputSTDFFile = lOutputData;
}

void PATGtsStation::SetRecipeFile(const QString &lRecipe)
{
    mRecipeFile = lRecipe;
}

bool PATGtsStation::ExecuteTestProgram()
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("FT PAT Simulator: executing test program...").toLatin1().constData());

    if (GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT() == false &&
        GS::LPPlugin::ProductInfo::getInstance()->isPATMan() == false)
    {
        mErrorMessage = "This function requires either PAT-Man or Examinator-PAT license.";

        emit aborted(mErrorMessage);
        return false;
    }

    if (InitStation() == false)
    {
        emit aborted(mErrorMessage);
        return false;
    }

    if (OpenTestProgram() == false)
    {
        emit aborted(mErrorMessage);
        return false;
    }

    if (ProcessFile() == false)
    {
        emit aborted(mErrorMessage);

        CloseTestProgram();

        return false;
    }

    if (CloseTestProgram() == false)
    {
        emit aborted(mErrorMessage);
        return false;
    }

    emit finished(mOutputSTDFFile, mTraceabilityFile);

    return true;
}

bool PATGtsStation::IsValidSTDF()
{
    GQTL_STDF::StdfParse	lSTDFReader;

    // Open STDF file
    if (lSTDFReader.Open(mInputSTDFFile.toLatin1().constData()) == false)
    {
        mErrorMessage = "Failed to open input file " + mInputSTDFFile;
        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
        return false;
    }

    // Check for MIR record
    if(lSTDFReader.LoadNextRecord(GQTL_STDF::Stdf_Record::Rec_MIR, true) != GQTL_STDF::StdfParse::NoError)
    {
        mErrorMessage = "Invalid STDF file (no MIR record) " + mInputSTDFFile;
        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
        return false;
    }

    // Check for PIR record
    if(lSTDFReader.LoadNextRecord(GQTL_STDF::Stdf_Record::Rec_PIR, true) != GQTL_STDF::StdfParse::NoError)
    {
        mErrorMessage = "Invalid STDF file (no PIR record): " + mInputSTDFFile;
        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
        return false;
    }

    // Check for WIR, WRR or WCR record
    if(lSTDFReader.LoadNextRecord(GQTL_STDF::Stdf_Record::Rec_WIR, true) == GQTL_STDF::StdfParse::NoError)
    {
        mErrorMessage = "Invalid STDF file (WIR record found): " + mInputSTDFFile;
        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
        return false;
    }

    if(lSTDFReader.LoadNextRecord(GQTL_STDF::Stdf_Record::Rec_WRR, true) == GQTL_STDF::StdfParse::NoError)
    {
        mErrorMessage = "Invalid STDF file (WRR record found): " + mInputSTDFFile;
        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
        return false;
    }

    if(lSTDFReader.LoadNextRecord(GQTL_STDF::Stdf_Record::Rec_WCR, true) == GQTL_STDF::StdfParse::NoError)
    {
        mErrorMessage = "Invalid STDF file (WCR record found): " + mInputSTDFFile;
        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
        return false;
    }

    return true;
}


bool PATGtsStation::ConvertToSTDF()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL,"Converting input file to STDF (if needed)");

    PAT_PERF_BENCH

    //-- Build output name and extension
    GS::Gex::ConvertToSTDF  StdfConvert;
    bool            lFileCreated    = false;
    int             lStatus         = false;

    mInputSTDFFile  = Engine::GetInstance().Get("TempFolder").toString() + QDir::separator() + QFileInfo(mInputDataFile).fileName() + GEX_TEMPORARY_STDF;

    //lStatus         = StdfConvert.Convert(mInputDataFile, mInputSTDFFile);
    lStatus         = StdfConvert.Convert(false, true, false, true, mInputDataFile, mInputSTDFFile,"", lFileCreated, mErrorMessage, false);

    if(lStatus == GS::Gex::ConvertToSTDF::eConvertError)
        return false;

    // -- Check if Input file was already in STDF format!
    if(lFileCreated == false)
        mInputSTDFFile = mInputDataFile;
    else
        // - Add file to list of temporary files to erase on Examinator exit...
        Engine::GetInstance().GetTempFilesManager().addFile(mInputSTDFFile, TemporaryFile::BasicCheck);

    return true;
}

bool PATGtsStation::InitStation()
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("FT PAT Simulator: initializing station...").toLatin1().constData());

    if (mInputDataFile.isEmpty())
    {
        mErrorMessage = "Input STDF file path is empty";
        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
        return false;
    }

    if (mRecipeFile.isEmpty())
    {
        mErrorMessage = "Recipe file path is empty";
        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
        return false;
    }

    if (mTesterConf.isEmpty())
    {
        mErrorMessage = "Tester configuration file path is empty";
        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
        return false;
    }

   if(ConvertToSTDF() == false)
   {
       mErrorMessage = "Convertion to std file failed";
       GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
       return false;
   }

    if (IsValidSTDF() == false)
        return false;

    GQTL_STDF::Stdf_PIR_V4		lRecordPIR;
    GQTL_STDF::Stdf_SBR_V4		lRecordSBR;
    GQTL_STDF::Stdf_HBR_V4		lRecordHBR;
    GQTL_STDF::Stdf_GDR_V4		lRecordGDR;
    GQTL_STDF::StdfParse	lSTDFReader;
    CBinning *                  lBinning    = NULL;
    QElapsedTimer               lTimer;
    int                         nStatus;
    int                         nRecordType;

    unsigned short siteNumber = 0;

    // Open STDF file
    if (lSTDFReader.Open(mInputSTDFFile.toLatin1().constData()) == false)
    {
        mErrorMessage = QString("Failed to open input STDF file %1").arg(mInputSTDFFile);
        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
        return false;
    }

    // Read all records to get nb of sites, list of binnings...
    mPartRead   = 0;
    mPartCount  = 0;
    mSites.clear();
    mMIRRecord.Reset();

    qDeleteAll(mSoftBinning);
    mSoftBinning.clear();

    qDeleteAll(mHardBinning);
    mHardBinning.clear();

    lTimer.start();

    nStatus = lSTDFReader.LoadNextRecord(&nRecordType);

    while((nStatus == GQTL_STDF::StdfParse::NoError) ||
          (nStatus == GQTL_STDF::StdfParse::UnexpectedRecord))
    {
        switch(nRecordType)
        {
            case GQTL_STDF::Stdf_Record::Rec_MIR:
                // PIR record
                if (lSTDFReader.ReadRecord((GQTL_STDF::Stdf_Record *)&mMIRRecord) == false)
                {
                    mErrorMessage = QString("Failed to read MIR record at index %1")
                                    .arg(lSTDFReader.GetReadRecordPos());
                    GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
                    return false;
                }
                break;

            case GQTL_STDF::Stdf_Record::Rec_PIR:
                // PIR record
                if (lSTDFReader.ReadRecord((GQTL_STDF::Stdf_Record *)&lRecordPIR) == false)
                {
                    mErrorMessage = QString("Failed to read PIR record at index %1")
                                    .arg(lSTDFReader.GetReadRecordPos());
                    GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
                    return false;
                }

                siteNumber = GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
                    ( lSTDFReader, lRecordPIR );

                if ( mSites.contains( siteNumber ) == false )
                    mSites.insert( siteNumber, eSiteIdle );

                mPartCount++;
                break;

            case GQTL_STDF::Stdf_Record::Rec_SBR:
                // SBR record
                lSTDFReader.ReadRecord((GQTL_STDF::Stdf_Record *)&lRecordSBR);

                siteNumber = GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
                    ( lSTDFReader, lRecordSBR );

                if ( mSoftBinning.contains( siteNumber ) )
                {
                    lBinning = mSoftBinning[ siteNumber ];

                    if ( lBinning->Find( siteNumber ) == NULL )
                        lBinning->Append(lRecordSBR.m_u2SBIN_NUM, lRecordSBR.m_c1SBIN_PF,
                                         lRecordSBR.m_u4SBIN_CNT, lRecordSBR.m_cnSBIN_NAM);
                }
                else
                {
                    lBinning = new CBinning();

                    lBinning->iBinValue     = lRecordSBR.m_u2SBIN_NUM;
                    lBinning->cPassFail     = lRecordSBR.m_c1SBIN_PF;
                    lBinning->strBinName    = lRecordSBR.m_cnSBIN_NAM;
                    lBinning->ldTotalCount  = lRecordSBR.m_u4SBIN_CNT;

                    mSoftBinning.insert( siteNumber, lBinning );
                }

                break;

            case GQTL_STDF::Stdf_Record::Rec_HBR:
                // HBR record
                lSTDFReader.ReadRecord((GQTL_STDF::Stdf_Record *)&lRecordHBR);

                siteNumber = GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
                    ( lSTDFReader, lRecordHBR );

                if ( mHardBinning.contains( siteNumber ) )
                {
                    lBinning = mHardBinning[ siteNumber ];

                    if ( lBinning->Find( siteNumber ) == NULL )
                        lBinning->Append(lRecordHBR.m_u2HBIN_NUM, lRecordHBR.m_c1HBIN_PF,
                                         lRecordHBR.m_u4HBIN_CNT, lRecordHBR.m_cnHBIN_NAM);
                }
                else
                {
                    lBinning = new CBinning();

                    lBinning->iBinValue     = lRecordHBR.m_u2HBIN_NUM;
                    lBinning->cPassFail     = lRecordHBR.m_c1HBIN_PF;
                    lBinning->strBinName    = lRecordHBR.m_cnHBIN_NAM;
                    lBinning->ldTotalCount  = lRecordHBR.m_u4HBIN_CNT;

                    mHardBinning.insert( siteNumber, lBinning );
                }

                break;

        case GQTL_STDF::Stdf_Record::Rec_GDR:
            // set deciphering mode if needed for site number
            lSTDFReader.ReadRecord( &lRecordGDR );
            GQTL_STDF::HeadAndSiteNumberDecipher::SetDecipheringModeInParser
                ( lRecordGDR, lSTDFReader );
            break;

            default:
                break;
        }

        // Load next record
        nStatus = lSTDFReader.LoadNextRecord(&nRecordType);
    }

    // Close and re-open STDF file (rewind doesn't work once too much data read)
    lSTDFReader.Close();

    return true;
}

#define GETSTRING(x) #x

bool PATGtsStation::CloseTestProgram()
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("FT PAT Simulator: closing test program...").toLatin1().constData());

    // If required, call GTL close function
    // gcore-1180
    //if(gtl_get_lib_state() != GTL_STATE_NOT_INITIALIZED)
    char lLibState[1024]="?";
    gtl_get(GTL_KEY_LIB_STATE, lLibState);
    if (QString(lLibState).toInt()!=GTL_STATE_NOT_INITIALIZED)
    {
        //int lResult = gtl_close();
        int lResult = gtl_command((char*)GTL_COMMAND_CLOSE);
        //        PopGtlMessageStack();
        if (lResult != GTL_CORE_ERR_OKAY)
        {
            mErrorMessage = QString("gtl_close failed (return code %1).").arg(lResult);
            GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
            return false;
        }
    }

    return true;
}

bool PATGtsStation::OpenTestProgram()
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("FT PAT Simulator: opening test program...").toLatin1().constData());

    // gcore-1180
    /*
    int lResult = gtl_set_prod_info(mMIRRecord.m_cnJOB_NAM.toLatin1().data(),
                                    mMIRRecord.m_cnJOB_REV.toLatin1().data(),
                                    mMIRRecord.m_cnLOT_ID.toLatin1().data(),
                                    mMIRRecord.m_cnSBLOT_ID.toLatin1().data(),
                                    mMIRRecord.m_cnPART_TYP.toLatin1().data());
    */

    GSLOG(SYSLOG_SEV_DEBUG, QString("FT PAT Simulator: setting GTL_KEY_JOB_NAME...").toLatin1().constData());
    int lResult=gtl_set((char*)GTL_KEY_JOB_NAME, mMIRRecord.m_cnJOB_NAM.toLatin1().data());
    if (lResult!=0)
        goto gtl_set_prod_info_failed;
    GSLOG(SYSLOG_SEV_DEBUG, QString("FT PAT Simulator: setting GTL_KEY_JOB_REVISION...").toLatin1().constData());
    lResult=gtl_set((char*)GTL_KEY_JOB_REVISION, mMIRRecord.m_cnJOB_REV.toLatin1().data());
    if (lResult!=0)
        goto gtl_set_prod_info_failed;
    GSLOG(SYSLOG_SEV_DEBUG, QString("FT PAT Simulator: setting GTL_KEY_LOT_ID...").toLatin1().constData());
    lResult=gtl_set((char*)GTL_KEY_LOT_ID, mMIRRecord.m_cnLOT_ID.toLatin1().data());
    if (lResult!=0)
        goto gtl_set_prod_info_failed;
    GSLOG(SYSLOG_SEV_DEBUG, QString("FT PAT Simulator: setting GTL_KEY_SUBLOT_ID...").toLatin1().constData());
    lResult=gtl_set((char*)GTL_KEY_SUBLOT_ID, mMIRRecord.m_cnSBLOT_ID.toLatin1().data());
    if (lResult!=0)
        goto gtl_set_prod_info_failed;
    GSLOG(SYSLOG_SEV_DEBUG, QString("FT PAT Simulator: setting GTL_KEY_PRODUCT_ID...").toLatin1().constData());
    lResult=gtl_set((char*)GTL_KEY_PRODUCT_ID, mMIRRecord.m_cnPART_TYP.toLatin1().data());
    if (lResult!=0)
        goto gtl_set_prod_info_failed;

    //    PopGtlMessageStack();

    if (lResult != GTL_CORE_ERR_OKAY)
    {
        gtl_set_prod_info_failed:
        mErrorMessage = QString("gtl set prod info failed (return code %1), program will be unloaded.")
                        .arg(lResult);
        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
        return false;
    }

    QString         lTestJobFile    = mMIRRecord.m_cnJOB_NAM + ".load";
    QString         lSTDFFilePath   = QFileInfo(mInputSTDFFile).absolutePath();
    CGSystemInfo    lSystemInfo;

    GSLOG(SYSLOG_SEV_DEBUG, QString("FT PAT Simulator: setting GTL_KEY_STATION_NUMBER...").toLatin1().constData());
    gtl_set((char*)GTL_KEY_STATION_NUMBER, (char*)"0");
    //gtl_set(GTL_KEY_HOSTID, (char*) lSystemInfo.m_strHostID.c_str());
    GSLOG(SYSLOG_SEV_DEBUG, QString("FT PAT Simulator: setting GTL_KEY_TESTER_NAME...").toLatin1().constData());
    gtl_set((char*)GTL_KEY_TESTER_NAME, (char*) lSystemInfo.m_strHostName.c_str() );
    GSLOG(SYSLOG_SEV_DEBUG, QString("FT PAT Simulator: setting GTL_KEY_USER_NAME...").toLatin1().constData());
    gtl_set((char*)GTL_KEY_USER_NAME, (char*) lSystemInfo.m_strAccountName.c_str() );
    GSLOG(SYSLOG_SEV_DEBUG, QString("FT PAT Simulator: setting GTL_KEY_TESTER_TYPE...").toLatin1().constData());
    gtl_set((char*)GTL_KEY_TESTER_TYPE, (char*)"GTS");
    GSLOG(SYSLOG_SEV_DEBUG, QString("FT PAT Simulator: setting GTL_KEY_TESTER_EXEC_TYPE...").toLatin1().constData());
    gtl_set((char*)GTL_KEY_TESTER_EXEC_TYPE, (char*) "Quantix Tester Simulator");
    GSLOG(SYSLOG_SEV_DEBUG, QString("FT PAT Simulator: setting GTL_KEY_JOB_NAME...").toLatin1().constData());
    gtl_set((char*)GTL_KEY_JOB_NAME, mMIRRecord.m_cnJOB_NAM.toLatin1().data());
    //gtl_set(GTL_KEY_JOB_FILE, lTestJobFile.toLatin1().data());

    if (lResult != GTL_CORE_ERR_OKAY)
    {
        mErrorMessage = QString("gtl_set_node_info failed (return code %1), program will be unloaded.")
                        .arg(lResult);
        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().data());
        return false;
    }

    // Check if traceability file already exist, if so remove it.
    char lTempFolder[2056]="";

    GSLOG(SYSLOG_SEV_DEBUG, QString("FT PAT Simulator: getting GTL_KEY_TEMP_FOLDER...").toLatin1().constData());
    lResult = gtl_get(GTL_KEY_TEMP_FOLDER, lTempFolder);
    if (lResult != GTL_CORE_ERR_OKAY)
    {
        mErrorMessage = QString("gtl is unable to retrieve the temp folder (return code %1), program will be unloaded.")
                        .arg(lResult);
        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().data());
        return false;
    }

    QString lPreviousDB = QString("%1%2gtl.sqlite").arg(lTempFolder)
                          .arg(lTempFolder[strlen(lTempFolder)-1]=='/'?"":"/");

    if (QFile::exists(lPreviousDB) == true)
        QFile::remove(lPreviousDB);

    GSLOG(SYSLOG_SEV_DEBUG, QString("FT PAT Simulator: setting GTL_KEY_OUTPUT_FOLDER...").toLatin1().constData());
    lResult = gtl_set(GTL_KEY_OUTPUT_FOLDER, QFileInfo(mOutputSTDFFile).absolutePath().toLatin1().data());
    if (lResult != GTL_CORE_ERR_OKAY)
    {
        GSLOG(SYSLOG_SEV_ERROR,
            QString("Bad gtl output_folder '%1'").arg(QFileInfo(mOutputSTDFFile).absolutePath()).toLatin1().data() );
        mErrorMessage = QString("gtl_set output_folder failed (return code %1), program will be unloaded.")
                        .arg(lResult);
        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().data());
        return false;
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("FT PAT Simulator: setting GTL_KEY_OUTPUT_FILENAME...").toLatin1().constData());
    lResult = gtl_set(GTL_KEY_OUTPUT_FILENAME, QFileInfo(mTraceabilityFile).fileName().toLatin1().data());
    if (lResult != GTL_CORE_ERR_OKAY)
    {
        GSLOG(SYSLOG_SEV_ERROR,
          QString("Bad output_filename %1").arg(QFileInfo(mTraceabilityFile).fileName()).toLatin1().data() );
        mErrorMessage = QString("gtl_set output_filename failed (return code %1), program will be unloaded.")
                        .arg(lResult);
        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().data());
        return false;
    }

//    lResult = gtl_set(GTL_KEY_TRACEABILITY_MODE, (char*)"off");
//    if (lResult != GTL_CORE_ERR_OKAY)
//    {
//        mErrorMessage = QString("gtl_set traceability_mode failed (return code %1), program will be unloaded.");
//        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage);
//        return false;
//    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("FT PAT Simulator: setting gtm_communication_mode...").toLatin1().constData());
    lResult = gtl_set("gtm_communication_mode", (char*)"synchronous");
    if (lResult != GTL_CORE_ERR_OKAY)
    {
        mErrorMessage = QString("gtl_set gtm_communication_mode failed (return code %1), program will be unloaded.")
                        .arg(lResult);
        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
        return false;
    }

    //int lSiteNumbers[256];
    //int lSiteIdx = 0;

    QString lSiteNumbersString;
    foreach(int lSite, mSites.keys())
    {
        //lSiteNumbers[lSiteIdx++] = lSite;
        lSiteNumbersString+=QString::number(lSite) + " ";
    }

    // gcore-1180
    //lResult = gtl_init(mTesterConf.toLatin1().data(), mRecipeFile.toLatin1().data(),
      //                 mSites.count(), lSiteNumbers, 1024*1024);
    GSLOG(SYSLOG_SEV_DEBUG, QString("FT PAT Simulator: setting GTL_KEY_CONFIG_FILE...").toLatin1().constData());
    lResult=gtl_set((char*)GTL_KEY_CONFIG_FILE, mTesterConf.toLatin1().data() ); // needed for gtl_open
    if (lResult!=GTL_CORE_ERR_OKAY)
        goto gtl_error;
    GSLOG(SYSLOG_SEV_DEBUG, QString("FT PAT Simulator: setting GTL_KEY_RECIPE_FILE...").toLatin1().constData());
    lResult=gtl_set((char*)GTL_KEY_RECIPE_FILE, mRecipeFile.toLatin1().data()); // csv recipe is deprecated
    if (lResult!=GTL_CORE_ERR_OKAY)
        goto gtl_error;
    GSLOG(SYSLOG_SEV_DEBUG, QString("FT PAT Simulator: setting GTL_KEY_MAX_NUMBER_OF_ACTIVE_SITES...").toLatin1().constData());
    lResult=gtl_set((char*)GTL_KEY_MAX_NUMBER_OF_ACTIVE_SITES, QString::number(mSites.count()).toLatin1().data() );
    if (lResult!=GTL_CORE_ERR_OKAY)
        goto gtl_error;
    GSLOG(SYSLOG_SEV_DEBUG, QString("FT PAT Simulator: setting GTL_KEY_SITES_NUMBERS...").toLatin1().constData());
    lResult=gtl_set((char*)GTL_KEY_SITES_NUMBERS, lSiteNumbersString.toLatin1().data() );
    if (lResult!=GTL_CORE_ERR_OKAY)
        goto gtl_error;
    GSLOG(SYSLOG_SEV_DEBUG, QString("FT PAT Simulator: setting GTL_KEY_MAX_MESSAGES_STACK_SIZE...").toLatin1().constData());
    lResult=gtl_set((char*)GTL_KEY_MAX_MESSAGES_STACK_SIZE, (char*)"900064");
    if (lResult!=GTL_CORE_ERR_OKAY)
        goto gtl_error;

    GSLOG(SYSLOG_SEV_DEBUG, QString("FT PAT Simulator: command GTL_COMMAND_OPEN...").toLatin1().constData());
    lResult=gtl_command((char*)GTL_COMMAND_OPEN);

    // Check status
    if(lResult != GTL_CORE_ERR_OKAY)
    {
        gtl_error:

        if (lResult == GTL_CORE_ERR_GTL_NOLIC)
        {
            mErrorMessage = "No license available for this feature.";
        }
        else
        {
            mErrorMessage = QString("gtl_init failed (return code %1), program will be unloaded.")
                            .arg(lResult);
        }

        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());

        CloseTestProgram();

        return false;
    }

    return true;
}

bool PATGtsStation::ProcessPRR(bool& lEOR)
{
    GQTL_STDF::Stdf_PRR_V4	lPRRRecord;
    int                     lNewSoftBin, lNewHardBin;

    // Read record from STDF parser
    mSTDFReader.ReadRecord((GQTL_STDF::Stdf_Record *)&lPRRRecord);

    unsigned short siteNumber =
        GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
        ( mSTDFReader, lPRRRecord );

    // Call GTL Binning function
    int lResult = gtl_binning(siteNumber, lPRRRecord.m_u2HARD_BIN, lPRRRecord.m_u2SOFT_BIN,
                              &lNewHardBin, &lNewSoftBin, lPRRRecord.m_cnPART_ID.toLatin1().constData());
//    PopGtlMessageStack();
    if (lResult != GTL_CORE_ERR_OKAY)
    {
        mErrorMessage = QString("gtl_binning failed (return code %1).").arg(lResult);
        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
        return false;
    }

    // If valid PRR, update site results (ie Eagle testers use first PIR/PRR for
    // static test definition (Test Plan Delimiter). First PRR has HBin=65535.
    if(lPRRRecord.m_u2HARD_BIN != 65535)
    {
        ++mPartRead;
        mSites[siteNumber] = eSiteProcessed;

        // Check if binning modified by GTL (PAT binning)
        if(lNewSoftBin != lPRRRecord.m_u2SOFT_BIN)
        {
            if (mSoftBinning.contains(siteNumber))
            {
                CBinning * lTmpBinning  = NULL;
                CBinning * lBinning     = mSoftBinning[siteNumber];

                if ((lTmpBinning = lBinning->Find(lPRRRecord.m_u2SOFT_BIN)) != NULL)
                    --lTmpBinning->ldTotalCount;

                if ((lTmpBinning = lBinning->Find(lNewSoftBin)) != NULL)
                    ++lTmpBinning->ldTotalCount;
                else
                    lBinning->Append(lNewSoftBin, 'F', 1, "DPAT Outlier");
            }

            if (mSoftBinning.contains(255))
            {
                CBinning * lTmpBinning  = NULL;
                CBinning * lBinning     = mSoftBinning[255];

                if ((lTmpBinning = lBinning->Find(lPRRRecord.m_u2SOFT_BIN)) != NULL)
                    --lTmpBinning->ldTotalCount;

                if ((lTmpBinning = lBinning->Find(lNewSoftBin)) != NULL)
                    ++lTmpBinning->ldTotalCount;
                else
                    lBinning->Append(lNewSoftBin, 'F', 1, "DPAT Outlier");
            }

            if (mHardBinning.contains(siteNumber))
            {
                CBinning * lTmpBinning  = NULL;
                CBinning * lBinning     = mHardBinning[siteNumber];

                if ((lTmpBinning = lBinning->Find(lPRRRecord.m_u2SOFT_BIN)) != NULL)
                    --lTmpBinning->ldTotalCount;

                if ((lTmpBinning = lBinning->Find(lNewHardBin)) != NULL)
                    ++lTmpBinning->ldTotalCount;
                else
                    lBinning->Append(lNewHardBin, 'F', 1, "DPAT Outlier");
            }

            if (mHardBinning.contains(255))
            {
                CBinning * lTmpBinning  = NULL;
                CBinning * lBinning     = mHardBinning[255];

                if ((lTmpBinning = lBinning->Find(lPRRRecord.m_u2SOFT_BIN)) != NULL)
                    --lTmpBinning->ldTotalCount;

                if ((lTmpBinning = lBinning->Find(lNewHardBin)) != NULL)
                    ++lTmpBinning->ldTotalCount;
                else
                    lBinning->Append(lNewHardBin, 'F', 1, "DPAT Outlier");
            }

            lPRRRecord.SetSOFT_BIN(lNewSoftBin);
            lPRRRecord.SetHARD_BIN(lNewHardBin);
            lPRRRecord.SetPART_FLG(0x08);
        }
    }
    else
        // Invalid run, consider site is not running
        mSites[siteNumber] = eSiteIdle;


    lEOR = true;
    foreach(int lSiteID, mSites.keys())
        lEOR &= (mSites.value(lSiteID) == eSiteProcessed);

    // Write PRR
    if (mGenerateOutputTestData)
        mSTDFWriter.WriteRecord((GQTL_STDF::Stdf_Record *)(&lPRRRecord));

    return true;
}

bool PATGtsStation::ProcessPTR()
{
    GQTL_STDF::Stdf_PTR_V4     lPTRRecord;
    QString						strTestName;
    int							nStatus;

    // Read record from STDF parser
    mSTDFReader.ReadRecord((GQTL_STDF::Stdf_Record *)&lPTRRecord);

    unsigned short siteNumber =
        GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
        ( mSTDFReader, lPTRRecord );

    // If test name valid, normalize it
    if(lPTRRecord.IsFieldValid(GQTL_STDF::Stdf_PTR_V4::eposTEST_TXT))
    {
        strTestName = lPTRRecord.m_cnTEST_TXT.trimmed();
        nStatus = strTestName.indexOf(" <>");
        if(nStatus >= 0)
            strTestName.truncate(nStatus);
        lPTRRecord.SetTEST_TXT(strTestName);
    }

    // Call GTL test result function
    if(lPTRRecord.IsFieldValid(GQTL_STDF::Stdf_PTR_V4::eposRESULT))
    {
        int lResult = gtl_test(siteNumber, lPTRRecord.m_u4TEST_NUM,
                               lPTRRecord.m_cnTEST_TXT.toLatin1().data(), lPTRRecord.m_r4RESULT);
        //      PopGtlMessageStack();
        if (lResult != GTL_CORE_ERR_OKAY &&
            lResult != GTL_CORE_ERR_INVALID_TEST)
        {
            mErrorMessage = QString("gtl_test failed for site %1, test %2/%3 (return code %4).")
                            .arg(siteNumber).arg(lPTRRecord.m_u4TEST_NUM)
                            .arg(lPTRRecord.m_cnTEST_TXT).arg(lResult);
          GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
          return false;
        }
    }

    return true;
}

bool PATGtsStation::WriteBinRecords()
{
    foreach(int lSite, mSoftBinning.keys())
    {
        CBinning * lBinning = mSoftBinning[lSite];

        while (lBinning)
        {
            GQTL_STDF::Stdf_SBR_V4		lRecordSBR;

            lRecordSBR.SetHEAD_NUM(0);
            lRecordSBR.SetSITE_NUM(lSite);
            lRecordSBR.SetSBIN_NUM(lBinning->iBinValue);
            lRecordSBR.SetSBIN_NAM(lBinning->strBinName);
            lRecordSBR.SetSBIN_PF(lBinning->cPassFail);
            lRecordSBR.SetSBIN_CNT(lBinning->ldTotalCount);

            if (mSTDFWriter.WriteRecord(&lRecordSBR) == false)
            {
                mErrorMessage = QString("Failed to write SBR for soft bin# %1 on site %2")
                                .arg(lBinning->iBinValue).arg(lSite);
                GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
                return false;
            }

            lBinning = lBinning->ptNextBin;
        }
    }

    foreach(int lSite, mHardBinning.keys())
    {
        CBinning * lBinning = mHardBinning[lSite];

        while (lBinning)
        {
            GQTL_STDF::Stdf_HBR_V4		lRecordHBR;

            lRecordHBR.SetHEAD_NUM(0);
            lRecordHBR.SetSITE_NUM(lSite);
            lRecordHBR.SetHBIN_NUM(lBinning->iBinValue);
            lRecordHBR.SetHBIN_NAM(lBinning->strBinName);
            lRecordHBR.SetHBIN_PF(lBinning->cPassFail);
            lRecordHBR.SetHBIN_CNT(lBinning->ldTotalCount);

            if (mSTDFWriter.WriteRecord(&lRecordHBR) == false)
            {
                mErrorMessage = QString("Failed to write HBR for hard bin# %1 on site %2")
                                .arg(lBinning->iBinValue).arg(lSite);
                GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
                return false;
            }

            lBinning = lBinning->ptNextBin;
        }
    }

    return true;
}

bool PATGtsStation::ProcessFile()
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("FT PAT Simulator: processing STDF file...").toLatin1().constData());

    bool    lPartDetected       = false;
    bool    lUseLastPIR         = false;
    bool    lEOR                = false;
    bool    lDuplicateRecord    = true;
    int     lRecordType;
    int     lStatus;

    if (mSTDFReader.Open(mInputSTDFFile.toLatin1().constData()) == false)
    {
        mErrorMessage = QString("Failed to open input file %1").arg(mInputSTDFFile);
        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
        return false;
    }

    if (mOutputSTDFFile.isEmpty() == false)
    {
        int lCPUType;
        mSTDFReader.GetCpuType(&lCPUType);

        if (mSTDFWriter.Open(mOutputSTDFFile.toLatin1().constData(), STDF_WRITE, lCPUType) == false)
        {
            mErrorMessage = QString("Failed to open output file %1").arg(mOutputSTDFFile);
            GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
            return false;
        }

        mGenerateOutputTestData = true;
    }
    else
        mGenerateOutputTestData = false;

    // Retrieve the STDF file size
    QFileInfo   lFileInfo(mInputSTDFFile);
    qint64      lFileSize = lFileInfo.size();

    // Update progress status
    emit dataReadProgress("Reading Data", 0, lFileSize);

    lStatus = mSTDFReader.LoadNextRecord(&lRecordType);

    while ((lStatus == GQTL_STDF::StdfParse::NoError) ||
           (lStatus == GQTL_STDF::StdfParse::UnexpectedRecord))
    {
        lDuplicateRecord    = true;
        lEOR                = false;

        if (lUseLastPIR)
        {
            lUseLastPIR = false;

            unsigned short siteNumber =
                GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
                ( mSTDFReader, mLastPIRRecord );

            mSites[siteNumber] = eSiteRunning;

            // Call GTL beginjob function
            int lResult = gtl_beginjob();

            if (lResult != GTL_CORE_ERR_OKAY)
            {
                mErrorMessage = QString("gtl_beginjob failed (return code %1).").arg(lResult);
                GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
                return false;
            }

            lPartDetected = true;
        }

        switch(lRecordType)
        {
            case GQTL_STDF::Stdf_Record::Rec_PIR:
                if (ProcessPIR(lEOR, lUseLastPIR, lPartDetected) == false)
                    return false;
                break;

            case GQTL_STDF::Stdf_Record::Rec_PRR:
                if (ProcessPRR(lEOR) == false)
                    return false;
                lDuplicateRecord = false;
                break;

            case GQTL_STDF::Stdf_Record::Rec_PTR:
                if (ProcessPTR() == false)
                    return false;
                break;

            case GQTL_STDF::Stdf_Record::Rec_MPR:
                if (ProcessMPR() == false)
                    return false;
                break;

            case GQTL_STDF::Stdf_Record::Rec_HBR:
            case GQTL_STDF::Stdf_Record::Rec_SBR:
                if (mBinRecordsCreated == false)
                {
                    if (WriteBinRecords() == false)
                        return false;

                    mBinRecordsCreated = true;
                }

                lDuplicateRecord = false;
                break;

        case GQTL_STDF::Stdf_Record::Rec_GDR:
        {
            // set deciphering mode for site number if needed
            GQTL_STDF::Stdf_GDR_V4 gdr;
            mSTDFReader.ReadRecord( &gdr );

            GQTL_STDF::HeadAndSiteNumberDecipher::SetDecipheringModeInParser
                ( gdr, mSTDFReader );

            break;
        }

            default:
                break;
        }

        // Write record to output file
        if(lDuplicateRecord && mGenerateOutputTestData)
            mSTDFReader.DumpRecord(&mSTDFWriter);

        if (lEOR)
        {
            if (ProcessEOR() == false)
                return false;

            lPartDetected = false;
        }

        lStatus = mSTDFReader.LoadNextRecord(&lRecordType);

        // Update progress status
        emit dataReadProgress("Reading Data", mSTDFReader.GetFilePos(), lFileSize);
    }

    mSTDFReader.Close();

    if (mGenerateOutputTestData)
        mSTDFWriter.Close();

    // Update progress status
    emit dataReadProgress("Reading Data", lFileSize, lFileSize);

    return true;
}

bool PATGtsStation::ProcessEOR()
{
    // Call GTL endjob function
    // gcore-1180
    //int lResult = gtl_endjob();
    int lResult = gtl_command((char*)GTL_COMMAND_ENDJOB);

    //    PopGtlMessageStack();
    if (lResult != GTL_CORE_ERR_OKAY)
    {
        mErrorMessage = QString("gtl_endjob failed (return code %1).").arg(lResult);
        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
        return false;
    }

    // Update part count and binning
//    UpdateSummary();

    // Reset sites
    foreach(int lSiteID, mSites.keys())
        mSites[lSiteID] = eSiteIdle;

    return true;
}

bool PATGtsStation::ProcessMPR()
{
    GQTL_STDF::Stdf_MPR_V4     lMPRRecord;
    QString						strTestName;
    int							nStatus;

    // Read record from STDF parser
    mSTDFReader.ReadRecord((GQTL_STDF::Stdf_Record *)&lMPRRecord);

    unsigned short siteNumber =
        GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
        ( mSTDFReader, lMPRRecord );

    // If test name valid, normalize it
    if(lMPRRecord.IsFieldValid(GQTL_STDF::Stdf_PTR_V4::eposTEST_TXT))
    {
        strTestName = lMPRRecord.m_cnTEST_TXT.trimmed();
        nStatus = strTestName.indexOf(" <>");
        if(nStatus >= 0)
            strTestName.truncate(nStatus);
        lMPRRecord.SetTEST_TXT(strTestName);
    }

    // Call GTL test result function
    if(lMPRRecord.IsFieldValid(GQTL_STDF::Stdf_PTR_V4::eposRESULT))
    {
        QSharedPointer<int>     lPinIndexes;
        QSharedPointer<double>  lPinResults;
        int                     lResult     = 0;

        try
        {
            // Prepare pin indexes array
            lPinIndexes = QSharedPointer<int>(new int[lMPRRecord.m_u2RSLT_CNT],
                                              SharedPointerArrayDeleter<int>);

            if(lPinIndexes.isNull())
            {
                mErrorMessage = QString("Could not allocate Pin Indexes array for %1 integers.")
                                .arg(lMPRRecord.m_u2RSLT_CNT);

                GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
                return false;
            }

            lPinResults = QSharedPointer<double>(new double[lMPRRecord.m_u2RSLT_CNT],
                                                 SharedPointerArrayDeleter<double>);

            if(lPinResults.isNull())
            {
                mErrorMessage = QString("Could not allocate Pin Results array for %1 doubles.")
                                .arg(lMPRRecord.m_u2RSLT_CNT);

                GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
                return false;
            }

            for(int lIndex = 0; lIndex < lMPRRecord.m_u2RSLT_CNT; ++lIndex)
                lPinIndexes.data()[lIndex] = lIndex;

            for(int lIndex = 0; lIndex < lMPRRecord.m_u2RSLT_CNT; ++lIndex)
                lPinResults.data()[lIndex] = lMPRRecord.m_kr4RTN_RSLT[lIndex];

            lResult = gtl_mptest(siteNumber, lMPRRecord.m_u4TEST_NUM,
                                     lMPRRecord.m_cnTEST_TXT.toLatin1().data(), lPinResults.data(),
                                     lPinIndexes.data(),lMPRRecord.m_u2RSLT_CNT);

            if (lResult != GTL_CORE_ERR_OKAY &&
                lResult != GTL_CORE_ERR_INVALID_TEST &&
                lResult != GTL_CORE_ERR_INVALID_PIN)
            {
                mErrorMessage = QString("gtl_mptest failed for site %1, test %2/%3 (return code %4).")
                                .arg(siteNumber).arg(lMPRRecord.m_u4TEST_NUM)
                                .arg(lMPRRecord.m_cnTEST_TXT).arg(lResult);

                GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());

              return false;
            }

        }
        catch(const std::bad_alloc& )
        {
            mErrorMessage = "Memory allocation exception caught during MPR processing";
            GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
            return false;
        }
        catch(...)
        {
            mErrorMessage = "Exception caught during MPR processing";
            GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());

            return false;
        }
    }

    return true;
}

bool PATGtsStation::ProcessPIR(bool &lEOR, bool &lUseLastPIR, bool &lPartDetected)
{
    // PIR record
    mSTDFReader.ReadRecord((GQTL_STDF::Stdf_Record *)&mLastPIRRecord);

    unsigned short siteNumber =
        GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
        ( mSTDFReader, mLastPIRRecord );

    if(mSites.value(siteNumber) == eSiteProcessed)
    {
        lUseLastPIR = true;
        lEOR        = true;
    }
    else
        mSites[siteNumber] = eSiteRunning;

    if(lPartDetected == false)
    {
        lPartDetected = true;

        // Call GTL beginjob function
        int lResult = gtl_beginjob();

//                    PopGtlMessageStack();

        if (lResult != GTL_CORE_ERR_OKAY)
        {
            mErrorMessage = QString("gtl_beginjob failed (return code %1).").arg(lResult);
            GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
            return false;
        }
    }

    return true;
}


}
}
