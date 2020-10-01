#ifndef STATISTICAL_MONITORING_PROD_WIDGET_H
#define STATISTICAL_MONITORING_PROD_WIDGET_H

#include <QTableWidget>
#include <QTextEdit>

#include "common_widgets/split_widget.h"

class CGexMoTaskStatisticalMonitoring;
class CollapsibleButton;
class InfoWidget;

namespace GS
{
namespace Gex
{

class SMLegendWidget;
class StatisticalMonitoringLimitsWidget;
class StatisticalMonitoringVersionWidget;

class StatisticalMonitoringProdWidget : public SplittedWindowWidget
{
    Q_OBJECT

public:
    /// \brief Constructor
    explicit StatisticalMonitoringProdWidget(CGexMoTaskStatisticalMonitoring *task,
                                             QWidget *parent = 0);
    /// \brief Destructor
    virtual ~StatisticalMonitoringProdWidget();
    /// \brief Load limits from DB
    bool LoadLimitsFromDatabase(int versionId = -1 );
    /// \brief Load the versions from database and fill the table view.
    bool LoadVersionsFromDatabase(bool forceReload = false);

signals:
    /// \brief emited whenener a limits version have been copied in the draft task
    void NewLimitCopied();

public slots:
    /// \brief erase the view if the viewed version is the one deleted
    void UpdateUIAfterDelete(int deletedVersion);
    /// \brief load a version id
    void LoadVersionId(int versionId);
    /// \brief copy the limit from a task id version to the draft version
    void CopyTaskVersion();
    /// \brief Init overall UI
    void InitializeUI();

    bool IsDataModelEmpty();


protected:
    virtual StatisticalMonitoringLimitsWidget* CreateLimitsWidget() = 0;
    virtual StatisticalMonitoringVersionWidget* CreateVersionWidget() = 0;
    /// \brief create widget for the left banner
    void CustomizeLeftBannerUI(QLayout *);
    /// \brief Init specialized UI
    void CustomizeRightBannerUI(QLayout* layout);
    /// \brief Init the search bar associated with the data table
    void InitSearchBarLimitUI(CollapsibleButton *widget);

    StatisticalMonitoringLimitsWidget*      mLimitsWidget;          ///< Holds ptr to the limits widget
    StatisticalMonitoringVersionWidget*     mVersionWidget;         ///< Holds ptr to the version table widget
    CGexMoTaskStatisticalMonitoring*        mTask;                  ///< Holds ptr to current draft
    SMLegendWidget*                         mLegend;                ///< Holds ptr to the legend of data's color
    CollapsibleButton*                           mLimitUnFoldButton;     ///< Holds ptr to unfold button displaying the limit table
    CollapsibleButton*                           mVersionUnFoldButton;   ///< Holds ptr to unfold button displaying the version table
    QTableWidget*                           mTableInfoWidget;       ///< Holds ptr to the info widget
    InfoWidget*                             mInfoWidget;            ///< Holds ptr to the info widget
    int                                     mIdViewed;              ///< Holds the task id version currently viewed in the table
};

} // namespace Gex
} // namespace GS

#endif // STATISTICAL_MONITORING_PROD_WIDGET_H
