#ifndef GEX_IMPORT_PCM_HYNIX_H
#define GEX_IMPORT_PCM_HYNIX_H

#include <qmap.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"

typedef QMap<int, float> fValueOfSiteMap;

class CGPcmHynixParameter
{
public:
	QString strName;	// Parameter name. E.g: "VT_N4 N1u"
	QString strUnits;	// Parameter units,E.g: "V 10/.18"
	bool	bLowLimit;	// have low limit
	float	fLowLimit;	// Parameter Low Spec limit, E.g: 4.452E-03
	bool	bHighLimit;	// have high limit
	float	fHighLimit;	// Parameter High Spec limit, E.g: 0.360
    fValueOfSiteMap fValue;	// Parameter values for the XXX sites locations.
	bool	bStaticHeaderWritten;	// 'true' after first STDF PTR static header data written.
};

class CGPcmHynixWafer
{
public:

	~CGPcmHynixWafer();
	
	int		iWaferID;							// WaferID E.g: 6
	int		iLowestSiteID;						// Lowest SiteID found in PCM file for this wafer
	int		iHighestSiteID;						// Highest SiteID found in PCM file for this wafer
	int		iNbParts;							// Nb parts for this wafer
	int		iNbGood;							// Nb good parts for this wafer
	QList <CGPcmHynixParameter*> pParameterList;	// List of Parameters in Wafer
};

class CGPCM_Hynix_toSTDF
{
public:
	CGPCM_Hynix_toSTDF();
	~CGPCM_Hynix_toSTDF();
	
    bool	Convert(const char *PcmFileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:

	void LoadParameterIndexTable(void);
	void DumpParameterIndexTable(void);
	void UpdateParameterIndexTable(QString strParamName);
	void UpdateWaferInfo(int iWaferID,int iTotalSites,int iTotalGood);
	CGPcmHynixParameter *FindParameterEntry(int iWaferID,int iSiteID,QString strParamName,QString strUnits="");
	void SaveParameterResult(int iWaferID,int iSiteID,QString strName,QString strUnits,QString strValue);
	void SaveParameterLimit(QString strName,QString strValue,int iLimit);
    bool ReadPcmFile(const char *PcmFileName);
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



	QString m_strLastError;	// Holds last error string during PCM->STDF convertion
	int		m_iLastError;	// Holds last error ID
	enum  errCodes
	{
		errNoError,			// No erro (default)
		errOpenFail,		// Failed Opening PCM file
		errInvalidFormat,	// Invalid PCM format
		errLicenceExpired,	// File date out of Licence window!
		errWriteSTDF		// Failed creating STDF intermediate file
	};
	enum eLimitType
	{
		eLowLimit,			// Flag to specify to save the PCM Parameter LOW limit
		eHighLimit			// Flag to specify to save the PCM Parameter HIGH limit
	};

	QString				m_strProductID;				// ProductID
	QString				m_strLotID;					// LotID
	QString				m_strProcessID;				// ProcessID
	long				m_lStartTime;					// Startup time
	bool				m_bNewPcmParameterFound;		// set to true if PCM file has a Parameter name not in our reference table=> makes it to be updated
	QList <CGPcmHynixWafer*> m_pWaferList;		// List of Wafers in PCM file
	QStringList			m_pFullPcmParametersList;	// Complete list of ALL Pcm parameters known.
};


#endif
