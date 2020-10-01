#ifdef GCORE15334

#ifndef GTM_MAINWIDGET_H
#define GTM_MAINWIDGET_H

#include <QWidget>
#include <QMap>
#include <QPushButton>
#include "gtc_netmessage.h"
//#include "clientnode.h"

namespace GS
{
namespace Gex
{
    class ClientNode;
    class ClientSocket;
    class ClientThread;
}
}
class Gtm_TesterWindow;
class QHBoxLayout;

class Gtm_MainWidget : public QWidget
{
    Q_OBJECT

    // Station number <-> tester
    //QMap<int, Gtm_TesterWindow*> mTesters;
    QPushButton mPlayStopButton;

public:
    Gtm_MainWidget(QWidget* parent = 0, Qt::WindowFlags fl = 0);
    virtual ~Gtm_MainWidget();

    // Notifications from the client socket (Tester station)
    // Open connecting with GUI
    //void clientStationOpen(GS::Gex::ClientSocket *hSocket);
    // Close connection with GUI
    //void clientStationClose(int iSocketID);
    // Show tester GUI
    void clientShowTesterGUI(qintptr iSocketID);

    //  Virtual function overwritten to avoid closing the GTM if tester connected
    void closeEvent(QCloseEvent *e);
    // Handle resize function.
    void resizeEvent(QResizeEvent *e);

private:
  QHBoxLayout* Gtm_MainWidget_baseLayout;

  // List of client nodes (tester stations)
  //QList<GS::Gex::ClientNode*> pClientNodes;
  //QList<Gtm_TesterWindow*> mTesterWindows;
  QMap<GS::Gex::ClientNode*, Gtm_TesterWindow*> mClients;

  // Return handle to relevant Tester station structure
  GS::Gex::ClientNode *getClientNode(int iSocketID);
  // Return handle to relevant Tester station structure
  GS::Gex::ClientNode *getClientNode(Gtm_TesterWindow *hTester);

public slots:
  //
  void OnPlayStopButtonPressed();
  void OnUpdatePlayStopButton(bool lRunning);

  // Get testers map
  //const QMap<int, Gtm_TesterWindow*> GetTesters() { return mTesters; }
  // 'New' button clicked
  void fileNew();
  // 'Save' button clicked
  void fileSave();
  // Draw all active Tester & Stations.
  void OnRefreshAllDialogs();
  // Detach tester from window.
  void DetachTester(Gtm_TesterWindow *hTester);
  // Attach tester to window.
  QString AttachTester(Gtm_TesterWindow *hTester);
  // on new client
  void OnNewClient(QWeakPointer<GS::Gex::ClientThread>);
  // When a new client appear
  void OnNewClientNode( QWeakPointer<GS::Gex::ClientNode> );
  // triggered when this client is going to die very soon
  void OnCloseClientNode( QWeakPointer<GS::Gex::ClientNode> );
  //! \brief Called when clientthread destroyed
  void OnClientThreadDestroyed();
  // Returns total stations detached/attached from GTM
  int TotalStations(bool bAttached);
  // Returns the number of stations/clients ? Bernard ?
  int TotalStations() { return mClients.size(); }
  // Send message to tester : use NotifyTester instead
  //void	SendMessage(int lStationNumber, QString strMessage, int lSeverity);
};

#endif // GTM_MAINWIDGET_H
#endif
