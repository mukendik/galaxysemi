#ifndef GEX_IMPORT_PROBER_TSK_H
#define GEX_IMPORT_PROBER_TSK_H

#include <qmap.h>
#include <qdatetime.h>
#include <qstringlist.h>

#include "gex_constants.h"
#include "stdf.h"

#define PROBER_TSK_BLOCK_SIZE	1536


class CGProberTskBinInfo
{
public:
	QString strBinName;
	int		nBinNumber;
	bool	bPass;
	int		nNbCnt;
};




class CGProberTsktoSTDF
{
public:
	enum eConvertStatus
	{
		eConvertSuccess,		// Conversion successful
		eConvertDelay,			// Delay conversion
		eConvertError			// Conversion failed
	};
	enum  errCodes
	{
		errNoError,							// No erro (default)
		errWarning,							// File generated with warning
		errOpenFail,						// Failed Opening ProberTsk file
		errConversionFailed,				// Not compatible
		errLicenceExpired,					// File date out of Licence window!
		errUnsupportedMapVerion,			// Only some Map version are allowed
		errWriteSTDF						// Failed creating STDF intermediate file
	};

	CGProberTsktoSTDF();
	~CGProberTsktoSTDF();
    int			Convert(const char *ProberTskFileName, QString &strFileNameSTDF);
	QString		GetLastError();
    int			GetLastErrorCode() {return m_iLastError;}

	static bool	IsCompatible(const char *szFileName);

private:

    int		ReadProberTskFile(const char *strFileNameSTDF);
	int		ReadHeaderInformationRecord(QFile &f);
	int		ReadHeaderInformationExtensionRecord(QFile &f);
	int		ReadTestResultExtensionRecord(QFile &f);
	int		ReadLineCategoryExtensionRecord(QFile &f);
	int		WriteStdfFile(QFile &f, const char *strFileNameSTDF);
	QString	GetDataMapVersion();

	int		ReadBlock(QFile* pFile, char *data, qint64 len);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int	iProgressStep;
	int	iNextFilePos;
	int iFileLoaded;
	int	iFileSize;

	QString m_strLastError;					// Holds last error string during ProberTsk->STDF convertion
	int		m_iLastError;					// Holds last error ID

	QString		m_strProberTskFileName;
	QString		m_strOperatorName;
	QString		m_strProductName;
	QString		m_strMachineId;

	QString		m_strWaferId;
	QString		m_strRtstCod;
	QString		m_strLotId;
	QString		m_strProductId;
	QString		m_strSubLotId;


	QString		m_strDataFilePath;
	int			m_iDataMapVersion;
	int			m_iMapFileConfiguration;
	int			m_iTestResultAddress;
	int			m_iLineCategorySize;
	int			m_iLineCategoryAddress;

	long		m_lStartTime;				// Startup time (seconds)
	long		m_lEndTime;					// End time	(seconds)
	
	float		m_fWaferSize;
	float		m_fWaferDieSizeX;
	float		m_fWaferDieSizeY;
	int			m_iWaferUnit;
	int			m_iWaferRowSize;
	int			m_iWaferLineSize;

	QString		m_strWaferFlat;


	int			m_iTotalDiesGood;
	int			m_iTotalDiesFail;
	int			m_iTotalDies;

	QList<int>	m_lstExtendedTestResult;
	int			m_nPartNumber;
	int			m_nXLocation;
	int			m_nYLocation;
	int			m_nHardBin;
	int			m_nSoftBin;
	int			m_nPassStatus;

};


#endif
