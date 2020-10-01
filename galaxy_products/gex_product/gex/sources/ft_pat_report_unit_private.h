#ifndef FT_PAT_REPORT_UNIT_PRIVATE_H
#define FT_PAT_REPORT_UNIT_PRIVATE_H

#include "pat_recipe.h"
#include "pat_db_traceability_abstract.h"
#include <QMap>
#include <QStringList>

class CPatOutlierPart;

namespace GS
{
namespace Gex
{

class FTPATReportUnit;

class FTPATReportUnitPrivate
{
public:

    FTPATReportUnitPrivate(FTPATReportUnit * parent);
    ~FTPATReportUnitPrivate();

    const QString&  GetErrorMessage() const;

    bool            Init();
    bool            CheckForNewPage(const QString& lPageName, const QString &lBookmark,
                                    const QString &lSectionTitle);
    bool            CreateHeaderPage();
    bool            CreatePartsFailingPATLimitsPage();
    bool            CreateSPATTestsLimitsPage();
    bool            CreateDPATResultsPage();
    bool            CreateDPATTestsLimitsPage();
    bool            CreateTestsFailingPATLimitsPage();
    bool            CreateWarningLogsPage();

protected:

    bool                IsTestMatch(unsigned long TestNumber, long PinIndex,
                                   const QString& TestName,
                                   const PATTestLimits &PatTestLimit);
    CPatDefinition *    FindUnivariateRule(QList<CPatDefinition*> UnivariateRules,
                                           const PATTestLimits& PatTestLimit);

    void                BuildHeaderPartsFailingPATLimits();
    void                BuildHeaderTestsFailingPATLimits();

    void                WriteBackwardLink();
    void                WriteReportEmptyLine();
    void                WriteReportLine(const QString& lDecoration,
                                        const QString& lText = QString());
    void                WriteReportLine(const QString &lDecoration,
                                        const QStringList& lText);
    void                WriteTableCell(const QString& lDecoration,
                                       const QString& lText = QString(),
                                       bool lLast = false);
    void                WriteTableCell(const QString& lDecoration,
                                       const QStringList& lText,
                                       bool lLast = false);

private:

    FTPATReportUnit *                   mParent;

    int                                 mPartsTested;
    int                                 mPartsRetested;
    QString                             mErrorMessage;
    QString                             mOutputFormat;
    QString                             mRecipeName;
    PATRecipe                           mRecipe;
    QList<CPatOutlierPart *>            mOutliers;
    QList<PATTestLimits>                mDPATTestLimits;
    QList<PATTestLimits>                mSPATTestLimits;
    QMap<QString, int>                  mDPATOutliers;      // Outliers count per Test/Site/Tuning
    QStringList                         mWarningLogs;
    QMap<QString, QString>              mReportPagesName;
    bool                                mSplitReport;
    bool                                mSyncData;
};

}
}

#endif // FT_PAT_REPORT_UNIT_PRIVATE_H
