#ifndef LVM_WS_BIN_MAP_ITEM_H
#define LVM_WS_BIN_MAP_ITEM_H

#include "bin_map_item_base.h"

#include <string>

namespace Qx
{
namespace BinMapping
{

class LvmWsBinMapItem : public BinMapItemBase
{
    friend class LvmWsBinMapStore;

public :
    std::string GetBinName() const { return mBinningName; }
    std::string GetTestName() const { return mTestName; }
    int GetTestNumber() const { return mTestNumber; }
    int GetTestNumberAlias() const { return mTestNumberAlias; }
    int GetSoftBinNumber() const { return mSoftBinNumber; }

private :
    std::string mBinningName;
    std::string mTestName;
    int mTestNumber;
    int mSoftBinNumber;
    int mTestNumberAlias;
};

}
}

#endif // LVM_WS_BIN_MAPPING_ITEM_H
