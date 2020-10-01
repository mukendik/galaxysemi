#ifndef BIN_MAP_ITEM_BASE_H
#define BIN_MAP_ITEM_BASE_H

#include "bin_mapping_api.h"

#include <string>

namespace Qx
{
namespace BinMapping
{

struct QX_BIN_MAPPING_API_DECL CannotAccessUnhandledField {};
struct QX_BIN_MAPPING_API_DECL CannotUsedUnhandledMethod {};

struct QX_BIN_MAPPING_API_DECL BinMapItemBase
{
    virtual ~BinMapItemBase();

    virtual std::string GetBinName() const { throw CannotAccessUnhandledField(); }
    virtual std::string GetBinNameAlias() const { throw CannotAccessUnhandledField(); }
    virtual std::string GetTestName() const { throw CannotAccessUnhandledField(); }
    virtual char GetBinCategory() const { throw CannotAccessUnhandledField(); }
    virtual int GetTestNumber() const { throw CannotAccessUnhandledField(); }
    virtual int GetTestNumberAlias() const { throw CannotAccessUnhandledField(); }
    virtual int GetSoftBinNumber() const { throw CannotAccessUnhandledField(); }
    virtual int GetHardBinNumber() const { throw CannotAccessUnhandledField(); }
    virtual int GetBinNumber() const { throw CannotAccessUnhandledField(); }
    virtual bool IsPass() const {throw CannotAccessUnhandledField();}
    virtual bool GetEnabled() const { throw CannotAccessUnhandledField(); }

protected :
    BinMapItemBase() {}
    BinMapItemBase( const BinMapItemBase & );
    BinMapItemBase & operator =( const BinMapItemBase & );
};

}
}

#endif // BIN_MAP_ITEM_BASE_H
