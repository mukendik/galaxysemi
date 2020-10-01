#include <gqtl_log.h>

#include "settings_dialog.h"
#include "settings_sql.h"
#include "picktest_dialog.h"
#include "gex_shared.h"
#include "gex_constants.h"
#include "browser_dialog.h"
#include "report_options.h"
#include "browser.h"
#include "product_info.h"
#include "engine.h"
#include "report_template_gui.h"
#include "message.h"
#include <QMessageBox>


extern bool checkProfessionalFeature(bool bProductionReleaseOnly);
extern bool FillComboBox(QComboBox * pCombo, const char * szTextTab[]);
extern GexMainwindow *	pGexMainWindow;
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)

///////////////////////////////////////////////////////////
// Read a Test list file, and return its content as a string
///////////////////////////////////////////////////////////
QString ImportTestListFromFile(const QString &strTestListFile)
{
    QFile file( strTestListFile ); // Read the text from a file
    if(file.open( QIODevice::ReadOnly ) == false)
        return "";	// Failed to read file!!!

    // Read File Handle
    QTextStream hFile;
    hFile.setDevice(&file);	// Assign file handle to data stream

    QString		strString;
    QStringList	strScatterList;
    QStringList	strScatterListX;
    QStringList	strScatterListY;
    QString		strTestListResult;
    do
    {
        // Read one line from file
        strString = hFile.readLine();
        strString = strString.simplified();	// reduce blanks (tab, etc) to space, and remove duplicated spaces
        strString = strString.remove(' ');			// Remove all spaces (and as a result: all tabs, etc...)
        strString = strString.toLower();				// Convert to lower case

        // Check if comment line
        if(strString.startsWith("#"))
            goto next_line;

        // Check if Scatter line
        if(strString.startsWith("x="))
        {
            // Scatter line, format is:
            //	X=6001,6002,Y=6004,6006,6008,6019,6033,6047,6053
            strScatterList = strString.split("y=", QString::SkipEmptyParts);

            // check if this line is valid (i.e.: has strings 'x=' and "y=')
            if(strScatterList.count() != 2)
                goto next_line;

            // Cleanup the X list to only include test number
            strString = strScatterList[0].replace("x=","");	// remove string 'x='
            // Split it to enumerate all tests to see in X
            strScatterListX = strString.split(",", QString::SkipEmptyParts);

            // Cleanup the Y list to only include test number
            strString = strScatterList[1].replace(",y=","");	// remove string ',y='
            strString = strScatterList[1].replace("y=","");	// and remove string 'y='
            // Split it to enumerate all tests to see in Y
            strScatterListY = strString.split(",", QString::SkipEmptyParts);

            // Build return string: enumerate all combinations of X and Y lists.
            QStringList::Iterator itX;
            QStringList::Iterator itY;
            for (itX = strScatterListX.begin(); itX != strScatterListX.end(); ++itX )
            {
                // For each test in X, enumerate all tests in Y
                for (itY = strScatterListY.begin(); itY != strScatterListY.end(); ++itY )
                {
                    // If not the first test pair, insert a separator
                    if(strTestListResult.isEmpty() == false)
                        strTestListResult += "; ";

                    // Add new test pair
                    strTestListResult += *itX;
                    strTestListResult += ",";
                    strTestListResult += *itY;
                }
            }
        }
        else
        if(strString[0].isDigit())
        {
            // Regular test list (starts with a number), just take it as is!
            if(!strTestListResult.isEmpty())
                strTestListResult += ',';	// Incase we already have a list in the string, insert separator

            // Concatenate test list sequence.
            strTestListResult += strString;
        }

next_line:;
    }
    while(hFile.atEnd() == false);
    file.close();

    // Return list of tests
    return strTestListResult;
}



///////////////////////////////////////////////////////////
// Pick list of tests from list: Test Statistics field
///////////////////////////////////////////////////////////
void GexSettings::OnPickTestFromListStats(void)
{
    QString strSelection = PickTestFromList(true, (PickTestDialog::TestParametric | PickTestDialog::TestMultiParametric | PickTestDialog::TestGenericGalaxy));

    // If valid, save the list selected into the edit field...
    if(!strSelection.isEmpty())
        editStats->setText(strSelection);
}

///////////////////////////////////////////////////////////
// Settings switched to : Create Instant Report
///////////////////////////////////////////////////////////
void GexSettings::OnDoInstantReport(void)
{
    // Keep track of Module menu selected
    if(pGexMainWindow != NULL)
        pGexMainWindow->iGexAssistantSelected = GEX_MODULE_INSTANT_REPORT;

    // Show Instant report Notes
    labelInstantReportHint->show();

    // Show relevant report GUI
    widgetStackReport->setCurrentIndex(WIDGET_REPORT_STD);
    if(!buttonBuildReport->isEnabled())
        buttonBuildReport->setEnabled(true);

    if (pGexMainWindow != NULL && pGexMainWindow->GetWizardType() == GEX_FT_PAT_FILES_WIZARD)
    {
        comboAdvancedReport->setCurrentIndex(comboAdvancedReport->findData(GEX_ADV_PAT_TRACEABILITY));
        comboAdvancedReport->setEnabled(false);
    }
    else if (pGexMainWindow != NULL && pGexMainWindow->GetWizardType() == GEX_SHIFT_WIZARD)
    {
        comboAdvancedReport->setCurrentIndex(comboAdvancedReport->findData(GEX_ADV_SHIFT));
        comboAdvancedReport->setEnabled(false);
    }
    else
        comboAdvancedReport->setEnabled(true);
}

///////////////////////////////////////////////////////////
// User want to import a list of test pairs to scatter...
///////////////////////////////////////////////////////////
void GexSettings::OnImportScatterPairs(void)
{
    QString strTestListFile;
    // Let's user pick the File List to load
    strTestListFile = QFileDialog::getOpenFileName(this, "Load a Test List...", "", "Import Test list (*.txt)");

    // If no file selected, ignore command.
    if(strTestListFile.isEmpty())
        return;

    QString strTestList = ImportTestListFromFile(strTestListFile);

    // IF valid test list imported, load it into GUI!
    if(strTestList.isEmpty() == false)
    {
        editTestX->setText(strTestList);	// Insert list into X field
        editTestY->setText("");				// Clear the Y field
    }
}

///////////////////////////////////////////////////////////
// User changing report output format: HTML, WORD, CSV,...
///////////////////////////////////////////////////////////
void GexSettings::OnChangeReportFormat(void)
{
    // Not all formats are allowed...depends of the software Edition
    bool	bAllowAdvancedFormats=true;
    bool	bAllowInteractive=true;
    bool	bAllowPdfFormats=true;

    // Examinator-OEM doesn't allow advanced formats like: WORD, PDF, etc...
    switch(GS::LPPlugin::ProductInfo::getInstance()->getProductID())
    {
        case GS::LPPlugin::LicenseProvider::eSzOEM:			// OEM-Examinator for Credence SZ
            bAllowInteractive = false;
            bAllowAdvancedFormats = false;
            bAllowPdfFormats = true;
            break;
        case GS::LPPlugin::LicenseProvider::eLtxcOEM:		// OEM-Examinator for LTXC
            bAllowInteractive = true;
            bAllowAdvancedFormats = false;
            bAllowPdfFormats = true;
            break;
        default:
            break;
    }

    // Maintain same report output format in 'Settings' and 'Options' page.
    int iSettingsOutputFormat = comboBoxOutputFormat->currentIndex();

    if(bAllowInteractive && iSettingsOutputFormat == GEX_SETTINGS_OUTPUT_INTERACTIVE)
    {
        // In Interactive mode, disable all the settings options!
        widgetStackReport->setEnabled(false);

        // Disable 'Advanced options' so to avoid conflict if What-If selected...
        comboAdvancedReport->setCurrentIndex(comboAdvancedReport->findData(GEX_ADV_DISABLED));
        OnComboAdvancedReportChange();
    }
    else
    {
        // Report to create: enable report settings
        widgetStackReport->setEnabled(true);
    }

    switch(iSettingsOutputFormat)
    {
        case GEX_SETTINGS_OUTPUT_INTERACTIVE: // No report to create, interactive mode only.
            if(bAllowInteractive == false)
            {
                GS::Gex::Message::information(
                    "", "Your version of the product does not allow "
                    "interactive analysis\nContact " +
                    QString(GEX_EMAIL_SALES) + " for more information.");
                comboBoxOutputFormat->setCurrentIndex(GEX_SETTINGS_OUTPUT_HTML);	// Reset output format to: HTML
                return;
            }

            // If 'Enterprise Reports', no Interactive mode allowed!
            //if(pGexMainWindow->iWizardType == GEX_SQL_QUERY_WIZARD)
            if(pGexMainWindow->GetWizardType() == GEX_SQL_QUERY_WIZARD)
            {
                GS::Gex::Message::information(
                    "", "Interactive mode is not for Enterprise Reports!");
                comboBoxOutputFormat->setCurrentIndex(GEX_SETTINGS_OUTPUT_HTML);	// Reset output format to: HTML
                widgetStackReport->setEnabled(true);
                return;
            }
            break;

        case GEX_SETTINGS_OUTPUT_HTML:	// HTML output
        case GEX_SETTINGS_OUTPUT_CSV:	// CSV  output
            break;

        case GEX_SETTINGS_OUTPUT_WORD: // Word output: only available under the Professional Edition.
            if(bAllowAdvancedFormats == false)
            {
                GS::Gex::Message::information(
                    "", "Your current license does not allow Word document "
                    "generation\nPlease contact " +
                    QString(GEX_EMAIL_SALES) + " for more information.");
                    comboBoxOutputFormat->setCurrentIndex(GEX_SETTINGS_OUTPUT_HTML);	// Reset output format to: HTML
                return;
            }
            #if defined unix || __MACH__
                // Not available under unix...
                GS::Gex::Message::information(
                    "", "The Word document generation is not available under "
                    "Unix & Linux\nYou need the Windows version.\nContact " +
                    QString(GEX_EMAIL_SALES) + " for more information!");
                comboBoxOutputFormat->setCurrentIndex(GEX_SETTINGS_OUTPUT_HTML);	// Reset output format to: HTML
                return;
            #endif
            break;

    case GEX_SETTINGS_OUTPUT_PPT: // PowerPoint slides output:
            if(bAllowAdvancedFormats == false)
            {
                GS::Gex::Message::information(
                    "", "Your current license does not allow PowerPoint "
                    "slides generation\n\nContact " +
                    QString(GEX_EMAIL_SALES) + " for more information!");
                    comboBoxOutputFormat->setCurrentIndex(GEX_SETTINGS_OUTPUT_HTML);	// Reset output format to: HTML
                return;
            }
            #if defined unix || __MACH__
                // Not available under unix...
                GS::Gex::Message::information(
                    "", "The PowerPoint document generation is not available "
                    "under Unix & Linux\n\nContact " +
                    QString(GEX_EMAIL_SALES) + " for more information!");
                    comboBoxOutputFormat->setCurrentIndex(GEX_SETTINGS_OUTPUT_HTML);	// Reset output format to: HTML
                return;
            #endif
            break;

        case GEX_SETTINGS_OUTPUT_PDF: // PDF output: only available under the Professional Edition or ASL-100 OEM version
            if(bAllowPdfFormats == false)
            {
                GS::Gex::Message::information(
                    "", "Your current license does not allow PDF document "
                    "generation\nContact " +
                    QString(GEX_EMAIL_SALES) + " for more information.");
                    comboBoxOutputFormat->setCurrentIndex(GEX_SETTINGS_OUTPUT_HTML);	// Reset output format to: HTML
                return;
            }
            break;
    }

    // Have the 'Options' page reflect the new report format selected from this 'Settings' page.
   GS::Gex::OptionsHandler optionsHandler(GS::Gex::Engine::GetInstance().GetOptionsHandler());

    // If Function called from the 'Settings' page (iSettingsOutputFormat >= 0), we must have the 'Options' page align itself on the 'Settings' selection
    switch(iSettingsOutputFormat)
    {
        case GEX_SETTINGS_OUTPUT_HTML:			// Short HTML pages report.
            if(!optionsHandler.SetOption("output", "format", QString("HTML")))
                GEX_ASSERT(false);
        break;
        case GEX_SETTINGS_OUTPUT_WORD:			// WORD report.
            if(!optionsHandler.SetOption("output", "format", QString("DOC")))
                GEX_ASSERT(false);
        break;
        case GEX_SETTINGS_OUTPUT_PPT:			// PowerPoint slides report.
            if(!optionsHandler.SetOption("output", "format", QString("PPT")))
                GEX_ASSERT(false);
        break;
        case GEX_SETTINGS_OUTPUT_CSV:			// CSV report.
            if(!optionsHandler.SetOption("output", "format", QString("CSV")))
                GEX_ASSERT(false);
        break;
        case GEX_SETTINGS_OUTPUT_PDF:			// PDF report.
            if(!optionsHandler.SetOption("output", "format", QString("PDF")))
                GEX_ASSERT(false);
        break;
        case GEX_SETTINGS_OUTPUT_INTERACTIVE:	// Interactive mode, no report.
            if(!optionsHandler.SetOption("output", "format", QString("INTERACTIVE")))
                GEX_ASSERT(false);
        break;
        default:
            GSLOG(SYSLOG_SEV_WARNING, QString("Unknown output format : %1").arg(iSettingsOutputFormat)
                  .toLatin1().constData());
    }

    // Update default options map
    GS::Gex::Engine::GetInstance().SetOptionsMap(optionsHandler.GetOptionsMap());

    // Reset HTML sections to create flag: ALL pages to create.
    pGexMainWindow->iHtmlSectionsToSkip = 0;
}


///////////////////////////////////////////////////////////
// Pick list of tests from list: Wafermap field
///////////////////////////////////////////////////////////
void GexSettings::OnPickTestFromListWaferMap(void)
{
    int nTestType;

    if ((comboWafer->currentIndex() == GEX_WAFMAP_TEST_PASSFAIL) || (comboWafer->currentIndex() == GEX_WAFMAP_STACK_TEST_PASSFAIL))
        nTestType = PickTestDialog::TestParametric | PickTestDialog::TestMultiParametric | PickTestDialog::TestFunctional | PickTestDialog::TestGenericGalaxy;
    else
        nTestType = PickTestDialog::TestParametric | PickTestDialog::TestMultiParametric | PickTestDialog::TestGenericGalaxy;

    QString strSelection = PickTestFromList(true, nTestType);

    // If valid, save the list selected into the edit field...
    if(!strSelection.isEmpty())
        editWafer->setText(strSelection);
}

///////////////////////////////////////////////////////////
// Pick list of tests from list: Histograms field
///////////////////////////////////////////////////////////
void GexSettings::OnPickTestFromListHistogram(void)
{
    QString strSelection = PickTestFromList(true, (PickTestDialog::TestParametric | PickTestDialog::TestMultiParametric | PickTestDialog::TestGenericGalaxy));

    // If valid, save the list selected into the edit field...
    if(!strSelection.isEmpty())
        editHistogram->setText(strSelection);
}

///////////////////////////////////////////////////////////
// Pick list of tests from list: Advanced field
///////////////////////////////////////////////////////////
void GexSettings::OnPickTestFromListAdvanced(void)
{

    QString strSelection ;
    if(comboAdvancedReport->currentData() == GEX_ADV_FUNCTIONAL || comboAdvancedReport->currentData() == GEX_ADV_FTR_CORRELATION)
        strSelection = PickTestFromList(true, PickTestDialog::TestFunctional);
    else
        strSelection = PickTestFromList(true, (PickTestDialog::TestParametric | PickTestDialog::TestMultiParametric | PickTestDialog::TestGenericGalaxy));

    // If valid, save the list selected into the edit field...
    if(!strSelection.isEmpty())
        editAdvanced->setText(strSelection);
}

///////////////////////////////////////////////////////////
// Pick test to draw on Scatter X axis
///////////////////////////////////////////////////////////
void GexSettings::OnPickTestScatterX(void)
{
    QString strSelection = PickTestFromList(
                true,
                (PickTestDialog::TestParametric | PickTestDialog::TestMultiParametric | PickTestDialog::TestGenericGalaxy),
                false);

    // If valid, save the list selected into the edit field...
    if(!strSelection.isEmpty())
        editTestX->setText(strSelection);
}

///////////////////////////////////////////////////////////
// Pick test to draw on Scatter Y axis
///////////////////////////////////////////////////////////
void GexSettings::OnPickTestScatterY(void)
{
    QString strSelection = PickTestFromList(
                true,
                (PickTestDialog::TestParametric | PickTestDialog::TestMultiParametric | PickTestDialog::TestGenericGalaxy),
                false);

    // If valid, save the list selected into the edit field...
    if(!strSelection.isEmpty())
        editTestY->setText(strSelection);
}

///////////////////////////////////////////////////////////
// Test statistics value field edited
///////////////////////////////////////////////////////////
void GexSettings::OnStatisticsValueChanged(void)
{
    // Force flag to ensure HTML section 'WAFERMAP' will be updated
    pGexMainWindow->iHtmlSectionsToSkip &= ~(GEX_HTMLSECTION_STATISTICS);
}

///////////////////////////////////////////////////////////
// Wafermap Test# value field edited
///////////////////////////////////////////////////////////
void GexSettings::OnWaferMapValueChanged(void)
{
    // Force flag to ensure HTML section 'WAFERMAP' will be updated
    pGexMainWindow->iHtmlSectionsToSkip &= ~(GEX_HTMLSECTION_WAFERMAP);
}

///////////////////////////////////////////////////////////
// Histogram Test# value field edited
///////////////////////////////////////////////////////////
void GexSettings::OnHistogramValueChanged(void)
{
    // Force flag to ensure HTML section 'HISTOGRAM' will be updated
    pGexMainWindow->iHtmlSectionsToSkip &= ~(GEX_HTMLSECTION_HISTOGRAM);
}

///////////////////////////////////////////////////////////
// Advanced settings Test# value field edited
///////////////////////////////////////////////////////////
void GexSettings::OnAdvancedValueChanged(void)
{
    // Force flag to ensure HTML section 'ADVANCED' will be updated
    pGexMainWindow->iHtmlSectionsToSkip &= ~(GEX_HTMLSECTION_ADVANCED);
}

///////////////////////////////////////////////////////////
// Enable/Disable Test number for Test Statistics
///////////////////////////////////////////////////////////
void GexSettings::OnComboStatisticsChange(void)
{
    static int iPrevious=-1;
    int iComboPosition=comboStats->currentIndex();

    // Force flag to ensure HTML section 'STATISTICS' will be updated
    if(iPrevious != iComboPosition)
        pGexMainWindow->iHtmlSectionsToSkip &= ~(GEX_HTMLSECTION_STATISTICS);
    iPrevious = iComboPosition;

	QString editStatsText = editStats->text();
    switch(iComboPosition)
    {
    case GEX_STATISTICS_DISABLED:		// disabled
    case GEX_STATISTICS_ALL:			// All tests
    case GEX_STATISTICS_FAIL:			// Failing tests
    case GEX_STATISTICS_OUTLIERS:		// Tests with outliers
        editStats->hide();
        PickTestStats->hide();
        break;
    case GEX_STATISTICS_LIST:			// List of tests
        PickTestStats->show();			// Allow Test pickup from list...then fall in other cases hereafter...
    case GEX_STATISTICS_BADCP:			// Cp limit (report all tests with Cp lower or equal)
    case GEX_STATISTICS_BADCPK:			// Cpk limit (report all tests with Cpk lower or equal)
		if(!(editStatsText.size() > 0))
		   editStats->setText("");
        editStats->show();
        editStats->setFocus();
        break;
    case GEX_STATISTICS_TOP_N_FAILTESTS:
		if(!(editStatsText.size() > 0))
		   editStats->setText("5");
        editStats->show();
        editStats->setFocus();
        PickTestStats->hide();
        break;
    }
}

///////////////////////////////////////////////////////////
// Enable/Disable Test number for Wafermap Zoning
///////////////////////////////////////////////////////////
void GexSettings::OnComboWaferMapChange(void)
{
    static int iPrevious=-1;
    int iComboPosition=comboWafer->currentIndex();

    // Force flag to ensure HTML section 'WAFERMAP' will be updated
    if(iPrevious != iComboPosition)
        pGexMainWindow->iHtmlSectionsToSkip &= ~(GEX_HTMLSECTION_WAFERMAP);
    iPrevious = iComboPosition;

    switch(iComboPosition)
    {
        default: // Refuse this selection!
            comboWafer->setCurrentIndex(GEX_WAFMAP_SOFTBIN);	// Wafermap: Soft bin
            break;

        case GEX_WAFMAP_DISABLED:		// disabled
        case GEX_WAFMAP_SOFTBIN:		// Software Binning
        case GEX_WAFMAP_HARDBIN:		// Hardware binning
            comboWafermapTests->hide();
            editWafer->hide();
            PickTestWaferMap->hide();	// No pick from 'Test List'
            break;

        case GEX_WAFMAP_STACK_SOFTBIN:	// Stacked wafers: show Binlist field.
        case GEX_WAFMAP_STACK_HARDBIN:
        case GEX_WAFMAP_ZONAL_SOFTBIN:	// Zonal Softbin
        case GEX_WAFMAP_ZONAL_HARDBIN:	// Zonal Hardbin
            if((checkProfessionalFeature(true) == false)
               || ((GS::LPPlugin::ProductInfo::getInstance()
                            ->isNotSupportedCapability(GS::LPPlugin::ProductInfo::waferStack))))
            {
                QString m=ReportOptions.GetOption("messages", "upgrade").toString();
                GS::Gex::Message::information("", m);
                comboWafer->setCurrentIndex(GEX_WAFMAP_SOFTBIN);	// Wafermap: Soft bin
                return;
            }
            comboWafermapTests->hide();
            PickTestWaferMap->hide();	// No pick from 'Test List'
			if(editWafer->text().size() == 0)
            	editWafer->setText("1");
            editWafer->show();
            editWafer->setFocus();
            break;

        case GEX_WAFMAP_STACK_TEST_PASSFAIL:
        case GEX_WAFMAP_STACK_TESTOVERLIMITS:
        case GEX_WAFMAP_STACK_TESTOVERDATA:
            if(checkProfessionalFeature(true) == false)
            {
                QString m=ReportOptions.GetOption("messages", "upgrade").toString();
                GS::Gex::Message::information("", m);
                comboWafer->setCurrentIndex(GEX_WAFMAP_SOFTBIN);	// Wafermap: Soft bin
                return;
            }
        // STACKING mode allowed...fall into the following 'cases'...
        case GEX_WAFMAP_TEST_PASSFAIL:
        case GEX_WAFMAP_TESTOVERLIMITS: // Test characterization (Zoning) wafermap
        case GEX_WAFMAP_TESTOVERDATA:	// Test characterization (Zoning) wafermap
            comboWafermapTests->show();
            OnComboWaferMapTestChange();
            break;
    }
}

///////////////////////////////////////////////////////////
// Enable/Disable Test number list for wafermap
///////////////////////////////////////////////////////////
void GexSettings::OnComboWaferMapTestChange(void)
{
    static int	iPreviousWafmap	= -1;
    int			iComboPosition	= comboWafermapTests->currentIndex();

    // Force flag to ensure HTML section 'WFAERMAP' will be updated
    if(iPreviousWafmap != iComboPosition)
        pGexMainWindow->iHtmlSectionsToSkip &= ~(GEX_HTMLSECTION_WAFERMAP);

    iPreviousWafmap = iComboPosition;

    switch(iComboPosition)
    {
        case GEX_WAFMAP_ALL:	// Wafermap for All tests
            editWafer->hide();
            PickTestWaferMap->hide();	// No pick from 'Test List'
            break;
        case GEX_WAFMAP_LIST:	// Wafermap for list of tests
			if(editWafer->text().size() == 0)
	            editWafer->setText("");
            editWafer->show();
            PickTestWaferMap->show();	// Allow pick from 'Test List'
            editWafer->setFocus();
            break;
        case GEX_WAFMAP_TOP_N_FAILTESTS:
			if(editWafer->text().size() == 0)
            	editWafer->setText("5");
            editWafer->show();
            editWafer->setFocus();
            PickTestWaferMap->hide();
            break;
    }
}

///////////////////////////////////////////////////////////
// Enable/Disable histogram combos
///////////////////////////////////////////////////////////
void GexSettings::OnComboHistogramTypeChange(void)
{
    static int iPrevious=-1;
    int iComboPosition=comboHistogram->currentIndex();

    // Force flag to ensure HTML section 'HISTOGRAM' will be updated
    if(iPrevious != iComboPosition)
        pGexMainWindow->iHtmlSectionsToSkip &= ~(GEX_HTMLSECTION_HISTOGRAM);
    iPrevious = iComboPosition;

    switch(iComboPosition)
    {
    case GEX_HISTOGRAM_DISABLED:	// Histogram disabled
        comboHistogramTests->hide();
        // disable histogram edit.
        editHistogram->hide();
        PickTestHistogam->hide();	// No pick from 'Test List'
        break;

    default: // Histogram enabled.
    case GEX_HISTOGRAM_CUMULLIMITS:
    case GEX_HISTOGRAM_OVERDATA:
    case GEX_HISTOGRAM_CUMULDATA:
    case GEX_HISTOGRAM_DATALIMITS:
        comboHistogramTests->show();
        // Update the next histogram combo accordingly...
        OnComboHistogramChange();
        break;
    }
}

///////////////////////////////////////////////////////////
// Enable/Disable Test number list for Chart option
///////////////////////////////////////////////////////////
void GexSettings::OnComboHistogramChange(void)
{
    static int iPrevious=-1;
    int iComboPosition=comboHistogramTests->currentIndex();

    //GCORE-13587 (in case OnComboHistogramTypeChange slot was not called although it should be)
     if(comboHistogram->currentIndex() == GEX_HISTOGRAM_DISABLED)
     {
         comboHistogramTests->hide();
         editHistogram->hide();
         PickTestHistogam->hide();
     }

    // Force flag to ensure HTML section 'HISTOGRAM' will be updated
    if(iPrevious != iComboPosition)
        pGexMainWindow->iHtmlSectionsToSkip &= ~(GEX_HTMLSECTION_HISTOGRAM);
    iPrevious = iComboPosition;

	QString editHistogramText = editHistogram->text();
    switch(iComboPosition)
    {
    case GEX_HISTOGRAM_ALL:	// Chart All tests
        editHistogram->hide();
        PickTestHistogam->hide();	// No pick from 'Test List'
        break;
    case GEX_HISTOGRAM_LIST:	// Chart list of tests
		if(!(editHistogramText.size() > 0))
    	    editHistogram->setText("");
        editHistogram->show();
        PickTestHistogam->show();	// Allow pick from 'Test List'
        editHistogram->setFocus();
        break;
    case GEX_HISTOGRAM_TOP_N_FAIL_TESTS:
		if(!(editHistogramText.size() > 0))
    	    editHistogram->setText("5");
        editHistogram->show();
        editHistogram->setFocus();
        PickTestHistogam->hide();
        break;
    }
}

///////////////////////////////////////////////////////////
// Toggle Production report advanced options
///////////////////////////////////////////////////////////
void GexSettings::OnProductionReportOptions(void)
{
    if(groupBoxAdvProductionOptions->isEnabled())
    {
        // Do not show Production report advanced options
        groupBoxAdvProductionOptions->hide();
        groupBoxAdvProductionOptions->setEnabled(false);
        pushButtonProductionOptions->setText(">>");
    }
    else
    {
        // Display Production report advanced options
        groupBoxAdvProductionOptions->show();
        groupBoxAdvProductionOptions->setEnabled(true);
        pushButtonProductionOptions->setText("<<");
    }
}

///////////////////////////////////////////////////////////
// Production report advanced options changed
///////////////////////////////////////////////////////////
void GexSettings::OnProductionReportOptionsChanged(void)
{
    QString strSelection = comboBoxVolumeAxis->currentText();

    // Show/Hide some controls
    if(comboBoxVolumeAxis->currentText().indexOf("custom", 0, Qt::CaseInsensitive) == -1)
        spinBoxVolumeAxis->setEnabled(false);
    else
        spinBoxVolumeAxis->setEnabled(true);
    if(checkBoxYieldMarker->isChecked())
        spinBoxYieldMarker->setEnabled(true);
    else
        spinBoxYieldMarker->setEnabled(false);
}

///////////////////////////////////////////////////////////
// User has selected (or dropped) a Template file
///////////////////////////////////////////////////////////
void GexSettings::OnSelectReportTemplate(QString &strTemplateFile)
{
    // Make sure Report output is valid (must any type except Interactive)
    if(comboBoxOutputFormat->currentIndex() == GEX_SETTINGS_OUTPUT_INTERACTIVE)
    {
        // Interactive output: this is not correct, then force it to HTML!
        RefreshOutputFormat(GEX_SETTINGS_OUTPUT_HTML);
        OnChangeReportFormat();
    }

    // Make sure 'My Report' is selected in GUI
    comboAdvancedReport->setCurrentIndex(comboAdvancedReport->findData(GEX_ADV_TEMPLATE));

    // Report to create: enable report settings
    //GroupBoxInstantReports->setEnabled(true);

    // Refresh GUI
    OnComboAdvancedReportChange();

    // Save selection
    m_strTemplateFile = strTemplateFile;

    // Display name on GUI (without path)
    QFileInfo cFileInfo(strTemplateFile);
    strTemplateFile = cFileInfo.fileName();
    if(strTemplateFile.length() > 23)
        strTemplateFile = strTemplateFile.left(10) + "..." + strTemplateFile.right(10);
    strTemplateFile = "Template: " + strTemplateFile;
    textLabelTemplateFile->setText(strTemplateFile);
}

///////////////////////////////////////////////////////////
// User wants to browse disk & select the Report Template file to use.
///////////////////////////////////////////////////////////
void GexSettings::OnSelectReportTemplate(void)
{
    QString strTemplateFile;

    // User wants to select a Test# mapping file...
    strTemplateFile = QFileDialog::getOpenFileName(this, "Select Report Template File", m_strTemplateFile, "Quantix Report Template (*.grt)");

    if(strTemplateFile.isEmpty())
        return;

    // Save selection
    OnSelectReportTemplate(strTemplateFile);
}

///////////////////////////////////////////////////////////
// Launch Report template Wizard
///////////////////////////////////////////////////////////
void GexSettings::OnReportTemplateWizard(void)
{
    GS::Gex::ReportTemplateGui cTemplateWizard(pGexMainWindow->pScrollArea);

    // Load template file if already defined
    if(QFile::exists(m_strTemplateFile))
        cTemplateWizard.OnLoadReportTemplate(m_strTemplateFile);

    // Launch Report Template Wizard
    if(cTemplateWizard.exec() != 1)
        return;	// User cancelled Wizard

    // Save Tremplate file selection
    m_strTemplateFile = cTemplateWizard.m_strTemplateFile;

    // Display name on GUI (without path)
    QString strTemplateFile= m_strTemplateFile;
    QFileInfo cFileInfo(strTemplateFile);
    strTemplateFile = cFileInfo.fileName();
    if(strTemplateFile.length() > 23)
        strTemplateFile = strTemplateFile.left(10) + "..." + strTemplateFile.right(10);
    strTemplateFile = "Template: " + strTemplateFile;
    textLabelTemplateFile->setText(strTemplateFile);
}

///////////////////////////////////////////////////////////
// Updates list box showing Advanced reports settings
///////////////////////////////////////////////////////////
void GexSettings::OnComboAdvancedReportChange(void)
{
    static	int iPreviousContent=-1;	// Keep track of previous view so when only
                                    // refreshing a same list, do not lose selection!

    int lComboData=comboAdvancedReport->currentData().toInt();

    if(ReportOptions.mComputeAdvancedTest == false &&
       (lComboData == GEX_ADV_PROBABILITY_PLOT || lComboData == GEX_ADV_BOXPLOT) )
    {
        QMessageBox::warning(this, "No data computed", "This chart requires that you enable advanced statistics computation through "
                             "the option [Speed optimization] -> [Computing advanced statistics].");

        comboAdvancedReport->setCurrentIndex(comboAdvancedReport->findData(GEX_ADV_DISABLED));
        return;
    }


    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" new combo position = %1").arg(lComboData).toLatin1().constData());

    bool	bOemRelease=false;
    if(GS::LPPlugin::ProductInfo::getInstance()->getProductID() == GS::LPPlugin::LicenseProvider::eLtxcOEM)
        bOemRelease = true;

    // If only redrawing the same page...ignore request
    if(iPreviousContent == lComboData)
    {
        // Refreshes the status of the Advanced edit box
        OnComboAdvancedReportSettingsChange();
        return;
    }
    else
    {
        // Force flag to ensure HTML section 'HISTOGRAM' will be updated
        pGexMainWindow->iHtmlSectionsToSkip &= ~(GEX_HTMLSECTION_ADVANCED);
    }


    comboAdvanced->clear();
    comboAdvanced->addItem(ADV_ALL_TESTS);
    comboAdvanced->addItem(ADV_TEST_LIST);
    comboAdvanced->addItem(ADV_TOP_N_FAILTEST);
    comboAdvanced->setCurrentIndex(GEX_ADV_ALL);

    // Enable ALL Standard InstantReport combos...
    enableInstantReportStandardCombos(true);

    switch(lComboData)
    {
    case GEX_ADV_GO_DIAGNOSTICS:
        // Disable Histogram test list...and force it to ALL tests!
        comboHistogramTests->setEnabled(false);
        // Then enable the Histogram combo...so user can decide decide of histogram type (over limits or results)
        comboHistogram->setEnabled(true);
        // Then ensure 'All' tests is selected for the test list.
        comboHistogramTests->setCurrentIndex(GEX_HISTOGRAM_ALL);	// Histogram: all tests
        // Refresh Histogram type combo/edit boxes.
        OnComboHistogramTypeChange();
        // ...then fall in same case as 'default'
    case GEX_ADV_DISABLED: // Advanced reports is Disabled
    case GEX_ADV_GUARDBANDING:
    case GEX_ADV_PAT_TRACEABILITY:
    default:	// Text separators
        // Disable Advanced settings list box + edit box
        widgetStackAdvanced->hide();
        // Erase list only if we had one already...
        comboAdvancedReportSettings->clear();
        comboAdvancedReportSettings->hide();
        // Refreshes the status of the Advanced edit box
        OnComboAdvancedReportSettingsChange();
        break;

    case GEX_ADV_PEARSON:
        if(bOemRelease)
        {
            QString m=ReportOptions.GetOption("messages", "upgrade").toString();
            GS::Gex::Message::information("", m);
            comboAdvancedReport->setCurrentIndex(comboAdvancedReport->findData(GEX_ADV_DISABLED));	// Disabled
            return;
        }

        comboAdvancedReportSettings->show();
        comboAdvancedReportSettings->clear();
        FillComboBox(comboAdvancedReportSettings, PearsonItems);
        comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_PEARSON_OVERLIMITS);
        OnComboAdvancedReportSettingsChange();
        break;

    case GEX_ADV_HISTOGRAM:	// Adv-Histogram
        // Fill Advanced settings list box with Histogram options
        comboAdvancedReportSettings->show();
        // Erase list only if we had one already...
        comboAdvancedReportSettings->clear();
        FillComboBox(comboAdvancedReportSettings, HistogramItems);

        // Default selection is: GEX_ADV_HISTOGRAM_OVERLIMITS
        comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_HISTOGRAM_OVERDATA);
        // Refreshes the status of the Advanced edit box
        OnComboAdvancedReportSettingsChange();
        break;
    case GEX_ADV_FUNCTIONAL: //Functional tests report
        if(bOemRelease)
        {
            QString m=ReportOptions.GetOption("messages", "upgrade").toString();
            GS::Gex::Message::information("", m);
            comboAdvancedReport->setCurrentIndex(comboAdvancedReport->findData(GEX_ADV_DISABLED));	// Disabled
            return;
        }

        // Fill Advanced settings list box with Histogram options
        comboAdvancedReportSettings->show();
        // Erase list only if we had one already...
        comboAdvancedReportSettings->clear();
        FillComboBox(comboAdvancedReportSettings, FunctionalReportItems);
        // Default selection is: GEX_ADV_HISTOGRAM_OVERLIMITS
        comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_FUNCTIONAL_CYCL_CNT);
        // Refreshes the status of the Advanced edit box
        OnComboAdvancedReportSettingsChange();

        comboAdvanced->clear();
        comboAdvanced->addItem(ADV_ALL_TESTS);
        comboAdvanced->addItem(ADV_TEST_LIST);
        comboAdvanced->setCurrentIndex(0);
        if(comboAdvanced->isHidden())
            comboAdvanced->show();

        break;
    case GEX_ADV_FTR_CORRELATION: //Functional tests report
        if(bOemRelease)
        {
            QString m=ReportOptions.GetOption("messages", "upgrade").toString();
            GS::Gex::Message::information("", m);
            comboAdvancedReport->setCurrentIndex(comboAdvancedReport->findData(GEX_ADV_DISABLED));	// Disabled
            return;
        }

        // Fill Advanced settings list box with Histogram options
        comboAdvancedReportSettings->show();
        // Erase list only if we had one already...
        comboAdvancedReportSettings->clear();
        FillComboBox(comboAdvancedReportSettings, FTRCorrelationReportItems);
        // Default selection is: GEX_ADV_HISTOGRAM_OVERLIMITS
        comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_FTR_CORRELATION_ALL);
        // Refreshes the status of the Advanced edit box
        OnComboAdvancedReportSettingsChange();

        comboAdvanced->clear();
        comboAdvanced->hide();
        break;

    case GEX_ADV_TREND:	// Trend chart
        // Fill Advanced settings list box with Trend options
        comboAdvancedReportSettings->show();
        comboAdvancedReportSettings->clear();
        FillComboBox(comboAdvancedReportSettings, TrendItems);
        // Default selection is: GEX_ADV_TREND_OVERLIMITS
        comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_TREND_OVERLIMITS);
        // Refreshes the status of the Advanced edit box
        OnComboAdvancedReportSettingsChange();
        break;

    case GEX_ADV_CORRELATION:	// Adv-Scatter
        // Fill Advanced settings list box with Histogram options
        comboAdvancedReportSettings->show();
        // Erase list only if we had one already...
        comboAdvancedReportSettings->clear();
        FillComboBox(comboAdvancedReportSettings, ScatterItems);
        // Default selection is: GEX_ADV_CORR_OVERLIMITS
        comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_CORR_OVERLIMITS);
        // Refreshes the status of the Advanced edit box
        OnComboAdvancedReportSettingsChange();
        break;

    case GEX_ADV_PROBABILITY_PLOT:	// Adv-Probability plot
        if(bOemRelease)
        {
            QString m=ReportOptions.GetOption("messages", "upgrade").toString();
            GS::Gex::Message::information("", m);
            comboAdvancedReport->setCurrentIndex(comboAdvancedReport->findData(GEX_ADV_DISABLED));	// Disabled
            return;
        }

        // Fill Advanced settings list box with boxplot options
        comboAdvancedReportSettings->show();
        comboAdvancedReportSettings->clear();
        FillComboBox(comboAdvancedReportSettings, ProbabilityPlotItems);
        // Default selection is: GEX_ADV_PROBPLOT_OVERLIMITS
        comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_PROBPLOT_OVERLIMITS);
        // Refreshes the status of the Advanced edit box
        OnComboAdvancedReportSettingsChange();
        break;

    case GEX_ADV_BOXPLOT:	// Adv-BoxplotEx
        // Fill Advanced settings list box with boxplot options
        comboAdvancedReportSettings->show();
        comboAdvancedReportSettings->clear();
        FillComboBox(comboAdvancedReportSettings, BoxPlotExItems);

        // Default selection is: GEX_ADV_BOXPLOT_OVERLIMITS
        comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_BOXPLOT_OVERLIMITS);
        // Refreshes the status of the Advanced edit box
        OnComboAdvancedReportSettingsChange();
        break;

    case GEX_ADV_MULTICHART:	// Adv-Multi-charts
        if(bOemRelease)
        {
            QString m=ReportOptions.GetOption("messages", "upgrade").toString();
            GS::Gex::Message::information("", m);
            comboAdvancedReport->setCurrentIndex(comboAdvancedReport->findData(GEX_ADV_DISABLED));	// Disabled
            return;
        }

        // Fill Advanced settings list box with Multi-charts options
        comboAdvancedReportSettings->show();
        // Erase list only if we had one already...
        comboAdvancedReportSettings->clear();
        FillComboBox(comboAdvancedReportSettings, MultiChartItems);

        // Default selection is: GEX_ADV_HISTOGRAM_OVERLIMITS
        comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_MULTICHART_OVERDATA);
        // Refreshes the status of the Advanced edit box
        OnComboAdvancedReportSettingsChange();
        break;

    case GEX_ADV_CANDLE_MEANRANGE:	// Adv-BoxPlot
        // Fill Advanced settings list box with Histogram options
        comboAdvancedReportSettings->show();
        // Erase list only if we had one already...
        comboAdvancedReportSettings->clear();
        FillComboBox(comboAdvancedReportSettings, BoxPlotItems);

        // Default selection is: GEX_ADV_ALL
        comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_ALL);
        // Refreshes the status of the Advanced edit box
        OnComboAdvancedReportSettingsChange();
        break;


    case GEX_ADV_DATALOG:	// Datalog
        // Fill Advanced settings list box with datalog options
        comboAdvancedReportSettings->show();
        // Erase list only if we had one already...
        comboAdvancedReportSettings->clear();
        FillComboBox(comboAdvancedReportSettings, DalalogItems);

        // Default selection is: GEX_ADV_DATALOG_FAIL
        comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_DATALOG_FAIL);
        // Refreshes the status of the Advanced edit box
        OnComboAdvancedReportSettingsChange();
        break;

    case GEX_ADV_PROD_YIELD:	// Production reports (yield trend)
        if(checkProfessionalFeature(true) == false)
        {
            QString m=ReportOptions.GetOption("messages", "upgrade").toString();
            GS::Gex::Message::information("", m);
            comboAdvancedReport->setCurrentIndex(comboAdvancedReport->findData(GEX_ADV_DISABLED));	// Disabled
            return;
        }
        // Fill Advanced settings list box with Histogram options
        comboAdvancedReportSettings->show();
        // Erase list only if we had one already...
        comboAdvancedReportSettings->clear();
        FillComboBox(comboAdvancedReportSettings, ProductionYieldItems);

        // Default selection is: GEX_ADV_PRODYIELD_SBLOT
        comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_PRODYIELD_SBLOT);
        // Refreshes the status of the Advanced edit box
        OnComboAdvancedReportSettingsChange();
        break;

        case GEX_ADV_TEMPLATE:
            if(!bOemRelease && checkProfessionalFeature(false) == false)
            {
                QString m=ReportOptions.GetOption("messages", "upgrade").toString();
                GS::Gex::Message::information("", m);
                comboAdvancedReport->setCurrentIndex(comboAdvancedReport->findData(GEX_ADV_DISABLED));	// Disabled
                return;
            }
            // Disable advanced drop-down box
            comboAdvancedReportSettings->show();
            comboAdvancedReportSettings->clear();
            comboAdvancedReportSettings->setEnabled(false);

            // Refreshes the status of the Advanced edit box
            OnComboAdvancedReportSettingsChange();
            break;

        case GEX_ADV_SHIFT:
            GSLOG(SYSLOG_SEV_INFORMATIONAL, "Shift analysis choosed");
            if(bOemRelease)
            {
                QString m=ReportOptions.GetOption("messages", "upgrade").toString();
                GS::Gex::Message::information("", m);
                comboAdvancedReport->setCurrentIndex(comboAdvancedReport->findData(GEX_ADV_DISABLED));	// Disabled
                return;
            }

            // Disable Advanced settings list box + edit box
            widgetStackAdvanced->hide();
            // Erase list only if we had one already...
            comboAdvancedReportSettings->clear();
            comboAdvancedReportSettings->hide();
            // Refreshes the status of the Advanced edit box
            OnComboAdvancedReportSettingsChange();
            break;

//        case GEX_ADV_CHARAC_BOXWHISKER_CHART:
//            GSLOG(SYSLOG_SEV_INFORMATIONAL, "Charac1 analysis choosed");
//            // Fill Advanced settings list box with Multi-charts options
//            comboAdvancedReportSettings->show();
//            // Erase list only if we had one already...
//            comboAdvancedReportSettings->clear();
//            FillComboBox(comboAdvancedReportSettings, Charac1ChartItems);
//            // Default selection is: GEX_ADV_HISTOGRAM_OVERLIMITS
//            comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_MULTICHART_OVERDATA);
//            // Refreshes the status of the Advanced edit box
//            OnComboAdvancedReportSettingsChange();
//        break;
    }
    // update list being viewed.
    iPreviousContent = lComboData;
}

void GexSettings::OnComboAdvancedReportSettingsChange(void)
{
    static int iPrevious=-1;
    int iComboPosition=comboAdvancedReportSettings->currentIndex();

    // Force flag to ensure HTML section 'HISTOGRAM' will be updated
    if(iPrevious != iComboPosition)
        pGexMainWindow->iHtmlSectionsToSkip &= ~(GEX_HTMLSECTION_ADVANCED);
    iPrevious = iComboPosition;

    int iAdvancedReport=comboAdvancedReport->currentData().toInt();

    // Make standard settings visible & enabled (only disabled if 'MyReports' template Wizard is selected)
    frameStandardSettings->setEnabled(true);

    switch(iAdvancedReport)
    {
    case GEX_ADV_DISABLED:			// Advanced report is Disabled
    case GEX_ADV_GUARDBANDING:		// Guard banding
    case GEX_ADV_PEARSON:
    case GEX_ADV_PAT_TRACEABILITY:
    case GEX_ADV_GO_DIAGNOSTICS:	// Galaxy Optimizer Diagnostics
    default:	// Text separators
        // ensure edit box is disabled!
        widgetStackAdvanced->hide();	// No pick from 'Test List'
        break;

    case GEX_ADV_HISTOGRAM: // Advanced report: Histogram
        // Show standard 'test#' edit box, hide 'Scatter edit box'
        widgetStackAdvanced->setCurrentIndex(0);
        widgetStackAdvanced->show();
        switch(iComboPosition)
        {
            case GEX_ADV_HISTOGRAM_OVERLIMITS:
            case GEX_ADV_HISTOGRAM_CUMULLIMITS:
            case GEX_ADV_HISTOGRAM_OVERDATA:
            case GEX_ADV_HISTOGRAM_CUMULDATA:
            case GEX_ADV_HISTOGRAM_DATALIMITS:
                // enable edit box so user can enter test list.
                comboAdvanced->show();
                #ifdef QT_DEBUG
                 //comboAdvanced->setCurrentIndex(1);
                #endif
                editAdvanced->show();
                PickTestAdvanced->show();
                editAdvanced->setFocus();
                break;
            default:	// Refuse selections that are comments!
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_HISTOGRAM_OVERLIMITS);
                OnComboAdvancedReportSettingsChange();
                break;
        }
        break;

    case GEX_ADV_FTR_CORRELATION:
    {
        widgetStackAdvanced->hide();
        switch(iComboPosition)
        {
            case GEX_ADV_FTR_CORRELATION_ALL:
                editAdvanced->hide();
                comboAdvanced->hide();
                PickTestAdvanced->hide();	// Allow pick from 'Test List'
            break;

            case GEX_ADV_FTR_CORRELATION_LIST:
                comboAdvanced->hide();
                editAdvanced->show();
                PickTestAdvanced->show();
                editAdvanced->show();
                editAdvanced->setFocus();
                break;
        }
    }
        break;

    case GEX_ADV_FUNCTIONAL:
        {
            widgetStackAdvanced->setCurrentIndex(0);
            widgetStackAdvanced->show();
            //comboAdvanced->setCurrentIndex(0);

        }
        break;
    case GEX_ADV_TREND:	// Trend chart
        // Show standard 'test#' edit box, hide 'Scatter edit box'
        widgetStackAdvanced->setCurrentIndex(0);
        widgetStackAdvanced->show();
        switch(iComboPosition)
        {
            case GEX_ADV_TREND_OVERLIMITS:
            case GEX_ADV_TREND_OVERDATA:
            case GEX_ADV_TREND_DATALIMITS:
            case GEX_ADV_TREND_AGGREGATE_MEAN:
            case GEX_ADV_TREND_AGGREGATE_SIGMA:
            case GEX_ADV_TREND_AGGREGATE_CP:
            case GEX_ADV_TREND_AGGREGATE_CPK:
                // enable edit box so user can enter test list.
                comboAdvanced->show();
                #ifdef QT_DEBUG
                 //comboAdvanced->setCurrentIndex(1);
                #endif
                editAdvanced->show();
                PickTestAdvanced->show();
                editAdvanced->setFocus();
                break;

            case GEX_ADV_TREND_SOFTBIN_ROLLING:
            case GEX_ADV_TREND_HARDBIN_ROLLING:
                if(checkProfessionalFeature(true) == false)
                {
                    QString m=ReportOptions.GetOption("messages", "upgrade").toString();
                    GS::Gex::Message::information("", m);
                    comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_TREND_SOFTBIN_PARTS);	// Trend: Soft bin on all parts
                    return;
                }
            case GEX_ADV_TREND_SOFTBIN_SBLOTS:
            case GEX_ADV_TREND_SOFTBIN_PARTS:
            case GEX_ADV_TREND_HARDBIN_SBLOTS:
            case GEX_ADV_TREND_HARDBIN_PARTS:
                // enable edit box so user can enter test list.
                comboAdvanced->show();
                #ifdef QT_DEBUG
                 //comboAdvanced->setCurrentIndex(1);
                #endif
                editAdvanced->show();
                editAdvanced->setFocus();
                // No pick from 'Test List'
                PickTestAdvanced->hide();
                break;
            case GEX_ADV_TREND_DIFFERENCE:
                // Hide standard 'test#' edit box, Show 'Scatter edit box'
                widgetStackAdvanced->setCurrentIndex(1);
                widgetStackAdvanced->show();
                buttonImportScatterPairs->hide();	// Hide icon to pick list of pairs to scatter
                GroupBoxInstantReportTwoTests->setTitle("Enter tests:");
                labelTest1->setText("Test#1:");
                labelTest2->setText("Test#2:");
                // Ensure focus is on the first Scatter axis edit field (test in X)
                editTestX->setFocus();
                break;
            default:	// Refuse selections that are comments!
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_TREND_OVERLIMITS);
                OnComboAdvancedReportSettingsChange();
                break;
        }
        break;

    case GEX_ADV_CORRELATION:	// Adv-Scatter
        // Hide standard 'test#' edit box, Show 'Scatter edit box'
        labelTest1->setText("Tests in X:");
        labelTest2->setText("Tests in Y:");
        widgetStackAdvanced->setCurrentIndex(1);
        widgetStackAdvanced->show();
        buttonImportScatterPairs->show();	// Hide icon to pick list of pairs to scatter
        GroupBoxInstantReportTwoTests->setTitle("Plot Axis:");
        switch(iComboPosition)
        {
            case GEX_ADV_CORR_OVERLIMITS:
            case GEX_ADV_CORR_OVERDATA:
            case GEX_ADV_CORR_DATALIMITS:
                // Ensure focus is on the first Scatter axis edit field (test in X)
                editTestX->setFocus();
                break;
            default:	// Refuse selections that are comments!
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_CORR_OVERLIMITS);
                OnComboAdvancedReportSettingsChange();
                break;
        }
        break;

    case GEX_ADV_PROBABILITY_PLOT: // Advanced report: Probability plot
        // Show standard 'test#' edit box, hide 'Scatter edit box'
        widgetStackAdvanced->setCurrentIndex(0);
        widgetStackAdvanced->show();
        switch(iComboPosition)
        {
            case GEX_ADV_PROBPLOT_OVERLIMITS:
            case GEX_ADV_PROBPLOT_OVERDATA:
            case GEX_ADV_PROBPLOT_DATALIMITS:
                // enable edit box so user can enter test list.
                comboAdvanced->show();
                #ifdef QT_DEBUG
                 //comboAdvanced->setCurrentIndex(1);
                #endif
                editAdvanced->show();
                PickTestAdvanced->show();
                editAdvanced->setFocus();
                break;
            default:	// Refuse selections that are comments!
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_PROBPLOT_OVERLIMITS);
                OnComboAdvancedReportSettingsChange();
                break;
        }
        break;

    case GEX_ADV_BOXPLOT: // Advanced report: Boxplot
        // Show standard 'test#' edit box, hide 'Scatter edit box'
        widgetStackAdvanced->setCurrentIndex(0);
        widgetStackAdvanced->show();
        switch(iComboPosition)
        {
            case GEX_ADV_BOXPLOT_OVERLIMITS:
            case GEX_ADV_BOXPLOT_OVERDATA:
            case GEX_ADV_BOXPLOT_DATALIMITS:
                // enable edit box so user can enter test list.
                comboAdvanced->show();
                #ifdef QT_DEBUG
                 //comboAdvanced->setCurrentIndex(1);
                #endif
                editAdvanced->show();
                PickTestAdvanced->show();
                editAdvanced->setFocus();
                break;
            default:	// Refuse selections that are comments!
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_BOXPLOT_OVERLIMITS);
                OnComboAdvancedReportSettingsChange();
                break;
        }
        break;

    case GEX_ADV_MULTICHART: // Advanced report: Multi-chart plot
        // Show standard 'test#' edit box, hide 'Scatter edit box'
        widgetStackAdvanced->setCurrentIndex(0);
        widgetStackAdvanced->show();
        switch(iComboPosition)
        {
            case GEX_ADV_MULTICHART_OVERLIMITS:
            case GEX_ADV_MULTICHART_OVERDATA:
            case GEX_ADV_MULTICHART_DATALIMITS:
                // enable edit box so user can enter test list.
                comboAdvanced->show();
                #ifdef QT_DEBUG
                 //comboAdvanced->setCurrentIndex(1);
                #endif
                editAdvanced->show();
                PickTestAdvanced->show();
                editAdvanced->setFocus();
                break;
            default:	// Refuse selections that are comments!
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_MULTICHART_OVERLIMITS);
                OnComboAdvancedReportSettingsChange();
                break;
        }
        break;

    case GEX_ADV_CANDLE_MEANRANGE:	// Adv-BoxPlot Gage RnR
        // Show standard 'test#' edit box, hide 'Scatter edit box'
        widgetStackAdvanced->setCurrentIndex(0);
        widgetStackAdvanced->show();
        switch(iComboPosition)
        {
            case GEX_ADV_ALL:
                editAdvanced->hide();
                PickTestAdvanced->hide();	// Allow pick from 'Test List'
                comboAdvanced->hide();
                break;
            case GEX_ADV_LIST:
                comboAdvanced->hide();
                editAdvanced->show();
                PickTestAdvanced->show();
                editAdvanced->setFocus();
                break;
            default:	// Refuse selections that are comments!
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_ALL);
                OnComboAdvancedReportSettingsChange();
                break;
        }
        break;

    case GEX_ADV_DATALOG:	// Datalog
        // Show standard 'test#' edit box, hide 'Scatter edit box'
        widgetStackAdvanced->setCurrentIndex(0);
        widgetStackAdvanced->show();
        switch(iComboPosition)
        {
            case GEX_ADV_DATALOG_ALL:
            case GEX_ADV_DATALOG_FAIL:
                editAdvanced->hide();
                comboAdvanced->hide();
                PickTestAdvanced->hide();	// Allow pick from 'Test List'
                break;
            case GEX_ADV_DATALOG_OUTLIER:
            case GEX_ADV_DATALOG_LIST:
            case GEX_ADV_DATALOG_RAWDATA:
                comboAdvanced->hide();
                editAdvanced->show();
                PickTestAdvanced->show();
                editAdvanced->setFocus();
                break;
            default:	// Refuse selections that are comments!
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_DATALOG_ALL);
                OnComboAdvancedReportSettingsChange();
                break;
        }
        break;

        case GEX_ADV_PROD_YIELD:		// Production reports (yield trend)
            // Display Production edit box
            widgetStackAdvanced->setCurrentIndex(2);
            widgetStackAdvanced->show();
            editAdvancedProduction->setFocus();
            break;

        case GEX_ADV_TEMPLATE:			// Custom Report layout
            // Display Tremplate wizard Widget
            widgetStackAdvanced->setCurrentIndex(3);
            widgetStackAdvanced->show();
            // Disable the standard settings as they no longer are relevant under the Template Wizard.
            frameStandardSettings->setEnabled(false);
            break;

        case GEX_ADV_SHIFT:
            GSLOG(SYSLOG_SEV_DEBUG, "Shift");
            // ensure edit box is disabled!
            widgetStackAdvanced->hide();	// No pick from 'Test List'
            break;

//        case GEX_ADV_CHARAC_BOXWHISKER_CHART:
//            GSLOG(SYSLOG_SEV_DEBUG, "Adv repot Charac1 selected");

//            widgetStackAdvanced->setCurrentIndex(0);
//            widgetStackAdvanced->show();
//            switch(iComboPosition)
//            {
//                case GEX_ADV_CHARAC_CHART_OVERLIMITS:
//                case GEX_ADV_CHARAC_CHART_OVERDATA:
//                case GEX_ADV_CHARAC_CHART_DATALIMITS:
//                    // enable edit box so user can enter test list.
//                    comboAdvanced->show();
//                    #ifdef QT_DEBUG
//                     comboAdvanced->setCurrentIndex(1);
//                    #endif
//                    editAdvanced->show();
//                    PickTestAdvanced->show();
//                    editAdvanced->setFocus();
//                    break;
//                default:	// Refuse selections that are comments!
//                    comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_CHARAC_CHART_OVERDATA);
//                    OnComboAdvancedReportSettingsChange();
//                    break;
//            }
//            break;
    }
    //GCORE-17481: hide unnecessary elements
    if(comboAdvanced->currentText() == ADV_ALL_TESTS)
    {
        editAdvanced->hide();
        PickTestAdvanced->hide();
    }
    else if (comboAdvanced->currentText() == ADV_TOP_N_FAILTEST)
    {
        PickTestAdvanced->hide();
    }
}

void GexSettings::OnComboAdvancedTestsChanged(int i)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" new pos : %1").arg( i).toLatin1().constData());

    switch(i)
    {
        case 0: // All tests
            editAdvanced->hide();
            PickTestAdvanced->hide();
            break;
        case 1:	// Tests list
            //editAdvanced->setText("");
            editAdvanced->show();
            PickTestAdvanced->show();
            break;
        case 2:	// Top N fail tests
            //editAdvanced->setText("5");
            editAdvanced->show();
            PickTestAdvanced->hide();
            break;
    }
}
