#ifndef GEX_IMPORT_MCUBE_H
#define GEX_IMPORT_MCUBE_H

#include <QDateTime>
#include <QMap>
#include <QStringList>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"


class CGMcubeBinning
{
public:
	int				nNumber;	// Bin number
	QString			strName;	// Bin name
	bool			bPass;		// Bin cat
	int				nCount;
	QMap<int,int>	mapSiteCount;	// Bin count
};

class CGMcubeParameter
{
public:
	int		nNumber;
	QString strTestName;
	QString strTestCondition;
	QString	strTestPin;
	QString strUnits;
	int		nScale;
	float	fLowLimit;
	float	fHighLimit;
	bool	bValidLowLimit;
	bool	bValidHighLimit;
	bool	bStaticHeaderWritten;
};




class CGMcubetoSTDF
{
public:
	CGMcubetoSTDF();
	~CGMcubetoSTDF();
    bool	Convert(const char *McubeFileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:
    bool ReadMcubeFile(const char *McubeFileName, const char *strFileNameSTDF);
	bool WriteStdfFile(QTextStream *hMcubeFile,const char *strFileNameSTDF);
	void NormalizeLimits(QString &strUnit, int &nScale);

	long GetDateTimeFromString(QString strDateTime);
	bool ReadHeaderInformation(QTextStream &hMcubeFile);
	bool ReadTestDefInformation(QTextStream &hMcubeFile);
	bool ReadBinNamesInformation(QTextStream &hMcubeFile);
	bool ReadLimitsDefInformation(QTextStream &hMcubeFile);
	bool ReadLimitsDataInformation(QTextStream &hMcubeFile);

	QString ReadLine(QTextStream& hFile);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int	iProgressStep;
	int	iNextFilePos;
	int	iFileSize;

	QString strErrorMessage;
	QString strLastError;	// Holds last error string during Mcube->STDF convertion
	int	iLastError;			// Holds last error ID
	enum  errCodes
	{
		errNoError,			// No erro (default)
		errOpenFail,		// Failed Opening Mcube file
		errInvalidFormat,	// Invalid Mcube format
		errInvalidSectionFormat,
		errInvalidTestFormat,
		errInvalidBinResult,
		errLicenceExpired,	// File date out of Licence window!
		errWriteSTDF		// Failed creating STDF intermediate file
	};

	QString					m_strCurrentLine;
	int						m_nCurrentLine;
	int						m_iTestCondIndex;
	QMap<QString,QString>	m_mapMcubeTestDef;
	QMap<QString,QString>	m_mapMcubeLimitsDef;
	QMap<int,CGMcubeParameter>  m_mapMcubeParameters;	// List of Parameters tables.
	QMap<int,CGMcubeBinning>	m_mapMcubeSoftBinning;	// List of Bin tables.
	QMap<int,CGMcubeBinning>	m_mapMcubeHardBinning;	// List of Bin tables.

	long		m_lStartTime;				//Beginning Time
	long		m_lStopTime;				//Ending Time
	bool		m_bIsWafer;
	QString		m_strLotID;					//Lot
	QString		m_strWaferID;				//Wafer
	QString		m_strTesterID;				//Tester ID,Demo Rev2
	QString		m_strFacilID;
	QString		m_strTesterType;
	QString		m_strTestCode;
	QString		m_strJobRev;				// Job revision
	QString		m_strSpecRev;				// Spec revision
	QString		m_strParserRev;
	QString		m_strExecType;
	QString		m_strExecVer;
	QString		m_strTemperature;
	QString		m_strRetestCode;
	QString		m_strLoadBoardID;
	QString		m_strJobName;

};


#endif
