#ifdef GCORE15334
#ifndef PAT_PROCESS_PRIVATE_H
#define PAT_PROCESS_PRIVATE_H

#include "gex_pat_processing.h"
#include "pat_external_map_details.h"
#include "pat_plugins.h"
#include "pat_sinf_info.h"

namespace GS
{
namespace Gex
{

class PATProcessWSPrivate
{
public:

    PATProcessWSPrivate(CPatInfo* context):mContext(context)  {}
    ~PATProcessWSPrivate() {}

    QString                         mErrorMessage;
    QDateTime                       mStartTime;
    PAT::ExternalMapDetails         mExternalMapDetails;
    PAT::SINFInfo                   mSINFInfo;
    PATProcessing                   mSettings;
    GexExternalPat                  mExternalPAT;
    QMap<QString, WaferCoordinate>  mInclusionDies;
    CPatInfo*                       mContext;
};

}   // namespace Gex
}   // namespace GS

#endif // PAT_PROCESS_PRIVATE_H
#endif
