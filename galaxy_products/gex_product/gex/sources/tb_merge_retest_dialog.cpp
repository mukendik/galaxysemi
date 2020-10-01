///////////////////////////////////////////////////////////
// Toolbox: Merge Test & Retest
///////////////////////////////////////////////////////////
#include "tb_merge_retest_dialog.h"
#include "tb_merge_retest.h"
#include "import_all.h"
#include "gex_constants.h"
#include "browser_dialog.h"
#include <gqtl_log.h>
#include "import_constants.h"
#include "engine.h"
#include "message.h"
#include <time.h>
#include "product_info.h"
#include "gex_shared.h"

#ifdef _WIN32
#include <io.h>
#endif

#include <QApplication>
#include <QCursor>
#include <QFileDialog>
#include <QMessageBox>
#include <QDragEnterEvent>
#include <QUrl>
#include <QMimeData>

#if defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
#endif

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexTbMergeRetestDialog::GexTbMergeRetestDialog( QWidget* parent, bool modal, Qt::WindowFlags fl) : QDialog(parent, fl)
{
    setupUi(this);
    setModal(modal);

    m_poMerge = new GexTbMergeRetest(true);

    // Apply Examinator palette
    GexMainwindow::applyPalette(this);

    QObject::connect(pushButtonMergeFiles, SIGNAL(clicked()), this, SLOT(OnMergeFiles()));

    // Support for Drag&Drop
    setAcceptDrops(true);

    // Disable sorting mode: merge is based on order files where selected
    mUi_pQtwTestFilesView->setSortingEnabled(false);
    mUi_pQtwRetestFilesView->setSortingEnabled(false);

    // Connect signals
    connect((QObject *)buttonSelectTestFile,SIGNAL(clicked()),this,SLOT(OnAddTestFile(void)));
    connect((QObject *)buttonUpTest,SIGNAL(clicked()),this,SLOT(OnMoveUpTestFile(void)));
    connect((QObject *)buttonDownTest,SIGNAL(clicked()),this,SLOT(OnMoveDownTestFile(void)));

    connect((QObject *)buttonSelectRetestFile,SIGNAL(clicked()),this,SLOT(OnAddRetestFile(void)));
    connect((QObject *)buttonUpRetest,SIGNAL(clicked()),this,SLOT(OnMoveUpRetestFile(void)));
    connect((QObject *)buttonDownRetest,SIGNAL(clicked()),this,SLOT(OnMoveDownRetestFile(void)));

    connect((QObject *)buttonRemoveTestFile,SIGNAL(clicked()),this,SLOT(OnDeleteTestFile(void)));
    connect((QObject *)buttonRemoveRetestFile,SIGNAL(clicked()),this,SLOT(OnDeleteRetestFile(void)));
    connect((QObject *)PushButtonClearAllTestFiles,SIGNAL(clicked()),this,SLOT(OnDeleteAllTestFiles(void)));
    connect((QObject *)PushButtonClearAllRetestFiles,SIGNAL(clicked()),this,SLOT(OnDeleteAllRetestFiles(void)));
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexTbMergeRetestDialog::~GexTbMergeRetestDialog()
{
    if(m_poMerge)
        delete m_poMerge;

}

///////////////////////////////////////////////////////////
// Starting DRAG sequence
///////////////////////////////////////////////////////////
void GexTbMergeRetestDialog::dragEnterEvent(QDragEnterEvent *e)
{
    // Accept Drag if files list dragged over.
    if(e->mimeData()->formats().contains("text/uri-list"))
        e->acceptProposedAction();
}

///////////////////////////////////////////////////////////
// Dropping files in Widget (Drag & Drop)
///////////////////////////////////////////////////////////
void GexTbMergeRetestDialog::dropEvent(QDropEvent *e)
{
    if(!e->mimeData()->formats().contains("text/uri-list"))
    {
        // No files dropped...ignore drag&drop.
        e->ignore();
        return;
    }

    QString		strFileName;
    QStringList strFileList;
    QList<QUrl> lstUrls = e->mimeData()->urls();

    for (int nUrl = 0; nUrl < lstUrls.count(); nUrl++)
    {
        strFileName = lstUrls.at(nUrl).toLocalFile();

        if (!strFileName.isEmpty())
            strFileList << strFileName;
    }

    if(strFileList.count() <= 0)
    {
        // Items dropped are not regular files...ignore.
        e->ignore();
        return;
    }

    // Check if active tab is TEST or RETEST?
    if(tabWidget->currentIndex() == 0)
        OnAddTestFileList(strFileList);	// TEST tab
    else
        OnAddRetestFileList(strFileList);	// RE-TEST tab

    e->acceptProposedAction();
}

///////////////////////////////////////////////////////////
// Insert list of files in TEST list.
///////////////////////////////////////////////////////////
void	GexTbMergeRetestDialog::OnAddTestFileList(QStringList &strFiles)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(GS::LPPlugin::ProductInfo::toolbox))
    {
      // OEM mode, refuse to run this function!
      GS::Gex::Message::information(
          "", "This function is disabled in teradyne mode\n\nContact " +
          QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }

    if(strFiles.count() < 1)
        return;

    // Add file to list
    QStringList::Iterator it;
    QStringList	strListView;
    QString	strFile;
    for(it = strFiles.begin(); it != strFiles.end(); ++it )
    {
        strFile = *it;
        if(strFile.startsWith("file:///"))
            strFile = QUrl(strFile).toLocalFile();

        // Insert file unless already in list!
        strListView = getTestFileList(true);
        if(strListView.indexOf (strFile) < 0)
        {
            QTreeWidgetItem *pQtwiItem = new QTreeWidgetItem(mUi_pQtwTestFilesView);
            pQtwiItem->setText(0, strFile);
        }
    }
}

///////////////////////////////////////////////////////////
// Get list of files in TEST or RE-TESTlist.
///////////////////////////////////////////////////////////
QStringList	GexTbMergeRetestDialog::getTestFileList(bool bTestList/*=true*/)
{
    QStringList strFiles;

    QTreeWidget *pQtwConcernedTreeWidget;
    if(bTestList)
        pQtwConcernedTreeWidget = mUi_pQtwTestFilesView;
    else
        pQtwConcernedTreeWidget = mUi_pQtwRetestFilesView;

    int nTopLevelItemCount = pQtwConcernedTreeWidget->topLevelItemCount();
    if(nTopLevelItemCount<=0)
    {
        GSLOG(SYSLOG_SEV_WARNING, "call get (re)test file list without list ... ");
        return strFiles;
    }

    QTreeWidgetItem *pQtwiItem;
    for(int ii=0; ii<nTopLevelItemCount; ii++)
    {
        pQtwiItem = pQtwConcernedTreeWidget->topLevelItem(ii);
        strFiles.append(pQtwiItem->text(0));
    }

    return strFiles;
}

///////////////////////////////////////////////////////////
// Select Test data file(s) to merge with
///////////////////////////////////////////////////////////
void	GexTbMergeRetestDialog::OnAddTestFile()
{
    GS::Gex::SelectDataFiles	cSelectFile;

    // User wants to analyze a single file
    QStringList strFiles;
    QString		strFile;
    strFiles = cSelectFile.GetFiles(this, strFile, "Select Data files");

    // Insert selected files in TEST list
    OnAddTestFileList(strFiles);
}

///////////////////////////////////////////////////////////
// Test list: Move selections UP
///////////////////////////////////////////////////////////
void	GexTbMergeRetestDialog::OnMoveUpTestFile()
{
    // If no item in list, just return!
    if(mUi_pQtwTestFilesView->topLevelItemCount() <= 1)
        return;

    // Scan the list to move UP the selected item.
    QList<QTreeWidgetItem *> qlSelectedItemList = mUi_pQtwTestFilesView->selectedItems();
    if(qlSelectedItemList.isEmpty())
        return;

    QTreeWidgetItem *pQtwiItem=NULL;

    QMap<int, QTreeWidgetItem *> qmItemOrderingMap;
    // order and get position of selected items
    for(int ii=0; ii<qlSelectedItemList.count(); ii++)
    {
        pQtwiItem = qlSelectedItemList.at(ii);
        int nItemPostion = mUi_pQtwTestFilesView->indexOfTopLevelItem(pQtwiItem);
        qmItemOrderingMap.insert(nItemPostion, pQtwiItem);
    }

    QMapIterator<int, QTreeWidgetItem *>	qmiIterator(qmItemOrderingMap);
    while(qmiIterator.hasNext())
    {
        qmiIterator.next();
        if(qmiIterator.key() == 0)	// if 1st file selected...can't move up..ignore!
        {
            if(qmiIterator.hasNext())
                qmiIterator.next();
            else
                return;
        }

        pQtwiItem = mUi_pQtwTestFilesView->takeTopLevelItem(qmiIterator.key());
        mUi_pQtwTestFilesView->insertTopLevelItem( (qmiIterator.key() - 1), pQtwiItem );
    }

    mUi_pQtwTestFilesView->setCurrentItem(pQtwiItem);
}

///////////////////////////////////////////////////////////
// Test list: Move selections DOWN
///////////////////////////////////////////////////////////
void	GexTbMergeRetestDialog::OnMoveDownTestFile()
{
    // If no item in list, just return!
    int nTopLevelItemCount = mUi_pQtwTestFilesView->topLevelItemCount();
    if( nTopLevelItemCount <= 1)
        return;

    // Scan the list to move DOWN the selected item.
    QList<QTreeWidgetItem *> qlSelectedItemList = mUi_pQtwTestFilesView->selectedItems();
    if(qlSelectedItemList.isEmpty())
        return;

    QTreeWidgetItem *pQtwiItem=NULL;

    QMap<int, QTreeWidgetItem *> qmItemOrderingMap;
    // order and get position of selected items
    for(int ii=0; ii<qlSelectedItemList.count(); ii++)
    {
        pQtwiItem = qlSelectedItemList.at(ii);
        int nItemPostion = mUi_pQtwTestFilesView->indexOfTopLevelItem(pQtwiItem);
        qmItemOrderingMap.insert(nItemPostion, pQtwiItem);
    }

    QMapIterator<int, QTreeWidgetItem *>	qmiIterator(qmItemOrderingMap);

    qmiIterator.toBack();

    while(qmiIterator.hasPrevious())
    {
        qmiIterator.previous();
        if(qmiIterator.key() == nTopLevelItemCount-1)	// if 1st file selected...can't move up..ignore!
        {
            if(qmiIterator.hasPrevious())
                qmiIterator.previous();
            else
                return;
        }

        pQtwiItem = mUi_pQtwTestFilesView->takeTopLevelItem(qmiIterator.key());
        mUi_pQtwTestFilesView->insertTopLevelItem( (qmiIterator.key() + 1), pQtwiItem );
    }

    mUi_pQtwTestFilesView->setCurrentItem(pQtwiItem);
}

///////////////////////////////////////////////////////////
// Remove Test data file(s) from list
///////////////////////////////////////////////////////////
void	GexTbMergeRetestDialog::OnDeleteTestFile()
{
    if(mUi_pQtwTestFilesView->topLevelItemCount()<=0)
        return;

    // Scan the list to remove selected items.
    QList<QTreeWidgetItem *> qlSelectedItemsList = mUi_pQtwTestFilesView->selectedItems();
    if(qlSelectedItemsList.isEmpty())
    {return;}

    QTreeWidgetItem *pQtwiItem;
    for(int ii=0; ii<qlSelectedItemsList.count(); ii++)
    {
        pQtwiItem = qlSelectedItemsList.at(ii);
        int nCurrentItemPosition = mUi_pQtwTestFilesView->indexOfTopLevelItem(pQtwiItem);
        mUi_pQtwTestFilesView->takeTopLevelItem(nCurrentItemPosition);		// Remove selected item from list
        delete pQtwiItem;
    }
}

///////////////////////////////////////////////////////////
// Remove All Test data file(s) from list
///////////////////////////////////////////////////////////
void	GexTbMergeRetestDialog::OnDeleteAllTestFiles()
{
    QTreeWidgetItem *qtwiTreeWidgetItem;
    while(mUi_pQtwTestFilesView->topLevelItemCount()>0)
    {
        qtwiTreeWidgetItem = mUi_pQtwTestFilesView->takeTopLevelItem(0);
        delete qtwiTreeWidgetItem;
        qtwiTreeWidgetItem=0;
    }
}

///////////////////////////////////////////////////////////
// Insert list of files in RE-TEST list.
///////////////////////////////////////////////////////////
void	GexTbMergeRetestDialog::OnAddRetestFileList(QStringList &strFiles)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(GS::LPPlugin::ProductInfo::toolbox))
    {
      // OEM mode, refuse to run this function!
      GS::Gex::Message::information(
          "", "This function is disabled in teradyne mode\n\nContact " +
          QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }

    if(strFiles.count() < 1)
        return;

    // Add file to list
    QStringList strListView;
    QStringList::Iterator it;
    QString		strFile;
    for(it = strFiles.begin(); it != strFiles.end(); ++it )
    {
        strFile = *it;
        if(strFile.startsWith("file:///"))
            strFile = QUrl(strFile).toLocalFile();

        // Insert file unless already in list!
        strListView = getTestFileList(false);
        if(strListView.indexOf (strFile) < 0)
        {
            QTreeWidgetItem *pQtwiItem = new QTreeWidgetItem(mUi_pQtwRetestFilesView);
            pQtwiItem->setText(0, strFile);
        }
    }
}

///////////////////////////////////////////////////////////
// Select retest data files to merge with
///////////////////////////////////////////////////////////
void	GexTbMergeRetestDialog::OnAddRetestFile()
{
    GS::Gex::SelectDataFiles	cSelectFile;

    // User wants to analyze a single file
    QStringList strFiles;
    QString		strFile;
    strFiles = cSelectFile.GetFiles(this, strFile, "Select Retest Data Files");

    // Insert list in RE-Test tab
    OnAddRetestFileList(strFiles);
}

///////////////////////////////////////////////////////////
// ReTest list: Move selections UP
///////////////////////////////////////////////////////////
void	GexTbMergeRetestDialog::OnMoveUpRetestFile()
{
    // If no item in list, just return!
    if(mUi_pQtwRetestFilesView->topLevelItemCount() <= 1)
        return;

    // Scan the list to move UP the selected item.
    QList<QTreeWidgetItem *> qlSelectedItemList = mUi_pQtwRetestFilesView->selectedItems();
    if(qlSelectedItemList.isEmpty())
        return;

    QTreeWidgetItem *pQtwiItem=NULL;

    QMap<int, QTreeWidgetItem *> qmItemOrderingMap;
    // order and get position of selected items
    for(int ii=0; ii<qlSelectedItemList.count(); ii++)
    {
        pQtwiItem = qlSelectedItemList.at(ii);
        int nItemPostion = mUi_pQtwRetestFilesView->indexOfTopLevelItem(pQtwiItem);
        qmItemOrderingMap.insert(nItemPostion, pQtwiItem);
    }

    QMapIterator<int, QTreeWidgetItem *>	qmiIterator(qmItemOrderingMap);

    while(qmiIterator.hasNext())
    {
        qmiIterator.next();
        if(qmiIterator.key() == 0)	// if 1st file selected...can't move up..ignore!
        {
            if(qmiIterator.hasNext())
                qmiIterator.next();
            else
                return;
        }

        pQtwiItem = mUi_pQtwRetestFilesView->takeTopLevelItem(qmiIterator.key());
        mUi_pQtwRetestFilesView->insertTopLevelItem( (qmiIterator.key() - 1), pQtwiItem );
    }

    mUi_pQtwRetestFilesView->setCurrentItem(pQtwiItem);
}

///////////////////////////////////////////////////////////
// ReTest list: Move selections DOWN
///////////////////////////////////////////////////////////
void	GexTbMergeRetestDialog::OnMoveDownRetestFile()
{
    // If no item in list, just return!
    int nTopLevelItemCount = mUi_pQtwRetestFilesView->topLevelItemCount();
    if( nTopLevelItemCount <= 1)
        return;

    // Scan the list to move DOWN the selected item.
    QList<QTreeWidgetItem *> qlSelectedItemList = mUi_pQtwRetestFilesView->selectedItems();
    if(qlSelectedItemList.isEmpty())
        return;

    QTreeWidgetItem *pQtwiItem=NULL;

    QMap<int, QTreeWidgetItem *> qmItemOrderingMap;
    // order and get position of selected items
    for(int ii=0; ii<qlSelectedItemList.count(); ii++)
    {
        pQtwiItem = qlSelectedItemList.at(ii);
        int nItemPostion = mUi_pQtwRetestFilesView->indexOfTopLevelItem(pQtwiItem);
        qmItemOrderingMap.insert(nItemPostion, pQtwiItem);
    }

    QMapIterator<int, QTreeWidgetItem *>	qmiIterator(qmItemOrderingMap);

    qmiIterator.toBack();
    while(qmiIterator.hasPrevious())
    {
        qmiIterator.previous();
        if(qmiIterator.key() == nTopLevelItemCount-1)	// if 1st file selected...can't move up..ignore!
        {
            if(qmiIterator.hasPrevious())
                qmiIterator.previous();
            else
                return;
        }

        pQtwiItem = mUi_pQtwRetestFilesView->takeTopLevelItem(qmiIterator.key());
        mUi_pQtwRetestFilesView->insertTopLevelItem( (qmiIterator.key() + 1), pQtwiItem );
    }

    mUi_pQtwRetestFilesView->setCurrentItem(pQtwiItem);
}

///////////////////////////////////////////////////////////
// Remove Retest data file(s) from list
///////////////////////////////////////////////////////////
void	GexTbMergeRetestDialog::OnDeleteRetestFile()
{
    if(mUi_pQtwRetestFilesView->topLevelItemCount()<=0)
        return;

    // Scan the list to remove selected items.
    QList<QTreeWidgetItem *> qlSelectedItemsList = mUi_pQtwRetestFilesView->selectedItems();
    if(qlSelectedItemsList.isEmpty())
    {return;}

    QTreeWidgetItem *pQtwiItem;
    for(int ii=0; ii<qlSelectedItemsList.count(); ii++)
    {
        pQtwiItem = qlSelectedItemsList.at(ii);
        int nCurrentItemPosition = mUi_pQtwRetestFilesView->indexOfTopLevelItem(pQtwiItem);
        mUi_pQtwRetestFilesView->takeTopLevelItem(nCurrentItemPosition);		// Remove selected item from list
        delete pQtwiItem;
    }
}

///////////////////////////////////////////////////////////
// Remove All Retest data file(s) from list
///////////////////////////////////////////////////////////
void	GexTbMergeRetestDialog::OnDeleteAllRetestFiles()
{
    QTreeWidgetItem *qtwiTreeWidgetItem;
    while(mUi_pQtwRetestFilesView->topLevelItemCount()>0)
    {
        qtwiTreeWidgetItem = mUi_pQtwRetestFilesView->takeTopLevelItem(0);
        delete qtwiTreeWidgetItem;
    }
}

///////////////////////////////////////////////////////////
// ON Button cliked "Merge files"
///////////////////////////////////////////////////////////


void	GexTbMergeRetestDialog::OnMergeFiles()
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(GS::LPPlugin::ProductInfo::toolbox))
    {
      // OEM mode, refuse to run this function!
      GS::Gex::Message::information(
          "", "This function is disabled in teradyne mode\n\nContact " +
          QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }

    QString strError;

    // Get list of files to merge
    QStringList strTestDataFiles = getTestFileList(true);
    QStringList strRetestDataFiles = getTestFileList(false);

    if(strTestDataFiles.isEmpty())
        return;

    // Build path to destination: use input folder!
    strOutputFile = (strTestDataFiles.isEmpty()) ? "" : strTestDataFiles.first();
    strOutputFile = strOutputFile + "_merged.std";

    // Select output file where data files will be merged
    strOutputFile = QFileDialog::getSaveFileName(this,
                                                 "Define output file name...",
                                                 strOutputFile,
                                                 "STDF file(*.std *.stdf)",
                                                 NULL,
                                                 QFileDialog::DontConfirmOverwrite);

    // If no file selected, quiet return
    if (strOutputFile.isEmpty())
    {
        return;
    }

    // Make sure file name ends with ".std" or ".stdf" extension
    if((strOutputFile.endsWith(".std",Qt::CaseInsensitive) == false) &&
            (strOutputFile.endsWith(".stdf",Qt::CaseInsensitive) == false))
        strOutputFile += ".std";

    // If output file already exists, request confirmation to overwrite
    if(QFile::exists(strOutputFile) == true)
    {
        bool lOk;
        GS::Gex::Message::request("", "File already exists. Overwrite it ?", lOk);
        if (! lOk)
        {
            return;
        }
    }


    // Check that we have at least one test and one retest file selected!
    if(strTestDataFiles.count() < 1)
    {
        GS::Gex::Message::
                information("", "You have not selected any Test data file!");
        return;
    }

    bool bIsGDF = false;
    m_poMerge->SetGDFFile(bIsGDF);

    // Merge (& sort) all Test files in one
    QString strMergedTestFile = strTestDataFiles[0] + "_test_galaxy.stdf";
    if(strRetestDataFiles.count() < 1)
        strMergedTestFile	= strOutputFile;	// If only merging Test data (no retest), then use Output name as merged test name.
    MergeSamplesFiles(strTestDataFiles,strMergedTestFile,true,strError);
    bIsGDF = (bIsGDF || m_poMerge->IsGDFFile());

    // If only had to merge Test files, we're done!
    if(strRetestDataFiles.count() < 1)
    {
        if(strError.isEmpty())
        {
            strError = "Merge successful!\nTest data merged to: " + strOutputFile;
            GS::Gex::Message::information("", strError);
        }
        else
        {
            GS::Gex::Message::critical("", strError);
        }
        // Closes dialog pop-up.
        done(1);
        return;
    }

    // Merge all retest files in one.
    QString strMergedRetestFile = strRetestDataFiles[0] + "_retest_galaxy.stdf";
    MergeSamplesFiles(strRetestDataFiles,strMergedRetestFile,true,strError);
    bIsGDF = (bIsGDF || m_poMerge->IsGDFFile());

    // Change cursor to Hour glass (Wait cursor)...
    QGuiApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    m_poMerge->SetGDFFile(bIsGDF);
    // Merge test & retest files.
    QStringList strSources;
    strSources << strMergedTestFile << strMergedRetestFile;
    int iStatus = MergeFiles(strSources,strOutputFile,strError);

    // Change cursor back to normal
    QGuiApplication::restoreOverrideCursor();

    // Cleanup: delete all temporary files
    unlink(strMergedTestFile.toLatin1().constData());		// File xx_test_galaxy.stdf
    unlink(strMergedRetestFile.toLatin1().constData());	// File xx_retest_galaxy.stdf

    // Check Status and display message accordingly.
    if(iStatus == GexTbMergeRetest::NoError)
    {
        strError = "Merge successful!\nFile created: " + strOutputFile;
        GS::Gex::Message::information("", strError);

        // Closes dialog pop-up.
        done(1);
    }
    else
    {
        GS::Gex::Message::critical("", strError);
    }
}

///////////////////////////////////////////////////////////
// Merge files (data samples only) ignore all summary & HBR/SBRs
///////////////////////////////////////////////////////////
int	GexTbMergeRetestDialog::MergeSamplesFiles(QStringList strSources,QString strOutput,bool bRebuildHbrSbr,QString &strErrorMessage)
{
    // Change cursor to Hour glass (Wait cursor)...
    QGuiApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    int iRet = m_poMerge->MergeSamplesFiles(strSources,strOutput,bRebuildHbrSbr,(comboBoxMergingMode->currentIndex() == 0));

    if (iRet != GexTbMergeRetest::NoError)
        strErrorMessage = m_poMerge->GetErrorMessage();

    // Change cursor back to normal
    QGuiApplication::restoreOverrideCursor();
    return iRet;

}

///////////////////////////////////////////////////////////
// Merge files function: updates HBR/SBR as need be
///////////////////////////////////////////////////////////

int	GexTbMergeRetestDialog::MergeFiles(QStringList strSources,QString strOutput,QString &strErrorMessage)
{
    int lRet = GexTbMergeRetest::NoError;

    if(GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(GS::LPPlugin::ProductInfo::toolbox))
    {
      // OEM mode, refuse to run this function!
      GS::Gex::Message::information(
          "", "This function is disabled in teradyne mode\n\nContact " +
          QString(GEX_EMAIL_SALES) + " for more information!");
        strErrorMessage = "error: This function is disabled in teradyne mode";
        return GexTbMergeRetest::LicenseError;
    }

    lRet = m_poMerge->loadFilesToMerge(strSources);
    if(lRet != GexTbMergeRetest::NoError)
    {
        strErrorMessage = m_poMerge->GetErrorMessage();
        return lRet;
    }

    if(m_poMerge->GetTestData()->lTotalParts < m_poMerge->GetReTestData()->lTotalParts)
    {
        QMessageBox oMergeBox(this);
        oMergeBox.setWindowTitle(
                    GS::Gex::Engine::GetInstance().Get("AppFullName").toString());
        oMergeBox.setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));
        oMergeBox.setText(QString(" Your retest data contains more parts  (%1) than your initial test data (%2)."
                                  " This could be normal if you entered retest data for several retest levels.\n"
                                  "Do you want to proceed ?").arg(m_poMerge->GetReTestData()->lTotalParts).arg(m_poMerge->GetTestData()->lTotalParts ));
        oMergeBox.addButton("Proceed", QMessageBox::YesRole);
        QPushButton *oSwap = oMergeBox.addButton("Swap", QMessageBox::NoRole);
        QPushButton *oCancel = oMergeBox.addButton("Cancel", QMessageBox::RejectRole);
        oMergeBox.exec();
        QAbstractButton *oUserChoice = oMergeBox.clickedButton();
        if(!oUserChoice || oUserChoice == oCancel)
        {
            strErrorMessage = "Merge Canceled";
            m_poMerge->ExitCleanup();
            return GexTbMergeRetest::NoFile;
        }
        else if(oUserChoice == oSwap)
        {
            m_poMerge->swapTestReTest();
        }
    }

    lRet = m_poMerge->MergeFiles(strSources,strOutput);

    if (lRet != GexTbMergeRetest::NoError)
        strErrorMessage = m_poMerge->GetErrorMessage();

    return lRet;
}


