#ifndef ABSTRACT_CSV_CONVERTER_H
#define ABSTRACT_CSV_CONVERTER_H

#include <time.h>

#include <gqtl_utils.h>

#include "export_csv_test.h"
#include "report_build.h"
#include "stdf.h"
#include "cpart_info.h"

class QProgressDialog;

#define GALAXY_CSV_VERSION_V1_MAJOR      1
#define GALAXY_CSV_VERSION_V2_MAJOR      2

class AbstractCsvConverter
{
public:
    AbstractCsvConverter(unsigned int majorVersion, unsigned int minorVersion);
    virtual ~AbstractCsvConverter();

    static AbstractCsvConverter* CreateConverter(int majorVersion, int minorVersion);

    // Convert 'strFileNameSTDF' STDF file, to CSV 'CsvFileName' file
    bool        Convert(const QString& strFileNameSTDF, const QString& strCsvFileName);
    //! \brief Get Last Error
    QString     GetLastError() const;

protected:

    virtual bool    UpdateOptions(CReportOptions*);         // update cached options
    virtual void    Clear(void);
    virtual void    WriteResultsHeader(void) = 0;
    virtual bool    ProcessMIR();                           // Extract MIR data
    virtual void	ProcessSDR(void);                       // Extract Site info details (loadboard, etc...)
    virtual void	ProcessPDR(void);                       // Extract Parametric Test Definition (STDF V3 only.)
    virtual void	ProcessFDR(void);                       // Extract Functional Test Definition (STDF V3 only.)
    virtual void	ProcessFTR(void);                       // Extract FTR data
    virtual void	ProcessPTR(void);                       // Extract Test results
    virtual void	ProcessMPR(void);                       // Extract Test results (Multiple Parametric)
    virtual void    ProcessPIR(void);                       // first PIR is used as trigger to write result header !
    virtual void	ProcessPRR(void);                       // Binning result.
    virtual void	ProcessTSR(void);                       // To extract test name in case not found in PTRs
    virtual void	ProcessWCR(void);                       // Extract wafer flat info
    virtual void	ProcessWIR(void);                       // Extract wafer#
    virtual void	ProcessMRR(void);                       // Extract End testing time.
    virtual void	ProcessPMR(void);                       // Extract Pinmap info.
    virtual void	ProcessDTR(void);                       // Extract DTR data
    virtual bool	ProcessHBR(void);                       // Extract HARD Bin results
    virtual bool    ProcessSBR(void);                       // Extract SOFT Bin results
    virtual void    ProcessGDR(void);                       // extract generic datas

    // STDF records ignored:
    //	virtual void	ProcessATR(void);	// Extract ATR data
    //	virtual void	ProcessPCR(void);	// Extract PCR data
    //	virtual void	ProcessPGR(void);	// Pin Group Record
    //	virtual void	ProcessPLR(void);	// Pin List Record

    int             ReadStringToField(char *szField);
    QString         FormatTestName(const char * szString) const;                                                    // Format test name ( replace ',' by ';')
    QString         buildTestNameString(CShortTest * ptTestCell);                                                   // Build the test name string
    int             FindTestCell(unsigned int lTestNumber,int lPinmapIndex, CShortTest **ptTestCellFound,
                                    BOOL bCreateIfNew, BOOL bResetList, const QString& strName = QString());
    int             FindPinmapCell(CPinmap **ptPinmapCellFound,int iPinmapIndex);
    unsigned int    findMappedTestName(long lTestType, unsigned int nTestNumber, const QString& strTestName);       // Test name mapping method

protected:

    enum  errCodes
    {
        errNoError,					// No erro (default)
        errOpenReadFail,			// Failed Opening STDF file
        errOpenWriteFail,			// Failed Opening CSV file
        errInvalidFormatParameter,	// Invalid CSV format: didn't find the 'Parameter' section
        errInvalidFormatLowInRows,	// Invalid CSV format: Didn't find parameter rows
        errInvalidFormatMissingUnit,// Invalid CSV format: 'Unit' line missing
        errInvalidFormatMissingUSL,	// Invalid CSV format: 'USL' line missing
        errInvalidFormatMissingLSL,	// Invalid CSV format: 'LSL' line missing
        errLicenceExpired,			// File date out of Licence window!
        errWriteSTDF				// Failed creating STDF intermediate file
    };

    enum StdfCompliancy
    {
        STRINGENT,
        FLEXIBLE
    };            // for Option("dataprocessing", "stdf_compliancy")

    enum MPRMergeMode
    {
        MERGE,
        NO_MERGE
    };				// for Option("dataprocessing","multi_parametric_merge_mode"

    enum MPRMergeCriteria
    {
        FIRST,
        LAST,
        MIN,
        MAX,
        MEAN,
        MEDIAN
    }; // for Option ("dataprocessing","multi_parametric_merge_criteria"

    enum MergeDuplicateTestRule
    {
        MERGE_TEST_NUMBER,
        MERGE_TEST_NAME,
        NEVER_MERGE_TEST
    };	// for Option("dataprocessing", "duplicate_test"

    enum UnitsMode
    {
        UnitsNormalized		= 0,
        UnitsScalingFactor	= 1
    };

    enum SortingField
    {
        SortOnTestID	= 0,
        SortOnFlowID	= 1
    };

    int                     iLastError;				// Holds last error ID
    GS::StdLib::Stdf					StdfFile;	// Handle to STDF file to read
    QTextStream				hCsvTableFile;			// Handle to CSV file to create
    GS::StdLib::StdfRecordReadInfo		StdfRecordHeader;
    int						lPass;					// 2 passes are required to process the STDF file.
    CPartInfo				PartInfo;				// Structure to hold part info: DieXY, Exec time, Tests executed, partID, etc...
    CShortTest *			ptTestList;				// Pointing to list of Tests
    CPinmap	*				ptPinmapList;			// Pointing to list of Pinmap
    QMap <int,int>			m_cSitesUsed;			// Holds list of sites used
    cTestList				m_cFullTestFlow;		// Holds longest test flow found (Bin1 flow)
    CFileDataInfo			m_cFileData;			// Holds various info extracted from file during Pass#1.

    // Options
    StdfCompliancy          m_eStdfCompliancy;
    MPRMergeMode            m_eMPRMergeMode;
    MPRMergeCriteria        m_eMPRMergeCriteria;
    MergeDuplicateTestRule  m_eTestMergeRule;
    UnitsMode               m_eUnitsMode;
    SortingField            m_eSortingField;
    bool                    m_bSplitExport;

    BYTE                    bData;						// Temporary buffer when reading STDF object
    int                     wData;						// same as above
    long                    lData;						// same as above
    float                   fData;						// same as above
    bool                    m_bIsResultsHeaderWritten;    // set to 'true' when result header is written
    bool                    m_FlexFormatMultiSites;		// set to 'true' if original data file is from Teradyne Flex family of testers and multi-sites data.
    qint32                  m_nFlowID;					// Keep last flow ID number

    unsigned int            mMajorVersion;              // Csv Major version
    unsigned int            mMinorVersion;              // Csv Minor version

    QMap<unsigned int, QString>      mSoftBins;
    QMap<unsigned int, QString>      mHardBins;

    bool                    m_bRemoveSeqName;
    bool                    m_bRemovePinName;

    QSet<QString>           mChannelNames;

    GS::QtLib::NumberFormat mNumberFormat;              // Used to format numbers into a string

private:

    class CSVTestNameMappingPrivate
    {
    public:

        CSVTestNameMappingPrivate()	{ }
        ~CSVTestNameMappingPrivate()	{ }

        unsigned int						findTestNumber(unsigned int nTestNumber, const QString& strTestName);

    private:

        QMap<unsigned int, unsigned int>	m_mapTestNumber;
        QMap <QString, unsigned int>		m_mapTestName;
    };

    // MAPs used if must ignore test# and only rely on test name.
    QMap<int, CSVTestNameMappingPrivate>	m_testNameMap;
    QProgressDialog *m_poProgDialog;
    int m_iFileNumber;
    int m_iFileCount;
public:
    // Store customer value PIR - DTR = <@field=value> - PRR
    // syntax - <@field=value>, remove any @
    // error if more than one < or >
    // DTR must be under one PIR/PRR
    // interlaced PIR/PIR/PRR/PRR is allowed if have site info: <@field=site#|value>
    QMap <QString, QMap <int,QString> >      m_mapVariableValue;  // <field, [site]value>
    QMap <QString,   int>          m_mapVariableSeq;    // <field, first sequence position>
    // Reset all variables value
    // Keep all entries
    void resetVariables() {foreach(QString key, m_mapVariableValue.keys()) m_mapVariableValue[key].clear();}

    // Keep the nb of record in the flow ie <"MIR",1>
    QMap<QString,int>       m_cRecordCount;
    int setRecordCount(int recordType, int recordSubType);
    int getRecordCount(int recordType, int recordSubType);

    //! \brief ?
    void setProgressDialog(QProgressDialog* poProgDialog, int iFileNumber, int iFileCount);
    void formatSequencerAndPinTestName(QString &strTestName);
};

#endif // ABSTRACT_CSV_CONVERTER_H
