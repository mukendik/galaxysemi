#include <gqtl_log.h>
#include "ctest.h"
#include "report_options.h"
#include "gex_report.h"
#include "pat_engine.h"
#include "engine.h"
#include "patman_lib.h"
#include "drill_parametric_table.h"
#include "component.h"

extern CGexReport *gexReport;			// Handle to report class


QMap<QString, QString> DrillParametricTable::mSortCol;

///////////////////////////////////////////////////////////
// Statistics Table constructor
///////////////////////////////////////////////////////////
DrillParametricTable::DrillParametricTable(GexWizardTable *parentWizard, QWidget * parent, Loading* lLoading)
    : DrillTable(parent, lLoading), mHideFunctionalTests(false)
{
    setSortingEnabled(true);
    setGeometry( QRect( 21, 79, 620, 255 ) );

    mParentWizard = static_cast<GexWizardTable *>(parentWizard);
    mSortCol["test_number"]     = TEST_TNUMBER;
    mSortCol["test_name"]       = TEST_TNAME;
    mSortCol["test_flow_id"]    = TEST_TESTFLOWID;
    mSortCol["fail_count"]      = TEST_FAIL;
    mSortCol["mean"]            = TEST_MEAN;
    mSortCol["mean_shift"]      = TEST_MEANSHIFT;
    mSortCol["sigma"]           = TEST_SIGMA;
    mSortCol["cp"]              = TEST_CP;
    mSortCol["cpk"]             = TEST_CPK;
}

DrillParametricTable::~DrillParametricTable()
{

}

void DrillParametricTable::SetHideFunctionalTests(bool hideFunctionalTests)
{
    mHideFunctionalTests = hideFunctionalTests;
}

void DrillParametricTable::onHideFunctionalTests(bool bHideFunctionalTests)
{
    SetHideFunctionalTests(bHideFunctionalTests);

    // Hide/Show functional tests row
    mLoading->start(200);
    ResetTable();
    mLoading->stop();
}

QString DrillParametricTable::GetItemValue(int line, int column)
{
    QTableWidgetItem* lItem = item(line, column);
    if(lItem)
        return lItem->text();
    return "";
}

void DrillParametricTable::toJson(QJsonObject& element, bool capabilityTable)
{
    //-- serialize the group list
    GroupsListToJson(element);
    //-- serialize the test list
    TestsListToJson(element);

    //-- serialiez the fields list
    if(capabilityTable)
    {
        //-- serialize specific capability table fields
        QJsonArray lFieldsList;
        lFieldsList.append(TEST_LTL);
        lFieldsList.append(TEST_HTL);
        lFieldsList.append(TEST_EXEC);
        lFieldsList.append(TEST_MEAN);
        lFieldsList.append(TEST_SIGMA);
        lFieldsList.append(TEST_CP);
        lFieldsList.append(TEST_CPK);
        lFieldsList.append(TEST_CPKL);
        lFieldsList.append(TEST_CPKH);
        element.insert("Fields", lFieldsList);
    }
    else
    {
        //-- serialize all the displayed column
        FieldsListToJson(element);
    }
}

///< serialize the fields list to the json format
void DrillParametricTable::FieldsListToJson(QJsonObject& lElt)
{
    QJsonArray lFieldsList;
    for (int lColumn=0; lColumn< columnCount(); ++lColumn)
    {
        // -- check if colum index exist and if it is displayed
        if(horizontalHeaderItem(lColumn) && isColumnHidden(lColumn) == false)
        {
            lFieldsList.append(ColIdentifier(lColumn));
        }
    }

    lElt.insert("Fields", lFieldsList);
}

void DrillParametricTable::TestsListToJson(QJsonObject& lElt)
{
    QJsonArray lTestList;

    int lTestNumberColumn = mColPosition.value(TEST_TNUMBER);
    int lTestNameColumn   = mColPosition.value(TEST_TNAME);
    int lTestGroupColumn  = mColPosition.value(TEST_GROUP);
    QList<CGexGroupOfFiles*> lGroupList = gexReport->getGroupsList();

    for (int lLine=0; lLine<this->rowCount(); ++lLine)
    {
        Test lTest;
        lTest.mNumber = GetItemValue(lLine, lTestNumberColumn);
        lTest.mName   = GetItemValue(lLine, lTestNameColumn);

        if (item(lLine,lTestGroupColumn))
        {
            if (lGroupList.size() == 1)
                lTest.mGroupId = 0;
            else
            {
                QString lGroupName = item(lLine,lTestGroupColumn)->text();
                for (int i=0; i<lGroupList.size(); ++i)
                {
                    if (lGroupList[i] && lGroupList[i]->strGroupName.compare(lGroupName, Qt::CaseInsensitive) == 0)
                    {
                        lTest.mGroupId = lGroupList[i]->GetGroupId();
                        break;
                    }
                }
            }
        }
        lTestList.append(lTest.toJson());
        // TO DO: if we want in the future to add only selected items
        //if(table->item(lLine, 0) && table->item(lLine, 0)->isSelected())
    }

    lElt.insert("Tests", lTestList);
}

void DrillParametricTable::GroupsListToJson(QJsonObject& lElt)
{
    QList<CGexGroupOfFiles*> lGroupFileList = gexReport->getGroupsList();
    QJsonArray lGroupsList;

    for (int lGroupId = 0; lGroupId < lGroupFileList.size(); ++lGroupId)
    {
        if (lGroupFileList[lGroupId])
        {
            Group lGroup;
            lGroup.mNumber = QString::number(lGroupId);
            lGroup.mName   = lGroupFileList[lGroupId]->strGroupName;
            lGroupsList.append(lGroup.toJson());
        }
    }

    lElt.insert("Groups", lGroupsList);
}

void DrillParametricTable::InitShiftColumnLabels()
{
    int lMaxGroupToCheck = 1;
    if ((ReportOptions.GetOption("statistics","shift_with")).toString() == "shift_to_all")
    {
        lMaxGroupToCheck = gexReport->getGroupsList().size();
    }

    for(int lIdx = 0; lIdx < lMaxGroupToCheck; ++lIdx)
    {
        if (gexReport->getGroupsList()[lIdx])
        {
            QString lGroupName = gexReport->getGroupsList()[lIdx]->strGroupName;
            if (!columnsLabel.contains(TEST_MEANSHIFT + lGroupName))
            {
                columnsLabel.insert(TEST_MEANSHIFT + lGroupName,
                                    columnsLabel.value(TEST_MEANSHIFT) + " (Ref: " + lGroupName + ")");
                columnsLabel.insert(TEST_SIGMASHIFT + lGroupName,
                                    columnsLabel.value(TEST_SIGMASHIFT) + " (Ref: " + lGroupName + ")");
                columnsLabel.insert(TEST_CPSHIFT + lGroupName,
                                    columnsLabel.value(TEST_CPSHIFT) + " (Ref: " + lGroupName + ")");
                columnsLabel.insert(TEST_CRSHIFT + lGroupName,
                                    columnsLabel.value(TEST_CRSHIFT) + " (Ref: " + lGroupName + ")");
                columnsLabel.insert(TEST_CPKSHIFT + lGroupName,
                                    columnsLabel.value(TEST_CPKSHIFT) + " (Ref: " + lGroupName + ")");
            }
        }
    }
}

void DrillParametricTable::InitColumnPositions()
{
    mColPosition.clear();

    // Compute column count
    int lColumnCount = DRILL_PTEST_TOTAL_STATIC_COLS;

    // Add dynamic columns
    // multi limits
   // lColumnCount += gexReport->GetMaxMultiLimitItems()*2; // LL + HL

    setColumnCount(lColumnCount);

    int lColumnIndex = 0;
    // set columns positions
    mColPosition.insert(TEST_TNUMBER, lColumnIndex);
    mColPosition.insert(TEST_TNAME, ++lColumnIndex);
    mColPosition.insert(TEST_GROUP, ++lColumnIndex);
    mColPosition.insert(TEST_TYPE, ++lColumnIndex);
    mColPosition.insert(TEST_LTL, ++lColumnIndex);
    mColPosition.insert(TEST_HTL, ++lColumnIndex);
    mColPosition.insert(TEST_LSL, ++lColumnIndex);
    mColPosition.insert(TEST_HSL, ++lColumnIndex);
    mColPosition.insert(TEST_DRIFTL, ++lColumnIndex);
    mColPosition.insert(TEST_DRIFTLOW, ++lColumnIndex);
    mColPosition.insert(TEST_DRIFTHIGH, ++lColumnIndex);
   // mColPosition.insert(QString("ll"), ++lColumnIndex);
   // mColPosition.insert(QString("hl"), ++lColumnIndex);
    mColPosition.insert(TEST_SHAPE, ++lColumnIndex);
    mColPosition.insert(TEST_STATS_SRC, ++lColumnIndex);
    mColPosition.insert(TEST_EXEC, ++lColumnIndex);
    mColPosition.insert(TEST_FAIL, ++lColumnIndex);
    mColPosition.insert(TEST_FAILPERCENT, ++lColumnIndex);
    mColPosition.insert(TEST_FAILBIN, ++lColumnIndex);
    mColPosition.insert(TEST_TESTFLOWID, ++lColumnIndex);
    mColPosition.insert(TEST_OUTLIER, ++lColumnIndex);
    mColPosition.insert(TEST_MEAN, ++lColumnIndex);
    InsertShiftPosition(TEST_MEANSHIFT, lColumnIndex);
    mColPosition.insert(TEST_T_TEST, ++lColumnIndex);
    mColPosition.insert(TEST_SIGMA, ++lColumnIndex);
    InsertShiftPosition(TEST_SIGMASHIFT, lColumnIndex);
    mColPosition.insert(TEST_2_SIGMA	, ++lColumnIndex);
    mColPosition.insert(TEST_3_SIGMA	, ++lColumnIndex);
    mColPosition.insert(TEST_6_SIGMA, ++lColumnIndex);
    mColPosition.insert(TEST_MIN, ++lColumnIndex);
    mColPosition.insert(TEST_MAX, ++lColumnIndex);
    mColPosition.insert(TEST_RANGE, ++lColumnIndex);
    mColPosition.insert(TEST_MAX_RANGE, ++lColumnIndex);
    mColPosition.insert(TEST_CP, ++lColumnIndex);
    mColPosition.insert(TEST_CR, ++lColumnIndex);
    InsertShiftPosition(TEST_CPSHIFT, lColumnIndex);
    InsertShiftPosition(TEST_CRSHIFT, lColumnIndex);
    mColPosition.insert(TEST_CPK, ++lColumnIndex);
    mColPosition.insert(TEST_CPKL, ++lColumnIndex);
    mColPosition.insert(TEST_CPKH, ++lColumnIndex);
    InsertShiftPosition(TEST_CPKSHIFT, lColumnIndex);
    mColPosition.insert(TEST_YIELD, ++lColumnIndex);
    mColPosition.insert(TEST_GAGE_EV, ++lColumnIndex);
    mColPosition.insert(TEST_GAGE_AV, ++lColumnIndex);
    mColPosition.insert(TEST_GAGE_RR, ++lColumnIndex);
    mColPosition.insert(TEST_GAGE_GB, ++lColumnIndex);
    mColPosition.insert(TEST_GAGE_PV, ++lColumnIndex);
    mColPosition.insert(TEST_GAGE_TV, ++lColumnIndex);
    mColPosition.insert(TEST_GAGE_P_T, ++lColumnIndex);
    mColPosition.insert(TEST_SKEW, ++lColumnIndex);
    mColPosition.insert(TEST_KURTOSIS, ++lColumnIndex);
    mColPosition.insert(TEST_P0_5, ++lColumnIndex);
    mColPosition.insert(TEST_P2_5, ++lColumnIndex);
    mColPosition.insert(TEST_P10, ++lColumnIndex);
    mColPosition.insert(TEST_Q1, ++lColumnIndex);
    mColPosition.insert(TEST_Q2, ++lColumnIndex);
    mColPosition.insert(TEST_Q3, ++lColumnIndex);
    mColPosition.insert(TEST_P90, ++lColumnIndex);
    mColPosition.insert(TEST_P97_5, ++lColumnIndex);
    mColPosition.insert(TEST_P99_5, ++lColumnIndex);
    mColPosition.insert(TEST_IQR, ++lColumnIndex);
    mColPosition.insert(TEST_SIGMAIQR, ++lColumnIndex);
}

void DrillParametricTable::InsertShiftPosition(const QString &columnName, int& lColumnIndex)
{
    int lMaxGroupToCheck = 1;
    if ((ReportOptions.GetOption("statistics","shift_with")).toString() == "shift_to_all")
    {
        lMaxGroupToCheck = gexReport->getGroupsList().size();
    }
    QString lKeyName;
    for (int lIdx = 0; lIdx < lMaxGroupToCheck; ++lIdx)
    {
        if (gexReport->getGroupsList()[lIdx])
        {
            lKeyName = columnName + gexReport->getGroupsList()[lIdx]->strGroupName;
            mColPosition.insert(lKeyName, ++lColumnIndex);
        }
        else
        {
            GSLOG(SYSLOG_SEV_ERROR, "Invalid group");
        }
    }
}

void DrillParametricTable::InitShiftHorizontalHeader(const QString &columnName)
{
    int lMaxGroupToCheck = 1;
    if ((ReportOptions.GetOption("statistics","shift_with")).toString() == "shift_to_all")
    {
        lMaxGroupToCheck = gexReport->getGroupsList().size();
    }
    for(int lIdx = 0; lIdx < lMaxGroupToCheck; ++lIdx)
    {
        if (gexReport->getGroupsList()[lIdx])
        {
            QString lKeyName = columnName + gexReport->getGroupsList()[lIdx]->strGroupName;
            setHorizontalHeaderItem(ColPosition(lKeyName), new QTableWidgetItem(columnsLabel[lKeyName]));
        }
        else
        {
            GSLOG(SYSLOG_SEV_ERROR, "Invalid group");
        }
    }
}

void DrillParametricTable::InitShiftHeaderSize(const QString &columnName)
{
    int lMaxGroupToCheck = 1;
    if ((ReportOptions.GetOption("statistics","shift_with")).toString() == "shift_to_all")
    {
        lMaxGroupToCheck = gexReport->getGroupsList().size();
    }
    for(int lIdx = 0; lIdx < lMaxGroupToCheck; ++lIdx)
    {
        if (gexReport->getGroupsList()[lIdx])
        {
            QString lKeyName = columnName + gexReport->getGroupsList()[lIdx]->strGroupName;
            setColumnWidth(ColPosition(lKeyName),60);	// Mean shift: column width
        }
        else
        {
            GSLOG(SYSLOG_SEV_ERROR, "Invalid group");
        }
    }
}

void DrillParametricTable::ResetTable()
{
    QHeaderView	*pTableHeader=0;
    DrillTableItem *pCell=0;
    CTest *ptTestCell=0;
    char szString[2048]="";
    QString	strString;
    double lfData=0;
    int iRow = 0;
    //int iCol = 0;
    QString strOptionStorageDevice = (ReportOptions.GetOption("statistics","generic_galaxy_tests")).toString();
    QString strBoxplotShiftOver	= ReportOptions.GetOption("adv_boxplot","delta").toString();
    bool lCompareAllGroups = (ReportOptions.GetOption("statistics","shift_with")).toString() == "shift_to_all";
    bool bBoxplotShiftOverTV = (strBoxplotShiftOver == "over_tv");
    QString strFormatDouble;

    InitShiftColumnLabels();

    setSortingEnabled(false);
    // Quiet return if no data & report available!
    if(gexReport == NULL)
        return;

    // Disbaled paint when filling the gui
    setUpdatesEnabled(false);

    // Get pointer to horizontal header: show that Test# are sorted in ascending order (default)
    pTableHeader = horizontalHeader();
    pTableHeader->setSortIndicator(0,Qt::AscendingOrder);
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Delete content of table if any...
    while (rowCount() > 0)
    {
        removeRow(0);
    }

    // Test #, name, LowL, HighL, Cp, Cpk, Yield
    // Get pointer to first group & first file (we always have them exist)
    int lNbreOfGroups = gexReport->getGroupsList().size();
    QListIterator<CGexGroupOfFiles*>	itGroupsList(gexReport->getGroupsList());
    CGexGroupOfFiles*					pGroup = NULL;
    QList<CGexGroupOfFiles*> lGroupList(gexReport->getGroupsList());

    // Set number of rows in table counter
    int maxRow = 0;



    while (itGroupsList.hasNext())
    {
        pGroup     = itGroupsList.next();
        ptTestCell = pGroup->cMergedData.ptMergedTestList;
        iRow = 0;
        while (ptTestCell != NULL)
        {
            // IF Muti-result parametric test, do not show master test record
            if (ptTestCell->lResultArraySize > 0)
            {
                goto NextCell;
            }

            // case 1871 - HTH - Allow functional tests to be shown in the
            // parametric tab except when "Hide Functional Tests" check box is
            // checked
            if (mHideFunctionalTests && ptTestCell->bTestType == 'F')
            {
                goto NextCell;
            }

            // Ignore Generic Galaxy Parameters
            if ((ptTestCell->bTestType == '-') &&
                (strOptionStorageDevice == "hide"))
            {
                goto NextCell;
            }

            if (gexReport->isInteractiveTestFiltered(ptTestCell) == false)
            {
                goto NextCell;
            }

            // this is a valid test, count it!
            iRow++;

            // Point to next test cell
        NextCell:
            ptTestCell = ptTestCell->GetNextTest();
        }
        if (maxRow < iRow)
        {
            maxRow = iRow;
        }
    }

    int nbreRow = maxRow * gexReport->getGroupsList().count();
    setRowCount(nbreRow);
    InitColumnPositions();

    setHorizontalHeaderItem(ColPosition(TEST_TNUMBER), new QTableWidgetItem(columnsLabel[TEST_TNUMBER]));
    setHorizontalHeaderItem(ColPosition(TEST_TNAME), new QTableWidgetItem(columnsLabel[TEST_TNAME]));
    setHorizontalHeaderItem(ColPosition(TEST_GROUP), new QTableWidgetItem(columnsLabel[TEST_GROUP]));
    setHorizontalHeaderItem(ColPosition(TEST_TYPE), new QTableWidgetItem(columnsLabel[TEST_TYPE]));
    setHorizontalHeaderItem(ColPosition(TEST_LTL), new QTableWidgetItem(columnsLabel[TEST_LTL]));
    setHorizontalHeaderItem(ColPosition(TEST_HTL), new QTableWidgetItem(columnsLabel[TEST_HTL]));
    setHorizontalHeaderItem(ColPosition(TEST_LSL), new QTableWidgetItem(columnsLabel[TEST_LSL]));
    setHorizontalHeaderItem(ColPosition(TEST_HSL), new QTableWidgetItem(columnsLabel[TEST_HSL]));
    setHorizontalHeaderItem(ColPosition(TEST_DRIFTL), new QTableWidgetItem(columnsLabel[TEST_DRIFTL]));
    setHorizontalHeaderItem(ColPosition(TEST_DRIFTLOW), new QTableWidgetItem(columnsLabel[TEST_DRIFTLOW]));
    setHorizontalHeaderItem(ColPosition(TEST_DRIFTHIGH), new QTableWidgetItem(columnsLabel[TEST_DRIFTHIGH]));
    setHorizontalHeaderItem(ColPosition(TEST_SHAPE), new QTableWidgetItem(columnsLabel[TEST_SHAPE]));	// Advanced statistic.
    setHorizontalHeaderItem(ColPosition(TEST_STATS_SRC), new QTableWidgetItem(columnsLabel[TEST_STATS_SRC]));
    setHorizontalHeaderItem(ColPosition(TEST_EXEC), new QTableWidgetItem(columnsLabel[TEST_EXEC]));
    setHorizontalHeaderItem(ColPosition(TEST_FAIL), new QTableWidgetItem(columnsLabel[TEST_FAIL]));
    setHorizontalHeaderItem(ColPosition(TEST_FAILPERCENT), new QTableWidgetItem(columnsLabel[TEST_FAILPERCENT]));
    setHorizontalHeaderItem(ColPosition(TEST_FAILBIN), new QTableWidgetItem(columnsLabel[TEST_FAILBIN]));
    setHorizontalHeaderItem(ColPosition(TEST_TESTFLOWID), new QTableWidgetItem(columnsLabel[TEST_TESTFLOWID]));
    setHorizontalHeaderItem(ColPosition(TEST_OUTLIER),  new QTableWidgetItem(columnsLabel[TEST_OUTLIER]));
    setHorizontalHeaderItem(ColPosition(TEST_MEAN),  new QTableWidgetItem(columnsLabel[TEST_MEAN]));
    InitShiftHorizontalHeader(TEST_MEANSHIFT);
    setHorizontalHeaderItem(ColPosition(TEST_T_TEST), new QTableWidgetItem(columnsLabel[TEST_T_TEST]));
    setHorizontalHeaderItem(ColPosition(TEST_SIGMA), new QTableWidgetItem(columnsLabel[TEST_SIGMA]));
    InitShiftHorizontalHeader(TEST_SIGMASHIFT);
    setHorizontalHeaderItem(ColPosition(TEST_2_SIGMA), new QTableWidgetItem(columnsLabel[TEST_2_SIGMA]));
    setHorizontalHeaderItem(ColPosition(TEST_3_SIGMA), new QTableWidgetItem(columnsLabel[TEST_3_SIGMA]));
    setHorizontalHeaderItem(ColPosition(TEST_6_SIGMA), new QTableWidgetItem(columnsLabel[TEST_6_SIGMA]));
    setHorizontalHeaderItem(ColPosition(TEST_MIN), new QTableWidgetItem(columnsLabel[TEST_MIN]));
    setHorizontalHeaderItem(ColPosition(TEST_MAX), new QTableWidgetItem(columnsLabel[TEST_MAX]));
    setHorizontalHeaderItem(ColPosition(TEST_RANGE), new QTableWidgetItem(columnsLabel[TEST_RANGE]));
    setHorizontalHeaderItem(ColPosition(TEST_MAX_RANGE), new QTableWidgetItem(columnsLabel[TEST_MAX_RANGE]));
    setHorizontalHeaderItem(ColPosition(TEST_CP), new QTableWidgetItem(columnsLabel[TEST_CP]));
    setHorizontalHeaderItem(ColPosition(TEST_CR), new QTableWidgetItem(columnsLabel[TEST_CR])); // GCORE-199
    InitShiftHorizontalHeader(TEST_CPSHIFT);
    InitShiftHorizontalHeader(TEST_CRSHIFT);
    setHorizontalHeaderItem(ColPosition(TEST_CPK), new QTableWidgetItem(columnsLabel[TEST_CPK]));
    setHorizontalHeaderItem(ColPosition(TEST_CPKL), new QTableWidgetItem(columnsLabel[TEST_CPKL]));
    setHorizontalHeaderItem(ColPosition(TEST_CPKH), new QTableWidgetItem(columnsLabel[TEST_CPKH]));
    InitShiftHorizontalHeader(TEST_CPKSHIFT);
    setHorizontalHeaderItem(ColPosition(TEST_YIELD), new QTableWidgetItem(columnsLabel[TEST_YIELD]));
    setHorizontalHeaderItem(ColPosition(TEST_GAGE_EV), new QTableWidgetItem(columnsLabel[TEST_GAGE_EV]));
    setHorizontalHeaderItem(ColPosition(TEST_GAGE_AV), new QTableWidgetItem(columnsLabel[TEST_GAGE_AV]));
    setHorizontalHeaderItem(ColPosition(TEST_GAGE_RR), new QTableWidgetItem(columnsLabel[TEST_GAGE_RR]));
    setHorizontalHeaderItem(ColPosition(TEST_GAGE_GB), new QTableWidgetItem(columnsLabel[TEST_GAGE_GB]));
    setHorizontalHeaderItem(ColPosition(TEST_GAGE_PV), new QTableWidgetItem(columnsLabel[TEST_GAGE_PV]));
    setHorizontalHeaderItem(ColPosition(TEST_GAGE_TV), new QTableWidgetItem(columnsLabel[TEST_GAGE_TV]));
    setHorizontalHeaderItem(ColPosition(TEST_GAGE_P_T), new QTableWidgetItem(columnsLabel[TEST_GAGE_P_T]));
    // Advanced Statistics
    setHorizontalHeaderItem(ColPosition(TEST_SKEW), new QTableWidgetItem(columnsLabel[TEST_SKEW]));
    setHorizontalHeaderItem(ColPosition(TEST_KURTOSIS), new QTableWidgetItem(columnsLabel[TEST_KURTOSIS]));
    setHorizontalHeaderItem(ColPosition(TEST_P0_5), new QTableWidgetItem(columnsLabel[TEST_P0_5]));
    setHorizontalHeaderItem(ColPosition(TEST_P2_5), new QTableWidgetItem(columnsLabel[TEST_P2_5]));
    setHorizontalHeaderItem(ColPosition(TEST_P10), new QTableWidgetItem(columnsLabel[TEST_P10]));
    setHorizontalHeaderItem(ColPosition(TEST_Q1), new QTableWidgetItem(columnsLabel[TEST_Q1]));
    setHorizontalHeaderItem(ColPosition(TEST_Q2), new QTableWidgetItem(columnsLabel[TEST_Q2]));
    setHorizontalHeaderItem(ColPosition(TEST_Q3), new QTableWidgetItem(columnsLabel[TEST_Q3]));
    setHorizontalHeaderItem(ColPosition(TEST_P90), new QTableWidgetItem(columnsLabel[TEST_P90]));
    setHorizontalHeaderItem(ColPosition(TEST_P97_5), new QTableWidgetItem(columnsLabel[TEST_P97_5]));
    setHorizontalHeaderItem(ColPosition(TEST_P99_5), new QTableWidgetItem(columnsLabel[TEST_P99_5]));
    setHorizontalHeaderItem(ColPosition(TEST_IQR), new QTableWidgetItem(columnsLabel[TEST_IQR]));
    setHorizontalHeaderItem(ColPosition(TEST_SIGMAIQR), new QTableWidgetItem(columnsLabel[TEST_SIGMAIQR]));
    // multi limit
  /*  for (int lIndex = 0; lIndex < gexReport->GetMaxMultiLimitItems(); ++lIndex)
    {
        setHorizontalHeaderItem(ColPosition(QString("ll-%1").arg(lIndex + 1)),
                                new QTableWidgetItem(QString("LL-%1").arg(lIndex + 1)));
        setHorizontalHeaderItem(ColPosition(QString("hl-%1").arg(lIndex + 1)),
                                new QTableWidgetItem(QString("HL-%1").arg(lIndex + 1)));
    }*/


#ifdef _WIN32
    // Resize columns so most of columns fit in the window...
    setColumnWidth(ColPosition(TEST_TNUMBER),60);	// Test# : column width
    setColumnWidth(ColPosition(TEST_TNAME),140);		// Testname : column width
    setColumnWidth(ColPosition(TEST_GROUP),100);		// Group/Dataset : column width
    setColumnWidth(ColPosition(TEST_TYPE),40);		// Test type : column width
    setColumnWidth(ColPosition(TEST_LTL),80);		// LTL : column width
    setColumnWidth(ColPosition(TEST_HTL),80);		// HTL : column width
    setColumnWidth(ColPosition(TEST_LSL),80);		// LSL : column width
    setColumnWidth(ColPosition(TEST_HSL),80);		// HSL : column width
    setColumnWidth(ColPosition(TEST_DRIFTL),100);		// Test/Spec % drift
    setColumnWidth(ColPosition(TEST_DRIFTLOW),100);		// Low Test/Spec % drift
    setColumnWidth(ColPosition(TEST_DRIFTHIGH),100);		// High Test/Spec % drift
    setColumnWidth(ColPosition(TEST_SHAPE),50);		// Distribution type/shape column width
    setColumnWidth(ColPosition(TEST_STATS_SRC),50);		// Siurce: Samples, Summary,...
    setColumnWidth(ColPosition(TEST_TESTFLOWID),50);	// test flow ID, column width
    setColumnWidth(ColPosition(TEST_EXEC),50);		// Exec. count : column width
    setColumnWidth(ColPosition(TEST_FAIL),50);		// Fail. count : column width
    setColumnWidth(ColPosition(TEST_FAILPERCENT),50);	// Fail %: column width
    setColumnWidth(ColPosition(TEST_FAILBIN),50);	// Fail Bin: column width
    setColumnWidth(ColPosition(TEST_OUTLIER),50);	// Outliers: column width
    setColumnWidth(ColPosition(TEST_MEAN),80);		// Mean: column width
    InitShiftHeaderSize(TEST_MEANSHIFT);	// Mean shift: column width
    setColumnWidth(ColPosition(TEST_T_TEST),60);		// Student's T-Test: column width
    setColumnWidth(ColPosition(TEST_SIGMA),80);		// Sigma: column width
    InitShiftHeaderSize(TEST_SIGMASHIFT);	// Sigma shift: column width
    setColumnWidth(ColPosition(TEST_2_SIGMA),80);	// 2xsigma: column width
    setColumnWidth(ColPosition(TEST_3_SIGMA),80);	// 3xsigma: column width
    setColumnWidth(ColPosition(TEST_6_SIGMA),80);	// 6xsigma: column width
    setColumnWidth(ColPosition(TEST_MIN),80);		// Min: column width
    setColumnWidth(ColPosition(TEST_MAX),80);		// Max: column width
    setColumnWidth(ColPosition(TEST_RANGE),80);		// Range : column width
    setColumnWidth(ColPosition(TEST_MAX_RANGE),80);	// Max. Range : column width
    setColumnWidth(ColPosition(TEST_CP),50);			// Cp : column width
    setColumnWidth(ColPosition(TEST_CR),50);			// Cr : GCORE-199
    InitShiftHeaderSize(TEST_CPSHIFT);	// Cp shift : column width
    setColumnWidth(ColPosition(TEST_CPK),50);		// Cpk: column width
    setColumnWidth(ColPosition(TEST_CPKL),50);		// CpkL: column width
    setColumnWidth(ColPosition(TEST_CPKH),50);		// CpkH: column width
    InitShiftHeaderSize(TEST_CPKSHIFT);	// Cpk shift: column width
    setColumnWidth(ColPosition(TEST_YIELD),50);		// Yield : column width
    setColumnWidth(ColPosition(TEST_GAGE_EV),100);	// Gage EV : column width
    setColumnWidth(ColPosition(TEST_GAGE_AV),100);	// Gage AV : column width
    setColumnWidth(ColPosition(TEST_GAGE_RR),100);	// Gage R&R : column width
    setColumnWidth(ColPosition(TEST_GAGE_GB),100);	// Gage GB: column width
    setColumnWidth(ColPosition(TEST_GAGE_PV),100);	// Gage EP : column width
    setColumnWidth(ColPosition(TEST_GAGE_TV),100);	// Gage TV : column width
    setColumnWidth(ColPosition(TEST_GAGE_P_T),100);	// Gage P/T : column width
    // Advanced sttistics
    setColumnWidth(ColPosition(TEST_SKEW),60);		// Skew : column width
    setColumnWidth(ColPosition(TEST_KURTOSIS),60);	// Kurtosis : column width
    setColumnWidth(ColPosition(TEST_P0_5),80);		// Percentile 0.5% : column width
    setColumnWidth(ColPosition(TEST_P2_5),80);		// Percentile 2.5% : column width
    setColumnWidth(ColPosition(TEST_P10),80);		// Percentile 10% : column width
    setColumnWidth(ColPosition(TEST_Q1),80);			// Quartile1 : column width
    setColumnWidth(ColPosition(TEST_Q2),80);			// Median : column width
    setColumnWidth(ColPosition(TEST_Q3),80);			// Quartile3 : column width
    setColumnWidth(ColPosition(TEST_P90),80);		// Percentile 90% : column width
    setColumnWidth(ColPosition(TEST_P97_5),80);		// Percentile 97.5% : column width
    setColumnWidth(ColPosition(TEST_P99_5),80);		// Percentile 99.5% : column width
    setColumnWidth(ColPosition(TEST_IQR),80);		// IQR : column width
    setColumnWidth(ColPosition(TEST_SIGMAIQR),80);	// IQR SD : column width
#endif

    // List of tests ...for all groups
    CGexFileInGroup *pFile=0;

    pGroup = gexReport->getGroupsList().isEmpty()?NULL:gexReport->getGroupsList().first();
    ptTestCell = pGroup->cMergedData.ptMergedTestList;
    iRow=0;
    CTest	*lOtherTestCell=0;
    QString lBackgroundColor;
    QString lCrString(GEX_NA); // GCORE-199
    QString lCpString(GEX_NA);
    char	lTestName[1024]="";
    char	lTempString[GEX_LIMIT_LABEL]="";
    int		lGroupNum=0;
    //int     lLimitItemIdx = 0;
    long	lPinmapIndex=0;
    double	lData=0;
    double	lfLimitSpace=0;
    double  lfPercent=0;
    QString lOutputString;
    GSLOG(6, QString("Looping over %2 tests...").arg(pGroup->cMergedData.GetNumberOfTests()).toLatin1().data());
    CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();

    //-- Load options, one and for all

    //- alarm_mean
    double  lfOptionAlarmMean   =   0.0;
    bool    bGetOptionMean      =   false;
    lfOptionAlarmMean = (ReportOptions.GetOption("statistics","alarm_mean")).toDouble(&bGetOptionMean);

    //- alarm_sigma
    double  lfOptionAlarmSigma  =   0.0;
    bool    bGetOptionRsltSigma =   0.0;
    lfOptionAlarmSigma = (ReportOptions.GetOption("statistics","alarm_sigma")).toDouble(&bGetOptionRsltSigma);

    //- alarm_test_cp
    bool bRedAlarmCpValidity = false, bYellowAlarmCpValidity = false;
    double lfRedAlarmCpValue = 0.0, lfYellowAlarmCpValue= 0.0;
    lfRedAlarmCpValue = ReportOptions.GetOption("statistics", "alarm_test_cp").toDouble(&bRedAlarmCpValidity);
    lfYellowAlarmCpValue = ReportOptions.GetOption("statistics", "warning_test_cp").toDouble(&bYellowAlarmCpValidity);
    GEX_ASSERT( bRedAlarmCpValidity && bYellowAlarmCpValidity);			// check conversion

    //- alarm_test_cpk
    bool bRedAlarmCpkValidity = false, bYellowAlarmCpkValidity = false;
    double lfRedAlarmCpkValue = 0.0, lfYellowAlarmCpkValue= 0.0;
    lfRedAlarmCpkValue = ReportOptions.GetOption("statistics", "alarm_test_cpk").toDouble(&bRedAlarmCpkValidity);
    lfYellowAlarmCpkValue = ReportOptions.GetOption("statistics", "warning_test_cpk").toDouble(&bYellowAlarmCpkValidity);
    GEX_ASSERT( bRedAlarmCpkValidity && bYellowAlarmCpkValidity);			// check conversion

    //- alarm_cp_cr_cpk shifts
    double  lfOptionAlarmCp    =   0.0;
    bool    bGetOptionRsltCp   =   false;
    lfOptionAlarmCp = (ReportOptions.GetOption("statistics","alarm_cp")).toDouble(&bGetOptionRsltCp);
    GEX_ASSERT(bGetOptionRsltCp);
    double  lfOptionAlarmCr    =   0.0;
    bool    bGetOptionRsltCr   =   false;
    lfOptionAlarmCr = (ReportOptions.GetOption("statistics","alarm_cr")).toDouble(&bGetOptionRsltCr);
    GEX_ASSERT(bGetOptionRsltCr);
    double  lfOptionAlarmCpk    =   0.0;
    bool    bGetOptionRsltCpk   =   false;
    lfOptionAlarmCpk = (ReportOptions.GetOption("statistics","alarm_cpk")).toDouble(&bGetOptionRsltCpk);
    GEX_ASSERT(bGetOptionRsltCpk);

    //- alarm_test_yield
    double lfYieldAlarmOptionStorageDevice = 0.0, lfYieldWarningoptionStorageDevice = 0.0;
    bool bYieldAlarmGetOptionRslt = false, bYieldWarningGetOptionRslt = false;

    lfYieldAlarmOptionStorageDevice = (ReportOptions.GetOption("statistics","alarm_test_yield"))
            .toDouble(&bYieldAlarmGetOptionRslt);
    lfYieldWarningoptionStorageDevice = (ReportOptions.GetOption("statistics","warning_test_yield"))
            .toDouble(&bYieldWarningGetOptionRslt);
    GEX_ASSERT(bYieldAlarmGetOptionRslt && bYieldWarningGetOptionRslt);

    int lMax = pGroup->cMergedData.GetNumberOfTests();
    GS::Gex::Engine::GetInstance().UpdateProgressStatus(true, 100, 0);
    //QCoreApplication::processEvents();
    int lStep = 0, lCounter = 0;

    bool alarmRaised = false;
    int lMaxGroupToCheck = 1;
    if (lCompareAllGroups)
    {
        lMaxGroupToCheck = lGroupList.size();
    }

    while(ptTestCell != NULL)
    {
        GSLOG(7, QString("Considering test %1...").arg(ptTestCell->lTestNumber).toLatin1().data() );

        // IF Muti-result parametric test, do not show master test record
        if(ptTestCell->lResultArraySize > 0)
            goto NextTestCell;

        // case 1871 - HTH - Allow functional tests to be shown in the parametric tab except
        // when "Hide Functional Tests" check box is checked
        if(mHideFunctionalTests && ptTestCell->bTestType == 'F')
            goto NextTestCell;

        // Ignore Generic Galaxy Parameters
        {
            if((ptTestCell->bTestType == '-') && (strOptionStorageDevice == "hide") )
                goto NextTestCell;
        }

        if (gexReport->isInteractiveTestFiltered(ptTestCell) == false)
            goto NextTestCell;

        // First: write info about reference group#1
        itGroupsList.toFront();
        pGroup = NULL;
        lGroupNum = 0;

        GSLOG(6, QString("Looping over %1 group(s)...").arg(gexReport->getGroupsList().size()).toLatin1().data() );

        // Create one stats line per test of each group...
        while(itGroupsList.hasNext())
        {
            // List of tests in group#2 or group#3, etc...
            pGroup = itGroupsList.next();
            pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

            if(ptTestCell->lPinmapIndex >= 0)
              lPinmapIndex = ptTestCell->lPinmapIndex;
            else
              lPinmapIndex = ptTestCell->lPinmapIndex; //GEX_PTEST;

            // Find pointer to Test Cell in group#x.
            if(pFile->FindTestCell(ptTestCell->lTestNumber,lPinmapIndex,&lOtherTestCell,false,false,
                                   ptTestCell->strTestName.toLatin1().data()) !=1)
            {
                // Point to next group
                lGroupNum++;
                continue;
            }

            // Insert test info into the table...
            // Test #
            if (lOtherTestCell->szTestLabel[0] == '\0')
                gexReport->BuildTestNumberString(lOtherTestCell);
            pCell = new DrillTableItem(QTableWidgetItem::Type, lOtherTestCell->szTestLabel, iRow);
            setItem( iRow, ColPosition(TEST_TNUMBER), pCell );

            // Test name
            gexReport->BuildTestNameString(pFile,ptTestCell,lTestName);
            pCell = new DrillTableItem(QTableWidgetItem::Type, lTestName, iRow);
            setItem( iRow, ColPosition(TEST_TNAME), pCell );

            // Group name / Dataset
            /*( this, QTableWidgetItem::Type, pGroup->strGroupName.toLatin1().constData());*/
            pCell = new DrillTableItem(QTableWidgetItem::Type, pGroup->strGroupName.toLatin1().constData(), iRow);
            setItem( iRow, ColPosition(TEST_GROUP), pCell );

            // Test type.
            gexReport->BuildTestTypeString(pFile,lOtherTestCell,szString,true);
            /*( this, QTableWidgetItem::Type, szString, iRow);*/
            pCell = new DrillTableItem(QTableWidgetItem::Type, szString, iRow);
            setItem( iRow, ColPosition(TEST_TYPE), pCell );

            // LL
            if((lOtherTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
                pFile->FormatTestLimit(lOtherTestCell, lTempString, lOtherTestCell->GetCurrentLimitItem()->lfLowLimit,
                                       lOtherTestCell->llm_scal, false);
            else
                strcpy(lTempString, GEX_NA);
            pCell = new DrillTableItem(QTableWidgetItem::Type, lTempString, iRow);
            setItem( iRow, ColPosition(TEST_LTL), pCell );
            checkItemAndAddUnit(pCell);

            // HL
            if((lOtherTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
                pFile->FormatTestLimit(
                            ptTestCell, lTempString, lOtherTestCell->GetCurrentLimitItem()->lfHighLimit, lOtherTestCell->hlm_scal, false);
            else
                strcpy(lTempString, GEX_NA);
            pCell = new DrillTableItem( QTableWidgetItem::Type, lTempString, iRow);
            setItem( iRow, ColPosition(TEST_HTL), pCell );
            checkItemAndAddUnit(pCell);

            // L Spec. L
            if((lOtherTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLSL) == 0)
                pFile->FormatTestLimit(
                       ptTestCell, lTempString, lOtherTestCell->lfLowSpecLimit, lOtherTestCell->llm_scal, false);
            else
                strcpy(lTempString, GEX_NA);
            pCell = new DrillTableItem( QTableWidgetItem::Type, lTempString, iRow);
            setItem( iRow, ColPosition(TEST_LSL), pCell );
            checkItemAndAddUnit(pCell);

            // H Spec. L
            if((lOtherTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHSL) == 0)
                pFile->FormatTestLimit(
                       ptTestCell, lTempString, lOtherTestCell->lfHighSpecLimit, lOtherTestCell->hlm_scal, false);
            else
                strcpy(lTempString, GEX_NA);
            pCell = new DrillTableItem( QTableWidgetItem::Type, lTempString, iRow);
            setItem( iRow, ColPosition(TEST_HSL), pCell );
            checkItemAndAddUnit(pCell);

            // Test/Spec limits drift: both Test & spec limits must exist
            pCell = new DrillTableItem( QTableWidgetItem::Type, gexReport->CreateTestDrilTL(lOtherTestCell), iRow);
            setItem( iRow, ColPosition(TEST_DRIFTL), pCell );

            // LOW Test/Spec limits drift %
            pCell = new DrillTableItem( QTableWidgetItem::Type, gexReport->CreateTestDrilFlow(lOtherTestCell), iRow);
            setItem( iRow, ColPosition(TEST_DRIFTLOW), pCell );

            // HIGH Test/Spec limits drift %
            pCell = new DrillTableItem( QTableWidgetItem::Type, gexReport->CreateTestDriftHigh(lOtherTestCell), iRow);
            setItem( iRow, ColPosition(TEST_DRIFTHIGH), pCell );

            // Multi Limits
           /* lLimitItemIdx = 0;
            for (lLimitItemIdx = 0; lLimitItemIdx < lOtherTestCell->MultiLimitItemCount(); ++lLimitItemIdx)
            {
                strcpy(lTempString, GEX_NA);
                GS::Core::MultiLimitItem* lMultiLimitItem = lOtherTestCell->GetMultiLimitItem(lLimitItemIdx);
                if (!lMultiLimitItem)
                {
                    continue;
                }
                if (lMultiLimitItem->IsValidLowLimit())
                {
                    pFile->FormatTestLimit(lOtherTestCell,
                                           lTempString,
                                           lMultiLimitItem->GetLowLimit(),
                                           lOtherTestCell->llm_scal);
                }

                int lColumnPosition = ColPosition(QString("ll"));
                if(lColumnPosition != -1)
                {

                    pCell = new DrillTableItem( QTableWidgetItem::Type, lTempString, iRow);
                    setItem( iRow, lColumnPosition, pCell );
                }

                strcpy(lTempString, GEX_NA);
                if (lMultiLimitItem->IsValidHighLimit())
                {
                    pFile->FormatTestLimit(lOtherTestCell,
                                           lTempString,
                                           lMultiLimitItem->GetHighLimit(),
                                           lOtherTestCell->hlm_scal);
                }

                lColumnPosition = ColPosition(QString("hl"));
                if(lColumnPosition != -1)
                {
                    pCell = new DrillTableItem( QTableWidgetItem::Type, lTempString, iRow);
                    setItem( iRow, lColumnPosition, pCell );
                }
            }*/

            // then complete with empty columns if needed
          /*  for (; lLimitItemIdx < gexReport->GetMaxMultiLimitItems(); ++lLimitItemIdx)
            {
                strcpy(lTempString, GEX_NA);
                pCell = new DrillTableItem( QTableWidgetItem::Type, lTempString, iRow);
                setItem( iRow, ColPosition(QString("ll-%1").arg(lLimitItemIdx + 1)), pCell );
                pCell = new DrillTableItem( QTableWidgetItem::Type, lTempString, iRow);
                setItem( iRow, ColPosition(QString("hl-%1").arg(lLimitItemIdx + 1)), pCell );
            }*/

            // Distribution shape
            if (lPatInfo)
            {
                strString = patlib_GetDistributionName(patlib_GetDistributionType(lOtherTestCell,
                                                       lPatInfo->GetRecipeOptions().iCategoryValueCount,
                                                       lPatInfo->GetRecipeOptions().bAssumeIntegerCategory,
                                                       lPatInfo->GetRecipeOptions().mMinConfThreshold));
            }
            else
                strString = patlib_GetDistributionName(patlib_GetDistributionType(lOtherTestCell));

            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
            setItem( iRow, ColPosition(TEST_SHAPE), pCell );

            // Stats source: Samples, summary,...
            strString = (lOtherTestCell->bStatsFromSamples) ? "Samples": "Summary";
            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
            setItem( iRow, ColPosition(TEST_STATS_SRC), pCell );

            // Exec. count
            pCell = new DrillTableItem( QTableWidgetItem::Type,
                                        gexReport->CreateResultString(lOtherTestCell->ldExecs), iRow);
            setItem( iRow, ColPosition(TEST_EXEC), pCell );

            // Fail count
            pCell = new DrillTableItem( QTableWidgetItem::Type,
                                        gexReport->CreateResultString(lOtherTestCell->GetCurrentLimitItem()->ldFailCount), iRow);
            setItem( iRow, ColPosition(TEST_FAIL), pCell );

            // Fail %
            gexReport->CreateTestFailPercentage(lOtherTestCell, lOutputString);
            pCell = new DrillTableItem( QTableWidgetItem::Type, lOutputString, iRow);
            setItem( iRow, ColPosition(TEST_FAILPERCENT), pCell );

            // Fail Bin#
            if(lOtherTestCell->iFailBin >= 0){
                sprintf(szString,"%d",lOtherTestCell->iFailBin);
            }
            else
                strcpy(szString,"0");
            pCell = new DrillTableItem( QTableWidgetItem::Type, szString, iRow);
            setItem( iRow, ColPosition(TEST_FAILBIN), pCell );

            // Test Flow ID (order in testing flow)
            // -- replace -1 by n/a . in order to have the galaxy test (-1) in the end of the list when ascending order
            if(lOtherTestCell->lTestFlowID <0)
                strcpy(szString, "n/a .");
            else
                sprintf(szString,"%d",lOtherTestCell->lTestFlowID);

            pCell = new DrillTableItem( QTableWidgetItem::Type, szString, iRow);
            setItem( iRow, ColPosition(TEST_TESTFLOWID), pCell );

            // Outlier count
            pCell = new DrillTableItem( QTableWidgetItem::Type,
                                        gexReport->CreateResultString(lOtherTestCell->GetCurrentLimitItem()->ldOutliers), iRow);
            setItem( iRow, ColPosition(TEST_OUTLIER), pCell );

            // Mean
            lfData = lOtherTestCell->lfMean;
            if(fabs(lfData)>= C_INFINITE || lOtherTestCell->bTestType == 'F')
                strString = GEX_NA;	// MIN value not available.
            else{
                strString = pFile->FormatTestResult(lOtherTestCell,lfData,lOtherTestCell->res_scal, false);
            }
            pCell = new DrillTableItem( QTableWidgetItem::Type,strString, iRow);
            setItem( iRow, ColPosition(TEST_MEAN), pCell );
            checkItemAndAddUnit(pCell);

            if(fabs(lfData) < C_INFINITE)
            {
                // Highlight in red if mean outside of limits!
                if( (((lOtherTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
                     && (lOtherTestCell->lfMean < lOtherTestCell->GetCurrentLimitItem()->lfLowLimit)) ||
                    (((lOtherTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
                     && (lOtherTestCell->lfMean > lOtherTestCell->GetCurrentLimitItem()->lfHighLimit)) )
                    pCell->mFailing = GEX_DRILL_CELLTYPE_ALARM;
            }

            // Mean Shift
            if (lNbreOfGroups > 1)
            {
                if(lOtherTestCell->bTestType == 'F')
                {
                    if (lGroupList[0])
                    {
                        int lCol = ColPosition(TEST_MEANSHIFT+ lGroupList[0]->strGroupName);
                        if (lCol)
                        {
                            strString = GEX_NA;
                            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
                            setItem( iRow, lCol, pCell );
                        }
                    }
                }
                else
                {
                    // print NA for all previous groups, for the rest of groups, print the shift
                    for (int lGroupIdx = 0; lGroupIdx < lMaxGroupToCheck; ++lGroupIdx)
                    {
                        if (!lGroupList[lGroupIdx])
                        {
                            GSLOG(SYSLOG_SEV_ERROR, "Invalid group");
                        }
                        else
                        {
                            GS::Core::MLShift lMLShifts;
                            GetmultiLimitShift(lOtherTestCell, lGroupList[lGroupIdx]->strGroupName, lMLShifts);
                            TestShift lTestShift = lOtherTestCell->mTestShifts.value(lGroupList[lGroupIdx]->strGroupName);
                            alarmRaised = false;
                            if (lTestShift.IsValid() == false)
                            {
                                strString.sprintf( "%s", GEX_NA );
                            }
                            else
                            {

                                strString.sprintf("%.2f", lMLShifts.mMeanShiftPct);
                                strFormatDouble = gexReport->getNumberFormat()->formatNumericValue(lMLShifts.mMeanShiftPct,
                                                                                                   true, QString(strString));
                                strString.sprintf("%s %%",strFormatDouble.toLatin1().constData());
                                alarmRaised = (fabs(lMLShifts.mMeanShiftPct) >= lfOptionAlarmMean) && (bGetOptionMean == true);
                            }
                            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
                            setItem(iRow, ColPosition(TEST_MEANSHIFT + lGroupList[lGroupIdx]->strGroupName), pCell);

                            // Color the cell if needed
                            if(alarmRaised)
                            {
                                pCell->mFailing = GEX_DRILL_CELLTYPE_ALARM;
                                pCell->paint(iRow);
                            }
                        }
                    }
                }
            }
            // T-Test
            sprintf(szString,"%s ",gexReport->getNumberFormat()->formatNumericValue(
                        lOtherTestCell->getP_Value(), false).toLatin1().constData());
            pCell = new DrillTableItem( QTableWidgetItem::Type,szString, iRow);
            setItem( iRow, ColPosition(TEST_T_TEST), pCell );
            checkItemAndAddUnit(pCell);

            // Sigma
            // No sigma for functional tests
            if(lOtherTestCell->bTestType == 'F')
                strString = GEX_NA;
            else
            {
                lfData = lOtherTestCell->lfSigma;
                strString = pFile->FormatTestResult(lOtherTestCell,lfData,lOtherTestCell->res_scal, false);
            }
            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
            setItem( iRow, ColPosition(TEST_SIGMA), pCell );
            checkItemAndAddUnit(pCell);

            // Sigma Shift
            // No sigma for functional tests
            if (lNbreOfGroups > 1)
            {
                if(lOtherTestCell->bTestType == 'F')
                {
                    if (lGroupList[0])
                    {
                        int lCol = ColPosition(TEST_SIGMASHIFT+ lGroupList[0]->strGroupName);
                        if (lCol)
                        {
                            strString = GEX_NA;
                            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
                            setItem( iRow, lCol, pCell );
                        }
                    }
                }
                else
                {
                    // print NA for all previous groups, for the rest of groups, print the shift
                    for (int lGroupIdx=0; lGroupIdx < lMaxGroupToCheck; ++lGroupIdx)
                    {
                        if (!lGroupList[lGroupIdx])
                        {
                            GSLOG(SYSLOG_SEV_ERROR, "Invalid group");
                        }
                        else
                        {
                            TestShift lTestShift = lOtherTestCell->mTestShifts.value(lGroupList[lGroupIdx]->strGroupName);
                            alarmRaised = false;
                            if (lTestShift.IsValid() == false)
                            {
                                strString.sprintf( "%s", GEX_NA );
                            }
                            else
                            {
                                strString.sprintf("%.2f", lTestShift.mSigmaShiftPercent);
                                strFormatDouble = gexReport->getNumberFormat()->formatNumericValue(lTestShift.mSigmaShiftPercent,
                                                                                                   true, QString(strString));
                                strString.sprintf("%s %%",strFormatDouble.toLatin1().constData());
                                alarmRaised = (fabs(lTestShift.mSigmaShiftPercent) >= lfOptionAlarmSigma) && (bGetOptionRsltSigma == true);
                            }
                            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
                            setItem(iRow, ColPosition(TEST_SIGMASHIFT + lTestShift.mRefGroup), pCell);
                            // Color the cell if needed
                            if(alarmRaised)
                            {
                                pCell->mFailing = GEX_DRILL_CELLTYPE_ALARM;
                                pCell->paint(iRow);
                            }
                        }
                    }
                }
            }
            // 2xSigma
            // No sigma for functional tests
            if(lOtherTestCell->bTestType == 'F')
                strString = GEX_NA;
            else
            {
                lfData = 2.0*lOtherTestCell->lfSigma;
                strString = pFile->FormatTestResult(lOtherTestCell,lfData,lOtherTestCell->res_scal, false);
            }
            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
            setItem( iRow, ColPosition(TEST_2_SIGMA), pCell );
            checkItemAndAddUnit(pCell);

            // 3xSigma
            // No sigma for functional tests
            if(lOtherTestCell->bTestType == 'F')
                strString = GEX_NA;
            else
            {
                lfData = 3.0*lOtherTestCell->lfSigma;
                strString = pFile->FormatTestResult(lOtherTestCell,lfData,lOtherTestCell->res_scal, false);
            }
            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
            setItem( iRow, ColPosition(TEST_3_SIGMA), pCell );
            checkItemAndAddUnit(pCell);

            // 6xSigma
            // No sigma for functional tests
            if(lOtherTestCell->bTestType == 'F')
                strString = GEX_NA;
            else
            {
                lfData = 6.0*lOtherTestCell->lfSigma;
                strString = pFile->FormatTestResult(lOtherTestCell,lfData,lOtherTestCell->res_scal, false);
            }
            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
            setItem( iRow, ColPosition(TEST_6_SIGMA), pCell );
            checkItemAndAddUnit(pCell);

            // Min
            lfData = lOtherTestCell->lfMin;
            if(lfData == C_INFINITE || lOtherTestCell->bTestType == 'F')
                strString = GEX_NA;	// MIN value not available.
            else
                strString = pFile->FormatTestResult(lOtherTestCell,lfData,lOtherTestCell->res_scal, false);
            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
            setItem( iRow, ColPosition(TEST_MIN), pCell );
            checkItemAndAddUnit(pCell);

            // Max
            lfData = lOtherTestCell->lfMax;
            if(lfData == -C_INFINITE || lOtherTestCell->bTestType == 'F')
                strString = GEX_NA;	// MIN value not available.
            else
                strString = pFile->FormatTestResult(lOtherTestCell,lfData,lOtherTestCell->res_scal, false);
            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
            setItem( iRow, ColPosition(TEST_MAX), pCell );
            checkItemAndAddUnit(pCell);

            // Range
            lfData = lOtherTestCell->lfRange;
            if(lfData == -C_INFINITE-C_INFINITE || lOtherTestCell->bTestType == 'F')
                strString = GEX_NA;	// MIN value not available.
            else
                strString = pFile->FormatTestResult(lOtherTestCell,lfData,lOtherTestCell->res_scal, false);
            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
            setItem( iRow, ColPosition(TEST_RANGE), pCell );
            checkItemAndAddUnit(pCell);

            // Max. Range (only if comparing datasets)
            if (lOtherTestCell->bTestType == 'F')
                strString = GEX_NA;
            else
            {
                if(lGroupNum)
                {
                    // 2nd group and higher.
                    strString = " ";
                }
                else
                {
                    // First group
                    lfData = lOtherTestCell->lfMaxRange;
                    strString = pFile->FormatTestResult(lOtherTestCell,lfData,lOtherTestCell->res_scal, false);
                }
            }
            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
            setItem( iRow, ColPosition(TEST_MAX_RANGE), pCell );
            checkItemAndAddUnit(pCell);

            // Cp
            lCpString=gexReport->CreateResultStringCpCrCpk(lOtherTestCell->GetCurrentLimitItem()->lfCp);
            pCell = new DrillTableItem( QTableWidgetItem::Type, lCpString, iRow);
            setItem( iRow, ColPosition(TEST_CP), pCell );
            // Highlight in red if Cp Alarm!
            {
                bRedAlarmCpValidity = (lfRedAlarmCpValue >=0) && (lOtherTestCell->GetCurrentLimitItem()->lfCp != C_NO_CP_CPK)
                        && (lOtherTestCell->GetCurrentLimitItem()->lfCp < lfRedAlarmCpValue);
                bYellowAlarmCpValidity = (lfYellowAlarmCpValue >=0) && (lOtherTestCell->GetCurrentLimitItem()->lfCp != C_NO_CP_CPK)
                        && (lOtherTestCell->GetCurrentLimitItem()->lfCp < lfYellowAlarmCpValue);

                if(bRedAlarmCpValidity)
                    pCell->mFailing = GEX_DRILL_CELLTYPE_ALARM;		// Alarm
                else if(bYellowAlarmCpValidity)
                    pCell->mFailing = GEX_DRILL_CELLTYPE_WARNING;	// Warning
                pCell->paint(iRow);
            }

            // GCORE-199 : Cr
            lCrString=GEX_NA;
            if (lCpString!=GEX_NA && lOtherTestCell->GetCurrentLimitItem()->lfCp!=0.0)
                lCrString=gexReport->CreateResultStringCpCrCpk(1/lOtherTestCell->GetCurrentLimitItem()->lfCp);
            pCell = new DrillTableItem( QTableWidgetItem::Type, lCrString, iRow);
            setItem( iRow, ColPosition(TEST_CR), pCell );

            // Cp Shift
            if (lNbreOfGroups > 1)
            {
                if(lOtherTestCell->bTestType == 'F')
                {
                    if (lGroupList[0])
                    {
                        int lCol = ColPosition(TEST_CPSHIFT+ lGroupList[0]->strGroupName);
                        if (lCol)
                        {
                            strString = GEX_NA;
                            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
                            setItem( iRow, lCol, pCell );
                        }
                    }
                }
                else
                {
                    // print NA for all previous groups, for the rest of groups, print the shift
                    for (int lGroupIdx=0; lGroupIdx < lMaxGroupToCheck; ++lGroupIdx)
                    {
                        if (!lGroupList[lGroupIdx])
                        {
                            GSLOG(SYSLOG_SEV_ERROR, "Invalid group");
                        }
                        else
                        {
                            GS::Core::MLShift lMLShifts;
                            GetmultiLimitShift(lOtherTestCell, lGroupList[lGroupIdx]->strGroupName, lMLShifts);

                            TestShift lTestShift = lOtherTestCell->mTestShifts.value(lGroupList[lGroupIdx]->strGroupName);
                            alarmRaised = false;
                            if (lTestShift.IsValid() == false)
                            {
                                strString.sprintf( "%s", GEX_NA );
                            }
                            else
                            {
                                strString.sprintf("%.2f", lMLShifts.mCpShift);
                                strFormatDouble = gexReport->getNumberFormat()->formatNumericValue(lMLShifts.mCpShift,
                                                                                                   true, QString(strString));
                                strString.sprintf("%s %%",strFormatDouble.toLatin1().constData());
                                alarmRaised = (fabs(lMLShifts.mCpShift) >= lfOptionAlarmCp) && (bGetOptionRsltCp == true);
                            }
                            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
                            setItem(iRow, ColPosition(TEST_CPSHIFT + lGroupList[lGroupIdx]->strGroupName), pCell);
                            // Color the cell if needed
                            if(alarmRaised)
                            {
                                pCell->mFailing = GEX_DRILL_CELLTYPE_ALARM;
                                pCell->paint(iRow);
                            }
                        }
                    }
                }
            }
            // Cr Shift
            if (lNbreOfGroups > 1)
            {
                if(lOtherTestCell->bTestType == 'F')
                {
                    if (lGroupList[0])
                    {
                        int lCol = ColPosition(TEST_CRSHIFT+ lGroupList[0]->strGroupName);
                        if (lCol)
                        {
                            strString = GEX_NA;
                            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
                            setItem( iRow, lCol, pCell );
                        }
                    }
                }
                else
                {
                    // print NA for all previous groups, for the rest of groups, print the shift
                    for (int lGroupIdx=0; lGroupIdx < lMaxGroupToCheck; ++lGroupIdx)
                    {
                        if (!lGroupList[lGroupIdx])
                        {
                            GSLOG(SYSLOG_SEV_ERROR, "Invalid group");
                        }
                        else
                        {
                            GS::Core::MLShift lMLShifts;
                            GetmultiLimitShift(lOtherTestCell, lGroupList[lGroupIdx]->strGroupName, lMLShifts);
                            TestShift lTestShift = lOtherTestCell->mTestShifts.value(lGroupList[lGroupIdx]->strGroupName);
                            alarmRaised = false;
                            if (lTestShift.IsValid() == false)
                            {
                                strString.sprintf( "%s", GEX_NA );
                            }
                            else
                            {
                                strString.sprintf("%.2f", lMLShifts.mCrShift);
                                strFormatDouble = gexReport->getNumberFormat()->formatNumericValue(lMLShifts.mCrShift,
                                                                                                   true, QString(strString));
                                strString.sprintf("%s %%",strFormatDouble.toLatin1().constData());
                                alarmRaised = (fabs(lMLShifts.mCrShift) >= lfOptionAlarmCr) && (bGetOptionRsltCr == true);
                            }
                            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
                            setItem(iRow, ColPosition(TEST_CRSHIFT + lGroupList[lGroupIdx]->strGroupName), pCell);
                            // Color the cell if needed
                            if(alarmRaised)
                            {
                                pCell->mFailing = GEX_DRILL_CELLTYPE_ALARM;
                                pCell->paint(iRow);
                            }
                        }
                    }
                }
            }

            // Cpk
            pCell = new DrillTableItem( QTableWidgetItem::Type,
                                        gexReport->CreateResultStringCpCrCpk(lOtherTestCell->GetCurrentLimitItem()->lfCpk), iRow);
            setItem( iRow, ColPosition(TEST_CPK), pCell );
            // Highlight in red if Cpk Alarm!
            {
                bRedAlarmCpkValidity = (lfRedAlarmCpkValue >=0) && (lOtherTestCell->GetCurrentLimitItem()->lfCpk != C_NO_CP_CPK)
                        && (lOtherTestCell->GetCurrentLimitItem()->lfCpk < lfRedAlarmCpkValue);
                bYellowAlarmCpkValidity = (lfYellowAlarmCpkValue >=0) && (lOtherTestCell->GetCurrentLimitItem()->lfCpk != C_NO_CP_CPK)
                        && (lOtherTestCell->GetCurrentLimitItem()->lfCpk < lfYellowAlarmCpkValue);

                if(bRedAlarmCpkValidity)
                    pCell->mFailing = GEX_DRILL_CELLTYPE_ALARM;		// Alarm
                else if(bYellowAlarmCpkValidity)
                    pCell->mFailing = GEX_DRILL_CELLTYPE_WARNING;	// Warning
                pCell->paint(iRow);
            }
            // Cpk Low
            pCell = new DrillTableItem( QTableWidgetItem::Type,
                                        gexReport->CreateResultStringCpCrCpk(lOtherTestCell->GetCurrentLimitItem()->lfCpkLow), iRow);
            setItem( iRow, ColPosition(TEST_CPKL), pCell );

            // Cpk High
            pCell = new DrillTableItem( QTableWidgetItem::Type,
                                        gexReport->CreateResultStringCpCrCpk(lOtherTestCell->GetCurrentLimitItem()->lfCpkHigh), iRow);
            setItem( iRow, ColPosition(TEST_CPKH), pCell );

            // Cpk Shift
            if (lNbreOfGroups > 1)
            {
                if(lOtherTestCell->bTestType == 'F')
                {
                    if (lGroupList[0])
                    {
                        int lCol = ColPosition(TEST_CPKSHIFT+ lGroupList[0]->strGroupName);
                        if (lCol)
                        {
                            strString = GEX_NA;
                            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
                            setItem( iRow, lCol, pCell );
                        }
                    }
                }
                else
                {
                    // print NA for all previous groups, for the rest of groups, print the shift
                    for (int lGroupIdx=0; lGroupIdx < lMaxGroupToCheck; ++lGroupIdx)
                    {
                        if (!lGroupList[lGroupIdx])
                        {
                            GSLOG(SYSLOG_SEV_ERROR, "Invalid group");
                        }
                        else
                        {
                            GS::Core::MLShift lMLShifts;
                            GetmultiLimitShift(lOtherTestCell, lGroupList[lGroupIdx]->strGroupName, lMLShifts);
                            TestShift lTestShift = lOtherTestCell->mTestShifts.value(lGroupList[lGroupIdx]->strGroupName);
                            alarmRaised = false;
                            if (lTestShift.IsValid() == false)
                            {
                                strString.sprintf( "%s", GEX_NA );
                            }
                            else
                            {
                                strString.sprintf("%.2f", lMLShifts.mCpkShift);
                                strFormatDouble = gexReport->getNumberFormat()->formatNumericValue(lMLShifts.mCpkShift,
                                                                                                   true, QString(strString));
                                strString.sprintf("%s %%",strFormatDouble.toLatin1().constData());
                                alarmRaised = (fabs(lMLShifts.mCpkShift) >= lfOptionAlarmCpk)
                                        && (bGetOptionRsltCpk == true);
                            }
                            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
                            setItem(iRow, ColPosition(TEST_CPKSHIFT + lGroupList[lGroupIdx]->strGroupName), pCell);
                            // Color the cell if needed
                            if(alarmRaised)
                            {
                                pCell->mFailing = GEX_DRILL_CELLTYPE_ALARM;
                                pCell->paint(iRow);
                            }
                        }
                    }
                }
            }
            // Yield
            if(lOtherTestCell->ldExecs > 0)
            {
                lfData = 100.0 -(100.0*lOtherTestCell->GetCurrentLimitItem()->ldFailCount/lOtherTestCell->ldExecs);
                sprintf(szString,"%.2f",lfData);
                strFormatDouble = gexReport->getNumberFormat()->formatNumericValue(lfData, false,QString(szString));
                sprintf(szString,"%s",strFormatDouble.toLatin1().constData());
            }
            else
                strcpy(szString,GEX_NA);
            pCell = new DrillTableItem( QTableWidgetItem::Type, szString, iRow);
            setItem( iRow, ColPosition(TEST_YIELD), pCell );
            // Highlight in red if Yield Alarm!
            {
                bYieldAlarmGetOptionRslt = (lfYieldAlarmOptionStorageDevice >= 0)
                        && (lfData < lfYieldAlarmOptionStorageDevice);
                bYieldWarningGetOptionRslt = (lfYieldWarningoptionStorageDevice >= 0)
                        && (lfData < lfYieldWarningoptionStorageDevice);
                // !!!!! if (lfData < lfYieldWarningOptionStorageDevice),
                // of course (lfData < lfYieldAlarmOptionStorageDevice) !!!!!


                if( lOtherTestCell->ldSamplesValidExecs && bYieldAlarmGetOptionRslt)
                    pCell->mFailing = GEX_DRILL_CELLTYPE_ALARM;		// Alarm
                else if( lOtherTestCell->ldSamplesValidExecs && bYieldWarningGetOptionRslt
                         && (!bYieldAlarmGetOptionRslt) )
                    pCell->mFailing = GEX_DRILL_CELLTYPE_WARNING;	// Warning
            }

            // Compute limit space (used in R&R)
            if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL)) == 0)
            {
                lData = ptTestCell->GetCurrentLimitItem()->lfHighLimit;		// High limit exists
#if 0
                // convert HighLimit to same scale as results:
                pFile->FormatTestResultNoUnits(&fData,ptTestCell->hlm_scal);
                fData *=  ScalingPower(ptTestCell->hlm_scal);	// normalized
                fData /=  ScalingPower(ptTestCell->res_scal);	// normalized
#endif
                lfLimitSpace = lData;

                lData = ptTestCell->GetCurrentLimitItem()->lfLowLimit;		// Low limit exists
#if 0
                // convert LowLimit to same scale as results:
                pFile->FormatTestResultNoUnits(&fData,ptTestCell->llm_scal);
                fData *=  ScalingPower(ptTestCell->llm_scal);	// normalized
                fData /=  ScalingPower(ptTestCell->res_scal);	// normalized
#endif
                lfLimitSpace -= lData;
            }
            else
                lfLimitSpace = 0.0;

            // Gage EV (only display value for first group.)
            if((lGroupNum || ptTestCell->pGage == NULL))
            {
                strString = "";
            }
            else
            {
                // Check if % computed over TV or Limit space
                if (bBoxplotShiftOverTV)
                    lData = (ptTestCell->pGage->lfTV) ? ptTestCell->pGage->lfTV : 1;
                else
                    lData = (lfLimitSpace) ? lfLimitSpace : 1;

                strString = gexReport->ValueAndPercentageString(ptTestCell->pGage->lfEV,lData,lfPercent);
            }
            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
            setItem( iRow, ColPosition(TEST_GAGE_EV), pCell );

            // Gage AV (only display value for first group.)
            if((lGroupNum || ptTestCell->pGage == NULL))
            {
                strString = "";
            }
            else
            {
                // Check if % computed over TV or Limit space
                if(bBoxplotShiftOverTV)
                    lData = (ptTestCell->pGage->lfTV) ? ptTestCell->pGage->lfTV : 1;
                else
                    lData = (lfLimitSpace) ? lfLimitSpace : 1;

                strString = gexReport->ValueAndPercentageString(ptTestCell->pGage->lfAV,lData,lfPercent);
            }
            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
            setItem( iRow, ColPosition(TEST_GAGE_AV), pCell );

            // Gage R&R (only display value for first group.)
            if(lGroupNum || ptTestCell->pGage == NULL)
            {
                pCell = new DrillTableItem( QTableWidgetItem::Type,"", iRow);
            }
            else
            {
                if(bBoxplotShiftOverTV)
                    lData = (ptTestCell->pGage->lfTV) ? ptTestCell->pGage->lfTV : 1;
                else
                    lData = (lfLimitSpace) ? lfLimitSpace : 1;
                strString = gexReport->ValueAndPercentageString(ptTestCell->pGage->lfRR,lData,lfPercent);
                pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
                // Check if R&R% alarm
                gexReport->getR_R_AlarmColor(ptTestCell,lfPercent,lBackgroundColor,pCell->mCustomBkColor);
                if(qstricmp(lBackgroundColor.toLatin1().constData(),szDataColor))
                    pCell->mFailing = GEX_DRILL_CELLTYPE_USR_COLOR;
            }
            setItem( iRow, ColPosition(TEST_GAGE_RR), pCell );

            // Gage GB (only display value for first group.)
            if((lGroupNum || ptTestCell->pGage == NULL))
            {
                strString = "";
            }
            else
            {
                // Check if % computed over TV or Limit space
                if(bBoxplotShiftOverTV)
                    lData = (ptTestCell->pGage->lfGB) ? ptTestCell->pGage->lfGB : 1;
                else
                    lData = (lfLimitSpace) ? lfLimitSpace : 1;

                strString = gexReport->ValueAndPercentageString(ptTestCell->pGage->lfGB,-1,lfPercent);
            }
            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
            setItem( iRow, ColPosition(TEST_GAGE_GB), pCell );

            // Gage PV (only display value for first group.)
            if((lGroupNum || ptTestCell->pGage == NULL))
            {
                strString = "";
            }
            else
            {
                // Check if % computed over TV or Limit space
                if(bBoxplotShiftOverTV)
                    lData = (ptTestCell->pGage->lfTV) ? ptTestCell->pGage->lfTV : 1;
                else
                    lData = (lfLimitSpace) ? lfLimitSpace : 1;

                strString = gexReport->ValueAndPercentageString(ptTestCell->pGage->lfPV,lData,lfPercent);
            }
            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
            setItem( iRow, ColPosition(TEST_GAGE_PV), pCell );

            // Gage TV (only display value for first group.)
            if((lGroupNum || ptTestCell->pGage == NULL))
            {
                strString = "";
            }
            else
            {
                // Check if % computed over TV or Limit space
                if(bBoxplotShiftOverTV)
                    lData = (ptTestCell->pGage->lfTV) ? ptTestCell->pGage->lfTV : 1;
                else
                    lData = (lfLimitSpace) ? lfLimitSpace : 1;

                strString = gexReport->ValueAndPercentageString(ptTestCell->pGage->lfTV,lData,lfPercent);
            }
            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
            setItem( iRow, ColPosition(TEST_GAGE_TV), pCell );

            // Gage P/T Ratio (only display value for first group.)
            if((lGroupNum || ptTestCell->pGage == NULL)){
                strString = "";
            }else {
                strString = QString::number(ptTestCell->pGage->lfP_T,'f',2);
                strFormatDouble = gexReport->getNumberFormat()->formatNumericValue(
                            ptTestCell->pGage->lfP_T, false, strString);
                strString = strFormatDouble + QString("%");
            }

            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
            setItem( iRow, ColPosition(TEST_GAGE_P_T), pCell );

            // Skew
            strString = (lOtherTestCell->lfSamplesSkew == -C_INFINITE || lOtherTestCell->bTestType == 'F') ? GEX_NA
              : gexReport->getNumberFormat()->formatNumericValue(
                  lOtherTestCell->lfSamplesSkew, false, QString::number(lOtherTestCell->lfSamplesSkew));
            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
            setItem( iRow, ColPosition(TEST_SKEW), pCell );

            // Kurtosis
            strString = (lOtherTestCell->lfSamplesKurt == -C_INFINITE || lOtherTestCell->bTestType == 'F') ? GEX_NA
              : gexReport->getNumberFormat()->formatNumericValue(
                  lOtherTestCell->lfSamplesKurt, false, QString::number(lOtherTestCell->lfSamplesKurt));
            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
            setItem( iRow, ColPosition(TEST_KURTOSIS), pCell );

            // P0.5%
            if (lOtherTestCell->bTestType == 'F')
                strString = GEX_NA;
            else
            {
                lfData = lOtherTestCell->lfSamplesP0_5;
                strString = pFile->FormatTestResult(lOtherTestCell,lfData,lOtherTestCell->res_scal, false);
            }
            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
            setItem( iRow, ColPosition(TEST_P0_5), pCell );
            checkItemAndAddUnit(pCell);

            // P2.5%
            if (lOtherTestCell->bTestType == 'F')
                strString = GEX_NA;
            else
            {
                lfData = lOtherTestCell->lfSamplesP2_5;
                strString = pFile->FormatTestResult(lOtherTestCell,lfData,lOtherTestCell->res_scal, false);
            }
            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
            setItem( iRow, ColPosition(TEST_P2_5), pCell );
            checkItemAndAddUnit(pCell);

            // P10%
            if (lOtherTestCell->bTestType == 'F')
                strString = GEX_NA;
            else
            {
                lfData = lOtherTestCell->lfSamplesP10;
                strString = pFile->FormatTestResult(lOtherTestCell,lfData,lOtherTestCell->res_scal, false);
            }
            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
            setItem( iRow, ColPosition(TEST_P10), pCell );
            checkItemAndAddUnit(pCell);

            // P25% - Quartile1
            if (lOtherTestCell->bTestType == 'F')
                strString = GEX_NA;
            else
            {
                lfData = lOtherTestCell->lfSamplesQuartile1;
                strString = pFile->FormatTestResult(lOtherTestCell,lfData,lOtherTestCell->res_scal, false);
            }
            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
            setItem( iRow, ColPosition(TEST_Q1), pCell );
            checkItemAndAddUnit(pCell);

            // P50% - Median
            if (lOtherTestCell->bTestType == 'F')
                strString = GEX_NA;
            else
            {
                lfData = lOtherTestCell->lfSamplesQuartile2;
                strString = pFile->FormatTestResult(lOtherTestCell,lfData,lOtherTestCell->res_scal, false);
            }
            pCell = new DrillTableItem(QTableWidgetItem::Type, strString, iRow);/*( this, ,strString);*/
            setItem( iRow, ColPosition(TEST_Q2), pCell );
            checkItemAndAddUnit(pCell);
            // Highlight in red if median outside of limits!
            if( (((lOtherTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
                 && (lOtherTestCell->lfSamplesQuartile2 < lOtherTestCell->GetCurrentLimitItem()->lfLowLimit)) ||
                (((lOtherTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
                 && (lOtherTestCell->lfSamplesQuartile2 > lOtherTestCell->GetCurrentLimitItem()->lfHighLimit)) )
                pCell->mFailing = GEX_DRILL_CELLTYPE_ALARM;

            // P75% - Quartile3
            if (lOtherTestCell->bTestType == 'F')
                strString = GEX_NA;
            else
            {
                lfData = lOtherTestCell->lfSamplesQuartile3;
                strString = pFile->FormatTestResult(lOtherTestCell,lfData,lOtherTestCell->res_scal, false);
            }
            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
            setItem( iRow, ColPosition(TEST_Q3), pCell );
            checkItemAndAddUnit(pCell);

            // P90%
            if (lOtherTestCell->bTestType == 'F')
                strString = GEX_NA;
            else
            {
                lfData = lOtherTestCell->lfSamplesP90;
                strString = pFile->FormatTestResult(lOtherTestCell,lfData,lOtherTestCell->res_scal, false);
            }
            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
            setItem( iRow, ColPosition(TEST_P90), pCell );
            checkItemAndAddUnit(pCell);

            // P97.5%
            if (lOtherTestCell->bTestType == 'F')
                strString = GEX_NA;
            else
            {
                lfData = lOtherTestCell->lfSamplesP97_5;
                strString = pFile->FormatTestResult(lOtherTestCell,lfData,lOtherTestCell->res_scal, false);
            }
            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
            setItem( iRow, ColPosition(TEST_P97_5), pCell );
            checkItemAndAddUnit(pCell);

            // P99.5%
            if (lOtherTestCell->bTestType == 'F')
                strString = GEX_NA;
            else
            {
                lfData = lOtherTestCell->lfSamplesP99_5;
                strString = pFile->FormatTestResult(lOtherTestCell,lfData,lOtherTestCell->res_scal, false);
            }
            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
            setItem( iRow, ColPosition(TEST_P99_5), pCell );
            checkItemAndAddUnit(pCell);

            // IQR
            if (lOtherTestCell->bTestType == 'F')
                strString = GEX_NA;
            else
            {
                lfData = lOtherTestCell->lfSamplesQuartile3-lOtherTestCell->lfSamplesQuartile1;
                strString = pFile->FormatTestResult(lOtherTestCell,lfData,lOtherTestCell->res_scal, false);
            }
            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
            setItem( iRow, ColPosition(TEST_IQR), pCell );
            checkItemAndAddUnit(pCell);

            // IQR SD
            if (lOtherTestCell->bTestType == 'F')
                strString = GEX_NA;
            else
            {
                lfData = lOtherTestCell->lfSamplesSigmaInterQuartiles;
                strString = pFile->FormatTestResult(lOtherTestCell,lfData,lOtherTestCell->res_scal, false);
            }
            pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
            setItem( iRow, ColPosition(TEST_SIGMAIQR), pCell );
            checkItemAndAddUnit(pCell);

            // Update row index.
            iRow++;
            ++lGroupNum;
        };

        // Point to next test cell
        NextTestCell:
        ptTestCell = ptTestCell->GetNextTest();

        ++lStep;
        ++lCounter;
        if(lCounter == 500)
        {
            lCounter = 0;
            GS::Gex::Engine::GetInstance().UpdateProgressStatus(true, 100, (lStep*100)/lMax);
            inProgress();
        }

    };	// Loop until all test cells read.


    if(iRow < rowCount())
    {
        for(int i = rowCount() ; i >= iRow; --i)
            removeRow(i);
    }

    // Enable paint after filling the gui
    setUpdatesEnabled(true);
    setSortingEnabled(true);

    QString lSortOption = ReportOptions.GetOption( QString("statistics"), QString("sorting") ).toString();

    if(mSortCol.contains(lSortOption))
    {
        //GS::Gex::Engine::GetInstance().UpdateLabelStatus(" Loading Parametric Table (Sorting elements) ...");
        //QCoreApplication::processEvents();
        inProgress();
        int lColum = mColPosition[mSortCol[lSortOption]];
        sortColumn(lColum, true, true);
    }

    horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

void DrillParametricTable::GetmultiLimitShift(CTest* testCell,
                                              const QString& groupName,
                                              GS::Core::MLShift& multiLimitShifts)
{
    // Get the shift for the reference test for the case of compare only with reference
    if (testCell->mTestShifts.size() <= 0)
        return;

    // Take the shift corresponding to the current multi limit
    TestShift lRefShift = testCell->mTestShifts.value(groupName);
    for (int lShiftIndex=0; lShiftIndex<lRefShift.mMLShifts.size(); ++lShiftIndex)
    {
        QPair<GS::Core::MultiLimitItem*, GS::Core::MLShift> lMLShiftLocal;
        lMLShiftLocal = lRefShift.mMLShifts[lShiftIndex];
        if (lMLShiftLocal.first == testCell->GetCurrentLimitItem())
        {
            multiLimitShifts = lRefShift.mMLShifts[lShiftIndex].second;
            break;
        }
    }
}

void DrillParametricTable::sortColumn(int col, bool ascending,
                                      bool )
{
    // Refuse to sort Test limits columns
    if(col == 4 || col == 5)
        return;

    // do sort
    Qt::SortOrder ascendingOrder = Qt::AscendingOrder;
    if (!ascending) ascendingOrder = Qt::DescendingOrder;
    QTableWidget::sortItems( col, ascendingOrder);

    // Set focus on first cell in the column!
    setCurrentCell(0,1);
}

QString DrillParametricTable::ColIdentifier(int columnPos) const
{
     QMap<QString, int>::const_iterator lIterBegin(mColPosition.begin()), lIterEnd(mColPosition.end());
     for(;lIterBegin != lIterEnd; ++lIterBegin)
     {
         if(lIterBegin.value() == columnPos)
             return lIterBegin.key();
     }
     return "";
}

int DrillParametricTable::ColPosition(const QString& column) const
{
    if (!mColPosition.contains(column))
    {
        GSLOG(3, QString("Invalid column id: %1").arg(column).toLatin1().data());
        return -1;
    }
    return mColPosition.value(column);
}
