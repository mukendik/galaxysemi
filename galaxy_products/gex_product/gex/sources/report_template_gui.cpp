/******************************************************************************!
 * \file report_template_gui.cpp
 ******************************************************************************/

#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QDragEnterEvent>
#include <QDropEvent>

#include "report_template_gui.h"
#include "report_template.h"
#include "report_template_io.h"
#include "browser_dialog.h"
#include "engine.h"
#include "picktest_dialog.h"
#include "gex_pixmap_extern.h"
#include "settings_dialog.h"
#include "product_info.h"
#include <gqtl_log.h>
#include "pick_audit_filter_dialog.h"
#include "csl/csl_engine.h"
#include "script_wizard.h"
#include "gex_report.h"
#include "report_options.h"
#include "gex_group_of_files.h"
#include "gex_file_in_group.h"
#include "message.h"

// Selecting Custom report section to create
#define GEX_CRPT_SECTION_NONE           0
#define GEX_CRPT_SECTION_TEST_AGGREGATE 1
#define GEX_CRPT_SECTION_TEST_WAFMAP    2
#define GEX_CRPT_SECTION_TEST_BINNING   3
#define GEX_CRPT_SECTION_TEST_PARETO    4
#define GEX_CRPT_SECTION_TEST_PEARSON   5
#define GEX_CRPT_SECTION_CORR_GB        6
#define GEX_CRPT_SECTION_PRODUCTION     7
#define GEX_CRPT_SECTION_FILE_AUDIT     8
#define GEX_CRPT_SECTION_GLOBAL_INFO    9
#define GEX_CRPT_SECTION_DATALOG        10

#define GEX_VALID_IMAGEFILES_TYPES "Image Files (*.bmp *.gif *.png *.jpg)"

// In main.cpp
extern GexMainwindow* pGexMainWindow;
extern CReportOptions ReportOptions;

// In script_wizard.h
extern void ConvertToScriptString(QString& strFile);

// In report_build.h
extern CGexReport* gexReport;  // Handle to report class

namespace GS
{
namespace Gex
{

/******************************************************************************!
 * \fn getReportTemplate
 ******************************************************************************/
ReportTemplate* ReportTemplateGui::getReportTemplate()
{
    if (mReportTemplate == NULL)
    {
        mReportTemplate = new ReportTemplate;
    }
    return mReportTemplate;
}

/******************************************************************************!
 * \fn getReportTemplateIO
 ******************************************************************************/
ReportTemplateIO* ReportTemplateGui::getReportTemplateIO()
{
    if (mReportTemplateIO == NULL)
    {
        mReportTemplateIO = new ReportTemplateIO;
    }
    return mReportTemplateIO;
}

/******************************************************************************!
 * \fn ReportTemplateGui
 * \brief Report Template Wizard/editor
 ******************************************************************************/
ReportTemplateGui::ReportTemplateGui(QWidget*   parent,
                                     bool       modal,
                                     Qt::WindowFlags fl)
    : QDialog(parent, fl),
      mReportTemplate(NULL),
      mReportTemplateIO(NULL)
{
    setupUi(this);
    setModal(modal);

    // Apply Examinator palette
    GexMainwindow::applyPalette(this);

    // No sorting on list view
    ListView->setSortingEnabled(false);

    // Empty list at start-up
    ListView->clear();

    // Support for Drag&Drop
    setAcceptDrops(true);

    // Startup
    m_bTemplateModified = false;

    // Connect signals
    connect((QObject*) pushButtonOnOk,
            SIGNAL(clicked()), SLOT(OnOk()));
    connect((QObject*) buttonLoadReportTemplate,
            SIGNAL(clicked()), SLOT(OnLoadReportTemplate()));
    connect((QObject*) buttonSaveReportTemplate,
            SIGNAL(clicked()), SLOT(OnSaveAsReportTemplate()));
    connect((QObject*) buttonAddLogo,
            SIGNAL(clicked()), SLOT(OnSelectLogo()));
    connect((QObject*) buttonRemoveLogo,
            SIGNAL(clicked()), SLOT(OnRemoveLogo()));
    connect((QObject*) buttonAddSection,
            SIGNAL(clicked()), SLOT(OnNewSection()));
    connect((QObject*) PushButtonAutoFill,
            SIGNAL(clicked()), SLOT(OnAutoFill()));
    connect((QObject*) buttonDuplicateSection,
            SIGNAL(clicked()), SLOT(OnDuplicateSection()));
    connect((QObject*) buttonRemoveSection,
            SIGNAL(clicked()), SLOT(OnDeleteSection()));
    connect((QObject*) PushButtonClearAll,
            SIGNAL(clicked()), SLOT(OnDeleteAllSections()));
    connect((QObject*) buttonUp,
            SIGNAL(clicked()), SLOT(OnMoveSectionUp()));
    connect((QObject*) buttonDown,
            SIGNAL(clicked()), SLOT(OnMoveSectionDown()));

    // Edit existing entry
    connect((QObject*) buttonEditSection,
            SIGNAL(clicked()), SLOT(OnEditSection()));
    connect(ListView,
            SIGNAL(doubleClicked(QModelIndex)), SLOT(OnEditSection()));

    // If HTML Home page text edited, make sure we'll update
    // the template on exit
    connect((QObject*) textHtml,
            SIGNAL(textChanged()), SLOT(OnTemplateModified()));

    // Resize List view columns
    ListView->setColumnWidth(0, 150);  // Room for: Report Section title

    // Before UpdateGUI
    mReportTemplate = new ReportTemplate;

    // Update GUI fields
    UpdateGUI();
}

/******************************************************************************!
 * \fn ~ReportTemplateGui_EditSection
 * \brief Destructor
 ******************************************************************************/
ReportTemplateGui::~ReportTemplateGui()
{
    if (mReportTemplateIO != NULL) {
        delete mReportTemplateIO;
    }
    if (mReportTemplate != NULL) {
        delete mReportTemplate;
    }
}

/******************************************************************************!
 * \fn OnTemplateModified
 * \brief Connected to some signal when GUI / Template is changed
 ******************************************************************************/
void ReportTemplateGui::OnTemplateModified()
{
    // Mark to save Template file on exit
    m_bTemplateModified = true;
}

/******************************************************************************!
 * \fn OnOk
 ******************************************************************************/
void ReportTemplateGui::OnOk()
{
    if (m_bTemplateModified)
    {
        OnSaveReportTemplate(true);
    }

    done(1);
}

/******************************************************************************!
 * \fn reject
 * \brief ReportTemplateGui: Empty function to ignore the ESCAPE key hit
 ******************************************************************************/
void ReportTemplateGui::reject()
{
    done(0);
}

/******************************************************************************!
 * \fn dragEnterEvent
 * \brief Starting DRAG sequence
 ******************************************************************************/
void ReportTemplateGui::dragEnterEvent(QDragEnterEvent* e)
{
    // Accept Drag if files list dragged over
    if (e->mimeData()->formats().contains("text/uri-list"))
    {
        e->acceptProposedAction();
    }
}

/******************************************************************************!
 * \fn dropEvent
 * \brief Dropping files in Widget (Drag & Drop)
 ******************************************************************************/
void ReportTemplateGui::dropEvent(QDropEvent* e)
{
    if (! e->mimeData()->formats().contains("text/uri-list"))
    {
        // No files dropped... ignore drag&drop
        e->ignore();
        return;
    }

    QString     strFileName;
    QStringList strFileList;
    QList<QUrl> lstUrls = e->mimeData()->urls();

    for (int nUrl = 0; nUrl < lstUrls.count(); nUrl++)
    {
        strFileName = lstUrls.at(nUrl).toLocalFile();

        if (! strFileName.isEmpty())
        {
            strFileList << strFileName;
        }
    }

    if (strFileList.count() <= 0)
    {
        // Items dropped are not regular files... ignore
        e->ignore();
        return;
    }

    // Load Report Template file...unless file extension is NOT '.grt'
    QString strFile = strFileList[0];
    if (strFile.endsWith(".grt", Qt::CaseInsensitive))
    {
        OnLoadReportTemplate(strFile);
    }

    e->acceptProposedAction();
}

/******************************************************************************!
 * \fn UpdateGUI
 * \brief Refresh GUI fields
 ******************************************************************************/
void ReportTemplateGui::UpdateGUI()
{
    // Template file
    QString strTemplateFile = "";
    if (m_strTemplateFile.isEmpty() == false)
    {
        strTemplateFile = "<b>Template file:</b> " + m_strTemplateFile;
    }
    textLabelTemplateFile->setText(strTemplateFile);

    // Frontpage text & logo
    SetSelection(mReportTemplate->GetFrontPageText(),
                 mReportTemplate->GetFrontPageImage());

    // Show/hide Admin icons
    int iEntries = ListView->topLevelItemCount();
    if (iEntries <= 0)
    {
        // Empty list
        buttonSaveReportTemplate->hide();
        Line2->hide();
        buttonEditSection->hide();
        buttonDuplicateSection->hide();
        Line3->hide();
        buttonRemoveSection->hide();
        PushButtonClearAll->hide();
        Line4->hide();
        buttonUp->hide();
        buttonDown->hide();

        // Disable the Wizard 'Next >>' button
        pushButtonOnOk->setEnabled(false);
    }
    else if (iEntries == 1)
    {
        // Only one item in list
        buttonSaveReportTemplate->show();
        Line2->show();
        buttonEditSection->show();
        buttonDuplicateSection->show();
        Line3->show();
        buttonRemoveSection->show();
        PushButtonClearAll->hide();
        Line4->hide();
        buttonUp->hide();
        buttonDown->hide();

        // Enable the Wizard 'Next >>' button
        pushButtonOnOk->setEnabled(true);
    }
    else
    {
        // Multile items in list
        buttonSaveReportTemplate->show();
        Line2->show();
        buttonEditSection->show();
        buttonDuplicateSection->show();
        Line3->show();
        buttonRemoveSection->show();
        PushButtonClearAll->show();
        Line4->show();
        buttonUp->show();
        buttonDown->show();

        // Enable the Wizard 'Next >>' button
        pushButtonOnOk->setEnabled(true);
    }
}

/******************************************************************************!
 * \fn UpdateGuiSectionsList
 * \brief Recomputes & display the Template sections list
 ******************************************************************************/
void ReportTemplateGui::UpdateGuiSectionsList()
{
    // Empty existing list view
    ListView->clear();

    ReportTemplateSection* pNewSection = NULL;
    ReportTemplateSectionIterator iter = mReportTemplate->End();
    while (iter != mReportTemplate->Begin())
    {
        --iter;
        pNewSection = *iter;

        // insert section in list view
        // (done in revert order to compesate innsertion reversion)
        QStringList labels;
        labels.append(pNewSection->getSectionTitle());
        labels.append(pNewSection->getTestList());
        labels.append(pNewSection->getCharts());
        new QTreeWidgetItem(ListView,
                            labels);
    }
}

/******************************************************************************!
 * \fn OnLoadReportTemplate
 * \brief Manually select a template
 ******************************************************************************/
void ReportTemplateGui::OnLoadReportTemplate()
{
    OnLoadReportTemplate("");
}

/******************************************************************************!
 * \fn OnLoadReportTemplate
 * \brief Load Template from disk
 ******************************************************************************/
bool
ReportTemplateGui::OnLoadReportTemplate(QString strTemplateFile  /*=""*/,
                                        bool    bQuietLoad  /*=false*/)
{
    // If no file selected yet, prompt user to select Template file to load
    if (QFile::exists(strTemplateFile) == false)
    {
        // Specified file doesn't exist, quietly return
        if (bQuietLoad)
        {
            return false;
        }

        QString strDefaultSelection = m_strTemplateFile;

        // Prompt user to select file
        strTemplateFile = QFileDialog::getOpenFileName(
            this,
            "Select Report Template File",
            strDefaultSelection,
            "Quantix Report Template (*.grt)");

        if (strTemplateFile.isEmpty())
        {
            return false;
        }
    }

    // Save selection
    m_strTemplateFile = strTemplateFile;

    // Clear memory (remove any existing template)
    mReportTemplate->Clear();

    // Load template from disk
    this->getReportTemplateIO()->
        ReadTemplateFromDisk(mReportTemplate, strTemplateFile);
    this->UpdateGuiSectionsList();

    // Flag to tell template saved
    m_bTemplateModified = false;

    // Update GUI
    UpdateGUI();

    // Success
    return true;
}

/******************************************************************************!
 * \fn OnSaveAsReportTemplate
 * \brief Save temaplte to disk: 'Save as...'
 ******************************************************************************/
void ReportTemplateGui::OnSaveAsReportTemplate()
{
    // Ask to confirm output file name when saving Template file
    OnSaveReportTemplate(true);
}

/******************************************************************************!
 * \fn OnSaveReportTemplate
 * \brief Save temaplte to disk
 ******************************************************************************/
void ReportTemplateGui::OnSaveReportTemplate()
{
    // Ask to confirm output file name when saving Template file
    OnSaveReportTemplate(false);
}

/******************************************************************************!
 * \fn OnSaveReportTemplate
 * \brief Save Template to disk
 ******************************************************************************/
void ReportTemplateGui::OnSaveReportTemplate(bool bAskConfirm)
{
    // If need to define output file name
    if (bAskConfirm || m_strTemplateFile.isEmpty())
    {
        // Lets user tell where to save the XML Template file
        QString strSaveAs;
        QString strFileName = m_strTemplateFile;
        if (strFileName.isEmpty())
        {
            strFileName = "quantix_template.grt";
        }
        strSaveAs = QFileDialog::getSaveFileName(
                this,
                "Save Report Template as...",
                strFileName,
                "Quantix Report Template (*.grt)",
                NULL,
                QFileDialog::
                DontConfirmOverwrite);

        // If no file selected, ignore command
        if (strSaveAs.isEmpty())
        {
            return;
        }

        // If file already exists, request confirmation to overwrite
        // File exists, overwrite it ?
        if (QFile::exists(strSaveAs))
        {
            bool lOk;
            GS::Gex::Message::request("", "File already exists. Overwrite it ?", lOk);
            if (! lOk)
            {
                return;
            }
        }

        // Keep track of changes (in case new name used)
        m_strTemplateFile = strSaveAs;
    }

    // Save template file to disk
    if (this->getReportTemplateIO()->
        WriteTemplateToDisk(mReportTemplate, m_strTemplateFile, this) == false)
    {
        return;
    }

    // Flag to tell if template requires to be saved
    OnTemplateModified();

    // Update GUI
    UpdateGUI();
}

/******************************************************************************!
 * \fn OnSelectLogo
 * \brief Select a new logo image
 ******************************************************************************/
void ReportTemplateGui::OnSelectLogo()
{
    QString strFile =
        QFileDialog::getOpenFileName(this,
                                     "Select Image",
                                     mReportTemplate->GetFrontPageImage(),
                                     GEX_VALID_IMAGEFILES_TYPES);

    if (strFile.isEmpty())
    {
        return;  // No selection made

    }
    // Update screen: Repaint logo image
    SetSelection("", strFile);

    // Flag to tell if template requires to be saved
    OnTemplateModified();
}

/******************************************************************************!
 * \fn OnRemoveLogo
 * \brief Unselect logo (remove from home page)
 ******************************************************************************/
void ReportTemplateGui::OnRemoveLogo()
{
    // Update screen: Repaint with NO logo image
    SetSelection("", "");

    // Flag to tell if template requires to be saved
    OnTemplateModified();
}

/******************************************************************************!
 * \fn SetSelection
 * \brief Set text + Logo to be used on report's front page
 ******************************************************************************/
void ReportTemplateGui::SetSelection(QString strFrontPageText,
                                     QString strFrontPageImage)
{
    // Update the text window
    if (strFrontPageText.isEmpty() == false)
    {
        textHtml->setPlainText(strFrontPageText);
    }

    // Update the logo image
    mReportTemplate->SetFrontPageImage(strFrontPageImage);
    TextLabelPreviewStatus->hide();
    if (strFrontPageImage.isEmpty() == false)
    {
        QPixmap pPixmap(strFrontPageImage);
        pixmapLogo->setPixmap(pPixmap);
        QFileInfo cFileInfo(strFrontPageImage);
        TextLabelImageName->setText(cFileInfo.fileName());

        // If .GIF file, says we can't display it here, but we support it
        if (strFrontPageImage.endsWith(".gif", Qt::CaseInsensitive))
        {
            TextLabelPreviewStatus->show();
            pixmapLogo->hide();
        }
        else
        {
            TextLabelPreviewStatus->hide();
            pixmapLogo->show();
        }
    }
    else
    {
        pixmapLogo->clear();  // No image to show: clear window
        TextLabelImageName->clear();
    }
}

/******************************************************************************!
 * \fn getSectionDetails
 * \brief Extract Section details from GUI fields
 ******************************************************************************/
void ReportTemplateGui_EditSection::getSectionDetails(
    ReportTemplateSection* pSection,
    bool                   bCopy  /*=false*/)
{
    QString strString;
    double  lfValue;
    bool    bFlag;
    int     iIndex;

    // Check which widget is visible
    switch (widgetStackSectionType->currentIndex())
    {
    case GEX_CRPT_WIDGET_AGGREGATE:
        // Create structure to hold 'Aggregate' settings
        pSection->pAggregate = new CustomReportTestAggregateSection();

        // Saves Section type
        pSection->iSectionType = GEX_CRPT_WIDGET_AGGREGATE;

        // Section's title
        pSection->pAggregate->strSectionTitle = editSectionTitle->text();

        // Check if copy (then update title)
        if (bCopy)
        {
            pSection->pAggregate->strSectionTitle = "Copy of " +
                pSection->pAggregate->strSectionTitle;
        }

        // Type of tests (fail, all, test list, ...)
        pSection->pAggregate->iTestType = comboBoxTestList->currentIndex();

        // Test list
        pSection->pAggregate->strRangeList = editTestStats->text();

        // Include Test Statistics table ?
        pSection->pAggregate->bStats = checkBoxStats->isChecked();

        // Charting mode: one overmay chart, one chart per test, both ?
        pSection->pAggregate->iChartMode = comboChartingType->currentIndex();

        // Include Histogram ?
        if (checkBoxHistogram->isChecked() == false)
        {
            pSection->pAggregate->iHistogramType = -1;
        }
        else
        {
            pSection->pAggregate->iHistogramType =
                comboHistogram->currentIndex();
        }

        // Include Trend ?
        if (checkBoxTrend->isChecked() == false)
        {
            pSection->pAggregate->iTrendType = -1;
        }
        else
        {
            pSection->pAggregate->iTrendType = comboTrend->currentIndex();
        }

        // Include Scatter ?
        if (checkBoxScatter->isChecked() == false)
        {
            pSection->pAggregate->iScatterType = -1;
        }
        else
        {
            pSection->pAggregate->iScatterType  = comboScatter->currentIndex();
            pSection->pAggregate->strRangeListY = editTestY->text();
        }

        // Include Probability plot ?
        if (checkBoxProbability->isChecked() == false)
        {
            pSection->pAggregate->iProbabilityPlotType = -1;
        }
        else
        {
            pSection->pAggregate->iProbabilityPlotType =
                comboProbabilityPlot->currentIndex();
        }

        // Include box plot extended ?
        if (checkBoxBoxplotEx->isChecked() == false)
        {
            pSection->pAggregate->iBoxplotExType = -1;
        }
        else
        {
            pSection->pAggregate->iBoxplotExType =
                comboBoxplotEx->currentIndex();
        }

        // Include Multi-charts ?
        if (checkBoxMultiChart->isChecked() == false)
        {
            pSection->pAggregate->iMultiChartsType = -1;
        }
        else
        {
            pSection->pAggregate->iMultiChartsType =
                comboBoxMultiChart->currentIndex();
        }

        // Include BoxPlot ?
        if (checkBoxBoxPlot->isChecked() == false)
        {
            pSection->pAggregate->iBoxPlotType = -1;
        }
        else
        {
            pSection->pAggregate->iBoxPlotType = 0;
        }

        // Viewport & Legends
        pSection->pAggregate->bChartTitle   = groupBoxChartTitle->isChecked();
        pSection->pAggregate->strChartTitle = lineEditChartTitle->text();

        pSection->pAggregate->bLegendX = groupBoxChartLegendX->isChecked();
        pSection->pAggregate->strAxisLegendX = lineEditAxisLegendX->text();

        pSection->pAggregate->bLegendY = groupBoxChartLegendY->isChecked();
        pSection->pAggregate->strAxisLegendY = lineEditAxisLegendY->text();

        // LowX/Y, High X/Y
        lfValue = lineEditLowX->text().toDouble(&bFlag);
        if (bFlag)
        {
            pSection->pAggregate->lfLowX = lfValue;
        }
        lfValue = lineEditLowY->text().toDouble(&bFlag);
        if (bFlag)
        {
            pSection->pAggregate->lfLowY = lfValue;
        }
        lfValue = lineEditHighX->text().toDouble(&bFlag);
        if (bFlag)
        {
            pSection->pAggregate->lfHighX = lfValue;
        }
        lfValue = lineEditHighY->text().toDouble(&bFlag);
        if (bFlag)
        {
            pSection->pAggregate->lfHighY = lfValue;
        }

        // Logscale X-axis
        bFlag = (comboBoxScaleX->currentIndex() != 0) ? true : false;
        pSection->pAggregate->bLogScaleX = bFlag;

        // Logscale Y-axis
        bFlag = (comboBoxScaleY->currentIndex() != 0) ? true : false;
        pSection->pAggregate->bLogScaleY = bFlag;

        pSection->pAggregate->cColor = cColor;

        // Plotting style
        // Pen width
        pSection->pAggregate->iLineWidth = spinBoxLineWidth->value();
        // Line Style: dashed, plain, etc.
        pSection->pAggregate->iLineStyle = comboBoxLineStyle->currentIndex();
        // Spot style: circle, triangle, etc.
        pSection->pAggregate->iSpotStyle =
            comboBoxLineSpotStyle->currentIndex();
        // true if draw with BARS (histogram charting only)
        pSection->pAggregate->bBoxBars = checkBoxBars->isChecked();
        // true if draw fitting curve / spin
        pSection->pAggregate->bFittingCurve = checkBoxfittingCurve->isChecked();
        // true if draw Guaussian Bell-curve shape
        pSection->pAggregate->bBellCurve = checkBoxBellCurve->isChecked();
        // true if connect points with a line
        pSection->pAggregate->bLines = checkBoxLines->isChecked();
        // true if draw a spot at each data point
        pSection->pAggregate->bSpots = checkBoxspots->isChecked();
        // Array of Marker colors & width
        for (iIndex = 0; iIndex < GEX_CRPT_MARKER_TOTALSIZE; iIndex++)
        {
            pSection->pAggregate->icMarkerWidth[iIndex] = icMarkerWidth[iIndex];
            pSection->pAggregate->cMarkerColor[iIndex]  = cMarkerColor[iIndex];
        }
        break;

    case GEX_CRPT_WIDGET_WAFMAP:
        // Create structure to hold 'Wafer map' settings
        pSection->pWafmap = new CGexCustomReport_WafMap_Section();

        // Saves Section type
        pSection->iSectionType = GEX_CRPT_WIDGET_WAFMAP;

        // Section's title
        pSection->pWafmap->strSectionTitle = editSectionTitle->text();

        // Check if copy (then update title)
        if (bCopy)
        {
            pSection->pWafmap->strSectionTitle = "Copy of " +
                pSection->pWafmap->strSectionTitle;
        }

        // Type of tests (fail, all, test list, ...)
        pSection->pWafmap->iWafermapType = comboBoxWafmap->currentIndex();

        // Test list
        pSection->pWafmap->strRangeList = editWafmap->text();

        switch (pSection->pWafmap->iWafermapType + 1)
        {
        default:
            pSection->pWafmap->iWafermapType = GEX_WAFMAP_STACK_SOFTBIN - 1;
        case GEX_WAFMAP_STACK_SOFTBIN:
        case GEX_WAFMAP_SOFTBIN:  // Standard Wafer map: Software Binning
        case GEX_WAFMAP_ZONAL_SOFTBIN:
        case GEX_WAFMAP_HARDBIN:  // Standard Wafer map: Hardware Binning
        case GEX_WAFMAP_STACK_HARDBIN:
        case GEX_WAFMAP_ZONAL_HARDBIN:
            if (pSection->pWafmap->strRangeList.isEmpty())
            {
                pSection->pWafmap->strRangeList = "1";
            }
            break;
        case GEX_WAFMAP_TESTOVERLIMITS:  // zoning on test limits
        case GEX_WAFMAP_STACK_TESTOVERLIMITS:
        case GEX_WAFMAP_TESTOVERDATA:  // zoning on test values range
        case GEX_WAFMAP_STACK_TESTOVERDATA:
        case GEX_WAFMAP_TEST_PASSFAIL:  // zoning on test values range
        case GEX_WAFMAP_STACK_TEST_PASSFAIL:
            if (pSection->pWafmap->strRangeList.isEmpty())
            {
                pSection->pWafmap->strRangeList = "1000";
            }
            break;
        }
        break;

    case GEX_CRPT_WIDGET_BINNING:
        // Create structure to hold 'Binning' settings
        pSection->pBinning = new CGexCustomReport_Binning_Section();

        // Saves Section type
        pSection->iSectionType = GEX_CRPT_WIDGET_BINNING;

        // Section's title
        pSection->pBinning->strSectionTitle = editSectionTitle->text();

        // Check if copy (then update title)
        if (bCopy)
        {
            pSection->pBinning->strSectionTitle = "Copy of " +
                pSection->pBinning->strSectionTitle;
        }
        break;

    case GEX_CRPT_WIDGET_PARETO:
        // Create structure to hold 'Pareto' settings
        pSection->pPareto = new CGexCustomReport_Pareto_Section();

        // Saves Section type
        pSection->iSectionType = GEX_CRPT_WIDGET_PARETO;

        // Section's title
        pSection->pPareto->strSectionTitle = editSectionTitle->text();
        break;

    case GEX_CRPT_WIDGET_PEARSON:
        // Create structure to hold 'Pearson' settings
        pSection->pPearson = new CGexCustomReport_Pearson_Section();

        // Saves Section type
        pSection->iSectionType = GEX_CRPT_WIDGET_PEARSON;

        // Section's title
        pSection->pPearson->strSectionTitle = editSectionTitle->text();

        // Check if copy (then update title)
        if (bCopy)
        {
            pSection->pPearson->strSectionTitle = "Copy of " +
                pSection->pPearson->strSectionTitle;
        }
        break;

    case GEX_CRPT_WIDGET_CORR_GB:
        // Create structure to hold
        // 'Tester-to-tester correlatioon & GB' settings
        pSection->pTesterCorrelationGB =
            new CGexCustomReport_TesterCorrelationGB_Section();

        // Saves Section type
        pSection->iSectionType = GEX_CRPT_WIDGET_CORR_GB;

        // Section's title
        pSection->pTesterCorrelationGB->strSectionTitle =
            editSectionTitle->text();

        // Check if copy (then update title)
        if (bCopy)
        {
            pSection->pTesterCorrelationGB->strSectionTitle = "Copy of " +
                pSection->pTesterCorrelationGB->strSectionTitle;
        }

        // Guard band file
        pSection->pTesterCorrelationGB->strGuardBandFile = editGBFile->text();
        break;

    case GEX_CRPT_WIDGET_PRODUCTION:
        // Create structure to hold 'Production' settings
        pSection->pProduction = new CGexCustomReport_Production_Section();

        // Saves Section type
        pSection->iSectionType = GEX_CRPT_WIDGET_PRODUCTION;

        // Section's title
        pSection->pProduction->strSectionTitle = editSectionTitle->text();

        // Check if copy (then update title)
        if (bCopy)
        {
            pSection->pProduction->strSectionTitle = "Copy of " +
                pSection->pProduction->strSectionTitle;
        }

        // Charting mode
        pSection->pProduction->iChartingType =
            comboBoxProduction->currentIndex();
        break;

    case GEX_CRPT_WIDGET_GLOBALINFO:
        // Create structure to hold 'Global Info' settings
        pSection->pGlobalInfo = new CGexCustomReport_GlobalInfo_Section();

        // Saves Section type
        pSection->iSectionType = GEX_CRPT_WIDGET_GLOBALINFO;

        // Section's title
        pSection->pGlobalInfo->strSectionTitle = editSectionTitle->text();

        // Check if copy (then update title)
        if (bCopy)
        {
            pSection->pGlobalInfo->strSectionTitle = "Copy of " +
                pSection->pGlobalInfo->strSectionTitle;
        }
        break;

    case GEX_CRPT_WIDGET_FILE_AUDIT:
    {
        // Create structure to hold 'File Audit' settings
        pSection->pFileAudit = new CustomReportFileAuditSection();

        // Saves Section type
        pSection->iSectionType = GEX_CRPT_WIDGET_FILE_AUDIT;

        // Section's title
        pSection->pFileAudit->strSectionTitle = editSectionTitle->text();

        // File audit selections
        pSection->pFileAudit->bCheckBadRecords =
            checkBoxAudit_BadRec->isChecked();
        pSection->pFileAudit->bCheckMissingRecords =
            checkBoxAudit_MissingRec->isChecked();
        pSection->pFileAudit->bCheckEndRecords =
            checkBoxAudit_EndRec->isChecked();
        pSection->pFileAudit->bCheckTestNames =
            checkBoxAudit_DuplicateTest->isChecked();
        pSection->pFileAudit->bCheckTestLimits =
            checkBoxAudit_LimitsIssue->isChecked();
        pSection->pFileAudit->bCheckMissingLimits =
            checkBoxAudit_MissingLimits->isChecked();

        // Dataset analysis
        pSection->pFileAudit->bCheckZeroRange =
            checkBoxAudit_ZeroRange->isChecked();
        pSection->pFileAudit->bCheckCategories =
            checkBoxAudit_Categories->isChecked();
        pSection->pFileAudit->bCheckMultiModal =
            checkBoxAudit_MultiModal->isChecked();
        pSection->pFileAudit->bCheckMeasurementIssue =
            checkBoxAudit_MeasurementIssue->isChecked();
        pSection->pFileAudit->bCheckRangeIssue =
            checkBoxAudit_RangeIssue->isChecked();
        pSection->pFileAudit->bCheckSiteCorrelation =
            checkBoxAudit_SiteToSite->isChecked();
        pSection->pFileAudit->bShowHistogram =
            checkBoxAudit_ShowHisto->isChecked();

        if (checkBoxAudit_SampleLimit->isChecked())
        {
            pSection->pFileAudit->ldSamplesLimit =
                lineEditAuditSampleLimit->text().toLong();
        }
        else
        {
            pSection->pFileAudit->ldSamplesLimit = -1;
        }

        if (checkBoxAudit_CpkLimit->isChecked())
        {
            pSection->pFileAudit->lfCpkLimit =
                lineEditAuditCpkLimit->text().toDouble();
        }
        else
        {
            pSection->pFileAudit->lfCpkLimit = -1;
        }

        // Test filters
        AuditParameterFilter* pFilter;
        QString strFilterType;
        QTreeWidgetItem* ptItem = listViewAuditFilters->itemAt(0, 0);
        while (ptItem)
        {
            pFilter = new AuditParameterFilter;

            pFilter->strProduct = ptItem->text(0);
            strFilterType = ptItem->text(1);
            if (strFilterType == "Ignore Test names...")
            {
                pFilter->iFilterType = GEX_AUDIT_FILTER_TNAME;
            }
            if (strFilterType == "Ignore test numbers...")
            {
                pFilter->iFilterType = GEX_AUDIT_FILTER_TNUMBER;
            }
            pFilter->strFilter = ptItem->text(2);

            // Add filter to list
            pSection->pFileAudit->cTestFilters.append(pFilter);

            // Next filter entry
            ptItem = listViewAuditFilters->itemBelow(ptItem);
        }

        // Other
        pSection->pFileAudit->bCheckProductID =
            checkBoxAudit_ProductID->isChecked();
        pSection->pFileAudit->bCheckLotID =
            checkBoxAudit_LotID->isChecked();
        pSection->pFileAudit->bCheckWaferID =
            checkBoxAudit_WaferID->isChecked();
        pSection->pFileAudit->bCheckDieLoc =
            checkBoxAudit_DieLoc->isChecked();
        pSection->pFileAudit->bLongReportMode =
            (comboBoxAuditReportType->currentIndex() == 1) ? true : false;


        // Check if copy (then update title)
        if (bCopy)
        {
            pSection->pFileAudit->strSectionTitle = "Copy of " +
                pSection->pFileAudit->strSectionTitle;
        }
    }
    break;

    case GEX_CRPT_WIDGET_DATALOG:
        // Create structure to hold 'Wafer map' settings
        pSection->pDatalog = new CGexCustomReport_Datalog_Section();

        // Saves Section type
        pSection->iSectionType = GEX_CRPT_WIDGET_DATALOG;

        // Section's title
        pSection->pDatalog->strSectionTitle = editSectionTitle->text();

        // Check if copy (then update title)
        if (bCopy)
        {
            pSection->pDatalog->strSectionTitle = "Copy of " +
                pSection->pDatalog->strSectionTitle;
        }

        // Type of tests (fail, all, test list...)
        pSection->pDatalog->iDatalogType = comboBoxTestTypeDatalog->itemData(
                comboBoxTestTypeDatalog->currentIndex()).toInt();

        // Test list
        pSection->pDatalog->strTestList = editTestListDatalog->text();

        switch (pSection->pDatalog->iDatalogType)
        {
        default:
            pSection->pDatalog->iDatalogType = GEX_ADV_DATALOG_ALL;
            break;
        case GEX_ADV_DATALOG_ALL:
        case GEX_ADV_DATALOG_FAIL:
            break;
        case GEX_ADV_DATALOG_LIST:
        case GEX_ADV_DATALOG_RAWDATA:
            if (pSection->pDatalog->strTestList.isEmpty())
            {
                pSection->pDatalog->strTestList = "1000";
            }
            break;
        }
        break;
    }
}

/******************************************************************************!
 * \fn OnSectionAggregate
 * \brief Section type to create: Test Aggregates
 ******************************************************************************/
void ReportTemplateGui_EditSection::OnSectionAggregate()
{
    // Show Title field
    TextLabelTitle->show();
    editSectionTitle->show();

    int iComboPosition = comboBoxTestList->currentIndex();
    switch (iComboPosition)
    {
    case GEX_CRPT_TEST_TYPE_ALL:  // All tests
    case GEX_CRPT_TEST_TYPE_FAIL:  // Failing tests
    case GEX_CRPT_TEST_TYPE_OUTLIERS:  // Tests with outliers
        TextLabelList->hide();
        editTestStats->hide();
        PickTestStats->hide();
        break;
    case GEX_CRPT_TEST_TYPE_LIST:  // List of tests
        TextLabelList->setText("Test list:");
        TextLabelList->show();
        // Allow Test pickup from list, then fall in other cases hereafter
        PickTestStats->show();
        editTestStats->show();
        editTestStats->setFocus();
        break;
    case GEX_CRPT_TEST_TYPE_TOP_N_FAIL:  // Top N failing tests
        TextLabelList->setText("N:");
        TextLabelList->show();
        PickTestStats->hide();
        editTestStats->show();
        editTestStats->setFocus();
        break;
    case GEX_CRPT_TEST_TYPE_BADCP:
        // Cp limit (report all tests with Cp lower or equal)
        TextLabelList->setText("Cp limit:");
        TextLabelList->show();
        PickTestStats->hide();
        editTestStats->show();
        editTestStats->setFocus();
        break;
    case GEX_CRPT_TEST_TYPE_BADCPK:
        // Cpk limit (report all tests with Cpk lower or equal)
        TextLabelList->setText("Cpk limit:");
        TextLabelList->show();
        PickTestStats->hide();
        editTestStats->show();
        editTestStats->setFocus();
        break;
    }

    // Histogram type
    if (checkBoxHistogram->isChecked())
    {
        comboHistogram->show();
    }
    else
    {
        comboHistogram->hide();
    }

    // Trend chart type
    if (checkBoxTrend->isChecked())
    {
        comboTrend->show();
    }
    else
    {
        comboTrend->hide();
    }

    // Scatter plot type
    if (checkBoxScatter->isChecked())
    {
        comboScatter->show();
        labelTest2->show();
        editTestY->show();
        PickTestScatterY->show();
    }
    else
    {
        comboScatter->hide();
        labelTest2->hide();
        editTestY->hide();
        PickTestScatterY->hide();
    }

    // Probability plot type
    if (checkBoxProbability->isChecked())
    {
        comboProbabilityPlot->show();
    }
    else
    {
        comboProbabilityPlot->hide();
    }

    // Box plot type
    if (checkBoxBoxplotEx->isChecked())
    {
        comboBoxplotEx->show();
    }
    else
    {
        comboBoxplotEx->hide();
    }

    // MultiChart type
    if (checkBoxMultiChart->isChecked())
    {
        comboBoxMultiChart->show();
    }
    else
    {
        comboBoxMultiChart->hide();
    }

    // BoxPlot: Disable support of the Whisker mode for now
    if (checkBoxBoxPlot->isChecked())
    {
        //comboBoxWhiskerType->show();
    }
    else
    {
        //comboBoxWhiskerType->hide();
    }
}

/******************************************************************************!
 * \fn OnSectionWafmap
 * \brief Section type to create: Wafermap
 ******************************************************************************/
void ReportTemplateGui_EditSection::OnSectionWafmap()
{
    // Show Title field
    TextLabelTitle->show();
    editSectionTitle->show();

    // Get item selected (add +1 because the first item of this list is
    // valid unlike the 'define' that start with a 'disabled' item)
    int iComboPosition = comboBoxWafmap->currentIndex() + 1;
    switch (iComboPosition)
    {
    default:
    case GEX_WAFMAP_STACK_SOFTBIN:
    case GEX_WAFMAP_SOFTBIN:  // Standard Wafer map: Software Binning
    case GEX_WAFMAP_ZONAL_SOFTBIN:
    case GEX_WAFMAP_HARDBIN:  // Standard Wafer map: Hardware Binning
    case GEX_WAFMAP_STACK_HARDBIN:
    case GEX_WAFMAP_ZONAL_HARDBIN:
        PickTestWafermap->hide();  // Hide Test# picker
        break;
    case GEX_WAFMAP_TESTOVERLIMITS:  // zoning on test limits
    case GEX_WAFMAP_STACK_TESTOVERLIMITS:
    case GEX_WAFMAP_TESTOVERDATA:  // zoning on test values range
    case GEX_WAFMAP_STACK_TESTOVERDATA:
    case GEX_WAFMAP_TEST_PASSFAIL:  // zoning on test values range
    case GEX_WAFMAP_STACK_TEST_PASSFAIL:
        PickTestWafermap->show();  // Show Test# picker
        break;
    }
}

/******************************************************************************!
 * \fn OnPickTestWafermap
 * \brief Section: Wafermap
 *        Display Test list and let use pick some
 ******************************************************************************/
void ReportTemplateGui_EditSection::OnPickTestWafermap()
{
    // Show TestList
    PickTestDialog dPickTest;

    // Allow Multiple selections.
    dPickTest.setMultipleSelection(true);

    // Check if List was successfuly loaded
    // Then prompt dialog box, let user pick tests from the list
    if (dPickTest.fillParameterList() && dPickTest.exec() == QDialog::Accepted)
    {
        // Save the list selected into the edit field
        editWafmap->setText(dPickTest.testList());
    }
}

/******************************************************************************!
 * \fn OnOk
 * \brief ReportTemplateGui_EditSection: 'Ok' button clicked
 ******************************************************************************/
void ReportTemplateGui_EditSection::OnOk()
{
    done(1);
}

/******************************************************************************!
 * \fn OnNewSection
 * \brief ReportTemplateGui:
 ******************************************************************************/
void ReportTemplateGui::OnNewSection()
{
    ReportTemplateGui_EditSection cNewSection(this);

    if (cNewSection.exec() != 1)
    {
        return;  // User cancelled task
    }
    // Retrieve section edits
    ReportTemplateSection* pNewSection = new ReportTemplateSection;
    cNewSection.getSectionDetails(pNewSection);

    // If tryinng to create a Wafermap section while we already have one,
    // then reject such new section! Only one wafermap section allowed right now
    if (mReportTemplate->GetWafmapSectionDetails() != NULL &&
        pNewSection->pWafmap != NULL)
    {
        GS::Gex::Message::information("",
                                      "Only one Wafer map section allowed in "
                                      "report!\nEdit existing one if needed.");

        // Delete wafmap section just created
        delete pNewSection->pWafmap;
        delete pNewSection;
        return;
    }

    // Insert this new section after the current selection,
    // or at the end if no selection made
    int iEntryIndex = 0;
    QTreeWidgetItem* ptNewEntry = NULL;
    QTreeWidgetItem* ptItem     = ListView->itemAt(0,0);

    while (ptItem != NULL)
    {
        if (ptItem == ListView->currentItem())
        {
            // Insert entry just after current selection (GUI)
            QStringList labels;
            labels.append(pNewSection->getSectionTitle());
            labels.append(pNewSection->getTestList());
            labels.append(pNewSection->getCharts());
            ptNewEntry = new QTreeWidgetItem(ListView,/*ptItem,*/
                                             labels);

            // Increment index so insertion of structure in
            // our internal list is done AFTER selection
            iEntryIndex++;

            // Exit from loop
            break;
        }

        // Move to next node
        ptItem = ListView->itemBelow( ptItem);

        // Update Item#
        iEntryIndex++;
    }

    // Check if inseryion already done
    if (ptItem == NULL)
    {
        // insert item to GUI
        QStringList labels;
        labels.append(pNewSection->getSectionTitle());
        labels.append(pNewSection->getTestList());
        labels.append(pNewSection->getCharts());
        ptNewEntry = new QTreeWidgetItem(ListView,
                                         labels);
    }

    // Save new section structure (at same index as in GUI)
    // into our internal list
    mReportTemplate->Insert(iEntryIndex, pNewSection);

    // Make new item inserted in list the new selection
    ListView->setCurrentItem(ptNewEntry);

    // Flag to tell if template requires to be saved
    OnTemplateModified();

    // Show/Hide Admin icons
    UpdateGUI();
}


/******************************************************************************!
 * \fn OnAutoFill
 * \brief ReportTemplateGui: Auto fill report with typical sections
 ******************************************************************************/
void ReportTemplateGui::OnAutoFill()
{
    // Empty GUI
    ListView->clear();

    // Delete ALL tructures from memory
    mReportTemplate->Clear();

    // Build all default sections
    int iSectionID = 0;
    ReportTemplateSection*               pNewSection;
    CustomReportTestAggregateSection*    pAggregate;
    CGexCustomReport_WafMap_Section*     pWafmap;
    CGexCustomReport_Binning_Section*    pBinning;
    CGexCustomReport_Pareto_Section*     pPareto;
    CGexCustomReport_GlobalInfo_Section* pGlobalInfo;

    // Section 1: All Statistics
    pNewSection = new ReportTemplateSection;
    pNewSection->iSectionType = GEX_CRPT_WIDGET_AGGREGATE;
    pNewSection->pAggregate   = pAggregate =
            new CustomReportTestAggregateSection;
    pAggregate->strSectionTitle = "Test Statistics";
    pAggregate->iTestType = GEX_CRPT_TEST_TYPE_ALL;
    pAggregate->bStats    = true;
    // Insert section
    mReportTemplate->Insert(iSectionID, pNewSection);
    iSectionID++;

    // Section 2: All Histograms
    pNewSection = new ReportTemplateSection;
    pNewSection->iSectionType = GEX_CRPT_WIDGET_AGGREGATE;
    pNewSection->pAggregate   = pAggregate =
            new CustomReportTestAggregateSection;
    pAggregate->strSectionTitle = "Histogram of Tests";
    // One test per chart (all layers plotted)
    pAggregate->iChartMode      = GEX_CRPT_CHART_MODE_SINGLE_TEST;
    pAggregate->iTestType       = GEX_CRPT_TEST_TYPE_ALL;
    // Decrement value as MyReport doesn't include a first dummy line
    // in the selection drop-down box
    pAggregate->iHistogramType  = GEX_ADV_HISTOGRAM_OVERDATA - 1;
    // Insert section
    mReportTemplate->Insert(iSectionID, pNewSection);
    iSectionID++;

    // Section 3: Wafermap
    pNewSection = new ReportTemplateSection;
    pNewSection->iSectionType = GEX_CRPT_WIDGET_WAFMAP;
    pNewSection->pWafmap      = pWafmap = new CGexCustomReport_WafMap_Section;
    pWafmap->strSectionTitle  = "Wafer maps";
    // Decrement value as MyReport doesn't include a first dummy line
    // in the selection drop-down box
    pWafmap->iWafermapType    = GEX_WAFMAP_SOFTBIN - 1;
    // Insert section
    mReportTemplate->Insert(iSectionID, pNewSection);
    iSectionID++;

    // Section 4: Binning
    pNewSection = new ReportTemplateSection;
    pNewSection->iSectionType = GEX_CRPT_WIDGET_BINNING;
    pNewSection->pBinning     = pBinning =
        new CGexCustomReport_Binning_Section;
    pBinning->strSectionTitle = "Binning";
    // Insert section
    mReportTemplate->Insert(iSectionID, pNewSection);
    iSectionID++;

    // Section 5: Pareto
    pNewSection = new ReportTemplateSection;
    pNewSection->iSectionType = GEX_CRPT_WIDGET_PARETO;
    pNewSection->pPareto      = pPareto = new CGexCustomReport_Pareto_Section;
    pPareto->strSectionTitle  = "Pareto";
    // Insert section
    mReportTemplate->Insert(iSectionID, pNewSection);
    iSectionID++;

    // Section 6: Global Info
    pNewSection = new ReportTemplateSection;
    pNewSection->iSectionType = GEX_CRPT_WIDGET_GLOBALINFO;
    pNewSection->pGlobalInfo  = pGlobalInfo =
            new CGexCustomReport_GlobalInfo_Section;
    pGlobalInfo->strSectionTitle = "Global Information";
    // Insert section
    mReportTemplate->Insert(iSectionID, pNewSection);
    iSectionID++;

    // Flag to tell if template requires to be saved
    OnTemplateModified();

    // Refresh GUI to rebuild/update the list of sections
    UpdateGuiSectionsList();

    // Show/Hide Admin icons
    UpdateGUI();
}

/******************************************************************************!
 * \fn OnEditSection
 * \brief ReportTemplateGui: Edit section settings
 ******************************************************************************/
void ReportTemplateGui::OnEditSection()
{
    // Get handle to the entry to edit
    int iEntryIndex = 0;
    QTreeWidgetItem* ptItem = ListView->itemAt(0,0);

    while (ptItem != NULL)
    {
        // Exit from loop when selection is found
        if (ptItem == ListView->currentItem())
        {
            break;
        }

        // Move to next node
        ptItem = ListView->itemBelow(ptItem);

        // Update Item#
        iEntryIndex++;
    }

    // If no selection, quietly return
    if (ptItem == NULL)
    {
        return;
    }

    // Get handle to data to edit
    ReportTemplateSection* pEditSection = mReportTemplate->At(iEntryIndex);

    // Preload Edit box with relevant fields
    ReportTemplateGui_EditSection cNewSection(this);
    cNewSection.fillFields(pEditSection);

    // Display edit dialog box
    if (cNewSection.exec() != 1)
    {
        return;  // User cancelled task

    }
    // Rebuild List View (with updated selections, titles, etc.)
    ReportTemplateSection* pNewSection = new ReportTemplateSection;
    cNewSection.getSectionDetails(pNewSection);
    mReportTemplate->Replace(iEntryIndex, pNewSection);
    UpdateGuiSectionsList();

    // Make focus back to Item index edited
    ptItem = ListView->itemAt(0,0);
    while (ptItem != NULL && iEntryIndex >= 0)
    {
        // Move to next node
        ptItem = ListView->itemBelow(ptItem);

        // Update Item#
        iEntryIndex--;
    }

    // We've reached the node# just edited: set focus on it
    if (ptItem != NULL)
    {
        ListView->setCurrentItem(ptItem);
    }

    // Flag to tell if template requires to be saved
    OnTemplateModified();
}

/******************************************************************************!
 * \fn fillFields
 * \brief Prefill fields for editing existing section
 ******************************************************************************/
void ReportTemplateGui_EditSection::fillFields(
    ReportTemplateSection* pSection)
{
    if (pSection == NULL)
    {
        return;
    }

    // Reflect section type (Aggregate, wafermap, etc.)
    // but grey selection so it can't be chanegd by user
    comboBoxSectionType->setCurrentIndex(pSection->iSectionType);
    comboBoxSectionType->setEnabled(false);

    // Make sure the relevant Stacked widget is made visible
    // (aggregate, Wafer map, etc.)
    OnSectionType();

    // Update GUI fields
    CustomReportTestAggregateSection* pAggregate = pSection->pAggregate;
    CGexCustomReport_WafMap_Section*  pWafmap  = pSection->pWafmap;
    CGexCustomReport_Binning_Section* pBinning = pSection->pBinning;
    CGexCustomReport_Pareto_Section*  pPareto  = pSection->pPareto;
    CGexCustomReport_Pearson_Section* pPearson = pSection->pPearson;
    CGexCustomReport_TesterCorrelationGB_Section* pTesterCorrelationGB =
        pSection->pTesterCorrelationGB;
    CGexCustomReport_Production_Section* pProduction =
        pSection->pProduction;
    CGexCustomReport_GlobalInfo_Section* pGlobalInfo =
        pSection->pGlobalInfo;
    CustomReportFileAuditSection* pFileAudit =
        pSection->pFileAudit;
    CGexCustomReport_Datalog_Section* pDatalog =
        pSection->pDatalog;

    int     iIndex;
    QString strString;
    switch (pSection->iSectionType)
    {
    case GEX_CRPT_WIDGET_AGGREGATE:
        // Section's title
        editSectionTitle->setText(pAggregate->strSectionTitle);

        // Type of tests (fail, all, test list, ...)
        comboBoxTestList->setCurrentIndex(pAggregate->iTestType);

        // Test list
        editTestStats->setText(pAggregate->strRangeList);

        // Include Test Statistics table?
        checkBoxStats->setChecked(pAggregate->bStats);

        // Charting mode: one overmay chart, one chart per test, both ?
        comboChartingType->setCurrentIndex(pAggregate->iChartMode);

        // Include Histogram ?
        if (pAggregate->iHistogramType < 0)
        {
            checkBoxHistogram->setChecked(false);
        }
        else
        {
            checkBoxHistogram->setChecked(true);
            comboHistogram->setCurrentIndex(pAggregate->iHistogramType);
        }

        // Include Trend ?
        if (pAggregate->iTrendType < 0)
        {
            checkBoxTrend->setChecked(false);
        }
        else
        {
            checkBoxTrend->setChecked(true);
            comboTrend->setCurrentIndex(pAggregate->iTrendType);
        }

        // Include Scatter ?
        if (pAggregate->iScatterType < 0)
        {
            checkBoxScatter->setChecked(false);
        }
        else
        {
            checkBoxScatter->setChecked(true);
            comboScatter->setCurrentIndex(pAggregate->iScatterType);
            editTestY->setText(pAggregate->strRangeListY);
        }

        // Include Probability plot ?
        if (pAggregate->iProbabilityPlotType < 0)
        {
            checkBoxProbability->setChecked(false);
        }
        else
        {
            checkBoxProbability->setChecked(true);
            comboProbabilityPlot->
                setCurrentIndex(pAggregate->iProbabilityPlotType);
        }

        // Include Box plot extended ?
        if (pAggregate->iBoxplotExType < 0)
        {
            checkBoxBoxplotEx->setChecked(false);
        }
        else
        {
            checkBoxBoxplotEx->setChecked(true);
            comboBoxplotEx->setCurrentIndex(pAggregate->iBoxplotExType);
        }

        // Include MultiChart plot ?
        if (pAggregate->iMultiChartsType < 0)
        {
            checkBoxMultiChart->setChecked(false);
        }
        else
        {
            checkBoxMultiChart->setChecked(true);
            comboBoxMultiChart->setCurrentIndex(pAggregate->iMultiChartsType);
        }

        // Include BoxPlot?
        if (pAggregate->iBoxPlotType < 0)
        {
            checkBoxBoxPlot->setChecked(false);
        }
        else
        {
            checkBoxBoxPlot->setChecked(true);
            comboBoxWhiskerType->setCurrentIndex(pAggregate->iBoxPlotType);
        }

        // Viewport & Legends
        groupBoxChartTitle->setChecked(pAggregate->bChartTitle);
        lineEditChartTitle->setText(pAggregate->strChartTitle);

        groupBoxChartLegendX->setChecked(pAggregate->bLegendX);
        lineEditAxisLegendX->setText(pAggregate->strAxisLegendX);

        groupBoxChartLegendY->setChecked(pAggregate->bLegendY);
        lineEditAxisLegendY->setText(pAggregate->strAxisLegendY);

        // LowX/Y, High X/Y
        if (pAggregate->lfLowX != -C_INFINITE)
        {
            lineEditLowX->setText(QString::number(pAggregate->lfLowX));
        }
        if (pAggregate->lfLowY != -C_INFINITE)
        {
            lineEditLowY->setText(QString::number(pAggregate->lfLowY));
        }
        if (pAggregate->lfHighX != C_INFINITE)
        {
            lineEditHighX->setText(QString::number(pAggregate->lfHighX));
        }
        if (pAggregate->lfHighY != C_INFINITE)
        {
            lineEditHighY->setText(QString::number(pAggregate->lfHighY));
        }

        // Logscale X-axis
        if (pAggregate->bLogScaleX)
        {
            comboBoxScaleX->setCurrentIndex(1);
        }
        // Logscale Y-axis
        if (pAggregate->bLogScaleY)
        {
            comboBoxScaleY->setCurrentIndex(1);
        }

        cColor = pAggregate->cColor;

        // Plotting style
        // Pen width
        spinBoxLineWidth->setValue(pAggregate->iLineWidth);
        // Line Style: dashed, plain, etc.
        comboBoxLineStyle->setCurrentIndex(pAggregate->iLineStyle);
        // Spot style: circle, triangle, etc.
        comboBoxLineSpotStyle->setCurrentIndex(pAggregate->iSpotStyle);
        // true if draw with BARS (histogram charting only)
        checkBoxBars->setChecked(pAggregate->bBoxBars);
        // true if draw fitting curve / spin
        checkBoxfittingCurve->setChecked(pAggregate->bFittingCurve);
        // true if draw Guaussian Bell-curve shape
        checkBoxBellCurve->setChecked(pAggregate->bBellCurve);
        // true if connect points with a line
        checkBoxLines->setChecked(pAggregate->bLines);
        // true if draw a spot at each data point
        checkBoxspots->setChecked(pAggregate->bSpots);
        // Array of Marker colors & width
        for (iIndex = 0; iIndex < GEX_CRPT_MARKER_TOTALSIZE; iIndex++)
        {
            icMarkerWidth[iIndex] = pAggregate->icMarkerWidth[iIndex];
            cMarkerColor[iIndex]  = pAggregate->cMarkerColor[iIndex];
        }

        // Refresh GUI selections
        OnSectionAggregate();
        break;

    case GEX_CRPT_WIDGET_WAFMAP:
        // Section's title
        editSectionTitle->setText(pWafmap->strSectionTitle);

        // Type of tests (fail, all, test list, ...)
        comboBoxWafmap->setCurrentIndex(pWafmap->iWafermapType);

        // Test list
        editWafmap->setText(pWafmap->strRangeList);

        // Refresh GUI selections
        OnSectionWafmap();
        break;

    case GEX_CRPT_WIDGET_BINNING:
        // Section's title
        editSectionTitle->setText(pBinning->strSectionTitle);
        break;

    case GEX_CRPT_WIDGET_PARETO:
        // Section's title
        editSectionTitle->setText(pPareto->strSectionTitle);
        break;

    case GEX_CRPT_WIDGET_PEARSON:
        // Section's title
        editSectionTitle->setText(pPearson->strSectionTitle);
        break;

    case GEX_CRPT_WIDGET_CORR_GB:
        // Section's title
        editSectionTitle->setText(pTesterCorrelationGB->strSectionTitle);
        editGBFile->setText(pTesterCorrelationGB->strGuardBandFile);
        break;

    case GEX_CRPT_WIDGET_GLOBALINFO:
        // Section's title
        editSectionTitle->setText(pGlobalInfo->strSectionTitle);
        break;

    case GEX_CRPT_WIDGET_FILE_AUDIT:
    {
        // Section's title
        editSectionTitle->setText(pFileAudit->strSectionTitle);

        // Section's title
        pSection->pFileAudit->strSectionTitle = editSectionTitle->text();

        // File audit selections
        checkBoxAudit_BadRec->setChecked(pFileAudit->bCheckBadRecords);
        checkBoxAudit_MissingRec->setChecked(pFileAudit->bCheckMissingRecords);
        checkBoxAudit_EndRec->setChecked(pFileAudit->bCheckEndRecords);
        checkBoxAudit_DuplicateTest->setChecked(pFileAudit->bCheckTestNames);
        checkBoxAudit_LimitsIssue->setChecked(pFileAudit->bCheckTestLimits);
        checkBoxAudit_MissingLimits->
            setChecked(pFileAudit->bCheckMissingLimits);

        // Dataset analysis
        checkBoxAudit_ZeroRange->setChecked(pFileAudit->bCheckZeroRange);
        checkBoxAudit_Categories->setChecked(pFileAudit->bCheckCategories);
        checkBoxAudit_MultiModal->setChecked(pFileAudit->bCheckMultiModal);
        checkBoxAudit_MeasurementIssue->setChecked(
            pFileAudit->bCheckMeasurementIssue);
        checkBoxAudit_RangeIssue->setChecked(pFileAudit->bCheckRangeIssue);
        ///////// DEBUG : HIDE Widget for now
        checkBoxAudit_RangeIssue->hide();
        /////////
        checkBoxAudit_SiteToSite->setChecked(pFileAudit->bCheckSiteCorrelation);
        checkBoxAudit_ShowHisto->setChecked(pFileAudit->bShowHistogram);
        if (pFileAudit->ldSamplesLimit >= 0)
        {
            checkBoxAudit_SampleLimit->setChecked(true);
            lineEditAuditSampleLimit->setText(QString::number(pFileAudit->
                                                              ldSamplesLimit));
        }
        else
        {
            checkBoxAudit_SampleLimit->setChecked(false);
            lineEditAuditSampleLimit->setText("30");
        }
        if (pFileAudit->lfCpkLimit >= 0)
        {
            checkBoxAudit_CpkLimit->setChecked(true);
            lineEditAuditCpkLimit->setText(QString::number(pFileAudit->
                                                           lfCpkLimit));
        }
        else
        {
            checkBoxAudit_CpkLimit->setChecked(false);
            lineEditAuditCpkLimit->setText("20");
        }

        // Test filters
        listViewAuditFilters->clear();

        QList<AuditParameterFilter*>::iterator itBegin =
            pFileAudit->cTestFilters.begin();
        QList<AuditParameterFilter*>::iterator itEnd =
            pFileAudit->cTestFilters.end();

        while (itBegin != itEnd)
        {
            switch ((*itBegin)->iFilterType)
            {
            case GEX_AUDIT_FILTER_TNAME:
                strString = "Ignore Test names...";
                break;
            case GEX_AUDIT_FILTER_TNUMBER:
                strString = "Ignore test numbers...";
                break;
            }

            // insert section in list view
            QStringList labels;
            labels.append((*itBegin)->strProduct);
            labels.append(strString);
            labels.append((*itBegin)->strFilter);
            new QTreeWidgetItem(listViewAuditFilters,
                                labels);
        }

        // Other options
        checkBoxAudit_ProductID->setChecked(pFileAudit->bCheckProductID);
        checkBoxAudit_LotID->setChecked(pFileAudit->bCheckLotID);
        checkBoxAudit_WaferID->setChecked(pFileAudit->bCheckWaferID);
        checkBoxAudit_DieLoc->setChecked(pFileAudit->bCheckDieLoc);
        if (pFileAudit->bLongReportMode)
        {
            comboBoxAuditReportType->setCurrentIndex(1);
        }
        else
        {
            comboBoxAuditReportType->setCurrentIndex(0);
        }

        // Refresh GUI selections
        OnSectionDataAudit();
        break;
    }

    case GEX_CRPT_WIDGET_PRODUCTION:
        // Section's title
        editSectionTitle->setText(pProduction->strSectionTitle);
        break;

    case GEX_CRPT_WIDGET_DATALOG:
        // Section's title
        editSectionTitle->setText(pDatalog->strSectionTitle);

        // Type of tests (fail, all, test list, ...)
        comboBoxTestTypeDatalog->setCurrentIndex(
            comboBoxTestTypeDatalog->
            findData(QVariant(pDatalog->iDatalogType)));

        // Test list
        editTestListDatalog->setText(pDatalog->strTestList);

        // Refresh GUI selections
        onTestTypeDatalog();
        break;

    default:
        break;
    }
}

/******************************************************************************!
 * \fn OnDuplicateSection
 * \brief ReportTemplateGui: Duplicate section
 ******************************************************************************/
void ReportTemplateGui::OnDuplicateSection()
{
    int iEntryIndex = 0;
    ReportTemplateSection* pSelectedSection;
    ReportTemplateSection* pDuplicate;
    QTreeWidgetItem* ptItem = ListView->itemAt(0,0);

    while (ptItem != NULL)
    {
        if (ptItem == ListView->currentItem())
        {
            // Get handle to structure associated with item clicked on GUI
            pSelectedSection = mReportTemplate->At(iEntryIndex);

            // Unexpected error
            if (pSelectedSection == NULL)
            {
                return;
            }

            // Preload Edit box with relevant fields +
            // update name as this is a 'copy' of an existing entry
            ReportTemplateGui_EditSection cNewSection(this);
            cNewSection.fillFields(pSelectedSection);

            // Rebuild List View (with updated selections, titles, etc.)
            pDuplicate = new ReportTemplateSection;
            cNewSection.getSectionDetails(pDuplicate, true);

            mReportTemplate->Insert(iEntryIndex + 1, pDuplicate);
            UpdateGuiSectionsList();

            // Flag to tell if template requires to be saved
            OnTemplateModified();

            // Show/Hide Admin icons
            UpdateGUI();

            // Make focus back to Item index created
            ptItem = ListView->itemAt(0,0);
            while (ptItem != NULL && iEntryIndex >= 0)
            {
                // Move to next node
                ptItem = ListView->itemBelow(ptItem);

                // Update Item#
                iEntryIndex--;
            }

            // We've reached the node# just edited: set focus on it
            if (ptItem != NULL)
            {
                ListView->setCurrentItem(ptItem);
            }

            // Only accept one duplicate at a time
            return;
        }

        // Move to next node
        ptItem = ListView->itemBelow(ptItem);

        // Update Item#
        iEntryIndex++;
    }
}

/******************************************************************************!
 * \fn OnDeleteSection
 * \brief ReportTemplateGui:
 ******************************************************************************/
void ReportTemplateGui::OnDeleteSection()
{
    int iEntryIndex = 0;
    ReportTemplateSection* pSelectedSection;
    QTreeWidgetItem* ptItem = ListView->currentItem();
    if(!ptItem)
        return ;

    while (ptItem != NULL)
    {
        if (ptItem == ListView->currentItem())
        {
            // Get handle to structure associated with item clicked on GUI
            pSelectedSection = mReportTemplate->At(iEntryIndex);

            // Unexpected error...
            if (pSelectedSection == NULL)
            {
                return;
            }

            // Ask 'Delete' confirmation
            QString strMessage = "Confirm to Delete section: '" +
                pSelectedSection->getSectionTitle() + "' ?";
            bool lOk;
            GS::Gex::Message::request("Delete Section", strMessage, lOk);
            if (lOk)
            {
                // Delete GUI selection
                delete ptItem;

                // Delete structure from memory
                delete mReportTemplate->TakeAt(iEntryIndex);

                // Flag to tell if template requires to be saved
                OnTemplateModified();

                // Show/Hide Admin icons
                UpdateGUI();

                // Only accept one delete at a time
                return;
            }
        }

        // Move to next node
        ptItem = ListView->itemBelow(ptItem);

        // Update Item#
        iEntryIndex++;
    }
}

/******************************************************************************!
 * \fn OnDeleteAllSections
 ******************************************************************************/
void ReportTemplateGui::OnDeleteAllSections()
{
    // Empty list, do nothing
    if (mReportTemplate->Begin() ==
        mReportTemplate->End())
    {
        return;
    }

    // Ask 'Delete' confirmation
    QString     strMessage = "Confirm to Delete All sections?";
    bool lOk;
    GS::Gex::Message::request("Delete Section", strMessage, lOk);
    if (! lOk)
    {
        return;
    }

    // Empty GUI
    ListView->clear();

    // Delete ALL tructures from memory
    mReportTemplate->Clear();

    // Flag to tell if template requires to be saved
    OnTemplateModified();

    // Show/Hide Admin icons
    UpdateGUI();
}

/******************************************************************************!
 * \fn OnMoveSectionUp
 ******************************************************************************/
void ReportTemplateGui::OnMoveSectionUp()
{
    // If no item in list, just return
    if (ListView->topLevelItemCount() <= 1)
    {
        return;
    }

    // Scan the list to move UP the selected item
    QTreeWidgetItem* ptItem     = ListView->itemAt(0,0);

    int iIndex = 0;
    while (ptItem != NULL)
    {
        // Find first object selected
        if (ptItem->isSelected() == true)
        {
            // if Item selected is first in list, can't move up, ignore
            if (iIndex != 0)
            {
                // Move Item above selection
                ListView->takeTopLevelItem(iIndex);
                ListView->insertTopLevelItem(iIndex - 1, ptItem);
                ListView->setCurrentItem(ptItem);

                // Reflect same order in our internal Section's list
                mReportTemplate->Insert(iIndex - 1,
                                        mReportTemplate->TakeAt(iIndex));

                // Flag to tell if template requires to be saved
                OnTemplateModified();
            }
            return;
        }
        else
        {
            // Move to next item
            ptItem = ListView->itemBelow(ptItem);
        }

        // Keep track of item#
        iIndex++;
    }

    // Flag to tell if template requires to be saved
    OnTemplateModified();
}

/******************************************************************************!
 * \fn OnMoveSectionDown
 ******************************************************************************/
void ReportTemplateGui::OnMoveSectionDown()
{
    // If no item in list, just return!
    if (ListView->topLevelItemCount() <= 1)
    {
        return;
    }

    // Scan the list to move DOWN the selected item
    QTreeWidgetItem* ptItem;
    ptItem = ListView->itemAt(0,0);
    int iIndex = 0;
    while (ptItem != NULL)
    {
        // Find first object selected
        if (ptItem->isSelected() == true)
        {
            // If last item selected, do noting: item already last in list
            if (iIndex < ListView->topLevelItemCount() - 1)
            {
                // Move selected item after following item in the list
                ListView->takeTopLevelItem(iIndex);
                ListView->insertTopLevelItem(iIndex +1, ptItem);
                ListView->setCurrentItem(ptItem);

                // Reflect same order in our internal Section's list
                mReportTemplate->Insert(iIndex + 1,
                                        mReportTemplate->TakeAt(iIndex));

                // Flag to tell if template requires to be saved
                OnTemplateModified();
            }
            return;
        }
        else
        {
            // Move to next item
            ptItem = ListView->itemBelow(ptItem);
        }
        // Keep track of item#
        iIndex++;
    }

    // Flag to tell if template requires to be saved
    OnTemplateModified();
}

/******************************************************************************!
 * \fn ReportTemplateGui_EditSection
 * \brief Report section editor
 ******************************************************************************/
ReportTemplateGui_EditSection::ReportTemplateGui_EditSection(
    QWidget*   parent,
    bool       modal,
    Qt::WindowFlags fl)
    : QDialog(parent, fl)
{
    setupUi(this);
    setModal(modal);

    // Apply Examinator palette
    GexMainwindow::applyPalette(this);

    QObject::connect(PushButtonCancel, SIGNAL(clicked()), SLOT(reject()));

    // Disable some GUI fields for now
    comboBoxWhiskerType->hide();
    // For now, hide the 'Style' tab
    tabWidgetAggregate->removeTab(3);

    // Fill Report types available
    comboBoxSectionType->clear();
    comboBoxSectionType->addItem(QIcon(*pixTasks),
                                    "Select section type to create...");
    comboBoxSectionType->addItem(
        *pixGexApplication,
        "Test charts aggregate: statistics, histogram, trend chart, etc.");
    comboBoxSectionType->addItem(QIcon(*pixWafermap), "Wafer map");
    comboBoxSectionType->addItem(QIcon(*pixBinning), "Binning Summary");
    comboBoxSectionType->addItem(QIcon(*pixPareto), "Pareto Reports");
    comboBoxSectionType->addItem(
        QIcon(*pixStopwatch),
        "Pearson's correlation: Test flow & Test time reduction");
    comboBoxSectionType->addItem(QIcon(*pixGuardBanding),
                                    "Tester-to-Tester Correlation & Guardband");
    comboBoxSectionType->addItem(QIcon(*pixTrend), "Production reports");
    comboBoxSectionType->addItem(
        QIcon(*pixTestStatistics),
        "Data Audit (STDF Sanity check, PAT compliancy, etc)");
    comboBoxSectionType->addItem(QIcon(*pixDatalog), "Global Information");
    comboBoxSectionType->addItem(QIcon(*pixDatalog), "Datalog");

    // Fill 'test Statistics'
    pGexMainWindow->pWizardSettings->
        FillListBox_TestStatistics(comboBoxTestList, false);

    // Fill 'Histogram' list
    pGexMainWindow->pWizardSettings->
        FillListBox_Histograms(comboHistogram, false);

    // Fill 'Trend chart' list
    comboTrend->clear();
    comboTrend->addItem(QIcon(*pixTrend), "Chart over test limits");
    comboTrend->addItem(QIcon(*pixTrend), "Chart over test results");
    comboTrend->addItem(QIcon(*pixTrend), "Adaptive: data & limits");

    // Fill 'Scatter' list
    comboScatter->clear();
    comboScatter->addItem(QIcon(*pixScatter), "Chart over test limits");
    comboScatter->addItem(QIcon(*pixScatter), "Chart over test results");

    // Fill 'Probability plot' list
    comboProbabilityPlot->clear();
    comboProbabilityPlot->addItem(QIcon(*pixProbabilityPlot),
                                     "Chart over test limits");
    comboProbabilityPlot->addItem(QIcon(*pixProbabilityPlot),
                                     "Chart over test results");
    comboProbabilityPlot->addItem(QIcon(*pixProbabilityPlot),
                                     "Adaptive: data & limits");

    // Fill 'Box plot' list
    comboBoxplotEx->clear();
    comboBoxplotEx->addItem(QIcon(*pixBoxPlot), "Chart over test limits");
    comboBoxplotEx->addItem(QIcon(*pixBoxPlot), "Chart over test results");
    comboBoxplotEx->addItem(QIcon(*pixBoxPlot), "Adaptive: data & limits");

    // Fill 'Multi-Charts' list
    comboBoxMultiChart->clear();
    comboBoxMultiChart->addItem(QIcon(*pixMultiCharts), "Chart over test limits");
    comboBoxMultiChart->addItem(QIcon(*pixMultiCharts), "Chart over test results");
    comboBoxMultiChart->addItem(QIcon(*pixMultiCharts), "Adaptive: data & limits");

    // Fill 'wafermap list'
    pGexMainWindow->pWizardSettings->
        FillListBox_Wafermap(comboBoxWafmap, false);

    // Fill Datalog list
    comboBoxTestTypeDatalog->clear();
    comboBoxTestTypeDatalog->insertItem(0, QIcon(), "All tests",
                                        QVariant(GEX_ADV_DATALOG_ALL));
    comboBoxTestTypeDatalog->insertItem(1, QIcon(), "Failing tests only",
                                        QVariant(GEX_ADV_DATALOG_FAIL));
    comboBoxTestTypeDatalog->insertItem(2, QIcon(), "Test list...",
                                        QVariant(GEX_ADV_DATALOG_LIST));
    comboBoxTestTypeDatalog->insertItem(3, QIcon(), "Test list : values only",
                                        QVariant(GEX_ADV_DATALOG_RAWDATA));

    // Connect signals: Aggregate report
    connect((QObject*) comboBoxSectionType,
            SIGNAL(activated(int)), SLOT(OnSectionType()));
    connect((QObject*) comboBoxTestList,
            SIGNAL(activated(int)), SLOT(OnSectionAggregate()));
    connect((QObject*) checkBoxStats,
            SIGNAL(clicked()), SLOT(OnSectionAggregate()));
    connect((QObject*) checkBoxHistogram,
            SIGNAL(clicked()), SLOT(OnSectionAggregate()));
    connect((QObject*) checkBoxTrend,
            SIGNAL(clicked()), SLOT(OnSectionAggregate()));
    connect((QObject*) checkBoxScatter,
            SIGNAL(clicked()), SLOT(OnSectionAggregate()));
    connect((QObject*) checkBoxProbability,
            SIGNAL(clicked()), SLOT(OnSectionAggregate()));
    connect((QObject*) checkBoxBoxplotEx,
            SIGNAL(clicked()), SLOT(OnSectionAggregate()));
    connect((QObject*) checkBoxMultiChart,
            SIGNAL(clicked()), SLOT(OnSectionAggregate()));
    connect((QObject*) checkBoxBoxPlot,
            SIGNAL(clicked()), SLOT(OnSectionAggregate()));
    // Pick Test from test-list
    connect((QObject*) PickTestStats,
            SIGNAL(clicked()), SLOT(OnPickAggregateTest()));
    connect((QObject*) PickTestScatterY,
            SIGNAL(clicked()), SLOT(OnPickAggregateTestY()));
    // Plotting style
    connect((QObject*) pushButtonLineColor,
            SIGNAL(colorChanged(const QColor &)),
            SLOT(OnAggregateLineColor(const QColor &)));
    connect((QObject*) comboBoxMarker,
            SIGNAL(activated(int)), SLOT(OnAggregateMarkerSelection()));
    connect((QObject*) pushButtonMarkerColor,
            SIGNAL(colorChanged(const QColor &)),
            SLOT(OnAggregateMarkerColor(const QColor &)));
    connect((QObject*) spinBoxMarkerWidth,
            SIGNAL(valueChanged(int)), SLOT(OnAggregateMarkerSize()));

    // Connect signals: Wafermap report
    connect((QObject*) comboBoxWafmap,
            SIGNAL(activated(int)), SLOT(OnSectionWafmap()));
    connect((QObject*) PickTestWafermap,
            SIGNAL(clicked()), SLOT(OnPickTestWafermap()));

    // Connect signals:  'tester-to-tester correlation & GuardBand'
    connect((QObject*) buttonImportGBFile,
            SIGNAL(clicked()), SLOT(OnCorrelationGbSelectFile()));
    connect((QObject*) pushButtonCreateGBfile,
            SIGNAL(clicked()), SLOT(OnCreateGuardBandFile()));

    // Connect signals:  'Data Audit'
    connect((QObject*) checkBoxAudit_SampleLimit,
            SIGNAL(clicked()), SLOT(OnSectionDataAudit()));
    connect((QObject*) checkBoxAudit_CpkLimit,
            SIGNAL(clicked()), SLOT(OnSectionDataAudit()));
    connect((QObject*) buttonFilterParameters,
            SIGNAL(clicked()), SLOT(OnAddFilter()));
    connect((QObject*) buttonEditFilter,
            SIGNAL(clicked()), SLOT(OnEditFilter()));
    connect((QObject*) buttonDuplicateFilter,
            SIGNAL(clicked()), SLOT(OnDuplicateFilter()));

    connect((QObject*) buttonRemoveFilter,
            SIGNAL(clicked()), SLOT(OnRemoveFilter()));
    connect((QObject*) PushButtonClearAllFilters,
            SIGNAL(clicked()), SLOT(OnRemoveAllFilters()));

    connect(listViewAuditFilters,
            SIGNAL(doubleClicked(QTreeWidgetItem*)), SLOT(OnEditFilter()));

    // Connect signal: 'Datalog'
    connect(PickTestDatalog, SIGNAL(clicked()), SLOT(onPickTestDatalog()));
    connect(comboBoxTestTypeDatalog,
            SIGNAL(activated(int)), SLOT(onTestTypeDatalog()));


    // Connect signals: Exit dialog box
    connect((QObject*) PushButtonOk, SIGNAL(clicked()), SLOT(OnOk()));

    // Default Title
    editSectionTitle->setText("Section...");

    // GUI fields at startup
    checkBoxBars->setChecked(true);
    checkBoxfittingCurve->setChecked(false);
    checkBoxBellCurve->setChecked(false);
    checkBoxLines->setChecked(true);
    checkBoxspots->setChecked(false);

    // Reset variables
    clear();

    // Reset GUI
    OnSectionType();
}

/******************************************************************************!
 * \fn clear
 * \brief Clear variables
 ******************************************************************************/
void ReportTemplateGui_EditSection::clear()
{
    cColor = QColor(0, 85, 0);

    // All markers
    int iMakerIndex;
    for (iMakerIndex = 0;
         iMakerIndex < GEX_CRPT_MARKER_TOTALSIZE;
         iMakerIndex++)
    {
        icMarkerWidth[iMakerIndex] = 0;
        cMarkerColor[iMakerIndex]  = Qt::red;
    }

    // Markers to make visible by default
    icMarkerWidth[GEX_CRPT_MARKER_STYLE_MEAN]   = 1;
    icMarkerWidth[GEX_CRPT_MARKER_STYLE_LIMITS] = 1;
}

/******************************************************************************!
 * \fn OnSectionType
 * \brief Display relevant report section editor
 ******************************************************************************/
void ReportTemplateGui_EditSection::OnSectionType()
{
    // Force focus on Section's title + suggest default title
    editSectionTitle->setFocus();

    switch (comboBoxSectionType->currentIndex())
    {
    case GEX_CRPT_SECTION_NONE:  // No valid selection
        // Disable OK button
        PushButtonOk->setEnabled(false);

        // Display 'Welcome' GUI
        widgetStackSectionType->setCurrentIndex(GEX_CRPT_WIDGET_WELCOME);

        // Hide section's title
        TextLabelTitle->hide();
        editSectionTitle->hide();
        break;

    case GEX_CRPT_SECTION_TEST_AGGREGATE:
        // Section type: Test aggregate: stats, charts, ...
        // Enable OK button
        PushButtonOk->setEnabled(true);

        // Suggest section title
        editSectionTitle->setText("Section: Charts & Statistics");

        // Display 'Aggregate' GUI
        widgetStackSectionType->setCurrentIndex(GEX_CRPT_WIDGET_AGGREGATE);
        OnSectionAggregate();  // Update GUI
        break;

    case GEX_CRPT_SECTION_TEST_WAFMAP:  // Section type: wafer map
        // Enable OK button
        PushButtonOk->setEnabled(true);

        // Suggest section title
        editSectionTitle->setText("Section: Wafer maps");

        // Display 'Wafermap' GUI
        widgetStackSectionType->setCurrentIndex(GEX_CRPT_WIDGET_WAFMAP);
        OnSectionWafmap();  // Update GUI
        break;

    case GEX_CRPT_SECTION_TEST_BINNING:  // Section type: Binning Summary
        // Enable OK button
        PushButtonOk->setEnabled(true);

        // Suggest section title
        editSectionTitle->setText("Section: Binning Summary");

        // Display 'Binning' GUI
        widgetStackSectionType->setCurrentIndex(GEX_CRPT_WIDGET_BINNING);
        break;

    case GEX_CRPT_SECTION_TEST_PARETO:  // Section type: Pareto reports
        // Enable OK button
        PushButtonOk->setEnabled(true);

        // Suggest section title
        editSectionTitle->setText("Section: Pareto reports");

        // Display 'Pareto' GUI
        widgetStackSectionType->setCurrentIndex(GEX_CRPT_WIDGET_PARETO);
        break;

    case GEX_CRPT_SECTION_TEST_PEARSON:  // Section type: Pearson's correlation
        // Enable OK button
        PushButtonOk->setEnabled(true);

        // Suggest section title
        editSectionTitle->setText("Section: Pearson's correlation");

        // Display 'Pearson's correlation' GUI
        widgetStackSectionType->setCurrentIndex(GEX_CRPT_WIDGET_PEARSON);
        break;

    case GEX_CRPT_SECTION_CORR_GB:
        // Section type: Tester-to-tester,
        // Correlation & GB: Disabled under PAT-Man
        if (GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT())
        {
            GS::Gex::Message::information("",
                                          "This report is not available "
                                          "under Examinator-PAT.");
            break;
        }

        // Enable OK button
        PushButtonOk->setEnabled(true);

        // Suggest section title
        editSectionTitle->
            setText("Section: Tester-to-tester Correlation + GuardBand");

        // Display 'Global info' GUI
        widgetStackSectionType->setCurrentIndex(GEX_CRPT_SECTION_CORR_GB);
        break;

    case GEX_CRPT_SECTION_GLOBAL_INFO:  // Section type: Global info
        // Enable OK button
        PushButtonOk->setEnabled(true);

        // Suggest section title
        editSectionTitle->setText("Section: Global Info");

        // Display 'Global info' GUI
        widgetStackSectionType->setCurrentIndex(GEX_CRPT_SECTION_GLOBAL_INFO);
        break;

    case GEX_CRPT_SECTION_FILE_AUDIT:  // Section type: File Audit
        // Enable OK button
        PushButtonOk->setEnabled(true);

        // Suggest section title
        editSectionTitle->setText("Section: Data Audit");

        // Display 'File Audit' GUI
        widgetStackSectionType->setCurrentIndex(GEX_CRPT_SECTION_FILE_AUDIT);
        OnSectionDataAudit();
        break;

    case GEX_CRPT_SECTION_PRODUCTION:  // Section type: Production reports
        // Enable OK button
        PushButtonOk->setEnabled(true);

        // Suggest section title
        editSectionTitle->setText("Section: Production reports");

        // Show Title field
        TextLabelTitle->show();
        editSectionTitle->show();

        // Display 'Production reports' GUI
        widgetStackSectionType->setCurrentIndex(GEX_CRPT_SECTION_PRODUCTION);
        break;

    case GEX_CRPT_SECTION_DATALOG:  // Section type: Datalog
        // Enable OK button
        PushButtonOk->setEnabled(true);

        // Suggest section title
        editSectionTitle->setText("Section: Datalog");

        // Show Title field
        TextLabelTitle->show();
        editSectionTitle->show();

        // Display 'Production reports' GUI
        widgetStackSectionType->setCurrentIndex(GEX_CRPT_WIDGET_DATALOG);
        onTestTypeDatalog();
        break;
    }
}

/******************************************************************************!
 * \fn OnCorrelationGbSelectFile
 * \brief Section: Correlation GuardBand
 *        User selects Guardband reference file
 ******************************************************************************/
void ReportTemplateGui_EditSection::OnCorrelationGbSelectFile()
{
    QString strDefaultSelection = editGBFile->text();;

    // Prompt user to select file
    QString strGBFile =
        QFileDialog::getOpenFileName(this,
                                     "Select Guard Band File",
                                     strDefaultSelection,
                                     "Guard band file (*.csv)");
    if (strGBFile.isEmpty())
    {
        return;
    }

    editGBFile->setText(strGBFile);
}

/******************************************************************************!
 * \fn OnCreateGuardBandFile
 * \brief Section: Correlation GuardBand
 *        User wants to have a Guardband reference file generated automatically
 ******************************************************************************/
void ReportTemplateGui_EditSection::OnCreateGuardBandFile()
{
    // Prompt user to select QC file
    QString strQCfile = QFileDialog:: getOpenFileName(
        this,
        "Select QC data file",
        "",
        "STDF QC file (*.std;*.stdf;*.zip;*.gz;*.Z)");

    if (strQCfile.isEmpty())
    {
        return;
    }

    QString strFTfile = QFileDialog::getOpenFileName(
        this,
        "Select FT data file",
        "",
        "STDF FT file (*.std;*.stdf;*.zip;*.gz;*.Z)");

    if (strFTfile.isEmpty())
    {
        return;
    }

    // Create a script to load the data files in 'compare'
    // files mode (so to have individual statistics
    // Create script that will read data file +
    // build the Outlier Removal report + wafermap
    QString strString;
    QString strScriptFile = GS::Gex::Engine::GetInstance().GetAssistantScript();
    FILE*   hFile = fopen(strScriptFile.toLatin1().constData(), "w");

    if (hFile == NULL)
    {
        strString = "  > Failed to create script file: " + strScriptFile;
        GS::Gex::Message::warning("", strString);
        return;
    }

    // Display status box
    QProgressDialog cProgress("Analyzing files...", "Abort", 0, 1, this);
    cProgress.setModal(true);
    cProgress.show();

    // Creates 'Preferences' section
    pGexMainWindow->pWizardScripting->WritePreferencesSection(hFile);

    // Creates 'SetOptions' section
    if (! ReportOptions.WriteOptionSectionToFile(hFile))
    {
        GEX_ASSERT(false);
        strString = QString("Error : can't write option section");
        GS::Gex::Message::warning("", strString);
        return;
    }

    // Creates 'SetProcessData' section
    fprintf(hFile, "SetProcessData()\n");
    fprintf(hFile, "{\n");
    fprintf(hFile, "  var group_id;\n");
    fprintf(hFile, "  gexGroup('reset','all');\n");

    // Write dataset QC group
    QString strNewSTDF = strQCfile;
    ConvertToScriptString(strNewSTDF);

    fprintf(hFile, "  group_id = gexGroup('insert','QC');\n");
    fprintf(hFile,
            "  gexFile(group_id,'insert','%s','All','all',' ','','',' ');\n\n",
            strNewSTDF.toLatin1().constData());

    // Write dataset FT group
    strNewSTDF = strFTfile;
    ConvertToScriptString(strNewSTDF);

    fprintf(hFile, "  group_id = gexGroup('insert','FT');\n");
    fprintf(hFile,
            "  gexFile(group_id,'insert','%s','All','all',' ','','',' ');\n\n",
            strNewSTDF.toLatin1().constData());

    fprintf(hFile, "}\n\n");

    // Creates 'main' section
    fprintf(hFile, "main()\n");
    fprintf(hFile, "{\n");
    fprintf(hFile, "  SetOptions();\n");
    fprintf(hFile, "  gexOptions('dataprocessing', 'duplicate_test', 'merge');\n");
    fprintf(hFile, "  gexOptions('output','format','interactive');\n");
    // Disable Examinator outlier GUI filter
    fprintf(hFile,"  gexOptions('dataprocessing','data_cleaning_mode','none');\n");
    // Forces to ALWAYS compute advanced statistics
    fprintf(hFile, "  gexOptions('speed','adv_stats','always');\n");
    // Force to compute statistics from samples, ignore summary
    fprintf(hFile,
            "  gexOptions('statistics','computation','samples_only');\n");
    fprintf(hFile, "  SetProcessData();\n");
    // Get report built
    fprintf(hFile, "  gexBuildReport('home','0');\n");  // Show report home page

    // Now that custom report is created, make sure
    // we reload standard Examinator settings
    fprintf(hFile, "\n");
    fprintf(hFile, "  SetPreferences();\n");
    fprintf(hFile, "  SetOptions();\n");

    // Last line of script executed must be the report format
    // otherwse it may conflict with the default options giving
    // a different format than what the user wants
    fprintf(hFile, "  gexOptions('output','format','html');\n");

    fprintf(hFile, "}\n\n");
    fclose(hFile);

    // Execute script
    CSLStatus lStatus = CSLEngine::GetInstance().RunScript(strScriptFile);

    // Hide progress bar.
    cProgress.hide();

    // Sets flag to say if we succeeded reading the STDF files
    if (lStatus.IsFailed())
    {
        // Failed processing QT & FT files
        GS::Gex::Message::
            warning("", "Failed analyzing data files...Files corrupted?");
        return;
    }

    // Ask where to save the GuardBande file
    QString strGuardBandFile = QFileDialog::getSaveFileName(
            this,
            "Save Guard Band as...",
            "guard_band.csv",
            "Quantix Guard Band (*.csv)",
            NULL,
            QFileDialog::
            DontConfirmOverwrite);

    // If no file selected, ignore command.
    if (strGuardBandFile.isEmpty())
    {
        return;
    }

    if (QFile::exists(strGuardBandFile))
    {
        bool lOk;
        GS::Gex::Message::request("", "Guard band file already exists, overwrite it ?", lOk);
        if (! lOk)
        {
            return;
        }
    }

    hFile = fopen(strGuardBandFile.toLatin1().constData(), "w");
    if (hFile == NULL)
    {
        strString = "Failed to create file. Write protected?\nFile: " +
            strGuardBandFile;
        GS::Gex::Message::warning("", strString);
        return;
    }

    // Write file header.
    fprintf(hFile,
            "########################################################\n");
    fprintf(hFile, "# Quantix Guard Band file\n");
    fprintf(hFile, "# Created with: %s\n",
            Engine::GetInstance().Get(
                "AppFullName").toString().toLatin1().data());
    fprintf(hFile, "# Date : %s\n",
            (char*) QDateTime::currentDateTime().toString(
                "d MMMM yyyy h:mm:ss").toLatin1().constData());
    fprintf(hFile,
            "########################################################\n");
    fprintf(hFile, "# Test,Name,Guard band\n");

    ////////////////////////////////////////////////////////////////////////////
    // Create guard band file
    ////////////////////////////////////////////////////////////////////////////
    CGexGroupOfFiles* pGroup_QC =
        (gexReport->getGroupsList().isEmpty()) ? NULL :
        (gexReport->getGroupsList().at(0));
    CGexGroupOfFiles* pGroup_FT =
        (gexReport->getGroupsList().size() < 2) ? NULL :
        (gexReport->getGroupsList().at(1));
    CGexFileInGroup* pFile_FT =
        (pGroup_FT->pFilesList.isEmpty()) ? NULL :
        (pGroup_FT->pFilesList.first());
    CTest* ptTestCell_QC;
    CTest* ptTestCell_FT;
    int    iTotalNoDualLimits = 0;
    int    iTotalTestsEntries = 0;
    double lfDeltaLowL, lfDeltaHighL, lfMinDelta;

    ptTestCell_QC = pGroup_QC->cMergedData.ptMergedTestList;
    while (ptTestCell_QC != NULL)
    {
        // Get pointer to matching test# in the FT dataset.
        pFile_FT->
            FindTestCell(ptTestCell_QC->lTestNumber,
                         ptTestCell_QC->lPinmapIndex,
                         &ptTestCell_FT,
                         false,
                         false,
                         NULL);

        if (ptTestCell_FT == NULL)
        {
            goto next_test;
        }

        // Step1: extract high and low limit from QC program
        // Step2: extract high and low limit from FT program
        // Step3: extract units
        // Step4: Calculate delta of low limits (absolute number) ... A
        // Step5: Calculate delta of high limits (absolute number) ... B
        // Step6: Calculate max delta (A or B)
        // Step7: Normailise (using extracted units as multiplier)
        // Step8: Output file containing: <test #>, <test name>,
        //        <guardband value>

        if (((ptTestCell_QC->GetCurrentLimitItem()->bLimitFlag &
              (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL)) == 0) &&
            ((ptTestCell_FT->GetCurrentLimitItem()->bLimitFlag &
              (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL)) == 0))
        {
            // Both limits exist
            lfDeltaLowL =
                fabs(ptTestCell_QC->GetCurrentLimitItem()->lfLowLimit - ptTestCell_FT->GetCurrentLimitItem()->lfLowLimit);
            lfDeltaHighL =
                fabs(ptTestCell_QC->GetCurrentLimitItem()->lfHighLimit - ptTestCell_FT->GetCurrentLimitItem()->lfHighLimit);
            lfMinDelta = gex_max(lfDeltaLowL, lfDeltaHighL);

            // Write info into file.
            fprintf(hFile,
                    "%d,%s,%g\n",
                    ptTestCell_QC->lTestNumber,
                    ptTestCell_QC->strTestName.toLatin1().constData(),
                    lfMinDelta);

            // Keep track of total test entries written in GuardBand file
            iTotalTestsEntries++;
        }
        else
        {
            // Keep track of total tests without dual limits
            iTotalNoDualLimits++;
        }
        // See next test cell
next_test:
        ptTestCell_QC = ptTestCell_QC->GetNextTest();
    }

    // Close guard band file
    fclose(hFile);

    // Success
    strString =
        "Guard band file successfully created!\nFile: " + strGuardBandFile;
    GS::Gex::Message::information("", strString);
}

/******************************************************************************!
 * \fn OnSectionDataAudit
 * \brief Section: Data Audit
 *        User checks/unchecks some GUI Widget
 ******************************************************************************/
void ReportTemplateGui_EditSection::OnSectionDataAudit()
{
    // If PAT enabled, show the PAT related tabs
    bool bEnablePatTab = (GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT());
    // 'Tests audit' tab
    tabWidgetStdfChecker->setTabEnabled(1, bEnablePatTab);
    // 'Filters' tab
    tabWidgetStdfChecker->setTabEnabled(2, bEnablePatTab);

    // Samples limit set ?
    if (checkBoxAudit_SampleLimit->isChecked())
    {
        lineEditAuditSampleLimit->show();
    }
    else
    {
        lineEditAuditSampleLimit->hide();
    }

    // Cpk limit set ?
    if (checkBoxAudit_CpkLimit->isChecked())
    {
        lineEditAuditCpkLimit->show();
    }
    else
    {
        lineEditAuditCpkLimit->hide();
    }
}

/******************************************************************************!
 * \fn OnAddFilter
 * \brief Section: Data Audit
 *        User adds a filter
 ******************************************************************************/
void ReportTemplateGui_EditSection::OnAddFilter()
{
    PickAuditFilterDialog cAuditFilter;

    if (cAuditFilter.exec() != 1)
    {
        return;
    }

    // insert section in list view
    QStringList labels;
    labels.append(cAuditFilter.getProduct());
    labels.append(cAuditFilter.getFilterType());
    labels.append(cAuditFilter.getFilterString());
    new QTreeWidgetItem(listViewAuditFilters,
                        labels);
}

/******************************************************************************!
 * \fn OnEditFilter
 * \brief Section: Data Audit
 *        User adds a filter
 ******************************************************************************/
void ReportTemplateGui_EditSection::OnEditFilter()
{
    PickAuditFilterDialog cAuditFilter;

    // Extract fields from GUI
    QTreeWidgetItem* ptItem = listViewAuditFilters->currentItem();
    if (ptItem == NULL)
    {
        return;
    }

    QString strProduct      = ptItem->text(0);
    QString strFilterType   = ptItem->text(1);
    QString strFilterString = ptItem->text(2);
    cAuditFilter.setFields(strProduct, strFilterType, strFilterString);

    // Show fields
    if (cAuditFilter.exec() != 1)
    {
        return;
    }

    // Delete original filter to replace with new one
    delete ptItem;

    // update section in list view
    QStringList labels;
    labels.append(cAuditFilter.getProduct());
    labels.append(cAuditFilter.getFilterType());
    labels.append(cAuditFilter.getFilterString());
    new QTreeWidgetItem(listViewAuditFilters,
                        labels);
}

/******************************************************************************!
 * \fn OnDuplicateFilter
 * \brief Section: Data Audit
 *        User adds a filter
 ******************************************************************************/
void ReportTemplateGui_EditSection::OnDuplicateFilter()
{
    // Extract fields from GUI
    QTreeWidgetItem* ptItem = listViewAuditFilters->currentItem();
    if (ptItem == NULL)
    {
        return;
    }

    QString strProduct      = ptItem->text(0);
    QString strFilterType   = ptItem->text(1);
    QString strFilterString = ptItem->text(2);
    PickAuditFilterDialog cAuditFilter;
    cAuditFilter.setFields(strProduct, strFilterType, strFilterString);

    // add entry in list view
    QStringList labels;
    labels.append(cAuditFilter.getProduct());
    labels.append(cAuditFilter.getFilterType());
    labels.append(cAuditFilter.getFilterString());
    new QTreeWidgetItem(listViewAuditFilters,
                        labels);
}

/******************************************************************************!
 * \fn OnRemoveFilter
 * \brief Section: Data Audit
 *        User removes selected filter
 ******************************************************************************/
void ReportTemplateGui_EditSection::OnRemoveFilter()
{
    int iEntryIndex = 0;
    QTreeWidgetItem* ptItem = listViewAuditFilters->itemAt(0,0);

    while (ptItem != NULL)
    {
        if (ptItem == listViewAuditFilters->currentItem())
        {
            // Delete GUI selection
            delete ptItem;
            return;
        }

        // Move to next node
        ptItem = listViewAuditFilters->itemBelow(ptItem);

        // Update Item#
        iEntryIndex++;
    }
}

/******************************************************************************!
 * \fn OnRemoveAllFilters
 * \brief Section: Data Audit
 *        User removes all filters
 ******************************************************************************/
void ReportTemplateGui_EditSection::OnRemoveAllFilters()
{
    // Empty list, do nothing
    if (listViewAuditFilters->topLevelItemCount() == 0)
    {
        return;
    }

    // Ask 'Delete' confirmation
    QString     strMessage = "Confirm to Remove All filters?";
    bool lOk;
    GS::Gex::Message::request("Remove Filters", strMessage, lOk);
    if (! lOk)
    {
        return;
    }

    // Empty GUI
    listViewAuditFilters->clear();
}

/******************************************************************************!
 * \fn OnPickAggregateTest
 * \brief Section: Aggregate report
 *        Display Test list and let use pick some
 ******************************************************************************/
void ReportTemplateGui_EditSection::OnPickAggregateTest()
{
    // Show TestList
    PickTestDialog dPickTest;

    // Allow Multiple selections
    dPickTest.setMultipleSelection(true);

    // Check if List was successfuly loaded
    // then Prompt dialog box, let user pick tests from the list
    if (dPickTest.fillParameterList() && dPickTest.exec() == QDialog::Accepted)
    {
        // Save the list selected into the edit field
        editTestStats->setText(dPickTest.testList());
    }
}

/******************************************************************************!
 * \fn OnAggregateLineColor
 * \brief Section: Aggregate report
 *        Define pen size
 ******************************************************************************/
void ReportTemplateGui_EditSection::OnAggregateLineColor(
    const QColor& colorChanged)
{
    // Save selected color
    cColor = colorChanged;
}

/******************************************************************************!
 * \fn OnAggregateMarkerSelection
 * \brief Section: Aggregate report
 *        Define Marker to customize
 ******************************************************************************/
void ReportTemplateGui_EditSection::OnAggregateMarkerSelection()
{
    // Get marker index
    int iMarkerIndex = comboBoxMarker->currentIndex();

    // Display current line width assigned to this marker
    spinBoxMarkerWidth->setValue(icMarkerWidth[iMarkerIndex]);

    // Display current color assigned to this marker
    pushButtonMarkerColor->setActiveColor(cMarkerColor[iMarkerIndex]);
}

/******************************************************************************!
 * \fn OnAggregateMarkerColor
 * \brief Section: Aggregate report
 *        Define Marker pen color
 ******************************************************************************/
void ReportTemplateGui_EditSection::OnAggregateMarkerColor(
    const QColor& colorChanged)
{
    // Get marker index
    int iMarkerIndex = comboBoxMarker->currentIndex();

    // Save selected color
    cMarkerColor[iMarkerIndex] = colorChanged;
}

/******************************************************************************!
 * \fn OnAggregateMarkerSize
 * \brief Section: Aggregate report
 *        Define Marker pen size
 ******************************************************************************/
void ReportTemplateGui_EditSection::OnAggregateMarkerSize()
{
    // Get marker index
    int iMarkerIndex = comboBoxMarker->currentIndex();

    // Save MArker width selected
    icMarkerWidth[iMarkerIndex] = spinBoxMarkerWidth->value();
}

/******************************************************************************!
 * \fn OnPickAggregateTestY
 * \brief Section: Aggregate report
 *        Display Test list and let user pick some
 *        (Aggregate Scatter plot Y axis report)
 ******************************************************************************/
void ReportTemplateGui_EditSection::OnPickAggregateTestY()
{
    // Show TestList
    PickTestDialog dPickTest;

    // Allow Multiple selections
    dPickTest.setMultipleSelection(true);

    // Check if List was successfuly loaded
    // Then prompt dialog box, let user pick tests from the list
    if (dPickTest.fillParameterList() && dPickTest.exec() == QDialog::Accepted)
    {
        // Save the list selected into the edit field
        editTestY->setText(dPickTest.testList());
    }
}

/******************************************************************************!
 * \fn onPickTestDatalog
 * \brief Section: Datalog
 *        Display Test list and let use pick some
 ******************************************************************************/
void ReportTemplateGui_EditSection::onPickTestDatalog()
{
    // Show TestList
    PickTestDialog dPickTest;

    // Allow Multiple selections
    dPickTest.setMultipleSelection(true);

    // Check if List was successfuly loaded
    // Then prompt dialog box, let user pick tests from the list
    if (dPickTest.fillParameterList() && dPickTest.exec() == QDialog::Accepted)
    {
        // Save the list selected into the edit field
        editTestListDatalog->setText(dPickTest.testList());
    }
}

/******************************************************************************!
 * \fn onTestTypeDatalog
 * \brief Section type to create: Wafermap
 ******************************************************************************/
void ReportTemplateGui_EditSection::onTestTypeDatalog()
{
    // Show Title field
    TextLabelTitle->show();
    editSectionTitle->show();

    // Get item selected
    int iComboValue = comboBoxTestTypeDatalog->itemData(
            comboBoxTestTypeDatalog->currentIndex()).toInt();

    switch (iComboValue)
    {
    default:
    case GEX_ADV_DATALOG_ALL:
    case GEX_ADV_DATALOG_FAIL:
        editTestListDatalog->hide();
        PickTestDatalog->hide();
        TextLabelTestListDatalog->hide();
        break;
    case GEX_ADV_DATALOG_LIST:
    case GEX_ADV_DATALOG_RAWDATA:
        editTestListDatalog->show();
        PickTestDatalog->show();
        TextLabelTestListDatalog->show();
        break;
    }
}

}
}
