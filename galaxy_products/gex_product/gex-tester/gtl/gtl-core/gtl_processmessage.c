#include <stdlib.h>
#include <stdio.h>
#include <gtc_netmessage.h>

#include "gtl_socket.h"
#include "gtl_core.h"
#include "gtl_message.h"
#include "gtl_testlist.h"
#include "gtl_constants.h"

extern int gtl_push_back_new_message(int severity, char* message, int message_id);
extern int gtl_IsInitialized();
extern void	gtl_Restart_Baseline();
extern GTL_ProdInfo	gtl_stProdInfo;	// Tester Production information
extern GTL_GlobalInfo gtl_stGlobalInfo;	// GTL Global information

/*======================================================================================*/
/* Description  : process GNM_TYPE_NOTIFY message received from GTM.					*/
/* Argument(s)  :                                                                       */
/* Return Codes : GTL_CORE_ERR_OKAY.													*/
int gtl_ProcessMessage_Notify(PT_GNM_NOTIFY pMsg_NOTIFY)
{
    //7103
    //tester_Printf(pMsg_NOTIFY->szMessage, pMsg_NOTIFY->nTimeout);
    int r=gtl_push_back_new_message(pMsg_NOTIFY->nTimeout, pMsg_NOTIFY->szMessage, -1);
    if (r<0)
        GTL_LOG(3, "gtl_create_new_message failed : %d : %s", r, pMsg_NOTIFY->szMessage);

    if (pMsg_NOTIFY->szMessage)
        GTL_LOG(5, "Notify message: %s", pMsg_NOTIFY->szMessage, 0);

    /* Free stuff */
    gtcnm_FreeStruct(GNM_TYPE_NOTIFY, pMsg_NOTIFY);
    free(pMsg_NOTIFY);

    return r; //GTL_CORE_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : process GNM_TYPE_PATCONFIG_DYNAMIC message received from GTM.			*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes : GTL_CORE_ERR_OKAY.													*/
/*======================================================================================*/
int gtl_ProcessMessage_DynamicPatConfig(PT_GNM_PATCONFIG_DYNAMIC  pMsg_PATCONFIG_DYNAMIC)
{
    if(!gtl_IsInitialized())
    {
        GTL_LOG(4, "DPAT config received but GTL not init (state=%d)", gtl_stGlobalInfo.nCurrentLibState);
        return GTL_CORE_ERR_OKAY;
    }

    // 7501
    if (gtl_stProdInfo.mRetestIndex>0)
    {
        GTL_LOG(5, "Ignoring new DPAT config because of retest (retest index=%d)", gtl_stProdInfo.mRetestIndex);
    }
    else
    {
        // Sites receiving DPAT limits will switch to DPAT mode
        // todo : clean this function to return OK or any errro message
        gtl_tl_UpdateWithDynamicPat(pMsg_PATCONFIG_DYNAMIC);
    }

    /* Free stuff */
    gtcnm_FreeStruct(GNM_TYPE_PATCONFIG_DYNAMIC, pMsg_PATCONFIG_DYNAMIC);
    free(pMsg_PATCONFIG_DYNAMIC);

    return GTL_CORE_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : process GNM_TYPE_COMMAND message received from GTM.					*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes : GTL_CORE_ERR_OKAY.													*/
/*======================================================================================*/
int gtl_ProcessMessage_Command(PT_GNM_COMMAND pMsg_COMMAND)
{
    int lSiteNb=0, lFields=0;

    if(!gtl_IsInitialized())
        return GTL_CORE_ERR_OKAY;

    int r=GTL_CORE_ERR_OKAY;

    /* Check command ID */
    switch(pMsg_COMMAND->uiCommandID)
    {
        case GTC_GTLCOMMAND_RESTARTBASELINE:
            /* case 7260: extract site# of site for which BL is restarted */
            lFields=sscanf(pMsg_COMMAND->szCommand, "--site=%d", &lSiteNb);
            gtl_Restart_Baseline(lSiteNb);
            break;
#if 0 // Deprecated for now since GTL V3.6
        case GTC_GTLCOMMAND_RESET:
            r=gtl_reset();
            break;
#endif
        case GTC_GTLCOMMAND_ASCII:
            break;
    }

    /* Free stuff */
    gtcnm_FreeStruct(GNM_TYPE_COMMAND, pMsg_COMMAND);
    free(pMsg_COMMAND);

    return r;
}

/*======================================================================================*/
/* Description  : process GNM_TYPE_WRITETOSTDF message received from GTM.				*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes : GTL_CORE_ERR_OKAY.													*/
/*======================================================================================*/
int gtl_ProcessMessage_WriteToStdf(PT_GNM_WRITETOSTDF pMsg_WRITETOSTDF)
{
    int	nRecTyp = pMsg_WRITETOSTDF->nRecTyp;
    int	nRecSub = pMsg_WRITETOSTDF->nRecSub;
    char szMessage[GTC_MAX_STRINGTOSTDF]="";

    if(!gtl_IsInitialized())
        return GTL_CORE_ERR_OKAY;

    /* Check which record to use to write to STDF file */
    if((nRecTyp == 50) && (nRecSub == 10))
    {
        /* Write to GDR record */
        strcpy(szMessage, "String written to STDF file (GDR):\n");
        strcat(szMessage, pMsg_WRITETOSTDF->szString);
        //tester_Printf(szMessage, 0);
    }

    if((nRecTyp == 50) && (nRecSub == 30))
    {
        /* Write to DTR record */
        strcpy(szMessage, "String written to STDF file (DTR):\n");
        strcat(szMessage, pMsg_WRITETOSTDF->szString);
        //tester_Printf(szMessage, 0);
    }

    if((nRecTyp == 50) && (nRecSub == 10))
    {
        /* Write to custom record */
        sprintf(szMessage, "String written to STDF file (%d:%d):\n", pMsg_WRITETOSTDF->nRecTyp, pMsg_WRITETOSTDF->nRecSub);
        strcat(szMessage, pMsg_WRITETOSTDF->szString);
        //tester_Printf(szMessage, 0);
    }

    /* Free stuff */
    gtcnm_FreeStruct(GNM_TYPE_WRITETOSTDF, pMsg_WRITETOSTDF);
    free(pMsg_WRITETOSTDF);

    return GTL_CORE_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : process message received from GTM.									*/
/* Argument(s)  :                                                                       */
/* Return Codes : GTL_CORE_ERR_OKAY.													*/
int gtl_ProcessMessage(GTL_SOCK_MSG *pstMessage)
{
    /* Process received message */
    switch(pstMessage->mType)
    {
        case GNM_TYPE_ACK:
            /* Nothing to process for an Acknowledge */
            break;
        case GNM_TYPE_NOTIFY:
            gtl_ProcessMessage_Notify((PT_GNM_NOTIFY)(pstMessage->mMsgStruct));
            break;
        case GNM_TYPE_PATCONFIG_DYNAMIC:
            gtl_ProcessMessage_DynamicPatConfig((PT_GNM_PATCONFIG_DYNAMIC)(pstMessage->mMsgStruct));
            break;
        case GNM_TYPE_COMMAND:
            gtl_ProcessMessage_Command((PT_GNM_COMMAND)(pstMessage->mMsgStruct));
            break;

        case GNM_TYPE_WRITETOSTDF:
            gtl_ProcessMessage_WriteToStdf((PT_GNM_WRITETOSTDF)(pstMessage->mMsgStruct));
            break;
        default:
            GTL_LOG(3, "Unknown message type %d", pstMessage->mType);
            return GTL_CORE_ERR_UNKNOWN_MESSAGE_TYPE;
            break;
    }

    return GTL_CORE_ERR_OKAY;
}
