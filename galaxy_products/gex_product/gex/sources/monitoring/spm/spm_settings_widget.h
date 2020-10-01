#ifndef SPM_SETTINGS_WIDGET_H
#define SPM_SETTINGS_WIDGET_H

#include "statistical_monitoring_settings_widget.h"

namespace Qx
{
namespace Gex
{
class SpmDatabaseStructureInspector;
}
}

namespace GS
{
namespace Gex
{

/**
 * \brief The SPMSettingsWidget class provides a widget for letting the user customize the settings of a SPM task
 */
class SPMSettingsWidget : public StatisticalMonitoringSettingsWidget
{
    Q_OBJECT

public:
    /**
     * \brief Constructs an empty SPM settings widget. The widget will not contain any settings/control.
     * Use the initializeUI() method to load the widget with controls to manipulate SPM settings.
     *
     * \param parent parent widget. Is passed to the QWidget constructor.
     */
    explicit SPMSettingsWidget(QWidget *parent = 0);
    /**
     * \brief Destroys the SPM settings widget.
     */
    ~SPMSettingsWidget();

protected:
    /**
     * \brief This method initializes the widget with controls to manipulate SPM settings and loads SPM settings data
     *
     * \return true if the widget initialization succeeds, false else
     */
    bool        initializeSpecializedUI();

    // Loads Property Browser from XML definition file
    CGexPB* loadSpecializedPropertyBrowserFromXML();
    void    rawReplace(const QString &key, QString &value);

private:
    // TODO - SebL: today, this stuff is very specific to SPM feature but a refacto will be needed as soon as we'll
    //              implement DB structure check for SYA.
    Qx::Gex::SpmDatabaseStructureInspector *m_databaseStructureInspector;

private :
    void resetDatabaseStructureInspector( GexDatabaseEntry * aTDREntry = NULL );
    void setupDatabaseStructureInspector( GexDatabaseEntry * aTDREntry = NULL );
    GexDatabaseEntry * getADREntry();
    void checkDatabasesConsistency(GexDatabaseEntry * aDBEntry);

private slots:
    void    onCreateManualLimitsClicked();
    void    onPickProducts();
    void    onPickItems();
    void    onPickFlow();
    void    onPickInsertion();

};

} // namespace Gex
} // namespace GS

#endif // SPM_SETTINGS_WIDGET_H
