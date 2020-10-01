#include "promis_interpreter_base.h"
#include "promis_item_base.h"
#include "promis_exceptions.h"

#include <fstream>
#include <algorithm>

namespace Qx
{
namespace BinMapping
{

PromisInterpreterBase::PromisInterpreterBase(const std::string &aKey,
                                             const std::string &aPromisFilePath,
                                             const std::string &aConverterExternalFilePath ) :
    mPromisItem( NULL ),
    mKey( aKey ),
    mPromisFilePath( aPromisFilePath ),
    mConverterExternalFilePath( aConverterExternalFilePath )
{
    if( mKey.empty() )
        throw PromisKeyCannotBeEmpty();
}

PromisInterpreterBase::~PromisInterpreterBase()
{
    delete mPromisItem;
}

void PromisInterpreterBase::TrySetPromisItemWithStream( std::ifstream &aFileStream )
{
    for( std::string lReadLine; GetNextTrimmedLineFromStream( aFileStream, lReadLine ); )
        ProcessReadLine( lReadLine );

    if( mPromisItem == NULL )
        throw PromisItemNotFound( mKey.c_str(), mPromisFilePath.c_str(), mConverterExternalFilePath.c_str() );
}

bool PromisInterpreterBase::GetNextTrimmedLineFromStream(std::ifstream &aFileStream, std::string &aOutputReadLine) const
{
    bool lLineIsRead;

    do
        lLineIsRead = ReadAndTrimLineFromStream(aFileStream, aOutputReadLine);
    while( lLineIsRead && ! IsRelevantLine( aOutputReadLine ) );

    return lLineIsRead;
}

bool PromisInterpreterBase::ReadAndTrimLineFromStream(std::ifstream &aFileStream, std::string &aOutputReadLine) const
{
    bool lLineIsRead = static_cast< bool >( std::getline( aFileStream, aOutputReadLine ) );
    aOutputReadLine = StringManipulations::GetSpaceTrimmedString( aOutputReadLine );

    return lLineIsRead;
}

bool PromisInterpreterBase::IsRelevantLine(const std::string &aLine) const
{
    return ( ! IsEmpty( aLine ) && ! IsComment( aLine ) && ! IsHeader( aLine ) );
}

bool PromisInterpreterBase::IsEmpty(const std::string &aLine) const
{
    if( aLine.empty() )
        return true;

    std::string lStringWithoutEndOfLineChars( aLine );

    lStringWithoutEndOfLineChars.erase( std::remove( lStringWithoutEndOfLineChars.begin(),
                                                     lStringWithoutEndOfLineChars.end(), '\r'),
                                        lStringWithoutEndOfLineChars.end() );
    lStringWithoutEndOfLineChars.erase( std::remove( lStringWithoutEndOfLineChars.begin(),
                                                     lStringWithoutEndOfLineChars.end(), '\n'),
                                        lStringWithoutEndOfLineChars.end() );

    return lStringWithoutEndOfLineChars.empty();
}

}
}
