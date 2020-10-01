#ifndef GEX_IMPORT_PROBER_TSMC_H
#define GEX_IMPORT_PROBER_TSMC_H

#include <qdatetime.h>
#include <qstringlist.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"

class CGPROBER_TSMCtoSTDF
{
public:
	CGPROBER_TSMCtoSTDF();
	~CGPROBER_TSMCtoSTDF();
    bool	Convert(const char *Prober_TsmcFileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:

    bool ReadProber_TsmcFile(const char *Prober_TsmcFileName, const char *strFileNameSTDF);
	bool WriteStdfFile(QTextStream *hProber_TsmcFile,const char *strFileNameSTDF);

	QString ReadLine(QTextStream& hFile);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int	iProgressStep;
	int	iNextFilePos;
	int	iFileSize;

	QString m_strLastError;	// Holds last error string during PROBER_TSMC->STDF convertion
	int		m_iLastError;			// Holds last error ID
	enum  errCodes
	{
		errNoError,			// No erro (default)
		errOpenFail,		// Failed Opening PROBER_TSMC file
		errInvalidFormat,	// Invalid PROBER_TSMC format
		errLicenceExpired,	// File date out of Licence window!
		errWriteSTDF		// Failed creating STDF intermediate file
	};

	unsigned int	m_lStartTime;
	QString			m_strDeviceId;			//
	QString			m_strWaferId;			//
	QString			m_strLotId;				//
	int				m_nGoodBinCnt;
	int				m_nBadBinCnt;
	QString			m_strUserTxt;			// String to write into MIR.USER_TXT ("TSMC", "JAZZ")
};

#endif
