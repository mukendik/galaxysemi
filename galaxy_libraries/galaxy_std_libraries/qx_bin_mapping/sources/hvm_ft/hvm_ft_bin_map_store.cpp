#include "hvm_ft_bin_map_store.h"
#include "tabular_file_line_fields.h"
#include "hvm_ft_bin_map_item.h"
#include "bin_map_store_predicates.h"

#include <fstream>
#include <algorithm>

namespace Qx
{
namespace BinMapping
{

HvmFtBinMapStore::HvmFtBinMapStore(const std::string &aBinMapFilePath,
                                   const std::string &aConverterExternalFilePath) :
    BinMapStoreBase( aBinMapFilePath, aConverterExternalFilePath )
{
    if( aBinMapFilePath.empty() )
        throw InvalidBinMapFilePath( aBinMapFilePath.c_str(), aConverterExternalFilePath.c_str() );

    std::ifstream lFileStream( aBinMapFilePath.c_str(), std::ios_base::in );
    if( ! lFileStream.is_open() )
        throw CannotOpenBinMappingFile( mBinMapFilePath.c_str(), mConverterExternalFilePath.c_str() );

    FillBinMapWithStream( lFileStream );
}

const BinMapItemBase &HvmFtBinMapStore::GetBinMapItemByBinName( const std::string& aBinName) const
{
    BinMapContainer::const_iterator lItemIterator =
        std::find_if( mBinMapItems.begin(), mBinMapItems.end(), ContainsBinMapItemWithBinName( aBinName ) );

    mDereferencedItem =
        ( lItemIterator != mBinMapItems.end() ) ?
        *lItemIterator :
        throw BinMapItemNotFound< std::string >( mBinMapFilePath.c_str(),
                                                 mConverterExternalFilePath.c_str(),
                                                 aBinName );

    return *mDereferencedItem;
}

bool HvmFtBinMapStore::IsComment( const std::string &aLine ) const
{
    return ( aLine[ 0 ] == '#' );
}

bool HvmFtBinMapStore::CanAddItemInBinMap( const std::string& aBinName) const
{
    return std::find_if( mBinMapItems.begin(), mBinMapItems.end(),
                         ContainsBinMapItemWithBinName( aBinName ) ) == mBinMapItems.end();
}

bool HvmFtBinMapStore::IsHeader(const std::string &aLine) const
{
    const std::string &lLowerCaseLine = StringManipulations::ToLowerCase( aLine );
    return StringManipulations::StringStartsWith( lLowerCaseLine, "hw bin" );
}

bool HvmFtBinMapStore::IsEmpty(const std::string &aLine) const
{
    return aLine.empty() || IsEndOfLine( aLine );
}

void HvmFtBinMapStore::AppendBinMapItemForBin(const std::string& aBinName , const TabularFileLineFields & aFields)
{
    HvmFtBinMapItem *lItem = new HvmFtBinMapItem();

    lItem->mBinNameAlias = aBinName;
    lItem->mHardBinNumber = aFields.GetValueAt< int >( 0 );
    lItem->mSoftBinNumber = aFields.GetValueAt< int >( 1 );
    lItem->mBinName = StringManipulations::UniqueConsecutiveWhitespaces(aFields.GetValueAt< std::string >( 3 ));
    lItem->mBinCategory = (lItem->mHardBinNumber == 1) ? 'P' : 'F';

    mBinMapItems.push_back( lItem );
}

void HvmFtBinMapStore::ProcessReadLine(const std::string &aLine)
{
    const TabularFileLineFields &lFields = ExtractNFieldsFromLine< InvalidHvmFtBinMappingFileFormat >( aLine, 4 );

    int lHardBin = lFields.GetValueAt< int >( 0 );

    std::string lBinBame =
        ( lHardBin == 1 )
        ? "001 /S/L"
        : StringManipulations::UniqueConsecutiveWhitespaces( lFields.GetValueAt< std::string >( 2 ) );

    if( CanAddItemInBinMap( lBinBame ) )
        AppendBinMapItemForBin( lBinBame, lFields );
}

}
}
