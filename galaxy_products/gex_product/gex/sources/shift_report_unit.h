#ifndef SHIFT_REPORT_UNIT
#define SHIFT_REPORT_UNIT

#include <QMap>
#include "gex_report_unit.h"

class CTest;
class CPartInfo;

namespace GS
{
namespace Gex
{
class ShiftReportUnit : public ReportUnit
  {
      Q_DISABLE_COPY(ShiftReportUnit)

      QString mCalcMethod;
      QString mSortColumn; ///brief When writting the shift analysis table, this option set the column used to sort the output table.
      unsigned int mMaxNumberOfLinesPerSlideInPres;

      /**
       * \brief This enumerate contains the polarity of limit delta
       */
      enum Polarity
      {
          positive,
          negative
      };

      /**
       * \brief This enumerate contains the type of the files in group
       * samples: means we have only samples in the list of files
       * controls: means we have only controls in the list of files
       * both: means we have samples and controls in the list of files
       */
      enum TypeFiles
      {
          samples,
          controls,
          both
      };

      class ShiftAlert
      {
         public:
            CPartInfo* mPartInfo;
            CTest*  mTest;
            QMap<CGexGroupOfFiles*, double> mTestResultsPerGroup;
            ShiftAlert(CTest* t, CPartInfo* pi) { mTest=t; mPartInfo=pi; mMaxGroup=0; mMinGroup=0; }
            QMap<QString, QVariant> mParams;    // partid, maxdelta, percent,...
            CGexGroupOfFiles* mMinGroup;
            CGexGroupOfFiles* mMaxGroup;
            Polarity mPolarity;
            static bool percentLessThan(const ShiftAlert* sa1, const ShiftAlert* sa2)
                    { return sa1->mParams.value("percent").toDouble() < sa2->mParams.value("percent").toDouble(); }
            static bool percentGreatThan(const ShiftAlert* sa1, const ShiftAlert* sa2)
                    { return !percentLessThan(sa1, sa2); }
      };

      QMultiMap< QString, ShiftAlert* > mPartsAlertsMap;    // map of alerts ordered by partID
      QMultiMap< CTest*, ShiftAlert* > mTestsAlertsMap;     // map of alerts ordered by CTest
      QMultiMap< double, ShiftAlert* > mPercentLSAlertsMap;    // map of alerts ordered by % of LS

      // Mine for shift alert (when %LS is upper than threshold)
      // Will probaly create some ShiftAlerts : they will be deleted at destructor
      // the ShiftAlerts pointers will be inserted in the maps (mPartsAlertsMap, ...)
      QString SearchForShiftAlerts();

      QString WriteDeviceReport(QString lTitlesAttributes, QString lTableAtts, QString lFirstTableLineAtts);
      QString WriteTestReport(const QString &lTitlesAttributes, const QString &lTableAtts, const QString &lFirstTableLineAtts);
      QString WriteParetoReport(QString lTitlesAttributes, QString lTableAtts, QString lFirstTableLineAtts);

      QString WriteGroupsSummaryTable(QString lTitlesAttributes, QString lTableAtts, QString lFirstTableLineAtts);
      QString WriteOptionsTable(QString lTableAtts, QString lFirstTableLineAtts);


      // compute the absolute Max delta using the calculation method in ReportOptions
      // mingroup will be the group with the lowest value
      // could return GEX_C_DOUBLE_NAN on failure
      double ComputeMaxDelta(QMap<int, double> testResultPerGroup,
                             CGexGroupOfFiles* &mingroup,
                             CGexGroupOfFiles* &maxgroup,
                             int ref_group);


      bool CalculateMaxControlsDelta(ShiftAlert* sampleShiftAlert, ShiftAlert* &controlShiftAlert, unsigned short &nbreShiftAlert);
public:
        //
        ShiftReportUnit(CGexReport*, const QString& cslkey
                        //CReportOptions &ro, const QList<CGexGroupOfFiles*> lgof
                        );
        virtual ~ShiftReportUnit();
        // inherated from ReportUnit
        QString CreatePages();
        QString CloseSection();
        QString PrepareSection(bool bValidSection);

private:
        TypeFiles mTypeFilesInGroup;
        QList<CGexGroupOfFiles*> mSamplesGroups;

};
} // namespace Gex
} // namespace GS

#endif
