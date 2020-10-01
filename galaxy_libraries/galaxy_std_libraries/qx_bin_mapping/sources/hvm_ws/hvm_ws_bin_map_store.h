#ifndef HVM_WS_BIN_MAP_STORE_H
#define HVM_WS_BIN_MAP_STORE_H

#include <string>

#include "bin_mapping_api.h"
#include "bin_map_store_base.h"
#include "queryable_by_test_number_store.h"

namespace Qx
{
namespace BinMapping
{

class QX_BIN_MAPPING_API_DECL HvmWsBinMapStore : public BinMapStoreBase, public QueryableByTestNumber
{
public :
    const BinMapItemBase & GetBinMapItemByTestNumber( int aTestNumber ) const;

protected:
    HvmWsBinMapStore( const std::string &aBinMapFilePath,
                      const std::string &aConverterExternalFilePath );

    virtual void ProcessLineFields(const TabularFileLineFields &aFields) = 0;
    void AppendBinMapItem(int aTestNumber, const TabularFileLineFields &aFields);
    bool CanAddItemInBinMap(int aTestNumber ) const;
    void ProcessReadLine( const std::string &aLine );
    bool IsComment(const std::string &aLine) const;
    bool IsHeader(const std::string &aLine) const;
    bool IsEmpty( const std::string &aLine ) const;
    bool TryExtractOptionalTestNumberFromFields( const TabularFileLineFields &aFields,
                                                 int &aOutputTestNumber ) const;

private :
    HvmWsBinMapStore( const HvmWsBinMapStore & );
    HvmWsBinMapStore & operator =( const HvmWsBinMapStore & );
};

}
}

#endif // HVM_WS_BIN_MAP_STORE_H
