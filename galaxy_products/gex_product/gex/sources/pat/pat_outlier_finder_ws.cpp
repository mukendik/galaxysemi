#ifdef GCORE15334
#include "pat_outlier_finder_ws.h"
#include "pat_outlier_finder_ws_private.h"
#include "gex_report.h"
#include "pat_info.h"
#include "pat_definition.h"
#include "gqtl_log.h"
#include "stats_engine.h"
#include "engine.h"


extern CGexReport * gexReport;

namespace GS
{
namespace Gex
{

PATOutlierFinderWS::PATOutlierFinderWS(CPatInfo * lContext, QObject * parent)
    : PATOutlierFinder(* new PATOutlierFinderWSPrivate, parent)
{
    Q_D(PATOutlierFinderWS);

    d->Init(lContext);
}

PATOutlierFinderWS::PATOutlierFinderWS(PATOutlierFinderWSPrivate &lPrivateData, CPatInfo *lContext, QObject *parent)
    : PATOutlierFinder(lPrivateData, parent)
{
    Q_D(PATOutlierFinderWS);

    d->Init(lContext);
}

PATOutlierFinderWS::~PATOutlierFinderWS()
{
}

CTest *PATOutlierFinderWS::FindTestCell(CPatDefinition *lPatDef)
{
    Q_D(PATOutlierFinderWS);

    return d->FindTestCell(lPatDef->m_lTestNumber, lPatDef->mPinIndex, lPatDef->m_strTestName);
}

bool PATOutlierFinderWS::UpdateDatasetPointer(int lSite)
{
    Q_D(PATOutlierFinderWS);

    // Update filters to use the current working site
    if(d->mCurrentSite != lSite)
    {
        d->mCurrentSite = lSite;

        if (d->FillPartFilter(d->mPartFilter, d->mCurrentSite) == false)
        {
            GSLOG(SYSLOG_SEV_ERROR, "Unable to fill the part filter.");
            return false;
        }
    }

    return true;
}

bool PATOutlierFinderWS::AnalyzeWaferSurface()
{
    Q_D(PATOutlierFinderWS);

    bool lResult = true;

    if (d->mInitialized)
    {
        // Check if need to include PPAT bins in maps.
        if (d->mContext->OverloadRefMapWithPPATBins() == false)
            return false;

        // Execute rules according to specified precedence (order)
        QString	lRuleName;
        for(int lIdx = 0; lIdx < d->mContext->GetRecipeOptions().strRulePrecedence.count() && lResult; ++lIdx)
        {
            // Get rule name
            lRuleName = d->mContext->GetRecipeOptions().strRulePrecedence.at(lIdx);

            if(lRuleName.compare("NNR",Qt::CaseInsensitive) == 0)
            {
                // NNR Algorithm (Near neighbour Residual)
                lResult = d->AnalyzeWaferSurfaceNNR();
            }
            else if(lRuleName.compare("IDDQ-Delta",Qt::CaseInsensitive) == 0)
            {
                // IDDQ-Delta Algorithm
                lResult = d->AnalyzeWaferSurfaceIDDQ_Delta();
            }
            else if(lRuleName.compare("GDBN",Qt::CaseInsensitive) == 0)
            {
                // GDBN Algorithm
                lResult = d->AnalyzeWaferSurfaceGDBN();
            }
            else if(lRuleName.compare("Clustering",Qt::CaseInsensitive) == 0)
            {
                // Cluster of bad dies ('potato' cluster)
                lResult = d->AnalyzeWaferSurfacePotatoCluster();
            }
            else if(lRuleName.compare("Reticle",Qt::CaseInsensitive) == 0)
            {
                // Reticle
                lResult = d->AnalyzeWaferSurfaceReticle();
            }
        }
    }

    return lResult;
}

bool PATOutlierFinderWS::AnalyzeWaferSurfaceZPAT(CWaferMap *lWaferMap)
{
    Q_D(PATOutlierFinderWS);

    if (d->mInitialized)
    {
        // Perform GDBN over Z-PAT exclusion map
        if(d->mContext->GetRecipeOptions().bZPAT_GDBN_Enabled)
        {
            if (d->AnalyzeWaferSurfaceGDBN(lWaferMap, true) == false)
                return false;
        }

        // Perform Reticle over Z-PAT exclusion map
        if(d->mContext->GetRecipeOptions().bZPAT_Reticle_Enabled)
        {
            if (d->AnalyzeWaferSurfaceReticle(lWaferMap) == false)
                return false;
        }

        // Perform Clustering over Z-PAT exclusion map
        if(d->mContext->GetRecipeOptions().bZPAT_Clustering_Enabled)
        {
            if (d->AnalyzeWaferSurfacePotatoCluster(lWaferMap) == false)
                return false;
        }

        return true;
    }

    return false;
}

bool PATOutlierFinderWS::ComputeMVPATOutliers()
{
    Q_D(PATOutlierFinderWS);

    if (d->mInitialized)
    {
        if (d->mContext->GetRecipeOptions().GetMVPATEnabled())
        {
            QString lAppDir = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
            QString lError;
            for (int lIdx = 0; lIdx < d->mContext->GetMultiVariateRules().count(); ++lIdx)
            {
                if (d->mContext->GetMultiVariateRules().at(lIdx).GetEnabled())
                {
                    GS::SE::StatsEngine * lStatsEngine = GS::SE::StatsEngine::GetInstance(lAppDir, lError);
                    if (lStatsEngine)
                    {
                        bool lStatus = d->ComputeMVPATOutlier(d->mContext->GetMultiVariateRules().at(lIdx), lIdx, lStatsEngine);
                        GS::SE::StatsEngine::ReleaseInstance();
                        if (!lStatus)
                            return false;
                    }
                    else
                    {
                        GSLOG(SYSLOG_SEV_CRITICAL, QString("Unable to instantiate StatsEngine: %1. Application will now exit.")
                              .arg(lError).toLatin1().data());
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }
        else
            GSLOG(SYSLOG_SEV_INFORMATIONAL, "MV PAT disabled");

        return true;
    }

    return false;
}

}
}
#endif
