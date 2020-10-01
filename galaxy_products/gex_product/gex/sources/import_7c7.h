// import_7c7.h: interface for the C7C7toSTDF class.
//
//////////////////////////////////////////////////////////////////////

#ifndef GEX_IMPORT_7C7_H
#define GEX_IMPORT_7C7_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <math.h>

#include <qdatetime.h>
#include <qstringlist.h>
#include <qmap.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"
#include <QFile>


#define WAFER_ID	1
#define PACKAGE_ID	2

#define BIT0 1
#define BIT1 2
#define BIT2 4
#define BIT3 8
#define BIT4 16
#define BIT5 32
#define BIT6 64
#define BIT7 128


class C7C7DutResult
{
public:
	C7C7DutResult();
	void reset();

// parameters
	UINT	nFlags; // 1 = WAFER_ID
					// 2 = PACKAGE_ID

	bool	bSite;	// true if have site info

 	int		X;			// for wafer
	int		Y;			// for wafer
	int		Pkg_id;		// for package
	int		SW_Bin;
	int		HW_Bin;
	int		P_F;
	long	Time;		// in second
	float	Temp;
	int		FF;
	int		TF;
	int		TT;
	int		Site;		// if site
	QString Bin_desc;
	int		Retest;
};

class C7C7TestResult
{
public:
	C7C7TestResult();
	void reset();

	int			test_primary_id;
	int			test_num;
// parameters
	QString		test_name;
	QString		seq_name;
	int			Result;
	QStringList ResUnit;
	QString		strResultUnit;
	QString		PinName;
	QString		LimitName;
};

class C7C7Summary
{
public:
	C7C7Summary();
	void reset();

// parameters
	int		SW_Bin;
	int		HW_Bin;
	int		P_F;
	int		Number;
	int		Prcnt;
	int		NumberOfSite;	// 0 if no site info
	float	*pSite;
	QString	Bin_desc;
};

class C7C7Sbr
{
public:
	C7C7Sbr(int iSW_Bin=-1,	int iHW_Bin=-1,	int iP_F=0, int iNumber=0,int nPrcnt=0, QString strBin_desc="");
// parameters
	int		SW_Bin;
	int		HW_Bin;
	int		P_F;
	int		Number;
	int		Prcnt;
	QString	Bin_desc;
};

class C7C7PartCnt
{
public:
	C7C7PartCnt();
	void reset();

// parameters
	bool	bSite;	// true if have site info

	int		Site;
	int		Number;
	int		Prcnt;
};



class C7C7Results
{
public:
	C7C7Results();

    int		exec_cnt;
    int		fail_cnt;
    int		alarm_tests;
    UINT	opt_flag;
    BYTE	pad_byte;
    float	test_min;
    float	test_max;
    float	tst_mean;
    float	tst_sdev;
    float	tst_sums;
    float	tst_sqrs;
};

class C7C7Pin 
{
public:
	C7C7Pin();
    QString		testNameSuffix;
    int			test_num;
    C7C7Results results;
};

class C7C7Limit
{
public:
	C7C7Limit();
    char	opt_flag;
    QString	units;
    int		llm_scal;
    int		hlm_scal;
    float	lo_limit;
    QString	low_limit_param;
    float	hi_limit;
    QString	high_limit_param;
    float	lo_spec;
    float	hi_spec;
    QString	limitName;

	bool SetLimit(QString strLowUnits, float fLowValue, 
				  QString strHighUnits, float fHighValue, 
				  QString strLowParam="", QString strHighParam="");
};

class C7C7Test
{
public:
	C7C7Test();
	~C7C7Test();

	bool			bLimitSaved;
	int				test_primary_id;	// unique identifier
	int				test_num;			// save the test_num to verify if the num is the same for each run
    QString			test_name;			// == suite name
    QString			seq_name;
    unsigned char	spreadFlag;
	QString			strResultUnit;
    QMap<QString,C7C7Pin*>		pin_list;
    QMap<QString,C7C7Limit*>	limit_list;
    bool			bFunctional; // else parametric

	C7C7Pin*	GetPin(int iIndex=0, QString strPinName=""); // return the good pin or create it if not exist
	C7C7Limit* GetLimit(int iIndex=0, QString strLimitName=""); // return the good pin or create it if not exist
	bool		HaveLimit(int iIndex=0, QString strLimitName="");
};

class C7C7PinChannel
{
public:
	C7C7PinChannel();
	~C7C7PinChannel();
	void reset();

	int			iBitPosition;
	QStringList lstChannel;
	QString		strPinName;
	
};



class C7C7toSTDF
{
public:
	C7C7toSTDF();
	virtual ~C7C7toSTDF();

    bool	Convert(const char *szFileName7C7, const char *szFileNameSTDF, bool bModeDatabase = false);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:
    bool FirstParse7C7File();
	bool SecondParse7C7File();

	QString readNext();
	QString readFirst();
	QString unEncapsuled(QString);
	QString m_strCurrentLine;
	bool	m_bEndOfFile;

	bool	m_bModeDatabase; // true if we want to ignore all test number and used only the table index saved
	QStringList m_lstTestName;
	void LoadParameterIndexTable(void);
	void DumpParameterIndexTable(void);
	void FindParameterIndex(QString strParamName);

	QString m_strLastError;	// Holds last error string during 7C7->STDF convertion
	int		m_iLastError;			// Holds last error ID
	enum  errCodes
	{
		errNoError,			// No erro (default)
		errOpenFail,		// Failed Opening 7C7 file
		errInvalidFormat,	// Invalid 7C7 format
		errLicenceExpired,	// File date out of Licence window!
		errWriteSTDF		// Failed creating STDF intermediate file
	};

	QFile*			m_pf7C7;
	QTextStream*	m_ph7C7File;

    GS::StdLib::Stdf*			m_pStdfFile;

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int		iProgressStep;
	int		iNextFilePos;
	int		iFileSize;



	bool	AnalyzeDutResult();
	bool	AnalyzeTestDef();
	bool	AnalyzeTestResult();
	bool	AnalyzeTestReport();
	bool	AnalyzeSummary();
	
	bool	GotoMarker(const char *szEndMarker);

	// information collected
	bool	m_bMirWriten;	// before to write the MIR, we have to collect the max of info

	long	m_lSetupT;
	long	m_lStartT;
	long	m_lEndT;
	long	m_lLastPartTime;
	int		m_nStatNum;	
	char	m_cModeCod;
	char	m_cRtstCod;
	char	m_cProtCod;
	long	m_lBurnTim;
	char	m_cCModCod;
	QString m_strLotId;
	QString m_strPartTyp;
	QString m_strNodeNam;
	QString m_strTestLevel;
	QString m_strEquipmentID;
	QString	m_strJobNam;
	QString	m_strJobRev;
	QString	m_strSbLotId;
	QString	m_strOperNam;
	QString m_strExecTyp;
	QString m_strExecVer;
	QString m_strTestCod;
	QString m_strTstTemp;
	QString m_strAuxFile;
	QString m_strPckTyp;
	QString m_strFamlyId;
	QString m_strDateCod;
	QString m_strFacilId;
	QString m_strFloorId;
	QString m_strProcId;
	QString m_strOperFrq;
	QString m_strSpecNam;
	QString m_strSpecVer;
	QString m_strFlowId;
	QString m_strSetupId;
	QString m_strDsgnRev;
	QString m_strEngId;
	QString m_strRomCod;
	QString m_strSerlNum;
	QString m_strSuprNam;
	QString m_strDataOrigin;
	
	int		m_nTotalWafers;

	QString m_strWaferId;
	int		m_nWaferNumDie;
	int		m_nWaferNumPassed;
	int		m_nWaferNumRetested;

	int		m_nTotalParts;
	int		m_nTotalPassed;
	int		m_nTotalRetested;
	
	QString m_strUserDesc;
	QString m_strExecDesc;
	int		m_nDispCode;
	
	C7C7DutResult				m_cl7C7DutResult;
	C7C7Summary					m_cl7C7Summary;
	C7C7PartCnt					m_cl7C7PartCnt;
	C7C7TestResult				m_cl7C7TestResult;
	QMap<int,C7C7Sbr*>			m_clSbrList;
	QMap<int,C7C7PinChannel*>	m_clPinChannel;
	QMap<int,C7C7Test*>			m_clTest;
	C7C7Test*					m_pcl7C7Test;
	QMap<QString,QString>		m_clVarValue;

	bool		SetLimit(C7C7Test*pclTest, int iIndex, QString strLimitName, QString strLowUnit, QString strLowValue, QString strHighUnit, QString strHighValue);
	float		ToFloat(QString strVar, bool *pbFloat=NULL);
	QString		ToValue(QString strVar);

	C7C7Test*	GetTest(int iTestPrimaryId); // create it if not exist
	C7C7Sbr*	GetSbr(int iSbrId);
	void		SaveSbr();

	// STDF functions
	void WriteFar();
	void WriteMir();
	void WriteWir();
	void WritePir();
	void WritePtr(C7C7TestResult* pclTestResultParam=NULL);
	void WriteFtr();
	void WritePrr();
	void WriteWrr();
	void WritePcr();
	void WriteSbr();
	void WriteHbr();
	void WriteMrr();
	void WritePmr();
	void WriteTsr();


};

int		PrefixUnitToScall_7C7(QString strUnit);
QString	PrefixUnitToUnit_7C7(QString strUnit);
int		StringToHex_7C7(QString strHexa);

#endif // #ifndef GEX_IMPORT_7C7_H
