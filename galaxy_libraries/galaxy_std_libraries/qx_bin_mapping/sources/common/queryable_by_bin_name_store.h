#ifndef QUERYABLE_BY_BIN_NAME_STORE_H
#define QUERYABLE_BY_BIN_NAME_STORE_H

#include "bin_mapping_api.h"

#include <string>

namespace Qx
{
namespace BinMapping
{

struct BinMapItemBase;

struct QX_BIN_MAPPING_API_DECL QueryableByBinName
{
    virtual ~QueryableByBinName();

    virtual const BinMapItemBase & GetBinMapItemByBinName( const std::string& aBinName ) const = 0;
};

}
}

#endif // QUERYABLE_BY_BIN_NAME_STORE_H
