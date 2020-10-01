#ifdef GCORE15334
#include "pat_widget_ft.h"
#include "ui_pat_widget_ft.h"
#include "import_all.h"
#include "browser_dialog.h"
#include "tb_toolbox.h"
#include "message.h"
#include "pat_gts_station.h"
#include "testerserver.h"
#include "gqtl_log.h"
#include "engine.h"
#include "export_atdf.h"
#include "pat_report_ft.h"
#include "pat_recipe_editor.h"

#include <QSettings>
#include <QFileDialog>
#include <QDragEnterEvent>
#include <QDropEvent>


extern GexMainwindow *	pGexMainWindow;

namespace GS
{
namespace Gex
{

PATWidgetFT::PATWidgetFT(QWidget *parent) :
    QWidget(parent),
    mUi(new Ui::PATWidgetFT),
    mTesterServer(new TesterServer(this)),
    mChildWindow(true)
{
    mUi->setupUi(this);

    // Refuse drag&drop on line edit
    mUi->lineEditDataFile->setAcceptDrops(false);
    mUi->lineEditRecipeFile->setAcceptDrops(false);

    // Support for Drag&Drop
    setAcceptDrops(true);

    // Update detach window button
    mUi->buttonDetachWindow->setChecked(mChildWindow);

    // Fill Report Format combo box
    mUi->comboBoxReportFormat->clear();
    mUi->comboBoxReportFormat->addItem(QIcon(QString::fromUtf8(":/gex/icons/stop.png")),
                                      "Disabled", QVariant(ReportDisabled));
    mUi->comboBoxReportFormat->addItem(QIcon(QString::fromUtf8(":/gex/icons/explorer.png")),
                                      "Full HTML report", QVariant(ReportHTML));
    mUi->comboBoxReportFormat->addItem(QIcon(QString::fromUtf8(":/gex/icons/word.png")),
                                      "Full WORD report", QVariant(ReportWord));
    mUi->comboBoxReportFormat->addItem(QIcon(QString::fromUtf8(":/gex/icons/csv_spreadsheet.png")),
                                      "Full CSV report", QVariant(ReportCsv));
    mUi->comboBoxReportFormat->addItem(QIcon(QString::fromUtf8(":/gex/icons/powerpoint.png")),
                                      "Full PowerPoint report", QVariant(ReportPpt));
    mUi->comboBoxReportFormat->addItem(QIcon(QString::fromUtf8(":/gex/icons/pdf.png")),
                                      "Full PDF report", QVariant(ReportPdf));
    mUi->comboBoxReportFormat->addItem(QIcon(QString::fromUtf8(":/gex/icons/zoom_in.png")),
                                      "Full Interactive only report", QVariant(ReportInteractive));

    // Fill Output Test Data Format combo box
    mUi->comboBoxOutputTestDataFormat->clear();
    mUi->comboBoxOutputTestDataFormat->addItem(QIcon(QString::fromUtf8(":/gex/icons/stop.png")),
                                              "Disabled", QVariant(OutputDisabled));
    mUi->comboBoxOutputTestDataFormat->addItem(QIcon(QString::fromUtf8(":/gex/icons/options_datalog.png")),
                                              "STDF", QVariant(OutputSTDF));
    mUi->comboBoxOutputTestDataFormat->addItem(QIcon(QString::fromUtf8(":/gex/icons/options_datalog.png")),
                                              "ATDF", QVariant(OutputATDF));

    // Default report output: HTML
    mUi->comboBoxReportFormat->setCurrentIndex(1);

    // Default file output: STDF
    mUi->comboBoxOutputTestDataFormat->setCurrentIndex(1);

    connect(mUi->lineEditDataFile,          SIGNAL(textChanged(QString)),
            this, SLOT(UpdateUI()));
    connect(mUi->lineEditRecipeFile,        SIGNAL(textChanged(QString)),
            this, SLOT(UpdateUI()));
    connect(mUi->buttonSelectDataFile,      SIGNAL(clicked()),
            this, SLOT(OnSelectTestDataFile()));
    connect(mUi->buttonSelectRecipeFile,    SIGNAL(clicked()),
            this, SLOT(OnSelectRecipeFile()));
    connect(mUi->buttonEditRecipe,          SIGNAL(clicked()),
            this, SLOT(OnEditRecipeFile()));
    connect(mUi->pushButtonProcessFile,     SIGNAL(clicked()),
            this, SLOT(RunSimulator()));
    connect(mUi->buttonDetachWindow,        SIGNAL(clicked()),
            this, SLOT(OnDetachWindow()));
    connect(mUi->comboBoxReportFormat,      SIGNAL(currentIndexChanged(int)),
            this, SLOT(UpdateUI()));
    connect(mUi->comboBoxOutputTestDataFormat,      SIGNAL(currentIndexChanged(int)),
            this, SLOT(UpdateUI()));

    // Update UI
    UpdateUI();
}

PATWidgetFT::~PATWidgetFT()
{
    delete mUi;
}

QString PATWidgetFT::GetRecipeFile() const
{
    return mUi->lineEditRecipeFile->text();
}

QString PATWidgetFT::GetTestDataFile() const
{
    return mUi->lineEditDataFile->text();
}

void PATWidgetFT::SetRecipeFile(const QString &lRecipe)
{
    mUi->lineEditRecipeFile->setText(lRecipe);

    // Always update recipe editor
    if (PATRecipeEditor::IsInstantiated())
        PATRecipeEditor::GetInstance().RefreshEditor(lRecipe);
}

void PATWidgetFT::SetTestDataFile(const QString &lTestData)
{
    mUi->lineEditDataFile->setText(lTestData);
}

void PATWidgetFT::ForceAttachWindow(bool lAttach /*= true*/, bool /*lToFront*/ /*= true*/)
{
    mChildWindow = lAttach;
    mUi->buttonDetachWindow->setChecked(mChildWindow);

    if(mChildWindow && pGexMainWindow != NULL)
    {
        // Re-attacch dialog box to Examinator's scroll view Widget
        pGexMainWindow->pScrollArea->layout()->addWidget(this);

        // Minimum width is 720 pixels
        setMinimumWidth(720);
    }
    else
    {
        // Make sure the HTML page in Examinator's window remains visible
        if(pGexMainWindow != NULL && isVisible())
            pGexMainWindow->ShowHtmlBrowser();

        pGexMainWindow->pScrollArea->layout()->removeWidget(this);
        setParent(NULL, Qt::Dialog);

         // Setup the application buttons
        setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);

        move(QPoint(100,100));
        show();

        // No Minimum width
        setMinimumWidth(10);
    }
}

void PATWidgetFT::UpdateUI()
{
    bool    lApply  = true;
    int     lReport = mUi->comboBoxReportFormat->itemData(mUi->comboBoxReportFormat->currentIndex()).toInt();
    int     lOutput = mUi->comboBoxOutputTestDataFormat->itemData(mUi->comboBoxOutputTestDataFormat->currentIndex()).toInt();

    if (lReport == ReportDisabled && lOutput == OutputDisabled)
        lApply = false;

    mUi->pushButtonProcessFile->setEnabled(mUi->lineEditDataFile->text().isEmpty() == false &&
                                           mUi->lineEditRecipeFile->text().isEmpty() == false &&
                                           mTesterServer->isListening() == false &&
                                           lApply == true);

    mUi->buttonEditRecipe->setEnabled(mUi->lineEditRecipeFile->text().isEmpty() == false);
}

void PATWidgetFT::RunSimulator()
{
    if (mTesterServer->isListening())
    {
        GSLOG(SYSLOG_SEV_WARNING,
              QString("TesterServer already listening to port %1")
              .arg(mTesterServer->serverPort()).toLatin1().constData() );

        return;
    }

    // Check if we max nb of allowed connections reached
    bool    lOk;
    int     lDefaultSocket = Engine::GetInstance().GetOptionsHandler().GetOptionsMap().GetOption("ft_simulator", "server_port").toInt(&lOk);

    if (!lOk)
    {
        Message::warning("", QString("Cannot get default socket to connect to: %1").arg(lDefaultSocket));
        return;
    }

    bool    lConnected  = false;
    int     lSocket     = lDefaultSocket;

    // Try to find an availabled port starting default given port to port + 10
    while (!lConnected && lSocket < (lDefaultSocket+10))
    {
        QString lResult = mTesterServer->Start(lSocket);
        if (lResult.startsWith("err"))
            ++lSocket;
        else
            lConnected = true;
    }

    if (lConnected)
    {
        GSLOG(SYSLOG_SEV_NOTICE,
              QString("TesterServer successfully listening to port %1").arg(lSocket).toLatin1().constData());
    }
    else
    {
        Message::warning("",
                         QString("Failed to listening to port in the range of %1 to %2")
                         .arg(lDefaultSocket).arg(lDefaultSocket+10).toLatin1().constData());
        return;
    }

    setEnabled(false);

    QString lInputFile      = mUi->lineEditDataFile->text();
    QString lRecipeFile     = mUi->lineEditRecipeFile->text();
    QString lTesterConfFile = Engine::GetInstance().Get("GalaxySemiFolder").toString() +
                              QDir::separator() + "temp" + QDir::separator() + "gtl_tester_temp.conf";

    if (CreateGtlTesterConf(lTesterConfFile, lSocket) == true)
    {
        QFileInfo   lFileInfo(lInputFile);
        QString     lOutputfile         = lFileInfo.absolutePath() + QDir::separator() +
                                          lFileInfo.completeBaseName() + "_patman." +
                                          lFileInfo.suffix();
        QString     lTraceabilityFile   = lFileInfo.absolutePath() + QDir::separator() +
                                          lFileInfo.completeBaseName() + "_traceability.sqlite";

        QThread *       gtsThread   = new QThread();
        PATGtsStation * gtsStation  = new PATGtsStation();

        gtsStation->SetInputDataFile(lInputFile);
        gtsStation->SetRecipeFile(lRecipeFile);
        gtsStation->SetTesterConf(lTesterConfFile);
        gtsStation->SetOutputDataFile(lOutputfile);
        gtsStation->SetTraceabilityFile(lTraceabilityFile);

        gtsStation->moveToThread(gtsThread);
        connect(gtsThread,  SIGNAL(started()),  gtsStation, SLOT(ExecuteTestProgram()));
        connect(gtsStation, SIGNAL(aborted(QString)),
                this,       SLOT(OnPATProcessingFailed(QString)));
        connect(gtsStation, SIGNAL(finished(QString, QString)),
                this,       SLOT(OnPATProcessingDone(QString, QString)));
        connect(gtsStation, SIGNAL(finished(QString, QString)),
                gtsThread,  SLOT(quit()));
        connect(gtsThread,  SIGNAL(finished()), gtsStation, SLOT(deleteLater()));
        connect(gtsThread,  SIGNAL(finished()), gtsThread,  SLOT(deleteLater()));
        connect(gtsStation, SIGNAL(dataReadProgress(QString, qint64,qint64)),
                this,       SLOT(OnStatusProgress(QString, qint64, qint64)));
        gtsThread->start();

        GSLOG(SYSLOG_SEV_NOTICE, QString("FT PAT processing started").toLatin1().constData());
    }
}

void PATWidgetFT::OnDetachWindow()
{
    ForceAttachWindow(mUi->buttonDetachWindow->isChecked());
}

void PATWidgetFT::OnEditRecipeFile()
{
    QString lRecipeFile = mUi->lineEditRecipeFile->text();

    // Clean input file name strings in case they come from drag & drop (start with string 'file:///'
    if(lRecipeFile.startsWith("file:///"))
        lRecipeFile = QUrl(lRecipeFile).toLocalFile();

    // Remove leading \r & \n if any
    lRecipeFile.replace("\r","");
    lRecipeFile.replace("\n","");

    // If file doesn't exists, simply ignore request!
    if(QFile::exists(lRecipeFile))
    {
        // Switch to the Spreadsheet-type sheet page (create it if needed)
        pGexMainWindow->Wizard_GexTb_EditPAT_Limits();

        // Detach the Editor window so we can still see the PAT: Process file wizard page
        GS::Gex::PATRecipeEditor::GetInstance().DetachRecipeEditor();

        // Re-Load Config file in the Spreasheet-type table & internal structures
        GS::Gex::PATRecipeEditor::GetInstance().RefreshEditor(lRecipeFile);

        // Force to reload the wizard PAT page
        pGexMainWindow->Wizard_FT_PAT();
    }
    else
        Message::information("", QString("Recipe file %1 does not exist").arg(lRecipeFile));
}

void PATWidgetFT::OnSelectTestDataFile()
{
    SelectDataFiles	lSelectFile;
    QSettings           lSettings;
    QString             lDataFile;

    // Get single Test Data File
    lDataFile = lSelectFile.GetSingleFile(this, lSettings.value("toolbox/workingFolder").toString(),
                                          "Select Test Data File to process");

    if(lDataFile.isEmpty() == false)
        SetTestDataFile(lDataFile);
}

void PATWidgetFT::OnSelectRecipeFile()
{
    // User wants to analyze a single file
    QSettings   lSettings;
    QString     lRecipeFile;

    lRecipeFile = QFileDialog::getOpenFileName(this, "Select recipe file",
                                               lSettings.value("editor/workingFolder").toString(),
                                               "Recipe file (*.csv *.json)");

    if(lRecipeFile.isEmpty() == false)
        SetRecipeFile(lRecipeFile);
}

void PATWidgetFT::OnPATProcessingDone(const QString& lOutputDataFile,
                                      const QString& lTraceabilityFile)
{
    mTesterServer->close();

    // Ask for hiding both progress bar and label status
    Engine::GetInstance().UpdateProgressStatus(true, 0, -1);
    Engine::GetInstance().UpdateLabelStatus();

    // Execute report
    int lReport = mUi->comboBoxReportFormat->itemData(mUi->comboBoxReportFormat->currentIndex()).toInt();

    if (lReport != ReportDisabled)
    {
        QString lReportFormat;

        if (lReport == ReportHTML)
            lReportFormat = "html";
        else if (lReport == ReportCsv)
            lReportFormat = "csv";
        else if (lReport == ReportWord)
            lReportFormat = "word";
        else if (lReport == ReportPpt)
            lReportFormat = "ppt";
        else if (lReport == ReportPdf)
            lReportFormat = "pdf";
        else if (lReport == ReportInteractive)
            lReportFormat =  "interactive";

        PATReportFT lPATReportFT;

        lPATReportFT.SetReportFormat(lReportFormat);
        if (lPATReportFT.Generate(QStringList(lOutputDataFile), lTraceabilityFile,
                                  mUi->lineEditRecipeFile->text()) == false)
        {
            Message::warning("", lPATReportFT.GetErrorMessage());
            setEnabled(true);
            return;
        }
    }

    // Post processing action on generared data file
    int lOutput = mUi->comboBoxOutputTestDataFormat->itemData(mUi->comboBoxOutputTestDataFormat->currentIndex()).toInt();

    if (lOutput == OutputDisabled)
        QFile::remove(lOutputDataFile);
    else if (lOutput == OutputATDF)
    {
        // Convert STDF to ATDF
        CSTDFtoATDF lATDFConverter(false);
        QFileInfo   lSTDFFileInfo(lOutputDataFile);
        QString     lATDFName   = lSTDFFileInfo.absolutePath() + "/" +
                                  lSTDFFileInfo.completeBaseName() + ".atd";

        lATDFConverter.SetProcessRecord(true);          // Convert ALL STDF records to ATDF
        lATDFConverter.SetWriteHeaderFooter(false);     // Disable DUMP comments in ATDF file.

        if (lATDFConverter.Convert(lOutputDataFile, lATDFName) == false)
        {
            QString lErrorMessage;
            lATDFConverter.GetLastError(lErrorMessage);

            lErrorMessage = "Failed to convert STDF output to ATDF: " + lErrorMessage;

            Message::warning("", lErrorMessage);
            setEnabled(true);
            return;
        }
        else
            // Delete intermadiate STDF file.
            QFile::remove(lOutputDataFile);
    }

    if (lReport == ReportDisabled)
        Message::information("", "Outlier processing successful!");

    setEnabled(true);
}

void PATWidgetFT::OnPATProcessingFailed(const QString &lErrorMessage)
{
    mTesterServer->close();

    // Ask for hiding both progress bar and label status
    Engine::GetInstance().UpdateProgressStatus(true, 0, -1);
    Engine::GetInstance().UpdateLabelStatus();

    Message::warning("", lErrorMessage);

    setEnabled(true);
}

void PATWidgetFT::OnStatusProgress(const QString& lMessage, qint64 lDone, qint64 lTotal)
{
    Engine::GetInstance().UpdateLabelStatus(lMessage);

    if (lDone == 0)
        Engine::GetInstance().UpdateProgressStatus(true, 100, 0);
    else
    {
        int lPercent = (int) (lDone * 100 / lTotal) ;

        Engine::GetInstance().UpdateProgressStatus(false, 100, lPercent);
    }
}

void PATWidgetFT::dragEnterEvent(QDragEnterEvent * lEvent)
{
    // Accept Drag if files list dragged over.
    if(lEvent->mimeData()->formats().contains("text/uri-list"))
        lEvent->acceptProposedAction();
}

void PATWidgetFT::dropEvent(QDropEvent *lEvent)
{
    if(lEvent->mimeData()->formats().contains("text/uri-list"))
    {
        QString		lFile;
        QStringList lFiles;
        QList<QUrl> lUrls = lEvent->mimeData()->urls();

        for (int lIdx = 0; lIdx < lUrls.count(); lIdx++)
        {
            lFile = lUrls.at(lIdx).toLocalFile();

            if (!lFile.isEmpty())
                lFiles << lFile;
        }

        // Insert first file selected into the listbox
        foreach(lFile, lFiles)
        {
            if (lFile.endsWith(".csv", Qt::CaseInsensitive) ||
                lFile.endsWith(".json", Qt::CaseInsensitive))
                SetRecipeFile(lFile);
            else
                SetTestDataFile(lFile);
        }

        lEvent->acceptProposedAction();
    }
    else
        lEvent->ignore();
}

bool PATWidgetFT::CreateGtlTesterConf(const QString &lTesterConf,
                                      int lSocket)
{
    QFileInfo   lFileInfo(lTesterConf);
    QDir        lTempDir;

    if(!lTempDir.exists(lFileInfo.absolutePath()))
    {
        if(!lTempDir.mkpath(lFileInfo.absolutePath()))
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Failed to create temporary directory %1").arg(lFileInfo.absolutePath())
                  .toLatin1().constData());
            return false;
        }
    }

    QFile       lFile(lTesterConf);

    if (lFile.open(QIODevice::WriteOnly))
    {
        lFile.write("[Server]\n");
        lFile.write("Name=localhost\n");
        lFile.write("IP=127.0.0.1\n");
        lFile.write(QString("SocketPort=%1\n").arg(lSocket).toLatin1());

        lFile.close();
    }
    else
    {
        GSLOG(SYSLOG_SEV_ERROR,
              QString("Failed to create temporary file %1").arg(lTesterConf).toLatin1().constData());
        return false;
    }

    return true;
}

}
}
#endif
