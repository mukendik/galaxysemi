///////////////////////////////////////////////////////////
// ALL Browser wizard pages for 'Analyze Dataset' (1 query) : 'On' events
///////////////////////////////////////////////////////////
#ifdef _WIN32
  #include <windows.h>
#endif

#include "browser_dialog.h"
#include "gex_shared.h"
#include "db_engine.h"
#include "db_onequery_wizard.h"
#include "report_build.h"
#include "report_options.h"
#include "calendar_dialog.h"
#include "db_single_query_filterform_dialog.h"
#include <gqtl_log.h>
#include "engine.h"
#include "pick_fields_dialog.h"
#include "test_condition_level_widget.h"

#include <qtooltip.h>
#include <QFileDialog>
#include "message.h"

// in main.cpp
extern GexMainwindow *	pGexMainWindow;

// report_build.cpp
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)

// in classes.cpp
extern bool SetCurrentComboItem(QComboBox *pCombo, const QString & strItem, bool bInsertIfMissing=false);

extern bool FillComboBox(QComboBox * pCombo, const char * szTextTab[]);

///////////////////////////////////////////////////////////
// 'FilterForm' button clicked
///////////////////////////////////////////////////////////
void GexOneQueryWizardPage1::OnAllFilters()
{
    GexOneQueryFilterFormDialog clFilterFormDlg;

    clFilterFormDlg.exec();
}

///////////////////////////////////////////////////////////
// Display list of Selections valid for Filter1
///////////////////////////////////////////////////////////
void	GexOneQueryWizardPage1::OnPickFilter1(void)
{
    // Pick Filter string from list for Filter#1
    GexDatabaseFilter dbFilter;

    // Fill current filters
    FillDatabaseFilter(dbFilter, comboBoxFilter1->currentText());

    PickFilterFromLiveList(dbFilter, FilterString1);
}

///////////////////////////////////////////////////////////
// Display list of Selections valid for Filter2
///////////////////////////////////////////////////////////
void	GexOneQueryWizardPage1::OnPickFilter2(void)
{
    // Pick Filter string from list for Filter#2
    GexDatabaseFilter dbFilter;

    // Fill current filters
    FillDatabaseFilter(dbFilter, comboBoxFilter2->currentText());

    PickFilterFromLiveList(dbFilter, FilterString2);
}

///////////////////////////////////////////////////////////
// Display list of Selections valid for Filter3
///////////////////////////////////////////////////////////
void	GexOneQueryWizardPage1::OnPickFilter3(void)
{
    // Pick Filter string from list for Filter#3
    GexDatabaseFilter dbFilter;

    // Fill current filters
    FillDatabaseFilter(dbFilter, comboBoxFilter3->currentText());

    PickFilterFromLiveList(dbFilter, FilterString3);
}

///////////////////////////////////////////////////////////
// Display list of Selections valid for Filter4
///////////////////////////////////////////////////////////
void	GexOneQueryWizardPage1::OnPickFilter4(void)
{
    // Pick Filter string from list for Filter#4
    GexDatabaseFilter dbFilter;

    // Fill current filters
    FillDatabaseFilter(dbFilter, comboBoxFilter4->currentText());

    PickFilterFromLiveList(dbFilter, FilterString4);
}

///////////////////////////////////////////////////////////
// Display list of Selections valid for Filter5
///////////////////////////////////////////////////////////
void	GexOneQueryWizardPage1::OnPickFilter5(void)
{
    // Pick Filter string from list for Filter#5
    GexDatabaseFilter dbFilter;

    // Fill current filters
    FillDatabaseFilter(dbFilter, comboBoxFilter5->currentText());

    PickFilterFromLiveList(dbFilter, FilterString5);
}

///////////////////////////////////////////////////////////
// Display list of Selections valid for Filter6
///////////////////////////////////////////////////////////
void	GexOneQueryWizardPage1::OnPickFilter6(void)
{
    // Pick Filter string from list for Filter#6
    GexDatabaseFilter dbFilter;

    // Fill current filters
    FillDatabaseFilter(dbFilter, comboBoxFilter6->currentText());

    PickFilterFromLiveList(dbFilter, FilterString6);
}

///////////////////////////////////////////////////////////
// Display list of Selections valid for Filter7
///////////////////////////////////////////////////////////
void	GexOneQueryWizardPage1::OnPickFilter7(void)
{
    // Pick Filter string from list for Filter#7
    GexDatabaseFilter dbFilter;

    // Fill current filters
    FillDatabaseFilter(dbFilter, comboBoxFilter7->currentText());

    PickFilterFromLiveList(dbFilter, FilterString7);
}

///////////////////////////////////////////////////////////
// Display list of Selections valid for Filter8
///////////////////////////////////////////////////////////
void	GexOneQueryWizardPage1::OnPickFilter8(void)
{
    // Pick Filter string from list for Filter#8
    GexDatabaseFilter dbFilter;

    // Fill current filters
    FillDatabaseFilter(dbFilter, comboBoxFilter8->currentText());

    PickFilterFromLiveList(dbFilter, FilterString8);
}

void GexOneQueryWizardPage1::OnPickTCFilter1()
{
    // Pick Filter string from list for TC Filter#1
    GexDatabaseFilter dbFilter;

    // Fill current filters
    FillDatabaseFilter(dbFilter, comboBoxTCFilter1->currentText());

    PickFilterFromLiveList(dbFilter, comboBoxTCValue1, false);
}

void GexOneQueryWizardPage1::OnPickTCFilter2()
{
    // Pick Filter string from list for TC Filter#2
    GexDatabaseFilter dbFilter;

    // Fill current filters
    FillDatabaseFilter(dbFilter, comboBoxTCFilter2->currentText());

    PickFilterFromLiveList(dbFilter, comboBoxTCValue2, false);
}

void GexOneQueryWizardPage1::OnPickTCFilter3()
{
    // Pick Filter string from list for TC Filter#3
    GexDatabaseFilter dbFilter;

    // Fill current filters
    FillDatabaseFilter(dbFilter, comboBoxTCFilter3->currentText());

    PickFilterFromLiveList(dbFilter, comboBoxTCValue3, false);
}

void GexOneQueryWizardPage1::OnPickTCFilter4()
{
    // Pick Filter string from list for TC Filter#3
    GexDatabaseFilter dbFilter;

    // Fill current filters
    FillDatabaseFilter(dbFilter, comboBoxTCFilter4->currentText());

    PickFilterFromLiveList(dbFilter, comboBoxTCValue4, false);
}

void GexOneQueryWizardPage1::OnPickTCFilter5()
{
    // Pick Filter string from list for TC Filter#3
    GexDatabaseFilter dbFilter;

    // Fill current filters
    FillDatabaseFilter(dbFilter, comboBoxTCFilter5->currentText());

    PickFilterFromLiveList(dbFilter, comboBoxTCValue5, false);
}

///////////////////////////////////////////////////////////
// Display list of Selections valid for split
///////////////////////////////////////////////////////////
void	GexOneQueryWizardPage1::OnPickSplitFields()
{
    // Open the dialog to select the fields
    PickFieldsDialog fieldsDialog(mSplitFieldsAvailable, this);

    // Set the initial selected fields
    fieldsDialog.setSelectedFieldList(lineEditSplitField->text().split(",", QString::SkipEmptyParts));

    // Show the dialog box
    if (fieldsDialog.exec() == QDialog::Accepted)
        lineEditSplitField->setText(fieldsDialog.selectedFieldList().join(","));
}

///////////////////////////////////////////////////////////
// Display list of Selections valid for test conditions
///////////////////////////////////////////////////////////
void	GexOneQueryWizardPage1::OnPickTestCondtions()
{
    if (mTestConditionsAvailable.isEmpty() == false)
    {
//        // Open the dialog to select the fields
//        PickFieldsDialog fieldsDialog(lTestConditions , this);

//        // Set the initial selected fields
//        fieldsDialog.setSelectedFieldList(lineEditTestCondition->text().split(",", QString::SkipEmptyParts));

//        // Show the dialog box
//        if (fieldsDialog.exec() == QDialog::Accepted)
//            SetSelectedTestConditions(fieldsDialog.selectedFieldList());

//        GS::Gex::TestConditionLevelWidget ConditionWidget(this);

//        ConditionWidget.SetAvailableFields(mTestConditionsAvailable);
//        ConditionWidget.SetSelectedFields(lineEditTestCondition->text().split(",", QString::SkipEmptyParts));

//        ConditionWidget.exec();

//        SetSelectedTestConditions(ConditionWidget.GetSelectedFields());
    }
    else
        GSLOG(SYSLOG_SEV_WARNING, "No test conditions available for the current filter");
}

///////////////////////////////////////////////////////////
// User asked to select parameters from a list
// Note: only applies to SQL databases (Galaxy, mySQL,...)
///////////////////////////////////////////////////////////
void GexOneQueryWizardPage1::OnPickParameterList(void)
{
    // Get user selection
    QString strSelection = SqlPickParameterList(false);

    // If valid, save the list selected into the edit field...
    if(!strSelection.isEmpty())
        lineEditParameterList->setText(strSelection);
}


///////////////////////////////////////////////////////////
// User has changed a Query filter...
///////////////////////////////////////////////////////////
void	GexOneQueryWizardPage1::FilterChanged(const QString &strString)
{
    // Check if user combining ? or * with the '|' character: it's not allowed!
    if(((strString.indexOf("*") >= 0) || (strString.indexOf("?") >= 0)) &&
        (strString.indexOf("|") >= 0))
    {
        GS::Gex::Message::information(
            "", "You can't combine the '*' or '?' wildcar\n"
            "with the OR '|' character. Use either one grammar.");
    }

    // Reset HTML sections to create flag: ALL pages to create.
    if(pGexMainWindow != NULL)
    {
        pGexMainWindow->iHtmlSectionsToSkip = 0;
        pGexMainWindow->m_bDatasetChanged	= true;
    }
}

void GexOneQueryWizardPage1::OnStandardFilterChange(const QString & filter)
{
    // Test conditions are not synchronized anymore as the filters have changed
    SetSynchronizedTestConditions(false);

    // Change some status as the filter has changed
    FilterChanged(filter);
}

void GexOneQueryWizardPage1::OnTCFilterChange(const QString & filter)
{
    QHash<QString, QString> filters;
    if (comboBoxTCFilter1->currentText() != gexLabelFilterChoices[GEX_QUERY_FILTER_NONE] &&
        comboBoxTCValue1->currentText() != "*")
        filters.insert(comboBoxTCFilter1->currentText(), comboBoxTCValue1->currentText());

    if (comboBoxTCFilter2->currentText() != gexLabelFilterChoices[GEX_QUERY_FILTER_NONE] &&
        comboBoxTCValue2->currentText() != "*")
        filters.insert(comboBoxTCFilter2->currentText(), comboBoxTCValue2->currentText());

    if (comboBoxTCFilter3->currentText() != gexLabelFilterChoices[GEX_QUERY_FILTER_NONE] &&
        comboBoxTCValue3->currentText() != "*")
        filters.insert(comboBoxTCFilter3->currentText(), comboBoxTCValue3->currentText());

    if (comboBoxTCFilter4->currentText() != gexLabelFilterChoices[GEX_QUERY_FILTER_NONE] &&
        comboBoxTCValue4->currentText() != "*")
        filters.insert(comboBoxTCFilter4->currentText(), comboBoxTCValue4->currentText());

    if (comboBoxTCFilter5->currentText() != gexLabelFilterChoices[GEX_QUERY_FILTER_NONE] &&
        comboBoxTCValue5->currentText() != "*")
        filters.insert(comboBoxTCFilter5->currentText(), comboBoxTCValue5->currentText());

    widgetConditionHierarchy->SetFilters(filters);

    // Change some status as the filter has changed
    FilterChanged(filter);
}

///////////////////////////////////////////////////////////
// User want to use the calendar object to set 'From' date
///////////////////////////////////////////////////////////
void	GexOneQueryWizardPage1::OnFromDateCalendar(void)
{
    // Create Calendar object
    CalendarDialog *pCalendar = new CalendarDialog();
    pCalendar->setDate(From);

    // Show calendar, let user pick a date
    if(pCalendar->exec() != 1)
        return;

    // Read date picked by the user
    From = pCalendar->getDate();

    // Reset HTML sections to create flag: ALL pages to create.
    OnStandardFilterChange("");

    // Update GUI accordingly
    UpdateFromToFields(false);
}

///////////////////////////////////////////////////////////
// User want to use the calendar object to set 'To' date
///////////////////////////////////////////////////////////
void	GexOneQueryWizardPage1::OnToDateCalendar(void)
{
    // Create Calendar object
    CalendarDialog *pCalendar = new CalendarDialog();
    pCalendar->setDate(To);

    // Show calendar, let user pick a date
    if(pCalendar->exec() != 1)
        return;

    // Read date picked by the user
    To = pCalendar->getDate();

    // Reset HTML sections to create flag: ALL pages to create.
    OnStandardFilterChange("");

    // Update GUI accordingly
    UpdateFromToFields(false);
}

///////////////////////////////////////////////////////////
// User want to use the Date SpinWheel object to set 'From' date
///////////////////////////////////////////////////////////
void	GexOneQueryWizardPage1::OnFromDateSpinWheel(const QDate& Fromdate)
{
    // If no change...ignore (also avoids infinite call due to 'UpdateFromToFields')
    if(From == Fromdate)
        return;

    // Update variable
    From = Fromdate;

    // Reset HTML sections to create flag: ALL pages to create.
    OnStandardFilterChange("");

    // Update GUI accordingly
    UpdateFromToFields(false);
}

///////////////////////////////////////////////////////////
// User want to use the Date SpinWheel object to set 'To' date
///////////////////////////////////////////////////////////
void	GexOneQueryWizardPage1::OnToDateSpinWheel(const QDate& Todate)
{
    // If no change...ignore (also avoids infinite call due to 'UpdateFromToFields')
    if(To == Todate)
        return;

    // Update variable
    To = Todate;

    // Reset HTML sections to create flag: ALL pages to create.
    OnStandardFilterChange("");

    // Update GUI accordingly
    UpdateFromToFields(false);
}

///////////////////////////////////////////////////////////
// User want to use the Time SpinWheel object to set 'From' time
///////////////////////////////////////////////////////////
void	GexOneQueryWizardPage1::OnFromTimeSpinWheel(const QTime& NewFromTime)
{
    // If no change...ignore (also avoids infinite call due to 'UpdateFromToFields')
    if(FromTime == NewFromTime)
        return;

    // Update variable
    FromTime = NewFromTime;

    // Reset HTML sections to create flag: ALL pages to create.
    OnStandardFilterChange("");

    // Update GUI accordingly
    UpdateFromToFields(false);
}

///////////////////////////////////////////////////////////
// User want to use the Time SpinWheel object to set 'To' time
///////////////////////////////////////////////////////////
void	GexOneQueryWizardPage1::OnToTimeSpinWheel(const QTime& NewToTime)
{
    // If no change...ignore (also avoids infinite call due to 'UpdateFromToFields')
    if(ToTime == NewToTime)
        return;

    // Update variable
    ToTime = NewToTime;

    // Reset HTML sections to create flag: ALL pages to create.
    OnStandardFilterChange("");

    // Update GUI accordingly
    UpdateFromToFields(false);
}


///////////////////////////////////////////////////////////
// Changing the TimePeriod selection...
///////////////////////////////////////////////////////////
void GexOneQueryWizardPage1::OnTimePeriod(void)
{
    // Change Filter position...depends if the Calendar is visible or not.
    if(comboBoxTimePeriod->currentIndex() != GEX_QUERY_TIMEPERIOD_CALENDAR
            && comboBoxTimePeriod->currentIndex() != GEX_QUERY_TIMEPERIOD_LAST_N_X)
    {
        // Hide Calendar
        TextLabelFrom->hide();
        FromDate->hide();
        FromTimeEdit->hide();
        FromDateCalendar->hide();
        TextLabelFromDate->hide();
        TextLabelTo->hide();
        ToDate->hide();
        ToTimeEdit->hide();
        ToDateCalendar->hide();
        TextLabelToDate->hide();
        iCalendarOffset=0;	// Hold Calendar Hights (if visible)
        comboBoxLastNXStep->hide();
        lineEditLastNXFactor->hide();
    }
    else if (comboBoxTimePeriod->currentIndex() == GEX_QUERY_TIMEPERIOD_LAST_N_X)
    {
        TextLabelFrom->hide();
        FromDate->hide();
        FromTimeEdit->hide();
        FromDateCalendar->hide();
        TextLabelFromDate->hide();
        TextLabelTo->hide();
        ToDate->hide();
        ToTimeEdit->hide();
        ToDateCalendar->hide();
        TextLabelToDate->hide();
        comboBoxLastNXStep->show();
        lineEditLastNXFactor->show();
    }
    else
    {
        // Show Calendar
        comboBoxLastNXStep->hide();
        lineEditLastNXFactor->hide();
        TextLabelFrom->show();
        FromDate->show();
        FromTimeEdit->show();
        FromDateCalendar->show();
        TextLabelFromDate->show();
        TextLabelTo->show();
        ToDate->show();
        ToTimeEdit->show();
        ToDateCalendar->show();
        TextLabelToDate->show();
        iCalendarOffset=60;	// Hold Calendar Hights (if visible)
    }
    // Resize dialog if in pop-up mode
    if(bPopupMode == true)
    {
        if(m_iDialogMode == eHouseKeeping)
            resize(580,310+iCalendarOffset);
        else
            resize(670,370+iCalendarOffset);
    }

    // Reset HTML sections to create flag: ALL pages to create.
    OnStandardFilterChange("");
}

///////////////////////////////////////////////////////////
// If Dataset title entered, allow to go to next page
///////////////////////////////////////////////////////////
void GexOneQueryWizardPage1::OnDatasetName(void)
{
    QString strQueryTitle = lineEditReportTitle->text();
    strQueryTitle = strQueryTitle.trimmed();

    if(strQueryTitle.isEmpty() == true)
        pushButtonNext->setEnabled(false);	// Empty title: Can't go to next page!
    else
        pushButtonNext->setEnabled(true);	// Got title: can move on!

    // Reset HTML sections to create flag: ALL pages to create.
    OnStandardFilterChange("");
}

///////////////////////////////////////////////////////////
// User changged Offline switch
///////////////////////////////////////////////////////////
void GexOneQueryWizardPage1::OnOfflineSwitch(void)
{
    OnDatabaseChanged(true);
}

///////////////////////////////////////////////////////////
// User selecting a database
///////////////////////////////////////////////////////////
void GexOneQueryWizardPage1::OnSelectDatabase(void)
{
    OnDatabaseChanged();
}


///////////////////////////////////////////////////////////
// Update filters
///////////////////////////////////////////////////////////
void GexOneQueryWizardPage1::OnUpdateFilters(void)
{
    GSLOG(SYSLOG_SEV_DEBUG, "GexOneQueryWizardPage1::OnUpdateFilters");
    // Get database pointer
    GexDatabaseEntry	*pDatabaseEntry=NULL;
    QString				strDatabaseLogicalName = comboBoxDatabaseName->currentText();
    if(strDatabaseLogicalName.startsWith("[Local]") || strDatabaseLogicalName.startsWith("[Server]"))
        strDatabaseLogicalName = strDatabaseLogicalName.section("]",1).trimmed();
    pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(strDatabaseLogicalName);

    // Do nothing if database not found
    if(!pDatabaseEntry)
        return;

    // Call update function
    UpdateFilters(pDatabaseEntry);
    UpdateExtractionModeUi();
    if(mPurgeDb)
        checkBoxOfflineQuery->hide();
    }

void GexOneQueryWizardPage1::OnTabChanged(int)
{
    if (TabWidgetFilters->currentWidget() == tabTestConditions)
    {
        UpdateTestConditions();
    }
}


///////////////////////////////////////////////////////////
// Filter toggle button: 'More >>' or 'Less <<'
///////////////////////////////////////////////////////////
void GexOneQueryWizardPage1::OnMoreFilters(void)
{
    if(bMoreFilters == true)
    {
        // Reduced list of filters
        bMoreFilters = false;

        // Show only 2 filters
        comboBoxFilter3->hide();
        comboBoxFilter4->hide();
        comboBoxFilter5->hide();
        comboBoxFilter6->hide();
        comboBoxFilter7->hide();
        comboBoxFilter8->hide();
        FilterString3->hide();
        FilterString4->hide();
        FilterString5->hide();
        FilterString6->hide();
        FilterString7->hide();
        FilterString8->hide();
        PickFilter3->hide();
        PickFilter4->hide();
        PickFilter5->hide();
        PickFilter6->hide();
        PickFilter7->hide();
        PickFilter8->hide();

        // Button title
        pushButtonMoreFilters->setText("More >>");
    }
    else
    {
        // Maximize the list of filters
        bMoreFilters = true;

        // Show ALL filters available...
        // Show only 2 filters
        comboBoxFilter3->show();
        comboBoxFilter4->show();
        comboBoxFilter5->show();
        comboBoxFilter6->show();
        comboBoxFilter7->show();
        comboBoxFilter8->show();
        FilterString3->show();
        FilterString4->show();
        FilterString5->show();
        FilterString6->show();
        FilterString7->show();
        FilterString8->show();
        PickFilter3->show();
        PickFilter4->show();
        PickFilter5->show();
        PickFilter6->show();
        PickFilter7->show();
        PickFilter8->show();

        // Button title
        pushButtonMoreFilters->setText("Less <<");
    }

    // Resize dialog if in pop-up mode
    if(bPopupMode == true)
    {
        if(m_iDialogMode == eHouseKeeping)
            resize(580,310+iCalendarOffset);
        else
            resize(670,370+iCalendarOffset);
    }
}

///////////////////////////////////////////////////////////
// Combo to select type of parts to process
///////////////////////////////////////////////////////////
void GexOneQueryWizardPage1::OnComboProcessChange(void)
{
    GSLOG(SYSLOG_SEV_DEBUG, "GexOneQueryWizardPage1::OnComboProcessChange");
    int iComboPosition=DataTypeFilter->currentIndex();
    switch(iComboPosition)
    {
    case GEX_PROCESSPART_ALL:		// Process ALL Bins
    case GEX_PROCESSPART_GOOD:		// Process Good bins only
    case GEX_PROCESSPART_FAIL:		// Process Fail bins only
    case GEX_PROCESSPART_ODD:		// Process Odd parts only (1,3,5...)
    case GEX_PROCESSPART_EVEN:		// Process Odd parts only (2,4,6...)
        lineEditProcess->hide();
        break;

    case GEX_PROCESSPART_PARTLIST:		// Process list of parts.
    case GEX_PROCESSPART_EXPARTLIST:	// All parts except...
    case GEX_PROCESSPART_SBINLIST:		// Process list of Soft binnings
    case GEX_PROCESSPART_EXSBINLIST:	// All Soft bins except...
    case GEX_PROCESSPART_HBINLIST:		// Process list of Soft binnings
    case GEX_PROCESSPART_EXHBINLIST:	// All Soft bins except...
    case GEX_PROCESSPART_PARTSINSIDE:	// Parts inside...
    case GEX_PROCESSPART_PARTSOUTSIDE:	// Parts outside...
        lineEditProcess->show();
        lineEditProcess->setFocus();
        break;

    case GEX_PROCESSPART_FIRSTINSTANCE:		// Process first test instance
    case GEX_PROCESSPART_LASTINSTANCE:		// Process last test instance
        break;
    }

    // Reset HTML sections to create flag: ALL pages to create.
    OnStandardFilterChange("");
}

void GexOneQueryWizardPage1::OnConsolidatedChange(void)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("On Consolidated Change : new selected : %1").arg( comboBoxExtractionMode->currentText()).toLatin1().constData());

    if (comboBoxExtractionMode->currentIndex()==2)
    {
        mExtractionModeLineEdit->setValidator(new QIntValidator( 2, 65535, this));
        mExtractionModeLineEdit->show();
        if (ReportOptions.GetOption("statistics","computation").toString() == "summary_only" )
            GS::Gex::Message::warning(
                "Warning",
                "You have chosen to query based on 1 out of n sampling of "
                "parametric results, however the statistics computation "
                "method is set to 'Summary'.\n"
                "Summary statistics are based on data from the stdf summary "
                "fields and not the parametric values.\n"
                "Click OK to continue with Summary Statistics, or, to see "
                "the statistics based on your 1 out of n sampling of your "
                "data, change the Test Statistics > How are stats computed "
                "to 'From Samples Data Only'.");
    }
    else
        mExtractionModeLineEdit->hide();

    if (!comboBoxDatabaseType->currentText().toLower().startsWith("wafer") &&
            !comboBoxDatabaseType->currentText().toLower().startsWith("final"))
        return;

    if (comboBoxExtractionMode->currentText().toLower().startsWith("consolidated"))
        DataTypeFilter->setCurrentIndex(12);
    else
        DataTypeFilter->setCurrentIndex(0);
}

void GexOneQueryWizardPage1::OnCreateMapTests(void)
{
    QString	strFilePath;


    // Current Map file in the Edit field...
    strFilePath = LineEditMapTests->text();
    strFilePath = strFilePath.trimmed();
    if(strFilePath.isEmpty() == true)
    {
        // No file assigned...use template.
        strFilePath = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
        strFilePath += "/samples/gex_dataset_config.xml";
    }

    GexMainwindow::OpenFileInEditor(strFilePath, GexMainwindow::eDocUnknown);

//    if (ReportOptions.GetOption("preferences","use_os_default").toBool() && QDesktopServices::openUrl(QUrl(QString("file:///%1").arg(strFilePath)))) //Case 4619 - Use OS's default program to open file
//      return;
//#ifdef _WIN32
//    // If file includes a space, we need to bath it between " "
//    if(strFilePath.indexOf(' ') != -1)
//    {
//        strFilePath = "\"" + strFilePath;
//        strFilePath = strFilePath + "\"";
//    }
//    ShellExecuteA(NULL,
//        "open",
//        ReportOptions.GetOption("preferences", "text_editor").toString().toLatin1().data(),	//ReportOptions.strTextEditor.toLatin1().constData(),
//           strFilePath.toLatin1().constData(),
//           NULL,
//           SW_SHOWNORMAL);
//#endif
//#if defined unix || __MACH__
//    char	szString[2048];
//    sprintf(szString,"%s %s&",
//        ReportOptions.GetOption("preferences", "text_editor").toString().toLatin1().data(),	//ReportOptions.strTextEditor.toLatin1().constData(),
//        strFilePath.toLatin1().constData());
//  if (system(szString) == -1) {
//    //FIXME: send error
//  }
//#endif
}

///////////////////////////////////////////////////////////
// Select a .CSV Test Number mapping file...
///////////////////////////////////////////////////////////
void GexOneQueryWizardPage1::OnMapTests(void)
{
    QString fn;

    // User wants to select a Test# mapping file...
    fn = QFileDialog::getOpenFileName(this, "Select dataset config File", "", "Dataset config file (*.csv;*.txt;*.xml)");

    if(fn.isEmpty())
        return;

    LineEditMapTests->setText(fn);

    // Reset HTML sections to create flag: ALL pages to create.
    OnStandardFilterChange("");
}

