#ifndef GEX_IMPORT_LAURIER_DIE_SORT_H
#define GEX_IMPORT_LAURIER_DIE_SORT_H

#include <QDateTime>
#include <QStringList>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"

class CGLaurierDieSorttoSTDF
{
public:
	CGLaurierDieSorttoSTDF();
	~CGLaurierDieSorttoSTDF();
    bool	Convert(const char *LaurierDieSortFileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:

    bool ReadLaurierDieSortFile(const char *LaurierDieSortFileName, const char *strFileNameSTDF);
	bool WriteStdfFile(QTextStream *hLaurierDieSortFile,const char *strFileNameSTDF);

	QString ReadLine(QTextStream& hFile);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int	iProgressStep;
	int	iNextFilePos;
	int	iFileSize;

	QString m_strLastError;	// Holds last error string during LaurierDieSort->STDF convertion
	int		m_iLastError;	// Holds last error ID
	enum  errCodes
	{
		errNoError,			// No erro (default)
		errOpenFail,		// Failed Opening LaurierDieSort file
		errInvalidFormat,	// Invalid LaurierDieSort format
		errLicenceExpired,	// File date out of Licence window!
		errWriteSTDF		// Failed creating STDF intermediate file
	};

	unsigned int	m_lStartTime;
	QString			m_strDeviceId;			// Product Id
	QString			m_strJobRev;
	float			m_fDieSizeX;
	float			m_fDieSizeY;
	QString			m_strUserTxt;			// String to write into MIR.USER_TXT ("LAURIER VERSION")
};

#endif
