#ifndef GEX_IMPORT_PCM_TOWER_H
#define GEX_IMPORT_PCM_TOWER_H

#include <qdatetime.h>
#include <qmap.h>
#include <qstringlist.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"

typedef QMap<int, float> fValueOfSiteMap;

class CGPcmTowerParameter
{
public:
    QString strName;    // Parameter name. E.g: "Idd_Total_Shutdown"
    QString strUnits;   // Parameter units,E.g: "A"
    float   fLowLimit;  // Parameter Low Spec limit, E.g: 0.00004
    float   fHighLimit; // Parameter High Spec limit, E.g: -0.00004
    bool    bValidLowLimit; // Low limit defined
    bool    bValidHighLimit;// High limit defined
    float   fValue;     // Parameter result
    bool    bStaticHeaderWritten;// 'true' after first STDF PTR static header data written.
};


class CGPCM_TOWERtoSTDF
{
public:
    CGPCM_TOWERtoSTDF();
    ~CGPCM_TOWERtoSTDF();
    bool    Convert(const char *PcmTowerFileName, const char *strFileNameSTDF);
    QString GetLastError();

    static bool IsCompatible(const char *szFileName);

private:

    void LoadParameterIndexTable(void);
    void DumpParameterIndexTable(void);
    void UpdateParameterIndexTable(QString strParamName);
    bool ReadPcmTowerFile(const char *PcmTowerFileName, const char *strFileNameSTDF);
    bool WriteStdfFile(QTextStream *hPcmTowerFile,const char *strFileNameSTDF);
    long GetDateTimeFromString(QString strDateTime);

    QString ReadLine(QTextStream& hFile);

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    int	iProgressStep;
    int	iNextFilePos;
    int	iFileSize;

    QString strLastError;   // Holds last error string during PCM_TOWER->STDF convertion
    int	iLastError;         // Holds last error ID
    enum  errCodes
    {
        errNoError,         // No erro (default)
        errOpenFail,        // Failed Opening PCM_TOWER file
        errInvalidFormat,   // Invalid PCM_TOWER format
        errInvalidFormatLowInRows,  // Didn't find parameter rows
        errNoLimitsFound,   // Missing limits...no a valid PCM_TOWER file
        errLicenceExpired,  // File date out of Licence window!
        errWriteSTDF        // Failed creating STDF intermediate file
    };

    bool                    m_bNewPcmTowerParameterFound;   // set to true if PCM_TOWER file has a Parameter name not in our reference table=> makes it to be updated
    CGPcmTowerParameter *   m_pCGPcmTowerParameter;         // List of Parameters tables.
    int                     m_iTotalParameters;             // Holds the total number of parameters / tests in each part tested
    QStringList             m_pFullPcmTowerParametersList;  // Complete list of ALL PCM_TOWER parameters known.

    long    m_lSetupTime;               // Setup time
    long    m_lStartTime;               // Startup time
    long    m_lFinishTime;              // Finsh time
    QString m_strLotID;                 // LotID string
    QString m_strProductID;             // Product / Device name
    QString m_strProgramID;             // Program name
    QString m_strTestCode;
    QString m_strFamilyID;
    QString m_strSpecName;
    QString m_strProcessID;

};


#endif
