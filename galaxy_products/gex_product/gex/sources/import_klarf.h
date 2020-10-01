#ifndef GEX_IMPORT_KLARF_H
#define GEX_IMPORT_KLARF_H

#include <qmap.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"

class CGKLARFtoSTDF
{
public:
	CGKLARFtoSTDF();
	~CGKLARFtoSTDF();
    bool	Convert(const char *KlarfFileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:

    bool ReadKlarfFile(const char *KlarfFileName);
	bool WriteStdfFile(const char *strFileNameSTDF);

	QString ReadLine(QTextStream& hFile);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int		iProgressStep;
	int		iFileSize;
	int		iNextFilePos;

	QString m_strLastError;					// Holds last error string during KLARF->STDF convertion
	int		m_iLastError;					// Holds last error ID
	enum  errCodes
	{
		errNoError,							// No erro (default)
		errOpenFail,						// Failed Opening KLARF file
		errInvalidFormat,					// Invalid KLARF format
		errLicenceExpired,					// File date out of Licence window!
		errWriteSTDF						// Failed creating STDF intermediate file
	};

	QString		m_strProductID;				// ProductID
	QString		m_strLotID;					// LotID
	QString		m_strStepId;				// StepId
	QString		m_strSetupId;				// StepId
	QString		m_strDeviceID;				// DeviceID
	QString		m_strWaferID;				// WaferID
	QString		m_strProcessID;				// ProcessID
    QString		m_strJobName;				// Job Name
    QString     m_strNotch;                 // Notch
    char        mPosX;                      // POS_X
    char        mPosY;                      // POS_Y
	long		m_lStartTime;				// Startup time
	float		m_fDieHt;
	float		m_fDieWid;

	int			m_nBinGood;
	

	QMap<int,QString>	m_mapBinName;
	QMap<int,int>		m_mapBinCnt;

	QMap<QString,QStringList*>	m_mapPartBinning;
	
};


#endif
