///////////////////////////////////////////////////////////
// ALL Browser wizard pages for 'Analyze Dataset' (1 query)
///////////////////////////////////////////////////////////
#ifdef _WIN32
  #include <windows.h>
#endif

#include "db_onequery_wizard.h"
#include "browser_dialog.h"
#include "engine.h"
#include "settings_dialog.h"
#include "report_build.h"
#include "report_options.h"
#include "calendar_dialog.h"
#include "picktest_dialog.h"
#include "db_transactions.h"
#include "gex_database_entry.h"
#include "pickfilter_dialog.h"
#include "filter_dialog.h"
#include "gex_shared.h"
#include "gex_constants.h"
#include "db_engine.h"
#include "db_external_database.h"
#include "db_single_query_filterform_dialog.h"
#include <gqtl_log.h>
#include "product_info.h"
#include "script_wizard.h"
#include "csl/csl_engine.h"
#include "admin_gui.h"
#include "message.h"

#include <qtooltip.h>

// in main.cpp
extern GexMainwindow *	pGexMainWindow;
// in script_wizard.h
extern void ConvertToScriptString(QString &strFile);

// report_build.cpp
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)

// in classes.cpp
extern bool SetCurrentComboItem(QComboBox *pCombo, const QString & strItem, bool bInsertIfMissing=false);

extern bool FillComboBox(QComboBox * pCombo, const char * szTextTab[]);

extern GexMainwindow	*pGexMainWindow;

bool GexMainwindow::SetWizardType(int lWizardType)
{
    if( (lWizardType < GEX_WIZARD_MIN) || (lWizardType > GEX_WIZARD_MAX) )
    {
        GSLOG(SYSLOG_SEV_ERROR, "Try to set invalid wizard type");
        GEX_ASSERT(false);
        return false;
    }

    mWizardType = lWizardType;
    //emit sWizardTypeChanged();

    return true;
}

int GexMainwindow::GetWizardType() const
{
    return mWizardType;
}

///////////////////////////////////////////////////////////
// WIZARD PAGE 1: Enter Query
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_OneQuery_Page1(void)
{
    int	iDatabasesEntries;

    // Check if data access type if FILES or DATABASE
    if(!GS::LPPlugin::ProductInfo::getInstance()->isDBPluginAllowed())
    {
//        // DATABASE type
//        case GEX_DATATYPE_GEX_MONITORING:	// ExaminatorMonitoring
//        case GEX_DATATYPE_GEX_PATMAN:		// Galaxy PAT-Man
//        case GEX_DATATYPE_ALLOWED_DATABASE:	// ExaminatorDB: Database access, any file type.
//        case GEX_DATATYPE_GEX_YIELD123:		// Yield123
//        case GEX_DATATYPE_ALLOWED_DATABASEWEB: // ExaminatorWeb
//            break;

//        default:
            QString m=ReportOptions.GetOption("messages", "upgrade").toString();
            GS::Gex::Message::information("", m);
            return;
    }

    // Make sure ExaminatorDB has the latest list of databases (do not repaint Admin GUI page)
    iDatabasesEntries = ReloadDatabasesList(false);

    if(iDatabasesEntries > 0)
    {
        // Fill the combo box on the 'Single Query' page with the list of databases
        QComboBox *pValue = pWizardOneQuery->comboBoxDatabaseName;
        QString strCurrentSelection = pValue->currentText();
        // Show only uptodate DB (including filebased) because querying old DB is not supported
        pWizardAdminGui->ListDatabasesInCombo(pWizardOneQuery->comboBoxDatabaseName,DB_SELECT_FOR_EXTRACTION);
        if(strCurrentSelection.isEmpty() == false)
            // Make sure we keep the database name selected.
            SetCurrentComboItem(pValue, strCurrentSelection);
    }

    // If No database available...display the HTML message instead
    if(pWizardOneQuery->comboBoxDatabaseName->count() == 0)
    {
        // No report available
        QString strFilesPage;

        // Display relevant message
        if(GS::LPPlugin::ProductInfo::getInstance()->isDBPluginAllowed())
        {
//        case GEX_DATATYPE_ALLOWED_DATABASE:
//        case GEX_DATATYPE_GEX_MONITORING:	// ExaminatorMonitoring
//        case GEX_DATATYPE_GEX_PATMAN:		// Galaxy PAT-Man
            strFilesPage = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
            strFilesPage += GEX_HELP_FOLDER;
            strFilesPage += GEX_HTMLPAGE_DB_NODATABASE;
//            break;
//        case GEX_DATATYPE_GEX_YIELD123:		// Yield123
//            strFilesPage = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
//            strFilesPage += GEX_HELP_FOLDER;
//            strFilesPage += GEXY123_HTMLPAGE_NODATABASE;
//            break;

//        case GEX_DATATYPE_ALLOWED_DATABASEWEB:
//            strFilesPage = strExaminatorWebUserHome;
//            strFilesPage += GEX_HELP_FOLDER;
//            strFilesPage += GEX_HTMLPAGE_WEB_NODATABASE;
//            break;
        }

        LoadUrl(strFilesPage);

        return;
    }

    // Updates Wizard type
    // GCore-17482
    // Update the Wizard type before the OnDoInstantReport
    SetWizardType(GEX_ONEQUERY_WIZARD);

    // Force Report mode = Standard reports (not Enterprise ones)
    pGexMainWindow->pWizardSettings->OnDoInstantReport();

    // Show wizard page : Query dataset.
    ShowWizardDialog(GEX_ONEQUERY_WIZARD_P1);
}

///////////////////////////////////////////////////////////
// Manage the Query page
///////////////////////////////////////////////////////////
GexOneQueryWizardPage1::GexOneQueryWizardPage1( QWidget* parent, bool modal, Qt::WindowFlags fl, bool bUsedForPurge)
    : QDialog(parent, fl)
{
    setupUi(this);
    setModal(modal);

    // apply gex palette
    GexMainwindow::applyPalette(this);

    QObject::connect(PushButtonOk,				SIGNAL(clicked()),				this, SLOT(accept()));
    QObject::connect(DataTypeFilter,			SIGNAL(activated(QString)),		this, SLOT(OnComboProcessChange()));
    QObject::connect(pushButtonCreateMapTests,	SIGNAL(clicked()),				this, SLOT(OnCreateMapTests()));
    QObject::connect(lineEditReportTitle,		SIGNAL(textChanged(QString)),	this, SLOT(OnDatasetName()));
    QObject::connect(comboBoxFilter4,			SIGNAL(activated(QString)),		this, SLOT(OnStandardFilterChange(QString)));
    QObject::connect(FilterString1,				SIGNAL(editTextChanged(QString)),	this, SLOT(OnStandardFilterChange(QString)));
    QObject::connect(FilterString2,				SIGNAL(editTextChanged(QString)),	this, SLOT(OnStandardFilterChange(QString)));
    QObject::connect(comboBoxFilter2,			SIGNAL(activated(QString)),		this, SLOT(OnStandardFilterChange(QString)));
    QObject::connect(comboBoxTimePeriod,		SIGNAL(editTextChanged(QString)),	this, SLOT(OnStandardFilterChange(QString)));
    QObject::connect(comboBoxFilter3,			SIGNAL(activated(QString)),		this, SLOT(OnStandardFilterChange(QString)));
    QObject::connect(comboBoxDatabaseName,		SIGNAL(activated(QString)),		this, SLOT(OnStandardFilterChange(QString)));
    QObject::connect(comboBoxFilter1,			SIGNAL(activated(QString)),		this, SLOT(OnStandardFilterChange(QString)));
    QObject::connect(FilterString3,				SIGNAL(editTextChanged(QString)),	this, SLOT(OnStandardFilterChange(QString)));
    QObject::connect(FilterString5,				SIGNAL(editTextChanged(QString)),	this, SLOT(OnStandardFilterChange(QString)));
    QObject::connect(comboBoxFilter5,			SIGNAL(activated(QString)),		this, SLOT(OnStandardFilterChange(QString)));
    QObject::connect(FilterString4,				SIGNAL(editTextChanged(QString)),	this, SLOT(OnStandardFilterChange(QString)));
    QObject::connect(FromDateCalendar,			SIGNAL(clicked()),				this, SLOT(OnFromDateCalendar()));
    QObject::connect(FromDate,					SIGNAL(dateChanged(QDate)),		this, SLOT(OnFromDateSpinWheel(QDate)));
    QObject::connect(FromTimeEdit,				SIGNAL(timeChanged(QTime)),		this, SLOT(OnFromTimeSpinWheel(QTime)));
    QObject::connect(pushButtonMapTests,		SIGNAL(clicked()),				this, SLOT(OnMapTests()));
    QObject::connect(pushButtonMoreFilters,		SIGNAL(clicked()),				this, SLOT(OnMoreFilters()));
    QObject::connect(PickFilter1,				SIGNAL(clicked()),				this, SLOT(OnPickFilter1()));
    QObject::connect(PickFilter2,				SIGNAL(clicked()),				this, SLOT(OnPickFilter2()));
    QObject::connect(PickFilter3,				SIGNAL(clicked()),				this, SLOT(OnPickFilter3()));
    QObject::connect(PickFilter4,				SIGNAL(clicked()),				this, SLOT(OnPickFilter4()));
    QObject::connect(PickFilter5,				SIGNAL(clicked()),				this, SLOT(OnPickFilter5()));
    QObject::connect(comboBoxTimePeriod,		SIGNAL(activated(QString)),		this, SLOT(OnTimePeriod()));
    QObject::connect(ToDateCalendar,			SIGNAL(clicked()),				this, SLOT(OnToDateCalendar()));
    QObject::connect(ToDate,					SIGNAL(dateChanged(QDate)),		this, SLOT(OnToDateSpinWheel(QDate)));
    QObject::connect(ToTimeEdit,				SIGNAL(timeChanged(QTime)),		this, SLOT(OnToTimeSpinWheel(QTime)));
    QObject::connect(PushButtonCancel,			SIGNAL(clicked()),				this, SLOT(reject()));
    QObject::connect(comboBoxDatabaseName,		SIGNAL(activated(QString)),     this, SLOT(OnSelectDatabase()));
    QObject::connect(comboBoxDatabaseType,		SIGNAL(activated(QString)),		this, SLOT(OnUpdateFilters()));
    QObject::connect(comboBoxExtractionMode,	SIGNAL(activated(QString)),		this, SLOT(OnConsolidatedChange()));
    QObject::connect(checkBoxOfflineQuery,      SIGNAL(stateChanged(int)),      this, SLOT(OnOfflineSwitch()));
    QObject::connect(pushButtonAllFilters,		SIGNAL(clicked()),				this, SLOT(OnAllFilters()));
    QObject::connect(FilterString6,				SIGNAL(editTextChanged(QString)),	this, SLOT(OnStandardFilterChange(QString)));
    QObject::connect(FilterString7,				SIGNAL(editTextChanged(QString)),	this, SLOT(OnStandardFilterChange(QString)));
    QObject::connect(FilterString8,				SIGNAL(editTextChanged(QString)),	this, SLOT(OnStandardFilterChange(QString)));
    QObject::connect(comboBoxFilter6,			SIGNAL(activated(QString)),		this, SLOT(OnStandardFilterChange(QString)));
    QObject::connect(comboBoxFilter7,			SIGNAL(activated(QString)),		this, SLOT(OnStandardFilterChange(QString)));
    QObject::connect(comboBoxFilter8,			SIGNAL(activated(QString)),		this, SLOT(OnStandardFilterChange(QString)));
    QObject::connect(PickFilter6,				SIGNAL(clicked()),				this, SLOT(OnPickFilter6()));
    QObject::connect(PickFilter7,				SIGNAL(clicked()),				this, SLOT(OnPickFilter7()));
    QObject::connect(PickFilter8,				SIGNAL(clicked()),				this, SLOT(OnPickFilter8()));
    QObject::connect(toolButtonSplitField,      SIGNAL(clicked()),				this, SLOT(OnPickSplitFields()));
    QObject::connect(TabWidgetFilters,          SIGNAL(currentChanged(int)),    this, SLOT(OnTabChanged(int)));
    QObject::connect(toolButtonPickTC1,			SIGNAL(clicked()),              this, SLOT(OnPickTCFilter1()));
    QObject::connect(toolButtonPickTC2,			SIGNAL(clicked()),              this, SLOT(OnPickTCFilter2()));
    QObject::connect(toolButtonPickTC3,			SIGNAL(clicked()),              this, SLOT(OnPickTCFilter3()));
    QObject::connect(toolButtonPickTC4,			SIGNAL(clicked()),              this, SLOT(OnPickTCFilter4()));
    QObject::connect(toolButtonPickTC5,			SIGNAL(clicked()),              this, SLOT(OnPickTCFilter5()));
    QObject::connect( pushButtonNext,           SIGNAL(clicked()),              this, SLOT(OnExecuteAction()));

    connect(widgetConditionLevel,           &GS::Gex::TestConditionLevelWidget::levelChanged,
            this, &GexOneQueryWizardPage1::OnConditionsLevelChanged);
    connect(pushButtonBuildHierarchy,       &QPushButton::clicked,
            this, &GexOneQueryWizardPage1::BuildHierarchy);
    connect(radioButtonRebuildHierarchy,    &QRadioButton::toggled /*bool*/,
            this, &GexOneQueryWizardPage1::OnRebuildHierarchyOnDemandSelected /*bool*/);
    connect(radioButtonHierarchyUpToDate,   &QRadioButton::toggled /*bool*/,
            this, &GexOneQueryWizardPage1::OnKeepHierarchyUpToDateSelected /*bool*/);
    connect(widgetConditionHierarchy,       &GS::Gex::TestConditionHierarchyWidget::sEmptyCond /*QString*/,
            this, &GexOneQueryWizardPage1::OnNullConditionDetected /*QString*/);


    radioButtonRebuildHierarchy->setChecked(true);

    ConnectTCFilterControls(true);

    // Clear flag
    m_iDialogMode = eCreate;
    bPopupMode = false;
    iCalendarOffset = 0;	// Hold Calendar Hights (if visible)
    m_bSqlDatabase = false;
    m_pRdbOptionsWidget = NULL;
    labelPlaceHolder->setText("");
    m_bRdbOptionsInitialized = false;

    // Set Test Condition Synchronized flag to false
    mSynchronizedTestCond = false;

    // Until a file is selected, disable the 'Next' & properties buttons
    pushButtonNext->setEnabled(false);

    // Hide/Show relevant widgets
    pushButtonAllFilters->hide();
    PushButtonOk->hide();
    PushButtonCancel->hide();
    comboBoxDatabaseType->hide();	// Will be made visible if a plug-in is defined
    checkBoxOfflineQuery->hide();	// Will be made visible if a plug-in is defined

    // Default date filter is ALL Dates
    comboBoxTimePeriod->setCurrentIndex(GEX_QUERY_TIMEPERIOD_ALLDATES);

    // Default tab: the first
    TabWidgetFilters->setCurrentIndex(0);

    lineEditLastNXFactor->setValidator(new QIntValidator( 1, 65535, this));

    // Has to be set from options later (options not loaded yet)
    lineEditParameterList->setText("");

    InitComboBoxExtractionMode();

    // Default Calendar dates are 'Today'
    QDate Today = QDate::currentDate();
    From = To = Today;
    FromTime = QTime(0,0);
    ToTime = QTime(23,59,59);

    // Initialise the Filter fields
    FillComboBox(DataTypeFilter, ProcessPartsItems);

    // Update GUI accordingly: Update Calendar
    UpdateFromToFields(false);

    // Update 'Filters' position according to calendar visible/hidden
    OnTimePeriod();

    // Default: only show the first 2 Filter fields
    bMoreFilters=true;	// Set to true, so following line sets it to false! Reduced list of Filters shown.
    OnMoreFilters();

    // Update Filter Combo boxes status
    OnComboProcessChange();

    // Focus on Title edit field
    lineEditReportTitle->setText("New Query");
    lineEditReportTitle->setFocus();

    // Update filters and othet GUI controls
    OnDatabaseChanged();

    // Enable/disable some features...
    UpdateSkin(GS::LPPlugin::ProductInfo::getInstance()->getProductID());

    // Connect signals
    connect((QObject *)buttonPickParameterList,SIGNAL(clicked()),this,SLOT(OnPickParameterList(void)));
    mPurgeDb = bUsedForPurge;
    if (mPurgeDb)
    {
        while(TabWidgetFilters->count() >1)
        {
            TabWidgetFilters->removeTab(1);
        }
        comboBoxDatabaseType->show();
        gridLayout_4->addWidget(new QLabel("Data Type: ",comboBoxDatabaseType->parentWidget()), 3, 0, 1, 2);
        comboBoxDatabaseType->setCurrentIndex(0);
        comboBoxDatabaseName->setEnabled(false);
        comboBoxDatabaseName->hide();
        TextLabel2->hide();
        lineEditReportTitle->hide();
        TextLabelTitle->hide();
        line->hide();
        checkBoxOfflineQuery->hide();
        TextLabelHeaderTitle->setText("Define your query to purge the database");
    }
    UpdateExtractionModeUi();
    InitNextActionFrame();
}

void GexOneQueryWizardPage1::OnExecuteAction()
{
    NextAction lAction = static_cast<NextAction>(
                comboBoxNextAction->itemData(comboBoxNextAction->currentIndex()).toInt());

    if (mPurgeDb)
        emit sExecuteAction();
    else
    {
        if (lAction == BUILDREPORT)
            pGexMainWindow->ViewSettings();
        else if (lAction == EXPORTTOCSV)
        {
            if (pGexMainWindow && pGexMainWindow->GetWizardType() == GEX_CHAR_QUERY_WIZARD)
                ExportToCSVConditionFormat();
            else
                ExportQueryToCSV();
        }
    }
}
///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexOneQueryWizardPage1::~GexOneQueryWizardPage1(void)
{
    ResetRdbPluginHandles();
}

///////////////////////////////////////////////////////////
// GexOneQueryWizardPage1: Empty function to ignore the ESCAPE key hit!
// ...unless it's displayed as a child dialog box when comparing
// datasets.
///////////////////////////////////////////////////////////
void GexOneQueryWizardPage1::reject(void)
{
    switch(m_iDialogMode)
    {
        case eCreate:
        case eEdit:
            if(pGexMainWindow != NULL && pGexMainWindow->GetWizardType() == GEX_CMPQUERY_WIZARD)
                done(0);
            if(pGexMainWindow != NULL && pGexMainWindow->GetWizardType() == GEX_MIXQUERY_WIZARD)
                done(0);
            break;

        case ePATRecipe:
        case eHouseKeeping:
        case ePurge:
            done(0);
            break;
    }
}

///////////////////////////////////////////////////////////
// Show page: make it visible & update GUI fields
///////////////////////////////////////////////////////////
void GexOneQueryWizardPage1::ShowPage(void)
{
    // Enable/disable some features...
    UpdateSkin(GS::LPPlugin::ProductInfo::getInstance()->getProductID());

    // Make sure remote DB also detail the data type & 'off-line' flag
    OnDatabaseChanged(true);

    // Make Widget visible.
    show();
}

///////////////////////////////////////////////////////////
// Update GUI fileds based on Product type running
///////////////////////////////////////////////////////////
void GexOneQueryWizardPage1::UpdateSkin(int /*lProductID*/)
{
    // Nothing to do
}

void GexOneQueryWizardPage1::UpdateConditionsRender()
{
    if (mIsHierarchyUpToDate)
    {
        pushButtonBuildHierarchy->setText("Hierarchy Up To Date");
        pushButtonBuildHierarchy->setEnabled(false);
    }
    else
    {
        if (radioButtonRebuildHierarchy->isChecked())
        {
            pushButtonBuildHierarchy->setEnabled(true);
            pushButtonBuildHierarchy->setText("Rebuild Hierarchy");
        }
        else if (radioButtonHierarchyUpToDate)
        {
            pushButtonBuildHierarchy->setEnabled(false);
        }
    }
}

void GexOneQueryWizardPage1::ConnectTCFilterControls(bool lConnect)
{
    if (lConnect)
    {
        connect(comboBoxTCFilter1,	SIGNAL(currentIndexChanged(QString)),
                this,               SLOT(OnTCFilterChange(QString)));
        connect(comboBoxTCFilter2,	SIGNAL(currentIndexChanged(QString)),
                this,               SLOT(OnTCFilterChange(QString)));
        connect(comboBoxTCFilter3,	SIGNAL(currentIndexChanged(QString)),
                this,               SLOT(OnTCFilterChange(QString)));
        connect(comboBoxTCFilter4,	SIGNAL(currentIndexChanged(QString)),
                this,               SLOT(OnTCFilterChange(QString)));
        connect(comboBoxTCFilter5,	SIGNAL(currentIndexChanged(QString)),
                this,               SLOT(OnTCFilterChange(QString)));
        connect(comboBoxTCValue1,	SIGNAL(editTextChanged(QString)),
                this,               SLOT(OnTCFilterChange(QString)));
        connect(comboBoxTCValue2,	SIGNAL(editTextChanged(QString)),
                this,               SLOT(OnTCFilterChange(QString)));
        connect(comboBoxTCValue3,	SIGNAL(editTextChanged(QString)),
                this,               SLOT(OnTCFilterChange(QString)));
        connect(comboBoxTCValue4,	SIGNAL(editTextChanged(QString)),
                this,               SLOT(OnTCFilterChange(QString)));
        connect(comboBoxTCValue5,	SIGNAL(editTextChanged(QString)),
                this,               SLOT(OnTCFilterChange(QString)));
    }
    else
    {
        disconnect(comboBoxTCFilter1,	SIGNAL(currentIndexChanged(QString)),
                   this,                SLOT(OnTCFilterChange(QString)));
        disconnect(comboBoxTCFilter2,	SIGNAL(currentIndexChanged(QString)),
                   this,                SLOT(OnTCFilterChange(QString)));
        disconnect(comboBoxTCFilter3,	SIGNAL(currentIndexChanged(QString)),
                   this,                SLOT(OnTCFilterChange(QString)));
        disconnect(comboBoxTCFilter4,	SIGNAL(currentIndexChanged(QString)),
                   this,                SLOT(OnTCFilterChange(QString)));
        disconnect(comboBoxTCFilter5,	SIGNAL(currentIndexChanged(QString)),
                   this,                SLOT(OnTCFilterChange(QString)));
        disconnect(comboBoxTCValue1,	SIGNAL(editTextChanged(QString)),
                   this,                SLOT(OnTCFilterChange(QString)));
        disconnect(comboBoxTCValue2,	SIGNAL(editTextChanged(QString)),
                   this,                SLOT(OnTCFilterChange(QString)));
        disconnect(comboBoxTCValue3,	SIGNAL(editTextChanged(QString)),
                   this,                SLOT(OnTCFilterChange(QString)));
        disconnect(comboBoxTCValue4,	SIGNAL(editTextChanged(QString)),
                   this,                SLOT(OnTCFilterChange(QString)));
        disconnect(comboBoxTCValue5,	SIGNAL(editTextChanged(QString)),
                   this,                SLOT(OnTCFilterChange(QString)));
    }
}

///////////////////////////////////////////////////////////
// Get the filters from the GUI
///////////////////////////////////////////////////////////
void GexOneQueryWizardPage1::FillDatabaseFilter(GexDatabaseFilter & dbFilter,
                                                const QString& queryFieldName /*= QString()*/)
{
    // Clear any previous filter
    dbFilter.reset();

    // Set the field to query
    dbFilter.setQueryFilter(queryFieldName);

    // Filter 1
    dbFilter.addNarrowFilter(comboBoxFilter1->currentText(), FilterString1->currentText());

    // Filter 2
    dbFilter.addNarrowFilter(comboBoxFilter2->currentText(), FilterString2->currentText());

    // Filter 3
    dbFilter.addNarrowFilter(comboBoxFilter3->currentText(), FilterString3->currentText());

    // Filter 4
    dbFilter.addNarrowFilter(comboBoxFilter4->currentText(), FilterString4->currentText());

    // Filter 5
    dbFilter.addNarrowFilter(comboBoxFilter5->currentText(), FilterString5->currentText());

    // Filter 6
    dbFilter.addNarrowFilter(comboBoxFilter6->currentText(), FilterString6->currentText());

    // Filter 7
    dbFilter.addNarrowFilter(comboBoxFilter7->currentText(), FilterString7->currentText());

    // Filter 8
    dbFilter.addNarrowFilter(comboBoxFilter8->currentText(), FilterString8->currentText());

    // TC Filter 1
    dbFilter.addNarrowFilter(comboBoxTCFilter1->currentText(), comboBoxTCValue1->currentText());

    // TC Filter 2
    dbFilter.addNarrowFilter(comboBoxTCFilter2->currentText(), comboBoxTCValue2->currentText());

    // TC Filter 3
    dbFilter.addNarrowFilter(comboBoxTCFilter3->currentText(), comboBoxTCValue3->currentText());

    // TC Filter 4
    dbFilter.addNarrowFilter(comboBoxTCFilter4->currentText(), comboBoxTCValue4->currentText());

    // TC Filter 5
    dbFilter.addNarrowFilter(comboBoxTCFilter5->currentText(), comboBoxTCValue5->currentText());

    dbFilter.strDataTypeQuery   = comboBoxDatabaseType->currentText();	// Query over: Wafer sort, Final test, E-test (only applies to remote databases)

    // Get Database name we have to look into...get string '[Local] <Database name>'
    QString strDatabaseName     = comboBoxDatabaseName->currentText();

    // Skip the [Local]/[Server] info, and extract database name.
    if(strDatabaseName.startsWith("[Local]") || strDatabaseName.startsWith("[Server]"))
        strDatabaseName = strDatabaseName.section("]",1).trimmed();

    bool ok = false;
    dbFilter.iTimeNFactor = lineEditLastNXFactor->text().toInt(&ok);
    if (!ok)
    {
        QString m = QString("Last N factor in GUI ('%1') cannot be converted to integer.")
                            .arg(lineEditLastNXFactor->text());

        GS::Gex::Message::warning("Error", m);
        dbFilter.iTimeNFactor = 0;
    }

    int lastNXStepIdx = comboBoxLastNXStep->currentIndex();

    if (lastNXStepIdx >= 0)
        dbFilter.m_eTimeStep = (GexDatabaseFilter::eTimeStep) (lastNXStepIdx);
    else
        dbFilter.m_eTimeStep = GexDatabaseFilter::DAYS;

    dbFilter.strDatabaseLogicalName = strDatabaseName;
    dbFilter.iTimePeriod            = comboBoxTimePeriod->currentIndex();
    // to have corresponding value with combo-box values
    if(m_iDialogMode == eHouseKeeping)
        dbFilter.iTimePeriod += GEX_QUERY_HOUSEKPERIOD_TODAY;

    dbFilter.calendarFrom           = FromDate->date();			// Filter: From date
    dbFilter.calendarTo             = ToDate->date();			// Filter: To date
    dbFilter.calendarFrom_Time      = FromTimeEdit->time();     // Filter: From time
    dbFilter.calendarTo_Time        = ToTimeEdit->time();		// Filter: To time

    dbFilter.bOfflineQuery          = checkBoxOfflineQuery->isChecked();
}

///////////////////////////////////////////////////////////
// Parses a filter entry and set the relevant query fields.
///////////////////////////////////////////////////////////
void	GexOneQueryWizardPage1::setQueryFilter(GexDatabaseEntry	*pDatabaseEntry,
                                               GexDatabaseQuery &cQuery,
                                               QString &strFilterName,
                                               QString &strFilterValue)
{
    // Check if local or remote DB to be used
    if(pDatabaseEntry->IsExternal() && !checkBoxOfflineQuery->isChecked())
    {
        // Remote SQL DB
        QString strSqlFilter = strFilterName;
        strSqlFilter += "=";
        strSqlFilter += strFilterValue;
        cQuery.strlSqlFilters.append(strSqlFilter);
        return;
    }

    // Local DB
    int	iFilterIndex=0;
    while(gexLabelFilterChoices[iFilterIndex] != 0)
    {
        if( strFilterName == gexLabelFilterChoices[iFilterIndex])
            break;	// found matching string
        iFilterIndex++;
    };	// loop until we have found the string entry.

    // If filter not matching any entry, ignore call.
    if(gexLabelFilterChoices[iFilterIndex] == 0 || (iFilterIndex == 0))
        return;

    // Update relevant Query field
    quint64	uFlag = 0;
    switch(iFilterIndex)
    {
        case GEX_QUERY_FILTER_NONE:
            uFlag = 0;
            break;
        case	GEX_QUERY_FILTER_BURNIN:
            cQuery.iBurninTime = strFilterValue.toLong();
            uFlag = GEX_QUERY_FLAG_BURNIN;
            break;
        case	GEX_QUERY_FILTER_ORIGIN:
            cQuery.strDataOrigin = strFilterValue;
            uFlag = GEX_QUERY_FLAG_ORIGIN;
            break;
        case	GEX_QUERY_FILTER_FACILITY:
            cQuery.strFacilityID = strFilterValue;
            uFlag = GEX_QUERY_FLAG_FACILITY;
            break;
        case	GEX_QUERY_FILTER_FAMILY:
            cQuery.strFamilyID = strFilterValue;
            uFlag = GEX_QUERY_FLAG_FAMILY;
            break;
        case	GEX_QUERY_FILTER_FLOOR:
            cQuery.strFloorID = strFilterValue;
            uFlag = GEX_QUERY_FLAG_FLOOR;
            break;
        case	GEX_QUERY_FILTER_FREQUENCYSTEP:
            cQuery.strFrequencyStep = strFilterValue;
            uFlag = GEX_QUERY_FLAG_FREQUENCYSTEP;
            break;
        case	GEX_QUERY_FILTER_LOADBOARDNAME:
            cQuery.strLoadBoardName = strFilterValue;
            uFlag = GEX_QUERY_FLAG_LOADBOARDNAME;
            break;
        case	GEX_QUERY_FILTER_LOADBOARDTYPE:
            cQuery.strLoadBoardType = strFilterValue;
            uFlag = GEX_QUERY_FLAG_LOADBOARDTYPE;
            break;
        case	GEX_QUERY_FILTER_DIBNAME:
            cQuery.strDibName = strFilterValue;
            uFlag = GEX_QUERY_FLAG_DIBNAME;
            break;
        case	GEX_QUERY_FILTER_DIBTYPE:
            cQuery.strDibType = strFilterValue;
            uFlag = GEX_QUERY_FLAG_DIBTYPE;
            break;
        case	GEX_QUERY_FILTER_LOT:
            cQuery.strLotID = strFilterValue;
            uFlag = GEX_QUERY_FLAG_LOT;
            break;
        case	GEX_QUERY_FILTER_OPERATOR:
            cQuery.strOperator = strFilterValue;
            uFlag = GEX_QUERY_FLAG_OPERATOR;
            break;
        case	GEX_QUERY_FILTER_PACKAGE:
            cQuery.strPackageType = strFilterValue;
            uFlag = GEX_QUERY_FLAG_PACKAGE;
            break;
        case	GEX_QUERY_FILTER_PROBERNAME:
            cQuery.strProberName = strFilterValue;
            uFlag = GEX_QUERY_FLAG_PROBERNAME;
            break;
        case	GEX_QUERY_FILTER_PROBERTYPE:
            cQuery.strProberType = strFilterValue;
            uFlag = GEX_QUERY_FLAG_PROBERTYPE;
            break;
        case	GEX_QUERY_FILTER_PROCESS:
            cQuery.strProcessID = strFilterValue;
            uFlag = GEX_QUERY_FLAG_PROCESS;
            break;
        case	GEX_QUERY_FILTER_PRODUCT:
            cQuery.strProductID = strFilterValue;
            uFlag = GEX_QUERY_FLAG_PRODUCT;
            break;
        case	GEX_QUERY_FILTER_PROGRAMNAME:
            cQuery.strJobName = strFilterValue;
            uFlag = GEX_QUERY_FLAG_PROGRAMNAME;
            break;
        case	GEX_QUERY_FILTER_PROGRAMREVISION:
            cQuery.strJobRev = strFilterValue;
            uFlag = GEX_QUERY_FLAG_PROGRAMREVISION;
            break;
        case	GEX_QUERY_FILTER_RETESTNBR:
            cQuery.strRetestNbr = strFilterValue;
            uFlag = GEX_QUERY_FLAG_RETESTNBR;
            break;
        case	GEX_QUERY_FILTER_SUBLOT:
            cQuery.strSubLotID = strFilterValue;
            uFlag = GEX_QUERY_FLAG_SUBLOT;
            break;
        case	GEX_QUERY_FILTER_TEMPERATURE:
            cQuery.strTemperature = strFilterValue;
            uFlag = GEX_QUERY_FLAG_TEMPERATURE;
            break;
        case	GEX_QUERY_FILTER_TESTERNAME:
            cQuery.strNodeName = strFilterValue;
            uFlag = GEX_QUERY_FLAG_TESTERNAME;
            break;
        case	GEX_QUERY_FILTER_TESTERTYPE:
            cQuery.strTesterType = strFilterValue;
            uFlag = GEX_QUERY_FLAG_TESTERTYPE;
            break;
        case	GEX_QUERY_FILTER_TESTCODE:
            cQuery.strTestingCode = strFilterValue;
            uFlag = GEX_QUERY_FLAG_TESTCODE;
            break;
        case GEX_QUERY_FILTER_SITENBR:
            cQuery.iSite = strFilterValue.toLong();
            uFlag = GEX_QUERY_FLAG_SITENBR;
            break;
        case GEX_QUERY_FILTER_WAFERID:
            cQuery.strWaferID = strFilterValue;
            uFlag = GEX_QUERY_FLAG_WAFERID;
            break;
        case GEX_QUERY_FILTER_USER1:
            cQuery.strUser1 = strFilterValue;
            uFlag = GEX_QUERY_FLAG_USER1;
            break;
        case GEX_QUERY_FILTER_USER2:
            cQuery.strUser2 = strFilterValue;
            uFlag = GEX_QUERY_FLAG_USER2;
            break;
        case GEX_QUERY_FILTER_USER3:
            cQuery.strUser3 = strFilterValue;
            uFlag = GEX_QUERY_FLAG_USER3;
            break;
        case GEX_QUERY_FILTER_USER4:
            cQuery.strUser4 = strFilterValue;
            uFlag = GEX_QUERY_FLAG_USER4;
            break;
        case GEX_QUERY_FILTER_USER5:
            cQuery.strUser5 = strFilterValue;
            uFlag = GEX_QUERY_FLAG_USER5;
            break;
        case GEX_QUERY_FILTER_TOTALPARTS:
            cQuery.strProcessData = strFilterValue;
            uFlag = GEX_QUERY_FLAG_TOTALPARTS;
            break;
    }

    // Update the Filter flags (filters activated). eg:GEX_QUERY_FLAG_FLOOR | GEX_QUERY_FLAG_PROBERNAME...
    cQuery.uFilterFlag |= uFlag;
}

///////////////////////////////////////////////////////////
// Return the Query GUI fields into a Query structure.
///////////////////////////////////////////////////////////
void    GexOneQueryWizardPage1::GetQueryFields(GexDatabaseQuery &cQuery)
{
    GSLOG(SYSLOG_SEV_DEBUG, " ");

    cQuery.clear();

    // Get database pointer
    GexDatabaseEntry	*pDatabaseEntry = GetSelectedDatabase();

    // Do nothing if database not found
    if(!pDatabaseEntry)
        return;

    // bExternal: 'true' if remote database
    cQuery.bExternal = pDatabaseEntry->IsExternal();

    // bLocalDatabase: 'true' if Database is Local, false if on server
    QString strDatabaseName = comboBoxDatabaseName->currentText();
    cQuery.bLocalDatabase = pDatabaseEntry->IsStoredInFolderLocal();

    if(strDatabaseName.startsWith("[Local]") || strDatabaseName.startsWith("[Server]"))
        strDatabaseName = strDatabaseName.section("]",1).trimmed();

    eExtractionMode lExtractionMode = static_cast<GexOneQueryWizardPage1::eExtractionMode>(
                    comboBoxExtractionMode->itemData(
                    comboBoxExtractionMode->currentIndex()).toInt());
    switch (lExtractionMode)
    {
        case GexOneQueryWizardPage1::eRawData:
            cQuery.bConsolidatedExtraction=false;
            break;
        case GexOneQueryWizardPage1::eConsolidatedData:
            cQuery.bConsolidatedExtraction=true;
            break;
        case GexOneQueryWizardPage1::e1outOfNsamples:
            cQuery.m_gexQueries.push_back(
                        QList<QString>()<<"db_extraction_mode"<< "1 out of N" << mExtractionModeLineEdit->text() ); // case 6050
            break;
        default :
            GSLOG(SYSLOG_SEV_ERROR, QString("Unknown Extraction mode : %1").arg( comboBoxExtractionMode->currentText()).toLatin1().constData() );
    }

    // Skip the [Local]/[Server] info, and extract database name.
    cQuery.strDatabaseLogicalName = strDatabaseName;				// Database logical name
    cQuery.strDataTypeQuery = comboBoxDatabaseType->currentText();	// Query over: Wafer sort, Final test, E-test (only applies to remote databases)
    cQuery.bOfflineQuery = checkBoxOfflineQuery->isChecked();		// 'true' if query to perform over local cache  (only applies to remote databases)

    cQuery.iTimePeriod = comboBoxTimePeriod->currentIndex();			// GEX_QUERY_TIMEPERIOD_TODAY, etc...
    bool ok=false;
    cQuery.iTimeNFactor = lineEditLastNXFactor->text().toInt(&ok);
    cQuery.m_eTimeStep = (GexDatabaseQuery::eTimeStep) comboBoxLastNXStep->currentIndex();

    cQuery.calendarFrom = FromDate->date();							// Filter: From date
    cQuery.calendarTo = ToDate->date();								// Filter: To date
    cQuery.calendarFrom_Time = FromTimeEdit->time();				// Filter: From time
    cQuery.calendarTo_Time = ToTimeEdit->time();					// Filter: To time

    ///////////////////////////////////////////////////////////////////
    // Set filters                                                   //
    ///////////////////////////////////////////////////////////////////
    // Get filters selected: Filter1
    QString strFilterName = comboBoxFilter1->currentText();
    QString strFilterValue = FilterString1->currentText();
    setQueryFilter(pDatabaseEntry,cQuery,strFilterName,strFilterValue);

    // Get filters selected: Filter2
    strFilterName = comboBoxFilter2->currentText();
    strFilterValue = FilterString2->currentText();
    setQueryFilter(pDatabaseEntry,cQuery,strFilterName,strFilterValue);

    // Get filters selected: Filter3
    strFilterName = comboBoxFilter3->currentText();
    strFilterValue = FilterString3->currentText();
    setQueryFilter(pDatabaseEntry,cQuery,strFilterName,strFilterValue);

    // Get filters selected: Filter4
    strFilterName = comboBoxFilter4->currentText();
    strFilterValue = FilterString4->currentText();
    setQueryFilter(pDatabaseEntry,cQuery,strFilterName,strFilterValue);

    // Get filters selected: Filter5
    strFilterName = comboBoxFilter5->currentText();
    strFilterValue = FilterString5->currentText();
    setQueryFilter(pDatabaseEntry,cQuery,strFilterName,strFilterValue);

    // Get filters selected: Filter6
    strFilterName = comboBoxFilter6->currentText();
    strFilterValue = FilterString6->currentText();
    setQueryFilter(pDatabaseEntry,cQuery,strFilterName,strFilterValue);

    // Get filters selected: Filter7
    strFilterName = comboBoxFilter7->currentText();
    strFilterValue = FilterString7->currentText();
    setQueryFilter(pDatabaseEntry,cQuery,strFilterName,strFilterValue);

    // Get filters selected: Filter8
    strFilterName = comboBoxFilter8->currentText();
    strFilterValue = FilterString8->currentText();
    setQueryFilter(pDatabaseEntry,cQuery,strFilterName,strFilterValue);

    // Get filters selected: TCFilter1
    strFilterName   = comboBoxTCFilter1->currentText();
    strFilterValue  = comboBoxTCValue1->currentText();
    setQueryFilter(pDatabaseEntry,cQuery,strFilterName,strFilterValue);

    // Get filters selected: TCFilter2
    strFilterName   = comboBoxTCFilter2->currentText();
    strFilterValue  = comboBoxTCValue2->currentText();
    setQueryFilter(pDatabaseEntry,cQuery,strFilterName,strFilterValue);

    // Get filters selected: TCFilter3
    strFilterName   = comboBoxTCFilter3->currentText();
    strFilterValue  = comboBoxTCValue3->currentText();
    setQueryFilter(pDatabaseEntry,cQuery,strFilterName,strFilterValue);

    // Get filters selected: TCFilter4
    strFilterName   = comboBoxTCFilter4->currentText();
    strFilterValue  = comboBoxTCValue4->currentText();
    setQueryFilter(pDatabaseEntry,cQuery,strFilterName,strFilterValue);

    // Get filters selected: TCFilter5
    strFilterName   = comboBoxTCFilter5->currentText();
    strFilterValue  = comboBoxTCValue5->currentText();
    setQueryFilter(pDatabaseEntry,cQuery,strFilterName,strFilterValue);
}

///////////////////////////////////////////////////////////
// Change Form attributes so to display as a pop-up dialog...
///////////////////////////////////////////////////////////
void    GexOneQueryWizardPage1::PopupSkin(int iDialogMode, bool bPopup)
{
    // Set flag
    bPopupMode = bPopup;
    m_iDialogMode = iDialogMode;

    if(bPopupMode == true)
    {
        // Shown as a pop-up dialog box
        if(iDialogMode == eHouseKeeping)
            resize(580,370);
        else
            resize(670,370);

        // Show/hide relevant fields & buttons
        frameNextAction->hide();
        PushButtonOk->show();
        PushButtonCancel->show();
    }
    else
    {
        // Shown as a page in the Examinator client area.
        setWindowFlags(Qt::FramelessWindowHint);
        move(0,0);

        // minimum X size allowed:
        setMinimumWidth(720);
    }

    switch(iDialogMode)
    {

        case eCreate:
            // Create new Query entry.
            setWindowTitle("Create a new Dataset (Query)");
            PushButtonOk->setText("Insert");
            // Tooltip
            PushButtonOk->setToolTip("Save this Dataset query" );
            break;

        case eEdit:
            // Edit Query entry (Properties)
            setWindowTitle("Edit Dataset (Query)");
            PushButtonOk->setText("OK");
            // Tooltip
            PushButtonOk->setToolTip("Save this Dataset query");
            // Timer period combo-box
            comboBoxTimePeriod->clear();
            FillComboBox(comboBoxTimePeriod, gexLabelTimePeriodChoices);
            comboBoxTimePeriod->setMaxVisibleItems(comboBoxTimePeriod->count());
            break;

        case eHouseKeeping:
            // Database housekeeping
            setWindowTitle("Database HouseKeeping");
            TextLabelHeaderTitle->setText("Database HouseKeeping: Purge data records");
            PushButtonOk->setText("Clean-up...");
            // Tooltip
            PushButtonOk->setToolTip("Purge this Database");

            // Timer period combo-box
            comboBoxTimePeriod->clear();
            FillComboBox(comboBoxTimePeriod, gexLabelHousekeepingPeriodChoices);

            // Hide some fields
            TabWidgetFilters->removeTab(TabWidgetFilters->indexOf(tabAdvanced));
            TextLabelTitle->hide();
            lineEditReportTitle->hide();
            break;

        case ePATRecipe:
            // Create new Query.
            TextLabelHeaderTitle->setText("Define your historical dataset");

            // Disabled datbase type combo box
            comboBoxDatabaseType->setEnabled(false);
            checkBoxOfflineQuery->hide();

            // Hide Query name fields
            TextLabelTitle->hide();
            lineEditReportTitle->hide();

            // Remove Advanced Tab
            TabWidgetFilters->removeTab(TabWidgetFilters->indexOf(tabAdvanced));
            TabWidgetFilters->removeTab(TabWidgetFilters->indexOf(tabTestConditions));
            TabWidgetFilters->removeTab(TabWidgetFilters->indexOf(tabRdbOptions));
            break;
    }

    // Update Filter Combo boxes status
    OnComboProcessChange();

    ShowPage();
}

///////////////////////////////////////////////////////////
// Refreshes fields to have GUI updated
///////////////////////////////////////////////////////////
void	GexOneQueryWizardPage1::UpdateGui(void)
{
    // Update GUI accordingly
    OnTimePeriod();
    OnComboProcessChange();
    OnDatabaseChanged(true);
}

QStringList GexOneQueryWizardPage1::GetTestConditionsLevel() const
{
    return widgetConditionLevel->GetSelectedFields();
}

///////////////////////////////////////////////////////////
// Swap dates if needed, update fields.
///////////////////////////////////////////////////////////
void	GexOneQueryWizardPage1::UpdateFromToFields(bool bCheckForSwap)
{
    QDate		Date;
    QTime		Time;
    QDateTime	clDateTime_From, clDateTime_To;

    // Check if need to swap dates!
    if((bCheckForSwap == true) && (From > To))
    {
        Date = To;
        To= From;
        From= Date;
    }

    // Check if need to swap times!
    if((bCheckForSwap == true) && (FromTime > ToTime))
    {
        Time = ToTime;
        ToTime = FromTime;
        FromTime = Time;
    }

    // Update 'From' fields + comment
    FromDate->setDate(From);
    FromTimeEdit->setTime(FromTime);
    clDateTime_From.setDate(From);
    clDateTime_From.setTime(FromTime);
    TextLabelFromDate->setText(clDateTime_From.toString(Qt::TextDate));

    // Update 'To' fields + comment
    ToDate->setDate(To);
    ToTimeEdit->setTime(ToTime);
    clDateTime_To.setDate(To);
    clDateTime_To.setTime(ToTime);
    TextLabelToDate->setText(clDateTime_To.toString(Qt::TextDate));
}

///////////////////////////////////////////////////////////
// Update GUI for currently selected database
///////////////////////////////////////////////////////////
void GexOneQueryWizardPage1::OnDatabaseChanged(bool bKeepValues/*=false*/)
{
    // Get database pointer
    GexDatabaseEntry	*pDatabaseEntry = GetSelectedDatabase();
    if (!pDatabaseEntry)
        return;

    // Check if local or remote DB to be used, update GUI controls accordingly
    m_bSqlDatabase = false;
    if(pDatabaseEntry->IsExternal())
    {
        if(checkBoxOfflineQuery->isChecked())
        {
            // Use local cache of remote DB: hide some controls
            TabWidgetFilters->setTabEnabled(TabWidgetFilters->indexOf(tabRdbOptions), false);
            comboBoxDatabaseType->hide();

            TabWidgetFilters->setTabEnabled(TabWidgetFilters->indexOf(tabTestConditions), false);
        }
        else
        {
            m_bSqlDatabase = true;

            // Hide or show Test Conditions tab depending on the DB type
            if (pDatabaseEntry->IsCharacTdr())
            {
                // If tab is hidden, then show it
                if (TabWidgetFilters->indexOf(tabTestConditions) == -1)
                    TabWidgetFilters->insertTab(2, tabTestConditions, "Test Conditions");
                else
                    // Enabled Test Conditions tab
                    TabWidgetFilters->setTabEnabled(TabWidgetFilters->indexOf(tabTestConditions), true);
            }
            else
            {
                TabWidgetFilters->removeTab(TabWidgetFilters->indexOf(tabTestConditions));

                // Remove characterization restriction imposed with the
                // Characterization flow
                lineEditMinimumPartPerFile->setEnabled(true);
            }

            // Enabled RDB Options tab
            TabWidgetFilters->setTabEnabled(TabWidgetFilters->indexOf(tabRdbOptions), true);

            // Check if plug-in has an option widget
            QWidget *pNewWidget = pDatabaseEntry->m_pExternalDatabase->GetRdbOptionsGui();
            if(pNewWidget)
            {
                labelPlaceHolder->hide();
                if(m_pRdbOptionsWidget == NULL)
                {
                    m_pRdbOptionsWidget = pNewWidget;
                    tabRdbOptions->layout()->addWidget(m_pRdbOptionsWidget);
                    m_pRdbOptionsWidget->show();
                }
                else if(m_pRdbOptionsWidget != pNewWidget)
                {
                    // Remove old RDB Options widget
                    // and set its parent to NULL as the RDB plugin is responsible to delete it
                    m_pRdbOptionsWidget->hide();
                    tabRdbOptions->layout()->removeWidget(m_pRdbOptionsWidget);
                    m_pRdbOptionsWidget->setParent(NULL);
                    m_pRdbOptionsWidget = pNewWidget;
                    tabRdbOptions->layout()->addWidget(m_pRdbOptionsWidget);
                    m_pRdbOptionsWidget->show();
                }
            }
            else
            {
                ResetRdbPluginHandles();
            }

            // Save currently selected data type
            QString strDataType = comboBoxDatabaseType->currentText();

            // Fill the Data type combo with appropriate values
            comboBoxDatabaseType->clear();

            // Set plugin-name
            labelPluginName->setText(pDatabaseEntry->m_pExternalDatabase->GetPluginID()->pluginName());

            // Show/Hide GUI elements depending on what's supported by the database
            // Parameter selection edit
            if(pDatabaseEntry->m_pExternalDatabase->IsParameterSelectionSupported())
            {
                lineEditParameterList->setEnabled(true);
                buttonPickParameterList->setEnabled(true);
            }
            else
            {
                lineEditParameterList->setEnabled(false);
                buttonPickParameterList->setEnabled(false);
            }
            // For selecting data type to query (wafer sort, or final test,...)
            if(pDatabaseEntry->m_pExternalDatabase->IsTestingStagesSupported())
            {
                QStringList strlSupportedTestingStages;
                pDatabaseEntry->m_pExternalDatabase->GetSupportedTestingStages(strlSupportedTestingStages);
                comboBoxDatabaseType->insertItems(0,strlSupportedTestingStages);

                // Force saved selection (if available) and show combo
                SetCurrentComboItem(comboBoxDatabaseType, strDataType);
                comboBoxDatabaseType->show();
            }
            else
                comboBoxDatabaseType->hide();
        }

        // Check box to run query over local cache
        checkBoxOfflineQuery->show();

    }
    else
    {
        // Not a remoteDB, hide controls!
        GSLOG(SYSLOG_SEV_DEBUG, " LocalDB found" );
        checkBoxOfflineQuery->setChecked(false);
        TabWidgetFilters->setTabEnabled(TabWidgetFilters->indexOf(tabRdbOptions), false);
        comboBoxDatabaseType->hide();
        checkBoxOfflineQuery->hide();

        // Always hide test conditions tab for non-rdb
        TabWidgetFilters->removeTab(TabWidgetFilters->indexOf(tabTestConditions));
    }

    // Update Extraction mode controls
    UpdateExtractionModeUi();

    // Update filters
    UpdateFilters(pDatabaseEntry, true, bKeepValues);

    if (pGexMainWindow->iWizardPage == GEX_ENTERPRISE_SQL_P1)
    {
        TabWidgetFilters->tabBar()->hide();
        labelNextStep->hide();
        comboBoxNextAction->hide();
    }
    else if (m_iDialogMode == ePATRecipe)
    {
        labelNextStep->hide();
        comboBoxNextAction->hide();
        pushButtonNext->hide();
        checkBoxOfflineQuery->hide();
    }
    else
    {
        TabWidgetFilters->tabBar()->show();
        labelNextStep->show();
        comboBoxNextAction->show();
    }

    if(mPurgeDb)
    {
        checkBoxOfflineQuery->hide();

        // Hide Level and hierarchy tab
        if (tabWidgetTC->indexOf(tabTCLevelHierarchy) != -1)
            tabWidgetTC->removeTab(tabWidgetTC->indexOf(tabTCLevelHierarchy));
    }
    else
    {
        // Show Level and hierarchy tab
        if (tabWidgetTC->indexOf(tabTCLevelHierarchy) == -1)
            tabWidgetTC->insertTab(0, tabTCLevelHierarchy, "Level and Hierarchy");
    }
}

///////////////////////////////////////////////////////////
// Update filters
///////////////////////////////////////////////////////////
void GexOneQueryWizardPage1::UpdateFilters(GexDatabaseEntry *pDatabaseEntry, bool bKeepSelectedFilters/*=true*/, bool bKeepValues/*=false*/)
{
    QString	strFilter1, strFilter2, strFilter3, strFilter4, strFilter5, strFilter6, strFilter7, strFilter8;
    QString strValue1, strValue2, strValue3, strValue4, strValue5, strValue6, strValue7, strValue8;
    QString	strParameterList;
    QStringList splitFields;

    // Make sure filters are up-to-date in the Enterprise reports settings page
    pGexMainWindow->pWizardSettings->UpdateFilters(bKeepSelectedFilters, bKeepValues);

    // Save filters and values
    strFilter1 = comboBoxFilter1->currentText();
    strFilter2 = comboBoxFilter2->currentText();
    strFilter3 = comboBoxFilter3->currentText();
    strFilter4 = comboBoxFilter4->currentText();
    strFilter5 = comboBoxFilter5->currentText();
    strFilter6 = comboBoxFilter6->currentText();
    strFilter7 = comboBoxFilter7->currentText();
    strFilter8 = comboBoxFilter8->currentText();

    strValue1 = FilterString1->currentText();
    strValue2 = FilterString2->currentText();
    strValue3 = FilterString3->currentText();
    strValue4 = FilterString4->currentText();
    strValue5 = FilterString5->currentText();
    strValue6 = FilterString6->currentText();
    strValue7 = FilterString7->currentText();
    strValue8 = FilterString8->currentText();

    strParameterList = lineEditParameterList->text();

    splitFields = lineEditSplitField->text().split(",", QString::SkipEmptyParts);

    if(!m_bRdbOptionsInitialized)
    {
        // Set RDB parameter list from option
        QString QDP=ReportOptions.GetOption("databases", "rdb_default_parameters").toString();
        if(QDP=="all")	//(ReportOptions.iRdbQueryDefaultParameters == GEX_OPTION_RDB_EXTRACT_PARAMETERS_ALL)
            strParameterList = "*";
        else if(QDP=="none") //(ReportOptions.iRdbQueryDefaultParameters == GEX_OPTION_RDB_EXTRACT_PARAMETERS_NONE)
            strParameterList = "";
        lineEditParameterList->setText(strParameterList);
        m_bRdbOptionsInitialized = true;
    }

    // Clear current filters
    comboBoxFilter1->clear();
    comboBoxFilter2->clear();
    comboBoxFilter3->clear();
    comboBoxFilter4->clear();
    comboBoxFilter5->clear();
    comboBoxFilter6->clear();
    comboBoxFilter7->clear();
    comboBoxFilter8->clear();
    lineEditSplitField->clear();

    // Clear current values
    FilterString1->setCurrentIndex(FilterString1->findText("*"));
    FilterString2->setCurrentIndex(FilterString2->findText("*"));
    FilterString3->setCurrentIndex(FilterString3->findText("*"));
    FilterString4->setCurrentIndex(FilterString4->findText("*"));
    FilterString5->setCurrentIndex(FilterString5->findText("*"));
    FilterString6->setCurrentIndex(FilterString6->findText("*"));
    FilterString7->setCurrentIndex(FilterString7->findText("*"));
    FilterString8->setCurrentIndex(FilterString8->findText("*"));

    // Clear current fields available for the split
    mSplitFieldsAvailable.clear();

    // Initialize with the default field list
    int idx = 0;
    while(gexLabelFilterChoices[++idx] != 0)
        mSplitFieldsAvailable.append(gexLabelFilterChoices[idx]);

    // Check if local or remote DB to be used
    if(pDatabaseEntry->IsExternal())
    {
        if(checkBoxOfflineQuery->isChecked())
        {
            // Load list of filters parameters in Filter#1-Filter#8 combos
            FillComboBox(comboBoxFilter1,		gexLabelFilterChoices);
            FillComboBox(comboBoxFilter2,		gexLabelFilterChoices);
            FillComboBox(comboBoxFilter3,		gexLabelFilterChoices);
            FillComboBox(comboBoxFilter4,		gexLabelFilterChoices);
            FillComboBox(comboBoxFilter5,		gexLabelFilterChoices);
            FillComboBox(comboBoxFilter6,		gexLabelFilterChoices);
            FillComboBox(comboBoxFilter7,		gexLabelFilterChoices);
            FillComboBox(comboBoxFilter8,		gexLabelFilterChoices);

            // Default filter Parameters selected...
            comboBoxFilter1->setCurrentIndex(GEX_QUERY_FILTER_PRODUCT);
            comboBoxFilter2->setCurrentIndex(GEX_QUERY_FILTER_LOT);
            comboBoxFilter3->setCurrentIndex(GEX_QUERY_FILTER_NONE);
            comboBoxFilter4->setCurrentIndex(GEX_QUERY_FILTER_NONE);
            comboBoxFilter5->setCurrentIndex(GEX_QUERY_FILTER_NONE);
            comboBoxFilter6->setCurrentIndex(GEX_QUERY_FILTER_NONE);
            comboBoxFilter7->setCurrentIndex(GEX_QUERY_FILTER_NONE);
            comboBoxFilter8->setCurrentIndex(GEX_QUERY_FILTER_NONE);
        }
        else
        {
            // Load list of filters parameters in Filter#1-Filter#8 combos
            QStringList strlFilters;
            pDatabaseEntry->m_pExternalDatabase->GetLabelFilterChoices(comboBoxDatabaseType->currentText(), strlFilters);
            if(strlFilters.isEmpty())
            {
                // Load list of filters parameters in Filter#1-Filter#8 combos
                FillComboBox(comboBoxFilter1,		gexLabelFilterChoices);
                FillComboBox(comboBoxFilter2,		gexLabelFilterChoices);
                FillComboBox(comboBoxFilter3,		gexLabelFilterChoices);
                FillComboBox(comboBoxFilter4,		gexLabelFilterChoices);
                FillComboBox(comboBoxFilter5,		gexLabelFilterChoices);
                FillComboBox(comboBoxFilter6,		gexLabelFilterChoices);
                FillComboBox(comboBoxFilter7,		gexLabelFilterChoices);
                FillComboBox(comboBoxFilter8,		gexLabelFilterChoices);

                // Default filter Parameters selected...
                comboBoxFilter1->setCurrentIndex(GEX_QUERY_FILTER_PRODUCT);
                comboBoxFilter2->setCurrentIndex(GEX_QUERY_FILTER_LOT);
                comboBoxFilter3->setCurrentIndex(GEX_QUERY_FILTER_NONE);
                comboBoxFilter4->setCurrentIndex(GEX_QUERY_FILTER_NONE);
                comboBoxFilter5->setCurrentIndex(GEX_QUERY_FILTER_NONE);
                comboBoxFilter6->setCurrentIndex(GEX_QUERY_FILTER_NONE);
                comboBoxFilter7->setCurrentIndex(GEX_QUERY_FILTER_NONE);
                comboBoxFilter8->setCurrentIndex(GEX_QUERY_FILTER_NONE);
            }
            else
            {
                // Load list of filters parameters in Filter#1-Filter#5 combos
                comboBoxFilter1->insertItem(0,gexLabelFilterChoices[GEX_QUERY_FILTER_NONE]);
                comboBoxFilter1->insertItems(1,strlFilters);
                comboBoxFilter2->insertItem(0,gexLabelFilterChoices[GEX_QUERY_FILTER_NONE]);
                comboBoxFilter2->insertItems(1,strlFilters);
                comboBoxFilter3->insertItem(0,gexLabelFilterChoices[GEX_QUERY_FILTER_NONE]);
                comboBoxFilter3->insertItems(1,strlFilters);
                comboBoxFilter4->insertItem(0,gexLabelFilterChoices[GEX_QUERY_FILTER_NONE]);
                comboBoxFilter4->insertItems(1,strlFilters);
                comboBoxFilter5->insertItem(0,gexLabelFilterChoices[GEX_QUERY_FILTER_NONE]);
                comboBoxFilter5->insertItems(1,strlFilters);
                comboBoxFilter6->insertItem(0,gexLabelFilterChoices[GEX_QUERY_FILTER_NONE]);
                comboBoxFilter6->insertItems(1,strlFilters);
                comboBoxFilter7->insertItem(0,gexLabelFilterChoices[GEX_QUERY_FILTER_NONE]);
                comboBoxFilter7->insertItems(1,strlFilters);
                comboBoxFilter8->insertItem(0,gexLabelFilterChoices[GEX_QUERY_FILTER_NONE]);
                comboBoxFilter8->insertItems(1,strlFilters);

                mSplitFieldsAvailable = strlFilters;

                // Default filter Parameters selected...
                comboBoxFilter1->setCurrentIndex(GEX_QUERY_FILTER_NONE);
                comboBoxFilter2->setCurrentIndex(GEX_QUERY_FILTER_NONE);
                comboBoxFilter3->setCurrentIndex(GEX_QUERY_FILTER_NONE);
                comboBoxFilter4->setCurrentIndex(GEX_QUERY_FILTER_NONE);
                comboBoxFilter5->setCurrentIndex(GEX_QUERY_FILTER_NONE);
                comboBoxFilter6->setCurrentIndex(GEX_QUERY_FILTER_NONE);
                comboBoxFilter7->setCurrentIndex(GEX_QUERY_FILTER_NONE);
                comboBoxFilter8->setCurrentIndex(GEX_QUERY_FILTER_NONE);
            }

            // Set parameters list from default?
            if(!bKeepValues && !strParameterList.isEmpty() && (strParameterList != "*"))
            {
                QString QDP=ReportOptions.GetOption("databases", "rdb_default_parameters").toString();
                if(QDP=="all")	//(ReportOptions.iRdbQueryDefaultParameters == GEX_OPTION_RDB_EXTRACT_PARAMETERS_ALL)
                    lineEditParameterList->setText("*") ;
                else if(QDP=="none") //(ReportOptions.iRdbQueryDefaultParameters == GEX_OPTION_RDB_EXTRACT_PARAMETERS_NONE)
                    lineEditParameterList->setText("") ;
            }
        }
    }
    else
    {
        // Load list of filters parameters in Filter#1-Filter#5 combos
        FillComboBox(comboBoxFilter1,		gexLabelFilterChoices);
        FillComboBox(comboBoxFilter2,		gexLabelFilterChoices);
        FillComboBox(comboBoxFilter3,		gexLabelFilterChoices);
        FillComboBox(comboBoxFilter4,		gexLabelFilterChoices);
        FillComboBox(comboBoxFilter5,		gexLabelFilterChoices);
        FillComboBox(comboBoxFilter6,		gexLabelFilterChoices);
        FillComboBox(comboBoxFilter7,		gexLabelFilterChoices);
        FillComboBox(comboBoxFilter8,		gexLabelFilterChoices);

        // Default filter Parameters selected...
        comboBoxFilter1->setCurrentIndex(GEX_QUERY_FILTER_PRODUCT);
        comboBoxFilter2->setCurrentIndex(GEX_QUERY_FILTER_LOT);
        comboBoxFilter3->setCurrentIndex(GEX_QUERY_FILTER_NONE);
        comboBoxFilter4->setCurrentIndex(GEX_QUERY_FILTER_NONE);
        comboBoxFilter5->setCurrentIndex(GEX_QUERY_FILTER_NONE);
        comboBoxFilter6->setCurrentIndex(GEX_QUERY_FILTER_NONE);
        comboBoxFilter7->setCurrentIndex(GEX_QUERY_FILTER_NONE);
        comboBoxFilter8->setCurrentIndex(GEX_QUERY_FILTER_NONE);
    }

    // Set saved values?
    if(bKeepSelectedFilters)
    {
        if(SetCurrentComboItem(comboBoxFilter1, strFilter1) && bKeepValues)
            SetCurrentComboItem(FilterString1, strValue1, true);

        if(SetCurrentComboItem(comboBoxFilter2, strFilter2) && bKeepValues)
            SetCurrentComboItem(FilterString2, strValue2, true);

        if(SetCurrentComboItem(comboBoxFilter3, strFilter3) && bKeepValues)
            SetCurrentComboItem(FilterString3, strValue3, true);

        if(SetCurrentComboItem(comboBoxFilter4, strFilter4) && bKeepValues)
            SetCurrentComboItem(FilterString4, strValue4, true);

        if(SetCurrentComboItem(comboBoxFilter5, strFilter5) && bKeepValues)
            SetCurrentComboItem(FilterString5, strValue5, true);

        if(SetCurrentComboItem(comboBoxFilter6, strFilter6) && bKeepValues)
            SetCurrentComboItem(FilterString6, strValue6, true);

        if(SetCurrentComboItem(comboBoxFilter7, strFilter7) && bKeepValues)
            SetCurrentComboItem(FilterString7, strValue7, true);

        if(SetCurrentComboItem(comboBoxFilter8, strFilter8) && bKeepValues)
            SetCurrentComboItem(FilterString8, strValue8, true);

        QStringList selectedSplitFields;
        foreach(const QString &field, splitFields)
        {
            if (mSplitFieldsAvailable.contains(field))
                selectedSplitFields.append(field);
        }

        lineEditSplitField->setText(selectedSplitFields.join(","));
    }

    // Update test conditions
    if(pDatabaseEntry->IsExternal() && (TabWidgetFilters->currentWidget() == tabTestConditions))
    {
        UpdateTestConditions(bKeepSelectedFilters, bKeepValues);
    }

#if 0
    // Keep value if specified by bKeepValues OR saved value is '*' OR saved value is ''
    if((strParameterList == "*") || (strParameterList.isEmpty()) || bKeepValues)
        lineEditParameterList->setText(strParameterList);
#endif
}

///////////////////////////////////////////////////////////
// Update Test Conditions
///////////////////////////////////////////////////////////
void GexOneQueryWizardPage1::UpdateTestConditions(bool lKeepSelectedFilters /* = true */,
                                                  bool lKeepValues /* = true */)
{
    QStringList lSelectedTC;
    QString     lFilter1, lFilter2, lFilter3, lFilter4, lFilter5;
    QString     lValue1, lValue2, lValue3, lValue4, lValue5;

    labelCharacMessages->clear();

    if (pGexMainWindow && (mSynchronizedTestCond == false))
    {
        mTestConditionsAvailable.clear();
        widgetConditionHierarchy->Clear();

        // Disconnect signals between tc filters and tc widgets
        ConnectTCFilterControls(false);

        lFilter1 = comboBoxTCFilter1->currentText();
        lFilter2 = comboBoxTCFilter2->currentText();
        lFilter3 = comboBoxTCFilter3->currentText();
        lFilter4 = comboBoxTCFilter4->currentText();
        lFilter5 = comboBoxTCFilter5->currentText();

        lValue1 = comboBoxTCValue1->currentText();
        lValue2 = comboBoxTCValue2->currentText();
        lValue3 = comboBoxTCValue3->currentText();
        lValue4 = comboBoxTCValue4->currentText();
        lValue5 = comboBoxTCValue5->currentText();

        comboBoxTCFilter1->clear();
        comboBoxTCFilter2->clear();
        comboBoxTCFilter3->clear();
        comboBoxTCFilter4->clear();
        comboBoxTCFilter5->clear();

        if (checkBoxOfflineQuery->isChecked() == false)
        {
            GexDatabaseFilter   lDbFilter;

            // Get the current filter from the GUI
            FillDatabaseFilter(lDbFilter);

            // Get the test conditions list
            // GCORE-994
            //mTestConditionsAvailable = GS::Gex::Engine::GetInstance().GetDatabaseEngine().QueryTestConditionsList(dbFilter);
            QString lRes = GS::Gex::Engine::GetInstance().GetDatabaseEngine().
                    QueryTestConditionsList(lDbFilter, mTestConditionsAvailable);
            if (lRes.startsWith("err"))
            {
                // no way to return the error
                // Let s emit message
                GS::Gex::Message::warning("Query Test Conditions failed", lRes);
                // Herve: any need to continue?
                return;
            }


            // If we need to keep the selected filters, compare the existing selection with
            // the new conditions list
            if (lKeepSelectedFilters)
            {
                lSelectedTC = widgetConditionLevel->GetSelectedFields();

                if (lSelectedTC.isEmpty())
                    lSelectedTC = mTestConditionsAvailable;
                else
                {
                    foreach(const QString &condition, lSelectedTC)
                    {
                        if (mTestConditionsAvailable.contains(condition) == false)
                        {
                            lSelectedTC = mTestConditionsAvailable;
                            break;
                        }
                    }
                }
            }
            else
                lSelectedTC = mTestConditionsAvailable;
        }

        // Load list of filters parameters in Filter#1-Filter#5 combos
        comboBoxTCFilter1->insertItem(0,gexLabelFilterChoices[GEX_QUERY_FILTER_NONE]);
        comboBoxTCFilter1->insertItems(1,mTestConditionsAvailable);

        comboBoxTCFilter2->insertItem(0,gexLabelFilterChoices[GEX_QUERY_FILTER_NONE]);
        comboBoxTCFilter2->insertItems(1,mTestConditionsAvailable);

        comboBoxTCFilter3->insertItem(0,gexLabelFilterChoices[GEX_QUERY_FILTER_NONE]);
        comboBoxTCFilter3->insertItems(1,mTestConditionsAvailable);

        comboBoxTCFilter4->insertItem(0,gexLabelFilterChoices[GEX_QUERY_FILTER_NONE]);
        comboBoxTCFilter4->insertItems(1,mTestConditionsAvailable);

        comboBoxTCFilter5->insertItem(0,gexLabelFilterChoices[GEX_QUERY_FILTER_NONE]);
        comboBoxTCFilter5->insertItems(1,mTestConditionsAvailable);

        comboBoxTCValue1->setCurrentIndex(comboBoxTCValue1->findText("*"));
        comboBoxTCValue2->setCurrentIndex(comboBoxTCValue2->findText("*"));
        comboBoxTCValue3->setCurrentIndex(comboBoxTCValue3->findText("*"));
        comboBoxTCValue4->setCurrentIndex(comboBoxTCValue4->findText("*"));
        comboBoxTCValue5->setCurrentIndex(comboBoxTCValue5->findText("*"));

        // Select old filters if requested
        if (lKeepSelectedFilters)
        {
            if(SetCurrentComboItem(comboBoxTCFilter1, lFilter1) && lKeepValues)
                SetCurrentComboItem(comboBoxTCValue1, lValue1, false);
            if(SetCurrentComboItem(comboBoxTCFilter2, lFilter2) && lKeepValues)
                SetCurrentComboItem(comboBoxTCValue2, lValue2, false);
            if(SetCurrentComboItem(comboBoxTCFilter3, lFilter3) && lKeepValues)
                SetCurrentComboItem(comboBoxTCValue3, lValue3, false);
            if(SetCurrentComboItem(comboBoxTCFilter4, lFilter4) && lKeepValues)
                SetCurrentComboItem(comboBoxTCValue4, lValue4, false);
            if(SetCurrentComboItem(comboBoxTCFilter5, lFilter5) && lKeepValues)
                SetCurrentComboItem(comboBoxTCValue5, lValue5, false);
        }

        // Reconnect signals between tc filters and tc widgets
        ConnectTCFilterControls(true);

        if (!mPurgeDb)
        {
            widgetConditionLevel->SetAvailableFields(mTestConditionsAvailable, lKeepSelectedFilters);
            SetSelectedTestConditions(lSelectedTC);
            widgetConditionHierarchy->setEnabled(false);
        }

        // Set synchronized flags to true
        SetSynchronizedTestConditions(true);
        mIsHierarchyUpToDate = false;
        UpdateConditionsRender();
        pushButtonBuildHierarchy->setText("Build Hierarchy");
    }
}


void GexOneQueryWizardPage1::BuildHierarchy()
{
    pushButtonBuildHierarchy->setEnabled(false);
    widgetConditionHierarchy->setEnabled(false);
    QStringList lSelectedTC = widgetConditionLevel->GetSelectedFields();
    mTestConditionsAvailable = lSelectedTC;
    widgetConditionHierarchy->SetConditionLevel(lSelectedTC);
    mNullTestConditions.clear();

    GexDatabaseQuery lQuery;
    GetQueryFields(lQuery);
    mTestConditionHierarchy = GS::Gex::Engine::GetInstance().GetDatabaseEngine().QuerySelectFilter(lQuery, lSelectedTC);


    QList<QMap<QString, QString> > lConditionGroups;

    for (int lIdx = 0; lIdx < mTestConditionHierarchy.count(); ++lIdx)
    {
        QStringList values = mTestConditionHierarchy.at(lIdx).split("|", QString::KeepEmptyParts);

        if (values.count() == mTestConditionsAvailable.count())
        {
            QMap<QString, QString> conditionGroup;

            for (int lIdxValue = 0; lIdxValue < values.count(); ++lIdxValue)
            {
                conditionGroup.insert(mTestConditionsAvailable.at(lIdxValue),
                                      values.at(lIdxValue));
            }

            lConditionGroups.append(conditionGroup);
        }
    }

    // Set the diffetents conditions combinations
    widgetConditionHierarchy->SetConditionsGroups(lConditionGroups);

    QHash<QString, QString> lFilters;
    if (comboBoxTCFilter1->currentText() != gexLabelFilterChoices[GEX_QUERY_FILTER_NONE] &&
            comboBoxTCValue1->currentText() != "*")
        lFilters.insert(comboBoxTCFilter1->currentText(), comboBoxTCValue1->currentText());

    if (comboBoxTCFilter2->currentText() != gexLabelFilterChoices[GEX_QUERY_FILTER_NONE] &&
            comboBoxTCValue2->currentText() != "*")
        lFilters.insert(comboBoxTCFilter2->currentText(), comboBoxTCValue2->currentText());

    if (comboBoxTCFilter3->currentText() != gexLabelFilterChoices[GEX_QUERY_FILTER_NONE] &&
            comboBoxTCValue3->currentText() != "*")
        lFilters.insert(comboBoxTCFilter3->currentText(), comboBoxTCValue3->currentText());

    if (comboBoxTCFilter4->currentText() != gexLabelFilterChoices[GEX_QUERY_FILTER_NONE] &&
            comboBoxTCValue4->currentText() != "*")
        lFilters.insert(comboBoxTCFilter4->currentText(), comboBoxTCValue4->currentText());

    if (comboBoxTCFilter5->currentText() != gexLabelFilterChoices[GEX_QUERY_FILTER_NONE] &&
            comboBoxTCValue5->currentText() != "*")
        lFilters.insert(comboBoxTCFilter5->currentText(), comboBoxTCValue5->currentText());

    widgetConditionHierarchy->SetFilters(lFilters);
    widgetConditionHierarchy->SetConditionLevel(lSelectedTC);

    widgetConditionHierarchy->setEnabled(true);
    pushButtonBuildHierarchy->setEnabled(false);
    mIsHierarchyUpToDate = true;

    mNullTestConditions.removeDuplicates();
    if (mNullTestConditions.isEmpty() == false)
    {
        labelCharacMessages->setText("Test condition with <b>NULL</b> value detected: <b>" + mNullTestConditions.join(", ") + "</b>");
    }
    else
    {
        labelCharacMessages->clear();
    }

}

void GexOneQueryWizardPage1::OnKeepHierarchyUpToDateSelected(bool aIsSelected)
{
    if (aIsSelected)
    {
        radioButtonHierarchyUpToDate->setChecked(false);
    }

    if (mIsHierarchyUpToDate == false)
    {
        BuildHierarchy();
    }
    UpdateConditionsRender();
}

void GexOneQueryWizardPage1::OnRebuildHierarchyOnDemandSelected(bool aIsSelected)
{
    if (aIsSelected)
    {
        radioButtonHierarchyUpToDate->setChecked(false);
    }
    UpdateConditionsRender();
}

void GexOneQueryWizardPage1::OnConditionsLevelChanged()
{
    mIsHierarchyUpToDate = false;
    if (radioButtonHierarchyUpToDate->isChecked())
    {
        BuildHierarchy();
    }
    UpdateConditionsRender();
}

void GexOneQueryWizardPage1::OnNullConditionDetected(QString aCondtion)
{
    mNullTestConditions.append(aCondtion);
}


///////////////////////////////////////////////////////////
// Returns the Script string for the associated filter selection
///////////////////////////////////////////////////////////
void GexOneQueryWizardPage1::WriteFilterScriptLine(GexDatabaseEntry	*pDatabaseEntry,
                                                   FILE *hFile,
                                                   const QString& strFilterName,
                                                   QString strFilterValue)
{
    // Check if valid filter
    if(strFilterName.isEmpty() || (strFilterName == gexLabelFilterChoices[GEX_QUERY_FILTER_NONE]))
        return;		// Filter not active

    // Ignore '*' filter as it has no effect!
    if(strFilterValue == "*")
        return;

    ConvertToScriptString(strFilterValue);

    // Get script argument to return. e.g: 'tester_id'
    // Script command depends on whether remote SQL DB or not
    if(pDatabaseEntry->IsExternal() && !checkBoxOfflineQuery->isChecked())
    {
        // Query on remote SQL DB
        fprintf(hFile,"  gexQuery('dbf_sql','%s','%s');\n",strFilterName.toLatin1().constData(),strFilterValue.toLatin1().constData());
    }
    else
    {
        // Query on local file-based DB
        int filterIndex = GS::Gex::Engine::GetInstance().GetDatabaseEngine().GetLabelFilterIndex(strFilterName);

        if (filterIndex != 0)
        {
            QString lFieldName = gexFilterChoices[filterIndex];
            fprintf(hFile,"  gexQuery('%s','%s');\n",
                    lFieldName.toLatin1().constData(),
                    strFilterValue.toLatin1().constData());
        }
    }
}

///////////////////////////////////////////////////////////
// Returns the Script string for the associated split field
///////////////////////////////////////////////////////////
void GexOneQueryWizardPage1::WriteSplitFieldScriptLine(GexDatabaseEntry	*pDatabaseEntry,
                                                       FILE *hFile,
                                                       const QStringList &splitFields)
{
    // Check if valid is active
    if(splitFields.isEmpty() == false)
    {
        foreach(const QString &fieldName, splitFields)
        {
            // Get script argument to return. e.g: 'tester_id'
            // Script command depends on whether remote SQL DB or not
            if(pDatabaseEntry->IsExternal() && !checkBoxOfflineQuery->isChecked())
            {
                // Query on remote SQL DB
                fprintf(hFile,"  gexQuery('db_split','dbf_sql','%s');\n", fieldName.toLatin1().constData());
            }
            else
            {
                // Query on local file-based DB
                int filterIndex = GS::Gex::Engine::GetInstance().GetDatabaseEngine().GetLabelFilterIndex(fieldName);

                if (filterIndex != 0)
                {
                    QString lNewFieldName = gexFilterChoices[filterIndex];
                    fprintf(hFile,"  gexQuery('db_split','%s');\n", lNewFieldName.toLatin1().constData());
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////
// Returns the string for the associated filter selection
// Used for creating XML SQL Enterprise report
// Format: <filed_name>=<value>
///////////////////////////////////////////////////////////
QString GexOneQueryWizardPage1::GetFilterXmlLine(QComboBox *pFilter,QComboBox *pFilterString)
{
    if(pFilter->currentIndex() == GEX_QUERY_FILTER_NONE)
        return "";	// Filter not active

    // Check if SQL database
    GexDatabaseEntry	*pDatabaseEntry = GetSelectedDatabase();

    // Do nothing if database not found
    if(!pDatabaseEntry)
        return "";

    QString strFilterName,strFilterValue;
    if((pDatabaseEntry->IsExternal()) && !checkBoxOfflineQuery->isChecked())
    {
        strFilterName = pFilter->currentText();
        strFilterValue = pFilterString->currentText();
    }
    else
    {
        strFilterName = gexLabelFilterChoices[pFilter->currentIndex()];
        strFilterValue = pFilterString->currentText();
    }

    if(strFilterValue == "*")
        return "";	// Ignore '*' filter as it has no effect!

    return (strFilterName + "=" + strFilterValue);
}

///////////////////////////////////////////////////////////
// Returns list of strings for all 8 filter selections
// Used for creating XML SQL Enterprise report
// Format is lines of: <filed_name>=<value>
///////////////////////////////////////////////////////////
QStringList GexOneQueryWizardPage1::GetFiltersXmlLines(void)
{
    QStringList strXmlFilters;
    QString		strString;
    strString = GetFilterXmlLine(comboBoxFilter1,FilterString1);
    if(strString.isEmpty() == false)
        strXmlFilters += strString;
    strString = GetFilterXmlLine(comboBoxFilter2,FilterString2);
    if(strString.isEmpty() == false)
        strXmlFilters += strString;
    strString = GetFilterXmlLine(comboBoxFilter3,FilterString3);
    if(strString.isEmpty() == false)
        strXmlFilters += strString;
    strString = GetFilterXmlLine(comboBoxFilter4,FilterString4);
    if(strString.isEmpty() == false)
        strXmlFilters += strString;
    strString = GetFilterXmlLine(comboBoxFilter5,FilterString5);
    if(strString.isEmpty() == false)
        strXmlFilters += strString;
    strString = GetFilterXmlLine(comboBoxFilter6,FilterString6);
    if(strString.isEmpty() == false)
        strXmlFilters += strString;
    strString = GetFilterXmlLine(comboBoxFilter7,FilterString7);
    if(strString.isEmpty() == false)
        strXmlFilters += strString;
    strString = GetFilterXmlLine(comboBoxFilter8,FilterString8);
    if(strString.isEmpty() == false)
        strXmlFilters += strString;

    // Return list of filters.
    return strXmlFilters;
}

///////////////////////////////////////////////////////////
// Script function to define a Query
///////////////////////////////////////////////////////////
void GexOneQueryWizardPage1::WriteProcessFilesSection(FILE *hFile, bool bGenSTDFList)
{
    QString				strTitle;
    QString				strDatabaseName,strLocation;
    QString				strDataType;
    QDate				dFrom,dTo;
    QTime				tFrom, tTo;
    QString				strNewMinimumPartsPerFile,strParts,strRange,strMapFile;
    QString				strTestList;		// Filter can narrow to a given test list (only supported in RemoteDatabase mode)
    QString				strOptionsString;	// RDB plug-in option string (only supported in RemoteDatabase mode)
    GexDatabaseEntry	*pDatabaseEntry=NULL;

    QStringList         conditionHierarchy;
    QStringList         conditionLevel = widgetConditionLevel->GetSelectedFields();

    // Get ptr to database entry
    strLocation = strDatabaseName = comboBoxDatabaseName->currentText();
    strLocation = "[Local]";
    if(strDatabaseName.startsWith("[Local]") || strDatabaseName.startsWith("[Server]"))
        strDatabaseName = strDatabaseName.section("]",1).trimmed();
    pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(strDatabaseName);

    // Do nothing if database not found
    if(!pDatabaseEntry)
        return;

    if (pDatabaseEntry->IsCharacTdr())
        conditionHierarchy = widgetConditionHierarchy->GetItems();

    if(!pDatabaseEntry->IsStoredInFolderLocal())
        strLocation = "[Server]";

    // Get Query
    strTitle = lineEditReportTitle->text();			// Query Title
    ConvertToScriptString(strTitle);				// to be script compatible '\' become '\\'
    ConvertToScriptString(strDatabaseName);			// to be script compatible '\' become '\\'
    strDataType = comboBoxDatabaseType->currentText();
    dFrom= FromDate->date();
    dTo= ToDate->date();
    tFrom = FromTimeEdit->time();
    tTo = ToTimeEdit->time();
    strNewMinimumPartsPerFile = lineEditMinimumPartPerFile->text();
    strParts    = gexFileProcessPartsItems[DataTypeFilter->currentIndex()];	// Data type to filter GEX_PROCESSPART_ALL, etc..

    pGexMainWindow->forceToComputeFromDataSample(hFile, DataTypeFilter->currentIndex());

    strRange	= lineEditProcess->text();			// range list.
    strMapFile  = LineEditMapTests->text();			// Mapping file.
    ConvertToScriptString(strMapFile);
    strTestList = lineEditParameterList->text();	// Test list to focus on (only applies to Remote DB)
    // Get plugin custom options (option string)
    GetPluginOptionsString(strOptionsString, pDatabaseEntry);

    // Write script sequence
    fprintf(hFile,"  // Single query...\n");
    fprintf(hFile,"  gexGroup('reset','all');\n");

    // Write test condition declaration
    foreach(const QString &condition, conditionLevel)
        fprintf(hFile, "  gexGroup('declare_condition', '%s');\n", condition.toLatin1().data());

    fprintf(hFile,"  gexQuery('db_report','%s');\n", strTitle.toLatin1().constData());	// Report folder name or CSV file name

    // If Enterprise report (SQL) to be created, then there are no file(s) dataset to analyze!
    if(pGexMainWindow->pWizardSettings->IsEnterpriseSqlReport())
        return;

    if (this->comboBoxExtractionMode->currentIndex()==1)
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, " consolidated selected : forcing stat and bin computations from samples");
        fprintf(hFile,"   // consolidation mode : forcing computation from samples to be sure to have all pareto...\n");
        fprintf(hFile,"   gexOptions('statistics','computation','samples_only');\n");
        // 6374
        QString bin_comp="samples";
        if (strDataType=="Wafer Sort")
            bin_comp="wafer_map";

        fprintf(hFile,"   gexOptions('binning','computation','%s');\n", bin_comp.toLatin1().data());
        fprintf(hFile,"   gexOptions('dataprocessing','part_identification','auto');\n");
    }

    do
    {
        QStringList lConditionValue;
        QString     lQueryName = strTitle;

        if (conditionHierarchy.isEmpty() == false)
            lConditionValue = conditionHierarchy.takeFirst().split("|", QString::SkipEmptyParts);

        // Database location: [Local] or [Server]
        fprintf(hFile,"  gexQuery('db_location','%s');\n",strLocation.toLatin1().constData());

        // Database name
        fprintf(hFile,"  gexQuery('db_name','%s');\n",strDatabaseName.toLatin1().constData());

        // Tell if query over local cache or over database
        if(checkBoxOfflineQuery->isChecked())
            fprintf(hFile,"  gexQuery('db_offline_query','true');\n");
        else
            fprintf(hFile,"  gexQuery('db_offline_query','false');\n");

        // Data type (to query within database. eg: Wafer sort, e-test, final-test)
        fprintf(hFile,"  gexQuery('db_data_type','%s');\n", strDataType.toLatin1().constData());

        fprintf(hFile,"  gexQuery('db_consolidated','%s');\n",
                this->comboBoxExtractionMode->currentIndex()==1?"true":"false" ); // 6050

        // 6050
        if (comboBoxExtractionMode->currentIndex()==2)
            fprintf(hFile,"  gexQuery('db_extraction_mode','1_out_of_N_samples', '%s');\n", mExtractionModeLineEdit->text().toLatin1().data() );

        // Specify time period
        QString strFromTime, strToTime;
        strFromTime = tFrom.toString("hh:mm:ss");
        strToTime = tTo.toString("hh:mm:ss");
        if (comboBoxTimePeriod->currentIndex()!=GEX_QUERY_TIMEPERIOD_LAST_N_X)
            fprintf(hFile,"  gexQuery('db_period','%s','%d %d %d %d %d %d %s %s');\n",
                gexTimePeriodChoices[comboBoxTimePeriod->currentIndex()],
                dFrom.year(),dFrom.month(),dFrom.day(),
                dTo.year(),dTo.month(),dTo.day(),
                strFromTime.toLatin1().constData(), strToTime.toLatin1().constData());
        else
        {
            fprintf(hFile,"  gexQuery('db_period','%s','%s %s');\n",
                gexTimePeriodChoices[comboBoxTimePeriod->currentIndex()],
                lineEditLastNXFactor->text().toLatin1().data(),
                comboBoxLastNXStep->currentText().toLatin1().data()
                    );
        }

        // Save filters (if activated)
        WriteFilterScriptLine(pDatabaseEntry, hFile, comboBoxFilter1->currentText(),
                              FilterString1->currentText());
        WriteFilterScriptLine(pDatabaseEntry, hFile, comboBoxFilter2->currentText(),
                              FilterString2->currentText());
        WriteFilterScriptLine(pDatabaseEntry, hFile, comboBoxFilter3->currentText(),
                              FilterString3->currentText());
        WriteFilterScriptLine(pDatabaseEntry, hFile, comboBoxFilter4->currentText(),
                              FilterString4->currentText());
        WriteFilterScriptLine(pDatabaseEntry, hFile, comboBoxFilter5->currentText(),
                              FilterString5->currentText());
        WriteFilterScriptLine(pDatabaseEntry, hFile, comboBoxFilter6->currentText(),
                              FilterString6->currentText());
        WriteFilterScriptLine(pDatabaseEntry, hFile, comboBoxFilter7->currentText(),
                              FilterString7->currentText());
        WriteFilterScriptLine(pDatabaseEntry, hFile, comboBoxFilter8->currentText(),
                              FilterString8->currentText());

        if (pDatabaseEntry->IsCharacTdr())
        {
            WriteFilterScriptLine(pDatabaseEntry, hFile,
                                  comboBoxTCFilter1->currentText(),
                                  comboBoxTCValue1->currentText());
            WriteFilterScriptLine(pDatabaseEntry, hFile,
                                  comboBoxTCFilter2->currentText(),
                                  comboBoxTCValue2->currentText());
            WriteFilterScriptLine(pDatabaseEntry, hFile,
                                  comboBoxTCFilter3->currentText(),
                                  comboBoxTCValue3->currentText());
            WriteFilterScriptLine(pDatabaseEntry, hFile,
                                  comboBoxTCFilter4->currentText(),
                                  comboBoxTCValue4->currentText());
            WriteFilterScriptLine(pDatabaseEntry, hFile,
                                  comboBoxTCFilter5->currentText(),
                                  comboBoxTCValue5->currentText());
        }

        // Save split field (if activated)
        QStringList splitFields;

        splitFields.append(lineEditSplitField->text().split(',', QString::SkipEmptyParts));

        if (pDatabaseEntry->IsCharacTdr())
        {
            if (lConditionValue.isEmpty() == false && lConditionValue.count() == conditionLevel.count())
            {
                lQueryName.clear();
                for (int lIdx = 0; lIdx < conditionLevel.count(); ++lIdx)
                {
                    WriteFilterScriptLine(pDatabaseEntry, hFile,
                                          conditionLevel.at(lIdx), lConditionValue.at(lIdx));

                    if (lQueryName.isEmpty() == false)
                        lQueryName += "_";

                    lQueryName += conditionLevel.at(lIdx) + "@" + lConditionValue.at(lIdx);
                }
            }
            else
                splitFields.append(conditionLevel);
        }

        WriteSplitFieldScriptLine(pDatabaseEntry, hFile, splitFields);

        // Minimum parts per file
        fprintf(hFile,"  gexQuery('db_minimum_samples','%s');\n",strNewMinimumPartsPerFile.toLatin1().constData());

        // Parts type to process (all, good only, fail, etc.)
        strRange.replace(",",";");	// Ensure comma is not found in range string!
        fprintf(hFile,"  gexQuery('db_data','%s','%s');\n",strParts.toLatin1().constData(),strRange.toLatin1().constData());

        // Mapping file
        fprintf(hFile,"  gexQuery('db_mapping','%s');\n",strMapFile.toLatin1().constData());

        // Remote Database Parameter list (ignored if query not done over a remoteDB)
        fprintf(hFile,"  gexQuery('db_testlist','%s');\n",strTestList.toLatin1().constData());

        // Remote Database option string (ignored if query not done over a remoteDB, or no options supported)
        if(!strOptionsString.isEmpty())
            fprintf(hFile,"  gexQuery('db_plugin_options','%s');\n",strOptionsString.toLatin1().constData());

        // All Query parameters specified...now trigger query!
        if(!bGenSTDFList)
        {
            fprintf(hFile,"  group_id = gexGroup('insert_query','%s');\n",lQueryName.toLatin1().constData());

            if (lConditionValue.isEmpty() == false &&
                lConditionValue.count() == conditionLevel.count())
            {
                for (int lIdx = 0; lIdx < conditionLevel.count(); ++lIdx)
                    fprintf(hFile,"  gexCondition(group_id,'%s','%s');\n",
                            conditionLevel.at(lIdx).toLatin1().constData(),
                            lConditionValue.at(lIdx).toLatin1().constData());
            }
        }
        else{
            QString strQueryName = lineEditReportTitle->text();
            strQueryName.replace(QRegExp("[^a-zA-Z0-9\\\\d\\\\s]"),"_");
            QDir oDestDir(m_strCSVDestinationPath + QDir::separator() + strQueryName);
            if(!oDestDir.exists())
                oDestDir.mkdir(oDestDir.absolutePath());
            fprintf(hFile,"  gexGenCSVFromQuery('%s');\n",oDestDir.absolutePath().toLatin1().constData());
        }
    } while (conditionHierarchy.isEmpty() == false);


    fprintf(hFile,"  gexAnalyseMode('SingleDataset');\n");
    fprintf(hFile,"\n  sysLog('* Quantix ExaminatorDB Query set ! *');\n\n");
}

///////////////////////////////////////////////////////////
// Database Center deleted: reset RDB plug-ins handle
///////////////////////////////////////////////////////////
void GexOneQueryWizardPage1::ResetRdbPluginHandles()
{
    // Database Center deleted, so the ptrs to plug-in option widget is no more valid,
    // as the pointed widgets don't exist anymore. If in use, just remove the ptr from the GUI
    // and show the placeholder (to keep a consistent layout)
    if(m_pRdbOptionsWidget)
    {
        // Remove old RDB Options widget
        // and set its parent to NULL as the RDB plugin is responsible to delete it
        m_pRdbOptionsWidget->hide();
        tabRdbOptions->layout()->removeWidget(m_pRdbOptionsWidget);
        m_pRdbOptionsWidget->setParent(NULL);
        m_pRdbOptionsWidget = NULL;
    }
    labelPlaceHolder->show();
}

///////////////////////////////////////////////////////////
// Get plug-in options string if applicable
///////////////////////////////////////////////////////////
void GexOneQueryWizardPage1::GetPluginOptionsString(QString & strOptionsString, GexDatabaseEntry *pDatabaseEntry/*=NULL*/)
{
    GexDatabaseEntry	*pLocalDatabaseEntry=pDatabaseEntry;
    QString				strLocation, strDatabaseName;

    if(!pLocalDatabaseEntry)
    {
        // Get ptr to database entry
        strLocation = strDatabaseName = comboBoxDatabaseName->currentText();
        strLocation = "[Local]";
        if(strDatabaseName.startsWith("[Local]") || strDatabaseName.startsWith("[Server]"))
            strDatabaseName = strDatabaseName.section("]",1).trimmed();
        pLocalDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(strDatabaseName);

        // Do nothing if database not found
        if(!pLocalDatabaseEntry)
            return;

        if(!pLocalDatabaseEntry->IsStoredInFolderLocal())
            strLocation = "[Server]";
    }

    if(pLocalDatabaseEntry->IsExternal() && pLocalDatabaseEntry->m_pExternalDatabase)
        pLocalDatabaseEntry->m_pExternalDatabase->GetRdbOptionsString(strOptionsString);
}

///////////////////////////////////////////////////////////
// Set plug-in options string if applicable
///////////////////////////////////////////////////////////
void GexOneQueryWizardPage1::SetPluginOptionsString(QString & strOptionsString, GexDatabaseEntry *pDatabaseEntry/*=NULL*/)
{
    GexDatabaseEntry	*pLocalDatabaseEntry=pDatabaseEntry;
    QString				strLocation, strDatabaseName;

    if(!pLocalDatabaseEntry)
    {
        // Get ptr to database entry
        strLocation = strDatabaseName = comboBoxDatabaseName->currentText();
        strLocation = "[Local]";
        if(strDatabaseName.startsWith("[Local]") || strDatabaseName.startsWith("[Server]"))
            strDatabaseName = strDatabaseName.section("]",1).trimmed();
        pLocalDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(strDatabaseName);

        // Do nothing if database not found
        if(!pLocalDatabaseEntry)
            return;

        if(!pLocalDatabaseEntry->IsStoredInFolderLocal())
            strLocation = "[Server]";

    }

    if(pLocalDatabaseEntry->IsExternal() && pLocalDatabaseEntry->m_pExternalDatabase)
        pLocalDatabaseEntry->m_pExternalDatabase->SetRdbOptionsString(strOptionsString);
}

QString GexOneQueryWizardPage1::ExportToCSVConditionFormat()
{
    GexDatabaseQuery lQuery;
    GetQueryFields(lQuery);
    if(lineEditParameterList && !lineEditParameterList->text().isEmpty())
    {
        lQuery.strTestList = lineEditParameterList->text();
    }
    QMap<QString, QString>  lConditions;
    QStringList lConditionsName = widgetConditionLevel->GetSelectedFields();
    QStringList lConditionsValues = widgetConditionHierarchy->GetItems();
    QStringList lConditionsSort = lConditionsName;
    QStringList lConditionsValuesOrder = lConditionsValues;
    for(int lValuesIdx=0; lValuesIdx<lConditionsValues.count(); lValuesIdx++)
    {
        QStringList oValuesList = lConditionsValues[lValuesIdx].split("|");
        for(int iValIdx=0; iValIdx<oValuesList.count(); iValIdx++)
        {
            if(!lConditions.contains(lConditionsName[iValIdx]))
            {
                lConditions.insert(lConditionsName[iValIdx],oValuesList[iValIdx]);
            } else
            {
                if(!lConditions[lConditionsName[iValIdx]].contains(oValuesList[iValIdx]))
                    lConditions[lConditionsName[iValIdx]]+= "|" + oValuesList[iValIdx];
            }
        }
    }

    if(!lConditions.isEmpty())
    {
        lConditions.insert("gui_order", lConditionsSort.join("|"));
        lConditions.insert("values_order", lConditionsValuesOrder.join("#"));
    }

    pushButtonNext->setDisabled(true);
    QProgressDialog lDialog("", "Cancel", 0,100, this);
    lDialog.setAutoClose(false);
    lDialog.setAutoReset(false);
    lDialog.show();
    GS::Gex::Engine::GetInstance().GetDatabaseEngine().ExportCSVCondition(&lQuery, lConditions, &lDialog);
    lDialog.reset();
    pushButtonNext->setDisabled(false);
    return "ok";
}

QString GexOneQueryWizardPage1::ExportQueryToCSV()
{
    QString lDefaultPath(QDir::homePath()+QDir::separator()+ GEX_DEFAULT_DIR +QDir::separator()+ "exports") ;
    QDir lDestDir(lDefaultPath);
    if(!lDestDir.exists())
        lDestDir.mkdir(lDefaultPath);
    if(m_strCSVDestinationPath.isEmpty())
        m_strCSVDestinationPath = lDefaultPath;

    QString lSelectedDir =
            QFileDialog::getExistingDirectory(this,
                                              "Select the destination directory for the generated CSV",
                                              m_strCSVDestinationPath);
    if(lSelectedDir.isEmpty())
        return "ok";
    if(lSelectedDir != lDefaultPath)
    {
        lDestDir.setPath(lSelectedDir);
        if(!lDestDir.exists())
            lDestDir.mkdir(lSelectedDir);
    }
    m_strCSVDestinationPath = lDestDir.absolutePath();

    // If a script is already running, then the button is a 'abort' instead!
    if(GS::Gex::CSLEngine::GetInstance().IsRunning())
    {
        GS::Gex::CSLEngine::GetInstance().AbortScript();
        return "error : a script is already running. Please wait and retry.";
    }

    // Writes script footer : writes 'main' function+ call to 'Favorites'+SetOptions+SetDrill
    int lFlags = GEX_SCRIPT_SECTION_OPTIONS | GEX_SCRIPT_SECTION_GROUPS;

    // Create header section of the script file
    FILE *lFile=NULL;
    // Creates header of Gex Assistant script file.
    lFile = pGexMainWindow->CreateGexScript(GEX_SCRIPT_ASSISTANT);
    if(lFile == NULL)
        return QString("error : cant create script file '%1' !").arg(GEX_SCRIPT_ASSISTANT);

    // Creates 'SetOptions' section
    if(!(ReportOptions.WriteOptionSectionToFile(lFile)))
    {
        QString lErrorMsg("Error : problem occured when write option section");
        GSLOG(SYSLOG_SEV_WARNING, lErrorMsg.toLatin1().constData());
        GEX_ASSERT(false);
        return lErrorMsg;
    }

    QString lStartupPage;
    // Creates 'SetReportType' section.
    pGexMainWindow->pWizardSettings->WriteScriptReportSettingsSection(lFile,false,lStartupPage);

    // Writes 'SetProcessData' section
    fprintf(lFile,"//////////////////////////////////////////\n");
    fprintf(lFile,"// List Queries or files groups to process\n");
    fprintf(lFile,"//////////////////////////////////////////\n");
    fprintf(lFile,"SetProcessData()\n");
    fprintf(lFile,"{\n");
    fprintf(lFile,"  var group_id;  // Holds group_id when creating groups\n\n");

    // Creates 'SetProcessData' section
    this->WriteProcessFilesSection(lFile, true);//=> generate script file.

    fprintf(lFile,"}\n");
    fprintf(lFile,"\n");

    pGexMainWindow->CloseGexScript(lFile,lFlags);

    // Launch script!
    GS::Gex::CSLEngine::GetInstance().RunScript(GS::Gex::Engine::GetInstance().GetAssistantScript().toLatin1().data());

    return "ok";
}

void GexOneQueryWizardPage1::setProhibatedList(const QStringList &oProhibitedName){
    m_oProhibitedName = oProhibitedName;
    connect(lineEditReportTitle, SIGNAL(textChanged(const QString & )), this, SLOT(checkGroupName(const QString & )));
    checkGroupName(QString());

}

void GexOneQueryWizardPage1::checkGroupName(const QString &){
    QPalette oPal;
    if(m_oProhibitedName.contains(lineEditReportTitle->text())){
        oPal.setColor(QPalette::Text, Qt::red);
        PushButtonOk->setDisabled(true);
    }
    else{
        oPal.setColor(QPalette::Text, Qt::black);
        PushButtonOk->setDisabled(false);
    }
    lineEditReportTitle->setPalette(oPal);
}

void GexOneQueryWizardPage1::InitNextActionLabels()
{
    mNextActionLabels.clear();
    mNextActionLabels.insert(BUILDREPORT, "Build Quantix Report");
    mNextActionLabels.insert(EXPORTTOCSV, "Export to CSV");
}

void GexOneQueryWizardPage1::InitExtractionModeLabels()
{
    mExtractionModeLabels.clear();
    mExtractionModeLabels.insert(eRawData, "Extract all parts (test & retest)");
    mExtractionModeLabels.insert(eConsolidatedData, "Consolidated parts (merge test & retest)");
    mExtractionModeLabels.insert(e1outOfNsamples, "Extract 1 out of N (sampling)");
}

void GexOneQueryWizardPage1::InitComboBoxExtractionMode()
{
    InitExtractionModeLabels();
    comboBoxExtractionMode->addItem(mExtractionModeLabels.value(eRawData), eRawData);
    comboBoxExtractionMode->addItem(mExtractionModeLabels.value(eConsolidatedData), eConsolidatedData);
    comboBoxExtractionMode->addItem(mExtractionModeLabels.value(e1outOfNsamples), e1outOfNsamples);
}

void GexOneQueryWizardPage1::InitNextActionFrame()
{
    InitNextActionLabels();
    comboBoxNextAction->clear();

    if (m_iDialogMode == ePATRecipe)
    {
        labelNextStep->hide();
        comboBoxNextAction->hide();
        PushButtonOk->hide();
        pushButtonNext->hide();
    }
    else if (mPurgeDb)
    {
        labelNextStep->hide();
        comboBoxNextAction->hide();
        pushButtonNext->setText("Purge DataBase");
    }
    else
    {
        labelNextStep->show();
        comboBoxNextAction->show();
        pushButtonNext->setText("Next >>");
        comboBoxNextAction->addItem(mNextActionLabels.value(BUILDREPORT), BUILDREPORT);
        comboBoxNextAction->addItem(mNextActionLabels.value(EXPORTTOCSV), EXPORTTOCSV);
    }
}

void GexOneQueryWizardPage1::UpdateExtractionModeUi(int extractionMode/*= -1*/)
{
    // Show if:
    // - not charac DB selected
    // - not eTestSelected
    bool lShowExtractionMode = false;
    GexDatabaseEntry	*lDatabaseEntry = GetSelectedDatabase();

    // Keep current selected mode if any
    if (extractionMode == -1)
    {
        extractionMode = static_cast<GexOneQueryWizardPage1::eExtractionMode>(
                        comboBoxExtractionMode->itemData(
                        comboBoxExtractionMode->currentIndex()).toInt());
    }

    if (lDatabaseEntry)
    {
        // Test if the Data base is >= b27
        QString lDbVersion = lDatabaseEntry->DatabaseVersion().section(' ',2,2).simplified();
        int version = (lDbVersion.section("B",1)).toInt();
        if (version < 27
            && comboBoxDatabaseType->currentText().toLower() == "final test"
            /*&& comboBoxExtractionMode->currentIndex() == 1*/)
        {
            comboBoxExtractionMode->clear();
            comboBoxExtractionMode->addItem(mExtractionModeLabels.value(eRawData), eRawData);
            comboBoxExtractionMode->addItem(mExtractionModeLabels.value(eConsolidatedData), eConsolidatedData);
            comboBoxExtractionMode->addItem(mExtractionModeLabels.value(e1outOfNsamples), e1outOfNsamples);
            comboBoxExtractionMode->show();
            // Get the index of the value to disable
            QModelIndex lIndex = comboBoxExtractionMode->model()->index(1, 0);
            // This is the effective 'disable' flag
            QVariant lVariant(0);
            // the magic
            comboBoxExtractionMode->model()->setData(lIndex, lVariant, Qt::UserRole - 1);
            GSLOG(SYSLOG_SEV_WARNING, QString("Database '%1' has to be updated !").arg(lDatabaseEntry->LogicalName())
                                                .toLatin1().constData());
        }
        else
        {
            comboBoxExtractionMode->clear();
            comboBoxExtractionMode->addItem(mExtractionModeLabels.value(eRawData), eRawData);
            comboBoxExtractionMode->addItem(mExtractionModeLabels.value(eConsolidatedData), eConsolidatedData);
            comboBoxExtractionMode->addItem(mExtractionModeLabels.value(e1outOfNsamples), e1outOfNsamples);
            comboBoxExtractionMode->show();
        }
    }


    if (((comboBoxDatabaseType->currentText().toLower() == "final test")
            || (comboBoxDatabaseType->currentText().toLower() == "wafer sort"))
            && lDatabaseEntry
            && !lDatabaseEntry->IsCharacTdr()
            && lDatabaseEntry->IsExternal()
            && !mPurgeDb
            && m_iDialogMode != ePATRecipe)
        lShowExtractionMode = true;

    if (!lShowExtractionMode)
    {
        comboBoxExtractionMode->hide();
        mExtractionModeLineEdit->hide();
        return;
    }
    else
        comboBoxExtractionMode->show();

    eExtractionMode lExtractionMode;
    // if forced value
    if (extractionMode != -1)
    {
        lExtractionMode = static_cast<eExtractionMode>(extractionMode);
        comboBoxExtractionMode->setCurrentIndex(
                    comboBoxExtractionMode->findData(QVariant(lExtractionMode)));
    }
    else
    {
        lExtractionMode = static_cast<eExtractionMode>(
                        comboBoxExtractionMode->itemData(
                        comboBoxExtractionMode->currentIndex()).toInt());
    }

    switch(lExtractionMode)
    {
    case GexOneQueryWizardPage1::eRawData:
    case GexOneQueryWizardPage1::eConsolidatedData:
        mExtractionModeLineEdit->hide();
        break;
    case GexOneQueryWizardPage1::e1outOfNsamples:
        mExtractionModeLineEdit->show();
        break;
    default:
        GSLOG(SYSLOG_SEV_WARNING, QString("Unknown extraction mode : (%1) %2")
              .arg(comboBoxExtractionMode->currentIndex())
              .arg(comboBoxExtractionMode->currentText()).toLatin1().constData());
    }
}

bool GexOneQueryWizardPage1::FillDataTab(const QString &dataDescription)
{
    return true;
}

GexDatabaseEntry *GexOneQueryWizardPage1::GetSelectedDatabase()
{
    GexDatabaseEntry	*lDatabaseEntry = NULL;
    QString				lDatabaseLogicalName = comboBoxDatabaseName->currentText();

    if (!lDatabaseLogicalName.isEmpty())
    {
        if(lDatabaseLogicalName.startsWith("[Local]") || lDatabaseLogicalName.startsWith("[Server]"))
            lDatabaseLogicalName = lDatabaseLogicalName.section("]",1).trimmed();
        lDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(lDatabaseLogicalName);
    }

    return lDatabaseEntry;
}


