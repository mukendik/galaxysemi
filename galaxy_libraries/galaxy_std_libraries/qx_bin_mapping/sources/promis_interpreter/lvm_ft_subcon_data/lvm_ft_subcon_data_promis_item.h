#ifndef LVM_FT_SUBCON_DATA_PROMIS_ITEM_H
#define LVM_FT_SUBCON_DATA_PROMIS_ITEM_H

#include "promis_item_base.h"

namespace Qx
{
namespace BinMapping
{

class LvmFtSubconDataPromisItem : public PromisItemBase
{
    friend class LvmFtSubconDataPromisInterpreter;

public:
    std::string GetPartNumber() const { return mPartNumber; }
    std::string GetPackageType() const { return mPackageType; }
    std::string GetGeometryName() const { return mGeometryName; }

private:
    std::string mPartNumber;
    std::string mPackageType;
    std::string mGeometryName;
};

}
}

#endif // LVM_FT_SUBCON_DATA_PROMIS_ITEM_H
