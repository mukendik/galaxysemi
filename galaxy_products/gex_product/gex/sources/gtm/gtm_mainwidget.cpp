
#ifdef GCORE15334

#include <QApplication>
#include <QObject>
#include <QScriptEngine>
#include <QSharedPointer>

#include <gqtl_log.h>
#include <gqtl_sysutils.h>
/// ONLY While debugging, so to display the PAT dialog box!
#include <classes.h>
#include <ctest.h>
#include <patman_lib.h>

#include "browser_dialog.h"
#include "gtm_mainwidget.h"
#include "gtm_testerwindow.h"
#include "gs_qa_dump.h"
#include "gex_scriptengine.h"
#include "clientnode.h"
#include "clientsocket.h"
#include "clientthread.h"
#include "engine.h"
#include "testerserver.h"

//#include "DebugMemory.h" // must be the last include


// global variables
extern GexMainwindow *pGexMainWindow;
extern GexScriptEngine* pGexScriptEngine;

#define GTM_PLAY_LABEL "Play"
#define GTM_STOP_LABEL "Stop"

Gtm_MainWidget::Gtm_MainWidget(QWidget* parent/*=0*/, Qt::WindowFlags fl/*=0*/)
    : QWidget(parent, fl)
{
    // Setup UI
    //setupUi(this);

    //Gtm_MainWidget_baseLayout = new QHBoxLayout(this, 11, 6, "Gtm_MainWidget_baseLayout"); // Qt3
    Gtm_MainWidget_baseLayout = new QHBoxLayout(this);
    // 11 = border : Qt4 ?
    Gtm_MainWidget_baseLayout->setSpacing(6);  // 6 = spacing : Qt4 ?
    Gtm_MainWidget_baseLayout->setObjectName("Gtm_MainWidget_baseLayout");
    //Gtm_MainWidget_baseLayout->setContentsMargins(); // is it the way to set border ?

    /*
        QAction* lActionButtonPlayStop = new QAction(this);
        lActionButtonPlayStop->setObjectName(QString::fromUtf8("actionPlayStop"));
        //lActionButtonPlayStop->setProperty("name", QVariant(QCoreApplication::translate(
                  //                                                       "GexMainWindow",
                    //                                                     "actionConnect",
                      //                                                   0,
                        //                                                 QCoreApplication::UnicodeUTF8)));
        QIcon icon;
        //icon.addFile("./images/icon-report.png", QSize(), QIcon::Normal, QIcon::Off);
        icon.addFile(QString::fromUtf8(":/gex/icons/icon-pause.png"), QSize(), QIcon::Normal, QIcon::Off);
        lActionButtonPlayStop->setIcon(icon);
        pGexMainWindow->toolBarButton->addSeparator();
        pGexMainWindow->toolBarButton->addAction(lActionButtonPlayStop);
    */

    // 7246
    static QIcon icon;
    //icon.addFile("./images/icon-report.png", QSize(), QIcon::Normal, QIcon::Off);
    icon.addFile(QString::fromUtf8(":/gex/icons/stop_service.png"), QSize(), QIcon::Normal, QIcon::Off);
    // start_service.png pause.png stop_service.png
    mPlayStopButton.setParent(pGexMainWindow->centralWidget());
    mPlayStopButton.setIcon(icon); //=new QPushButton(icon, "Stop", pGexMainWindow->centralWidget());
    mPlayStopButton.setText(GTM_STOP_LABEL);
    // Expanding : max size, Minimum:max size
    // Fixed : enough for Stop but will it resize ?
    // Max : small : enough for the text ?
    mPlayStopButton.setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    //pGexMainWindow->layout()->addWidget( );
    // added at the bottom of the main window
    //pGexMainWindow->centralWidget()->layout()->addWidget(&mPlayStopButton);
    QGridLayout* lGL=(QGridLayout*)pGexMainWindow->centralWidget()->layout();
    // 0 0 : over the top bar : horrible
    // 0 1 : at right in a dedicated row: horrible
    // 1 0 : at left, over the testerwindow
    //lGL->addWidget(&mPlayStopButton, 0, 0, Qt::AlignHCenter); // in the center of the HtlmTop (not perfect)
    lGL->addWidget(&mPlayStopButton, 1, 0, Qt::AlignHCenter|Qt::AlignTop); // in the center of the HtlmTop (not perfect)

    if (!QObject::connect(&mPlayStopButton, SIGNAL(clicked()), this, SLOT(OnPlayStopButtonPressed())) )
        GSLOG(SYSLOG_SEV_ERROR, "Connect PlayStop button clicked signal failed");

    if (!QObject::connect(&GS::Gex::Engine::GetInstance().GetTesterServer(),    SIGNAL(sStatusChanged(bool)),
                          this, SLOT(OnUpdatePlayStopButton(bool))))
        GSLOG(SYSLOG_SEV_WARNING, "Failed to connect signal emitted by TesterServer");

    // Let's register this to the scriptengine
    QScriptValue lSV = pGexScriptEngine->newQObject( this );
    if (!lSV.isNull())
        pGexScriptEngine->globalObject().setProperty("GSGTMMainWidget", lSV);

    if (!QObject::connect(
                &(GS::Gex::Engine::GetInstance().GetTesterServer()),
                SIGNAL(sNewClient(QWeakPointer<GS::Gex::ClientThread>)),
                this,
                SLOT(OnNewClient(QWeakPointer<GS::Gex::ClientThread>)) ))
        GSLOG(3, "Cannot connect NewClient signal");
}

Gtm_MainWidget::~Gtm_MainWidget()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Gtm_MainWidget::~Gtm_MainWidget");

    /*
        QMap<QString, QVariant> lCounts;
        CGexSystemUtils::GetQObjectsCount(*this, lCounts, true);
        foreach(QString k, lCounts.keys())
            GSLOG(7, k+":"+QString::number(lCounts[k].toInt()) );
    */

    if (Gtm_MainWidget_baseLayout)
        delete(Gtm_MainWidget_baseLayout); // ?
}

void Gtm_MainWidget::OnPlayStopButtonPressed()
{
    GSLOG(6, "On PlayStop Button Pressed...");

    if (mPlayStopButton.text()==GTM_PLAY_LABEL)
    {
        GS::Gex::Engine::GetInstance().GetTesterServer().Resume();
        return;
    }

    if (mClients.size()==0)
    {
        GS::Gex::Engine::GetInstance().GetTesterServer().Pause();
        return;
    }

    QDialog lD(this);
        lD.setWindowTitle(" "); //lD.setCaption(" ");
        lD.setModal(true);
        lD.setLayout(new QVBoxLayout(&lD));
    QLabel lQuestion("What would you like to do ?", &lD);
    lQuestion.setAlignment(Qt::AlignHCenter);
    lD.layout()->addWidget(&lQuestion);
    QPushButton lWB("Wait for current clients to finish and refuse any new one...", &lD);
        connect(&lWB, SIGNAL(clicked()), &lWB, SLOT(hide())); // 1
        connect(&lWB, SIGNAL(clicked()), &lD, SLOT(accept())); // 1
    lD.layout()->addWidget(&lWB);
    QPushButton lKB("Kill all current clients", &lD);
        connect(&lKB, SIGNAL(clicked()), &lKB, SLOT(hide()) );
        connect(&lKB, SIGNAL(clicked()), &lD, SLOT(accept())); // reject=0 close=0
    lD.layout()->addWidget(&lKB);
    QPushButton lCB("Cancel.", &lD);
    connect(&lCB, SIGNAL(clicked()), &lD, SLOT(reject()));
    lD.layout()->addWidget(&lCB);
    lD.show();
    // if the user clicks the close small button, exec returns 0...
    int lR=lD.exec();
    GSLOG(5, QString("User selected %1: WB hidden:%2 KB hidden:%3").arg(lR).arg(lWB.isHidden()).arg(lKB.isHidden())
          .toLatin1().data() );

    if (lR==0)
        return;

    // Wait for all clients to finish, or force exit
    // Let's swap to pause mode and make sure no more connections are accepted
    GS::Gex::Engine::GetInstance().GetTesterServer().Pause();
    mPlayStopButton.setText(GTM_PLAY_LABEL);
    static QIcon icon;
    icon.addFile(QString::fromUtf8(":/gex/icons/start_service.png"), QSize(), QIcon::Normal, QIcon::Off);
    mPlayStopButton.setIcon(icon);

    // If kill selected, force exit
    if (lKB.isHidden()) // && mPlayStopButton.text()=="Stop")
    {
        // refuse new clients ?
        //GS::Gex::Engine::GetInstance().GetTesterServer().Pause();
        // Kill all current connections if any
        GS::Gex::Engine::GetInstance().GetTesterServer().OnExit();
        // According to 7246 functionnal spec we dont have to exit now.
        //GS::Gex::Engine::GetInstance().Exit(0);
    }
}

void Gtm_MainWidget::OnUpdatePlayStopButton(bool lRunning)
{
    QIcon   lIcon;
    QString lLabel;

    if (lRunning == true)
    {
        lLabel = GTM_STOP_LABEL;
        lIcon.addFile(QString::fromUtf8(":/gex/icons/stop_service.png"), QSize(), QIcon::Normal, QIcon::Off);
    }
    else
    {
        lLabel = GTM_PLAY_LABEL;
        lIcon.addFile(QString::fromUtf8(":/gex/icons/start_service.png"), QSize(), QIcon::Normal, QIcon::Off);
    }

    mPlayStopButton.setText(lLabel);
    mPlayStopButton.setIcon(lIcon);
}

void Gtm_MainWidget::OnNewClient( QWeakPointer<GS::Gex::ClientThread> lCTWP)
{
    if (lCTWP.isNull())
        return;

    //QWeakPointer<GS::Gex::ClientThread> lCTWP(lCT);
    QSharedPointer<GS::Gex::ClientThread> lCTSP(lCTWP);
    //if (lCTWP.toStrongRef().isNull())
    if (lCTSP.isNull())
        return;
    if (!lCTWP.data())
        return;

    GS::Gex::ClientThread* lClientThread=lCTWP.data();

    GSLOG(6, QString("New client : on socket %1").arg(lClientThread->mSocketDescriptor).toLatin1().data() );

    //QWeakPointer lClientThreadWeakPointer(lClientThreadSharedPointer);

    if (!lClientThread)
    {
        GSLOG(3, "CLientThread null");
        return;
    }

    if (!QObject::connect(lClientThread, SIGNAL(sNewClient(QWeakPointer<GS::Gex::ClientNode>)),
           this, SLOT(OnNewClientNode(QWeakPointer<GS::Gex::ClientNode>))))
    {
        GSLOG(3, "Failed to connect NewclientNode to OnNewClientNode");
        GS::Gex::Engine::GetInstance().Exit(-100);
        return;
    }

    if (!QObject::connect(lClientThread, SIGNAL(sCloseClient( QWeakPointer<GS::Gex::ClientNode>)),
           this, SLOT(OnCloseClientNode( QWeakPointer<GS::Gex::ClientNode>))))
        GSLOG(3, "Failed to connect OnCloseClientNode");

    //if (!QObject::connect(lClientThread, SIGNAL(destroyed()), this, SLOT(OnCloseClientNode( ))) )
      //  GSLOG(3, "Failed to connect destroyed");

    return;
}

void Gtm_MainWidget::OnNewClientNode(QWeakPointer<GS::Gex::ClientNode> lCNWP)
{
    if (lCNWP.toStrongRef().isNull())
    {
        GSLOG(4, "Cannot strongify weak pointer on ClientNode");
        return ;
    }

    if (!lCNWP.data())
    {
        GSLOG(3, "ClientNode null");
        return;
    }

    GS::Gex::ClientNode* lCN=lCNWP.data();

    if (mClients.contains(lCN))
    {
        GSLOG(3, "This client is already registered by GTM MainWidget.");
        return;
    }

    GSLOG(5, QString("Creating a new TesterWindow for client %1").arg(lCN->objectName()).toLatin1().data() );

    Gtm_TesterWindow* lTesterWindow = new Gtm_TesterWindow(lCNWP, this);

    mClients.insert(lCN, lTesterWindow);

    // Qt3
    //lTesterWindow->reparent(this,
     // Qt::WStyle_Customize | Qt::WStyle_NormalBorder | Qt::WStyle_SysMenu | Qt::WStyle_MinMax | Qt::WStyle_Title,
     // QPoint(0,0),true);
    lTesterWindow->setParent(this,
      Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint | Qt::WindowTitleHint);

    QString r=AttachTester(lTesterWindow);
    if (!r.startsWith("ok"))
        GSLOG(3, QString("AttachTester failed: %1").arg(r).toLatin1().data() );

    // Hide on creation, will be shown later
    //lTesterWindow->hide();

    GSLOG(5, "On New ClientNode end" );
}

void Gtm_MainWidget::OnClientThreadDestroyed()
{
    GS::Gex::ClientThread* lCT = qobject_cast<GS::Gex::ClientThread*>(sender());
    if (!lCT)
    {
        GSLOG(5, "ClientThread destrroyed but unidentifiable");
    }
    else
    {
        GSLOG(5, QString("ClientThread '%1' destroyed ").arg(lCT->objectName()).toLatin1().constData() );
    }
}

void Gtm_MainWidget::OnCloseClientNode( QWeakPointer<GS::Gex::ClientNode> lCNWP)
{
    if (lCNWP.toStrongRef().isNull())
    {
        GSLOG(5, "Cannot strongify WeakPointer...");
        //return;
    }

    if (!lCNWP.data())
    {
        GSLOG(5, "Cannot guess which ClientNode because ClientNode pointer null");
        //return;
    }

    //if (!GS::Gex::ClientNode::IsClientRegistered(cn))
      //  return;

    if ( lCNWP.data() && mClients.contains(lCNWP.data())==false)
    {
        GSLOG(4, "This client is unfindable in GUI");
        return;
    }

    //GSLOG(5, QString("Closing client of socket %1").arg(cn->GetSocketInstance()).toLatin1().data() );

    //mTesters.remove()

    //foreach(Gtm_TesterWindow* lTesterWindow, mTesterWindows)
    for (QMap<GS::Gex::ClientNode*,Gtm_TesterWindow*>::const_iterator lIt=mClients.constBegin();
         lIt!=mClients.constEnd(); lIt++)
    {
        Gtm_TesterWindow* lTesterWindow=lIt.value();
        if (
                (lCNWP.data() && lTesterWindow && lTesterWindow->mClientNodeWP.data()==lCNWP.data())
                ||
                (lTesterWindow && lTesterWindow->property(Gtm_TesterWindow::sClientAboutToCloseProp).toBool()==true)
           )
        {
            //pClientNodes.removeAll(lTesterWindow->mClientNodeWP.data());
            mClients.remove(lIt.key());
            lTesterWindow->hide();
            //mTesterWindows.remove(tw); // Qt3
            //mTesterWindows.removeAll(lTesterWindow);
            //delete lTesterWindow;
            lTesterWindow->deleteLater();
            break;
        }
    }
    OnRefreshAllDialogs();
}

void Gtm_MainWidget::closeEvent(QCloseEvent *e)
{
    GSLOG(5, "Gtm_MainWidget::closeEvent");
    Q_UNUSED(e)

    // Leave section empty as we just want to ignore the 'Close' message!
    e->accept(); // FOR DEBUG ONLY, ALLOWS TO CLOSE THE EXECUTABLE!!!!
}

///////////////////////////////////////////////////////////
// Resizing GTM: need to resize all child stations
///////////////////////////////////////////////////////////
void Gtm_MainWidget::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e)
    OnRefreshAllDialogs();
}

QString Gtm_MainWidget::AttachTester(Gtm_TesterWindow *lTesterWindow)
{
    if (!lTesterWindow)
        return "error: Tester NULL";

    if (lTesterWindow->mClientNodeWP.toStrongRef().isNull())
        return "error: cannot strongify ClientNode";

    GSLOG(6, QString("Attach Tester Window %1...")
          .arg(lTesterWindow->mClientNodeWP.data()->mSocket->m_iSocketInstance).toLatin1().data() );

    // Add to layout
    if (Gtm_MainWidget_baseLayout && Gtm_MainWidget_baseLayout->indexOf(lTesterWindow)==-1)
        Gtm_MainWidget_baseLayout->addWidget(lTesterWindow);
    // Force repaint
    update();

    return "ok";
}

void Gtm_MainWidget::DetachTester(Gtm_TesterWindow *hTester)
{
    GSLOG(6, "DetachTester");
    // Remove from layout
    //Gtm_MainWidget_baseLayout->remove(hTester); //Qt3
    Gtm_MainWidget_baseLayout->removeWidget(hTester);
    // Force repaint
    update();
}

/*
void Gtm_MainWidget::SendMessage(int lStationNumber, QString strMessage,int lSeverity)
{
    GSLOG(5, QString("SendMessage %1").arg(strMessage) );
    foreach(GS::Gex::ClientNode *ptClientList, pClientNodes)
    {
        // Station now belongs to ClientNode
        if(ptClientList->cStation.cStationInfo.iStation == lStationNumber)
        {
            ptClientList->NotifyTester(strMessage, lSeverity);
            break;
        }
    };
}
*/

///////////////////////////////////////////////////////////
// Returns number of stations attached (or detached) to GTM
///////////////////////////////////////////////////////////
int Gtm_MainWidget::TotalStations(bool bAttached)
{
    int	iTotalAttached = 0;
    int	iTotalDetached = 0;
    //foreach(GS::Gex::ClientNode *ptClientList, pClientNodes)
    foreach(Gtm_TesterWindow* tw, mClients.values())
    {
        if(tw && tw->bChildWindow)
            iTotalAttached++;
        else
            iTotalDetached++;
    };

    if(bAttached)
        return iTotalAttached;	// Return total stations detached
    else
        return iTotalDetached;	// Return total stations attached
}

///////////////////////////////////////////////////////////
// Return handle to structure holding all Nodes's info.
GS::Gex::ClientNode *Gtm_MainWidget::getClientNode(int iSocketID)
{
    if (mClients.size()==0)
        return 0;
    GS::Gex::ClientNode *ptClientList = 0;
    foreach(ptClientList, mClients.keys())
    {
        // Exit when node found
        if(ptClientList && ptClientList->GetSocketInstance() == iSocketID)
            break;
    };

    // Return handle
    return ptClientList;
}

///////////////////////////////////////////////////////////
// Return handle to structure holding all Nodes's info.
// Same purpose as 'getClientNode(int iSocketID)'
///////////////////////////////////////////////////////////
GS::Gex::ClientNode *Gtm_MainWidget::getClientNode(Gtm_TesterWindow	*lTesterWindow)
{
    if (!lTesterWindow)
        return 0;

    //if (pClientNodes.size()==0)
      //  return 0;

    if (lTesterWindow->mClientNodeWP.toStrongRef().isNull())
        return 0;

    return lTesterWindow->mClientNodeWP.data();

    /*
    GS::Gex::ClientNode *ptClientList = pClientNodes.first();
    //foreach(ptClientList, pClientNodes)
    foreach(Gtm_TesterWindow* tw, mTesterWindows)
    {
        // Exit when node found
        //if(tw->mClientNode == hTester)
            //break;
    };
    */

    // Return handle
    //return ptClientList;
}

void Gtm_MainWidget::OnRefreshAllDialogs()
{
    // A GEX node is telling server that it is still alive...
    // Get total number of Tester Windows to display
    if(mClients.size() <= 0)
    {
        // Force repaint
        update();
        return;	// No tester station connected.
    }
}

///////////////////////////////////////////////////////////
// client packet: Tester station is just starting a new program.
/*
void Gtm_MainWidget::clientStationOpen(GS::Gex::ClientSocket *hSocket)
{
    GSLOG(5, "Gtm MainWidget client Station Open...");
    // Create + Add client entry to the list.
    GS::Gex::ClientNode *cNewClient = new GS::Gex::ClientNode(hSocket);

    // Save structure into list
    pClientNodes.append(cNewClient);

    // Connections between tester station window and socket communicating with tester station
    if (!connect(cNewClient, SIGNAL(sNotifyTester(const QString &, int)),
                 hSocket, SLOT(OnNotifyTester(const QString &, int))))
        GSLOG(4, "Cannot connect signal NotifyTester");
    connect(cNewClient, SIGNAL(sSendDynamicPatLimits(void *)), hSocket, SLOT(OnSendDynamicPatLimits(void *)));
    connect(cNewClient, SIGNAL(sSendCommand(void *)), hSocket, SLOT(OnSendCommand(void *)));
    connect(cNewClient, SIGNAL(sSendWriteToStdf(void *)), hSocket, SLOT(OnSendWriteToStdf(void *)));

    // Notify GTM GUI so to display the new Tester node now connected.
    OnRefreshAllDialogs();
}
*/

///////////////////////////////////////////////////////////
// client packet: A Tester/Station is closing the socket link
/*
void Gtm_MainWidget::clientStationClose(int iSocketID)
{
    GSLOG(5, QString("Gtm MainWidget client Station Close %1...").arg(iSocketID).toLatin1().data() );
    // Get handle to node structure to delete.
    GS::Gex::ClientNode *pClient = getClientNode(iSocketID);
    if(pClient == NULL)
        return;

    // Remove entry
    pClientNodes.remove(pClient);

    // free structure
    //GSLOG(5, "Deleting ClientNode...")
    //delete pClient;

    // Update the GUI as socket is closing
    OnRefreshAllDialogs();
}
*/

///////////////////////////////////////////////////////////
// show tester GUI
///////////////////////////////////////////////////////////
void Gtm_MainWidget::clientShowTesterGUI(qintptr iSocketID)
{
    foreach(Gtm_TesterWindow* tw, mClients.values())
    {
        if (!tw)
            continue;
        if (!tw->mClientNodeWP.toStrongRef().isNull())
            continue;
        QObject *lParent=tw->mClientNodeWP.toStrongRef().data()->parent();
        if (!lParent)
            continue;
        GS::Gex::ClientSocket* lCS=qobject_cast<GS::Gex::ClientSocket*>(lParent);
        if (!lCS)
            continue;
        if (lCS->m_iSocketInstance!=iSocketID)
            continue;
        tw->show();
        OnRefreshAllDialogs();
        break;
    }

    /*
    GS::Gex::ClientNode *pClient = getClientNode(iSocketID);
    if(pClient)
    {
        if (pClient->mTesterWindow)
            pClient->mTesterWindow->show();
        OnRefreshAllDialogs();
    }
    */
}

void Gtm_MainWidget::fileNew()
{
}

void Gtm_MainWidget::fileSave()
{
}
#endif
