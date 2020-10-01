#ifndef GEX_IMPORT_PCM_JAZZ_H
#define GEX_IMPORT_PCM_JAZZ_H

#include <qdatetime.h>
#include <qstringlist.h>

#include "gex_constants.h"
#include "stdf.h"
#include "import_wat.h"

class CGJazzParameter
{
public:
	QString strName;	// Parameter name. E.g: "Idd_Total_Shutdown"
	QString strUnits;	// Parameter units,E.g: "A"
	float	fLowLimit;	// Parameter Low Spec limit, E.g: 0.00004
	float	fHighLimit;	// Parameter High Spec limit, E.g: -0.00004
	int		scale;		// Scale factor for limits
	bool	bValidLowLimit;		// Low limit defined
	bool	bValidHighLimit;	// High limit defined
    float	fValue;		// Parameter result
	bool	bStaticHeaderWritten;	// 'true' after first STDF PTR static header data written.
};

class CGPCM_JAZZtoSTDF
{
public:
	CGPCM_JAZZtoSTDF();
	~CGPCM_JAZZtoSTDF();
    bool	Convert(const char *JazzFileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:

	void LoadParameterIndexTable(void);
	void DumpParameterIndexTable(void);
	void UpdateParameterIndexTable(QString strParamName);
	CGWatParameter *FindParameterEntry(int iWaferID,int iSiteID,QString strParamName,QString strUnits="");
	void SaveParameterResult(int iWaferID,int iSiteID,QString strName,QString strUnits,QString strValue);
	void SaveParameterLimit(QString strName,QString strValue,int iLimit);
	void NormalizeLimits(int iIndex,QString &strUnits);
    bool ReadJazzFile(const char *JazzFileName, const char *strFileNameSTDF);
	bool WriteStdfFile(QTextStream *hWatFile,const char *strFileNameSTDF);

	QString ReadLine(QTextStream& hFile);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int	iProgressStep;
	int	iNextFilePos;
	int	iFileSize;


	QString m_strLastError;	// Holds last error string during WAT->STDF convertion
	int		m_iLastError;	// Holds last error ID
	enum  errCodes
	{
		errNoError,					// No erro (default)
		errOpenFail,				// Failed Opening CSV file
		errInvalidHeader,			// Invalid header content
		errInvalidFormatParameter,	// Invalid CSV format: didn't find the 'Parameter' section
		errInvalidFormatLowInRows,	// Invalid CSV format: Didn't find parameter rows
		errInvalidFormatWaferId,	// Found non-numeric wafer ID
		errLicenceExpired,			// File date out of Licence window!
		errWriteSTDF				// Failed creating STDF intermediate file
	};

	enum eLimitType
	{
		eLowLimit,			// Flag to specify to save the WAT Parameter LOW limit
		eHighLimit			// Flag to specify to save the WAT Parameter HIGH limit
	};

	QString	m_strProductID;				// ProductID
	QString	m_strLotID;					// LotID
	QString	m_strProgramName;			// Program name
	QString	m_strTesterName;			// Tester name

	QString				m_strProcessID;				// ProcessID
	QString				m_strComment;				// Comment line about the manufacturing process
	long				m_lStartTime;				// Startup time
	bool				m_bNewWatParameterFound;	// set to true if WAT file has a Parameter name not in our reference table=> makes it to be updated
	CGJazzParameter *	m_pCGPcmParameter;			// List of Parameters tables.
	char				m_cDelimiter;
	int					m_iTotalParameters;			// Parameter results on a line
	QStringList			m_pFullWatParametersList;	// Complete list of ALL Wat parameters known.
};


#endif
