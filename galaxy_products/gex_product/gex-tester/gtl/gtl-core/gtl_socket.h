/****************************************************************************************/
/* Copyright Galaxy                                                                     */
/* This computer program is protected by copyright law                                  */
/* and international treaties. Unauthorized reproduction or                             */
/* distribution of this program, or any portion of it,may                               */
/* result in severe civil and criminal penalties, and will be                           */
/* prosecuted to the maximum extent possible under the law.                             */
/****************************************************************************************/
/*                                                                                      */
/* Module: gtl_socket.h                                                                 */
/*                                                                                      */
/* Socket communication functions to communicate with GTM (Galaxy Tester Monitor).      */
/*                                                                                      */
/****************************************************************************************/
/*                                                                                      */
/* Notes:                                                                               */
/*                                                                                      */
/****************************************************************************************/
/*                                                                                      */
/* Revision history:                                                                    */
/*                                                                                      */
/* o [BG] 18 May 2005: Created.                                                         */
/*                                                                                      */
/****************************************************************************************/

#ifndef _GTL_SOCKET_H_
#define _GTL_SOCKET_H_

/*======================================================================================*/
/* Includes                                                                             */
/*======================================================================================*/
#if defined unix || defined __MACH__
#include <errno.h>
#else
#include <winsock.h>
#endif

/*======================================================================================*/
/* Macros                                                                               */
/*======================================================================================*/
#if defined unix || __MACH__
#include <netinet/in.h>
#define SOCK_SOCKETTYPE     int
#define SOCK_CLOSE          close
#define SOCK_SHUTDOWN_MODE  2
#define INVALID_SOCKET      -1
#define SOCKET_ERROR        -1
#define SOCK_IOCTL          ioctl
#define SOCK_TIMEVAL        struct timeval
typedef in_addr_t           SOCK_IN_ADDR;
#else
#define SOCK_SOCKETTYPE     SOCKET
#define SOCK_CLOSE          closesocket
#define SOCK_SHUTDOWN_MODE  2 /* SD_BOTH */
#define SOCK_IOCTL          ioctlsocket
#define SOCK_TIMEVAL        TIMEVAL
typedef IN_ADDR             SOCK_IN_ADDR;
#endif

/*======================================================================================*/
/* Defines                                                                              */
/*======================================================================================*/
/* Socket parameters */
#define GTL_CLIENT_SOCKET_TYPE				SOCK_STREAM
#define GTL_CLIENT_SOCKET_PORT				0
#define GTL_CLIENT_SOCKET_FAMILY			AF_INET

/* Timeouts (in seconds) */
#ifdef _DEBUG
#define GTL_SOCK_CONNECT_TIMEOUT			60
#define GTL_SOCK_SND_TIMEOUT				5
#define GTL_SOCK_RCV_TIMEOUT				120
#else
#define GTL_SOCK_CONNECT_TIMEOUT			60
#define GTL_SOCK_SND_TIMEOUT				10
#define GTL_SOCK_RCV_TIMEOUT				10
#endif

/* String lengths */
#define GTL_SOCK_MAX_HOSTNAME				512

/* Trace file */
#ifdef unix
#define GTL_SOCKET_TRACE_FILE				"./trace_socket.txt"
#else
#define GTL_SOCKET_TRACE_FILE				"c:\\trace_socket.txt"
#endif

/* Trace codes */
#define GTL_TRACE_ONRECEIVE					0x01		/* Received ONRECEIVE notification */
#define GTL_TRACE_RECV_ENTER				0x02		/* Enter a receive function */
#define GTL_TRACE_RECEIVED_BUFFER			0x03		/* Received a buffer */
#define GTL_TRACE_SEND_ENTER				0x04		/* Enter a send function */
#define GTL_TRACE_SENT_BUFFER				0x05		/* Sent a buffer */
#define GTL_TRACE_SEND_ERROR				0x06		/* Error sending the buffer */
#define GTL_TRACE_RECV_ERROR				0x07		/* Error receiving a buffer */
#define GTL_TRACE_CANCEL_BLOCKING			0x08		/* Blocking call cancelled on timeout */
#define GTL_TRACE_ONRECEIVE_DIS				0x0a		/* Received ONRECEIVE notification while ONRECEIVE disabled */
#define GTL_TRACE_RECV_HEADER				0x0b		/* Trying to receive the header. */
#define GTL_TRACE_RECV_DATA					0x0c		/* Trying to receive the data buffer. */
#define GTL_TRACE_SEND_ACK_HEADER			0x0d		/* Sending Acknowledge for the header. */
#define GTL_TRACE_SEND_ACK_DATA				0x0e		/* Sending Acknowledge for the data. */
#define GTL_TRACE_SEND_HEADER				0x0f		/* Sending the header buffer. */
#define GTL_TRACE_SEND_DATA					0x10		/* Sending the data buffer. */
#define GTL_TRACE_RECV_ACK_HEADER			0x11		/* Trying to receive Acknowledge for the header. */
#define GTL_TRACE_RECV_ACK_DATA				0x12		/* Trying to receive Acknowledge for the data. */

/*======================================================================================*/
/* Error Codes																			*/
/*======================================================================================*/
#define GTL_SOCK_ERR_OKAY					0x00		/* No Error */
#define GTL_SOCK_ERR_CREATE 				0x01		/* Couldn't create socket */
#define GTL_SOCK_ERR_BIND					0x02		/* Couldn't bind socket */
#define GTL_SOCK_ERR_READBACK				0x03		/* Couldn't read back socket informations */
#define GTL_SOCK_ERR_HOSTNAME				0x04		/* Couldn't retrieve IP address for Galaxy server */
#define GTL_SOCK_ERR_OPTION 				0x05		/* Couldn't set socket options */
#define GTL_SOCK_ERR_CONNECT				0x06		/* Couldn't connect to server socket */
#define GTL_SOCK_ERR_INIT					0x07		/* Couldn't initialize socket system */
#define GTL_SOCK_ERR_CLEANUP				0x08		/* Couldn't cleanup socket system */
#define GTL_SOCK_ERR_SEND					0x09		/* Couldn't send request to server */
#define GTL_SOCK_ERR_REC					0x0a		/* Couldn't receive request from server */
#define GTL_SOCK_ERR_MAF					0x0b		/* Memory allocation failure */
#define GTL_SOCK_ERR_CBUF					0x0c		/* Error creating network buffer */
#define GTL_SOCK_ERR_RBUF					0x0d		/* Error reading network buffer */
#define GTL_SOCK_ERR_DOWN					0x0e		/* Socket is not up */
#define GTL_SOCK_ERR_CONFIG 				0x0f		/* Couldn't configure socket */
#define GTL_SOCK_ERR_COMM	 				0x10		/* Error during communication (send/receive) */
#define GTL_SOCK_ERR_TRANSFER_INIT			0x11		/* Error during INIT message */
#define GTL_SOCK_ERR_TRANSFER_HBEAT			0x12		/* Error during HEARTBEAT message */
#define GTL_SOCK_ERR_TRANSFER_RECONNECT	 	0x13		/* Error during RECONNECT message */
#define GTL_SOCK_ERR_CONNECT_MAXCONN		0x14		/* Couldn't connect to server socket because max conn reached on server */
#define GTL_SOCK_ERR_VERSIONMISMATCH		0x15		/* GTM/GTL version mismatch */
#define GTL_SOCK_ERR_NO_GTM_LIC             0x16        /*NO GTM LICENSE*/
#define GTL_SOCK_ERR_NO_GTL_LIC             0x17        /*NO GTM LICENSE*/

/*======================================================================================*/
/* Type definitions																		*/
/*======================================================================================*/
typedef struct tagGTL_SOCK_MSG
{
    unsigned short  mType;
    unsigned short  mAckRequested;
    void            *mMsgStruct;
	
} GTL_SOCK_MSG, *PT_GTL_SOCK_MSG;

/*======================================================================================*/
/* PUBLIC Functions																		*/
/*======================================================================================*/
#ifdef _GTL_SOCKET_MODULE_

    int				gtl_sock_Start();
    int				gtl_sock_Restart();
    int				gtl_sock_CheckConnection();
    void			gtl_sock_Stop();
    int				gtl_sock_SendHeartbeat();
    void			gtl_sock_GetLastSocketError();
    int	gtl_sock_SendMessage(PT_GTL_SOCK_MSG, PT_GTL_SOCK_MSG, unsigned short);
    int				gtl_sock_ReceiveMessage();
    int				gtl_sock_CheckForMessage();

#elif defined(__cplusplus)

extern "C"
{
    /* Description  : create/connect client socket to communicate with 1 or more GTM servers.
        Argument(s)  :
           int nSilent
                0   : display connection messages
                !=0 : don't display connection messages
           int NumOfServerToTry: number of server to try to connect to. The config file could contains several.
            If 1, this function will try to connect only to the one pointed by gtl_stGTMServer
            and if failure will modify gtl_stGTMServer to point to the next one if any.
            If 0 or negatif, this function will function will try all servers registered in gtl_stGTMServer list.

        Return Codes : GTL_SOCK_ERR_OKAY if successful, GTL_SOCK_ERR_XXX error code else  */
    int	gtl_sock_Start(int nSilent, unsigned NumOfServerToTry);

    int	gtl_sock_Restart(int nSilent);
    int	gtl_sock_CheckConnection();
    void gtl_sock_Stop();
    int	gtl_sock_SendHeartbeat(int nSilent);
    void gtl_sock_GetLastSocketError(char *szErrMessage);
    // Send message to server. If ptMsgToReceive is not null, wait for server reply
    int	gtl_sock_SendMessage(PT_GTL_SOCK_MSG ptMsgToSend, PT_GTL_SOCK_MSG ptMsgToReceive,
                             unsigned short ExpectedRcvMsgType);
    int	gtl_sock_ReceiveMessage(PT_GTL_SOCK_MSG ptMessage);
    int	gtl_sock_CheckForMessage(PT_GTL_SOCK_MSG ptMessage, int *pnMessageReceived);
}
 
#else

extern int		gtl_sock_Start();
extern int		gtl_sock_Restart();
extern int		gtl_sock_CheckConnection();
extern void		gtl_sock_Stop();
extern int		gtl_sock_SendHeartbeat();
extern void		gtl_sock_GetLastSocketError();
extern int		gtl_sock_SendMessage();
extern int		gtl_sock_ReceiveMessage();
extern int		gtl_sock_CheckForMessage();

#endif /* #ifdef _GTL_SOCKET_MODULE_ */

#endif /* #ifndef _GTL_SOCKET_H_ */



