#ifndef PAT_REPORT_FT_H
#define PAT_REPORT_FT_H

#include "pat_recipe.h"

#include <QStringList>
#include <QList>

namespace GS
{
namespace Gex
{

class PATReportFT
{
public:

    PATReportFT();
    ~PATReportFT();

    bool            Generate(const QStringList &outputFiles, const QString& traceabilityFile,
                             const QString& lRecipe);

    const QString&  GetErrorMessage() const;
    void            SetReportFormat(const QString& lReportFormat);

protected:

    bool    CreateScriptFile(const QString& scriptFile);
    bool    ExecuteScriptFile(const QString& scriptFile);

private:

    Q_DISABLE_COPY(PATReportFT)

    QString         mErrorMessage;
    QStringList     mOutputFiles;
    QString         mTraceabilityFile;
    QString         mReportFormat;
    QList<int>      mSites;
    PATRecipe       mRecipe;
};

}   // namespace Gex
}   // namespace GS


#endif // PAT_REPORT_FT_H
