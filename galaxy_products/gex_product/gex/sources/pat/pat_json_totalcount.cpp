#include "pat_json_totalcount.h"

namespace GS
{
namespace PAT
{

    PatJsonTotalCountV1::PatJsonTotalCountV1()
    {
    }

    void PatJsonTotalCountV1::buildJson(const CPatInfo* patInfo, QJsonObject& sumObjectToFill)
    {
        if(patInfo == 0)
            return;

        QJsonObject lSumObject;
        lSumObject.insert("DieTested",  static_cast<qint64>(patInfo->GetSTDFTotalDies()) );
        lSumObject.insert("Pass",       patInfo->GetTotalGoodPartsPostPAT() );
        lSumObject.insert("PATFails",   patInfo->GetTotalPATFailingParts() );
        lSumObject.insert("DPAT",       patInfo->GetDPATPartCount());
        lSumObject.insert("SPAT",       patInfo->GetSPATPartCount());
        lSumObject.insert("NNR",        patInfo->GetTotalNNRPATFailingParts());
        lSumObject.insert("GDBN",       patInfo->mGDBNOutliers.count());
        lSumObject.insert("Cluster",    patInfo->mClusteringOutliers.count());
        lSumObject.insert("ZPAT",       patInfo->mZPATOutliers.count());
        lSumObject.insert("MVPAT",      patInfo->GetMVOutliers().count());
        lSumObject.insert("IDDQ",       patInfo->mIDDQOutliers.count());
        lSumObject.insert("Reticle",    patInfo->mReticleOutliers.count());

        sumObjectToFill.insert("TotalCounts", lSumObject);
    }
}
}

