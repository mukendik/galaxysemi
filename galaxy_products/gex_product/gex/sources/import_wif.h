#ifndef GEX_IMPORT_WIF_H
#define GEX_IMPORT_WIF_H

#include <QDateTime>
#include <QMap>
#include <QStringList>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"


class CGWifBinning
{
public:
	int		nNumber;	// Bin number
	QString strName;	// Bin name
	bool	bPass;		// Bin cat
	int		nCount;		// Bin count
};

class CGWifParameter
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




class CGWiftoSTDF
{
public:
	CGWiftoSTDF();
	~CGWiftoSTDF();
    bool	Convert(const char *WifFileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:
	void LoadParameterIndexTable(void);
	void DumpParameterIndexTable(void);
	void UpdateParameterIndexTable(QString strParamName);
    bool ReadWifFile(const char *WifFileName, const char *strFileNameSTDF);
	bool WriteStdfFile(QTextStream *hWifFile,const char *strFileNameSTDF);
	void NormalizeValues(QString &strUnits,float &fValue, int&nScale, bool &bIsNumber);

	QString ReadLine(QTextStream& hFile);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int	iProgressStep;
	int	iNextFilePos;
	int	iFileSize;

	QString strLastError;	// Holds last error string during Wif->STDF convertion
	int	iLastError;			// Holds last error ID
	enum  errCodes
	{
		errNoError,			// No erro (default)
		errOpenFail,		// Failed Opening Wif file
		errInvalidFormat,	// Invalid Wif format
		errInvalidFormatLowInRows,			// Didn't find parameter rows
		errNoLimitsFound,	// Missing limits...no a valid Wif file
		errLicenceExpired,	// File date out of Licence window!
		errWriteSTDF		// Failed creating STDF intermediate file
	};

	bool					m_bNewWifParameterFound;	// set to true if Wif file has a Parameter name not in our reference table=> makes it to be updated
	int						m_iTotalParameters;			// Holds the total number of parameters / tests in each part tested
	QStringList				m_pFullWifParametersList;	// Complete list of ALL Wif parameters known.
	CGWifParameter *		m_pWifParameter;			// List of Parameters tables.
	QMap<int,CGWifBinning>	m_mapWifBinning;		// List of Bin tables.

	long	m_lStartTime;
	long	m_lStopTime;
	QString m_strSoftRev;
	QString	m_strProductID;
	QString	m_strLotID;
	QString	m_strWaferID;
	QString	m_strSideID;
	QString	m_strSlotID;
	QString	m_strOperatorID;
	QString	m_strProgramID;
	QString m_strTestCode;
	QString	m_strTesterID;
	QString	m_strTesterType;
	QString	m_strSetupId;				// Setup Id
	QString	m_strJobName;				// Job Name
	QString	m_strJobRev;				// Job revision
	QString m_strAuxFile;

	int		m_iIndexOffset;
	int		m_nPassFailIndex;

};


#endif
