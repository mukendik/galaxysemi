#ifndef GEX_IMPORT_SDI_H
#define GEX_IMPORT_SDI_H

#include <qdatetime.h>
#include <qstringlist.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"

class CGSdiBin
{
public:

    CGSdiBin(QString strValue);

    int     m_nNumber;      // Bin num
    QString m_strName;      // Bin name
    int     m_nCount;
    bool    m_bPassFlag;    // 'true' if pass bin.
};

class CGSditoSTDF
{
public:
    CGSditoSTDF();
    ~CGSditoSTDF();
    bool    Convert(const char *SdiFileName, QStringList &lstFileNameSTDF);
    QString GetLastError();

    static bool IsCompatible(const char *szFileName);

private:

    void LoadParameterIndexTable(void);
    void DumpParameterIndexTable(void);
    int	 UpdateParameterIndexTable(QString strParamName);
    bool ReadSdiFile(const char *SdiFileName);
    bool WriteStdfFile(QTextStream *hSdiFile);

    QString ReadLine(QTextStream& hFile);

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    int iProgressStep;
    int iNextFilePos;
    int iFileSize;

    QString     m_strDataFilePath;
    QString     m_strStdfFileName;
    QStringList m_lstStdfFileName;

    QString m_strLastError;                 // Holds last error string during Sdi->STDF convertion
    int     m_iLastError;                   // Holds last error ID
    enum  errCodes
    {
        errNoError,                         // No erro (default)
        errOpenFail,                        // Failed Opening Sdi file
        errInvalidFormat,                   // Invalid Sdi format
        errLicenceExpired,                  // File date out of Licence window!
        errWriteSTDF                        // Failed creating STDF intermediate file
    };
    enum eLimitType
    {
        eLowLimit,                          // Flag to specify to save the CSM Parameter LOW limit
        eHighLimit                          // Flag to specify to save the CSM Parameter HIGH limit
    };

    QString     m_strLotId;                 // LotID
    QString     m_strWaferId;               // WaferID
    QString     m_strProductId;             // ProductID
    QString     m_strFamilyId;              // FamilyID
    QString     m_strOperator;              // Operator
    QString     m_strContractorId;          // Contractor
    QString     m_strFacilId;
    QString     m_strTesterName;
    QString     m_strTemp;
    QString     m_strSupervisorName;
    QString     m_strTestCod;
    QString     m_strRtstCod;
    QString     m_strJobName;
    QString     m_strJobRev;
    QString     m_strExecVer;
    QString     m_strDibId;
    long        m_lStartTime;               // Startup time
    long        m_lStopTime;                // Startup time

    int         m_nPosDate;
    int         m_nPosTime;
    int         m_nPosWaferId;
    int         m_nPosPartId;
    int         m_nPosBin;
    int         m_nPosBinDesc;
    int         m_nPosXY;

    int         m_nNbItems;

    int         m_iTotalParameters;
    bool        m_bNewSdiParameterFound;    // set to true if Sdi file has a Parameter name not in our reference table=> makes it to be updated
    QStringList m_pFullSdiParametersList;   // Complete list of ALL Sdi parameters known.

    // To extract only the 4 possibles parameters
    QMap<int,QString> m_mapIndexParameters;
};


#endif
