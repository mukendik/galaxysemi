#undef QT_MOC_COMPAT
#include <QScriptValue>
#include <QProgressBar>
#include <QShortcut>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QUndoView>
#include <QUndoStack>
#include <QScrollBar>
#include <QWebFrame>

#include <gqtl_sqlbrowser.h>
#include <gqtl_sysutils.h>

#include "message.h"
#include "drillDataMining3D.h"
#include "drill_table.h"
#include "drill_chart.h"
#include "gex_pixmap_extern.h"
#include "browser_dialog.h"
#include "engine.h"
#include "report_options.h"
#include "libgexoc.h"
//#include <gqtl_log.h>
#include "options_center_widget.h"
#include "pickuser_dialog.h"
#include "pat_info.h"
#include "script_wizard.h"
#include "settings_dialog.h"
#include "gex_version.h"
#include "gex_shared.h"
#include "gex_constants.h"
#include "db_engine.h"
#include "browser.h"
#include "db_onequery_wizard.h"
#include "onefile_wizard.h"
#include "comparefiles_wizard.h"
#include "mixfiles_wizard.h"
#include "mergefiles_wizard.h"
#include "compare_query_wizard.h"
#include "admin_gui.h"	// ExaminatorDB
#include "reports_center_widget.h"
#include "temporary_files_manager.h"
#include "tasks_manager_gui.h"
#include "tasks_manager_engine.h"
#include "tb_toolbox.h"
#include "gex_web_browser.h"
#include "gex_group_of_files.h"
#include "gex_file_in_group.h"
#include "admin_engine.h"
#include "gex_options_handler.h"
#include "product_info.h"
#include "db_keys_editor.h"
#include "gtm_mainwidget.h"
#include "regexp_validator.h"
#include "gex_undo_command.h"
#include "gex_options_map.h"
#include "pat_recipe_editor.h"
#include "gex_report.h"
#include "user_input_filter.h"
#include "ofr_controller.h"
#include <gqtl_log.h>
#include "gex_web.h"
#include "QDetachableTabWindowContainer.h"

#define XSTR(s) STR(s)
//#define STR(s) s

extern CReportOptions       ReportOptions;			// Holds options (report_build.h)
extern GexScriptEngine*     pGexScriptEngine;		// global Gex QScriptEngine
extern QProgressBar*        GexProgressBar;	// Handle to progress bar in status bar
extern QLabel*              GexScriptStatusLabel;	// Handle to script status text in status bar
extern CGexReport *         gexReport;				// Handle to report class

void GexMainwindow::OpenFileInEditor(const QString &lFileName, GexDocType eDocType)
{
    if (ReportOptions.GetOption("preferences","use_os_default").toBool() && QDesktopServices::openUrl(QUrl(QString("file:///%1").arg(lFileName)))) //Case 4619 - Use OS's default program to open file
      return;

    QString	lReportViewer;

    if (eDocType == eDocPdf || lFileName.endsWith(".pdf",Qt::CaseInsensitive) == true)
    {
#ifdef __linux__
        // Check if environment variable is set
        char *ptEnv = getenv("GEX_PDF_VIEWER");
        if(ptEnv)
            lReportViewer = ptEnv;
        else
            // if no system variable, use the one in ReportOptions
            lReportViewer = ReportOptions.GetOption("preferences","pdf_editor").toString(); //ReportOptions.m_PrefMap["pdf_editor"];
#else
        lReportViewer = ReportOptions.GetOption("preferences","pdf_editor").toString(); //ReportOptions.m_PrefMap["pdf_editor"];
#endif
    }
    else if (eDocType == eDocSpreadSheet || lFileName.endsWith(".csv",Qt::CaseInsensitive) == true)
        lReportViewer = ReportOptions.GetOption("preferences","ssheet_editor").toString();
    else if (eDocType == eDocText || lFileName.endsWith(".xml", Qt::CaseInsensitive) == true)
        lReportViewer = ReportOptions.GetOption("preferences", "text_editor").toString();
    else
    {
#if defined __unix__ || __MACH__
        lReportViewer = ReportOptions.GetOption("preferences", "text_editor").toString();
#endif
    }
#if defined __unix__ || __MACH__
    QString	lCommandLine;

    lCommandLine = lReportViewer + " " + lFileName + "&";
    if (system(lCommandLine.toLatin1().constData()) == -1)
    {
      //FIXME: send error
    }
#else
    QString lString;
    lString = lFileName;
    lString = lString.replace("/","\\");	// Replace '/' with '\\'
    // If file includes a space, we need to bath it between " "
    if(lString.indexOf(' ') != -1)
    {
        lString = "\"" + lString;
        lString = lString + "\"";
    }
    // Launch Editor
    if (lReportViewer.isEmpty() == true)
        ShellExecuteA(NULL,
           "open",
           lString.toLatin1().constData(),
           NULL,
           NULL,
           SW_SHOWNORMAL);
    else
        ShellExecuteA(NULL,
               "open",
               lReportViewer.toLatin1().constData(),
               lString.toLatin1().constData(),
               NULL,
               SW_SHOWNORMAL);
#endif

}

QScriptValue toScriptValue(QScriptEngine *engine, const GS::Gex::Engine::ClientState &s)
{
   QScriptValue obj = engine->newObject();
   obj.setProperty("state", (int)s);
   //obj.setProperty("x", s.x);
   //obj.setProperty("y", s.y);
   return obj;
}

void fromScriptValue(const QScriptValue &obj, GS::Gex::Engine::ClientState &s)
{
    s=(GS::Gex::Engine::ClientState)obj.property("state").toInt32();
}

QScriptValue createClientState(QScriptContext *, QScriptEngine *engine)
 {
     GS::Gex::Engine::ClientState s=GS::Gex::Engine::eState_Init;
     return engine->toScriptValue(s);
 }

///////////////////////////////////////////////////////////
// Contructor: setup Widgets
///////////////////////////////////////////////////////////
GexMainwindow::GexMainwindow(QWidget* parent, Qt::WindowFlags fl)
    : QMainWindow(parent, fl),
      m_pJavaScriptCenter(NULL),
      m_pSqlBrowser(NULL),
      m_pLogsCenter(NULL),
      m_pReportsCenter(NULL),
      m_pOptionsCenter(NULL),
      mAddElementInReportBuilderMovie(0)
{
    GSLOG(SYSLOG_SEV_DEBUG, "new GexMainwindow: launching setupUi()...");
    setupUi(this);

    setObjectName("GSMainwindow");

    mOnClose = false;
    mWizardsHandler.SetGexMainwindow(this);
    // Customize the button toolbar
    // Set separator action
    actionSeparatorStop->setSeparator(true);
    actionSeparatorScripting->setSeparator(true);
    actionSeparator->setSeparator(true);

    // Hide Reports Center icon at start-up
    actionReportsCenter->setVisible(false);

    // Add action to the button toolbar
    toolBarButton->addAction(actionBack);
    toolBarButton->addAction(actionForward);
    toolBarButton->addAction(actionStop);
    toolBarButton->addAction(actionSeparatorStop);
    toolBarButton->addAction(actionScripting);
    toolBarButton->addAction(actionSeparatorScripting);
    toolBarButton->addAction(actionCopy);
    toolBarButton->addAction(actionSeparator);
    toolBarButton->addAction(actionDBAdmin);
    toolBarButton->addAction(actionReportsCenter);
    if (! GS::LPPlugin::ProductInfo::getInstance()->isMonitoring()
        && !GS::LPPlugin::ProductInfo::getInstance()->isExaminator())
    {
        toolBarButton->addAction(actionOFR);
    }

    // Custonmize adress toolbar
    TextLabelURL = new QLabel(this);
    TextLabelURL->setObjectName(QString::fromUtf8("TextLabelURL"));
    TextLabelURL->setMaximumSize(QSize(60, 32767));
    TextLabelURL->setWordWrap(false);
    GexURL = new QComboBox(this);
    GexURL->setObjectName(QString::fromUtf8("GexURL"));
    GexURL->setMinimumSize(QSize(0, 0));
    GexURL->setEditable(true);
    GexURL->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    GexUrlBrowse = new QPushButton(this);
    GexUrlBrowse->setObjectName(QString::fromUtf8("GexUrlBrowse"));
    GexUrlBrowse->setMaximumSize(QSize(85, 32767));

    toolBarAddress->addWidget(TextLabelURL);
    toolBarAddress->addWidget(GexURL);
    toolBarAddress->addWidget(GexUrlBrowse);

    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
    {
        toolBarButton->hide();
        toolBarAddress->hide();
    }

    TextLabelURL->setText(QCoreApplication::translate("GexMainWindow", "Address:", 0, QCoreApplication::UnicodeUTF8));

#ifndef QT_NO_TOOLTIP
    GexURL->setToolTip(QCoreApplication::translate("GexMainWindow", "HTML pages visited", 0, QCoreApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP

#ifndef QT_NO_TOOLTIP
    GexUrlBrowse->setToolTip(QCoreApplication::translate("GexMainWindow", "Open a HTML page (Browse your disk)", 0, QCoreApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP

    GexUrlBrowse->setText(QCoreApplication::translate("GexMainWindow", "Browse...", 0, QCoreApplication::UnicodeUTF8));
    // Pointer to modules that are only created for specific GEX running modes (DB or WEB)
    mGexWeb = NULL;

    m_pReportsCenter = NULL;
    m_pOptionsCenter = NULL;
    pWizardSchedulerGui = NULL;
    mMonitoringWidget=NULL;
    mDbKeysEditor = 0;
    mRegExpValidator = 0;
    mUnsavedOptions = false;
    mForwardBackAction = false;

    QString strErrorMsg = "";
    if (!loadAdditionalModules(&strErrorMsg))
        GSLOG(SYSLOG_SEV_WARNING, QString("error while loading additional modules: '%1'")
            .arg(strErrorMsg).toLatin1().constData() );

    /*
    //  DEPRECATED
    if(!LoadModuleLogLevels(&strErrorMsg))
        GSLOG(SYSLOG_SEV_WARNING, QString("error while loading module loglevels : '%1'")
               .arg(strErrorMsg).toLatin1().data());
    */

    // Create basic classes & assistants
    pWizardScripting		= new GexScripting(centralWidget());
    pWizardSettings			= new GexSettings(centralWidget());
    pWizardOneFile			= new GexOneFileWizardPage1(centralWidget());
    pWizardCmpFiles			= new GexCmpFileWizardPage1(centralWidget());
    pWizardAddFiles			= new GexAddFileWizardPage1(centralWidget());
    pWizardMixFiles			= new GexMixFileWizardPage1(centralWidget());
    pWizardOneQuery			= new GexOneQueryWizardPage1(centralWidget());		// Single Query
    pWizardCmpQueries		= new GexCmpQueryWizardPage1(centralWidget());		// Compare Queries
    pWizardAdminGui         = new AdminGui(centralWidget());			// Data base Admin

    // Widget for GTM (Galaxy Tester Monitor). Used for FT PAT,
    // instantiated in onStart().
    mGtmWidget = NULL;

#ifndef GEX_NO_JS_SOLARIS
    m_pReportsCenter		= new ReportsCenterWidget(this, centralWidget());
#endif

    //m_pTemporaryFilesManager= new TemporaryFilesManager();
    mTaskManagerDialog			= new GS::Gex::TasksManagerGui(this);

    connect(&GS::Gex::Engine::GetInstance(), SIGNAL(sExit(int)), this, SLOT(ExitGexApplication(int)));

    // Show the dialog when stop task required
    // ShowDialog only if have pending tasks
    connect(&GS::Gex::Engine::GetInstance().GetTaskManager(), SIGNAL(sStopAllRunningTask()),
            mTaskManagerDialog, SLOT(OnShowDialog()));
    // Refresh tasks manager dialog when requested by engine
    connect(&GS::Gex::Engine::GetInstance().GetTaskManager(), SIGNAL(sTaskListUpdated(QStringList)),
            mTaskManagerDialog, SLOT(OnRefreshTaskList(QStringList)));
    // Cancel stop tasks when user request it
    connect(mTaskManagerDialog, SIGNAL(sCancelled()),
            &GS::Gex::Engine::GetInstance().GetTaskManager(), SLOT(OnCancelRequested()));

    // Keep track of Module menu selected. Default is GEX-Instant Report
    iGexAssistantSelected = GEX_MODULE_INSTANT_REPORT;

    // Default is 'hide'...until we call 'OnStart' initialization routine!
    hide();

    // Get current time
    dtSessionStarted = QDateTime::currentDateTime ();

    // Set application icon
    setWindowIcon(*pixGexApplication);

    // At GEX startup, default is to create ALL report pages
    iHtmlSectionsToSkip = 0;
    m_bDatasetChanged	= true;

    // Setup Progress bar
    GexProgressBar = new QProgressBar(statusBar());
    GexProgressBar->setMaximum(100);
    GexProgressBar->setTextVisible(false);
    GexProgressBar->setValue(0);
    // Script status string to be seen in the status bar
    GexScriptStatusLabel = new QLabel(statusBar());

    // Add the progress bar into the status bar
    statusBar()->addPermanentWidget(GexProgressBar);
    statusBar()->addPermanentWidget(GexScriptStatusLabel);

    // Set main window settings: title, color, etc...
    QPalette palette;
    palette.setColor(this->backgroundRole(), QColor(255, 255, 255));
    this->setPalette(palette);
    //setBackgroundColor(QColor(255, 255, 255));

    pageHeader->setContextMenuPolicy(Qt::NoContextMenu);
    pageHeader->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    pageHeader->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    pageHeader->setMinimumSize(QSize(0, 50));
    pageHeader->setMaximumSize(QSize(16777215, 50));
    pageHeader->setContentsMargins(0, 0, 0, 0);
    pageHeader->setFrameStyle( QFrame::NoFrame );
    pageHeader->verticalScrollBar()->setEnabled(false);

    pageFooter->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    pageFooter->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    pageFooter->setFrameStyle( QFrame::NoFrame );
    pageFooter->setContextMenuPolicy(Qt::NoContextMenu);
    pageFooter->verticalScrollBar()->setEnabled(false);

    // Create a browser for the Report pages !
    m_pWebViewReport	= new GexWebBrowser(pageHtmlBrowser);
    pageHtmlBrowser->layout()->setSizeConstraint(QLayout::SetMinAndMaxSize);
    pageHtmlBrowser->layout()->addWidget(m_pWebViewReport->widget());


    if (pGexScriptEngine)
    {
        QScriptValue GWBobject = pGexScriptEngine->newQObject((QObject*)m_pWebViewReport); // QScriptEngine::AutoCreateDynamicProperties ?
        if (!GWBobject.isNull())
//            GSLOG(SYSLOG_SEV_ERROR, QString("Cant create ScriptEngine QObject for GexWebBrowser").toLatin1().constData());
//        else
            pGexScriptEngine->globalObject().setProperty("GSWebBrowser", GWBobject);
    }

    // Load Navigation HTML tabs
    LoadNavigationTabs(GS::LPPlugin::LicenseProvider::eExaminator,true,true);

    // Prepare ScrollView that will receive dialog boxes
    pScrollArea = new QScrollArea(pageQTView);

    pageQTView->layout()->addWidget(pScrollArea);
    pScrollArea->setLayout(new QGridLayout(pScrollArea));
    pScrollArea->layout()->setSizeConstraint(QLayout::SetMinAndMaxSize);
    pScrollArea->layout()->setContentsMargins(0, 0, 0, 0);

    pScrollArea->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);		///		TO REVIEW (case 3353)
    pScrollArea->hide();	// will only appear when user have to enter data.
    pScrollArea->layout()->addWidget(pWizardSettings);
    pScrollArea->layout()->addWidget(pWizardOneFile);
    pScrollArea->layout()->addWidget(pWizardCmpFiles);
    pScrollArea->layout()->addWidget(pWizardAddFiles);
    pScrollArea->layout()->addWidget(pWizardMixFiles);
    pScrollArea->layout()->addWidget(pWizardOneQuery);			 // Single Query
    pScrollArea->layout()->addWidget(pWizardCmpQueries);		 // Compare Queries
    pScrollArea->layout()->addWidget(pWizardAdminGui);		 // Data base Admin

    if (m_pLogsCenter)
    {
        m_pLogsCenter->setParent(NULL);
        QDesktopWidget* lDesktop = QApplication::desktop();
        if (lDesktop)
        {
            m_pLogsCenter->resize(lDesktop->width(), lDesktop->height() * 9 / 10);
            m_pLogsCenter->move(0,0);
        }
        //GSLOG(SYSLOG_SEV_DEBUG, "adding LogsCenter widget to ScrollArea...");
        //m_pLogsCenter->setParent(pScrollArea);
        //pScrollArea->layout()->addWidget(m_pLogsCenter);
        new QShortcut(Qt::CTRL+Qt::Key_L, this, SLOT(ShowLogsCenter()));
    }

    new QShortcut(Qt::CTRL+Qt::Key_S, this, SLOT(ShowSqlBrowser()));

    /*
      // Now done in ShowSqlBrowser()
    if (!m_pSqlBrowser)
    {
        GSLOG(SYSLOG_SEV_DEBUG, "Loading SqlBrowser ...");
        QString o;
        SQLBROWSER_GET_INSTANCE(m_pSqlBrowser, NULL, o);
        GSLOG(SYSLOG_SEV_NOTICE, QString("SQLBROWSER_GET_INSTANCE : '%1' : %2").arg(
                 m_pSqlBrowser?m_pSqlBrowser->objectName().toLatin1().data():"NULL", o.toLatin1().data());
        if (m_pSqlBrowser)
        {
            //m_pSqlBrowser->Refresh(); // will refresh db list
            //m_pSqlBrowser->isMinimized()
            //m_pSqlBrowser->resize(m_pSqlBrowser->maximumSize()); // resize too much
            //pScrollArea->layout()->addWidget( m_pSqlBrowser );
            QObject::connect(this, SIGNAL(RefreshDB()), m_pSqlBrowser, SLOT(Refresh()) );
            //RefreshDB(); // ?
        }
        else
            GSLOG(SYSLOG_SEV_WARNING, "Cant load SqlBrowser library");
    }
    */

    new QShortcut(Qt::CTRL+Qt::Key_J, this, SLOT(ShowJavaScriptCenter()));

    if (m_pReportsCenter)
        pScrollArea->layout()->addWidget(m_pReportsCenter);
    if (m_pOptionsCenter)
        pScrollArea->layout()->addWidget(m_pOptionsCenter);

    pScrollArea->layout()->addWidget(pWizardScripting);

    // Force 'Single query' dialog as a child of Examinator space, not a pop-up.
    pWizardOneQuery->PopupSkin(GexOneQueryWizardPage1::eCreate,false);

    // Ensure optionnal Dialog boxes have pointers reset at startup.
    pWizardWhatIf   = NULL;	// Interactive Drill: What-If
    pGexTbTools     = NULL;	// Examinator-ToolBox.
#ifdef GCORE15334
    mWizardWSPAT    = NULL;	// PAT WS Wizard: processes a file for Outlier removal.
#endif
    mWizardFTPAT    = NULL; // PAT FT Wizard: processes a file for Outlier removal.
    mPATReportFTGui = NULL; // PAT FT report gui
    mQDetachableTabWindowContainer = 0;
    mOFRBuilder = GS::Gex::OFR_Controller::GetInstance();

    // Hides ALL Dialog boxes !
    ShowWizardDialog(GEX_BROWSER);

    // Configure connection with toolbars

    connect(actionStop,			SIGNAL(triggered()), this,				SLOT(OnStop()));
    connect(actionScripting,	SIGNAL(triggered()), this,				SLOT(ScriptingLink()));
    connect(actionEmail,		SIGNAL(triggered()), this,				SLOT(OnEmail()));
    connect(actionCopy,         SIGNAL(triggered()), this,              SLOT(OnSnapshot()));
    connect(actionDBAdmin,      SIGNAL(triggered()), this,              SLOT(Wizard_AdminGui()));
    if (! GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        connect(actionOFR, SIGNAL(triggered()), mOFRBuilder, SLOT(OFR_Admin()));
    }


    mAddElementInReportBuilderMovie =
        new QMovie(":/gex/icons/addInReportBuiler.mng");
    mAddElementInReportBuilderMovie->setCacheMode(QMovie::CacheAll);

    connect(mAddElementInReportBuilderMovie, SIGNAL(frameChanged(int)), this, SLOT(UpdateActionIcon(int)));

    connect(actionReportsCenter,SIGNAL(triggered()), this,              SLOT(ShowReportsCenter()));
    connect(GexURL,				SIGNAL(activated(QString)), this,		SLOT(ViewNewUrl()));
    connect(GexUrlBrowse,		SIGNAL(clicked()), this,				SLOT(OnBrowseUrl()));
    connect(actionForward,		SIGNAL(triggered()), m_pWebViewReport,	SLOT(forward()));
    connect(actionBack,			SIGNAL(triggered()), m_pWebViewReport,	SLOT(back()));

    // Signals to enable/disable the 'back' 'forward' browser icons
    connect( m_pWebViewReport,	SIGNAL(sBackAvailable(bool)), actionBack, SLOT(setEnabled(bool)));
    connect( m_pWebViewReport,	SIGNAL(sForwardAvailable(bool)), actionForward, SLOT(setEnabled(bool)));
    connect( m_pWebViewReport,	SIGNAL(sUrlChanged(const QUrl&)), this, SLOT(AddNewUrl(const QUrl&)));
#ifdef GEX_NO_WEBKIT
    connect( m_pWebViewReport,	SIGNAL(sLinkHovered(const QString&)),
             this,				SLOT(CommentGexHyperLink(const QString&)));
#else
    connect( m_pWebViewReport,	SIGNAL(sLinkHovered(const QString&, const QString&, const QString&)),
             this,				SLOT(OnGexLinkHovered(const QString&, const QString&, const QString&)));
#endif
    connect( m_pWebViewReport,	SIGNAL(sActionTriggered(const GexWebBrowserAction&)),
             this,				SLOT(onBrowserAction(const GexWebBrowserAction&)));

    QObject::connect
      (
        pageFooter, SIGNAL( anchorClicked( const QUrl & ) ),
        this, SLOT( OnFooterLinks(const QUrl &) )
      );

    QObject::connect
      (
        pageFooter, SIGNAL( highlighted( QString ) ),
        this, SLOT( OnHighlightFooterLinks( QString ) )
      );

    QObject::connect
      (
        pageHeader, SIGNAL( highlighted( QString ) ),
        this, SLOT( OnGexLinkHovered( const QString & ) )
      );

    QObject::connect
      (
        pageHeader, SIGNAL( anchorClicked( const QUrl & ) ),
        m_pWebViewReport, SLOT(linkClicked(const QUrl & ))
      );

    // Signal when user clicks 'Settings' in any of the wizard dialogs
    connect( (QObject *)pWizardOneFile->buttonSettings, SIGNAL( clicked() ),
                 this, SLOT( ViewSettings(void) ) );
    connect( (QObject *)pWizardCmpFiles->buttonSettings, SIGNAL( clicked() ),
         this, SLOT( ViewSettings(void) ) );
    connect( (QObject *)pWizardAddFiles->buttonSettings, SIGNAL( clicked() ),
         this, SLOT( ViewSettings(void) ) );
    connect( (QObject *)pWizardMixFiles->buttonSettings, SIGNAL( clicked() ),
         this, SLOT( ViewSettings(void) ) );

    connect( (QObject *)pWizardCmpQueries->buttonSettings, SIGNAL( clicked() ),
         this, SLOT( ViewSettings(void) ) );

    // Signals to notice a change in option file
    connect(pWizardScripting, SIGNAL(OptionChanged()),
            this, SLOT(OnOptionChanged()));

    // Accelerators
    // 'Ctrl+F' key
    new QShortcut(Qt::CTRL + Qt::Key_F, this, SLOT(OnFind()));

    // Build report buttons
    connect( (QObject *)pWizardSettings->buttonBuildReport, SIGNAL( clicked() ),
         this, SLOT( BuildReport(void) ) );

    // Initially, no navigation button is enabled.
    actionForward->setEnabled(false);
    actionBack->setEnabled(false);

    // Initial size & position at startup: trick=we size it
    // one pixel less than once displayed (to ensure we have 2 resize events)
    // because it is only at 2nd repaint that all client dimensions (toolbar) are known!
    ResizeGexApplication(0);

    // Support for Drag&Drop
    setAcceptDrops(true);

    m_poUndoStack = new QUndoStack(this);
    m_poUndoStack->setUndoLimit(50);
    m_poUndoView = 0;
    //new QUndoView(NULL);
    //m_poUndoView->setStack(m_poUndoStack);
    //m_poUndoView->hide();

    connect(&GS::Gex::Engine::GetInstance(), SIGNAL(sStarting()),
            this, SLOT(OnEngineStarting()));
    connect(&GS::Gex::Engine::GetInstance(), SIGNAL(sReady()),
            this, SLOT(OnEngineReady()));
    connect(&GS::Gex::Engine::GetInstance(), SIGNAL(sLicenseGranted()),
            this, SLOT(OnLicenseGranted()));

    mActionLicRelease = 0;
    mCanCreateNewDetachableWindow = true;
    GSLOG(SYSLOG_SEV_NOTICE, " ctor end.");

}

void GexMainwindow::UpdateActionIcon(int)
{
    if (!GS::LPPlugin::ProductInfo::getInstance()->isExaminator())
    {
        actionOFR->setIcon(QIcon(mAddElementInReportBuilderMovie->currentPixmap()) );
    }

}

GexMainwindow::~GexMainwindow()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Main window destructor...");

    QMap<QString, QVariant> lCounts;
    CGexSystemUtils::GetQObjectsCount(*this, lCounts, true);
    foreach(const QString &k, lCounts.keys())
        GSLOG(SYSLOG_SEV_DEBUG, (k+":"+QString::number(lCounts[k].toInt())).toLatin1().constData() );

    // Makes all necessary savings before closing
    OnClose();

    // Delete classes allocated in Mainwindow constructor
    // Delete these objects before deleting the database plugins
    if(pWizardScripting)
    {
        delete pWizardScripting;
        pWizardScripting=NULL;
    }
    if(pWizardSettings)
    {
        delete pWizardSettings;
        pWizardSettings=NULL;
    }
    if(pWizardOneFile)
    {
        delete pWizardOneFile;
        pWizardOneFile=NULL;
    }
    if(pWizardCmpFiles)
    {
        delete pWizardCmpFiles;
        pWizardCmpFiles=NULL;
    }
    if(pWizardAddFiles)
    {
        delete pWizardAddFiles;
        pWizardAddFiles=NULL;
    }
    if(pWizardMixFiles)
    {
        delete pWizardMixFiles;
        pWizardMixFiles=NULL;
    }
    if(pWizardOneQuery)
    {
        delete pWizardOneQuery;
        pWizardOneQuery=NULL;
    }
    if(pWizardCmpQueries)
    {
        delete pWizardCmpQueries;
        pWizardCmpQueries=NULL;
    }
    if(pWizardAdminGui)
    {
        delete pWizardAdminGui;
        pWizardAdminGui=NULL;
    }
    if(mTaskManagerDialog)
    {
        delete mTaskManagerDialog;
        mTaskManagerDialog=NULL;
    }

    // Delete classes that may have been detached from the Examinator Widget.
    if(pGexTbTools != NULL)
    {
        delete pGexTbTools; pGexTbTools=0;
    }

    if (m_pReportsCenter)
    {
        // Since Qt5, closing app does not auto delete/close the RC widgets...
        m_pReportsCenter->DeleteAllParamsWidgets();
        GSLOG(SYSLOG_SEV_INFORMATIONAL, "deleting ReportsCenter...");
        delete m_pReportsCenter;
    }
    m_pReportsCenter=NULL;

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" still %1 instances of CReportsCenterParamsWidget.")
          .arg(CReportsCenterParamsWidget::GetNumberOfInstances()).toLatin1().constData());
    //GSLOG(SYSLOG_SEV_NOTICE, QString(" still %1 instances of CReportsCenterParamsDialog.").arg( ReportsCenterParamsDialog::GetNumberOfInstances());
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" still %1 instances of CReportsCenterDataset.")
          .arg(CReportsCenterDataset::GetNumberOfInstances()).toLatin1().constData() );
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" still %1 instances of CReportsCenterMultiField.")
          .arg(CReportsCenterMultiFields::GetNumberOfInstances()).toLatin1().constData() );
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" still %1 instances of CReportsCenterField.")
          .arg(CReportsCenterField::GetNumberOfInstances()).toLatin1().constData() );

#ifdef GCORE15334
    if(mGtmWidget)
    {
        delete mGtmWidget;
        mGtmWidget=NULL;
    }
#endif

    if (m_pLogsCenter)
    {
        //pScrollArea->layout()->removeWidget(m_pLogsCenter);
        delete m_pLogsCenter;
        m_pLogsCenter=0;
    }

    if (m_pJavaScriptCenter)
    {
        delete m_pJavaScriptCenter;
        m_pJavaScriptCenter=NULL;
    }

    if (m_pSqlBrowser)
    {
        m_pSqlBrowser->hide();
        delete m_pSqlBrowser;
        m_pSqlBrowser=0;
    }

    if (mDbKeysEditor)
    {
        delete mDbKeysEditor;
        mDbKeysEditor = 0;
    }

    if (mRegExpValidator)
    {
        delete mRegExpValidator;
        mRegExpValidator = 0;
    }

    if (mGexWeb)
    {
        delete mGexWeb;
        mGexWeb = NULL;
    }

    delete mQDetachableTabWindowContainer;

#ifdef GCORE15334
    GS::Gex::PATRecipeEditor::DestroyInstance();
#endif
    GSLOG(SYSLOG_SEV_DEBUG, "Main window deleted");
}

QString	GexMainwindow::LoadOptionsCenter()
{
#if 0
    // Do not load options center if running GTM
    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, "OptionsCenter not loaded when running GTM");
        return "ok";
    }
#endif

    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Looking at OptionsCenter dll/so...");

    QGuiApplication::setOverrideCursor( QCursor(Qt::BusyCursor) );

    QString strGetOptionsCenterRslt;
    OPTIONSCENTER_GET_INSTANCE(m_pOptionsCenter, centralWidget(), pGexScriptEngine, this, 5, strGetOptionsCenterRslt );
    if (!m_pOptionsCenter)
    {
        QGuiApplication::restoreOverrideCursor();
        return "error : can't find dll/so or cant GET_INSTANCE !";
    }

    // If YM, PM, GTM, hide 'Builf Report' button
    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM() || GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        m_pOptionsCenter->m_BuildReport->hide();

    if (!QObject::connect(m_pOptionsCenter, SIGNAL(signalMessage(QString)),
                     this, SLOT(UpdateLabelStatus(QString))))
        GSLOG(4, "Failed to connect OptionsCenter message signal");

    if(strGetOptionsCenterRslt.startsWith(QString("error"), Qt::CaseInsensitive))
    {
        return strGetOptionsCenterRslt + QString("\nerror : can't load OptionsCenter !");
    }

    if(!(RefreshOptionsCenter()))
    {
        GSLOG(SYSLOG_SEV_WARNING, "problem occured when refreshing options center");
        QGuiApplication::restoreOverrideCursor();
        return QString("error : can't refresh 'options center'");
        //return false;
    }

    bool b=false;
    b=connect(m_pOptionsCenter, SIGNAL(signalOptionChanged(QString,QString,QString)),
            this, SLOT(SlotOptionChanged(QString,QString,QString)));
    if (!b)
        GSLOG(SYSLOG_SEV_ERROR, "cant connect OptionsCenter signal");
    b=connect(m_pOptionsCenter->m_LoadProfile, SIGNAL(clicked()), this, SLOT(OnLoadUserOptions(void)));
    b=connect(m_pOptionsCenter->m_SaveProfile, SIGNAL(clicked()), this, SLOT(OnSaveUserOptions(void)));
    b=connect(m_pOptionsCenter->m_ResetButton, SIGNAL(clicked()), this, SLOT(OnDefaultOptions(void)));

    // Build report buttons
    b=connect(m_pOptionsCenter->m_BuildReport,		SIGNAL(clicked()),	this,		SLOT(BuildReport()));
    // "Save Options" button in Options page
    b=connect(m_pOptionsCenter->m_SaveButton,		SIGNAL(clicked()),	this,		SLOT(OnSaveEnvironment()));

    GSLOG(SYSLOG_SEV_DEBUG, " OptionsCenter configured.");

    QGuiApplication::restoreOverrideCursor();

    return "ok";
}

void GexMainwindow::SlotOptionChanged(QString section, QString f, QString new_value)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, (QString(" option changed to '%1' '%2' '%3'")
          .arg(section)
          .arg(f)
          .arg(new_value)).toLatin1().constData());
#if 0
    //TODO : add here the code of undo/redo here
    if(getUndoStack()){
        QString strField = f;
        QString strSection = section;
        QString strOld = "";
        QString strNew = new_value;
        if(m_optionsHandler.getOptionsMap().map().contains(strSection) && (m_optionsHandler.getOptionsMap().map()[strSection]).contains(strField)){
            strOld  = (m_optionsHandler.getOptionsMap().map()[strSection])[strField];
            OptionChangedCommand *poObj = new OptionChangedCommand(strSection, strField, QVariant(strOld), QVariant(strNew));
            getUndoStack()->push(poObj);
        }
    }
//	ReportOptions.SetOption(section, f, new_value);
#endif

    if(!f.compare("auto_close"))
        GS::Gex::UserInputFilter::geLastInstance()->resetValueTimerBeforeAutoClose(new_value);


    GS::Gex::OptionsHandler::ActiveNeedToRebuild(section.toStdString(), f.toStdString());

    GS::Gex::OptionsHandler optionsHandler(GS::Gex::Engine::GetInstance().GetOptionsHandler());

    OnOptionChanged();

    if (optionsHandler.SetOption(section, f, new_value))
        GS::Gex::Engine::GetInstance().SetOptionsMap(optionsHandler.GetOptionsMap());
}


void GexMainwindow::OnOptionChanged()
{
    mUnsavedOptions = true;
    UpdateCaption();
}


///////////////////////////////////////////////////////////
// Export SBL (Statistical Bin Limits) to file:
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_ExportSBL(void)
{
    // Enable/disable some features...
    if(GS::LPPlugin::ProductInfo::getInstance()->isOEM() ||
       !GS::LPPlugin::ProductInfo::getInstance()->isSYLSBLAllowed())
    {
        QString m=ReportOptions.GetOption("messages", "upgrade").toString();
        GS::Gex::Message::information("", m);
        return;
    }

    CGexGroupOfFiles *pGroup;	CGexFileInGroup *pFile;
    // Get pointer to first group
    pGroup = gexReport->getGroupsList().isEmpty()?NULL:gexReport->getGroupsList().first();
    if(pGroup == NULL)
        return;
    pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
    if(pFile == NULL)
        return;

    // Suggest output file.
    QString strOutputFile;
    if(*pFile->getMirDatas().szPartType)
    {
        strOutputFile = pFile->getMirDatas().szPartType;
        strOutputFile+= ".sbl";
    }

    // Select output file where SBL data to be saved.
    // convert QT3 to QT4
    strOutputFile = QFileDialog::getSaveFileName(this,
                                                 "Save Yield & Statistical Bin Limits to file...",
                                                 strOutputFile,
                                                 QString("SBL file(*.sbl)").toLatin1().constData());
//    strOutputFile = QFileDialog::getSaveFileName(
//         strOutputFile,
//        "SBL file(*.sbl)",
//        this, "save file dialog",
//        "Save Yield & Statistical Bin Limits to file..." );

    // If no file selected, quiet return
    if(strOutputFile.isEmpty())
        return;

    // Create SBL file
    QFile file(strOutputFile); // Write the SBL/YBL file
    if (file.open(QIODevice::WriteOnly | QIODevice::Text) == false)
        return;	// Failed.

    // Write YBL/SBL File
    QTextStream hFile(&file);	// Assign file handle to data stream

    // Fill file with Database details...
    hFile << "<galaxy_ybl_sbl>"  << endl;	// Start marker
    hFile << "Bin#,Mean (%),Low-SBL (%),High-SBL (%)"  << endl;

    hFile << "</galaxy_ybl_sbl>" << endl;	// End marker

    file.close();
}

bool GexMainwindow::RefreshOptionsCenter()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL,QString(" RefreshOptionsCenter : %1 sections...").arg(
             GS::Gex::Engine::GetInstance().GetOptionsHandler().GetOptionsMap().map().size()).toLatin1().constData());

    QMapIterator<QString, QMap<QString, QString> > it2(GS::Gex::Engine::GetInstance().GetOptionsHandler().GetOptionsMap().map());
    while (it2.hasNext())
    {
        it2.next();
        QMapIterator<QString, QString> it(it2.value());
        while (it.hasNext())
        {
            it.next();
            bool r=false;
            OPTIONSCENTER_SET_OPTION(r, it2.key(),
                                     it.key().toLatin1().data(),
                                     it.value().toLatin1().data());
            if (!r)
                GSLOG(SYSLOG_SEV_NOTICE, QString("OptionsCenter SetOption %1 %2 failed !")
                      .arg( it.key())
                        .arg(it.value()).toLatin1().constData());
        }
    }

    UpdateCaption();
    m_pOptionsCenter->mSourceLabel->setText(QDir::cleanPath(GS::Gex::Engine::GetInstance().GetStartupScript()));

    return true;
}



bool GexMainwindow::forceToComputeFromDataSample(FILE *hFile, int iProcessPart){
    QString strProcessParts = gexFileProcessPartsItems[iProcessPart];
    if( strProcessParts != "all" && strProcessParts != "no_samples" ){
        //Force stats and binning to be computed from samples.
        fprintf(hFile,"  //Force stats and binning to be computed from samples for this filter.\n");
        fprintf(hFile,"  gexOptions('binning','computation','samples');\n");
        fprintf(hFile,"  gexOptions('statistics','computation','samples_only');\n");
        return true;
    }
    return false;
}

void GexMainwindow::enableLicRelease()
{
    if(!mActionLicRelease)
    {
        mActionLicRelease = new QToolButton();
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/gex/icons/lm.png"), QSize(), QIcon::Normal, QIcon::Off);
        mActionLicRelease->setIcon(icon);
        mActionLicRelease->setText("Release license");
        mActionLicRelease->setToolTip("Release license");
        mActionLicRelease->setAutoRaise(true);
        statusBar()->addPermanentWidget(mActionLicRelease,0);
        statusBar()->show();
        connect(mActionLicRelease, SIGNAL(clicked()), &GS::Gex::Engine::GetInstance(), SLOT(licenseReleaseRequest()));
    }
}

