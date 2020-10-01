#ifndef GEX_EXPORT_ATDF_H
#define GEX_EXPORT_ATDF_H
#include <time.h>

#include <qdatetime.h>
#include <QTextStream>

 // Galaxy modules includes
#include <gstdl_errormgr.h>

#include "stdf.h"
#include "stdfparse.h"
#include "classes.h"
#include <QFile>

class QProgressBar;
class CSTDFtoATDF;

//! \class STDF to Atdf dump class
class CSTDFtoATDF : public QObject
{
    Q_OBJECT
    // Dev rules : all public functions must be slots
    // example of why : Kate want to do STDF to ATDF in JS, not with a YM task
public slots:
    //! \brief Add to list of records to process
    void SetProcessRecord(const int nRecordType);
    // All/no records should be dumped
    void SetProcessRecord(bool bProcessRecords);
    // Set size limit of destination file
    void SetSizeFileLimit(int nSizeLimit)	{ m_nFileSizeLimit = nSizeLimit; }
    // Set size limit option text
    void SetSizeFileLimitOptionText(const QString & strFileSizeLimitText)	{ m_strFileSizeLimitText = strFileSizeLimitText; }
    // Set option concerning consecutive records
    void SetConsecutiveRecordOption(bool bDumpFirstOnly) { m_bDumpFirstOnly = bDumpFirstOnly; }
    // Convert 'szFileNameSTDF' STDF file, to ATDF 'szAtdfFileName' file
    bool Convert(const QString & strFileNameSTDF, QString & strAtdfFileName, QProgressBar* pProgressBar = NULL);
    // Same as convert but with 2 const QString& in order to be used in JS
    QString Convert(const QString & strFileNameSTDF, const QString & strAtdfFileName);
    // Get Error
    void GetLastError(QString & strError);
    void SetWriteHeaderFooter(bool bWriteHeaderFooter) { m_bWriteHeaderFooter = bWriteHeaderFooter; }
    // accessor to m_TNFilteringList
    void setTNFilteringList(QString strListe);
    //! \brief Clear data
    void Clear();
    bool OpenAtdfFiles(const QString & strStdfFileName);
    void CloseAtdfFiles(void);
    void ResetProgress(bool bForceCompleted = false);
    bool UpdateProgress(void);
    bool IsConsecutiveRecord(int nRecordType);
    void WriteDumpStatus(QTextStream & hStream, int nStatus);

public:
	GDECLARE_ERROR_MAP(CSTDFtoATDF)
	{
		eOpenReadFail,			// Failed Opening STDF file
		eOpenWriteFail, 		// Failed Opening ATDF file
		eLicenceExpired, 		// File date out of Licence window!
		eStdfRead				// Error reading STDF file
	}
	GDECLARE_END_ERROR_MAP(CSTDFtoATDF)

	// Constructor / destructor functions
    CSTDFtoATDF();
    CSTDFtoATDF(bool bEvaluationMode);
    CSTDFtoATDF(const CSTDFtoATDF&);
	~CSTDFtoATDF();

private:
    void	ProcessRecord(int nRecordType, GQTL_STDF::Stdf_Record* pclRecord, bool* pbStopDump);	// Extract record data, write to ATDF
    void	ProcessPTR(bool* pbStopDump);									// Extract PTR data, write to ATDF
    void	ProcessFTR(bool* pbStopDump);									// Extract FTR data, write to ATDF
    void	ProcessMPR(bool* pbStopDump);									// Extract MPR data, write to ATDF
    bool	ProcessMIR(bool* pbStopDump);			// Extract MIR data, write to ATDF
    bool	ProcessMRR(bool* pbStopDump);			// Extract MRR data, write to ATDF
    bool	ProcessWIR(bool* pbStopDump);			// Extract WIR data, write to ATDF
    bool	ProcessWRR(bool* pbStopDump);			// Extract WRR data, write to ATDF
    void	ProcessRESERVED_IMAGE(bool* pbStopDump);						// Reserved for use by Image
    void	ProcessRESERVED_IG900(bool* pbStopDump);						// Reserved for use by IG900
    void	ProcessUNKNOWN(bool* pbStopDump);								// Unknown record
// DATA
	bool			m_bEvaluationMode;				// Set to true if evaluation mode
    GQTL_STDF::StdfParse	m_clStdfParse;		// STDF V4 parser
	QString			m_strStdfFile;					// STDF File to convert
	QFile			m_hAtdfFile;					// Handle to ATDF file
	QTextStream		m_hAtdfStream;					// Handle to ATDF stream
	int 			lPass;							// 2 passes are required to process the STDF file.
	int				m_nStdtTotalRecords;			// Total number of records in STDF file
	int				m_nStdfRecordsProcessed;		// Nb of Stdf records processed so far in current pass
	int				m_nStdfRecordsToProcess;		// Nb of Stdf records to process for next pass
	int				m_nProgress;					// Used for progress bar
    bool			m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_COUNT];	// Array of records to display
	QString			m_strFileSizeLimitText;			// Size limit option text
	int				m_nFileSizeLimit;				// Size limit of destination file
	QProgressBar*	m_pProgressBar;					// Progress bar
	bool			m_bDumpFirstOnly;				// Set to true if only the first of consecutive records should be dumped
	bool			m_bWriteHeaderFooter;			// Set to true if we header/footer shoud be dumped

    GS::QtLib::Range m_clTNFilteringList;			// Stock test-number list to filter

	// For Lot Summary
	unsigned int	m_uiWIRCount;					// Nb of WIR records
	unsigned int	m_uiPartsInWafer;				// Nb of PRR's in wafer
	QString			m_strWaferIDFromWIR;			// Wafer ID read in WIR
	bool			m_bWaferOpened;					// Set to true when a WIR is read, and to false when the WRR is read

	// To handle consecutive records
	int				m_nRecordType_Last;				// Type of last record read
	unsigned int	m_uiRecordCount_Consecutive;	// Consecutive records counter

	// Record objects and record counters
    unsigned int	m_uiRecordCount[GQTL_STDF::Stdf_Record::Rec_COUNT];	// Arrray of record counters
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
    GQTL_STDF::Stdf_VUR_V4	m_clStdfVUR;
    GQTL_STDF::Stdf_PSR_V4 	m_clStdfPSR;
    GQTL_STDF::Stdf_NMR_V4 	m_clStdfNMR;
    GQTL_STDF::Stdf_CNR_V4 	m_clStdfCNR;
    GQTL_STDF::Stdf_SSR_V4 	m_clStdfSSR;
    GQTL_STDF::Stdf_CDR_V4 	m_clStdfCDR;
    GQTL_STDF::Stdf_STR_V4 	m_clStdfSTR;
};


#endif
