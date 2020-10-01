#include "engine.h"
#include "gex_shared.h"
#include "sya_engine.h"
#include <gqtl_log.h>
#include <libgexpb.h>
#include "gex_constants.h"
#include "message.h"
#include "db_external_database.h"
#include "db_engine.h"
#include "gexdb_plugin_base.h"
#include "gex_database_entry.h"
#include "statistical_monitoring_taskdata.h"
#include "sya_settings_widget.h"
#include <qmessagebox.h>
#include "sm_filters.h"

namespace GS
{
namespace Gex
{

SYASettingsWidget::SYASettingsWidget(QWidget *parent) :
    StatisticalMonitoringSettingsWidget(parent)
{
}

SYASettingsWidget::~SYASettingsWidget()
{
}

void SYASettingsWidget::onCreateManualLimitsClicked()
{
    createManualLimitsClicked("Bin Number", "Bin Name");
}

bool SYASettingsWidget::initializeSpecializedUI( )
{
    mUI->labelItemType->setText("Bin type:");
    QMap<QString, QString> supportedItemTypes = Engine::GetInstance().GetSYAEngine().GetSupportedMonitoredItemTypes();
    QListIterator<QString> iter = supportedItemTypes.keys();
    while(iter.hasNext())
    {
        QString key = iter.next();
        mUI->comboItemType->addItem(supportedItemTypes.value(key), QVariant(key));
    }

    mUI->labelItemNums->setText("Bins #:");
    mUI->labelTDR->setText("Database:");
    mUI->labelTestFlow->hide();
    mUI->editFlow->hide();
    mUI->buttonPickFlow->hide();
    mUI->labelConsoType->hide();
    mUI->comboConsoType->hide();
    mUI->labelConsoLevel->hide();
    mUI->comboConsoLevel->hide();
    mUI->labelInsertion->hide();
    mUI->editInsertion->hide();
    mUI->buttonPickInsertion->hide();

    mUI->toolButtonSave->setToolButtonStyle(Qt::ToolButtonTextOnly);
    return true;
}


CGexPB* SYASettingsWidget::loadSpecializedPropertyBrowserFromXML()
{
    return loadPropertyBrowserFromXML("SYA", "SYASettings", ":/gex/xml/sya_settings.xml");
}

void SYASettingsWidget::rawReplace(const QString & key, QString & value)
{
    if(key == "key")
    {
        //value.replace("$K_STATSTOMONITOR", C_STATSTOMONITOR);
        value.replace("$K_USEGROSSDIE", C_GROSSDIE);
        value.replace("$K_MINPERCENTBIN_THRESHOLD", C_THRESHOLD);
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
        if(value.contains("$CV_DEFAULTALGO"))
        {
            value.replace("$CV_DEFAULTALGO", QStringList(Engine::GetInstance().GetSYAEngine().GetSupportedOutlierAlgorithms().keys()).join("|"));
        }
    }
    if(key == "displayvalues")
    {
        if(value.contains("$DV_DEFAULTALGO"))
        {
            value.replace("$DV_DEFAULTALGO", QStringList(Engine::GetInstance().GetSYAEngine().GetSupportedOutlierAlgorithms().values()).join("|"));
        }
    }
}

void SYASettingsWidget::onPickProducts()
{
    // Get database pointer
    QString	lDBName = mUI->comboTDR->currentText();
    if(lDBName.isEmpty())
        return;
    GexDatabaseEntry* lDBEntry = Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(lDBName,true);
    if(!lDBEntry || !lDBEntry->m_pExternalDatabase)
        return;

    // Get product List
    QStringList	lProductList;

    // Fill external database query object
    GexDbPlugin_Filter lPluginFilter(this);
    lPluginFilter.strlQueryFilters.clear();
    lPluginFilter.strDataTypeQuery = mUI->comboTestingStage->currentText();
    lPluginFilter.iTimePeriod = GEX_QUERY_TIMEPERIOD_ALLDATES;

    QString lProductFilter = mUI->editProducts->text();

    // Execute query on remote database
    bool qpl=lDBEntry->m_pExternalDatabase->QueryProductList(lPluginFilter,lProductList,lProductFilter);
    if (!qpl)
    {
        GSLOG(SYSLOG_SEV_ERROR, " QueryProductList failed !");
        Message::critical("GEX error", "Impossible to query available products!");
        return;
    }
    if(lProductList.count() == 0)
    {
        Message::warning("GEX warning", "No product available!");
        return;
    }

    pickProducts(lProductList);
}

void SYASettingsWidget::onPickItems(void)
{
    // Get database pointer
    QString	lDBName = mUI->comboTDR->currentText();
    if(lDBName.isEmpty())
    {
        return;
    }
    GexDatabaseEntry* lDBEntry = Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(lDBName,true);
    if(!lDBEntry || !lDBEntry->m_pExternalDatabase)
    {
        return;
    }

    QString lProductFilter = mUI->editProducts->text();

    GexDatabaseFilter lFilter;

    lFilter.addNarrowFilter(gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT], lProductFilter);
    lFilter.strDataTypeQuery = mUI->comboTestingStage->currentText();
    lFilter.strDatabaseLogicalName = lDBName;

    // Get list of strings to fill in list box
    QStringList lBinningList = GS::Gex::Engine::GetInstance().GetDatabaseEngine().QueryGetBinningList(
                                   lFilter, mUI->comboItemType->currentData() == "S");
    if(lBinningList.count() == 0)
    {
        return;
    }
    lBinningList.append("-1#GEX#Yield (all pass)");

    pickBins(lBinningList);
}


}

}
