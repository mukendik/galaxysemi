#ifndef GEX_IMPORT_ETEST_FAB7_H
#define GEX_IMPORT_ETEST_FAB7_H

#include <QDateTime>
#include <QMap>
#include <QStringList>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"



class CGEtestFab7Parameter
{
public:
	int		nNumber;
	QString strName;	// Parameter name. E.g: "Idd_Total_Shutdown"
	QString strUnits;	// Parameter units,E.g: "A"
	int		nScale;
	float	fLowLimit;	// Parameter Low Spec limit, E.g: 0.00004
	float	fHighLimit;	// Parameter High Spec limit, E.g: -0.00004
	bool	bValidLowLimit;		// Low limit defined
	bool	bValidHighLimit;	// High limit defined
    float	fValue;		// Parameter result
	bool	bStaticHeaderWritten;	// 'true' after first STDF PTR static header data written.
};




class CGEtestFab7toSTDF
{
public:
	CGEtestFab7toSTDF();
	~CGEtestFab7toSTDF();
        bool	Convert(const char *EtestFab7FileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:
	void LoadParameterIndexTable(void);
	void DumpParameterIndexTable(void);
	void UpdateParameterIndexTable(QString strParamName);
        bool ReadEtestFab7File(const char *EtestFab7FileName, const char *strFileNameSTDF);
	bool WriteStdfFile(QTextStream *hEtestFab7File,const char *strFileNameSTDF);
	void NormalizeLimits(QString &strUnit, int &nScale);

	QString ReadLine(QTextStream& hFile);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int	iProgressStep;
	int	iNextFilePos;
	int	iFileSize;

	QString strLastError;	// Holds last error string during EtestFab7->STDF convertion
	int	iLastError;			// Holds last error ID
	enum  errCodes
	{
		errNoError,			// No erro (default)
		errOpenFail,		// Failed Opening EtestFab7 file
		errInvalidFormat,	// Invalid EtestFab7 format
		errInvalidFormatLowInRows,			// Didn't find parameter rows
		errNoLimitsFound,	// Missing limits...no a valid EtestFab7 file
		errLicenceExpired,	// File date out of Licence window!
		errWriteSTDF		// Failed creating STDF intermediate file
	};

	bool					m_bNewEtestFab7ParameterFound;	// set to true if EtestFab7 file has a Parameter name not in our reference table=> makes it to be updated
	int						m_iTotalParameters;			// Holds the total number of parameters / tests in each part tested
	int						m_iDataOffset;
	QStringList				m_pFullEtestFab7ParametersList;	// Complete list of ALL EtestFab7 parameters known.
	CGEtestFab7Parameter *		m_pEtestFab7Parameter;			// List of Parameters tables.

	long	m_lStartTime;				//Beginning Time: 2010-02-25 ¤U¤È 02:22:49
	long	m_lStopTime;				//Ending Time: 2010-02-25 ¤U¤È 03:18:23
	QString	m_strProductID;				//Product,{product-id}
	QString	m_strLotID;					//Lot,944
	QString	m_strWaferID;				//Wafer,17870-235DT
	QString	m_strTesterType;
	QString	m_strTemperature;
	QString	m_strTestCod;
	QString	m_strFabId;
	QString	m_strProcId;
	QString	m_strJobName;				// Job Name

	int		m_nTotalParts;
	int		m_nPassParts;
	int		m_nFailParts;

};


#endif
