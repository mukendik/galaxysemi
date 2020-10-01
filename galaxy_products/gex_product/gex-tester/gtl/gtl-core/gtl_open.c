#include "gtl_core.h"
#include "gtl_constants.h"

extern GTL_GlobalInfo gtl_stGlobalInfo; /* GTL Global information */
extern GTL_Station gtl_stStation; /* Tester station information */

int gtl_open()
{
    GTL_LOG(5, "gtl_open: Nbsites=%d", gtl_stStation.uiNbSites);
    return gtl_init(gtl_stGlobalInfo.mConfigFileName, gtl_stGlobalInfo.mRecipeFileName,
             gtl_stStation.uiNbSites, // code me
             gtl_stStation.pnSiteNumbers,
             gtl_stGlobalInfo.mMessages.maxsize*sizeof(GTL_Message));
}
