#ifndef GEX_IMPORT_ADVANTEST_T2000_H
#define GEX_IMPORT_ADVANTEST_T2000_H

#include <qmap.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"

typedef QMap<QString, float> fValueOfPinMap;

class CGAdvantestT2000Parameter
{
public:

	CGAdvantestT2000Parameter() {};
	
	QString			m_strName;					// Parameter name.
	QString			m_strUnits;					// Parameter units
	int 			m_nNumber;					// Parameter number.
	int				m_nScale;					// Parameter Scale
	float			m_fLowLimit;				// Parameter Low Spec limit
	float			m_fHighLimit;				// Parameter High Spec limit
	bool			m_bValidLowLimit;			// Low limit defined
	bool			m_bValidHighLimit;			// High limit defined
    QMap<int, bool> m_mapPass;					// Parameter status for the XXX sites locations.
    QMap<int, fValueOfPinMap> m_mapValue;		// Parameter values for the XXX sites locations.
												// FTR : fValueOfPinMap is empty
												// PTR : fValueOfPinMap[""] = fValue
												// else MPR : fValueOfPinMap[PinName] = fValue
	bool			m_bStaticHeaderWritten;		// 'true' after first STDF PTR static header data written.
};


class CAdvantest_T2000BinInfo
{
public:
	QMap <int, int>			m_nCount;					// Bin count for the XXX sites locations.
};

class CAdvantest_T2000PartInfo
{
public:
	QMap <int, int>			m_nXLoc;					// X Location for the XXX sites locations.
	QMap <int, int>			m_nYLoc;					// Y Location for the XXX sites locations.
	QMap <int, bool>		m_bPass;					// Pass status for the XXX sites locations.
};


class CGADVANTEST_T2000toSTDF
{
public:
	CGADVANTEST_T2000toSTDF();
    bool	Convert(const char *AdvantestT2000FileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:

	void NormalizeValues(QString &strUnits,float &fValue, int&nScale, bool &bIsNumber);
    bool ReadAdvantestT2000File(const char *AdvantestT2000FileName, const char *strFileNameSTDF);
	bool WriteStdfFile(QTextStream &hAdvantestT2000File,const char *strFileNameSTDF);

	QString ReadLine(QTextStream& hFile);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int		iProgressStep;
	int		iFileSize;
	int		iNextFilePos;


	QString m_strLastError;					// Holds last error string during ADVANTEST_T2000->STDF convertion
	int		m_iLastError;					// Holds last error ID
	enum  errCodes
	{
		errNoError,							// No erro (default)
		errOpenFail,						// Failed Opening ADVANTEST_T2000 file
		errInvalidFormat,					// Invalid ADVANTEST_T2000 format
		errLicenceExpired,					// File date out of Licence window!
		errWriteSTDF						// Failed creating STDF intermediate file
	};
	enum eLimitType
	{
		eLowLimit,							// Flag to specify to save the ADVANTEST_T2000 Parameter LOW limit
		eHighLimit							// Flag to specify to save the ADVANTEST_T2000 Parameter HIGH limit
	};

	QString		m_strWaferId;				// WaferID
	QString		m_strDeviceId;				// DeviceID
	QString		m_strTesterId;				// TesterID
	QString		m_strLotId;					// LotID
	QString		m_strProcessId;				// ProcessID
	QString		m_strProgramId;				// ProgramID
	QString		m_strStationId;				// StationID
	long		m_lStartTime;				// Startup time

	QMap<int, CAdvantest_T2000BinInfo>	m_mapBinsCount;		// Bin stats.
	CAdvantest_T2000PartInfo			m_clPartsInfo;		// Parts status.
	QList <CGAdvantestT2000Parameter*>	m_plstParameterFlow;// List of Parameters in the flow
	QStringList m_lstParametersStaticHeaderWritten;			// list of parameters known.
	
	QMap<QString,int>	m_qMapPinIndex;

	bool		m_bFlagGenerateMpr;
};


#endif
