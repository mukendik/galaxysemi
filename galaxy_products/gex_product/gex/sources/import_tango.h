#ifndef GEX_IMPORT_TANGO_H
#define GEX_IMPORT_TANGO_H

#include <qmap.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"

class CTANGOBin
{
public:
    CTANGOBin(int iSWBin=-1,	int iHWBin=-1,	int iPF=-1, QString strBinDesc="",int nCnt=0);
    // parameters
    int     iSBin;
    int     iHBin;
    int     iPass;
    int     iCnt;
    QString strBinName;
};

class CTANGOtoSTDF
{
public:
    CTANGOtoSTDF();
    ~CTANGOtoSTDF();
    bool    Convert(const char *TangoFileName, const char *strFileNameSTDF);
    QString GetLastError();

    static bool IsCompatible(const char *szFileName);

private:
    bool    ReadTangoFile(const char *TangoFileName);
    bool    WriteStdfFile(const char *strFileNameSTDF);

    bool    GotoMarker(const char *szMarker, QString *pstrValue=NULL);
    bool    NextMarker(QString *pstrNextMarker);

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    int     iProgressStep;
    int     iFileSize;
    int     iNextFilePos;

    bool    ProcessLimits();
    bool    ProcessFTData();
    bool    ProcessBinSum();
    bool    ProcessCatSum();
    bool    ProcessDieData();
    bool    ProcessBinMap();

    // For File Access
    QTextStream *m_pTangoFile;
    QString      m_strLine;

    // For Error Manager
    QString m_strLastError;                 // Holds last error string during TANGO->STDF convertion
    int		m_iLastError;                   // Holds last error ID
    enum  errCodes
    {
        errNoError,                         // No erro (default)
        errOpenFail,                        // Failed Opening TANGO file
        errInvalidFormat,                   // Invalid TANGO format
        errLicenceExpired,                  // File date out of Licence window!
        errWriteSTDF                        // Failed creating STDF intermediate file
    };

    // STDF information
    QString m_strWaferID;           // WaferID
    QString m_strLotID;             // LotID
    QString m_strSubLotID;          // SubLotID
    QString m_strNodeName;          // MIR.NODE_NAM
    QString m_strOperName;          // MIR.OPER_NAM
    QString m_strJobName;           // MIR.JOB_NAME
    QString m_strPartType;          // MIR.PART_TYP
    QString m_strTesterType;        // MIR.TSTR_TYP
    QString m_strTestTemperature;   // MIR.TSTR_TEMP
    QString m_strTestingCode;       // MIR.TEST_COD
    QString m_strPkgType;           // MIR.PKG_TYP
    QString m_strFacility;          // MIR.FACIL_ID
    char    m_cWaferFlat;           // WCR.WF_FLAT
    char    m_cPosX;                // WCR.POS_X
    char    m_cPosY;                // WCR.POS_Y
    long    m_lStartTime;           // Startup time
    long    m_lEndTime;             // End time
    char    m_cRetestIndex;         // MIR.RTST_COD

    bool    m_bFinalTest;
    int     m_iTotalParameters;
    int     m_iParameterCount;
    int     m_iNextParameter;
    bool    m_bBinSummary;

    QMap<int,CTANGOBin*>    m_mapHbrList;
    QMap<int,CTANGOBin*>    m_mapSbrList;
    QMap<QString,QString>   m_mapPartBinning;

};


#endif
