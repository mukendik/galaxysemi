#ifndef GEX_IMPORT_PCM_X_FAB_H
#define GEX_IMPORT_PCM_X_FAB_H

#include <qdatetime.h>
#include <qstringlist.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"

class CGPcmXFabParameter
{
public:

	CGPcmXFabParameter();
	
	QString			m_strName;					// Parameter name
	QString			m_strUnit;					// Parameter unit
	int				m_nScale;					// Parameter Scale
	float			m_fLowLimit;				// Parameter Low Spec limit
	float			m_fHighLimit;				// Parameter High Spec limit
	bool			m_bValidLowLimit;			// Low limit defined
	bool			m_bValidHighLimit;			// High limit defined
	bool			m_bStaticHeaderWritten;		// 'true' after first STDF PTR static header data written.
};

class CGPCM_X_FABtoSTDF
{
public:
	CGPCM_X_FABtoSTDF();
	~CGPCM_X_FABtoSTDF();
    bool	Convert(const char *PcmXFabFileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:

    long GetDateTimeFromString(QString strDateTime);
	void LoadParameterIndexTable(void);
	void DumpParameterIndexTable(void);
	void UpdateParameterIndexTable(QString strParamName);
	void NormalizeLimits(QString &strUnit, int &nScale);
    void SaveParameter(int iIndex, QString strName, QString strUnit, const QString &fLowLimit, const QString &fHighLimit);
    bool ReadPcmXFabFile(const char *PcmXFabFileName, const char *strFileNameSTDF);
	bool WriteStdfFile(QTextStream *hPcmXFabFile,const char *strFileNameSTDF);

	QString ReadLine(QTextStream& hFile);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int	iProgressStep;
	int	iNextFilePos;
	int	iFileSize;

	QString m_strLastError;					// Holds last error string during PCM_X_FAB->STDF convertion
	int		m_iLastError;					// Holds last error ID
	enum  errCodes
	{
		errNoError,							// No erro (default)
		errOpenFail,						// Failed Opening PCM_X_FAB file
		errInvalidFormat,					// Invalid PCM_X_FAB format
		errLicenceExpired,					// File date out of Licence window!
		errWriteSTDF						// Failed creating STDF intermediate file
	};
	enum eLimitType
	{
		eLowLimit,							// Flag to specify to save the CSM Parameter LOW limit
		eHighLimit							// Flag to specify to save the CSM Parameter HIGH limit
	};

	QString		m_strLotId;					// LotID
	int			m_nWaferId;					// WaferID
	QString		m_strProductId;				// ProductID
	QString		m_strTesterId;				// TesterID
	QString		m_strProgramId;				// ProgramID
	QString		m_strStationId;				// StationID
	long		m_lStartTime;				// Startup time
	bool		m_bNewPcmXFabParameterFound;	// set to true if PCM_X_FAB file has a Parameter name not in our reference table=> makes it to be updated
	QStringList m_pFullPcmXFabParametersList;	// Complete list of ALL PcmXFab parameters known.
	QMap <int, CGPcmXFabParameter*> m_pParameterList;	// List of Parameters in Wafer
};


#endif
