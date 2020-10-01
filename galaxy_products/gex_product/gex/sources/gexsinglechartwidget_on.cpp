///////////////////////////////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////////////////////////////
#include "gexsinglechartwidget.h"
#include "gexhistogramchart.h"
#include "gextrendchart.h"
#include "gexboxplotchart.h"
#include "gexprobabilityplotchart.h"
#include "gexscatterchart.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "gex_report.h"
#include "browser_dialog.h"
#include "drill_chart.h"
#include "drill_editchart_options.h"
#include "engine.h"
#include "gexinputdialog.h"
#include "scripting_io.h"
#include "settings_dialog.h"
#include "temporary_files_manager.h"
#include <gqtl_log.h>
#include "csl/csl_engine.h"
#include "message.h"
#include <gqtl_global.h>

#include <QClipboard>
#include <QPainter>
#include <QMessageBox>
#include <QJsonValue>

//////////////////////////////////////////////////////////////////////////////////
// External object
///////////////////////////////////////////////////////////////////////////////////
extern CGexReport *		gexReport;
extern GexMainwindow *	pGexMainWindow;
extern void				ConvertToScriptString(QString &strFile);	// in script_wizard.h
extern double			ScalingPower(int iPower);

// report_build.cpp
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)

///////////////////////////////////////////////////////////////////////////////////
// External definitions for pointers to pixmaps.
///////////////////////////////////////////////////////////////////////////////////
#include "gex_pixmap_extern.h"
#include "script_wizard.h"
//#include "options_dialog.h"

///////////////////////////////////////////////////////////////////////////////////
// method for creating the datasetconfig .xml files for each files and writing the gexFile(...) commands in the given csl
// todo : move me in another class (CGexGroupOfFiles, ... ?)
// hFile is the CSL file
// gof is the group of file to scan
// eFilterType is the filter type for splitting
// xml_fn_pattern is the xml filename pattern without extension (for example : ".datasetconfig_" )
// group_id_name is the name of the group id
// CTest, to get the testNumber, that be used to split the data
// str_xml_output_path : where to save the xmls ?
// return false if error
bool GexSingleChartWidget::WriteSplitGexFileInCSL(FILE *hFile,
    CGexGroupOfFiles* gof,
    CGexPartFilter::filterType eFilterType,
    const QString& xml_fn_pattern,
    QString group_id_name,
    CTest* ptTest,
        QString str_xml_output_path,double dLow,double dHigh)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("in %1").arg(group_id_name).toLatin1().constData());
    if (!gof)
        return false;

    // To do : create this map at gex initialization as, for example, a static of CGexEngine.
    QMap<QString, int> ProcessBinsMap;
    ProcessBinsMap["all"]=GEX_PROCESSPART_ALL;
    ProcessBinsMap["allparts_except"]=GEX_PROCESSPART_EXPARTLIST;
    ProcessBinsMap["good"]=GEX_PROCESSPART_GOOD;
    ProcessBinsMap["fails"]=GEX_PROCESSPART_FAIL;
    ProcessBinsMap["parts"]=GEX_PROCESSPART_PARTLIST;
    ProcessBinsMap["bins"]=GEX_PROCESSPART_SBINLIST;
    ProcessBinsMap["allbins_except"]=GEX_PROCESSPART_EXSBINLIST;
    ProcessBinsMap["hbins"]=GEX_PROCESSPART_HBINLIST;
    ProcessBinsMap["allhbins_except"]=GEX_PROCESSPART_EXHBINLIST;
    ProcessBinsMap["odd_parts"]=GEX_PROCESSPART_ODD;
    ProcessBinsMap["even_parts"]=GEX_PROCESSPART_EVEN;
    ProcessBinsMap["first_instance"]=GEX_PROCESSPART_FIRSTINSTANCE;
    ProcessBinsMap["last_instance"]=GEX_PROCESSPART_LASTINSTANCE;
    ProcessBinsMap["parts_inside"]=GEX_PROCESSPART_PARTSINSIDE;
    ProcessBinsMap["parts_outside"]=GEX_PROCESSPART_PARTSOUTSIDE;

    QString strFileName(""), strProcessSite("All"), strProcessPart("all"), strProcessList(""), strRangeList(""); //0 to 2147483647
    QString strExtratWafer(""), strTemp(""), strDatasetName("");
    for (int i=0; i<gof->pFilesList.count(); i++)
    {
        // retrieve file
        CGexFileInGroup *f=gof->pFilesList.at(i); GEX_ASSERT(f);
        // file source
        strFileName=f->strFileName;
        ConvertToScriptString(strFileName);
        // site
        if (f->iProcessSite==-1)
            strProcessSite="All";
        else
            strProcessSite=QString("%1").arg(f->iProcessSite);
        // process part
        strProcessPart=ProcessBinsMap.key(f->GetProcessBins());
        // range list
        switch(f->GetProcessBins())
        {
            case GEX_PROCESSPART_EXPARTLIST: case GEX_PROCESSPART_PARTLIST: case GEX_PROCESSPART_SBINLIST:
            case GEX_PROCESSPART_EXSBINLIST:	case GEX_PROCESSPART_HBINLIST: case GEX_PROCESSPART_EXHBINLIST:
            case GEX_PROCESSPART_PARTSINSIDE: case GEX_PROCESSPART_PARTSOUTSIDE:
                // A range of parts-bins must be specified in CGexFileInGroup
                if ( f->strRangeList == "")
                {
                    GSLOG(SYSLOG_SEV_ERROR, "Write split GexFile in CSL : missing  RangeList !");
                    return false;
                }
                strRangeList = f->strRangeList; //szRangeList = (char*) csl->get("range").constBuffer();
                if (f->GetProcessBins() == GEX_PROCESSPART_PARTSINSIDE || f->GetProcessBins() == GEX_PROCESSPART_PARTSOUTSIDE)
                {
                    if (CGexRangeCoord(strRangeList).isValid() == false)
                    {
                        GSLOG(SYSLOG_SEV_ERROR, QString("Write Split GexFile In CSL: invalid range value : '%1'").arg( strRangeList).toLatin1().constData() );
                        return false;
                    }
                }
                break;
            default: // Allow any test#
                strRangeList = ""; //szRangeList = "0 to 2147483647";	// Test is a 31bits number.
                break;
        }

        //extractwafer
        strExtratWafer = f->strWaferToExtract;

        // temperature
        if (f->m_lfTemperature != -1000 )
            strTemp.append(QString("%1").arg(f->m_lfTemperature));

        // datasetname
        strDatasetName=f->strDatasetName;
        // datasetconfig
        // Split value must be written with normalized units
                if(eFilterType == CGexPartFilter::filterInside || eFilterType == CGexPartFilter::filterOutside){
                    if(dLow == C_INFINITE || dHigh == C_INFINITE)
                        return false;
                    f->addPartFilterToDatasetConfig(ptTest->lTestNumber, dLow * ScalingPower(ptTest->res_scal), dHigh * ScalingPower(ptTest->res_scal), eFilterType);
                }
                else
                    f->addPartFilterToDatasetConfig(ptTest->lTestNumber, m_pChartMenuInfo->m_dSplitValue * ScalingPower(ptTest->res_scal), eFilterType);

        QString dsc_fn("");
        dsc_fn.append(str_xml_output_path);
        dsc_fn.append(QString(xml_fn_pattern).append( QString("%1.xml").arg(i)) );
        ConvertToScriptString(dsc_fn);
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("writing %1 ").arg(dsc_fn).toLatin1().constData() );
        f->writeDatasetConfig(dsc_fn);
        GS::Gex::Engine::GetInstance().GetTempFilesManager().addFile(dsc_fn, TemporaryFile::BasicCheck);
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("adding file %1 ").arg(f->strFileName).toLatin1().data() );
        QString s;
        s.append("  gexFile(");
        s.append(group_id_name);	// const group_id,
        s.append(",	'insert', '");	// const action, : 'insert',....
        s.append( strFileName ); s += "', '";	 //	const file,
        s.append( strProcessSite ); s += "', '"; //	const site, : 'All' for all sites else the site number
        s.append( strProcessPart ); s += "', '"; //	const process, : 'all' for all type of parts, else
        // optional params [const range, const maptests, const extractwafer, const temperature, const datasetname]
        s.append( strRangeList ); s += "', '";
        s.append( dsc_fn ); s += "', '";		// Map test
        s.append( strExtratWafer ); s += "', '";
        s.append( strTemp ); s += "', '";	// temperature
        s.append( strDatasetName ); s += "');\n\n";
                fprintf(hFile, "%s", s.toLatin1().constData() );
    }
    return true;
}

#define ERROR_MESSAGE "Error while writing split csl script.\n Please contact Quantix support."

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void onSplitDataset()
//
// Description	:	on split action in contextual menu
//
///////////////////////////////////////////////////////////////////////////////////
void	GexSingleChartWidget::onSplitDataset()
{
    double	lfCustomScaleFactor;

    QString strUnitsX = m_pAbstractChart->referenceTestX()->GetScaledUnits(&lfCustomScaleFactor, ReportOptions.GetOption("dataprocessing","scaling").toString());

    strUnitsX.truncate(10);

    GSLOG(SYSLOG_SEV_DEBUG, QString(" x=%1 y=%2 unitsX=%3 CharType=%4 %5 %6")
            .arg(m_pAbstractChart->xCursorValue())
            .arg(m_pAbstractChart->yCursorValue())
            .arg(strUnitsX) //.arg(strUnitsY)
            .arg(m_nChartType)
            .arg(this->abstractChart()->referenceTestX()->lTestNumber)
            .arg(this->abstractChart()->referenceTestY()->lTestNumber)
            .toLatin1().constData()
            );

    if (!m_pChartMenuInfo || !m_pChartMenuInfo->actionTriggered())
    {
        GSLOG(SYSLOG_SEV_ERROR, "on Split Dataset : problem with ChartMenuInfo ");
        return;
    }

    CTest *	ptTest = abstractChart()->referenceTestX();

    switch(abstractChart()->type())
    {
        case GexAbstractChart::chartTypeHistogram:
        case GexAbstractChart::chartTypeProbabilityPlot:
            m_pChartMenuInfo->m_dSplitValue=m_pAbstractChart->xCursorValue();
            break;

        case GexAbstractChart::chartTypeTrend:
        case GexAbstractChart::chartTypeBoxPlot:
            m_pChartMenuInfo->m_dSplitValue=m_pAbstractChart->yCursorValue(); break;
            break;

        case GexAbstractChart::chartTypeScatter:	// correlation/bivariate : no split
        case GexAbstractChart::chartTypeBoxWhisker:
            GSLOG(SYSLOG_SEV_ERROR, " chartType = ScatterPlot : no split available. ");
            break;

        default							:
            GEX_ASSERT(false);
            break;
    }

    // Dialog to confirm/adjust
    QString strString	= m_pChartMenuInfo->actionTriggered()->text() + " ?";
    GexInputDialog		inputDialog("Confirm split", strString);
    inputDialog.setModal(true);
    GexDoubleInputItem	inputItemSplitValue( m_pChartMenuInfo->m_dSplitValue, QString());
    inputItemSplitValue.setDecimals(qMax(2, ptTest->findoutSmartScalingFactor() - ptTest->res_scal + 3));
    inputItemSplitValue.setSingleStep(GS_POW(10.0, -(ptTest->findoutSmartScalingFactor() - ptTest->res_scal)));

    inputDialog.addInputItem(&inputItemSplitValue);
    if (inputDialog.exec() == 0)
        return; // canceled...

    m_pChartMenuInfo->m_dSplitValue=inputItemSplitValue.value().toDouble();

    // writing csl
    QString strScriptFile = GS::Gex::Engine::GetInstance().GetAssistantScript();
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" spliting at %1, opening %2").arg(m_pChartMenuInfo->m_dSplitValue).arg(strScriptFile).toLatin1().constData());
    FILE *hFile = fopen(strScriptFile.toLatin1().constData(),"w");
    if (hFile == NULL)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("on Split Dataset : unable to open/write '%1' !").arg(strScriptFile)
               .toLatin1().constData());
        QString strErrMessage = "Failed to create script file:\n" + strScriptFile;
        strErrMessage += "\n\nPossible cause: Read/Write access issue.";
        GS::Gex::Message::information("", strErrMessage);
        return;
    }

    // Creates 'Preferences' section
    pGexMainWindow->pWizardScripting->WritePreferencesSection(hFile);

    // Creates 'SetOptions' section
    if(!ReportOptions.WriteOptionSectionToFile(hFile))
    {
        GEX_ASSERT(false);
        GSLOG(SYSLOG_SEV_ERROR, "on Split Dataset : can't write option section" );
        QString strErrMessage = QString("Failed to write option section. \n");
        GS::Gex::Message::information("", strErrMessage);
        return;
    }

    // Creates 'SetReportType' section.
    QString strStartupPage;
    pGexMainWindow->pWizardSettings->WriteScriptReportSettingsSection(hFile, false, strStartupPage);

    // Creates 'SetProcessData' section
    fprintf(hFile,"SetProcessData()\n");
    fprintf(hFile,"{\n");
    fprintf(hFile,"\n  sysLog('* SetProcessData: ... *');\n");
    fprintf(hFile,"  var under_group_id;\n");
    fprintf(hFile,"  var over_group_id;\n\n");
    fprintf(hFile,"  gexGroup('reset','all');\n\n");

    CGexGroupOfFiles* gof = (gexReport->getGroupsList().isEmpty()) ? NULL : gexReport->getGroupsList().at(0);
    GEX_ASSERT(gof);

    // To Do : find the paths ReportOptions.strReportDirectory and ReportOptions.strOutputPath;
    QString str_xml_output_path=GS::Gex::Engine::GetInstance().Get("UserFolder").toString();	// see main.cpp

    GSLOG(SYSLOG_SEV_DEBUG, QString("%1 groups found. First group has %2 files. Will write xml in %3...")
        .arg(gexReport->getGroupsList().count()).arg(gof->pFilesList.count())
        .arg( str_xml_output_path )
        .toLatin1().constData() );

    fprintf(hFile,"  under_group_id = gexGroup('insert','DataSet_under');\n");
    fprintf(hFile,"  over_group_id = gexGroup('insert','DataSet_over');\n");

    // ToDo : move the the temp xml datasets names to gexconstants.h ?

    fprintf(hFile,"  sysLog('* SetProcessData: creating over group ... *');\n");
    if (!WriteSplitGexFileInCSL(hFile, gof, CGexPartFilter::filterOver, "/.split_over_", "over_group_id", ptTest, str_xml_output_path))
    {
        GSLOG(SYSLOG_SEV_ERROR, "on Split Dataset : WriteSplitGexFileInCSL failed");
        GS::Gex::Message::information("", ERROR_MESSAGE);
        return;
    }
    fprintf(hFile,"\n  sysLog('* SetProcessData: creating under group ... *');\n");
    if (!WriteSplitGexFileInCSL(hFile, gof, CGexPartFilter::filterUnder, "/.split_under_", "under_group_id", ptTest, str_xml_output_path))
    {
        GSLOG(SYSLOG_SEV_ERROR, "on Split Dataset : WriteSplitGexFileInCSL failed.");
        GS::Gex::Message::information("", ERROR_MESSAGE);
        return;
    }
    fprintf(hFile,"\n  sysLog('* SetProcessData: ok. *');\n");
    fprintf(hFile,"}\n\n");	// end of SetProcessData

    // Creates 'main' section
    fprintf(hFile,"main()\n");
    fprintf(hFile,"{\n");
    fprintf(hFile,"\n  sysLog('* main : *');\n");
    // ToDo : Nowadays, Gex will write only the latest version of the csl format.
    //fprintf(hFile,"  gexCslVersion('%1.2f');\n\n", GEX_MAX_CSL_VERSION);
    fprintf(hFile,"  SetOptions();\n\n");
    fprintf(hFile,"  SetProcessData();\n");

    // Setup the GEX Report type (Settings)
    fprintf(hFile,"  SetReportType();");

    fprintf(hFile,"  gexBuildReport('home','0');\n");	// Show report home page
    // Now that custom report is created, make sure we reload standard Examinator settings
    fprintf(hFile,"\n");
    fprintf(hFile,"  SetPreferences();\n");
    fprintf(hFile,"  SetOptions();\n");
    // Last line of script executed must be the report format otherwse it may conflict with the default options giving a different format than what the user wants.
    fprintf(hFile,"\n  sysLog('* main : ok.*');\n");
    fprintf(hFile,"}\n\n");

    fclose(hFile);

    // Set the pointer to null to avoid crash during report generation. The pointer is managed
    // by the CGexReport class and is destroyed when the report is regenerated.
    m_pAbstractChart->SetChartOverlays(NULL);

    // Greyed out the widget in order to avoid any user action
    mWizardParent->setEnabled(false);

    // Execute script.
    if (GS::Gex::CSLEngine::GetInstance().RunScript(strScriptFile).IsFailed())
    {
        GSLOG(SYSLOG_SEV_ERROR, "on Split Dataset : error while running split script");
    }

    // Enable the widget at the end of the report generation.
    mWizardParent->setEnabled(true);
    connect(gexReport, SIGNAL(interactiveFilterChanged(bool)), mWizardParent,
                  SLOT(onFilterChanged(bool)));
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void onCopyChartToClipboard()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexSingleChartWidget::onCopyChartToClipboard()
{
    // Capture Chart area into Clipboard
    QPixmap			pixmapChart(QPixmap::grabWidget(this));
    QClipboard *	lClipBoard = QGuiApplication::clipboard();
    if (!lClipBoard)
    {
        GSLOG(SYSLOG_SEV_WARNING, "failed to retrieve application clipboard");
    }
    else
    {
        lClipBoard->setPixmap(pixmapChart, QClipboard::Clipboard);
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void onSaveChartToDisk()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexSingleChartWidget::onSaveChartToDisk()
{
    // Capture Chart area into disk
    QPixmap		pixmapChart(QPixmap::grabWidget(this));
    QString		strFile = QFileDialog::getSaveFileName(this,"Save Chart as...","image","PNG (*.png)",NULL,QFileDialog::DontConfirmOverwrite);

    // If no file selected, ignore command.
    if(strFile.isEmpty() == false)
    {
        if(strFile.endsWith(".png", Qt::CaseInsensitive) == false)
            strFile += ".png";	// make sure file extension is .png

        // Check if file exists.
        QFile	file(strFile);
        bool	bCreate = true;

        if(file.exists() == true)
        {
            GS::Gex::Message::request("", "File already exists. Overwrite it ?", bCreate);
        }

        if (bCreate)
            pixmapChart.save(strFile,"PNG");
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void onDeviceResults()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexSingleChartWidget::onDeviceResults()
{
    emit deviceResults((long) m_pAbstractChart->xCursorValue()-1);
}


QString GexSingleChartWidget::GetOFRElementTypeName()
{
    QString chartTypeName;
    GexAbstractChart::chartType lTypeChart= abstractChart()->type();
    if( lTypeChart == GexAbstractChart::chartTypeHistogram)
        chartTypeName = "Histogram";
    else if( lTypeChart == GexAbstractChart::chartTypeTrend)
        chartTypeName = "Trend";
    else if(lTypeChart == GexAbstractChart::chartTypeProbabilityPlot)
        chartTypeName = "ProbabilityPlot";
    else if(lTypeChart == GexAbstractChart::chartTypeBoxPlot)
        chartTypeName = "BoxPlot";

    return chartTypeName;
}

void GexSingleChartWidget::OnAddToLastUpdatedOFRSection()
{
     mWizardParent->AddToLastUpdatedOFRSection(GetOFRElementTypeName());
}

void GexSingleChartWidget::OnAddToAnExistingOFRSection(QAction* action)
{
    QVariant lData = action->data();
    if(lData.isValid())
    {
        int lIndexSection = lData.toInt();
        mWizardParent->AddToAnExistingOFRSection(lIndexSection,  GetOFRElementTypeName());
    }
}

void GexSingleChartWidget::OnAddNewOFRSection()
{
    mWizardParent->AddNewOFRSection( GetOFRElementTypeName() );
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void onEditMarkers()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexSingleChartWidget::onEditMarkers()
{
    emit editMarkers();
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void onEditStyles()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexSingleChartWidget::onEditStyles()
{
    emit editStyles();
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void onRemoveDatapointHigher()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexSingleChartWidget::onRemoveDatapointHigher()
{
    if (m_pChartMenuInfo && m_pChartMenuInfo->actionTriggered())
    {
        // Request to confirm data removal
        QString strString	= m_pChartMenuInfo->actionTriggered()->text() + "?";

        GexDoubleInputItem	inputItemOutlier(m_pChartMenuInfo->outlierLimit(), QString());
        GexBoolInputItem	inputItemCheck(false, "Remove ALL matching parts", "Remove all parameters in all parts matching this criteria");
        GexInputDialog		inputDialog("Confirm removal", strString);

        inputDialog.setModal(true);
        inputDialog.addInputItem(&inputItemOutlier);
        inputDialog.addInputItem(&inputItemCheck);

        if (inputDialog.exec() != 0)
        {
            emit removeDatapointHigher(inputItemOutlier.value().toDouble() * m_pChartMenuInfo->m_dFactor, inputItemCheck.value().toBool());
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void onRemoveDatapointLower()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexSingleChartWidget::onRemoveDatapointLower()
{
    if (m_pChartMenuInfo && m_pChartMenuInfo->actionTriggered())
    {
        // Request to confirm data removal
        QString strString	= m_pChartMenuInfo->actionTriggered()->text() + "?";

        GexDoubleInputItem	inputItemOutlier(m_pChartMenuInfo->outlierLimit());
        GexBoolInputItem	inputItemCheck(false, "Remove ALL matching parts", "Remove all parameters in all parts matching this criteria");
        GexInputDialog		inputDialog("Confirm removal", strString);

        inputDialog.setModal(true);
        inputDialog.addInputItem(&inputItemOutlier);
        inputDialog.addInputItem(&inputItemCheck);

        if (inputDialog.exec() != 0)
            emit removeDatapointLower(inputItemOutlier.value().toDouble() * m_pChartMenuInfo->m_dFactor, inputItemCheck.value().toBool());
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void onRemoveDatapointRange()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexSingleChartWidget::onRemoveDatapointRange()
{
    if (m_pChartMenuInfo)
    {
        // Small Edit dialog box to prompt/edit the data.
        GexDoubleInputItem	inputItemFrom(m_pChartMenuInfo->outlierLimit(), "From:", "Lowest range of values to remove");
        GexDoubleInputItem	inputItemTo(m_pChartMenuInfo->outlierLimit(), "To", "Highest range of values to remove");
        GexBoolInputItem	inputItemCheck(false, "Remove ALL matching parts", "Remove all parameters in all parts matching this criteria");
        GexInputDialog		inputDialog("Remove Range of samples", "Define range of value to remove...");

        inputDialog.setModal(true);
        inputDialog.addInputItem(&inputItemFrom);
        inputDialog.addInputItem(&inputItemTo);
        inputDialog.addInputItem(&inputItemCheck);

        // Display Edit window
        if (inputDialog.exec() != 0)
      emit removeDatapointRange(
            inputItemFrom.value().toDouble() * m_pChartMenuInfo->m_dFactor,
            inputItemTo.value().toDouble() * m_pChartMenuInfo->m_dFactor,
            inputItemCheck.value().toBool());
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void onRemoveIQR()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexSingleChartWidget::onRemoveIQR()
{
    // Default is: +/-1.5*IQR
    double	dValue	= 1.5;

    GexBoolInputItem	inputItemCheck(false, "Remove ALL matching parts", "Remove all parameters in all parts matching this criteria");
    GexDoubleInputItem	inputItemIQR(dValue, QString());
    inputItemIQR.setRange(0, 100);
    inputItemIQR.setDecimals(1);

    GexInputDialog		inputDialog("N*IQR removal", "Confirm +/-N*IQR removal? Where 'N' is:");
    inputDialog.setModal(true);
    inputDialog.addInputItem(&inputItemIQR);
    inputDialog.addInputItem(&inputItemCheck);

    // Prompt user to confirm...
    if (inputDialog.exec() != 0)
    {
        dValue = fabs(inputItemIQR.value().toDouble());

        emit removeIQR(dValue, inputItemCheck.value().toBool());
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void onRemoveNSigma()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexSingleChartWidget::onRemoveNSigma()
{
    // Default is: 3*Sigma
    double	dValue	= 3.0;

    GexBoolInputItem	inputItemCheck(false, "Remove ALL matching parts", "Remove all parameters in all parts matching this criteria");
    GexDoubleInputItem	inputItemNSigma(dValue, QString());
    inputItemNSigma.setDecimals(1);

    GexInputDialog		inputDialog("N*Sigma removal", "Confirm +/-N*Sigma removal? Where 'N' is:");
    inputDialog.setModal(true);
    inputDialog.addInputItem(&inputItemNSigma);
    inputDialog.addInputItem(&inputItemCheck);

    // Prompt user to confirm...
    if (inputDialog.exec() != 0)
        emit removeNSigma(inputItemNSigma.value().toDouble(), inputItemCheck.value().toBool());
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void onActionHovered(QAction*pAction)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexSingleChartWidget::onActionHovered(QAction*pAction)
{
    if (m_pChartMenuInfo)
        m_pChartMenuInfo->setActionTriggered(pAction);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void onChartOptionChanged(GexAbstractChart::chartOption eChartOption)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexSingleChartWidget::onChartOptionChanged(GexAbstractChart::chartOption eChartOption)
{
    if (m_pAbstractChart)
        m_pAbstractChart->onOptionChanged(eChartOption);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void onEditProperties()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexSingleChartWidget::onEditProperties()
{
    // Edit Chart Title + Legends + Scales.
    EditChartOptionsDialog cOptions;
    int iType = m_pAbstractChart->chartOverlays()->getAppliedToChart();
    m_pAbstractChart->chartOverlays()->setAppliedToChart(m_pAbstractChart->type());

    cOptions.setVariables(m_pAbstractChart->chartOverlays());

    if(cOptions.exec() == 1)
    {
        // User has clicked the 'Ok' button...so update the Charting changes...
        cOptions.getVariables(m_pAbstractChart->chartOverlays());
        m_pAbstractChart->chartOverlays()->setAppliedToChart(m_pAbstractChart->type());
        m_pAbstractChart->setKeepViewport(true);
        // Force Redraw.
        repaint();
        return ;
    }
    //Restore Value
    m_pAbstractChart->chartOverlays()->setAppliedToChart(iType);
}


///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void onWafermapDatapointHigher()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexSingleChartWidget::onWafermapDatapointHigher()
{
    if (m_pChartMenuInfo && m_pChartMenuInfo->actionTriggered())
    {
        // Request to confirm data removal
        QString strString		= m_pChartMenuInfo->actionTriggered()->text() + "?";

        GexDoubleInputItem	inputItemOutlier(m_pChartMenuInfo->outlierLimit(), QString());
        GexInputDialog		inputDialog("Confirm drill-down", strString);

        inputDialog.setModal(true);
        inputDialog.addInputItem(&inputItemOutlier);

        if (inputDialog.exec() != 0)
            emit selectWafermapRange(inputItemOutlier.value().toDouble(), 1e99);
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void onWafermapDatapointLower()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexSingleChartWidget::onWafermapDatapointLower()
{
    if (m_pChartMenuInfo && m_pChartMenuInfo->actionTriggered())
    {
        // Request to confirm data removal
        QString strString		= m_pChartMenuInfo->actionTriggered()->text() + "?";

        GexDoubleInputItem	inputItemOutlier(m_pChartMenuInfo->outlierLimit(), QString());
        GexInputDialog		inputDialog("Confirm drill-down", strString);

        inputDialog.setModal(true);
        inputDialog.addInputItem(&inputItemOutlier);

        if (inputDialog.exec() != 0)
            emit selectWafermapRange(-1e99, inputItemOutlier.value().toDouble());
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void onWafermapDatapointRange()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexSingleChartWidget::onWafermapDatapointRange()
{
    if (m_pChartMenuInfo)
    {
        // Small Edit dialog box to prompt/edit the data.
        GexDoubleInputItem	inputItemFrom(m_pChartMenuInfo->outlierLimit(), "From:", "Lowest range of values to remove");
        GexDoubleInputItem	inputItemTo(m_pChartMenuInfo->outlierLimit(), "To:", "Highest range of values to remove");
        GexInputDialog		inputDialog("Wafermap drill-down", "Define parameter value range...");

        inputDialog.setModal(true);
        inputDialog.addInputItem(&inputItemFrom);
        inputDialog.addInputItem(&inputItemTo);

        if (inputDialog.exec() != 0)
        {
            double dOutlierFrom = inputItemFrom.value().toDouble();
            double dOutlierTo	= inputItemTo.value().toDouble();

            emit selectWafermapRange(dOutlierFrom, dOutlierTo);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void onWafermapDie()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexSingleChartWidget::onWafermapDie()
{
    selectCurrentDieLocation();
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void onKeepViewportChanged(bool isChecked)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexSingleChartWidget::onKeepViewportChanged(bool isChecked)
{
    if (m_pAbstractChart)
        m_pAbstractChart->setKeepViewport(isChecked);
}

///////////////////////////////////////////////////////////////////////////////////
// Name			:	void onRebuildReport_<Format>()
///////////////////////////////////////////////////////////////////////////////////
void GexSingleChartWidget::onRebuildReport_Web()
{
    // Build report now!
    emit rebuildReport("HTML");
}

void GexSingleChartWidget::onRebuildReport_Word()
{
    emit rebuildReport("DOC");
}

void GexSingleChartWidget::onRebuildReport_PPT()
{
    // Build report now!
    emit rebuildReport("PPT");
}

void GexSingleChartWidget::onRebuildReport_PDF()
{
    // Build report now!
    emit rebuildReport("PDF");
}

void GexSingleChartWidget::onRebuildReport_Excel()
{
    // Build report now!
    emit rebuildReport("CSV");
}

void	GexSingleChartWidget::onSplitDatasetZone()
{
    m_pAbstractChart->computeCursorValue(m_ptFirst);
    double dFirstX = m_pAbstractChart->xCursorValue();
    double dFirstY = m_pAbstractChart->yCursorValue();

    m_pAbstractChart->computeCursorValue(m_ptLast);
    double dLastX = m_pAbstractChart->xCursorValue();
    double dLastY = m_pAbstractChart->yCursorValue();

    double dLow = C_INFINITE, dHigh =C_INFINITE;

    double	lfCustomScaleFactor;

    QString strUnitsX = m_pAbstractChart->referenceTestX()->GetScaledUnits(&lfCustomScaleFactor, ReportOptions.GetOption("dataprocessing","scaling").toString());

    strUnitsX.truncate(10);


    CTest *	ptTest = abstractChart()->referenceTestX();

    switch(abstractChart()->type())
    {
            case GexAbstractChart::chartTypeHistogram:
            case GexAbstractChart::chartTypeProbabilityPlot:
            {
                dLow =  qMin(dFirstX, dLastX);
                dHigh = qMax(dFirstX, dLastX);
                break;
            }
            case GexAbstractChart::chartTypeTrend:
            case GexAbstractChart::chartTypeBoxPlot:
            {
                dLow = qMin(dFirstY, dLastY);
                dHigh = qMax(dFirstY, dLastY);
                break;
            }

            case GexAbstractChart::chartTypeScatter:	// correlation/bivariate : no split
            case GexAbstractChart::chartTypeBoxWhisker:
                    GSLOG(SYSLOG_SEV_ERROR, " chartType = ScatterPlot : no split available. ");
                    break;

            default							:
                    GEX_ASSERT(false);
                    break;
    }

    // writing csl
    QString strScriptFile = GS::Gex::Engine::GetInstance().GetAssistantScript();
    FILE *hFile = fopen(strScriptFile.toLatin1().constData(),"w");
    if (hFile == NULL)
    {
            GSLOG(SYSLOG_SEV_ERROR, QString("on Split Dataset : unable to open/write '%1' !").arg(strScriptFile)
                       .toLatin1().constData());
            QString strErrMessage = "Failed to create script file:\n" + strScriptFile;
            strErrMessage += "\n\nPossible cause: Read/Write access issue.";
            GS::Gex::Message::information("", strErrMessage);
            return;
    }

    // Creates 'Preferences' section
    pGexMainWindow->pWizardScripting->WritePreferencesSection(hFile);

    // Creates 'SetOptions' section
    if(!ReportOptions.WriteOptionSectionToFile(hFile))
    {
            GEX_ASSERT(false);
            GSLOG(SYSLOG_SEV_ERROR, "on Split Dataset : can't write option section" );
            QString strErrMessage = QString("Failed to write option section. \n");
            GS::Gex::Message::information("", strErrMessage);
            return;
    }

    // Creates 'SetReportType' section.
    QString strStartupPage;
    pGexMainWindow->pWizardSettings->WriteScriptReportSettingsSection(hFile, false, strStartupPage);

    // Creates 'SetProcessData' section
    fprintf(hFile,"SetProcessData()\n");
    fprintf(hFile,"{\n");
    fprintf(hFile,"\n  sysLog('* SetProcessData: ... *');\n");
    fprintf(hFile,"  var inside_group_id;\n");
    fprintf(hFile,"  var outside_group_id;\n\n");
    fprintf(hFile,"  gexGroup('reset','all');\n\n");

    CGexGroupOfFiles* gof = (gexReport->getGroupsList().isEmpty()) ? NULL : gexReport->getGroupsList().at(0);
    GEX_ASSERT(gof);

    // To Do : find the paths ReportOptions.strReportDirectory and ReportOptions.strOutputPath;
    QString str_xml_output_path=GS::Gex::Engine::GetInstance().Get("UserFolder").toString();	// see main.cpp

    GSLOG(SYSLOG_SEV_DEBUG, QString("%1 groups found. First group has %2 files. Will write xml in %3...")
            .arg(gexReport->getGroupsList().count()).arg(gof->pFilesList.count())
            .arg( str_xml_output_path )
            .toLatin1().constData() );

    fprintf(hFile,"  inside_group_id = gexGroup('insert','DataSet_inside');\n");
    fprintf(hFile,"  outside_group_id = gexGroup('insert','DataSet_outside');\n");

    // ToDo : move the the temp xml datasets names to gexconstants.h ?

    fprintf(hFile,"  sysLog('* SetProcessData: creating over group ... *');\n");
    if (!WriteSplitGexFileInCSL(hFile, gof, CGexPartFilter::filterInside, "/.split_inside_", "inside_group_id", ptTest, str_xml_output_path, dLow, dHigh))
    {
            GSLOG(SYSLOG_SEV_ERROR, "on Split Dataset : WriteSplitGexFileInCSL failed");
            GS::Gex::Message::information("", ERROR_MESSAGE);
            return;
    }
    fprintf(hFile,"\n  sysLog('* SetProcessData: creating under group ... *');\n");
    if (!WriteSplitGexFileInCSL(hFile, gof, CGexPartFilter::filterOutside, "/.split_outside_", "outside_group_id", ptTest, str_xml_output_path, dLow, dHigh))
    {
            GSLOG(SYSLOG_SEV_ERROR, "on Split Dataset : WriteSplitGexFileInCSL failed.");
            GS::Gex::Message::information("", ERROR_MESSAGE);
            return;
    }
    fprintf(hFile,"\n  sysLog('* SetProcessData: ok. *');\n");
    fprintf(hFile,"}\n\n");	// end of SetProcessData

    // Creates 'main' section
    fprintf(hFile,"main()\n");
    fprintf(hFile,"{\n");
    fprintf(hFile,"\n  sysLog('* main : *');\n");
    // ToDo : Nowadays, Gex will write only the latest version of the csl format.
    //fprintf(hFile,"  gexCslVersion('%1.2f');\n\n", GEX_MAX_CSL_VERSION);
    fprintf(hFile,"  SetOptions();\n\n");
    fprintf(hFile,"  SetProcessData();\n");

    // Setup the GEX Report type (Settings)
    fprintf(hFile,"  SetReportType();");

    fprintf(hFile,"  gexBuildReport('home','0');\n");	// Show report home page
    // Now that custom report is created, make sure we reload standard Examinator settings
    fprintf(hFile,"\n");
    fprintf(hFile,"  SetPreferences();\n");
    fprintf(hFile,"  SetOptions();\n");
    // Last line of script executed must be the report format otherwse it may conflict with the default options giving a different format than what the user wants.
    fprintf(hFile,"\n  sysLog('* main : ok.*');\n");
    fprintf(hFile,"}\n\n");

    fclose(hFile);

    // Set the pointer to null to avoid crash during report generation. The pointer is managed
    // by the CGexReport class and is destroyed when the report is regenerated.
    m_pAbstractChart->SetChartOverlays(NULL);

    // Greyed out the widget in order to avoid any user action
     mWizardParent->SetEnabledCharts(false);
     //mWizardParent->m_pMultiChartWidget->setEnabled(false);
    // Execute script.
    if (GS::Gex::CSLEngine::GetInstance().RunScript(strScriptFile).IsFailed())
    {
        GSLOG(SYSLOG_SEV_ERROR, "on Split Dataset : error while running split script");
    }

    // Enable the widget at the end of the report generation.
     //mWizardParent->m_pMultiChartWidget->setEnabled(true);
     mWizardParent->SetEnabledCharts(true);
}

void GexSingleChartWidget::onSelectScaterArea(){
    m_eMouseMode = mouseZoneSelect;
    setMouseMode(mouseZoneSelect);

    if(m_pRubberBand)
        delete m_pRubberBand;
    m_pRubberBand = new QRubberBand(QRubberBand::Rectangle, this);
    m_pRubberBand->hide();
    m_pRubberBand->setGeometry(QRect());
    update( QRegion (m_pAbstractChart->leftMargin(), m_pAbstractChart->topMargin(),
                     m_pAbstractChart->width() - m_pAbstractChart->horizontalMargin(),
                     m_pAbstractChart->height() - m_pAbstractChart->verticalMargin()));
}

