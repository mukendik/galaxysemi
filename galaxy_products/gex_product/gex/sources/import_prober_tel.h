#ifndef GEX_IMPORT_PROBER_TEL_H
#define GEX_IMPORT_PROBER_TEL_H

#include <qmap.h>
#include <qdatetime.h>
#include <qstringlist.h>

#include "gex_constants.h"
#include "stdf.h"
#include "gs_types.h"

#define PROBER_TEL_BLOCK_SIZE	1536


class CGProberTelBinInfo
{
public:
    int		nBinNumber;
    bool	bPass;
    int		nNbCnt;
};

class CGProberTelBaseToSTDF
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
        errOpenFail,						// Failed Opening ProberTel file
        errConversionFailed,				// Not compatible
        errLicenceExpired,					// File date out of Licence window!
        errWriteSTDF						// Failed creating STDF intermediate file
    };


    CGProberTelBaseToSTDF();
    virtual ~CGProberTelBaseToSTDF();

    bool		Convert(const char *ProberTelFileName, QString &strFileNameSTDF);
    QString		GetLastError();
    int			GetLastErrorCode() {return m_iLastError;}

protected:

    int         ReadBlock(QFile* pFile, char *data, qint64 len);
    int         ReadProberTelFile(const char *strFileNameSTDF);
    bool        WriteStdfFile(QFile &f, const char *strFileNameSTDF);

    virtual bool ReadDieData(char * data, int len, int dieIndex, gsint32& bin, BYTE& status) = 0;
    virtual bool ReadDataRecords(QFile& inputFile) = 0;
    virtual int	ReadWaferDataRecord(QFile& inputFile) = 0;
    virtual int	ReadTestTotalRecord(QFile& inputFile) = 0;
    virtual int ReadMapHeaderRecord(QFile& inputFile) = 0;
    virtual int ReadMapRowDataRecord(QFile& inputFile, int totalDies, char ** data, int& len) = 0;
    virtual int	ReadMapRowHeaderRecord(QFile& inputFile, int& origX, int& origY, int& dieCount) = 0;

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    int         iProgressStep;
    int         iNextFilePos;
    int         iFileSize;

    QString     m_strLastError;					// Holds last error string during ProberTel->STDF convertion
    int         m_iLastError;					// Holds last error ID

    QString		m_strProberTelFileName;

    QChar		m_strRtstCod;
    QString		m_strLotId;
    QString		m_strWaferId;
    QString		m_strProductId;
    QString		m_strSubLotId;

    QString		m_strDataFilePath;
    gsint32		m_lStartTime;				// Startup time (seconds)
    gsint32		m_lEndTime;					// End time	(seconds)
    int         mTotalRecords;

    QMap<int, CGProberTelBinInfo> m_mapBinInfo;
};

class CGProberTelThousandToSTDF : public CGProberTelBaseToSTDF
{
public:

    CGProberTelThousandToSTDF();
    ~CGProberTelThousandToSTDF();

    static bool         IsCompatible(const char *szFileName);
    static QDateTime    ByteArrayToDate(const char *data, int &index);

protected:

    bool    ReadDieData(char * data, int len, int dieIndex, gsint32& bin, BYTE& status);
    bool    ReadDataRecords(QFile& inputFile);
    int		ReadWaferDataRecord(QFile& inputFile);
    int		ReadTestTotalRecord(QFile& inputFile);
    int     ReadMapHeaderRecord(QFile& inputFile);
    int     ReadMapRowDataRecord(QFile& inputFile, int totalDies, char ** data, int& len);
    int     ReadMapRowHeaderRecord(QFile& inputFile, int& origX, int& origY, int& dieCount);
};

class CGProberTelMillionToSTDF : public CGProberTelBaseToSTDF
{
public:

    CGProberTelMillionToSTDF();
    ~CGProberTelMillionToSTDF();

    static bool         IsCompatible(const char *szFileName);
    static QDateTime    StringToDate(const QString& lDate);

protected:

    bool    ReadDieData(char * data, int len, int dieIndex, gsint32 &bin, BYTE& status);
    bool    ReadDataRecords(QFile& inputFile);
    int		ReadWaferDataRecord(QFile& inputFile);
    int		ReadTestTotalRecord(QFile& inputFile);
    int     ReadMapHeaderRecord(QFile& inputFile);
    int     ReadMapRowDataRecord(QFile& inputFile, int totalDies, char ** data, int& len);
    int     ReadMapRowHeaderRecord(QFile& inputFile, int& origX, int& origY, int& dieCount);
};

#endif
