#ifndef GEX_IMPORT_VERIGY_EDF_H
#define GEX_IMPORT_VERIGY_EDF_H

#include <qmap.h>
#include <qdatetime.h>
#include <qstringlist.h>

#include "gex_constants.h"
#include "stdf.h"

class CVerigyEdfBinInfo
{
public:
    CVerigyEdfBinInfo();

    QMap<int,int>   mapSiteNbCnt;
    int             nPassFlag;
};

class CVerigyEdfPartInfo
{
public:
    CVerigyEdfPartInfo()
    {
        nXWafer = nYWafer = -32768;
        nBinning = 1;
        bPassStatus = true;
        bPassStatusValid = false;
        nTotalTests = 0;
    }

    int     nXWafer;
    int     nYWafer;
    QString strPartId;
    int     nBinning;
    bool    bPassStatusValid;
    bool    bPassStatus;
    long    nTotalTests;
};

class CGVERIGY_EDFtoSTDF
{
public:
    CGVERIGY_EDFtoSTDF();
    ~CGVERIGY_EDFtoSTDF();
    bool            Convert(const char *VerigyEdfFileName, const char *strFileNameSTDF);
    QString         GetLastError();

    static bool     IsCompatible(const char *szFileName);

private:

    long GetDateTimeFromString(QString strDateTime);
    void LoadParameterIndexTable(void);
    void DumpParameterIndexTable(void);
    void UpdateParameterIndexTable(QString strParamName);
    void NormalizeLimits(QString &strUnit, int &nScale);
    bool ReadVerigyEdfFile(const char *VerigyEdfFileName, const char *strFileNameSTDF);
    bool WriteStdfFile(FILE *fp,const char *strFileNameSTDF);

    int ReadLine(FILE *fp, char *bp);

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    int iProgressStep;
    int iNextFilePos;
    int iFileSize;

    QString m_strLastError;                 // Holds last error string during VERIGY_EDF->STDF convertion
    int     m_iLastError;                   // Holds last error ID
    enum  errCodes
    {
        errNoError,                         // No erro (default)
        errOpenFail,                        // Failed Opening VERIGY_EDF file
        errInvalidFormat,                   // Invalid VERIGY_EDF format
        errLicenceExpired,                  // File date out of Licence window!
        errWriteSTDF                        // Failed creating STDF intermediate file
    };

    QString     m_strWaferId;               // WaferID
    QString     m_strTesterId;              // TesterID
    QString     m_strLotId;                 // LotID
    QString     m_strProcessId;             // ProcessID
    QString     m_strProgramId;             // ProgramID
    QString     m_strStationId;             // StationID
    long        m_lStartTime;               // Startup time
    bool        m_bNewVerigyEdfParameterFound;  // set to true if VERIGY_EDF file has a Parameter name not in our reference table=> makes it to be updated
    QStringList m_pFullVerigyEdfParametersList; // Complete list of ALL VerigyEdf parameters known.
    QMap<int, CVerigyEdfBinInfo*> m_mapBins;
    QMap<int, CVerigyEdfPartInfo*> m_mapSitePartsInfo;
    QMap<QString, int> m_mapPartsIdExec;
};


#endif
