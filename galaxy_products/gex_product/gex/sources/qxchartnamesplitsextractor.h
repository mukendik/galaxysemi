#ifndef QXCHARTNAMESPLITSEDEXTRACTOR_H
#define QXCHARTNAMESPLITSEDEXTRACTOR_H

#include <QList>
#include <QString>


///
/// \brief The QxChartNameSplitsExtractor class enable to extract from a name like
/// "New Query[Lot Id=..][...] - Test Name, coming from a db extraction,
/// the information that are share among all charts (like the type of setting use to split) and the info that are
/// specific (like the value of the split).
/// So that the display can be reduce for a chart to the specific value (at charge of the UI class to do so)
/// The shared value will put on common legend (at charge of the UI class to do so)
///
class QxChartNameSplitsExtractor
{
public:
    QxChartNameSplitsExtractor() {}


    ///
    /// \brief SeparateSplitsNameValue will extracte and separate in 2 lists, the splits namee/value from
    /// the chart name "New Query[Lot Id=value1][splits2=value2]"
    /// \param aChartName: the raw name of the chart that will be processed
    /// \param aOutsplitsName: The list of splits name in output  "[LotId][splits2]"
    /// \param aOutsplitsValue: the list of the splits value in output "[value1][value2]"
    ///
    bool SeparateSplitsNameValue(const QString& aChartName,
                                  QList<QString>& aOutSplitsName,
                                  QList<QString>& aOutSplitsValue);



    ///
    /// \brief ExtractRequestTitle will extract the title of the resquest create for the db analysis
    /// \param aChartName: the raw name of the chart that will be processed
    /// \return in the exemple "New Query[Lot Id=..][...] - Test1000" will return "New Query"
    ///
    QString ExtractRequestTitle(const QString& aChartName);

    ///
    /// \brief ExtractTestName will extract the test name
    /// \param aChartName: the raw name of the chart that will be processed
    /// \return in the exemple "New Query[Lot Id=..][...] - Test1000" will return "Test1000"
    ///
    QString ExtractTestName(const QString& aChartName);

private:
    ///
    /// \brief ExtractSplits, extract the list of the splits dataset
    /// \param aName: the raw name of the chart that will be processed
    /// \return a list of split dataset [Name=Value][...]...
    ///
    QList<QString> ExtractSplitsDataset(const QString& aName);

};


#endif // QXCHARTNAMESPLITSEXTRACTOR_H
