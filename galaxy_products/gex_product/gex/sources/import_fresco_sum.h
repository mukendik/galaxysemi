#ifndef GEX_IMPORT_FRESCO_SUMMARY_H
#define GEX_IMPORT_FRESCO_SUMMARY_H

#include <QDateTime>
#include <QMap>
#include <QStringList>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"


class CGFrescoSummaryBinning
{
public:
	int				nNumber;	// Bin number
	QString			strName;	// Bin name
	bool			bPass;		// Bin cat
	QMap<int,int>	mapCount;	// Bin Summary count
};


class CGFrescoSummarytoSTDF
{
public:
	CGFrescoSummarytoSTDF();
	~CGFrescoSummarytoSTDF();
        bool	Convert(const char *FrescoSummaryFileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:
        bool ReadFrescoSummaryFile(const char *FrescoSummaryFileName, const char *strFileNameSTDF);
	bool WriteStdfFile(const char *strFileNameSTDF);

	QString ReadLine(QTextStream& hFile);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int	iProgressStep;
	int	iNextFilePos;
	int	iFileSize;

	QString strLastError;	// Holds last error string during FrescoSummary->STDF convertion
	int	iLastError;			// Holds last error ID
	enum  errCodes
	{
		errNoError,			// No erro (default)
		errOpenFail,		// Failed Opening FrescoSummary file
		errInvalidFormat,	// Invalid FrescoSummary format
		errInvalidFormatLowInRows,			// Didn't find parameter rows
		errLicenceExpired,	// File date out of Licence window!
		errWriteSTDF		// Failed creating STDF intermediate file
	};

	QMap<int,CGFrescoSummaryBinning>	m_mapFrescoSummarySoftBinning;	// List of Bin tables.
	QMap<int,CGFrescoSummaryBinning>	m_mapFrescoSummaryHardBinning;	// List of Bin tables.

	int		m_nTotalParts;
	int		m_nPassParts;
	int		m_nFailParts;

	long	m_lStartTime;				//Beginning Time: 2010-02-25 ¤U¤È 02:22:49
	long	m_lStopTime;				//Ending Time: 2010-02-25 ¤U¤È 03:18:23
	QString	m_strProductID;				//Product,{product-id}
	QString	m_strLotID;					//Lot,944
	QString	m_strSubLotID;				//Lot,944
	QString	m_strWaferID;				//Wafer,17870-235DT
	QString	m_strOperatorID;			//Operator ID,JRG
	QString	m_strTesterID;				//Tester ID,Demo Rev2
	QString	m_strTesterType;
	QString	m_strExecType;
	QString	m_strExecVer;
	QString	m_strJobName;				// Job Name
	QString	m_strJobRev;				// Job revision

};


#endif
