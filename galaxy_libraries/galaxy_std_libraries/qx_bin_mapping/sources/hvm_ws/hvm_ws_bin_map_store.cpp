#ifndef HVM_WS_BIN_MAP_STORE_CPP
#define HVM_WS_BIN_MAP_STORE_CPP

#include <fstream>
#include <algorithm>

#include "hvm_ws_bin_map_item.h"
#include "hvm_ws_bin_map_store.h"
#include "bin_map_store_predicates.h"

namespace Qx
{
namespace BinMapping
{

const BinMapItemBase &HvmWsBinMapStore::GetBinMapItemByTestNumber(int aKeyNumber) const
{
    BinMapContainer::const_iterator lItemIterator =
        std::find_if( mBinMapItems.begin(), mBinMapItems.end(), ContainsBinMapItemWithTestNumber( aKeyNumber ) );

    mDereferencedItem =
        ( lItemIterator != mBinMapItems.end() ) ?
        *lItemIterator :
        throw BinMapItemNotFound< int >( mBinMapFilePath.c_str(),
                                         mConverterExternalFilePath.c_str(),
                                         aKeyNumber );

    return *mDereferencedItem;
}

HvmWsBinMapStore::HvmWsBinMapStore(const std::string &aBinMapFilePath,
                                   const std::string &aConverterExternalFilePath) :
    BinMapStoreBase( aBinMapFilePath, aConverterExternalFilePath )
{
    if( aBinMapFilePath.empty() )
        throw InvalidBinMapFilePath( aBinMapFilePath.c_str(), aConverterExternalFilePath.c_str() );
}

bool HvmWsBinMapStore::IsComment( const std::string &aLine ) const
{
    return ( aLine[ 0 ] == '#' );
}

void HvmWsBinMapStore::ProcessReadLine(const std::string &aLine)
{
    const TabularFileLineFields &lFields = ExtractNFieldsFromLine< InvalidHvmWsBinMappingFileFormat >( aLine, 4 );

    ProcessLineFields(lFields);
}


bool HvmWsBinMapStore::CanAddItemInBinMap(int aTestNumber) const
{
    return std::find_if( mBinMapItems.begin(), mBinMapItems.end(),
                         ContainsBinMapItemWithTestNumber( aTestNumber ) ) == mBinMapItems.end();
}

bool HvmWsBinMapStore::IsHeader(const std::string &aLine) const
{
    const std::string &lLowerCaseLine = StringManipulations::ToLowerCase( aLine );

    return
        StringManipulations::StringStartsWith( lLowerCaseLine, "hw bin" )
            || StringManipulations::StringStartsWith( lLowerCaseLine, "bin #" );
}

bool HvmWsBinMapStore::IsEmpty(const std::string &aLine) const
{
    return aLine.empty() || IsEndOfLine( aLine );
}

bool HvmWsBinMapStore::TryExtractOptionalTestNumberFromFields(const TabularFileLineFields &aFields, int &aOutputTestNumber) const
{
    // file format is messy, and sometime it is normal a conversion fails for the field 2
    try
    {
        aOutputTestNumber = aFields.GetValueAt< int >( 2 );
    }
    catch ( const BadFieldFormat & )
    {
        return false;
    }

    return true;
}

void HvmWsBinMapStore::AppendBinMapItem(int aTestNumber , const TabularFileLineFields &aFields)
{
    HvmWsBinMapItem *lItem  = new HvmWsBinMapItem();

    lItem->mTestNumber = aTestNumber;
    lItem->mTestName = aFields.GetValueAt< std::string >(3);
    lItem->mBinningNumber =  aFields.GetValueAt< int >(0);
    lItem->mBinningName = lItem->mTestName;
    lItem->mPass = ( lItem->mBinningNumber == 1 );

    mBinMapItems.push_back( lItem );
}

} //namespace BinMapping
} //namespace Qx

#endif // HVM_WS_BIN_MAP_STORE_CPP
