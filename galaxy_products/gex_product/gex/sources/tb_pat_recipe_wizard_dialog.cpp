#include <qprogressdialog.h>
#include <qmenu.h>

#include "tb_pat_recipe_wizard_dialog.h"
#include "browser_dialog.h"
#include "engine.h"
#include "report_build.h"
#include "filter_dialog.h"
#include "script_wizard.h"
#include "scripting_io.h"
#include "pat_info.h"
#include "patman_lib.h"
#include "temporary_files_manager.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include <gqtl_log.h>
#include "gex_version.h"
#include "csl/csl_engine.h"
#include "message.h"

// For MemoryLeak detection: ALWAYS have to be LAST #include in file
#include "DebugMemory.h"

// main.cpp
extern GexMainwindow *	pGexMainWindow;

// script_wizard.h
extern void ConvertToScriptString(QString &strFile);

// report_build.cpp
extern CGexReport *		gexReport;				// Handle to report class

//////////////////////////////////////////////////////////
// Constructor
PatRecipeWizardDialog::PatRecipeWizardDialog(QWidget* parent, bool modal, Qt::WindowFlags f )
    : QDialog( parent, f )
{
    setupUi(this);
    setModal(modal);

    // Apply Examinator palette
    GexMainwindow::applyPalette(this);

    QObject::connect(pushButtonOk,				SIGNAL(clicked()),	this, SLOT(accept()));
    QObject::connect(pushButtonCancel,			SIGNAL(clicked()),	this, SLOT(reject()));

    // Make GUI slots connections
    connect((QObject *)pushButtonBack,			SIGNAL(clicked()),	this,SLOT(OnButtonBack(void)));
    connect((QObject *)buttonTestsToEnable,		SIGNAL(clicked()),	this,SLOT(OnEnableTests(void)));
    connect((QObject *)buttonTestsToDisable,	SIGNAL(clicked()),	this,SLOT(OnDisableTests(void)));
    connect(listViewDisabledTests,	SIGNAL( customContextMenuRequested(const QPoint&) ), this, SLOT( contextMenuTestsToEnable(const QPoint&) ) );
    connect(listViewEnabledTests,	SIGNAL( customContextMenuRequested(const QPoint&) ), this, SLOT( contextMenuTestsToDisable(const QPoint&) ) );

    TextLabelTitle->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    // GUI fields
    setWindowTitle("Recipe Wizard: Tests under PAT (Page 2)");
}

//////////////////////////////////////////////////////////
// Tells if parametric tests detected in the file
//////////////////////////////////////////////////////////
bool PatRecipeWizardDialog::hasParametricTests()
{
    if((listViewEnabledTests->topLevelItemCount() >= 0) || (listViewDisabledTests->topLevelItemCount() >= 0))
        return true;	// We have some tests!
    else
        return false;	// Absolutely no parametric test in this file! (probably a wafermap file!).
}

//////////////////////////////////////////////////////////
// Set data file to read and analyse
//////////////////////////////////////////////////////////
bool PatRecipeWizardDialog::setFile(QString strFile,
                                    const COptionsPat &patOptions,
                                    QString strFilterType/*="all"*/,
                                    QString strFilterList/*=""*/)
{
    QString strErrorMsg;

    // Reset wizard GUI
    listViewDisabledTests->clear();
    listViewEnabledTests->clear();

    // If not a STDF file (eg.: ATDF, GDF, CSV,...)...convert it to STDF!
    QString                 strStdfFile;
    GS::Gex::ConvertToSTDF  StdfConvert;
    QString                 strErrorMessage;
    bool                    bFileCreated;

    // 3rd parameter (bDatabaseAccessMode) is set to true, so that uncomplete files get rejected, like with database insertion
    int nConvertStatus = StdfConvert.Convert(false,true,true,false,strFile, strStdfFile,GEX_TEMPORARY_STDF,bFileCreated,strErrorMessage);
    // Failed converting to STDF file
    if(nConvertStatus == GS::Gex::ConvertToSTDF::eConvertError)
    {
        // Set short error message
        GS::Gex::Message::critical("", strErrorMessage);
        return false;
    }

    // Update list of temporary STDF files created if needed.
    if(bFileCreated == true)
        GS::Gex::Engine::GetInstance().GetTempFilesManager().addFile(strStdfFile, TemporaryFile::BasicCheck);
    else
        strStdfFile = strFile;	// Already STDF file!


    ///////////////////////////////////////////////////////////////////
    // Get list of sites in the data file...
    // Show selection made...
    FilterDialog cFilter;
    if(cFilter.GetValidSites(strStdfFile) == false)
    {
        // Failed reading the data file...
        strErrorMsg = "Failed reading test data file: corrupted or unknown format.";
        GS::Gex::Message::critical("", strErrorMsg);
        return false;
    }

    // Create script that will read data file + compute all statistics (but NO report created)
    QString strScriptFile = GS::Gex::Engine::GetInstance().GetAssistantScript();
    FILE *hFile = fopen(strScriptFile.toLatin1().constData(),"w");

    if(hFile == NULL)
    {
        // Set short error message
        strErrorMsg = "Failed to create script file: ";
        strErrorMsg += strScriptFile;
        GS::Gex::Message::critical("", strErrorMsg);
        return false;
    }

    // Creates 'SetOptions' section
    if(ReportOptions.WriteOptionSectionToFile(hFile) == false)
    {
        GEX_ASSERT(false);
        strErrorMessage = QString("Error : can't write option section");
        return false;
    }

    // Make STDF file name CSL compatible
    QString	strStdfFile_CslCompatible = strStdfFile;
    ConvertToScriptString(strStdfFile_CslCompatible);	// Make sure any '\' in string is doubled.

    // Creates 'SetProcessData' section
    fprintf(hFile,"SetProcessData()\n");
    fprintf(hFile,"{\n");
    fprintf(hFile,"  var group_id;\n");
    fprintf(hFile,"  gexGroup('reset','all');\n");

    // If too many sites, do not compare them, too time consuming!
    // Wizard will only check for abnormal merged distributions.
    if(cFilter.mSitesList.count() > 16)
    {
        cFilter.mSitesList.clear();
        cFilter.mSitesList << -1;
    }

    // Create as many groups as sites found in file...
    QString		strSiteNumber;
    QList <int> ::iterator it;
    int	iSiteID=0;
    for ( it = cFilter.mSitesList.begin(); it != cFilter.mSitesList.end(); ++it )
    {
        // Keep track of group#
        iSiteID++;

        // Get Site#
        if(cFilter.mSitesList.count() > 1)
            strSiteNumber = QString::number(*it);
        else
            strSiteNumber = "All";

        // Create group + define group name.
        fprintf(hFile,"  group_id = gexGroup('insert','Group %d');\n",iSiteID);

        // Insert file in group...filtering on parts if requested.
        fprintf(hFile,"  gexFile(group_id,'insert','%s','%s','%s','%s','');\n",
                strStdfFile_CslCompatible.toLatin1().constData(),
                strSiteNumber.toLatin1().constData(),
                strFilterType.toLatin1().constData(),
                strFilterList.toLatin1().constData());

    }
    fprintf(hFile,"}\n\n");

    // Creates 'main' section
    fprintf(hFile,"main()\n");
    fprintf(hFile,"{\n");
    fprintf(hFile,"  SetOptions();\n");

    // Ensure continue on fail
    fprintf(hFile,"  gexOptions('dataprocessing', 'fail_count', 'all');\n");
    // Default: keep test values as is (no scaling)
    fprintf(hFile,"  gexOptions('dataprocessing', 'scaling', 'normalized');\n");
    // Ensure we load samples + compute all stats & advanced stats.
    fprintf(hFile,"  gexOptions('speed','adv_stats','always');\n");
    // Ensure we rely on samples
    fprintf(hFile,"  gexOptions('statistics','computation','samples_only');\n");

    // Specific Final Test options
    if(patOptions.GetRecipeType() == GS::Gex::PAT::RecipeFinalTest)
    {
        // MPR split always forced at Final Test level
        fprintf(hFile,"  gexOptions('dataprocessing','multi_parametric_merge_mode','no_merge');\n");
    }

    // Choose test merge option based on recipe test key option
    switch (patOptions.mTestKey)
    {
        case GEX_TBPAT_KEY_TESTNUMBER:
            // Merge tests with same test number (even if test name is different)
            fprintf(hFile,"  gexOptions('dataprocessing', 'duplicate_test', 'merge');\n");
            break;

        case GEX_TBPAT_KEY_TESTNAME:
            // Merge tests with same test name (even if test number is different)
            fprintf(hFile,"  gexOptions('dataprocessing', 'duplicate_test', 'merge_name');\n");
            break;

        case GEX_TBPAT_KEY_TESTMIX:
            // Merge tests with same test number and test name
            fprintf(hFile,"  gexOptions('dataprocessing', 'duplicate_test', 'no_merge');\n");
            break;

        default:
            // Use default option from options tab
            break;
    }

    // Avoids report creation
    fprintf(hFile,"  gexOptions('output','format','interactive');\n");
    fprintf(hFile,"  SetProcessData();\n");
    // Only data analysis, no report created!
    fprintf(hFile,"  gexOptions('report','build','false');\n");

    // Show 'Database Admin' page after processing file.
    fprintf(hFile,"  gexBuildReport('none','0');\n");
    fprintf(hFile,"}\n\n");
    fclose(hFile);

    // Display progress dialog so user waits....
    QProgressDialog cProgressDialog("Identifying tests 'PAT-Ready'...","Cancel",0,1,this);

    // Cancel button will
    if (pGexMainWindow)
        connect(&cProgressDialog, SIGNAL(canceled()), pGexMainWindow, SLOT(OnStop()));
    cProgressDialog.show();

    // Execute script.
    if (GS::Gex::CSLEngine::GetInstance().RunScript(strScriptFile).IsFailed())
    {
        strErrorMsg = "Failed analyzing test data file...";
        GS::Gex::Message::critical("", strErrorMsg);
        return false;
    }

    // Identify tests matching or failing PAT-ready criteria...
    CGexGroupOfFiles	*	pGroup		= NULL;
    CGexFileInGroup		*	pFile		= NULL;
    CTest				*	ptTestCell	= NULL;
    PatRecipeListViewItem*	ptItem		= NULL;
    QString					strText,strWizardAdvice;
    QList <QString>			cTestsToKeep;		// Holds test# suggested to keep for PAT
    QList <QString>			cTestsToDisable;	// Holds test# suggested to Disabled from PAT
    QString                 lTestKey;
    int						iShape;

    for (QList<CGexGroupOfFiles*>::ConstIterator
         iter  = gexReport->getGroupsList().begin();
         iter != gexReport->getGroupsList().end(); ++iter) {
        // Get handle to dataset in each group
        pGroup = *iter;
        if(pGroup != NULL)
            pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
        if((pGroup == NULL) || (pFile == NULL))
        {
            strErrorMsg = "Data file doesn't include any data...use other files!";
            GS::Gex::Message::critical("", strErrorMsg);
            return false;
        }

        // Scan all tests in dataset, identofy PAT-ready ones.
        ptTestCell = pFile->ptTestList;
        while(ptTestCell)
        {
            // Generate a identification key for the current test based on recipe test key option
            lTestKey = ptTestCell->generateTestKey(patOptions.mTestKey);

            // If no PTR values, this test was not executed !
            // IF Muti-result parametric test, do not show master test record
            if(ptTestCell->lResultArraySize > 0)
                goto NextTestCell;

            // If not a parametric / multiparametric (eg: functional) test, ignore!
            if(ptTestCell->bTestType == 'F')
                goto NextTestCell;

            // If custom Galaxy parameter (die loc, bin#, et), simply skip test!
            if(ptTestCell->bTestType == '-')
                goto NextTestCell;

            // Check all optential problems for this test...
            if((ptTestCell->ldExecs <= 0) || (ptTestCell->ldSamplesValidExecs <= 0))
            {
                // Texts to display
                if(gexReport->getGroupsList().count() == 1)
                    strText.sprintf("Test NEVER executed in this file!");
                else
                    strText.sprintf("Test NEVER executed on at least one site!");
                strWizardAdvice = "Disable";

                // Add entry to 'Disable' listView
                goto DisableTest;
            }

            // All samples equal only one value (categories)?
            if(ptTestCell->lDifferentValues == 0)
            {
                // Texts to display
                strText = "All values for this test are identical (Zero-range) and equal to: " ;
                strText += QString::number(ptTestCell->lfSamplesMin);
                strText += ptTestCell->szTestUnits;
                strWizardAdvice = "Disable";

                // Add entry to 'Disable' listView
                goto DisableTest;
            }

            // Only integer values?
            if(ptTestCell->bIntegerValues && patOptions.bAssumeIntegerCategory)
            {
                // Texts to display
                strText.sprintf("Category distribution exclusively made of INTEGER values (%ld different integer values over the %d samples)",ptTestCell->lDifferentValues,ptTestCell->ldSamplesValidExecs);
                strWizardAdvice = "Disable";

                // Add entry to 'Disable' listView
                goto DisableTest;
            }

            // Extract distribution shape.
            iShape = patlib_GetDistributionType(ptTestCell,
                                                patOptions.iCategoryValueCount,
                                                patOptions.bAssumeIntegerCategory,
                                                patOptions.mMinConfThreshold);
            // Only very few values
            if(iShape == PATMAN_LIB_SHAPE_CATEGORY)
            {
                // Texts to display
                strText.sprintf("Category distribution with only few different values (Only %ld different values for all %d samples)",ptTestCell->lDifferentValues,ptTestCell->ldSamplesValidExecs);
                strWizardAdvice = "Disable";

                // Add entry to 'Disable' listView
                goto DisableTest;
            }

            if(ptTestCell->lDifferentValues <= patOptions.iCategoryValueCount)
            {
                GS::Gex::Message::information(
                    "", "Category because <= " +
                    QString::number(patOptions.iCategoryValueCount));
                // Texts to display
                strText.sprintf("Category-like distribution with only few different values (Only %ld different values for all %d samples)",ptTestCell->lDifferentValues,ptTestCell->ldSamplesValidExecs);
                strWizardAdvice = "Disable";

                // Add entry to 'Disable' listView
                goto DisableTest;
            }

            ///////////////////////////////////////////////////////////
            // If reaching this point, then this test is fine
            // and can be added to the 'Keep Test' list...
            // ...unless this test was disabled in another site#
            if(cTestsToDisable.indexOf(lTestKey) != -1)
                goto NextTestCell; // Test already disabled from another site analysis...so just move to next test.

            // If test already listed as enabled (eg: from previous site analyzed), drop it!
            if(cTestsToKeep.indexOf(lTestKey) != -1)
                goto NextTestCell;

            // Test really okay, so suggest to keep it + keep track of test number
            strWizardAdvice = "PAT-Ready";
            strText = "";
            cTestsToKeep.append(lTestKey);

            ptItem = new PatRecipeListViewItem(listViewEnabledTests, lTestKey);

            ptItem->setText(0,QString::number(ptTestCell->lTestNumber));
            ptItem->setText(1, ptTestCell->strTestName);
            ptItem->setText(2, QString(QChar(ptTestCell->bTestType)));
            ptItem->setText(3, ptTestCell->GetCurrentLimitItem()->szLowL);
            ptItem->setText(4, ptTestCell->GetCurrentLimitItem()->szHighL);
            ptItem->setText(5, strWizardAdvice);
            ptItem->setText(6, strText);


            // Move to next test
            goto NextTestCell;

            // Test fails one of the criteria; suggest to Disable from PAT.
DisableTest:
            if(cTestsToDisable.indexOf(lTestKey) != -1)
                goto NextTestCell; // Test already disabled from another site analysis...so just move to next test.

            // Add entry to 'Disable' listView
            cTestsToDisable.append(lTestKey);
            ptItem = new PatRecipeListViewItem(listViewDisabledTests, lTestKey);
            ptItem->setText(0, QString::number(ptTestCell->lTestNumber));
            ptItem->setText(1, ptTestCell->strTestName);
            ptItem->setText(2, QString(QChar(ptTestCell->bTestType)));
            ptItem->setText(3, ptTestCell->GetCurrentLimitItem()->szLowL);
            ptItem->setText(4, ptTestCell->GetCurrentLimitItem()->szHighL);
            ptItem->setText(5, strWizardAdvice);
            ptItem->setText(6, strText);

            // If this test was also in the 'Keep test', ensure to remove it from it!
            if(cTestsToKeep.indexOf(lTestKey) != -1)
            {
                // Remove from list
                cTestsToKeep.removeAll(lTestKey);

                // Ensure to remove relevant line from GUI ListView.
                ptItem = static_cast<PatRecipeListViewItem*> (listViewEnabledTests->topLevelItem(0));
                while(ptItem)
                {
                    if(ptItem->GetTestID() == lTestKey)
                    {
                        // Remove GUI line and exit from loop.
                        listViewEnabledTests->takeTopLevelItem(listViewEnabledTests->indexOfTopLevelItem(ptItem));
                        break;
                    }

                    // Next Listview line
                    ptItem = static_cast<PatRecipeListViewItem*> (listViewEnabledTests->topLevelItem((listViewEnabledTests->indexOfTopLevelItem(ptItem) +1 )));
                };
            }

            // Move to next test
            goto NextTestCell;

            // Move to next test cell.
NextTestCell:
            ptTestCell = ptTestCell->GetNextTest();
        };
    }

    // Success
    return true;

}

//////////////////////////////////////////////////////////
// User wants to DISABLE the tests selected: move them from the
// 'Enabled' list to the 'Disabled one'
//////////////////////////////////////////////////////////
void PatRecipeWizardDialog::OnDisableTests(void)
{
    // Identify the list of tests selected....and DISABLE them (move them to 'Disable' list)
    PatRecipeListViewItem	* ptItem = NULL;
    PatRecipeListViewItem	* ptDisabledItem = NULL;

    // If no selection, just ignore action.
    ptItem = static_cast<PatRecipeListViewItem*>(listViewEnabledTests->currentItem());

    if(ptItem == NULL)
        return;

    QList<QTreeWidgetItem *> selectedItems = listViewEnabledTests->selectedItems ();

    foreach(QTreeWidgetItem *poTreeWidgetItem,  selectedItems)
    {

        ptItem = static_cast<PatRecipeListViewItem*>(poTreeWidgetItem);

        // Add item to 'Disable' list.
        ptDisabledItem = new PatRecipeListViewItem(listViewDisabledTests, ptItem->GetTestID());
        ptDisabledItem->setText(0, ptItem->text(0));	// Test#
        ptDisabledItem->setText(1, ptItem->text(1));	// Test name
        ptDisabledItem->setText(2, ptItem->text(2));	//Test Type
        ptDisabledItem->setText(3, ptItem->text(3));	// Low Limit.
        ptDisabledItem->setText(4, ptItem->text(4));	// High Limit
        ptDisabledItem->setText(5, ptItem->text(5));	// Wizard advice
        ptDisabledItem->setText(6, ptItem->text(6));	// Comment

        // Remove item from current list
        listViewEnabledTests->takeTopLevelItem(listViewEnabledTests->indexOfTopLevelItem(ptItem));
    }
}

//////////////////////////////////////////////////////////
// Disabled page: contextual menu (right-click)
//////////////////////////////////////////////////////////
void
PatRecipeWizardDialog::contextMenuTestsToEnable(const QPoint&)
{
    QMenu menu(this);

    menu.addAction("KEEP selected tests into PAT recipe", this, SLOT(OnEnableTests()));

    menu.setMouseTracking(true);
    menu.exec(QCursor::pos());
    menu.setMouseTracking(false);
}

//////////////////////////////////////////////////////////
// User wants to ENABLE tests selected: move them from the
// 'Disabled' list to the 'Enabled' one'
//////////////////////////////////////////////////////////
void PatRecipeWizardDialog::OnEnableTests(void)
{
    // Identify the list of tests selected....and ENABLE them (move them to 'PAT-Ready' list)
    PatRecipeListViewItem *	ptItem			= NULL;
    PatRecipeListViewItem *	ptDisabledItem	= NULL;

    // If no selection, just ignore action.
    ptItem = static_cast<PatRecipeListViewItem*>(listViewDisabledTests->currentItem());
    if(ptItem == NULL)
        return;

    QList<QTreeWidgetItem *> selectedItems = listViewDisabledTests->selectedItems ();
    foreach(QTreeWidgetItem *poTreeWidgetItem, selectedItems){

        //ptItem = static_cast<PatRecipeListViewItem*>(listViewDisabledTests->topLevelItem(0));
        ptItem = static_cast<PatRecipeListViewItem*>(poTreeWidgetItem);

        // check if item selected...if selected, move them to the 'Disable' list & remove it from this list
//        if(ptItem->isSelected() == true)
//		{
            // Add item to 'Disable' list.
            ptDisabledItem = new PatRecipeListViewItem(listViewEnabledTests, ptItem->GetTestID());
            ptDisabledItem->setText(0, ptItem->text(0));	// Test#
            ptDisabledItem->setText(1, ptItem->text(1));	// Test name
            ptDisabledItem->setText(2, ptItem->text(2));	// Test Type
            ptDisabledItem->setText(3, ptItem->text(3));	// Low Limit.
            ptDisabledItem->setText(4, ptItem->text(4));	// High Limit
            ptDisabledItem->setText(5, ptItem->text(5));	// Wizard advice
            ptDisabledItem->setText(6, ptItem->text(6));	// Comment

            // Remove item from current list
            listViewDisabledTests->takeTopLevelItem (listViewDisabledTests->indexOfTopLevelItem(ptItem));
//        }
    }
}


//////////////////////////////////////////////////////////
// Enabled page: contextual menu (right-click)
//////////////////////////////////////////////////////////
void
PatRecipeWizardDialog::contextMenuTestsToDisable(const QPoint &)
{
    QMenu menu(this);
    menu.addAction("DISABLE selected tests from PAT", this, SLOT(OnDisableTests()));

    menu.setMouseTracking(true);
    menu.exec(QCursor::pos());
    menu.setMouseTracking(false);
}

//////////////////////////////////////////////////////////
// Return the list of tests to Disable from PAT recipe.
//////////////////////////////////////////////////////////
QList<TestInfo> PatRecipeWizardDialog::getDisabledTests(void)
{
    QList<TestInfo>         strExcludedTests;
    TestInfo                lTestInfo;
    PatRecipeListViewItem * ptItem = NULL;

    for (int lIdx = 0; lIdx < listViewDisabledTests->topLevelItemCount(); ++lIdx)
    {
        ptItem = static_cast<PatRecipeListViewItem *>(listViewDisabledTests->topLevelItem(lIdx));

        if (ptItem)
        {
            lTestInfo.mNumber       = ptItem->text(0).toInt();
            lTestInfo.mName         = ptItem->text(1);
            if (ptItem->text(2) == "M")
                lTestInfo.mPinIndex    = GEX_MPTEST;
            else
                lTestInfo.mPinIndex    = GEX_PTEST;

            // Add excluded test to the list7
            strExcludedTests += lTestInfo;  // Test#
        }
        else
            GSLOG(SYSLOG_SEV_WARNING, QString("Illegal item pointer at index %1").arg(lIdx).toLatin1().data());
    }

    // Return test list
    return strExcludedTests;
}

//////////////////////////////////////////////////////////
// Return the list of tests to Enabled from PAT recipe.
//////////////////////////////////////////////////////////
QList<TestInfo> PatRecipeWizardDialog::getEnabledTests(void)
{
    QList<TestInfo>         strIncludedTests;
    TestInfo                lTestInfo;
    PatRecipeListViewItem * ptItem = NULL;

    for (int lIdx = 0; lIdx < listViewEnabledTests->topLevelItemCount(); ++lIdx)
    {
        ptItem = static_cast<PatRecipeListViewItem *>(listViewEnabledTests->topLevelItem(lIdx));

        if (ptItem)
        {
            lTestInfo.mNumber       = ptItem->text(0).toInt();
            lTestInfo.mName         = ptItem->text(1);
            if (ptItem->text(2) == "M")
                lTestInfo.mPinIndex    = GEX_MPTEST;
            else
                lTestInfo.mPinIndex    = GEX_PTEST;

            // Add included test to the list
            strIncludedTests += lTestInfo;  // Test info
        }
        else
            GSLOG(SYSLOG_SEV_WARNING, QString("Illegal item pointer at index %1").arg(lIdx).toLatin1().data());
    }

    // Return test list
    return strIncludedTests;
}

//////////////////////////////////////////////////////////
// show previous Wizard page...
//////////////////////////////////////////////////////////
void PatRecipeWizardDialog::OnButtonBack()
{
    // Notify dialog box that it can return to caller!
    done(-2);
}

///////////////////////////////////////////////////////////////////////////////////
// Class PatRecipeListViewItem - class which represents an Pat Recipe item
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
PatRecipeListViewItem::PatRecipeListViewItem(QTreeWidget * pListView, const QString& testID)
    : QTreeWidgetItem(pListView), mTestID(testID)
{
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
PatRecipeListViewItem::~PatRecipeListViewItem()
{
}

const QString &PatRecipeListViewItem::GetTestID() const
{
    return mTestID;
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	int	compare(QTreeWidgetItem* pOtherItem,
//                              int nCol,
//                              bool /*bAscending*/) const
//
// Description	:	Override the comparison methods
//
///////////////////////////////////////////////////////////////////////////////////
int	PatRecipeListViewItem::compare(QTreeWidgetItem* pOtherItem,
                                   int nCol,
                                   bool /*bAscending*/) const
{
    QString strLeft		= text(nCol);
    QString strRight	= pOtherItem->text(nCol);

    if (nCol == 0) {
        return strLeft.toInt() - strRight.toInt();
    } else {
        return strLeft.compare(strRight);
    }
}

