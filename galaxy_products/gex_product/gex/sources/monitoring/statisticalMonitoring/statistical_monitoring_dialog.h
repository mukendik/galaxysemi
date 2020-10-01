#ifndef STATISTICAL_MONITORING_DIALOG_H
#define STATISTICAL_MONITORING_DIALOG_H

#include <QDialog>
#include "ui_statistical_monitoring_dialog.h"

class CGexMoTaskStatisticalMonitoring;

namespace GS
{
namespace Gex
{

class SPMProdWidget;
class StatisticalMonitoringDraftWidget;
class StatisticalMonitoringSettingsWidget;
class StatisticalMonitoringProdWidget;

class StatisticalMonitoringDialog : public QDialog
{
    Q_OBJECT
public:
    //! \brief Holds tabs index
    enum TabIndex
    {
        TAB_SETTINGS = 0,
        TAB_DRAFT = 1,
        TAB_VERSIONS = 2
    };
    //! \brief Constructor
    StatisticalMonitoringDialog(CGexMoTaskStatisticalMonitoring* task, QWidget *parent = 0, bool modal = false);
    virtual ~StatisticalMonitoringDialog();


    //! \brief Initializes Statictical Monitoring task GUI
    bool InitializeUI(const int DatabaseType, bool aIsNewTask);

protected:
    //! \brief Initializes specific task GUI
    virtual bool InitializeSpecificUI() = 0;

    virtual StatisticalMonitoringSettingsWidget*    createSettingsWidget() = 0;
    virtual StatisticalMonitoringDraftWidget*       createDraftLimitsWidget() = 0;
    virtual StatisticalMonitoringProdWidget*        createProdWidget() = 0;

    //! \brief return the duplicate title if any. If no duplicate, an empty string is returned
    virtual bool                                    AnalyseDuplicatedTasks() = 0;

    //! \brief to be reimplement. Will be called during the task saving
    //! Enable to perfomed some specific.
    virtual bool                                    SaveSpecifTask() = 0;


    Ui::StatisticalMonitoringDialog        *mUI;                    ///< Holds ptr to UI
    StatisticalMonitoringSettingsWidget    *mSettingsWidget;        ///< Holds ptr to settings widget
    StatisticalMonitoringDraftWidget       *mDraftWidget;           ///< Holds ptr to compute widget
    StatisticalMonitoringProdWidget        *mProdWidget;            ///< Holds ptr to production limits widget
    CGexMoTaskStatisticalMonitoring        *mTask;                  ///< Holds ptr to current draft task
    bool                                   mDraftLimitsWidgetInitialized;  ///< true if comput widget initialized
    bool                                   mProdWidgetInitialized;     ///< true if prod widget initialized
    bool                                   mSettingsChanged;           ///< true if settings chanaged without save


public slots:
    //! \brief This slot should be called whenever limit from previous version have been copied
    void OnNewLimitCopied();
    //! \brief This slot should be called whenever the SPM settings have changed
    void OnSettingsChanged();
    //! \brief This slot should be called whenever the SPM settings have loaded
    void OnSettingsLoaded();
    //! \brief Called when tab changed
    void OnCurrentTabChanged(int newIndex);
    //! \brief Read/Save all fields entered.
    void OnCancel(void);
    //! \brief Read/Save all fields entered.
    void OnOk();
    //! brief Ignore the ESCAPE key
    void reject() { }
    //! \brief Ignore the ENTER key
    void keyPressEvent(QKeyEvent* event);

    //! \brief disable if needed tabs
    virtual void DisableTab() = 0;

private slots:
    bool OnSave();
    void OnCompute();
    void OnUpdate();
    void OnSimulate(const QDateTime& dateFrom, const QDateTime& dateTo);
    void OnCreateManualLimits();
    void OnValidationSucceeded();

private:
    //! \brief Refresh according members
    void RefreshUI();
    bool SaveTask();

};


}
}

#endif // STATISTICAL_MONITORING_DIALOG_H
