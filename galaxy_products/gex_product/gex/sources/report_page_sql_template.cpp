/////////////////////////////////////////////////////////////////////////////
// Creates Report section based on SQL query
/////////////////////////////////////////////////////////////////////////////

#if defined unix || __MACH__
#include <stdlib.h>
#include <unistd.h>
#endif

#ifdef _WIN32
  #include <windows.h>
#endif
#include <qclipboard.h>
#include <qapplication.h>
#include <QMessageBox>
#include <QFileDialog>

#include "browser_dialog.h"
#include "gex_shared.h"
#include "gex_er_prod_yield.h"
#include "gex_er_prod_uph.h"
#include "gex_er_prod_adv_yield.h"
#include "gex_er_prod_cons_yield.h"
#include "gex_er_genealogy_yield_vs_param.h"
#include "gex_er_genealogy_yield_vs_yield.h"
#include "gex_er_wyr_standard.h"
#include "gex_report.h"
#include "db_engine.h"
#include "report_options.h"
#include "product_info.h"
#include "gex_database_entry.h"
#include "engine.h"
#include "report_template.h"
#include "message.h"
#include "xychart_data.h"

// main.cpp
extern GexMainwindow *	pGexMainWindow;

/////////////////////////////////////////////////////////////////////////////
// Creates ALL the .CSV or HTML pages for 'Enterprise Report' report section
/////////////////////////////////////////////////////////////////////////////
int CGexReport::CreatePages_ER(GS::Gex::ReportTemplateSection* pSection)
{
    GS::Gex::CustomReportEnterpriseReportSection* pEnterpriseReport =
        pSection->pEnterpriseReport;

    // Update process bar...
    GS::Gex::Engine::GetInstance().UpdateProgressStatus(false,-1,-1);	// Increment to next step.

    // Retreive database handle
    GexDatabaseEntry *pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(pEnterpriseReport->m_strDatabaseLogicalName);
    if(pDatabaseEntry == NULL)
    {
        // Failed finding database
        fprintf(hReportFile,"<b>*ERROR*</b> Failed finding database '%s'<br>",pEnterpriseReport->m_strDatabaseLogicalName.toLatin1().constData());
        return GS::StdLib::Stdf::NoError;
    }

    // Check if the selected DB is a SQL DB
    if(!pDatabaseEntry->IsExternal())
    {
        // Not a SQL DB
        fprintf(hReportFile,"<b>*ERROR*</b> The database '%s' doesn't support SQL queries. Report can't be created<br>",pEnterpriseReport->m_strDatabaseLogicalName.toLatin1().constData());
        return GS::StdLib::Stdf::NoError;
    }

    // Check external DB pointer
    if(!pDatabaseEntry->m_pExternalDatabase)
    {
        // Not a SQL DB
        fprintf(hReportFile,"<b>*ERROR*</b> Database pointer not available for database '%s'<br>",pEnterpriseReport->m_strDatabaseLogicalName.toLatin1().constData());
        return GS::StdLib::Stdf::NoError;
    }

    // Init query Filter object
    GexDbPlugin_Filter	cFilters(this);
    unsigned int		uiIndex=0;
    bool				bFound = false;
    cFilters.SetFilters(pEnterpriseReport->m_strFiltersList);
    cFilters.strDataTypeQuery	= pEnterpriseReport->m_strDatabaseType;
    cFilters.calendarFrom		= pEnterpriseReport->m_clFromDate;
    cFilters.calendarTo			= pEnterpriseReport->m_clToDate;
    while((gexTimePeriodChoices[uiIndex] != 0) && !bFound)
    {
        if(pEnterpriseReport->m_strTimePeriod == gexTimePeriodChoices[uiIndex])
        {
            cFilters.iTimePeriod = uiIndex;
            bFound = true;
        }
        uiIndex++;
    }

    // Clear global report objects (those objects are CGexReport members, so that there are persistent, which
    // enables to use them when a report hyperlink is clicked).
    clER_PartsData.Clear();
    clWyrData.clear();
    clXYChartList.clear();
    // Init some chart variables
    clER_PartsData.Init(pEnterpriseReport->m_YieldWizard_strlFields_GraphSplit, pEnterpriseReport->m_YieldWizard_strlFields_LayerSplit,
                        pEnterpriseReport->m_strAggregateField, pEnterpriseReport->m_YieldWizard_bSoftBin,
                        pEnterpriseReport->m_plistSeries, pEnterpriseReport->m_YieldWizard_EtWt_MaverickWafer_strAlarmType,
                        pEnterpriseReport->m_YieldWizard_EtWt_MaverickWafer_nAlarmCount, pEnterpriseReport->m_YieldWizard_EtWt_MaverickLot_nWaferCount,
                        pEnterpriseReport->m_Genealogy_Options_strGranularity);

    // Init some chart variables
    clXYChartList.m_strCumulField = pEnterpriseReport->m_strAggregateField;
    clXYChartList.m_strSplitField = pEnterpriseReport->m_strSplitField;
    clXYChartList.m_strBinList = pEnterpriseReport->m_strBinList;

    // Report Function dispatcher
    if(pEnterpriseReport->m_strReportType.toLower() == "prod_uph")
    {
        GexERProdUPH erProdUPH(hReportFile, pDatabaseEntry, cFilters, pSection, clXYChartList);
        erProdUPH.create();
    }

    if(pEnterpriseReport->m_strReportType.toLower() == "prod_yield")
    {
        GexERProdYield erProdYield(hReportFile, pDatabaseEntry, cFilters, pSection, clXYChartList);
        erProdYield.create();
    }

    if(pEnterpriseReport->m_strReportType.toLower() == "prod_consolidatedyield")
    {
        GexERProdConsolidatedYield erProdConsolidatedYield(hReportFile, pDatabaseEntry, cFilters, pSection, clXYChartList);
        erProdConsolidatedYield.create();
    }
    if(pEnterpriseReport->m_strReportType.toLower() == "prod_advancedyield")
    {
        GexERProdAdvancedYield erProdAdvancedYield(hReportFile, pDatabaseEntry, cFilters, pSection, clER_PartsData);
        erProdAdvancedYield.create();
    }

    if(pEnterpriseReport->m_strReportType.toLower() == "wyr_standard")
    {
        GexERWyrStandard erWyrStandard(hReportFile, pDatabaseEntry, cFilters, pSection, clWyrData);
        erWyrStandard.create();
    }

    if(pEnterpriseReport->m_strReportType.toLower() == "genealogy_yieldvsyield")
    {
        GexERGenealogyYieldvsYield erGenealogyYieldvsYield(hReportFile, pDatabaseEntry, cFilters, pSection, clER_PartsData);
        erGenealogyYieldvsYield.create();
    }

    if(pEnterpriseReport->m_strReportType.toLower() == "genealogy_yieldvsparameter")
    {
        GexERGenealogyYieldvsParameter erGenealogyYieldvsParameter(hReportFile, pDatabaseEntry, cFilters, pSection, clER_PartsData);
        erGenealogyYieldvsParameter.create();
    }

    return GS::StdLib::Stdf::NoError;
}

///////////////////////////////////////////////////////////
// Action link clicked, relative to Enterprise Reports
///////////////////////////////////////////////////////////
void CGexReport::ProcessActionLink_EnterpriseReports(const QString & strGexHtmlToolbarAction)
{
    QString strToken, strMission, strReport, strAction;

    // Get mission
    strToken = strGexHtmlToolbarAction.section("--",1,1);
    if(!strToken.toLower().startsWith("mission="))
        return;
    strMission = strToken.section("=", 1, 1);

    // Get report
    strToken = strGexHtmlToolbarAction.section("--",2,2);
    if(!strToken.toLower().startsWith("report="))
        return;
    strReport = strToken.section("=", 1, 1);

    // Get action
    strToken = strGexHtmlToolbarAction.section("--",3,3);
    if(!strToken.toLower().startsWith("action="))
        return;
    strAction = strToken.section("=", 1, 1);

    // Dispatch action
    if((strMission.toLower() == "wyr") && (strReport.toLower() == "standard") && (strAction.toLower() == "saveexcelclipboard"))
        Enterprise_Wyr_ExportToExcelClipboard(strGexHtmlToolbarAction);
    else if((strMission.toLower() == "wyr") && (strReport.toLower() == "standard") && (strAction.toLower() == "saveexcelfile"))
        Enterprise_Wyr_ExportToExcelFile(strGexHtmlToolbarAction);
    else if((strMission.toLower() == "prod") && (strReport.toLower() == "uph") && (strAction.toLower() == "saveexcelclipboard"))
        Enterprise_Prod_Uph_ExportToExcelClipboard(strGexHtmlToolbarAction);
    else if((strMission.toLower() == "prod") && (strReport.toLower() == "uph") && (strAction.toLower() == "saveexcelfile"))
        Enterprise_Prod_Uph_ExportToExcelFile(strGexHtmlToolbarAction);
    else if((strMission.toLower() == "prod") && (strReport.toLower() == "yield") && (strAction.toLower() == "saveexcelclipboard"))
        Enterprise_Prod_Yield_ExportToExcelClipboard(strGexHtmlToolbarAction);
    else if((strMission.toLower() == "prod") && (strReport.toLower() == "yield") && (strAction.toLower() == "saveexcelfile"))
        Enterprise_Prod_Yield_ExportToExcelFile(strGexHtmlToolbarAction);
    else if((strMission.toLower() == "prod") && (strReport.toLower() == "yield_advanced") && (strAction.toLower() == "saveexcelclipboard"))
        Enterprise_Prod_Yield_Advanced_ExportToExcelClipboard(strGexHtmlToolbarAction);
    else if((strMission.toLower() == "prod") && (strReport.toLower() == "yield_advanced") && (strAction.toLower() == "saveexcelfile"))
        Enterprise_Prod_Yield_Advanced_ExportToExcelFile(strGexHtmlToolbarAction);
}

///////////////////////////////////////////////////////////
// Save WYR data into clipboard, ready to paste to spreadsheet file (Windows only)
///////////////////////////////////////////////////////////
#ifdef _WIN32
void CGexReport::Enterprise_Wyr_ExportToExcelClipboard(const QString & strGexHtmlToolbarAction)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_EVALUATION)
    {
        // Evaluation mode, refuse to run this function!
        GS::Gex::Message::information(
            "", "This function is disabled in Evaluation mode\n\nContact " +
            QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }

    // Extract data, and generate buffer for the clipboard
    QString					strBigLine, strMessage;
    QString					strToken, strSiteName;
    unsigned int			uiYear, uiWeekNb;

    // Get site name
    strToken = strGexHtmlToolbarAction.section("--",4,4);
    if(!strToken.toLower().startsWith("site="))
        return;
    strSiteName = strToken.section("=", 1, 1);

    // Get year
    strToken = strGexHtmlToolbarAction.section("--",5,5);
    if(!strToken.toLower().startsWith("year="))
        return;
    uiYear = strToken.section("=", 1, 1).toUInt();

    // Get week nb
    strToken = strGexHtmlToolbarAction.section("--",6,6);
    if(!strToken.toLower().startsWith("week="))
        return;
    uiWeekNb = strToken.section("=", 1, 1).toUInt();

    // Find WYR data corresponding to parameters specified in the link
//    GexDbPlugin_WyrDataset *pWyrDataset = clWyrData.first();
//    while(pWyrDataset)
    bool wyrDataFound(false);
    GexDbPlugin_WyrDataset *pWyrDataset = NULL;
    foreach(pWyrDataset, clWyrData)
    {
        if( (pWyrDataset->m_strSiteName == strSiteName)
             && (pWyrDataset->m_uiYear == uiYear)
             && (pWyrDataset->m_uiWeekNb == uiWeekNb)
           )
        {
            wyrDataFound = true;
            break;
        }
//        pWyrDataset = clWyrData.next();
    }

    if(!wyrDataFound)
        return;

    // Add tile row to clipboard buffer
    strBigLine = pWyrDataset->m_strTitleRow.replace(',', '\t');
    strBigLine += "\n";

    // Add all data rows
    QStringList::iterator it;
    for(it = pWyrDataset->m_strlDataRows.begin(); it != pWyrDataset->m_strlDataRows.end(); it++)
    {
        strBigLine += (*it).replace(',', '\t');
        strBigLine += "\n";
    }

    // Copy data to clipboard
    QClipboard *cb = QGuiApplication::clipboard();
    cb->setText(strBigLine,QClipboard::Clipboard );


    strMessage = "Table saved to clipboard in Spreadsheet format,\nready to paste!\nLaunch Spreadsheet application now?";
    bool lOk;
    GS::Gex::Message::request("", strMessage, lOk);
    if (! lOk)
    {
        return;
    }

//#ifdef _WIN32
        // Launch excel
    ShellExecuteA(NULL,
           "open",
            m_pReportOptions->GetOption("preferences", "ssheet_editor").toString().toLatin1().constData(),
           NULL,
           NULL,
           SW_SHOWNORMAL);
//#else
//	system(pReportOptions->GetOption("preferences", "ssheet_editor").toString().toLatin1().constData());  //system(pReportOptions->m_PrefMap["ssheet_editor"].toLatin1().constData());
//#endif
}
#else
void CGexReport::Enterprise_Wyr_ExportToExcelClipboard(const QString& /*strGexHtmlToolbarAction*/)
{
}
#endif

///////////////////////////////////////////////////////////
// Save WYR data into CSV file
///////////////////////////////////////////////////////////
void CGexReport::Enterprise_Wyr_ExportToExcelFile(const QString & strGexHtmlToolbarAction)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_EVALUATION)
    {
        // Evaluation mode, refuse to run this function!
        GS::Gex::Message::information(
            "", "This function is disabled in Evaluation mode\n\nContact " +
            QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }

    // Extract data, and generate buffer for the clipboard
    QString					strBigLine, strMessage;
    QString					strToken, strSiteName;
    unsigned int			uiYear, uiWeekNb;

    // Get site name
    strToken = strGexHtmlToolbarAction.section("--",4,4);
    if(!strToken.toLower().startsWith("site="))
        return;
    strSiteName = strToken.section("=", 1, 1);

    // Get year
    strToken = strGexHtmlToolbarAction.section("--",5,5);
    if(!strToken.toLower().startsWith("year="))
        return;
    uiYear = strToken.section("=", 1, 1).toUInt();

    // Get week nb
    strToken = strGexHtmlToolbarAction.section("--",6,6);
    if(!strToken.toLower().startsWith("week="))
        return;
    uiWeekNb = strToken.section("=", 1, 1).toUInt();

    // Build file name
    QString strFileName;
    strFileName.sprintf("weekly_yield_report_ww%d_%d_%s.csv", uiWeekNb, uiYear, strSiteName.toLatin1().constData());

    // Let's user tell where to save the configuration.
    strFileName = QFileDialog::getSaveFileName(pGexMainWindow, "Save Weekly Yield Report as...",
                                               strFileName, "Weekly Yield Report (*.csv)");

    // If no file selected, ignore command.
    if(strFileName.isEmpty())
        return;

    // Find WYR data corresponding to parameters specified in the link
//    GexDbPlugin_WyrDataset *pWyrDataset = clWyrData.first();
//    while(pWyrDataset)
    bool wyrDataFound(false);
    GexDbPlugin_WyrDataset * pWyrDataset = NULL;
    foreach(pWyrDataset, clWyrData)
    {
        if( (pWyrDataset->m_strSiteName == strSiteName)
            && (pWyrDataset->m_uiYear == uiYear)
            && (pWyrDataset->m_uiWeekNb == uiWeekNb)
           )
        {
            wyrDataFound = true;
            break;
        }
//        pWyrDataset = clWyrData.next();
    }

    if(!wyrDataFound)
        return;

    // Open CSV file file, and write WYR data!
    FILE	*hDest;

    // Check if destination exists...
    hDest = fopen(strFileName.toLatin1().constData(),"r");
    if(hDest != NULL)
    {
        // File already exist...ask if overwrite!
        fclose(hDest);

        int i=QMessageBox::warning(pGexMainWindow, GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
        "File already exists. Overwrite it?",
        "No",
        "Yes", 0, 0, 1 );
        if(i != 1)
            return;
    }

    // Open file for writing
    hDest = fopen(strFileName.toLatin1().constData(),"w");
    if(hDest == NULL)
    {
        GS::Gex::Message::critical("",
                                   GEX_T("ERROR: Failed creating CSV file..."));
        return;	// Failed writing to destination
    }

    // Write tile row to file
    fprintf(hDest, "%s\n", pWyrDataset->m_strTitleRow.toLatin1().constData());

    // Add all data rows
    QStringList::iterator it;
    for(it = pWyrDataset->m_strlDataRows.begin(); it != pWyrDataset->m_strlDataRows.end(); it++)
        fprintf(hDest, "%s\n", (*it).toLatin1().constData());

    // Close file
    fclose(hDest);
}

///////////////////////////////////////////////////////////
// Save PROD - UPH data into clipboard, ready to paste to spreadsheet file (Windows only)
///////////////////////////////////////////////////////////
#ifdef _WIN32
void CGexReport::Enterprise_Prod_Uph_ExportToExcelClipboard(const QString & strGexHtmlToolbarAction)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_EVALUATION)
    {
        // Evaluation mode, refuse to run this function!
        GS::Gex::Message::information(
            "", "This function is disabled in Evaluation mode\n\nContact " +
            QString(GEX_EMAIL_SALES)+" for more information!");
        return;
    }

    // Extract data, and generate buffer for the clipboard
    QString						strBigLine, strMessage;
    QString						strToken, strSplitValue;
    GexDbPlugin_XYChart_Data	*pXYChartData;
    int							nIndex;

    // Get split
    strToken = strGexHtmlToolbarAction.section("--",4,4);
    if(strToken.toLower().startsWith("split="))
        strSplitValue = strToken.section("=", 1, 1);

    if(strSplitValue.isEmpty())
    {
        // First line = column names
        if(!clXYChartList.m_strSplitField.isEmpty())
            strBigLine = clXYChartList.m_strSplitField + "\t";
        strBigLine += clXYChartList.m_strCumulField + "\t";
        strBigLine += "Total Parts\tTotal Time (s)\tUPH\n";
        // Data
//        for(pXYChartData=clXYChartList.first(); pXYChartData; pXYChartData=clXYChartList.next())
        foreach(pXYChartData, clXYChartList)
        {
            for(nIndex=0; nIndex<pXYChartData->m_nNbDataPoints; nIndex++)
            {
                if(!clXYChartList.m_strSplitField.isEmpty())
                    strBigLine += pXYChartData->m_strSplitValue + "\t";
                strBigLine += pXYChartData->m_pcXAxisLabels[nIndex];
                strBigLine += "\t";
                strBigLine += QString::number((int)(pXYChartData->m_plfData_1[nIndex])) + "\t";
                strBigLine += QString::number((int)(pXYChartData->m_plfData_2[nIndex])) + "\t";
                strBigLine += QString::number((int)(pXYChartData->m_plfData[nIndex])) + "\n";
            }
        }
    }
    else
    {
        // First line = column names
        strBigLine = clXYChartList.m_strCumulField + "\t";
        strBigLine += "Total Parts\tTotal Time (s)\tUPH\n";
        // Data
//        for(pXYChartData=clXYChartList.first(); pXYChartData; pXYChartData=clXYChartList.next())
        foreach(pXYChartData, clXYChartList)
        {
            if(pXYChartData->m_strSplitValue == strSplitValue)
            {
                for(nIndex=0; nIndex<pXYChartData->m_nNbDataPoints; nIndex++)
                {
                    strBigLine += pXYChartData->m_pcXAxisLabels[nIndex];
                    strBigLine += "\t";
                    strBigLine += QString::number((int)(pXYChartData->m_plfData_1[nIndex])) + "\t";
                    strBigLine += QString::number((int)(pXYChartData->m_plfData_2[nIndex])) + "\t";
                    strBigLine += QString::number((int)(pXYChartData->m_plfData[nIndex])) + "\n";
                }
                break;
            }
        }
    }

    // Copy data to clipboard
    QClipboard *cb = QGuiApplication::clipboard();
    cb->setText(strBigLine,QClipboard::Clipboard );

    strMessage = "Table saved to clipboard in Spreadsheet format,\nready to paste!\nLaunch Spreadsheet application now?";
    bool lOk;
    GS::Gex::Message::request("", strMessage, lOk);
    if (! lOk)
    {
        return;
    }

//#ifdef _WIN32
        // Launch spreadsheet editor
    ShellExecuteA(NULL,
           "open",
        m_pReportOptions->GetOption("preferences", "ssheet_editor").toString().toLatin1().constData(),
           NULL,
           NULL,
           SW_SHOWNORMAL);
//#else
//	system(pReportOptions->GetOption("preferences", "ssheet_editor").toString().toLatin1().constData());  	//system(pReportOptions->m_PrefMap["ssheet_editor"].toLatin1().constData());
//#endif
}
#else
void CGexReport::Enterprise_Prod_Uph_ExportToExcelClipboard(const QString& /*strGexHtmlToolbarAction*/)
{
}
#endif

///////////////////////////////////////////////////////////
// Save PROD - UPH data into CSV file
///////////////////////////////////////////////////////////
void CGexReport::Enterprise_Prod_Uph_ExportToExcelFile(const QString & strGexHtmlToolbarAction)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_EVALUATION)
    {
        // Evaluation mode, refuse to run this function!
        GS::Gex::Message::information(
            "", "This function is disabled in Evaluation mode\n\nContact " +
            QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }

    // Extract data, and generate buffer for the clipboard
    QString		strToken, strSplitValue;

    // Get split
    strToken = strGexHtmlToolbarAction.section("--",4,4);
    if(strToken.toLower().startsWith("split="))
        strSplitValue = strToken.section("=", 1, 1);

    // Build file name
    QString strFileName;
    strFileName = "prouction_uph_report";
    if(!strSplitValue.isEmpty())
        strFileName += "_" + strSplitValue;
    strFileName += ".csv";

    // Let's user tell where to save the configuration.
    strFileName = QFileDialog::getSaveFileName(pGexMainWindow, "Save Production UPH Report as...",
                                               strFileName, "Prouction UPH Report (*.csv)");

    // If no file selected, ignore command.
    if(strFileName.isEmpty())
        return;

    // Open CSV file file, and write WYR data!
    FILE	*hDest;

    // Check if destination exists...
    hDest = fopen(strFileName.toLatin1().constData(),"r");
    if(hDest != NULL)
    {
        // File already exist...ask if overwrite!
        fclose(hDest);

        int i=QMessageBox::warning(pGexMainWindow, GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
        "File already exists. Overwrite it?",
        "No",
        "Yes", 0, 0, 1 );
        if(i != 1)
            return;
    }

    // Open file for writing
    hDest = fopen(strFileName.toLatin1().constData(),"w");
    if(hDest == NULL)
    {
        GS::Gex::Message::critical("",
                                   GEX_T("ERROR: Failed creating CSV file..."));
        return;	// Failed writing to destination
    }

    // Extract data, and generate buffer for the clipboard
    GexDbPlugin_XYChart_Data	*pXYChartData;
    int							nIndex;

    if(strSplitValue.isEmpty())
    {
        // First line = column names
        if(!clXYChartList.m_strSplitField.isEmpty())
            fprintf(hDest, "%s,", clXYChartList.m_strSplitField.toLatin1().constData());
        fprintf(hDest, "%s,", clXYChartList.m_strCumulField.toLatin1().constData());
        fprintf(hDest, "Total Parts,Total Time (s),UPH\n");
        // Data
//        for(pXYChartData=clXYChartList.first(); pXYChartData; pXYChartData=clXYChartList.next())
        foreach(pXYChartData, clXYChartList)
        {
            for(nIndex=0; nIndex<pXYChartData->m_nNbDataPoints; nIndex++)
            {
                if(!clXYChartList.m_strSplitField.isEmpty())
                    fprintf(hDest, "%s,", pXYChartData->m_strSplitValue.toLatin1().constData());
                fprintf(hDest, "%s,", pXYChartData->m_pcXAxisLabels[nIndex]);
                fprintf(hDest, "%s,", QString::number((int)(pXYChartData->m_plfData_1[nIndex])).toLatin1().constData());
                fprintf(hDest, "%s,", QString::number((int)(pXYChartData->m_plfData_2[nIndex])).toLatin1().constData());
                fprintf(hDest, "%s,", QString::number((int)(pXYChartData->m_plfData[nIndex])).toLatin1().constData());
            }
        }
    }
    else
    {
        // First line = column names
        fprintf(hDest, "%s,", clXYChartList.m_strCumulField.toLatin1().constData());
        fprintf(hDest, "Total Parts,Total Time (s),UPH\n");
        // Data
//        for(pXYChartData=clXYChartList.first(); pXYChartData; pXYChartData=clXYChartList.next())
        foreach(pXYChartData, clXYChartList)
        {
            if(pXYChartData->m_strSplitValue == strSplitValue)
            {
                for(nIndex=0; nIndex<pXYChartData->m_nNbDataPoints; nIndex++)
                {
                    fprintf(hDest, "%s,", pXYChartData->m_pcXAxisLabels[nIndex]);
                    fprintf(hDest, "%s,", QString::number((int)(pXYChartData->m_plfData_1[nIndex])).toLatin1().constData());
                    fprintf(hDest, "%s,", QString::number((int)(pXYChartData->m_plfData_2[nIndex])).toLatin1().constData());
                    fprintf(hDest, "%s,", QString::number((int)(pXYChartData->m_plfData[nIndex])).toLatin1().constData());
                }
                break;
            }
        }
    }

    // Close file
    fclose(hDest);
}

///////////////////////////////////////////////////////////
// Save PROD - Yield data into clipboard, ready to paste to spreadsheet file (Windows only)
///////////////////////////////////////////////////////////
#ifdef _WIN32
void CGexReport::Enterprise_Prod_Yield_ExportToExcelClipboard(const QString & strGexHtmlToolbarAction)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_EVALUATION)
    {
        // Evaluation mode, refuse to run this function!
        GS::Gex::Message::information(
            "", "This function is disabled in Evaluation mode\n\nContact " +
            QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }

    // Extract data, and generate buffer for the clipboard
    QString						strBigLine, strMessage;
    QString						strToken, strSplitValue;
    GexDbPlugin_XYChart_Data	*pXYChartData;
    int							nIndex;

    // Get split
    strToken = strGexHtmlToolbarAction.section("--",4,4);
    if(strToken.toLower().startsWith("split="))
        strSplitValue = strToken.section("=", 1, 1);

    if(strSplitValue.isEmpty())
    {
        // First line = column names
        if(!clXYChartList.m_strSplitField.isEmpty())
            strBigLine = clXYChartList.m_strSplitField + "\t";
        strBigLine += clXYChartList.m_strCumulField + "\t";
        if(clXYChartList.m_strBinList.toLower() == "pass")
            strBigLine += "Total Parts\tTotal Good\tYield (%)\n";
        else
            strBigLine += "Total Parts\tTotal Matching Parts\tYield (%)\n";
        // Data
//        for(pXYChartData=clXYChartList.first(); pXYChartData; pXYChartData=clXYChartList.next())
        foreach(pXYChartData, clXYChartList)
        {
            for(nIndex=0; nIndex<pXYChartData->m_nNbDataPoints; nIndex++)
            {
                if(!clXYChartList.m_strSplitField.isEmpty())
                    strBigLine += pXYChartData->m_strSplitValue + "\t";
                strBigLine += pXYChartData->m_pcXAxisLabels[nIndex];
                strBigLine += "\t";
                strBigLine += QString::number((int)(pXYChartData->m_plfData_1[nIndex])) + "\t";
                strBigLine += QString::number((int)(pXYChartData->m_plfData_2[nIndex])) + "\t";
                strBigLine += QString::number(pXYChartData->m_plfData[nIndex], 'f', 2) + "\n";
            }
        }
    }
    else
    {
        // First line = column names
        strBigLine = clXYChartList.m_strCumulField + "\t";
        if(clXYChartList.m_strBinList.toLower() == "pass")
            strBigLine += "Total Parts\tTotal Good\tYield (%)\n";
        else
            strBigLine += "Total Parts\tTotal Matching Parts\tYield (%)\n";
        // Data
//        for(pXYChartData=clXYChartList.first(); pXYChartData; pXYChartData=clXYChartList.next())
        foreach(pXYChartData, clXYChartList)
        {
            if(pXYChartData->m_strSplitValue == strSplitValue)
            {
                for(nIndex=0; nIndex<pXYChartData->m_nNbDataPoints; nIndex++)
                {
                    strBigLine += pXYChartData->m_pcXAxisLabels[nIndex];
                    strBigLine += "\t";
                    strBigLine += QString::number((int)(pXYChartData->m_plfData_1[nIndex])) + "\t";
                    strBigLine += QString::number((int)(pXYChartData->m_plfData_2[nIndex])) + "\t";
                    strBigLine += QString::number(pXYChartData->m_plfData[nIndex], 'f', 2) + "\n";
                }
                break;
            }
        }
    }

    // Copy data to clipboard
    QClipboard *cb = QGuiApplication::clipboard();
    cb->setText(strBigLine,QClipboard::Clipboard );

    strMessage = "Table saved to clipboard in Spreadsheet format,\nready to paste!\nLaunch Spreadsheet application now?";
    bool lOk;
    GS::Gex::Message::request("", strMessage, lOk);
    if (! lOk)
    {
        return;
    }

//#ifdef _WIN32
        // Launch spreadsheet editor
    ShellExecuteA(NULL,
           "open",
        m_pReportOptions->GetOption("preferences", "ssheet_editor").toString().toLatin1().constData(),
           NULL,
           NULL,
           SW_SHOWNORMAL);
//#else
//	system(pReportOptions->GetOption("preferences", "ssheet_editor").toString().toLatin1().constData());
//#endif
}
#else
void CGexReport::Enterprise_Prod_Yield_ExportToExcelClipboard(const QString& /*strGexHtmlToolbarAction*/)
{
}
#endif

///////////////////////////////////////////////////////////
// Save PROD - Yield data into CSV file
///////////////////////////////////////////////////////////
void CGexReport::Enterprise_Prod_Yield_ExportToExcelFile(const QString & strGexHtmlToolbarAction)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_EVALUATION)
    {
        // Evaluation mode, refuse to run this function!
        GS::Gex::Message::information(
            "", "This function is disabled in Evaluation mode\n\nContact " +
            QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }

    // Extract data, and generate buffer for the clipboard
    QString		strToken, strSplitValue;

    // Get split
    strToken = strGexHtmlToolbarAction.section("--",4,4);
    if(strToken.toLower().startsWith("split="))
        strSplitValue = strToken.section("=", 1, 1);

    // Build file name
    QString strFileName;
    strFileName = "prouction_yield_report";
    if(!strSplitValue.isEmpty())
        strFileName += "_" + strSplitValue;
    strFileName += ".csv";

    // Let's user tell where to save the configuration.
    strFileName = QFileDialog::getSaveFileName(pGexMainWindow, "Save Production Yield Report as...",
                                               strFileName, "Prouction Yield Report (*.csv)");

    // If no file selected, ignore command.
    if(strFileName.isEmpty())
        return;

    // Open CSV file file, and write WYR data!
    FILE	*hDest;

    // Check if destination exists...
    hDest = fopen(strFileName.toLatin1().constData(),"r");
    if(hDest != NULL)
    {
        // File already exist...ask if overwrite!
        fclose(hDest);

        int i=QMessageBox::warning(pGexMainWindow, GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
        "File already exists. Overwrite it?",
        "No",
        "Yes", 0, 0, 1 );
        if(i != 1)
            return;
    }

    // Open file for writing
    hDest = fopen(strFileName.toLatin1().constData(),"w");
    if(hDest == NULL)
    {
        GS::Gex::Message::critical("",
                                   GEX_T("ERROR: Failed creating CSV file..."));
        return;	// Failed writing to destination
    }

    // Extract data, and generate buffer for the clipboard
    GexDbPlugin_XYChart_Data	*pXYChartData;
    int							nIndex;

    if(strSplitValue.isEmpty())
    {
        // First line = column names
        if(!clXYChartList.m_strSplitField.isEmpty())
            fprintf(hDest, "%s,", clXYChartList.m_strSplitField.toLatin1().constData());
        fprintf(hDest, "%s,", clXYChartList.m_strCumulField.toLatin1().constData());
        if(clXYChartList.m_strBinList.toLower() == "pass")
            fprintf(hDest, "Total Parts,Total Good,Yield (%%)\n");
        else
            fprintf(hDest, "Total Parts,Total Matching Parts,Yield (%%)\n");
        // Data
//        for(pXYChartData=clXYChartList.first(); pXYChartData; pXYChartData=clXYChartList.next())
        foreach(pXYChartData, clXYChartList)
        {
            for(nIndex=0; nIndex<pXYChartData->m_nNbDataPoints; nIndex++)
            {
                if(!clXYChartList.m_strSplitField.isEmpty())
                    fprintf(hDest, "%s,", pXYChartData->m_strSplitValue.toLatin1().constData());
                fprintf(hDest, "%s,", pXYChartData->m_pcXAxisLabels[nIndex]);
                fprintf(hDest, "%s,", QString::number((int)(pXYChartData->m_plfData_1[nIndex])).toLatin1().constData());
                fprintf(hDest, "%s,", QString::number((int)(pXYChartData->m_plfData_2[nIndex])).toLatin1().constData());
                fprintf(hDest, "%s\n", QString::number(pXYChartData->m_plfData[nIndex], 'f', 2).toLatin1().constData());
            }
        }
    }
    else
    {
        // First line = column names
        fprintf(hDest, "%s,", clXYChartList.m_strCumulField.toLatin1().constData());
        if(clXYChartList.m_strBinList.toLower() == "pass")
            fprintf(hDest, "Total Parts,Total Good,Yield (%%)\n");
        else
            fprintf(hDest, "Total Parts,Total Matching Parts,Yield (%%)\n");
        // Data
//        for(pXYChartData=clXYChartList.first(); pXYChartData; pXYChartData=clXYChartList.next())
        foreach(pXYChartData, clXYChartList)
        {
            if(pXYChartData->m_strSplitValue == strSplitValue)
            {
                for(nIndex=0; nIndex<pXYChartData->m_nNbDataPoints; nIndex++)
                {
                    fprintf(hDest, "%s,", pXYChartData->m_pcXAxisLabels[nIndex]);
                    fprintf(hDest, "%s,", QString::number((int)(pXYChartData->m_plfData_1[nIndex])).toLatin1().constData());
                    fprintf(hDest, "%s,", QString::number((int)(pXYChartData->m_plfData_2[nIndex])).toLatin1().constData());
                    fprintf(hDest, "%s\n", QString::number(pXYChartData->m_plfData[nIndex], 'f', 2).toLatin1().constData());
                }
                break;
            }
        }
    }

    // Close file
    fclose(hDest);
}

///////////////////////////////////////////////////////////
// Save PROD - Advanced Yield data into clipboard, ready to paste to spreadsheet file (Windows only)
///////////////////////////////////////////////////////////
#ifdef _WIN32
void CGexReport::Enterprise_Prod_Yield_Advanced_ExportToExcelClipboard(const QString & strGexHtmlToolbarAction)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_EVALUATION)
    {
        // Evaluation mode, refuse to run this function!
        GS::Gex::Message::information(
            "", "This function is disabled in Evaluation mode\n\nContact " +
            QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }

    // Extract data, and generate buffer for the clipboard
    QString							strBigLine, strMessage;
    QString							strToken, strSplitValue;
    GexDbPlugin_ER_Parts_Graph		*pGraph;
    GexDbPlugin_ER_Parts_Layer		*pLayer;
    GexDbPlugin_ER_Parts_SerieDef	*pSerieDef;
    GexDbPlugin_ER_Parts_SerieData	*pSerieData;
    QStringList::iterator			it;

    // Get split
    strToken = strGexHtmlToolbarAction.section("--",4,4);
    if(strToken.toLower().startsWith("split="))
        strSplitValue = strToken.section("=", 1, 1);

    // Go through all graphs, and find the good one
//    pGraph = clER_PartsData.first();
//    while(pGraph)
    foreach(pGraph, clER_PartsData)
    {
        // If a split specified, make sure we are on the right graph
        if(strSplitValue.isEmpty() || (strSplitValue == pGraph->m_strlGraphSplitValues.join("_")))
        {
            // Graph found, fill clipboard buffer
            // First line = column names
            for(it=clER_PartsData.m_strlFields_GraphSplit.begin(); it!=clER_PartsData.m_strlFields_GraphSplit.end(); it++)
                strBigLine += (*it) + "\t";
            for(it=clER_PartsData.m_strlFields_LayerSplit.begin(); it!=clER_PartsData.m_strlFields_LayerSplit.end(); it++)
                strBigLine += (*it) + "\t";
            strBigLine += clER_PartsData.m_strField_Aggregate + "\t";
            strBigLine += "Total Parts\t";
//            pSerieDef = clER_PartsData.m_plistSerieDefs.first();
//            while(pSerieDef)
            foreach(pSerieDef, clER_PartsData.m_plistSerieDefs)
            {
                if(pSerieDef->m_strTableData.toLower().indexOf("yield") != -1)
                    strBigLine += pSerieDef->m_strSerieName + " (%)\t";
                if(pSerieDef->m_strTableData.toLower().indexOf("volume") != -1)
                    strBigLine += pSerieDef->m_strSerieName + " (#)\t";
//                pSerieDef = clER_PartsData.m_plistSerieDefs.next();
            }
            strBigLine += "\n";

            // Data
            unsigned int	uiDataIndex, uiNbTotalParts , uiNbSerieParts;
            double			lfData;
//            pLayer = pGraph->first();
//            while(pLayer)
            foreach(pLayer, *pGraph)
            {
                for(uiDataIndex=0; uiDataIndex<pLayer->m_uiDataPoints; uiDataIndex++)
                {
                    uiNbTotalParts = pLayer->m_uilNbParts[uiDataIndex];

                    for(it=pGraph->m_strlGraphSplitValues.begin(); it!=pGraph->m_strlGraphSplitValues.end(); it++)
                        strBigLine += (*it) + "\t";
                    for(it=pLayer->m_strlLayerSplitValues.begin(); it!=pLayer->m_strlLayerSplitValues.end(); it++)
                        strBigLine += (*it) + "\t";
                    strBigLine += pLayer->m_strlAggregateLabels[uiDataIndex] + "\t";
                    strBigLine += QString::number(uiNbTotalParts) + "\t";

//                    pSerieData = pLayer->first();
//                    pSerieDef = clER_PartsData.m_plistSerieDefs.first();
//                    while(pSerieData)
                    int indexSerieDef(0);
                    foreach(pSerieData, *pLayer)
                    {
                        pSerieDef = clER_PartsData.m_plistSerieDefs[indexSerieDef];
                        if (!pSerieData || !pSerieDef)
                            break;
                        uiNbSerieParts = (*pSerieData)[uiDataIndex].m_uiMatchingParts;
                        lfData = ((double)uiNbSerieParts/(double)uiNbTotalParts)*100.0;
                        if(pSerieDef->m_strTableData.toLower().indexOf("yield") != -1)
                            strBigLine += QString::number(lfData, 'g', 4) + "\t";
                        if(pSerieDef->m_strTableData.toLower().indexOf("volume") != -1)
                            strBigLine += QString::number(uiNbSerieParts) + "\t";

//                        pSerieData = pLayer->next();
                        indexSerieDef++;
                    }

                    strBigLine += "\n";
                }

//                pLayer = pGraph->next();
            }

            // Graph found, exit the loop
            break;
        }
//        pGraph = clER_PartsData.next();
    }

    // Copy data to clipboard
    QClipboard *cb = QGuiApplication::clipboard();
    cb->setText(strBigLine,QClipboard::Clipboard );

    strMessage = "Table saved to clipboard in Spreadsheet format,\nready to paste!\nLaunch Spreadsheet application now?";
    bool lOk;
    GS::Gex::Message::request("", strMessage, lOk);
    if (! lOk)
    {
        return;
    }

//#ifdef _WIN32
        // Launch spreadsheet editor
    ShellExecuteA(NULL, "open",
                  m_pReportOptions->GetOption("preferences", "ssheet_editor").toString().toLatin1().constData(),
                  NULL, NULL, SW_SHOWNORMAL);
//#else
//	system(pReportOptions->GetOption("preferences","ssheet_editor").toString().toLatin1().constData());	//system(pReportOptions->m_PrefMap["ssheet_editor"].toLatin1().constData());
//#endif
}
#else
void CGexReport::Enterprise_Prod_Yield_Advanced_ExportToExcelClipboard(const QString& /*strGexHtmlToolbarAction*/)
{
}
#endif

///////////////////////////////////////////////////////////
// Save PROD - Advanced Yield data into CSV file
///////////////////////////////////////////////////////////
void CGexReport::Enterprise_Prod_Yield_Advanced_ExportToExcelFile(const QString & strGexHtmlToolbarAction)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_EVALUATION)
    {
        // Evaluation mode, refuse to run this function!
        GS::Gex::Message::information(
            "", "This function is disabled in Evaluation mode\n\nContact " +
            QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }

    // Extract data, and generate buffer for the clipboard
    QString		strToken, strSplitValue;

    // Get split
    strToken = strGexHtmlToolbarAction.section("--",4,4);
    if(strToken.toLower().startsWith("split="))
        strSplitValue = strToken.section("=", 1, 1);

    // Build file name
    QString strFileName;
    strFileName = "prouction_yield_adv_report";
    if(!strSplitValue.isEmpty())
        strFileName += "_" + strSplitValue;
    strFileName += ".csv";

    // Let's user tell where to save the configuration.
    strFileName = QFileDialog::getSaveFileName(pGexMainWindow, "Save Production Yield Report as...",
                                               strFileName, "Prouction Yield Report (*.csv)");

    // If no file selected, ignore command.
    if(strFileName.isEmpty())
        return;

    // Open CSV file file, and write WYR data!
    FILE	*hDest;

    // Check if destination exists...
    hDest = fopen(strFileName.toLatin1().constData(),"r");
    if(hDest != NULL)
    {
        // File already exist...ask if overwrite!
        fclose(hDest);

        int i=QMessageBox::warning(pGexMainWindow, GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
            "File already exists. Overwrite it?",
            "No",
            "Yes", 0, 0, 1 );
        if(i != 1)
            return;
    }

    // Open file for writing
    hDest = fopen(strFileName.toLatin1().constData(),"w");
    if(hDest == NULL)
    {
        GS::Gex::Message::critical("",
                                   GEX_T("ERROR: Failed creating CSV file..."));
        return;	// Failed writing to destination
    }

    // Extract data, and write to CSV file
    GexDbPlugin_ER_Parts_Graph		*pGraph;
    GexDbPlugin_ER_Parts_Layer		*pLayer;
    GexDbPlugin_ER_Parts_SerieDef	*pSerieDef;
    GexDbPlugin_ER_Parts_SerieData	*pSerieData;
    QStringList::iterator			it;

    // Get split
    strToken = strGexHtmlToolbarAction.section("--",4,4);
    if(strToken.toLower().startsWith("split="))
        strSplitValue = strToken.section("=", 1, 1);

    // Go through all graphs, and find the good one
//    pGraph = clER_PartsData.first();
//    while(pGraph)
    foreach(pGraph, clER_PartsData)
    {
        // If a split specified, make sure we are on the right graph
        if(strSplitValue.isEmpty() || (strSplitValue == pGraph->m_strlGraphSplitValues.join("_")))
        {
            // Graph found, write to CSV file
            // First line = column names
            for(it=clER_PartsData.m_strlFields_GraphSplit.begin(); it!=clER_PartsData.m_strlFields_GraphSplit.end(); it++)
                fprintf(hDest, "%s,", (*it).toLatin1().constData());
            for(it=clER_PartsData.m_strlFields_LayerSplit.begin(); it!=clER_PartsData.m_strlFields_LayerSplit.end(); it++)
                fprintf(hDest, "%s,", (*it).toLatin1().constData());
            fprintf(hDest, "%s,", clER_PartsData.m_strField_Aggregate.toLatin1().constData());
            fprintf(hDest, "Total Parts,");
//            pSerieDef = clER_PartsData.m_plistSerieDefs.first();
//            while(pSerieDef)
            foreach(pSerieDef, clER_PartsData.m_plistSerieDefs)
            {
                if(pSerieDef->m_strTableData.toLower().indexOf("yield") != -1)
                    fprintf(hDest, "%s (%%),", pSerieDef->m_strSerieName.toLatin1().constData());
                if(pSerieDef->m_strTableData.toLower().indexOf("volume") != -1)
                    fprintf(hDest, "%s (#),", pSerieDef->m_strSerieName.toLatin1().constData());
//                pSerieDef = clER_PartsData.m_plistSerieDefs.next();
            }
            fprintf(hDest, "\n");

            // Data
            unsigned int	uiDataIndex, uiNbTotalParts , uiNbSerieParts;
            double			lfData;
//            pLayer = pGraph->first();
//            while(pLayer)
            foreach(pLayer, *pGraph)
            {
                for(uiDataIndex=0; uiDataIndex<pLayer->m_uiDataPoints; uiDataIndex++)
                {
                    uiNbTotalParts = pLayer->m_uilNbParts[uiDataIndex];

                    for(it=pGraph->m_strlGraphSplitValues.begin(); it!=pGraph->m_strlGraphSplitValues.end(); it++)
                        fprintf(hDest, "%s,", (*it).toLatin1().constData());
                    for(it=pLayer->m_strlLayerSplitValues.begin(); it!=pLayer->m_strlLayerSplitValues.end(); it++)
                        fprintf(hDest, "%s,", (*it).toLatin1().constData());
                    fprintf(hDest, "%s,", pLayer->m_strlAggregateLabels[uiDataIndex].toLatin1().constData());
                    fprintf(hDest, "%s,", QString::number(uiNbTotalParts).toLatin1().constData());

//                    pSerieData = pLayer->first();
//                    pSerieDef = clER_PartsData.m_plistSerieDefs.first();
//                    while(pSerieData)
                    int indexSerieDef = 0;
                    foreach(pSerieData, *pLayer)
                    {
                        pSerieDef = clER_PartsData.m_plistSerieDefs[indexSerieDef];
                        if (!pSerieData || !pSerieDef)
                            break;
                        uiNbSerieParts = (*pSerieData)[uiDataIndex].m_uiMatchingParts;
                        lfData = ((double)uiNbSerieParts/(double)uiNbTotalParts)*100.0;
                        if(pSerieDef->m_strTableData.toLower().indexOf("yield") != -1)
                            fprintf(hDest, "%s,", QString::number(lfData, 'g', 4).toLatin1().constData());
                        if(pSerieDef->m_strTableData.toLower().indexOf("volume") != -1)
                            fprintf(hDest, "%s,", QString::number(uiNbSerieParts).toLatin1().constData());

//                        pSerieData = pLayer->next();
                        indexSerieDef++;
                    }

                    fprintf(hDest, "\n");
                }

//                pLayer = pGraph->next();
            }

            // Graph found, exit the loop
            break;
        }
//        pGraph = clER_PartsData.next();
    }

    // Close file
    fclose(hDest);
}

