#include "cbinning.h"
#include "gqtl_log.h"
#include "pat_global.h"
#include "pat_info.h"
#include "pat_rules.h"
#include "pat_definition.h"
#include "product_info.h"
#include "ctest.h"
#include "gex_scriptengine.h"
#include "bin_description.h"
#include "engine.h"

#include <QRect>
#include <QJsonArray>

extern GexScriptEngine*	pGexScriptEngine ;

extern QString  patlib_GetSeverityName(int iSeverityType);
//extern int      patlib_GetDistributionType(const CTest *ptTestCell,
//                                           int lCategoryValueCount = 5,
//                                           bool lAssumeIntegerCategory = true,
//                                           GS::Gex::PATPartFilter *partFilter = NULL);

#ifdef WS_PAT_PERFORMANCE
#include <QFile>
#include <QDir>

int PATPerfLogger::mSession = 0;
QHash<QString, QPair<int,qint64> > PATPerfLogger::mLogs;

PATPerfBench::PATPerfBench(const QString &lKey)
    : mKey(lKey)
{
    mTimer.start();
}

PATPerfBench::~PATPerfBench()
{
    PATPerfLogger::AddBench(mKey, mTimer.elapsed());
}

PATPerfLogger::PATPerfLogger()
{
    ++mSession;
    mLogs.clear();
}

PATPerfLogger::~PATPerfLogger()
{
    Dump();
}

void PATPerfLogger::Dump()
{
    QString lTempFolder = GS::Gex::Engine::GetInstance().Get("TempFolder").toString();
    QString lFileName   = QString("PATPerfLogs_%1.log").arg(mSession);
    QFile   lFileIO;

    lFileIO.setFileName(lTempFolder + QDir::separator() + lFileName);
    if(lFileIO.open(QIODevice::WriteOnly))
    {
        QTextStream lTextStream(&lFileIO);
        QHash<QString, QPair<int,qint64> >::iterator    itLogs;

        lTextStream << "Function,Calls,TotalTime" << endl;

        for (itLogs = mLogs.begin(); itLogs != mLogs.end(); ++itLogs)
        {
            lTextStream << itLogs.key() << ",";
            lTextStream << itLogs.value().first << ",";
            lTextStream << itLogs.value().second << endl;
        }
        lFileIO.close();
    }
    else
    {
        GSLOG(SYSLOG_SEV_WARNING,
              QString("Unable to create PAT Perf log file %1 in folder %2").arg(lFileName).arg(lTempFolder)
              .toLatin1().constData());
    }
}

void PATPerfLogger::AddBench(const QString &lKey, qint64 lElapsedTime)
{
    if (mLogs.contains(lKey))
    {
        mLogs[lKey].first   += 1;
        mLogs[lKey].second  += lElapsedTime;
    }
    else
        mLogs.insert(lKey, QPair<int, qint64>(1, lElapsedTime));
}

#endif

CPatFailureDeviceDetails::CPatFailureDeviceDetails()
{
    clear();
}

CPatFailureDeviceDetails::~CPatFailureDeviceDetails()
{

}

void CPatFailureDeviceDetails::clear()
{
    iSite       = 0;    // Testing site
    iPatRules   = 0;    // PAT algorithms that failed on that part.
    iPatSBin    = -1;
    iPatHBin    = -1;   // PAT bin
}

///////////////////////////////////////////////////////////
// Constructor: Structure that holds all global information
// pasted to the class building the report .
///////////////////////////////////////////////////////////
CPatInfo::CPatInfo(QObject* parent)
    : QObject(parent), mExternalBins(NULL), mSTDFHardBins(NULL), mSTDFSoftBins(NULL),
      mHardBins(NULL), mSoftBins(NULL), mSTDFTotalDies(0)
{
    // Clear structures
    clear();
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CPatInfo::~CPatInfo()
{
    // Clear structures
    clear();
}

///////////////////////////////////////////////////////////
// Empty structure's content.
///////////////////////////////////////////////////////////
void CPatInfo::clear()
{
    GSLOG(6, "Clearing PAT info..." );
    strDataFile.clear();
    strDataSources.clear();
    mGeneratedFilesList.clear();
    mReticleStepInfo.clear();

    // Clear outliers list
    mGDBNOutliers.clear();
    mReticleOutliers.clear();
    mNNROutliers.clear();
    mIDDQOutliers.clear();
    mClusteringOutliers.clear();
    mZPATOutliers.clear();
    mMVOutliers.clear();

    mMVPATCharts.clear();

    // Free the lists of outlier parts and their details (list of outlier tests)
    qDeleteAll(m_lstOutlierParts);
    m_lstOutlierParts.clear();

    // Free the list of NNR outliers
    qDeleteAll(pNNR_OutlierTests);
    pNNR_OutlierTests.clear();

    // Holds List of IDDQ-delta outliers
    qDeleteAll(pIDDQ_Delta_OutlierTests);
    pIDDQ_Delta_OutlierTests.clear();

    // Reset/Clear stacked wafermap
    m_ProberMap.clear();
    m_Stdf_SbinMap.clear();		// STDF: Softbin wafermap
    m_Stdf_HbinMap.clear();		// STDF: Hardbin wafermap
    m_AllSitesMap_Sbin.clear();
    m_AllSitesMap_Hbin.clear();

    // holds number of parts failing the Outlier limits (allows to compute yield loss): PPAT+SPAT
    mPatFailingParts                = 0;
    lPatShapeMismatch               = 0;			// Holds total number of distributions that mismatch from historical shapes.
    lPartsWithoutDatalog            = 0;		// Used to compute total parts without datalog.
    mTotalGoodAfterPAT              = 0;
    lTotalGoodAfterPAT_OutputMap    = 0;
    mSTDFTotalDies                  = 0;

    // unload Recipe details
    mPATRecipe.Reset(mPATRecipe.GetOptions().GetRecipeType());

    cMapTotalGoodParts.clear();
    cMapTotalFailPatParts.clear();
    cMapTotalFailParts.clear();
    m_strLogWarnings.clear();

    // Clear PAT Hard and Soft bins count
    mPATSoftBins.clear();
    mPATHardBins.clear();

    // Delete bins list
    if (mExternalBins)
    {
        CBinning * lTmpBin = NULL;
        while(mExternalBins != NULL)
        {
          lTmpBin = mExternalBins->ptNextBin;
          delete mExternalBins;
          mExternalBins = lTmpBin;
        };

        mExternalBins = NULL;
    }

    if (mSTDFHardBins)
    {
        CBinning * lTmpBin = NULL;
        while(mSTDFHardBins != NULL)
        {
          lTmpBin = mSTDFHardBins->ptNextBin;
          delete mSTDFHardBins;
          mSTDFHardBins = lTmpBin;
        };
        mSTDFHardBins = NULL;
    }

    if (mSTDFSoftBins)
    {
        CBinning * lTmpBin = NULL;
        while(mSTDFSoftBins != NULL)
        {
          lTmpBin = mSTDFSoftBins->ptNextBin;
          delete mSTDFSoftBins;
          mSTDFSoftBins = lTmpBin;
        };
        mSTDFSoftBins = NULL;
    }

    if (mHardBins)
    {
        CBinning * lTmpBin = NULL;
        while(mHardBins != NULL)
        {
          lTmpBin = mHardBins->ptNextBin;
          delete mHardBins;
          mHardBins = lTmpBin;
        };
        mHardBins = NULL;
    }

    if (mSoftBins)
    {
        CBinning * lTmpBin = NULL;
        while(mSoftBins != NULL)
        {
          lTmpBin = mSoftBins->ptNextBin;
          delete mSoftBins;
          mSoftBins = lTmpBin;
        };
        mSoftBins = NULL;
    }

    mReticleResults.clear();
}

///////////////////////////////////////////////////////////
// reset the outlier count for good parts 'true' , or all parts 'false'
///////////////////////////////////////////////////////////
void CPatInfo::clearOutlierCount(bool bAll)
{
    QHash<QString, CPatDefinition*>::iterator   itPATDefinifion;
    CPatDefinition	*                           ptPatDef = NULL;
    int	iSeverityLimit;

    for(itPATDefinifion = mPATRecipe.GetUniVariateRules().begin();
        itPATDefinifion != mPATRecipe.GetUniVariateRules().end(); ++itPATDefinifion)
    {
        ptPatDef = *itPATDefinifion;

        // Reset ALL outlier counts
        if(bAll)
        {
            ptPatDef->m_lStaticFailuresLow_AllParts=0;
            ptPatDef->m_lStaticFailuresHigh_AllParts=0;
            for(iSeverityLimit = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR; iSeverityLimit < GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES; iSeverityLimit++)
                ptPatDef->m_lDynamicFailuresLow_AllParts[iSeverityLimit] = ptPatDef->m_lDynamicFailuresHigh_AllParts[iSeverityLimit] = 0;
            ptPatDef->m_TotalFailures_AllParts = 0;
        }

        // For Good Parts: Holds statistics (number of failures from STATIC pats, and Dynamic pats)
        ptPatDef->m_lStaticFailuresLow=0;
        ptPatDef->m_lStaticFailuresHigh=0;
        for(iSeverityLimit = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR; iSeverityLimit < GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES; iSeverityLimit++)
            ptPatDef->m_lDynamicFailuresLow[iSeverityLimit] = ptPatDef->m_lDynamicFailuresHigh[iSeverityLimit] = 0;
        ptPatDef->m_TotalFailures= 0;
    }
}

///////////////////////////////////////////////////////////
// Tells if given die location has PPAT outliers detected.
///////////////////////////////////////////////////////////
int CPatInfo::isDieFailure_PPAT(int iDieX, int iDieY,int &iPatSBin,int &iPatHBin)
{
    tdIterPatOutlierParts	itOutlierParts(m_lstOutlierParts);
    CPatOutlierPart *		pOutlierPart = NULL;

    // Init
    iPatSBin = iPatHBin = -1;

    while(itOutlierParts.hasNext())
    {
        pOutlierPart = itOutlierParts.next();

        // Get PatBin & Die location.
        if(pOutlierPart->iDieX == iDieX && pOutlierPart->iDieY == iDieY)
        {
            iPatSBin = pOutlierPart->iPatSBin; // iOrgSoftbin
            iPatHBin = pOutlierPart->iPatHBin; // iOrgHardbin
            return iPatSBin;
        }
    };

    // This die was NOT an outlier, returns -1
    return -1;
}

///////////////////////////////////////////////////////////
// Tells if given die location has MVPAT outliers detected.
///////////////////////////////////////////////////////////
int CPatInfo::isDieFailure_MVPAT(int iDieX, int iDieY,int &iPatSBin,int &iPatHBin)
{
    tdIterMVPATOutliers     itOutlierParts(GetMVOutliers());

    // Init
    iPatSBin = iPatHBin = -1;

    while(itOutlierParts.hasNext())
    {
        itOutlierParts.next();

        // Get PatBin & Die location.
        if(itOutlierParts.value().GetCoordinate().GetX() == iDieX &&
           itOutlierParts.value().GetCoordinate().GetY() == iDieY)
        {
            iPatSBin = itOutlierParts.value().GetSoftBin();
            iPatHBin = itOutlierParts.value().GetHardBin();

            return iPatSBin;
        }
    };

    // This die was NOT an outlier, returns -1
    return -1;
}

///////////////////////////////////////////////////////////
// Tells if given die location has GPAT outliers detected.
///////////////////////////////////////////////////////////
int CPatInfo::isDieFailure_GPAT(int iDieX, int iDieY,int &iPatSBin,int &iPatHBin)
{
    QString lKey = QString::number(iDieX) + "." + QString::number(iDieY);
    iPatSBin = -1;

    if(mNNROutliers.contains(lKey))
    {
        iPatSBin = mNNROutliers.value(lKey).mPatSBin;
        iPatHBin = mNNROutliers.value(lKey).mPatHBin;
    }
    else if(mIDDQOutliers.contains(lKey))
    {
        iPatSBin = mIDDQOutliers.value(lKey).mPatSBin;
        iPatHBin = mIDDQOutliers.value(lKey).mPatHBin;
    }
    else if(mGDBNOutliers.contains(lKey))
    {
        iPatSBin = mGDBNOutliers.value(lKey).mPatSBin;
        iPatHBin = mGDBNOutliers.value(lKey).mPatHBin;
    }
    else if(mReticleOutliers.contains(lKey))
    {
        iPatSBin = mReticleOutliers.value(lKey).mPatSBin;
        iPatHBin = mReticleOutliers.value(lKey).mPatHBin;
    }
    else if(mClusteringOutliers.contains(lKey))
    {
        iPatSBin = mClusteringOutliers.value(lKey).mPatSBin;
        iPatHBin = mClusteringOutliers.value(lKey).mPatHBin;
    }
    else if(mZPATOutliers.contains(lKey))
    {
        iPatSBin = mZPATOutliers.value(lKey).mPatSBin;
        iPatHBin = mZPATOutliers.value(lKey).mPatHBin;
    }

    // Returns GPAT Sbin, or -1 if NOT a GPAT failing die
    return iPatSBin;
}

///////////////////////////////////////////////////////////
// Return original bin (Soft or Hard) for a given location
// -1 returned if invalid location or no bin at location.
///////////////////////////////////////////////////////////
int CPatInfo::GetOriginalBin(bool bSoftBin,int iDieX,int iDieY)
{
    CWaferMap *ptOriginalMap;

    // Get handle to relevant Wafermap (eg: HardBin map)
    if(bSoftBin)
        ptOriginalMap = &m_AllSitesMap_Sbin;
    else
        ptOriginalMap = &m_AllSitesMap_Hbin;

    return ptOriginalMap->binValue(iDieX,iDieY,CWaferMap::BeforeRetest);
}

///////////////////////////////////////////////////////////
// Get total outliers found on a given testing site (and optionnally for given binned parts)
///////////////////////////////////////////////////////////
int CPatInfo::getOutliersCount(int iOutlierType,int iSite/*=-1*/,bool bSoftBin/*=true*/,int iBin/*=-1*/, int iBinPat )
{
    int	iOutliersCount=0;

    // Report Outlier count (for given Outlier Algorithm)
    CPatOutlierPart	*		pOutlier_DPAT_Part	= NULL;
    int                     iOrgBin;

    tdIterPatOutlierParts				itOutlierParts(m_lstOutlierParts);
    tdIterGPATOutliers                  itGDBN(mGDBNOutliers);
    tdIterGPATOutliers                  itReticle(mReticleOutliers);
    tdIterGPATOutliers                  itNNR(mNNROutliers);
    tdIterGPATOutliers                  itIDDQ(mIDDQOutliers);
    tdIterGPATOutliers                  itClustering(mClusteringOutliers);
    tdIterGPATOutliers                  itZPat(mZPATOutliers);
    tdIterMVPATOutliers                 itMVPat(GetMVOutliers());

    switch(iOutlierType)
    {
        case CPatInfo::Outlier_DPAT:	// DPAT Outlier count
            while(itOutlierParts.hasNext())
            {
                pOutlier_DPAT_Part = itOutlierParts.next();

                // Get PatBin & Die location.
                if(iSite < 0 || pOutlier_DPAT_Part->iSite == iSite)
                {
                    // Check if only count outliers for given  binning parts?
                    iOrgBin = GetOriginalBin(bSoftBin,pOutlier_DPAT_Part->iDieX,pOutlier_DPAT_Part->iDieY);
                    int iDPATBin = bSoftBin ? pOutlier_DPAT_Part->iPatSBin : pOutlier_DPAT_Part->iPatHBin;
                    if(iBin < 0 || (iBin == iOrgBin))
                    {
                        if(iBinPat<0 || (iBinPat == iDPATBin))
                            iOutliersCount++;
                    }
                }
            };
            break;

        case CPatInfo::Outlier_NNR:	// NNR Outlier count
            itNNR.toFront();
            while (itNNR.hasNext())
            {
                itNNR.next();

                if(itNNR.value().mSite < 0 || itNNR.value().mSite == iSite)
                {
                    iOrgBin = GetOriginalBin(bSoftBin, itNNR.value().mDieX, itNNR.value().mDieY);
                    if(iBin < 0 || (iBin == iOrgBin))
                        iOutliersCount++;
                }
            }
            break;

        case CPatInfo::Outlier_IDDQ_Delta:	// IDDQ-Delta Outlier count
            itIDDQ.toFront();
            while(itIDDQ.hasNext())
            {
                itIDDQ.next();

                if(itIDDQ.value().mSite < 0 || itIDDQ.value().mSite == iSite)
                {
                    iOrgBin = GetOriginalBin(bSoftBin, itIDDQ.value().mDieX, itIDDQ.value().mDieY);
                    if(iBin < 0 || (iBin == iOrgBin))
                        iOutliersCount++;
                }
            }
            break;

        case CPatInfo::Outlier_GDBN:	// GDBN Outlier count
            itGDBN.toFront();
            while(itGDBN.hasNext())
            {
                itGDBN.next();

                if(itGDBN.value().mSite < 0 || itGDBN.value().mSite == iSite)
                {
                    iOrgBin = GetOriginalBin(bSoftBin, itGDBN.value().mDieX, itGDBN.value().mDieY);

                    if(iBin < 0 || (iBin == iOrgBin))
                        iOutliersCount++;
                }
            }
            break;

        case CPatInfo::Outlier_Reticle:	// Reticle Outlier count
            itReticle.toFront();
            while(itReticle.hasNext())
            {
                itReticle.next();

                if(itReticle.value().mSite < 0 || itReticle.value().mSite == iSite)
                {
                    iOrgBin = GetOriginalBin(bSoftBin, itReticle.value().mDieX, itReticle.value().mDieY);
                    if(iBin < 0 || (iBin == iOrgBin))
                        iOutliersCount++;
                }
            }
            break;

        case CPatInfo::Outlier_Clustering:	// Clustering Outlier count
            itClustering.toFront();
            while (itClustering.hasNext())
            {
                itClustering.next();

                if(itClustering.value().mSite < 0 || itClustering.value().mSite == iSite)
                {
                    iOrgBin = GetOriginalBin(bSoftBin, itClustering.value().mDieX,
                                             itClustering.value().mDieY);
                    if(iBin < 0 || (iBin == iOrgBin))
                        iOutliersCount++;
                }
            }
            break;

        case CPatInfo::Outlier_ZPAT:    // ZPat outlier count
            itZPat.toFront();
            while (itZPat.hasNext())
            {
                itZPat.next();

                if(itZPat.value().mSite < 0 || itZPat.value().mSite == iSite)
                {
                    iOrgBin = GetOriginalBin(bSoftBin, itZPat.value().mDieX, itZPat.value().mDieY);
                    if(iBin < 0 || (iBin == iOrgBin))
                        iOutliersCount++;
                }
            }
            break;

        case CPatInfo::Outlier_MVPAT:    // MVPAT outlier count

            itMVPat.toFront();
            while (itMVPat.hasNext())
            {
                itMVPat.next();

                if(itMVPat.value().GetSite() < 0 || itMVPat.value().GetSite() == iSite)
                {
                    iOrgBin = GetOriginalBin(bSoftBin, itMVPat.value().GetCoordinate().GetX(),
                                             itMVPat.value().GetCoordinate().GetY());

                    if(iBin < 0 || (iBin == iOrgBin))
                        iOutliersCount++;
                }
            }
            break;

        default:
            GSLOG(SYSLOG_SEV_WARNING, "Unknown outlier type");
            break;
    }

    // Return total outliers detected on a giventesting site.
    return iOutliersCount;
}

///////////////////////////////////////////////////////////
// Return Outlier type based on its PAT bin#
///////////////////////////////////////////////////////////
int CPatInfo::getOutlierType(int iBin)
{
    int lPATType = GetRecipeOptions().GetPATSoftBinFailType(iBin);

    // Outlier type...
    if(GetRecipeOptions().IsNNREnabled() && lPATType == GEX_TPAT_BINTYPE_NNR)
    {
        return CPatInfo::Outlier_NNR;			// NNR
    }
    else if(GetRecipeOptions().mIsIDDQ_Delta_enabled && lPATType == GEX_TPAT_BINTYPE_IDDQ_DELTA)
    {
        return CPatInfo::Outlier_IDDQ_Delta;	// IDDQ-Delta
    }
    else if(GetRecipeOptions().mIsGDBNEnabled && lPATType == GEX_TPAT_BINTYPE_BADNEIGHBORS)
    {
        return CPatInfo::Outlier_GDBN;			// GDBN
    }
    else if(GetRecipeOptions().GetReticleEnabled() && lPATType == GEX_TPAT_BINTYPE_RETICLE)
    {
        return CPatInfo::Outlier_Reticle;		// Reticle
    }
    else if(GetRecipeOptions().mClusteringPotato && lPATType == GEX_TPAT_BINTYPE_BADCLUSTER)
    {
        return CPatInfo::Outlier_Clustering;	// Clustering
    }
    else if(GetRecipeOptions().GetExclusionZoneEnabled() &&
            (GetRecipeOptions().lfCompositeExclusionZoneYieldThreshold > 0.0) &&
            lPATType == GEX_TPAT_BINTYPE_ZPAT)
    {
        return CPatInfo::Outlier_ZPAT;
    }
    else if (GetRecipeOptions().GetMVPATEnabled() && lPATType == GEX_TPAT_BINTYPE_MVPAT)
    {
        return CPatInfo::Outlier_MVPAT;
    }

    // If none of above, then assume it is a DPAT
    return CPatInfo::Outlier_DPAT;
}

///////////////////////////////////////////////////////////
// Check if given Test at die location is a NNR
// Return 'true' if given Test for die location is a NNR
///////////////////////////////////////////////////////////
bool CPatInfo::isNNR_Die(int iDieX, int iDieY, unsigned uTestNumber,int iPinmapIndex)
{
    // Check if NNR records exist!
    if(pNNR_OutlierTests.count() == 0)
        return false;

    foreach(CPatOutlierNNR *ptNRR_OutlierPart, pNNR_OutlierTests)
    {
        if(ptNRR_OutlierPart->mDieX == iDieX && ptNRR_OutlierPart->mDieY == iDieY &&
            ptNRR_OutlierPart->mTestNumber == uTestNumber && ptNRR_OutlierPart->mPinmap == iPinmapIndex)
            return true;	// NNR found!
    };

    // This test & die location is not a NNR
    return false;
}

///////////////////////////////////////////////////////////
// Records PAT processing exception
///////////////////////////////////////////////////////////
void	CPatInfo::logPatWarning(QString strType,int iTestNumer,int iPinmap,QString strMessage)
{
    QString strString;
    strString += "[" + strType;
    if(iTestNumer >= 0)
    {
       strString += " - T" + QString::number(iTestNumer);
        if(iPinmap >= 0)
           strString += "." + QString::number(iPinmap);
    }
    strString += "] : " + strMessage;

    // Add warning list to history buffer
    m_strLogWarnings += strString;
}

///////////////////////////////////////////////////////////
// Returns list of PAT binnings detected in data files for a given site or all sites
///////////////////////////////////////////////////////////
QString	CPatInfo::strGetPatBinList(int iSite/*=-1*/)
{
    QMap <int,int> cPatBinsGenerated;
    int	iBin;

    tdIterPatOutlierParts	itOutlierParts(m_lstOutlierParts);
    CPatOutlierPart *		pOutlierPart = NULL;

    while(itOutlierParts.hasNext())
    {
        pOutlierPart = itOutlierParts.next();

        // Get list of PAT bins issues for a given site or all sites.
        if((pOutlierPart->iSite == iSite) || (iSite == -1))
        {
            iBin = pOutlierPart->iPatSBin;
            if(cPatBinsGenerated.find(iBin) == cPatBinsGenerated.end())
                cPatBinsGenerated[iBin] = 1;
            else
                cPatBinsGenerated[iBin] = 1+cPatBinsGenerated[iBin];
        }
    };

    // Build list of PAT binnings
    QString strBinList="";
    QMap <int,int>::Iterator it;
    for ( it = cPatBinsGenerated.begin(); it != cPatBinsGenerated.end(); ++it )
        strBinList += QString::number(it.key()) + ",";

    return strBinList;
}

bool CPatInfo::AddMVOutlier(const GS::Gex::PATMVOutlier &lMVOutlier)
{
    QString lKey = QString::number(lMVOutlier.GetCoordinate().GetX()) + "." +
                   QString::number(lMVOutlier.GetCoordinate().GetY());

    if (mMVOutliers.contains(lKey) == false)
    {
        mMVOutliers.insert(lKey, lMVOutlier);
        return true;
    }

    return false;
}

bool CPatInfo::AddMVFailingRule(const GS::Gex::WaferCoordinate &lCoord,
                                const GS::Gex::PATMVFailingRule &lMVFailingRule)
{
    QString lKey = QString::number(lCoord.GetX()) + "." +
                   QString::number(lCoord.GetY());
    QHash<QString, GS::Gex::PATMVOutlier>::Iterator itOutlier = mMVOutliers.find(lKey);

    if (itOutlier != mMVOutliers.end())
        itOutlier.value().AddFailingRule(lMVFailingRule);
    else
    {
        GSLOG(SYSLOG_SEV_ERROR,
              QString("No outlier detected at coordinate (%1,%2)")
              .arg(lCoord.GetX()).arg(lCoord.GetY()).toLatin1().constData());
        return false;
    }

    return true;
}

void CPatInfo::IncrementPATHardBins(int lHBin, unsigned int lFailType)
{
    // Update Hard bin summary
    if(mPATHardBins.contains(lHBin) == false)
    {
        mPATHardBins[lHBin].iBinCount    = 1;
        mPATHardBins[lHBin].iBin         = lHBin;
    }
    else
        mPATHardBins[lHBin].iBinCount++;

    // Overwrite all other failing flags (fail Staic or Dynamic pat).
    mPATHardBins[lHBin].bFailType = lFailType;
}

void CPatInfo::IncrementPATSoftBins(int lSBin, unsigned int lFailType)
{
    // Update Hard bin summary
    if(mPATSoftBins.contains(lSBin) == false)
    {
        mPATSoftBins[lSBin].iBinCount    = 1;
        mPATSoftBins[lSBin].iBin         = lSBin;
    }
    else
        mPATSoftBins[lSBin].iBinCount++;

    // Overwrite all other failing flags (fail Staic or Dynamic pat).
    mPATSoftBins[lSBin].bFailType = lFailType;
}

bool CPatInfo::ConsolidatePATStatistics()
{
    GSLOG(SYSLOG_SEV_DEBUG, "---PAT : entering PostProcessing");

    CBinning *  lBinCell    = NULL;

    mPatFailingParts        = 0;
    mTotalGoodAfterPAT      = 0;
    mSTDFTotalDies          = 0;

    lBinCell = mSTDFSoftBins;
    while(lBinCell != NULL)
    {
        // Keep track of total STDF bins
        mSTDFTotalDies += lBinCell->ldTotalCount;

        // Move to next Bin cell
        lBinCell = lBinCell->ptNextBin;
    };

    GSLOG(SYSLOG_SEV_DEBUG, "---PAT : die total calculated.");

    lBinCell = mSoftBins;
    while(lBinCell != NULL)
    {
        // Keep track of total PAT bins
        if(IsPATBinning(lBinCell->iBinValue))
            mPatFailingParts += lBinCell->ldTotalCount;

        // Keep track of total Good dies & total fail dies.
        if(GetRecipeOptions().pGoodSoftBinsList->Contains(lBinCell->iBinValue))
            mTotalGoodAfterPAT += lBinCell->ldTotalCount;

        // Move to next Bin cell
        lBinCell = lBinCell->ptNextBin;
    };

    GSLOG(SYSLOG_SEV_DEBUG, "---PAT : good/bad part counts calculated.");

    GSLOG(SYSLOG_SEV_DEBUG, "---PAT : exiting PostProcessing");

    return true;
}

void CPatInfo::OverloadRefWafermap(int lXCoord, int lYCoord, int lSoftBin, int lHardBin)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        return;

    CWaferMap * lWafermap   = NULL;
    CBinning *  lBinning    = NULL;
    int			lIndex      = -1;
    int         lOldBin     = GEX_WAFMAP_EMPTY_CELL;

    // Overload SBIN map.
    lWafermap   = &m_AllSitesMap_Sbin;

    if(lWafermap)
    {
        if (lWafermap->indexFromCoord(lIndex, lXCoord, lYCoord))
        {
            // Get the current soft bin
            lOldBin = lWafermap->getWafMap()[lIndex].getBin();

            // Set with the overload bin
            lWafermap->getWafMap()[lIndex].setBin(lSoftBin);

            // Update bin list and count
            if (mSoftBins)
            {
                // Update current bin count
                lBinning = mSoftBins->Find(lOldBin);

                if (lBinning && lBinning->ldTotalCount > 0)
                    --lBinning->ldTotalCount;

                // Update overload bin count
                lBinning = mSoftBins->Find(lSoftBin);

                if (lBinning)
                    ++lBinning->ldTotalCount;
                else
                {
                    int lFailType = GetRecipeOptions().GetPATSoftBinFailType(lSoftBin);

                    if (lFailType > 0)
                    {
                        QString lOutlierTypeName = GS::Gex::PAT::GetOutlierBinName(lFailType);

                        mSoftBins->Insert(lSoftBin, 'F', 1, lOutlierTypeName);
                    }
                    else
                        GSLOG(SYSLOG_SEV_WARNING,
                              QString("No PAT Soft bin %1 found").arg(lSoftBin).toLatin1().constData());
                }
            }
        }
    }

    // Overload HBIN map.
    lWafermap = &m_AllSitesMap_Hbin;
    if(lWafermap)
    {
        if (lWafermap->indexFromCoord(lIndex, lXCoord, lYCoord))
        {
            // Get the current soft bin
            lOldBin = lWafermap->getWafMap()[lIndex].getBin();

            // Set with the overload bin
            lWafermap->getWafMap()[lIndex].setBin(lHardBin);

            // Update bin list and count
            if (mHardBins)
            {
                // Update current bin count
                lBinning = mHardBins->Find(lOldBin);

                if (lBinning && lBinning->ldTotalCount > 0)
                    --lBinning->ldTotalCount;

                // Update overload bin count
                lBinning = mHardBins->Find(lHardBin);

                if (lBinning)
                    ++lBinning->ldTotalCount;
                else
                {
                    int lFailType = GetRecipeOptions().GetPATHardBinFailType(lHardBin);

                    if (lFailType > 0)
                    {
                        QString lOutlierTypeName = GS::Gex::PAT::GetOutlierBinName(lFailType);

                        mHardBins->Insert(lHardBin, 'F', 1, lOutlierTypeName);
                    }
                    else
                    {
                        GSLOG(SYSLOG_SEV_WARNING,
                              QString("No PAT Hard bin %1 found").arg(lHardBin).toLatin1().constData());
                    }
                }
            }

        }
    }
}

CPatOutlierPart *CPatInfo::FindPPATOutlierPart(int lXCoord, int lYCoord)
{
    tdIterPatOutlierParts	itOutlierParts(m_lstOutlierParts);
    CPatOutlierPart *		pOutlierPart = NULL;

    while(itOutlierParts.hasNext())
    {
        pOutlierPart = itOutlierParts.next();

        // Get PatBin & Die location.
        if(pOutlierPart->iDieX == lXCoord && pOutlierPart->iDieY == lYCoord)
           return pOutlierPart;
    };

    return NULL;
}

bool CPatInfo::OverloadRefMapWithPPATBins()
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        return true;

    // If PPAT bins do not have to be considered in GPAT, simply return!
    if(GetRecipeOptions().mGPAT_IgnorePPatBins)
        return true;

    // Update MAPS (STDF SBIN, HBIN & External map) to include PPAT failures.
    int                     iIndex;
    tdIterPatOutlierParts	itOutlierParts(m_lstOutlierParts);
    tdIterMVPATOutliers     itMVOutliers(mMVOutliers);

    // HTH : WARNING
    // Enumerate all PPAT outliers
    while(itOutlierParts.hasNext())
    {
        OverloadRefWafermap(itOutlierParts.peekNext()->iDieX, itOutlierParts.peekNext()->iDieY,
                            itOutlierParts.peekNext()->iPatSBin, itOutlierParts.peekNext()->iPatHBin);

        if (m_ProberMap.indexFromCoord(iIndex, itOutlierParts.peekNext()->iDieX,
                                       itOutlierParts.peekNext()->iDieY) == true)
        {
            // Overload bin in ref wafmap
            m_ProberMap.getWafMap()[iIndex].setBin(itOutlierParts.peekNext()->iPatSBin);
        }

        itOutlierParts.next();
    };

    while (itMVOutliers.hasNext())
    {
        itMVOutliers.next();

        OverloadRefWafermap(itMVOutliers.value().GetCoordinate().GetX(),
                            itMVOutliers.value().GetCoordinate().GetY(),
                            itMVOutliers.value().GetSoftBin(), itMVOutliers.value().GetHardBin());

        if (m_ProberMap.indexFromCoord(iIndex, itMVOutliers.value().GetCoordinate().GetX(),
                                       itMVOutliers.value().GetCoordinate().GetY()) == true)
        {
            // Overload bin in external map
            m_ProberMap.getWafMap()[iIndex].setBin(itMVOutliers.value().GetSoftBin());
        }
    }

    return true;
}

bool CPatInfo::CleanPATDieAlarms(int lXCoord, int lYCoord, bool lLastPass)
{
    CPatOutlierPart *	lOutlierPart    = NULL;
    bool                lCleaned        = false;

    for(int lIdx = m_lstOutlierParts.size()-1; lIdx >= 0; --lIdx)
    {
        lOutlierPart = m_lstOutlierParts.at(lIdx);

        if((lOutlierPart->iDieX == lXCoord) && (lOutlierPart->iDieY == lYCoord))
        {
            delete m_lstOutlierParts.takeAt(lIdx);

            // Ensure Parts fail count remains accurate.
            if(lLastPass && (mPatFailingParts > 0))
                mPatFailingParts--;

            lCleaned = true;
        }
    }

    return lCleaned;
}

bool CPatInfo::IsPATBinning(int bin) const
{
    GSLOG(SYSLOG_SEV_DEBUG, "---PAT : entering IsPATBinning");
    GSLOG(SYSLOG_SEV_DEBUG, "---PAT : exiting IsPATBinning");

    return (GetRecipeOptions().GetPATSoftBinFailType(bin) > 0);
}

bool CPatInfo::IsMVOutlier(const GS::Gex::WaferCoordinate &coord) const
{
    QString lKey = QString::number(coord.GetX()) + "." +
                   QString::number(coord.GetY());

    return mMVOutliers.contains(lKey);
}

GS::Gex::WaferCoordinate CPatInfo::GetOriginalExternalCoordinate(const GS::Gex::WaferCoordinate &coord)
{
    return mExternalTransform.Map(coord);
}

GS::Gex::WaferTransform CPatInfo::GetExternalTransformation() const
{
    return mExternalTransform;
}

CBinning *CPatInfo::GetSTDFSoftBins() const
{
    return mSTDFSoftBins;
}

CBinning *CPatInfo::GetSTDFHardBins() const
{
    return mSTDFHardBins;
}

CBinning *CPatInfo::GetExternalBins() const
{
    return mExternalBins;
}

CBinning *CPatInfo::GetSoftBins() const
{
    return mSoftBins;
}

CBinning *CPatInfo::GetHardBins() const
{
    return mHardBins;
}

const tdPATBins &CPatInfo::GetPATSoftBins() const
{
    return mPATSoftBins;
}

const tdPATBins &CPatInfo::GetPATHardBins() const
{
    return mPATHardBins;
}

const QString &CPatInfo::GetRecipeFilename() const
{
    return mRecipeFile;
}

const QString &CPatInfo::GetSTDFFilename() const
{
    return mSTDFFile;
}


const CWaferMap* CPatInfo::GetReticleStepInformation() const
{
    if (mReticleStepInfo.compare("input_file", Qt::CaseInsensitive) == 0 && (m_Stdf_HbinMap.HasReticle()))
        return &m_Stdf_HbinMap;
    else if (mReticleStepInfo.compare("map_file", Qt::CaseInsensitive) == 0 && (m_ProberMap.HasReticle()))
        return &m_ProberMap;
    return NULL;
}

const QString &CPatInfo::GetOutputDataFilename() const
{
    return mOuputDataFile;
}

long CPatInfo::GetSTDFTotalDies() const
{
    return mSTDFTotalDies;
}

int CPatInfo::GetTotalGoodPartsPostPAT() const
{
    return mTotalGoodAfterPAT;
}

int CPatInfo::GetTotalGoodPartsPrePAT() const
{
    return mTotalGoodAfterPAT + mPatFailingParts;
}

int CPatInfo::GetTotalPATFailingParts() const
{
    return mPatFailingParts;
}

int CPatInfo::GetTotalNNRPATFailingParts() const
{
   return pNNR_OutlierTests.count();
}

int CPatInfo::GetTotalPPATFailingParts() const
{
    int lTotalPPATParts = 0;

    // Get total Parametric PPAT failures
    lTotalPPATParts = mPatFailingParts;                         // Total dies rejected (all PAT rule)
    lTotalPPATParts -= GetMVPATPartCount();           // Deduct MVPAT outliers
    lTotalPPATParts -= GetGPATPartCount();			// Deduct GPAT total PAT failures

    return lTotalPPATParts;
}

CPatDefinition *CPatInfo::GetPatDefinition(long lTestNumber, long lPinmapIndex,
                                           const QString& lTestName)
{
    // To be multi thread protected for GTM ? Uncomment me if crash here.
    //QMutexLocker lML(&mMutex);

    CPatDefinition *    lPatDef = NULL;
    QString             lKey;

    switch(GetRecipeOptions().mTestKey)
    {
        case GEX_TBPAT_KEY_TESTNUMBER:
            lKey = QString::number(lTestNumber);
            if(lPinmapIndex >= 0)
                lKey += "." + QString::number(lPinmapIndex);
            break;

        case GEX_TBPAT_KEY_TESTNAME:
            lKey = lTestName.trimmed();
            if(lPinmapIndex >= 0)
                lKey += "." + QString::number(lPinmapIndex);
            break;

        case GEX_TBPAT_KEY_TESTMIX:
            lKey = lTestName.trimmed();
            lKey += "." + QString::number(lTestNumber);
            if(lPinmapIndex >= 0)
                lKey += "." + QString::number(lPinmapIndex);

            break;
    }

    if (mPATRecipe.GetUniVariateRules().contains(lKey))
        lPatDef = mPATRecipe.GetUniVariateRules().value(lKey);

    // Holds the Static PAT limits for each test
    return lPatDef;
}

QString CPatInfo::GetTestName(long lTestNumber, long lPinmapIndex)
{
    QHash<QString, CPatDefinition*>::iterator   itPATDefinifion;
    CPatDefinition *                            ptPatDef = NULL;

    for(itPATDefinifion = mPATRecipe.GetUniVariateRules().begin();
        itPATDefinifion != mPATRecipe.GetUniVariateRules().end(); ++itPATDefinifion)
    {
        ptPatDef = *itPATDefinifion;
        if (ptPatDef->m_lTestNumber   == (unsigned long) lTestNumber &&
            ptPatDef->mPinIndex == lPinmapIndex)
            return ptPatDef->m_strTestName;
    }

    return "";
}

int CPatInfo::GetPPATPartCount() const
{
    return m_lstOutlierParts.count();
}

int CPatInfo::GetSPATPartCount() const
{
    int lCount  = 0;
    int lPATBin = -1;

    for (int lIdx = 0; lIdx < m_lstOutlierParts.count(); ++lIdx)
    {
        lPATBin = m_lstOutlierParts.at(lIdx)->iPatSBin;

        if (mPATRecipe.GetOptions().GetPATSoftBinFailType(lPATBin) == GEX_TPAT_BINTYPE_STATICFAIL)
            ++lCount;
    }

    return lCount;
}

int CPatInfo::GetDPATPartCount() const
{
    int lCount  = 0;
    int lPATBin = -1;

    for (int lIdx = 0; lIdx < m_lstOutlierParts.count(); ++lIdx)
    {
        lPATBin = m_lstOutlierParts.at(lIdx)->iPatSBin;

        if (mPATRecipe.GetOptions().GetPATSoftBinFailType(lPATBin) == GEX_TPAT_BINTYPE_DYNAMICFAIL)
            ++lCount;
    }

    return lCount;
}

int CPatInfo::GetMVPATPartCount() const
{
    return mMVOutliers.count();
}

int CPatInfo::GetGPATPartCount() const
{
    int lTotalParts = 0;

    lTotalParts += mGDBNOutliers.count();
    lTotalParts += mReticleOutliers.count();
    lTotalParts += mClusteringOutliers.count();
    lTotalParts += mNNROutliers.count();
    lTotalParts += mIDDQOutliers.count();
    lTotalParts += mZPATOutliers.count();	// dies rejected because of Z-PAT

    return lTotalParts;
}

void CPatInfo::GetPatFailureDeviceDetails(int lX, int lY, CPatFailureDeviceDetails &lDevice)
{
    int lSBin = -1;
    int lHBin = -1;

    // Init
    lDevice.clear();

    // Get PPAT bin details
    // PPAT binning (or -1 if this part didn't fail PAT)
    if (isDieFailure_PPAT(lX, lY, lSBin, lHBin) >= 0)
    {
        lDevice.iDieX       = lX;
        lDevice.iDieY       = lY;
        lDevice.iPatSBin    = lSBin;
        lDevice.iPatHBin    = lHBin;

        // Check if this die has failed some PPAT rules
        if(lDevice.iPatSBin >=0)
            lDevice.iPatRules |= (GEX_TPAT_BINTYPE_STATICFAIL | GEX_TPAT_BINTYPE_DYNAMICFAIL);
    }

    if (isDieFailure_MVPAT(lX, lY, lSBin, lHBin) >= 0)
    {
        lDevice.iDieX       = lX;
        lDevice.iDieY       = lY;
        lDevice.iPatSBin    = lSBin;
        lDevice.iPatHBin    = lHBin;
        lDevice.iPatRules |= GEX_TPAT_BINTYPE_MVPAT;
    }

    QString lKey = QString::number(lX) + "." + QString::number(lY);

    // Check if die failed GPAT
    if(mNNROutliers.contains(lKey))
    {
        lDevice.iPatRules |= GEX_TPAT_BINTYPE_NNR; 			// NNR rule failed this part
        lDevice.iPatSBin = mNNROutliers.value(lKey).mPatSBin;
        lDevice.iPatHBin = mNNROutliers.value(lKey).mPatHBin;
    }
    if(mIDDQOutliers.contains(lKey))
    {
        lDevice.iPatRules |= GEX_TPAT_BINTYPE_IDDQ_DELTA; 	// IDDQ_DELTA rule failed this part
        lDevice.iPatSBin = mIDDQOutliers.value(lKey).mPatSBin;
        lDevice.iPatHBin = mIDDQOutliers.value(lKey).mPatHBin;
    }
    if(mGDBNOutliers.contains(lKey))
    {
        lDevice.iPatRules |= GEX_TPAT_BINTYPE_BADNEIGHBORS; // GDBN rule failed this part
        lDevice.iPatSBin = mGDBNOutliers.value(lKey).mPatSBin;
        lDevice.iPatHBin = mGDBNOutliers.value(lKey).mPatHBin;
    }
    if(mReticleOutliers.contains(lKey))
    {
        lDevice.iPatRules |= GEX_TPAT_BINTYPE_RETICLE; 		// Reticle rule failed this part
        lDevice.iPatSBin = mReticleOutliers.value(lKey).mPatSBin;
        lDevice.iPatHBin = mReticleOutliers.value(lKey).mPatHBin;
    }
    if(mClusteringOutliers.contains(lKey))
    {
        lDevice.iPatRules |= GEX_TPAT_BINTYPE_BADCLUSTER; 	// BadCluster rule failed this part
        lDevice.iPatSBin = mClusteringOutliers.value(lKey).mPatSBin;
        lDevice.iPatHBin = mClusteringOutliers.value(lKey).mPatHBin;
    }
    if(mZPATOutliers.contains(lKey))
    {
        lDevice.iPatRules |= GEX_TPAT_BINTYPE_ZPAT;         // ZPAT rule failed this part
        lDevice.iPatSBin = mZPATOutliers.value(lKey).mPatSBin;
        lDevice.iPatHBin = mZPATOutliers.value(lKey).mPatHBin;
    }
}

const QList<GS::Gex::PATMultiVariateRule> &CPatInfo::GetMultiVariateRules() const
{
    return mPATRecipe.GetMultiVariateRules();
}

QList<GS::Gex::PATMultiVariateRule> &CPatInfo::GetMultiVariateRules()
{
    return mPATRecipe.GetMultiVariateRules();
}


bool CPatInfo::IsPATTestExecutedWithPass(int lCoordX,
                                         int lCoordY,
                                         unsigned long lTestNumber,
                                         int lPinmapIndex,
                                         QString lTestName)
{
    CPatOutlierPart* lPPATOutlier = FindPPATOutlierPart(lCoordX, lCoordY);

    // If null, means the test has been executed with pass status
    if (!lPPATOutlier)
        return true;

    QList<CPatFailingTest> lOutlierList = lPPATOutlier->cOutlierList;
    for (int i=0; i<lOutlierList.size(); ++i)
    {
        CPatFailingTest lPatFailTest = lOutlierList[i];
        if (lPatFailTest.mTestName == lTestName
            && lPatFailTest.mTestNumber == lTestNumber
            && lPatFailTest.mPinIndex == lPinmapIndex)
        {
            return false;
        }
    }

    return true;
}
int CPatInfo::GetPPATAlgoTested(int lCoordX,
                                int lCoordY,
                                unsigned long lTestNumber,
                                int lPinmapIndex,
                                QString lTestName,
                                int& lFailAlgo)
{
    // Retrieve the algorithm a given has been tested through.
    //
    // Test first SPAT
    // - If SPAT enabled for this test
    //      Die is SPAT tested
    // - If DPAT enabled for this test and die is not a SPAT outlier
    //      Die is DPAT tested
    // - If NNR enabled for this test and die is not SPAT or DPAT outlier
    //

    lFailAlgo = 0;
    int                 lPATRule    = 0;
    CPatDefinition *    lPATDef     = mPATRecipe.FindUniVariateRule(lTestNumber, lPinmapIndex, lTestName);

    // If test exists, check which algorithm has been applied
    if (lPATDef != NULL)
    {
        CPatOutlierPart *   lPPATOutlier    = FindPPATOutlierPart(lCoordX, lCoordY);
        bool                lOutlier        = false;

        // Check if die tested through SPAT
        if (lPATDef->m_lFailStaticBin > -1 && lPATDef->m_SPATRuleType != GEX_TPAT_SPAT_ALGO_IGNORE)
        {
            lPATRule |= GEX_TPAT_BINTYPE_STATICFAIL;
        }

        // Check if the part is a SPAT outlier
        if (lPPATOutlier &&
            mPATRecipe.GetOptions().GetPATSoftBinFailType(lPPATOutlier->iPatSBin) == GEX_TPAT_BINTYPE_STATICFAIL)
        {
            lFailAlgo |= GEX_TPAT_BINTYPE_STATICFAIL;
            lOutlier = true;
        }

        // Check if die is tested through DPAT
        if (lPATDef->m_lFailDynamicBin > -1 && lPATDef->mOutlierRule != GEX_TPAT_RULETYPE_IGNOREID &&
            lOutlier == false)
        {
            lPATRule |= GEX_TPAT_BINTYPE_DYNAMICFAIL;
        }

        // Check if the part is a DPAT outlier
        if (lPPATOutlier &&
            mPATRecipe.GetOptions().GetPATSoftBinFailType(lPPATOutlier->iPatSBin) == GEX_TPAT_BINTYPE_DYNAMICFAIL)
        {
            lFailAlgo |= GEX_TPAT_BINTYPE_DYNAMICFAIL;
            lOutlier = true;
        }

        // Check if the part is tested with some NNR rules
        if (mPATRecipe.GetOptions().IsNNREnabled() && lPATDef->m_iNrrRule == GEX_TPAT_NNR_ENABLED && lOutlier == false)
        {
            int lFirstNNREnabled = -1;

            // Check if at least one NNR rule is enabled
            for (int lIdx = 0; lIdx < mPATRecipe.GetOptions().GetNNRRules().count() && lFirstNNREnabled == -1; ++lIdx)
            {
                if (mPATRecipe.GetOptions().GetNNRRules().at(lIdx).IsEnabled())
                    lFirstNNREnabled = lIdx;
            }

            // If NNR enabled, check if a given die has been tested for the given parameter
            if (lFirstNNREnabled == -1)
            {
                QString                   lKey            = QString::number(lCoordX) + "." + QString::number(lCoordY);
                tdGPATOutliers::iterator  itNNROutlier    = mNNROutliers.find(lKey);

                // If no outlier found for NNR algorithms, it means all NNR rules have been applied.
                if (itNNROutlier != mNNROutliers.end())
                {
                    // The rule which fails this die is not the first rule, so, it means, this die has been tested
                    // over all parameters at leat for one NNR rule
                    if ((*itNNROutlier).mRuleName != mPATRecipe.GetOptions().GetNNRRules().at(lFirstNNREnabled).GetRuleName())
                    {
                        lPATRule |= GEX_TPAT_BINTYPE_NNR;
                    }
                    else
                    {
                        CPatOutlierNNR * lNNROutlierPart = NULL;

                        // Find the test which fails the die
                        for (int lIdx = 0; lIdx < pNNR_OutlierTests.count(); ++lIdx)
                        {
                            lNNROutlierPart = pNNR_OutlierTests.at(lIdx);

                            if (lNNROutlierPart
                                && lNNROutlierPart->mDieX == lCoordX
                                && lNNROutlierPart->mDieY == lCoordY)
                            {
                                // NNR is executed over all parameters based on their Test number then pinmap index
                                // If the test which fails the die has a test number lower thant the given parameter
                                //      The die has not been tested with the given parameter
                                // Othewise
                                //      The die has been tested for the given parameter
                                if (lTestNumber < lNNROutlierPart->mTestNumber ||
                                    (lTestNumber == lNNROutlierPart->mTestNumber &&
                                     lPinmapIndex < lNNROutlierPart->mPinmap))
                                {
                                    lPATRule |= GEX_TPAT_BINTYPE_NNR;
                                }
                            }
                        }
                    }
                }
                else
                {
                    lPATRule |= GEX_TPAT_BINTYPE_NNR;
                }

                if ((lPATRule & GEX_TPAT_BINTYPE_NNR)
                    && lPPATOutlier
                    && mPATRecipe.GetOptions().GetPATSoftBinFailType(lPPATOutlier->iPatSBin) == GEX_TPAT_BINTYPE_NNR)
                {
                    lFailAlgo |= GEX_TPAT_BINTYPE_NNR;
                }
            }
        }
    }

    return lPATRule;
}

const tdMVPATOutliers &CPatInfo::GetMVOutliers() const
{
    return mMVOutliers;
}

const QList<int> &CPatInfo::GetSiteList() const
{
    return mSiteList;
}

QJsonObject CPatInfo::GetReticleResults(const QString &ruleName) const
{
    if (mReticleResults.contains(ruleName))
        return mReticleResults.value(ruleName);

    return QJsonObject();
}

void CPatInfo::SetExternalTransformation(const GS::Gex::WaferTransform &transform)
{
    mExternalTransform = transform;
}

void CPatInfo::SetSTDFHardBins(const CBinning *binnings)
{
    if (binnings)
    {
        mSTDFHardBins   = binnings->Clone();
        mHardBins       = binnings->Clone();
    }
}

void CPatInfo::SetHardBins(const CBinning *binnings)
{
    if (binnings)
    {
        mHardBins       = binnings->Clone();
    }
}

void CPatInfo::SetSTDFSoftBins(const CBinning *binnings)
{
    if (binnings)
    {
        mSTDFSoftBins   = binnings->Clone();
        mSoftBins       = binnings->Clone();
    }
}

void CPatInfo::SetSoftBins(const CBinning *binnings)
{
    if (binnings)
    {
        mSoftBins       = binnings->Clone();
    }
}
void CPatInfo::SetExternalBins(CBinning * binnings)
{
    mExternalBins = binnings;
}

void CPatInfo::SetRecipeFilename(const QString &fileName)
{
    mRecipeFile = fileName;
}

void CPatInfo::SetSTDFFilename(const QString &fileName)
{
    mSTDFFile = fileName;
}

void CPatInfo::SetReticleStepInfo(const QString &reticleStepInfo   )
{
    mReticleStepInfo = reticleStepInfo;
}

void CPatInfo::SetOutputDataFilename(const QString &fileName)
{
    mOuputDataFile = fileName;
}

void CPatInfo::SetSiteList(const QList<int> &siteList)
{
    mSiteList = siteList;
}

void CPatInfo::SetReticleResults(const QString &ruleName, const QJsonObject& results)
{
    mReticleResults.insert(ruleName, results);
}

///////////////////////////////////////////////////////////
// Checks if given die location is already identified as outlier
// Used to avoid duplicating identification between algorithms
///////////////////////////////////////////////////////////
bool CPatInfo::isDieOutlier(int iDieX,int iDieY,int &iBin)
{

    // Check if PPAT failure
    int	iHbin=0;
    if(isDieFailure_PPAT(iDieX,iDieY,iBin,iHbin) >= 0)	// DPAT binning (or -1 if this part didn't fail PPAT)
        return true;

    // Check if MVPAT failure
    if (isDieFailure_MVPAT(iDieX, iDieY, iBin, iHbin) >= 0)
        return true;

    // Check if GPAT failure
    if(isDieFailure_GPAT(iDieX,iDieY,iBin,iHbin) >= 0)	// GPAT binning (or -1 if this part didn't fail GPAT)
        return true;


    // This die is NOT yet a outlier!
    iBin = -1;

    return false;
}

///////////////////////////////////////////////////////////
// Return handle to Mask definition
///////////////////////////////////////////////////////////
CMask_Rule * CPatInfo::GetMaskDefinition(QString maskName)
{
    int iIndex;

    for(iIndex=0; iIndex < GetRecipeOptions().mMaskRules.count(); iIndex++)
    {
        if(GetRecipeOptions().mMaskRules.at(iIndex)->mRuleName == maskName)
            return GetRecipeOptions().mMaskRules[iIndex];
    }


    // Mask entry doesn't exist
    return NULL;
}

const GS::Gex::PATRecipe* CPatInfo::GetRecipe() const
{
    return &mPATRecipe;
}

GS::Gex::PATRecipe &CPatInfo::GetRecipe()
{
    return mPATRecipe;
}

COptionsPat &CPatInfo::GetRecipeOptions()
{
    return mPATRecipe.GetOptions();
}

const COptionsPat &CPatInfo::GetRecipeOptions() const
{
    return mPATRecipe.GetOptions();
}

tdPATDefinitions& CPatInfo::GetUnivariateRules()
{
    return mPATRecipe.GetUniVariateRules();
}



void CPatInfo::AddMVPATRuleChartPath(const QString& lRuleName, const QString& lChartPath)
{
    if (mMVPATCharts.contains(lRuleName))
        mMVPATCharts[lRuleName].append(lChartPath);
    else
        mMVPATCharts.insert(lRuleName, QStringList(lChartPath));
}

const QMap<QString, QStringList>& CPatInfo::GetMVPATChartsPath() const
{
    return mMVPATCharts;
}

const QStringList CPatInfo::GetMVPATRuleChartsPath(const QString& lRuleName) const
{
    if (mMVPATCharts.contains(lRuleName))
        return mMVPATCharts[lRuleName];

    return QStringList();
}

void CPatInfo::addToGeneratedFilesList(const QString &file)
{
    mGeneratedFilesList.append(file);
}

void CPatInfo::CleanupGeneratedFileList(bool bDelete)
{
    if (bDelete)
    {
        GSLOG(SYSLOG_SEV_NOTICE,
              QString("Deleting generated PAT files").toLatin1().constData());

        for (int lIdx = 0; lIdx < mGeneratedFilesList.count(); ++lIdx)
            GS::Gex::Engine::GetInstance().RemoveFileFromDisk(mGeneratedFilesList.at(lIdx));
    }

    GSLOG(SYSLOG_SEV_NOTICE,
          QString("Clearing list of generated PAT files").toLatin1().constData());

    mGeneratedFilesList.clear();
}

