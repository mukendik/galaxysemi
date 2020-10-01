///////////////////////////////////////////////////////////
// ALL Browser wizard pages for Characterization Query Wizard
///////////////////////////////////////////////////////////
#ifdef _WIN32
  #include <windows.h>
#endif

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qtooltip.h>
#include <qbuttongroup.h>

#include "gex_shared.h"
#include "db_onequery_wizard.h"
#include "browser_dialog.h"
#include "settings_dialog.h"
#include "report_build.h"
#include "report_options.h"
#include "calendar_dialog.h"
#include "picktest_dialog.h"
#include "db_transactions.h"
#include "pickfilter_dialog.h"
#include "gex_constants.h"
#include "db_external_database.h"
#include "product_info.h"
#include "engine.h"
#include "db_engine.h"
#include "test_condition_hierarchy_widget.h"
#include "admin_gui.h"
#include "message.h"

// in main.cpp
extern GexMainwindow *	pGexMainWindow;

// report_build.cpp
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)

// in classes.cpp
extern bool SetCurrentComboItem(QComboBox *pCombo, const QString & strItem, bool bInsertIfMissing=false);

///////////////////////////////////////////////////////////
// WIZARD PAGE 1: Characterization Query Wizard
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_CharQuery(void)
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

    QString lPreviousDatabaseName = pWizardOneQuery->comboBoxDatabaseName->currentText();

    // Make sure ExaminatorDB has the latest list of databases (do not repaint Admin GUI page)
    iDatabasesEntries = ReloadDatabasesList(false);

    if (iDatabasesEntries > 0)
    {
        // Fill the combo box on the 'Single Query' page with the list of databases
        QString strCurrentSelection = pWizardOneQuery->comboBoxDatabaseName->currentText();

        // Show only uptodate DB (including filebased) because querying old DB is not supported
        pWizardAdminGui->ListDatabasesInCombo(pWizardOneQuery->comboBoxDatabaseName,
                             DB_TYPE_SQL|DB_STATUS_UPTODATE|DB_STATUS_CONNECTED,DB_TDR_MANUAL_CHARAC);

        // Make sure we keep the database name selected.
        if(strCurrentSelection.isEmpty() == false)
            SetCurrentComboItem(pWizardOneQuery->comboBoxDatabaseName, strCurrentSelection);
    }

    // If No database available...display the HTML message instead
    if(pWizardOneQuery->comboBoxDatabaseName->count() == 0)
    {
        // No report available
        QString strFilesPage;

        // Display relevant message
        if(GS::LPPlugin::ProductInfo::getInstance()->isDBPluginAllowed())
        {
//            case GEX_DATATYPE_GEX_YIELD123:		// Yield123
//                strFilesPage = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
//                strFilesPage += GEX_HELP_FOLDER;
//                strFilesPage += GEXY123_HTMLPAGE_NODATABASE;
//                break;

//            case GEX_DATATYPE_ALLOWED_DATABASE:
//            case GEX_DATATYPE_GEX_MONITORING:	// ExaminatorMonitoring
//            case GEX_DATATYPE_GEX_PATMAN:		// Galaxy PAT-Man
                strFilesPage = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
                strFilesPage += GEX_HELP_FOLDER;
                strFilesPage += GEX_HTMLPAGE_DB_NODATABASE;
//                break;

//            case GEX_DATATYPE_ALLOWED_DATABASEWEB:
//                strFilesPage = strExaminatorWebUserHome;
//                strFilesPage += GEX_HELP_FOLDER;
//                strFilesPage += GEX_HTMLPAGE_WEB_NODATABASE;
//                break;
        }

        LoadUrl(strFilesPage);

        return;
    }

    // Force 'Settings' page to only display 'Characterization reports'!
    pGexMainWindow->pWizardSettings->OnDoCharacterizationReport();

    // Show wizard page : Query dataset.
    ShowWizardDialog(GEX_ONEQUERY_WIZARD_P1);

    // If the wizard type is not characterization
    // Apply characterization restriction to the GUI
    if (pGexMainWindow->GetWizardType() != GEX_CHAR_QUERY_WIZARD)
    {
        // Force default value
        pWizardOneQuery->UpdateExtractionModeUi(GexOneQueryWizardPage1::eRawData);

        // Set minimum parts to 0
        pWizardOneQuery->lineEditMinimumPartPerFile->setText("0");
        pWizardOneQuery->lineEditMinimumPartPerFile->setEnabled(false);

        // Force last test instance as default value
        pWizardOneQuery->DataTypeFilter->setCurrentIndex(GEX_PROCESSPART_LASTINSTANCE);

        // Force
        if (lPreviousDatabaseName != pWizardOneQuery->comboBoxDatabaseName->currentText())
            pWizardOneQuery->OnDatabaseChanged();
    }

    // Updates Wizard type
    SetWizardType(GEX_CHAR_QUERY_WIZARD);
}

void GexOneQueryWizardPage1::SetSelectedTestConditions(const QStringList &selection)
{
    if (selection != widgetConditionLevel->GetSelectedFields())
    {
        // Fill Test Condition Line Edit
//        lineEditTestCondition->setText(selection.join(","));

        widgetConditionLevel->SetSelectedFields(selection);
    }
}

void GexOneQueryWizardPage1::SetSynchronizedTestConditions(bool synchronized)
{
    if (mSynchronizedTestCond != synchronized)
    {
        mSynchronizedTestCond = synchronized;

        // Do some Stuff here
//        int lIndex = TabWidgetFilters->indexOf(tabTestConditions);

//        if (mSynchronizedTestCond)
//        {
//            if (lIndex >= 0)
//                TabWidgetFilters->setTabIcon(lIndex, QIcon(QString(":/gex/icons/green-flag.png")));
//        }
//        else
//        {
//            if (lIndex >= 0)
//                TabWidgetFilters->setTabIcon(lIndex, QIcon(QString(":/gex/icons/red-flag.png")));
//        }
    }
}

