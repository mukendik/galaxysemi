#include <QJsonArray>

#include "pat_recipe.h"
#include "pat_json_outlier.h"
#include "pat_global.h"

namespace GS
{
namespace PAT
{


PatJsonOutlierV1::PatJsonOutlierV1()
{
}

void PatJsonOutlierV1::buildJson(const CPatOutlierPart* outlier,int patBinType, IPatJsonFailingTest *patJsonFailingTest, QJsonObject& objectToFill)
{

    if(outlier == 0 || patJsonFailingTest == 0)
        return;

    QList<CPatFailingTest> lPatFailingTest = outlier->cOutlierList;

    if(lPatFailingTest.isEmpty() == false)
    {
        QJsonArray lJSonFailingTests;
        QList<CPatFailingTest>::iterator lIterB(lPatFailingTest.begin()), lIterE(lPatFailingTest.end());
        for(; lIterB != lIterE; ++lIterB)
        {
            QJsonObject lJSonTest;
            patJsonFailingTest->buildJson(&(*lIterB), lJSonTest);

            lJSonFailingTests.append(lJSonTest);
        }

        objectToFill.insert("FailingTests", lJSonFailingTests);
    }

    objectToFill.insert("X", outlier->iDieX);
    objectToFill.insert("Y", outlier->iDieY);
    objectToFill.insert("HBinNumber", outlier->iPatHBin);
    objectToFill.insert("SBinNumber", outlier->iPatSBin);
    objectToFill.insert("Org_HBinNumber", outlier->iOrgSoftbin);
    objectToFill.insert("Org_SBinNumber", outlier->iOrgHardbin);
    objectToFill.insert("Type", GS::Gex::PAT::GetOutlierTypeName(patBinType));
    objectToFill.insert("PartId", outlier->strPartID);
    objectToFill.insert("Site", outlier->iSite);
}

void PatJsonOutlierV1::buildJson(const CPatDieCoordinates& outlier, QJsonObject &objectToFill)
{
    objectToFill.insert("X", outlier.mDieX);
    objectToFill.insert("Y", outlier.mDieY);
    objectToFill.insert("HBinNumber", outlier.mPatHBin);
    objectToFill.insert("SBinNumber", outlier.mPatSBin);
    objectToFill.insert("Org_HBinNumber", outlier.mOrigHBin);
    objectToFill.insert("Org_SBinNumber", outlier.mOrigSBin);
    objectToFill.insert("Type", GS::Gex::PAT::GetOutlierTypeName(outlier.mFailType));
    objectToFill.insert("PartId", outlier.mPartId);
    objectToFill.insert("Site", outlier.mSite);
}

void PatJsonOutlierV1::buildJson(const GS::Gex::PATMVOutlier &outlier, QJsonObject &objectToFill)
{
    objectToFill.insert("X", outlier.GetCoordinate().GetX());
    objectToFill.insert("Y", outlier.GetCoordinate().GetX());
    objectToFill.insert("HBinNumber", outlier.GetHardBin());
    objectToFill.insert("SBinNumber", outlier.GetSoftBin());
    objectToFill.insert("Org_HBinNumber", outlier.GetOriginalHardBin());
    objectToFill.insert("Org_SBinNumber", outlier.GetOriginalSoftBin());
    objectToFill.insert("Type", QString("MVPAT"));
    objectToFill.insert("PartId", outlier.GetPartID());
    objectToFill.insert("Site", outlier.GetSite());
}

}
}
