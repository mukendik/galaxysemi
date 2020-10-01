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

#include "browser_dialog.h"
#include "pickproduct_idsql_dialog.h"
#include <gqtl_log.h>

// Galaxy QT libraries
#include "gqtl_sysutils.h"

// main.cpp
extern GexMainwindow *	pGexMainWindow;

///////////////////////////////////////////////////////////
// Constructor
PickProductIdSQLDialog::PickProductIdSQLDialog( QWidget* parent, bool modal, Qt::WindowFlags f )
    : QDialog( parent, f )
{
    setupUi(this);
    setModal(modal);

    // Apply Examinator palette
    GexMainwindow::applyPalette(this);

    QObject::connect(PushButtonOk,		SIGNAL(clicked()), this, SLOT(accept()));
    QObject::connect(PushButtonCancel,	SIGNAL(clicked()), this, SLOT(reject()));
    QObject::connect(mUi_treeWidget,	SIGNAL(doubleClicked(QModelIndex)),		this, SLOT(accept()));

    // Empty Filter list
    mUi_treeWidget->clear();

    // Fill the list with sorting enabled
    mUi_treeWidget->sortItems(0,Qt::AscendingOrder);	// Sorting on the first column.

    // Set minimum size for the Value column
    mUi_treeWidget->setColumnWidth(0,50);
}

///////////////////////////////////////////////////////////
// Fills List with given list of values
///////////////////////////////////////////////////////////
void PickProductIdSQLDialog::fillList(QStringList & cList, bool bAllowMultiSelections, bool bMultiTestingStage)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString(" with %1 products, %2...")
          .arg(cList.size())
          .arg(bMultiTestingStage?"(MultiTS)":"(SingleTS)").toLatin1().constData());
    //PickProductIdSQLDialogItem* ptTreeItem;
    QString					strItem;
    QStringList				strlElements;
    QStringList::Iterator	it1, it2;

    // Empty test list
    mUi_treeWidget->clear();

    // Check if specified list is empty
    if(cList.isEmpty())
    {
        QStringList strList(QString("Product list is empty!!"));
        /*ptTreeItem = */new PickProductIdSQLDialogItem(mUi_treeWidget,strList);
        PushButtonOk->setEnabled(false);
        return;
    }

    // Save multi-testingstage flag
    m_bMultiTestingStage = bMultiTestingStage;

    // Set dialog text depending on selection mode, and multi-testingstage mode
    QString strText;
    if(bAllowMultiSelections == true)
    {
        // Allow multiple selections: keep default text + behavior!
        mUi_treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
        strText = "Pick one product...or hold the 'Ctrl' key for multiple selections, ";
        strText += "or hold the 'Shift' key for a range selection.";
    }
    else
    {
        // Single selection mode...set text + Widget flags
        mUi_treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        strText = "Pick ONE product from the list...";
    }


    if(bMultiTestingStage)
        strText += "\n\nEach column corresponds to a testing stage.";

    TextLabel->setText(strText);

    // Read list of product values, so to fill listView.
    if(bMultiTestingStage)
    {
        // In multi-testingstage mode, first item is the column names,
        // and each item has as many values as columns (testing stages), separated by a ","
        it1 = cList.begin();
        strItem = *it1;
        GSLOG(SYSLOG_SEV_DEBUG, QString(" adding first : %1...").arg( strItem).toLatin1().constData());
        strlElements = strItem.split(',', QString::SkipEmptyParts);

        QTreeWidgetItem *ptHeaderTreeItem = new QTreeWidgetItem(strlElements);
        mUi_treeWidget->setHeaderItem(ptHeaderTreeItem);

        for(it1++; it1 != cList.end(); ++it1 )
        {
            strItem = *it1;
            strlElements = strItem.split(',', QString::KeepEmptyParts);
            /*ptTreeItem = */new PickProductIdSQLDialogItem(mUi_treeWidget,strlElements);
        }

    }
    else
    {
        // In single testing stage mode, each item is a product
        QTreeWidgetItem *ptHeaderTreeItem = new QTreeWidgetItem();
        ptHeaderTreeItem->setText( (mUi_treeWidget->columnCount()),QString::fromUtf8("ProductID") );
        mUi_treeWidget->setHeaderItem(ptHeaderTreeItem);

        (mUi_treeWidget->header())->hide();

        for(it1 = cList.begin(); it1 != cList.end(); ++it1 )
        {
            strItem = *it1;
            /*ptTreeItem = */new PickProductIdSQLDialogItem(mUi_treeWidget,QStringList(strItem));
        }
    }

    // Select first item
    mUi_treeWidget->setCurrentItem(mUi_treeWidget->topLevelItem(0));


    // Sort list
    for(int ii=0; ii<mUi_treeWidget->columnCount(); ii++)
        mUi_treeWidget->resizeColumnToContents(ii);
}

///////////////////////////////////////////////////////////
// Return list of selected product(s)
///////////////////////////////////////////////////////////
void PickProductIdSQLDialog::getProductList(QStringList & cList)
{
    QTreeWidgetItem *ptTreeItem;
    QString			strItem;

    QList<QTreeWidgetItem *>	qlistSelectedItems = mUi_treeWidget->selectedItems();

    cList.clear();

    // Return empty string if no products selected
    if(qlistSelectedItems.count() == 0)
        return;


    // Pointer to first selection (if any)
    ptTreeItem = qlistSelectedItems.at(0);

    for(int nCount = 0; nCount < qlistSelectedItems.count(); nCount++)
    {
        ptTreeItem = qlistSelectedItems.at(nCount);
        if(m_bMultiTestingStage && (nCount == 0))
        {
            // In multi-testingstage mode, first add column titles as first element of the list
            strItem = (mUi_treeWidget->headerItem())->text(0);

            for(int i=1; i<mUi_treeWidget->columnCount(); i++)
            {
                strItem += ",";
                strItem += (mUi_treeWidget->headerItem())->text(i);
            }
            cList.append(strItem);
        }

        strItem = ptTreeItem->text(0);
        if(m_bMultiTestingStage)
        {
            // In multi-testingstage mode, concatenate all columns
            for(int i=1; i < (mUi_treeWidget->columnCount()) ; i++)
            {
                strItem += ",";
                strItem += ptTreeItem->text(i);
            }
        }
        cList.append(strItem);
    }
}


///////////////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
PickProductIdSQLDialogItem::PickProductIdSQLDialogItem(QTreeWidget *pTreeWidget, QStringList strTreeListItem) : QTreeWidgetItem( pTreeWidget, strTreeListItem )
{
}

PickProductIdSQLDialogItem::PickProductIdSQLDialogItem(QStringList strTreeListItem) : QTreeWidgetItem(strTreeListItem)
{
}

PickProductIdSQLDialogItem::PickProductIdSQLDialogItem(QTreeWidget *pTreeWidget) : QTreeWidgetItem(pTreeWidget)
{
}

PickProductIdSQLDialogItem::~PickProductIdSQLDialogItem()
{
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool PickProductIdSQLDialogItem::operator< ( const QTreeWidgetItem & other ) const
//
// Description	:	Override the comparison methods
//
///////////////////////////////////////////////////////////////////////////////////
bool PickProductIdSQLDialogItem::operator< ( const QTreeWidgetItem & other ) const
{
    int		nCol		= treeWidget() ? treeWidget()->sortColumn() : 0;
    QString strLeft		= text(nCol);
    QString strRight	= other.text(nCol);

    switch(nCol)
    {
    case 0 :
        return (QString::compare(strLeft,strRight,Qt::CaseInsensitive) < 0);
    default:
        GEX_ASSERT(false);
        return false;
    }
}

