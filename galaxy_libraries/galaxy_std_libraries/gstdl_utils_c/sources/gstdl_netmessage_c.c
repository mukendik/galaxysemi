/*======================================================================*/
/* Galaxy                                                               */
/* Copyright 1996-2001, Softlink                                        */
/* This computer program is protected by copyrigt law                   */
/* and international treaties. Unauthorized reproduction or             */
/* distribution of this program, or any portion of it,may               */
/* result in severe civil and criminal penalties, and will be           */
/* prosecuted to the maximum extent possible under the low.             */
/*======================================================================*/

#define _GNET_MESSAGE_MODULE_

/*======================================================================*/
/*======================== GNETMESSAGE.C  LIBRARY ======================*/
/*======================================================================*/
#include <stdio.h>
#include <string.h>
#if defined __APPLE__&__MACH__
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif
#include <stdlib.h>
#include "gstdl_netmessage_c.h"

/*======================================================================*/
/*====================== Private functions =============================*/
/*======================================================================*/
#ifdef __STDC__
    static int (*gnm_fhReadBufferHook)(unsigned int  uiType, PT_GNB_HBUFFER phBuffer, void **ptptData) = NULL;
    static int (*gnm_fhAddDataToBufferHook)(unsigned int  uiType, PT_GNB_HBUFFER phBuffer, void *ptData) = NULL;
#else
    static int (*gnm_fhReadBufferHook)() = NULL;
    static int (*gnm_fhAddDataToBufferHook)() = NULL;
#endif

/*======================================================================*/
/* Create buffer from Header structure                                  */
/*                                                                      */
/* Parameter :                                                          */
/*          ptstHeader    : pointer on Header structure.                */
/*          ptptBuffer    : pointer on pointer on buffer to create.     */
/* Return :                                                             */
/*          GNET_ERR_OKAY if successfull                                */
/*          Error code from gnetbuffer module else                      */
/*======================================================================*/
int gnm_CreateBufferHeader(ptstHeader, ptptBuffer)
PT_GNM_HEADER ptstHeader;
char          **ptptBuffer;
{
	GNB_HBUFFER     hBuffer;
	int             nStatus;
	
	/* Open Buffer */
	nStatus = gnb_Open(&hBuffer);
	if(nStatus != GNET_ERR_OKAY)
	{
		return nStatus;
	}
	
	/* Add Data to buffer */
    gnb_AddDWord(&hBuffer, (GNB_DWORD)ptstHeader->mMessageLength);
    gnb_AddDWord(&hBuffer, (GNB_DWORD)ptstHeader->mMessageType);
    gnb_AddByte(&hBuffer, ptstHeader->mAckRequested);
	
	/* Close Buffer */
	gnb_Close(&hBuffer);
	
	/* Set ptptBuffer */
	*ptptBuffer = (char *)hBuffer.ptBuffer;
	
	return GNET_ERR_OKAY;
}

/*======================================================================*/
/* Read Header structure from buffer                                    */
/*                                                                      */
/* Parameter :                                                          */
/*          ptstHeader    : pointer on Header structure.                */
/*          ptBuffer      : pointer on buffer to use.                   */
/* Return :                                                             */
/*          GNET_ERR_OKAY if successfull                                 */
/*          Error code from gnetbuffer module else                      */
/*======================================================================*/
int gnm_ReadBufferHeader(ptstHeader, ptBuffer)
PT_GNM_HEADER ptstHeader;
char          *ptBuffer;
{
	GNB_HBUFFER     hBuffer;
	int             nStatus;
	GNB_DWORD       dwData;
    GNB_BYTE        bData;
	
	/* Init Buffer */
	nStatus = gnb_Init(&hBuffer, ptBuffer, GNM_HEADER_SIZE);
	if(nStatus != GNET_ERR_OKAY)
	{
		return nStatus;
	}
	
	/* Read Data from buffer */
	gnb_ReadDWord(&hBuffer, &dwData);
    ptstHeader->mMessageLength = (unsigned int)dwData;
	gnb_ReadDWord(&hBuffer, &dwData);
    ptstHeader->mMessageType = (unsigned int)dwData;
    gnb_ReadByte(&hBuffer, &bData);
    ptstHeader->mAckRequested = bData;

	return GNET_ERR_OKAY;
}

#ifdef __STDC__
/*======================================================================*/
/* Set the callback function use to read user buffer.					*/
/*																		*/
/* Parameter :															*/
/*			fhReadBufferHook : Pointer to the callback function.		*/
/*======================================================================*/
void gnm_SetReadBufferHook(fhReadBufferHook)
int (*fhReadBufferHook)(unsigned int  uiType, PT_GNB_HBUFFER phBuffer, void **ptptData);
{
	gnm_fhReadBufferHook = fhReadBufferHook;
}

/*======================================================================*/
/* Set the callback function use to create user buffer.					*/
/*																		*/
/* Parameter :															*/
/*			fhAddDataToBufferHook : Pointer to the callback function.	*/
/*======================================================================*/
void gnm_SetAddDataToBufferHook(fhAddDataToBufferHook)
int (*fhAddDataToBufferHook)(unsigned int  uiType, PT_GNB_HBUFFER phBuffer, void *ptData);
{
	gnm_fhAddDataToBufferHook = fhAddDataToBufferHook;
}
#else
/*======================================================================*/
/* Set the callback function use to read user buffer.					*/
/*																		*/
/* Parameter :															*/
/*			fhReadBufferHook : Pointer to the callback function.		*/
/*======================================================================*/
void gnm_SetReadBufferHook(fhReadBufferHook)
int (*fhReadBufferHook)();
{
	gnm_fhReadBufferHook = fhReadBufferHook;
}

/*======================================================================*/
/* Set the callback function use to create user buffer.					*/
/*																		*/
/* Parameter :															*/
/*			fhAddDataToBufferHook : Pointer to the callback function.	*/
/*======================================================================*/
void gnm_SetAddDataToBufferHook(fhAddDataToBufferHook)
int (*fhAddDataToBufferHook)();
{
	gnm_fhAddDataToBufferHook = fhAddDataToBufferHook;
}
#endif

/*======================================================================*/
/* Create buffer from Data structure                                    */
/*                                                                      */
/* Parameter :                                                          */
/*          nType         : type of data structure                      */
/*          ptData        : pointer on Data structure.                  */
/*          ptptBuffer    : pointer on pointer on buffer to create.     */
/*          ptuiBufSize   : pointer on unsigned int to store the size   */
/*                          of the created buffer                       */
/* Return :                                                             */
/*          GNET_ERR_OKAY if successfull                                 */
/*          Error code from gnetbuffer module else                      */
/*======================================================================*/
int gnm_CreateBufferData(uiType ,ptData, ptptBuffer, ptuiBufSize)
unsigned int  uiType;
void          *ptData;
char          **ptptBuffer;
unsigned int  *ptuiBufSize;
{
	GNB_HBUFFER     hBuffer;
	int             nStatus;
	
	/* Open Buffer */
	nStatus = gnb_Open(&hBuffer);
	if(nStatus != GNET_ERR_OKAY)
	{
		return nStatus;
	}
	/* Make sure the callback function has been set */
	if(gnm_fhAddDataToBufferHook == NULL)
	{
		(void)gnb_Close(&hBuffer);
		return GNET_ERR_NCF;
	}
	/* Call the callback function to write Data to Buffer */
	nStatus = gnm_fhAddDataToBufferHook(uiType,&hBuffer,ptData);
	if(nStatus != GNET_ERR_OKAY)
	{
		(void)gnb_Close(&hBuffer);
		return nStatus;
	}
	
	/* Close Buffer */
	nStatus = gnb_Close(&hBuffer);
	if(nStatus != GNET_ERR_OKAY)
		return nStatus;
	
	/* Set ptptBuffer and ptuiBufSize */
	gnb_GetBufferPointer(&hBuffer, ptptBuffer);
	gnb_GetBufferSize(&hBuffer, ptuiBufSize);
	
	return GNET_ERR_OKAY;
}

/*======================================================================*/
/* Read Data structure from buffer                                      */
/*                                                                      */
/* Parameter :                                                          */
/*          uiType        : message type            .                   */
/*          ptptData      : pointer on pointer on void, which will      */
/*                          point on the received data structure.	    */
/*          ptBuffer      : pointer on buffer to use.                   */
/*          uiBufferSize  : buffer size             .                   */
/* Return :                                                             */
/*          GNET_ERR_OKAY if successfull							    */
/*          Error code from gnetbuffer module else                      */
/*======================================================================*/
int gnm_ReadBufferData(uiType, ptptData, ptBuffer, uiBufferSize)
unsigned int  uiType;
void          **ptptData;
char          *ptBuffer;
unsigned int  uiBufferSize;
{
	GNB_HBUFFER     hBuffer;
	int             nStatus;
	
	/* Init Buffer */
	nStatus = gnb_Init(&hBuffer, ptBuffer, uiBufferSize);
	if(nStatus != GNET_ERR_OKAY)
	{
		return nStatus;
	}
	/* Make sure the callback function has been set */
	if(gnm_fhReadBufferHook == NULL)
	{
		(void)gnb_Close(&hBuffer);
		return GNET_ERR_NCF;
	}

	/* Call the call-back function to write Data to Buffer */
	return gnm_fhReadBufferHook(uiType,&hBuffer,ptptData);
}

/*======================================================================*/
/* Create buffer from Header and Data structures                        */
/*                                                                      */
/* Parameter :                                                          */
/*          Type         : type of data structure                       */
/*          AckRequested : set to 1 if an Acknowledge is requested      */
/*          Data         : pointer on Data structure.                   */
/*          Buffer       : pointer on pointer on buffer to create.      */
/*          BufSize      : pointer on unsigned int to store the size    */
/*                          of the created buffer                       */
/* Return :                                                             */
/*          GNET_ERR_OKAY if successfull                                */
/*          Error code from gnetbuffer module else                      */
/*======================================================================*/
int gnm_CreateBufferHeaderData(Type, AckRequested, Data, Buffer, BufSize)
unsigned int    Type;
unsigned short  AckRequested;
void *          Data;
char **         Buffer;
unsigned int *  BufSize;
{
	GNB_HBUFFER     hBuffer;
	int             nStatus;
	
	/* Open Buffer */
	nStatus = gnb_Open(&hBuffer);
	if(nStatus != GNET_ERR_OKAY)
	{
		return nStatus;
	}

	/* Make sure the callback function has been set */
	if(gnm_fhAddDataToBufferHook == NULL)
	{
		(void)gnb_Close(&hBuffer);
		return GNET_ERR_NCF;
	}

	/* Add dummy Header data to buffer: the real size will be known only when the data has been added */
	gnb_AddDWord(&hBuffer, (GNB_DWORD)0);
	gnb_AddDWord(&hBuffer, (GNB_DWORD)0);
    gnb_AddByte(&hBuffer, (GNB_BYTE)0);

	/* Call the callback function to write Data to Buffer */
    nStatus = gnm_fhAddDataToBufferHook(Type,&hBuffer,Data);
	if(nStatus != GNET_ERR_OKAY)
	{
		(void)gnb_Close(&hBuffer);
		return nStatus;
	}
	
	/* Close Buffer */
	nStatus = gnb_Close(&hBuffer);
	if(nStatus != GNET_ERR_OKAY)
		return nStatus;

    /* Set Buffer and BufSize */
    gnb_GetBufferPointer(&hBuffer, Buffer);
    gnb_GetBufferSize(&hBuffer, BufSize);

	/* Re-open buffer */
    gnb_ReOpen(&hBuffer, *Buffer, *BufSize);
	
	/* Add real Header data to buffer */
    gnb_AddDWord(&hBuffer, (GNB_DWORD)(*BufSize - GNM_HEADER_SIZE));
    gnb_AddDWord(&hBuffer, (GNB_DWORD)Type);
    gnb_AddByte(&hBuffer, (GNB_BYTE)AckRequested);

	/* Close Buffer */
	nStatus = gnb_Close(&hBuffer);
	if(nStatus != GNET_ERR_OKAY)
		return nStatus;
	
	return GNET_ERR_OKAY;
}

