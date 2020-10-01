/*======================================================================*/
/* Galaxy 															*/
/* Copyright 1996-98, Softlink											*/
/* This computer program is protected by copyrigt law 				*/
/* and international treaties. Unauthorized reproduction or 			*/
/* distribution of this program, or any portion of it,may 			*/
/* result in severe civil and criminal penalties, and will be 		*/
/* prosecuted to the maximum extent possible under the low. 			*/
/*======================================================================*/

#define _GBinFilesModule_

/*======================================================================*/
/*========================== GBINFILES.C	LIBRARY ======================*/
/*======================================================================*/
#include <stdio.h>
#include <string.h>
#if defined __APPLE__&__MACH__
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif
#include <stdlib.h>
#include "gstdl_binfiles_c.h"

/*======================================================================*/
/* Defines																*/
/*======================================================================*/
#ifndef SEEK_SET				/* Because on sunos4, SEEK_SET not defined */
#define SEEK_SET 0
#endif

#define GBF_STRINGSIZE	 1024  /* Char. buffers can handle 1024 char.	*/

#define GBF_BUF_SIZE	 4000  /* Default size for GBF buffer 			*/

/*======================================================================*/
/* PRIVATE Variables : declaration										*/
/*======================================================================*/

/*======================================================================*/
/* PRIVATE Functions : declaration										*/
/* (cannot use "static" because of itlc compiler) 						*/
/*======================================================================*/
#ifdef ANSI
int gbf_AllocBuffer(PT_GBF_REC ptRecord);
int gbf_ReallocBuffer(PT_GBF_REC ptRecord);
int gbf_LoadOneRecord(FILE *hFile, PT_GBF_REC *ptptRecord);
#else
int 	gbf_AllocBuffer();
int 	gbf_ReallocBuffer();
int 	gbf_LoadOneRecord();
#endif

/*======================================================================*/
/* PUBLIC Functions 													*/
/*======================================================================*/
/*======================================================================*/
/* Returns in buffer the error message for a given error code 			*/
/*																		*/
/* Return : 															*/
/* GBF_ERR_OKAY : no error												*/
/*======================================================================*/
#ifdef ANSI
int gbf_GetErrorMessage(int nErrCode, char *szText, char *szErrorMessage)
#else
int gbf_GetErrorMessage(nErrCode, szText, szErrorMessage)
int 	nErrCode;
char	*szText, *szErrorMessage;
#endif
{
	switch(nErrCode)
	{
	case GBF_ERR_MAF:
		strcpy(szErrorMessage,GBF_STR_ERR_MAF);
		break;

	case GBF_ERR_COF:
		sprintf(szErrorMessage,GBF_STR_ERR_COF,szText);
		break;

	case GBF_ERR_FTS:
		sprintf(szErrorMessage,GBF_STR_ERR_FTS,szText);
		break;

	case GBF_ERR_NFR:
		sprintf(szErrorMessage,GBF_STR_ERR_NFR, szText);
		break;

	case GBF_ERR_UEF:
		sprintf(szErrorMessage,GBF_STR_ERR_UEF, szText);
		break;

	case GBF_ERR_IRT:
		sprintf(szErrorMessage,GBF_STR_ERR_IRT, szText);
		break;

	case GBF_ERR_AOM:
		sprintf(szErrorMessage,GBF_STR_ERR_AOM);
		break;

	case GBF_ERR_RNF:
		sprintf(szErrorMessage,GBF_STR_ERR_RNF, szText);
		break;

	case GBF_ERR_LOGGMFNF:
		sprintf(szErrorMessage,GBF_STR_ERR_LOGGMFNF, szText);
		break;

	case GBF_ERR_FNF:
		sprintf(szErrorMessage,GBF_STR_ERR_FNF, szText);
		break;

	case GBF_ERR_NFL:
		strcpy(szErrorMessage,GBF_STR_ERR_NFL);
		break;

	case GBF_ERR_CWF:
		sprintf(szErrorMessage,GBF_STR_ERR_CWF, szText);
		break;

	case GBF_ERR_NSQ:
		strcpy(szErrorMessage,GBF_STR_ERR_NSQ);
		break;

	case GBF_ERR_MAV:
		strcpy(szErrorMessage,GBF_STR_ERR_MAV);
		break;

	case GBF_ERR_CRF:
		sprintf(szErrorMessage,GBF_STR_ERR_CRF, szText);
		break;

	case GBF_ERR_EBS:
		strcpy(szErrorMessage,GBF_STR_ERR_EBS);
		break;

	case GBF_ERR_FAE:
		sprintf(szErrorMessage,GBF_STR_ERR_FAE, szText);
		break;

	case GBF_ERR_RLE:
		strcpy(szErrorMessage,GBF_STR_ERR_RLE);
		break;

	case GBF_ERR_REN:
		sprintf(szErrorMessage,GBF_STR_ERR_REN, szText);
		break;

	case GBF_ERR_CNF:
		strcpy(szErrorMessage,GBF_STR_ERR_CNF);
		break;

	case GBF_ERR_EMR:
		strcpy(szErrorMessage,GBF_STR_ERR_EMR);
		break;

	case GBF_ERR_BADHANDLE:
		strcpy(szErrorMessage,GBF_STR_ERR_BADHANDLE);
		break;

	case GBF_ERR_CORRUPTEDREC:
		sprintf(szErrorMessage,GBF_STR_ERR_CORRUPTEDREC, szText);
		break;

	case GBF_ERR_COPY:
		sprintf(szErrorMessage,GBF_STR_ERR_COPY, szText);
		break;

	default:
		sprintf(szErrorMessage,GBF_STR_ERR_UNKNOWN, nErrCode);
		break;
	}
	return GBF_ERR_OKAY;
}

/*======================================================================*/
/* Create a record list 												*/
/*																		*/
/* Return : 															*/
/* Pointer on new record list created if success, NULL else 			*/
/*======================================================================*/
#ifdef ANSI
PT_GBF_RECLIST gbf_NewRecList(GBF_WORD wVersion)
#else
PT_GBF_RECLIST gbf_NewRecList(wVersion)
GBF_WORD wVersion;
#endif
{
	PT_GBF_RECLIST	ptNewRecList;
	PT_GBF_REC		ptRecord;

	/* Allocates memory for new record list */
	ptNewRecList = (PT_GBF_RECLIST)malloc(sizeof(GBF_RECLIST));
	if(ptNewRecList == NULL)
	return NULL;

	/* Initialize record list */
	ptNewRecList->ptFirstRecord = NULL;
	ptNewRecList->ptCurRecord = NULL;
	ptNewRecList->ptLastRecord = NULL;

	/* Create FAR record : mandatory */
	ptRecord = gbf_BeginRecord(RECTYPE_FAR);
	if(ptRecord == NULL)
	{
		free(ptNewRecList);
		return NULL;
	}
	/* FAR_VERSION */
	gbf_REC_AddWord(ptRecord, wVersion);

	gbf_EndRecord(ptRecord);
	gbf_AddRecord(ptNewRecList, ptRecord);

	return ptNewRecList;
}

/*======================================================================*/
/* Dekete a record list 												*/
/*																		*/
/* Return : 															*/
/* GBF_ERR_OKAY : no error												*/
/*======================================================================*/
#ifdef ANSI
int gbf_DeleteRecList(PT_GBF_RECLIST *ptptRecList)
#else
int gbf_DeleteRecList(ptptRecList)
PT_GBF_RECLIST *ptptRecList;
#endif
{
	/* Check list pointer */
	if(*ptptRecList == NULL)
	return GBF_ERR_OKAY;

	/* Check if list empty. If not, empty it */
	if((*ptptRecList)->ptFirstRecord != NULL)
	gbf_DeleteRecords(*ptptRecList, RECTYPE_ALL);

	/* Free memory for record list */
	free(*ptptRecList);
	*ptptRecList = NULL;
	return GBF_ERR_OKAY;
}

/*======================================================================*/
/* Begin a new record 													*/
/*																		*/
/* Return : 															*/
/* Pointer on new record created if success, NULL else					*/
/*======================================================================*/
#ifdef ANSI
PT_GBF_REC gbf_BeginRecord(GBF_WORD wRecordType)
#else
PT_GBF_REC gbf_BeginRecord(wRecordType)
GBF_WORD wRecordType;
#endif
{
	PT_GBF_REC	ptNewRecord;
	int 		iStatus;

	/* Allocates memory for new record */
	ptNewRecord = (PT_GBF_REC)malloc(sizeof(GBF_REC));
	if(ptNewRecord == NULL)
	return NULL;

	/* Allocate memory for record buffer */
	ptNewRecord->ptBuffer = NULL;
	iStatus = gbf_AllocBuffer(ptNewRecord);
	if(iStatus != GBF_ERR_OKAY)
	return NULL;

	/* Initialize record */
	ptNewRecord->nBuildOffset = 6;
	ptNewRecord->ptNext = NULL;
	gbf_REC_ChangeWord(ptNewRecord,4,wRecordType);

	return ptNewRecord;
}

/*======================================================================*/
/* Close a record 														*/
/*																		*/
/* Return Codes : 														*/
/* GBF_ERR_OKAY : no error												*/
/* GBF_ERR_MAF : memory allocation failure								*/
/*======================================================================*/
#ifdef ANSI
int gbf_EndRecord(PT_GBF_REC ptRecord)
#else
int gbf_EndRecord(ptRecord)
PT_GBF_REC ptRecord;
#endif
{
	GBF_BYTE	*ptNewBuffer;
	GBF_DWORD dwRecSize;

	/* Check pointer */
	if(ptRecord == NULL)
		return GBF_ERR_MAF;

	/* Update record size field : doens't include REC_LENGTH and REC_TYPE */
	dwRecSize = ptRecord->nBuildOffset-6;
	gbf_REC_ChangeDWord(ptRecord,0,dwRecSize);

	/* Allocate memory for new buffer (with exact size) */
	ptNewBuffer = (GBF_BYTE *)malloc(dwRecSize+6);
	if(ptNewBuffer == NULL)
	return GBF_ERR_MAF;

	/* Copy Data from old to new Buffer */
	memcpy((void *)ptNewBuffer,(void *)ptRecord->ptBuffer,dwRecSize+6);

	/* Free old buffer */
	free(ptRecord->ptBuffer);

	/* Reaffect record buffer */
	ptRecord->ptBuffer = ptNewBuffer;

	return GBF_ERR_OKAY;		/* No Error */
}

/*======================================================================*/
/* Free record															*/
/*																		*/
/* Return : 															*/
/* GBF_ERR_OKAY : no error												*/
/*======================================================================*/
#ifdef ANSI
int gbf_FreeRecord(PT_GBF_REC *ptptRecord)
#else
int gbf_FreeRecord(ptptRecord)
PT_GBF_REC *ptptRecord;
#endif
{
	free((*ptptRecord)->ptBuffer);
	free((*ptptRecord));
	*ptptRecord = NULL;
	return GBF_ERR_OKAY;
}

/*======================================================================*/
/* Add a record to the record structure 								*/
/*																		*/
/* Return Codes : 														*/
/* GBF_ERR_OKAY : no error												*/
/* GBF_ERR_MAF : memory allocation failure								*/
/*======================================================================*/
#ifdef ANSI
int gbf_AddRecord(PT_GBF_RECLIST ptRecList, PT_GBF_REC ptRecord)
#else
int gbf_AddRecord(ptRecList, ptRecord)
PT_GBF_RECLIST	ptRecList;
PT_GBF_REC		ptRecord;
#endif
{
	/* Check pointer */
	if((ptRecList == NULL) || (ptRecord == NULL))
	return GBF_ERR_MAF;

	/* Insert record into record list */
	if(ptRecList->ptFirstRecord == NULL)
	{
	ptRecList->ptFirstRecord = ptRecList->ptLastRecord = ptRecord;
	}
	else
	{
	ptRecList->ptLastRecord->ptNext = ptRecord;
	ptRecList->ptLastRecord = ptRecList->ptLastRecord->ptNext;
	}

	return GBF_ERR_OKAY;		/* No Error */
}

/*======================================================================*/
/* Find the first record in record list matching the given record type	*/
/*																		*/
/* Return Codes : 														*/
/* GBF_ERR_OKAY : record found											*/
/* GBF_ERR_RNF : record not found 										*/
/*======================================================================*/
#ifdef ANSI
int gbf_FindFirstRecord(PT_GBF_RECLIST ptRecList, PT_GBF_REC *ptptRecord, GBF_WORD wRecordType)
#else
int gbf_FindFirstRecord(ptRecList, ptptRecord, wRecordType)
PT_GBF_RECLIST	ptRecList;
PT_GBF_REC		*ptptRecord;
GBF_WORD		wRecordType;
#endif
{
	PT_GBF_REC	ptRecord;
	GBF_WORD	wRecType = 0;

	/* Check pointer */
	if(ptRecList == NULL || ptRecList->ptFirstRecord == NULL)
	return GBF_ERR_RNF;

	/* Point on first record */
	ptRecord = ptRecList->ptFirstRecord;

	/* Scan record list */
	while(ptRecord != NULL)
	{
	gbf_REC_ReadWord(ptRecord, 4, &wRecType);
	if(wRecType == wRecordType)
	{
		/* We have found a record that matches our type. */
		/* We return the pointer...*/
		*ptptRecord = ptRecord;
		ptRecList->ptCurRecord = ptRecord;
			return GBF_ERR_OKAY;
	}

	/* Next record */
	ptRecord = ptRecord->ptNext;
	}

	/* Record not found */
	ptRecList->ptCurRecord = NULL;
	return GBF_ERR_RNF;
}

/*======================================================================*/
/* Find the next record in record list matching the given record type.	*/
/* Function gbf_FindFirstRecord() must have been called at least once	*/
/* before use of this function. 										*/
/*																		*/
/* Return Codes : 														*/
/* GBF_ERR_OKAY : record found											*/
/* GBF_ERR_RNF : record not found 										*/
/*======================================================================*/
#ifdef ANSI
int gbf_FindNextRecord(PT_GBF_RECLIST ptRecList, PT_GBF_REC *ptptRecord, GBF_WORD wRecordType)
#else
int gbf_FindNextRecord(ptRecList, ptptRecord, wRecordType)
PT_GBF_RECLIST	ptRecList;
PT_GBF_REC		*ptptRecord;
GBF_WORD		wRecordType;
#endif
{
	PT_GBF_REC	ptRecord;
	GBF_WORD		wRecType = 0;

	/* Check pointer */
	if(ptRecList == NULL || ptRecList->ptFirstRecord == NULL || ptRecList->ptCurRecord == NULL)
	return GBF_ERR_RNF;

	/* Point on next record */
	ptRecord = ptRecList->ptCurRecord->ptNext;

	/* Scan record list */
	while(ptRecord != NULL)
	{
	gbf_REC_ReadWord(ptRecord, 4, &wRecType);
	if(wRecType == wRecordType)
	{
		/* We have found a record that matches our type. */
		/* We return the pointer...*/
		*ptptRecord = ptRecord;
		ptRecList->ptCurRecord = ptRecord;
			return GBF_ERR_OKAY;
	}

	/* Next record */
	ptRecord = ptRecord->ptNext;
	}

	/* Record not found */
	ptRecList->ptCurRecord = NULL;
	return GBF_ERR_RNF;
}

/*======================================================================*/
/* Delete all records of given type from record list					*/
/* (if type = RECTYPE_ALL : delete all records from list				*/
/*																		*/
/* Return Codes : 														*/
/* GBF_ERR_OKAY : no error												*/
/* GBF_ERR_IRT : invalid record type									*/
/*======================================================================*/
#ifdef ANSI
int gbf_DeleteRecords(PT_GBF_RECLIST ptRecList, GBF_WORD wRecordType)
#else
int gbf_DeleteRecords(ptRecList, wRecordType)
PT_GBF_RECLIST	ptRecList;
GBF_WORD		wRecordType;
#endif
{
	PT_GBF_REC	ptRecord, ptPrevRecord=NULL;
	GBF_WORD		wRecType = 0;

	/* FAR cannot be deleted !! */
	if(wRecordType == RECTYPE_FAR)
	return GBF_ERR_IRT; /* Invalid	record type */

	/* Check pointer */
	if((ptRecList == NULL) || (ptRecList->ptFirstRecord == NULL))
	return GBF_ERR_OKAY;

	/* Scan record list and delete specified records */
	ptRecord = ptRecList->ptFirstRecord;
	while(ptRecord)
	{
	gbf_REC_ReadWord(ptRecord, 4, &wRecType);
	if((wRecType == wRecordType) || (wRecordType == RECTYPE_ALL))
	{
		/* We have found a record that matches our type. */
		if(ptPrevRecord == NULL)
		{
		/* Remove First record of list */
		ptRecList->ptFirstRecord = ptRecord->ptNext;
		free(ptRecord->ptBuffer);
		free(ptRecord);
		ptRecord = ptRecList->ptFirstRecord;
		}
		else
		{
		/* Remove record of list */
		ptPrevRecord->ptNext = ptRecord->ptNext;
		free(ptRecord->ptBuffer);
		free(ptRecord);
		ptRecord = ptPrevRecord->ptNext;
		}
	}
	else
	{
		ptPrevRecord = ptRecord;
		ptRecord = ptRecord->ptNext;
	}
	};

	/* Update pointer on last record */
	ptRecList->ptLastRecord = ptPrevRecord;

	return GBF_ERR_OKAY;		/* No Error */
}

/*======================================================================*/
/* Add a struct to the record 											*/
/*																		*/
/* Return Codes : 														*/
/* GBF_ERR_OKAY : no error												*/
/* GBF_ERR_MAF : memory allocation failure								*/
/*======================================================================*/
#ifdef ANSI
int gbf_REC_AddStruct(PT_GBF_REC ptRecord, GBF_BYTE *ptBuffer, int nStructSize)
#else
int gbf_REC_AddStruct(ptRecord, ptBuffer, nStructSize)
PT_GBF_REC	ptRecord;
GBF_BYTE	*ptBuffer;
int			nStructSize;
#endif
{
	int 	iStatus;

	/* Check pointer */
	if(ptRecord == NULL)
	return GBF_ERR_MAF;

	/* Check if enough space in buffer */
	while((ptRecord->nBuildOffset*sizeof(GBF_BYTE) + nStructSize) >= (GBF_BUF_SIZE*ptRecord->nAllocCount*sizeof(GBF_BYTE)))
	{
		/* Increase buffer size */
		iStatus = gbf_ReallocBufferBySize(ptRecord, nStructSize);
		if(iStatus != GBF_ERR_OKAY)
			return iStatus;
	}

	/* Add Data */
	memcpy(ptRecord->ptBuffer + (ptRecord->nBuildOffset*sizeof(GBF_BYTE)), (void *)ptBuffer, nStructSize);
	ptRecord->nBuildOffset += nStructSize;

	return GBF_ERR_OKAY;		/* No Error */
}

/*======================================================================*/
/* Add a byte to the record 											*/
/*																		*/
/* Return Codes : 														*/
/* GBF_ERR_OKAY : no error												*/
/* GBF_ERR_MAF : memory allocation failure								*/
/*======================================================================*/
#ifdef ANSI
int gbf_REC_AddByte(PT_GBF_REC ptRecord, GBF_BYTE bData)
#else
int gbf_REC_AddByte(ptRecord, bData)
PT_GBF_REC	ptRecord;
GBF_BYTE	bData;
#endif
{
	int 	iStatus;

	/* Check pointer */
	if(ptRecord == NULL)
	return GBF_ERR_MAF;

	/* Check if enough space in buffer */
	if((ptRecord->nBuildOffset+sizeof(GBF_BYTE)) >= (GBF_BUF_SIZE*ptRecord->nAllocCount))
	{
	/* Increase buffer size */
	iStatus = gbf_ReallocBuffer(ptRecord);
	if(iStatus != GBF_ERR_OKAY)
		return iStatus;
	}

	/* Add Data */
	ptRecord->ptBuffer[ptRecord->nBuildOffset] = bData;
	ptRecord->nBuildOffset += sizeof(GBF_BYTE);

	return GBF_ERR_OKAY;		/* No Error */
}

/*======================================================================*/
/* Add a word to the record 											*/
/*																		*/
/* Return Codes : 														*/
/* GBF_ERR_OKAY : no error												*/
/* GBF_ERR_MAF : memory allocation failure								*/
/*======================================================================*/
#ifdef ANSI
int gbf_REC_AddWord(PT_GBF_REC ptRecord, GBF_WORD wData)
#else
int gbf_REC_AddWord(ptRecord, wData)
PT_GBF_REC	ptRecord;
GBF_WORD	wData;
#endif
{
	int 	iStatus;
	GBF_BYTE	bLowByte,bHighByte;

	/* Check pointer */
	if(ptRecord == NULL)
	return GBF_ERR_MAF;

	/* Check if enough space in buffer */
	if((ptRecord->nBuildOffset+sizeof(GBF_WORD)) >= (GBF_BUF_SIZE*ptRecord->nAllocCount))
	{
	/* Increase buffer size */
	iStatus = gbf_ReallocBuffer(ptRecord);
	if(iStatus != GBF_ERR_OKAY)
		return iStatus;
	}

	/* Add Data */
	bLowByte = (GBF_BYTE) (wData & 0xff);
	bHighByte = (GBF_BYTE) ((wData >> 8)& 0xff);

	ptRecord->ptBuffer[ptRecord->nBuildOffset] = bLowByte;
	ptRecord->nBuildOffset += sizeof(GBF_BYTE);
	ptRecord->ptBuffer[ptRecord->nBuildOffset] = bHighByte;
	ptRecord->nBuildOffset += sizeof(GBF_BYTE);

	return GBF_ERR_OKAY;		/* No Error */
}

/*======================================================================*/
/* Add a dword to the record											*/
/*																		*/
/* Return Codes : 														*/
/* GBF_ERR_OKAY : no error												*/
/* GBF_ERR_MAF : memory allocation failure								*/
/*======================================================================*/
#ifdef ANSI
int gbf_REC_AddDWord(PT_GBF_REC ptRecord, GBF_DWORD dwData)
#else
int gbf_REC_AddDWord(ptRecord, dwData)
PT_GBF_REC	ptRecord;
GBF_DWORD dwData;
#endif
{
	int 		iStatus;
	GBF_BYTE	bLowByte0,bLowByte1,bHighByte0,bHighByte1;

	/* Check pointer */
	if(ptRecord == NULL)
	return GBF_ERR_MAF;

	/* Check if enough space in buffer */
	if((ptRecord->nBuildOffset+sizeof(GBF_DWORD)) >= (GBF_BUF_SIZE*ptRecord->nAllocCount))
	{
	/* Increase buffer size */
	iStatus = gbf_ReallocBuffer(ptRecord);
	if(iStatus != GBF_ERR_OKAY)
		return iStatus;
	}

	/* Add Data */
	bLowByte0 = (GBF_BYTE) (dwData & 0xff);
	bLowByte1 = (GBF_BYTE) ((dwData >> 8)& 0xff);
	bHighByte0 = (GBF_BYTE) ((dwData >> 16)& 0xff);
	bHighByte1 = (GBF_BYTE) ((dwData >> 24)& 0xff);

	ptRecord->ptBuffer[ptRecord->nBuildOffset] = bLowByte0;
	ptRecord->nBuildOffset += sizeof(GBF_BYTE);
	ptRecord->ptBuffer[ptRecord->nBuildOffset] = bLowByte1;
	ptRecord->nBuildOffset += sizeof(GBF_BYTE);
	ptRecord->ptBuffer[ptRecord->nBuildOffset] = bHighByte0;
	ptRecord->nBuildOffset += sizeof(GBF_BYTE);
	ptRecord->ptBuffer[ptRecord->nBuildOffset] = bHighByte1;
	ptRecord->nBuildOffset += sizeof(GBF_BYTE);

	return GBF_ERR_OKAY;		/* No Error */
}

/*======================================================================*/
/* Add a int4 to the record 											*/
/*																		*/
/* Return Codes : 														*/
/* GBF_ERR_OKAY : no error												*/
/* GBF_ERR_MAF : memory allocation failure								*/
/*======================================================================*/
#ifdef ANSI
int gbf_REC_AddInt4(PT_GBF_REC ptRecord, GBF_INT4 nData)
#else
int gbf_REC_AddInt4(ptRecord, nData)
PT_GBF_REC	ptRecord;
GBF_INT4	nData;
#endif
{
	int 	iStatus;
	GBF_BYTE	bLowByte0,bLowByte1,bHighByte0,bHighByte1;

	/* Check pointer */
	if(ptRecord == NULL)
	return GBF_ERR_MAF;

	/* Check if enough space in buffer */
	if((ptRecord->nBuildOffset+sizeof(GBF_INT4)) >= (GBF_BUF_SIZE*ptRecord->nAllocCount))
	{
	/* Increase buffer size */
	iStatus = gbf_ReallocBuffer(ptRecord);
	if(iStatus != GBF_ERR_OKAY)
		return iStatus;
	}

	/* Add Data */
	bLowByte0 = (GBF_BYTE) (nData & 0xff);
	bLowByte1 = (GBF_BYTE) ((nData >> 8)& 0xff);
	bHighByte0 = (GBF_BYTE) ((nData >> 16)& 0xff);
	bHighByte1 = (GBF_BYTE) ((nData >> 24)& 0xff);

	ptRecord->ptBuffer[ptRecord->nBuildOffset] = bLowByte0;
	ptRecord->nBuildOffset += sizeof(GBF_BYTE);
	ptRecord->ptBuffer[ptRecord->nBuildOffset] = bLowByte1;
	ptRecord->nBuildOffset += sizeof(GBF_BYTE);
	ptRecord->ptBuffer[ptRecord->nBuildOffset] = bHighByte0;
	ptRecord->nBuildOffset += sizeof(GBF_BYTE);
	ptRecord->ptBuffer[ptRecord->nBuildOffset] = bHighByte1;
	ptRecord->nBuildOffset += sizeof(GBF_BYTE);

	return GBF_ERR_OKAY;		/* No Error */
}

/*======================================================================*/
/* Add a float to the record											*/
/*																		*/
/* Return Codes : 														*/
/* GBF_ERR_OKAY : no error												*/
/* GBF_ERR_MAF : memory allocation failure								*/
/*======================================================================*/
#ifdef ANSI
int gbf_REC_AddFloat(PT_GBF_REC ptRecord, float fData)
#else
int gbf_REC_AddFloat(ptRecord, fData)
PT_GBF_REC	ptRecord;
float 	fData;
#endif
{
	int 	iStatus;
	GBF_BYTE	*ptData;
	float 	fLocal;

	/* Check pointer */
	if(ptRecord == NULL)
	return GBF_ERR_MAF;

	/* Check if enough space in buffer */
	if((ptRecord->nBuildOffset+sizeof(float)) >= (GBF_BUF_SIZE*ptRecord->nAllocCount))
	{
	/* Increase buffer size */
	iStatus = gbf_ReallocBuffer(ptRecord);
	if(iStatus != GBF_ERR_OKAY)
		return iStatus;
	}

	/* Add Data */
	fLocal = (float)fData;
	ptData = (GBF_BYTE *)&fLocal;
#if defined __unix__ || __APPLE__&__MACH__
	/* CPU Type = SUN */
	ptRecord->ptBuffer[ptRecord->nBuildOffset] = ptData[3];
	ptRecord->nBuildOffset += sizeof(GBF_BYTE);
	ptRecord->ptBuffer[ptRecord->nBuildOffset] = ptData[2];
	ptRecord->nBuildOffset += sizeof(GBF_BYTE);
	ptRecord->ptBuffer[ptRecord->nBuildOffset] = ptData[1];
	ptRecord->nBuildOffset += sizeof(GBF_BYTE);
	ptRecord->ptBuffer[ptRecord->nBuildOffset] = ptData[0];
	ptRecord->nBuildOffset += sizeof(GBF_BYTE);
#else
	/* CPU Type = PC,Sun386i */
	ptRecord->ptBuffer[ptRecord->nBuildOffset] = ptData[0];
	ptRecord->nBuildOffset += sizeof(GBF_BYTE);
	ptRecord->ptBuffer[ptRecord->nBuildOffset] = ptData[1];
	ptRecord->nBuildOffset += sizeof(GBF_BYTE);
	ptRecord->ptBuffer[ptRecord->nBuildOffset] = ptData[2];
	ptRecord->nBuildOffset += sizeof(GBF_BYTE);
	ptRecord->ptBuffer[ptRecord->nBuildOffset] = ptData[3];
	ptRecord->nBuildOffset += sizeof(GBF_BYTE);
#endif

	return GBF_ERR_OKAY;		/* No Error */
}

/*======================================================================*/
/* Add a string to the record 											*/
/*																		*/
/* Return Codes : 														*/
/* GBF_ERR_OKAY : no error												*/
/* GBF_ERR_MAF : memory allocation failure								*/
/*======================================================================*/
#ifdef ANSI
int gbf_REC_AddString(PT_GBF_REC ptRecord, char *szString)
#else
int gbf_REC_AddString(ptRecord, szString)
PT_GBF_REC	ptRecord;
char		*szString;
#endif
{
	int 	iStatus, bLength;
	char	*ptChar;

	/* Check pointer */
	if(ptRecord == NULL)
	return GBF_ERR_MAF;

	bLength = strlen(szString);

	/* Check if enough space in buffer */
	while((ptRecord->nBuildOffset*sizeof(GBF_BYTE) + bLength + sizeof(GBF_BYTE)) >= (GBF_BUF_SIZE*ptRecord->nAllocCount*sizeof(GBF_BYTE)))
	{
		/* Increase buffer size */
		iStatus = gbf_ReallocBuffer(ptRecord);
		if(iStatus != GBF_ERR_OKAY)
			return iStatus;
	}

	/* Add Data */
	ptRecord->ptBuffer[ptRecord->nBuildOffset] = (GBF_BYTE) (bLength & 0xff); /* string length. */
	ptRecord->nBuildOffset += sizeof(GBF_BYTE);
	ptChar = szString;
	while(bLength)
	{
		ptRecord->ptBuffer[ptRecord->nBuildOffset] = (GBF_BYTE)*ptChar;
		ptRecord->nBuildOffset += sizeof(GBF_BYTE);
		bLength--;
		ptChar++;
	}

	return GBF_ERR_OKAY;		/* No Error */
}

/*======================================================================*/
/* Add a buffer to the record: max buffer size = 65535					*/
/*																		*/
/* Return Codes : 														*/
/* GBF_ERR_OKAY : no error												*/
/* GBF_ERR_MAF : memory allocation failure								*/
/*======================================================================*/
#ifdef ANSI
int gbf_REC_AddBuffer(PT_GBF_REC ptRecord, char *szBuffer, int nBufferSize)
#else
int gbf_REC_AddBuffer(ptRecord, szBuffer, nBufferSize)
PT_GBF_REC	ptRecord;
char		*szBuffer;
int			nBufferSize;
#endif
{
	int 	iStatus;
	char	*ptChar;

	/* Check pointer */
	if(ptRecord == NULL)
		return GBF_ERR_MAF;

	/* Check buffer size */
	if(nBufferSize <= 0)
		return GBF_ERR_OKAY;

	/* Check if enough space in buffer */
	while((ptRecord->nBuildOffset*sizeof(GBF_BYTE) + nBufferSize + sizeof(GBF_WORD)) >= (GBF_BUF_SIZE*ptRecord->nAllocCount*sizeof(GBF_BYTE)))
	{
		/* Increase buffer size */
		iStatus = gbf_ReallocBufferBySize(ptRecord, nBufferSize + sizeof(GBF_WORD));
		if(iStatus != GBF_ERR_OKAY)
			return iStatus;
	}

	/* Add Data */
	/* First add buffer size : maximum is 65535 */
	gbf_REC_AddWord(ptRecord, (GBF_WORD)(nBufferSize & 0xffff));
	ptChar = szBuffer;
	while(nBufferSize)
	{
		ptRecord->ptBuffer[ptRecord->nBuildOffset] = (GBF_BYTE)*ptChar;
		ptRecord->nBuildOffset += sizeof(GBF_BYTE);
		nBufferSize--;
		ptChar++;
	}

	return GBF_ERR_OKAY;		/* No Error */
}

/*======================================================================*/
/* Change a byte in the record at specified offset						*/
/*																		*/
/* Return Codes : 														*/
/* GBF_ERR_OKAY : no error												*/
/* GBF_ERR_MAF : memory allocation failure								*/
/*======================================================================*/
#ifdef ANSI
int gbf_REC_ChangeByte(PT_GBF_REC ptRecord, int nOffset, GBF_BYTE bData)
#else
int gbf_REC_ChangeByte(ptRecord, nOffset, bData)
PT_GBF_REC	ptRecord;
int 		nOffset;
GBF_BYTE	bData;
#endif
{
	/* Check pointer */
	if(ptRecord == NULL)
	return GBF_ERR_MAF;

	/* Check if offset points inside the buffer */
	if((nOffset < 0) || ((nOffset+sizeof(GBF_BYTE)) > ptRecord->nBuildOffset))
	return GBF_ERR_MAF;

	/* Change Data */
	ptRecord->ptBuffer[nOffset] = bData;

	return GBF_ERR_OKAY;		/* No Error */
}

/*======================================================================*/
/* Change a word in the record at specified offset						*/
/*																		*/
/* Return Codes : 														*/
/* GBF_ERR_OKAY : no error												*/
/* GBF_ERR_MAF : memory allocation failure								*/
/*======================================================================*/
#ifdef ANSI
int gbf_REC_ChangeWord(PT_GBF_REC ptRecord, int nOffset, GBF_WORD wData)
#else
int gbf_REC_ChangeWord(ptRecord, nOffset, wData)
PT_GBF_REC	ptRecord;
int 		nOffset;
GBF_WORD	wData;
#endif
{
	GBF_BYTE	bLowByte,bHighByte;

	/* Check pointer */
	if(ptRecord == NULL)
	return GBF_ERR_MAF;

	/* Check if offset points inside the buffer */
	if((nOffset < 0) || ((nOffset+sizeof(GBF_WORD)) > ptRecord->nBuildOffset))
	return GBF_ERR_MAF;

	/* Change Data */
	bLowByte = (GBF_BYTE) (wData & 0xff);
	bHighByte = (GBF_BYTE) ((wData >> 8)& 0xff);

	ptRecord->ptBuffer[nOffset] = bLowByte;
	nOffset += sizeof(GBF_BYTE);
	ptRecord->ptBuffer[nOffset] = bHighByte;

	return GBF_ERR_OKAY;		/* No Error */
}

/*======================================================================*/
/* Change a dword in the record at specified offset 					*/
/*																		*/
/* Return Codes : 														*/
/* GBF_ERR_OKAY : no error												*/
/* GBF_ERR_MAF : memory allocation failure								*/
/*======================================================================*/
#ifdef ANSI
int gbf_REC_ChangeDWord(PT_GBF_REC ptRecord, int nOffset, GBF_DWORD dwData)
#else
int gbf_REC_ChangeDWord(ptRecord, nOffset, dwData)
PT_GBF_REC	ptRecord;
int 		nOffset;
GBF_DWORD dwData;
#endif
{
	GBF_BYTE	bLowByte0,bHighByte0,bLowByte1,bHighByte1;

	/* Check pointer */
	if(ptRecord == NULL)
	return GBF_ERR_MAF;

	/* Check if offset points inside the buffer */
	if((nOffset < 0) || ((nOffset+sizeof(GBF_DWORD)) > ptRecord->nBuildOffset))
	return GBF_ERR_MAF;

	/* Change Data */
	bLowByte0 = (GBF_BYTE) (dwData & 0xff);
	bLowByte1 = (GBF_BYTE) ((dwData >> 8)& 0xff);
	bHighByte0 = (GBF_BYTE) ((dwData >> 16)& 0xff);
	bHighByte1 = (GBF_BYTE) ((dwData >> 24)& 0xff);

	ptRecord->ptBuffer[nOffset] = bLowByte0;
	nOffset += sizeof(GBF_BYTE);
	ptRecord->ptBuffer[nOffset] = bLowByte1;
	nOffset += sizeof(GBF_BYTE);
	ptRecord->ptBuffer[nOffset] = bHighByte0;
	nOffset += sizeof(GBF_BYTE);
	ptRecord->ptBuffer[nOffset] = bHighByte1;

	return GBF_ERR_OKAY;		/* No Error */
}

/*======================================================================*/
/* Change a int4 in the record at specified offset						*/
/*																		*/
/* Return Codes : 														*/
/* GBF_ERR_OKAY : no error												*/
/* GBF_ERR_MAF : memory allocation failure								*/
/*======================================================================*/
#ifdef ANSI
int gbf_REC_ChangeInt4(PT_GBF_REC ptRecord, int nOffset, GBF_INT4 nData)
#else
int gbf_REC_ChangeInt4(ptRecord, nOffset, nData)
PT_GBF_REC	ptRecord;
int 				nOffset;
GBF_INT4		nData;
#endif
{
	GBF_BYTE	bLowByte0,bHighByte0,bLowByte1,bHighByte1;

	/* Check pointer */
	if(ptRecord == NULL)
		return GBF_ERR_MAF;

	/* Check if offset points inside the buffer */
	if((nOffset < 0) || ((nOffset+sizeof(GBF_INT4)) > ptRecord->nBuildOffset))
		return GBF_ERR_MAF;

	/* Change Data */
	bLowByte0 = (GBF_BYTE) (nData & 0xff);
	bLowByte1 = (GBF_BYTE) ((nData >> 8)& 0xff);
	bHighByte0 = (GBF_BYTE) ((nData >> 16)& 0xff);
	bHighByte1 = (GBF_BYTE) ((nData >> 24)& 0xff);

	ptRecord->ptBuffer[nOffset] = bLowByte0;
	nOffset += sizeof(GBF_BYTE);
	ptRecord->ptBuffer[nOffset] = bLowByte1;
	nOffset += sizeof(GBF_BYTE);
	ptRecord->ptBuffer[nOffset] = bHighByte0;
	nOffset += sizeof(GBF_BYTE);
	ptRecord->ptBuffer[nOffset] = bHighByte1;

	return GBF_ERR_OKAY;		/* No Error */
}

/*======================================================================*/
/* Change a float in the record at specified offset 					*/
/*																		*/
/* Return Codes : 														*/
/* GBF_ERR_OKAY : no error												*/
/* GBF_ERR_MAF : memory allocation failure								*/
/*======================================================================*/
#ifdef ANSI
int gbf_REC_ChangeFloat(PT_GBF_REC ptRecord, int nOffset, float fData)
#else
int gbf_REC_ChangeFloat(ptRecord, nOffset, fData)
PT_GBF_REC	ptRecord;
int 				nOffset;
float 			fData;
#endif
{
	GBF_BYTE	*ptData;
	float 		fLocal;

	/* Check pointer */
	if(ptRecord == NULL)
		return GBF_ERR_MAF;

	/* Check if offset points inside the buffer */
	if((nOffset < 0) || ((nOffset+sizeof(float)) > ptRecord->nBuildOffset))
		return GBF_ERR_MAF;

	/* Change Data */
	fLocal = (float)fData;
	ptData = (GBF_BYTE *)&fLocal;
#if defined __unix__ || __APPLE__&__MACH__
	/* CPU Type = SUN */
	ptRecord->ptBuffer[nOffset] = ptData[3];
	nOffset += sizeof(GBF_BYTE);
	ptRecord->ptBuffer[nOffset] = ptData[2];
	nOffset += sizeof(GBF_BYTE);
	ptRecord->ptBuffer[nOffset] = ptData[1];
	nOffset += sizeof(GBF_BYTE);
	ptRecord->ptBuffer[nOffset] = ptData[0];
	nOffset += sizeof(GBF_BYTE);
#else
	/* CPU Type = PC,Sun386i */
	ptRecord->ptBuffer[nOffset] = ptData[0];
	nOffset += sizeof(GBF_BYTE);
	ptRecord->ptBuffer[nOffset] = ptData[1];
	nOffset += sizeof(GBF_BYTE);
	ptRecord->ptBuffer[nOffset] = ptData[2];
	nOffset += sizeof(GBF_BYTE);
	ptRecord->ptBuffer[nOffset] = ptData[3];
	nOffset += sizeof(GBF_BYTE);
#endif

	return GBF_ERR_OKAY;		/* No Error */
}

/*======================================================================*/
/* Read a struct in the record at specified offset						*/
/*																		*/
/* Return Codes : 														*/
/* GBF_ERR_OKAY : no error												*/
/* GBF_ERR_MAF : memory allocation failure								*/
/*======================================================================*/
#ifdef ANSI
int gbf_REC_ReadStruct(PT_GBF_REC ptRecord, int nOffset, GBF_BYTE *ptBuffer, int nStructSize)
#else
int gbf_REC_ReadStruct(ptRecord, nOffset, ptBuffer, nStructSize)
PT_GBF_REC	ptRecord;
int 		nOffset;
GBF_BYTE	*ptBuffer;
int			nStructSize;
#endif
{
	/* Check pointer */
	if(ptRecord == NULL)
		return GBF_ERR_MAF;

	/* Check if offset points inside the buffer */
	if((nOffset < 0) || ((nOffset+nStructSize) > (int)ptRecord->nBuildOffset))
		return GBF_ERR_MAF;

	/* Read Data */
	memcpy(ptBuffer, ptRecord->ptBuffer + (nOffset*sizeof(GBF_BYTE)), nStructSize);

	return GBF_ERR_OKAY;		/* No Error */
}

/*======================================================================*/
/* Read a byte in the record at specified offset						*/
/*																		*/
/* Return Codes : 														*/
/* GBF_ERR_OKAY : no error												*/
/* GBF_ERR_MAF : memory allocation failure								*/
/*======================================================================*/
#ifdef ANSI
int gbf_REC_ReadByte(PT_GBF_REC ptRecord, int nOffset, GBF_BYTE *bData)
#else
int gbf_REC_ReadByte(ptRecord, nOffset, bData)
PT_GBF_REC	ptRecord;
int 				nOffset;
GBF_BYTE		*bData;
#endif
{
	/* Check pointer */
	if(ptRecord == NULL)
		return GBF_ERR_MAF;

	/* Check if offset points inside the buffer */
	if((nOffset < 0) || ((nOffset+sizeof(GBF_BYTE)) > ptRecord->nBuildOffset))
		return GBF_ERR_MAF;

	/* Read Data */
	*bData = ptRecord->ptBuffer[nOffset];

	return GBF_ERR_OKAY;		/* No Error */
}

/*======================================================================*/
/* Read a word in the record at specified offset						*/
/*																		*/
/* Return Codes : 														*/
/* GBF_ERR_OKAY : no error												*/
/* GBF_ERR_MAF : memory allocation failure								*/
/*======================================================================*/
#ifdef ANSI
int gbf_REC_ReadWord(PT_GBF_REC ptRecord, int nOffset, GBF_WORD *wData)
#else
int gbf_REC_ReadWord(ptRecord, nOffset, wData)
PT_GBF_REC	ptRecord;
int 		nOffset;
GBF_WORD	*wData;
#endif
{
	GBF_BYTE	bLowByte,bHighByte;

	/* Check pointer */
	if(ptRecord == NULL)
		return GBF_ERR_MAF;

	/* Check if offset points inside the buffer */
	if((nOffset < 0) || ((nOffset+sizeof(GBF_WORD)) > ptRecord->nBuildOffset))
		return GBF_ERR_MAF;

	/* Read Data */
	bLowByte = ptRecord->ptBuffer[nOffset];
	nOffset += sizeof(GBF_BYTE);
	bHighByte = ptRecord->ptBuffer[nOffset];

	*wData = (GBF_WORD)((bHighByte << 8) + bLowByte);

	return GBF_ERR_OKAY;		/* No Error */
}

/*======================================================================*/
/* Read a dword in the record at specified offset 						*/
/*																		*/
/* Return Codes : 														*/
/* GBF_ERR_OKAY : no error												*/
/* GBF_ERR_MAF : memory allocation failure								*/
/*======================================================================*/
#ifdef ANSI
int gbf_REC_ReadDWord(PT_GBF_REC ptRecord, int nOffset, GBF_DWORD *dwData)
#else
int gbf_REC_ReadDWord(ptRecord, nOffset, dwData)
PT_GBF_REC	ptRecord;
int 				nOffset;
GBF_DWORD 	*dwData;
#endif
{
	GBF_BYTE	bLowByte0,bHighByte0,bLowByte1,bHighByte1;

	/* Check pointer */
	if(ptRecord == NULL)
		return GBF_ERR_MAF;

	/* Check if offset points inside the buffer */
	if((nOffset < 0) || ((nOffset+sizeof(GBF_DWORD)) > ptRecord->nBuildOffset))
		return GBF_ERR_MAF;

	/* Read Data */
	bLowByte0 = ptRecord->ptBuffer[nOffset];
	nOffset += sizeof(GBF_BYTE);
	bLowByte1 = ptRecord->ptBuffer[nOffset];
	nOffset += sizeof(GBF_BYTE);
	bHighByte0 = ptRecord->ptBuffer[nOffset];
	nOffset += sizeof(GBF_BYTE);
	bHighByte1 = ptRecord->ptBuffer[nOffset];

	*dwData = (GBF_DWORD)((bHighByte1 << 24) + (bHighByte0 << 16) + (bLowByte1 << 8) + bLowByte0);

	return GBF_ERR_OKAY;		/* No Error */
}

/*======================================================================*/
/* Read a int4 in the record at specified offset						*/
/*																		*/
/* Return Codes : 														*/
/* GBF_ERR_OKAY : no error												*/
/* GBF_ERR_MAF : memory allocation failure								*/
/*======================================================================*/
#ifdef ANSI
int gbf_REC_ReadInt4(PT_GBF_REC ptRecord, int nOffset, GBF_INT4 *nData)
#else
int gbf_REC_ReadInt4(ptRecord, nOffset, nData)
PT_GBF_REC	ptRecord;
int 				nOffset;
GBF_INT4		*nData;
#endif
{
	GBF_BYTE	bLowByte0,bHighByte0,bLowByte1,bHighByte1;

	/* Check pointer */
	if(ptRecord == NULL)
		return GBF_ERR_MAF;

	/* Check if offset points inside the buffer */
	if((nOffset < 0) || ((nOffset+sizeof(GBF_INT4)) > ptRecord->nBuildOffset))
		return GBF_ERR_MAF;

	/* Read Data */
	bLowByte0 = ptRecord->ptBuffer[nOffset];
	nOffset += sizeof(GBF_BYTE);
	bLowByte1 = ptRecord->ptBuffer[nOffset];
	nOffset += sizeof(GBF_BYTE);
	bHighByte0 = ptRecord->ptBuffer[nOffset];
	nOffset += sizeof(GBF_BYTE);
	bHighByte1 = ptRecord->ptBuffer[nOffset];

	*nData = (GBF_DWORD)((bHighByte1 << 24) + (bHighByte0 << 16) + (bLowByte1 << 8) + bLowByte0);

	return GBF_ERR_OKAY;		/* No Error */
}

/*======================================================================*/
/* Read a float in the record at specified offset 						*/
/*																		*/
/* Return Codes : 														*/
/* GBF_ERR_OKAY : no error												*/
/* GBF_ERR_MAF : memory allocation failure								*/
/*======================================================================*/
#ifdef ANSI
int gbf_REC_ReadFloat(PT_GBF_REC ptRecord, int nOffset, float *fData)
#else
int gbf_REC_ReadFloat(ptRecord, nOffset, fData)
PT_GBF_REC	ptRecord;
int 				nOffset;
float 			*fData;
#endif
{
	GBF_BYTE	*ptData;
	float 		fLocal;

	/* Check pointer */
	if(ptRecord == NULL)
		return GBF_ERR_MAF;

	/* Check if offset points inside the buffer */
	if((nOffset < 0) || ((nOffset+sizeof(float)) > ptRecord->nBuildOffset))
		return GBF_ERR_MAF;

	/* Read Data */
	ptData = (GBF_BYTE *)&fLocal;
#if defined __unix__ || __APPLE__&__MACH__
	/* CPU Type = SUN */
	ptData[3] = ptRecord->ptBuffer[nOffset];
	nOffset += sizeof(GBF_BYTE);
	ptData[2] = ptRecord->ptBuffer[nOffset];
	nOffset += sizeof(GBF_BYTE);
	ptData[1] = ptRecord->ptBuffer[nOffset];
	nOffset += sizeof(GBF_BYTE);
	ptData[0] = ptRecord->ptBuffer[nOffset];
	nOffset += sizeof(GBF_BYTE);
#else
	/* CPU Type = PC,Sun386i */
	ptData[0] = ptRecord->ptBuffer[nOffset];
	nOffset += sizeof(GBF_BYTE);
	ptData[1] = ptRecord->ptBuffer[nOffset];
	nOffset += sizeof(GBF_BYTE);
	ptData[2] = ptRecord->ptBuffer[nOffset];
	nOffset += sizeof(GBF_BYTE);
	ptData[3] = ptRecord->ptBuffer[nOffset];
	nOffset += sizeof(GBF_BYTE);
#endif

	*fData = fLocal;
	return GBF_ERR_OKAY;		/* No Error */
}

/*======================================================================*/
/* Read a string in the record at specified offset						*/
/*																		*/
/* Return Codes : 														*/
/* GBF_ERR_OKAY : no error												*/
/* GBF_ERR_MAF : memory allocation failure								*/
/*======================================================================*/
#ifdef ANSI
int gbf_REC_ReadString(PT_GBF_REC ptRecord, int nOffset, char* szString)
#else
int gbf_REC_ReadString(ptRecord, nOffset, szString)
PT_GBF_REC	ptRecord;
int 				nOffset;
char				*szString;
#endif
{
	int iLength, iIndex;

	/* Check pointer */
	if(ptRecord == NULL)
		return GBF_ERR_MAF;

	/* Check if offset points inside the buffer */
	if((nOffset < 0) || (nOffset >= (int)ptRecord->nBuildOffset))
		return GBF_ERR_MAF;
	iLength = (int)ptRecord->ptBuffer[nOffset];
	nOffset += sizeof(GBF_BYTE);
	if(nOffset+iLength > (int)ptRecord->nBuildOffset)
		return GBF_ERR_MAF;

	/* Read Data */
	if(iLength == 0)
	{
		*szString = 0;
	}
	else
	{
		for(iIndex=0;iIndex<iLength;iIndex++)
			szString[iIndex] = (char)ptRecord->ptBuffer[nOffset+iIndex];
		szString[iIndex] = '\0';
	}

	return GBF_ERR_OKAY;		/* No Error */
}

/*======================================================================*/
/* Read a buffer in the record at specified offset						*/
/*																		*/
/* Return Codes : 														*/
/* GBF_ERR_OKAY : no error												*/
/* GBF_ERR_CORRUPTEDREC: corrupted record								*/
/* GBF_ERR_MAF : memory allocation failure								*/
/*======================================================================*/
#ifdef ANSI
int gbf_REC_ReadBuffer(PT_GBF_REC ptRecord, int nOffset, char **ppcBuffer, int *pnBufferSize)
#else
int gbf_REC_ReadBuffer(ptRecord, nOffset, ppcBuffer, pnBufferSize)
PT_GBF_REC	ptRecord;
int 		nOffset;
char		**ppcBuffer;
int			*pnBufferSize;
#endif
{
	int			iLength, iIndex;
	GBF_WORD	wData = 0;

	*ppcBuffer = NULL;
	*pnBufferSize = 0;

	/* Check pointer */
	if(ptRecord == NULL)
		return GBF_ERR_MAF;

	/* Check if offset points inside the buffer */
	if((nOffset < 0) || (nOffset >= (int)ptRecord->nBuildOffset))
		return GBF_ERR_MAF;

	/* Read buffer size */
	gbf_REC_ReadWord(ptRecord, nOffset, &wData);
	iLength = (int)wData;
	nOffset += sizeof(GBF_WORD);

	/* Check size */
	if(iLength <= 0)
		return GBF_ERR_CORRUPTEDREC;

	/* Check if buffer length not corrupted: still inside the buffer */
	if(nOffset+iLength > (int)ptRecord->nBuildOffset)
		return GBF_ERR_MAF;

	/* Malloc reception buffer */
	*ppcBuffer = (char *)malloc(iLength*sizeof(char));
	if(*ppcBuffer == NULL)
		return GBF_ERR_MAF;

	/* Read Data */
	for(iIndex=0;iIndex<iLength;iIndex++)
		(*ppcBuffer)[iIndex] = (char)ptRecord->ptBuffer[nOffset+iIndex];
	*pnBufferSize = iLength;

	return GBF_ERR_OKAY;		/* No Error */
}

/*======================================================================*/
/* Read file and load all records in memory 							*/
/*																		*/
/* Return Codes : 														*/
/* GBF_ERR_OKAY : no error												*/
/* GBF_ERR_MAF : memory allocation failure								*/
/* GBF_ERR_COF : can't open file										*/
/* GBF_ERR_FTS : file too small 										*/
/* GBF_ERR_NFR : no FAR record											*/
/*======================================================================*/
#ifdef ANSI
int gbf_LoadFromFile(char *szFileName, PT_GBF_RECLIST *ptptRecList)
#else
int gbf_LoadFromFile(szFileName, ptptRecList)
char						*szFileName;
PT_GBF_RECLIST	*ptptRecList;
#endif
{
	FILE				*hFile;
	PT_GBF_REC	ptRecord;
	GBF_BYTE		szBuffer[7];
	GBF_WORD		wRecType;
	int 				iRecordNumber;
	int 				iStatus;

	iRecordNumber= 0;

	/* Open file */
	hFile=fopen(szFileName,"rb");

	if(hFile == NULL)
	{
		*ptptRecList = NULL;
		return GBF_ERR_COF;  /* Cannot open File, return null pointer.*/
	}

	/* Check if first record is FAR record */
	if(fread(szBuffer,1,7,hFile) < 7)
	{
		fclose(hFile);
		*ptptRecList = NULL;
		return GBF_ERR_FTS;  /* file too small */
	}

	wRecType = (GBF_WORD)((szBuffer[5] << 8) + szBuffer[4]);
	if(wRecType != RECTYPE_FAR)
	{
		fclose(hFile);
		*ptptRecList = NULL;
		return GBF_ERR_NFR; /* invalid record : Not FAR Record */
	}

	/* Allocate memory for Record List */
	*ptptRecList = (PT_GBF_RECLIST)malloc(sizeof(GBF_RECLIST));
	if(*ptptRecList == NULL)
	{
		fclose(hFile);
		return GBF_ERR_MAF;
	}

	/* Initialize record list */
	(*ptptRecList)->ptFirstRecord = NULL;
	(*ptptRecList)->ptCurRecord = NULL;
	(*ptptRecList)->ptLastRecord = NULL;

	/* Rewind file to the begining */
	rewind(hFile);

	/* Load pointed record. */
	iStatus = gbf_LoadOneRecord(hFile, &ptRecord);
	while((ptRecord) && (iStatus == GBF_ERR_OKAY))
	{
		/* Number of Records read. */
		iRecordNumber++;

		/* Add record to record list */
		gbf_AddRecord(*ptptRecList, ptRecord);

		/* Load Next Record */
		iStatus = gbf_LoadOneRecord(hFile, &ptRecord);
	};

	/* Check status */
	if(iStatus != GBF_ERR_OKAY)
	{
		/* Cleanup */
		fclose(hFile);
		gbf_DeleteRecList(ptptRecList);
		return iStatus;
	}

	/* No errors */
	fclose(hFile);
	return GBF_ERR_OKAY;

}

/*======================================================================*/
/* Read file and load header records in memory 							*/
/*																		*/
/* Return Codes : 														*/
/* GBF_ERR_OKAY : no error												*/
/* GBF_ERR_MAF : memory allocation failure								*/
/* GBF_ERR_COF : can't open file										*/
/* GBF_ERR_FTS : file too small 										*/
/* GBF_ERR_NFR : no FAR record											*/
/*======================================================================*/
#ifdef ANSI
int gbf_LoadHeaderFromFile(char *szFileName, PT_GBF_RECLIST *ptptRecList, int *ptnPosInFile)
#else
int gbf_LoadHeaderFromFile(szFileName, ptptRecList, ptnPosInFile)
char			*szFileName;
PT_GBF_RECLIST	*ptptRecList;
int				*ptnPosInFile;
#endif
{
	FILE				*hFile;
	PT_GBF_REC	ptRecord;
	GBF_BYTE		szBuffer[7];
	GBF_WORD		wRecType;
	int 				iRecordNumber;
	int 				iStatus;

	iRecordNumber= 0;

	/* Open file */
	hFile=fopen(szFileName,"rb");

	if(hFile == NULL)
	{
		*ptptRecList = NULL;
		return GBF_ERR_COF;  /* Cannot open File, return null pointer.*/
	}

	/* Check if first record is FAR record */
	if(fread(szBuffer,1,7,hFile) < 7)
	{
		fclose(hFile);
		*ptptRecList = NULL;
		return GBF_ERR_FTS;  /* file too small */
	}

	wRecType = (GBF_WORD)((szBuffer[5] << 8) + szBuffer[4]);
	if(wRecType != RECTYPE_FAR)
	{
		fclose(hFile);
		*ptptRecList = NULL;
		return GBF_ERR_NFR; /* invalid record : Not FAR Record */
	}

	/* Allocate memory for Record List */
	*ptptRecList = (PT_GBF_RECLIST)malloc(sizeof(GBF_RECLIST));
	if(*ptptRecList == NULL)
	{
		fclose(hFile);
		return GBF_ERR_MAF;
	}

	/* Initialize record list */
	(*ptptRecList)->ptFirstRecord = NULL;
	(*ptptRecList)->ptCurRecord = NULL;
	(*ptptRecList)->ptLastRecord = NULL;

	/* Rewind file to the begining */
	rewind(hFile);

	/* Load pointed record. */
	iStatus = gbf_LoadOneRecord(hFile, &ptRecord);
	/* Number of Records read. */
	iRecordNumber++;

	/* Add record to record list */
	gbf_AddRecord(*ptptRecList, ptRecord);

	/* Load pointed record. */
	iStatus = gbf_LoadOneRecord(hFile, &ptRecord);
	/* Number of Records read. */
	iRecordNumber++;

	/* Add record to record list */
	gbf_AddRecord(*ptptRecList, ptRecord);

	/* Check status */
	if(iStatus != GBF_ERR_OKAY)
	{
		/* Cleanup */
		fclose(hFile);
		gbf_DeleteRecList(ptptRecList);
		return iStatus;
	}

	/* No errors */
	*ptnPosInFile = ftell(hFile);
	fclose(hFile);
	return GBF_ERR_OKAY;

}

/*======================================================================*/
/* Read file and load next records in memory 							*/
/*																		*/
/* Return Codes : 														*/
/* GBF_ERR_OKAY : no error												*/
/* GBF_ERR_MAF : memory allocation failure								*/
/* GBF_ERR_COF : can't open file										*/
/* GBF_ERR_FTS : file too small 										*/
/* GBF_ERR_NFR : no FAR record											*/
/*======================================================================*/
#ifdef ANSI
int gbf_LoadNextRecordFromFile(char *szFileName, PT_GBF_RECLIST *ptptRecList, int *ptnPosInFile, int nNbRecordToLoad)
#else
int gbf_LoadNextRecordFromFile(szFileName, ptptRecList, ptnPosInFile, nNbRecordToLoad)
char			*szFileName;
PT_GBF_RECLIST	*ptptRecList;
int				*ptnPosInFile, nNbRecordToLoad;
#endif
{
	FILE				*hFile;
	PT_GBF_REC	ptRecord;
	GBF_BYTE		szBuffer[7];
	GBF_WORD		wRecType;
	int 				iRecordNumber;
	int 				iStatus;

	iRecordNumber= 0;

	/* Open file */
	hFile=fopen(szFileName,"rb");

	if(hFile == NULL)
	{
		*ptptRecList = NULL;
		return GBF_ERR_COF;  /* Cannot open File, return null pointer.*/
	}

	/* Check if first record is FAR record */
	if(fread(szBuffer,1,7,hFile) < 7)
	{
		fclose(hFile);
		*ptptRecList = NULL;
		return GBF_ERR_FTS;  /* file too small */
	}

	wRecType = (GBF_WORD)((szBuffer[5] << 8) + szBuffer[4]);
	if(wRecType != RECTYPE_FAR)
	{
		fclose(hFile);
		*ptptRecList = NULL;
		return GBF_ERR_NFR; /* invalid record : Not FAR Record */
	}

	/* Allocate memory for Record List */
	*ptptRecList = (PT_GBF_RECLIST)malloc(sizeof(GBF_RECLIST));
	if(*ptptRecList == NULL)
	{
		fclose(hFile);
		return GBF_ERR_MAF;
	}

	/* Initialize record list */
	(*ptptRecList)->ptFirstRecord = NULL;
	(*ptptRecList)->ptCurRecord = NULL;
	(*ptptRecList)->ptLastRecord = NULL;

	/* Rewind file to the good pos */
	fseek(hFile, *ptnPosInFile, SEEK_SET);

	/* Load pointed record. */
	iStatus = gbf_LoadOneRecord(hFile, &ptRecord);
	while((ptRecord) && (iStatus == GBF_ERR_OKAY) && (iRecordNumber < nNbRecordToLoad))
	{
		/* Number of Records read. */
		iRecordNumber++;
		*ptnPosInFile = ftell(hFile);

		/* Add record to record list */
		gbf_AddRecord(*ptptRecList, ptRecord);

		/* Load Next Record */
		iStatus = gbf_LoadOneRecord(hFile, &ptRecord);
	};

	if(ptRecord)
	{
		/* free the last load not saved*/
		free(ptRecord->ptBuffer);
		free(ptRecord);
		ptRecord = NULL;
	}

	/* Check status */
	if(iStatus != GBF_ERR_OKAY)
	{
		/* Cleanup */
		fclose(hFile);
		gbf_DeleteRecList(ptptRecList);
		return iStatus;
	}

	/* No errors */
	fclose(hFile);
	return GBF_ERR_OKAY;

}

/*======================================================================*/
/* Write all records to a file											*/
/*																		*/
/* Return Codes : 														*/
/* GBF_ERR_OKAY : no error												*/
/* GBF_ERR_RLE : record list empty										*/
/* GBF_ERR_COF : can't open file										*/
/* GBF_ERR_FAE : file already exists									*/
/*======================================================================*/
#ifdef ANSI
int gbf_SaveToFile(char *szFileName, PT_GBF_RECLIST ptRecList, int bOverWrite)
#else
int gbf_SaveToFile(szFileName, ptRecList, bOverWrite)
char			*szFileName;
PT_GBF_RECLIST	ptRecList;
int 			bOverWrite;
#endif
{
	FILE		*hFile;
	PT_GBF_REC	ptRecord;
	GBF_BYTE	*ptBuffer;
	GBF_DWORD 	dwRecordSize;
  int iWritten;
  //int nPos;

	/* Check pointer */
	if(ptRecList == NULL || ptRecList->ptFirstRecord == NULL)
		return GBF_ERR_RLE;

	/* Check if file exists */
	if(!bOverWrite)
	{
		hFile=fopen(szFileName,"r");
		if(hFile != NULL)
		{
			fclose(hFile);
			return GBF_ERR_FAE;  /* File Already Exists. */
		}
	}

	/* Open file */
	hFile=fopen(szFileName,"wb");
	if(hFile == NULL)
		return GBF_ERR_COF;  /* Cannot open File. */

	/* Pointing first GBF record record */
	ptRecord = ptRecList->ptFirstRecord;

	while(ptRecord != NULL)
	{
		/* Pointer to TIF record */
		ptBuffer = ptRecord->ptBuffer;

		dwRecordSize	= (((GBF_WORD)ptBuffer[0]) & 0xff) + ((((GBF_WORD)ptBuffer[1]) << 8) & 0xff00);
		dwRecordSize += ((((GBF_WORD)ptBuffer[2]) << 16) & 0xff0000) + ((((GBF_WORD)ptBuffer[3]) << 24) & 0xff000000);

		/* Write GBF header (6 bytes) + GBF Record to disk. */
		iWritten = fwrite(ptBuffer,dwRecordSize+6,1,hFile);
  if (iWritten < 1) {
    fclose(hFile);
    return GBF_ERR_CORRUPTEDREC;
  }

		/* Move to next GBF record in memory */
		ptRecord = ptRecord->ptNext;
	};

	/* No errors */
	//nPos= ftell(hFile);
	fclose(hFile);
	return GBF_ERR_OKAY;
}

/*======================================================================*/
/* Append all records to a file											*/
/*																		*/
/* Return Codes : 														*/
/* GBF_ERR_OKAY : no error												*/
/* GBF_ERR_RLE : record list empty										*/
/* GBF_ERR_COF : can't open file										*/
/* GBF_ERR_FAE : file already exists									*/
/*======================================================================*/
#ifdef ANSI
int gbf_AppendToFile(char *szFileName, PT_GBF_RECLIST ptRecList)
#else
int gbf_AppendToFile(szFileName, ptRecList)
char			*szFileName;
PT_GBF_RECLIST	ptRecList;
#endif
{
	FILE		*hFile;
	PT_GBF_REC	ptRecord;
	GBF_BYTE	*ptBuffer;
	GBF_DWORD 	dwRecordSize;
  int iWritten;
  //int nPos;

	/* Check pointer */
	if(ptRecList == NULL || ptRecList->ptFirstRecord == NULL)
		return GBF_ERR_RLE;

	/* Check if file exists */
	hFile=fopen(szFileName,"r");
	if(hFile == NULL)
	{
		return GBF_ERR_COF;  /* File Doesn't Exists. */
	}
	fclose(hFile);

	/* Open file */
	hFile=fopen(szFileName,"a+b");
	if(hFile == NULL)
		return GBF_ERR_COF;  /* Cannot open File. */

	//nPos= ftell(hFile);
	/* Pointing first GBF record record */
	ptRecord = ptRecList->ptFirstRecord;

	while(ptRecord != NULL)
	{
		/* Pointer to TIF record */
		ptBuffer = ptRecord->ptBuffer;

		dwRecordSize	= (((GBF_WORD)ptBuffer[0]) & 0xff) + ((((GBF_WORD)ptBuffer[1]) << 8) & 0xff00);
		dwRecordSize += ((((GBF_WORD)ptBuffer[2]) << 16) & 0xff0000) + ((((GBF_WORD)ptBuffer[3]) << 24) & 0xff000000);

		/* Write GBF header (6 bytes) + GBF Record to disk. */
		iWritten = fwrite(ptBuffer,dwRecordSize+6,1,hFile);
  if (iWritten < 1) {
    fclose(hFile);
    return GBF_ERR_CORRUPTEDREC;
  }

		/* Move to next GBF record in memory */
		ptRecord = ptRecord->ptNext;
	};

	/* No errors */
	//nPos= ftell(hFile);
	fclose(hFile);
	return GBF_ERR_OKAY;
}

/*======================================================================*/
/* Rename a file														*/
/*																		*/
/* Return Codes : 														*/
/* GBF_ERR_OKAY : no error												*/
/* GBF_ERR_REN : couldn't renam the file								*/
/*======================================================================*/
#ifdef ANSI
int gbf_RenameFile(char *szOldFileName, char *szNewFileName)
#else
int gbf_RenameFile(szOldFileName, szNewFileName)
char	*szOldFileName, *szNewFileName;
#endif
{
	/* Rename file */
	if(rename(szOldFileName, szNewFileName) != 0)
		return GBF_ERR_REN;

	/* Success */
	return GBF_ERR_OKAY;
}

/*======================================================================*/
/* PRIVATE Functions																										*/
/*======================================================================*/
/*======================================================================*/
/* Allocate Record Buffer 												*/
/*																		*/
/* Return Codes : 														*/
/* GBF_ERR_OKAY : no error												*/
/* GBF_ERR_MAF : memory allocation failure								*/
/*======================================================================*/
#ifdef ANSI
int gbf_AllocBuffer(PT_GBF_REC ptRecord)
#else
int gbf_AllocBuffer(ptRecord)
PT_GBF_REC	ptRecord;
#endif
{
	if(ptRecord->ptBuffer != NULL)
	{
		/* We already have some space allocated : free previous one */
		free(ptRecord->ptBuffer);
		ptRecord->ptBuffer = NULL;
	}

	ptRecord->ptBuffer = (GBF_BYTE *)malloc(GBF_BUF_SIZE);
	if(ptRecord->ptBuffer == NULL)
		return GBF_ERR_MAF; /* Memory allocation failure */

	ptRecord->nAllocCount = 1;
	return GBF_ERR_OKAY;	/* No error. */
}

/*======================================================================*/
/* Reallocate Record Buffer 											*/
/*																		*/
/* Return Codes : 														*/
/* GBF_ERR_OKAY : no error												*/
/* GBF_ERR_MAF : memory allocation failure								*/
/*======================================================================*/
#ifdef ANSI
int gbf_ReallocBuffer(PT_GBF_REC ptRecord)
#else
int gbf_ReallocBuffer(ptRecord)
PT_GBF_REC ptRecord;
#endif
{
	GBF_BYTE		*ptNewBuffer;
	unsigned int	nNewSize;

	/* Calculate new buffer size */
	nNewSize = (++ptRecord->nAllocCount)*GBF_BUF_SIZE;

	/* Allocate new Buffer */
	ptNewBuffer = (GBF_BYTE*)malloc(nNewSize);
	if(ptNewBuffer == NULL)
		return GBF_ERR_MAF; /* Memory allocation failure */

	/* The new buffer is ready : copy data */
	memcpy((void *)ptNewBuffer,(void *)ptRecord->ptBuffer,nNewSize-GBF_BUF_SIZE);

	/* Free old buffer */
	free(ptRecord->ptBuffer);

	/* Reaffect record buffer */
	ptRecord->ptBuffer = ptNewBuffer;

	return GBF_ERR_OKAY;	/* No error. */
}

/*======================================================================*/
/* Reallocate Record Buffer 											*/
/*																		*/
/* Return Codes : 														*/
/* GBF_ERR_OKAY : no error												*/
/* GBF_ERR_MAF : memory allocation failure								*/
/*======================================================================*/
#ifdef ANSI
int gbf_ReallocBufferBySize(PT_GBF_REC ptRecord, int nNewSize)
#else
int gbf_ReallocBufferBySize(ptRecord, nNewSize)
PT_GBF_REC	ptRecord;
int			nNewSize;
#endif
{
	GBF_BYTE		*ptNewBuffer;
	int				div_result;

	/* Calculate new buffer size */
	div_result = nNewSize/GBF_BUF_SIZE;

	/* Allocate new Buffer */
	ptNewBuffer = (GBF_BYTE*)malloc((ptRecord->nAllocCount + div_result + 1)*GBF_BUF_SIZE);
	if(ptNewBuffer == NULL)
		return GBF_ERR_MAF; /* Memory allocation failure */

	/* The new buffer is ready : copy data */
	memcpy((void *)ptNewBuffer,(void *)ptRecord->ptBuffer, ptRecord->nAllocCount*GBF_BUF_SIZE);
	ptRecord->nAllocCount += div_result + 1;

	/* Free old buffer */
	free(ptRecord->ptBuffer);

	/* Reaffect record buffer */
	ptRecord->ptBuffer = ptNewBuffer;

	return GBF_ERR_OKAY;	/* No error. */
}

/*======================================================================*/
/* Read one record from the file, load it in memory.					*/
/*																		*/
/* Return Codes : pointer on buffer (NULL if EOF or error)				*/
/* GBF_ERR_OKAY : no error												*/
/* GBF_ERR_MAF : memory allocation failure								*/
/* GBF_ERR_UEF : Unexpected End Of File 								*/
/*======================================================================*/
#ifdef ANSI
int gbf_LoadOneRecord(FILE *hFile, PT_GBF_REC *ptptRecord)
#else
int gbf_LoadOneRecord(hFile, ptptRecord)
FILE		*hFile;
PT_GBF_REC	*ptptRecord;
#endif
{
	char				szBuffer[6];
	int 				iIndex, iRead, nAllocCount, iDiv;
	GBF_DWORD 	dwRecordSize;
	PT_GBF_REC	ptNewRecord;

	/* Read first 6 bytes of record from disk/network = Record Header */
	if(fread(szBuffer,1,6,hFile) < 6)
	{
		*ptptRecord = NULL;
		return GBF_ERR_OKAY;	/* End Of File. */
	}

	dwRecordSize	= (((GBF_WORD)szBuffer[0]) & 0xff) + ((((GBF_WORD)szBuffer[1]) << 8) & 0xff00);
	dwRecordSize += ((((GBF_WORD)szBuffer[2]) << 16) & 0xff0000) + ((((GBF_WORD)szBuffer[3]) << 24) & 0xff000000);

	/* Calculate nAllocCount */
	iDiv = ((int)(dwRecordSize+6))/GBF_BUF_SIZE;
	if((iDiv*GBF_BUF_SIZE) == (int)(dwRecordSize+6))
		nAllocCount = iDiv;
	else
		nAllocCount = iDiv+1;

	/* Allocate memory for new record */
	ptNewRecord = (PT_GBF_REC)malloc(sizeof(GBF_REC));
	if(ptNewRecord == NULL)
	{
		*ptptRecord = NULL;
		return GBF_ERR_MAF;  /* Memory Allocation Failure */
	}

	/* Allocates buffer to save the whole Header+record. */
	ptNewRecord->ptBuffer = (GBF_BYTE *)malloc(nAllocCount*GBF_BUF_SIZE);
	if(ptNewRecord->ptBuffer == NULL)
	{
		free(ptNewRecord);
		*ptptRecord = NULL;
		return GBF_ERR_MAF;  /* Memory Allocation Failure */
	}

	/* Copy Header data in memory buffer. */
	for(iIndex=0;iIndex<6;iIndex++)
		ptNewRecord->ptBuffer[iIndex] = szBuffer[iIndex];

	/* Read rest of record from disk/network */
	iRead = fread(&(ptNewRecord->ptBuffer[6]),1,dwRecordSize,hFile);
	if((GBF_DWORD)iRead != dwRecordSize)
	{
		free(ptNewRecord->ptBuffer);
		free(ptNewRecord);
		*ptptRecord = NULL;
		return GBF_ERR_UEF; /* Unexpected End Of File. */
	}

	/* Initialize rest of record */
	ptNewRecord->nBuildOffset = dwRecordSize + 6;
	ptNewRecord->nAllocCount = nAllocCount;
	ptNewRecord->ptNext = NULL;

	*ptptRecord = ptNewRecord;
	return GBF_ERR_OKAY;	/* Record read. */
}

