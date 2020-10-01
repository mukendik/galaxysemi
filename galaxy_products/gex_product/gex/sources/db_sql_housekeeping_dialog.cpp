///////////////////////////////////////////////////////////
// SQL Housekeeping dialog (DB update, DB purge...)
///////////////////////////////////////////////////////////

// QT includes
#include <qapplication.h>
#include <QSqlQuery>
#include <QSqlResult>
#include <QSqlError>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QTextStream>
#include <QtWidgets>
#include <QSqlRecord>
#include <QSqlTableModel>

// Galaxy modules includes
#include <gqtl_sysutils.h>

// Local includes
#include "gex_shared.h"
#include "db_engine.h"
#include "admin_engine.h"
#include "engine.h"
#include "browser_dialog.h"
#include "db_onequery_wizard.h"
#include "db_transactions.h"
#include "gex_database_entry.h"
#include "db_external_database.h"
#include "db_sql_housekeeping_dialog.h"
#include "report_build.h"
#include <gqtl_log.h>
#include "scheduler_engine.h"
#include "mo_scheduler_gui.h"
#include "gexdb_plugin_option.h"
#include "product_info.h"
#include "message.h"
#include "dir_access_base.h"
#include "gex_options_map.h"

#include <sstream>

#define BACKGROUND_EVENT "background_transfer"

// in main.cpp
extern GexMainwindow	*pGexMainWindow;

// report_build.cpp
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)
extern CGexSkin*		pGexSkin;			// holds the skin settings

// Update to InnoDb Barracuda Compressed
//#define BARRACUDA_ENABLED

///////////////////////////////////////////////////////////
// Constructor/Destructor
///////////////////////////////////////////////////////////
GexSqlHousekeepingDialog::GexSqlHousekeepingDialog(
        const QString & strDatabaseName, QWidget* parent, bool modal, Qt::WindowFlags fl)
    : QDialog(parent, fl), m_pToInnoDBPushButton(NULL), m_pEngineLabel(NULL)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString(" on %1").arg( strDatabaseName).toLatin1().constData() );
    setupUi(this);
    setModal(modal);

    // Set Examinator skin
    if (pGexSkin)
        pGexSkin->applyPalette(this);

    editLog->setLineWrapMode(QTextEdit::NoWrap);
    editLog_Incremental->setLineWrapMode(QTextEdit::NoWrap);
    editLog_MetaData->setLineWrapMode(QTextEdit::NoWrap);
    editLog_OptionSettings->setLineWrapMode(QTextEdit::NoWrap);
    editLog_History->setLineWrapMode(QTextEdit::NoWrap);

    buttonMetaData_Cancel->setEnabled(false);
    buttonOptionSettings_Cancel->setEnabled(false);

    // Because BUG
    editLog_History->setProperty("textInteractionFlags",Qt::NoTextInteraction);


    m_bUpdateProcessRunning = false;

    // InnoDb management
    m_pToInnoDBPushButton =new QPushButton("Update to InnoDB");
    verticalLayout->addWidget(m_pToInnoDBPushButton); m_pToInnoDBPushButton->hide();

    m_pEngineLabel=new QLabel( "MySQL Engine");
    gridLayout_2->addWidget(m_pEngineLabel, 3, 0); m_pEngineLabel->hide();

    m_pEngineValueLabel=new QLineEdit( "?");
    gridLayout_2->addWidget(m_pEngineValueLabel, 3, 1); m_pEngineValueLabel->setReadOnly(true); m_pEngineValueLabel->hide();

    QObject::connect(m_pToInnoDBPushButton,   SIGNAL(released()),             this, SLOT(OnButtonUpdateToInnoDB()) );
    QObject::connect(buttonClose,             SIGNAL(clicked()),              this, SLOT(onAccept()));
    QObject::connect(buttonUpdate,            SIGNAL(clicked()),              this, SLOT(OnButtonUpdate()));
    QObject::connect(buttonStartStopBGEvent,  SIGNAL(clicked()),              this, SLOT(OnStartStopBGEvent()));
    QObject::connect(listSelection,           SIGNAL(currentRowChanged(int)), this, SLOT(OnItemSelected(int)));
    QObject::connect(buttonIncremental_Update,SIGNAL(clicked()),              this, SLOT(OnButtonIncremental_Update()));
    QObject::connect(buttonIncremental_Check, SIGNAL(clicked()),              this, SLOT(OnButtonIncremental_Check()));
    QObject::connect(buttonIncremental_Cancel,SIGNAL(clicked()),              this, SLOT(OnButtonIncremental_Cancel()));
    QObject::connect(buttonIncremental_Apply, SIGNAL(clicked()),              this, SLOT(OnButtonIncremental_Apply()));
    QObject::connect(comboBoxTestingStage,    SIGNAL(activated(QString)),     this, SLOT(OnTestingStageSelected()));
    QObject::connect(buttonMetaData_Cancel,   SIGNAL(clicked()),              this, SLOT(OnButtonMetaData_Cancel()));
    QObject::connect(buttonMetaData_Apply,    SIGNAL(clicked()),              this, SLOT(OnButtonMetaData_Apply()));
    QObject::connect(buttonMoreLessTables,    SIGNAL(clicked()),              this, SLOT(OnButtonBackgroundMoreInfo()));
    QObject::connect(buttonMoreLessPartitions,SIGNAL(clicked()),              this, SLOT(OnButtonBackgroundMorePartitionInfo()));
    QObject::connect(buttonMoreLessLogs,      SIGNAL(clicked()),              this, SLOT(OnButtonBackgroundMoreLogs()));
    QObject::connect(buttonRefreshBackground, SIGNAL(clicked()),              this, SLOT(FillBackgroundTransferInformations()));

    tableMetaData->setContextMenuPolicy(Qt::CustomContextMenu);
    tableMetaData->verticalHeader()->hide();

    QObject::connect(buttonOptionSettings_Cancel, SIGNAL(clicked()),          this, SLOT(OnButtonOptionSettings_Cancel()));
    QObject::connect(buttonOptionSettings_Apply,  SIGNAL(clicked()),          this, SLOT(OnButtonOptionSettings_Apply()));

    tableOptionSettings->setContextMenuPolicy(Qt::CustomContextMenu);

    tableIncrementalUpdates->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(tableIncrementalUpdates,   SIGNAL(customContextMenuRequested ( const QPoint & )),  this, SLOT(OnIncrementalUpdateContextualMenu(const QPoint&)));

    mCurrentProgressBar = NULL;

    progressBar_Update->hide();
    progressBar_Incremental->hide();
    progressBar_MetaData->hide();
    progressBar_OptionSettings->hide();

    // Fill Database name combo
    m_pTdrDatabaseEntry = NULL;
    m_TdrDbPlugin = NULL;
    m_pAdrDatabaseEntry = NULL;
    m_AdrDbPlugin = NULL;

    buttonUpdate->setEnabled(false);
    buttonUpdate->setText("Update");
    buttonIncremental_Update->setEnabled(false);
    buttonIncremental_Update->setText("Run");

    // Set database name
    editDatabaseName->setText(strDatabaseName);

    OnDatabaseRefresh();

    setMaximumWidth(
                QApplication::desktop()->availableGeometry().width()  * 90 / 100);
    setMaximumHeight(
                QApplication::desktop()->availableGeometry().height() * 95 / 100);
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    resize(QApplication::desktop()->availableGeometry().width() * 80 / 100,
           this->maximumHeight());
    //resize(this->sizeHint());

    // Select first item in list (DB update)
    m_nCurrentSection = UPDATE_ITEM_INDEX;
    listSelection->setCurrentItem(listSelection->item(UPDATE_ITEM_INDEX));

    mShowMoreBackgroundTableInfo = false;
    mShowMoreBackgroundPartitionInfo = false;
    mShowMoreBackgroundLogs = false;
    listSelection->show();
}

GexSqlHousekeepingDialog::~GexSqlHousekeepingDialog()
{
    // Remove the Consolidation widget
    // that not must be delete by this class
    QWidget* ctWidget = widgetStack->widget(7);
    if(ctWidget != NULL)
    {
        // Remove current widget
        widgetStack->removeWidget(ctWidget);
        ctWidget->setParent(NULL);
    }
}

void GexSqlHousekeepingDialog::OnButtonUpdateToInnoDB()
{
    GSLOG(SYSLOG_SEV_DEBUG, " asking for the innoDB update.");
    if (!m_pTdrDatabaseEntry->m_pExternalDatabase)
    {
        GSLOG(SYSLOG_SEV_WARNING, " error : m_pExternalDatabase NULL !");
        return;
}

    QString strCommand = "InnoDB";

#ifdef BARRACUDA_ENABLED
    strCommand = "Barracuda";
#endif

    if(UpdateDbSummaryDialog(strCommand) != QMessageBox::Yes)
        return;

    GSLOG(SYSLOG_SEV_INFORMATIONAL, " Let's go for the innoDB update.");

    bool bButtonStatus = buttonUpdate->isEnabled();
    buttonUpdate->setEnabled(false);
    buttonClose->setEnabled(false);
    progressBar_Update->show();
    m_pToInnoDBPushButton->setEnabled(false);
    m_bUpdateProcessRunning = true;

    m_pTdrDatabaseEntry->TraceUpdate("HOUSEKEEPING","INFO","Start converting Database to "+strCommand);
    connect(m_TdrDbPlugin, SIGNAL(sLogRichMessage(QString,bool)), this, SLOT(UpdateStandardLog(QString,bool)));
    if (!m_pTdrDatabaseEntry->m_pExternalDatabase->UpdateDb(strCommand))
    {
        m_pTdrDatabaseEntry->TraceUpdate("HOUSEKEEPING","INFO","Start converting Database to "+
                                         strCommand+" error: ToInnoDB failed");
        GSLOG(SYSLOG_SEV_WARNING, " ToInnoDB failed !");
    }
    disconnect(m_TdrDbPlugin, SIGNAL(sLogRichMessage(QString,bool)), this, SLOT(UpdateStandardLog(QString,bool)));

    m_bUpdateProcessRunning = false;
    progressBar_Update->hide();
    buttonClose->setEnabled(true);
    buttonUpdate->setEnabled(bButtonStatus);

    // Update InnoDB controls
    UpdateInnoDbControls(m_TdrDbPlugin);
}

void GexSqlHousekeepingDialog::OnDatabaseRefresh()
{
    QString         lTdrDatabaseName, lAdrDatabaseName, strDatabaseVersion, strSupportedVersion;
    // Version for CONSOLIDATION_TREE
    // Will be REMOVED when REFACTORING
    // Just hack the code for the moment
    unsigned int    uiDatabaseBuild = 16;
    int             lIncrementalSplitlots;

    // Clear log
    progressBar_Update->reset();

    // Extract database logical name from current selection.
    // Skip the [Local]/[Server] info.
    lTdrDatabaseName = editDatabaseName->text();
    if(lTdrDatabaseName.startsWith("[Local]") || lTdrDatabaseName.startsWith("[Server]"))
        lTdrDatabaseName = lTdrDatabaseName.section("]",1).trimmed();

    m_pTdrDatabaseEntry = NULL;
    m_TdrDbPlugin = NULL;
    m_pAdrDatabaseEntry = NULL;
    m_AdrDbPlugin = NULL;
    m_bHaveAdr = false;

    // Get TDR database ptr
    m_pTdrDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(lTdrDatabaseName);
    if (!m_pTdrDatabaseEntry)
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("impossible to retrieve this DB entry (%1)").arg( lTdrDatabaseName).toLatin1().constData());
        return;
    }
    if (!m_pTdrDatabaseEntry->m_pExternalDatabase)
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("this DB (%1) is not an external DB").arg( lTdrDatabaseName).toLatin1().constData());
        return;
    }
    GS::Gex::Engine::GetInstance().GetDatabaseEngine().CheckDatabaseEntryStatus(m_pTdrDatabaseEntry,false);
    bool lIsDbUpToDate = m_pTdrDatabaseEntry->IsUpToDate();

    GexDbPlugin_ID	*pPlugin = m_pTdrDatabaseEntry->m_pExternalDatabase->GetPluginID();
    if(!pPlugin)
    {
        GSLOG(SYSLOG_SEV_WARNING, "impossible to retrieve the pluginId from this DB entry");
        return;
    }

    if (m_TdrDbPlugin)
    {
        disconnect(m_TdrDbPlugin, SIGNAL(sUpdateProgress(int)), this, SLOT(UpdateProgress(int)));
        disconnect(m_TdrDbPlugin, SIGNAL(sResetProgress(bool)), this, SLOT(ResetProgress(bool)));
        disconnect(m_TdrDbPlugin, SIGNAL(sMaxProgress(int)), this, SLOT(SetMaxProgress(int)));
    }
    m_TdrDbPlugin = pPlugin->m_pPlugin;
    if (!m_TdrDbPlugin)
    {
        GSLOG(SYSLOG_SEV_WARNING, " error : Exernal DB plugin NULL !");
        return;
    }
    connect(m_TdrDbPlugin, SIGNAL(sUpdateProgress(int)), this, SLOT(UpdateProgress(int)));
    connect(m_TdrDbPlugin, SIGNAL(sResetProgress(bool)), this, SLOT(ResetProgress(bool)));
    connect(m_TdrDbPlugin, SIGNAL(sMaxProgress(int)), this, SLOT(SetMaxProgress(int)));

    // Set DB name
    editPluginName->setText(pPlugin->pluginName());

    // By default, disable 'Update' buttons
    buttonUpdate->setEnabled(false);
    buttonUpdate->setText("Update");
    buttonIncremental_Update->setEnabled(false);

    // Set debug mode
    if(m_pTdrDatabaseEntry->m_pExternalDatabase->IsCustomerDebugModeActivated())
        labelDebugModeYN->setText("YES");
    else
        labelDebugModeYN->setText("NO");

    labelDebugMode->hide();
    labelDebugModeYN->hide();

    m_TdrDbPlugin->GetIncrementalUpdatesCount(true,lIncrementalSplitlots);

    strDatabaseVersion = m_pTdrDatabaseEntry->DatabaseVersion();
    strSupportedVersion = pPlugin->pluginName();
    editCurrentGexdbVersion->setText(strDatabaseVersion);
    editPluginGexdbVersion->setText(strSupportedVersion);
    editIncrementalUpdate_Splitlots->setText(QString::number(lIncrementalSplitlots));

    // Enable update buttons?
    if(!lIsDbUpToDate)
        buttonUpdate->setEnabled(true);
    else
        buttonUpdate->setEnabled(false);

    m_bHaveAdr = m_pTdrDatabaseEntry->m_pExternalDatabase->MustHaveAdrLink();

    if(m_bHaveAdr)
    {
        // Get ADR database ptr
        lAdrDatabaseName = m_pTdrDatabaseEntry->m_pExternalDatabase->GetAdrLinkName();
        if(!lAdrDatabaseName.isEmpty())
        {
            m_pAdrDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(lAdrDatabaseName, false);
            if (!m_pAdrDatabaseEntry)
            {
                GSLOG(SYSLOG_SEV_WARNING, QString("impossible to retrieve this DB entry (%1)").arg( lAdrDatabaseName).toLatin1().constData());
                lIsDbUpToDate = false;
            }
            else if (!m_pAdrDatabaseEntry->m_pExternalDatabase)
            {
                GSLOG(SYSLOG_SEV_WARNING, QString("this DB (%1) is not an external DB").arg( lAdrDatabaseName).toLatin1().constData());
                lIsDbUpToDate = false;
            }
            else
                m_AdrDbPlugin = m_pAdrDatabaseEntry->m_pExternalDatabase->GetPluginID()->m_pPlugin;

        }
        editLog->insertHtml("<BR>\n");
        editLog->insertHtml("<BR>\n<b>Database ADR Status:</b>");
        if(m_pAdrDatabaseEntry)
        {
            QStringList lAgents = m_pAdrDatabaseEntry->DatabaseVersion().split("\n");
            foreach (QString lAgent, lAgents)
            {
                if(!lAgent.isEmpty())
                {
                    editLog->insertHtml("<BR>\n"+lAgent);
                }
            }
        }
        else
        {
            editLog->insertHtml("<BR>\nNo ADR associated");
        }
    }

    ResetItemSelectionList(lIsDbUpToDate, 16);

    // Display new field DB_STATUS
    labelPluginGexdbVersion->setText("Database Status:");
    if(m_pTdrDatabaseEntry->IsUpToDate())
    {
        //editPluginGexdbVersion->setBackgroundColor(QColor(Qt::white));
        QPalette palette;
        palette.setColor(editPluginGexdbVersion->backgroundRole(), QColor(Qt::white));
        editPluginGexdbVersion->setPalette(palette);
        editPluginGexdbVersion->setText("Ready");
    }
    else if(m_pTdrDatabaseEntry->IsCompatible())
    {
        editPluginGexdbVersion->setText("Compatible - New build available - "+strSupportedVersion);
        //editPluginGexdbVersion->setBackgroundColor(QColor(Qt::green));
        QPalette palette;
        palette.setColor(editPluginGexdbVersion->backgroundRole(), QColor(Qt::green));
        editPluginGexdbVersion->setPalette(palette);

    }

    if(!m_pTdrDatabaseEntry->m_strLastInfoMsg.isEmpty())
    {
        editPluginGexdbVersion->setText(m_pTdrDatabaseEntry->m_strLastInfoMsg);
        if(m_pTdrDatabaseEntry->m_strLastInfoMsg.contains(":"))
            editPluginGexdbVersion->setText(m_pTdrDatabaseEntry->m_strLastInfoMsg.section(":",1));
        //editPluginGexdbVersion->setBackgroundColor(QColor(Qt::red));
        QPalette palette;
        palette.setColor(editPluginGexdbVersion->backgroundRole(), QColor(Qt::red));
        editPluginGexdbVersion->setPalette(palette);

    }

    if(lIsDbUpToDate && (lIncrementalSplitlots > 0))
        buttonIncremental_Update->setEnabled(true);
    else
        buttonIncremental_Update->setEnabled(false);

    // Custom incremental update
    if(uiDatabaseBuild < 15)
    {
        buttonIncremental_Check->setToolTip("Check incremental update\nThis function is not avalaible for this database version\nYou need to update your database before...");
        buttonIncremental_Check->setEnabled(false);
    }
    else
    {
        buttonIncremental_Check->setToolTip("Check incremental update");
        buttonIncremental_Check->setEnabled(true);

        if(uiDatabaseBuild > 15)
        {
            // Consolidation widget
            // To refresh the widget, we need to remove the page
            QWidget* ctWidget = widgetStack->widget(7);
            if(ctWidget != NULL)
            {
                // Remove current widget
                widgetStack->removeWidget(ctWidget);
                ctWidget->setParent(NULL);
    }
        }
    }

    LoadIncrementalSettings();

    // MetaData
    comboBoxTestingStage->clear();
    comboBoxTestingStage->setEnabled(false);

    // For selecting data type to query (wafer sort, or final test,...)
    if(m_pTdrDatabaseEntry->m_pExternalDatabase->IsTestingStagesSupported())
    {
        QStringList strlSupportedTestingStages;
        m_pTdrDatabaseEntry->m_pExternalDatabase->GetSupportedTestingStages(strlSupportedTestingStages);
        comboBoxTestingStage->insertItems(0,strlSupportedTestingStages);
        comboBoxTestingStage->setEnabled(true);
        OnTestingStageSelected();
    }

    // Global Options
    LoadOptionSettings();

    // Update InnoDB controls
    UpdateInnoDbControls(m_TdrDbPlugin);

}

void GexSqlHousekeepingDialog::OnButtonUpdate()
{
    if(pGexMainWindow == NULL)
        return;

    // Clear log
    editLog->clear();
    progressBar_Update->reset();
    mCurrentProgressBar = progressBar_Update;

    // Get database ptr
    // Check if the TDR is UpToDate
    // The TDR can be uptodate but not the ADR
    bool lUpdateTdr = false;
    bool lCreateAdr = false;
    bool lUpdateAdrLink = false;

    if(m_bHaveAdr)
    {
        // Get ADR database ptr
        if(m_pAdrDatabaseEntry == NULL)
        {
            // No ADR found
            // Must create one
            lCreateAdr = true;
        }
    }

    if(m_pTdrDatabaseEntry)
        lUpdateTdr = !(m_pTdrDatabaseEntry->StatusFlags() & STATUS_DBUPTODATE);

    if(lCreateAdr || lUpdateTdr)
    {
        // Display a summary
        // Special case if the TDR is up to date
        // Add information for the creation of the ADR and the update
        QString			strDatabaseVersion, strSupportedVersion;
        strDatabaseVersion = m_pTdrDatabaseEntry->DatabaseVersion();
        GexDbPlugin_ID	*pPlugin;
        pPlugin = m_pTdrDatabaseEntry->m_pExternalDatabase->GetPluginID();
        if(pPlugin)
            strSupportedVersion = pPlugin->pluginName();

        QString lCommand = "UpdateDbFrom="+strDatabaseVersion
                +"|UpdateDbTo="+strSupportedVersion
                +"|DbStatus="+m_pTdrDatabaseEntry->m_strLastInfoMsg.section(":",1);
        if(m_bHaveAdr)
        {
            if(lCreateAdr)
            {
                lCommand += "|AdrStatus=";
                lCommand += "Creation of the "+QString(GEXDB_ADR_NAME);
            }
        }

        int nResult = UpdateDbSummaryDialog(lCommand);
        if((nResult != QMessageBox::Yes) && (nResult != QMessageBox::YesToAll))
            return;
    }
    if(lCreateAdr)
    {
        // Have to create the associted ADR
        // Need to create a new DatabaseEntry
        // Configurate the connector
        // Execute the ADR creation script
        GexDbPlugin_ID	*pPlugin;
        pPlugin = m_pTdrDatabaseEntry->m_pExternalDatabase->GetPluginID();
        GexDbPlugin_Connector lAdrConnector("Adr Connector");
        // Get the default value according to the TDR name
        GS::Gex::Engine::GetInstance().GetDatabaseEngine().UpdateAdrDatabaseConnector(lAdrConnector, pPlugin->m_pPlugin->m_pclDatabaseConnector);

        // If External database
        m_pAdrDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(pPlugin->pluginName(),
                                                                                                   lAdrConnector.m_strHost_Name,
                                                                                                   lAdrConnector.m_uiPort,
                                                                                                   lAdrConnector.m_strDriver,
                                                                                                   lAdrConnector.m_strSchemaName,
                                                                                                   lAdrConnector.m_strDatabaseName);

        if(m_pAdrDatabaseEntry != NULL)
        {
            bool bStatus = true;
            //if the link already exists and doesn't respect the naming rule, delete the existing link and create a new one
            if (!(m_pAdrDatabaseEntry->StatusFlags() & STATUS_CONNECTED)
                || !m_pAdrDatabaseEntry->LogicalName().contains("@"))
            {
                // Remove database folder and content from disk
                // Request Database Transaction module to remove Database Entry
                bStatus = GS::Gex::Engine::GetInstance().GetDatabaseEngine().DeleteDatabaseEntry(m_pAdrDatabaseEntry->LogicalName());
                if(bStatus == false)
                {
                    return;
                }

                m_pAdrDatabaseEntry = NULL;

                lUpdateAdrLink = true;
            }
            else
            {
                UpdateStandardLog(QString("Adr already referenced"), true);
                m_AdrDbPlugin = m_pAdrDatabaseEntry->m_pExternalDatabase->GetPluginID()->m_pPlugin;
            }

        }

        bool bStatus = true;

        if(m_pAdrDatabaseEntry == NULL)
        {
            // build XML file path to External database settings (login, mappinng tables...)
            GexRemoteDatabase lRemoteDatabase;
            QString strPluginFile = pPlugin->pluginFileName();
            QString strPluginName = pPlugin->pluginName();
            if(!lRemoteDatabase.LoadPluginID(strPluginFile, 1, strPluginName))
            {
                return;  // Corrupted file.
            }
            // Initialize and Check Remote connection
            GexDbPlugin_ID *pAdrPlugin = lRemoteDatabase.GetPluginID();
            if(!pAdrPlugin || !pAdrPlugin->m_pPlugin)
            {
                return;
            }

            pAdrPlugin->m_pPlugin->m_pclDatabaseConnector = new GexDbPlugin_Connector("Adr Connector");

            QMap<QString, QString>      lProductInfoMap;
            QMap<QString, QString>      lGuiOptionsMap;
            // Set  product info
            lProductInfoMap.insert("product_id",
                                   QString::number(GS::LPPlugin::ProductInfo::getInstance()->getProductID()));
            lProductInfoMap.insert("optional_modules",
                                   QString::number(GS::LPPlugin::ProductInfo::getInstance()->getOptionalModules()));
            // Set GUI options
            lGuiOptionsMap.insert("open_mode","creation");
            lGuiOptionsMap.insert("read_only","yes");
            lGuiOptionsMap.insert("allowed_database_type",GEXDB_ADR_KEY);

            // Copy ADR configuration
            *pAdrPlugin->m_pPlugin->m_pclDatabaseConnector = lAdrConnector;
            bStatus = pAdrPlugin->m_pPlugin->ConfigWizard(lProductInfoMap,lGuiOptionsMap);
            if(bStatus)
            {
                lAdrConnector = *pAdrPlugin->m_pPlugin->m_pclDatabaseConnector;
                // Create the new Database Entry for the ADR
                QString lError;
                if(!GS::Gex::Engine::GetInstance().GetDatabaseEngine().CreateDatabaseEntry(pAdrPlugin,
                                                                                           pAdrPlugin->m_pPlugin->m_pclDatabaseConnector->m_strSchemaName
                                                                                           + "@"
                                                                                           + lAdrConnector.m_strHost_Name,
                                                                                           lError))
                {
                    UpdateStandardLog(lError, true);
                    return;
                }
            }

            // Clean new pointeur
            delete pAdrPlugin->m_pPlugin->m_pclDatabaseConnector;
            pAdrPlugin->m_pPlugin->m_pclDatabaseConnector = NULL;
        }

        if(bStatus)
        {
            // Need to associate this new ADR with the TDR
            m_pTdrDatabaseEntry->m_pExternalDatabase->SetAdrLinkName(lAdrConnector.m_strDatabaseName
                                                                     + "@"
                                                                     + lAdrConnector.m_strHost_Name);
            GS::Gex::Engine::GetInstance().GetAdminEngine().SaveDatabase(m_pTdrDatabaseEntry);
            GS::Gex::Engine::GetInstance().GetDatabaseEngine().LoadDatabasesListIfEmpty(true);
        }

        // Need to update the m_pAdrDatabaseEntry
        OnDatabaseRefresh();
    }

    if(lUpdateTdr || lUpdateAdrLink)
    {
        bool bStatus = true;
        QString strCommand;

        // Update DB
        m_pToInnoDBPushButton->setEnabled(false);
        buttonUpdate->setEnabled(false);
        buttonClose->setEnabled(false);
        m_bUpdateProcessRunning = true;
        progressBar_Update->show();

        m_pTdrDatabaseEntry->TraceUpdate("HOUSEKEEPING","INFO","Start updating Database");

        if(lUpdateTdr)
        {
            GexDbPlugin_ID	*pPlugin;

            pPlugin = m_pTdrDatabaseEntry->m_pExternalDatabase->GetPluginID();
            if(pPlugin)

            {
                    m_TdrDbPlugin = pPlugin->m_pPlugin;
                    connect(m_TdrDbPlugin, SIGNAL(sLogRichMessage(QString,bool)), this, SLOT(UpdateStandardLog(QString,bool)));
                    connect(m_TdrDbPlugin, SIGNAL(sUpdateProgress(int)), this, SLOT(UpdateProgress(int)));
                    connect(m_TdrDbPlugin, SIGNAL(sResetProgress(bool)), this, SLOT(ResetProgress(bool)));
                    connect(m_TdrDbPlugin, SIGNAL(sMaxProgress(int)), this, SLOT(SetMaxProgress(int)));

                    // Need the update to check
                    // ° The MySql version
                    // ° The users privileges
                    // ° and update the TDR Schema
                    bStatus = m_pTdrDatabaseEntry->m_pExternalDatabase->UpdateDb(strCommand);
                    disconnect(m_TdrDbPlugin, SIGNAL(sLogRichMessage(QString,bool)), this, SLOT(UpdateStandardLog(QString,bool)));
                    disconnect(m_TdrDbPlugin, SIGNAL(sUpdateProgress(int)), this, SLOT(UpdateProgress(int)));
                    disconnect(m_TdrDbPlugin, SIGNAL(sResetProgress(bool)), this, SLOT(ResetProgress(bool)));
                    disconnect(m_TdrDbPlugin, SIGNAL(sMaxProgress(int)), this, SLOT(SetMaxProgress(int)));
                    pPlugin->m_pPlugin->m_pclDatabaseConnector->Disconnect();
                }
            if(bStatus)
            {
                // Update ym_databases to indicate that database was updated
                // Save new plugin version ...
                // Update the XML file
                if( GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
                {
                    if(m_pTdrDatabaseEntry->IsStoredInDb())
                        GS::Gex::Engine::GetInstance().GetAdminEngine().SaveDatabase(m_pTdrDatabaseEntry);
                }
                m_pTdrDatabaseEntry->SaveToXmlFile(QString());
                // Reload database list & update GUI
                // Will be done with the OnDatabaseRefresh
                // GS::Gex::Engine::GetInstance().GetDatabaseEngine().CheckDatabaseEntryStatus(m_pTdrDatabaseEntry,false);
            }
            else
                m_pTdrDatabaseEntry->TraceUpdate("HOUSEKEEPING","INFO","Start updating Database error: Update failed");
        }
        if(bStatus && lUpdateAdrLink)
        {
            // Update ym_databases to indicate that database was updated
            // Save new plugin version ...
            // Update the XML file
            if( GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
            {
                if(m_pAdrDatabaseEntry->IsStoredInDb())
                    GS::Gex::Engine::GetInstance().GetAdminEngine().SaveDatabase(m_pAdrDatabaseEntry);
            }
            m_pAdrDatabaseEntry->SaveToXmlFile(QString());
            // Reload database list & update GUI
            GS::Gex::Engine::GetInstance().GetDatabaseEngine().CheckDatabaseEntryStatus(m_pAdrDatabaseEntry,false);
        }

        progressBar_Update->hide();
        m_bUpdateProcessRunning = false;
        buttonClose->setEnabled(true);
        buttonUpdate->setEnabled(true);
    }
    OnDatabaseRefresh();
}

void GexSqlHousekeepingDialog::OnButtonIncremental_Update(QString strIncrementalName/*=QString()*/)
{
    if(m_bUpdateProcessRunning)
    {
        m_bAbortRequested = true;
        editLog_Incremental->append("<font color=RED><b>ABORT REQUESTED NOW</b></font>");
        editLog_Incremental->append("<font color=RED><b>WAIT THE END OF THE CURRENT UPDATE</b></font>");
        return;
    }

    // Clear log
    editLog_Incremental->clear();
    progressBar_Incremental->reset();

    // Update UI
    buttonIncremental_Check->setEnabled(false);
    buttonIncremental_Update->setEnabled(false);
    buttonClose->setEnabled(false);

    // Get database ptr
    if(m_pTdrDatabaseEntry->m_pExternalDatabase
            && m_TdrDbPlugin
            && m_TdrDbPlugin->m_pclDatabaseConnector)
    {

        // Dialod Box
        // Total splitlot for update
        // Run one step
        // Run all step
        // Cancel

        bool bUpdateAll=false;
        if(strIncrementalName.isEmpty())
        {
            int nResult = UpdateDbSummaryDialog("IncrementalUpdate");
            if(nResult == QMessageBox::Cancel)
            {
                // Update UI
                buttonClose->setEnabled(true);
                buttonIncremental_Check->setEnabled(true);
                buttonIncremental_Update->setEnabled(true);
                return;
            }

            bUpdateAll= (nResult  == QMessageBox::YesToAll);
        }

        // Get incremental updates
        int    lIncrementalUpdateNb;
        QMap< QString, QString > lSummary;

        // Update DB
        m_bUpdateProcessRunning = true;
        m_bAbortRequested = false;
        progressBar_Incremental->show();

        if(bUpdateAll)
        {
            m_TdrDbPlugin->GetIncrementalUpdatesCount(false, lIncrementalUpdateNb);
            buttonIncremental_Update->setText("Abort");
            buttonIncremental_Update->setEnabled(true);
            progressBar_Incremental->setMaximum(lIncrementalUpdateNb);
            mCurrentProgressBar = NULL;
        }
        else
            mCurrentProgressBar = progressBar_Incremental;

        connect(m_TdrDbPlugin, SIGNAL(sLogRichMessage(QString,bool)), this, SLOT(IncrementalUpdateLog(QString,bool)));
        connect( m_pTdrDatabaseEntry, &GexDatabaseEntry::sLogRichMessage, this, &GexSqlHousekeepingDialog::IncrementalUpdateLog );

        while(true)
        {
            // Get the list of next incremental updates to do
            bool lStatus = true;
            QMap< QString,QMap< QString,QStringList > > lIncrementalUpdatesList;
            m_pTdrDatabaseEntry->m_pExternalDatabase->GetFirstIncrementalUpdatesList(strIncrementalName, lIncrementalUpdatesList);
            if(lIncrementalUpdatesList.isEmpty())
                break;

            // Execute the first of each db_update_name
            foreach(const QString lIncrementalName, lIncrementalUpdatesList.keys())
            {
                foreach(const QString lTestingStage, lIncrementalUpdatesList[lIncrementalName].keys())
                {
                    QString lTarget = lIncrementalUpdatesList[lIncrementalName][lTestingStage].takeFirst();
                    lSummary.clear();
                    lStatus = m_pTdrDatabaseEntry->IncrementalUpdate(lIncrementalName, lTestingStage, lTarget, lSummary);

                    m_pTdrDatabaseEntry->TraceUpdate("HOUSEKEEPING","INFO","Run Incremental Update: "+
                                                     GS::Gex::Engine::GetInstance().GetAdminEngine().JoinCommand(lSummary));
                    if(!lStatus)
                        break;
                }
                if(!lStatus)
                    break;
            }

            if(!lStatus)
                break;
            if(!strIncrementalName.isEmpty())
                break;
            if(!bUpdateAll)
                break;
            if(m_bAbortRequested)
                break;

            // Count was updated by the last Incremental Update process
            m_TdrDbPlugin->GetIncrementalUpdatesCount(false, lIncrementalUpdateNb);
            progressBar_Incremental->setValue(progressBar_Incremental->maximum()-lIncrementalUpdateNb);

            if(lIncrementalUpdateNb == 0)
                break;

            LoadIncrementalSettings();
        }
        disconnect(m_TdrDbPlugin, SIGNAL(sLogRichMessage(QString,bool)), this, SLOT(IncrementalUpdateLog(QString,bool)));
        disconnect( m_pTdrDatabaseEntry, &GexDatabaseEntry::sLogRichMessage, this, &GexSqlHousekeepingDialog::IncrementalUpdateLog );

        progressBar_Incremental->hide();
        m_bUpdateProcessRunning = false;
        m_bAbortRequested = false;
        buttonIncremental_Update->setText("Run");
    }

    OnDatabaseRefresh();

    // Update UI
    buttonClose->setEnabled(true);
    buttonIncremental_Check->setEnabled(true);
}

void GexSqlHousekeepingDialog::OnButtonIncremental_Check()
{
    // Clear log
    editLog_Incremental->clear();
    progressBar_Incremental->reset();

    // Update UI
    buttonIncremental_Check->setEnabled(false);
    buttonIncremental_Update->setEnabled(false);
    buttonClose->setEnabled(false);

    // Get database ptr
    if(m_pTdrDatabaseEntry->m_pExternalDatabase)
    {
        GexDbPlugin_ID	*pPlugin;
        pPlugin = m_pTdrDatabaseEntry->m_pExternalDatabase->GetPluginID();
        if(pPlugin)
        {
            // Update DB
            GexDbPlugin_Query clQuery(m_TdrDbPlugin, QSqlDatabase::database(m_TdrDbPlugin->m_pclDatabaseConnector->m_strConnectionName));
            QString lQuery;

            m_bUpdateProcessRunning = true;
            progressBar_Incremental->show();
            // Clean the incremental_update column when the valid_splitlots='N'
            // filter also on insertion_update to not affect current insertion
            // if the user was able to open an housekeeping GUI at the same time !
            int nTestingStage;
            QString lTableName;
            for(nTestingStage=0; nTestingStage<3; nTestingStage++)
            {
                switch(nTestingStage)
                {
                case 0 :
                    // FOR WAFER TEST
                    lTableName = "WT_SPLITLOT";
                    break;
                case 1 :
                    // FOR ELECT TEST
                    lTableName = "ET_SPLITLOT";
                    break;
                case 2 :
                    // FOR FINAL TEST
                    lTableName = "FT_SPLITLOT";
                    break;
                }

                lQuery = "UPDATE "+lTableName;
                lQuery+= " SET INCREMENTAL_UPDATE=null";
                lQuery+= " WHERE VALID_SPLITLOT='N' ";
                lQuery+= " AND INSERTION_TIME<UNIX_TIMESTAMP(SUBDATE(now(),1))";
                clQuery.exec(lQuery);
            }

            m_pTdrDatabaseEntry->TraceUpdate("HOUSEKEEPING","INFO","Start running Incremental Update for CheckIncremental");
            connect(m_TdrDbPlugin, SIGNAL(sLogRichMessage(QString,bool)), this, SLOT(IncrementalUpdateLog(QString,bool)));
            int lCount;
            bool bStatus = m_pTdrDatabaseEntry->m_pExternalDatabase->GetIncrementalUpdatesCount(true,lCount);
            disconnect(m_TdrDbPlugin, SIGNAL(sLogRichMessage(QString,bool)), this, SLOT(IncrementalUpdateLog(QString,bool)));
            if(!bStatus)
                m_pTdrDatabaseEntry->TraceUpdate("HOUSEKEEPING","INFO","Start running Incremental Update for CheckIncremental error: Update failed");
            progressBar_Incremental->hide();
            m_bUpdateProcessRunning = false;
        }
    }

    OnDatabaseRefresh();

    // Update UI
    buttonClose->setEnabled(true);
    buttonIncremental_Check->setEnabled(true);
}

void GexSqlHousekeepingDialog::OnButtonIncremental_Apply()
{
    // Clear log
    editLog_Incremental->clear();
    progressBar_Incremental->reset();

    // Update UI
    // Get database ptr
    if(m_pTdrDatabaseEntry->m_pExternalDatabase)
    {
        GexDbPlugin_ID	*pPlugin;
        pPlugin = m_pTdrDatabaseEntry->m_pExternalDatabase->GetPluginID();
        if(pPlugin)
        {
            int             lRow, lCol;
            QString         lIncrementalName;
            QStringList     lPropertyNames;
            lPropertyNames << "db_update_name" << "processed_splitlots" << "remaining_splitlots" << "status" << "frequency" << "max_items" << "last_schedule" << "last_execution" << "db_update_name_description ";

            // Get incremental updates
            QMap< QString,QMap< QString,QString > >		lIncrementalUpdates;
            for(lRow=0;lRow<tableIncrementalUpdates->rowCount();++lRow)
            {
                lIncrementalName = tableIncrementalUpdates->item(lRow,0)->text();
                for(lCol=0;lCol<lPropertyNames.count();++lCol)
                    lIncrementalUpdates[lIncrementalName][lPropertyNames.at(lCol)]=tableIncrementalUpdates->item(lRow,lCol)->text();
            }
            // Update DB
            m_pTdrDatabaseEntry->TraceUpdate("HOUSEKEEPING","INFO","Save Incremental Update settings");
            connect(m_TdrDbPlugin, SIGNAL(sLogRichMessage(QString,bool)), this, SLOT(IncrementalUpdateLog(QString,bool)));
            bool bStatus = m_pTdrDatabaseEntry->m_pExternalDatabase->SetIncrementalUpdatesSettings(lIncrementalUpdates);
            disconnect(m_TdrDbPlugin, SIGNAL(sLogRichMessage(QString,bool)), this, SLOT(IncrementalUpdateLog(QString,bool)));
            if(!bStatus)
                m_pTdrDatabaseEntry->TraceUpdate("HOUSEKEEPING","INFO","Save Incremental Update settings error: Update failed");
        }
    }
    LoadIncrementalSettings();
}

void GexSqlHousekeepingDialog::OnButtonIncremental_Cancel()
{
    // Clear log
    editLog_Incremental->clear();
    progressBar_Incremental->reset();

    LoadIncrementalSettings();
}
void GexSqlHousekeepingDialog::OnItemSelected(int nItem)
{
    // Ignore this command when a process running
    if(m_bUpdateProcessRunning)
        return;

    CancelChangeRequested();

    if(nItem < listSelection->count())
    {
        m_nCurrentSection = nItem;
        if (nItem == UPDATE_ITEM_INDEX)
        {
            mCurrentProgressBar = progressBar_Update;
        }
        else if (nItem == INCREMENTAL_ITEM_INDEX)
        {
            mCurrentProgressBar = progressBar_Incremental;
        }
        else if (nItem == GLOBAL_SETTINGS_ITEM_INDEX)
        {
            mCurrentProgressBar = progressBar_OptionSettings;
        }
        else if (nItem == METADATA_ITEM_INDEX)
        {
            mCurrentProgressBar = progressBar_MetaData;
        }
        else if (nItem == CONSOLIDATION_ITEM_INDEX)
        {
            if (widgetStack->widget(nItem) == NULL)
            {
                QWidget * ctWidget = m_TdrDbPlugin->GetConsolidationWidget();

                if (ctWidget == NULL)
                {
                    GSLOG(SYSLOG_SEV_WARNING, "This plugin does not support consolidation widget");
                    ctWidget = new QLabel("This DB plugin does not support consolidation GUI.", this);
                }

                int idx = widgetStack->addWidget(ctWidget);
                GSLOG(SYSLOG_SEV_INFORMATIONAL,
                      QString("added Consolidation widget to HouseKeeping widget stack at position %1")
                      .arg( idx)
                      .toLatin1().constData());
                m_pTdrDatabaseEntry->TraceUpdate("HOUSEKEEPING","INFO","View Consolidation Tree");
            }
            mCurrentProgressBar = NULL;
        }
        else if (nItem == PURGE_ITEM_INDEX)
        {
            if(purgeDBWidget->children().count() <= 1)
            {
                GexOneQueryWizardPage1 *poWidget = new GexOneQueryWizardPage1(0,false,0,true);
                purgeDBWidgetLayout->addWidget(poWidget, 0, 0, 1, 1);

                connect( poWidget, SIGNAL(sExecuteAction()),
                         this, SLOT(OnPurgeDataBase()));
                poWidget->comboBoxDatabaseName->addItem(editDatabaseName->text());
                poWidget->OnDatabaseChanged(true);
                poWidget->mExtractionModeLineEdit->hide();

                QString strDB = editDatabaseName->text();
                if(strDB.startsWith("[Local]") || strDB.startsWith("[Server]"))
                    strDB = strDB.section("]",1).trimmed();

                if(GS::Gex::Engine::GetInstance().GetDatabaseEngine().isYMDB(strDB)){
                    poWidget->TabWidgetFilters->setEnabled(false);
                }
            }
            mCurrentProgressBar = progressBar_DBPurge;
        }
        else if (nItem == BACKGROUND_TRANSFER_ITEM_INDEX)
        {
            editLog_Background->clear();
            mCurrentProgressBar = progressBar_Background;
            FillBackgroundTransferInformations();
        }
        else
        {
            mCurrentProgressBar = NULL;
        }
        widgetStack->setCurrentIndex(nItem);

        editLog_MetaData->clear();
        editLog_OptionSettings->clear();

        if (nItem == RELEASE_HISTORY_ITEM_INDEX)
        {
            UpdateDbSummaryDialog("ReleasesHistory",editLog_History);
        }
    }
    else
        GSLOG(SYSLOG_SEV_WARNING, QString("unknown item selected : %1").arg( nItem).toLatin1().constData());
}



void GexSqlHousekeepingDialog::FillBackgroundTransferInformations()
{
    tableBackgroundHistory->hide();
    buttonRefreshBackground->setEnabled(false);
    // Show the button of the background if the database has a background update process
    if(!(listSelection->item(BACKGROUND_TRANSFER_ITEM_INDEX)->flags() | Qt::ItemIsEnabled ))
    {
        QStringList lTables;
        m_TdrDbPlugin->m_pclDatabaseConnector->EnumTables(lTables);
        if(lTables.contains("background_transfer_history",Qt::CaseInsensitive))
        {
            GexDbPlugin_Query clQuery(m_TdrDbPlugin, QSqlDatabase::database(m_TdrDbPlugin->m_pclDatabaseConnector->m_strConnectionName));
            QString lQuery;
            lQuery = "SELECT update_id FROM background_transfer_history LIMIT 1";
            if(clQuery.Execute(lQuery) && clQuery.First())
            {
                listSelection->item(BACKGROUND_TRANSFER_ITEM_INDEX)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
            }
        }
    }

    if(listSelection->item(BACKGROUND_TRANSFER_ITEM_INDEX)->flags() | Qt::ItemIsEnabled )
    {
        GexDbPlugin_Query clQuery(m_TdrDbPlugin, QSqlDatabase::database(m_TdrDbPlugin->m_pclDatabaseConnector->m_strConnectionName));
        QString lQuery;
        int lCurrentUpdateId = 0;

        lQuery = "CALL background_update_id(@CurrentUpdateId)";
        if(clQuery.Execute(lQuery))
        {
            lQuery = "SELECT @CurrentUpdateId";
            if(clQuery.Execute(lQuery))
            {
                // A background process is running
                clQuery.First();
                lCurrentUpdateId = clQuery.value("@CurrentUpdateId").toInt();
            }
        }

        FillProgressInfo(lCurrentUpdateId);

        QSqlTableModel* lBackHistoryModel = dynamic_cast<QSqlTableModel*>(tableBackgroundHistory->model());
        if(lBackHistoryModel == NULL)
        {
            QSqlDatabase lDataBase = QSqlDatabase::database(m_TdrDbPlugin->m_pclDatabaseConnector->m_strConnectionName);
            lBackHistoryModel = new QSqlTableModel(NULL, lDataBase);
            lBackHistoryModel->setTable("background_transfer_history");
            tableBackgroundHistory->setSortingEnabled(true);
            tableBackgroundHistory->setSelectionBehavior(QAbstractItemView::SelectRows);
            tableBackgroundHistory->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
            tableBackgroundHistory->setModel(lBackHistoryModel);
            tableBackgroundHistory->setEditTriggers(QAbstractItemView::NoEditTriggers);
            lBackHistoryModel->setFilter("update_id>="+QString::number(lCurrentUpdateId));
            lBackHistoryModel->select();
        }
        else
            lBackHistoryModel->select();

        tableBackgroundHistory->resizeColumnsToContents();
        listSelection->item(BACKGROUND_TRANSFER_ITEM_INDEX)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

        tableBackgroundTables->hide();
        tableBackgroundPartitions->hide();

        // customize the display
        tableBackgroundHistory->verticalHeader()->hide();
        // tableBackgroundHistory->hideColumn(lBackHistoryModel->fieldIndex("update_id"));
        tableBackgroundHistory->hideColumn(lBackHistoryModel->fieldIndex("min_index"));
        tableBackgroundHistory->hideColumn(lBackHistoryModel->fieldIndex("max_index"));

        tableBackgroundHistory->setItemDelegateForColumn(lBackHistoryModel->fieldIndex("current_index"),
                                                         new BackgroundProgressDelegate(this));
        tableBackgroundHistory->setItemDelegateForColumn(lBackHistoryModel->fieldIndex("status"),
                                                         new BackgroundStatusDelegate(this));
        // Add the Background Transfer tables
        QSqlTableModel* lBackTransferTableModel = dynamic_cast<QSqlTableModel*>(tableBackgroundTables->model());
        if(lBackTransferTableModel == NULL)
        {
            QSqlDatabase lDataBase = QSqlDatabase::database(m_TdrDbPlugin->m_pclDatabaseConnector->m_strConnectionName);
            lBackTransferTableModel = new QSqlTableModel(NULL, lDataBase);
            lBackTransferTableModel->setTable("background_transfer_tables");
            lBackTransferTableModel->setHeaderData(lBackTransferTableModel->fieldIndex("current_index"),
                                                   Qt::Horizontal, QObject::tr("transfer_progress"));
            tableBackgroundTables->setSortingEnabled(true);
            tableBackgroundTables->setSelectionBehavior(QAbstractItemView::SelectRows);
            tableBackgroundTables->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
            tableBackgroundTables->setModel(lBackTransferTableModel);
            tableBackgroundTables->setEditTriggers(QAbstractItemView::NoEditTriggers);
            lBackTransferTableModel->setFilter("update_id>="+QString::number(lCurrentUpdateId));
            lBackTransferTableModel->select();
        }
        else
            lBackTransferTableModel->select();
        tableBackgroundTables->resizeColumnsToContents();
        if (mShowMoreBackgroundTableInfo)
        {
            buttonMoreLessTables->setIcon(QIcon(":/gex/icons/DownTriangle.png"));
            tableBackgroundTables->show();
        }
        else
        {
            buttonMoreLessTables->setIcon(QIcon(":/gex/icons/calendar_right.png"));
            tableBackgroundTables->hide();
        }

        // customize the display
        tableBackgroundTables->verticalHeader()->hide();
        // tableBackgroundTables->hideColumn(lBackTransferTableModel->fieldIndex("update_id"));
        tableBackgroundTables->hideColumn(lBackTransferTableModel->fieldIndex("min_index"));
        tableBackgroundTables->hideColumn(lBackTransferTableModel->fieldIndex("max_index"));

        tableBackgroundTables->setItemDelegateForColumn(lBackTransferTableModel->fieldIndex("current_index"),
                                                        new BackgroundProgressDelegate(this));
        tableBackgroundTables->setItemDelegateForColumn(lBackTransferTableModel->fieldIndex("status"),
                                                        new BackgroundStatusDelegate(this));

        // Add the Background Transfer partitions
        QSqlTableModel* lBackTransferPartitionsModel = dynamic_cast<QSqlTableModel*>(tableBackgroundPartitions->model());
        if(lBackTransferPartitionsModel == NULL)
        {
            QSqlDatabase lDataBase = QSqlDatabase::database(m_TdrDbPlugin->m_pclDatabaseConnector->m_strConnectionName);
            lBackTransferPartitionsModel = new QSqlTableModel(NULL, lDataBase);
            lBackTransferPartitionsModel->setTable("background_transfer_partitions");
            tableBackgroundPartitions->setSortingEnabled(true);
            tableBackgroundPartitions->setSelectionBehavior(QAbstractItemView::SelectRows);
            tableBackgroundPartitions->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
            tableBackgroundPartitions->setModel(lBackTransferPartitionsModel);
            tableBackgroundPartitions->setEditTriggers(QAbstractItemView::NoEditTriggers);
            lBackTransferPartitionsModel->setFilter("update_id>="+QString::number(lCurrentUpdateId));
            lBackTransferPartitionsModel->select();
        }
        else
            lBackTransferPartitionsModel->select();
        tableBackgroundPartitions->resizeColumnsToContents();
        if (mShowMoreBackgroundPartitionInfo)
        {
            tableBackgroundPartitions->show();
            buttonMoreLessPartitions->setIcon(QIcon(":/gex/icons/DownTriangle.png"));
        }
        else
        {
            tableBackgroundPartitions->hide();
            buttonMoreLessPartitions->setIcon(QIcon(":/gex/icons/calendar_right.png"));
        }

        tableBackgroundPartitions->verticalHeader()->hide();
        // tableBackgroundPartitions->hideColumn(lBackTransferPartitionsModel->fieldIndex("update_id"));
        tableBackgroundPartitions->hideColumn(lBackTransferPartitionsModel->fieldIndex("min_index"));
        tableBackgroundPartitions->hideColumn(lBackTransferPartitionsModel->fieldIndex("max_index"));
        tableBackgroundPartitions->setItemDelegateForColumn(lBackTransferPartitionsModel->fieldIndex("status"),
                                                            new BackgroundStatusDelegate(this));

        // Add the Background Transfer Logs
        QSqlTableModel* lBackTransferLogModel = dynamic_cast<QSqlTableModel*>(tableBackgroundLogs->model());
        if(lBackTransferLogModel == NULL)
        {
            QSqlDatabase lDataBase = QSqlDatabase::database(m_TdrDbPlugin->m_pclDatabaseConnector->m_strConnectionName);
            lBackTransferLogModel = new QSqlTableModel(NULL, lDataBase);
            lBackTransferLogModel->setTable("background_transfer_logs");
            tableBackgroundLogs->setSortingEnabled(true);
            tableBackgroundLogs->setSelectionBehavior(QAbstractItemView::SelectRows);
            tableBackgroundLogs->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
            tableBackgroundLogs->setModel(lBackTransferLogModel);
            tableBackgroundLogs->setEditTriggers(QAbstractItemView::NoEditTriggers);
            lBackTransferLogModel->setFilter("update_id>="+QString::number(lCurrentUpdateId)
     +" AND log_id>(SELECT (CASE WHEN max(log_id)<500 THEN 0 ELSE max(log_id)-500 END) FROM background_transfer_logs)");
            lBackTransferLogModel->select();
            tableBackgroundLogs->resizeColumnsToContents();
        }
        else
            lBackTransferLogModel->select();

        tableBackgroundLogs->scrollToBottom();
        if (mShowMoreBackgroundLogs)
        {
            buttonMoreLessLogs->setIcon(QIcon(":/gex/icons/DownTriangle.png"));
            tableBackgroundLogs->show();
        }
        else
        {
            buttonMoreLessLogs->setIcon(QIcon(":/gex/icons/calendar_right.png"));
            tableBackgroundLogs->hide();
        }

        // customize the display
        tableBackgroundLogs->verticalHeader()->hide();
        // tableBackgroundLogs->hideColumn(lBackTransferLogModel->fieldIndex("update_id"));

        tableBackgroundLogs->setItemDelegateForColumn(lBackTransferLogModel->fieldIndex("status"),
                                                      new BackgroundStatusDelegate(this));

        QString lMessage;
        editLog_Background->clear();
        // Initialize with some info
        QString lStartAt, lStopAt, lStatus;
        lStartAt = lBackHistoryModel->index(0,lBackHistoryModel->fieldIndex("start_time")).data().toString();
        lStopAt = lBackHistoryModel->index(0,lBackHistoryModel->fieldIndex("end_time")).data().toString();
        lStatus = lBackHistoryModel->index(0,lBackHistoryModel->fieldIndex("status")).data().toString();


        if(lStatus == "IN PROGRESS")
            lMessage = "o Background process IN PROGRESS:\n";
        else if(lStatus == "DONE")
            lMessage = "o Background process COMPLETED:\n";

        lMessage += QString(" - Start at %1\n - Last execution at %2\n\n")
                .arg(QDateTime::fromString(lStartAt, Qt::ISODate).toString(Qt::SystemLocaleShortDate))
                .arg(QDateTime::fromString(lStopAt, Qt::ISODate).toString(Qt::SystemLocaleShortDate));

        // UpdateLogMessage(editLog_Background,lMessage,true);

        // Show the progress bar
        if(lStatus != "DONE")
        {
            qint64 lMax = lBackHistoryModel->index(0,
                                                   lBackHistoryModel->fieldIndex("max_index")).data(Qt::DisplayRole).toLongLong();
            qint64 lMin = lBackHistoryModel->index(0,
                                                   lBackHistoryModel->fieldIndex("min_index")).data(Qt::DisplayRole).toLongLong();
            qint64 lCurrent = lBackHistoryModel->index(0,
                                                       lBackHistoryModel->fieldIndex("current_index")).data(Qt::DisplayRole).toLongLong();
            if( lCurrent == 0 )
                lCurrent = lMin;
            if(lMax == lMin)
                lMax++;

            int lPercent = (static_cast<double>(lMax - lCurrent) / (lMax - lMin)) * 100;
            if(progressBar_Background)
            {
                progressBar_Background->show();
                progressBar_Background->setValue(lPercent);
            }
        }
        else if(progressBar_Background)
            progressBar_Background->hide();

    }

    buttonRefreshBackground->setEnabled(true);
    UpdateStartStopBGEventButton();
}

void GexSqlHousekeepingDialog::FillProgressInfo(int currentUpdateId)
{
    // Fill progression info
    GexDbPlugin_Query lQuery(m_TdrDbPlugin, QSqlDatabase::database(
                                 m_TdrDbPlugin->m_pclDatabaseConnector->m_strConnectionName));
    QString lQueryStr, lETWhereClause, lFTWhereClause, lWTWhereClause;
    lETWhereClause = lFTWhereClause = lWTWhereClause = "";

    lQueryStr = "SELECT update_id, MIN(start_time) AS start_time, MAX(end_time) AS end_time FROM background_transfer_history WHERE update_id>="
            + QString::number(currentUpdateId) + " GROUP BY update_id";
    if(!lQuery.Execute(lQueryStr))
    {
        QString lMsg = "QUERY=" + lQueryStr + " ERROR=" + lQuery.lastError().text();
        GSLOG(SYSLOG_SEV_ERROR, lMsg.toLatin1().constData() );
        UpdateLogMessage(editLog_Background, lMsg.append("\n"), true);
        return;
    }

    QStringList lUpdateIds;
    while(lQuery.Next())
        lUpdateIds << lQuery.value("update_id").toString();

    int lUpdateId;
    int lTableRowCount = 0;
    QMap<QString, qint64> lCurrentIndexTS;
    QMap<QString, int> lInsertDate;
    QStringList lStages;
    lStages << "et" << "wt" << "ft";

    for (int lUpdateIndex = 0; lUpdateIndex< lUpdateIds.size(); ++lUpdateIndex)
    {
        lUpdateId = lUpdateIds.at(lUpdateIndex).toInt();
        if(currentUpdateId == lUpdateId)
        {
            labelBGStartTime->setText(QDateTime::fromString(lQuery.value("start_time").toString(), Qt::ISODate).toString(Qt::SystemLocaleShortDate));
            labelBGLastUpdate->setText(QDateTime::fromString(lQuery.value("end_time").toString(), Qt::ISODate).toString(Qt::SystemLocaleShortDate));
        }
        // et
        lQueryStr = "select max(current_index) as mindex, max(max_index) as startindex from background_transfer_tables where update_id="+QString::number(lUpdateId)+" and table_name like 'et%'";
        if (!lQuery.Execute(lQueryStr))
        {
            QString lMsg = "QUERY=" + lQueryStr + " ERROR=" + lQuery.lastError().text();
            GSLOG(SYSLOG_SEV_ERROR, lMsg.toLatin1().constData() );
            UpdateLogMessage(editLog_Background, lMsg.append("\n"), true);
            return;
        }

        if (lQuery.first())
        {
            lCurrentIndexTS.insert("et", lQuery.value("mindex").toLongLong());
            lCurrentIndexTS.insert("et_startindex", lQuery.value("startindex").toLongLong());
        }
        else
        {
            lCurrentIndexTS.insert("et", 0);
            lCurrentIndexTS.insert("et_startindex", 0);
        }

        // ft
        lQueryStr = "select max(current_index) as mindex, max(max_index) as startindex from background_transfer_tables where update_id="+QString::number(currentUpdateId)+" and table_name like 'ft%'";
        if (!lQuery.Execute(lQueryStr))
        {
            QString lMsg = "QUERY=" + lQueryStr + " ERROR=" + lQuery.lastError().text();
            GSLOG(SYSLOG_SEV_ERROR, lMsg.toLatin1().constData() );
            UpdateLogMessage(editLog_Background, lMsg.append("\n"), true);
            return;
        }

        if (lQuery.first())
        {
            lCurrentIndexTS.insert("ft", lQuery.value("mindex").toLongLong());
            lCurrentIndexTS.insert("ft_startindex", lQuery.value("startindex").toLongLong());
        }
        else
        {
            lCurrentIndexTS.insert("ft", 0);
            lCurrentIndexTS.insert("ft_startindex", 0);
        }

        // wt
        lQueryStr = "select max(current_index) as mindex, max(max_index) as startindex from background_transfer_tables where update_id="+QString::number(currentUpdateId)+" and table_name like 'wt%'";
        if (!lQuery.Execute(lQueryStr))
        {
            QString lMsg = "QUERY=" + lQueryStr + " ERROR=" + lQuery.lastError().text();
            GSLOG(SYSLOG_SEV_ERROR, lMsg.toLatin1().constData() );
            UpdateLogMessage(editLog_Background, lMsg.append("\n"), true);
            return;
        }

        if (lQuery.first())
        {
            lCurrentIndexTS.insert("wt", lQuery.value("mindex").toLongLong());
            lCurrentIndexTS.insert("wt_startindex", lQuery.value("startindex").toLongLong());
        }
        else
        {
            lCurrentIndexTS.insert("wt", 0);
            lCurrentIndexTS.insert("wt_startindex", 0);
        }

        if(currentUpdateId == lUpdateId)
        {
            if (lCurrentIndexTS["et"] != 0)
                lETWhereClause = "where splitlot_id<=" + QString::number(lCurrentIndexTS["et"]) + " ";
            if (lCurrentIndexTS["ft"] != 0)
                lFTWhereClause = "where splitlot_id<=" + QString::number(lCurrentIndexTS["ft"]) + " ";
            if (lCurrentIndexTS["wt"] != 0)
                lWTWhereClause = "where splitlot_id<=" + QString::number(lCurrentIndexTS["wt"]) + " ";

            // Get first and current
            lQueryStr ="select 'et' as ts,"
                       "min(insertion_time) as first_insert, "
                       "max(insertion_time) as current "
                       "from et_splitlot "
                    + lETWhereClause +
                    "union "
                    "select 'wt' as ts, "
                    "min(insertion_time) as first_insert, "
                    "max(insertion_time) as current "
                    "from wt_splitlot "
                    + lWTWhereClause +
                    "union "
                    "select 'ft' as ts, "
                    "min(insertion_time) as first_insert, "
                    "max(insertion_time) as current "
                    "from ft_splitlot "
                    + lFTWhereClause;

            if (!lQuery.Execute(lQueryStr))
            {
                QString lMsg = "QUERY=" + lQueryStr + " ERROR=" + lQuery.lastError().text();
                GSLOG(SYSLOG_SEV_ERROR, lMsg.toLatin1().constData() );
                UpdateLogMessage(editLog_Background, lMsg.append("\n"), true);
                return;
            }

            while (lQuery.next())
            {
                QString lTestingStage = lQuery.value("ts").toString();
                lInsertDate.insert(lTestingStage + "first",lQuery.value("first_insert").toInt());
                lInsertDate.insert(lTestingStage + "current",lQuery.value("current").toInt());
            }

            // Get max
            if (lCurrentIndexTS["et"] != 0)
                lETWhereClause = " where splitlot_id<=" + QString::number(lCurrentIndexTS["et_startindex"]) + " ";
            if (lCurrentIndexTS["ft"] != 0)
                lFTWhereClause = " where splitlot_id<=" + QString::number(lCurrentIndexTS["ft_startindex"]) + " ";
            if (lCurrentIndexTS["wt"] != 0)
                lWTWhereClause = " where splitlot_id<=" + QString::number(lCurrentIndexTS["wt_startindex"]) + " ";

            lQueryStr ="select 'et' as ts, "
                       " max(insertion_time) as last_insert "
                       " from et_splitlot "
                        + lETWhereClause +
                       " union "
                       "select 'wt' as ts, "
                       " max(insertion_time) as last_insert "
                       " from wt_splitlot "
                        + lWTWhereClause +
                       " union "
                       "select 'ft' as ts, "
                       " max(insertion_time) as last_insert "
                       " from ft_splitlot"
                        + lFTWhereClause;
            if (!lQuery.Execute(lQueryStr))
            {
                QString lMsg = "QUERY=" + lQueryStr + " ERROR=" + lQuery.lastError().text();
                GSLOG(SYSLOG_SEV_ERROR, lMsg.toLatin1().constData() );
                UpdateLogMessage(editLog_Background, lMsg.append("\n"), true);
                return;
            }

            while (lQuery.next())
            {
                QString lTestingStage = lQuery.value("ts").toString();
                lInsertDate.insert(lTestingStage + "last",lQuery.value("last_insert").toInt());
            }
        }

        // Fill table
        for (int lTSIndex = 0; lTSIndex< lStages.size(); ++lTSIndex)
        {
            QString lTestingStage = lStages.at(lTSIndex);

            tableWidgetTSProgress->setRowCount(lTableRowCount+1);
            tableWidgetTSProgress->setItem(lTableRowCount, 0, new QTableWidgetItem(QString::number(lUpdateId)));
            tableWidgetTSProgress->setItem(lTableRowCount, 1, new QTableWidgetItem(lTestingStage));
            int lDaysTo, lDaysDone, lProgressPercent = -1;
            // Get number of day between min and max
            lDaysTo = QDateTime::fromTime_t(lInsertDate[lTestingStage + "first"]).daysTo(QDateTime::fromTime_t(lInsertDate[lTestingStage + "last"])) + 1;
            lDaysDone =  lDaysTo - QDateTime::fromTime_t(lInsertDate[lTestingStage + "first"]).daysTo(QDateTime::fromTime_t(lInsertDate[lTestingStage + "current"])) - 1;
            if (lInsertDate[lTestingStage + "first"] == 0)
            {
                tableWidgetTSProgress->setItem(lTableRowCount, 2, new QTableWidgetItem("--"));
                tableWidgetTSProgress->setItem(lTableRowCount, 3, new QTableWidgetItem("--"));
                tableWidgetTSProgress->setItem(lTableRowCount, 4, new QTableWidgetItem("no data"));
                lTableRowCount++;
                continue;
            }

            // if finished
            if (lCurrentIndexTS[lTestingStage] == 0)
            {
                lProgressPercent = 100;
                lDaysDone = lDaysTo;
            }
            // if 1 day but not finished or not even 1 day copied
            else if (((lDaysTo == 1) && (lCurrentIndexTS[lTestingStage] != 0)) || (lDaysDone == 0))
                lProgressPercent = 0;
            else
                lProgressPercent = 100.0 * static_cast<double>(lDaysDone) / lDaysTo;
            if(currentUpdateId != lUpdateId)
                lProgressPercent = 0;

            tableWidgetTSProgress->setItem(lTableRowCount, 2, new QTableWidgetItem(
                                               QDateTime::fromTime_t(lInsertDate[lTestingStage + "first"])
                                           .toString(Qt::SystemLocaleShortDate)));
            tableWidgetTSProgress->setItem(lTableRowCount, 3, new QTableWidgetItem(
                                               QDateTime::fromTime_t(lInsertDate[lTestingStage + "last"])
                                           .toString(Qt::SystemLocaleShortDate)));
            QString lProgress = QString("%1/%2 (%3%)").arg(QString::number(lDaysDone))
                    .arg(QString::number(lDaysTo))
                    .arg(QString::number(lProgressPercent));
            tableWidgetTSProgress->setItem(lTableRowCount, 4, new QTableWidgetItem(lProgress));

            lTableRowCount++;
        }
    }
    tableWidgetTSProgress->resizeColumnsToContents();
}



void GexSqlHousekeepingDialog::OnButtonBackgroundMoreInfo()
{
    mShowMoreBackgroundTableInfo = !mShowMoreBackgroundTableInfo;
    if (mShowMoreBackgroundTableInfo)
    {
        tableBackgroundTables->show();
        buttonMoreLessTables->setIcon(QIcon(":/gex/icons/DownTriangle.png"));
    }
    else
    {
        tableBackgroundTables->hide();
        buttonMoreLessTables->setIcon(QIcon(":/gex/icons/calendar_right.png"));
    }
}


void GexSqlHousekeepingDialog::OnButtonBackgroundMorePartitionInfo()
{
    mShowMoreBackgroundPartitionInfo = !mShowMoreBackgroundPartitionInfo;
    if (mShowMoreBackgroundPartitionInfo)
    {
        tableBackgroundPartitions->show();
        buttonMoreLessPartitions->setIcon(QIcon(":/gex/icons/DownTriangle.png"));

    }
    else
    {
        tableBackgroundPartitions->hide();
        buttonMoreLessPartitions->setIcon(QIcon(":/gex/icons/calendar_right.png"));
    }
}

void GexSqlHousekeepingDialog::OnButtonBackgroundMoreLogs()
{
    mShowMoreBackgroundLogs = !mShowMoreBackgroundLogs;
    if (mShowMoreBackgroundLogs)
    {
        tableBackgroundLogs->show();
        buttonMoreLessLogs->setIcon(QIcon(":/gex/icons/DownTriangle.png"));

    }
    else
    {
        tableBackgroundLogs->hide();
        buttonMoreLessLogs->setIcon(QIcon(":/gex/icons/calendar_right.png"));
    }
}



///////////////////////////////////////////////////////////
// Update table meta-data with existing DB metedata_mapping
///////////////////////////////////////////////////////////
void GexSqlHousekeepingDialog::OnTestingStageSelected()
{
    if(m_strCurrentTestingStage.isEmpty())
        m_strCurrentTestingStage = comboBoxTestingStage->currentText();

    if(comboBoxTestingStage->isEnabled()
            && (m_strCurrentTestingStage != comboBoxTestingStage->currentText()))
        CancelChangeRequested();

    m_strCurrentTestingStage = comboBoxTestingStage->currentText();

    QString strValue;
    QString strTestingStage = m_strCurrentTestingStage;

    strTestingStage = strTestingStage.toLower().left(1) + "t_";

    buttonMetaData_Cancel->setEnabled(false);
    buttonMetaData_Apply->setEnabled(false);
    buttonMetaData_Apply->setText("Apply && Rebuild");

    if(buttonUpdate->isEnabled())
    {
        // Can force new/delete meta-data for update
        buttonMetaData_Apply->setEnabled(true);
        buttonMetaData_Apply->setText("Save");
    }

    // Get database ptr
    if(m_pTdrDatabaseEntry->m_pExternalDatabase
            && m_TdrDbPlugin
            && m_TdrDbPlugin->m_pclDatabaseConnector)
    {
        int         nRow, nCol;
        bool        lStaticMetaData;
        bool        lReadOnlyField;
        QString     strQuery, strToolTip;
        QStringList lstLabels;

        // Get the list of MetaData from this GexDb (Static and Dynamic)
        // To check if the MetaData already exists (for MetaData creation)
        QString lMetaName;
        QStringList lstValues;
        QMap<QString, QString> mapStaticMetaData;
        QMap<QString, QString> mapStaticMetaDataProperties;
        m_pTdrDatabaseEntry->m_pExternalDatabase->GetRdbFieldsList(m_strCurrentTestingStage,lstValues,"*","*","*","*","*","*",true);
        // For case insensitive
        while(!lstValues.isEmpty())
        {
            lMetaName = lstValues.takeFirst();
            mapStaticMetaData[lMetaName.toUpper()] = lMetaName;
        }

        tableMetaData->clear();
        tableMetaData->setRowCount(0);
        tableMetaData->disconnect();

        GexDbPlugin_Query clQuery(m_TdrDbPlugin,
                                  QSqlDatabase::database(m_TdrDbPlugin->m_pclDatabaseConnector->m_strConnectionName));

        // Update the header label with the DB metadata_mapping column name
        if(m_TdrDbPlugin->m_pclDatabaseConnector->IsOracleDB())
            strQuery = "SELECT LOWER(column_name) FROM user_tab_columns WHERE LOWER(table_name)='"+strTestingStage+"metadata_mapping' ORDER BY column_id";
        else
            strQuery = "DESCRIBE "+strTestingStage+"metadata_mapping";
        if(!clQuery.Execute(strQuery))
        {
            editLog_MetaData->insertHtml("<BR>\nQUERY=");
            editLog_MetaData->insertHtml("<BR>"+strQuery);
            editLog_MetaData->insertHtml("<BR>\nERROR=");
            editLog_MetaData->insertHtml("<BR>"+clQuery.lastError().text().replace("<","&lt;").replace("\n","<BR>"));
            editLog_MetaData->insertHtml("<BR>\n");
            editLog_MetaData->insertHtml("<BR><b>Status = FAIL.</b>");
            return;
        }

        int lCol = 0;
        QStringList lConsolidatedCol;
        while(clQuery.next())
        {
            strValue = clQuery.value(0).toString().toLower();
            if(strValue == "consolidated_field")
                lConsolidatedCol << QString::number(lCol);
            if(strValue == "az_field")
                lConsolidatedCol << QString::number(lCol);

            lstLabels << strValue;

            ++lCol;
        }

        tableMetaData->setColumnCount(lstLabels.count());
        tableMetaData->setHorizontalHeaderLabels(lstLabels);

        QString strEngine,strFormat;
        m_TdrDbPlugin->GetStorageEngineName(strEngine,strFormat);
        if(strEngine.toUpper() == "SPIDER")
        {
            foreach(QString sCol, lConsolidatedCol)
                tableMetaData->hideColumn(sCol.toInt());
            }

        // Then extract current values
        strQuery = "SELECT "+lstLabels.join(",")+" FROM "+strTestingStage+"metadata_mapping";
        if(!clQuery.Execute(strQuery))
        {
            editLog_MetaData->insertHtml("<BR>\nQUERY=");
            editLog_MetaData->insertHtml("<BR>"+strQuery);
            editLog_MetaData->insertHtml("<BR>\nERROR=");
            editLog_MetaData->insertHtml("<BR>"+clQuery.lastError().text().replace("<","&lt;").replace("\n","<BR>"));
            editLog_MetaData->insertHtml("<BR>\n");
            editLog_MetaData->insertHtml("<BR><b>Status = FAIL.</b>");
        }
        nRow = nCol = 0;
        while(clQuery.next())
        {
            nCol = 0;
            lStaticMetaData = false;
            lReadOnlyField = false;
            mapStaticMetaDataProperties.clear();

            // Insert item into ListView
            tableMetaData->setRowCount(nRow+1);
            tableMetaData->setRowHeight(nRow, 20);

            for(nCol=0; nCol<lstLabels.count(); nCol++)
            {
                strToolTip = "";
                strValue = clQuery.value(nCol).toString();
                if(strValue == "n/a")
                    strValue = "";

                // For column 'meta_name'
                // Check if it is a Static MetaData
                // In this case, on the GUI can be updated
                if(nCol == 0)
                {
                    if(mapStaticMetaData.contains(strValue.toUpper()))
                    {
                        lReadOnlyField = true;
                        lStaticMetaData = true;
                        m_pTdrDatabaseEntry->m_pExternalDatabase->GetRdbFieldProperties(m_strCurrentTestingStage,
                                                                                        mapStaticMetaData[strValue.toUpper()],mapStaticMetaDataProperties);
                    }
                }
                else
                    lReadOnlyField = false;

                if(lStaticMetaData)
                {
                    strToolTip = "Gex Static Meta-Data for GUI update only";
                    // For all other properties than the GUI
                    // Use the Gex value
                    if(lstLabels.at(nCol).endsWith("_gui"))
                        lReadOnlyField = false;
                    else
                    {
                        lReadOnlyField = true;
                        strValue = mapStaticMetaDataProperties[lstLabels.at(nCol)];
                    }
                }

                // Update the tooltip with allowed values
                if(lstLabels.at(nCol) == "bintype_field")
                    strToolTip = "H, S or N (or even *)";
                else if(lstLabels.at(nCol) == "az_field")
                    strToolTip = "Y or N";
                else if(lstLabels.at(nCol).endsWith("_gui"))
                    strToolTip = "Y, N or *";
                else if(lstLabels.at(nCol).endsWith("_field"))
                    strToolTip = "Y, N or *";

                fillCellData(tableMetaData,nRow,nCol,strValue,strToolTip,lReadOnlyField);
            }
            nRow++;
        }
        QObject::connect(tableMetaData,			SIGNAL(cellChanged(int,int)),							this, SLOT(OnCellMetaData(int,int)));
        QObject::connect(tableMetaData,			SIGNAL(customContextMenuRequested ( const QPoint & )),	this,SLOT(OnTableMetaDataContextualMenu(const QPoint&)));
    }


}

///////////////////////////////////////////////////////////
// Table Cancel-click
///////////////////////////////////////////////////////////
void GexSqlHousekeepingDialog::OnButtonMetaData_Cancel()
{
    editLog_MetaData->insertHtml("<BR>\n");
    editLog_MetaData->insertHtml("<BR>CANCELED.");

    OnTestingStageSelected();
    buttonMetaData_Cancel->setEnabled(false);
}

///////////////////////////////////////////////////////////
// Table Apply-click
///////////////////////////////////////////////////////////
void GexSqlHousekeepingDialog::OnButtonMetaData_Apply()
{
    bool	bStatus;
    bool	bUpdateConsolidatedTable;
    QString strValue;
    QString strError;
    int eTestingStage;

    m_strCurrentTestingStage = comboBoxTestingStage->currentText();

    bStatus = true;
    bUpdateConsolidatedTable = false;

    // Testing Stage selected
    m_TdrDbPlugin->SetTestingStage(m_strCurrentTestingStage);
    eTestingStage = m_pTdrDatabaseEntry->m_pExternalDatabase->GetTestingStageEnum(m_strCurrentTestingStage);
    QString strPrefixTable = m_strCurrentTestingStage.left(1).toLower()+"t_";

    progressBar_MetaData->setMaximum(100);
    progressBar_MetaData->setValue(0);
    progressBar_MetaData->show();


    if(m_pTdrDatabaseEntry->m_pExternalDatabase
            && m_TdrDbPlugin
            && m_TdrDbPlugin->m_pclDatabaseConnector)
    {
        int                 nRow, nCol;
        int                 nNew,nUpdated,nDeleted,nIgnored;
        bool                bNew,bUpdated,bDeleted,bStaticMetaData;
        QString             strQuery;
        QStringList         lstLabels;
        QStringList         lstValues;
        QTableWidgetItem    *ptItem;

        m_pTdrDatabaseEntry->TraceUpdate("HOUSEKEEPING","INFO","Start updating MetaData ");

        editLog_MetaData->clear();
        editLog_MetaData->insertHtml("<BR><b>["+QDate::currentDate().toString(Qt::ISODate)+" "+QTime::currentTime().toString(Qt::ISODate)+"] Save new Meta-Data\n");
        editLog_MetaData->insertHtml("<BR>\n");
        editLog_MetaData->insertHtml("<BR><b>Database "+m_TdrDbPlugin->m_pclDatabaseConnector->m_strSchemaName+"\n");
        editLog_MetaData->insertHtml("<BR><b>Testing Stage "+m_strCurrentTestingStage+"\n");

        bUpdateConsolidatedTable = false;
        nNew = nUpdated = nDeleted = nIgnored = 0;
        bNew = bUpdated = bDeleted = bStaticMetaData =false;

        GexDbPlugin_Query	clQuery(m_TdrDbPlugin, QSqlDatabase::database(m_TdrDbPlugin->m_pclDatabaseConnector->m_strConnectionName));

        // Get the list of MetaData from this GexDb (Static and Dynamic)
        // To check if the MetaData already exists (for MetaData creation)
        QStringList listStaticMetaData, listUserMetaData;
        m_pTdrDatabaseEntry->m_pExternalDatabase->GetRdbFieldsList(m_strCurrentTestingStage,lstValues,"*","*","*","*","*","*",true);
        // For case insensitive
        while(!lstValues.isEmpty())
            listStaticMetaData.append(lstValues.takeFirst().toUpper());

        // Elimnate entries from User table
        strQuery = "SELECT UPPER(meta_name) FROM "+strPrefixTable+"metadata_mapping";
        if(!clQuery.Execute(strQuery))
        {
            strError="\nQUERY=";
            strError+=strQuery;
            strError+="\nERROR=";
            strError+=clQuery.lastError().text();
            bStatus = false;
            goto labelExit;
        }
        while(clQuery.Next())
        {
            if(listStaticMetaData.contains(clQuery.value(0).toString().toUpper()))
                continue;
            listUserMetaData.append(clQuery.value(0).toString().toUpper());
        }

        // Extract new value from tableMetaData
        strQuery = "";
        for(nCol=0;nCol<tableMetaData->columnCount();nCol++)
        {
            ptItem = tableMetaData->horizontalHeaderItem(nCol);
            if(ptItem)
                lstLabels << ptItem->text();
            else
                lstLabels << "";
        }

        nRow = nCol = 0;
        for(nRow=0;nRow<tableMetaData->rowCount();nRow++)
        {
            strValue = "";
            bNew = bUpdated = bDeleted = bStaticMetaData = false;

            ptItem = tableMetaData->item(nRow, 0);

            // Check if line is correctly updated
            if(	ptItem
                    && (ptItem->text() == "NewMetaData"))
            {
                editLog_MetaData->insertHtml("<BR>\n");
                editLog_MetaData->insertHtml("<BR><b>New Meta-Data '"+ptItem->text().replace("<","&lt;")+"' will be ignored.\n");
                editLog_MetaData->insertHtml("<BR>Reserved Meta-Data name!\n");
                editLog_MetaData->insertHtml("<BR>Change your Meta-Data name.\n");
                editLog_MetaData->insertHtml("<BR>\n");
                nIgnored++;
                continue;
            }

            if( ptItem
                    && (ptItem->backgroundColor() == QColor(255, 152, 152)))
                bDeleted = true;
            else if( ptItem
                     && (ptItem->backgroundColor() == QColor(0,198,0)))
            {
                // This is a new MetaData
                if(listStaticMetaData.contains(ptItem->text().toUpper()))
                {
                    // This MetaData already uploaded
                    // Check if it is a Static MetaData
                    editLog_MetaData->insertHtml("<BR>\n");
                    editLog_MetaData->insertHtml("<BR><b>New Meta-Data '"+ptItem->text().replace("<","&lt;")+"' is a Reserved Meta-Data name.\n");
                    editLog_MetaData->insertHtml("<BR>Only GUI settings can be updated!\n");
                    editLog_MetaData->insertHtml("<BR>\n");
                    bStaticMetaData = true;
                    // nIgnored++;
                    // continue;
                }
                if(listUserMetaData.contains(ptItem->text().toUpper()))
                {
                    // This MetaData already uploaded
                    // Check if it is a Static MetaData
                    editLog_MetaData->insertHtml("<BR>\n");
                    editLog_MetaData->insertHtml("<BR><b>New Meta-Data '"+ptItem->text().replace("<","&lt;")+"' will be ignored.\n");
                    editLog_MetaData->insertHtml("<BR>This Meta-Data name already exists!\n");
                    editLog_MetaData->insertHtml("<BR>Change your Meta-Data name.\n");
                    editLog_MetaData->insertHtml("<BR>\n");
                    nIgnored++;
                    continue;
                }
                bNew = true;
            }

            // For each column, extract value
            for(nCol=0;nCol<tableMetaData->columnCount();nCol++)
            {
                ptItem = tableMetaData->item(nRow, nCol);

                // Update consolidated table when 'consolidated_field' changed
                if(!bStaticMetaData
                        && (lstLabels.at(nCol) == "consolidated_field")
                        &&  ptItem
                        && (ptItem->backgroundColor() == QColor(0,198,0)))
                {
                    // Check if it it a new line
                    // then only update if 'Y'
                    if(tableMetaData->item(nRow, 0)->backgroundColor() != QColor(0,198,0))
                        bUpdateConsolidatedTable = true;
                    else if (ptItem->text().toUpper() == "Y")
                        bUpdateConsolidatedTable = true;
                    }

                // Update consolidated table when 'linked field' changed with 'consolidated_field' ='Y'
                if(!bStaticMetaData
                        && (lstLabels.at(nCol) == "consolidated_field")
                        &&  ptItem
                        && (ptItem->backgroundColor() != QColor(0,198,0))
                        && (ptItem->backgroundColor() != QColor(255, 152, 152))
                        && (ptItem->text().toUpper() == "Y"))
                {
                    // Check if it linked fields changed
                    if( tableMetaData->item(nRow, 0) // meta_name
                            && (tableMetaData->item(nRow, 0)->backgroundColor() == QColor(0,198,0)))
                        bUpdateConsolidatedTable = true;
                    if( tableMetaData->item(nRow, 1) // gex_name
                            && (tableMetaData->item(nRow, 1)->backgroundColor() == QColor(0,198,0)))
                        bUpdateConsolidatedTable = true;
                    if( tableMetaData->item(nRow, 2) // gexdb_table_name
                            && (tableMetaData->item(nRow, 2)->backgroundColor() == QColor(0,198,0)))
                        bUpdateConsolidatedTable = true;
                    if( tableMetaData->item(nRow, 3) // gexdb_field_fullname
                            && (tableMetaData->item(nRow, 3)->backgroundColor() == QColor(0,198,0)))
                        bUpdateConsolidatedTable = true;
                    if( tableMetaData->item(nRow, 4) // gexdb_link_name
                            && (tableMetaData->item(nRow, 4)->backgroundColor() == QColor(0,198,0)))
                        bUpdateConsolidatedTable = true;
                    }

                // Update consolidated table when 'consolidated_field' deleted and was 'Y'
                if(!bStaticMetaData
                        && (lstLabels.at(nCol) == "consolidated_field")
                        &&  ptItem
                        && (ptItem->backgroundColor() == QColor(255, 152, 152))
                        && (ptItem->text().toUpper() == "Y"))
                    bUpdateConsolidatedTable = true;

                // Then save value for valid lines
                if( ptItem
                        && (ptItem->backgroundColor() == QColor(255, 152, 152)))
                    continue;

                if( ptItem
                        && (ptItem->backgroundColor() == QColor(0,198,0)))
                    bUpdated = true;

                if(!strValue.isEmpty())
                    strValue += ",";

                if(ptItem == NULL)
                    strValue += "''";
                else
                    strValue += "'"+ptItem->text()+"'";
                }
            if(bDeleted)
                nDeleted++;
            else
                if(bNew)
                nNew++;
                else
                    if(bUpdated)
                nUpdated++;

            if(strValue.isEmpty())
                continue;

            lstValues << strValue;

        }

        if(bUpdateConsolidatedTable)
        {
            // If the DB is not up to date
            // force rebuild
            // If the DB Build is not up-to-date
            // The consolidation process only add a new action
            if(!buttonUpdate->isEnabled())
            {

                // Have to update consolidated table
                // Check if the user really want to do this
                if(!clQuery.Execute("SELECT count(*) from "+strPrefixTable+"splitlot"))
                {
                    strError="\nQUERY=";
                    strError+="SELECT count(*) from "+strPrefixTable+"splitlot";
                    strError+="\nERROR=";
                    strError+=clQuery.lastError().text();
                    bStatus = false;
                    goto labelExit;
                }
                clQuery.first();
                nUpdated = clQuery.value(0).toInt();

                QMessageBox msgBox(this);

                if (pGexSkin)
                    pGexSkin->applyPalette(&msgBox);

                QString strMessage = "Your GexDB "+strPrefixTable.left(2).toUpper();
                strMessage+= " consolidated table needs to be re-generated due to your changes in the meta-data mapping.\n";
                strMessage+= QString::number(nUpdated)+" splitlots will be processed and consolidated to populate this table.\n";
                strMessage+= "This may take several minutes to a few hours depending on the size of your splitlot table and the performance of your database server.\n";
                strMessage+= "\nDo you confirm ? ";

                msgBox.setWindowTitle("Updating Consolidated Table");
                msgBox.setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));
                msgBox.setText(strMessage);
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
                msgBox.setDefaultButton(QMessageBox::Cancel);

                msgBox.setButtonText( QMessageBox::Yes, "Update" );
                msgBox.setButtonText( QMessageBox::Cancel, "&Cancel" );

                // If not, do nothing
                if(msgBox.exec() == QMessageBox::Cancel)
                {
                    editLog_OptionSettings->insertPlainText(" * 0 splitlot updated\n");
                    goto labelExit;
                }
            }
            else
            {
                // Database is not up-to-date
                // Consolidation process will be done during the last B17 update
                bUpdateConsolidatedTable = false;
                bStatus = true;
            }
        }

        // Delete old entries
        strQuery = "DELETE FROM "+strPrefixTable+"metadata_mapping";
        if(!clQuery.Execute(strQuery))
        {
            strError="\nQUERY=";
            strError+=strQuery;
            strError+="\nERROR=";
            strError+=clQuery.lastError().text();
            bStatus = false;
            goto labelExit;
        }

        // Update metadata_mapping table
        while(!lstValues.isEmpty())
        {
            strValue = lstValues.takeFirst();
            strQuery = "INSERT INTO "+strPrefixTable+"metadata_mapping("+lstLabels.join(",")+") VALUES(";
            strQuery+= strValue+")";

            if(!clQuery.Execute(strQuery))
            {
                strError="\nQUERY=";
                strError+=strQuery;
                strError+="\nERROR=";
                strError+=clQuery.lastError().text();
                bUpdateConsolidatedTable = false;
                bStatus = false;
                goto labelExit;
            }
        }

        if(nIgnored > 0)
            editLog_MetaData->insertHtml("<BR>"+QString::number(nIgnored)+" Meta-Data ignored\n");
        if(nNew > 0)
            editLog_MetaData->insertHtml("<BR>"+QString::number(nNew)+" Meta-Data added\n");
        if(nUpdated > 0)
            editLog_MetaData->insertHtml("<BR>"+QString::number(nUpdated)+" Meta-Data updated\n");
        if(nDeleted > 0)
            editLog_MetaData->insertHtml("<BR>"+QString::number(nDeleted)+" Meta-Data deleted\n");

        if(bUpdateConsolidatedTable)
        {
            // Have to update consolidated table
            // Check if the user really want to do this
            editLog_MetaData->insertHtml("<BR>\n");
            editLog_MetaData->insertHtml("<BR>Updating Consolidated Table.");

            m_bUpdateProcessRunning = true;
            m_pTdrDatabaseEntry->TraceUpdate("HOUSEKEEPING","INFO","Start running Consolidation Process for "+m_strCurrentTestingStage);
            connect(m_TdrDbPlugin, SIGNAL(sLogRichMessage(QString,bool)), this, SLOT(UpdateMetadataLog(QString,bool)));
            connect(m_TdrDbPlugin, SIGNAL(sUpdateProgress(int)), this, SLOT(UpdateProgress(int)));
            connect(m_TdrDbPlugin, SIGNAL(sResetProgress(bool)), this, SLOT(ResetProgress(bool)));
            connect(m_TdrDbPlugin, SIGNAL(sMaxProgress(int)), this, SLOT(SetMaxProgress(int)));
            bStatus = m_pTdrDatabaseEntry->m_pExternalDatabase->UpdateConsolidationProcess(eTestingStage);
            disconnect(m_TdrDbPlugin, SIGNAL(sLogRichMessage(QString,bool)), this, SLOT(UpdateMetadataLog(QString,bool)));
            disconnect(m_TdrDbPlugin, SIGNAL(sUpdateProgress(int)), this, SLOT(UpdateProgress(int)));
            disconnect(m_TdrDbPlugin, SIGNAL(sResetProgress(bool)), this, SLOT(ResetProgress(bool)));
            disconnect(m_TdrDbPlugin, SIGNAL(sMaxProgress(int)), this, SLOT(SetMaxProgress(int)));
            m_bUpdateProcessRunning = false;
            if(!bStatus)
            {
                m_pTdrDatabaseEntry->m_pExternalDatabase->GetLastError(strError);
                m_pTdrDatabaseEntry->TraceUpdate("HOUSEKEEPING","INFO","Start running Consolidation Process error: "+strError);
                goto labelExit;
            }
            editLog_OptionSettings->insertPlainText(" * "+QString::number(nUpdated)+" splitlots updated\n");
        }
        else
        {
            //editLog_MetaData->insertHtml("<BR>\n");
            //editLog_MetaData->insertHtml("<BR>Consolidated Process already updated.\n");
        }
    }

labelExit:

    progressBar_MetaData->hide();

    if(bStatus == false)
    {
        editLog_MetaData->insertHtml("<BR>\n");
        editLog_MetaData->insertHtml("<BR>"+strError.replace("<","&lt;").replace("\n","<BR>"));
        editLog_MetaData->insertHtml("<BR>\n");
        editLog_MetaData->insertHtml("<BR><b>Status = FAIL.</b>");
    }
    else
    {
        editLog_MetaData->insertHtml("<BR>\n");
        editLog_MetaData->insertHtml("<BR><b>Status = SUCCESS.</b>");

        buttonMetaData_Cancel->setEnabled(false);
        buttonMetaData_Apply->setEnabled(false);
        OnTestingStageSelected();
    }

    if(bUpdateConsolidatedTable)
    {
        // Create Log file
        QString			strLogFile;
        strLogFile = GS::Gex::Engine::GetInstance().Get("UserFolder").toString()
                + "/gexdb_updatedb_consolidated_process_";
        strLogFile += QDate::currentDate().toString(Qt::ISODate);
        strLogFile += ".log";
        CGexSystemUtils::NormalizePath(strLogFile);

        // Open log file
        FILE *pLogFile = fopen(strLogFile.toLatin1().data(), "w+");
        if(!pLogFile)
        {
            editLog_MetaData->insertHtml("<BR>\n");
            editLog_MetaData->insertHtml("<BR>Error writing to log file "+strLogFile+".\n");
        }
        else
        {
            fprintf(pLogFile, "%s",editLog_MetaData->toHtml().toLatin1().constData());
            fclose(pLogFile);
            editLog_MetaData->insertHtml("<BR>\n");
            editLog_MetaData->insertHtml("<BR>Update history saved to log file "+strLogFile+".\n");
        }

        buttonMetaData_Cancel->setEnabled(false);
        buttonMetaData_Apply->setEnabled(false);
        OnDatabaseRefresh();
        //comboBoxTestingStage->setCurrentText(m_strCurrentTestingStage);
        comboBoxTestingStage->setCurrentIndex(comboBoxTestingStage->findText(m_strCurrentTestingStage));
        OnTestingStageSelected();

    }

    if (m_TdrDbPlugin)
        m_TdrDbPlugin->LoadMetaData();

    editLog_MetaData->moveCursor(QTextCursor::End);
}

///////////////////////////////////////////////////////////
// Table cell updated
///////////////////////////////////////////////////////////
void GexSqlHousekeepingDialog::OnCellMetaData(int nRow,int nCol)
{
    QTableWidgetItem	*ptItem;
    ptItem = tableMetaData->item(nRow,nCol);

    if(ptItem == NULL)
        return;

    // Item value changed
    QString	strValue = ptItem->text();
    QString strToolTip = ptItem->toolTip();

    // check if have the good value (saved in the tooltip)
    if(!strToolTip.isEmpty())
    {
        strToolTip = strToolTip.remove("or").remove("even").remove(',').remove('(').remove(')').replace("*","\\*");
        strToolTip = strToolTip.simplified().replace(" ","|");
        QRegExp clRegExp("("+strToolTip+")",Qt::CaseInsensitive);

        if(!clRegExp.exactMatch(strValue))
        {
            // Value is incorrect
            // overwrite it with 'N'
            tableMetaData->disconnect();
            ptItem->setText("N");

        QObject::connect(tableMetaData,			SIGNAL(cellChanged(int,int)),							this, SLOT(OnCellMetaData(int,int)));
        QObject::connect(tableMetaData,			SIGNAL(customContextMenuRequested ( const QPoint & )),	this,SLOT(OnTableMetaDataContextualMenu(const QPoint&)));
    }

    }
    ptItem->setBackgroundColor(QColor(0,198,0));
    buttonMetaData_Cancel->setEnabled(true);
    buttonMetaData_Apply->setEnabled(true);

}

///////////////////////////////////////////////////////////
// Table mouse Right-click
///////////////////////////////////////////////////////////
void GexSqlHousekeepingDialog::OnIncrementalUpdateContextualMenu(const QPoint& /*ptMousePoint*/)
{
    QMenu               *pMenu = new QMenu(this);
    QAction             *pAction_RunIncremental = NULL;
    QAction             *pAction_RunThis = NULL;
    QAction             *pAction_Check = NULL;
    QTableWidgetItem     *ptItem = NULL;

    // Create menu
    if(buttonIncremental_Update->isEnabled())
        pAction_RunIncremental = pMenu->addAction("Run incremental updates..." );
    pAction_Check = pMenu->addAction("Check incremental updates count..." );

    if(buttonIncremental_Update->isEnabled())
    {
        //if(tableIncrementalUpdates->columnAt(ptMousePoint.x() >= 0))
        {
            // If at least one item in table
            if(tableIncrementalUpdates->selectedItems().count() > 0)
                ptItem = tableIncrementalUpdates->currentItem();

            ptItem = tableIncrementalUpdates->item(tableIncrementalUpdates->currentRow(),0);
            // If on an existing line
            // Option delete
            if(ptItem && !ptItem->text().isEmpty())
            {
                pMenu->addSeparator();
                pAction_RunThis = pMenu->addAction("Run this incremental update '"+ptItem->text()+"'" );
            }
        }
    }

    // Display menu...
    pMenu->setMouseTracking(true);
    QAction *pActionResult = pMenu->exec(QCursor::pos());
    pMenu->setMouseTracking(false);

    if(pActionResult == NULL)
        return;

    // Check menu selection activated
    if(pActionResult == pAction_RunIncremental)
    {
        OnButtonIncremental_Update();
        return;
    }
    if(pActionResult == pAction_Check)
    {
        OnButtonIncremental_Check();
        return;
    }
    if(ptItem && (pActionResult == pAction_RunThis))
    {
        OnButtonIncremental_Update(ptItem->text());
        return;
    }
}



///////////////////////////////////////////////////////////
// Table mouse Right-click
///////////////////////////////////////////////////////////
void GexSqlHousekeepingDialog::OnTableMetaDataContextualMenu(const QPoint& ptMousePoint)
{
    int					nRow, nCol;
    QMenu				*pMenu = new QMenu(this);
    QAction				*pAction_NewEntry = NULL;
    QAction				*pAction_DeleteEntry = NULL;
    QTableWidgetItem	*ptItem = NULL;

    // Create menu
    pAction_NewEntry = pMenu->addAction("New Meta-Data..." );

    // If at least one item in table
    if(tableMetaData->rowCount() > 0)
    {
        if(tableMetaData->selectedItems().count() > 0)
        {
            ptItem = tableMetaData->item(tableMetaData->currentRow(),0);
        }
    }

    // If on an existing line
    // Option delete
    if(ptItem)
        pAction_DeleteEntry = pMenu->addAction("Delete Meta-Data '"+ptItem->text()+"'" );

    // Display menu...
    pMenu->setMouseTracking(true);
    QAction *pActionResult = pMenu->exec(QCursor::pos());
    pMenu->setMouseTracking(false);

    tableMetaData->disconnect();

    // Check menu selection activated
    if(pActionResult == pAction_NewEntry)
    {
        // new Metadata
        QString strName, strToolTip;
        nRow = tableMetaData->rowCount();
        tableMetaData->setRowCount(nRow+1);
        fillCellData(tableMetaData,nRow,0,"NewMetaData","",false,true);
        for(nCol=1; nCol<tableMetaData->columnCount(); nCol++)
        {
            strName = strToolTip = "";
            ptItem = tableMetaData->horizontalHeaderItem(nCol);
            if(ptItem
                    && (ptItem->text().endsWith("_field") || ptItem->text().endsWith("_gui")))
            {
                strName = "N";

                if(ptItem->text() == "bintype_field")
                    strToolTip = "H, S or N (or even *)";
                else if(ptItem->text() == "az_field")
                    strToolTip = "Y or N";
                else if(ptItem->text().endsWith("_gui"))
                    strToolTip = "Y, N or *";
                else if(ptItem->text().endsWith("_field"))
                    strToolTip = "Y, N or *";

            }

            fillCellData(tableMetaData,nRow,nCol,strName,strToolTip,false,true);
        }

        buttonMetaData_Cancel->setEnabled(true);
        buttonMetaData_Apply->setEnabled(true);

    }
    if(pActionResult == pAction_DeleteEntry)
    {
        //delete the current row;
        nRow = tableMetaData->rowAt(ptMousePoint.y());
        for(nCol=0; nCol<tableMetaData->columnCount(); nCol++)
        {
            ptItem = tableMetaData->item(nRow,nCol);
            if(ptItem == NULL)
                continue;
            ptItem->setBackgroundColor(QColor(255, 152, 152));
            ptItem->setFlags(ptItem->flags() & ~Qt::ItemIsEditable);
        }
        buttonMetaData_Cancel->setEnabled(true);
        buttonMetaData_Apply->setEnabled(true);

    }

    QObject::connect(tableMetaData,			SIGNAL(cellChanged(int,int)),							this, SLOT(OnCellMetaData(int,int)));
    QObject::connect(tableMetaData,			SIGNAL(customContextMenuRequested ( const QPoint & )),	this,SLOT(OnTableMetaDataContextualMenu(const QPoint&)));

}

///////////////////////////////////////////////////////////
// set Table cell contents + tootip
///////////////////////////////////////////////////////////
void GexSqlHousekeepingDialog::fillCellData(QTableWidget *ptTable,int iRow,int iCol,QString strText,QString strTooltip/*=""*/,bool bReadOnly/*=false*/,bool bAlarm/*=false*/)
{
    // Create item
    QTableWidgetItem *ptItem = NULL;

    ptItem = ptTable->item(iRow, iCol);
    if(ptItem == NULL)
    {
        ptItem = new QTableWidgetItem(strText);
        // Add item to table cell
        ptTable->setItem(iRow,iCol,ptItem);
    }

    // Add tooltip
    ptItem->setToolTip(strTooltip);

    // Check if Read-Only mode
    if(bReadOnly)
    {
        ptItem->setFlags(ptItem->flags() & ~Qt::ItemIsEditable);
        ptItem->setBackgroundColor(QColor(200,200,200));	// grey background
    }

    // Check if Alarm color (orange) to use as background color
    if(bAlarm)
        ptItem->setBackgroundColor(QColor(0,198,0));	// Orange background

}


///////////////////////////////////////////////////////////
// Update table incrementalSettings with existing DB incremental_update
///////////////////////////////////////////////////////////
void GexSqlHousekeepingDialog::LoadIncrementalSettings()
{

    buttonIncremental_Cancel->setEnabled(true);
    buttonIncremental_Apply->setEnabled(false);

    // Get database ptr
    if(m_pTdrDatabaseEntry->m_pExternalDatabase
            && m_TdrDbPlugin
            && m_TdrDbPlugin->m_pclDatabaseConnector)
    {
        int             lRow, lCol;
        QStringList     lLabels;
        bool            lIsDefined;
        bool            lIsReadOnly;
        QString         lToolTip;

        // Get incremental updates
        QMap< QString,QMap< QString,QString > > lIncrementalUpdates;

        tableIncrementalUpdates->clear();
        tableIncrementalUpdates->setRowCount(0);
        tableIncrementalUpdates->disconnect();
        if(m_pTdrDatabaseEntry->m_pExternalDatabase->GetIncrementalUpdatesSettings(lIncrementalUpdates))
        {
            lLabels << "db_update_name" << "processed_splitlots" << "remaining_splitlots" << "status" << "frequency" << "max_items" << "last_schedule" << "last_execution" << "db_update_name_description";
            tableIncrementalUpdates->setColumnCount(lLabels.count());
            tableIncrementalUpdates->setHorizontalHeaderLabels(lLabels);
            tableIncrementalUpdates->verticalHeader()->hide();

            tableIncrementalUpdates->setSortingEnabled(false);		// disable sorting
            QString lIncrementalName;
            QString strValue;
            // Add all incremental updates
            lRow = lCol = 0;

            foreach(lIncrementalName, lIncrementalUpdates.keys())
            {
                // Insert item into ListView
                tableIncrementalUpdates->setRowCount(lRow+1);
                tableIncrementalUpdates->setRowHeight(lRow, 20);

                lCol=0;
                foreach(strValue, lLabels)
                {
                    lToolTip = strValue.toUpper();
                    lIsReadOnly = true;
                    lIsDefined = false;
                    if(lIncrementalUpdates[lIncrementalName].contains(strValue+"_type"))
                    {
                        lIsReadOnly = false;
                        lToolTip += "\nPossible values = "+lIncrementalUpdates[lIncrementalName][strValue+"_type"];
                    }
                    if(lIncrementalUpdates[lIncrementalName].contains(strValue+"_default"))
                    {
                        lToolTip += "\nDefault value = "+lIncrementalUpdates[lIncrementalName][strValue+"_default"];
                        if(lIncrementalUpdates[lIncrementalName][strValue].isEmpty())
                            lIncrementalUpdates[lIncrementalName][strValue] = lIncrementalUpdates[lIncrementalName][strValue+"_default"];
                        if(lIncrementalUpdates[lIncrementalName][strValue]!=lIncrementalUpdates[lIncrementalName][strValue+"_default"])
                        {
                            lIsDefined = true;
                            lToolTip += "\nUser value = "+lIncrementalUpdates[lIncrementalName][strValue];
                        }
                    }
                    if(lIncrementalUpdates[lIncrementalName].contains(strValue+"_description"))
                    {
                        lToolTip += "\nDescription\n "+lIncrementalUpdates[lIncrementalName][strValue+"_description"];
                    }
                    fillCellData(tableIncrementalUpdates,lRow,lCol, lIncrementalUpdates[lIncrementalName][strValue],lToolTip,lIsReadOnly,lIsDefined);
                    ++lCol;
                }
                lRow++;
            }

            // resize columns
            for(lCol=0; lCol<lLabels.count(); lCol++)		// 4 columns, 4 incremental updates
                tableIncrementalUpdates->resizeColumnToContents(lCol);
        }


        QObject::connect(tableIncrementalUpdates, SIGNAL(cellChanged(int,int)),
                         this, SLOT(OnCellIncrementalSettings(int,int)));
        QObject::connect(tableIncrementalUpdates, SIGNAL(customContextMenuRequested ( const QPoint & )),
                         this,SLOT(OnIncrementalUpdateContextualMenu(const QPoint&)));
    }
}

///////////////////////////////////////////////////////////
// Update table optionSettings with existing DB global_options
///////////////////////////////////////////////////////////
void GexSqlHousekeepingDialog::LoadOptionSettings()
{
    QString strValue;

    buttonOptionSettings_Cancel->setEnabled(false);
    buttonOptionSettings_Apply->setEnabled(false);

    // Get database ptr
    if(m_pTdrDatabaseEntry->m_pExternalDatabase
            && m_TdrDbPlugin
            && m_TdrDbPlugin->m_pclDatabaseConnector)
    {
        int         nRow, nCol;
        QStringList lstLabels;

        tableOptionSettings->clear();
        tableOptionSettings->setRowCount(0);
        tableOptionSettings->disconnect();

        int     nOptionNb;
        bool    bIsDefined;
        bool    bIsReadOnly;
        QString strDescription;
        QString strType;
        QString strToolTip;
        QString strOptionName;
        QString strDefaultValue;

        lstLabels << "Name" << "Value" << "Number" << "Type" << "Description";
        tableOptionSettings->setColumnCount(lstLabels.count());
        tableOptionSettings->setHorizontalHeaderLabels(lstLabels);
        tableOptionSettings->hideColumn(2);
        tableOptionSettings->hideColumn(3);
        tableOptionSettings->verticalHeader()->hide();

        nRow = nCol = 0;
        m_mapOptionsDefined.clear();
        m_lstOptionsAllowed.clear();
        for(nOptionNb=0; nOptionNb<=eMaxOptions; nOptionNb++)
        {

            if(!m_pTdrDatabaseEntry->m_pExternalDatabase->GetGlobalOptionName(nOptionNb,strOptionName))
                continue;
            m_lstOptionsAllowed.append(nOptionNb);

            if(!m_pTdrDatabaseEntry->m_pExternalDatabase->GetGlobalOptionValue(nOptionNb,strValue,bIsDefined))
                continue;

            m_mapOptionsDefined[nOptionNb]=strValue;

            // Insert item into ListView
            tableOptionSettings->setRowCount(nRow+1);
            tableOptionSettings->setRowHeight(nRow, 20);

            m_pTdrDatabaseEntry->m_pExternalDatabase->GetGlobalOptionReadOnly(nOptionNb,bIsReadOnly);
            m_pTdrDatabaseEntry->m_pExternalDatabase->GetGlobalOptionDescription(nOptionNb,strDescription);

            m_pTdrDatabaseEntry->m_pExternalDatabase->GetGlobalOptionTypeValue(nOptionNb,strType);
            strToolTip = "";
            if(bIsReadOnly)
                strToolTip += "ReadOnly option\n";
            if(bIsDefined)
                strToolTip += "User value = "+strValue+"\n";
            m_pTdrDatabaseEntry->m_pExternalDatabase->GetGlobalOptionDefaultValue(nOptionNb,strDefaultValue);
            if(strDefaultValue.isEmpty())
                strDefaultValue = "NULL or empty";
            strToolTip += "Default value = "+strDefaultValue+"\n";
            if(strType.contains(","))
                strToolTip += "Possible values = "+strType;
            else
                strToolTip += "Value type = "+strType;
            strToolTip = strDescription + "\n    ------------------    \n"+strToolTip;
            fillCellData(tableOptionSettings,nRow,0,strOptionName,strToolTip,true);
            fillCellData(tableOptionSettings,nRow,1,strValue,strType,bIsReadOnly);
            // Color if defined by the user
            if(bIsDefined)
                tableOptionSettings->item(nRow,1)->setBackgroundColor(QColor(0,198,0));

            fillCellData(tableOptionSettings,nRow,2,QString::number(nOptionNb),strToolTip,true);
            fillCellData(tableOptionSettings,nRow,3,strType,strToolTip,true);
            fillCellData(tableOptionSettings,nRow,4,strDescription.section("\n",0,0),strToolTip,true);

            // Set font color
            QColor rgbColor = QColor(0,0,0);
            if(bIsReadOnly)
                rgbColor = QColor(180,180,180);

            for(nCol=0;nCol<tableOptionSettings->columnCount();nCol++){
                // Resize
                tableOptionSettings->resizeColumnToContents(nCol);
                tableOptionSettings->item(nRow,nCol)->setTextColor(rgbColor);
            }

            nRow++;
        }
        QObject::connect(tableOptionSettings, SIGNAL(cellChanged(int,int)),
                         this, SLOT(OnCellOptionSettings(int,int)));
        QObject::connect(tableOptionSettings, SIGNAL(customContextMenuRequested ( const QPoint & )),
                         this,SLOT(OnTableOptionSettingsContextualMenu(const QPoint&)));
    }
}

///////////////////////////////////////////////////////////
// Update InnoDB-related controls
///////////////////////////////////////////////////////////
void GexSqlHousekeepingDialog::UpdateInnoDbControls(GexDbPlugin_Base *pDbPlugin)
{
    if(!pDbPlugin)
        return;

    m_pToInnoDBPushButton->hide();
    m_pEngineLabel->hide();
    m_pEngineValueLabel->hide();
    m_pToInnoDBPushButton->setEnabled(false);

    if((GS::LPPlugin::ProductInfo::getInstance()->isMonitoring()) // Yieldman or Patserver
            && (GS::Gex::Engine::GetInstance().GetSchedulerEngine().isSchedulerStopped()) // pause mode
            && (pDbPlugin->m_pclDatabaseConnector->IsMySqlDB()))
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, " MySQL DB selected.");
        m_pEngineLabel->show();
        m_pEngineValueLabel->show();

        QString strEngine, strFormat;
        if (pDbPlugin->GetStorageEngineName(strEngine,strFormat))
        {
            m_pEngineValueLabel->setText(strEngine+" "+strFormat.section("=",1));
            if (strEngine.toUpper()=="MYISAM")
            {
                m_pToInnoDBPushButton->setText("Update to InnoDB");

#ifdef BARRACUDA_ENABLED
                m_pToInnoDBPushButton->setText("Update to InnoDB\nBarracuda Compressed");
#endif
                m_pToInnoDBPushButton->setEnabled(true);
                m_pToInnoDBPushButton->show();
            }
            else
            {
#ifdef BARRACUDA_ENABLED
                // Engine is InnoDB
                // Check if it is the compressed mode
                if (strFormat.endsWith("compressed",Qt::CaseInsensitive))
                    m_pEngineValueLabel->setText(strEngine + " Barracuda Compressed");
                else
                {
                    m_pToInnoDBPushButton->setText("Update to InnoDB Barracuda Compressed");
                    m_pToInnoDBPushButton->setEnabled(true);
                    m_pToInnoDBPushButton->show();
                }
#endif
            }
        }
        else
            m_pEngineValueLabel->setText("(unknown)");
    }

    // Do not allow toInnoDb if DB is not up-to-date
    if(buttonUpdate->isEnabled())
        m_pToInnoDBPushButton->setEnabled(false);
}

///////////////////////////////////////////////////////////
// Table Cancel-click
///////////////////////////////////////////////////////////
void GexSqlHousekeepingDialog::OnButtonOptionSettings_Cancel()
{

    editLog_OptionSettings->insertPlainText("\n");
    editLog_OptionSettings->insertPlainText("CANCELED.");

    LoadOptionSettings();
    buttonOptionSettings_Cancel->setEnabled(false);
}

///////////////////////////////////////////////////////////
// Table Apply-click
///////////////////////////////////////////////////////////
void GexSqlHousekeepingDialog::OnButtonOptionSettings_Apply()
{
    bool	bStatus;
    QString lValue;
    QString lError;

    bStatus = true;

    progressBar_OptionSettings->show();
    progressBar_OptionSettings->setMaximum(100);
    progressBar_OptionSettings->setValue(0);


    // Get database ptr
    if(m_pTdrDatabaseEntry->m_pExternalDatabase
            && m_TdrDbPlugin
            && m_TdrDbPlugin->m_pclDatabaseConnector)
    {
        editLog_OptionSettings->clear();
        editLog_OptionSettings->insertPlainText("["+QDate::currentDate().toString(Qt::ISODate)+" "+QTime::currentTime().toString(Qt::ISODate)+"] Save new Option Settings\n");
        editLog_OptionSettings->insertPlainText("\n");
        editLog_OptionSettings->insertPlainText("Database "+m_TdrDbPlugin->m_pclDatabaseConnector->m_strSchemaName+"\n");
        QCoreApplication::processEvents();

        m_pTdrDatabaseEntry->TraceUpdate("HOUSEKEEPING","INFO","Start updating Option Settings");

        int                 nOptionNb;
        int                 nRow, nCol;
        int                 nUpdated, nDeleted;
        bool                bUpdateConsolidatedTable;
        bool                bUpdateConsolidationProcess;
        bool                bUpdateSecurityAccess;
        QString             lUpdateConsolidatedTableValue;
        QString             lQuery;
        QString             lOption;
        QStringList         lstValues;
        QTableWidgetItem    *ptItem;

        bUpdateConsolidatedTable = false;
        bUpdateConsolidationProcess = false;
        bUpdateSecurityAccess = false;
        nUpdated = nDeleted = 0;

        GexDbPlugin_Query	clQuery(m_TdrDbPlugin, QSqlDatabase::database(m_TdrDbPlugin->m_pclDatabaseConnector->m_strConnectionName));

        // Extract new value from tableOptionSettings
        lQuery = "";
        nRow = nCol = 0;
        for(nRow=0;nRow<tableOptionSettings->rowCount();nRow++)
        {
            lValue = "";

            ptItem = tableOptionSettings->item(nRow, 0);
            if(ptItem == NULL)
            {
                continue;
            }
            lOption = ptItem->text();
            ptItem = tableOptionSettings->item(nRow, 2);
            if(ptItem == NULL)
            {
                continue;
            }
            nOptionNb = ptItem->text().toInt();
            // Extract value
            ptItem = tableOptionSettings->item(nRow, 1);
            if(ptItem == NULL)
            {
                continue;
            }
            // Is value updated
            m_pTdrDatabaseEntry->m_pExternalDatabase->GetGlobalOptionValue(nOptionNb,lValue);
            if(lValue.toUpper() != ptItem->text().toUpper())
            {
                editLog_OptionSettings->insertPlainText(QString("%1 = %2\n").arg(lOption).arg(ptItem->text()));

                // Check if have to update Security Access
                if(nOptionNb == eDatabaseRestrictionMode)
                {
                    bUpdateSecurityAccess = true;
                    // Do not use the standard update option
                    // Must use a special method
                    continue;
                }

                // Check if have to Consolidate Lot_SBin
                if(nOptionNb == eBinningFtConsolidateSBin)
                {
                    lUpdateConsolidatedTableValue = ptItem->text();
                    bUpdateConsolidatedTable = true;
                }

                if(nOptionNb == eBinningConsolidationProcess)
                {
                    if(ptItem->text().toUpper() == "DELAY")
                    {
                        bUpdateConsolidationProcess = true;
                    }
                }

                lstValues << QString(lOption+"," + ptItem->text());
            }
        }

        if(bUpdateSecurityAccess && m_pTdrDatabaseEntry->m_pExternalDatabase)
        {
            GS::DAPlugin::DirAccessBase* lDirAccess = GS::Gex::Engine::GetInstance().GetAdminEngine().GetDirAccessPlugin();
            if(lDirAccess)
            {
                GS::DAPlugin::UsersBase * users = lDirAccess->GetUsers();
                GS::DAPlugin::GroupsBase * groups = lDirAccess->GetGroups();
                GS::DAPlugin::AppEntriesBase * applications = lDirAccess->GetAppEntries();
                if(!users || !applications || !groups)
                {
                    bStatus = false;
                    goto labelExit;
                }

                bool toSecured = !m_pTdrDatabaseEntry->m_pExternalDatabase->IsSecuredMode();

                // Update the DB
                if(!m_pTdrDatabaseEntry->m_pExternalDatabase->SetSecuredMode(toSecured))
                {
                    QString lPluginError;
                    m_pTdrDatabaseEntry->m_pExternalDatabase->GetLastError(lPluginError);
                    lError = "\n";
                    lError+= "******************************\n";
                    lError+= "Updating GEXDB Restriction Mode\n";
                    lError+= "\n";
                    lError+= lPluginError+"\n";
                    lError+= "\n";
                    lError+= "TDR Restriction Mode update CANCELED\n";
                    lError+= "******************************\n";
                    lError+= "\n";
                    if(lstValues.isEmpty())
                        bStatus = false;
                }
                else
                {

                    // Update the DA GALAXY
                    if(toSecured)
                    {
                        applications->Add("galaxy:examinator","databases");
                        applications->Add("galaxy:examinator:databases",m_pTdrDatabaseEntry->LogicalName());
                        applications->UpdateEntryAttribute("galaxy:examinator:databases:"+m_pTdrDatabaseEntry->LogicalName(),"description","Access to "+m_pTdrDatabaseEntry->LogicalName());
                        applications->UpdateEntryAttribute("galaxy:examinator:databases:"+m_pTdrDatabaseEntry->LogicalName(),"name",m_pTdrDatabaseEntry->LogicalName());

                        // Add databases to all users and groups
                        foreach(const QString &UserId, users->GetUsersList())
                            applications->AddUserPrivilege("galaxy:examinator:databases:"+m_pTdrDatabaseEntry->LogicalName(),UserId,GS::DAPlugin::NOACCESS);

                        foreach(const QString &GroupId, groups->GetGroupsList())
                            applications->AddGroupPrivilege("galaxy:examinator:databases:"+m_pTdrDatabaseEntry->LogicalName(),GroupId,GS::DAPlugin::NOACCESS);
                    }
                    else
                    {
                        applications->Remove("galaxy:examinator:databases:"+m_pTdrDatabaseEntry->LogicalName());
                        applications->Remove("galaxy:yieldman:databases:"+m_pTdrDatabaseEntry->LogicalName());
                        applications->Remove("galaxy:patman:databases:"+m_pTdrDatabaseEntry->LogicalName());
                    }

                    lDirAccess->SaveChanges();

                    // Open the GUI if Security Acces activated
                    if(toSecured)
                        lDirAccess->OpenAdministrationUi();
                    else
                    {
                        // Remove node databases if no entries
                        //TODO
                    }
                }
            }
        }

        // Update global_options table
        bool bUpdated = !lstValues.isEmpty();
        while(!lstValues.isEmpty())
        {
            lValue = lstValues.takeFirst();
            lOption = lValue.section(",",0,0);
            lValue = lValue.section(",",1);
            if(!m_pTdrDatabaseEntry->m_pExternalDatabase->SetGlobalOptionValue(lOption,lValue))
            {
                m_pTdrDatabaseEntry->m_pExternalDatabase->GetLastError(lError);
                bStatus = false;
                goto labelExit;
            }
        }
        if(bUpdated)
        {
            editLog_OptionSettings->insertPlainText("Options updated\n");
        }

        if(bUpdateConsolidatedTable)
        {

            // Have to update consolidated table
            // Check if the user really want to do this
            editLog_OptionSettings->insertPlainText("\n");
            editLog_OptionSettings->insertPlainText("Updating Consolidated Table.\n");

            lQuery = "SELECT count(DISTINCT lot_id) FROM ft_splitlot WHERE valid_splitlot='Y'";
            if(!clQuery.Execute(lQuery))
            {
                lError="\nQUERY=";
                lError+=lQuery;
                lError+="\nERROR=";
                lError+=clQuery.lastError().text();
                bStatus = false;
                goto labelExit;
            }
            clQuery.first();

            QMessageBox msgBox(this);

            if (pGexSkin)
                pGexSkin->applyPalette(&msgBox);

            if(lUpdateConsolidatedTableValue == "TRUE")
            {
                if( m_mapOptionsDefined.contains(eBinningFtConsolidateSBin)
                        && (m_mapOptionsDefined[eBinningFtConsolidateSBin] == "TRUE"))
                {
                    // SoftBin already updated
                    editLog_OptionSettings->insertPlainText(" * SoftBin already updated\n");
                }
                else
                {
                    QString strMessage = "Your GexDB FT";
                    strMessage+= " SoftBins table needs to be re-generated due to your changes in the global options.\n";
                    strMessage+= clQuery.value(0).toString()+" lots will be processed and consolidated to populate SoftBins table.\n";
                    strMessage+= "This may take several minutes to a few hours depending on the size of your splitlot table and the performance of your database server.\n";
                    strMessage+= "\nDo you confirm ? ";

                    msgBox.setWindowTitle("Updating SoftBin Table");
                    msgBox.setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));
                    msgBox.setText(strMessage);
                    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
                    msgBox.setDefaultButton(QMessageBox::Cancel);

                    msgBox.setButtonText( QMessageBox::Yes, "Consolidate now" );
                    msgBox.setButtonText( QMessageBox::No, "Incremental Update" );
                    msgBox.setButtonText( QMessageBox::Cancel, "Do not consolidate" );

                    // If not, do nothing
                    int nStatus = msgBox.exec();
                    if(nStatus == QMessageBox::Cancel)
                    {
                        editLog_OptionSettings->insertPlainText(" * No consolidation updated\n");
                        goto labelExit;
                    }

                    lQuery = "SELECT DISTINCT lot_id FROM ft_splitlot WHERE valid_splitlot='Y'";
                    if(!clQuery.Execute(lQuery))
                    {
                        lError="\nQUERY=";
                        lError+=lQuery;
                        lError+="\nERROR=";
                        lError+=clQuery.lastError().text();
                        bStatus = false;
                        goto labelExit;
                    }
                    QStringList lstLots;
                    while(clQuery.next())
                        lstLots << clQuery.value(0).toString();

                    if(nStatus == QMessageBox::No)
                    {
                        // Marked with SOFTBIN_CONSOLIDATION
                        editLog_OptionSettings->insertPlainText("Incremental Update\n");
                        progressBar_OptionSettings->setMaximum(lstLots.count());
                        progressBar_OptionSettings->setValue(0);
                        while(!lstLots.isEmpty())
                        {
                            lValue = lstLots.takeFirst();
                            lQuery = "UPDATE ft_splitlot S ";
                            lQuery+= " SET incremental_update='FT_CONSOLIDATE_SOFTBIN' ";
                            lQuery+= " WHERE ";
                            lQuery+= " S.lot_id='"+lValue+"' ";
                            lQuery+= " AND valid_splitlot<>'N' ";
                            lQuery+= " AND ((S.incremental_update IS NULL) OR (S.incremental_update=''))";
                            clQuery.Execute(lQuery);
                            lQuery = "UPDATE ft_splitlot S ";
                            lQuery+= " SET incremental_update=CONCAT(S.incremental_update,'|FT_CONSOLIDATE_SOFTBIN') ";
                            lQuery+= " WHERE ";
                            lQuery+= " S.lot_id='"+lValue+"'";
                            lQuery+= " AND valid_splitlot<>'N' ";
                            lQuery+= " AND NOT((S.incremental_update IS NULL) OR (S.incremental_update='') OR (S.incremental_update LIKE '%FT_CONSOLIDATE_SOFTBIN%'))";
                            clQuery.Execute(lQuery);
                            progressBar_OptionSettings->setValue(progressBar_OptionSettings->value()+1);

                        }

                        int lCount;
                        m_pTdrDatabaseEntry->m_pExternalDatabase->GetIncrementalUpdatesCount(true,lCount);

                        lQuery = " SELECT remaining_splitlots FROM incremental_update";
                        lQuery+= " WHERE db_update_name = 'FT_CONSOLIDATE_SOFTBIN'";
                        clQuery.Execute(lQuery);
                        clQuery.first();
                        nUpdated = clQuery.value(0).toInt();

                        editLog_OptionSettings->insertPlainText(" * "+QString::number(nUpdated)+" splitlots updated\n");

                        //reload all info
                        OnDatabaseRefresh();

                        goto labelExit;
                    }

                    // Set Testing Stage to Final Test
                    QStringList lstTestingStages;
                    m_pTdrDatabaseEntry->m_pExternalDatabase->GetSupportedTestingStages(lstTestingStages);
                    while(!lstTestingStages.isEmpty())
                    {
                        lValue = lstTestingStages.takeFirst();
                        if(lValue.startsWith("Final",Qt::CaseInsensitive))
                        {
                            m_pTdrDatabaseEntry->m_pExternalDatabase->SetTestingStage(lValue);
                            break;
                        }
                    }
                    m_pTdrDatabaseEntry->TraceUpdate("HOUSEKEEPING","INFO","Start running Consolidation Lot for "+lValue);

                    nUpdated = 0;
                    progressBar_OptionSettings->setMaximum(lstLots.count());
                    progressBar_OptionSettings->setValue(0);
                    while(!lstLots.isEmpty())
                    {
                        lValue = lstLots.takeFirst();
                        if(!m_pTdrDatabaseEntry->m_pExternalDatabase->ConsolidateLot(lValue,true,false))
                        {
                            m_pTdrDatabaseEntry->m_pExternalDatabase->GetLastError(lError);
                            m_pTdrDatabaseEntry->TraceUpdate("HOUSEKEEPING","INFO","Start running Consolidation Lot for "
                                                             +lValue+" error: "+lError);
                            bStatus = false;
                            goto labelExit;
                        }
                        nUpdated++;
                        progressBar_OptionSettings->setValue(progressBar_OptionSettings->value()+1);
                    }
                    editLog_OptionSettings->insertPlainText(" * "+QString::number(nUpdated)+" lots updated\n");
                }
            }
            else
            {
                // Delete all consolidate info
                QString strMessage = "Your GexDB FT";
                strMessage+= " SoftBins table will be reset due to your changes in the global options.\n";
                strMessage+= "\nDo you confirm ? ";

                msgBox.setWindowTitle("Reseting SoftBin Table");
                msgBox.setText(strMessage);
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
                msgBox.setDefaultButton(QMessageBox::Cancel);

                // Multiple files selected: ask if Compare or Merge?
                msgBox.setButtonText( QMessageBox::Yes, "Reset now" );
                msgBox.setButtonText( QMessageBox::Cancel, "Do not reset" );

                // If not, do nothing
                int nStatus = msgBox.exec();
                if(nStatus == QMessageBox::Cancel)
                {
                    lQuery = "UPDATE ft_splitlot S ";
                    lQuery+= " SET incremental_update=REPLACE(S.incremental_update,'FT_CONSOLIDATE_SOFTBIN','') ";
                    clQuery.Execute(lQuery);
                    editLog_OptionSettings->insertPlainText(" * No reset\n");
                    goto labelExit;
                }

                lQuery = "SELECT DISTINCT lot_id from ft_splitlot WHERE valid_splitlot='Y'";
                if(!clQuery.Execute(lQuery))
                {
                    lError="\nQUERY=";
                    lError+=lQuery;
                    lError+="\nERROR=";
                    lError+=clQuery.lastError().text();
                    bStatus = false;
                    goto labelExit;
                }
                QStringList lstLots;
                while(clQuery.next())
                    lstLots << clQuery.value(0).toString();

                nUpdated = 0;
                // Set Testing Stage to Final Test
                QStringList lstTestingStages;
                m_pTdrDatabaseEntry->m_pExternalDatabase->GetSupportedTestingStages(lstTestingStages);
                progressBar_OptionSettings->setMaximum(lstLots.count());
                progressBar_OptionSettings->setValue(0);
                while(!lstTestingStages.isEmpty())
                {
                    lValue = lstTestingStages.takeFirst();
                    if(lValue.startsWith("Final",Qt::CaseInsensitive))
                    {
                        m_pTdrDatabaseEntry->m_pExternalDatabase->SetTestingStage(lValue);
                        break;
                    }
                }
                m_pTdrDatabaseEntry->TraceUpdate("HOUSEKEEPING","INFO","Start running Consolidation Lot for "+lValue);
                while(!lstLots.isEmpty())
                {
                    lValue = lstLots.takeFirst();
                    if(!m_pTdrDatabaseEntry->m_pExternalDatabase->ConsolidateLot(lValue,true,false))
                    {
                        m_pTdrDatabaseEntry->m_pExternalDatabase->GetLastError(lError);
                        m_pTdrDatabaseEntry->TraceUpdate("HOUSEKEEPING","INFO","Start running Consolidation Lot for "+
                                                         lValue+" error: "+lError);
                        bStatus = false;
                        goto labelExit;
                    }
                    nUpdated++;
                    progressBar_OptionSettings->setValue(progressBar_OptionSettings->value()+1);
                }
                editLog_OptionSettings->insertPlainText(" * "+QString::number(nUpdated)+" lots updated\n");
            }
        }

        if(bUpdateConsolidationProcess)
        {

            // User wants to DELAY the Consolidation Process
            // Warm him if the incremental update is disabled
            // Get incremental updates
            QMap< QString,QMap< QString,QString > > lIncrementalUpdates;
            m_pTdrDatabaseEntry->m_pExternalDatabase->GetIncrementalUpdatesSettings(lIncrementalUpdates);
            if(lIncrementalUpdates.contains("BINNING_CONSOLIDATION")
                    && lIncrementalUpdates["BINNING_CONSOLIDATION"]["status"] != "ENABLED")
            {
                // Delete all consolidate info
                QString strMessage = "You chose to DELAY the Consolidation Process.\n";
                strMessage+= " The BINNING_CONSOLIDATION Incremental Update process is currently disabled.\n";
                strMessage+= " You need to turn on the Consolidation process.\n";
                strMessage+= " Do you want to turn on this Incremental Update feature ";

                QMessageBox msgBox(this);

                if (pGexSkin)
                    pGexSkin->applyPalette(&msgBox);

                msgBox.setWindowTitle("Consolidation Process");
                msgBox.setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));
                msgBox.setText(strMessage);
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
                msgBox.setDefaultButton(QMessageBox::Cancel);

                // Multiple files selected: ask if Compare or Merge?
                msgBox.setButtonText( QMessageBox::Yes, "Turn on now" );
                msgBox.setButtonText( QMessageBox::Cancel, "No" );

                // If not, do nothing
                int nStatus = msgBox.exec();
                if(nStatus == QMessageBox::Yes)
                {
                    lIncrementalUpdates["BINNING_CONSOLIDATION"]["status"] = "ENABLED";
                    m_pTdrDatabaseEntry->m_pExternalDatabase->SetIncrementalUpdatesSettings(lIncrementalUpdates);
                    OnButtonIncremental_Check();
                    editLog_OptionSettings->insertPlainText("\n");
                    editLog_OptionSettings->insertPlainText("Updating Consolidation Process.\n");
                }
            }
        }
    }

labelExit:

    progressBar_OptionSettings->hide();
    if(bStatus == false)
    {
        editLog_OptionSettings->insertPlainText("\n");
        editLog_OptionSettings->insertPlainText(lError.replace(":",":\n"));
        editLog_OptionSettings->insertPlainText("\n");
        editLog_OptionSettings->insertPlainText("Status = FAIL.");
    }
    else
    {
        editLog_OptionSettings->insertPlainText("\n");
        if(lError.isEmpty())
            editLog_OptionSettings->insertPlainText("Status = SUCCESS.");
        else
        {
            editLog_OptionSettings->insertPlainText(lError.replace(":",":\n"));
            editLog_OptionSettings->insertPlainText("\n");
            editLog_OptionSettings->insertPlainText("Status = WARNING.");
        }

        buttonOptionSettings_Cancel->setEnabled(false);
        buttonOptionSettings_Apply->setEnabled(false);
        LoadOptionSettings();
    }
    editLog_OptionSettings->moveCursor(QTextCursor::End);

}

///////////////////////////////////////////////////////////
// Table cell updated
///////////////////////////////////////////////////////////
void GexSqlHousekeepingDialog::OnCellIncrementalSettings(int nRow,int nCol)
{
    QTableWidgetItem	*ptItem;
    ptItem = tableIncrementalUpdates->item(nRow,nCol);

    if(ptItem == NULL)
        return;

    if(m_pTdrDatabaseEntry->m_pExternalDatabase == NULL)
        return;

    // Item value changed
    QString lToolTip = ptItem->toolTip();

    // check if have the good value (saved in the tooltip)
    if(!lToolTip.isEmpty())
    {
        QStringList lOptionNames;
        QString	lValue;
        QString lOriginalValue;
        QString lUpdateName;

        lOptionNames << "db_update_name" << "processed_splitlots" << "remaining_splitlots" << "status" << "frequency" << "max_items" << "last_schedule" << "last_execution";
        lUpdateName = lOptionNames.at(nCol);
        lOriginalValue = lToolTip.section("User value = ",1).section("\n",0,0).simplified();
        if(lOriginalValue.isEmpty())
            lOriginalValue = lToolTip.section("Default value = ",1).section("\n",0,0).simplified();
        // New Value
        lValue = ptItem->text();
        if(!m_pTdrDatabaseEntry->m_pExternalDatabase->IsIncrementalUpdatesSettingsValidValue(lUpdateName,lValue))
        {
            // Value is incorrect
            // already updated
        }
        // Update the GUI with the good format
        // - upper case
        // - remove space
        // - ...
        if(lValue != ptItem->text())
        {
            tableIncrementalUpdates->disconnect();
            ptItem->setText(lValue);

            QObject::connect(tableIncrementalUpdates, SIGNAL(cellChanged(int,int)),                           this,SLOT(OnCellIncrementalSettings(int,int)));
            QObject::connect(tableIncrementalUpdates, SIGNAL(customContextMenuRequested ( const QPoint & )),  this,SLOT(OnTableIncrementalContextualMenu(const QPoint&)));
        }
        if(lOriginalValue != lValue)
        {
            ptItem->setBackgroundColor(QColor(Qt::red));
            buttonIncremental_Apply->setEnabled(true);
        }
        else
            ptItem->setBackgroundColor(QColor(Qt::transparent));
    }
}


///////////////////////////////////////////////////////////
// Table cell updated
///////////////////////////////////////////////////////////
void GexSqlHousekeepingDialog::OnCellOptionSettings(int nRow,int nCol)
{
    QTableWidgetItem	*ptItem;
    ptItem = tableOptionSettings->item(nRow,nCol);

    if(ptItem == NULL)
        return;

    if(m_pTdrDatabaseEntry->m_pExternalDatabase == NULL)
        return;

    // Item value changed
    QString strToolTip = ptItem->toolTip();

    // check if have the good value (saved in the tooltip)
    if(!strToolTip.isEmpty())
    {
        QString	strValue;
        QString strOptionName;
        int nOptionNb;
        strOptionName = tableOptionSettings->item(nRow,0)->text();
        for(nOptionNb=0; nOptionNb<=eMaxOptions; nOptionNb++)
        {
            if(!m_pTdrDatabaseEntry->m_pExternalDatabase->GetGlobalOptionName(nOptionNb,strValue))
                continue;
            if(strOptionName == strValue)
                break;
        }
        if(nOptionNb==eMaxOptions)
            return; // Not found ?

        strValue = ptItem->text();
        if(!m_pTdrDatabaseEntry->m_pExternalDatabase->IsGlobalOptionValidValue(nOptionNb,strValue))
        {
            // Value is incorrect
            m_pTdrDatabaseEntry->m_pExternalDatabase->GetGlobalOptionDefaultValue(nOptionNb,strValue);
        }
        // Update the GUI with the good format
        // - upper case
        // - remove space
        // - ...
        tableMetaData->disconnect();
        ptItem->setText(strValue);

        QObject::connect(tableMetaData, SIGNAL(cellChanged(int,int)),                           this,SLOT(OnCellMetaData(int,int)));
        QObject::connect(tableMetaData, SIGNAL(customContextMenuRequested ( const QPoint & )),  this,SLOT(OnTableMetaDataContextualMenu(const QPoint&)));


    }
    ptItem->setBackgroundColor(QColor(Qt::red));
    buttonOptionSettings_Cancel->setEnabled(true);
    buttonOptionSettings_Apply->setEnabled(true);

}

///////////////////////////////////////////////////////////
// Table mouse Right-click
///////////////////////////////////////////////////////////
void GexSqlHousekeepingDialog::OnTableOptionSettingsContextualMenu(const QPoint& /*ptMousePoint*/)
{
    QMenu               *pMenu = new QMenu(this);
    QTableWidgetItem    *ptItem = NULL;

    if(m_pTdrDatabaseEntry->m_pExternalDatabase == NULL)
        return;

    // If at least one item in table
    if(tableOptionSettings->rowCount() == 0)
        return;
    if(tableOptionSettings->selectedItems().count() == 0)
        return;
    // Check if the value is editable
    ptItem = tableOptionSettings->item(tableOptionSettings->currentRow(),1);
    if(ptItem == NULL)
        return;
    if(!(ptItem->flags() & Qt::ItemIsEditable))
        return;

    // Create menu
    int nOptionNb;
    ptItem = tableOptionSettings->item(tableOptionSettings->currentRow(),2);
    if(ptItem == NULL)
        return;
    nOptionNb = ptItem->text().toInt();
    ptItem = tableOptionSettings->item(tableOptionSettings->currentRow(),0);
    if(ptItem == NULL)
        return;
    QString Name = ptItem->text();
    ptItem = tableOptionSettings->item(tableOptionSettings->currentRow(),1);
    if(ptItem == NULL)
        return;
    QString Value = ptItem->text();
    QString Default;
    m_pTdrDatabaseEntry->m_pExternalDatabase->GetGlobalOptionDefaultValue(nOptionNb,Default);
    if(Value.toUpper() == Default.toUpper())
        return;

    // If on an existing line
    // Option reset
    QAction *pActionResult = pMenu->addAction("Reset option '"+Name+"' to "+Default );

    if(pMenu->actions().count() == 0)
        return;

    // Display menu...
    pMenu->setMouseTracking(true);
    pActionResult = pMenu->exec(QCursor::pos());
    pMenu->setMouseTracking(false);

    if(pActionResult == NULL)
        return;

    //tableOptionSettings->disconnect();

    // Reset option
    ptItem->setText(Default);
    //buttonOptionSettings_Cancel->setEnabled(true);
    //buttonOptionSettings_Apply->setEnabled(true);
    //QObject::connect(tableOptionSettings,			SIGNAL(cellChanged(int,int)),							this, SLOT(OnCellOptionSettings(int,int)));
    //QObject::connect(tableOptionSettings,			SIGNAL(customContextMenuRequested ( const QPoint & )),	this,SLOT(OnTableOptionSettingsContextualMenu(const QPoint&)));

}

///////////////////////////////////////////////////////////
// Update DB Summary
///////////////////////////////////////////////////////////
// strCommand is a number = current gexdb version build
// strCommand = "ReleasesHistory", display all version
// strCommand = "InnoDB", display InnoDb Summary
// strCommand = "Barracuda", display Barracuda Summary
// strCommand = "IncrementalUpdate", display Incremental Update Summary
///////////////////////////////////////////////////////////
// return QMessageBox::Yes, QMessageBox::YesToAll, QMessageBox::Cancel
///////////////////////////////////////////////////////////
int GexSqlHousekeepingDialog::UpdateDbSummaryDialog(QString strCommand, QTextEdit *pTextEdit_Log)
{

    QString         ToDbVersion, FromDbVerion;
    unsigned int    uiToDbVersion_Build = 0;
    unsigned int    uiToDbVersion_Minor = 0;
    unsigned int    uiToDbVersion_Major = 0;
    unsigned int    uiFromDbVersion_Build = 0;
    unsigned int    uiFromDbVersion_Minor = 0;
    unsigned int    uiFromDbVersion_Major = 0;
    QString DbStatus;
    QString AdrStatus;
    QString strTopMessage;
    QString strBottomMessage;
    QString strHtmlText;
    QString strCompatibilityVersion;

    // command= "UpdateDbFrom=GEXDB V2.01 B23 (MySQL)|UpdateDbTo=GEXDB V3.00 B24 (MySQL)|DbStatus="
    // extract info
    QStringList lCommands = strCommand.split("|");
    foreach(QString cmd, lCommands)
    {
        if(cmd.startsWith("UpdateDbFrom=",Qt::CaseInsensitive))
        {
            FromDbVerion = cmd.section("=",1).section("(",0,0).simplified();
            cmd = FromDbVerion.toUpper().section(" V",1);
            uiFromDbVersion_Major = cmd.section(".",0,0).toUInt();
            uiFromDbVersion_Minor = cmd.section(".",1).section(" ",0,0).toUInt();
            uiFromDbVersion_Build = cmd.section(" B",1).toUInt();

        }
        else if(cmd.startsWith("UpdateDbTo=",Qt::CaseInsensitive))
        {
            ToDbVersion = cmd.section("=",1).section("(",0,0).simplified();
            cmd = ToDbVersion.toUpper().section(" V",1);
            uiToDbVersion_Major = cmd.section(".",0,0).toUInt();
            uiToDbVersion_Minor = cmd.section(".",1).section(" ",0,0).toUInt();
            uiToDbVersion_Build = cmd.section(" B",1).toUInt();

        }
        else if(cmd.startsWith("DbStatus=",Qt::CaseInsensitive))
        {
            cmd = cmd.section("=",1);
            DbStatus = cmd;
        }
        else if(cmd.startsWith("AdrStatus=",Qt::CaseInsensitive))
        {
            cmd = cmd.section("=",1);
            AdrStatus = cmd;
        }
    }


    if(strCommand.startsWith("UpdateDbFrom",Qt::CaseInsensitive))
    {
        // Build msg when compatibility is not supported by one GEX application
        if((uiToDbVersion_Build > uiFromDbVersion_Build)
                && ((uiFromDbVersion_Major != uiToDbVersion_Major) || (uiFromDbVersion_Minor != uiToDbVersion_Minor)))
        {
            QString lCurrentVersion = "V"+GS::Gex::Engine::GetInstance().Get("AppVersionMajor").toString()
                    +"."+GS::Gex::Engine::GetInstance().Get("AppVersionMinor").toString();
            QString lPreviousVersion;
            if(GS::Gex::Engine::GetInstance().Get("AppVersionMinor").toInt() == 0)
                lPreviousVersion = "V"+QString::number(GS::Gex::Engine::GetInstance().Get("AppVersionMajor").toInt()-1)+".x";
            else
                lPreviousVersion = "V"+GS::Gex::Engine::GetInstance().Get("AppVersionMajor").toString()
                        +"."+QString::number(GS::Gex::Engine::GetInstance().Get("AppVersionMinor").toInt()-1);

            strCompatibilityVersion+= "<ul>\n";
            if(uiFromDbVersion_Major != uiToDbVersion_Major)
                strCompatibilityVersion+= " <li> <b>This is a <font color=\"red\">major update</font>:</b> Only Quantix "+lCurrentVersion+" applications will be allowed to use this database\n";
            else if(uiFromDbVersion_Minor != uiToDbVersion_Minor)
                strCompatibilityVersion+= " <li> <b>This is a minor update:</b> Only Yield-Man and Pat-Man applications will need to be updated to "+lCurrentVersion+" to manage this database\n";
            strCompatibilityVersion+= " <li> <b>Current database version: </b> "+FromDbVerion+"\n";
            strCompatibilityVersion+= " <li> <b>Update to database version: </b> "+ToDbVersion+"\n";
            strCompatibilityVersion+= "</ul>\n";
            QString GexUpdate = lPreviousVersion+" compatible";
            QString YMUpdate = lPreviousVersion+" compatible";
            if(uiFromDbVersion_Major != uiToDbVersion_Major)
                YMUpdate = GexUpdate = "<font color=\"red\">Requires "+lCurrentVersion+"</font>";
            else if(uiFromDbVersion_Minor != uiToDbVersion_Minor)
                YMUpdate = "Requires "+lCurrentVersion;
            strCompatibilityVersion+= "<center><table width=80% border=1 cellspacing=1 bordercolor=\"#999999\">\n";
            strCompatibilityVersion+= "<tr align=center>\n";
            strCompatibilityVersion+= "<td></td>\n";
            strCompatibilityVersion+= "<td><b> Examinator-PRO </b></td>\n";
            strCompatibilityVersion+= "<td><b> Yield-Man </b></td>\n";
            strCompatibilityVersion+= "<td><b> PAT-Man </b></td>\n";
            strCompatibilityVersion+= "</tr>\n";
            strCompatibilityVersion+= "<tr align=center>\n";
            strCompatibilityVersion+= "<td> Database Access </td>\n";
            strCompatibilityVersion+= "<td align=center>"+GexUpdate+"</td>\n";
            strCompatibilityVersion+= "<td align=center>"+YMUpdate+"</td>\n";
            strCompatibilityVersion+= "<td align=center>"+YMUpdate+"</td>\n";
            strCompatibilityVersion+= "</tr>\n";
            strCompatibilityVersion+= "</table></center>\n";

            // Update Database with Mandatory Gex application update
            strHtmlText += strCompatibilityVersion+"<BR>";
            if(uiFromDbVersion_Major != uiToDbVersion_Major)
            {
                // If you update this database to GEXDB V2.01 B23, ALL Examinator users
                // will need to also update their Examinator version to V7.1 in order to
                // connect to this database version.
                strBottomMessage += "If you update this database to <b><font color=\"red\">V"
                        +QString::number(uiToDbVersion_Major)+"."+QString::number(uiToDbVersion_Minor)+"</font></b>"
                        +", <b><font color=\"red\">all</font></b> Examinator users will need to use <b><font color=\"red\">Examinator V"
                        +GS::Gex::Engine::GetInstance().Get("AppVersionMajor").toString()+"."
                        +GS::Gex::Engine::GetInstance().Get("AppVersionMinor").toString()
                        +"</font></b> in order<BR> to connect to this database version.<BR>";
            }


        }

        if(!DbStatus.isEmpty())
        {
            strHtmlText = "<table width=\"100%\"><tbody>"
                          "<tr>"
                          "<td halign=\"center\" height=\"29\" valign=\"middle\" width=\"15\"><img src=\":/gex/icons/gex_history_arrow_orange.png\" height=\"9\" width=\"10\"></td>"
                          "<td class=\"main-content-title-blue\" align=\"left\" height=\"29\"><p align=\"left\">"
                          "<font size=\"4\" face=\"arial\" color=\"#006699\">Database status</font>  </p></td>"
                          "</tr>"
                          "<tr>"
                          "<td height=\"1\" width=\"15\"></td>"
                          "<td height=\"1\"></td>"
                          "</tr>"
                          "<tr>"
                          "<td width=\"15\"></td>"
                          "<td class=\"main-content-text\"><b><font size=\"5\"></font></b>"
                          "<table width=\"100%\" border=\"1\" cellpadding=\"2\" cellspacing=\"0\"><tbody>"
                          "<tr>"
                          "<td class=\"main-content-text\" align=\"left\" valign=\"top\">"
                          "<ul>"
                          "<li><b>"
                    +DbStatus.replace(" - ","</b><BR> - ")+
                    "</b>"
                    "</ul>"
                    "</td>"
                    "</tr>"
                    "</tbody></table>"
                    "</td>"
                    "</tr>"
                    "<tr>"
                    "<td></td>"
                    "</tr>"
                    "</tbody></table>";
        }
    }

    if((strCommand == "InnoDB") || (strCommand == "Barracuda") || (uiFromDbVersion_Build > 0))
    {
        strTopMessage = "<center><font size=\"6\" face=\"arial\" color=\"#858585\">Information about Quantix database update</font>\n";
        strTopMessage+= "<hr color=\"#afc22a\"></center>";

        strTopMessage+= "You are about to update your database";
        if(strCommand == "InnoDB")
            strTopMessage+=" from <b>MyISAM</b> storage engine to <b>InnoDB</b> storage engine";
        else if(strCommand == "Barracuda")
            strTopMessage+=" to <b>InnoDB Barracuda</b> format";

        strTopMessage+= ".<br>\nClick the 'Yes' button to update:\n";

        strBottomMessage+= "<h4>Do you want to update your database now ?</h4>";

    }
    else if(strCommand == "ReleasesHistory")
    {
        strTopMessage = "<center><font size=\"6\" face=\"arial\" color=\"#858585\">Quantix database Releases History</font>\n";
        strTopMessage+= "<hr color=\"#afc22a\"></center>";

        uiFromDbVersion_Build = 9;
    }
    else if(strCommand == "IncrementalUpdate")
    {
        strTopMessage = "<center><font size=\"6\" face=\"arial\" color=\"#858585\">Quantix database Incremental Update</font>\n";
        strTopMessage+= "<hr color=\"#afc22a\"></center>";
        strBottomMessage+= "<h4>Do you want to update all your database ?</h4>";
    }

    if(uiFromDbVersion_Build > 0)
    {
        QString		strHistoryFile	= GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString() + "/help/pages/_gexdb_history.htm";
        QFile file(strHistoryFile); // Read the text from a file
        if (file.open(QIODevice::ReadOnly) == false)
            return false;	// Failed opening file.


        QTextStream fileHistory(&file);
        QString		strLine;

        int nStartVersion = (int) uiFromDbVersion_Build;
        int nCurrentSection = 0;

        while(!fileHistory.atEnd())
        {
            strLine = fileHistory.readLine();
            if(strLine.startsWith("<!-- B"))
            {
                nCurrentSection = strLine.section("<!-- B",1).section("-->",0,0).simplified().toInt();
                break;
            }
        }
        // nCurrentSection is the last release
        if(nCurrentSection >= nStartVersion)
        {
            while(!fileHistory.atEnd())
            {
                strLine = fileHistory.readLine();
                if(strLine.startsWith("<!-- END"))
                    break;

                if(strLine.startsWith("<!-- B"))
                {
                    nCurrentSection = strLine.section("<!-- B",1).section("-->",0,0).simplified().toInt();
                    if(nCurrentSection <= nStartVersion)
                        break;
                }

                // Remove Top marker
                strLine = strLine.remove("<a href=\"#Top\">(Top)</a>");

                // Change table width to 100%
                strLine = strLine.replace("width=\"800\"","width=\"100%\"");

                // Change images sources directory to Gex ressources
                strLine = strLine.replace("../images/",":/gex/icons/");

                // Change font
                strLine = strLine.replace(QRegExp("(<a name=\"B\\d+\">)"),"<font size=\"4\" face=\"arial\" color=\"#006699\">").replace("</a>","</font>");

                strHtmlText+=strLine;
            }
        }

    }
    else if(strCommand == "InnoDB")
    {
        // InnoDb convertion

        int nSize=1;
        m_pTdrDatabaseEntry->m_pExternalDatabase->GetTotalSize(nSize);

        strHtmlText+= "<table width=\"100%\"><tbody>";
        strHtmlText+= "  <tr>";
        strHtmlText+= "	<td halign=\"center\" height=\"29\" valign=\"middle\" width=\"15\"><img src=\":/gex/icons/gex_history_arrow_orange.png\" height=\"9\" width=\"10\"></td>";
        strHtmlText+= "	<td align=\"left\" height=\"29\"><p align=\"left\">";
        strHtmlText+= "	<font size=\"4\" face=\"arial\" color=\"#006699\">InnoDB storage engine conversion</font></p></td>";
        strHtmlText+= "  </tr>";
        strHtmlText+= "  <tr>";
        strHtmlText+= "	<td height=\"1\" width=\"15\"></td>";
        strHtmlText+= "	<td height=\"1\"></td>";
        strHtmlText+= "  </tr>";
        strHtmlText+= "  <tr>";
        strHtmlText+= "	<td width=\"15\"></td>";
        strHtmlText+= "	<td><b><font size=\"5\"></font></b>";
        strHtmlText+= "	  <table width=\"100%\" border=\"1\" cellpadding=\"2\" cellspacing=\"0\"><tbody>";
        strHtmlText+= "	  <tr>";
        strHtmlText+= "		  <td valign=\"top\">";
        strHtmlText+= "			<ul>";
        strHtmlText+= "			<li><b>Conversion time</b>";
        strHtmlText+= "			<BR>The update to the InnoDB storage engine may take several hours to a few days, depending on your database size.";
        // Update the message according to the current database size
        // table less that 140Gb = <24h
        if(nSize < 1400)
            strHtmlText+= "			<BR>According to the current DB size, the conversion time is estimated at least a few minutes.";
        else
            if(nSize < 140000)
                strHtmlText+= "			<BR>According to the current DB size, the conversion time is estimated at least a few hours.";
            else
                strHtmlText+= "			<BR>According to the current DB size, the conversion time is estimated to a few days.";
        strHtmlText+= "			<li><b>Free disk space</b>";
        strHtmlText+= "			<BR>Moreover, the SQL server must have enough free HDD space for the full process to success.";
        strHtmlText+= "			<BR>"+QString("It would need around 6 times the current DB size (%1Mo * 6 = %2Mo).\n").arg(nSize).arg(nSize*6);
        strHtmlText+= "			</ul>";
        strHtmlText+= "		  </td>";
        strHtmlText+= "		</tr>";
        strHtmlText+= "	  </tbody></table>";
        strHtmlText+= "	</td>";
        strHtmlText+= "  </tr>";
        strHtmlText+= "  <tr>";
        strHtmlText+= "	<td></td>";
        strHtmlText+= "  </tr>";
        strHtmlText+= "</tbody></table>";

        strHtmlText+= "<table width=\"100%\"><tbody>";
        strHtmlText+= "  <tr>";
        strHtmlText+= "	<td halign=\"center\" height=\"29\" valign=\"middle\" width=\"15\"><img src=\":/gex/icons/gex_history_arrow_orange.png\" height=\"9\" width=\"10\"></td>";
        strHtmlText+= "	<td align=\"left\" height=\"29\"><p align=\"left\">";
        strHtmlText+= "	<font size=\"4\" face=\"arial\" color=\"#006699\">Before to start</font></p></td>";
        strHtmlText+= "  </tr>";
        strHtmlText+= "  <tr>";
        strHtmlText+= "	<td height=\"1\" width=\"15\"></td>";
        strHtmlText+= "	<td height=\"1\"></td>";
        strHtmlText+= "  </tr>";
        strHtmlText+= "  <tr>";
        strHtmlText+= "	<td width=\"15\"></td>";
        strHtmlText+= "	<td><b><font size=\"5\"></font></b>";
        strHtmlText+= "	  <table width=\"100%\" border=\"1\" cellpadding=\"2\" cellspacing=\"0\"><tbody>";
        strHtmlText+= "	  <tr>";
        strHtmlText+= "		  <td valign=\"top\">";
        strHtmlText+= "			<ul>";
        strHtmlText+= "			<li><b>Check your MySql Server configuration</b>";
        strHtmlText+= "			<BR>- GLOBAL_VARIABLES[VERSION] > 5.1";
        strHtmlText+= "			<BR>- GLOBAL_VARIABLES[HAVE_PARTITIONING] = YES";
        strHtmlText+= "			<BR>- GLOBAL_VARIABLES[HAVE_INNODB] = YES";
        strHtmlText+= "			<BR>- GLOBAL_VARIABLES[INNODB_FILE_PER_TABLE] = ON";
        strHtmlText+= "			</ul>";
        strHtmlText+= "		  </td>";
        strHtmlText+= "		</tr>";
        strHtmlText+= "	  </tbody></table>";
        strHtmlText+= "	</td>";
        strHtmlText+= "  </tr>";
        strHtmlText+= "  <tr>";
        strHtmlText+= "	<td></td>";
        strHtmlText+= "  </tr>";
        strHtmlText+= "</tbody></table>";


        strHtmlText+= "<table width=\"100%\"><tbody>";
        strHtmlText+= "  <tr>";
        strHtmlText+= "	<td halign=\"center\" height=\"29\" valign=\"middle\" width=\"15\"><img src=\":/gex/icons/gex_history_arrow_orange.png\" height=\"9\" width=\"10\"></td>";
        strHtmlText+= "	<td align=\"left\" height=\"29\"><p align=\"left\">";
        strHtmlText+= "	<font size=\"4\" face=\"arial\" color=\"#006699\">Why InnoDB storage engine ?</font></p></td>";
        strHtmlText+= "  </tr>";
        strHtmlText+= "  <tr>";
        strHtmlText+= "	<td height=\"1\" width=\"15\"></td>";
        strHtmlText+= "	<td height=\"1\"></td>";
        strHtmlText+= "  </tr>";
        strHtmlText+= "  <tr>";
        strHtmlText+= "	<td width=\"15\"></td>";
        strHtmlText+= "	<td><b><font size=\"5\"></font></b>";
        strHtmlText+= "	  <table width=\"100%\" border=\"1\" cellpadding=\"2\" cellspacing=\"0\"><tbody>";
        strHtmlText+= "	  <tr>";
        strHtmlText+= "		  <td valign=\"top\">";
        strHtmlText+= "			<ul>";
        strHtmlText+= "			<li><b>Data-integrity</b>";
        strHtmlText+= "			<BR>InnoDB is transaction-safe meaning data-integrity is maintained throughout the entire query process. ";
        strHtmlText+= "			<BR>InnoDB also provides row-locking, as opposed to table-locking, meaning while one query is busy updating or inserting a row, another query can update a different row at the same time. ";
        strHtmlText+= "			<BR>These features increase multi-user concurrency and performance.";
        strHtmlText+= "			<BR>";
        strHtmlText+= "			<li><b>Faster in write-intensive</b>";
        strHtmlText+= "			<BR>Because of its row-locking feature InnoDB is said to thrive in high load environments. ";
        strHtmlText+= "			<BR>Its CPU efficiency is probably not matched by any other disk-based relational database engine. ";
        strHtmlText+= "			</ul>";
        strHtmlText+= "		  </td>";
        strHtmlText+= "		</tr>";
        strHtmlText+= "	  </tbody></table>";
        strHtmlText+= "	</td>";
        strHtmlText+= "  </tr>";
        strHtmlText+= "  <tr>";
        strHtmlText+= "	<td></td>";
        strHtmlText+= "  </tr>";
        strHtmlText+= "</tbody></table>";
    }
    else if(strCommand == "Barracuda")
    {
        // InnoDb convertion

        int nSize=1;
        m_pTdrDatabaseEntry->m_pExternalDatabase->GetTotalSize(nSize);

        strHtmlText+= "<table width=\"100%\"><tbody>";
        strHtmlText+= "  <tr>";
        strHtmlText+= "	<td halign=\"center\" height=\"29\" valign=\"middle\" width=\"15\"><img src=\":/gex/icons/gex_history_arrow_orange.png\" height=\"9\" width=\"10\"></td>";
        strHtmlText+= "	<td align=\"left\" height=\"29\"><p align=\"left\">";
        strHtmlText+= "	<font size=\"4\" face=\"arial\" color=\"#006699\">InnoDB Barracuda compression</font></p></td>";
        strHtmlText+= "  </tr>";
        strHtmlText+= "  <tr>";
        strHtmlText+= "	<td height=\"1\" width=\"15\"></td>";
        strHtmlText+= "	<td height=\"1\"></td>";
        strHtmlText+= "  </tr>";
        strHtmlText+= "  <tr>";
        strHtmlText+= "	<td width=\"15\"></td>";
        strHtmlText+= "	<td><b><font size=\"5\"></font></b>";
        strHtmlText+= "	  <table width=\"100%\" border=\"1\" cellpadding=\"2\" cellspacing=\"0\"><tbody>";
        strHtmlText+= "	  <tr>";
        strHtmlText+= "		  <td valign=\"top\">";
        strHtmlText+= "			<ul>";
        strHtmlText+= "			<li><b>Conversion time</b>";
        strHtmlText+= "			<BR>The update to the InnoDB storage engine may take several hours to a few days, depending on your database size.";
        // Update the message according to the current database size
        // table less that 140Gb = <24h
        if(nSize < 1400)
            strHtmlText+= "			<BR>According to the current DB size, the conversion time is estimated at least a few minutes.";
        else
            if(nSize < 140000)
                strHtmlText+= "			<BR>According to the current DB size, the conversion time is estimated at least a few hours.";
            else
                strHtmlText+= "			<BR>According to the current DB size, the conversion time is estimated to a few days.";
        strHtmlText+= "			</ul>";
        strHtmlText+= "		  </td>";
        strHtmlText+= "		</tr>";
        strHtmlText+= "	  </tbody></table>";
        strHtmlText+= "	</td>";
        strHtmlText+= "  </tr>";
        strHtmlText+= "  <tr>";
        strHtmlText+= "	<td></td>";
        strHtmlText+= "  </tr>";
        strHtmlText+= "</tbody></table>";

        strHtmlText+= "<table width=\"100%\"><tbody>";
        strHtmlText+= "  <tr>";
        strHtmlText+= "	<td halign=\"center\" height=\"29\" valign=\"middle\" width=\"15\"><img src=\":/gex/icons/gex_history_arrow_orange.png\" height=\"9\" width=\"10\"></td>";
        strHtmlText+= "	<td align=\"left\" height=\"29\"><p align=\"left\">";
        strHtmlText+= "	<font size=\"4\" face=\"arial\" color=\"#006699\">Before to start</font></p></td>";
        strHtmlText+= "  </tr>";
        strHtmlText+= "  <tr>";
        strHtmlText+= "	<td height=\"1\" width=\"15\"></td>";
        strHtmlText+= "	<td height=\"1\"></td>";
        strHtmlText+= "  </tr>";
        strHtmlText+= "  <tr>";
        strHtmlText+= "	<td width=\"15\"></td>";
        strHtmlText+= "	<td><b><font size=\"5\"></font></b>";
        strHtmlText+= "	  <table width=\"100%\" border=\"1\" cellpadding=\"2\" cellspacing=\"0\"><tbody>";
        strHtmlText+= "	  <tr>";
        strHtmlText+= "		  <td valign=\"top\">";
        strHtmlText+= "			<ul>";
        strHtmlText+= "			<li><b>Check your MySql Server configuration</b>";
        strHtmlText+= "			<BR>- GLOBAL_VARIABLES[VERSION] > 5.5";
        strHtmlText+= "			<BR>- GLOBAL_VARIABLES[INNODB_FILE_FORMAT] = Barracuda";
        strHtmlText+= "			</ul>";
        strHtmlText+= "		  </td>";
        strHtmlText+= "		</tr>";
        strHtmlText+= "	  </tbody></table>";
        strHtmlText+= "	</td>";
        strHtmlText+= "  </tr>";
        strHtmlText+= "  <tr>";
        strHtmlText+= "	<td></td>";
        strHtmlText+= "  </tr>";
        strHtmlText+= "</tbody></table>";


        strHtmlText+= "<table width=\"100%\"><tbody>";
        strHtmlText+= "  <tr>";
        strHtmlText+= "	<td halign=\"center\" height=\"29\" valign=\"middle\" width=\"15\"><img src=\":/gex/icons/gex_history_arrow_orange.png\" height=\"9\" width=\"10\"></td>";
        strHtmlText+= "	<td align=\"left\" height=\"29\"><p align=\"left\">";
        strHtmlText+= "	<font size=\"4\" face=\"arial\" color=\"#006699\">Why InnoDB Barracuda ?</font></p></td>";
        strHtmlText+= "  </tr>";
        strHtmlText+= "  <tr>";
        strHtmlText+= "	<td height=\"1\" width=\"15\"></td>";
        strHtmlText+= "	<td height=\"1\"></td>";
        strHtmlText+= "  </tr>";
        strHtmlText+= "  <tr>";
        strHtmlText+= "	<td width=\"15\"></td>";
        strHtmlText+= "	<td><b><font size=\"5\"></font></b>";
        strHtmlText+= "	  <table width=\"100%\" border=\"1\" cellpadding=\"2\" cellspacing=\"0\"><tbody>";
        strHtmlText+= "	  <tr>";
        strHtmlText+= "		  <td valign=\"top\">";
        strHtmlText+= "			<ul>";
        strHtmlText+= "			<li><b>InnoDB Data Compression</b>";
        strHtmlText+= "			<BR>By setting InnoDB configuration options, you can update Quantix tables where the data is stored in compressed form. ";
        strHtmlText+= "			<BR>The compression means less data is transferred between disk and memory, and takes up less space in memory. ";
        strHtmlText+= "			<BR>The benefits are amplified for tables with secondary indexes, because index data is compressed also. ";
        strHtmlText+= "			</ul>";
        strHtmlText+= "		  </td>";
        strHtmlText+= "		</tr>";
        strHtmlText+= "	  </tbody></table>";
        strHtmlText+= "	</td>";
        strHtmlText+= "  </tr>";
        strHtmlText+= "  <tr>";
        strHtmlText+= "	<td></td>";
        strHtmlText+= "  </tr>";
        strHtmlText+= "</tbody></table>";
    }
    else if(strCommand == "IncrementalUpdate")
    {
        // InnoDb convertion

        // Get incremental updates
        uint			uiIncrementalSplitlots = 0;
        QMap< QString,QMap< QString,QString > > lIncrementalUpdates;
        QString         lIncrementalName;

        if (m_TdrDbPlugin == NULL)
            return false;

        m_TdrDbPlugin->GetIncrementalUpdatesSettings(lIncrementalUpdates);

        foreach(lIncrementalName, lIncrementalUpdates.keys())
            uiIncrementalSplitlots+=lIncrementalUpdates[lIncrementalName]["remaining_splitlots"].toInt();

        if(uiIncrementalSplitlots == 0)
            return false;

        strHtmlText+= "<table width=\"100%\"><tbody>";
        strHtmlText+= "  <tr>";
        strHtmlText+= "	<td halign=\"center\" height=\"29\" valign=\"middle\" width=\"15\"><img src=\":/gex/icons/gex_history_arrow_orange.png\" height=\"9\" width=\"10\"></td>";
        strHtmlText+= "	<td align=\"left\" height=\"29\"><p align=\"left\">";
        strHtmlText+= "	<font size=\"4\" face=\"arial\" color=\"#006699\">Quantix Incremental Update</font></p></td>";
        strHtmlText+= "  </tr>";
        strHtmlText+= "  <tr>";
        strHtmlText+= "	<td height=\"1\" width=\"15\"></td>";
        strHtmlText+= "	<td height=\"1\"></td>";
        strHtmlText+= "  </tr>";
        strHtmlText+= "  <tr>";
        strHtmlText+= "	<td width=\"15\"></td>";
        strHtmlText+= "	<td><b><font size=\"5\"></font></b>";
        strHtmlText+= "	  <table width=\"100%\" border=\"1\" cellpadding=\"2\" cellspacing=\"0\"><tbody>";
        strHtmlText+= "	  <tr>";
        strHtmlText+= "		  <td valign=\"top\">Update Name</td>";
        strHtmlText+= "		  <td valign=\"top\">Remaining splitlots</td>";
        strHtmlText+= "		  <td valign=\"top\">Description</td>";
        strHtmlText+= "		</tr>";
        foreach(lIncrementalName, lIncrementalUpdates.keys())
        {
            if(lIncrementalUpdates[lIncrementalName]["remaining_splitlots"].toInt() == 0)
                continue;

            strHtmlText+= "	  <tr>";
            strHtmlText+= "		  <td valign=\"top\">";
            strHtmlText+=lIncrementalUpdates[lIncrementalName]["db_update_name"];
            strHtmlText+= "		  </td>";
            strHtmlText+= "		  <td valign=\"top\">";
            strHtmlText+=lIncrementalUpdates[lIncrementalName]["remaining_splitlots"];
            strHtmlText+= "		  </td>";
            strHtmlText+= "		  <td valign=\"top\">";
            strHtmlText+=lIncrementalUpdates[lIncrementalName]["db_update_name_description"];
            strHtmlText+= "		  </td>";

            strHtmlText+= "		</tr>";
        }

        strHtmlText+= "	  </tbody></table>";
        strHtmlText+= "	</td>";
        strHtmlText+= "  <tr>";
        strHtmlText+= "	<td width=\"15\"></td>";
        strHtmlText+= "	<td>";
        strHtmlText+= "			<li><b>"+QString::number(uiIncrementalSplitlots)+ " splitlots to update</b>";
        strHtmlText+= "</td>";
        strHtmlText+= "  </tr>";
        strHtmlText+= "  </tr>";
        strHtmlText+= "  <tr>";
        strHtmlText+= "	<td></td>";
        strHtmlText+= "  </tr>";
        strHtmlText+= "</tbody></table>";
        strHtmlText+= "			<li>Click 'Run all' to process all incremental updates.";
        strHtmlText+= "			<li>Click 'Run once' to run one pass of incremental updates.";
    }

    if(!AdrStatus.isEmpty())
        strHtmlText+= "<img src=\""+QString::fromUtf8(":/gex/icons/warning.png")+"\">"+AdrStatus+"";

    if(strHtmlText.isEmpty())
        strHtmlText+= "<h2><img src=\""+QString::fromUtf8(":/gex/icons/warning.png")+"\"> No information available</h2>";

    strHtmlText+="<BR>";

    if(pTextEdit_Log == NULL)
    {
        QDialog clDialogBox;
        QVBoxLayout clVBoxLayout;
        QHBoxLayout clHBoxLayout;
        QButtonGroup clButtonGroup;

        QLabel clTopLabel;
        QTextBrowser clHtmlText;
        QLabel clBottomLabel;
        QPushButton clButtonOne;
        QPushButton clButtonTwo;
        QPushButton clButtonThree;

        clHtmlText.setReadOnly(true);
        clHtmlText.setAcceptDrops(false);
        clHtmlText.setProperty("textInteractionFlags",Qt::NoTextInteraction);
        clHtmlText.setLineWrapMode(QTextEdit::NoWrap);
        clDialogBox.setLayout(&clVBoxLayout);
        clDialogBox.setWindowTitle("Quantix Database Update");
        clTopLabel.setText(strTopMessage);
        clVBoxLayout.addWidget(&clTopLabel);
        clHtmlText.setText(strHtmlText);
        clVBoxLayout.addWidget(&clHtmlText);
        clBottomLabel.setText(strBottomMessage);
        clVBoxLayout.addWidget(&clBottomLabel);

        if(uiFromDbVersion_Build < 999)
        {
            // DbUpdate or IncrementalUpdate
            if(strCommand == "IncrementalUpdate")
            {
                clButtonOne.setText("Yes");
                clButtonGroup.addButton(&clButtonOne, QMessageBox::Yes);
                clHBoxLayout.addWidget(&clButtonOne);
                clButtonOne.setText("Run once");
                clButtonTwo.setText("Run all");
                clButtonThree.setText("Cancel");

                clButtonGroup.addButton(&clButtonTwo,QMessageBox::YesToAll);
                clButtonGroup.addButton(&clButtonThree,QMessageBox::Cancel);

                clHBoxLayout.addWidget(&clButtonTwo);
                clHBoxLayout.addWidget(&clButtonThree);
            }
            else if(strCommand.startsWith("UpdateDbFrom",Qt::CaseInsensitive))
            {
                if(uiFromDbVersion_Build+1 < uiToDbVersion_Build)
                {
                    clButtonOne.setText("Yes");
                    clButtonThree.setText("Not now");

                    clButtonGroup.addButton(&clButtonOne, QMessageBox::YesToAll);
                    clButtonGroup.addButton(&clButtonThree,QMessageBox::Cancel);

                    clHBoxLayout.addWidget(&clButtonOne);
                    clHBoxLayout.addWidget(&clButtonThree);
                }
                else
                {
                    clButtonOne.setText("Yes");
                    clButtonGroup.addButton(&clButtonOne, QMessageBox::YesToAll);
                    clHBoxLayout.addWidget(&clButtonOne);
                    clButtonTwo.setText("Not now");
                    clButtonGroup.addButton(&clButtonTwo,QMessageBox::Cancel);
                    clHBoxLayout.addWidget(&clButtonTwo);
                }

            }
            else
            {
                clButtonOne.setText("Yes");
                clButtonGroup.addButton(&clButtonOne, QMessageBox::Yes);
                clHBoxLayout.addWidget(&clButtonOne);
                clButtonTwo.setText("Not now");
                clButtonGroup.addButton(&clButtonTwo,QMessageBox::Cancel);
                clHBoxLayout.addWidget(&clButtonTwo);
            }
        }
        else
        {
            clButtonOne.setText("Close");
            clButtonGroup.addButton(&clButtonOne, QMessageBox::Cancel);
            clHBoxLayout.addWidget(&clButtonOne);
        }

        clVBoxLayout.addLayout(&clHBoxLayout);

        QObject::connect(&clButtonGroup, SIGNAL(buttonClicked(int)), &clDialogBox, SLOT(done(int)));

        clDialogBox.setModal(true);
        clDialogBox.exec();
        int nResult = clDialogBox.result();
        if((nResult != QMessageBox::Cancel) && (nResult != QMessageBox::Yes) && (nResult != QMessageBox::YesToAll))
            nResult = QMessageBox::Cancel;

        if(strCommand.startsWith("UpdateDbFrom",Qt::CaseInsensitive)
                && (nResult == QMessageBox::YesToAll))
        {
            if((uiFromDbVersion_Major != uiToDbVersion_Major)
                    && (m_pTdrDatabaseEntry->StatusFlags() & STATUS_DBCOMPATIBLE))
            {
                // If you update this database to GEXDB V2.01 B23, ALL Examinator users
                // will need to also update their Examinator version to V7.1 in order to
                // connect to this database version.
                strBottomMessage = "<b>This database version is compatible with your current application version.</b><BR> "
                                   "If you don't need new database features, you don't have to update.<BR><BR>"
                                   "If you update this database to <b><font color=\"red\">V"
                        +QString::number(uiToDbVersion_Major)+"."+QString::number(uiToDbVersion_Minor)+"</font></b>"
                        +", <b><font color=\"red\">all</font></b> Examinator users will need to use <b><font color=\"red\">Examinator V"
                        +GS::Gex::Engine::GetInstance().Get("AppVersionMajor").toString()+"."
                        +GS::Gex::Engine::GetInstance().Get("AppVersionMinor").toString()
                        +"</font></b> in order to connect to this database version.<BR><BR>"
                         "Do you want to update your database?";
                nResult = QMessageBox::warning(
                            this,"Database Update Compatibility",
                            strBottomMessage,QMessageBox::Yes,QMessageBox::Cancel);
                if(nResult != QMessageBox::Yes)
                    nResult = QMessageBox::Cancel;
                else
                    nResult = QMessageBox::YesToAll;
            }
        }
        QObject::disconnect(&clButtonGroup, SIGNAL(buttonClicked(int)), &clDialogBox, SLOT(done(int)));
        return nResult;
    }
    else
    {
        pTextEdit_Log->clear();
        pTextEdit_Log->insertHtml(strTopMessage);
        pTextEdit_Log->insertHtml(strHtmlText);
        pTextEdit_Log->insertHtml(strBottomMessage);
    }

    return QMessageBox::Cancel;
}

QString GexSqlHousekeepingDialog::EventStatus()
{
    GexDbPlugin_Query lQuery(m_TdrDbPlugin, QSqlDatabase::database(m_TdrDbPlugin->m_pclDatabaseConnector->m_strConnectionName));
    QString lQueryStr, lBGStatus, lEventStatus, lFrequencyValue, lFrequencyField;
    int lFieldNo = -1;

    // first check if exist
    lQueryStr = "SHOW EVENTS "
                "WHERE name='" + QString(BACKGROUND_EVENT) + "' "
                "AND db='" + m_TdrDbPlugin->m_pclDatabaseConnector->m_strSchemaName + "'";
    if(!lQuery.Execute(lQueryStr))
    {
        QString lMsg = "QUERY=" + lQueryStr + " ERROR=" + lQuery.lastError().text();
        GSLOG(SYSLOG_SEV_ERROR, lMsg.toLatin1().constData() );
        UpdateLogMessage(editLog_Background, lMsg.append("\n"), true);
        lEventStatus = "ERROR";
    }
    else
    {
        if (!lQuery.first())
            lEventStatus = "NO EVENT";
        else
        {
            // then check status
            lFieldNo = lQuery.record().indexOf("Status");
            lEventStatus = lQuery.value(lFieldNo).toString();

            lFieldNo = lQuery.record().indexOf("Interval value");
            lFrequencyValue = lQuery.value(lFieldNo).toString();

            lFieldNo = lQuery.record().indexOf("Interval field");
            lFrequencyField = lQuery.value(lFieldNo).toString();

            lEventStatus.append(" | EVERY " + lFrequencyValue + " " + lFrequencyField);
        }
    }

    int lCurrentUpdateId = 0;
    lQueryStr = "CALL background_update_id(@CurrentUpdateId)";
    if(!lQuery.Execute(lQueryStr))
    {
        QString lMsg = "QUERY=" + lQueryStr + " ERROR=" + lQuery.lastError().text();
        GSLOG(SYSLOG_SEV_ERROR, lMsg.toLatin1().constData() );
        UpdateLogMessage(editLog_Background, lMsg.append("\n"), true);
        lEventStatus = "ERROR";
    }
    lQueryStr = "SELECT @CurrentUpdateId";
    if(!lQuery.Execute(lQueryStr))
    {
        QString lMsg = "QUERY=" + lQueryStr + " ERROR=" + lQuery.lastError().text();
        GSLOG(SYSLOG_SEV_ERROR, lMsg.toLatin1().constData() );
        UpdateLogMessage(editLog_Background, lMsg.append("\n"), true);
        lEventStatus = "ERROR";
    }
    // A background process is running
    lQuery.First();
    lCurrentUpdateId = lQuery.value("@CurrentUpdateId").toInt();

    lQueryStr = "SELECT status FROM background_transfer_history WHERE update_id="+QString::number(lCurrentUpdateId);
    if(!lQuery.Execute(lQueryStr))
    {
        QString lMsg = "QUERY=" + lQueryStr + " ERROR=" + lQuery.lastError().text();
        GSLOG(SYSLOG_SEV_ERROR, lMsg.toLatin1().constData() );
        UpdateLogMessage(editLog_Background, lMsg.append("\n"), true);
    }
    else
    {
        if (!lQuery.first())
            lBGStatus = "NO STATUS";
        else
        {
            // then check status
            lFieldNo = lQuery.record().indexOf("Status");
            lBGStatus = lQuery.value(lFieldNo).toString();
        }
    }

    labelBGStatusValue->setStyleSheet("QLabel { color : black; }");
    QString lMessage = lEventStatus;
    if(lBGStatus != "DONE")
    {
        if((lEventStatus == "NO EVENT") || (lEventStatus == "ERROR"))
        {
            lMessage = "WARNING: You need to start the Background Process!";
            labelBGStatusValue->setStyleSheet("QLabel { color : red; }");
        }
        else if(lEventStatus.contains("ENABLED",Qt::CaseInsensitive))
        {
            lMessage = "Data transfer RUNNING...";
            labelBGStatusValue->setStyleSheet("QLabel { color : green; }");
        }
        else
        {
            lMessage = "WARNING: You need to enable the Background Process event!";
            labelBGStatusValue->setStyleSheet("QLabel { color : red; }");
        }
    }
    else if(lBGStatus == "DONE")
    {
        if(lEventStatus != "NO EVENT")
        {
            lMessage = "Data transfer COMPLETED. You can Stop the Background Process...";
            labelBGStatusValue->setStyleSheet("QLabel { color : orange; }");
        }
        else
        {
            lEventStatus += "|DONE";
            lMessage = "Data transfer COMPLETED.";
        }
    }


    labelBGStatusValue->setText(lMessage);

    return lEventStatus;
}

void GexSqlHousekeepingDialog::UpdateStandardLog(const QString &message, bool isPlainText)
{
    UpdateLogMessage(editLog, message, isPlainText);
}

void GexSqlHousekeepingDialog::IncrementalUpdateLog(const QString &message, bool isPlainText)
{
    UpdateLogMessage(editLog_Incremental, message, isPlainText);
}

void GexSqlHousekeepingDialog::UpdateMetadataLog(const QString &message, bool isPlainText)
{
    UpdateLogMessage(editLog_MetaData, message, isPlainText);
}

void GexSqlHousekeepingDialog::UpdateLogMessage(QTextEdit *textEdit, const QString &message, bool isPlainText)
{
    QString lText = message;
    if(!isPlainText && message.startsWith("o "))
        lText = "<b>"+message+"</b>";

    if(isPlainText)
        textEdit->insertPlainText(lText);
    else
        textEdit->insertHtml(lText);

    textEdit->moveCursor(QTextCursor::End);
    textEdit->ensureCursorVisible();

    QCoreApplication::processEvents();
}

void GexSqlHousekeepingDialog::UpdateProgress(int prog)
{
    if(mCurrentProgressBar)
        mCurrentProgressBar->setValue(prog);
}

void GexSqlHousekeepingDialog::ResetProgress(bool forceCompleted)
{
    int lProgress;
    if(mCurrentProgressBar)
    {
        if(forceCompleted == true)
            lProgress = 100;
        else
        {
            lProgress = 0;
            mCurrentProgressBar->setMaximum(100);
            mCurrentProgressBar->setTextVisible(true);
            mCurrentProgressBar->show();
        }
        mCurrentProgressBar->setValue(lProgress);
    }
}

void GexSqlHousekeepingDialog::SetMaxProgress(int max)
{
    if(mCurrentProgressBar)
        mCurrentProgressBar->setMaximum(max);
}

void GexSqlHousekeepingDialog::OnPurgeDataBase()
{
    QWidget *lSender = qobject_cast<QWidget *>(sender());
    GexOneQueryWizardPage1 * lWidget = 0;
    if(lSender)
        lWidget = qobject_cast<GexOneQueryWizardPage1 *>(lSender);

    if(!lSender || !lWidget)
    {
        GS::Gex::Message::warning("Purge Database", "Can not retrieve database request");
        return ;
    }

    int lAnswer = QMessageBox::No;
    QString lDB = editDatabaseName->text();
    if(lDB.startsWith("[Local]") || lDB.startsWith("[Server]"))
        lDB = lDB.section("]",1).trimmed();

    if(!GS::Gex::Engine::GetInstance().GetDatabaseEngine().isYMDB(lDB))
    {
        lAnswer = QMessageBox::question(this,
                                        "Purge Database",
                                        QString("Are you sure to purge the Database <%1> ?").arg(lDB),
                                        QMessageBox::Yes,QMessageBox::No);
    }
    else
    {
        lAnswer = QMessageBox::question(this,
                                        "Purge Database",
                                        QString("Are you sure to purge the invalid entry in ""Yield Man"" Database <%1> ?").arg(lDB),
                                        QMessageBox::Yes,QMessageBox::No);
    }
    if(lAnswer == QMessageBox::No)
        return ;

    m_pTdrDatabaseEntry->TraceUpdate("HOUSEKEEPING","INFO","Start purging Database Process");

    progressBar_DBPurge->show();
    progressBar_DBPurge->reset();

    setEnabled(false);

    GexDatabaseQuery oQuery;
    lWidget->GetQueryFields(oQuery);

    bool lRet = GS::Gex::Engine::GetInstance().GetDatabaseEngine().PurgeDataBase(
                &oQuery, progressBar_DBPurge);
    if(!lRet)
    {
        m_pTdrDatabaseEntry->TraceUpdate(
                    "HOUSEKEEPING","INFO","Start purging Database Process error: The purge did not complete successfully");
        GS::Gex::Message::warning(
                    "Purge Database",
                    "The purge did not complete successfully. Please check log files.");
    }
    else
        GS::Gex::Message::information("Purge Database",
                                      "The purge complete successfully.");

    setEnabled(true);
}

void GexSqlHousekeepingDialog::OnStartStopBGEvent()
{
    // check status
    QString lStatus = EventStatus();

    if (lStatus == "ERROR")
        return;
    // if status: no event
    else if (lStatus == "NO EVENT")
    {
        CreateBackgroundEvent();
        EnableBackgroundEvent();
    }
    // if status: exists
    else
    {
        // check if still things to do
        QString lBGStatus, lQueryStr;
        GexDbPlugin_Query lQuery(m_TdrDbPlugin, QSqlDatabase::database(m_TdrDbPlugin->m_pclDatabaseConnector->m_strConnectionName));
        int lFieldNo;
        int lCurrentUpdateId = 0;
        lQueryStr = "CALL background_update_id(@CurrentUpdateId)";
        if(!lQuery.Execute(lQueryStr))
        {
            QString lMsg = "QUERY=" + lQueryStr + " ERROR=" + lQuery.lastError().text();
            GSLOG(SYSLOG_SEV_ERROR, lMsg.toLatin1().constData() );
            UpdateLogMessage(editLog_Background, lMsg.append("\n"), true);
        }
        lQueryStr = "SELECT @CurrentUpdateId";
        if(!lQuery.Execute(lQueryStr))
        {
            QString lMsg = "QUERY=" + lQueryStr + " ERROR=" + lQuery.lastError().text();
            GSLOG(SYSLOG_SEV_ERROR, lMsg.toLatin1().constData() );
            UpdateLogMessage(editLog_Background, lMsg.append("\n"), true);
        }
        // A background process is running
        lQuery.First();
        lCurrentUpdateId = lQuery.value("@CurrentUpdateId").toInt();

        lQueryStr = "SELECT status FROM background_transfer_history WHERE update_id="+QString::number(lCurrentUpdateId);
        if(!lQuery.Execute(lQueryStr))
        {
            QString lMsg = "QUERY=" + lQueryStr + " ERROR=" + lQuery.lastError().text();
            GSLOG(SYSLOG_SEV_ERROR, lMsg.toLatin1().constData() );
            UpdateLogMessage(editLog_Background, lMsg.append("\n"), true);
        }
        else
        {
            if (!lQuery.first())
                lBGStatus = "NO STATUS";
            else
            {
                // then check status
                lFieldNo = lQuery.record().indexOf("Status");
                lBGStatus = lQuery.value(lFieldNo).toString();
            }
        }

        if(lBGStatus != "DONE")
        {
            QString lMessage = "You can resume the background transfer later,\n"
                               "Do you want to stop the background transfer?\n";
            bool lOk;
            GS::Gex::Message::request("Stop background transfer", lMessage, lOk);
            if (! lOk)
            {
                return;
            }
        }

        DisableBackgroundEvent();
        DropBackgroundEvent();
    }
    // update button
    UpdateStartStopBGEventButton();
}

void GexSqlHousekeepingDialog::CreateBackgroundEvent()
{
    GexDbPlugin_Query lQuery(m_TdrDbPlugin, QSqlDatabase::database(m_TdrDbPlugin->m_pclDatabaseConnector->m_strConnectionName));
    QString lQueryStr, lEventFrequency;
    int lFieldNo = -1;

    int lCurrentUpdateId = 0;
    lQueryStr = "CALL background_update_id(@CurrentUpdateId)";
    if(!lQuery.Execute(lQueryStr))
    {
        QString lMsg = "QUERY=" + lQueryStr + " ERROR=" + lQuery.lastError().text();
        GSLOG(SYSLOG_SEV_ERROR, lMsg.toLatin1().constData());
        UpdateLogMessage(editLog_Background, lMsg, true);
        UpdateBGLogMessages("TRANSFER EVENT", "Create event", "ERROR", lMsg);
        return;
    }
    lQueryStr = "SELECT @CurrentUpdateId";
    if(!lQuery.Execute(lQueryStr))
    {
        QString lMsg = "QUERY=" + lQueryStr + " ERROR=" + lQuery.lastError().text();
        GSLOG(SYSLOG_SEV_ERROR, lMsg.toLatin1().constData());
        UpdateLogMessage(editLog_Background, lMsg, true);
        UpdateBGLogMessages("TRANSFER EVENT", "Create event", "ERROR", lMsg);
        return;
    }
    // A background process is running
    lQuery.First();
    lCurrentUpdateId = lQuery.value("@CurrentUpdateId").toInt();

    // Getting frequency
    lQueryStr = "SELECT setting_value "
                "FROM background_transfer_settings "
                "WHERE update_id="+QString::number(lCurrentUpdateId)+" AND setting_key='BACKGROUND_TRANSFER_SCHEDULER_FREQUENCY'";
    if(!lQuery.Execute(lQueryStr) || !lQuery.first())
    {
        QString lMsg = "QUERY=" + lQueryStr + " ERROR=" + lQuery.lastError().text();
        GSLOG(SYSLOG_SEV_ERROR, lMsg.toLatin1().constData());
        UpdateLogMessage(editLog_Background, lMsg, true);
        UpdateBGLogMessages("TRANSFER EVENT", "Create event", "ERROR", lMsg);
        return;
    }

    lFieldNo = lQuery.record().indexOf("setting_value");
    lEventFrequency = lQuery.value(lFieldNo).toString();
    // Build query
    lQueryStr = "CREATE EVENT IF NOT EXISTS " + QString(BACKGROUND_EVENT) + " "
                "ON SCHEDULE "
                "EVERY " + lEventFrequency + " "
                "DISABLE "
                "DO "
                "CALL background_transfer_step(@status, @message);";

    // Exec Query
    if(!lQuery.Execute(lQueryStr))
    {
        QString lMsg = "QUERY=" + lQueryStr + " ERROR=" + lQuery.lastError().text();
        GSLOG(SYSLOG_SEV_ERROR, lMsg.toLatin1().constData());
        UpdateLogMessage(editLog_Background, lMsg.append("\n"), true);
        UpdateBGLogMessages("TRANSFER EVENT", "Create event", "ERROR", lMsg);
        return;
    }

    UpdateBGLogMessages("TRANSFER EVENT", "Create event", "DONE", QString());
    UpdateLogMessage(editLog_Background, "Background transfer event successfuly created\n", true);
}

void GexSqlHousekeepingDialog::DropBackgroundEvent()
{
    GexDbPlugin_Query lQuery(m_TdrDbPlugin, QSqlDatabase::database(m_TdrDbPlugin->m_pclDatabaseConnector->m_strConnectionName));
    QString lQueryStr;
    lQueryStr = "DROP EVENT IF EXISTS " + QString(BACKGROUND_EVENT) + " ";
    // Exec Query
    if(!lQuery.Execute(lQueryStr))
    {
        QString lMsg = "QUERY=" + lQueryStr + " ERROR=" + lQuery.lastError().text();
        GSLOG(SYSLOG_SEV_ERROR, lMsg.toLatin1().constData());
        UpdateLogMessage(editLog_Background, lMsg.append("\n"), true);
        UpdateBGLogMessages("TRANSFER EVENT", "Drop event", "ERROR", lMsg);
        return;
    }

    UpdateBGLogMessages("TRANSFER EVENT", "Drop event", "DONE", QString());
    UpdateLogMessage(editLog_Background, "Background transfer event successfuly dropped\n", true);
}

void GexSqlHousekeepingDialog::DisableBackgroundEvent()
{
    GexDbPlugin_Query lQuery(m_TdrDbPlugin, QSqlDatabase::database(m_TdrDbPlugin->m_pclDatabaseConnector->m_strConnectionName));
    QString lQueryStr;
    lQueryStr = "ALTER EVENT " + QString(BACKGROUND_EVENT) + " "
                                                             "DISABLE;";
    // Exec Query
    if(!lQuery.Execute(lQueryStr))
    {
        QString lMsg = "QUERY=" + lQueryStr + " ERROR=" + lQuery.lastError().text();
        GSLOG(SYSLOG_SEV_ERROR, lMsg.toLatin1().constData());
        UpdateLogMessage(editLog_Background, lMsg.append("\n"), true);
        UpdateBGLogMessages("TRANSFER EVENT", "Disable event", "ERROR", lMsg);
        return;
    }

    UpdateBGLogMessages("TRANSFER EVENT", "Disable event", "DONE", QString());
    UpdateLogMessage(editLog_Background, "Background transfer event successfuly disabled\n", true);
}

void GexSqlHousekeepingDialog::EnableBackgroundEvent()
{
    GexDbPlugin_Query lQuery(m_TdrDbPlugin, QSqlDatabase::database(m_TdrDbPlugin->m_pclDatabaseConnector->m_strConnectionName));
    QString lQueryStr;
    lQueryStr = "ALTER EVENT " + QString(BACKGROUND_EVENT) + " "
                                                             "ENABLE;";
    // Exec Query
    if(!lQuery.Execute(lQueryStr))
    {
        QString lMsg = "QUERY=" + lQueryStr + " ERROR=" + lQuery.lastError().text();
        GSLOG(SYSLOG_SEV_ERROR, lMsg.toLatin1().constData());
        UpdateLogMessage(editLog_Background, lMsg.append("\n"), true);
        UpdateBGLogMessages("TRANSFER EVENT", "Enable event", "ERROR", lMsg);
        return;
    }

    UpdateBGLogMessages("TRANSFER EVENT", "Enable event", "DONE", QString());
    UpdateLogMessage(editLog_Background, "Background transfer event successfuly enabled\n", true);
}

void GexSqlHousekeepingDialog::UpdateStartStopBGEventButton()
{
    // Get status
    QString lStatus = EventStatus();

    buttonStartStopBGEvent->show();
    if (lStatus == "ERROR")
        return;

    // if no event: change to start
    if (lStatus == "NO EVENT")
        buttonStartStopBGEvent->setText("Start");
    // else: change to stop
    else
        buttonStartStopBGEvent->setText("Stop");

    if(lStatus.contains("DONE",Qt::CaseInsensitive))
        buttonStartStopBGEvent->hide();
}

void GexSqlHousekeepingDialog::UpdateBGLogMessages(const QString &action, const QString &desc,
                                                   const QString &status, const QString &message)
{
    GexDbPlugin_Query lQuery(m_TdrDbPlugin,
                             QSqlDatabase::database(
                                 m_TdrDbPlugin->m_pclDatabaseConnector->m_strConnectionName));
    QString lQueryStr;
    int lCurrentUpdateId = 0;
    lQueryStr = "CALL background_update_id(@CurrentUpdateId)";
    if(lQuery.Execute(lQueryStr))
    {
        lQueryStr = "SELECT @CurrentUpdateId";
        if(lQuery.Execute(lQueryStr))
        {
            // A background process is running
            lQuery.First();
            lCurrentUpdateId = lQuery.value("@CurrentUpdateId").toInt();
        }
    }
    lQueryStr = "CALL  background_transfer_log_message(" + QString::number(lCurrentUpdateId) + ",'" + action + "', '" + desc +
            "', '" + status + "', '" + message + "');";

    // Exec Query
    if(!lQuery.Execute(lQueryStr))
    {
        QString lMsg = "QUERY=" + lQueryStr + " ERROR=" + lQuery.lastError().text();
        GSLOG(SYSLOG_SEV_ERROR, lMsg.toLatin1().constData());
        UpdateLogMessage(editLog_Background, lMsg.append("\n"), true);
        return;
    }
}

void GexSqlHousekeepingDialog::ResetItemSelectionList(bool isDbUpToDate, unsigned int databaseBuild)
{
    // Update item
    listSelection->item(UPDATE_ITEM_INDEX)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    listSelection->item(INCREMENTAL_ITEM_INDEX)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

    listSelection->item(BACKGROUND_TRANSFER_ITEM_INDEX)->setFlags(Qt::NoItemFlags);
    // gcore-755 feedback: the background update tab should be always enabled if some background transfer in progress,
    // even if TDR not up-to-date. Typical use case is the background transfer is not completed and the customer
    // install a YM with a new TDR version.
    //if(isDbUpToDate)
    {
        QStringList lTables;
        m_TdrDbPlugin->m_pclDatabaseConnector->EnumTables(lTables);
        if(lTables.contains("background_transfer_history",Qt::CaseInsensitive))
        {
            GexDbPlugin_Query clQuery(m_TdrDbPlugin, QSqlDatabase::database(m_TdrDbPlugin->m_pclDatabaseConnector->m_strConnectionName));
            QString lQuery;
            lQuery = "SELECT update_id FROM background_transfer_history LIMIT 1";
            if(clQuery.Execute(lQuery) && clQuery.First())
                listSelection->item(BACKGROUND_TRANSFER_ITEM_INDEX)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        }
    }

    listSelection->item(METADATA_ITEM_INDEX)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    listSelection->item(GLOBAL_SETTINGS_ITEM_INDEX)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    listSelection->item(RELEASE_HISTORY_ITEM_INDEX)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    listSelection->item(PURGE_ITEM_INDEX)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    listSelection->item(CONSOLIDATION_ITEM_INDEX)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

    // Incremental update item
    if (isDbUpToDate && (m_pTdrDatabaseEntry->m_pExternalDatabase->IsYmProdTdr() || m_pTdrDatabaseEntry->m_pExternalDatabase->IsManualProdTdr()))
    {
        listSelection->item(INCREMENTAL_ITEM_INDEX)->setFlags(
                    Qt::ItemIsSelectable|
                    Qt::ItemIsEnabled);
    }
    else
        listSelection->item(INCREMENTAL_ITEM_INDEX)->setFlags(Qt::NoItemFlags);
    // MetaData item
    listSelection->item(UPDATE_ITEM_INDEX)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    // Global Settings item
    if (isDbUpToDate)
    {
        listSelection->item(GLOBAL_SETTINGS_ITEM_INDEX)->setFlags(
                    Qt::ItemIsSelectable|
                    Qt::ItemIsEnabled);
    }
    else
        listSelection->item(GLOBAL_SETTINGS_ITEM_INDEX)->setFlags(Qt::NoItemFlags);
    // Release History item
    listSelection->item(RELEASE_HISTORY_ITEM_INDEX)->setFlags(
                Qt::ItemIsSelectable|
                Qt::ItemIsEnabled);
    // Purge tool item
    listSelection->item(PURGE_ITEM_INDEX)->setFlags(
                Qt::ItemIsSelectable|
                Qt::ItemIsEnabled);
    // Consolidation item
    if (m_pTdrDatabaseEntry->m_pExternalDatabase->IsYmProdTdr() && (databaseBuild > 15))
    {
        listSelection->item(CONSOLIDATION_ITEM_INDEX)->setFlags(
                    Qt::ItemIsSelectable|
                    Qt::ItemIsEnabled);
    }
    else
        listSelection->item(CONSOLIDATION_ITEM_INDEX)->setFlags(Qt::NoItemFlags);
}

void GexSqlHousekeepingDialog::onAccept(void)
{
    CancelChangeRequested();

    accept();
}

bool GexSqlHousekeepingDialog::CancelChangeRequested()
{
    if(!buttonMetaData_Cancel->isEnabled()
            && !buttonOptionSettings_Cancel->isEnabled())
        return true;

    QMessageBox msgBox(this);

    if (pGexSkin)
        pGexSkin->applyPalette(&msgBox);

    QString strMessage = "Some settings have been changed.\n Please commit these changes or cancel.";
    strMessage += "\n Do you want to commit this changes?";

    msgBox.setWindowTitle("Updating Settings");
    msgBox.setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));
    msgBox.setText(strMessage);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);

    msgBox.setButtonText( QMessageBox::Yes, "Commit" );
    msgBox.setButtonText( QMessageBox::Cancel, "&Cancel" );

    // If not, do nothing
    if(msgBox.exec() == QMessageBox::Cancel)
    {
        if(buttonMetaData_Cancel->isEnabled())
            OnButtonMetaData_Cancel();
        else if(buttonOptionSettings_Cancel->isEnabled())
            OnButtonOptionSettings_Cancel();
        return false;
    }

    if(buttonMetaData_Cancel->isEnabled())
        OnButtonMetaData_Apply();
    else if(buttonOptionSettings_Cancel->isEnabled())
        OnButtonOptionSettings_Apply();

    return true;
}


BackgroundStatusDelegate::BackgroundStatusDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

void BackgroundStatusDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int row = index.row();
    const QAbstractItemModel *model = index.model();

    QString lStatus;

    for (int i = 0; i < index.model()->columnCount(); i++)
    {
        if (qVariantValue<QString>(model->headerData(i,Qt::Horizontal)) == "status")
        {
            lStatus = qVariantValue<QString>(model->index(row,i).data());
        }
    }
    QColor lColor = QColor(Qt::white);
    if(lStatus == "DONE")
        lColor = QColor(Qt::green);
    else if(lStatus == "IN PROGRESS")
        lColor = QColor(Qt::yellow);
    else if(lStatus == "FAIL")
        lColor = QColor(Qt::red);

    painter->fillRect(option.rect, QBrush(lColor));
    drawDisplay(painter, option, option.rect, lStatus);
}


BackgroundProgressDelegate::BackgroundProgressDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

void BackgroundProgressDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int row = index.row();
    const QAbstractItemModel *model = index.model();


    qint64 lValue = 0;
    qint64 lMax = 0;
    qint64 lMin = 0;
    qint64 lCurrent = 0;

    for (int i = 0; i < index.model()->columnCount(); i++)
    {
        lValue = qVariantValue<qint64>(model->index(row,i).data().toLongLong());
        if (qVariantValue<QString>(model->headerData(i,Qt::Horizontal)) == "min_index")
            lMin = lValue;
        else if (qVariantValue<QString>(model->headerData(i,Qt::Horizontal)) == "max_index")
            lMax = lValue;
        else if (qVariantValue<QString>(model->headerData(i,Qt::Horizontal)) == "transfer_progress")
            lCurrent = lValue;
    }
    if( lCurrent == 0 )
        lCurrent = lMin;
    if(lMax == lMin)
        lMax++;

    int lPercent = (static_cast<double>(lMax - lCurrent) / (lMax - lMin)) * 100;
    drawDisplay(painter, option, option.rect, QString::number(lPercent)+"%");
}
