#ifndef LVM_FT_BIN_MAP_ITEM_H
#define LVM_FT_BIN_MAP_ITEM_H

#include "bin_map_item_base.h"

#include <string>

namespace Qx
{
namespace BinMapping
{

class LvmFtBinMapItem : public BinMapItemBase
{
    friend class LvmFtFinalTestBinMapStore;
    friend class LvmFtSortEntriesBinMapStore;
    friend class LvmFtSortEntriesNewBinMapStore;

public :
    int GetTestNumber() const {return mTestNumber;}
    std::string GetTestName() const {return mTestName;}
    int GetBinNumber() const {return mBinning;}
    int GetSoftBinNumber() const { return mBinning; }
    int GetHardBinNumber() const { return mBinning; }
    std::string GetBinName() const {return mBinName;}
    bool GetEnabled() const { return mEnabled; }

private :
    int mTestNumber;
    std::string mTestName;
    int mBinning;
    std::string mBinName;
    bool mPass;
    int  mCount;
    bool mEnabled;
};



}
}

#endif // LVM_FT_BIN_MAP_ITEM_H
