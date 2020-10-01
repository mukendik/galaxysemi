#include "pat_sinf_info.h"

namespace GS
{
namespace Gex
{
namespace PAT
{

SINFInfo::SINFInfo()
{
    Clear();
}

SINFInfo::~SINFInfo()
{
}

SINFInfo& SINFInfo::operator=(const SINFInfo& lOther)
{
    if (this != &lOther)
    {
        mColRdc=lOther.mColRdc;
        mFlatOrientation=lOther.mFlatOrientation;
        mRefPX=lOther.mRefPX;
        mRefPY=lOther.mRefPY;
        mRowRdc=lOther.mRowRdc;
        mWaferAndPaddingCols=lOther.mWaferAndPaddingCols;
        mWaferAndPaddingRows=lOther.mWaferAndPaddingRows;
        mDieSizeX=lOther.mDieSizeX;
        mDieSizeY=lOther.mDieSizeY;
        mBCEQ=lOther.mBCEQ;
        mDeviceName=lOther.mDeviceName;
        mINKONLYBCBC=lOther.mINKONLYBCBC;
        mLot=lOther.mLot;
        mNOTOUCHBC=lOther.mNOTOUCHBC;
        mSKIPBC=lOther.mSKIPBC;
        mWaferID=lOther.mWaferID;
        mNewWafermap=lOther.mNewWafermap;
    }

    return *this;
}

void SINFInfo::Clear()
{
    mBCEQ.clear();
    mDeviceName.clear();
    mINKONLYBCBC.clear();
    mLot.clear();
    mNOTOUCHBC.clear();
    mSKIPBC.clear();
    mWaferID.clear();
    mNewWafermap.clear();

    mColRdc                 = 0;
    mFlatOrientation        = -1;
    mRefPX                  = -32768;
    mRefPY                  = -32768;
    mRowRdc                 = 0;
    mWaferAndPaddingCols    = 0;
    mWaferAndPaddingRows    = 0;
    mDieSizeX               = -1;
    mDieSizeY               = -1;
}

}   // namespace PAT
}   // namespace Gex
}   // namespace GS
