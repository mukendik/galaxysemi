#ifdef GCORE15334
#ifndef PAT_REPORT_WS_H
#define PAT_REPORT_WS_H

#include "gex_pat_processing.h"
#include "pat_engine.h"

namespace GS
{
namespace Gex
{

class PATReportWS
{
public:

    PATReportWS(bool lShow = false);
    ~PATReportWS();

    bool            Generate(const QString& outputFile, const PATProcessing &settings);

    const QString&  GetErrorMessage() const;
    void            SetSites(const QList<int>& sites);
    void            SetShow(bool show);

protected:

    bool    CreateScriptFile(const QString& scriptFile);
    bool    CreateMarkersFile(const QString& markersFile, QString& sampleBins);
    bool    ExecuteScriptFile(const QString& scriptFile);
    bool    WriteOutlierFailure(FILE * handle, QString& sampleBins);
    bool    WriteMapAxisDirection(FILE * handle);

    QString GetOutliersTestList() const;

private:

    Q_DISABLE_COPY(PATReportWS)

    PATProcessing   mSettings;
    QString         mErrorMessage;
    QString         mOutputFile;
    QList<int>      mSites;
    bool            mShow;
    CPatInfo* lPatInfo;
};

}   // namespace Gex
}   // namespace GS

#endif // PAT_REPORT_WS_H
#endif
