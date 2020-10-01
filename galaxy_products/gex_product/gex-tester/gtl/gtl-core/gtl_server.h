/****************************************************************************************/
/* Copyright Galaxy                                                                     */
/* This computer program is protected by copyright law                                  */
/* and international treaties. Unauthorized reproduction or                             */
/* distribution of this program, or any portion of it,may                               */
/* result in severe civil and criminal penalties, and will be                           */
/* prosecuted to the maximum extent possible under the law.                             */
/****************************************************************************************/
/*                                                                                      */
/* Module: gtl_server.h                                                                 */
/*                                                                                      */
/* Functions to communicate with GTM (Galaxy Tester Monitor) server.                    */
/*                                                                                      */
/****************************************************************************************/
/*                                                                                      */
/* Notes:                                                                               */
/*                                                                                      */
/****************************************************************************************/
/*                                                                                      */
/* Revision history:                                                                    */
/*                                                                                      */
/* o [BG] 01 June 2005: Created.                                                        */
/*                                                                                      */
/****************************************************************************************/

#ifndef _GTL_SERVER_H_
#define _GTL_SERVER_H_

/*======================================================================================*/
/* Includes                                                                             */
/*======================================================================================*/
#include <gtc_netmessage.h>

#include "gtl_socket.h"

/*======================================================================================*/
/* Macros                                                                               */
/*======================================================================================*/

/*======================================================================================*/
/* Defines                                                                              */
/*======================================================================================*/

/*======================================================================================*/
/* PUBLIC Functions																		*/
/*======================================================================================*/

/*======================================================================================*/

/*  int	gtl_svr_Init(...)
    Create/connect client socket to communicate with any GTM server of the list.
    Return Codes : 0 if successful, another number GTL_CORE_ERR_... else
*/

#ifdef _GTL_SERVER_MODULE_

    int	gtl_svr_Init(PT_GNM_R_INIT *ppstInit_Reply, char *szFullRecipeFileName);
    int	gtl_svr_SendProdInfo();
    int	gtl_svr_InitTestList(PT_GNM_R_TESTLIST *ppstTestList_Reply);
    int	gtl_svr_InitPat(PT_GNM_R_PATCONFIG_STATIC *ppstStaticPatConfig_Reply,
                        PT_GNM_PATCONFIG_DYNAMIC *ppstDynamicPatConfig_Reply);
    int	gtl_svr_CheckForMessage(int *pnMessageReceived, GTL_SOCK_MSG *pstMessage);
    int	gtl_svr_Results();
    int	gtl_svr_Endlot();

#elif defined(__cplusplus)

extern "C"
{
    int	gtl_svr_Init(PT_GNM_R_INIT *ppstInit_Reply, char *szFullRecipeFileName);
    int	gtl_svr_SendProdInfo();
    int	gtl_svr_InitTestList(PT_GNM_R_TESTLIST *ppstTestList_Reply);
    int	gtl_svr_InitPat(PT_GNM_R_PATCONFIG_STATIC *ppstStaticPatConfig_Reply,
                        PT_GNM_PATCONFIG_DYNAMIC *ppstDynamicPatConfig_Reply);
    int	gtl_svr_CheckForMessage(int *pnMessageReceived, GTL_SOCK_MSG *pstMessage);
    // send results to GTM server.
    // Return Codes : 1 if successful, 0 else
    int	gtl_svr_Results(unsigned int uiSilentMode);
    int	gtl_svr_Endlot();
    int	gtl_svr_EndOfSplitlot();
}

#else

    extern int gtl_svr_Init();
    extern int gtl_svr_SendProdInfo();
    extern int gtl_svr_InitTestList();
    extern int gtl_svr_InitPat();
    extern int gtl_svr_CheckForMessage();
    extern int gtl_svr_Results();
    extern int gtl_svr_Endlot();
    extern int gtl_svr_EndOfSplitlot();

#endif /* #ifdef _GTL_SERVER_MODULE_ */

#endif /* #ifndef _GTL_SERVER_H_ */
