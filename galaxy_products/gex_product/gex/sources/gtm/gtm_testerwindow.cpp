#ifdef GCORE15334


#include <QInputDialog>
#include <QFileDialog>
#include <classes.h>
#include <ctest.h>
#include <gqtl_log.h>
#include <gqtl_sysutils.h>

#include "pat_info.h"
#include "pat_definition.h"
#include "patman_lib.h"

#include "browser_dialog.h"
#include "gtm_mainwidget.h"
#include "gtm_testerwindow.h"
#include "engine.h"
#include "message.h"
#include "time.h"
#include "gs_qa_dump.h"
#include "clientnode.h"
#include "clientsocket.h"

//#include "DebugMemory.h" // must be the last include


// global variables
extern GexMainwindow *pGexMainWindow;

const char* Gtm_TesterWindow::sClientAboutToCloseProp="ClientAboutToClose";

/////////////////////////////////////////////////////////////////////////////
// Call-back Function used by the 'sort' function to sort PAT definitions in execution flow order
/////////////////////////////////////////////////////////////////////////////
bool comparePatDefinition(CPatDefinition *test1, CPatDefinition *test2)
{
    return(test1->m_lSequenceID < test2->m_lSequenceID);
}

Gtm_TesterWindow::Gtm_TesterWindow( QWeakPointer<GS::Gex::ClientNode> lCNWP, QWidget* parent, Qt::WindowFlags fl)
    : QDialog(parent, fl), mClientNodeWP(lCNWP), mSitesTableWidget(this) //, mClientNode(cn)
{
    setProperty(sClientAboutToCloseProp, false);

    // Setup UI
    setupUi(this);
    //setPaletteBackgroundColor(Qt::white); // Qt3
    QPalette p = palette(); p.setColor(backgroundRole(), Qt::white); setPalette(p);

    // Remove unused tabs
    tabWidget->removeTab(5);    // Admin tab

    QDir lDir;
    // Startup GUI fields.
    textGlobalInfo->setText("");
    textProduct->setText("");
    textLot->setText("");
    textTesterType->setText("");
    textTesterName->setText("");

    // Not implemented yet, so hide the GUI elements
    labelLoadPatLimits->hide();
    buttonLoadPatLimits->hide();

    // Clear variables
    bChildWindow = true;

    // Set yield alarm level and histo frame shape
    if (!mClientNodeWP.toStrongRef().isNull())
    {
        frameHistoChart->setYieldAlarmLevel(mClientNodeWP.data()->mStation.GetPatOptions().mFT_YieldLevel);
        frameHistoChart->setFrameShape( QFrame::NoFrame );
    }
    else
    {
        GSLOG(3, "Cannot strongify ClientNode weak pointer");
        GS::Gex::Engine::GetInstance().Exit(-100);
    }
    // Connect signal/slots

    GS::Gex::ClientNode* lCN=mClientNodeWP.toStrongRef().data();

    // Qt::DirectConnection is impossible because these 2 does not belong to the same thread
    // Qt::BlockingQueuedConnection ?
    if (!connect(lCN, SIGNAL(sAboutToClose()), this, SLOT(OnClientAboutToClose()) ))
        GSLOG(3, "Cannot connect signal from ClientNode");

    if (!connect(lCN, SIGNAL(destroyed()), this, SLOT(OnClientDestroyed()) ))
        GSLOG(3, "Cannot connect signal destroyed from ClientNode");

    if (!connect(lCN, SIGNAL(sReset()), this, SLOT(OnResetMessage()) ))
        GSLOG(3, "Cannot connect signal Reset from ClientNode");

    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(OnTabChange(int)));
    connect(pushButtonOkWarning, SIGNAL(clicked()), this, SLOT(OnWarningOk()));
    connect(pushButtonOkStop, SIGNAL(clicked()), this, SLOT(OnStopOk()));
    connect(buttonDetachWindow, SIGNAL(clicked()), this, SLOT(OnDetachWindow()));
    connect(buttonReset, SIGNAL(clicked()), this, SLOT(OnResetMessage()));
    connect(buttonLoadPatLimits, SIGNAL(clicked()), this, SLOT(OnLoadPatLimitsFromfile()));

    /*
      Best seems to be : Queued Connection :
        The slot is invoked when control returns to the event loop of the receiver's thread.
        The slot is executed in the receiver's thread (here the main GUI thread).
      */

    if (!QObject::connect(lCN, SIGNAL(sClientInit(CStation*)),
                          this, SLOT(OnClientInit(CStation*)),
          Qt::AutoConnection // Should be set to QueuedConnection
          //Qt::DirectConnection // connection failed: does not seem to work when multithread. No way : the GUI cannot be updated in any sub thread.
          //Qt::QueuedConnection // QObject::connect: Cannot queue arguments of type 'CStation' (Make sure 'CStation' is registered using qRegisterMetaType().)
          //Qt::BlockingQueuedConnection // connection fail : ?
          //Qt::UniqueConnection // QObject::connect: Cannot queue arguments of type 'CStation' (Make sure 'CStation' is registered using qRegisterMetaType().)
        ))
    {
        GSLOG(3, "Cannot connect signal ClientInit");
        setWindowTitle("Cannot connect signal ClientInit"); //setCaption("Cannot connect signal ClientInit");
        //setBackgroundColor(QColor(255,0,0, 125)); // Qt3
        QPalette p = palette(); p.setColor(backgroundRole(), QColor(255,0,0, 125)); setPalette(p);
        GS::Gex::Engine::GetInstance().Exit(EXIT_FAILURE);
        return;
    }
    else
        GSLOG(5, "Signal ClientInit connected");

    GS::Gex::ClientSocket* lCS=qobject_cast<GS::Gex::ClientSocket*>(lCN->parent());
    if (!lCS)
    {
        GSLOG(3, "Cannot cast ClientNode parent to ClientSocket");
        GS::Gex::Engine::GetInstance().Exit(EXIT_FAILURE);
        return;
    }
    if (!QObject::connect( lCS, SIGNAL(sClientAccepted()), this, SLOT(show())))
    {
        GSLOG(3, "Cannot connect ClientAccepted signal");
        GS::Gex::Engine::GetInstance().Exit(EXIT_FAILURE);
        return;
    }

    if (!QObject::connect(lCN, SIGNAL(sReset()), this, SLOT(OnResetMessage())))
    {
        GSLOG(3, "Cannot connect reset message");
        GS::Gex::Engine::GetInstance().Exit(EXIT_FAILURE);
        return;
    }

    if (!QObject::connect(lCN, SIGNAL(sNewTestResults()), this, SLOT(RefreshGUI()))) // Qt::AutoConnection ?????
    {
        GSLOG(3, "Cannot connect NewTestResults signal");
        GS::Gex::Engine::GetInstance().Exit(EXIT_FAILURE);
        return;
    }

    if (!QObject::connect(lCN, SIGNAL(sNewMessage(int,QString,int)), this, SLOT(OnNewMessage(int,QString,int))))
    {
        GSLOG(3, "Cannot connect sNewMessage signal");
        GS::Gex::Engine::GetInstance().Exit(EXIT_FAILURE);
        return;
    }

    if (!QObject::connect(lCN, SIGNAL(sNewBinResult(int,int)), this, SLOT(OnNewBinResult(int,int))))
    {
        GSLOG(3, "Cannot connect sNewBinResult signal");
        GS::Gex::Engine::GetInstance().Exit(EXIT_FAILURE);
        return;
    }
    // Create spooling directories (later used when writing Email failes & traceability files)
    QString lGsFolder = GS::Gex::Engine::GetInstance().Get("GalaxySemiFolder").toString();
    if(lGsFolder.isEmpty() == false)
    {
        // Server found, then make sure spool folders are created.
        QString	strFolder = lGsFolder + GTM_DIR_SPOOL;
        lDir.mkdir(strFolder);
        strFolder = lGsFolder + GTM_DIR_EMAILS;	// Emails
        lDir.mkdir(strFolder);
        strFolder = lGsFolder + GTM_DIR_TRACE;	// Traceability files
        lDir.mkdir(strFolder);
    }

    // Sites widget
    //mSitesTableWidget.setParent(tabWidget); // which parent to give ?
    mSitesTableWidget.setParent(SitesTab);
    SitesTab->layout()->addWidget(&mSitesTableWidget);
    mSitesTableWidget.setAlternatingRowColors(true);
    //mSitesTableWidget.setContentsMargins();
    // Hide on creation, will be shown later if accepted ?
    //hide();

    #ifdef _PAT_DEBUG
        QFile::remove("C:\\temp\\gtm_debug.txt");
    #endif
}

Gtm_TesterWindow::~Gtm_TesterWindow()
{
    GSLOG(6, "Gtm_TesterWindow destructor");

    mSitesTableWidget.clear();
    /*
    for(QList<QTableWidgetItem*>::Iterator i=mSitesTableWidgetItems.begin(); i!=mSitesTableWidgetItems.end(); ++i )
        if (*i)
            delete *i;
    mSitesTableWidgetItems.clear();
    */
}

#define UPDATE_ITEM(row, col, new_text) lIt=mSitesTableWidget.item( row, col ); \
    if (lIt) lIt->setText( new_text );

void Gtm_TesterWindow::ReloadSitesTab(void)
{
    GS::Gex::SiteTestResults*   lSite=NULL;
    QColor lColor=Qt::white;

    QSharedPointer<GS::Gex::ClientNode> lCNSP=mClientNodeWP.toStrongRef();

    if (lCNSP.isNull())
    {
        GSLOG(5, "Cannot strongify clientnode weak pointer.");
        mSitesTableWidget.setEnabled(false);
        return;
    }

    if (lCNSP->GetSocketInstance()==-1)
    {
        mSitesTableWidget.setEnabled(false);
        return;
    }


    //mSitesTabString = QString("<table style=\"text-align: left; width: 100%;\" border=\"1\"cellpadding=\"2\" cellspacing=\"2\">");
    //mSitesTabString += QString("<tbody>");

    GS::Gex::ClientNode* lCN=lCNSP.data();

    if (lCN->mStation.cTestData.mSites.size()==0)
        return; // Sites config not yet received ?

    if (mSitesTableWidget.rowCount()==0)
        SetupSitesTable(&lCN->mStation); // will generate headers and TableWidgetItems

    GS::Gex::SiteList::Iterator it = lCN->mStation.cTestData.mSites.begin();
    for(it=lCN->mStation.cTestData.mSites.begin(); it!=lCN->mStation.cTestData.mSites.end(); ++it)
    {
        // Get valid site# used and site ptr.
        lSite = it.value(); // 6935: use value, instead of []
        if(!lSite)
        {
            GSLOG(4, "Null site found in station site lists");
            continue; // or break ?
        }

        // Update of the new TableWidget
        int lColNb=mSitesNbToColIndex.value(lSite->SiteNb());
        GS::Gex::SiteTestResults::siteState lSiteState=lSite->SiteState();
        switch(lSiteState)
        {
            default:
                lColor = Qt::gray;   //QColor(150,150,150);
                break;
            case GS::Gex::SiteTestResults::SITESTATE_DISABLED:
                lColor = Qt::red;   //QColor(255,0,0); // red
                break;
            case GS::Gex::SiteTestResults::SITESTATE_BASELINE:
                lColor = Qt::yellow;   //QColor(255,255,0); // yellow
                break;
            case GS::Gex::SiteTestResults::SITESTATE_DPAT:
                if(lSite->WaitForEarlyTuning())
                    lColor = QColor(150,255,150); //QString(" background-color: lime;");
                else
                    lColor = Qt::green;   //QColor(0,255,0);   //QString(" background-color: green;");
                break;
        }

        QTableWidgetItem* lIt=0; // will be used by macro to search for the item
        UPDATE_ITEM(0, lColNb, lSite->SiteStateName())
        if (lIt)
          lIt->setBackgroundColor(lColor);
        UPDATE_ITEM(1, lColNb, QString::number(lSite->BaselineCount()))
        UPDATE_ITEM(2, lColNb, QString::number(lSite->TuningCount()))
        // RB
        UPDATE_ITEM(3, lColNb, QString::number(lSite->RBSize()))
        UPDATE_ITEM(4, lColNb, QString::number(lSite->RBCurIndex()))
        UPDATE_ITEM(5, lColNb, QString::number(lSite->RBValidParts() ) )
        // Lot : lSite->LotParts() returns a PartCounters &
        UPDATE_ITEM(6, lColNb, QString::number(lSite->LotParts().TestedParts() ) )
        UPDATE_ITEM(7, lColNb, QString::number(lSite->LotParts().PassParts() ) )
        UPDATE_ITEM(8, lColNb, QString::number(lSite->LotParts().TestedParts() - lSite->LotParts().PassParts() ) )
        UPDATE_ITEM(9, lColNb, QString::number(lSite->LotParts().RealOutlierParts() ) )
        UPDATE_ITEM(10, lColNb, QString::number(lSite->LotParts().VirtualOutlierParts() ) )

        // Last Part : LastPart() returns a copy !
        //UPDATE_ITEM(11, lColNb, lSite->LastPart().PartID() ) // crash
        UPDATE_ITEM(11, lColNb, lSite->LastPartID()) // no crash
        //UPDATE_ITEM(12, lColNb, QString::number(lSite->LastPart().PartIndex()) )
        UPDATE_ITEM(12, lColNb, QString::number(lSite->LastPartIndex()) )

        // SLL : SLLParts() is returning a PartCounters &
        unsigned int lSLLTP=lSite->SLLPartsTestedParts();
        unsigned int lSLLPP=lSite->SLLPartsPassParts();
        UPDATE_ITEM(13, lColNb, QString::number( lSLLTP ) )
        UPDATE_ITEM(14, lColNb, QString::number( lSLLPP ) )
        UPDATE_ITEM(15, lColNb, QString::number( lSLLTP - lSLLPP ) )
        UPDATE_ITEM(16, lColNb, QString::number(lSite->SLLParts().RealOutlierParts() ) )
        UPDATE_ITEM(17, lColNb, QString::number(lSite->SLLParts().VirtualOutlierParts() ) )

        // Previous impl using html
        //if(lSite)
        //{
            // Let's try to do a copy to prevent to multithreading fighting on this ressource ?
            //GS::Gex::PartResult lLastPartResult=lSite->LastPart();
            /*
            GS::Gex::SiteTestResults::siteState lSiteState=lSite->SiteState();
            switch(lSiteState)
            {
                default:
                    lColor = QString(" background-color: grey;");
                    break;
                case GS::Gex::SiteTestResults::SITESTATE_DISABLED:
                    lColor = QString(" background-color: red;");
                    break;
                case GS::Gex::SiteTestResults::SITESTATE_BASELINE:
                    lColor = QString(" background-color: yellow;");
                    break;
                case GS::Gex::SiteTestResults::SITESTATE_DPAT:
                    if(lSite->WaitForEarlyTuning())
                        lColor = QString(" background-color: lime;");
                    else
                        lColor = QString(" background-color: green;");
                    break;
            }
            if(lRow%2==0)
                mSitesTabString += QString("<tr>");
            mSitesTabString += QString("<td style=\"width: 8%; text-align: left; vertical-align: top;%1\">").arg(lColor);
            mSitesTabString += QString("Site<br>%1").arg(lSite->SiteNb());
            mSitesTabString += QString("</td>");
            */
            // first col
                /*
                mSitesTabString += QString("<td style=\"width: 14%;\">");
                    mSitesTabString += QString("<span style=\"text-decoration:underline;\">Info:</span><br>");
                    // SiteStateName() is Mutex locked: could slow down if called too much
                    //mSitesTabString += QString("State=%1<br>").arg(lSite->SiteStateName()); // Mutex locked !
                    mSitesTabString += QString("Baselines=%1<br>").arg(lSite->BaselineCount());
                    mSitesTabString += QString("Tunings=%1<br>").arg(lSite->TuningCount());
                    mSitesTabString += QString("<span style=\"text-decoration:underline;\">Rolling Buffer:</span><br>");
                    mSitesTabString += QString("Size=%1<br>").arg(lSite->RBSize());
                    mSitesTabString += QString("Index=%1<br>").arg(lSite->RBCurIndex());
                    mSitesTabString += QString("ValidParts=%1<br>").arg(lSite->RBValidParts());
                mSitesTabString += QString("</td>");
                */

            // second col
                /*
                mSitesTabString += QString("<td style=\"width: 14%;\">");
                    mSitesTabString += QString("<span style=\"text-decoration:underline;\">Lot:</span><br>");
                    mSitesTabString += QString("Parts=%1<br>").arg(lSite->LotParts().TestedParts());
                    mSitesTabString += QString("Pass=%1<br>").arg(lSite->LotParts().PassParts());
                    mSitesTabString += QString("Fail=%1<br>").arg(lSite->LotParts().TestedParts() - lSite->LotParts().PassParts());
                    mSitesTabString += QString("RealOutliers=%1<br>").arg(lSite->LotParts().RealOutlierParts());
                    mSitesTabString += QString("VirtualOutliers=%1<br>").arg(lSite->LotParts().VirtualOutlierParts());
                    // LastPart() is currently doing a copy of the class PartResult... not good but multithread safer ?
                    // Let's create a reusable PartResult copy to optimize
                    static GS::Gex::PartResult lLastPart;
                    lLastPart=lSite->LastPart();
                    //mSitesTabString += QString("Last PartID=%1<br>").arg( lSite->LastPart().PartID() ); // lLastPartResult.PartID() );
                    mSitesTabString += QString("Last PartID=%1<br>").arg( lLastPart.PartID() );
                    //mSitesTabString += QString("Last PartIndex=%1<br>").arg( lSite->LastPart().PartIndex() ); //lSite->LastPart().PartIndex()); or lLastPartResult
                    mSitesTabString += QString("Last PartIndex=%1<br>").arg( lLastPart.PartIndex() );
                mSitesTabString += QString("</td>");
                */

            // third col
            /*
                mSitesTabString += QString("<td style=\"width: 14%;\">");
                    mSitesTabString += QString("<span style=\"text-decoration:underline;\">SinceLastLimits:</span><br>");
                    //todo: use 1 ref for all called functions ?
                    // GS::Gex::PartCounters& lPartsCounter=lSite->SLLParts();
                    mSitesTabString += QString("Parts=%1<br>").arg(lSite->SLLParts().TestedParts());
                    mSitesTabString += QString("Pass=%1<br>").arg(lSite->SLLParts().PassParts());
                    mSitesTabString += QString("Fail=%1<br>").arg(lSite->SLLParts().TestedParts() - lSite->SLLParts().PassParts());
                    mSitesTabString += QString("RealOutliers=%1<br>").arg(lSite->SLLParts().RealOutlierParts());
                    mSitesTabString += QString("VirtualOutliers=%1<br>").arg(lSite->SLLParts().VirtualOutlierParts());
                mSitesTabString += QString("</td>");


            if(lRow%2!=0)
                mSitesTabString += QString("</tr>");
            ++lRow;
            */
        //} // if Site
    }

    //if(lRow>0 && lRow%2==0)
        //mSitesTabString += QString("</tr>");

    //mSitesTabString += QString("</tbody></table>");

    // Let s comment that to check if it is what is slowing down everything.
    //textEditDebug->setText(mSitesTabString); // todo: use setHtml() instead ?
    //textEditDebug->setHtml(mSitesTabString);
}

///////////////////////////////////////////////////////////
// Refresh BINNING tab contents
void Gtm_TesterWindow::ReloadBinningTab(void)
{
    if (mClientNodeWP.toStrongRef().isNull())
        return;

    GS::Gex::ClientNode* lClientNode=mClientNodeWP.toStrongRef().data();

    QTreeWidgetItem* twi=0;
    while ( (twi=treeWidgetBinning->takeTopLevelItem(0))!=0)
    {
        //treeWidgetBinning->removeItemWidget(twi, 0);
        if (twi)
            delete twi;
        //mTreeWidgetItemsBuffer.append(twi);
    }

    // Empty list prior to rebuild it.
    // Bernard : are you sure it deletes all previous QTreeWidgetItem ?
    // William : should be : "Note: Since each item is removed from the tree widget before being deleted"
    treeWidgetBinning->clear();

    // If no data available yet, simply exit!
    // todo : QMutexLocker TotalTestedParts() ?
    int lTotalTestedParts=lClientNode->mStation.cTestData.mSites.TotalTestedParts();
    if( lTotalTestedParts <= 0)
        return;

    // Fill list with current binning results
    QStringList row;
    QMap<int,int>::Iterator it;
    int	iBinCount=0;
    double	lfPercentage=0.0;
    for ( it = lClientNode->mStation.cTestData.mSoftBinSummary.begin();
          it != lClientNode->mStation.cTestData.mSoftBinSummary.end(); ++it )
    {
        // Get Bin count and yield
        iBinCount = it.value(); //iBinCount = it.data(); // Qt3
        lfPercentage = (100.0*iBinCount)/(double)lTotalTestedParts;

        // Fill columns: binning name | yield | total parts
        row.clear();
        row << QString("Bin %1").arg(it.key());
        row << QString::number(lfPercentage,'f',2) + "%";
        row << QString::number(iBinCount);

        // Insert row into tree widget.
        twi=new QTreeWidgetItem(treeWidgetBinning, row);
        //mTreeWidgetItemsBuffer.append(twi);
    }

    // Display cumul of all bins (all parts)
    row.clear();
    row << "All bins" << "100.00%" << QString::number(lTotalTestedParts);
    twi=new QTreeWidgetItem(treeWidgetBinning, row);
        //mTreeWidgetItemsBuffer.append(twi);
}

// Avoids user to manually close station on 'OK' signal
void Gtm_TesterWindow::accept()
{
}

void Gtm_TesterWindow::SetupSitesTable(CStation* lStation)
{
    GSLOG(5, QString("Setting up a Sites GUI for %1 site(s)....")
          .arg(lStation->cTestData.mSites.size()).toLatin1().data() );
    mSitesTableWidget.setColumnCount(lStation->cTestData.mSites.size());
    GS::Gex::SiteList::Iterator it = lStation->cTestData.mSites.begin();
    QStringList lSitesNumbers;
    //lSitesNumbers.append("Site");
    for(it=lStation->cTestData.mSites.begin(); it!=lStation->cTestData.mSites.end(); ++it)
        if (it.value())
        {
            mSitesNbToColIndex.insert(it.value()->SiteNb(), lSitesNumbers.size());
            lSitesNumbers.append(QString::number(it.value()->SiteNb()));
        }
        else
        {
            GSLOG(4, "Null site found. Abnormal.");
        }
    mSitesTableWidget.setHorizontalHeaderLabels(lSitesNumbers);

    QStringList lRowsLabels;
    lRowsLabels<<"State"<<"Baselines"<<"Tunings";
    lRowsLabels<<"RollingBuffer Size"<<"RollingBuffer Index"<<"RollingBuffer ValidParts";
    lRowsLabels<<"Lot Parts"<<"Lot Pass"<<"Lot Fail"<<"Lot RealOutliers"<<"Lot VirtualOutliers";
    lRowsLabels<<"Last part ID"<<"Last part index";
    lRowsLabels<<"Since last limits Parts"<<"Since last limits Pass"<<"Since last limits Fail"
              <<"Since last limits RealOutliers"<<"Since last limits VirtualOutliers";
    mSitesTableWidget.setRowCount(lRowsLabels.size()); // 18 ?
    mSitesTableWidget.setVerticalHeaderLabels(lRowsLabels);

    for (int i=0; i<lStation->cTestData.mSites.size(); i++)
    {
        for (int j=0; j<lRowsLabels.size(); j++)
        {
            // Do we have to keep all items in order to delete it later ?
            QTableWidgetItem *lNewItem=new QTableWidgetItem("?");
            mSitesTableWidgetItems.append(lNewItem);
            mSitesTableWidget.setItem(j, i, lNewItem);
        }
    }
    SitesTab->update(); // needed ?
    mSitesTableWidget.repaint();
}

void Gtm_TesterWindow::OnClientInit(CStation* lStation)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("On ClientInit station %1").arg(lStation?lStation->GetStationNb():-1).toLatin1().data() );
    if (!lStation)
    {
        GSLOG(4, "OnClientInit impossible: null station");
        return;
    }

    QString strMessage = "Station " + QString::number(lStation->GetStationNb());
    strMessage += ": " + lStation->Get(CStationInfo::sJobNamePropName).toString();

    setWindowTitle(strMessage); // Qt3: setCaption(strMessage);
    // Update GUI
    textGlobalInfo->setText(QString("Station: %1, Splitlot: %2, RetestIndex: %3")
                            .arg(lStation->GetStationNb())
                            .arg(lStation->Get(CStationInfo::sSplitlotIDPropName).toInt())
                            .arg(lStation->Get(CStationInfo::sRetestIndexPropName).toInt()));
    textProduct->setText(lStation->Get(CStationInfo::sProductPropName).toString());
    textLot->setText(lStation->Get(CStationInfo::sLotPropName).toString());
    textTesterType->setText(lStation->Get(CStationInfo::sTesterTypePropName).toString());
    textTesterName->setText(lStation->Get(CStationInfo::sTesterNamePropName).toString());

    // Update sites widget if we have sites
    if (lStation->cTestData.mSites.size()>0)
        SetupSitesTable(lStation);
}

void Gtm_TesterWindow::reject()
{
    GSLOG(5, "Test window reject");
}

void Gtm_TesterWindow::OnTabChange(int index)
{
    switch(index)
    {
        case 0: // 'Status' tab
            ReloadStatusTab();
            break;

        case 1: // 'Binning' tab
            ReloadBinningTab();
            break;

        case 4: // 'Tests' tab
            ReloadTestsTab();
            break;

        default:
            break;
    }
}

// Refresh STATUS tab contents
void Gtm_TesterWindow::ReloadStatusTab(void)
{
}

void Gtm_TesterWindow::OnDetachWindow()
{
    // If only one station left attached to GTM, do not accept to detach anymore.
    if(pGexMainWindow->mGtmWidget->TotalStations(true) <= 1 && (bChildWindow))
        return;

    if(bChildWindow)
        bChildWindow = false;
    else
        bChildWindow = true;
    buttonDetachWindow->setChecked(bChildWindow); //or setEnabled(bChildWindow); ? //was setOn(bChildWindow); (Qt3)

    if(bChildWindow)
    {
        // Re-attach dialog box to Examinator's scroll view Widget
        //reparent(pGexMainWindow->mGtmWidget, // Qt3
        setParent(pGexMainWindow->mGtmWidget,
          //Qt::WStyle_Customize | Qt::WStyle_NormalBorder | Qt::WStyle_SysMenu | Qt::WStyle_MinMax | Qt::WStyle_Title,
          Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint | Qt::WindowTitleHint
          //QPoint(0,0),true);
        );
        move(QPoint(0,0));
        QString r=pGexMainWindow->mGtmWidget->AttachTester(this);
        if (r.startsWith("err"))
            GSLOG(3, QString("Failed to attach tester: %1").arg(r).toLatin1().data() );
    }
    else
    {
        // Accept to detach window.
        //reparent(NULL,
        setParent(NULL,
          //Qt::WStyle_Customize | Qt::WStyle_NormalBorder | Qt::WStyle_SysMenu | Qt::WStyle_MinMax | Qt::WStyle_Title,
          Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint | Qt::WindowTitleHint
          //QPoint(100,100),true);
                );
        move(QPoint(100,100));
        pGexMainWindow->mGtmWidget->DetachTester(this);
    }

    // Force repaint
    update();

    // Have GTM windows resized as detach/attach has affected individual child window sizes.
    pGexMainWindow->mGtmWidget->OnRefreshAllDialogs();
}

void Gtm_TesterWindow::RefreshGUI()
{
    if (mClientNodeWP.toStrongRef().isNull())
        return;

    // Repaint Histogram chart
    frameHistoChart->update();

    // Make sure different GUI tabs are refreshed
    ReloadStatusTab(); // empty ?
    ReloadBinningTab();
    ReloadTestsTab();

    ReloadSitesTab(); // too slow. Fixing, fixing... William.

    //return;


    GS::Gex::ClientNode* lClientNode=mClientNodeWP.toStrongRef().data();
    if (!lClientNode)
        return;
    // Update global yield
    double lfYieldLevel=0;
    if(lClientNode->mStation.cTestData.mSites.TotalTestedParts() > 0)
        lfYieldLevel = ((double)lClientNode->mStation.cTestData.mSites.TotalPassParts()*100.0)
                / lClientNode->mStation.cTestData.mSites.TotalTestedParts();
    else
        lfYieldLevel = 100.0;

    QString strYieldColor;
    if(lfYieldLevel < lClientNode->mStation.GetPatOptions().mFT_YieldLevel)
        strYieldColor = "#C60000";	// Bright RED
    else
        strYieldColor = "#00C600";	// Dark green

    QString strYield = QString("<font color=\"%1\"><p align=\"center\">Yield<br><b>").arg(strYieldColor);
    strYield += QString("<h1>%1%</h1></b></p></font>").arg(lfYieldLevel,0,'f',2);
    textLabelYield->setText(strYield);

    // If tester requested to no longer receive alarm notification, return now!
    // (eg: if flushing last few runs when issuing end-of-lot)
    if(lClientNode->GetSilentMode())    //m_uiSilentMode)
        return;

    // Refresh 'Status' tab: display updated Histogram bars...unless alarms raised
    if(lClientNode && lClientNode->mStation.cTestData.mErrorLevel <= GTM_ERRORTYPE_INFO)
        ReloadStatusTab();

    return;
}

void Gtm_TesterWindow::ReloadTestsTab(void)
{
    if (mClientNodeWP.toStrongRef().isNull())
        return;

    GS::Gex::ClientNode* lClientNode=mClientNodeWP.toStrongRef().data();
    if (!lClientNode)
        return;
    if (lClientNode->GetSocketInstance()==-1)
        return;

    QTreeWidgetItem* twi=0;
    while ( (twi=treeWidgetTests->takeTopLevelItem(0))!=0)
    {
        //treeWidgetBinning->removeItemWidget(twi, 0);
        if (twi)
            delete twi;
        //mTreeWidgetItemsBuffer.append(twi);
    }

    // Empty list prior to rebuild it.
    treeWidgetTests->clear();

    QString	strCpkAlarm;
    QString	strCpk;
    QStringList row;
    int	iSite=0;
    int iSeverityLimits = 0;
    GS::Gex::SiteTestResults *pSite=0;
    CPatDefinition *ptPatDef=0;
    CTest *ptTestCell=0;
    GS::Gex::SiteList::Iterator it = lClientNode->mStation.cTestData.mSites.begin();
    while(it != lClientNode->mStation.cTestData.mSites.end())
    {
        // Get valid site# used and site ptr.
        iSite = it.key();
        pSite = it.value(); // 6935: use value, instead of []
        if(pSite == NULL)
            goto next_site;

        // Save test data to buffer.
        ptTestCell = pSite->TestList();
        while(ptTestCell != NULL)
        {
            // Get handle to PAT definition for this test
            ptPatDef = lClientNode->mStation.GetPatDefinition(ptTestCell->lTestNumber,
                                                              ptTestCell->lPinmapIndex,
                                                              ptTestCell->strTestName);

            // Only include tests that are PAT enabled or Cpk/Monitoring enabled....
            if(ptPatDef == NULL || ptPatDef->IsTestDisabled())
                goto next_test_cell;

            iSeverityLimits = ptPatDef->m_iOutlierLimitsSet;

            // samples sent by the tester (ie after buffer reset)
            // Insert Test info into List view.
            if(ptPatDef->m_SPC_CpkAlarm <= 0.0)
                strCpkAlarm = "-";
            else
                strCpkAlarm = QString::number(ptPatDef->m_SPC_CpkAlarm,'f',2);

            if(ptTestCell->GetCurrentLimitItem()->lfCpk == C_NO_CP_CPK)
                strCpk = "-";
            else
            if(fabs(ptTestCell->GetCurrentLimitItem()->lfCpk) >= 10000.0)
                strCpk = QString::number(ptTestCell->GetCurrentLimitItem()->lfCpk,'g',0);
            else
                strCpk = QString::number(ptTestCell->GetCurrentLimitItem()->lfCpk,'f',2);

            // Add new row
            row.clear();
            row << QString::number(ptTestCell->lTestNumber);
            row << ptPatDef->m_strTestName;
            row << QString::number(iSite);
            row << strCpk;
            // Column 'Alarm level' removed for now
            //row << strCpkAlarm;
            row << QString::number(ptTestCell->ldSamplesValidExecs);
            row << QString::number(ptTestCell->lPinmapIndex);
            row << QString::number(ptPatDef->m_lfLowStaticLimit);
            row << QString::number(ptPatDef->m_lfHighStaticLimit);
            row << QString::number(ptPatDef->mDynamicLimits[iSite].mLowDynamicLimit1[iSeverityLimits]);
            row << QString::number(ptPatDef->mDynamicLimits[iSite].mHighDynamicLimit1[iSeverityLimits]);
            row << QString::number(ptPatDef->mDynamicLimits[iSite].mLowDynamicLimit2[iSeverityLimits]);
            row << QString::number(ptPatDef->mDynamicLimits[iSite].mHighDynamicLimit2[iSeverityLimits]);
            new QTreeWidgetItem(treeWidgetTests, row);

next_test_cell:
            // Move to next test
            ptTestCell = ptTestCell->GetNextTest();
        };
        // Move to next site
next_site:
        ++it;
    };
}

void Gtm_TesterWindow::OnWarningOk()
{
    if (mClientNodeWP.toStrongRef().isNull())
        return;

    // Reset message priority level to lowest (so any futur message is displayed)
    mClientNodeWP.data()->mStation.cTestData.mErrorLevel = GTM_ERRORTYPE_INFO;

    // Switch back to the Yield / Histogram view.
    stackedWidget_Status->setCurrentIndex(0);
}

void Gtm_TesterWindow::OnStopOk()
{
    if (mClientNodeWP.toStrongRef().isNull())
        return;

    // Reset message priority level to lowest (so any futur message is displayed)
    mClientNodeWP.data()->mStation.cTestData.mErrorLevel = GTM_ERRORTYPE_INFO;

    // Switch back to the Yield / Histogram view.
    stackedWidget_Status->setCurrentIndex(0);
}

void Gtm_TesterWindow::OnClientAboutToClose()
{
    GS::Gex::ClientNode* lCN=qobject_cast<GS::Gex::ClientNode*>(sender());

    GSLOG(5, QString("Client '%1' about to close...").arg(lCN?lCN->objectName():"?").toLatin1().constData() );

    setProperty(sClientAboutToCloseProp, true);
    //QObject::disconnect(this, 0, 0, 0);
    setEnabled(false);

    //if (!lCN)
        ((Gtm_MainWidget*)parent())->OnCloseClientNode(mClientNodeWP);
}

void Gtm_TesterWindow::OnClientDestroyed()
{
    GSLOG(5, "ClientNode destroyed...");
    setProperty(sClientAboutToCloseProp, true);
    ((Gtm_MainWidget*)parent())->OnCloseClientNode(mClientNodeWP);
}

///////////////////////////////////////////////////////////
// Reset PAT (eg: starting new lot)
void Gtm_TesterWindow::OnResetMessage(bool bCheckPassword)
{
    GSLOG(6, "TesterWindow Reset...");

    // Request password
    if(bCheckPassword)
    {
        if(!checkValidPassword())
            return;
    }

    // Reset the histogram bars
    frameHistoChart->clear();

    // Clear Yield message
    textLabelYield->clear();

    // Refresh GUI.
    ReloadStatusTab();
}

void Gtm_TesterWindow::OnNewBinResult(int /*lPatBin*/, int lSoftBin)
{
    // Do we have to processEvents now or at the end of the function or not at all ?
    //QCoreApplication::processEvents(QEventLoop::ExcludeSocketNotifiers);

    if (mClientNodeWP.toStrongRef().isNull())
        return;

    GS::Gex::ClientNode* lClientNode=mClientNodeWP.toStrongRef().data();
    if (!lClientNode)
        return;

    //QMutex lMutex;
    //lMutex.lock();
    if (!GS::Gex::ClientNode::IsClientRegistered(lClientNode)) // probably already deleted...
    {
        GSLOG(3, QString("Client node unregistered...").toLatin1().data() );
        return;
    }

    if (property(sClientAboutToCloseProp).toBool()==true)
        return;

    //QMutex lMutex;
    //lMutex.lock();
    if (lClientNode->property("ClientAboutToClose").toBool()==true)
    {
        //lMutex.unlock();
        return;
    }
    //lMutex.unlock();

    //GSLOG(SYSLOG_SEV_DEBUG, QString("On NewBinResult bin %1").arg(lSoftBin).toLatin1().data() );

    //if (mClientNode->GetSocketInstance()==-1) // even the socket is not accessible
    //if (this->)
    //{
    //    GSLOG(4, "On new bin result: bad socket: no refresh...");
    //    return;
    //}

    // Update Histogram structures
    if(lClientNode->mStation.GetPatInfo()->GetRecipeOptions().pGoodSoftBinsList->Contains(lSoftBin))
    {
        // Update Histogram structures
        // Buffer holding last XX Histogram bars of Bin1
        frameHistoChart->iHistoCount[frameHistoChart->iHistoBarInUse]++;
    }

    // Check if need to scroll histo bars to free last bar (scroll to the left)
    if(lClientNode->mStation.cTestData.mSites.TotalTestedParts() % GTM_SPC_DATA_IN_CELL == 0)
    {
        // Total parts per histogram bar reached, check if need to scroll one bar (lose the oldest, and shift them all)
        if(frameHistoChart->iHistoBarInUse < GTM_SPC_HISTO_CELLS-1)
            frameHistoChart->iHistoBarInUse++;	// Not the last bar yet, so no scroll required yet
        else
        {
            // All histogram bars filled, need to scroll to the left!
            int iIndex=1;
            for(iIndex=1; iIndex < GTM_SPC_HISTO_CELLS; iIndex++)
                frameHistoChart->iHistoCount[iIndex-1] = frameHistoChart->iHistoCount[iIndex];
            // Empty last histogram bar.
            frameHistoChart->iHistoCount[GTM_SPC_HISTO_CELLS-1] = 0;
        }
    }
}

void Gtm_TesterWindow::OnNewMessage(int type, QString lMessage, int /*lSeverity*/ )
{
    GSLOG(6, lMessage.toLatin1().data());

    QString strError;
    QDateTime lDateTime = QDateTime::currentDateTime();	// Current time.
    QString	strDate = lDateTime.toString("[d MMM h:mm] ");	// 9 Jan HH:MM
    switch(type)
    {
        case GTM_ERRORTYPE_WARNING:
            strError = strDate + "Warning: " + lMessage;
            labelWarning->setText(strError);
            break;
        case GTM_ERRORTYPE_CRITICAL:
            strError = strDate + "Error: " + lMessage;
            labelStop->setText(strError);
            break;
        default:
            strError=strDate + lMessage;
        break;
    }

    labelStatus->setText(lMessage);
    // Add message to Log file and make sure log window shown the latest messages (bottom)
    textEditLog->append(strError.replace('\n',' ')); // + QString(" (severity %1)").arg(lSeverity));

    //  stackedWidget_Status->setCurrentIndex(iWidgetStackID);
}

void Gtm_TesterWindow::OnResetMessage()
{
    GSLOG(6, "OnResetMessage");
    OnResetMessage(false);
}

bool Gtm_TesterWindow::checkValidPassword()
{
    bool bOk;
    QString text;
    QString strApplicationName = GS::Gex::Engine::GetInstance().Get("AppFullName").toString();

    // Qt3
    //text = QInputDialog::getText(
    //    "Admin Password", "Enter password:", QLineEdit::Password,
    //    QString::null, &bOk, this );
    text = QInputDialog::getText(this, "Admin Password", "Enter password:", QLineEdit::Password, QString::null, &bOk);

    // If escape key hit, quietly return
    if(!bOk)
        return false;

    // Correct password entered!
    if(text == "admin")
        return true;

    // Incorrect password entered
    GS::Gex::Message::information(strApplicationName, "Invalid password!");
    return false;
}

void Gtm_TesterWindow::OnLoadPatLimitsFromfile()
{
    GSLOG(5, "Gtm On Load Pat Limits From file...");
    // Request password
    if(!checkValidPassword())
        return;

    QString strPatLimits;

    // Qt3
    //strPatLimits = QFileDialog::getOpenFileName(
    //            "",
    //            "PAT Limits file *.plf",
    //            this,
    //            "PAT",
    //            "Load PAT Limits file");
    strPatLimits = QFileDialog::getOpenFileName(
      (QWidget*)this, QString("Load PAT Limits file"), QString(""), QString("PAT Limits file *.plf") );

    if(strPatLimits.isEmpty() == true)
        return;
}
#endif
