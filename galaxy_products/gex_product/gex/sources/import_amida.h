#ifndef GEX_IMPORT_AMIDA_H
#define GEX_IMPORT_AMIDA_H

#include <qdatetime.h>
#include <qstringlist.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"

class CGAmidaBinning
{
public:
	int		nNumber;	// Bin number
	QString strName;	// Bin name
	bool	bPass;		// Bin cat
	int		nCount;		// Bin count
};


class CGAmidaParameter
{
public:

	CGAmidaParameter();
	
	int				m_nTestNumber;				// Parameter num
	QString			m_strName;					// Parameter name
	QString			m_strUnit;					// Parameter unit
	int				m_nScale;					// Parameter Scale
	float			m_fLowLimit;				// Parameter Low Spec limit
	float			m_fHighLimit;				// Parameter High Spec limit
	bool			m_bValidLowLimit;			// Low limit defined
	bool			m_bValidHighLimit;			// High limit defined
	bool			m_bStaticHeaderWritten;		// 'true' after first STDF PTR static header data written.
};

class CGAmidatoSTDF
{
public:
	CGAmidatoSTDF();
	~CGAmidatoSTDF();
    bool	Convert(const char *AmidaFileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:

	long GetDateTimeFromString(QString strDateTime);
	void LoadParameterIndexTable(void);
	void DumpParameterIndexTable(void);
	void UpdateParameterIndexTable(QString strParamName);
	void NormalizeLimits(QString &strUnit, int &nScale);
	void SaveParameter(int iIndex,QString &strName,QString &strUnit,QString &strLowLimit,QString &strHighLimit);
    bool ReadAmidaFile(const char *AmidaFileName,const char *strFileNameSTDF);
	bool WriteStdfFile(QTextStream *hAmidaFile,const char *strFileNameSTDF);

	QString ReadLine(QTextStream& hFile);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int	iProgressStep;
	int	iNextFilePos;
	int	iFileSize;

	QString m_strLastError;					// Holds last error string during AMIDA->STDF convertion
	int		m_iLastError;					// Holds last error ID
	enum  errCodes
	{
		errNoError,							// No erro (default)
		errOpenFail,						// Failed Opening AMIDA file
		errInvalidFormat,					// Invalid AMIDA format
		errLicenceExpired,					// File date out of Licence window!
		errWriteSTDF						// Failed creating STDF intermediate file
	};
	enum eLimitType
	{
		eLowLimit,							// Flag to specify to save the CSM Parameter LOW limit
		eHighLimit							// Flag to specify to save the CSM Parameter HIGH limit
	};

	bool		m_bValidXYpos;
	bool		m_bValidSiteNum;
	int			m_nDataOffset;
	int			m_nGoodParts;
	int			m_nFailParts;
	int			m_nTotalParts;
	QString		m_strLotId;					// LotID
	int			m_nWaferId;					// WaferID
	QString		m_strProductId;				// ProductID
	QString		m_strTesterId;				// TesterID
	QString		m_strTesterType;
	QString		m_strProgramId;				// ProgramID
	QString		m_strOperatorId;			// OperatorID
	QString		m_strStationId;				// StationID
	long		m_lStartTime;				// Startup time
	bool		m_bNewAmidaParameterFound;	// set to true if AMIDA file has a Parameter name not in our reference table=> makes it to be updated
	QStringList m_pFullAmidaParametersList;	// Complete list of ALL Amida parameters known.
	CGAmidaParameter* m_pParameterList;		// List of Parameters in Wafer
	int			m_iTotalParameters;
	QMap<int,CGAmidaBinning *>	m_mapAmidaBinning;		// List of Bin tables.
};


#endif
