///////////////////////////////////////////////////////////
// ALL Browser wizard pages for SQL Production/Template & SQL GUI
///////////////////////////////////////////////////////////
#ifdef _WIN32
#include <windows.h>
#endif

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qtooltip.h>
#include <qbuttongroup.h>
#include "message.h"
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
#include "admin_gui.h"

extern GexMainwindow *	pGexMainWindow;

// report_build.cpp
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)

// in classes.cpp
extern bool SetCurrentComboItem(QComboBox *pCombo, const QString & strItem, bool bInsertIfMissing=false);

///////////////////////////////////////////////////////////
// WIZARD PAGE 1: SQL Production/Template and SQL GUI
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_SqlQuery(void)
{
    int	iDatabasesEntries;

    // Check if data access type if FILES or DATABASE
    if(!GS::LPPlugin::ProductInfo::getInstance()->isDBPluginAllowed())
    {
        QString m=ReportOptions.GetOption("messages", "upgrade").toString();
        GS::Gex::Message::information("", m);
        return;
    }

    // Make sure ExaminatorDB has the latest list of databases (do not repaint Admin GUI page)
    iDatabasesEntries = ReloadDatabasesList(false);

    if (iDatabasesEntries > 0)
    {
        // Fill the combo box on the 'Single Query' page with the list of databases
        QComboBox *pValue = pWizardOneQuery->comboBoxDatabaseName;
        QString strCurrentSelection = pValue->currentText();
        // Show only uptodate DB (including filebased) because querying old DB is not supported
        pWizardAdminGui->ListDatabasesInCombo(pWizardOneQuery->comboBoxDatabaseName,
                             DB_SELECT_FOR_EXTRACTION, DB_TDR_MANUAL_PROD|DB_TDR_YM_PROD);
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
            strFilesPage = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
            strFilesPage += GEX_HELP_FOLDER;
            strFilesPage += GEX_HTMLPAGE_DB_NODATABASE;
        }
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
//        }

        LoadUrl(strFilesPage);

        return;
    }

    // Force 'Settings' page to only display 'Enterprise reports'!
    pGexMainWindow->pWizardSettings->OnDoSqlReport();

    // Show wizard page : Query dataset.
    ShowWizardDialog(GEX_ENTERPRISE_SQL_P1);

    // Updates Wizard type
    SetWizardType(GEX_SQL_QUERY_WIZARD);
}

