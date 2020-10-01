#ifndef GEX_IMPORT_TESSERA_EVG40_H
#define GEX_IMPORT_TESSERA_EVG40_H

#include <QDateTime>
#include <QMap>
#include <QStringList>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"



class CGTesseraEvg40toSTDF
{
public:
	CGTesseraEvg40toSTDF();
	~CGTesseraEvg40toSTDF();
    bool	Convert(const char *TesseraEvg40FileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:
    bool ReadTesseraEvg40File(const char *TesseraEvg40FileName,const char *strFileNameSTDF);
	bool WriteStdfFile(QTextStream *hTesseraEvg40File,const char *strFileNameSTDF);

	QString ReadLine(QTextStream& hFile);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int	iProgressStep;
	int	iNextFilePos;
	int	iFileSize;

	QString strLastError;	// Holds last error string during TesseraEvg40->STDF convertion
	int	iLastError;			// Holds last error ID
	enum  errCodes
	{
		errNoError,			// No erro (default)
		errOpenFail,		// Failed Opening TesseraEvg40 file
		errInvalidFormat,	// Invalid TesseraEvg40 format
		errInvalidFormatLowInRows,			// Didn't find parameter rows
		errInvalidFormatPitch,			// Didn't find pitch information
		errLicenceExpired,	// File date out of Licence window!
		errWriteSTDF		// Failed creating STDF intermediate file
	};

	QString	m_strSeparator;
	QString m_strUnit;

	long	m_lStartTime;
	long	m_lStopTime;
	QString	m_strLotID;
	QString	m_strOperatorID;
	QString	m_strTesterID;
	QString	m_strTesterType;
	QString	m_strProductID;
	QString	m_strWaferID;
	QString	m_strSetupId;				// Setup Id
	int		m_nWaferSize;


};


#endif
