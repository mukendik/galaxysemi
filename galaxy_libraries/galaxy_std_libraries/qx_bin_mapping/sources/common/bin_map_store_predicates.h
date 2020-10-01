#ifndef BIN_MAP_STORE_PREDICATES_H
#define BIN_MAP_STORE_PREDICATES_H

#include "bin_map_item_base.h"
#include "string_manipulations.h"

#include <string>

namespace Qx
{
namespace BinMapping
{


struct ContainsBinMapItemWithBin
{
    int mExpectedBinNumber;
    std::string mExpectedBinName;
    ContainsBinMapItemWithBin( int aBinNumber, const std::string& aBinName ):
        mExpectedBinNumber(aBinNumber),
        mExpectedBinName(aBinName) {}
    bool operator ()( const BinMapItemBase *aItem ) const { return ( (aItem->GetBinNumber() == mExpectedBinNumber) &&
                                                                     (aItem->GetBinName() == mExpectedBinName) ); }
};

struct ContainsBinMapItemWithBinNumber
{
    int mExpectedBinNumber;
    ContainsBinMapItemWithBinNumber( int aBinNumber):mExpectedBinNumber(aBinNumber){}
    bool operator ()( const BinMapItemBase *aItem ) const { return ( (aItem->GetBinNumber() == mExpectedBinNumber)); }
};

struct ContainsBinMapItemWithTestNumber
{
    int mExpectedTestNumber;
    ContainsBinMapItemWithTestNumber( int aTestNumber ) : mExpectedTestNumber( aTestNumber ) {}
    bool operator ()( const BinMapItemBase *aItem ) const { return aItem->GetTestNumber() == mExpectedTestNumber; }
};

struct ContainsBinMapItemWithBinName
{
    const std::string& mExpectedBinNameAlias;
    ContainsBinMapItemWithBinName( const std::string& aBinNameAlias) : mExpectedBinNameAlias( aBinNameAlias ) {}
    bool operator ()( const BinMapItemBase *aItem ) const { return aItem->GetBinNameAlias().compare( mExpectedBinNameAlias ) == 0; }
};

struct ContainsBinMapItemWithTestName
{
    const std::string mExpectedTestName;

    ContainsBinMapItemWithTestName( const std::string& aTestName) :
        mExpectedTestName( StringManipulations::ToLowerCase( aTestName ) ) {}

    bool operator ()( const BinMapItemBase *aItem ) const { return aItem->GetTestName().compare( mExpectedTestName ) == 0; }
};

}
}

#endif // BIN_MAP_STORE_PREDICATES_H
