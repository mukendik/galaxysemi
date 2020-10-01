#ifndef SATISTICAL_MONITORING_SETTINGS_WIDGET_H
#define SATISTICAL_MONITORING_SETTINGS_WIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QMap>
#include "common_widgets/collapsible_button.h"
#include "task_properties.h"
#include "ui_statistical_monitoring_settings_widget.h"

// Forward declarations
class CGexPB;               // libgexpb.h
class GexMoStatisticalMonitoringTaskData;     // spm_taskdata.h
class CGexMoTaskItem;       // mo_task.h
class smFilters;
class GexDatabaseEntry;

namespace GS
{
namespace Gex
{

/**
 * \brief The SettingsWidget class provides a widget for letting the user customize the settings of a  task
 */
class StatisticalMonitoringSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * \brief Constructs an empty  settings widget. The widget will not contain any settings/control.
     * Use the initializeUI() method to load the widget with controls to manipulate  settings.
     *
     * \param parent parent widget. Is passed to the QWidget constructor.
     */
    explicit StatisticalMonitoringSettingsWidget(QWidget *parent = 0);
    /**
     * \brief Destroys the  settings widget.
     */
    virtual ~StatisticalMonitoringSettingsWidget();

    /**
     * \brief This method initializes the widget with controls to manipulate Statistical Monitoring settings and
     * loads settings data
     *
     * \return true if the widget initialization succeeds, false else
     */
    bool        initializeUI(const int databaseType, bool aIsNewTask);

    /**
     * \brief This method loads fields from  data structure into the GUI fields
     *
     * \return true if the load succeeds, false else
     */
    virtual bool  loadFields(GexMoStatisticalMonitoringTaskData* taskData);
    /**
     * \brief This method dumps GUI fields into fields from SPM data structure
     *
     * \return true if the load succeeds, false else
     */
    virtual bool  DumpFields(GexMoStatisticalMonitoringTaskData* taskData);
    /**
     * \brief Returns a list of new tests for which manual limits should be created
     */
    QList<QPair<int, QString> > &GetNewTests();

    Ui::StatisticalMonitoringSettingsWidget*    getUI() { return mUI;}

protected:
    // UI
    Ui::StatisticalMonitoringSettingsWidget*    mUI;
    QLayout*                                    mFrameLayout;

    // filters management
    CollapsibleButton*                          mButtonShowHide;
    smFilters*                                  mFilters;

    // Property Browser management
    CGexPB*                                     mSettingsPB;
    CollapsibleButton*                          mButtonProperties;

    // Task data cache for discard
    GexMoStatisticalMonitoringTaskData*         mCacheData;

    // GUI state management
    bool                                        mSettingsChanged;

    QList<QPair<int, QString> > mNewTests;
    QMap<QString, int>          mSettingKeyToPropID;

    // Loads Property Browser from XML definition file
    virtual CGexPB* loadSpecializedPropertyBrowserFromXML() = 0;
    virtual bool    initializeSpecializedUI() = 0;
    virtual void    rawReplace(const QString & key, QString & value) = 0;
    virtual QString getCurrentDBName();
    void ConnectEditSignals();
    void DisconnectEditSignals();

    /// \brief load the propertie of the xml file
    /// type : the type of monitoring (SPM, SYA)
    /// firstElement: the name of the first element that must be seek on the document
    /// fileName : the name of the file that must be loaded
    /// return a CGexPB
    CGexPB* loadPropertyBrowserFromXML(QString type, QString firstElement, QString fileName);

    void    pickProducts    (QStringList &productList);
    void    pickTests       (QStringList &testList);
    void    pickFlow        (QStringList &flowList);
    void    pickInsertion   (QStringList &insertionList);
    void    pickBins        (QStringList& binList);
    void    initFiltersUI();
    void    createManualLimitsClicked(const QString& labelNumber, const QString& labelName );

public slots:
      bool    onCheckMandatoryFields();

protected:
    virtual void checkDatabasesConsistency( GexDatabaseEntry *aDBEntry ) = 0;

protected slots:

    void    onDatabaseChanged();
    void    onSettingsChanged();
    bool    onSaveSettings();
    void    onSettingsLoaded();
    void    onTestingStageChanged();
    void    onSaveClicked();
    void    onComputeClicked();
    void    onDiscardClicked();
    void    onPropertyChanged(int settingID, const QVariant &newValue);
    virtual void  onCreateManualLimitsClicked() = 0;
    void    onProductsChanged();
    void    onItemTypeChanged();
    void    onFlowChanged();
    void    onConsoTypeLevelChanged();
    virtual void onPickProducts() = 0;
    virtual void onPickItems() = 0;
    virtual void onPickFlow() = 0;
    virtual void onPickInsertion() = 0;
    void    onResetFilters();

signals:
    void    sSettingsChanged();
    void    sSettingsLoaded();
    void    sSaveClicked();
    void    sComputeClicked();
    void    sCreateLimitsClicked();
};

} // namespace Gex
} // namespace GS

#endif // SATISTICAL_MONITORING_SETTINGS_WIDGET_H
