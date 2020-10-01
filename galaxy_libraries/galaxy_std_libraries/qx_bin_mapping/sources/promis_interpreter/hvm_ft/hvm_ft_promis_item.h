#ifndef HVM_FT_PROMIS_ITEM_H
#define HVM_FT_PROMIS_ITEM_H

#include "promis_item_base.h"

namespace Qx
{
namespace BinMapping
{

class HvmFtPromisItem : public PromisItemBase
{
    friend class HvmFtPromisInterpreter;

public:
    std::string GetDateCode() const { return mDateCode; }
    std::string GetPackageType() const { return mPackageType; }
    std::string GetEquipmentID() const { return mEquipmentId; }
    std::string GetPartNumber() const { return mPartNumber; }
    std::string GetSiteLocation() const { return mSiteLocation; }
    std::string GetGeometryName() const { return mGeometryName; }
    std::string GetDiePart() const { return mDiePart; }
    std::string GetDivision() const { return mDivision; }

private:
    std::string mDateCode;
    std::string mPackageType;
    std::string mEquipmentId;
    std::string mPartNumber;
    std::string mSiteLocation;
    std::string mGeometryName;
    std::string mDiePart;
    std::string mDivision;
};

}
}

#endif // HVM_FT_PROMIS_ITEM_H
