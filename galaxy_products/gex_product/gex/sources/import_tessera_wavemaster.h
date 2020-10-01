#ifndef GEX_IMPORT_TESSERA_WAVEMASTER_H
#define GEX_IMPORT_TESSERA_WAVEMASTER_H

#include <QMap>
#include <QList>
#include <QDateTime>
#include <QStringList>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"

class CGTesseraWavemastertoSTDF
{
public:
	CGTesseraWavemastertoSTDF();
	~CGTesseraWavemastertoSTDF();
    bool	Convert(const char *TesseraWavemasterFileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:

	void NormalizeLimits(QString &strUnit, int &nScale);
    bool ReadTesseraWavemasterFile(const char *TesseraWavemasterFileName, const char *strFileNameSTDF);
	bool WriteStdfFile(QTextStream& hFile,const char *strFileNameSTDF);


	QString ReadLine(QTextStream& hTesseraWavemasterFile);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int	iProgressStep;
	int	iNextFilePos;
	int	iFileSize;


	QString m_strLastError;					// Holds last error string during TesseraWavemaster->STDF convertion
	int		m_iLastError;					// Holds last error ID
	enum  errCodes
	{
		errNoError,							// No erro (default)
		errOpenFail,						// Failed Opening TesseraWavemaster file
		errInvalidFormat,					// Invalid TesseraWavemaster format
		errLicenceExpired,					// File date out of Licence window!
		errWriteSTDF						// Failed creating STDF intermediate file
	};

	QString		m_strLotId;					// LotID
	QString		m_strWaferId;				// WaferID
	QString		m_strProgramId;				// ProgramID
	QString		m_strOperatorId;			// OperatorID
	QString		m_strComputerId;			// ComputerID
	QString		m_strTesterType;			// TesterType
	QString		m_strSetupId;				// Setup Id
	QString		m_strJobName;				// Job Name
	QString		m_strJobRev;				// Job revision

	long		m_lStartTime;				// Startup time

	QStringList m_lstTest;
	int			m_nLinearIndex;
};


#endif
