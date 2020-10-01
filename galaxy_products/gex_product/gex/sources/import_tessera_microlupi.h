#ifndef GEX_IMPORT_TESSERA_MICROLUPI_H
#define GEX_IMPORT_TESSERA_MICROLUPI_H

#include <QDateTime>
#include <QMap>
#include <QStringList>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"


class CGTesseraMicrolupiParameter
{
public:
	int		nNumber;
	QString strName;	// Parameter name. E.g: "Idd_Total_Shutdown"
	QString strUnits;	// Parameter units,E.g: "A"
	int		nScale;
	float	fLowLimit;	// Parameter Low Spec limit, E.g: 0.00004
	float	fHighLimit;	// Parameter High Spec limit, E.g: -0.00004
	bool	bValidLowLimit;		// Low limit defined
	bool	bValidHighLimit;	// High limit defined
    float	fValue;		// Parameter result
	bool	bStaticHeaderWritten;	// 'true' after first STDF PTR static header data written.
};




class CGTesseraMicrolupitoSTDF
{
public:
	CGTesseraMicrolupitoSTDF();
	~CGTesseraMicrolupitoSTDF();
    bool	Convert(const char *TesseraMicrolupiFileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:
	void LoadParameterIndexTable(void);
	void DumpParameterIndexTable(void);
	void UpdateParameterIndexTable(QString strParamName);
    bool ReadTesseraMicrolupiFile(const char *TesseraMicrolupiFileName,const char *strFileNameSTDF);
	bool WriteStdfFile(QTextStream *hTesseraMicrolupiFile,const char *strFileNameSTDF);
	void NormalizeLimits(QString &strUnit, int &nScale);

	QString ReadLine(QTextStream& hFile);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int	iProgressStep;
	int	iNextFilePos;
	int	iFileSize;

	QString strLastError;	// Holds last error string during TesseraMicrolupi->STDF convertion
	int	iLastError;			// Holds last error ID
	enum  errCodes
	{
		errNoError,			// No erro (default)
		errOpenFail,		// Failed Opening TesseraMicrolupi file
		errInvalidFormat,	// Invalid TesseraMicrolupi format
		errInvalidFormatLowInRows,			// Didn't find parameter rows
		errLicenceExpired,	// File date out of Licence window!
		errWriteSTDF		// Failed creating STDF intermediate file
	};

	bool					m_bNewTesseraMicrolupiParameterFound;	// set to true if TesseraMicrolupi file has a Parameter name not in our reference table=> makes it to be updated
	QStringList				m_pFullTesseraMicrolupiParametersList;	// Complete list of ALL TesseraMicrolupi parameters known.
	QMap<QString,CGTesseraMicrolupiParameter *>	m_mapTesseraMicrolupiParameter;		// List of Parameters tables.

	long	m_lStartTime;				//~Start Date/Time,27/01/2009 10:53:46
	long	m_lStopTime;				//~End Date/Time,27/01/2009 11:00:41
	QString m_strSoftVer;
	QString	m_strProductID;
	QString	m_strLotID;
	QString	m_strWaferID;
	QString	m_strSideID;
	QString	m_strSlotID;
	QString	m_strOperatorID;
	QString	m_strProgramID;
	QString	m_strTesterID;
	QString	m_strTesterType;
	QString	m_strSetupId;				// Setup Id
	QString	m_strJobName;				// Job Name
	QString	m_strJobRev;				// Job revision


	int m_iRowOffset;
	int	m_iColOffset;
	int m_iRowMult;
	int m_iColMult;

};


#endif
