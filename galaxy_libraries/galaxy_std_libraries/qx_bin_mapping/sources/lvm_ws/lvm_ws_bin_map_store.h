#ifndef LVM_WS_BIN_MAP_STORE_H
#define LVM_WS_BIN_MAP_STORE_H

#include "bin_mapping_api.h"
#include "bin_map_store_base.h"
#include "queryable_by_test_number_store.h"

#include <string>

namespace Qx
{
namespace BinMapping
{

class QX_BIN_MAPPING_API_DECL LvmWsBinMapStore : public BinMapStoreBase, public QueryableByTestNumber
{
public :
    LvmWsBinMapStore( const std::string &aBinMapFilePath, const std::string &aConverterExternalFilePath );

    const BinMapItemBase & GetBinMapItemByTestNumber( int aTestNumber ) const;

private :
    LvmWsBinMapStore( const LvmWsBinMapStore & );
    LvmWsBinMapStore & operator =( const LvmWsBinMapStore & );

    bool IsComment(const std::string &aLine) const;
    bool CanAddItemInBinMap(int aTestNumber ) const;
    bool IsHeader( const std::string &aLine ) const;
    bool IsEmpty( const std::string &aLine ) const;
    void AppendBinMapItemForTest(int aTestNumber, const TabularFileLineFields &aFields);
    void ProcessReadLine( const std::string &aLine );
};

}
}

#endif // LVM_WS_BIN_MAP_STORE_H
