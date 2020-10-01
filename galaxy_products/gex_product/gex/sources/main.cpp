///////////////////////////////////////////////////////////
// Entry point of the whole GEX application..
///////////////////////////////////////////////////////////

#ifdef _WIN32
#include <windows.h>

#ifndef SID_MAX_SUB_AUTHORITIES
#define SID_MAX_SUB_AUTHORITIES 15
#endif
#elif (defined __sun__) // Solaris stuff
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#else // linux stuff
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#if __MACH__
#include <sys/uio.h>
#include <sys/mount.h>
#else
#include <sys/vfs.h>
#endif // _WIN32
#endif

#include <QFileInfo>
#include <QFile>
#include <QImage>
#include <QApplication>
#include <QDir>
#include <QNetworkInterface>
#include <QList>
#include <QSysInfo>
#include <QHostInfo>
#include <QHostAddress>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <QScriptValue>
#include <QScriptEngineDebugger>
//#include <QSystemStorageInfo> // To Do : compile and include Qt Mobility ?

#ifndef GSDAEMON
    #include "gtm_testerwindow.h"
    #include <QMessageBox>
    #include <QToolBar>
    #include <QWebView>
    #include <QVBoxLayout>
#endif

#include <gqtl_log.h>
#include <pat_options.h>

#include "db_gexdatabasequery.h"
#include "gqtl_datakeys.h"
#include "db_inserted_files.h"
#include "gqtl_datakeys_engine.h"
#include "engine.h"
#include "csl/csl_engine.h"
#include "command_line_options.h"
#include "license_provider.h"
#include "gqtl_datakeys.h"

#if MODULE == GEX
    #include "browser_dialog.h"
    #include "reports_center_field.h"
    #include "reports_center_params_widget.h"
    // Service implementation only on Windows!!
    #include "daemon.h"
#endif

#include "import_all.h"
#include "gex_scriptengine.h"
#include "gex_database_entry.h"
#include "gex_pat_processing.h"
#include "gex_version.h"
#include "file.h"
#include "mo_task.h"
#include "outlierremoval/outlierremoval_taskdata.h"
#include "report_options.h"
#include "tb_merge_retest.h"    // GexTbMergeRetest
#include "ctest.h"
#include "colors_generator.h"
#include "temporary_files_manager.h"
#include "gexdb_plugin_base.h"
#include "product_info.h"
#include "admin_engine.h"
#include "db_engine.h"
#include "message.h"
#include "export_atdf.h" // in order to register CSTDFtoATDF
#include "export_ascii.h" // in order to register CSTDFtoASCII
#include "pat_gts_station.h"
#include "stats_engine.h"
#include "wafer_export.h"
#include "wafermap.h"
#include "bin_description.h"
#include "pat_engine.h"
#include "ofr_controller.h"

#ifdef _WIN32
    #include <cpuid.h>
#endif

#include <fenv.h>

#include "user_input_filter.h"
#include "license_provider_manager.h"
#include "map_merge_api.h"

// Pixmap icons
#include <QPixmap>
#include <gqtl_sysutils.h>
#include "gex_pixmap.h"

// Examinator Revision
const char* szDefaultServiceName			= "Examinator Yield & PAT Service";
const char* szDefaultServiceDescription     = "Examinator Yield & PAT management (visit http://www.mentor.com)";

CGexSkin*			pGexSkin				= NULL;	// create object the latest possible to check if everything's alright
GexScriptEngine*	pGexScriptEngine		= NULL;	// global for the moment
QProgressBar *		GexProgressBar			= NULL;	// Handle to progress bar in status bar
QLabel *			GexScriptStatusLabel	= NULL;	// Handle to script status text in status bar

#if MODULE == GEX
GexMainwindow *		pGexMainWindow			= NULL;
#endif

QString				strIniClientSection;	//

// ############ DEBUG TOOL ############
// For Debug: Dump Memory Leaks detected
//#include "DebugMemory.h" // Does not work prefectly
#include <new>

static unsigned sCurrentAllocated=0;

#if defined(QT_DEBUG) && defined(GS_NEW)
    // Temporary test in order to catch mem leaks.

    QMap<void*, size_t> mAllocated;

    void* operator new(size_t sz)
    {
      //printf("operator new: %d octets\n", sz);
      void* m = malloc(sz);
      if(!m)
      {
          puts("low memory");
      }
      else
      {
          mAllocated.insert(m, sz);
          sCurrentAllocated+=sz;
      }
      return m;
    }

    void operator delete(void* m)
    {
      if (mAllocated.contains(m))
      {
          sCurrentAllocated-=mAllocated.value(m);
          mAllocated.remove(m);
      }
      else
          GSLOG(4, QString("Unknown allocation ")); //.arg(m, 0, 16) );
      free(m);
    }
#endif

// Pixmap images used in different places in the project
QPixmap *pixGexApplication		= NULL;
QPixmap *pixChildTree			= NULL;
QPixmap *pixTestStatistics		= NULL;
QPixmap *pixHisto				= NULL;
QPixmap *pixWafermap			= NULL;
QPixmap *pixAdvHisto			= NULL;
QPixmap *pixTrend				= NULL;
QPixmap *pixShift				= NULL;
QPixmap *pixScatter				= NULL;
QPixmap *pixBoxPlot				= NULL;
QPixmap *pixProbabilityPlot		= NULL;
QPixmap *pixMultiCharts			= NULL;
QPixmap *pixPareto				= NULL;
QPixmap *pixBinning				= NULL;
QPixmap *pixDatalog				= NULL;
QPixmap *pixOutput				= NULL;
QPixmap *pixOutlier				= NULL;
QPixmap *pixBoxMeanRange		= NULL;
QPixmap *pixPrinter				= NULL;
QPixmap *pixGetStarted			= NULL;
QPixmap *pixTasks				= NULL;
QPixmap *pixSeparator			= NULL;
QPixmap *pixDisabled			= NULL;
QPixmap *pixGuardBanding		= NULL;
QPixmap *pixPatHistory			= NULL;
QPixmap *pixNetwork				= NULL;
QPixmap *pixStandalone			= NULL;
QPixmap *pixMoDataPump			= NULL;
QPixmap *pixMoYieldMonitoring	= NULL;
QPixmap *pixMoSpecMonitoring	= NULL;
QPixmap *pixCalendar			= NULL;
QPixmap *pixSave				= NULL;
QPixmap *pixCopy				= NULL;
QPixmap *pixStopwatch			= NULL;
QPixmap *pixCSVSpreadSheet				= NULL;
QPixmap *pixWord				= NULL;
QPixmap *pixPdf					= NULL;
QPixmap *pixPowerpoint			= NULL;
QPixmap *pixRemove				= NULL;
QPixmap *pixOpen				= NULL;
QPixmap *pixCreateFolder		= NULL;
QPixmap *pixProperties			= NULL;
QPixmap *pixGlobalInfo			= NULL;
QPixmap *pixAdvFunctional       = NULL;
QPixmap *pixAdvFTRCorr          = NULL;

extern CReportOptions ReportOptions;		// Holds options (report_build.cpp/h)
//extern CGexReport *gexReport; // in order to be registered in ScriptEngine
//extern QScriptValue toScriptValue(QScriptEngine *engine, const GexMainwindow::ClientState &s);
//extern void fromScriptValue(const QScriptValue &obj, GexMainwindow::ClientState &s);
extern QScriptValue createClientState(QScriptContext *, QScriptEngine *engine);
extern QString UpdateLogLevels(const QMap<QString, int> &m);

///////////////////////////////////////////////////////////
// QT Warning/Error handler...under Unix only report Fatal errors.
///////////////////////////////////////////////////////////
#if defined unix || __MACH__
void myMessageOutput( QtMsgType type, const char *msg )
{
    switch ( type )
    {
    case QtDebugMsg:
        // fprintf( stderr, "GEX Debug: %s\n", msg );
        break;
    case QtWarningMsg:
        // fprintf( stderr, "GEX Warning: %s\n", msg );
        break;
    case QtCriticalMsg:
        // fprintf( stderr, "GEX Critical: %s\n", msg );
        break;
    case QtFatalMsg:
        fprintf( stderr, "GEX Fatal: %s\n", msg);
        GSLOG(SYSLOG_SEV_CRITICAL, msg);
        exit(EXIT_FAILURE);  // dump core on purpose
    }
}
#endif

///////////////////////////////////////////////////////////
// For debug purpose, append string to file
///////////////////////////////////////////////////////////
void WriteDebugMessageFile(const QString & strMessage)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, strMessage.toLatin1().constData() );

    if(GS::Gex::Engine::GetInstance().GetCommandLineOptions().IsCustomDebugMode())
    {
        FILE *hFile=0;
        QString			strTraceFile;

        // Construct file name
        strTraceFile = GS::Gex::Engine::GetInstance().Get("GalaxySemiFolder").toString() +
                       "/logs/" + QDate::currentDate().toString(Qt::ISODate);
        QDir d(strTraceFile);
        if (!d.exists(strTraceFile))
            if (!d.mkpath(strTraceFile))
                return;

        strTraceFile += QDir::separator() + QString("gslog_");
        strTraceFile += QDate::currentDate().toString(Qt::ISODate);
        strTraceFile += ".txt";
        CGexSystemUtils::NormalizePath(strTraceFile);

        // Write message to file
        hFile = fopen(strTraceFile.toLatin1().constData(),"a");
        if(hFile == NULL)
            return;
        fprintf(hFile,"[%s %s ?] %s\n",
                QDate::currentDate().toString(Qt::ISODate).toLatin1().constData(),
                QTime::currentTime().toString(Qt::ISODate).toLatin1().constData(),
                strMessage.toLatin1().constData());
        fclose(hFile);
    }

}

///////////////////////////////////////////////////////////
// Cleanup Gex Install when license has expired
///////////////////////////////////////////////////////////
bool cleanupOnExpiration()
{
    // Trace message
    QString strMessage = "License expired cleanup...";
    GSLOG(7, strMessage.toLatin1().data());

    // ********* REMOVE PDF DOC FILE
    // User Manual keys
    QString strPDFDocFilePrefix, strPDFDocFileSuffix;
    bool bRemovePDFFile = true;

    // If Examinator OEM for the Credence tester, no PDF to remove...
    if(GS::LPPlugin::ProductInfo::getInstance()->getProductID() == GS::LPPlugin::LicenseProvider::eSzOEM)
        bRemovePDFFile = false;
    else if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
        strPDFDocFilePrefix = QString("PM_");
    else if(GS::LPPlugin::ProductInfo::getInstance()->isYieldMan())
        strPDFDocFilePrefix = QString("YM_");
    else
        strPDFDocFilePrefix = QString("GEX_");


    strPDFDocFileSuffix = QString("V") + QString::number(GEX_APP_VERSION_MAJOR) +QString(".") + QString::number(GEX_APP_VERSION_MINOR);

    // Build path to doc pdf file
    QString strPDFDocFilePath = GS::Gex::Engine::GetInstance().Get("UserFolder").toString() +
                                QDir::separator() + strPDFDocFilePrefix + "UserManual_" +
                                strPDFDocFileSuffix +".pdf";

    strPDFDocFilePath = QDir::cleanPath(strPDFDocFilePath);
    // Remove the file
    if (QFile::exists(strPDFDocFilePath) && bRemovePDFFile)
    {
        if (QFile::remove(strPDFDocFilePath))
        {
            strMessage = strPDFDocFilePath + " removed.";
            GSLOG(7, strMessage.toLatin1().data());
            return true;
        }
        else
        {
            strMessage = "Unable to remove existing file: " + strPDFDocFilePath + ".";
            GSLOG(7, strMessage.toLatin1().data());
        }
    }

    // Nothing to clean
    return false;
}

static QScriptValue SetExpirationDate(QScriptContext *context, QScriptEngine *engine)
{
    //QScriptValue callee = context->callee();
    if (context->argumentCount() == 1) // writing?
    {
        QDate ed = context->argument(0).toDateTime().date(); //callee.setProperty("value", context->argument(0));
        GS::Gex::Engine::GetInstance().SetExpirationDate(ed);
    }
    return engine->newDate( QDateTime( GS::Gex::Engine::GetInstance().GetExpirationDate() ) );    //callee.property("value");
}

//________________________________________________________________________________________
// Move me in gex_script_engine.cpp

template <typename T>
QScriptValue qScriptValueFromQObject(QScriptEngine *engine, T const &qobject)
{
    return engine->newQObject(qobject);
}

template <typename T>
void qScriptValueToQObject(const QScriptValue &value, T &qobject)
{
    qobject = qobject_cast<T>(value.toQObject());
}

template <typename T>
int qScriptRegisterQObjectMetaType(
        QScriptEngine *engine,
        const QScriptValue &prototype /*= QScriptValue()*/,
        T* /* dummy  = 0*/
        )
{
    return qScriptRegisterMetaType<T>(
                engine, qScriptValueFromQObject, qScriptValueToQObject, prototype);
}

// Q_DECLARE_METATYPE needs both a default & copy constructor
Q_DECLARE_METATYPE(GexDatabaseEntry*)
Q_DECLARE_METATYPE(GexDatabaseEntry)
Q_DECLARE_METATYPE(GS::Gex::PATProcessing*)
Q_DECLARE_METATYPE(GS::Gex::PATProcessing)
Q_DECLARE_METATYPE(GS::Gex::BinDescription)
Q_DECLARE_METATYPE(GS::Gex::BinDescription*)
// For the moment CGexCompositePatProcessing is not needed but who knows
//Q_DECLARE_METATYPE(CGexCompositePatProcessing)
//Q_DECLARE_METATYPE(CGexCompositePatProcessing*)
Q_DECLARE_METATYPE(GS::Gex::WaferExport*)
//Q_DECLARE_METATYPE(GS::Gex::WaferExport) // needs a copy constructor
Q_DECLARE_METATYPE(COptionsPat*)
Q_DECLARE_METATYPE(CWaferMap*)
Q_DECLARE_METATYPE(ColorsGenerator*)
//Q_DECLARE_METATYPE(COptionsPat)
Q_DECLARE_METATYPE(GS::Gex::CSLStatus*)
Q_DECLARE_METATYPE(GS::Gex::CSLStatus)
Q_DECLARE_METATYPE(GexDatabaseInsertedFiles)
//Q_DECLARE_METATYPE(CGexMoTaskItem) // needs a copy constructor
Q_DECLARE_METATYPE(CGexMoTaskItem*)

Q_DECLARE_METATYPE(GS::Gex::MapMergeApi*)
//Q_DECLARE_METATYPE(GS::Gex::MapMergeApi) // needs a copy constructor

Q_DECLARE_METATYPE(CSTDFtoATDF)
Q_DECLARE_METATYPE(GS::Gex::ConvertToSTDF)
Q_SCRIPT_DECLARE_QMETAOBJECT(GS::Gex::ConvertToSTDF, QObject*)

Q_SCRIPT_DECLARE_QMETAOBJECT(QFile, QObject*)

#ifndef GSDAEMON
    Q_SCRIPT_DECLARE_QMETAOBJECT(QToolBar, QWidget*)
    Q_SCRIPT_DECLARE_QMETAOBJECT(QDialog, QWidget*)
    Q_SCRIPT_DECLARE_QMETAOBJECT(QMessageBox, QWidget*)
    Q_SCRIPT_DECLARE_QMETAOBJECT(QWebView, QWidget*)
    Q_SCRIPT_DECLARE_QMETAOBJECT(QLabel, QWidget*)
    Q_SCRIPT_DECLARE_QMETAOBJECT(QLineEdit, QWidget*)
    Q_SCRIPT_DECLARE_QMETAOBJECT(QVBoxLayout, QWidget*)
#endif

// these class needs a constructor with a QObject* as first arg
//Q_SCRIPT_DECLARE_QMETAOBJECT(GexDatabaseEntry, QObject*)
Q_SCRIPT_DECLARE_QMETAOBJECT(GS::Gex::CSLStatus, QObject*)
Q_SCRIPT_DECLARE_QMETAOBJECT(GS::Gex::PATProcessing, QObject*)
Q_SCRIPT_DECLARE_QMETAOBJECT(GS::Gex::MapMergeApi, QObject*)
//Q_SCRIPT_DECLARE_QMETAOBJECT(CGexMoTaskItem, QObject*)
Q_SCRIPT_DECLARE_QMETAOBJECT(CWaferMap, QObject*)
Q_SCRIPT_DECLARE_QMETAOBJECT(GS::Gex::BinDescription, QObject*)
Q_SCRIPT_DECLARE_QMETAOBJECT(GS::QtLib::DatakeysContent, QObject*)

bool UpdateGexScriptEngine(CReportOptions* pReportOptions)
{
    if (!pGexScriptEngine )
    {
        GSLOG(4, "GexScriptEngine not yet initialized !");
        return false;
    }

    pGexScriptEngine->setObjectName("GSScriptEngine");
    // no more usefull : use GexEngine.Get('ApplicationDir') instead
    //pGexScriptEngine->globalObject().setProperty(PROP_APP_DIR,
      //GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString(), QScriptValue::ReadOnly);

    // No more usefull : use GexEngine.Get('UserFolder') instead
    // Warning : strUserFolder is perhaps not yet well defined !
    //pGexScriptEngine->globalObject().setProperty(PROP_GEX_USER_FOLDER,
      //GS::Gex::Engine::GetInstance().Get("UserFolder").toString(), QScriptValue::ReadOnly);


    /*
    // ToDo : find the current user name which has launched gex (Nokia, what are you waiting for ?)
    #ifdef WINDOWS
        pGexScriptEngine->globalObject().setProperty(PROP_GEX_USER, getenv("USERNAME") );
    #else	// LINUX
        pGexScriptEngine->globalObject().setProperty(PROP_GEX_USER, getenv("USER") );
    #endif
    */
    //
    pGexScriptEngine->globalObject().setProperty(PROP_LOCALE_NAME,
                              QLocale::system().name(), QScriptValue::ReadOnly );
    pGexScriptEngine->globalObject().setProperty(PROP_LOCALE_LANGUAGE,
                              QLocale::languageToString(QLocale::system().language()), QScriptValue::ReadOnly );
    pGexScriptEngine->globalObject().setProperty(PROP_LOCALE_COUNTRY,
                              QLocale::countryToString(QLocale::system().country()), QScriptValue::ReadOnly );
    pGexScriptEngine->globalObject().setProperty(PROP_LOCALE_DECIMALPOINT,
                              QScriptValue(QLocale::system().decimalPoint()), QScriptValue::ReadOnly );

    // ExpirationDate : QDate is not QObject
    //QScriptValue EDobject = pGexScriptEngine->newQObject( &ExpirationDate );
    //if (!EDobject.isNull())
    //QScriptValue EDobject = pGexScriptEngine->newObject();
    QScriptValue EDf=pGexScriptEngine->newFunction(SetExpirationDate);
    //EDobject.setProperty("GetSetExpirationDate", EDf, QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
    pGexScriptEngine->globalObject().setProperty("GetSetExpirationDate", EDf);

    #ifdef QT_DEBUG
        pGexScriptEngine->globalObject().setProperty(PROP_DEBUG, QScriptValue(true), QScriptValue::ReadOnly );
    #else
        pGexScriptEngine->globalObject().setProperty(PROP_DEBUG, QScriptValue(false), QScriptValue::ReadOnly );
    #endif

    QString osname("unknown");

#ifdef Q_OS_WIN
    switch (QSysInfo::windowsVersion())
    {
        case QSysInfo::WV_NT : osname="WV_NT"; break;
        case QSysInfo::WV_2000 : osname="WV_2000"; break;
        case QSysInfo::WV_XP : osname="WV_XP"; break;
        case QSysInfo::WV_2003 : osname="WV_2003"; break;
        case QSysInfo::WV_VISTA : osname="WV_VISTA"; break;
        case QSysInfo::WV_WINDOWS7 : osname="WV_WINDOWS7"; break;
        case QSysInfo::WV_WINDOWS8 : osname="WV_WINDOWS8"; break;
        default: osname="WV : Not NT based windows (Win98, ME,...) !"; break;
    }
#else
#if defined unix || __MACH__
    // UNIX Platform
#ifdef __sun__
    osname = "Sun-Solaris";
#endif
#ifdef hpux
    osname = "HP-UX";
#endif
#ifdef __linux__
    osname = "Linux";
#endif
#endif
#endif
    pGexScriptEngine->globalObject().setProperty(PROP_OS, QScriptValue(osname), QScriptValue::ReadOnly );

    QScriptValue PIobject = pGexScriptEngine->newQObject(GS::LPPlugin::ProductInfo::getInstance() );
    if (!PIobject.isNull())
        pGexScriptEngine->globalObject().setProperty("GSProductInfo", PIobject);

    if (GS::LPPlugin::ProductInfo::getInstance()->isYieldMan())
        pGexScriptEngine->globalObject().setProperty(PROP_GEXPRODUCT, PRODUCT_YIELDMAN, QScriptValue::ReadOnly );
    else if (GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
        pGexScriptEngine->globalObject().setProperty(PROP_GEXPRODUCT, PRODUCT_PATSERVER, QScriptValue::ReadOnly );
    else if (GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        pGexScriptEngine->globalObject().setProperty(PROP_GEXPRODUCT, PRODUCT_GTM, QScriptValue::ReadOnly );
    else if (GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT())
        pGexScriptEngine->globalObject().setProperty( PROP_GEXPRODUCT, PRODUCT_GEXPAT, QScriptValue::ReadOnly );
    else if (GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPRO() ||
             GS::LPPlugin::ProductInfo::getInstance()->isExaminatorTerProPlus())
        pGexScriptEngine->globalObject().setProperty( PROP_GEXPRODUCT, PRODUCT_GEXPRO, QScriptValue::ReadOnly );
    else if (GS::LPPlugin::ProductInfo::getInstance()->isExaminator() ||
             GS::LPPlugin::ProductInfo::getInstance()->isOEM())
        pGexScriptEngine->globalObject().setProperty( PROP_GEXPRODUCT, PRODUCT_GEX, QScriptValue::ReadOnly );

    pGexScriptEngine->globalObject().setProperty( PROP_PRODUCT_ID, QString());
    pGexScriptEngine->globalObject().setProperty( PROP_LOT_ID, QString());
    pGexScriptEngine->globalObject().setProperty( PROP_SUBLOT_ID, QString());

    pGexScriptEngine->globalObject().setProperty( PROP_OPTIONAL_MODULES,
                                                  GS::LPPlugin::ProductInfo::getInstance()->getOptionalModules(),
                                                  QScriptValue::ReadOnly);

    // TODO : Look at QScriptValue::Undeletable | QScriptValue::PropertyGetter | QScriptValue::PropertySetter
    // QScriptEngine::AutoCreateDynamicProperties

    QScriptValue lQFileSV =  pGexScriptEngine->scriptValueFromQMetaObject<QFile>();
    if (!lQFileSV.isNull())
        pGexScriptEngine->globalObject().setProperty("QFile", lQFileSV);
    else
        GSLOG(4, "Cannot register QFile meta class");

    QScriptValue lScriptEngineSV = pGexScriptEngine->newQObject( pGexScriptEngine );
    if (!lScriptEngineSV.isNull())
    {
        pGexScriptEngine->globalObject().setProperty("GSScriptEngine", lScriptEngineSV);
    }


#ifndef GSDAEMON
    // QApplication ?
//    QScriptValue lQAppSV;
//    if (qApp) lQAppSV= pGexScriptEngine->newQObject(qApp);
//    if (! lQAppSV.isNull())
//    {
//        pGexScriptEngine->globalObject().setProperty("GSApplication", lQAppSV);
//    }

    // All GUI objects
    QScriptValue lMBSV =  pGexScriptEngine->scriptValueFromQMetaObject<QMessageBox>();
    if (!lMBSV.isNull())
        pGexScriptEngine->globalObject().setProperty("QMessageBox", lMBSV);
    else
        GSLOG(4, "Cannot register QMessageBox meta class");

    QScriptValue lTBSV =  pGexScriptEngine->scriptValueFromQMetaObject<QToolBar>();
    if (!lTBSV.isNull())
        pGexScriptEngine->globalObject().setProperty("QToolBar", lTBSV);
    else
        GSLOG(4, "Cannot register QToolBar meta class");

    QScriptValue lDialogSV =  pGexScriptEngine->scriptValueFromQMetaObject<QDialog>();
    if (!lDialogSV.isNull())
        pGexScriptEngine->globalObject().setProperty("QDialog", lDialogSV);
    else
        GSLOG(4, "Cannot register QDialog meta class");

    QScriptValue lLabelSV =  pGexScriptEngine->scriptValueFromQMetaObject<QLabel>();
    if (!lLabelSV.isNull())
        pGexScriptEngine->globalObject().setProperty("QLabel", lLabelSV);
    else
        GSLOG(4, "Cannot register QLabel meta class");

    QScriptValue lLineEditSV = pGexScriptEngine->scriptValueFromQMetaObject<QLineEdit>();
    if (!lLineEditSV.isNull())
        pGexScriptEngine->globalObject().setProperty("QLineEdit", lLineEditSV);
    else
        GSLOG(4, "Cannot register QLineEdit meta class");

    QScriptValue lVBoxLayoutSV = pGexScriptEngine->scriptValueFromQMetaObject<QVBoxLayout>();
    if (!lVBoxLayoutSV.isNull())
        pGexScriptEngine->globalObject().setProperty("QVBoxLayout", lVBoxLayoutSV);
    else
        GSLOG(4, "Cannot register QVBoxLayout meta class");

//    QScriptValue GMWobject = pGexScriptEngine->newQObject(pGexMainWindow);
//    if (!GMWobject.isNull())
//    {
//        pGexScriptEngine->globalObject().setProperty("GSMainWindow", GMWobject);
//    }

//    if(pGexMainWindow && pGexMainWindow->pWizardSchedulerGui
//            && !pGexScriptEngine->globalObject().property("GSSchedulerGui").isValid() )
//    {
//        // QScriptEngine::AutoCreateDynamicProperties ?
//        QScriptValue GMSobject = pGexScriptEngine->newQObject((QObject*)pGexMainWindow->pWizardSchedulerGui);
//        pGexScriptEngine->globalObject().setProperty("GSSchedulerGui", GMSobject);
//    }

//    if (pGexMainWindow && pGexMainWindow->pWizardAdminGui)
//    {
//        AdminGui* pAdmin = pGexMainWindow->pWizardAdminGui;
//        QScriptValue adminObject = pGexScriptEngine->newQObject((QObject*) pAdmin);
//        if (!adminObject.isNull())
//        {
//            pGexScriptEngine->globalObject().setProperty("GSAdminGui", adminObject);
//        }
//    }

    // for GTM only
    /*
    QScriptValue lTesterSV = pGexScriptEngine->scriptValueFromQMetaObject<Gtm_TesterWindow>();
    int id=qScriptRegisterQObjectMetaType<Gtm_TesterWindow*>( pGexScriptEngine );
    GSLOG(6, QString("Gtm_TesterWindow registered with id %1").arg(id) );
    pGexScriptEngine->globalObject().setProperty("GSTester", lTesterSV );
    */

#endif

    QScriptValue GSTobject = pGexScriptEngine->newQObject((QObject*) &GS::Gex::CSLEngine::GetInstance());
    if (!GSTobject.isNull())
        pGexScriptEngine->globalObject().setProperty("GSCSLEngine", GSTobject);

    static CGexSystemUtils lSystemUtils;
    QScriptValue SUObject = pGexScriptEngine->newQObject((QObject*)&lSystemUtils ); // QScriptEngine::AutoCreateDynamicProperties ?
    if (!SUObject.isNull())
        pGexScriptEngine->globalObject().setProperty("GSSystemUtils", SUObject );

    static CSTDFtoATDF lSTDFtoATDF;
    QScriptValue lSTDFtoATDFSV = pGexScriptEngine->newQObject((QObject*)&lSTDFtoATDF ); // QScriptEngine::AutoCreateDynamicProperties ?
    if (!lSTDFtoATDFSV.isNull())
        pGexScriptEngine->globalObject().setProperty("GSSTDFtoATDF", lSTDFtoATDFSV );


    static CSTDFtoASCII lSTDFtoASCII(
                GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_EVALUATION);
    QScriptValue lSTDFtoASCIISV = pGexScriptEngine->newQObject((QObject*)&lSTDFtoASCII); // QScriptEngine::AutoCreateDynamicProperties ?
    if (!lSTDFtoASCIISV.isNull())
        pGexScriptEngine->globalObject().setProperty("GSSTDFtoASCII", lSTDFtoASCIISV );

    static GexTbMergeRetest lFilesMerger(false);
    QScriptValue lFilesMergerSV= pGexScriptEngine->newQObject((QObject*)&lFilesMerger ); // QScriptEngine::AutoCreateDynamicProperties ?
    if (!lFilesMergerSV.isNull())
        pGexScriptEngine->globalObject().setProperty("GSFilesMerger", lFilesMergerSV);

    static File f;
    QScriptValue FileObject = pGexScriptEngine->newQObject((QObject*)&f ); // QScriptEngine::AutoCreateDynamicProperties ?
    if (!FileObject.isNull())
        pGexScriptEngine->globalObject().setProperty("GSFile", FileObject );

    QScriptValue FTSimulatorObject = pGexScriptEngine->newQObject((QObject*) new GS::Gex::PATGtsStation(pGexScriptEngine)); // QScriptEngine::AutoCreateDynamicProperties ?
    if (!FTSimulatorObject.isNull())
        pGexScriptEngine->globalObject().setProperty("GSFTSimulator", FTSimulatorObject );
#ifdef GCORE15334
    QScriptValue TesterServerObject = pGexScriptEngine->newQObject((QObject*)&GS::Gex::Engine::GetInstance().GetTesterServer() ); // QScriptEngine::AutoCreateDynamicProperties ?
    if (!TesterServerObject.isNull())
        pGexScriptEngine->globalObject().setProperty("GSTesterServer", TesterServerObject );
#endif
    /*
        QDomDocument lDomDoc("ScriptEngineDomDoc");
        QScriptValue lDDSV = pGexScriptEngine->newQObject((QObject*)&f ); // QScriptEngine::AutoCreateDynamicProperties ?
        if (!FileObject.isNull())
            pGexScriptEngine->globalObject().setProperty("GexFile", FileObject );
    */

    if (pReportOptions)
    {
        QScriptValue ROobject = pGexScriptEngine->newQObject((QObject*)pReportOptions);
        if (!ROobject.isNull())
            pGexScriptEngine->globalObject().setProperty("GSReportOptions", ROobject);
    }

    QScriptValue lPATEngineSV = pGexScriptEngine->newQObject( (QObject*) &GS::Gex::PATEngine::GetInstance());
    if (!lPATEngineSV.isNull() )
        pGexScriptEngine->globalObject().setProperty("GSPATEngine", lPATEngineSV);

    QScriptValue lSchedulerEngineSV = pGexScriptEngine->newQObject( (QObject*) &GS::Gex::Engine::GetInstance().GetSchedulerEngine() );
    if (! lSchedulerEngineSV.isNull() )
        pGexScriptEngine->globalObject().setProperty("GSSchedulerEngine", lSchedulerEngineSV );

    if (GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated() )
    {
        QScriptValue lSV = pGexScriptEngine->newQObject((QObject*)&GS::Gex::Engine::GetInstance().GetAdminEngine() ); // QScriptEngine::AutoCreateDynamicProperties ?
        pGexScriptEngine->globalObject().setProperty("GSAdminEngine", lSV);
    }

    if (GS::Gex::Engine::GetInstance().GetDatabaseEngine().isActivated())
    {
        QScriptValue desv = pGexScriptEngine->newQObject((QObject*) &GS::Gex::Engine::GetInstance().GetDatabaseEngine() );
        if (! desv.isNull())
        {
            pGexScriptEngine->globalObject().setProperty("GSDatabaseEngine", desv);
        }
    }

    // now GS::Gex::Engine
    QScriptValue sv = pGexScriptEngine->newQObject((QObject*) &GS::Gex::Engine::GetInstance() );
    if (! sv.isNull())
    {
        pGexScriptEngine->globalObject().setProperty("GSEngine", sv);
    }

    // Let s refresh scripts/include
    // todo : loop on all files in this folder
    QDir lIncludeFolder(GS::Gex::Engine::GetInstance().Get("ApplicationDir").
                        toString() + "/scripts/include");
    QStringList lJSFiles = lIncludeFolder.entryList(QDir::nameFiltersFromString("*.js"), QDir::Files);
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("%1 js files found to be included").
          arg(lJSFiles.count()).toLatin1().data());
    QStringList::const_iterator lJSFilesIter;
    for (lJSFilesIter = lJSFiles.begin();
         lJSFilesIter != lJSFiles.end(); ++lJSFilesIter)
    {
        QString lFileName = *lJSFilesIter;
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Evaluating %1...").arg(lFileName).toLatin1().data());
        QFile lFile(GS::Gex::Engine::GetInstance().Get("ApplicationDir").
                    toString() + "/scripts/include/" + lFileName);
        if (lFile.open(QIODevice::ReadOnly))
        {
            QScriptValue lSV=pGexScriptEngine->evaluate(lFile.readAll());
            if (pGexScriptEngine->hasUncaughtException())
                GSLOG(SYSLOG_SEV_WARNING, QString("Exception in %1: %2 line %3")
                      .arg(lFileName).arg(pGexScriptEngine->uncaughtException().toString())
                      .arg(pGexScriptEngine->uncaughtExceptionLineNumber())
                      .toLatin1().data() );
            lFile.close();
        }
        else
            GSLOG(SYSLOG_SEV_WARNING,QString("Cannot open %1").arg(lFileName).toLatin1().data() );
    }

    return true;
}

#ifdef _WIN32
bool IsLocalSystem()
{
    HANDLE hToken;

    // open process token
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY,	&hToken))
        return false;

    UCHAR		bTokenUser[sizeof(TOKEN_USER) + 8 + 4 * SID_MAX_SUB_AUTHORITIES];
    PTOKEN_USER pTokenUser = (PTOKEN_USER)bTokenUser;
    ULONG		cbTokenUser;

    // retrieve user SID
    if (!GetTokenInformation(hToken, TokenUser, pTokenUser,	sizeof(bTokenUser), &cbTokenUser))
    {
        CloseHandle(hToken);
        return false;
    }

    CloseHandle(hToken);

    SID_IDENTIFIER_AUTHORITY siaNT = {SECURITY_NT_AUTHORITY};
    PSID pSystemSid;

    // allocate LocalSystem well-known SID
    if (!AllocateAndInitializeSid(&siaNT, 1, SECURITY_LOCAL_SYSTEM_RID,0, 0, 0, 0, 0, 0, 0, &pSystemSid))
        return false;

    // compare the user SID from the token with the LocalSystem SID
    bool bSystem = EqualSid(pTokenUser->User.Sid, pSystemSid);

    FreeSid(pSystemSid);

    return bSystem;
}
#endif // __WIN32

bool WriteSysInfo()
{
    // Let's find this machine config
    QString p(QDir::homePath()+QDir::separator()+ GEX_DEFAULT_DIR +QDir::separator());
    QDir::cleanPath(p);
    QDir d;
    if (!d.mkpath(p))
        return false;

    QFile f(p+"sysinfo.txt");
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&f);

    out<<"WordSize="<<QSysInfo::WordSize<<"bit\n";

#ifdef Q_OS_WIN
    if (qApp)
        out<<"SessionID = " << qApp->sessionId() <<"\n"; // Session ID not available on unix platforms (Linux/Solaris)
    out << "Windows version = "<< QSysInfo::windowsVersion();
    QString osname="unknown";
    switch (QSysInfo::windowsVersion())
    {	case QSysInfo::WV_NT : osname="WV_NT"; break;
        case QSysInfo::WV_2000 : osname="WV_2000"; break;
        case QSysInfo::WV_XP : osname="WV_XP"; break;
        case QSysInfo::WV_2003 : osname="WV_2003"; break;
        case QSysInfo::WV_VISTA : osname="WV_VISTA"; break;
        case QSysInfo::WV_WINDOWS7 : osname="WV_WINDOWS7"; break;
        case QSysInfo::WV_WINDOWS8 : osname="WV_WINDOWS8"; break;
        default: osname="Not NT based windows (Win98, ME,...) !"; break;
    }
    out << " = " << osname<<"\n";
#else
    out << "Linux, Solaris or Mac platform detected.";
#endif

    out<<"\n";

    out << "GEX built with Qt " << QT_VERSION_STR <<"\n";
#ifdef __GNUC__
    out << QString("GEX built with GCC version %1.%2.%3")
           .arg(__GNUC__).arg(__GNUC_MINOR__).arg(__GNUC_PATCHLEVEL__) << "\n";
#else
    out << QString("GEX NOT built with GCC !") << "\n";
#endif

    QString lProfileFolder = GS::LPPlugin::ProductInfo::getInstance()->GetProfileFolder();
    out << GS::LPPlugin::ProductInfo::getInstance()->GetProfileVariable();

    if (lProfileFolder.isNull() || lProfileFolder.isEmpty())
        out << "?" <<  "\n";
    else
        out << lProfileFolder <<  "\n";

    char *gp= getenv("GEX_PATH");
    out << "GEX_PATH = " <<  QString(gp?gp:"?") <<"\n";
    out << "App version = " << QCoreApplication::applicationVersion()<<"\n";
    out << "App Dir Path = " << QCoreApplication::applicationDirPath()<<"\n";
    out << "App name = " << QCoreApplication::applicationName()<<"\n";
    out << "App Pid = " << QCoreApplication::applicationPid()<<"\n";
    out << "Organization Domain = "<< QCoreApplication::organizationDomain()<<"\n";
    out << "Organization Name = "<< QCoreApplication::organizationName()<<"\n";
    out<<"\n";
    out << "PROCESSOR_ARCHITECTURE = " << QString(getenv("PROCESSOR_ARCHITECTURE")) << "\n" ;
    char* nop=getenv("NUMBER_OF_PROCESSORS");
    out << "NUMBER_OF_PROCESSORS = " << (nop?nop:"?") <<"\n";

    /*
    out << "Drives :\n";
    QFileInfoList fil=QDir::drives();
    QFileInfo fi;
    foreach(fi, fil)
    {
        double t=0, f=0;
        getFreeTotalSpace(fi.absolutePath(), t, f);
        // QString::number(fi.size())
        out<<"\t"<<fi.fileName()<<" "<< fi.absolutePath()
                <<" Total="<< QString::number(t/(1024.*1024.f),'f',0)
                << "Mo Free=" << QString::number(f/(1024*1024.f),'f') <<"\n";
    }
    out<<"\n";
    */

    char* c=getenv("SESSIONNAME");
    out << "SESSIONNAME = " << (c?QString(c):QString("?")); out << QString("\n");

    out << "USER = " << QString(getenv("USER")) << "\n" ;	// Linux
    out << "USERNAME = " << QString(getenv("USERNAME")) << "\n" ; // Windows

    out << "DESKTOP_SESSION = " << QString(getenv("DESKTOP_SESSION")) <<"\n";
    out << "LANG = " << QString(getenv("LANG")) <<"\n";
    out << "HOME = " << QString(getenv("HOME")) <<"\n";
    out << "DISPLAY = " << QString(getenv("DISPLAY")) <<"\n";

    //PATH

    out << "Local Domain Name = " << QHostInfo::localDomainName()<<"\n";
    out << "Local Host Name = " << QHostInfo::localHostName()<<"\n";

    out << "Machine IPs:\n";
    QList<QHostAddress> hosts = QNetworkInterface::allAddresses();
    for (int i = 0; i < hosts.count(); ++i)
        if (hosts[i].protocol()==0) // only IPv4 ATM...
            // To Do : hosts[i].protocol() ???
            out<<"\t"<<i<<": "<<hosts[i].toString()<< " "
              << "\n"; //  somewhere output or process hosts[i]. of course 127.0.0.1 can be ignored.

    out << "Hardware addresses:\n";
    foreach (const QNetworkInterface &ni, QNetworkInterface::allInterfaces())
    {
        QString strHardwareAdress = ni.hardwareAddress();
        if (!strHardwareAdress.isEmpty())
        {
            out << "\t"<< ni.humanReadableName() << ": " << ni.hardwareAddress();
            QNetworkInterface::InterfaceFlags niFlags = ni.flags();
            if (niFlags & QNetworkInterface::IsUp)
                out << " | Up ";
            if (niFlags & QNetworkInterface::IsRunning)
                out << " | Running ";
            if (niFlags & QNetworkInterface::CanBroadcast)
                out << " | CanBroadcast ";
            if (niFlags & QNetworkInterface::IsLoopBack)
                out << " | LoopBack ";
            if (niFlags & QNetworkInterface::IsPointToPoint)
                out << " | PointToPoint ";
            if (niFlags & QNetworkInterface::CanMulticast)
                out << " | CanMulticast ";
            out << "\n";
        }
    }

    f.close();

    return true;
}

QVariantMap lmAppConfigData()
{
    QVariantMap appData;
    QString appDir =  GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
    CGexSystemUtils::NormalizePath(appDir);

    appData.insert("ApplicationDir",appDir);
    GSLOG(7, QString("UserFolder passed to LP (%1);").arg(GS::Gex::Engine::GetInstance().Get("UserFolder").toString()).toLatin1().constData());
    appData.insert("UserFolder", GS::Gex::Engine::GetInstance().Get("UserFolder").toString());
    appData.insert("GalaxySemiFolder", GS::Gex::Engine::GetInstance().Get("GalaxySemiFolder").toString());

    appData.insert("AppVersion", QString("%1.%2")
                   .arg(GS::Gex::Engine::GetInstance().Get("AppVersionMajor").toString())
                   .arg(GS::Gex::Engine::GetInstance().Get("AppVersionMinor").toString()));
    appData.insert("AppFullName", GS::Gex::Engine::GetInstance().Get("AppFullName").toString());

    appData.insert("WelcomeMode",  GS::Gex::Engine::GetInstance().GetCommandLineOptions().IsWelcomeBoxEnabled());
    appData.insert("HiddenMode",  GS::Gex::Engine::GetInstance().GetCommandLineOptions().IsHidden());

    appData.insert("ClientApplicationId", GS::Gex::Engine::GetInstance().Get("ClientApplicationId").toString());
    appData.insert("IniClientSection", strIniClientSection);

    appData["RunScript"] = GS::Gex::Engine::GetInstance().GetCommandLineOptions().GetRunScript();

//    appData["ReleaseDate"] =  GS::Gex::Engine::GetInstance().GetReleaseDate();
    appData["ExpirationDate"] = GS::Gex::Engine::GetInstance().GetExpirationDate();
    appData["MaintenanceExpirationDate"] =  GS::Gex::Engine::GetInstance().GetMaintenanceExpirationDate();

    appData["TimeBeforeAutoClose"] = ReportOptions.GetTimeBeforeAutoClose();


#ifdef GSDAEMON
    appData["isDAEMON"] = true;
    appDir =  GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
    CGexSystemUtils::NormalizePath(appDir);

    appData.insert("ApplicationDir",appDir);
    GSLOG(7, QString("UserFolder passed to LP (%1);").arg(GS::Gex::Engine::GetInstance().Get("UserFolder").toString()).toLatin1().constData());
    appData.insert("UserFolder", GS::Gex::Engine::GetInstance().Get("UserFolder").toString());
    appData.insert("GalaxySemiFolder", GS::Gex::Engine::GetInstance().Get("GalaxySemiFolder").toString());

#else
    appData["isDAEMON"] = false;
#endif

    int idxLP = GS::Gex::Engine::GetInstance().GetCommandLineOptions().GetArguments().indexOf(QRegExp("-uselp=.*"));
    GSLOG(SYSLOG_SEV_DEBUG, QString("lmAppConfigData => idxLP : (%1)").arg(idxLP).toLatin1().constData());

#ifdef TER_GEX
    // If Teradyne OEM package, force using ter_lp license provider
    appData.insert("UseLP",QString("ter_lp"));
#else
    // Determine "UseLP" key from the command line arguments
    if(idxLP != -1)
    {
        appData.insert("UseLP",QString(GS::Gex::Engine::GetInstance().GetCommandLineOptions().GetArguments()[idxLP]).section("=",1,1));
    }
    else
    {
        appData.insert("UseLP",QString("fnp_lp"));
    }
#endif


    QString lpUsed = "NA";
    if(appData.contains("UseLP"))
        lpUsed = appData["UseLP"].toString();

    GSLOG(SYSLOG_SEV_DEBUG, QString("lmAppConfigData => lpUsed : (%1)").arg(lpUsed).toLatin1().constData());

    return appData;
}

bool initializeLicenseManager()
{
#ifndef GCORE15334
    GS::LPPlugin::LicenseProvider::GexProducts lRequestedProd = GS::Gex::Engine::GetInstance().GetCommandLineOptions().GetProduct();
    if (lRequestedProd == GS::LPPlugin::LicenseProvider::eExaminatorPAT ||
            lRequestedProd == GS::LPPlugin::LicenseProvider::ePATMan ||
            lRequestedProd == GS::LPPlugin::LicenseProvider::eGTM ||
            lRequestedProd == GS::LPPlugin::LicenseProvider::ePATManEnterprise)
    {
        QString lMsg = "The current installed package does not support this functionality.\n"
                        "Please contact your Quantix representative for further details.";
        GS::Gex::Message::critical("", lMsg);
        GSLOG(SYSLOG_SEV_CRITICAL, lMsg.toLatin1().constData());
        // Exit!
        exit(EXIT_FAILURE);
    }
#endif

    GSLOG(SYSLOG_SEV_DEBUG, "initializing License Provider");

    int errorCode = GS::LPPlugin::LicenseProviderManager::eNoError;
    QString errorMessage = "";
    GS::LPPlugin::LicenseProviderManager *lpManager = GS::LPPlugin::LicenseProviderManager::instanciate(
        GS::Gex::Engine::GetInstance().GetCommandLineOptions().GetProduct(),
        GS::LPPlugin::ProductInfo::getInstance(),
        lmAppConfigData(),
        errorCode,
        errorMessage,
        QCoreApplication::instance());
    if(!lpManager || errorCode !=  GS::LPPlugin::LicenseProviderManager::eNoError)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Error (%1) : %2").arg(errorCode).arg(errorMessage).toLatin1().constData());
        GS::Gex::Message::critical(GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
                                   QString("Error (%1) : %2").arg(errorCode).arg(errorMessage).toLatin1().constData());
        return false;
    }

    bool ret = false;
    //Communication with LP
    ret = QObject::connect(lpManager, SIGNAL(forwardLPMessage(const GS::LPPlugin::LPMessage &)),
                           &GS::Gex::Engine::GetInstance(), SLOT(processLPMessage(const GS::LPPlugin::LPMessage &)));

    ret &= (bool)QObject::connect(&GS::Gex::Engine::GetInstance(), SIGNAL(sendGEXMessage(const GS::LPPlugin::GEXMessage &)),
                            lpManager, SLOT (receiveGexMessage(const GS::LPPlugin::GEXMessage &)));

    if(!ret)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Error when initializing license provider").toLatin1().constData());
        GS::Gex::Message::critical(GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
                                   QString("Error when initializing license provider").toLatin1().constData());
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////
// main function
///////////////////////////////////////////////////////////
int MainInit()
{
    GSLOG(SYSLOG_SEV_NOTICE, "Entering MainInit..." );

    GSLOG(SYSLOG_SEV_DEBUG, QString("Current allocated mem: %1").arg(sCurrentAllocated).toLatin1().constData());

    // Update Product name following the license settings.
    GS::LPPlugin::ProductInfo::getInstance()->setProductID(GS::Gex::Engine::GetInstance().GetCommandLineOptions().GetProduct());

    GSLOG(SYSLOG_SEV_NOTICE,
          QString("According to CLI args : Product=%1")
          .arg(GS::LPPlugin::ProductInfo::getInstance()->getProductID()).toLatin1().constData());

    QString lRes;

    // Check if any profile variable is defined, either for server or client application:
    // check if env set to overwrite user profile directory
    lRes = GS::Gex::Engine::GetInstance().OverloadUserFolder();
    if (lRes.startsWith("error"))
    {
        GSLOG(SYSLOG_SEV_ERROR,
              QString("Failed to set the overload user folder: %1").arg(lRes).toLatin1().data() );

        return EXIT_FAILURE;
    }

    lRes = GS::Gex::Engine::GetInstance().UpdateProductStringName();
    if (lRes!="ok")
    {
        GSLOG(SYSLOG_SEV_ERROR,
              QString("Failed to retrieve ProductName: %1").arg(lRes).toLatin1().data() );
    }

    QCoreApplication::setApplicationVersion(GEX_APP_VERSION);

    // If in debug mode, write startup message (don't write any debug message before,
    // as the debug file is created in the user's home folder)
    QString strMessage = "Starting ";
    strMessage += GS::Gex::Engine::GetInstance().Get("AppFullName").toString();
    strMessage += " (";
    strMessage += QDateTime::currentDateTime().toString("dd MMM yyyy hh:mm:ss");
    strMessage += ")...";

    GSLOG(5, strMessage.toLatin1().data() );

    strMessage = "Compilation date: ";
    QString strDateTime;
    strDateTime = __DATE__;
    strDateTime += " ";
    strDateTime += __TIME__;
    strMessage += strDateTime;
    GSLOG(5, strMessage.toLatin1().data() );

    GSLOG(5, QString("using QT version %1").arg(QT_VERSION_STR).toLatin1().data() );
#ifdef __GNUC__
    GSLOG(5, QString("using exec built with GCC version %1.%2.%3")
          .arg(__GNUC__).arg(__GNUC_MINOR__).arg(__GNUC_PATCHLEVEL__)
          .toLatin1().data()
          );
#else
    GSLOG(5, QString("not using exec built with GCC").toLatin1().data() );
#endif

    if (!ReportOptions.CreateDefaultRnRRulesFile())
        GSLOG(4, "cant write default RnR warning file !");

    WriteSysInfo();

    // Create script engine once the QApp has been created
    pGexScriptEngine = new GexScriptEngine(qApp);
    if (!pGexScriptEngine)
    {
        GSLOG(3, "cant create the Gex ScriptEngine !");
        return EXIT_FAILURE;
    }
    pGexScriptEngine->setObjectName("GSScriptEngine");
    //qApp->setProperty("ScriptEngine", QVariant(pGexScriptEngine));

    //GS::Gex::Engine::GetInstance()

    //QScriptValue lPatProcessingRefSV = pGexScriptEngine->scriptValueFromQMetaObject<GS::Gex::PATProcessing&>();


    QScriptValue lSTDFConverterSV = pGexScriptEngine->scriptValueFromQMetaObject<GS::Gex::ConvertToSTDF>();
    if (!lSTDFConverterSV.isNull())
    {
        //qScriptRegisterMetaType(pGexScriptEngine, );
        int id=qScriptRegisterQObjectMetaType<GS::Gex::ConvertToSTDF*>( pGexScriptEngine );
        GSLOG(7, QString("Registring CGConvertToSTDF : %1").arg(id).toLatin1().constData() );
        //qScriptRegisterQObjectMetaType<GS::Gex::PATProcessing>( pGexScriptEngine );
        //qScriptRegisterQObjectMetaType<GS::Gex::PATProcessing&>( pGexScriptEngine );
        pGexScriptEngine->globalObject().setProperty("GSSTDFConverter", lSTDFConverterSV );
    }
    else
        GSLOG(4, "Cannot register CGConvertToSTDF meta class");

    QScriptValue lPatProcessingSV = pGexScriptEngine->scriptValueFromQMetaObject<GS::Gex::PATProcessing>();
    if (!lPatProcessingSV.isNull())
    {
        //qScriptRegisterMetaType(pGexScriptEngine, );
        int id=qScriptRegisterQObjectMetaType<GS::Gex::PATProcessing*>( pGexScriptEngine );
        GSLOG(7, QString("Registring GS::Gex::PATProcessing : %1").arg(id).toLatin1().constData() );
        //qScriptRegisterQObjectMetaType<GS::Gex::PATProcessing>( pGexScriptEngine );
        //qScriptRegisterQObjectMetaType<GS::Gex::PATProcessing&>( pGexScriptEngine );
        pGexScriptEngine->globalObject().setProperty("GSPatProcessing", lPatProcessingSV );
    }
    else
        GSLOG(4, "Cannot register GS::Gex::PATProcessing meta class");
    QScriptValue lMapMergeApi = pGexScriptEngine->scriptValueFromQMetaObject<GS::Gex::MapMergeApi>();
    if (!lMapMergeApi.isNull())
    {
        //qScriptRegisterMetaType(pGexScriptEngine, );
        int id=qScriptRegisterQObjectMetaType<GS::Gex::MapMergeApi*>( pGexScriptEngine );
        GSLOG(7, QString("Registring GS::Gex::MergeMapApi : %1").arg(id).toLatin1().constData() );
        pGexScriptEngine->globalObject().setProperty("GSMergeMap", lMapMergeApi );
    }
    else
        GSLOG(4, "Cannot register GS::Gex::MapMergeApi meta class");

    QScriptValue lCSLStatusSV = pGexScriptEngine->scriptValueFromQMetaObject<GS::Gex::CSLStatus>();
    if (!lCSLStatusSV.isNull())
    {
        int id=qScriptRegisterQObjectMetaType<GS::Gex::CSLStatus*>( pGexScriptEngine );
        GSLOG(7, QString("Registring GS::Gex::CSLStatus : %1").arg(id).toLatin1().constData() );
        pGexScriptEngine->globalObject().setProperty("GSCSLStatus", lCSLStatusSV );
    }
    else
        GSLOG(4, "Cannot register GS::Gex::CSLStatus meta class");

    QScriptValue lKCSV = pGexScriptEngine->scriptValueFromQMetaObject<GS::QtLib::DatakeysContent>();
    if (!lKCSV.isNull())
    {
        //qScriptRegisterMetaType(pGexScriptEngine, );
        int id=qScriptRegisterQObjectMetaType<GS::QtLib::DatakeysContent*>( pGexScriptEngine );
        GSLOG(7, QString("Registring GS::QtLib::DatakeysContent : %1").arg(id).toLatin1().constData() );
        pGexScriptEngine->globalObject().setProperty("GSKeysContent", lKCSV );
    }
    else
        GSLOG(4, "Cannot register GS::Gex::GS::QtLib::DatakeysContent meta class");

    QScriptValue lTaskItemSV = pGexScriptEngine->scriptValueFromQMetaObject<CGexMoTaskItem>();
    if (!lTaskItemSV.isNull())
    {
        int id=qScriptRegisterQObjectMetaType<CGexMoTaskItem*>( pGexScriptEngine );
        GSLOG(7, QString("Registring CGexMoTaskItem : %1").arg(id).toLatin1().constData() );
        pGexScriptEngine->globalObject().setProperty("GSTaskItem", lTaskItemSV );
    }
    else
        GSLOG(4, "Cannot register CGexMoTaskItem meta class");

    QScriptValue lWaferMapSV = pGexScriptEngine->scriptValueFromQMetaObject<CWaferMap>();
    if (!lWaferMapSV.isNull())
    {
        int id=qScriptRegisterQObjectMetaType<CWaferMap*>( pGexScriptEngine );
        GSLOG(7, QString("Registring WaferMap : %1").arg(id).toLatin1().constData() );
        pGexScriptEngine->globalObject().setProperty("GSWaferMap", lWaferMapSV );
    }
    else
        GSLOG(4, "Cannot register CGexMoTaskItem meta class");

    QScriptValue lBinDescriptionSV = pGexScriptEngine->scriptValueFromQMetaObject<GS::Gex::BinDescription>();
    if (!lBinDescriptionSV.isNull())
    {
        int id=qScriptRegisterQObjectMetaType<GS::Gex::BinDescription*>( pGexScriptEngine );
        GSLOG(7, QString("Registring BinDescription : %1").arg(id).toLatin1().constData() );
        pGexScriptEngine->globalObject().setProperty("GSBinInfo", lBinDescriptionSV );
    }
    else
        GSLOG(4, "Cannot register GS::Gex::BinDescription meta class");


    //qScriptRegisterMetaType(pGexScriptEngine, )

    //pGexScriptEngine->newArray();

    // For the moment let's desactivate defaultly the debugger
    //GS::Gex::Engine::GetInstance().GetScriptEngineDebbugger().attachTo(pGexScriptEngine);

#ifdef GSDAEMON
    pGexScriptEngine->setProperty("GS_DAEMON", true);
#else
    pGexScriptEngine->setProperty("GS_DAEMON", false);
#endif

    // Update Script Engine properties.
    UpdateGexScriptEngine(NULL);

    if(!initializeLicenseManager())
    {
        return EXIT_FAILURE;
    }

    QString lDefinitionLoaderMessage;
    if(!GS::QtLib::DataKeysDefinitionLoader::GetInstance().LoadingPass(lDefinitionLoaderMessage))
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL,
        QString("Error when loading Definition Data Keys %1").arg(lDefinitionLoaderMessage).toLatin1().constData());
        return EXIT_FAILURE;
    }

    GSLOG(SYSLOG_SEV_INFORMATIONAL, "quitting MainInit function" );

    GSLOG(7, QString("Current allocated mem: %1").arg(sCurrentAllocated).toLatin1().constData() );

    return 0;
}

///////////////////////////////////////////////////////////
// Start GEX as application
int StartAsApplication(int argc, char **argv)
{
    GSLOG(SYSLOG_SEV_NOTICE, QString("Starting as Application...").toLatin1().data());

    // This create a Singleton and must be deleted at the very end of the main(...) function.
    // Pointer on QApplication can be retrieved using either the QT macrio qApp or QCoreApplication::instance()
    new QApplication(argc, argv);

    //QObject::connect(qApplication, SIGNAL(aboutToQuit()), &GS::Gex::Engine::GetInstance(), SLOT(FreeLicenseProvider()));

    // call main init function
    int nStatus = MainInit();

    if (nStatus == 0)
    {
        QImage	img;
        QString strIconName, strIconPath;
        QString strIconFolder(":/gex/icons/");

        // Pixmap images used in different places in the project
        GSLOG(SYSLOG_SEV_DEBUG, "Pixmaps creation...");

        // PropertyBrowser icons update :
//        strIconName         = QString("options_gexapplication.png");
        strIconName         = QString("gex_application_v2.png");
        strIconPath         = strIconFolder + strIconName;
        pixGexApplication   = new QPixmap(strIconPath);

        strIconName = QString("options_output.png");
        strIconPath = strIconFolder + strIconName;
        pixOutput   = new QPixmap(strIconPath);

        strIconName = QString("options_tasks.png");
        strIconPath = strIconFolder + strIconName;
        pixTasks    = new QPixmap(strIconPath);

        strIconName = QString("options_teststatistics.png");
        strIconPath = strIconFolder + strIconName;
        pixTestStatistics = new QPixmap(strIconPath);

        strIconName = QString("options_wafermap.png");
        strIconPath = strIconFolder + strIconName;
        pixWafermap = new QPixmap(strIconPath);

        strIconName = QString("options_advhisto.png");
        strIconPath = strIconFolder + strIconName;
        pixAdvHisto = new QPixmap(strIconPath);

        strIconName = QString("options_trend.png");
        strIconPath = strIconFolder + strIconName;
        pixTrend    = new QPixmap(strIconPath);

        strIconName = QString("shift.png");
        strIconPath = strIconFolder + strIconName;
        pixShift    = new QPixmap(strIconPath);

        strIconName = QString("options_scatter.png");
        strIconPath = strIconFolder + strIconName;
        pixScatter  = new QPixmap(strIconPath);

        strIconName = QString("options_boxmeanrange.png");
        strIconPath = strIconFolder + strIconName;
        pixBoxMeanRange = new QPixmap(strIconPath);

        strIconName = QString("options_pareto.png");
        strIconPath = strIconFolder + strIconName;
        pixPareto   = new QPixmap(strIconPath);

        strIconName = QString("options_binning.png");
        strIconPath = strIconFolder + strIconName;
        pixBinning  = new QPixmap(strIconPath);
        strIconName = QString("options_datalog.png");
        strIconPath = strIconFolder + strIconName;
        pixDatalog  = new QPixmap(strIconPath);
        strIconName = QString("options_guardbanding.png");
        strIconPath = strIconFolder + strIconName;
        pixGuardBanding = new QPixmap(strIconPath);
        strIconName = QString("options_stopwatch.png");
        strIconPath = strIconFolder + strIconName;
        pixStopwatch= new QPixmap(strIconPath);

        strIconName = QString("digital_icon.png");
        strIconPath = strIconFolder + strIconName;
        pixAdvFunctional= new QPixmap(strIconPath);

        strIconName = QString("ftr_fail_table.png");
        strIconPath = strIconFolder + strIconName;
        pixAdvFTRCorr= new QPixmap(strIconPath);

        pixChildTree    = new QPixmap(( const char** ) pixmap_ChildTree);
        pixHisto        = new QPixmap(( const char** ) pixmap_Histo);
        pixBoxPlot      = new QPixmap(QString::fromUtf8(":/gex/icons/options_candlemeanrange.png"));
        pixMultiCharts  = new QPixmap(QString::fromUtf8(":/gex/icons/options_advmultichart.png"));
        pixOutlier      = new QPixmap(( const char** ) pixmap_Outlier);
        pixPrinter      = new QPixmap(( const char** ) image_Printer);
        pixGetStarted   = new QPixmap(( const char** ) image_Help);
        pixSeparator    = new QPixmap(( const char** ) image_Separator);
        pixDisabled     = new QPixmap(( const char** ) image_Disabled);
        pixNetwork      = new QPixmap(( const char** ) image_Network);
        pixStandalone   = new QPixmap(( const char** ) image_Standalone);
        pixMoDataPump   = new QPixmap(( const char** ) pixmap_MoDataPump);
        pixCalendar     = new QPixmap(( const char** ) pixmap_Calendar);
        pixSave         = new QPixmap(( const char** ) pixmap_Save);
        pixCopy         = new QPixmap(( const char** ) pixmap_Copy);
        pixGlobalInfo   = new QPixmap(QString::fromUtf8(":/gex/icons/options_global_info.png"));
        pixProbabilityPlot      = new QPixmap(QString::fromUtf8(":/gex/icons/options_probabilityplot.png"));
        pixMoYieldMonitoring    = new QPixmap(( const char** ) pixmap_MoYieldMonitoring);
        pixMoSpecMonitoring     = new QPixmap(( const char** ) pixmap_MoSpecMonitoring);
        pixCSVSpreadSheet= new QPixmap(QString::fromUtf8(":/gex/icons/csv_spreadsheet.png"));

        // Since QT V3.3, Pixmap images are saved as PNG hexa array instaed of 7bits ascii array
        img.loadFromData( pixmap_Word, sizeof( pixmap_Word), "PNG" );
        pixWord= new QPixmap(QPixmap::fromImage(img));

        img.loadFromData( pixmap_Pdf, sizeof( pixmap_Pdf), "PNG" );
        pixPdf= new QPixmap(QPixmap::fromImage(img));

        img.loadFromData( pixmap_Powerpoint, sizeof( pixmap_Powerpoint), "PNG" );
        pixPowerpoint= new QPixmap(QPixmap::fromImage(img));

        img.loadFromData( pixmap_Remove, sizeof( pixmap_Remove), "PNG" );
        pixRemove= new QPixmap(QPixmap::fromImage(img));

        img.loadFromData( pixmap_Open, sizeof( pixmap_Open), "PNG" );
        pixOpen= new QPixmap(QPixmap::fromImage(img));

        img.loadFromData( pixmap_CreateFolder, sizeof( pixmap_CreateFolder), "PNG" );
        pixCreateFolder= new QPixmap(QPixmap::fromImage(img));

        img.loadFromData( pixmap_Properties, sizeof( pixmap_Properties), "PNG" );
        pixProperties= new QPixmap(QPixmap::fromImage(img));

        img.loadFromData( pixmap_PatHistory, sizeof( pixmap_PatHistory), "PNG" );
        pixPatHistory= new QPixmap(QPixmap::fromImage(img));

        // Create GEX Explorer application, show it.
        GSLOG(SYSLOG_SEV_INFORMATIONAL, "Creating main window...");

        // Execute application
        try
        {
            pGexMainWindow = new GexMainwindow();
            GS::Gex::UserInputFilter lUIF(pGexMainWindow);
            if (qApp) qApp->installEventFilter(&lUIF);

            pGexMainWindow->show();

            QCoreApplication::postEvent(&GS::Gex::Engine::GetInstance(),
                            new QEvent(QEvent::Type(GS::Gex::Engine::EVENT_START)));

            nStatus = QApplication::exec();
            GSLOG(nStatus==0?5:3, QString("Application exec returned %1").arg(nStatus).toLatin1().data() );
        }
        catch (const std::bad_alloc &e)
        {
            printf("\nstd::bad_alloc caught\n");
            GSLOG(2, "std::bad_alloc exception caught");
            nStatus = EXIT_FAILURE;
        }

        GSLOG(7, QString("Current allocated mem: %1").arg(sCurrentAllocated).toLatin1().constData() );

        GSLOG(SYSLOG_SEV_INFORMATIONAL, "deleting pGexMainWindow...");

        delete pGexMainWindow;
        pGexMainWindow=0;

        delete pixGexApplication;
        delete pixOutput;
        delete pixTasks;
        delete pixTestStatistics;
        delete pixWafermap;
        delete pixAdvHisto;
        delete pixTrend;
        delete pixShift;
        delete pixScatter;
        delete pixBoxMeanRange;
        delete pixPareto;
    }
    else
        GSLOG(3, QString("MainInit failed : %1").arg(nStatus).toLatin1().constData() );

    return nStatus;
}

// Start as service
int StartAsService(int argc, char **argv)
{
    GSLOG(SYSLOG_SEV_NOTICE, QString("Starting As a Service").toLatin1().data());

    QString lServiceName;
    QString lServiceDescription;

    if (GS::Gex::Engine::GetInstance().GetCommandLineOptions().GetProduct() == GS::LPPlugin::LicenseProvider::eYieldMan)
    {
        lServiceName        = "QX-Yield-Man";
        lServiceDescription = "Quantix Yield-Man service";
    }
    else if (GS::Gex::Engine::GetInstance().GetCommandLineOptions().GetProduct() == GS::LPPlugin::LicenseProvider::eYieldManEnterprise)
    {
        lServiceName        = "QX-Yield-Man-Enterprise";
        lServiceDescription = "Quantix Yield-Man Enterprise service";
    }
    else if (GS::Gex::Engine::GetInstance().GetCommandLineOptions().GetProduct() == GS::LPPlugin::LicenseProvider::ePATMan)
    {
        lServiceName        = "QX-PAT-Man";
        lServiceDescription = "Quantix PAT-Man service";
    }
    else if (GS::Gex::Engine::GetInstance().GetCommandLineOptions().GetProduct() == GS::LPPlugin::LicenseProvider::ePATManEnterprise)
    {
        lServiceName        = "QX-PAT-Man-Enterprise";
        lServiceDescription = "Quantix PAT-Man Enterprise service";
    }
    else if (GS::Gex::Engine::GetInstance().GetCommandLineOptions().GetProduct() == GS::LPPlugin::LicenseProvider::eGTM)
    {
        lServiceName        = "QX-GTM";
        lServiceDescription = "Quantix GTM service";
    }
    else
    {
        GSLOG(3, QString("This product %1 cannot be run as a daemon")
              .arg(GS::Gex::Engine::GetInstance().GetCommandLineOptions().GetProduct()).toLatin1().constData() );
        return EXIT_FAILURE;
    }

    // Init service name and description
    GS::Gex::Daemon lDaemon(argc, argv, lServiceName, lServiceDescription);

    GSLOG(5, lDaemon.serviceDescription().toLatin1().constData());

    // Start the application using QtService
    int nStatus = lDaemon.exec();

    return nStatus;
}


///////////////////////////////////////////////////////////
//	Get the local folder for GEX. Could be :
//	- the user folder + GEX_DEFAULT_DIR
//	- the server profile dir (as defined by the Env Variable ) + GEX_DEFAULT_DIR
// If an error occurs when creating the folder, returns the user folder
///////////////////////////////////////////////////////////
QString workingFolder()
{
    QString strFolderAbsPath(GS::Gex::Engine::GetInstance().Get("UserFolder").toString()
                             + QDir::separator() + GEX_DEFAULT_DIR + QDir::separator());
    strFolderAbsPath = QDir::cleanPath(strFolderAbsPath);
    QDir dir;
    if (!dir.mkpath(strFolderAbsPath))
        return QDir::homePath();
    return strFolderAbsPath;
}

bool getFreeTotalSpace(const QString& sDirPath,double& fTotal, double& fFree)
{
#ifdef _WIN32
    QString sCurDir = QDir::current().absolutePath();
    QDir::setCurrent( sDirPath );
    ULARGE_INTEGER free,total;
    bool bRes = ::GetDiskFreeSpaceExA( 0 , &free , &total , NULL );
    if ( !bRes ) return false;
    QDir::setCurrent( sCurDir );
    fFree = static_cast<double>( static_cast<__int64>(free.QuadPart) );
    fTotal = static_cast<double>( static_cast<__int64>(total.QuadPart) );
#elif (defined __sun__) // Solaris
    struct stat stst;
    struct statvfs stfs;
    if (::stat(sDirPath.local8Bit().constData(), &stst) == -1)
    {
        return false;
    }
    if (::statvfs(sDirPath.local8Bit().constData(), &stfs) == -1)
    {
        return false;
    }
    fFree = stfs.f_bavail * ( stst.st_blksize );
    fTotal = stfs.f_blocks * ( stst.st_blksize );
#else //linux
    struct stat stst;
    struct statfs stfs;
    if (::stat(sDirPath.toLocal8Bit().constData(), &stst) == -1)
    {
        return false;
    }
    if (::statfs(sDirPath.toLocal8Bit().constData(), &stfs) == -1)
    {
        return false;
    }
    fFree = stfs.f_bavail * ( stst.st_blksize );
    fTotal = stfs.f_blocks * ( stst.st_blksize );
#endif // _WIN32
    return true;
}

#if defined(__linux__) || defined(_WIN32)
///////////////////////////////////////////////////////////
// Set control word FPU register
///////////////////////////////////////////////////////////
void set_fpu (unsigned int mode)
{
    asm ("fldcw %0" : : "m" (*&mode));
}
#endif // defined(__linux__) || defined(_WIN32)

///////////////////////////////////////////////////////////
// Keeps track of argc,argv given when launching the application
///////////////////////////////////////////////////////////
int main( int argc, char ** argv )
{
    // Decimal point
    setlocale(LC_NUMERIC, (char*) "POSIX");

#if defined(__linux__) || defined(_WIN32)
    /* FIX:
       Excess precision when using a 64-bit mantissa for FPU math ops can
       cause unexpected results with some of the MSVCRT math functions.  For
       example, unless the function return value is stored (truncating to
       53-bit mantissa), calls to pow with both x and y as integral values
       sometimes produce a non-integral result.*/
    //set_fpu (0x27F);// Round Control: round nearest -- Precision Controle: 53 bits
#endif // defined(__linux__) || defined(_WIN32)

#ifndef GSDAEMON
    QApplication::setColorSpec( QApplication::CustomColor );
#endif

#if defined __unix__ || __MACH__
    // Check if HOME and GEX_PATH defined...if not ABORT!
    if(NULL == getenv("HOME"))
    {
        GS::Gex::Message::critical(
            "",
            "ERROR: The environment variable HOME is not set...\n"
            "you must set it to the path of your HOME directory.\n"
            "GEX will now exit!");
        // Exit!
        exit(EXIT_FAILURE);
    }
    if(NULL == getenv("GEX_PATH"))
    {
        GS::Gex::Message::critical(
            "",
            "ERROR: The environment variable GEX_PATH is not set...\nyou must "
            "set it to the path of the Quantix Examinator application.\n"
            "GEX will now exit!");
        exit(EXIT_FAILURE);
    }
    // Under Unix, DO NOT show warning messages on TTY (only fatal errors) !
    qInstallMsgHandler( myMessageOutput );
#endif

    // Check Home Directory
    QString lHomeDir; // Now stored in Engine
    if (CGexSystemUtils::GetUserHomeDirectory(lHomeDir) != CGexSystemUtils::NoError)
    {
        GSLOG(SYSLOG_SEV_CRITICAL,
              "Failed retrieving your home directory...The software will now exit");

        GS::Gex::Message::critical("",
                                   "ERROR: Failed retrieving your home directory...\nGEX will now exit!!");
        exit(EXIT_FAILURE);
    }

    // Get application directory
    QString lApplicationDir;
    if (CGexSystemUtils::GetApplicationDirectory(lApplicationDir) != CGexSystemUtils::NoError)
    {
        GSLOG(SYSLOG_SEV_CRITICAL,
              "Failed retrieving the application directory...The software will now exit");

        GS::Gex::Message::critical("",
                                   "ERROR: Failed retrieving the application directory...\nGEX will now exit!!");
        exit(EXIT_FAILURE);
    }

    // Start application
    int	nStatus = 0;
    pGexScriptEngine    = NULL;	// static variables are sometimes not well initialized
    // GexSkin is MANDATORY even in DAEMON mode in order to retrieve html page template !
    pGexSkin = new CGexSkin();

    if (GS::Gex::Engine::GetInstance().GetCommandLineOptions().ParseArguments(argc, argv) == false)
    {
        GSLOG(SYSLOG_SEV_CRITICAL,
              QString("Cannot parse arguments: %1")
              .arg(GS::Gex::Engine::GetInstance().GetCommandLineOptions().GetArguments().join(" "))
              .toLatin1().data() );
        exit(EXIT_FAILURE);
    }

#ifdef _WIN32
    // GEXYM-400: support GS_NO_ERRORBOX env var in addition of -noErrorBox, so to use the feature
    // even when running in daemon mode.
    char *lEBD= getenv("GS_NO_ERRORBOX");
    if (GS::Gex::Engine::GetInstance().GetCommandLineOptions().IsErrorBoxDisabled() ||
            (lEBD && ((stricmp(lEBD, "1")==0) || (stricmp(lEBD, "true")==0))))
    {
        QString lLogMsg = "-noErrorBox argument or GS_NO_ERRORBOX env variable detected.";
        lLogMsg += " Adding SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX to error mode.";
        GSLOG(SYSLOG_SEV_NOTICE, lLogMsg.toLatin1().data());
        DWORD dwMode = SetErrorMode(0);
        SetErrorMode(dwMode | SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
    }
#endif // _WIN32

#ifdef GSDAEMON
    nStatus = StartAsService(argc, argv);
#else
    nStatus = StartAsApplication(argc, argv);
#endif

    GSLOG(5, QString("StartAsXXX returned %1").arg(nStatus).toLatin1().constData() );

    GSLOG(7, QString("Current allocated mem: %1").arg(sCurrentAllocated).toLatin1().constData() );

    //GS::Gex::Engine::GetInstance().Exit(EXIT_SUCCESS); // dont call Engine::Exit here, it should be done inside the App...

    GSLOG(SYSLOG_SEV_NOTICE, "quitting main()");

    if (pGexSkin)
    {
        delete pGexSkin;
        pGexSkin=0;
    }

#ifdef DEBUG_MEMORY
    // ############ DEBUG TOOL ############
    // For Debug: Dump Memory Leaks detected
    DumpUnfreed(true);
#endif

    // blocks the execution. Fix me.
    //if (pGexScriptEngine->isEvaluating())
    //  pGexScriptEngine->abortEvaluation();

    GS::SE::StatsEngine::Destroy();

    GS::Gex::CSLEngine::Destroy();

    GS::Gex::Engine::Destroy(); // connection to QCoreApp::AboutToQuit() slot does not work...

    GS::Gex::OFR_Controller::Destroy();

#ifndef GSDAEMON
    #ifdef Q_OS_MAC
        GS::LPPlugin::LicenseProviderManager::Destroy();
    #endif
#endif

    GS::LPPlugin::ProductInfo::Destroy();
    GS::QtLib::DataKeysDefinitionLoader::DestroyInstance();

#ifndef GSDAEMON
    if (pGexScriptEngine)
    {
        delete pGexScriptEngine;
        pGexScriptEngine=0;
    }
#endif

    int t=CTest::GetNumberOfInstances()
            +CTestResult::GetNumberOfInstances()
            +TemporaryFile::GetNumberOfInstances()
            +CReportsCenterParamsWidget::GetNumberOfInstances()
            //+ReportsCenterParamsDialog::GetNumberOfInstances()
            +CReportsCenterDataset::GetNumberOfInstances()
            +CReportsCenterMultiFields::GetNumberOfInstances()
            +CReportsCenterField::GetNumberOfInstances()
            +GexDbPlugin_Connector::GetNumberOfInstances();

    if (t>0)
    {
        GSLOG(SYSLOG_SEV_CRITICAL, QString("Exiting: %1 memory leaks detected").arg(t).toLatin1().data() );
        if (CTest::GetNumberOfInstances()>0)
            GSLOG(SYSLOG_SEV_WARNING, QString("Still %1 instances of CTests").arg(CTest::GetNumberOfInstances())
                  .toLatin1().data());

    }

    GSLOG(SYSLOG_SEV_NOTICE, QString("Returning from main() with code %1...").arg(nStatus).toLatin1().constData());

    GSLOG(SYSLOG_SEV_DEBUG, QString("Current allocated mem: %1").arg(sCurrentAllocated).toLatin1().constData() );

    QStringList lConnNameList = QSqlDatabase::connectionNames();
    QStringList::const_iterator lConnNameIter;
    for (lConnNameIter = lConnNameList.begin();
         lConnNameIter != lConnNameList.end(); ++lConnNameIter)
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("removeDatabase %1").
              arg(*lConnNameIter).toLatin1().constData());
        QSqlDatabase::removeDatabase(*lConnNameIter);
    }

    // Delete QApplication object.
    if (QCoreApplication::instance())
    {
        GSLOG(SYSLOG_SEV_DEBUG, "Deleting QApplication object");
        delete QCoreApplication::instance();
    }

    return nStatus;
}
