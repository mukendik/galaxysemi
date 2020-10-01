#ifdef GCORE15334


#include <QDir>
#include <stdf.h>
#include <gqtl_log.h>
#include <time.h> // needed for the time() function

#include "ctest.h"
#include "gs_qa_dump.h"
#include "engine.h"
#include "clientnode.h"
#include "clientsocket.h"
#include "patman_lib.h"
#include "pat_info.h"
#include "pat_definition.h"
#include "pat_options.h"
#include "gtl_core.h"
#include "gex_pat_constants_extern.h"


// Call-back Function used by the 'sort' function to sort PAT definitions in execution flow order
extern bool comparePatDefinition(CPatDefinition *test1, CPatDefinition *test2);

namespace GS
{
namespace Gex
{

QMap<qintptr, ClientNode*> ClientNode::sInstances;

const QString ClientNode::sPropTotalComputationTime("TotalComputationTime");
const QString ClientNode::sPropStartTime("StartTime");

ClientNode::ClientNode(GS::Gex::ClientSocket *lClientSocket)
    : QObject(lClientSocket), m_uiSilentMode(0), m_iMessageID(0), mStation(this)
{
    GSLOG(SYSLOG_SEV_NOTICE, QString("New ClientNode on socket %1")
          .arg(lClientSocket?lClientSocket->m_iSocketInstance:-1).toLatin1().data() );

    setProperty(sPropStartTime.toLatin1().data(), QDateTime::currentDateTime());
    setProperty(sPropTotalComputationTime.toLatin1().data(), 0);

    setObjectName(lClientSocket?QString::number(lClientSocket->socketDescriptor()):"-1" );

    if (!lClientSocket)
        GSLOG(3, "Dangerous to create a ClientNode with a null ClientSocket")
    else
        lClientSocket->mClientNode=this;

    m_iMessageID = 0;	// Email message ID reset
    m_uiSilentMode = 0;

    sInstances.insert(lClientSocket?lClientSocket->socketDescriptor():-1, this);

    mSocket = lClientSocket;

    /* Initial state for each site should be BASELINE */
    for(unsigned int i=0; i<256; ++i)
        mSitesStates[i] = GS::Gex::SiteTestResults::SITESTATE_BASELINE;

    GSLOG(SYSLOG_SEV_NOTICE, "Emitting signal NewClient...");
    emit sNewClient();
}

ClientNode::~ClientNode()
{
    GSLOG(5, QString("ClientNode for socket %1 destructor: total %2secs spend in computation ...")
          .arg( mSocket->m_iSocketInstance )
          .arg( property(sPropTotalComputationTime.toLatin1().data()).toUInt()/1000)
          .toLatin1().data() ); // GetSocketInstance() could return -1 if disconnected

    QDateTime lStart=property(sPropStartTime.toLatin1().data()).toDateTime();
    qint64 lTotal=lStart.secsTo(QDateTime::currentDateTime());
    GSLOG(5, QString("ClientNode %1: total time: %2secs").arg(mSocket->m_iSocketInstance)
          .arg( lTotal ).toLatin1().data() );

    sInstances.remove( mSocket->m_iSocketInstance ); // hSocket->socketDescriptor()
}

void ClientNode::ClientInit(PT_GNM_Q_INIT pMsg_Q_INIT, PT_GNM_R_INIT pMsg_R_INIT)
{
    //    // Check if software license hasn't expired!
    //    QDate ExpirationDate = GS::Gex::Engine::GetInstance().GetExpirationDate();
    //    QDate CurrentDate = QDate::currentDate();
    //    if((CurrentDate > ExpirationDate))
    //    {
    //        QString lMessage = QString("License expired (expiration date=%1)!").arg(ExpirationDate.toString());
    //        // Failed reading configuration file...display error message.
    //        DisplayMessage(GTM_ERRORTYPE_CRITICAL, lMessage);
    //        // Return status to the tester station
    //        pMsg_R_INIT->nStatus = GTC_STATUS_LICEXPIRED;	// License expired
    //        gtcnm_CopyString(pMsg_R_INIT->mMessage, lMessage.toLatin1().constData(), GTC_MAX_MESSAGE_LEN);
    //        // Error: Exit.
    //        return;
    //    }
    if (GS::Gex::Engine::GetInstance().GetClientState() != Engine::eState_NodeReady)
    {
        GSLOG(4, QString("Node state not ready: %1").arg(GS::Gex::Engine::GetInstance().GetClientState())
              .toLatin1().data() );
    }

    // GS_QA: dump Init structure
    unsigned int lMaxNbSites = pMsg_Q_INIT->mNbSites > 256 ? 256 : pMsg_Q_INIT->mNbSites;
    GS::Gex::GsQaDump lQaDump;
    QString lQaFileShortName;
    if(pMsg_Q_INIT->mNbSites == 1)
        lQaFileShortName = QString("gs_gtm_socket_init_rx_s%1.csv").arg(pMsg_Q_INIT->mSiteNumbers[0]);
    else
        lQaFileShortName = QString("gs_gtm_socket_init_rx.csv");
    //lQaDump.Open(lQaFileShortName, QIODevice::IO_WriteOnly | QIODevice::IO_Append); // Qt3
    lQaDump.Open(lQaFileShortName, QIODevice::WriteOnly | QIODevice::Append);
    lQaDump.WriteString(QString("#### GTL Init ####\n"));
    lQaDump.WriteString(QString("uiStationNb=%1\n").arg(pMsg_Q_INIT->mStationNb));
    lQaDump.WriteString(QString("uiNbSites=%1\n").arg(pMsg_Q_INIT->mNbSites));
    lQaDump.WriteString(QString("mSiteNumbers="));
    for(unsigned int i=0; i<lMaxNbSites; ++i)
        lQaDump.WriteString(QString("%1 ").arg(pMsg_Q_INIT->mSiteNumbers[i]));
    lQaDump.WriteString(QString("\n"));
    lQaDump.WriteString(QString("mSitesStates="));
    for(unsigned int i=0; i<lMaxNbSites; ++i)
        lQaDump.WriteString(QString("%1 ").arg(pMsg_Q_INIT->mSitesStates[i]));
    lQaDump.WriteString(QString("\n"));
    lQaDump.WriteString(QString("szNodeName=%1\n").arg(pMsg_Q_INIT->mNodeName));
    lQaDump.WriteString(QString("szTesterType=%1\n").arg(pMsg_Q_INIT->mTesterType));
    lQaDump.WriteString(QString("szTestJobName=%1\n").arg(pMsg_Q_INIT->mTestJobName));
    lQaDump.WriteString(QString("szGtlVersion=%1\n").arg(pMsg_Q_INIT->mGtlVersion));
    //lQaDump.WriteString(QString("recipeBuffer length=%1\n").arg( strlen(pMsg_Q_INIT->mRecipeBuffer) ));
    lQaDump.WriteString(QString("recipeBuffer=%1\n").arg(pMsg_Q_INIT->mRecipeBuffer));
    lQaDump.WriteString(QString("#########################################\n"));
    lQaDump.Close();

    // Read PAT configuration file.
    QString lError;
    QString	lRecipeName = pMsg_Q_INIT->mTestJobName;
    QString lRecipeFullFilename="";
    lRecipeName = lRecipeName.replace(' ','_');	// Replace spaces with underscore
    lRecipeName  += "_patman_config.csv";

    // Open config file
    QString lGsFolder = GS::Gex::Engine::GetInstance().Get("GalaxySemiFolder").toString();
    if(lGsFolder.isEmpty() == false)
    {
        lRecipeFullFilename = lGsFolder + GTM_DIR_RECIPES;
    }
    else
    {
        // Server path doesn't exist...if under MicroSoft, try c:\pat (for debug only!)
        #ifdef _WIN32
            lRecipeFullFilename = "c:\\pat\\";
        #endif
    }
    // Build full recipe path name: <server>/recipies/<file.csv>
    lRecipeFullFilename += lRecipeName;

    // Extract Station details just received
    mStation.SetInfo(pMsg_Q_INIT);

    mStation.cTestData.mNbSites         = pMsg_Q_INIT->mNbSites;			// Nb of sites

    for(unsigned int i=0; i<256; ++i)
        if(pMsg_Q_INIT->mSitesStates[i] != -1)
        {
            if (pMsg_Q_INIT->mSitesStates[i]==1)
            {
                GSLOG(5, QString("Site %1 will be in DPAT state").arg(i).toLatin1().data() );
                mSitesStates[i] = GS::Gex::SiteTestResults::SITESTATE_DPAT;
            }
        }
    // Update GUI window caption
    QString strMessage = "Station " + QString::number(mStation.GetStationNb());
    strMessage += ": " + mStation.Get(CStationInfo::sJobNamePropName).toString();

    // emit now or later ?
    //GSLOG(5, "emiting clientInit...");
    emit sClientInit(&mStation);

    // Read PAT recipe from buffer or file
    bool lStatus = (pMsg_Q_INIT->mRecipeBuffer && (strlen(pMsg_Q_INIT->mRecipeBuffer)>0)) ?
                mStation.ReadPatRecipe(pMsg_Q_INIT->mRecipeBuffer, lError) :
                mStation.ReadPatRecipe(lRecipeFullFilename, lError);
    if(lStatus == false)
    {
        // Failed reading configuration file...display error message.
        DisplayMessage(GTM_ERRORTYPE_CRITICAL, lError);

        // Return status to the tester station
        pMsg_R_INIT->nStatus = GTC_STATUS_CRCF;	// Failed reading config file.
        gtcnm_CopyString(pMsg_R_INIT->mMessage, lError.toLatin1().constData(), GTC_MAX_MESSAGE_LEN);
        // Error: Exit.
        return;
    }

    // Create Traceability file and insert header line.
    CreateTraceabilityFile();

    // Success
    strMessage = "Station initialized: " + QString(pMsg_Q_INIT->mGtlVersion);
    DisplayMessage(GTM_ERRORTYPE_INFO,strMessage);

    // Reply packet status: OK.
    pMsg_R_INIT->nStatus = GTC_STATUS_OK;
    pMsg_R_INIT->nGtlModules = GTC_GTLMODULE_PAT;
    // Number of test program runs per socket packet
    pMsg_R_INIT->uiResultFrequency = mStation.GetPatOptions().mFT_RunsPerPacket;

    unsigned int uiBinning, uiIndex=0;
    for(uiBinning=0; uiBinning<256; uiBinning++)
    {
        // TODO : handle any bin number for good bins : from 0....
        if(mStation.GetPatOptions().pGoodSoftBinsList->Contains(uiBinning))
            pMsg_R_INIT->pnGoodSoftBins[uiIndex++] = uiBinning;
    }
    pMsg_R_INIT->uiNbGoodSBins = uiIndex;

    uiIndex=0;
    for(uiBinning=0; uiBinning<256; uiBinning++)
    {
       if(mStation.GetPatOptions().pGoodHardBinsList->Contains(uiBinning))
          pMsg_R_INIT->pnGoodHardBins[uiIndex++] = uiBinning;
    }
    pMsg_R_INIT->uiNbGoodHBins = uiIndex;

    // Tell how test are identified: by number, name or both.
    switch(mStation.GetPatOptions().mTestKey)
    {
        case GEX_TBPAT_KEY_TESTNUMBER:
        default:
            pMsg_R_INIT->uiFindTestKey = GTC_FINDTEST_TESTNUMBER;
            break;
        case GEX_TBPAT_KEY_TESTNAME:
            pMsg_R_INIT->uiFindTestKey = GTC_FINDTEST_TESTNAME;
            break;
        case GEX_TBPAT_KEY_TESTMIX:
            pMsg_R_INIT->uiFindTestKey = GTC_FINDTEST_TESTNUMBERANDNAME;
            break;
    }
    // Number of test program runs before tester sends Production Info (lotID, sublotID, Operator name...)
    pMsg_R_INIT->uiNbRunsBeforeProdInfo = 0;
}

void ClientNode::clientProdInfo(PT_GNM_PRODINFO pMsg_PRODINFO)
{
    QMutexLocker lML(&mMutex);

    // Extract Production details just received
    QString lR=mStation.SetProdInfo(pMsg_PRODINFO);

    // Should update GUI. But is calling TesterWindow::OnClientInit()...
    emit sClientInit(&mStation);

    // Rename Traceability file as some of its fields used in the name can now be more accurate (eg: Operator name,...)
    UpdateTraceabilityFileName();

    // Traceability message.
    QString strMessage = "<b>Production info:</b>";
    strMessage += " Lot: " + mStation.Get(CStationInfo::sLotPropName).toString();
    strMessage += " SubLot: " + mStation.Get(CStationInfo::sSublotPropName).toString();
    strMessage += " SplitotID: " + mStation.Get(CStationInfo::sSplitlotIDPropName).toString();
    strMessage += + "<br>";

    DisplayMessage(GTM_ERRORTYPE_INFO,strMessage);
}

void ClientNode::ClientEndSplitlot(PT_GNM_Q_END_OF_SPLITLOT pMsg_Q)
{
    GSLOG(5, "Client End Splitlot");

    //QMutexLocker lML(&mMutex);

    unsigned long luTime = time(NULL);
    if(luTime-pMsg_Q->mTimeStamp > 24*3600)
    {
        DisplayMessage(GTM_ERRORTYPE_INFO,"GTM running");
        return;
    }

    // Build outlier summary report into HTML/STDF/CSV traceability file
    // Todo: Is it going to overwrite any html of previous splitlots ?????
    GSLOG(4, "Check me: is TraceabilityFileOutlierSummary needed per splitlot ???");
    TraceabilityFileOutlierSummary();

    // Traceability message.
    QString strMessage = "<b>End of Splitlot reached:</b>";
    strMessage += "<br>  Lot: " + mStation.Get(CStationInfo::sLotPropName).toString();
    strMessage += "<br>  SubLot: " + mStation.Get(CStationInfo::sSublotPropName).toString() + "<br>";
    strMessage += "<br>  SplitlotID: " + mStation.Get(CStationInfo::sSplitlotIDPropName).toString()+ "<br>";
    strMessage += "<br>  RetestIndex: " + mStation.Get(CStationInfo::sRetestIndexPropName).toString()+ "<br>";

    DisplayMessage(GTM_ERRORTYPE_INFO,strMessage);

    // Do not Reset station because it would clear test list and perhaps they dont want to if they are doing on the fly retest/resume
    GSLOG(4, "Client end of splitlot: check me: do we have to reset client node (stats, counters,... ) ?");
    //Reset(); //OnResetMessage(false);
}

void ClientNode::clientEndlot(PT_GNM_Q_ENDLOT pMsg_Q_ENDLOT)
{
    GSLOG(5, "clientEndlot");

    unsigned long luTime = time(NULL);
    if(luTime-pMsg_Q_ENDLOT->uiTimeStamp > 24*3600)
    {
        DisplayMessage(GTM_ERRORTYPE_INFO,"GTM running");
        return;
    }

    // Build outlier summary report into HTML/STDF/CSV traceability file
    TraceabilityFileOutlierSummary();

    // Traceability message.
    QString strMessage = "<b>End of lot reached:</b>";
    strMessage += "<br>  Lot: " + mStation.Get(CStationInfo::sLotPropName).toString();
    strMessage += "<br>  SubLot: " + mStation.Get(CStationInfo::sSublotPropName).toString() + "<br>";

    DisplayMessage(GTM_ERRORTYPE_INFO,strMessage);

    // Reset station
    Reset(); //OnResetMessage(false);
}

QString ClientNode::NewSplitlot()
{
    return "error: code me";
}

QString ClientNode::Reset()
{
    //QMutexLocker lML(&mMutex);

    GSLOG(5, "Reset");

    emit sReset(); // should now be connected to TesterWindow::OnResetMessage()

    // Reset station status
    mStation.SetStationStatus(CStation::STATION_ENABLED);
    // Clear silent mode: tester will receive alarms notifications (unless it later sets this flag)
    m_uiSilentMode = 0;
    // Reset statistics.
    mStation.cTestData.Reset();
    // Reset station info (except fields that do not change between lots: tester name, program name, etc....)
    mStation.ClearInfo(false);
    // Create new traceability file

    //emit sClientInit(&cStation); // ?

    return "ok";
}

// Case 7260: restart BL for specified site only
QString ClientNode::RestartBaseline(GS::Gex::SiteTestResults* Site)
{
    //QMutexLocker lML(&mMutex);

    //QElapsedTimer lElTimer;
    //lElTimer.start();

    // Check site ptr
    if(!Site)
    {
        GSLOG(SYSLOG_SEV_CRITICAL, QString("Restart BaseLine failed: null site pointer").toLatin1().data());
        return "error: null site pointer.";
    }

    GSLOG(SYSLOG_SEV_NOTICE, QString("Restart BaseLine for site %1").arg(Site->SiteNb()).toLatin1().data() );

    // Set site for a new baseline (reset RB, set site state to BASELINE...)
    Site->RestartBaseline();

    //setProperty(sPropTotalComputationTime.toLatin1().data(),
      //          property(sPropTotalComputationTime.toLatin1().data()).toUInt()+lElTimer.elapsed() );

    return "ok";
}

QString ClientNode::clientTestList(PT_GNM_Q_TESTLIST pMsg_Q_TESTLIST, PT_GNM_R_TESTLIST pMsg_R_TESTLIST)
{
    //QMutexLocker lML(&mMutex);

    QElapsedTimer lElTimer;
    lElTimer.start();

    // pMsg_Q_TESTLIST not used for now. Avoid warning.
    Q_UNUSED(pMsg_Q_TESTLIST)

    unsigned int uiTotalTests=mStation.GetPatDefinitionsCount();

    // Malloc space for test definitions
    pMsg_R_TESTLIST->pstTestDefinitions = (PT_GNM_TESTDEF)malloc(uiTotalTests*sizeof(GNM_TESTDEF));
    if (!pMsg_R_TESTLIST->pstTestDefinitions)
        return "error: malloc failed";

    // Read PAT Test settings
    qtPatDefinition PatDefList;
    CPatDefinition* ptPatDef = NULL;
    QHash<QString, CPatDefinition*>::iterator   itPATDefinifion;

    for(itPATDefinifion = mStation.GetPatDefinitions().begin();
        itPATDefinifion != mStation.GetPatDefinitions().end(); ++itPATDefinifion)
    {
        ptPatDef = *itPATDefinifion;

        // Check if we've reached the end of the list...
        if(ptPatDef == NULL)
            break;

        // Only include tests that are PAT enabled or Cpk/Monitoring enabled....
        if(!ptPatDef->IsTestDisabled())
        {
            // Save pointer to this valid entry...to be part of the list to sort
            PatDefList.append(ptPatDef);
        }
    }

    // Sort list
    qSort(PatDefList.begin(), PatDefList.end(), comparePatDefinition);

    // Fill buffer list, with test sorted in execution order
    unsigned int uiIndex=0, uiFlags=0;

    foreach(ptPatDef, PatDefList)
    {
        uiFlags = 0;

        GSLOG(6, QString("Test definition for test %1 pin %2").arg(ptPatDef->m_lTestNumber).arg(ptPatDef->mPinIndex)
              .toLatin1().constData() );

        (pMsg_R_TESTLIST->pstTestDefinitions)[uiIndex].lTestNumber = ptPatDef->m_lTestNumber;
        gtcnm_CopyString((pMsg_R_TESTLIST->pstTestDefinitions)[uiIndex].szTestName,
                         ptPatDef->m_strTestName.toLatin1().constData(), GTC_MAX_TESTNAME_LEN);
        // CHECK ME
        (pMsg_R_TESTLIST->pstTestDefinitions)[uiIndex].mPinIndex = ptPatDef->mPinIndex;

        gtcnm_CopyString((pMsg_R_TESTLIST->pstTestDefinitions)[uiIndex].szTestUnits,
                         ptPatDef->m_strUnits.toLatin1().constData(), GTC_MAX_TESTUNITS_LEN);
        if(ptPatDef->m_lfLowLimit == -GEX_TPAT_DOUBLE_INFINITE)
            (pMsg_R_TESTLIST->pstTestDefinitions)[uiIndex].fLowLimit = -GTL_INFINITE_VALUE_FLOAT;
        else
        {
            uiFlags |= GTL_TFLAG_HAS_LL;
            (pMsg_R_TESTLIST->pstTestDefinitions)[uiIndex].fLowLimit = (float)ptPatDef->m_lfLowLimit;
        }
        if(ptPatDef->m_lfHighLimit == GEX_TPAT_DOUBLE_INFINITE)
            (pMsg_R_TESTLIST->pstTestDefinitions)[uiIndex].fHighLimit = GTL_INFINITE_VALUE_FLOAT;
        else
        {
            uiFlags |= GTL_TFLAG_HAS_HL;
            (pMsg_R_TESTLIST->pstTestDefinitions)[uiIndex].fHighLimit = (float)ptPatDef->m_lfHighLimit;
        }
        (pMsg_R_TESTLIST->pstTestDefinitions)[uiIndex].uiTestFlags  = uiFlags;

        // Increment array offest
        uiIndex++;
    }

    // Update nb tests really in TestList
    pMsg_R_TESTLIST->uiNbTests = PatDefList.count();

    // Success
    QString strMessage = "TestList loaded: " + QString::number(uiIndex) + " Tests";
    DisplayMessage(GTM_ERRORTYPE_INFO,strMessage);

    setProperty(sPropTotalComputationTime.toLatin1().data(),
                    property(sPropTotalComputationTime.toLatin1().data()).toUInt()+lElTimer.elapsed() );

    return "ok";
}

void ClientNode::clientPatInit_Static(PT_GNM_Q_PATCONFIG_STATIC pMsg_Q_PATCONFIG_STATIC,
                                      PT_GNM_R_PATCONFIG_STATIC pMsg_R_PATCONFIG_STATIC)
{
    //QMutexLocker lML(&mMutex);

    QElapsedTimer lElTimer;
    lElTimer.start();

    // pMsg_Q_PATCONFIG_STATIC not used for now. Avoid warning.
    Q_UNUSED(pMsg_Q_PATCONFIG_STATIC)

    unsigned int uiTotalTests = mStation.GetPatDefinitionsCount();
    unsigned int uiFlags = 0;

    // Allocate space for test definitions
    pMsg_R_PATCONFIG_STATIC->pstTestDefinitions
            = (PT_GNM_TESTDEF_STATICPAT)malloc(uiTotalTests*sizeof(GNM_TESTDEF_STATICPAT));
    if (!pMsg_R_PATCONFIG_STATIC->pstTestDefinitions)
    {
        GSLOG(3, "malloc failed");
        return;
    }

    // Read PAT Global settings
    // Baseline
    pMsg_R_PATCONFIG_STATIC->uiBaseline = mStation.GetPatOptions().mFT_BaseLine;
    pMsg_R_PATCONFIG_STATIC->uiFlags = uiFlags;
    // Always 1 single site for ststic PAT limits.
    pMsg_R_PATCONFIG_STATIC->uiNbSites = 1;

    // Read PAT Test settings
    qtPatDefinition PatDefList;
    CPatDefinition *ptPatDef=0;
    QHash<QString, CPatDefinition*>::iterator itPATDefinifion;

    for(itPATDefinifion = mStation.GetPatDefinitions().begin();
        itPATDefinifion != mStation.GetPatDefinitions().end(); ++itPATDefinifion)
    {
        ptPatDef = *itPATDefinifion;

        // Check if we've reached the end of the list...
        if(ptPatDef == NULL)
            break;

        // Test with Valid Static Bin (not Static disabled)....then save pointer to this valid entry...
        // to be part of the list to sort
        if(ptPatDef->m_lFailStaticBin != -1)
            PatDefList.append(ptPatDef);
    }

    // Sort list: makes tester Pat-Lib faster find test cells.
    qSort(PatDefList.begin(), PatDefList.end(), comparePatDefinition);

    // Fill buffer list, with test sorted in execution order
    unsigned int uiIndex=0;
    foreach(ptPatDef, PatDefList)
    {
        (pMsg_R_PATCONFIG_STATIC->pstTestDefinitions)[uiIndex].mTestNumber = ptPatDef->m_lTestNumber;
        (pMsg_R_PATCONFIG_STATIC->pstTestDefinitions)[uiIndex].mPinIndex = ptPatDef->mPinIndex;
        gtcnm_CopyString((pMsg_R_PATCONFIG_STATIC->pstTestDefinitions)[uiIndex].mTestName,
                         ptPatDef->m_strTestName.toLatin1().constData(), GTC_MAX_TESTNAME_LEN);
        // Provide the Static PAT limits available in the configuration file
        if(ptPatDef->m_lfLowStaticLimit == -GEX_TPAT_DOUBLE_INFINITE)
            (pMsg_R_PATCONFIG_STATIC->pstTestDefinitions)[uiIndex].mLowLimit1 = -GTL_INFINITE_VALUE_FLOAT;
        else
            (pMsg_R_PATCONFIG_STATIC->pstTestDefinitions)[uiIndex].mLowLimit1 = (float)(ptPatDef->m_lfLowStaticLimit);
        if(ptPatDef->m_lfHighStaticLimit == GEX_TPAT_DOUBLE_INFINITE)
            (pMsg_R_PATCONFIG_STATIC->pstTestDefinitions)[uiIndex].mHighLimit1 = GTL_INFINITE_VALUE_FLOAT;
        else
            (pMsg_R_PATCONFIG_STATIC->pstTestDefinitions)[uiIndex].mHighLimit1 = (float)(ptPatDef->m_lfHighStaticLimit);
        (pMsg_R_PATCONFIG_STATIC->pstTestDefinitions)[uiIndex].mTestFlags  =
                GTL_TFLAG_HAS_LL1 | GTL_TFLAG_HAS_HL1 | GTL_TFLAG_HAS_LL2 | GTL_TFLAG_HAS_HL2;

        (pMsg_R_PATCONFIG_STATIC->pstTestDefinitions)[uiIndex].mPatBinning = ptPatDef->m_lFailStaticBin;
        // Provide stats used for computing these Static limits
        gtcnm_CopyString((pMsg_R_PATCONFIG_STATIC->pstTestDefinitions)[uiIndex].mTestStats.mDistributionShape,
                         patlib_GetDistributionName(ptPatDef->m_iDistributionShape).toLatin1().constData(),
                         GTC_MAX_DISTRIBUTIONSHAPE_LEN);
        (pMsg_R_PATCONFIG_STATIC->pstTestDefinitions)[uiIndex].mTestStats.mN_Factor =
                (float)(ptPatDef->m_lfSpatOutlierNFactor);
        (pMsg_R_PATCONFIG_STATIC->pstTestDefinitions)[uiIndex].mTestStats.mT_Factor =
                (float)(ptPatDef->m_lfSpatOutlierTFactor);
        (pMsg_R_PATCONFIG_STATIC->pstTestDefinitions)[uiIndex].mTestStats.mMean =
                (float)(ptPatDef->m_lfMean);
        (pMsg_R_PATCONFIG_STATIC->pstTestDefinitions)[uiIndex].mTestStats.mSigma =
                (float)(ptPatDef->m_lfSigma);
        (pMsg_R_PATCONFIG_STATIC->pstTestDefinitions)[uiIndex].mTestStats.mMin = GTL_INVALID_VALUE_FLOAT;
        (pMsg_R_PATCONFIG_STATIC->pstTestDefinitions)[uiIndex].mTestStats.mQ1 = GTL_INVALID_VALUE_FLOAT;
        (pMsg_R_PATCONFIG_STATIC->pstTestDefinitions)[uiIndex].mTestStats.mMedian = (float)ptPatDef->m_lfMedian;
        (pMsg_R_PATCONFIG_STATIC->pstTestDefinitions)[uiIndex].mTestStats.mQ3 = GTL_INVALID_VALUE_FLOAT;
        (pMsg_R_PATCONFIG_STATIC->pstTestDefinitions)[uiIndex].mTestStats.mMax = GTL_INVALID_VALUE_FLOAT;

        // Increment array offest
        uiIndex++;
    }

    // Update nb tests with PAT enabled
    pMsg_R_PATCONFIG_STATIC->uiNbTests = PatDefList.count();	// Total enabled tests in config file.

    // Success
    QString strMessage = "Static PAT settings loaded: " + QString::number(uiIndex) + " Tests";
    DisplayMessage(GTM_ERRORTYPE_INFO,strMessage);

    // .....add other code if needed

    // Success
    DisplayMessage(GTM_ERRORTYPE_INFO,"<b>Building baseline...</b>");
    //DisplayMessage(GTM_ERRORTYPE_INFO,"<b>Site 1:baselining(23%) Site 2:DPAT Site 3:baselining(85%)...</b>");

    setProperty(sPropTotalComputationTime.toLatin1().data(),
                property(sPropTotalComputationTime.toLatin1().data()).toUInt()+lElTimer.elapsed() );
}

void ClientNode::clientPatInit_Dynamic(PT_GNM_Q_PATCONFIG_DYNAMIC pMsg_Q_PATCONFIG_DYNAMIC,
                                      PT_GNM_PATCONFIG_DYNAMIC pMsg_PATCONFIG_DYNAMIC)
{
    //QMutexLocker lML(&mMutex);

    QElapsedTimer lElTimer; lElTimer.start();

    // pMsg_Q_PATCONFIG_DYNAMIC not used for now. Avoid warning.
    Q_UNUSED(pMsg_Q_PATCONFIG_DYNAMIC)

    unsigned int uiTotalTests = mStation.GetPatDefinitionsCount();

    // Allocate space for test definitions
    pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions
            = (PT_GNM_TESTDEF_DYNAMICPAT)malloc(uiTotalTests*sizeof(GNM_TESTDEF_DYNAMICPAT));
    if (!pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)
    {
        GSLOG(3, "malloc failed");
        return;
    }

    // Read PAT Global settings
    // Always 1 single site for ststic PAT limits.
    pMsg_PATCONFIG_DYNAMIC->uiNbSites = 1;

    // Read PAT Test settings
    qtPatDefinition PatDefList;
    CPatDefinition *ptPatDef=0;
    QHash<QString, CPatDefinition*>::iterator itPATDefinifion;

    for(itPATDefinifion = mStation.GetPatDefinitions().begin();
        itPATDefinifion != mStation.GetPatDefinitions().end(); ++itPATDefinifion)
    {
        ptPatDef = *itPATDefinifion;

        // Check if we've reached the end of the list...
        if(ptPatDef == NULL)
            break;

        // Check if test DPAT enabled, then save pointer to this valid entry
        if((ptPatDef->m_lFailDynamicBin != -1) && (ptPatDef->mOutlierRule != GEX_TPAT_RULETYPE_IGNOREID))
            PatDefList.append(ptPatDef);
    }

    // Sort list: makes tester Pat-Lib faster find test cells.
    qSort(PatDefList.begin(), PatDefList.end(), comparePatDefinition);

    // Fill buffer list, with test sorted in execution order
    unsigned int uiIndex=0;
    foreach(ptPatDef, PatDefList)
    {
        // Specify these DPAT config are for all sites
        (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[uiIndex].mSiteNb = -1;
        (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[uiIndex].mTestNumber = ptPatDef->m_lTestNumber;
        (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[uiIndex].mPinIndex = ptPatDef->mPinIndex;
        gtcnm_CopyString((pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[uiIndex].mTestName,
                         ptPatDef->m_strTestName.toLatin1().constData(), GTC_MAX_TESTNAME_LEN);
        // No DPAT limits yet, set all DPAT limits to infinite
        (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[uiIndex].mLowLimit1 = -GTL_INFINITE_VALUE_FLOAT;
        (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[uiIndex].mHighLimit1 = GTL_INFINITE_VALUE_FLOAT;
        (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[uiIndex].mLowLimit2 = -GTL_INFINITE_VALUE_FLOAT;
        (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[uiIndex].mHighLimit2 = GTL_INFINITE_VALUE_FLOAT;

        (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[uiIndex].mTestFlags  =
                GTL_TFLAG_HAS_LL1 | GTL_TFLAG_HAS_HL1 | GTL_TFLAG_HAS_LL2 | GTL_TFLAG_HAS_HL2;
        (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[uiIndex].mPatBinning = ptPatDef->m_lFailDynamicBin;

        // Provide stats used for computing static limits
        gtcnm_CopyString((pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[uiIndex].mTestStats.mDistributionShape,
                         patlib_GetDistributionName(PATMAN_LIB_SHAPE_UNKNOWN).toLatin1().constData(),
                         GTC_MAX_DISTRIBUTIONSHAPE_LEN);
        (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[uiIndex].mTestStats.mN_Factor = GTL_INVALID_VALUE_FLOAT;
        (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[uiIndex].mTestStats.mT_Factor = GTL_INVALID_VALUE_FLOAT;
        (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[uiIndex].mTestStats.mMean = GTL_INVALID_VALUE_FLOAT;
        (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[uiIndex].mTestStats.mSigma = GTL_INVALID_VALUE_FLOAT;
        (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[uiIndex].mTestStats.mMin = GTL_INVALID_VALUE_FLOAT;
        (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[uiIndex].mTestStats.mQ1 = GTL_INVALID_VALUE_FLOAT;
        (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[uiIndex].mTestStats.mMedian = GTL_INVALID_VALUE_FLOAT;
        (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[uiIndex].mTestStats.mQ3 = GTL_INVALID_VALUE_FLOAT;
        (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[uiIndex].mTestStats.mMax = GTL_INVALID_VALUE_FLOAT;

        // Increment array offest
        uiIndex++;
    }

    // Update nb tests with DPAT enabled
    pMsg_PATCONFIG_DYNAMIC->uiNbTests = PatDefList.count();	// Total enabled tests in config file.

    // Success
    QString strMessage = "DPAT settings loaded: " + QString::number(uiIndex) + " Tests";
    DisplayMessage(GTM_ERRORTYPE_INFO,strMessage);

    // .....add other code if needed

    //setProperty(sPropTotalComputationTime.toLatin1().data(),
      //          property(sPropTotalComputationTime.toLatin1().data()).toUInt()+lElTimer.elapsed() );

    GSLOG(5, QString("clientPatInit_Dynamic done in %1 msecs").arg(lElTimer.elapsed()).toLatin1().data() );
    // Success
}

void ClientNode::clientPatInit_Dynamic(const GS::Gex::SiteTestResults* Site)
{
    //QMutexLocker lML(&mMutex);

    QHash<QString, CPatDefinition*>::iterator   itPATDefinifion;
    CPatDefinition *lPatDef=0;
    unsigned int lNbTests=0;
    QString lMessage;

    // Check site pointer
    if(!Site)
        return;

    // Get Site Nb
    int lSiteNb=Site->SiteNb();

    // First loop to get nb of tests with dynamic PAT
    for(itPATDefinifion = mStation.GetPatDefinitions().begin();
        itPATDefinifion != mStation.GetPatDefinitions().end(); ++itPATDefinifion)
    {
        lPatDef = *itPATDefinifion;

        // Check if we've reached the end of the list...
        if(lPatDef == NULL)
            break;

        // Filter on current site
        if(!lPatDef->mDynamicLimits.contains(lSiteNb))
            continue;

        // Check if test enabled
        if((lPatDef->m_lFailDynamicBin != -1) && (lPatDef->mOutlierRule != GEX_TPAT_RULETYPE_IGNOREID))
            lNbTests++;
    }

    // If no tests in list, simply ignore send command
    if(lNbTests == 0)
    {
        lMessage = QString(
            "<font color=\"#ff0000\"><b>Site %1: all tests disabled, no Dynamic PAT limits sent to tester</b></font>")
            .arg(lSiteNb);
        DisplayMessage(GTM_ERRORTYPE_INFO,lMessage);
        return;
    }

    // We've got one or more tests with valid dynamic limits to send...
    PT_GNM_PATCONFIG_DYNAMIC pMsg_PATCONFIG_DYNAMIC=0;
    // Create dynamic PAT config structure to pass to socket
    pMsg_PATCONFIG_DYNAMIC = (PT_GNM_PATCONFIG_DYNAMIC)malloc(sizeof(GNM_PATCONFIG_DYNAMIC));
    if (!pMsg_PATCONFIG_DYNAMIC)
    {
        GSLOG(3, "malloc failed");
        return;
    }

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Creating buffer to send PAT limits to GTL for site %1")
          .arg(lSiteNb).toLatin1().data() );

    // GS_QA: open dump file
    GS::Gex::GsQaDump lQaDump;
    lQaDump.Open(QString("gs_gtm_socket_dpatlimits_tx_s%1.csv").arg(lSiteNb),
                 //QIODevice::IO_WriteOnly | QIODevice::IO_Append // Qt3
                 QIODevice::WriteOnly | QIODevice::Append
                 );
    lQaDump.WriteString(QString("#### DPAT limits ####\n"));

    // Init structure
    gtcnm_InitStruct(GNM_TYPE_PATCONFIG_DYNAMIC, pMsg_PATCONFIG_DYNAMIC);

    // Allocate space for test definitions
    pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions
            = (PT_GNM_TESTDEF_DYNAMICPAT)malloc(1*lNbTests*sizeof(GNM_TESTDEF_DYNAMICPAT));
    if (!pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)
    {
        GSLOG(3, "malloc failed");
        return;
    }

    // Read PAT Global settings
    pMsg_PATCONFIG_DYNAMIC->uiNbSites = 1;
    pMsg_PATCONFIG_DYNAMIC->uiNbTests = lNbTests;

    // Read PAT Test settings
    unsigned int lIndex=0;

    // GS_QA: dump header
    lQaDump.WriteString(QString("Site#,Test#,Pin#,TestName,LL2,LL1,HL1,HL2\n"));

    // Rewind to first test in test list
    for(itPATDefinifion = mStation.GetPatDefinitions().begin();
        itPATDefinifion != mStation.GetPatDefinitions().end(); ++itPATDefinifion)
    {
        lPatDef = *itPATDefinifion;

        // Check if we've reached the end of the list...
        if(lPatDef == NULL)
            break;

        // Severity level tells which outlier limits to use (near, medium, far)
        int iSeverityLimits = lPatDef->m_iOutlierLimitsSet;

        if((lPatDef->m_lFailDynamicBin != -1) && (lPatDef->mOutlierRule != GEX_TPAT_RULETYPE_IGNOREID))
        {
            (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[lIndex].mSiteNb = lSiteNb;
            (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[lIndex].mTestNumber = lPatDef->m_lTestNumber;
            (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[lIndex].mPinIndex = lPatDef->mPinIndex;

            gtcnm_CopyString((pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[lIndex].mTestName,
                             lPatDef->m_strTestName.toLatin1().constData(), GTC_MAX_TESTNAME_LEN);

            // Write limits for one given testing site
            if(Site->SiteState() == GS::Gex::SiteTestResults::SITESTATE_DISABLED)
            {
                // If PAT is disabled, return infinite limits!
                (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[lIndex].mLowLimit1 = -GTL_INFINITE_VALUE_FLOAT;
                (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[lIndex].mHighLimit1 = GTL_INFINITE_VALUE_FLOAT;
                (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[lIndex].mLowLimit2 = -GTL_INFINITE_VALUE_FLOAT;
                (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[lIndex].mHighLimit2 = GTL_INFINITE_VALUE_FLOAT;
            }
            else
            {
                // Provide new dynamic limits computed...check with severity level must be used!

                // Get relevant limits set
                if(lPatDef->mDynamicLimits[lSiteNb].mLowDynamicLimit1[iSeverityLimits] == -GEX_TPAT_DOUBLE_INFINITE)
                    (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[lIndex].mLowLimit1  = -GTL_INFINITE_VALUE_FLOAT;
                else
                    (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[lIndex].mLowLimit1  =
                        (float)(lPatDef->mDynamicLimits[lSiteNb].mLowDynamicLimit1[iSeverityLimits]);
                if(lPatDef->mDynamicLimits[lSiteNb].mHighDynamicLimit1[iSeverityLimits] == GEX_TPAT_DOUBLE_INFINITE)
                    (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[lIndex].mHighLimit1 = GTL_INFINITE_VALUE_FLOAT;
                else
                    (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[lIndex].mHighLimit1 =
                        (float)(lPatDef->mDynamicLimits[lSiteNb].mHighDynamicLimit1[iSeverityLimits]);
                if(lPatDef->mDynamicLimits[lSiteNb].mLowDynamicLimit2[iSeverityLimits] == -GEX_TPAT_DOUBLE_INFINITE)
                    (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[lIndex].mLowLimit2 = -GTL_INFINITE_VALUE_FLOAT;
                else
                    (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[lIndex].mLowLimit2  =
                        (float)(lPatDef->mDynamicLimits[lSiteNb].mLowDynamicLimit2[iSeverityLimits]);
                if(lPatDef->mDynamicLimits[lSiteNb].mHighDynamicLimit2[iSeverityLimits] == GEX_TPAT_DOUBLE_INFINITE)
                    (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[lIndex].mHighLimit2 = GTL_INFINITE_VALUE_FLOAT;
                else
                    (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[lIndex].mHighLimit2 =
                        (float)(lPatDef->mDynamicLimits[lSiteNb].mHighDynamicLimit2[iSeverityLimits]);
            }

            (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[lIndex].mTestFlags  =
                    GTL_TFLAG_HAS_LL1 | GTL_TFLAG_HAS_HL1 | GTL_TFLAG_HAS_LL2 | GTL_TFLAG_HAS_HL2;
            (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[lIndex].mPatBinning = lPatDef->m_lFailDynamicBin;

            // Provide stats used for computing these Dynamic limits
            gtcnm_CopyString((pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[lIndex].mTestStats.mDistributionShape,
                             patlib_GetDistributionName(lPatDef->mDynamicLimits[lSiteNb].mDistributionShape)
                             .toLatin1().constData(),
                             GTC_MAX_DISTRIBUTIONSHAPE_LEN);
            (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[lIndex].mTestStats.mN_Factor =
                    (float)patlib_GetOutlierFactor(mStation.GetPatInfo(), lPatDef, lSiteNb, true);
            (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[lIndex].mTestStats.mT_Factor =
                    (float)patlib_GetOutlierFactor(mStation.GetPatInfo(), lPatDef, lSiteNb, false);
            (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[lIndex].mTestStats.mMean =
                    (float)(lPatDef->mDynamicLimits[lSiteNb].mDynMean);
            (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[lIndex].mTestStats.mSigma =
                    (float)(lPatDef->mDynamicLimits[lSiteNb].mDynSigma);
            (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[lIndex].mTestStats.mMin = GTL_INVALID_VALUE_FLOAT;
            (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[lIndex].mTestStats.mQ1 = lPatDef->mDynamicLimits[lSiteNb].mDynQ1;
            (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[lIndex].mTestStats.mMedian = lPatDef->mDynamicLimits[lSiteNb].mDynQ2;
            (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[lIndex].mTestStats.mQ3 = lPatDef->mDynamicLimits[lSiteNb].mDynQ3;
            (pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[lIndex].mTestStats.mMax = GTL_INVALID_VALUE_FLOAT;

            // Log limits
            GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Adding limits for site %1, test %2 (%3), severity %4")
               .arg(lSiteNb).arg(lPatDef->m_lTestNumber).arg(lPatDef->m_strTestName).arg(iSeverityLimits)
               .toLatin1().constData() );
            GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Low limit 1 = %1")
                    .arg((pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[lIndex].mLowLimit1).toLatin1().data() );
            GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("High limit 1 = %1")
                    .arg((pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[lIndex].mHighLimit1).toLatin1().data() );
            GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Low limit 2 = %1")
                    .arg((pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[lIndex].mLowLimit2).toLatin1().data() );
            GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("High limit 2 = %1")
                    .arg((pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[lIndex].mHighLimit2).toLatin1().data() );

            // GS_QA: dump limits
            // use sprintf to make sure we have same output as fprintf (QString.sprintf does not give same output)
            char lLL1[100], lLL2[100], lHL1[100], lHL2[100];
            sprintf(lLL1,"%.6e",(pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[lIndex].mLowLimit1);
            sprintf(lLL2,"%.6e",(pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[lIndex].mLowLimit2);
            sprintf(lHL1,"%.6e",(pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[lIndex].mHighLimit1);
            sprintf(lHL2,"%.6e",(pMsg_PATCONFIG_DYNAMIC->pstTestDefinitions)[lIndex].mHighLimit2);
            lQaDump.WriteString(QString("%1,%2,%3,%4,%5,%6,%7,%8\n").arg(lSiteNb)
              .arg(lPatDef->m_lTestNumber).arg(lPatDef->mPinIndex).arg(lPatDef->m_strTestName)
              .arg(lLL2).arg(lLL1).arg(lHL1).arg(lHL2));

            // Increment test buffer index
            lIndex++;
        }
    }

    // GS_QA: dump end marker & close
    lQaDump.WriteString(QString("#########################################\n"));
    lQaDump.Close();

    // Success
    switch(Site->SiteState())
    {
        case GS::Gex::SiteTestResults::SITESTATE_DISABLED:
            lMessage = QString("<font color=\"#ff0000\"><b>Site %1: Dynamic PAT disabled</b></font>")
                    .arg(Site->SiteNb());
            DisplayMessage(GTM_ERRORTYPE_INFO,lMessage);
            break;

        case GS::Gex::SiteTestResults::SITESTATE_BASELINE:
        case GS::Gex::SiteTestResults::SITESTATE_DPAT:
        default:
            lMessage = QString("<font color=\"#ff0000\"><b>Site %1: Dynamic PAT limits computed...</b></font>")
                    .arg(Site->SiteNb());
            DisplayMessage(GTM_ERRORTYPE_INFO,lMessage);
            break;
    }

    // Send dynamic limits to tester
    //emit sSendDynamicPatLimits((void *)pMsg_PATCONFIG_DYNAMIC);
    // or (to go faster)
    mSocket->OnSendDynamicPatLimits((void *)pMsg_PATCONFIG_DYNAMIC); // should send DPAT limits faster than using a signal/slot

}

GS::Gex::SiteTestResults* ClientNode::FindSite(const int SiteNb)
{
    GS::Gex::SiteTestResults*   lSite=0;

    // Get handle to relevant Site structure, if exists.
    if(mStation.cTestData.mSites.contains(SiteNb))
        lSite = mStation.cTestData.mSites[SiteNb];

    return lSite;
}

GS::Gex::SiteTestResults* ClientNode::FindSite(const unsigned int SiteIndex, const int SiteNb, bool bCreate/*=true*/)
{
    unsigned int                lRB_Size=0;
    QString                     lResult;
    GS::Gex::SiteTestResults*   lSite=0;

    // Get handle to relevant Site structure. Create it if need be.
    if(mStation.cTestData.mSites.contains(SiteNb))
        lSite = mStation.cTestData.mSites[SiteNb];
    else if(bCreate)
    {
        // Site structure doesn't exist yet? Then create it!

        // Compute rolling buffer size
        lRB_Size = gex_max(mStation.GetPatOptions().mFT_BaseLine, mStation.GetPatOptions().mFT_TuningSamples);

        // Create site structure
        GSLOG(7, QString("Creating a new SiteTestResults for site (SiteIndex=%1, SiteNb=%2), rolling buffer is %3...")
              .arg(SiteIndex).arg(SiteNb).arg(lRB_Size).toLatin1().data() );
        lSite = new GS::Gex::SiteTestResults(SiteIndex, SiteNb, lRB_Size, mStation.GetPatOptions().mTestKey);
        if(lSite)
        {
            // Create test list for this site (uses list read from config file)
            lResult=lSite->Init(mStation.GetPatDefinitions(), mSitesStates[SiteNb]);
            if (lResult.startsWith("error"))
            {
                // Error message
                QString strMessage = QString("Initialization failed for site (SiteIndex=%1, SiteNb=%2): ")
                        .arg(SiteIndex).arg(SiteNb)+ lResult;
                DisplayMessage(GTM_ERRORTYPE_CRITICAL, strMessage);
                delete lSite;
                lSite = 0;
            }
            else
                mStation.cTestData.mSites[SiteNb] = lSite;
        }
        else
            GSLOG(3, "new SiteTestResults failed");
    }

    return lSite;
}

void ClientNode::DumpTestResults(PT_GNM_RESULTS pMsg_RESULTS)
{
    unsigned int lNbSites = pMsg_RESULTS->mNbSites;
    unsigned int lNbTests = pMsg_RESULTS->mNbTests;
    unsigned int lNbRuns = pMsg_RESULTS->mNbAllocatedRuns;

    // GS_QA: open dump file
    GS::Gex::GsQaDump   lQaDump;
    QString             lQaFileShortName;
    if(mStation.cTestData.mNbSites == 1)
        lQaFileShortName = QString("gs_gtm_socket_results_rx_s%1.csv")
                .arg(pMsg_RESULTS->mTestStats[0].mSiteNb);
    else
        lQaFileShortName = QString("gs_gtm_socket_results_rx.csv");
    if(lQaDump.Open(lQaFileShortName, QIODevice::WriteOnly | QIODevice::Append))
    {
        // Dump received information
        unsigned int    lNbValidRuns = pMsg_RESULTS->mNbValidRuns;
        unsigned long   lRunResult_SiteIndex=0;
        unsigned int    lRunIndex=0;
        // Header
        lQaDump.WriteString(QString("#### RESULT packet %1 ####\n").arg(pMsg_RESULTS->mPacketNb));
        lQaDump.WriteString(QString("Nb of sites,%1\n").arg(lNbSites));
        lQaDump.WriteString(QString("Nb of tests,%1\n").arg(lNbTests));
        lQaDump.WriteString(QString("Nb of allocated runs,%1\n").arg(lNbRuns));
        lQaDump.WriteString(QString("Nb of valid runs,%1\n\n").arg(lNbValidRuns));

        // Site loop
        for(unsigned int lSiteIndex=0; lSiteIndex<lNbSites; lSiteIndex++)
        {
            lQaDump.WriteString(QString("#### SITE,%1\n").
                              arg(pMsg_RESULTS->mTestStats[lSiteIndex*lNbTests].mSiteNb));
            // Original Software binning
            lQaDump.WriteString(QString("Original Software binning,"));
            lRunResult_SiteIndex = lSiteIndex*lNbRuns;
            for(lRunIndex=0; lRunIndex<lNbRuns; lRunIndex++)
            {
                lQaDump.WriteString(QString("%1 ").
                                  arg(pMsg_RESULTS->mRunResults[lRunResult_SiteIndex+lRunIndex].mOrgSoftBin));
            }
            lQaDump.WriteString(QString("\n"));
            // Original Hardware binning
            lQaDump.WriteString(QString("Original Hardware binning,"));
            for(lRunIndex=0; lRunIndex<lNbRuns; lRunIndex++)
            {
                lQaDump.WriteString(QString("%1 ").
                                  arg(pMsg_RESULTS->mRunResults[lRunResult_SiteIndex+lRunIndex].mOrgHardBin));
            }
            lQaDump.WriteString(QString("\n"));
            // PAT Software binning
            lQaDump.WriteString(QString("PAT Software binning,"));
            for(lRunIndex=0; lRunIndex<lNbRuns; lRunIndex++)
            {
                lQaDump.WriteString(QString("%1 ").
                                  arg(pMsg_RESULTS->mRunResults[lRunResult_SiteIndex+lRunIndex].mPatSoftBin));
            }
            lQaDump.WriteString(QString("\n"));
            // PAT Hardware binning
            lQaDump.WriteString(QString("PAT Hardware binning,"));
            for(lRunIndex=0; lRunIndex<lNbRuns; lRunIndex++)
            {
                lQaDump.WriteString(QString("%1 ").
                                  arg(pMsg_RESULTS->mRunResults[lRunResult_SiteIndex+lRunIndex].mPatHardBin));
            }
            lQaDump.WriteString(QString("\n"));

            // Part flags
            lQaDump.WriteString(QString("Part flags,"));
            for(lRunIndex=0; lRunIndex<lNbRuns; lRunIndex++)
            {
                lQaDump.WriteString(QString("%1 ").
                                  arg((int)(pMsg_RESULTS->mRunResults[lRunResult_SiteIndex+lRunIndex].mFlags)));
            }
            lQaDump.WriteString(QString("\n"));

            // Tests executed
            lQaDump.WriteString(QString("Tests executed,"));
            for(lRunIndex=0; lRunIndex<lNbRuns; lRunIndex++)
            {
                lQaDump.WriteString(QString("%1 ").
                                  arg(pMsg_RESULTS->mRunResults[lRunResult_SiteIndex+lRunIndex].mNbTestsExecuted));
            }
            lQaDump.WriteString(QString("\n"));

            // PartID
            lQaDump.WriteString(QString("PartID,"));
            for(lRunIndex=0; lRunIndex<lNbRuns; lRunIndex++)
            {
                lQaDump.WriteString(QString("%1 ").
                                  arg(pMsg_RESULTS->mRunResults[lRunResult_SiteIndex+lRunIndex].mPartID));
            }
            lQaDump.WriteString(QString("\n"));

            // PartIndex
            lQaDump.WriteString(QString("PartIndex,"));
            for(lRunIndex=0; lRunIndex<lNbRuns; lRunIndex++)
            {
                lQaDump.WriteString(QString("%1 ").
                                  arg(pMsg_RESULTS->mRunResults[lRunResult_SiteIndex+lRunIndex].mPartIndex));
            }
            lQaDump.WriteString(QString("\n\n"));

            // Header for test results
            lQaDump.WriteString("Test#,Pin#,TestName,Nb. results,(Result|Flags)x(Nb. results)\n");

            // Test loop
            unsigned long      ulIndex=0;
            PT_GNM_TESTRESULT   lTestResult;
            for(unsigned int lTestIndex=0; lTestIndex<lNbTests; lTestIndex++)
            {
                ulIndex = lSiteIndex*lNbTests + lTestIndex;
                lQaDump.WriteString(QString("%1,%2,%3,%4,").arg(pMsg_RESULTS->mTestStats[ulIndex].lTestNumber)
                                  .arg(pMsg_RESULTS->mTestStats[ulIndex].mPinIndex)
                                  .arg(pMsg_RESULTS->mTestStats[ulIndex].mTestName)
                                  .arg(pMsg_RESULTS->mTestStats[ulIndex].uiExecs));
                // Run loop
                ulIndex = lSiteIndex*lNbTests*lNbRuns + lTestIndex*lNbRuns;
                for(lRunIndex=0; lRunIndex<lNbRuns; lRunIndex++)
                {
                    char lTemp[100];
                    lTestResult = (pMsg_RESULTS->mTestResults)+(ulIndex+lRunIndex);
                    if(lTestResult->mValue == GTL_INVALID_VALUE_FLOAT)
                    {
                        sprintf(lTemp,"n/a|%d ", lTestResult->mFlags);
                        lQaDump.WriteString(QString(lTemp));
                    }
                    else
                    {
                        // use sprintf to make sure we have same output as fprintf
                        // (QString.sprintf does not give same output)
                        sprintf(lTemp,"%.6e|%d ", lTestResult->mValue, lTestResult->mFlags);
                        lQaDump.WriteString(QString(lTemp));
                    }
                }
                lQaDump.WriteString(QString("\n"));
            }
            lQaDump.WriteString(QString("\n"));
        }

        // Write end marker & close dump file
        lQaDump.WriteString(QString("#########################################\n\n"));
        lQaDump.Close();
    }
}

QString ClientNode::ProcessTestResults(const PT_GNM_RESULTS pMsg_RESULTS, SiteTestResults *Site,
                                       const PT_GNM_RUNRESULT GtlPartResult, unsigned int RunIndex, bool UpdateRB)
{
    if(!pMsg_RESULTS)
        return "error: invalid GtlResults ptr";
    if(!Site)
        return "error: invalid Site ptr";
    if(!GtlPartResult)
        return "error: invalid GtlPartResult ptr";

    unsigned int    lNbTests = pMsg_RESULTS->mNbTests;
    unsigned int    lNbRuns = pMsg_RESULTS->mNbAllocatedRuns;
    unsigned int    lSiteIndex = Site->SiteIndex();
    unsigned int    lSiteOffset=lSiteIndex*lNbRuns*lNbTests;

    // Test loop
    CTest*            lTestCell=0;
    unsigned int        lTestIndex=0;
    int                 lTestNumber=0;
    int                 lRtnIndex=-1;
    char*               lTestName=0;
    PT_GNM_TESTRESULT   lTestResult=0;
    for(lTestIndex=0; lTestIndex<lNbTests; lTestIndex++)
    {
        lTestNumber = pMsg_RESULTS->mTestStats[lSiteIndex*lNbTests + lTestIndex].lTestNumber;
        lTestName   = pMsg_RESULTS->mTestStats[lSiteIndex*lNbTests + lTestIndex].mTestName;
        lRtnIndex   = pMsg_RESULTS->mTestStats[lSiteIndex*lNbTests + lTestIndex].mPinIndex;
        lTestCell   = Site->FindTestCell(lTestNumber,lRtnIndex,false,true,lTestName);
        if(!lTestCell)
        {
            // CHECKME: what should we do if error finding/creating test item?
            GSLOG(4, QString("Failed to find a test cell: %1 pin %2: '%3'").arg(lTestNumber).arg(lRtnIndex).arg(lTestName)
                  .toLatin1().data() );
            continue;
        }
        lTestResult = (pMsg_RESULTS->mTestResults)+(lSiteOffset+lTestIndex*lNbRuns+RunIndex);

        // Update test results
        QString lR=Site->CollectTestResult(lTestCell, lTestResult, UpdateRB);
        if (lR.startsWith("error"))
            GSLOG(SYSLOG_SEV_ERROR, "CollectTestResult failed");

        // Check for production issues
        // If tester requested to no longer receive alarm notification, continue with next test!
        // (eg: if flushing last few runs when issuing end-of-lot)
        if(m_uiSilentMode)
            continue;

        CPatDefinition* lPatDef=0;
        lPatDef = mStation.GetPatDefinition(lTestCell->lTestNumber, lTestCell->lPinmapIndex,
                                            lTestCell->strTestName);
        if(lPatDef)
        {
            QString lMsg;

            // Production outliers?
            lR=Site->CheckForProductionOutlier(lTestCell, lPatDef, GtlPartResult, lTestResult,
                                               mStation.cTestData.FT_OutliersSummary, lMsg);
            if (lR.startsWith("error"))
            {
                GSLOG(SYSLOG_SEV_WARNING,
                      QString("CheckForProductionOutlier failed: %1").arg(lR).toLatin1().data());
            }
            else if(!lMsg.isEmpty())
            {
                DisplayMessage(GTM_ERRORTYPE_INFO, lMsg);
            }

            // Cpk alarms?
            lR=Site->CheckForCpkAlarm(lTestCell, lPatDef, lMsg);
            if (lR.startsWith("error"))
            {
                GSLOG(SYSLOG_SEV_WARNING,QString("CheckForCpkAlarm failed: %1").arg(lR).toLatin1().data());
            }
            else if(!lMsg.isEmpty())
            {
                // 7103
                // Get timeout value (in seconds) for this alarm condition
                //int iTimeout = GetTimeoutValue(cStation.GetPatOptions().iFT_Alarm_Timeout_ParamCpk);
                long long lOV=mStation.GetPatOptions().property(ALARM_SEVERITY_PARAM_CPK).toLongLong();
                int lSeverity=GetAlarmSeverity( lOV );
                bool lNotifyTester=lOV!=3?true:false;
                DisplayMessage(GTM_ERRORTYPE_CRITICAL, lMsg, false, lNotifyTester, lSeverity);
            }
        }
    }

    return "ok";
}

QString ClientNode::ProcessPartResult(const PT_GNM_RUNRESULT GtlPartResult, GS::Gex::SiteTestResults* Site,
                                      const CPatInfo* PatInfo, bool UpdateRB)
{
    if(!GtlPartResult)
        return "error: invalid GtlPartResult ptr";
    if(!Site)
        return "error: invalid Site ptr";
    if(!PatInfo)
        return "error: invalid PatInfo ptr";

    // Update global binning summary
    QString lR=mStation.cTestData.UpdateBinSummary(GtlPartResult->mPatSoftBin, GtlPartResult->mPatHardBin);
    if (!lR.startsWith("ok"))
        GSLOG(3, QString("UpdateBinSummary failed: %1").arg(lR).toLatin1().data() );

    // Update per site counters
    lR=Site->CollectPartResult(GtlPartResult, PatInfo, UpdateRB);
    if (!lR.startsWith("ok"))
        GSLOG(3, QString("CollectPartResult failed: %1").arg(lR).toLatin1().data() );

    emit sNewBinResult(GtlPartResult->mPatSoftBin, GtlPartResult->mOrgSoftBin);

    return "ok";
}

QString ClientNode::clientTestResults(PT_GNM_RESULTS pMsg_RESULTS)
{
    mElTimer.start();

    m_uiSilentMode = pMsg_RESULTS->mSilentMode;

    unsigned int lNbSites = pMsg_RESULTS->mNbSites;
    unsigned int lNbTests = pMsg_RESULTS->mNbTests;
    unsigned int lNbRuns = pMsg_RESULTS->mNbAllocatedRuns;

    // QA: dump test results
    DumpTestResults(pMsg_RESULTS);

    // Make sure station is Enabled
    if(mStation.GetStationStatus() != CStation::STATION_ENABLED)
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Station %1 is DISABLED!").arg(mStation.GetStationNb()).toLatin1().data());
        return "error: station disabled";
    }

    // Get PAT info
    CPatInfo *lPatInfo = mStation.GetPatInfo();
    if (!lPatInfo)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("PatInfo is null for station %1!")
              .arg(mStation.GetStationNb()).toLatin1().data());
        return "error: PatInfo null";
    }

    // Site loop
    int                         lSiteNb=0;
    GS::Gex::SiteTestResults*   lSite=0;
    QString                     lR;
    for(unsigned int lSiteIndex=0; lSiteIndex<lNbSites; lSiteIndex++)
    {
        // Get site item (will be created if not existing yet)
        lSiteNb = pMsg_RESULTS->mTestStats[lSiteIndex*lNbTests].mSiteNb;
        lSite = FindSite(lSiteIndex, lSiteNb);
        if(!lSite)
        {
            // CHECKME: what should we do if error allocating new site or error creating testlist?
            continue;
        }

        // Run loop
        //ulIndex = ulTestResult_SiteIndex + uiTestIndex*uiNbRuns;
        //ulRunResult_SiteIndex =²*uiNbRuns;
        unsigned int lRunIndex=0;
        PT_GNM_RUNRESULT lPartResult=NULL;
        for(lRunIndex=0; lRunIndex<lNbRuns; lRunIndex++)
        {
            // Only collect results and perform checks for valid parts
            lPartResult = &pMsg_RESULTS->mRunResults[(lSiteIndex*lNbRuns)+lRunIndex];
            if(lPartResult->mPatSoftBin < 0)
                continue;

            // Only update rolling buffer for certain part types as selected in recipe
            // For now, at FT, update rolling buffer for good parts
            bool lUpdateRB =
                    mStation.GetPatInfo()->GetRecipeOptions().pGoodSoftBinsList->Contains(lPartResult->mOrgSoftBin);

            // Process test results: update RB, check for outliers, alarms...
            lR=ProcessTestResults(pMsg_RESULTS, lSite, lPartResult, lRunIndex, lUpdateRB);
            if (lR.startsWith("error"))
                GSLOG(3, QString("ProcessTestResults failed: %1").arg(lR).toLatin1().data());

            // Process part result: update RB, counters, bin summary...
            lR=ProcessPartResult(lPartResult, lSite, lPatInfo, lUpdateRB);
            if (lR.startsWith("error"))
                GSLOG(3, QString("ProcessPartResult failed: %1").arg(lR).toLatin1().data());
        }

        // Force computing test stats for current site (otherwise only computed when computing dynamic limits)
        bool lB=mStation.ComputeTestStats(lSite);
        if (!lB)
            GSLOG(3, "ComputeTestStats failed");
    }

    // All site have been updated: check if we need to compute new dynamic limits!!
    lR=CheckForComputeDynamicLimits();
    if (lR.startsWith("error"))
        GSLOG(3, QString("CheckForComputeDynamicLimits failed: %1").arg(lR).toLatin1().data() );

    // Tell the GUI new results are available
    emit sNewTestResults(); // Should be connected to RefreshGUI()

    setProperty(sPropTotalComputationTime.toLatin1().data(),
                property(sPropTotalComputationTime.toLatin1().data()).toUInt()+mElTimer.elapsed() );

    return lR;
}

void ClientNode::SetTraceabilityFileName(QString & FileName)
{
    FileName = "";
    QString lJobName=mStation.Get(CStationInfo::sJobNamePropName).toString();

    // Build file name...
    QString lGsFolder = GS::Gex::Engine::GetInstance().Get("GalaxySemiFolder").toString();
    FileName = lGsFolder + GTM_DIR_TRACE;	// GTM Traceability Sub-folder
    char* lQaOutputEnv=getenv("GS_QA_OUTPUT_FOLDER");
    if(getenv("GS_QA") && lQaOutputEnv)
    {
        // Create output directory if it doesn't exist
        QDir lDir;
        QString lQaOutputFolder = lDir.cleanPath(QString(lQaOutputEnv));
        lDir.setPath(lQaOutputFolder);
        if(lDir.exists() || lDir.mkpath(lDir.absolutePath()))
            FileName=QString("%1/").arg(lQaOutputFolder);
    }
    QString lLot=mStation.Get(CStationInfo::sLotPropName).toString();
    if (lLot.isEmpty() || lLot=="?" || lLot=="-")
        FileName += QString("lotid");
    else
        FileName += lLot;

    QString lTesterName=mStation.Get(CStationInfo::sTesterNamePropName).toString();
    if(lTesterName.isEmpty() || lTesterName=="?" || lTesterName=="-")
        FileName += QString("_testername");
    else
        FileName += "_" + lTesterName;

    FileName += "_" + QString::number(mStation.GetStationNb());

    if(lJobName.isEmpty() || lJobName=="?" || lJobName=="-")
        FileName += QString("_jobname");
    else
        FileName += "_" + lJobName;

    QString lSublot=mStation.Get(CStationInfo::sSublotPropName).toString();
    if(lSublot.isEmpty() || lSublot=="?" || lSublot=="-")
        FileName += QString("_sublotid");
    else
        FileName += "_" + lSublot;
    FileName += "_" + mStation.Get(CStationInfo::sLotStartedPropName).toDateTime().toString("ddMMMyy_hhmmss");
    FileName += "_" + QString::number(mSocket->m_iSocketInstance);

    // Add extension depending on format
    switch(mStation.GetPatOptions().mFT_Traceability)
    {
        case GEX_TPAT_TRACEABILITY_ASCII:
            // Traceability is ASCII file.
            FileName += ".txt";
            break;

        case GEX_TPAT_TRACEABILITY_HTML:
            // Traceability is HTML file.
            FileName += ".htm";
            break;

        case GEX_TPAT_TRACEABILITY_STDF:
            // Traceability is STDF file.
            FileName += ".stdf";
            break;

        default:
            // Unknown format, no extension added
            break;
    }

    // Try to clean file name (check for a better method>
    FileName = FileName.remove("<");
    FileName = FileName.remove(">");
    FileName = FileName.replace(" ", "_");
}

bool ClientNode::CreateTraceabilityFile(void)
{
    mTraceabilityFile = "";

    // If Traceability is disabled, return!
    if(mStation.GetPatOptions().mFT_Traceability == GEX_TPAT_TRACEABILITY_NONE)
        return true;

    // For now, only Html format supported
    if(mStation.GetPatOptions().mFT_Traceability != GEX_TPAT_TRACEABILITY_HTML)
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Unsupported format %1 for GTM traceability file.")
              .arg(mStation.GetPatOptions().mFT_Traceability).toLatin1().constData());
        return false;
    }

    // First set file name
    SetTraceabilityFileName(mTraceabilityFile);

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Trying to create traceability file: format=%1, name=%2")
          .arg(mStation.GetPatOptions().mFT_Traceability).arg(mTraceabilityFile).toLatin1().data() );

    QString	strTitle = "Quantix PAT-MAN Traceability Report";
    QString	strMessage;	// header message in report

    switch(mStation.GetPatOptions().mFT_Traceability)
    {
        case GEX_TPAT_TRACEABILITY_ASCII:
            // Traceability is ASCII file.
            // Get Station info (lotID, Operator, Job name, etc....) in ASCII format
            BuildStationInfoString(strMessage,strTitle,false);
            // Insert Title in Traceability file
            TraceabilityMessage(strMessage,false);
            break;

        case GEX_TPAT_TRACEABILITY_HTML:
            // Traceability is HTML file.
            // Get Station info (lotID, Operator, Job name, etc....) in HTML format
            BuildStationInfoString(strMessage,strTitle);

            // Insert Title in Traceability file
            TraceabilityMessage(strMessage,false);

            break;

        case GEX_TPAT_TRACEABILITY_STDF:
            // Traceability is ASCII file.
            // Create STDF file
            GS::StdLib::StdfRecordReadInfo StdfRecordHeader;
            GS::StdLib::Stdf cStdf;
            if(cStdf.Open(mTraceabilityFile.toLatin1().constData(), STDF_WRITE) != GS::StdLib::Stdf::NoError)
                return false;

            StdfRecordHeader.iStdfVersion = 4;	// STDF V4
            #if defined unix || __MACH__
                        StdfRecordHeader.iCpuType = 1;		// Sparc format
            #endif
            #ifdef _WIN32
                        StdfRecordHeader.iCpuType = 2;		// Pc format
            #endif

            // Define the bit order to use when writing in STDF file.
            cStdf.SetStdfCpuType(StdfRecordHeader.iCpuType);

            // Write FAR
            StdfRecordHeader.iRecordType = 0;
            StdfRecordHeader.iRecordSubType = 10;
            cStdf.WriteHeader(&StdfRecordHeader);
            cStdf.WriteByte(StdfRecordHeader.iCpuType);
            cStdf.WriteByte(StdfRecordHeader.iStdfVersion);
            cStdf.WriteRecord();

            // Write MIR
            StdfRecordHeader.iRecordType = 1;
            StdfRecordHeader.iRecordSubType = 10;
            cStdf.WriteHeader(&StdfRecordHeader);
            QDateTime lLS=mStation.Get(CStationInfo::sLotStartedPropName).toDateTime();
            cStdf.WriteDword(lLS.toTime_t());	// Setup_T
            cStdf.WriteDword(lLS.toTime_t());	// Start_T
            cStdf.WriteByte((BYTE)mStation.GetStationNb());	// station #
            cStdf.WriteByte(0);			// mode_code
            cStdf.WriteByte(0);			// rtst_code
            cStdf.WriteByte(0);			// prot_cod #
            cStdf.WriteWord(65535);		// burn_time
            cStdf.WriteByte(0);			// cmode_code
            cStdf.WriteString( mStation.Get(CStationInfo::sLotPropName).toString().toLatin1().constData() );
            cStdf.WriteString( mStation.Get(CStationInfo::sProductPropName).toString().toLatin1().constData() );
            cStdf.WriteString( mStation.Get(CStationInfo::sTesterNamePropName).toString().toLatin1().constData() );
            cStdf.WriteString( mStation.Get(CStationInfo::sTesterTypePropName).toString().toLatin1().constData() );
            cStdf.WriteString( mStation.Get(CStationInfo::sJobNamePropName).toString().toLatin1().constData() );
            cStdf.WriteString( mStation.Get(CStationInfo::sJobRevPropName).toString().toLatin1().constData() );
            cStdf.WriteString( mStation.Get(CStationInfo::sSublotPropName).toString().toLatin1().constData() );
            cStdf.WriteString( mStation.Get(CStationInfo::sOperatorPropName).toString().toLatin1().constData() );
            cStdf.WriteRecord();

            // Write ATR record
            StdfRecordHeader.iRecordType = 0;
            StdfRecordHeader.iRecordSubType = 20;
            cStdf.WriteHeader(&StdfRecordHeader);
            cStdf.WriteDword(time(NULL));		// MOD_TIME: current time & date.
            cStdf.WriteString("PAT-MAN");	// ATR string.
            cStdf.WriteRecord();

            // Close file
            cStdf.Close();

            // Append to STDF file the report header (title)
            strMessage  = "Quantix PAT-MAN Traceability Report";
            TraceabilityMessage(strMessage,false);
            strMessage = "======================================================";
            TraceabilityMessage(strMessage,false);
            strMessage = "Date        : " + mStation.Get(CStationInfo::sLotStartedPropName).toString();
            TraceabilityMessage(strMessage,false);
            strMessage = "Operator    : " + mStation.Get(CStationInfo::sOperatorPropName).toString();
            TraceabilityMessage(strMessage,false);
            strMessage = "Tester      : " + mStation.Get(CStationInfo::sTesterNamePropName).toString();
            TraceabilityMessage(strMessage,false);
            strMessage = "Station     : " + QString::number(mStation.GetStationNb());
            TraceabilityMessage(strMessage,false);
            strMessage = "Product     : " + mStation.Get(CStationInfo::sProductPropName).toString();
            TraceabilityMessage(strMessage,false);
            strMessage = "Lot         : " + mStation.Get(CStationInfo::sLotPropName).toString();
            TraceabilityMessage(strMessage,false);
            strMessage = "Program name: " + mStation.Get(CStationInfo::sJobNamePropName).toString();
            TraceabilityMessage(strMessage,false);
            strMessage = "Program rev.: " + mStation.Get(CStationInfo::sJobRevPropName).toString();
            TraceabilityMessage(strMessage,false);
            strMessage = "======================================================";
            TraceabilityMessage(strMessage,false);
            break;
    }

    // Success
    return true;
}

bool ClientNode::UpdateTraceabilityFileName(void)
{
    // If Traceability is disabled, return!
    if(mStation.GetPatOptions().mFT_Traceability == GEX_TPAT_TRACEABILITY_NONE)
        return true;

    // For now, only Html and Ascii formats supported
    if((mStation.GetPatOptions().mFT_Traceability != GEX_TPAT_TRACEABILITY_HTML) &&
            (mStation.GetPatOptions().mFT_Traceability != GEX_TPAT_TRACEABILITY_ASCII))
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Unsupported format %1 for GTM traceability file.")
              .arg(mStation.GetPatOptions().mFT_Traceability).toLatin1().constData());
        return false;
    }

    // Set file name (could have changed if some prod info have been updated)
    QString strNewName;
    SetTraceabilityFileName(strNewName);

    // If name hasn't changed, then simply return, no need to rename it
    if(mTraceabilityFile == strNewName)
        return true;

    // Name has changed (because some fields used in its name are now initialized. E.g: lot ID, etc...), need to rename the file.
    QDir cDir;
    // Erase destination file in case it exists (should never happen...)
    cDir.remove(strNewName);

    // Rename file...check if rename is successful
    if(cDir.rename(mTraceabilityFile,strNewName) == false)
        return false;

    // Rename is successful, we can save this new file name.
    mTraceabilityFile = strNewName;
    return true;
}

void ClientNode::TraceabilityMessage(QString &strMessage,bool bPrefix/*=true*/)
{
    // If Traceability is disabled, return!
    if(mStation.GetPatOptions().mFT_Traceability == GEX_TPAT_TRACEABILITY_NONE)
        return;

    // For now, only Html format supported
    if(mStation.GetPatOptions().mFT_Traceability != GEX_TPAT_TRACEABILITY_HTML)
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Unsupported format %1 for GTM traceability file.")
              .arg(mStation.GetPatOptions().mFT_Traceability).toLatin1().constData());
        return;
    }

    //clientLogMessageToStdf(strMessage);

    // If no valid file name (yet), return!
    if(mTraceabilityFile.isEmpty() == true)
        return;

    // Insert prefix string (unless otherwise specified)
    if(bPrefix)
        strMessage = "[PAT-Message] : " + strMessage;

    if(mStation.GetPatOptions().mFT_Traceability == GEX_TPAT_TRACEABILITY_ASCII ||
        mStation.GetPatOptions().mFT_Traceability == GEX_TPAT_TRACEABILITY_HTML)
    {
        // If saving into ASCII file, need to remove HTML codes
        if(mStation.GetPatOptions().mFT_Traceability == GEX_TPAT_TRACEABILITY_ASCII)
            cleanHtmlString(strMessage);

        // Save message into ASCII or HTML file
        QFile cFile(mTraceabilityFile);
        //if(cFile.open(QIODevice::WriteOnly | IO_Append | IO_Translate) == FALSE) // Qt3
        if(cFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text) == FALSE)
            return;	// Failed opening traceability file...
        QTextStream clStream(&cFile);
        clStream <<	strMessage << endl;
        cFile.close();
    }
    else
    if(mStation.GetPatOptions().mFT_Traceability == GEX_TPAT_TRACEABILITY_STDF)
    {
        // Save message into STDF file, GDR record
        GS::StdLib::StdfRecordReadInfo	StdfRecordHeader;
        GS::StdLib::Stdf	cStdf;
        if(cStdf.Open( mTraceabilityFile.toLatin1().data(), STDF_APPEND) != GS::StdLib::Stdf::NoError)
            return;
        StdfRecordHeader.iStdfVersion = 4;	// STDF V4
        #if defined unix || __MACH__
                StdfRecordHeader.iCpuType = 1;		// Sparc format
        #endif
        #ifdef _WIN32
                StdfRecordHeader.iCpuType = 2;		// Pc format
        #endif

        // Define the bit order to use when writing in STDF file.
        cStdf.SetStdfCpuType(StdfRecordHeader.iCpuType);

        // Write DTR line
        StdfRecordHeader.iRecordType = 50;
        StdfRecordHeader.iRecordSubType = 30;
        cStdf.WriteHeader(&StdfRecordHeader);
        cStdf.WriteString( strMessage.toLatin1().constData());	// DTR string.
        cStdf.WriteRecord();

        // Close file
        cStdf.Close();
    }
}

#if 0
// Not used for now. If required, need to use new SiteList object instead of CSiteList
void ClientNode::SaveDistributionShape(void)
{
    GSLOG(6, "Save Distribution Shape...");
    // 6935: Check global status, then state per site
    if(mStation.GetStationStatus() != CStation::STATION_ENABLED)
        return;

    // list all sites, all test results.
    // int  iSite = 0;
    CTest *ptTestCell=0;
    GS::Gex::CSiteTestResults *pSite=0;
    CPatDefinition	*ptPatDef=0;
    CSiteList::Iterator it = mStation.cTestData.mCSites.begin();
    while(it != mStation.cTestData.mCSites.end())
    {
        // Get valid site# used and site ptr.
        // iSite = it.key();
        pSite = it.value(); // 6935: use value, instead of []

        if(pSite == NULL)
            goto next_site;

        // 6935: If not in end of base line, but tuning then no need to save again the distribution shape.
        if(pSite->GetSiteState() != GS::Gex::CSiteTestResults::SITESTATUS_BASELINE)
            goto next_site;

        // Save test data to buffer.
        ptTestCell = pSite->getTestList();
        while(ptTestCell != NULL)
        {
            // Get handle to PAT definition for this test
            ptPatDef = mStation.GetPatDefinition(ptTestCell->lTestNumber, ptTestCell->lPinmapIndex,
                                                 ptTestCell->strTestName);

            // Only include tests that are PAT enabled
            if(ptPatDef == NULL || ptPatDef->m_lFailDynamicBin == -1
                    || ptPatDef->m_OutlierRuleType == GEX_TPAT_RULETYPE_IGNOREID)
                goto next_test_cell;

            // Get distribution shape (from baseline samples currently in memory)
            ptPatDef->m_iDistributionShape = patlib_GetDistributionType(ptTestCell, mStation.GetPatInfo());

            // Next test cell
next_test_cell:
            ptTestCell = ptTestCell->GetNextTest();
        };

        // Next site
next_site:
        ++it;
    }
}

void ClientNode::SaveDistributionShape(GS::Gex::CSiteTestResults *pSite)
{
    GSLOG(6, "Save Distribution Shape...");

    // Check pointers
    if(!pSite)
        return;

    // If not in end of base line, but tuning then no need to save again the distribution shape.
    if((mStation.GetStationStatus() != CStation::STATION_ENABLED)
            || (pSite->GetSiteState() != GS::Gex::CSiteTestResults::SITESTATUS_BASELINE))
        return;

    // list all test results.
    CTest *ptTestCell=0;
    CPatDefinition *ptPatDef=0;

    // Save test data to buffer.
    ptTestCell = pSite->getTestList();
    while(ptTestCell != NULL)
    {
        // Get handle to PAT definition for this test
        ptPatDef = mStation.GetPatDefinition(ptTestCell->lTestNumber, ptTestCell->lPinmapIndex,
                                             ptTestCell->strTestName);

        // Only include tests that are PAT enabled
        if(ptPatDef && (ptPatDef->m_lFailDynamicBin != -1)
                && (ptPatDef->m_OutlierRuleType != GEX_TPAT_RULETYPE_IGNOREID))
        {
            // Get distribution shape (from baseline samples currently in memory)
            ptPatDef->m_iDistributionShape = patlib_GetDistributionType(ptTestCell, mStation.GetPatInfo());
        }

        // Next test cell
        ptTestCell = ptTestCell->GetNextTest();
    };
}
#endif

void ClientNode::TraceabilityDynamicLimits(GS::Gex::SiteTestResults* Site)
{
    GSLOG(6, "Traceability DynamicLimits...");

    // Check pointers
    if(!Site)
        return;

    //QMutexLocker lML(&mMutex);

    // If Traceability is disabled, return!
    if(mStation.GetPatOptions().mFT_Traceability == GEX_TPAT_TRACEABILITY_NONE)
        return;
    // If no valid file name (yet), return!
    if(mTraceabilityFile.isEmpty() == true)
        return;

    // For now, only Html format supported
    if(mStation.GetPatOptions().mFT_Traceability != GEX_TPAT_TRACEABILITY_HTML)
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Unsupported format %1 for GTM traceability file.")
              .arg(mStation.GetPatOptions().mFT_Traceability).toLatin1().constData());
        return;
    }

    int                             iSeverityLimits=0;	// Tells which outlier limits to use (near, medium, far)
    double                          lfLowLimit=0,lfHighLimit=0;
    CPatDefinition                  *ptPatDef=0;
    GS::StdLib::StdfRecordReadInfo	StdfRecordHeader;
    GS::StdLib::Stdf                cStdf;
    QFile                           cFile;
    QTextStream                     clStream;

    // Save PAT limits into STDF file, PTR records
    if(mStation.GetPatOptions().mFT_Traceability == GEX_TPAT_TRACEABILITY_STDF)
    {
        if(cStdf.Open( mTraceabilityFile.toLatin1().data(), STDF_APPEND) != GS::StdLib::Stdf::NoError)
            return;

        StdfRecordHeader.iStdfVersion = 4;	// STDF V4
        #if defined unix || __MACH__
            StdfRecordHeader.iCpuType = 1;		// Sparc format
        #endif
        #ifdef _WIN32
            StdfRecordHeader.iCpuType = 2;		// Pc format
        #endif

        // Define the bit order to use when writing in STDF file.
        cStdf.SetStdfCpuType(StdfRecordHeader.iCpuType);
    }
    else
    if(mStation.GetPatOptions().mFT_Traceability == GEX_TPAT_TRACEABILITY_ASCII ||
        mStation.GetPatOptions().mFT_Traceability == GEX_TPAT_TRACEABILITY_HTML)
    {
        // If saving into ASCII or HTML file
        cFile.setFileName(mTraceabilityFile);
        if(cFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text) == FALSE)
            return;	// Failed opening traceability file...
        clStream.setDevice(&cFile);
    }

    // list all test results.
    int		iSite=0;
    int		iTotalTests=0;
    CTest	*ptTestCell=0;

    // Get valid site#.
    iSite = Site->SiteNb();
    GSLOG(6, QString("Traceability dyn limits: %1 sites").arg(iSite).toLatin1().data() );

    if(mStation.GetPatOptions().mFT_Traceability == GEX_TPAT_TRACEABILITY_STDF)
    {
        // Write PIR for this site
        StdfRecordHeader.iRecordType = 5;
        StdfRecordHeader.iRecordSubType = 10;
        cStdf.WriteHeader(&StdfRecordHeader);		// PIR header
        cStdf.WriteByte(1);							// HEAD_NUM
        cStdf.WriteByte(iSite);						// SITE_NUM
        cStdf.WriteRecord();
    }

    // Save test data to buffer.
    ptTestCell = Site->TestList();
    iTotalTests = 0;

    if(mStation.GetPatOptions().mFT_Traceability == GEX_TPAT_TRACEABILITY_ASCII)
    {
        clStream <<	"Testing site: " + QString::number(iSite) << endl;
        clStream <<	"Test number  Test name                  Low Limit      High Limit    Low PAT Limit High PAT Limit" << endl;
        clStream <<	"-----------  -------------------------- ------------- -------------- ------------- --------------"<< endl;
    }
    else
    if(mStation.GetPatOptions().mFT_Traceability == GEX_TPAT_TRACEABILITY_HTML)
    {
        clStream <<	"<h1>Testing site: <b>" + QString::number(iSite) + "</b></h1>" << endl;
        clStream <<	"<table border=\"1\" cellspacing=\"1\" style=\"border-collapse: collapse\" width=\"100%\">" << endl;
        clStream <<	"<tr>" << endl;
        clStream <<	"<td width=\"10%\" bgcolor=\"#CCECFF\"><b>Test#</b></td>" << endl;
        clStream <<	"<td width=\"50%\" bgcolor=\"#CCECFF\"><b>Test name</b></td>" << endl;
        clStream <<	"<td width=\"10%\" bgcolor=\"#CCECFF\"><b>Original<br>Low Limit</b></td>" << endl;
        clStream <<	"<td width=\"10%\" bgcolor=\"#CCECFF\"><b>Original<br>High Limit</b></td>" << endl;
        clStream <<	"<td width=\"10%\" bgcolor=\"#CCECFF\"><b>Dynamic PAT<br>Low Limit</b></td>" << endl;
        clStream <<	"<td width=\"10%\" bgcolor=\"#CCECFF\"><b>Dynamic PAT<br>High Limit</b></td>" << endl;
        clStream <<	"</tr>" << endl;
    }
    while(ptTestCell != NULL)
    {
        // Get handle to PAT definition for this test
        ptPatDef = mStation.GetPatDefinition(ptTestCell->lTestNumber, ptTestCell->lPinmapIndex,
                                             ptTestCell->strTestName);

        // Only include tests that are PAT enabled
        if(ptPatDef == NULL || ptPatDef->m_lFailDynamicBin == -1
                || ptPatDef->mOutlierRule == GEX_TPAT_RULETYPE_IGNOREID)
            goto next_test_cell;

        if((mStation.GetStationStatus() == CStation::STATION_DISABLED)
                || (Site->SiteState() == GS::Gex::SiteTestResults::SITESTATE_DISABLED))
        //if(cStation.iStage == GTM_STAGE_DISABLED)
        {
            // If PAT is disabled, return infinite limits!
            lfLowLimit = -GEX_TPAT_FLOAT_INFINITE;
            lfHighLimit = GEX_TPAT_FLOAT_INFINITE;
        }
        else
        {
            // Provide new dynamic limits computed...check which severity level must be used!
            iSeverityLimits = ptPatDef->m_iOutlierLimitsSet;

            // Get Dynmaics limits for this site
            GS::PAT::DynamicLimits lDynLimits = ptPatDef->GetDynamicLimits(iSite);

            // Get relevant limits set, as the STDF only allows one limit set, we need to specify the range of the two limits sets
            lfLowLimit = gex_min(lDynLimits.mLowDynamicLimit1[iSeverityLimits],
                                 lDynLimits.mLowDynamicLimit2[iSeverityLimits]);
            if(lfLowLimit < -GEX_TPAT_FLOAT_INFINITE)
                lfLowLimit = -GEX_TPAT_FLOAT_INFINITE;

            lfHighLimit = gex_max(lDynLimits.mHighDynamicLimit1[iSeverityLimits],
                                  lDynLimits.mHighDynamicLimit2[iSeverityLimits]);
            if(lfHighLimit > GEX_TPAT_FLOAT_INFINITE)
                lfHighLimit = GEX_TPAT_FLOAT_INFINITE;
        }

        // Write Test limits
        if(mStation.GetPatOptions().mFT_Traceability == GEX_TPAT_TRACEABILITY_ASCII)
        {
            clStream <<	ptTestCell->lTestNumber << "\t";
            clStream <<	ptPatDef->m_strTestName << "\t";

            // Original program: Low Limit
            if((ptTestCell->GetCurrentLimitItem()->lfLowLimit > -GEX_TPAT_FLOAT_INFINITE) && ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & 1) == 0))
                clStream <<	ptTestCell->GetCurrentLimitItem()->lfLowLimit  << " " << ptPatDef->m_strUnits << "\t";
            else
                clStream <<	"-" << "\t";

            // Original program: High Limit
            if((ptTestCell->GetCurrentLimitItem()->lfHighLimit < GEX_TPAT_FLOAT_INFINITE) && ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & 2) == 0))
                clStream <<	ptTestCell->GetCurrentLimitItem()->lfHighLimit  << " " << ptPatDef->m_strUnits << "\t";
            else
                clStream <<	"-" << "\t";

            // PAT limits
            clStream <<	lfLowLimit  << " " << ptPatDef->m_strUnits << "\t";
            clStream <<	lfHighLimit << " " << ptPatDef->m_strUnits << endl;
        }
        else
        if(mStation.GetPatOptions().mFT_Traceability == GEX_TPAT_TRACEABILITY_HTML)
        {
            clStream <<	"<tr>" << endl;
            clStream <<	"<td width=\"10%\" bgcolor=\"#F8F8F8\">" << ptTestCell->lTestNumber << "</td>" << endl;
            clStream <<	"<td width=\"50%\" bgcolor=\"#F8F8F8\">" << ptPatDef->m_strTestName << "</td>" << endl;

            // Original program: Low Limit
            if((ptTestCell->GetCurrentLimitItem()->lfLowLimit > -GEX_TPAT_FLOAT_INFINITE) && ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & 1) == 0))
                clStream <<	"<td width=\"10%\" bgcolor=\"#F8F8F8\">" << ptTestCell->GetCurrentLimitItem()->lfLowLimit  << " " << ptPatDef->m_strUnits << "</td>" << endl;
            else
                clStream <<	"<td width=\"10%\" bgcolor=\"#F8F8F8\">" << "n/a" "</td>" << endl;

            // Original program: High Limit
            if((ptTestCell->GetCurrentLimitItem()->lfHighLimit < GEX_TPAT_FLOAT_INFINITE) && ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & 2) == 0))
                clStream <<	"<td width=\"10%\" bgcolor=\"#F8F8F8\">" << ptTestCell->GetCurrentLimitItem()->lfHighLimit  << " " << ptPatDef->m_strUnits << "</td>" << endl;
            else
                clStream <<	"<td width=\"10%\" bgcolor=\"#F8F8F8\">" << "n/a" "</td>" << endl;

            clStream <<	"<td width=\"10%\" bgcolor=\"#F8F8F8\">" << lfLowLimit  << " " << ptPatDef->m_strUnits << "</td>" << endl;
            clStream <<	"<td width=\"10%\" bgcolor=\"#F8F8F8\">" << lfHighLimit << " " << ptPatDef->m_strUnits << "</td>" << endl;
            clStream <<	"</tr>" << endl;
        }
        else
        if(mStation.GetPatOptions().mFT_Traceability == GEX_TPAT_TRACEABILITY_STDF)
        {
            StdfRecordHeader.iRecordType = 15;
            StdfRecordHeader.iRecordSubType = 10;
            cStdf.WriteHeader(&StdfRecordHeader);		// PTR header
            cStdf.WriteDword(ptTestCell->lTestNumber);	// TEST_NUM
            cStdf.WriteByte(1);							// HEAD_NUM
            cStdf.WriteByte(iSite);						// SITE_NUM
            cStdf.WriteByte(0);							// TEST_FLG
            cStdf.WriteByte(0);							// PARM_FLG
            cStdf.WriteFloat(0.0f);						// RESULT
            cStdf.WriteString( ptPatDef->m_strTestName.toLatin1().constData() ); // TEST Name
            cStdf.WriteString("");						// Alarm Name
            cStdf.WriteByte(2);							// OPT_FLG
            cStdf.WriteByte(0);							// RES_SCAL
            cStdf.WriteByte(0);							// LLM_SCAL
            cStdf.WriteByte(0);							// HLM_SCAL
            cStdf.WriteFloat((float)lfLowLimit);		// LLM
            cStdf.WriteFloat((float)lfHighLimit);		// HLM
            if(ptPatDef->m_strUnits.isEmpty() == false)
                cStdf.WriteString(ptPatDef->m_strUnits.toLatin1().constData());	// UNITS
            cStdf.WriteRecord();
        }

        // Keep track of total tests listed
        iTotalTests++;

        // Nest test cell
next_test_cell:
        ptTestCell = ptTestCell->GetNextTest();
    }

    if(mStation.GetPatOptions().mFT_Traceability == GEX_TPAT_TRACEABILITY_ASCII)
    {
        clStream <<	endl;
    }
    else
    if(mStation.GetPatOptions().mFT_Traceability == GEX_TPAT_TRACEABILITY_HTML)
    {
        clStream <<	"</table>" << endl;
    }
    else
    if(mStation.GetPatOptions().mFT_Traceability == GEX_TPAT_TRACEABILITY_STDF)
    {
        // Write PRR for this site
        StdfRecordHeader.iRecordType = 5;
        StdfRecordHeader.iRecordSubType = 20;
        cStdf.WriteHeader(&StdfRecordHeader);		// PTR header
        cStdf.WriteByte(1);							// HEAD_NUM
        cStdf.WriteByte(iSite);						// SITE_NUM
        cStdf.WriteByte(0);							// PART_FLG
        cStdf.WriteWord(iTotalTests);				// NUM_TEST
        cStdf.WriteWord(1);							// HARD_BIN
        cStdf.WriteWord(1);							// SOFT_BIN
        cStdf.WriteRecord();
    }

    // Close file
    if(mStation.GetPatOptions().mFT_Traceability == GEX_TPAT_TRACEABILITY_ASCII ||
        mStation.GetPatOptions().mFT_Traceability == GEX_TPAT_TRACEABILITY_HTML)
    {
        cFile.close();
    }
    else
    if(mStation.GetPatOptions().mFT_Traceability == GEX_TPAT_TRACEABILITY_STDF)
    {
        cStdf.Close();
    }
    GSLOG(6, "Trace dyn limits done.");
}

void ClientNode::TraceabilityFileOutlierSummaryTests(int SiteNb, QMap<QString,COutlierSummary*> & OutliersSummary,
                                                     QTextStream & lStream)
{
    int                                         lSeverityLimits=0;
    double                                      lfLowLimit=0,lfHighLimit=0,lfFactor=0;
    CPatDefinition*                             lPatDef=0;
    CTest*                                      lTestCell=0;
    QMap<QString,COutlierSummary*>::Iterator    it;
    COutlierSummary*                            lOutlierSummary=0;

    for ( it = OutliersSummary.begin();
          it != OutliersSummary.end(); ++it )
    {
        //lOutlierSummary = it.data();	// get handle to outlier summary entry
        lOutlierSummary = it.value();	// get handle to outlier summary entry

        // If this test belong to the PAT config file, then list it
        lTestCell = lOutlierSummary->ptTestCell;

        // Get handle to PAT definition for this test
        lPatDef = mStation.GetPatDefinition(lTestCell->lTestNumber, lTestCell->lPinmapIndex,
                                             lTestCell->strTestName);
        if(lPatDef != NULL)
        {
            GS::PAT::DynamicLimits lDynLimits = lPatDef->GetDynamicLimits(SiteNb);

            // Write test info into summary file
            switch(mStation.GetPatOptions().mFT_Traceability)
            {
                case GEX_TPAT_TRACEABILITY_ASCII:
                    lStream <<	lTestCell->lTestNumber << "\t";
                    lStream <<	lPatDef->m_strTestName << "\t";
                    lStream <<	QString::number(lOutlierSummary->iBaselineFails)  << "\t";
                    lStream <<	QString::number(lOutlierSummary->iProdFails) <<  "\t";
                    lStream <<	QString::number(lOutlierSummary->iBaselineFails+lOutlierSummary->iProdFails);
                    lStream << endl;
                    break;

                case GEX_TPAT_TRACEABILITY_HTML:
                    lStream <<	"<tr>" << endl;
                    lStream <<	"<td bgcolor=\"#F8F8F8\">" << lTestCell->lTestNumber << "</td>" << endl;
                    lStream <<	"<td bgcolor=\"#F8F8F8\">" << lPatDef->m_strTestName << "</td>" << endl;

                    lStream <<	"<td align=\"center\" bgcolor=\"#F8F8F8\">" << gexIgnoreSamplesSetItemsGUI[lPatDef->m_SamplesToIgnore]  << "</td>" << endl;
                    lStream <<	"<td align=\"center\" bgcolor=\"#F8F8F8\">" << gexKeepOutliersSetItemsGUI[lPatDef->m_OutliersToKeep]  << "</td>" << endl;
                    lStream <<	"<td align=\"center\" bgcolor=\"#F8F8F8\">" << gexOutlierLimitsSetItemsGUI[lPatDef->m_iOutlierLimitsSet]  << "</td>" << endl;
                    lStream <<	"<td align=\"center\" bgcolor=\"#F8F8F8\">" << gexRuleSetItemsGUI[lPatDef->mOutlierRule]  << "</td>" << endl;
                    lStream <<	"<td align=\"center\" bgcolor=\"#F8F8F8\">" << patlib_GetDistributionName(lPatDef->m_iDistributionShape) << "</td>" << endl;

                    lfFactor = patlib_GetOutlierFactor(mStation.GetPatInfo(),lPatDef,SiteNb,true);
                    if(lfFactor)
                        lStream <<	"<td align=\"center\" bgcolor=\"#F8F8F8\">" << QString::number(lfFactor,'f',2)  << "</td>" << endl;
                    else
                        lStream <<	"<td align=\"center\" bgcolor=\"#F8F8F8\">-</td>" << endl;

                    lfFactor = patlib_GetOutlierFactor(mStation.GetPatInfo(),lPatDef,SiteNb,false);
                    if(lfFactor)
                        lStream <<	"<td align=\"center\" bgcolor=\"#F8F8F8\">" << QString::number(lfFactor,'f',2)  << "</td>" << endl;
                    else
                        lStream <<	"<td align=\"center\" bgcolor=\"#F8F8F8\">-</td>" << endl;

                    // Provide new dynamic limits computed...check which severity level must be used!
                    lSeverityLimits = lPatDef->m_iOutlierLimitsSet;

                    // Get relevant limits set, as the STDF only allows one limit set, we need to specify the range of the two limits sets
                    lfLowLimit = gex_min(lDynLimits.mLowDynamicLimit1[lSeverityLimits],
                                         lDynLimits.mLowDynamicLimit2[lSeverityLimits]);

                    if(lfLowLimit < -GEX_TPAT_FLOAT_INFINITE)
                        lfLowLimit = -GEX_TPAT_FLOAT_INFINITE;

                    lfHighLimit = gex_max(lDynLimits.mHighDynamicLimit1[lSeverityLimits],
                                          lDynLimits.mHighDynamicLimit2[lSeverityLimits]);

                    if(lfHighLimit > GEX_TPAT_FLOAT_INFINITE)
                        lfHighLimit = GEX_TPAT_FLOAT_INFINITE;

                    // Write Test limits
                    lStream <<	"<td align=\"center\" bgcolor=\"#F8F8F8\">" << lfLowLimit  << " " << lPatDef->m_strUnits << "</td>" << endl;
                    lStream <<	"<td align=\"center\" bgcolor=\"#F8F8F8\">" << lfHighLimit << " " << lPatDef->m_strUnits << "</td>" << endl;

                    // Outlier counts
                    lStream <<	"<td align=\"center\" bgcolor=\"#F8F8F8\">" << QString::number(lOutlierSummary->iBaselineFails)  << "</td>" << endl;
                    lStream <<	"<td align=\"center\" bgcolor=\"#F8F8F8\">" << QString::number(lOutlierSummary->iProdFails) <<  "</td>" << endl;
                    lStream <<	"<td align=\"center\" bgcolor=\"#F8F8F8\">" << QString::number(lOutlierSummary->iBaselineFails+lOutlierSummary->iProdFails) << "</td>" << endl;
                    lStream <<	"</tr>" << endl;
                    break;

                case GEX_TPAT_TRACEABILITY_STDF:
                    break;

            }
        }
    }
}

void ClientNode::TraceabilityFileOutlierSummary(void)
{
    // If Traceability is disabled, return!
    if(mStation.GetPatOptions().mFT_Traceability == GEX_TPAT_TRACEABILITY_NONE)
        return;
    // If no valid file name (yet), return!
    if(mTraceabilityFile.isEmpty() == true)
        return;

    // For now, only Html format supported
    if(mStation.GetPatOptions().mFT_Traceability != GEX_TPAT_TRACEABILITY_HTML)
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Unsupported format %1 for GTM traceability file.")
              .arg(mStation.GetPatOptions().mFT_Traceability).toLatin1().constData());
        return;
    }

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Add outlier summary to traceability file %1").arg(mTraceabilityFile)
          .toLatin1().data());

    // Open traceability file...
    GS::StdLib::StdfRecordReadInfo StdfRecordHeader;
    GS::StdLib::Stdf cStdf;
    QFile cFile;
    QTextStream clStream;

    if(mStation.GetPatOptions().mFT_Traceability == GEX_TPAT_TRACEABILITY_STDF)
    {
        if(cStdf.Open(mTraceabilityFile.toLatin1().constData(), STDF_APPEND) != GS::StdLib::Stdf::NoError)
            return;

        StdfRecordHeader.iStdfVersion = 4;	// STDF V4
        #if defined unix || __MACH__
            StdfRecordHeader.iCpuType = 1;		// Sparc format
        #endif
        #ifdef _WIN32
            StdfRecordHeader.iCpuType = 2;		// Pc format
        #endif

        // Define the bit order to use when writing in STDF file.
        cStdf.SetStdfCpuType(StdfRecordHeader.iCpuType);
    }
    else
    if(mStation.GetPatOptions().mFT_Traceability == GEX_TPAT_TRACEABILITY_ASCII ||
        mStation.GetPatOptions().mFT_Traceability == GEX_TPAT_TRACEABILITY_HTML)
    {
        // If saving into ASCII or HTML file
        cFile.setFileName(mTraceabilityFile);
        if(cFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text) == FALSE)
            return;	// Failed opening traceability file...
        clStream.setDevice(&cFile);
    }

    // Summary Title
    switch(mStation.GetPatOptions().mFT_Traceability)
    {
        case GEX_TPAT_TRACEABILITY_ASCII:
            clStream <<	"Outlier summary:" << endl;
            break;
        case GEX_TPAT_TRACEABILITY_HTML:
            clStream <<	"<hr><h1>Outliers summary:</h1>" << endl;
            break;
        case GEX_TPAT_TRACEABILITY_STDF:
            break;
    }

    // Check if any outlier detected.
    if(mStation.cTestData.mSites.TotalRealOutlierParts() + mStation.cTestData.mSites.TotalVirtualOutlierParts() <= 0)
    {
        switch(mStation.GetPatOptions().mFT_Traceability)
        {
            case GEX_TPAT_TRACEABILITY_ASCII:
                clStream <<	"No outlier in this lot!" << endl;
                break;
            case GEX_TPAT_TRACEABILITY_HTML:
                clStream <<	"<b>No outlier in this lot!</b><br>" << endl;
                break;
            case GEX_TPAT_TRACEABILITY_STDF:
                break;
        }

        // Return
        return;
    }

    // Write summary table header
    float fValue=0.0f;
    switch(mStation.GetPatOptions().mFT_Traceability)
    {
        case GEX_TPAT_TRACEABILITY_ASCII:
            clStream <<	"Test number  Test name                  BaseLine fails  Prod. Fails  Total fails" << endl;
            clStream <<	"-----------  -------------------------- --------------- ------------ -----------"<< endl;
            break;

        case GEX_TPAT_TRACEABILITY_HTML:
            clStream <<	"<table border=\"1\" cellspacing=\"1\" style=\"border-collapse: collapse\" width=\"100%\">\n";
            // Virtual outlier parts in baseline
            clStream <<	"<tr>\n";
            clStream <<	"<td width=\"50%\" bgcolor=\"#CCECFF\"><b>Virtual outlier parts in baseline</b></td>\n";
            clStream <<	"<td width=\"50%\" bgcolor=\"#F8F8F8\"><b>"
              + QString::number(mStation.cTestData.mSites.TotalVirtualOutlierParts()) + "</b></td>\n";
            clStream <<	"</tr>\n";
            // Real outlier after baseline
            clStream <<	"<tr>\n";
            clStream <<	"<td width=\"50%\" bgcolor=\"#CCECFF\"><b>Real outlier parts after baseline</b></td>\n";
            clStream <<	"<td width=\"50%\" bgcolor=\"#F8F8F8\"><b>"
              + QString::number(mStation.cTestData.mSites.TotalRealOutlierParts()) + "</b></td>\n";
            clStream <<	"</tr>\n";
            // PAT Yield
            fValue = (100.0*(float)mStation.cTestData.mSites.TotalPassParts())
                    /(float)mStation.cTestData.mSites.TotalTestedParts();
            clStream <<	 "<tr>\n";
            clStream <<	 "<td width=\"50%\" bgcolor=\"#CCECFF\"><b>Yield</b></td>\n";
            clStream <<	 "<td width=\"50%\" bgcolor=\"#F8F8F8\"><b>"
              + QString::number(fValue,'f',2) + "%</b></td>\n";
            clStream <<	 "</tr>\n";
            // PAT Yield loss
            fValue = (100.0*(float)mStation.cTestData.mSites.TotalRealOutlierParts())
                    /(float)mStation.cTestData.mSites.TotalTestedParts();
            clStream <<	 "<tr>\n";
            clStream <<	 "<td width=\"50%\" bgcolor=\"#CCECFF\"><b>Yield loss (PAT Bins)</b></td>\n";
            clStream <<	 "<td width=\"50%\" bgcolor=\"#F8F8F8\"><b>"
              + QString::number(fValue,'f',2) + "%</b></td>\n";
            clStream <<	 "</tr>\n";
            clStream << "</table>\n&nbsp;<hr>\n";

            clStream <<	"<table border=\"1\" cellspacing=\"1\" style=\"border-collapse: collapse\" width=\"100%\">" << endl;
            clStream <<	"<tr>" << endl;
            clStream <<	"<td bgcolor=\"#CCECFF\"><b>Test#</b></td>" << endl;
            clStream <<	"<td bgcolor=\"#CCECFF\"><b>Test name</b></td>" << endl;
            clStream <<	"<td align=\"center\" bgcolor=\"#CCECFF\"><b>Samples<br>to ignore</b></td>" << endl;
            clStream <<	"<td align=\"center\" bgcolor=\"#CCECFF\"><b>Outliers<br>to keep</b></td>" << endl;
            clStream <<	"<td align=\"center\" bgcolor=\"#CCECFF\"><b>Outlier<br>Limits set</b></td>" << endl;
            clStream <<	"<td align=\"center\" bgcolor=\"#CCECFF\"><b>Outlier<br>Rule</b></td>" << endl;
            clStream <<	"<td align=\"center\" bgcolor=\"#CCECFF\"><b>Baseline<br>Shape</b></td>" << endl;
            clStream <<	"<td align=\"center\" bgcolor=\"#CCECFF\"><b>N factor</b></td>" << endl;
            clStream <<	"<td align=\"center\" bgcolor=\"#CCECFF\"><b>T factor</b></td>" << endl;
            clStream <<	"<td align=\"center\" bgcolor=\"#CCECFF\"><b>Dynamic PAT<br>Low Limit</b></td>" << endl;
            clStream <<	"<td align=\"center\" bgcolor=\"#CCECFF\"><b>Dynamic PAT<br>High Limit</b></td>" << endl;
            clStream <<	"<td align=\"center\" bgcolor=\"#CCECFF\"><b>Baseline<br>failures</b></td>" << endl;
            clStream <<	"<td align=\"center\" bgcolor=\"#CCECFF\"><b>Production<br>failures</b></td>" << endl;
            clStream <<	"<td align=\"center\" bgcolor=\"#CCECFF\"><b>Total<br>failures</b></td>" << endl;
            clStream <<	"</tr>" << endl;
            break;

        case GEX_TPAT_TRACEABILITY_STDF:
            break;
    }

    SiteList::Iterator itSite;
    itSite=mStation.cTestData.mSites.begin();
    if(itSite == mStation.cTestData.mSites.end())
        return;	// unexpected Failure: no tedting site available!

    // Get first site in use.
    int lSiteNb = itSite.key();

    TraceabilityFileOutlierSummaryTests(lSiteNb, mStation.cTestData.FT_OutliersSummary, clStream);

#if 0
    // Per Site summary
    for(itSite=mStation.cTestData.mSites.begin(); itSite!=mStation.cTestData.mSites.end(); ++itSite)
    {
        SiteTestResults* lSite = itSite.value();
        if(!lSite)
            continue;

        clStream <<	"<h2>Testing site: <b>" + QString::number(lSite->SiteNb()) + "</b></h2>" << endl;
        TraceabilityFileOutlierSummaryTests(lSite->SiteNb(), lSite->mOutliersSummary, clStream);
    }
#endif

    // Close report
    switch(mStation.GetPatOptions().mFT_Traceability)
    {
        case GEX_TPAT_TRACEABILITY_ASCII:
            clStream << endl << endl;
            break;

        case GEX_TPAT_TRACEABILITY_HTML:
            clStream <<	"</table>" << endl;
            clStream <<	"<br><br>" << endl;
            break;

        case GEX_TPAT_TRACEABILITY_STDF:
            break;
    }

    // Close file
    if(mStation.GetPatOptions().mFT_Traceability == GEX_TPAT_TRACEABILITY_ASCII ||
        mStation.GetPatOptions().mFT_Traceability == GEX_TPAT_TRACEABILITY_HTML)
    {
        cFile.close();
    }
    else
    if(mStation.GetPatOptions().mFT_Traceability == GEX_TPAT_TRACEABILITY_STDF)
    {
        cStdf.Close();
    }
}

void ClientNode::BuildStationInfoString(QString &strMessage, QString &strTitle, bool bHtmlFormat/*=true*/)
{
    QDateTime cDate = QDateTime::currentDateTime();
    if(bHtmlFormat)
    {
        // Build HTML title
        strMessage  = "<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" style=\"border-collapse: collapse\"";
        strMessage += "bordercolor=\"#111111\" width=\"100%\">\n";
        strMessage += "<tr>\n<td width=\"100%\">\n";
        strMessage += "<h1><font color=\"#000080\">" + strTitle + "</font></h1>\n";
        strMessage += "</td>\n</tr>\n</table>\n";

        // Insert global info: product, program, tester ID, etc...
        strMessage += "<table border=\"1\" cellspacing=\"1\" style=\"border-collapse: collapse\" width=\"100%\">\n";
        // Date
        strMessage += "<tr>\n";
        strMessage += "<td width=\"20%\" bgcolor=\"#CCECFF\"><b>Date</b></td>\n";
        strMessage += "<td width=\"88%\" bgcolor=\"#F8F8F8\"><b>" + cDate.toString() + "</b></td>\n";
        strMessage += "</tr>\n";
        // Lot starting date
        strMessage += "<tr>\n";
        strMessage += "<td width=\"20%\" bgcolor=\"#CCECFF\"><b>Lot started</b></td>\n";
        strMessage += "<td width=\"88%\" bgcolor=\"#F8F8F8\"><b>"
                + mStation.Get(CStationInfo::sLotStartedPropName).toString() + "</b></td>\n";
        strMessage += "</tr>\n";
        // Operator
        strMessage += "<tr>\n";
        strMessage += "<td width=\"20%\" bgcolor=\"#CCECFF\"><b>Operator</b></td>\n";
        strMessage += "<td width=\"88%\" bgcolor=\"#F8F8F8\"><b>"
                + mStation.Get(CStationInfo::sOperatorPropName).toString() + "</b></td>\n";
        strMessage += "</tr>\n";
        // Tester
        strMessage += "<tr>\n";
        strMessage += "<td width=\"20%\" bgcolor=\"#CCECFF\"><b>Tester</b></td>\n";
        strMessage += "<td width=\"88%\" bgcolor=\"#F8F8F8\"><b>"
                + mStation.Get(CStationInfo::sTesterNamePropName).toString() + "</b></td>\n";
        strMessage += "</tr>\n";
        // Station#
        strMessage += "<tr>\n";
        strMessage += "<td width=\"20%\" bgcolor=\"#CCECFF\"><b>Station</b></td>\n";
        strMessage += "<td width=\"88%\" bgcolor=\"#F8F8F8\"><b>"
                + QString::number(mStation.GetStationNb())+ "</b></td>\n";
        strMessage += "</tr>\n";
        // Product
        strMessage += "<tr>\n";
        strMessage += "<td width=\"20%\" bgcolor=\"#CCECFF\"><b>Product</b></td>\n";
        strMessage += "<td width=\"88%\" bgcolor=\"#F8F8F8\"><b>"
                + mStation.Get(CStationInfo::sProductPropName).toString() + "</b></td>\n";
        strMessage += "</tr>\n";
        // Lot
        strMessage += "<tr>\n";
        strMessage += "<td width=\"20%\" bgcolor=\"#CCECFF\"><b>Lot</b></td>\n";
        strMessage += "<td width=\"88%\" bgcolor=\"#F8F8F8\"><b>"
                + mStation.Get(CStationInfo::sLotPropName).toString()+ "</b></td>\n";
        strMessage += "</tr>\n";
        // Program name
        strMessage += "<tr>\n";
        strMessage += "<td width=\"20%\" bgcolor=\"#CCECFF\"><b>Program name</b></td>\n";
        strMessage += "<td width=\"88%\" bgcolor=\"#F8F8F8\"><b>"
                + mStation.Get(CStationInfo::sJobNamePropName).toString() + "</b></td>\n";
        strMessage += "</tr>\n";
        // Program Rev.
        strMessage += "<tr>\n";
        strMessage += "<td width=\"20%\" bgcolor=\"#CCECFF\"><b>Program rev.</b></td>\n";
        strMessage += "<td width=\"88%\" bgcolor=\"#F8F8F8\"><b>"
                + mStation.Get(CStationInfo::sJobRevPropName).toString()+ "</b></td>\n";
        strMessage += "</tr>\n";

        strMessage += "</table>\n&nbsp;<hr>\n";
    }
    else
    {
        strMessage  = strTitle + "\n";
        strMessage += "======================================================\n\n";
        if (!getenv("GS_QA"))
        {
            strMessage += "Date        : " + cDate.toString() + "\n";
            strMessage += "Lot started : " + mStation.Get(CStationInfo::sLotStartedPropName).toString() + "\n";
        }
        strMessage += "Operator    : " + mStation.Get(CStationInfo::sOperatorPropName).toString() + "\n";
        strMessage += "Tester      : " + mStation.Get(CStationInfo::sTesterNamePropName).toString() + "\n";
        strMessage += "Station     : " + QString::number(mStation.GetStationNb()) + "\n";
        strMessage += "Product     : " + mStation.Get(CStationInfo::sProductPropName).toString() + "\n";
        strMessage += "Lot         : " + mStation.Get(CStationInfo::sLotPropName).toString() + "\n";
        strMessage += "Program name: " + mStation.Get(CStationInfo::sJobNamePropName).toString() + "\n";
        strMessage += "Program rev.: " + mStation.Get(CStationInfo::sJobRevPropName).toString() + "\n";
        strMessage += "======================================================\n\n";
    }
}

bool ClientNode::SendEmailMessage(QString &strMessage)
{
    // If no email address or mailing list defined, quietly exit
    if(mStation.GetPatOptions().strFT_MailingList.isEmpty())
        return true;

    // Prepare Email message
    QString	strTitle = "PAT-MAN Notification";
    QString	strEmailMessage,strString;

    if(mStation.GetPatOptions().mFT_EmailFormat == GEX_PAT_EMAIL_ASCII)
    {
        // Email format: ASCII
        BuildStationInfoString(strEmailMessage,strTitle,false);

        // Remove HTML keyword from error message
        cleanHtmlString(strMessage);

        // Add message content
        strEmailMessage += strMessage;
    }
    else
    {
        // Email header
        strEmailMessage  = "<html>\n";
        strEmailMessage += "<head>\n";
        strEmailMessage += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=ISO-8859-1\">\n";
        strEmailMessage += "<title>";
        strEmailMessage += strTitle;
        strEmailMessage += "</title>\n";
        strEmailMessage += "</head>\n";
        strEmailMessage += "<body bgcolor=\"#FFFFFF\" text=\"#000000\">\n";

        // Email format: HTML
        BuildStationInfoString(strString,strTitle);
        strEmailMessage += strString;

        // Add message content
        strEmailMessage += strMessage;
    }

    // Check email format
    QString strEmailFormat;
    if(mStation.GetPatOptions().mFT_EmailFormat == GEX_PAT_EMAIL_HTML)
        strEmailFormat = "HTML";
    else
        strEmailFormat = "TEXT";

    // Create email file in spooling folder.
    // Name: YYYYMMDD_HHMMSS_MS.mail
    QDir cDir;
    QDateTime cCurrentDateTime=QDateTime::currentDateTime();
    QString lGsFolder = GS::Gex::Engine::GetInstance().Get("GalaxySemiFolder").toString();
    QString strEmailFile = lGsFolder + GTM_DIR_EMAILS;
    cDir.mkdir(strEmailFile);	// <gtm>/spool/emails
    strEmailFile += cCurrentDateTime.toString("/yyyyMMdd_hhmmss_zzz");
    strEmailFile += QString::number(m_iMessageID,16);
    strEmailFile += ".mail";
    strEmailFile += GTM_TEMPORARY_EMAIL;
    QString strEmailBody = lGsFolder + GTM_DIR_EMAILS;
    strEmailBody += cCurrentDateTime.toString("/yyyyMMdd_hhmmss_zzz");
    strEmailBody += QString::number(m_iMessageID,16);
    strEmailBody += ".body";

    // Update Message ID
    m_iMessageID++;

    // Create Email description file
    QFile file( strEmailFile );
    if (file.open(QIODevice::WriteOnly) == false)
        return false;	// Failed opening email file.

    QTextStream stream( &file );
    stream << "Type=PatmanReport" << endl;				// Report type
    stream << "Format=" << strEmailFormat << endl;		// Email format "HTML" or "TEXT"
    stream << "To=" << mStation.GetPatOptions().strFT_MailingList << endl;	// Destination: email list, or mailing list file
    stream << "From=" << "support@mentor.com" << endl;				// Sender name.
    stream << "Subject=" << strTitle << endl;			// Subject
    stream << "Body=" <<  strEmailBody << endl;			// File holding the "Body"
    #if 0
        if(strAttachments.isEmpty() == false)
            stream << "Attachment=" <<  strAttachments << endl;	// Files to attach
    #endif
    // Close Email description file.
    file.close();

    // Create Email body file
    file.setFileName(strEmailBody);
    if (file.open(QIODevice::WriteOnly) == false)
    {
        cDir.remove(strEmailFile);	// Remove email description file just created
        return false;	// Failed opening body file.
    }

    // Write Body file, then close it.
    stream.setDevice(&file);
    stream << strEmailMessage << endl;

    QString strApplicationName = GS::Gex::Engine::GetInstance().Get("AppFullName").toString();

    // Email footer. Examinator Version.
    if(mStation.GetPatOptions().mFT_EmailFormat == GEX_PAT_EMAIL_HTML)
    {
        stream << "<br>"  << endl;
        stream << "<br>"  << endl;
        stream << "<hr>"  << endl;
        stream << "&nbsp;"  << endl;
        stream << "Email from: "  << strApplicationName << endl;
        stream << "<br>&nbsp;"  << endl;
        stream << "&quot;Semiconductor Intelligence (tm)&quot;<br>"  << endl;
        stream << "<br>"  << endl;
        stream << "&nbsp; <a href=\"http://www.mentor.com\">www.mentor.com</a>&nbsp; "  << endl;
        stream << "<hr>"  << endl;
        stream << "</body>" << endl;
        stream << "</html>" << endl;
    }
    else
    {
        stream << endl << "============================================================" << endl;
        stream <<  "  Email from: ";
        stream <<  strApplicationName << endl;
        stream <<  "  \"Semiconductor Intelligence (tm)\"" << endl << endl;
        stream <<  "  www.mentor.com" << endl;
        stream <<  "============================================================" << endl;
    }
    file.close();

    // Rename Email description to correct extension...so spooling mailing daemons can capture the new email!
    QString strDest = strEmailFile;
    //int iIndex = strDest.findRev(GTM_TEMPORARY_EMAIL); // Qt3
    int iIndex = strDest.lastIndexOf(GTM_TEMPORARY_EMAIL, -1, Qt::CaseSensitive);
    strDest = strDest.left(iIndex);

    // Erase destination
    cDir.remove(strDest);
    // Rename source to destination
    if(cDir.rename(strEmailFile,strDest) == true)
        return true;	// Success
    else
        return false;
}


QString ClientNode::DisplayMessage(int errorType,
  QString lMessage, bool bSendEmail/*=false*/, bool bNotifyTester/*=false*/, int lSeverity/*=0*/)
{
    // errorType: GTM_ERRORTYPE_INFO, GTM_ERRORTYPE_WARNING or GTM_ERRORTYPE_CRITICAL
    if (errorType!=GTM_ERRORTYPE_INFO)
        GSLOG(5, lMessage.toLatin1().data() );
    QDateTime cDateTime = QDateTime::currentDateTime();	// Current time.
    QString	strDate = cDateTime.toString("[d MMM h:mm] ");	// 9 Jan HH:MM
    QString strError;
    //int	iWidgetStackID=0;	// To hold Widget ID to raise

    // Make sure the line always ends with a line feed.
    lMessage = lMessage.replace("\n","<br>");	// make sure text is HTML type.
    if(lMessage.endsWith("<br>") == false)
        lMessage += "<br>";

    switch(errorType)
    {
        case GTM_ERRORTYPE_INFO:
            if (!getenv("GS_QA"))
                strError = strDate;
            strError += lMessage;
            lSeverity=1;
            break;
        case GTM_ERRORTYPE_WARNING:
            strError = strDate + "Warning: " + lMessage;
            lSeverity=0;
            break;
        case GTM_ERRORTYPE_CRITICAL:
            strError = strDate + "Error: " + lMessage;
            lSeverity=-1;
            break;
    }

    emit sNewMessage(errorType, lMessage, lSeverity); // Should be catch by the GUI

    // Raise relevant widget unless severity is lower than a previous message!
    if(errorType >= mStation.cTestData.mErrorLevel)
    {
        // Keep track of top severity level found in session.
        mStation.cTestData.mErrorLevel = errorType;
    }

    // Send string to Traceability output file (if enabled)
    TraceabilityMessage(strError);

    // If Warning or Critical error, send email notification (unless email disabled)
    if(bSendEmail)
        if (!SendEmailMessage(strError))
            GSLOG(3, "SendEmailMessage failed");

    QString lR("ok");

    // Send ASCII version of the message to the tester (if requested)
    if(bNotifyTester)
    {
        // Remove HTML keyword from text
        cleanHtmlString(lMessage);
        // Insert prefix string to ease operator spoting the message
        lMessage = "[PAT-Message] : " + lMessage;
        // Send message to Tester
        lR=NotifyTester(lMessage, lSeverity);
        if (lR.startsWith("err"))
          GSLOG(3, QString("Failed to notify tester: %1").arg(lR).toLatin1().data() );
    }
    return lR;
}

void ClientNode::DisableDynamicPat(const QList<int> & Sites)
{
    GS::Gex::SiteTestResults *lSite=0;
    QString lMessage;
    for (int i = 0; i < Sites.size(); ++i)
    {
        // Get site ptr.
        lSite = FindSite(Sites.at(i));
        if(lSite)
        {
            // Prepare message
            lMessage = QString("<font color=\"#ff0000\"><b>Site %1: failed computing Dynamic PAT limits</b></font>")
                    .arg(lSite->SiteNb());
            lMessage += "<br>Using Static PAT limits only...";

            // Send message to GTM
            DisplayMessage(GTM_ERRORTYPE_CRITICAL,lMessage);

            // Disabled PAT feature: switch running mode + notify tester (send all PAT limits to infinite)
            DisableDynamicPat(lSite);
        }
    };
}

void ClientNode::DisableDynamicPat(GS::Gex::SiteTestResults* Site)
{
    if(!Site)
    {
        GSLOG(3, "Disable Dynamic Pat impossible because site null");
        return;
    }

    GSLOG(SYSLOG_SEV_NOTICE, QString("Disabling Dynamic PAT for site %1...").arg(Site->SiteNb()).toLatin1().constData() );
    Site->SetSiteState(GS::Gex::SiteTestResults::SITESTATE_DISABLED);

    // If tester requested to no longer receive alram notification, return now! (eg: if flushing last few runs when issuing end-of-lot)
    if(m_uiSilentMode)
        return;

    // Resend new test limits to tester (will be infinite as PAT is disabled!)
    clientPatInit_Dynamic(Site);
}

// New limits just computed. Perform post-compute actions.
QString ClientNode::ApplyDynamicPatLimits(GS::Gex::SiteTestResults* Site, unsigned int NbSitesUsedToCompute/*=0*/)
{   
    if(!Site)
        return "error: given site null";

    //QMutexLocker lML(&mMutex);

    //QElapsedTimer lElTimer;
    //lElTimer.start();

    DisplayMessage(GTM_ERRORTYPE_INFO,QString(
                   "<font color=\"#00c000\"><b>Site %1: Computed DPAT limits using %2 sites ...</b></font>")
                   .arg(Site->SiteNb()).arg(NbSitesUsedToCompute));

    // Parts since last limits
    Site->SLLParts().Reset();

    // Nb of baselines
    if(Site->SiteState() == GS::Gex::SiteTestResults::SITESTATE_BASELINE)
        Site->IncBaselineCount();

    // Nb of tunings
    if(Site->SiteState() == GS::Gex::SiteTestResults::SITESTATE_DPAT)
        Site->IncTuningCount();

    // Check for total outliers detected...display error if needed
    if(CheckForOutliersAlarm(Site) == true)
        // Outliers alarm detected
        //return "error: failed to check for outliers alarm"; // was false	// Error in baseline processing.
        return "ok";

    // Send new PAT limits to the tester
    clientPatInit_Dynamic(Site);

#if 0  // CODEME
    // Save distribution shape detected for each test during base line
    SaveDistributionShape(Site);
#endif

    // Write PAT Limits to Traceability file
    TraceabilityDynamicLimits(Site);

#if 0  // CODEME
    // Check if PAT-Median drift exceeds a given user-defined threshold...
    if(CheckForPatDriftAlert(Site) == true)
    {
        // Disabled PAT feature: switch running mode + notify tester (send all PAT limits to infinite)
        DisableDynamicPat(Site);
        // PAT disabled
        return "ok";
    }
#endif

    if(Site->SiteState() == GS::Gex::SiteTestResults::SITESTATE_BASELINE)
    {
        // Base line reached: now start production
        DisplayMessage(GTM_ERRORTYPE_INFO,QString("<font color=\"#00c000\"><b>Site %1: PAT Testing enabled...</b></font>")
                       .arg(Site->SiteNb()));
        Site->SetSiteState(GS::Gex::SiteTestResults::SITESTATE_DPAT);
    }

    //setProperty(sPropTotalComputationTime.toLatin1().data(),
      //          property(sPropTotalComputationTime.toLatin1().data()).toUInt()+lElTimer.elapsed() );

    return "ok";
}

// Check if need to compute dynamic limits on any site using single site algo
// (either end of baseline, or too many outliers, or N devices tested)
QString ClientNode::CheckForComputeDynamicLimits_SingleSiteAlgo()
{
    // If PAT disabled, nothing to do
    if(mStation.GetStationStatus() != CStation::STATION_ENABLED)
        return "ok";

    QElapsedTimer lElTimer;
    lElTimer.start();

    // Build list of sites for which dyn limits should be computed
    GS::Gex::SiteTestResults*   lSite=0;
    QString                     lMessage;
    for(GS::Gex::SiteList::Iterator it= mStation.cTestData.mSites.begin();it!= mStation.cTestData.mSites.end();++it)
    {
        // Get site ptr.
        lSite = it.value();
        if(!lSite)
            continue;

        // Check site state
        bool ComputeDynLimits=false;
        switch(lSite->SiteState())
        {
            default:
            case GS::Gex::SiteTestResults::SITESTATE_DISABLED:
                // PAT disabled on this site: nothing to do
                break;

            case GS::Gex::SiteTestResults::SITESTATE_BASELINE:
                if(lSite->RBValidParts() < (unsigned int)mStation.GetPatOptions().mFT_BaseLine)
                {
                    // Not enough parts processed yet!
                    // Update display of current number of parts tested in baseline...
                    float	fPercentage = (100.0F*(float)lSite->RBValidParts()) / (float)mStation.GetPatOptions().mFT_BaseLine;
                    lMessage = QString("<b>Site %1: building baseline (%2%)</b>(%3 parts collected of %4)")
                            .arg(lSite->SiteNb()).arg(fPercentage,0,'f',0)
                            .arg(lSite->RBValidParts()).arg(mStation.GetPatOptions().mFT_BaseLine);

                    // Send message to GTM window
                    DisplayMessage(GTM_ERRORTYPE_INFO, lMessage);
                }
                else
                    // Dyn limits must be computed for this site
                    ComputeDynLimits=true;
                break;

            case GS::Gex::SiteTestResults::SITESTATE_DPAT:
                // Make sure RB is full before doing any tuning
                if(lSite->RBValidParts() < lSite->RBSize())
                    break;

                // In production, check if Tuning of Dynamic limits is enabled...
                if(mStation.GetPatOptions().mFT_TuningIsEnabled && (mStation.GetPatOptions().mFT_Tuning > 0))
                {
                    switch(mStation.GetPatOptions().mFT_TuningType)
                    {
                        default:
                        case FT_PAT_TUNING_EVERY_N_PARTS:		// Tune every 'N' parts
                            if(lSite->SLLParts().TestedParts() >= (unsigned int)mStation.GetPatOptions().mFT_Tuning)
                            {
                                // Time for tuning!
                                // Non-blocking Info message sent both to GTM and tester windows
                                lMessage = QString("Site %1: periodic Dynamic PAT limits update...")
                                        .arg(lSite->SiteNb());
                                DisplayMessage(GTM_ERRORTYPE_INFO,lMessage,false,true);
                                ComputeDynLimits=true;
                            }
                            break;

                        case FT_PAT_TUNING_EVERY_N_OUTLIERS:		// Tune if more than 'N' outliers
                            if(lSite->SLLParts().RealOutlierParts() >= (unsigned int)mStation.GetPatOptions().mFT_Tuning)
                            {
                                // Time for tuning!
                                // Non-blocking Warning message sent both to GTM and tester windows
                                lMessage = QString("Site %1: too many outliers, updating Dynamic PAT limits...")
                                        .arg(lSite->SiteNb());
                                DisplayMessage(GTM_ERRORTYPE_INFO,lMessage,false,true);
                                ComputeDynLimits=true;
                            }
                            break;
                    }
                }
                break;
        }

        if(!ComputeDynLimits)
            continue;

        // Compute dyn limits and update status and counters
        if(mStation.ComputeDynamicPatLimits(lSite) == false)
        {
            // Error while computing Dynamic PAT limits...start production, but stick to Static limits only.
            DisableDynamicPat(lSite);
            return QString("error: failed to compute dyn PAT limits for site %1").arg(lSite->SiteNb());
        }

        // Compute post-compute actions
        ApplyDynamicPatLimits(lSite, 1);
    };

    setProperty(sPropTotalComputationTime.toLatin1().data(),
                property(sPropTotalComputationTime.toLatin1().data()).toUInt()+lElTimer.elapsed() );

    return "ok";
}

// Check if need to compute dynamic limits on any site using merge sites algo
// (either end of baseline, or too many outliers, or N devices tested)
QString ClientNode::CheckForComputeDynamicLimits_MergeSitesAlgo()
{
    QList<int>  lBLSitesToCompute;
    QList<int>  lSitesToUse;
    GS::Gex::SiteTestResults*   lSite=0;
    QString                     lMessage;
    // CODEME: use recipe value instead of hard-coded 10
    unsigned int                lMinParts=mStation.GetPatOptions().mFT_MinSamplesPerSite;

    // If PAT disabled, nothing to do
    if(mStation.GetStationStatus() != CStation::STATION_ENABLED)
        return "ok";

    // 1) build list of all sites that could be used by merge algo
    for(GS::Gex::SiteList::Iterator it= mStation.cTestData.mSites.begin();it!= mStation.cTestData.mSites.end();++it)
    {
        // Get site ptr.
        lSite = it.value();
        if(!lSite)
            continue;
        if(lSite->RBValidParts() >= lMinParts)
            lSitesToUse.append(lSite->SiteNb());
    }

    // 2) check sites still in Baseline
    // 2.1) Build list of sites for which dyn limits should be computed
    for(GS::Gex::SiteList::Iterator it= mStation.cTestData.mSites.begin();it!= mStation.cTestData.mSites.end();++it)
    {
        // Get site ptr.
        lSite = it.value();
        if(!lSite)
            continue;

        // Check if site in Baseline
        if(lSite->SiteState() == GS::Gex::SiteTestResults::SITESTATE_BASELINE)
        {
            if((lSite->RBValidParts() >= lMinParts) &&
                    (mStation.cTestData.mSites.RBTotalValidParts(lMinParts) >= mStation.GetPatOptions().mFT_BaseLine))
                lBLSitesToCompute.append(lSite->SiteNb());
            else
            {
                // Not enough parts processed yet!
                lMessage = QString("<b>Site %1: building baseline</b> (Site: %2 parts of %3, Total: %4 parts of %5)")
                        .arg(lSite->SiteNb()).arg(lSite->RBValidParts()).arg(lMinParts)
                        .arg(mStation.cTestData.mSites.RBTotalValidParts(lMinParts))
                        .arg(mStation.GetPatOptions().mFT_BaseLine);

                // Send message to GTM window
                DisplayMessage(GTM_ERRORTYPE_INFO, lMessage);
            }
        }
    }

    // 2.2) Check if some limits to be computed using merge algorithm
    if(!lBLSitesToCompute.isEmpty())
    {
        // Compute dyn limits and update status and counters
        if(mStation.ComputeDynamicPatLimits(lBLSitesToCompute, lSitesToUse) == false)
        {
            // Error while computing Dynamic PAT limits...start production, but stick to Static limits only.
            DisableDynamicPat(lBLSitesToCompute);
            return QString("error: failed to compute dyn PAT limits on %1 sites").arg(lBLSitesToCompute.count());
        }

        // Post-compute actions
        for (int i = 0; i < lBLSitesToCompute.size(); ++i)
        {
            // Get site ptr.
            lSite = FindSite(lBLSitesToCompute.at(i));
            if(lSite)
            {
                ApplyDynamicPatLimits(lSite, lSitesToUse.count());

                // Need to wait for early tuning ?
                if(lSite->RBValidParts() < (unsigned int) mStation.GetPatOptions().mFT_SubsequentBL)
                    lSite->SetWaitForEarlyTuning(true);
            }
        }
    }

    // 3) Check for DPAT sites requiring tuning
    for(GS::Gex::SiteList::Iterator it= mStation.cTestData.mSites.begin();it!= mStation.cTestData.mSites.end();++it)
    {
        // Get site ptr.
        lSite = it.value();
        if(!lSite)
            continue;

        // Maybe this site has just been computed
        if(lBLSitesToCompute.contains(lSite->SiteNb()))
            continue;

        // If not in DPAT mode, continue
        bool ComputeDynLimits=false;
        if(lSite->SiteState() != GS::Gex::SiteTestResults::SITESTATE_DPAT)
            continue;

        // If not reached subsequent baseline parts, continue
        if(lSite->RBValidParts() < (unsigned int) mStation.GetPatOptions().mFT_SubsequentBL)
            continue;

        // Check if early tuning should be performed
        if(lSite->WaitForEarlyTuning())
                ComputeDynLimits=true;
        else if(mStation.GetPatOptions().mFT_TuningIsEnabled && (mStation.GetPatOptions().mFT_Tuning > 0))
        {
            // Make sure RB is full before doing any tuning
            if(lSite->RBValidParts() < lSite->RBSize())
                continue;

            // In production, after early tuning, check if Tuning of Dynamic limits is enabled...
            switch(mStation.GetPatOptions().mFT_TuningType)
            {
                default:
                case FT_PAT_TUNING_EVERY_N_PARTS:		// Tune every 'N' parts
                    if(lSite->SLLParts().TestedParts() >= (unsigned int)mStation.GetPatOptions().mFT_Tuning)
                    {
                        // Time for tuning!
                        // Non-blocking Info message sent both to GTM and tester windows
                        lMessage = QString("Site %1: periodic Dynamic PAT limits update...")
                                .arg(lSite->SiteNb());
                        DisplayMessage(GTM_ERRORTYPE_INFO,lMessage,false,true);
                        ComputeDynLimits=true;
                    }
                    break;

                case FT_PAT_TUNING_EVERY_N_OUTLIERS:		// Tune if more than 'N' outliers
                    if(lSite->SLLParts().RealOutlierParts() >= (unsigned int)mStation.GetPatOptions().mFT_Tuning)
                    {
                        // Time for tuning!
                        // Non-blocking Warning message sent both to GTM and tester windows
                        lMessage = QString("Site %1: too many outliers, updating Dynamic PAT limits...")
                                .arg(lSite->SiteNb());
                        DisplayMessage(GTM_ERRORTYPE_INFO,lMessage,false,true);
                        ComputeDynLimits=true;
                    }
                    break;
            }
        }

        if(!ComputeDynLimits)
            continue;

        // Compute dyn limits and update status and counters
        if(mStation.ComputeDynamicPatLimits(lSite) == false)
        {
            // Error while computing Dynamic PAT limits...start production, but stick to Static limits only.
            DisableDynamicPat(lSite);
            return QString("error: failed to compute dyn PAT limits for site %1").arg(lSite->SiteNb());
        }

        // Compute post-compute actions
        ApplyDynamicPatLimits(lSite, 1);

        // Reset early tuning
        lSite->SetWaitForEarlyTuning(false);
    }

    return "ok";
}

// Check if need to compute dynamic limits on any site
// (either end of baseline, or too many outliers, or N devices tested)
QString ClientNode::CheckForComputeDynamicLimits()
{
    // If PAT disabled, nothing to do
    if(mStation.GetStationStatus() != CStation::STATION_ENABLED)
        return "ok";

    // Check sites for which dyn mimits have to be computed, using desired algorithm
    QString     lR;
    switch(mStation.GetPatOptions().mFT_BaseLineAlgo)
    {
        case FT_PAT_SINGLE_SITE_ALGO:
            lR=CheckForComputeDynamicLimits_SingleSiteAlgo();
            break;

        case FT_PAT_MERGED_SITE_ALGO:
            lR=CheckForComputeDynamicLimits_MergeSitesAlgo();
            break;

        default:
            lR=QString("error: unknown algorithm %1.").arg(mStation.GetPatOptions().mFT_BaseLineAlgo);
            break;
    }

    return lR;
}

///////////////////////////////////////////////////////////
// Check for outlier during production...
///////////////////////////////////////////////////////////
QString	ClientNode::CheckForProductionOutlier(CTest* TestCell, GS::Gex::SiteTestResults* Site,
                                              PT_GNM_RUNRESULT BinResult, PT_GNM_TESTRESULT TestResult)
{
    // Check pointers
    if(!Site || !TestCell)
        return "error: site or testcell null";

    QElapsedTimer lElTimer;
    lElTimer.start();

    int lSiteNb = Site->SiteNb();

    if((mStation.GetStationStatus() != CStation::STATION_ENABLED) ||
        (Site->SiteState() != GS::Gex::SiteTestResults::SITESTATE_DPAT))
        return "ok: nothing to do";

    // If this run doesn't include outliers, return.
    if(BinResult->mOrgSoftBin == BinResult->mPatSoftBin)
        return "ok";

    // If test is not an outlier, return.
    if((TestResult->mValue == GTL_INVALID_VALUE_FLOAT) || !(TestResult->mFlags & GTC_TRFLAG_VALID) ||
            !(TestResult->mFlags & (GTC_TRFLAG_SPAT_OUTLIER|GTC_TRFLAG_DPAT_OUTLIER)))
        return "ok";

    // We have an outlier: add it to outlier summary for this test
    QString	lOutliersIdentified;
    CPatDefinition *lPatDef=0;

    // Get handle to PAT definition for this test
    lPatDef = mStation.GetPatDefinition(TestCell->lTestNumber, TestCell->lPinmapIndex,
                                         TestCell->strTestName);
    if(lPatDef == NULL)
        return "ok"; // Bernard : is it an error / normal ?

    // Update the outlier summary structure of this test.
    COutlierSummary *pOutlierSummary=0;	// Structure to hold/update outlier count
    QString strString = QString::number(TestCell->lTestNumber) + "." + TestCell->strTestName;
    pOutlierSummary = mStation.cTestData.FT_OutliersSummary[strString];
    if(pOutlierSummary == NULL)
    {
        // Test never failed before...create structure
        pOutlierSummary = new COutlierSummary();
        pOutlierSummary->ptTestCell = TestCell;
        pOutlierSummary->iProdFails = 1;
    }
    else
        pOutlierSummary->iProdFails++;
    mStation.cTestData.FT_OutliersSummary[strString] = pOutlierSummary;

    // If option to display outliers identified, show it!
    int iSeverityLimits = lPatDef->m_iOutlierLimitsSet;

    lOutliersIdentified += QString("[Site %1, PartID %2] <b>T%3: %4</b> LL: %5, HL: %6")
            .arg(lSiteNb).arg(BinResult->mPartID).arg(TestCell->lTestNumber).arg(TestCell->strTestName)
            .arg(lPatDef->mDynamicLimits[lSiteNb].mLowDynamicLimit1[iSeverityLimits])
            .arg(lPatDef->mDynamicLimits[lSiteNb].mHighDynamicLimit1[iSeverityLimits]);

    // If we have two sets of limits, display the other set too!
    if((lPatDef->mDynamicLimits[lSiteNb].mHighDynamicLimit2[iSeverityLimits]
            != lPatDef->mDynamicLimits[lSiteNb].mHighDynamicLimit1[iSeverityLimits]) &&
        (lPatDef->mDynamicLimits[lSiteNb].mHighDynamicLimit2[iSeverityLimits] < GEX_TPAT_FLOAT_INFINITE))
    {
        lOutliersIdentified += QString("LL2: %1, HL2: %2")
                .arg(lPatDef->mDynamicLimits[lSiteNb].mLowDynamicLimit2[iSeverityLimits])
                .arg(lPatDef->mDynamicLimits[lSiteNb].mHighDynamicLimit2[iSeverityLimits]);
    }

    lOutliersIdentified += QString(" value: %1 %2<br>").arg(TestResult->mValue).arg(lPatDef->m_strUnits);
    DisplayMessage(GTM_ERRORTYPE_INFO,lOutliersIdentified);

    setProperty(sPropTotalComputationTime.toLatin1().data(),
                property(sPropTotalComputationTime.toLatin1().data()).toUInt()+lElTimer.elapsed() );

    return "ok";
}

///////////////////////////////////////////////////////////
// Check for 'Too many Outliers in baseline/production' alert for specified site (all parts)
// Returns true if alarm detected
bool ClientNode::CheckForOutliersAlarm(GS::Gex::SiteTestResults* Site)
{
    // Check pointer
    if(!Site)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Invalid site ptr.");
        return false;
    }

    // Check if PAT Disabled
    if(mStation.GetStationStatus() != CStation::STATION_ENABLED)
        return false;

    // Only check baseline virtual outliers for now
    if(Site->SiteState() != GS::Gex::SiteTestResults::SITESTATE_BASELINE)
        return false;

    int                 lSiteNb = Site->SiteNb();
    bool                lDisplayOutliers=true;
    QString             lOutliersIdentified="<br>Outliers identified in baseline:<br>";
    QList<int>          lOutlierParts;
    COutlierSummary     *lOutlierSummary=0;	// Structure to hold/update outlier count
    QString             lString;
    float               lResult;
    CPatDefinition*     lPatDef=0;
    CTest*              lTC=0;

    // Test loop.
    lTC = Site->TestList();
    while(lTC != NULL)
    {
        // Get handle to PAT definition for this test
        lPatDef = mStation.GetPatDefinition(lTC->lTestNumber, lTC->lPinmapIndex,
                                             lTC->strTestName);
        if(!lPatDef)
        {
            // CHECKME: error message
        }
        else
        {
            for(unsigned int lRunIndex=0;lRunIndex < Site->RBSize(); lRunIndex++)
            {
                // Get part result
                const PartResult *lPR = Site->GetPartResult(lRunIndex);
                if(!lPR)
                    continue;

                // Only check for outliers on good bins
                if(!mStation.GetPatInfo()->GetRecipeOptions().pGoodSoftBinsList->Contains(lPR->OrgSoftBin()))
                    continue;

                // Make sure we have a valid test result
                if(!lTC->m_testResult.isValidIndex(lRunIndex) || !lTC->m_testResult.isValidResultAt(lRunIndex))
                    continue;

                // Good Part & valid result, check if test result executed and is an outlier
                int lSeverity = lPatDef->m_iOutlierLimitsSet;

                lResult = lTC->m_testResult.resultAt(lRunIndex);
                if((lResult <= lPatDef->mDynamicLimits[lSiteNb].mHighDynamicLimit1[lSeverity] &&
                   lResult >= lPatDef->mDynamicLimits[lSiteNb].mLowDynamicLimit1[lSeverity]) ||
                   (lResult <= lPatDef->mDynamicLimits[lSiteNb].mHighDynamicLimit2[lSeverity] &&
                   lResult >= lPatDef->mDynamicLimits[lSiteNb].mLowDynamicLimit2[lSeverity]))
                {
                    // This value is within the outlier limits set, so do not fail it!
                }
                else
                {
                    // Mark this part as outlier
                    if(!lOutlierParts.contains(lRunIndex))
                        lOutlierParts.append(lRunIndex);

                    // Update the outlier summary structure of this test.
                    lString = QString::number(lTC->lTestNumber) + "." + lTC->strTestName;
                    lOutlierSummary = mStation.cTestData.FT_OutliersSummary[lString];
                    if(lOutlierSummary == NULL)
                    {
                        // Test never failed before...create structure
                        lOutlierSummary = new COutlierSummary();
                        lOutlierSummary->ptTestCell = lTC;
                        lOutlierSummary->iBaselineFails = 1;
                    }
                    else
                       lOutlierSummary->iBaselineFails ++;
                    mStation.cTestData.FT_OutliersSummary[lString] = lOutlierSummary;

                    // If option to display outliers identified, show it!
                    if(lDisplayOutliers)
                    {
                        // Part# is now the real partID from the tester.
                        lOutliersIdentified += QString("[Site %1, PartID %2] <b>T%3: %4</b><br> LL: %5, HL: %6<br>")
                                .arg(lSiteNb).arg(lPR->PartID())
                                .arg(QString::number(lTC->lTestNumber)
                                     +(lTC->lPinmapIndex==-1?"":(QString(".")+lTC->GetPinIndex())) )
                                .arg(lTC->strTestName)
                                .arg(lPatDef->mDynamicLimits[lSiteNb].mLowDynamicLimit1[lSeverity])
                                .arg(lPatDef->mDynamicLimits[lSiteNb].mHighDynamicLimit1[lSeverity]);

                        // If we have two sets of limits, display the other set too!
                        if(lPatDef->mDynamicLimits[lSiteNb].mHighDynamicLimit2[lSeverity] !=
                            lPatDef->mDynamicLimits[lSiteNb].mHighDynamicLimit1[lSeverity] &&
                            lPatDef->mDynamicLimits[lSiteNb].mHighDynamicLimit2[lSeverity]
                            < GEX_TPAT_FLOAT_INFINITE)
                        {
                            lOutliersIdentified += QString("LL2: %1, HL2: %2<br>")
                                    .arg(lPatDef->mDynamicLimits[lSiteNb].mLowDynamicLimit2[lSeverity])
                                    .arg(lPatDef->mDynamicLimits[lSiteNb].mHighDynamicLimit2[lSeverity]);
                        }

                        lOutliersIdentified += QString("value: %1 %2<br>").arg(lResult).arg(lPatDef->m_strUnits);
                    }
                }
            }
        }

        // Move to next test
        lTC = lTC->GetNextTest();
    };

    // Set nb. of virtual outliers for the site (would have been outliers with new limits)
    Site->LotParts().SetVirtualOutliers(lOutlierParts.count());
    Site->IRBParts().SetVirtualOutliers(lOutlierParts.count());

    bool bSendEmail=false;
    //int		iTimeout; // 7103

    // Check if Too many outlier parts detected...in baseline
    if( (mStation.GetPatOptions().mFT_BaseLineMaxOutliers >= 0) &&
        (lOutlierParts.count() > mStation.GetPatOptions().mFT_BaseLineMaxOutliers) )
    {
        // If enabled, display list of outliers identified.
        if(lDisplayOutliers)
            DisplayMessage(GTM_ERRORTYPE_INFO,lOutliersIdentified);

        // Too many outliers  in baseline
        QString strErrorMsg = QString("<font color=\"#ff0000\"><b>Site %1: too many outliers in baseline</b>")
                .arg(lSiteNb);
        strErrorMsg += QString(" (Baseline will be restarted)<br></font>");
        strErrorMsg += QString("Outlier parts detected: <font color=\"#ff0000\"><b>%1").arg(lOutlierParts.count());
        strErrorMsg += QString(" </b></font> &gt; <b>%1 </b> (threshold)")
                .arg(mStation.GetPatOptions().mFT_BaseLineMaxOutliers);

        // 7103
        //iTimeout = GetTimeoutValue(cStation.GetPatOptions().iFT_Alarm_Timeout_Outliers);
        long long lOV=mStation.GetPatOptions().property(ALARM_SEVERITY_TOO_MUCH_OUTLIERS_IN_BASELINE).toLongLong();
        int lSeverity=GetAlarmSeverity(lOV);
        bSendEmail = (mStation.GetPatOptions().iFT_Alarm_Email_Outliers == 1) ? true: false;
        bool lNotifyTester=(lOV!=3)?true:false;
        DisplayMessage(GTM_ERRORTYPE_CRITICAL, strErrorMsg, bSendEmail, lNotifyTester, lSeverity);

        // Notify  tester that we have to reset STDF file & summary and restart baseline building.
        clientRestart_Baseline(lSiteNb);

        // Force back to 'baseline' generation for this site
        RestartBaseline(Site);
        return true;
    }
    else
    if(lOutlierParts.count())
    {
        // Some outliers in base line
        QString strInfoMsg = QString("Site %1: total outliers in baseline: %2")
                .arg(lSiteNb).arg(lOutlierParts.count());

        // If this we are in base line, tell operator that parts must be retested
        strInfoMsg += QString("<br><b>Need retest later (before end of lot)</b>");

        // Some outliers in base line: display info message to GTM and Tester windows
        DisplayMessage(GTM_ERRORTYPE_INFO, strInfoMsg, false,true);

        // If enabled, display list of outliers identified (only to GTM console, not to tester: string may be too long!
        if(lDisplayOutliers)
            DisplayMessage(GTM_ERRORTYPE_INFO,lOutliersIdentified);

        // Continue production.
        return false;
    }
    else
    {
        // No outliers in base line
        QString strInfoMsg = "No outlier in baseline for site "+QString::number(lSiteNb);
        // No outliers in base line: display info message to GTM and Tester windows
        // (info message: doesn't stop production)
        DisplayMessage(GTM_ERRORTYPE_INFO,strInfoMsg,false,true, 1); // severity = 1 = info
        return false;	// No outliers
    }

    return false;
}

#if 0
// Not used for now. If required, need to use new SiteList object instead of CSiteList
///////////////////////////////////////////////////////////
// Check for Low Cpk alert
///////////////////////////////////////////////////////////
bool ClientNode::CheckForParameterCpkAlert(CTest *ptTestCell, GS::Gex::CSiteTestResults *pSite)
{
    // If tester requested to no longer receive alram notification, return now!
    // (eg: if flushing last few runs when issuing end-of-lot)
    if(m_uiSilentMode)
        return false;

    // Check pointers
    if(!pSite || !ptTestCell)
        return false;

    // Quietly return if calling while not in production mode (eg: building baseline)
    if((mStation.GetStationStatus() != CStation::STATION_ENABLED)
            || (pSite->GetSiteState() != GS::Gex::CSiteTestResults::SITESTATUS_DPAT))
        return false;

    // Need at least 2 samples to compute Cpk
    if(ptTestCell->ldSamplesExecs < 2)
        return false;	// No alarm. All is fine!

    // Get handle to PAT definition for this test
    CPatDefinition *ptPatDef = mStation.GetPatDefinition(ptTestCell->lTestNumber, ptTestCell->lPinmapIndex,
                                                         ptTestCell->strTestName);
    if(ptPatDef == NULL)
        return false;	// This test is not part of PAT definition file, so simply ignore it!

    // Compute Cpk and see if lower than alarm level.
    double	lfMean = ptTestCell->lfSamplesTotal / ptTestCell->ldSamplesExecs;
    double	lfSigma = sqrt(fabs((((double)ptTestCell->ldSamplesExecs*ptTestCell->lfSamplesTotalSquare)
                                 - pow(ptTestCell->lfSamplesTotal,2))/
                        ((double)ptTestCell->ldSamplesExecs*((double)ptTestCell->ldSamplesExecs-1))));

    // If all values are identical (sigma = 0), then quietly return
    if(lfSigma == 0)
        return false;

    double	lfCpkL = fabs((lfMean-ptPatDef->m_lfLowLimit)/(3.0*lfSigma));
    double	lfCpkH = fabs((ptPatDef->m_lfHighLimit-lfMean)/(3.0*lfSigma));
    double	lfCpk = ptTestCell->lfCpk = gex_min(lfCpkL,lfCpkH);

    // Check if Cpk lower than threshold...
    if(lfCpk < ptPatDef->m_SPC_CpkAlarm)
    {
        QString strErrorMsg = QString("<font color=\"#ff0000\"><b>Low Cpk alarm</b></font>");
        strErrorMsg += QString("<br>Site #:    %1").arg(pSite->GetSiteNb());
        strErrorMsg += QString("<br>Test #:    %1 (%2)").arg(ptTestCell->lTestNumber).arg(ptPatDef->m_strTestName);
        strErrorMsg += QString("<br>Cpk alarm: <font color=\"#ff0000\"><b>%1</b></font>").arg(ptTestCell->lfCpk,0,'f',2);
        if(ptPatDef->m_SPC_CpkAlarm > 0)
        {
            strErrorMsg += QString(" &lt; <b>%1</b> (threshold)").arg(ptPatDef->m_SPC_CpkAlarm,0,'f',2);
        }


        // 7103
        // Get timeout value (in seconds) for this alarm condition
        //int iTimeout = GetTimeoutValue(cStation.GetPatOptions().iFT_Alarm_Timeout_ParamCpk);
        long long lOV=mStation.GetPatOptions().property(ALARM_SEVERITY_PARAM_CPK).toLongLong();
        int lSeverity=GetAlarmSeverity( lOV );
        bool lNotifyTester=lOV!=3?true:false;
        bool bSendEmail = (mStation.GetPatOptions().iFT_Alarm_Email_ParamCpk == 1) ? true: false;

        // Display Error message to GTM and tester consoles
        DisplayMessage(GTM_ERRORTYPE_CRITICAL, strErrorMsg, bSendEmail, lNotifyTester, lSeverity);
        return true;	// Trigger Alarm!
    }

    // No alarm
    return false;
}

///////////////////////////////////////////////////////////
// Check for Parameter PAT limits drift (once Dynamic PAT limits computed)
///////////////////////////////////////////////////////////
bool ClientNode::CheckForPatDriftAlert(GS::Gex::CSiteTestResults *pSite)
{
    bool            bDriftAlert=false;
    int             iTotalDrifts=0;
    QString         strErrorMsg;
    bool            bSendEmail;
    //int             iTimeout; // 7103
//    int             iSite;
    CPatDefinition  *ptPatDef;
    CTest           *ptTestCell;
    double          lfDrift;

    // Check pointers
    if(!pSite)
        return false;

    // Get valid site# used and site ptr.
//    iSite = pSite->GetSiteNb();

    // Get handle to test structure
    ptTestCell = pSite->getTestList();
    while(ptTestCell != NULL)
    {
        // Get handle to PAT definition for this test
        ptPatDef = mStation.GetPatDefinition(ptTestCell->lTestNumber, ptTestCell->lPinmapIndex,
                                             ptTestCell->strTestName);

        // Make sure handle and statistics valid fro this test
        if(ptPatDef && (ptTestCell->lfSamplesQuartile2 != -C_INFINITE))
        {
            // Compute Median drift (between median in Static PAT, and Dynamic PAT).
            bDriftAlert = false;
            lfDrift = fabs(ptPatDef->m_lfMedian - ptTestCell->lfSamplesQuartile2);
            switch(ptPatDef->m_SPC_PatMedianDriftAlarmUnits)
            {
                default:
                case GEX_TPAT_DRIFT_UNITS_NONE:
                case GEX_TPAT_DRIFT_UNITS_TEST:
                    if(ptPatDef->m_SPC_PatMedianDriftAlarm < 0)
                        break;	// We need a positive drift alarm to support this option.

                    if(lfDrift > ptPatDef->m_SPC_PatMedianDriftAlarm)
                        bDriftAlert = true;	// Drift too high
                    break;

                case GEX_TPAT_DRIFT_UNITS_SIGMA:
                    if(ptTestCell->lfSigma <= 0)
                        break;	// We need a positive sigma to support this option!
                    if(	lfDrift > (ptPatDef->m_SPC_PatMedianDriftAlarm*ptTestCell->lfSigma))
                        bDriftAlert = true;	// Drift too high
                    break;

                case GEX_TPAT_DRIFT_UNITS_P_LIMITS:
                    if(ptPatDef->m_lfHighLimit == ptPatDef->m_lfLowLimit)
                        break;	// This option requires two valid limits!
                    if(	lfDrift > (100.0*ptPatDef->m_SPC_PatMedianDriftAlarm)/(ptPatDef->m_lfHighLimit - ptPatDef->m_lfLowLimit))
                        bDriftAlert = true;	// Drift too high
                    break;
            }

            if(bDriftAlert)
            {
                // Keep track of total drift alarms
                iTotalDrifts++;

                // Prompt message
                strErrorMsg = QString("<font color=\"#ff0000\"><b>Site %1: PAT Limits drift alarm</b><br>")
                        .arg(pSite->GetSiteNb());
                strErrorMsg += QString("*PAT Disabled*<br></font>Test #:       %1 (%2)<br>Drift value : %3")
                        .arg(ptTestCell->lTestNumber).arg(ptPatDef->m_strTestName).arg(lfDrift);


                // 7103 :
                // Get severity value (in seconds) for this alarm condition
                //iTimeout = GetTimeoutValue(cStation.GetPatOptions().iFT_Alarm_Timeout_PatDrift);
                long long lOV=mStation.GetPatOptions().property(ALARM_SEVERITY_PARAM_DRIFT).toLongLong();
                bool lNotifyTester=lOV!=3?true:false;
                int lSeverity=GetAlarmSeverity(lOV);
                bSendEmail = (mStation.GetPatOptions().iFT_Alarm_Email_PatDrift == 1) ? true: false;
                DisplayMessage(GTM_ERRORTYPE_CRITICAL, strErrorMsg, bSendEmail, lNotifyTester, lSeverity);
            }
        }

        // Move to next test
        ptTestCell = ptTestCell->GetNextTest();
    };

    // Return alarm status
    if(iTotalDrifts >0)
        return true;	// Alarms occured
    else
        return false;	// No alarm occured
}

///////////////////////////////////////////////////////////
// Check for Low Yield alert on specified site
///////////////////////////////////////////////////////////
QString ClientNode::CheckForYieldAlert(GS::Gex::CSiteTestResults *pSite, bool &bYieldAlert)
{
    // Update Yield level displayed
    QString     strYieldColor;
    double      lfYieldLevel;

    // Display global yield
    if(mStation.cTestData.mCSites.GetTotalPartsTested() > 0)
        lfYieldLevel = ((double)mStation.cTestData.mCSites.GetTotalPartsGood()*100.0)
                / mStation.cTestData.mCSites.GetTotalPartsTested();
    else
        lfYieldLevel = 100.0;
    if(lfYieldLevel < mStation.GetPatOptions().mFT_YieldLevel)
        strYieldColor = "#C60000";	// Bright RED
    else
        strYieldColor = "#00C600";	// Dark green

    QString strYield = QString("<font color=\"%1\"><p align=\"center\">Yield<br><b>").arg(strYieldColor);
    strYield += QString("<h1>%1%</h1></b></p></font>").arg(lfYieldLevel,0,'f',2);

    // If base line not completed yet, do not check for yield alarm (only display current yield level).
    if(pSite->GetSiteState() != GS::Gex::CSiteTestResults::SITESTATUS_DPAT)
        return "ok: site not yet in DPAT state"; // was false

    // Compute yield for current site
    //bool	bYieldAlert = false;
    if(pSite->GetTotalPartsTested() > 0)
        lfYieldLevel = ((double)pSite->GetTotalPartsGood()*100.0)/pSite->GetTotalPartsTested();
    else
        lfYieldLevel = 100.0;

    // Notify of Low yield level (if under alarm threshold AND base line tested)
    bool bSendEmail=false;
    //int		iTimeout; 7103
    if(lfYieldLevel < mStation.GetPatOptions().mFT_YieldLevel)
    {
        // Flag that we have a Yield alert
        bYieldAlert = true;

        QString strErrorMsg = QString("<font color=\"#ff0000\"><b>Site %1: low Yield alarm</b></font>")
                .arg(pSite->GetSiteNb());
        strErrorMsg += QString("<br>Bin 1: <font color=\"#ff0000\"><b>%1% </b></font> &lt; <b>%2% </b> (threshold)")
                .arg(lfYieldLevel,0,'f',2)
                .arg((double)(mStation.GetPatOptions().mFT_YieldLevel),0,'f',2);


        //iTimeout = GetTimeoutValue(cStation.GetPatOptions().iFT_Alarm_Timeout_LowGoodYield);
        // 7103
        long long lOV=mStation.GetPatOptions().property(ALARM_SEVERITY_LOW_GOOD_YIELD).toLongLong();
        bool lNotifyTester=lOV!=3?true:false;
        int lSeverity=GetAlarmSeverity( lOV );
        bSendEmail = (mStation.GetPatOptions().iFT_Alarm_Email_YieldLoss == 1) ? true: false;
        DisplayMessage(GTM_ERRORTYPE_WARNING,strErrorMsg,bSendEmail,lNotifyTester, lSeverity);
    }

    // Check if combined critical bins exceed the user-defined threshold (if any)
    GS::QtLib::Range *lCriticalBinsList = mStation.GetPatOptions().mFT_CriticalBinsList;
    if(lCriticalBinsList == NULL)
        return "error: bin list range null"; //bYieldAlert;

    long	lTotalCriticalBins = pSite->GetCriticalBinsParts(lCriticalBinsList);
    // Compute yield level of combined critical bins
    lfYieldLevel = (100.0*lTotalCriticalBins)/(double)pSite->GetTotalPartsTested();
    if(lfYieldLevel > mStation.GetPatOptions().mFT_CriticalBinsYield)
    {
        // Too many critical bins!
        // Flag that we have a Yield alert
        bYieldAlert = true;

        QString strErrorMsg = QString("<font color=\"#ff0000\"><b>Site %1: critical Bins alarm</b></font>")
                .arg(pSite->GetSiteNb());
        strErrorMsg += QString("<br>Critical Bins: %1<br>Yield level:   <font color=\"#ff0000\"><b>%2")
                .arg(lCriticalBinsList->GetRangeList())
                .arg(lfYieldLevel,0,'f',2);
        strErrorMsg += QString("% </b></font> &gt; <b>%1% </b> (threshold)")
                .arg((double)(mStation.GetPatOptions().mFT_CriticalBinsYield),0,'f',2);

        // 7103
        //iTimeout = GetTimeoutValue(cStation.GetPatOptions().iFT_Alarm_Timeout_YieldLoss);
        long long lOV=mStation.GetPatOptions().property(ALARM_SEVERITY_YIELD_LOSS).toLongLong();
        bool lNotifyTester=lOV!=3?true:false;
        int lSeverity = GetAlarmSeverity( lOV );
        bSendEmail = (mStation.GetPatOptions().iFT_Alarm_Email_YieldLoss == 1) ? true: false;
        DisplayMessage(GTM_ERRORTYPE_CRITICAL, strErrorMsg, bSendEmail, lNotifyTester, lSeverity);
    }

    // Return alarm status
    return "ok"; //bYieldAlert;
}

// Check for Parameter rolling Mean drift
bool ClientNode::CheckForParameterDriftAlert(GS::Gex::CSiteTestResults *pSite)
{
    // Quietly return if calling while not in production mode (eg: building baseline)
    switch(pSite->GetSiteState())
    {
        case GS::Gex::CSiteTestResults::SITESTATUS_BASELINE:
            return false;

        case GS::Gex::CSiteTestResults::SITESTATUS_DISABLED:
        case GS::Gex::CSiteTestResults::SITESTATUS_DPAT:
            break;
        default:
          GSLOG(SYSLOG_SEV_WARNING, QString("Unknown site state %1").arg(pSite->GetSiteState()).toLatin1().data() );
        break;
    }

    // No alarm
    return false;
}
#endif

void ClientNode::clientRestart_Baseline(const int lSiteNb)
{
    PT_GNM_COMMAND pMsg_COMMAND=0;

    // Create COMMAND structure to pass to socket
    pMsg_COMMAND = (PT_GNM_COMMAND)malloc(sizeof(GNM_COMMAND));
    // Init structure
    gtcnm_InitStruct(GNM_TYPE_COMMAND, pMsg_COMMAND);
    // Set command
    pMsg_COMMAND->uiCommandID = GTC_GTLCOMMAND_RESTARTBASELINE;
    sprintf(pMsg_COMMAND->szCommand, "--site=%d", lSiteNb);
    // Display message
    DisplayMessage(GTM_ERRORTYPE_INFO, QString("<font color=\"#ff0000\"><b>Reseting Baseline for site %1...</b></font>")
                   .arg(lSiteNb));
    // Send command to tester
    emit sSendCommand((void *)pMsg_COMMAND);
}

///////////////////////////////////////////////////////////
// send Reset message to tester station.
///////////////////////////////////////////////////////////
/*
void ClientNode::clientReset()
{
    PT_GNM_COMMAND	pMsg_COMMAND;

    // Create COMMAND structure to pass to socket
    pMsg_COMMAND = (PT_GNM_COMMAND)malloc(sizeof(GNM_COMMAND));
    // Init structure
    gtcnm_InitStruct(GNM_TYPE_COMMAND, pMsg_COMMAND);
    // Set command
    pMsg_COMMAND->uiCommandID = GTC_GTLCOMMAND_RESET;
    // Display message
    DisplayMessage(GTM_ERRORTYPE_INFO,"<b>GTL has been reset...</b>");

    // Send command to tester
    emit sSendCommand((void *)pMsg_COMMAND);
}
*/

///////////////////////////////////////////////////////////
// send Message to write into tester's STDF DTR record
///////////////////////////////////////////////////////////
void ClientNode::clientLogMessageToStdf(QString &strString)
{
    PT_GNM_WRITETOSTDF pMsg_WRITETOSTDF=0;

    // Create WRITETOSTDF structure to pass to socket
    pMsg_WRITETOSTDF = (PT_GNM_WRITETOSTDF)malloc(sizeof(GNM_WRITETOSTDF));

    // Init structure
    gtcnm_InitStruct(GNM_TYPE_WRITETOSTDF, pMsg_WRITETOSTDF);

    // Set parameters
    pMsg_WRITETOSTDF->nRecTyp = 50;		// Record = DTR
    pMsg_WRITETOSTDF->nRecSub = 30;		// Record = DTR
    strcpy(pMsg_WRITETOSTDF->szString, strString.toLatin1().constData() );	// String to write to STDF file

    // Send request to tester
    emit sSendWriteToStdf((void *)pMsg_WRITETOSTDF);
}

COptionsPat& ClientNode::GetPatOptions()
{
    return mStation.GetPatOptions();
}

int ClientNode::GetSocketInstance()
{
    QObject* lP=parent();
    if (!lP)
        return -1;
    ClientSocket* lCS=qobject_cast<ClientSocket*>(lP);
    if (!lCS)
        return -1;
    return lCS->socketDescriptor();
}

QString ClientNode::NotifyTester(const QString & strMessage, int lSeverity/* = 0*/)
{
    // If tester requested to no longer receive alarm notification, return now!
    // (eg: if flushing last few runs when issuing end-of-lot)
    if(GetSilentMode()) //m_uiSilentMode)
        return "ok (silent)";

    QString lR=mSocket->OnNotifyTester(strMessage, lSeverity); // faster than emiting a signal
    //if (lR.startsWith("err"))
      //  GSLOG(3, QString("Failed to notify tester: %1").arg(lR).toLatin1().data() );
    //emit sNotifyTester(strMessage, lSeverity);
    return lR;
}

void ClientNode::cleanHtmlString(QString &strString)
{
    // Replace <br> with \n and two spaces (so multi-line text is easy to spot as a paragraph)
    strString.replace("<br>","\n  ");

    // Replace &lt; with '>'
    strString.replace("&gt;",">");
    strString.replace("&lt;","<");

    // Remove HTML markers & colors sequences (red & green)
    QRegExp cHtmlMarkers("<b>|</b>|<font color=\"#ff0000\">|<font color=\"#00c000\"|</font>");
    strString.remove(cHtmlMarkers);

    // Make sure line ends with a cr-lf
    if(strString.endsWith("\n") == false)
        strString += "\n";
}

int ClientNode::GetAlarmSeverity(int lIndex)
{
    switch(lIndex)
    {
        case 0: return -1; break; // critical
        case 1: return 0; break; // warning
        case 2: return 1; break; // notice
        case 3: return 2; break; // ignore: to be handled bu user
        default: return -1; // ignore or all other : critical
    }

    return -1;
}

} // Gex
} // GS

#endif
