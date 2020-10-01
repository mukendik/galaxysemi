///////////////////////////////////////////////////////////
// Interactive Drill: What-If
///////////////////////////////////////////////////////////
#ifdef _WIN32
  #include <windows.h>
#endif

#include <QCheckBox>
#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QPainter>
#include <QFileDialog>
#include <QDragEnterEvent>

#include "drill_what_if.h"
#include "browser_dialog.h"
#include "engine.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "gex_report.h"
#include "gex_shared.h"
#include "report_build.h"
#include "report_options.h"
#include "snapshot_dialog.h"
#include "cstats.h"
#include "math.h"
#include <gqtl_log.h>
#include "product_info.h"
#include "message.h"
#include <gqtl_global.h>

#if defined unix || __MACH__
#include <stdlib.h>
#include <unistd.h>
#endif

// External definitions for pointers to pixmaps.
#include "gex_pixmap_extern.h"

// main.cpp
extern GexMainwindow *	pGexMainWindow;

// in report_build.cpp
extern CGexReport*		gexReport;			// Handle to report class
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)

// Define header label
#define TABLE_HEADER_TEST			"Test"
#define TABLE_HEADER_NAME			"Name"
#define TABLE_HEADER_FAIL_BIN		"Fail Bin"						// Introduced in V6.1 B155
#define TABLE_HEADER_LL_NEW			"Low Limit - New"
#define TABLE_HEADER_HL_NEW			"High Limit - New"
#define TABLE_HEADER_CP_NEW			"Cp - New"
#define TABLE_HEADER_CR_NEW         "Cr - New"
#define TABLE_HEADER_CPK_NEW		"Cpk - New"
#define TABLE_HEADER_YIELD_NEW		"Yield - New"
#define TABLE_HEADER_LL_REF			"Low L. - Ref"
#define TABLE_HEADER_HL_REF			"High L. - Ref"
#define TABLE_HEADER_CP_REF			"Cp - Ref"
#define TABLE_HEADER_CR_REF         "Cr - Ref"
#define TABLE_HEADER_CPK_REF		"Cpk - Ref"
#define TABLE_HEADER_YIELD_REF		"Yield - Ref"
#define TABLE_HEADER_MEAN			"Mean"
#define TABLE_HEADER_SIGMA			"Sigma"
#define TABLE_HEADER_DELTA_YIELD	"Delta Yield (New - Ref)"

// Define column position
#define TABLE_COLUMN_TEST			0
#define TABLE_COLUMN_NAME			1
#define TABLE_COLUMN_FAIL_BIN		2
#define TABLE_COLUMN_LL_NEW			3
#define TABLE_COLUMN_HL_NEW			4
#define TABLE_COLUMN_CP_NEW			5
#define TABLE_COLUMN_CPK_NEW		6
#define TABLE_COLUMN_CR_NEW         7
#define TABLE_COLUMN_YIELD_NEW		8
#define TABLE_COLUMN_LL_REF			9
#define TABLE_COLUMN_HL_REF			10
#define TABLE_COLUMN_CP_REF			11
#define TABLE_COLUMN_CPK_REF		12
#define TABLE_COLUMN_CR_REF         13
#define TABLE_COLUMN_YIELD_REF		14
#define TABLE_COLUMN_MEAN			15
#define TABLE_COLUMN_SIGMA			16
#define TABLE_COLUMN_DELTA_YIELD	17


#define TABLE_COLUMNS				18

///////////////////////////////////////////////////////////
// Dialog box Constructor
///////////////////////////////////////////////////////////
GexDrillWhatIf::GexDrillWhatIf( QWidget* parent, bool modal, Qt::WindowFlags fl) : QDialog(parent, fl)
{
    setupUi(this);
    setModal(modal);

    // apply gex palette
    GexMainwindow::applyPalette(this);

    // Support for Drag&Drop
    setAcceptDrops(true);

    QObject::connect(buttonSnapshot,    SIGNAL(clicked()), this, SLOT(OnSnapshot()));
    QObject::connect(buttonReset,    SIGNAL(clicked()), this, SLOT(OnResetTable()));
    QObject::connect(buttonExportData,    SIGNAL(clicked()), this, SLOT(OnSaveTableToDisk()));
    QObject::connect(m_poExportToMapFile,            SIGNAL(clicked()), this, SLOT(OnSaveLimitsToMapFile()));
    QObject::connect(buttonExportExcelClipboard,    SIGNAL(clicked()), this, SLOT(OnSaveTableToExcelClipboard()));
    QObject::connect(buttonIdealLimits,    SIGNAL(clicked()), this, SLOT(OnSuggestIdealLimits()));
    QObject::connect(buttonDetachWindow,    SIGNAL(clicked()), this, SLOT(OnDetachWindow()));
    QObject::connect(buttonAddFile,    SIGNAL(clicked()), this, SLOT(OnLoadTableFromDisk()));

    // At startip, this window is a child of Examinator Framr
    bChildWindow = true;
    buttonDetachWindow->setChecked(bChildWindow);

    bResetAllTable = false;

    // Create table.
    TableGuardBanding = new Table(this);
    horizontalLayoutTable->addWidget(TableGuardBanding);

    // User can select multiple cells in a same column.
    TableGuardBanding->setSelectionBehavior(QAbstractItemView::SelectRows);
    TableGuardBanding->setItemDelegate(new WhatIfItemDelegate());
    TableGuardBanding->verticalHeader()->setDefaultSectionSize(20);

    // Signal that a cell content has been changed.
    connect( TableGuardBanding, SIGNAL( cellChanged( int, int ) ),
         this, SLOT( OnCellChanged( int, int ) ) );

    // Connect signal for 'contextual menu'
    connect( TableGuardBanding, SIGNAL(customContextMenuRequested(const QPoint &)),
         this, SLOT(OnContextualMenu(const QPoint &)));

    // Disable some functions under Solaris & HPUX (clipboard copy).
#if (defined __sun__ || __hpux__)
    buttonExportExcelClipboard->hide();
#endif

    // Enable/disable some features...
    switch(GS::LPPlugin::ProductInfo::getInstance()->getProductID())
    {
        case GS::LPPlugin::LicenseProvider::eLtxcOEM:   // OEM-Examinator for LTXC
            buttonExportExcelClipboard->hide();
            Line1->hide();
            buttonIdealLimits->hide();
            break;

        case GS::LPPlugin::LicenseProvider::eSzOEM:			// OEM-Examinator for Credence SZ
        {
                buttonExportExcelClipboard->hide();
                Line1->hide();
                buttonIdealLimits->hide();
                // Credence requested backround to be white...
                QPalette p = palette();
                p.setColor(backgroundRole(), Qt::white);
                setPalette(p);
        }
            break;

        default:
            break;
    }

    pSuggestLimits = NULL;

    comboBoxComputeOn->setItemData(0, QVariant(CGexReport::allParts));
    comboBoxComputeOn->setItemData(1, QVariant(CGexReport::goodParts));
    comboBoxComputeOn->setItemData(2, QVariant(CGexReport::failedParts));
}

const char* CalculateCr(QString CP)
{
    // -- change "," by "." to perform the double convertion
    int lIndex =CP.indexOf(",");

    if(lIndex > 0)
        CP.replace(lIndex, 1, ".");

    bool lConvertion = false;
    double lCPDouble = CP.toDouble(&lConvertion);
    if(lConvertion && lCPDouble != 0)
    {
       return gexReport->CreateResultStringCpCrCpk(1./lCPDouble);
    }
    else
    {
        return "n/a";
    }
}

///////////////////////////////////////////////////////////
// Starting DRAG sequence
///////////////////////////////////////////////////////////
void GexDrillWhatIf::dragEnterEvent(QDragEnterEvent *e)
{
    // Accept Drag if files list dragged over.
    if(e->mimeData()->formats().contains("text/uri-list"))
        e->acceptProposedAction();
}

///////////////////////////////////////////////////////////
// Dropping files in Widget (Drag & Drop)
///////////////////////////////////////////////////////////
void GexDrillWhatIf::dropEvent(QDropEvent *e)
{
    if(!e->mimeData()->formats().contains("text/uri-list"))
    {
        // No files dropped...ignore drag&drop.
        e->ignore();
        return;
    }

    QString		strFileName;
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

    // Check file extension: only process .CSV file!
    QString strFile = strFileList[0];
    if(strFile.endsWith(".csv", Qt::CaseInsensitive))
    {
        // Select file
        OnLoadTableFromDisk(strFile);
    }


    e->acceptProposedAction();
}

///////////////////////////////////////////////////////////
// Close signal: If window is detached, this reparent it instead
///////////////////////////////////////////////////////////
void GexDrillWhatIf::closeEvent(QCloseEvent *e)
{
    if(bChildWindow)
        e->accept();
    else
    if(pGexMainWindow!=NULL)
    {
        // Re-attac window to Examinator!
        buttonDetachWindow->setChecked(true);
        ForceAttachWindow(buttonDetachWindow->isChecked(), false);

        e->ignore();
    }

}

///////////////////////////////////////////////////////////
// NON-GUI Function to decide to attached or detach the window
///////////////////////////////////////////////////////////
void GexDrillWhatIf::ForceAttachWindow(bool bAttach/*=true*/, bool bToFront /*= true*/)
{
    bChildWindow = bAttach;
    buttonDetachWindow->setChecked(bChildWindow);

    if(bChildWindow && pGexMainWindow != NULL)
    {
        // Re-attacch dialog box to Examinator's scroll view Widget
        pGexMainWindow->pScrollArea->layout()->addWidget(this);

        if (bToFront)
            pGexMainWindow->ShowWizardDialog(GEX_DRILL_WHATIF_WIZARD_P1);
        else
            hide();
    }
    else
    {
        // Checks window has no parent
        if(parent() != NULL)
        {
            pGexMainWindow->pScrollArea->layout()->removeWidget(this);
            setParent(NULL, Qt::Dialog);

            // Setup the application buttons
            setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);

            move(QPoint(100, 100));
        }

        show();

        // Make sure the HTML page is visible now that this window is detached from Examinator.
        if(pGexMainWindow != NULL)
            pGexMainWindow->ShowHtmlBrowser();
    }
}

///////////////////////////////////////////////////////////
// GUI toggle button: Attach / Detach window
///////////////////////////////////////////////////////////
void GexDrillWhatIf::OnDetachWindow(void)
{
    ForceAttachWindow(buttonDetachWindow->isChecked());
}

///////////////////////////////////////////////////////////
// Show page: make it visible & update GUI fields
///////////////////////////////////////////////////////////
void GexDrillWhatIf::ShowPage(void)
{
    // Make Widget visible.
    show();

    // check if 'outlier removal' correctly set....
    if (ReportOptions.GetOption("dataprocessing","data_cleaning_mode").toString()=="n_pourcent")
        GS::Gex::Message::warning(
            "",
            "If you want accurate 'Yield' values computed, you must disable\n"
            "the 'Data cleaning over limits' "
            "(from 'Options/Data processing options' tab).\n"
            "Either disable it or set it to a N*Sigma value.");
}


///////////////////////////////////////////////////////////
// Constructor: Compute Ideal Test Limits
///////////////////////////////////////////////////////////
GexSuggestLimits::GexSuggestLimits( QWidget* parent, bool modal, Qt::WindowFlags fl)
    : QDialog( parent, fl )
{
    setupUi(this);
    setModal(modal);

    // Apply Examinator palette
    GexMainwindow::applyPalette(this);

    QObject::connect(PushButtonOk,		SIGNAL(clicked()),			this, SLOT(accept()));
    QObject::connect(PushButtonCancel,	SIGNAL(clicked()),			this, SLOT(reject()));
    QObject::connect(comboBoxRuleType,	SIGNAL(activated(QString)), this, SLOT(OnRuleTypeChange()));

    // Default
    comboBoxRuleType->setCurrentIndex(0);	// Limit is +/-N*Sigma
    OnRuleTypeChange();
    setContextMenuPolicy(Qt::CustomContextMenu);
}

///////////////////////////////////////////////////////////
// User selects the rule to use when computing Ideal limits
///////////////////////////////////////////////////////////
void	GexSuggestLimits::OnRuleTypeChange()
{
    switch(comboBoxRuleType->currentIndex())
    {
        case 0:	// Limits is N*Sigma
            TextLabelParameterType->setText("With N =");

            // Default Value
            lineEdit->setText("3.0");
            break;

        case 1:	// Limits is X% of original limits space
            TextLabelParameterType->setText("With X =");

            // Default Value
            lineEdit->setText("100.0 %");
            break;

        case 2:	// Limits is computed so we have a targetted Cp
            TextLabelParameterType->setText("With Cp target:");

            // Default Value
            lineEdit->setText("1.33");
            break;

        case 3:	// Limits is computed so we have a targetted Cpk
            TextLabelParameterType->setText("With Cpk target:");

            // Default Value
            lineEdit->setText("1.33");
            break;

        case 4:	// Limits is computed so we have a minimum given Cp
            TextLabelParameterType->setText("With Cp no less than:");

            // Default Value
            lineEdit->setText("1.33");
            break;

        case 5:	// Limits is computed so we have a minimum given Cpk
            TextLabelParameterType->setText("With Cpk no less than:");

            // Default Value
            lineEdit->setText("1.33");
            break;
    }

    // Set focus on Edit field
    lineEdit->setFocus();
}

///////////////////////////////////////////////////////////
// User selects the rule to use when computing Ideal limits
///////////////////////////////////////////////////////////
int	GexSuggestLimits::GetRuleMode()
{
    switch(comboBoxRuleType->currentIndex())
    {
        default:
        case 0:	// Limits is +/-N*Sigma
            return DRILL_WHAT_IF_RULE_NSIGMA;

        case 1:	// Limits is X% of original limits space
            return DRILL_WHAT_IF_RULE_XPERCENT;

        case 2:	// Limits is computed so we have a given Cp
            return DRILL_WHAT_IF_RULE_CP_TARGET;

        case 3:	// Limits is computed so we have a given Cpk
            return DRILL_WHAT_IF_RULE_CPK_TARGET;

        case 4:	// Limits is computed so we have a minimum given Cp
            return DRILL_WHAT_IF_RULE_CP;

        case 5:	// Limits is computed so we have a minimum given Cpk
            return DRILL_WHAT_IF_RULE_CPK;
    }
}

///////////////////////////////////////////////////////////
// User selects the rule to use when computing Ideal limits
///////////////////////////////////////////////////////////
double GexSuggestLimits::GetTarget()
{
   return lineEdit->text().toDouble();

    /*QString strValue = lineEdit->text();
    double lfValue=0.0;
    if(strValue.isEmpty() == false)
        sscanf(strValue.toLatin1().constData(),"%lf",&lfValue);

    return lfValue;*/
}

///////////////////////////////////////////////////////////
// Dialog box to compute + show ideal limits
///////////////////////////////////////////////////////////
void GexDrillWhatIf::OnSuggestIdealLimits(void)
{
    if(pSuggestLimits == NULL)
        pSuggestLimits = (GexSuggestLimits *) new GexSuggestLimits(this);

    // Display Ideal-Limit rule picker Diaog box
    if(pSuggestLimits->exec() != 1)
        return;	// User 'Abort'

    // Get target limit.
    double lfTargetCutoff = fabs(pSuggestLimits->GetTarget());

    QString strLabel;
    QString strName;
    QString strString;
    int	iLine;
    TableItem *pCell;
    unsigned long	lTestNumber;
    long	lPinmapIndex;
    CGexGroupOfFiles *pGroup;
    CGexFileInGroup  *pFile;
    CTest			 *ptTestCell;	// Pointer to test cell

    if(gexReport == NULL)
        return;	// Just in case

    // First: write info about reference group#1
    pGroup = gexReport->getGroupsList().isEmpty()?NULL:gexReport->getGroupsList().first();	// Group#1
    if(pGroup == NULL)
        return;	// Just in case

    // First file in Group#1...must always exist.
    pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
    if(pFile == NULL)
        return;	// Just in case

    // Check all selections...in a same column...and force them to the same value.
    double	lfLowL,lfHighL,lfOffset=0.0;
    double	fYield=0.0,fOrgYield=0.0,lfSaveLowL=0.0,lfSaveHighL=0.0,lfSaveCp=0.0,lfSaveCpk=0.0;
    BYTE	bSaveFlag=0;
    char	szString[50];
    bool	bUpdatedLowLimits,bUpdatedHighLimits;
    CGexStats cStats;

    // Ensure we use latest options set
    cStats.UpdateOptions(&ReportOptions);

    for(iLine=0;iLine<TableGuardBanding->rowCount();iLine++)
    {
        // Get test number
        pCell = (TableItem *) TableGuardBanding->item(iLine, TABLE_COLUMN_TEST);
        strLabel = pCell->text();

        if(sscanf(strLabel.toLatin1().constData(),"%lu%*c%ld",&lTestNumber,&lPinmapIndex) < 2)
            lPinmapIndex = GEX_PTEST;

        // Get test name
        pCell = (TableItem *) TableGuardBanding->item(iLine, TABLE_COLUMN_NAME);
        strName = pCell->text();

        // Check if can find given test#.Pinmap#
        if(pFile->FindTestCell(lTestNumber,lPinmapIndex,&ptTestCell,false,false, strName.toLatin1().data())==1)
        {
            bUpdatedLowLimits = bUpdatedHighLimits = false;
            if (ptTestCell->lfSigma == 0.0)
            {
                // Get handle to GUI cell that holds the LowLimit
                pCell = (TableItem *) TableGuardBanding->item(iLine, TABLE_COLUMN_LL_NEW);
                pCell->SetWhatIfFlags(GEX_DRILL_NULL_SIGMA);
                pCell->setText("n/a Sigma=0");
                // Force to repaint this cell
                pCell->paint();

                // Get handle to GUI cell that holds the highLimit
                pCell = (TableItem *) TableGuardBanding->item(iLine, TABLE_COLUMN_HL_NEW);
                pCell->SetWhatIfFlags(GEX_DRILL_NULL_SIGMA);
                pCell->setText("n/a Sigma=0");
                // Force to repaint this cell
                pCell->paint();
            }
            else
            {

                switch(pSuggestLimits->GetRuleMode())
                {
                case DRILL_WHAT_IF_RULE_NSIGMA:
                    // Formula: LowL  = Median - Offset
                    // Formula: HighL = Median + Offset
                    // with, Offset = N*Sigma (limits = +/- N*Sigma)
                    lfOffset = fabs(lfTargetCutoff*ptTestCell->lfSigma);

                    // Compute: LowL
                    lfLowL = ptTestCell->lfSamplesQuartile2 - lfOffset;
                    // Compute: HighL
                    lfHighL = ptTestCell->lfSamplesQuartile2 + lfOffset;

                    // Flags that limits have been updated
                    bUpdatedLowLimits = bUpdatedHighLimits = true;
                    break;

                case DRILL_WHAT_IF_RULE_XPERCENT:
                    // Formula: LowL  = Low - X%*LimitsSpace/2
                    // Formula: HighL = High + X%*LimitsSpace/2
                    if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL)) == 0)
                        lfOffset = (lfTargetCutoff-100)*fabs((ptTestCell->GetCurrentLimitItem()->lfHighLimit - ptTestCell->GetCurrentLimitItem()->lfLowLimit)/200.0);
                    else
                        if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
                            lfOffset = (lfTargetCutoff-100)*fabs((ptTestCell->lfSamplesQuartile2 - ptTestCell->GetCurrentLimitItem()->lfLowLimit)/100.0);
                        else
                            if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
                                lfOffset = (lfTargetCutoff-100)*fabs((ptTestCell->GetCurrentLimitItem()->lfHighLimit - ptTestCell->lfSamplesQuartile2)/100.0);

                    // Compute: LowL
                    if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
                    {
                        lfLowL = ptTestCell->GetCurrentLimitItem()->lfLowLimit - lfOffset;

                        // Flags that limits have been updated
                        bUpdatedLowLimits = true;
                    }
                    else
                        lfLowL = ptTestCell->GetCurrentLimitItem()->lfLowLimit;


                    // Compute: HighL
                    if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
                    {
                        lfHighL = ptTestCell->GetCurrentLimitItem()->lfHighLimit + lfOffset;

                        // Flags that limits have been updated
                        bUpdatedHighLimits = true;
                    }
                    else
                        lfHighL = ptTestCell->GetCurrentLimitItem()->lfHighLimit;
                    break;

                case DRILL_WHAT_IF_RULE_CP:
                    // Fall into next case (DRILL_WHAT_IF_RULE_CP_TARGET) only if Cp too low...
                    if(ptTestCell->GetCurrentLimitItem()->lfCp >= lfTargetCutoff)
                        break;	// Cp higher than cut-off, so don't change its limits!
                case DRILL_WHAT_IF_RULE_CP_TARGET:
                    lfOffset = fabs(lfTargetCutoff*3*ptTestCell->lfSigma);

                    // Compute: LowL
                    lfLowL = ptTestCell->lfSamplesQuartile2 - lfOffset;
                    // Compute: HighL
                    lfHighL = ptTestCell->lfSamplesQuartile2 + lfOffset;

                    // Flags that limits have been updated
                    bUpdatedLowLimits = bUpdatedHighLimits = true;
                    break;

                case DRILL_WHAT_IF_RULE_CPK:
                    // Fall into next case (DRILL_WHAT_IF_RULE_CPK_TARGET) only if Cpk too low...
                    if(ptTestCell->GetCurrentLimitItem()->lfCpk >= lfTargetCutoff)
                        break;	// Cpk higher than cut-off, so don't change its limits!
                case DRILL_WHAT_IF_RULE_CPK_TARGET:
                    lfOffset = fabs(lfTargetCutoff*3*ptTestCell->lfSigma);

                    // Compute: LowL & HighL
                    if(ptTestCell->ldSamplesValidExecs)
                    {
                        lfLowL = (ptTestCell->lfSamplesTotal/ptTestCell->ldSamplesValidExecs) - lfOffset;
                        lfHighL = (ptTestCell->lfSamplesTotal/ptTestCell->ldSamplesValidExecs) + lfOffset;
                    }
                    else
                    {
                        lfLowL = ptTestCell->lfSamplesQuartile2 - lfOffset;
                        lfHighL = ptTestCell->lfSamplesQuartile2 + lfOffset;
                    }

                    // Flags that limits have been updated
                    bUpdatedLowLimits = bUpdatedHighLimits = true;
                    break;
                }


                if(bUpdatedLowLimits || bUpdatedHighLimits)
                {
                    // Save real limits
                    bSaveFlag  = ptTestCell->GetCurrentLimitItem()->bLimitFlag;
                    lfSaveCp   = ptTestCell->GetCurrentLimitItem()->lfCp;
                    lfSaveCpk  = ptTestCell->GetCurrentLimitItem()->lfCpk;
                }

                if(bUpdatedLowLimits)
                {
                    // Get handle to GUI cell that holds the LowLimit
                    pCell = (TableItem *) TableGuardBanding->item(iLine, TABLE_COLUMN_LL_NEW);
                    pCell->SetWhatIfFlags(GEX_DRILL_LL);
                    pFile->FormatTestLimit(ptTestCell,szString,lfLowL,ptTestCell->llm_scal, false);
                    pCell->setText(szString);
                    // Force to repaint this cell
                    pCell->paint();

                    // Update the Low & High limits in the Test structure...
                    RescaleValue(&lfLowL,ptTestCell->llm_scal, ptTestCell->szTestUnits, ptTestCell->szTestUnits);
                    pFile->FormatTestResultNoUnits(&lfLowL,-ptTestCell->llm_scal);

                    // Save real limits
                    lfSaveLowL = ptTestCell->GetCurrentLimitItem()->lfLowLimit;

                    // Overwrite them so to call stats computation (for new Cp, Cpk)
                    ptTestCell->GetCurrentLimitItem()->lfLowLimit  = lfLowL;
                    ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~(CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL);
                }

                if(bUpdatedHighLimits)
                {
                    // Get handle to GUI cell that holds the highLimit
                    pCell = (TableItem *) TableGuardBanding->item(iLine, TABLE_COLUMN_HL_NEW);
                    pCell->SetWhatIfFlags(GEX_DRILL_HL);
                    pFile->FormatTestLimit(ptTestCell,szString,lfHighL,ptTestCell->hlm_scal, false);
                    pCell->setText(szString);
                    // Force to repaint this cell
                    pCell->paint();

                    // Update the Low & High limits in the Test structure...
                    RescaleValue(&lfHighL,ptTestCell->hlm_scal,ptTestCell->szTestUnits, ptTestCell->szTestUnits);
                    pFile->FormatTestResultNoUnits(&lfHighL,-ptTestCell->hlm_scal);

                    // Save real limits
                    lfSaveHighL= ptTestCell->GetCurrentLimitItem()->lfHighLimit;
                    bSaveFlag  = ptTestCell->GetCurrentLimitItem()->bLimitFlag;

                    // Overwrite them so to call stats computation (for new Cp, Cpk)
                    ptTestCell->GetCurrentLimitItem()->lfHighLimit = lfHighL;
                    ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~(CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL);
                }

                if(bUpdatedLowLimits || bUpdatedHighLimits)
                {
                    // Update statistics using data samples(Cp, Cpk will be affected...
                    cStats.ComputeBasicTestStatistics(ptTestCell,true);

                    // Write new Cp, Cpk, new Yield into the table...
                    // Cp
                    pCell = (TableItem *) TableGuardBanding->item(iLine, TABLE_COLUMN_CP_NEW);
                    pCell->SetWhatIfFlags(GEX_DRILL_RESULT);
                    pCell->setText(gexReport->CreateResultStringCpCrCpk(ptTestCell->GetCurrentLimitItem()->lfCp));

                    // Cr
                    pCell = (TableItem *) TableGuardBanding->item(iLine, TABLE_COLUMN_CR_NEW);
                    pCell->SetWhatIfFlags(GEX_DRILL_RESULT);
                    if(ptTestCell->GetCurrentLimitItem()->lfCp != 0.)
                        pCell->setText(gexReport->CreateResultStringCpCrCpk(1/ptTestCell->GetCurrentLimitItem()->lfCp));
                    else
                       pCell->setText("n/a");

                    // Cpk
                    pCell = (TableItem *) TableGuardBanding->item(iLine, TABLE_COLUMN_CPK_NEW);
                    pCell->SetWhatIfFlags(GEX_DRILL_RESULT);
                    pCell->setText(gexReport->CreateResultStringCpCrCpk(ptTestCell->GetCurrentLimitItem()->lfCpk));

                    // Yield
                    if(ptTestCell->ldSamplesValidExecs)
                        fYield = 100.0f - (100.0f * ptTestCell->GetCurrentLimitItem()->ldSampleFails)/((float)(ptTestCell->ldSamplesValidExecs));
                    else
                        fYield = -1.0f;

                    pCell = (TableItem *) TableGuardBanding->item(iLine, TABLE_COLUMN_YIELD_NEW);
                    pCell->SetWhatIfFlags(GEX_DRILL_RESULT);
                    pCell->setText(gexReport->CreateResultStringPercent(fYield));

                    // Compute Delta yield
                    pCell = (TableItem *) TableGuardBanding->item(iLine, TABLE_COLUMN_YIELD_REF);	// Original Yield
                    strString = pCell->text();
                    sscanf(strString.toLatin1().constData(),"%lf",&fOrgYield);

                    pCell = (TableItem *) TableGuardBanding->item(iLine, TABLE_COLUMN_DELTA_YIELD);	// Pointer to Delta yield
                    pCell->SetWhatIfFlags(GEX_DRILL_RESULT);
                    if(fYield < 0)
                    {
                        // Update Delta yield
                        pCell->setText("99.99 %");
                    }
                    else
                    {
                        // Update Delta yield
                        strString.sprintf("%.2lf %%",fYield-fOrgYield);
                        pCell->setText(strString);
                    }

                    // Reload real Limits, Cp, Cpk.
                    ptTestCell->GetCurrentLimitItem()->lfLowLimit	= lfSaveLowL;
                    ptTestCell->GetCurrentLimitItem()->lfHighLimit	= lfSaveHighL;
                    ptTestCell->GetCurrentLimitItem()->bLimitFlag	= bSaveFlag;
                    ptTestCell->GetCurrentLimitItem()->lfCp		= lfSaveCp;
                    ptTestCell->GetCurrentLimitItem()->lfCpk		= lfSaveCpk;
                }
            }
        }
    }
    setUpdatesEnabled(true);
}

///////////////////////////////////////////////////////////
// If user entered a unit different from the scaling factor, must scale value
///////////////////////////////////////////////////////////
void GexDrillWhatIf::RescaleValue(double *lfLimit,int iPower, char *szScale, char * szTestUnit)
{
    double	fScale1,fScale2,fScale3;

    switch(iPower)
    {
        default:
        case 2:	// '%'
            fScale1= GS_POW(10.0,iPower);
            break;
        case 0:
            fScale1 = 1;
            break;
        case 253:	// for unsigned -3
        case -3:
            fScale1 = 1e-3;
            break;
        case 250:	// for unsigned -6
        case -6:
            fScale1 = 1e-6;
            break;
        case 247:	// for unsigned -9
        case -9:
            fScale1 = 1e-9;
            break;
        case 244:	// for unsigned -13
        case -12:
            fScale1 = 1e-12;
            break;
        case 3:
            fScale1 = 1e3;
            break;
        case 6:
            fScale1 = 1e6;
            break;
        case 9:
            fScale1 = 1e9;
            break;
        case 12:
            fScale1 = 1e12;
            break;
        case 15:
            fScale1 = 1e15;
            break;
    }

    switch(*szScale)
    {
        case 'K':	// for unsigned -3 'K'
            fScale2 =1e3;
            break;
        case 'M':	// for unsigned -6 'M'
            fScale2 =1e6;
            break;
        case 'G':	// for unsigned -9 'G'
            fScale2 =1e9;
            break;
        case 'T':	// for unsigned -13 'T'
            fScale2 =1e12;
            break;
        case 'm':		// 'm'
            fScale2 =1e-3;
            break;
        case 'u':		// 'u'
            fScale2 =1e-6;
            break;
        case 'n':		// 'n'
            fScale2 =1e-9;
            break;
        case 'p':	// 'p'
            fScale2 =1e-12;
            break;
        case 'f':	// 'f'
            fScale2 =1e-15;
            break;
        case 0:
        default:
            fScale2 = 1;
            break;
        case '%':		// '%'
            fScale2 = 1e-2;	// In all GEX application, '%' is never scaled...expect here!
            break;
    }

    switch(*szTestUnit)
    {
        case 'K':	// for unsigned -3 'K'
            fScale3 =1e-3;
            break;
        case 'M':	// for unsigned -6 'M'
            fScale3 =1e-6;
            break;
        case 'G':	// for unsigned -9 'G'
            fScale3 =1e-9;
            break;
        case 'T':	// for unsigned -13 'T'
            fScale3 =1e-12;
            break;
        case 'm':		// 'm'
            fScale3 =1e3;
            break;
        case 'u':		// 'u'
            fScale3 =1e6;
            break;
        case 'n':		// 'n'
            fScale3 =1e9;
            break;
        case 'p':	// 'p'
            fScale3 =1e12;
            break;
        case 'f':	// 'f'
            fScale3 =1e15;
            break;
        case 0:
        default:
            fScale3 = 1;
            break;
        case '%':		// '%'
            fScale3 = 1e2;	// In all GEX application, '%' is never scaled...expect here!
            break;
    }

    // Scale value accordingly.
    *lfLimit = (*lfLimit) * (fScale1*fScale2*fScale3);
}

///////////////////////////////////////////////////////////
// Save FULL table into clipboard, ready to paste to Spreadsheet file (Windows only)
///////////////////////////////////////////////////////////
void GexDrillWhatIf::OnExportExcelClipboard(void)
{
#ifdef _WIN32
    if(GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_EVALUATION)
    {
        // Evaluation mode, refuse to run this function!
        GS::Gex::Message::information(
            "", "This function is disabled in Evaluation mode\n\nContact " +
            QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }


    // If no report...quietly return
    if(gexReport == NULL)
        return;

    QString strCell;
    QString strMessage;
    long	lLine,lCol,lMaxCol;
    QTableWidget *table=TableGuardBanding;
    QString strBigLine;

    // Write as first data line the Query names.
    for(lCol=0;lCol<table->columnCount();lCol++)
    {
        // Write cell string (visible columns only)
        if(table->isColumnHidden(lCol) == false)
        {
            strBigLine += table->horizontalHeaderItem(lCol)->text();
            strBigLine += "\t";
        }
    }
    strBigLine += "\n";

    for(lLine=0;lLine<table->rowCount();lLine++)
    {
        // Ignore all leading empty cells on each line
        lMaxCol = table->columnCount();
        while(lMaxCol > 0)
        {
            strCell = (table->item(lLine,lMaxCol-1)->text()).trimmed();
            if(strCell.isEmpty() == false)
                break;	// Reached the last cell that is not blank.
            lMaxCol--;
        }


        // Write cell string.
        for(lCol=0;lCol<lMaxCol;lCol++)
        {
            // Only dump visible columns!
            if(table->isColumnHidden(lCol) == false)
            {
                // Write Data , Units
                strCell = table->item(lLine,lCol)->text();

                // Export to clipboard in XL format:
                strBigLine += strCell;
                strBigLine += "\t";
            }
        }
        strBigLine += "\n";
    }

    // Copy data to clipboard
    QClipboard *lClipBoard = QGuiApplication::clipboard();
    if (!lClipBoard)
    {
        GSLOG(SYSLOG_SEV_WARNING, "failed to retrieve application clipboard");
    }
    else
    {
        lClipBoard->setText(strBigLine,QClipboard::Clipboard );
    }


    strMessage = "Table saved to clipboard in Spreadsheet format,\nready to paste!\nLaunch Spreadsheet application now?";
    bool lOk;
    GS::Gex::Message::request("", strMessage, lOk);
    if (! lOk)
    {   
        return;
    }
#ifdef _WIN32
        // Launch spreadsheet editor
    ShellExecuteA(NULL,
           "open",
        ReportOptions.GetOption("preferences", "ssheet_editor").toString().toLatin1().constData(),	//ReportOptions.m_PrefMap["ssheet_editor"],
           NULL,
           NULL,
           SW_SHOWNORMAL);
#else
    system(ReportOptions.GetOption("preferences", "ssheet_editor").toString().toLatin1().constData()); //system(ReportOptions.m_PrefMap["ssheet_editor"].toLatin1().constData());
#endif

#endif
}

///////////////////////////////////////////////////////////
// Save selection into clipboard, ready to paste to Spreadsheet file (Windows only)
///////////////////////////////////////////////////////////
void GexDrillWhatIf::OnExportSelectionExcelClipboard(void)
{
#if (defined _WIN32 || __linux__)
    if(GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_EVALUATION)
    {
        // Evaluation mode, refuse to run this function!
        GS::Gex::Message::information(
            "", "This function is disabled in Evaluation mode\n\nContact " +
            QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }


    // If no report...quietly return
    if(gexReport == NULL)
        return;

    QString strCell;
    QString strMessage;
    long	lLine,lCol,lMaxCol;
    QTableWidget *table=TableGuardBanding;
    QString strBigLine;

    // Write as first data line the Query names.
    for(lCol=0;lCol<table->columnCount();lCol++)
    {
        // Write cell string (visible columns only)
        if(table-> isColumnHidden(lCol) == false)
        {
            strBigLine += table->horizontalHeaderItem(lCol)->text() ;
            strBigLine += "\t";
        }
    }
    strBigLine += "\n";

    bool	bValidLine;
    for(lLine=0;lLine<table->rowCount();lLine++)
    {
        // Ignore all leading empty cells on each line
        lMaxCol = table->columnCount();
        while(lMaxCol > 0)
        {
            strCell = (table->item(lLine,lMaxCol-1)->text()).trimmed();
            if(strCell.isEmpty() == false)
                break;	// Reached the last cell that is not blank.
            lMaxCol--;
        }


        // Write cell string.
        bValidLine = false;
        for(lCol=0;lCol<lMaxCol;lCol++)
        {
            // Only dump visible columns that are SELECTED!
            if( table->item(lLine, lCol)->isSelected() &&  (table->isColumnHidden(lCol) == false))
            {
                // Write Data , Units
                strCell = table->item(lLine,lCol)->text();

                // Export to clipboard in XL format:
                strBigLine += strCell;
                strBigLine += "\t";

                // Flag we've written fields on this line
                bValidLine = true;
            }
        }
        if(bValidLine)
            strBigLine += "\n";
    }

    // Copy data to clipboard
    QClipboard *lClipBoard = QGuiApplication::clipboard();
    if (!lClipBoard)
    {
        GSLOG(SYSLOG_SEV_WARNING, "failed to retrieve application clipboard");
    }
    else
    {
        lClipBoard->setText(strBigLine,QClipboard::Clipboard );
    }


    strMessage = "Data saved to clipboard in Spreadsheet format,\nready to paste!\nLaunch Spreadsheet application now?";
    bool lOk;
    GS::Gex::Message::request("", strMessage, lOk);
    if (! lOk)
    {   
        return;
    }
#ifdef _WIN32
        // Launch spreadsheet editor
    ShellExecuteA(NULL,
           "open",
        ReportOptions.GetOption("preferences", "ssheet_editor").toString().toLatin1().constData(),	//ReportOptions.m_PrefMap["ssheet_editor"],
           NULL,
           NULL,
           SW_SHOWNORMAL);
#else
  //system(ReportOptions.m_PrefMap["ssheet_editor"].toLatin1().constData());
  if (system(ReportOptions.GetOption("preferences", "ssheet_editor").
             toString().toLatin1().constData()) == -1) {
    //FIXME: send error
  }
#endif

#endif
}

///////////////////////////////////////////////////////////
// Contextual menu Table clicked
//////////////////////////////////////////////////////////////////////
void GexDrillWhatIf::OnContextualMenu(const QPoint& pos)
{
    QString		strSelection;
    QIcon	icSave;
    QIcon	icExcel;
    QIcon	icHistogram;
    QIcon	icTrend;
    QAction *	pActionStatsChartHistogram	= NULL;
    QAction *	pActionStatsChartTrend		= NULL;
    QMenu		menu(this);

    // Select the full line so selection is very visible!
    QTableWidgetItem* item = TableGuardBanding->itemAt(pos.x(), pos.y());
    int row = item->row();
    // Build menu.
    icHistogram.addPixmap(*pixAdvHisto);
    icTrend.addPixmap(*pixTrend);

    pActionStatsChartHistogram	= menu.addAction(icHistogram,"Histogram Chart");
    pActionStatsChartTrend		= menu.addAction(icTrend,"Trend Chart");

    menu.addSeparator();

    // Save Test list (test# of selected lines (in column 0).)
    for(row=0;row<TableGuardBanding->rowCount();row++)
    {
        if(TableGuardBanding->item(row, 0)->isSelected())
        {
            // Save test#
            strSelection = TableGuardBanding->item(row,0)->text();
            strSelection += ";";
        }

    }

    // Menu common to all tables
    icSave.addPixmap(*pixSave);
    icExcel.addPixmap(*pixCSVSpreadSheet);

    // Enable/disable some features...
    QAction *	pActionResetDefaultLimits		= NULL;

    switch(GS::LPPlugin::ProductInfo::getInstance()->getProductID())
    {
        case GS::LPPlugin::LicenseProvider::eLtxcOEM:   // OEM-Examinator for LTXC
        case GS::LPPlugin::LicenseProvider::eSzOEM:     // OEM-Examinator for Credence SZ
            return;

        default:

            menu.addAction("Reset ALL limits to default", this, SLOT(OnResetTable()));
            pActionResetDefaultLimits	= menu.addAction("Reset limits to default (selected lines only)");

#ifdef _WIN32
            menu.addSeparator();

            // Allow to save to clipboard in spreadsheet format
            QAction* pActionSaveToClipboard			= menu.addAction(icExcel,"Save FULL Table to clipboard (Spreadsheet format)", this, SLOT(OnExportExcelClipboard()));
            if (!pActionSaveToClipboard)
                GSLOG(SYSLOG_SEV_WARNING, "error : pActionSaveToClipboard NULL !");
            QAction* pActionSaveSelectionToClipboard = menu.addAction(icExcel,"Save selection to clipboard (Spreadsheet format)", this, SLOT(OnExportSelectionExcelClipboard()));
            if (!pActionSaveSelectionToClipboard)
                GSLOG(SYSLOG_SEV_WARNING, "error : pActionSaveSelectionToClipboard NULL !");
#endif

            // Allow to export to disk.

            menu.addAction(icSave,"Save Full Table to disk...", this, SLOT(OnSaveTableToDisk()));
        break;
    }

    menu.setMouseTracking(true);
    QAction * pActionResult = menu.exec(QCursor::pos());
    menu.setMouseTracking(false);

    if (pActionResult == NULL)
        return;

    // Reset default limits for selected lines
    if(pActionResult == pActionResetDefaultLimits)
        OnResetOriginalLimits(false);

    // 'Histogram Chart'
    if(pActionResult == pActionStatsChartHistogram)
    {
        // Force to detach the What-If window from the GUI
        ForceAttachWindow(false);

        // Build hyperlink argument string: drill_chart=adv_histo--data=<test#>
        QString strLink = "drill--drill_chart=adv_histo--data=";
        strLink += strSelection;
        pGexMainWindow->Wizard_DrillChart(strLink);
        return;
    }

    // 'Trend Chart'
    if(pActionResult == pActionStatsChartTrend)
    {
        // Force to detach the What-If window from the GUI
        ForceAttachWindow(false);

        // Build hyperlink argument string: drill_chart=adv_trend--data=<test#>
        QString strLink = "drill--drill_chart=adv_trend--data=";
        strLink += strSelection;
        pGexMainWindow->Wizard_DrillChart(strLink);
        return;
    }
}

///////////////////////////////////////////////////////////
// A cell has been edited...change its color!
///////////////////////////////////////////////////////////
void GexDrillWhatIf::OnCellChanged(int iRow,int iCol)
{
   TableGuardBanding->blockSignals(true);
  // Ensure we have a valid cell location
  if(iCol < TABLE_COLUMN_LL_NEW || iCol > TABLE_COLUMN_YIELD_NEW)
      return;

  // Flags buffer = Y lines of 5 columns (LowL,HighL,Cp,Cpk,Yield)
  TableItem *pCell;
  QString strString;
  unsigned char cFlag=0, cMask=0;
  pCell = (TableItem *) TableGuardBanding->item(iRow,iCol);
  strString = pCell->text();
  switch(iCol)
  {
      case TABLE_COLUMN_LL_NEW		:	// LowLimit cell edited...reset any non-limit flag
          cFlag =  GEX_DRILL_LL;
          cMask =  GEX_DRILL_LL | GEX_DRILL_HL;

          formatNewLimit(strString, iRow, GEX_DRILL_LL);
          break;

      case TABLE_COLUMN_HL_NEW		:	// HighLimit cell edited...reset any non-limit flag
          cFlag =  GEX_DRILL_HL;
          cMask =  GEX_DRILL_LL | GEX_DRILL_HL;

          formatNewLimit(strString, iRow, GEX_DRILL_HL);
          break;

      case TABLE_COLUMN_CP_NEW		:	// Cp cell edited...reset all other flags
          cFlag = cMask = GEX_DRILL_CP;
          break;

      case TABLE_COLUMN_CPK_NEW		:	// Cpk cell edited...reset all other flags
          cFlag = cMask = GEX_DRILL_CPK;
          break;

      case TABLE_COLUMN_YIELD_NEW	:	// Yield cell edited...reset all other flags
          cFlag = cMask = GEX_DRILL_YIELD;
          break;
  }

  pCell->SetWhatIfFlags(pCell->WhatIfFlags() & cMask);
  pCell->SetWhatIfFlags(pCell->WhatIfFlags() | cFlag);

  // Check all selections...in a same column...and force them to the same value.
  for(int iLine=0;iLine<TableGuardBanding->rowCount();iLine++)
  {
      if((TableGuardBanding->item(iLine,iCol)->isSelected() == true) || (iLine == iRow))
      {
        pCell = (TableItem *) TableGuardBanding->item(iLine,iCol);
          pCell->SetWhatIfFlags(pCell->WhatIfFlags() & cMask);
          pCell->SetWhatIfFlags(pCell->WhatIfFlags() | cFlag);
          // Repaint the whole line to reflect the color status change!
          for(int iColumn = TABLE_COLUMN_LL_NEW; iColumn <= TABLE_COLUMN_YIELD_NEW; iColumn++)
          {
              // For all other cells of the same test, clear flags using mask.
                pCell = (TableItem *) TableGuardBanding->item(iLine,iColumn);
                pCell->SetWhatIfFlags(pCell->WhatIfFlags() & cMask);

                // Set Flag for selected cell, reset flag for other cells.
                if(iColumn == iCol)
                {
                    pCell->SetWhatIfFlags(pCell->WhatIfFlags() | cFlag);
                    pCell->setText(strString);
                }

              // Force to repaint this cell
                pCell->paint();
          }

          // Reset Delta yield cell
        pCell = (TableItem *) TableGuardBanding->item(iLine, TABLE_COLUMN_DELTA_YIELD);
        pCell->SetWhatIfFlags(0);

        // Force to repaint this cell
        pCell->paint();
      }
  }

  // Force to repaint this cell
  pCell = (TableItem *) TableGuardBanding->item(iRow,iCol);
  pCell->setText(strString);
  pCell->paint();

  // Enable capture of signals in TableGuardBanding
  TableGuardBanding->blockSignals(false);
}

///////////////////////////////////////////////////////////
// Reset table to its default values.
///////////////////////////////////////////////////////////
void GexDrillWhatIf::OnResetTable(void)
{
    TableItem *pCell;
    int		iLine,iCol;

    // If no table created, there is nothing to reset!
    if(TableGuardBanding == NULL)
        return;

    // Request confirmation to clear first!
    bool lOk;
    GS::Gex::Message::request("", "Confirm to reload default Test Limits,statistics\nand lose all changes ?", lOk);
    if (! lOk)
    {
        return;
    }

    // Hourglass cursor...
    setCursor(Qt::WaitCursor);

    // Clear ALL cells as we will reload them with default values.
    for(iLine=0;iLine<TableGuardBanding->rowCount();iLine++)
    {
        for(iCol = TABLE_COLUMN_LL_NEW; iCol < TABLE_COLUMN_DELTA_YIELD; iCol++)
        {
            // If table is empty, we won't find any cell!
            pCell = (TableItem *) TableGuardBanding->item(iLine,iCol);
            if(pCell == NULL)
                return;	// empty table, nothing to do!

            pCell->SetWhatIfFlags(0);
        }

        // Reset Delta Yield to 0
        pCell = (TableItem *) TableGuardBanding->item(iLine, TABLE_COLUMN_DELTA_YIELD);	// Pointer to Delta yield
        pCell->SetWhatIfFlags(0);
        pCell->setText("0.00 %");
    }

    // Set flag so ALL cells will be reloaded from Data files analysis..
    bResetAllTable = true;

    // Have the files & report scanned over.
    pGexMainWindow->BuildReportSetDrill();

    // Reset originals limits
    OnResetOriginalLimits(true);

    setCursor(Qt::ArrowCursor);

    // Enable capture of signals in TableGuardBanding
    TableGuardBanding->blockSignals(false);
}

void GexDrillWhatIf::ComputeRefValues(int line , CGexFileInGroup  *file)
{
    //-- Get test number
    QString strLabel = static_cast<TableItem *>(TableGuardBanding->item(line, TABLE_COLUMN_TEST))->text();

    int	lTestNumber = 0,lPinmapIndex = 0;
    if(sscanf(strLabel.toLatin1().constData(),"%d%*c%d",&lTestNumber,&lPinmapIndex) < 2)
        lPinmapIndex = GEX_PTEST;

    //-- Get test name
    QString strName = static_cast<TableItem *>(TableGuardBanding->item(line, TABLE_COLUMN_NAME))->text();

     //-- Check if can find given test#.Pinmap#
    double  fYield = -1.0f;
    CTest			 *ptTestCell = 0;
    if(file->FindTestCell(lTestNumber,lPinmapIndex,&ptTestCell,false,false, strName.toLatin1().data())==1)
    {
        if(ptTestCell->ldSamplesValidExecs)
            fYield = 100.0f - (100.0f * ptTestCell->GetCurrentLimitItem()->ldSampleFails)/((float)(ptTestCell->ldSamplesValidExecs));

        // -- Yied ref
        static_cast<TableItem *>(TableGuardBanding->item(line, TABLE_COLUMN_YIELD_REF))->setText(gexReport->CreateResultStringPercent(fYield));

        //-- Cp ref
        static_cast<TableItem *>(TableGuardBanding->item(line, TABLE_COLUMN_CP_REF))->setText(gexReport->CreateResultStringCpCrCpk(ptTestCell->GetCurrentLimitItem()->lfCp));

        //-- Cpk ref
        static_cast<TableItem *>(TableGuardBanding->item(line, TABLE_COLUMN_CPK_REF))->setText(gexReport->CreateResultStringCpCrCpk(ptTestCell->GetCurrentLimitItem()->lfCpk));

        //-- Mean
        static_cast<TableItem *>(TableGuardBanding->item(line, TABLE_COLUMN_MEAN))->setText(file->FormatTestResult(ptTestCell,ptTestCell->lfMean,ptTestCell->res_scal));

        //-- Sigma
        static_cast<TableItem *>(TableGuardBanding->item(line, TABLE_COLUMN_SIGMA))->setText(file->FormatTestResult(ptTestCell,ptTestCell->lfSigma,ptTestCell->res_scal));

        static_cast<TableItem *>(TableGuardBanding->item(line, TABLE_COLUMN_YIELD_REF))->paint();
        static_cast<TableItem *>(TableGuardBanding->item(line, TABLE_COLUMN_CP_REF))->paint();
        static_cast<TableItem *>(TableGuardBanding->item(line, TABLE_COLUMN_CPK_REF))->paint();
        static_cast<TableItem *>(TableGuardBanding->item(line, TABLE_COLUMN_MEAN))->paint();
        static_cast<TableItem *>(TableGuardBanding->item(line, TABLE_COLUMN_SIGMA))->paint();
    };
}

void GexDrillWhatIf::normalizeValueAccordingToUnitPrefix(QString& aLocalValue, const QString& aLocalUnit)
{
    int lScalingFactor = 0;
    gsfloat64 lNormValue = (gsfloat64) aLocalValue.toDouble();
    /*lLocalUnit = */GS::Core::NormalizeUnit(aLocalUnit, lScalingFactor/*, STDFUnitPrefixToScale*/);
    if(aLocalUnit == "%" && lScalingFactor == 0)
        lScalingFactor = 2;//STDFUnitPrefixToScale submethod doesn't manage properly '%' units
    lNormValue = GS::Core::NormalizeValue(lNormValue, lScalingFactor);
    aLocalValue = QString::number(lNormValue);
}

///////////////////////////////////////////////////////////
// 'Compute' button pressed
///////////////////////////////////////////////////////////
void GexDrillWhatIf::OnComputeWhatIf(void)
{
    TableItem *pCell;
    QString strLabel;
    QString strName;
    QString strLimit;
    char	szScale[5];
    int		iLine,iCol;
    bool	bEdited;
    CGexGroupOfFiles *pGroup;
    CGexFileInGroup  *pFile;
    CTest			 *ptTestCell;	// Pointer to test cell
    int		lTestNumber,lPinmapIndex;
    double	lfValue,lfLimit;

    // Enable capture of signals in TableGuardBanding
    TableGuardBanding->blockSignals(false);
    // Have the files & report scanned over.
    pGexMainWindow->BuildReportSetDrill();

    if(gexReport == NULL)
        return;	// Just in case

    // First: write info about reference group#1
    pGroup = gexReport->getGroupsList().isEmpty()?NULL:gexReport->getGroupsList().first();	// Group#1
    if(pGroup == NULL)
        return;	// Just in case

    // First file in Group#1...must always exist.
    pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
    if(pFile == NULL)
        return;	// Just in case

    // Will have ALL reports show that new limits & new stats values.
    pGexMainWindow->iHtmlSectionsToSkip = 0;

    // Check all selections...in a same column...and force them to the same value.
    bEdited=false;
    long	lTotalEdits=0;	// Keeps track of total test lines edited.

   bool bStopOnFail	= (comboBoxStopOnFail->currentIndex() == 0) ? true: false;
   int  nComputeOn		= comboBoxComputeOn->itemData(comboBoxComputeOn->currentIndex()).toInt();
   gexReport->VirtualRetest(bStopOnFail, false, (CGexReport::virtualRetestParts) nComputeOn);

    for(iLine=0;iLine<TableGuardBanding->rowCount();iLine++)
    {
        for(iCol = TABLE_COLUMN_LL_NEW; iCol <= TABLE_COLUMN_YIELD_NEW;iCol++)
        {

            // If table is empty, we may not find any table item
            pCell = (TableItem *) TableGuardBanding->item(iLine,iCol);
            if(pCell == NULL)
                return;	// Table is empty!
            ComputeRefValues(iLine, pFile);
            switch(pCell->WhatIfFlags())
            {
                case GEX_DRILL_LL:
                case GEX_DRILL_HL:
                case (GEX_DRILL_LL | GEX_DRILL_HL):
                case GEX_DRILL_CP:
                case GEX_DRILL_CPK:
                case GEX_DRILL_YIELD:
                    bEdited = true;
                    break;
            }
        }

        // If test line has cells edited: update Test limits!
        if((bEdited == true) && (bResetAllTable==false))
        {
            for (int nGroup = 0; nGroup < gexReport->getGroupsList().count(); nGroup++)
            {
                pGroup	= gexReport->getGroupsList().at(nGroup);

                if(pGroup == NULL)
                    continue;	// Just in case

                pFile	= (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

                if(pFile == NULL)
                    continue;	// Just in case

                // Get test number
                pCell = (TableItem *) TableGuardBanding->item(iLine, TABLE_COLUMN_TEST);
                strLabel = pCell->text();

                if(sscanf(strLabel.toLatin1().constData(),"%d%*c%d",&lTestNumber,&lPinmapIndex) < 2)
                    lPinmapIndex = GEX_PTEST;

                // Get test name
                pCell = (TableItem *) TableGuardBanding->item(iLine, TABLE_COLUMN_NAME);
                strName = pCell->text();

                // Check if can find given test#.Pinmap#
                if(pFile->FindTestCell(lTestNumber,lPinmapIndex,&ptTestCell,false,false, strName.toLatin1().data())!=1)
                    continue;

                // Clear Whatif limitrs flags
                ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag = CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL;

                // Keep track of total lines edited
                lTotalEdits++;

                // Format name to be Script-string compliant
                strName = strName.replace("'","\\'");	// Replace ' with \' as it is the srring delimiter!

                // Get LowLimit...and check if it is a custom one.
                pCell = (TableItem *) TableGuardBanding->item(iLine, TABLE_COLUMN_LL_NEW);
                if(pCell->WhatIfFlags() & GEX_DRILL_LL)
                {
                    // It's a custom LowLimit...write it into Script file
                    strLimit= pCell->text();
                    *szScale=0;
                    // Scale limit to be in STDF limit scaling format
                    if(sscanf(strLimit.toLatin1().constData(),"%lf %1s",&lfLimit,szScale) > 0)
                    {
                        // If user entered a unit different from the scaling factor, must scale value
                        RescaleValue(&lfLimit,ptTestCell->llm_scal, szScale, ptTestCell->szTestUnits);
                        pFile->FormatTestResultNoUnits(&lfLimit,-ptTestCell->llm_scal);

                        // Flag what-if limits
                        ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag	&= ~CTEST_LIMITFLG_NOLTL;
                        ptTestCell->GetCurrentLimitItem()->bLimitFlag			&= ~CTEST_LIMITFLG_NOLTL;

                        // New limit
                        ptTestCell->GetCurrentLimitItem()->lfLowLimit = lfLimit;

                        if(ptTestCell->GetCurrentLimitItem()->lfLowLimit == -C_INFINITE)
                            ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOLTL;	// Disable Low limit
                    }
                }

                // Get HighLimit...and check if it is a custom one.
                pCell = (TableItem *) TableGuardBanding->item(iLine, TABLE_COLUMN_HL_NEW);
                if(pCell->WhatIfFlags() & GEX_DRILL_HL)
                {
                    // It's a custom HighLimit...write it into Script file
                    strLimit= pCell->text();
                    *szScale=0;
                    // Scale limit to be in STDF limit scaling format
                    if(sscanf(strLimit.toLatin1().constData(),"%lf %1s",&lfLimit,szScale) > 0)
                    {
                        // If user entered a unit different from the scaling factor, must scale value
                        RescaleValue(&lfLimit, ptTestCell->hlm_scal, szScale, ptTestCell->szTestUnits);
                        pFile->FormatTestResultNoUnits(&lfLimit,-ptTestCell->hlm_scal);

                        // Flag what-if limits
                        ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag	&= ~CTEST_LIMITFLG_NOHTL;
                        ptTestCell->GetCurrentLimitItem()->bLimitFlag			&= ~CTEST_LIMITFLG_NOHTL;

                        // Save limit
                        ptTestCell->GetCurrentLimitItem()->lfHighLimit = lfLimit;

                        if(ptTestCell->GetCurrentLimitItem()->lfHighLimit == C_INFINITE)
                            ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOHTL;	// Disable Low limit
                    }
                }

                // If custom Cp
                pCell = (TableItem *) TableGuardBanding->item(iLine, TABLE_COLUMN_CP_NEW);
                if(pCell->WhatIfFlags() & GEX_DRILL_CP)
                {
                    // Compute test limits to match specified Cp
                    strLimit= pCell->text();
                    // Extract Cp value
                    if(sscanf(strLimit.toLatin1().constData(),"%lf",&lfValue) > 0)
                    {
                        // Compute :6*Sigma*Cp (this is equal to the limit space to define)
                        lfValue *= 6*ptTestCell->lfSigma;

                        // Flag what-if limits
                        ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag &= ~(CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL);

                        // Compute LL
                        ptTestCell->GetCurrentLimitItem()->lfLowLimit = ptTestCell->lfMean - (lfValue/2);

                        // Compute HL
                        ptTestCell->GetCurrentLimitItem()->lfHighLimit = ptTestCell->lfMean + (lfValue/2);

                    }
                }

                // If custom Cpk
                pCell = (TableItem *) TableGuardBanding->item(iLine, TABLE_COLUMN_CPK_NEW);
                if(pCell->WhatIfFlags() & GEX_DRILL_CPK)
                {
                    // Compute test limits to match specified Cpk
                    strLimit= pCell->text();
                    // Extract Cpk value
                    if(sscanf(strLimit.toLatin1().constData(),"%lf",&lfValue) > 0)
                    {
                        // Compute :3*Sigma*Cpk (this is equal to the half limit space centered on mean)
                        lfValue *= 3*ptTestCell->lfSigma;

                        // Flag what-if limits
                        ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag &= ~(CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL);

                        // Compute LL
                        ptTestCell->GetCurrentLimitItem()->lfLowLimit = ptTestCell->lfMean - lfValue;

                        // Compute HL
                        ptTestCell->GetCurrentLimitItem()->lfHighLimit = ptTestCell->lfMean + lfValue;
                    }
                }
            }
        }
    // Loop on each line in table.
    bEdited = false;
    }

    // Case 6575:
    // update data even if no edits, so to make sure we take into
    // account the failure mode ('Stop on Fail'/'Continue on fail') selected
    // in the What-If GUI.

    //if(lTotalEdits) // IF edits, ensure we focus on data samples!
    if(1)
    {
        bool bUpdateReport = comboBoxOutputFormat->isEnabled();

        // Get Report output format requested in what-if page, force it in settings page
        if(bUpdateReport)
        {
            // Define report format
            QStringList sl; sl << "HTML" <<"CSV"<< "DOC" << "PDF"<< "PPT"<< "ODT"<< "INTERACTIVE" ;
            ReportOptions.SetOption("output", "format", sl.value(comboBoxOutputFormat->currentIndex())); //ReportOptions.iOutputFormat = 1 << comboBoxOutputFormat->currentIndex();
        }

        // Detach What-if page
        ForceAttachWindow(false);

        // Get flags for VirtualRetest
        bool bStopOnFail	= (comboBoxStopOnFail->currentIndex() == 0) ? true: false;
        int  nComputeOn		= comboBoxComputeOn->itemData(comboBoxComputeOn->currentIndex()).toInt();

        // Ensure to overload Binning computation
        bool bOrgBinningUseWafermapOnly=false, bOrgBinningUseSamplesOnly=false;

        {
            QString strOptionStorageDevice = (ReportOptions.GetOption("binning","computation")).toString();

            if(strOptionStorageDevice == "samples")
            {
                bOrgBinningUseSamplesOnly = true;
                bOrgBinningUseWafermapOnly = false;
            }
            else if (strOptionStorageDevice == "wafer_map")
            {
                bOrgBinningUseSamplesOnly = false;
                bOrgBinningUseWafermapOnly = true;
            }
            else if (strOptionStorageDevice == "summary")
            {
                bOrgBinningUseSamplesOnly = false;
                bOrgBinningUseWafermapOnly = false;
            }
            else
                GEX_ASSERT(false);

            ReportOptions.SetOption("binning","computation","wafer_map");
        }

        // Rebuild PRR data (Pass/Fail) based on  test limits
        gexReport->VirtualRetest(bStopOnFail,bUpdateReport, (CGexReport::virtualRetestParts) nComputeOn);

        // Recreate report (unless Interactive mode selected!
        QString of=ReportOptions.GetOption("output", "format").toString();
        if(bUpdateReport && (of!="INTERACTIVE") ) //(ReportOptions.iOutputFormat != GEX_OPTION_OUTPUT_INTERACTIVEONLY))
            gexReport->RebuildReport();

        // Ensure What-If table shows updated statistics
        OnUpdateTableColors();

        // Display report page in case the what-if page is detached.
        if(bUpdateReport)
            pGexMainWindow->ReportLink();

        // Reset Binning computation to its original stage
        if( (bOrgBinningUseSamplesOnly) && (!bOrgBinningUseWafermapOnly) )
            ReportOptions.SetOption("binning","computation","samples");
        else if( (!bOrgBinningUseSamplesOnly) && (bOrgBinningUseWafermapOnly) )
            ReportOptions.SetOption("binning","computation","wafer_map");
        else if( (!bOrgBinningUseSamplesOnly) && (!bOrgBinningUseWafermapOnly) )
            ReportOptions.SetOption("binning","computation","summary");
        else
        {
            GEX_ASSERT(false);
            ReportOptions.SetOption("binning","computation","summary");
        }
    }
    TableGuardBanding->setFocus(Qt::ActiveWindowFocusReason);
    // Enable capture of signals in TableGuardBanding
    TableGuardBanding->blockSignals(false);
}

///////////////////////////////////////////////////////////
// Compute new table info...and update content+ colors.
///////////////////////////////////////////////////////////
void GexDrillWhatIf::OnUpdateTableColors(void)
{
    int			iLine,iCol;
    TableItem	*pCell;
    bool		lEdited;//, lReset;

    QString		strNewYield,strRefYield;
    float		fNewYield,fRefYield;
    CGexGroupOfFiles *pGroup;
    CGexFileInGroup  *pFile;
    CTest			 *ptTestCell;	// Pointer to test cell
    double		fYield;
    int			lTestNumber;
    int			lPinmapIndex;

    if(gexReport == NULL)
        return;	// Just in case

    // First: write info about reference group#1
    pGroup = gexReport->getGroupsList().isEmpty()?NULL:gexReport->getGroupsList().first();	// Group#1
    if(pGroup == NULL)
        return;	// Just in case

    // First file in Group#1...must always exist.
    pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
    if(pFile == NULL)
        return;	// Just in case

  // Check all selections...in a same column...and force them to the same value.
  for(iLine=0;iLine<TableGuardBanding->rowCount();iLine++)
  {
    lEdited = false;
    //lReset = false;
    for(iCol = TABLE_COLUMN_LL_NEW; iCol <= TABLE_COLUMN_YIELD_NEW; iCol++)
    {
        // Cell only exist if table is not empty...
        pCell = (TableItem *) TableGuardBanding->item(iLine,iCol);
        if(pCell == NULL)
            return;	// Table is probably empty!

        switch(pCell->WhatIfFlags())
        {
            case GEX_DRILL_LL:
            case GEX_DRILL_HL:
            case (GEX_DRILL_LL | GEX_DRILL_HL):
            case GEX_DRILL_CP:
            case GEX_DRILL_CPK:
            case GEX_DRILL_YIELD:
                lEdited = true;
                break;
        }
//        if ((pCell->WhatIfFlags() == 0) && (pCell->WhatIfPreviousFlags() != 0))
//            lReset = true;
    }

    //if((bEdited == true) || (bResetAllTable == true))
    {
        // Get pointer to Test cell
        pCell = (TableItem *) TableGuardBanding->item(iLine, TABLE_COLUMN_TEST);
        QString strString = pCell->text();
        char	szLimit[GEX_LIMIT_LABEL];
        if(sscanf(strString.toLatin1().constData(),"%d%*c%d",&lTestNumber,&lPinmapIndex) < 2)
            lPinmapIndex = GEX_PTEST;

        pCell = (TableItem *) TableGuardBanding->item(iLine, TABLE_COLUMN_NAME);
        QString strName = pCell->text();

        // Check if can find given test#.Pinmap#
        if(pFile->FindTestCell(lTestNumber,lPinmapIndex,&ptTestCell,false,false, strName.toLatin1().data())==1)
        {
            for(int iCol = TABLE_COLUMN_LL_NEW; iCol < TABLE_COLUMNS; iCol++)
            {
                pCell = (TableItem *) TableGuardBanding->item(iLine,iCol);

                // For edited test, ensure we will highlight results
                if(lEdited && (pCell->WhatIfFlags() == 0) && (bResetAllTable == false))
                {
                    // Mark this field as custom result!
                    pCell->SetWhatIfFlags(GEX_DRILL_RESULT);
                }
//                else if (lReset)
//                {
//                    // Mark this field as reset result
//                    pCell->SetWhatIfFlags(0);
//                }
                // Update table cell string...
                switch(iCol)
                {
                    case TABLE_COLUMN_LL_NEW		: // LowLimit
                        if(lEdited && (pCell->WhatIfFlags() & GEX_DRILL_LL) == 0)
                        {
                            // If low limit exists
                            if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
                                pFile->FormatTestLimit(ptTestCell, szLimit, ptTestCell->GetCurrentLimitItem()->lfLowLimit, ptTestCell->llm_scal, false);
                            else
                                strcpy(szLimit, "n/a");
                            pCell->setText(szLimit);
                        }
                        break;
                    case TABLE_COLUMN_HL_NEW		: // HighLimit
                        if(lEdited && (pCell->WhatIfFlags() & GEX_DRILL_HL) == 0)
                        {
                            // If high limit exists
                            if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
                                pFile->FormatTestLimit(ptTestCell, szLimit, ptTestCell->GetCurrentLimitItem()->lfHighLimit, ptTestCell->hlm_scal, false);
                            else
                                strcpy(szLimit, "n/a");

                            pCell->setText(szLimit);
                        }
                        break;
                    case TABLE_COLUMN_CP_NEW		: // Cp
                        if((pCell->WhatIfFlags() & GEX_DRILL_CP) == 0)
                        {
                            pCell->setText(gexReport->CreateResultStringCpCrCpk(ptTestCell->GetCurrentLimitItem()->lfCp));
                        }
                        break;
                    case TABLE_COLUMN_CR_NEW		: // Cr
                        if((pCell->WhatIfFlags() & GEX_DRILL_CP) == 0)
                        {
                            if(ptTestCell->GetCurrentLimitItem()->lfCp != 0)
                                pCell->setText(gexReport->CreateResultStringCpCrCpk(1./ptTestCell->GetCurrentLimitItem()->lfCp));
                            else
                                pCell->setText("n/a");
                        }
                        break;
                    case TABLE_COLUMN_CPK_NEW		: // Cpk
                        if((pCell->WhatIfFlags() & GEX_DRILL_CPK) == 0)
                            pCell->setText(gexReport->CreateResultStringCpCrCpk(ptTestCell->GetCurrentLimitItem()->lfCpk));
                        break;
                    case TABLE_COLUMN_YIELD_NEW		: // Yield
                        if(ptTestCell->ldSamplesValidExecs)
                            fYield = 100.0f - (100.0f * ptTestCell->GetCurrentLimitItem()->ldSampleFails)/((float)(ptTestCell->ldSamplesValidExecs));
                        else
                            fYield = -1.0f;
                        if((pCell->WhatIfFlags() & GEX_DRILL_YIELD) == 0)
                            pCell->setText(gexReport->CreateResultStringPercent(fYield));
                        break;
                    case TABLE_COLUMN_DELTA_YIELD	:
                        // Update Delta yield.
                        pCell = (TableItem *) TableGuardBanding->item(iLine, TABLE_COLUMN_YIELD_NEW);		// New Yield
                        strNewYield = pCell->text();
                        pCell = (TableItem *) TableGuardBanding->item(iLine, TABLE_COLUMN_YIELD_REF);	// Reference Yield
                        strRefYield = pCell->text();


                        if(strRefYield.contains("n/a") || strNewYield.contains("n/a"))
                        {
                            // Update Delta yield
                            pCell = (TableItem *) TableGuardBanding->item(iLine,iCol);	// Delta yield
                            pCell->setText("n/a");
                        }
                        else
                        {
                            // Compute Delta yield.
                            sscanf(strRefYield.toLatin1().constData(),"%f",&fRefYield);
                            sscanf(strNewYield.toLatin1().constData(),"%f",&fNewYield);

                            // Update Delta yield
                            pCell = (TableItem *) TableGuardBanding->item(iLine,iCol);	// Delta yield
                            strNewYield.sprintf("%.2lf %%",fNewYield-fRefYield);
                            pCell->setText(strNewYield);
                        }
                        if(lEdited)
                            pCell->SetWhatIfFlags(GEX_DRILL_RESULT);	// Will make delta-yield highlighted.

                        break;
                }

                // Force to repaint this cell
                pCell->paint();
            }
        }
    }
  }

  // Reset flag.
  bResetAllTable = false;
}

///////////////////////////////////////////////////////////
// Load the Test limits cells from What-If CSV file
///////////////////////////////////////////////////////////
void	GexDrillWhatIf::OnLoadTableFromDisk(void)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_EVALUATION)
    {
        // Evaluation mode, refuse to run this function!
        GS::Gex::Message::information(
            "", "This function is disabled in Evaluation mode\n\nContact " +
            QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }

    // Get file to load
    QString strFile = QFileDialog::getOpenFileName(this,
          "Load Test limits from...", "", "Spreadsheet CSV(*.csv)");

    // If no file selected, ignore command.
    if(strFile.isEmpty())
        return;

    OnLoadTableFromDisk(strFile);
}

///////////////////////////////////////////////////////////
// Load the Test limits cells from What-If CSV file
///////////////////////////////////////////////////////////
void	GexDrillWhatIf::OnLoadTableFromDisk(QString strFile)
{
    // Open CSV Parameter table file
    QFile f( strFile );
    if(!f.open( QIODevice::ReadOnly ))
        return;

    // Assign file I/O stream
    QTextStream				hCsvTableFile(&f);
    LimitFileVersion		eLimitFileVersion	= versionUnknown;

    // First line must be the What-if header line
    QString strString;
    strString = hCsvTableFile.readLine();

    // Check if whatif limit file has new format
    if (strString.contains(WHAT_IF_CSV_HEADER))
        eLimitFileVersion = versionV5;
    else if (strString.contains(WHAT_IF_CSV_HEADER_V6))
        eLimitFileVersion = versionV6;
    else if (strString.contains(WHAT_IF_CSV_HEADER_V61))
        eLimitFileVersion = versionV61;

    // Skip all the header lines
    do
    {
        strString = hCsvTableFile.readLine();
    }
    while((strString.startsWith(TABLE_HEADER_TEST,  Qt::CaseInsensitive) == false) && (hCsvTableFile.atEnd() == false));

    // If we reached the end of file, then do nothing
    if (hCsvTableFile.atEnd() == false)
    {
        switch (eLimitFileVersion)
        {
            case versionUnknown	:
            default				:
                GS::Gex::Message::information(
                    "",
                    "This file doesn't comply with the 'What-If' CSV format.\n"
                    "To create a valid template, simply save your current "
                    "'What-if' table\nin .CSV format, "
                    "then edit the test limits of your choice.");
                                    break;

            case versionV6		:
            case versionV61		:	loadLimitFile(hCsvTableFile, eLimitFileVersion);
                                    break;

            case versionV5		:	loadCompatibleLimitFile(hCsvTableFile);
                                    break;
        }
    }
}

///////////////////////////////////////////////////////////
// Load limit file V6 format
///////////////////////////////////////////////////////////
void	GexDrillWhatIf::loadLimitFile(QTextStream& txtStream, LimitFileVersion eLimitFileVersion)
{
    // Read all tests, extract their limits
    QString		strLine;
    QStringList strCells;
    long		lLine;

    // Fields from table
    QString		strLineTest;
    QString		strLineName;

    // Fields read from file
    QString		strTestName;
    QString		strTestNumber;
    QString		strLowL;
    QString		strHighL;
    QString		strCP;
    QString		strCPk;
    int			nStatusLL	= 0;
    int			nStatusHL	= 0;
    int			nStatusCP	= 0;
    int			nStatusCPk	= 0;

    TableItem * pCell = NULL;

    do
    {
        // Read line
        strLine = txtStream.readLine();

        // Split line in cells
        strCells = strLine.split(",",QString::KeepEmptyParts);

        // Check if line holds enough cells
        if(strCells.count() < 24)
            return;

        strTestNumber	= strCells[0].trimmed();		// Test number cell#0
        strTestName		= strCells[1].trimmed();		// Test name cell#1
        strTestName.replace(",",";");					// Replace "," with ";" to avoid .CSV conflicting data.

        if (eLimitFileVersion == versionV6)
        {
            strLowL			= strCells[2] + " " + strCells[3];	// Low  Limit + units (cell#2 & cell 3)
            strLowL			= strLowL.simplified();
            strHighL		= strCells[5] + " " + strCells[6];	// High Limit + units (cell#4 & cell 5)
            strHighL		= strHighL.simplified();
            strCP			= strCells[8].trimmed();
            strCPk			= strCells[10].trimmed();

            nStatusLL		= strCells[4].toInt();
            nStatusHL		= strCells[7].toInt();
            nStatusCP		= strCells[9].toInt();
            nStatusCPk		= strCells[11].toInt();
        }
        else if (eLimitFileVersion == versionV61)
        {
            strLowL			= strCells[3] + " " + strCells[4];	// Low  Limit + units (cell#3 & cell 4)
            strLowL			= strLowL.simplified();
            strHighL		= strCells[6] + " " + strCells[7];	// High Limit + units (cell#5 & cell 6)
            strHighL		= strHighL.simplified();
            strCP			= strCells[9].trimmed();
            strCPk			= strCells[11].trimmed();

            nStatusLL		= strCells[5].toInt();
            nStatusHL		= strCells[8].toInt();
            nStatusCP		= strCells[10].toInt();
            nStatusCPk		= strCells[12].toInt();
        }
        else
            GSLOG(SYSLOG_SEV_WARNING, "GexDrillWhatIf::loadLimitFile... Bad file version");

        // Find matching table cell that will hold this new set of test limits
        for(lLine=0; lLine < TableGuardBanding->rowCount(); lLine++)
        {
            // See if we have a matching test with different limits...
            strLineTest		= TableGuardBanding->item(lLine, TABLE_COLUMN_TEST)->text().trimmed();
            strLineName		= TableGuardBanding->item(lLine, TABLE_COLUMN_NAME)->text().trimmed();
            strLineName.replace(",",";");						// Replace "," with ";" to avoid .CSV conflicting data.

            // If matching test with one or two different limits, then get them!
            if((strTestNumber == strLineTest) && (strTestName   == strLineName))
            {
                for(int nCol = TABLE_COLUMN_LL_NEW; nCol < TABLE_COLUMNS; nCol++)
                {
                    // If table is empty, we won't find any cell!
                    pCell = (TableItem *) TableGuardBanding->item(lLine, nCol);
                    if(pCell != NULL)
                        pCell->SetWhatIfFlags(0);
                    pCell->paint();

                }

                // Reset Delta Yield to 0
                pCell = (TableItem *) TableGuardBanding->item(lLine, TABLE_COLUMN_DELTA_YIELD);	// Pointer to Delta yield
                pCell->SetWhatIfFlags(0);
                pCell->setText("0.00 %");
                pCell->paint();

                // Cell found, overwrite its Test limits!
                if (nStatusLL == 1)
                {
                    TableGuardBanding->item(lLine, TABLE_COLUMN_LL_NEW)->setText(strLowL);
                    OnCellChanged(lLine, TABLE_COLUMN_LL_NEW);
                }
                else
                {
                    pCell = (TableItem *)(TableGuardBanding->item(lLine, TABLE_COLUMN_LL_NEW));
                    pCell->setText(TableGuardBanding->item(lLine, TABLE_COLUMN_LL_REF)->text().trimmed());
                    pCell->SetWhatIfFlags(0);

                    // Force to repaint this cell
                    pCell->paint();
                }


                if (nStatusHL == 1)
                {
                    // Cell found, overwrite its Test limits!
                    TableGuardBanding->item(lLine, TABLE_COLUMN_HL_NEW)->setText(strHighL);
                    OnCellChanged(lLine, TABLE_COLUMN_HL_NEW);
                }
                else
                {
                    pCell = (TableItem *)(TableGuardBanding->item(lLine, TABLE_COLUMN_HL_NEW));
                    pCell->setText(TableGuardBanding->item(lLine, TABLE_COLUMN_HL_REF)->text().trimmed());
                    pCell->SetWhatIfFlags(0);

                    // Force to repaint this cell
                    pCell->paint();
                }

                // Cell found, overwrite its Test limits!
                if (nStatusCP == 1)
                {
                    // Cell found, overwrite its Test limits!
                    TableGuardBanding->item(lLine, TABLE_COLUMN_CP_NEW)->setText(strCP);
                    OnCellChanged(lLine, TABLE_COLUMN_CP_NEW);
                }
                else
                {
                    pCell = (TableItem *)(TableGuardBanding->item(lLine, TABLE_COLUMN_CP_NEW));
                    pCell->setText(TableGuardBanding->item(lLine, TABLE_COLUMN_CP_REF)->text().trimmed());
                    pCell->SetWhatIfFlags(0);

                    // Force to repaint this cell
                    pCell->paint();

                    pCell = (TableItem *)(TableGuardBanding->item(lLine, TABLE_COLUMN_CR_NEW));
                    pCell->setText(TableGuardBanding->item(lLine, TABLE_COLUMN_CR_REF)->text().trimmed());
                    pCell->SetWhatIfFlags(0);

                    // Force to repaint this cell
                    pCell->paint();
                }

                // Cell found, overwrite its Test limits!
                if (nStatusCPk == 1)
                {
                    // Cell found, overwrite its Test limits!
                    TableGuardBanding->item(lLine, TABLE_COLUMN_CPK_NEW)->setText(strCPk);
                    OnCellChanged(lLine, TABLE_COLUMN_CPK_NEW);
                }
                else
                {
                    pCell = (TableItem *)(TableGuardBanding->item(lLine, TABLE_COLUMN_CPK_NEW));
                    pCell->setText(TableGuardBanding->item(lLine, TABLE_COLUMN_CPK_REF)->text().trimmed());
                    pCell->SetWhatIfFlags(0);

                    // Force to repaint this cell
                    pCell->paint();
                }

                // reload original yield
                pCell = (TableItem *)(TableGuardBanding->item(lLine, TABLE_COLUMN_YIELD_NEW));
                pCell->setText(TableGuardBanding->item(lLine, TABLE_COLUMN_YIELD_REF)->text().trimmed());
                pCell->SetWhatIfFlags(0);

                // Force to repaint this cell
                pCell->paint();

                break;	// Stop scanning table as we've found the entry.
            }
        }
    }
    while(txtStream.atEnd() == false);
}

///////////////////////////////////////////////////////////
// Load compatible limit file (older format)
///////////////////////////////////////////////////////////
void	GexDrillWhatIf::loadCompatibleLimitFile(QTextStream& txtStream)
{
    // Read all tests, extract their limits
    QString		strLine;
    QStringList strCells;
    long		lLine;
    QString		strTestName,strTestNumber,strTestPinmap;
    QString		strLowL,strHighL;
    QString		strCP, strCPk;

    // Fields read from file
    QString		strLineTest;
    QString		strLineName;
    QString		strLineLL;
    QString		strLineHL;
    QString		strLineCP;
    QString		strLineCPk;
    QString		strLineLLRef;
    QString		strLineHLRef;
    QString		strLineCPRef;
    QString		strLineCPkRef;

    TableItem *	pCell;

    do
    {
        // Read line
        strLine = txtStream.readLine();

        // Split line in cells
        strCells = strLine.split(",", QString::KeepEmptyParts);

        // Check if line holds enough cells
        if(strCells.count() < 20)
            return;

        strTestNumber	= strCells[0].trimmed();		// Test number cell#0
        strTestName		= strCells[1].trimmed();		// Test name cell#1
        strTestName.replace(",",";");						// Replace "," with ";" to avoid .CSV conflicting data.
        strLowL			= strCells[2] + " " + strCells[3];	// Low  Limit + units (cell#2 & cell 3)
        strLowL			= strLowL.simplified();
        strHighL		= strCells[4] + " " + strCells[5];	// High Limit + units (cell#4 & cell 5)
        strHighL		= strHighL.simplified();
        strCP			= strCells[6].trimmed();
        strCPk			= strCells[7].trimmed();

        // Find matching table cell that will hold this new set of test limits
        for(lLine=0;lLine<TableGuardBanding->rowCount();lLine++)
        {
            // See if we have a matching test with different limits...
            strLineTest		= TableGuardBanding->item(lLine, TABLE_COLUMN_TEST)->text().trimmed();
            strLineName		= TableGuardBanding->item(lLine, TABLE_COLUMN_NAME)->text().trimmed();
            strLineName.replace(",",";");						// Replace "," with ";" to avoid .CSV conflicting data.

            strLineLL		= TableGuardBanding->item(lLine, TABLE_COLUMN_LL_NEW)->text().trimmed();
            strLineHL		= TableGuardBanding->item(lLine, TABLE_COLUMN_HL_NEW)->text().trimmed();
            strLineCP		= TableGuardBanding->item(lLine, TABLE_COLUMN_CP_NEW)->text().trimmed();
            strLineCPk		= TableGuardBanding->item(lLine, TABLE_COLUMN_CPK_NEW)->text().trimmed();

            strLineLLRef	= TableGuardBanding->item(lLine, TABLE_COLUMN_LL_REF)->text().trimmed();
            strLineHLRef	= TableGuardBanding->item(lLine, TABLE_COLUMN_HL_REF)->text().trimmed();
            strLineCPRef	= TableGuardBanding->item(lLine, TABLE_COLUMN_CP_REF)->text().trimmed();
            strLineCPkRef	= TableGuardBanding->item(lLine, TABLE_COLUMN_CPK_REF)->text().trimmed();

            // If matching test with one or two different limits, then get them!
            if((strTestNumber == strLineTest) && (strTestName   == strLineName))
            {
                for(int nCol = TABLE_COLUMN_LL_NEW; nCol < TABLE_COLUMNS; nCol++)
                {
                    // If table is empty, we won't find any cell!
                    pCell = (TableItem *) TableGuardBanding->item(lLine, nCol);
                    if(pCell != NULL)
                        pCell->SetWhatIfFlags(0);

                    pCell->paint();
                }

                // Reset Delta Yield to 0
                pCell = (TableItem *) TableGuardBanding->item(lLine, TABLE_COLUMN_DELTA_YIELD);	// Pointer to Delta yield
                pCell->SetWhatIfFlags(0);
                pCell->setText("0.00 %");
                pCell->paint();

                // Cell found, overwrite its Test limits!
                if (strLineLL != strLowL)
                {
                    TableGuardBanding->item(lLine, TABLE_COLUMN_LL_NEW)->setText(strLowL);

                    // New low limit different from ref low limit, highlight it.
                    if (strLineLLRef != strLowL)
                        OnCellChanged(lLine, TABLE_COLUMN_LL_NEW);
                    else
                    {
                        pCell = (TableItem *)(TableGuardBanding->item(lLine, TABLE_COLUMN_LL_NEW));
                        pCell->SetWhatIfFlags(0);

                        // Force to repaint this cell
                        pCell->paint();
                    }
                }

                if (strLineHL != strHighL)
                {
                    // Cell found, overwrite its Test limits!
                    TableGuardBanding->item(lLine, TABLE_COLUMN_HL_NEW)->setText(strHighL);

                    if (strLineHLRef != strHighL)
                        OnCellChanged(lLine, TABLE_COLUMN_HL_NEW);
                    else
                    {
                        pCell = (TableItem *)(TableGuardBanding->item(lLine, TABLE_COLUMN_HL_NEW));
                        pCell->SetWhatIfFlags(0);

                        // Force to repaint this cell
                        pCell->paint();
                    }
                }

                break;	// Stop scanning table as we've found the entry.
            }
        }
    }
    while(txtStream.atEnd() == false);
}


///////////////////////////////////////////////////////////
// Reset some tests to original limits
///////////////////////////////////////////////////////////
void	GexDrillWhatIf::OnResetOriginalLimits(bool bResetAllLines)
{
    TableItem *pCell;
    QString	strString;
    int		iLine;

    CGexGroupOfFiles * pGroup = gexReport->getGroupsList().isEmpty()?NULL:gexReport->getGroupsList().first();	// Group#1
    if(pGroup == NULL)
        return;	// Just in case

    // First file in Group#1...must always exist.
    CGexFileInGroup  *pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
    if(pFile == NULL)
        return;	// Just in case

   // bool bStopOnFail	= (comboBoxStopOnFail->currentIndex() == 0) ? true: false;
    int  nComputeOn		= comboBoxComputeOn->itemData(comboBoxComputeOn->currentIndex()).toInt();
    gexReport->VirtualRetest(false, false, (CGexReport::virtualRetestParts) nComputeOn);

    for(iLine=0;iLine<TableGuardBanding->rowCount();iLine++)
    {
        if(TableGuardBanding->item(iLine, 0)->isSelected() || bResetAllLines)
        {
            // Reset LL to default
            pCell = (TableItem *) TableGuardBanding->item(iLine, TABLE_COLUMN_LL_REF);	// Original LL
            strString = pCell->text();
            pCell = (TableItem *) TableGuardBanding->item(iLine, TABLE_COLUMN_LL_NEW);	// What-If LL
            pCell->setText(strString);
            pCell->SetWhatIfFlags(0);

            // Force to repaint this cell
            pCell->paint();

            // Reset HL to default
            pCell = (TableItem *) TableGuardBanding->item(iLine, TABLE_COLUMN_HL_REF);	// Original HL
            strString = pCell->text();
            pCell = (TableItem *) TableGuardBanding->item(iLine, TABLE_COLUMN_HL_NEW);	// What-If HL
            pCell->setText(strString);
            pCell->SetWhatIfFlags(0);

            // Force to repaint this cell
            pCell->paint();

            // Reset Cp to default
            pCell = (TableItem *) TableGuardBanding->item(iLine, TABLE_COLUMN_CP_REF);	// Original Cp
            strString = pCell->text();
            pCell = (TableItem *) TableGuardBanding->item(iLine, TABLE_COLUMN_CP_NEW);	// What-If Cp
            pCell->setText(strString);
            pCell->SetWhatIfFlags(0);

            // Reset Cr to default
            pCell = (TableItem *) TableGuardBanding->item(iLine, TABLE_COLUMN_CR_NEW);	// What-If Cp
            pCell->setText( CalculateCr(strString) ) ;
            pCell->SetWhatIfFlags(0);

            // Force to repaint this cell
            pCell->paint();

            ComputeRefValues(iLine, pFile);

            // Reset Cpk to default
           /* pCell = (TableItem *) TableGuardBanding->item(iLine, TABLE_COLUMN_CPK_REF);// Original Cpk
            strString = pCell->text();
            pCell = (TableItem *) TableGuardBanding->item(iLine, TABLE_COLUMN_CPK_NEW);	// What-If Cpk
            pCell->setText(strString);
            pCell->SetWhatIfFlags(0);

            // Force to repaint this cell
            pCell->paint();

            // Reset Yield to default
            pCell = (TableItem *) TableGuardBanding->item(iLine, TABLE_COLUMN_YIELD_REF);// Original yield
            strString = pCell->text();
            pCell = (TableItem *) TableGuardBanding->item(iLine, TABLE_COLUMN_YIELD_NEW);	// What-If yield
            pCell->setText(strString);
            pCell->SetWhatIfFlags(0);

            // Force to repaint this cell
            pCell->paint();*/

            // Reset Delta yield to 0.
            pCell = (TableItem *) TableGuardBanding->item(iLine, TABLE_COLUMN_DELTA_YIELD);	// Delta yield
            pCell->SetWhatIfFlags(0);
            pCell->setText("0.00 %");

            // Force to repaint this cell
            pCell->paint();
        }
    }
}

///////////////////////////////////////////////////////////
// Save the Table into CSV file
///////////////////////////////////////////////////////////
void	GexDrillWhatIf::OnSaveTableToDisk(void)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_EVALUATION)
    {
        // Evaluation mode, refuse to run this function!
        GS::Gex::Message::information(
            "", "This function is disabled in Evaluation mode\n\nContact " +
            QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }

    QString strTitle = gexReport->getReportOptions() ? gexReport->getReportOptions()->strReportTitle : "";
    if(strTitle.isEmpty())
        strTitle = "Data";
    strTitle += "_WhatIf.csv";


    // Let's user tell where to save the table.
    QString strFile = QFileDialog::getSaveFileName(this,
                    "Save Table data as...",
                    strTitle,
                    "Spreadsheet CSV(*.csv)",
                    NULL,
                    QFileDialog::DontConfirmOverwrite);

    // If no file selected, ignore command.
    if(strFile.isEmpty())
        return;

    // Check if file exists.
    QFile f(strFile);
    if(f.exists() == true)
    {
        // File exists...overwrite it?
        bool lOk;
        GS::Gex::Message::request("", "File already exists. Overwrite it ?", lOk);
        if (! lOk)
        {
            return;
        }
    }

    // Assign file I/O stream
    QTextStream hCsvTableFile;
    QString strCell;
    QString strMessage;
    long	lLine,lCol;

    // Create file
    if(!f.open(QIODevice::WriteOnly))
    {
        // Error. Can't create CSV file!
        GS::Gex::Message::information(
            "", "Failed to create CSV data file\n"
            "Disk full or write protection issue.");
        return;
    }

    hCsvTableFile.setDevice(&f);

    // Write as first data line the Query names.
    hCsvTableFile << "# Quantix " << WHAT_IF_CSV_HEADER_V61 << endl ;
    hCsvTableFile << GS::Gex::Engine::GetInstance().Get("AppFullName").toString() << endl ;
    hCsvTableFile << "www.mentor.com" << endl ;
    hCsvTableFile << endl;
    hCsvTableFile << "Status column definition : 0 = original value; 1 = edited value; 2 = calculated value" << endl;
    hCsvTableFile << endl << endl;

    for(lCol = 0; lCol < TableGuardBanding->columnCount(); lCol++)
    {
        // Only save columns that are visible (not hidden).
        hCsvTableFile << TableGuardBanding->horizontalHeaderItem(lCol)->text() << "," ;
        switch(lCol)
        {
            case TABLE_COLUMN_LL_NEW		:	// Low  Limit (What-if)
                // Add column for test limits units + Column separator
                hCsvTableFile << "Units," ;
                // Add column for status cell + Column separator
                hCsvTableFile << "Status," ;
                break;

            case TABLE_COLUMN_HL_NEW		:	// High  Limit (What-if)
                // Add column for test limits units + Column separator
                hCsvTableFile << "Units," ;
                // Add column for status cell + Column separator
                hCsvTableFile << "Status," ;
                break;

            case TABLE_COLUMN_CP_NEW		:	// CP (What-if)
                // Add column for status cell + Column separator
                hCsvTableFile << "Status," ;
                break;

            case TABLE_COLUMN_CPK_NEW		:	// CPk (What-if)
                // Add column for status cell + Column separator
                hCsvTableFile << "Status," ;
                break;

            case TABLE_COLUMN_LL_REF		:	// Low  Limit (Ref)
            case TABLE_COLUMN_HL_REF		:	// High  Limit (Ref)
            case TABLE_COLUMN_MEAN			:	// Mean
            case TABLE_COLUMN_SIGMA			:	// Sigma
                // Add column for test limits units + Column separator
                hCsvTableFile << "Units," ;
                break;
        }
    }
    hCsvTableFile << endl;

    // Write table content
    TableItem * pCell = NULL;
    int	iIndex;
    for(lLine = 0; lLine < TableGuardBanding->rowCount() ;lLine++)
    {
        // Write cell string.
        for(lCol = 0; lCol < TableGuardBanding->columnCount(); lCol++)
        {
            // Write Data <tab> Units <tab>
            pCell   = (TableItem *)TableGuardBanding->item(lLine, lCol);
            strCell = pCell->text();

            switch(lCol)
            {
                case TABLE_COLUMN_NAME			: // Test name: ensure no ',' in test name!
                    strCell.replace(",",";");
                    break;

                case TABLE_COLUMN_LL_NEW		:	// Low  Limit (What-if)
                    // Split the Test limit from its units.
                    iIndex = strCell.indexOf(' ');
                    if(iIndex >= 0)
                        strCell.insert(iIndex,",");
                    else
                        strCell += ",";	// No limits, ensure we append the field separator.


                    if (pCell->WhatIfFlags() & GEX_DRILL_LL)
                        strCell += ",1";
                    else if (pCell->WhatIfFlags() & GEX_DRILL_RESULT)
                        strCell += ",2";
                    else
                        strCell += ",0";
                    break;

                case TABLE_COLUMN_HL_NEW		:	// High  Limit (What-if)
                    // Split the Test limit from its units.
                    iIndex = strCell.indexOf(' ');
                    if(iIndex >= 0)
                        strCell.insert(iIndex,",");
                    else
                        strCell += ",";	// No limits, ensure we append the field separator.

                    if (pCell->WhatIfFlags() & GEX_DRILL_HL)
                        strCell += ",1";
                    else if (pCell->WhatIfFlags() & GEX_DRILL_RESULT)
                        strCell += ",2";
                    else
                        strCell += ",0";

                    break;

                case TABLE_COLUMN_CP_NEW		: // CP (what if)
                    if (pCell->WhatIfFlags() & GEX_DRILL_CP)
                        strCell += ",1";
                    else if (pCell->WhatIfFlags() & GEX_DRILL_RESULT)
                        strCell += ",2";
                    else
                        strCell += ",0";
                    break;

                case TABLE_COLUMN_CPK_NEW		: // CPk (what if)
                    if (pCell->WhatIfFlags() & GEX_DRILL_CPK)
                        strCell += ",1";
                    else if (pCell->WhatIfFlags() & GEX_DRILL_RESULT)
                        strCell += ",2";
                    else
                        strCell += ",0";
                    break;

                case TABLE_COLUMN_LL_REF		:	// Low  Limit (Ref)
                case TABLE_COLUMN_HL_REF		:	// High  Limit (Ref)
                case TABLE_COLUMN_MEAN			:	// Mean
                case TABLE_COLUMN_SIGMA			:	// Sigma
                        // Split the Test limit from its units.
                        iIndex = strCell.indexOf(' ');
                        if(iIndex >= 0)
                            strCell.insert(iIndex,",");
                        else
                            strCell += ",";	// No limits, ensure we append the field separator.
                        break;

                }

            hCsvTableFile << strCell << ",";
        }

        // Add new line character
        hCsvTableFile << endl;
    }
    // Close file.
    f.close();

    strMessage = "Table saved!\nFile created: " + strFile;
    GS::Gex::Message::information("", strMessage);
}

///////////////////////////////////////////////////////////
// Save table into clipboard, ready to paste to spreadsheet CSV file (Windows only)
///////////////////////////////////////////////////////////
void	GexDrillWhatIf::OnSaveTableToExcelClipboard(void)
{
#if (defined _WIN32 || __linux__)
    if(GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_EVALUATION)
    {
        // Evaluation mode, refuse to run this function!
        GS::Gex::Message::information(
            "", "This function is disabled in Evaluation mode\n\nContact " +
            QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }

    QString strCell;
    QString strMessage;
    long	lLine,lCol,lMaxCol;
    int		iIndex;
    QString strBigLine;

    // Write as first data line the Query names.
    for(lCol=0;lCol<TableGuardBanding->columnCount();lCol++)
    {
        // Write cell string.
        strBigLine += TableGuardBanding->horizontalHeaderItem(lCol)->text() ;

        switch(lCol)
        {
            case TABLE_COLUMN_LL_NEW	:	// Low  Limit (What-if)
            case TABLE_COLUMN_HL_NEW	:	// High  Limit (What-if)
            case TABLE_COLUMN_LL_REF	:	// Low  Limit (Ref)
            case TABLE_COLUMN_HL_REF	:	// High  Limit (Ref)
            case TABLE_COLUMN_MEAN		:	// Mean
            case TABLE_COLUMN_SIGMA		:	// Sigma
                // Add column for test limits units + Column separator
                strBigLine += "\tUnits";
                break;
        }

        // Column separator
        strBigLine += "\t";
    }
    strBigLine += "\n";

    for(lLine = 0; lLine < TableGuardBanding->rowCount(); lLine++)
    {
        // Ignore all leading empty cells on each line
        lMaxCol = TableGuardBanding->columnCount();
        while(lMaxCol > 0)
        {
            strCell = (TableGuardBanding->item(lLine,lMaxCol-1)->text()).trimmed();
            if(strCell.isEmpty() == false)
                break;	// Reached the last cell that is not blank.
            lMaxCol--;
        }


        // Write cell string.
        for(lCol = 0; lCol < lMaxCol; lCol++)
        {
            // Write Data <tab> Units <tab>
            strCell = TableGuardBanding->item(lLine,lCol)->text();

            switch(lCol)
            {
            case TABLE_COLUMN_LL_NEW		:	// Low  Limit (What-if)
            case TABLE_COLUMN_HL_NEW		:	// High  Limit (What-if)
            case TABLE_COLUMN_LL_REF		:	// Low  Limit (Ref)
            case TABLE_COLUMN_HL_REF		:	// High  Limit (Ref)
            case TABLE_COLUMN_MEAN			:	// Mean
            case TABLE_COLUMN_SIGMA			:	// Sigma
                    // Split the Test limit from its units.
                    iIndex = strCell.indexOf(' ');
                    if(iIndex >= 0)
                        strCell.insert(iIndex,"\t");
                    else
                        strCell += "\t";	// No limits, ensure we append the field separator.
                    break;

            }

            // Export to clipboard in XL format:
            strBigLine += strCell;
            strBigLine += "\t";
        }
        strBigLine += "\n";
    }

    // clipboard copy.
    QClipboard *lClipBoard = QGuiApplication::clipboard();
    if (!lClipBoard)
    {
        GSLOG(SYSLOG_SEV_WARNING, "failed to retrieve application clipboard");
    }
    else
    {
        lClipBoard->setText(strBigLine,QClipboard::Clipboard );
    }

    strMessage = "Table saved to clipboard in Spreadsheet format,\nready to paste!\nLaunch Spreadsheet application now?";
    bool lOk;
    GS::Gex::Message::request("", strMessage, lOk);
    if (! lOk)
    {   
        return;
    }
#ifdef _WIN32
        // Launch spreadsheet editor
    ShellExecuteA(NULL,
           "open",
        ReportOptions.GetOption("preferences", "ssheet_editor").toString().toLatin1().constData(),	//ReportOptions.m_PrefMap["ssheet_editor"],
           NULL,
           NULL,
           SW_SHOWNORMAL);
#else
  //system(ReportOptions.m_PrefMap["ssheet_editor"].toLatin1().constData());
  if (system(ReportOptions.GetOption("preferences", "ssheet_editor").
             toString().toLatin1().constData()) == -1) {
    //FIXME: send error
  }
#endif

#endif
}

// -------------------------------------------------------------------------- //
// OnSnapshot
// -------------------------------------------------------------------------- //
void GexDrillWhatIf::OnSnapshot()
{
  QIcon copy_icon;
  QIcon save_icon;
  copy_icon.addPixmap(*pixCopy);
  save_icon.addPixmap(*pixSave);

  QMenu menu;
  QAction* copy_item = menu.addAction(copy_icon, "Copy chart to clipboard");
  QAction* save_item = menu.addAction(save_icon, "Save chart to disk");
  QAction* repo_item =
    menu.addAction(copy_icon, "Add chart to the report file");

  menu.setMouseTracking(true);
  QAction* action = menu.exec(QCursor::pos());
  menu.setMouseTracking(false);

  if      (action == copy_item) { return pGexMainWindow->SnapshotCopy(); }
  else if (action == save_item) { return pGexMainWindow->SnapshotSave(); }
  else if (action != repo_item) { return; }

  int iLine, iCol;
  TableItem* pCell;
  bool bEdited;
  const char* szBackColor;
  SnapshotDialog dialogSnapshot;

  // Check if we have at least one line edited!
  bEdited = false;
  for (iLine = 0; iLine < TableGuardBanding->rowCount(); iLine++) {
    for (iCol = TABLE_COLUMN_LL_NEW; iCol <= TABLE_COLUMN_YIELD_NEW; iCol++) {
      // Cell only exist if table is not empty...
      pCell = (TableItem*) TableGuardBanding->item(iLine, iCol);
      if (pCell == NULL)
        return;  // Table is probably empty!

      switch (pCell->WhatIfFlags())
      {
      case GEX_DRILL_LL:
      case GEX_DRILL_HL:
      case (GEX_DRILL_LL | GEX_DRILL_HL):
      case GEX_DRILL_CP:
      case GEX_DRILL_CPK:
      case GEX_DRILL_YIELD:
        bEdited = true;
        break;
      }
    }
  }

  // If nothing changed in the table, simply ignore snapshot!
  if (bEdited == false)
  {
      GS::Gex::Message::
          warning("", "No test limits edited...\nSnapshot is ignored!");
      return;
  }

  // Prompt Dialog box: Request 'link name' + notes
  if (dialogSnapshot.exec() != 1)
    return;

  // Create snapshot page with given notes, also update snapshot home page.
  QString strLinkName;
  strLinkName = dialogSnapshot.snapshotTitle->text();
  if (strLinkName.isEmpty() == true)
    strLinkName = "No name given";
  strLinkName = strLinkName.trimmed();
  SnapShotReport cSnapReport;
  if (cSnapReport.AddLink(strLinkName,
                          dialogSnapshot.textEditSnapshotComment) !=
      GS::StdLib::Stdf::NoError)
    return;  // Internal error.

  // Create the snapshot page header
  FILE* hReportFile = cSnapReport.AddPage(strLinkName);
  if (hReportFile == NULL) {
    // Failed creating header of snapshot page...
      GS::Gex::Message::
          critical("", "ERROR: Failed creating HTML snapshot page...");
    return;  // Failed
  }

  // Write table header
  fprintf(hReportFile,
          "<table border=\"0\" width=\"98%%\" cellspacing=\"1\">\n");
  fprintf(hReportFile, "<tr bgcolor=%s>\n", szFieldColor);
  fprintf(hReportFile, "<td><b>Test</b></td>\n");
  fprintf(hReportFile, "<td><b>Name</b></td>\n");
  fprintf(hReportFile, "<td><b>Fail Bin</b></td>\n");
  fprintf(hReportFile, "<td><b>Low L.<br/>(New)</b></td>\n");
  fprintf(hReportFile, "<td><b>High L.<br/>(New)</b></td>\n");
  fprintf(hReportFile, "<td><b>Cp<br/>(New)</b></td>\n");
  fprintf(hReportFile, "<td><b>Cpk<br/>(New)</b></td>\n");
  fprintf(hReportFile, "<td><b>Yield<br/>(New)</b></td>\n");
  fprintf(hReportFile, "<td><b>Low L.<br/>(Ref)</b></td>\n");
  fprintf(hReportFile, "<td><b>High L.<br/>(Ref)</b></td>\n");
  fprintf(hReportFile, "<td><b>Cp<br/>(Ref)</b></td>\n");
  fprintf(hReportFile, "<td><b>Cpk<br/>(Ref)</b></td>\n");
  fprintf(hReportFile, "<td><b>Yield<br/>(Ref)</b></td>\n");
  fprintf(hReportFile, "<td><b>Mean</b></td>\n");
  fprintf(hReportFile, "<td><b>Sigma</b></td>\n");
  fprintf(hReportFile, "<td><b>Delta Yield<br/>(New)</b></td>\n");
  fprintf(hReportFile, "</tr>\n");

  // Write report into page
  // Check all selections...in a same column...
  // and force them to the same value.
  for (iLine = 0; iLine < TableGuardBanding->rowCount(); iLine++) {
    bEdited = false;
    for (iCol = TABLE_COLUMN_LL_NEW; iCol <= TABLE_COLUMN_YIELD_NEW; iCol++) {
      // Cell only exist if table is not empty...
      pCell = (TableItem*) TableGuardBanding->item(iLine, iCol);
      if (pCell == NULL)
        return;  // Table is probably empty!

      switch (pCell->WhatIfFlags())
      {
      case GEX_DRILL_LL:
      case GEX_DRILL_HL:
      case (GEX_DRILL_LL | GEX_DRILL_HL):
      case GEX_DRILL_CP:
      case GEX_DRILL_CPK:
      case GEX_DRILL_YIELD:
        bEdited = true;
        break;
      }
    }

    // If test line has cells edited, add this line to the HTML page!
    if (bEdited == true) {
      // Copy cells to HTML page...
      fprintf(hReportFile, "<tr>\n");

      for (int iCol = 0; iCol < TABLE_COLUMNS; iCol++) {
        pCell = (TableItem*) TableGuardBanding->item(iLine, iCol);
        switch (pCell->WhatIfFlags())
        {
        case GEX_DRILL_LL:
        case GEX_DRILL_HL:
        case (GEX_DRILL_LL | GEX_DRILL_HL):
        case GEX_DRILL_CP:
        case GEX_DRILL_CPK:
        case GEX_DRILL_YIELD:
          szBackColor = szDrillColorIf;  // 'If' (edited field)
          break;
        default:
          szBackColor = szDrillColorThen;  // 'Then' (Result field)
          break;
        }
        // Keep default color for first 2 columns,
        // or Ref columns (>=7) (Test# , Test name)
        if (iCol < TABLE_COLUMN_LL_NEW || iCol > TABLE_COLUMN_YIELD_NEW)
          szBackColor = szDataColor;

        fprintf(hReportFile, "<td bgcolor=%s>%s</td>\n", szBackColor,
                pCell->text().toLatin1().constData());
      }
      // End of table line.
      fprintf(hReportFile, "</tr>\n");
    }
  }

  // Close table
  fprintf(hReportFile, "</table>\n<br><br>");

  // Write color legend
  fprintf(hReportFile, "<br>\n<p><b>Color legend:</b></p>\n");
  fprintf(hReportFile, "<table border=\"0\" cellspacing=\"1\">\n");
  fprintf(hReportFile, "<tr>\n");
  fprintf(hReportFile, "<td bgcolor=%s width=30px></td>\n", szDrillColorIf);
  fprintf(hReportFile, "<td>If... (Color for the 'If' condition)</td>\n");
  fprintf(hReportFile, "</tr>\n");
  fprintf(hReportFile, "<tr>\n");
  fprintf(hReportFile, "<td bgcolor=%s width=30px></td>\n", szDrillColorThen);
  fprintf(hReportFile, "<td>Then... (Color for the 'Then' results)</td>\n");
  fprintf(hReportFile, "</tr>\n");
  fprintf(hReportFile, "</table>\n");

  // Write 'Notes' + Close page
  cSnapReport.ClosePage(dialogSnapshot.textEditSnapshotComment);

  // Message of success!
    GS::Gex::Message::information("", "Snapshot taken!");
}

///////////////////////////////////////////////////////////
// Format new limit columns after editing
///////////////////////////////////////////////////////////
void GexDrillWhatIf::formatNewLimit(QString& strString, int nLine, int nType)
{
    if(gexReport == NULL)
        return;	// Just in case

    CGexGroupOfFiles *	pGroup		= NULL;
    CGexFileInGroup  *	pFile		= NULL;
    TableItem *			pCell		= NULL;
    CTest *				ptTestCell	= NULL;
    QString				strLabel;
    QString				strName;
    long				lTestNumber;
    long				lPinmapIndex;

    // First: write info about reference group#1
    pGroup = gexReport->getGroupsList().isEmpty()?NULL:gexReport->getGroupsList().first();	// Group#1
    if(pGroup == NULL)
        return;	// Just in case

    // First file in Group#1...must always exist.
    pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
    if(pFile == NULL)
        return;	// Just in case

    // Get test number
    pCell = (TableItem *) TableGuardBanding->item(nLine, 0);

    if (pCell)
    {
        strLabel	= pCell->text();

        if (sscanf(strLabel.toLatin1().constData(),"%ld%*c%ld",
                   &lTestNumber, &lPinmapIndex) < 2)
            lPinmapIndex = GEX_PTEST;

        // Get test name
        pCell	= (TableItem *) TableGuardBanding->item(nLine, 1);
        strName = pCell->text();

        // Check if can find given test#.Pinmap#
        if (pFile->FindTestCell(lTestNumber, lPinmapIndex, &ptTestCell, false, false, strName.toLatin1().data()) ==1)
        {
            char	szLimit[GEX_LIMIT_LABEL];
            char	szString[50];
            char	szScale[10];
            double	dLimit		= 0.0;
            int		nArgs		= sscanf(strString.toLatin1().constData(),"%lf %1s", &dLimit, szScale);
            int		nLimScale	= 0;

            // Get the right limit and scale
            if (nType == GEX_DRILL_LL)
            {
                nLimScale = ptTestCell->llm_scal;
                // If low limit exists
                if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
                    pFile->FormatTestLimit(ptTestCell, szLimit, ptTestCell->GetCurrentLimitItem()->lfLowLimit, nLimScale, false);
                else
                    strcpy(szLimit, "n/a");
            }
            else
            {
                nLimScale = ptTestCell->hlm_scal;
                // If high limit exists
                if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
                    pFile->FormatTestLimit(ptTestCell, szLimit, ptTestCell->GetCurrentLimitItem()->lfHighLimit, nLimScale, false);
                else
                    strcpy(szLimit, "n/a");
            }

            switch (nArgs)
            {
                // No unit found, use reference limit unit
                case 1	:	if (sscanf(szLimit, "%lf %s", &dLimit, szScale) > 1)
                                strString += QString(" ") + QString(szScale);
                            break;

                // Unit found, reformat string
                case 2	:	RescaleValue(&dLimit, nLimScale, szScale, ptTestCell->szTestUnits);
                            pFile->FormatTestResultNoUnits(&dLimit,-nLimScale);
                            pFile->FormatTestLimit(ptTestCell, szString, dLimit, nLimScale, false);
                            strString = szString;

                default	:	break;
            }
        }
    }
}

///////////////////////////////////////////////////////////
// Table constructor
///////////////////////////////////////////////////////////
Table::Table(QWidget * parent)
    : QTableWidget(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    setSortingEnabled(true);
    setGeometry( QRect( 21, 79, 620, 255 ) );

    // Clear table + reload it with test info.
    ResetTable(true);
}

///////////////////////////////////////////////////////////
// Reset table: Empties table, then loads it with all default values (unless otherwise specified)
///////////////////////////////////////////////////////////
void Table::ResetTable(bool bLoadDefaultValues)
{
    QHeaderView *pTableHeader;
    CTest	*ptTestCell;	// Pointer to test cell to receive STDF info.
    TableItem *pCell;
    float	fYield;
    int	iRow,iCol;
    CGexStats	cStats;

    // Ensure we use latest options set
    cStats.UpdateOptions(&ReportOptions);

    // Get pointer to horizontal header: show that Test# are sorted in ascending order (default)
    pTableHeader = horizontalHeader();
    pTableHeader->setSortIndicator(0, Qt::AscendingOrder);

    // Delete content of table if any...
    for(iRow=0;iRow< rowCount(); iRow++)
    {
        for(iCol=0;iCol< columnCount(); iCol++)
        {
            pCell = (TableItem*)item(iRow,iCol);
            if(pCell != NULL)
            {
                takeItem(iRow, iCol);
                delete pCell;
            }
        }
    }

    // Test #, name, LowL, HighL, Cp, Cpk, Yield
    // Get pointer to first group & first file (we always have them exist)
    CGexGroupOfFiles *pGroup = gexReport->getGroupsList().isEmpty()?NULL:gexReport->getGroupsList().first();
    CGexFileInGroup *pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

    // Set number of rows in table counter
    ptTestCell = pGroup->cMergedData.ptMergedTestList;
    iRow=0;
    while(ptTestCell != NULL)
    {
        // If neither PTR values, this test was not executed !
        if(ptTestCell->ldSamplesValidExecs <= 0)
            goto NextCell;

        // IF Muti-result parametric test, do not show master test record
        if(ptTestCell->lResultArraySize > 0)
            goto NextCell;

        // If not a parametric / multiparametric (eg: functional) test, ignore!
        if(ptTestCell->bTestType == 'F')
            goto NextCell;

        // Ignore Generic Galaxy Parameters
        {
            QString strOptionStorageDevice = (ReportOptions.GetOption("statistics","generic_galaxy_tests")).toString();
            if((ptTestCell->bTestType == '-')
            && (strOptionStorageDevice == "hide") )
                goto NextCell;
        }
    // Update statistics to be based on samples only
    cStats.ComputeBasicTestStatistics(ptTestCell,true);
        // this is a valid test, count it!
        iRow++;
        // Point to next test cell
        NextCell:
        ptTestCell = ptTestCell->GetNextTest();
    };
    setRowCount(iRow);

    setColumnCount(TABLE_COLUMNS);
    QStringList headerList;
    headerList.append(TABLE_HEADER_TEST);
    headerList.append(TABLE_HEADER_NAME);
    headerList.append(TABLE_HEADER_FAIL_BIN);
    headerList.append(TABLE_HEADER_LL_NEW);
    headerList.append(TABLE_HEADER_HL_NEW);
    headerList.append(TABLE_HEADER_CP_NEW);
    headerList.append(TABLE_HEADER_CPK_NEW);
    headerList.append(TABLE_HEADER_CR_NEW);
    headerList.append(TABLE_HEADER_YIELD_NEW);
    headerList.append(TABLE_HEADER_LL_REF);
    headerList.append(TABLE_HEADER_HL_REF);
    headerList.append(TABLE_HEADER_CP_REF);
    headerList.append(TABLE_HEADER_CPK_REF);
    headerList.append(TABLE_HEADER_CR_REF);
    headerList.append(TABLE_HEADER_YIELD_REF);
    headerList.append(TABLE_HEADER_MEAN);
    headerList.append(TABLE_HEADER_SIGMA);
    headerList.append(TABLE_HEADER_DELTA_YIELD);
    setHorizontalHeaderLabels(headerList);


#ifdef _WIN32
    // Resize columns so most of columns fit in the window...
    setColumnWidth (TABLE_COLUMN_TEST,		80);	// Test# : column width
    setColumnWidth (TABLE_COLUMN_LL_NEW,	80);	// LL : column width
    setColumnWidth (TABLE_COLUMN_HL_NEW,	80);	// HL : column width
    setColumnWidth (TABLE_COLUMN_CP_NEW,	50);	// Cp : column width
    setColumnWidth (TABLE_COLUMN_CR_NEW,	50);	// Cr : column width
    setColumnWidth (TABLE_COLUMN_CPK_NEW,	50);	// Cpk : column width
    setColumnWidth (TABLE_COLUMN_YIELD_NEW,	60);	// Yield : column width
    setColumnWidth (TABLE_COLUMN_LL_REF,	80);	// LL (original): column width
    setColumnWidth (TABLE_COLUMN_HL_REF,	80);	// HL (original): column width
    setColumnWidth (TABLE_COLUMN_CP_REF,	60);	// Cp (original): column width
    setColumnWidth (TABLE_COLUMN_CR_REF,	60);	// Cr (original): column width
    setColumnWidth (TABLE_COLUMN_CPK_REF,	60);	// Cpk (original): column width
    setColumnWidth (TABLE_COLUMN_YIELD_REF,	60);	// Yield (original): column width
#endif


    // Check if has to initialize table with test entries...
    if(bLoadDefaultValues == false)
        return;

    // List of tests in this group.
    char	szLimit[GEX_LIMIT_LABEL];
    ptTestCell	= pGroup->cMergedData.ptMergedTestList;
    iRow		= 0;

    while(ptTestCell != NULL)
    {
        // If neither PTR values, this test was not executed !
        if(ptTestCell->ldSamplesValidExecs <= 0)
            goto NextTestCell;

        // IF Muti-result parametric test, do not show master test record
        if(ptTestCell->lResultArraySize > 0)
            goto NextTestCell;

        // If not a parametric / multiparametric (eg: functional) test, ignore!
        if(ptTestCell->bTestType == 'F')
            goto NextTestCell;

        // Ignore Generic Galaxy Parameters
        {
            QString strOptionStorageDevice = (ReportOptions.GetOption("statistics","generic_galaxy_tests")).toString();
            GEX_ASSERT( (strOptionStorageDevice == "hide") || (strOptionStorageDevice=="show") );
            if((ptTestCell->bTestType == '-')
            && (strOptionStorageDevice=="hide"))
                goto NextTestCell;
        }

        // Insert test info into the table...
        // Test #
        pCell = new TableItem( QTableWidgetItem::Type, ptTestCell->szTestLabel);
        pCell->SetWhatIfFlags(0);
        setItem( iRow, TABLE_COLUMN_TEST, pCell );

        // Test name
        pCell = new TableItem( QTableWidgetItem::Type, ptTestCell->strTestName);
        pCell->SetWhatIfFlags(0);
        setItem( iRow, TABLE_COLUMN_NAME, pCell );

        // Test Fail bin
        pCell = new TableItem( QTableWidgetItem::Type, QString::number(ptTestCell->iFailBin));
        pCell->SetWhatIfFlags(0);
        setItem( iRow, TABLE_COLUMN_FAIL_BIN, pCell );

        // LL
        // If low limit exists
        if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
            pFile->FormatTestLimit(ptTestCell, szLimit, ptTestCell->GetCurrentLimitItem()->lfLowLimit, ptTestCell->llm_scal, false);
        else
            strcpy(szLimit, "n/a");

        pCell = new TableItem( QTableWidgetItem::Type, szLimit);
        pCell->SetWhatIfFlags(0);
        pCell->setFlags(pCell->flags()|Qt::ItemIsEditable);
        setItem( iRow, TABLE_COLUMN_LL_NEW, pCell );
        // LL (Original)
        pCell = new TableItem( QTableWidgetItem::Type, szLimit);
        pCell->SetWhatIfFlags(0);
        setItem( iRow, TABLE_COLUMN_LL_REF, pCell );

        // HL
        // If high limit exists
        if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
            pFile->FormatTestLimit(ptTestCell, szLimit, ptTestCell->GetCurrentLimitItem()->lfHighLimit, ptTestCell->hlm_scal, false);
        else
            strcpy(szLimit,"n/a");

        pCell = new TableItem( QTableWidgetItem::Type, szLimit);
        pCell->SetWhatIfFlags(0);
        pCell->setFlags(pCell->flags()|Qt::ItemIsEditable);
        setItem( iRow, TABLE_COLUMN_HL_NEW, pCell );
        // HL (Original)
        pCell = new TableItem( QTableWidgetItem::Type, szLimit);
        pCell->SetWhatIfFlags(0);
        setItem( iRow, TABLE_COLUMN_HL_REF, pCell );

        // Cp
        pCell = new TableItem( QTableWidgetItem::Type, gexReport->CreateResultStringCpCrCpk(ptTestCell->GetCurrentLimitItem()->lfCp));
        pCell->SetWhatIfFlags(0);
        pCell->setFlags(pCell->flags()|Qt::ItemIsEditable);
        setItem( iRow, TABLE_COLUMN_CP_NEW, pCell );
        // Cp (Original)
        pCell = new TableItem( QTableWidgetItem::Type, gexReport->CreateResultStringCpCrCpk(ptTestCell->GetCurrentLimitItem()->lfCp));
        pCell->SetWhatIfFlags(0);
        setItem( iRow, TABLE_COLUMN_CP_REF, pCell );

        // Cpk
        pCell = new TableItem( QTableWidgetItem::Type, gexReport->CreateResultStringCpCrCpk(ptTestCell->GetCurrentLimitItem()->lfCpk));
        pCell->SetWhatIfFlags(0);
        pCell->setFlags(pCell->flags()|Qt::ItemIsEditable);
        setItem( iRow, TABLE_COLUMN_CPK_NEW, pCell );
        // Cpk (Original)
        pCell = new TableItem( QTableWidgetItem::Type, gexReport->CreateResultStringCpCrCpk(ptTestCell->GetCurrentLimitItem()->lfCpk));
        pCell->SetWhatIfFlags(0);
        setItem( iRow, TABLE_COLUMN_CPK_REF, pCell );

        // Cr
        pCell = new TableItem( QTableWidgetItem::Type,  CalculateCr(gexReport->CreateResultStringCpCrCpk(ptTestCell->GetCurrentLimitItem()->lfCp)));
        pCell->SetWhatIfFlags(0);
        setItem( iRow, TABLE_COLUMN_CR_NEW, pCell );
        // Cr (Original)
        pCell = new TableItem( QTableWidgetItem::Type, CalculateCr(gexReport->CreateResultStringCpCrCpk(ptTestCell->GetCurrentLimitItem()->lfCp)));
        pCell->SetWhatIfFlags(0);
        setItem( iRow, TABLE_COLUMN_CR_REF, pCell );


        // Yield
        if(ptTestCell->ldSamplesValidExecs)
            fYield = 100.0f - (100.0f * ptTestCell->GetCurrentLimitItem()->ldSampleFails)/((float)(ptTestCell->ldSamplesValidExecs));
        else
            fYield = -1.0f;
        pCell = new TableItem( QTableWidgetItem::Type, gexReport->CreateResultStringPercent(fYield));
        pCell->SetWhatIfFlags(0);
        setItem( iRow, TABLE_COLUMN_YIELD_NEW, pCell );
        // Yield (Original)
        pCell = new TableItem( QTableWidgetItem::Type, gexReport->CreateResultStringPercent(fYield));
        pCell->SetWhatIfFlags(0);
        setItem( iRow, TABLE_COLUMN_YIELD_REF, pCell );

        // Mean
        pCell = new TableItem( QTableWidgetItem::Type, pFile->FormatTestResult(ptTestCell,ptTestCell->lfMean,ptTestCell->res_scal));
        pCell->SetWhatIfFlags(0);
        setItem( iRow, TABLE_COLUMN_MEAN, pCell );

        // Sigma
        pCell = new TableItem( QTableWidgetItem::Type, pFile->FormatTestResult(ptTestCell,ptTestCell->lfSigma,ptTestCell->res_scal));
        pCell->SetWhatIfFlags(0);
        setItem( iRow, TABLE_COLUMN_SIGMA, pCell );

        // Delata Yield
        pCell = new TableItem( QTableWidgetItem::Type, "0.00 %");
        pCell->SetWhatIfFlags(0);
        setItem( iRow, TABLE_COLUMN_DELTA_YIELD, pCell );

        // Next line
        iRow++;

        // Point to next test cell
        NextTestCell:
        ptTestCell = ptTestCell->GetNextTest();
    };	// Loop until all test cells read.


}

///////////////////////////////////////////////////////////
// A column has to be sorted...
///////////////////////////////////////////////////////////
void Table::sortColumn(int col, bool ascending, bool /*wholeRows*/)
{
    QHeaderView *pTableHeader;

    // Refuse to sort Test limits columns
    if(col == TABLE_COLUMN_LL_NEW || col == TABLE_COLUMN_HL_NEW || col == TABLE_COLUMN_LL_REF || col == TABLE_COLUMN_HL_REF)
    {
        // Set focus on first cell in the column!
        setCurrentCell(0, col);
        return;
    }

    // Get pointer to horizontal header
    pTableHeader = horizontalHeader();
    Qt::SortOrder ascendingOrder = Qt::AscendingOrder;
    if (!ascending) ascendingOrder = Qt::DescendingOrder;
    pTableHeader->setSortIndicator(col, ascendingOrder);

    // do sort
    QTableWidget::sortItems( col, ascendingOrder);

    // Set focus on first cell in the column!
    setCurrentCell(0,col);
}

///////////////////////////////////////////////////////////
// Re-implementation of 'key' so sorting on text & numbers is fine.
///////////////////////////////////////////////////////////
QString TableItem::key() const
{
    long   lData=0;
    float fData=0;
    bool	bNegative=false;

    QString strText=text();	// GetText string in cell.
    int iCol = column();
    // If column is one of: Cp, Cpk, Yield...we deal with floating point numbers!
    switch(iCol)
    {
        case TABLE_COLUMN_TEST		: // Test #
        case TABLE_COLUMN_CP_NEW	: // Cp
        case TABLE_COLUMN_CPK_NEW	: // Cpk
        case TABLE_COLUMN_YIELD_NEW	: // Yield
        case TABLE_COLUMN_CP_REF	: // Cp (Original)
        case TABLE_COLUMN_CPK_REF	:// Cpk (Original)
        case TABLE_COLUMN_YIELD_REF	:// Yield (Original)
        case TABLE_COLUMN_MEAN		:// Mean
        case TABLE_COLUMN_SIGMA		:// Sigma
            if(strText.isEmpty())
                break;

            if(strText[0] == 'n')
                lData = 100000000;	// This cell included string 'n/a'
            else
            {
                sscanf(strText.toLatin1().constData(),"%f",&fData);
                fData *= 100.0;	// we multiply by 100 so number is an integer!
                if(fData<0)
                {
                  fData = 1e8+fData;
                  bNegative = true;
                }
                if(fData > 1e8)
                    lData = 100000000;
                else
                    lData = (long) fData;
            }

            strText = QString::number(lData);
            // Ensures string is in format '00000<data>'.
            strText = strText.rightJustified(10,'0');

            if(bNegative)
                strText = "A" + strText;
            else
                strText = "Z" + strText;

            break;

        default:
        case TABLE_COLUMN_NAME		: // Test name
        case TABLE_COLUMN_FAIL_BIN	: // Fail bin
        case TABLE_COLUMN_LL_NEW	: // LL
        case TABLE_COLUMN_HL_NEW	: // HL
        case TABLE_COLUMN_LL_REF	: // LL (Original)
        case TABLE_COLUMN_HL_REF	: // HL (Original)
            break;
    }

    return strText;
}
///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
TableItem::TableItem(int et, const QString &txt): QTableWidgetItem(txt, et)
{
    mFlags = 0;
    mPreviousFlags = 0;
    setFlags(flags()&~Qt::ItemIsEditable);
    paint();
}

///////////////////////////////////////////////////////////
// A cell has to be painted...check color to use!
///////////////////////////////////////////////////////////
void TableItem::paint()
{
    // Disable capture of signals in TableGuardBanding
    QTableWidget* tab = this->tableWidget();
    if (tab)
        tab->blockSignals(true);
    switch(mFlags)
    {
        case GEX_DRILL_LL:
        case GEX_DRILL_HL:
        case (GEX_DRILL_LL | GEX_DRILL_HL):
        case GEX_DRILL_CP:
        case GEX_DRILL_CPK:
        case GEX_DRILL_YIELD:
            setBackground(QBrush(QColor(Qt::yellow)));
            break;

        case GEX_DRILL_RESULT:
            setBackground(QBrush(QColor(85,255,127)));
            break;

        case GEX_DRILL_NULL_SIGMA:
                setBackground(QBrush(QColor(Qt::red)));
                break;

        default:
            setBackground(QBrush(QColor(Qt::white)));
            break;
    }
}

void TableItem::SetWhatIfFlags(unsigned char flag)
{
    mPreviousFlags = mFlags;
    mFlags = flag;
}

unsigned char TableItem::WhatIfFlags() const
{
    return mFlags;
}

//unsigned char TableItem::WhatIfPreviousFlags() const
//{
//    return mPreviousFlags;
//}

void	GexDrillWhatIf::OnSaveLimitsToMapFile(void)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_EVALUATION)
    {
        // Evaluation mode, refuse to run this function!
        GS::Gex::Message::information(
            "", "This function is disabled in Evaluation mode\n\nContact " +
            QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }

    QString strTitle = gexReport->getReportOptions() ? gexReport->getReportOptions()->strReportTitle : "";
    if(strTitle.isEmpty())
        strTitle = "Data";
    strTitle += "_WhatIf.xml";


    // Let's user tell where to save the table.
    QString strFile = QFileDialog::getSaveFileName(this,
                    "Save tests limits data as...",
                    strTitle,
                    "XML file(*.xml)",
                    NULL,
                    QFileDialog::DontConfirmOverwrite);
    // If no file selected, ignore command.
    if(strFile.isEmpty())
        return;

    // Check if file exists.
    QFile f(strFile);
    if(f.exists() == true)
    {
        // File exists...overwrite it?
        bool lOk;
        GS::Gex::Message::request("", "File already exists. Overwrite it ?", lOk);
        if (! lOk)
        {
            return;
        }
    }

    // Create file
    if(!f.open(QIODevice::WriteOnly))
    {
        // Error. Can't create CSV file!
        GS::Gex::Message::information(
            "", "Failed to create XML data file\n"
    "Disk full or write protection issue.");
return;
}

QTextStream hXMLTableFile;
hXMLTableFile.setDevice(&f);

hXMLTableFile << "<gex_dataset_config version=\"1.0\"> \n";
hXMLTableFile << "<test>\n";

TableItem * pCell = NULL;
QString strCell;
QString strXMLContent;

for(int iLine = 0; iLine < TableGuardBanding->rowCount() ;iLine++)
{
strXMLContent.clear();
        bool bEditedLimits = false;

        for(int iCol = 0; iCol < TableGuardBanding->columnCount(); iCol++){

            pCell   = (TableItem *)TableGuardBanding->item(iLine, iCol);
            strCell = pCell->text();

            if(iCol == TABLE_COLUMN_TEST){
                strXMLContent += QString("<number>%1</number>\n").arg(strCell);
            }else if(iCol == TABLE_COLUMN_NAME){
                QString strName = strCell;
                strName.replace("&","&amp;");
                strName.replace("<","&lt;");
                strName.replace(">","&gt;");
                strXMLContent += QString("<name>%1</name>\n").arg(strName);

            }else if(iCol == TABLE_COLUMN_LL_NEW){
                if(pCell->WhatIfFlags() & GEX_DRILL_LL || pCell->WhatIfFlags() & GEX_DRILL_RESULT){
                    bEditedLimits = true;
                    QString lLocalValue = strCell.section(' ', 0, 0);
                    if (! lLocalValue.isEmpty()){
                        QString lLocalUnit = strCell.section(' ', 1, 1);
                        if(! lLocalUnit.isEmpty())
                            normalizeValueAccordingToUnitPrefix(lLocalValue, lLocalUnit);
                        strXMLContent += QString("<LL>%1</LL>\n").arg(lLocalValue);
                    }
                }

            }else if(iCol == TABLE_COLUMN_HL_NEW){
                if(pCell->WhatIfFlags() & GEX_DRILL_HL || pCell->WhatIfFlags() & GEX_DRILL_RESULT){
                    bEditedLimits = true;
                    QString lLocalValue = strCell.section(' ', 0, 0);
                    if (! lLocalValue.isEmpty()){
                        QString lLocalUnit = strCell.section(' ', 1, 1);
                        if(! lLocalUnit.isEmpty())
                            normalizeValueAccordingToUnitPrefix(lLocalValue, lLocalUnit);
                        strXMLContent += QString("<HL>%1</HL>\n").arg(lLocalValue);
                    }
                }

            }
        }
        if(!bEditedLimits) continue;

        if(!strXMLContent.isEmpty()){

            hXMLTableFile << "<update source=\"whatif\">\n";
            hXMLTableFile << strXMLContent;
            hXMLTableFile << "</update>\n";


        }
    }

    hXMLTableFile << "</test>\n";
    hXMLTableFile << "</gex_dataset_config> \n";

    // Close file.
    f.close();
    QString strMessage = "Test mapping file created: " + strFile;
    GS::Gex::Message::information("", strMessage);
}


void WhatIfItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItemV4 myOption = option;
    myOption.text = index.data().toString();
    QBrush lBackgrounBrush = index.data(Qt::BackgroundRole).value<QBrush>();
    QBrush lForegroundBrush = index.data(Qt::ForegroundRole).value<QBrush>();

    if (option.showDecorationSelected && (option.state & QStyle::State_Selected))
    {
        myOption.palette.setBrush(QPalette::Normal, QPalette::HighlightedText, lForegroundBrush.color());
        if (lBackgrounBrush.color() == QColor(Qt::white))
            myOption.palette.setBrush(QPalette::Normal, QPalette::Highlight, Qt::lightGray);
        else
        {
            myOption.palette.setBrush(QPalette::Normal, QPalette::Highlight, QColor(lBackgrounBrush.color()).darker(150));
        }
    }

    //draw the cell with myOption:
    QStyledItemDelegate::paint(painter, myOption, index);
}

