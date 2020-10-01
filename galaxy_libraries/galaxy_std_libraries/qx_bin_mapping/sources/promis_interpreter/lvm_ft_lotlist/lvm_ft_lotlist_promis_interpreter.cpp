#include "lvm_ft_lotlist_promis_interpreter.h"
#include "promis_exceptions.h"
#include "lvm_ft_lotlist_promis_item.h"

#include <fstream>

namespace Qx
{
namespace BinMapping
{

LvmFtLotlistPromisInterpreter::LvmFtLotlistPromisInterpreter( const std::string &aKey,
                                                              const std::string &aPromisFilePath,
                                                              const std::string &aConverterExternalFilePath ) :
    PromisInterpreterBase ( aKey, aPromisFilePath, aConverterExternalFilePath )
{
    if( aPromisFilePath.empty() )
        throw InvalidPromisFilePath( mConverterExternalFilePath.c_str() );

    std::ifstream lFileStream( aPromisFilePath.c_str(), std::ios_base::in );
    if( ! lFileStream.is_open() )
        throw CannotOpenPromisFile( aPromisFilePath.c_str(), aConverterExternalFilePath.c_str() );

    TrySetPromisItemWithStream( lFileStream );
}

LvmFtLotlistPromisInterpreter::~LvmFtLotlistPromisInterpreter() {}

const PromisItemBase &LvmFtLotlistPromisInterpreter::GetPromisItem() const
{
    return *mPromisItem;
}

void LvmFtLotlistPromisInterpreter::ProcessReadLine(const std::string &aLine)
{
    const TabularFileLineFields &lFields = ExtractNFieldsFromLine< InvalidLvmFtLotlistPromisFileFormat >( aLine, 3 );

    const std::string &lKey = lFields.GetValueAt< std::string >( 0 );

    if( CanSetItem( lKey ) )
        SetItem( lFields );
}

bool LvmFtLotlistPromisInterpreter::IsComment(const std::string &aLine) const
{
    // the concept of a comment is not specified yet
    ( void ) aLine;

    return false;
}

bool LvmFtLotlistPromisInterpreter::IsHeader(const std::string &aLine) const
{
    // the concept of a header is not specified yet
    (void) aLine;

    return false;
}

bool LvmFtLotlistPromisInterpreter::CanSetItem(const std::string &lKey) const
{
    return ( ( mPromisItem == NULL ) && ( lKey.compare( mKey ) == 0 ) );
}

void LvmFtLotlistPromisInterpreter::SetItem(const TabularFileLineFields &aFields)
{
    LvmFtLotlistPromisItem *lItem = new LvmFtLotlistPromisItem();

    lItem->mDateCode = aFields.GetValueAt< std::string >( 1 );
    lItem->mSiteLocation = aFields.GetValueAt< std::string >( 2 );

    mPromisItem = lItem;
}

}
}