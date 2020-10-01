#ifndef PAT_DB_TRACEABILITY_ABSTRACT_H
#define PAT_DB_TRACEABILITY_ABSTRACT_H

#include <QString>
#include <QStringList>
#include <QMap>

class CPatOutlierPart;
class CGexReport;
class QSqlQuery;

QString TestNumberLabel(int lTestNumber, int lPinIndex);

namespace GS
{
namespace Gex
{

struct PATTestLimits
{
    int                 mTestNumber;
    int                 mPinIndex;
    QString             mTestName;
    int                 mSite;
    QString             mDistributionShape;
    double              mNFactor;
    double              mTFactor;
    double              mPATLL;
    double              mPATHL;
    double              mMean;
    double              mSigma;
    double              mQ1;
    double              mMedian;
    double              mQ3;
    int                 mRunID;
    int                 mRetestIndex;
};

class PATDbTraceabilityAbstract
{
protected:

    PATDbTraceabilityAbstract(CGexReport * lReportContext, const QString& lDbFilename,
                              const QString& lConnectionName);

public:

    virtual ~PATDbTraceabilityAbstract();

    const QString&  GetDbFilename() const;
    const QString&  GetErrorMessage() const;

    virtual bool    QueryRecipeID(const QString& lProductId, const QString& lLotId,
                                  const QString& lSublotId, int &lRecipeID);
    virtual bool    QueryRecipeContent(int lRecipeID,
                                       QString &lRecipeName, QString &lRecipeContent);
    bool            QuerySplitLotCount(int &lSplitlotCount, const QString& lProductId,
                                       const QString& lLotId, const QString& lSublotId,
                                       int lRetestIndex = -1);
    bool            QueryTotalParts(const QString& lProductId, const QString& lLotId,
                                    const QString& lSublotId, int &lTested, int& lRetested);
    bool            QueryOutliers(QList<CPatOutlierPart*>& lOutlierList,
                                  QMap<QString, int>& lOutliersCount,
                                  QStringList& lWarnings);
    bool            QueryDPATTestLimits(QList<PATTestLimits>& lTestLimits);
    bool            QuerySPATTestLimits(QList<PATTestLimits>& lTestLimits);
    bool            QueryRollingLimits(QStringList& lWarnings);
    virtual bool    QueryWarningsLog(QStringList& lWarnings);

    static PATDbTraceabilityAbstract * CreateDbTraceability(CGexReport * lReportContext,
                                                            const QString& lDbFilename,
                                                            QString &lErrorMessage);

    enum DbVersion
    {
        DBVersion1  = 1,
        DBVersion2  = 2,
        DBVersion3  = 3
    };

protected:

    const QString&  GetConnectionName() const;
    CGexReport *    GetReportContext() const;

    virtual QString BuildQueryOutliers() const = 0;
    virtual QString BuildQueryDPATTestLimits() const = 0;
    virtual QString BuildQuerySPATTestLimits() const = 0;
    virtual QString BuildQueryRollingLimits() const = 0;

    virtual bool    ParseQueryOutliers(QSqlQuery &lDbQuery,
                                       QList<CPatOutlierPart *>& lOutliersList,
                                       QMap<QString, int> &lOutliersCount,
                                       QStringList &lWarnings);
    virtual bool    ParseQueryPATTestLimits(QSqlQuery& lDbQuery,
                                            QList<PATTestLimits>& lTestLimits);
    virtual bool    ParseQueryRollingLimits(QSqlQuery& lDbQuery,
                                            QStringList& lWarnings);

    QString         mErrorMessage;

private:

    CGexReport *    mReportContext;
    QString         mDbFilename;
    QString         mConnectionName;
};

}   // namespace Gex
}   // namespase GS

#endif // PAT_DB_TRACEABILITY_ABSTRACT_H
