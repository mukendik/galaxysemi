/*======================================================================*/
/* Galaxy                                                               */
/* Copyright 1996-99, Softlink                                          */
/* This computer program is protected by copyrigt law                   */
/* and international treaties. Unauthorized reproduction or             */
/* distribution of this program, or any portion of it,may               */
/* result in severe civil and criminal penalties, and will be           */
/* prosecuted to the maximum extent possible under the low.             */
/*======================================================================*/

#define _GNET_ERROR_MODULE_

/*======================================================================*/
/*======================== GSOCKETERROR.C  LIBRARY ====================*/
/*======================================================================*/
#include <stdio.h>
#include <string.h>
#if defined __APPLE__&__MACH__
    #include <malloc/malloc.h>
#else
    #include <malloc.h>
#endif
#include <stdlib.h>
#include "gstdl_neterror_c.h"

#if defined __unix__ || __APPLE__&__MACH__
#include <netdb.h>
#endif

/*======================================================================*/
/* PUBLIC Functions                                                     */
/*======================================================================*/
/*======================================================================*/
/* Retreive the error message corresponding to the specified error code.*/
/*                                                                      */
/*======================================================================*/
void gnet_GetErrorMessage(int nErrorCode,
                          int nSocketError,
                          char* szErrMessage)
{
	char szSocketError[GNET_ERR_MSG_SIZE];

	if(!szErrMessage)
		return;

	switch(nErrorCode)
	{
    /* Net Buffer errors */
	case GNET_ERR_OKAY:
		strcpy(szErrMessage,GNET_SZ_OKAY);
		break;

	case GNET_ERR_MAF:
		strcpy(szErrMessage,GNET_SZ_MAF);
		break;

	case GNET_ERR_BNO:
		strcpy(szErrMessage,GNET_SZ_BNO);
		break;

	case GNET_ERR_BNC:
		strcpy(szErrMessage,GNET_SZ_BNC);
		break;

	case GNET_ERR_OOB:
		strcpy(szErrMessage,GNET_SZ_OOB);
		break;

	case GNET_ERR_IMT:
		strcpy(szErrMessage,GNET_SZ_IMT);
		break;

	case GNET_ERR_IBS:
		strcpy(szErrMessage,GNET_SZ_IBS);
		break;

	case GNET_ERR_RBA:
		strcpy(szErrMessage,GNET_SZ_RBA);
		break;

	case GNET_ERR_WHS:
		strcpy(szErrMessage,GNET_SZ_WHS);
		break;
	
	case GNET_ERR_WAS:
		strcpy(szErrMessage,GNET_SZ_WAS);
		break;
	
	case GNET_ERR_WDS:
		strcpy(szErrMessage,GNET_SZ_WDS);
		break;
	
	case GNET_ERR_RIH:
		strcpy(szErrMessage,GNET_SZ_RIH);
		break;
	
	/* Errors with socket error condition */
	case GNET_ERR_EDR:
		gnet_GetLastSocketError(nSocketError, szSocketError);
		sprintf(szErrMessage, "%s (%s) !", GNET_SZ_EDR, szSocketError);
		break;
	
	case GNET_ERR_EDS:
		gnet_GetLastSocketError(nSocketError, szSocketError);
		sprintf(szErrMessage, "%s (%s) !", GNET_SZ_EDS, szSocketError);
		break;
	
	case GNET_ERR_CCS:
		gnet_GetLastSocketError(nSocketError, szSocketError);
		sprintf(szErrMessage, "%s (%s) !", GNET_SZ_CCS, szSocketError);
		break;

	case GNET_ERR_CSV:
		gnet_GetLastSocketError(nSocketError, szSocketError);
		sprintf(szErrMessage, "%s (%s) !", GNET_SZ_CSV, szSocketError);
		break;

	case GNET_ERR_REC_HDR:
		gnet_GetLastSocketError(nSocketError, szSocketError);
		sprintf(szErrMessage, "%s (%s) !", GNET_SZ_REC_HDR, szSocketError);
		break;

	case GNET_ERR_REC_ACK:
		gnet_GetLastSocketError(nSocketError, szSocketError);
		sprintf(szErrMessage, "%s (%s) !", GNET_SZ_REC_ACK, szSocketError);
		break;

	case GNET_ERR_REC_DATA:
		gnet_GetLastSocketError(nSocketError, szSocketError);
		sprintf(szErrMessage, "%s (%s) !", GNET_SZ_REC_DATA, szSocketError);
		break;

	case GNET_ERR_SND_HDR:
		gnet_GetLastSocketError(nSocketError, szSocketError);
		sprintf(szErrMessage, "%s (%s) !", GNET_SZ_SND_HDR, szSocketError);
		break;

	case GNET_ERR_SND_ACK:
		gnet_GetLastSocketError(nSocketError, szSocketError);
		sprintf(szErrMessage, "%s (%s) !", GNET_SZ_SND_ACK, szSocketError);
		break;

	case GNET_ERR_SND_DATA:
		gnet_GetLastSocketError(nSocketError, szSocketError);
		sprintf(szErrMessage, "%s (%s) !", GNET_SZ_SND_DATA, szSocketError);
		break;

	case GNET_ERR_SCL:
		gnet_GetLastSocketError(nSocketError, szSocketError);
		sprintf(szErrMessage, "%s (%s) !", GNET_SZ_SCL, szSocketError);
		break;

	case GNET_ERR_LST:
		gnet_GetLastSocketError(nSocketError, szSocketError);
		sprintf(szErrMessage, "%s (%s) !", GNET_SZ_LST, szSocketError);
		break;

	case GNET_ERR_ACP:
		gnet_GetLastSocketError(nSocketError, szSocketError);
		sprintf(szErrMessage, "%s (%s) !", GNET_SZ_ACP, szSocketError);
		break;

	case GNET_ERR_NCF:
		sprintf(szErrMessage, "%s !", GNET_SZ_NCF);
		break;

    case GNET_ERR_RWR:
        sprintf(szErrMessage, "%s !", GNET_SZ_RWR);
        break;

    default:
		sprintf(szErrMessage, "%s (%d) !", GNET_SZ_UNKNOW, nErrorCode);
		break;
	}
}

/*======================================================================*/
/* Retreive the error message corresponding to the specified error code.*/
/*                                                                      */
/*======================================================================*/
void  gnet_GetLastSocketError(nErrorCode, szErrMessage)
int   nErrorCode;
char	*szErrMessage;
{
  if(!szErrMessage)
    return;

	switch(nErrorCode)
	{
  case WSAEINTR:
		strcpy(szErrMessage,SZ_WSAEINTR);
		break;
    
  case WSAEBADF:
		strcpy(szErrMessage,SZ_WSAEBADF);
		break;
    
  case WSAEACCES:
		strcpy(szErrMessage,SZ_WSAEACCES);
		break;
    
  case WSAEFAULT:
		strcpy(szErrMessage,SZ_WSAEFAULT);
		break;
    
  case WSAEINVAL:
		strcpy(szErrMessage,SZ_WSAEINVAL);
		break;
    
  case WSAEMFILE:
		strcpy(szErrMessage,SZ_WSAEMFILE);
		break;
    
  case WSAEWOULDBLOCK:
		strcpy(szErrMessage,SZ_WSAEWOULDBLOCK);
		break;
    
  case WSAEINPROGRESS:
		strcpy(szErrMessage,SZ_WSAEINPROGRESS);
		break;
    
  case WSAEALREADY:
		strcpy(szErrMessage,SZ_WSAEALREADY);
		break;
    
  case WSAENOTSOCK:
		strcpy(szErrMessage,SZ_WSAENOTSOCK);
		break;
    
  case WSAEDESTADDRREQ:
		strcpy(szErrMessage,SZ_WSAEDESTADDRREQ);
		break;
    
  case WSAEMSGSIZE:
		strcpy(szErrMessage,SZ_WSAEMSGSIZE);
		break;
    
  case WSAEPROTOTYPE:
		strcpy(szErrMessage,SZ_WSAEPROTOTYPE);
		break;
    
  case WSAENOPROTOOPT:
		strcpy(szErrMessage,SZ_WSAENOPROTOOPT);
		break;
    
  case WSAEPROTONOSUPPORT:
		strcpy(szErrMessage,SZ_WSAEPROTONOSUPPORT);
		break;
    
  case WSAESOCKTNOSUPPORT:
		strcpy(szErrMessage,SZ_WSAESOCKTNOSUPPORT);
		break;
    
  case WSAEOPNOTSUPP:
		strcpy(szErrMessage,SZ_WSAEOPNOTSUPP);
		break;
    
  case WSAEPFNOSUPPORT:
		strcpy(szErrMessage,SZ_WSAEPFNOSUPPORT);
		break;
    
  case WSAEAFNOSUPPORT:
		strcpy(szErrMessage,SZ_WSAEAFNOSUPPORT);
		break;
    
  case WSAEADDRINUSE:
		strcpy(szErrMessage,SZ_WSAEADDRINUSE);
		break;
    
  case WSAEADDRNOTAVAIL:
		strcpy(szErrMessage,SZ_WSAEADDRNOTAVAIL);
		break;
    
  case WSAENETDOWN:
		strcpy(szErrMessage,SZ_WSAENETDOWN);
		break;
    
  case WSAENETUNREACH:
		strcpy(szErrMessage,SZ_WSAENETUNREACH);
		break;
    
  case WSAENETRESET:
		strcpy(szErrMessage,SZ_WSAENETRESET);
		break;
    
  case WSAECONNABORTED:
		strcpy(szErrMessage,SZ_WSAECONNABORTED);
		break;
    
  case WSAECONNRESET:
		strcpy(szErrMessage,SZ_WSAECONNRESET);
		break;
    
  case WSAENOBUFS:
		strcpy(szErrMessage,SZ_WSAENOBUFS);
		break;
    
  case WSAEISCONN:
		strcpy(szErrMessage,SZ_WSAEISCONN);
		break;
    
  case WSAENOTCONN:
		strcpy(szErrMessage,SZ_WSAENOTCONN);
		break;
    
  case WSAESHUTDOWN:
		strcpy(szErrMessage,SZ_WSAESHUTDOWN);
		break;
    
  case WSAETOOMANYREFS:
		strcpy(szErrMessage,SZ_WSAETOOMANYREFS);
		break;
    
  case WSAETIMEDOUT:
		strcpy(szErrMessage,SZ_WSAETIMEDOUT);
		break;
    
  case WSAECONNREFUSED:
		strcpy(szErrMessage,SZ_WSAECONNREFUSED);
		break;
    
  case WSAELOOP:
		strcpy(szErrMessage,SZ_WSAELOOP);
		break;
    
  case WSAENAMETOOLONG:
		strcpy(szErrMessage,SZ_WSAENAMETOOLONG);
		break;
    
  case WSAEHOSTDOWN:
		strcpy(szErrMessage,SZ_WSAEHOSTDOWN);
		break;
    
  case WSAEHOSTUNREACH:
		strcpy(szErrMessage,SZ_WSAEHOSTUNREACH);
		break;
    
  case WSAENOTEMPTY:
		strcpy(szErrMessage,SZ_WSAENOTEMPTY);
		break;
    
  case WSAEUSERS:
		strcpy(szErrMessage,SZ_WSAEUSERS);
		break;
    
  case WSAEDQUOT:
		strcpy(szErrMessage,SZ_WSAEDQUOT);
		break;
    
  case WSAESTALE:
		strcpy(szErrMessage,SZ_WSAESTALE);
		break;
    
  case WSAEREMOTE:
		strcpy(szErrMessage,SZ_WSAEREMOTE);
		break;
 
#if !defined __unix__ && !defined __APPLE__&__MACH__
  case WSAHOST_NOT_FOUND:
		strcpy(szErrMessage,SZ_WSAHOST_NOT_FOUND);
		break;

  case WSATRY_AGAIN:
		strcpy(szErrMessage,SZ_WSATRY_AGAIN);
		break;

  case WSANO_RECOVERY:
		strcpy(szErrMessage,SZ_WSANO_RECOVERY);
		break;

  case WSANO_DATA:
		strcpy(szErrMessage,SZ_WSANO_DATA);
		break;

  case WSAEPROCLIM:
		strcpy(szErrMessage,SZ_WSAEPROCLIM);
		break;
    
  case WSAEDISCON:
		strcpy(szErrMessage,SZ_WSAEDISCON);
		break;
    
  case WSASYSNOTREADY:
		strcpy(szErrMessage,SZ_WSASYSNOTREADY);
		break;
    
  case WSAVERNOTSUPPORTED:
		strcpy(szErrMessage,SZ_WSAVERNOTSUPPORTED);
		break;
    
  case WSANOTINITIALISED:
		strcpy(szErrMessage,SZ_WSANOTINITIALISED);
		break;
#endif

  default:
		sprintf(szErrMessage,"%s (%d)",SZ_WSAUNKNOWN,nErrorCode);
		break;
	}
}
