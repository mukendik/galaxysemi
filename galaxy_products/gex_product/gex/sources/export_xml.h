#ifndef GEX_EXPORT_XML_H
#define GEX_EXPORT_XML_H
#include <time.h>

 // Galaxy modules includes
#include <gstdl_errormgr.h>

#include "report_build.h"
#include "stdf.h"
#include "stdfparse.h"
#include "cpart_info.h"

#define GEX_MAX_STRING	257 	// Max. String length in STDF file.
#define GEX_UNITS 		8 		// Max. string length for units (truncated if bigger)
#define GEX_TEST_LABEL	20		// Max chars. to build a TestLabel string!
#define GEX_LIMIT_LABEL 20		// Max chars. to build a test limit string!

class QProgressBar;

//////////////////////////////////////////////////////
// Test cell definition used for XML file creation.
//////////////////////////////////////////////////////
class CXmlShortTest
{
public:
	CXmlShortTest();
	unsigned int 	lTestNumber;				// TestNumber
	int 			lPinmapIndex; 				// =GEX_PTEST (-1) for standard Parametric test 
												// =GEX_MPTEST (-2) for parametric multi-result master test
												// >=0 for MultiParametric results test, tells result pinmap index
	double			lfResult; 					// Test result for a given run.
	double			lfLowLimit; 				// Test Low Limit
	double			lfHighLimit;				// Test High Limit
	int 			res_scal; 					// Scale factor on test results
	int 			llm_scal; 					// Scale factor on LowLimit
	int 			hlm_scal; 					// Scale factor on HighLimit
	BYTE			bLimitFlag; 				// bit0=1 (no low limit), bit1=1 (no high limit)
	BYTE			bTestType;					// P=Parametric,F=Functionnal,M=Multiple-result parametric.
	bool			bTestExecuted;				// Flag telling if part was in the run flow.
	QString			strTestName;				// Test Name
	char			szTestUnits[GEX_UNITS]; 	// Test Units.
	// Arrays to store displayed info...avoids to rebuild them for each report page
	char			szTestLabel[GEX_TEST_LABEL];
	char			szLowL[GEX_LIMIT_LABEL];
	char			szHighL[GEX_LIMIT_LABEL];
	CXmlShortTest*	ptNextTest;					// Pointer to CTest structure
};


class CSTDFtoXML
{
public:
	GDECLARE_ERROR_MAP(CSTDFtoXML)
	{
		eOpenReadFail,			// Failed Opening STDF file
		eOpenWriteFail, 		// Failed Opening XML file
		eLicenceExpired, 		// File date out of Licence window!
		eStdfRead				// Error reading STDF file
	}
	GDECLARE_END_ERROR_MAP(CSTDFtoXML)

	// Constructor / destructor functions
	CSTDFtoXML();
	~CSTDFtoXML();
    //! \brief Clear data
    void			Clear();
    bool			Convert(const char* strFileNameSTDF, const char* XmlFileName, QProgressBar* pProgressBar = NULL);
	QString 		GetLastError();

private:
	void			WriteXmlHeader(QTextStream & hStream);
	void			WriteXmlFooter(QTextStream & hStream);

    GQTL_STDF::StdfParse		m_clStdfParse;		// STDF V4 parser
	QString				m_strStdfFile;					// STDF File to convert
	QFile				m_hXmlMainSequenceFile;			// Handle to XML file
	QFile				m_hXmlInitialSequenceFile;		// Handle to XML file
	QFile				m_hXmlPinmapSequenceFile;		// Handle to XML file
	QFile				m_hXmlLimitsSequenceFile;		// Handle to XML file
	QFile				m_hXmlResultsSequenceFile;		// Handle to XML file
	QFile				m_hXmlSummarySequenceFile;		// Handle to XML file
	QFile				m_hXmlSummaryTSRSequenceFile;	// Handle to XML file
	QFile				m_hXmlSummaryHBRSequenceFile;	// Handle to XML file
	QFile				m_hXmlSummarySBRSequenceFile;	// Handle to XML file
	QFile				m_hXmlSummaryPCRSequenceFile;	// Handle to XML file
	QFile				m_hXmlFinalSequenceFile;		// Handle to XML file
	QFile				m_hDebugDumpFile;				// Handle to Dump file for debug
	QTextStream			m_hXmlMainSequenceStream;		// Handle to XML stream
	QTextStream			m_hXmlInitialSequenceStream;	// Handle to XML stream
	QTextStream			m_hXmlPinmapSequenceStream;		// Handle to XML stream
	QTextStream			m_hXmlLimitsSequenceStream;		// Handle to XML stream
	QTextStream			m_hXmlResultsSequenceStream;	// Handle to XML stream
	QTextStream			m_hXmlSummarySequenceStream;	// Handle to XML stream
	QTextStream			m_hXmlSummaryTSRSequenceStream;	// Handle to XML stream
	QTextStream			m_hXmlSummaryHBRSequenceStream;	// Handle to XML stream
	QTextStream			m_hXmlSummarySBRSequenceStream;	// Handle to XML stream
	QTextStream			m_hXmlSummaryPCRSequenceStream;	// Handle to XML stream
	QTextStream			m_hXmlFinalSequenceStream;		// Handle to XML stream
	QTextStream			m_hDebugDumpStream;				// Handle to Dump stream for debug
	int					m_nMainSequenceIndentationLevel;	// Current level of indentation in Main sequence
	int					m_nInitialSequenceIndentationLevel;	// Current level of indentation in Initial sequence
	int					m_nPinmapSequenceIndentationLevel;	// Current level of indentation in Pinmap sequence
	int					m_nLimitsSequenceIndentationLevel;	// Current level of indentation in Limits sequence
	int					m_nResultsSequenceIndentationLevel;	// Current level of indentation in Results sequence
	int					m_nSummarySequenceIndentationLevel;	// Current level of indentation in Summary sequence
	int					m_nFinalSequenceIndentationLevel;	// Current level of indentation in Final sequence
	int 				lPass;					// 2 passes are required to process the STDF file.
	CPartInfo			PartInfo; 				// Structure to hold part info: DieXY, Exec time, Tests executed, partID, etc...
	CXmlShortTest*		ptTestList;				// Pointing to list of Tests
	int					m_nPTRCount;			// Counts nb of PTRs for each part during pass 1
	int					m_nMPRCount;			// Counts nb of MPRs for each part during pass 1
	int					m_nFTRCount;			// Counts nb of FTRs for each part during pass 1
	int					m_nStdfRecords;			// Counts nb of records in STDF file during pass 1
	int					m_nProgress;			// Used for progress bar
	bool				m_bPartStarted;			// Set to true once at least 1 part has been started
	bool				m_bTestProcessed;		// Set to true once at least 1 test has been processed
	bool				m_bAddToLimits;			// Set to true if tests should be added to limit file
	
	bool	ProcessFAR(void);		// Extract FAR data, write to XML + write XML header
	bool	ProcessATR(void);		// Extract ATR data, write to XML
    bool	ProcessMIR(); // Extract MIR data, write to XML
	bool	ProcessMRR(void);		// Extract MRR data, write to XML
	bool	ProcessPCR(void);		// Extract PCR data, write to XML
	bool	ProcessHBR(void);		// Extract HBR data, write to XML
	bool	ProcessSBR(void);		// Extract SBR data, write to XML
	bool	ProcessPMR(void);		// Extract PMR data, write to XML
	bool	ProcessPGR(void);		// Extract PGR data, write to XML
	bool	ProcessPLR(void);		// Extract PLR data, write to XML
	bool	ProcessRDR(void);		// Extract RDR data, write to XML
	bool	ProcessSDR(void);		// Extract SDR data, write to XML
	bool	ProcessWIR(void);		// Extract WIR data, write to XML
	bool	ProcessWRR(void);		// Extract WRR data, write to XML
	bool	ProcessWCR(void);		// Extract WCR data, write to XML
	bool	ProcessPIR(void);		// Extract PIR data, write to XML
	bool	ProcessPRR(void);		// Extract PRR data, write to XML
	bool	ProcessTSR(void);		// Extract TSR data, write to XML
	bool	ProcessPTR(void);		// Extract PTR data, write to XML
	bool	ProcessMPR(void);		// Extract MPR data, write to XML
	bool	ProcessFTR(void);		// Extract FTR data, write to XML
	bool	ProcessBPS(void);		// Extract BPS data, write to XML
	bool	ProcessEPS(void);		// Extract EPS data, write to XML
	bool	ProcessGDR(void);		// Extract GDR data, write to XML
	bool	ProcessDTR(void);		// Extract DTR data, write to XML
	bool	ProcessRESERVED_IMAGE(void);	// Reserved for use by Image
	bool	ProcessRESERVED_IG900(void);	// Reserved for use by IG900
	bool	ProcessUNKNOWN(void);	// Unknown record

	bool	OpenFiles(const QString & strStdfFileName, const QString & strXmlFileName);
	void	CloseFiles(void);
	void	InitXmlSequenceFiles(void);
	void	CloseXmlSequenceFiles(void);
	void	WriteTag(QTextStream & hStream, const char* szTag, int nIndentationLevel);
	void	DebugDumpRecord(int nRecordType, int nIndentationLevel);
	void	UpdateProgressBar(QProgressBar* pProgressBar);

    //int 	FindTestCell(unsigned int lTestNumber,int lPinmapIndex, CXmlShortTest **ptTestCellFound,BOOL bCreateIfNew,BOOL bResetList);
};


#endif
