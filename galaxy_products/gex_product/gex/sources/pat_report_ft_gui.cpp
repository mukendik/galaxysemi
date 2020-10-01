#include "pat_report_ft_gui.h"
#include "ui_pat_report_ft_gui.h"
#include "pat_db_traceability_abstract.h"
#include "pat_recipe_io.h"
#include "pat_recipe.h"
#include "pat_options.h"
#include "browser_dialog.h"
#include "import_all.h"
#include "stdf.h"
#include "report_options.h"
#include "gqtl_log.h"
#include "stdf_content_utils.h"

#include <QSettings>
#include <QFileDialog>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QShortcut>

extern GexMainwindow *	pGexMainWindow;
extern CReportOptions   ReportOptions;
extern QString          ConvertToScriptString(const QString &strFile);
extern CGexReport *     gexReport;

namespace GS
{
namespace Gex
{

PATReportFTGui::PATReportFTGui(QWidget *parent)
    : QWidget(parent), mUi(new Ui::pat_report_ft_gui)
{
    mUi->setupUi(this);

    connect(mUi->listWidgetDataFiles,       SIGNAL(itemSelectionChanged()),
            this, SLOT(UpdateUI()));
    connect(mUi->lineEditTraceabilityFile,  SIGNAL(textChanged(QString)),
            this, SLOT(UpdateUI()));
    connect(mUi->buttonBrowseDataFile,      SIGNAL(clicked()),
            this,                           SLOT(OnSelectTestDataFile()));
    connect(mUi->buttonBrowseTraceability,  SIGNAL(clicked()),
            this,                           SLOT(OnSelectTraceabilityFile()));
    connect(mUi->buttonNext,                SIGNAL(clicked()),
            pGexMainWindow,                 SLOT(ViewSettings(void)));
    connect(mUi->buttonRemoveDataFile,      SIGNAL(clicked()),
            this,                           SLOT(OnRemoveDataFile()));

    // Create a key shortcut to remove data fime
    new QShortcut(Qt::Key_Delete, this, SLOT(OnRemoveDataFile()));

    // Install event filter on list widget to catch delete key press
    mUi->listWidgetDataFiles->installEventFilter(this);

    // Enable drops on lineEdit widget
    mUi->listWidgetDataFiles->setAcceptDrops(true);
    mUi->lineEditTraceabilityFile->setAcceptDrops(true);

    // Support for Drag&Drop
    setAcceptDrops(true);

    // Update UI
    UpdateUI();
}

PATReportFTGui::~PATReportFTGui()
{
    delete mUi;
}

bool PATReportFTGui::IsDataReady() const
{
    QStringList lOutputDataFiles = GetDataFiles();

    if (lOutputDataFiles.count() == 0 ||
        mUi->lineEditTraceabilityFile->text().isEmpty())
        return false;

    return true;
}

const QString &PATReportFTGui::GetWorkingPath() const
{
    return mWorkingPath;
}

QStringList PATReportFTGui::GetDataFiles() const
{
    QStringList lOutputDataFiles;

    for (int lIdx = 0; lIdx < mUi->listWidgetDataFiles->count(); ++lIdx)
    {
        if (mUi->listWidgetDataFiles->item(lIdx)->text().isEmpty() == false)
            lOutputDataFiles << mUi->listWidgetDataFiles->item(lIdx)->text();
    }

    return lOutputDataFiles;
}

QString PATReportFTGui::GetTraceabilityFile() const
{
    return mUi->lineEditTraceabilityFile->text();
}

void PATReportFTGui::SetWorkingPath(const QString &lWorkingPath)
{
    mWorkingPath = lWorkingPath;
}

void PATReportFTGui::WriteProcessFilesSection(FILE *hFile)
{
    if (hFile)
    {
        QString lTestMerge          = "merge";
        QString lTraceabilityFile   = GetTraceabilityFile();
        QString lErroMessage;

        // If Traceability file is correctly read, extract recipe content
        QScopedPointer<PATDbTraceabilityAbstract> lDbTraceability(PATDbTraceabilityAbstract::CreateDbTraceability(gexReport,
                                                                                                                  lTraceabilityFile,
                                                                                                                  lErroMessage));

        if (lDbTraceability.isNull() == false)
        {
            QString lRecipeFileName;
            QString lRecipeContent;

            // Extract the recipe content from traceability file, if not exists or invalid, use default test merge option
            if (lDbTraceability->QueryRecipeContent(-1, lRecipeFileName, lRecipeContent) == true)
            {
                QTextStream lRecipeStream(&lRecipeContent, QIODevice::ReadOnly);

                QScopedPointer<PATRecipeIO> lRecipeIO(PATRecipeIO::CreateRecipeIo(lRecipeStream));

                if (lRecipeIO.isNull() == false)
                {
                    PATRecipe lPATRecipe;
                    if (lRecipeIO->Read(lRecipeContent.toLatin1(), lPATRecipe) == true)
                    {
                        switch(lPATRecipe.GetOptions().mTestKey)
                        {
                            case GEX_TBPAT_KEY_TESTNUMBER:
                                // Merge tests with same test number (even if test name is different)
                                lTestMerge = "merge";
                                break;

                            case GEX_TBPAT_KEY_TESTNAME:
                                // Merge tests with same test name (even if test number is different)
                                lTestMerge = "merge_name";
                                break;

                            case GEX_TBPAT_KEY_TESTMIX:
                                // Merge tests with same test number and test name
                                lTestMerge = "no_merge";
                                break;

                            default:
                                // Use default option from options tab
                                break;
                        }
                    }
                    else
                    {
                        GSLOG(SYSLOG_SEV_WARNING,
                              QString("Bad recipe found: %1").arg(lRecipeIO->GetErrorMessage()).toLatin1().constData());
                    }
                }
                else
                {
                    GSLOG(SYSLOG_SEV_WARNING, "Unsupported recipe content found in traceability file");
                }
            }
            else
            {
                GSLOG(SYSLOG_SEV_WARNING, lDbTraceability->GetErrorMessage().toLatin1().constData());
            }
        }
        else
        {
            GSLOG(SYSLOG_SEV_WARNING, lErroMessage.toLatin1().constData());
        }

        // Force some FT PAT specific option
        // If no PAT limits clamping
        // Disable Examinator outlier GUI filter!
        fprintf(hFile,"  gexOptions('dataprocessing','data_cleaning_mode','none');\n");
        // Merge tests option
        fprintf(hFile,"  gexOptions('dataprocessing', 'duplicate_test', '%s');\n", lTestMerge.toLatin1().constData());

        // Forces to ALWAYS compute advanced statistics.
        fprintf(hFile,"  gexOptions('speed','adv_stats','always');\n");

        // Disables PartID support (uses less memory this way,
        // and not needed in reports created)
        fprintf(hFile,"  gexOptions('dataprocessing','part_id','hide');\n");

        // Force to compute statistics from samples, ignore summary.
        fprintf(hFile,"  gexOptions('statistics','computation','samples_only');\n");
        fprintf(hFile,"  gexOptions('pareto','section','');\n");

        // Force computing binning from wafermap (so to avoid counting retest multiple times.
        fprintf(hFile,"  gexOptions('binning','computation','samples');\n");

        // Disable STDF.FTR processing (for PAT speed improvement
        // since FTR are not used in PAT algorihtms)
        fprintf(hFile,"  gexOptions('dataprocessing','functional_tests','disabled');\n");
        // Force Part identification to use part_id
        fprintf(hFile,"  gexOptions('dataprocessing', 'part_identification', 'part_id');\n");
        // Force data file order to be done based on the date
        fprintf(hFile,"  gexOptions('dataprocessing', 'sorting', 'date');\n");
        // Force to split MPR
        fprintf(hFile,"  gexOptions('dataprocessing','multi_parametric_merge_mode','no_merge');\n");
        // Force Trend chart to be on run_id instead of part_id
        fprintf(hFile,"  gexOptions('adv_trend','x_axis','run_id');\n");

        // Dataset definition
        fprintf(hFile, "  gexGroup('reset','all');\n");

        // Split file over each site
        QStringList lOutputDataFiles;

        for (int lIdx = 0; lIdx < mUi->listWidgetDataFiles->count(); ++lIdx)
            lOutputDataFiles << mUi->listWidgetDataFiles->item(lIdx)->text();

        QString     lSTDFCompliancy = ReportOptions.GetOption("dataprocessing", "stdf_compliancy").toString();
        bool        lValidSiteOnly  = false;
        QList<int>  lSites;

        if (lSTDFCompliancy == "stringent")
            lValidSiteOnly = true;

        if (lOutputDataFiles.isEmpty())
        {
            GSLOG(SYSLOG_SEV_ERROR, "No input data files provided");
            return;
        }

        std::vector<int> lSite;
        if (GS::Gex::StdfContentUtils::GetSites((std::string)(lOutputDataFiles.first().toLatin1().constData()),
                                                lSite,
                                                lValidSiteOnly) == false)
        {
            GSLOG(SYSLOG_SEV_ERROR, "Error : Cannot extract site from input file");
            return;
        }
        lSites = QList<int>::fromVector( QVector<int>::fromStdVector(lSite));

        // Write dataset group
        // Check if single-site or multi-sites...
        if(lSites.count() > 1)
        {
            for (QList<int>::const_iterator iter = lSites.begin(); iter != lSites.end(); ++iter)
            {
                fprintf(hFile, "  group_id = gexGroup('insert','Site %d');\n", *iter);

                for (int lIdx = 0; lIdx < lOutputDataFiles.count(); ++lIdx)
                {
                    fprintf(hFile,
                            "  gexFile(group_id,'insert','%s','%d','last_instance',' ','','',' ');\n\n",
                            ConvertToScriptString(lOutputDataFiles.at(lIdx)).toLatin1().constData(),
                            *iter);
                }
            }
        }
        else
        {
            fprintf(hFile, "  group_id = gexGroup('insert','DataSet_1');\n");

            for (int lIdx = 0; lIdx < lOutputDataFiles.count(); ++lIdx)
            {
                fprintf(hFile,
                        "  gexFile(group_id,'insert','%s','all','last_instance',' ','','',' ');\n\n",
                        ConvertToScriptString(lOutputDataFiles.at(lIdx)).toLatin1().constData());
            }
        }
    }
}

void PATReportFTGui::UpdateUI()
{
    mUi->buttonRemoveDataFile->setEnabled(mUi->listWidgetDataFiles->selectedItems().count() > 0);
    mUi->buttonNext->setEnabled(IsDataReady());
}

void PATReportFTGui::OnRemoveDataFile()
{
    QList<QListWidgetItem*> lSelectedItems = mUi->listWidgetDataFiles->selectedItems();

    for (int lIdx = 0; lIdx < lSelectedItems.count(); ++lIdx)
        delete mUi->listWidgetDataFiles->takeItem(mUi->listWidgetDataFiles->row(lSelectedItems.at(lIdx)));
}

void PATReportFTGui::OnSelectTestDataFile()
{
    SelectDataFiles	lSelectFile;
    QSettings           lSettings;
    QStringList         lDataFiles;

    // Get single Test Data File
    lDataFiles = lSelectFile.GetFiles(this, lSettings.value("editor/workingFolder").toString(),
                                          "Select Initial Test data files");

    if(lDataFiles.isEmpty() == false)
    {
        mUi->listWidgetDataFiles->addItems(lDataFiles);
        UpdateUI();
    }
}

void PATReportFTGui::OnSelectTraceabilityFile()
{
    // User wants to analyze a single file
    QSettings   lSettings;
    QString     lTraceabilityFile;

    lTraceabilityFile = QFileDialog::getOpenFileName(this, "Select PAT traceability file",
                                               lSettings.value("editor/workingFolder").toString(),
                                               "Traceability file (*.*)");

    if(lTraceabilityFile.isEmpty() == false)
        mUi->lineEditTraceabilityFile->setText(lTraceabilityFile);
}

void PATReportFTGui::dragEnterEvent(QDragEnterEvent *lEvent)
{
    // Accept Drag if files list dragged over.
    if((lEvent->mimeData()->formats().contains("text/uri-list")))
        lEvent->acceptProposedAction();
}

void PATReportFTGui::dropEvent(QDropEvent *lEvent)
{
    if((lEvent->mimeData()->formats().contains("text/uri-list")))
    {
        QString		lFile;
        QStringList lFiles;
        QList<QUrl> lUrls = lEvent->mimeData()->urls();

        for (int lIdx = 0; lIdx < lUrls.count(); ++lIdx)
        {
            lFile = lUrls.at(lIdx).toLocalFile();

            if (!lFile.isEmpty())
                lFiles << lFile;
        }

        // Insert first file selected into the listbox
        foreach(lFile, lFiles)
        {
            if (lFile.endsWith(".sqlite", Qt::CaseInsensitive))
                mUi->lineEditTraceabilityFile->setText(lFile);
            else
                mUi->listWidgetDataFiles->addItem(lFile);
        }

        lEvent->acceptProposedAction();

        UpdateUI();
    }
}

bool PATReportFTGui::eventFilter(QObject *lObj, QEvent *lEvent)
{
    if (lObj == mUi->listWidgetDataFiles)
    {
        if (lEvent->type() == QEvent::KeyRelease)
        {
            QKeyEvent * lKeyEvent = static_cast<QKeyEvent *>(lEvent);

            if (lKeyEvent->key() == Qt::Key_Delete)
                OnRemoveDataFile();
            return true;
        }
    }

    // standard event processing
    return QObject::eventFilter(lObj, lEvent);
}

}
}
