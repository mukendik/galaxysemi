#ifndef FT_PAT_REPORT_UNIT_H
#define FT_PAT_REPORT_UNIT_H

#include "gex_report_unit.h"

namespace GS
{
namespace Gex
{

class FTPATReportUnitPrivate;

class FTPATReportUnit : public ReportUnit
{

public:

    FTPATReportUnit(CGexReport* gr, const QString &cslkey);
    virtual ~FTPATReportUnit();

    QString     PrepareSection(bool bValidSection);
    QString     CreatePages();
    QString     CloseSection();

private:

    Q_DISABLE_COPY(FTPATReportUnit)

    FTPATReportUnitPrivate *  mPrivate;

    friend class FTPATReportUnitPrivate;

};

}
}
#endif // FT_PAT_REPORT_UNIT_H
