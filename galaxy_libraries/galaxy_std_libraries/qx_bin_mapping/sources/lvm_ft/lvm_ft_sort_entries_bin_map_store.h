#ifndef LVM_FT_SORT_ENTRIES_BIN_MAP_STORE_H
#define LVM_FT_SORT_ENTRIES_BIN_MAP_STORE_H

#include "bin_mapping_api.h"
#include "lvm_ft_sort_entries_base_bin_map_store.h"
#include "queryable_by_test_name_store.h"

#include <string>

namespace Qx
{
namespace BinMapping
{

class QX_BIN_MAPPING_API_DECL LvmFtSortEntriesBinMapStore : public LvmFtSortEntriesBaseBinMapStore,
                                                            public QueryableByTestName
{
public :
    LvmFtSortEntriesBinMapStore( const std::string &aBinMapFilePath, const std::string &aConverterExternalFilePath );

    const BinMapItemBase & GetBinMapItemByTestName( const std::string& aBinName ) const;

private :
    void ProcessReadLine( const std::string &aLine );
    bool CanAddItemInBinMap( const std::string &aTestName ) const;
    void AppendBinMapItemForTest(const std::string &aTestName, const TabularFileLineFields &aFields);
};

}
}

#endif // LVM_FT_SORT_ENTRIES_BIN_MAP_STORE_H
