///////////////////////////////////////////////////////////
// gex_pdf_report.cpp: implementation of CGexPdfReport class.
///////////////////////////////////////////////////////////
#include <QPrinter>
#include <qfile.h>
#include <qtextstream.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qpushbutton.h>
#include <gqtl_sysutils.h>
#include <gqtl_log.h>
#include <product_info.h>
#include <QElapsedTimer>
#include <QThread>

#include "gex_pdf_report.h"
#include "engine.h"
#include "command_line_options.h"
#include "process_launcher.h"
#include "conversionprogress_dialog.h"
#include "browser_dialog.h"

#define PROP_WEBVIEW_LOADED "PropWebViewLoaded"
#define PROP_WEBVIEW_STATUS "PropWebViewStatus"

CGexPdfReport::CGexPdfReport(): QObject(0), m_pclProgressDlg(NULL)
{
    setProperty(PROP_WEBVIEW_STATUS, false);
    setProperty(PROP_WEBVIEW_LOADED, false);
#ifndef GSDAEMON
    connect(&mWebView, SIGNAL(loadFinished(bool)), this, SLOT(OnLoadFinished(bool)) );
    connect(&mWebView, SIGNAL(loadProgress(int)), this, SLOT(OnLoadProgress(int)) );
    connect(&mWebView, SIGNAL(loadStarted()), this, SLOT(OnLoadStarted()) );
#endif
}

CGexPdfReport::~CGexPdfReport()
{
}

void CGexPdfReport::OnLoadStarted()
{
    GSLOG(SYSLOG_SEV_NOTICE, "Pdf print webview load started...");
}

void CGexPdfReport::OnLoadFinished(bool lRes)
{
    setProperty(PROP_WEBVIEW_STATUS, lRes);
    setProperty(PROP_WEBVIEW_LOADED, true);
}

void CGexPdfReport::OnLoadProgress(int lProgress)
{
    GSLOG(SYSLOG_SEV_NOTICE, QString("Pdf report load progress %1").arg(lProgress).toLatin1().data() );
}

int CGexPdfReport::GeneratePdfFromHtmlUsingQPrinter(
        QWidget* parent,
        const GexPdfOptions& stGexPdfOptions,
        const QString& lHtmlFullFileName,
        QString strPdfFullFileName/* = ""*/)
{
    Q_UNUSED(parent)
    #ifdef GSDAEMON

        Q_UNUSED(stGexPdfOptions)
        Q_UNUSED(lHtmlFullFileName)
        Q_UNUSED(strPdfFullFileName)
        return NotSupported;
    #else

        mWebView.load(QUrl::fromLocalFile(lHtmlFullFileName));
        QElapsedTimer lTimer; lTimer.start();
        while (!property(PROP_WEBVIEW_LOADED).toBool()) //&& lTimer.elapsed()/1000<10)
        {
            QThread::currentThread()->usleep(100000);
            qApp->processEvents();
        }
        if (property(PROP_WEBVIEW_LOADED).toBool()==false)
            return Err_TimeOut;
        setProperty(PROP_WEBVIEW_LOADED, false);
        if (!property(PROP_WEBVIEW_STATUS).toBool())
            return Err_ProcessError;
        QPrinter lPrinter;
        lPrinter.setOutputFileName(strPdfFullFileName);
        // lPrinter.setFullPage(true); ?
        // todo:
        //lPrinter.setPageMargins();
        //lPrinter.setPaperSize();
        //lPrinter.setFontEmbeddingEnabled();

        if (stGexPdfOptions.m_nPaperOrientation==t_GexPdfOptions::ePaperOrientationPortrait)
            lPrinter.setOrientation(QPrinter::Portrait);
        else
            lPrinter.setOrientation(QPrinter::Landscape);
        GSLOG(SYSLOG_SEV_NOTICE, "Check me: check if other options usable with QPrinter/WebView");
        lPrinter.setOutputFormat(QPrinter::PdfFormat);
        mWebView.print(&lPrinter);
        // digia: is there a way to know if print succeeded ?
        return NoError;
    #endif
}

int	CGexPdfReport::GeneratePdfFromHtmlUsingHtmlDoc(QWidget* parent, const GexPdfOptions& stGexPdfOptions,
                        const QString& strHtmlFullFileName, QString strPdfFullFileName)
{
    QString	strApplicationDir;
    // Get application directory
    if(CGexSystemUtils::GetApplicationDirectory(strApplicationDir) != CGexSystemUtils::NoError)
        return Err_MissingDir_App;

    // Construct Htmldoc command
    #if (defined __sun__)
        QString strHtmldocCommand = strApplicationDir + "/htmldoc/htmldoc_sol";
    #elif (defined __MACH__)
        QString strHtmldocCommand = strApplicationDir + "/htmldoc/htmldoc_mac";
    #elif (defined __linux__)
        QString strHtmldocCommand = strApplicationDir + "/htmldoc/htmldoc_linux";
    #else
        QString strHtmldocCommand = strApplicationDir + "/htmldoc/htmldoc.exe";
    #endif
        CGexSystemUtils::NormalizePath(strHtmldocCommand);
        if(QFile::exists(strHtmldocCommand) != true)
            return Err_MissingExe;

    // Check for htmldoc data directories
    QString strDirectory;
    strDirectory = strApplicationDir + "/htmldoc/data";
    CGexSystemUtils::NormalizePath(strDirectory);
    if(QFile::exists(strDirectory) != true)
        return Err_MissingDir_Data;
    strDirectory = strApplicationDir + "/htmldoc/fonts";
    CGexSystemUtils::NormalizePath(strDirectory);
    if(QFile::exists(strDirectory) != true)
        return Err_MissingDir_Fonts;

    // Generate htmldoc argument
    QStringList	strHtmldocArguments;
    GenerateHtmldocArgument(strApplicationDir, strHtmldocArguments,
      stGexPdfOptions, strHtmlFullFileName, strPdfFullFileName);

    // Construct name of animation file: <application>/images/html2pdf.mng
    QString strAnimationFile = strApplicationDir + "/images/html2pdf.mng";
    CGexSystemUtils::NormalizePath(strAnimationFile);

    // Construct dialog tite
    QString strDlgTitle;
    strDlgTitle =	"<font color=\"#5555ff\" face=\"Arial\"><b>";
    strDlgTitle +=	"<p align=\"center\"> <i>Now finalizing Pdf document...!</i></b></p>";
    strDlgTitle +=	"</font>";

    // Run the process
    CProcessLauncher clProcessLauncher(strHtmldocCommand, strHtmldocArguments,
                                   #ifdef GSDAEMON
                                       true // synchronous ?
                                   #else
                                       GS::Gex::Engine::GetInstance().GetCommandLineOptions().IsHidden()
                                   #endif
                                       );

    // Gex is running as a batch, don't display the conversion dialog
    // or daemon
    #ifndef GSDAEMON
    if (GS::Gex::Engine::GetInstance().GetCommandLineOptions().IsHidden() == true)
    {
    #else
        Q_UNUSED(parent)
    #endif

         if(clProcessLauncher.Launch() == false)
             return Err_LaunchHtmlDocProcess;

    #ifndef GSDAEMON
    }
    else
    {
        m_pclProgressDlg = new ConversionProgressDialog("PDF Report",
          strDlgTitle, strAnimationFile, strHtmlFullFileName, strPdfFullFileName, parent);

        // Hide Progress bar "Cancel" button.
        if(!stGexPdfOptions.m_bShowProgressBar)
            m_pclProgressDlg->buttonCancel->hide();

        connect( &clProcessLauncher, SIGNAL( sScriptFinished() ), m_pclProgressDlg, SLOT( accept() ) );

        if(clProcessLauncher.Launch() == false)
        {
            delete m_pclProgressDlg; m_pclProgressDlg=0;
            return Err_LaunchHtmlDocProcess;
        }

        if(m_pclProgressDlg->exec() == QDialog::Rejected)
        {
            clProcessLauncher.AbortScript();
            delete m_pclProgressDlg; m_pclProgressDlg=0;
            return ConversionCancelled;
        }

        delete m_pclProgressDlg; m_pclProgressDlg=0;
    }
#endif

    // Check if destination file exists
    if(QFile::exists(strPdfFullFileName) == false)
        return Err_ProcessError;

    return NoError;
}

int CGexPdfReport::GeneratePdfFromHtml(
        QWidget* parent,
        const GexPdfOptions& stGexPdfOptions,
        const QString& strHtmlFullFileName,
        QString strPdfFullFileName/* = ""*/)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Generate Pdf from html: target=%1").arg(strPdfFullFileName)
          .toLatin1().data());

    // Set Pdf document file name if empty
    if(strPdfFullFileName.isEmpty())
        strPdfFullFileName = strHtmlFullFileName + ".pdf";

    // Normalize file names
    QString strRealHtmlFullFileName = strHtmlFullFileName;
    CGexSystemUtils::NormalizePath(strRealHtmlFullFileName);
    CGexSystemUtils::NormalizePath(strPdfFullFileName);

    // Try to remove destination file
    QDir clDir;
    if((QFile::exists(strPdfFullFileName) == true) && (clDir.remove(strPdfFullFileName) == false))
        return Err_RemoveDestFile;

    if (stGexPdfOptions.mPrinterType==t_GexPdfOptions::eQPrinter)
        return GeneratePdfFromHtmlUsingQPrinter(parent,stGexPdfOptions,strRealHtmlFullFileName,strPdfFullFileName);

    if (stGexPdfOptions.mPrinterType==t_GexPdfOptions::eHtmlDoc)
        return GeneratePdfFromHtmlUsingHtmlDoc(parent, stGexPdfOptions, strRealHtmlFullFileName, strPdfFullFileName);

    return Err_UnknownPrinterType;
}

void CGexPdfReport::GenerateHtmldocArgument(
        const QString& strApplicationDir,
        QStringList& strArguments,
        const GexPdfOptions& stGexPdfOptions,
        const QString& strHtmlFullFileName,
        QString& strPdfFullFileName)
{
    //CGexSystemUtils	clSystemUtils;
    QString	strHtmldocDatadir = strApplicationDir + "/htmldoc";

    CGexSystemUtils::NormalizePath(strHtmldocDatadir);

    strArguments.empty();

    // Output format = PDF 1.4
    strArguments += "-t";
    strArguments += "pdf14";

    // Output file
    strArguments += "-f";
    strArguments += strPdfFullFileName;

    // Input format = HTML
    strArguments += "--webpage";

    // Data directory
    strArguments += "--datadir";
    strArguments += strHtmldocDatadir;

    // Paper size
    strArguments += "--size";
    if(stGexPdfOptions.m_nPaperFormat == GexPdfOptions::ePaperFormatA4)
        strArguments += "A4";
    else
        strArguments += "Letter";

    // Paper orientation
    if(stGexPdfOptions.m_nPaperOrientation == GexPdfOptions::ePaperOrientationPortrait)
        strArguments += "--portrait";
    else
        strArguments += "--landscape";

    // Margins
    QString strMargin;
    if(stGexPdfOptions.m_nMarginUnits == GexPdfOptions::eMarginUnitsInches)
    {
        strMargin.sprintf("%gin", stGexPdfOptions.m_lfMargin_Top);
        strArguments += "--top";
        strArguments += strMargin;
        strMargin.sprintf("%gin", stGexPdfOptions.m_lfMargin_Left);
        strArguments += "--left";
        strArguments += strMargin;
        strMargin.sprintf("%gin", stGexPdfOptions.m_lfMargin_Right);
        strArguments += "--right";
        strArguments += strMargin;
        strMargin.sprintf("%gin", stGexPdfOptions.m_lfMargin_Bottom);
        strArguments += "--bottom";
        strArguments += strMargin;
    }
    else
    {
        strMargin.sprintf("%gcm", stGexPdfOptions.m_lfMargin_Top);
        strArguments += "--top";
        strArguments += strMargin;
        strMargin.sprintf("%gcm", stGexPdfOptions.m_lfMargin_Left);
        strArguments += "--left";
        strArguments += strMargin;
        strMargin.sprintf("%gcm", stGexPdfOptions.m_lfMargin_Right);
        strArguments += "--right";
        strArguments += strMargin;
        strMargin.sprintf("%gcm", stGexPdfOptions.m_lfMargin_Bottom);
        strArguments += "--bottom";
        strArguments += strMargin;
    }

    // Header/Footer
    strArguments += "--header";
    strArguments += ".t.";

    //image footer
    if(stGexPdfOptions.m_imageFooter.isEmpty() == false)
    {
        strArguments += "--logoimage";
        strArguments += stGexPdfOptions.m_imageFooter;
        strArguments += "--footer";
        strArguments += "ld/ " + stGexPdfOptions.m_imageFooterFile;
    }
    else
    {
        strArguments += "--footer";
        strArguments += "c./";
    }

    // Colors and styles
    strArguments += "--textcolor";
    strArguments += "BLACK";
    strArguments += "--linkcolor";
    strArguments += "BLUE";
    strArguments += "--linkstyle";
    strArguments += "underline";

    // Fonts
    strArguments += "--fontsize";
    strArguments += "8";
    strArguments += "--fontspacing";
    strArguments += "1.3";
    strArguments += "--bodyfont";
    strArguments += "Times";
    strArguments += "--headingfont";
    strArguments += "Helvetica";
    strArguments += "--headfootsize";
    strArguments += "8";
    strArguments += "--headfootfont";
    strArguments += "Helvetica";

    // Charset
    strArguments += "--charset";
    strArguments += "iso-8859-1";

    // Other options
    strArguments += "--color";			// Generate a color document
    strArguments += "--no-title";		// No title page
    strArguments += "--no-toc";			// No table-of-contents page
    strArguments += "--no-jpeg";		// No JPEG compression on big images
    strArguments += "--jpeg=0";			// JPEG quality/compression ([0-100])
    strArguments += "--compression=1";	// Compression level on output file ([1-9])
    strArguments += "--nup";			// Pages per output page
    strArguments += "1";
    if (stGexPdfOptions.m_bEmbedFonts)
        strArguments += "--embedfonts";		// Embed fonts into document
    else
        strArguments += "--no-embedfonts";	// not embededed
    strArguments += "--pagemode";		// Page mode (document, outline, fullscreen)
    strArguments += "document";
    strArguments += "--pagelayout";		// Page layout (single, one, twoleft, tworight)
    strArguments += "single";
    strArguments += "--pageeffect";		// Page effect
    strArguments += "none";
    strArguments += "--firstpage";		// First page = page 1
    strArguments += "p1";
    strArguments += "--links";			// Use links in document
    strArguments += "--no-encryption";	// No encryption
    strArguments += "--permissions";	// All permssion granted to the user
    strArguments += "all";
    strArguments += "--no-strict";		// Turn off strict HTML conformance checking
    strArguments += "--browserwidth";	// Browser width for image resizing
    strArguments += "1200";

    // Input html file
    strArguments += strHtmlFullFileName;

    return;
}
