#include "hvm_ws_bin_map_store_specktra.h"
#include "bin_map_store_predicates.h"

#include <fstream>

namespace Qx
{
namespace BinMapping
{

HvmWsBinMapStoreSpecktra::HvmWsBinMapStoreSpecktra(const std::string &aBinMapFilePath,
                                                   const std::string &aConverterExternalFilePath) :
    HvmWsBinMapStore( aBinMapFilePath, aConverterExternalFilePath )
{
    std::ifstream lIfstream (aBinMapFilePath.c_str(), std::ios_base::in );

    if( ! lIfstream.is_open() )
        throw CannotOpenBinMappingFile(mBinMapFilePath.c_str(), mConverterExternalFilePath.c_str());

    FillBinMapWithStream( lIfstream );
}

void HvmWsBinMapStoreSpecktra::ProcessLineFields(const TabularFileLineFields &aFields)
{
    int lBinCode = aFields.GetValueAt< int >(0);

    BinMapContainer::iterator lIter = std::find_if( mBinMapItems.begin(), mBinMapItems.end(),
                                                    ContainsBinMapItemWithTestNumber( lBinCode ) );

    if(lIter != mBinMapItems.end())
        mBinMapItems.erase(lIter);

    AppendBinMapItem(lBinCode, aFields);
}

}
}
