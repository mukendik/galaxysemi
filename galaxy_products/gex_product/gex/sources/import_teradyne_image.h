#ifndef GEX_IMPORT_TERADYNE_IMAGE_H
#define GEX_IMPORT_TERADYNE_IMAGE_H

#include <qmap.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"

class CTeradyneImageBinInfo
{
public:
	CTeradyneImageBinInfo();
	
	int		nNbCnt;
	int		nPassFlag;
};

class CGTERADYNE_IMAGEtoSTDF
{
public:
	CGTERADYNE_IMAGEtoSTDF();
	~CGTERADYNE_IMAGEtoSTDF();
    bool	Convert(const char *TeradyneImageFileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:
	void LoadParameterIndexTable(void);
	void DumpParameterIndexTable(void);
	void UpdateParameterIndexTable(QString strParamName);
	void NormalizeLimits(QString &strUnit, int &nScale);
    bool ReadTeradyneImageFile(const char *TeradyneImageFileName, const char *strFileNameSTDF);
	bool WriteStdfFile(QTextStream *hTeradyneImageFile,const char *strFileNameSTDF);

	QString ReadLine(QTextStream& hFile);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int	iProgressStep;
	int	iNextFilePos;
	int	iFileSize;

	QString m_strLastError;					// Holds last error string during TERADYNE_IMAGE->STDF convertion
	int		m_iLastError;					// Holds last error ID
	enum  errCodes
	{
		errNoError,							// No erro (default)
		errOpenFail,						// Failed Opening TERADYNE_IMAGE file
		errInvalidFormat,					// Invalid TERADYNE_IMAGE format
		errLicenceExpired,					// File date out of Licence window!
		errWriteSTDF						// Failed creating STDF intermediate file
	};

	QString		m_strWaferId;				// WaferID
	QString		m_strTesterId;				// TesterID
	QString		m_strLotId;					// LotID
	QString		m_strProcessId;				// ProcessID
	QString		m_strProgramId;				// ProgramID
	QString		m_strStationId;				// StationID
	QString		m_strTestProgram;			// Test program
	QString		m_strTestFlow;				// Test Flow
	long		m_lStartTime;				// Startup time
	bool		m_bNewTeradyneImageParameterFound;	// set to true if TERADYNE_IMAGE file has a Parameter name not in our reference table=> makes it to be updated
	QStringList m_pFullTeradyneImageParametersList;	// Complete list of ALL TeradyneImage parameters known.
	QMap<int, CTeradyneImageBinInfo*> m_qMapBins;
};


#endif
