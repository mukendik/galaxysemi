#ifndef GEX_IMPORT_TESSERA_QV302_H
#define GEX_IMPORT_TESSERA_QV302_H

#include <QDateTime>
#include <QMap>
#include <QStringList>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"


class CGTesseraQV302Binning
{
public:
	int		nNumber;	// Bin number
	QString strName;	// Bin name
	bool	bPass;		// Bin cat
	int		nCount;		// Bin count
};

class CGTesseraQV302Parameter
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




class CGTesseraQV302toSTDF
{
public:
	CGTesseraQV302toSTDF();
	~CGTesseraQV302toSTDF();
    bool	Convert(const char *TesseraQV302FileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:

	void LoadParameterIndexTable(void);
	void DumpParameterIndexTable(void);
	void UpdateParameterIndexTable(QString strParamName);
    bool ReadTesseraQV302File(const char *TesseraQV302FileName,const char *strFileNameSTDF);
	bool WriteStdfFile(QTextStream *hTesseraQV302File,const char *strFileNameSTDF);
	void NormalizeLimits(QString &strUnit, int &nScale);

	QString ReadLine(QTextStream& hFile);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int	iProgressStep;
	int	iNextFilePos;
	int	iFileSize;

	QString strLastError;	// Holds last error string during TesseraQV302->STDF convertion
	int	iLastError;			// Holds last error ID
	enum  errCodes
	{
		errNoError,			// No erro (default)
		errOpenFail,		// Failed Opening TesseraQV302 file
		errInvalidFormat,	// Invalid TesseraQV302 format
		errInvalidFormatLowInRows,			// Didn't find parameter rows
		errNoLimitsFound,	// Missing limits...no a valid TesseraQV302 file
		errLicenceExpired,	// File date out of Licence window!
		errWriteSTDF		// Failed creating STDF intermediate file
	};

	bool					m_bNewTesseraQV302ParameterFound;	// set to true if TesseraQV302 file has a Parameter name not in our reference table=> makes it to be updated
	int						m_iTotalParameters;					// Holds the total number of parameters / tests in each part tested
	QStringList				m_pFullTesseraQV302ParametersList;	// Complete list of ALL TesseraQV302 parameters known.
	CGTesseraQV302Parameter *	m_pTesseraQV302Parameter;		// List of Parameters tables.
	QMap<int,CGTesseraQV302Binning *>	m_mapTesseraQV302Binning;		// List of Bin tables.

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
	QString	m_strTesterID;
	QString	m_strTesterType;
	QString	m_strSetupId;				// Setup Id
	QString	m_strJobName;				// Job Name
	QString	m_strJobRev;				// Job revision

	int		m_iIndexOffset;
	int		m_nPassFailIndex;
	int		m_nSiteIndex;


};


#endif
