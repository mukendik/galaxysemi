#include "bin_map_store_base.h"
#include "bin_map_item_base.h"

#include <fstream>

namespace Qx
{
namespace BinMapping
{

BinMapStoreBase::BinMapStoreBase( const std::string &aBinMapFilePath, const std::string &aConverterExternalFilePath ) :
    mBinMapFilePath( aBinMapFilePath ),
    mConverterExternalFilePath( aConverterExternalFilePath ) {}

BinMapStoreBase::~BinMapStoreBase()
{
    for( BinMapContainer::const_iterator lIterator = mBinMapItems.begin(), lEnd = mBinMapItems.end();
         lIterator != lEnd;
         ++lIterator )
        delete *lIterator;
}

void BinMapStoreBase::FillBinMapWithStream(std::ifstream &aFileStream)
{
    for( std::string lReadLine; GetNextTrimmedLineFromStream( aFileStream, lReadLine ); )
        ProcessReadLine( lReadLine );
}

bool BinMapStoreBase::ReadAndTrimLineFromStream(std::ifstream &aFileStream, std::string &aOutputReadLine) const
{
    bool lLineIsRead = static_cast< bool >( std::getline( aFileStream, aOutputReadLine ) );
    aOutputReadLine = StringManipulations::GetSpaceTrimmedString( aOutputReadLine );

    return lLineIsRead;
}

bool BinMapStoreBase::IsRelevantLine( const std::string &aLine ) const
{
    return ( ! IsEmpty( aLine ) && ! IsComment( aLine ) && ! IsHeader( aLine ) );
}

bool BinMapStoreBase::IsEndOfLine(const std::string &aLine) const
{
    return aLine == "\n" || aLine == "\r\n" || aLine == "\r";
}

bool BinMapStoreBase::GetNextTrimmedLineFromStream( std::ifstream &aFileStream,
                                                    std::string &aOutputReadLine) const
{
    bool lLineIsRead;

    do
        lLineIsRead = ReadAndTrimLineFromStream(aFileStream, aOutputReadLine);
    while( lLineIsRead && ! IsRelevantLine( aOutputReadLine ) );

    return lLineIsRead;
}

}
}
