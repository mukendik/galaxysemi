#include "parserParameter.h"

#include <limits>

namespace GS
{
namespace Parser
{

ParserParameter::ParserParameter()
{
    mLowLimit = mHighLimit = mLowSpecLimit = mHighSpecLimit = 0.0;
    mValidLowLimit = mValidHighLimit = mValidHighSpecLimit = mValidLowSpecLimit = mStaticHeaderWritten = false;
    mStaticMultiLimitsWritten = false;
    mTestName = mTestUnits = "";
    mExecCount = mExecFail = 0;
    mExecTime = 0.0;
    mResScale = 0;
    mSpecificFlags = 0;
    mIsParametricTest = false;
}

long ParserParameter::GetTestNumber() const
{
   return mTestNumber;
}

QString ParserParameter::GetTestName() const
{
   return mTestName;
}

QString ParserParameter::GetTestUnits() const
{
    return mTestUnits;
}

int ParserParameter::GetResultScale() const
{
    return mResScale;
}

double ParserParameter::GetLowLimit() const
{
   return mLowLimit;
}

double ParserParameter::GetHighLimit() const
{
   return mHighLimit;
}

bool ParserParameter::GetValidLowLimit() const
{
   return mValidLowLimit;
}

bool ParserParameter::GetValidHighLimit() const
{
   return mValidHighLimit;
}

double ParserParameter::GetLowSpecLimit() const
{
   return mLowSpecLimit;
}

double ParserParameter::GetHighSpecLimit() const
{
   return mHighSpecLimit;
}

bool ParserParameter::GetValidLowSpecLimit() const
{
   return mValidLowSpecLimit;
}

bool ParserParameter::GetValidHighSpecLimit() const
{
   return mValidHighSpecLimit;
}

double ParserParameter::GetTestValue() const
{
    return mTestValue;
}

double ParserParameter::GetTestValueForSite(const int aSite)
{
    if (mTestValueBySite.contains(aSite))
    {
        return mTestValueBySite.value(aSite);
    }
    else
    {
        return std::numeric_limits<double>::quiet_NaN();
    }
}

bool ParserParameter::HasTestValueForSite(const int aSite)
{
    return mTestValueBySite.contains(aSite);
}

bool ParserParameter::GetStaticHeaderWritten() const
{
   return mStaticHeaderWritten;
}

bool ParserParameter::GetStaticMultiLimitsWritten() const
{
   return mStaticMultiLimitsWritten;
}

unsigned long ParserParameter::GetExecCount() const
{
    return mExecCount;
}

unsigned long ParserParameter::GetExecFail() const
{
    return mExecFail;
}

float ParserParameter::GetExecTime() const
{
    return mExecTime;
}

QString ParserParameter::GetTestTxt() const
{
    return mTestTxt;
}


////////////////////////////////////
//// Setteur
void ParserParameter::SetTestNumber(const long testNumber)
{
   mTestNumber = testNumber;
}

void ParserParameter::SetTestName(const QString testName)
{
   mTestName = testName;
}

void ParserParameter::SetTestUnit(const QString testUnits)
{
    mTestUnits = testUnits;
}

void ParserParameter::SetResultScale(int resScale)
{
    mResScale = resScale;
}

void ParserParameter::SetLowLimit(const double lowLimit)
{
   mLowLimit = lowLimit;
}

void ParserParameter::SetHighLimit(const double highLimit)
{
   mHighLimit = highLimit;
}

void ParserParameter::SetValidLowLimit(const bool validLowLimit)
{
   mValidLowLimit = validLowLimit;
}

void ParserParameter::SetValidHighLimit(const bool validHighLimit)
{
   mValidHighLimit = validHighLimit;
}

void ParserParameter::SetLowSpecLimit(const double lowLimit)
{
   mLowSpecLimit = lowLimit;
}

void ParserParameter::SetHighSpecLimit(const double highLimit)
{
   mHighSpecLimit = highLimit;
}

void ParserParameter::SetValidLowSpecLimit(const bool validLowLimit)
{
   mValidLowSpecLimit = validLowLimit;
}

void ParserParameter::SetValidHighSpecLimit(const bool validHighLimit)
{
   mValidHighSpecLimit = validHighLimit;
}

void ParserParameter::SetTestValue(const double testValue)
{
    mTestValue = testValue;
}

void ParserParameter::SetTestValueForSite(const int aSite, const double aValue)
{
    mTestValueBySite.insert(aSite, aValue);
}

void ParserParameter::SetStaticHeaderWritten(const bool staticHeaderWritten)
{
    mStaticHeaderWritten = staticHeaderWritten;
}

void ParserParameter::SetStaticMultiLimitsWritten(const bool staticMultiLimitsWritten)
{
    mStaticMultiLimitsWritten = staticMultiLimitsWritten;
}


void ParserParameter::SetExecCount(const unsigned long execCount)
{
    mExecCount = execCount;
}

void ParserParameter::SetExecFail(const unsigned long execFail)
{
    mExecFail = execFail;
}

void ParserParameter::SetExecTime(const float aValue)
{
    mExecTime = aValue;
}

void ParserParameter::SetTestTxt(const QString testTxt)
{
    mTestTxt = testTxt;
}


void ParserParameter::IncrementFailTest()
{
    ++mExecFail;
}

void ParserParameter::IncrementExecTest()
{
    ++mExecCount;
}


void ParserParameter::AddMultiLimitItem(const GS::Core::MultiLimitItem &lMLS,
                                        MultiLimit keepDuplicate /* = SkipDuplicateMultiLimit */)
{
    // Keep all multi-limits if flag is set or keep only new set of limit when skip duplicate multi-limit is set
    if (keepDuplicate == KeepDuplicateMultiLimit || mMultiLimits.indexOf(lMLS) == -1)
        mMultiLimits.append(lMLS);
}

int ParserParameter::GetMultiLimitCount() const
{
    return mMultiLimits.count();
}

const GS::Core::MultiLimitItem &ParserParameter::GetMultiLimitSetAt(int lIndex) const
{
    return mMultiLimits.at(lIndex);
}

GS::Core::MultiLimitItem &ParserParameter::GetMultiLimit(int lIndex)
{
    return mMultiLimits[lIndex];
}

int ParserParameter::GetSpecificFlags() const
{
    return mSpecificFlags;
}

bool ParserParameter::IsParametricTest() const
{
    return mIsParametricTest;
}

void ParserParameter::SetSpecificFlags(int flags)
{
    mSpecificFlags = flags;
}

void ParserParameter::SetIsParamatricTest(bool aValue)
{
    mIsParametricTest = aValue;
}

}
}
