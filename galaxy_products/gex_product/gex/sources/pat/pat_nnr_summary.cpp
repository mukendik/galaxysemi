#include <pat_nnr_summary.h>
#include <pat_rules.h>

namespace GS
{
namespace Gex
{

PATNNRTestSummary::PATNNRTestSummary()
    : mTestNumber(-1), mPinmap(-1)
{

}

PATNNRTestSummary::PATNNRTestSummary(int lTestNumber, int lPinmap, const QString& lTestName)
    : mTestNumber(lTestNumber), mPinmap(lPinmap), mTestName(lTestName)
{

}

PATNNRTestSummary::PATNNRTestSummary(const PATNNRTestSummary &other)
{
    *this = other;
}

PATNNRTestSummary::~PATNNRTestSummary()
{

}

PATNNRTestSummary &PATNNRTestSummary::operator=(const PATNNRTestSummary &other)
{
    if (this != &other)
    {
        mTestNumber     = other.mTestNumber;
        mPinmap         = other.mPinmap;
        mTestName       = other.mTestName;
        mRuleSummary    = other.mRuleSummary;
    }

    return *this;
}

bool PATNNRTestSummary::operator<(const PATNNRTestSummary &other) const
{
    if (mTestNumber < other.mTestNumber)
        return true;

    if (mTestNumber > other.mTestNumber)
        return false;

    if (mPinmap < other.mPinmap)
        return true;

    if (mPinmap > other.mPinmap)
        return false;

    return (mTestName.compare(other.mTestName, Qt::CaseInsensitive) < 0);
}

void PATNNRTestSummary::AddOutlier(const QString& lRuleName, int lCount)
{
    if (mRuleSummary.contains(lRuleName))
        mRuleSummary[lRuleName] = mRuleSummary[lRuleName] + lCount;
    else
        mRuleSummary[lRuleName] = lCount;
}

int PATNNRTestSummary::GetTestNumber() const
{
    return mTestNumber;
}

int PATNNRTestSummary::GetPinmap() const
{
    return mPinmap;
}

const QString &PATNNRTestSummary::GetTestName() const
{
    return mTestName;
}

int PATNNRTestSummary::GetOutlierCount() const
{
    QMap<QString, int>::const_iterator    itBegin;
    QMap<QString, int>::const_iterator    itEnd     = mRuleSummary.constEnd();

    int lCount = 0;

    for (itBegin = mRuleSummary.constBegin(); itBegin != itEnd; ++itBegin)
        lCount += itBegin.value();

    return lCount;
}

const QMap<QString, int> &PATNNRTestSummary::GetRuleSummary() const
{
    return mRuleSummary;
}

void PATNNRTestSummary::SetTestNumber(int lTestNumber)
{
    mTestNumber = lTestNumber;
}

void PATNNRTestSummary::SetPinmap(int lPinmap)
{
    mPinmap = lPinmap;
}

void PATNNRTestSummary::SetTestName(const QString &lTestName)
{
    mTestName = lTestName;
}

}
}
