#ifndef GEX_IMPORT_PCM_SILAN_H
#define GEX_IMPORT_PCM_SILAN_H

#include <qdatetime.h>
#include <qstringlist.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"

class CGPcmSilanParameter
{
public:

	CGPcmSilanParameter();
	
	QString			m_strName;					// Parameter name
	QString			m_strUnit;					// Parameter unit
	int				m_nNumber;
	int				m_nScale;					// Parameter Scale
	float			m_fLowLimit;				// Parameter Low limit
	float			m_fHighLimit;				// Parameter High limit
	bool			m_bValidLowLimit;			// Low limit defined
	bool			m_bValidHighLimit;			// High limit defined
	float			m_fSpecLowLimit;			// Parameter Spec Low limit
	float			m_fSpecHighLimit;			// Parameter Spec High limit
	bool			m_bValidSpecLowLimit;		// Spec Low limit defined
	bool			m_bValidSpecHighLimit;		// Spec High limit defined
	bool			m_bStaticHeaderWritten;		// 'true' after first STDF PTR static header data written.
};

class CGPcmSilantoSTDF
{
public:
	CGPcmSilantoSTDF();
	~CGPcmSilantoSTDF();
    bool	Convert(const char *PcmSilanFileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:

	void LoadParameterIndexTable(void);
	void DumpParameterIndexTable(void);
	int	 UpdateParameterIndexTable(QString strParamName);
	void NormalizeLimits(QString &strUnit, int &nScale);
	void SaveParameter(int iIndex,QString strName,QString strUnit,QString strLowLimit,QString strHighLimit,QString strSpecLowLimit,QString strSpecHighLimit);
    bool ReadPcmSilanFile(const char *PcmSilanFileName, const char *strFileNameSTDF);
	bool WriteStdfFile(QTextStream *hPcmSilanFile,const char *strFileNameSTDF);

	QString ReadLine(QTextStream& hFile);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int	iProgressStep;
	int	iNextFilePos;
	int	iFileSize;

	QString m_strLastError;					// Holds last error string during PcmSilan->STDF convertion
	int		m_iLastError;					// Holds last error ID
	enum  errCodes
	{
		errNoError,							// No erro (default)
		errOpenFail,						// Failed Opening PcmSilan file
		errInvalidFormat,					// Invalid PcmSilan format
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
	long		m_lStartTime;				// Startup time

	int			m_iTotalParameters;
	bool		m_bNewPcmSilanParameterFound;	// set to true if PcmSilan file has a Parameter name not in our reference table=> makes it to be updated
	QStringList m_pFullPcmSilanParametersList;	// Complete list of ALL PcmSilan parameters known.
	CGPcmSilanParameter* m_pParameterList;	// List of Parameters in Wafer
};


#endif
