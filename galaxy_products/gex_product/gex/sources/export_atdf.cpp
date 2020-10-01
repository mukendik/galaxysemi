//////////////////////////////////////////////////////////////////////
// export_atdf.cpp: Converts a STDF file to ATDF

#include <qprogressbar.h>
#include <qdir.h>
#include <gqtl_archivefile.h>
#include <gqtl_sysutils.h>
#include <gqtl_log.h>

#include "engine.h"
#include "admin_engine.h"
#include "export_atdf.h"
#include "gex_shared.h"
#include "gex_constants.h"
#include "product_info.h"

// Output File format: ATDF

// Error map
GBEGIN_ERROR_MAP(CSTDFtoATDF)
    GMAP_ERROR(eOpenReadFail,"Failed to open STDF file:\n%s.")
    GMAP_ERROR(eOpenWriteFail,"Failed to create ATDF file:\n%s.")
    GMAP_ERROR(eLicenceExpired,QString("License has expired or Data file\n%s out of date...\nPlease contact  %1").arg(GEX_EMAIL_SALES).toLatin1().constData())
    GMAP_ERROR(eStdfRead, "STDF read error.")
GEND_ERROR_MAP(CSTDFtoATDF)

#define RECORD_ERROR "** ERROR: unexpected end of record!!\n"

CSTDFtoATDF::CSTDFtoATDF(bool bEvaluationMode)
    : QObject(0), m_clTNFilteringList(NULL)
{
    // Init progress bar ptr
    m_pProgressBar = NULL;

    // Init table of records to process
    for(unsigned int i=0; i<GQTL_STDF::Stdf_Record::Rec_COUNT; i++)
        m_bRecordsToProcess[i] = false;

    // Init other options
    m_nFileSizeLimit = 0;
    m_bDumpFirstOnly = false;
    m_bWriteHeaderFooter = false;

    // Evaluation mode ??
    m_bEvaluationMode = bEvaluationMode;
}

CSTDFtoATDF::CSTDFtoATDF():QObject(0), m_clTNFilteringList(NULL)
{
    // Init table of records to process
    for(unsigned int i=0; i<GQTL_STDF::Stdf_Record::Rec_COUNT; i++)
        m_bRecordsToProcess[i] = false;

    // Init other options
    m_nFileSizeLimit = 0;
    m_bDumpFirstOnly = false;
    m_bWriteHeaderFooter = false;
    m_bEvaluationMode = false;
    m_pProgressBar = NULL;

}

CSTDFtoATDF::CSTDFtoATDF(const CSTDFtoATDF& o) : QObject(0), m_clTNFilteringList(o.m_clTNFilteringList)
{
    m_bEvaluationMode = o.m_bEvaluationMode;
    m_pProgressBar=o.m_pProgressBar;
    // Init table of records to process
    for(unsigned int i=0; i<GQTL_STDF::Stdf_Record::Rec_COUNT; i++)
        m_bRecordsToProcess[i] = false;
    m_nFileSizeLimit=o.m_nFileSizeLimit;
    m_bDumpFirstOnly=o.m_bDumpFirstOnly;
    m_bWriteHeaderFooter = o.m_bWriteHeaderFooter;
}

CSTDFtoATDF::~CSTDFtoATDF()
{
}

void CSTDFtoATDF::Clear()
{
    // Reset consecutive record information
    m_nRecordType_Last = -1;
    m_uiRecordCount_Consecutive = 0;

    // Reset Lot Summary information
    m_uiWIRCount = 0;
    m_uiPartsInWafer = 0;
    m_strWaferIDFromWIR = "";
    m_bWaferOpened= false;

    // Reset record counters
    m_nStdfRecordsProcessed = 0;
    m_nStdtTotalRecords = 0;
    for(unsigned int i=0; i<GQTL_STDF::Stdf_Record::Rec_COUNT; i++)
        m_uiRecordCount[i] = 0;

    // Reset progress
    ResetProgress();
}

void CSTDFtoATDF::SetProcessRecord(const int nRecordType)
{
    if(nRecordType < GQTL_STDF::Stdf_Record::Rec_COUNT)
        m_bRecordsToProcess[nRecordType] = true;
}

void CSTDFtoATDF::SetProcessRecord(bool bProcessRecords)
{
    for(int nRecordType = 0; nRecordType < GQTL_STDF::Stdf_Record::Rec_COUNT; nRecordType++)
        m_bRecordsToProcess[nRecordType] = bProcessRecords;
}

//////////////////////////////////////////////////////////////////////
// Write Dump status
//////////////////////////////////////////////////////////////////////
void CSTDFtoATDF::WriteDumpStatus(QTextStream & hStream, int nStatus)
{
    if(m_bWriteHeaderFooter)
    {
        hStream  << "<!--STDF dump status!-->" << endl;
        if(nStatus == GQTL_STDF::StdfParse::FileCorrupted)
            hStream  << "** Status          : Unexpected end of file!!" << endl;
        else
            hStream  << "** Status          : SUCCESS!!" << endl;
        hStream  << "** UNKNOWN records : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_UNKNOWN]) << endl;
        hStream  << "<!--STDF dump status!-->";
        hStream  << endl;
        hStream  << endl;
    }
}

void CSTDFtoATDF::GetLastError(QString & strError)
{
    strError = GGET_LASTERRORMSG(CSTDFtoATDF,this);
}

QString CSTDFtoATDF::Convert(const QString & strFileNameSTDF, const QString & strAtdfFileName)
{
    if (GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_EVALUATION)
        return "error: feature not available in evaluation mode";

    QString lATDF=strAtdfFileName;
    bool lR=Convert(strFileNameSTDF, lATDF, NULL);
    if (!lR)
        return "error: convert failed. Check logs for root cause.";
    return "ok";
}

bool CSTDFtoATDF::Convert(const QString & strFileNameSTDF, QString & strAtdfFileName, QProgressBar* pProgressBar /*= NULL*/)
{
    bool bStopDump=false;
    QDir cDir;

    //QString lAtdfFileName=strAtdfFileName;
    // Build destination file name?
    if(strAtdfFileName.isEmpty())
        strAtdfFileName = strFileNameSTDF + ".atd";

    // Remove destination file
    cDir.remove(strAtdfFileName);

    // Set progress bar ptr
    m_pProgressBar = pProgressBar;

    // Set STDF file name
    m_strStdfFile = strFileNameSTDF;

    // Open ATDF files
    if(OpenAtdfFiles(strAtdfFileName) == false)
    {
        GSLOG(4, QString("Cannot open ATDF file '%1'").arg(strAtdfFileName).toLatin1().data() );
        // Error. Can't create ATDF file!
        GS::Gex::Engine::GetInstance().GetAdminEngine().Log("ERROR","Convert: terminated in error", QString("Cannot open ATDF file '%1'").arg(strAtdfFileName));
        return false;
    }

    // Read all file...and build ATDF file as we go...
    int	iStatus=0, nRecordType=0;

    // Pass 1: count nb of records (for eventual progress bar), and in debug dump STDF structure
    // Pass 2: create ATDF file
    for(lPass = 1;lPass <=2; lPass++)
    {
        // Clear data between passes
        Clear();

        // Open STDF file to read...
        iStatus = m_clStdfParse.Open(strFileNameSTDF.toLatin1().constData());
        if(iStatus == false)
        {
            // Error. Can't open STDF file in read mode!
            GSET_ERROR1(CSTDFtoATDF, eOpenReadFail, NULL, strFileNameSTDF.toLatin1().constData());
            GSLOG(4, QString("Cannot open STDF '%1'").arg(strFileNameSTDF).toLatin1().data() );
            GS::Gex::Engine::GetInstance().GetAdminEngine().Log("ERROR","Convert: terminated in error", QString("Cannot open STDF '%1'").arg(strFileNameSTDF));
            // Close ATDF files
            CloseAtdfFiles();
            return false;
        }

        // Read one record from STDF file.
        iStatus = m_clStdfParse.LoadNextRecord(&nRecordType);
        while((iStatus == GQTL_STDF::StdfParse::NoError) || (iStatus == GQTL_STDF::StdfParse::UnexpectedRecord))
        {
            m_nStdtTotalRecords++;
            bStopDump = false;

            // Process STDF record read.
            switch(nRecordType)
            {
                case GQTL_STDF::Stdf_Record::Rec_FAR:
                    ProcessRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfFAR, &bStopDump);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_ATR:
                    ProcessRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfATR, &bStopDump);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_MIR:
                    if(ProcessMIR(&bStopDump) != true)
                    {
                        GSLOG(4, QString("Processing MIR failed").toLatin1().data() );
                        GS::Gex::Engine::GetInstance().GetAdminEngine().Log("ERROR","Convert: terminated in error", QString("Processing MIR failed"));
                        // File timestamp is invalid, and higher than expiration date!
                        // Close STDF file
                        m_clStdfParse.Close();
                        // Close ATDF files
                        CloseAtdfFiles();
                        return false;
                    }
                    break;
                case GQTL_STDF::Stdf_Record::Rec_MRR:
                    if(ProcessMRR(&bStopDump) != true)
                    {
                        GSLOG(4, QString("Processing MRR failed").toLatin1().data() );
                        GS::Gex::Engine::GetInstance().GetAdminEngine().Log("ERROR","Convert: terminated in error", QString("Processing MRR failed"));
                        // File timestamp is invalid, and higher than expiration date!
                        // Close STDF file
                        m_clStdfParse.Close();
                        // Close ATDF files
                        CloseAtdfFiles();
                        return false;
                    }
                    break;
                case GQTL_STDF::Stdf_Record::Rec_PCR:
                    ProcessRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfPCR, &bStopDump);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_HBR:
                    ProcessRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfHBR, &bStopDump);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_SBR:
                    ProcessRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfSBR, &bStopDump);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_PMR:
                    ProcessRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfPMR, &bStopDump);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_PGR:
                    ProcessRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfPGR, &bStopDump);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_PLR:
                    ProcessRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfPLR, &bStopDump);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_RDR:
                    ProcessRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfRDR, &bStopDump);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_SDR:
                    ProcessRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfSDR, &bStopDump);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_WIR:
                    if(ProcessWIR(&bStopDump) != true)
                    {
                        GSLOG(4, QString("Processing WIR failed").toLatin1().data() );
                        GS::Gex::Engine::GetInstance().GetAdminEngine().Log("ERROR","Convert: terminated in error", QString("Processing WIR failed"));
                        // File timestamp is invalid, and higher than expiration date!
                        // Close STDF file
                        m_clStdfParse.Close();
                        // Close ATDF files
                        CloseAtdfFiles();
                        return false;
                    }
                    break;
                case GQTL_STDF::Stdf_Record::Rec_WRR:
                    if(ProcessWRR(&bStopDump) != true)
                    {
                        GSLOG(4, QString("Processing WRR failed").toLatin1().data() );
                        GS::Gex::Engine::GetInstance().GetAdminEngine().Log("ERROR","Convert: terminated in error", QString("Processing WRR failed"));
                        // File timestamp is invalid, and higher than expiration date!
                        // Close STDF file
                        m_clStdfParse.Close();
                        // Close ATDF files
                        CloseAtdfFiles();
                        return false;
                    }
                    break;
                case GQTL_STDF::Stdf_Record::Rec_WCR:
                    ProcessRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfWCR, &bStopDump);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_PIR:
                    ProcessRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfPIR, &bStopDump);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_PRR:
                    m_uiPartsInWafer++;
                    ProcessRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfPRR, &bStopDump);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_TSR:
                    ProcessRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfTSR, &bStopDump);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_PTR:
                    ProcessPTR(&bStopDump);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_MPR:
                    ProcessMPR(&bStopDump);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_FTR:
                    ProcessFTR(&bStopDump);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_BPS:
                    ProcessRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfBPS, &bStopDump);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_EPS:
                    ProcessRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfEPS, &bStopDump);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_GDR:
                    ProcessRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfGDR, &bStopDump);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_DTR:
                    ProcessRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfDTR, &bStopDump);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_RESERVED_IMAGE:
                    ProcessRESERVED_IMAGE(&bStopDump);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_RESERVED_IG900:
                    ProcessRESERVED_IG900(&bStopDump);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_UNKNOWN:
                    ProcessUNKNOWN(&bStopDump);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_VUR :
                    ProcessRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfVUR, &bStopDump);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_PSR :
                    ProcessRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfPSR, &bStopDump);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_NMR :
                    ProcessRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfNMR, &bStopDump);
                        break;
                case GQTL_STDF::Stdf_Record::Rec_CNR :
                    ProcessRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfCNR, &bStopDump);
                        break;
                case GQTL_STDF::Stdf_Record::Rec_SSR :
                    ProcessRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfSSR, &bStopDump);
                        break;
                case GQTL_STDF::Stdf_Record::Rec_CDR :
                    ProcessRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfCDR, &bStopDump);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_STR :
                    ProcessRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfSTR, &bStopDump);
                        break;
            }

            // Check if dump should be stopped
            if(bStopDump == true)
            {
                // Force progress bar to completed
                ResetProgress(true);
                // Close STDF file
                m_clStdfParse.Close();
                // Close ATDF files
                CloseAtdfFiles();
                GS::Gex::Engine::GetInstance().GetAdminEngine().Log("INFO","Convert: terminated with no error", "");
                return true;
            }

            // Read one record from STDF file.
            iStatus = m_clStdfParse.LoadNextRecord(&nRecordType);
        };

        // Write dump status
        if(lPass == 1)
            WriteDumpStatus(m_hAtdfStream, iStatus);

        // Save nb of records to process
        m_nStdfRecordsToProcess = m_nStdfRecordsProcessed;

        // Close input STDF file between passes.
        m_clStdfParse.Close();
    }	// 2 passes.

    // Force progress bar to completed
    ResetProgress(true);

    // Convertion successful, close ATDF files.
    CloseAtdfFiles();
    GS::Gex::Engine::GetInstance().GetAdminEngine().Log("INFO","Convert: terminated with no error", "");
    return true;
}

///////////////////////////////////////////////////////////
// Check if consecutive record
///////////////////////////////////////////////////////////
bool CSTDFtoATDF::IsConsecutiveRecord(int nRecordType)
{
    // Check if option to display only first of consecutive records is ON
    if(m_bDumpFirstOnly)
    {
        if(m_nRecordType_Last == -1)
        {
            // First record to be processed
            m_nRecordType_Last = nRecordType;
            m_uiRecordCount_Consecutive = 1;
            return false;
        }

        if(m_nRecordType_Last == nRecordType)
        {
            // Consecutive record
            m_uiRecordCount_Consecutive++;
            return true;
        }

#if 0
        // This record is different from previous
        if((m_uiRecordCount_Consecutive > 1) && (lPass == 2))
            WriteConsecutiveRecordInformation(m_hAsciiStream);
#endif

        m_nRecordType_Last = nRecordType;
        m_uiRecordCount_Consecutive = 1;
    }
    return false;
}

///////////////////////////////////////////////////////////
// Extract record data and dump it
///////////////////////////////////////////////////////////
void CSTDFtoATDF::ProcessRecord(int nRecordType, GQTL_STDF::Stdf_Record* pclRecord, bool* pbStopDump)
{
    // Increment record counter for this record
    (m_uiRecordCount[nRecordType])++;

    // Check if record should be processed
    if(m_bEvaluationMode || m_bRecordsToProcess[nRecordType] == false)
        return;

    // Check if consecutve records
    if(m_bDumpFirstOnly && IsConsecutiveRecord(nRecordType))
        return;

    // Update global record counter
    m_nStdfRecordsProcessed++;

    // Pass#1: update counters, dump if in debug
    if(lPass == 1)
    {
    }

    // Pass#2: write record to ATDF
    if(lPass == 2)
    {
        QString	strAtdfString;
        if(m_clStdfParse.ReadRecord(pclRecord) == false)
        {
            // Error reading STDF file. Dump data read and add an error message.
            pclRecord->GetAtdfString(strAtdfString);
            m_hAtdfStream  << strAtdfString;
            m_hAtdfStream  << RECORD_ERROR;
        }
        else if ( (nRecordType != GQTL_STDF::Stdf_Record::Rec_TSR) ||
                  (m_clTNFilteringList.Contains(m_clStdfTSR.m_u4TEST_NUM) ) )	// test_number filter
        {
            pclRecord->GetAtdfString(strAtdfString);
            m_hAtdfStream  << strAtdfString;
        }

        // Update progress bar
        if(UpdateProgress() == false)
            *pbStopDump = true;
    }
}

///////////////////////////////////////////////////////////
// Extract PTR record data and dump it
///////////////////////////////////////////////////////////
void CSTDFtoATDF::ProcessFTR(bool* pbStopDump)
{
    // Increment record counter for this record
    (m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_FTR])++;

    // Check if record should be processed
    if(m_bEvaluationMode || m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_FTR] == false)
        return;

    // Check if consecutve records
    if(m_bDumpFirstOnly && IsConsecutiveRecord(GQTL_STDF::Stdf_Record::Rec_FTR))
        return;

    // Update global record counter
    m_nStdfRecordsProcessed++;

    // Pass#1: update counters, dump if in debug
    if(lPass == 1)
    {
    }

    // Pass#2: write record to ATDF
    if(lPass == 2)
    {
        QString	strAtdfString;
        if(m_clStdfParse.ReadRecord(&m_clStdfFTR) == false)
        {
            // Error reading STDF file. Dump data read and add an error message.
            m_clStdfFTR.GetAtdfString(strAtdfString);
            m_hAtdfStream  << strAtdfString;
            m_hAtdfStream  << RECORD_ERROR;
        }
        else if (m_clTNFilteringList.Contains(m_clStdfFTR.m_u4TEST_NUM))	// test-number filter
        //if(m_clStdfFTR.m_u4TEST_NUM == 2102)	// DEBUG: dump specific tests only
        {
            m_clStdfFTR.GetAtdfString(strAtdfString);
            m_hAtdfStream  << strAtdfString;
        }

        // Update progress bar
        if(UpdateProgress() == false)
            *pbStopDump = true;
    }
}

///////////////////////////////////////////////////////////
// Extract PTR record data and dump it
///////////////////////////////////////////////////////////
void CSTDFtoATDF::ProcessPTR(bool* pbStopDump)
{
    // Increment record counter for this record
    (m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_PTR])++;

    // Check if record should be processed
    if(m_bEvaluationMode || m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_PTR] == false)
        return;

    // Check if consecutve records
    if(m_bDumpFirstOnly && IsConsecutiveRecord(GQTL_STDF::Stdf_Record::Rec_PTR))
        return;

    // Update global record counter
    m_nStdfRecordsProcessed++;

    // Pass#1: update counters, dump if in debug
    if(lPass == 1)
    {
    }

    // Pass#2: write record to ATDF
    if(lPass == 2)
    {
        QString	strAtdfString;
        if(m_clStdfParse.ReadRecord(&m_clStdfPTR) == false)
        {
            // Error reading STDF file. Dump data read and add an error message.
            m_clStdfPTR.GetAtdfString(strAtdfString);
            m_hAtdfStream  << strAtdfString;
            m_hAtdfStream  << RECORD_ERROR;
        }
        else if (m_clTNFilteringList.Contains(m_clStdfPTR.m_u4TEST_NUM))	// test_number filter
        //if(m_clStdfPTR.m_u4TEST_NUM == 2102)	// DEBUG: dump specific tests only
        {
            m_clStdfPTR.GetAtdfString(strAtdfString);
            m_hAtdfStream  << strAtdfString;
        }

        // Update progress bar
        if(UpdateProgress() == false)
            *pbStopDump = true;
    }
}

///////////////////////////////////////////////////////////
// Extract MPR record data and dump it
///////////////////////////////////////////////////////////
void CSTDFtoATDF::ProcessMPR(bool* pbStopDump)
{
    // Increment record counter for this record
    (m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_MPR])++;

    // Check if record should be processed
    if(m_bEvaluationMode || m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_MPR] == false)
        return;

    // Check if consecutve records
    if(m_bDumpFirstOnly && IsConsecutiveRecord(GQTL_STDF::Stdf_Record::Rec_MPR))
        return;

    // Update global record counter
    m_nStdfRecordsProcessed++;

    // Pass#1: update counters, dump if in debug
    if(lPass == 1)
    {
    }

    // Pass#2: write record to ATDF
    if(lPass == 2)
    {
        QString	strAtdfString;
        if(m_clStdfParse.ReadRecord(&m_clStdfMPR) == false)
        {
            // Error reading STDF file. Dump data read and add an error message.
            m_clStdfMPR.GetAtdfString(strAtdfString);
            m_hAtdfStream  << strAtdfString;
            m_hAtdfStream  << RECORD_ERROR;
        }
        else if (m_clTNFilteringList.Contains(m_clStdfMPR.m_u4TEST_NUM))	// test_number filter
        //if(m_clStdfMPR.m_u4TEST_NUM == 2102)	// DEBUG: dump specific tests only
        {
            m_clStdfMPR.GetAtdfString(strAtdfString);
            m_hAtdfStream  << strAtdfString;
        }

        // Update progress bar
        if(UpdateProgress() == false)
            *pbStopDump = true;
    }
}

/////////////////////////////////////////////////////////////////////////////
// Read STDF.MIR record, dump it to ATDF file...
/////////////////////////////////////////////////////////////////////////////
bool CSTDFtoATDF::ProcessMIR(bool* pbStopDump)	// Extract MIR data
{
    // Increment record counter for this record
    (m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_MIR])++;

    // Update global record counter
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_MIR] == true)
        m_nStdfRecordsProcessed++;

    // Pass#1: check license validity, update counters, dump if in debug
    if(lPass == 1)
    {
        // Check if file date is not more recent than license expiration date!
        if(m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record *)&m_clStdfMIR) == false)
        {
            // Error reading STDF file
            // Convertion failed.
            GSET_ERROR0(CSTDFtoATDF, eStdfRead, GGET_LASTERROR(StdfParse,&m_clStdfParse));
            return false;
        }

        return true;
    }

    // Pass#2: write record to ATDF
    if((lPass == 2) && (m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_MIR] == true))
    {
        QString	strAtdfString;
        if(m_clStdfParse.ReadRecord(&m_clStdfMIR) == false)
        {
            // Error reading STDF file. Dump data read and add an error message.
            m_clStdfMIR.GetAtdfString(strAtdfString);
            m_hAtdfStream  << strAtdfString;
            m_hAtdfStream  << RECORD_ERROR;
        }
        else
        {
            m_clStdfMIR.GetAtdfString(strAtdfString);
            m_hAtdfStream  << strAtdfString;
        }

        // Update progress bar
        if(UpdateProgress() == false)
            *pbStopDump = true;

        return true;
    }

    return true;
}

///////////////////////////////////////////////////////////
// Read STDF.MRR record, dump it to ATDF file...
///////////////////////////////////////////////////////////
bool CSTDFtoATDF::ProcessMRR(bool* pbStopDump)
{
    // Increment record counter for this record
    (m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_MRR])++;

    // Check if evaluation mode
    if(m_bEvaluationMode)
        return true;

    // Update global record counter
    if(m_bEvaluationMode || m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_MRR] == true)
        m_nStdfRecordsProcessed++;

    // Pass#1: update counters, dump if in debug
    if(lPass == 1)
    {
        // Check if file date is not more recent than license expiration date!
        if(m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record *)&m_clStdfMRR) == false)
        {
            // Error reading STDF file
            // Convertion failed.
            GSET_ERROR0(CSTDFtoATDF, eStdfRead, GGET_LASTERROR(StdfParse,&m_clStdfParse));
            return false;
        }

        return true;
    }

    // Pass#2: write record to ATDF
    if((lPass == 2) && (m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_MRR] == true))
    {
        QString	strAtdfString;
        if(m_clStdfParse.ReadRecord(&m_clStdfMRR) == false)
        {
            // Error reading STDF file. Dump data read and add an error message.
            m_clStdfMRR.GetAtdfString(strAtdfString);
            m_hAtdfStream  << strAtdfString;
            m_hAtdfStream  << RECORD_ERROR;
        }
        else
        {
            m_clStdfMRR.GetAtdfString(strAtdfString);
            m_hAtdfStream  << strAtdfString;
        }

        // Update progress bar
        if(UpdateProgress() == false)
            *pbStopDump = true;

        return true;
    }

    return true;
}

///////////////////////////////////////////////////////////
// Read STDF.WIR record, dump it to ATDF file...
///////////////////////////////////////////////////////////
bool CSTDFtoATDF::ProcessWIR(bool* pbStopDump)
{
    // Increment record counter for this record
    (m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_WIR])++;

    // Check if evaluation mode
    if(m_bEvaluationMode)
        return true;

    // Update global record counter
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_WIR] == true)
        m_nStdfRecordsProcessed++;

    // Pass#1: update counters, dump if in debug
    if(lPass == 1)
    {
        // Check if file date is not more recent than license expiration date!
        if(m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record *)&m_clStdfWIR) == false)
        {
            // Error reading STDF file
            // Convertion failed.
            GSET_ERROR0(CSTDFtoATDF, eStdfRead, GGET_LASTERROR(StdfParse,&m_clStdfParse));
            return false;
        }

        // Update Lot/Sublot summary information
        m_uiWIRCount++;
        m_uiPartsInWafer = 0;
        m_strWaferIDFromWIR = m_clStdfWIR.m_cnWAFER_ID;
        m_bWaferOpened= true;

        return true;
    }

    // Pass#2: write record to ATDF
    if((lPass == 2) && (m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_WIR] == true))
    {
        QString	strAtdfString;
        if(m_clStdfParse.ReadRecord(&m_clStdfWIR) == false)
        {
            // Error reading STDF file. Dump data read and add an error message.
            m_clStdfWIR.GetAtdfString(strAtdfString);
            m_hAtdfStream  << strAtdfString;
            m_hAtdfStream  << RECORD_ERROR;
        }
        else
        {
            m_clStdfWIR.GetAtdfString(strAtdfString);
            m_hAtdfStream  << strAtdfString;
        }

        // Update progress bar
        if(UpdateProgress() == false)
            *pbStopDump = true;

        return true;
    }

    return true;
}

///////////////////////////////////////////////////////////
// Read STDF.WRR record, dump it to ATDF file...
///////////////////////////////////////////////////////////
bool CSTDFtoATDF::ProcessWRR(bool* pbStopDump)
{
    // Increment record counter for this record
    (m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_WRR])++;

    // Check if evaluation mode
    if(m_bEvaluationMode)
        return true;

    // Update global record counter
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_WRR] == true)
        m_nStdfRecordsProcessed++;

    // Pass#1: update counters, dump if in debug
    if(lPass == 1)
    {
        // Check if file date is not more recent than license expiration date!
        if(m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record *)&m_clStdfWRR) == false)
        {
            // Error reading STDF file
            // Convertion failed.
            GSET_ERROR0(CSTDFtoATDF, eStdfRead, GGET_LASTERROR(StdfParse,&m_clStdfParse));
            return false;
        }

        // Upate Lot/Sublot Information
        m_bWaferOpened= false;

        return true;
    }

    // Pass#2: write record to ATDF
    if((lPass == 2) && (m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_WRR] == true))
    {
        QString	strAtdfString;
        if(m_clStdfParse.ReadRecord(&m_clStdfWRR) == false)
        {
            // Error reading STDF file. Dump data read and add an error message.
            m_clStdfWRR.GetAtdfString(strAtdfString);
            m_hAtdfStream  << strAtdfString;
            m_hAtdfStream  << RECORD_ERROR;
        }
        else
        {
            m_clStdfWRR.GetAtdfString(strAtdfString);
            m_hAtdfStream  << strAtdfString;
        }

        // Update progress bar
        if(UpdateProgress() == false)
            *pbStopDump = true;

        return true;
    }

    return true;
}

///////////////////////////////////////////////////////////
// Read STDF.RESERVED_IMAGE record, dump it to ATDF file...
///////////////////////////////////////////////////////////
void CSTDFtoATDF::ProcessRESERVED_IMAGE(bool* pbStopDump)
{
    // Increment record counter for this record
    (m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_RESERVED_IMAGE])++;

    // Check if evaluation mode
    if(m_bEvaluationMode)
        return;

    // Update global record counter
    m_nStdfRecordsProcessed++;

    // Pass#1: update counters, dump if in debug
    if(lPass == 1)
    {
    }

    // Pass#2: write record to ATDF
    if(lPass == 2)
    {
        QString	strAtdfString;
        if(m_clStdfParse.ReadRecord(&m_clStdfRESERVED_IMAGE) == false)
        {
            // Error reading STDF file. Dump data read and add an error message.
            m_clStdfRESERVED_IMAGE.GetAtdfString(strAtdfString);
            m_hAtdfStream  << strAtdfString;
            m_hAtdfStream  << RECORD_ERROR;
        }
        else
        {
            m_clStdfRESERVED_IMAGE.GetAtdfString(strAtdfString);
            m_hAtdfStream  << strAtdfString;
        }

        // Update progress bar
        if(UpdateProgress() == false)
            *pbStopDump = true;
    }
}

///////////////////////////////////////////////////////////
// Read STDF.RESERVED_IG900 record, dump it to ATDF file...
///////////////////////////////////////////////////////////
void CSTDFtoATDF::ProcessRESERVED_IG900(bool* pbStopDump)
{
    // Increment record counter for this record
    (m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_RESERVED_IG900])++;

    // Check if evaluation mode
    if(m_bEvaluationMode)
        return;

    // Update global record counter
    m_nStdfRecordsProcessed++;

    // Pass#1: update counters, dump if in debug
    if(lPass == 1)
    {
    }

    // Pass#2: write record to ATDF
    if(lPass == 2)
    {
        QString	strAtdfString;
        if(m_clStdfParse.ReadRecord(&m_clStdfRESERVED_IG900) == false)
        {
            // Error reading STDF file. Dump data read and add an error message.
            m_clStdfRESERVED_IG900.GetAtdfString(strAtdfString);
            m_hAtdfStream  << strAtdfString;
            m_hAtdfStream  << RECORD_ERROR;
        }
        else
        {
            m_clStdfRESERVED_IG900.GetAtdfString(strAtdfString);
            m_hAtdfStream  << strAtdfString;
        }

        // Update progress bar
        if(UpdateProgress() == false)
            *pbStopDump = true;
    }
}

///////////////////////////////////////////////////////////
// Read STDF.UNKNOWN record, dump it to ATDF file...
///////////////////////////////////////////////////////////
void CSTDFtoATDF::ProcessUNKNOWN(bool* pbStopDump)
{
    // Increment record counter for this record
    (m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_UNKNOWN])++;

    // Check if evaluation mode
    if(m_bEvaluationMode || m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_UNKNOWN] == false)
        return;
    // Update global record counter
    m_nStdfRecordsProcessed++;

    // Pass#1: update counters, dump if in debug
    if(lPass == 1)
    {
    }

    // Pass#2: write record to ATDF
    if(lPass == 2)
    {
        QString	strAtdfString;
        if(m_clStdfParse.ReadRecord(&m_clStdfUNKNOWN) == false)
        {
            // Error reading STDF file. Dump data read and add an error message.
            m_clStdfUNKNOWN.GetAtdfString(strAtdfString);
            m_hAtdfStream  << strAtdfString;
            m_hAtdfStream  << RECORD_ERROR;
        }
        else
        {
            m_clStdfUNKNOWN.GetAtdfString(strAtdfString);
            m_hAtdfStream  << strAtdfString;
        }

        // Update progress bar
        if(UpdateProgress() == false)
            *pbStopDump = true;
    }
}

///////////////////////////////////////////////////////////
// Open file for atdf dump
///////////////////////////////////////////////////////////
bool CSTDFtoATDF::OpenAtdfFiles(const QString & strAtdfFileName)
{
    // Open ATDF files
    m_hAtdfFile.setFileName(strAtdfFileName);
    if(!m_hAtdfFile.open(QIODevice::WriteOnly))
    {
        GSET_ERROR1(CSTDFtoATDF, eOpenWriteFail, NULL, strAtdfFileName.toLatin1().constData());
        return false;
    }

    // Assign file I/O stream
    m_hAtdfStream.setDevice(&m_hAtdfFile);

    return true;
}

///////////////////////////////////////////////////////////
// Close atdf dump file
///////////////////////////////////////////////////////////
void CSTDFtoATDF::CloseAtdfFiles(void)
{
    m_hAtdfFile.close();
}

///////////////////////////////////////////////////////////
// Reset progress bar
///////////////////////////////////////////////////////////
void CSTDFtoATDF::ResetProgress(bool bForceCompleted /*= false*/)
{
    if(bForceCompleted == true)
        m_nProgress = 100;
    else
        m_nProgress = 0;

    if(m_pProgressBar != NULL)
        m_pProgressBar->setValue(m_nProgress);
}

///////////////////////////////////////////////////////////
// Update progress bar
///////////////////////////////////////////////////////////
bool CSTDFtoATDF::UpdateProgress(void)
{
    int nProgress;

    if(m_nFileSizeLimit == 0)
    {
        // No file size limit
        if((m_pProgressBar != NULL) && (m_nStdfRecordsToProcess != 0))
        {
            nProgress = m_nStdfRecordsProcessed*100/m_nStdfRecordsToProcess;
            if(nProgress != m_nProgress)
            {
                m_nProgress = nProgress;
                m_pProgressBar->setValue(m_nProgress);
            }
        }
    }
    else
    {
        // A file size limit has been set
        int nCurrentFileSize = m_hAtdfFile.size();
        int nFileSize = nCurrentFileSize < m_nFileSizeLimit ? nCurrentFileSize : m_nFileSizeLimit;
        if(m_pProgressBar != NULL)
        {
            nProgress = nFileSize*100/m_nFileSizeLimit;
            if(nProgress != m_nProgress)
            {
                m_nProgress = nProgress;
                m_pProgressBar->setValue(m_nProgress);
            }
        }
        if(nFileSize == m_nFileSizeLimit)
            return false;
    }

    return true;
}

////////////////////////////////////////////////
// Set Test_Number list
////////////////////////////////////////////////
void CSTDFtoATDF::setTNFilteringList(QString strListe)
{
    m_clTNFilteringList.SetRange(strListe);
}


