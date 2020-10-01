#ifndef LVM_FT_SORT_ENTRIES_BASE_BIN_MAP_STORE_H
#define LVM_FT_SORT_ENTRIES_BASE_BIN_MAP_STORE_H

#include "bin_mapping_api.h"
#include "bin_map_store_base.h"
#include "validity_check_for_bin.h"

namespace Qx
{
namespace BinMapping
{

class QX_BIN_MAPPING_API_DECL LvmFtSortEntriesBaseBinMapStore : public BinMapStoreBase,
                                                                public ValidatableByBin
{
public :
    LvmFtSortEntriesBaseBinMapStore( const std::string &aBinMapFilePath,
                                     const std::string &aConverterExternalFilePath );

    virtual bool IsBinCorrect( int aNumBin, const std::string& aBinName) const;
    virtual std::string RetrieveBinName(int aNumBin) const;

protected:
    virtual bool IsComment(const std::string &aLine) const;
    virtual bool IsHeader( const std::string &aLine ) const;
    bool IsEmpty( const std::string &aLine ) const;
};

}
}

#endif // LVM_FT_SORT_ENTRIES_BASE_BIN_MAP_STORE_H
