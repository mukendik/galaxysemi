#ifndef QUERYABLE_BY_TEST_NUMBER_STORE_H
#define QUERYABLE_BY_TEST_NUMBER_STORE_H

#include "bin_mapping_api.h"

namespace Qx
{
namespace BinMapping
{

struct BinMapItemBase;

struct QX_BIN_MAPPING_API_DECL QueryableByTestNumber
{
    virtual ~QueryableByTestNumber();

    virtual const BinMapItemBase & GetBinMapItemByTestNumber( int aTestNumber ) const = 0;
};

}
}

#endif // QUERYABLE_BY_TEST_NUMBER_STORE_H
