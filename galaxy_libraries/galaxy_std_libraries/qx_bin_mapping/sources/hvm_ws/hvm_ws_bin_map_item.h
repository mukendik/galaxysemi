#ifndef HVM_WS_BIN_MAP_ITEM_H
#define HVM_WS_BIN_MAP_ITEM_H

#include "bin_map_item_base.h"

#include <string>

namespace Qx
{
namespace BinMapping
{

class HvmWsBinMapItem : public BinMapItemBase
{
    friend class HvmWsBinMapStore;

public :
    std::string GetBinName() const { return mBinningName; }
    std::string GetTestName() const { return mTestName; }
    int GetTestNumber() const { return mTestNumber; }
    int GetTestNumberAlias() const { return mTestNumberAlias; }
    int GetBinNumber() const { return mBinningNumber; }
    int GetSoftBinNumber() const { return mBinningNumber;  }
    int GetHardBinNumber() const { return mBinningNumber; }
    bool IsPass() const {return mPass;}

private :
    std::string mBinningName;
    std::string mTestName;
    int mTestNumber;
    int mBinningNumber;
    int mTestNumberAlias;
    bool mPass;
};

}
}

#endif // HVM_WS_BIN_MAP_ITEM_H
