/****************************************************************************************/
/* Copyright Galaxy                                                                     */
/* This computer program is protected by copyright law                                  */
/* and international treaties. Unauthorized reproduction or                             */
/* distribution of this program, or any portion of it,may                               */
/* result in severe civil and criminal penalties, and will be                           */
/* prosecuted to the maximum extent possible under the law.                             */
/****************************************************************************************/
/*                                                                                      */
/* Module: gtl_socket.c                                                                 */
/*                                                                                      */
/* Socket communication functions to communicate with GTM (Galaxy Tester Monitor).      */
/*                                                                                      */
/****************************************************************************************/

#define _GTL_ERROR_MODULE_

/*======================================================================================*/
/* Includes                                                                             */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <syslog.h>
#include <gstdl_neterror_c.h>    /* For error codes */

#include "gtl_error.h"
#include "gtl_message.h"
#include "gtl_core.h"
#include "gtl_socket.h"

/*======================================================================================*/
/* Public declarations                                                                  */
/*======================================================================================*/

/*======================================================================================*/
/* Private declarations                                                                 */
/*======================================================================================*/
void gtl_err_Main();
void gtl_err_Socket();
void gtl_err_Gnet();

/*======================================================================================*/
/* Public CODE                                                                          */
/*======================================================================================*/
/*======================================================================================*/
/* Description  : displays error codes returned by GTL modules.                         */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     int nModule                                                                      */
/*         specifies the module who caused the error (see gtl_error.h)                  */
/*                                                                                      */
/*     int nErrorCode                                                                   */
/*         code specifying error (see module's header file)                             */
/*                                                                                      */
/*     int nInfo                                                                        */
/*         specifies some error dependant info                                          */
/*                                                                                      */
/*     char* szInfo                                                                     */
/*         specifies some error dependant text                                          */
/*                                                                                      */
/* Return Codes : none                                                                  */
/*======================================================================================*/
void gtl_err_Display(nModule, nErrorCode, nInfo, szInfo)
int nModule;
int	nErrorCode;
int	nInfo;
char *szInfo;
{
    GTL_LOG(4, "GTL error: Code=%d Info=%s", nErrorCode, szInfo?szInfo:"?")

	/* Check module */
	switch(nModule)
	{
		case GTL_MODULE_MAIN:
			gtl_err_Main(nErrorCode, nInfo, szInfo, GTL_MSG_TODISPLAY, NULL);
			break;
		case GTL_MODULE_SOCKET:
			gtl_err_Socket(nErrorCode, nInfo, szInfo, GTL_MSG_TODISPLAY, NULL); 
			break;
		case GTL_MODULE_GNET:
            gtl_err_Gnet(nErrorCode, nInfo, GTL_MSG_TODISPLAY, NULL);
			break;
		default:
			break;
	}
	
	return;
}

/*======================================================================================*/
/* Description  : retrieves error codes returned by GTL modules.                        */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     int nModule                                                                      */
/*         specifies the module who caused the error (see gtl_error.h)                  */
/*                                                                                      */
/*     int nErrorCode                                                                   */
/*         code specifying error (see module's header file)                             */
/*                                                                                      */
/*     int nInfo                                                                        */
/*         specifies some error dependant info                                          */
/*                                                                                      */
/*     char* szInfo                                                                     */
/*         specifies some error dependant text                                          */
/*                                                                                      */
/*     char* szDestBuffer                                                               */
/*         string to receive message                                                    */
/*                                                                                      */
/* Return Codes : none                                                                  */
/*======================================================================================*/
void gtl_err_Retrieve(nModule, nErrorCode, nInfo, szInfo, szDestBuffer)
int nModule;
int	nErrorCode;
int	nInfo;
char *szInfo, *szDestBuffer;
{
	/* Check module */
	switch(nModule)
	{
		case GTL_MODULE_MAIN:
			gtl_err_Main(nErrorCode, nInfo, szInfo, GTL_MSG_TOBUFFER, szDestBuffer);
			break;
		case GTL_MODULE_SOCKET:
			gtl_err_Socket(nErrorCode, nInfo, szInfo, GTL_MSG_TOBUFFER, szDestBuffer); 
			break;
		case GTL_MODULE_GNET:
            gtl_err_Gnet(nErrorCode, nInfo, GTL_MSG_TOBUFFER, szDestBuffer);
			break;
		default:
            GTL_LOG(4, "Unknown GTL module %d", nModule);
			break;
	}
	
	return;
}

/*======================================================================================*/
/* Private CODE                                                                         */
/*======================================================================================*/
/*======================================================================================*/
/* Description  : handles error codes returned by GTL main module.                      */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     int nErrorCode                                                                   */
/*         code specifying error (see module's header file)                             */
/*                                                                                      */
/*     int nInfo                                                                        */
/*         specifies some error dependant info                                          */
/*                                                                                      */
/*     char* szInfo                                                                     */
/*         specifies some error dependant text                                          */
/*                                                                                      */
/*     int nMode                                                                        */
/*         specifies destination of the message                                         */
/*         GTL_MSG_TODISPLAY : send msg to display                                      */
/*         GTL_MSG_TOBUFFER  : send msg to buffer                                       */
/*                                                                                      */
/*     char *szDestBuffer                                                               */
/*         string to receive msg (if GTL_MSG_TOBUFFER)                                  */
/*                                                                                      */
/* Return Codes : none                                                                  */
/*======================================================================================*/
void gtl_err_Main(nErrorCode, nInfo, szInfo, nMode, szDestBuffer)
int nErrorCode;
int	nInfo;
char *szInfo;
int nMode;
char *szDestBuffer;
{
	/* Check ErrorCode */
	switch(nErrorCode)
	{
        case GTL_CORE_ERR_CONF:
			gtl_msg_Error(GTL_MSGERROR_MAIN_CONF, szInfo, nMode, szDestBuffer);
			break;
        case GTL_CORE_ERR_CRRF:
            gtl_msg_Error(GTL_MSGERROR_MAIN_CRRF, szInfo, nMode, szDestBuffer);
            break;
        case GTL_CORE_ERR_GTM_INIT:
			gtl_msg_Error(GTL_MSGERROR_MAIN_GTM_INIT, szInfo, nMode, szDestBuffer);
			break;
        case GTL_CORE_ERR_GTM_RECIPE:
            gtl_msg_Error(GTL_MSGERROR_MAIN_GTM_RECIPE, szInfo, nMode, szDestBuffer);
			break;
        case GTL_CORE_ERR_GTM_LICEXPIRED:
			gtl_msg_Error(GTL_MSGERROR_MAIN_GTM_LICEXPIRED, szInfo, nMode, szDestBuffer);
			break;
        case GTL_CORE_ERR_GTM_NOLIC:
            gtl_msg_Error(GTL_MSGERROR_MAIN_GTM_NOLIC, szInfo, nMode, szDestBuffer);
            break;
        case GTL_CORE_ERR_GTM_TESTLIST:
			gtl_msg_Error(GTL_MSGERROR_MAIN_GTM_TESTLIST, szInfo, nMode, szDestBuffer);
			break;
        case GTL_CORE_ERR_GTM_INITPAT:
			gtl_msg_Error(GTL_MSGERROR_MAIN_GTM_INITPAT, szInfo, nMode, szDestBuffer);
			break;
        case GTL_CORE_ERR_GTM_COMM:
			gtl_msg_Error(GTL_MSGERROR_MAIN_GTM_COMM, szInfo, nMode, szDestBuffer);
			break;
        case GTL_CORE_ERR_GTM_UNKNOWN_STATUS:
            gtl_msg_Error(GTL_MSGERROR_MAIN_GTM_UNKNOWN_STATUS, szInfo, nMode, szDestBuffer);
            break;
        case GTL_CORE_ERR_UPDATE_SPAT:
            gtl_msg_Error(GTL_MSGERROR_MAIN_UPDATE_SPAT, szInfo, nMode, szDestBuffer);
            break;
        case GTL_CORE_ERR_UPDATE_DPAT:
            gtl_msg_Error(GTL_MSGERROR_MAIN_UPDATE_DPAT, szInfo, nMode, szDestBuffer);
            break;
        case GTL_CORE_ERR_PROCESSMESSAGE:
            gtl_msg_Error(GTL_MSGERROR_MAIN_PROCESSMESSAGE, szInfo, nMode, szDestBuffer);
            break;
        default:
            GTL_LOG(4, "Unknown error code %d", nErrorCode);
			break;
	}

    /* To avoid warning on unused parameter */
    nInfo = 0;
}

/*======================================================================================*/
/* Description  : handles error codes returned by socket module.                        */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     int nErrorCode                                                                   */
/*         code specifying error (see module's header file)                             */
/*                                                                                      */
/*     int nInfo                                                                        */
/*         specifies some error dependant info                                          */
/*                                                                                      */
/*     char* szInfo                                                                     */
/*         specifies some error dependant text                                          */
/*                                                                                      */
/*     int nMode                                                                        */
/*         specifies destination of the message                                         */
/*         GTL_MSG_TODISPLAY : send msg to display                                      */
/*         GTL_MSG_TOBUFFER  : send msg to buffer                                       */
/*                                                                                      */
/*     char *szDestBuffer                                                               */
/*         string to receive msg (if GTL_MSG_TOBUFFER)                                  */
/*                                                                                      */
/* Return Codes : none                                                                  */
/*======================================================================================*/
void gtl_err_Socket(nErrorCode, nInfo, szInfo, nMode, szDestBuffer)
int nErrorCode;
int	nInfo;
char *szInfo;
int nMode;
char *szDestBuffer;
{
	/* Check ErrorCode */
	switch(nErrorCode)
	{
		case GTL_SOCK_ERR_CREATE:
			gtl_msg_Error(GTL_MSGERROR_SOCKET_CREATE, szInfo, nMode, szDestBuffer);
			break;
		case GTL_SOCK_ERR_BIND:
			gtl_msg_Error(GTL_MSGERROR_SOCKET_BIND, szInfo, nMode, szDestBuffer);
			break;
		case GTL_SOCK_ERR_READBACK:
			gtl_msg_Error(GTL_MSGERROR_SOCKET_READBACK, szInfo, nMode, szDestBuffer);
			break;
		case GTL_SOCK_ERR_HOSTNAME:
			gtl_msg_Error(GTL_MSGERROR_SOCKET_HOSTNAME, szInfo, nMode, szDestBuffer);
			break;
		case GTL_SOCK_ERR_OPTION:
			gtl_msg_Error(GTL_MSGERROR_SOCKET_OPTION, szInfo, nMode, szDestBuffer);
			break;
		case GTL_SOCK_ERR_CONNECT:
			gtl_msg_Error(GTL_MSGERROR_SOCKET_CONNECT, szInfo, nMode, szDestBuffer);
			break;
		case GTL_SOCK_ERR_INIT:
			gtl_msg_Error(GTL_MSGERROR_SOCKET_INIT, szInfo, nMode, szDestBuffer);
			break;
		case GTL_SOCK_ERR_CLEANUP:
			gtl_msg_Error(GTL_MSGERROR_SOCKET_CLEANUP, szInfo, nMode, szDestBuffer);
			break;
		case GTL_SOCK_ERR_SEND:
			gtl_msg_Error(GTL_MSGERROR_SOCKET_SEND, szInfo, nMode, szDestBuffer);
			break;
		case GTL_SOCK_ERR_REC:
			gtl_msg_Error(GTL_MSGERROR_SOCKET_REC, szInfo, nMode, szDestBuffer);
			break;
		case GTL_SOCK_ERR_MAF:
			gtl_msg_Error(GTL_MSGERROR_SOCKET_MAF, szInfo, nMode, szDestBuffer);
			break;
		case GTL_SOCK_ERR_CBUF:
			gtl_msg_Error(GTL_MSGERROR_SOCKET_CBUF, szInfo, nMode, szDestBuffer);
			break;
		case GTL_SOCK_ERR_RBUF:
			gtl_msg_Error(GTL_MSGERROR_SOCKET_RBUF, szInfo, nMode, szDestBuffer);
			break;
		case GTL_SOCK_ERR_DOWN:
			gtl_msg_Error(GTL_MSGERROR_SOCKET_DOWN, szInfo, nMode, szDestBuffer);
			break;
		case GTL_SOCK_ERR_CONFIG:
			gtl_msg_Error(GTL_MSGERROR_SOCKET_CONFIG, szInfo, nMode, szDestBuffer);
			break;
		case GTL_SOCK_ERR_COMM:
			gtl_msg_Error(GTL_MSGERROR_SOCKET_COMM, szInfo, nMode, szDestBuffer);
			break;
		case GTL_SOCK_ERR_TRANSFER_INIT:
			gtl_msg_Error(GTL_MSGERROR_SOCKET_TRANSFER_INIT, szInfo, nMode, szDestBuffer);
			break;
		case GTL_SOCK_ERR_TRANSFER_HBEAT:
			gtl_msg_Error(GTL_MSGERROR_SOCKET_TRANSFER_HBEAT, szInfo, nMode, szDestBuffer);
			break;
		case GTL_SOCK_ERR_TRANSFER_RECONNECT:
			gtl_msg_Error(GTL_MSGERROR_SOCKET_TRANSFER_RECONNECT, szInfo, nMode, szDestBuffer);
			break;
		default:
            GTL_LOG(4, "Unknown error code : %d", nErrorCode);
			break;
	}

    /* To avoid warning on unused parameter */
    nInfo = 0;
}

/*======================================================================================*/
/* Description  : handles error codes returned by Gnet module.                          */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     int nErrorCode                                                                   */
/*         code specifying error (see module's header file)                             */
/*                                                                                      */
/*     int nInfo                                                                        */
/*         specifies some error dependant info                                          */
/*                                                                                      */
/*     char* szInfo                                                                     */
/*         specifies some error dependant text                                          */
/*                                                                                      */
/*     int nMode                                                                        */
/*         specifies destination of the message                                         */
/*         GTL_MSG_TODISPLAY : send msg to display                                      */
/*         GTL_MSG_TOBUFFER  : send msg to buffer                                       */
/*                                                                                      */
/*     char *szDestBuffer                                                               */
/*         string to receive msg (if GTL_MSG_TOBUFFER)                                  */
/*                                                                                      */
/* Return Codes : none                                                                  */
/*======================================================================================*/
void gtl_err_Gnet(nErrorCode, nInfo, nMode, szDestBuffer)
int 	nErrorCode;
int		nInfo;
int 	nMode;
char	*szDestBuffer;
{
    char szErrorMessage[GNET_ERR_MSG_SIZE]="";
	gnet_GetErrorMessage(nErrorCode, nInfo, szErrorMessage);
	gtl_msg_Error(GTL_MSGERROR_GNET_ERROR, szErrorMessage, nMode, szDestBuffer);
}
