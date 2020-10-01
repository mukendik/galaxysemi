#ifndef STATS_ENGINE_H
#define STATS_ENGINE_H

/*! \class StatsEngine
 * \brief
 *
 */

#include <QObject>
#include <QMutex>

#include "stats_algo.h"

namespace GS
{
namespace SE
{

class StatsEnginePrivate;

class StatsEngine : public QObject
{
    Q_OBJECT
public:
    /// \brief Destructor
    ~StatsEngine();
    /// \brief return instance of stats engine
    static StatsEngine *GetInstance(const QString &appDir, QString &error);
    /// \brief release instance of stats engine
    static void ReleaseInstance();
    /// \brief return pointer to selected algo if instanciated
    StatsAlgo*  GetAlgorithm(const StatsAlgo::Algorithm& algo);
    /// \brief return last error
    QString     GetLastError();
    /// \brief Destroy StatsEngine object
    static void Destroy();

private:
    Q_DECLARE_PRIVATE_D(mPrivate, StatsEngine)
    /// \brief Constructor
    StatsEngine(const QString &appDir);
    /// \brief init
    bool        Init();
    /// \brief load algorithms
    bool        LoadAlgorithms();
    /// \brief  load R algorithms
    bool        LoadRAlgorithms();

    StatsEnginePrivate*         mPrivate;   ///< ptr to private members
    static StatsEngine *        mInstance;  ///< holds instance of stats engine
    static QMutex               mMutex;     ///< holds mutex on stats engine
};

} // namespace SE
} // namespace GS

#endif // STATS_ENGINE_H
