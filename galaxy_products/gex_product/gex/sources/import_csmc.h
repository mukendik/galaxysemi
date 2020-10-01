#ifndef GEX_IMPORT_CSMC_H
#define GEX_IMPORT_CSMC_H

#include <qmap.h>
#include <qdatetime.h>
#include <qstringlist.h>

#include "gex_constants.h"
#include "stdf.h"

typedef QMap<int, float> fValueOfSiteMap;

class CGCsmcParameter
{
public:
	QString			m_strName;					// Parameter name. E.g: "VT_N4 N1u"
	QString			m_strUnits;					// Parameter units,E.g: "V 10/.18"
	int				m_nScale;					// Parameter Scale
	float			m_fLowLimit;				// Parameter Low Spec limit, E.g: 4.452E-03
	float			m_fHighLimit;				// Parameter High Spec limit, E.g: 0.360
	bool			m_bValidLowLimit;			// Low limit defined
	bool			m_bValidHighLimit;			// High limit defined
    fValueOfSiteMap m_fValue;					// Parameter values for the XXX sites locations.
	bool			m_bStaticHeaderWritten;		// 'true' after first STDF PTR static header data written.
};

class CGCsmcWafer
{
public:

	~CGCsmcWafer();

	int				m_iWaferID;					// WaferID E.g: 6
	int				m_iLowestSiteID;			// Lowest SiteID found in CSMC file for this wafer
	int				m_iHighestSiteID;			// Highest SiteID found in CSMC file for this wafer
	QList<CGCsmcParameter*> m_pParameterList;	// List of Parameters in Wafer
};

class CGCSMCtoSTDF
{
public:
	CGCSMCtoSTDF();
	~CGCSMCtoSTDF();
	
    bool		Convert(const char *CsmcFileName, const char *strFileNameSTDF);
	QString		GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:

	void LoadParameterIndexTable(void);
	void DumpParameterIndexTable(void);
	void UpdateParameterIndexTable(QString strParamName);
	CGCsmcParameter *FindParameterEntry(int iWaferID,int iSiteID,QString strParamName,QString strUnits="");
	void SaveParameterResult(int iWaferID,int iSiteID,QString strName,QString strUnits,QString strValue);
	void SaveParameterLimit(QString strName,QString strValue,int iLimit);
	void NormalizeLimits(CGCsmcParameter *pParameter);
    bool ReadCsmcFile(const char *CsmcFileName);
	bool WriteStdfFile(const char *strFileNameSTDF);

	QString ReadLine(QTextStream& hFile);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int		iProgressStep;
	int		iFileSize;
	int		iNextFilePos;
	int		iNextParameter;
	int		iTotalParameters;

	QString m_strLastError;					// Holds last error string during CSMC->STDF convertion
	int		m_iLastError;					// Holds last error ID
	enum  errCodes
	{
		errNoError,							// No erro (default)
		errOpenFail,						// Failed Opening CSMC file
		errInvalidFormat,					// Invalid CSMC format
		errInvalidFormatLowInRows,			// Didn't find parameter rows
		errLicenceExpired,					// File date out of Licence window!
		errWriteSTDF						// Failed creating STDF intermediate file
	};
	enum eLimitType
	{
		eLowLimit,							// Flag to specify to save the CSMC Parameter LOW limit
		eHighLimit							// Flag to specify to save the CSMC Parameter HIGH limit
	};

	QString		m_strProductID;				// ProductID
	QString		m_strLotID;					// LotID
	QString		m_strOperatorID;			// ProcessID
	QString		m_strNodeName;				// ProcessID
	long		m_lStartTime;				// Startup time
	bool		m_bNewCsmcParameterFound;	// set to true if CSMC file has a Parameter name not in our reference table=> makes it to be updated
	QList<CGCsmcWafer*> m_pWaferList;		// List of Wafers in CSMC file
	QStringList m_pFullCsmcParametersList;	// Complete list of ALL Csmc parameters known.
};


#endif
