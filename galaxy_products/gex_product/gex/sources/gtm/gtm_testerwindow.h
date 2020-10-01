
#ifdef GCORE15334

#ifndef GTM_TESTERWINDOW_H
#define GTM_TESTERWINDOW_H

#include <QTableWidget>
#include "ui_gtm_testerwindow_base.h"

// From gtc
#include "gtc_netmessage.h"

// From GEX
#include "patman_lib.h"
#include "pat_info.h"
#include "station.h"
#include "clientnode.h"

///////////////////////////////////////////////////////////
// External classes
class GexTbPatDialog;
class CPatInfo;
class CTest;

// Tester Station GUI class.
class Gtm_TesterWindow : public QDialog, public Ui::Gtm_TesterWindow_base
{
    Q_OBJECT

public:
    static const char* sClientAboutToCloseProp;

    // 7042 : ClientNode now mandatory
    Gtm_TesterWindow(QWeakPointer<GS::Gex::ClientNode>, QWidget* parent = 0, Qt::WindowFlags fl = 0);
    virtual ~Gtm_TesterWindow();

    // true if window child of GTM, false if window is detached!
    bool bChildWindow;

    //GS::Gex::ClientNode* mClientNode; // replaced by client node weak pointer
    QWeakPointer<GS::Gex::ClientNode> mClientNodeWP;

public slots:
    // Refresh 'Status' tab
    void ReloadStatusTab(void);

    // Refresh GUI: this could be called 1 per run so a quite a lot in simulation.
    // Usually connected to signal ClientNode::sNewTestResults()
    void RefreshGUI();

    // should be triggered when the client has received his init message
    void OnClientInit(CStation*);

    // setup the table: rows and columns from Station, i.e. Sites list
    void SetupSitesTable(CStation* lStation);

    // Should be triggered when clientNode destroyed but never exec...
    void OnClientAboutToClose();

    //! \brief CN destroyed
    void OnClientDestroyed();

    // User requests to display a different tab
    void OnTabChange(int index);

    // Attach / Detach window to GTM
    void OnDetachWindow();

    // Ok buttons when Warning or Error event & message is triggered
    // Warning message displayed, user accepts it
    void OnWarningOk();

    // Criticial 'Stop' message displayed, user accepts it
    void OnStopOk();

    // Should be triggered when ClientNode emit a message
    void OnNewMessage(int type, QString message, int lSeverity);

    // Should be triggerd when new bin results received
    void OnNewBinResult(int lPatBin, int lSoftBin);

    // Administration Tab
    // Reset PAT as if restarting lot from scratch
    void OnResetMessage();

    // Reload Dynamic PAT limits from disk (typically after
    // a tester crash & reboot)
    void OnLoadPatLimitsFromfile();	// Load Dynamic PAT limits from disk (typically after a tester crash)

    // virtual functions overloaded...
    void accept();	// to avoid user closing the dialog box!
    //! \brief to avoid user closing the dialog box! Avoids user to manually close station on 'Escape/Cancel' signal
    void reject();

private:
    Q_DISABLE_COPY(Gtm_TesterWindow)
	
    // Refresh 'Tests' tab
    void ReloadTestsTab(void);

    // Case 7260: added debug tab with per site info
    // Refresh STATUS tab contents
    // Refresh debug tab
    void ReloadSitesTab(void);
    // Refresh 'Binning' tab
    void ReloadBinningTab(void);
    // Returns timeout (in seconds) for a given alarm type.
    // 7103 : deprecated
    //int GetTimeoutValue(int iTimeoutID);

    // Removes all HTML codes within a string (makes it pure ASCII)
    void cleanHtmlString(QString &strString);
    bool CopyGivenFile(QString strFrom, QString strTo, bool bEraseAfterCopy=false);	// Copy file.

    // Reset pat (eg: starting new lot)
    void OnResetMessage(bool bCheckPassword);

    // Check/Request Sys Admin password
    // Ask for password, check if matches the one defined in
    // the recipe
    bool checkValidPassword(void);

	// Mailing functions / spooler

	// internal varables
    //Gtm_HistoChart*	frameHistogram;					// Holds Frame where the Histogram is painted.

    // QString used to refresh the sites tab
    QString mSitesTabString;
    //! \brief Widget for the Sites tab
    QTableWidget mSitesTableWidget;
    //! \brief Map to give the col index from site nb in Sites widget
    QMap<int,int> mSitesNbToColIndex;
    //! \brief Items used in SitesTableWidget. To be deleted at destruction.
    QList<QTableWidgetItem*> mSitesTableWidgetItems;
signals:

};

#endif // GTM_TESTERWINDOW_H
#endif
