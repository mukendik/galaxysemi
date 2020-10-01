#ifndef GEX_IMPORT_PCM_H
#define GEX_IMPORT_PCM_H

#include <qdatetime.h>
#include <qmap.h>
#include <qstringlist.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"

typedef QMap<int, float> fValueOfSiteMap;

class CGPcmParameter
{
public:
	QString strName;	// Parameter name. E.g: "Idd_Total_Shutdown"
	QString strUnits;	// Parameter units,E.g: "A"
	float	fLowLimit;	// Parameter Low Spec limit, E.g: 0.00004
	float	fHighLimit;	// Parameter High Spec limit, E.g: -0.00004
	bool	bValidLowLimit;		// Low limit defined
	bool	bValidHighLimit;	// High limit defined
    float	fValue;		// Parameter result
	bool	bStaticHeaderWritten;	// 'true' after first STDF PTR static header data written.
};


class CGPCMtoSTDF
{
public:
	CGPCMtoSTDF();
	~CGPCMtoSTDF();
    bool	Convert(const char *PcmFileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:

	void LoadParameterIndexTable(void);
	void DumpParameterIndexTable(void);
	void UpdateParameterIndexTable(QString strParamName);
    bool ReadPcmFile(const char *PcmFileName, const char *strFileNameSTDF);
	bool WriteStdfFile(QTextStream *hPcmFile,const char *strFileNameSTDF);

	QString ReadLine(QTextStream& hFile);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int	iProgressStep;
	int	iNextFilePos;
	int	iFileSize;


	QString m_strLastError;		// Holds last error string during PCM->STDF convertion
	int		m_iLastError;			// Holds last error ID
	enum  errCodes
	{
		errNoError,			// No erro (default)
		errOpenFail,		// Failed Opening PCM file
		errInvalidFormat,	// Invalid PCM format
		errInvalidFormatLowInRows,			// Didn't find parameter rows
		errNoLimitsFound,	// Missing limits...no a valid PCM file
		errLicenceExpired,	// File date out of Licence window!
		errWriteSTDF		// Failed creating STDF intermediate file
	};

	bool			m_bNewPcmParameterFound;	// set to true if PCM file has a Parameter name not in our reference table=> makes it to be updated
	CGPcmParameter *m_pCGPcmParameter;			// List of Parameters tables.
	int				m_iTotalParameters;			// Holds the total number of parameters / tests in each part tested
	QStringList		m_pFullPcmParametersList;	// Complete list of ALL PCM parameters known.
	int				m_iParametersOffset;
	int				m_iSitePos;
	int				m_iBackSidePos;

	long			m_lStartTime;				// Startup time
	QString			m_strLotID;					// LotID string
	QString			m_strProductID;				// Product / Device name
	QString			m_strProcessID;				// ProcessID
};


#endif
