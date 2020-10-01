#ifndef GEX_IMPORT_SEQUOIA_H
#define GEX_IMPORT_SEQUOIA_H

#include <qdatetime.h>
#include <qmap.h>
#include <qstringlist.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"

typedef QMap<int, float> fValueOfSiteMap;

class CGSequoiaTestLimit
{
public:
	float	fLowLimit;			// Parameter Low Spec limit, E.g: 0.00004
	float	fHighLimit;			// Parameter High Spec limit, E.g: -0.00004
	bool	bValidLowLimit;		// Low limit defined
	bool	bValidHighLimit;	// High limit defined
};

class CGSequoiaTestParameter
{
public:
	CGSequoiaTestParameter() {fValue = 0; bValidValue = bTestExecuted = bStaticHeaderWritten = false;};
	QString strName;			// Parameter name. E.g: "Idd_Total_Shutdown"
	QString strUnits;			// Parameter units,E.g: "A"
	QMap<QString, CGSequoiaTestLimit *> mapGTestCondTestLimits; // Contains all limits define for each GlobalTestCond
    float	fValue;				// Parameter result
	bool	bValidValue;		// Can contains N/A
	bool	bTestExecuted;		// Flow can change for each TestCondition
	bool	bStaticHeaderWritten;	// 'true' after first STDF PTR static header data written.
};


class CGSequoiaTestCondition
{
public:
	CGSequoiaTestCondition() {bValidValue = bStaticHeaderWritten = false;};

	QString strName;		// Parameter name. E.g: "Idd_Total_Shutdown"
    QString	strValue;		// Parameter result
	bool	bValidValue;	// Can contains N/A
	bool	bStaticHeaderWritten;	// 'true' after first STDF PTR static header data written.
};


class CGSEQUOIAtoSTDF
{
public:
	CGSEQUOIAtoSTDF();
	~CGSEQUOIAtoSTDF();
    bool	Convert(const char *SequoiaFileName, QString &strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:

	void LoadParameterIndexTable(void);
	void DumpParameterIndexTable(void);
	void UpdateParameterIndexTable(QString strParamName);
    bool ReadSequoiaFile(const char *SequoiaFileName);
	bool WriteStdfFile(QTextStream *hSequoiaFile);

	bool WriteBeginGlobalTestCondition();
	bool WriteEndGlobalTestCondition();
	
	QString ReadLine(QTextStream& hFile);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int	iProgressStep;
	int	iNextFilePos;
	int	iFileSize;

	QString m_strLastError;	// Holds last error string during SEQUOIA->STDF convertion
	int		m_iLastError;	// Holds last error ID
	enum  errCodes
	{
		errNoError,			// No erro (default)
		errOpenFail,		// Failed Opening SEQUOIA file
		errInvalidFormat,	// Invalid SEQUOIA format
		errNoLimitsFound,	// Missing limits...no a valid SEQUOIA file
		errLicenceExpired,	// File date out of Licence window!
		errMultiCond,		// File date out of Licence window!
		errWriteSTDF		// Failed creating STDF intermediate file
	};
	QString		m_strStdfFileName;
	QString		m_strLastStdfFileName;
	bool		m_bSplitMultiCond;
	bool		m_bHaveMultiCond;
    GS::StdLib::Stdf		m_StdfFile;
	long		m_iTotalGoodBin, m_iTotalFailBin;
	QString		m_strTestCondId;
	
	bool		m_bNewSequoiaParameterFound;		// set to true if SEQUOIA file has a Parameter name not in our reference table=> makes it to be updated
	QStringList m_pFullSequoiaParametersList;	// Complete list of ALL SEQUOIA parameters known.
	
	QString		m_strSectionHeader;
	CGSequoiaTestParameter	*m_tabTestParameter;	// List of Parameters tables.
	CGSequoiaTestCondition	*m_tabTestCondition;	// List of TestConditions tables.
	int			m_iTotalTestParameters;		// Holds the total number of parameters
	int			m_iTotalTestCondition;		// Holds the total number of TestConditions
	QStringList m_lstGlobalTestCondition;	// in first pos, the current test condition to check;

	long	m_lStartTime;				// Startup time
	QString	m_strLotID;					// LotID string
	QString	m_strProductID;				// Product / Device name
	QString	m_strProgramID;				// Program name

	QString m_strLimitFileName;

};


#endif
