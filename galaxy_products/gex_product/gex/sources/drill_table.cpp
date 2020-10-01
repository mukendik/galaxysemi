///////////////////////////////////////////////////////////
// Interactive Table Drill
///////////////////////////////////////////////////////////
#ifdef _WIN32
  #include <windows.h>
#endif

#include "gex_shared.h"
#include "drill_table.h"
#include "drill_parametric_table.h"
#include "browser_dialog.h"
#include "browser.h"
#include "report_build.h"
#include "report_options.h"
#include "snapshot_dialog.h"
#include "assistant_flying.h"
#include "patman_lib.h"				// List of '#define' to return type of distribution (gaussian, lognormal, etc...)
#include "picktest_dialog.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "gex_scriptengine.h"
#include "cbinning.h"
#include "cpart_info.h"
#include <gqtl_log.h>
#include "product_info.h"
#include "engine.h"
#include "message.h"
#include "pat_info.h"

#include <QFileDialog>
#include <QTabWidget>
#include <QCheckBox>
#include <QClipboard>
#include <QSettings>
#include <QMessageBox>
#include <QPalette>
#include <QCloseEvent>

// External definitions for pointers to pixmaps.
#include "gex_pixmap_extern.h"
#include "gex_algorithms.h"

#include "gexperformancecounter.h"

extern GexScriptEngine *pGexScriptEngine;

// cstats.cpp
extern double	ScalingPower(int iPower);

#if defined unix || __MACH__
#include <stdlib.h>
#include <unistd.h>
#endif

#include <math.h>

// main.cpp
extern GexMainwindow *pGexMainWindow;

// in report_build.cpp
extern CGexReport *	gexReport;			// Handle to report class
extern CReportOptions ReportOptions;		// Holds options (report_build.h)

//const char* const cUnitColumnLabel = "Units";

void GexMainwindow::Wizard_DrillTable(QString strLink, bool fromHTMBrowser)
{
    GSLOG(5, "Opening Wizard DrillTable...")
    bool bRefuse=false;

    // Enable/disable some features...
    if(GS::LPPlugin::ProductInfo::getInstance()->getProductID() == GS::LPPlugin::LicenseProvider::eSzOEM)
    {
        bRefuse = true;
    }

    // Reject Drill?
    if(bRefuse)
    {
        QString m=ReportOptions.GetOption("messages", "upgrade").toString();
        GS::Gex::Message::information("", m);
        return;
    }

    // Show wizard page : Intercactive Tables
    ShowWizardDialog(GEX_TABLE_WIZARD_P1, fromHTMBrowser);

    // Update the URL edit box
    QString strString = GEX_BROWSER_ACTIONLINK;
    strString += GEX_BROWSER_DRILL_TABLE;
    AddNewUrl(strString);

    if ( LastCreatedWizardTable()->isDataAvailable() == false)
    {
        GS::Gex::Message::information("", "No data available yet\nSelect your data first ('Home' page)!");

        // Switch to the home page
        OnHome();
        return;
    }
    if(LastCreatedWizardTable())
        LastCreatedWizardTable()->ShowTable(strLink);

    mCanCreateNewDetachableWindow = true;


}

void GexMainwindow::Wizard_DrillAll(void)
{
    // Examinator doesn't have the 'All' link. In case the URL is used, display error message
    if(GS::LPPlugin::ProductInfo::getInstance()->isExaminator())
    {
        QString m=ReportOptions.GetOption("messages", "upgrade").toString();
        GS::Gex::Message::information("", m);
        return;
    }

    // Create a 'Table' tab
    Wizard_DrillTable("");

    // Create a 'Chart' tab
    Wizard_DrillChart();

    // 3D wafermap is not allowed in TER-GEX, so display it only if not TER-GEX
    if(!GS::LPPlugin::ProductInfo::getInstance()->isExaminatorTer())
        Wizard_Drill3D();
}

void GexMainwindow::Wizard_DrillTable(void)
{
    Wizard_DrillTable("");
}

const QString DrillTable::sUnitColumnLabel("Units");


DrillTable::DrillTable(QWidget * parent, Loading *lLoading)
    : QTableWidget (parent),
      mParentWizard(static_cast<GexWizardTable*>(parent)),
      mLoading(lLoading),
      mClearSelectionAvalaible(true)

{
    mSortOnAbsoluteValue = false;
    setContextMenuPolicy(Qt::CustomContextMenu);
    verticalHeader()->setDefaultSectionSize(20);
    setAlternatingRowColors(true);
    setStyleSheet(
                "QTableWidget "
                "{"
                    "alternate-background-color: rgb(192,255,192);"
                "}"
                );

}


DrillTable::DrillTable(GexWizardTable* wizardParent, QWidget * parent, Loading *lLoading)
                : QTableWidget (parent),
                mParentWizard(static_cast<GexWizardTable*>(wizardParent)),
                mLoading(lLoading),
                mClearSelectionAvalaible(true)
{
    mSortOnAbsoluteValue = false;
    setContextMenuPolicy(Qt::CustomContextMenu);
    verticalHeader()->setDefaultSectionSize(20);
    setAlternatingRowColors(true);
    setStyleSheet(
                "QTableWidget "
                "{"
                    "alternate-background-color: rgb(192,255,192);"
                "}"
                );
}

void DrillTable::SetSortOnAbsoluteValue(bool sortOnAbsoluteValue)
{
    mSortOnAbsoluteValue = sortOnAbsoluteValue;
}

void DrillTable::checkItemAndAddUnit(DrillTableItem *item)
{
    if(item)
    {
        if (horizontalHeaderItem(item->column()))
        {
            //horizontalHeaderItem(item->column())->setData(Qt::UserRole, QString(cUnitColumnLabel));
            horizontalHeaderItem(item->column())->setData(Qt::UserRole, sUnitColumnLabel);
            item->setData(Qt::UserRole, sUnitColumnLabel );
        }
    }
}

void DrillTable::keyPressEvent(QKeyEvent* e)
{
    switch (e->key())
    {
    case Qt::Key_Escape:
        // ignore
        break;
    default:
        QTableWidget::keyPressEvent(e);
        break;
    }
}

unsigned DrillTableItem::sNumOfIntances=0;

///////////////////////////////////////////////////////////
// Constructor.
///////////////////////////////////////////////////////////
DrillTableItem::DrillTableItem( int et, const QString &txt, int row/* = -1*/  )
    : QTableWidgetItem(txt, et)
{
    sNumOfIntances++;
    // eg: Set to 'GEX_DRILL_CELLTYPE_ALARM' for failing cells that will have a RED background.
    mFailing = GEX_DRILL_CELLTYPE_NORMAL;
    mCustomBkColor = Qt::white;	// Default background color
    setTextAlignment(Qt::AlignLeft);
    paint(row);
}
DrillTableItem::~DrillTableItem()
{
    sNumOfIntances--;
}

///////////////////////////////////////////////////////////
// Re-implementation painting, so to have red background if needed.
///////////////////////////////////////////////////////////
void DrillTableItem::paint(int row )
{
    if(mFailing == GEX_DRILL_CELLTYPE_ALARM)
        setBackground(QBrush(QColor(Qt::red)));		// Alarm: Paint with RED background color.
    else if(mFailing == GEX_DRILL_CELLTYPE_WARNING)
        setBackground(QBrush(QColor(Qt::yellow)));  // Warning: Paint with YELLOW background color.
    else if(mFailing == GEX_DRILL_CELLTYPE_USR_COLOR)
        setBackground(QBrush(QColor(0,255,0))); // USer defined custom background color
    else if (mFailing == GEX_DRILL_CELLTYPE_ALARM_LIGHT)
        setBackground(QBrush(QColor(255,90,90)));	// Test failed with valid value inside limits
//    else if(row & 1)
//        setBackground(QBrush(QColor(192,255,192)));	// Odd lines (1,3,5,..) have a pastel green background (Listing style!).

    bool isInteger = false;
    bool isFloat = false;
    text().toInt(&isInteger, 10);
    text().toFloat(&isFloat);
    if (isInteger || isFloat)
        setTextAlignment(2);
    else
        setTextAlignment(1);
}

///////////////////////////////////////////////////////////
// Re-implementation of 'operator <' so sorting on text & numbers is fine.
///////////////////////////////////////////////////////////
bool DrillTableItem::operator<(QTableWidgetItem const &other) const
{
    const DrillTableItem *drillElement = static_cast<const DrillTableItem*>(&other);
    double ret = algorithms::gexCompareDoubleString(key(), drillElement->key(), Qt::CaseInsensitive);
    if (ret >= 0)
        return false;
    else
        return true;
}


///////////////////////////////////////////////////////////
// Re-implementation of 'key' so sorting on text & numbers is fine.
///////////////////////////////////////////////////////////
QString &DrillTableItem::key() const
{

    int lIndex = static_cast<DrillTable*>(tableWidget())->getWizardParent()->tabWidgetTables->currentIndex();
    //int lIndex = pGexMainWindow->pWizardTable->tabWidgetTables->currentIndex();

    if(mSortedKey.contains(lIndex))
        return mSortedKey[lIndex];

    double		lfData=0;
    bool		bIsString=false;
    bool		bR_R_Sort=false;
    DrillTable*	pTable = (DrillTable*)tableWidget();

    QString strText=text();	// GetText string in cell.
    int iCol = column();

    Qt::SortOrder       lSortOrder      = Qt::AscendingOrder;
    QHeaderView *       lColumnHeader   = pTable->horizontalHeader();

    // Retrieve sorting order from the header
    if (lColumnHeader)
        lSortOrder = lColumnHeader->sortIndicatorOrder();

    // Check which Table we're sorting: Statistics, Binning...?
    switch(lIndex)
    {
        case 0:	// Parametric Test
            DrillParametricTable* lParametricTable;
            lParametricTable = dynamic_cast<DrillParametricTable*>(tableWidget());
            if (lParametricTable)
            {
                if(iCol == lParametricTable->ColPosition(TEST_TNAME) ||
                        iCol == lParametricTable->ColPosition(TEST_GROUP) ||
                        iCol == lParametricTable->ColPosition(TEST_TYPE) ||
                        iCol == lParametricTable->ColPosition(TEST_LTL) ||
                        iCol == lParametricTable->ColPosition(TEST_HTL) ||
                        iCol == lParametricTable->ColPosition(TEST_LSL) ||
                        iCol == lParametricTable->ColPosition(TEST_HSL) ||
                        iCol == lParametricTable->ColPosition(TEST_DRIFTL) ||
                        iCol == lParametricTable->ColPosition(TEST_DRIFTLOW) ||
                        iCol == lParametricTable->ColPosition(TEST_DRIFTHIGH) ||
                        iCol == lParametricTable->ColPosition(TEST_SHAPE)  ||
                        iCol == lParametricTable->ColPosition(TEST_STATS_SRC))
                    bIsString = true;	// These columns are strings, not numbers (Testname, type, LL, HL, Distribution shape)
                else
                {
                    bIsString = false;	// These columns are numbers (so sorting is different than for strings)
                    bR_R_Sort = (iCol == lParametricTable->ColPosition(TEST_GAGE_RR));
                }
            }
            else
            {
                // remove log because too much...
                // GSLOG(3, QString("Unable to get pointer to parametric table").toLatin1().data());
            }
            break;
        case 1: // Functional tests
        case 2:	// Software binning
        case 3: // Hardware binning
            if(iCol >=1 && iCol <= 2)
                bIsString = true;	// These columns are strings, not numbers (Bin name, Pass/Fail flag)
            else
                bIsString = false;	// These columns are numbers (so sorting is different than for strings)
            break;
        case 4: // Device results
            if((iCol >=2 && iCol <= 3) || (iCol == 11))
                bIsString = true;	// These columns are strings, not numbers (Bin name, Pass/Fail flag)
            else
                bIsString = false;	// These columns are numbers (so sorting is different than for strings)
            break;
    }

    // If n/a value, add a character to make it at the end in both order (asc and desc)
    if (strText.compare(GEX_NA) == 0)
    {
        if (lSortOrder == Qt::AscendingOrder)
            strText = QChar(255) + strText;
        else
            strText = QChar(1) + strText;

        mSortedKey[lIndex] = strText;
        return  mSortedKey[lIndex];
    }

    // If column is a number, we deal with floating point numbers, not strings!
    if(bIsString == true)
    {
        mSortedKey[lIndex] = strText;
        return  mSortedKey[lIndex];
    }

    // We have to sort numbers....
    bool bNegative = false;

    if(strText[0] == 'n')
        lfData = 1e21;	// This cell included string 'n/a'
    else
    {
        if(bR_R_Sort)
        {
            // If sorting R&R% column, then must extract the % value from string: <value> (xx%)
            if(strText.count('(') > 0)
                strText = strText.section('(',1);		// extract the % value string.
            sscanf(strText.toLatin1().constData(),"%lf",&lfData);		// extract value
        }
        else
        {
            int lR=sscanf(strText.toLatin1().constData(),"%lf", &lfData);		// extract value
            if (lR<=0)
                GSLOG(4, QString("Failed to translate string '%1' to double").arg(strText).toLatin1().data() );
        }
        lfData *= 1e10;
        if(lfData > 1e21)
            lfData = 1e21;
        if(lfData < -1e21)
            lfData = -1e21;
        if(lfData < 0)
        {
            if(pTable->SortOnAbsoluteValue())
                lfData = -lfData;
            else
            {
                lfData = 1e21+lfData;
                bNegative = true;
            }
        }
    }
    strText = QString::number(lfData,'f',0);

    // Ensures string is in format '00000<data>'.
    strText = strText.rightJustified(23,'0');

    // Ensures negative values are smaller than positive values
    if(bNegative)
        strText = "A" + strText;
    else
        strText = "Z" + strText;

    mSortedKey[lIndex] = strText;
    return  mSortedKey[lIndex];
}
