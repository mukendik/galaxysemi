#ifndef VALIDITY_CHECK_FOR_BIN_H
#define VALIDITY_CHECK_FOR_BIN_H
#include "bin_mapping_api.h"

namespace Qx
{
namespace BinMapping
{

struct BinMapItemBase;

struct QX_BIN_MAPPING_API_DECL ValidatableByBin
{
    virtual ~ValidatableByBin(){}

    virtual bool IsBinCorrect( int aNumBin, const std::string& aBinName) const = 0;
    virtual std::string RetrieveBinName(int aNumBin) const = 0;

};

}
}
#endif // VALIDITY_CHECK_FOR_BIN_H
