///////////////////////////////////////////////////////////
// All about 'Enterprise Reports' (SQL based reports)
///////////////////////////////////////////////////////////
#include <QColorDialog>
#include <QColor>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QList>

#include "settings_dialog.h"
#include "settings_sql.h"		// Holds SQL related defines
#include "engine.h"
#include "gex_shared.h"
#include "gex_constants.h"
#include "db_engine.h"
#include "browser_dialog.h"
#include "db_onequery_wizard.h"
#include "compare_query_wizard.h"
#include "db_transactions.h"
#include "gex_database_entry.h"
#include "db_external_database.h"
#include "pickfilter_dialog.h"
#include "pickserie_dialog.h"
#include "report_build.h"
#include "report_options.h"
#include "report_template.h"
#include "report_template_io.h"
#include "report_template_gui.h"
#include "message.h"

// in main.cpp
extern GexMainwindow	*pGexMainWindow;

// report_build.cpp
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)

// script_wizard.h
extern void ConvertToScriptString(QString &strFile);

// in classes.cpp
extern bool SetCurrentComboItem(QComboBox *pCombo, const QString & strItem, bool bInsertIfMissing=false);

extern bool FillComboBox(QComboBox * pCombo, const char * szTextTab[]);


///////////////////////////////////////////////////////////
// Adds a char array to QStringList.
///////////////////////////////////////////////////////////
static void AddCharArrayToStringList(QStringList & strlStringList, const char *pItems [])
{
    int	nIndex=0;

    while(pItems[nIndex])
        strlStringList.append(pItems[nIndex++]);
}

///////////////////////////////////////////////////////////
// Settings switched to : Create SQL Report
///////////////////////////////////////////////////////////
void GexSettings::OnDoSqlReport(void)
{
    // Keep track of Module menu selected
    if(pGexMainWindow != NULL)
        pGexMainWindow->iGexAssistantSelected = GEX_MODULE_INSTANT_REPORT;

    // ensure GUI buttons are in known status.
    OnSqlReport_UpdateGui();

    // Show relevant report GUI
    widgetStackReport->setCurrentIndex(WIDGET_REPORT_SQL);
    toolbox_Mission->setCurrentIndex(0);
}


///////////////////////////////////////////////////////////
// Make sure filters are updated
///////////////////////////////////////////////////////////
void GexSettings::UpdateFilters(bool bKeepSelectedFilters,
                                bool /*bKeepValues = false*/)
{
    if(m_SqlReportSelected == 0)
        return;

    if(!pGexMainWindow)
        return;

    // Get database ptr
    GexOneQueryWizardPage1	*pWizard = pGexMainWindow->pWizardOneQuery;
    QString					strDatabaseName = pWizard->comboBoxDatabaseName->currentText();
    if(strDatabaseName.startsWith("[Local]") || strDatabaseName.startsWith("[Server]"))
        strDatabaseName = strDatabaseName.section("]",1).trimmed();
    GexDatabaseEntry		*pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(strDatabaseName);

    if(!pDatabaseEntry || !pDatabaseEntry->m_pExternalDatabase)
        return;

    // X-Axis charting fields and Split-By fields
    const char *szTimeFilters[]=
    {
        "Day","Week","Month","Year","--------------",0
    };

    // Save filters and values
    QString					strUPH_Xaxis, strUPH_SplitBy;
    QString					strConsolidatedYield_Xaxis, strConsolidatedYield_SplitBy;;
    QString					strYield_Xaxis, strYield_SplitBy;
    QString					strYieldWizard_Xaxis;
    QString					strGenealogy_YieldVsYield_TestingStage_Xaxis, strGenealogy_YieldVsYield_TestingStage_Yaxis;
    QString					strGenealogy_YieldVsYield_Binning_Xaxis, strGenealogy_YieldVsYield_Binning_Yaxis;
    QString					strGenealogy_YieldVsYield_Binlist_Xaxis, strGenealogy_YieldVsYield_Binlist_Yaxis;
    QString					strGenealogy_YieldVsParameter_TestingStage_Xaxis, strGenealogy_YieldVsParameter_TestingStage_Yaxis;
    QString					strGenealogy_YieldVsParameter_Binning_Yaxis;
    QString					strGenealogy_YieldVsParameter_Parameter_Xaxis, strGenealogy_YieldVsParameter_Binlist_Yaxis;
    QStringList				strlYieldWizard_SelectedChartSplitFields, strlYieldWizard_SelectedLayerSplitFields;
    QTreeWidgetItem			*pItem;
    bool					bGenealogy_YieldVsYield_WaferGranularity, bGenealogy_YieldVsYield_GranularityButtonGroupEnabled;
    bool					bGenealogy_YieldVsParameter_WaferGranularity, bGenealogy_YieldVsParameter_GranularityButtonGroupEnabled;

    strUPH_Xaxis = comboSQL_UPH_Xaxis->currentText();
    strUPH_SplitBy = comboSQL_UPH_SplitBy->currentText();
    strConsolidatedYield_Xaxis = comboSQL_ConsolidatedYield_Xaxis->currentText();
    strConsolidatedYield_SplitBy = comboSQL_ConsolidatedYield_SplitBy->currentText();
    strYield_Xaxis = comboSQL_Yield_Xaxis->currentText();
    strYield_SplitBy = comboSQL_Yield_SplitBy->currentText();
    strYieldWizard_Xaxis = comboSQL_YieldWizard_Xaxis->currentText();
    pItem = listviewSQL_YieldWizard_SelectedChartSplitFields->itemAt(0,0);
    while(pItem)
    {
        strlYieldWizard_SelectedChartSplitFields += pItem->text(0);
        pItem = listviewSQL_YieldWizard_SelectedChartSplitFields->itemBelow(pItem);
    }
    pItem = listviewSQL_YieldWizard_SelectedLayerSplitFields->itemAt(0,0);
    while(pItem)
    {
        strlYieldWizard_SelectedLayerSplitFields += pItem->text(0);
        pItem = listviewSQL_YieldWizard_SelectedLayerSplitFields->itemBelow(pItem);
    }
    strGenealogy_YieldVsYield_TestingStage_Xaxis = comboSQL_Genealogy_YieldVsYield_TestingStage_Xaxis->currentText();
    strGenealogy_YieldVsYield_TestingStage_Yaxis = comboSQL_Genealogy_YieldVsYield_TestingStage_Yaxis->currentText();
    strGenealogy_YieldVsYield_Binning_Xaxis = comboSQL_Genealogy_YieldVsYield_Binning_Xaxis->currentText();
    strGenealogy_YieldVsYield_Binning_Yaxis = comboSQL_Genealogy_YieldVsYield_Binning_Yaxis->currentText();
    strGenealogy_YieldVsYield_Binlist_Xaxis = editSQL_Genealogy_YieldVsYield_Binlist_Xaxis->text();
    strGenealogy_YieldVsYield_Binlist_Yaxis = editSQL_Genealogy_YieldVsYield_Binlist_Yaxis->text();
    strGenealogy_YieldVsParameter_TestingStage_Xaxis = comboSQL_Genealogy_YieldVsParameter_TestingStage_Xaxis->currentText();
    strGenealogy_YieldVsParameter_TestingStage_Yaxis = comboSQL_Genealogy_YieldVsParameter_TestingStage_Yaxis->currentText();
    strGenealogy_YieldVsParameter_Binning_Yaxis = comboSQL_Genealogy_YieldVsParameter_Binning_Yaxis->currentText();
    strGenealogy_YieldVsParameter_Parameter_Xaxis = editSQL_Genealogy_YieldVsParameter_Parameter_Xaxis->text();
    strGenealogy_YieldVsParameter_Binlist_Yaxis = editSQL_Genealogy_YieldVsParameter_Binlist_Yaxis->text();
    bGenealogy_YieldVsYield_WaferGranularity = radioSQL_Genealogy_YieldVsYield_Granularity_Wafer->isChecked();
    bGenealogy_YieldVsYield_GranularityButtonGroupEnabled = frameSQL_Genealogy_YieldVsYield_Granularity->isEnabled();
    bGenealogy_YieldVsParameter_WaferGranularity = radioSQL_Genealogy_YieldVsParameter_Granularity_Wafer->isChecked();
    bGenealogy_YieldVsParameter_GranularityButtonGroupEnabled = frameSQL_Genealogy_YieldVsParameter_Granularity->isEnabled();

    // Clear current filters
    comboSQL_UPH_Xaxis->clear();
    comboSQL_UPH_SplitBy->clear();
    comboSQL_ConsolidatedYield_Xaxis->clear();
    comboSQL_ConsolidatedYield_SplitBy->clear();
    comboSQL_Yield_Xaxis->clear();
    comboSQL_Yield_SplitBy->clear();
    comboSQL_YieldWizard_Xaxis->clear();
    listviewSQL_YieldWizard_AllChartSplitFields->clear();
    listviewSQL_YieldWizard_AllChartSplitFields->setRootIsDecorated(false);

    listviewSQL_YieldWizard_SelectedChartSplitFields->clear();
    listviewSQL_YieldWizard_SelectedChartSplitFields->setRootIsDecorated(false);

    listviewSQL_YieldWizard_AllLayerSplitFields->clear();
    listviewSQL_YieldWizard_AllLayerSplitFields->setRootIsDecorated(false);

    listviewSQL_YieldWizard_SelectedLayerSplitFields->clear();
    listviewSQL_YieldWizard_SelectedLayerSplitFields->setRootIsDecorated(false);

    comboSQL_Genealogy_YieldVsYield_TestingStage_Xaxis->clear();
    comboSQL_Genealogy_YieldVsYield_TestingStage_Yaxis->clear();
    editSQL_Genealogy_YieldVsYield_Binlist_Xaxis->clear();
    editSQL_Genealogy_YieldVsYield_Binlist_Yaxis->clear();
    frameSQL_Genealogy_YieldVsYield_Granularity->setEnabled(false);
    radioSQL_Genealogy_YieldVsYield_Granularity_Lot->setChecked(true);
    comboSQL_Genealogy_YieldVsParameter_TestingStage_Xaxis->clear();
    comboSQL_Genealogy_YieldVsParameter_TestingStage_Yaxis->clear();
    editSQL_Genealogy_YieldVsParameter_Parameter_Xaxis->clear();
    editSQL_Genealogy_YieldVsParameter_Binlist_Yaxis->clear();
    frameSQL_Genealogy_YieldVsParameter_Granularity->setEnabled(false);
    radioSQL_Genealogy_YieldVsParameter_Granularity_Lot->setChecked(true);

    // Fill filter combo boxes
    QStringList		strlYieldWizard_Fields;
    FillComboBox(comboSQL_UPH_Xaxis, szTimeFilters);
    FillComboBox(comboSQL_UPH_SplitBy, szTimeFilters);
    FillComboBox(comboSQL_ConsolidatedYield_Xaxis, szTimeFilters);
    FillComboBox(comboSQL_ConsolidatedYield_SplitBy, szTimeFilters);
    FillComboBox(comboSQL_Yield_Xaxis, szTimeFilters);
    FillComboBox(comboSQL_Yield_SplitBy, szTimeFilters);
    FillComboBox(comboSQL_YieldWizard_Xaxis, szTimeFilters);

    AddCharArrayToStringList(strlYieldWizard_Fields, szTimeFilters);

    // Add SiteNb filter
    comboSQL_Yield_Xaxis->addItem(gexLabelFilterChoices[GEX_QUERY_FILTER_SITENBR]);
    comboSQL_Yield_Xaxis->addItem("--------------");
    comboSQL_Yield_SplitBy->addItem(gexLabelFilterChoices[GEX_QUERY_FILTER_SITENBR]);
    comboSQL_Yield_SplitBy->addItem("--------------");

    // Added mapped fields filters
    QStringList				strlFilters_All, strlFilters_LotLevel, strlFilters_WaferLevel;
    QStringList::iterator	it;
    pDatabaseEntry->m_pExternalDatabase->GetLabelFilterChoices(pGexMainWindow->pWizardOneQuery->comboBoxDatabaseType->currentText(), strlFilters_All);
    pDatabaseEntry->m_pExternalDatabase->GetLabelFilterChoicesLotLevel(pGexMainWindow->pWizardOneQuery->comboBoxDatabaseType->currentText(), strlFilters_LotLevel);
    pDatabaseEntry->m_pExternalDatabase->GetLabelFilterChoicesWaferLevel(pGexMainWindow->pWizardOneQuery->comboBoxDatabaseType->currentText(), strlFilters_WaferLevel);
    if(!strlFilters_All.isEmpty())
    {
        comboSQL_UPH_Xaxis->addItems(strlFilters_All);
        comboSQL_UPH_SplitBy->addItems(strlFilters_All);
        comboSQL_Yield_Xaxis->addItems(strlFilters_All);
        comboSQL_Yield_SplitBy->addItems(strlFilters_All);
        comboSQL_YieldWizard_Xaxis->addItems(strlFilters_All);

        for(it=strlFilters_All.begin(); it!=strlFilters_All.end(); it++)
        {
            if((*it) != gexLabelFilterChoices[GEX_QUERY_FILTER_SITENBR])
                strlYieldWizard_Fields += (*it);
        }
    }
    if(!strlFilters_LotLevel.isEmpty())
    {
        comboSQL_ConsolidatedYield_Xaxis->addItems(strlFilters_LotLevel);
        comboSQL_ConsolidatedYield_SplitBy->addItems(strlFilters_LotLevel);
    }
    if(!strlFilters_WaferLevel.isEmpty())
    {
        comboSQL_ConsolidatedYield_Xaxis->addItems(strlFilters_WaferLevel);
        comboSQL_ConsolidatedYield_SplitBy->addItems(strlFilters_WaferLevel);
    }


    foreach(const QString &strString, strlYieldWizard_Fields)
    {
        QStringList strList(strString);
        // Field not in list, so add it!
        new QTreeWidgetItem(listviewSQL_YieldWizard_AllChartSplitFields, strList);
        new QTreeWidgetItem(listviewSQL_YieldWizard_AllLayerSplitFields, strList);
    }

    // Set some specific GUI items depending on database options
    if(pWizard->comboBoxDatabaseType->currentText().toLower()	== "final test")
        widgetstackSQL_YieldWizard_Maverick->setCurrentIndex(1);
    else
        widgetstackSQL_YieldWizard_Maverick->setCurrentIndex(0);

    // Genealogy controls
    editSQL_Genealogy_YieldVsYield_Product->clear();
    groupSQL_Genealogy_YieldVsYield_Xaxis->setEnabled(false);
    groupSQL_Genealogy_YieldVsYield_Yaxis->setEnabled(false);
    groupSQL_Genealogy_YieldVsYield_Options->setEnabled(false);
    pDatabaseEntry->m_pExternalDatabase->GetSupportedTestingStages(m_strlAvailableTestingStages);
    comboSQL_Genealogy_YieldVsYield_TestingStage_Xaxis->addItems(m_strlAvailableTestingStages);
    comboSQL_Genealogy_YieldVsYield_TestingStage_Yaxis->addItems(m_strlAvailableTestingStages);
    SetCurrentComboItem(comboSQL_Genealogy_YieldVsYield_Binning_Xaxis, "PASS Bins only");
    SetCurrentComboItem(comboSQL_Genealogy_YieldVsYield_Binning_Yaxis, "PASS Bins only");
    editSQL_Genealogy_YieldVsParameter_Product->clear();
    groupSQL_Genealogy_YieldVsParameter_Xaxis->setEnabled(false);
    groupSQL_Genealogy_YieldVsParameter_Yaxis->setEnabled(false);
    groupSQL_Genealogy_YieldVsParameter_Options->setEnabled(false);
    pDatabaseEntry->m_pExternalDatabase->GetTestingStageName_Foundry(m_strFoundryTestingStage);
    comboSQL_Genealogy_YieldVsParameter_TestingStage_Xaxis->addItem(m_strFoundryTestingStage);
    comboSQL_Genealogy_YieldVsParameter_TestingStage_Yaxis->addItems(m_strlAvailableTestingStages);
    SetCurrentComboItem(comboSQL_Genealogy_YieldVsParameter_Binning_Yaxis, "PASS Bins only");

    // Set saved values?
    if(!bKeepSelectedFilters)
        return;

    SetCurrentComboItem(comboSQL_UPH_Xaxis, strUPH_Xaxis);
    SetCurrentComboItem(comboSQL_UPH_SplitBy, strUPH_SplitBy);
    SetCurrentComboItem(comboSQL_ConsolidatedYield_Xaxis, strConsolidatedYield_Xaxis);
    SetCurrentComboItem(comboSQL_ConsolidatedYield_SplitBy, strConsolidatedYield_SplitBy);
    SetCurrentComboItem(comboSQL_Yield_Xaxis, strYield_Xaxis);
    SetCurrentComboItem(comboSQL_Yield_SplitBy, strYield_SplitBy);
    SetCurrentComboItem(comboSQL_YieldWizard_Xaxis, strYieldWizard_Xaxis);
    for(it=strlYieldWizard_SelectedChartSplitFields.begin(); it!=strlYieldWizard_SelectedChartSplitFields.end(); it++)
    {
        pItem = listviewSQL_YieldWizard_AllChartSplitFields->findItems(*it, Qt::MatchExactly).first();
        if(pItem)
        {
            QStringList list;
            list.append(*it);
            new QTreeWidgetItem(listviewSQL_YieldWizard_SelectedChartSplitFields, list);
            delete pItem;
        }
    }
    for(it=strlYieldWizard_SelectedChartSplitFields.begin(); it!=strlYieldWizard_SelectedChartSplitFields.end(); it++)
    {
        pItem = listviewSQL_YieldWizard_AllChartSplitFields->findItems(*it, Qt::MatchExactly).first();
        if(pItem)
        {
            QStringList list;
            list.append(*it);
            new QTreeWidgetItem(listviewSQL_YieldWizard_SelectedChartSplitFields, list);
            delete pItem;
        }
    }
    for(it=strlYieldWizard_SelectedLayerSplitFields.begin(); it!=strlYieldWizard_SelectedLayerSplitFields.end(); it++)
    {
        pItem = listviewSQL_YieldWizard_AllLayerSplitFields->findItems(*it, Qt::MatchExactly).first();
        if(pItem)
        {
            QStringList list;
            list.append(*it);
            new QTreeWidgetItem(listviewSQL_YieldWizard_SelectedLayerSplitFields, list);
            delete pItem;
        }
    }
    SetCurrentComboItem(comboSQL_Genealogy_YieldVsYield_TestingStage_Xaxis, strGenealogy_YieldVsYield_TestingStage_Xaxis);
    SetCurrentComboItem(comboSQL_Genealogy_YieldVsYield_TestingStage_Yaxis, strGenealogy_YieldVsYield_TestingStage_Yaxis);
    SetCurrentComboItem(comboSQL_Genealogy_YieldVsYield_Binning_Xaxis, strGenealogy_YieldVsYield_Binning_Xaxis);
    SetCurrentComboItem(comboSQL_Genealogy_YieldVsYield_Binning_Yaxis, strGenealogy_YieldVsYield_Binning_Yaxis);
    editSQL_Genealogy_YieldVsYield_Binlist_Xaxis->setText(strGenealogy_YieldVsYield_Binlist_Xaxis);
    editSQL_Genealogy_YieldVsYield_Binlist_Yaxis->setText(strGenealogy_YieldVsYield_Binlist_Yaxis);
    radioSQL_Genealogy_YieldVsYield_Granularity_Wafer->setChecked(bGenealogy_YieldVsYield_WaferGranularity);
    frameSQL_Genealogy_YieldVsYield_Granularity->setEnabled(bGenealogy_YieldVsYield_GranularityButtonGroupEnabled);
    SetCurrentComboItem(comboSQL_Genealogy_YieldVsParameter_TestingStage_Xaxis, strGenealogy_YieldVsParameter_TestingStage_Xaxis);
    SetCurrentComboItem(comboSQL_Genealogy_YieldVsParameter_TestingStage_Yaxis, strGenealogy_YieldVsParameter_TestingStage_Yaxis);
    SetCurrentComboItem(comboSQL_Genealogy_YieldVsParameter_Binning_Yaxis, strGenealogy_YieldVsParameter_Binning_Yaxis);
    editSQL_Genealogy_YieldVsParameter_Parameter_Xaxis->setText(strGenealogy_YieldVsParameter_Parameter_Xaxis);
    editSQL_Genealogy_YieldVsParameter_Binlist_Yaxis->setText(strGenealogy_YieldVsParameter_Binlist_Yaxis);
    radioSQL_Genealogy_YieldVsParameter_Granularity_Wafer->setChecked(bGenealogy_YieldVsParameter_WaferGranularity);
    frameSQL_Genealogy_YieldVsParameter_Granularity->setEnabled(bGenealogy_YieldVsParameter_GranularityButtonGroupEnabled);
}

///////////////////////////////////////////////////////////
// SQL Report GUI browsed (no report selected yet)
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_MissionChanged(void)
{
    // No report type selected yet: clear flags
    m_SqlReportSelected = SQL_REPORT_NONE;
    widgetStackSqlMissions->setCurrentIndex(SQL_MISSION_WIDGET_SELECT);

    // Prevent user from Building report!
    // buttonBuildReport->setEnabled(false);		// PYC, 30/05/2011, case 4819

    // Set all tool buttons to Off
    toolButtonSql_UPH->setChecked(false);
    toolButtonSql_Yield->setChecked(false);
    toolButtonSql_WYR_Standard->setChecked(false);
    toolButtonSql_ConsolidatedYield->setChecked(false);
    toolButtonSql_YieldWizard->setChecked(false);
    toolButtonSql_Genealogy_YieldVsYield->setChecked(false);
    toolButtonSql_Genealogy_YieldVsParameter->setChecked(false);
}

///////////////////////////////////////////////////////////
// Startup routine: called at Examinator launch to reset to default SQL status
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_Startup(void)
{
    const char *szXaxis[]=
    {
        "Day","Week","Month","Year","Tester name","Product ID",
        "--------------",
        "Data Origin","DIB ID","DIB type","Facility ID","Family ID","Floor ID","Freq/Step ID",
        "Handler ID","Handler type","Load board ID","Load board type","Lot ID",
        "Operator name","Package type","Prober ID","Prober type","Process ID","Program name","Program rev.",
        "Retest instance","Sublot ID","Temperature","Tester type","Testing code","Site#",0
    };

    const char *szLeftRightaxis[]=
    {
        "Disabled (hide axis)","Serie - Yield","Total - Volume","Total - Splitlots","Total - Wafers","Total - Lots",
        "Total - Maverick wafers","Total - Maverick lots",0
    };

    const char *szSplitBy[]=
    {
        "Tester name","Site#",
        "--------------",
        "Data Origin","DIB ID","DIB type","Facility ID","Family ID","Floor ID","Freq/Step ID",
        "Handler ID","Handler type","Load board ID","Load board type","Lot ID",
        "Operator name","Package type","Prober ID","Prober type","Process ID","Product ID","Program name","Program rev.",
        "Retest instance","Sublot ID","Temperature","Tester type","Testing code",0
    };

    toolbox_Mission->setCurrentIndex(toolbox_Mission->count()-1);
    m_SqlReportSelected = SQL_REPORT_NONE;

    // Hide all SQL advanced settings.
    groupSQL_UPH_MoreSettings->hide();					// UPH report
    groupSQL_Yield_MoreSettings->hide();				// Yield report
    groupSQL_ConsolidatedYield_MoreSettings->hide();	// Consolidated Yield report

    // X-Axis charting fields
    comboSQL_UPH_Xaxis->clear();
    FillComboBox(comboSQL_UPH_Xaxis, szXaxis);

    comboSQL_Yield_Xaxis->clear();
    FillComboBox(comboSQL_Yield_Xaxis, szXaxis);

    comboSQL_ConsolidatedYield_Xaxis->clear();
    comboSQL_ConsolidatedYield_Xaxis->addItem("Lot ID");

    comboSQL_YieldWizard_LeftAxis->clear();
    FillComboBox(comboSQL_YieldWizard_LeftAxis, szLeftRightaxis);
    comboSQL_YieldWizard_LeftAxis->setCurrentIndex(1);	// 2nd item = 'Volume'

    comboSQL_YieldWizard_RightAxis->clear();
    FillComboBox(comboSQL_YieldWizard_RightAxis, szLeftRightaxis);

    comboSQL_YieldWizard_Xaxis->clear();
    FillComboBox(comboSQL_YieldWizard_Xaxis, szXaxis);

    // Fill Yield Wizard Chart-Split 'All fields' list views.
    // Note: the 'split by layer' list is filled when user clicks the 'Next' button!
    int	iIndex=0;
    QStringList	listString;;
    listviewSQL_YieldWizard_AllChartSplitFields->clear();
    while(szSplitBy[iIndex])
    {
        // Get field
        QString strString = szSplitBy[iIndex];

        // Ensure the 'site#' field is NOT included to the list created
        if(strString.toLower().contains("site#") == 0)
            // Add field item to Chart fields listview
            listString.append(strString);

        // Move to next field available
        iIndex++;
    };
    new QTreeWidgetItem(listviewSQL_YieldWizard_AllChartSplitFields, listString);



    // Split-By fields
    comboSQL_UPH_SplitBy->clear();
    FillComboBox(comboSQL_UPH_SplitBy, szSplitBy);

    comboSQL_Yield_SplitBy->clear();
    FillComboBox(comboSQL_Yield_SplitBy, szSplitBy);

    // Load XML settings into GUI
    // If fail reading file (or file doesn't exist), quietly ignore
    QString strXmlFile =
        GS::Gex::Engine::GetInstance().Get("UserFolder").toString() +
        "/.galaxy_sql.grt";  // name = <path>/.galaxy_sql.grt
    GS::Gex::ReportTemplate   reportTemplate;
    GS::Gex::ReportTemplateIO reportTemplateIO;
    if (reportTemplateIO.ReadTemplateFromDisk(&reportTemplate, strXmlFile))
    {
        // Success loading Enterprise Reports XML settings file
        // Check if holds Settings + Update GUI
        GS::Gex::ReportTemplateSection* pSection;
        GS::Gex::ReportTemplateSectionIterator iter;
        for (iter  = reportTemplate.Begin();
             iter != reportTemplate.End(); ++iter)
        {
            pSection = *iter;
            switch(pSection->iSectionType)
            {
                case GEX_CRPT_WIDGET_ER:
                    // Update Examinator 'Enterprise Reports' GUI to
                    // show user preferences
                    OnSqlReport_LoadSettings(pSection->pEnterpriseReport);
                    return;
            }
        }
    }

    // No XML file, or failed loading it: Ensure we load the default settings into GUI
    OnSqlReport_LoadSettings(NULL);
}

///////////////////////////////////////////////////////////
// SQL Report GUI: User clicked '+' icon for more SQL settings
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_MoreSettings(void)
{
    QPushButton *pPushButton;	// To hold handle to 'More settings' button clicked
    QGroupBox	*pGroupBox;		// To hold handle to 'More Settings' group box

    // Check which SQL report is visible so to see which Widget to show/hide
    switch(m_SqlReportSelected)
    {
        case SQL_REPORT_PROD_UPH:
            pPushButton = buttonSQL_UPH_MoreSettings;
            pGroupBox = groupSQL_UPH_MoreSettings;
            break;

        case SQL_REPORT_PROD_YIELD:
            pPushButton = buttonSQL_Yield_MoreSettings;
            pGroupBox = groupSQL_Yield_MoreSettings;
            break;

        case SQL_REPORT_PROD_CONSOLIDATEDYIELD:
            pPushButton = buttonSQL_ConsolidatedYield_MoreSettings;
            pGroupBox = groupSQL_ConsolidatedYield_MoreSettings;
            break;

        default:
            return;
    }

    // Check button status
    if(pPushButton->isChecked())
    {
        // Button is up, current status is: Advanced settings hidden. So Show it now!
        pPushButton->setText("-");
        pGroupBox->show();
    }
    else
    {
        // Button is down, current status is: Advanced settigs visible. So hide it now!
        pPushButton->setText("+");
        pGroupBox->hide();
    }
}

///////////////////////////////////////////////////////////
// SQL Report Signal: some chart style fields modified
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_UpdateGui(void)
{
    QString strBackground = comboSQL_BackgroundStyle->currentText();
    if(strBackground.startsWith("Custom color...", Qt::CaseInsensitive) || strBackground.startsWith("Gradient color...", Qt::CaseInsensitive))
    {
        // Show color button
        buttonSQL_BackgroundColor->show();
    }
    else
    {
        // Hide color button
        buttonSQL_BackgroundColor->hide();
    }

    // Make sure GUI is updated for all specific report types
    OnSqlReport_Prod_UPH_UpdateGui();
    OnSqlReport_Prod_Yield_UpdateGui();
    OnSqlReport_Prod_ConsolidatedYield_UpdateGui();
    OnSqlReport_Prod_YieldWizard_UpdateGui();
    OnSqlReport_WYR_Standard_UpdateGui();
    OnSqlReport_Genealogy_YieldVsYield_UpdateGui();
    OnSqlReport_Genealogy_YieldVsParameter_UpdateGui();
}

///////////////////////////////////////////////////////////
// SQL Report: some chart style fields modified
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_LoadSettings(
    GS::Gex::CustomReportEnterpriseReportSection* pEnterpriseReport /*= NULL*/)
{
    // If custom GUI settings defined, used them; otherwise use defaults (as per pSQL constructor)
    GS::Gex::CustomReportEnterpriseReportSection* pNewEnterpriseReport = NULL;
    GS::Gex::CustomReportEnterpriseReportSection* pReport;
    if(pEnterpriseReport != NULL)
        pReport = pEnterpriseReport;
    else
    {
        pNewEnterpriseReport =
            new GS::Gex::CustomReportEnterpriseReportSection;
        pReport = pNewEnterpriseReport;
    }

    // Update Settings preferences (caller is probably Examinator loader with XML user settings)

    // Std GUI (UPH, Yield, Consolidated Yield)
    SetCurrentComboItem(comboSQL_BarStyle, pReport->m_strBarStyle);
    checkSQL_GradientBarColor->setChecked(pReport->m_bGradientBarColor);
    checkSQL_SemiTransparentBar->setChecked(pReport->m_bSemiTransparentBar);
    SetCurrentComboItem(comboSQL_LineStyle, pReport->m_strLineStyle);
    checkSQL_DashedLine->setChecked(pReport->m_bDashedLine);
    SetCurrentComboItem(comboSQL_LineSpots, pReport->m_strLineSpots);
    SetCurrentComboItem(comboSQL_BackgroundStyle, pReport->m_strBackgroundStyle);
    buttonSQL_DefaultColor->setActiveColor(pReport->m_cDefaultColor);
    buttonSQL_BackgroundColor->setActiveColor(pReport->m_cBackgroundColor);
    checkSQL_PrintChartValues->setChecked(pReport->m_bShowValueMarkers);
    checkSQL_DumpRawData->setChecked(pReport->m_bDumpRawData);

    // Report using Yield Wizard GUI
    SetCurrentComboItem(comboSQL_YieldWizard_DataSource, pReport->m_YieldWizard_strDataSource);
    SetCurrentComboItem(comboSQL_YieldWizard_ColorMgt, pReport->m_YieldWizard_strColorMgt);
    if(pReport->m_YieldWizard_bSoftBin)
        comboSQL_YieldWizard_BinType->setCurrentIndex(1);
    else
        comboSQL_YieldWizard_BinType->setCurrentIndex(0);
    // Global style
    SetCurrentComboItem(comboSQL_YieldWizard_BarStyle, pReport->m_YieldWizard_Global_strBarStyle);
    checkSQL_YieldWizard_GradientBarColor->setChecked(pReport->m_YieldWizard_Global_bGradientBarColor);
    checkSQL_YieldWizard_SemiTransparentBar->setChecked(pReport->m_YieldWizard_Global_bSemiTransparentBar);
    spinSQL_YieldWizard_OverlappingRatio->setValue(pReport->m_YieldWizard_Global_nOverlappingRatio);
    SetCurrentComboItem(comboSQL_YieldWizard_BackgroundStyle, pReport->m_YieldWizard_Global_strBackgroundStyle);
    buttonSQL_YieldWizard_BackgroundColor->setActiveColor(pReport->m_YieldWizard_Global_cBackgroundColor);
    SetCurrentComboItem(comboSQL_YieldWizard_LayerPolicy, pReport->m_YieldWizard_Global_strLayerPolicy);
    // Volume style
    SetCurrentComboItem(comboSQL_YieldWizard_VolumeStyle, pReport->m_YieldWizard_Volume_strChartingMode);
    SetCurrentComboItem(comboSQL_YieldWizard_VolumeDataLabels, pReport->m_YieldWizard_Volume_strDataLabels);
    SetCurrentComboItem(comboSQL_YieldWizard_VolumeLineStyle, pReport->m_YieldWizard_Volume_strLineStyle);
    SetCurrentComboItem(comboSQL_YieldWizard_VolumeLineSpots, pReport->m_YieldWizard_Volume_strLineSpots);
    SetCurrentComboItem(comboSQL_YieldWizard_VolumeLineProperty, pReport->m_YieldWizard_Volume_strLineProperty);
    buttonSQL_YieldWizard_VolumeColor->setActiveColor(pReport->m_YieldWizard_Volume_cColor);
    // Binning pareto options
    editSQL_YieldWizard_BinningParetoTitle->setText(pReport->m_YieldWizard_BinPareto_strTitle);
    SetCurrentComboItem(comboSQL_YieldWizard_BinningParetoBinnings, pReport->m_YieldWizard_BinPareto_strBinnings);
    spinSQL_YieldWizard_BinningParetoNbCat->setValue(pReport->m_YieldWizard_BinPareto_nMaxCategories);
    SetCurrentComboItem(comboSQL_YieldWizard_BinningParetoStyle, pReport->m_YieldWizard_BinPareto_strChartingMode);
    SetCurrentComboItem(comboSQL_YieldWizard_BinningParetoDataLabels, pReport->m_YieldWizard_BinPareto_strDataLabels);
    SetCurrentComboItem(comboSQL_YieldWizard_BinningParetoLineStyle, pReport->m_YieldWizard_BinPareto_strLineStyle);
    SetCurrentComboItem(comboSQL_YieldWizard_BinningParetoLineSpots, pReport->m_YieldWizard_BinPareto_strLineSpots);
    SetCurrentComboItem(comboSQL_YieldWizard_BinningParetoLineProperty, pReport->m_YieldWizard_BinPareto_strLineProperty);
    buttonSQL_YieldWizard_BinningParetoColor->setActiveColor(pReport->m_YieldWizard_BinPareto_cColor);
    // Maverick definition
    checkSQL_YieldWizard_BinParetoOverMaverick->setChecked(pReport->m_YieldWizard_bShowBinParetoOverMaverick);
    SetCurrentComboItem(comboSQL_YieldWizard_ETWT_MavWafer_AlarmType, pReport->m_YieldWizard_EtWt_MaverickWafer_strAlarmType);
    spinSQL_YieldWizard_ETWT_MavWafer_AlarmCount->setValue(pReport->m_YieldWizard_EtWt_MaverickWafer_nAlarmCount);
    spinSQL_YieldWizard_ETWT_MavLot_WaferCount->setValue(pReport->m_YieldWizard_EtWt_MaverickLot_nWaferCount);

    // Genealogy
    editSQL_Genealogy_YieldVsYield_Product->setText(pReport->m_Genealogy_strProduct);
    GexDbPlugin_ER_Parts_SerieDef	*pSerie_X = NULL;
    GexDbPlugin_ER_Parts_SerieDef	*pSerie_Y = NULL;
    if (pReport->m_plistSeries.count()>0)
    {
        pSerie_X = pReport->m_plistSeries.at(0);
        if (pReport->m_plistSeries.count()>1)
            pSerie_Y = pReport->m_plistSeries.at(1);
    }

    QString	strBinning;
    if(pSerie_X)
    {
        SetCurrentComboItem(comboSQL_Genealogy_YieldVsYield_TestingStage_Xaxis, pSerie_X->m_strTestingStage);
        strBinning = pSerie_X->m_strBinnings.toLower();
        if((strBinning.indexOf("pass") == -1) && (strBinning.indexOf("fail") == -1))
        {
            SetCurrentComboItem(comboSQL_Genealogy_YieldVsYield_Binning_Xaxis, "Specific Bins...");
            editSQL_Genealogy_YieldVsYield_Binlist_Xaxis->setText(pSerie_X->m_strBinnings);
        }
        else
            SetCurrentComboItem(comboSQL_Genealogy_YieldVsYield_Binning_Xaxis, pSerie_X->m_strBinnings);
    }
    if(pSerie_Y)
    {
        SetCurrentComboItem(comboSQL_Genealogy_YieldVsYield_TestingStage_Yaxis, pSerie_Y->m_strTestingStage);
        strBinning = pSerie_Y->m_strBinnings.toLower();
        if((strBinning.indexOf("pass") == -1) && (strBinning.indexOf("fail") == -1))
        {
            SetCurrentComboItem(comboSQL_Genealogy_YieldVsYield_Binning_Yaxis, "Specific Bins...");
            editSQL_Genealogy_YieldVsYield_Binlist_Yaxis->setText(pSerie_Y->m_strBinnings);
        }
        else
            SetCurrentComboItem(comboSQL_Genealogy_YieldVsYield_Binning_Yaxis, pSerie_Y->m_strBinnings);
    }
    radioSQL_Genealogy_YieldVsYield_Granularity_Lot->setChecked(true);
    if(pReport->m_Genealogy_Options_strGranularity.toLower() == "wafer")
        radioSQL_Genealogy_YieldVsYield_Granularity_Wafer->setChecked(true);

    // Delete object if created
    if(pNewEnterpriseReport)
        delete pNewEnterpriseReport;
    pNewEnterpriseReport=0;

    // Make sure all GUI controls are in a coherent state
    OnSqlReport_UpdateGui();
}

///////////////////////////////////////////////////////////
// SQL Report selected
///////////////////////////////////////////////////////////
void GexSettings::SqlReportSelected(int iSqlReport)
{
    // If first time a mission is selected, update filters
    if(m_SqlReportSelected == SQL_REPORT_NONE)
    {
        // Keep track of report selected
        m_SqlReportSelected = iSqlReport; // Eg: SQL_REPORT_PROD_UPH or SQL_REPORT_PROD_YIELD
        UpdateFilters(false);
    }
    else
        // Keep track of report selected
        m_SqlReportSelected = iSqlReport; // Eg: SQL_REPORT_PROD_UPH or SQL_REPORT_PROD_YIELD

    // Ensure the SQL widget tabs/stacks are correctly set
    switch(iSqlReport)
    {
        case SQL_REPORT_PROD_YIELD:
            widgetStackSqlMissions->setCurrentIndex(SQL_MISSION_WIDGET_STDGUI);
            tabWidgetSql_Std->setCurrentIndex(0);
            widgetStackSqlSettings_Std->setCurrentIndex(SQL_STDGUI_SETTIGS_WIDGET_YIELD);
            break;

        case SQL_REPORT_PROD_UPH:
            widgetStackSqlMissions->setCurrentIndex(SQL_MISSION_WIDGET_STDGUI);
            tabWidgetSql_Std->setCurrentIndex(0);
            widgetStackSqlSettings_Std->setCurrentIndex(SQL_STDGUI_SETTIGS_WIDGET_UPH);
            break;

        case SQL_REPORT_PROD_CONSOLIDATEDYIELD:
            widgetStackSqlMissions->setCurrentIndex(SQL_MISSION_WIDGET_STDGUI);
            tabWidgetSql_Std->setCurrentIndex(0);
            widgetStackSqlSettings_Std->setCurrentIndex(SQL_STDGUI_SETTIGS_WIDGET_CONSOLIDATEDYIELD);
            break;

        case SQL_REPORT_WYR_STANDARD:
            widgetStackSqlMissions->setCurrentIndex(SQL_MISSION_WIDGET_STDGUI);
            tabWidgetSql_Std->setCurrentIndex(0);
            widgetStackSqlSettings_Std->setCurrentIndex(SQL_STDGUI_SETTIGS_WIDGET_WYRSTD);
            break;

        case SQL_REPORT_PROD_YIELDWIZARD:
            widgetStackSqlMissions->setCurrentIndex(SQL_MISSION_WIDGET_WIZARD);
            tabWidgetSql_Wizard->setCurrentIndex(0);
            break;

        case SQL_REPORT_GENEALOGY_YIELDVSYIELD:
            widgetStackSqlMissions->setCurrentIndex(SQL_MISSION_WIDGET_STDGUI);
            tabWidgetSql_Std->setCurrentIndex(0);
            widgetStackSqlSettings_Std->setCurrentIndex(SQL_STDGUI_SETTIGS_WIDGET_GENEALOGY_YIELDVSYIELD);
            break;

        case SQL_REPORT_GENEALOGY_YIELDVSPARAMETER:
            widgetStackSqlMissions->setCurrentIndex(SQL_MISSION_WIDGET_STDGUI);
            tabWidgetSql_Std->setCurrentIndex(0);
            widgetStackSqlSettings_Std->setCurrentIndex(SQL_STDGUI_SETTIGS_WIDGET_GENEALOGY_YIELDVSPARAMETER);
            break;

        case SQL_REPORT_NONE:
        default:
            widgetStackSqlMissions->setCurrentIndex(SQL_MISSION_WIDGET_SELECT);
            return;
    }

    // All user to click 'Building report' button as a report type is defined!
    buttonBuildReport->setEnabled(true);
}

///////////////////////////////////////////////////////////
// SQL Report: write settings
///////////////////////////////////////////////////////////
void GexSettings::WriteSettings_SqlReport(FILE *hFile)
{
    // Build path to template file to create (to hold SQL query details in template format)
    m_strTemplateFile = GS::Gex::Engine::GetInstance().Get("UserFolder").toString() + "/.galaxy_sql.grt";		// name = <path>/.galaxy_sql.grt

    // Create template file to hold SQL info
    QFile fTemplateFile(m_strTemplateFile);
    if(!fTemplateFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return;	// Disk write error.

    // Assign file I/O stream
    QTextStream hTemplateFile(&fTemplateFile);

    // Write file header
    hTemplateFile << "<galaxy_template>" << endl;

    // Save relevant settings in file
    switch(m_SqlReportSelected)
    {
        case SQL_REPORT_NONE:
            break;

        case SQL_REPORT_PROD_YIELD:
            SqlReport_Prod_Yield_WriteSettings(hTemplateFile);
            break;

        case SQL_REPORT_PROD_UPH:
            SqlReport_Prod_UPH_WriteSettings(hTemplateFile);
            break;

        case SQL_REPORT_PROD_CONSOLIDATEDYIELD:
            SqlReport_Prod_ConsolidatedYield_WriteSettings(hTemplateFile);
            break;

        case SQL_REPORT_PROD_YIELDWIZARD:
            SqlReport_Prod_YieldWizard_WriteSettings(hTemplateFile);
            break;

        case SQL_REPORT_WYR_STANDARD:
            SqlReport_Wyr_Standard_WriteSettings(hTemplateFile);
            break;

        case SQL_REPORT_GENEALOGY_YIELDVSYIELD:
            SqlReport_Genealogy_YieldVsYield_WriteSettings(hTemplateFile);
            break;

        case SQL_REPORT_GENEALOGY_YIELDVSPARAMETER:
            SqlReport_Genealogy_YieldVsParameter_WriteSettings(hTemplateFile);
            break;
    }

    // Close template file created
    hTemplateFile << "</galaxy_template>" << endl;
    fTemplateFile.close();

    // Enable MyReport templare support where template file holds SQL report query details
    QString strString = m_strTemplateFile;
    ConvertToScriptString(strString);	// Convert file path to be script compatible '\' become '\\'
    fprintf(hFile,"  gexReportType('adv_my_report','template','%s');\n",strString.toLatin1().constData());
}

///////////////////////////////////////////////////////////
// SQL Report: Write charting style & options
///////////////////////////////////////////////////////////
void GexSettings::SqlReport_Options_WriteSettings(QTextStream &hTemplateFile)
{
    int		iValue;
    QString strColor;

    // Style options
    hTemplateFile << "<style>" << endl;

    // ------ Bar style
    hTemplateFile << "BarStyle=" << comboSQL_BarStyle->currentText() << endl;
    // Color type: gradient or plain
    iValue = (checkSQL_GradientBarColor->isChecked()) ? 1: 0;
    hTemplateFile << "BarColorGradient=" << QString::number(iValue) << endl;
    // Color mode: semi-transparent (or plain)
    iValue = (checkSQL_SemiTransparentBar->isChecked()) ? 1: 0;
    hTemplateFile << "BarColorSemiTransparent=" << QString::number(iValue) << endl;

    // ------ Line style
    hTemplateFile << "LineStyle=" << comboSQL_LineStyle->currentText() << endl;
    // Dashed line ?
    iValue = (checkSQL_DashedLine->isChecked()) ? 1: 0;
    hTemplateFile << "DashedLine=" << QString::number(iValue) << endl;
    // Spot type
    hTemplateFile << "LineSpots=" << comboSQL_LineSpots->currentText() << endl;

    // ------ Background style
    hTemplateFile << "BackgroundStyle=" << comboSQL_BackgroundStyle->currentText() << endl;
    // Background color
    QColor cColor = buttonSQL_BackgroundColor->activeColor();
    strColor.sprintf("0x%02x%02x%02x",cColor.red(),cColor.green(),cColor.blue());
    hTemplateFile << "BackgroundColor=" << strColor	<< endl;											//  Bkg color RGB Hexa. string: 0xRRGGBB

    // ------ More styles: Dislay value markers on charts.
    iValue = (checkSQL_PrintChartValues->isChecked()) ? 1: 0;
    hTemplateFile << "ValueMarkers=" << QString::number(iValue) << endl;

    hTemplateFile << "</style>" << endl;

    // Advanced options
    hTemplateFile << "<advanced>" << endl;

    // Enable dump of SQL raw data
    iValue = (checkSQL_DumpRawData->isChecked()) ? 1: 0;
    hTemplateFile << "DumpRawData=" << QString::number(iValue) << endl;
    // Default color
    cColor = buttonSQL_DefaultColor->activeColor();
    strColor.sprintf("0x%02x%02x%02x",cColor.red(),cColor.green(),cColor.blue());
    hTemplateFile << "DefaultColor=" << strColor	<< endl;											//  Bkg color RGB Hexa. string: 0xRRGGBB

    hTemplateFile << "</advanced>" << endl;
}

///////////////////////////////////////////////////////////
// SQL Report = Production - UPH
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Report selected
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_Prod_UPH(void)
{
    // Update button states
    toolButtonSql_UPH->setChecked(true);
    toolButtonSql_Yield->setChecked(false);
    toolButtonSql_WYR_Standard->setChecked(false);
    toolButtonSql_ConsolidatedYield->setChecked(false);
    toolButtonSql_YieldWizard->setChecked(false);
    toolButtonSql_Genealogy_YieldVsYield->setChecked(false);
    toolButtonSql_Genealogy_YieldVsParameter->setChecked(false);

    // Keep track of report selected
    SqlReportSelected(SQL_REPORT_PROD_UPH);

    // Ensure UPH Gui is up to date
    OnSqlReport_Prod_UPH_UpdateGui();

    // Focus on label field.
    editSQL_UPH_Title->setFocus();
}

///////////////////////////////////////////////////////////
// Make sure all controls are in a correct state
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_Prod_UPH_UpdateGui(void)
{
    if(checkSQL_UPH_SplitBy->isChecked())
    {
        // Split charting enabled
        comboSQL_UPH_SplitBy->show();
        comboSQL_UPH_MultiChart->show();
    }
    else
    {
        // Split charting disabled
        comboSQL_UPH_SplitBy->hide();
        comboSQL_UPH_MultiChart->hide();
    }
}

///////////////////////////////////////////////////////////
// Write settings
///////////////////////////////////////////////////////////
void GexSettings::SqlReport_Prod_UPH_WriteSettings(QTextStream &hTemplateFile)
{
    // Get database name
    QString strDatabaseName = pGexMainWindow->pWizardOneQuery->comboBoxDatabaseName->currentText();

    // Skip the [Local]/[Server] info, and extract database name.
    if(strDatabaseName.startsWith("[Local]") || strDatabaseName.startsWith("[Server]"))
        strDatabaseName = strDatabaseName.section("]",1).trimmed();

    // Home page
    hTemplateFile << "<section_home_page>" << endl;								// Start template marker
    hTemplateFile << "HomeText="<< ReportOptions.GetOption("output", "front_page_text").toString() << endl; //hTemplateFile << "HomeText=" << ReportOptions.strFrontPageText << endl;		// Home page text
    hTemplateFile << "HomeLogo="<< ReportOptions.GetOption("output", "front_page_image").toString() << endl;	// Home page logo path //hTemplateFile << "HomeLogo=" << ReportOptions.strFrontPageImage << endl;	// Home page logo path
    hTemplateFile << "</section_home_page>" << endl;							// End template marker

    // Write UPH report settings
    hTemplateFile << "<section_enterprise_report>" << endl;
    hTemplateFile << "Title=" << editSQL_UPH_Title->text() << endl;
    hTemplateFile << "ReportFunction=Prod_UPH" << endl;

    // Data (Database, Time period, filters)
    hTemplateFile << "<data>" << endl;											// Data marker
    hTemplateFile << "DatabaseName=" << strDatabaseName  << endl;
    hTemplateFile << "DatabaseType=" << pGexMainWindow->pWizardOneQuery->comboBoxDatabaseType->currentText() << endl;
    hTemplateFile << "TimePeriod=" << gexTimePeriodChoices[pGexMainWindow->pWizardOneQuery->comboBoxTimePeriod->currentIndex()] << endl;
    hTemplateFile << "CalendarFrom=" << pGexMainWindow->pWizardOneQuery->FromDate->date().toString(Qt::ISODate) << endl;
    hTemplateFile << "CalendarTo=" << pGexMainWindow->pWizardOneQuery->ToDate->date().toString(Qt::ISODate) << endl;
    QStringList strlFilters = pGexMainWindow->pWizardOneQuery->GetFiltersXmlLines();
    hTemplateFile << "<filters>" << endl;
    for(QStringList::Iterator it = strlFilters.begin(); it != strlFilters.end(); ++it )
        hTemplateFile << *it << endl;
    hTemplateFile << "</filters>" << endl;
    hTemplateFile << "</data>" << endl;											// Data end marker

    // UPH report settings
    hTemplateFile << "<prod_uph>" << endl;
    hTemplateFile << "Aggregate=" << comboSQL_UPH_Xaxis->currentText() << endl;
    QString strYAxis = comboSQL_UPH_Yaxis->currentText().toLower();
    if(strYAxis == "uph only")
    {
        hTemplateFile << "LeftAxis=" << "Total - UPH" << endl;
        hTemplateFile << "RightAxis=" << "Disabled (hide axis)" << endl;
    }
    else if(strYAxis == "volume only")
    {
        hTemplateFile << "LeftAxis=" << "Total - Volume" << endl;
        hTemplateFile << "RightAxis=" << "Disabled (hide axis)" << endl;
    }
    else
    {
        hTemplateFile << "LeftAxis=" << "Total - UPH" << endl;
        hTemplateFile << "RightAxis=" << "Total - Volume" << endl;
    }
    hTemplateFile << "ChartingMode=" << comboSQL_UPH_Style->currentText() << endl;

    // If 'Split by' enabled, save additional GUI fields
    if(checkSQL_UPH_SplitBy->isChecked())
    {
        hTemplateFile << "SplitField=" << comboSQL_UPH_SplitBy->currentText() << endl;
        hTemplateFile << "LayerMode=" << comboSQL_UPH_MultiChart->currentText() << endl;
    }

    // Write charting style & options
    SqlReport_Options_WriteSettings(hTemplateFile);

    // Close UPH section
    hTemplateFile << "</prod_uph>" << endl;

    // Close section
    hTemplateFile << "</section_enterprise_report>" << endl;
}

///////////////////////////////////////////////////////////
// SQL Report = Production - Yield
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Report selected
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_Prod_Yield(void)
{
    // Update button states
    toolButtonSql_UPH->setChecked(false);
    toolButtonSql_Yield->setChecked(true);
    toolButtonSql_WYR_Standard->setChecked(false);
    toolButtonSql_ConsolidatedYield->setChecked(false);
    toolButtonSql_YieldWizard->setChecked(false);
    toolButtonSql_Genealogy_YieldVsYield->setChecked(false);
    toolButtonSql_Genealogy_YieldVsParameter->setChecked(false);

    // Keep track of report selected
    SqlReportSelected(SQL_REPORT_PROD_YIELD);

    // Ensure Yield Gui is up to date
    OnSqlReport_Prod_Yield_UpdateGui();

    // Focus on label field.
    editSQL_Yield_Title->setFocus();
}

///////////////////////////////////////////////////////////
// Make sure all controls are in a correct state
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_Prod_Yield_UpdateGui(void)
{
    if(checkSQL_Yield_SplitBy->isChecked())
    {
        // Split charting enabled
        comboSQL_Yield_SplitBy->show();
        comboSQL_Yield_MultiChart->show();
    }
    else
    {
        // Split charting disabled
        comboSQL_Yield_SplitBy->hide();
        comboSQL_Yield_MultiChart->hide();
    }

    if(comboSQL_Yield_Binning->currentIndex() == 0)
    {
        editSQL_Yield_Bin->hide();
        buttonSQL_Yield_PickFilter_Binning->hide();
    }
    else
    {
        editSQL_Yield_Bin->show();
        buttonSQL_Yield_PickFilter_Binning->show();
    }
}

///////////////////////////////////////////////////////////
// User clicked the binning 'PickFilter' button
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_Prod_Yield_PickFilter_Binning(void)
{
    QString strBinning;

    if(comboSQL_Yield_Binning->currentIndex() == 1)
        strBinning = pGexMainWindow->pWizardOneQuery->SqlPickBinningList(false, false);
    else if(comboSQL_Yield_Binning->currentIndex() == 2)
        strBinning = pGexMainWindow->pWizardOneQuery->SqlPickBinningList(false, true);

    if(!strBinning.isEmpty())
        editSQL_Yield_Bin->setText(strBinning);
}

///////////////////////////////////////////////////////////
// Write settings
///////////////////////////////////////////////////////////
void GexSettings::SqlReport_Prod_Yield_WriteSettings(QTextStream &hTemplateFile)
{
    // Get database name
    QString strDatabaseName = pGexMainWindow->pWizardOneQuery->comboBoxDatabaseName->currentText();

    // Skip the [Local]/[Server] info, and extract database name.
    if(strDatabaseName.startsWith("[Local]") || strDatabaseName.startsWith("[Server]"))
        strDatabaseName = strDatabaseName.section("]",1).trimmed();

    // Home page
    hTemplateFile<< "<section_home_page>" << endl;								// Start template marker
    hTemplateFile<< "HomeText=" << ReportOptions.GetOption("output", "front_page_text").toString() << endl; //hTemplateFile << "HomeText=" << ReportOptions.strFrontPageText << endl;		// Home page text
    hTemplateFile<< "HomeLogo=" << ReportOptions.GetOption("output", "front_page_image").toString() << endl;	//hTemplateFile << "HomeLogo=" << ReportOptions.strFrontPageImage << endl;	// Home page logo path
    hTemplateFile<< "</section_home_page>" << endl;							// End template marker

    // Write Yield/Volume report settings
    hTemplateFile << "<section_enterprise_report>" << endl;
    hTemplateFile << "Title=" << editSQL_Yield_Title->text() << endl;
    hTemplateFile << "ReportFunction=Prod_Yield" << endl;

    // Data (Database, Time period, filters)
    hTemplateFile << "<data>" << endl;											// Data marker
    hTemplateFile << "DatabaseName=" << strDatabaseName  << endl;
    hTemplateFile << "DatabaseType=" << pGexMainWindow->pWizardOneQuery->comboBoxDatabaseType->currentText() << endl;
    hTemplateFile << "TimePeriod=" << gexTimePeriodChoices[pGexMainWindow->pWizardOneQuery->comboBoxTimePeriod->currentIndex()] << endl;
    hTemplateFile << "CalendarFrom=" << pGexMainWindow->pWizardOneQuery->FromDate->date().toString(Qt::ISODate) << endl;
    hTemplateFile << "CalendarTo=" << pGexMainWindow->pWizardOneQuery->ToDate->date().toString(Qt::ISODate) << endl;
    QStringList strlFilters = pGexMainWindow->pWizardOneQuery->GetFiltersXmlLines();
    hTemplateFile << "<filters>" << endl;
    for(QStringList::Iterator it = strlFilters.begin(); it != strlFilters.end(); ++it )
        hTemplateFile << *it << endl;
    hTemplateFile << "</filters>" << endl;
    hTemplateFile << "</data>" << endl;											// Data end marker

    // Yield report settings
    hTemplateFile << "<prod_yield>" << endl;
    hTemplateFile << "Aggregate=" << comboSQL_Yield_Xaxis->currentText() << endl;
    QString strYAxis = comboSQL_Yield_Yaxis->currentText().toLower();
    if(strYAxis == "yield only")
    {
        hTemplateFile << "LeftAxis=" << "Serie - Yield" << endl;
        hTemplateFile << "RightAxis=" << "Disabled (hide axis)" << endl;
    }
    else if(strYAxis == "volume only")
    {
        hTemplateFile << "LeftAxis=" << "Disabled (hide axis)" << endl;
        hTemplateFile << "RightAxis=" << "Total - Volume" << endl;
    }
    else
    {
        hTemplateFile << "LeftAxis=" << "Serie - Yield" << endl;
        hTemplateFile << "RightAxis=" << "Total - Volume" << endl;
    }
    hTemplateFile << "ChartingMode=" << comboSQL_Yield_Style->currentText() << endl;
    if(comboSQL_Yield_Binning->currentIndex() == 0)
        hTemplateFile << "BinList=pass" << endl;									// All pass binnings
    else if(comboSQL_Yield_Binning->currentIndex() == 1)
        hTemplateFile << "BinList=HBIN:" << editSQL_Yield_Bin->text() << endl;		// Binlist
    else
        hTemplateFile << "BinList=SBIN:" << editSQL_Yield_Bin->text() << endl;		// Binlist

    // If 'Split by' enabled, save additional GUI fields
    if(checkSQL_Yield_SplitBy->isChecked())
    {
        hTemplateFile << "SplitField=" << comboSQL_Yield_SplitBy->currentText() << endl;
        hTemplateFile << "LayerMode=" << comboSQL_Yield_MultiChart->currentText() << endl;
    }

    // Write charting style & options
    SqlReport_Options_WriteSettings(hTemplateFile);

    // Close Yield section
    hTemplateFile << "</prod_yield>" << endl;

    // Close section
    hTemplateFile << "</section_enterprise_report>" << endl;
}

///////////////////////////////////////////////////////////
// SQL Report = Production - Consolidated Yield
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Report selected
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_Prod_ConsolidatedYield(void)
{
    // Update button states
    toolButtonSql_UPH->setChecked(false);
    toolButtonSql_Yield->setChecked(false);
    toolButtonSql_WYR_Standard->setChecked(false);
    toolButtonSql_ConsolidatedYield->setChecked(true);
    toolButtonSql_YieldWizard->setChecked(false);
    toolButtonSql_Genealogy_YieldVsYield->setChecked(false);
    toolButtonSql_Genealogy_YieldVsParameter->setChecked(false);

    // Keep track of report selected
    SqlReportSelected(SQL_REPORT_PROD_CONSOLIDATEDYIELD);

    // Ensure Consolidated Yield Gui is up to date
    OnSqlReport_Prod_ConsolidatedYield_UpdateGui();

    // Focus on label field.
    editSQL_ConsolidatedYield_Title->setFocus();
}

///////////////////////////////////////////////////////////
// Make sure all controls are in a correct state
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_Prod_ConsolidatedYield_UpdateGui(void)
{
    if(checkSQL_ConsolidatedYield_SplitBy->isChecked())
    {
        // Split charting enabled
        comboSQL_ConsolidatedYield_SplitBy->show();
        comboSQL_ConsolidatedYield_MultiChart->show();
    }
    else
    {
        // Split charting disabled
        comboSQL_ConsolidatedYield_SplitBy->hide();
        comboSQL_ConsolidatedYield_MultiChart->hide();
    }
}

///////////////////////////////////////////////////////////
// SQL Report selected: Production - Consolidated Yield/Volume
///////////////////////////////////////////////////////////
void GexSettings::SqlReport_Prod_ConsolidatedYield_WriteSettings(QTextStream &hTemplateFile)
{
    // Get database name
    QString strDatabaseName = pGexMainWindow->pWizardOneQuery->comboBoxDatabaseName->currentText();

    // Skip the [Local]/[Server] info, and extract database name.
    if(strDatabaseName.startsWith("[Local]") || strDatabaseName.startsWith("[Server]"))
        strDatabaseName = strDatabaseName.section("]",1).trimmed();

    // Home page
    hTemplateFile << "<section_home_page>" << endl;								// Start template marker
    hTemplateFile<< "HomeText="<< ReportOptions.GetOption("output", "front_page_text").toString() << endl; //hTemplateFile << "HomeText=" << ReportOptions.strFrontPageText << endl;		// Home page text
    hTemplateFile<< "HomeLogo="<< ReportOptions.GetOption("output", "front_page_image").toString() << endl;	 //hTemplateFile << "HomeLogo=" << ReportOptions.strFrontPageImage << endl;	// Home page logo path
    hTemplateFile << "</section_home_page>" << endl;							// End template marker

    // Write Consolidated Yield report settings
    hTemplateFile << "<section_enterprise_report>" << endl;
    hTemplateFile << "Title=" << editSQL_ConsolidatedYield_Title->text() << endl;
    hTemplateFile << "ReportFunction=Prod_ConsolidatedYield" << endl;

    // Data (Database, Time period, filters)
    hTemplateFile << "<data>" << endl;											// Data marker
    hTemplateFile << "DatabaseName=" << strDatabaseName  << endl;
    hTemplateFile << "DatabaseType=" << pGexMainWindow->pWizardOneQuery->comboBoxDatabaseType->currentText() << endl;
    hTemplateFile << "TimePeriod=" << gexTimePeriodChoices[pGexMainWindow->pWizardOneQuery->comboBoxTimePeriod->currentIndex()] << endl;
    hTemplateFile << "CalendarFrom=" << pGexMainWindow->pWizardOneQuery->FromDate->date().toString(Qt::ISODate) << endl;
    hTemplateFile << "CalendarTo=" << pGexMainWindow->pWizardOneQuery->ToDate->date().toString(Qt::ISODate) << endl;
    QStringList strlFilters = pGexMainWindow->pWizardOneQuery->GetFiltersXmlLines();
    hTemplateFile << "<filters>" << endl;
    for(QStringList::Iterator it = strlFilters.begin(); it != strlFilters.end(); ++it )
        hTemplateFile << *it << endl;
    hTemplateFile << "</filters>" << endl;
    hTemplateFile << "</data>" << endl;											// Data end marker

    // Consolidated Yield report settings
    hTemplateFile << "<prod_consolidatedyield>" << endl;
    hTemplateFile << "Aggregate=" << comboSQL_ConsolidatedYield_Xaxis->currentText() << endl;
    QString strYAxis = comboSQL_ConsolidatedYield_Yaxis->currentText().toLower();
    if(strYAxis == "yield only")
    {
        hTemplateFile << "LeftAxis=" << "Serie - Yield" << endl;
        hTemplateFile << "RightAxis=" << "Disabled (hide axis)" << endl;
    }
    else if(strYAxis == "volume only")
    {
        hTemplateFile << "LeftAxis=" << "Disabled (hide axis)" << endl;
        hTemplateFile << "RightAxis=" << "Total - Volume" << endl;
    }
    else
    {
        hTemplateFile << "LeftAxis=" << "Serie - Yield" << endl;
        hTemplateFile << "RightAxis=" << "Total - Volume" << endl;
    }
    hTemplateFile << "ChartingMode=" << comboSQL_ConsolidatedYield_Style->currentText() << endl;

    // If 'Split by' enabled, save additional GUI fields
    if(checkSQL_ConsolidatedYield_SplitBy->isChecked())
    {
        hTemplateFile << "SplitField=" << comboSQL_ConsolidatedYield_SplitBy->currentText() << endl;
        hTemplateFile << "LayerMode=" << comboSQL_ConsolidatedYield_MultiChart->currentText() << endl;
    }

    // Write charting style & options
    SqlReport_Options_WriteSettings(hTemplateFile);

    // Close Yield section
    hTemplateFile << "</prod_consolidatedyield>" << endl;

    // Close section
    hTemplateFile << "</section_enterprise_report>" << endl;
}

///////////////////////////////////////////////////////////
// SQL Report = WYR - Standard
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Report selected
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_WYR_Standard(void)
{
    // Update button states
    toolButtonSql_UPH->setChecked(false);
    toolButtonSql_Yield->setChecked(false);
    toolButtonSql_WYR_Standard->setChecked(true);
    toolButtonSql_ConsolidatedYield->setChecked(false);
    toolButtonSql_YieldWizard->setChecked(false);
    toolButtonSql_Genealogy_YieldVsYield->setChecked(false);
    toolButtonSql_Genealogy_YieldVsParameter->setChecked(false);

    // Keep track of report selected
    SqlReportSelected(SQL_REPORT_WYR_STANDARD);

    // Ensure WYR Gui is up to date
    OnSqlReport_WYR_Standard_UpdateGui();

    // Focus on label field.
    editSQL_StdWYR_Title->setFocus();
}

///////////////////////////////////////////////////////////
// Make sure all controls are in a correct state
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_WYR_Standard_UpdateGui(void)
{
}

///////////////////////////////////////////////////////////
// Site pickfilter button clicked
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_WYR_Standard_PickFilter_Site(void)
{
    OnSqlReport_WYR_Standard_PickFilter(GEXDB_PLUGIN_DBFIELD_WYR_SITE, comboSQL_StdWYR_Site, GEXDB_PLUGIN_DBFIELD_WYR_YEAR, comboSQL_StdWYR_Year, GEXDB_PLUGIN_DBFIELD_WYR_WEEK, comboSQL_StdWYR_Week);
}

void GexSettings::OnSqlReport_WYR_Standard_PickFilter_Year(void)
{
    OnSqlReport_WYR_Standard_PickFilter(GEXDB_PLUGIN_DBFIELD_WYR_YEAR, comboSQL_StdWYR_Year, GEXDB_PLUGIN_DBFIELD_WYR_SITE, comboSQL_StdWYR_Site, GEXDB_PLUGIN_DBFIELD_WYR_WEEK, comboSQL_StdWYR_Week);
}

///////////////////////////////////////////////////////////
// Week pickfilter button clicked
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_WYR_Standard_PickFilter_Week(void)
{
    OnSqlReport_WYR_Standard_PickFilter(GEXDB_PLUGIN_DBFIELD_WYR_WEEK, comboSQL_StdWYR_Week, GEXDB_PLUGIN_DBFIELD_WYR_SITE, comboSQL_StdWYR_Site, GEXDB_PLUGIN_DBFIELD_WYR_YEAR, comboSQL_StdWYR_Year);
}

///////////////////////////////////////////////////////////
// Pickfilter button clicked
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_WYR_Standard_PickFilter(QString strQueryField, QComboBox *pQueryValue, QString strNarrowFilter_1, QComboBox *pNarrowValue_1, QString strNarrowFilter_2, QComboBox *pNarrowValue_2)
{
    if(pGexMainWindow != NULL)
    {
        // Extract database logical name from current selection.
        // Skip the [Local]/[Server] info.
        QString strDatabaseName = pGexMainWindow->pWizardOneQuery->comboBoxDatabaseName->currentText();
        if(strDatabaseName.startsWith("[Local]") || strDatabaseName.startsWith("[Server]"))
            strDatabaseName = strDatabaseName.section("]",1).trimmed();

        // Get database ptr
        GexDatabaseEntry *pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(strDatabaseName);
        if(pDatabaseEntry && pDatabaseEntry->m_pExternalDatabase)
        {
            // Fill external database query object
            GexDbPlugin_Filter clPluginFilter(this);

            // Set field to query
            clPluginFilter.SetFields(strQueryField);

            // Set filters
            QString strField, strValue, strFilter;
            clPluginFilter.strlQueryFilters.clear();
            // Add filter 1 ?
            strField = strNarrowFilter_1;
            strValue = pNarrowValue_1->currentText();
            if(strValue != "*")
            {
                strFilter = strField;
                strFilter += "=";
                strFilter += strValue;
                clPluginFilter.strlQueryFilters.append(strFilter);
            }
            // Add Week filter ?
            strField = strNarrowFilter_2;
            strValue = pNarrowValue_2->currentText();
            if(strValue != "*")
            {
                strFilter = strField;
                strFilter += "=";
                strFilter += strValue;
                clPluginFilter.strlQueryFilters.append(strFilter);
            }

            // Other filter parameters
            clPluginFilter.bUseTimePeriod = false;
            clPluginFilter.strDataTypeQuery = pGexMainWindow->pWizardOneQuery->comboBoxDatabaseType->currentText();

            QStringList cMatchingValues;
            pDatabaseEntry->m_pExternalDatabase->QueryField(clPluginFilter,cMatchingValues);

            // Fill Filter list with relevant strings
            PickFilterDialog dPickFilter;
            dPickFilter.fillList(cMatchingValues);

            // Prompt dialog box, let user pick Filter string from the list
            if(dPickFilter.exec() != 1)
                return;	// User 'Abort'

            // Save the list selected into the edit field...unless it is already in!
            QString strSelection = dPickFilter.filterList();

            for(int i=0;i< pQueryValue->count(); i++)
            {
                if(strSelection == pQueryValue->itemText(i))
                {
                    // Selection already in combo...simply select it.
                    SetCurrentComboItem(pQueryValue, strSelection);
                    return;
                }
            }
            pQueryValue->addItem(strSelection);
            SetCurrentComboItem(pQueryValue, strSelection);
        }
    }
}

///////////////////////////////////////////////////////////
// Write settings
///////////////////////////////////////////////////////////
void GexSettings::SqlReport_Wyr_Standard_WriteSettings(QTextStream &hTemplateFile)
{
    // Get database name
    QString strDatabaseName = pGexMainWindow->pWizardOneQuery->comboBoxDatabaseName->currentText();

    // Skip the [Local]/[Server] info, and extract database name.
    if(strDatabaseName.startsWith("[Local]") || strDatabaseName.startsWith("[Server]"))
        strDatabaseName = strDatabaseName.section("]",1).trimmed();

    // Home page
    hTemplateFile<< "<section_home_page>" << endl;								// Start template marker
    hTemplateFile<< "HomeText="<< ReportOptions.GetOption("output", "front_page_text").toString() << endl; //hTemplateFile << "HomeText=" << ReportOptions.strFrontPageText << endl;		// Home page text
    hTemplateFile<< "HomeLogo="<< ReportOptions.GetOption("output", "front_page_image").toString() << endl;	//hTemplateFile << "HomeLogo=" << ReportOptions.strFrontPageImage << endl;	// Home page logo path
    hTemplateFile<< "</section_home_page>" << endl;							// End template marker

    // Write Wyr report settings
    hTemplateFile << "<section_enterprise_report>" << endl;
    hTemplateFile << "Title=" << editSQL_StdWYR_Title->text() << endl;
    hTemplateFile << "ReportFunction=Wyr_Standard" << endl;

    // Data (Database, Time period, filters)
    hTemplateFile << "<data>" << endl;											// Data marker
    hTemplateFile << "DatabaseName=" << strDatabaseName  << endl;
    hTemplateFile << "DatabaseType=" << pGexMainWindow->pWizardOneQuery->comboBoxDatabaseType->currentText() << endl;
    hTemplateFile << "TimePeriod=" << gexTimePeriodChoices[pGexMainWindow->pWizardOneQuery->comboBoxTimePeriod->currentIndex()] << endl;
    hTemplateFile << "CalendarFrom=" << pGexMainWindow->pWizardOneQuery->FromDate->date().toString(Qt::ISODate) << endl;
    hTemplateFile << "CalendarTo=" << pGexMainWindow->pWizardOneQuery->ToDate->date().toString(Qt::ISODate) << endl;
    QStringList strlFilters;
    QString		strFilterName, strFilterValue;
    strFilterName = GEXDB_PLUGIN_DBFIELD_WYR_SITE;
    strFilterValue = comboSQL_StdWYR_Site->currentText();
    if(strFilterValue != "*")
        strlFilters += (strFilterName + "=" + strFilterValue);
    strFilterName = GEXDB_PLUGIN_DBFIELD_WYR_YEAR;
    strFilterValue = comboSQL_StdWYR_Year->currentText();
    if(strFilterValue != "*")
        strlFilters += (strFilterName + "=" + strFilterValue);
    strFilterName = GEXDB_PLUGIN_DBFIELD_WYR_WEEK;
    strFilterValue = comboSQL_StdWYR_Week->currentText();
    if(strFilterValue != "*")
        strlFilters += (strFilterName + "=" + strFilterValue);
    hTemplateFile << "<filters>" << endl;
    for(QStringList::Iterator it = strlFilters.begin(); it != strlFilters.end(); ++it )
        hTemplateFile << *it << endl;
    hTemplateFile << "</filters>" << endl;
    hTemplateFile << "</data>" << endl;											// Data end marker

    // WYR report settings
    hTemplateFile << "<wyr_standard>" << endl;

    // Write charting style & options
    SqlReport_Options_WriteSettings(hTemplateFile);

    // Close Yield section
    hTemplateFile << "</wyr_standard>" << endl;

    // Close section
    hTemplateFile << "</section_enterprise_report>" << endl;
}

///////////////////////////////////////////////////////////
// SQL Report = Production - Yield Wizard
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Report selected
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_Prod_YieldWizard(void)
{
    // Update button states
    toolButtonSql_UPH->setChecked(false);
    toolButtonSql_Yield->setChecked(false);
    toolButtonSql_WYR_Standard->setChecked(false);
    toolButtonSql_ConsolidatedYield->setChecked(false);
    toolButtonSql_YieldWizard->setChecked(true);
    toolButtonSql_Genealogy_YieldVsYield->setChecked(false);
    toolButtonSql_Genealogy_YieldVsParameter->setChecked(false);

    // Keep track of report selected
    SqlReportSelected(SQL_REPORT_PROD_YIELDWIZARD);

    // Ensure Yield Wizard Gui is up to date
    OnSqlReport_Prod_YieldWizard_UpdateGui();

    // Focus on label field.
    editSQL_YieldWizard_Title->setFocus();
}

///////////////////////////////////////////////////////////
// Make sure all controls are in a correct state
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_Prod_YieldWizard_UpdateGui(void)
{
    // Hide data source selection for now
    labelSQL_YieldWizard_DataSource->hide();
    comboSQL_YieldWizard_DataSource->hide();

    QString strBackground = comboSQL_YieldWizard_BackgroundStyle->currentText();
    if(strBackground.startsWith("Custom color...", Qt::CaseInsensitive) || strBackground.startsWith("Gradient color...", Qt::CaseInsensitive))
    {
        // Show color button
        buttonSQL_YieldWizard_BackgroundColor->show();
    }
    else
    {
        // Hide color button
        buttonSQL_YieldWizard_BackgroundColor->hide();
    }
}

///////////////////////////////////////////////////////////
// Yield WIZARD: 'NEXT' button clicked
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_Prod_YieldWizard_NextPage(void)
{
    // Get page ID
    int iPage = widgetStackSqlSettings_Wizard->currentIndex();

    // Next page (unless last one)
    iPage += (iPage < 5) ? 1: 0;

    // Show it
    widgetStackSqlSettings_Wizard->setCurrentIndex(iPage);

    // Check if pointed page is the 'Layer split'....if so, need to update the field list based on the Chart-split list
    if(iPage != 4)
        return;

    // Get all fields list, then remove the selected ones found in 'Chart split'
    // Result loaded into Layer split! (avoid splitting charts & lmayers over same field)
    // Add it to the selection list (unless already in list)
    QTreeWidgetItemIterator itChartSplit( listviewSQL_YieldWizard_SelectedChartSplitFields );
    QStringList	strChartSplitList;
    while(itChartSplit.operator *())
    {
        // Update list of Chart-split fields.
        strChartSplitList += itChartSplit.operator *()->text(0);

        // Move to next item in listview
        ++itChartSplit;
    };

    // Fill Layer-split list
    listviewSQL_YieldWizard_AllLayerSplitFields->clear();
    QTreeWidgetItemIterator	itFullList(listviewSQL_YieldWizard_AllChartSplitFields);
    QString					strString;

    while(itFullList.operator *())
    {
        // Get field name
        strString = itFullList.operator *()->text(0);

        // If field not already in chart-split list, then add it to the Layer list.
        if(strChartSplitList.indexOf(strString) == -1)
        {
            QStringList strList(strString);
            // Field not in list, so add it!
            new QTreeWidgetItem(listviewSQL_YieldWizard_AllLayerSplitFields/*,listviewSQL_YieldWizard_AllLayerSplitFields->lastItem()*/,strList);
        }
        // Move to next item in listview
        ++itFullList;
    };
}

///////////////////////////////////////////////////////////
// Yield WIZARD: 'PREVIOUS' button clicked
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_Prod_YieldWizard_PreviousPage(void)
{
    // Get page ID
    int iPage = widgetStackSqlSettings_Wizard->currentIndex();

    // Previous page (unless first one)
    iPage -= (iPage > 0) ? 1: 0;

    // Show it
    widgetStackSqlSettings_Wizard->setCurrentIndex(iPage);
}

///////////////////////////////////////////////////////////
// Yield WIZARD: 'Add Serie' button clicked
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_Prod_YieldWizard_AddSerie(void)
{
    PickSerieDialog cPickSerie(mUi_qtwSQLYieldWizardSeriesView, pGexMainWindow);

    // Set Binning type used (SOFT bin or HARD bin)
    bool	bUseSoftBin = (comboSQL_YieldWizard_BinType->currentIndex() == 1) ? true: false;
    cPickSerie.setBinningType(bUseSoftBin);

    // Show dialog box
    if(cPickSerie.exec() != 1)
        return;

    // Retreive the GUI fields edited
    CPickSerieFields	cFields;
    cPickSerie.getFields(cFields);
    QString strColor;
    strColor.sprintf("0x%02x%02x%02x",cFields.cSerieColor.red(),cFields.cSerieColor.green(),cFields.cSerieColor.blue());
    QString strStyle = cFields.strPlotStyle;
    strStyle += "|" + strColor;
    strStyle += "|" + cFields.strDataLabels;
    strStyle += "|" + cFields.strLineStyle;
    strStyle += "|" + cFields.strLineSpots;
    strStyle += "|" + cFields.strLineProperty;

    QString strYieldThreshold;
    if(cFields.bBinningParetoOverException)
        strYieldThreshold.sprintf("%s %.2f %%",cFields.strAlarmType.toLatin1().constData(),cFields.fAlarmLimit);
    else
        strYieldThreshold = "-";

    int nTopLevelItemCount = mUi_qtwSQLYieldWizardSeriesView->topLevelItemCount();
    QTreeWidgetItem* qtwiPtrLastItem = mUi_qtwSQLYieldWizardSeriesView->topLevelItem(nTopLevelItemCount-1);
    QTreeWidgetItem* qtwiPtrNewItem = new QTreeWidgetItem( mUi_qtwSQLYieldWizardSeriesView, qtwiPtrLastItem);

    qtwiPtrNewItem->setText(0, cFields.strTitle);						// 'Serie' column
    qtwiPtrNewItem->setText(1, cFields.strBinType);						// 'Bin type' column
    qtwiPtrNewItem->setText(2, cFields.strBinList);						// 'Bin list' column
    qtwiPtrNewItem->setText(3, QString::number(cFields.bPlotSerie));	// 'Plot' column
    qtwiPtrNewItem->setText(4, strStyle);								// 'Style' column
    qtwiPtrNewItem->setText(5, strYieldThreshold);						// 'Bin pareto' column
    qtwiPtrNewItem->setText(6, cFields.strTableData);					// 'Table data' column

    // resize gui columns to content
    int nColumnCount = mUi_qtwSQLYieldWizardSeriesView->columnCount();
    for( int ii=0; ii<nColumnCount; ii++)
        mUi_qtwSQLYieldWizardSeriesView->resizeColumnToContents(ii);
}

///////////////////////////////////////////////////////////
// Yield WIZARD: 'Remove Serie' button clicked
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_Prod_YieldWizard_RemoveSerie(void)
{
    // Get handle to serie entry
    QTreeWidgetItem *qtwiPtrItemToDelete = mUi_qtwSQLYieldWizardSeriesView->currentItem();

    if(qtwiPtrItemToDelete == NULL)
        return;

    // Remove item
    int nCurrentItemPos = mUi_qtwSQLYieldWizardSeriesView->indexOfTopLevelItem(qtwiPtrItemToDelete);
    mUi_qtwSQLYieldWizardSeriesView->takeTopLevelItem(nCurrentItemPos);
    delete qtwiPtrItemToDelete;
    qtwiPtrItemToDelete=0;
}

///////////////////////////////////////////////////////////
// Yield WIZARD: 'Clear all Series' button clicked
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_Prod_YieldWizard_ClearAllSeries(void)
{
    // Ask 'Delete' confirmation...
    QString strMessage = "Confirm to Delete All series?";
    bool lOk;
    GS::Gex::Message::request("Delete series", strMessage, lOk);
    if (! lOk)
    {
        return;
    }

    // Empty list view.
    mUi_qtwSQLYieldWizardSeriesView->clear();
}

///////////////////////////////////////////////////////////
// Yield WIZARD: 'Edit Serie' (line double-clicked)
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_Prod_YieldWizard_EditSerie(void)
{
    // Get handle to serie entry
    // QTreeWidgetItem *ptItem = listviewSQL_YieldWizard_Series->currentItem();
    QTreeWidgetItem	*ptItem = mUi_qtwSQLYieldWizardSeriesView->currentItem();
    if(ptItem == NULL)
        return;

    // Get GUI info, load info structure for editing
    CPickSerieFields	cFields;
    QString				strColor, strStyle, strYieldThreshold;

    cFields.strTitle		= ptItem->text(0);
    cFields.strBinType		= ptItem->text(1);
    cFields.strBinList		= ptItem->text(2);
    cFields.bPlotSerie		= (bool) (ptItem->text(3)).toInt();

    strStyle				= ptItem->text(4);
    cFields.strPlotStyle	= strStyle.section("|", 0, 0);
    strColor				= strStyle.section("|", 1, 1).mid(2);	// Skip '0x' string so to read the RGB color
    int	ir,ig,ib;
    sscanf(strColor.toLatin1().constData(),"%02x%02x%02x",&ir,&ig,&ib);
    cFields.cSerieColor		= QColor(ir,ig,ib);
    cFields.strDataLabels	= strStyle.section("|", 2, 2);
    cFields.strLineStyle	= strStyle.section("|", 3, 3);
    cFields.strLineSpots	= strStyle.section("|", 4, 4);
    cFields.strLineProperty	= strStyle.section("|", 5, 5);

    strYieldThreshold = ptItem->text(5);
    if(strYieldThreshold.startsWith("-"))
        cFields.bBinningParetoOverException = false;
    else
    {
        cFields.bBinningParetoOverException = true;
        cFields.strAlarmType = strYieldThreshold;
        // Extract string 'Yield <=' or 'Yield >='
        cFields.strAlarmType.truncate(strYieldThreshold.indexOf("=")+1);

        // Extract yield limit value
        strYieldThreshold = strYieldThreshold.section("=",1);
        if(strYieldThreshold.isEmpty())
            strYieldThreshold = "-1";
        sscanf(strYieldThreshold.toLatin1().constData(),"%f",&cFields.fAlarmLimit);
    }

    cFields.strTableData = ptItem->text(6);

    // Load dialog box with fields to edit
    PickSerieDialog cPickSerie(NULL, pGexMainWindow);
    cPickSerie.setFields(cFields);

    // Set Binning type used (SOFT bin or HARD bin)
    bool	bUseSoftBin = (comboSQL_YieldWizard_BinType->currentIndex() == 1) ? true: false;
    cPickSerie.setBinningType(bUseSoftBin);

    // Show dialog box
    if(cPickSerie.exec() != 1)
        return;

    // Edits done...update GUI!
    cPickSerie.getFields(cFields);
    ptItem->setText(0,cFields.strTitle);
    ptItem->setText(1,cFields.strBinType);
    ptItem->setText(2,cFields.strBinList);
    ptItem->setText(3,QString::number(cFields.bPlotSerie));
    strColor.sprintf("0x%02x%02x%02x",cFields.cSerieColor.red(),cFields.cSerieColor.green(),cFields.cSerieColor.blue());
    strStyle = cFields.strPlotStyle;
    strStyle += "|" + strColor;
    strStyle += "|" + cFields.strDataLabels;
    strStyle += "|" + cFields.strLineStyle;
    strStyle += "|" + cFields.strLineSpots;
    strStyle += "|" + cFields.strLineProperty;
    ptItem->setText(4,strStyle);

    // yield threshold
    if(cFields.bBinningParetoOverException)
        strYieldThreshold.sprintf("%s %.2f %%",cFields.strAlarmType.toLatin1().constData(),cFields.fAlarmLimit);
    else
        strYieldThreshold = "-";
    ptItem->setText(5,strYieldThreshold);

    ptItem->setText(6,cFields.strTableData);
}

///////////////////////////////////////////////////////////
// Yield WIZARD: Add Chart split field
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_Prod_YieldWizard_AddChartSplit(void)
{
    // Get handle to field selected
    QTreeWidgetItem *ptItem = listviewSQL_YieldWizard_AllChartSplitFields->currentItem();
    if(ptItem == NULL)
        return;

    // Get string selected (field name)
    QString strString = ptItem->text(0);

    // Add it to the selection list (unless already in list)
    QTreeWidgetItemIterator it( listviewSQL_YieldWizard_SelectedChartSplitFields );

    while ( it.operator *() )
    {
        // Field already in list?....If so quietly ignore this duplicate selection.
        if(strString == it.operator *()->text(0))
            return;

        // Move to next item in listview
        ++it;
    };

    QStringList strList(strString);
    // Field not in list, so add it!
    new QTreeWidgetItem(listviewSQL_YieldWizard_SelectedChartSplitFields/*,listviewSQL_YieldWizard_SelectedChartSplitFields->lastItem()*/,strList);

    // Ensure this field is NOT selected (to avoid to remove it if 'Remove' button hit)
    listviewSQL_YieldWizard_SelectedChartSplitFields->clearSelection();
}

///////////////////////////////////////////////////////////
// Yield WIZARD: Remove Chart split field
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_Prod_YieldWizard_RemoveChartSplit(void)
{
    // Ensure an item is selected in the list
    if(listviewSQL_YieldWizard_SelectedChartSplitFields->selectedItems().count() == 0)
        return;

    // Get handle to field selected
    QTreeWidgetItem *ptItem = listviewSQL_YieldWizard_SelectedChartSplitFields->currentItem();
    if(ptItem == NULL)
        return;

    // Remove item
    delete ptItem;
}

///////////////////////////////////////////////////////////
// Yield WIZARD: Remove ALL Chart split fields selected
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_Prod_YieldWizard_RemoveAllChartSplit(void)
{
    listviewSQL_YieldWizard_SelectedChartSplitFields->clear();
}

///////////////////////////////////////////////////////////
// Yield WIZARD: Add Layer split field
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_Prod_YieldWizard_AddLayerSplit(void)
{
    // Get handle to field selected
    QTreeWidgetItem *ptItem = listviewSQL_YieldWizard_AllLayerSplitFields->currentItem();
    if(ptItem == NULL)
        return;

    // Get string selected (field name)
    QString strString = ptItem->text(0);

    // Add it to the selection list (unless already in list)
    QTreeWidgetItemIterator it(listviewSQL_YieldWizard_SelectedLayerSplitFields);

    while ( it.operator *() )
    {
        // Field already in list?....If so quietly ignore this duplicate selection.
        if(strString == it.operator *()->text(0))
            return;

        // Move to next item in listview
        ++it;
    };

    // Field not in list, so add it!
    QStringList strList(strString);
    new QTreeWidgetItem(listviewSQL_YieldWizard_SelectedLayerSplitFields/*,listviewSQL_YieldWizard_SelectedLayerSplitFields->lastItem()*/,strList);

    // Ensure this field is NOT selected (to avoid to remove it if 'Remove' button hit)
    listviewSQL_YieldWizard_SelectedLayerSplitFields->clearSelection();
}

///////////////////////////////////////////////////////////
// Yield WIZARD: Remove Layer split field
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_Prod_YieldWizard_RemoveLayerSplit(void)
{
    // Ensure an item is selected in the list
    if(listviewSQL_YieldWizard_SelectedLayerSplitFields->selectedItems().count() == 0)
        return;

    // Get handle to field selected
    QTreeWidgetItem *ptItem = listviewSQL_YieldWizard_SelectedLayerSplitFields->currentItem();
    if(ptItem == NULL)
        return;

    // Remove item
    delete ptItem;
}

///////////////////////////////////////////////////////////
// Yield WIZARD: Remove ALL Layer split fields selected
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_Prod_YieldWizard_RemoveAllLayerSplit(void)
{
    listviewSQL_YieldWizard_SelectedLayerSplitFields->clear();
}

///////////////////////////////////////////////////////////
// SQL Report selected: Production - Yield Wizard report
///////////////////////////////////////////////////////////
void GexSettings::SqlReport_Prod_YieldWizard_WriteSettings(QTextStream &hTemplateFile)
{
    // Get database name
    QString strDatabaseName = pGexMainWindow->pWizardOneQuery->comboBoxDatabaseName->currentText();

    // Skip the [Local]/[Server] info, and extract database name.
    if(strDatabaseName.startsWith("[Local]") || strDatabaseName.startsWith("[Server]"))
        strDatabaseName = strDatabaseName.section("]",1).trimmed();

    // Home page
    hTemplateFile << "<section_home_page>" << endl;	// Start template marker
    hTemplateFile<<"HomeText="<< ReportOptions.GetOption("output", "front_page_text").toString() << endl; //hTemplateFile << "HomeText=" << ReportOptions.strFrontPageText << endl;		// Home page text
    hTemplateFile<<"HomeLogo="<< ReportOptions.GetOption("output", "front_page_image").toString() << endl; //hTemplateFile << "HomeLogo=" << ReportOptions.strFrontPageImage << endl;	// Home page logo path
    hTemplateFile << "</section_home_page>" << endl; // End template marker

    // Write Yield/Volume report settings
    hTemplateFile << "<section_enterprise_report>" << endl;
    hTemplateFile << "Title=" << editSQL_YieldWizard_Title->text() << endl;
    hTemplateFile << "ReportFunction=Prod_AdvancedYield" << endl;

    // Data (Database, Time period, filters)
    hTemplateFile << "<data>" << endl;											// Data marker
    hTemplateFile << "DatabaseName=" << strDatabaseName  << endl;
    hTemplateFile << "DatabaseType=" << pGexMainWindow->pWizardOneQuery->comboBoxDatabaseType->currentText() << endl;
    hTemplateFile << "TimePeriod=" << gexTimePeriodChoices[pGexMainWindow->pWizardOneQuery->comboBoxTimePeriod->currentIndex()] << endl;
    hTemplateFile << "CalendarFrom=" << pGexMainWindow->pWizardOneQuery->FromDate->date().toString(Qt::ISODate) << endl;
    hTemplateFile << "CalendarTo=" << pGexMainWindow->pWizardOneQuery->ToDate->date().toString(Qt::ISODate) << endl;
    QStringList strlFilters = pGexMainWindow->pWizardOneQuery->GetFiltersXmlLines();
    hTemplateFile << "<filters>" << endl;
    for(QStringList::Iterator it = strlFilters.begin(); it != strlFilters.end(); ++it )
        hTemplateFile << *it << endl;
    hTemplateFile << "</filters>" << endl;
    hTemplateFile << "</data>" << endl;											// Data end marker

    // Yield report settings
    hTemplateFile << "<prod_advancedyield>" << endl;
    hTemplateFile << "Aggregate=" << comboSQL_YieldWizard_Xaxis->currentText() << endl;
    hTemplateFile << "LeftAxis=" << comboSQL_YieldWizard_LeftAxis->currentText() << endl;
    hTemplateFile << "RightAxis=" << comboSQL_YieldWizard_RightAxis->currentText() << endl;
    hTemplateFile << "DataSource=" << comboSQL_YieldWizard_DataSource->currentText() << endl;
    hTemplateFile << "ColorMgt=" << comboSQL_YieldWizard_ColorMgt->currentText() << endl;
    // Dump Graph splits
    QTreeWidgetItemIterator itGraphSplits(listviewSQL_YieldWizard_SelectedChartSplitFields);
    while(itGraphSplits.operator *())
    {
        // Save field name
        hTemplateFile << "GraphSplit=" << itGraphSplits.operator *()->text(0);
        hTemplateFile << endl;
        // Move to next item in listview
        ++itGraphSplits;
    };
    // Dump Layers splits
    QTreeWidgetItemIterator itLayerSplits(listviewSQL_YieldWizard_SelectedLayerSplitFields );
    while(itLayerSplits.operator *())
    {
        // Save field name
        hTemplateFile << "LayerSplit=" << itLayerSplits.operator *()->text(0);
        hTemplateFile << endl;
        // Move to next item in listview
        ++itLayerSplits;
    };
    hTemplateFile << "UseSoftBin=";
    if(comboSQL_YieldWizard_BinType->currentIndex() == 1)
        hTemplateFile << "1" << endl;	// SOFT bin used
    else
        hTemplateFile << "0" << endl;	// HARD bin used

    // Dump series
    QTreeWidgetItemIterator	qtwiiSeriesIterator(mUi_qtwSQLYieldWizardSeriesView);
    QString					strBinType, strColor, strStyle;
    int						nValue;
    QColor					cColor;

    // while(itSeries.operator *())
    while(*qtwiiSeriesIterator)
    {
        strBinType	= (*qtwiiSeriesIterator)->text(1).toLower();
        strStyle	= (*qtwiiSeriesIterator)->text(4);


        hTemplateFile << "<serie>" << endl;
        hTemplateFile << "Name=" << (*qtwiiSeriesIterator)->text(0) << endl;
        hTemplateFile << "Binnings=";
        if((strBinType.indexOf("pass") == -1) && (strBinType.indexOf("fail") == -1) &&
                (strBinType.indexOf("all") == -1))
        {
            hTemplateFile << (*qtwiiSeriesIterator)->text(2) << endl;					// Serie Bin list: list of specific bins
        }
        else
        {
            hTemplateFile << (*qtwiiSeriesIterator)->text(1) << endl;					// Serie Bin list: "pass only","fail only", or "all bins"
        }
        hTemplateFile << "PlotSerie=" << (*qtwiiSeriesIterator)->text(3) << endl;		// 1=plot serie, 0 = do not plot serie
        hTemplateFile << "YieldException=" << (*qtwiiSeriesIterator)->text(5) << endl;	// Yield exception threshold
        hTemplateFile << "TableData=" << (*qtwiiSeriesIterator)->text(6) << endl;		// Serie table data type (Yield, Volume, Yield & volume,none...)
        hTemplateFile << "ChartingMode=" << strStyle.section("|", 0, 0) << endl;	// Serie plot style (Bars or Lines)
        hTemplateFile << "Color=" << strStyle.section("|", 1, 1) << endl;			// Serie color RGB Hexa. string: 0xRRGGBB
        hTemplateFile << "DataLabels=" << strStyle.section("|", 2, 2) << endl;		// Label position (none, top, left...)
        hTemplateFile << "LineStyle=" << strStyle.section("|", 3, 3) << endl;		// Line style (line, spline...)
        hTemplateFile << "LineSpots=" << strStyle.section("|", 4, 4) << endl;		// Spot shape (none, square...)
        hTemplateFile << "LineProperty=" << strStyle.section("|", 5, 5) << endl;	// Line property (solid, dashed,...)
        hTemplateFile << "</serie>" << endl;

        // Move to next item in listview
        ++qtwiiSeriesIterator;
    };

    // Global style
    hTemplateFile << "<style_global>" << endl;
    hTemplateFile << "BarStyle=" << comboSQL_YieldWizard_BarStyle->currentText() << endl;				// Bar style (2D, 3D...)
    nValue = (checkSQL_YieldWizard_GradientBarColor->isChecked()) ? 1: 0;
    hTemplateFile << "BarColorGradient=" << QString::number(nValue) << endl;							// Gradient bar color
    nValue = (checkSQL_YieldWizard_SemiTransparentBar->isChecked()) ? 1: 0;
    hTemplateFile << "BarColorSemiTransparent=" << QString::number(nValue) << endl;						// Semi transparent bar color
    nValue = spinSQL_YieldWizard_OverlappingRatio->value();
    hTemplateFile << "BarOverlappingRatio=" << QString::number(nValue) << endl;							// Overlapping ratio between bars
    hTemplateFile << "BackgroundStyle=" << comboSQL_YieldWizard_BackgroundStyle->currentText() << endl;	// Bkg style (none; b&w, color...)
    cColor = buttonSQL_YieldWizard_BackgroundColor->activeColor();
    strColor.sprintf("0x%02x%02x%02x",cColor.red(),cColor.green(),cColor.blue());
    hTemplateFile << "BackgroundColor=" << strColor	<< endl;											//  Bkg color RGB Hexa. string: 0xRRGGBB
    hTemplateFile << "LayerPolicy=" << comboSQL_YieldWizard_LayerPolicy->currentText() << endl;			// Layer policy (dark to light, light to dark...)
    hTemplateFile << "</style_global>" << endl;

    // Volume style
    hTemplateFile << "<style_volume>" << endl;
    hTemplateFile << "ChartingMode=" << comboSQL_YieldWizard_VolumeStyle->currentText() << endl;		// Volume style (bars, lines...)
    hTemplateFile << "DataLabels=" << comboSQL_YieldWizard_VolumeDataLabels->currentText() << endl;		// Volume data labels (none, top, bottom...)
    hTemplateFile << "LineStyle=" << comboSQL_YieldWizard_VolumeLineStyle->currentText() << endl;		// Line style (line, spline, ...)
    hTemplateFile << "LineSpots=" << comboSQL_YieldWizard_VolumeLineSpots->currentText() << endl;		// Line spots (none, square, ...)
    hTemplateFile << "LineProperty=" << comboSQL_YieldWizard_VolumeLineProperty->currentText() << endl;	// Line property (solid, dashed, ...)
    cColor = buttonSQL_YieldWizard_VolumeColor->activeColor();
    strColor.sprintf("0x%02x%02x%02x",cColor.red(),cColor.green(),cColor.blue());
    hTemplateFile << "Color=" << strColor	<< endl;													//  Volume color RGB Hexa. string: 0xRRGGBB
    hTemplateFile << "</style_volume>" << endl;

    // BinPareto style
    hTemplateFile << "<style_binpareto>" << endl;
    hTemplateFile << "Title=" << editSQL_YieldWizard_BinningParetoTitle->text() << endl;					// Title for Binning Pareto's
    hTemplateFile << "Binnings=" << comboSQL_YieldWizard_BinningParetoBinnings->currentText() << endl;		// Binnings to use for pareto
    nValue = spinSQL_YieldWizard_BinningParetoNbCat->value();
    hTemplateFile << "MaxCategories=" << QString::number(nValue) << endl;									// Max binning categories for pareto
    hTemplateFile << "ChartingMode=" << comboSQL_YieldWizard_BinningParetoStyle->currentText() << endl;		// Binning Pareto style (bars, lines...)
    hTemplateFile << "DataLabels=" << comboSQL_YieldWizard_BinningParetoDataLabels->currentText() << endl;	// Binning Pareto data labels (none, top, bottom...)
    hTemplateFile << "LineStyle=" << comboSQL_YieldWizard_BinningParetoLineStyle->currentText() << endl;	// Line style (line, spline, ...)
    hTemplateFile << "LineSpots=" << comboSQL_YieldWizard_BinningParetoLineSpots->currentText() << endl;	// Line spots (none, square, ...)
    hTemplateFile << "LineProperty=" << comboSQL_YieldWizard_BinningParetoLineProperty->currentText() << endl;// Line property (solid, dashed, ...)
    cColor = buttonSQL_YieldWizard_BinningParetoColor->activeColor();
    strColor.sprintf("0x%02x%02x%02x",cColor.red(),cColor.green(),cColor.blue());
    hTemplateFile << "Color=" << strColor	<< endl;														//  Binning Pareto color RGB Hexa. string: 0xRRGGBB
    hTemplateFile << "</style_binpareto>" << endl;

    // Advanced options
    hTemplateFile << "<advanced>" << endl;
    nValue = (checkSQL_YieldWizard_DumpRawData->isChecked()) ? 1: 0;
    hTemplateFile << "DumpRawData=" << QString::number(nValue) << endl;
    hTemplateFile << "EtWt_MaverickWafer_AlarmType=" << comboSQL_YieldWizard_ETWT_MavWafer_AlarmType->currentText() << endl;
    nValue = spinSQL_YieldWizard_ETWT_MavWafer_AlarmCount->value();
    hTemplateFile << "EtWt_MaverickWafer_AlarmCount=" << QString::number(nValue) << endl;
    nValue = spinSQL_YieldWizard_ETWT_MavLot_WaferCount->value();
    hTemplateFile << "EtWt_MaverickLot_WaferCount=" << QString::number(nValue) << endl;
    nValue = (checkSQL_YieldWizard_BinParetoOverMaverick->isChecked()) ? 1: 0;
    hTemplateFile << "ShowBinParetoForMaverick=" << QString::number(nValue) << endl;
    hTemplateFile << "</advanced>" << endl;

    // Close Yield Wizard section
    hTemplateFile << "</prod_advancedyield>" << endl;

    // Close section
    hTemplateFile << "</section_enterprise_report>" << endl;
}

///////////////////////////////////////////////////////////
// SQL Report = Genealogy - Yield vs. Yield
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Report selected
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_Genealogy_YieldVsYield(void)
{
    // Update button states
    toolButtonSql_UPH->setChecked(false);
    toolButtonSql_Yield->setChecked(false);
    toolButtonSql_WYR_Standard->setChecked(false);
    toolButtonSql_ConsolidatedYield->setChecked(false);
    toolButtonSql_YieldWizard->setChecked(false);
    toolButtonSql_Genealogy_YieldVsYield->setChecked(true);
    toolButtonSql_Genealogy_YieldVsParameter->setChecked(false);

    // Keep track of report selected
    SqlReportSelected(SQL_REPORT_GENEALOGY_YIELDVSYIELD);

    // Ensure Gui is up to date
    OnSqlReport_Genealogy_YieldVsYield_UpdateGui();

    // Focus on label field.
    editSQL_Genealogy_YieldVsYield_Title->setFocus();
}

///////////////////////////////////////////////////////////
// Make sure all controls are in a correct state
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_Genealogy_YieldVsYield_UpdateGui(void)
{
    // Hide/Show controls
    if(editSQL_Genealogy_YieldVsYield_Product->text().isEmpty())
    {
        groupSQL_Genealogy_YieldVsYield_Xaxis->setEnabled(false);
        groupSQL_Genealogy_YieldVsYield_Yaxis->setEnabled(false);
        groupSQL_Genealogy_YieldVsYield_Options->setEnabled(false);
        return;
    }

    // If product selected, enable all groups
    groupSQL_Genealogy_YieldVsYield_Xaxis->setEnabled(true);
    groupSQL_Genealogy_YieldVsYield_Yaxis->setEnabled(true);
    groupSQL_Genealogy_YieldVsYield_Options->setEnabled(true);

    // Binning buttons/controls
    switch(comboSQL_Genealogy_YieldVsYield_Binning_Xaxis->currentIndex())
    {
        case 0:	// PASS only bins
        case 1:	// FAIL only bins
            editSQL_Genealogy_YieldVsYield_Binlist_Xaxis->hide();
            buttonSQL_Genealogy_YieldVsYield_PickBinlist_Xaxis->hide();
            break;
        case 2:	// Specific bins...
            editSQL_Genealogy_YieldVsYield_Binlist_Xaxis->show();
            buttonSQL_Genealogy_YieldVsYield_PickBinlist_Xaxis->show();
            // Set focus to edit field.
            editSQL_Genealogy_YieldVsYield_Binlist_Xaxis->setFocus();
            break;
    }
    switch(comboSQL_Genealogy_YieldVsYield_Binning_Yaxis->currentIndex())
    {
        case 0:	// PASS only bins
        case 1:	// FAIL only bins
            editSQL_Genealogy_YieldVsYield_Binlist_Yaxis->hide();
            buttonSQL_Genealogy_YieldVsYield_PickBinlist_Yaxis->hide();
            break;
        case 2:	// Specific bins...
            editSQL_Genealogy_YieldVsYield_Binlist_Yaxis->show();
            buttonSQL_Genealogy_YieldVsYield_PickBinlist_Yaxis->show();
            // Set focus to edit field.
            editSQL_Genealogy_YieldVsYield_Binlist_Yaxis->setFocus();
            break;
    }

    // Disable granularity radios if Final Test selected for one axis
    frameSQL_Genealogy_YieldVsYield_Granularity->setEnabled(false);
    radioSQL_Genealogy_YieldVsYield_Granularity_Lot->setChecked(true);
    if(pGexMainWindow && pGexMainWindow->pWizardOneQuery)
    {
        QString				strDatabaseName = pGexMainWindow->pWizardOneQuery->comboBoxDatabaseName->currentText();
        if(strDatabaseName.startsWith("[Local]") || strDatabaseName.startsWith("[Server]"))
            strDatabaseName = strDatabaseName.section("]",1).trimmed();
        GexDatabaseEntry	*pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(strDatabaseName);
        if(pDatabaseEntry && pDatabaseEntry->m_pExternalDatabase)
        {
            if(	!pDatabaseEntry->m_pExternalDatabase->IsTestingStage_FinalTest(comboSQL_Genealogy_YieldVsYield_TestingStage_Xaxis->currentText()) &&
                !pDatabaseEntry->m_pExternalDatabase->IsTestingStage_FinalTest(comboSQL_Genealogy_YieldVsYield_TestingStage_Yaxis->currentText()))
                frameSQL_Genealogy_YieldVsYield_Granularity->setEnabled(true);
        }
    }
}

///////////////////////////////////////////////////////////
// Open dialog with possible bins for X axis
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_Genealogy_YieldVsYield_OnPickBinlist_Xaxis()
{
    QString		strBinning, strDataType, strProduct;
    QStringList strlProducts = editSQL_Genealogy_YieldVsYield_Product->text().split(',', QString::SkipEmptyParts);

    strDataType = comboSQL_Genealogy_YieldVsYield_TestingStage_Xaxis->currentText();
    strProduct = strlProducts[m_strlValidTestingStages.indexOf(strDataType)];
    strBinning = pGexMainWindow->pWizardOneQuery->SqlPickBinningList(true, strDataType, strProduct, true);

    if(!strBinning.isEmpty())
        editSQL_Genealogy_YieldVsYield_Binlist_Xaxis->setText(strBinning);
}

///////////////////////////////////////////////////////////
// Open dialog with possible bins for Y axis
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_Genealogy_YieldVsYield_OnPickBinlist_Yaxis()
{
    QString		strBinning, strDataType, strProduct;
    QStringList strlProducts = editSQL_Genealogy_YieldVsYield_Product->text().split(',', QString::SkipEmptyParts);

    strDataType = comboSQL_Genealogy_YieldVsYield_TestingStage_Yaxis->currentText();
    strProduct = strlProducts[m_strlValidTestingStages.indexOf(strDataType)];
    strBinning = pGexMainWindow->pWizardOneQuery->SqlPickBinningList(true, strDataType, strProduct, true);

    if(!strBinning.isEmpty())
        editSQL_Genealogy_YieldVsYield_Binlist_Yaxis->setText(strBinning);
}

///////////////////////////////////////////////////////////
// Open dialog with possible products
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_Genealogy_YieldVsYield_OnPickProduct()
{
    QStringList strlSelection, strlProducts, strlTestingStages;

    pGexMainWindow->pWizardOneQuery->SqlPickProductList_Genealogy(strlSelection, true, false);

    if(strlSelection.count() < 2)
        return;

    // Display selected product
    editSQL_Genealogy_YieldVsYield_Product->setText(strlSelection[1]);

    // Update available testingStages
    m_strlValidTestingStages.clear();
    strlTestingStages = strlSelection[0].split(',', QString::KeepEmptyParts);
    strlProducts = strlSelection[1].split(',', QString::KeepEmptyParts);
    for(int nIndex = 0; nIndex < strlTestingStages.count(); nIndex++)
    {
        if(!(strlProducts[nIndex].isEmpty()))
            m_strlValidTestingStages.append(strlTestingStages[nIndex]);
    }

    QString	strSelected;
    strSelected = comboSQL_Genealogy_YieldVsYield_TestingStage_Xaxis->currentText();
    comboSQL_Genealogy_YieldVsYield_TestingStage_Xaxis->clear();
    comboSQL_Genealogy_YieldVsYield_TestingStage_Xaxis->addItems(m_strlValidTestingStages);
    SetCurrentComboItem(comboSQL_Genealogy_YieldVsYield_TestingStage_Xaxis, strSelected);
    strSelected = comboSQL_Genealogy_YieldVsYield_TestingStage_Yaxis->currentText();
    comboSQL_Genealogy_YieldVsYield_TestingStage_Yaxis->clear();
    comboSQL_Genealogy_YieldVsYield_TestingStage_Yaxis->addItems(m_strlValidTestingStages);
    SetCurrentComboItem(comboSQL_Genealogy_YieldVsYield_TestingStage_Yaxis, strSelected);

    OnSqlReport_Genealogy_YieldVsYield_UpdateGui();
}

///////////////////////////////////////////////////////////
// Write settings
///////////////////////////////////////////////////////////
void GexSettings::SqlReport_Genealogy_YieldVsYield_WriteSettings(QTextStream &hTemplateFile)
{
    // Get database name
    QString strDatabaseName = pGexMainWindow->pWizardOneQuery->comboBoxDatabaseName->currentText();

    // Skip the [Local]/[Server] info, and extract database name.
    if(strDatabaseName.startsWith("[Local]") || strDatabaseName.startsWith("[Server]"))
        strDatabaseName = strDatabaseName.section("]",1).trimmed();

    // Home page
    hTemplateFile<< "<section_home_page>" << endl;								// Start template marker
    hTemplateFile<< "HomeText=" << ReportOptions.GetOption("output", "front_page_text").toString() << endl; //hTemplateFile<< "HomeText=" << ReportOptions.strFrontPageText << endl;		// Home page text
    hTemplateFile<< "HomeLogo=" << ReportOptions.GetOption("output", "front_page_image").toString() << endl;	//hTemplateFile << "HomeLogo=" << ReportOptions.strFrontPageImage << endl;	// Home page logo path
    hTemplateFile<< "</section_home_page>" << endl;							// End template marker

    // Write Yield/Volume report settings
    hTemplateFile << "<section_enterprise_report>" << endl;
    hTemplateFile << "Title=" << editSQL_Genealogy_YieldVsYield_Title->text() << endl;
    hTemplateFile << "ReportFunction=Genealogy_YieldVsYield" << endl;

    // Data (Database, Time period, filters)
    hTemplateFile << "<data>" << endl;											// Data marker
    hTemplateFile << "DatabaseName=" << strDatabaseName  << endl;
    hTemplateFile << "DatabaseType=" << pGexMainWindow->pWizardOneQuery->comboBoxDatabaseType->currentText() << endl;
    hTemplateFile << "TimePeriod=" << gexTimePeriodChoices[pGexMainWindow->pWizardOneQuery->comboBoxTimePeriod->currentIndex()] << endl;
    hTemplateFile << "CalendarFrom=" << pGexMainWindow->pWizardOneQuery->FromDate->date().toString(Qt::ISODate) << endl;
    hTemplateFile << "CalendarTo=" << pGexMainWindow->pWizardOneQuery->ToDate->date().toString(Qt::ISODate) << endl;
    QStringList strlFilters = pGexMainWindow->pWizardOneQuery->GetFiltersXmlLines();
    hTemplateFile << "<filters>" << endl;
    for(QStringList::Iterator it = strlFilters.begin(); it != strlFilters.end(); ++it )
        hTemplateFile << *it << endl;
    hTemplateFile << "</filters>" << endl;
    hTemplateFile << "</data>" << endl;											// Data end marker

    // Genealogy report settings
    QString strTestingStage, strBinType;
    QStringList strlProducts = editSQL_Genealogy_YieldVsYield_Product->text().split(',', QString::SkipEmptyParts);

    hTemplateFile << "<genealogy_yieldvsyield>" << endl;
    hTemplateFile << "Product=" << editSQL_Genealogy_YieldVsYield_Product->text() << endl;
    strTestingStage = comboSQL_Genealogy_YieldVsYield_TestingStage_Xaxis->currentText();
    hTemplateFile << "XAxis_Product=" << strlProducts[m_strlValidTestingStages.indexOf(strTestingStage)] << endl;
    hTemplateFile << "XAxis_TestingStage=" << strTestingStage << endl;
    strBinType = comboSQL_Genealogy_YieldVsYield_Binning_Xaxis->currentText().toLower();
    hTemplateFile << "XAxis_Binnings=";
    if((strBinType.indexOf("pass") == -1) && (strBinType.indexOf("fail") == -1))
        hTemplateFile << editSQL_Genealogy_YieldVsYield_Binlist_Xaxis->text() << endl;			// Bin list: list of specific bins
    else
        hTemplateFile << comboSQL_Genealogy_YieldVsYield_Binning_Xaxis->currentText() << endl;	// Bin list: "pass only","fail only"
    strTestingStage = comboSQL_Genealogy_YieldVsYield_TestingStage_Yaxis->currentText();
    hTemplateFile << "YAxis_Product=" << strlProducts[m_strlValidTestingStages.indexOf(strTestingStage)] << endl;
    hTemplateFile << "YAxis_TestingStage=" << strTestingStage << endl;
    strBinType = comboSQL_Genealogy_YieldVsYield_Binning_Yaxis->currentText().toLower();
    hTemplateFile << "YAxis_Binnings=";
    if((strBinType.indexOf("pass") == -1) && (strBinType.indexOf("fail") == -1))
        hTemplateFile << editSQL_Genealogy_YieldVsYield_Binlist_Yaxis->text() << endl;			// Bin list: list of specific bins
    else
        hTemplateFile << comboSQL_Genealogy_YieldVsYield_Binning_Yaxis->currentText() << endl;	// Bin list: "pass only","fail only"
    if(frameSQL_Genealogy_YieldVsYield_Granularity->isEnabled() && radioSQL_Genealogy_YieldVsYield_Granularity_Wafer->isChecked())
        hTemplateFile << "Opions_Granularity=wafer" << endl;
    else
        hTemplateFile << "Opions_Granularity=lot" << endl;

    // Write charting style & options
    SqlReport_Options_WriteSettings(hTemplateFile);

    // Close Yield section
    hTemplateFile << "</genealogy_yieldvsyield>" << endl;

    // Close section
    hTemplateFile << "</section_enterprise_report>" << endl;
}


///////////////////////////////////////////////////////////
// SQL Report = Genealogy - Yield vs. Parameter
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Report selected
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_Genealogy_YieldVsParameter(void)
{
    // Update button states
    toolButtonSql_UPH->setChecked(false);
    toolButtonSql_Yield->setChecked(false);
    toolButtonSql_WYR_Standard->setChecked(false);
    toolButtonSql_ConsolidatedYield->setChecked(false);
    toolButtonSql_YieldWizard->setChecked(false);
    toolButtonSql_Genealogy_YieldVsYield->setChecked(false);
    toolButtonSql_Genealogy_YieldVsParameter->setChecked(true);

    // Keep track of report selected
    SqlReportSelected(SQL_REPORT_GENEALOGY_YIELDVSPARAMETER);

    // Ensure Gui is up to date
    OnSqlReport_Genealogy_YieldVsParameter_UpdateGui();

    // Focus on label field.
    editSQL_Genealogy_YieldVsParameter_Title->setFocus();
}

///////////////////////////////////////////////////////////
// Make sure all controls are in a correct state
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_Genealogy_YieldVsParameter_UpdateGui(void)
{
    // Hide/Show controls
    if(editSQL_Genealogy_YieldVsParameter_Product->text().isEmpty())
    {
        groupSQL_Genealogy_YieldVsParameter_Xaxis->setEnabled(false);
        groupSQL_Genealogy_YieldVsParameter_Yaxis->setEnabled(false);
        groupSQL_Genealogy_YieldVsParameter_Options->setEnabled(false);
        return;
    }

    // If product selected, enable all groups
    groupSQL_Genealogy_YieldVsParameter_Xaxis->setEnabled(true);
    groupSQL_Genealogy_YieldVsParameter_Yaxis->setEnabled(true);
    groupSQL_Genealogy_YieldVsParameter_Options->setEnabled(true);

    // Binning buttons/controls
    switch(comboSQL_Genealogy_YieldVsParameter_Binning_Yaxis->currentIndex())
    {
        case 0:	// PASS only bins
        case 1:	// FAIL only bins
            editSQL_Genealogy_YieldVsParameter_Binlist_Yaxis->hide();
            buttonSQL_Genealogy_YieldVsParameter_PickBinlist_Yaxis->hide();
            break;
        case 2:	// Specific bins...
            editSQL_Genealogy_YieldVsParameter_Binlist_Yaxis->show();
            buttonSQL_Genealogy_YieldVsParameter_PickBinlist_Yaxis->show();
            // Set focus to edit field.
            editSQL_Genealogy_YieldVsParameter_Binlist_Yaxis->setFocus();
            break;
    }

    // Disable granularity radios if Final Test selected for one axis
    frameSQL_Genealogy_YieldVsParameter_Granularity->setEnabled(false);
    radioSQL_Genealogy_YieldVsParameter_Granularity_Lot->setChecked(true);
    if(pGexMainWindow && pGexMainWindow->pWizardOneQuery)
    {
        QString				strDatabaseName = pGexMainWindow->pWizardOneQuery->comboBoxDatabaseName->currentText();
        if(strDatabaseName.startsWith("[Local]") || strDatabaseName.startsWith("[Server]"))
            strDatabaseName = strDatabaseName.section("]",1).trimmed();
        GexDatabaseEntry	*pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(strDatabaseName);
        if(pDatabaseEntry && pDatabaseEntry->m_pExternalDatabase)
        {
            if(	!pDatabaseEntry->m_pExternalDatabase->IsTestingStage_FinalTest(comboSQL_Genealogy_YieldVsParameter_TestingStage_Xaxis->currentText()) &&
                !pDatabaseEntry->m_pExternalDatabase->IsTestingStage_FinalTest(comboSQL_Genealogy_YieldVsParameter_TestingStage_Yaxis->currentText()))
                frameSQL_Genealogy_YieldVsParameter_Granularity->setEnabled(true);
        }
    }
}

///////////////////////////////////////////////////////////
// Open dialog with possible bins for Y axis
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_Genealogy_YieldVsParameter_OnPickBinlist_Yaxis()
{
    QString		strBinning, strDataType, strProduct;
    QStringList strlProducts = editSQL_Genealogy_YieldVsParameter_Product->text().split(',', QString::SkipEmptyParts);

    strDataType = comboSQL_Genealogy_YieldVsParameter_TestingStage_Yaxis->currentText();
    strProduct = strlProducts[m_strlValidTestingStages.indexOf(strDataType)];
    strBinning = pGexMainWindow->pWizardOneQuery->SqlPickBinningList(true, strDataType, strProduct, true);

    if(!strBinning.isEmpty())
        editSQL_Genealogy_YieldVsParameter_Binlist_Yaxis->setText(strBinning);
}

///////////////////////////////////////////////////////////
// Open dialog with possible parameters for X axis
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_Genealogy_YieldVsParameter_OnPickParameter_Xaxis()
{
    QString		strParameter, strDataType, strProduct;
    QStringList strlProducts = editSQL_Genealogy_YieldVsParameter_Product->text().split(',', QString::SkipEmptyParts);

    strDataType = comboSQL_Genealogy_YieldVsParameter_TestingStage_Xaxis->currentText();
    strProduct = strlProducts[m_strlValidTestingStages.indexOf(strDataType)];
    strParameter = pGexMainWindow->pWizardOneQuery->SqlPickParameterList(true, false, strDataType, strProduct);

    if(!strParameter.isEmpty())
        editSQL_Genealogy_YieldVsParameter_Parameter_Xaxis->setText(strParameter);
}

///////////////////////////////////////////////////////////
// Open dialog with possible products
///////////////////////////////////////////////////////////
void GexSettings::OnSqlReport_Genealogy_YieldVsParameter_OnPickProduct()
{
    QStringList strlSelection, strlProducts, strlTestingStages;

    pGexMainWindow->pWizardOneQuery->SqlPickProductList_Genealogy(strlSelection, true, false, m_strFoundryTestingStage);

    if(strlSelection.count() < 2)
        return;

    // Display selected product
    editSQL_Genealogy_YieldVsParameter_Product->setText(strlSelection[1]);

    // Update available testingStages.
    m_strlValidTestingStages.clear();
    strlTestingStages = strlSelection[0].split(',', QString::KeepEmptyParts);
    strlProducts = strlSelection[1].split(',', QString::KeepEmptyParts);
    for(int nIndex = 0; nIndex < strlTestingStages.count(); nIndex++)
    {
        if(!(strlProducts[nIndex].isEmpty()))
            m_strlValidTestingStages.append(strlTestingStages[nIndex]);
    }

    // Update Y-Axis testing stages (yield). X-Axis (parameter) supports only E-Test for now.
    QString	strSelected;
    strSelected = comboSQL_Genealogy_YieldVsParameter_TestingStage_Yaxis->currentText();
    comboSQL_Genealogy_YieldVsParameter_TestingStage_Yaxis->clear();
    comboSQL_Genealogy_YieldVsParameter_TestingStage_Yaxis->addItems(m_strlValidTestingStages);
    SetCurrentComboItem(comboSQL_Genealogy_YieldVsParameter_TestingStage_Yaxis, strSelected);

    OnSqlReport_Genealogy_YieldVsParameter_UpdateGui();
}

///////////////////////////////////////////////////////////
// Write settings
///////////////////////////////////////////////////////////
void GexSettings::SqlReport_Genealogy_YieldVsParameter_WriteSettings(QTextStream &hTemplateFile)
{
    // Get database name
    QString strDatabaseName = pGexMainWindow->pWizardOneQuery->comboBoxDatabaseName->currentText();

    // Skip the [Local]/[Server] info, and extract database name.
    if(strDatabaseName.startsWith("[Local]") || strDatabaseName.startsWith("[Server]"))
        strDatabaseName = strDatabaseName.section("]",1).trimmed();

    // Home page
    hTemplateFile << "<section_home_page>" << endl;	// Start template marker
    hTemplateFile << "HomeText=" << ReportOptions.GetOption("output", "front_page_text").toString() << endl; //hTemplateFile << "HomeText=" << ReportOptions.strFrontPageText << endl;		// Home page text
    hTemplateFile << "HomeLogo=" << ReportOptions.GetOption("output", "front_page_image").toString() << endl; //hTemplateFile << "HomeLogo=" << ReportOptions.strFrontPageImage << endl;	// Home page logo path
    hTemplateFile << "</section_home_page>" << endl; // End template marker

    // Write Yield/Volume report settings
    hTemplateFile << "<section_enterprise_report>" << endl;
    hTemplateFile << "Title=" << editSQL_Genealogy_YieldVsParameter_Title->text() << endl;
    hTemplateFile << "ReportFunction=Genealogy_YieldVsParameter" << endl;

    // Data (Database, Time period, filters)
    hTemplateFile << "<data>" << endl;											// Data marker
    hTemplateFile << "DatabaseName=" << strDatabaseName  << endl;
    hTemplateFile << "DatabaseType=" << pGexMainWindow->pWizardOneQuery->comboBoxDatabaseType->currentText() << endl;
    hTemplateFile << "TimePeriod=" << gexTimePeriodChoices[pGexMainWindow->pWizardOneQuery->comboBoxTimePeriod->currentIndex()] << endl;
    hTemplateFile << "CalendarFrom=" << pGexMainWindow->pWizardOneQuery->FromDate->date().toString(Qt::ISODate) << endl;
    hTemplateFile << "CalendarTo=" << pGexMainWindow->pWizardOneQuery->ToDate->date().toString(Qt::ISODate) << endl;
    QStringList strlFilters = pGexMainWindow->pWizardOneQuery->GetFiltersXmlLines();
    hTemplateFile << "<filters>" << endl;
    for(QStringList::Iterator it = strlFilters.begin(); it != strlFilters.end(); ++it )
        hTemplateFile << *it << endl;
    hTemplateFile << "</filters>" << endl;
    hTemplateFile << "</data>" << endl;											// Data end marker

    // Genealogy report settings
    QString		strTestingStage, strBinType;
    QStringList strlProducts = editSQL_Genealogy_YieldVsParameter_Product->text().split(',', QString::SkipEmptyParts);

    hTemplateFile << "<genealogy_yieldvsparameter>" << endl;
    hTemplateFile << "Product=" << editSQL_Genealogy_YieldVsParameter_Product->text() << endl;
    strTestingStage = comboSQL_Genealogy_YieldVsParameter_TestingStage_Xaxis->currentText();
    hTemplateFile << "XAxis_Product=" << strlProducts[m_strlValidTestingStages.indexOf(strTestingStage)] << endl;
    hTemplateFile << "XAxis_TestingStage=" << strTestingStage << endl;
    hTemplateFile << "XAxis_Parameter=" << editSQL_Genealogy_YieldVsParameter_Parameter_Xaxis->text() << endl;
    strTestingStage = comboSQL_Genealogy_YieldVsParameter_TestingStage_Yaxis->currentText();
    hTemplateFile << "YAxis_Product=" << strlProducts[m_strlValidTestingStages.indexOf(strTestingStage)] << endl;
    hTemplateFile << "YAxis_TestingStage=" << strTestingStage << endl;
    strBinType = comboSQL_Genealogy_YieldVsParameter_Binning_Yaxis->currentText().toLower();
    hTemplateFile << "YAxis_Binnings=";
    if((strBinType.indexOf("pass") == -1) && (strBinType.indexOf("fail") == -1))
        hTemplateFile << editSQL_Genealogy_YieldVsParameter_Binlist_Yaxis->text() << endl;			// Bin list: list of specific bins
    else
        hTemplateFile << comboSQL_Genealogy_YieldVsParameter_Binning_Yaxis->currentText() << endl;	// Bin list: "pass only","fail only"
    if(frameSQL_Genealogy_YieldVsParameter_Granularity->isEnabled() && radioSQL_Genealogy_YieldVsParameter_Granularity_Wafer->isChecked())
        hTemplateFile << "Opions_Granularity=wafer" << endl;
    else
        hTemplateFile << "Opions_Granularity=lot" << endl;

    // Write charting style & options
    SqlReport_Options_WriteSettings(hTemplateFile);

    // Close Yield section
    hTemplateFile << "</genealogy_yieldvsparameter>" << endl;

    // Close section
    hTemplateFile << "</section_enterprise_report>" << endl;
}

