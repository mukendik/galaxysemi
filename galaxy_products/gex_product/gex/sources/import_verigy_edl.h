#ifndef GEX_IMPORT_VERIGY_EDL_H
#define GEX_IMPORT_VERIGY_EDL_H

#include <qmap.h>
#include <qdatetime.h>
#include <qstringlist.h>

#include "gex_constants.h"
#include "stdf.h"

#define VERIGY_EDL_BLOCK_SIZE	1536
/*
 * The following event types are defined so far:
 */
enum {
  TestProgramStartEventType=1,	/* 1 */
  TestProgramEndEventType,	/* 2 */
  TestFlowStartEventType,	/* 3 */
  TestFlowEndEventType,		/* 4 */
  TestSuiteStartEventType,	/* 5 */
  TestSuiteEndEventType,	/* 6 */
  TestFunctionStartEventType,	/* 7 */
  TestFunctionEndEventType,	/* 8 */
  UserProcedureEventType,	/* 9 */
  TestEventType,		/* 10 */
  AssignmentEventType,		/* 11 */
  BinReachedEventType,		/* 12 */
  ConfigurationChangedEventType,/* 13 */
  AppModelLevelStartEventType,	/* 14 */
  AppModelLevelEndEventType,	/* 15 */
  PauseEventType,		/* 16 */
  AlarmEventType,		/* 17 */
  WarningEventType,		/* 18 */
  PrintToDatalogEventType,      /* 19 */
  GenericDataEventType,         /* 20 */
  ManualExecutionEventType,     /* 21 */
  AppModelLevelLoopEventType,   /* 22 */
  UserProcedureStartEventType,	/* 23 */

  NumberOfEventTypes
};


#define EVENT_NAMES { \
  "", \
  "TestProgramStartEventType", \
  "TestProgramEndEventType", \
  "TestFlowStartEventType", \
  "TestFlowEndEventType", \
  "TestSuiteStartEventType", \
  "TestSuiteEndEventType", \
  "TestFunctionStartEventType", \
  "TestFunctionEndEventType", \
  "UserProcedureEventType", \
  "TestEventType", \
  "AssignmentEventType", \
  "BinReachedEventType", \
  "ConfigurationChangedEventType", \
  "AppModelLevelStartEventType", \
  "AppModelLevelEndEventType", \
  "PauseEventType", \
  "AlarmEventType", \
  "WarningEventType", \
  "PrintToDatalogEventType", \
  "GenericDataEventType", \
  "ManualExecutionEventType", \
  "AppModelLevelLoopEventType", \
  "UserProcedureStartEventType" \
    }


enum {
  DeviceDirectoryAttrType = 1,    /*  1 */
  TestProgramNameAttrType,        /*  2 */
  WorkorderNameAttrType,          /*  3 */
  PinListAttrType,                /*  4 */
  UIDAttrType,	                  /*  5 */
  HostNameAttrType,               /*  6 */
  NamedValueAttrType,             /*  7 */
  DeviceIdAttrType,               /*  8 */
  SiteSetAttrType,                /*  9 */
  SiteIdAttrType,                 /* 10 */
  TestSuiteNameAttrType,          /* 11 */
  PassFailResultAttrType,         /* 12 */
  ValueResultAttrType,            /* 13 */
  PinValueResultAttrType,         /* 14 */
  PinPFResultAttrType,            /* 15 */
  PinRangeResultAttrType,         /* 16 */
  FailingCycleAttrType,	          /* 17 */
  VectorResultAttrType,	          /* 18 */
  TestFunctionTypeAttrType,       /* 19 */
  TestFunctionParametersAttrType, /* 20 */
  BinAttrType,	                  /* 21 */
  TestIdAttrType,	          /* 22 */
  TimeStampAttrType,	          /* 23 */
  TimingsAttrType,                /* 24 */
  LevelsAttrType,                 /* 25 */
  LabelAttrType,                  /* 26 */
  PlainDataAttrType,              /* 27 */
  UserProcedureNameAttrType,      /* 28 */
  AppModelLevelAttrType,          /* 29 */
  TestNameAttrType,               /* 30 */
  PassRangeAttrType,              /* 31 */
  MeasuredRangeAttrType,          /* 32 */
  ShmooLegendAttrType,            /* 33 */
  ShmooCellAttrType,              /* 34 */
  LinearWaveformAttrType,         /* 35 */
  CycleAttrType,                  /* 36 */
  VectorAttrType,	          /* 37 */
  ExpressionAttrType,		  /* 38 */
  PinGroupListAttrType,		  /* 39 */
  PortDescAttrType,               /* 40 */
  VectorFineResultAttrType,       /* 41 */
  PortAttrType,                   /* 42 */
  PortTimingsAttrType,            /* 43 */
  PortPFResultAttrType,           /* 44 */
  XYDataResultAttrType,           /* 45 */
  TagAttrType,                    /* 46 */ 
  TestsuiteListAttrType,          /* 47 */
  BinListAttrType,                /* 48 */
  SpecValueAttrType,              /* 49 */
  /*CR Number: CR15791*/
  GangAttrType,                   /* 50 */
  VectorResultLongNameAttrType,	  /* 51 */
  GenericDataFieldAttrType,	  /* 52 */
  LotTypeAttrType,                /* 53 */
  SWVersionAttrType,              /* 54 */
  TesterProductAttrType,          /* 55 */
  TestHeadNumberAttrType,         /* 56 */
  BypassStateAttrType,            /* 57 */
  ManualExecutionStateAttrType,   /* 58 */
  PerPinOutputOnPassAttrType,     /* 59 */
  PerPinOutputOnFailAttrType,     /* 60 */
  TestDescriptionAttrType,        /* 61 */
  ParametricOrFunctionalTestAttrType,        /* 62 */
  OutputOnPassAttrType,           /* 63 */
  OutputOnFailAttrType,           /* 64 */

  NumberOfAttributeTypes
};


/*CR Number: CR15791*/
#define ATTRIBUTE_NAMES { \
  "", \
  "DeviceDirectoryAttrType", \
  "TestProgramNameAttrType", \
  "WorkorderNameAttrType", \
  "PinListAttrType", \
  "UIDAttrType", \
  "HostNameAttrType", \
  "NamedValueAttrType", \
  "DeviceIdAttrType", \
  "SiteSetAttrType", \
  "SiteIdAttrType", \
  "TestSuiteNameAttrType", \
  "PassFailResultAttrType", \
  "ValueResultAttrType", \
  "PinValueResultAttrType", \
  "PinPFResultAttrType", \
  "PinRangeResultAttrType", \
  "FailingCycleAttrType", \
  "VectorResultAttrType", \
  "TestFunctionTypeAttrType", \
  "TestFunctionParametersAttrType", \
  "BinAttrType", \
  "TestIdAttrType", \
  "TimeStampAttrType", \
  "TimingsAttrType", \
  "LevelsAttrType", \
  "LabelAttrType", \
  "PlainDataAttrType", \
  "UserProcedureNameAttrType", \
  "AppModelLevelAttrType", \
  "TestNameAttrType", \
  "PassRangeAttrType", \
  "MeasuredRangeAttrType", \
  "ShmooLegendAttrType", \
  "ShmooCellAttrType", \
  "LinearWaveformAttrType", \
  "CycleAttrType", \
  "VectorAttrType", \
  "ExpressionAttrType", \
  "PinGroupListAttrType", \
  "PortAttrDescType", \
  "VectorFineResultAttrType", \
  "PortAttrType", \
  "PortTimingsAttrType", \
  "PortPFResultAttrType", \
  "XYDataResultAttrType", \
  "TagAttrType", \
  "TestsuiteListAttrType", \
  "BinListAttrType", \
  "SpecValueAttrType", \
  "GangAttrType", \
  "VectorResultLongNameAttrType", \
  "GenericDataFieldAttrType", \
  "LotTypeAttrType", \
  "SWVersionAttrType", \
  "TesterProductAttrType", \
  "TestHeadNumberAttrType", \
  "BypassStateAttrType", \
  "ManualExecutionStateAttrType", \
  "PerPinOutputOnPassAttrType", \
  "PerPinOutputOnFailAttrType", \
  "TestDescriptionAttrType", \
  "ParametricOrFunctionalTestAttrType", \
  "OutputOnPassAttrType", \
  "OutputOnFailAttrType" \
    }

enum 
{
	DRLShmooTestNormal,
	DRLShmooTestERCT,
	DRLShmooTestFFCI 
};

class CGVERIGY_EDLParameter
{
public:
	CGVERIGY_EDLParameter();

	int				m_nTestNum;
	QString			m_strName;					// Parameter name
	QString			m_strSuiteName;					// Parameter name
	QString			m_strUnits;					// Parameter units
	int				m_nScale;					// Parameter Scale
	float			m_fLowLimit;				// Parameter Low Spec limit
	float			m_fHighLimit;				// Parameter High Spec limit
	bool			m_bValidLowLimit;			// Low limit defined
	bool			m_bValidHighLimit;			// High limit defined
	bool			m_bStrictLowLimit;			// Strict Low limit defined
	bool			m_bStrictHighLimit;			// Strict High limit defined
	bool			m_bStaticHeaderWritten;		// 'true' after first STDF PTR static header data written.
};


class CGVERIGY_EDLBinInfo
{
public:
	CGVERIGY_EDLBinInfo();
	
	void UpdateCount(int nSite) 
	{
		mapSiteNbCnt[-1]++;
		if(nSite != -1)
		{
			if(!mapSiteNbCnt.contains(nSite))
				mapSiteNbCnt[nSite] = 0;
			mapSiteNbCnt[nSite]++;
		}
	};
	int GetCount(int nSite) 
	{
		return mapSiteNbCnt[nSite];
	};
	void ResetCount() 
	{
		QMap<int,int>::Iterator itSiteCnt;
		for(itSiteCnt=mapSiteNbCnt.begin(); itSiteCnt!=mapSiteNbCnt.end(); itSiteCnt++)
			mapSiteNbCnt[itSiteCnt.key()] = 0;
	};
	
	QMap<int,int>	mapSiteNbCnt;
	int				nHardBin;
	bool			bPass;
	QString			strBinName;
};

class CGVERIGY_EDLPinInfo
{
public:

	QMap<int,int> mapSiteChannel;
	QMap<int,int> mapSiteIndex;
	QString	strName;
	QString strNumber;
};

class CGVERIGY_EDLSiteInfo
{
public:
	CGVERIGY_EDLSiteInfo()
	{
		nXWafer		= 0;
		nYWafer		= 0;
		nPartNumber = 0;
		nTotalPass	= 0;
		nTotalFail	= 0;
		nTotalTests = 0;
		nPassStatus = -1;
		nHardBin	= -1;
		nSoftBin	= -1;
		bExecuted	= false;
	};
	
	int		nXWafer;
	int		nYWafer;
	int		nPartNumber;
	int		nTotalPass;
	int		nTotalFail;
	int		nTotalTests;
	int		nPassStatus;
	int		nHardBin;
	int		nSoftBin;
	bool	bExecuted;

};


class CGVERIGY_EDLtoSTDF
{
public:
	enum eConvertStatus
	{
		eConvertSuccess,		// Conversion successful
		eConvertDelay,			// Delay conversion
		eConvertError			// Conversion failed
	};
	enum  errCodes
	{
		errNoError,							// No erro (default)
		errWarning,							// File generated with warning
		errOpenFail,						// Failed Opening VERIGY_EDL file
		errConversionFailed,				// Not compatible
		errLicenceExpired,					// File date out of Licence window!
		errMultiLot,						// File date out of Licence window!
		errWriteSTDF						// Failed creating STDF intermediate file
	};

	CGVERIGY_EDLtoSTDF();
	~CGVERIGY_EDLtoSTDF();
    int			Convert(const char *VerigyEdlFileName, QStringList &lstFileNameSTDF, bool bAllowOnlyOneFile);
	QString		GetLastError();
    int			GetLastErrorCode() {return m_iLastError;}

	static bool	IsCompatible(const char *szFileName);

private:
    int		ReadVerigyEdlFile();
	int		WriteMir();
	int		WriteMrr();
	int		WriteWir();
	int		WriteWrr();
	int		WritePir();
	int		WritePrr();
	int		WriteResult();
	int		WritePtr(CGVERIGY_EDLParameter *pTest);
	int		WriteFtr(CGVERIGY_EDLParameter *pTest);
	int		WriteMpr(CGVERIGY_EDLParameter *pTest);
	int		WriteDtr();
	int		WriteGdr();

    int		ReadBlock(QFile* pFile, char *data, quint64 len);
	int		ReadEventRecord(QFile* pFile);
	int		ReadAttributeRecord();
	int		PrefixUnitToScall(QString strUnit);
	QString	PrefixUnitToUnit(QString strUnit);


	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int			iProgressStep;
	ulong		iNextFilePos;
	ulong		iFileSize;

	QString		m_strLastError;					// Holds last error string during VERIGY_EDL->STDF convertion
	QString		m_strErrorDetail;				// Detailed info about error exception
	int			m_iLastError;					// Holds last error ID
	QString		m_strVerigyEdlFileName;
	QString		m_strStdfFileName;
	QStringList	m_lstStdfFileName;
	bool		m_bSplitMultiLot;
	bool		m_bHaveMultiLot;
    GS::StdLib::Stdf		m_StdfFile;
	QString		m_strLotId;
	int			m_nEventRecordsCount[NumberOfEventTypes];


	QString		m_strDataFilePath;
	long		m_lStartTime;				// Startup time (seconds)
	long		m_lEndTime;					// End time	(seconds)
	long		m_lStartMicroSeconds;		// with micro seconds
	long		m_lEndMicroSeconds;			// with micro seconds
	char*		m_szBlock;
	int			m_nBlockSize;
	int			m_nBlockData;
	int			m_nBlockIndex;

	bool		m_bMirWritten;
	bool		m_bMrrWritten;
	QMap<QString,bool>	m_bPirWritten;
	
	int			m_iTotalGoodBin;
	int			m_iTotalFailBin;
	int			m_iTotalTests;
	int			m_nPartNumber;
	QMap<QString,int> m_mapPartNumber;
	int			m_nXLocation;
	int			m_nYLocation;
	int			m_nHardBin;
	int			m_nSoftBin;
	int			m_nPassStatus;
	QStringList	m_lstSites;

	QString		m_strTestName;
	QString		m_strTestSuiteName;
	QString		m_strUnit;
	float		m_fTestResult;
	bool		m_bTestResult;
	int			m_nScale;
	int			m_nTestType;
	int			m_nTestPFResult;
	QStringList m_lstPinIndex;
	QStringList m_lstPinResult;
	QStringList m_lstPinStat;

	QMap<QString,QString> m_mapAssignment;
	QStringList	m_lstGenericData; // first pos = type then = lengh then = value

	int			m_iTestNumber;
	bool		m_bWaferMap;
	QString		m_strWaferId;
	QString		m_strEquipmentID;
	QString		m_strProber;

	QMap<int, CGVERIGY_EDLPinInfo*> m_qMapPins;
	QMap<int, CGVERIGY_EDLBinInfo*> m_qMapBins;
	QMap<int, CGVERIGY_EDLBinInfo*> m_qMapBins_Soft;
  QStringList m_lstSoftBinIssue;

	// if no TestId info (TestNumber unavailable)
	QMap<QString, CGVERIGY_EDLParameter*> m_qMapParameterList;	// List of Parameters
	bool		m_bIgnoreTestNumber;

	QMap<QString, CGVERIGY_EDLSiteInfo*>  m_mapSiteInfo;

};


#endif
