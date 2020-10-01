#include "hvm_ws_bin_map_store_fet_test.h"
#include "bin_map_store_predicates.h"

#include <fstream>

namespace Qx
{
namespace BinMapping
{

HvmWsBinMapStoreFetTest::HvmWsBinMapStoreFetTest(const std::string &aBinMapFilePath,
                                                 const std::string &aConverterExternalFilePath ) :
    HvmWsBinMapStore( aBinMapFilePath, aConverterExternalFilePath )
{
    std::ifstream lIfstream (aBinMapFilePath.c_str(), std::ios_base::in );

    if( ! lIfstream.is_open() )
        throw CannotOpenBinMappingFile(mBinMapFilePath.c_str(), mConverterExternalFilePath.c_str());

    FillBinMapWithStream( lIfstream );
}

void HvmWsBinMapStoreFetTest::ProcessLineFields(const TabularFileLineFields &aFields)
{
    int lTestNumber;

    if( ! TryExtractOptionalTestNumberFromFields( aFields, lTestNumber ) )
        return;

    if( CanAddItemInBinMap( lTestNumber ) )
        AppendBinMapItem( lTestNumber, aFields );
}

}
}
