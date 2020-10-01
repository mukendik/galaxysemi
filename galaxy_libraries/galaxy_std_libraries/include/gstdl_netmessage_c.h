/*======================================================================*/
/* Galaxy                                                               */
/* Copyright 1996-2001, Softlink                                        */
/* This computer program is protected by copyrigt law                   */
/* and international treaties. Unauthorized reproduction or             */
/* distribution of this program, or any portion of it,may               */
/* result in severe civil and criminal penalties, and will be           */
/* prosecuted to the maximum extent possible under the low.             */
/*======================================================================*/

/*
    Galaxy Network Message :
*/

/*======================================================================*/
/*======================= GNETMESSAGE.H HEADER =========================*/
/*======================================================================*/
#ifndef _GNET_MESSAGE_H_
#define _GNET_MESSAGE_H_

/*======================================================================*/
/* Includes :															*/
/*======================================================================*/
#include "gstdl_netbuffer_c.h"

/*======================================================================*/
/* Global defines :				                                        */
/*======================================================================*/
#define	GNM_HEADER_SIZE			9
#define GNM_ACK_SIZE			12

#define	GNM_MAX_MSG_SIZE		0xFFFFFFF	/* Max message size allowed. *** Very important to control header validity */

/*======================================================================*/
/* Common Message structures.                                           */
/*======================================================================*/

/* HEADER */
typedef struct tagHEADER
{
    // message length : in bytes
    unsigned int  mMessageLength;
    // Message type : depends on the user
    unsigned int  mMessageType;
    // 0 : No ack requested, 1 : requested
    GNB_BYTE      mAckRequested;
} GNM_HEADER, *PT_GNM_HEADER;

/*======================================================================*/
/* Exported Functions.                                                  */
/*======================================================================*/
#ifdef _GNET_MESSAGE_MODULE_

int             gnm_CreateBufferHeader();
int             gnm_CreateBufferData();
int             gnm_CreateBufferHeaderData();
int             gnm_ReadBufferHeader();
int             gnm_ReadBufferData();
void			gnm_SetReadBufferHook();
void			gnm_SetAddDataToBufferHook();

#else

#if defined(__cplusplus)
extern "C"
{
    int         gnm_CreateBufferHeader(PT_GNM_HEADER, char **);
    int         gnm_CreateBufferData(unsigned int , void *, char **, unsigned int *);
    int         gnm_CreateBufferHeaderData(unsigned int , unsigned short, void *, char **, unsigned int *);
    int         gnm_ReadBufferHeader(PT_GNM_HEADER, char *);
    int         gnm_ReadBufferData(unsigned int, void **, char *, unsigned int);
    void        gnm_SetReadBufferHook(int (*fhReadBufferHook)(unsigned int,PT_GNB_HBUFFER,void*));
    void        gnm_SetAddDataToBufferHook(int (*fhAddDataToBufferHook)(unsigned int,PT_GNB_HBUFFER,void*));
}
#else

extern int      gnm_CreateBufferHeader();
extern int      gnm_CreateBufferData();
extern int      gnm_CreateBufferHeaderData();
extern int      gnm_ReadBufferHeader();
extern int      gnm_ReadBufferData();
extern void		gnm_SetReadBufferHook();
extern void		gnm_SetAddDataToBufferHook();

#endif /* #if defined(__cplusplus) */

#endif /* _GNET_MESSAGE_MODULE_ */

#endif /* _GNET_MESSAGE_H_ */
