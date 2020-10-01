#include <QProgressDialog>
#include <QMessageBox>
#include "message.h"
#include "report_build.h"
#include "report_options.h"
#include "browser_dialog.h"
#include "browser.h"
#include "picktest_dialog.h"
#include "picktest_item.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "gex_report.h"
#include "scripting_io.h"
#include "classes.h"
#include "product_info.h"
#include "engine.h"

// in main.cpp
extern GexMainwindow *	pGexMainWindow;

// in report_build.cpp
extern CGexReport*		gexReport;					// Handle to report class
extern CReportOptions	ReportOptions;			// Holds options (report_build.h)


//////////////////////////////////////////////////////////
// Constructor
PickTestDialog::PickTestDialog(QWidget* parent, bool modal, Qt::WindowFlags f )
    : QDialog( parent, f ), m_strFilterExpr("*"), m_eTestTypeAllowed(TestParametric | TestMultiParametric | TestGenericGalaxy)
{
    setupUi(this);
    setModal(modal);

    // Apply Examinator palette
    GexMainwindow::applyPalette(this);

    QObject::connect(PushButtonOk,			SIGNAL(clicked()),	this, SLOT(accept()));
    QObject::connect(PushButtonCancel,		SIGNAL(clicked()),	this, SLOT(reject()));
    QObject::connect(comboBoxGroupName,		SIGNAL(activated(int)),			this, SLOT(OnSelectGroup(int)));
    QObject::connect(lineEditParamFilter,	SIGNAL(textChanged(QString)),	this, SLOT(OnParamFilter()));
    QObject::connect(treeWidget,			SIGNAL(itemSelectionChanged()),						this, SLOT(OnTestSelection()));
    QObject::connect(treeWidget,			SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),	this, SLOT(accept()));

    QObject::connect(lineEditTestFilter, SIGNAL(textChanged(const QString &)), this, SLOT(updatePickerFromFilter(const QString &)));
    QObject::connect(comboBoxSyntax, SIGNAL(currentIndexChanged(int)),
                     SLOT(OnChangeSyntax(int)));
    comboBoxSyntax->addItem(QString("RegExp"));
    comboBoxSyntax->addItem(QString("Wildcard"));
    comboBoxSyntax->setCurrentIndex(1);
    mPatternSyntax = QRegExp::Wildcard;

    // Change labels according to the application type.
    if(GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPRO() ||
       GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT() ||
       GS::LPPlugin::ProductInfo::getInstance()->isExaminatorTerProPlus())
    {
            TextLabelGroupName->setText("Dataset:");
    }
    else
    {
            TextLabelGroupName->setText("Group name:");
    }

    // Defaults
    setMultipleSelection(false);
    setMultipleGroups(true,true);

    m_bUseInteractiveFilters = false;

    // Compress blocks when it s possible
    mCompressBlocks = true;

    // Enbaled sort
    treeWidget->setSortingEnabled(true);
    treeWidget->sortByColumn(0, Qt::AscendingOrder);

    treeWidget->resizeColumnToContents(0);
    treeWidget->setColumnWidth(1, 250);
    treeWidget->resizeColumnToContents(2);
    treeWidget->resizeColumnToContents(3);

    // Set the focus on the search field
    lineEditParamFilter->setFocus();
    lineEditParamFilter->selectAll();
}

PickTestDialog::~PickTestDialog()
{
    clearPickTestItemsList();
}

///////////////////////////////////////////////////////////
// Loads Combo box with list of group names.
///////////////////////////////////////////////////////////
void PickTestDialog::EnumerateGroupNames(void)
{
    // Empty list box
    comboBoxGroupName->clear();

    int iItemCount = 0;
    // If allowed to select "All datasets", insert this item first in the list
    if(bAllowAllGroupsSelection){
        comboBoxGroupName->insertItem(comboBoxGroupName->count(), "All Datasets!");
        iItemCount = 1;
    }

    QListIterator<CGexGroupOfFiles*>	itGroupsList(gexReport->getGroupsList());
    CGexGroupOfFiles *					pGroup = NULL;

    // Insert all the datasets names in the list
    while(itGroupsList.hasNext())
    {
        pGroup = itGroupsList.next();
        comboBoxGroupName->insertItem(comboBoxGroupName->count(), pGroup->strGroupName);
        iItemCount++;
        if(iItemCount == 1)
            comboBoxGroupName->setCurrentIndex(0);
        else if(iItemCount == 2)
            comboBoxGroupName->setCurrentIndex(1);
    }

    // If only one group, hide the group selection!
    if(gexReport->getGroupsList().count() <= 1)
    {
        TextLabelGroupName->hide();
        comboBoxGroupName->hide();
    }
}

///////////////////////////////////////////////////////////
// Returns the GroupID to which the test# selected belong to.
///////////////////////////////////////////////////////////
int PickTestDialog::getGroupID(void)
{
    int	iGroupID = comboBoxGroupName->currentIndex();

    // If list box includes "All dataset", then this selection (0) must be mapped to -1, and all string selections as well
    if(bAllowAllGroupsSelection)
        iGroupID--;

    return iGroupID;
}

///////////////////////////////////////////////////////////
// User selecting a specific Group from combo selection
///////////////////////////////////////////////////////////
void PickTestDialog::OnSelectGroup(int iGroup)
{
    int	iGroupID = iGroup;

    // If list box includes "All dataset", then GroupID must be offest by 1
    if(bAllowAllGroupsSelection)
        iGroupID--;

    updateTreeWidget(iGroupID);
}

/******************************************************************************!
 * \fn OnChangeSyntax
 ******************************************************************************/
void PickTestDialog::OnChangeSyntax(int lSyntax)
{
    switch (lSyntax)
    {
    case 0:
        mPatternSyntax = QRegExp::RegExp;
        break;
    default:
        mPatternSyntax = QRegExp::Wildcard;
        break;
    }
    this->updateTreeWidget(this->getGroupID());
}

///////////////////////////////////////////////////////////
// User filtering the test list to report (eg: Vcc*)
///////////////////////////////////////////////////////////
void PickTestDialog::OnParamFilter(void)
{
    // Get Parameter name filter expression
    m_strFilterExpr = lineEditParamFilter->text();

    // Get GroupID to focus on
    int iGroupID	= getGroupID();

    // Rebuild list based on regular expression
    updateTreeWidget(iGroupID);
}

///////////////////////////////////////////////////////////
// Seelction made in test list....
///////////////////////////////////////////////////////////
void PickTestDialog::OnTestSelection(void)
{
    QObject::disconnect(lineEditTestFilter, SIGNAL(textChanged(const QString &)), this, SLOT(updatePickerFromFilter(const QString &)));
    // Get current selection made, display it in the Text# editor field
    if(m_bItemizeList)
        lineEditTestFilter->setText(testItemizedList(false));
    else
        lineEditTestFilter->setText(testList(false));
    QObject::connect(lineEditTestFilter, SIGNAL(textChanged(const QString &)), this, SLOT(updatePickerFromFilter(const QString &)));

}

///////////////////////////////////////////////////////////
// Build test list from file.
///////////////////////////////////////////////////////////
bool PickTestDialog::buildParameterList(bool bForceBuildList)
{
    CGexGroupOfFiles *	pGroup				= NULL;
    CGexFileInGroup *	pFile				= NULL;
    bool				bErrorNeedRun		= false;
    bool				bRebuildTestList	= false;

    if(gexReport == NULL)
        bErrorNeedRun = true;
    else
    {
        // First group of files
        pGroup = gexReport->getGroupsList().isEmpty()?NULL:gexReport->getGroupsList().first();	// Group#1
        if(pGroup == NULL)
            bErrorNeedRun = true;
        else
        {
            // First file in Group#1...must always exist.
            pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
            if(pFile == NULL)
            {
                // Can't create list...no data available!
                GS::Gex::Message::
                    information("", "Sorry...No test data found!");
                return false;
            }
        }
    }

    // NO Test list exists yet..or request to force building it...we must build one!
    if(bErrorNeedRun == true || bForceBuildList == true)
    {
        if(bForceBuildList == false)
        {
            bool lOk;
            GS::Gex::Message::request("", "Examinator doesn't have a Tests/Parameters list yet.\n"
                                          "Do you want it to read your data now ?", lOk);
            if (! lOk)
            {
                return false;
            }
        }

        // Flag rebuild.
        bRebuildTestList = true;
    }
    else if(pGexMainWindow->m_bDatasetChanged)
    {
        // All reports pages to be created (maybe new query entered)
        QMessageBox mb( GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
                        "Your Tests/Parameters list may be out of date.\nDo you want to use it anyway?",
                        QMessageBox::Question,
                        QMessageBox::Yes | QMessageBox::Default,
                        QMessageBox::No  | QMessageBox::Escape,
                        QMessageBox::NoAll | QMessageBox::Escape,
                        0 );
        mb.setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));
        mb.setButtonText( QMessageBox::Yes, "&Use it" );
        mb.setButtonText( QMessageBox::No, "&Rebuild it" );
        mb.setButtonText( QMessageBox::NoAll, "&Cancel" );

        switch( mb.exec())
        {
            case QMessageBox::Yes:
                pGexMainWindow->m_bDatasetChanged	= false;	// Avoids to ask again this question!
                break;	// Use list, do not rebuild it.

            case QMessageBox::NoAll:
                return false;	// User cancel.

            case QMessageBox::No:
                bRebuildTestList = true;	// User requests to rebuild test list
                break;
        }
    }

    if(bRebuildTestList == true)
    {
        int iGexAssistantSelected = pGexMainWindow->iGexAssistantSelected;
        pGexMainWindow->iGexAssistantSelected = -1;
        QString strSettingsPage = GEX_BROWSER_ACTIONLINK;
        strSettingsPage += GEX_BROWSER_SETTINGS;

        // Scan data, NO report created.
        pGexMainWindow->BuildReportNow(false,strSettingsPage,false);
        pGexMainWindow->iGexAssistantSelected = iGexAssistantSelected;
    }

    return true;
}

///////////////////////////////////////////////////////////
// build the test list if needed
// then fill the tree widget with list from given GroupID.
///////////////////////////////////////////////////////////
bool PickTestDialog::fillParameterList(int iGroup /* = -1 */, bool bForceBuildList /* = false */)
{
    // Check if we need to rebuild the parameter list
    if (buildParameterList(bForceBuildList))
    {
        if(gexReport == NULL)
            return false;

        fillPickTestItemsList(gexReport);

        updateTreeWidget(iGroup);

        return (isEmpty() == false);
    }

    return false;
}

///////////////////////////////////////////////////////////
// fill the tree widget with list from given GroupID.
///////////////////////////////////////////////////////////
void PickTestDialog::updateTreeWidget(int iGroup)
{
    QString             strRegExp(m_strFilterExpr);
    QTreeWidgetItem *   pTreeWidgetItem     = NULL;
    bool                queryAllGroups      = false;
    int                 iSeekGroup          = iGroup;

    treeWidget->clear();

    if ((iGroup < 0 || iGroup >= gexReport->getGroupsList().count()) && bAllowAllGroupsSelection)
        queryAllGroups = true;

    // Check if List to return must always be itemized or not
    m_bItemizeList = (strRegExp.trimmed() == "*") ? false : true;

    // Regular expression filter (for only filtering test names matching criteria).
    // If wildcar used, set its support.
    QRegExp	strFilter;
    strFilter.setCaseSensitivity(Qt::CaseInsensitive);
    strFilter.setPatternSyntax(mPatternSyntax);
    strFilter.setPattern(strRegExp);

    PickTestItem* pickTestItem = NULL;
    QListIterator<PickTestItem*> itPickTestItem(m_pickTestItemsList);
    while (itPickTestItem.hasNext())
    {
        pickTestItem = itPickTestItem.next();
        // Don't show it if it's not in the pointed group
        if (!queryAllGroups && !pickTestItem->isInGroup(iSeekGroup) && iSeekGroup!=-1)
            continue;

        // Check if Test name matching regular expression criteria!
        if(strFilter.exactMatch(pickTestItem->testName()))
        {
            pTreeWidgetItem = new PickTestTreeWidgetItem(treeWidget);
            if (!pickTestItem->testNumber().isEmpty())
                pTreeWidgetItem->setText(0, pickTestItem->testNumber());
            if (!pickTestItem->testName().isEmpty())
                pTreeWidgetItem->setText(1, pickTestItem->testName());
            if (!pickTestItem->testType().isEmpty())
                pTreeWidgetItem->setText(2, pickTestItem->testType());
            static_cast<PickTestTreeWidgetItem*>(pTreeWidgetItem)->mItem = pickTestItem;
        }
    }
    // Seek to the relevant group#
    if (bAllowAllGroupsSelection)
        iSeekGroup++;
    // Update Group selection combo
    comboBoxGroupName->setCurrentIndex(iSeekGroup);

    treeWidget->resizeColumnToContents(0);
    treeWidget->setColumnWidth(1, 250);
    treeWidget->resizeColumnToContents(2);
    treeWidget->resizeColumnToContents(3);
}

///////////////////////////////////////////////////////////
// Fills the GUI with the list of parameters (used for remote database).
///////////////////////////////////////////////////////////
bool PickTestDialog::fillParameterList(QStringList &strParameterList, const QStringList &pinNumber)
{
    if (strParameterList.isEmpty())
        return false;

    fillPickTestItemsList(strParameterList,pinNumber);

    updateTreeWidget(-1);

    // Hide comboBox group name to ensure not load group names of a previous selection in memory
    comboBoxGroupName->hide();

    return true;
}

///////////////////////////////////////////////////////////
// Select operating mode: single or multiple selection...
///////////////////////////////////////////////////////////
void PickTestDialog::setMultipleSelection(bool bAllowMultiSelections)
{
    QString strText;

    bAllowMultipleSelections = bAllowMultiSelections;
    if(bAllowMultipleSelections == true)
    {
        // Allow multiple selections: keep default text + behavior!
        treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
        strText = "Pick one test/parameter, or hold the 'Ctrl' key for multiple selections, ";
        strText += "or hold the 'Shift' key for a range selection.";
    }
    else
    {
        // Single selection mode...set text + Widget flags
        treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        strText = "Pick ONE test from the list...";
    }

    // Update text
    TextLabel->setText(strText);
}

///////////////////////////////////////////////////////////
// Select operating mode: Allow user to pick from any group,
// or only 1st group. Default is only 1st group.
///////////////////////////////////////////////////////////
void PickTestDialog::setMultipleGroups(bool bAllowMultiGroups, bool bAllowAllGroups)
{
    bAllowMultipleGroups = bAllowMultiGroups;
    bAllowAllGroupsSelection = bAllowAllGroups;	// User can select "All groups"
    if(bAllowMultipleGroups == true)
    {
        // Allow to select from any group (data set)
        TextLabelGroupName->show();
        comboBoxGroupName->show();

        // Loads Combo box with list of group names.
        EnumerateGroupNames();

        // If we only have one group, then hide the list box any way as it only holds one group name!
        if(gexReport->getGroupsList().count() <= 1)
        {
            TextLabelGroupName->hide();
            comboBoxGroupName->hide();
        }
    }
    else
    {
        // Only select from 1st group (default)
        TextLabelGroupName->hide();
        comboBoxGroupName->hide();
    }
}

void PickTestDialog::setAllowedTestType(PickTestDialog::TestType eTestType)
{
    m_eTestTypeAllowed = eTestType;
    /*
    if (bParametric)
        m_eTestTypeAllowed |= TestParametric;
    else
        m_eTestTypeAllowed &= ~(TestParametric);

    if (bMultiParametric)
        m_eTestTypeAllowed |= TestMultiParametric;
    else
        m_eTestTypeAllowed &= ~(TestMultiParametric);

    if (bFunctional)
        m_eTestTypeAllowed |= TestFunctional;
    else
        m_eTestTypeAllowed &= ~(TestFunctional);
    */
}

void PickTestDialog::setBlockCompressionEnabled(bool enable)
{
    mCompressBlocks = enable;
}

///////////////////////////////////////////////////////////
// Tells if the list of test is empty or not...
///////////////////////////////////////////////////////////
bool PickTestDialog::isEmpty(void)
{
    if(treeWidget->topLevelItemCount() <= 0)
        return true;	// Empty list
    else
        return false;	// List initialized, tests in list.
}

// Select all items defined in custom list
///////////////////////////////////////////////////////////
// Makes ListView selection mathcing the custom 'test#' edit box content...
///////////////////////////////////////////////////////////
bool PickTestDialog::SetSelectionToCustomList(void)
{
    bool bItemSelected = false;

    // Clear all selections.
    treeWidget->clearSelection();

    // Pointer to first selection (if any)
    QTreeWidgetItem * pTreeWidgetItem = treeWidget->topLevelItem(0);

    if(pTreeWidgetItem == NULL)
        return bItemSelected;

    // 'Test#' custom list...
    QString strString = lineEditTestFilter->text();
    if(strString.isEmpty())
        return bItemSelected;

    GS::QtLib::Range cTestList(strString.toLatin1().constData());
    long		lTestNumber;

    // Scan the list and compute the test list (if any selection made)
    while(pTreeWidgetItem != NULL)
    {
        // Get test#
        lTestNumber = pTreeWidgetItem->text(0).toLong();

        // If this test belong to our list, make it selected.
        if(cTestList.Contains(lTestNumber))
        {
            pTreeWidgetItem->setSelected(true);
            bItemSelected = true;
        }

        // Move to next item.
        pTreeWidgetItem = treeWidget->itemBelow(pTreeWidgetItem);
    };

    return bItemSelected;
}

void PickTestDialog::clearPickTestItemsList()
{
    while (!m_pickTestItemsList.isEmpty())
        delete m_pickTestItemsList.takeFirst();
}

void PickTestDialog::fillPickTestItemsList(QStringList &parameterList, const QStringList &pinNumber)
{
    PickTestItem *newItem = NULL;
    clearPickTestItemsList();

    // Fill list
    QStringList::Iterator it;
    QStringList::const_iterator itPin = pinNumber.constBegin();
    for(it = parameterList.begin(); it != parameterList.end(); ++it )
    {
        newItem = new PickTestItem();
        // Get Test#
        newItem->setTestNumber(*it);
        // Get Test name
        it++;
        newItem->setTestName(*it);
        // Get Test type
        it++;
        newItem->setTestType(*it);
        if(!pinNumber.isEmpty() && itPin != pinNumber.constEnd())
        {
            newItem->setPinName(*itPin);
            ++itPin;
        }
        // Add item to list
        m_pickTestItemsList.append(newItem);
    }
}


void PickTestDialog::fillPickTestItemsList(CGexReport *ptGexReport)
{
    // Read ALL tests found in Group# 'iGroup'
    if(ptGexReport == NULL)
        return;

    CGexGroupOfFiles *      pGroup          = NULL;
    CGexFileInGroup *       pFile           = NULL;
    CTest *                 ptTestCell      = NULL;	// Pointer to test cell
    CTest *                 ptTestCellTmp   = NULL;	// Pointer to test cell
    PickTestItem *          lPickTestItem    = NULL;
    QList<CGexGroupOfFiles *> groupsList    = ptGexReport->getGroupsList();
    QString                 lPinName;
    QString                 lTestName;
    bool                    bValidTest;

    clearPickTestItemsList();
    // First group of files
    pGroup = groupsList.isEmpty() ? NULL : groupsList.first();	// Group#1
    if(pGroup == NULL)
        return;
    // Get option about generix test created by galaxy
    QString		strOptionStorageDevice = (ReportOptions.GetOption("statistics","generic_galaxy_tests")).toString();
    QList<BYTE>	lstTestTypeAllowed;

    if (m_eTestTypeAllowed & TestParametric)
        lstTestTypeAllowed << 'P';
    if (m_eTestTypeAllowed & TestMultiParametric)
        lstTestTypeAllowed << 'M';
    if (m_eTestTypeAllowed & TestFunctional)
        lstTestTypeAllowed << 'F';
    if ((m_eTestTypeAllowed & TestGenericGalaxy) && strOptionStorageDevice == "show")
        lstTestTypeAllowed << '-';

    // Read all tests in the group.
    ptTestCell = pGroup->cMergedData.ptMergedTestList;
    QHash<QString, PickTestItem *> lExistingTest;
    while(ptTestCell != NULL)
    {
        bValidTest = false;

        // hide filtered test
        if(m_bUseInteractiveFilters && gexReport->isInteractiveTestFiltered(ptTestCell) == false)
            goto NextTestCell;

        for (int nGroup = 0; nGroup < groupsList.count(); nGroup++)
        {
            pGroup	= groupsList.at(nGroup);
            pFile	= (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

            if(pFile->FindTestCell(ptTestCell->lTestNumber,
                                   ptTestCell->lPinmapIndex,
                                   &ptTestCellTmp,
                                   true,
                                   false,
                                   ptTestCell->strTestName.toLatin1().data()) == 1)
            {
                // GCORE-6006: Tests missing in " pick tests from list " window after performing a what-if analysis
                // Only list
//                if((ptTestCellTmp->ldExecs == 0 && ptTestCellTmp->GetCurrentLimitItem()->ldOutliers == 0))
//                    continue;

                // IF Muti-result parametric test, do not show master test record
                if(ptTestCellTmp->lResultArraySize > 0)
                    continue;

                // Do not display functional test in list, only Parametric tests
                if(lstTestTypeAllowed.contains(ptTestCellTmp->bTestType) == false)
                    continue;

                // if not already done
                if (!bValidTest)
                    // Insert test# + test name
                    ptGexReport->BuildPinNameString(pFile,ptTestCellTmp,lPinName);

                // Pick the correct test name
                if (ptTestCellTmp->GetUserTestName().isEmpty())
                {
                    lTestName = ptTestCellTmp->strTestName;
                }
                else
                {
                    lTestName = ptTestCellTmp->GetUserTestName();
                }

                // Try to retrieve matching PickTestItem from list
                QString lKeyString = ptTestCellTmp->szTestLabel + lTestName + lPinName;
                lPickTestItem = NULL;
                QHash<QString, PickTestItem *>::const_iterator lIterator = lExistingTest.find(lKeyString);
                if (lIterator != lExistingTest.end())
                    lPickTestItem = lIterator.value();
                /*lPickTestItem = pickTestItemFromList(ptTestCellTmp->szTestLabel,
                                                    lTestName,
                                                    lPinName,
                                                    ptTestCell->bTestType);*/
                // If not already in list create it!
                if (!lPickTestItem)
                {
                    lPickTestItem = new PickTestItem();
                    lPickTestItem->setTestNumber(ptTestCellTmp->szTestLabel);
                    lPickTestItem->setTestName(lTestName);

                    mNameAndFormatedName.insert(ptTestCellTmp->GetUserTestName(), ptTestCellTmp->strTestName);

                    if(ptTestCell->bTestType == 'M')
                    {
                        lPickTestItem->setTestType("MP");
                        lPickTestItem->setPinName(lPinName);
                    }
                    else if(ptTestCell->bTestType == 'I')
                    {
                        lPickTestItem->setTestType("Inv");
                    }
                    else if(ptTestCell->bTestType == ' ')
                    {
                        lPickTestItem->setTestType("Unk");
                    }
                    else
                    {
                        lPickTestItem->setTestType(QChar(ptTestCellTmp->bTestType));
                    }
                    lPickTestItem->setGroupIndex(nGroup);
                    lPickTestItem->setFileIndex(0);
                    lPickTestItem->setPinName(QString::number(ptTestCell->lPinmapIndex));
                    m_pickTestItemsList.append(lPickTestItem);

                    // Add Test item to hash table
                    lExistingTest.insert(lKeyString, lPickTestItem);
                }
                lPickTestItem->addGroupId(nGroup);
                // Data samples found
                bValidTest = true;
            }
        }

        // No data samples found
        if (bValidTest == false)
            goto NextTestCell;

NextTestCell:
        // Point to next test cell
        ptTestCell = ptTestCell->GetNextTest();
    };	// Loop until all test cells read.
}

/*PickTestItem* PickTestDialog::pickTestItemFromList(const QString& testNumber,
                                                   const QString& testName,
                                                   const QString& pinName,
                                                   char testType)
{
    PickTestItem*   lPickTestItem = NULL;

    QListIterator<PickTestItem*> itPickTestItem(m_pickTestItemsList);
    while (itPickTestItem.hasNext())
    {
        lPickTestItem = itPickTestItem.next();
        if (testNumber == lPickTestItem->testNumber() &&
            testName == lPickTestItem->testName() )
        {
            if (testType != 'M')
                return lPickTestItem;
            else if (pinName == lPickTestItem->pinName())
                return lPickTestItem;
        }
    }

    return NULL;
}
*/
///////////////////////////////////////////////////////////
// Returns the list of tests selected
// format accepst concatenation. eg: "1-25,7,60-90"
///////////////////////////////////////////////////////////
QString PickTestDialog::testList(bool bCheckCustomList/*=true*/)
{
    QString strTestList="";
    QString	strFirstTest,strLastTest;
    QString	strFirstTestName,strLastTestName;
    bool	bTestBlocFound=false;
    // If test list to return MUST be itemized, then do it! (this is the case when the listView shows filtered tests).
    if(m_bItemizeList)
    {
        return testItemizedList(bCheckCustomList);
    }

    // Pointer to first selection (if any)
    QTreeWidgetItem * pTreeWidgetItem = treeWidget->topLevelItem(0);

    // Return empty string if no tests selected
    if(pTreeWidgetItem == NULL)
        return strTestList;

    // Scan the list and compute the test list (if any selection made)
    while(pTreeWidgetItem != NULL)
    {
        QStringList lTestsBlock;
        // Prepare to detect next bloc.
        strFirstTest = strLastTest = pTreeWidgetItem->text(0);
        strFirstTestName = strLastTestName = pTreeWidgetItem->text(1);
        // For each group of selected tests
        while(pTreeWidgetItem && pTreeWidgetItem->isSelected() == true)
        {
            // Flag that we have found a bloc of selections
            bTestBlocFound = true;
            // Update end of bloc marker
            strLastTest = pTreeWidgetItem->text(0);
            strLastTestName = pTreeWidgetItem->text(1);
            if (strLastTest != strFirstTest)
                lTestsBlock.append(strLastTest);
            // Move to next item.
            pTreeWidgetItem = treeWidget->itemBelow(pTreeWidgetItem);
            if( treeWidget->sortColumn() != 0)
                break;
        };

        // check if we have to save a bloc just found
        if(bTestBlocFound == true )
        {
            // Save bloc data
            if(strTestList.isEmpty() == false)
                strTestList += ",";	// Bloc separator

            // Update bloc list to include the bloc just detected
            strTestList += strFirstTest;

            // If bloc is a single test, no need to create a X-X string!
            bool singleTest = (strFirstTest == strLastTest) && (strFirstTestName == strLastTestName);

            if(!singleTest && treeWidget->sortColumn() == 0 )
            {
                if (mCompressBlocks)
                {
                    strTestList += "-";
                    strTestList += strLastTest;
                }
                else
                {
                    lTestsBlock.removeDuplicates();
                    if (!lTestsBlock.isEmpty())
                        strTestList += "," + lTestsBlock.join(",");
                }
            }

            // Reset flag.
            bTestBlocFound = false;
        }
        else
        {
            // Move to next item...and update test bloc markers.
            pTreeWidgetItem = treeWidget->itemBelow(pTreeWidgetItem);
        }
    };

    // If we have to check the 'test#' edit field, then we give priority to its content.
    if(bCheckCustomList)
    {
        QString strCustomTestList = lineEditTestFilter->text();
        if(strTestList != strCustomTestList)
        {
            strTestList = strCustomTestList;
            QObject::disconnect(treeWidget, SIGNAL(itemSelectionChanged()),
                                this, SLOT(OnTestSelection()));
            // Select all items defined in custom list
            if (SetSelectionToCustomList() == false)
                strTestList.clear();
            QObject::connect(treeWidget, SIGNAL(itemSelectionChanged()),
                                this, SLOT(OnTestSelection()));
        }
    }

    // Return the list of tests selected
    return strTestList;
}


QList<PickTestItem*> PickTestDialog::testSelected()
{
    QList<PickTestItem*> lTests;
    QTreeWidgetItem * pTreeWidgetItem = treeWidget->topLevelItem(0);
    while(pTreeWidgetItem != NULL)
    {
        if(pTreeWidgetItem->isSelected() == true)
        {
            lTests.append(static_cast<PickTestTreeWidgetItem*>(pTreeWidgetItem)->mItem);
        }
        pTreeWidgetItem = treeWidget->itemBelow(pTreeWidgetItem);
    }
    return lTests;
}

/*QList<const PickTestItem*> PickTestDialog::testSelected()
{
    QList<PickTestItem> lTests;
    QTreeWidgetItem * pTreeWidgetItem = treeWidget->topLevelItem(0);
    while(pTreeWidgetItem != NULL)
    {
        PickTestItem lItem;
        // For each group of selected tests
        if(pTreeWidgetItem->isSelected() == true)
        {
            lItem.setTestNumber ( pTreeWidgetItem->text(0));
            lItem.setTestName   ( pTreeWidgetItem->text(1));
            lItem.setTestType   ( pTreeWidgetItem->text(2));
            lItem.setPinName    ( pTreeWidgetItem->text(3));
            lTests.append(lTests);
        }

        // Move to next item...and update test bloc markers.
        pTreeWidgetItem = treeWidget->itemBelow(pTreeWidgetItem);
    }

    return lTests;

}*/

///////////////////////////////////////////////////////////
// Returns the itemized list of tests selected
// eg: 1,2,7,9  (lists ALL selections made, no concatenation
// like 1-6)
///////////////////////////////////////////////////////////
QString PickTestDialog::testItemizedList(bool bCheckCustomList/*=true*/)
{
    QString strTestList="";
    QString strCustomTestList = lineEditTestFilter->text();

    // Pointer to first selection (if any)
    QTreeWidgetItem * pTreeWidgetItem = treeWidget->topLevelItem(0);

    // Return empty string if no tests selected
    if(pTreeWidgetItem == NULL)
    {
        if(bCheckCustomList)
            strTestList = strCustomTestList;
        return strTestList;
    }

    // Scan the list and compute the test list (if any selection made)
    while(pTreeWidgetItem != NULL)
    {
        // For each group of selected tests
        if(pTreeWidgetItem->isSelected() == true)
        {
            // Save bloc data
            if(strTestList.isEmpty() == false)
                strTestList += ",";	// Bloc separator

            // Update bloc list to include the bloc just detected
            strTestList += pTreeWidgetItem->text(0);
        }

        // Move to next item...and update test bloc markers.
        pTreeWidgetItem = treeWidget->itemBelow(pTreeWidgetItem);
    };

    // If we have to check the 'test#' edit field, then we give priority to its content.
    if(bCheckCustomList)
    {
        // Ony consider cutom 'test#' edit box if it itemized (no '-' character)
        if(m_bItemizeList || ((m_bItemizeList == false) && (strCustomTestList.indexOf('-') < 0)))
        {
            if(strTestList != strCustomTestList)
            {
                // Custom list takes precedence
                strTestList = strCustomTestList;

                // Select all items defined in custom list
                if (SetSelectionToCustomList() == false)
                    strTestList.clear();
            }
        }
    }

    // Return the list of tests selected
    return strTestList;
}

///////////////////////////////////////////////////////////
// Returns the itemized list of tests NAMES selected
// eg: "Vcc Idqq" "ABC Ref" ....
///////////////////////////////////////////////////////////
QStringList PickTestDialog::testItemizedListNames(void)
{
    QStringList strTestListNames;

    // Clear list
    strTestListNames.clear();

    // Pointer to first selection (if any)
    QTreeWidgetItem * pTreeWidgetItem = treeWidget->topLevelItem(0);

    // Return empty string if no tests selected
    if(pTreeWidgetItem == NULL)
        return strTestListNames;

    // Scan the list and compute the test list (if any selection made)
    while(pTreeWidgetItem != NULL)
    {
        // For each Parameter selected, save its name.
        if(pTreeWidgetItem->isSelected() == true)
            strTestListNames += pTreeWidgetItem->text(1);

        // Move to next item...and update test bloc markers.
        pTreeWidgetItem = treeWidget->itemBelow(pTreeWidgetItem);
    };

    // Return the list of tests NAMES selected
    return strTestListNames;
}

QString PickTestDialog::GetOriginalTestName(const QString formatedName) const
{
    return mNameAndFormatedName.value(formatedName, formatedName);
}

void PickTestDialog::setUseInteractiveFilters(bool bUseFilters)
{
    m_bUseInteractiveFilters = bUseFilters;
}

///////////////////////////////////////////////////////////////////////////////////
// Class PickTestTreeWidgetItem - class which represents an pick test item
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
PickTestTreeWidgetItem::PickTestTreeWidgetItem(QTreeWidget * pTreeWidget)
    : QTreeWidgetItem(pTreeWidget)
{
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
PickTestTreeWidgetItem::~PickTestTreeWidgetItem()
{
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool operator< ( const QTreeWidgetItem & other ) const
//
// Description	:	Override the comparison methods
//
///////////////////////////////////////////////////////////////////////////////////
bool PickTestTreeWidgetItem::operator< ( const QTreeWidgetItem & other ) const
{
    int nCol = treeWidget() ? treeWidget()->sortColumn() : 0;

    QString strLeft		= text(nCol);
    QString strRight	= other.text(nCol);

    if((nCol == 1) || (nCol == 2) || (nCol == 3))
    {
        // Sorting on name or type...it's strings!
        return (strLeft.compare(strRight, Qt::CaseInsensitive) < 0);
    }

    // Extract Testnumber & pinmap from string
    int		iIndex;
    long	lTestNumberLeft;
    long	lTestNumberRight;
    int		iPinmapIndexLeft;
    int		iPinmapIndexRight;

    iIndex = strLeft.indexOf('.');
    if(iIndex < 0)
    {
        // No pinmap,
        iPinmapIndexLeft	= GEX_PTEST;	// -1
        lTestNumberLeft		= strLeft.toLong();
    }
    else
    {
        // Pinmap exist
        QString strPinmap	= strLeft.mid(iIndex+1);
        iPinmapIndexLeft	= strPinmap.toInt();
        strLeft.truncate(iIndex);
        lTestNumberLeft		= strLeft.toLong();
    }

    // Extract Testnumber & pinmap from string
    iIndex = strRight.indexOf('.');
    if(iIndex < 0)
    {
        // No pinmap,
        iPinmapIndexRight	= GEX_PTEST;	// -1
        lTestNumberRight	= strRight.toLong();
    }
    else
    {
        // Pinmap exist
        QString strPinmap	= strRight.mid(iIndex+1);
        iPinmapIndexRight	= strPinmap.toInt();
        strRight.truncate(iIndex);
        lTestNumberRight	= strRight.toLong();
    }

    // Check the two test# to eachother
    if(lTestNumberLeft < lTestNumberRight)
        return true;
    if(lTestNumberLeft > lTestNumberRight)
        return false;

    // Test number are identical...check Pinmap#
    if(iPinmapIndexLeft < iPinmapIndexRight)
        return true;
    if(iPinmapIndexLeft > iPinmapIndexRight)
        return false;

    return false;
}

void PickTestDialog::updatePickerFromFilter(const QString &){

    //qDebug()<<"lineEditTestFilter : "<< lineEditTestFilter->text();
    if( lineEditTestFilter->text().isEmpty())
        return ;
    QObject::disconnect(treeWidget, SIGNAL(itemSelectionChanged()),
                        this, SLOT(OnTestSelection()));
    GS::QtLib::Range cTestList(lineEditTestFilter->text().toLatin1().constData());
    QTreeWidgetItem * pTreeWidgetItem = treeWidget->topLevelItem(0);
    while(pTreeWidgetItem != NULL)
    {
        long lTestNumber = pTreeWidgetItem->text(0).toLong();
        if(cTestList.Contains(lTestNumber))
        {
            pTreeWidgetItem->setSelected(true);
        }
        else
        {
            pTreeWidgetItem->setSelected(false);
        }

        // Move to next item...and update test bloc markers.
        pTreeWidgetItem = treeWidget->itemBelow(pTreeWidgetItem);
    };
    QObject::connect(treeWidget, SIGNAL(itemSelectionChanged()),
                     this, SLOT(OnTestSelection()));
}
