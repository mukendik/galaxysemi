#ifndef GEX_IMPORT_WAT_ASMC_H
#define GEX_IMPORT_WAT_ASMC_H

#include <qmap.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"

typedef QMap<int, float> fValueOfSiteMap;

class CGWatAsmcParameter
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

class CGWatAsmcWafer
{
public:

	~CGWatAsmcWafer();

	int				m_iWaferID;					// WaferID E.g: 6
	int				m_iLowestSiteID;			// Lowest SiteID found in WAT_ASMC file for this wafer
	int				m_iHighestSiteID;			// Highest SiteID found in WAT_ASMC file for this wafer
	QList <CGWatAsmcParameter*> m_pParameterList;	// List of Parameters in Wafer
};

class CGWAT_ASMCtoSTDF
{
public:
	CGWAT_ASMCtoSTDF();
	~CGWAT_ASMCtoSTDF();

	bool	Convert(const char *WatAsmcFileName, const char *strFileNameSTDF,QDate *pExpirationDate=NULL);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:
	bool CheckValidityDate(QDate *pExpirationDate);
	void LoadParameterIndexTable(void);
	void DumpParameterIndexTable(void);
	void UpdateParameterIndexTable(QString strParamName);
	CGWatAsmcParameter *FindParameterEntry(int iWaferID,int iSiteID,QString strParamName,QString strUnits="");
	void SaveParameterResult(int iWaferID,int iSiteID,QString strName,QString strUnits,QString strValue);
	void SaveParameterLimit(QString strName,QString strUnits,QString strValue,int iLimit);
	void NormalizeLimits(CGWatAsmcParameter *pParameter);
	bool ReadWatAsmcFile(const char *WatAsmcFileName,QDate *pExpirationDate);
	bool WriteStdfFile(const char *strFileNameSTDF);

	QString ReadLine(QTextStream& hFile);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int		iProgressStep;
	int		iFileSize;
	int		iNextFilePos;
	int		iNextParameter;
	int		iTotalParameters;
	int		iParameterCount;

	QString m_strLastError;					// Holds last error string during WAT_ASMC->STDF convertion
	int		m_iLastError;					// Holds last error ID
	enum  errCodes
	{
		errNoError,							// No erro (default)
		errOpenFail,						// Failed Opening WAT_ASMC file
		errInvalidFormat,					// Invalid WAT_ASMC format
		errLicenceExpired,					// File date out of Licence window!
		errWriteSTDF						// Failed creating STDF intermediate file
	};
	enum eLimitType
	{
		eLowLimit,							// Flag to specify to save the WAT_ASMC Parameter LOW limit
		eHighLimit							// Flag to specify to save the WAT_ASMC Parameter HIGH limit
	};

	QString		m_strProductId;				// ProductID
	QString		m_strLotId;					// LotID
	QString		m_strProcessId;				// ProcessID
	QString		m_strTestCode;				// Step code
	QString		m_strTesterType;			// TesterType
	long		m_lStartTime;				// Startup time
	bool		m_bNewWatAsmcParameterFound;	// set to true if WAT_ASMC file has a Parameter name not in our reference table=> makes it to be updated
	QList<CGWatAsmcWafer*> m_pWaferList;		// List of Wafers in WAT_ASMC file
	QStringList m_pFullWatAsmcParametersList;	// Complete list of ALL WatAsmc parameters known.
};


#endif
