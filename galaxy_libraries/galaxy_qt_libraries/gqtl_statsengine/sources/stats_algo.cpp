#include "stats_algo_private.h"
#include "stats_algo.h"

namespace GS
{
namespace SE
{

StatsAlgoPrivate::StatsAlgoPrivate()
    :mResults(0)
{
}

StatsAlgoPrivate::~StatsAlgoPrivate()
{
    if (mResults)
    {
        delete mResults;
        mResults = 0;
    }
}

StatsAlgo::StatsAlgo(StatsAlgoPrivate & lPrivateData)
    :mPrivate(&lPrivateData)
{
}

StatsAlgo::~StatsAlgo()
{
    if (mPrivate)
    {
        delete mPrivate;
        mPrivate = 0;
    }
}

void StatsAlgo::Init()
{
    InitRequiredData();
    InitRequiredParameters();
}

bool StatsAlgo::CheckInputParameters(QMap<Parameter, QVariant> params)
{
    for (int i = 0; i < mPrivate->mRequiredParameter.size(); ++i)
    {
        if (!params.contains(mPrivate->mRequiredParameter.at(i)))
        {
            mPrivate->mErrors.append(QString("Missing paramater %1").arg(mPrivate->mRequiredParameter.at(i)));
            return false;
        }
    }

    return true;
}

bool StatsAlgo::CheckInputData(StatsData *data)
{
    if (!data)
        return false;

    return true;
}

bool StatsAlgo::Execute(QMap<Parameter, QVariant> params, StatsData *data)
{
    if (!CheckInputParameters(params))
        return false;

    if (!CheckInputData(data))
        return false;

    return DoExecute(params, data);
}

QString StatsAlgo::GetLastError()
{
    QString lError;
    if (!mPrivate->mErrors.isEmpty())
        lError = mPrivate->mErrors.last();

    return lError;
}

QStringList StatsAlgo::GetErrors()
{
    return mPrivate->mErrors;
}

void StatsAlgo::ClearErrors()
{
    mPrivate->mErrors.clear();
}

} // namespace SE
} // namespace GS
