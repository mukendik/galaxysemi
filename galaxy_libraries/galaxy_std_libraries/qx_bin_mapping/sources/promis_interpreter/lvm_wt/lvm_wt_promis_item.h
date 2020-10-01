#ifndef LVM_WT_PROMIS_ITEM_H
#define LVM_WT_PROMIS_ITEM_H

#include "promis_item_base.h"

namespace Qx
{
namespace BinMapping
{

class LvmWtPromisItem : public PromisItemBase
{
    friend class LvmWtPromisInterpreter;

public:
    std::string GetSublotID() const { return mSublotId; }
    std::string GetNbWafers() const { return mNbWafers; }
    std::string GetFabSite() const { return mFabSite; }
    std::string GetEquipmentID() const { return mEquipmentID; }
    std::string GetPartID() const { return mPartID; }
    std::string GetGeometryName() const { return mGeometryName; }
    std::string GetGrossDiePerWafer() const { return mGrossDiePerWafer; }
    std::string GetDieWidth() const { return mDieWidth; }
    std::string GetDieHeight() const { return mDieHeight; }
    std::string GetFlatOrientation() const { return mFlatOrientation; }
    std::string GetTestSite() const { return mTestSite; }

private:
    std::string mSublotId;
    std::string mNbWafers;
    std::string mFabSite;
    std::string mEquipmentID;
    std::string mPartID;
    std::string mGeometryName;
    std::string mGrossDiePerWafer;
    std::string mDieWidth;
    std::string mDieHeight;
    std::string mFlatOrientation;
    std::string mTestSite;
};

}
}

#endif // LVM_WT_PROMIS_ITEM_H
