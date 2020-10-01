#ifndef GEX_IMPORT_CSM_H
#define GEX_IMPORT_CSM_H

#include <qmap.h>
#include <qdatetime.h>
#include <qstringlist.h>

#include "gex_constants.h"
#include "stdf.h"

typedef QMap<int, float> fValueOfSiteMap;

class CGCsmParameter
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

class CGCsmWafer
{
public:
	
	~CGCsmWafer();
	int				m_iWaferID;					// WaferID E.g: 6
	int				m_iLowestSiteID;			// Lowest SiteID found in CSM file for this wafer
	int				m_iHighestSiteID;			// Highest SiteID found in CSM file for this wafer
	QList<CGCsmParameter*> m_pParameterList;		// List of Parameters in Wafer
};

class CGCSMtoSTDF
{
public:
	CGCSMtoSTDF();
	~CGCSMtoSTDF();

    bool	Convert(const char *CsmFileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:

	void LoadParameterIndexTable(void);
	void DumpParameterIndexTable(void);
	void UpdateParameterIndexTable(QString strParamName);
	CGCsmParameter *FindParameterEntry(int iWaferID,int iSiteID,QString strParamName,QString strUnits="");
	void SaveParameterResult(int iWaferID,int iSiteID,QString strName,QString strUnits,QString strValue);
	void SaveParameterLimit(QString strName,QString strValue,int iLimit);
	void NormalizeLimits(CGCsmParameter *pParameter);
    bool ReadCsmFile(const char *CsmFileName);
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


	QString m_strLastError;					// Holds last error string during CSM->STDF convertion
	int		m_iLastError;					// Holds last error ID
	enum  errCodes
	{
		errNoError,							// No erro (default)
		errOpenFail,						// Failed Opening CSM file
		errInvalidFormat,					// Invalid CSM format
		errLicenceExpired,					// File date out of Licence window!
		errWriteSTDF						// Failed creating STDF intermediate file
	};
	enum eLimitType
	{
		eLowLimit,							// Flag to specify to save the CSM Parameter LOW limit
		eHighLimit							// Flag to specify to save the CSM Parameter HIGH limit
	};

	QString		m_strProductID;				// ProductID
	QString		m_strLotID;					// LotID
	QString		m_strProcessID;				// ProcessID
	QString		m_strJobName;				// ProcessID
	long		m_lStartTime;				// Startup time
	bool		m_bNewCsmParameterFound;	// set to true if CSM file has a Parameter name not in our reference table=> makes it to be updated
	QList <CGCsmWafer*> m_pWaferList;		// List of Wafers in CSM file
	QStringList m_pFullCsmParametersList;	// Complete list of ALL Csm parameters known.
	QStringList	m_lstSites;					// Always the same for all the file
};


#endif
