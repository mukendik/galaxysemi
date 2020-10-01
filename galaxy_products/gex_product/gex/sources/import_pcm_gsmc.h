#ifndef GEX_IMPORT_PCM_GSMC_H
#define GEX_IMPORT_PCM_GSMC_H

#include <QDateTime>
#include <QMap>
#include <QStringList>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"


class CGPcmGsmcBinning
{
public:
	int		nNumber;	// Bin number
	QString strName;	// Bin name
	bool	bPass;		// Bin cat
	int		nCount;		// Bin count
};

class CGPcmGsmcParameter
{
public:
	int		nNumber;
	QString strName;				// Parameter name. E.g: "Idd_Total_Shutdown"
	QString strUnits;				// Parameter units,E.g: "A"
	int		nScale;
	float	fLowLimit;				// Parameter Low Spec limit, E.g: 0.00004
	float	fHighLimit;				// Parameter High Spec limit, E.g: -0.00004
	bool	bValidLowLimit;			// Low limit defined
	bool	bValidHighLimit;		// High limit defined
	float	fSpecLowLimit;			// Parameter Low Spec limit, E.g: 0.00004
	float	fSpecHighLimit;			// Parameter High Spec limit, E.g: -0.00004
	bool	bValidSpecLowLimit;		// Low limit defined
	bool	bValidSpecHighLimit;	// High limit defined
	bool	bStaticHeaderWritten;	// 'true' after first STDF PTR static header data written.
};




class CGPcmGsmctoSTDF
{
public:
	CGPcmGsmctoSTDF();
	~CGPcmGsmctoSTDF();
    bool	Convert(const char *PcmGsmcFileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:

    bool ReadPcmGsmcFile(const char *PcmGsmcFileName,const char *strFileNameSTDF);
	bool WriteStdfFile(QTextStream *hPcmGsmcFile,const char *strFileNameSTDF);
	void NormalizeLimits(QString &strUnit, int &nScale);

	QString ReadLine(QTextStream& hFile);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int	iProgressStep;
	int	iNextFilePos;
	int	iFileSize;

	// Need 2 passes
	int		m_nPass;

	QString strLastError;	// Holds last error string during PcmGsmc->STDF convertion
	int	iLastError;			// Holds last error ID
	enum  errCodes
	{
		errNoError,			// No erro (default)
		errOpenFail,		// Failed Opening PcmGsmc file
		errInvalidFormat,	// Invalid PcmGsmc format
		errInvalidFormatLowInRows,			// Didn't find parameter rows
		errNoLimitsFound,	// Missing limits...no a valid PcmGsmc file
		errLicenceExpired,	// File date out of Licence window!
		errWriteSTDF		// Failed creating STDF intermediate file
	};

	int									m_iTotalParameters;			// Holds the total number of parameters / tests in each part tested
	CGPcmGsmcParameter *				m_pPcmGsmcParameter;		// List of Parameters tables.
	QMap<QString,CGPcmGsmcParameter *>	m_mapNameParameter;			// List of Parameters map for limit.
	QMap<int,CGPcmGsmcBinning *>		m_mapBinning;				// List of Bin tables.

	long	m_lStartTime;
	int		m_nWaferID;
	QString	m_strLotID;
	QString	m_strFamilyID;
	QString	m_strPackageID;
	QString	m_strNodeName;
	QString	m_strPartType;
	QString m_strProcessId;
	QString m_strProbeCardId;
	QString m_strProgramId;
	QString	m_strOperatorName;
	QString	m_strAuxFile;
	QString	m_strSpecName;
	QString	m_strWaferFlat;

};


#endif
