#ifndef GEX_IMPORT_TESSERA_DIFFRACTIVE_H
#define GEX_IMPORT_TESSERA_DIFFRACTIVE_H

#include <QDateTime>
#include <QMap>
#include <QStringList>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"



class CGTesseraDiffractivetoSTDF
{
public:
	CGTesseraDiffractivetoSTDF();
	~CGTesseraDiffractivetoSTDF();
    bool	Convert(const char *TesseraDiffractiveFileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:
    bool ReadTesseraDiffractiveFile(const char *TesseraDiffractiveFileName,const char *strFileNameSTDF);
	bool WriteStdfFile(QTextStream *hTesseraDiffractiveFile,const char *strFileNameSTDF);

	QString ReadLine(QTextStream& hFile);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int	iProgressStep;
	int	iNextFilePos;
	int	iFileSize;

	// Need 2 passes
	// Find XMin, YMin
	// XPitch, YPitch
	int		m_nPass;
	float	m_fXMin;
	float	m_fYMin;
	float	m_fXMax;
	float	m_fYMax;
	float	m_fXPitch;
	float	m_fYPitch;


	QString strLastError;	// Holds last error string during TesseraDiffractive->STDF convertion
	int	iLastError;			// Holds last error ID
	enum  errCodes
	{
		errNoError,			// No erro (default)
		errOpenFail,		// Failed Opening TesseraDiffractive file
		errInvalidFormat,	// Invalid TesseraDiffractive format
		errInvalidFormatLowInRows,			// Didn't find parameter rows
		errInvalidFormatPitch,			// Didn't find pitch information
		errLicenceExpired,	// File date out of Licence window!
		errWriteSTDF		// Failed creating STDF intermediate file
	};

	long	m_lStartTime;
	long	m_lStopTime;
	QString	m_strProductID;
	QString	m_strLotID;
	QString	m_strWaferID;
	QString	m_strSideID;
	QString	m_strSlotID;
	QString	m_strOperatorID;
	QString	m_strJobName;				// Job Name
	QString	m_strJobRev;				// Job revision
	QString m_strProcessID;
	QString	m_strTesterID;
	QString	m_strTesterType;
	QString	m_strSetupId;				// Setup Id

};


#endif
