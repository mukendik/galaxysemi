#ifndef GEX_IMPORT_TESSERA_INSPECTION_H
#define GEX_IMPORT_TESSERA_INSPECTION_H

#include <QDateTime>
#include <QMap>
#include <QStringList>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"


class CGTesseraInspectionBinning
{
public:
	int		nNumber;	// Bin number
	QString strName;	// Bin name
	bool	bPass;		// Bin cat
	int		nCount;		// Bin count

	bool	bToIgnore;	//Unpopulated
};

class CGTesseraInspectionParameter
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




class CGTesseraInspectiontoSTDF
{
public:
	CGTesseraInspectiontoSTDF();
	~CGTesseraInspectiontoSTDF();
    bool	Convert(const char *TesseraInspectionFileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:
	void LoadParameterIndexTable(void);
	void DumpParameterIndexTable(void);
	void UpdateParameterIndexTable(QString strParamName);
    bool ReadTesseraInspectionFile(const char *TesseraInspectionFileName,const char *strFileNameSTDF);
	bool WriteStdfFile(QTextStream *hTesseraInspectionFile,const char *strFileNameSTDF);
	void NormalizeLimits(QString &strUnit, int &nScale);

	QString ReadLine(QTextStream& hFile);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int	iProgressStep;
	int	iNextFilePos;
	int	iFileSize;

	QString strLastError;	// Holds last error string during TesseraInspection->STDF convertion
	int	iLastError;			// Holds last error ID
	enum  errCodes
	{
		errNoError,			// No erro (default)
		errOpenFail,		// Failed Opening TesseraInspection file
		errInvalidFormat,	// Invalid TesseraInspection format
		errInvalidFormatLowInRows,			// Didn't find parameter rows
		errNoLimitsFound,	// Missing limits...no a valid TesseraInspection file
		errLicenceExpired,	// File date out of Licence window!
		errWriteSTDF		// Failed creating STDF intermediate file
	};

	bool					m_bNewTesseraInspectionParameterFound;	// set to true if TesseraInspection file has a Parameter name not in our reference table=> makes it to be updated
	int						m_iTotalParameters;				// Holds the total number of parameters / tests in each part tested
	QStringList				m_pFullTesseraInspectionParametersList;	// Complete list of ALL TesseraInspection parameters known.
	CGTesseraInspectionParameter *	m_pTesseraInspectionParameter;		// List of Parameters tables.
	QMap<int,CGTesseraInspectionBinning *>	m_mapTesseraInspectionBinning;		// List of Bin tables.

	long	m_lStartTime;				//~Start Date/Time,27/01/2009 10:53:46
	long	m_lStopTime;				//~End Date/Time,27/01/2009 11:00:41
	QString m_strSoftVer;				//~Version,v1.1
	QString	m_strProductID;				//~Product,{product-id}
	QString	m_strLotID;					//~Lot,944
	QString	m_strWaferID;				//~Wafer,17870-235DT
	QString	m_strSideID;				//~Side,B
	QString	m_strSlotID;				//~Slot ID,0
	QString	m_strOperatorID;			//~Operator ID,JRG
	QString	m_strAuxFile;				//~Recipe,C:\ICOS\RECIPES\944B_8D_TESSERAMAP_FIELD_EME_1.RCP
	QString	m_strTesterID;				//~Machine ID,Demo Rev2
	QString	m_strTesterType;
	QString	m_strSetupId;				// Setup Id
	QString	m_strJobName;				// Job Name
	QString	m_strJobRev;				// Job revision

};


#endif
