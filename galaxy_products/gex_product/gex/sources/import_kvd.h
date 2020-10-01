#ifndef GEX_IMPORT_KVD_H
#define GEX_IMPORT_KVD_H

#include <qmap.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"

class CKVDBinInfo
{
public:
	QMap <int, int>			m_nCount;					// Bin count for the XXX sites locations.
	bool	bPass;
};


class CGKVDtoSTDF
{
public:
	CGKVDtoSTDF();
    bool	Convert(const char *KVDFileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:

	long GetDateTimeFromString(QString strDateTime);
	void NormalizeValues(QString &strUnits,float &fValue, int&nScale);
    bool ReadKVDFile(const char *KVDFileName, const char *strFileNameSTDF);
	bool WriteStdfFile(QTextStream &hKVDFile,const char *strFileNameSTDF);

	QString ReadLine(QTextStream& hFile);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int		iProgressStep;
	int		iFileSize;
	int		iNextFilePos;


	QString m_strLastError;					// Holds last error string during KVD->STDF convertion
	int		m_iLastError;					// Holds last error ID
	enum  errCodes
	{
		errNoError,							// No erro (default)
		errOpenFail,						// Failed Opening KVD file
		errInvalidFormat,					// Invalid KVD format
		errLicenceExpired,					// File date out of Licence window!
		errWriteSTDF						// Failed creating STDF intermediate file
	};
	enum eLimitType
	{
		eLowLimit,							// Flag to specify to save the KVD Parameter LOW limit
		eHighLimit							// Flag to specify to save the KVD Parameter HIGH limit
	};

	QString		m_strWaferId;				// WaferID
	QString		m_strTesterId;				// TesterID
	QString		m_strLotId;					// LotID
	QString		m_strOperatorId;			// OperatorID
	QString		m_strProgramId;				// ProgramID
	QString		m_strExecType;				// Executive Software Type
	long		m_lStartTime;				// Startup time

	QMap<int, CKVDBinInfo>	m_mapBinsCount;		// Bin stats.
	QStringList m_lstParametersStaticHeaderWritten;			// list of parameters known.
	
};


#endif
