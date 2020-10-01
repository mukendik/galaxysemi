#include "db_engine.h"
#include "pickfilter_dialog.h"
#include "engine.h"

#include <QHeaderView>

// in main.cpp
extern GexMainwindow *	pGexMainWindow;

///////////////////////////////////////////////////////////
// Constructor
PickFilterDialog::PickFilterDialog( QWidget* parent, bool modal, Qt::WindowFlags f )
    : QDialog( parent, f )
{
    setupUi(this);
    setModal(modal);

    // Apply Examinator palette
    GexMainwindow::applyPalette(this);

    QObject::connect(PushButtonOk,		SIGNAL(clicked()),									this, SLOT(accept()));
    QObject::connect(PushButtonCancel,	SIGNAL(clicked()),									this, SLOT(reject()));
    QObject::connect(treeWidget,		SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),	this, SLOT(accept()));

    // Empty Filter list
    treeWidget->clear();

    // Multiple selection mode allowed, Fill the list with sorting enabled
    treeWidget->header()->setSortIndicatorShown(false);
    treeWidget->header()->setSortIndicator(0, Qt::AscendingOrder);

    // Set minimum size for the Value column
    treeWidget->setColumnWidth(0, 200);

    // Allow multiple selections by default
    setMultipleSelection(true);
}

///////////////////////////////////////////////////////////
// Tells if the list of test is empty or not...
///////////////////////////////////////////////////////////
bool PickFilterDialog::isEmpty(void)
{
    if (treeWidget->invisibleRootItem()->childCount() == 0)
        return true;
    else
        return false;
}

///////////////////////////////////////////////////////////
// Fills List with given list of values
///////////////////////////////////////////////////////////
void PickFilterDialog::fillList(QStringList & cList, bool bAddStar/*=true*/)
{
    QTreeWidgetItem	* pTreeItem = NULL;

    // Empty test list
    treeWidget->clear();

    // Read Filter list returned, so to fill list.
    for(QStringList::Iterator it = cList.begin(); it != cList.end(); ++it )
    {
        pTreeItem =   new QTreeWidgetItem(treeWidget);
        pTreeItem->setText(0, *it);
    }

    // Have item '*' ??
    if(bAddStar)
    {
        pTreeItem = new QTreeWidgetItem(treeWidget);
        pTreeItem->setText(0,"*");
    }

    // Sort the list
    treeWidget->sortByColumn(0, Qt::AscendingOrder);

    // Select first item: '*'
    treeWidget->setCurrentItem(treeWidget->invisibleRootItem()->child(0));
}

///////////////////////////////////////////////////////////
// Fills List for given database & FilterID
// Under this mode, filter IS contextual: and ONLY lists
// database values matching the other filter criteria
// As we have up-to 5 filters, we have this function fill
// a list box of the Query filter, but narrowing the fill
// checking all the other 4 filters match the lookup criteria.
///////////////////////////////////////////////////////////
void PickFilterDialog::fillList(const GexDatabaseFilter &dbFilter, bool bAddStar)
{
    if(pGexMainWindow == NULL)
        return;

    // Get list of strings to fill in list box
    QStringList cList = GS::Gex::Engine::GetInstance().GetDatabaseEngine().QuerySelectFilter(dbFilter);

    QTreeWidgetItem	* pTreeItem = NULL;

    // Empty test list
    treeWidget->clear();

    // Read Filter list returned, so to fill list.
    for(QStringList::Iterator it = cList.begin(); it != cList.end(); ++it )
    {
        pTreeItem =   new QTreeWidgetItem(treeWidget);
        pTreeItem->setText(0, *it);
    }

    if (bAddStar)
    {
        pTreeItem = new QTreeWidgetItem(treeWidget);
        pTreeItem->setText(0,"*");
    }

    // Sort the list
    treeWidget->sortByColumn(0, Qt::AscendingOrder);

    // Select first item: '*'
    treeWidget->setCurrentItem(treeWidget->invisibleRootItem()->child(0));
}

///////////////////////////////////////////////////////////
// Fills List for given database & Query object
// Under this mode, filter IS contextual: and ONLY lists
// database values matching the other filter criteria
// defined in the Query object.
///////////////////////////////////////////////////////////
void PickFilterDialog::fillList(const GexDatabaseQuery& dbQuery, QString & strFilterName)
{
    if(pGexMainWindow == NULL)
        return;

    // Get list of strings to fill in list box
    QStringList cList = GS::Gex::Engine::GetInstance().GetDatabaseEngine().QuerySelectFilter(dbQuery, QStringList(strFilterName));

    QTreeWidgetItem	* pTreeItem = NULL;

    // Empty test list
    treeWidget->clear();

    // Read Filter list returned, so to fill list.
    for(QStringList::Iterator it = cList.begin(); it != cList.end(); ++it )
    {
        pTreeItem =   new QTreeWidgetItem(treeWidget);
        pTreeItem->setText(0, *it);
    }

    pTreeItem = new QTreeWidgetItem(treeWidget);
    pTreeItem->setText(0,"*");

    // Sort the list
    treeWidget->sortByColumn(0, Qt::AscendingOrder);

    // Select first item: '*'
    treeWidget->setCurrentItem(treeWidget->invisibleRootItem()->child(0));
}

///////////////////////////////////////////////////////////
// Returns the list of tests selected
///////////////////////////////////////////////////////////
QString PickFilterDialog::filterList(void)
{
    QString								strTestList		= "";
    int									iSelectionCount	= 0;
    QList<QTreeWidgetItem*>				selectedItems	= treeWidget->selectedItems();
    QList<QTreeWidgetItem*>::iterator	itBegin			= selectedItems.begin();
    QList<QTreeWidgetItem*>::iterator	itEnd			= selectedItems.end();

    // Return empty string if no tests selected
    if(treeWidget->invisibleRootItem()->childCount() == 0)
        return "*";

    while(itBegin != itEnd)
    {
        if(iSelectionCount)
            strTestList += ",";	// Separator between multiple selections
        strTestList += (*itBegin)->text(0);

        // Update counter index
        iSelectionCount++;

        itBegin++;
    }

    return strTestList;
}

///////////////////////////////////////////////////////////
// Select operating mode: single or multiple selection...
///////////////////////////////////////////////////////////
void PickFilterDialog::setMultipleSelection(bool bAllowMultiSelections)
{
    QString strText;

    if(bAllowMultiSelections == true)
    {
        // Allow multiple selections: keep default text + behavior!
        treeWidget->setSelectionMode(QTreeWidget::ExtendedSelection);
        strText = "Pick one filter value...or hold the 'Ctrl' key for multiple selections, ";
        strText += "or hold the 'Shift' key for a range selection.";
    }
    else
    {
        // Single selection mode...set text + Widget flags
        treeWidget->setSelectionMode(QTreeWidget::SingleSelection);
        strText = "Pick ONE filter value from the list...";
    }

    // Update text
    TextLabel->setText(strText);
}

