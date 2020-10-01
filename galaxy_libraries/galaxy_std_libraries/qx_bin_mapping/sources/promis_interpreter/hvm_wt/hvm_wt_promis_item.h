#ifndef HVM_WT_PROMIS_ITEM_H
#define HVM_WT_PROMIS_ITEM_H

#include "promis_item_base.h"

namespace Qx
{
namespace BinMapping
{

class HvmWtPromisItem : public PromisItemBase
{
    friend class HvmWtPromisInterpreter;

public:
    std::string GetSublotID() const { return mSublotId; }
    std::string GetNbWafers() const { return mNbWafers; }
    std::string GetFabLocation() const { return mFabLocation; }
    std::string GetDiePart() const { return mDiePart; }
    std::string GetGeometryName() const { return mGeometryName; }
    std::string GetGrossDiePerWafer() const { return mGrossDiePerWafer; }
    std::string GetDieWidth() const { return mDieWidth; }
    std::string GetDieHeight() const { return mDieHeight; }
    std::string GetFlatOrientation() const { return mFlatOrientation; }
    std::string GetSiteLocation() const { return mSiteLocation; }

private:
    std::string mSublotId;
    std::string mNbWafers;
    std::string mFabLocation;
    std::string mDiePart;
    std::string mGeometryName;
    std::string mGrossDiePerWafer;
    std::string mDieWidth;
    std::string mDieHeight;
    std::string mFlatOrientation;
    std::string mSiteLocation;
};

}
}

#endif // HVM_WT_PROMIS_ITEM_H
