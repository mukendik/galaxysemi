#ifndef GEX_IMPORT_YOKOGAWA_H
#define GEX_IMPORT_YOKOGAWA_H

#include <qmap.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"

class CGYOKOGAWA_Part
{
public:
	CGYOKOGAWA_Part() {return;};
	
	bool	bValid;
	int		nPartNb;
	bool	bPass;
	int		nHBin;
	int		nSBin;
	int		nX;
	int		nY;
};


class CGYOKOGAWAtoSTDF
{
public:
	CGYOKOGAWAtoSTDF();
	~CGYOKOGAWAtoSTDF();
    bool		Convert(const char *YokogawaFileName, const char *strFileNameSTDF);
	QString		GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:
	void LoadParameterIndexTable(void);
	void DumpParameterIndexTable(void);
	void UpdateParameterIndexTable(QString strParamName);
	void NormalizeLimits(QString &strUnit, int &nScale);
    bool ReadYokogawaFile(const char *YokogawaFileName, const char *strFileNameSTDF);
	bool WriteStdfFile(QTextStream *hYokogawaFile,const char *strFileNameSTDF);

	QString ReadLine(QTextStream& hFile);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int	iProgressStep;
	int	iNextFilePos;
	int	iFileSize;

	QString m_strLastError;					// Holds last error string during YOKOGAWA->STDF convertion
	int		m_iLastError;					// Holds last error ID
	enum  errCodes
	{
		errNoError,							// No erro (default)
		errOpenFail,						// Failed Opening YOKOGAWA file
		errInvalidFormat,					// Invalid YOKOGAWA format
		errInvalidFormatLowInRows,			// Didn't find parameter rows
		errLicenceExpired,					// File date out of Licence window!
		errWriteSTDF						// Failed creating STDF intermediate file
	};

	QString		m_strTesterId;				// TesterID
	QString		m_strStationNumber;			// Station Number
	QString		m_strProberName;			// ProberName
	QString		m_strProgramName;			// Program Name
	QString		m_strDeviceName;			// Device Name
	QString		m_strWaferId;				// Wafer Id
	QString		m_strOperatorName;			// Operator Name
	QString		m_strLotId;					// LotID
	long		m_lStartTime;				// Startup time
	long		m_lEndTime;					// End time
	bool		m_bNewYokogawaParameterFound;	// set to true if YOKOGAWA file has a Parameter name not in our reference table=> makes it to be updated
	QStringList m_pFullYokogawaParametersList;	// Complete list of ALL Yokogawa parameters known.
};


#endif
