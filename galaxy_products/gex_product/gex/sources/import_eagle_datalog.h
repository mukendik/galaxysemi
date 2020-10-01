#ifndef GEX_IMPORT_EAGLE_DATALOG_H
#define GEX_IMPORT_EAGLE_DATALOG_H

#include <qmap.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"

class CGEagleDatalogParameter
{
public:
	CGEagleDatalogParameter();

	unsigned int	m_uiTestNumLeft;
	unsigned int	m_uiTestNumRight;
	QString			m_strName;					// Parameter name
	QString			m_strUnits;					// Parameter units
	int				m_nScale;					// Parameter Scale
	int				m_nFormatRes;				// Used for formatting the test result %9.m_nFormatRes
	float			m_fLowLimit;				// Parameter Low Spec limit
	float			m_fHighLimit;				// Parameter High Spec limit
	bool			m_bValidLowLimit;			// Low limit defined
	bool			m_bValidHighLimit;			// High limit defined
	bool			m_bStaticHeaderWritten;		// 'true' after first STDF PTR static header data written.
};

class CGEAGLE_DATALOGtoSTDF
{
public:
	CGEAGLE_DATALOGtoSTDF();
	~CGEAGLE_DATALOGtoSTDF();
    bool	Convert(const char *EagleDatalogFileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:

	void LoadParameterIndexTable(void);
	void DumpParameterIndexTable(void);
	void UpdateParameterIndexTable(QString strParamName);
	void NormalizeLimits(QString &strUnit, int &nScale);
    bool ReadEagleDatalogFile(const char *EagleDatalogFileName, const char *strFileNameSTDF);
	bool WriteStdfFile(QTextStream *hEagleDatalogFile,const char *strFileNameSTDF);

	QString ReadLine(QTextStream& hFile);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int	iProgressStep;
	int	iNextFilePos;
	int	iFileSize;

	QString m_strLine;						// current line read from the file
	QString m_strLastError;					// Holds last error string during EAGLE_DATALOG->STDF convertion
	int		m_iLastError;					// Holds last error ID
	enum  errCodes
	{
		errNoError,							// No erro (default)
		errOpenFail,						// Failed Opening EAGLE_DATALOG file
		errInvalidFormat,					// Invalid EAGLE_DATALOG format
		errLicenceExpired,					// File date out of Licence window!
		errInvalidFormatLowInRows,			// Didn't find parameter rows
        errTestNumberOverflow,				// Quantix generated test number is too big
		errWriteSTDF						// Failed creating STDF intermediate file
	};

	bool			m_bIsAWafer;
	QString			m_strWaferId;				// WaferID
	QString			m_strTesterId;				// TesterID
	QString			m_strLotId;					// LotID
	QString			m_strSubLotId;				// SubLotID
	QString			m_strOperName;				// OperName
	QString			m_strNodeName;				// NodeName
	QString			m_strExecVer;				// ExecVer
	QString			m_strHandId;				// HandId
	QString			m_strPartType;				// PartType
	QString			m_strAuxFile;				// AuxFile
	QString			m_strSpecName;				// SpecName
	QString			m_strJobName;				// JobName
	QString			m_strSpecRev;				// SpecRev
	QString			m_strJobRev;				// JobRev
	QString			m_strDateCode;				// DateCode
	QString			m_strProcessId;				// ProcessID
	QString			m_strProgramId;				// ProgramID
	QString			m_strStationId;				// StationID
	long			m_lStartTime;				// Startup time
	bool			m_bNewEagleDatalogParameterFound;	// set to true if EAGLE_DATALOG file has a Parameter name not in our reference table=> makes it to be updated
	QStringList		m_pFullEagleDatalogParametersList;	// Complete list of ALL EagleDatalog parameters known.
	QMap<QString, CGEagleDatalogParameter*> m_qMapParameterList;	// List of Parameters
	QStringList		m_lstPartResult;
	unsigned int	m_uiMaxTestNumLeft;
	unsigned int	m_uiMaxTestNumRight;
};


#endif
