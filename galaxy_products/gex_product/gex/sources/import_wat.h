#ifndef GEX_IMPORT_WAT_H
#define GEX_IMPORT_WAT_H

#include <qmap.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"

typedef QMap<int, float> fValueOfSiteMap;

class CGWatParameter
{
public:
	QString strName;	// Parameter name. E.g: "VT_N4 N1u"
	QString strUnits;	// Parameter units,E.g: "V 10/.18"
	float	fLowLimit;	// Parameter Low Spec limit, E.g: 4.452E-03
	float	fHighLimit;	// Parameter High Spec limit, E.g: 0.360
	bool	bValidLowLimit;		// Low limit defined
	bool	bValidHighLimit;	// High limit defined
    fValueOfSiteMap fValue;	// Parameter values for the XXX sites locations.
	bool	bStaticHeaderWritten;	// 'true' after first STDF PTR static header data written.
};

class CGWatWafer
{
public:
	
	~CGWatWafer();
	
	int		iWaferID;						// WaferID E.g: 6
	int		iLowestSiteID;					// Lowest SiteID found in WAT file for this wafer
	int		iHighestSiteID;					// Highest SiteID found in WAT file for this wafer
	QList<CGWatParameter*> pParameterList;	// List of Parameters in Wafer
};

class CGWATtoSTDF
{
public:
	CGWATtoSTDF();
	~CGWATtoSTDF();

    bool	Convert(const char *WatFileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:

	void LoadParameterIndexTable(void);
	void DumpParameterIndexTable(void);
	void UpdateParameterIndexTable(QString strParamName);
	CGWatParameter *FindParameterEntry(int iWaferID,int iSiteID,QString strParamName,QString strUnits="");
	void SaveParameterResult(int iWaferID,int iSiteID,QString strName,QString strUnits,QString strValue);
	void SaveParameterLimit(QString strName,QString strValue,int iLimit);
    bool ReadWatFile(const char *WatFileName);
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

	QString m_strLastError;	// Holds last error string during WAT->STDF convertion
	int		m_iLastError;	// Holds last error ID
	enum  errCodes
	{
		errNoError,			// No erro (default)
		errOpenFail,		// Failed Opening WAT file
		errInvalidFormat,	// Invalid WAT format
		errLicenceExpired,	// File date out of Licence window!
		errWriteSTDF		// Failed creating STDF intermediate file
	};
	enum eLimitType
	{
		eLowLimit,			// Flag to specify to save the WAT Parameter LOW limit
		eHighLimit			// Flag to specify to save the WAT Parameter HIGH limit
	};

	QString	m_strProductID;					// ProductID
	QString	m_strLotID;						// LotID
	QString	m_strProcessID;					// ProcessID
	QString	m_strSpecID;					// SpecID
	long	m_lStartTime;					// Startup time
	bool	m_bNewWatParameterFound;		// set to true if WAT file has a Parameter name not in our reference table=> makes it to be updated
	QList<CGWatWafer*> m_pWaferList;		// List of Wafers in WAT file
	QStringList m_pFullWatParametersList;	// Complete list of ALL Wat parameters known.
};


#endif
