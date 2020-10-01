#ifndef GEX_IMPORT_CSM_TYPE_2_H
#define GEX_IMPORT_CSM_TYPE_2_H

#include <time.h>

#include <qdatetime.h>
#include <qstringlist.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"

// Predefined parameter fields.
#define	CSM_RAW_PRODUCT		0
#define	CSM_RAW_LOT			1
#define	CSM_RAW_WAFER		2
#define	CSM_RAW_SPLIT		3
#define	CSM_RAW_SITE		4
#define	CSM_RAW_X_ADDR		5
#define	CSM_RAW_Y_ADDR		6

class	GexDatabaseEntry;

class CGCsm2Parameter
{
public:
    QString strName;			// Parameter name. E.g: "Idd_Total_Shutdown"
    QString strUnits;			// Parameter units,E.g: "A"
    unsigned long	lTestNumber;// Test#
    long	lPinmapIndex;		// Test pinmap index
    float	fLowLimit;			// Parameter Low Spec limit, E.g: 0.00004
    float	fHighLimit;			// Parameter High Spec limit, E.g: -0.00004
    bool	bValidLowLimit;		// Low limit defined
    bool	bValidHighLimit;	// High limit defined
    float	fLowSpecLimit;		// Parameter Low Spec limit, E.g: 0.00004
    float	fHighSpecLimit;		// Parameter High Spec limit, E.g: -0.00004
    bool	bValidLowSpecLimit;	// Low limit defined
    bool	bValidHighSpecLimit;// High limit defined
    float	fTargetValue;		// Parameter target spec value, E.g: -0.00004
    bool	bValidTarget;		// Target spec value
    float	fValue;				// Parameter result
    bool	bStaticHeaderWritten;// 'true' after first STDF PTR static header data written.
};

class CGCSM2toSTDF
{
public:
    CGCSM2toSTDF();
    ~CGCSM2toSTDF();
    static bool IsCompatible(QString CsmFileName);
    bool Convert(const char *CsmFileName, const char *strFileNameSTDF);
    QString GetLastError();

private:
    void clear(void);

    void LoadParameterIndexTable(void);
    void DumpParameterIndexTable(void);
    void UpdateParameterIndexTable(QString strParamName);
    bool ReadCsmFile(const char *CsmFileName, const char *strFileNameSTDF);
    bool WriteStdfFile(QTextStream *hCsvFile,const char *strFileNameSTDF);
    int	 ResultDisplayScaleFactor(unsigned uIndex);

    QString ReadLine(QTextStream& hFile);

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    int	iProgressStep;
    int	iNextFilePos;
    int	iFileSize;

    QString		m_strLastError;			// Holds last error string during CSV->STDF convertion
    int			m_iLastError;			// Holds last error ID
    enum  errCodes
    {
        errNoError,						// No erro (default)
        errOpenFail,					// Failed Opening CSV file
        errInvalidFormatParameter,		// Invalid CSV format: didn't find the 'Parameter' section
        errInvalidFormatLowInRows,		// Invalid CSV format: Didn't find parameter rows
        errInvalidFormatMissingUnit,	// Invalid CSV format: 'Unit' line missing
        errInvalidFormatMissingUSL,		// Invalid CSV format: 'USL' line missing
        errInvalidFormatMissingLSL,		// Invalid CSV format: 'LSL' line missing
        errInvalidFormatMissingTarget,	// Invalid CSV format: 'TARGET' line missing
        errInvalidFormatMissingLTL,		// Invalid CSV format: 'LTL' line missing
        errInvalidFormatMissingHTL,		// Invalid CSV format: 'HTL' line missing
        errLicenceExpired,				// File date out of Licence window!
        errWriteSTDF					// Failed creating STDF intermediate file
    };

    bool				m_bNewCsmParameterFound;	// set to true if CSV file has a Parameter name not in our reference table=> makes it to be updated
    CGCsm2Parameter *	m_pCGCsm2Parameter;			// List of Parameters tables.
    unsigned			m_uTotalParameters;			// Holds the total number of parameters / tests in each part tested
    QStringList			m_pFullCsmParametersList;	// Complete list of ALL CSV parameters known.

    long				m_lStartParsingOffset;

    time_t				m_lStartTime;				// Dataset creation time
    int					m_iBurninTime;				// Burn-In Time
    QString				m_strLotID;					// LotID string
    QString				m_strProductID;				// Product / Device name
    QString				m_strTesterName;			// Tester name string
    QString				m_strTesterType;			// Tester type.
    QString 			m_strProgramName;			// Job name
    QString 			m_strProgramRev;			// Job rev
    QString 			m_strOperator;				// operator
    QString 			m_strExecType;				// exec-type
    QString 			m_strExecRev;				// exe-ver
    QString 			m_strTestCode;				// test-cod
    QString 			m_strFacilityID;			// Facility-ID
    QString 			m_strFloorID;				// FloorID
    QString 			m_strProcessID;				// ProcessID
    QString				m_strSubLotID;				// SubLotID
    QString				m_strWaferID;				// WaferID
    QString 			m_strTemperature;			// Temperature testing.
    QString				m_strPackageType;			// PAckage type.
    QString				m_strFamilyID;				// FamilyID
    QString 			m_strFrequencyStep;			// Frequency / Step

    // Wafer details
    float				m_lfWaferDiameter;			// Wafer diameter in mm
    float				m_lfWaferDieWidth;			// Die X size
    float				m_lfWaferDieHeight;			// Die Y size
    char				m_cWaferFlat;				// Flat orientation: U, D, R or L
    int					m_iWaferCenterDieX;			// X coordinate of center die on wafer
    int					m_iWaferCenterDieY;			// Y coordinate of center die on wafer
    char				m_cWaferPosX;				// Positive direction for X axis
    char				m_cWaferPosY;				// Positive direction for Y axis
};


#endif
