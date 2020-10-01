#ifndef GEX_IMPORT_ASL1000_H
#define GEX_IMPORT_ASL1000_H

#include <qmap.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"

class CGASL1000toSTDF
{
public:
	CGASL1000toSTDF();
	~CGASL1000toSTDF();
    bool	Convert(const char *Asl1000FileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:

	void LoadParameterIndexTable(void);
	void DumpParameterIndexTable(void);
	void UpdateParameterIndexTable(QString strParamName);
	void NormalizeLimits(QString &strUnit, int &nScale);
    bool ReadAsl1000File(const char *Asl1000FileName, const char *strFileNameSTDF);
	bool WriteStdfFile(FILE *fp,const char *strFileNameSTDF);


	QString ReadLine(QTextStream& hFile);
	int		ReadLine(FILE *fp, char *bp);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int	iProgressStep;
	int	iNextFilePos;
	int	iFileSize;


	QString m_strLastError;						// Holds last error string during ASL1000->STDF convertion
	int		m_iLastError;						// Holds last error ID
	enum  errCodes
	{
		errNoError,								// No erro (default)
		errOpenFail,							// Failed Opening ASL1000 file
		errInvalidFormat,						// Invalid ASL1000 format
		errLicenceExpired,						// File date out of Licence window!
		errWriteSTDF							// Failed creating STDF intermediate file
	};

	QString		m_strLotId;						// LotID
	QString		m_strProgramId;					// ProgramID
	QString		m_strOperatorId;				// OperatorID
	QString		m_strComputerId;				// ComputerID
	long		m_lStartTime;					// Startup time
	bool		m_bNewAsl1000ParameterFound;	// set to true if ASL1000 file has a Parameter name not in our reference table=> makes it to be updated
	QStringList m_pFullAsl1000ParametersList;	// Complete list of ALL Asl1000 parameters known.
};


#endif
