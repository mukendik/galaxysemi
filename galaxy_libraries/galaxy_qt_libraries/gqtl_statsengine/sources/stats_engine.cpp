#include <QMap>
#include <QStringList>
#include <QCoreApplication>

#include "stats_engine.h"
#include "mv_groups_builder.h"
#include "mv_outliers_finder.h"
#include "shape_identifier.h"
#include "r_engine.h"

#include <cstdio>

namespace GS
{
namespace SE
{

StatsEngine* StatsEngine::mInstance = NULL;
QMutex StatsEngine::mMutex;

class StatsEnginePrivate
{
public:
    StatsEnginePrivate(const QString& appDir)
        :mREngine(0), mAppDir(appDir), mEngineInitialized(false)
    {

    }
    ~StatsEnginePrivate()
    {
        // Clean loaded algorithm
        while(!mLoadedAlgo.isEmpty())
            delete mLoadedAlgo.take(mLoadedAlgo.begin().key());

        if (mREngine)
        {
            delete mREngine;
            mREngine = 0;
        }
    }

    REngine*                                mREngine;           ///< holds R engine
    QStringList                             mErrors;            ///< holds errors
    QString                                 mAppDir;            ///< holds apps dir
    bool                                    mEngineInitialized; ///< true if engine is initialized
    QMap<StatsAlgo::Algorithm, StatsAlgo*>  mLoadedAlgo;        ///< holds loaded algo
};

StatsEngine::StatsEngine(const QString& appDir)
    :mPrivate(new StatsEnginePrivate(appDir))
{
}

StatsEngine::~StatsEngine()
{
    if (mPrivate)
    {
        delete mPrivate;
        mPrivate = 0;
    }
}

StatsEngine* StatsEngine::GetInstance(const QString& appDir, QString& error)
{
    mMutex.lock();
    if (mInstance)
        return mInstance;

    mInstance = new StatsEngine(appDir);
    bool lSuccess = mInstance->Init();

    if (!lSuccess)
    {
        error = mInstance->GetLastError();
        mMutex.unlock();
        return 0;
    }

    return mInstance;
}

void StatsEngine::ReleaseInstance()
{
    mMutex.unlock();
}

bool StatsEngine::Init()
{
    // instanciate/init R engine
    mPrivate->mREngine = new REngine();
    if (!mPrivate->mREngine->Init(mPrivate->mAppDir))
    {
        mPrivate->mErrors.append(mPrivate->mREngine->GetLastError());
        return false;
    }
    // Load Algos
    if (!LoadAlgorithms())
    {
        mPrivate->mErrors.append(mPrivate->mREngine->GetLastError());
        return false;
    }

    // everything went well!
    mPrivate->mEngineInitialized = true;

    return true;
}

bool StatsEngine::LoadAlgorithms()
{
    // Load R algos
    bool lSuccess = LoadRAlgorithms();

    return lSuccess;
}

bool StatsEngine::LoadRAlgorithms()
{
    // Load MV Groups Builder
    MVGroupsBuilder* lGroupsBuilder = new MVGroupsBuilder();
    if (!mPrivate->mREngine->SourceScripts(lGroupsBuilder->GetScripts()))
    {
        mPrivate->mErrors.append(mPrivate->mREngine->GetLastError());
        return false;
    }
    // keep track of loaded algo
    mPrivate->mLoadedAlgo.insert(StatsAlgo::MV_GROUPS_BUILDER, lGroupsBuilder);

    // Load MV Outliers Finder
    MVOutliersFinder* lOutliersFinder = new MVOutliersFinder();
    if(!mPrivate->mREngine->SourceScripts(lOutliersFinder->GetScripts()))
    {
        mPrivate->mErrors.append(mPrivate->mREngine->GetLastError());
        return false;
    }
    // keep track of loaded algo
    mPrivate->mLoadedAlgo.insert(StatsAlgo::MV_OUTLIERS_FINDER, lOutliersFinder);

    // Load Shape identifier
    ShapeIdentifier* lShapeIdentifier = new ShapeIdentifier();
    if(!mPrivate->mREngine->SourceScripts(lShapeIdentifier->GetScripts()))
    {
        mPrivate->mErrors.append(mPrivate->mREngine->GetLastError());
        return false;
    }
    // keep track of loaded algo
    mPrivate->mLoadedAlgo.insert(StatsAlgo::SHAPE_IDENTIFIER, lShapeIdentifier);

    return true;
}

StatsAlgo *StatsEngine::GetAlgorithm(const StatsAlgo::Algorithm &algo)
{
    // Engine not initialized...
    if (!mPrivate->mEngineInitialized)
    {
        mPrivate->mErrors.append("StatsEngine not initialized");
        return 0;
    }
    // Search algo
    if (!mPrivate->mLoadedAlgo.contains(algo))
    {
        mPrivate->mErrors.append("Unable to find algorithm");
        return 0;
    }

    return mPrivate->mLoadedAlgo.value(algo);
}

QString StatsEngine::GetLastError()
{
    return mPrivate->mErrors.last();
}

void StatsEngine::Destroy()
{
    if (mInstance)
    {
        delete mInstance;
        mInstance = 0;
    }
}

} // namespace SE
} // namespace GS

