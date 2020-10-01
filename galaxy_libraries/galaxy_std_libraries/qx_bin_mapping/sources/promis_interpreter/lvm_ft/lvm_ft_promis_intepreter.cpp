#include "lvm_ft_promis_intepreter.h"
#include "promis_exceptions.h"
#include "lvm_ft_promis_item.h"
#include "bin_mapping_exceptions.h"

#include <fstream>

namespace Qx
{
namespace BinMapping
{

LvmFtPromisInterpreter::LvmFtPromisInterpreter( const std::string &aKey,
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

LvmFtPromisInterpreter::~LvmFtPromisInterpreter() {}

const PromisItemBase &LvmFtPromisInterpreter::GetPromisItem() const
{
    return *mPromisItem;
}

void LvmFtPromisInterpreter::ProcessReadLine(const std::string &aLine)
{
    const TabularFileLineFields &lFields = ExtractRangeOfFieldsFromLine< InvalidLvmFtPromisFileFormat >( aLine, 8, 14 );

    const std::string &lKey = lFields.GetValueAt< std::string >( 0 );

    if( CanSetItem( lKey ) )
        SetItem( lFields );
}

bool LvmFtPromisInterpreter::IsComment(const std::string &aLine) const
{
    // the concept of a comment is not specified yet
    ( void ) aLine;

    return false;
}

bool LvmFtPromisInterpreter::IsHeader(const std::string &aLine) const
{
    // the concept of a header is not specified yet
    (void) aLine;

    return false;
}

bool LvmFtPromisInterpreter::CanSetItem(const std::string &lKey) const
{
    return ( ( mPromisItem == NULL ) &&
             ( Qx::StringManipulations::ToLowerCase(mKey) == Qx::StringManipulations::ToLowerCase(lKey) ) );
}

void LvmFtPromisInterpreter::SetItem(const TabularFileLineFields &aFields)
{
    LvmFtPromisItem *lItem = new LvmFtPromisItem();

    ExtractMandatoryFieldsInto( lItem, aFields );
    ExtractOptionalFieldsInto( lItem, aFields );

    mPromisItem = lItem;
}

void LvmFtPromisInterpreter::ExtractMandatoryFieldsInto(LvmFtPromisItem *aItem, const TabularFileLineFields &aFields) const
{
    try{
        aItem->mDateCode = aFields.GetValueAt< std::string >( 1 );
        aItem->mPackage = aFields.GetValueAt< std::string >( 2 );
        aItem->mEquipmentId = aFields.GetValueAt< std::string >( 3 );
        aItem->mProductId = aFields.GetValueAt< std::string >( 4 );
        aItem->mSiteId = aFields.GetValueAt< std::string >( 5 );
        aItem->mGeometryName = aFields.GetValueAt< std::string >( 6 );
        aItem->mPackageType = aFields.GetValueAt< std::string >( 7 );
    }
    catch( const FieldIndexOutOfRange &)
    {
        throw InvalidLvmFtFieldNbPromisFileFormat( mPromisFilePath.c_str(), mConverterExternalFilePath.c_str(), 8);
    }
}

void LvmFtPromisInterpreter::ExtractOptionalFieldsInto(LvmFtPromisItem *aItem, const TabularFileLineFields &aFields) const
{
    try
    {
        aItem->mLotIdP2 = aFields.GetValueAt< std::string >( 8 );
        aItem->mGeometryNameP2 = aFields.GetValueAt< std::string >( 9 );
        aItem->mLotIdP3 = aFields.GetValueAt< std::string >( 10 );
        aItem->mGeometryNameP3 = aFields.GetValueAt< std::string >( 11 );
        aItem->mLotIdP4 = aFields.GetValueAt< std::string >( 12 );
        aItem->mGeometryNameP4 = aFields.GetValueAt< std::string >( 13 );
    }
    catch( const FieldIndexOutOfRange & )
    {
        // simply ignore this kind of exception as a line may contain between 8 and 14 fields
    }
    catch ( ... )
    {
        throw;
    }
}

}

}
