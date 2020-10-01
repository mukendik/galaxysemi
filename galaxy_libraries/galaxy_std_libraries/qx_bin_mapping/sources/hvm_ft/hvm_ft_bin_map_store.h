#ifndef HVM_FT_BIN_MAP_STORE_H
#define HVM_FT_BIN_MAP_STORE_H

#include "bin_mapping_api.h"
#include "bin_map_store_base.h"
#include "queryable_by_bin_name_store.h"

#include <string>

namespace Qx
{
namespace BinMapping
{

class QX_BIN_MAPPING_API_DECL HvmFtBinMapStore : public BinMapStoreBase, public QueryableByBinName
{
public :
    HvmFtBinMapStore( const std::string &aBinMapFilePath,
                      const std::string &aConverterExternalFilePath );

    const BinMapItemBase & GetBinMapItemByBinName( const std::string& aBinName ) const;

private :
    HvmFtBinMapStore( const HvmFtBinMapStore & );
    HvmFtBinMapStore & operator =( const HvmFtBinMapStore & );

    bool CanAddItemInBinMap(const std::string& aBinName) const;
    bool IsComment(const std::string &aLine) const;
    bool IsHeader( const std::string &aLine ) const;
    bool IsEmpty( const std::string &aLine ) const;
    void AppendBinMapItemForBin(const std::string &aBinName, const TabularFileLineFields &aFields);
    void ProcessReadLine( const std::string &aLine );
};

}
}

#endif // HVM_FT_BIN_MAP_STORE_H
