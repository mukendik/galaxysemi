#ifndef HVM_WS_BIN_MAP_STORE_FET_TEST_H
#define HVM_WS_BIN_MAP_STORE_FET_TEST_H

#include "hvm_ws_bin_map_store.h"
#include "bin_mapping_api.h"

namespace Qx
{
namespace BinMapping
{

class QX_BIN_MAPPING_API_DECL HvmWsBinMapStoreFetTest : public HvmWsBinMapStore
{
public:
     HvmWsBinMapStoreFetTest(const std::string &aBinMapFilePath , const std::string &aConverterExternalFilePath);

private:
    void ProcessLineFields(const TabularFileLineFields &aFields);
    HvmWsBinMapStoreFetTest( const HvmWsBinMapStore & );
    HvmWsBinMapStoreFetTest & operator =( const HvmWsBinMapStore & );
};

}
}

#endif // HVM_WS_BIN_MAP_STORE_FET_TEST_H
