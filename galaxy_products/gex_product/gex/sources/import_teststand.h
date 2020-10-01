#ifndef GEX_IMPORT_TESTSTAND_H
#define GEX_IMPORT_TESTSTAND_H

#include <time.h>

#include <qmap.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"

class CGNiParameter
{
public:
	CGNiParameter();	// Constructor
	void	clear(void);	// Reset content

	QString strName;	// Parameter name. E.g: "Idd_Total_Shutdown"
	QString strUnits;	// Parameter units,E.g: "A"
	float	lfTestTime;	// Module execution time. (<0 if not test time available)
	unsigned long	lTestNumber;// Test#
	float	fLowLimit;	// Parameter Low Spec limit, E.g: 0.00004
	float	fHighLimit;	// Parameter High Spec limit, E.g: -0.00004
	bool	bValidLowLimit;		// Low limit defined
	bool	bValidHighLimit;	// High limit defined
	bool	bValidResult;	// 'false' if no test result available (the case when only test time is given).
    float	fValue;		// Parameter result
	bool	bPassed;	// true if test passed
};

class CGNItoSTDF
{
public:
	CGNItoSTDF();
	~CGNItoSTDF();
    bool Convert(const char *strNiFileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);


private:
	void LoadParameterIndexTable(void);
	void DumpParameterIndexTable(void);
	int	 UpdateParameterIndexTable(const QString &strParamName);
	QDate extractDate(const QString &strDate);
	QTime extractTime(const QString &strTime);
    bool ReadNiFile(const char *strNiFileName, const char *strFileNameSTDF);
    bool WriteStdfFile(QTextStream *hNiFile,const char *strFileNameSTDF);

	QString ReadLine(QTextStream& hFile);
	
	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int	iProgressStep;
	int	iNextFilePos;
	int	iFileSize;

	void WriteRunHeader(void);	
	void WriteMIR(void);
	void WriteDTR(const QString &strString,const QString &strColor=NULL);
	void WritePIR(void);
	void WritePTR(void);
	void WritePRR(void);
	void WriteSBR_HBR();
	void WritePCR();
	void WriteMRR();


	QString strLastError;	// Holds last error string during CSV->STDF convertion
	int	iLastError;			// Holds last error ID
	enum  errCodes
	{
		errNoError,			// No erro (default)
		errOpenFail,		// Failed Opening NI file
		errInvalidHeader,	// Failed to find the NI header
		errInvalidFormatParameter,	// Invalid CSV format: didn't find the 'Parameter' section
		errLicenceExpired,	// File date out of Licence window!
		errWriteSTDF		// Failed creating STDF intermediate file
	};

    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;

	bool	bNewCsvParameterFound;		// set to true if CSV file has a Parameter name not in our reference table=> makes it to be updated
	CGNiParameter m_NiParameter;		// Parameters to datalog
	QStringList pFullCsvParametersList;	// Complete list of ALL CSV parameters known.

	time_t	m_lUnitTestDate;					// Dataset creation time
	int		iBurninTime;				// Burn-In Time
	QString	strLotID;					// LotID string
	QString	strProductID;				// Product / Device name
	QString	strTesterType;				// Tester type.
	QString strProgramRev;				// Job rev
	QString strExecType;				// exec-type
	QString strExecRev;					// exe-ver
	QString strTestCode;				// test-cod
	QString strFacilityID;				// Facility-ID
	QString strFloorID;					// FloorID
	QString strProcessID;				// ProcessID
	QString	strSubLotID;				// SubLotID
	QString strTemperature;				// Temperature testing.
	QString	strPackageType;				// PAckage type.
	QString	strFamilyID;					// FamilyID
	QString strFrequencyStep;			// Frequency / Step
	QString strProberType;				// Prober type/family
	QString strProberName;				// Prober name
	QString strLoadBoardType;			// Loadboard type/family
	QString strLoadBoardName;			// LoadBoardID

	QDate	m_cDate;					// UUT Testing Date
	QTime	m_cTime;					// UUT Testing Time
	QDateTime m_cDateTime;				// UUT Testing date & time
	QString	strTesterName;				// Tester name string
	QString	strPartID;					// Part ID (serial name)
	QString strOperator;				// operator
	QString strProgramName;				// Job name
	float	m_lfExecTime;				// UUT Execution time.
	long	m_iTotalTests;				// Total tests executed in test sequence
	long	m_iTotalSamplesTests;		// Total tests dataloged in test sequence
	long	m_iTotalUnits;				// Total Units dataloged
	bool	m_bPassFail;				// 'true' if test sequenced passed (Bin1)
	int		m_iTotalGoodBin;			// Total good parts (used to create summary record)
	int		m_iTotalFailBin;			// Total bad parts (used to create summary record)


};


#endif
