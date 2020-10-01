#ifndef GEX_IMPORT_TSMC_WPR_H
#define GEX_IMPORT_TSMC_WPR_H

#include <qdatetime.h>
#include <qmap.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"


class CGTsmcWprtoSTDF
{
public:
	CGTsmcWprtoSTDF();
	~CGTsmcWprtoSTDF();
    bool	Convert(const char *TsmcWprFileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:

    bool ReadTsmcWprFile(const char *TsmcWprFileName, const char *strFileNameSTDF);
	bool WriteStdfFile(QTextStream *hTsmcWprFile,const char *strFileNameSTDF);

	QString ReadLine(QTextStream& hFile);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int	iProgressStep;
	int	iNextFilePos;
	int	iFileSize;

	QString m_strLastError;	// Holds last error string during TsmcWpr->STDF convertion
	int		m_iLastError;			// Holds last error ID
	enum  errCodes
	{
		errNoError,			// No erro (default)
		errOpenFail,		// Failed Opening TsmcWpr file
		errInvalidFormat,	// Invalid TsmcWpr format
		errLicenceExpired,	// File date out of Licence window!
		errWriteSTDF		// Failed creating STDF intermediate file
	};

	unsigned int	m_lStartTime;
	QString	m_strPartType;
	QString	m_strTesterType;
	QString	m_strPackageType;
	QString	m_strTestCode;
	QString	m_strLotId;
	QString	m_strWaferId;
};

#endif
