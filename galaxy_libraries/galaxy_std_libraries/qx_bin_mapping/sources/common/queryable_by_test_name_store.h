#ifndef QUERYABLE_BY_TEST_NAME_STORE_H
#define QUERYABLE_BY_TEST_NAME_STORE_H

#include "bin_mapping_api.h"

#include <string>

namespace Qx
{
namespace BinMapping
{

struct BinMapItemBase;

struct QX_BIN_MAPPING_API_DECL QueryableByTestName
{
    virtual ~QueryableByTestName();

    virtual const BinMapItemBase & GetBinMapItemByTestName( const std::string& aTestName ) const = 0;
};

}
}

#endif // QUERYABLE_BY_TEST_NAME_STORE_H
