#ifndef GEX_IMPORT_SPEA_DATALOG_H
#define GEX_IMPORT_SPEA_DATALOG_H

#include <qmap.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"


class CSpeaDatalogBinInfo
{
public:
    CSpeaDatalogBinInfo();

    QString strName;
    int     nNbCnt;
    int     nPassFlag;
};

class CGSpeaDatalogtoSTDF
{
public:
    CGSpeaDatalogtoSTDF();
    ~CGSpeaDatalogtoSTDF();
    bool    Convert(const char *SpeaDatalogFileName, const char *strFileNameSTDF);
    QString GetLastError();

    static bool	IsCompatible(const char *szFileName);

private:
    void NormalizeLimits(QString &strUnit, int &nScale);
    bool ReadSpeaDatalogFile(const char *SpeaDatalogFileName, const char *strFileNameSTDF);
    bool WriteStdfFile(QTextStream *hSpeaDatalogFile,const char *strFileNameSTDF);

    QString ReadLine(QTextStream& hFile);
    long GetDateTimeFromString(QString strDateTime);

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    int iProgressStep;
    int iNextFilePos;
    int iFileSize;

    QString m_strLine;

    QString m_strLastError;                 // Holds last error string during SpeaDatalog->STDF convertion
    int     m_iLastError;                   // Holds last error ID
    enum  errCodes
    {
        errNoError,                         // No erro (default)
        errOpenFail,                        // Failed Opening SpeaDatalog file
        errInvalidFormat,                   // Invalid SpeaDatalog format
        errInvalidFormatParameter,          // Invalid SpeaDatalog format: didn't find the 'Parameter' section
        errLicenceExpired,                  // File date out of Licence window!
        errWriteSTDF                        // Failed creating STDF intermediate file
    };

    QString     m_strWaferId;               // WaferID
    QString     m_strTesterId;              // TesterID
    QString     m_strOperatorName;          // OperatorName
    QString     m_strLotId;                 // LotID
    QString     m_strSubLotId;              // LotID
    QString     m_strPackageType;
    QString     m_strPartType;
    QString     m_strHandType;
    QString     m_strHandId;
    QString     m_strLoadId;
    QString     m_strProcessId;
    QString     m_strRomCode;
    QString     m_strProgramId;             // ProgramID
    QString     m_strProgramRev;            // ProgramRev
    QString     m_strNodeId;                // NodeID
    QString     m_strStationId;             // StationID
    QString     m_strTestProgram;           // Test program
    QString     m_strTestRev;               // Test Rev
    QString     m_strSoftwareName;          // Software
    QString     m_strSoftwareRev;           // Software
    QString     m_strTestTemperature;       // Test temperature
    long        m_lStartTime;               // Startup time
    long        m_lStopTime;                // Stop time

    bool        m_bQuoteFormat;
    int         m_iTestNamePos;
    int         m_iPinNamePos;
    int         m_iLowLimitPos;
    int         m_iResultPos;
    int         m_iHighLimitPos;
    int         m_iUnitPos;
    int         m_iFlagPos;

    QMap<QString,int>                   m_qMapPinIndex;
    QMap<int, CSpeaDatalogBinInfo*>     m_qMapHBins;
    QMap<int, CSpeaDatalogBinInfo*>     m_qMapSBins;

    int         GetTestNum(QString strLine,bool &bIsValue);
    QString     GetTestName(QString strLine);
    QString     GetPinName(QString strLine);
    QString     GetPatternResult(QString strLine);
    float       GetLowLimit(QString strLine,bool &bIsValue);
    float       GetResult(QString strLine,bool &bIsValue);
    float       GetHighLimit(QString strLine,bool &bIsValue);
    QString     GetUnit(QString strLine);
    bool        IsPassFlag(QString strLine);
};


#endif
