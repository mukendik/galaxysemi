#include "lvm_ft_subcon_data_promis_interpreter.h"
#include "promis_exceptions.h"
#include "lvm_ft_subcon_data_promis_item.h"

#include <fstream>

namespace Qx
{
namespace BinMapping
{

LvmFtSubconDataPromisInterpreter::LvmFtSubconDataPromisInterpreter( const std::string &aKey,
                                                                    const std::string &aPromisFilePath ,
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

LvmFtSubconDataPromisInterpreter::~LvmFtSubconDataPromisInterpreter() {}

const PromisItemBase &LvmFtSubconDataPromisInterpreter::GetPromisItem() const
{
    return *mPromisItem;
}

void LvmFtSubconDataPromisInterpreter::ProcessReadLine(const std::string &aLine)
{
    const TabularFileLineFields &lFields = ExtractNFieldsFromLine< InvalidLvmFtSubconDataPromisFileFormat >( aLine, 4 );

    const std::string &lKey = lFields.GetValueAt< std::string >( 0 );

    if( CanSetItem( lKey ) )
        SetItem( lFields );
}

bool LvmFtSubconDataPromisInterpreter::IsComment(const std::string &aLine) const
{
    // the concept of a comment is not specified yet
    ( void ) aLine;

    return false;
}

bool LvmFtSubconDataPromisInterpreter::IsHeader(const std::string &aLine) const
{
    // the concept of a header is not specified yet
    (void) aLine;

    return false;
}

bool LvmFtSubconDataPromisInterpreter::CanSetItem(const std::string &lKey) const
{
    return ( ( mPromisItem == NULL ) && ( lKey.compare( mKey ) == 0 ) );
}

void LvmFtSubconDataPromisInterpreter::SetItem(const TabularFileLineFields &aFields)
{
    LvmFtSubconDataPromisItem *lItem = new LvmFtSubconDataPromisItem();

    lItem->mPartNumber = aFields.GetValueAt< std::string >( 1 );
    lItem->mPackageType = aFields.GetValueAt< std::string >( 2 );
    lItem->mGeometryName = aFields.GetValueAt< std::string >( 3 );

    mPromisItem = lItem;
}

}
}

