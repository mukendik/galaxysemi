#ifdef GCORE15334

#include "sitetestresults.h"


namespace GS
{
namespace Gex
{
    PartResult::PartResult(const PartResult& lOther)
    {
        this->mOrgSoftBin=lOther.OrgSoftBin();
        this->mOrgHardBin=lOther.OrgHardBin();
        this->mPatSoftBin=lOther.PatSoftBin();
        this->mPatHardBin=lOther.PatHardBin();
        this->mPartID=lOther.PartID();
        this->mPartIndex=lOther.PartIndex();
    }

    QString PartResult::PartID() const
    {
        QMutexLocker locker(&mMutex);
        return mPartID;
    }

    PartResult& PartResult::operator=(const PartResult& lOther)
    {
        this->mOrgSoftBin=lOther.OrgSoftBin();
        this->mOrgHardBin=lOther.OrgHardBin();
        this->mPatSoftBin=lOther.PatSoftBin();
        this->mPatHardBin=lOther.PatHardBin();
        this->mPartID=lOther.PartID();
        this->mPartIndex=lOther.PartIndex();
        return *this;
    }

    void PartResult::Reset()
    {
        QMutexLocker locker(&mMutex);
        mOrgSoftBin=-1;
        mOrgHardBin=-1;
        mPatSoftBin=-1;
        mPatHardBin=-1;
        mPartID.clear();
        mPartIndex=0;
    }

    void PartResult::Set(PT_GNM_RUNRESULT PartResult)
    {
        QMutexLocker locker(&mMutex);
        mOrgSoftBin=PartResult->mOrgSoftBin;
        mOrgHardBin=PartResult->mOrgHardBin;
        mPatSoftBin=PartResult->mPatSoftBin;
        mPatHardBin=PartResult->mPatHardBin;
        mPartID=PartResult->mPartID;
        mPartIndex=PartResult->mPartIndex;
    }

}
}

#endif
