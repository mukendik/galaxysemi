#ifndef WS_PAT_REPORT_UNIT_H
#define WS_PAT_REPORT_UNIT_H

#include "gex_report_unit.h"
#include <QList>

class CPatDefinition;

namespace GS
{
namespace Gex
{

class WSPATReportUnit : public ReportUnit
{

public:

    WSPATReportUnit(CGexReport* gr, const QString &cslkey);
    virtual ~WSPATReportUnit();

    QString     PrepareSection(bool bValidSection);
    QString     CreatePages();
    QString     CloseSection();

protected:

    bool        FindPATFailuresPareto(QList<CPatDefinition*>& lRuleFailures,
                                      QList<CPatDefinition*>& lFamilyFailures,
                                      long &lTotalPartsProcessed);
private:

    Q_DISABLE_COPY(WSPATReportUnit)

};

}
}

#endif // WS_PAT_REPORT_UNIT_H
