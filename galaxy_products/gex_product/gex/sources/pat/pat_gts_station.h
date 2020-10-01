#ifndef PAT_GTS_STATION_H
#define PAT_GTS_STATION_H

#include <QString>

#include "stdfrecords_v4.h"
#include "stdfparse.h"
#include <QObject>

class CBinning;

namespace GS
{
namespace Gex
{

class PATGtsStation : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PATGtsStation)

public:

    PATGtsStation(QObject * lParent = NULL);
    ~PATGtsStation();

    Q_INVOKABLE QString         GetErrorMessage() const;

    Q_INVOKABLE void            SetInputDataFile(const QString& lInputData);
    Q_INVOKABLE void            SetOutputDataFile(const QString& lOutputData);
    Q_INVOKABLE void            SetRecipeFile(const QString& lRecipe);
    Q_INVOKABLE void            SetTesterConf(const QString& lTesterConf);
    Q_INVOKABLE void            SetTraceabilityFile(const QString& lTraceabilityFile);

public slots:

    bool                        ExecuteTestProgram();

signals:

    void    aborted(const QString& lErrorMessage);
    void    started();
    void    finished(const QString& lOutputDataFile, const QString& lTraceabilityFile);
    void    dataReadProgress(const QString& lMessage, qint64 lDone, qint64 lTotal);

protected:

    bool    CloseTestProgram();
    bool    InitStation();
    bool    IsValidSTDF();
    bool    OpenTestProgram();
    bool    ProcessFile();
    bool    ProcessEOR();
    bool    ProcessMPR();
    bool    ProcessPIR(bool &lEOR, bool& lUseLastPIR, bool& lPartDetected);
    bool    ProcessPRR(bool &lEOR);
    bool    ProcessPTR();
    bool    WriteBinRecords();
    bool    ConvertToSTDF();

    enum SiteStatus
    {
        eSiteIdle,
        eSiteRunning,
        eSiteProcessed
    };

private:
    QString                     mInputDataFile;
    QString                     mInputSTDFFile;
    QString                     mOutputSTDFFile;
    QString                     mTraceabilityFile;
    QString                     mRecipeFile;
    QString                     mTesterConf;
    QString                     mErrorMessage;
    bool                        mGenerateOutputTestData;
    bool                        mBinRecordsCreated;
    long                        mPartCount;
    long                        mPartRead;
    GQTL_STDF::Stdf_MIR_V4		mMIRRecord;
    GQTL_STDF::Stdf_PIR_V4		mLastPIRRecord;
    GQTL_STDF::StdfParse	mSTDFReader;
    GQTL_STDF::StdfParse	mSTDFWriter;
    QMap<int, SiteStatus>       mSites;
    QMap<int, CBinning *>       mSoftBinning;
    QMap<int, CBinning *>       mHardBinning;

};

}
}

#endif // PAT_GTS_STATION_H
