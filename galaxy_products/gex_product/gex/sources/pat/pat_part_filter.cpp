#include "pat_part_filter.h"
#include "ctest.h"
#include "gqtl_log.h"

namespace GS
{
namespace Gex
{

PATPartFilterElement::PATPartFilterElement(const CTest *lTest,
                                           PATPartFilterElement::FilterType lType, const QtLib::Range &lRange)
    : mTest(lTest), mType(lType), mRange(lRange)
{

}

PATPartFilterElement::PATPartFilterElement(const PATPartFilterElement &lOther)
{
    mTest   = lOther.mTest;
    mType   = lOther.mType;
    mRange  = lOther.mRange;
}

PATPartFilterElement::~PATPartFilterElement()
{

}

bool PATPartFilterElement::IsFiltered(int lIndex) const
{
    bool lResult = true;

    if (mTest && mTest->m_testResult.isValidIndex(lIndex) && mTest->m_testResult.isValidResultAt(lIndex))
    {
        switch(mType)
        {
            case InFilter:
                lResult =  mRange.Contains(mTest->m_testResult.resultAt(lIndex));
                break;

            case OutFilter:
                lResult = !mRange.Contains(mTest->m_testResult.resultAt(lIndex));
                break;

            default:
                GSLOG(SYSLOG_SEV_ERROR, "Incorrect PAT Part Filter type defined.");
                break;
        }
    }

    return lResult;
}

PATPartFilter::PATPartFilter()
{

}

PATPartFilter::~PATPartFilter()
{

}

void PATPartFilter::AddFilterElement(const PATPartFilterElement &lFilter)
{
    mFilters.append(lFilter);
}

void PATPartFilter::RemoveAllFilters()
{
    mFilters.clear();
}

bool PATPartFilter::IsFiltered(int lIndex) const
{
    bool lResult = true;

    for (int lIdx = 0; lIdx < mFilters.count() && lResult; ++lIdx)
    {
        lResult &= mFilters.at(lIdx).IsFiltered(lIndex);
    }

    return lResult;
}

}
}
