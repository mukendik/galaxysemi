#ifndef PAT_ENGINE_PRIVATE_H
#define PAT_ENGINE_PRIVATE_H

#include <QString>
#include "pat_info.h"

namespace GS
{
namespace Gex
{

class PATEnginePrivate
{

public:

    PATEnginePrivate();
    ~PATEnginePrivate();

    QString         mErrorMessage;
    CPatInfo *      mContext;
};

}
}

#endif // PAT_ENGINE_PRIVATE_H
