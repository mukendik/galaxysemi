#include "lvm_ft_sort_entries_new_bin_map_store.h"
#include "bin_map_store_predicates.h"
#include "lvm_ft_bin_map_item.h"

#include <fstream>

namespace Qx
{
namespace BinMapping
{

LvmFtSortEntriesNewBinMapStore::LvmFtSortEntriesNewBinMapStore(const std::string &aBinMapFilePath,
                                                               const std::string &aConverterExternalFilePath) :
    LvmFtSortEntriesBaseBinMapStore( aBinMapFilePath, aConverterExternalFilePath )
{
    if( aBinMapFilePath.empty() )
        throw InvalidBinMapFilePath( aBinMapFilePath.c_str(), aConverterExternalFilePath.c_str() );

    std::ifstream lFileStream( aBinMapFilePath.c_str(), std::ios_base::in );
    if( ! lFileStream.is_open() )
        throw CannotOpenBinMappingFile(mBinMapFilePath.c_str(), mConverterExternalFilePath.c_str());

    FillBinMapWithStream( lFileStream );
}

const BinMapItemBase &LvmFtSortEntriesNewBinMapStore::GetBinMapItemByTestNumber(int aTestNumber) const
{
    BinMapContainer::const_iterator lItemIterator =
        std::find_if( mBinMapItems.begin(),
                      mBinMapItems.end(),
                      ContainsBinMapItemWithTestNumber( aTestNumber ) );

    mDereferencedItem =
        ( lItemIterator != mBinMapItems.end() ) ?
        *lItemIterator :
        throw BinMapItemNotFound< int >( mBinMapFilePath.c_str(),
                                         mConverterExternalFilePath.c_str(),
                                         aTestNumber );

    return *mDereferencedItem;
}

void LvmFtSortEntriesNewBinMapStore::ProcessReadLine(const std::string &aLine)
{
    const TabularFileLineFields &lFields = ExtractNFieldsFromLine< InvalidLvmFtSeBinMappingFileFormat >( aLine, 7 );

    int lTestNumber = lFields.GetValueAt< int >( 3 );

    if( CanAddItemInBinMap( lTestNumber ) )
        AppendBinMapItemForTest( lFields );
}

bool LvmFtSortEntriesNewBinMapStore::CanAddItemInBinMap( int aTestNumber ) const
{
    BinMapContainer::const_iterator lFoundItem =
        std::find_if(mBinMapItems.begin(), mBinMapItems.end(),
                     ContainsBinMapItemWithTestNumber(aTestNumber));

    if( lFoundItem != mBinMapItems.end() )
        throw DuplicatedTestInLvmSeBinMappingFile( aTestNumber, mBinMapFilePath.c_str(), mConverterExternalFilePath.c_str() );

    return true;
}

void LvmFtSortEntriesNewBinMapStore::AppendBinMapItemForTest( const TabularFileLineFields &aFields )
{
    LvmFtBinMapItem *lItem = new LvmFtBinMapItem();

    lItem->mPass = false;
    lItem->mCount = 0;
    lItem->mTestName = StringManipulations::ToLowerCase( aFields.GetValueAt< std::string >( 2 ) );
    lItem->mTestNumber = aFields.GetValueAt< int >( 3 );
    lItem->mEnabled = aFields.GetValueAt<int>( 0 );
    lItem->mBinning = aFields.GetValueAt<int>( 6 );
    lItem->mBinName = aFields.GetValueAt<std::string>( 1 );

    mBinMapItems.push_back( lItem );
}

}
}
