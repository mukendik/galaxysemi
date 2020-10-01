#include <gqtl_log.h>
#include "drill_device_table.h"
#include "gex_report.h"
#include "browser_dialog.h"
#include "cpart_info.h"

extern CGexReport *		gexReport;			// Handle to report class
//extern GexMainwindow *	pGexMainWindow;
// from cstats.cpp
extern double ScalingPower(int iPower);

bool DeviceItemLessThan(QString& string1, QString& string2)
{
    int lNumber1, lNumber2;
    lNumber1 = string1.section(".", 0, 0).toInt();
    lNumber2 = string2.section(".", 0, 0).toInt();
    if (lNumber1 == lNumber2)
    {
        return (string1.section(".", 1, 1).toInt() < string2.section(".", 1, 1).toInt());
    }
    else
        return (lNumber1 < lNumber2);
}

DrillDeviceTable::DrillDeviceTable(GexWizardTable *parentWizard, QWidget * parent, Loading* loading)
    : DrillTable(parentWizard, parent, loading), mHideTestsWithNoResults(false), mSubLotIndex(-1)
{

    setObjectName("GSDeviceTable");
    setSortingEnabled(true);
    setGeometry( QRect( 21, 79, 620, 255 ) );
}

void DrillDeviceTable::SetSubLotIndex(const gsint8 subLotIndex)
{
    mSubLotIndex = subLotIndex;
}

gsint8 DrillDeviceTable::GetSubLotIndex() const
{
    return mSubLotIndex;
}


void DrillDeviceTable::setHideTestsWithNoResults(bool lHideTestsWithNoResults)
{
    mHideTestsWithNoResults = lHideTestsWithNoResults;
}

void DrillDeviceTable::ResetTable(const gsint8 lSubLotIndex/*=-1*/)
{
    GSLOG(5, "Drill Device Table Reset...");

    QHeaderView	*pTableHeader=0;
    DrillTableItem *pCell=0;
    int	iRow=0,iCol=0;
    CGexGroupOfFiles *pGroup = 0;
    CGexFileInGroup *pFile = 0;
    // Quiet return if no data & report available!
    if(gexReport == NULL)
      return;

    // Get pointer to horizontal header: show that Test# are sorted in ascending order (default)
    pTableHeader = horizontalHeader();
    pTableHeader->setSortIndicator(0, Qt::AscendingOrder);
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Delete content of table if any...
    GSLOG(SYSLOG_SEV_NOTICE, QString("Deleting up to %1 DrillTableItem").arg(rowCount()*columnCount())
          .toLatin1().data() );
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

    GSLOG(SYSLOG_SEV_NOTICE, QString("Now %1 intances of DrillTableItem").arg(DrillTableItem::GetNumOfInstances())
          .toLatin1().data());

    // Clear the device list
    if (lSubLotIndex == 0 || lSubLotIndex == -1)
        mParentWizard->comboBoxDevice->clear();
    mParentWizard->m_poCBGroupPartResult->clear();
    // Load Device list...
    if(gexReport->getGroupsList().count() > 1 )
    {
        //compare files
        mParentWizard->m_poCBGroupPartResult->show();
        mParentWizard->m_poLabelGroupPartResult->show();

        //Update ComboBox
        for(int iIdx=0; iIdx < gexReport->getGroupsList().count();iIdx++)
        {
            mParentWizard->m_poCBGroupPartResult->addItem(
                        gexReport->getGroupsList()[iIdx]->strGroupName, QVariant((int)iIdx));
        }
        //Need to update part Result combo box.
        pGroup = gexReport->getGroupsList()[0];
        pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
        mParentWizard->m_poCBGroupPartResult->setCurrentIndex(0);
    }
    else
    {
        mParentWizard->m_poCBGroupPartResult->hide();
        mParentWizard->m_poLabelGroupPartResult->hide();

        // Get handle to HardBin parameter so to know how many parts tested in total.
        pGroup = gexReport->getGroupsList().first();
        pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
    }

    updatePartResultComboBox(pGroup, pFile, lSubLotIndex);
    GSLOG(5, "Device table reset end.");
}

void DrillDeviceTable::updatePartResultComboBox(CGexGroupOfFiles *pGroup,
                                                CGexFileInGroup *pFile,
                                                const gsint8 lSubLotIndex/*=-1*/)
{
    GSLOG(6, "Update Part Result ComboBox starts...");

    mSubLotIndex = lSubLotIndex;
    CTest*	ptBinTestCell = NULL;
    QString	lDeviceString;
    int lBegin=0;
    int lEnd=0;

    if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_HBIN, GEX_PTEST, &ptBinTestCell, true, false) != 1)
      return;

    for (gsint16 lFile = 0; lFile < pGroup->pFilesList.count(); ++lFile)
    {
        // If lSubLotIndex=-1; show all parts
        if ((lSubLotIndex != -1) && (lFile != lSubLotIndex))
        {
            continue;
        }

        // Get the sublot offset
        ptBinTestCell->findSublotOffset(lBegin, lEnd, lFile);
        GSLOG(6, QString("Looping over %1 test results...").arg(lEnd-lBegin).toLatin1().data() );
        pFile = pGroup->pFilesList.at(lFile);
        QVariant lTRIndex=0; // 7523 : Test Result index variant

        for(gsint32 lIndex = lBegin; lIndex < lEnd; lIndex++)
        {
            // Add device only if he's not filtered
            if (ptBinTestCell->m_testResult.isValidIndex(lIndex)
                 && ptBinTestCell->m_testResult.isValidResultAt(lIndex))
            {
                lDeviceString	=  QString::number(lIndex+1);
                lDeviceString	+= " - Part : ";
                CPartInfo* lPartInfo = pFile->pPartInfoList.at(lIndex-lBegin);
                if (lPartInfo)
                {
                    lDeviceString += lPartInfo->getPartID();

                    // 7523 : addItem speed is not linear, but more and more slow with time
                    lTRIndex=(int)lIndex;

                    // Fill the list of devices QMultiHash((x,y), index))
                    QPair<gsint16,gsint16> lCoord(lPartInfo->GetDieX(), lPartInfo->GetDieY());

                    // add the device only if it doesn't exist
                    if (mParentWizard->comboBoxType->currentText().contains("Die XY") &&
                        (!mParentWizard->mDevicesInComboBox.contains(lCoord)))
                    {
                        QString lCurrentItemText = QString::number(lCoord.first) + "." +QString::number(lCoord.second);
                        if (!mParentWizard->mItemsPartsComboBox.contains(lCurrentItemText))
                            mParentWizard->mItemsPartsComboBox.append(lCurrentItemText);
                    }
                    else
                        mParentWizard->comboBoxDevice->addItem(lDeviceString, lTRIndex);

                    if (mParentWizard->mDevicesByRun.contains(lCoord))
                    {
                        QList<gsint32> lTempList = mParentWizard->mDevicesByRun.value(lCoord);
                        if (!lTempList.contains(lIndex))
                        {
                            lTempList.append(lIndex);
                            mParentWizard->mDevicesByRun.insert(lCoord,lTempList);
                        }
                    }
                    else
                    {
                        QList<gsint32> lTempList;
                        lTempList.append(lIndex);
                        mParentWizard->mDevicesByRun.insert(lCoord, lTempList);
                    }
                    // todo : try addItems(QStringList) ? impossible : we want to specify a user data
                }
            }
        }
    }

    GSLOG(6, QString("Device combobox has now %1 item(s)").arg(mParentWizard->comboBoxDevice->count())
          .toLatin1().data() );
    if (mParentWizard->comboBoxType->currentText().contains("Die XY"))
    {
        // Sort the items in the combo box
        qSort(mParentWizard->mItemsPartsComboBox.begin(),
              mParentWizard->mItemsPartsComboBox.end(),
              DeviceItemLessThan);

        //-- check that the value has been already inserted
        QList<QString>::iterator lIter = mParentWizard->mItemsPartsComboBox.begin();
        QList<QString>::iterator lIterEnd = mParentWizard->mItemsPartsComboBox.end();
        for(; lIter != lIterEnd; ++lIter)
        {
            if(mParentWizard->comboBoxDevice->findText(*lIter) == -1)
            {
                mParentWizard->comboBoxDevice->addItems(mParentWizard->mItemsPartsComboBox);
            }
        }
    }
    // Select the first item
    mParentWizard->comboBoxDevice->setCurrentIndex(0);
    // Refresh GUI
    mParentWizard->OnGetDeviceResults(lSubLotIndex);
}

void DrillDeviceTable::OnHideTestsWithNoResults(bool lHideTestsWithNoResults)
{
    setHideTestsWithNoResults(lHideTestsWithNoResults);

    // Hide/Show tests with no results rows
    if (mParentWizard)
    {
        int lSubLotIndex(-1);
        if (mParentWizard->comboBoxType->currentText().contains("Die XY"))
            lSubLotIndex = mSubLotIndex;
        mParentWizard->OnGetDeviceResults(lSubLotIndex);
    }
    else
        GSLOG(SYSLOG_SEV_WARNING, "No Interactive Table widget instantiated, unable to hide/show test with no results");
}


QString DrillDeviceTable::LoadDeviceResults(long lRunOffset)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Load Device Results for run offset %1...").arg(lRunOffset)
          .toLatin1().data() );

    if (!gexReport)
        return "error: report null";

    // Get handle to HardBin parameter
    int lCurrentCBIndex=mParentWizard->m_poCBGroupPartResult->currentIndex();
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Current Group ComboBox index %1").arg(lCurrentCBIndex).toLatin1().data() );
    QVariant lItemData=mParentWizard->m_poCBGroupPartResult->itemData( lCurrentCBIndex );
    bool lOk=false;
    int lCurrentGroup=lItemData.toInt(&lOk);// will returns 0 on error
    //if (!lOk)
      //  return "error: cannot retrieve current selected group index:"+lItemData.toString();
    if (lCurrentGroup<0 || lCurrentGroup>gexReport->getGroupsList().size()-1)
        return "error: current group index out of range"+lItemData.toString();
    CGexGroupOfFiles *pGroup = gexReport->getGroupsList()[lCurrentGroup];
    if (!pGroup)
        return "error: group null";
    CGexFileInGroup *pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
    if (!pFile)
        return "error: file null";
    CTest *ptTestCell=0;
    if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_HBIN,GEX_PTEST,&ptTestCell,true,false) != 1)
        return QString("error: Cannot find Test cell on file %1").arg(pFile->lFileID);

    QString strOptionStorageDevice = (ReportOptions.GetOption("statistics","generic_galaxy_tests")).toString();

    // GCORE-1336
    setSortingEnabled(false);

    // Set number of rows in table counter
    int iRow=0,iCol=0;
    ptTestCell = pGroup->cMergedData.ptMergedTestList;
    while(ptTestCell != NULL)
    {
        // IF Muti-result parametric test, do not show master test record
        if(ptTestCell->lResultArraySize > 0)
            goto NextCell;

        // Ignore Generic Galaxy Parameters
        {
            if((ptTestCell->bTestType == '-') && (strOptionStorageDevice == "hide"))
                goto NextCell;
        }

        if (gexReport->isInteractiveTestFiltered(ptTestCell) == false)
            goto NextCell;

        // Add device only if he's not filtered
        // Run not found or invalid result for this run
        if ((ptTestCell->m_testResult.isValidIndex(lRunOffset) == false ||
            ptTestCell->m_testResult.isValidResultAt(lRunOffset) == false) && mHideTestsWithNoResults)
        {
            goto NextCell;
        }

        // this is a valid test, count it!
        iRow++;

        // Point to next test cell
        NextCell:
        ptTestCell = ptTestCell->GetNextTest();
    };
    setRowCount(iRow);

    setColumnCount(12); // GCORE-31
    QStringList headerList;
    headerList.append("Test");
    headerList.append("FlowID");
    headerList.append("Name");
    headerList.append("Type");
    headerList.append("Low Limit");
    headerList.append("High Limit");
    // GCORE-31
    headerList.append("Mean");
    headerList.append("Sigma");

    headerList.append("Measurement");
    // GCORE-31
    headerList.append("Z Score");
    headerList.append("Absolute Z Score");

    headerList.append("P/F");

    setHorizontalHeaderLabels(headerList);
    horizontalHeaderItem(9)->
        setToolTip("Normalized distance (Z Score) ="
                   " (TestValue - TestMean)/(TestSigma)");
    horizontalHeaderItem(10)->
        setToolTip("Absolute Z Score ="
                   " | (TestValue - TestMean)/(TestSigma) |");

#ifdef _WIN32
    // Resize columns so most of columns fit in the window...
    setColumnWidth(0,60); // Test #
    setColumnWidth(1,60); // FlowID
    setColumnWidth(2,140); // Test name
    setColumnWidth(3,40); // Test type
    setColumnWidth(4,80); // Low Limit
    setColumnWidth(5,80); // High Limit
    setColumnWidth(6,100); // Mean
    setColumnWidth(7,100); // Sigma
    setColumnWidth(8,130); // Measurement
    setColumnWidth(9,80);	// Dist
    setColumnWidth(10,80);	    // Abs Dist

    setColumnWidth(11,40);	// Pass/Fail
#endif

    // List of test results...
    DrillTableItem *			pCell=0;
    CPartInfo *					pPartInfo = NULL;
    char						szString[1024]="";
    QString						strString,strUnits;
    double						lfData=0.0, lfCustomScaleFactor=0.0;
    bool						bFailedTestGoodResult = false;
    CTestResult::PassFailStatus lRunStatus = CTestResult::statusUndefined;

    ptTestCell = pGroup->cMergedData.ptMergedTestList;

    // Looking for partinfo
    int idxPartInfo = lRunOffset;
    for (int idxFile = 0; idxFile < pGroup->pFilesList.count() && pPartInfo==NULL; ++idxFile)
    {
        CGexFileInGroup * pTmpFile = pGroup->pFilesList.at(idxFile);
        if (pTmpFile)
        {
            if (idxPartInfo < pTmpFile->pPartInfoList.count())
                pPartInfo = pTmpFile->pPartInfoList.at(idxPartInfo);
            else
                idxPartInfo -= pTmpFile->pPartInfoList.count();
        }
    }

    QString scalingOption = ReportOptions.GetOption("dataprocessing","scaling").toString();
    iRow = 0;
    double lDistance=0.0;
    while(ptTestCell != NULL)
    {
        lfData=GEX_C_DOUBLE_NAN;  //or POSIX NAN ?
        lDistance=0.0;
        char lTempString[GEX_LIMIT_LABEL]="";

        //Reset the value for each test
        lRunStatus = CTestResult::statusUndefined;
        // If Muti-result parametric test, do not show master test record
        if(ptTestCell->lResultArraySize > 0)
            goto NextTestCell;

        // Ignore Generic Galaxy Parameters
        {
            if((ptTestCell->bTestType == '-') && (strOptionStorageDevice == "hide"))
                goto NextTestCell;
        }

        if (gexReport->isInteractiveTestFiltered(ptTestCell) == false)
            goto NextTestCell;

        // Add device only if he's not filtered
        if ((ptTestCell->m_testResult.isValidIndex(lRunOffset) == false ||
            ptTestCell->m_testResult.isValidResultAt(lRunOffset) == false) &&
            mHideTestsWithNoResults)
        {
            goto NextTestCell;
        }

        // Check if test is failed even its value is inside the limits
        if (pPartInfo && pPartInfo->lstFailedTestGoodResult.indexOf(ptTestCell) != -1)
            bFailedTestGoodResult = true;
        else
            bFailedTestGoodResult = false;

        // Insert test info into the table...
        iCol = 0;

        // Test #
        pCell = new DrillTableItem( QTableWidgetItem::Type, ptTestCell->szTestLabel, iRow);
        setItem( iRow, iCol++, pCell );

        // FlowID
        // -- replace -1 by n/a . in order to have the galaxy test (-1) in the end of the list when ascending order
        if(ptTestCell->lTestFlowID <0)
            strcpy(szString, "n/a .");
        else
            sprintf(szString,"%d",ptTestCell->lTestFlowID);
        pCell = new DrillTableItem( QTableWidgetItem::Type, szString, iRow);
        setItem( iRow, iCol++, pCell );

        // Test name
        gexReport->BuildTestNameString(pFile,ptTestCell,szString);
        pCell = new DrillTableItem( QTableWidgetItem::Type, szString, iRow);
        setItem( iRow, iCol++, pCell );

        // Test type.
        gexReport->BuildTestTypeString(pFile, ptTestCell, szString, true);
        pCell = new DrillTableItem( QTableWidgetItem::Type, szString, iRow);
        setItem( iRow, iCol++, pCell );

        // LL

        if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
            pFile->FormatTestLimit(ptTestCell,
                                   lTempString,
                                   ptTestCell->GetCurrentLimitItem()->lfLowLimit,
                                   ptTestCell->llm_scal,
                                   false);
        else
            strcpy(lTempString, GEX_NA);
        pCell = new DrillTableItem( QTableWidgetItem::Type, lTempString, iRow);
        setItem( iRow, iCol++, pCell );
        checkItemAndAddUnit(pCell);

        // HL
        if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
            pFile->FormatTestLimit(ptTestCell,
                                   lTempString,
                                   ptTestCell->GetCurrentLimitItem()->lfHighLimit,
                                   ptTestCell->hlm_scal,
                                   false);
        else
            strcpy(lTempString, GEX_NA);
        pCell = new DrillTableItem( QTableWidgetItem::Type, lTempString, iRow);
        setItem( iRow, iCol++, pCell );
        checkItemAndAddUnit(pCell);


        // GCORE-31 :
        // Test mean
        /* // formatNumericValue does not always return the mean in the same scale as the data
         strString = gexReport->getNumberFormat()->formatNumericValue(
                    ptTestCell->lfMean, // *lfCustomScaleFactor,
                    true,
                    QString::number(ptTestCell->lfMean,'g',15))
                + strUnits;
        */
        // FormatTestResult will add the units at the end of the string
        strString=QString(pFile->FormatTestResult(ptTestCell, ptTestCell->lfMean, ptTestCell->res_scal));
        pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow );
        setItem( iRow, iCol++, pCell );
        checkItemAndAddUnit(pCell);

        // Test sigma
        strString=QString(pFile->FormatTestResult(ptTestCell, ptTestCell->lfSigma, ptTestCell->res_scal));
        pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow );
        setItem( iRow, iCol++, pCell );
        checkItemAndAddUnit(pCell);

        // Measurement
        strString = GEX_NA;     // default value if test result isn't correct
        if( (ptTestCell->m_testResult.isValidIndex(lRunOffset)) && (ptTestCell->ldSamplesValidExecs != 0))
        {
            if(ptTestCell->m_testResult.isValidResultAt(lRunOffset))
            {
                // test result
                lfData = ptTestCell->m_testResult.resultAt(lRunOffset);

                // Build result string
                strUnits = ptTestCell->GetScaledUnits(&lfCustomScaleFactor, scalingOption);
                strString = gexReport->getNumberFormat()->formatNumericValue(
                            lfData*lfCustomScaleFactor, true,
                            QString::number(lfData*lfCustomScaleFactor,'g',15))
                            + " "+strUnits; // GCORE-31 : Nico want the units separated from the numbers

                // Scale result to be normalized as test limits
                lfData *= ScalingPower(ptTestCell->res_scal);
                // Check if failing value
                if(ptTestCell->isFailingValue(lfData, ptTestCell->m_testResult.passFailStatus(lRunOffset)))
                    lRunStatus = CTestResult::statusFail;
                else
                    lRunStatus = CTestResult::statusPass;
            }
            else
            {
                lRunStatus = ptTestCell->m_testResult.passFailStatus(lRunOffset);
            }


        }
        pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
        if (bFailedTestGoodResult)
            pCell->mFailing = GEX_DRILL_CELLTYPE_ALARM_LIGHT;
        else if(lRunStatus == CTestResult::statusFail)
            pCell->mFailing = GEX_DRILL_CELLTYPE_ALARM;
        pCell->paint(iRow);
        setItem( iRow, iCol++, pCell );
        checkItemAndAddUnit(pCell);

        // GCORE-31

        // Normalized distance = (TestResult - TestMean)/(Test Sigma)
        if ( lfData!=GEX_C_DOUBLE_NAN && ptTestCell->lfSigma!=0.0)
        {
            lDistance=(lfData - ptTestCell->lfMean)/ptTestCell->lfSigma;
            strString = QString::number(lDistance); //+" "+strUnits;
        }
        if (ptTestCell->lfSigma==0.0)
            strString = GEX_NA;
        pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow );
        setItem( iRow, iCol++, pCell );

        // Absolute distance
        if (lfData!=GEX_C_DOUBLE_NAN && ptTestCell->lfSigma!=0.0)
        {
            strString = QString::number( fabs(lDistance) );
        }
        if (ptTestCell->lfSigma==0.0)
            strString = GEX_NA;
        pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow );
        setItem( iRow, iCol++, pCell );


        // P/F
        if (lRunStatus == CTestResult::statusFail)
            strString = "* F *";
        else if(lRunStatus == CTestResult::statusPass)
            strString = "  P";
        else
            strString = GEX_NA;
        pCell = new DrillTableItem( QTableWidgetItem::Type, strString, iRow);
        setItem( iRow, iCol++, pCell );



        // Next table row
        iRow++;

        // Next test cell
NextTestCell:
        ptTestCell = ptTestCell->GetNextTest();
    };

    GSLOG(SYSLOG_SEV_NOTICE, QString("Now %1 instances of DrillTableItem").arg(DrillTableItem::GetNumOfInstances())
          .toLatin1().data());

    // Write Part info (site#, DieXY, SBIN)...unless this part was manually erased (in which case all data are NaN)
    bool bValidPart=true;
    if (mSubLotIndex >= 0)
    {
        strString = QString("WaferID: %1, ").arg(mSubLotIndex + 1);
    }
    else
    {
        strString = "";
    }
    if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_TESTING_SITE,GEX_PTEST,&ptTestCell,true,false) == 1)
    {
        if(ptTestCell->m_testResult.isValidIndex(lRunOffset))
        {
            if(ptTestCell->m_testResult.isValidResultAt(lRunOffset))
                strString += "Site: " + QString::number(ptTestCell->m_testResult.resultAt(lRunOffset));
            else
                // Flag that this Run no longer exists (eg: because manually erased from Interactive wafermap)
                bValidPart = false;
        }
    }

    if (bValidPart)
    {
        if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEX,GEX_PTEST,&ptTestCell,true,false) == 1)
        {
            //if(ptTestCell->m_testResult.count() > 0 && lRunOffset < ptTestCell->ldSamplesExecs)
            if( ptTestCell->m_testResult.isValidIndex(lRunOffset) )
                strString += ",  DieXY: (" + QString::number(ptTestCell->m_testResult.resultAt(lRunOffset)) + ",";

            if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEY,GEX_PTEST,&ptTestCell,true,false) == 1)
            {
                //if(ptTestCell->m_testResult.count() > 0 && lRunOffset < ptTestCell->ldSamplesExecs)
                if( ptTestCell->m_testResult.isValidIndex(lRunOffset) )
                    strString += QString::number(ptTestCell->m_testResult.resultAt(lRunOffset)) + ")";
            }
        }

        if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_SBIN,GEX_PTEST,&ptTestCell,true,false) == 1)
        {
            //if(ptTestCell->m_testResult.count() > 0 && lRunOffset < ptTestCell->ldSamplesExecs)
            if( ptTestCell->m_testResult.isValidIndex(lRunOffset) )
                strString += " - Soft BIN: " + QString::number(ptTestCell->m_testResult.resultAt(lRunOffset));
        }

        if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_HBIN,GEX_PTEST,&ptTestCell,true,false) == 1)
        {
            //if(ptTestCell->m_testResult.count() > 0 && lRunOffset < ptTestCell->ldSamplesExecs)
            if(ptTestCell->m_testResult.isValidIndex(lRunOffset))
                strString += ",  Hard BIN: " + QString::number(ptTestCell->m_testResult.resultAt(lRunOffset));
        }

        QString strTestTimes;
        if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_TTIME,GEX_PTEST,&ptTestCell,true,false) == 1)
        {
            //if(ptTestCell->m_testResult.count() > 0 && lRunOffset < ptTestCell->ldSamplesExecs)
            if(ptTestCell->m_testResult.isValidIndex(lRunOffset))
            {
                strTestTimes += ", Test time: " + QString::number(ptTestCell->m_testResult.resultAt(lRunOffset));
                strTestTimes += " sec";
            }
        }
        if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_TEST_TIME_T,GEX_PTEST,&ptTestCell,true,false) == 1)
        {
            //if(ptTestCell->m_testResult.count() > 0 && lRunOffset < ptTestCell->ldSamplesExecs)
            if(ptTestCell->m_testResult.isValidIndex(lRunOffset))
            {
                QDateTime clDateTime;
                clDateTime.setTime_t((uint)ptTestCell->m_testResult.resultAt(lRunOffset));
                if(!strTestTimes.isEmpty())
                    strTestTimes += ", ";
                strTestTimes += "Time of test: " + clDateTime.toString(Qt::ISODate);
            }
        }
        if(!strTestTimes.isEmpty())
            strString += strTestTimes;
    }
    else
        strString = "No Data available for this run (probably manualy filtered by user)";

    // Update GUI with part details
    emit onUpdatePartLabel(strString);

    // GCORE-1336
    setSortingEnabled(true);


    QString lSortOption = ReportOptions.GetOption( QString("statistics"), QString("sorting") ).toString();

    if(lSortOption == "test_number")
        sortColumn(0, true, true);
    else if(lSortOption == "test_flow_id")
        sortColumn(1, true, true);
    else if(lSortOption == "test_name")
        sortColumn(2, true, true);

    return "ok";
}

void DrillDeviceTable::sortColumn(int col, bool ascending,
                                  bool /*wholeRows*/)
{
    GSLOG(SYSLOG_SEV_NOTICE, QString("DrillDeviceTable sort by column %1 %2").arg(col).arg(ascending?"asc":"dsc")
          .toLatin1().data());
    QHeaderView *pTableHeader=0;

    Qt::SortOrder order = Qt::AscendingOrder;
    if (!ascending)
        order = Qt::DescendingOrder;

    // Get pointer to horizontal header
    pTableHeader = horizontalHeader();
    pTableHeader->setSortIndicator(col,order);

    // do sort
    QTableWidget::sortItems( col, order);

    // Set focus on first cell in the column!
    setCurrentCell(0,1);
}


DrillDeviceWidget::DrillDeviceWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout * lVBoxLayout = new QVBoxLayout(this);

    mDeviceLabel    = new QLabel(this);
    mDeviceTable    = new DrillDeviceTable(static_cast<GexWizardTable*>(parent), this);

    lVBoxLayout->addWidget(mDeviceLabel);
    lVBoxLayout->addWidget(mDeviceTable);
    lVBoxLayout->setContentsMargins(0,6,0,0);
    lVBoxLayout->setSpacing(6);

    setLayout(lVBoxLayout);

    connect(mDeviceTable, SIGNAL(onUpdatePartLabel(QString)),
            mDeviceLabel, SLOT(setText(QString)));
}

DrillDeviceWidget::~DrillDeviceWidget()
{

}

DrillDeviceTable * DrillDeviceWidget::GetDeviceTable()
{
    return mDeviceTable;
}

void DrillDeviceTable::UpdateMultiLimit(int multiLimit)
{
    QList<CGexGroupOfFiles*> lGroupsList = gexReport->getGroupsList();
    for (int lIndex=0; lIndex<lGroupsList.size(); ++lIndex)
    {
        lGroupsList[lIndex]->setLimitSetId(multiLimit);
        lGroupsList[lIndex]->SwitchLimitIndex();

    }
    int lSubLotIndex(-1);

    if (mParentWizard->comboBoxType->currentText().contains("Die XY"))
        lSubLotIndex = mSubLotIndex;
    mParentWizard->OnGetDeviceResults(lSubLotIndex);
}

