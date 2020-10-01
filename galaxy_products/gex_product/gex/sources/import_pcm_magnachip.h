#ifndef GEX_IMPORT_PCM_MAGNACHIP_H
#define GEX_IMPORT_PCM_MAGNACHIP_H

#include <qdatetime.h>
#include <qstringlist.h>

#include "gex_constants.h"
#include "stdf.h"
#include "import_wat.h"

class CGPCM_KDFtoSTDF
{
public:
	CGPCM_KDFtoSTDF();
	~CGPCM_KDFtoSTDF();

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

	QString	m_strProductID;				// ProductID
	QString	m_strLotID;					// LotID
	QString	m_strProcessID;				// ProcessID
	QString	m_strComment;				// Comment line about the manufacturing process
	long	m_lStartTime;				// Startup time
	bool	m_bNewWatParameterFound;	// set to true if WAT file has a Parameter name not in our reference table=> makes it to be updated
	QStringList	m_pFullWatParametersList;// Complete list of ALL Wat parameters known.
	QList <CGWatWafer*> m_pWaferList;		// List of Wafers in WAT file
};


#endif
