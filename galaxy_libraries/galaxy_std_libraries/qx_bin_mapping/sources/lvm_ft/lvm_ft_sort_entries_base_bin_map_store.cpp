#include "lvm_ft_sort_entries_base_bin_map_store.h"
#include "bin_map_store_predicates.h"
#include "lvm_ft_bin_map_item.h"

namespace Qx
{
namespace BinMapping
{

LvmFtSortEntriesBaseBinMapStore::LvmFtSortEntriesBaseBinMapStore(const std::string &aBinMapFilePath,
                                                                 const std::string &aConverterExternalFilePath) :
    BinMapStoreBase( aBinMapFilePath, aConverterExternalFilePath ) {}

bool LvmFtSortEntriesBaseBinMapStore::IsBinCorrect(int aNumBin, const std::string &aBinName) const
{
    BinMapContainer::const_iterator lItemIterator =
        std::find_if( mBinMapItems.begin(), mBinMapItems.end(), ContainsBinMapItemWithBin( aNumBin, aBinName ) );

    return  ( lItemIterator != mBinMapItems.end() );
}

std::string LvmFtSortEntriesBaseBinMapStore::RetrieveBinName(int aNumBin) const
{
    BinMapContainer::const_iterator lItemIterator =
        std::find_if( mBinMapItems.begin(), mBinMapItems.end(), ContainsBinMapItemWithBinNumber( aNumBin ) );

    mDereferencedItem =
        ( lItemIterator != mBinMapItems.end() ) ?
        *lItemIterator :
        throw BinMapItemNotFound< int >( mBinMapFilePath.c_str(),
                                         mConverterExternalFilePath.c_str(),
                                         aNumBin );

    return mDereferencedItem->GetBinName();
}

bool LvmFtSortEntriesBaseBinMapStore::IsComment(const std::string &aLine) const
{
    return ( aLine[ 0 ] == '#' );
}

bool LvmFtSortEntriesBaseBinMapStore::IsHeader(const std::string &aLine) const
{
    const std::string &lLowerCaseLine = StringManipulations::ToLowerCase( aLine );

    return StringManipulations::StringStartsWith( lLowerCaseLine, "oti" )
           || StringManipulations::StringStartsWith( lLowerCaseLine, "enabled" );
}

bool LvmFtSortEntriesBaseBinMapStore::IsEmpty(const std::string &aLine) const
{
    return aLine.empty() || IsEndOfLine( aLine );
}

}
}
