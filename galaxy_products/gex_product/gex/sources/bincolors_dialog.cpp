#include <qtabwidget.h>
#include <qimage.h>
#include <qmenu.h>
#include <qcolordialog.h>
#include <qmessagebox.h>
#include "message.h"
#include "browser_dialog.h"
#include "report_build.h"
#include "report_options.h"
#include "cbinning.h"
#include "bincolors_dialog.h"
#include "report_classes_sorting.h"	// Classes to sort lists
#include "gex_group_of_files.h"
#include "gex_report.h"
#include "gex_shared.h"
#include "product_info.h"
#include "engine.h"

// External definitions for pointers to pixmaps.
#include "gex_pixmap_extern.h"

// in main.cpp
extern GexMainwindow *	pGexMainWindow;

// report_build.cpp
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)
extern CGexReport* gexReport;			// Handle to report class

bool CompareBinning2(CBinning* bin1, CBinning* bin2)
{
    return(bin1->ldTotalCount < bin2->ldTotalCount);
}

///////////////////////////////////////////////////////////
// Display dialog box to edit Binning colors (for bin charts & wafer maps)
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_BinColors(void)
{
    // Enable/disable some features...
    bool bRefuse = false;

    if (GS::LPPlugin::ProductInfo::getInstance()->isOEM())
        bRefuse = true;

    // Reject Drill?
    if(bRefuse)
    {
    QString m=ReportOptions.GetOption("messages", "upgrade").toString();
    GS::Gex::Message::information("", m);
        return;
    }

    // Popup Color palette editor
    BinColorsDialog cBinColors;
    if(cBinColors.exec() != 1)
        return;

    pGexMainWindow->OnOptionChanged();

    // Color changed, then check if have to rebuild report pages...
    int iStatus = QMessageBox::question(pGexMainWindow,
        "Bin Color Changed",
        "You've edited the Bin colors and your profile has been updated.\n\n"
        "You will have to save your profile to keep these changes.\n\n"
        "Do you want the report to be rebuilt?",
        QMessageBox::Yes  | QMessageBox::Default,
        QMessageBox::No  | QMessageBox::Escape);

    // check if user confirms that pages have to be rebuilt.
    if(iStatus != QMessageBox::Yes)
        return;

    // Yes: then rebuild pages now + reload visible HTML page if any (no data analysis required)!
    gexReport->RebuildReport();
}

///////////////////////////////////////////////////////////
// Constructor: Binning color management dialog box
BinColorsDialog::BinColorsDialog( QWidget* parent, bool modal, Qt::WindowFlags f )
    : QDialog( parent, f )
{
    setupUi(this);
    setModal(modal);

    // Apply Examinator palette
    GexMainwindow::applyPalette(this);

    // Update check box (defines is use Examinator colors, or custom colors)
    if(ReportOptions.bUseCustomBinColors)
        radioButtonCustom->setChecked(true);	// Create custom Bin colors
    else
        radioButtonDefault->setChecked(true);	// Use default colors

    // Have GUI updated according to the check-box status
    OnUseDefaultColors();

    // Fill list with existing binnings & colors.
    FillBinningList();

    // At startup, focus is on SoftBin list. Make sure we have all variables set accordingly.
    OnTabChange(0);

    // Connections
    connect(buttonNewSoftBin,				SIGNAL( clicked() ), this, SLOT( OnAddBin() ) );
    connect(buttonNewHardBin,				SIGNAL( clicked() ), this, SLOT( OnAddBin() ) );
    connect(buttonNewSite,                  SIGNAL( clicked() ), this, SLOT( OnAddBin() ) );

    connect(buttonSoftBinProperties,		SIGNAL( clicked() ), this, SLOT( OnEditBin() ) );
    connect(buttonHardBinProperties,		SIGNAL( clicked() ), this, SLOT( OnEditBin() ) );
    connect(buttonSiteProperties,           SIGNAL( clicked() ), this, SLOT( OnEditBin() ) );

    connect(ListWidgetSoftBin,				SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(OnEditBin()) );
    connect(ListWidgetHardBin,				SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(OnEditBin()) );
    connect(ListWidgetSite,                 SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(OnEditBin()) );

    connect(buttonRemoveSoftBin,			SIGNAL( clicked() ), this, SLOT( OnDeleteBin() ) );
    connect(buttonRemoveHardBin,			SIGNAL( clicked() ), this, SLOT( OnDeleteBin() ) );
    connect(buttonRemoveSite,           	SIGNAL( clicked() ), this, SLOT( OnDeleteBin() ) );

    connect(PushButtonOk,					SIGNAL( clicked() ), this, SLOT( OnOk() ) );
    connect(tabWidgetBins,					SIGNAL( currentChanged(int) ), this, SLOT( OnTabChange(int) ) );
    connect(pushButtonFillDefaultColors,	SIGNAL( clicked() ), this, SLOT( OnFillWithDefaultColors() ) );
    connect(PushButtonCancel,				SIGNAL( clicked() ), this, SLOT( reject() ) );

    connect(radioButtonDefault,             SIGNAL( clicked(bool) ), this, SLOT( OnUseDefaultColors() ));
    connect(radioButtonCustom,              SIGNAL( clicked(bool) ), this, SLOT( OnUseDefaultColors() ));
}


///////////////////////////////////////////////////////////
// Fill List (Soft or Hard Bins) & colors
///////////////////////////////////////////////////////////
void BinColorsDialog::AddToList(QListWidget* listWidget, QList<CBinColor> &listColor, CGexGroupOfFiles *group, bool displayColorPerSite)
{

    m_ListWidget = listWidget;
    QList<CBinColor>::iterator itBegin	= listColor.begin();
    QList<CBinColor>::iterator itEnd	= listColor.end();

    int lSiteMax = 0;
    int lSiteNumber = 1;
    if (group)
    {
        lSiteMax = group->cMergedData.GetNbSites();
        if (group->cMergedData.GetSitesList().contains(0))
        {
            lSiteNumber = 0;
            --lSiteMax;
        }
    }

    while(itBegin != itEnd)
    {
        // Get BinColor info and add to GUI list
        if(displayColorPerSite)
            AddBinToList(QString("%1").arg(lSiteNumber), (*itBegin).cBinColor);
        else
            AddBinToList((*itBegin).cBinRange->GetRangeList(),(*itBegin).cBinColor);

        // Move to next Bin Color entry
        ++itBegin;
        if(displayColorPerSite)
        {
            ++lSiteNumber;
            if(group && lSiteNumber > lSiteMax)
                break;
        }
    };
}

///////////////////////////////////////////////////////////
// Fill List (Soft or Hard Bins) & colors
///////////////////////////////////////////////////////////
void BinColorsDialog::FillBinningList(void)
{
    // First: erase GUI list content in case old colors remain.
    ListWidgetSoftBin->clear();
    ListWidgetHardBin->clear();
    ListWidgetSite->clear();

    // Second: fill the list!
    CGexGroupOfFiles *	pGroup		= (gexReport->getGroupsList().isEmpty()) ? NULL : gexReport->getGroupsList().at(0);

    AddToList(ListWidgetSoftBin, ReportOptions.softBinColorList, pGroup, false);
    AddToList(ListWidgetHardBin, ReportOptions.hardBinColorList, pGroup, false);
    AddToList(ListWidgetSite,  ReportOptions.siteColorList, pGroup, true);

    OnTabChange(0);
}

///////////////////////////////////////////////////////////
// Fill custom color list with current defautl colors
///////////////////////////////////////////////////////////
void BinColorsDialog::OnFillWithDefaultColors()
{
    if(gexReport == NULL)
        return;

    // Get handle to relevant group
    int iGroupID=0;
    CGexGroupOfFiles *	pGroup		= (gexReport->getGroupsList().isEmpty()) ? NULL : gexReport->getGroupsList().at(iGroupID);

    if (pGroup)
    {
        CBinning *			ptBinCell;	// Pointer to Bin cell

        // Create the list of Binning counts
        // Working on SOFT bins
        ListWidgetSoftBin->clear();
        m_ListWidget	= ListWidgetSoftBin;
        ptBinCell		= pGroup->cMergedData.ptMergedSoftBinList;

        FillCustomBinList(ptBinCell);

        // Working on HARD bins
        ListWidgetHardBin->clear();
        m_ListWidget = ListWidgetHardBin;
        ptBinCell= pGroup->cMergedData.ptMergedHardBinList;

        FillCustomBinList(ptBinCell);

        //-- Working on Site
        ListWidgetSite->clear();
        m_ListWidget = ListWidgetSite;

        int lSiteMax = pGroup->cMergedData.GetNbSites();
        int indexSite = 1;
        if (pGroup->cMergedData.GetSitesList().contains(0))
        {
            indexSite = 0;
            --lSiteMax;
        }
        for(; indexSite <= lSiteMax; ++indexSite)
        {
            AddBinToList(QString("%1").arg(indexSite), gexReport->cDieColor.assignBinColor(indexSite, (indexSite%2==0)?true:false));
        }
    }
    else
    {
        QMap<int, QColor>::const_iterator itBegin	= gexReport->cDieColor.mapDefaultBinColors().constBegin();
        QMap<int, QColor>::const_iterator itEnd		= gexReport->cDieColor.mapDefaultBinColors().constEnd();

        // Clear current widget
        ListWidgetSoftBin->clear();
        ListWidgetHardBin->clear();

        int lSiteNumber = 0;
        while (itBegin != itEnd)
        {
            // Add it to Soft Bin list
            m_ListWidget	= ListWidgetSoftBin;
            AddBinToList(QString::number(itBegin.key()), itBegin.value());

            // Add it to Hard Bin list
            m_ListWidget	= ListWidgetHardBin;
            AddBinToList(QString::number(itBegin.key()), itBegin.value());

             // Add it to Site list
            m_ListWidget	= ListWidgetSite;

            AddBinToList(QString("%1").arg(lSiteNumber), itBegin.value());
            ++lSiteNumber;

            ++itBegin;
        }
    }
    OnTabChange(0);
}

void BinColorsDialog::FillCustomBinList(CBinning * ptBinCell)
{
    if (ptBinCell)
    {
        qtTestListBinning	qtBinningList;

        while(ptBinCell != NULL)
        {
            qtBinningList.append(ptBinCell);
            ptBinCell = ptBinCell->ptNextBin;
        };

        // Sort list of binning
        qSort(qtBinningList.begin(), qtBinningList.end(), CompareBinning2);

        QString			strBinList;
        QColor			cColor;

        ptBinCell = qtBinningList.last();

        for(int indexBinCell=qtBinningList.count(); indexBinCell>0; --indexBinCell)
        {
            ptBinCell = qtBinningList[indexBinCell-1];
            cColor		= gexReport->cDieColor.assignBinColor(ptBinCell->iBinValue, (bool)(ptBinCell->cPassFail == 'P'));
            strBinList	= QString::number(ptBinCell->iBinValue);

            AddBinToList(strBinList, cColor);
        }
    }
}

///////////////////////////////////////////////////////////
// Select if use default colors, or custom colors.
///////////////////////////////////////////////////////////
void BinColorsDialog::OnUseDefaultColors()
{
    if(radioButtonCustom->isChecked())
        ReportOptions.bUseCustomBinColors = true;	// Create custom Bin colors
    else
        ReportOptions.bUseCustomBinColors = false;	// Use default colors

    // Readback ComboBox selection
  /*  if(comboBoxBinColor->currentIndex())
        ReportOptions.bUseCustomBinColors = true;	// Create custom Bin colors
    else
        ReportOptions.bUseCustomBinColors = false;	// Use default colors*/

    // Update GUI based on the check-box status
    tabWidgetBins->setEnabled(ReportOptions.bUseCustomBinColors);
    if(ReportOptions.bUseCustomBinColors)
        pushButtonFillDefaultColors->setEnabled(true);
    else
        pushButtonFillDefaultColors->setEnabled(false);
}

///////////////////////////////////////////////////////////
// Add bin to list
///////////////////////////////////////////////////////////
void BinColorsDialog::OnTabChange(int lIndex)
{
    // Keeps track of which Bin list we focus on
    if(lIndex == 0)
    {
        // Get handle to SoftBin list (GUI)
        m_ListWidget = ListWidgetSoftBin;

        // Update comment string about Bin type
        m_strBinType = "Software Bin";
    }
    else if(lIndex == 1)
    {
        // Get handle to HardBin list (GUI)
        m_ListWidget = ListWidgetHardBin;

        // Update comment string about Bin type
        m_strBinType = "Hardware Bin";
    }
    else
    {
        // Get handle to HardBin list (GUI)
        m_ListWidget = ListWidgetSite;

        // Update comment string about Bin type
        m_strBinType = "Site";
    }
}

///////////////////////////////////////////////////////////
// Add/replace bin into GUI list
///////////////////////////////////////////////////////////
void BinColorsDialog::AddBinToList(QString strBinList,
                                   QColor cColor,
                                   QListWidgetItem *cSelection/*=NULL*/)
{
    // Read Bin range & color and add it to the Bin list GUI.
    QListWidgetItem *ptItem = cSelection;
    if(cSelection == NULL)
    {
        ptItem = new QListWidgetItem(m_ListWidget);
        m_ListWidget->insertItem(m_ListWidget->count(), ptItem);
    }

    // Create pixmap to show Bin color
    QPixmap pixmap(17,17);
    pixmap.fill(cColor);

    // Save text
    ptItem->setText(strBinList);
    ptItem->setIcon(pixmap);
}

///////////////////////////////////////////////////////////
// Add bin to list
///////////////////////////////////////////////////////////
void BinColorsDialog::OnAddBin(void)
{
    EditBinEntryDialog cEditBinColor(m_strBinType);

    // User Aborts clor edit.
    if(cEditBinColor.exec() == -1)
        return;

    // Add selected BinList & color to the GUI list.
    AddBinToList(cEditBinColor.GetBinList(),cEditBinColor.GetColor());
}

///////////////////////////////////////////////////////////
// Edit color of selected bin or bin double-clicked.
///////////////////////////////////////////////////////////
void BinColorsDialog::OnEditBin(void)
{
    // If no item selected, quietly return!
    QListWidgetItem *cSelection = m_ListWidget->currentItem();
    if(cSelection == NULL)
        return;

    // Retrieve color assigned to this selection
    QPixmap pix = cSelection->icon().pixmap(QSize(17,17));
    QImage cImage = pix.toImage();	// Copy pixmap into image so to read its color.
    QColor cColor = cImage.pixel(5,5);

    // Display Properties dialog box to edit the Color & Bin list.
    EditBinEntryDialog cEditBinColor(m_strBinType,cSelection->text(),cColor);

    // User Aborts ignore edit.
    if(cEditBinColor.exec() == -1)
        return;

    // Replace selection with new BinList & color to the GUI list.
    AddBinToList(cEditBinColor.GetBinList(),cEditBinColor.GetColor(),cSelection);
}

///////////////////////////////////////////////////////////
// Delete bins selected
///////////////////////////////////////////////////////////
void BinColorsDialog::OnDeleteBin(void)
{
    // If no item selected, quietly return!
    QListWidgetItem *cSelection = m_ListWidget->currentItem();
    if(cSelection == NULL)
        return;

    // Delete selection
    m_ListWidget->takeItem(m_ListWidget->currentRow());

    // Destroy it from memory.
    delete cSelection;
}

///////////////////////////////////////////////////////////
// Save the content of a GUI Bin list into Examiantor's memory.
///////////////////////////////////////////////////////////
void BinColorsDialog::SaveColorList(QListWidget * pListWidget,QList<CBinColor> &pBinList)
{
    QImage		cImage;
    QColor		cColor;

    // Empty Examinator's list as we change it with what's on the GUI...
    pBinList.clear();

    // If no item selected, quietly return!
    QListWidgetItem *ptItem = NULL;

    for( int nRow = 0; nRow < pListWidget->count(); nRow++)
    {
        ptItem = pListWidget->item(nRow);

        // Retrieve color assigned to this selection
        cImage = ptItem->icon().pixmap(QSize(17,17)).toImage();	// Copy pixmap into image so to read its color.
        cColor = cImage.pixel(5,5);

        // Save Bin list & color GUI item into Examinator internal list
        CBinColor cBinColor;
        cBinColor.cBinRange = new GS::QtLib::Range(ptItem->text().toLatin1().constData());
        cBinColor.cBinColor = cColor;
        pBinList.append(cBinColor);
    };
}

///////////////////////////////////////////////////////////
// 'Ok' button clicked, Save Colors edits...
///////////////////////////////////////////////////////////
void BinColorsDialog::OnOk()
{
    // Save the Soft-Bin GUI list into Examinator's memory so
    // to be saved into the user configuration file.
    SaveColorList(ListWidgetSoftBin,ReportOptions.softBinColorList);

    // Save the Hard-Bin GUI list into Examinator's memory so
    // to be saved into the user configuration file.
    SaveColorList(ListWidgetHardBin,ReportOptions.hardBinColorList);

    // Save the Site list into Examinator's memory so
    // to be saved into the user configuration file.
    SaveColorList(ListWidgetSite,ReportOptions.siteColorList);
    done(1);
}

///////////////////////////////////////////////////////////
// Constructor (Edit a specific Bin color entry)
EditBinEntryDialog::EditBinEntryDialog(QString strBinType, QString strBinList,
     QColor cColor, QWidget* parent, bool modal, Qt::WindowFlags f )
    : QDialog( parent, f )
{
    setupUi(this);
    setModal(modal);

    // Apply Examinator palette
    GexMainwindow::applyPalette(this);

    QObject::connect(PushButtonCancel,		SIGNAL(clicked()), this, SLOT(reject()));
    QObject::connect(PushButtonOk,			SIGNAL(clicked()), this, SLOT(OnOk()));

    // Fill GUI fields
    TextLabelBinType->setText(strBinType);	// Tells Bin type edited: "Software bin" or "Hardware bin"
    lineEditBinList->setText(strBinList);	// Bin list
    pushButtonEditColor->setActiveColor(cColor);	// Bin color assigned

    // Set focus on Bin range string
    lineEditBinList->setFocus();
}

///////////////////////////////////////////////////////////
// Return color selected
///////////////////////////////////////////////////////////
QColor EditBinEntryDialog::GetColor(void)
{
    return (pushButtonEditColor->activeColor());
}

///////////////////////////////////////////////////////////
// Return Bin list selected
///////////////////////////////////////////////////////////
QString EditBinEntryDialog::GetBinList(void)
{
    return lineEditBinList->text();
}

///////////////////////////////////////////////////////////
// 'Ok' button clicked, check if valid info entered
///////////////////////////////////////////////////////////
void EditBinEntryDialog::OnOk()
{
    QString strBinList = lineEditBinList->text();
    strBinList = strBinList.trimmed();
    if(strBinList.isEmpty() == true)
    {
        GS::Gex::Message::warning("", "You must specify a Bin, or a Bin list!");
        return;
    }
    done(1);
}
