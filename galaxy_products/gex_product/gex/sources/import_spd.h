#ifndef GEX_IMPORT_SPD_H
#define GEX_IMPORT_SPD_H

#include <time.h>

#include <qdatetime.h>
#include <qstringlist.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"

// Predefined parameter fields.
#define	CSV_RAW_SBIN	0
#define	CSV_RAW_HBIN	1
#define	CSV_RAW_DIEX	2
#define	CSV_RAW_DIEY	3
#define	CSV_RAW_SITE	4
#define	CSV_RAW_TIME	5

class CGSpdParameter
{
public:
	QString strName;	// Parameter name. E.g: "Idd_Total_Shutdown"
	QString strUnits;	// Parameter units,E.g: "A"
	unsigned long	lTestNumber;// Test#
	long	lPinmapIndex;// Test pinmap index
	float	fLowLimit;	// Parameter Low Spec limit, E.g: 0.00004
	float	fHighLimit;	// Parameter High Spec limit, E.g: -0.00004
	int		scale;		// Scale factor for limits
	bool	bValidLowLimit;		// Low limit defined
	bool	bValidHighLimit;	// High limit defined
    float	fValue;		// Parameter result
	bool	bStaticHeaderWritten;	// 'true' after first STDF PTR static header data written.
};

class CGSPDtoSTDF
{
public:
	CGSPDtoSTDF();
	~CGSPDtoSTDF();
    bool	Convert(const char *SpdFileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:
    bool ReadSpdFile(const char *SpdFileName, const char *strFileNameSTDF);
	bool WriteStdfFile(QTextStream *hCsvFile,const char *strFileNameSTDF);
	void NormalizeLimits(int iIndex, const QString& strUnits);

	QString ReadLine(QTextStream& hFile);
	
	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int	iProgressStep;
	int	iNextFilePos;
	int	iFileSize;

	QString m_strLastError;	// Holds last error string during CSV->STDF convertion
	int		m_iLastError;	// Holds last error ID
	enum  errCodes
	{
		errNoError,			// No erro (default)
		errOpenFail,		// Failed Opening CSV file
		errInvalidHeader,	// Invalid header content
		errInvalidFormatParameter,	// Invalid CSV format: didn't find the 'Parameter' section
		errInvalidFormatLowInRows,	// Invalid CSV format: Didn't find parameter rows
		errLicenceExpired,	// File date out of Licence window!
		errWriteSTDF		// Failed creating STDF intermediate file
	};

	char m_cDelimiter;			// Holds the delimiter character to use (' or \t typically)

	CGSpdParameter *m_pCGSpdParameter;	// List of Parameters tables.
	int				m_iTotalParameters;			// Holds the total number of parameters / tests in each part tested

	time_t	m_lStartTime;				// Dataset creation time
	int		m_iBurninTime;				// Burn-In Time
	QString m_strProgramName;			// Job name
	QString	m_strProductName;			// Product name
	QString	m_strLotID;					// LotID string
	QString m_strOperator;				// operator
	QString	m_strTesterName;			// Tester name string
	QString	m_strTesterType;			// Tester type.
};


#endif
