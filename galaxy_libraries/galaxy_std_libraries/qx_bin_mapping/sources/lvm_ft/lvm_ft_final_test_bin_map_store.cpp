#include "lvm_ft_final_test_bin_map_store.h"
#include "tabular_file_line_fields.h"
#include "lvm_ft_bin_map_item.h"
#include "bin_map_store_predicates.h"

#include <fstream>
#include <algorithm>

namespace Qx
{
namespace BinMapping
{

LvmFtFinalTestBinMapStore::LvmFtFinalTestBinMapStore(const std::string &aBinMapFilePath,
                                                     const std::string &aConverterExternalFilePath) :
    BinMapStoreBase( aBinMapFilePath, aConverterExternalFilePath )
{
    if( aBinMapFilePath.empty() )
        throw InvalidBinMapFilePath( aBinMapFilePath.c_str(), aConverterExternalFilePath.c_str() );

    std::ifstream lFileStream( aBinMapFilePath.c_str(), std::ios_base::in );
    if( ! lFileStream.is_open() )
        throw CannotOpenBinMappingFile(mBinMapFilePath.c_str(), mConverterExternalFilePath.c_str());

    FillBinMapWithStream( lFileStream );
}

bool LvmFtFinalTestBinMapStore::IsBinCorrect(int aNumBin, const std::string& aBinName) const
{
    BinMapContainer::const_iterator lItemIterator =
        std::find_if( mBinMapItems.begin(), mBinMapItems.end(), ContainsBinMapItemWithBin( aNumBin, aBinName ) );

    return  ( lItemIterator != mBinMapItems.end() );
}

std::string LvmFtFinalTestBinMapStore::RetrieveBinName(int aNumBin) const
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

const BinMapItemBase &LvmFtFinalTestBinMapStore::GetBinMapItemByTestNumber( int aTestNumber) const
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

bool LvmFtFinalTestBinMapStore::IsComment( const std::string &aLine ) const
{
    return ( aLine[ 0 ] == '#' );
}

bool LvmFtFinalTestBinMapStore::CanAddItemInBinMap(int aTestNumber) const
{
    if( aTestNumber == 0 )
        return false;

    if (aTestNumber > 100 && aTestNumber <= 200)
        throw InvalidTestInLvmFtBinMappingFile( mBinMapFilePath.c_str(),
                                                mConverterExternalFilePath.c_str(),
                                                aTestNumber );

    BinMapContainer::const_iterator lFoundItem =
        std::find_if(mBinMapItems.begin(), mBinMapItems.end(),
                     ContainsBinMapItemWithTestNumber(aTestNumber));

    if( lFoundItem != mBinMapItems.end() )
        throw DuplicatedTestInLvmFtBinMappingFile( aTestNumber, mBinMapFilePath.c_str(), mConverterExternalFilePath.c_str() );

    return true;
}

bool LvmFtFinalTestBinMapStore::IsHeader(const std::string &aLine) const
{
    const std::string &lLowerCaseLine = StringManipulations::ToLowerCase( aLine );

    return StringManipulations::StringStartsWith( lLowerCaseLine, "oti" )
           || StringManipulations::StringStartsWith( lLowerCaseLine, "enabled" );
}

bool LvmFtFinalTestBinMapStore::IsEmpty(const std::string &aLine) const
{
    return aLine.empty() || IsEndOfLine( aLine );
}

void LvmFtFinalTestBinMapStore::AppendBinMapItemForTest(int aTestNumber , const TabularFileLineFields & aFields)
{
    LvmFtBinMapItem *lItem = new LvmFtBinMapItem();

    lItem->mTestNumber = aTestNumber;
    lItem->mEnabled = aFields.GetValueAt<bool>(0);

    if(lItem->mEnabled)
    {
        lItem->mBinning = aFields.GetValueAt<int>(7);
        lItem->mBinName = aFields.GetValueAt<std::string>(8);
        lItem->mTestName = aFields.GetValueAt< std::string >(9);
    }

    mBinMapItems.push_back( lItem );
}

std::vector<int> LvmFtFinalTestBinMapStore::ExtractAllTestNumbersFromString(const std::string &aNumbers) const
{
    std::vector<int> lList;
    lList.reserve( 2 );

    std::size_t lSeparatorPosition = aNumbers.find("/");

    if (lSeparatorPosition == std::string::npos)
        lList.push_back( StringManipulations::GetValueFromString< int >( aNumbers ) );
    else
    {
        lList.push_back( StringManipulations::GetValueFromString< int >( aNumbers.substr( 0, lSeparatorPosition ) ) );
        lList.push_back( StringManipulations::GetValueFromString< int >( aNumbers.substr( lSeparatorPosition + 1 ) ) );
    }

    return lList;
}

bool LvmFtFinalTestBinMapStore::AreAllFieldsRelevant(const TabularFileLineFields &aFields) const
{
    const std::string &lTestNameLower = StringManipulations::ToLowerCase(aFields.GetValueAt<std::string>(8));

    return ! lTestNameLower.empty() && ! StringManipulations::StringStartsWith(lTestNameLower, "condition");
}

std::vector<int> LvmFtFinalTestBinMapStore::GetAllTestNumberFromFields(const TabularFileLineFields &aFields) const
{
    std::string lTestNumbersString = aFields.GetValueAt<std::string>(2);

    return ExtractAllTestNumbersFromString(lTestNumbersString);
}

void LvmFtFinalTestBinMapStore::ProcessReadLineForAllTestNumbersInFields(const TabularFileLineFields &aFields)
{
    const std::vector<int> &lTestNumbers = GetAllTestNumberFromFields( aFields );

    for ( std::size_t lIndex=0; lIndex < lTestNumbers.size(); ++lIndex )
        if( CanAddItemInBinMap( lTestNumbers[ lIndex ] ) )
            AppendBinMapItemForTest( lTestNumbers[ lIndex ], aFields );
}

void LvmFtFinalTestBinMapStore::ProcessReadLine(const std::string &aLine)
{
    const TabularFileLineFields &lFields = ExtractNFieldsFromLine< InvalidLvmFtFtBinMappingFileFormat >( aLine, 10 );

    int lTestNumber = lFields.GetValueAt<int>(6);

    if( CanAddItemInBinMap( lTestNumber ) )
        AppendBinMapItemForTest( lTestNumber, lFields );
}

}
}
