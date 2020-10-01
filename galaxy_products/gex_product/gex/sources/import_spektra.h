#ifndef GEX_IMPORT_SPEKTRA_H
#define GEX_IMPORT_SPEKTRA_H

#include <time.h>

#include <qdatetime.h>
#include <QTextStream>

// Galaxy modules includes
#include <gstdl_errormgr.h>

#include "cspektra.h"
#include "cspektraparse_v3.h"
#include "stdf.h"

class QProgressBar;

class CSPEKTRAtoASCII
{
public:
	GDECLARE_ERROR_MAP(CSPEKTRAtoASCII)
	{
		eOpenReadFail,			// Failed Opening SPEKTRA file
		eOpenWriteFail, 		// Failed Opening ASCII file
		eLicenceExpired, 		// File date out of Licence window!
		eSpektraRead			// Error reading SPEKTRA file
	}
	GDECLARE_END_ERROR_MAP(CSPEKTRAtoASCII)

	// Constructor / destructor functions
	CSPEKTRAtoASCII(bool bEvaluationMode);
	~CSPEKTRAtoASCII();
	void			Clear();									// Clear data
	void			SetProcessRecord(const int nRecordType);	// Add to list of records to process
	void			SetFieldFilter(const int nFilter)	{ m_nFieldFilter = nFilter; }		// Set Field filter
	void			SetFieldFilterOptionText(const QString & strFieldFilterText)	{ m_strFieldFilterText = strFieldFilterText; }		// Set Field filter option text
	void			SetSizeFileLimit(int nSizeLimit)	{ m_nFileSizeLimit = nSizeLimit; }	// Set size limit of destination file
	void			SetSizeFileLimitOptionText(const QString & strFileSizeLimitText)	{ m_strFileSizeLimitText = strFileSizeLimitText; }	// Set size limit option text
    bool			Convert(const char* szFileNameSPEKTRA, const char* szAsciiFileName, QProgressBar* pProgressBar = NULL);
	void			GetLastError(QString & strError);

private:
	void			WriteAsciiHeader(QTextStream & hStream);
	void			WriteAsciiFooter(QTextStream & hStream);

        bool			m_bEvaluationMode;				// Set to true if evaluation mode
	CSpektraParse_V3	m_clSpektraParse;				// SPEKTRA V3 parser
        QString			m_strSpektraFile;				// SPEKTRA File to convert
        QFile			m_hAsciiFile;					// Handle to ASCII file
        QTextStream		m_hAsciiStream;					// Handle to ASCII stream
        int 			lPass;							// 2 passes are required to process the SPEKTRA file.
        int			m_nSpektraRecords;				// Counts nb of records in SPEKTRA file during each pass
        int			m_nSpektraRecordsToProcess;		// Nb of Spektra records to process
        int			m_nProgress;					// Used for progress bar
        bool			m_bRecordsToProcess[CSpektra_Record_V3::Rec_COUNT];	// Array of records to display
        QString			m_strFieldFilterText;			// Field filter option text
        int			m_nFieldFilter;					// Field filter (no fields, present fields...)
        QString			m_strFileSizeLimitText;			// Size limit option text
        int			m_nFileSizeLimit;				// Size limit of destination file
	QProgressBar*		m_pProgressBar;					// Progress bar
	
	void	ProcessRecord(CSpektra_Record_V3* pclRecord, bool* pbStopDump);	// Extract record data, write to ASCII
    bool	ProcessLabel(CSpektra_Label_V3* pclSpektraLabel, bool* pbStopDump);	// Extract Label data, write to ASCII

	bool	OpenAsciiFile(const QString & strAsciiFileName);
	void	CloseAsciiFile(void);
	void	ResetProgress(bool bForceCompleted = false);
	bool	UpdateProgress(void);
};

class CSPEKTRABinningInfo
{
public:
	// Constructor / destructor functions
	CSPEKTRABinningInfo();
	~CSPEKTRABinningInfo();

	int		m_nCount;
	char	m_cPassFail;

	void	Reset();
};

class CSPEKTRAtoSTDF
{
public:
	GDECLARE_ERROR_MAP(CSPEKTRAtoSTDF)
	{
		eOpenReadFail,			// Failed Opening SPEKTRA file
		eOpenWriteFail, 		// Failed Opening STDF file
		eLicenceExpired, 		// File date out of Licence window!
		eSpektraRead			// Error reading SPEKTRA file
	}
	GDECLARE_END_ERROR_MAP(CSPEKTRAtoSTDF)

	// Constructor / destructor functions
	CSPEKTRAtoSTDF();
	~CSPEKTRAtoSTDF();
	void			Clear();									// Clear data
    bool			Convert(const char* szFileNameSPEKTRA, const char* szStdfFileName, QProgressBar* pProgressBar = NULL);
	void			GetLastError(QString & strError);

	static bool		IsCompatible(const char *szFileName);

private:


	CSpektraParse_V3		m_clSpektraParse;				// SPEKTRA V3 parser
	QString					m_strSpektraFile;				// SPEKTRA File to convert
    GS::StdLib::Stdf					m_clStdf;   		// STDF V4 read/write
	int 					lPass;							// 2 passes are required to process the SPEKTRA file.
	int						m_nSpektraRecords;				// Counts nb of records in SPEKTRA file during each pass
	int						m_nSpektraRecordsToProcess;		// Nb of Spektra records to process
	int						m_nProgress;					// Used for progress bar
	QProgressBar*			m_pProgressBar;					// Progress bar

	uint					m_uiDateTime;					// Dete and time stored in Spektra file
	CSpektra_TestItem_V3*	m_pTestItems;					// Array of TestItem records read in the SPEKTRA file
	int						m_nTestItemIndex;				// Index in TestItem array
	int						m_nWaferCount;					// Nb of wafers
	int						m_nParts;						// Nb of parts
	int						m_nGoodParts;					// Nb of good parts
	int						m_nFailParts;					// Nb of fail parts
	int						m_nPartsInWafer;				// Nb of parts in current wafer
	int						m_nGoodPartsInWafer;			// Nb of good parts in current wafer
	int						m_nTests;						// Nb of tests
	int						m_nCurrentDeviceID;				// Current Device ID (Serial nb)
	int						m_nCurrentBinning;				// Current Device Binning
	CSPEKTRABinningInfo		m_pclBinnings[251];				// Binning count for each binning read in SPEKTRA file
	int						m_nCurrentWaferID;				// Current Wafer ID
	int						m_nFailTestsInDevice;			// Nb of fail tests in current device
	bool					m_bFirstDevice;					// First device of the file is being processed
	
    bool	ProcessLabel(CSpektra_Label_V3* pclSpektraLabel);	// Extract Label data, write to STDF
	bool	ProcessTestItem(CSpektra_TestItem_V3* pclSpektraTestItem);	// Extract TestItem data, write to STDF
	bool	ProcessWaferID(CSpektra_WaferID_V3* pclSpektraWaferID);		// Extract WaferID data, write to STDF
	bool	ProcessDeviceID(CSpektra_DeviceID_V3* pclSpektraDeviceID);	// Extract DeviceID data, write to STDF
	bool	ProcessTestData(CSpektra_TestData_V3* pclSpektraTestData);	// Extract TestData data, write to STDF
	void	UpdateTestItem(CSpektra_TestData_V3* pclSpektraTestData);	// Update test item (limit attributes...) according to tes result
	void	WriteMIR(CSpektra_Label_V3* pclSpektraLabel);				// Write MIR record to STDF file
	void	WriteWIR(CSpektra_WaferID_V3* pclSpektraWaferID);			// Write WIR record to STDF file
	void	WriteWRR(void);												// Write WRR record to STDF file
	void	WritePIR(CSpektra_DeviceID_V3* pclSpektraDeviceID);			// Write PIR record to STDF file
	void	WritePRR(void);												// Write PRR record to STDF file
	void	WritePTR(CSpektra_TestData_V3* pclSpektraTestData);			// Write PTR record to STDF file
	void	WriteHBRs(void);											// Write HBR records to STDF file
	void	WriteSBRs(void);											// Write SBR records to STDF file
	void	WritePCR(void);												// Write PCR record to STDF file
	void	WriteMRR(void);												// Write MRR record to STDF file

	bool	OpenStdfFile(const QString & strStdfFileName);
	void	CloseStdfFile(void);
	void	ResetProgress(bool bForceCompleted = false);
	void	UpdateProgress(void);
	void	CreateDebugCsvFile();
};


#endif
