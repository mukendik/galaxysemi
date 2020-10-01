#include <QLibrary>
#include <QTreeView>
#include <QMutex>
#include <QMessageBox>
#include <QDomDocument>

#include "gex_shared.h"
#include "reports_center_widget.h"
#include "engine.h"
#include "ui_reports_center_widget.h"
#include "settings_dialog.h"
//#include "libjasper.h"
#include "browser_dialog.h"
#include "report_options.h"
#include <gqtl_log.h>
#include "libgexoc.h"
#include "product_info.h"
#include "admin_gui.h"

extern void				WriteDebugMessageFile(const QString & strMessage);
extern void				ConvertToScriptString(QString &strFile);

QLibrary* ReportsCenterWidget::pclJasperLibrary=NULL;

extern CReportOptions	ReportOptions;

void GexMainwindow::ShowReportsCenter()
{
    GS::Gex::Engine::GetInstance().UpdateLabelStatus(QString("Showing ReportsCenter..."));
    QCoreApplication::processEvents();

    GSLOG(SYSLOG_SEV_INFORMATIONAL, "ShowReportsCenter..." );

#if 0
    if (!ReportsCenterWidget::pclJasperLibrary)
    {
        #ifndef QT_DEBUG
            ReportsCenterWidget::pclJasperLibrary = new QLibrary("libjasper");
        #else
            ReportsCenterWidget::pclJasperLibrary = new QLibrary("libjasper_d");
        #endif
    }

    if(!ReportsCenterWidget::pclJasperLibrary || !ReportsCenterWidget::pclJasperLibrary->load())
    {
        WriteDebugMessageFile("\tUnable to load libjasper !");
    }

    GetVersionF gvf=NULL;
    if (ReportsCenterWidget::pclJasperLibrary)
     gvf=(GetVersionF)ReportsCenterWidget::pclJasperLibrary->resolve("jasper_get_lib_version");
    if (!gvf)
    {
        WriteDebugMessageFile("\tcant resolve version function");
    }
    else
        WriteDebugMessageFile(QString("GexMainwindow::JasperCenter: lijasper version %1").arg(gvf()));

    JasperListDirThread::LaunchListDirThread("/", pReportsCenter->s_model->m_rootItem);
    connect( JasperListDirThread::s_singleton, SIGNAL(UpdateGUI()), pReportsCenter->s_model, SLOT(UpdateGUI()) );
    connect( JasperListDirThread::s_singleton, SIGNAL(UpdateGUI()), pReportsCenter, SLOT(UpdateGUI()) );
    connect( JasperListDirThread::s_singleton, SIGNAL(SetStatusLabel(QString)),
        pReportsCenter, SLOT(SetStatusLabel(QString)));
    connect( JasperListDirThread::s_singleton, SIGNAL(PopMessageBox(QString, int)),
        pReportsCenter, SLOT(PopMessageBox(QString, int)));

    JasperRunReportThread::LaunchRunReportThread();
    connect( JasperRunReportThread::s_singleton, SIGNAL(SetStatusLabel(QString)),
        pReportsCenter, SLOT(SetStatusLabel(QString)));
    connect( JasperRunReportThread::s_singleton, SIGNAL(ReportGenerated(QString, QString)),
        pReportsCenter, SLOT(ReportGenerated(QString, QString)));
#endif

    //if (m_pReportsCenter->GetDatabaseComboBox()->count()==0)  // case 5856, pyc, 17-02-12
    // all DB but uptodate only	!
    if (pWizardAdminGui)
       pWizardAdminGui->ListDatabasesInCombo( m_pReportsCenter->GetDatabaseComboBox(),
                          DB_TYPE_SQL|DB_STATUS_CONNECTED|DB_STATUS_UPTODATE|DB_SUPPORT_RC,
                          DB_TDR_YM_PROD|DB_TDR_MANUAL_PROD);

    if (m_pReportsCenter->GetDatabaseComboBox()->count()==0)
    {
        // No report available
        QString strFilesPage;

        // Display relevant message
        if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring()
                || GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPRO()
                || GS::LPPlugin::ProductInfo::getInstance()->isExaminatorTerProPlus()
                || GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT())
        {
            strFilesPage = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
            strFilesPage += GEX_HELP_FOLDER;
            strFilesPage += GEX_HTMLPAGE_DB_NODATABASE;

        }

        LoadUrl( strFilesPage );
        return;
    }

    //pReportsCenter->ScanFolder(GS::Gex::Engine::GetInstance().GetUserFolder()+QDir::separator()+ "GalaxySemi" +QDir::separator()+"GRXML",
                               //pReportsCenter->s_model->m_rootItem); //+ TEMPLATES_FOLDER

    ShowWizardDialog(GEX_REPORTS_CENTER);
    SetWizardType(GEX_JASPER_WIZARD);
    // iWizardType = GEX_JASPER_WIZARD;		// PYC, 27,05/2011

    QCoreApplication::processEvents();

    QString r;
    if (m_pReportsCenter->s_model->m_rootItem->childCount()==0)
    {
        GS::Gex::Engine::GetInstance().UpdateLabelStatus(QString("Loading GRXML..."));
        QCoreApplication::processEvents();

        r=m_pReportsCenter->ScanUsualFolders();
    }

    m_pReportsCenter->ReplaceOutputCombobox();

    if (m_pReportsCenter->s_pCurrentParamsWidget)
    {
        m_pReportsCenter->s_pCurrentParamsWidget->show();
        m_pReportsCenter->s_pCurrentParamsWidget->setFocus();
        QApplication::setActiveWindow(m_pReportsCenter->s_pCurrentParamsWidget);
        //QApplication::focusWidget()
    }

}

bool
GexMainwindow::
WriteScriptGalaxyReportCenterSettingsSection(FILE* hFile,
                                             bool /*bFromWhatIf*/,
                                             QString /*strStartupPage*/,
                                             QString outputFormat,
                                             QString grtfile)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString(" for format %1").arg( outputFormat).toLatin1().constData());

    bool bWhatIf = false;

    // Writes 'Favorite Scripts' section
    fprintf(hFile,"//////////////////////////////////////////\n");
    fprintf(hFile,"// Setup the GEX 'Settings' section\n");
    fprintf(hFile,"//////////////////////////////////////////\n");
    fprintf(hFile,"SetReportType()\n");
    fprintf(hFile,"{\n");
    fprintf(hFile,"\n");

    fprintf(hFile,"  gexOptions('output','format','%s');\n", outputFormat.toLatin1().data());
    //fprintf(hFile,"  gexOptions('output','format','html');\n");

    QString grt=grtfile;
    ConvertToScriptString(grt);
    fprintf(hFile,"  gexReportType('adv_report_center','template','%s');\n", grt.toLatin1().constData());

    // Close function
    fprintf(hFile,"}\n");
    fprintf(hFile,"\n");

    // Notifies caller if a 'Drill' action is included...if so, will not create the standard Drill script section to avoid conflict.
    return bWhatIf;
}
