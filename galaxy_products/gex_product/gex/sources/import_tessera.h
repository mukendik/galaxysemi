#ifndef GEX_IMPORT_TESSERA_H
#define GEX_IMPORT_TESSERA_H

#include <QMap>
#include <QList>
#include <QDateTime>
#include <QStringList>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"

class CGTesseratoSTDF
{
public:
	CGTesseratoSTDF();
	~CGTesseratoSTDF();
    bool	Convert(const char *TesseraFileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:
	void NormalizeLimits(QString &strUnit, int &nScale);
    bool ReadTesseraFile(const char *TesseraFileName, const char *strFileNameSTDF);
	bool WriteStdfFile(QTextStream& hFile,const char *strFileNameSTDF);


	QString ReadLine(QTextStream& hTesseraFile);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int	iProgressStep;
	int	iNextFilePos;
	int	iFileSize;


	QString m_strLastError;					// Holds last error string during Tessera->STDF convertion
	int		m_iLastError;					// Holds last error ID
	enum  errCodes
	{
		errNoError,							// No erro (default)
		errOpenFail,						// Failed Opening Tessera file
		errInvalidFormat,					// Invalid Tessera format
		errLicenceExpired,					// File date out of Licence window!
		errWriteSTDF						// Failed creating STDF intermediate file
	};

	QString		m_strJobName;				// Job Name
	QString		m_strJobRev;				// Job revision
	QString		m_strSoftRev;				// Soft revision
	QString		m_strLotId;					// LotID
	QString		m_strProductId;				// ProductID
	QString		m_strFrequency;				// Operation Frequency
	QString		m_strOperatorId;			// OperatorID
	QString		m_strComputerId;			// ComputerID
	QString		m_strTesterType;			// TesterType
	QString		m_strAuxFile;				// Aux file
	QString		m_strSetupId;				// Setup Id
	long		m_lStartTime;				// Startup time

	QStringList m_lstTest;
	int			m_nTestPitch;
};


#endif
