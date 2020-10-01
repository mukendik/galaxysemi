#ifndef GEX_IMPORT_FREESCALE_MAXWELL_H
#define GEX_IMPORT_FREESCALE_MAXWELL_H

#include <qdatetime.h>
#include <qmap.h>
#include <qstringlist.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"

typedef QMap<int, float> fValueOfSiteMap;

class CGFreescaleParameter
{
public:
    QString strName;        // Parameter name. E.g: "Idd_Total_Shutdown"
    int     nNumber;        // Parameter number
    QString strUnits;       // Parameter units,E.g: "A"
    float   fLowLimit;      // Parameter Low Spec limit, E.g: 0.00004
    float   fHighLimit;     // Parameter High Spec limit, E.g: -0.00004
    bool    bValidLowLimit; // Low limit defined
    bool    bValidHighLimit;// High limit defined
    float   fValue;         // Parameter result
    bool    bStaticHeaderWritten;  // 'true' after first STDF PTR static header data written.

    // Associated binning
    int     nHardBin;
    int     nSoftBin;

    // For Register parameter
    // Associated Register
    // The Fused and Unfused must be equal
    bool    bRegisterParameter;      // If result are in Hexa or Float
    int     nAssociatedIndex;
};

class CGFreescaleBinning
{
public:
    int		nNumber;	// Bin number
    QString strName;	// Bin name
    bool	bPass;		// Bin cat
    int		nCount;		// Bin count
};

class CGFreescaletoSTDF
{
public:
    CGFreescaletoSTDF();
    ~CGFreescaletoSTDF();
    bool	Convert(const char *FreescaleFileName, const char *strFileNameSTDF);
    QString GetLastError();

    static bool	IsCompatible(const char *szFileName);

private:

    void LoadParameterIndexTable(void);
    void DumpParameterIndexTable(void);
    void UpdateParameterIndexTable(QString strParamName);
    bool ReadFreescaleFile(const char *FreescaleFileName, const char *strFileNameSTDF);
    bool ReadFreescaleBinmapFile(QString &strDataFilePath);
    bool WriteStdfFile(QTextStream *hFreescaleFile,const char *strFileNameSTDF);

    QString ReadLine(QTextStream& hFile);

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    int	iProgressStep;
    int	iNextFilePos;
    int	iFileSize;

    QString strLastError;     // Holds last error string during Freescale->STDF convertion
    int	iLastError;           // Holds last error ID
    enum  errCodes
    {
        errNoError,                 // No erro (default)
        errOpenFail,                // Failed Opening Freescale file
        errInvalidFormat,           // Invalid Freescale format
        errInvalidFormatLowInRows,  // Didn't find parameter rows
        errNoLimitsFound,           // Missing limits...no a valid Freescale file
        errMissingData,             // Missing mandatory data
        errLicenceExpired,          // File date out of Licence window!
        errWriteSTDF                // Failed creating STDF intermediate file
    };

    // Paremeters info
    bool                     m_bNewFreescaleParameterFound;   // set to true if Freescale file has a Parameter name not in our reference table=> makes it to be updated
    CGFreescaleParameter *   m_pCGFreescaleParameter;         // List of Parameters tables.
    int                      m_iTotalParameters;              // Holds the total number of parameters / tests in each part tested
    int                      m_iIndexParametersOffset;
    QStringList              m_pFullFreescaleParametersList;  // Complete list of ALL Freescale parameters known.

    // Bin info
    bool          m_bHaveBinmapFile;
    QStringList   m_lstSoftBin;       // Use this list when no mapping is possible

    long      m_lStartTime;           // Startup time
    QString   m_strLotID;             // LotID string
    QString   m_strProductID;         // Product / Device name
    QString   m_strProgramID;         // Program name

};


#endif
