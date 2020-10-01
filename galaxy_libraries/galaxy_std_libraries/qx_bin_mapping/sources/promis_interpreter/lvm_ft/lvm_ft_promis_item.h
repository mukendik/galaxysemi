#ifndef LVM_FT_PROMIS_ITEM_H
#define LVM_FT_PROMIS_ITEM_H

#include "promis_item_base.h"

namespace Qx
{
namespace BinMapping
{

class LvmFtPromisItem : public PromisItemBase
{
    friend class LvmFtPromisInterpreter;

public:
    std::string GetDateCode() const { return mDateCode; }
    std::string GetPackage() const { return  mPackage; }
    std::string GetEquipmentID() const { return mEquipmentId; }
    std::string GetProductId() const { return  mProductId; }
    std::string GetSiteId() const { return mSiteId; }
    std::string GetGeometryName() const { return mGeometryName; }
    std::string GetPackageType() const { return mPackageType; }
    std::string GetLotIdP2() const { return  mLotIdP2; }
    std::string GetGeometryNameP2() const { return mGeometryNameP2; }
    std::string GetLotIdP3() const { return mLotIdP3; }
    std::string GetGeometryNameP3() const { return mGeometryNameP3; }
    std::string GetLotIdP4() const { return mLotIdP4; }
    std::string GetGeometryNameP4() const { return mGeometryNameP4; }

private:
    std::string mDateCode;
    std::string mPackage;
    std::string mEquipmentId;
    std::string mProductId;
    std::string mSiteId;
    std::string mGeometryName;
    std::string mPackageType;
    std::string mLotIdP2;
    std::string mGeometryNameP2;
    std::string mLotIdP3;
    std::string mGeometryNameP3;
    std::string mLotIdP4;
    std::string mGeometryNameP4;
};

}
}

#endif // LVM_FT_PROMIS_ITEM_H
