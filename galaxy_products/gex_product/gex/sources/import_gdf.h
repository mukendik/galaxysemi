// import_gdf.h: interface for the CGGDFtoSTDF class.
//
//////////////////////////////////////////////////////////////////////

#ifndef GEX_IMPORT_GDF_H
#define GEX_IMPORT_GDF_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qdatetime.h>
#include <math.h>
#include <QTextStream>
#include "gex_constants.h"
#include "stdf.h"


#define WAFER_ID 1
#define PACKAGE_ID 2

#define BIT0 1
#define BIT1 2
#define BIT2 4
#define BIT3 8
#define BIT4 16
#define BIT5 32
#define BIT6 64
#define BIT7 128

#define FIRST_BIN_NON_NUMBER 32001

class CGGdfDutResult
{
public:
    CGGdfDutResult();
    void reset();

// parameters
    UINT    nFlags; // 1 = WAFER_ID
                    // 2 = PACKAGE_ID

    bool    bSite;  // true if have site info

    int     X;          // for wafer
    int     Y;          // for wafer
    int     Pkg_id;     // for package
    int     SW_Bin;
    int     HW_Bin;
    int     P_F;
    long    Time;       // in second
    float   Temp;
    int     FF;
    int     TF;
    int     TT;
    int     Site;       // if site
    QString Bin_desc;
    int     Retest;
};

class CGGdfTestResult
{
public:
    CGGdfTestResult();
    void reset();

    int         test_num;
// parameters
    QString     test_name;
    QString     seq_name;
    int         Result;
    QStringList ResUnit;
    QString     VectorName;
    QString     PinName;
    QString     LimitName;
};

class CGGdfSummary
{
public:
    CGGdfSummary();
    void reset();

// parameters
    int     SW_Bin;
    int     HW_Bin;
    int     P_F;
    int     Number;
    float   Prcnt;
    int     NumberOfSite;   // 0 if no site info
    float   *pSite;
    QString Bin_desc;
};

class CGGdfSbr
{
public:
    CGGdfSbr(int iSW_Bin=-1,    int iP_F=0, int iNumber=0,float fPrcnt=0, QString strBin_desc="");
// parameters
    int     SW_Bin;
    int     P_F;
    int     Number;
    float   Prcnt;
    QString Bin_desc;
};

class CGGdfPartCnt
{
public:
    CGGdfPartCnt();
    void reset();

// parameters
    bool    bSite;  // true if have site info

    int     Site;
    int     Number;
    int     Prcnt;
};



class CGGdfResults
{
public:
    CGGdfResults();

    int     exec_cnt;
    int     fail_cnt;
    int     alarm_tests;
    UINT    opt_flag;
    BYTE    pad_byte;
    float   test_min;
    float   test_max;
    float   tst_mean;
    float   tst_sdev;
    float   tst_sums;
    float   tst_sqrs;
};

class CGGdfPin
{
public:
    CGGdfPin();
    QString     testNameSuffix;
    int         test_num;
    CGGdfResults results;
};

class CGGdfLimit
{
public:
    CGGdfLimit();
    char    opt_flag;
    QString units;
    int     llm_scal;
    int     hlm_scal;
    float   lo_limit;
    QString low_limit_param;
    float   hi_limit;
    QString high_limit_param;
    float   lo_spec;
    float   hi_spec;
    QString limitName;

    bool SetLimit(QString strLowUnits, float fLowValue,
                  QString strHighUnits, float fHighValue,
                  QString strLowParam="", QString strHighParam="");
};

class CGGdfTest
{
public:
    CGGdfTest();
    ~CGGdfTest();

    bool            bTestSaved;
    bool            bLimitSaved;
    QList<int>      lstLimitSavedForSites;
    int             test_num;   // save the test_num to verify if the num is the same for each run
    QString         test_name; // == suite name
    QString         seq_name;
    QString         vector_name;
    unsigned char   spreadFlag;
    QMap<QString,CGGdfPin*>     pin_list;
    QMap<QString,CGGdfLimit*>   limit_list;
    int             nTestType; // 1=PTR 2=MPR 3=FTR
    int             nPinCount;
    bool            bOnlySaveMinMaxPinResult;

    CGGdfPin*   GetPin(int iIndex=0, QString strPinName=""); // return the good pin or create it if not exist
    CGGdfLimit* GetLimit(int iIndex=0, QString strLimitName=""); // return the good pin or create it if not exist
    bool        HaveLimit(int iIndex=0, QString strLimitName="");

private:
    Q_DISABLE_COPY(CGGdfTest)       // please define correctly = operator and copy constructor if necessary
};

class CGGdfPinChannel
{
public:
    CGGdfPinChannel();
    ~CGGdfPinChannel();
    void reset();

    int         iBitPosition;
    QStringList lstChannel;
    QString     strPinName;

};

class CGGDFtoSTDF
{
public:
    enum  errCodes
    {
        errNoError,         // No erro (default)
        errWarningEndOfFile,            // File generated with warning
        errErrorOpenFail,       // Failed Opening GDF file
        errErrorInvalidFormat,  // Invalid GDF format
        errErrorEndOfFile,  // Invalid GDF format
        errErrorWriteSTDF       // Failed creating STDF intermediate file
    };

    CGGDFtoSTDF();
    virtual ~CGGDFtoSTDF();

    int     Convert(const char *szFileNameGdf, const char *szFileNameSTDF, bool bModeDatabase = false, bool bMonitoringMode = false);
    //! \brief Get Error ?
    QString GetLastError();

    //! \brief Check if File is compatible with GDF format
    static bool IsCompatible(const char *szFileName);

private:
    Q_DISABLE_COPY(CGGDFtoSTDF) // please define correctly = operator and copy constructor if necessary
    bool ParseGdfFile();
    bool ParseGdfCode();

    bool    m_bModeDatabase; // true if we want to ignore all test number and used only the table index saved
    bool    m_bMonitoringMode; // true if we are in monitoring or database mode (no warning possible)

    bool    m_bAcceptMprDynamicPins; // For MPR with several count for PinResult

    QStringList m_lstTestName;

    //! \brief Load GDF Parameter table from DISK
    void LoadParameterIndexTable(void);
    void DumpParameterIndexTable(void);
    void FindParameterIndex(QString strParamName);

    QString ReadLine();
    QString m_strLine;
    UINT    m_nCurrentLine;

    QStringList m_stackMarkerTag;
    QStringList m_stackMarkerLine;

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    int m_iProgressStep;
    int m_iNextFilePos;
    int m_iFileSize;

    QString m_strLastError; // Holds last error string during GDF->STDF convertion
    int     m_iLastError;           // Holds last error ID
    QString m_strLastErrorMsg;
    QFile*          m_pfGdf;
    QTextStream*    m_phGdfFile;
    GS::StdLib::Stdf*           m_pStdfFile;

    bool    AnalyzeSoftBinNameMapping();
    int     GetSoftBinMappingNumber(QString strBinString,int iDefaultBin=-1);
    QString GetSoftBinMappingName(int iSoftBin, QString strBinString);
    //! \brief Analyze DUT result: will probably call AnalyzeTestResult(), WritePir(), WritePrr()...
    bool    AnalyzeDutResult();
    //! \brief Analyse test result: will call WriteResult()
    bool    AnalyzeTestResult();
    bool    AnalyzeSummary();
    bool    AnalyzePartCnt();
    bool    AnalyzeUserData();
    bool    AnalyzePinList(QString strEndMarker);
    bool    AnalyzeDataPins();
    bool    AnalyzeLimitData();

    bool    GotoMarker(const char *szEndMarker);

    // information collected
    bool    m_bMirWriten;   // before to write the MIR, we have to collect the max of info
    int     m_nPass;        // 2 pass for MPR and PMR records
    bool    m_bGotoNextPass;

    long    m_lSetupT;
    long    m_lStartT;
    int     m_nStatNum;
    char    m_cModeCod;
    char    m_cRtstCod;
    char    m_cProtCod;
    long    m_lBurnTim;
    char    m_cCModCod;
    QString m_strLotId;
    QString m_strPartTyp;
    QString m_strNodeNam;
    QString m_strTstrTyp;
    QString m_strJobNam;
    QString m_strJobRev;
    QString m_strSbLotId;
    QString m_strOperNam;
    QString m_strExecTyp;
    QString m_strExecVer;
    QString m_strTestCod;
    QString m_strTstTemp;
    QString m_strUserTxt;
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
    QString m_strHandType;
    QString m_strHandId;
    QString m_strCardId;
    QString m_strLoadId;
    QString m_strExtrId;

    QString m_strWaferId;
    int     m_nWaferNumDie;
    int     m_nWaferNumPassed;
    int     m_nWaferNumRetested;

    int     m_nTotalParts;
    int     m_nTotalPassed;
    int     m_nTotalRetested;

    QString m_strUserDesc;
    QString m_strExecDesc;
    int     m_nDispCode;

    long    m_lLastPartTime;

    QList<int>      m_lstSites;

    CGGdfDutResult  m_clGdfDutResult;
    CGGdfSummary    m_clGdfSummary;
    CGGdfPartCnt    m_clGdfPartCnt;
    CGGdfTestResult m_clGdfTestResult;
    QList<CGGdfSbr*>    m_clSbrList;
    QMap<int,CGGdfPinChannel*>  m_mapPinChannel;
    QMap<QString,int>           m_mapPinNum;
    QMap<QString,CGGdfTest*>    m_mapTest;
    int                         m_nMaxTestNum;
    QMap<QString,int>           m_mapTestsFlow;
    CGGdfTest* GetTest(QString strSuite); // create it if not exist
    QMap<QString,QString> m_mapVarValue;
    QStringList m_bin_name_mapping; // List of SoftBin strings (we map them to a SoftBin ourselves)
    QMap<QString,QString> m_mapSoftBinDesc;
    QStringList mBinNonNumber;


    bool    SetLimit(CGGdfTest*pclTest, int iIndex, QString strLimitName, QString strLowUnit, QString strLowValue, QString strHighUnit, QString strHighValue);
    float   ToFloat(QString strVar, bool *pbFloat=NULL);
    QString ToValue(QString strVar);

    bool    IsFunctionalTestSequence(QString &strSequenceName);

    // STDF functions
    void WriteFar();
    void WriteMir();
    void CheckWriteMir();
    void WriteWir();
    void WritePir();

    bool WriteResult();

    void WritePtr(CGGdfTestResult* pclTestResultParam=NULL, CGGdfTest*  pclTestParam=NULL);
    void WriteMpr(CGGdfTestResult* pclTestResultParam=NULL, CGGdfTest*  pclTestParam=NULL);
    void WriteFtr(CGGdfTest*    pclTestParam=NULL);
    void WritePrr();
    void WriteWrr();
    void SaveSbr();
    void WriteSbr();
    void WriteHbr();
    void WritePcr();
    void WriteMrr();
    void WritePmr();
    void WriteTsr();

};

int     PrefixUnitToScall(QString strUnit);
QString PrefixUnitToUnit(QString strUnit);
int     StringToHex(QString strHexa);
#endif // #ifndef GEX_IMPORT_GDF_H
