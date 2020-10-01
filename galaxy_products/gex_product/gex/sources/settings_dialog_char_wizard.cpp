///////////////////////////////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////////////////////////////
#include <gqtl_log.h>

#include "settings_dialog.h"
#include "browser_dialog.h"
#include "settings_sql.h"
#include "picktest_dialog.h"
#include "charac_line_chart_settings_dialog.h"
#include "charac_box_whisker_template_dialog.h"
#include "db_onequery_wizard.h"
#include "report_options.h"

// External definitions for pointers to pixmaps.
#include "gex_pixmap_extern.h"

///////////////////////////////////////////////////////////////////////////////////
// External objects
///////////////////////////////////////////////////////////////////////////////////
extern GexMainwindow *	pGexMainWindow;
extern CReportOptions	ReportOptions;

///////////////////////////////////////////////////////////
// Settings switched to : Create Instant Report
///////////////////////////////////////////////////////////
void GexSettings::OnDoCharacterizationReport(void)
{
    // Keep track of Module menu selected
    if(pGexMainWindow != NULL)
        pGexMainWindow->iGexAssistantSelected = GEX_MODULE_INSTANT_REPORT;

    if (mCharacLineChartTemplate)
    {
        delete mCharacLineChartTemplate;
        mCharacLineChartTemplate = NULL;
    }

    if (mCharacBoxWhiskerTemplate)
    {
        delete mCharacBoxWhiskerTemplate;
        mCharacBoxWhiskerTemplate = NULL;
    }

    // Show relevant report GUI
    widgetStackReport->setCurrentIndex(WIDGET_REPORT_CHAR_WIZARD);

    if(!buttonBuildReport->isEnabled())
        buttonBuildReport->setEnabled(true);
}

///////////////////////////////////////////////////////////
// Initialize Char Wizard Report page
///////////////////////////////////////////////////////////
void GexSettings::InitCharWizardPage(void)
{
    // Fill list of templates
    comboCharWizardTemplate->clear();
    comboCharWizardTemplate->addItem(*pixShift, "Box-whisker chart report", QVariant(GEX_ADV_CHARAC_BOXWHISKER_CHART));
    comboCharWizardTemplate->addItem(*pixShift, "Line chart report", QVariant(GEX_ADV_CHARAC_LINE_CHART));
    comboCharWizardTemplate->setCurrentIndex(comboCharWizardTemplate->findData(QVariant(GEX_ADV_CHARAC_BOXWHISKER_CHART)));

    // Hide settings button
    toolButtonCharSettings->setVisible(true);

    // Fill list of viewport
    comboCharWizardViewport->clear();
    comboCharWizardViewport->addItem("Chart over test limits", QVariant(GEX_ADV_CHARAC_CHART_OVERLIMITS));
    comboCharWizardViewport->addItem("Chart over test results", QVariant(GEX_ADV_CHARAC_CHART_OVERDATA));
    comboCharWizardViewport->addItem("Adaptive: data & limits", QVariant(GEX_ADV_CHARAC_CHART_DATALIMITS));

    comboCharWizardViewport->setCurrentIndex(comboCharWizardViewport->findData(QVariant(GEX_ADV_CHARAC_CHART_OVERDATA)));

    // Fill test selection
    comboCharWizardTest->clear();
    comboCharWizardTest->addItem(ADV_ALL_TESTS, QVariant(GEX_ADV_ALLTESTS));
    comboCharWizardTest->addItem(ADV_TEST_LIST, QVariant(GEX_ADV_TESTSLIST));
    comboCharWizardTest->addItem(ADV_TOP_N_FAILTEST, QVariant(GEX_ADV_TOP_N_FAILTESTS));
    comboCharWizardTest->setCurrentIndex(comboCharWizardTest->findData(QVariant(GEX_ADV_ALLTESTS)));

    // Hide widgets
    lineEditCharWizardTest->hide();
    PickTestCharWizard->hide();

    // Connect Signal/Slots
    connect(comboCharWizardTemplate,    SIGNAL(currentIndexChanged(int)),
            this,                       SLOT(OnCharacWizardTemplateChanged(int)));
    connect(comboCharWizardTest,        SIGNAL(currentIndexChanged(int)),
            this,                       SLOT(OnCharacWizardTestChanged(int)));
    connect(PickTestCharWizard,         SIGNAL(clicked()),
            this,                       SLOT(OnPickTestFromListCharWizard()));
    connect(toolButtonCharSettings,     SIGNAL(clicked()),
            this,                       SLOT(OnCharWizardSettings()));
}

///////////////////////////////////////////////////////////
// Write Char Wizard Report settings
///////////////////////////////////////////////////////////
void GexSettings::WriteSettings_CharacterizationReport(FILE *hFile)
{
    QString lTestList;
    QString lTemplate;

    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Write Characterization settings.");

    // Force to not sort datasets in Characterization reports
    // Datasets are always sorted in the query definition
    fprintf(hFile,"  gexOptions('dataprocessing','sorting','none');");

    fprintf(hFile,"  gexReportType('stats','disabled');\n");
    fprintf(hFile,"  gexReportType('wafer','disabled');\n");
    fprintf(hFile,"  gexReportType('histogram','disabled');\n");

    fprintf(hFile,"\n  // Section: Advanced Report\n");
    fprintf(hFile,"  gexReportType('adv_my_report','disabled');\n");	// Disable template...unless template file selected later on

    int idxTemplate = comboCharWizardTemplate->currentIndex();

    if (idxTemplate != -1)
    {
        switch(comboCharWizardTemplate->itemData(idxTemplate).toInt())
        {
            case GEX_ADV_CHARAC_BOXWHISKER_CHART:
                lTemplate = "adv_charac1";
                break;

            case GEX_ADV_CHARAC_LINE_CHART:
                lTemplate = "adv_charac2";
                break;

            default:
                GSLOG(SYSLOG_SEV_WARNING, "Unknown template");
                break;
        }
    }
    else
        GSLOG(SYSLOG_SEV_WARNING, "No template selected.");

    int idxTest = comboCharWizardTest->currentIndex();

    if (idxTest != -1)
    {
        switch(comboCharWizardTest->itemData(idxTest).toInt())
        {
            case GEX_ADV_ALLTESTS:
            default:
                lTestList = "all";	// GEX_ADV_ALLTESTS:		// ALL tests
                break;

            case GEX_ADV_TESTSLIST:
                lTestList = lineEditCharWizardTest->text();  // Test#/list/range to process //GEX_ADV_TESTSLIST:	// list of tests
                break;

            case GEX_ADV_TOP_N_FAILTESTS:
                lTestList = "top "+ lineEditCharWizardTest->text()+" failtests";	// GEX_ADV_TOP_N_FAILTESTS: // Top N fail test
                break;
        }

        lTestList = lTestList.trimmed();
    }
    else
        GSLOG(SYSLOG_SEV_WARNING, "No Test mode selected.");

    int idxViewport = comboCharWizardViewport->currentIndex();

    if (idxViewport != -1)
    {
        switch(comboCharWizardViewport->itemData(idxViewport).toInt())
        {
            case GEX_ADV_CHARAC_CHART_OVERLIMITS:		// chart over test limits
            default:
                fprintf(hFile,"  gexReportType('%s','test_over_limits','%s');\n",
                        lTemplate.toLatin1().constData(),
                        lTestList.toLatin1().constData());
                break;

            case GEX_ADV_CHARAC_CHART_OVERDATA: // chart over test results
                fprintf(hFile,"  gexReportType('%s','test_over_range','%s');\n",
                        lTemplate.toLatin1().constData(),
                        lTestList.toLatin1().constData());
                break;

            case GEX_ADV_CHARAC_CHART_DATALIMITS:	// Adaptive: data + test limits visible on chart.
                fprintf(hFile,"  gexReportType('%s','adaptive','%s');\n",
                        lTemplate.toLatin1().constData(),
                        lTestList.toLatin1().constData());
                break;
        }
    }
    else
        GSLOG(SYSLOG_SEV_WARNING, "No Characterization template selected.");

    int lTemplateID = comboCharWizardTemplate->itemData(idxTemplate).toInt();

    if (lTemplateID == GEX_ADV_CHARAC_LINE_CHART && mCharacLineChartTemplate)
    {
        if (mCharacLineChartTemplate->GetVariable().isEmpty() == false)
        {
            fprintf(hFile,"  gexReportType('%s','chart','variable','%s');\n",
                    lTemplate.toLatin1().constData(),
                    mCharacLineChartTemplate->GetVariable().toLatin1().data());
        }

        if (mCharacLineChartTemplate->GetSerieConditions().count() > 0)
        {
            fprintf(hFile,"  gexReportType('%s','chart','serie_conditions','%s');\n",
                    lTemplate.toLatin1().constData(),
                    mCharacLineChartTemplate->GetSerieConditions().join("|").toLatin1().data());
        }

        for(int lIdx = 0; lIdx < mCharacLineChartTemplate->GetSerieDefinitionCount(); ++lIdx)
        {
            QString lDefinition;
            GS::Gex::CharacLineChartSerie serie = mCharacLineChartTemplate->GetSerieDefinitionAt(lIdx);

            lDefinition += "--conditions=" + serie.GetConditions().join("|") + ";";
            lDefinition += "--color=" + serie.GetColor().name() + ";";

            if (serie.GetName().isEmpty() == false)
                lDefinition += "--name=" + serie.GetName() + ";";

            if (serie.GetVariable().isEmpty() == false)
                lDefinition += "--variable=" + serie.GetVariable() + ";";

            fprintf(hFile,"  gexReportType('%s','chart','add_serie','%s');\n",
                    lTemplate.toLatin1().constData(),
                    lDefinition.toLatin1().constData());
        }
    }

    if (lTemplateID == GEX_ADV_CHARAC_BOXWHISKER_CHART && mCharacBoxWhiskerTemplate)
    {
        QStringList lAggregates = mCharacBoxWhiskerTemplate->GetTopLevelAggregates();

        for(int lIdx = 0; lIdx < lAggregates.count(); ++lIdx)
        {
            QString lDefinition;

            lDefinition += "--conditions=" + lAggregates.at(lIdx) + ";";
            lDefinition += "--color=" + mCharacBoxWhiskerTemplate->GetTopLevelColor(lAggregates.at(lIdx)).name() + ";";

            fprintf(hFile,"  gexReportType('%s','chart','top_level_aggregate','%s');\n",
                    lTemplate.toLatin1().constData(),
                    lDefinition.toLatin1().constData());
        }
    }

}

void GexSettings::OnCharacWizardTemplateChanged(int lIndex)
{
    int lTemplate = -1;

    if (lIndex != -1)
        lTemplate = comboCharWizardTemplate->itemData(lIndex).toInt();
    else
    {
        GEX_ASSERT(false);
        GSLOG(SYSLOG_SEV_WARNING, "No Characterization template selected.");
    }

    switch (lTemplate)
    {
        case GEX_ADV_CHARAC_BOXWHISKER_CHART:
        case GEX_ADV_CHARAC_LINE_CHART:
            toolButtonCharSettings->setVisible(true);
            break;

        default:
            GSLOG(SYSLOG_SEV_WARNING, "Unknown Characterization template selected.");
            toolButtonCharSettings->setVisible(false);
            break;
    }
}

///////////////////////////////////////////////////////////
// Char Wizard Test combo box changed
///////////////////////////////////////////////////////////
void GexSettings::OnCharacWizardTestChanged(int lNewIndex)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" Charac Wizard Test changed [new pos : %1]").arg(lNewIndex)
          .toLatin1().constData());

    switch(comboCharWizardTest->itemData(lNewIndex).toInt())
    {
        case GEX_ADV_ALLTESTS: // All tests
        default:
            lineEditCharWizardTest->hide();
            PickTestCharWizard->hide();
            break;

        case GEX_ADV_TESTSLIST:	// Tests list
            lineEditCharWizardTest->setText("");
            lineEditCharWizardTest->show();
            PickTestCharWizard->show();
            break;
        case GEX_ADV_TOP_N_FAILTESTS:	// Top N fail tests
            lineEditCharWizardTest->setText("5");
            lineEditCharWizardTest->show();
            PickTestCharWizard->hide();
            break;
    }
}

///////////////////////////////////////////////////////////
// Pick list of tests from list: Characacterization Wizard
///////////////////////////////////////////////////////////
void GexSettings::OnPickTestFromListCharWizard(void)
{
    QString lTestSelection = PickTestFromList(true, PickTestDialog::TestAll);

    // If valid, save the list selected into the edit field...
    if(!lTestSelection.isEmpty())
        lineEditCharWizardTest->setText(lTestSelection);
}

void GexSettings::OnCharWizardSettings()
{
    int lIndex      = comboCharWizardTemplate->currentIndex();
    int lTemplate   = -1;

    if (lIndex != -1)
        lTemplate = comboCharWizardTemplate->itemData(lIndex).toInt();
    else
    {
        GEX_ASSERT(false);
        GSLOG(SYSLOG_SEV_WARNING, "No Characterization template selected.");
    }

    if (lTemplate == GEX_ADV_CHARAC_BOXWHISKER_CHART)
    {
        GS::Gex::CharacBoxWhiskerTemplateDialog templateDialog(this);

        if (pGexMainWindow && pGexMainWindow->pWizardOneQuery)
        {
            QList<QMap<QString, QString> > conditionValues;
            QStringList conditionHierarchy  = pGexMainWindow->pWizardOneQuery->widgetConditionHierarchy->GetItems();

            foreach(const QString &l, conditionHierarchy)
            {
                QMap<QString, QString>  b;
                QStringList             m = l.split("|", QString::SkipEmptyParts);

                if (m.isEmpty() == false &&
                    m.count() == pGexMainWindow->pWizardOneQuery->GetTestConditionsLevel().count())
                {
                    for (int lIdx = 0; lIdx < pGexMainWindow->pWizardOneQuery->GetTestConditionsLevel().count(); ++lIdx)
                        b.insert(pGexMainWindow->pWizardOneQuery->GetTestConditionsLevel().at(lIdx), m.at(lIdx));

                    conditionValues.append(b);
                }
            }

            templateDialog.SetConditionsLevel(pGexMainWindow->pWizardOneQuery->GetTestConditionsLevel());
            templateDialog.SetConditionsValues(conditionValues);

            if (mCharacBoxWhiskerTemplate != NULL)
                templateDialog.SetTemplate(*mCharacBoxWhiskerTemplate);

            templateDialog.FillGui();

            if (templateDialog.exec() == QDialog::Accepted)
            {
                if (mCharacBoxWhiskerTemplate == NULL)
                    mCharacBoxWhiskerTemplate = new GS::Gex::CharacBoxWhiskerTemplate();

                *mCharacBoxWhiskerTemplate = templateDialog.GetTemplate();
            }
        }
    }
    else if (lTemplate == GEX_ADV_CHARAC_LINE_CHART)
    {
        GS::Gex::CharacLineChartSettingsDialog settingsDialog(this);

        if (pGexMainWindow && pGexMainWindow->pWizardOneQuery)
        {
            QList<QMap<QString, QString> > conditionValues;
            QStringList conditionHierarchy  = pGexMainWindow->pWizardOneQuery->widgetConditionHierarchy->GetItems();
            QString     lVariable           = ReportOptions.GetOption("adv_charac2", "variable").toString();

            foreach(const QString &l, conditionHierarchy)
            {
                QMap<QString, QString>  b;
                QStringList             m = l.split("|", QString::SkipEmptyParts);

                if (m.isEmpty() == false &&
                    m.count() == pGexMainWindow->pWizardOneQuery->GetTestConditionsLevel().count())
                {
                    for (int lIdx = 0; lIdx < pGexMainWindow->pWizardOneQuery->GetTestConditionsLevel().count(); ++lIdx)
                        b.insert(pGexMainWindow->pWizardOneQuery->GetTestConditionsLevel().at(lIdx), m.at(lIdx));

                    conditionValues.append(b);
                }
            }

            settingsDialog.SetConditionsLevel(pGexMainWindow->pWizardOneQuery->GetTestConditionsLevel());
            settingsDialog.SetConditionsValues(conditionValues);
            settingsDialog.SetDefaultVariable(lVariable);

            if (mCharacLineChartTemplate != NULL)
                settingsDialog.SetTemplate(*mCharacLineChartTemplate);

            settingsDialog.FillGui();

            if (settingsDialog.exec() == QDialog::Accepted)
            {
                if (mCharacLineChartTemplate == NULL)
                    mCharacLineChartTemplate = new GS::Gex::CharacLineChartTemplate();

                *mCharacLineChartTemplate = settingsDialog.GetTemplate();
            }
        }
    }
    else
        GSLOG(SYSLOG_SEV_WARNING, "Unknown Characterization template");

}
