#ifndef CHARAC2_REPORT_UNIT_H
#define CHARAC2_REPORT_UNIT_H

#include "gex_report_unit.h"

namespace GS
{
namespace Gex
{

class Charac2ReportUnit : public ReportUnit
{
public:

    Charac2ReportUnit(CGexReport* pGexReport, const QString& cslkey);
    ~Charac2ReportUnit();

    // inherated from ReportUnit
    QString PrepareSection(bool lValidSection);
    QString CreatePages();
    QString CloseSection();

protected:

    int     GetChartWidth() const;

private:

    // generate chart image and store image_path in image_path
    QString ComputeChart(CTest* pTest, QString& lImagePath);

    // List of tests for Adv report
    // Will be filled in PrepareSection_...
    QList<CTest*> mTestsListToReport;
};

} // end namespace Gex
}   // end namespace GS

#endif // CHARAC2_REPORT_UNIT_H
