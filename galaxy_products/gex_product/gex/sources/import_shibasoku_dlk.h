#ifndef GEX_IMPORT_DLK_H
#define GEX_IMPORT_DLK_H

#include <qmap.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"

class CGDLKtoSTDF
{
public:
	CGDLKtoSTDF();
	~CGDLKtoSTDF();
        bool	Convert(const char *DLKFileName, QStringList &lstFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:
	bool CheckValidityDate(QDate *pExpirationDate);
	void NormalizeLimits(QString &strUnit, int &nScale);
        bool ReadDLKFile(const char *DLKFileName, const char *strFileNameSTDF);
	bool WriteStdfFile(QTextStream& hFile);


	QString ReadLine(QTextStream& hFile);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int	iProgressStep;
	int	iNextFilePos;
	int	iFileSize;

	QString		m_strStdfFileName;
	QStringList	m_lstStdfFileName;

	QString m_strLastError;						// Holds last error string during DLK->STDF convertion
	int		m_iLastError;						// Holds last error ID
	enum  errCodes
	{
		errNoError,								// No erro (default)
		errOpenFail,							// Failed Opening DLK file
		errInvalidFormat,						// Invalid DLK format
		errLicenceExpired,						// File date out of Licence window!
		errWriteSTDF							// Failed creating STDF intermediate file
	};

	QString		m_strLotId;						// LotID
	QString		m_strProgramId;					// ProgramID
	QString		m_strOperatorId;				// OperatorID
	QString		m_strComputerId;				// ComputerID
	QString		m_strTesterType;				// Test Type
	QString		m_strNodeId;					// Node Id
	QString		m_strExecType;					// OutPut Mode
	QString		m_strTestCod;					// OutPut Mode
	long		m_lStartTime;					// Startup time
	bool		m_bNewDLKParameterFound;	// set to true if DLK file has a Parameter name not in our reference table=> makes it to be updated
	QStringList m_pFullDLKParametersList;	// Complete list of ALL DLK parameters known.
};


#endif
