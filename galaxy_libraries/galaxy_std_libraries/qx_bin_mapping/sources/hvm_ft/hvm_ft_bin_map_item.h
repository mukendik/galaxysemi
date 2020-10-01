#ifndef HVM_FT_BIN_MAP_ITEM_H
#define HVM_FT_BIN_MAP_ITEM_H

#include "bin_map_item_base.h"

namespace Qx
{
namespace BinMapping
{

class HvmFtBinMapItem : public BinMapItemBase
{
    friend class HvmFtBinMapStore;

public :
    std::string GetBinName() const { return mBinName; }
    std::string GetBinNameAlias() const { return mBinNameAlias; }
    char GetBinCategory() const { return mBinCategory; }
    int GetSoftBinNumber() const { return mSoftBinNumber; }
    int GetHardBinNumber() const { return mHardBinNumber; }

private :
    std::string mBinName;
    std::string mBinNameAlias;
    char mBinCategory;
    int mSoftBinNumber;
    int mHardBinNumber;
};

}
}

#endif // HVM_FT_BIN_MAP_ITEM_H
