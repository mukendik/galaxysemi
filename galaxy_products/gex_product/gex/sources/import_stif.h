#ifndef GEX_IMPORT_STIF_H
#define GEX_IMPORT_STIF_H

#include <qdatetime.h>
#include <qmap.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"

class CStifBinInfo
{
public:
    CStifBinInfo(const QString &strBin);
	
	int		nBinNb;
	int		nNbCnt;
	bool	bPass;
};

class CGSTIFtoSTDF
{
public:
	CGSTIFtoSTDF();
	~CGSTIFtoSTDF();
    bool	Convert(const char *STIFFileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:
    bool ReadSTIFFile(const char *STIFFileName, const char *strFileNameSTDF);
	bool WriteStdfFile(QTextStream *hSTIFFile,const char *strFileNameSTDF);

	QString ReadLine(QTextStream& hFile);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int	iProgressStep;
	int	iNextFilePos;
	int	iFileSize;

	QString m_strLastError;	// Holds last error string during STIF->STDF convertion
	QString m_strLastMessage;
	int		m_iLastError;	// Holds last error ID
	enum  errCodes
	{
		errNoError,			// No erro (default)
		errOpenFail,		// Failed Opening STIF file
		errInvalidFormat,	// Invalid STIF format
		errLicenceExpired,	// File date out of Licence window!
		errWriteSTDF		// Failed creating STDF intermediate file
	};

	unsigned int	m_lStartTime;
	unsigned int	m_lStopTime;

	QString	m_strNullBinCode;		//NULBC	126 = ASCII code number of the nul bin code.
	int		m_nWaferColumns;
	int		m_nWaferRows;
	int		m_nStepX;
	int		m_nStepY;
	int		m_nUpperLeftX;
	int		m_nUpperLeftY;
	QString	m_strLotId;				//LOT	B88280
	QString	m_strSubLotId;			//READER	B88280-02
	QString	m_strWaferId;			//WAFER	02
	QString m_strDevice;			//PRODUCT	XF50661_2C_PROD
	QString	m_strAuxFile;			//SETUP FILE
	QString m_strOperator;			//OPERATOR	AugTech
	QString m_strTesterType;		//TEST SYSTEM	NSX1746
	QString m_strJobName;			//TEST PROG
	QString m_strCardId;			//PROBE CARD
	QString m_strHandId;			//PROBER	NSX1746
	int		m_nFlatNotch;			//FLAT	0
	float	m_fWaferSize;			//DIAM	5905
	int		m_nDiesXSize;			//XSTEP	1312	UNITS	(0.1)MIL
	int		m_nDiesYSize;			//YSTEP	637	UNITS	(0.1)MIL
	int		m_nRefDieX;				//XFRST	21
	int		m_nRefDieY;				//YFRST	41
	int		m_nDiesUnit;			//(0.1)MIL
	float	m_fDiesScaleUnit;		//(0.1)
	QMap<QString, CStifBinInfo*> m_qMapBins;
};

#endif
