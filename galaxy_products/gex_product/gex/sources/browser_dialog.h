#ifndef GEX_DIAlOG_H
#define GEX_DIAlOG_H

class GexMainwindow;
class QUndoView;
class QUndoStack;
class QToolButton;

typedef void (GexMainwindow::*ptActions)(void);

#include "ui_browser_dialog.h"
#include "gex_constants.h"

#include <QTextBrowser>
#include <qtcpsocket.h>
#include <QLabel>
#include <QPushButton>
#include <qstatusbar.h>
#include <qtimer.h>
#include <qdatetime.h>
#include <QScrollArea>
#include <QComboBox>
#include <QScriptValue>
#include <QScriptEngine>
#include <wizards_handler.h>
#include "QDetachableTabWindowContainer.h"


class   QAction;

class   QScriptValue;
class   GexScripting;
class   GexDrillWhatIf;
class   GexWizardChart;
class   GexWizardTable;
#ifdef GCORE15334
class   GexTbPatDialog;
#endif
class   GexSettings;
class   GexWeb;
class   Gtm_MainWidget;
class	SchedulerGui;
class	GexVfeiService;
class	GexTbToolBox;
class	GexOneFileWizardPage1;
class	GexCmpFileWizardPage1;
class	GexAddFileWizardPage1;
class	GexMixFileWizardPage1;
class	GexOneQueryWizardPage1;
class	GexCmpQueryWizardPage1;
class	AdminGui;

//class	ReportsCenterWidget;
class	ActivationKeyDialog;
class	MonitoringWidget;
// 6953
// class	TemporaryFilesManager;
class	GexWebBrowser;
class	GexWebBrowserAction;
class	GexDbPlugin_Connector;
class   CPatDefinition;
class   COptionsPat;
class   CTest;
class  QDetachableTabWindowContainer;

struct  TestInfo;

namespace Gex
{
  class DrillDataMining3D;
}
namespace GS
{
    namespace Gex
    {
        class DbKeysEditor;
        class RegExpValidator;
        class TasksManagerGui;
        class CSLStatus;
        class AdminEngine;
        class PATWidgetFT;
        class PATReportFTGui;
        class PATRecipe;
        class OFR_Controller;
    }
    namespace QtLib
    {
        class SqlBrowser;
    }
    namespace SE
    {
        class StatsEngine;
    }
}

///////////////////////////////////////////////////////////
// Subclassing QTextBrowser so we can master mouse events.
///////////////////////////////////////////////////////////
class GexBrowser : public QTextBrowser
{
public:
    GexBrowser(QWidget * parent=0);

    int	HtmlToolBarAction;	// Holds the action to perform on HTML page: Zoom-in,Zoom-out

    // Overwrite virtual fcts from main class for Drag&Drop support.
    void dragEnterEvent(QDragEnterEvent *);
    void dragMoveEvent(QDragMoveEvent *);
    void dropEvent(QDropEvent *);
};

///////////////////////////////////////////////////////////
// GEX main window
///////////////////////////////////////////////////////////
class GexMainwindow : public QMainWindow, public Ui::GexMainWindow
{
    Q_OBJECT

public:
    GexMainwindow( QWidget* parent = 0, Qt::WindowFlags fl = Qt::Window );
    ~GexMainwindow();

    enum GexDocType
    {
     eDocSpreadSheet,
     eDocText,
     eDocPdf,
     eDocUnknown
    };

    /** \enum analyseMode
     *  \brief is an enumerate to contain the analyse mode and to be used to fill the appropriate data GUI.
     */
    enum analyseMode
    {
        InvalidAnalyse =-1,
        SingleFile = 0,
        MergeFiles,
        CompareFiles,
        CompareGroupOfFiles,
        SingleDataset,
        CompareDatasets,
        MergeDatasets
    };

    static void OpenFileInEditor(const QString &strFileName, GexDocType eDocType = eDocUnknown);

    GexScripting* pWizardScripting;		// Handle to Scripting dialog box

    // Timer for checking if script-thread is done...
    // 6953: Moved to Engine::GetScriptTimer()
    //QTimer	timerScriptDone;

    // int	iWizardType;							// Dataset Wizard type (type of analysis to conduct. eg: One file, or Compare file, etc...)
    int	iWizardPage;							// Wizard page shown (eg: InteractiveChart, IntreactiveTable, etc...)
    // Load a normal HTML page into Browser window.
    void OnBrowseUrl(QString strHtmlFile);
    Q_INVOKABLE void ShowReportPage(bool startup);					// Script successful+generated report: show it!
    Q_INVOKABLE void Wizard_OpenReportFile(const QString &strFileName);
    void Wizard_ExportWafermap(QString strLink);
    Q_INVOKABLE void LoadNavigationTabs(int iProduct,bool bLoadHeader=true,bool bLoadFooter=true);				// Load HTML Tabs (navigation HTML bar)
    Q_INVOKABLE void ResizeGexApplication(int iOffset);		// Resize GEX window to fit in screen.
    // Write GuardBanding Script section (can be called by GEX-Instant Report or GEX-Data Mining)
    //void WriteGuardBandingScriptSection(FILE *,bool);

    //  Reload Database list + refresh GUI (if requested)
    Q_INVOKABLE int	 ReloadDatabasesList(bool bRepaint);

    // REPORT link pressed
    void ReportLink(void);

    // Dialog boxes: Drill in What-If
    GexDrillWhatIf			*pWizardWhatIf;		// Wizard for drilling into 'What-If'


    Gex::WizardHandler              mWizardsHandler;

    bool                    IsWizardChartAvalaible      ()  { return !mWizardsHandler.ChartWizards().isEmpty();}
    ///
    /// \brief WizardChart
    /// \return the last Chart wizard created (because front insertion)
    ///
    GexWizardChart*         LastCreatedWizardChart      ()  { return mWizardsHandler.ChartWizards().isEmpty()?0:(*mWizardsHandler.ChartWizards().begin());  }
    GexWizardTable*         LastCreatedWizardTable      ()  { return mWizardsHandler.TableWizards().isEmpty()?0:(*mWizardsHandler.TableWizards().begin());  }
    Gex::DrillDataMining3D* LastCreatedWizardWafer3D    ()  { return mWizardsHandler.Wafer3DWizards().isEmpty()?0:(*mWizardsHandler.Wafer3DWizards().begin());  }

    void                    ReloadWizardHandler         (CTest* test = 0);

    // PAT-Man Process file Wizard
#ifdef GCORE15334
    GexTbPatDialog *        mWizardWSPAT;       // Wizard for Processing a file with PAT-Man configuration outlier rules.
#endif
    GS::Gex::PATWidgetFT *  mWizardFTPAT;       // Wizard for processing a file with FT Real Time Simulator

    // Dialog box to setup GEX settings
    GexSettings         *pWizardSettings;
    // Widget for GTM (Galaxy Tester Monitor). Used for FTP PAT
    Gtm_MainWidget *        mGtmWidget;

    // Manage the "Build Report!" button title
    QString	BuildReportButtonTitle(QString strTitle="", bool bBackupCurrentTitle=false, bool deleteDetachableWindows = true);

    // Delete all the interactive report in the detachable windows if any
    void ClearDetachableWindows();

    //List of HTML sections NOT to create
    // used when multi-pass involved such as for 'guard-banding'
    // Combination of : GEX_HTMLSECTION_ALL, GEX_HTMLSECTION_GLOBALINFO, ...
    int	iHtmlSectionsToSkip;

    // GEX Module selected: Instant-Report, Data-Mining, etc...
    int			iGexAssistantSelected;

    // ExaminatorMonitoring
    MonitoringWidget    *mMonitoringWidget;		// Wizard Page1 for Type7 action: Admin page

    // Dialog boxes: Database Admin
    AdminGui            *pWizardAdminGui;	// Monitoring: Database page.

    // Examinator-Monitoring
    SchedulerGui        *pWizardSchedulerGui;	// Monitoring: Tasks page

    // Examinator-ToolBox
    GexTbToolBox        *pGexTbTools;		// ToolBox: Convert files page.

    GS::Gex::TasksManagerGui	*mTaskManagerDialog;

    Q_INVOKABLE bool SetWizardType(int lWizardType);
    Q_INVOKABLE int  GetWizardType() const;

    // Public Wizards
    Q_INVOKABLE void Wizard_OneQuery_Page1(void);
    //! \brief Check STDF file contents to identify any discrepency
    // (can be used to see what affects PAT (eg: test number duplication, etc))
    Q_INVOKABLE void Wizard_OneFile_FileAudit(void);
    Q_INVOKABLE void Wizard_OneFile_Page1(void);
    Q_INVOKABLE void Wizard_CompareFile_Page1(void);
    Q_INVOKABLE void Wizard_ShiftAnalysis(void);
    Q_INVOKABLE void Wizard_MergeFile_Page1(void);
    Q_INVOKABLE void Wizard_SqlQuery(void);
    Q_INVOKABLE void Wizard_CharQuery(void);
    Q_INVOKABLE void Wizard_FT_PAT_FileAnalysis(void);
    //void Wizard_Options(void);
    Q_INVOKABLE void Wizard_Settings(void);
    Q_INVOKABLE void Wizard_Scripting(void);
    Q_INVOKABLE void Wizard_DrillWhatif(void);
    Q_INVOKABLE void Wizard_DrillChart(QString strLink, bool fromHTMLBrowser = true);
    //! \brief Assistant link to Table Drill!
    Q_INVOKABLE void Wizard_DrillTable(QString strLink, bool fromHTMBrowser = true);
    Q_INVOKABLE void Wizard_Drill3D(QString strLink,  bool fromHTMLBrowser = true);
    Q_INVOKABLE void Wizard_DrillChart();
    //! \brief Needed as table of addresses to function is only of type (void) for parameters...
    // WT : what does it mean ?
    Q_INVOKABLE void Wizard_DrillAll(void);
    Q_INVOKABLE void Wizard_DrillTable(void);
    Q_INVOKABLE void Wizard_Drill3D();
    Q_INVOKABLE void Wizard_FT_PAT(void);            // Clean file from outiers (PAT removal)

    Q_INVOKABLE void ShowHtmlBrowser(bool bBrowser=true);   // Shows/Hide the Examinator's HTML browser
    QScrollArea     *pScrollArea;               // any input dialog is stick into this scrollview!

    // Dialog boxes: Single File
    GexOneFileWizardPage1   *pWizardOneFile;    // Wizard Page1 for Type1 action: file processed

    // Dialog boxes: Compare files
    GexCmpFileWizardPage1   *pWizardCmpFiles;   // Wizard Page1 for Type2 action

    // Dialog boxes: Merge files
    GexAddFileWizardPage1   *pWizardAddFiles;   // Wizard Page1 for Type3 action

    // Dialog boxes: Compare+merge files
    GexMixFileWizardPage1   *pWizardMixFiles;   // Wizard Page1 for Type4 action

    // Dialog boxes: Analyze Dataset (single Query)
    GexOneQueryWizardPage1  *pWizardOneQuery;   // Wizard Page1 for Type5 action: Simple Query

    // Dialog boxes: Compare Datasets (Compare Queries)
    GexCmpQueryWizardPage1  *pWizardCmpQueries; // Wizard Page1 for Type6 action: Compare Queries

    // FT PAT reporting wizard
    GS::Gex::PATReportFTGui * mPATReportFTGui;


    QWidget*                        m_pJavaScriptCenter;
    class GS::QtLib::SqlBrowser*    m_pSqlBrowser;
    QWidget*                        m_pLogsCenter;
    class ReportsCenterWidget*      m_pReportsCenter;	//
    class OptionsCenterWidget*      m_pOptionsCenter;
    // try to load the OptionsCenter dll, then to retrieve the instance which will try to load gex.goxml,
    // and then to connect it to an action/icon
    Q_INVOKABLE QString	LoadOptionsCenter();

    // Class used to handle ExaminatorDB I/O  with clients
    GexWeb *                        mGexWeb;

    Q_INVOKABLE bool RefreshOptionsCenter();

    // Toolbox function made public
    Q_INVOKABLE void Wizard_GexTb_EditPAT_Limits(void); // Edit STATIC PAT Limits file.
    Q_INVOKABLE void Wizard_GexTb_PAT(void);            // Clean file from outiers (PAT removal)
    Q_INVOKABLE void Wizard_GexTb_Edit(void);           // Edit data file into Spreadsheet form.

    /// \brief This function computes MV pat group
    /// \param lTests : list of the numbers of enabled tests
    ///        lPATRecipe : PAT recipe
    /// \return true if the calculation is correct
    bool ComputeMVPATGroups(const QList<TestInfo> &lTests,
                            GS::Gex::PATRecipe &lPATRecipe,
                            GS::SE::StatsEngine *statsEngine);

    /// \brief fill  the list of univariate rules in the PATRecipe
    /// \param lTests : list of the numbers of enabled tests
    bool FillUniVariateRules(const QList<TestInfo> &lTest, GS::Gex::PATRecipe& patRecipe);

    /// \brief Create a PAT definition object from a CTest
    /// \param lOptions : PAT Recipe options
    /// \param lTest : list of the numbers of enabled tests
    ///
    /// This method has to be moved in another class
    CPatDefinition * CreatePATDefinition(const COptionsPat& lOptions, CTest& lTest);

    // Command line arguments
    QString strExaminatorWebUserHome;       // User $Home point if running ExaminatorWEB
    QString m_selectedURL;                  // Action-URL selected.
    bool    m_bDatasetChanged;              // 'true' if dataset has changed before previous analyze

    // Overwrite virtual fcts from main class for Drag&Drop support.
    void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);

    // create a csl script using defines (GEX_SCRIPT_ASSISTANT,...)
    // returns pointer to the new FILE that can be NULL if error
    FILE *CreateGexScript(int iScriptName);
    // Close the csl script
    void CloseGexScript(FILE *hScript, int iSections);

    // Write the csl 'BuildReport' section : just gexOptions('report','build','0'); & gexBuildReport(LoadPage, ...);
    // todo : move me to csl as soon as iHtmlSectionsToSkip has been moved
    void WriteBuildReportSection(
          FILE *hFile, QString strLoadPage="home", bool bCreateReport=true);

    // Bookmark clicked. Check if it is a GEX action...
    void onBrowserActionBookmark(const GexWebBrowserAction& webBrowserAction);
    void onBrowserActionCsv(const QString& strCsvFile);

    // Parse hyperlink to know the action to trigger...
    void onBrowserActionLink(const GexWebBrowserAction&);   //was : const QString& strLink, int nIndex);
    void onBrowserActionOpenUrl(const QString& strUrl);

    Q_INVOKABLE const QString&  navigationSkinDir() const			{ return strNavigationSkinDir; }

    static void     applyPalette(QWidget * pWidget);
    static int      LookupLinkName(const QString& strLink);

    // Load the additional module QMap from localconfig.conf file with associated value
    bool            loadAdditionalModules(QString* pStrErrorMsg = NULL);
    // Save the additional module QMap to localconfig.conf file
    bool            saveAdditionalModules(QString* pStrErrorMsg = NULL);
    // Return the evaluated additional module value
    // Returns an invlaid QScriptValue if module not found
    QScriptValue    additionalModule(const QString& strModuleName);

    // PYC : 08/04/02011
    // Load the module LogLevel QMap from localconfig.conf file with associated value
    // DEPRECATED, use env var since 6.6
    // Load the additional module QScript engine from localconfig.conf file
    //bool	LoadModuleLogLevels(QString* pStrErrorMsg = NULL);
    // Save the module LogLevel QMap to localconfig.conf file
    //bool	SaveModuleLogLevels(QString* pStrErrorMsg = NULL);
    // Return the evaluated module LogLevel value
    // Returns -1 if module loglevel not found or invalid (must be a javascript expression that evaluation resulting has integer between 0 and 7)
    //int	ModuleLogLevel(const QString& strModuleName);

    analyseMode GetAnalyseMode() const;

    void SetAnalyseMode(const analyseMode &analyseMode);

protected:
    void closeEvent(QCloseEvent *);
//    void keyReleaseEvent(QKeyEvent * event);
  //  void keyPressEvent(QKeyEvent * event);
   // bool event(QEvent *event);

private:

    static QPalette     m_palette;                      // Palette used for Gex windows

    QString             strPrintReportPath;             // HTML report path generated/loaded in browser.
    QString             strNavigationSkinDir;           // Set path to relevant skin HTML pages
    QDateTime           dtSessionStarted;               // Date time of GEX launch.
    QString             m_strExportWafermapPath;        // export Path for wafermap
    bool                mForwardBackAction;                 // action sparked by the forward arrow
    static ptActions    pActionFunctions[];             // Pointer to Wizard functions associated with Hyperlinks
    int                 iScreenSize;                    // Computer screen size (SMALL/MEDIUM/LARGE)
    // PYC, 30/05/2011
    int                 mWizardType;                    // Dataset Wizard type (type of analysis to conduct. eg: One file, or Compare file, etc...)

    GS::Gex::DbKeysEditor *mDbKeysEditor;               // Database key editor
    GS::Gex::RegExpValidator *mRegExpValidator;         // Database key editor

    bool                mUnsavedOptions;           // true if option change not saved

    // Web browser
    GexWebBrowser *     m_pWebViewReport;           // The HTML page where the report is seen.

    GS::Gex::OFR_Controller* mOFRBuilder;

    // control for URL toolbar
    QLabel *            TextLabelURL;
    QComboBox *         GexURL;
    QPushButton *       GexUrlBrowse;

    QDetachableTabWindowContainer* mQDetachableTabWindowContainer;
    bool mCanCreateNewDetachableWindow;

    // utility functions
    static QDomElement GetElementByIdIn( const QDomDocument &doc, const QString &id );

    // Functions
    /**
     * @brief GCORE-7369 Reload the header top page taking into account the
     * product id if OEM version is used
     * @param url url of the header top page
     */
    void ReloadHeaderPage( const QUrl &url );

    void closeGex();
    //QString		getClientApplicationId(); // moved to Engine
    QStringList extractSTDFZipFiles(QString strZipAbsoluteFilePath);
    void onProcessDataFile(QStringList strArgumentDataFile, bool bFromCommandLine);
    void SaveReportScript(void);
    void exportFTRCorrelationReport();
    void UpdateTopNavigationBar(int iIndex);
    void ResizeWizardDialog(int width,int height);
    void DoNothing(void);
    void HomeLink(void);
    void DataLink(void);
    void HelpLink(void);
    void Wizard_Assistant(void);
    void Wizard_OpenReport(void);
    void Wizard_MixFile_Page1(void);
    void Wizard_CmpQuery_Page1(void);
    void Wizard_MergeQuery_Page1(void);	// runned when merge datasets link is clicked

    void Wizard_BinColors(void);
    void Wizard_SavePatReport(void);
    void Wizard_ExportWafermap(void);
    void Wizard_ExportSBL(void);

    // ExaminatorMonitoring
    void Wizard_GexMo_Tasks(void);
    void Wizard_GexMo_History(void);

    // GTM
    void ShowGtmWidget(void);

    // Toolbox
    void Wizard_GexTb(void);                // Shows Toolbox home page.
    void Wizard_GexTb_STDF(void);           // Convert file to STDF
    void Wizard_GexTb_DumpSTDF(void);       // Dump STDF Records into an ASCII file.
    void Wizard_GexTb_DbKeys(void);         // Edit DB Keys files
    void Wizard_GexTb_RegExp(void);         // Edit Regular Expressions
    void Wizard_GexTb_MergeRetest(void);    // Merge Test + Retest data into one new file!
#ifdef GCORE15334
    void Wizard_GexTb_FT_PAT_Limits(void);  // Create FT PAT recipe file.
    void Wizard_GexTb_WS_PAT_Limits(void);  // Create WS PAT recipe file.
#endif
    void Wizard_GexTb_ProgramPAT(void);     // Modify test program to be PAT ready
    void Wizard_GexTb_ReleasePAT(void);     // Release PAT config file to PAT-Man production folder
    void Wizard_GexTb_ReloadDatasetPAT(void); //Force reprocessing a data file (used when batch PAT processing from GUI)
    void Wizard_GexTb_CSV(void);            // Convert file to CSV

    // GEX Debug functions
    void Wizard_GexDebug_Function1(void);           // GEX Debug: Function1
    void Wizard_GexDebug_Function2(void);           // GEX Debug: Function2
    void Wizard_GexDebug_Function3(void);           // GEX Debug: Function3
    void Wizard_GexDebug_Function4(void);           // GEX Debug: Function4
    void Wizard_GexDebug_Function5(void);           // GEX Debug: Function5
    void Wizard_GexDebug_Vishay_Db_Cleanup(void);   // GEX Debug: Vishay GEXDB Cleanup

    // write SetReportType() section in csl for given grt and output format for report center
    bool WriteScriptGalaxyReportCenterSettingsSection(FILE* hFile, bool bFromWhatIf,
                                              QString strStartupPage, QString ouputFormat, QString grtfile);
    void WriteProcessFilesSection(FILE *hFile);

    // PRINT one specific HTML page...
    // No more used ?
    //void PrintHtmlFile(QPrinter *printer, QPainter *p, QString file);

    void    FreeDrillingAssistants(bool bFromWhatIf);
    QString GetProfileLabel(const QString& startupScript);
    void    UpdateCaption();
    void    AddNewUrl(QString selectedURL);

public:
    void ReloadProductSkin(bool bSplash);
    void AddStatusBarButton(QPixmap *pPixmap);

    // Change The name of the tab that contains the tab
    template< class TITLED_WIZARD >
      void ChangeWizardTabName
      ( TITLED_WIZARD *wizard, const QString &prefix, const QString &title )
      {
        if( mQDetachableTabWindowContainer )
        {
          if( wizard->isCustomTitle() )
          {
            mQDetachableTabWindowContainer->changeTabTitleByContent
                ( wizard, wizard->getTitle() );

            mQDetachableTabWindowContainer->changeWindowTitleByContent
                ( wizard, wizard->getTitle() );
          }
          else
          {
            mQDetachableTabWindowContainer->changeTabTitleByContent
                ( wizard, prefix + ' ' + title );

            mQDetachableTabWindowContainer->changeWindowTitleByContent
                ( wizard, title );
          }
        }
      }

public slots:
    // public slots are the functions available through QScriptEngine (currently ECMA JavaScript)

    void UpdateActionIcon(int);
    // Get the tool bar
    QToolBar* GetToolBar() { return toolBarButton; }

    // update the GexScriptStatusLabel (which should not be global)
    void UpdateLabelStatus(const QString &message);

    // Exit GEX application...
    void ExitGexApplication(int nExitCode);		// Exit GEX application

    // User clicked link to see 'Settings' page
    void ViewSettings(void);

    // Build OPTIONS URL, and refresh it into the brwoser to trigger the display of its dialog box.
    void ViewOptions(void);

    // launched when user clicks ReportCenter button or link
    void ShowReportsCenter(void);
    void ShowOptionsCenter(void);
    void ShowLogsCenter(void);
    void ShowJavaScriptCenter(void);
    void ShowSqlBrowser(void);

    // Shows/Hides the relevant Wizard Dialog box.
    void ShowWizardDialog(int iPageID, bool fromHTLBrowser = true);

    //	Options center slots
    // Load options from user profile file
    void OnLoadUserOptions(void);

    // Save options to user profile file
    void OnSaveUserOptions(void);

    // Save options to user profile file
    void OnDefaultOptions(void);

    // Slot called when OptionsCenter detects an option change by the user
    void SlotOptionChanged(QString section, QString optionname, QString newvalue);
    // Slot to record a change made in options csl and update the GUI accordingly
    void OnOptionChanged();

    void BuildReportSetDrill(void);

    // Build report now : bFromWhatIf(Drill ?), QString ouputFormat, QString strRCgrtFile
    // returns "ok" or "error : ..." with the error message
    QString BuildReportNow(bool bFromWhatIf, QString strStartupPage="home",
                         bool bCreateReport=true, QString ouputFormat="",
                           QString strRCgrtFile="");

    // Build report from Assistants selections...do not
    // process drill settings yet...this is the first pass
    // when generating a standard report.
    void BuildReport();

    void OnGexLinkHovered(const QString &);
    void OnGexLinkHovered(const QString&, const QString&, const QString&);
    void OnHeaderPageLoaded();
    void OnHeaderLinkClicked(const QUrl&);
    void OnFooterLinks(const QUrl&);
    void OpenCopyrightLicense(void);
    void OpenExamQuickStart(void);
    void OpenExamInstall(void);
    void OpenUserLicense(void);
    void OpenRH(void);
    void OpenTerInstall(void);
    void OpenTerRH(void);
    void OpenYMQuickStart(void);
    void OpenYMInstall(void);
    void OpenBundledPdf(QString fileName);
    void OnHighlightFooterLinks(QString );
    void OnStop();
    void OnSnapshot();
    void SnapshotCopy();
    void SnapshotSave();
    void OnHome();
    // Email icon clicked: compress report to ease Email task!
    void OnEmail();
    void OnFind(void);
    void OnContextualHelp(void);
    void OnBrowseUrl();
    void AddNewUrl(const QUrl&);
    void ViewNewUrl();
    void OnEngineStarting();
    void OnEngineReady();
    void OnLicenseGranted();

    void OnClose(void);
    void OnSaveSettingsCharts();
    void OnSaveEnvironment();
    // Timer based: check if script is completed...
    void OnCSLScriptFinished(const GS::Gex::CSLStatus& lStatus);
    void OnCSLScriptStarted();
    void UpdateProgressStatus(bool lSet, int lRange, int lStep, bool lShow);
    void HideProgress();
    void OnShowWizardHelp(void);
    void ViewClientInfo(void);
    // will show DB admin widget if license allows
    void Wizard_AdminGui(void);
    // will show/browse to the BI server main page
    // Load a new url in the web browser
    void LoadUrl(const QString& strUrl);
    // Reload the current url in the web browser
    void reloadUrl();

    // Update GexProgressBar progress
    void UpdateProgress(int prog);
    // Reset GexProgressBar progress
    void ResetProgress(bool forceCompleted);
    // Set Maximumfor GexProgressBar progress
    void SetMaxProgress(int max);
    // Set Busyness of apllication
    void SetBusyness(bool isBusy);


private slots:

    void ScriptingLink(void);
    void onBrowserAction(const GexWebBrowserAction& webBrowserAction);

signals:
    void RefreshDB();
    void sShortcut();
    //void sWizardTypeChanged();		// PYC, 27/05/2011
public:
    bool forceToComputeFromDataSample(FILE *hFile, int iProcessPart);

        QMovie      *mAddElementInReportBuilderMovie;
private:
    QUndoView  *m_poUndoView;
    QUndoStack *m_poUndoStack;
    QToolButton *mActionLicRelease;


    bool        mOnClose;

    template<typename Wizard>
    void AddWizardInDetachableWindow(Wizard* wizard, const QString& label);

    analyseMode mAnalyseMode;

public:

    QUndoView  *getUndoView(){
        return m_poUndoView;
    }

    QUndoStack *getUndoStack(){
        return m_poUndoStack;
    }

public:
    void enableLicRelease();


    void OnNewWizard(int typeOfWizard, const QString &link=QString());

    /**
     * \function  void SaveCurrentProfile()
     * \brief     saves the current profile before charging the new profile (it has been developped for the load csl)
     *
     **/
    void SaveCurrentProfile();
    void LoadProfileFromScriptingCenter();
};

#endif
