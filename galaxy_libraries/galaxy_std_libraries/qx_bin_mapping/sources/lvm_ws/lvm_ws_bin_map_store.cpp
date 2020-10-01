#include "lvm_ws_bin_map_store.h"
#include "tabular_file_line_fields.h"
#include "lvm_ws_bin_map_item.h"
#include "bin_map_store_predicates.h"

#include <fstream>
#include <algorithm>

namespace Qx
{
namespace BinMapping
{

LvmWsBinMapStore::LvmWsBinMapStore(const std::string &aBinMapFilePath,
                                   const std::string &aConverterExternalFilePath ) :
    BinMapStoreBase( aBinMapFilePath, aConverterExternalFilePath )
{
    if( aBinMapFilePath.empty() )
        throw InvalidBinMapFilePath( aBinMapFilePath.c_str(), aConverterExternalFilePath.c_str() );

    std::ifstream lFileStream( aBinMapFilePath.c_str(), std::ios_base::in );
    if( ! lFileStream.is_open() )
        throw CannotOpenBinMappingFile( mBinMapFilePath.c_str(), mConverterExternalFilePath.c_str() );

    FillBinMapWithStream( lFileStream );
}

const BinMapItemBase &LvmWsBinMapStore::GetBinMapItemByTestNumber(int aTestNumber) const
{
    BinMapContainer::const_iterator lItemIterator =
        std::find_if( mBinMapItems.begin(), mBinMapItems.end(), ContainsBinMapItemWithTestNumber( aTestNumber ) );

    mDereferencedItem =
        ( lItemIterator != mBinMapItems.end() ) ?
        *lItemIterator :
        throw BinMapItemNotFound< int >( mBinMapFilePath.c_str(),
                                         mConverterExternalFilePath.c_str(),
                                         aTestNumber );

    return *mDereferencedItem;
}

bool LvmWsBinMapStore::IsComment( const std::string &aLine ) const
{
    return ( aLine[ 0 ] == '#' );
}

bool LvmWsBinMapStore::CanAddItemInBinMap(int aTestNumber) const
{
    BinMapContainer::const_iterator lFoundItem =
        std::find_if( mBinMapItems.begin(), mBinMapItems.end(),
                      ContainsBinMapItemWithTestNumber( aTestNumber ) );

    if( lFoundItem != mBinMapItems.end() )
        throw DuplicatedTestInLvmWsBinMappingFile( aTestNumber,
                                                   mBinMapFilePath.c_str(),
                                                   mConverterExternalFilePath.c_str() );

    return true;
}

bool LvmWsBinMapStore::IsHeader(const std::string &aLine) const
{
    const std::string &lLowerCaseLine = StringManipulations::ToLowerCase( aLine );
    return StringManipulations::StringStartsWith( lLowerCaseLine, "test name" );
}

bool LvmWsBinMapStore::IsEmpty(const std::string &aLine) const
{
    return aLine.empty() || IsEndOfLine( aLine );
}

void LvmWsBinMapStore::AppendBinMapItemForTest(int aTestNumber , const TabularFileLineFields &aFields)
{
    LvmWsBinMapItem *lItem = new LvmWsBinMapItem();
    lItem->mTestNumber = aTestNumber;
    lItem->mTestName = aFields.GetValueAt< std::string >( 3 );
    lItem->mSoftBinNumber =  aFields.GetValueAt< int >( 2 );
    lItem->mBinningName = lItem->mTestName;

    mBinMapItems.push_back( lItem );
}

void LvmWsBinMapStore::ProcessReadLine(const std::string &aLine)
{
    const TabularFileLineFields &lFields = ExtractNFieldsFromLine< InvalidLvmWsBinMappingFileFormat >( aLine, 4 );

    int lTestNumber = lFields.GetValueAt< int >( 1 );

    if( CanAddItemInBinMap( lTestNumber ) )
        AppendBinMapItemForTest( lTestNumber, lFields );
}

}
}
