#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

#include <stdio.h>
#include <QDialog>

#if defined unix || __MACH__
#include <unistd.h>
#include <stdlib.h>
#endif

#include "pickbin_single_dialog.h"
#include "browser_dialog.h"

// Galaxy QT libraries
#include "gqtl_sysutils.h"

// main.cpp
extern GexMainwindow *	pGexMainWindow;

///////////////////////////////////////////////////////////
// Constructor
PickBinSingleDialog::PickBinSingleDialog( QWidget* parent, bool modal, Qt::WindowFlags f )
    : QDialog( parent, f)
{
    setupUi(this);
    setModal(modal);

    // Apply Examinator palette
    GexMainwindow::applyPalette(this);

    QObject::connect(PushButtonOk,		SIGNAL(clicked()),								this, SLOT(accept()));
    QObject::connect(PushButtonCancel,	SIGNAL(clicked()),								this, SLOT(reject()));
    QObject::connect(mUi_TreeWidget,	SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),	this, SLOT(accept()));

    // Empty Filter list
    mUi_TreeWidget->clear();

    // Multiple selection mode allowed, Fill the list with sorting enabled
    setMultipleSelection(true);
    mUi_TreeWidget->sortByColumn(0,Qt::AscendingOrder);

    // Set minimum size for the Value column
    mUi_TreeWidget->setColumnWidth(0,50);
}

///////////////////////////////////////////////////////////
// Select operating mode: single or multiple selection...
///////////////////////////////////////////////////////////
void PickBinSingleDialog::setMultipleSelection(bool bAllowMultiSelections)
{
    QString strText;

    if(bAllowMultiSelections == true)
    {
        // Allow multiple selections: keep default text + behavior!
        mUi_TreeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
        strText = "Pick one binning...or hold the 'Ctrl' key for multiple selections, ";
        strText += "or hold the 'Shift' key for a range selection.";
    }
    else
    {
        // Single selection mode...set text + Widget flags
        mUi_TreeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        strText = "Pick ONE binning from the list...";
    }

    // Update text
    TextLabel->setText(strText);
}

///////////////////////////////////////////////////////////
// Fills List with given list of values
///////////////////////////////////////////////////////////
void PickBinSingleDialog::fillList(QStringList & cList)
{
    // QTreeWidgetItem	*ptTreeItem;
    //PickBinSingleDialogTreeItem *ptTreeItem;
    QString			strBinning;

    // Empty test list
    mUi_TreeWidget->clear();

    // Read Filter list returned, so to fill list.
    for(QStringList::Iterator it = cList.begin(); it != cList.end(); ++it )
    {
        strBinning = *it;

        QStringList strListTreeItem;
        strListTreeItem << strBinning.section("#GEX#", 0, 0) << strBinning.section("#GEX#", 1, 1);
        /*ptTreeItem = */new PickBinSingleDialogTreeItem(mUi_TreeWidget, strListTreeItem);
    }

    // Sort list
    mUi_TreeWidget->sortItems(0,Qt::AscendingOrder);

    // Select first item
    mUi_TreeWidget->setCurrentItem(mUi_TreeWidget->topLevelItem(0));
}

///////////////////////////////////////////////////////////
// Show a list of Bins (SoftBin) found in the file
///////////////////////////////////////////////////////////
QString PickBinSingleDialog::getBinsList(void)
{
    QList<QTreeWidgetItem*>	qlTreeItemList;
    QString strTreeBinList = "";

    // QList of selected items
    qlTreeItemList = mUi_TreeWidget->selectedItems();


    //Compute the test list (if any selection made)
    int iSelectionCount = 0;

    if (qlTreeItemList.count() == 0)
        return "*";
    else
    {
        strTreeBinList += qlTreeItemList[iSelectionCount]->text(0);
        iSelectionCount++;
    }


    for (;iSelectionCount<qlTreeItemList.count();iSelectionCount++)
    {
        strTreeBinList += "|";
        strTreeBinList += qlTreeItemList[iSelectionCount]->text(0);
    }

    return strTreeBinList;
}

QString PickBinSingleDialog::getBinsNameList(void){
    QList<QTreeWidgetItem*>	qlTreeItemList;
    QString strTreeBinNameList = "";

    // QList of selected items
    qlTreeItemList = mUi_TreeWidget->selectedItems();


    //Compute the test list (if any selection made)
    int iSelectionCount = 0;

    if (qlTreeItemList.count() == 0)
        return "*";
    else
    {
        strTreeBinNameList += qlTreeItemList[iSelectionCount]->text(1);
        iSelectionCount++;
    }


    for (;iSelectionCount<qlTreeItemList.count();iSelectionCount++)
    {
        strTreeBinNameList += "|";
        strTreeBinNameList += qlTreeItemList[iSelectionCount]->text(1);
    }

    return strTreeBinNameList;
}

///////////////////////////////////////////////////////////
// Constructor
PickBinSingleDialogTreeItem::PickBinSingleDialogTreeItem(QTreeWidget *pTreeWidget, QStringList strTreeListItem) : QTreeWidgetItem(pTreeWidget,strTreeListItem)
{
}

//////////////////////////////////////////////////////////
// Destructor
PickBinSingleDialogTreeItem::~PickBinSingleDialogTreeItem() {
}

bool PickBinSingleDialogTreeItem::sortInteger(const QString& strLeft, const QString& strRight) const
{
    return strLeft.toInt() < strRight.toInt();
}

bool PickBinSingleDialogTreeItem::sortBinName(const QString& strLeft, const QString& strRight) const
{
    return ( QString::compare(strLeft,strRight,Qt::CaseInsensitive) <0);
}



///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool operator< ( const PickBinSingleDialogTreeItem & other ) const
//
// Description	:	Override the comparison methods
//
///////////////////////////////////////////////////////////////////////////////////
bool PickBinSingleDialogTreeItem::operator< ( const QTreeWidgetItem & other ) const
{
    int		nCol		= treeWidget() ? treeWidget()->sortColumn() : 0;
    QString strLeft		= text(nCol);
    QString strRight	= other.text(nCol);

    switch(nCol)
    {
    case 0 :	return sortInteger(strLeft, strRight);
    case 1 :	return sortBinName(strLeft, strRight);

    default:	return false;
    }
}

