#ifndef GEX_EXPORT_ASCII_H
#define GEX_EXPORT_ASCII_H
#include <time.h>

#include <qdatetime.h>
#include <QTextStream>

 // Galaxy modules includes
#include <gstdl_errormgr.h>

#include "stdf.h"
#include "stdfparse.h"
#include "classes.h"
#include "stdfrecords_v4.h"
#include "stdfrecords_v3.h"

#include <QFile>

#define GALAXYSEMI_ASCII_HEADER "<!--Semiconductor Data Analysis is easy with Galaxy!-->"

class QProgressBar;
class CSTDFtoASCIIPrivate;

/*! \class STDF to Ascii dump class
*/
class CSTDFtoASCII : public QObject
{
    Q_OBJECT

public:
    GDECLARE_ERROR_MAP(CSTDFtoASCII)
    {
        eOpenReadFail,			// Failed Opening STDF file
                eOpenWriteFail, 		// Failed Opening ASCII file
                eLicenceExpired, 		// File date out of Licence window!
                eStdfRead				// Error reading STDF file
    }
    GDECLARE_END_ERROR_MAP(CSTDFtoASCII)

    // Constructor / destructor functions
    CSTDFtoASCII(bool bEvaluationMode);
    ~CSTDFtoASCII();
    //! \brief Add to list of records to process
    void		SetProcessRecord(const int nRecordType);
    //! \brief All/no records should be dumped
    void		SetProcessRecord(bool bProcessRecords);
    void		SetFieldFilter(const int nFilter)	{ m_nFieldFilter = nFilter; }	// Set Field filter
    void		SetFieldFilterOptionText(const QString & strFieldFilterText)	{ m_strFieldFilterText = strFieldFilterText; }	// Set Field filter option text
    void		SetSizeFileLimit(int nSizeLimit)	{ m_nFileSizeLimit = nSizeLimit; }	// Set size limit of destination file
    void		SetSizeFileLimitOptionText(const QString & strFileSizeLimitText)	{ m_strFileSizeLimitText = strFileSizeLimitText; }	// Set size limit option text
    void		SetConsecutiveRecordOption(bool bDumpFirstOnly) { m_bDumpFirstOnly = bDumpFirstOnly; }	// Set option concerning consecutive records
    // Convert STDF file to ASCII file
    bool		Convert(const QString & strFileNameSTDF, QString & strAsciiFileName, QProgressBar* pProgressBar = NULL);
    void		GetLastError(QString & strError);
    void		setTNFilteringList(QString strListe);							// accessor to m_TNFilteringList

private:
    CSTDFtoASCIIPrivate *mPrivate;

private:
    void		Clear();														// Clear data
    void		ProcessRecord(int nRecordType, GQTL_STDF::Stdf_Record* pclRecord, bool &pbStopDump);	// Extract record data, write to ASCII
    // General function to Process Record if it comes from v3 and v4
    bool        ProcessRecordV4(int recordType, bool	&stopDump);
    bool        ProcessRecordV3(int recordType, bool	&stopDump);
    void		ProcessPTR(GQTL_STDF::Stdf_Record*, bool* pbStopDump);									// Extract PTR data, write to ASCII

    void		ProcessFTR(GQTL_STDF::Stdf_Record*, bool* pbStopDump);									// Extract FTR data, write to ASCII
    void		ProcessMPR(GQTL_STDF::Stdf_Record* stdfRecord, bool* pbStopDump);									// Extract MPR data, write to ASCII
    bool		ProcessMIR(GQTL_STDF::Stdf_Record*, bool* pbStopDump);			// Extract MIR data, write to ASCII
    bool		ProcessMRR(GQTL_STDF::Stdf_Record*, bool* pbStopDump);			// Extract MRR data, write to ASCII
    bool		ProcessWIR(GQTL_STDF::Stdf_Record*, bool* pbStopDump);			// Extract WIR data, write to ASCII
    bool		ProcessWRR(GQTL_STDF::Stdf_Record*, bool* pbStopDump);			// Extract WRR data, write to ASCII

    void		ProcessRESERVED_IMAGE(GQTL_STDF::Stdf_Record* stdfRecord, bool* pbStopDump);						// Reserved for use by Image
    void		ProcessRESERVED_IG900(GQTL_STDF::Stdf_Record* stdfRecord, bool* pbStopDump);						// Reserved for use by IG900
    void		ProcessUNKNOWN(GQTL_STDF::Stdf_Record* stdfRecord, bool* pbStopDump);								// Unknown record

    bool		OpenAsciiFiles(const QString & strStdfFileName);
    void		CloseAsciiFiles(void);
    void		ResetProgress(bool bForceCompleted = false);
    bool		UpdateProgress(void);
    bool		IsConsecutiveRecord(int nRecordType);
    //! \brief Write Header section (starting file)
    void		WriteHeader(QTextStream & hStream);
    void		WriteFooter(QTextStream & hStream);
    void		WriteRecordSummary(QTextStream & hStream, int stdfVersion= STDF_V_4);
    void		WriteDumpStatus(QTextStream & hStream, int nStatus);
    void		WriteConsecutiveRecordInformation(QTextStream & hStream);
    void		WriteLotSummary_Open(QTextStream & hStream);
    void		WriteLotSummary_Close(QTextStream & hStream);
    void		WriteLotSummary_LotSublot(QTextStream & hStream, const QString &lotId, const QString &sbLotId);
    void		WriteLotSummary_Wafer(QTextStream & hStream, const QString & waferId, unsigned long partCNT);
    void		WriteLotSummary_Wafer(QTextStream & hStream);
    // DATA
private:
    bool			m_bEvaluationMode;				// Set to true if evaluation mode
    GQTL_STDF::StdfParse	m_clStdfParse;		// STDF V4 parser
    QString			m_strStdfFile;					// STDF File to convert
    QFile			m_hAsciiFile;					// Handle to ASCII file
    QTextStream		m_hAsciiStream;					// Handle to ASCII stream
    int 			lPass;							// 2 passes are required to process the STDF file.
    int				m_nStdtTotalRecords;			// Total number of records in STDF file
    int				m_nStdfRecordsProcessed;		// Nb of Stdf records processed so far in current pass
    int				m_nStdfRecordsToProcess;		// Nb of Stdf records to process for next pass
    int				m_nProgress;					// Used for progress bar
    bool			m_bRecordsToProcess[GQTL_STDF::Stdf_Record::Rec_COUNT];	// Array of records to display
    QString			m_strFieldFilterText;			// Field filter option text
    int				m_nFieldFilter;					// Field filter (no fields, present fields...)
    QString			m_strFileSizeLimitText;			// Size limit option text
    int				m_nFileSizeLimit;				// Size limit of destination file
    QProgressBar*	m_pProgressBar;					// Progress bar
    bool			m_bDumpFirstOnly;				// Set to true if only the first of consecutive records should be dumped
    GS::QtLib::Range		m_clTNFilteringList;			// Stock test-number list to filter

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

};


#endif
