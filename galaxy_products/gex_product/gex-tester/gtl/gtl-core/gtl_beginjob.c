#include <stdlib.h>
#include <stdio.h>
#include <gtl_core.h>
#include <time.h> // gettimeofday with usec precision
#include <sys/time.h> // should contain gettimeofday(struct timeval ...) with nsec precision !
#include "gtl_constants.h"
#include "gtl_socket.h"
#include "gtl_server.h"
#include "gtl_profile.h"

extern int gtl_nBeginJobExecuted;
extern int gtl_IsInitialized();
extern int gtl_IsDisabled();
extern GTL_GlobalInfo gtl_stGlobalInfo; /* GTL Global information */
extern int gtl_CheckForMessages();
extern int gtl_SetState_Enabled();
extern int gtl_svr_Init(PT_GNM_R_INIT *ppstInit_Reply, char *szFullRecipeFileName);
extern int gtcnm_FreeStruct(unsigned int, void*);

/* Subtract the `struct timeval' values X and Y, storing the result in RESULT.
        Return 1 if the difference is negative, otherwise 0.
*/
 int timeval_subtract(struct timeval *result, struct timeval *x, struct timeval *y)
 {
   /* Perform the carry for the later subtraction by updating y. */
   if (x->tv_usec < y->tv_usec) {
     int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
     y->tv_usec -= 1000000 * nsec;
     y->tv_sec += nsec;
   }
   if (x->tv_usec - y->tv_usec > 1000000) {
     int nsec = (x->tv_usec - y->tv_usec) / 1000000;
     y->tv_usec += 1000000 * nsec;
     y->tv_sec -= nsec;
   }

   /* Compute the time remaining to wait.
      tv_usec is certainly positive. */
   result->tv_sec = x->tv_sec - y->tv_sec;
   result->tv_usec = x->tv_usec - y->tv_usec;

   /* Return 1 if result is negative. */
   return x->tv_sec < y->tv_sec;
 }

int gtl_beginjob()
{
    GTL_PROFILE_START

    //GTL_LOG(7, "gtl_beginjob()...", 0); // too much log

    unsigned int uiIndex=0;

    /* Make sure GTL is initialized */
    if(!gtl_IsInitialized())
    {
        GTL_PROFILE_STOP
        return GTL_CORE_ERR_LIB_NOT_INITIALIZED;
    }

    /* If GTL disabled, return error code */
    if(gtl_IsDisabled())
    {
        GTL_PROFILE_STOP
        return GTL_CORE_ERR_DISABLED;
    }

    gtl_nBeginJobExecuted = 1;

#if 0 // gtl_PerformEndlot() now called directly from gtl_endlot() function
    /* Check if we received an endlot */
    if(gtl_nEndlotReceived)
    {
        GTL_LOG(5, "EndlotReceived",0);
        gtl_PerformEndlot();
        gtl_nEndlotReceived = 0;
    }
#endif

    /* Reset some variables */
    for(uiIndex=0; uiIndex<256; uiIndex++)
        gtl_stGlobalInfo.pnPatBinning[uiIndex] = -1;

    /* Check current GTL state */
    int r = GTL_CORE_ERR_OKAY;
    switch(gtl_stGlobalInfo.nCurrentLibState)
    {
        case GTL_STATE_ENABLED:
            // Check for message from GTM
            r=gtl_CheckForMessages();
            break;
        case GTL_STATE_DISABLED:
            break;
        case GTL_STATE_OFFLINE:
            if (gtl_stGlobalInfo.mReconnectionMode==GTL_RECONNECT_ON)
            {
                PT_GNM_R_INIT pstInit_Reply=0;
                r=gtl_svr_Init(&pstInit_Reply, ""); // when recipe name empty, the function will not reopen it and try to use the one in cache.

                GTL_LOG(5, "gtl_svr_Init returned %d", r);
                if (r==0)
                {
                    GTL_LOG(5, "Reconnection success: reply status=%d. Let s go back to state ENABLED.", pstInit_Reply->nStatus);
                    gtl_SetState_Enabled();
                    // reply is allocated only on success
                    gtcnm_FreeStruct(GNM_TYPE_R_INIT, pstInit_Reply);
                    free(pstInit_Reply);
                }
                else // For the moment lets turn off reconnection. The user can set it on when he d like with gtl_set.
                    gtl_stGlobalInfo.mReconnectionMode=GTL_RECONNECT_OFF;
            }
            break;
        default:
            GTL_LOG(3, "Unknown lib state %d", gtl_stGlobalInfo.nCurrentLibState);
            GTL_PROFILE_STOP
            return GTL_CORE_ERR_INVALID_LIBSTATE;
            break;
    }

    //GTL_LOG(7, "gtl_beginjob() DONE", 0); // this is too much Bernard, logs are unusable if such abuse

    GTL_PROFILE_STOP

    return r;
}

