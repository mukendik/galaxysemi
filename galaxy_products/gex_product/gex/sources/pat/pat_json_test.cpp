#include "pat_json_test.h"
#include <QJsonArray>

namespace GS
{
namespace PAT
{


    PatJsonTestV1::PatJsonTestV1()
    {

    }

    void PatJsonTestV1::buildJson(const CPatDefinition* testPATDef, QJsonObject& objectToFill)
    {
        if(testPATDef == 0)
            return ;

        objectToFill.insert("Name",         testPATDef->m_strTestName);
        objectToFill.insert("Number",       static_cast<qint64>(testPATDef->m_lTestNumber));
        objectToFill.insert("Pin",          static_cast<qint64>(testPATDef->mPinIndex));
        objectToFill.insert("Units",        testPATDef->m_strUnits);
        objectToFill.insert("LoLimitScale", testPATDef->m_llm_scal);
        objectToFill.insert("HiLimitScale", testPATDef->m_hlm_scal);

        QJsonObject lSPATLimit;
        lSPATLimit.insert("LoLimit",   testPATDef->m_lfLowStaticLimit);
        lSPATLimit.insert("HiLimit",   testPATDef->m_lfHighStaticLimit);

        objectToFill.insert("SPAT", lSPATLimit);

        QJsonArray  lDPATArray;

        QMap<int, GS::PAT::DynamicLimits>::const_iterator lIterBegin = testPATDef->mDynamicLimits.constBegin();
        QMap<int, GS::PAT::DynamicLimits>::const_iterator lIterEnd   = testPATDef->mDynamicLimits.constEnd();

        for(; lIterBegin!=lIterEnd; ++lIterBegin)
        {
            QJsonObject lDPATLimit;

            lDPATLimit.insert("Site",       lIterBegin.key());

            GS::PAT::DynamicLimits lDynLimits = testPATDef->GetDynamicLimits(lIterBegin.key());

            if (lDynLimits.mLowDynamicLimit1[testPATDef->m_iOutlierLimitsSet] != -GEX_TPAT_DOUBLE_INFINITE)
                lDPATLimit.insert("LoLimit",   lDynLimits.mLowDynamicLimit1[testPATDef->m_iOutlierLimitsSet]);

            if (lDynLimits.mHighDynamicLimit1[testPATDef->m_iOutlierLimitsSet] != GEX_TPAT_DOUBLE_INFINITE)
                lDPATLimit.insert("HiLimit",   lDynLimits.mHighDynamicLimit1[testPATDef->m_iOutlierLimitsSet]);

            lDPATArray.append(lDPATLimit);
        }

        objectToFill.insert("DPAT", lDPATArray);
    }

}
}
