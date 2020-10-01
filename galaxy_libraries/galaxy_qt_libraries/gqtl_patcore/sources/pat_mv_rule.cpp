#include <pat_mv_rule.h>

namespace GS
{
namespace Gex
{

PATMultiVariateRule::PATMultiVariateRule() : mEnabled(true), mBin(-1), mCustomDistance(0),
    mPrincipalComponents(-1), mOutlierDistanceMode(PAT::Near), mType(Manual)
{
}

PATMultiVariateRule::PATMultiVariateRule(const PATMultiVariateRule &lOther)
{
    *this = lOther;
}

bool PATMultiVariateRule::operator==(const PATMultiVariateRule& lOther)
{
    if(mEnabled != lOther.mEnabled)
        return false;
    if(mBin != lOther.mBin)
        return false;
    if(mCustomDistance != lOther.mCustomDistance)
        return false;
    if(mPrincipalComponents != lOther.mPrincipalComponents)
        return false;
    if(mName != lOther.mName)
        return false;
    if(mOutlierDistanceMode != lOther.mOutlierDistanceMode)
        return false;
    if(mType != lOther.mType)
        return false;

    return true;
}

PATMultiVariateRule::~PATMultiVariateRule()
{
}

PATMultiVariateRule &PATMultiVariateRule::operator =(const PATMultiVariateRule &lOther)
{
    if (this != &lOther)
    {
        mEnabled                = lOther.mEnabled;
        mBin                    = lOther.mBin;
        mCustomDistance         = lOther.mCustomDistance;
        mPrincipalComponents    = lOther.mPrincipalComponents;
        mName                   = lOther.mName;
        mOutlierDistanceMode    = lOther.mOutlierDistanceMode;
        mType                   = lOther.mType;
        mTestData               = lOther.mTestData;
    }

    return *this;
}

bool PATMultiVariateRule::GetEnabled() const
{
    return mEnabled;
}

int PATMultiVariateRule::GetBin() const
{
    return mBin;
}

double PATMultiVariateRule::GetCustomDistance() const
{
    return mCustomDistance;
}

int PATMultiVariateRule::GetPrincipalComponents() const
{
    return mPrincipalComponents;
}

const QString &PATMultiVariateRule::GetName() const
{
    return mName;
}

PAT::OutlierDistance PATMultiVariateRule::GetOutlierDistanceMode() const
{
    return mOutlierDistanceMode;
}

QString PATMultiVariateRule::GetOutlierDistanceModeString() const
{
    if(mOutlierDistanceMode == PAT::Near)
        return "Near";
    if(mOutlierDistanceMode == PAT::Medium)
        return "Medium";
    if(mOutlierDistanceMode == PAT::Far)
        return "Far";
    if(mOutlierDistanceMode == PAT::Custom)
        return "Custom";
    return "NA";
}

PATMultiVariateRule::RuleType PATMultiVariateRule::GetType() const
{
    return mType;
}

QString PATMultiVariateRule::GetTypeString() const
{
    if(mType == Generated)
        return "Generated";
    if(mType == Manual)
        return "Manual";
    return "NA";
}

void PATMultiVariateRule::AddTestData(const MVTestData &data)
{
    mTestData.append(data);
}

int PATMultiVariateRule::RemoveAllTestData(const PATMultiVariateRule::MVTestData &data)
{
    return mTestData.removeAll(data);
}

const QList<PATMultiVariateRule::MVTestData> &PATMultiVariateRule::GetMVTestData() const
{
    return mTestData;
}

void PATMultiVariateRule::SetEnabled(bool lEnable)
{
    mEnabled = lEnable;
}

void PATMultiVariateRule::SetBin(int lBin)
{
    mBin = lBin;
}

void PATMultiVariateRule::SetCustomDistance(double lDistance)
{
    mCustomDistance = lDistance;
}

void PATMultiVariateRule::SetPrincipalComponents(int lMaxComponents)
{
    mPrincipalComponents = lMaxComponents;
}

void PATMultiVariateRule::SetName(const QString &lName)
{
    mName = lName;
}

void PATMultiVariateRule::SetOutlierDistanceMode(PAT::OutlierDistance lMode)
{
    mOutlierDistanceMode = lMode;
}

void PATMultiVariateRule::SetRule(PATMultiVariateRule::RuleType lType)
{
    mType = lType;
}

PATMultiVariateRule::MVTestData::MVTestData(const QString &testName,int testNumber,int pinIdx):
    mTestName(testName),mTestNumber(testNumber),mPinIdx(pinIdx)
{
}

PATMultiVariateRule::MVTestData::~MVTestData()
{

}

PATMultiVariateRule::MVTestData::MVTestData(const MVTestData &other)
{
    *this=other;
}

PATMultiVariateRule::MVTestData&
PATMultiVariateRule::MVTestData::operator=(const PATMultiVariateRule::MVTestData& lOther)
{
     mTestName = lOther.mTestName;
     mTestNumber = lOther.mTestNumber;
     mPinIdx = lOther.mPinIdx;
     return *this;
}

bool PATMultiVariateRule::MVTestData::operator==(const MVTestData& lOther)
{
    return (mTestName == lOther.mTestName )
            &&(mTestNumber == lOther.mTestNumber)
            && (mPinIdx == lOther.mPinIdx);
}

QString PATMultiVariateRule::MVTestData::GetTestName() const
{
    return mTestName;
}

void PATMultiVariateRule::MVTestData::SetTestName(const QString &str)
{
    mTestName = str;
}

int PATMultiVariateRule::MVTestData::GetTestNumber() const
{
    return mTestNumber;
}

void PATMultiVariateRule::MVTestData::SetTestNumber(int num)
{
    mTestNumber = num;
}

int PATMultiVariateRule::MVTestData::GetPinIdx() const
{
    return mPinIdx;
}

void PATMultiVariateRule::MVTestData::SetPinIdx(int idx)
{
    mPinIdx = idx;
}

}   // namespace Gex
}   // namespace GS
