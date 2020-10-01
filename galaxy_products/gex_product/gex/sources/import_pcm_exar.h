#ifndef GEX_IMPORT_PCM_EXAR_H
#define GEX_IMPORT_PCM_EXAR_H

#include <qdatetime.h>
#include <qstringlist.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"

class CGPcmExarParameter
{
public:

	CGPcmExarParameter();
	
	QString			m_strName;					// Parameter name
	QString			m_strUnit;					// Parameter unit
	int				m_nNumber;
	int				m_nScale;					// Parameter Scale
	float			m_fLowLimit;				// Parameter Low limit
	float			m_fHighLimit;				// Parameter High limit
	bool			m_bValidLowLimit;			// Low limit defined
	bool			m_bValidHighLimit;			// High limit defined
	bool			m_bStaticHeaderWritten;		// 'true' after first STDF PTR static header data written.
};

class CGPcmExartoSTDF
{
public:
	CGPcmExartoSTDF();
	~CGPcmExartoSTDF();
    bool	Convert(const char *PcmExarFileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:

	void LoadParameterIndexTable(void);
	void DumpParameterIndexTable(void);
	int	 UpdateParameterIndexTable(QString strParamName);
	void NormalizeLimits(QString &strUnit, int &nScale);
	void SaveParameter(int iIndex,QString strName,QString strLowLimit,QString strHighLimit);
    bool ReadPcmExarFile(const char *PcmExarFileName,const char *strFileNameSTDF);
    bool WriteStdfFile(QTextStream *hPcmExarFile, const char *strFileNameSTDF);

	QString ReadLine(QTextStream& hFile);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int	iProgressStep;
	int	iNextFilePos;
	int	iFileSize;

	QString m_strLastError;					// Holds last error string during PcmExar->STDF convertion
	int		m_iLastError;					// Holds last error ID
	enum  errCodes
	{
		errNoError,							// No erro (default)
		errOpenFail,						// Failed Opening PcmExar file
		errInvalidFormat,					// Invalid PcmExar format
		errLicenceExpired,					// File date out of Licence window!
		errWriteSTDF						// Failed creating STDF intermediate file
	};
	enum eLimitType
	{
		eLowLimit,							// Flag to specify to save the CSM Parameter LOW limit
		eHighLimit							// Flag to specify to save the CSM Parameter HIGH limit
	};

	QString		m_strLotId;					// LotID
	QString		m_strWaferId;				// WaferID
	QString		m_strProductId;				// ProductID
	QString		m_strProcessId;				// ProcessID
	long		m_lStartTime;				// Startup time

	int			m_iParametersOffset;
	int			m_iTotalParameters;
	bool		m_bNewPcmExarParameterFound;	// set to true if PcmExar file has a Parameter name not in our reference table=> makes it to be updated
	QStringList m_pFullPcmExarParametersList;	// Complete list of ALL PcmExar parameters known.
	CGPcmExarParameter* m_pParameterList;	// List of Parameters in Wafer
};


#endif
