//////////////////////////////////////////////////////////////////////
// export_ascii.cpp: Converts a STDF file to ASCII
//////////////////////////////////////////////////////////////////////

#include <qdir.h>
#include <gqtl_archivefile.h>
#include <gqtl_sysutils.h>
#include <QProgressBar>

#include "export_ascii.h"
#include "engine.h"
#include "gex_shared.h"

// Output File format:
// <GEX Header: version, date, ...>
//
// ** <Record Name> **
// REC.rec_len  = 2
// REC.rec_typ  = 0
// REC.rec_sub  = 10
// REC.<field>  = <value>
// ...

// main.cpp

// Error map
GBEGIN_ERROR_MAP(CSTDFtoASCII)
    GMAP_ERROR(eOpenReadFail,"Failed to open STDF file:\n%s.")
    GMAP_ERROR(eOpenWriteFail,"Failed to create ASCII file:\n%s.")
    GMAP_ERROR(eLicenceExpired,QString("License has expired or Data file\n%s out of date...\nPlease contact %1").arg(GEX_EMAIL_SALES).toLatin1().constData())
    GMAP_ERROR(eStdfRead, "STDF read error.")
GEND_ERROR_MAP(CSTDFtoASCII)

#define RECORD_ERROR "** ERROR: unexpected end of record!!\n"

class CSTDFtoASCIIPrivate
{
    public:
    CSTDFtoASCIIPrivate();
    ~CSTDFtoASCIIPrivate();

    //V4 Records
    GQTL_STDF::Stdf_FAR_V4	mStdfFARv4;
    GQTL_STDF::Stdf_ATR_V4	mStdfATRv4;
    GQTL_STDF::Stdf_MIR_V4	mStdfMIRv4;
    GQTL_STDF::Stdf_MRR_V4	mStdfMRRv4;
    GQTL_STDF::Stdf_PCR_V4	mStdfPCRv4;
    GQTL_STDF::Stdf_HBR_V4	mStdfHBRv4;
    GQTL_STDF::Stdf_SBR_V4	mStdfSBRv4;
    GQTL_STDF::Stdf_PMR_V4	mStdfPMRv4;
    GQTL_STDF::Stdf_PGR_V4	mStdfPGRv4;
    GQTL_STDF::Stdf_PLR_V4	mStdfPLRv4;
    GQTL_STDF::Stdf_RDR_V4	mStdfRDRv4;
    GQTL_STDF::Stdf_SDR_V4	mStdfSDRv4;
    GQTL_STDF::Stdf_WIR_V4	mStdfWIRv4;
    GQTL_STDF::Stdf_WRR_V4	mStdfWRRv4;
    GQTL_STDF::Stdf_WCR_V4	mStdfWCRv4;
    GQTL_STDF::Stdf_PIR_V4	mStdfPIRv4;
    GQTL_STDF::Stdf_PRR_V4	mStdfPRRv4;
    GQTL_STDF::Stdf_TSR_V4	mStdfTSRv4;
    GQTL_STDF::Stdf_PTR_V4	mStdfPTRv4;
    GQTL_STDF::Stdf_MPR_V4	mStdfMPRv4;
    GQTL_STDF::Stdf_FTR_V4	mStdfFTRv4;
    GQTL_STDF::Stdf_BPS_V4	mStdfBPSv4;
    GQTL_STDF::Stdf_EPS_V4	mStdfEPSv4;
    GQTL_STDF::Stdf_GDR_V4	mStdfGDRv4;
    GQTL_STDF::Stdf_DTR_V4	mStdfDTRv4;
    GQTL_STDF::Stdf_RESERVED_IMAGE_V4	mStdfRESERVED_IMAGEv4;
    GQTL_STDF::Stdf_RESERVED_IG900_V4	mStdfRESERVED_IG900v4;
    GQTL_STDF::Stdf_UNKNOWN_V4	mStdfUNKNOWNv4;
    GQTL_STDF::Stdf_VUR_V4	mStdfVURv4;
    GQTL_STDF::Stdf_PSR_V4 	mStdfPSRv4;
    GQTL_STDF::Stdf_NMR_V4	mStdfNMRv4;
    GQTL_STDF::Stdf_CNR_V4	mStdfCNRv4;
    GQTL_STDF::Stdf_SSR_V4	mStdfSSRv4;
    GQTL_STDF::Stdf_CDR_V4	mStdfCDRv4;
    GQTL_STDF::Stdf_STR_V4	mStdfSTRv4;

    //V3 Records
    GQTL_STDF::Stdf_FAR_V3 mStdfFARv3;
    GQTL_STDF::Stdf_MIR_V3 mStdfMIRv3;
    GQTL_STDF::Stdf_MRR_V3 mStdfMRRv3;
    GQTL_STDF::Stdf_HBR_V3 mStdfHBRv3;
    GQTL_STDF::Stdf_SBR_V3 mStdfSBRv3;
    GQTL_STDF::Stdf_PMR_V3 mStdfPMRv3;
    GQTL_STDF::Stdf_WIR_V3 mStdfWIRv3;
    GQTL_STDF::Stdf_WRR_V3 mStdfWRRv3;
    GQTL_STDF::Stdf_WCR_V3 mStdfWCRv3;
    GQTL_STDF::Stdf_PIR_V3 mStdfPIRv3;
    GQTL_STDF::Stdf_PRR_V3 mStdfPRRv3;
    GQTL_STDF::Stdf_TSR_V3 mStdfTSRv3;
    GQTL_STDF::Stdf_PTR_V3 mStdfPTRv3;
    GQTL_STDF::Stdf_FTR_V3 mStdfFTRv3;
    GQTL_STDF::Stdf_BPS_V3 mStdfBPSv3;
    GQTL_STDF::Stdf_EPS_V3 mStdfEPSv3;
    GQTL_STDF::Stdf_GDR_V3 mStdfGDRv3;
    GQTL_STDF::Stdf_DTR_V3 mStdfDTRv3;
    GQTL_STDF::Stdf_FDR_V3 mStdfFDRv3;
    GQTL_STDF::Stdf_PDR_V3 mStdfPDRv3;
    GQTL_STDF::Stdf_SCR_V3 mStdfSCRv3;
    GQTL_STDF::Stdf_SHB_V3 mStdfSHBv3;
    GQTL_STDF::Stdf_SSB_V3 mStdfSSBv3;
    GQTL_STDF::Stdf_STS_V3 mStdfSTSv3;

};

CSTDFtoASCIIPrivate::CSTDFtoASCIIPrivate()
{

}

CSTDFtoASCIIPrivate::~CSTDFtoASCIIPrivate()
{

}

CSTDFtoASCII::CSTDFtoASCII(bool bEvaluationMode)
{
    // Init progress bar ptr
    m_pProgressBar = NULL;

    mPrivate = new CSTDFtoASCIIPrivate;

    // Init table of records to process
    for(unsigned int i=0; i<GQTL_STDF::Stdf_Record::Rec_COUNT; i++)
        m_bRecordsToProcess[i] = false;

    // Init other options
    m_nFieldFilter = GQTL_STDF::Stdf_Record::FieldFlag_Present;
    m_nFileSizeLimit = 0;
    m_bDumpFirstOnly = false;
    m_clTNFilteringList.SetRange("");

    // Evaluation mode ??
    m_bEvaluationMode = bEvaluationMode;
}

CSTDFtoASCII::~CSTDFtoASCII()
{
}

void CSTDFtoASCII::Clear()
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

void CSTDFtoASCII::SetProcessRecord(const int nRecordType)
{
    if(nRecordType < GQTL_STDF::Stdf_Record::Rec_COUNT)
        m_bRecordsToProcess[nRecordType] = true;
}

void CSTDFtoASCII::SetProcessRecord(bool bProcessRecords)
{
    for(int nRecordType = 0; nRecordType < GQTL_STDF::Stdf_Record::Rec_COUNT; nRecordType++)
        m_bRecordsToProcess[nRecordType] = bProcessRecords;
}

void CSTDFtoASCII::WriteHeader(QTextStream & hStream)
{
    hStream  << GALAXYSEMI_ASCII_HEADER << endl;
    hStream  << "<!--Report created with: "
      << GS::Gex::Engine::GetInstance().Get("AppFullName").toString()
      << " - www.mentor.com-->" << endl;
    hStream  << endl;
    hStream  << "<!--Report Header!-->";
    hStream  << endl;
    hStream  << "** Input file      : " << m_strStdfFile;
    hStream  << endl;
    hStream  << "** Date            : " << GS::Gex::Engine::GetInstance().GetClientDateTime().toString("ddd dd MMM yyyy h:mm:ss");
    hStream  << endl;
    hStream  << "** Records         :";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_FAR])
        hStream << " FAR";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_ATR])
        hStream << " ATR";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_MIR])
        hStream << " MIR";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_MRR])
        hStream << " MRR";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_PCR])
        hStream << " PCR";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_HBR])
        hStream << " HBR";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_SBR])
        hStream << " SBR";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_PMR])
        hStream << " PMR";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_PGR])
        hStream << " PGR";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_PLR])
        hStream << " PLR";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_RDR])
        hStream << " RDR";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_SDR])
        hStream << " SDR";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_WIR])
        hStream << " WIR";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_WRR])
        hStream << " WRR";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_WCR])
        hStream << " WCR";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_PIR])
        hStream << " PIR";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_PRR])
        hStream << " PRR";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_TSR])
        hStream << " TSR";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_PTR])
        hStream << " PTR";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_MPR])
        hStream << " MPR";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_FTR])
        hStream << " FTR";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_BPS])
        hStream << " BPS";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_EPS])
        hStream << " EPS";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_GDR])
        hStream << " GDR";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_DTR])
        hStream << " DTR";

    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_VUR])
        hStream << " VUR";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_PSR])
        hStream << " PSR";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_NMR])
        hStream << " NMR";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_CNR])
        hStream << " CNR";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_SSR])
        hStream << " SSR";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_SCR])
        hStream << " SCR";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_STR])
        hStream << " STR";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_CDR])
        hStream << " CDR";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_SHB])
        hStream << " SHB";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_SSB])
        hStream << " SSB";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_PDR])
        hStream << " PDR";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_FDR])
        hStream << " FDR";
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_STS])
        hStream << " STS";

    hStream  << endl;
    hStream  << "** Field filter    : " << m_strFieldFilterText;
    hStream  << endl;
    hStream  << "** File size limit : " << m_strFileSizeLimitText;
    hStream  << endl;
    hStream  << "<!--Report Header!-->";
    hStream  << endl;
    hStream  << endl;
}

//////////////////////////////////////////////////////////////////////
// Write Record summary
//////////////////////////////////////////////////////////////////////
void CSTDFtoASCII::WriteRecordSummary(QTextStream & hStream, int stdfVersion)
{
    hStream  << "<!--STDF record summary!-->" << endl;
    if(stdfVersion == STDF_V_4)
    {
        hStream  << "** FAR             : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_FAR]) << endl;
        hStream  << "** ATR             : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_ATR]) << endl;
        hStream  << "** MIR             : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_MIR]) << endl;
        hStream  << "** MRR             : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_MRR]) << endl;
        hStream  << "** PCR             : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_PCR]) << endl;
        hStream  << "** HBR             : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_HBR]) << endl;
        hStream  << "** SBR             : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_SBR]) << endl;
        hStream  << "** PMR             : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_PMR]) << endl;
        hStream  << "** PGR             : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_PGR]) << endl;
        hStream  << "** PLR             : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_PLR]) << endl;
        hStream  << "** RDR             : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_RDR]) << endl;
        hStream  << "** SDR             : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_SDR]) << endl;
        hStream  << "** WIR             : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_WIR]) << endl;
        hStream  << "** WRR             : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_WRR]) << endl;
        hStream  << "** WCR             : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_WCR]) << endl;
        hStream  << "** PIR             : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_PIR]) << endl;
        hStream  << "** PRR             : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_PRR]) << endl;
        hStream  << "** TSR             : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_TSR]) << endl;
        hStream  << "** PTR             : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_PTR]) << endl;
        hStream  << "** MPR             : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_MPR]) << endl;
        hStream  << "** FTR             : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_FTR]) << endl;
        hStream  << "** BPS             : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_BPS]) << endl;
        hStream  << "** EPS             : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_EPS]) << endl;
        hStream  << "** GDR             : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_GDR]) << endl;
        hStream  << "** DTR             : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_DTR]) << endl;
        hStream  << "** RESERVED_IMAGE  : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_RESERVED_IMAGE]) << endl;
        hStream  << "** RESERVED_IG900  : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_RESERVED_IG900]) << endl;
        hStream  << "** UNKNOWN record  : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_UNKNOWN]) << endl;
        hStream  << "** VUR             : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_VUR]) << endl;
        hStream  << "** PSR             : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_PSR]) << endl;
        hStream  << "** NMR             : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_NMR]) << endl;
        hStream  << "** CNR             : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_CNR]) << endl;
        hStream  << "** SSR             : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_SSR]) << endl;
        hStream  << "** CDR             : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_SCR]) << endl;
        hStream  << "** STR             : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_STR]) << endl;
    }
    else if(stdfVersion == STDF_V_3)
    {
        hStream << "**BPS              : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_BPS]) << endl;
        hStream << "**DTR              : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_DTR]) << endl;
        hStream << "**EPS              : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_EPS]) << endl;
        hStream << "**FAR              : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_FAR]) << endl;
        hStream << "**FDR              : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_FDR]) << endl;
        hStream << "**FTR              : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_FTR]) << endl;
        hStream << "**GDR              : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_GDR]) << endl;
        hStream << "**HBR              : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_HBR]) << endl;
        hStream << "**MIR              : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_MIR]) << endl;
        hStream << "**MRR              : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_MRR]) << endl;
        hStream << "**PDR              : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_PDR]) << endl;
        hStream << "**PIR              : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_PIR]) << endl;
        hStream << "**PMR              : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_PMR]) << endl;
        hStream << "**PRR              : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_PRR]) << endl;
        hStream << "**PTR              : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_PTR]) << endl;
        hStream << "**SBR              : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_SBR]) << endl;
        hStream << "**SCR              : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_SCR]) << endl;
        hStream << "**SHB              : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_SHB]) << endl;
        hStream << "**SSB              : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_SSB]) << endl;
        hStream << "**STS              : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_STS]) << endl;
        hStream << "**TSR              : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_TSR]) << endl;
        hStream << "**WCR              : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_WCR]) << endl;
        hStream << "**WIR              : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_WIR]) << endl;
        hStream << "**WRR              : " << QString::number(m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_WRR]) << endl;
    }

    hStream  << endl;
    hStream  << "** TOTAL RECORDS   : " << QString::number(m_nStdtTotalRecords) << endl;
    hStream  << "<!--STDF record summary!-->";
    hStream  << endl;
}

//////////////////////////////////////////////////////////////////////
// Write Dump status
//////////////////////////////////////////////////////////////////////
void CSTDFtoASCII::WriteDumpStatus(QTextStream & hStream, int nStatus)
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

//////////////////////////////////////////////////////////////////////
// Write Lot summary: open section
//////////////////////////////////////////////////////////////////////
void CSTDFtoASCII::WriteLotSummary_Open(QTextStream & hStream)
{
    hStream  << "<!--STDF Lot summary!-->";
}

//////////////////////////////////////////////////////////////////////
// Write Lot summary: Lot/Sublot
//////////////////////////////////////////////////////////////////////
void CSTDFtoASCII::WriteLotSummary_LotSublot(QTextStream & hStream, const QString &lotId, const QString &sbLotId)
{
    QString strLotID = lotId.isEmpty() ? "<empty>" : lotId;
    QString strSublotID = sbLotId.isEmpty() ? "<empty>" : sbLotId;

    hStream  << endl;
    hStream  << "** Lot             : ID = " << strLotID << endl;
    hStream  << "** Sublot          : ID = " << strSublotID;
}

//////////////////////////////////////////////////////////////////////
// Write Lot summary: closed Wafer (WIR/WRR)
//////////////////////////////////////////////////////////////////////
void CSTDFtoASCII::WriteLotSummary_Wafer(QTextStream & hStream, const QString & waferId, unsigned long partCNT)
{
    QString strWaferID_WIR = m_strWaferIDFromWIR.isEmpty() ? "<empty>" : m_strWaferIDFromWIR;
    QString strWaferID_WRR = waferId.isEmpty() ? "<empty>" : waferId;

    hStream  << endl;
    if((waferId.isEmpty()) || (strWaferID_WIR == strWaferID_WRR))
        hStream  << "** Wafer           : ID = " << strWaferID_WIR;
    else
        hStream  << "** Wafer           : ID = " << strWaferID_WIR << " (WIR), " << strWaferID_WRR << " (WRR)";

    hStream  << ", " << m_uiPartsInWafer << " parts (from nb of PRR's), " << partCNT  << " parts (from  WRR)";
}

//////////////////////////////////////////////////////////////////////
// Write Lot summary: unclosed Wafer (no WRR)
//////////////////////////////////////////////////////////////////////
void CSTDFtoASCII::WriteLotSummary_Wafer(QTextStream & hStream)
{
    QString strWaferID_WIR = m_strWaferIDFromWIR.isEmpty() ? "<empty>" : m_strWaferIDFromWIR;

    hStream  << endl;
    hStream  << "** Wafer           : ID = " << strWaferID_WIR;
    hStream  << ", " << m_uiPartsInWafer << " parts (from nb of PRR's), (!!!! NO WRR !!!!)";
}

//////////////////////////////////////////////////////////////////////
// Write Lot summary: close section
//////////////////////////////////////////////////////////////////////
void CSTDFtoASCII::WriteLotSummary_Close(QTextStream & hStream)
{
    if(m_bWaferOpened)
    {
        // Write last wafer (no WRR)
        WriteLotSummary_Wafer(hStream);
    }
    else if(m_uiWIRCount == 0)
    {
        // No wafers: write nb of PRR's
        hStream  << ", " << m_uiPartsInWafer << " parts (from nb of PRR's)";
    }
    hStream  << endl;
    hStream  << "<!--STDF Lot summary!-->";
    hStream  << endl;
    hStream  << endl;
}

//////////////////////////////////////////////////////////////////////
// Write consecutive record information
//////////////////////////////////////////////////////////////////////
void CSTDFtoASCII::WriteConsecutiveRecordInformation(QTextStream & hStream)
{
    hStream  << "** FIRST OF " << QString::number(m_uiRecordCount_Consecutive) << " records" << endl;
}

//////////////////////////////////////////////////////////////////////
// Write Footer section (end of file)
//////////////////////////////////////////////////////////////////////
void CSTDFtoASCII::WriteFooter(QTextStream & hStream)
{
    int nCurrentFileSize = m_hAsciiFile.size();

    hStream  << endl;
    hStream  << endl;

    if(m_bEvaluationMode)
    {
        hStream  << "<!--End of evaluation dump: contact " << GEX_EMAIL_SALES << " for a fully licensed version of the product-->" << endl;
        hStream  << endl;
    }

    if((m_nFileSizeLimit != 0) && (nCurrentFileSize >= m_nFileSizeLimit))
    {
        hStream  << "<!--Report truncated at " << QString::number(nCurrentFileSize/1024)  << " kB" << "-->" << endl;
        hStream  << endl;
    }

    hStream  << "<!--Report created with: "
             << GS::Gex::Engine::GetInstance().Get("AppFullName").toString()
             << " - www.mentor.com-->" << endl;
    hStream  << endl;
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
void CSTDFtoASCII::GetLastError(QString & strError)
{
    strError = GGET_LASTERRORMSG(CSTDFtoASCII,this);
}

bool CSTDFtoASCII::Convert(const QString & strFileNameSTDF, QString & strAsciiFileName, QProgressBar* pProgressBar /*= NULL*/)
{
    bool	bStopDump;
    QDir	cDir;

    // Build destination file name?
    if(strAsciiFileName.isEmpty())
        strAsciiFileName = strFileNameSTDF + ".txt";

    // Remove destination file
    cDir.remove(strAsciiFileName);

    // Set progress bar ptr
    m_pProgressBar = pProgressBar;

    // Set STDF file name
    m_strStdfFile = strFileNameSTDF;

    // Open ASCII files
    if(OpenAsciiFiles(strAsciiFileName) == false)
    {
        // Error. Can't create ASCII file!
        return false;
    }

    // Read all file...and build ASCII file as we go...
    int	iStatus, nRecordType;

    // Pass 1: count nb of records (for eventual progress bar), and in debug dump STDF structure
    // Pass 2: create ASCII file
    int lVersion = STDF_V_UNKNOWN;
    for(lPass = 1;lPass <=2; lPass++)
    {
        // Write record summary on pass 2
        if(lPass == 2)
            WriteRecordSummary(m_hAsciiStream, lVersion);

        // Clear data between passes
        Clear();

        // Open STDF file to read...
        iStatus = m_clStdfParse.Open(strFileNameSTDF.toLatin1().constData());
        if(iStatus == false)
        {
            // Error. Can't open STDF file in read mode!
            GSET_ERROR1(CSTDFtoASCII, eOpenReadFail, NULL, strFileNameSTDF.toLatin1().constData());
            // Close ASCII files
            CloseAsciiFiles();
            return false;
        }
        //Store the version will be used during the parsing and during the pass 2
        m_clStdfParse.GetVersion(&lVersion);
        // Open Lot summary
        if(lPass == 1)
            WriteLotSummary_Open(m_hAsciiStream);

        // Read one record from STDF file.
        iStatus = m_clStdfParse.LoadNextRecord(&nRecordType);
        while((iStatus == GQTL_STDF::StdfParse::NoError) || (iStatus == GQTL_STDF::StdfParse::UnexpectedRecord))
        {
            m_nStdtTotalRecords++;
            bStopDump = false;
            bool lResult = false;
            if(lVersion == STDF_V_4)
                lResult = ProcessRecordV4(nRecordType, bStopDump);
            else if(lVersion == STDF_V_3)
                lResult = ProcessRecordV3(nRecordType, bStopDump);
            else
            {
                // Close STDF file
                m_clStdfParse.Close();
                // Close ASCII files
                CloseAsciiFiles();
                lResult = false;
            }

            if(!lResult)
            {
                return false;
            }
            // Check if dump should be stopped
            if(bStopDump == true)
            {
                // Force progress bar to completed
                ResetProgress(true);
                // Close STDF file
                m_clStdfParse.Close();
                // Close ASCII files
                CloseAsciiFiles();
                return true;
            }

            // Read one record from STDF file.
            iStatus = m_clStdfParse.LoadNextRecord(&nRecordType);
        };

        // Close Lot summary and write dump status
        if(lPass == 1)
        {
            WriteLotSummary_Close(m_hAsciiStream);
            WriteDumpStatus(m_hAsciiStream, iStatus);
        }

        // Save nb of records to process
        m_nStdfRecordsToProcess = m_nStdfRecordsProcessed;

        // Close input STDF file between passes.
        m_clStdfParse.Close();
    }	// 2 passes.

    // Force progress bar to completed
    ResetProgress(true);

    // Convertion successful, close ASCII files.
    CloseAsciiFiles();

    return true;
}

///////////////////////////////////////////////////////////
// Check if consecutive record
///////////////////////////////////////////////////////////
bool CSTDFtoASCII::IsConsecutiveRecord(int nRecordType)
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

        // This record is different from previous
        if((m_uiRecordCount_Consecutive > 1) && (lPass == 2))
            WriteConsecutiveRecordInformation(m_hAsciiStream);

        m_nRecordType_Last = nRecordType;
        m_uiRecordCount_Consecutive = 1;
    }
    return false;
}

bool CSTDFtoASCII::ProcessRecordV3(int recordType, bool	&stopDump)
{
    // Process STDF record read.
    switch(recordType)
    {
    case GQTL_STDF::Stdf_Record::Rec_FAR:
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&mPrivate->mStdfFARv3, stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_MIR:
        if(ProcessMIR((GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfMIRv3), &stopDump) != true)
        {
            // File timestamp is invalid, and higher than expiration date!
            // Close STDF file
            m_clStdfParse.Close();
            // Close ASCII files
            CloseAsciiFiles();
            return false;
        }
        break;
    case GQTL_STDF::Stdf_Record::Rec_MRR:
        if(ProcessMRR((GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfMRRv3), &stopDump) != true)
        {
            // File timestamp is invalid, and higher than expiration date!
            // Close STDF file
            m_clStdfParse.Close();
            // Close ASCII files
            CloseAsciiFiles();
            return false;
        }
        break;
    case GQTL_STDF::Stdf_Record::Rec_HBR:
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfHBRv3), stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_SBR:
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfSBRv3), stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_PMR:
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfPMRv3), stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_WIR:
        if(ProcessWIR((GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfWIRv3), &stopDump) != true)
        {
            // File timestamp is invalid, and higher than expiration date!
            // Close STDF file
            m_clStdfParse.Close();
            // Close ASCII files
            CloseAsciiFiles();
            return false;
        }
        break;
    case GQTL_STDF::Stdf_Record::Rec_WRR:
        if(ProcessWRR((GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfWRRv3), &stopDump) != true)
        {
            // File timestamp is invalid, and higher than expiration date!
            // Close STDF file
            m_clStdfParse.Close();
            // Close ASCII files
            CloseAsciiFiles();
            return false;
        }
        break;
    case GQTL_STDF::Stdf_Record::Rec_WCR:
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfWCRv3), stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_PIR:
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfPIRv3), stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_PRR:
        m_uiPartsInWafer++;
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfPRRv3), stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_TSR:
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfTSRv3), stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_PTR:
        ProcessPTR((GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfPTRv3), &stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_FTR:
        ProcessFTR((GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfFTRv3), &stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_BPS:
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfBPSv3), stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_EPS:
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfEPSv3), stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_GDR:
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfGDRv3), stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_DTR:
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfDTRv3), stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_UNKNOWN:
        ProcessUNKNOWN((GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfUNKNOWNv4), &stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_FDR:
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfFDRv3), stopDump);
         break;
    case GQTL_STDF::Stdf_Record::Rec_PDR:
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfPDRv3), stopDump);
         break;
    case GQTL_STDF::Stdf_Record::Rec_SCR:
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfSCRv3), stopDump);
         break;
    case GQTL_STDF::Stdf_Record::Rec_SHB:
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfSHBv3), stopDump);
         break;
    case GQTL_STDF::Stdf_Record::Rec_SSB:
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfSSBv3), stopDump);
         break;
    case GQTL_STDF::Stdf_Record::Rec_STS:
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfSTSv3), stopDump);
         break;

    }

    return true;
}

bool CSTDFtoASCII::ProcessRecordV4(int recordType, bool &stopDump)
{
    // Process STDF record read.
    switch(recordType)
    {
    case GQTL_STDF::Stdf_Record::Rec_FAR:
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfFARv4), stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_ATR:
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfATRv4), stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_MIR:
        if(ProcessMIR((GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfMIRv4), &stopDump) != true)
        {
            // File timestamp is invalid, and higher than expiration date!
            // Close STDF file
            m_clStdfParse.Close();
            // Close ASCII files
            CloseAsciiFiles();
            return false;
        }
        break;
    case GQTL_STDF::Stdf_Record::Rec_MRR:
        if(ProcessMRR((GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfMRRv4), &stopDump) != true)
        {
            // File timestamp is invalid, and higher than expiration date!
            // Close STDF file
            m_clStdfParse.Close();
            // Close ASCII files
            CloseAsciiFiles();
            return false;
        }
        break;
    case GQTL_STDF::Stdf_Record::Rec_PCR:
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfPCRv4), stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_HBR:
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfHBRv4), stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_SBR:
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfSBRv4), stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_PMR:
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfPMRv4), stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_PGR:
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfPGRv4), stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_PLR:
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfPLRv4), stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_RDR:
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfRDRv4), stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_SDR:
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfSDRv4), stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_WIR:
        if(ProcessWIR((GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfWIRv4), &stopDump) != true)
        {
            // File timestamp is invalid, and higher than expiration date!
            // Close STDF file
            m_clStdfParse.Close();
            // Close ASCII files
            CloseAsciiFiles();
            return false;
        }
        break;
    case GQTL_STDF::Stdf_Record::Rec_WRR:
        if(ProcessWRR((GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfWRRv4), &stopDump) != true)
        {
            // File timestamp is invalid, and higher than expiration date!
            // Close STDF file
            m_clStdfParse.Close();
            // Close ASCII files
            CloseAsciiFiles();
            return false;
        }
        break;
    case GQTL_STDF::Stdf_Record::Rec_WCR:
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfWCRv4), stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_PIR:
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfPIRv4), stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_PRR:
        m_uiPartsInWafer++;
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfPRRv4), stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_TSR:
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfTSRv4), stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_PTR:
        ProcessPTR((GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfPTRv4) ,&stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_MPR:
        ProcessMPR((GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfMPRv4),&stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_FTR:
        ProcessFTR((GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfFTRv4),&stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_BPS:
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfBPSv4), stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_EPS:
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfEPSv4), stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_GDR:
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfGDRv4), stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_DTR:
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfDTRv4), stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_RESERVED_IMAGE:
        ProcessRESERVED_IMAGE((GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfRESERVED_IMAGEv4),&stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_RESERVED_IG900:
        ProcessRESERVED_IG900((GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfRESERVED_IG900v4),&stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_UNKNOWN:
        ProcessUNKNOWN((GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfUNKNOWNv4), &stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_VUR :
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfVURv4), stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_PSR :
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfPSRv4), stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_NMR :
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfNMRv4), stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_CNR :
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfCNRv4), stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_SSR :
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfSSRv4), stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_CDR :
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfCDRv4), stopDump);
        break;
    case GQTL_STDF::Stdf_Record::Rec_STR :
        ProcessRecord(recordType, (GQTL_STDF::Stdf_Record*)&(mPrivate->mStdfSTRv4), stopDump);
        break;

    }

    return true;
}

///////////////////////////////////////////////////////////
// Extract record data and dump it
///////////////////////////////////////////////////////////
void CSTDFtoASCII::ProcessRecord(int nRecordType, GQTL_STDF::Stdf_Record* pclRecord, bool &pbStopDump)
{
    if(!pclRecord)
        return;
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

    // Pass#2: write record to ASCII
    if(lPass == 2)
    {
        QString	strAsciiString;
        if(((m_nFieldFilter & GQTL_STDF::Stdf_Record::FieldFlag_None) == 0) && (m_clStdfParse.ReadRecord(pclRecord) == false))
        {
            // Error reading STDF file. Dump data read and add an error message.
            // Make sure to get only data that has been read (FieldFlag_Present).
            pclRecord->GetAsciiRecord(strAsciiString, m_nFieldFilter | GQTL_STDF::Stdf_Record::FieldFlag_Present);
            m_hAsciiStream  << endl;
            m_hAsciiStream  << strAsciiString;
            m_hAsciiStream  << RECORD_ERROR;
        }
        else if ( (nRecordType != GQTL_STDF::Stdf_Record::Rec_TSR) ||
                  (m_clTNFilteringList.Contains(mPrivate->mStdfTSRv4.m_u4TEST_NUM)
                   ||  m_clTNFilteringList.Contains(mPrivate->mStdfTSRv3.m_u4TEST_NUM) ) )	// test_number filter
        {
            pclRecord->GetAsciiRecord(strAsciiString, m_nFieldFilter);
            m_hAsciiStream  << endl;
            m_hAsciiStream  << strAsciiString;
        }

        // Update progress bar
        if(UpdateProgress() == false)
            pbStopDump = true;
    }
}

///////////////////////////////////////////////////////////
// Extract PTR record data and dump it
///////////////////////////////////////////////////////////
void CSTDFtoASCII::ProcessFTR(GQTL_STDF::Stdf_Record* stdfRecord, bool* pbStopDump)
{
    if(!stdfRecord || !pbStopDump)
        return;
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

    int lVersion = STDF_V_UNKNOWN;
    m_clStdfParse.GetVersion(&lVersion);
    unsigned long lTestNum = 0;

    // Pass#2: write record to ASCII
    if(lPass == 2)
    {
        QString	strAsciiString;
        bool lResult = m_clStdfParse.ReadRecord(stdfRecord);
        if(lVersion == STDF_V_4)
        {
            GQTL_STDF::Stdf_FTR_V4 *lFTR = dynamic_cast<GQTL_STDF::Stdf_FTR_V4 *>(stdfRecord);
            if(!lFTR)
                return ;
            lTestNum = lFTR->m_u4TEST_NUM;
        }
        else if(lVersion == STDF_V_3)
        {
            GQTL_STDF::Stdf_FTR_V3 *lFTR = dynamic_cast<GQTL_STDF::Stdf_FTR_V3 *>(stdfRecord);
            if(!lFTR)
                return ;
            lTestNum = lFTR->m_u4TEST_NUM;
        }

        if(((m_nFieldFilter & GQTL_STDF::Stdf_Record::FieldFlag_None) == 0) && (lResult == false))
        {
            // Error reading STDF file. Dump data read and add an error message.
            // Make sure to get only data that has been read (FieldFlag_Present).
            stdfRecord->GetAsciiRecord(strAsciiString, m_nFieldFilter | GQTL_STDF::Stdf_Record::FieldFlag_Present);
            m_hAsciiStream  << endl;
            m_hAsciiStream  << strAsciiString;
            m_hAsciiStream  << RECORD_ERROR;
        }
        else if (m_clTNFilteringList.Contains(lTestNum))	// test-number filter
        //if(m_clStdfFTR.m_u4TEST_NUM == 2102)	// DEBUG: dump specific tests only
        {
            stdfRecord->GetAsciiRecord(strAsciiString, m_nFieldFilter);
            m_hAsciiStream  << endl;
            m_hAsciiStream  << strAsciiString;
        }

        // Update progress bar
        if(UpdateProgress() == false)
            *pbStopDump = true;
    }
}

///////////////////////////////////////////////////////////
// Extract PTR record data and dump it
///////////////////////////////////////////////////////////
void CSTDFtoASCII::ProcessPTR(GQTL_STDF::Stdf_Record* stdfRecord,bool* pbStopDump)
{
    if(!stdfRecord || !pbStopDump)
        return;
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

    int lVersion = STDF_V_UNKNOWN;
    m_clStdfParse.GetVersion(&lVersion);
    unsigned long lTestNum = 0;

    // Pass#2: write record to ASCII
    if(lPass == 2)
    {
        QString	strAsciiString;
        bool lResult =  m_clStdfParse.ReadRecord(stdfRecord);
        if(lVersion == STDF_V_3)
        {
            GQTL_STDF::Stdf_PTR_V3 *lPTR = dynamic_cast<GQTL_STDF::Stdf_PTR_V3 *>(stdfRecord);
            if(!lPTR)
                return;
            lTestNum = lPTR->m_u4TEST_NUM;
        }
        else if(lVersion == STDF_V_4)
        {
            GQTL_STDF::Stdf_PTR_V4 *lPTR = dynamic_cast<GQTL_STDF::Stdf_PTR_V4 *>(stdfRecord);
            if(!lPTR)
                return;
            lTestNum = lPTR->m_u4TEST_NUM;
        }

        if(((m_nFieldFilter & GQTL_STDF::Stdf_Record::FieldFlag_None) == 0) && ( lResult == false))
        {
            // Error reading STDF file. Dump data read and add an error message.
            // Make sure to get only data that has been read (FieldFlag_Present).
            stdfRecord->GetAsciiRecord(strAsciiString, m_nFieldFilter | GQTL_STDF::Stdf_Record::FieldFlag_Present);
            m_hAsciiStream  << endl;
            m_hAsciiStream  << strAsciiString;
            m_hAsciiStream  << RECORD_ERROR;
        }
        else if (m_clTNFilteringList.Contains(lTestNum))	// test_number filter
        //if(m_clStdfPTR.m_u4TEST_NUM == 44105)	// DEBUG: dump specific tests only
        {
            stdfRecord->GetAsciiRecord(strAsciiString, m_nFieldFilter);
            m_hAsciiStream  << endl;
            m_hAsciiStream  << strAsciiString;
        }

        // Update progress bar
        if(UpdateProgress() == false)
            *pbStopDump = true;
    }
}
///////////////////////////////////////////////////////////
// Extract MPR record data and dump it
///////////////////////////////////////////////////////////
void CSTDFtoASCII::ProcessMPR(GQTL_STDF::Stdf_Record* stdfRecord, bool* pbStopDump)
{
    if(!stdfRecord || !pbStopDump)
        return;
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

    bool lResult = m_clStdfParse.ReadRecord(stdfRecord);
    int lVersion = STDF_V_UNKNOWN;
    m_clStdfParse.GetVersion(&lVersion);
    unsigned long lTestNum = 0;
    if(lVersion == STDF_V_4)
    {
        GQTL_STDF::Stdf_MPR_V4 *lMPR = dynamic_cast<GQTL_STDF::Stdf_MPR_V4 *>(stdfRecord);
        if(!lMPR)
            return;
        lTestNum = lMPR->m_u4TEST_NUM;
    }

    // Pass#2: write record to ASCII
    if(lPass == 2)
    {
        QString	strAsciiString;
        if(((m_nFieldFilter & GQTL_STDF::Stdf_Record::FieldFlag_None) == 0) && ( lResult == false))
        {
            // Error reading STDF file. Dump data read and add an error message.
            // Make sure to get only data that has been read (FieldFlag_Present).
            stdfRecord->GetAsciiRecord(strAsciiString, m_nFieldFilter | GQTL_STDF::Stdf_Record::FieldFlag_Present);
            m_hAsciiStream  << endl;
            m_hAsciiStream  << strAsciiString;
            m_hAsciiStream  << RECORD_ERROR;
        }
        else if (m_clTNFilteringList.Contains(lTestNum))	// test_number filter
        //if(m_clStdfMPR.m_u4TEST_NUM == 2102)	// DEBUG: dump specific tests only
        {
            stdfRecord->GetAsciiRecord(strAsciiString, m_nFieldFilter);
            m_hAsciiStream  << endl;
            m_hAsciiStream  << strAsciiString;
        }

        // Update progress bar
        if(UpdateProgress() == false)
            *pbStopDump = true;
    }
}

/////////////////////////////////////////////////////////////////////////////
// Read STDF.MIR record, dump it to ASCII file...
/////////////////////////////////////////////////////////////////////////////
bool CSTDFtoASCII::ProcessMIR(GQTL_STDF::Stdf_Record*stdfRecord, bool* pbStopDump)	// Extract MIR data
{
    if(!stdfRecord || !pbStopDump)
        return false;

    // Increment record counter for this record
    (m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_MIR])++;

    // Update global record counter
    if(m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_MIR] == true)
        m_nStdfRecordsProcessed++;

    QString lLotID, lSBLotId;
    time_t lStartT=0, lSetupT=0;
    int lVersion = STDF_V_UNKNOWN;
    m_clStdfParse.GetVersion(&lVersion);

    // Pass#1: check license validity, update counters, dump if in debug
    if(lPass == 1)
    {
        // Check if file date is not more recent than license expiration date!
        if(m_clStdfParse.ReadRecord(stdfRecord) == false)
        {
            // Error reading STDF file
            // Convertion failed.
            GSET_ERROR0(CSTDFtoASCII, eStdfRead, GGET_LASTERROR(StdfParse,&m_clStdfParse));
            return false;
        }

        if(lVersion == STDF_V_3)
        {
            GQTL_STDF::Stdf_MIR_V3 *lMIR = dynamic_cast<GQTL_STDF::Stdf_MIR_V3 *>(stdfRecord);
            if(!lMIR)
                return false;
            lStartT = lMIR->m_u4START_T;
            lSetupT = lMIR->m_u4SETUP_T;
            lLotID = lMIR->m_cnLOT_ID;
            lSBLotId = lMIR->m_cnSBLOT_ID;
        }
        else if(lVersion == STDF_V_4)
        {
            GQTL_STDF::Stdf_MIR_V4 *lMIR = dynamic_cast<GQTL_STDF::Stdf_MIR_V4 *>(stdfRecord);
            if(!lMIR)
                return false;
            lStartT = lMIR->m_u4START_T;
            lSetupT = lMIR->m_u4SETUP_T;
            lLotID = lMIR->m_cnLOT_ID;
            lSBLotId = lMIR->m_cnSBLOT_ID;
        }

        // Write Lot/Sublot Information to Ascii file (Lot summary section)
        WriteLotSummary_LotSublot(m_hAsciiStream, lLotID,lSBLotId);

        return true;
    }

    // Pass#2: write record to ASCII
    if((lPass == 2) && (m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_MIR] == true))
    {
        QString	strAsciiString;
        bool lResult = m_clStdfParse.ReadRecord(stdfRecord);
        if(((m_nFieldFilter & GQTL_STDF::Stdf_Record::FieldFlag_None) == 0) && (lResult == false))
        {
            // Error reading STDF file. Dump data read and add an error message.
            // Make sure to get only data that has been read (FieldFlag_Present).
            stdfRecord->GetAsciiRecord(strAsciiString, m_nFieldFilter | GQTL_STDF::Stdf_Record::FieldFlag_Present);
            m_hAsciiStream  << endl;
            m_hAsciiStream  << strAsciiString;
            m_hAsciiStream  << RECORD_ERROR;
        }
        else
        {
            stdfRecord->GetAsciiRecord(strAsciiString, m_nFieldFilter);
            m_hAsciiStream  << endl;
            m_hAsciiStream  << strAsciiString;
        }

        // Update progress bar
        if(UpdateProgress() == false)
            *pbStopDump = true;

        return true;
    }

    return true;
}

///////////////////////////////////////////////////////////
// Read STDF.MRR record, dump it to ASCII file...
///////////////////////////////////////////////////////////
bool CSTDFtoASCII::ProcessMRR(GQTL_STDF::Stdf_Record* stdfRecord, bool* pbStopDump)
{
    if(!stdfRecord || !pbStopDump)
        return false;

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
        bool lResult = m_clStdfParse.ReadRecord(stdfRecord);
        int lVersion = STDF_V_UNKNOWN;
        m_clStdfParse.GetVersion(&lVersion);
        time_t lFinishT = 0;
        if(lVersion == STDF_V_3)
        {
            GQTL_STDF::Stdf_MRR_V3 *lMRR = dynamic_cast<GQTL_STDF::Stdf_MRR_V3 *>(stdfRecord);
            if(!lMRR)
                return false;
            lFinishT = lMRR->m_u4FINISH_T;
        }
        else if(lVersion == STDF_V_4)
        {
            GQTL_STDF::Stdf_MRR_V4 *lMRR = dynamic_cast<GQTL_STDF::Stdf_MRR_V4 *>(stdfRecord);
            if(!lMRR)
                return false;
            lFinishT = lMRR->m_u4FINISH_T;
        }

        // Check if file date is not more recent than license expiration date!
        if(((m_nFieldFilter & GQTL_STDF::Stdf_Record::FieldFlag_None) == 0) && (lResult == false))
        {
            // Error reading STDF file
            // Convertion failed.
            GSET_ERROR0(CSTDFtoASCII, eStdfRead, GGET_LASTERROR(StdfParse,&m_clStdfParse));
            return false;
        }

        return true;
    }

    // Pass#2: write record to ASCII
    if((lPass == 2) && (m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_MRR] == true))
    {
        QString	strAsciiString;
        if(((m_nFieldFilter & GQTL_STDF::Stdf_Record::FieldFlag_None) == 0) && (m_clStdfParse.ReadRecord(stdfRecord) == false))
        {
            // Error reading STDF file. Dump data read and add an error message.
            // Make sure to get only data that has been read (FieldFlag_Present).
            stdfRecord->GetAsciiRecord(strAsciiString, m_nFieldFilter | GQTL_STDF::Stdf_Record::FieldFlag_Present);
            m_hAsciiStream  << endl;
            m_hAsciiStream  << strAsciiString;
            m_hAsciiStream  << RECORD_ERROR;
        }
        else
        {
            stdfRecord->GetAsciiRecord(strAsciiString, m_nFieldFilter);
            m_hAsciiStream  << endl;
            m_hAsciiStream  << strAsciiString;
        }

        // Update progress bar
        if(UpdateProgress() == false)
            *pbStopDump = true;

        return true;
    }

    return true;
}

///////////////////////////////////////////////////////////
// Read STDF.WIR record, dump it to ASCII file...
///////////////////////////////////////////////////////////
bool CSTDFtoASCII::ProcessWIR(GQTL_STDF::Stdf_Record *stdfRecord, bool* pbStopDump)
{
    if(!stdfRecord || !pbStopDump )
        return false;

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
        bool lResult = m_clStdfParse.ReadRecord(stdfRecord);
        int lVersion = STDF_V_UNKNOWN;
        m_clStdfParse.GetVersion(&lVersion);
        time_t lStartT = 0;
        QString lWaferId;
        if(lVersion == STDF_V_3)
        {
            GQTL_STDF::Stdf_WIR_V3 *lWIR = dynamic_cast<GQTL_STDF::Stdf_WIR_V3 *>(stdfRecord);
            if(!lWIR)
                return false;
            lStartT = lWIR->m_u4START_T;
            lWaferId = lWIR->m_cnWAFER_ID;
        }
        else if(lVersion == STDF_V_4)
        {
            GQTL_STDF::Stdf_WIR_V4 *lWIR = dynamic_cast<GQTL_STDF::Stdf_WIR_V4 *>(stdfRecord);
            if(!lWIR)
                return false;
            lStartT = lWIR->m_u4START_T;
            lWaferId = lWIR->m_cnWAFER_ID;
        }

        // Check if file date is not more recent than license expiration date!
        if(((m_nFieldFilter & GQTL_STDF::Stdf_Record::FieldFlag_None) == 0) && (lResult == false))
        {
            // Error reading STDF file
            // Convertion failed.
            GSET_ERROR0(CSTDFtoASCII, eStdfRead, GGET_LASTERROR(StdfParse,&m_clStdfParse));
            return false;
        }

        // Update Lot/Sublot summary information
        m_uiWIRCount++;
        m_uiPartsInWafer = 0;
        m_strWaferIDFromWIR = lWaferId;
        m_bWaferOpened= true;

        return true;
    }

    // Pass#2: write record to ASCII
    if((lPass == 2) && (m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_WIR] == true))
    {
        QString	strAsciiString;
        if(((m_nFieldFilter & GQTL_STDF::Stdf_Record::FieldFlag_None) == 0) && (m_clStdfParse.ReadRecord(stdfRecord) == false))
        {
            // Error reading STDF file. Dump data read and add an error message.
            // Make sure to get only data that has been read (FieldFlag_Present).
            stdfRecord->GetAsciiRecord(strAsciiString, m_nFieldFilter | GQTL_STDF::Stdf_Record::FieldFlag_Present);
            m_hAsciiStream  << endl;
            m_hAsciiStream  << strAsciiString;
            m_hAsciiStream  << RECORD_ERROR;
        }
        else
        {
            stdfRecord->GetAsciiRecord(strAsciiString, m_nFieldFilter);
            m_hAsciiStream  << endl;
            m_hAsciiStream  << strAsciiString;
        }

        // Update progress bar
        if(UpdateProgress() == false)
            *pbStopDump = true;

        return true;
    }

    return true;
}

///////////////////////////////////////////////////////////
// Read STDF.WRR record, dump it to ASCII file...
///////////////////////////////////////////////////////////
bool CSTDFtoASCII::ProcessWRR(GQTL_STDF::Stdf_Record *stdfRecord, bool* pbStopDump)
{
    if(!stdfRecord || !pbStopDump )
        return false;

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
        bool lResult = m_clStdfParse.ReadRecord(stdfRecord);

        // Check if file date is not more recent than license expiration date!
        if(((m_nFieldFilter & GQTL_STDF::Stdf_Record::FieldFlag_None) == 0) && (lResult == false))
        {
            // Error reading STDF file
            // Convertion failed.
            GSET_ERROR0(CSTDFtoASCII, eStdfRead, GGET_LASTERROR(StdfParse,&m_clStdfParse));
            return false;
        }

        int lVersion = STDF_V_UNKNOWN;
        m_clStdfParse.GetVersion(&lVersion);
        time_t lFinishT = 0;
        QString lWaferId;
        unsigned long lPartCnt = 0;
        if(lVersion == STDF_V_3)
        {
            GQTL_STDF::Stdf_WRR_V3 *lWRR = dynamic_cast<GQTL_STDF::Stdf_WRR_V3 *>(stdfRecord);
            if(!lWRR)
                return false;
            lFinishT = lWRR->m_u4FINISH_T;
            lWaferId = lWRR->m_cnWAFER_ID;
            lPartCnt = lWRR->m_u4PART_CNT;
        }
        else if(lVersion == STDF_V_4)
        {
            GQTL_STDF::Stdf_WRR_V4 *lWRR = dynamic_cast<GQTL_STDF::Stdf_WRR_V4 *>(stdfRecord);
            if(!lWRR)
                return false;
            lFinishT = lWRR->m_u4FINISH_T;
            lWaferId = lWRR->m_cnWAFER_ID;
            lPartCnt = lWRR->m_u4PART_CNT;
        }

        // Write Lot/Sublot Information to Ascii file (Lot summary section)
        m_bWaferOpened= false;
        WriteLotSummary_Wafer(m_hAsciiStream, lWaferId, lPartCnt);

        return true;
    }

    // Pass#2: write record to ASCII
    if((lPass == 2) && (m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_WRR] == true))
    {
        QString	strAsciiString;
        if(((m_nFieldFilter & GQTL_STDF::Stdf_Record::FieldFlag_None) == 0) && (m_clStdfParse.ReadRecord(stdfRecord) == false))
        {
            // Error reading STDF file. Dump data read and add an error message.
            // Make sure to get only data that has been read (FieldFlag_Present).
            stdfRecord->GetAsciiRecord(strAsciiString, m_nFieldFilter | GQTL_STDF::Stdf_Record::FieldFlag_Present);
            m_hAsciiStream  << endl;
            m_hAsciiStream  << strAsciiString;
            m_hAsciiStream  << RECORD_ERROR;
        }
        else
        {
            stdfRecord->GetAsciiRecord(strAsciiString, m_nFieldFilter);
            m_hAsciiStream  << endl;
            m_hAsciiStream  << strAsciiString;
        }

        // Update progress bar
        if(UpdateProgress() == false)
            *pbStopDump = true;

        return true;
    }

    return true;
}

///////////////////////////////////////////////////////////
// Read STDF.RESERVED_IMAGE record, dump it to ASCII file...
///////////////////////////////////////////////////////////
void CSTDFtoASCII::ProcessRESERVED_IMAGE(GQTL_STDF::Stdf_Record* stdfRecord, bool* pbStopDump)
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

    // Pass#2: write record to ASCII
    if(lPass == 2)
    {
        QString	strAsciiString;
        if(((m_nFieldFilter & GQTL_STDF::Stdf_Record::FieldFlag_None) == 0) && (m_clStdfParse.ReadRecord(stdfRecord) == false))
        {
            // Error reading STDF file. Dump data read and add an error message.
            // Make sure to get only data that has been read (FieldFlag_Present).
            stdfRecord->GetAsciiRecord(strAsciiString, m_nFieldFilter | GQTL_STDF::Stdf_Record::FieldFlag_Present);
            m_hAsciiStream  << endl;
            m_hAsciiStream  << strAsciiString;
            m_hAsciiStream  << RECORD_ERROR;
        }
        else
        {
            stdfRecord->GetAsciiRecord(strAsciiString, m_nFieldFilter);
            m_hAsciiStream  << endl;
            m_hAsciiStream  << strAsciiString;
        }

        // Update progress bar
        if(UpdateProgress() == false)
            *pbStopDump = true;
    }
}

///////////////////////////////////////////////////////////
// Read STDF.RESERVED_IG900 record, dump it to ASCII file...
///////////////////////////////////////////////////////////
void CSTDFtoASCII::ProcessRESERVED_IG900(GQTL_STDF::Stdf_Record* stdfRecord,bool* pbStopDump)
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

    // Pass#2: write record to ASCII
    if(lPass == 2)
    {
        QString	strAsciiString;
        if(((m_nFieldFilter & GQTL_STDF::Stdf_Record::FieldFlag_None) == 0) && (m_clStdfParse.ReadRecord(stdfRecord) == false))
        {
            // Error reading STDF file. Dump data read and add an error message.
            // Make sure to get only data that has been read (FieldFlag_Present).
            stdfRecord->GetAsciiRecord(strAsciiString, m_nFieldFilter | GQTL_STDF::Stdf_Record::FieldFlag_Present);
            m_hAsciiStream  << endl;
            m_hAsciiStream  << strAsciiString;
            m_hAsciiStream  << RECORD_ERROR;
        }
        else
        {
            stdfRecord->GetAsciiRecord(strAsciiString, m_nFieldFilter);
            m_hAsciiStream  << endl;
            m_hAsciiStream  << strAsciiString;
        }

        // Update progress bar
        if(UpdateProgress() == false)
            *pbStopDump = true;
    }
}

///////////////////////////////////////////////////////////
// Read STDF.UNKNOWN record, dump it to ASCII file...
///////////////////////////////////////////////////////////
void CSTDFtoASCII::ProcessUNKNOWN(GQTL_STDF::Stdf_Record* stdfRecord,bool* pbStopDump)
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

    // Pass#2: write record to ASCII
    if(lPass == 2)
    {
        QString	strAsciiString;
        if(((m_nFieldFilter & GQTL_STDF::Stdf_Record::FieldFlag_None) == 0) && (m_clStdfParse.ReadRecord(stdfRecord) == false))
        {
            // Error reading STDF file. Dump data read and add an error message.
            // Make sure to get only data that has been read (FieldFlag_Present).
            stdfRecord->GetAsciiRecord(strAsciiString, m_nFieldFilter | GQTL_STDF::Stdf_Record::FieldFlag_Present);
            m_hAsciiStream  << endl;
            m_hAsciiStream  << strAsciiString;
            m_hAsciiStream  << RECORD_ERROR;
        }
        else
        {
            stdfRecord->GetAsciiRecord(strAsciiString, m_nFieldFilter);
            m_hAsciiStream  << endl;
            m_hAsciiStream  << strAsciiString;
        }

        // Update progress bar
        if(UpdateProgress() == false)
            *pbStopDump = true;
    }
}

///////////////////////////////////////////////////////////
// Open file for ascii dump
///////////////////////////////////////////////////////////
bool CSTDFtoASCII::OpenAsciiFiles(const QString & strAsciiFileName)
{
    // Open ASCII files
    m_hAsciiFile.setFileName(strAsciiFileName);
    if(!m_hAsciiFile.open(QIODevice::WriteOnly))
    {
        GSET_ERROR1(CSTDFtoASCII, eOpenWriteFail, NULL, strAsciiFileName.toLatin1().constData());
        return false;
    }

    // Assign file I/O stream
    m_hAsciiStream.setDevice(&m_hAsciiFile);

    // Write header to file
    WriteHeader(m_hAsciiStream);

    return true;
}

///////////////////////////////////////////////////////////
// Close ascii dump file
///////////////////////////////////////////////////////////
void CSTDFtoASCII::CloseAsciiFiles(void)
{
    // Write footer to file
    WriteFooter(m_hAsciiStream);

    m_hAsciiFile.close();
}

///////////////////////////////////////////////////////////
// Reset progress bar
///////////////////////////////////////////////////////////
void CSTDFtoASCII::ResetProgress(bool bForceCompleted /*= false*/)
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
bool CSTDFtoASCII::UpdateProgress(void)
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
        int nCurrentFileSize = m_hAsciiFile.size();
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
void CSTDFtoASCII::setTNFilteringList(QString strListe)
{
    m_clTNFilteringList.SetRange(strListe);
}

