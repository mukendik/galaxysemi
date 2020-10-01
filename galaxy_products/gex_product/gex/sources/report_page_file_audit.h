///////////////////////////////////////////////////////////
// File Audit class
///////////////////////////////////////////////////////////

#ifndef GEX_FILE_AUDIT_H
#define GEX_FILE_AUDIT_H

class CGexGroupOfFiles;						// report_build.h
class CGexFileInGroup;						// report_build.h

namespace GS
{
    namespace Gex
    {
        class CustomReportFileAuditSection;
    }
}

class GexFileAudit
{
public:
    /*!
     * \fn GexFileAudit
     * \brief Constructor
     */
    GexFileAudit(
        GS::Gex::CustomReportFileAuditSection* pFileAudit,
        FILE* hReportFile,
        bool bCheckPAT,
        int iTotalSites,
        bool bCsvOutput);
	void DisplayHeaderDetails(void);
    //! \brief Check STDF file compliancy: PAT and Data sets.
    bool CheckStdfFileRecordsAudit(QString &strStdfFile);
	bool CheckStdfFileDatasetAudit(unsigned uGroupID);
	bool CheckStdfFileSitesCorrelationAudit(void);

private:
	void CheckHtmlPageChange(bool bForceChange=false,QString strPageName="");
	void WriteSectionTitle(QString strTitle);

	QString strTruncateName(QString &strName);
	void StdfPatCompliance_LogFileRecordError(bool bValidRecord,bool bPatCritical,QString strErrorType,QString strComment);
	void StdfPatCompliance_LogFileDatasetError(CTest *ptTestCell,bool bShowHistogram,QString strErrorType,QString strComment);
    int StdfPatCompliance_ReadRecord(int nRecordType, GQTL_STDF::Stdf_Record* pclRecord,QString &strErrorMessage);
	int StdfPatCompliance_ProcessMIR(void);
	int StdfPatCompliance_ProcessPTR(void);
	int StdfPatCompliance_ProcessMPR(void);
	int StdfPatCompliance_ProcessFTR(void);
	int StdfPatCompliance_ProcessPRR(void);

    GS::Gex::CustomReportFileAuditSection* m_pFileAudit;
	unsigned m_uGroupID;
	CGexGroupOfFiles *m_pGroup;
	CGexFileInGroup  *m_pFile;
	FILE	*m_hReportFile;
	bool	m_bCheckPAT;
	bool	m_bCsvOutput;

	// Variables for STDF PAT compliancy check.
    GQTL_STDF::StdfParse	m_clStdfParse;					// STDF V4 parser
    GQTL_STDF::Stdf_FAR_V4	m_clStdfFAR;
    GQTL_STDF::Stdf_ATR_V4	m_clStdfATR;
    GQTL_STDF::Stdf_MIR_V4	m_clStdfMIR;
    GQTL_STDF::Stdf_MRR_V4	m_clStdfMRR;
    GQTL_STDF::Stdf_PCR_V4	m_clStdfPCR;
    GQTL_STDF::Stdf_HBR_V4	m_clStdfHBR;
    GQTL_STDF::Stdf_SBR_V4	m_clStdfSBR;
    GQTL_STDF::Stdf_PMR_V4	m_clStdfPMR;
    GQTL_STDF::Stdf_PGR_V4	m_clStdfPGR;
    GQTL_STDF::Stdf_PLR_V4	m_clStdfPLR;
    GQTL_STDF::Stdf_RDR_V4	m_clStdfRDR;
    GQTL_STDF::Stdf_SDR_V4	m_clStdfSDR;
    GQTL_STDF::Stdf_WIR_V4	m_clStdfWIR;
    GQTL_STDF::Stdf_WRR_V4	m_clStdfWRR;
    GQTL_STDF::Stdf_WCR_V4	m_clStdfWCR;
    GQTL_STDF::Stdf_PIR_V4	m_clStdfPIR;
    GQTL_STDF::Stdf_PRR_V4	m_clStdfPRR;
    GQTL_STDF::Stdf_TSR_V4	m_clStdfTSR;
    GQTL_STDF::Stdf_PTR_V4	m_clStdfPTR;
    GQTL_STDF::Stdf_MPR_V4	m_clStdfMPR;
    GQTL_STDF::Stdf_FTR_V4	m_clStdfFTR;
    GQTL_STDF::Stdf_BPS_V4	m_clStdfBPS;
    GQTL_STDF::Stdf_EPS_V4	m_clStdfEPS;
    GQTL_STDF::Stdf_GDR_V4	m_clStdfGDR;
    GQTL_STDF::Stdf_DTR_V4	m_clStdfDTR;
    GQTL_STDF::Stdf_RESERVED_IMAGE_V4	m_clStdfRESERVED_IMAGE;
    GQTL_STDF::Stdf_RESERVED_IG900_V4	m_clStdfRESERVED_IG900;
    GQTL_STDF::Stdf_UNKNOWN_V4		m_clStdfUNKNOWN;

	int		m_ReportPageIndex;			// Keeps track of HTML report page written
	int		m_iTotalSites;
	bool	m_FlexFormatMultiSites;		// 'true' if data file analyzed is of Flex/J750 type
	int		m_iTotalLogRecordErrors;		// Count how many File Records errors have been logged
	int		m_iTotalLogDatasetErrors;		// Count how many Dataset errors have been logged
	int		m_InPIR;
	bool	m_InWIR;
	int		m_ErrType_EndOfRecord;		// Keeps track of total 'Unexpected end of record' errors.
	int		m_ErrType_PTR_No_PIR;		// Keeps track of total PTRs outside or PIR/PRR records.
	int		m_ErrType_PRR_No_DieLoc;	// Keeps track of total PRRs without Die coordinates.
	int		m_ErrType_TestFullNameIssue;	// Keep track of total test names discrepancies.
	int		m_ErrType_TestNameRootNameIssue; // Keep track of test name discrepency ignoring possible leading pin#
	int		m_ErrType_TestLimitIssue;	// Keep track of total test limits discrepancies.
	int		m_ErrType_LimitMissingIssue; // Keep track if missing limits issues
	int		m_ErrType_UnknownRec;		// Keep track of total unknown records
	int		m_ErrType_MIR;				// Keep track of total MIR (should be only one!)
	int		m_ErrType_WIR;				// Keep track of total WIR (should be only one!)
	int		m_ErrType_PCR;				// Keeps track of total PCR (should be only one!)
	int		m_ErrType_MRR;				// Keeps track of total MRR (should be only one!)
	QMap <long, QString>	m_clTestNames;
	QMap <long, double>		m_clTestLowLimits;
	QMap <long, double>		m_clTestHighLimits;
};
#endif

