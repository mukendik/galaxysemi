#ifndef SYA_SETTINGS_WIDGET_H
#define SYA_SETTINGS_WIDGET_H

#include "statistical_monitoring_settings_widget.h"
#include <QMap>

class GexDatabaseEntry;

namespace GS
{
namespace Gex
{
/**
 * \brief The SYASettingsWidget class provides a widget for letting the user customize the settings of a SYA task
 */
class SYASettingsWidget : public StatisticalMonitoringSettingsWidget
{
    Q_OBJECT

public:
    /**
     * \brief Constructs an empty SYA settings widget. The widget will not contain any settings/control.
     * Use the initializeUI() method to load the widget with controls to manipulate SYA settings.
     *
     * \param parent parent widget. Is passed to the QWidget constructor.
     */
    explicit SYASettingsWidget(QWidget *parent = 0);
    /**
     * \brief Destroys the SYA settings widget.
     */
    ~SYASettingsWidget();
    /**
     * \brief This method initializes the widget with controls to manipulate SYA settings and loads SYA settings data
     *
     * \return true if the widget initialization succeeds, false else
     */
    bool        initializeSpecializedUI();

    /**
     * @brief load the prpoerty that have to be display in the browser
     * @return
     */
    CGexPB*     loadSpecializedPropertyBrowserFromXML();

    /**
     * \brief This method dumps GUI fields into fields from SPM data structure
     *
     * \return true if the load succeeds, false else
     */
     void        rawReplace(const QString & key, QString & value);

private:
    // GCORE-14130: verifying DB consistency (both ADR and TDR) nothing specific todo today, gonna change in near future
    void checkDatabasesConsistency( GexDatabaseEntry * ) {}

private slots:
    void onPickProducts();
    void onPickItems();
    void onPickFlow() {}
    void onPickInsertion() {}
    void onCreateManualLimitsClicked();

};




}
}

#endif
