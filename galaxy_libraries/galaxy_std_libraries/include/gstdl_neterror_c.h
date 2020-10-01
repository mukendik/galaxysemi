/*======================================================================*/
/* Galaxy                                                               */
/* Copyright 1996-99, Softlink                                          */
/* This computer program is protected by copyrigt law                   */
/* and international treaties. Unauthorized reproduction or             */
/* distribution of this program, or any portion of it,may               */
/* result in severe civil and criminal penalties, and will be           */
/* prosecuted to the maximum extent possible under the low.             */
/*======================================================================*/

/*======================================================================*/
/*======================== GSOCKETERROR.H HEADER =======================*/
/*======================================================================*/
#ifndef _GNET_ERROR_H_
#define _GNET_ERROR_H_

#define GNET_ERR_OKAY			0					/* No Error */
#define GNET_ERR_RETRY			1					/* Need to retry the operation */
#define GNET_SZ_OKAY			"No error."
#define GNET_SZ_UNKNOW			"Unknown error."

/* Buffer */
#define GNET_ERR_OFFSET 		50000
#define GNET_ERR_MAF			GNET_ERR_OFFSET+1	/* Memory Allocation Failure */
#define GNET_ERR_BNO			GNET_ERR_OFFSET+2	/* Buffer not opened */
#define GNET_ERR_BNC			GNET_ERR_OFFSET+3	/* Buffer not closed */
#define GNET_ERR_OOB			GNET_ERR_OFFSET+4	/* Out Of Buffer */
#define GNET_ERR_IMT			GNET_ERR_OFFSET+5	/* Invalid Message Type */
#define GNET_ERR_IBS			GNET_ERR_OFFSET+6	/* Invalid buffer size */
#define GNET_ERR_RBA			GNET_ERR_OFFSET+7	/* Received Bad Ack */
#define GNET_ERR_WHS			GNET_ERR_OFFSET+8	/* Wrong header size */
#define GNET_ERR_WAS			GNET_ERR_OFFSET+9	/* Wrong ack size */
#define GNET_ERR_WDS			GNET_ERR_OFFSET+10	/* Wrong data size */
#define GNET_ERR_RIH			GNET_ERR_OFFSET+11	/* Received an invalid header */
#define GNET_ERR_NCF			GNET_ERR_OFFSET+12	/* No callback function defined */
#define GNET_ERR_RWR			GNET_ERR_OFFSET+13	/* Received wrong reply  */
#define GNET_SZ_MAF 			"Memory allocation failure"
#define GNET_SZ_BNO 			"Buffer not opened"
#define GNET_SZ_BNC 			"Buffer not closed"
#define GNET_SZ_OOB 			"Out of buffer access"
#define GNET_SZ_IMT 			"Invalid Message Type"
#define GNET_SZ_IBS 			"Invalid buffer size"
#define GNET_SZ_RBA 			"Receive a Bad Acknowledge message"
#define GNET_SZ_WHS 			"Wrong Header size"
#define GNET_SZ_WAS 			"Wrong Ack size"
#define GNET_SZ_WDS 			"Wrong Data size"
#define GNET_SZ_RIH 			"Received an invalid header"
#define GNET_SZ_NCF				"No callback function defined"
#define GNET_SZ_RWR				"Received unexpected reply message"

/* Socket */
#define GNET_ERR_EDR			GNET_ERR_OFFSET+100 /* Error during receive */
#define GNET_ERR_REC_HDR		GNET_ERR_OFFSET+101 /* Error during receive of Header */
#define GNET_ERR_REC_ACK		GNET_ERR_OFFSET+102 /* Error during receive of Ack */
#define GNET_ERR_REC_DATA		GNET_ERR_OFFSET+103 /* Error during receive of Data */
#define GNET_ERR_EDS			GNET_ERR_OFFSET+104 /* Error during send */
#define GNET_ERR_SND_HDR		GNET_ERR_OFFSET+105 /* Error during send of Header */
#define GNET_ERR_SND_ACK		GNET_ERR_OFFSET+106 /* Error during send of Ack */
#define GNET_ERR_SND_DATA		GNET_ERR_OFFSET+107 /* Error during send of Data */
#define GNET_ERR_CCS			GNET_ERR_OFFSET+108 /* Can't create client socket */
#define GNET_ERR_CSV			GNET_ERR_OFFSET+109 /* Can't connect server */
#define GNET_ERR_SCL			GNET_ERR_OFFSET+110	/* Socket close notification */
#define GNET_ERR_LST			GNET_ERR_OFFSET+111	/* Can't listen */
#define GNET_ERR_ACP			GNET_ERR_OFFSET+112	/* Accept connection error */
#define GNET_SZ_EDR 			"Error during receive operation"
#define GNET_SZ_REC_HDR 		"Error during Header receive operation"
#define GNET_SZ_REC_ACK 		"Error during Acknowledge receive operation"
#define GNET_SZ_REC_DATA		"Error during Data receive operation"
#define GNET_SZ_EDS 			"Error during send operation"
#define GNET_SZ_SND_HDR 		"Error during Header send operation"
#define GNET_SZ_SND_ACK 		"Error during Acknowledge send operation"
#define GNET_SZ_SND_DATA		"Error during Data send operation"
#define GNET_SZ_CCS 			"Cannot create socket"
#define GNET_SZ_CSV 			"Cannot connect"
#define GNET_SZ_LST				"Cannot listen"
#define GNET_SZ_SCL				"Socket closed"
#define GNET_SZ_ACP				"Accept connection error"

#define GNET_ERR_MSG_SIZE		256

/*==============================================================================*/
/* SOCKET ERRORS                                                                */
/* If necessary, map microsoft socket error code with unix error code.          */
/*==============================================================================*/

#if defined __unix__ || __APPLE__&__MACH__
#ifndef MAINWIN

#define WASBASEERR				1
#define UNMAPPED				0xFFFF

#include <sys/errno.h>

#define WSAEINTR				EINTR
#define WSAEBADF				EBADF
#define WSAEACCES				EACCES
#define WSAEFAULT				EFAULT
#define WSAEINVAL				EINVAL
#define WSAEMFILE				EMFILE
#define WSAEWOULDBLOCK			EWOULDBLOCK
#define WSAEINPROGRESS			EINPROGRESS
#define WSAEALREADY 			EALREADY
#define WSAENOTSOCK 			ENOTSOCK
#define WSAEDESTADDRREQ 		EDESTADDRREQ
#define WSAEMSGSIZE 			EMSGSIZE
#define WSAEPROTOTYPE			EPROTOTYPE
#define WSAENOPROTOOPT			ENOPROTOOPT
#define WSAEPROTONOSUPPORT		EPROTONOSUPPORT
#define WSAESOCKTNOSUPPORT		ESOCKTNOSUPPORT
#define WSAEOPNOTSUPP			EOPNOTSUPP
#define WSAEPFNOSUPPORT 		EPFNOSUPPORT
#define WSAEAFNOSUPPORT 		EAFNOSUPPORT
#define WSAEADDRINUSE			EADDRINUSE
#define WSAEADDRNOTAVAIL		EADDRNOTAVAIL
#define WSAENETDOWN 			ENETDOWN
#define WSAENETUNREACH			ENETUNREACH
#define WSAENETRESET			ENETRESET
#define WSAECONNABORTED 		ECONNABORTED
#define WSAECONNRESET			ECONNRESET
#define WSAENOBUFS				ENOBUFS
#define WSAEISCONN				EISCONN
#define WSAENOTCONN 			ENOTCONN
#define WSAESHUTDOWN			ESHUTDOWN
#define WSAETOOMANYREFS 		ETOOMANYREFS
#define WSAETIMEDOUT			ETIMEDOUT
#define WSAECONNREFUSED 		ECONNREFUSED
#define WSAELOOP				ELOOP
#define WSAENAMETOOLONG 		ENAMETOOLONG
#define WSAEHOSTDOWN			EHOSTDOWN
#define WSAEHOSTUNREACH 		EHOSTUNREACH
#define WSAENOTEMPTY			ENOTEMPTY
#define WSAEUSERS				EUSERS
#define WSAEDQUOT				EDQUOT
#define WSAESTALE				ESTALE
#define WSAEREMOTE				EREMOTE
#define WSAEPROCLIM 			EPROCLIM
#define WSAHOST_NOT_FOUND 		HOST_NOT_FOUND
#define WSATRY_AGAIN	 		TRY_AGAIN
#define WSANO_RECOVERY	 		NO_RECOVERY
#define WSANO_DATA		 		NO_DATA

#else /* #ifndef MAINWIN */

#include <WINSOCK.H>

#endif /* #ifndef MAINWIN */

#else /* #if defined __unix__ || __APPLE__&__MACH__ */

#include <WINSOCK.H>

#endif	/* ifdef __unix__ */

#define SZ_WSAEINTR 				"EINTR" 
#define SZ_WSAEBADF 				"EBADF"
#define SZ_WSAEACCES				"EACCES"
#define SZ_WSAEFAULT				"EFAULT"
#define SZ_WSAEINVAL				"EINVAL"
#define SZ_WSAEMFILE				"EMFILE"
#define SZ_WSAEWOULDBLOCK			"EWOULDBLOCK"
#define SZ_WSAEINPROGRESS			"EINPROGRESS"
#define SZ_WSAEALREADY				"EALREADY"
#define SZ_WSAENOTSOCK				"ENOTSOCK"
#define SZ_WSAEDESTADDRREQ			"EDESTADDRREQ"
#define SZ_WSAEMSGSIZE				"EMSGSIZE"
#define SZ_WSAEPROTOTYPE			"EPROTOTYPE"
#define SZ_WSAENOPROTOOPT			"ENOPROTOOPT"
#define SZ_WSAEPROTONOSUPPORT		"EPROTONOSUPPORT"
#define SZ_WSAESOCKTNOSUPPORT		"ESOCKTNOSUPPORT"
#define SZ_WSAEOPNOTSUPP			"EOPNOTSUPP"
#define SZ_WSAEPFNOSUPPORT			"EPFNOSUPPORT"
#define SZ_WSAEAFNOSUPPORT			"EAFNOSUPPORT"
#define SZ_WSAEADDRINUSE			"EADDRINUSE"
#define SZ_WSAEADDRNOTAVAIL 		"EADDRNOTAVAIL"
#define SZ_WSAENETDOWN				"ENETDOWN"
#define SZ_WSAENETUNREACH			"ENETUNREACH"
#define SZ_WSAENETRESET 			"ENETRESET"
#define SZ_WSAECONNABORTED			"ECONNABORTED"
#define SZ_WSAECONNRESET			"ECONNRESET"
#define SZ_WSAENOBUFS				"ENOBUFS"
#define SZ_WSAEISCONN				"EISCONN"
#define SZ_WSAENOTCONN				"ENOTCONN"
#define SZ_WSAESHUTDOWN 			"ESHUTDOWN"
#define SZ_WSAETOOMANYREFS			"ETOOMANYREFS"
#define SZ_WSAETIMEDOUT 			"ETIMEDOUT"
#define SZ_WSAECONNREFUSED			"ECONNREFUSED"
#define SZ_WSAELOOP 				"ELOOP"
#define SZ_WSAENAMETOOLONG			"ENAMETOOLONG"
#define SZ_WSAEHOSTDOWN 			"EHOSTDOWN"
#define SZ_WSAEHOSTUNREACH			"EHOSTUNREACH"
#define SZ_WSAENOTEMPTY 			"ENOTEMPTY"
#define SZ_WSAEPROCLIM				"EPROCLIM"
#define SZ_WSAEUSERS				"EUSERS"
#define SZ_WSAEDQUOT				"EDQUOT"
#define SZ_WSAESTALE				"ESTALE"
#define SZ_WSAEREMOTE				"EREMOTE"
#define SZ_WSAEDISCON				"EDISCON"
#define SZ_WSASYSNOTREADY			"SYSNOTREADY"
#define SZ_WSAVERNOTSUPPORTED		"VERNOTSUPPORTED"
#define SZ_WSANOTINITIALISED		"NOTINITIALISED"
#define SZ_WSAHOST_NOT_FOUND 		"HOST_NOT_FOUND"
#define SZ_WSATRY_AGAIN	 			"TRY_AGAIN"
#define SZ_WSANO_RECOVERY	 		"NO_RECOVERY"
#define SZ_WSANO_DATA		 		"NO_DATA"
#define SZ_WSAUNKNOWN				"Unknown Socket Error"


/*======================================================================*/
/* Exported Functions.                                                  */
/*======================================================================*/
#ifdef _GNET_ERROR_MODULE_

  void gnet_GetErrorMessage();
  void gnet_GetLastSocketError(int nErrorCode, char* szErrMessage);

#else

#if defined(__cplusplus)
extern "C"
{
	void		gnet_GetErrorMessage(int nErrorCode, int nSocketError, char* szErrMessage);
	void		gnet_GetLastSocketError(int nSocketError, char *szSocketMessage);
}
#else

	extern void gnet_GetErrorMessage();  
	extern void	gnet_GetLastSocketError();

#endif /* #if defined(__cplusplus) */

#endif /* #ifdef _GNET_ERROR_MODULE_ */

#endif /* _GNET_ERROR_H_ */



