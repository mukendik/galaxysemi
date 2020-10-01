#include "pat_reticle_defectivity_check.h"
#include "pat_reticle_map_abstract.h"
#include "gqtl_log.h"

#include <qjsonobject.h>

namespace GS
{
namespace Gex
{

PatReticleDefectivityCheck::PatReticleDefectivityCheck(const PATOptionReticle &reticleSettings)
    : PatReticleAbstractAlgorithm(reticleSettings)
{

}

PatReticleDefectivityCheck::~PatReticleDefectivityCheck()
{

}

bool PatReticleDefectivityCheck::ProcessReticleMap(PatReticleMapAbstract *reticleMap)
{
    // Loop over all individual reticle
    if (Init(reticleMap))
    {
        do
        {
            // process single reticle
            if (ProcessReticle() == false)
                return false;
        }
        while (reticleMap->Next());

        // Add reticle fields summary to json results object
        // Each element in the array has the following syntax
        // {
        //      "Status" : "Pass" or "Fail",
        //      "FailYield": "17.00",
        //      "PosX": 1,
        //      "PosY": 2
        // }
        mReticleResults.insert("values", mReticleFieldsResults);

        return true;
    }

    return false;
}

bool PatReticleDefectivityCheck::Init(PatReticleMapAbstract *reticleMap)
{
    if (reticleMap)
    {
        mReticleMap = reticleMap;
        mReticleResults.remove("values");

        return mReticleMap->Init();
    }

    return false;
}

bool PatReticleDefectivityCheck::IsReticleFieldFiltered()
{
    bool    lFiltered = false;

    switch (mReticleSettings.GetFieldSelection())
    {
        case PATOptionReticle::ALL_RETICLE_FIELDS:
            lFiltered = true;
            break;

        case PATOptionReticle::EDGE_RETICLE_FIELDS:
            lFiltered = mReticleMap->IsEdgeReticleField();
            break;

        case PATOptionReticle::LIST_RETICLE_FIELDS:
            for (int lIdx = 0; lIdx < mReticleSettings.GetFieldCoordinates().count() && lFiltered == false; ++lIdx)
            {
                if (mReticleMap->GetReticlePosX() == mReticleSettings.GetFieldCoordinates().at(lIdx).first &&
                    mReticleMap->GetReticlePosY() == mReticleSettings.GetFieldCoordinates().at(lIdx).second)
                {
                    lFiltered = true;
                }
            }
            break;

        default:
            GSLOG(SYSLOG_SEV_WARNING,
                  QString("Incorrect field selection for Reticle Defectivity Check: %1")
                  .arg(mReticleSettings.GetFieldSelection()).toLatin1().constData());
            break;
    }

    return lFiltered;
}

bool PatReticleDefectivityCheck::ProcessReticle()
{
    if (IsReticleFieldFiltered())
    {
        double lGoodYield = mReticleMap->GetReticleFieldYield(mReticleSettings.GetBadBinsReticleList());

        if (lGoodYield >= 0.0)
        {
            double lBadYield = (1.0 - lGoodYield) * 100.00;

            QJsonObject lReticleResult;

            if (lBadYield >= mReticleSettings.GetFieldThreshold())
            {
                // Identify all outliers for this reticle field
                FindOutliers();

                // Set status for this reticle
                lReticleResult.insert("Status", QJsonValue(QString("Fail")));
            }
            else
                lReticleResult.insert("Status", QJsonValue(QString("Pass")));

            lReticleResult.insert("FailYield", QJsonValue(QString::number(lBadYield,'f',2)));
            lReticleResult.insert("PosX", QJsonValue(mReticleMap->GetReticlePosX()));
            lReticleResult.insert("PosY", QJsonValue(mReticleMap->GetReticlePosY()));

            mReticleFieldsResults.append(lReticleResult);
        }
    }

    return true;
}

void PatReticleDefectivityCheck::FindOutliers()
{
    int lBin = -1;

    for (int lYCoord = mReticleMap->GetTopLeftDie().GetY(); lYCoord <= mReticleMap->GetBottomRightDie().GetY(); ++lYCoord)
    {
        for (int lXCoord = mReticleMap->GetTopLeftDie().GetX(); lXCoord <= mReticleMap->GetBottomRightDie().GetX(); ++lXCoord)
        {
            // Get die value SoftBin#
            lBin = mReticleMap->GetBin(lXCoord, lYCoord);

            // Keep track of valid dies at given reticle location
            if(lBin != GEX_WAFMAP_EMPTY_CELL)
            {
                mOutliers.append(WaferCoordinate(lXCoord, lYCoord));
            }
        }
    }
}

}
}
