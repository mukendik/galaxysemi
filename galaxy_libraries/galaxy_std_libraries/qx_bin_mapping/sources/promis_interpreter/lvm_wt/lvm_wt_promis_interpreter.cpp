#include "lvm_wt_promis_interpreter.h"
#include "promis_exceptions.h"
#include "lvm_wt_promis_item.h"

#include <fstream>

namespace Qx
{
namespace BinMapping
{

LvmWtPromisInterpreter::LvmWtPromisInterpreter( const std::string &aKey,
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

LvmWtPromisInterpreter::~LvmWtPromisInterpreter() {}

const PromisItemBase &LvmWtPromisInterpreter::GetPromisItem() const
{
    return *mPromisItem;
}

void LvmWtPromisInterpreter::ProcessReadLine(const std::string &aLine)
{
    try
    {
        const TabularFileLineFields &lFields = ExtractNFieldsFromLine< InvalidLvmWtPromisFileFormat >( aLine, 12 );

        const std::string &lKey = lFields.GetValueAt< std::string >( 0 );

        if( CanSetItem( lKey ) )
            SetItem( lFields );
    }
    catch( const InvalidLvmWtPromisFileFormat & )
    {
        // simply ignore this kind of exception as any line containing more or less than &é fields must be ignored
    }
}

bool LvmWtPromisInterpreter::IsComment(const std::string &aLine) const
{
    // the concept of a comment is not specified yet
    ( void ) aLine;

    return false;
}

bool LvmWtPromisInterpreter::IsHeader(const std::string &aLine) const
{
    // the concept of a header is not specified yet
    (void) aLine;

    return false;
}

bool LvmWtPromisInterpreter::CanSetItem(const std::string &lKey) const
{
    return ( ( mPromisItem == NULL ) &&
             ( Qx::StringManipulations::ToLowerCase(mKey) == Qx::StringManipulations::ToLowerCase(lKey) ) );
}

void LvmWtPromisInterpreter::SetItem(const TabularFileLineFields &aFields)
{
    LvmWtPromisItem *lItem = new LvmWtPromisItem();

    lItem->mSublotId = aFields.GetValueAt< std::string >(1);
    lItem->mNbWafers = aFields.GetValueAt< std::string >(2);
    lItem->mFabSite = aFields.GetValueAt< std::string >(3);
    lItem->mEquipmentID = aFields.GetValueAt< std::string >(4);
    lItem->mPartID = aFields.GetValueAt< std::string >(5);
    lItem->mGeometryName = aFields.GetValueAt< std::string >(6);
    lItem->mGrossDiePerWafer = aFields.GetValueAt< std::string >(7);
    lItem->mDieWidth = aFields.GetValueAt< std::string >(8);
    lItem->mDieHeight = aFields.GetValueAt< std::string >(9);
    lItem->mFlatOrientation = aFields.GetValueAt< std::string >(10);
    lItem->mTestSite = aFields.GetValueAt< std::string >(11);

    mPromisItem = lItem;
}

}
}
