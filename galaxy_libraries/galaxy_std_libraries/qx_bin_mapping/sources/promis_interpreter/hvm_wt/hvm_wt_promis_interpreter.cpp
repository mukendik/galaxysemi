#include "hvm_wt_promis_interpreter.h"
#include "promis_exceptions.h"
#include "hvm_wt_promis_item.h"

#include <fstream>

namespace Qx
{
namespace BinMapping
{

HvmWtPromisInterpreter::HvmWtPromisInterpreter( const std::string &aKey,
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

HvmWtPromisInterpreter::~HvmWtPromisInterpreter() {}

const PromisItemBase &HvmWtPromisInterpreter::GetPromisItem() const
{
    return *mPromisItem;
}

void HvmWtPromisInterpreter::ProcessReadLine(const std::string &aLine)
{
    const TabularFileLineFields &lFields = ExtractNFieldsFromLine< InvalidHvmWtPromisFileFormat >( aLine, 11 );

    const std::string &lKey = lFields.GetValueAt< std::string >( 0 );

    if( CanSetItem( lKey ) )
        SetItem( lFields );
}

bool HvmWtPromisInterpreter::IsComment(const std::string &aLine) const
{
    // the concept of a comment is not specified yet
    ( void ) aLine;

    return false;
}

bool HvmWtPromisInterpreter::IsHeader(const std::string &aLine) const
{
    // the concept of a header is not specified yet
    (void) aLine;

    return false;
}

bool HvmWtPromisInterpreter::CanSetItem(const std::string &lKey) const
{
    return ( ( mPromisItem == NULL ) &&
             ( Qx::StringManipulations::ToLowerCase(mKey) == Qx::StringManipulations::ToLowerCase(lKey) ) );
}

void HvmWtPromisInterpreter::SetItem(const TabularFileLineFields &aFields)
{
    HvmWtPromisItem *lItem = new HvmWtPromisItem();

    lItem->mSublotId = aFields.GetValueAt< std::string >(1);
    lItem->mNbWafers = aFields.GetValueAt< std::string >(2);
    lItem->mFabLocation = aFields.GetValueAt< std::string >(3);
    lItem->mDiePart = aFields.GetValueAt< std::string >(4);
    lItem->mGeometryName = aFields.GetValueAt< std::string >(5);
    lItem->mGrossDiePerWafer = aFields.GetValueAt< std::string >(6);
    lItem->mDieWidth = aFields.GetValueAt< std::string >(7);
    lItem->mDieHeight = aFields.GetValueAt< std::string >(8);
    lItem->mFlatOrientation = aFields.GetValueAt< std::string >(9);
    lItem->mSiteLocation = aFields.GetValueAt< std::string >(10);

    mPromisItem = lItem;
}

}
}
