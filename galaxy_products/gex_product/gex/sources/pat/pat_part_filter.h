#ifndef PAT_PART_FILTER_H
#define PAT_PART_FILTER_H

// Galaxy includes
#include <gqtl_utils.h>

class CTest;

namespace GS
{
namespace Gex
{

class PATPartFilterElement
{
public:

    enum FilterType
    {
        InFilter,
        OutFilter
    };

    PATPartFilterElement(const CTest * lTest, FilterType lType, const GS::QtLib::Range& lRange);
    PATPartFilterElement(const PATPartFilterElement& lOther);
    ~PATPartFilterElement();

    bool                IsFiltered(int lIndex) const;

private:

    const CTest *       mTest;
    FilterType          mType;
    GS::QtLib::Range    mRange;
};

class PATPartFilter
{
public:
    PATPartFilter();
    ~PATPartFilter();

    void                AddFilterElement(const PATPartFilterElement& lFilter);
    void                RemoveAllFilters();
    bool                IsFiltered(int lIndex) const;

private:

    QList<PATPartFilterElement> mFilters;

};

}
}
#endif // PAT_PART_FILTER_H
