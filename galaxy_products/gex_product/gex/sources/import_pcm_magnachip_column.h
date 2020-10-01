#ifndef GEX_IMPORT_PCM_MAGNACHIP_COLUMN_H
#define GEX_IMPORT_PCM_MAGNACHIP_COLUMN_H

#include <qmap.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"

typedef QMap<int, float> fValueOfSiteMap;

class CGPcmMagnachipParameter
{
public:
    QString strName;                // Parameter name. E.g: "Idd_Total_Shutdown"
    QString strUnits;               // Parameter units,E.g: "A"
    int     scale;                  // Scale factor for limits
    float   fLowLimit;              // Parameter Low Spec limit, E.g: 0.00004
    float   fHighLimit;             // Parameter High Spec limit, E.g: -0.00004
    bool    bValidLowLimit;         // Low limit defined
    bool    bValidHighLimit;        // High limit defined
    float   fValue;                 // Parameter result
    bool    bStaticHeaderWritten;   // 'true' after first STDF PTR static header data written.
};


class CGPCM_MAGNACHIP_COLUMNtoSTDF
{
public:
    CGPCM_MAGNACHIP_COLUMNtoSTDF();
    ~CGPCM_MAGNACHIP_COLUMNtoSTDF();
    bool    Convert(const char *PcmMagnachipFileName, const char *strFileNameSTDF);
    QString GetLastError();

    static bool IsCompatible(const char *szFileName);

private:
    void LoadParameterIndexTable(void);
    void DumpParameterIndexTable(void);
    void UpdateParameterIndexTable(QString strParamName);
    void NormalizeLimits(int iIndex);
    bool ReadPcmMagnachipFile(const char *PcmMagnachipFileName, const char *strFileNameSTDF);
    bool WriteStdfFile(QTextStream *hPcmMagnachipFile,const char *strFileNameSTDF);

    QString ReadLine(QTextStream& hFile);
    QStringList SplitLine(QString strLine);

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    int     iProgressStep;
    int     iNextFilePos;
    int     iFileSize;

    QString m_strLastError; // Holds last error string during PCM_MAGNACHIP_COLUMN->STDF convertion
    int     m_iLastError;   // Holds last error ID
    enum  errCodes
    {
        errNoError,         // No erro (default)
        errOpenFail,        // Failed Opening PCM_MAGNACHIP_COLUMN file
        errInvalidFormat,   // Invalid PCM_MAGNACHIP_COLUMN format
        errInvalidFormatLowInRows,  // Didn't find parameter rows
        errNoLimitsFound,   // Missing limits...no a valid PCM_MAGNACHIP_COLUMN file
        errLicenceExpired,  // File date out of Licence window!
        errWriteSTDF        // Failed creating STDF intermediate file
    };

    bool                        m_bNewPcmMagnachipParameterFound;   // set to true if PCM_MAGNACHIP_COLUMN file has a Parameter name not in our reference table=> makes it to be updated
    CGPcmMagnachipParameter *   m_pCGPcmMagnachipParameter;         // List of Parameters tables.
    int                         m_iTotalParameters;                 // Holds the total number of parameters / tests in each part tested
    QStringList                 m_pFullPcmMagnachipParametersList;  // Complete list of ALL PCM_MAGNACHIP_COLUMN parameters known.

    long	m_lStartTime;               // Startup time
    QString	m_strLotId;                 // LotId string
    QString	m_strProductId;             // Product / Device name
    QString	m_strProcessId;             // ProcessId
    QString	m_strTesterName;            // TesterName string
    QString	m_strOperatorName;          // Operator Name string
    QString m_strFacilityId;            // Facility Id
    QString m_strSpecificationName;     // SpecName
    QString m_strProgramName;           // JobName
    QString m_strAuxFile;               // AuxFile
    int     m_iStation;                 // Station


};


#endif
