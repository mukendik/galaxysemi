#include "hvm_ft_promis_interpreter.h"
#include "promis_exceptions.h"
#include "hvm_ft_promis_item.h"

#include <fstream>

namespace Qx
{
namespace BinMapping
{

HvmFtPromisInterpreter::HvmFtPromisInterpreter( const std::string &aKey,
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

HvmFtPromisInterpreter::~HvmFtPromisInterpreter() {}

const PromisItemBase &HvmFtPromisInterpreter::GetPromisItem() const
{
    return *mPromisItem;
}

void HvmFtPromisInterpreter::ProcessReadLine(const std::string &aLine)
{
    const TabularFileLineFields &lFields = ExtractNFieldsFromLine< InvalidHvmFtPromisFileFormat >( aLine, 9 );

    const std::string &lKey = lFields.GetValueAt< std::string >( 0 );

    if( CanSetItem( lKey ) )
        SetItem( lFields );
}

bool HvmFtPromisInterpreter::IsComment(const std::string &aLine) const
{
    // the concept of a comment is not specified yet
    ( void ) aLine;

    return false;
}

bool HvmFtPromisInterpreter::IsHeader(const std::string &aLine) const
{
    // the concept of a header is not specified yet
    (void) aLine;

    return false;
}

bool HvmFtPromisInterpreter::CanSetItem(const std::string &lKey) const
{
    return ( ( mPromisItem == NULL ) &&
             ( Qx::StringManipulations::ToLowerCase(mKey) == Qx::StringManipulations::ToLowerCase(lKey) ) );
}

void HvmFtPromisInterpreter::SetItem(const TabularFileLineFields &aFields)
{
    HvmFtPromisItem *lItem = new HvmFtPromisItem();

    lItem->mDateCode = aFields.GetValueAt< std::string >( 1 );
    lItem->mPackageType = aFields.GetValueAt< std::string >( 2 );
    lItem->mEquipmentId = aFields.GetValueAt< std::string >( 3 );
    lItem->mPartNumber = aFields.GetValueAt< std::string >( 4 );
    lItem->mSiteLocation = aFields.GetValueAt< std::string >( 5 );
    lItem->mGeometryName = aFields.GetValueAt< std::string >( 6 );
    lItem->mDiePart = aFields.GetValueAt< std::string >( 7 );
    lItem->mDivision = aFields.GetValueAt< std::string >( 8 );

    mPromisItem = lItem;
}

}
}
