#ifndef GEX_IMPORT_ACCO_H
#define GEX_IMPORT_ACCO_H

#include <QDateTime>
#include <QMap>
#include <QStringList>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"


class CGAccoBinning
{
public:
	int				nNumber;	// Bin number
	QString			strName;	// Bin name
	bool			bPass;		// Bin cat
	QMap<int,int>	mapSumCount;	// Bin Summary count
	QMap<int,int>	mapSiteCount;	// Bin count
};

class CGAccoParameter
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




class CGAccotoSTDF
{
public:
	CGAccotoSTDF();
	~CGAccotoSTDF();
    bool	Convert(const char *AccoFileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:
	void LoadParameterIndexTable(void);
	void DumpParameterIndexTable(void);
	void UpdateParameterIndexTable(QString strParamName);
    bool ReadAccoFile(const char *AccoFileName, const char *strFileNameSTDF);
	bool WriteStdfFile(QTextStream *hAccoFile,const char *strFileNameSTDF);
	void NormalizeLimits(QString &strUnit, int &nScale);

	QString ReadLine(QTextStream& hFile);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int	iProgressStep;
	int	iNextFilePos;
	int	iFileSize;

	QString strLastError;	// Holds last error string during Acco->STDF convertion
	int	iLastError;			// Holds last error ID
	enum  errCodes
	{
		errNoError,			// No erro (default)
		errOpenFail,		// Failed Opening Acco file
		errInvalidFormat,	// Invalid Acco format
		errInvalidFormatLowInRows,			// Didn't find parameter rows
		errNoLimitsFound,	// Missing limits...no a valid Acco file
		errLicenceExpired,	// File date out of Licence window!
		errWriteSTDF		// Failed creating STDF intermediate file
	};

	bool					m_bNewAccoParameterFound;	// set to true if Acco file has a Parameter name not in our reference table=> makes it to be updated
	int						m_iTotalParameters;			// Holds the total number of parameters / tests in each part tested
	int						m_iDataOffset;
	int						m_iXYCoordPos;
	QStringList				m_pFullAccoParametersList;	// Complete list of ALL Acco parameters known.
	CGAccoParameter *		m_pAccoParameter;			// List of Parameters tables.
	QMap<int,CGAccoBinning *>	m_mapAccoSoftBinning;	// List of Bin tables.
	QMap<int,CGAccoBinning *>	m_mapAccoHardBinning;	// List of Bin tables.

	long	m_lStartTime;				//Beginning Time: 2010-02-25 ¤U¤È 02:22:49
	long	m_lStopTime;				//Ending Time: 2010-02-25 ¤U¤È 03:18:23
	QString	m_strProductID;				//Product,{product-id}
	QString	m_strLotID;					//Lot,944
	QString	m_strWaferID;				//Wafer,17870-235DT
	QString	m_strOperatorID;			//Operator ID,JRG
	QString	m_strTesterID;				//Tester ID,Demo Rev2
	QString	m_strTesterType;
	QString	m_strJobName;				// Job Name
	QString	m_strJobRev;				// Job revision

	int		m_nTotalParts;
	int		m_nPassParts;
	int		m_nFailParts;

};


#endif
