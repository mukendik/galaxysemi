///////////////////////////////////////////////////////////
// gex_dialog: Examinator HTML interface
///////////////////////////////////////////////////////////
#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#include <tlhelp32.h>
#undef CopyFile
#endif

#if defined unix || __MACH__
#ifdef __sun__
#include <sys/sockio.h>
#include <sys/filio.h>
#endif
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>    // for open()
#include <cerrno>     // for errno
#include <sys/stat.h> // open flag mode
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <errno.h>
#endif

#include <gqtl_sysutils.h>
#include <gqtl_filelock.h>
#include <gqtl_sqlbrowser.h>
#include "parserFactory.h"

#include <QDockWidget>
#include <QSqlError>
#include <QShortcut>
#include <qprogressbar.h>
//#include <QtPrintSupport/qprinter.h>
#include <qdesktopwidget.h>
#include <qapplication.h>
#include <qmainwindow.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qclipboard.h>
#include <QProgressDialog>
#include <qinputdialog.h>
#include <QDesktopServices>
#include <QNetworkInterface>
#include <QCoreApplication>

#include "wizards_handler.h"
#include "gex_shared.h"
#include "gex_web_browser.h"
#include "db_engine.h"
#include "browser_dialog.h"
#include "browser.h"
#include "engine.h"
#include "javascriptcenter.h"
#include "script_wizard.h"
#include "settings_dialog.h"
#include "onefile_wizard.h"
#include "comparefiles_wizard.h"
#include "mergefiles_wizard.h"
#include "scheduler_engine.h"
#include "mo_scheduler_gui.h"
#include "mixfiles_wizard.h"
#include "db_onequery_wizard.h"
#include "compare_query_wizard.h"
#include "report_build.h"
#include "report_options.h"
#include "drill_what_if.h"
#include "drill_chart.h"
#include "drill_table.h"
#include "drillDataMining3D.h"
#include "admin_engine.h"
#include "admin_gui.h" // ExaminatorDB
#include "gex_web.h"  // ExaminatorWEB
#include <gex_scriptengine.h>
#include "tb_toolbox.h" // Examinator ToolBox
#include "patman_lib.h"
#include "testerserver.h"
#include "mo_admin.h"
#include "reports_center_widget.h"
#include "options_center_widget.h"
#include <gqtl_log.h>
#include "product_info.h"
#include "csl/csl_engine.h"
#include "command_line_options.h"
#include "pat_report_ft_gui.h"

#include "assistant_flying.h"
#include "scripting_io.h"

#include "license_provider_profile.h"
#include "gex_constants.h"
#include "db_transactions.h"
#include "gex_pixmap_extern.h"
#include "gex_skins.h"
#include "temporary_files_manager.h"
#include "tasks_manager_engine.h"
#include "gtm_mainwidget.h"
#include "message.h"
#include "admin_engine.h"
#include "pat_widget_ft.h"
#include "pat_recipe_editor.h"
#include "dir_access_base.h"
#include "QDetachableTabWindowContainer.h"
#include "QTitleFor.h"
#include "ofr_controller.h"

#include <gqtl_archivefile.h>

#include <QDesktopServices>

#define GEX_HTMLPAGE_HOME_GEXPAT        "_gex_patman_home.htm"      // Examinator-PAT home page.
#define GEX_HTMLPAGE_HOME_GEXPAT_TDR    "_gex_patman_tdr_home.htm"  // Examinator-PAT home page with TDR license.

// main.cpp
extern GexMainwindow *  pGexMainWindow;
extern QProgressBar  *  GexProgressBar; // Handle to progress bar in status bar
extern QLabel        *  GexScriptStatusLabel; // Handle to script status text in status bar
extern bool toolCopyFolderContent(QString strFromFolder,QString strToFolder);

extern QString strIniClientSection;
extern GexScriptEngine* pGexScriptEngine;

//extern QDate ExpirationDate;
// report_build.cpp
extern CReportOptions ReportOptions;  // Holds options (report_build.h)
extern CGexReport *gexReport;   // Handle to report class

extern CGexSkin*  pGexSkin;  // holds skin settings

///////////////////////////////////////////////////////////
// Keep track of temporary STDF files created...
///////////////////////////////////////////////////////////
void GexMainwindow::applyPalette(QWidget * pWidget)
{
    if (!pWidget)
    {
        GSLOG(SYSLOG_SEV_ERROR, "cant applyPalette on NULL widget");
        return;
    }

    if (!pGexSkin)
    {
        GSLOG(SYSLOG_SEV_ERROR, "cant applyPalette with a NULL GexSkin");
        return;
    }

    pGexSkin->applyPalette(pWidget);

}

///////////////////////////////////////////////////////////
// Returns the path of the given path+file name.
///////////////////////////////////////////////////////////
char *FindPath(char *szString)
{
    int  iIndex;

    iIndex = strlen(szString);
    if(iIndex < 1)
        return NULL;

    iIndex--;
    do
    {
        if(szString[iIndex] == '\\' || szString[iIndex] == ':' || szString[iIndex] == '/')
            return &szString[iIndex];
        iIndex--;
    }
    while(iIndex >= 0);
    return NULL;
}

///////////////////////////////////////////////////////////
// Returns the location of the file name extension
///////////////////////////////////////////////////////////
char *FindExt(char *szString)
{
    int  iIndex;

    iIndex = strlen(szString);
    if(iIndex < 1)
        return NULL;

    iIndex--;
    do
    {
        if(szString[iIndex] == '.')
            return &szString[iIndex];
        iIndex--;
    }
    while(iIndex >= 0);
    return NULL;
}

///////////////////////////////////////////////////////////
// Close signal...then delete all childs.
///////////////////////////////////////////////////////////
void GexMainwindow::closeEvent(QCloseEvent *e)
{
    GS::Gex::OFR_Controller::GetInstance()->Hide();
    GSLOG(SYSLOG_SEV_NOTICE, QString("Close event received Property(%1)").arg(e->type()).toLatin1().constData());

    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring()
            && GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
    {
        if(!GS::Gex::Engine::GetInstance().GetLicensePassive())
        {
            if(!GS::Gex::Engine::GetInstance().GetAdminEngine().IsUserConnected())
            {
                QString strAction = "close ";
                strAction += GS::Gex::Engine::GetInstance().Get("AppFullName").toString();
                // Get the short name
                if(strAction.count("-") > 1)
                    strAction = strAction.section("-",0,1);
                else
                    strAction = strAction.section("-",0,0);

                //
                if (GS::Gex::Engine::GetInstance().GetAdminEngine().GetDirAccessPlugin() &&
                        !GS::Gex::Engine::GetInstance().GetAdminEngine().GetDirAccessPlugin()->GetConnector()->IsConnected())
                {
                    bool lOk;
                    GS::Gex::Message::request("Exit", "Connection lost\nYou can Exit or Wait the connection to be"
                                              " reestablished, \nPress OK to Exit", lOk);
                    if (lOk)
                    {   
                        e->accept();
                        QCoreApplication::processEvents();
                        return;
                    }
                }
                else
                {
                    pWizardAdminGui->UserConnectionNeededToDialog(strAction);

                    pWizardAdminGui->ConnectUserDialog(true,
                                                       !GS::LPPlugin::ProductInfo::getInstance()->isMonitoring());
                    e->ignore();
                    return;
                }
            }
        }
    }

    if (mGtmWidget)
    {
#ifdef GCORE15334
        if (GS::Gex::Engine::GetInstance().GetTesterServer().GetNumOfClients()>0)
            //if (mGtmWidget->TotalStations()!=0)
        {
#ifdef QT_DEBUG
            // Temporary test to check the OnExit() which will be used for the daemon version will work
            //GS::Gex::Engine::GetInstance().GetTesterServer().OnExit();
            //return;
#endif

            mGtmWidget->OnPlayStopButtonPressed();

            QString lM=QString("%1 clients are still connected. Press Stop first to close clients.")
                    .arg( GS::Gex::Engine::GetInstance().GetTesterServer().GetNumOfClients() ); // mGtmWidget->TotalStations()
            GSLOG(SYSLOG_SEV_NOTICE, lM.toLatin1().constData());
            mGtmWidget->show();
            GexScriptStatusLabel->setText(lM);
            if (statusBar())
                statusBar()->showMessage(lM);
            e->ignore();
            return;
        }
        else
            mGtmWidget->close();
#endif
    }

#   if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
    // GCORE-6886
    if (mQDetachableTabWindowContainer)
    {
        delete mQDetachableTabWindowContainer;
        mQDetachableTabWindowContainer = 0;
    }
#   endif

    // Don't accept to close the application if running
    if (!GS::Gex::Engine::GetInstance().GetTaskManager().OnStopAllTaskRequired())
    {
        GSLOG(SYSLOG_SEV_NOTICE, "Some tasks are being executed. The program will exit when all tasks would have been stopped");
        e->ignore();
    }

#ifdef GCORE15334
    GS::Gex::PATRecipeEditor::DestroyInstance();
#endif
    GS::Gex::Engine::GetInstance().
        GetAdminEngine().DisconnectToAdminServer();
    GS::Gex::Engine::GetInstance().
        GetAdminEngine().DeleteAdminServerConnector();
    GSLOG(SYSLOG_SEV_NOTICE, "All tasks have been properly stopped.");
    // since Qt5 app close does not auro delete/close independant widgets ar RC widgets...
    if (m_pReportsCenter)
        m_pReportsCenter->DeleteAllParamsWidgets();

    if (pWizardAdminGui)
    {
        pWizardAdminGui->close();
        delete pWizardAdminGui;
        pWizardAdminGui=0;
    }
    e->accept();

}

///////////////////////////////////////////////////////////
// Close gex
///////////////////////////////////////////////////////////
void GexMainwindow::closeGex()
{
    GS::Gex::OFR_Controller::GetInstance()->Hide();
    GSLOG(SYSLOG_SEV_NOTICE, "closing gex...");
    close();
}

///////////////////////////////////////////////////////////
// Starting DRAG sequence
///////////////////////////////////////////////////////////
void GexMainwindow::dragEnterEvent(QDragEnterEvent *e)
{
    // GTM: reject grag&drop
    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        return;

    // no drag and drop if monitoring mode (Yield-Man or Pat-Man (case 4149)
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        return;

    // Accept Drag if files list dragged over.
    if(e->mimeData()->formats().contains("text/uri-list"))
        e->acceptProposedAction();
}

///////////////////////////////////////////////////////////
// Dropping files in Widget (Drag & Drop)
///////////////////////////////////////////////////////////
void GexMainwindow::dropEvent(QDropEvent *e)
{
    // Extract list of files dragged over...
    if(!e->mimeData()->formats().contains("text/uri-list"))
    {
        // No files dropped...ignore drag&drop.
        e->ignore();
        return;
    }

    QString  strFileName;
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

    // Accept drag & drop
    e->acceptProposedAction();

    onProcessDataFile(strFileList, false);

}
///////////////////////////////////////////////////////////
// Load Examinator navigation HTML bar + bottom bar
///////////////////////////////////////////////////////////
void GexMainwindow::LoadNavigationTabs(int lProductID, bool bLoadHeader,bool bLoadFooter)
{
    const char * ptTopBarPageName    = NULL;
    const char * ptBottomBarPageName = NULL;
    QString strString, strSource, strDest, strFolder;

    GSLOG(SYSLOG_SEV_DEBUG, QString(" lProductID = %1 ").arg(lProductID).toLatin1().data() );

    // Display relevant HTML page.
    if(lProductID == GS::LPPlugin::LicenseProvider::eYieldMan || lProductID == GS::LPPlugin::LicenseProvider::eYieldManEnterprise ||
            lProductID == GS::LPPlugin::LicenseProvider::ePATMan || lProductID == GS::LPPlugin::LicenseProvider::ePATManEnterprise)
    {
        ptTopBarPageName = GEXMO_HTMLPAGE_TOPBAR;
        ptBottomBarPageName = GEX_HTMLPAGE_BOOTOMBAR;
    }
    else
    {
        ptTopBarPageName = GEX_HTMLPAGE_TOPBAR;
        ptBottomBarPageName = GEX_HTMLPAGE_BOOTOMBAR;
    }
    // Check if GTM

    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
    {
        ptTopBarPageName = GEXGTM_HTMLPAGE_TOPBAR;
        ptBottomBarPageName = GEX_HTMLPAGE_BOOTOMBAR;
    }

    // GCORE-7369 - load an home page specific to this OEM version
    if (GS::LPPlugin::ProductInfo::getInstance()->isExaminatorTerProPlus() ||
        GS::LPPlugin::ProductInfo::getInstance()->getProductID() == GS::LPPlugin::LicenseProvider::eTerOEM)
    {
      ptTopBarPageName = GEX_HTMLPAGE_TOPBAR_TERADYNE;
    }

    // Default skin navigation folder (where HTML pages are stored)
    strFolder = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString() + GEX_HTML_FOLDER;

    // If OEM version, need to overwrite some of the images with the OEM ones!
    if(lProductID == GS::LPPlugin::LicenseProvider::eSzOEM)
    {
        // Copy folder <app>/html/images_oem/credence/* to <app>/html/images
        strSource = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString() + "/html/images_oem/credence/";
        strDest= GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString() + GEX_IMAGE_FOLDER;
        toolCopyFolderContent(strSource,strDest);

        // Copy folder <app>/help/pages_oem/credence/* to <app>/help/pages
        strSource = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString() + "/help/pages_oem/credence/";
        strDest= GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString() + GEX_HELP_FOLDER;
        toolCopyFolderContent(strSource,strDest);

        // Copy folder <app>/help/images_oem/credence/* to <app>/help/images
        strSource = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString() + "/help/images_oem/credence/";
        strDest= GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString() + GEX_IMAGE_FOLDER;
        toolCopyFolderContent(strSource,strDest);

        // Copy folder <app>/pages/pages_oem/credence/* to <app>/pages
        strSource = navigationSkinDir() + "/report/pages/pages_oem/credence/";
        strDest= GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString() + "/pages/";
        toolCopyFolderContent(strSource,strDest);

    }
    else
    {
        // Set path to relevant skin HTML pages
        strFolder =  navigationSkinDir() + "pages/";

    }

    if(bLoadHeader == true)
    {
        strString = strFolder;
        strString += ptTopBarPageName;

        pageHeader->setSource( QUrl::fromLocalFile(strString) );
    }

    if(bLoadFooter == true)
    {
        strString = strFolder;
        strString += ptBottomBarPageName;

        pageFooter->setSource(QUrl::fromLocalFile(strString) );
    }

    GSLOG(SYSLOG_SEV_DEBUG, " done");
}

void GexMainwindow::UpdateLabelStatus(const QString &message)
{
    if (GexScriptStatusLabel)
    {
        GexScriptStatusLabel->setText(message);
        if(message.isEmpty())
            GexScriptStatusLabel->hide();
        else
        {
            if((message.length()*7) > GexScriptStatusLabel->width())
                GexScriptStatusLabel->setMinimumWidth(message.length()*7);
            GexScriptStatusLabel->show();
        }
    }
}

// moved in Engine
void GexMainwindow::ExitGexApplication(int lExitCode)
{
    UpdateLabelStatus("Exiting...");
    QString strMessage = "The application will exit with code " + QString::number(lExitCode);
    GSLOG(SYSLOG_SEV_INFORMATIONAL,strMessage.toLatin1().constData());
}

///////////////////////////////////////////////////////////
// Resize GEX application to fit in screen...
///////////////////////////////////////////////////////////
void GexMainwindow::ResizeGexApplication(int iOffset)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Resize Gex Application with offset %1").arg( iOffset).toLatin1().constData());

    if( GS::Gex::Engine::GetInstance().GetClientState() ==  GS::Gex::Engine::eState_Exit)
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("Application exit no need to resize the application ").toLatin1().constData());

    }

    int iSizeX=0,iSizeY=0;
    iScreenSize = GEX_SCREENSIZE_LARGE;
    QDesktopWidget *lDesktop = QApplication::desktop();
    if (!lDesktop)
    {
        GSLOG(SYSLOG_SEV_WARNING, "Cant retrieve Desktop !");
        return;
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("returns desktop width").toLatin1().constData());
    int iWidth = lDesktop->width();     // returns desktop width

    if(iWidth <= 640)
    {
        iScreenSize = GEX_SCREENSIZE_SMALL;
        GSLOG(SYSLOG_SEV_DEBUG, QString("GEX_SCREENSIZE_SMALL").toLatin1().constData());
        move(0,0);
        iSizeX = 635+iOffset;
        iSizeY = 430;
    }
    else
        if(iWidth <= 800)
        {
            iScreenSize = GEX_SCREENSIZE_MEDIUM;
            GSLOG(SYSLOG_SEV_DEBUG, QString("GEX_SCREENSIZE_MEDIUM").toLatin1().constData());
            move(0,0);
            iSizeX = 795+iOffset;
            iSizeY = 550;
        }
        else
        {
            iScreenSize = GEX_SCREENSIZE_LARGE;
            GSLOG(SYSLOG_SEV_DEBUG, QString("GEX_SCREENSIZE_LARGE").toLatin1().constData());
            move(0,0); // my screen width is large but my screen height is low !

            iSizeX = 990+iOffset;
            iSizeY = 710;
#ifdef WILLIAM_TAMBELLINI
            //iSizeX=d->width();
            //iSizeY=(int)(0.9f*(float)d->height());
#endif
        }


    GSLOG(SYSLOG_SEV_DEBUG, QString("Resize application ( %1,%2 )").arg(iSizeX).arg(iSizeY).toLatin1().constData());
    // Resize application
    resize( iSizeX,iSizeY );
    //showMaximized();

    // If screen is too small, enable scroll bars.
    GSLOG(SYSLOG_SEV_DEBUG, QString("iScreenSize ( %1)").arg(iScreenSize).toLatin1().constData());
    switch(iScreenSize)
    {
    case GEX_SCREENSIZE_SMALL:
    case GEX_SCREENSIZE_MEDIUM:
        pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        pScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        break;
    case GEX_SCREENSIZE_LARGE:
        pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        pScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        break;
    }
}

void GexMainwindow::ClearDetachableWindows()
{
    if(mQDetachableTabWindowContainer)
    {
        mQDetachableTabWindowContainer->close();
        delete mQDetachableTabWindowContainer;
        mQDetachableTabWindowContainer = 0;
    }
}

///////////////////////////////////////////////////////////
// Set 'Build Report!' button title (or reset if 'Abort' command)
///////////////////////////////////////////////////////////
QString GexMainwindow::BuildReportButtonTitle(QString strTitle /*=""*/, bool bBackupCurrentTitle /*=false*/, bool deleteDetachableWindows)
{
    static QString strPreviousTitle;
    static QString strNewTitle;

    // Check if title to be reset following a 'Abort' command
    if(strTitle.isEmpty() == true)
    {
        // Reset title to previous string.
        strNewTitle = strPreviousTitle;
    }
    else
    {
        // Update title to new string and keep previous title
        if((strNewTitle.isEmpty() == true) || (bBackupCurrentTitle == false))
            strNewTitle = strPreviousTitle = strTitle;
        else
        {
            strPreviousTitle = strNewTitle;
            strNewTitle = strTitle;
        }
    }

    // Update Buttons title ('Settings' and 'Options' pages)
    pWizardSettings->buttonBuildReport->setText(strNewTitle);

    if(m_pOptionsCenter)
        m_pOptionsCenter->m_BuildReport->setText(strNewTitle);

    //-- if gex report not finished yet, we do not reload the interactive views
    if(gexReport && gexReport->getGroupsList().size() > 0)
    {
        if( deleteDetachableWindows)
        {
           ClearDetachableWindows();

            mWizardsHandler.Clear();
        }
        else
        {
            mWizardsHandler.ReloadWizards(true);
        }
    }

    // Return title
    return strNewTitle;
}

/**
 * @brief static utility function to recursively search into a dom document for
 * a specific element having an 'id' attribute initialized with a specific value
 * @param doc reference on a constant instance of QDomDocument
 * @param id the id attribute value we're looking for
 * @return a null QDomElement if not any element is found otherwise, the target
 * element
 */
QDomElement GexMainwindow::GetElementByIdIn
  ( const QDomDocument &doc, const QString &id )
{
  // check intrants
  if( ! doc.isNull() && ! id.isEmpty() )
  {
    // not recursive, but stack aware
    QStack< QDomNodeList > stack;

    do
    {
      // first list of children to check : the one of the document, otherwise
      // pop one list in the stack
      const QDomNodeList &node_list =
         stack.isEmpty() ? doc.childNodes() : stack.pop();

      for( int i = 0; i < node_list.length(); ++i )
      {
        const QDomNode &node = node_list.at( i );
        const QDomElement &elt = node.toElement();

        if( ! elt.isNull() )
        {
          if( elt.attribute( "id" ) == id )
            return elt;

          stack.push( elt.childNodes() );
        }
      }
    }
    while( ! stack.isEmpty() );
  }

  return QDomElement();
}

///////////////////////////////////////////////////////////
// Process file given as argument while launching Gex or while
// a drag and drop
///////////////////////////////////////////////////////////
void GexMainwindow::onProcessDataFile(QStringList strDataFile, bool bFromCommandLine)
{
    // Flag if the product type is 'Yield 123'
    bool bYield123 = false;

    // If list file is empty
    if (strDataFile.isEmpty())
        return;

    QString strAbsFileName = strDataFile.first();

    CArchiveFile lArchive;

    if (lArchive.IsCompressedFile(strAbsFileName))
    {
        if (GS::Parser::ParserFactory::GetInstance()->FindParserType(strAbsFileName.toStdString()) == GS::Parser::typeUnknown)
        {
            // If it's a supported compressed file (zip, tar, gz or Z),
            // Extract file list
            strDataFile = extractSTDFZipFiles(strAbsFileName);
            if ((strAbsFileName.endsWith(".gex", Qt::CaseSensitive) == true) && bFromCommandLine)
                GS::Gex::Engine::GetInstance().GetTempFilesManager().addFile(strAbsFileName, TemporaryFile::CloseCheck);
            // If list file is empty
            if (strDataFile.isEmpty())
                return;
        }
    }

    if (strDataFile.count() == 1)
    {
        strAbsFileName = strDataFile.first();
        // Check if file is a HTML or Test data file
        if (!bFromCommandLine && (strAbsFileName.endsWith(".htm", Qt::CaseInsensitive) || strAbsFileName.endsWith(".html", Qt::CaseInsensitive)))
        {
            // HTML file...simply load it into HTML browser!
            OnBrowseUrl(strAbsFileName);
            return;  // Clear flag so we know we've not dragged a Test data file.
        }

        // If file name ends with '_patman_config.csv', then edit the PAT configuration file!
        else if (!bFromCommandLine && strAbsFileName.endsWith(".csv", Qt::CaseInsensitive) && (strAbsFileName.indexOf("_patman_config") >= 0))
        {
#ifdef GCORE15334
            Wizard_GexTb_EditPAT_Limits();

            GS::Gex::PATRecipeEditor::GetInstance().RefreshEditor(strAbsFileName);
#endif
        }

        // If Report Template (.grt), select it in the 'Settings' page
        else if (!bFromCommandLine && strAbsFileName.endsWith(".grt", Qt::CaseInsensitive) && pWizardSettings != NULL)
        {
            pWizardSettings->OnSelectReportTemplate(strAbsFileName);
        }

        // If Script file (.csl), select it in the 'Scripting' page and not launch from command line
        else if (!bFromCommandLine && strAbsFileName.endsWith(".csl", Qt::CaseInsensitive) && pWizardScripting != NULL)
        {
            // Run script
            GS::Gex::CSLEngine::GetInstance().RunScript(strAbsFileName);
        }
        // Other extension...assume it's a data file.
        else
        {
            // select it for data analysis.
            pWizardOneFile->OnSelectFile(strAbsFileName);
            // Launch Single file analysis wizard...
            Wizard_OneFile_Page1();

            // Select 'Files' tab in web bar.
            QString gexPageName = navigationSkinDir() + "pages/" + GEX_BROWSER_ACTIONLINK;
            gexPageName += GEX_BROWSER_FILES_TOPLINK;
            pageHeader->setSource(QUrl::fromLocalFile(gexPageName));
        }
        return;
    }

    int iExecStatus;
    QMessageBox msgBox(this);
    msgBox.setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));
    msgBox.setText("Merge or Compare files ?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::No);

    // Multiple files selected: ask if Compare or Merge?
    msgBox.setButtonText( QMessageBox::Yes, "&Merge" );
    msgBox.setButtonText( QMessageBox::No, "&Compare" );
    msgBox.setButtonText( QMessageBox::Cancel, "&Cancel" );

    if (!bFromCommandLine)
    {
        if(!bYield123)
            iExecStatus = msgBox.exec();
        else
            iExecStatus = QMessageBox::No; // Under Yield123, always consider "compare files"
    }
    else
        iExecStatus = msgBox.exec();


    if (iExecStatus == QMessageBox::Yes)
    {
        // Merge all files...clear previous content of the listbox first
        pWizardAddFiles->OnAddFiles(strDataFile,true);
        Wizard_MergeFile_Page1();
    }
    else if (iExecStatus == QMessageBox::No)
    {
        // Compare files...clear previous content of the listbox first
        if(!bFromCommandLine && bYield123)
        {
            pWizardCmpFiles->OnAddFiles(strDataFile);
            Wizard_CompareFile_Page1();
        }
        else
        {
            pWizardCmpFiles->OnAddFiles(strDataFile,true);
            Wizard_CompareFile_Page1();
        }
    }
    else
        return;

    // Select 'Files' tab in web bar.
    QString gexPageName = navigationSkinDir() + "pages/" + GEX_BROWSER_ACTIONLINK;
    gexPageName += GEX_BROWSER_FILES_TOPLINK;
    pageHeader->setSource(QUrl::fromLocalFile(gexPageName));
}

///////////////////////////////////////////////////////////
// Extract the content of the zip files in tmp folder
// Return the list of extracted files
///////////////////////////////////////////////////////////
QStringList GexMainwindow::extractSTDFZipFiles(QString strZipAbsoluteFilePath)
{
    QString   strUnzipFolder; // Custom path
    QStringList  strUncompressedFiles, strValidSTDFFiles;
    QString   strFileName, strMessage, strFileNameSTDF;

    // Get custom path if defined
    if (ReportOptions.GetOption("dataprocessing", "stdf_intermediate").toString() == "custom")
        strUnzipFolder =  ReportOptions.GetOption("dataprocessing", "stdf_intermediate_path").toString();

    // If custom path doesn't exist or is not defined, use the default path (same as the data file)
    if(QFile::exists(strUnzipFolder) == false)
        strUnzipFolder = QFileInfo(strZipAbsoluteFilePath).absolutePath(); // Default path:same as data file

    CArchiveFile clZip(strUnzipFolder) ;
    QProgressDialog progressDialog("Extracts...", "Abort", 0, 0, this);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setWindowTitle("Extracting files...");
    progressDialog.setMaximumSize(this->size());
    connect(&clZip, SIGNAL(sUncompressBegin(QString)), &progressDialog, SLOT(setLabelText(QString)));
    progressDialog.show();
    QCoreApplication::processEvents();

    if (!clZip.Uncompress(strZipAbsoluteFilePath, strUncompressedFiles)) // If failed to uncompress file
    { // Send error message and exit
        strMessage = clZip.GetLastErrorMsg();
        GS::Gex::Message::information("", strMessage);
        strValidSTDFFiles.clear();
    }
    else              // If success to uncompress file
    {
        QStringList::iterator it;        // Iterator on uncompress files list
        for (it = strUncompressedFiles.begin(); it != strUncompressedFiles.end(); ++it)
        {
            QFileInfo cFileInfo;
            // Build full path to zip file.
            cFileInfo.setFile(strUnzipFolder, *it);
            strFileName = cFileInfo.filePath();

            // If strFileName is a compressed file
            if (clZip.IsCompressedFile(strFileName))
            {
                QStringList strSingleUncompressedFile;
                if (!clZip.Uncompress(strFileName, strSingleUncompressedFile))
                {
                    strMessage = clZip.GetLastErrorMsg();
                    GS::Gex::Message::information("", strMessage);
                    // No return to avoid stopping process because of one file
                }
                else// At this level the archive file is supposed to handle just ONE file so we retrieve the first one
                {
                    // remove temporary archive file
                    QFile(strFileName).remove();
                    // Build full path to zip file.
                    cFileInfo.setFile(strUnzipFolder, strSingleUncompressedFile.first());
                    strFileName = cFileInfo.filePath();
                }
            }

            // If compressed data are STDF files, append custom extension so they are deleted on Examinator exit.
            if(GS::StdLib::Stdf::IsCompatible(strFileName.toLatin1().constData()))
            {
                // Append custom extension so we can erase this STDF temporary file later on...
                strFileNameSTDF = strFileName;
                strFileNameSTDF += ".gexarg.std";
                QDir cDir;
                cDir.rename(strFileName,strFileNameSTDF);

                // Add file to list of temporary files to erase on Examinator exit...
                GS::Gex::Engine::GetInstance().GetTempFilesManager().addFile(strFileNameSTDF, TemporaryFile::CloseCheck);
                strValidSTDFFiles << strFileNameSTDF;
            }
            else
                // Add file to list of temporary files that could be remove at any time
                GS::Gex::Engine::GetInstance().GetTempFilesManager().addFile(strFileName, TemporaryFile::BasicCheck);
        }
        if (progressDialog.wasCanceled())
            strValidSTDFFiles.clear();
    }
    progressDialog.close();
    QCoreApplication::processEvents();
    return strValidSTDFFiles;
}

///////////////////////////////////////////////////////////
// Load relevant Examinator Skin (Examinator, ExaminatorDB,...)
///////////////////////////////////////////////////////////
void GexMainwindow::ReloadProductSkin(bool bSplash)
{
    QString       strString;

    // Unknown skin...stop here!
    if (navigationSkinDir().isEmpty())
        return;

    // Apply the palette to the parent widget
    if (pGexSkin)
        pGexSkin->applyPalette(centralWidget());
    else
        GSLOG(SYSLOG_SEV_WARNING, "cant apply Palette to central widget");

    // HTML skin pages Sub-folder .
    strString = navigationSkinDir() + "pages/";

    if(bSplash == true)
    {
        // Client/Server connection: show splash screen until server reply received...
        strString += GEX_HTMLPAGE_SPLASHHOME;

        // Update application Caption name to a temporary name...until server tells what's running!
        UpdateCaption();
    }
    else
    {
        UpdateCaption();

        // Display relevant HTML page.
        // switch(GS::LPPlugin::ProductInfo::getInstance()->getProductID())

        if(GS::LPPlugin::ProductInfo::getInstance()->isExaminator() || GS::LPPlugin::ProductInfo::getInstance()->isExaminatorEval())
        {
            strString += GEX_HTMLPAGE_HOME_PRO; // Gex-Pro Edition
        }
        else if(GS::LPPlugin::ProductInfo::getInstance()->isOEM())
        {
            if(GS::LPPlugin::ProductInfo::getInstance()->getProductID() == GS::LPPlugin::LicenseProvider::eSzOEM)
                strString += GEX_HTMLPAGE_HOME_SZ;
            else if(GS::LPPlugin::ProductInfo::getInstance()->getProductID() == GS::LPPlugin::LicenseProvider::eLtxcOEM)
                strString += GEX_HTMLPAGE_HOME_LTXC;
            else if(GS::LPPlugin::ProductInfo::getInstance()->getProductID() == GS::LPPlugin::LicenseProvider::eTerOEM)
                strString += GEX_HTMLPAGE_HOME_TERADYNE;

            // Script is DISABLED
            actionSeparatorStop->setVisible(false);
            actionScripting->setVisible(false);

            // All wizard pages have to be in White background (Credence request)
            QPalette palette;
            palette.setColor(pScrollArea->backgroundRole(), Qt::white);
            pScrollArea->setPalette(palette);
            pWizardOneFile->setPalette(palette);
            pWizardCmpFiles->setPalette(palette);
            pWizardAddFiles->setPalette(palette);
            pWizardMixFiles->setPalette(palette);
            pWizardOneQuery->setPalette(palette);
            pWizardCmpQueries->setPalette(palette);
            pWizardAdminGui->setPalette(palette);
            pWizardSettings->setPalette(palette);
            //            pScrollArea->setPaletteBackgroundColor(Qt::white);
            //            pWizardOneFile->setPaletteBackgroundColor(Qt::white);
            //            pWizardCmpFiles->setPaletteBackgroundColor(Qt::white);
            //            pWizardAddFiles->setPaletteBackgroundColor(Qt::white);
            //            pWizardMixFiles->setPaletteBackgroundColor(Qt::white);
            //            pWizardOneQuery->setPaletteBackgroundColor(Qt::white);
            //            pWizardCmpQueries->setPaletteBackgroundColor(Qt::white);
            //            pWizardAdminGui->setPaletteBackgroundColor(Qt::white);
            //            pWizardSettings->setPaletteBackgroundColor(Qt::white);

            if(m_pOptionsCenter)
                m_pOptionsCenter->setPalette(palette);
            //                m_pOptionsCenter->setPaletteBackgroundColor(Qt::white);

            pWizardScripting->setPalette(palette);
            //            pWizardScripting->setPaletteBackgroundColor(Qt::white);
        }
        else if(GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPRO() ||
                GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT() ||
                GS::LPPlugin::ProductInfo::getInstance()->isExaminatorTerProPlus())
        {
            // ExaminatorWEB...create I/O transaction module
            if (mGexWeb == NULL)
            {
                mGexWeb = new GexWeb();
            }

            //  ExaminatorWeb must fall into  Database!
            // ExaminatorDB: Database access, any file type.

            if(mMonitoringWidget == NULL)
            {
                mMonitoringWidget = new MonitoringWidget(centralWidget());
                mMonitoringWidget->hide();

                if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
                    pScrollArea->layout()->addWidget(mMonitoringWidget);

                // Signal to tell Admin tab that a MO history log file has been modified
                connect(&GS::Gex::Engine::GetInstance().GetSchedulerEngine(),
                        SIGNAL(sMoHistoryLogUpdated(const QString &,const QString &)),
                        mMonitoringWidget,
                        SLOT(OnMoHistoryLogUpdated(const QString &,const QString &)));
                // Signal to tell Admin tab that a MO report log file has been modified
                connect(&GS::Gex::Engine::GetInstance().GetSchedulerEngine(),
                        SIGNAL(sMoReportLogUpdated(const QString &,const QString &)),
                        mMonitoringWidget,
                        SLOT(OnMoReportLogUpdated(const QString &,const QString &)));
            }

            if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated() ||
                    GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
            {
                if(pWizardSchedulerGui == NULL)
                {
                    pWizardSchedulerGui = new SchedulerGui(centralWidget());
                    pWizardSchedulerGui->hide();
                    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
                        pScrollArea->layout()->addWidget(pWizardSchedulerGui);
                }

            }
            // Path to HTML home page.
            if(GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT())
            {
                if (GS::LPPlugin::ProductInfo::getInstance()->isTDRAllowed())
                    strString += GEX_HTMLPAGE_HOME_GEXPAT_TDR;  // Examinator-PAT home page with TDR license.
                else
                    strString += GEX_HTMLPAGE_HOME_GEXPAT;      // Examinator PAT-Man home page
            }
            else if (GS::LPPlugin::ProductInfo::getInstance()->isExaminatorTerProPlus())
            {
                strString += GEX_HTMLPAGE_HOME_DB_PRO_PLUS;  // Teradyne Examinator-Pro+ home page
            }
            else if (GS::LPPlugin::ProductInfo::getInstance()->isTDRAllowed())
                strString += GEX_HTMLPAGE_HOME_DB_TDR; // Examinator TDR home page
            else
                strString += GEX_HTMLPAGE_HOME_DB_PRO; // GexDB- Pro Edition
        }
        else if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        {
            if(pWizardSchedulerGui == NULL)
            {
                pWizardSchedulerGui = new SchedulerGui(centralWidget());
                pScrollArea->layout()->addWidget(pWizardSchedulerGui);

                // Task Manager
                connect(&GS::Gex::Engine::GetInstance().GetTaskManager(), SIGNAL(sRestartAllPendingTask()),
                        &GS::Gex::Engine::GetInstance().GetSchedulerEngine(), SLOT(OnRestartAllPendingTask()));
                connect(&GS::Gex::Engine::GetInstance().GetTaskManager(), SIGNAL(sCloseApplication()),
                        this, SLOT(close()));
            }

            if(mMonitoringWidget == NULL)
            {
                mMonitoringWidget = new MonitoringWidget(centralWidget());
                pScrollArea->layout()->addWidget(mMonitoringWidget);

                // Signal to tell Admin tab that a MO history log file has been modified
                connect(&GS::Gex::Engine::GetInstance().GetSchedulerEngine(),
                        SIGNAL(sMoHistoryLogUpdated(const QString &,const QString &)),
                        mMonitoringWidget,
                        SLOT(OnMoHistoryLogUpdated(const QString &,const QString &)));

                // Signal to tell Admin tab that a MO report log file has been modified
                connect(&GS::Gex::Engine::GetInstance().GetSchedulerEngine(),
                        SIGNAL(sMoReportLogUpdated(const QString &,const QString &)),
                        mMonitoringWidget,
                        SLOT(OnMoReportLogUpdated(const QString &,const QString &)));
            }

            // Path to HTML home page.
            if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
            {
                // PAT-Man home page
                if(GS::LPPlugin::ProductInfo::getInstance()->isEnterprise())
                    strString += GEX_HTMLPAGE_HOME_PME;
                else
                    strString += GEX_HTMLPAGE_HOME_PATSERVER;
            }
            else
            {
                // Yield-Man home page
                if(GS::LPPlugin::ProductInfo::getInstance()->isEnterprise())
                    strString += GEX_HTMLPAGE_HOME_YME;
                else
                    strString += GEX_HTMLPAGE_HOME_MONITORING;
            }

            // Change default 'Data' tab to point to 'Single Query' instead of 'One file analysis'
            SetWizardType(GEX_ONEQUERY_WIZARD);  //iWizardType = GEX_ONEQUERY_WIZARD;  // PYC, 27/05/2011

            // Hide some icons from the toolbar
            actionStop->setVisible(false);  // Stop Script action
            actionSeparatorStop->setVisible(false);
            actionScripting->setVisible(false);

            GSLOG(SYSLOG_SEV_DEBUG, "Server startup skin loaded (GEX_DATATYPE_GEX_PATMAN)");
        }
        else if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        {
            strString += GEX_HTMLPAGE_HOME_GTM;
            toolBarButton->hide();
            toolBarAddress->hide();
            GSLOG(SYSLOG_SEV_DEBUG, "Server startup skin loaded (GEX_DATATYPE_GTM)");
        }
    }

    // Browser showing home page at startup time.
    LoadUrl(strString);
}

QString GexMainwindow::GetProfileLabel(const QString &startupScript)
{
    QString lProfile;
    if (!startupScript.isEmpty() && !GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        if (startupScript.endsWith(GEX_SCRIPT_CONFIG_NAME) ||
                startupScript.endsWith(GEXDB_SCRIPT_CONFIG_NAME) ||
                startupScript.endsWith(GEXWEB_SCRIPT_CONFIG_NAME) ||
                startupScript.endsWith(GEX_SCRIPT_ASSISTANT_NAME) ||
                startupScript.endsWith(GEXDB_SCRIPT_ASSISTANT_NAME) ||
                startupScript.endsWith(GEXWEB_SCRIPT_ASSISTANT_NAME) ||
                startupScript.endsWith(GTM_SCRIPT_CONFIG_NAME) ||
                startupScript.endsWith(GTM_SCRIPT_ASSISTANT_NAME) ||
                startupScript.endsWith(YM_SCRIPT_CONFIG_NAME) ||
                startupScript.endsWith(PAT_SCRIPT_CONFIG_NAME) ||
                startupScript.endsWith(YM_SCRIPT_ASSISTANT_NAME) ||
                startupScript.endsWith(PAT_SCRIPT_ASSISTANT_NAME))
            lProfile += "default";
        else if (startupScript.endsWith(GEX_SCRIPTING_CENTER_NAME))
            lProfile += "CSL";
        else
        {
            QFileInfo lCslFile(startupScript);
            lProfile += lCslFile.fileName().remove("_user_profile.csl");
        }
    }

    return lProfile;
}

void GexMainwindow::UpdateCaption()
{
    // Update application Caption name
    QString lCaption = GS::Gex::Engine::GetInstance().Get("AppFullName").toString();
    if(lCaption.isEmpty())
        lCaption = "Examinator";
    QString lStartupScript = GS::Gex::Engine::GetInstance().GetStartupScript();
    QString lProfile = GetProfileLabel(lStartupScript);
    if (!lProfile.isEmpty())
        lCaption.prepend(lProfile + " - ");

    if (mUnsavedOptions)
        lCaption.prepend("*");
    setWindowTitle(lCaption);
}

#include <unistd.h>

///////////////////////////////////////////////////////////
// Adds ToolButton in the status bar
///////////////////////////////////////////////////////////
void GexMainwindow::AddStatusBarButton(QPixmap *pPixmap)
{
    // Script status string to be seen in the status bar
    QToolButton *ptButton= new QToolButton(statusBar());
    ptButton->setIcon(QIcon(*pPixmap));
    ptButton->show();
    ptButton->setAutoRaise(true);
    ptButton->setToolTip(trUtf8("Show Status"));
    // Add the 'Network' or 'Standalone' pixmap to the tool bar
    statusBar()->addPermanentWidget(ptButton);
    statusBar()->show();
    // If user clicks on it, will display Info like: running mode, Session duration, etc...
    connect( (QObject *)ptButton, SIGNAL( clicked() ),
             this, SLOT( ViewClientInfo(void) ) );
    //Yes I know it should be a non pointer member of GexMainWindow but
    // I m doing for free from Kate's request and Qt will anyway delete it at exit...
    new QShortcut(Qt::CTRL+Qt::Key_I, this, SLOT( ViewClientInfo()));
}

void GexMainwindow::OnSaveSettingsCharts()
{
    // Creates header of Gex Configuration startup script file.
     FILE *hFile = CreateGexScript(GEX_SCRIPT_CONFIG);
    if(hFile == NULL)
    {
        GSLOG(SYSLOG_SEV_WARNING, " error in CreateGexScript(GEX_SCRIPT_CONFIG) !");
        return;
    }

    // Creates 'Preferences' section
    pWizardScripting->WritePreferencesSection(hFile);

    // Creates 'SetOptions' section
    if(!ReportOptions.WriteOptionSectionToFile(hFile))
    {
        GEX_ASSERT(false);
    }

    // Writes script footer : writes 'main' function+ call to 'Favorites'+SetOptions
    CloseGexScript(hFile,GEX_SCRIPT_SECTION_FAVORITES | GEX_SCRIPT_SECTION_OPTIONS);

    // Force reload so all changes are effective
    /// TODO: to check if we really have to load this script in the SaveEnvironnement, it should already be loaded
   // GS::Gex::CSLEngine::GetInstance().RunStartupScript(GS::Gex::Engine::GetInstance().GetStartupScript());

}

///////////////////////////////////////////////////////////
// Save Environment into script file
///////////////////////////////////////////////////////////
void GexMainwindow::OnSaveEnvironment(void)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" OnSaveEnvironment strStartupScript= '%1' ")
          .arg(GS::Gex::Engine::GetInstance().GetStartupScript()).toLatin1().data() );

    FILE *hFile=0;
    mUnsavedOptions = false;
    // Debug message (written only if gex started with -D option)
    QString strDebugMessage;
    strDebugMessage =  "for ";
    strDebugMessage += GS::Gex::Engine::GetInstance().GetStartupScript();
    strDebugMessage += " ...";
    GSLOG(SYSLOG_SEV_INFORMATIONAL, strDebugMessage.toLatin1().data());

    if (GS::Gex::OFR_Controller::GetInstance()->IsEmptyReportBuilder() && GS::Gex::OptionsHandler::sNeedToRebuild)
    {
        bool lOk;
        GS::Gex::Message::request("", "If you save these options changes, your report will be lost.\n"
                                  "This includes any unsaved Report Builder session.\n"
                                  "Are you sure you want to proceed?", lOk);
        if (! lOk)
        {
            return;
        }
    }
    // Retreives names of last file selected...so next time it is the default!
    switch(mWizardType)
    {
    case GEX_ONEFILE_WIZARD:
        // Saves latest Path selected to load files...
        pWizardScripting->strGexConfLastFilePathSelected = pWizardOneFile->strWorkingPath;
        break;
    case GEX_SHIFT_WIZARD:
    case GEX_CMPFILES_WIZARD:
        // Saves latest Path selected to load files...
        pWizardScripting->strGexConfLastFilePathSelected = pWizardCmpFiles->GetWorkingPath();
        break;
    case GEX_FT_PAT_FILES_WIZARD:
        // Saves latest Path selected to load files...
        if (mPATReportFTGui)
            pWizardScripting->strGexConfLastFilePathSelected = mPATReportFTGui->GetWorkingPath();
        else
            GSLOG(SYSLOG_SEV_WARNING, "FT PAT report gui not allocated");
        break;
    case GEX_ADDFILES_WIZARD:
        // Saves latest Path selected to load files...
        pWizardScripting->strGexConfLastFilePathSelected = pWizardAddFiles->strWorkingPath;
        break;
    case GEX_MIXFILES_WIZARD:
        // Saves latest Path selected to load files...
        pWizardScripting->strGexConfLastFilePathSelected = pWizardMixFiles->strWorkingPath;
        break;
    case GEX_JASPER_WIZARD:
    case GEX_CHAR_QUERY_WIZARD:
    case GEX_ONEQUERY_WIZARD:
    case GEX_CMPQUERY_WIZARD:
    case GEX_MIXQUERY_WIZARD: // check me : does the "merge datasets" have a working path ?
        // Saves latest Path selected to load files...
        pWizardScripting->strGexConfLastFilePathSelected = "";
        break;
    default:
        GEX_ASSERT(false);
        GSLOG(SYSLOG_SEV_WARNING, "Invalid wizard type");
        break;

    }

    // Creates header of Gex Configuration startup script file.
    hFile = CreateGexScript(GEX_SCRIPT_CONFIG);
    if(hFile == NULL)
    {
        GSLOG(SYSLOG_SEV_WARNING, " error in CreateGexScript(GEX_SCRIPT_CONFIG) !");
        return;
    }

    // Creates 'Preferences' section
    pWizardScripting->WritePreferencesSection(hFile);

    // Creates 'SetOptions' section
    if(!ReportOptions.WriteOptionSectionToFile(hFile))
    {
        GEX_ASSERT(false);
    }

    // Writes script footer : writes 'main' function+ call to 'Favorites'+SetOptions
    CloseGexScript(hFile,GEX_SCRIPT_SECTION_FAVORITES | GEX_SCRIPT_SECTION_OPTIONS);

    if(GS::Gex::OptionsHandler::sNeedToRebuild)
    {
        // GCORE-7145
        mWizardsHandler.RemoveAllWizardSExcept(NULL);

        //-- reset the report builder if any
        OFRManager::GetInstance()->Reset();

        // Force reload so all changes are effective
        /// TODO: to check if we really have to load this script in the SaveEnvironnement, it should already be loaded
        GS::Gex::CSLEngine::GetInstance().RunStartupScript(GS::Gex::Engine::GetInstance().GetStartupScript());
    }
    else
    {
        // retrieve the otption
        GS::Gex::OptionsHandler optionsHandler(GS::Gex::Engine::GetInstance().GetOptionsHandler());
        optionsHandler.UpdateReportOption(ReportOptions);
        GS::Gex::Engine::GetInstance().SetOptionsMap(optionsHandler.GetOptionsMap());
        mUnsavedOptions = false;
        UpdateCaption();
    }

    // Admin Server
    // Update the database with current files saved
    // Synchronize with YieldManDB profiles
    if(GS::Gex::Engine::GetInstance().IsAdminServerMode(true))
        GS::Gex::Engine::GetInstance().GetAdminEngine().SynchronizeProfiles(GS::Gex::Engine::GetInstance().GetDefaultUserProfile());

    // reset the flag
    GS::Gex::OptionsHandler::sNeedToRebuild = false;

    GSLOG(SYSLOG_SEV_INFORMATIONAL, " ok.");
}


///////////////////////////////////////////////////////////
// Show Hide Browser / Assistants
///////////////////////////////////////////////////////////
void GexMainwindow::ShowHtmlBrowser(bool bBrowser/*=true*/)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString(" bBrowser = %1").arg(bBrowser?"browser":"no browser").toLatin1().data() );
    if(bBrowser == true)
    {
        // Show Web browser
        m_pWebViewReport->widget()->show();

        stackedWidget->setCurrentIndex(0);

        // Hide all assistants
        pScrollArea->hide();
        // ...including OpenGL window that is not child of the scroll view
        // if(LastCreatedWizardWafer3D() != NULL && LastCreatedWizardWafer3D()->bChildWindow)
        //    LastCreatedWizardWafer3D()->hideWindows();
    }
    else
    {
        stackedWidget->setCurrentIndex(1);

        // Show assistants
        pScrollArea->show();

        // Note: OpenGL page only shown when specifically addressed..
    }
}


void GexMainwindow::OnNewWizard(int typeOfWizard, const QString& link)
{
    if(mQDetachableTabWindowContainer && mQDetachableTabWindowContainer->isHidden() == false )
    {
        if(mCanCreateNewDetachableWindow)
        {
            mCanCreateNewDetachableWindow = false;
            if(typeOfWizard == GEX_TABLE_WIZARD_P1)
            {
                Wizard_DrillTable("", false);
            }
            else if(typeOfWizard == GEX_DRILL_3D_WIZARD_P1)
            {
                Wizard_Drill3D(link, false);
            }
            else if(typeOfWizard == GEX_CHART_WIZARD_P1)
            {
                Wizard_DrillChart(link, false);
            }
            mCanCreateNewDetachableWindow = true;
        }
    }
}

/*void GexMainwindow::keyPressEvent(QKeyEvent * event)
{
    if(mQDetachableTabWindowContainer && mQDetachableTabWindowContainer->isHidden() == false )
    {
          if(event->key() == Qt::Key_T && QApplication::keyboardModifiers() && Qt::ControlModifier)
          {
              Wizard_DrillTable("", false);
              //event->accept();
          }
          else if(event->key() == Qt::Key_W && QApplication::keyboardModifiers() && Qt::ControlModifier)
          {
              Wizard_Drill3D("#_gex_drill--drill_3d=wafer_sbin--g=0--f=0", false);
              //event->accept();
          }
          else if(event->key() == Qt::Key_D && QApplication::keyboardModifiers() && Qt::ControlModifier)
          {
              Wizard_DrillChart("", false);
             //event->accept();
          }
          else
          {
           //   event->ignore();
          }
    }
    else
    {
      // event->ignore();
    }

    QMainWindow::keyPressEvent(event);
}*/

template<typename Wizard>
void GexMainwindow::AddWizardInDetachableWindow(Wizard* wizard, const QString& label)
{
    if(mForwardBackAction == false)
    {
        wizard->setWizardHandler(&mWizardsHandler);

        wizard->setAttribute(Qt::WA_DeleteOnClose);
        if(mQDetachableTabWindowContainer == 0)
        {
            mQDetachableTabWindowContainer = new QDetachableTabWindowContainer();
            pScrollArea->layout()->addWidget(mQDetachableTabWindowContainer );
            mWizardsHandler.setQDetachableTabWindowContainer(mQDetachableTabWindowContainer);
        }

       // -- QDetachableTabWindowContainer does not have a main window
       // -- next add will create it. The wizard handler must be reset reagarding this fact
       if(mQDetachableTabWindowContainer->IsFirstWindowEmpty())
            mWizardsHandler.SetMainWindowAlreadyReset(false);

        GS::Gex::QTitleFor<QDialog>*
            widgetToSwitch = mWizardsHandler.GetWidgetToSwitch();
        if (widgetToSwitch)
        {
            if (widgetToSwitch->isCustomTitle())
            {
                wizard->setTitle(widgetToSwitch->getTitle());

                wizard->setCustomTitleFlag();
            }

            mQDetachableTabWindowContainer->
                replaceWidget(widgetToSwitch, wizard, label);
            mWizardsHandler.SetWidgetToSwitch(NULL);  // GCORE-10303
        }
        else
        {
            mQDetachableTabWindowContainer->
                addDetachableTabInActiveWindow(wizard, label);
        }

       // connect some slot here, after widget is attached in container
       QObject::connect
         (
           mWizardsHandler.GetDetachableWindowContainer(),
           SIGNAL( tabRenamed( QWidget*, const QString & ) ),
           wizard, SLOT( OnTabRenamed( QWidget*, const QString & ) )
         );
    }
    mForwardBackAction = false;
    mQDetachableTabWindowContainer->show();
}

void GexMainwindow::ShowWizardDialog(int iPageID, bool fromHTLBrowser )
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Show dialog page %1...").arg(iPageID).toLatin1().data() );

    // Save type of wizard page to show.
    iWizardPage = iPageID;

    // if wizard page is already detached, just bring it up in front
    switch(iWizardPage)
    {
    case GEXTB_CONVERT_STDF_WIZARD_P1:
    case GEXTB_CONVERT_CSV_WIZARD_P1:
    case GEXTB_CONVERT_PAT_WIZARD_P1:
    case GEXTB_EDIT_CSV_WIZARD_P1:
        if (pGexTbTools != NULL && pGexTbTools->mChildWindow == false)
        {
            pGexTbTools->showWindows();
            return;
        }
        break;

    case GEXTB_EDIT_PAT_WIZARD_P1:
#ifdef GCORE15334
        if (GS::Gex::PATRecipeEditor::GetInstance().parent() == NULL)
            GS::Gex::PATRecipeEditor::GetInstance().show();
#endif
        break;
    default:
        break;
    }

    // Hide.
    pWizardSettings->hide();
    pWizardOneFile->hide();
    pWizardCmpFiles->hide();
    pWizardAddFiles->hide();
    pWizardMixFiles->hide();
    pWizardOneQuery->hide();  // Single Query
    pWizardCmpQueries->hide();  // Compare Queries
    pWizardAdminGui->hide(); // Database Admin

    if (m_pReportsCenter)
        m_pReportsCenter->hide();
    if (m_pOptionsCenter)
        m_pOptionsCenter->hide();
    if(mMonitoringWidget)
        mMonitoringWidget->hide();
    if(pWizardSchedulerGui)
        pWizardSchedulerGui->hide();
    if(m_pOptionsCenter)
        m_pOptionsCenter->hide();

     pWizardScripting->hide();

#ifdef GCORE15334

    if(mGtmWidget)
        mGtmWidget->hide();

    // Recipe editor (only hide if it is a child window not detached)
    if (GS::Gex::PATRecipeEditor::GetInstance().IsInstantiated() &&
            GS::Gex::PATRecipeEditor::GetInstance().parent())
        GS::Gex::PATRecipeEditor::GetInstance().hide();
#endif
    // Examinator ToolBox (only hide it if it's a child window not detached from Examinator)
    if(pGexTbTools != NULL && pGexTbTools->mChildWindow)
        pGexTbTools->hide();

    // Delete any Interactive window if exist...
    if(pWizardWhatIf != NULL && pWizardWhatIf->bChildWindow)
        pWizardWhatIf->hide();
#ifdef GCORE15334

    // Hide 'WS PAT: Process file' wizard
    if(mWizardWSPAT != NULL)
        mWizardWSPAT->hide();

    // Hide 'FT PAT: Process file' wizard
    if(mWizardFTPAT != NULL)
        mWizardFTPAT->hide();
#endif
    // Hide 'FT PAT Report from file' wizard
    if(mPATReportFTGui != NULL)
        mPATReportFTGui->hide();

    if(mQDetachableTabWindowContainer)
        mQDetachableTabWindowContainer->hide();

    if(fromHTLBrowser)
        fromHTLBrowser =true;

    // if(fromHTLBrowser)
    {
        // Checks if it is a Wizard dialog box, or the HTML browser!
        if(iPageID == GEX_BROWSER)
            ShowHtmlBrowser(true);
        else
            ShowHtmlBrowser(false); // Show assitants
    }


    // If Wizard dialog, see which one to show...
    switch(iPageID)
    {
    case GEX_BROWSER:
        break;
    case GEX_OPTIONS:
    case GEX_OPTIONS_CENTER:
        if (m_pOptionsCenter)
            m_pOptionsCenter->show();
        else
            GSLOG(SYSLOG_SEV_WARNING, "Options Center not initialized");
        break;
    case GEX_GTM_TESTERS_WIDGET:
#ifdef GCORE15334

        if (mGtmWidget)
            mGtmWidget->show();
        else
#endif
            GSLOG(SYSLOG_SEV_WARNING, "GTM widget NULL");
        break;

    case GEX_LOGS_CENTER:
        if (m_pLogsCenter)
            m_pLogsCenter->show();
        else
            GSLOG(SYSLOG_SEV_ERROR, "LogsCenter widget NULL");
        break;
    case GEX_JAVASCRIPT_CENTER:
        if (!m_pJavaScriptCenter)
        {
            m_pJavaScriptCenter=new JavaScriptCenter(0);
            //pScrollArea->layout()->addWidget(m_pJavaScriptCenter);
        }
        m_pJavaScriptCenter->showMaximized();
        break;
    case GEX_SQLBROWSER:
        if (m_pSqlBrowser)
        {
            //RefreshDB();
            emit RefreshDB();
            m_pSqlBrowser->show();
        }
        break;
    case GEX_WIZARD_SETTINGS:
        pWizardSettings->ShowPage();
        break;
    case GEX_ONEFILE_WIZARD_P1:
        pWizardOneFile->ShowPage();
        break;
    case GEX_CMPFILES_WIZARD_P1:
        pWizardCmpFiles->SetMode(GexCmpFileWizardPage1::COMPARE);
        pWizardCmpFiles->ShowPage();
        break;
    case GEX_SHIFT_WIZARD_P1:
        pWizardCmpFiles->SetMode(GexCmpFileWizardPage1::SHIFT);
        pWizardCmpFiles->ShowPage();
        break;
    case GEX_ADDFILES_WIZARD_P1:
        pWizardAddFiles->ShowPage();
        break;
    case GEX_MIXFILES_WIZARD_P1:
        pWizardMixFiles->ShowPage();
        break;
    case GEX_ONEQUERY_WIZARD_P1:
    case GEX_ENTERPRISE_SQL_P1:
        pWizardOneQuery->ShowPage();
        break;

    case GEX_REPORTS_CENTER:
        if (m_pReportsCenter)
            m_pReportsCenter->ShowPage();
        break;

    case GEX_CMPQUERY_WIZARD_P1:
        pWizardCmpQueries->ShowPage();
        break;
    case GEX_MIXQUERY_WIZARD_P1:
        pWizardCmpQueries->ShowPage();
        break;

    case GEX_DATABASE_ADMIN:
        if (pWizardAdminGui)
        {
            pWizardAdminGui->ShowPage();
            if(mMonitoringWidget && (pWizardAdminGui->tabWidgetAdminGui->indexOf(mMonitoringWidget) == -1))
                pWizardAdminGui->tabWidgetAdminGui->addTab(
                            mMonitoringWidget,
                            QIcon(QPixmap(QString::fromUtf8(":/gex/icons/mo_logs.png"))),QString("Admin Logs"));
            if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
            {
                if(pWizardSchedulerGui && (pWizardAdminGui->tabWidgetAdminGui->indexOf(pWizardSchedulerGui) == -1))
                    pWizardAdminGui->tabWidgetAdminGui->insertTab(
                                1,pWizardSchedulerGui,
                                QIcon(QPixmap(QString::fromUtf8(":/gex/icons/mo_scheduler.png"))),"Tasks Scheduler");
            }
            // Select tab
            pWizardAdminGui->tabDatabases->show();
            pWizardAdminGui->tabWidgetAdminGui->setCurrentWidget(pWizardAdminGui->tabDatabases);
            pWizardAdminGui->mDisplayWindowOnError = false;
            pWizardAdminGui->OnDetachWindow(false);
        }
        break;

    case GEX_SCRIPTING:
        if (pWizardScripting)
            pWizardScripting->show();
        else
            GSLOG(SYSLOG_SEV_WARNING, "Cannot show Scripting center because null");
        break;

    case GEXMO_TASKS:
        if (pWizardAdminGui)
        {
            pWizardAdminGui->ShowPage();
            // Select tab
            if(mMonitoringWidget && (pWizardAdminGui->tabWidgetAdminGui->indexOf(mMonitoringWidget) == -1))
                pWizardAdminGui->tabWidgetAdminGui->addTab(mMonitoringWidget,
                                                           QIcon(QPixmap(QString::fromUtf8(":/gex/icons/mo_logs.png"))),QString("Admin Logs"));
            if(pWizardSchedulerGui)
            {
                if (pWizardAdminGui->tabWidgetAdminGui->indexOf(pWizardSchedulerGui) == -1)
                    pWizardAdminGui->tabWidgetAdminGui->insertTab(1,pWizardSchedulerGui,
                                                                  QIcon(QPixmap(QString::fromUtf8(":/gex/icons/mo_scheduler.png"))),"Tasks Scheduler");

                pWizardSchedulerGui->OnSelectTab(0);
                pWizardSchedulerGui->show();

                pWizardAdminGui->tabWidgetAdminGui->setCurrentWidget(pWizardSchedulerGui);
                pWizardAdminGui->mDisplayWindowOnError = false;
                pWizardAdminGui->OnDetachWindow(false);
            }
        }
        break;

    case GEXMO_HISTORY:
        if(mMonitoringWidget == NULL)
        {
            mMonitoringWidget = new MonitoringWidget(centralWidget());
            pScrollArea->layout()->addWidget(mMonitoringWidget);

            // Signal to tell Admin tab that a MO history log file has been modified
            connect(&GS::Gex::Engine::GetInstance().GetSchedulerEngine(),
                    SIGNAL(sMoHistoryLogUpdated(const QString &,const QString &)),
                    mMonitoringWidget,
                    SLOT(OnMoHistoryLogUpdated(const QString &,const QString &)));

            // Signal to tell Admin tab that a MO report log file has been modified
            connect(&GS::Gex::Engine::GetInstance().GetSchedulerEngine(),
                    SIGNAL(sMoReportLogUpdated(const QString &,const QString &)),
                    mMonitoringWidget,
                    SLOT(OnMoReportLogUpdated(const QString &,const QString &)));
        }

        if (pWizardAdminGui)
        {
            pWizardAdminGui->ShowPage();
            // Select tab
            if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
            {
                if(pWizardSchedulerGui && (pWizardAdminGui->tabWidgetAdminGui->indexOf(pWizardSchedulerGui) == -1))
                    pWizardAdminGui->tabWidgetAdminGui->insertTab(
                                1,pWizardSchedulerGui,QIcon(QPixmap(QString::fromUtf8(":/gex/icons/mo_scheduler.png"))),"Tasks Scheduler");
            }
            if(mMonitoringWidget && (pWizardAdminGui->tabWidgetAdminGui->indexOf(mMonitoringWidget) == -1))
                pWizardAdminGui->tabWidgetAdminGui->addTab(mMonitoringWidget,QIcon(QPixmap(QString::fromUtf8(":/gex/icons/mo_logs.png"))),QString("Admin Logs"));
            mMonitoringWidget->show();
            pWizardAdminGui->tabWidgetAdminGui->setCurrentWidget(mMonitoringWidget);
            pWizardAdminGui->mDisplayWindowOnError = false;
            pWizardAdminGui->OnDetachWindow(false);
        }
        break;
    case GEX_DRILL_3D_WIZARD_P1:
    {
        if(fromHTLBrowser && mWizardsHandler.Wafer3DWizards().count() > 0
                && GS::LPPlugin::ProductInfo::getInstance()->isExaminator() == false)
        {
             mQDetachableTabWindowContainer->show();

             //-- if last viewed does not exists anymore, used the last created
             Gex::DrillDataMining3D* lLastViewed = mWizardsHandler.GetLastWafer3DViewed();
             if(lLastViewed == 0)
                lLastViewed = mWizardsHandler.Wafer3DWizards()[0];
             mQDetachableTabWindowContainer->focusOnTab(lLastViewed);
        }
        else
        {
            //-- only on tab at the time for GEX product, so we clear the previous
            if(GS::LPPlugin::ProductInfo::getInstance()->isExaminator())
                mWizardsHandler.Clear();

            Gex::DrillDataMining3D* lWafer3D = mWizardsHandler.CreateWizardWaferMap3D();
            if(lWafer3D)
            { 
                AddWizardInDetachableWindow(lWafer3D, "");
            }
        }
        break;
    }
    case GEX_CHART_WIZARD_P1:
    {
        if(fromHTLBrowser && mWizardsHandler.ChartWizards().count() > 0
                 && GS::LPPlugin::ProductInfo::getInstance()->isExaminator() == false)
        {
            //mWizardsHandler.ChartWizards()[0]->show();
             mQDetachableTabWindowContainer->show();

             //-- if last viewed does not exists anymore, used the last created
             GexWizardChart* lLastViewed = mWizardsHandler.GetLastChartViewed();
             if(lLastViewed == 0)
                lLastViewed = mWizardsHandler.ChartWizards()[0];
             mQDetachableTabWindowContainer->focusOnTab(lLastViewed);
        }
        else
        {
            //-- only on tab at the time for GEX product, so we clear the previous
            if(GS::LPPlugin::ProductInfo::getInstance()->isExaminator())
                mWizardsHandler.Clear();

            GexWizardChart* lChart = mWizardsHandler.CreateWizardChart();
            if(lChart)
            {
                AddWizardInDetachableWindow(lChart, "");
            }
        }
        break;
    }
    case GEX_TABLE_WIZARD_P1:
    {
        if(fromHTLBrowser && mWizardsHandler.TableWizards().count() > 0
                && GS::LPPlugin::ProductInfo::getInstance()->isExaminator() == false)
        {
             mQDetachableTabWindowContainer->show();

             //-- if last viewed does not exists anymore, used the last created
             GexWizardTable* lLastViewed = mWizardsHandler.GetLastTableViewed();
             if(lLastViewed == 0)
                lLastViewed = mWizardsHandler.TableWizards()[0];
             mQDetachableTabWindowContainer->focusOnTab(lLastViewed);
        }
        else
        {
            //-- only on tab at the time for GEX product, so we clear the previous
            if(GS::LPPlugin::ProductInfo::getInstance()->isExaminator())
                mWizardsHandler.Clear();

            GexWizardTable* lTable = mWizardsHandler.CreateWizardTable(0);
            if(lTable)
                AddWizardInDetachableWindow(lTable, "Table"); //-- no test number for the tab header. Need to put something
        }
        break;
    }
    case GEX_DRILL_WHATIF_WIZARD_P1:
        // Create Drill guard banding page
        if(pWizardWhatIf == NULL)
        {
            pWizardWhatIf = new GexDrillWhatIf(centralWidget());
            pScrollArea->layout()->addWidget(pWizardWhatIf);
            // Enable 'Compute' button
            connect( (QObject *)pWizardWhatIf->buttonCompute, SIGNAL( clicked() ),
                     pWizardWhatIf, SLOT( OnComputeWhatIf(void) ) );
        }
        else
        {
            // Update table content + colors...in case report rebuilt with what-if data.
            pWizardWhatIf->OnUpdateTableColors();
        }

        // Display Drill-Guard banding page
        pWizardWhatIf->ShowPage();

        // First: Show report page
        ReportLink();

        // Second: Detach window! (keep last otherwise generates CRASH!)
        pWizardWhatIf->ForceAttachWindow(false);
        break;
#ifdef GCORE15334
    case GEX_WS_PAT_PROCESS_WIZARD_P1:
        if(mWizardWSPAT == NULL)
        {
            mWizardWSPAT = new GexTbPatDialog(centralWidget());
            pScrollArea->layout()->addWidget(mWizardWSPAT);
            if (pGexScriptEngine)
            {
                QScriptValue sv = pGexScriptEngine->newQObject((QObject*) mWizardWSPAT);
                if (!sv.isNull())
                    pGexScriptEngine->globalObject().setProperty("GSToolboxPat", sv);
            }
        }
        mWizardWSPAT->show();
        break;

    case GEX_FT_PAT_PROCESS_WIZARD_P1:
        if(mWizardFTPAT == NULL)
        {
            mWizardFTPAT = new GS::Gex::PATWidgetFT(centralWidget());
            pScrollArea->layout()->addWidget(mWizardFTPAT);
        }
        mWizardFTPAT->show();
        break;
    case GEXTB_EDIT_PAT_WIZARD_P1:
        if (! GS::Gex::PATRecipeEditor::GetInstance().parent())
        {
            GS::Gex::PATRecipeEditor::GetInstance().setParent(centralWidget());
            pScrollArea->layout()->addWidget(&GS::Gex::PATRecipeEditor::GetInstance());
        }
        GS::Gex::PATRecipeEditor::GetInstance().show();
        break;
#endif

    case GEX_FT_PAT_FILES_WIDGET:
        if(mPATReportFTGui == NULL)
        {
            mPATReportFTGui = new GS::Gex::PATReportFTGui(centralWidget());
            pScrollArea->layout()->addWidget(mPATReportFTGui);
        }
        mPATReportFTGui->show();
        break;

    case GEXTB_CONVERT_STDF_WIZARD_P1:
    case GEXTB_CONVERT_CSV_WIZARD_P1:
    case GEXTB_CONVERT_PAT_WIZARD_P1:
    case GEXTB_EDIT_CSV_WIZARD_P1:
        if(pGexTbTools == NULL)
        {
            pGexTbTools = new GexTbToolBox(centralWidget());
            pScrollArea->layout()->addWidget(pGexTbTools);
        }
        // Display wizard + update the fields accoring to the conversion type (STDF, CSV)
        pGexTbTools->showWindows();
        break;
    }

    QCoreApplication::processEvents();
}

///////////////////////////////////////////////////////////
// Resizes ALL Wizard dialog to use ALL Browser space
///////////////////////////////////////////////////////////
void GexMainwindow::ResizeWizardDialog(int width,int height)
{
    int iMinWidth;

    iMinWidth = (width < 900) ? 900: width;

    // Resize to client size
    pScrollArea->resize(iMinWidth,height);

    // If screen resolution is very small, scroll bar will be shown instead!
    if(iScreenSize != GEX_SCREENSIZE_LARGE)
        return;

    // Wizard pages that are not allowed to be smaller than 900 pixels:
    pWizardOneFile->resize(iMinWidth,height);
    pWizardCmpFiles->resize(iMinWidth,height);
    pWizardAddFiles->resize(iMinWidth,height);
    pWizardMixFiles->resize(iMinWidth,height);
    pWizardOneQuery->resize(iMinWidth,height);
    pWizardCmpQueries->resize(iMinWidth,height);
    pWizardAdminGui->resize(iMinWidth,height);
    pWizardSettings->resize(iMinWidth,height);
    pWizardScripting->resize(iMinWidth,height);

    // Wizard pages that resize with application
    if(m_pOptionsCenter)
        m_pOptionsCenter->resize(width, height);

    if(pWizardWhatIf != NULL && pWizardWhatIf->bChildWindow)
        pWizardWhatIf->resize(iMinWidth,height);
#ifdef GCORE15334

    if(mWizardWSPAT != NULL)
        mWizardWSPAT->resize(iMinWidth,height);

    if(mWizardFTPAT != NULL)
        mWizardFTPAT->resize(iMinWidth,height);
#endif
    if(mPATReportFTGui != NULL)
        mPATReportFTGui->resize(iMinWidth,height);

    // ExaminatorMonitoring
    if(pWizardSchedulerGui != NULL)
        pWizardSchedulerGui->resize(iMinWidth,height);
    if(mMonitoringWidget != NULL)
        mMonitoringWidget->resize(iMinWidth,height);

    // Examinator ToolBox
    if(pGexTbTools != NULL && pGexTbTools->mChildWindow)
    {
        pGexTbTools->resize(iMinWidth,height);
        pGexTbTools->mExcelTable->resize(iMinWidth-2,height-85);
    }
}

///////////////////////////////////////////////////////////
// New HTML report page viewed: keep track of history.
///////////////////////////////////////////////////////////
void GexMainwindow::AddNewUrl(QString selectedURL)
{
    if (selectedURL.isEmpty())
        return;

    GSLOG(SYSLOG_SEV_DEBUG, QString("Add New Url %1").arg( selectedURL).toLatin1().constData() );

    int i=0;

    // Check if already exists in the combo list.
    bool exists = false;
    for ( i = 0; i < GexURL->count(); ++i )
    {
        if ( GexURL->itemText( i ) == selectedURL )
        {
            exists = true;
            break;
        }
    }
    if ( exists == false)
    {
        // This is a new page: add it to the combo list!
        GexURL->insertItem(0, selectedURL );
        GexURL->setCurrentIndex(0);
    }
    else
    {
        // This path is already in the combo list, so just select it!
        if (GexURL->currentIndex() != i)
            GexURL->setCurrentIndex( i );
    }
}

///////////////////////////////////////////////////////////
// New HTML report page viewed: keep track of history.
///////////////////////////////////////////////////////////
void GexMainwindow::AddNewUrl(const QUrl& srcUrl)
{
    // Name of new HTML page listed.
    QString selectedURL = srcUrl.toLocalFile();
    if (selectedURL.isEmpty())
        selectedURL=srcUrl.toString();

    if (srcUrl.fragment().isNull() == false)
        selectedURL += "#" + srcUrl.fragment();

    AddNewUrl(selectedURL);
}

///////////////////////////////////////////////////////////
// User selected a Url from the History combo. Reload
// the selected page.
///////////////////////////////////////////////////////////
void GexMainwindow::ViewNewUrl()
{
    LoadUrl(GexURL->currentText());
}

void GexMainwindow::Wizard_FT_PAT_FileAnalysis(void)
{
    if(!(GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT()) &&
            !(GS::LPPlugin::ProductInfo::getInstance()->isPATMan()))
    {

        QString lErrorMsg = ReportOptions.GetOption("messages", "upgrade").toString();
        GS::Gex::Message::information("", lErrorMsg);
        return;
    }

    // Once wizard launches, triggers a report type.
    // GCore-17482
    // Update the Wizard type before the OnDoInstantReport
    SetWizardType(GEX_FT_PAT_FILES_WIZARD);

    pWizardSettings->OnDoInstantReport();

    // Show wizard page Settings
    ShowWizardDialog(GEX_FT_PAT_FILES_WIDGET);
}

///////////////////////////////////////////////////////////
// STOP icon clicked: stop processing query, if any...
///////////////////////////////////////////////////////////
void GexMainwindow::OnStop()
{
    GSLOG(SYSLOG_SEV_DEBUG, "On stop...");

    // Sets ABORT variable to true
    GS::Gex::CSLEngine::GetInstance().AbortScript();
}

// -------------------------------------------------------------------------- //
// OnSnapshot
// -------------------------------------------------------------------------- //
void GexMainwindow::OnSnapshot()
{
    QIcon copy_icon;
    QIcon save_icon;
    copy_icon.addPixmap(*pixCopy);
    save_icon.addPixmap(*pixSave);

    QMenu menu;
    QAction* copy_item = menu.addAction(copy_icon, "Screen snapshot to Clipboard");
    QAction* save_item = menu.addAction(save_icon, "Save screen snapshot to disk");

    menu.setMouseTracking(true);
    QAction* action = menu.exec(QCursor::pos());
    menu.setMouseTracking(false);

    if      (action == copy_item) { this->SnapshotCopy(); }
    else if (action == save_item) { this->SnapshotSave(); }
}

// -------------------------------------------------------------------------- //
// SnapshotCopy
// -------------------------------------------------------------------------- //
void GexMainwindow::SnapshotCopy()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "SnapshotCopy...");

    QWidget* lActiveWidget = QApplication::activeWindow();
    if (!lActiveWidget)
    {
        GSLOG(SYSLOG_SEV_WARNING, "Cannot find the active window");
        return;
    }
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Current active window= %1").arg(lActiveWidget->objectName())
          .toLatin1().data() );

    QPixmap pixmapWindow;

    if (lActiveWidget->isVisible())
    {
        //if (active_widget==pWizardDrill3D)
        //  GSLOG(5, "");
        lActiveWidget->repaint();
        // grabWindow is deprecated
        pixmapWindow = QPixmap(QPixmap::grabWindow(lActiveWidget->winId()));
    }
    else
    {
        pixmapWindow = QPixmap(QPixmap::grabWindow(m_pWebViewReport->widget()->winId()));
    }

    QClipboard* lClipBoard = QGuiApplication::clipboard();
    if (lClipBoard)
    {
        lClipBoard->setPixmap(pixmapWindow, QClipboard::Clipboard);
    }
    else
    {
        GSLOG(SYSLOG_SEV_WARNING, "Cannot retieve application clipboard");
    }
}

// -------------------------------------------------------------------------- //
// SnapshotSave
// -------------------------------------------------------------------------- //
void GexMainwindow::SnapshotSave()
{
    QWidget* lActiveWidget = QApplication::activeWindow();
    QPixmap pixmapWindow;

    if (lActiveWidget->isVisible()) {
        lActiveWidget->repaint();
        pixmapWindow =
                QPixmap(QPixmap::grabWindow(lActiveWidget->winId()));
    } else {
        pixmapWindow =
                QPixmap(QPixmap::grabWindow(m_pWebViewReport->widget()->winId()));
    }

    QString strFile =
            QFileDialog::getSaveFileName(this, "Save screen as...", "image",
                                         "PNG (*.png)", NULL,
                                         QFileDialog::DontConfirmOverwrite);

    // If no file selected, ignore command.
    if (strFile.isEmpty() == false) {
        if (strFile.endsWith(".png", Qt::CaseInsensitive) == false)
            strFile += ".png";  // make sure file extension is .png
        // Check if file exists.
        QFile f(strFile);
        bool bCreateFile = true;
        if (f.exists() == true) {
            // File exists...overwrite it?
            bool lOk;
            GS::Gex::Message::request("", "File already exists. Overwrite it ?", lOk);
            if (! lOk)
            {
                bCreateFile = false;  // No!...keep file, do not overwrite it!
            }
        }
        if (bCreateFile == true) {
            pixmapWindow.save(strFile, "PNG");
        }
    }
}

///////////////////////////////////////////////////////////
// HOME icon clicked: go back to home page (GEX wizards)
///////////////////////////////////////////////////////////
void GexMainwindow::OnHome()
{
    // Show Web Browser, hide assistants
    ShowHtmlBrowser(true);
    ReloadProductSkin(false);
    AddNewUrl(GEX_HTMLPAGE_HOME);
}

///////////////////////////////////////////////////////////
// Load a HTML file into the browser window
///////////////////////////////////////////////////////////
void GexMainwindow::OnBrowseUrl(QString strHtmlFile)
{
    // Unselect all tabs on navigation bar!
    QString strTopBarPage = navigationSkinDir() + "pages/" + GEX_BROWSER_ACTIONLINK;

    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        strTopBarPage += GEXMO_BROWSER_ELSE_TOPLINK;
    else
        strTopBarPage += GEX_BROWSER_ELSE_TOPLINK;

    pageHeader->setSource(QUrl::fromLocalFile(strTopBarPage));

    // Ensure HTML page is visible...then reload it with the selected page!
    // Show Web Browser, hide assistants
    LoadUrl(strHtmlFile);
}

///////////////////////////////////////////////////////////
// Action trigerred by the web browser
///////////////////////////////////////////////////////////
void GexMainwindow::onBrowserAction(const GexWebBrowserAction& webBrowserAction)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("onBrowserAction %1 : %2 (from url %3)...")
          .arg(webBrowserAction.action())
          .arg(webBrowserAction.link().toLatin1().data())
          .arg(webBrowserAction.m_DesiredUrl.toString()).toLatin1().constData() );
    // Save URL link.
    if (webBrowserAction.action() != GexWebBrowserAction::CsvAction)
        m_selectedURL = webBrowserAction.link();

    switch(webBrowserAction.action())
    {
    case GexWebBrowserAction::HtmlAction : ShowHtmlBrowser();
        break;

    case GexWebBrowserAction::BookmarkDrill3dAction :
    case GexWebBrowserAction::BookmarkDrillTableAction :
    case GexWebBrowserAction::BookmarkDrillChartAction :
    case GexWebBrowserAction::BookmarkERAction :
    case GexWebBrowserAction::BookmarkAdvERAction :
        onBrowserActionBookmark(webBrowserAction);
        break;

    case GexWebBrowserAction::LinkAction :
        onBrowserActionLink(webBrowserAction);        //webBrowserAction.link(), webBrowserAction.index());
        break;

    case GexWebBrowserAction::CsvAction :
        onBrowserActionCsv(webBrowserAction.link());
        break;

    case GexWebBrowserAction::OpenUrlAction :
        onBrowserActionOpenUrl(webBrowserAction.link());
        break;

    default:
        GSLOG(SYSLOG_SEV_WARNING, QString("on Browser action : action %1 not handled !").arg( webBrowserAction.action()).toLatin1().constData());
        break;
    }

    GSLOG(SYSLOG_SEV_DEBUG, "onBrowserAction ok");
}

void GexMainwindow::UpdateProgress(int prog)
{
    if(GexProgressBar)
        GexProgressBar->setValue(prog);
}

void GexMainwindow::ResetProgress(bool forceCompleted)
{
    int lProgress;
    if(GexProgressBar)
    {
        if(forceCompleted == true)
            lProgress = 100;
        else
        {
            lProgress = 0;
            GexProgressBar->setMaximum(100);
            GexProgressBar->setTextVisible(true);
            GexProgressBar->show();
        }
        GexProgressBar->setValue(lProgress);
    }
}

void GexMainwindow::SetMaxProgress(int max)
{
    if(GexProgressBar)
        GexProgressBar->setMaximum(max);
}

void GexMainwindow::SetBusyness(bool isBusy)
{
    if (isBusy)
        QGuiApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
    else
        QGuiApplication::restoreOverrideCursor();
}


void GexMainwindow::onBrowserActionBookmark(const GexWebBrowserAction& webBrowserAction)
{
    mForwardBackAction = webBrowserAction.forwardBack();
    switch (webBrowserAction.action())
    {
    case GexWebBrowserAction::BookmarkDrill3dAction  : Wizard_Drill3D(webBrowserAction.link());
        break;

    case GexWebBrowserAction::BookmarkDrillTableAction : Wizard_DrillTable(webBrowserAction.link());
        break;

    case GexWebBrowserAction::BookmarkDrillChartAction : Wizard_DrillChart(webBrowserAction.link());
        break;

    case GexWebBrowserAction::BookmarkERAction   : if (gexReport)
            gexReport->ProcessActionLink_EnterpriseReports(webBrowserAction.link());
        break;

    case GexWebBrowserAction::BookmarkAdvERAction  : if (gexReport)
            gexReport->ProcessActionLink_AdvEnterpriseReports(webBrowserAction.link());
        break;

    default            : break;
    }
}

///////////////////////////////////////////////////////////
// Csv file link has been clicked
///////////////////////////////////////////////////////////
void GexMainwindow::onBrowserActionCsv(const QString& strCsvFile)
{
    // Start Toolbox 'Spreadsheet editor' task
    Wizard_GexTb_Edit();

    // Load CSV file into Toolbox editor
    pGexTbTools->LoadExcelTable(strCsvFile);

    // Detach Toolbox
    pGexTbTools->ForceAttachWindow(false);
}

///////////////////////////////////////////////////////////
// External link url has been clicked
///////////////////////////////////////////////////////////
void GexMainwindow::onBrowserActionOpenUrl(const QString& strUrl)
{
    if (!QDesktopServices::openUrl(QUrl(strUrl)))
    {
        QString strMessage =  QString("Error opening url : %1.\nMake sure you have a internet connection.").arg(strUrl);

        GS::Gex::Message::warning("Open external url", strMessage);
    }

}

///////////////////////////////////////////////////////////
// Browse button selected...let's user select a HTML file
///////////////////////////////////////////////////////////
void GexMainwindow::OnBrowseUrl()
{
    char szString[2048];

    // User wants to load a HTML page from
    QString HtmlFile = QFileDialog::getOpenFileName(this, "Select HTML page to load", "", "HTML Files (*.htm *.html)");

    // If no file selected, ignore
    if(HtmlFile.isEmpty())
        return;

    // Check if this is an OLD GEX report ....look for string: "<title>Quantix STDF Examinator</title>"
    // Note: reports of release V2.4 and older are using frames!
    FILE *hFile;
    hFile = fopen(HtmlFile.toLatin1().constData(),"r");
    if(hFile == NULL)
    {
        GS::Gex::Message::information("", "Can't open HTML page!");
        return;
    }

    // Saves Report name in case we have to print it later on...
    strPrintReportPath = HtmlFile;

    BOOL bOldGexReport=false;
    while(!feof(hFile))
    {
        if(fgets(szString,255,hFile) == NULL)
            break; // The end.
        if(strstr(szString,"<title>Quantix STDF Examinator</title>") != NULL)
        {
            bOldGexReport = true;
            break;
        }
    };
    fclose(hFile);
    if(bOldGexReport==true)
    {
        // This is an old report, using frames...tell user to use a web browser
#if defined unix || __MACH__
        GS::Gex::Message::information(
                    "", "Old report using HTML frames\nUse your default Web viewer.");
#else
        GS::Gex::Message::information(
                    "", "Old report using HTML frames\n"
                        "Your default Web viewer will be launched.");
        ShellExecuteA(NULL,
                      "open",
                      HtmlFile.toLatin1().constData(),
                      NULL,
                      NULL,
                      SW_SHOWNORMAL);
#endif
        return;
    }

    // Load HTML file into browser window
    OnBrowseUrl(HtmlFile);
}

void GexMainwindow::OnEmail()
{
    GS::Gex::Message::information("", "Coming soon!");
}

///////////////////////////////////////////////////////////
// Starts the gex assistant
///////////////////////////////////////////////////////////
//void GexMainwindow::keyReleaseEvent(QKeyEvent * event)
//{
//    if (event->key() == Qt::Key_F1)
//        OnContextualHelp();
//    else
//        QMainWindow::keyReleaseEvent(event);
//}

void GexMainwindow::LoadUrl(const QString &strUrl)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("load url '%1'...").arg( strUrl).toLatin1().constData());
    if(m_pWebViewReport)
        m_pWebViewReport->loadUrl(strUrl);
}

void GexMainwindow::reloadUrl()
{
    if(m_pWebViewReport)
        m_pWebViewReport->reload();
}


///////////////////////////////////////////////////////////
// Find text in HTML page
///////////////////////////////////////////////////////////
void GexMainwindow::OnFind(void)
{
    static QString strFind;
    // If HTML browser is hidden by the scrollview, ignore the Ctrl+F key!
    if(pScrollArea->isVisible())
    {
        emit sShortcut();
        return;
    }

    bool ok;
    strFind = QInputDialog::getText(this, "Find text (Find/Find Next)", "Find what:", QLineEdit::Normal,strFind, &ok);

    // If escape, quietly return.
    if(!ok || strFind.isEmpty())
        return;

    bool bFind = m_pWebViewReport->find(strFind);

    // Check if instance found
    if(!bFind)
        return;
}

///////////////////////////////////////////////////////////
// Load the additional module QScript engine from localconfig.conf file
///////////////////////////////////////////////////////////
bool GexMainwindow::loadAdditionalModules(QString* pStrErrorMsg)
{
    GSLOG(SYSLOG_SEV_DEBUG, " begin");

    if (!pGexScriptEngine)
    {
        if (pStrErrorMsg)
            *pStrErrorMsg = "no Script Engine allocated";
        return false;
    }

    int iModuleStatus = -1;
    QStringList lstAdditionalModules;
    char szString[GEX_MAX_PATH+1];

    // Add additional module to check
    iModuleStatus = get_private_profile_string("Additional-Modules",
                                               "Modules",
                                               "",
                                               szString,
                                               GEX_MAX_PATH,
                                               GS::Gex::Engine::GetInstance().GetLocalConfigFileName().toLatin1().constData());
    if (iModuleStatus > 0)
    {
        QString strValue = QString(szString);
        pGexScriptEngine->globalObject().setProperty(PROP_ADDITIONAL_MODULES, strValue);
        lstAdditionalModules = strValue.split("|");
    }
    else
        pGexScriptEngine->globalObject().setProperty(PROP_ADDITIONAL_MODULES, "");

    QString strModuleName;
    // Retrieve each additional modules value
    foreach (strModuleName, lstAdditionalModules)
    {
        iModuleStatus = get_private_profile_string("Additional-Modules",
                                                   strModuleName.toLatin1().constData(),
                                                   "",
                                                   szString,
                                                   GEX_MAX_PATH,
                                                   GS::Gex::Engine::GetInstance().GetLocalConfigFileName().toLatin1().constData());
        if (iModuleStatus > 0)
        {
            QString strValue = QString(szString);
            pGexScriptEngine->globalObject().setProperty(strModuleName, strValue);
        }
        else
        {
            pGexScriptEngine->globalObject().setProperty(strModuleName, "");
            GSLOG(SYSLOG_SEV_WARNING, QString("Error when reading '%1' unable to find value for module '%2'!")
                  .arg(GS::Gex::Engine::GetInstance().GetLocalConfigFileName()).arg(strModuleName)
                  .toLatin1().constData()
                  );
        }
    }

    GSLOG(SYSLOG_SEV_DEBUG, " end");

    return true;
}

///////////////////////////////////////////////////////////
// Save the additional module QScript engine to localconfig.conf file
///////////////////////////////////////////////////////////
bool GexMainwindow::saveAdditionalModules(QString* pStrErrorMsg)
{
    if (!pGexScriptEngine)
    {
        if (pStrErrorMsg)
            *pStrErrorMsg = "no Script Engine allocated";
        return false;
    }

    if (pStrErrorMsg)
        *pStrErrorMsg = "";
    int iWriteStatus;


    // Get module list
    QString strModuleList = pGexScriptEngine->globalObject().property(PROP_ADDITIONAL_MODULES).toString();
    if (strModuleList.trimmed().isEmpty())
        return true;

    QStringList lstAdditionalModules = strModuleList.split("|");

    // Save module list to file
    iWriteStatus = write_private_profile_string("Additional-Modules",
                                                "Modules",
                                                lstAdditionalModules.join("|").toLatin1().constData(),
                                                GS::Gex::Engine::GetInstance().GetLocalConfigFileName().toLatin1().constData());
    // If nothing has been written
    if (iWriteStatus <= 0)
    {
        if (pStrErrorMsg)
            *pStrErrorMsg = QString("Unable to save: Additional Module list: %1 to file %2")
                .arg(lstAdditionalModules.join("|"))
                .arg(GS::Gex::Engine::GetInstance().GetLocalConfigFileName());
    }

    QString strModuleName;
    // Save each module valuee to file
    foreach (strModuleName, lstAdditionalModules)
    {
        QString strModuleValue = pGexScriptEngine->globalObject().property(strModuleName).toString();
        iWriteStatus = write_private_profile_string("Additional-Modules",
                                                    strModuleName.toLatin1().constData(),
                                                    strModuleValue.toLatin1().constData(),
                                                    GS::Gex::Engine::GetInstance().GetLocalConfigFileName().toLatin1().constData());
        // If nothing has been written
        if (iWriteStatus <= 0)
        {
            if (pStrErrorMsg)
                *pStrErrorMsg = QString("Unable to save: Additional Module: %1 = %2 to file %3")
                    .arg(strModuleName)
                    .arg(strModuleValue)
                    .arg(GS::Gex::Engine::GetInstance().GetLocalConfigFileName());
        }
    }

    // If an error has occured
    if (pStrErrorMsg && !(*pStrErrorMsg).isEmpty())
        return false;

    return true;
}

///////////////////////////////////////////////////////////
// Return the evaluated additional module value
///////////////////////////////////////////////////////////
QScriptValue GexMainwindow::additionalModule(const QString& strModuleName)
{
    if (!pGexScriptEngine)
    {
        GSLOG(SYSLOG_SEV_ERROR, "error while retrieving additional modules, no Script Engine allocated!");
        return QScriptValue();
    }

    QString strModuleValue = pGexScriptEngine->globalObject().property(strModuleName).toString();
    if (strModuleValue.trimmed().isEmpty())
        return QScriptValue();

    QScriptValue scriptValue = pGexScriptEngine->evaluate(strModuleValue);
    if (scriptValue.isError() || pGexScriptEngine->hasUncaughtException())
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Error when evaluating additional module '%1' = '%2'. '%3'")
              .arg(strModuleName)
              .arg(strModuleValue)
              .arg(pGexScriptEngine->uncaughtException().toString())
              .toLatin1().data()
              );
    }
    return scriptValue;
}


void   GexMainwindow::ReloadWizardHandler (CTest* test)
{
    if(test == 0)
       mWizardsHandler.ReloadWizards();
    else
        mWizardsHandler.ReloadWizardsDisplayingTest(test->GetTestNumber(), test->GetTestName(), test->GetPinIndex());
}
