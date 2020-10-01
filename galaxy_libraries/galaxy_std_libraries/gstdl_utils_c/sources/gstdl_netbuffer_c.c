/*======================================================================*/
/* Galaxy                                                               */
/* Copyright 1996-99, Softlink                                          */
/* This computer program is protected by copyrigt law                   */
/* and international treaties. Unauthorized reproduction or             */
/* distribution of this program, or any portion of it,may               */
/* result in severe civil and criminal penalties, and will be           */
/* prosecuted to the maximum extent possible under the low.             */
/*======================================================================*/

#define _GNET_BUFFER_MODULE_

/*======================================================================*/
/*======================== GNETBUFFER.C  LIBRARY =======================*/
/*======================================================================*/
#include <stdio.h>
#include <string.h>
#if defined __APPLE__&__MACH__
    #include <malloc/malloc.h>
#else
    #include <malloc.h>
#endif
#include <stdlib.h>
#include "gstdl_netbuffer_c.h"

/*======================================================================*/
/* PRIVATE Variables : declaration                                      */
/*======================================================================*/
/*======================================================================*/
/* PRIVATE Functions : declaration                                      */
/*======================================================================*/
static int gnb_IncreaseBuffer(PT_GNB_HBUFFER, unsigned int);
static int gnb_AdjustBuffer(PT_GNB_HBUFFER);

/*======================================================================*/
/* PUBLIC Functions                                                     */
/*======================================================================*/
/*======================================================================*/
/* Init a buffer handler.                                               */
/*                                                                      */
/* Parameter :                                                          */
/*          pthBuffer     : pointer on buffer handler.                  */
/*          ptBuffer      : pointer on buffer to use.                   */
/*          uiBufferSize  : size of buffer to use.                      */
/* Return :                                                             */
/*          GNET_ERR_OKAY.                                              */
/*======================================================================*/
int gnb_Init(PT_GNB_HBUFFER pthBuffer, char* ptBuffer, unsigned int uiBufferSize)
{
  /* Init pthBuffer members */
  pthBuffer->ptBuffer = ptBuffer;
  pthBuffer->uiBufferEnd = uiBufferSize;
  pthBuffer->uiAllocatedSize = uiBufferSize;
  pthBuffer->uiBufferPos = 0;
  pthBuffer->uiBufferOpened = 0;

  return GNET_ERR_OKAY;
}

/*======================================================================*/
/* Open a new buffer handler for writing.                               */
/* (Allocates memory for the buffer).                                   */
/*                                                                      */
/* Parameter :                                                          */
/*          pthBuffer     : pointer on buffer handler.                  */
/* Return :                                                             */
/*          GNET_ERR_OKAY if successfull.                               */
/*          GNET_ERR_MAF : memory allocation failure                    */
/*======================================================================*/
int gnb_Open(PT_GNB_HBUFFER pthBuffer)
{
  char* ptBuffer=0;

  /* Create new buffer and init handler */
  ptBuffer = (char*)calloc(GNB_BUFFER_BLOCKSIZE,sizeof(char));
  if(!ptBuffer)
    return GNET_ERR_MAF;

  pthBuffer->ptBuffer = ptBuffer;
  pthBuffer->uiAllocatedSize = GNB_BUFFER_BLOCKSIZE;
  pthBuffer->uiBufferEnd = 0;
  pthBuffer->uiBufferPos = 0;
  pthBuffer->uiBufferOpened = 1;

  return GNET_ERR_OKAY;
}

/*======================================================================*/
/* ReOpen a buffer handler.                                             */
/*                                                                      */
/* Parameter :                                                          */
/*          pthBuffer     : pointer on buffer handler.                  */
/*          ptBuffer      : pointer on buffer to use.                   */
/*          uiBufferSize  : size of buffer to use.                      */
/* Return :                                                             */
/*          GNET_ERR_OKAY.                                              */
/*======================================================================*/
int gnb_ReOpen(PT_GNB_HBUFFER pthBuffer, char* ptBuffer, unsigned int uiBufferSize)
{
  /* Init pthBuffer members */
  pthBuffer->ptBuffer = ptBuffer;
  pthBuffer->uiBufferEnd = uiBufferSize;
  pthBuffer->uiAllocatedSize = uiBufferSize;
  pthBuffer->uiBufferPos = 0;
  pthBuffer->uiBufferOpened = 1;

  return GNET_ERR_OKAY;
}

/*======================================================================*/
/* Seek to position in buffer.                                          */
/*                                                                      */
/* Parameter :                                                          */
/*          pthBuffer     : pointer on buffer handler.                  */
/*          uiSeekTo      : GNB_SEEK_BEGIN (seek to beginning of buffer)*/
/*          uiSeekTo      : GNB_SEEK_END (seek to end of buffer)        */
/* Return :                                                             */
/*          GNET_ERR_OKAY if successfull.                               */
/*======================================================================*/
int gnb_Seek(PT_GNB_HBUFFER pthBuffer, unsigned int uiSeekTo)
{
  if(uiSeekTo == GNB_SEEK_BEGIN)
    /* Set buffer pos to 0 */
    pthBuffer->uiBufferPos = 0;
  else if(uiSeekTo == GNB_SEEK_END)
    /* Set buffer pos to end of buffer */
    pthBuffer->uiBufferPos = pthBuffer->uiBufferEnd;

  return GNET_ERR_OKAY;
}

/*======================================================================*/
/* Close a buffer handler.                                              */
/* (Reallocates the exact size for the buffer).                         */
/*                                                                      */
/* Parameter :                                                          */
/*          pthBuffer     : pointer on buffer handler.                  */
/* Return :                                                             */
/*          GNET_ERR_OKAY if successfull.                               */
/*          GNET_ERR_MAF : memory allocation failure                    */
/*          GNET_ERR_BNO : buffer not opened                            */
/*======================================================================*/
int gnb_Close(PT_GNB_HBUFFER pthBuffer)
{
  int nStatus;

  /* Check if buffer opened */
  if(!pthBuffer->uiBufferOpened || !pthBuffer->ptBuffer)
    return GNET_ERR_BNO;

  /* Adjust buffer size */
  nStatus = gnb_AdjustBuffer(pthBuffer);
  if(nStatus != GNET_ERR_OKAY) return nStatus;

  /* Reset handler */
  pthBuffer->uiBufferPos = 0;
  pthBuffer->uiBufferOpened = 0;

  return GNET_ERR_OKAY;
}

/*======================================================================*/
/* Get buffer size       .                                              */
/*                                                                      */
/* Parameter :                                                          */
/*          pthBuffer     : pointer on buffer handler.                  */
/*          ptuiBufferSize: pointer on unsigned int to store buffer size*/
/* Return :                                                             */
/*          GNET_ERR_OKAY if successfull.                               */
/*          GNET_ERR_BNC : buffer not closed                            */
/*======================================================================*/
int gnb_GetBufferSize(PT_GNB_HBUFFER pthBuffer, unsigned int* ptuiBufferSize)
{
  /* Check if buffer closed */
  if(pthBuffer->uiBufferOpened || !pthBuffer->ptBuffer)
    return GNET_ERR_BNC;

  /* Store Buffer size */
  *ptuiBufferSize = pthBuffer->uiAllocatedSize;

  return GNET_ERR_OKAY;
}

/*======================================================================*/
/* Get pointer on buffer .                                              */
/*                                                                      */
/* Parameter :                                                          */
/*          pthBuffer     : pointer on buffer handler.                  */
/*          ptptBuffer    : ptr on ptr to store buffer pointer          */
/* Return :                                                             */
/*          GNET_ERR_OKAY if successfull.                               */
/*          GNET_ERR_BNC : buffer not closed                            */
/*======================================================================*/
int gnb_GetBufferPointer(PT_GNB_HBUFFER pthBuffer, char** ptptBuffer)
{
  /* Check if buffer closed */
  if(pthBuffer->uiBufferOpened || !pthBuffer->ptBuffer)
    return GNET_ERR_BNC;

  /* Store pointer on buffer */
  *ptptBuffer = pthBuffer->ptBuffer;

  return GNET_ERR_OKAY;
}

/*======================================================================*/
/* Add a BYTE to the buffer                                             */
/*                                                                      */
/* Parameter :                                                          */
/*          pthBuffer     : pointer on buffer handler.                  */
/*          bData         : BYTE to be added.                           */
/* Return :                                                             */
/*          GNET_ERR_OKAY if successfull.                               */
/*          GNET_ERR_BNO : buffer not opened                            */
/*======================================================================*/
int gnb_AddByte(PT_GNB_HBUFFER pthBuffer, GNB_BYTE bData)
{
  int nStatus;

  /* Check if buffer opened */
  if(!pthBuffer->uiBufferOpened || !pthBuffer->ptBuffer)
    return GNET_ERR_BNO;

  /* Increase Buffer size if needed */
  if(pthBuffer->uiBufferPos + sizeof(GNB_BYTE) > pthBuffer->uiAllocatedSize)
  {
    nStatus = gnb_IncreaseBuffer(pthBuffer, sizeof(GNB_BYTE));
    if(nStatus != GNET_ERR_OKAY) return nStatus;
  }

  /* Add BYTE */
  *(pthBuffer->ptBuffer + pthBuffer->uiBufferPos) = bData;
  (pthBuffer->uiBufferPos) += sizeof(GNB_BYTE);
  if(pthBuffer->uiBufferEnd < pthBuffer->uiBufferPos)
    pthBuffer->uiBufferEnd = pthBuffer->uiBufferPos;
  
  return GNET_ERR_OKAY;
}

/*======================================================================*/
/* Add a WORD to the buffer                                             */
/*                                                                      */
/* Parameter :                                                          */
/*          pthBuffer     : pointer on buffer handler.                  */
/*          wData         : WORD to be added.                           */
/* Return :                                                             */
/*          GNET_ERR_OKAY if successfull.                               */
/*          GNET_ERR_BNO : buffer not opened                            */
/*======================================================================*/
int gnb_AddWord(PT_GNB_HBUFFER pthBuffer, GNB_WORD wData)
{
  int nStatus;

  /* Check if buffer opened */
  if(!pthBuffer->uiBufferOpened || !pthBuffer->ptBuffer)
    return GNET_ERR_BNO;

  /* Increase Buffer size if needed */
  if(pthBuffer->uiBufferPos + sizeof(GNB_WORD) > pthBuffer->uiAllocatedSize)
  {
    nStatus = gnb_IncreaseBuffer(pthBuffer, sizeof(GNB_WORD));
    if(nStatus != GNET_ERR_OKAY) return nStatus;
  }

  /* Convert WORD */
  gnb_SetWordToBigEndian(&wData);

  /* Add WORD */
  memcpy(pthBuffer->ptBuffer + pthBuffer->uiBufferPos, &wData, sizeof(GNB_WORD));
  (pthBuffer->uiBufferPos) += sizeof(GNB_WORD);
  if(pthBuffer->uiBufferEnd < pthBuffer->uiBufferPos)
    pthBuffer->uiBufferEnd = pthBuffer->uiBufferPos;
  
  return GNET_ERR_OKAY;
}

/*======================================================================*/
/* Add a DWORD to the buffer                                            */
/*                                                                      */
/* Parameter :                                                          */
/*          pthBuffer     : pointer on buffer handler.                  */
/*          dwData        : DWORD to be added.                          */
/* Return :                                                             */
/*          GNET_ERR_OKAY if successfull.                               */
/*          GNET_ERR_BNO : buffer not opened                            */
/*======================================================================*/
int gnb_AddDWord(PT_GNB_HBUFFER pthBuffer, GNB_DWORD dwData)
{
  int nStatus;

  /* Check if buffer opened */
  if(!pthBuffer->uiBufferOpened || !pthBuffer->ptBuffer)
    return GNET_ERR_BNO;

  /* Increase Buffer size if needed */
  if(pthBuffer->uiBufferPos + sizeof(GNB_DWORD) > pthBuffer->uiAllocatedSize)
  {
    nStatus = gnb_IncreaseBuffer(pthBuffer, sizeof(GNB_DWORD));
    if(nStatus != GNET_ERR_OKAY) return nStatus;
  }

  /* Convert DWORD */
  gnb_SetDWordToBigEndian(&dwData);

  /* Add DWORD */
  memcpy(pthBuffer->ptBuffer + pthBuffer->uiBufferPos, &dwData, sizeof(GNB_DWORD));
  (pthBuffer->uiBufferPos) += sizeof(GNB_DWORD);
  if(pthBuffer->uiBufferEnd < pthBuffer->uiBufferPos)
    pthBuffer->uiBufferEnd = pthBuffer->uiBufferPos;
  
  return GNET_ERR_OKAY;
}

/*======================================================================*/
/* Add a INT4 to the buffer                                             */
/*                                                                      */
/* Parameter :                                                          */
/*          pthBuffer     : pointer on buffer handler.                  */
/*          iData         : INT4 to be added.                           */
/* Return :                                                             */
/*          GNET_ERR_OKAY if successfull.                                */
/*          GNET_ERR_BNO : buffer not opened                             */
/*======================================================================*/
int gnb_AddInt4(PT_GNB_HBUFFER pthBuffer, GNB_INT4 iData)
{
  int nStatus;

  /* Check if buffer opened */
  if(!pthBuffer->uiBufferOpened || !pthBuffer->ptBuffer)
    return GNET_ERR_BNO;

  /* Increase Buffer size if needed */
  if(pthBuffer->uiBufferPos + sizeof(GNB_INT4) > pthBuffer->uiAllocatedSize)
  {
    nStatus = gnb_IncreaseBuffer(pthBuffer, sizeof(GNB_INT4));
    if(nStatus != GNET_ERR_OKAY) return nStatus;
  }

  /* Convert INT4 */
  gnb_SetInt4ToBigEndian(&iData);

  /* Add INT4 */
  memcpy(pthBuffer->ptBuffer + pthBuffer->uiBufferPos, &iData, sizeof(GNB_INT4));
  (pthBuffer->uiBufferPos) += sizeof(GNB_INT4);
  if(pthBuffer->uiBufferEnd < pthBuffer->uiBufferPos)
    pthBuffer->uiBufferEnd = pthBuffer->uiBufferPos;
  
  return GNET_ERR_OKAY;
}

/*======================================================================*/
/* Add a DOUBLE to the buffer                                           */
/*                                                                      */
/* Parameter :                                                          */
/*          pthBuffer     : pointer on buffer handler.                  */
/*          dfData        : DOUBLE to be added.                         */
/* Return :                                                             */
/*          GNET_ERR_OKAY if successfull.                                */
/*          GNET_ERR_BNO : buffer not opened                             */
/*======================================================================*/
int gnb_AddDouble(PT_GNB_HBUFFER pthBuffer, GNB_DOUBLE dfData)
{
  int nStatus;

  /* Check if buffer opened */
  if(!pthBuffer->uiBufferOpened || !pthBuffer->ptBuffer)
    return GNET_ERR_BNO;

  /* Increase Buffer size if needed */
  if(pthBuffer->uiBufferPos + sizeof(GNB_DOUBLE) > pthBuffer->uiAllocatedSize)
  {
    nStatus = gnb_IncreaseBuffer(pthBuffer, sizeof(GNB_DOUBLE));
    if(nStatus != GNET_ERR_OKAY) return nStatus;
  }

  /* Convert double */
  gnb_SetDoubleToBigEndian(&dfData);

  /* Add double */
  memcpy(pthBuffer->ptBuffer + pthBuffer->uiBufferPos, &dfData, sizeof(GNB_DOUBLE));
  (pthBuffer->uiBufferPos) += sizeof(GNB_DOUBLE);
  if(pthBuffer->uiBufferEnd < pthBuffer->uiBufferPos)
    pthBuffer->uiBufferEnd = pthBuffer->uiBufferPos;
  
  return GNET_ERR_OKAY;
}

/*======================================================================*/
/* Add a FLOAT to the buffer                                            */
/*                                                                      */
/* Parameter :                                                          */
/*          pthBuffer     : pointer on buffer handler.                  */
/*          fData         : FLOAT to be added.                          */
/* Return :                                                             */
/*          GNET_ERR_OKAY if successfull.                                */
/*          GNET_ERR_BNO : buffer not opened                             */
/*======================================================================*/
int gnb_AddFloat(PT_GNB_HBUFFER pthBuffer, GNB_FLOAT fData)
{
  int nStatus;

  /* Check if buffer opened */
  if(!pthBuffer->uiBufferOpened || !pthBuffer->ptBuffer)
    return GNET_ERR_BNO;

  /* Increase Buffer size if needed */
  if(pthBuffer->uiBufferPos + sizeof(GNB_FLOAT) > pthBuffer->uiAllocatedSize)
  {
    nStatus = gnb_IncreaseBuffer(pthBuffer, sizeof(GNB_FLOAT));
    if(nStatus != GNET_ERR_OKAY) return nStatus;
  }

  /* Convert float */
  gnb_SetFloatToBigEndian(&fData);

  /* Add float */
  memcpy(pthBuffer->ptBuffer + pthBuffer->uiBufferPos, &fData, sizeof(GNB_FLOAT));
  (pthBuffer->uiBufferPos) += sizeof(GNB_FLOAT);
  if(pthBuffer->uiBufferEnd < pthBuffer->uiBufferPos)
    pthBuffer->uiBufferEnd = pthBuffer->uiBufferPos;
  
  return GNET_ERR_OKAY;
}

/*======================================================================*/
/* Add a STRING to the buffer                                           */
/*                                                                      */
/* Parameter :                                                          */
/*          pthBuffer     : pointer on buffer handler.                  */
/*          szString      : STRING to be added.                         */
/* Return :                                                             */
/*          GNET_ERR_OKAY if successfull.                                */
/*          GNET_ERR_BNO : buffer not opened                             */
/*======================================================================*/
int gnb_AddString(PT_GNB_HBUFFER pthBuffer, char* szString)
{
  int   nStatus, nStringLength, nTotalLength, i;

  /* Check if buffer opened */
  if(!pthBuffer->uiBufferOpened || !pthBuffer->ptBuffer)
    return GNET_ERR_BNO;

  /* Sizes */
  // strlen does NOT include the end '\0' char
  nStringLength = strlen(szString);
  nTotalLength = nStringLength + sizeof(GNB_INT4);

  /* Increase Buffer size if needed */
  if(pthBuffer->uiBufferPos + nTotalLength > pthBuffer->uiAllocatedSize)
  {
    nStatus = gnb_IncreaseBuffer(pthBuffer, nTotalLength);
    if(nStatus != GNET_ERR_OKAY) return nStatus;
  }

  /* Write string length */
  nStatus = gnb_AddInt4(pthBuffer, nStringLength);
  if(nStatus != GNET_ERR_OKAY)
      return nStatus; 

  /* Write string characters */
  for(i=0 ; i<nStringLength ; i++)
  {
    *(pthBuffer->ptBuffer + pthBuffer->uiBufferPos) = (GNB_BYTE)szString[i];
    (pthBuffer->uiBufferPos)++;
  }
  if(pthBuffer->uiBufferEnd < pthBuffer->uiBufferPos)
    pthBuffer->uiBufferEnd = pthBuffer->uiBufferPos;

  return GNET_ERR_OKAY; 
}

/*======================================================================*/
/* Read a BYTE from the buffer                                          */
/*                                                                      */
/* Parameter :                                                          */
/*          pthBuffer     : pointer on buffer handler.                  */
/*          pbData        : pointer on BYTE to store read value.        */
/* Return :                                                             */
/*          GNET_ERR_OKAY if successfull.                                */
/*          GNET_ERR_BNC : buffer not closed                             */
/*          GNET_ERR_OOB : out of buffer                                 */
/*======================================================================*/
int gnb_ReadByte(PT_GNB_HBUFFER pthBuffer, PT_GNB_BYTE pbData)
{
  /* Check if buffer closed */
  if(pthBuffer->uiBufferOpened || !pthBuffer->ptBuffer)
    return GNET_ERR_BNC;

  /* Check if not out of allocated buffer */
  if(pthBuffer->uiBufferPos + sizeof(GNB_BYTE) > pthBuffer->uiBufferEnd)
    return GNET_ERR_OOB;

  /* Read Data */
  *pbData = *(pthBuffer->ptBuffer + pthBuffer->uiBufferPos);
  pthBuffer->uiBufferPos += sizeof(GNB_BYTE);

  return GNET_ERR_OKAY;    
}

/*======================================================================*/
/* Read a WORD from the buffer                                          */
/*                                                                      */
/* Parameter :                                                          */
/*          pthBuffer     : pointer on buffer handler.                  */
/*          pwData        : pointer on WORD to store read value.        */
/* Return :                                                             */
/*          GNET_ERR_OKAY if successfull.                                */
/*          GNET_ERR_BNC : buffer not closed                             */
/*          GNET_ERR_OOB : out of buffer                                 */
/*======================================================================*/
int gnb_ReadWord(PT_GNB_HBUFFER pthBuffer, PT_GNB_WORD pwData)
{
  /* Check if buffer closed */
  if(pthBuffer->uiBufferOpened || !pthBuffer->ptBuffer)
    return GNET_ERR_BNC;

  /* Check if not out of allocated buffer */
  if(pthBuffer->uiBufferPos + sizeof(GNB_WORD) > pthBuffer->uiBufferEnd)
    return GNET_ERR_OOB;

  /* Read Data */
  memcpy(pwData, pthBuffer->ptBuffer + pthBuffer->uiBufferPos, sizeof(GNB_WORD));
  pthBuffer->uiBufferPos += sizeof(GNB_WORD);

  /* Convert data */
  gnb_SetBigEndianToWord(pwData);

  return GNET_ERR_OKAY;    
}

/*======================================================================*/
/* Read a DWORD from the buffer                                         */
/*                                                                      */
/* Parameter :                                                          */
/*          pthBuffer     : pointer on buffer handler.                  */
/*          pdwData       : pointer on DWORD to store read value.       */
/* Return :                                                             */
/*          GNET_ERR_OKAY if successfull.                                */
/*          GNET_ERR_BNC : buffer not closed                             */
/*          GNET_ERR_OOB : out of buffer                                 */
/*======================================================================*/
int gnb_ReadDWord(PT_GNB_HBUFFER pthBuffer, PT_GNB_DWORD pdwData)
{
  /* Check if buffer closed */
  if(pthBuffer->uiBufferOpened || !pthBuffer->ptBuffer)
    return GNET_ERR_BNC;

  /* Check if not out of allocated buffer */
  if(pthBuffer->uiBufferPos + sizeof(GNB_DWORD) > pthBuffer->uiBufferEnd)
    return GNET_ERR_OOB;

  /* Read Data */
  memcpy(pdwData, pthBuffer->ptBuffer + pthBuffer->uiBufferPos, sizeof(GNB_DWORD));
  pthBuffer->uiBufferPos += sizeof(GNB_DWORD);

  /* Convert data */
  gnb_SetBigEndianToDWord(pdwData);

  return GNET_ERR_OKAY;    
}

/*======================================================================*/
/* Read a INT4 from the buffer                                          */
/*                                                                      */
/* Parameter :                                                          */
/*          pthBuffer    : pointer on buffer handler.                   */
/*          piData       : pointer on INT4 to store read value.         */
/* Return :                                                             */
/*          GNET_ERR_OKAY if successfull.                                */
/*          GNET_ERR_BNC : buffer not closed                             */
/*          GNET_ERR_OOB : out of buffer                                 */
/*======================================================================*/
int gnb_ReadInt4(PT_GNB_HBUFFER pthBuffer, PT_GNB_INT4 piData)
{
  /* Check if buffer closed */
  if(pthBuffer->uiBufferOpened || !pthBuffer->ptBuffer)
    return GNET_ERR_BNC;

  /* Check if not out of allocated buffer */
  if(pthBuffer->uiBufferPos + sizeof(GNB_INT4) > pthBuffer->uiBufferEnd)
    return GNET_ERR_OOB;

  /* Read Data */
  memcpy(piData, pthBuffer->ptBuffer + pthBuffer->uiBufferPos, sizeof(GNB_INT4));
  pthBuffer->uiBufferPos += sizeof(GNB_INT4);

  /* Convert data */
  gnb_SetBigEndianToInt4(piData);

  return GNET_ERR_OKAY;    
}

/*======================================================================*/
/* Read a DOUBLE from the buffer                                        */
/*                                                                      */
/* Parameter :                                                          */
/*          pthBuffer     : pointer on buffer handler.                  */
/*          pdfData       : pointer on DOUBLE to store read value.      */
/* Return :                                                             */
/*          GNET_ERR_OKAY if successfull.                                */
/*          GNET_ERR_BNC : buffer not closed                             */
/*          GNET_ERR_OOB : out of buffer                                 */
/*======================================================================*/
int gnb_ReadDouble(PT_GNB_HBUFFER pthBuffer, PT_GNB_DOUBLE pdfData)
{
  /* Check if buffer closed */
  if(pthBuffer->uiBufferOpened || !pthBuffer->ptBuffer)
    return GNET_ERR_BNC;

  /* Check if not out of allocated buffer */
  if(pthBuffer->uiBufferPos + sizeof(GNB_DOUBLE) > pthBuffer->uiBufferEnd)
    return GNET_ERR_OOB;

  /* Read Data */
  memcpy(pdfData, pthBuffer->ptBuffer + pthBuffer->uiBufferPos, sizeof(GNB_DOUBLE));
  pthBuffer->uiBufferPos += sizeof(GNB_DOUBLE);

  /* Convert data */
  gnb_SetBigEndianToDouble(pdfData);

  return GNET_ERR_OKAY;    
}

/*======================================================================*/
/* Read a FLOAT from the buffer                                         */
/*                                                                      */
/* Parameter :                                                          */
/*          pthBuffer     : pointer on buffer handler.                  */
/*          pfData        : pointer on FLOAT to store read value.       */
/* Return :                                                             */
/*          GNET_ERR_OKAY if successfull.                                */
/*          GNET_ERR_BNC : buffer not closed                             */
/*          GNET_ERR_OOB : out of buffer                                 */
/*======================================================================*/
int gnb_ReadFloat(PT_GNB_HBUFFER pthBuffer, PT_GNB_FLOAT pfData)
{
  /* Check if buffer closed */
  if(pthBuffer->uiBufferOpened || !pthBuffer->ptBuffer)
    return GNET_ERR_BNC;

  /* Check if not out of allocated buffer */
  if(pthBuffer->uiBufferPos + sizeof(GNB_FLOAT) > pthBuffer->uiBufferEnd)
    return GNET_ERR_OOB;

  /* Read Data */
  memcpy(pfData, pthBuffer->ptBuffer + pthBuffer->uiBufferPos, sizeof(GNB_FLOAT));
  pthBuffer->uiBufferPos += sizeof(GNB_FLOAT);

  /* Convert data */
  gnb_SetBigEndianToFloat(pfData);

  return GNET_ERR_OKAY;    
}

/*======================================================================*/
/* Read a string from the buffer                                        */
/*                                                                      */
/* Parameter :                                                          */
/*          pthBuffer     : pointer on buffer handler.                  */
/*          szString      : pointer on the string to store read value.  */
/* Return :                                                             */
/*          GNET_ERR_OKAY if successfull.                                */
/*          GNET_ERR_BNC : buffer not closed                             */
/*          GNET_ERR_OOB : out of buffer                                 */
/*======================================================================*/
int gnb_ReadString(PT_GNB_HBUFFER pthBuffer, char* szString)
{
  GNB_INT4  nStringLength;
  int       nStatus, i;

  /* Check if buffer closed */
  if(pthBuffer->uiBufferOpened || !pthBuffer->ptBuffer)
    return GNET_ERR_BNC;

  /* Read String length */
  nStatus = gnb_ReadInt4(pthBuffer, &nStringLength);
  if(nStatus != GNET_ERR_OKAY) return nStatus;

  /* Check if not out of allocated buffer */
  if(pthBuffer->uiBufferPos + nStringLength > pthBuffer->uiBufferEnd)
    return GNET_ERR_OOB;

  /* Read Data */
  if(nStringLength == 0)
    *szString = 0;
  else
  {
    for(i=0 ; i<nStringLength ; i++)
    {
      *(szString+i) = (char )*(pthBuffer->ptBuffer + pthBuffer->uiBufferPos);
      (pthBuffer->uiBufferPos)++;
    }
	
    szString[i] = '\0';
  }

  return GNET_ERR_OKAY;    
}

/*======================================================================*/
/* Convert a machine dependant WORD to big endian format.               */
/*                                                                      */
/* Parameter :                                                          */
/*          pwData : A pointer to the WORD to convert.                  */
/* Return :                                                             */
/*======================================================================*/
void gnb_SetWordToBigEndian(PT_GNB_WORD pwData)
{
  GNB_WORD      wBuffer;
  PT_GNB_BYTE   pbData, pbBuffer;
  
  pbData = (PT_GNB_BYTE)pwData;
  pbBuffer = (PT_GNB_BYTE)&wBuffer;

#ifdef _BIGENDIAN
  *(pbBuffer+0) = *(pbData+0);
  *(pbBuffer+1) = *(pbData+1);
#else
  *(pbBuffer+0) = *(pbData+1);
  *(pbBuffer+1) = *(pbData+0);
#endif  

  *pwData = wBuffer;
}

/*======================================================================*/
/* Convert a WORD from big endian format to machine dependant WORD.     */
/*                                                                      */
/* Parameter :                                                          */
/*            pwData : A pointer to the WORD to convert.                */
/* Return :                                                             */
/*======================================================================*/
void gnb_SetBigEndianToWord(PT_GNB_WORD pwData)
{
  GNB_WORD      wBuffer;
  PT_GNB_BYTE   pbData, pbBuffer;
  
  pbData = (PT_GNB_BYTE)pwData;
  pbBuffer = (PT_GNB_BYTE)&wBuffer;

#ifdef _BIGENDIAN
  *(pbBuffer+0) = *(pbData+0);
  *(pbBuffer+1) = *(pbData+1);
#else
  *(pbBuffer+0) = *(pbData+1);
  *(pbBuffer+1) = *(pbData+0);
#endif  

  *pwData = wBuffer;
}

/*======================================================================*/
/* Convert a machine dependant DWORD to big endian format.              */
/*                                                                      */
/* Parameter :                                                          */
/*          pdwData : A pointer to the DWORD to convert.                */
/* Return :                                                             */
/*======================================================================*/
void gnb_SetDWordToBigEndian(PT_GNB_DWORD pdwData)
{
  GNB_DWORD     dwBuffer;
  PT_GNB_BYTE   pbData,pbBuffer;
  
  pbData = (PT_GNB_BYTE)pdwData;
  pbBuffer = (PT_GNB_BYTE)&dwBuffer;

#ifdef _BIGENDIAN
  *(pbBuffer+0) = *(pbData+0);
  *(pbBuffer+1) = *(pbData+1);
  *(pbBuffer+2) = *(pbData+2);
  *(pbBuffer+3) = *(pbData+3);
#else
  *(pbBuffer+0) = *(pbData+3);
  *(pbBuffer+1) = *(pbData+2);
  *(pbBuffer+2) = *(pbData+1);
  *(pbBuffer+3) = *(pbData+0);
#endif  

  *pdwData = dwBuffer;
}

/*======================================================================*/
/* Convert a DWORD from big endian format to machine dependant DWORD.   */
/*                                                                      */
/* Parameter :                                                          */
/*          pdwData : A pointer to the DWORD to convert.                */
/* Return :                                                             */
/*======================================================================*/
void gnb_SetBigEndianToDWord(PT_GNB_DWORD pdwData)
{
  GNB_DWORD     dwBuffer;
  PT_GNB_BYTE   pbData,pbBuffer;
  
  pbData = (PT_GNB_BYTE)pdwData;
  pbBuffer = (PT_GNB_BYTE)&dwBuffer;

#ifdef _BIGENDIAN
  *(pbBuffer+0) = *(pbData+0);
  *(pbBuffer+1) = *(pbData+1);
  *(pbBuffer+2) = *(pbData+2);
  *(pbBuffer+3) = *(pbData+3);
#else
  *(pbBuffer+0) = *(pbData+3);
  *(pbBuffer+1) = *(pbData+2);
  *(pbBuffer+2) = *(pbData+1);
  *(pbBuffer+3) = *(pbData+0);
#endif  

  *pdwData = dwBuffer;
}

/*======================================================================*/
/* Convert a machine dependant INT4 to big endian format.               */
/*                                                                      */
/* Parameter :                                                          */
/*          pnData : A pointer to the INT4 to convert.                  */
/* Return :                                                             */
/*======================================================================*/
void gnb_SetInt4ToBigEndian(PT_GNB_INT4 pnData)
{
  GNB_INT4      nBuffer;
  PT_GNB_BYTE   pbData,pbBuffer;
  
  pbData = (PT_GNB_BYTE)pnData;
  pbBuffer = (PT_GNB_BYTE)&nBuffer;

#ifdef _BIGENDIAN
  *(pbBuffer+0) = *(pbData+0);
  *(pbBuffer+1) = *(pbData+1);
  *(pbBuffer+2) = *(pbData+2);
  *(pbBuffer+3) = *(pbData+3);
#else
  *(pbBuffer+0) = *(pbData+3);
  *(pbBuffer+1) = *(pbData+2);
  *(pbBuffer+2) = *(pbData+1);
  *(pbBuffer+3) = *(pbData+0);
#endif  

  *pnData = nBuffer;
}

/*======================================================================*/
/* Convert a INT4 from big endian format to machine dependant INT4.     */
/*                                                                      */
/* Parameter :                                                          */
/*          pnData : A pointer to the INT4 to convert.                  */
/* Return :                                                             */
/*======================================================================*/
void gnb_SetBigEndianToInt4(PT_GNB_INT4 pnData)
{
  GNB_INT4      nBuffer;
  PT_GNB_BYTE   pbData, pbBuffer;
  
  pbData = (PT_GNB_BYTE)pnData;
  pbBuffer = (PT_GNB_BYTE)&nBuffer;

#ifdef _BIGENDIAN
  *(pbBuffer+0) = *(pbData+0);
  *(pbBuffer+1) = *(pbData+1);
  *(pbBuffer+2) = *(pbData+2);
  *(pbBuffer+3) = *(pbData+3);
#else
  *(pbBuffer+0) = *(pbData+3);
  *(pbBuffer+1) = *(pbData+2);
  *(pbBuffer+2) = *(pbData+1);
  *(pbBuffer+3) = *(pbData+0);
#endif  

  *pnData = nBuffer;
}

/*======================================================================*/
/* Convert a machine dependant double to big endian format.             */
/*                                                                      */
/* Parameter :                                                          */
/*          pdfData : A pointer to the double to convert.               */
/* Return :                                                             */
/*======================================================================*/
void gnb_SetDoubleToBigEndian(PT_GNB_DOUBLE pdfData)
{
  GNB_DOUBLE    dfBuffer;
  PT_GNB_BYTE   pbData,pbBuffer;
  
  pbData = (PT_GNB_BYTE)pdfData;
  pbBuffer = (PT_GNB_BYTE)&dfBuffer;

#ifdef _BIGENDIAN
  *(pbBuffer+0) = *(pbData+0);
  *(pbBuffer+1) = *(pbData+1);
  *(pbBuffer+2) = *(pbData+2);
  *(pbBuffer+3) = *(pbData+3);
  *(pbBuffer+4) = *(pbData+4);
  *(pbBuffer+5) = *(pbData+5);
  *(pbBuffer+6) = *(pbData+6);
  *(pbBuffer+7) = *(pbData+7);
#else
  *(pbBuffer+0) = *(pbData+7);
  *(pbBuffer+1) = *(pbData+6);
  *(pbBuffer+2) = *(pbData+5);
  *(pbBuffer+3) = *(pbData+4);
  *(pbBuffer+4) = *(pbData+3);
  *(pbBuffer+5) = *(pbData+2);
  *(pbBuffer+6) = *(pbData+1);
  *(pbBuffer+7) = *(pbData+0);
#endif  

  *pdfData = dfBuffer;
}

/*======================================================================*/
/* Convert a double from big endian format to machine dependant double. */
/*                                                                      */
/* Parameter :                                                          */
/*          pdfData : A pointer to the double to convert.               */
/* Return :                                                             */
/*======================================================================*/
void gnb_SetBigEndianToDouble(PT_GNB_DOUBLE pdfData)
{
  GNB_DOUBLE    dfBuffer;
  PT_GNB_BYTE   pbData,pbBuffer;
  
  pbData = (PT_GNB_BYTE)pdfData;
  pbBuffer = (PT_GNB_BYTE)&dfBuffer;

#ifdef _BIGENDIAN
  *(pbBuffer+0) = *(pbData+0);
  *(pbBuffer+1) = *(pbData+1);
  *(pbBuffer+2) = *(pbData+2);
  *(pbBuffer+3) = *(pbData+3);
  *(pbBuffer+4) = *(pbData+4);
  *(pbBuffer+5) = *(pbData+5);
  *(pbBuffer+6) = *(pbData+6);
  *(pbBuffer+7) = *(pbData+7);
#else
  *(pbBuffer+0) = *(pbData+7);
  *(pbBuffer+1) = *(pbData+6);
  *(pbBuffer+2) = *(pbData+5);
  *(pbBuffer+3) = *(pbData+4);
  *(pbBuffer+4) = *(pbData+3);
  *(pbBuffer+5) = *(pbData+2);
  *(pbBuffer+6) = *(pbData+1);
  *(pbBuffer+7) = *(pbData+0);
#endif  

  *pdfData = dfBuffer;
}

/*======================================================================*/
/* Convert a machine dependant float to big endian format.              */
/*                                                                      */
/* Parameter :                                                          */
/*          pfData : A pointer to the float to convert.                 */
/* Return :                                                             */
/*======================================================================*/
void gnb_SetFloatToBigEndian(PT_GNB_FLOAT pfData)
{
  GNB_FLOAT     fData = (GNB_FLOAT)(*pfData);
  GNB_FLOAT     fBuffer;
  PT_GNB_BYTE   pbData,pbBuffer;
  
  pbData = (PT_GNB_BYTE)&fData;
  pbBuffer = (PT_GNB_BYTE)&fBuffer;

#ifdef _BIGENDIAN
  *(pbBuffer+0) = *(pbData+0);
  *(pbBuffer+1) = *(pbData+1);
  *(pbBuffer+2) = *(pbData+2);
  *(pbBuffer+3) = *(pbData+3);
#else
  *(pbBuffer+0) = *(pbData+3);
  *(pbBuffer+1) = *(pbData+2);
  *(pbBuffer+2) = *(pbData+1);
  *(pbBuffer+3) = *(pbData+0);
#endif  

  *pfData = fBuffer;
}

/*======================================================================*/
/* Convert a float from big endian format to machine dependant float.   */
/*                                                                      */
/* Parameter :                                                          */
/*          pfData : A pointer to the float to convert.                 */
/* Return :                                                             */
/*======================================================================*/
void gnb_SetBigEndianToFloat(PT_GNB_FLOAT pfData)
{
  GNB_FLOAT		fData = (GNB_FLOAT)(*pfData);
  GNB_FLOAT     fBuffer;
  PT_GNB_BYTE   pbData,pbBuffer;
  
  pbData = (PT_GNB_BYTE)&fData;
  pbBuffer = (PT_GNB_BYTE)&fBuffer;

#ifdef _BIGENDIAN
  *(pbBuffer+0) = *(pbData+0);
  *(pbBuffer+1) = *(pbData+1);
  *(pbBuffer+2) = *(pbData+2);
  *(pbBuffer+3) = *(pbData+3);
#else
  *(pbBuffer+0) = *(pbData+3);
  *(pbBuffer+1) = *(pbData+2);
  *(pbBuffer+2) = *(pbData+1);
  *(pbBuffer+3) = *(pbData+0);
#endif  

  *pfData = fBuffer;
}

/*======================================================================*/
/* PRIVATE Functions                                                    */
/*======================================================================*/
/*======================================================================*/
/* Increase buffer size                                                 */
/* (The content of the buffer is keeped);                               */
/*                                                                      */
/* Parameter :                                                          */
/*          pthBuffer     : pointer on buffer handler.                  */
/*          uiMinSize     : minimum size to increase                    */
/* Return :                                                             */
/*          GNET_ERR_OKAY : no error                                     */
/*          GNET_ERR_MAF : memory allocation failure                     */
/*======================================================================*/
static int gnb_IncreaseBuffer(PT_GNB_HBUFFER pthBuffer, unsigned int uiMinSize)
{ 
  char*           ptNewBuffer;
  unsigned int    uiNewSize, uiIncreaseSize;

  /* Check amount to increase */
  uiIncreaseSize = (uiMinSize > GNB_BUFFER_BLOCKSIZE)?uiMinSize:GNB_BUFFER_BLOCKSIZE;

  /* Calculate new buffer size */
  uiNewSize = pthBuffer->uiAllocatedSize + uiIncreaseSize;

  /* Allocate new Buffer */
  ptNewBuffer = (char*)calloc(uiNewSize,sizeof(char));
  if(!ptNewBuffer)
    return GNET_ERR_MAF; 

  /* The new buffer is ready : copy data */
  if(!memcpy((void *)ptNewBuffer,(void *)pthBuffer->ptBuffer,pthBuffer->uiAllocatedSize))
  {
    free(ptNewBuffer);
    return GNET_ERR_MAF;
  }

  /* Free initial buffer */
  free(pthBuffer->ptBuffer);

  /* Reset pthBuffer settings */
  pthBuffer->ptBuffer = ptNewBuffer;
  pthBuffer->uiAllocatedSize = uiNewSize;

  return GNET_ERR_OKAY;  
}

/*======================================================================*/
/* Adjust buffer to its written size                                    */
/* (The content of the buffer is keeped);                               */
/*                                                                      */
/* Parameter :                                                          */
/*          pthBuffer     : pointer on buffer handler.                  */
/* Return :                                                             */
/*          GNET_ERR_OKAY : no error                                     */
/*          GNET_ERR_MAF : memory allocation failure                     */
/*======================================================================*/
static int gnb_AdjustBuffer(PT_GNB_HBUFFER pthBuffer)
{ 
  char* ptNewBuffer;

  /* If buffer already has correct size, do nothing */
  if(pthBuffer->uiAllocatedSize == pthBuffer->uiBufferEnd)
	  return GNET_ERR_OKAY;

  /* Create new buffer with the exact size of the buffer */
  ptNewBuffer = (char*)calloc(pthBuffer->uiBufferEnd,sizeof(char));
  if(!ptNewBuffer)
    return GNET_ERR_MAF;

  /* Copy buffer to new location */
  if(!memcpy((void*)ptNewBuffer, (void*)pthBuffer->ptBuffer, pthBuffer->uiBufferEnd*sizeof(char)))
  {
    free(ptNewBuffer);
    return GNET_ERR_MAF;
  }

  /* Free initial buffer */
  free(pthBuffer->ptBuffer);
  
  /* Set new buffer in handler */
  pthBuffer->ptBuffer = ptNewBuffer;
  pthBuffer->uiAllocatedSize = pthBuffer->uiBufferEnd;
  
  return GNET_ERR_OKAY;  
}
