#ifdef GCORE15334


#ifndef CLIENTNODE_H
#define CLIENTNODE_H

#include <QObject>
#include <QMap>
#include <QTextStream>
#include <QElapsedTimer>

#include "station.h"
#include "gtc_netmessage.h"

#define	GTM_ERRORTYPE_INFO		1
#define	GTM_ERRORTYPE_WARNING	2
#define	GTM_ERRORTYPE_CRITICAL	4
// DIRS
#define	GTM_DIR_SPOOL		"/spool/"			// GTM spooling folder
#define	GTM_DIR_EMAILS		"/spool/emails/"	// GTM Emails spooling sub-folder
#define	GTM_DIR_TRACE		"/spool/trace/"		// GTM treacability spooling sub-folder
#define	GTM_DIR_RECIPES		"/recipes/"			// Recipes folder on server...
// Suffix when temporarly file created
#define GTM_TEMPORARY_EMAIL	"_temp_gtm_email"

// Class used for sorting Pat Definitions in the test program execution order
// Clean me !
class qtPatDefinition: public QList <CPatDefinition*>
{
    public:
    //    int compareItems ( Q3PtrCollection::Item item1, Q3PtrCollection::Item item2 );
};

namespace GS
{
namespace Gex
{

class ClientSocket;

/*! \class  ClientNode
    \brief  The Client node class is here to handle the main porcess fo 1 client.
        This class is not supposed to do any network stuff that is to be done by ClientSocket friend class.
*/
class ClientNode : public QObject
{
    Q_OBJECT

    Q_DISABLE_COPY(ClientNode)

    // '1' if testers doesn't want to receive exceptions notifications.
    unsigned int m_uiSilentMode;
    // Holds full path to the treacability file (or empty if option is disabled)
    QString	mTraceabilityFile;
    // Unique Message ID (to avoid multiple emails with same file name!)
    int	m_iMessageID;
    // just store all instances
    static QMap<qintptr, ClientNode*> sInstances;

public:
    ClientNode(GS::Gex::ClientSocket *hClientSocket);	// Constructor
    virtual ~ClientNode();	// Destructor

    // Handle to socket instance created for this client.
    ClientSocket *mSocket; // remove me ?

    // Holds all data relative to the Tester Station connected to the socket.
    CStation mStation;

    static bool IsClientRegistered(ClientNode* lCN) { if (sInstances.key(lCN,-2)==-2) return false; return true; }
    //! \brief Counter to track total time spend in computation for this client
    static const QString sPropTotalComputationTime;
    //! \brief Start time of this client (a QDateTime)
    static const QString sPropStartTime;

private:
    //! \brief Timer to track/profile computation time
    QElapsedTimer mElTimer;
    QMutex mMutex;
    GS::Gex::SiteTestResults*   FindSite(const int SiteNb);
    GS::Gex::SiteTestResults*   FindSite(const unsigned int SiteIndex, const int SiteNb, bool bCreate=true);
    void                        DumpTestResults(PT_GNM_RESULTS pMsg_RESULTS);

    // Inital state to be used for each site
    GS::Gex::SiteTestResults::siteState mSitesStates[256];

public slots:

    /*! \brief Get PAT Options used by the background Station
    */
    COptionsPat& GetPatOptions();

    /*! \brief get sock id of the socket linked to this client.
        Returns -1 is socket disconnected.
    */
    int GetSocketInstance();

    /*! \brief  Notification to the tester station. Severity is .
    */
    QString NotifyTester(const QString & strMessage, int lSeverity = 0);

    /*! \brief Removes all HTML codes within a string (makes it pure ASCII)
     Clean a string from its HTML codes (make it pure ASCII)
    */
    void cleanHtmlString(QString &strString);

    /*! \brief  7103 : returns the severity(-1,0,1) according to the index : 0(critical), 1(warning), 2(Notice), 3(Ignore)
    */
    int GetAlarmSeverity(int lIndex);

    /*! \brief Get SilentMode
    */
    int GetSilentMode() { return m_uiSilentMode; }

    /*! \brief Message & Error management
        Actually bad name : also send message to tester if desired !!!
        errorType = 1 2 or 3
        lSeverity = -1 0 or 1 (dont ask why)
    */
    QString DisplayMessage(int errorType, QString strMessage, bool bSendEmail=false,
                           bool bNotifyTester=false, int lSeverity=0);

    /*! \brief Traceability functions (storage, ...): Set Traceability file name.
    */
    void SetTraceabilityFileName(QString & FileName);
    /*! \brief Traceability functions (storage, ...): Create Traceability file.
    */
    bool CreateTraceabilityFile(void);
    /*! \brief Update Traceability file name (in case LotID now known, etc...)
    */
    bool UpdateTraceabilityFileName(void);
    /*! \brief Send message into traceability file (ASCII, STDF,...): Saves message into Traceability output file( if enabled)
    */
    void TraceabilityMessage(QString &strMessage, bool bPrefix=true);
    /*! \brief Write PAT Dynamic limits to traceability file for specified site.
    */
    void TraceabilityDynamicLimits( GS::Gex::SiteTestResults* Site);
    /*! \brief Write outlier summary into traceability file (tests table)
    */
    void TraceabilityFileOutlierSummaryTests(int SiteNb, QMap<QString, COutlierSummary *> &OutliersSummary,
                                             QTextStream &lStream);
    /*! \brief Write outlier summary into traceability file
    */
    void TraceabilityFileOutlierSummary(void);

    /*! \brief Create header string with station info (lotID, job name, etc...)
    */
    void BuildStationInfoString(QString &strMessage, QString &strTitle, bool bHtmlFormat=true);

    /*! \brief  Send email message
    */
    bool SendEmailMessage(QString &strMessage);

#if 0
    // Not used for now. If required, need to use new SiteList object instead of CSiteList
    /*! \brief Compute & save distributioin shape of each test for specified site. Keep track of Distribution shape detected during baseline
    */
    void SaveDistributionShape(GS::Gex::CSiteTestResults *pSite);

    /*! \brief Compute & save distributioin shape of each test
    */
    void SaveDistributionShape(void);
#endif

    /*! \brief Disable PAT because of fatal runtime alarm (yield loss, drift, etc...) for specified sites
     Disable PAT on specified sites because of fatal alarm condition
    */
    void DisableDynamicPat(const QList<int> &Sites);

    /*! \brief Disable PAT because of fatal runtime alarm (yield loss, drift, etc...) for specified site
     Disable PAT on specified site because of fatal alarm condition
    */
    void DisableDynamicPat(SiteTestResults* Site);

    /*! \brief Save test results received from GTL for a given run
        Returns ok or error....
    */
    QString ProcessTestResults(const PT_GNM_RESULTS pMsg_RESULTS, SiteTestResults *Site,
                               const PT_GNM_RUNRESULT GtlPartResult, unsigned int RunIndex, bool UpdateRB);

    /*! \brief Save part result received from GTL for a given run
        Returns ok or error....
    */
    QString ProcessPartResult(const PT_GNM_RUNRESULT GtlPartResult, SiteTestResults *Site,
                              const CPatInfo* PatInfo, bool UpdateRB);

    /*! \brief client packet: Receive Tester station details (Tester name, Station#, Program,...)
         In reply, return the PAT config file details.
         Receive init data with station details (program, lot, product,...)
    */
    void ClientInit(PT_GNM_Q_INIT pMsg_Q_INIT, PT_GNM_R_INIT	pMsg_R_INIT);

    /*! \brief Receive Production info from tester (Operator name, Lot ID...)
        Client packet: Receive Tester production details (Operator name, lot ID, sublot ID, job revision...)
    */
    void clientProdInfo(PT_GNM_PRODINFO pMsg_PRODINFO);

    /*! \brief Received query for TestList request and return TestList details.
        client packet: received TestList request from Tester station in reply, return the TestList details.
      */
    QString clientTestList(PT_GNM_Q_TESTLIST pMsg_Q_TESTLIST,PT_GNM_R_TESTLIST pMsg_R_TESTLIST);

    /*! \brief Received query for static PAT config and return static PAT config details
        Client packet: received static Pat config request from Tester station
        In reply, return the static PAT config file details.
    */
    void clientPatInit_Static(PT_GNM_Q_PATCONFIG_STATIC pMsg_Q_PATCONFIG_STATIC,
                              PT_GNM_R_PATCONFIG_STATIC pMsg_R_PATCONFIG_STATIC);
    /*! \brief Received query for DPAT config and return DPAT config details
        Client packet: received dynamic Pat config request from Tester station
        In reply, return the dynamic PAT config file details.
    */
    void clientPatInit_Dynamic(PT_GNM_Q_PATCONFIG_DYNAMIC pMsg_Q_PATCONFIG_DYNAMIC,
                              PT_GNM_PATCONFIG_DYNAMIC pMsg_PATCONFIG_DYNAMIC);
    /*! \brief Send dynamic PAT configuration/limits for specific site to tester
    */
    void clientPatInit_Dynamic(const GS::Gex::SiteTestResults* Site);

    /*! \brief Notifications from the Tester station
        Receives one RUN or more (test results)
        Client packet: Receive test results from N*Run
    */
    QString clientTestResults(PT_GNM_RESULTS pMsg_RESULTS);

    //! \brief Received Endlot from tester station
    void clientEndlot(PT_GNM_Q_ENDLOT pMsg_Q_ENDLOT);

    //! \brief Received End Of Splitlot from tester station
    void ClientEndSplitlot(PT_GNM_Q_END_OF_SPLITLOT pMsg_Q);

    /*! \brief Send Baseline Restart command to tester station
     Case 7260: restart BL for specified site only
     send RestartBaseLine message to tester station.
    */
    void clientRestart_Baseline(const int lSiteNb);
    /*! \brief Send Reset command to tester station
    */
    // Deprecated
    //void clientReset();
    /*! \brief clientLogMessageToStdf
    */
    void clientLogMessageToStdf(QString &strString);
    /*! \brief Apply dynamic limits
        Perform necessary post-processing actions after new limits have been computed
        for a given site
        return ok or error...
    */
    QString ApplyDynamicPatLimits(GS::Gex::SiteTestResults* Site, unsigned int NbSitesUsedToCompute=0);
    /*! \brief Checks / Monitoring fuctions
        Check if need to compute dynamic limits for given site using single site algo
        (either end of baseline, or too many outliers, or N devices tested)
        return ok or error...
    */
    QString CheckForComputeDynamicLimits_SingleSiteAlgo();
    /*! \brief Checks / Monitoring fuctions
        Check if need to compute dynamic limits for given site using merge site algo
        (either end of baseline, or too many outliers, or N devices tested)
        return ok or error...
    */
    QString CheckForComputeDynamicLimits_MergeSitesAlgo();
    /*! \brief Checks / Monitoring fuctions
        Check if need to compute dynamic limits for given site
        (either end of baseline, or too many outliers, or N devices tested)
        return ok or error...
    */
    QString	CheckForComputeDynamicLimits();
    /*! \brief  Check for outlier during production...
    */
    QString	CheckForProductionOutlier(CTest* TestCell, SiteTestResults *Site,
                                      PT_GNM_RUNRESULT BinResult, PT_GNM_TESTRESULT TestResult);
    /*! \brief  After base line completed and also in production, check if too many outliers detected for specified site
    */
    bool CheckForOutliersAlarm(GS::Gex::SiteTestResults* Site);

#if 0
    // Not used for now. If required, need to use new SiteList object instead of CSiteList
    /*! \brief At each run, check if Cpk alert
    */
    bool CheckForParameterCpkAlert(CTest *ptTestCell, GS::Gex::CSiteTestResults *pSite);

    /*! \brief  At Dynamic PAT limits computation, check if drift alert for specified site
    */
    bool CheckForPatDriftAlert(GS::Gex::CSiteTestResults *pSite);

    /*! \brief At each run, check if yield alert on specified site
        The bool will say of alert emited or not
        return ok or error...
    */
    QString	CheckForYieldAlert(GS::Gex::CSiteTestResults *pSite, bool &bYieldAlert);

    /*! \brief  At each run, check if rolling mean drifts too much...alert */
    bool CheckForParameterDriftAlert(GS::Gex::CSiteTestResults *pSite);
#endif

    /*! \brief  Reset */
    QString Reset();

    //! \brief New splitlot
    QString NewSplitlot();

    // ???
    QString RestartBaseline(GS::Gex::SiteTestResults* Site);

signals:
    /*! \brief will be mainly catch by the GUI to create some GUI... */
    void sNewClient();
    /*! \brief emit when the client has received the init packet */
    void sClientInit(CStation*);
    /*! \brief emit when CFC has check the client is really accepted (correct version, slot available,...) */
    void sClientAccepted();
    void sReset();
    void sSendDynamicPatLimits(void*);
    void sNotifyTester(const QString &, int);
    void sSendCommand(void *);
    void sSendWriteToStdf(void *);
    void sNewTestResults();
    /*! \brief emited when a message has to be send to the tester or not */
    void sNewMessage(int type, QString message, int lSeverity);
    /*! \brief emitted when new bin result recieved */
    void sNewBinResult(int lPatBin, int lSoftBin);
    //! \brief Emitted just before closing/destroyed
    void sAboutToClose();
};
} // Gex
} // GS

#endif // CLIENTNODE_H

#endif
