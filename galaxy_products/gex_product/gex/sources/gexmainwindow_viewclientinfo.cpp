#include <QMessageBox>
#include <QDesktopWidget>
#include <QPainter>
#include <QPaintEngine>
#include <QNetworkRequest>
#if defined __APPLE__&__MACH__
    #include <malloc/malloc.h>
#else
    #include <malloc.h>
#endif
#include <stdlib.h>

#include <gstdl_mailer.h>
#include <gstdl_utils.h>
#include <gstdl_systeminfo.h>

#include <gqtl_sysutils.h>
#include <gqtl_log.h>

#include <gtl_core.h> // in order to get the GTL versions

#include "gex_version.h"
#include "browser_dialog.h"
#include "engine.h"
#include "admin_engine.h"
#include "report_options.h"
#include "product_info.h"
#include "pat_definition.h"
#include "test_result.h"
#include "license_provider_manager.h"

extern CReportOptions	ReportOptions;		// Holds options (report_build.h)

///////////////////////////////////////////////////////////
// Network icon clicked in the Status bar: show client info
///////////////////////////////////////////////////////////
void GexMainwindow::ViewClientInfo(void)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "View Client Info...");
    QGuiApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    QString		strMessage;
    QString		strEdition;
    bool		bShowExpirationDate=true;

    if(GS::LPPlugin::ProductInfo::getInstance()->getProductID() == GS::LPPlugin::LicenseProvider::eSzOEM)
    { 	// OEM-Examinator for Credence SZ
        strMessage += "Version for: Credence SZ Testers\n";
        bShowExpirationDate = false;
    }
    else if(GS::LPPlugin::ProductInfo::getInstance()->getProductID() == GS::LPPlugin::LicenseProvider::eLtxcOEM)
    {
        // OEM-Examinator for LTXC
        strMessage += "Version for: LTXC Testers\n";
        bShowExpirationDate = false;
    }


    switch(GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode())
    {
    case GEX_RUNNINGMODE_STANDALONE:
        strMessage += "Running mode: Standalone\n";
        strMessage += strEdition;
        break;
    case GEX_RUNNINGMODE_EVALUATION:
        strMessage += "Running mode: Evaluation (4 days)\n";
        strMessage += strEdition;
        break;

    case GEX_RUNNINGMODE_CLIENT:
        strMessage += "Running mode: Client/Server\n";
        strMessage += strEdition;
        // License server
        strMessage += "License server:\n";
        strMessage += GS::LPPlugin::LicenseProviderManager::getInstance()->getLPData("ServerIP").toString();
        strMessage += "\n";
        break;
    }

    strMessage += QString("\nLP Provider: '%1'")
      .arg(GS::LPPlugin::LicenseProviderManager::getInstance()->getCurrentProvider()->property(LP_FRIENDLY_NAME).toString());

#ifdef QT_DEBUG
    strMessage += QString("\nEditionID:%1 ").arg(GS::LPPlugin::ProductInfo::getInstance()->getEditionID());
    strMessage += QString("RunningMode:%1 ").arg(GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode());
    strMessage += QString("MonitorProducts:%1 ").arg(GS::LPPlugin::ProductInfo::getInstance()->getMonitorProducts());
    strMessage += QString("OptionalModules:%1 ").arg(GS::LPPlugin::ProductInfo::getInstance()->getOptionalModules());
    strMessage += QString("ProductID: %1\n").arg(GS::LPPlugin::ProductInfo::getInstance()->getProductID());
    QDate lDate = GS::Gex::Engine::GetInstance().GetExpirationDate();
    strMessage += QString("Expiration date: %1\n").arg(lDate.isValid() ? lDate.toString() : QString("none"));
    lDate = GS::Gex::Engine::GetInstance().GetMaintenanceExpirationDate();
    strMessage += QString("Maintenance expiration date: %1\n").arg(lDate.isValid() ? lDate.toString() : QString("none"));
#endif

    if(GS::Gex::Engine::GetInstance().GetLicensePassive())
        strMessage += QString("\nLicense Status: PASSIVE MODE\n");

    //strMessage += "Web:      www.mentor.com\n";
    // Show license expiration date (unless disabled. eg: OEM version).
    if(bShowExpirationDate)
    {
        strMessage += "\nLicense expires: ";
        strMessage += GS::Gex::Engine::GetInstance().GetExpirationDate().toString(Qt::TextDate );
        strMessage += "\n";
    }

    strMessage += "Session started: ";
    strMessage += dtSessionStarted.toString(Qt::TextDate );
    strMessage += " ";
    strMessage += "Time elapsed: ";
    int	iElapsed = dtSessionStarted.secsTo(QDateTime::currentDateTime());
    int hh = iElapsed / 3600;
    int mm = (iElapsed - 3600*hh) / 60;
    int sec= iElapsed % 60;
    strMessage += QString::number(hh) + "h ";
    strMessage += QString::number(mm) + "min ";
    strMessage += QString::number(sec) + "sec.\n";
    strMessage += "\n";

    // YIELDMANDB INFO
    if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
    {
        strMessage += GS::Gex::Engine::GetInstance().GetAdminEngine().GetServerVersionName(true)+"\n";
        if(!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
            strMessage += "Supported version: "
                    +GS::Gex::Engine::GetInstance().GetAdminEngine().GetCurrentVersionName(true)+"\n";
        QMap<QString,QString> lValues = GS::Gex::Engine::GetInstance().GetAdminEngine().GetServerVersion();
        foreach(QString key, lValues.keys())
        {
            if(key.startsWith("DB_CONNECTION_"))
                strMessage += " - "+key.section("DB_CONNECTION_",1).toLower()+"="+lValues[key];
        }
        strMessage += "\n\n";
    }

    strMessage += QString("Build number: %1\n").arg(GEX_APP_VERSION_BUILD);
#ifdef GEX_APP_REVISION
    strMessage += QString("From revision: %1\n").arg(GEX_APP_REVISION);
#endif
    strMessage += "Based on Qt: "; strMessage += QT_VERSION_STR; strMessage += "\n";

    if(GS::LPPlugin::ProductInfo::getInstance()->getProductID() == GS::LPPlugin::LicenseProvider::eGTM)
        strMessage += QString("Compatible with GTL version %1.%2\n").arg(GTL_VERSION_MAJOR).arg(GTL_VERSION_MINOR);

#ifdef __GNUC__
    strMessage += QString("Built on %1 at %2\n").arg(__DATE__).arg(__TIME__);
    strMessage += QString("with GCC: %1.%2.%3\n\n").arg(__GNUC__).arg(__GNUC_MINOR__).arg(__GNUC_PATCHLEVEL__);
#endif

    strMessage += QString("Word size=%1b sizeof(int)=%2o sizeof(void*)=%3o\n")
            .arg(QSysInfo::WordSize).arg(sizeof(int)).arg(sizeof(void*));
    strMessage += QString("Ideal thread count:%1 ").arg(QThread::idealThreadCount());
#ifndef _WIN32
    CGSystemInfo lSI;
    lSI.ReadSystemInfo();
    strMessage += QString("Estimated speed:%1mghz ").arg(lSI.EstimateCPUFreq());
#endif
    strMessage += QString("Process ID:%1\n\n").arg(QCoreApplication::applicationPid());

#ifdef Q_OS_WIN
    strMessage += "Windows version : ";
    QString osname;
    switch (QSysInfo::windowsVersion())
    {
    case QSysInfo::WV_NT : osname="WV_NT"; break;
    case QSysInfo::WV_2000 : osname="WV_2000"; break;
    case QSysInfo::WV_XP : osname="WV_XP"; break;
    case QSysInfo::WV_2003 : osname="WV_2003"; break;
    case QSysInfo::WV_VISTA : osname="WV_VISTA"; break;
    case QSysInfo::WV_WINDOWS7 : osname="WV_WINDOWS7"; break;
    case QSysInfo::WV_WINDOWS8 : osname="WV_WINDOWS8"; break;
    default: osname="Not NT based windows (Win98, ME,...) !"; break;
    }
    strMessage += osname;
    strMessage += "\n";
    strMessage += "USERDOMAIN : "+QString(getenv("USERDOMAIN"))+" ";
    strMessage += "PROCESSOR_ARCHITECTURE : "+QString(getenv("PROCESSOR_ARCHITECTURE"))+" ";
    strMessage += "PROCESSOR_IDENTIFIER : "+QString(getenv("PROCESSOR_IDENTIFIER"))+" ";
    strMessage += "NUMBER_OF_PROCESSORS : "+QString(getenv("NUMBER_OF_PROCESSORS"))+"\n";
#endif

    strMessage+="Locale: "; strMessage+=QLocale::system().name(); strMessage += " ";
    strMessage+="Country: "; strMessage+=QLocale::countryToString(QLocale::system().country()); strMessage += " ";
    strMessage+="Language: "; strMessage+=QLocale::languageToString(QLocale::system().language()); strMessage+=" ";
    strMessage+="Decimal Point: ";	strMessage+=QLocale::system().decimalPoint(); strMessage+="\n";

    strMessage+=QString("\nHome dir : %1\n").arg(QDir::homePath());
    // From 6.5, the loglevels are visible in LogsCenter
    //strMessage+=QString("\nCurrent log level : %1").arg(s_log_level);

    strMessage+=QString("%1: %2\n").arg(GS::LPPlugin::ProductInfo::getInstance()->GetProfileVariable())
                .arg(GS::LPPlugin::ProductInfo::getInstance()->GetProfileFolder());
    strMessage+=QString("Profile dir: %1\n").arg(GS::Gex::Engine::GetInstance().Get("UserFolder").toString());

#ifdef QT_DEBUG
    QPainter pa;
    // "X11","Windows", "QuickDraw", "CoreGraphics", "MacPrinter", "QWindowSystem", "PostScript", "OpenGL",
    // "Picture", "SVG", "Raster", "Direct3D", "Pdf", "OpenVG", "OpenGL2", "PaintBuffer"
    strMessage+=QString("\nPaint engine type : %1").arg(pa.paintEngine()?pa.paintEngine()->type():-1);

//    QStringList atl=QStringList()<<"Application::Tty"<<"QApplication::GuiClient"<<"QApplication::GuiServer";
//    strMessage+=QString("\nQApplication::type : %1").arg( atl.at( QApplication::type() ) );

    QDesktopWidget* lDesktop = QApplication::desktop();
    if (lDesktop)
    {
        strMessage += QString("\nDesktop screen count:%1").arg(lDesktop->screenCount());
        strMessage += QString(" Virtual desktop:%1").arg(lDesktop->isVirtualDesktop()?"true":"false");
        strMessage += QString(" Desktop geom:%1x%2").arg(lDesktop->availableGeometry().width()).arg(lDesktop->availableGeometry().height() );
        strMessage += QString(" Screen geom:%1x%2\n").arg(lDesktop->screen()->width()).arg(lDesktop->screen()->height());
    }
#endif

    strMessage+=QString("\nMemory used by all test results : %1 Mo\n")
            .arg(CTestResult::GetTotalAllocated()/(1024*1024));

    // Do not perform the mem raw test : too dangerous
    QMap<QString, QVariant> lMemInfo=CGexSystemUtils::GetMemoryInfo(false, true);
    strMessage+=QString("Memory info : ");
    foreach(const QString &k, lMemInfo.keys())
        strMessage+=QString("%1:%2, ")
                .arg(k).arg(lMemInfo.value(k).toString());
    strMessage+="\n\n";

    QMessageBox messageBox(this);
    messageBox.setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    messageBox.setText(strMessage);
    //messageBox.resize(QApplication::desktop()->screenGeometry());
    messageBox.setIcon(QMessageBox::Information);
    messageBox.setWindowTitle(GS::Gex::Engine::GetInstance().Get("AppFullName").toString());
    messageBox.setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));
    GexMainwindow::applyPalette(&messageBox);

    QGuiApplication::restoreOverrideCursor();

    messageBox.exec();
}

