#include "lvm_ft_sort_entries_bin_map_store.h"
#include "bin_map_store_predicates.h"
#include "lvm_ft_bin_map_item.h"

#include <fstream>

namespace Qx
{
namespace BinMapping
{

LvmFtSortEntriesBinMapStore::LvmFtSortEntriesBinMapStore(const std::string &aBinMapFilePath,
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

const BinMapItemBase &LvmFtSortEntriesBinMapStore::GetBinMapItemByTestName( const std::string& aBinName) const
{
    BinMapContainer::const_iterator lItemIterator =
        std::find_if( mBinMapItems.begin(),
                      mBinMapItems.end(),
                      ContainsBinMapItemWithTestName( aBinName ) );

    mDereferencedItem =
        ( lItemIterator != mBinMapItems.end() ) ?
        *lItemIterator :
        throw BinMapItemNotFound< std::string >( mBinMapFilePath.c_str(),
                                                 mConverterExternalFilePath.c_str(),
                                                 aBinName );

    return *mDereferencedItem;
}

void LvmFtSortEntriesBinMapStore::ProcessReadLine(const std::string &aLine)
{
    const TabularFileLineFields &lFields = ExtractNFieldsFromLine< InvalidLvmFtSeBinMappingFileFormat >( aLine, 7 );

    const std::string &lTestName = lFields.GetValueAt<std::string>( 2 );

    if(CanAddItemInBinMap(lTestName))
        AppendBinMapItemForTest(lTestName, lFields);
}

bool LvmFtSortEntriesBinMapStore::CanAddItemInBinMap(const std::string &aTestName) const
{
    const std::string &lBinNameLower = StringManipulations::ToLowerCase( aTestName );

    return
        ( ! lBinNameLower.empty() && ! StringManipulations::StringStartsWith( lBinNameLower, "condition" ) )
        && ( std::find_if( mBinMapItems.begin(),
                           mBinMapItems.end(),
                           ContainsBinMapItemWithTestName( aTestName ) ) == mBinMapItems.end() );
}

void LvmFtSortEntriesBinMapStore::AppendBinMapItemForTest( const std::string &aTestName,
                                                           const TabularFileLineFields &aFields )
{
    LvmFtBinMapItem *lItem = new LvmFtBinMapItem();

    lItem->mPass = false;
    lItem->mCount = 0;
    lItem->mTestName = StringManipulations::ToLowerCase( aTestName );
    lItem->mTestNumber = aFields.GetValueAt< int >( 3 );
    lItem->mEnabled = aFields.GetValueAt<int>( 0 );
    lItem->mBinning = aFields.GetValueAt<int>( 6 );
    lItem->mBinName = aFields.GetValueAt<std::string>( 1 );

    mBinMapItems.push_back( lItem );
}

}
}
