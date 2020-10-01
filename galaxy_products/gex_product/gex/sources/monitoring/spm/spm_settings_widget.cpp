#include "spm_settings_widget.h"
#include "browser_dialog.h"
#include "gex_database_entry.h"
#include "statistical_monitoring_taskdata.h"
#include "message.h"
#include "spm_engine.h"
#include "spm_database_structure_inspector.h"

#include <gqtl_log.h>

namespace GS
{
namespace Gex
{

SPMSettingsWidget::SPMSettingsWidget(QWidget *parent) :
    StatisticalMonitoringSettingsWidget(parent),
    m_databaseStructureInspector( NULL )
{
}

SPMSettingsWidget::~SPMSettingsWidget()
{
    if( m_databaseStructureInspector != NULL )
    {
        delete m_databaseStructureInspector;
    }
}

void SPMSettingsWidget::onCreateManualLimitsClicked()
{
    createManualLimitsClicked("Test Number", "Test Name");
}

bool SPMSettingsWidget::initializeSpecializedUI()
{
    mUI->labelItemType->setText("Test type:");
    mUI->labelItemType->hide();
    QMap<QString, QString> supportedItemTypes = Engine::GetInstance().GetSPMEngine().GetSupportedMonitoredItemTypes();
    QListIterator<QString> iter = supportedItemTypes.keys();
    while(iter.hasNext())
    {
        QString key = iter.next();
        mUI->comboItemType->addItem(supportedItemTypes.value(key), QVariant(key));
    }
    mUI->comboItemType->hide();

    mUI->labelItemNums->setText("Tests #:");

    mUI->comboConsoLevel->setEnabled(false);

    return true;
}


CGexPB* SPMSettingsWidget::loadSpecializedPropertyBrowserFromXML()
{
    return loadPropertyBrowserFromXML("SPM", "SPMSettings", ":/gex/xml/spm_settings.xml");
}

// This function will replace placeholders in the SPM settings XML file with their values
void SPMSettingsWidget::rawReplace(const QString & key, QString & value)
{
    if(key == "key")
    {
        value.replace("$K_STATSTOMONITOR", C_STATSTOMONITOR);
        value.replace("$K_MINLOTS", C_MINLOTS);
        value.replace("$K_MINDATAPOINTS", C_MINDATAPOINTS);
        value.replace("$K_VERSION_SITEMERGEMODE", C_SITEMERGEMODE);
        value.replace("$K_DEFAULTALGO", C_DEFAULTALGO);
        value.replace("$K_N1", C_N1);
        value.replace("$K_N2", C_N2);
        value.replace("$K_DATACLEANING", C_DATACLEANING);
        value.replace("$K_VALIDITYPERIOD", C_VALIDITYPERIOD);
        value.replace("$K_ACTIVEONINSERT", C_ACTIVEONINSERT);
        value.replace("$K_ACTIVEONTRIGGER", C_ACTIVEONTRIGGER);
        value.replace("$K_DAYSBEFOREEXPIRATION", C_DAYSBEFOREEXPIRATION);
        value.replace("$K_SENDEMAILBEFOREEXPIRATION", C_SENDEMAILBEFOREEXPIRATION);
        value.replace("$K_AUTORECOMPUTE", C_AUTORECOMPUTE);
        value.replace("$K_AUTORECOMPUTEMETHOD", C_AUTORECOMPUTEMETHOD);
        value.replace("$K_AUTORECOMPUTEPERIOD", C_AUTORECOMPUTEPERIOD);
        value.replace("$K_EXCEPTIONTYPE", C_EXCEPTIONTYPE);
        value.replace("$K_EMAILFROM", C_EMAILFROM);
        value.replace("$K_EMAILTO", C_EMAILTO);
        value.replace("$K_EMAILFORMAT", C_EMAILFORMAT);
    }
    if(key == "codingvalues")
    {
        if(value.contains("$CV_STATSTOMONITOR"))
        {
            value.replace("$CV_STATSTOMONITOR", QStringList(
                    Engine::GetInstance().GetSPMEngine().GetSupportedStats().keys()).join("|"));
        }
        if(value.contains("$CV_DEFAULTALGO"))
        {
            value.replace("$CV_DEFAULTALGO", QStringList(
                    Engine::GetInstance().GetSPMEngine().GetSupportedOutlierAlgorithms().keys()).join("|"));
        }
    }
    if(key == "displayvalues")
    {
        if(value.contains("$DV_STATSTOMONITOR"))
        {
            value.replace("$DV_STATSTOMONITOR", QStringList(
                    Engine::GetInstance().GetSPMEngine().GetSupportedStats().values()).join("|"));
        }
        if(value.contains("$DV_DEFAULTALGO"))
        {
            value.replace("$DV_DEFAULTALGO", QStringList(
                    Engine::GetInstance().GetSPMEngine().GetSupportedOutlierAlgorithms().values()).join("|"));
        }
    }
}

void SPMSettingsWidget::onPickProducts(void)
{
    // get database pointer
    GexDatabaseEntry *lADRDatabaseEntry = getADREntry();
    if (!lADRDatabaseEntry)
    {
        return;
    }
    if(!lADRDatabaseEntry->m_pExternalDatabase)
    {
        return;
    }

    QString lTestingStage = mUI->comboTestingStage->currentText();
    QString lProductFilter = mUI->editProducts->text();
    // Get list of strings to fill in list box
    QStringList lMatchingValues;
    if(!lADRDatabaseEntry->m_pExternalDatabase->SPM_GetProductList(lTestingStage,lProductFilter,lMatchingValues))
    {
        QString lError = GGET_LASTERRORMSG(GexRemoteDatabase, lADRDatabaseEntry->m_pExternalDatabase);
        GSLOG(SYSLOG_SEV_ERROR, lError.toLatin1().constData());
        Message::critical("GEX error", "Impossible to query available products!\n\n"+lError);
        return;
    }
    if(lMatchingValues.count() == 0)
    {
        Message::warning("GEX warning", "No product available!");
        return;
    }

    pickProducts(lMatchingValues);
 }

void SPMSettingsWidget::onPickFlow()
{
    GexDatabaseEntry *lADRDatabaseEntry = getADREntry();
    if(!lADRDatabaseEntry)
    {
        return;
    }
    if(!lADRDatabaseEntry->m_pExternalDatabase)
    {
        return;
    }

    QString lTestingStage = mUI->comboTestingStage->currentText();
    QString lProductFilter = mUI->editProducts->text();
    // Get list of strings to fill in list box
    QStringList lMatchingValues;
    if(!lADRDatabaseEntry->m_pExternalDatabase->SPM_GetFlowList(lTestingStage,lProductFilter,lMatchingValues))
    {
        QString lError = GGET_LASTERRORMSG(GexRemoteDatabase, lADRDatabaseEntry->m_pExternalDatabase);
        GSLOG(SYSLOG_SEV_ERROR, lError.toLatin1().constData());
        Message::critical("GEX error", "Impossible to query available products!\n\n"+lError);
        return;
    }
    if(lMatchingValues.count() == 0)
    {
        Message::warning("GEX warning", "No product flow available!");
        return;
    }

    pickFlow(lMatchingValues);
}

void SPMSettingsWidget::resetDatabaseStructureInspector(GexDatabaseEntry *aTDREntry)
{
    if( aTDREntry != NULL )
    {
        delete m_databaseStructureInspector;
        m_databaseStructureInspector = NULL;
        m_databaseStructureInspector = new Qx::Gex::SpmDatabaseStructureInspector( aTDREntry );
    }
}

void SPMSettingsWidget::setupDatabaseStructureInspector(GexDatabaseEntry *aTDREntry)
{
    if( m_databaseStructureInspector == NULL )
    {
        m_databaseStructureInspector = new Qx::Gex::SpmDatabaseStructureInspector( aTDREntry );
    }
    else
    {
        resetDatabaseStructureInspector(aTDREntry);
    }
}

void SPMSettingsWidget::checkDatabasesConsistency( GexDatabaseEntry *aDBEntry )
{
    setupDatabaseStructureInspector( aDBEntry );
    m_databaseStructureInspector->CheckTDRAndADRStructure();
}

void SPMSettingsWidget::onPickInsertion(void)
{
    GexDatabaseEntry *lADRDatabaseEntry = getADREntry();
    if(!lADRDatabaseEntry)
    {
        return;
    }
    if(!lADRDatabaseEntry->m_pExternalDatabase)
    {
        return;
    }

    QString lTestingStage = mUI->comboTestingStage->currentText();
    QString lProductFilter = mUI->editProducts->text();
    QString lTestFlow = mUI->editFlow->text();
    QString lConsoType = mUI->comboConsoType->currentData().toString();

    QStringList lMatchingValues;
    if(!lADRDatabaseEntry->m_pExternalDatabase->SPM_GetInsertionList(lTestingStage,lProductFilter,lTestFlow,lMatchingValues))
    {
        QString lError = GGET_LASTERRORMSG(GexRemoteDatabase, lADRDatabaseEntry->m_pExternalDatabase);
        GSLOG(SYSLOG_SEV_ERROR, lError.toLatin1().constData());
        Message::critical("GEX error", "Impossible to query available products!\n\n"+lError);
        return;
    }
    if(lMatchingValues.count() == 0)
    {
        Message::warning("GEX warning", "No product insertion available!");
        return;
    }

    pickInsertion(lMatchingValues);
}

void SPMSettingsWidget::onPickItems(void)
{
    GexDatabaseEntry *lADRDatabaseEntry = getADREntry();
    if (!lADRDatabaseEntry)
    {
        return;
    }
    if(!lADRDatabaseEntry->m_pExternalDatabase)
    {
        return;
    }

    QString lTestingStage = mUI->comboTestingStage->currentText();
    QString lProductFilter = mUI->editProducts->text();
    QString lTestFlow = mUI->editFlow->text();
    QString lTestInsertion = mUI->editInsertion->text();
    QString lTestType = mUI->comboItemType->currentData().toString();
    // Get list of strings to fill in list box
    QStringList lMatchingValues;
    if(!lADRDatabaseEntry->m_pExternalDatabase->SPM_GetItemsList(lTestingStage,lProductFilter,lTestFlow,lTestInsertion,lTestType,lMatchingValues))
    {
        QString lError = GGET_LASTERRORMSG(GexRemoteDatabase, lADRDatabaseEntry->m_pExternalDatabase);
        GSLOG(SYSLOG_SEV_ERROR, lError.toLatin1().constData());
        Message::critical("GEX error", "Impossible to query available products!\n\n"+lError);
        return;
    }
    if(lMatchingValues.count() == 0)
    {
        Message::warning("GEX warning", "No product items available!");
        return;
    }

    pickTests(lMatchingValues);
}

GexDatabaseEntry * SPMSettingsWidget::getADREntry()
{
    return m_databaseStructureInspector->GetADREntry();
}

} // namespace Gex
} // namespace GS
