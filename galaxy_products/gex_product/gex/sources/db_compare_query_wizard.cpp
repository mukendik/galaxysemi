///////////////////////////////////////////////////////////
// ALL Browser wizard pages for 'Compare Datasets' (N queries)
///////////////////////////////////////////////////////////
#include <QMenu>
#include <qevent.h>
#include <QRegExp>
#include <QMessageBox>

#include "csl/cslscriptparser.h"
#include "compare_query_wizard.h"
#include "db_onequery_wizard.h"
#include "settings_dialog.h"
#include "browser_dialog.h"
#include "report_build.h"
#include "report_options.h"
#include "db_transactions.h"
#include "gex_database_entry.h"
#include "gex_constants.h"
#include "db_engine.h"
#include "gex_shared.h"
#include <gqtl_log.h>
#include "product_info.h"
#include "common_widgets/conditions_widget.h"
#include "engine.h"
#include "admin_gui.h"
#include "message.h"

// External definitions for pointers to pixmaps.
#include "gex_pixmap_extern.h"

// in main.cpp
extern GexMainwindow *	pGexMainWindow;

// report_build.cpp
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)

// in script_wizard.h
extern void ConvertToScriptString(QString &strFile);

// in classes.cpp
extern bool SetCurrentComboItem(QComboBox *pCombo, const QString & strItem, bool bInsertIfMissing=false);

#define FILTERS_SEPARATOR ";;"
///////////////////////////////////////////////////////////
// WIZARD PAGE 1: Enter Queries to Merge
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_MergeQuery_Page1(void)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("lProductID = %1").arg(GS::LPPlugin::ProductInfo::getInstance()->getProductID()).toLatin1().data() );
    int	iDatabasesEntries=0;

    // Check if data access type if FILES or DATABASE
    if(!GS::LPPlugin::ProductInfo::getInstance()->isDBPluginAllowed())
    {
//    // DATABASE type
//    case GEX_DATATYPE_GEX_MONITORING:	// ExaminatorMonitoring
//    case GEX_DATATYPE_GEX_PATMAN:		// Galaxy PAT-Man
//    case GEX_DATATYPE_ALLOWED_DATABASE:	// ExaminatorDB: Database access, any file type.
//    case GEX_DATATYPE_GEX_YIELD123:		// Yield123
//    case GEX_DATATYPE_ALLOWED_DATABASEWEB: // ExaminatorWeb
//        break;
//    default:
        QString m=ReportOptions.GetOption("messages", "upgrade").toString();
        GS::Gex::Message::information("", m);
        return;
    }

    // Make sure ExaminatorDB has the latest list of databases (do not repaint Admin GUI page)
    iDatabasesEntries = ReloadDatabasesList(false);

    QComboBox cbDatabase;
    if (iDatabasesEntries > 0)
        // Load all database supported in the combo box
        pWizardAdminGui->ListDatabasesInCombo(&cbDatabase, DB_SELECT_FOR_EXTRACTION);

    // If No database available...display the HTML message instead
    if(cbDatabase.count() == 0)
    {
        // No report available
        QString strFilesPage;
        // Display relevant message
        if(GS::LPPlugin::ProductInfo::getInstance()->isDBPluginAllowed())
        {
//        case GEX_DATATYPE_GEX_YIELD123:		// Yield123
//            strFilesPage = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
//            strFilesPage += GEX_HELP_FOLDER;
//            strFilesPage += GEXY123_HTMLPAGE_NODATABASE;
//            break;
//        case GEX_DATATYPE_ALLOWED_DATABASE:
//        case GEX_DATATYPE_GEX_MONITORING:	// ExaminatorMonitoring
//        case GEX_DATATYPE_GEX_PATMAN:		// Galaxy PAT-Man
            strFilesPage = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
            strFilesPage += GEX_HELP_FOLDER;
            strFilesPage += GEX_HTMLPAGE_DB_NODATABASE;
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
    SetWizardType(GEX_MIXQUERY_WIZARD);

    // Force Report mode = Standard reports (not Enterprise ones)
    pGexMainWindow->pWizardSettings->OnDoInstantReport();

    // Show wizard page : Query dataset.
    ShowWizardDialog(GEX_MIXQUERY_WIZARD_P1);
}

///////////////////////////////////////////////////////////
// WIZARD PAGE 1: Enter Queries to Compare
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_CmpQuery_Page1(void)
{
    // Check if data access type if FILES or DATABASE
    if(!GS::LPPlugin::ProductInfo::getInstance()->isDBPluginAllowed())
    {
//    // DATABASE type
//    case GEX_DATATYPE_GEX_MONITORING:	// ExaminatorMonitoring
//    case GEX_DATATYPE_GEX_PATMAN:		// Galaxy PAT-Man
//    case GEX_DATATYPE_ALLOWED_DATABASE:	// ExaminatorDB: Database access, any file type.
//    case GEX_DATATYPE_GEX_YIELD123:		// Yield123
//    case GEX_DATATYPE_ALLOWED_DATABASEWEB: // ExaminatorWeb
//        break;

//    default:
        QString m=ReportOptions.GetOption("messages", "upgrade").toString();
        GS::Gex::Message::information("", m);
        return;
    }

    // Make sure ExaminatorDB has the latest list of databases (do not repaint Admin GUI page)
    int iDatabasesEntries = ReloadDatabasesList(false);

    QComboBox cbDatabase;
    if (iDatabasesEntries > 0)
        // Load all database supported in the combo box
        pWizardAdminGui->ListDatabasesInCombo(&cbDatabase, DB_SELECT_FOR_EXTRACTION);

    // If No database available...display the HTML message instead
    if(cbDatabase.count() == 0)
    {
        // No report available
        QString strFilesPage;

        // Display relevant message
        if(GS::LPPlugin::ProductInfo::getInstance()->isDBPluginAllowed())
        {
//        case GEX_DATATYPE_GEX_YIELD123:		// Yield123
//            strFilesPage = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
//            strFilesPage += GEX_HELP_FOLDER;
//            strFilesPage += GEXY123_HTMLPAGE_NODATABASE;
//            break;
//        case GEX_DATATYPE_ALLOWED_DATABASE:
//        case GEX_DATATYPE_GEX_MONITORING:	// ExaminatorMonitoring
//        case GEX_DATATYPE_GEX_PATMAN:		// Galaxy PAT-Man
            strFilesPage = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
            strFilesPage += GEX_HELP_FOLDER;
            strFilesPage += GEX_HTMLPAGE_DB_NODATABASE;
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
    SetWizardType(GEX_CMPQUERY_WIZARD);

    // Force Report mode = Standard reports (not Enterprise ones)
    pGexMainWindow->pWizardSettings->OnDoInstantReport();

    // Show wizard page : Query dataset.
    ShowWizardDialog(GEX_CMPQUERY_WIZARD_P1);
}

///////////////////////////////////////////////////////////
// Show page: make it visible & update GUI fields
///////////////////////////////////////////////////////////
void GexCmpQueryWizardPage1::ShowPage(void)
{
    if (pGexMainWindow->iWizardPage == GEX_MIXQUERY_WIZARD_P1)
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString(" GEX_MIXQUERY_WIZARD_P1 (%1)")
              .arg( pGexMainWindow->iWizardPage)
              .toLatin1().constData());
        this->TextLabelTitle->setText( "Please select datasets to merge with queries" );
        this->TextLabelComment->setText("Note: Double-click any query entry in the list to edit its content."); //this->TextLabelComment->hide();
        this->GroupBox1->setTitle("Datasets (queries) to merge:");
    }
    else
        if	(pGexMainWindow->iWizardPage == GEX_CMPQUERY_WIZARD_P1)
        {
            GSLOG(SYSLOG_SEV_DEBUG, " GEX_CMPQUERY_WIZARD_P1");
            this->TextLabelTitle->setText( "Please select datasets to compare with queries" );
            this->TextLabelComment->show();
            retranslateUi(this); // reset all widgets to default one (those in .ui file)
            this->GroupBox1->setTitle("Datasets (queries) to compare:");
        }
        else
        {
            GSLOG(SYSLOG_SEV_NOTICE, QString(" unknown wizard %1 ")
                  .arg( pGexMainWindow->GetWizardType())
                  .toLatin1().constData());
        }

    // Enable/disable some features...
    UpdateSkin(GS::LPPlugin::ProductInfo::getInstance()->getProductID());

    // Make Widget visible.
    show();
}

///////////////////////////////////////////////////////////
// GexCmpQueryWizardPage1: Empty function to ignore the ESCAPE key hit!
///////////////////////////////////////////////////////////
void GexCmpQueryWizardPage1::reject(void)
{
}

///////////////////////////////////////////////////////////
// Update GUI fileds based on Product type running
///////////////////////////////////////////////////////////
void GexCmpQueryWizardPage1::UpdateSkin(int /*lProductID*/)
{
    switch(GS::LPPlugin::ProductInfo::getInstance()->getProductID())
    {
//    case GEX_DATATYPE_GEX_YIELD123:		// Yield123
//        bSortingCapability = false;		// Sorting capability has no impact under Yield123, so simply hide it!
//        // Update GUI fileds & tooltips to match the mission (Yield 123)
//        TextLabelTitle->setText("Datasets to analyze: Queries");
//        GroupBox1->setTitle("Datasets (queries) to analyze");
//        TextLabelComment->hide();
//        break;

    default:
        bSortingCapability = true;
        break;
    }

    if (!GS::LPPlugin::ProductInfo::getInstance()->isDBPluginAllowed())
    {
        m_poCompareQueryLabel->setEnabled(false);
        m_poCompareQueryCondition->setEnabled(false);
    }
    else
    {
        m_poCompareQueryLabel->setEnabled(true);
        m_poCompareQueryCondition->setEnabled(true);
    }
}

///////////////////////////////////////////////////////////
// Manage the Query page
///////////////////////////////////////////////////////////
GexCmpQueryWizardPage1::GexCmpQueryWizardPage1( QWidget* parent, bool modal, Qt::WindowFlags fl) : QDialog(parent, fl)
{
    move(0,0);

    setupUi(this);
    setModal(modal);

    QObject::connect(buttonAddQuery,		SIGNAL(clicked()), this, SLOT(OnAddQuery()));
    QObject::connect(buttonProperties,		SIGNAL(clicked()), this, SLOT(OnProperties()));
    QObject::connect(buttonRemoveQuery,		SIGNAL(clicked()), this, SLOT(OnRemoveQuery()));
    QObject::connect(PushButtonClearAll,	SIGNAL(clicked()), this, SLOT(OnRemoveAll()));
    QObject::connect(buttonUp,				SIGNAL(clicked()), this, SLOT(OnMoveQueryUp()));
    QObject::connect(buttonDown,			SIGNAL(clicked()), this, SLOT(OnMoveQueryDown()));
    QObject::connect(buttonDuplicateTask,	SIGNAL(clicked()), this, SLOT(OnDuplicateQuery()));
    QObject::connect(buttonLoadDatasetList, SIGNAL(clicked()), this, SLOT(OnLoadDatasetList()));
    QObject::connect(buttonSaveDatasetList, SIGNAL(clicked()), this, SLOT(OnSaveDatasetList()));

    QObject::connect(treeWidget,			SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),	this,	SLOT(OnProperties()));
    QObject::connect(treeWidget,			SIGNAL(customContextMenuRequested(const QPoint&)),	this,	SLOT(onContextualMenu(const QPoint&)));
    QObject::connect(treeWidget,			SIGNAL(itemChanged(QTreeWidgetItem*,int)),			this,	SLOT(onItemChanged(QTreeWidgetItem*,int)));

    // Prevent from sorting items.
    treeWidget->setSortingEnabled(false);

    // Until a file is selected, disable the 'Next' & properties buttons
    buttonSettings->setEnabled(false);

    // Focus on Title edit field
    lineEditReportTitle->setText("My Datasets");
    lineEditReportTitle->setFocus();

    // Support for Drag&Drop of .CSL file only.
    setAcceptDrops(true);

    // Until a file is selected, disable the 'Next',FileOpen,Properties,ClearClearAll button
    buttonSettings->setEnabled(false);
    buttonProperties->hide();
    Line1->hide();
    buttonDuplicateTask->hide();
    buttonRemoveQuery->hide();
    PushButtonClearAll->hide();
    Line2->hide();
    buttonUp->hide();
    buttonDown->hide();
    buttonSaveDatasetList->hide();

    // Enable/disable some features...
    UpdateSkin(GS::LPPlugin::ProductInfo::getInstance()->getProductID());

    // Adjust columns width
    treeWidget->resizeColumnToContents(0);
    treeWidget->resizeColumnToContents(1);
    treeWidget->resizeColumnToContents(2);
    treeWidget->resizeColumnToContents(3);
    treeWidget->resizeColumnToContents(4);
    treeWidget->resizeColumnToContents(5);
    treeWidget->resizeColumnToContents(6);
    treeWidget->resizeColumnToContents(7);
    treeWidget->resizeColumnToContents(8);
    treeWidget->resizeColumnToContents(9);
    treeWidget->resizeColumnToContents(10);
    treeWidget->resizeColumnToContents(11);
    treeWidget->resizeColumnToContents(12);

    // Init some variables
    eNewExtractionMode = GexOneQueryWizardPage1::eRawData;
}

///////////////////////////////////////////////////////////
// Starting DRAG sequence
///////////////////////////////////////////////////////////
void GexCmpQueryWizardPage1::dragEnterEvent(QDragEnterEvent *e)
{
    // Accept Drag if files list dragged over.
    if(e->mimeData()->formats().contains("text/uri-list"))
        e->acceptProposedAction();
}

///////////////////////////////////////////////////////////
// Dropping files in Widget (Drag & Drop)
///////////////////////////////////////////////////////////
void GexCmpQueryWizardPage1::dropEvent(QDropEvent *e)
{
    if (!e->mimeData()->formats().contains("text/uri-list"))
    {
        e->ignore();
        return;
    }

    QString		strFileName;
    QStringList strFileList;
    QList<QUrl> lstUrls = e->mimeData()->urls();

    for (int nUrl = 0; nUrl < lstUrls.count(); nUrl++)
    {
        strFileName = lstUrls.at(nUrl).toLocalFile();
        if (!strFileName.isEmpty())
            strFileList << strFileName;
    }

    if(strFileList.count() <= 0)
    {
        // Items dropped are not regular files...ignore.
        e->ignore();
        return;
    }

    // If only one file dragged over and it is a .CSL...then execute it to load the File List!
    if((strFileList.count() == 1) && (strFileList[0].endsWith(".csl",Qt::CaseInsensitive)))
    {
        ReadProcessFilesSection(strFileList[0]);
    }

    e->acceptProposedAction();
}

///////////////////////////////////////////////////////////
// Contextual menu
///////////////////////////////////////////////////////////
void GexCmpQueryWizardPage1::onContextualMenu(const QPoint& /*pos*/)
{
    int		iTotalSelection	= totalSelections();
    QMenu	menu(this);

    // Build menu.
    menu.addAction(*pixCreateFolder,"Create a new dataset (query)", this, SLOT(OnAddQuery()));
    menu.addSeparator();

    if(iTotalSelection >= 1)
    {
        menu.addAction(*pixProperties,"Properties: Edit the dataset definition (query parameters)", this, SLOT(OnProperties()));
        menu.addAction("Duplicate Query", this, SLOT(OnDuplicateQuery()));
        menu.addAction(*pixRemove,"Remove Query", this, SLOT(OnRemoveQuery()));

    }
    // If at least 2 files in the list...
    if(treeWidget->topLevelItemCount() >= 1)
    {
        menu.addSeparator();
        menu.addAction(*pixRemove,"Remove ALL queries from list...", this, SLOT(OnRemoveAll()));
    }

    menu.setMouseTracking(true);
    menu.exec(QCursor::pos());
    menu.setMouseTracking(false);
}

///////////////////////////////////////////////////////////
// Returns total number of lines (files) selected
///////////////////////////////////////////////////////////
int GexCmpQueryWizardPage1::totalSelections(void)
{
    QTreeWidgetItem * pTreeWidgetItem = treeWidget->topLevelItem(0);

    int	iTotal = 0;

    while(pTreeWidgetItem != NULL)
    {
        // Find first object selected
        if(pTreeWidgetItem->isSelected() == true)
            iTotal++;
        // Move to next item
        pTreeWidgetItem = treeWidget->itemBelow(pTreeWidgetItem);
    };

    // return toal of selections found
    return iTotal;
}

///////////////////////////////////////////////////////////
// Returns the filter Script strings for the associated filter selection
///////////////////////////////////////////////////////////
bool	GexCmpQueryWizardPage1::GetFilterSelection(QString &strFilter, QComboBox *pFilter, QComboBox *pFilterString)
{
    if(pFilter->currentIndex() == GEX_QUERY_FILTER_NONE)
        return false;	// Filter not active, return Empty string.

    //QString strFilterName, strArg;
    // Build filter line. e.g: 'tester ID=*'
    strFilter = pFilter->currentText();
    strFilter += "=";
    strFilter += pFilterString->currentText();
    //strFilter += ", ";
    strFilter += FILTERS_SEPARATOR;
    return true;
}

///////////////////////////////////////////////////////////
// Sets the given filter GUI to the specified filter string
///////////////////////////////////////////////////////////
void	GexCmpQueryWizardPage1::SetFilterSelection(QString &strFilter, QComboBox *pFilter,QComboBox *pFilterString)
{
    // If no filter specified, don't try to select anything
    if(strFilter.isEmpty())
        return;

    QString strFilterName = strFilter.section('=',0,0).trimmed();
    QString strFilterValue = strFilter.section('=',1).trimmed();

    // Set filter in GUI combo boxes
    bool b=SetCurrentComboItem(pFilter, strFilterName);
    if (!b)
        GSLOG(SYSLOG_SEV_WARNING, "SetCurrentComboItem failed");

    if(!SetCurrentComboItem(pFilterString, strFilterValue))
    {
        pFilterString->insertItem(pFilterString->count(),strFilterValue);
        SetCurrentComboItem(pFilterString, strFilterValue);
    }
}

///////////////////////////////////////////////////////////
// Read Query GUI dialog box fields into private structure
///////////////////////////////////////////////////////////
void GexCmpQueryWizardPage1::GetQueryFields(GexOneQueryWizardPage1 *pQueryDialog)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("GetQueryFields for query wizard page %1").arg( pQueryDialog->objectName()).toLatin1().constData() );
    if (!pQueryDialog)
        return;

    int	iSelection=0;

    // Read Form fields....
    strNewName		= pQueryDialog->lineEditReportTitle->text();
    strNewDatabase	= pQueryDialog->comboBoxDatabaseName->currentText();
    QDateTime dFrom, dTo;
    dFrom.setDate( pQueryDialog->FromDate->date() );
    dTo.setDate( pQueryDialog->ToDate->date() );
    dFrom.setTime( pQueryDialog->FromTimeEdit->time() );
    dTo.setTime( pQueryDialog->ToTimeEdit->time() );
    iSelection		= pQueryDialog->comboBoxTimePeriod->currentIndex();
    strNewPeriod	= gexLabelTimePeriodChoices[iSelection];

    if(iSelection == GEX_QUERY_TIMEPERIOD_CALENDAR)
    {
        strNewPeriod = dFrom.toString("dd MM yyyy hh mm");
        strNewPeriod += " - ";
        strNewPeriod += dTo.toString("dd MM yyyy hh mm");
    }
    else
        if (iSelection == GEX_QUERY_TIMEPERIOD_LAST_N_X)
        {
            strNewPeriod = "Last "+pQueryDialog->lineEditLastNXFactor->text()+" "+pQueryDialog->comboBoxLastNXStep->currentText();
        }

    // Get the 5 filters
    QString strFilter;
    bool	bStatus=false;

    strNewFilters="";	// Erase string content.
    bStatus = GetFilterSelection(strFilter,pQueryDialog->comboBoxFilter1,pQueryDialog->FilterString1);
    if(bStatus == true)
        strNewFilters += strFilter;

    bStatus = GetFilterSelection(strFilter,pQueryDialog->comboBoxFilter2,pQueryDialog->FilterString2);
    if(bStatus == true)
        strNewFilters += strFilter;

    bStatus = GetFilterSelection(strFilter,pQueryDialog->comboBoxFilter3,pQueryDialog->FilterString3);
    if(bStatus == true)
        strNewFilters += strFilter;

    bStatus = GetFilterSelection(strFilter,pQueryDialog->comboBoxFilter4,pQueryDialog->FilterString4);
    if(bStatus == true)
        strNewFilters += strFilter;

    bStatus = GetFilterSelection(strFilter,pQueryDialog->comboBoxFilter5,pQueryDialog->FilterString5);
    if(bStatus == true)
        strNewFilters += strFilter;

    bStatus = GetFilterSelection(strFilter,pQueryDialog->comboBoxFilter6,pQueryDialog->FilterString6);
    if(bStatus == true)
        strNewFilters += strFilter;

    bStatus = GetFilterSelection(strFilter,pQueryDialog->comboBoxFilter7,pQueryDialog->FilterString7);
    if(bStatus == true)
        strNewFilters += strFilter;

    bStatus = GetFilterSelection(strFilter,pQueryDialog->comboBoxFilter8,pQueryDialog->FilterString8);
    if(bStatus == true)
        strNewFilters += strFilter;

    // Get split filter
    strNewSplitField = pQueryDialog->lineEditSplitField->text();

    strNewMinimumPartsPerFile = pQueryDialog->lineEditMinimumPartPerFile->text();

    strNewParts    = gexLabelFileProcessPartsItems[pQueryDialog->DataTypeFilter->currentIndex()];	// Data type to filter GEX_PROCESSPART_ALL, etc..
    if(pQueryDialog->lineEditProcess->isHidden() == false)
        strNewParts += ": " + pQueryDialog->lineEditProcess->text();

    // Build string: Mapping file
    strNewMappingFile = pQueryDialog->LineEditMapTests->text();			// Mapping file.

    // Get Test list and Plug-in options string (only used by remoteDB queries)
    strNewTestList = pQueryDialog->lineEditParameterList->text();	// Test list to focus on (only applies to Remote DB)
    pQueryDialog->GetPluginOptionsString(strNewPluginOptionsString);

    // Get data type and Offline flag
    strNewDataType = pQueryDialog->comboBoxDatabaseType->currentText();
    bNewOffline = pQueryDialog->checkBoxOfflineQuery->isChecked();

    // Get extraction mode
    eNewExtractionMode = static_cast<GexOneQueryWizardPage1::eExtractionMode>(
                    pQueryDialog->comboBoxExtractionMode->itemData(
                    pQueryDialog->comboBoxExtractionMode->currentIndex()).toInt());

    switch(eNewExtractionMode)
    {
    case GexOneQueryWizardPage1::eRawData:
    case GexOneQueryWizardPage1::eConsolidatedData:
        break;
    case GexOneQueryWizardPage1::e1outOfNsamples:
        strExtractionModeParam = pQueryDialog->mExtractionModeLineEdit->text();
        break;
    default:
        GSLOG(SYSLOG_SEV_WARNING, QString("Unknown extraction mode : (%1) %2")
              .arg(pQueryDialog->comboBoxExtractionMode->currentIndex())
              .arg(pQueryDialog->comboBoxExtractionMode->currentText())
              .toLatin1().data());
    }
}

///////////////////////////////////////////////////////////
// Write private structure into Query GUI dialog box fields
///////////////////////////////////////////////////////////
void GexCmpQueryWizardPage1::SetQueryFields(GexOneQueryWizardPage1 *pQueryDialog)
{
    QString	strString;

    // Write Form fields....

    // Set Title, Database name, and Offline flag
    pQueryDialog->lineEditReportTitle->setText(strNewName);
    bool b=SetCurrentComboItem(pQueryDialog->comboBoxDatabaseName, strNewDatabase);
    if (!b)
        GSLOG(SYSLOG_SEV_ERROR, "SetCurrentComboItem failed");

    pQueryDialog->checkBoxOfflineQuery->setChecked(bNewOffline);

    // Force data type in relevant combo
    SetCurrentComboItem(pQueryDialog->comboBoxDatabaseType, strNewDataType, true);

    // Update database GUI (loads supported filters, data types...)
    pQueryDialog->OnDatabaseChanged();

    // Time period
    int i=0;
    while(gexLabelTimePeriodChoices[i] != 0)
    {
        if(strNewPeriod.startsWith(gexLabelTimePeriodChoices[i]))
            break;	// found matching string
        i++;
    };	// loop until we have found the string entry.

    if(strNewPeriod.startsWith("Last"))
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Set query field : %1").arg( strNewPeriod).toLatin1().constData());
        pQueryDialog->comboBoxTimePeriod->setCurrentIndex(pQueryDialog->comboBoxTimePeriod->findText("Last..."));
        QString N=strNewPeriod.section(" ", 1,1);
        QString step=strNewPeriod.section(" ", 2,2);
        pQueryDialog->lineEditLastNXFactor->show(); pQueryDialog->lineEditLastNXFactor->setText(N);
        pQueryDialog->comboBoxLastNXStep->show();
        pQueryDialog->comboBoxLastNXStep->setCurrentIndex(pQueryDialog->comboBoxLastNXStep->findText(step));
    }
    else
        if(gexLabelTimePeriodChoices[i])
        {
            // We have a predefined Time period
            pQueryDialog->comboBoxTimePeriod->setCurrentIndex(i);
            if (i == GEX_QUERY_TIMEPERIOD_LAST_N_X)
            {
                GSLOG(SYSLOG_SEV_ERROR, "GEX_QUERY_TIMEPERIOD_LAST_N_X not implemented in compare query.");
                GEX_ASSERT(false);
            }
        }
        else
        {
            pQueryDialog->comboBoxTimePeriod->setCurrentIndex(GEX_QUERY_TIMEPERIOD_CALENDAR);

            QString strFrom = strNewPeriod.section( '-', 0, 0 ).trimmed();
            QDateTime dFrom = QDateTime::fromString( strFrom, "dd MM yyyy hh mm" );

            pQueryDialog->FromDate->setDate( dFrom.date() );
            pQueryDialog->FromTimeEdit->setTime( dFrom.time() );

            QString strTo = strNewPeriod.section( '-', 1, 1 ).trimmed();
            QDateTime dTo = QDateTime::fromString( strTo, "dd MM yyyy hh mm" );

            pQueryDialog->ToDate->setDate( dTo.date() );
            pQueryDialog->ToTimeEdit->setTime( dTo.time() );
        }

    // Load Filters
    strString = strNewFilters.section(FILTERS_SEPARATOR,0,0);	// Filter1 (if any)
    SetFilterSelection(strString, pQueryDialog->comboBoxFilter1,pQueryDialog->FilterString1);
    strString = strNewFilters.section(FILTERS_SEPARATOR,1,1);	// Filter2 (if any)
    SetFilterSelection(strString, pQueryDialog->comboBoxFilter2,pQueryDialog->FilterString2);
    strString = strNewFilters.section(FILTERS_SEPARATOR,2,2);	// Filter3 (if any)
    SetFilterSelection(strString, pQueryDialog->comboBoxFilter3,pQueryDialog->FilterString3);
    strString = strNewFilters.section(FILTERS_SEPARATOR,3,3);	// Filter4 (if any)
    SetFilterSelection(strString, pQueryDialog->comboBoxFilter4,pQueryDialog->FilterString4);
    strString = strNewFilters.section(FILTERS_SEPARATOR,4,4);	// Filter5 (if any)
    SetFilterSelection(strString, pQueryDialog->comboBoxFilter5,pQueryDialog->FilterString5);
    strString = strNewFilters.section(FILTERS_SEPARATOR,5,5);	// Filter6 (if any)
    SetFilterSelection(strString, pQueryDialog->comboBoxFilter6,pQueryDialog->FilterString6);
    strString = strNewFilters.section(FILTERS_SEPARATOR,6,6);	// Filter7 (if any)
    SetFilterSelection(strString, pQueryDialog->comboBoxFilter7,pQueryDialog->FilterString7);
    strString = strNewFilters.section(FILTERS_SEPARATOR,7,7);	// Filter8 (if any)
    SetFilterSelection(strString, pQueryDialog->comboBoxFilter8,pQueryDialog->FilterString8);

    // Set split filter
    pQueryDialog->lineEditSplitField->setText(strNewSplitField);

    strString = strNewParts.section(':',0,0);
    i=0;
    while(gexLabelFileProcessPartsItems[i] != 0)
    {
        if( strString == gexLabelFileProcessPartsItems[i])
            break;	// found matching string
        i++;
    };	// loop until we have found the string entry.
    pQueryDialog->DataTypeFilter->setCurrentIndex(i);
    strString = strNewParts.section(':',1);
    pQueryDialog->lineEditProcess->setText(strString);	// May be empty!

    pQueryDialog->lineEditMinimumPartPerFile->setText(strNewMinimumPartsPerFile);

    // Mapping file
    pQueryDialog->LineEditMapTests->setText(strNewMappingFile);

    pQueryDialog->lineEditParameterList->setText(strNewTestList);		// Test list to focus on (only applies to Remote DB)
    pQueryDialog->SetPluginOptionsString(strNewPluginOptionsString);	// Plugin custom options (only applies to Remote DB)


    // Set extraction mode
    pQueryDialog->UpdateExtractionModeUi(eNewExtractionMode);

    if (eNewExtractionMode == GexOneQueryWizardPage1::e1outOfNsamples)
        pQueryDialog->mExtractionModeLineEdit->setText(strExtractionModeParam);
    }

///////////////////////////////////////////////////////////
// Add a Query to the list of Queries to compare
///////////////////////////////////////////////////////////
void GexCmpQueryWizardPage1::OnAddQuery(void)
{
    GexOneQueryWizardPage1 cQueryDialog;
    cQueryDialog.setProhibatedList(getGroupNameList(true));
    if (pGexMainWindow->iWizardPage == GEX_MIXQUERY_WIZARD_P1)
    {
        cQueryDialog.lineEditSplitField->hide();
        cQueryDialog.TextLabelSplitField->hide();
    }
    else
    {
        cQueryDialog.lineEditSplitField->show();
        cQueryDialog.TextLabelSplitField->show();
    }

    pGexMainWindow->pWizardAdminGui->ListDatabasesInCombo(cQueryDialog.comboBoxDatabaseName, DB_SELECT_FOR_EXTRACTION);

    // Call member to resize & hide/show relevant fields when displayed as a PopUp dialog box.
    cQueryDialog.PopupSkin(GexOneQueryWizardPage1::eCreate,true);

    // Display dialog box.
    if(cQueryDialog.exec() != 1)
        return;	// User 'Abort'

    // Read Query fields
    GetQueryFields(&cQueryDialog);

    OnAddQuery(strNewName,strNewDatabase,strNewPeriod,strNewFilters,strNewMinimumPartsPerFile,strNewParts,strNewMappingFile,
               strNewTestList,strNewDataType,bNewOffline,strNewSplitField,strNewPluginOptionsString,
               eNewExtractionMode, strExtractionModeParam);
}

void GexCmpQueryWizardPage1::OnAddQuery(QString &strNewName,QString &strNewDatabase,QString &strNewPeriod,QString &strNewFilters,
                                        QString &strNewMinimumPartsPerFile,QString &strNewParts,QString &strNewMappingFile,
                                        QString &strNewTestList,QString &strNewDataType,bool bNewOffline,
                                        QString &strNewSplitField,QString &strNewPluginOptionsString,
                                        GexOneQueryWizardPage1::eExtractionMode eNewExtractionMode,
                                        const QString &ExtractionModeParameter)
{
    // Add database entry to te list.
    QTreeWidgetItem * pTreeWidgetItem = new QTreeWidgetItem(treeWidget);
    pTreeWidgetItem->setText(0, strNewName);
    pTreeWidgetItem->setText(1, strNewDatabase);
    pTreeWidgetItem->setText(2, strNewPeriod);
    pTreeWidgetItem->setText(3, strNewFilters);
    pTreeWidgetItem->setText(4, strNewMinimumPartsPerFile);
    pTreeWidgetItem->setText(5, strNewParts);
    pTreeWidgetItem->setText(6, strNewMappingFile);
    pTreeWidgetItem->setText(7, strNewTestList);
    pTreeWidgetItem->setText(8, strNewDataType);

    if(bNewOffline)
        pTreeWidgetItem->setText(9, "1");
    else
        pTreeWidgetItem->setText(9, "0");

    pTreeWidgetItem->setText(10, strNewSplitField);
    pTreeWidgetItem->setText(11, strNewPluginOptionsString);
    pTreeWidgetItem->setText(12, QString::number(eNewExtractionMode));
    pTreeWidgetItem->setText(13, ExtractionModeParameter );

    // Enable 'Next' button
    buttonSettings->setEnabled(true);

    // Reset HTML sections to create flag: ALL pages to create.
    pGexMainWindow->iHtmlSectionsToSkip = 0;
    pGexMainWindow->m_bDatasetChanged	= true;

    // Query(ies) selected, enable the 'Next',Properties button
    buttonSettings->setEnabled(true);
    buttonProperties->show();
    Line1->show();
    buttonDuplicateTask->show();
    buttonRemoveQuery->show();
    PushButtonClearAll->show();
    buttonSaveDatasetList->show();

    if(bSortingCapability && treeWidget->topLevelItemCount() >= 2)
    {
        Line2->show();
        buttonUp->show();
        buttonDown->show();
    }

    strNewSplitField.clear();
}

///////////////////////////////////////////////////////////
// Edit selected Query
///////////////////////////////////////////////////////////
void GexCmpQueryWizardPage1::OnProperties(void)
{
    // If no item in list, just return!
    if(treeWidget->topLevelItemCount() <= 0)
        return;

    // Get handle to selection...
    QTreeWidgetItem * pTreeWidgetItem = treeWidget->currentItem();
    if(pTreeWidgetItem == NULL)
        return;	// No selection!

    // Load fields with values of the query selected...
    strNewName					= pTreeWidgetItem->text(0);
    strNewDatabase				= pTreeWidgetItem->text(1);
    strNewPeriod				= pTreeWidgetItem->text(2);
    strNewFilters				= pTreeWidgetItem->text(3);
    strNewMinimumPartsPerFile	= pTreeWidgetItem->text(4);
    strNewParts					= pTreeWidgetItem->text(5);
    strNewMappingFile			= pTreeWidgetItem->text(6);
    strNewTestList				= pTreeWidgetItem->text(7);
    strNewDataType				= pTreeWidgetItem->text(8);
    if(pTreeWidgetItem->text(9) == "1")
        bNewOffline = true;
    else
        bNewOffline = false;

    strNewSplitField			= pTreeWidgetItem->text(10);
    strNewPluginOptionsString	= pTreeWidgetItem->text(11);

    if(pTreeWidgetItem->text(12) == "0")
        eNewExtractionMode = GexOneQueryWizardPage1::eRawData;
    else if(pTreeWidgetItem->text(12) == "1")
        eNewExtractionMode = GexOneQueryWizardPage1::eConsolidatedData;
    else if(pTreeWidgetItem->text(12) == "2")
    {
        eNewExtractionMode = GexOneQueryWizardPage1::e1outOfNsamples;
        strExtractionModeParam =  pTreeWidgetItem->text(13);
    }
    else
        GSLOG(SYSLOG_SEV_WARNING, QString("Unknonw Extraction mode %1").arg( pTreeWidgetItem->text(12)).toLatin1().constData());

    // Read Query fields
    GexOneQueryWizardPage1 cQueryDialog;
    cQueryDialog.setProhibatedList(getGroupNameList(false));

    if (pGexMainWindow->iWizardPage == GEX_MIXQUERY_WIZARD_P1)
    {
        cQueryDialog.lineEditSplitField->hide();
        cQueryDialog.TextLabelSplitField->hide();
    }
    else
    {
        cQueryDialog.lineEditSplitField->show();
        cQueryDialog.TextLabelSplitField->show();
    }

    pGexMainWindow->pWizardAdminGui->ListDatabasesInCombo(cQueryDialog.comboBoxDatabaseName, DB_SELECT_FOR_EXTRACTION);

    // Update GUI to reflect status.
    cQueryDialog.PopupSkin(GexOneQueryWizardPage1::eEdit,true);

    // Finalize initialization of different fields.
    SetQueryFields(&cQueryDialog);

    // Update GUI controls depending on current selections
    cQueryDialog.UpdateGui();

    // Display dialog box.
    if(cQueryDialog.exec() != 1){
        return;	// User 'Abort'
    }

    // Readback fields to update the Selection...
    GetQueryFields(&cQueryDialog);

    pTreeWidgetItem->setText(0, strNewName);
    pTreeWidgetItem->setText(1, strNewDatabase);
    pTreeWidgetItem->setText(2, strNewPeriod);
    pTreeWidgetItem->setText(3, strNewFilters);
    pTreeWidgetItem->setText(4, strNewMinimumPartsPerFile);
    pTreeWidgetItem->setText(5, strNewParts);
    pTreeWidgetItem->setText(6, strNewMappingFile);
    pTreeWidgetItem->setText(7, strNewTestList);
    pTreeWidgetItem->setText(8, strNewDataType);
    if(bNewOffline)
        pTreeWidgetItem->setText(9, "1");
    else
        pTreeWidgetItem->setText(9, "0");

    pTreeWidgetItem->setText(10, strNewSplitField);
    pTreeWidgetItem->setText(11, strNewPluginOptionsString);
    pTreeWidgetItem->setText(12, QString::number(eNewExtractionMode));
    pTreeWidgetItem->setText(13, strExtractionModeParam );

    // Reset HTML sections to create flag: ALL pages to create.
    pGexMainWindow->iHtmlSectionsToSkip = 0;
    pGexMainWindow->m_bDatasetChanged	= true;
}

///////////////////////////////////////////////////////////
// Reload ListView with list of files from disk
///////////////////////////////////////////////////////////
void GexCmpQueryWizardPage1::OnLoadDatasetList(void)
{
    // Let's user pick the File List to load
    strDatasetListFile = QFileDialog::getOpenFileName(this, "Load a File List...", strDatasetListFile, "File List / Script (*.csl)");

    // If no file selected, ignore command.
    if(strDatasetListFile.isEmpty())
        return;

    // Read section & load it in the list...
    ReadProcessFilesSection(strDatasetListFile);
}

///////////////////////////////////////////////////////////
// Save ListView into a ListFile on disk
///////////////////////////////////////////////////////////
void GexCmpQueryWizardPage1::OnSaveDatasetList(void)
{
    // Let's user tell where to save the File List.
    strDatasetListFile = QFileDialog::getSaveFileName(this,
                                                      "Save File List to...",
                                                      strDatasetListFile,
                                                      "File List / Script (*.csl)",
                                                      NULL);

    // If no file selected, ignore command.
    if(strDatasetListFile.isEmpty())
        return;

    // Make sure file name ends with ".csl" extension
    if(strDatasetListFile.endsWith(".csl",Qt::CaseInsensitive) == false)
        strDatasetListFile += ".csl";

    FILE *hFile;
    hFile = fopen(strDatasetListFile.toLatin1().constData(),"w");
    if(hFile == NULL)
    {
        GS::Gex::Message::
            critical("", "Failed creating file...folder is write protected?");
        return;
    }

    fprintf(hFile,"<gex_template>\n");

    if (pGexMainWindow->iWizardPage == GEX_CMPQUERY_WIZARD_P1)
        fprintf(hFile,"<BlockType = compare_queries>\n\n");
    else
        if (pGexMainWindow->iWizardPage == GEX_MIXQUERY_WIZARD_P1)
            fprintf(hFile,"<BlockType = merge_queries>\n\n");
        else
        {
            GSLOG(SYSLOG_SEV_ERROR, "OnSaveDatasetList : Cant find type of task to perform (compare or merge ?)");
            fclose(hFile);
            GS::Gex::Message::critical(
                "",
                "Failed writing file : Cant find type of task"
                " to perform (compare or merge ?)");
            return;
        }
    WriteProcessFilesSection(hFile, false);
    fprintf(hFile,"\n</gex_template>\n");
    fclose(hFile);
}

///////////////////////////////////////////////////////////
// Remove selected query
///////////////////////////////////////////////////////////
void GexCmpQueryWizardPage1::OnRemoveQuery(void)
{
    QString strMessage;

    // If no item in list, just return!
    if(treeWidget->topLevelItemCount() <= 0)
        return;

    // Remove selected item (no multi-selection allowed!)
    QTreeWidgetItem* pTreeWidgetItem = treeWidget->currentItem();
    if(pTreeWidgetItem == NULL)
        return;	// No selection!

    QString	strQueryName = pTreeWidgetItem->text(0);

    // Ask for confirmation...
    strMessage = "Confirm to remove query:\n" + strQueryName;
    bool lOk;
    GS::Gex::Message::request("Remove Query", strMessage, lOk);
    if (! lOk)
    {
        return;
    }

    // Remove selected item from list
    treeWidget->takeTopLevelItem(treeWidget->indexOfTopLevelItem(pTreeWidgetItem));
    delete pTreeWidgetItem; pTreeWidgetItem = NULL;

    // If least one item, enable 'Next' button
    if(treeWidget->topLevelItemCount() < 1)
    {
        buttonSettings->setEnabled(false);
        buttonProperties->hide();
        Line1->hide();
        buttonDuplicateTask->hide();
        buttonRemoveQuery->hide();
        PushButtonClearAll->hide();
        Line2->hide();
        buttonUp->hide();
        buttonDown->hide();
        buttonSaveDatasetList->hide();
    }
    else
        buttonSettings->setEnabled(true);

    if(treeWidget->topLevelItemCount() <= 1)
    {
        Line2->hide();
        buttonUp->hide();
        buttonDown->hide();
    }

    // Reset HTML sections to create flag: ALL pages to create.
    pGexMainWindow->iHtmlSectionsToSkip = 0;
    pGexMainWindow->m_bDatasetChanged	= true;
    if(m_oGroupConditions.contains(strQueryName))
        m_oGroupConditions.remove(strQueryName);
}

///////////////////////////////////////////////////////////
// Remove all queries
///////////////////////////////////////////////////////////
void GexCmpQueryWizardPage1::OnRemoveAll(void)
{
    // If no item in list, just return!
    if(treeWidget->topLevelItemCount() <= 0)
        return;

    // Ask for confirmation...
    bool lOk;
    GS::Gex::Message::request("Remove All Queries", "Confirm to remove ALL queries ?", lOk);
    if (! lOk)
    {
        return;
    }

    // Remove selected item from list
    clearQueries();

    // Disable 'Next' button
    buttonSettings->setEnabled(false);
    buttonProperties->hide();
    Line1->hide();
    buttonDuplicateTask->hide();
    buttonRemoveQuery->hide();
    PushButtonClearAll->hide();
    Line2->hide();
    buttonUp->hide();
    buttonDown->hide();
    buttonSaveDatasetList->hide();

    // Reset HTML sections to create flag: ALL pages to create.
    pGexMainWindow->iHtmlSectionsToSkip = 0;
    pGexMainWindow->m_bDatasetChanged	= true;
    m_poCompareQueryCondition->clear();
}

///////////////////////////////////////////////////////////
// Duplicate selected queries
///////////////////////////////////////////////////////////
void GexCmpQueryWizardPage1::OnDuplicateQuery(void)
{
    // If no item in list, just return!
    if(treeWidget->topLevelItemCount() <= 0)
        return;

    QTreeWidgetItem * pTreeWidgetItem		= treeWidget->topLevelItem(0);
    QTreeWidgetItem * pTreeWidgetCopyItem	= NULL;

    while(pTreeWidgetItem != NULL)
    {
        // Duplicate objects selected
        if(pTreeWidgetItem->isSelected() == true)
        {
            QString strTitle		= "Copy of ";
            strTitle += pTreeWidgetItem->text(0);
            QString strDatabaseName = pTreeWidgetItem->text(1);
            QString strPeriod		= pTreeWidgetItem->text(2);
            QString strFilters		= pTreeWidgetItem->text(3);
            QString strMode			= pTreeWidgetItem->text(4);
            QString strDataRecords	= pTreeWidgetItem->text(5);
            QString strMapFile		= pTreeWidgetItem->text(6);
            QString strTestList		= pTreeWidgetItem->text(7);

            // Add Query entry to the list.
            pTreeWidgetCopyItem = new QTreeWidgetItem(treeWidget);
            pTreeWidgetCopyItem->setText(0, strTitle);
            pTreeWidgetCopyItem->setText(1, strDatabaseName);
            pTreeWidgetCopyItem->setText(2, strPeriod);
            pTreeWidgetCopyItem->setText(3, strFilters);
            pTreeWidgetCopyItem->setText(4, strMode);
            pTreeWidgetCopyItem->setText(5, strDataRecords);
            pTreeWidgetCopyItem->setText(6, strMapFile);
            pTreeWidgetCopyItem->setText(7, strTestList);
            pTreeWidgetCopyItem->setText(8, pTreeWidgetItem->text(8));
            pTreeWidgetCopyItem->setText(9, pTreeWidgetItem->text(9));
            pTreeWidgetCopyItem->setText(10, pTreeWidgetItem->text(10));
            pTreeWidgetCopyItem->setText(11, pTreeWidgetItem->text(11));
            pTreeWidgetCopyItem->setText(12, pTreeWidgetItem->text(12));
            pTreeWidgetCopyItem->setText(13, pTreeWidgetItem->text(13));
        }

        // Move to next item
        pTreeWidgetItem = treeWidget->itemBelow(pTreeWidgetItem);
    };
}

///////////////////////////////////////////////////////////
// Move selected query UP
///////////////////////////////////////////////////////////
void GexCmpQueryWizardPage1::OnMoveQueryUp(void)
{
    // If no item in list, just return!
    if(treeWidget->topLevelItemCount() <= 1)
        return;

    // Reset HTML sections to create flag: ALL pages to create.
    pGexMainWindow->iHtmlSectionsToSkip = 0;
    pGexMainWindow->m_bDatasetChanged	= true;

    // Scan the list to move UP the selected item.
    QTreeWidgetItem *	pTreeWidgetItem		= treeWidget->topLevelItem(0);
    QTreeWidgetItem *	pTreeWidgetPrevItem	= NULL;
    int					nCurrentItem		= 0;

    while(pTreeWidgetItem != NULL)
    {
        // Find first object selected
        if(pTreeWidgetItem->isSelected() == true)
        {
            pTreeWidgetPrevItem = treeWidget->itemAbove(pTreeWidgetItem);

            // if 1st file selected...can't move up..ignore!
            if(pTreeWidgetPrevItem != NULL)
            {
                // Get position of the current item
                nCurrentItem = treeWidget->indexOfTopLevelItem(pTreeWidgetPrevItem);
                // Remove current group from the treewidget and insert it at its new position
                treeWidget->takeTopLevelItem(treeWidget->indexOfTopLevelItem(pTreeWidgetItem));
                treeWidget->insertTopLevelItem(nCurrentItem, pTreeWidgetItem);
                // Keeps the current item selected
                pTreeWidgetItem->setSelected(true);
            }

            return;
        }

        // Move to next item
        pTreeWidgetItem = treeWidget->itemBelow(pTreeWidgetItem);
    };
}

///////////////////////////////////////////////////////////
// Move selected query DOWN
///////////////////////////////////////////////////////////
void GexCmpQueryWizardPage1::OnMoveQueryDown(void)
{
    // If no item in list, just return!
    if(treeWidget->topLevelItemCount() <= 1)
        return;

    // Reset HTML sections to create flag: ALL pages to create.
    pGexMainWindow->iHtmlSectionsToSkip = 0;
    pGexMainWindow->m_bDatasetChanged	= true;

    // Scan the list to move DOWN the selected item.
    QTreeWidgetItem *	pTreeWidgetItem		= treeWidget->topLevelItem(0);
    QTreeWidgetItem *	pTreeWidgetNextItem	= NULL;
    int					nCurrentItem		= 0;

    while(pTreeWidgetItem != NULL)
    {
        // Find first object selected
        if(pTreeWidgetItem->isSelected() == true)
        {
            // Keep track of next item before removing it !
            pTreeWidgetNextItem = treeWidget->itemBelow(pTreeWidgetItem);

            // Move selected item after following file in the list
            if (pTreeWidgetNextItem)
            {
                // Get position of the current item
                nCurrentItem = treeWidget->indexOfTopLevelItem(pTreeWidgetItem);
                // Remove current group from the treewidget and insert it at its new position
                treeWidget->takeTopLevelItem(treeWidget->indexOfTopLevelItem(pTreeWidgetNextItem));
                treeWidget->insertTopLevelItem(nCurrentItem, pTreeWidgetNextItem);
            }

            // only move one selection at a time
            return;
        }

        // Move to next item
        pTreeWidgetItem = treeWidget->itemBelow(pTreeWidgetItem);
    };
}

///////////////////////////////////////////////////////////
// Returns the Script string for the associated filter selection
///////////////////////////////////////////////////////////
void GexCmpQueryWizardPage1::WriteFilterScriptLine(GexDatabaseEntry	*pDatabaseEntry,FILE *hFile,QComboBox *pFilter,QComboBox *pFilterString,bool bOfflineQuery, bool bIgnoreStarFilters)
{
    QString strFilterName	= pFilter->currentText();

    QString	strFilterValue = pFilterString->currentText();

    // Check if valid filter
    if(strFilterName.isEmpty() || (strFilterName == gexLabelFilterChoices[GEX_QUERY_FILTER_NONE]))
        return;		// Filter not active

    ConvertToScriptString(strFilterValue);
    // BG 23 Aug 07:
    // If called from function used to save dataset list (OnSaveDatasetList()), keep any filter (even '*') as long as the filter name
    // is correct, so to keep exact same filters (and filter order) when reloading the script.
    if(bIgnoreStarFilters && (strFilterValue == "*"))
        return;		// Ignore '*' filter as it has no effect!

    // Get script argument to return. e.g: 'tester_id'
    // Script command depends on whether remote SQL DB or not
    if(pDatabaseEntry->IsExternal() && !bOfflineQuery)
    {
        // Query on remote SQL DB
        fprintf(hFile,"  gexQuery('dbf_sql','%s','%s');\n",
                strFilterName.toLatin1().constData(),
                strFilterValue.toLatin1().constData());
    }
    else
    {
        // Query on local file-based DB
        strFilterName = gexFilterChoices[pFilter->currentIndex()];
        fprintf(hFile,"  gexQuery('%s','%s');\n",strFilterName.toLatin1().constData(),strFilterValue.toLatin1().constData());
    }
}

///////////////////////////////////////////////////////////
// Returns the Script string for the associated split field
///////////////////////////////////////////////////////////
void GexCmpQueryWizardPage1::WriteSplitFieldScriptLine(GexDatabaseEntry	* pDatabaseEntry,
                                                       FILE *hFile,
                                                       const QString &splitFields,
                                                       bool bOfflineQuery)
{
    // Check if valid is active
    if(splitFields.isEmpty() == false)
    {
        QStringList fieldList = splitFields.split("|");

        foreach(const QString &fieldName, fieldList)
        {
            // Get script argument to return. e.g: 'tester_id'
            // Script command depends on whether remote SQL DB or not
            if(pDatabaseEntry->IsExternal() && !bOfflineQuery)
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
// Script function to define the Queries
///////////////////////////////////////////////////////////
void GexCmpQueryWizardPage1::WriteProcessFilesSection(FILE *hFile, bool bIgnoreStarFilters/* = true*/)
{
    QString					strTitle;
    QString					strDatabaseName,strLocation;
    QDateTime				dFrom,dTo;
    QString					strParts,strRange,strMapFile;
    GexDatabaseEntry		*pDatabaseEntry=NULL;

    // PYC, 30/05/2011
    int nWizardType = pGexMainWindow->GetWizardType();

    // Write ALL the queries to compare
    QTreeWidgetItem * pTreeWidgetItem = treeWidget->topLevelItem(0);

    // Write script sequence
    //if(pGexMainWindow->iWizardType == GEX_MIXQUERY_WIZARD)
    if(nWizardType == GEX_MIXQUERY_WIZARD)
    {
        fprintf(hFile,"  // Merge queries...\n");
    }
    else
    {
        fprintf(hFile,"  // Compare queries...\n");
    }

    GexOneQueryWizardPage1	cQueryDialog;
    pGexMainWindow->pWizardAdminGui->ListDatabasesInCombo(cQueryDialog.comboBoxDatabaseName, DB_SELECT_FOR_EXTRACTION);
    pTreeWidgetItem = treeWidget->topLevelItem(0);
    while(pTreeWidgetItem != NULL)
    {
        SetQueryFields(&cQueryDialog);
        cQueryDialog.UpdateGui();
        if( pGexMainWindow->forceToComputeFromDataSample(hFile, cQueryDialog.DataTypeFilter->currentIndex()))
            break;
        pTreeWidgetItem = treeWidget->itemBelow(pTreeWidgetItem);
    }

    fprintf(hFile,"  gexGroup('reset','all');\n");
    strTitle = lineEditReportTitle->text();			// Query Title
    ConvertToScriptString(strTitle);				// to be script compatible '\' become '\\'
    fprintf(hFile,"  gexQuery('db_report','%s');\n",strTitle.toLatin1().constData()); // Report fodler name or CSV file

    if(!m_poCompareQueryCondition->text().isEmpty()){
        QStringList oOrdredCondition = m_poCompareQueryCondition->text().split(QRegExp("[;,]"),QString::SkipEmptyParts);
        foreach(const QString &strCondition, oOrdredCondition){
            fprintf(hFile,"  gexGroup('declare_condition','%s');\n",strCondition.toLatin1().constData());
        }
    }

    // If Enterprise report (SQL) to be created, then there are no file(s) dataset to analyze!
    if(pGexMainWindow->pWizardSettings->IsEnterpriseSqlReport())
        return;

    // If we are in 'Merge dataset' mode, create only 1 group, and add the different datset queries to this group
    //if(pGexMainWindow->iWizardType == GEX_MIXQUERY_WIZARD)
    if(nWizardType == GEX_MIXQUERY_WIZARD)
    {
        fprintf(hFile,"\n  // Create group...");
        fprintf(hFile,"\n  group_id = gexGroup('insert','%s');\n", lineEditReportTitle->text().toLatin1().constData());
    }

    pTreeWidgetItem = treeWidget->topLevelItem(0);
    // Write each Query.

    pGexMainWindow->pWizardAdminGui->ListDatabasesInCombo(cQueryDialog.comboBoxDatabaseName,DB_SELECT_FOR_EXTRACTION);
    pTreeWidgetItem = treeWidget->topLevelItem(0);
    while(pTreeWidgetItem != NULL)
    {
        // Load fields with values of the query selected...
        strNewName					= pTreeWidgetItem->text(0).trimmed();
        strNewDatabase				= pTreeWidgetItem->text(1).trimmed();
        strNewPeriod				= pTreeWidgetItem->text(2).trimmed();
        strNewFilters				= pTreeWidgetItem->text(3).trimmed();
        strNewMinimumPartsPerFile	= pTreeWidgetItem->text(4).trimmed();
        strNewParts					= pTreeWidgetItem->text(5).trimmed();
        strNewMappingFile			= pTreeWidgetItem->text(6).trimmed();
        strNewTestList				= pTreeWidgetItem->text(7).trimmed();
        strNewDataType				= pTreeWidgetItem->text(8).trimmed();
        if(pTreeWidgetItem->text(9) == "1")
            bNewOffline = true;
        else
            bNewOffline = false;
        strNewSplitField			= pTreeWidgetItem->text(10).trimmed();
        strNewPluginOptionsString	= pTreeWidgetItem->text(11).trimmed();
        if(pTreeWidgetItem->text(12) == "0")
            eNewExtractionMode = GexOneQueryWizardPage1::eRawData;
        else if(pTreeWidgetItem->text(12) == "1")
            eNewExtractionMode = GexOneQueryWizardPage1::eConsolidatedData;
        else if(pTreeWidgetItem->text(12) == "2")
            eNewExtractionMode = GexOneQueryWizardPage1::e1outOfNsamples;

        strExtractionModeParam = pTreeWidgetItem->text(13);

        // Get ptr to database entry
        pDatabaseEntry	= NULL;
        strLocation		= strDatabaseName = strNewDatabase;
        strLocation		= "[Local]";
        if(strDatabaseName.startsWith("[Local]") || strDatabaseName.startsWith("[Server]"))
            strDatabaseName = strDatabaseName.section("]",1).trimmed();

        pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(strDatabaseName);

        // Do nothing if database not found
        if(!pDatabaseEntry)
        {
            GSLOG(SYSLOG_SEV_ERROR, QString("In query %1: unable to find database %2!")
                  .arg(strNewName)
                  .arg(strDatabaseName)
                  .toLatin1().constData());
            pTreeWidgetItem = treeWidget->itemBelow(pTreeWidgetItem);
            continue;
        }

        if(!pDatabaseEntry->IsStoredInFolderLocal())
            strLocation	= "[Server]";

        // Read Query fields
        SetQueryFields(&cQueryDialog);
        // Update GUI controls depending on current selections
        cQueryDialog.UpdateGui();

        // Get Query fields to write in script file.
        strTitle	 = strNewName;			// Query Title
        ConvertToScriptString(strTitle);				// to be script compatible '\' become '\\'
        ConvertToScriptString(strDatabaseName);			// to be script compatible '\' become '\\'
        dFrom.setDate( cQueryDialog.FromDate->date() );
        dTo.setDate( cQueryDialog.ToDate->date() );
        dFrom.setTime( cQueryDialog.FromTimeEdit->time() );
        dTo.setTime( cQueryDialog.ToTimeEdit->time() );
        strParts    = gexFileProcessPartsItems[cQueryDialog.DataTypeFilter->currentIndex()];	// Data type to filter GEX_PROCESSPART_ALL, etc..

        strRange	= cQueryDialog.lineEditProcess->text();			// range list.
        strMapFile  = cQueryDialog.LineEditMapTests->text();			// Mapping file.
        ConvertToScriptString(strMapFile);

        // Database location: [Local] or [Server]
        fprintf(hFile,"\n  gexQuery('db_location','%s');\n",strLocation.toLatin1().constData());
        // Database name
        fprintf(hFile,"  gexQuery('db_name','%s');\n",strDatabaseName.toLatin1().constData());

        // Specify time period
        if (cQueryDialog.comboBoxTimePeriod->currentIndex()!=GEX_QUERY_TIMEPERIOD_LAST_N_X)
            fprintf(hFile,"  gexQuery('db_period','%s','%d %d %d %d %d %d %d %d %d %d');\n",
                    gexTimePeriodChoices[cQueryDialog.comboBoxTimePeriod->currentIndex()],
                    dFrom.date().year(), dFrom.date().month(), dFrom.date().day(),
                    dFrom.time().hour(), dFrom.time().minute(),
                    dTo.date().year(), dTo.date().month(), dTo.date().day(),
                    dTo.time().hour(), dTo.time().minute() );
        else
        {
            fprintf(hFile,"  gexQuery('db_period','%s','%s %s');\n",
                    gexTimePeriodChoices[cQueryDialog.comboBoxTimePeriod->currentIndex()],
                    cQueryDialog.lineEditLastNXFactor->text().toLatin1().data(),
                    cQueryDialog.comboBoxLastNXStep->currentText().toLatin1().data()
                    );
        }

        // Save filters (if activated)
        WriteFilterScriptLine(pDatabaseEntry,hFile,cQueryDialog.comboBoxFilter1,cQueryDialog.FilterString1,
                              cQueryDialog.checkBoxOfflineQuery->isChecked(),bIgnoreStarFilters);
        WriteFilterScriptLine(pDatabaseEntry,hFile,cQueryDialog.comboBoxFilter2,cQueryDialog.FilterString2,
                              cQueryDialog.checkBoxOfflineQuery->isChecked(),bIgnoreStarFilters);
        WriteFilterScriptLine(pDatabaseEntry,hFile,cQueryDialog.comboBoxFilter3,cQueryDialog.FilterString3,
                              cQueryDialog.checkBoxOfflineQuery->isChecked(),bIgnoreStarFilters);
        WriteFilterScriptLine(pDatabaseEntry,hFile,cQueryDialog.comboBoxFilter4,cQueryDialog.FilterString4,
                              cQueryDialog.checkBoxOfflineQuery->isChecked(),bIgnoreStarFilters);
        WriteFilterScriptLine(pDatabaseEntry,hFile,cQueryDialog.comboBoxFilter5,cQueryDialog.FilterString5,
                              cQueryDialog.checkBoxOfflineQuery->isChecked(),bIgnoreStarFilters);
        WriteFilterScriptLine(pDatabaseEntry,hFile,cQueryDialog.comboBoxFilter6,cQueryDialog.FilterString6,
                              cQueryDialog.checkBoxOfflineQuery->isChecked(),bIgnoreStarFilters);
        WriteFilterScriptLine(pDatabaseEntry,hFile,cQueryDialog.comboBoxFilter7,cQueryDialog.FilterString7,
                              cQueryDialog.checkBoxOfflineQuery->isChecked(),bIgnoreStarFilters);
        WriteFilterScriptLine(pDatabaseEntry,hFile,cQueryDialog.comboBoxFilter8,cQueryDialog.FilterString8,
                              cQueryDialog.checkBoxOfflineQuery->isChecked(),bIgnoreStarFilters);

        // Save split field (if activated)
        WriteSplitFieldScriptLine(pDatabaseEntry,hFile,cQueryDialog.lineEditSplitField->text(),cQueryDialog.checkBoxOfflineQuery->isChecked());

        // Minimum parts per file
        fprintf(hFile,"  gexQuery('db_minimum_samples','%s');\n",strNewMinimumPartsPerFile.toLatin1().constData());

        // Parts type to process (all, good only, fail, etc.)
        strRange.replace(",",";");	// Ensure comma is not found in range string!
        fprintf(hFile,"  gexQuery('db_data','%s','%s');\n",strParts.toLatin1().constData(),strRange.toLatin1().constData());

        // Mapping file
        fprintf(hFile,"  gexQuery('db_mapping','%s');\n",strMapFile.toLatin1().constData());

        // Remote Database Parameter list (ignored if query not done over a remoteDB)
        strNewTestList.replace( ",", "\\x2c" );
        fprintf(hFile,"  gexQuery('db_testlist','%s');\n",strNewTestList.toLatin1().constData());

        // Remote Database option string (ignored if query not done over a remoteDB, or no options supported)
        if(!strNewPluginOptionsString.isEmpty())
            fprintf(hFile,"  gexQuery('db_plugin_options','%s');\n",strNewPluginOptionsString.toLatin1().constData());

        // Extraction mode
        // 6050
        fprintf(hFile,"  gexQuery('db_consolidated','%s');\n", (eNewExtractionMode == GexOneQueryWizardPage1::eConsolidatedData)?"true":"false" );
        if (eNewExtractionMode == GexOneQueryWizardPage1::e1outOfNsamples)
            fprintf(hFile,"  gexQuery('db_extraction_mode','1_out_of_N_samples', '%s');\n",
                    strExtractionModeParam.toLatin1().data()); // 6050

        // Data type and offline flag
        fprintf(hFile,"  gexQuery('db_data_type','%s');\n",strNewDataType.toLatin1().constData());
        if(bNewOffline)
            fprintf(hFile,"  gexQuery('db_offline_query','true');\n");
        else
            fprintf(hFile,"  gexQuery('db_offline_query','false');\n");

        // All Query parameters specified...now trigger query!
        //if(pGexMainWindow->iWizardType == GEX_MIXQUERY_WIZARD)
        if(nWizardType == GEX_MIXQUERY_WIZARD)
            fprintf(hFile,"  gexGroup('insert_query','%s',group_id);\n",strTitle.toLatin1().constData());
        else
        {
            fprintf(hFile,"  group_id = gexGroup('insert_query','%s');\n",strTitle.toLatin1().constData());
            //insert the condition part if exist
            if(!m_oGroupConditions.isEmpty() && m_oGroupConditions.contains(strTitle))
            {
                QStringList oOrdredCondition = m_poCompareQueryCondition->text().split(QRegExp("[;,]"),QString::SkipEmptyParts);
                foreach(const QString &strKey, oOrdredCondition)
                {
                    QString strValue = ((m_oGroupConditions[strTitle])[strKey]).toString();
                    fprintf(hFile,"  gexCondition(group_id,'%s','%s');\n", strKey.toLatin1().constData(), strValue.toLatin1().constData());
                }
            }
        }

        // Next Query entry
        pTreeWidgetItem = treeWidget->itemBelow(pTreeWidgetItem);
    };

    if(nWizardType == GEX_MIXQUERY_WIZARD)
    {
        fprintf(hFile,"  gexAnalyseMode('MergeDatasets');\n");
    }
    else
    {
        fprintf(hFile,"  gexAnalyseMode('CompareDatasets');\n");
    }
    fprintf(hFile,"\n  sysLog('* Quantix ExaminatorDB Query set ! *');\n\n");
}

///////////////////////////////////////////////////////////
// Read a script file and extract its File List section
///////////////////////////////////////////////////////////
void GexCmpQueryWizardPage1::ReadProcessFilesSection(QString strScriptFile)
{
    QFile file( strScriptFile ); // Read the text from a file
    if (file.open(QIODevice::ReadOnly) == false)
    {
        GS::Gex::Message::
            critical("", "Failed reading file...folder is read protected?");
        return;
    }

    // Empty List
    clearQueries();

    // Read Tasks definition File
    QTextStream hFile;
    hFile.setDevice(&file);	// Assign file handle to data stream

    QString	strString,strSection;
    QString	strReportName,strLocation;
    QString	strFilterName,strFilterValue;
    QStringList testConditions;
    GEX::CSL::CslScriptParser cslScriptParser;
    int		i;
    int		iFromYear,iFromMonth,iFromDay,iFromHour,iFromMinute,iToYear,iToMonth,iToDay,iToHour,iToMinute;
    QDateTime dFrom,dTo;
    QString lQueryTag, lQueryValue;

    do
    {

        // Read one line from file
        strString = hFile.readLine();
        strString = strString.trimmed();	// Remove starting & leading spaces.

        // -- analyse csl string
        if(cslScriptParser.init(strString) == false)
        {
            strNewMappingFile.clear();
            continue;
        }

        if(cslScriptParser.startWith("gexQuery"))
        {
            lQueryTag   = cslScriptParser.getElementQuery(GEX::CSL::Query_Tag);
            lQueryValue = cslScriptParser.getElementQuery(GEX::CSL::Query_Value);

            if(lQueryTag.isEmpty() || lQueryValue.isEmpty())
                continue;
            // Extract Group name
            if(lQueryTag == "reset" && lQueryValue == "all")
            {
                // Empty List as a group starts...
                clearQueries();
            }

            // Extract Report Title
            else if(lQueryTag == "db_report")
            {
                strReportName = lQueryValue;

                strReportName.replace("\\'","'");
                strReportName.replace("\\\\","\\");

                // Set Report title...
                lineEditReportTitle->setText(strReportName);
            }

            // Extract Database location: [Local] or [Server]
            else if(lQueryTag == "db_location")
            {
                strLocation = lQueryValue;

                strLocation.replace("\\'","'");
                strLocation.replace("\\\\","\\");
            }

            // Extract Database name
            else if(lQueryTag == "db_name")
            {
                strNewDatabase = lQueryValue;
                strNewDatabase.replace("\\'","'");
                strNewDatabase.replace("\\\\","\\");

                GexDatabaseEntry* pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine()
                                                                        .FindDatabaseEntry(strNewDatabase);
                if (!pDatabaseEntry)
                {
                    GS::Gex::Message::critical("Error",
                       QString("Failed loading %1\n"
                               "Database link to '%2' not found in the databases list!")
                       .arg(strScriptFile)
                       .arg(strNewDatabase));
                    return;
                }

                // Compute full database name: <Location> <Name>
                strNewDatabase = strLocation + QString(" ") + strNewDatabase;

                // Clear filters
                strNewFilters = "";
            }

            // Extract Database period
            else if(lQueryTag == "db_period")
            {
                // Line format is:
                // gexQuery('db_period','<Period>','From_day From_Month From_Year To_day To_Month To_Year');"
                // or
                // gexQuery('db_period','Last N X','N X');"
                strNewPeriod   = lQueryValue;

                if (strNewPeriod=="Last N X")
                    GSLOG(SYSLOG_SEV_ERROR, "Last N X not implemented !");

                i=0;
                while(gexTimePeriodChoices[i] != 0)
                {
                    if( strNewPeriod == gexTimePeriodChoices[i])
                        break;	// found matching string
                    i++;
                };	// loop until we have found the string entry.

                // If date is custom dates....
                if(i == GEX_QUERY_TIMEPERIOD_CALENDAR)
                {
                    strNewPeriod = cslScriptParser.getElementQuery(GEX::CSL::Query_Value2);
                    int r=sscanf(strNewPeriod.toLatin1().constData(), "%d %d %d %d %d %d %d %d %d %d",
                                 &iFromYear,&iFromMonth,&iFromDay,&iFromHour,&iFromMinute,
                                 &iToYear,&iToMonth,&iToDay,&iToHour,&iToMinute);

                    if (r==EOF)
                        GSLOG(SYSLOG_SEV_WARNING, QString("failed to interpret gexQuery db_period command '%1'")
                              .arg(strNewPeriod)
                              .toLatin1().constData());

                    dFrom.setDate( QDate( iFromYear, iFromMonth, iFromDay ) );
                    dFrom.setTime( QTime( iFromHour, iFromMinute ) );
                    dTo.setDate( QDate( iToYear, iToMonth, iToDay ) );
                    dTo.setTime( QTime( iToHour, iToMinute ) );

                    strNewPeriod = dFrom.toString("dd MM yyyy hh mm");
                    strNewPeriod += " - ";
                    strNewPeriod += dTo.toString("dd MM yyyy hh mm");
                }
                else if(i == GEX_QUERY_TIMEPERIOD_LAST_N_X)
                {
                    strNewPeriod = QString( "Last %1" ).arg( cslScriptParser.getElementQuery(GEX::CSL::Query_Value2) );

                    GSLOG(SYSLOG_SEV_NOTICE, QString(" Last N X : %1")
                          .arg( strNewPeriod)
                          .toLatin1().constData() );
                }
                else
                {
                    // Map Period to GUI period (e.g.: 'all_dates' => "All Dates")
                    strNewPeriod = gexLabelTimePeriodChoices[i];
                }
            }

            // Extract Minimum samples
            else if(lQueryTag == "db_minimum_samples")
            {
                // Line format is:
                // gexQuery('db_minimum_samples'','<Number>');
                // Extract Samples#
                strNewMinimumPartsPerFile   = lQueryValue;
            }

            // Extract Records Data filter samples (eg: Bin1 only,....)
            else if(lQueryTag == "db_data")
            {
                // Line format is:
                // gexQuery('db_data'','<Records type>','<Bin list>');

                // Extract record type
                strNewParts   = lQueryValue;

                i=0;
                while(gexFileProcessPartsItems[i] != 0)
                {
                    if( strNewParts == gexFileProcessPartsItems[i])
                        break;	// found matching string
                    i++;
                };	// loop until we have found the string entry.
                strNewParts = gexLabelFileProcessPartsItems[i];
                // Extract Bin list: beware the comma can be present in bin list!
                strSection = strString.section(',',2);
                strSection = strSection.section('\'',1);
                if(strSection.startsWith("'"))
                    strSection = "";	// No bin list specified.
                // Remove starting and leading ' characters
                strSection = strSection.mid(1,strSection.length()-4);
                if(strSection.isEmpty() == false)
                    strNewParts += QString(": ") + strSection;
            }

            // Extract Mapping file
            else if(lQueryTag == "db_mapping")
            {
                // Line format is:
                // gexQuery('db_mapping'','<file_path>');
                // Extract Mapping file
                strNewMappingFile   = lQueryValue;
                strNewMappingFile.replace("\\'","'");
                strNewMappingFile.replace("\\\\","\\");
            }

            // Extract data type
            else if(lQueryTag == "db_data_type")
            {
                // Line format is:
                // gexQuery('db_data_type'','<data type>');
                // Extract Data type
                strNewDataType   = lQueryValue;
                strNewDataType.replace("\\'","'");
                strNewDataType.replace("\\\\","\\");
            }

            // Extract offline flag
            else if(lQueryTag == "db_offline_query")
            {
                // Line format is:
                // gexQuery('db_data_type'','true|false');
                // Extract Offline flag
                if(lQueryValue.trimmed() == "true")
                    bNewOffline = true;
                else
                    bNewOffline = false;
            }

            // Remote Database Parameter list (ignored if query not done over a remoteDB)
            else if(lQueryTag == "db_testlist")
            {
                // Extract test-list string
                strNewTestList = lQueryValue;

                strNewTestList.replace("\\'","'")
                .replace("\\\\","\\")
                .replace("\\x2c", ",");
            }

            // Remote Database option string (ignored if query not done over a remoteDB, or no options supported)
            else if(lQueryTag == "db_plugin_options")
            {
                // Extract options string
                strNewPluginOptionsString = lQueryValue;
                strNewPluginOptionsString.replace("\\'","'");
                strNewPluginOptionsString.replace("\\\\","\\");
            }

            else if( lQueryTag == "db_extraction_mode" )
            {
                if( lQueryValue == "1_out_of_N_samples" )
                    eNewExtractionMode = GexOneQueryWizardPage1::e1outOfNsamples;
                else
                    eNewExtractionMode = GexOneQueryWizardPage1::eRawData;
            }

            // Extraction mode string
            else if(lQueryTag == "db_consolidated")
            {
                // Line format is:
                // gexQuery('db_consolidated','true|false'));

                // Extract extraction mode
                if( lQueryValue.trimmed() == "true")
                    eNewExtractionMode = GexOneQueryWizardPage1::eConsolidatedData;
                else
                    eNewExtractionMode = GexOneQueryWizardPage1::eRawData;
            }

            // Extract Query Filter
            else if(lQueryTag.startsWith("dbf_") == true)
            {
                // Line format is:
                // Standard Fie-based DB:	gexQuery('dbf_<filter_name>','<filter_value>');
                // SQL DB:					gexQuery('dbf_sql','<filter_name>','<filter_value>');

                // Check if filter for SQL DB
                if(lQueryTag.toLower() == "dbf_sql")
                {
                    strFilterName  = cslScriptParser.getElementQuery(GEX::CSL::Query_Value);
                    strFilterValue = cslScriptParser.getElementQuery(GEX::CSL::Query_Value2);
                }
                else
                {
                    strFilterName = lQueryTag;

                    i=0;
                    while(gexFilterChoices[i] != 0)
                    {
                        if( strFilterName == gexFilterChoices[i])
                            break;	// found matching string
                        i++;
                    };	// loop until we have found the string entry.

                    strFilterName = gexLabelFilterChoices[i];
                    strFilterValue = cslScriptParser.getElementQuery(GEX::CSL::Query_Value);
                }

                strFilterValue = strFilterValue.replace("\\x2c", ",");

                // If we have multiple filters, insert comma
                if(strNewFilters.isEmpty() == false)
                    strNewFilters += FILTERS_SEPARATOR;

                // Update filters list
                strNewFilters += strFilterName;
                strNewFilters += "=";
                strNewFilters += strFilterValue;
            }

            // Extract Query Split field
            else if(lQueryTag == "db_split")
            {
                // Line format is:
                // Standard Fie-based DB:	gexQuery('db_split','dbf_<filter_name>');
                // SQL DB:					gexQuery('db_split','dbf_sql','<filter_name>');

                // Extract Filter name
                strFilterName   = lQueryValue;

                // Check if filter for SQL DB
                if(strFilterName.toLower() == "dbf_sql")
                {
                    strFilterName = cslScriptParser.getElementQuery(GEX::CSL::Query_Value2);
                }
                else
                {
                    i=0;
                    while(gexFilterChoices[i] != 0)
                    {
                        if( strFilterName == gexFilterChoices[i])
                            break;	// found matching string
                        i++;
                    };	// loop until we have found the string entry.
                    strFilterName = gexLabelFilterChoices[i];
                }

                // Update splitfield
                strNewSplitField = strFilterName;
            }
        }
        else if(cslScriptParser.startWith("gexGroup"))
        {
            QString lAction = cslScriptParser.getElementGroup(GEX::CSL::Group_Action);
            // Extract Query Name.
            if(lAction == "insert_query")
            {
                // Extract Query Title
                strNewName = cslScriptParser.getElementGroup(GEX::CSL::Group_Value);

                strNewName.replace("\\'","'");
                strNewName.replace("\\\\","\\");

                // Add query to the ListView!
                OnAddQuery(strNewName,strNewDatabase,strNewPeriod,strNewFilters,strNewMinimumPartsPerFile,strNewParts,strNewMappingFile,
                           strNewTestList,strNewDataType,bNewOffline,strNewSplitField,strNewPluginOptionsString,
                           eNewExtractionMode, strExtractionModeParam);
            }

            // Extract Report condition.
            else if(lAction == "declare_condition")
            {
                QString conditions = cslScriptParser.getElementGroup(GEX::CSL::Group_Value);
                testConditions.append(conditions);

            }
        }
        else if(cslScriptParser.startWith("gexCondition"))
        {
            // Extract condition
            QString conditionName = cslScriptParser.getElementCondition(GEX::CSL::Condition_Name);
            QString conditionValue = cslScriptParser.getElementCondition(GEX::CSL::Condition_Value);
            m_oGroupConditions[strNewName].insert(conditionName, conditionValue);
        }
    }
    while(hFile.atEnd() == false);
    file.close();

    if (testConditions.count() > 0)
        m_poCompareQueryCondition->setText(testConditions.join(","));
}

///////////////////////////////////////////////////////////
// If any SQL database in the list of datasets, return
// true, else return false
///////////////////////////////////////////////////////////
GexDatabaseEntry *GexCmpQueryWizardPage1::FindFirstSqlDataset(QTreeWidgetItem ** ppTreeWidgetItem)
{
    // If no item in list, return false!
    if(treeWidget->topLevelItemCount() <= 0)
        return NULL;

    // Go through all datasets...
    GexDatabaseEntry *	pDatabaseEntry=NULL;
    //QString				strDatabaseName;
    bool				bOffline;

    *ppTreeWidgetItem = treeWidget->topLevelItem(0);

    while(*ppTreeWidgetItem != NULL)
    {
        // Check if Offline flag set...
        if((*ppTreeWidgetItem)->text(9) == "1")
            bOffline = true;
        else
            bOffline = false;

        // Get ptr to database entry
        QString strDatabaseName = (*ppTreeWidgetItem)->text(1).trimmed();
        if(strDatabaseName.startsWith("[Local]") || strDatabaseName.startsWith("[Server]"))
            strDatabaseName = strDatabaseName.section("]",1).trimmed();

        pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(strDatabaseName);

        // Do nothing if database not found
        if(!pDatabaseEntry)
        {
            *ppTreeWidgetItem = treeWidget->itemBelow((*ppTreeWidgetItem));
            continue;
        }

        // Check if SQL database, and Offline disables
        if(pDatabaseEntry->IsExternal() && !bOffline)
            return pDatabaseEntry;

        pDatabaseEntry=NULL;
        *ppTreeWidgetItem = treeWidget->itemBelow((*ppTreeWidgetItem));
    }

    return NULL;
}

///////////////////////////////////////////////////////////
// Display list of Parameters available in database, and
// return user selection
// Note: only applies to SQL databases (Galaxy, mySQL,...)
///////////////////////////////////////////////////////////
QString GexCmpQueryWizardPage1::SqlPickParameterList(bool bParametricOnly)
{
    QTreeWidgetItem		* pTreeWidgetItem	= NULL;
    GexDatabaseEntry	* pDatabaseEntry	= FindFirstSqlDataset(&pTreeWidgetItem);

    if(!pDatabaseEntry)
        return "";

    // We found a SQL dataset, query tests for the corresponding database
    // Create a quert dialog, set fields, and call picktestlist function
    strNewName					= pTreeWidgetItem->text(0);
    strNewDatabase				= pTreeWidgetItem->text(1);
    strNewPeriod				= pTreeWidgetItem->text(2);
    strNewFilters				= pTreeWidgetItem->text(3);
    strNewMinimumPartsPerFile	= pTreeWidgetItem->text(4);
    strNewParts					= pTreeWidgetItem->text(5);
    strNewMappingFile			= pTreeWidgetItem->text(6);
    strNewTestList				= pTreeWidgetItem->text(7);
    strNewDataType				= pTreeWidgetItem->text(8);
    if(pTreeWidgetItem->text(9) == "1")
        bNewOffline = true;
    else
        bNewOffline = false;
    strNewSplitField			= pTreeWidgetItem->text(10);
    strNewPluginOptionsString	= pTreeWidgetItem->text(11);
    if(pTreeWidgetItem->text(12) == "0")
        eNewExtractionMode = GexOneQueryWizardPage1::eRawData;
    else if(pTreeWidgetItem->text(12) == "1")
        eNewExtractionMode = GexOneQueryWizardPage1::eConsolidatedData;
    else if(pTreeWidgetItem->text(12) == "2")
        eNewExtractionMode = GexOneQueryWizardPage1::e1outOfNsamples;

    // Read Query fields
    GexOneQueryWizardPage1 cQueryDialog;
    pGexMainWindow->pWizardAdminGui->ListDatabasesInCombo(cQueryDialog.comboBoxDatabaseName,DB_SELECT_FOR_EXTRACTION);

    // Finalize initialization of different fields.
    SetQueryFields(&cQueryDialog);

    // Update GUI controls depending on current selections
    cQueryDialog.UpdateGui();

    // Call picktestlist function
    return cQueryDialog.SqlPickParameterList(bParametricOnly);
}

bool GexCmpQueryWizardPage1::FillDataTab(const QString &dataDescription)
{
    return true;
}

///////////////////////////////////////////////////////////
// Clear and delete all queries in the treewidget
///////////////////////////////////////////////////////////
void GexCmpQueryWizardPage1::clearQueries()
{
    // Remove selected item from list
    QTreeWidgetItem * pTreeWidgetItem		= treeWidget->topLevelItem(0);
    QTreeWidgetItem * pTreeWidgetNextItem	= NULL;

    while (pTreeWidgetItem)
    {
        pTreeWidgetNextItem = treeWidget->itemBelow(pTreeWidgetItem);

        treeWidget->takeTopLevelItem(treeWidget->indexOfTopLevelItem(pTreeWidgetItem));
        delete pTreeWidgetItem;

        pTreeWidgetItem = pTreeWidgetNextItem;
    }

    m_oGroupConditions.clear();
}

///////////////////////////////////////////////////////////
// Auto adjust the column width to the content
///////////////////////////////////////////////////////////
void GexCmpQueryWizardPage1::onItemChanged(QTreeWidgetItem* /*pTreeWidgetItem*/,
                                           int nColumn)
{
    treeWidget->resizeColumnToContents(nColumn);
}

QStringList GexCmpQueryWizardPage1::getGroupNameList(bool bAll ){ //true All //false all without selected
    QStringList oGroupList;
    for(int iIdx=0; iIdx<treeWidget->topLevelItemCount(); ++iIdx){
        if(bAll)
            oGroupList.append(treeWidget->topLevelItem(iIdx)->text(0));
        else if(!treeWidget->topLevelItem(iIdx)->isSelected())
            oGroupList.append(treeWidget->topLevelItem(iIdx)->text(0));
    }
    return oGroupList;
}
