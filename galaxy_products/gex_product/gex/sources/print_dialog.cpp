///////////////////////////////////////////////////////////
// GEX Print Dialog box
///////////////////////////////////////////////////////////
#ifdef _WIN32
    #include <windows.h>
#endif
#include <qprinter.h>
#include <qfile.h>
#include <qpainter.h>
#include <qapplication.h>
#include <QProgressBar>
#include <QPrintDialog>
#include <gqtl_log.h>
#include <QTextBlock>
#include <QWebView>
#include <gqtl_log.h>

#include "print_dialog.h"
#include "drill_chart.h"
#include "drill_table.h"
#include "drill_what_if.h"
#include "drillDataMining3D.h"
#include "gex_web_browser.h"
#include "gex_pdf_report.h"
#include "engine.h"
#include "message.h"
#include "product_info.h"

#define FILE_RESOLUTION 600

extern int qt_defaultDpi();

// main.cpp
extern GexMainwindow* pGexMainWindow;

///////////////////////////////////////////////////////////
// GEX Print dialog box.
///////////////////////////////////////////////////////////
GexPrint::GexPrint( QWidget* parent, bool modal, Qt::WindowFlags fl) : QDialog(parent, fl), mHtmlReport(0)
{
    setupUi(this);
    setModal(modal);

    // Apply Examinator palette
    GexMainwindow::applyPalette(this);

    QObject::connect(PushButtonOk,		SIGNAL(clicked()), this, SLOT(accept()));
    QObject::connect(RadioCurrentPage,	SIGNAL(clicked()), this, SLOT(OnPrintCurrentPage()));
    QObject::connect(RadioReport,		SIGNAL(clicked()), this, SLOT(OnPrintReportSections()));

    // Font used for printing page numbers.
    pFont = new QFont("times", 10);

    // Default: print current page.
    OnPrintCurrentPage();
    // GCORE-118
    mWebView.hide();
    QObject::connect(&mWebView, SIGNAL(loadFinished(bool)), this, SLOT(OnLoadFinished(bool)) );
}

#define GEXPRINT_PROP_PDF_OUTPUT_FILE "PDFOutputFile"

QString GexPrint::PrintHtmlToPDF(const QString &source, const QString &output)
{
    setProperty(GEXPRINT_PROP_PDF_OUTPUT_FILE, output);
    mWebView.load(QUrl::fromLocalFile(source));
    return "ok";
}

void GexPrint::OnLoadFinished(bool lRes)
{
    GSLOG(SYSLOG_SEV_NOTICE, QString("GexPrint OnLoadFinished: %1").arg(lRes?"true":"false").toLatin1().data() );
    if (!lRes)
    {
        GSLOG(SYSLOG_SEV_WARNING, "Cannot load html source in WebView");
        return;
    }

    QPrinter lPrinter;
    if (property(GEXPRINT_PROP_PDF_OUTPUT_FILE).isNull())
        lPrinter.setOutputFileName(QDir::homePath()+"/GalaxySemi/temp/gexprint.pdf");
    else
        lPrinter.setOutputFileName(property(GEXPRINT_PROP_PDF_OUTPUT_FILE).toString());
    lPrinter.setOrientation(QPrinter::Portrait); // QPrinter::Landscape
    lPrinter.setOutputFormat(QPrinter::PdfFormat);
    mWebView.print(&lPrinter);    
}


GexPrint::~GexPrint()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "GexPrint destructor");
    delete pFont;
}

bool GexPrint::IsWidgetToPrint(void)
{
    switch(pGexMainWindow->iWizardPage)
    {
        case GEX_CHART_WIZARD_P1:			// Interactive Chart
        case GEX_TABLE_WIZARD_P1:			// Interactive Table
        case GEX_DRILL_WHATIF_WIZARD_P1:	// what-if
        case GEX_DRILL_3D_WIZARD_P1:		// 3D
            return true;
        default:
            return false;	// Real HTML page to print (will pop-up dialog box to detail which pages to print)
    }
}

void GexPrint::PrintPages(QPrinter *printer, QPainter *p, const QUrl& urlCurrent, QString strPrintReport)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Print Pages '%1' into '%2'")
          .arg(urlCurrent.toString().toLatin1().constData())
          .arg(strPrintReport).toLatin1().constData());
    int iTotalSteps=10;	// Total printing steps...
    int	iPage=0;		// Used to find HTML child pages (Histogram, Stats, Advanced).
    QFile cFile;

    // Computes printer resolution...
    //QPaintDeviceMetrics metrics(p->device());
    if (p->device())
    {
        dpix = p->device()->logicalDpiX();
        dpiy = p->device()->logicalDpiY();
        bodyRef.setRect(dpix, dpiy,
                    p->device()->width()  - dpix * 2,
                    p->device()->height() - dpiy * 2);
    }

    mPage = 1;

    if(!mHtmlReport)
        mHtmlReport = new QTextBrowser();
    // Update the status + progress bar...
    GS::Gex::Engine::GetInstance().UpdateProgressStatus(true,iTotalSteps,1);	// Show process bar...step 1
    UpdateStatusMessage(" Printing...");
    QCoreApplication::processEvents();

    // Build, then Print Header page...
    QString strFile;
    int iIndex=0;
    if(strPrintReport.isEmpty())
    {
        // No report...can only print current selection!
        strFile = urlCurrent.toLocalFile();
        GSLOG(SYSLOG_SEV_DEBUG, QString("Print pages : current url to local = %1").arg( strFile).toLatin1().constData() );
        iIndex = strFile.lastIndexOf('/');
        strFile.truncate(iIndex);
        strFile += "/print.htm";
    }
    else
    {
        iIndex = strPrintReport.indexOf("index.htm");
        strFile = strPrintReport;
        strFile.truncate(iIndex);
        strFile += "pages/print.htm";
    }

    FILE *hFile=fopen(strFile.toLatin1().constData(),"w");
    if(hFile != NULL)
    {
        //
        fprintf(hFile,"<html>\n");
        fprintf(hFile,"<!-- ***************************************************************************-->\n");
        fprintf(hFile,"<!-- %s-->\n", GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );
        fprintf(hFile,"<!-- %s.   www.mentor.com-->\n",C_STDF_COPYRIGHT);
        fprintf(hFile,"<!-- All rights reserved. Users of the program must be Registered Users (see-->\n");
        fprintf(hFile,"<!-- the Quantix examinator license terms).-->\n");
        fprintf(hFile,"<!-- The Quantix Examinator program (including its HTML pages and .png files)-->\n");
        fprintf(hFile,"<!-- is protected by copyright law and international treaties. You are no-->\n");
        fprintf(hFile,"<!-- allowed to alter any HTML page, .png file, or any other part of the-->\n");
        fprintf(hFile,"<!-- software. Unauthorized reproduction or distribution of this program,-->\n");
        fprintf(hFile,"<!-- or any portion of it, may result in severe civil and criminal -->\n");
        fprintf(hFile,"<!-- penalties, and will be prosecuted to the maximum extent possible.-->\n");
        fprintf(hFile,"<!-- ***************************************************************************-->\n");
        fprintf(hFile,"<head>\n");
        fprintf(hFile,"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=ISO-8859-1\">\n");
        fprintf(hFile,"<title>Quantix Examinator - the STDF detective</title>\n");
        fprintf(hFile,"</head>\n");
        fprintf(hFile,"<body>\n");
        if(strPrintReport.isEmpty() == false)
            fprintf(hFile,"<img border=\"0\" src=\"../images/print.png\">\n");

        fprintf(hFile,"<h1 align=\"left\"><font color=\"#006699\">Welcome to Quantix Examinator reports!</font></h1><br>\n");

        // Sets default background color = white, text color given in argument.
        time_t iTime = time(NULL);
        fprintf(hFile,"<align=\"left\"><font color=\"#000000\" size=\"%d\"><br><b>Date: %s</b><br></font>\n",3,ctime(&iTime));
        fprintf(hFile,"<align=\"left\"><font color=\"#000000\" size=\"%d\"><b>Report from: </b>%s<br></font>\n",3,
                GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );

        fprintf(hFile,"<align=\"left\"><font color=\"#000000\" size=\"%d\"><br><b>Notes: </b><br></font>\n",3);
        // Write the Notes lines
        int iTotalLines = TextEditComment->document()->blockCount();
        for(int iLine=0; iLine < iTotalLines; iLine++)
        {
            fprintf(hFile,"<align=\"left\"><font color=\"#000000\" size=\"%d\">%s<br></font>\n",3,
                    TextEditComment->document()->findBlockByNumber(iLine).text().toLatin1().constData());
        }
        fprintf(hFile,"</body>\n");
        fprintf(hFile,"</html>\n");
        fclose(hFile);
    }

    // Print cover page only if HTML pages to print, no cover page if its Interactive page to print.
    if(IsWidgetToPrint() == false)
        PrintHtmlFile(printer,p,strFile);

    // Update process bar...
    GS::Gex::Engine::GetInstance().UpdateProgressStatus(false,-1,-1);	// Increment to next step.
    QCoreApplication::processEvents();

    // Remove temporary report front page.
    remove(strFile.toLatin1().constData());

    // Check list of files to print...
    if(IsWidgetToPrint() || RadioCurrentPage->isChecked())
    {
        // Print current page only...
        QPixmap *pPixmap=0;
        switch(pGexMainWindow->iWizardPage)
        {
        case GEX_CHART_WIZARD_P1:	// Interactive Chart
            // Capture browser area into Clipboard
            pPixmap = new QPixmap(QPixmap::grabWidget(pGexMainWindow->LastCreatedWizardChart()));
            PrintPixmap(printer,p,pPixmap);
            break;
        case GEX_TABLE_WIZARD_P1:	// Interactive Table
            // Capture browser area into Clipboard
            pPixmap = new QPixmap(QPixmap::grabWidget(pGexMainWindow->LastCreatedWizardTable()));
            PrintPixmap(printer,p,pPixmap);
            break;
        case GEX_DRILL_WHATIF_WIZARD_P1:	// What-If
            // Capture browser area into Clipboard
            pPixmap = new QPixmap(QPixmap::grabWidget(pGexMainWindow->pWizardWhatIf));
            PrintPixmap(printer,p,pPixmap);
            break;
        case GEX_DRILL_3D_WIZARD_P1:	// 3D Drill
            // Capture browser area into Clipboard
            GSLOG(SYSLOG_SEV_INFORMATIONAL, "Print pages : capturing current 3d drill widget ...");
            // Case 4824
            // Capturing the current rendering of GLViewer often does not work with grabWindow/Widget:
            //QPixmap::grabWindow(pGexMainWindow->pWizardDrill3D->winId());
            //QPixmap::grabWidget(pGexMainWindow->pWizardDrill3D);
            // grabWidget seems to make disappear the wafermap but correctly catch the openGL rendering
            //pPixmap = new QPixmap(QPixmap::grabWidget(pGexMainWindow->pWizardDrill3D));
            // grabWindow cannot catch openGL image on many computers/OS.
            //pPixmap = new QPixmap(QPixmap::grabWindow(pGexMainWindow->pWizardDrill3D->winId()));
            pPixmap = new QPixmap(pGexMainWindow->LastCreatedWizardWafer3D()->GrabWidget());
            if (pPixmap->isNull())
            {
                GSLOG(SYSLOG_SEV_ERROR, "grab SYSLOG_SEV_ERRORd wizard page failed");
            }
            else
                pPixmap->save(QDir::homePath()+"/GalaxySemi/temp/wizard.png");

            PrintPixmap(printer, p, pPixmap);
            //delete pPixmap; // deleting here make it crash. Dont know why.

            break;

        default:	// Print current visible HTML page
            PrintHtmlFile(printer, p, urlCurrent.toLocalFile());
            // GCORE-118
            PrintHtmlToPDF(urlCurrent.toLocalFile(), QDir::homePath()+"/GalaxySemi/temp/gexprint.pdf");
            break;
        }
        goto tagEndPrinting;
    }

    // Update process bar...
    GS::Gex::Engine::GetInstance().UpdateProgressStatus(false,-1,-1);	// Increment to next step.
    QCoreApplication::processEvents();

    // Print report section(s)
    if(CheckGlobal->isChecked())
    {
        // Print globals.htm section
        strFile = strPrintReport;
        strFile.truncate(iIndex);
        strFile += "pages/global.htm";
        PrintHtmlFile(printer, p, strFile);
    }
    // Update process bar...
    GS::Gex::Engine::GetInstance().UpdateProgressStatus(false,-1,-1);	// Increment to next step.
    QCoreApplication::processEvents();

    if(CheckStats->isChecked())
    {
        // Print Test Statistics section
        strFile = strPrintReport;
        strFile.truncate(iIndex);
        strFile += "pages/stats.htm";
        //phil PrintHtmlFile(printer,p,strFile);

        // Scan all child pages : ./statsxxx.htm
        iPage = 1;	// First child page.
        while(1)
        {
            strFile = strPrintReport;
            strFile.truncate(iIndex);
            strFile += "pages/stats";
            strFile += QString::number(iPage);
            strFile += ".htm";

            cFile.setFileName(strFile);
            if(cFile.exists() == false)
                break;	// All Stats pages found and processed!
            // Print child page.
            PrintHtmlFile(printer,p,strFile);

            // Next page
            iPage++;
        };
    }
    // Update process bar...
    GS::Gex::Engine::GetInstance().UpdateProgressStatus(false,-1,-1);	// Increment to next step.
    QCoreApplication::processEvents();

    if(CheckHistograms->isChecked())
    {
        // Print Histograms section
        strFile = strPrintReport;
        strFile.truncate(iIndex);
        strFile += "pages/histogram.htm";
        // phil PrintHtmlFile(printer,p,strFile);

        // Scan all child pages : ./histogramxxx.htm
        iPage = 1;	// First child page.
        while(1)
        {
            strFile = strPrintReport;
            strFile.truncate(iIndex);
            strFile += "pages/histogram";
            strFile += QString::number(iPage);
            strFile += ".htm";

            cFile.setFileName(strFile);
            if(cFile.exists() == false)
                break;	// All Histogram pages found and processed!
            // Print child page.
            PrintHtmlFile(printer,p,strFile);

            // Next page
            iPage++;
        };
    }
    // Update process bar...
    GS::Gex::Engine::GetInstance().UpdateProgressStatus(false,-1,-1);	// Increment to next step.
    QCoreApplication::processEvents();

    if(CheckWafermap->isChecked()
      && !(GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(
                       GS::LPPlugin::ProductInfo::waferMap)))
    {
        // Print Wafer map section
        strFile = strPrintReport;
        strFile.truncate(iIndex);
        strFile += "pages/wafermap.htm";
        PrintHtmlFile(printer,p,strFile);
    }
    // Update process bar...
    GS::Gex::Engine::GetInstance().UpdateProgressStatus(false,-1,-1);	// Increment to next step.
    QCoreApplication::processEvents();

    if(CheckBinning->isChecked())
    {
        // Print Binning section
        strFile = strPrintReport;
        strFile.truncate(iIndex);
        strFile += "pages/binning.htm";
        PrintHtmlFile(printer,p,strFile);
    }
    // Update process bar...
    GS::Gex::Engine::GetInstance().UpdateProgressStatus(false,-1,-1);	// Increment to next step.
    QCoreApplication::processEvents();

    if(CheckPareto->isChecked())
    {
        // Print Pareto section
        strFile = strPrintReport;
        strFile.truncate(iIndex);
        strFile += "pages/pareto.htm";
        PrintHtmlFile(printer,p,strFile);
    }
    // Update process bar...
    GS::Gex::Engine::GetInstance().UpdateProgressStatus(false,-1,-1);	// Increment to next step.
    QCoreApplication::processEvents();

    if(CheckAdvanced->isChecked())
    {
        // Print Advanced report section
        strFile = strPrintReport;
        strFile.truncate(iIndex);
        strFile += "pages/advanced.htm";
        PrintHtmlFile(printer,p,strFile);

        // Scan all child pages : ./advancedxxx.htm
        iPage = 1;	// First child page.
        while(1)
        {
            strFile = strPrintReport;
            strFile.truncate(iIndex);
            strFile += "pages/advanced";
            strFile += QString::number(iPage);
            strFile += ".htm";

            cFile.setFileName(strFile);
            if(cFile.exists() == false)
                break;	// All Advanced pages found and processed!
            // Print child page.
            PrintHtmlFile(printer,p,strFile);

            // Next page
            iPage++;
        };
    }

    if(CheckSnapshotGallery->isChecked())
    {
        // Print Snapshot Gallery report section
        strFile = strPrintReport;
        strFile.truncate(iIndex);
        strFile += "drill/drill.htm";
        PrintHtmlFile(printer,p,strFile);

        // Scan all child pages : ./dxxx.htm
        iPage = 1;	// First child page.
        while(1)
        {
            strFile = strPrintReport;
            strFile.truncate(iIndex);
            strFile += "drill/d";
            strFile += QString::number(iPage);
            strFile += ".htm";

            cFile.setFileName(strFile);
            if(cFile.exists() == false)
                break;	// All Snapshot pages found and processed!
            // Print child page.
            PrintHtmlFile(printer,p,strFile);

            // Next page
            iPage++;
        };
    }

    // Update process bar...
    GS::Gex::Engine::GetInstance().UpdateProgressStatus(false,-1,-1);	// Increment to next step.
    QCoreApplication::processEvents();

tagEndPrinting:
    // Hides progress+status bars...
    GS::Gex::Engine::GetInstance().UpdateLabelStatus("");
    GS::Gex::Engine::GetInstance().UpdateProgressStatus(true,0,-1);
    QCoreApplication::processEvents();

    if (mHtmlReport)
    {
        delete mHtmlReport;
        mHtmlReport=0;
    }
}

///////////////////////////////////////////////////////////
// Updates status message
///////////////////////////////////////////////////////////
void GexPrint::UpdateStatusMessage(QString strMessage)
{
    GS::Gex::Engine::GetInstance().UpdateLabelStatus(strMessage);
    QCoreApplication::processEvents();
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void GexPrint::UpdateView(QString strHtmlReport)
{
    if(strHtmlReport.isEmpty())
    {
        // No report exist...can only print current page!
        OnPrintCurrentPage();
        RadioReport->setEnabled(false);
    }
    else
    {
        // A report exist, so user can decide which sections to print.
        RadioReport->setEnabled(true);
    }
}

///////////////////////////////////////////////////////////
// User selection: print current page
///////////////////////////////////////////////////////////
void GexPrint::OnPrintCurrentPage(void)
{
    RadioCurrentPage->setChecked(true);
    RadioReport->setChecked(false);
    CheckGlobal->setEnabled(false);
    CheckStats->setEnabled(false);
    CheckHistograms->setEnabled(false);
    CheckWafermap->setEnabled(false);
    CheckBinning->setEnabled(false);
    CheckPareto->setEnabled(false);
    CheckAdvanced->setEnabled(false);
    CheckSnapshotGallery->setEnabled(false);
}

///////////////////////////////////////////////////////////
// User selection: print report sections
///////////////////////////////////////////////////////////
void GexPrint::OnPrintReportSections(void)
{
    RadioCurrentPage->setChecked(false);
    RadioReport->setChecked(true);
    CheckGlobal->setEnabled(true);
    CheckStats->setEnabled(true);
    CheckHistograms->setEnabled(true);
    CheckWafermap->setEnabled(true);
    CheckBinning->setEnabled(true);
    CheckPareto->setEnabled(true);
    CheckAdvanced->setEnabled(true);
    CheckSnapshotGallery->setEnabled(true);
}

// new Implementation of MimeSourceFactory to handle src img  in html with relative path
// and/or  paths contains "?c*" strings.
//class NewMimeSourceFactory : public  Q3MimeSourceFactory{
//    QString m_strSourcePath;
//public:
//    NewMimeSourceFactory(QString sourcePath):Q3MimeSourceFactory(),m_strSourcePath(sourcePath){}
//    ~NewMimeSourceFactory(){ }
//    virtual const QMimeSource* data(const QString& abs_name) const{
//        QMimeSource *poMimeSource = (QMimeSource *)Q3MimeSourceFactory::data(abs_name);
//        if(!poMimeSource){
//            //check if abs name is not absolute name.
//            QString strNewAbsName = abs_name;
//            if( QFileInfo(strNewAbsName).isRelative()){
//                strNewAbsName = m_strSourcePath + QDir::separator() + abs_name;
//            }
//            poMimeSource = (QMimeSource *)Q3MimeSourceFactory::data(strNewAbsName);
//            if(poMimeSource) return poMimeSource;
//            //check if there some extra char like ?c*
//            QRegExp oRegExp("c*\\?c*");
//            int iIdx ;
//            if ((iIdx = strNewAbsName.indexOf(oRegExp)) != -1) {
//                strNewAbsName.remove(iIdx, strNewAbsName.count());
//            }
//            return Q3MimeSourceFactory::data(strNewAbsName);
//        }
//        else
//            return poMimeSource;
//    }
//};

QString GexPrint::PrintHtmlFile(QPrinter* printer, QPainter* lPainter, QString file)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("PrintHtmlFile: '%1'...").arg(file).toLatin1().data() );
    if (file.isEmpty())
    {
        return "error: empty filename";
    }
    // GCORE-113
    /*
    QStringList lSP=mHtmlReport->searchPaths();
    QFileInfo lFI(file);
    lSP.append(lFI.absolutePath());
    mHtmlReport->setSearchPaths(lSP);
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Search pathes: '%1'")
          .arg(mHtmlReport->searchPaths().join(';')).toLatin1().data() );
    */

    body = bodyRef;

    // If file page includes a anchor, remove it
    // (ie: xxxx/file.html#17 changed to xxxx/file.html)
    int iAnchor = file.lastIndexOf('#');
    if (iAnchor >= 0)
    {
        file.truncate(iAnchor);
    }

    QFile f(file);
    if (!f.open(QIODevice::ReadOnly))
    {
        return "error: cannot open file "+file;
    }
    QTextStream ts(&f);
    //    QString     strSrcPath    = QFileInfo(file).path();
    //    Q3MimeSourceFactory* poMS = new NewMimeSourceFactory(strSrcPath);
    //    HtmlReport->setMimeSourceFactory(poMS);
    mHtmlReport->setSource(QUrl::fromLocalFile(file));

    // If other pages printed prior to this file, add a page break!
    if (mPage > 1)
    {
        printer->newPage();
    }

    QRect view(body);

    /*
    // Using QWebView ? no way : perhaps this is a multi page output and we would have to join pdf at then end...
    QWebView lWebView(this);
    lWebView.load(QUrl::fromLocalFile(file));
    lWebView.show();
    qApp->processEvents();
    QThread::currentThread()->sleep(1);
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Webview has currently %1 colors")
          .arg(lWebView.colorCount()).toLatin1().data() );
    lWebView.ensurePolished();
    lWebView.print(printer);
    */

    /*
    // Using HtmlDoc: no way, perhaps the user want to print to something else than pdf
    CGexPdfReport lPdfReport;
    GexPdfOptions lPdfOptions;
    int lRes=lPdfReport.GeneratePdfFromHtml(this,lPdfOptions, file,
                                            GS::Gex::Engine::GetInstance().Get("TempFolder").toString()+"/print1.pdf");
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Generate Pdf from Htlm: %1").arg(lRes).toLatin1().data() );
    */

    // implementation using QTextDocument
    QTextDocument doc;
    doc.setHtml(ts.readAll());
    doc.setPageSize(body.size());
    // test : each images needs to be preloaded in order for QTextDoc to use it.
    //doc.addResource(QTextDocument::ImageResource, QUrl("../images/zoom_in.png"),
                    //QVariant( QImage(lFI.absolutePath()+"/../images/zoom_in.png") ));


    // Ensure painter is reset (no viewport offset, etc...)
    lPainter->resetTransform();
    lPainter->setViewport(body);
    do
    {
        doc.drawContents(lPainter);
        view.translate(0, body.height());
        lPainter->translate(0, -body.height());
        lPainter->setFont(*pFont);
        strFooter = "Quantix Examinator Printout - www.mentor.com" "     /     Page: ";
        strFooter += QString::number(mPage);
        lPainter->drawText(view.right() - lPainter->fontMetrics().width(strFooter),
                    view.bottom() + lPainter->fontMetrics().ascent() + 5, strFooter);
        if (view.top() >= doc.size().height())
        {
            break;
        }
        printer->newPage();
        mPage++;
    }
    while (true);

    // update to last page #.
    mPage++;
    return "ok";
}

void GexPrint::PrintPixmap(QPrinter *printer, QPainter *p, QPixmap *pPixmap)
{
    if (pPixmap->isNull())
    {
        GSLOG(4, "Pixmap NULL");
        return;
    }

    // Create temporary file.
    QString strFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString() + "/.gex_tmp_print.htm";
    QString	strImage = GS::Gex::Engine::GetInstance().Get("UserFolder").toString() + "/.gex_tmp_print_image.png";

    // Save pixmap to disk
    pPixmap->save(strImage,"PNG");

    // Create HTML file that shows the image (so we can print it!)
    FILE *hFile=fopen(strFile.toLatin1().constData(),"w");
    if(hFile == NULL)
        return;

    //
    fprintf(hFile,"<html>\n");
    fprintf(hFile,"<head>\n");
    fprintf(hFile,"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=ISO-8859-1\">\n");
    fprintf(hFile,"</head>\n");
    fprintf(hFile,"<body>\n");
    fprintf(hFile,
            "<img border=\"0\" src=\"%s\" align=\"left\""
            " width=\"%d\" border=\"0\">\n",
            strImage.toLatin1().constData(),
            800 * printer->resolution() / qt_defaultDpi());

    fprintf(hFile,"</body>\n");
    fprintf(hFile,"</html>\n");
    fclose(hFile);

    // Print file
    PrintHtmlFile(printer,p,strFile);

    // Erase file & image
    remove(strFile.toLatin1().constData());
    remove(strImage.toLatin1().constData());

    // Free pixmap
    delete pPixmap; pPixmap=0;
}
