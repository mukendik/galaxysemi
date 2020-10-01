#include "drill_table.h"
#include "ctest.h"
#include "gex_report.h"
#include "engine.h"

extern CGexReport* gexReport;
//extern const char * const cUnitColumnLabel;

///////////////////////////////////////////////////////////
// Functional tests Table constructor
///////////////////////////////////////////////////////////
DrillFunctionalTable::DrillFunctionalTable(GexWizardTable *parentWizard, QWidget * parent, Loading* loading)
    : DrillTable(parentWizard, parent, loading)
{
    setSortingEnabled(true);
    setGeometry( QRect( 21, 79, 620, 255 ) );
}

///////////////////////////////////////////////////////////
// Reset table: Empties table, then loads it with all Functional tests info
///////////////////////////////////////////////////////////
void DrillFunctionalTable::ResetTable(void)
{
    QHeaderView *pTableHeader=0;
    DrillTableItem *pCell=0;
    CTest *ptTestCell=0;
    char szString[2048]="";
    int	iRow=0,iCol=0;

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

    // Test #, name, Total Fails, Vector name, Vector fail count
    // Get pointer to first group & first file (we always have them exist)
    QListIterator<CGexGroupOfFiles*>	itGroupsList(gexReport->getGroupsList());
    CGexGroupOfFiles *					pGroup = NULL;

    // Set number of rows in table counter
    iRow=0;
    while(itGroupsList.hasNext())
    {
        pGroup		= itGroupsList.next();
        ptTestCell	= pGroup->cMergedData.ptMergedTestList;

        while(ptTestCell != NULL)
        {
            // Only read data from FTR records
            if(ptTestCell->bTestType != 'F')
                goto NextCell;

            // this is a valid test, count it!
            iRow++;

            // Point to next test cell
            NextCell:
            ptTestCell = ptTestCell->GetNextTest();
        };
    };
    setRowCount(iRow*gexReport->getGroupsList().count());

    setColumnCount(9);	// Total of 8 columns: ranging 0..8
    iCol = 0;
    QStringList headerList;
    headerList.append("Test");
    headerList.append("Name");
    headerList.append("Group/Dataset");
    headerList.append("Type");
    headerList.append("Pattern name");
    headerList.append("Exec.");
    headerList.append("Fail");
    headerList.append("Fail %");
    headerList.append("Flow ID");

    setHorizontalHeaderLabels(headerList);

#ifdef _WIN32
    // Resize columns so most of columns fit in the window...
    setColumnWidth (0,60);	// Test# : column width
    setColumnWidth (1,140);	// Testname : column width
    setColumnWidth (2,100);	// Group/Dataset : column width
    setColumnWidth (3,40);	// Test type : column width
    setColumnWidth (4,140);	// vector name : column width
    setColumnWidth (5,80);	// Total execs : column width
    setColumnWidth (6,80);	// vector fail count column width
    setColumnWidth (7,80);	// vector fail % of production
    setColumnWidth (8,50);	// Test Flow ID
#endif

    // List of tests ...for all groups
    CGexFileInGroup *pFile;

    pGroup = gexReport->getGroupsList().isEmpty()?NULL:gexReport->getGroupsList().first();
    ptTestCell = pGroup->cMergedData.ptMergedTestList;
    iRow=0;
    CTest	*ptOtherTestCell;
    char	szTestName[1024];
    int		iGroup;
    long	lPinmapIndex;
    double	lfData;
    QMap<QString,CFunctionalTest>::Iterator it;	// Used to display all vectors
    CFunctionalTest	cFunctTest;
    while(ptTestCell != NULL)
    {
        // Only read data from FTR records
        if(ptTestCell->bTestType != 'F')
            goto NextTestCell;

        // First: write info about reference group#1
        itGroupsList.toFront();
        pGroup = NULL;
        iGroup = 0;

        // Create one stats line per test of each group...
        while(itGroupsList.hasNext())
        {
            // List of tests in group#2 or group#3, etc...
            pGroup	= itGroupsList.next();
            pFile	= (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

            if(ptTestCell->lPinmapIndex >= 0)
                lPinmapIndex = ptTestCell->lPinmapIndex;
            else
                lPinmapIndex = GEX_FTEST;

            // Find pointer to Test Cell in group#x.
            if(pFile->FindTestCell(ptTestCell->lTestNumber,lPinmapIndex,&ptOtherTestCell,false,false, ptTestCell->strTestName.toLatin1().data()) !=1)
                goto NextGroup;

            for ( it = ptOtherTestCell->mVectors.begin(); it != ptOtherTestCell->mVectors.end(); ++it )
            {
                // if nb row is greater than previously setted, update it
                if (iRow >= verticalHeader()->count())
                    setRowCount(iRow+1);

                // Get functional test vector info
                cFunctTest = *it;

                // Insert test info into the table...
                iCol = 0;
                // Test #
                pCell = new DrillTableItem( QTableWidgetItem::Type, ptOtherTestCell->szTestLabel, iRow);
                setItem( iRow, iCol++, pCell );

                // Test name
                gexReport->BuildTestNameString(pFile,ptOtherTestCell,szTestName);
                pCell = new DrillTableItem( QTableWidgetItem::Type, szTestName, iRow);
                setItem( iRow, iCol++, pCell );

                // Group name / Dataset
                pCell = new DrillTableItem( QTableWidgetItem::Type, pGroup->strGroupName, iRow);
                setItem( iRow, iCol++, pCell );

                // Test type.
                gexReport->BuildTestTypeString(pFile,ptOtherTestCell,szString,true);
                pCell = new DrillTableItem( QTableWidgetItem::Type, szString, iRow);
                setItem( iRow, iCol++, pCell );

                // Vector name.
                pCell = new DrillTableItem( QTableWidgetItem::Type, cFunctTest.strVectorName, iRow);
                setItem( iRow, iCol++, pCell );

                // Exec. count
                //pCell = new DrillTableItem( QTableItem::OnTyping, gexReport->CreateResultString(ptOtherTestCell->ldExecs));
                pCell = new DrillTableItem( QTableWidgetItem::Type, gexReport->CreateResultString(cFunctTest.lExecs), iRow);
                setItem( iRow, iCol++, pCell );

                // Fail count
                //pCell = new DrillTableItem( QTableItem::OnTyping, gexReport->CreateResultString(ptOtherTestCell->ldFailCount));
                pCell = new DrillTableItem( QTableWidgetItem::Type, gexReport->CreateResultString(cFunctTest.lFails), iRow);
                setItem( iRow, iCol++, pCell );

                // Fail count %
                if(cFunctTest.lExecs > 0)
                {
                    lfData = (100.0*cFunctTest.lFails)/cFunctTest.lExecs;
                    sprintf(szString,"%.2f",lfData);
                }
                else
                    strcpy(szString,"0");
                pCell = new DrillTableItem( QTableWidgetItem::Type, szString, iRow);
                setItem( iRow, iCol++, pCell );

                // Flow ID
                // -- replace -1 by n/a . in order to have the galaxy test (-1) in the end of the list when ascending order
                if(ptOtherTestCell->lTestFlowID <0)
                    strcpy(szString, "n/a .");
                else
                    sprintf(szString, "%d", ptOtherTestCell->lTestFlowID);
                pCell = new DrillTableItem( QTableWidgetItem::Type, szString, iRow);
                setItem( iRow, iCol++, pCell );

                // Update row index.
                iRow++;
            }

            // Point to next group
            NextGroup:
            iGroup++;
        };

        // Point to next test cell
        NextTestCell:
        ptTestCell = ptTestCell->GetNextTest();

    };	// Loop until all test cells read.

    // Refresh exact number of rows.
    setRowCount(iRow);

    QString lSortOption = ReportOptions.GetOption( QString("statistics"), QString("sorting") ).toString();

    if(lSortOption == "test_number")
        sortColumn(0, true, true);
    else if(lSortOption == "test_flow_id")
        sortColumn(8, true, true);
    else if(lSortOption == "test_name")
        sortColumn(1, true, true);
}

///////////////////////////////////////////////////////////
// A column has to be sorted...
///////////////////////////////////////////////////////////
void DrillFunctionalTable::sortColumn(int col, bool ascending,
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
    setCurrentCell(0,1);
}
