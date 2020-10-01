#ifndef GEX_IMPORT_PCM_CYPRESS_H
#define GEX_IMPORT_PCM_CYPRESS_H

#include <qdatetime.h>
#include <qstringlist.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"

class CGPcmCypressParameter
{
public:

    CGPcmCypressParameter();

    QString         mName;                  // Parameter name
    int             mNumber;
    bool            mStaticHeaderWritten;   // 'true' after first STDF PTR static header data written.
};

class CGPcmCypresstoSTDF
{
public:
    CGPcmCypresstoSTDF();
    ~CGPcmCypresstoSTDF();
    bool    Convert(const char *PcmCypressFileName, QStringList &lstFileNameSTDF);
    QString GetLastError();

    static bool	IsCompatible(const char *szFileName);

private:
    void LoadParameterIndexTable(void);
    void DumpParameterIndexTable(void);
    int	 UpdateParameterIndexTable(QString strParamName);
    void SaveParameter(int iIndex,QString strName);
    bool ReadPcmCypressFile(const char *PcmCypressFileName,QStringList &lstFileNameSTDF);
    bool WriteStdfFile(QTextStream *hPcmCypressFile, QStringList &lstFileNameSTDF);

    QString ReadLine(QTextStream& hFile);

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    int	mProgressStep;
    int	mNextFilePos;
    int	mFileSize;
    int mFileLine;

    QString     mDataFilePath;
    QString     mStdfFileName;

    QString mLastError;                     // Holds last error string during PcmCypress->STDF convertion
    int     mLastErrorCode;                 // Holds last error ID
    enum  errCodes
    {
        errNoError,                         // No erro (default)
        errOpenFail,                        // Failed Opening PcmCypress file
        errInvalidFormat,                   // Invalid PcmCypress format
        errLicenceExpired,                  // File date out of Licence window!
        errWriteSTDF                        // Failed creating STDF intermediate file
    };

    QString     mLotId;                     // LotID
    QString     mWaferId;                   // WaferID
    QString     mProductId;                 // ProductID
    QString     mProcessId;                 // ProcessID
    long        mStartTime;                 // Startup time

    int         mParametersOffset;
    int         mTotalParameters;
    bool        mNewParameterFound;         // set to true if PcmCypress file has a Parameter name not in our reference table=> makes it to be updated
    QStringList mFullParametersList;        // Complete list of ALL PcmCypress parameters known.
    CGPcmCypressParameter* mParameterList;  // List of Parameters in Wafer
};


#endif
