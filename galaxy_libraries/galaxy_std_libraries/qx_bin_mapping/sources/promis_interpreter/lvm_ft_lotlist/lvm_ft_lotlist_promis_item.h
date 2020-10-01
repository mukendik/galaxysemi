#ifndef LVM_FT_LOTLIST_PROMIS_ITEM_H
#define LVM_FT_LOTLIST_PROMIS_ITEM_H

#include "promis_item_base.h"

namespace Qx
{
namespace BinMapping
{

class LvmFtLotlistPromisItem : public PromisItemBase
{
    friend class LvmFtLotlistPromisInterpreter;

public:
    std::string GetDateCode() const { return mDateCode; }
    std::string GetSiteLocation() const { return mSiteLocation; }

private:
    std::string mDateCode;
    std::string mSiteLocation;
};

}
}

#endif // LVM_FT_LOTLIST_PROMIS_ITEM_H
