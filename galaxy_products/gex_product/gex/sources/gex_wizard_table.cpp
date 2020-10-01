#include <gqtl_log.h>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QClipboard>
#include <QSplitter>
#include "loading.h"
#include "drill_table.h"
#include "drill_parametric_table.h"
#include "drill_device_table.h"
#include "gex_scriptengine.h"
#include "gex_report.h"
#include "gex_shared.h"
#include "gex_pixmap_extern.h"
#include "browser_dialog.h"
#include "engine.h"
#include "message.h"
#include "picktest_dialog.h"
#include "product_info.h"
#include "cpart_info.h"
#include "ofr_controller.h"
#include <QColorDialog>
#include <QMovie>
#include <QScrollBar>
//#include <QProgressBar>

extern GexScriptEngine* pGexScriptEngine;
//extern QProgressBar	*	GexProgressBar;	// Handle to progress bar in status bar
extern CGexReport *gexReport; // Handle to report class
extern GexMainwindow *	pGexMainWindow;
extern const char * const cUnitColumnLabel;


///////////////////////////////////////////////////////////
// Dialog box Constructor
///////////////////////////////////////////////////////////
GexWizardTable::GexWizardTable
  ( QWidget* parent, bool modal, Qt::WindowFlags fl) :mCurrentDrillTable(NULL),
  GS::Gex::QTitleFor< QDialog >( parent, fl)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "new Gex Wizard Table...");
    setupUi(this);
    setModal(modal);
    // in order to be used through scripting
    setObjectName("GSInteractiveTables");
      mIndex = -1;
    if (pGexScriptEngine)
    {
        QScriptValue lSV = pGexScriptEngine->newQObject((QObject*) this);
        if (!lSV.isNull())
            pGexScriptEngine->globalObject().setProperty("GSInteractiveTables", lSV);
    }

    // apply gex palette
    GexMainwindow::applyPalette(this);

    mTablesLoaded = false;

    InitInteractiveView();
    QObject::connect(buttonFilterParameters,					SIGNAL(clicked(bool)),
                     this,					SLOT(OnFilterParameters(bool)));
    QObject::connect(buttonExportHardBinning,					SIGNAL(clicked()),
                     this,					SLOT(OnSaveHardBinning()));
    QObject::connect(buttonExportSoftBinning,					SIGNAL(clicked()),
                     this,					SLOT(OnSaveSoftBinning()));
    QObject::connect(comboBoxDevice,							SIGNAL(activated(int)),
                     this,					SLOT(OnChangeDeviceResults(int)));
    QObject::connect(m_poCBGroupPartResult,                     SIGNAL(currentIndexChanged(int)),
                     this,                   SLOT(updatePartResultComboBox(int)));
    QObject::connect(buttonExportDeviceResults,					SIGNAL(clicked()),
                     this,					SLOT(OnSaveDeviceResults()));
    QObject::connect(buttonExportExcelClipboardHardBin,			SIGNAL(clicked()),
                     this,					SLOT(OnExportExcelClipboard()));
    QObject::connect(buttonExportExcelClipboardSoftBin,			SIGNAL(clicked()),
                     this,					SLOT(OnExportExcelClipboard()));
    QObject::connect(buttonExportExcelClipboardStats,			SIGNAL(clicked()),
                     this,					SLOT(OnExportExcelClipboard()));
    QObject::connect(buttonExportParametricTests,				SIGNAL(clicked()),
                     this,					SLOT(OnSaveParametricTests()));
    QObject::connect(buttonExportFunctionalTests,				SIGNAL(clicked()),
                     this,					SLOT(OnSaveFunctionalTests()));
    QObject::connect(checkBoxSortOnAbsoluteValue,				SIGNAL(toggled(bool)),
                     this,					SLOT(OnChangeSortOnAbsoluteValue(bool)));
    QObject::connect(buttonExportExcelClipboardFunctional,		SIGNAL(clicked()),
                     this,					SLOT(OnExportExcelClipboard()));
    QObject::connect(buttonExportExcelClipboardDeviceResults,	SIGNAL(clicked()),
                     this,					SLOT(OnExportExcelClipboard()));
    QObject::connect(gexReport,									SIGNAL(interactiveFilterChanged(bool)),
                     this,					SLOT(onFilterChanged(bool)));
    QObject::connect(comboBoxType,                              SIGNAL(currentIndexChanged(int)),
                     this,                   SLOT(updateTypeResultComboBox(int)));

    /*QObject::connect(buttonFindAllTests,                  SIGNAL(clicked()),  this,           SLOT(ShowFindBox()));
    QObject::connect(buttonFindFunctionalTests,           SIGNAL(clicked()),  this,           SLOT(ShowFindBox()));
    QObject::connect(buttonFindSoftBin,                   SIGNAL(clicked()),  this,           SLOT(ShowFindBox()));
    QObject::connect(buttonFindHardBin,                   SIGNAL(clicked()),  this,           SLOT(ShowFindBox()));
    QObject::connect(buttonFindDeviceResults,             SIGNAL(clicked()),  this,           SLOT(ShowFindBox()));*/
    QObject::connect(tabWidgetTables,                     SIGNAL(tabBarClicked(int)), this,   SLOT(ChangeTabBar(int)));
    QObject::connect(pGexMainWindow,                      SIGNAL(sShortcut()), this,          SLOT(ShowFindBox()));

    // Disable some functions under unix.
    #if (defined __sun__ || __hpux__)
        buttonExportExcelClipboardStats->hide();
        buttonExportExcelClipboardSoftBin->hide();
        buttonExportExcelClipboardHardBin->hide();
    #endif

    // Check Parameter filter
    buttonFilterParameters->setChecked(gexReport->hasInteractiveTestFilter());
    checkBoxHideFunctionalTests->setChecked(QSettings().value("interactive/tables/hideFunctionalTests").toBool());

    mLoading = new Loading();
    mWizardHandler = 0;
    // Create tables.
    mTableParametricTests = new DrillParametricTable(this, tabParametrics, mLoading);
    mTableParametricTests->SetSortOnAbsoluteValue(checkBoxSortOnAbsoluteValue->isChecked());
    mTableParametricTests->SetHideFunctionalTests(checkBoxHideFunctionalTests->isChecked());
    hBoxLayoutParametrics->addWidget(mTableParametricTests);


    mTableFunctionalTests = new DrillFunctionalTable(this, tabFunctional, mLoading);
    hBoxLayoutFunctional->addWidget(mTableFunctionalTests);

    mTableSoftBinning = new DrillBinningTable(this, tabSoftBin, mLoading);
    hBoxLayoutSoftware->addWidget(mTableSoftBinning);

    mTableHardBinning = new DrillBinningTable(this, tabHardBin, mLoading);
    hBoxLayoutHardware->addWidget(mTableHardBinning);


    mMaxDevicesByRun = 1;

    UpdateDevicesByRun();
    QSplitter *lSplitter = new QSplitter();
    lSplitter->setOrientation(Qt::Vertical);
    gsint8 lSubLotIndex = -1;
    for (int lDevicesByRun=0; lDevicesByRun<mMaxDevicesByRun; ++lDevicesByRun)
    {
        DrillDeviceWidget * lDeviceWidget = new DrillDeviceWidget(this);

        if(mMaxDevicesByRun == 1)
            lSubLotIndex = lDevicesByRun;

        lDeviceWidget->GetDeviceTable()->setMinimumHeight(150);
        lDeviceWidget->GetDeviceTable()->SetSubLotIndex(lSubLotIndex);
        mTableDeviceWidgets.append(lDeviceWidget);
        lSplitter->addWidget(lDeviceWidget);
        lDeviceWidget->hide();
    }
    for (int i = 0; i < mTableDeviceWidgets.size(); ++i)
    {
        for (int j = 0; j < mTableDeviceWidgets.size(); ++j)
        {
            if (i != j)
            {
                connect(mTableDeviceWidgets.at(i)->GetDeviceTable()->horizontalScrollBar(), SIGNAL(valueChanged(int)),
                        mTableDeviceWidgets.at(j)->GetDeviceTable()->horizontalScrollBar(), SLOT(setValue(int)));
            }
        }
    }

    scrollAreaPartResults->setWidget(lSplitter);


    // Connect signal for 'contextual menu'
    connect(mTableParametricTests,          SIGNAL(customContextMenuRequested(const QPoint &)),
                             this,          SLOT(OnContextualMenu( const QPoint &)));
    connect(checkBoxHideFunctionalTests,	SIGNAL(toggled(bool)),
                  mTableParametricTests,	SLOT(onHideFunctionalTests(bool)));
    connect(mTableFunctionalTests,			SIGNAL(customContextMenuRequested(QPoint)),
                             this,			SLOT(OnContextualMenu( const QPoint &)));
    connect(mTableSoftBinning,				SIGNAL(customContextMenuRequested(const QPoint &)),
                         this,              SLOT(OnContextualMenu( const QPoint &)));
    connect(mTableHardBinning,				SIGNAL(customContextMenuRequested(const QPoint &)),
                         this,				SLOT(OnContextualMenu( const QPoint &)));

    for (int i=0; i<mTableDeviceWidgets.size(); ++i)
    {
        connect(mTableDeviceWidgets[i]->GetDeviceTable(),   SIGNAL(customContextMenuRequested(const QPoint &)),
                this,                                       SLOT(OnContextualMenu( const QPoint &)));
        connect(checkBoxHideTestsNoResults,	SIGNAL(toggled(bool))                             ,
                mTableDeviceWidgets[i]->GetDeviceTable(),   SLOT(OnHideTestsWithNoResults(bool)));
    }

    // User can select multiple cells in a same column.

    mTableParametricTests->setSelectionBehavior( QAbstractItemView::SelectRows);
    mTableFunctionalTests->setSelectionMode( QAbstractItemView::SingleSelection);
    mTableSoftBinning->setSelectionMode( QAbstractItemView::SingleSelection);
    mTableHardBinning->setSelectionMode( QAbstractItemView::SingleSelection);
    for (int lDevicesByRun=0; lDevicesByRun<mMaxDevicesByRun; ++lDevicesByRun)
    {
        if (mTableDeviceWidgets[lDevicesByRun])
            mTableDeviceWidgets[lDevicesByRun]->GetDeviceTable()->setSelectionBehavior( QAbstractItemView::SelectRows);
    }

    // Enable/disable some features...
    switch(GS::LPPlugin::ProductInfo::getInstance()->getProductID())
    {
        case GS::LPPlugin::LicenseProvider::eSzOEM:			// OEM-Examinator for Credence SZ
            buttonExportExcelClipboardStats->hide();
            buttonExportExcelClipboardFunctional->hide();
            buttonExportExcelClipboardSoftBin->hide();
            buttonExportExcelClipboardHardBin->hide();
            buttonExportExcelClipboardDeviceResults->hide();
        break;

        case GS::LPPlugin::LicenseProvider::eLtxcOEM:			// OEM-Examinator for LTXC
            {
                buttonExportExcelClipboardStats->hide();
                buttonExportParametricTests->hide();
                buttonExportExcelClipboardFunctional->hide();
                buttonExportFunctionalTests->hide();
                buttonExportExcelClipboardSoftBin->hide();
                buttonExportSoftBinning->hide();
                buttonExportExcelClipboardHardBin->hide();
                buttonExportHardBinning->hide();
                buttonExportExcelClipboardDeviceResults->hide();
                buttonExportDeviceResults->hide();

                // Disable 'Part-Result tab'
                tabWidgetTables->setTabEnabled(tabWidgetTables->indexOf(tabWidgetTables->widget(4)),false);	// Disable 'Part-Result' tab

                // Credence requested backround to be white...
                QPalette palette;
                palette.setColor(backgroundRole(), Qt::white);
                setPalette(palette);
                break;
            }
        default:
            break;
    }

    //connect(this, SIGNAL(customContextMenuRequested(QPoint), this, SLOT())
   connect(mTableParametricTests,   SIGNAL(pressed(QModelIndex)), this, SLOT(OnTableClicked(QModelIndex)));
   connect(mTableFunctionalTests,   SIGNAL(pressed(QModelIndex)), this, SLOT(OnTableClicked(QModelIndex)));
   connect(mTableSoftBinning,       SIGNAL(pressed(QModelIndex)), this, SLOT(OnTableClicked(QModelIndex)));
   connect(mTableHardBinning,       SIGNAL(pressed(QModelIndex)), this, SLOT(OnTableClicked(QModelIndex)));

   connect(mTableParametricTests,   SIGNAL(clicked(QModelIndex)), this, SLOT(OnTableClicked(QModelIndex)));
   connect(mTableFunctionalTests,   SIGNAL(clicked(QModelIndex)), this, SLOT(OnTableClicked(QModelIndex)));
   connect(mTableSoftBinning,       SIGNAL(clicked(QModelIndex)), this, SLOT(OnTableClicked(QModelIndex)));
   connect(mTableHardBinning,       SIGNAL(clicked(QModelIndex)), this, SLOT(OnTableClicked(QModelIndex)));

   mTableParametricTests->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
   connect(mTableParametricTests->horizontalHeader(),   SIGNAL(customContextMenuRequested(const QPoint&)), this,  SLOT(OnRightCLicked(const QPoint&)));

   mTableFunctionalTests->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
   connect(mTableFunctionalTests->horizontalHeader(),   SIGNAL(customContextMenuRequested(const QPoint&)), this,  SLOT(OnRightCLicked(const QPoint&)));

   mTableSoftBinning->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
   connect(mTableSoftBinning->horizontalHeader(),       SIGNAL(customContextMenuRequested(const QPoint&)), this,  SLOT(OnRightCLicked(const QPoint&)));

   mTableHardBinning->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
   connect(mTableHardBinning->horizontalHeader(),       SIGNAL(customContextMenuRequested(const QPoint&)), this,  SLOT(OnRightCLicked(const QPoint&)));

   for (int lDevicesByRun=0; lDevicesByRun<mMaxDevicesByRun; ++lDevicesByRun) {
       if (mTableDeviceWidgets[lDevicesByRun]) {
           mTableDeviceWidgets[lDevicesByRun]->GetDeviceTable()->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
           connect(mTableDeviceWidgets[lDevicesByRun]->GetDeviceTable()->horizontalHeader(),SIGNAL(customContextMenuRequested(const QPoint&)), this,SLOT(OnRightCLicked(const QPoint&)));
           connect(mTableDeviceWidgets[lDevicesByRun]->GetDeviceTable(), SIGNAL(clicked(QModelIndex)), this, SLOT(SelectItems(QModelIndex)));
       }
   }



    mFindDialog= new FindDialog();
    gridLayout->addWidget(mFindDialog);


    connect(mFindDialog->lineEdit,
            SIGNAL(textChanged(QString)), this, SLOT(FindTxt(QString)));
    connect(mFindDialog->pushButtonPrevious,    SIGNAL(clicked()),              this, SLOT( PreviousItem( ) ));
    connect(mFindDialog->pushButtonNext,        SIGNAL(clicked()),              this, SLOT( NextItem( ) ));
    connect(mFindDialog->lineEdit,              SIGNAL(returnPressed()),        this, SLOT( NextItem( ) ));
    connect(mFindDialog,                        SIGNAL(finished(int)),          this, SLOT( FindBoxFinished(int)));
    //connect(mFindDialog->pushButtonWheelColor,  SIGNAL(clicked(bool)),          this, SLOT( SelectionColor()));

    connect(mFindDialog,                        SIGNAL(SelectionColor(QColor)),  this, SLOT(SelectionColor(QColor)));

    //mFindDialog->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint |  Qt::CustomizeWindowHint  | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint);
    RazSearch();
    mSelectionMode =false;
    mCurrentIndexTable = tabWidgetTables->currentIndex();
    mCurrentDrillTable = mTableParametricTests;

    //mColorSelection = Qt::yellow;

    ApplySelectionStylesheet();
    mCurrentDrillTable->clearSelection();

}

GexWizardTable::~GexWizardTable()
{
    // Make this option persistent through session.
    QSettings settings;
    mDevicesByRun.clear();
    mDevicesInComboBox.clear();
    mItemsPartsComboBox.clear();
    settings.setValue("interactive/tables/hideFunctionalTests", checkBoxHideFunctionalTests->isChecked());
    delete mLoading;

    mWizardHandler->RemoveWizardTable(this);
}

///////////////////////////////////////////////////////////
// Close signal: If window is detached, this reparent it instead
///////////////////////////////////////////////////////////
void GexWizardTable::closeEvent(QCloseEvent *event)
{
    if(pGexMainWindow!=NULL)
    {
        event->accept();

        mWizardHandler->RemoveWizardTable(this);
    }

}

///////////////////////////////////////////////////////////
// Make page visible.
///////////////////////////////////////////////////////////
void GexWizardTable::ShowPage(void)
{
    // restore window if it is minimized
    if (isMinimized())
        setWindowState(windowState() & ~Qt::WindowMinimized);
    else {
        // Show page
        show();
        setFocus();
    }
}

///////////////////////////////////////////////////////////
// Called each time the new Query & script is executed: resets flags
///////////////////////////////////////////////////////////
void GexWizardTable::clear(void)
{
    // Clear variable, so next time page must be shown, all tables will be reloaded.
    mTablesLoaded = false;
}

///////////////////////////////////////////////////////////
// Contextual menu Table clicked
//////////////////////////////////////////////////////////////////////
void GexWizardTable::OnContextualMenu(const QPoint& pos)
{
    QAction *	pActionStatsChartHistogram	= NULL;
    QAction *	pActionStatsChartTrend		= NULL;
    QAction *	pActionWafermap				= NULL;

    QString		strSelection;
    QIcon	icSave;
    QIcon	icExcel;
    QIcon	icHistogram;
    QIcon	icTrend;
    QIcon	icWafermap;
    icHistogram.addPixmap(*pixAdvHisto);
    icTrend.addPixmap(*pixTrend);
    icWafermap.addPixmap(*pixWafermap);
    icSave.addPixmap(*pixSave);
    icExcel.addPixmap(*pixCSVSpreadSheet);

    QString		strTestNumber;
    int			nGroup	= 0;
    QMenu       menu(this);
    QTableWidgetItem* lItem = NULL;
    QModelIndexList lSelectedRows;

    // Check which Table we're sorting: Statistics, Binning...?
    int lTable = tabWidgetTables->currentIndex();
    // Build floating menu.
    int	iTotalSelections=0;
    switch(lTable)
    {
        case 0:	// Parametric Test (statistics)
            {
                if (!GS::LPPlugin::ProductInfo::getInstance()->isExaminator())
                {
                     QItemSelectionModel* aItSelMod = mTableParametricTests->selectionModel();
                     if(aItSelMod != NULL)
                         lSelectedRows = aItSelMod->selectedRows();
                     QMenu* lMenuTableType = new QMenu();
                     lMenuTableType->setIcon(QIcon(":/gex/icons/ofr_icon.png"));
                     if(GS::Gex::OFR_Controller::GetInstance()->IsReportEmpty())
                     {

                        lMenuTableType->setTitle("Start a report builder session");
                     }
                     else
                     {
                         lMenuTableType->setTitle("Add to the report builder session");
                     }

                     lMenuTableType->addAction("Add a capability table", this,
                                               SLOT(OnAddCapabilityTablenOnReportBuilder()));
                     lMenuTableType->addAction("Add the current statistic table",	this,
                                               SLOT(OnAddStatisticTablenOnReportBuilder()));

                     menu.addMenu(lMenuTableType) ;
                     menu.addSeparator();
                }

                QTableWidgetItem* item = mTableParametricTests->itemAt(pos.x(), pos.y());
                if(item)
                {
                    int row = item->row();

                    bool								bFunctionalTestsSelected = false;
                    QString								strGroupName;
                    QListIterator<CGexGroupOfFiles*>	itGroupsList(gexReport->getGroupsList());
                    CGexGroupOfFiles *					pGroup	= NULL;
                    CGexFileInGroup *					pFile	= NULL;

                    strTestNumber	= mTableParametricTests->item(row,0)->text();
                    strGroupName	= mTableParametricTests->item(row,2)->text();

                    while (itGroupsList.hasNext())
                    {
                        pGroup = itGroupsList.next();

                        if (pGroup->strGroupName == strGroupName)
                        {
                            pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
                            break;
                        }

                        nGroup++;
                    }

                    // Save Test list (test# of selected lines (in column 0).)
                    for(row=0;row<mTableParametricTests->rowCount();row++)
                    {
                        // Only dump visible columns that are SELECTED!
                        lItem=mTableParametricTests->item(row, 0);
                        if(lItem && lItem->isSelected())
                        {
                            if (mTableParametricTests->item(row,
                                 mTableParametricTests->ColPosition(TEST_TYPE))->text() == "F")
                                bFunctionalTestsSelected = true;

                            // Keep track of total selections
                            iTotalSelections++;

                            // Save test#
                            strSelection += mTableParametricTests->item(row,0)->text();
                            strSelection += ";";
                        }
                    }

                    // Check if multiple selection...
                    if(iTotalSelections > 1)
                        strSelection = "Test_List " + strSelection;

                    // Link to Histogram, trend and wafermap only activated when test type is Parametric or Generic
                    // Interactive charts don't support functional test
                    if (bFunctionalTestsSelected == false)
                    {
                        // Build menu.
                        pActionStatsChartHistogram	= menu.addAction(icHistogram,	"Histogram Chart");
                        pActionStatsChartTrend		= menu.addAction(icTrend,		"Trend Chart");

                        if (pFile && pFile->getWaferMapData().bWirExists)
                            pActionWafermap		= menu.addAction(icWafermap,	"Wafermap");

                        menu.addSeparator();
                    }
                }
            }
            break;

        case 4: // Device results
            {
                if ((mTableDeviceWidgets.size() > 0) && mTableDeviceWidgets[0])
                {
                    DrillDeviceTable * lDeviceTable = mTableDeviceWidgets[0]->GetDeviceTable();
                    QTableWidgetItem * item         = lDeviceTable->itemAt(pos.x(), pos.y());

                    if(item)
                    {
                        int row = item->row();

                        bool				bFunctionalTestsSelected	= false;
                        CGexGroupOfFiles *	pGroup						= gexReport->getGroupsList().isEmpty()?NULL:gexReport->getGroupsList().first();
                        CGexFileInGroup *	pFile						= NULL;

                        strTestNumber	= lDeviceTable->item(row,0)->text();

                        if (pGroup)
                            pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

                        // Save Test list (test# of selected lines (in column 0).)
                        for(row=0;row<lDeviceTable->rowCount();row++)
                        {
                            // Only dump visible columns that are SELECTED!
                            lItem = lDeviceTable->item(row, 0);
                            if(lItem && lItem->isSelected())
                            {
                                if (lDeviceTable->item(row,
                                                       mTableParametricTests->ColPosition(TEST_TYPE))->text() == "F")
                                    bFunctionalTestsSelected = true;

                                // Keep track of total selections
                                iTotalSelections++;

                                // Save test#
                                strSelection += lDeviceTable->item(row,0)->text();
                                strSelection += ";";
                            }
                        }

                        // Check if multiple selection...
                        if(iTotalSelections > 1)
                            strSelection = "Test_List " + strSelection;

                        // Link to Histogram, trend and wafermap only activated when test type is Parametric or Generic
                        // Interactive charts don't support functional test
                        if (bFunctionalTestsSelected == false)
                        {
                            // Build menu.
                            pActionStatsChartHistogram	= menu.addAction(icHistogram,	"Histogram Chart");
                            pActionStatsChartTrend		= menu.addAction(icTrend,		"Trend Chart");

                            if (pFile && pFile->getWaferMapData().bWirExists)
                                pActionWafermap		= menu.addAction(icWafermap,	"Wafermap");

                            menu.addSeparator();
                        }
                    }
                }
            }
            break;


        case 1:	// Functional tests
            break;

        case 2:	// Software binning
        case 3: // Hardware binning
                CGexGroupOfFiles *	pGroup	= gexReport->getGroupsList().isEmpty()?NULL:gexReport->getGroupsList().first();
                CGexFileInGroup *	pFile	= (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
                if (pFile && pFile->getWaferMapData().bWirExists)
                {
                    pActionWafermap	= menu.addAction(icWafermap,	"Wafermap");
                    menu.addSeparator();
                }
            break;
    }

    // Enable/disable some features...
    QAction * pActionSaveToDisk					= NULL;
    QAction * pActionSaveToClipboard			= NULL;
    QAction * pActionSaveSelectionToClipboard	= NULL;

    switch(GS::LPPlugin::ProductInfo::getInstance()->getProductID())
    {
        case GS::LPPlugin::LicenseProvider::eLtxcOEM:       // OEM-Examinator for LTXC
        case GS::LPPlugin::LicenseProvider::eSzOEM:			// OEM-Examinator for Credence SZ
            break;

        default:

            // Allow to save to clipboard in spreadsheet CSV format
            pActionSaveToClipboard			= menu.addAction(icExcel,"Save FULL Table to clipboard (Spreadsheet format)");
            if (!lTable == 4 || !comboBoxType->currentText().contains("Die XY"))
                pActionSaveSelectionToClipboard = menu.addAction(icExcel,"Save selection to clipboard (Spreadsheet format)");
            // Allow to export to disk.
            pActionSaveToDisk				= menu.addAction(icSave,"Save Full Table to disk...");
        break;
    }

    menu.setMouseTracking(true);
    QAction * pActionResult = menu.exec(QCursor::pos());
    menu.setMouseTracking(false);

    if (pActionResult == NULL)
        return;

    // Show Binning wafermap
     if(pActionResult == pActionWafermap)
     {
        int nTestNumber	= 0;
        int nPinmap		= GEX_PTEST;
        switch(lTable)
        {
            case 0:	// Parametric test
            case 4: // Device results
                // If one '.' is present, extract test number and pinmap number
                if (strTestNumber.count('.') == 1)
                {
                    nTestNumber = strTestNumber.section('.', 0, 0).toInt();
                    nPinmap		= strTestNumber.section('.', 1, 1).toInt();
                }
                else
                    // Only test number is set.
                    nTestNumber = strTestNumber.section('.', 0, 0).toInt();
                break;

            case 1:	// Functional test
                return;
             break;

            case 2:	// Soft-Bin
             nGroup = 0;
             nTestNumber = GEX_TESTNBR_OFFSET_EXT_SBIN;
             break;
            case 3:	// Hard-Bin
             nGroup = 0;
             nTestNumber = GEX_TESTNBR_OFFSET_EXT_HBIN;
             break;
        }

        QString strLink = "#_gex_drill--drill_3d=wafer--g=";
        strLink += QString::number(nGroup);
        strLink += "--f=0--Test=";
        strLink += QString::number(nTestNumber);
        strLink += "--Pinmap=";
        strLink += QString::number(nPinmap);

        pGexMainWindow->Wizard_Drill3D(strLink, false);
        return;
     }

    // 'Save FULL table to Clipboard (spreadsheet format)'
    if(pActionResult == pActionSaveToClipboard)
        OnExportExcelClipboard();

    // 'Save selection Clipboard (spreadsheet format)'
    if(pActionResult == pActionSaveSelectionToClipboard)
        OnExportSelectionExcelClipboard(lSelectedRows);

    // 'Save to disk'
    if(pActionResult == pActionSaveToDisk)
    {
        if(lTable == 0)
            OnSaveParametricTests();
        else if(lTable == 1)
            OnSaveFunctionalTests();
        else if(lTable == 2)
            OnSaveSoftBinning();
        else if(lTable == 3)
            OnSaveHardBinning();
        else if(lTable == 4)
            OnSaveDeviceResults();
        return;
    }

    // 'Histogram Chart'
    if(pActionResult == pActionStatsChartHistogram)
    {
        // Build hyperlink argument string: drill_chart=adv_histo--data=<test#>
        QString strLink = "drill--drill_chart=adv_histo--data=";
        strLink += strSelection;
        pGexMainWindow->Wizard_DrillChart(strLink, false);
        return;
    }

    // 'Trend Chart'
    if(pActionResult == pActionStatsChartTrend)
    {
        // Build hyperlink argument string: drill_chart=adv_trend--data=<test#>
        QString strLink = "drill--drill_chart=adv_trend--data=";
        strLink += strSelection;
        pGexMainWindow->Wizard_DrillChart(strLink, false);
        return;
    }
}

void GexWizardTable::OnAddStatisticTablenOnReportBuilder()
{
    QJsonObject lElt;
    StatisticTableToJson(lElt);
    GS::Gex::OFR_Controller::GetInstance()->AddElementSettings(lElt);
}

void GexWizardTable::OnAddCapabilityTablenOnReportBuilder()
{
    QJsonObject lElt;
    CapabiltyTableToJson(lElt);
    GS::Gex::OFR_Controller::GetInstance()->AddElementSettings(lElt);
}

void GexWizardTable::StatisticTableToJson(QJsonObject& lElt)
{
    QJsonObject lSettings;

    // 'Test statistics' tab
    if (tabWidgetTables->currentIndex() == 0)
    {
        if (mTableParametricTests)
            mTableParametricTests->toJson(lSettings, false);
    }

    lSettings.insert("Name", QString("Statistic Table"));
    //-- export the setting into the builder element
    lElt.insert("Table", lSettings);

}

void GexWizardTable::CapabiltyTableToJson(QJsonObject& lElt)
{
    QJsonObject lSettings;

    // 'Test statistics' tab
    if (tabWidgetTables->currentIndex() == 0)
    {
        if (mTableParametricTests)
            mTableParametricTests->toJson(lSettings, true);
    }

    lSettings.insert("Name", QString("Capability Table"));
    //-- export the setting into the builder element
    lElt.insert("Table", lSettings);

}


///////////////////////////////////////////////////////////
// Switch to the Interactive Charts display
///////////////////////////////////////////////////////////
void GexWizardTable::OnInteractiveCharts(void)
{
    pGexMainWindow->Wizard_DrillChart("", false);
}



void GexWizardTable::OnSwitchInteractiveChart()
{
    pGexMainWindow->mWizardsHandler.SetWidgetToSwitch(this);
    OnInteractiveCharts();
}


void GexWizardTable::OnSwitchInteractiveWafermap()
{
    pGexMainWindow->mWizardsHandler.SetWidgetToSwitch(this);
    pGexMainWindow->mWizardsHandler.RemoveWizardTable(this);
    OnInteractiveWafermap();
}

void GexWizardTable::OnInteractiveTables()
{
    pGexMainWindow->Wizard_DrillTable("", false);
}

///////////////////////////////////////////////////////////
// Switch to the Interactive Wafermap display
///////////////////////////////////////////////////////////
void GexWizardTable::OnInteractiveWafermap(void)
{
    pGexMainWindow->Wizard_Drill3D("#_gex_drill--drill_3d=wafer_sbin--g=0--f=0", false);
}

///////////////////////////////////////////////////////////
// Check box for Sort mode changed
//////////////////////////////////////////////////////////////////////
void GexWizardTable::OnChangeSortOnAbsoluteValue(bool bSortOnAbsoluteValue)
{
    mTableParametricTests->SetSortOnAbsoluteValue(bSortOnAbsoluteValue);
}

///////////////////////////////////////////////////////////
// Tells if data available for charting.
///////////////////////////////////////////////////////////
bool GexWizardTable::isDataAvailable(void)
{
    if(gexReport == NULL)
        return false;

    // Get pointer to first group & first file
    CGexGroupOfFiles *pGroup = gexReport->getGroupsList().isEmpty()?NULL:gexReport->getGroupsList().first();
    if(pGroup == NULL)
        return false;

    if(pGroup->pFilesList.count() <= 0)
        return false;

    // Some data available.
    return true;
}

void GexWizardTable::ShowTable(QString strLink)
{
    buttonFilterParameters->hide();
    buttonExportParametricTests->hide();
    buttonExportExcelClipboardStats->hide();
    //buttonFindAllTests->hide();
    checkBoxSortOnAbsoluteValue->hide();
    checkBoxHideFunctionalTests->hide();

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Interactive tables: Show table '%1'").arg(strLink).toLatin1().data() );

    QString strStatisticsFieldsOptions = (ReportOptions.GetOption(QString("statistics"), QString("fields"))).toString();
    QStringList qslStatisticsFieldsList = strStatisticsFieldsOptions.split(QString("|"));

    QString strStatisticsCompareFieldsOptions
            = (ReportOptions.GetOption(QString("statistics"), QString("compare_fields"))).toString();
    QStringList qslStatisticsCompareFieldsList = strStatisticsCompareFieldsOptions.split(QString("|"));

    QString strStatisticsGageFieldsOptions
            = (ReportOptions.GetOption(QString("statistics"), QString("gage_fields"))).toString();
    QStringList qslStatisticsGageFieldsList = strStatisticsGageFieldsOptions.split(QString("|"));

    QString strStatisticsAdvFieldsOptions
            = (ReportOptions.GetOption(QString("statistics"), QString("adv_fields"))).toString();
    QStringList qslStatisticsAdvFieldsList = strStatisticsAdvFieldsOptions.split(QString("|"));

    // Enable/disable some features...
    switch(GS::LPPlugin::ProductInfo::getInstance()->getProductID())
    {
        case GS::LPPlugin::LicenseProvider::eLtxcOEM:			// OEM-Examinator for LTXC
            buttonExportParametricTests->hide();// Can't save Parametric Test statistics to disk
            buttonExportFunctionalTests->hide();// Can't save Functional Test statistics to disk
            buttonExportSoftBinning->hide();	// Can't save Soft Bin statistics to disk
            buttonExportHardBinning->hide();	// Can't save Hard Bin statistics to disk
            break;

        case GS::LPPlugin::LicenseProvider::eSzOEM:			// OEM-Examinator for Credence SZ
            break;
        default:
            break;
    }

    if((!strLink.isNull()) && (strLink.isEmpty() == false))
    {
        // Extract Table type to Drill into.
        // line is like:
        QString strString = strLink.section("--",1,1);	//= "drill_table=<type>"
        QString strTableType = strString.section('=',1,1);	// one of: "stats", "soft_bin", "hard_bin"...

        // Check which Table type is selected...
        if(strTableType == "stats")
        {
            // Show 'Parametric tests' tab
            tabWidgetTables->setCurrentIndex(0);
        }
        else
        if(strTableType == "func")
        {
            // Show 'Functional tests' tab
            tabWidgetTables->setCurrentIndex(1);
        }
        else
        if(strTableType == "soft_bin")
        {
            // Show 'Software Binning' tab
            tabWidgetTables->setCurrentIndex(2);
        }
        else
        if(strTableType == "hard_bin")
        {
            // Show 'Hardware Binning' tab
            tabWidgetTables->setCurrentIndex(3);
        }
        if(strTableType == "parts_results")
        {
            // Show 'Device Results' tab
            tabWidgetTables->setCurrentIndex(4);
        }
    }

    QTableWidget  * pActiveTable = NULL;

    switch(tabWidgetTables->currentIndex())
    {
        case 0	:	pActiveTable = mTableParametricTests;
                    break;

        case 1	:	pActiveTable = mTableFunctionalTests;
                    break;

        case 2	:	pActiveTable = mTableSoftBinning;
                    break;

        case 3	:	pActiveTable = mTableHardBinning;
                    break;

//        case 4	:	pActiveTable = mTableDeviceResults;
//                    break;

        default	:	break;

    }

    // Hide active table during customization to avoid flickering
    if (pActiveTable)
        pActiveTable->hide();

    if (!mTableParametricTests || !mTableFunctionalTests ||
            !mTableSoftBinning || !mTableHardBinning)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Invalid pointer to Drill Table!");
        return;
    }


    if(mTablesLoaded == false)
    {

        mLoading->start(200);

        mLoading->update(" Loading Tables...");
        mTableParametricTests->ResetTable();

        mLoading->update();
        mTableFunctionalTests->ResetTable();

        mLoading->update();
        mTableSoftBinning->ResetTable(true);

        mTableHardBinning->ResetTable(false);

        mLoading->update();

        if ((mTableDeviceWidgets.size() != 0) && mTableDeviceWidgets[0])
            mTableDeviceWidgets[0]->GetDeviceTable()->ResetTable(0);

        mLoading->update();
    }

    // Show-Hide relevant "Statistics" columns as defined in the "Options" page
    if(qslStatisticsFieldsList.contains(QString("test_name")))
    {
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_TNAME));
        mTableFunctionalTests->showColumn(DRILL_FTEST_TNAME);
    }
    else
    {
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_TNAME));
        mTableFunctionalTests->hideColumn(DRILL_FTEST_TNAME);
    }

    // Show Dataset name?
    if(ReportOptions.iGroups > 1)
    {
        // YES: We have multiple groups/datasets to compare
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_GROUP));
        mTableFunctionalTests->showColumn(DRILL_FTEST_GROUP);
        mTableSoftBinning->showColumn(DRILL_BIN_GROUP);
        mTableHardBinning->showColumn(DRILL_BIN_GROUP);
    }
    else
    {
        // NO: We only have one group/dataset.
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_GROUP));
        mTableFunctionalTests->hideColumn(DRILL_FTEST_GROUP);
        mTableSoftBinning->hideColumn(DRILL_BIN_GROUP);
        mTableHardBinning->hideColumn(DRILL_BIN_GROUP);
    }

    if(qslStatisticsFieldsList.contains(QString("test_type")))
    {
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_TYPE));
        mTableFunctionalTests->showColumn(DRILL_FTEST_TYPE);
    }
    else
    {
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_TYPE));
        mTableFunctionalTests->hideColumn(DRILL_FTEST_TYPE);
    }

    if(qslStatisticsFieldsList.contains(QString("limits")))
    {
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_LTL));
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_HTL));
    }
    else
    {
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_LTL));
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_HTL));
    }

    if(qslStatisticsFieldsList.contains(QString("spec_limits")))
    {
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_LSL));
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_HSL));
    }
    else
    {
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_LSL));
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_HSL));
    }
    if(qslStatisticsFieldsList.contains(QString("drift_limits")))
    {
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_DRIFTL));		// Drift limits
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_DRIFTLOW));	// Drift LOW limit side
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_DRIFTHIGH));	// Drift LOW limit side
    }
    else
    {
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_DRIFTL));
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_DRIFTLOW));
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_DRIFTHIGH));
    }

    if(qslStatisticsAdvFieldsList.contains(QString("shape")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_SHAPE));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_SHAPE));

    if(qslStatisticsFieldsList.contains(QString("stats_source")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_STATS_SRC));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_STATS_SRC));

    if(qslStatisticsFieldsList.contains(QString("test_flow_id")))
    {
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_TESTFLOWID));
        mTableFunctionalTests->showColumn(DRILL_FTEST_TESTFLOWID);
    }
    else
    {
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_TESTFLOWID));
        mTableFunctionalTests->hideColumn(DRILL_FTEST_TESTFLOWID);
    }

    if(qslStatisticsFieldsList.contains(QString("exec_count")))
    {
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_EXEC));
        mTableFunctionalTests->showColumn(DRILL_FTEST_EXEC);
    }
    else
    {
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_EXEC));
        mTableFunctionalTests->hideColumn(DRILL_FTEST_EXEC);
    }

    if(qslStatisticsFieldsList.contains(QString("fail_count")))
    {
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_FAIL));
        mTableFunctionalTests->showColumn(DRILL_FTEST_FAIL);
    }
    else
    {
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_FAIL));
        mTableFunctionalTests->hideColumn(DRILL_FTEST_FAIL);
    }

    if(qslStatisticsFieldsList.contains(QString("fail_percent")))
    {
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_FAILPERCENT));
        mTableFunctionalTests->showColumn(DRILL_FTEST_FAILPERCENT);
    }
    else
    {
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_FAILPERCENT));
        mTableFunctionalTests->hideColumn(DRILL_FTEST_FAILPERCENT);
    }

    if(qslStatisticsFieldsList.contains(QString("fail_bin")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_FAILBIN));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_FAILBIN));

    if(qslStatisticsFieldsList.contains(QString("removed_count")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_OUTLIER));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_OUTLIER));

    if(qslStatisticsFieldsList.contains(QString("mean")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_MEAN));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_MEAN));

    if((qslStatisticsCompareFieldsList.contains(QString("mean_shift"))) && (ReportOptions.iGroups > 1))
    {
        for (int i=0; i<gexReport->getGroupsList().size(); ++i)
        {
            if (gexReport->getGroupsList()[i])
                mTableParametricTests->showColumn(mTableParametricTests->
                                              ColPosition(TEST_MEANSHIFT + gexReport->getGroupsList()[i]->strGroupName));
        }
    }
    else
    {
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_MEANSHIFT+
                                                                             gexReport->getGroupsList()[0]->strGroupName));
    }

    if((qslStatisticsCompareFieldsList.contains(QString("t_test"))) && (ReportOptions.iGroups == 2))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_T_TEST));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_T_TEST));

    if(qslStatisticsFieldsList.contains(QString("sigma")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_SIGMA));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_SIGMA));

    if((qslStatisticsCompareFieldsList.contains(QString("sigma_shift"))) && (ReportOptions.iGroups > 1))
    {
        for (int i=0; i<gexReport->getGroupsList().size(); ++i)
        {
            if (gexReport->getGroupsList()[i])
                mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_SIGMASHIFT + gexReport->getGroupsList()[i]->strGroupName));
        }
    }
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_SIGMASHIFT+
                                                                             gexReport->getGroupsList()[0]->strGroupName));

    if(qslStatisticsFieldsList.contains(QString("2sigma")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_2_SIGMA));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_2_SIGMA));

    if(qslStatisticsFieldsList.contains(QString("3sigma")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_3_SIGMA));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_3_SIGMA));

    if(qslStatisticsFieldsList.contains(QString("6sigma")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_6_SIGMA));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_6_SIGMA));

    if(qslStatisticsFieldsList.contains(QString("min")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_MIN));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_MIN));

    if(qslStatisticsFieldsList.contains(QString("max")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_MAX));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_MAX));

    if(qslStatisticsFieldsList.contains(QString("range")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_RANGE));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_RANGE));

    if((qslStatisticsCompareFieldsList.contains(QString("max_range"))) && (ReportOptions.iGroups > 1))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_MAX_RANGE));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_MAX_RANGE));

    if(qslStatisticsFieldsList.contains(QString("cp")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_CP));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_CP));

    if((qslStatisticsCompareFieldsList.contains(QString("cp_shift"))) && (ReportOptions.iGroups > 1))
    {
        for (int i=0; i<gexReport->getGroupsList().size(); ++i)
        {
            if (gexReport->getGroupsList()[i])
                mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_CPSHIFT + gexReport->getGroupsList()[i]->strGroupName));
        }
    }
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_CPSHIFT+
                                                                             gexReport->getGroupsList()[0]->strGroupName));

    if(qslStatisticsFieldsList.contains(QString("cr")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_CR));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_CR));

    if((qslStatisticsCompareFieldsList.contains(QString("cr_shift"))) && (ReportOptions.iGroups > 1))
    {
        for (int i=0; i<gexReport->getGroupsList().size(); ++i)
        {
            if (gexReport->getGroupsList()[i])
                mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_CRSHIFT + gexReport->getGroupsList()[i]->strGroupName));
        }
    }
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_CRSHIFT+
                                                                             gexReport->getGroupsList()[0]->strGroupName));

    if(qslStatisticsFieldsList.contains(QString("cpk")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_CPK));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_CPK));

    if(qslStatisticsAdvFieldsList.contains(QString("cpk_l")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_CPKL));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_CPKL));

    if(qslStatisticsAdvFieldsList.contains(QString("cpk_h")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_CPKH));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_CPKH));

    if((qslStatisticsCompareFieldsList.contains(QString("cpk_shift"))) && (ReportOptions.iGroups > 1))
    {
        for (int i=0; i<gexReport->getGroupsList().size(); ++i)
        {
            if (gexReport->getGroupsList()[i])
                mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_CPKSHIFT + gexReport->getGroupsList()[i]->strGroupName));
        }
    }
    else
    {
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_CPKSHIFT+
                                                                             gexReport->getGroupsList()[0]->strGroupName));
    }

    if(qslStatisticsFieldsList.contains(QString("yield")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_YIELD));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_YIELD));

    if(qslStatisticsGageFieldsList.contains(QString("ev")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_GAGE_EV));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_GAGE_EV));

    if(qslStatisticsGageFieldsList.contains(QString("av")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_GAGE_AV));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_GAGE_AV));

    if(qslStatisticsGageFieldsList.contains(QString("r&r")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_GAGE_RR));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_GAGE_RR));

    if(qslStatisticsGageFieldsList.contains(QString("gb")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_GAGE_GB));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_GAGE_GB));

    if(qslStatisticsGageFieldsList.contains(QString("pv")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_GAGE_PV));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_GAGE_PV));

    if(qslStatisticsGageFieldsList.contains(QString("tv")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_GAGE_TV));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_GAGE_TV));

    if(qslStatisticsGageFieldsList.contains(QString("p_t")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_GAGE_P_T));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_GAGE_P_T));

    // Advanced Statistics
    if(qslStatisticsAdvFieldsList.contains(QString("skew")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_SKEW));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_SKEW));

    if(qslStatisticsAdvFieldsList.contains(QString("kurtosis")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_KURTOSIS));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_KURTOSIS));

    if(qslStatisticsAdvFieldsList.contains(QString("P0.5")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_P0_5));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_P0_5));

    if(qslStatisticsAdvFieldsList.contains(QString("P2.5")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_P2_5));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_P2_5));

    if(qslStatisticsAdvFieldsList.contains(QString("P10")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_P10));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_P10));

    if(qslStatisticsAdvFieldsList.contains(QString("quartile1")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_Q1));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_Q1));

    if(qslStatisticsAdvFieldsList.contains(QString("quartile2")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_Q2));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_Q2));

    if(qslStatisticsAdvFieldsList.contains(QString("quartile3")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_Q3));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_Q3));

    if(qslStatisticsAdvFieldsList.contains(QString("P90")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_P90));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_P90));

    if(qslStatisticsAdvFieldsList.contains(QString("P97.5")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_P97_5));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_P97_5));

    if(qslStatisticsAdvFieldsList.contains(QString("P99.5")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_P99_5));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_P99_5));

    if(qslStatisticsAdvFieldsList.contains(QString("inter_quartiles")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_IQR));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_IQR));

    if(qslStatisticsAdvFieldsList.contains(QString("sigma_inter_quartiles")))
        mTableParametricTests->showColumn(mTableParametricTests->ColPosition(TEST_SIGMAIQR));
    else
        mTableParametricTests->hideColumn(mTableParametricTests->ColPosition(TEST_SIGMAIQR));


//    mTableParametricTests->resizeRowsToContents();
//    mTableParametricTests->resizeColumnsToContents();

//    mTableFunctionalTests->resizeRowsToContents();
//    mTableFunctionalTests->resizeColumnsToContents();

//    mTableSoftBinning->resizeRowsToContents();
//    mTableSoftBinning->resizeColumnsToContents();

//    mTableHardBinning->resizeRowsToContents();
//    mTableHardBinning->resizeColumnsToContents();

//    for (int i=0; i<mTableDeviceWidgets.size(); ++i)
//    {
//        if (mTableDeviceWidgets[i])
//        {
//            mTableDeviceWidgets[i]->GetDeviceTable()->resizeRowsToContents();
//            mTableDeviceWidgets[i]->GetDeviceTable()->resizeColumnsToContents();
//        }
//    }

    // All operations finished, show the table
    if (pActiveTable)
    {
        pActiveTable->show();
    }

    // Set flag
    mTablesLoaded = true;

    buttonFilterParameters->show();
    buttonExportParametricTests->show();
    buttonExportExcelClipboardStats->show();
    //buttonFindAllTests->show();
    checkBoxSortOnAbsoluteValue->show();
    checkBoxHideFunctionalTests->show();

    mLoading->stop();

}

///////////////////////////////////////////////////////////
// Filter the parameters listed in the Test Statistics table.
///////////////////////////////////////////////////////////
void GexWizardTable::OnFilterParameters(bool bState)
{
    if (GS::LPPlugin::ProductInfo::getInstance()->getProductID() == GS::LPPlugin::LicenseProvider::eLtxcOEM)
    {
        // OEM-Examinator for LTX
        QString m = ReportOptions.GetOption("messages", "upgrade").toString();

        GS::Gex::Message::information("", m);
        buttonFilterParameters->setChecked(false);
        return;
    }

    if (bState)
    {
        // Show TestList
        PickTestDialog	dialogPickTest;

        // Allow/Disable Multiple selections.
        dialogPickTest.setMultipleSelection(true);
        dialogPickTest.setMultipleGroups(false, false);

        // Check if List was successfuly loaded
        if(dialogPickTest.fillParameterList() && dialogPickTest.exec() == QDialog::Accepted)
            // Get test# selected. string format: <Test#>.<Pinmap#> , <Test#>.<Pinmap#> , etc...
            gexReport->setInteractiveTestFilter(dialogPickTest.testItemizedList());
        else
            gexReport->clearInteractiveTestFilter();
    }
    else
        // Remove test filter
        gexReport->clearInteractiveTestFilter();
}

///////////////////////////////////////////////////////////
// Update view with parameter list
///////////////////////////////////////////////////////////
void GexWizardTable::onFilterChanged(bool /*bState*/)
{
    // To avoid flickering
    mTableParametricTests->hide();
//    mTableDeviceResults->hide();

    mTableParametricTests->ResetTable();
//    mTableDeviceResults->ResetTable();

    // To avoid flickering
    mTableParametricTests->show();
//    mTableDeviceResults->show();

    buttonFilterParameters->setChecked(gexReport->hasInteractiveTestFilter());
}

///////////////////////////////////////////////////////////
// Save the Statistics table into CSV file
///////////////////////////////////////////////////////////
void GexWizardTable::OnSaveParametricTests(void)
{
    QString strTitle = gexReport->getReportOptions()->strReportTitle;
    if(strTitle.isEmpty())
        strTitle = "Data";
    strTitle += "_Statistics.csv";

    ExportToExcel(strTitle,(QTableWidget  *) mTableParametricTests);
}

void GexWizardTable::OnTabRenamed( QWidget *widget, const QString &text )
{
  // base class valid conversion, allow pointer comparison as each slot is
  // triggered while tab is renamed
  QWidget *that = static_cast< QWidget * >( this );

  if( that == widget )
  {
    setTitle( text );
    setCustomTitleFlag();
  }
}

///////////////////////////////////////////////////////////
// Save the Functional tests table into CSV file
///////////////////////////////////////////////////////////
void	GexWizardTable::OnSaveFunctionalTests(void)
{
    QString strTitle = gexReport->getReportOptions()->strReportTitle;
    if(strTitle.isEmpty())
        strTitle = "Data";
    strTitle += "_Functional_Tests.csv";

    ExportToExcel(strTitle,(QTableWidget  *) mTableFunctionalTests);
}

///////////////////////////////////////////////////////////
// Save the Soft Binning table into CSV file
///////////////////////////////////////////////////////////
void	GexWizardTable::OnSaveSoftBinning(void)
{
    QString strTitle = gexReport->getReportOptions()->strReportTitle;
    if(strTitle.isEmpty())
        strTitle = "Data";
    strTitle += "_Software_Binning.csv";

    ExportToExcel(strTitle,(QTableWidget  *) mTableSoftBinning);
}

///////////////////////////////////////////////////////////
// Save the Hard Binning table into CSV file
///////////////////////////////////////////////////////////
void	GexWizardTable::OnSaveHardBinning(void)
{
    QString strTitle = gexReport->getReportOptions()->strReportTitle;
    if(strTitle.isEmpty())
        strTitle = "Data";
    strTitle += "_Hardware_Binning.csv";

    ExportToExcel(strTitle,(QTableWidget  *) mTableHardBinning);
}

///////////////////////////////////////////////////////////
// Save the Device Results table into CSV file
///////////////////////////////////////////////////////////
void	GexWizardTable::OnSaveDeviceResults(void)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "On Save Device Results...");
    QString lFileTitle = gexReport->getReportOptions()->strReportTitle;
    if(lFileTitle.isEmpty())
        lFileTitle = "Data";
    lFileTitle += "_Device_Results.csv";

    if (comboBoxType->currentText().contains("Die XY"))
    {
        QFile lFile;
        if (!OpenFile(lFileTitle, lFile))
            return;
        for (int i=0; i<mTableDeviceWidgets.size(); ++i)
        {
            if (mTableDeviceWidgets[i])
            {
                WriteInExcelFile(lFile,(QTableWidget*) mTableDeviceWidgets[i]->GetDeviceTable());
            }
        }
        // Close file.
        lFile.close();
    }
    else
        if ((mTableDeviceWidgets.size() > 0) && mTableDeviceWidgets[0])
            ExportToExcel(lFileTitle,(QTableWidget*) mTableDeviceWidgets[0]->GetDeviceTable());
}

void GexWizardTable::ExportToExcel(QString strCsvFile, QTableWidget *table)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Export to CSV '%1'").arg(strCsvFile).toLatin1().data() );

    QString strMessage;
    QFile lFile;
    if (OpenFile(strCsvFile, lFile))
        WriteInExcelFile(lFile, table);
    else
        return;

    // Close file.
    lFile.close();

//    strMessage = "Table saved!\nFile created: " + strFile;
//    GS::Gex::Message::information("", strMessage);
}


bool GexWizardTable::OpenFile(QString csvFileName, QFile& file)
{
    if (GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_EVALUATION)
    {
        // Evaluation mode, refuse to run this function!
        GS::Gex::Message::information(
            "", "This function is disabled in Evaluation mode\n\nContact " +
            QString(GEX_EMAIL_SALES) + " for more information!");
        return false;
    }

    // Let's user tell where to save the table.
    QString lFileName = QFileDialog::getSaveFileName(this,
                    "Save Table data as...",
                    csvFileName,
                    "Spreadsheet CSV(*.csv)",
                    NULL,
                    QFileDialog::DontConfirmOverwrite);

    // If no file selected, ignore command.
    if(lFileName.isEmpty())
        return false;

    // Check if file exists.
    file.setFileName(lFileName);
    if(file.exists() == true)
    {
        // File exists...overwrite it?
        bool lOk;
        GS::Gex::Message::request("", "File already exists. Overwrite it ?", lOk);
        if (! lOk)
        {
            return false;
        }
    }

    // Create file
    if(!file.open(QIODevice::WriteOnly))
    {
        // Error. Can't create CSV file!
        GS::Gex::Message::information(
            "", "Failed to create CSV data file\n"
            "Disk full or write protection issue.");
        return false;
    }

    return true;
}

void GexWizardTable::WriteInExcelFile(QFile& file, QTableWidget *table)
{
    // Assign file I/O stream
    QTextStream hCsvTableFile;
    QString strCell;

    long	lLine,lCol;
    hCsvTableFile.setDevice(&file);

    // Write as first data line the Query names.
    hCsvTableFile << GS::Gex::Engine::GetInstance().Get("AppFullName").toString() << endl ;
    hCsvTableFile << "www.mentor.com" << endl ;
    hCsvTableFile << endl << endl;

    // Write Header titles
    for(lCol=0;lCol<table->columnCount();lCol++)
    {
        // Only save columns that are visible (not hidden).
        if(table->isColumnHidden(lCol) == false)
        {
            hCsvTableFile << table->horizontalHeaderItem(lCol)->text() << "," ;
            if(table->horizontalHeaderItem(lCol)->data(Qt::UserRole).toString() == DrillTable::sUnitColumnLabel)
            {
                hCsvTableFile << DrillTable::sUnitColumnLabel << "," ;
            }
        }
    }
    hCsvTableFile << endl;

    // Write table content
    for(lLine=0;lLine<table->rowCount();lLine++)
    {
        // Write cell string.
        for(lCol=0;lCol<table->columnCount();lCol++)
        {
            // Only dump visible columns!
            if(table->isColumnHidden(lCol) == false)
            {
                // If not empty cell
                if (table->item(lLine,lCol))
                {
                    strCell = table->item(lLine,lCol)->text();

                    if(table->item(lLine,lCol)->data(Qt::UserRole).toString() == DrillTable::sUnitColumnLabel)
                    {
                        QStringList lCellContent = strCell.simplified().split(" ",QString::SkipEmptyParts);
                        if(lCellContent.count() == 2)
                        {
                            hCsvTableFile << lCellContent[0] << "," << lCellContent[1] << ",";
                        }
                        else
                        {
                            hCsvTableFile << strCell << ",,";
                        }
                    }
                    else
                        hCsvTableFile << strCell << ",";
                }
                else
                    hCsvTableFile << ",";
            }
        }

        // Add new line character
        hCsvTableFile << endl;
    }
}

///////////////////////////////////////////////////////////
// Save FULL table into clipboard, ready to paste to spreadsheet CSV file (Windows only)
///////////////////////////////////////////////////////////
void GexWizardTable::OnExportExcelClipboard(void)
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

    QString		strMessage;
    QTableWidget  *	table = NULL;
    QString		lBigLine("");

    switch(tabWidgetTables->currentIndex())
    {
        case 0:	// 'Parametric tests' tab
            table = mTableParametricTests;
            break;
        case 1:	// 'Functional tests' tab
            table = mTableFunctionalTests;
            break;
        case 2:	// 'Software Bin' tab
            table = mTableSoftBinning;
            break;
        case 3:	// 'Hardware Bin' tab
            table = mTableHardBinning;
            break;
        case 4:	// 'Device Results' tab
            table = mTableDeviceWidgets[0]->GetDeviceTable();
            break;
        default:
            return;
    }

    if (tabWidgetTables->currentIndex() == 4 && comboBoxType->currentText().contains("Die XY"))
    {
        for (int i=0; i<mTableDeviceWidgets.size(); ++i)
        {
            if (mTableDeviceWidgets[i] && mTableDeviceWidgets[i]->GetDeviceTable())
            {
                WriteTableInString(lBigLine, mTableDeviceWidgets[i]->GetDeviceTable());
                lBigLine += "\n\n\n";
            }
        }
    }
    else
    {
        WriteTableInString(lBigLine, table);
    }

    // Copy data to clipboard
    QClipboard *cb = QGuiApplication::clipboard();
    cb->setText(lBigLine,QClipboard::Clipboard );


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
            ReportOptions.GetOption("preferences", "ssheet_editor").toString().toLatin1().data(),
           NULL,
           NULL,
           SW_SHOWNORMAL);
#else
    if (system(ReportOptions.GetOption("preferences", "ssheet_editor").
                   toString().toLatin1().constData()) != EXIT_SUCCESS)
    {
    }
#endif

#endif
}


void GexWizardTable::WriteTableInString(QString& lBigLine, QTableWidget* table)
{

    QString		strCell;
    long		lLine,lCol,lMaxCol;
    // Write as first data line the Query names.
    for(lCol=0;lCol<table->columnCount();lCol++)
    {
        // Write cell string (visible columns only)
        if(table->isColumnHidden(lCol) == false)
        {
            lBigLine += table->horizontalHeaderItem(lCol)->text();
            if(table->horizontalHeaderItem(lCol)->data(Qt::UserRole).toString() == DrillTable::sUnitColumnLabel)
            {
                lBigLine += QString("\t%1").arg(DrillTable::sUnitColumnLabel);
            }
            lBigLine += "\t";
        }
    }
    lBigLine += "\n";

    for(lLine=0;lLine<table->rowCount();lLine++)
    {
        // Ignore all leading empty cells on each line
        lMaxCol = table->columnCount();
        while(lMaxCol > 0)
        {
            if (table->item(lLine,lMaxCol-1))
            {
                strCell = (table->item(lLine,lMaxCol-1)->text()).trimmed();
                if(strCell.isEmpty() == false)
                    break;	// Reached the last cell that is not blank.
            }
            lMaxCol--;
        }


        // Write cell string.
        for(lCol=0;lCol<lMaxCol;lCol++)
        {
            // Only dump visible columns!
            if(table->isColumnHidden(lCol) == false)
            {
                // Write Data , Units
                if (table->item(lLine,lCol))
                {
                    strCell = table->item(lLine,lCol)->text();
                    if(table->item(lLine,lCol)->data(Qt::UserRole).toString() == DrillTable::sUnitColumnLabel)
                    {
                        QStringList lCellContent = strCell.simplified().split(" ",QString::SkipEmptyParts);
                        // If the cell contains the result and the unit
                        if(lCellContent.count() > 1)
                        {
                            lBigLine += lCellContent[0] + "\t";
                            for (int lUnitCell=1; lUnitCell<lCellContent.size(); ++lUnitCell)
                                lBigLine += lCellContent[lUnitCell];
                            // Add the tabulation in the end of the cell
                            lBigLine += "\t";
                        }
                        else
                        {
                            lBigLine += strCell + "\t\t";
                        }
                    }
                    else
                    {
                        // Export to clipboard in XL format:
                        lBigLine += strCell;
                        lBigLine += "\t";
                    }
                }
                else
                    // Export empty cell into clipboard in XL format:
                    lBigLine += "\t";
            }
        }
        lBigLine += "\n";
    }
}

///////////////////////////////////////////////////////////
// Save selection into clipboard, ready to paste to spreadsheet file (Windows only)
///////////////////////////////////////////////////////////
void GexWizardTable::OnExportSelectionExcelClipboard(const QModelIndexList& aSelectedRows)
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
    long	lLine,lCol;
    QTableWidget  *table=NULL;
    QString lBigLine;

    switch(tabWidgetTables->currentIndex())
    {
        case 0:	// 'Test statistics' tab
            table = mTableParametricTests;
                        //FIXME: not used ?
                        //bStats = true;
            break;
        case 1:	// 'Functional tests' tab
            table = mTableFunctionalTests;
            break;
        case 2:	// 'Software Bin' tab
            table = mTableSoftBinning;
            break;
        case 3:	// 'Hardware Bin' tab
            table = mTableHardBinning;
            break;
        case 4:	// 'Device Results' tab
            table = mTableDeviceWidgets[0]->GetDeviceTable();
            break;
    }

    // Write as first data line the Query names.
    for(lCol=0;lCol<table->columnCount();lCol++)
    {
        // Write cell string (visible columns only)
        if(table-> isColumnHidden(lCol) == false)
        {
            lBigLine += table->horizontalHeaderItem(lCol)->text();
            if(table->horizontalHeaderItem(lCol)->data(Qt::UserRole).toString() == DrillTable::sUnitColumnLabel)
            {
                lBigLine += QString("\t%1").arg(DrillTable::sUnitColumnLabel);
            }
            lBigLine += "\t";
        }
    }
    lBigLine += "\n";

    // Write cell string.
    bool	bValidLine = false;
    QModelIndexList::const_iterator it = aSelectedRows.begin();
    for(; it != aSelectedRows.end(); it++)
    {
        lLine = it->row();
        for(lCol=0;lCol<table->columnCount();lCol++)
        {
            if (table->isColumnHidden(lCol) == false)
            {
                // Write Data , Units
                if (table->item(lLine,lCol))
                {
                    strCell = table->item(lLine,lCol)->text();
                    if(table->item(lLine,lCol)->data(Qt::UserRole).toString() == DrillTable::sUnitColumnLabel)
                    {
                        QStringList lCellContent = strCell.simplified().split(" ",QString::SkipEmptyParts);
                        if(lCellContent.count() == 2)
                        {
                            lBigLine += lCellContent[0] + "\t" + lCellContent[1] +"\t";
                        }
                        else
                        {
                            lBigLine += strCell + "\t\t";
                        }
                    }
                    else
                    {
                        // Export to clipboard in XL format:
                        lBigLine += strCell;
                        lBigLine += "\t";
                    }
                }
                else
                    // Export empty cell into clipboard in XL format:
                    lBigLine += "\t";

                // Flag we've written fields on this line
                bValidLine = true;
            }
        }
        if(bValidLine)
            lBigLine += "\n";
    }

    // Copy data to clipboard
    QClipboard *cb = QGuiApplication::clipboard();
    cb->setText(lBigLine,QClipboard::Clipboard );


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
           ReportOptions.GetOption("preferences", "ssheet_editor").toString().toLatin1().constData(),
           NULL,
           NULL,
           SW_SHOWNORMAL);
#else
    if (system(ReportOptions.GetOption("preferences", "ssheet_editor").
          toString().toLatin1().constData()) == -1) {
      //FIXME: send error
    }
#endif
#else
   (void) aSelectedRows;
#endif
}

void GexWizardTable::OnShowDeviceResults(long lRunOffset, int x/*=-32768*/, int y/*=-32768*/)
{
    // Force to select given run#
    if (comboBoxType->currentText().contains("Die XY"))
    {
        QString lCurrentItemText = "";
//        if (lRunOffset < mDevicesByRun.size())
//        {
            // Find coordinate corresponding to the
            if (x==-32768 || y==-32768)
            {
                QList< QPair<gsint16,gsint16> > lKeys = mDevicesByRun.keys();
                for (int i=0; i<lKeys.size(); ++i)
                {
                    if (mDevicesByRun.value(lKeys[i]).contains(lRunOffset))
                    {
                        lCurrentItemText = QString::number(lKeys[i].first) + "." + QString::number(lKeys[i].second);
                    }
                }
            }
            else
            {
                lCurrentItemText = QString::number(x) + "." + QString::number(y);
            }
//        }
        comboBoxDevice->setCurrentText(lCurrentItemText);
    }
    else
    {
        comboBoxDevice->setCurrentIndex(comboBoxDevice->findData(QVariant((int)lRunOffset)));
    }



    // Ensure 'Device results' tab is active.
    tabWidgetTables->setCurrentIndex(4);

    // Detach this Window so we can see side-by-side the Other  window
   /* if(mChildWindow)
    {
        ForceAttachWindow(false);

        // Force showing back the Interactive chart page
        pGexMainWindow->ShowWizardDialog(GEX_TABLE_WIZARD_P1);
    }*/

    OnChangeDeviceResults(lRunOffset);
}


void GexWizardTable::OnChangeDeviceResults(int )
{
    for (int lTable=0; lTable<mTableDeviceWidgets.size(); ++lTable)
        mTableDeviceWidgets[lTable]->hide();
    if (comboBoxType->currentText().contains("Die XY"))
    {
        // Find the part and get the coordinate
        QString lCurrentItem = comboBoxDevice->currentText();
        QPair<gsint16,gsint16> lCoord = QPair<gsint16,gsint16>( lCurrentItem.section(".", 0, 0).trimmed().toInt(),
                                                                lCurrentItem.section(".", 1, 1).trimmed().toInt());
        QList<gsint32> lDevices = mDevicesByRun.value(lCoord);
        for (int i=0; i<lDevices.size(); ++i)
            OnGetDeviceResults(i);
    }
    else
    {
        OnGetDeviceResults(-1);
    }
}
///////////////////////////////////////////////////////////
// Signal to load Device Results table with results for
// selected run#.
///////////////////////////////////////////////////////////
void GexWizardTable::OnGetDeviceResults(int lSubLotIndex/*=-1*/)
{
    if (comboBoxType->currentText().contains("Die XY"))
    {
        QString lDevice = comboBoxDevice->currentText();
        gsint32 lIndex;
        QList<gsint32> lDevices;
        if (!lDevice.contains(".") || lDevice.contains("Part"))
            lIndex = comboBoxDevice->itemData(comboBoxDevice->currentIndex()).toInt();
        else
        {
            QPair<gsint16,gsint16> lCoord = QPair<gsint16,gsint16>(lDevice.section(".", 0, 0).trimmed().toInt(),
                                                               lDevice.section(".", 1, 1).trimmed().toInt());
            lDevices = mDevicesByRun.value(lCoord);
            if (lSubLotIndex >= 0 && lSubLotIndex<lDevices.size())
                lIndex = lDevices[lSubLotIndex];
            else
                return;
            GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("OnGetDeviceResults device coordinate %1").arg(lDevice)
                  .toLatin1().data());
        }

        QString lRes;
        if ((mTableDeviceWidgets.size() > lSubLotIndex) && mTableDeviceWidgets[lSubLotIndex])
        {
            lRes=mTableDeviceWidgets[lSubLotIndex]->GetDeviceTable()->LoadDeviceResults(lIndex);

            mTableDeviceWidgets[lSubLotIndex]->show();

            RazSelectionColumn();
            InitCurrentDrillTable();
            if(!mCurrentText.isEmpty()) {
                InitListItemsResearched();
                SetSelectedItem();
            }

        }
        else
        {
            GSLOG(SYSLOG_SEV_ERROR, "error: Null TableDeviceResults");
            return;
        }
        if (lRes.startsWith("error"))
            GS::Gex::Message::warning("Interactive Table", QString("Failed to Load device result for offset %1: %2")
                                      .arg(lDevices[lSubLotIndex]).arg(lRes));
    }
    else
    {
        // Get run# selected
        long lRunOffset = comboBoxDevice->itemData(comboBoxDevice->currentIndex()).toInt();

        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("OnGetDeviceResults run offset %1").arg(lRunOffset)
              .toLatin1().data());

        // Display results for this run#
        QString lRes = "error: Null TableDeviceResults";
        if ((mTableDeviceWidgets.size() > 0) && mTableDeviceWidgets[0])
        {
            lRes=mTableDeviceWidgets[0]->GetDeviceTable()->LoadDeviceResults(lRunOffset);

            mTableDeviceWidgets[0]->show();
        }
        if (lRes.startsWith("error"))
            GS::Gex::Message::warning("Interactive Table",
                                      QString("Failed to Load device result for offset %1: %2").arg(lRunOffset).arg(lRes) );
    }
}

void GexWizardTable::updatePartResultComboBox(int )
{
    CGexGroupOfFiles *pGroup = gexReport->getGroupsList()[ m_poCBGroupPartResult->itemData(
                                                       m_poCBGroupPartResult->currentIndex()).toInt()];
    CGexFileInGroup *pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
    comboBoxDevice->clear();
    if(!comboBoxType->currentText().contains("Die XY") && (mTableDeviceWidgets.size() > 0) && mTableDeviceWidgets[0])
        mTableDeviceWidgets[0]->GetDeviceTable()->updatePartResultComboBox(pGroup, pFile);
    else
    {
        for (int i=0; i<mTableDeviceWidgets.size(); ++i)
            mTableDeviceWidgets[i]->GetDeviceTable()->updatePartResultComboBox(pGroup, pFile,i);
    }

}

void GexWizardTable::updateTypeResultComboBox(int )
{
    for (int lTable=0; lTable<mTableDeviceWidgets.size(); ++lTable)
        mTableDeviceWidgets[lTable]->hide();
    comboBoxDevice->clear();
    mItemsPartsComboBox.clear();
    QString lCurrentItem = comboBoxType->currentText();
    if (lCurrentItem.contains("run", Qt::CaseInsensitive) && (mTableDeviceWidgets.size() > 0) && mTableDeviceWidgets[0])
        mTableDeviceWidgets[0]->GetDeviceTable()->ResetTable(-1);
    else
    {
        for (int i=0; i<mTableDeviceWidgets.size(); ++i)
            mTableDeviceWidgets[i]->GetDeviceTable()->ResetTable(i);
    }
}


void GexWizardTable::UpdateDevicesByRun()
{
    mMaxDevicesByRun = 0;
    CTest*	lBinTestCell = NULL;
    int lBegin=0;
    int lEnd=0;
    CGexGroupOfFiles *lGroup = 0;
    CGexFileInGroup *lFile = 0;
    bool lClear(true);

    // Load Device list...
    if(gexReport->getGroupsList().count() > 1 )
    {
        //Need to update part Result combo box.
        lGroup = gexReport->getGroupsList()[0];
        lFile = (lGroup->pFilesList.isEmpty()) ? NULL : lGroup->pFilesList.first();
    }
    else if(gexReport->getGroupsList().count() )
    {
        // Get handle to HardBin parameter so to know how many parts tested in total.
        lGroup = gexReport->getGroupsList().first();
        lFile = (lGroup->pFilesList.isEmpty()) ? NULL : lGroup->pFilesList.first();
    }
    else
    {
        return;
    }


    if(lFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_HBIN, GEX_PTEST, &lBinTestCell, true, false) != 1)
      return;

    for (long lFileIndex = 0; lFileIndex < lGroup->pFilesList.count(); lFileIndex++)
    {
        // Get the sublot offset
        lBinTestCell->findSublotOffset(lBegin, lEnd, lFileIndex);
        GSLOG(6, QString("Looping over %1 test results...").arg(lEnd-lBegin).toLatin1().data() );
        lFile = lGroup->pFilesList.at(lFileIndex);

        for(gsint32 lIndex = lBegin; lIndex < lEnd; lIndex++)
        {
            // Add device only if it's not filtered
            if (lBinTestCell->m_testResult.isValidIndex(lIndex)
                 && lBinTestCell->m_testResult.isValidResultAt(lIndex))
            {
                CPartInfo* lPartInfo = lFile->pPartInfoList.at(lIndex-lBegin);
                if (lPartInfo)
                {
                    // Fill the list of devices QHash((x,y), List(index))
                    // insert only if doesn't exit in the list of index
                    QPair<gsint16,gsint16> lCoord(lPartInfo->GetDieX(), lPartInfo->GetDieY());
                    if (mDevicesByRun.contains(lCoord) && !mDevicesByRun.value(lCoord).contains(lIndex))
                    {
                        QList<gsint32> lTempList = mDevicesByRun.value(lCoord);
                        lTempList.append(lIndex);
                        mDevicesByRun.insert(lCoord, lTempList);
                    }
                    else
                    {
                        // Clear only if we will add elts to the combo box
                        if (lClear == true)
                        {
                            comboBoxDevice->clear();
                            lClear = false;
                        }

                        QList<gsint32> lTempList;
                        lTempList.append(lIndex);
                        mDevicesByRun.insert(lCoord, lTempList);
                    }
                }
            }
        }
    }
    QList< QList<gsint32> > lListValues = mDevicesByRun.values();
    for (int lKey=0; lKey<lListValues.size(); ++lKey)
    {
        if (mMaxDevicesByRun < lListValues[lKey].size())
            mMaxDevicesByRun = lListValues[lKey].size();
    }
}


void GexWizardTable::NextItem()
{
    if(mListItems.isEmpty())
        return;

    mListItems[mCurrentIndexItem]->setSelected(false);
    if(mCurrentIndexItem + 1 < mListItems.size())
    {
        ++mCurrentIndexItem;
    }
    else
    {
        mCurrentIndexItem = 0;
    }
    SetSelectedItem();
}

void GexWizardTable::PreviousItem()
{
    if(mListItems.isEmpty())
        return;

    mListItems[mCurrentIndexItem]->setSelected(false);
    if(mCurrentIndexItem - 1 >= 0)
    {
      --mCurrentIndexItem;
    }
    else
    {
        mCurrentIndexItem = mListItems.size() - 1;
    }
    SetSelectedItem();
}

void GexWizardTable::SetSelectedItem()
{
    if(!mListItems.isEmpty())
    {
        mListItems[mCurrentIndexItem]->setSelected(true);
        //mListItems[mCurrentIndexItem]->setSelected(!mListItems[mCurrentIndexItem]->isSelected());
        mListItems[mCurrentIndexItem]->tableWidget()->scrollToItem(mListItems[mCurrentIndexItem]);
        mFindDialog->DisplayNbOccurences(mCurrentIndexItem+1, mListItems.size());
    }
}

void GexWizardTable::InitListItemsResearched()
{
    InitCurrentDrillTable();

    mListItems.clear();
    if(mCurrentDrillTable)
    {
        if(mCurrentIndexTable == 4)
        {
            QList<DrillDeviceWidget*> ::iterator lIterBegin(mTableDeviceWidgets.begin()), lIterEnd(mTableDeviceWidgets.end());
            for(;lIterBegin != lIterEnd; ++lIterBegin )
            {
                mListItems += (*lIterBegin)->GetDeviceTable()->findItems(mCurrentText, Qt::MatchContains);
            }
        }
        else
        {
            mListItems   = mCurrentDrillTable->findItems(mCurrentText, Qt::MatchContains);
        }

        // -- remove items that are not visible -- maybe not the best way to do it....
        QMutableListIterator<QTableWidgetItem *> lIter(mListItems);
        while (lIter.hasNext())
        {
            QTableWidgetItem *  lItem   = lIter.next();
            QTableWidget     *  lParent = lItem->tableWidget();

            if(lParent == 0 || lItem == 0)
                continue;

            // -- if selection activated, remove all items not selected
            /*if(mSelectionMode == true && !lItem->isSelected())
            {
                lIter.remove();
                continue;
            }*/

            if(lParent->isRowHidden( lParent->row(lItem) ) ||  lParent->isColumnHidden(lParent->column(lItem)) )
            {
                lIter.remove();
                continue;
            }
        }

        mCurrentIndexItem    = 0;
        if(mListItems.isEmpty())
            mFindDialog->ColorFound(false);
        else
           mFindDialog->ColorFound(true);
    }
}


void GexWizardTable::FindTxt(const QString &textEdited)
{
    mFindDialog->SetStyleSheet(textEdited.isEmpty());

    if(!mListItems.isEmpty() )
        mListItems[mCurrentIndexItem]->setSelected(false);
        //mListItems[mCurrentIndexItem]->setSelected(!mListItems[mCurrentIndexItem]->isSelected());

    RazSearch();

    if(textEdited.isEmpty())
        return;

    mCurrentText = textEdited;
    InitListItemsResearched();
    SetSelectedItem();
}

/*void GexWizardTable::ShowFindBox()
{
    if(this->isHidden() == false)
    {
        RazSearch();
        const QPoint global = mapToGlobal(rect().center());
        mFindDialog->move(global.x() + mFindDialog->width() / 2, global.y() - mFindDialog->height() / 2);

        mFindDialog->Init();
        mFindDialog->show();
    }
}*/

void GexWizardTable::focusInEvent( QFocusEvent* ) {
    mWizardHandler->SetLastTableViewed(this);
}

void GexWizardTable::keyPressEvent(QKeyEvent *event)
{
    if (event->key() ==  Qt::Key_Escape)
    {
        event->accept();
    }
    // This condition can't be activated because the Ctrl+F is catched in the main window
   /* else if (event->key() == Qt::Key_F && QApplication::keyboardModifiers() && Qt::ControlModifier)
    {
        RazSearch();
        ShowFindBox();
        SetClearSelectionAvalaible(false);
        event->accept();
    }*/
    else if( (event->key() == Qt::Key_T || event->key() == Qt::Key_W || event->key() == Qt::Key_D)
             && QApplication::keyboardModifiers() && Qt::ControlModifier)
    {
        if(GS::LPPlugin::ProductInfo::getInstance()->isExaminator() == false)
        {
            if(event->key() == Qt::Key_T)
            {
                pGexMainWindow->OnNewWizard(GEX_TABLE_WIZARD_P1);
            }
            else if(event->key() == Qt::Key_W)
            {
                pGexMainWindow->OnNewWizard(GEX_DRILL_3D_WIZARD_P1, "#_gex_drill--drill_3d=wafer_sbin--g=0--f=0");
            }
            else if(event->key() == Qt::Key_D)
            {
                pGexMainWindow->OnNewWizard(GEX_CHART_WIZARD_P1);
            }
        }
    }
    else
    {
        event->ignore();
    }
    QDialog::keyPressEvent(event);
}

void GexWizardTable::ChangeTabBar(int indexTab)
{
    RazSelectionColumn();
    setFocus();
    mCurrentIndexTable = indexTab;
    InitCurrentDrillTable();

    if (mCurrentDrillTable)
    {
        mCurrentDrillTable->clearSelection();
    }
}

/// Set to true when searchBox is hidden, otherwise must be keept in order to narrow the research
/// to the current selection.
void GexWizardTable::SetClearSelectionAvalaible(bool status)
{
    mTableParametricTests->SetClearSelectionAvalaible(status);
    mTableFunctionalTests->SetClearSelectionAvalaible(status);
    mTableHardBinning->SetClearSelectionAvalaible(status);
    mTableSoftBinning->SetClearSelectionAvalaible(status);

    for (int i=0; i<mMaxDevicesByRun; ++i)
    {
        if (mTableDeviceWidgets[i])
        {
            mTableDeviceWidgets[i]->GetDeviceTable()->SetClearSelectionAvalaible(status) ;
        }
    }

}

void GexWizardTable::HorizontalHeaderClicked(const QPoint& pos)
{

    RazSelectionColumn();
    mSelectionMode = true;
    if(mCurrentIndexTable == 4)
    {
        for (int i=0; i<mMaxDevicesByRun; ++i)
        {
            if (mTableDeviceWidgets[i])
            {
                DrillTable* lDrillTable = mTableDeviceWidgets[i]->GetDeviceTable();

                lDrillTable->setSelectionBehavior(QAbstractItemView::SelectColumns);
                lDrillTable->selectColumn(lDrillTable->horizontalHeader()->logicalIndexAt(pos));
            }
        }
    }
    else
    {
          mCurrentDrillTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
          mCurrentDrillTable->setSelectionBehavior(QAbstractItemView::SelectColumns);
          mCurrentDrillTable->selectColumn(mCurrentDrillTable->horizontalHeader()->logicalIndexAt(pos));
    }
}

void GexWizardTable::InitCurrentDrillTable()
{
    switch(mCurrentIndexTable)
    {
        case 0	:	mCurrentDrillTable  = mTableParametricTests;     break;
        case 1	:	mCurrentDrillTable  = mTableFunctionalTests;     break;
        case 2	:	mCurrentDrillTable  = mTableSoftBinning;         break;
        case 3	:	mCurrentDrillTable  = mTableHardBinning;         break;
        case 4  :   mCurrentDrillTable  = mTableDeviceWidgets.isEmpty()?0: (*mTableDeviceWidgets.begin())->GetDeviceTable();       break;
        default  :
           mCurrentDrillTable = 0;
        break;
    }

    ApplySelectionStylesheet();

}

void GexWizardTable::ApplySelectionStylesheet()
{
    if (mCurrentDrillTable == NULL)
    {
        return;
    }

   if (mColorSelection.isValid())
   {
       mCurrentDrillTable->setStyleSheet( QString("QTableWidget"
                                                       "{"
                                                       " alternate-background-color: rgb(192,255,192);"
                                                       " color: black;"
                                                       "}"
                                                       "QTableWidget::item:selected"
                                                       "{"
                                                       " background-color: rgb(%1,%2,%3);"
                                                       " color: black;"
                                                       "}")
                                                 .arg(mColorSelection.red())
                                                 .arg(mColorSelection.green())
                                                 .arg(mColorSelection.blue())
                                                 );
      /* mCurrentDrillTable->setStyleSheet( QString("QTableWidget"
                                                       "{"
                                                       " alternate-background-color: rgb(192,255,192);"
                                                       " color: black;"
                                                       "}"
                                                       "QTableWidget::item:selected"
                                                       "{"
                                                       " background-color: rgb(%1,%2,%3);"
                                                       " border: 2px solid blue;"
                                                       " border-radius: 2px;"
                                                       " color: black;"
                                                       "}")
                                                 .arg(mColorSelection.red())
                                                 .arg(mColorSelection.green())
                                                 .arg(mColorSelection.blue())
                                                 );*/
   }
   else
   {
        mCurrentDrillTable->setStyleSheet( QString("QTableWidget"
                                                   "{"
                                                   " alternate-background-color: rgb(192,255,192);"
                                                   " color: black;"
                                                   "}"
                                                   )
                                             );
   }
}

void GexWizardTable::RazSelectionColumn()
{
    // -- clear Selection
    if(mCurrentIndexTable == 4)
    {
        for (int i=0; i<mMaxDevicesByRun; ++i)
        {
            if (mTableDeviceWidgets[i])
            {
                DrillTable* lDrillTable = mTableDeviceWidgets[i]->GetDeviceTable();
                lDrillTable->clearSelection();
            }
        }
    }
    else
    {
        mCurrentDrillTable->clearSelection();
    }
    mSelectionMode = false;
}

void GexWizardTable::OnTableClicked(QModelIndex modelIndex)
{
    if(mCurrentIndexTable != 0 )
        RazSelectionColumn();

    mSelectionMode = true;
    // -- reset to the normal selection mode
    if(mCurrentIndexTable == 0 )
    {
        mCurrentDrillTable->setSelectionBehavior( QAbstractItemView::SelectRows);
        //mCurrentDrillTable->selectRow(modelIndex.row());
        mSelectionMode = true;
    }
    else if(mCurrentIndexTable == 4)
    {
        for (int i=0; i<mMaxDevicesByRun; ++i)
        {
            if (mTableDeviceWidgets[i])
            {
                DrillTable* lDrillTable = mTableDeviceWidgets[i]->GetDeviceTable();
                lDrillTable->setSelectionBehavior( QAbstractItemView::SelectRows);
                lDrillTable->selectRow(modelIndex.row());
            }
        }
    }
    else
    {
           mCurrentDrillTable->setSelectionMode( QAbstractItemView::SingleSelection);
           mCurrentDrillTable->item(modelIndex.row(), modelIndex.column())->setSelected(true);
    }
}

void GexWizardTable::RazSearch()
{
    mCurrentIndexItem    = 0;
    mCurrentText         = "";
    mListItems.clear();
    if(mCurrentDrillTable)
    {
        mCurrentDrillTable->clearSelection();
    }
}


void GexWizardTable::OnRightCLicked(const QPoint& pos)
{
    QMenu pMenu(this);
    QAction *lActionSelectColumn = pMenu.addAction("Select Column");
    QAction *lActionResult = pMenu.exec(QCursor::pos());
     pMenu.setMouseTracking(false);
    if(lActionResult == lActionSelectColumn) {
        HorizontalHeaderClicked(pos);
    }
}

void GexWizardTable::InitInteractiveView()
{
    QPixmap  lTable(50, 50);
    lTable.load(QString::fromUtf8(":/gex/icons/options_statistics.png"));

    QPixmap lWafermap(50, 50);
    lWafermap.load(QString::fromUtf8(":/gex/icons/options_wafermap.png"));


    QPixmap lHisto(50, 50);
    lHisto.load(QString::fromUtf8(":/gex/icons/options_advhisto.png"));

    QString lOPenWafer("Open a Wafermap");
    if(GS::LPPlugin::ProductInfo::getInstance()->isExaminator() == false)
        lOPenWafer.append("\t(Ctrl+W)");

    QString lOPenTable("Open a Table");
    lOPenTable.append("\t(Ctrl+T)");

    QString lOPenDiagram("Open a Diagram");
    if(GS::LPPlugin::ProductInfo::getInstance()->isExaminator() == false)
        lOPenDiagram.append("\t(Ctrl+D)");


    QAction* lDisplayWafermap   = new QAction(lWafermap,  lOPenWafer, this );
    QAction* lDisplayTable      = new QAction(lTable,     lOPenTable, this );
    QAction* lDisplayHisto      = new QAction(lHisto,     lOPenDiagram, this );


    connect(lDisplayTable,      SIGNAL(triggered()), this, SLOT(OnInteractiveTables()));
    connect(lDisplayWafermap,   SIGNAL(triggered()), this, SLOT(OnInteractiveWafermap()));
    connect(lDisplayHisto,      SIGNAL(triggered()), this, SLOT(OnInteractiveCharts()));

    QMenu* lMenu= new QMenu(toolButtonInteractiveViews);

    lMenu->addAction(lDisplayWafermap);
    if(GS::LPPlugin::ProductInfo::getInstance()->isExaminator() == false)
        lMenu->addAction(lDisplayTable);
    lMenu->addAction(lDisplayHisto);

    toolButtonInteractiveViews->setMenu(lMenu);
}


void GexWizardTable::SelectionColor(QColor lColor)
{

   if(lColor.isValid() )
   {
       mColorSelection = lColor;
       ApplySelectionStylesheet();
   }
}

FindDialog::FindDialog(QWidget* parent):QDialog(parent), mIsEmptyStyle(false), mColorChosen(Qt::yellow)
{
    setupUi(this);

    //-- add the action settings to the left
    mSearchSettings = lineEdit->addAction( QIcon(":/gex/icons/new_parameters.png"), QLineEdit::LeadingPosition);
    connect(mSearchSettings, SIGNAL(triggered(bool)), this, SLOT(PopupOption(bool)) );


   //-- add actions to the menu that will be exec when triggering the mSearchSettings action
    mSelectionColor = mOptions.addAction(QIcon(":/gex/icons/colour-wheel.png"), "HighLight Color");
    connect(mSelectionColor, SIGNAL(triggered(bool)), this, SLOT(ChooseSelectionColor(bool)) );
}

FindDialog::~FindDialog()
{
}


void FindDialog::PopupOption(bool)
{
    mOptions.exec(lineEdit->mapToGlobal(QPoint(0, 0 + lineEdit->height())));
}

void FindDialog::ChooseSelectionColor(bool)
{
    QColor lColor = QColorDialog::getColor(mColorChosen, 0, "Choose the HightLight Color"/*,
                                              QColorDialog::DontUseNativeDialog*/);

    if(lColor.isValid() )
    {
        mColorChosen = lColor;
        emit SelectionColor(lColor);
    }
}

void FindDialog::Init()
{
    mIsEmptyStyle = true;
    lineEdit->clear();
}


void FindDialog::ColorFound(bool found)
{
    if(found)
    {
        lineEdit->setStyleSheet( "padding-left: 0px;"\
                                 "border: 1px solid black;"\
                                 "border-radius: 5px;"
                                );

        pushButtonNext->setDisabled(false);
        pushButtonPrevious->setDisabled(false);
    }
    else
    {
        lineEdit->setStyleSheet( "padding-left: 0px;"\
                                 "border: 1px solid black;"\
                                 "border-radius: 5px;"\
                                 "color: red;"
                                );
         pushButtonNext->setDisabled(true);
         pushButtonPrevious->setDisabled(true);
    }

}

void FindDialog::DisplayNbOccurences(int aNumOccurence, int aNbTotal)
{
    if(aNbTotal)
    {
        labelOccurence->setText( QString("%1/%2").arg(aNumOccurence).arg(aNbTotal));
    }
    else
    {
        labelOccurence->setText("   ");
    }
}

void FindDialog::SetStyleSheet(bool emptyString)
{
    DisplayNbOccurences(0, 0);
    if(!emptyString)
    {
        if(mIsEmptyStyle)
        {

            lineEdit->setStyleSheet( "padding-left: 0px;"\
                                     "background:;"\
                                     "border: 1px solid gray;"\
                                     "border-radius: 5px;"
                                     );
            mIsEmptyStyle = false;
        }
    }
    else
    {
        lineEdit->setStyleSheet(
                                 "border: 1px solid gray;"\
                                 "border-radius: 5px;"
                                 );

        mIsEmptyStyle = true;
    }
}
