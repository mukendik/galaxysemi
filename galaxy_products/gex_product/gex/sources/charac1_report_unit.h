#ifndef CHARAC1_REPORT_UNIT
#define CHARAC1_REPORT_UNIT

#include <QMap>
#include "gex_report_unit.h"

class CTest;
struct HtmlTableItem;
typedef QList<HtmlTableItem> HtmlTableRow;

namespace GS
{
namespace Gex
{
  class Charac1ReportUnit : public ReportUnit
  {
      QString WriteGroupsSummaryTable(QString lTitlesAttributes, QString lTableAtts, QString lFirstTableLineAtts);
      //QString WriteOptionsTable(QString lTableAtts, QString lFirstTableLineAtts);
      // generate chart image and store image_path in image_path
      QString ComputeBoxWhiskerChart(CTest* test, QString& image_path, int& bottomMargin, int& leftMargin);
      QString ComputeHistoChart(CTest* test, QString& image_path, int bottomMargin);

    public:
        Charac1ReportUnit(CGexReport*, const QString& cslkey);
        ~Charac1ReportUnit();
        // inherated from ReportUnit
        QString PrepareSection(bool bValidSection);
        QString CreatePages();
        QString CloseSection();

  protected:

        int     GetChartWidth() const;


  private:

        // List of tests for Adv report
        // Will be filled in PrepareSection_...
        QList<CTest*> mTestsListToReport;
        void WriteChart(CTest *aTest,
                       const int aNbCol,
                       int aLeftMargin);
        void BuildZeroTestCondTableRow(HtmlTableRow &aTableRow, const QList<CGexGroupOfFiles *> lColsToReport);
        void BuildTestCondTableRow(HtmlTableRow &aTableRow, int aTestCondId, const QList<CGexGroupOfFiles *> lColsToReport);
        void BuildCharacFieldsTableRow(HtmlTableRow &aTableRow, const QString& aField, CTest *aTest, const QList<CGexGroupOfFiles *> lColsToReport);

        void WriteTableSplit(QList<CGexGroupOfFiles *> lColsToReport,
                             const QString& aOutputFormat,
                             const QString& aPrinterType,
                             unsigned aHTML4FontSize,
                             float aViewPort,
                             const QStringList& aCharacFields,
                             const int aTestCondCount,
                             CTest * aTest);
        int GetMaxColumns();


  };
} // namespace Gex
} // namespace GS

#endif
