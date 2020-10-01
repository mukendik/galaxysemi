#ifndef GEX_IMPORT_SPIL_WS_H
#define GEX_IMPORT_SPIL_WS_H

#include <qmap.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"

class CSpilWsBinInfo
{
public:
	CSpilWsBinInfo(int iBin);
	
	int		nBinNb;
	int		nNbCnt;
	bool	bPass;
	QString	strName;
};

class CGSpilWstoSTDF
{
public:
	CGSpilWstoSTDF();
	~CGSpilWstoSTDF();
    bool	Convert(const char *SpilWsFileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:
    bool ReadSpilWsFile(const char *SpilWsFileName, const char *strFileNameSTDF);
	bool WriteStdfFile(QTextStream *hSpilWsFile,const char *strFileNameSTDF);

	QString ReadLine(QTextStream& hFile);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int	iProgressStep;
	int	iNextFilePos;
	int	iFileSize;

	QString m_strLastError;	// Holds last error string during SpilWs->STDF convertion
	QString m_strLastErrorSpecification;
	int		m_iLastError;			// Holds last error ID
	enum  errCodes
	{
		errNoError,			// No erro (default)
		errOpenFail,		// Failed Opening SpilWs file
		errInvalidFormat,	// Invalid SpilWs format
		errLicenceExpired,	// File date out of Licence window!
		errWriteSTDF		// Failed creating STDF intermediate file
	};

	unsigned int	m_lStartTime;
	unsigned int	m_lEndTime;

	QString	m_strWaferId;			//WAFER_ID = "B83541.1-01-D6"

	int		m_nWaferColumns;		//COLUMNS = 47
	int		m_nFlatNotch;			//FLAT_NOTCH = 0
	QString	m_strDeviceId;			//DEVICE = "ADI"
	QString	m_strLotId;				//LOT_ID = "B83541.1"
	QString m_strTesterType;		//TESTER_TYPE
	QString m_strHandlerId;
	QString m_strOperatorId;
	QString m_strHandlerType;
	QString m_strJobName;
	QString m_strJobRev;
	QString m_strTemperature;
	QString m_strNodeName;

	int		m_nRefDieX;				//REF_DIE = 36 -2
	int		m_nRefDieY;				//REF_DIE = 36 -2
	int		m_nBinType;
	QString m_strNullBin;
	QMap<int, CSpilWsBinInfo*> m_qMapBins;
};

#endif
