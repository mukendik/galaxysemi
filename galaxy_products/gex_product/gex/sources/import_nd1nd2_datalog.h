#ifndef GEX_IMPORT_ND1ND2_DATALOG_H
#define GEX_IMPORT_ND1ND2_DATALOG_H

#include <qmap.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"

typedef QMap<QString, QString> strValueOfPinMap;

class CGNd1Nd2DatalogParameter
{
public:

	CGNd1Nd2DatalogParameter() {};

	QString			m_strName;					// Parameter name.
	QString			m_strUnits;					// Parameter units
	int 			m_nNumber;					// Parameter number.
	int				m_nScale;					// Parameter Scale
	float			m_fLowLimit;				// Parameter Low Spec limit
	float			m_fHighLimit;				// Parameter High Spec limit
	bool			m_bValidLowLimit;			// Low limit defined
	bool			m_bValidHighLimit;			// High limit defined
	QMap<int, QString> m_mapBin;				// Parameter status for the XXX sites locations.
	QMap<int, strValueOfPinMap> m_mapValue;		// Parameter values for the XXX sites locations.
												// FTR : fValueOfPinMap is empty
												// PTR : fValueOfPinMap[""] = fValue
												// else MPR : fValueOfPinMap[PinName] = fValue
	bool			m_bStaticHeaderWritten;		// 'true' after first STDF PTR static header data written.
};


class CNd1Nd2DatalogBinInfo
{
public:
	int						m_nBinNum;
	QString					m_strBinName;
	QMap <int, int>			m_nCount;					// Bin count for the XXX sites locations.
};


class CNd1Nd2DatalogPartInfo
{
public:
	QMap <int, int>			m_nXLoc;					// X Location for the XXX sites locations.
	QMap <int, int>			m_nYLoc;					// Y Location for the XXX sites locations.
	QMap <int, QString>		m_strBin;					// Pass status for the XXX sites locations.
};

class CGNd1Nd2DatalogtoSTDF
{
public:
	CGNd1Nd2DatalogtoSTDF();
    bool	Convert(const char *Nd1Nd2DatalogFileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:

	void NormalizeValues(QString &strUnits,float &fValue, int&nScale, bool &bIsNumber);
    bool ReadNd1Nd2DatalogFile(const char *Nd1Nd2DatalogFileName, const char *strFileNameSTDF);
	bool WriteStdfFile(QTextStream &hNd1Nd2DatalogFile,const char *strFileNameSTDF);
	long GetDateTimeFromString(QString strDateTime);

	QString ReadLine(QTextStream& hFile);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int		iProgressStep;
	int		iFileSize;
	int		iNextFilePos;


	QString m_strLastError;					// Holds last error string during Nd1Nd2Datalog->STDF convertion
	int		m_iLastError;					// Holds last error ID
	enum  errCodes
	{
		errNoError,							// No erro (default)
		errOpenFail,						// Failed Opening Nd1Nd2Datalog file
		errInvalidFormat,					// Invalid Nd1Nd2Datalog format
		errLicenceExpired,					// File date out of Licence window!
		errWriteSTDF						// Failed creating STDF intermediate file
	};
	enum eLimitType
	{
		eLowLimit,							// Flag to specify to save the Nd1Nd2Datalog Parameter LOW limit
		eHighLimit							// Flag to specify to save the Nd1Nd2Datalog Parameter HIGH limit
	};

	QString		m_strWaferId;				// WaferID
	QString		m_strDeviceId;				// DeviceID
	QString		m_strTesterId;				// TesterID
	QString		m_strLotId;					// LotID
	QString		m_strSubLotId;				// SubLotID
	QString		m_strProcessId;				// ProcessID
	QString		m_strProgramId;				// ProgramID
	QString		m_strStationId;				// StationID
	QString		m_strOperator;
	long		m_lStartTime;				// Startup time

	int			m_nPass;					// 2 pass for MPR and PMR records

	QMap<QString,int>						m_qMapPinIndex;
	QMap<QString, CNd1Nd2DatalogBinInfo>	m_mapSoftBinsCount;		// Bin stats.
	QMap<QString, CNd1Nd2DatalogBinInfo>	m_mapHardBinsCount;		// Bin stats.
	CNd1Nd2DatalogPartInfo					m_clPartsInfo;		// Parts status.
	QList <CGNd1Nd2DatalogParameter*>		m_lstpParameterFlow; // List of Parameters in the flow
	QStringList m_lstParametersStaticHeaderWritten;			// list of parameters known.

	bool		m_bFlagGenerateMpr;
};


#endif
