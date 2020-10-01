#ifndef REPORT_LOG_UNIT_H
#define REPORT_LOG_UNIT_H

#include "report_log_list.h"
#include "gex_report_unit.h"


namespace GS
{
namespace Gex
{

/*! \class ReportLogUnit
 * \brief Report Unit to show the warning/error logs.
 *
 */
class ReportLogUnit : public ReportUnit
{
public:
    /// \brief Constructor
    ReportLogUnit(CGexReport*, const QString&);
    /// \brief Destructor
    virtual ~ReportLogUnit();

    QString PrepareSection(bool bValidSection);
    QString CreatePages();
    QString CloseSection();
};

}
}
#endif // REPORT_LOG_UNIT_H
