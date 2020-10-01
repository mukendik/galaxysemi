#include "drill_table.h"
#include "gex_report.h"
#include "cbinning.h"
#include "product_info.h"

extern GexMainwindow *	pGexMainWindow;
extern CGexReport *gexReport; // Handle to report class

///////////////////////////////////////////////////////////
// Binning Table constructor
///////////////////////////////////////////////////////////
DrillBinningTable::DrillBinningTable(GexWizardTable *parentWizard, QWidget * parent, Loading* loading)
    : DrillTable(parentWizard, parent, loading)
{
    setSortingEnabled(true);
    setGeometry( QRect( 21, 79, 620, 255 ) );
}

///////////////////////////////////////////////////////////
// Reset table: Empties table, then loads it with all BINNING info
///////////////////////////////////////////////////////////
void DrillBinningTable::ResetTable(bool bSoftwareBinning)
{
    QHeaderView			*pTableHeader;
    DrillTableItem	*pCell;
    char	szString[2048];
    int	iRow,iCol;

    // Quiet return if no data & report available!
    if(gexReport == NULL)
        return;

    // Get pointer to horizontal header: show that Test# are sorted in ascending order (default)
    pTableHeader = horizontalHeader();
    pTableHeader->setSortIndicator(0, Qt::AscendingOrder);
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Delete content of table if any...
    for(iRow=0;iRow< rowCount(); iRow++)
    {
        for(iCol=0;iCol< columnCount(); iCol++)
        {
            pCell = (DrillTableItem*)item(iRow,iCol);
            if(pCell != NULL)
            {
                takeItem(iRow, iCol);
                delete pCell;
            }
        }
    }

    // Get pointer to first group & first file (we always have them exist)
    CBinning *ptBinCell;
    iRow=0;
    long	lCumulPassBins;
    long	lCumulFailBins;
    long	lTotalBins;

    QListIterator<CGexGroupOfFiles*>	itGroupsList(gexReport->getGroupsList());
    CGexGroupOfFiles *					pGroup = NULL;

    // Counts total number of lines for all groups.
    while(itGroupsList.hasNext())
    {
        pGroup = itGroupsList.next();

        // Set number of rows in table counter
        if(bSoftwareBinning == true)
        {
            ptBinCell = pGroup->cMergedData.ptMergedSoftBinList;
            lTotalBins = pGroup->cMergedData.lTotalSoftBins;
        }
        else
        {
            ptBinCell = pGroup->cMergedData.ptMergedHardBinList;
            lTotalBins = pGroup->cMergedData.lTotalHardBins;
        }

        lCumulPassBins=0;
        lCumulFailBins=0;

        while(ptBinCell != NULL)
        {
            // keep track of the number of Bin classes
            if(ptBinCell->ldTotalCount > 0)
            {
                iRow++;

                // Keep track of cumul of PASS / FAIL bins.
                if(toupper(ptBinCell->cPassFail) == 'P')
                    lCumulPassBins+= ptBinCell->ldTotalCount;	// Compute cumul of all PASS bins
                if(toupper(ptBinCell->cPassFail) == 'F')
                    lCumulFailBins+= ptBinCell->ldTotalCount;	// Compute cumul of all FAIL bins
            }

            // Point to next Bin cell
            ptBinCell = ptBinCell->ptNextBin;
        };

        // One more row to show Cumul of PASS Bins
        if(lCumulPassBins > 0)
            iRow++;

        // One more row to show Cumul of FAIL Bins
        if(lCumulFailBins > 0)
            iRow++;

        // One additional row for the Cumul of Bins in each group.
        if(lTotalBins)
            iRow++;
    };
    setRowCount(iRow);

    setColumnCount(6);

    QStringList headerList;
    if(bSoftwareBinning == true)
        headerList.append("Software Bin");
    else
        headerList.append("Hardware Bin");
    headerList.append("Group/Dataset");
    headerList.append("Bin name");
    headerList.append("Pass/Fail");
    headerList.append("Total Count");
    headerList.append("Percentage");
    setHorizontalHeaderLabels(headerList);

#ifdef _WIN32
    // Resize columns so most of columns fit in the window...
    setColumnWidth (0,80);	// Bin# : column width
    setColumnWidth (1,100);	// Group/Dataset : column width
    setColumnWidth (2,160);	// BinName : column width
    setColumnWidth (3,60);	// Pass/Fail info (original): column width
    setColumnWidth (4,80);	// Total count : column width
    setColumnWidth (5,80);	// Percentage: column width
#endif

    QString strString;
    float	fPercentage;

    // Rewind row count & rewind to first group.
    iRow=0;

    // Insert Bin info into the table...
    itGroupsList.toFront();
    pGroup = NULL;

    while(itGroupsList.hasNext())
    {
        pGroup = itGroupsList.next();

        // Reset cumul counter in group.
        lCumulPassBins=0;
        lCumulFailBins=0;

        if(bSoftwareBinning == true)
        {
            ptBinCell = pGroup->cMergedData.ptMergedSoftBinList;
            lTotalBins = pGroup->cMergedData.lTotalSoftBins;
        }
        else
        {
            ptBinCell = pGroup->cMergedData.ptMergedHardBinList;
            lTotalBins = pGroup->cMergedData.lTotalHardBins;
        }

        while(ptBinCell != NULL)
        {
            if(ptBinCell->ldTotalCount <= 0)
                goto next_cell;

            // Bin #
            pCell = new DrillTableItem( QTableWidgetItem::Type, gexReport->CreateResultString(ptBinCell->iBinValue), iRow);
            setItem( iRow, 0, pCell );

            // Group name / Dataset
            pCell = new DrillTableItem( QTableWidgetItem::Type, pGroup->strGroupName, iRow);
            setItem( iRow, 1, pCell );

            // Bin name
            if(ptBinCell->strBinName.isEmpty())
                strString="-";
            else
                strString = ptBinCell->strBinName;
            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
            setItem( iRow, 2, pCell );

            // Pass/Fail
            sprintf(szString,"%c",ptBinCell->cPassFail);
            pCell = new DrillTableItem( QTableWidgetItem::Type, szString, iRow);
            setItem( iRow, 3, pCell );

            // Total count
            pCell = new DrillTableItem( QTableWidgetItem::Type, gexReport->CreateResultString(ptBinCell->ldTotalCount), iRow);
            setItem( iRow, 4, pCell );

            // Percentage
            fPercentage = (100.0*ptBinCell->ldTotalCount)/(lTotalBins);
            if((fPercentage < 0.0) || (fPercentage > 100.0) || (lTotalBins <= 0))
            {
                // Happens when corrupted binning data (as seen on LTX Fusion systems!).
                strcpy(szString,GEX_NA);
            }
            else
                sprintf(szString,"%.2f %%",fPercentage);
            pCell = new DrillTableItem( QTableWidgetItem::Type, szString, iRow);
            setItem( iRow, 5, pCell );

            // Keep track of cumul of PASS / FAIL bins.
            if(toupper(ptBinCell->cPassFail) == 'P')
                lCumulPassBins+= ptBinCell->ldTotalCount;	// Compute cumul of all PASS bins
            if(toupper(ptBinCell->cPassFail) == 'F')
                lCumulFailBins+= ptBinCell->ldTotalCount;	// Compute cumul of all FAIL bins

            // Update row index.
            iRow++;

            // Point to next bin cell
next_cell:
            ptBinCell = ptBinCell->ptNextBin;
        };	// Loop until all Bin cells read.

        // Write Cumul of PASS BINS in group.
        if(lCumulPassBins > 0)
        {
            // Bin #
            pCell = new DrillTableItem( QTableWidgetItem::Type, "All PASS Bins", iRow);
            setItem( iRow, 0, pCell );

            // Group name / Dataset
            pCell = new DrillTableItem( QTableWidgetItem::Type, pGroup->strGroupName, iRow);
            setItem( iRow, 1, pCell );

            // Bin name
            pCell = new DrillTableItem( QTableWidgetItem::Type, "All PASS Bins", iRow);
            setItem( iRow, 2, pCell );

            // Pass/Fail
            pCell = new DrillTableItem( QTableWidgetItem::Type, "P", iRow);
            setItem( iRow, 3, pCell );

            // Total count
            pCell = new DrillTableItem( QTableWidgetItem::Type, gexReport->CreateResultString(lCumulPassBins), iRow);
            setItem( iRow, 4, pCell );

            // Percentage
            fPercentage = (100.0*lCumulPassBins)/(lTotalBins);

                sprintf(szString,"%.2f %%",fPercentage);
            pCell = new DrillTableItem( QTableWidgetItem::Type, szString, iRow);
            setItem( iRow, 5, pCell );

            // Update row index.
            iRow++;
        }

        // Write Cumul of FAIL BINS in group.
        if(lCumulFailBins > 0)
        {
            // Bin #
            pCell = new DrillTableItem( QTableWidgetItem::Type, "All FAIL Bins", iRow);
            setItem( iRow, 0, pCell );

            // Group name / Dataset
            pCell = new DrillTableItem( QTableWidgetItem::Type, pGroup->strGroupName, iRow);
            setItem( iRow, 1, pCell );

            // Bin name
            pCell = new DrillTableItem( QTableWidgetItem::Type, "All FAIL Bins", iRow);
            setItem( iRow, 2, pCell );

            // Pass/Fail
            pCell = new DrillTableItem( QTableWidgetItem::Type, "F", iRow);
            setItem( iRow, 3, pCell );

            // Total count
            pCell = new DrillTableItem( QTableWidgetItem::Type, gexReport->CreateResultString(lCumulFailBins), iRow);
            setItem( iRow, 4, pCell );

            // Percentage
            fPercentage = (100.0*lCumulFailBins)/(lTotalBins);

                sprintf(szString,"%.2f %%",fPercentage);
            pCell = new DrillTableItem( QTableWidgetItem::Type, szString, iRow);
            setItem( iRow, 5, pCell );

            // Update row index.
            iRow++;
        }

        // Write Cumul in group.
        if(lTotalBins > 0)
        {
            // Bin #
            pCell = new DrillTableItem( QTableWidgetItem::Type, "Cumul.", iRow);
            setItem( iRow, 0, pCell );

            // Group name / Dataset
            pCell = new DrillTableItem( QTableWidgetItem::Type, pGroup->strGroupName, iRow);
            setItem( iRow, 1, pCell );

            // Bin name
            pCell = new DrillTableItem( QTableWidgetItem::Type, "All Bins", iRow);
            setItem( iRow, 2, pCell );

            // Pass/Fail
            pCell = new DrillTableItem( QTableWidgetItem::Type, "-", iRow);
            setItem( iRow, 3, pCell );

            // Total count
            pCell = new DrillTableItem( QTableWidgetItem::Type, gexReport->CreateResultString(lTotalBins), iRow);
            setItem( iRow, 4, pCell );

            // Percentage
            pCell = new DrillTableItem( QTableWidgetItem::Type, "100.00 %", iRow);
            setItem( iRow, 5, pCell );

            // Update row index.
            iRow++;
        }
    };
}

///////////////////////////////////////////////////////////
// A column has to be sorted...
///////////////////////////////////////////////////////////
void DrillBinningTable::sortColumn(int col, bool ascending,
                                   bool /*wholeRows*/)
{
    QHeaderView *pTableHeader;

    Qt::SortOrder order = Qt::AscendingOrder;
    if (!ascending) order = Qt::DescendingOrder;

    // Get pointer to horizontal header
    pTableHeader = horizontalHeader();
    pTableHeader->setSortIndicator(col,order);

    // do sort
    QTableWidget::sortItems( col, order);

    // Set focus on first cell in the column!
    setCurrentCell(0,col);
}

