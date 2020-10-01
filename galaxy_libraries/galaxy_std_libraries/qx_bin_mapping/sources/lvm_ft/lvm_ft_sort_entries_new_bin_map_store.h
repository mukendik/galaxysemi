#ifndef LVM_FT_SORT_ENTRIES_NEW_BIN_MAP_STORE_H
#define LVM_FT_SORT_ENTRIES_NEW_BIN_MAP_STORE_H

#include "bin_mapping_api.h"
#include "lvm_ft_sort_entries_base_bin_map_store.h"
#include "queryable_by_test_number_store.h"

namespace Qx
{
namespace BinMapping
{

class QX_BIN_MAPPING_API_DECL LvmFtSortEntriesNewBinMapStore : public LvmFtSortEntriesBaseBinMapStore,
                                                               public QueryableByTestNumber
{
public :
    LvmFtSortEntriesNewBinMapStore( const std::string &aBinMapFilePath, const std::string &aConverterExternalFilePath);

    virtual const BinMapItemBase & GetBinMapItemByTestNumber( int aTestNumber ) const;

private :
    void ProcessReadLine( const std::string &aLine );
    bool CanAddItemInBinMap(int aTestNumber ) const;
    void AppendBinMapItemForTest(const TabularFileLineFields &aFields);
};


}
}

#endif // LVM_FT_SORT_ENTRIES_NEW_BIN_MAP_STORE_H
