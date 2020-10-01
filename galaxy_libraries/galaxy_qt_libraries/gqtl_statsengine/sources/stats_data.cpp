#include "stats_data_private.h"
#include "stats_data.h"

namespace GS
{
namespace SE
{

StatsDataPrivate::StatsDataPrivate()
{

}

StatsDataPrivate::~StatsDataPrivate()
{

}

StatsData::StatsData(StatsDataPrivate &lPrivateData)
    :mPrivate(&lPrivateData)
{

}

StatsData::~StatsData()
{
    if (mPrivate)
    {
        delete mPrivate;
        mPrivate = 0;
    }
}

QString StatsData::GetLastError() const
{
    QString lError;
    if (!mPrivate->mErrors.isEmpty())
        lError = mPrivate->mErrors.last();

    return lError;
}

QStringList StatsData::GetErrors() const
{
    return mPrivate->mErrors;
}

void StatsData::ClearErrors()
{
    mPrivate->mErrors.clear();
}


} // namespace SE
} // namespace GS
