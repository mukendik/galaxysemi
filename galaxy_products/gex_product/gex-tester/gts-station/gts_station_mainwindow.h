#ifndef gts_station_mainwindow_H
#define gts_station_mainwindow_H

#include <QList>
#include <QMap>
#include <QPair>
#include <QPixmap>
#include <QDateTime>
#include <QMessageBox>
#include <QMap>
#include <QCloseEvent>
#include <QDropEvent>
#include <QDragEnterEvent>

#include <stdfparse.h>
#include <gqtl_log.h>

#include "ui_gts_station_mainwindow.h"
#include "gts_station_objects.h"

class QApplication;
class GtsStationInfowidget;
class GtsStationStatswidget;
class GtsStationOutputwidget;
class GtsStation_GtlCommandsdialog;
class GtsStationSetupwidget;
class GtsStationGtlWidget;

typedef struct t_GtsStation_Page
{
	QWidget*	pPage;
	bool		bDisableWhenRunning;
} GtsStation_Page;

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GtsStation_TestDef
// Test Definition (flags, limits...)
/////////////////////////////////////////////////////////////////////////////////////////
class GtsStation_TestDef
{
public:
    GtsStation_TestDef(GQTL_STDF::Stdf_PTR_V4 *PtrRecord);
    GtsStation_TestDef(GQTL_STDF::Stdf_MPR_V4* MprRecord);
    ~GtsStation_TestDef();

    QString         GetTestName();

    bool			Update(GQTL_STDF::Stdf_PTR_V4 *CurrentPTR);
    bool			Update(GQTL_STDF::Stdf_MPR_V4 *CurrentMPR);
    bool			HasLowLimit();
	bool			HasHighLimit();

protected:
    // PTR holding test limits, test name...
    GQTL_STDF::Stdf_PTR_V4*     mPTR;
    GQTL_STDF::Stdf_MPR_V4*     mMPR;

	// Limits forced by test program (or PAT library)
    bool			mForceNewLimits;
    bool			mNewLimitsValid;
    float			mNewLowLimit;
    float			mNewHighLimit;

	// To update TSR records
    unsigned int	mExecCount;
    unsigned int	mFailCount;
};

typedef QMap<long, GtsStation_TestDef *> GtsTestlistMap;

// Properties
#define GTS_PROPERTY_STATION_NB         "StationNb"
#define GTS_PROPERTY_RUNNING_MODE       "RunningMode"
#define GTS_PROPERTY_HIDDEN             "Hidden"
#define GTS_PROPERTY_AUTO_CLOSE         "AutoClose"
// delay to force between 2 runs (ms)
#define GTS_PROPERTY_RUN_DELAY          "RunDelay"
// delay to force before stopping (unload) the test program (ms)
#define GTS_PROPERTY_STOP_DELAY         "StopDelay"
#define GTS_PROPERTY_TC_FILE            "TesterConfigFile"
#define GTS_PROPERTY_RECIPE_FILE        "RecipeFile"
#define GTS_PROPERTY_STDF_FILE          "StdfFile"
#define GTS_PROPERTY_DATAFOLDER         "DataFolder"
#define GTS_PROPERTY_QA_OUTPUT_FOLDER   "QaOutputFolder"
// true if running in automatic mode
#define GTS_PROPERTY_AUTO_RUN           "AutoRun"
#define GTS_PROPERTY_GTM_COMM_MODE      "GtmCommunicationMode"
#define GTS_PROPERTY_MULTIFILE_MODE     "MultiFileMode"
#define GTS_PROPERTY_DESIRED_LIMITS     "DesiredLimits"
#define GTS_PROPERTY_AUTO_TESTNUMBERS   "AutoTestNumbers"

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GtsStationMainwindow
// Main station window
/////////////////////////////////////////////////////////////////////////////////////////
class GtsStationMainwindow : public QMainWindow, public Ui::GtsStationMainwindow
{
    Q_OBJECT

public:
    enum RunningMode
    {
        Normal,
        QA,
        Bench
    };

    GtsStationMainwindow(QApplication* qApplication, const QString & ApplicationPath, const QString & ApplicationName,
                         QWidget *parent=0);
    ~GtsStationMainwindow();
    
    bool            InitGts(int argc, char ** argv);

    Q_INVOKABLE unsigned int	StationNb() { return mProperties.value(GTS_PROPERTY_STATION_NB, QVariant(1)).toUInt(); }
    Q_INVOKABLE unsigned int	RunDelay() { return mProperties.value(GTS_PROPERTY_RUN_DELAY, QVariant(0)).toUInt(); }
    Q_INVOKABLE unsigned int	StopDelay() { return mProperties.value(GTS_PROPERTY_STOP_DELAY, QVariant(0)).toUInt(); }
    Q_INVOKABLE unsigned int	NbSites()	{ return m_listSites.count(); }
    Q_INVOKABLE unsigned int	PartCount();
    Q_INVOKABLE unsigned int	PassCount();
    Q_INVOKABLE unsigned int	FailCount();
    void			TestJobName(QString & strTestJobName) { strTestJobName = m_bProgramLoaded ?  mKeysContent.Get("ProgramName").toString() : ""; }
    void			TestJobFile(QString & strTestJobFile) { strTestJobFile = m_bProgramLoaded ?  mKeysContent.Get("ProgramName").toString() + ".load" : ""; }
	void			TestJobPath(QString & strTestJobPath) { strTestJobPath = m_bProgramLoaded ?  m_strStdfFilePath : ""; }
    void			TestDataFile(QString & strTestDataFile) { strTestDataFile = m_bProgramLoaded ?  m_strStdfFileName : ""; }
    void			TestSourceFilesPath(QString & strTestSourceFilesPath) { strTestSourceFilesPath = m_bProgramLoaded ?  m_strStdfFilePath : ""; }
    void			ProductID(QString & strProductID) { strProductID = m_bProgramLoaded ?  mKeysContent.Get("Product").toString() : ""; }
    void			OperatorName(QString & strOperatorName) { strOperatorName = m_bProgramLoaded ?  mKeysContent.Get("Operator").toString() : ""; }
    void			JobRevision(QString & strJobRevision) { strJobRevision = m_bProgramLoaded ?  mKeysContent.Get("ProgramRevision").toString() : ""; }
    void			LotID(QString & strLotID) { strLotID = m_bProgramLoaded ?  mKeysContent.Get("Lot").toString() : ""; }
    void			SublotID(QString & strSublotID) { strSublotID = m_bProgramLoaded ?  mKeysContent.Get("SubLot").toString() : ""; }
    void			TesterName(QString & strTesterName) { strTesterName = m_bProgramLoaded ?  mKeysContent.Get("TesterName").toString() : ""; }
    void			TesterType(QString & strTesterType) { strTesterType = m_bProgramLoaded ?  mKeysContent.Get("TesterType").toString() : ""; }
    void			Facility(QString & strFacility) { strFacility = m_bProgramLoaded ?  mKeysContent.Get("Facility").toString() : ""; }
    void			Family(QString & strFamily) { strFamily = m_bProgramLoaded ?  mKeysContent.Get("Family").toString() : ""; }
    void			SpecName(QString & strSpecName) { strSpecName = m_bProgramLoaded ?  mKeysContent.Get("SpecName").toString() : ""; }
    void			UserText(QString & strUserText) { strUserText = m_bProgramLoaded ?  mKeysContent.Get("UserText").toString() : ""; }
    void			Temperature(QString & strTemperature) { strTemperature = m_bProgramLoaded ?  mKeysContent.Get("Temperature").toString() : ""; }
    void			TestingCode(QString & strTestingCode) { strTestingCode = m_bProgramLoaded ?  mKeysContent.Get("TestingCode").toString() : ""; }
    void			Station(QString & strStation);
    void			RetestIndex(QString & strRetestIndex) { strRetestIndex = m_bProgramLoaded ?  mKeysContent.Get("RetestIndex").toString() : ""; }
    void			ModeCode(QChar & cModeCode);
    void			ApplicationName(QString & strApplicationName) { strApplicationName = m_strApplicationName; }
    void            FillSiteNumbersString(QString & SiteNumbers);
    void			FillSiteNumbersArray(int *pnSiteNumbers);
    Q_INVOKABLE bool isRunningModeQA();
    Q_INVOKABLE bool isRunningModeBench();
    Q_INVOKABLE bool isAutoRun() {return mProperties.value(GTS_PROPERTY_AUTO_RUN, QVariant(false)).toBool();}
    Q_INVOKABLE bool RunQA();
    Q_INVOKABLE bool isHidden() {return mProperties.value(GTS_PROPERTY_HIDDEN, QVariant(false)).toBool();}
    Q_INVOKABLE bool isAutoClose() {return mProperties.value(GTS_PROPERTY_AUTO_CLOSE, QVariant(false)).toBool();}
    Q_INVOKABLE bool isAutoTestNumbers() {return mProperties.value(GTS_PROPERTY_AUTO_TESTNUMBERS, QVariant(false)).toBool();}

	// Overwrite virtual fcts from main class for Drag&Drop support.
	void			dragEnterEvent(QDragEnterEvent *);
    void			dropEvent(QDropEvent *);

signals:
    void sCloseStation(void *);
    void sLoadStdfFile(const QString &);

// Overrides
protected:
    void			closeEvent(QCloseEvent *e);

protected:
	// Enums
	enum Status
	{
		StatusReady
	};
	enum ErrorCode
	{
		eNoError
	};
	enum InfoCode
	{
		iStartApp,
		iStopApp
	};

	// Variables
    // GTS properties/options
    QVariantMap                     mProperties;
    // List of STDF files to process: key is (RetestIndex, start_t), value is (StdfFileName, KeysContent)
    GtsStdfFiles                    mStdfFiles;
    GQTL_STDF::StdfParse        mStdfReader;				// STDF reader
    GQTL_STDF::StdfParse        mStdfWriter;				// STDF writer
	QString							m_strApplicationPath;		// Application path
	QString							m_strApplicationName;		// Application name
    QList<GtsStation_Page>          m_listPages;				// List og widget pages
	unsigned int					m_uiPage_Help;				// Help page ID
	unsigned int					m_uiPage_Main;				// Main page ID
	unsigned int					m_uiPage_Stats;				// Stats page ID
	unsigned int					m_uiPage_Output;			// Output page ID
	unsigned int					m_uiPage_Setup;				// Setup page ID
    unsigned int					m_uiPage_GtlWidget;			// GTL widget page ID
    QMessageBox						m_qMessageBox;				// Messagebox with GTS icon
    //! \brief Help page
    QTextEdit                       mHelpPage;
	bool							m_bProgramLoaded;			// TRUE if a test program is loaded, FALSE else
	QString							m_strStdfFullFileName;		// STDF file loaded (full name)
	QString							m_strStdfFileName;			// STDF file loaded (without path)
	QString							m_strStdfFilePath;			// Path to STDF file loaded
	QString							m_strStdfFullFileName_Out;	// STDF file generated (full name)
    // Keys content (for quick read and config keys file processing)
    GS::QtLib::DatakeysContent      mKeysContent;
    GQTL_STDF::Stdf_PIR_V4         m_clLastPIR;                 // Last PIR read from STDF file
	bool							m_bUseLastPIR;				// Set to TRUE if m_clLastPIR contains last PIR read and should be used
	GtsStation_SiteList				m_listSites;				// List of sites
	GtsTestlistMap					m_mapTestlist;				// Map with TestNumber/TestName.
	GtsStation_GtlCommandsdialog	*m_pGtlCommandsDialog;		// Dialog to run with GTL arguments
	int								m_nStdf_CPU_Type;			// CPU_Type of input STDF file
	bool							m_bPartIsFail;				// Set to TRUE when a test fails
	bool							m_bGenerateStdfFile;		// TRUE if simulator should generate a STDF file.
    QString                         mCurrentDir;                // Used to store directory when selecting files
                                                                // with QFileDialog::getOpenFileName()
    QString                         mDataFolder;                // Directory containing data files to process
    int                             mLastAutoTestNumber;        // Last test number used with auto-increment test number option

    // Counters
    unsigned int            m_ui_gtl_test_lib;
    unsigned int            m_ui_gtl_mptest_lib;
    unsigned int            m_ui_gtl_close_lib;
    unsigned int            m_ui_gtl_beginjob_lib;
    unsigned int            m_ui_gtl_endjob_lib;
    unsigned int            m_ui_gtl_binning_lib;
    unsigned int            m_uiTotalPartsInSTDF;		// Total parts in STDF file

	// QT Application
	QApplication					*m_qApplication;

	// Widgets
	GtsStationInfowidget			*m_pageInfo;
	GtsStationStatswidget			*m_pageStats;
	GtsStationOutputwidget			*m_pageOutput;
	GtsStationSetupwidget			*m_pageSetup;
    GtsStationGtlWidget             *m_pageGtlWidget;

	// Methods
    bool                            CheckArguments(int argc, char ** argv);
    bool                            GTL_Open(bool RemoveTempGtlSqlite);
    bool                            GTL_Close();
    bool                            GTL_Retest();
    bool                            GTL_BeginJob();
    bool                            GTL_EndJob();
    bool							ValidateStdfFile(QString strFileName);
    bool							RunTestProgram();
    //! \brief Load command: ?
    bool							LoadCommand(const QStringList & FileNames);
    bool							RunCommand();
	bool							DeleteCommand();
    void							SetStatus(int nStatus);
    void							AddPage(const QString& strSelectionText, const QPixmap& pixmap, QWidget* pPage, bool bDisableWhenRunning);
	void							ClearStation();
	void							ResetStation();
	void							InitStation();
    void                            UpdateGtlState();
    void							UpdateSummary();
	void							SetStdfOutputFileName();
    bool                            ProcessPRR();
    bool                            ProcessPTR();
    bool                            ProcessMPR();
    void                            RunBenchmark();
    bool                            LoadStdfFile(const QString & StdfFile);
    bool                            RunAll();
    void                            Message(const QString &lMessage, const int Severity=SYSLOG_SEV_NOTICE);
    void                            Warning(const QString &lMessage, const int Severity=SYSLOG_SEV_WARNING);
    void                            Error(const QString &lMessage, const int Severity=SYSLOG_SEV_ERROR);
    bool                            EndLot();

public slots:
    void                            OnButtonLoadDelete();
    void                            OnButtonExit();

protected slots:
    void    OnJobNameChanged(const QString & text);
    void	OnMainSelectionChanged();
    void	OnButtonHelp();
    void	OnButtonRun();
    void	OnButtonRunN();
    void    OnButtonRunAll();
    void	OnButtonReset();
    void	OnButtonDatalog();
    void	DisplayHelp();
    void	OnButtonGtlCommands();
    void	OnGtlCommandsDialog_Close();
    void	OnGtlCommandsDialog_Send(unsigned int uiCommand, long lTestNumber);
    void	OnButtonNewlot();
    void    OnButtonRecipe();
    void    OnButtonTesterConf();
    void    PopGtlMessageStack(bool lDisplay=true);
};

#endif // gts_station_mainwindow_H
