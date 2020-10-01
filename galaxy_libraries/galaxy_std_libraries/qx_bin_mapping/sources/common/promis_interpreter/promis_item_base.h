#ifndef PROMIS_ITEM_BASE_H
#define PROMIS_ITEM_BASE_H

#include "bin_mapping_api.h"

#include <string>

namespace Qx
{
namespace BinMapping
{

struct QX_BIN_MAPPING_API_DECL PromisItemBase
{
    /// TODO - HTH - Make this exception common with bin mapping one
    struct QX_BIN_MAPPING_API_DECL CannotAccessUnhandledField {};

    virtual ~PromisItemBase();

    virtual std::string GetLotID() const { throw CannotAccessUnhandledField(); }
    virtual std::string GetSublotID() const { throw CannotAccessUnhandledField(); }
    virtual std::string GetPartNumber() const { throw CannotAccessUnhandledField(); }
    virtual std::string GetPackageType() const { throw CannotAccessUnhandledField(); }
    virtual std::string GetPackage() const { throw CannotAccessUnhandledField(); }
    virtual std::string GetGeometryName() const { throw CannotAccessUnhandledField(); }
    virtual std::string GetDateCode() const { throw CannotAccessUnhandledField(); }
    virtual std::string GetSiteLocation() const { throw CannotAccessUnhandledField(); }
    virtual std::string GetNbWafers() const { throw CannotAccessUnhandledField(); }
    virtual std::string GetFabSite() const { throw CannotAccessUnhandledField(); }
    virtual std::string GetEquipmentID() const { throw CannotAccessUnhandledField(); }
    virtual std::string GetPartID() const { throw CannotAccessUnhandledField(); }
    virtual std::string GetGrossDiePerWafer() const { throw CannotAccessUnhandledField(); }
    virtual std::string GetDieWidth() const { throw CannotAccessUnhandledField(); }
    virtual std::string GetDieHeight() const { throw CannotAccessUnhandledField(); }
    virtual std::string GetFlatOrientation() const { throw CannotAccessUnhandledField(); }
    virtual std::string GetTestSite() const { throw CannotAccessUnhandledField(); }
    virtual std::string GetFabLocation() const { throw CannotAccessUnhandledField(); }
    virtual std::string GetDiePart() const { throw CannotAccessUnhandledField(); }
    virtual std::string GetProductId() const { throw CannotAccessUnhandledField(); }
    virtual std::string GetSiteId() const { throw CannotAccessUnhandledField(); }
    virtual std::string GetLotIdP2() const { throw CannotAccessUnhandledField(); }
    virtual std::string GetGeometryNameP2() const { throw CannotAccessUnhandledField(); }
    virtual std::string GetLotIdP3() const { throw CannotAccessUnhandledField(); }
    virtual std::string GetGeometryNameP3() const { throw CannotAccessUnhandledField(); }
    virtual std::string GetLotIdP4() const { throw CannotAccessUnhandledField(); }
    virtual std::string GetGeometryNameP4() const { throw CannotAccessUnhandledField(); }
    virtual std::string GetDivision() const { throw CannotAccessUnhandledField(); }

protected :
    PromisItemBase() {}
    PromisItemBase( const PromisItemBase & );
    PromisItemBase & operator =( const PromisItemBase & );
};

}
}

#endif // PROMIS_ITEM_BASE_H
