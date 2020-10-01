/****************************************************************************************/
/* Copyright Galaxy                                                                     */
/* This computer program is protected by copyright law                                  */
/* and international treaties. Unauthorized reproduction or                             */
/* distribution of this program, or any portion of it,may                               */
/* result in severe civil and criminal penalties, and will be                           */
/* prosecuted to the maximum extent possible under the law.                             */
/****************************************************************************************/
/*                                                                                      */
/* Module: gtc_netmessage.c                                                             */
/* Write structures used to communicate between GTL (Galaxy Tester Library) and GTM     */
/* (Galaxy Tester Monitor) to a buffer (in network byte order) for transfer through     */
/* a socket.                                                                            */
/*                                                                                      */
/****************************************************************************************/
/*                                                                                      */
/* Notes:                                                                               */
/*                                                                                      */
/****************************************************************************************/

#define _GTC_NETMESSAGE_MODULE_

/*======================================================================================*/
/* Includes                                                                             */
/*======================================================================================*/
#include <stdio.h>
#include <string.h>
#if defined __APPLE__&__MACH__
    #include <malloc/malloc.h>
#else
    #include <malloc.h>
#endif
#include <stdlib.h>

#include "gtl_core.h" // needed for MAJOR & MINOR
#include "gtc_netmessage.h"

/*======================================================================================*/
/* PRIVATE Variables : declaration                                                      */
/*======================================================================================*/

/*======================================================================================*/
/* PRIVATE Functions : declaration                                                      */
/*======================================================================================*/
/* Type conversion */
static int gnm_WordToInt();

/* R_ACK structure */
static int gnm_AddDataFrom_ACK();
static int gnm_ReadDataTo_ACK();
static int gnm_InitStruct_ACK();
static int gnm_FreeStruct_ACK();

/* Q_CFC structure */
static int gnm_AddDataFrom_Q_CFC();
static int gnm_ReadDataTo_Q_CFC();
static int gnm_InitStruct_Q_CFC();
static int gnm_FreeStruct_Q_CFC();

/* R_CFC structure */
static int gnm_AddDataFrom_R_CFC();
static int gnm_ReadDataTo_R_CFC();
static int gnm_InitStruct_R_CFC();
static int gnm_FreeStruct_R_CFC();

/* Q_INIT structure */
static int gnm_AddDataFrom_Q_INIT();
static int gnm_ReadDataTo_Q_INIT();
static int gnm_InitStruct_Q_INIT();
static int gnm_FreeStruct_Q_INIT();

/* R_INIT structure */
static int gnm_AddDataFrom_R_INIT();
static int gnm_ReadDataTo_R_INIT();
static int gnm_InitStruct_R_INIT();
static int gnm_FreeStruct_R_INIT();

/* PRODINFO structure */
static int gnm_AddDataFrom_PRODINFO();
static int gnm_ReadDataTo_PRODINFO();
static int gnm_InitStruct_PRODINFO();
static int gnm_FreeStruct_PRODINFO();

/* Q_TESTLIST structure */
static int gnm_AddDataFrom_Q_TESTLIST();
static int gnm_ReadDataTo_Q_TESTLIST();
static int gnm_InitStruct_Q_TESTLIST();
static int gnm_FreeStruct_Q_TESTLIST();

/* R_TESTLIST structure */
static int gnm_AddDataFrom_R_TESTLIST();
static int gnm_ReadDataTo_R_TESTLIST();
static int gnm_InitStruct_R_TESTLIST();
static int gnm_FreeStruct_R_TESTLIST();

/* Q_PATCONFIG_STATIC structure */
static int gnm_AddDataFrom_Q_PATCONFIG_STATIC();
static int gnm_ReadDataTo_Q_PATCONFIG_STATIC();
static int gnm_InitStruct_Q_PATCONFIG_STATIC();
static int gnm_FreeStruct_Q_PATCONFIG_STATIC();

/* R_PATCONFIG_STATIC structure */
static int gnm_AddDataFrom_R_PATCONFIG_STATIC();
static int gnm_ReadDataTo_R_PATCONFIG_STATIC();
static int gnm_InitStruct_R_PATCONFIG_STATIC();
static int gnm_FreeStruct_R_PATCONFIG_STATIC();

/* Q_PATCONFIG_DYNAMIC structure */
static int gnm_AddDataFrom_Q_PATCONFIG_DYNAMIC();
static int gnm_ReadDataTo_Q_PATCONFIG_DYNAMIC();
static int gnm_InitStruct_Q_PATCONFIG_DYNAMIC();
static int gnm_FreeStruct_Q_PATCONFIG_DYNAMIC();

/* PATCONFIG_DYNAMIC structure */
static int gnm_AddDataFrom_PATCONFIG_DYNAMIC();
static int gnm_AddDataFrom_PATCONFIG_DYNAMIC_2();
static int gnm_ReadDataTo_PATCONFIG_DYNAMIC();
static int gnm_ReadDataTo_PATCONFIG_DYNAMIC_2();
static int gnm_InitStruct_PATCONFIG_DYNAMIC();
static int gnm_InitStruct_PATCONFIG_DYNAMIC_2();
static int gnm_FreeStruct_PATCONFIG_DYNAMIC();
static int gnm_FreeStruct_PATCONFIG_DYNAMIC_2();

/* RESULTS structure */
static int gnm_AddDataFrom_RESULTS();
static int gnm_ReadDataTo_RESULTS();
static int gnm_InitStruct_RESULTS();
static int gnm_FreeStruct_RESULTS();

/* NOTIFY structure */
static int gnm_AddDataFrom_NOTIFY();
static int gnm_ReadDataTo_NOTIFY();
static int gnm_InitStruct_NOTIFY();
static int gnm_FreeStruct_NOTIFY();

/* COMMAND structure */
static int gnm_AddDataFrom_COMMAND();
static int gnm_ReadDataTo_COMMAND();
static int gnm_InitStruct_COMMAND();
static int gnm_FreeStruct_COMMAND();

/* WRITETOSTDF structure */
static int gnm_AddDataFrom_WRITETOSTDF();
static int gnm_ReadDataTo_WRITETOSTDF();
static int gnm_InitStruct_WRITETOSTDF();
static int gnm_FreeStruct_WRITETOSTDF();

/* Q_ENDLOT structure */
static int gnm_AddDataFrom_Q_ENDLOT();
static int gnm_ReadDataTo_Q_ENDLOT();
static int gnm_InitStruct_Q_ENDLOT();
static int gnm_FreeStruct_Q_ENDLOT();

/* R_ENDLOT structure */
static int gnm_AddDataFrom_R_ENDLOT();
static int gnm_ReadDataTo_R_ENDLOT();
static int gnm_InitStruct_R_ENDLOT();
static int gnm_FreeStruct_R_ENDLOT();

/* HEARTBEAT structure */
static int gnm_AddDataFrom_HEARTBEAT();
static int gnm_ReadDataTo_HEARTBEAT();
static int gnm_InitStruct_HEARTBEAT();
static int gnm_FreeStruct_HEARTBEAT();

/* Q_END_SPLITLOT structure */
static int gnm_AddDataFrom_Q_END_SPLITLOT();
static int gnm_ReadDataTo_Q_END_SPLITLOT();
static int gnm_InitStruct_Q_END_SPLITLOT();
static int gnm_FreeStruct_Q_END_SPLITLOT();
/* R_END_SPLITLOT structure */
static int gnm_AddDataFrom_R_END_SPLITLOT();
static int gnm_ReadDataTo_R_END_SPLITLOT();
static int gnm_InitStruct_R_END_SPLITLOT();
static int gnm_FreeStruct_R_END_SPLITLOT();


/*======================================================================================*/
/* PUBLIC Functions                                                                     */
/*======================================================================================*/
/*======================================================================================*/
/* Description  : initialize Data structure.											*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     unsigned int uiType                                                              */
/*         Type of data structure                                                       */
/*                                                                                      */
/*     void* ptData																		*/
/*         Ptr to the data structure to initialize.										*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY if successfull; otherwise, return an error	code defined in         */
/*     gnetbuffer.h header file.                                                        */
/*======================================================================================*/
int gtcnm_InitStruct(unsigned int uiType, void* ptData)
{
	/* Initialize structure */
	switch(uiType)
	{
        case GNM_TYPE_ACK:
            gnm_InitStruct_ACK((PT_GNM_ACK)ptData);
            break;

        case GNM_TYPE_Q_CFC:
			gnm_InitStruct_Q_CFC((PT_GNM_Q_CFC)ptData);
			break;
			
		case GNM_TYPE_R_CFC:
			gnm_InitStruct_R_CFC((PT_GNM_R_CFC)ptData);
			break;
			
        case GNM_TYPE_Q_INIT:
            gnm_InitStruct_Q_INIT((PT_GNM_Q_INIT)ptData);
			break;
			
        case GNM_TYPE_R_INIT:
            gnm_InitStruct_R_INIT((PT_GNM_R_INIT)ptData);
            break;

        case GNM_TYPE_PRODINFO:
			gnm_InitStruct_PRODINFO((PT_GNM_PRODINFO)ptData);
			break;
			
		case GNM_TYPE_Q_TESTLIST:
			gnm_InitStruct_Q_TESTLIST((PT_GNM_Q_TESTLIST)ptData);
			break;
			
		case GNM_TYPE_R_TESTLIST:
			gnm_InitStruct_R_TESTLIST((PT_GNM_R_TESTLIST)ptData);
			break;
			
		case GNM_TYPE_Q_PATCONFIG_STATIC:
			gnm_InitStruct_Q_PATCONFIG_STATIC((PT_GNM_Q_PATCONFIG_STATIC)ptData);
			break;
			
		case GNM_TYPE_R_PATCONFIG_STATIC:
			gnm_InitStruct_R_PATCONFIG_STATIC((PT_GNM_R_PATCONFIG_STATIC)ptData);
			break;
			
        case GNM_TYPE_Q_PATCONFIG_DYNAMIC:
            gnm_InitStruct_Q_PATCONFIG_DYNAMIC((PT_GNM_Q_PATCONFIG_DYNAMIC)ptData);
            break;

        case GNM_TYPE_PATCONFIG_DYNAMIC:
			gnm_InitStruct_PATCONFIG_DYNAMIC((PT_GNM_PATCONFIG_DYNAMIC)ptData);
			break;
			
        case GNM_TYPE_PATCONFIG_DYNAMIC_2:
            gnm_InitStruct_PATCONFIG_DYNAMIC_2((PT_GNM_PATCONFIG_DYNAMIC_2)ptData);
            break;

        case GNM_TYPE_RESULTS:
			gnm_InitStruct_RESULTS((PT_GNM_RESULTS)ptData);
			break;
					
		case GNM_TYPE_NOTIFY:
			gnm_InitStruct_NOTIFY((PT_GNM_NOTIFY)ptData);
			break;
					
		case GNM_TYPE_COMMAND:
			gnm_InitStruct_COMMAND((PT_GNM_COMMAND)ptData);
			break;
					
		case GNM_TYPE_WRITETOSTDF:
			gnm_InitStruct_WRITETOSTDF((PT_GNM_WRITETOSTDF)ptData);
			break;
					
        case GNM_TYPE_Q_ENDLOT:
            gnm_InitStruct_Q_ENDLOT((PT_GNM_Q_ENDLOT)ptData);
			break;
					
        case GNM_TYPE_R_ENDLOT:
            gnm_InitStruct_R_ENDLOT((PT_GNM_R_ENDLOT)ptData);
            break;

        case GNM_TYPE_HEARTBEAT:
			gnm_InitStruct_HEARTBEAT((PT_GNM_HEARTBEAT)ptData);
			break;

        case GNM_TYPE_Q_END_OF_SPLITLOT:
            gnm_InitStruct_Q_END_SPLITLOT((PT_GNM_Q_END_OF_SPLITLOT)ptData);
            break;
        case GNM_TYPE_R_END_OF_SPLITLOT:
            gnm_InitStruct_R_END_SPLITLOT((PT_GNM_R_END_OF_SPLITLOT)ptData);
            break;


		default:
            GTL_LOG(4, "Unknown GNM type %d", uiType);
			return GNET_ERR_IMT;
			break;
	}
	
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : free Data structure.													*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     unsigned int uiType                                                              */
/*         Type of data structure                                                       */
/*                                                                                      */
/*     void* ptData																		*/
/*         Ptr to the data structure to free.											*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY if successfull; otherwise, return an error	code defined in         */
/*     gnetbuffer.h header file.                                                        */
/*======================================================================================*/
int gtcnm_FreeStruct(unsigned int uiType, void* ptData)
{
    /* If ptr is null, nothing to free */
    if(ptData == NULL)
        return GNET_ERR_OKAY;

    /* Initialize structure */
	switch(uiType)
	{
        case GNM_TYPE_ACK:
            gnm_FreeStruct_ACK((PT_GNM_ACK)ptData);
            break;

        case GNM_TYPE_Q_CFC:
            gnm_FreeStruct_Q_CFC((PT_GNM_Q_CFC)ptData);
			break;
			
		case GNM_TYPE_R_CFC:
			gnm_FreeStruct_R_CFC((PT_GNM_R_CFC)ptData);
			break;
			
		case GNM_TYPE_Q_INIT:
			gnm_FreeStruct_Q_INIT((PT_GNM_Q_INIT)ptData);
			break;
			
		case GNM_TYPE_R_INIT:
			gnm_FreeStruct_R_INIT((PT_GNM_R_INIT)ptData);
			break;
			
		case GNM_TYPE_PRODINFO:
			gnm_FreeStruct_PRODINFO((PT_GNM_PRODINFO)ptData);
			break;
			
		case GNM_TYPE_Q_TESTLIST:
			gnm_FreeStruct_Q_TESTLIST((PT_GNM_Q_TESTLIST)ptData);
			break;
			
		case GNM_TYPE_R_TESTLIST:
			gnm_FreeStruct_R_TESTLIST((PT_GNM_R_TESTLIST)ptData);
			break;
			
		case GNM_TYPE_Q_PATCONFIG_STATIC:
			gnm_FreeStruct_Q_PATCONFIG_STATIC((PT_GNM_Q_PATCONFIG_STATIC)ptData);
			break;
			
		case GNM_TYPE_R_PATCONFIG_STATIC:
			gnm_FreeStruct_R_PATCONFIG_STATIC((PT_GNM_R_PATCONFIG_STATIC)ptData);
			break;
			
        case GNM_TYPE_Q_PATCONFIG_DYNAMIC:
            gnm_FreeStruct_Q_PATCONFIG_DYNAMIC((PT_GNM_Q_PATCONFIG_DYNAMIC)ptData);
            break;

        case GNM_TYPE_PATCONFIG_DYNAMIC:
			gnm_FreeStruct_PATCONFIG_DYNAMIC((PT_GNM_PATCONFIG_DYNAMIC)ptData);
			break;
			
        case GNM_TYPE_PATCONFIG_DYNAMIC_2:
            gnm_FreeStruct_PATCONFIG_DYNAMIC_2((PT_GNM_PATCONFIG_DYNAMIC_2)ptData);
            break;

        case GNM_TYPE_RESULTS:
			gnm_FreeStruct_RESULTS((PT_GNM_RESULTS)ptData);
			break;
					
		case GNM_TYPE_NOTIFY:
			gnm_FreeStruct_NOTIFY((PT_GNM_NOTIFY)ptData);
			break;
					
		case GNM_TYPE_COMMAND:
			gnm_FreeStruct_COMMAND((PT_GNM_COMMAND)ptData);
			break;
					
		case GNM_TYPE_WRITETOSTDF:
			gnm_FreeStruct_WRITETOSTDF((PT_GNM_WRITETOSTDF)ptData);
			break;

        case GNM_TYPE_Q_ENDLOT:
            gnm_FreeStruct_Q_ENDLOT((PT_GNM_Q_ENDLOT)ptData);
			break;
        case GNM_TYPE_Q_END_OF_SPLITLOT:
            gnm_FreeStruct_Q_END_SPLITLOT((PT_GNM_Q_END_OF_SPLITLOT)ptData);
            break;

        case GNM_TYPE_R_ENDLOT:
            gnm_FreeStruct_R_ENDLOT((PT_GNM_R_ENDLOT)ptData);
            break;
        case GNM_TYPE_R_END_OF_SPLITLOT:
            gnm_FreeStruct_R_END_SPLITLOT((PT_GNM_R_END_OF_SPLITLOT)ptData);
            break;

        case GNM_TYPE_HEARTBEAT:
			gnm_FreeStruct_HEARTBEAT((PT_GNM_HEARTBEAT)ptData);
			break;
					
		default:
			return GNET_ERR_IMT;
			break;
	}

	return GNET_ERR_OKAY;
}

int gtcnm_CreateBufferData(uiType, phBuffer, ptData)
unsigned int	uiType;
PT_GNB_HBUFFER	phBuffer;
void*			ptData;
{
	/* Write Data to Buffer */
	switch(uiType)
	{
        case GNM_TYPE_ACK:
            gnm_AddDataFrom_ACK(phBuffer, (PT_GNM_ACK)ptData);
            break;

        case GNM_TYPE_Q_CFC:
			gnm_AddDataFrom_Q_CFC(phBuffer, (PT_GNM_Q_CFC)ptData);
			break;
			
		case GNM_TYPE_R_CFC:
			gnm_AddDataFrom_R_CFC(phBuffer, (PT_GNM_R_CFC)ptData);
			break;
			
		case GNM_TYPE_Q_INIT:
			gnm_AddDataFrom_Q_INIT(phBuffer, (PT_GNM_Q_INIT)ptData);
			break;
			
		case GNM_TYPE_R_INIT:
			gnm_AddDataFrom_R_INIT(phBuffer, (PT_GNM_R_INIT)ptData);
			break;
			
		case GNM_TYPE_PRODINFO:
			gnm_AddDataFrom_PRODINFO(phBuffer, (PT_GNM_PRODINFO)ptData);
			break;
			
		case GNM_TYPE_Q_TESTLIST:
			gnm_AddDataFrom_Q_TESTLIST(phBuffer, (PT_GNM_Q_TESTLIST)ptData);
			break;
			
		case GNM_TYPE_R_TESTLIST:
			gnm_AddDataFrom_R_TESTLIST(phBuffer, (PT_GNM_R_TESTLIST)ptData);
			break;
			
		case GNM_TYPE_Q_PATCONFIG_STATIC:
			gnm_AddDataFrom_Q_PATCONFIG_STATIC(phBuffer, (PT_GNM_Q_PATCONFIG_STATIC)ptData);
			break;
			
		case GNM_TYPE_R_PATCONFIG_STATIC:
			gnm_AddDataFrom_R_PATCONFIG_STATIC(phBuffer, (PT_GNM_R_PATCONFIG_STATIC)ptData);
			break;
			
        case GNM_TYPE_Q_PATCONFIG_DYNAMIC:
            gnm_AddDataFrom_Q_PATCONFIG_DYNAMIC(phBuffer, (PT_GNM_Q_PATCONFIG_DYNAMIC)ptData);
            break;

        case GNM_TYPE_PATCONFIG_DYNAMIC:
			gnm_AddDataFrom_PATCONFIG_DYNAMIC(phBuffer, (PT_GNM_PATCONFIG_DYNAMIC)ptData);
			break;
			
        case GNM_TYPE_PATCONFIG_DYNAMIC_2:
            gnm_AddDataFrom_PATCONFIG_DYNAMIC_2(phBuffer, (PT_GNM_PATCONFIG_DYNAMIC_2)ptData);
            break;

        case GNM_TYPE_RESULTS:
			gnm_AddDataFrom_RESULTS(phBuffer, (PT_GNM_RESULTS)ptData);
			break;
					
		case GNM_TYPE_NOTIFY:
			gnm_AddDataFrom_NOTIFY(phBuffer, (PT_GNM_NOTIFY)ptData);
			break;
					
		case GNM_TYPE_COMMAND:
			gnm_AddDataFrom_COMMAND(phBuffer, (PT_GNM_COMMAND)ptData);
			break;
					
		case GNM_TYPE_WRITETOSTDF:
			gnm_AddDataFrom_WRITETOSTDF(phBuffer, (PT_GNM_WRITETOSTDF)ptData);
			break;
					
        case GNM_TYPE_Q_ENDLOT:
            gnm_AddDataFrom_Q_ENDLOT(phBuffer, (PT_GNM_Q_ENDLOT)ptData);
			break;

        case GNM_TYPE_Q_END_OF_SPLITLOT:
            gnm_AddDataFrom_Q_END_SPLITLOT(phBuffer, (PT_GNM_Q_END_OF_SPLITLOT)ptData);
            break;

        case GNM_TYPE_R_ENDLOT:
            gnm_AddDataFrom_R_ENDLOT(phBuffer, (PT_GNM_R_ENDLOT)ptData);
            break;

        case GNM_TYPE_R_END_OF_SPLITLOT:
            gnm_AddDataFrom_R_END_SPLITLOT(phBuffer, (PT_GNM_R_END_OF_SPLITLOT)ptData);
            break;

        case GNM_TYPE_HEARTBEAT:
			gnm_AddDataFrom_HEARTBEAT(phBuffer, (PT_GNM_HEARTBEAT)ptData);
			break;
					
        default:
			return GNET_ERR_IMT;
			break;
	}
	
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : read Data structure from buffer.                                      */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     unsigned int uiType                                                              */
/*         Type of data structure                                                       */
/*                                                                                      */
/*     PT_GNB_HBUFFER	phBuffer                                                        */
/*         Ptr to the buffer that contains the data to read.                            */
/*                                                                                      */
/*     void** ptptData                                                                  */
/*         Ptr to the data structure that will be filled by the buffer content.         */
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY if successfull; otherwise, return an error	code defined in         */
/*     gnetbuffer.h header file.                                                        */
/*======================================================================================*/
int gtcnm_ReadBufferData(uiType, phBuffer, ptptData)
unsigned int	uiType;
PT_GNB_HBUFFER	phBuffer;
void**			ptptData;
{
	/* Read Data from buffer */
	switch(uiType)
	{
        case GNM_TYPE_ACK:
            gnm_ReadDataTo_ACK(phBuffer, ptptData);
            break;

        case GNM_TYPE_Q_CFC:
			gnm_ReadDataTo_Q_CFC(phBuffer, ptptData);
			break;
			
		case GNM_TYPE_R_CFC:
			gnm_ReadDataTo_R_CFC(phBuffer, ptptData);
			break;
			
		case GNM_TYPE_Q_INIT:
			gnm_ReadDataTo_Q_INIT(phBuffer, ptptData);
			break;
			
		case GNM_TYPE_R_INIT:
			gnm_ReadDataTo_R_INIT(phBuffer, ptptData);
			break;
			
		case GNM_TYPE_PRODINFO:
			gnm_ReadDataTo_PRODINFO(phBuffer, ptptData);
			break;
			
		case GNM_TYPE_Q_TESTLIST:
			gnm_ReadDataTo_Q_TESTLIST(phBuffer, ptptData);
			break;
			
		case GNM_TYPE_R_TESTLIST:
			gnm_ReadDataTo_R_TESTLIST(phBuffer, ptptData);
			break;
			
		case GNM_TYPE_Q_PATCONFIG_STATIC:
			gnm_ReadDataTo_Q_PATCONFIG_STATIC(phBuffer, ptptData);
			break;
			
		case GNM_TYPE_R_PATCONFIG_STATIC:
			gnm_ReadDataTo_R_PATCONFIG_STATIC(phBuffer, ptptData);
			break;
			
        case GNM_TYPE_Q_PATCONFIG_DYNAMIC:
            gnm_ReadDataTo_Q_PATCONFIG_DYNAMIC(phBuffer, ptptData);
            break;

        case GNM_TYPE_PATCONFIG_DYNAMIC:
			gnm_ReadDataTo_PATCONFIG_DYNAMIC(phBuffer, ptptData);
			break;
			
        case GNM_TYPE_PATCONFIG_DYNAMIC_2:
            gnm_ReadDataTo_PATCONFIG_DYNAMIC_2(phBuffer, ptptData);
            break;

        case GNM_TYPE_RESULTS:
			gnm_ReadDataTo_RESULTS(phBuffer, ptptData);
			break;

		case GNM_TYPE_NOTIFY:
			gnm_ReadDataTo_NOTIFY(phBuffer, ptptData);
			break;

		case GNM_TYPE_COMMAND:
			gnm_ReadDataTo_COMMAND(phBuffer, ptptData);
			break;

		case GNM_TYPE_WRITETOSTDF:
			gnm_ReadDataTo_WRITETOSTDF(phBuffer, ptptData);
			break;

        case GNM_TYPE_Q_ENDLOT:
            gnm_ReadDataTo_Q_ENDLOT(phBuffer, ptptData);
			break;
        case GNM_TYPE_Q_END_OF_SPLITLOT:
            gnm_ReadDataTo_Q_END_SPLITLOT(phBuffer, ptptData);
            break;

        case GNM_TYPE_R_ENDLOT:
            gnm_ReadDataTo_R_ENDLOT(phBuffer, ptptData);
            break;
        case GNM_TYPE_R_END_OF_SPLITLOT:
            gnm_ReadDataTo_R_END_SPLITLOT(phBuffer, ptptData);
            break;

        case GNM_TYPE_HEARTBEAT:
			gnm_ReadDataTo_HEARTBEAT(phBuffer, ptptData);
			break;

		default:
			return GNET_ERR_IMT;
			break;
	}
	
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* PRIVATE Functions                                                                    */
/*======================================================================================*/
/*======================================================================================*/
/* Description  : convert a GNB_WORD to a signed integer.			                    */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_WORD	wData																	*/
/*         data to convert																*/
/*                                                                                      */
/* Return Codes : converted signed integer                                              */
/*======================================================================================*/
int gnm_WordToInt(wData)
GNB_WORD wData;
{
	if(wData <=32767)
		return (int)wData;

	return (int)wData - 65536;
}

/*======================================================================================*/
/* Description  : add data from GNM_TYPE_ACK structure to buffer.                       */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that will receive the data.                                */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_ACK data structure used to fill the buffer.              */
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_AddDataFrom_ACK(pthBuffer, ptstData)
PT_GNB_HBUFFER  pthBuffer;
PT_GNM_ACK      ptstData;
{
    /* Add Data to buffer */
    gnb_AddDWord(pthBuffer, (GNB_DWORD)ptstData->mTimeStamp);
    gnb_AddDWord(pthBuffer, (GNB_DWORD)ptstData->mStatus);

    return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : read data from buffer to GNM_TYPE_ACK structure.                      */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that contains the data to read.                            */
/*                                                                                      */
/*     void** ptptData                                                                  */
/*         Ptr to the GNM_TYPE_ACK data structure that will be filled by the buffer     */
/*         content.                                                                     */
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY if successfull.                                                    */
/*     GNET_ERR_MAF : memory allocation failure.                                        */
/*======================================================================================*/
static int gnm_ReadDataTo_ACK(pthBuffer, ptptData)
PT_GNB_HBUFFER  pthBuffer;
void            **ptptData;
{
    GNB_DWORD     dwData;
    PT_GNM_ACK    ptstData;

    /* Alloc memory for data structure */
    ptstData = (PT_GNM_ACK)malloc(sizeof(GNM_ACK));
    if(!ptstData)
        return GNET_ERR_MAF;

    /* Initialize structure */
    gnm_InitStruct_ACK(ptstData);

    /* Read Data from buffer */
    gnb_ReadDWord(pthBuffer, &dwData);
    ptstData->mTimeStamp = (unsigned int)dwData;
    gnb_ReadDWord(pthBuffer, &dwData);
    ptstData->mStatus = (int)dwData;

    *ptptData = (void*)ptstData;
    return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : init data in the GNM_TYPE_ACK structure.							    */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_ACK data structure to be initialized.					*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_InitStruct_ACK(ptstData)
PT_GNM_ACK  ptstData;
{
    /* Initialize structure */
    ptstData->mTimeStamp = 0;
    ptstData->mStatus = 0;

    return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : free data in the GNM_TYPE_ACK structure.							    */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_ACK data structure to be freeed.						    */
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_FreeStruct_ACK(ptstData)
PT_GNM_ACK    ptstData;
{
    /* If ptr is null, nothing to free */
    if(ptstData == NULL)
        return GNET_ERR_OKAY;

    /* Free structure */

    return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : add data from GNM_TYPE_Q_CFC structure to buffer.                     */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that will receive the data.                                */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_Q_CFC data structure used to fill the buffer.            */
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_AddDataFrom_Q_CFC(PT_GNB_HBUFFER pthBuffer, PT_GNM_Q_CFC ptstData)
{
	/* Add Data to buffer */
    gnb_AddDWord(pthBuffer, (GNB_DWORD)ptstData->mTimeStamp);
    gnb_AddDWord(pthBuffer, (GNB_DWORD)ptstData->mGtlVersionMajor);
    gnb_AddDWord(pthBuffer, (GNB_DWORD)ptstData->mGtlVersionMinor);

	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : read data from buffer to GNM_TYPE_Q_CFC structure.                    */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that contains the data to read.                            */
/*                                                                                      */
/*     void** ptptData                                                                  */
/*         Ptr to the GNM_TYPE_Q_CFC data structure that will be filled by the buffer   */
/*         content.                                                                     */
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY if successfull.                                                    */
/*     GNET_ERR_MAF : memory allocation failure.                                        */
/*======================================================================================*/
static int gnm_ReadDataTo_Q_CFC(pthBuffer, ptptData)
PT_GNB_HBUFFER  pthBuffer;
void            **ptptData;
{
	GNB_DWORD     dwData;
	PT_GNM_Q_CFC  ptstData;
	
	/* Alloc memory for data structure */
	ptstData = (PT_GNM_Q_CFC)malloc(sizeof(GNM_Q_CFC));
	if(!ptstData)
		return GNET_ERR_MAF;
	
	/* Initialize structure */
    gnm_InitStruct_Q_CFC(ptstData);

	/* Read Data from buffer */
	gnb_ReadDWord(pthBuffer, &dwData);
    ptstData->mTimeStamp = (unsigned int)dwData;
    gnb_ReadDWord(pthBuffer, &dwData);
    ptstData->mGtlVersionMajor = (unsigned int)dwData;
    gnb_ReadDWord(pthBuffer, &dwData);
    ptstData->mGtlVersionMinor = (unsigned int)dwData;

    *ptptData = (void*)ptstData; // dont we have to do a memcpy( sizeof(GNM_Q_CFC) ) ?
    //memcpy(*ptptData, sizeof(GNM_Q_CFC), ptstData ); // no : ptptData is a pointer to pointer
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : init data in the GNM_TYPE_Q_CFC structure.							*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_Q_CFC data structure to be initialized.					*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_InitStruct_Q_CFC(PT_GNM_Q_CFC ptstData)
{
	/* Initialize structure */
    ptstData->mTimeStamp = 0;
    ptstData->mGtlVersionMajor = GTL_VERSION_MAJOR;
    ptstData->mGtlVersionMinor = GTL_VERSION_MINOR;

	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : free data in the GNM_TYPE_Q_CFC structure.							*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_Q_CFC data structure to be freeed.						*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_FreeStruct_Q_CFC(ptstData)
PT_GNM_Q_CFC    ptstData;
{
    /* If ptr is null, nothing to free */
    if(ptstData == NULL)
        return GNET_ERR_OKAY;

    /* Free structure */

	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : add data from GNM_TYPE_R_CFC structure to buffer.                     */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that will receive the data.                                */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_R_CFC data structure used to fill the buffer.            */
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_AddDataFrom_R_CFC(pthBuffer, ptstData)
PT_GNB_HBUFFER  pthBuffer;
PT_GNM_R_CFC    ptstData;
{
	/* Add Data to buffer */
    gnb_AddDWord(pthBuffer, (GNB_DWORD)ptstData->mTimeStamp);
    gnb_AddDWord(pthBuffer, (GNB_DWORD)ptstData->mStatus);

	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : read data from buffer to GNM_TYPE_R_CFC structure.                    */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that contains the data to read.                            */
/*                                                                                      */
/*     void** ptptData                                                                  */
/*         Ptr to the GNM_TYPE_R_CFC data structure that will be filled by the buffer   */
/*         content.                                                                     */
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY if successfull.                                                    */
/*     GNET_ERR_MAF : memory allocation failure.                                        */
/*======================================================================================*/
static int gnm_ReadDataTo_R_CFC(pthBuffer, ptptData)
PT_GNB_HBUFFER  pthBuffer;
void            **ptptData;
{
	GNB_DWORD     dwData;
	PT_GNM_R_CFC  ptstData;
	
	/* Alloc memory for data structure */
	ptstData = (PT_GNM_R_CFC)malloc(sizeof(GNM_R_CFC));
	if(!ptstData)
		return GNET_ERR_MAF;
	
	/* Initialize structure */
	gnm_InitStruct_R_CFC(ptstData);

	/* Read Data from buffer */
	gnb_ReadDWord(pthBuffer, &dwData);
    ptstData->mTimeStamp = (unsigned int)dwData;
    gnb_ReadDWord(pthBuffer, &dwData);
    ptstData->mStatus = (int)dwData;

	*ptptData = (void*)ptstData;
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : init data in the GNM_TYPE_R_CFC structure.							*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_R_CFC data structure to be initialized.					*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_InitStruct_R_CFC(ptstData)
PT_GNM_R_CFC    ptstData;
{
	/* Initialize structure */
    ptstData->mTimeStamp = 0;
    ptstData->mStatus = 0;

    return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : free data in the GNM_TYPE_R_CFC structure.							*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_R_CFC data structure to be freeed.						*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_FreeStruct_R_CFC(ptstData)
PT_GNM_R_CFC    ptstData;
{
    /* If ptr is null, nothing to free */
    if(ptstData == NULL)
        return GNET_ERR_OKAY;

    /* Free structure */
	
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : add data from GNM_TYPE_Q_INIT structure to buffer.                    */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that will receive the data.                                */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_Q_INIT data structure used to fill the buffer.           */
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_AddDataFrom_Q_INIT(pthBuffer, ptstData)
PT_GNB_HBUFFER	pthBuffer;
PT_GNM_Q_INIT	ptstData;
{
	unsigned int	uiIndex;
    unsigned int	uiNbSites = ptstData->mNbSites < 256 ? ptstData->mNbSites : 256;

	/* Add Data to buffer */
    gnb_AddWord(pthBuffer, (GNB_WORD)ptstData->mStationNb);
	gnb_AddWord(pthBuffer, (GNB_WORD)uiNbSites);
    for(uiIndex=0; uiIndex<uiNbSites; ++uiIndex)
        gnb_AddWord(pthBuffer, (GNB_WORD)(ptstData->mSiteNumbers[uiIndex]));
    // gcore-590 : let s send the full 256 sites
    //for(uiIndex=0; uiIndex<uiNbSites; ++uiIndex)
    for(uiIndex=0; uiIndex<256; ++uiIndex)
        gnb_AddWord(pthBuffer, (GNB_WORD)(ptstData->mSitesStates[uiIndex]));
    gnb_AddString(pthBuffer, ptstData->mGtlVersion);
    gnb_AddString(pthBuffer, ptstData->mHostID);
    gnb_AddString(pthBuffer, ptstData->mNodeName);
    gnb_AddString(pthBuffer, ptstData->mUserName);
    gnb_AddString(pthBuffer, ptstData->mTesterExec);
    gnb_AddString(pthBuffer, ptstData->mTesterType);
    gnb_AddString(pthBuffer, ptstData->mTestJobName);
    gnb_AddString(pthBuffer, ptstData->mTestJobFile);
    gnb_AddString(pthBuffer, ptstData->mTestJobPath);
    gnb_AddString(pthBuffer, ptstData->mTestSourceFilesPath);
    // Case 7484: WORD -> DWORD for recipe size
    if(ptstData->mRecipeBuffer == NULL)
        gnb_AddDWord(pthBuffer, (GNB_DWORD)0);
    else
    {
        GTL_LOG(7, "Add GNM_Q_INIT to socket buffer: strlen(mRecipeBuffer)=%d",
                (GNB_DWORD)strlen(ptstData->mRecipeBuffer), 0);

        /* CHECKME: add security if buffer too big */
        // The strlen() function calculates the length of the string s, not including the terminating '\0' character.
        size_t lRecipeLength=strlen(ptstData->mRecipeBuffer);
        GTL_LOG(5, "Data Recipe length = %d", lRecipeLength);
        gnb_AddDWord(pthBuffer, (GNB_DWORD)lRecipeLength);
        gnb_AddString(pthBuffer, ptstData->mRecipeBuffer);
    }

	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : read data from buffer to GNM_TYPE_Q_INIT structure.                   */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that contains the data to read.                            */
/*                                                                                      */
/*     void** ptptData                                                                  */
/*         Ptr to the GNM_TYPE_Q_INIT data structure that will be filled by the buffer  */
/*         content.                                                                     */
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY if successfull.                                                    */
/*     GNET_ERR_MAF : memory allocation failure.                                        */
/*======================================================================================*/
static int gnm_ReadDataTo_Q_INIT(pthBuffer, ptptData)
PT_GNB_HBUFFER  pthBuffer;
void            **ptptData;
{
    GNB_WORD wData;
    GNB_DWORD dwData;
    PT_GNM_Q_INIT ptstData;
    unsigned int uiNbSites, uiIndex;
	
	/* Alloc memory for data structure */
	ptstData = (PT_GNM_Q_INIT)malloc(sizeof(GNM_Q_INIT));
	if(!ptstData)
		return GNET_ERR_MAF;
	
	/* Initialize structure */
	gnm_InitStruct_Q_INIT(ptstData);

	/* Read Data from buffer */
    gnb_ReadWord(pthBuffer, &wData);
    ptstData->mStationNb = (unsigned int)wData;
	gnb_ReadWord(pthBuffer, &wData);
    ptstData->mNbSites = uiNbSites = (unsigned int)wData;
    for(uiIndex=0; uiIndex<uiNbSites; ++uiIndex)
	{
		gnb_ReadWord(pthBuffer, &wData);
        ptstData->mSiteNumbers[uiIndex] = gnm_WordToInt(wData);
	}
    // gcore-590 : let s get the full 256 sites states
    //for(uiIndex=0; uiIndex<uiNbSites; ++uiIndex)
    for(uiIndex=0; uiIndex<256; ++uiIndex)
    {
        gnb_ReadWord(pthBuffer, &wData);
        ptstData->mSitesStates[uiIndex] = gnm_WordToInt(wData);
    }
    gnb_ReadString(pthBuffer, ptstData->mGtlVersion);
    gnb_ReadString(pthBuffer, ptstData->mHostID);
    gnb_ReadString(pthBuffer, ptstData->mNodeName);
    gnb_ReadString(pthBuffer, ptstData->mUserName);
    gnb_ReadString(pthBuffer, ptstData->mTesterExec);
    gnb_ReadString(pthBuffer, ptstData->mTesterType);
    gnb_ReadString(pthBuffer, ptstData->mTestJobName);
    gnb_ReadString(pthBuffer, ptstData->mTestJobFile);
    gnb_ReadString(pthBuffer, ptstData->mTestJobPath);
    gnb_ReadString(pthBuffer, ptstData->mTestSourceFilesPath);
    // Case 7484: WORD -> DWORD for recipe size
    gnb_ReadDWord(pthBuffer, &dwData);
    if(dwData > 0)
    {
        /* Add 1 char for end-of-string '\0' */
        ptstData->mRecipeBuffer = (char*)malloc((dwData+1)*sizeof(char));
        if (ptstData->mRecipeBuffer==0)
            return GNET_ERR_MAF;
        // Let's be sure the recipe will have in any cases a end of string
        memset(ptstData->mRecipeBuffer, '\0', (dwData+1)*sizeof(char));
        gnb_ReadString(pthBuffer, ptstData->mRecipeBuffer);

        GTL_LOG(7, "Read GNM_Q_INIT from socket buffer: size read=%d, strlen(mRecipeBuffer)=%d", dwData,
                (GNB_DWORD)strlen(ptstData->mRecipeBuffer), 0);
    }

	*ptptData = (void*)ptstData;
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : init data in the GNM_TYPE_Q_INIT structure.							*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_Q_INIT data structure to be initialized.					*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_InitStruct_Q_INIT(PT_GNM_Q_INIT ptstData)
{
    unsigned int uiIndex=0;

	/* Initialize structure */
    ptstData->mStationNb = 0;
    ptstData->mNbSites = 0;
    for(uiIndex=0; uiIndex<256; ++uiIndex)
        ptstData->mSiteNumbers[uiIndex] = -1;
    for(uiIndex=0; uiIndex<256; ++uiIndex)
        ptstData->mSitesStates[uiIndex] = -1;
    strcpy(ptstData->mGtlVersion, "");
    strcpy(ptstData->mHostID, "");
    strcpy(ptstData->mNodeName, "");
    strcpy(ptstData->mUserName, "");
    strcpy(ptstData->mTesterExec, "");
    strcpy(ptstData->mTesterType, "");
    strcpy(ptstData->mTestJobName, "");
    strcpy(ptstData->mTestJobFile, "");
    strcpy(ptstData->mTestJobPath, "");
    strcpy(ptstData->mTestSourceFilesPath, "");
    ptstData->mRecipeBuffer=NULL;

	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : free data in the GNM_TYPE_Q_INIT structure.							*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_Q_INIT data structure to be freeed.						*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_FreeStruct_Q_INIT(ptstData)
PT_GNM_Q_INIT	ptstData;
{
    /* If ptr is null, nothing to free */
    if(ptstData == NULL)
        return GNET_ERR_OKAY;

    /* Free structure */
    if(ptstData->mRecipeBuffer)
    {
        free(ptstData->mRecipeBuffer);
        ptstData->mRecipeBuffer=NULL;
    }
	
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : add data from GNM_TYPE_R_INIT structure to buffer.                    */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that will receive the data.                                */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_R_INIT data structure used to fill the buffer.           */
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_AddDataFrom_R_INIT(pthBuffer, ptstData)
PT_GNB_HBUFFER	pthBuffer;
PT_GNM_R_INIT	ptstData;
{
	unsigned int	uiIndex;
    unsigned int	uiNbGoodSBins = ptstData->uiNbGoodSBins < 256 ? ptstData->uiNbGoodSBins : 256;
    unsigned int    uiNbGoodHBins = ptstData->uiNbGoodHBins < 256 ? ptstData->uiNbGoodHBins : 256;

	/* Add Data to buffer */
	gnb_AddWord(pthBuffer, (GNB_WORD)ptstData->nStatus);
	gnb_AddWord(pthBuffer, (GNB_WORD)ptstData->nGtlModules);
	gnb_AddWord(pthBuffer, (GNB_WORD)ptstData->uiFindTestKey);
	gnb_AddWord(pthBuffer, (GNB_WORD)ptstData->uiResultFrequency);
	gnb_AddWord(pthBuffer, (GNB_WORD)ptstData->uiNbRunsBeforeProdInfo);
    gnb_AddWord(pthBuffer, (GNB_WORD)uiNbGoodSBins);
    for(uiIndex=0; uiIndex<uiNbGoodSBins; uiIndex++)
        gnb_AddWord(pthBuffer, (GNB_WORD)(ptstData->pnGoodSoftBins[uiIndex]));

    gnb_AddWord(pthBuffer, (GNB_WORD)uiNbGoodHBins);
    for(uiIndex=0; uiIndex<uiNbGoodHBins; uiIndex++)
        gnb_AddWord(pthBuffer, (GNB_WORD)(ptstData->pnGoodHardBins[uiIndex]));

    gnb_AddString(pthBuffer, ptstData->mMessage);

	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : read data from buffer to GNM_TYPE_R_INIT structure.                   */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that contains the data to read.                            */
/*                                                                                      */
/*     void** ptptData                                                                  */
/*         Ptr to the GNM_TYPE_R_INIT data structure that will be filled by the buffer  */
/*         content.                                                                     */
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY if successfull.                                                    */
/*     GNET_ERR_MAF : memory allocation failure.                                        */
/*======================================================================================*/
static int gnm_ReadDataTo_R_INIT(pthBuffer, ptptData)
PT_GNB_HBUFFER  pthBuffer;
void            **ptptData;
{
    GNB_WORD wData;
    PT_GNM_R_INIT ptstData=0;
    //unsigned int	uiNbGoodBins=0,
    unsigned int uiIndex=0;
	
	/* Alloc memory for data structure */
	ptstData = (PT_GNM_R_INIT)malloc(sizeof(GNM_R_INIT));
	if(!ptstData)
		return GNET_ERR_MAF;
	
	/* Initialize structure */
	gnm_InitStruct_R_INIT(ptstData);

	/* Read Data from buffer */
	gnb_ReadWord(pthBuffer, &wData);
	ptstData->nStatus = gnm_WordToInt(wData);
	gnb_ReadWord(pthBuffer, &wData);
	ptstData->nGtlModules = gnm_WordToInt(wData);
	gnb_ReadWord(pthBuffer, &wData);
	ptstData->uiFindTestKey = (unsigned int)wData;
	gnb_ReadWord(pthBuffer, &wData);
	ptstData->uiResultFrequency = (unsigned int)wData;
	gnb_ReadWord(pthBuffer, &wData);
	ptstData->uiNbRunsBeforeProdInfo = (unsigned int)wData;

    gnb_ReadWord(pthBuffer, &wData);
    ptstData->uiNbGoodSBins = (unsigned int)wData;
    for(uiIndex=0; uiIndex<ptstData->uiNbGoodSBins; uiIndex++)
	{
		gnb_ReadWord(pthBuffer, &wData);
        ptstData->pnGoodSoftBins[uiIndex] = gnm_WordToInt(wData);
	}

    gnb_ReadWord(pthBuffer, &wData);
    ptstData->uiNbGoodHBins = (unsigned int)wData;
    for(uiIndex=0; uiIndex<ptstData->uiNbGoodHBins; uiIndex++)
    {
        gnb_ReadWord(pthBuffer, &wData);
        ptstData->pnGoodHardBins[uiIndex] = gnm_WordToInt(wData);
    }

    gnb_ReadString(pthBuffer, ptstData->mMessage);

	*ptptData = (void*)ptstData;
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : init data in the GNM_TYPE_R_INIT structure.							*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_R_INIT data structure to be initialized.					*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_InitStruct_R_INIT(PT_GNM_R_INIT ptstData)
{
    unsigned int uiIndex=0;

	/* Initialize structure */
	ptstData->nStatus = GTC_STATUS_OK;
	ptstData->nGtlModules = 0;
	ptstData->uiFindTestKey = GTC_FINDTEST_TESTNUMBER;
	ptstData->uiResultFrequency = 1;
    ptstData->uiNbRunsBeforeProdInfo = 0;

    ptstData->uiNbGoodSBins = 1;
    ptstData->pnGoodSoftBins[0] = 1;
    ptstData->uiNbGoodHBins = 1;
    ptstData->pnGoodHardBins[0] = 1;

	for(uiIndex=1; uiIndex<256; uiIndex++)
        ptstData->pnGoodSoftBins[uiIndex] = -1;
    for(uiIndex=1; uiIndex<256; uiIndex++)
        ptstData->pnGoodHardBins[uiIndex] = -1;

    strcpy(ptstData->mMessage, "");

	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : free data in the GNM_TYPE_R_INIT structure.							*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_R_INIT data structure to be freeed.						*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_FreeStruct_R_INIT(ptstData)
PT_GNM_R_INIT	ptstData;
{
    /* If ptr is null, nothing to free */
    if(ptstData == NULL)
        return GNET_ERR_OKAY;

	/* Free structure */
	
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : add data from GNM_TYPE_PRODINFO structure to buffer.					*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that will receive the data.                                */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_PRODINFO data structure used to fill the buffer.			*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_AddDataFrom_PRODINFO(pthBuffer, ptstData)
PT_GNB_HBUFFER	pthBuffer;
PT_GNM_PRODINFO	ptstData;
{
	/* Add Data to buffer */
	gnb_AddString(pthBuffer, ptstData->szOperatorName);
	gnb_AddString(pthBuffer, ptstData->szJobRevision);
	gnb_AddString(pthBuffer, ptstData->szLotID);
	gnb_AddString(pthBuffer, ptstData->szSublotID);
	gnb_AddString(pthBuffer, ptstData->szProductID);
    gnb_AddInt4(pthBuffer, ptstData->mSplitlotID);
    gnb_AddInt4(pthBuffer, ptstData->mRetestIndex);

	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : read data from buffer to GNM_TYPE_PRODINFO structure.					*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that contains the data to read.                            */
/*                                                                                      */
/*     void** ptptData                                                                  */
/*         Ptr to the GNM_TYPE_PRODINFO data structure that will be filled by the buffer*/
/*         content.                                                                     */
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY if successfull.                                                    */
/*     GNET_ERR_MAF : memory allocation failure.                                        */
/*======================================================================================*/
static int gnm_ReadDataTo_PRODINFO(pthBuffer, ptptData)
PT_GNB_HBUFFER  pthBuffer;
void            **ptptData;
{
	PT_GNM_PRODINFO	ptstData;
	
	/* Alloc memory for data structure */
	ptstData = (PT_GNM_PRODINFO)malloc(sizeof(GNM_PRODINFO));
	if(!ptstData)
		return GNET_ERR_MAF;
	
	/* Initialize structure */
	gnm_InitStruct_PRODINFO(ptstData);

	/* Read Data from buffer */
	gnb_ReadString(pthBuffer, ptstData->szOperatorName);
	gnb_ReadString(pthBuffer, ptstData->szJobRevision);
	gnb_ReadString(pthBuffer, ptstData->szLotID);
	gnb_ReadString(pthBuffer, ptstData->szSublotID);
	gnb_ReadString(pthBuffer, ptstData->szProductID);
    gnb_ReadInt4(pthBuffer, &ptstData->mSplitlotID);
    gnb_ReadInt4(pthBuffer, &ptstData->mRetestIndex);

	*ptptData = (void*)ptstData;
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : init data in the GNM_TYPE_PRODINFO structure.							*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_PRODINFO data structure to be initialized.				*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_InitStruct_PRODINFO(ptstData)
PT_GNM_PRODINFO	ptstData;
{
	/* Initialize structure */
	strcpy(ptstData->szOperatorName, "");
	strcpy(ptstData->szJobRevision, "");
	strcpy(ptstData->szLotID, "");
	strcpy(ptstData->szSublotID, "");
	strcpy(ptstData->szProductID, "");
    ptstData->mSplitlotID=-1; // invalid
    ptstData->mRetestIndex=-1; // invalid

	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : free data in the GNM_TYPE_PRODINFO structure.							*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_PRODINFO data structure to be freeed.					*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_FreeStruct_PRODINFO(ptstData)
PT_GNM_PRODINFO	ptstData;
{
    /* If ptr is null, nothing to free */
    if(ptstData == NULL)
        return GNET_ERR_OKAY;

    /* Free structure */
	
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : add data from GNM_TYPE_Q_TESTLIST structure to buffer.				*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that will receive the data.                                */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_Q_TESTLIST data structure used to fill the buffer.		*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_AddDataFrom_Q_TESTLIST(pthBuffer, ptstData)
PT_GNB_HBUFFER		pthBuffer;
PT_GNM_Q_TESTLIST	ptstData;
{
	/* Add Data to buffer */
	gnb_AddDWord(pthBuffer, (GNB_DWORD)ptstData->uiTimeStamp );

	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : read data from buffer to GNM_TYPE_Q_TESTLIST structure.				*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that contains the data to read.                            */
/*                                                                                      */
/*     void** ptptData                                                                  */
/*         Ptr to the GNM_TYPE_Q_TESTLIST data structure that will be filled by the		*/
/*         buffer content.                                                              */
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY if successfull.                                                    */
/*     GNET_ERR_MAF : memory allocation failure.                                        */
/*======================================================================================*/
static int gnm_ReadDataTo_Q_TESTLIST(PT_GNB_HBUFFER pthBuffer, void **ptptData)
{
	GNB_DWORD			dwData;
	PT_GNM_Q_TESTLIST	ptstData;
	
	/* Alloc memory for data structure */
	ptstData = (PT_GNM_Q_TESTLIST)malloc(sizeof(GNM_Q_TESTLIST));
	if(!ptstData)
		return GNET_ERR_MAF;

	/* Initialize structure */
	gnm_InitStruct_Q_TESTLIST(ptstData);

	/* Read Data from buffer */
	gnb_ReadDWord(pthBuffer, &dwData);
	ptstData->uiTimeStamp = (unsigned int)dwData;
	
	*ptptData = (void*)ptstData;
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : init data in the GNM_TYPE_Q_TESTLIST structure.						*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_Q_TESTLIST data structure to be initialized.				*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_InitStruct_Q_TESTLIST(ptstData)
PT_GNM_Q_TESTLIST	ptstData;
{
	/* Initialize structure */
	ptstData->uiTimeStamp = 0;
	
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : free data in the GNM_TYPE_Q_TESTLIST structure.						*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_Q_TESTLIST data structure to be freeed.					*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_FreeStruct_Q_TESTLIST(ptstData)
PT_GNM_Q_TESTLIST	ptstData;
{
    /* If ptr is null, nothing to free */
    if(ptstData == NULL)
        return GNET_ERR_OKAY;

    /* Free structure */
	
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : add data from GNM_TYPE_R_TESTLIST structure to buffer.				*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that will receive the data.                                */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_R_TESTLIST data structure used to fill the buffer.		*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_AddDataFrom_R_TESTLIST(pthBuffer, ptstData)
PT_GNB_HBUFFER		pthBuffer;
PT_GNM_R_TESTLIST	ptstData;
{
    unsigned int uiTestIndex=0;

	/* Add Data to buffer */
	gnb_AddWord(pthBuffer, (GNB_WORD)ptstData->uiNbTests);

	/* For every test */
	for(uiTestIndex=0; uiTestIndex<ptstData->uiNbTests; uiTestIndex++)
	{
		gnb_AddDWord(pthBuffer, (GNB_DWORD)((ptstData->pstTestDefinitions)[uiTestIndex].lTestNumber));
        gnb_AddString(pthBuffer, (ptstData->pstTestDefinitions)[uiTestIndex].szTestName);
        gnb_AddString(pthBuffer, (ptstData->pstTestDefinitions)[uiTestIndex].szTestUnits);
        gnb_AddFloat(pthBuffer, (GNB_FLOAT)((ptstData->pstTestDefinitions)[uiTestIndex].fLowLimit));
        gnb_AddFloat(pthBuffer, (GNB_FLOAT)((ptstData->pstTestDefinitions)[uiTestIndex].fHighLimit));
        gnb_AddDWord(pthBuffer, (GNB_DWORD)((ptstData->pstTestDefinitions)[uiTestIndex].uiTestFlags));
        gnb_AddInt4(pthBuffer, (GNB_INT4)((ptstData->pstTestDefinitions)[uiTestIndex].mPinIndex));
    }
	
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : read data from buffer to GNM_TYPE_R_TESTLIST structure.				*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that contains the data to read.                            */
/*                                                                                      */
/*     void** ptptData                                                                  */
/*         Ptr to the GNM_TYPE_R_TESTLIST data structure that will be filled by the		*/
/*         buffer content.																*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY if successfull.                                                    */
/*     GNET_ERR_MAF : memory allocation failure.                                        */
/*======================================================================================*/
static int gnm_ReadDataTo_R_TESTLIST(pthBuffer, ptptData)
PT_GNB_HBUFFER  pthBuffer;
void            **ptptData;
{
    GNB_WORD wData;
    GNB_DWORD dwData;
    GNB_INT4 iData;
    GNB_FLOAT fData;
    PT_GNM_R_TESTLIST ptstData=0;
	unsigned int		uiTestIndex;
	unsigned long		ulIndex;
	
	/* Alloc memory for data structure */
	ptstData = (PT_GNM_R_TESTLIST)malloc(sizeof(GNM_R_TESTLIST));
	if(!ptstData)
		return GNET_ERR_MAF;
	
	/* Initialize structure */
	gnm_InitStruct_R_TESTLIST(ptstData);

	/* Read Data from buffer */
	gnb_ReadWord(pthBuffer, &wData);
	ptstData->uiNbTests = (unsigned int)wData;

	/* Alloc memory for test definition structures */
	ulIndex = ptstData->uiNbTests;
	if(ulIndex == 0)
		ptstData->pstTestDefinitions = NULL;
	else
	{
        ptstData->pstTestDefinitions = (PT_GNM_TESTDEF)malloc(ulIndex*sizeof(GNM_TESTDEF));
		if(!ptstData->pstTestDefinitions)
		{
			free(ptstData);
			return GNET_ERR_MAF;
		}

		/* Fill test definition structures */
		for(uiTestIndex=0; uiTestIndex<ptstData->uiNbTests; uiTestIndex++)
		{
			gnb_ReadDWord(pthBuffer, &dwData);
			(ptstData->pstTestDefinitions)[uiTestIndex].lTestNumber = (long)dwData;
			gnb_ReadString(pthBuffer, (ptstData->pstTestDefinitions)[uiTestIndex].szTestName);
            gnb_ReadString(pthBuffer, (ptstData->pstTestDefinitions)[uiTestIndex].szTestUnits);
            gnb_ReadFloat(pthBuffer, &fData);
            (ptstData->pstTestDefinitions)[uiTestIndex].fLowLimit = (float)fData;
            gnb_ReadFloat(pthBuffer, &fData);
            (ptstData->pstTestDefinitions)[uiTestIndex].fHighLimit = (float)fData;
            gnb_ReadDWord(pthBuffer, &dwData);
			(ptstData->pstTestDefinitions)[uiTestIndex].uiTestFlags = (unsigned int)dwData;
            gnb_ReadInt4(pthBuffer, &iData);
            (ptstData->pstTestDefinitions)[uiTestIndex].mPinIndex = (int)iData;
        }
	}

	*ptptData = (void*)ptstData;
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : init data in the GNM_TYPE_R_TESTLIST structure.						*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_R_TESTLIST data structure to be initialized.				*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_InitStruct_R_TESTLIST(ptstData)
PT_GNM_R_TESTLIST	ptstData;
{
	/* Initialize structure */
	ptstData->uiNbTests = 0;
	ptstData->pstTestDefinitions = NULL;
	
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : free data in the GNM_TYPE_R_TESTLIST structure.						*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_R_TESTLIST data structure to be freeed.					*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_FreeStruct_R_TESTLIST(PT_GNM_R_TESTLIST ptstData)
{
    /* If ptr is null, nothing to free */
    if(ptstData == NULL)
        return GNET_ERR_OKAY;

    /* Free structure */
	if(ptstData->pstTestDefinitions)
	{
		free(ptstData->pstTestDefinitions);
		ptstData->pstTestDefinitions = NULL;
	}
	
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : add data from GNM_TYPE_Q_PATCONFIG_STATIC structure to buffer.		*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that will receive the data.                                */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_Q_PATCONFIG_STATIC data structure used to fill the buffer*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_AddDataFrom_Q_PATCONFIG_STATIC(PT_GNB_HBUFFER pthBuffer, PT_GNM_Q_PATCONFIG_STATIC ptstData)
{
	/* Add Data to buffer */
	gnb_AddDWord(pthBuffer, (GNB_DWORD)ptstData->uiTimeStamp );

	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : read data from buffer to GNM_TYPE_Q_PATCONFIG_STATIC structure.		*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that contains the data to read.                            */
/*                                                                                      */
/*     void** ptptData                                                                  */
/*         Ptr to the GNM_TYPE_Q_PATCONFIG_STATIC data structure that will be filled by */
/*         the buffer content.															*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY if successfull.                                                    */
/*     GNET_ERR_MAF : memory allocation failure.                                        */
/*======================================================================================*/
static int gnm_ReadDataTo_Q_PATCONFIG_STATIC(PT_GNB_HBUFFER pthBuffer, void **ptptData)
{
    GNB_DWORD dwData;
    PT_GNM_Q_PATCONFIG_STATIC ptstData;

	/* Alloc memory for data structure */
	ptstData = (PT_GNM_Q_PATCONFIG_STATIC)malloc(sizeof(GNM_Q_PATCONFIG_STATIC));
	if(!ptstData)
		return GNET_ERR_MAF;

	/* Initialize structure */
	gnm_InitStruct_Q_PATCONFIG_STATIC(ptstData);

	/* Read Data from buffer */
	gnb_ReadDWord(pthBuffer, &dwData);
	ptstData->uiTimeStamp = (unsigned int)dwData;
	
	*ptptData = (void*)ptstData;
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : init data in the GNM_TYPE_Q_PATCONFIG_STATIC structure.				*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_Q_PATCONFIG_STATIC data structure to be initialized.		*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_InitStruct_Q_PATCONFIG_STATIC(ptstData)
PT_GNM_Q_PATCONFIG_STATIC	ptstData;
{
	/* Initialize structure */
	ptstData->uiTimeStamp = 0;
	
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : free data in the GNM_TYPE_Q_PATCONFIG_STATIC structure.				*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_Q_PATCONFIG_STATIC data structure to be freeed.			*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_FreeStruct_Q_PATCONFIG_STATIC(ptstData)
PT_GNM_Q_PATCONFIG_STATIC	ptstData;
{
    /* If ptr is null, nothing to free */
    if(ptstData == NULL)
        return GNET_ERR_OKAY;

    /* Free structure */	
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : add data from GNM_TYPE_R_PATCONFIG_STATIC structure to buffer.		*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that will receive the data.                                */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_R_PATCONFIG_STATIC data structure used to fill the buffer*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_AddDataFrom_R_PATCONFIG_STATIC(pthBuffer, ptstData)
PT_GNB_HBUFFER				pthBuffer;
PT_GNM_R_PATCONFIG_STATIC	ptstData;
{
    unsigned int	uiTestIndex=0, uiSiteIndex=0;
    unsigned long	ulIndex=0;

	/* Add Data to buffer */
	gnb_AddWord(pthBuffer, (GNB_WORD)ptstData->uiBaseline);
	gnb_AddWord(pthBuffer, (GNB_WORD)ptstData->uiFlags);
	gnb_AddWord(pthBuffer, (GNB_WORD)ptstData->uiNbSites);
	gnb_AddWord(pthBuffer, (GNB_WORD)ptstData->uiNbTests);

	/* For every test */
	for(uiSiteIndex=0; uiSiteIndex<ptstData->uiNbSites; uiSiteIndex++)
	{
		for(uiTestIndex=0; uiTestIndex<ptstData->uiNbTests; uiTestIndex++)
		{
			ulIndex = uiSiteIndex*ptstData->uiNbTests + uiTestIndex;
            gnb_AddDWord(pthBuffer, (GNB_DWORD)((ptstData->pstTestDefinitions)[ulIndex].mTestNumber));
            gnb_AddString(pthBuffer, (ptstData->pstTestDefinitions)[ulIndex].mTestName);
            gnb_AddInt4(pthBuffer, (ptstData->pstTestDefinitions)[ulIndex].mPinIndex); // Bernard ?
            gnb_AddFloat(pthBuffer, (GNB_FLOAT)((ptstData->pstTestDefinitions)[ulIndex].mLowLimit1));
            gnb_AddFloat(pthBuffer, (GNB_FLOAT)((ptstData->pstTestDefinitions)[ulIndex].mHighLimit1));
            gnb_AddDWord(pthBuffer, (GNB_DWORD)((ptstData->pstTestDefinitions)[ulIndex].mTestFlags));
            gnb_AddWord(pthBuffer, (GNB_WORD)((ptstData->pstTestDefinitions)[ulIndex].mPatBinning));
            /* Test stats */
            gnb_AddString(pthBuffer, (ptstData->pstTestDefinitions)[ulIndex].mTestStats.mDistributionShape);
            gnb_AddFloat(pthBuffer, (GNB_FLOAT)((ptstData->pstTestDefinitions)[ulIndex].mTestStats.mN_Factor));
            gnb_AddFloat(pthBuffer, (GNB_FLOAT)((ptstData->pstTestDefinitions)[ulIndex].mTestStats.mT_Factor));
            gnb_AddFloat(pthBuffer, (GNB_FLOAT)((ptstData->pstTestDefinitions)[ulIndex].mTestStats.mMean));
            gnb_AddFloat(pthBuffer, (GNB_FLOAT)((ptstData->pstTestDefinitions)[ulIndex].mTestStats.mSigma));
            gnb_AddFloat(pthBuffer, (GNB_FLOAT)((ptstData->pstTestDefinitions)[ulIndex].mTestStats.mMin));
            gnb_AddFloat(pthBuffer, (GNB_FLOAT)((ptstData->pstTestDefinitions)[ulIndex].mTestStats.mQ1));
            gnb_AddFloat(pthBuffer, (GNB_FLOAT)((ptstData->pstTestDefinitions)[ulIndex].mTestStats.mMedian));
            gnb_AddFloat(pthBuffer, (GNB_FLOAT)((ptstData->pstTestDefinitions)[ulIndex].mTestStats.mQ3));
            gnb_AddFloat(pthBuffer, (GNB_FLOAT)((ptstData->pstTestDefinitions)[ulIndex].mTestStats.mMax));
            gnb_AddWord(pthBuffer, (GNB_WORD)((ptstData->pstTestDefinitions)[ulIndex].mTestStats.mExecCount));
            gnb_AddWord(pthBuffer, (GNB_WORD)((ptstData->pstTestDefinitions)[ulIndex].mTestStats.mFailCount));
		}
	}
	
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : read data from buffer to GNM_TYPE_R_PATCONFIG_STATIC structure.		*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that contains the data to read.                            */
/*                                                                                      */
/*     void** ptptData                                                                  */
/*         Ptr to the GNM_TYPE_R_PATCONFIG_STATIC data structure that will be filled by	*/
/*         the buffer content.															*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY if successfull.                                                    */
/*     GNET_ERR_MAF : memory allocation failure.                                        */
/*======================================================================================*/
static int gnm_ReadDataTo_R_PATCONFIG_STATIC(pthBuffer, ptptData)
PT_GNB_HBUFFER  pthBuffer;
void            **ptptData;
{
    GNB_WORD wData;
    GNB_DWORD dwData;
    GNB_FLOAT fData;
    GNB_INT4 iData;
	PT_GNM_R_PATCONFIG_STATIC	ptstData;
	unsigned int				uiSiteIndex, uiTestIndex;
	unsigned long				ulIndex;
	
	/* Alloc memory for data structure */
	ptstData = (PT_GNM_R_PATCONFIG_STATIC)malloc(sizeof(GNM_R_PATCONFIG_STATIC));
	if(!ptstData)
		return GNET_ERR_MAF;
	
	/* Initialize structure */
	gnm_InitStruct_R_PATCONFIG_STATIC(ptstData);

	/* Read Data from buffer */
	gnb_ReadWord(pthBuffer, &wData);
	ptstData->uiBaseline = (unsigned int)wData;
	gnb_ReadWord(pthBuffer, &wData);
	ptstData->uiFlags = (unsigned int)wData;
	gnb_ReadWord(pthBuffer, &wData);
	ptstData->uiNbSites = (unsigned int)wData;
	gnb_ReadWord(pthBuffer, &wData);
	ptstData->uiNbTests = (unsigned int)wData;

	/* Alloc memory for test definition structures */
	ulIndex = ptstData->uiNbSites * ptstData->uiNbTests;
	if(ulIndex == 0)
		ptstData->pstTestDefinitions = NULL;
	else
	{
		ptstData->pstTestDefinitions = (PT_GNM_TESTDEF_STATICPAT)malloc(ulIndex*sizeof(GNM_TESTDEF_STATICPAT));
		if(!ptstData->pstTestDefinitions)
		{
			free(ptstData);
			return GNET_ERR_MAF;
		}

		/* Fill test definition structures */
		for(uiSiteIndex=0; uiSiteIndex<ptstData->uiNbSites; uiSiteIndex++)
		{
			for(uiTestIndex=0; uiTestIndex<ptstData->uiNbTests; uiTestIndex++)
			{
				ulIndex = uiSiteIndex*ptstData->uiNbTests + uiTestIndex;
				gnb_ReadDWord(pthBuffer, &dwData);
                (ptstData->pstTestDefinitions)[ulIndex].mTestNumber = (long)dwData;
                gnb_ReadString(pthBuffer, (ptstData->pstTestDefinitions)[ulIndex].mTestName);

                gnb_ReadInt4(pthBuffer, &iData);
                (ptstData->pstTestDefinitions)[ulIndex].mPinIndex = iData;

                gnb_ReadFloat(pthBuffer,&fData);
                (ptstData->pstTestDefinitions)[ulIndex].mLowLimit1 = (float)fData;
				gnb_ReadFloat(pthBuffer,&fData);
                (ptstData->pstTestDefinitions)[ulIndex].mHighLimit1 = (float)fData;
				gnb_ReadDWord(pthBuffer, &dwData);
                (ptstData->pstTestDefinitions)[ulIndex].mTestFlags = (unsigned int)dwData;
				gnb_ReadWord(pthBuffer, &wData);
                (ptstData->pstTestDefinitions)[ulIndex].mPatBinning = gnm_WordToInt(wData);
                /* Test Stats */
                gnb_ReadString(pthBuffer, (ptstData->pstTestDefinitions)[ulIndex].mTestStats.mDistributionShape);
                gnb_ReadFloat(pthBuffer,&fData);
                (ptstData->pstTestDefinitions)[ulIndex].mTestStats.mN_Factor = (float)fData;
                gnb_ReadFloat(pthBuffer,&fData);
                (ptstData->pstTestDefinitions)[ulIndex].mTestStats.mT_Factor = (float)fData;
                gnb_ReadFloat(pthBuffer,&fData);
                (ptstData->pstTestDefinitions)[ulIndex].mTestStats.mMean = (float)fData;
                gnb_ReadFloat(pthBuffer,&fData);
                (ptstData->pstTestDefinitions)[ulIndex].mTestStats.mSigma = (float)fData;
                gnb_ReadFloat(pthBuffer,&fData);
                (ptstData->pstTestDefinitions)[ulIndex].mTestStats.mMin = (float)fData;
                gnb_ReadFloat(pthBuffer,&fData);
                (ptstData->pstTestDefinitions)[ulIndex].mTestStats.mQ1 = (float)fData;
                gnb_ReadFloat(pthBuffer,&fData);
                (ptstData->pstTestDefinitions)[ulIndex].mTestStats.mMedian = (float)fData;
                gnb_ReadFloat(pthBuffer,&fData);
                (ptstData->pstTestDefinitions)[ulIndex].mTestStats.mQ3 = (float)fData;
                gnb_ReadFloat(pthBuffer,&fData);
                (ptstData->pstTestDefinitions)[ulIndex].mTestStats.mMax = (float)fData;
                gnb_ReadWord(pthBuffer, &wData);
                (ptstData->pstTestDefinitions)[ulIndex].mTestStats.mExecCount = gnm_WordToInt(wData);
                gnb_ReadWord(pthBuffer, &wData);
                (ptstData->pstTestDefinitions)[ulIndex].mTestStats.mFailCount = gnm_WordToInt(wData);
            }
		}
	}

	*ptptData = (void*)ptstData;
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : init data in the GNM_TYPE_R_PATCONFIG_STATIC structure.				*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_R_PATCONFIG_STATIC data structure to be initialized.		*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_InitStruct_R_PATCONFIG_STATIC(ptstData)
PT_GNM_R_PATCONFIG_STATIC	ptstData;
{
	/* Initialize structure */
	ptstData->uiBaseline = 0;
	ptstData->uiFlags = 0;
	ptstData->uiNbSites = 0;
	ptstData->uiNbTests = 0;
	ptstData->pstTestDefinitions = NULL;
	
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : free data in the GNM_TYPE_R_PATCONFIG_STATIC structure.				*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_R_PATCONFIG_STATIC data structure to be freeed.			*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_FreeStruct_R_PATCONFIG_STATIC(ptstData)
PT_GNM_R_PATCONFIG_STATIC	ptstData;
{
    /* If ptr is null, nothing to free */
    if(ptstData == NULL)
        return GNET_ERR_OKAY;

    /* Free structure */
	if(ptstData->pstTestDefinitions)
	{
		free(ptstData->pstTestDefinitions);
		ptstData->pstTestDefinitions = NULL;
	}
	
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : add data from GNM_TYPE_Q_PATCONFIG_DYNAMIC structure to buffer.		*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that will receive the data.                                */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_Q_PATCONFIG_DYNAMIC data structure used to fill the buffer*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_AddDataFrom_Q_PATCONFIG_DYNAMIC(PT_GNB_HBUFFER pthBuffer, PT_GNM_Q_PATCONFIG_DYNAMIC ptstData)
{
    /* Add Data to buffer */
    gnb_AddDWord(pthBuffer, (GNB_DWORD)ptstData->uiTimeStamp );

    return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : read data from buffer to GNM_TYPE_Q_PATCONFIG_DYNAMIC structure.		*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that contains the data to read.                            */
/*                                                                                      */
/*     void** ptptData                                                                  */
/*         Ptr to the GNM_TYPE_Q_PATCONFIG_DYNAMIC data structure that will be filled by */
/*         the buffer content.															*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY if successfull.                                                    */
/*     GNET_ERR_MAF : memory allocation failure.                                        */
/*======================================================================================*/
static int gnm_ReadDataTo_Q_PATCONFIG_DYNAMIC(PT_GNB_HBUFFER pthBuffer, void **ptptData)
{
    GNB_DWORD dwData;
    PT_GNM_Q_PATCONFIG_DYNAMIC ptstData;

    /* Alloc memory for data structure */
    ptstData = (PT_GNM_Q_PATCONFIG_DYNAMIC)malloc(sizeof(GNM_Q_PATCONFIG_DYNAMIC));
    if(!ptstData)
        return GNET_ERR_MAF;

    /* Initialize structure */
    gnm_InitStruct_Q_PATCONFIG_DYNAMIC(ptstData);

    /* Read Data from buffer */
    gnb_ReadDWord(pthBuffer, &dwData);
    ptstData->uiTimeStamp = (unsigned int)dwData;

    *ptptData = (void*)ptstData;
    return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : init data in the GNM_TYPE_Q_PATCONFIG_DYNAMIC structure.				*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_Q_PATCONFIG_DYNAMIC data structure to be initialized.		*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_InitStruct_Q_PATCONFIG_DYNAMIC(ptstData)
PT_GNM_Q_PATCONFIG_DYNAMIC	ptstData;
{
    /* Initialize structure */
    ptstData->uiTimeStamp = 0;

    return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : free data in the GNM_TYPE_Q_PATCONFIG_DYNAMIC structure.				*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_Q_PATCONFIG_DYNAMIC data structure to be freeed.			*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_FreeStruct_Q_PATCONFIG_DYNAMIC(ptstData)
PT_GNM_Q_PATCONFIG_DYNAMIC	ptstData;
{
    /* If ptr is null, nothing to free */
    if(ptstData == NULL)
        return GNET_ERR_OKAY;

    /* Free structure */
    return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : add data from GNM_TYPE_PATCONFIG_DYNAMIC structure to buffer.			*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that will receive the data.                                */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_PATCONFIG_DYNAMIC data structure used to fill the buffer	*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_AddDataFrom_PATCONFIG_DYNAMIC(pthBuffer, ptstData)
PT_GNB_HBUFFER				pthBuffer;
PT_GNM_PATCONFIG_DYNAMIC	ptstData;
{
	unsigned int	uiTestIndex, uiSiteIndex;
	unsigned long	ulIndex;

	/* Add Data to buffer */
	gnb_AddWord(pthBuffer, (GNB_WORD)ptstData->uiNbSites);
	gnb_AddWord(pthBuffer, (GNB_WORD)ptstData->uiNbTests);

	/* For every test */
	for(uiSiteIndex=0; uiSiteIndex<ptstData->uiNbSites; uiSiteIndex++)
	{
		for(uiTestIndex=0; uiTestIndex<ptstData->uiNbTests; uiTestIndex++)
		{
			ulIndex = uiSiteIndex*ptstData->uiNbTests + uiTestIndex;
            gnb_AddDWord(pthBuffer, (GNB_DWORD)((ptstData->pstTestDefinitions)[ulIndex].mSiteNb));
            gnb_AddDWord(pthBuffer, (GNB_DWORD)((ptstData->pstTestDefinitions)[ulIndex].mTestNumber));
            gnb_AddString(pthBuffer, (ptstData->pstTestDefinitions)[ulIndex].mTestName);
            gnb_AddInt4(pthBuffer, (ptstData->pstTestDefinitions)[ulIndex].mPinIndex);

            gnb_AddFloat(pthBuffer, (GNB_FLOAT)((ptstData->pstTestDefinitions)[ulIndex].mLowLimit1));
            gnb_AddFloat(pthBuffer, (GNB_FLOAT)((ptstData->pstTestDefinitions)[ulIndex].mHighLimit1));
            gnb_AddFloat(pthBuffer, (GNB_FLOAT)((ptstData->pstTestDefinitions)[ulIndex].mLowLimit2));
            gnb_AddFloat(pthBuffer, (GNB_FLOAT)((ptstData->pstTestDefinitions)[ulIndex].mHighLimit2));
            gnb_AddDWord(pthBuffer, (GNB_DWORD)((ptstData->pstTestDefinitions)[ulIndex].mTestFlags));
            gnb_AddWord(pthBuffer, (GNB_WORD)((ptstData->pstTestDefinitions)[ulIndex].mPatBinning));
            /* Test stats */
            gnb_AddString(pthBuffer, (ptstData->pstTestDefinitions)[ulIndex].mTestStats.mDistributionShape);
            gnb_AddFloat(pthBuffer, (GNB_FLOAT)((ptstData->pstTestDefinitions)[ulIndex].mTestStats.mN_Factor));
            gnb_AddFloat(pthBuffer, (GNB_FLOAT)((ptstData->pstTestDefinitions)[ulIndex].mTestStats.mT_Factor));
            gnb_AddFloat(pthBuffer, (GNB_FLOAT)((ptstData->pstTestDefinitions)[ulIndex].mTestStats.mMean));
            gnb_AddFloat(pthBuffer, (GNB_FLOAT)((ptstData->pstTestDefinitions)[ulIndex].mTestStats.mSigma));
            gnb_AddFloat(pthBuffer, (GNB_FLOAT)((ptstData->pstTestDefinitions)[ulIndex].mTestStats.mMin));
            gnb_AddFloat(pthBuffer, (GNB_FLOAT)((ptstData->pstTestDefinitions)[ulIndex].mTestStats.mQ1));
            gnb_AddFloat(pthBuffer, (GNB_FLOAT)((ptstData->pstTestDefinitions)[ulIndex].mTestStats.mMedian));
            gnb_AddFloat(pthBuffer, (GNB_FLOAT)((ptstData->pstTestDefinitions)[ulIndex].mTestStats.mQ3));
            gnb_AddFloat(pthBuffer, (GNB_FLOAT)((ptstData->pstTestDefinitions)[ulIndex].mTestStats.mMax));
            gnb_AddWord(pthBuffer, (GNB_WORD)((ptstData->pstTestDefinitions)[ulIndex].mTestStats.mExecCount));
            gnb_AddWord(pthBuffer, (GNB_WORD)((ptstData->pstTestDefinitions)[ulIndex].mTestStats.mFailCount));
        }
	}
	
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : add data from GNM_TYPE_PATCONFIG_DYNAMIC_2 structure to buffer.		*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that will receive the data.                                */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_PATCONFIG_DYNAMIC_2 data structure used to fill the      */
/*         buffer                                                                       */
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_AddDataFrom_PATCONFIG_DYNAMIC_2(pthBuffer, ptstData)
PT_GNB_HBUFFER				pthBuffer;
PT_GNM_PATCONFIG_DYNAMIC_2	ptstData;
{
    unsigned int                uiTestIndex, uiSiteIndex;
    PT_GNM_SITE_DYNAMICPAT      lSite=NULL;
    PT_GNM_TESTDEF_DYNAMICPAT   lTestDefinition=NULL;

    /* Add Data to buffer */
    gnb_AddWord(pthBuffer, (GNB_WORD)ptstData->uiNbSites);
    gnb_AddWord(pthBuffer, (GNB_WORD)ptstData->uiNbTests);

    /* For every test */
    for(uiSiteIndex=0; uiSiteIndex<ptstData->uiNbSites; uiSiteIndex++)
    {
        lSite = ptstData->pstSitesDynamicPatConfig+uiSiteIndex;
        if(lSite)
        {
            gnb_AddWord(pthBuffer, (GNB_WORD)lSite->uiSiteNb);
            for(uiTestIndex=0; uiTestIndex<ptstData->uiNbTests; uiTestIndex++)
            {
                lTestDefinition=lSite->pstTestDefinitions+uiTestIndex;
                if(lTestDefinition)
                {
                    gnb_AddDWord(pthBuffer, (GNB_DWORD)(lTestDefinition->mTestNumber));
                    gnb_AddString(pthBuffer, lTestDefinition->mTestName);
                    gnb_AddInt4(pthBuffer, lTestDefinition->mPinIndex);
                    gnb_AddFloat(pthBuffer, (GNB_FLOAT)(lTestDefinition->mLowLimit1));
                    gnb_AddFloat(pthBuffer, (GNB_FLOAT)(lTestDefinition->mHighLimit1));
                    gnb_AddFloat(pthBuffer, (GNB_FLOAT)(lTestDefinition->mLowLimit2));
                    gnb_AddFloat(pthBuffer, (GNB_FLOAT)(lTestDefinition->mHighLimit2));
                    gnb_AddDWord(pthBuffer, (GNB_DWORD)(lTestDefinition->mTestFlags));
                    gnb_AddWord(pthBuffer, (GNB_WORD)(lTestDefinition->mPatBinning));
                }
            }
        }
    }

    return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : read data from buffer to GNM_TYPE_PATCONFIG_DYNAMIC structure.		*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that contains the data to read.                            */
/*                                                                                      */
/*     void** ptptData                                                                  */
/*         Ptr to the GNM_TYPE_PATCONFIG_DYNAMIC data structure that will be filled by	*/
/*         the buffer content.															*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY if successfull.                                                    */
/*     GNET_ERR_MAF : memory allocation failure.                                        */
/*======================================================================================*/
static int gnm_ReadDataTo_PATCONFIG_DYNAMIC(pthBuffer, ptptData)
PT_GNB_HBUFFER  pthBuffer;
void            **ptptData;
{
    GNB_WORD wData;
    GNB_DWORD dwData;
    GNB_FLOAT fData;
    GNB_INT4 iData;
	PT_GNM_PATCONFIG_DYNAMIC	ptstData;
	unsigned int				uiSiteIndex, uiTestIndex;
	unsigned long				ulIndex;
	
	/* Alloc memory for data structure */
	ptstData = (PT_GNM_PATCONFIG_DYNAMIC)malloc(sizeof(GNM_PATCONFIG_DYNAMIC));
	if(!ptstData)
		return GNET_ERR_MAF;
	
	/* Initialize structure */
	gnm_InitStruct_PATCONFIG_DYNAMIC(ptstData);

	/* Read Data from buffer */
	gnb_ReadWord(pthBuffer, &wData);
	ptstData->uiNbSites = (unsigned int)wData;
	gnb_ReadWord(pthBuffer, &wData);
	ptstData->uiNbTests = (unsigned int)wData;

	/* Alloc memory for test definition structures */
	ulIndex = ptstData->uiNbSites * ptstData->uiNbTests;
	if(ulIndex == 0)
		ptstData->pstTestDefinitions = NULL;
	else
	{
		ptstData->pstTestDefinitions = (PT_GNM_TESTDEF_DYNAMICPAT)malloc(ulIndex*sizeof(GNM_TESTDEF_DYNAMICPAT));
		if(!ptstData->pstTestDefinitions)
		{
			free(ptstData);
			return GNET_ERR_MAF;
		}

		/* Fill test definition structures */
		for(uiSiteIndex=0; uiSiteIndex<ptstData->uiNbSites; uiSiteIndex++)
		{
			for(uiTestIndex=0; uiTestIndex<ptstData->uiNbTests; uiTestIndex++)
			{
				ulIndex = uiSiteIndex*ptstData->uiNbTests + uiTestIndex;
                gnb_ReadDWord(pthBuffer, &dwData);
                (ptstData->pstTestDefinitions)[ulIndex].mSiteNb = (int)dwData;
                gnb_ReadDWord(pthBuffer, &dwData);
                (ptstData->pstTestDefinitions)[ulIndex].mTestNumber = (long)dwData;
                gnb_ReadString(pthBuffer, (ptstData->pstTestDefinitions)[ulIndex].mTestName);

                gnb_ReadInt4(pthBuffer, &iData);
                (ptstData->pstTestDefinitions)[ulIndex].mPinIndex = (int)iData;

                gnb_ReadFloat(pthBuffer,&fData);
                (ptstData->pstTestDefinitions)[ulIndex].mLowLimit1 = (float)fData;
				gnb_ReadFloat(pthBuffer,&fData);
                (ptstData->pstTestDefinitions)[ulIndex].mHighLimit1 = (float)fData;
				gnb_ReadFloat(pthBuffer,&fData);
                (ptstData->pstTestDefinitions)[ulIndex].mLowLimit2 = (float)fData;
				gnb_ReadFloat(pthBuffer,&fData);
                (ptstData->pstTestDefinitions)[ulIndex].mHighLimit2 = (float)fData;
				gnb_ReadDWord(pthBuffer, &dwData);
                (ptstData->pstTestDefinitions)[ulIndex].mTestFlags = (unsigned int)dwData;
				gnb_ReadWord(pthBuffer, &wData);
                (ptstData->pstTestDefinitions)[ulIndex].mPatBinning = gnm_WordToInt(wData);
                /* Test Stats */
                gnb_ReadString(pthBuffer, (ptstData->pstTestDefinitions)[ulIndex].mTestStats.mDistributionShape);
                gnb_ReadFloat(pthBuffer,&fData);
                (ptstData->pstTestDefinitions)[ulIndex].mTestStats.mN_Factor = (float)fData;
                gnb_ReadFloat(pthBuffer,&fData);
                (ptstData->pstTestDefinitions)[ulIndex].mTestStats.mT_Factor = (float)fData;
                gnb_ReadFloat(pthBuffer,&fData);
                (ptstData->pstTestDefinitions)[ulIndex].mTestStats.mMean = (float)fData;
                gnb_ReadFloat(pthBuffer,&fData);
                (ptstData->pstTestDefinitions)[ulIndex].mTestStats.mSigma = (float)fData;
                gnb_ReadFloat(pthBuffer,&fData);
                (ptstData->pstTestDefinitions)[ulIndex].mTestStats.mMin = (float)fData;
                gnb_ReadFloat(pthBuffer,&fData);
                (ptstData->pstTestDefinitions)[ulIndex].mTestStats.mQ1 = (float)fData;
                gnb_ReadFloat(pthBuffer,&fData);
                (ptstData->pstTestDefinitions)[ulIndex].mTestStats.mMedian = (float)fData;
                gnb_ReadFloat(pthBuffer,&fData);
                (ptstData->pstTestDefinitions)[ulIndex].mTestStats.mQ3 = (float)fData;
                gnb_ReadFloat(pthBuffer,&fData);
                (ptstData->pstTestDefinitions)[ulIndex].mTestStats.mMax = (float)fData;
                gnb_ReadWord(pthBuffer, &wData);
                (ptstData->pstTestDefinitions)[ulIndex].mTestStats.mExecCount = gnm_WordToInt(wData);
                gnb_ReadWord(pthBuffer, &wData);
                (ptstData->pstTestDefinitions)[ulIndex].mTestStats.mFailCount = gnm_WordToInt(wData);

			}
		}
	}

	*ptptData = (void*)ptstData;
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : read data from buffer to GNM_TYPE_PATCONFIG_DYNAMIC_2 structure.		*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that contains the data to read.                            */
/*                                                                                      */
/*     void** ptptData                                                                  */
/*         Ptr to the GNM_TYPE_PATCONFIG_DYNAMIC_2 data structure that will be filled 	*/
/*         by the buffer content.														*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY if successfull.                                                    */
/*     GNET_ERR_MAF : memory allocation failure.                                        */
/*======================================================================================*/
static int gnm_ReadDataTo_PATCONFIG_DYNAMIC_2(pthBuffer, ptptData)
PT_GNB_HBUFFER  pthBuffer;
void            **ptptData;
{
    GNB_WORD					wData;
    GNB_DWORD					dwData;
    GNB_FLOAT					fData;
    GNB_INT4 iData;
    PT_GNM_PATCONFIG_DYNAMIC_2	ptstData;
    PT_GNM_SITE_DYNAMICPAT      lSite=NULL;
    unsigned int				uiSiteIndex, uiTestIndex;

    /* Alloc memory for data structure */
    ptstData = (PT_GNM_PATCONFIG_DYNAMIC_2)malloc(sizeof(GNM_PATCONFIG_DYNAMIC_2));
    if(!ptstData)
        return GNET_ERR_MAF;

    /* Initialize structure */
    gnm_InitStruct_PATCONFIG_DYNAMIC_2(ptstData);

    /* Read Data from buffer */
    gnb_ReadWord(pthBuffer, &wData);
    ptstData->uiNbSites = (unsigned int)wData;
    gnb_ReadWord(pthBuffer, &wData);
    ptstData->uiNbTests = (unsigned int)wData;

    /* Alloc memory for site structures */
    if(ptstData->uiNbSites == 0)
        ptstData->pstSitesDynamicPatConfig = NULL;
    else
    {
        ptstData->pstSitesDynamicPatConfig = (PT_GNM_SITE_DYNAMICPAT)malloc(ptstData->uiNbSites*sizeof(GNM_SITE_DYNAMICPAT));
        if(!ptstData->pstSitesDynamicPatConfig)
        {
            free(ptstData);
            return GNET_ERR_MAF;
        }

        /* Init pointers to NULL, so we can use freeStruct if any malloc fails */
        for(uiSiteIndex=0; uiSiteIndex<ptstData->uiNbSites; uiSiteIndex++)
        {
            lSite = ptstData->pstSitesDynamicPatConfig+uiSiteIndex;
            lSite->pstTestDefinitions=NULL;
        }

        /* For each site, fill site structure */
        for(uiSiteIndex=0; uiSiteIndex<ptstData->uiNbSites; uiSiteIndex++)
        {
            lSite = ptstData->pstSitesDynamicPatConfig+uiSiteIndex;
            /* Malloc & Fill test definition structures */
            lSite->pstTestDefinitions = (PT_GNM_TESTDEF_DYNAMICPAT)malloc(ptstData->uiNbTests*sizeof(GNM_TESTDEF_DYNAMICPAT));
            if(!lSite->pstTestDefinitions)
            {
                gnm_FreeStruct_PATCONFIG_DYNAMIC_2(ptstData);
                free(ptstData);
                return GNET_ERR_MAF;
            }

            for(uiTestIndex=0; uiTestIndex<ptstData->uiNbTests; uiTestIndex++)
            {
                gnb_ReadDWord(pthBuffer, &dwData);
                (lSite->pstTestDefinitions)[uiTestIndex].mTestNumber = (long)dwData;
                gnb_ReadString(pthBuffer, (lSite->pstTestDefinitions)[uiTestIndex].mTestName);

                gnb_ReadInt4(pthBuffer, &iData);
                (lSite->pstTestDefinitions)[uiTestIndex].mPinIndex=(int)iData;

                gnb_ReadFloat(pthBuffer,&fData);
                (lSite->pstTestDefinitions)[uiTestIndex].mLowLimit1 = (float)fData;
                gnb_ReadFloat(pthBuffer,&fData);
                (lSite->pstTestDefinitions)[uiTestIndex].mHighLimit1 = (float)fData;
                gnb_ReadFloat(pthBuffer,&fData);
                (lSite->pstTestDefinitions)[uiTestIndex].mLowLimit2 = (float)fData;
                gnb_ReadFloat(pthBuffer,&fData);
                (lSite->pstTestDefinitions)[uiTestIndex].mHighLimit2 = (float)fData;
                gnb_ReadDWord(pthBuffer, &dwData);
                (lSite->pstTestDefinitions)[uiTestIndex].mTestFlags = (unsigned int)dwData;
                gnb_ReadWord(pthBuffer, &wData);
                (lSite->pstTestDefinitions)[uiTestIndex].mPatBinning = gnm_WordToInt(wData);
            }
        }
    }

    *ptptData = (void*)ptstData;
    return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : init data in the GNM_TYPE_PATCONFIG_DYNAMIC structure.				*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_PATCONFIG_DYNAMIC data structure to be initialized.		*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_InitStruct_PATCONFIG_DYNAMIC(PT_GNM_PATCONFIG_DYNAMIC ptstData)
{
	/* Initialize structure */
	ptstData->uiNbSites = 0;
	ptstData->uiNbTests = 0;
	ptstData->pstTestDefinitions = NULL;
	
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : init data in the GNM_TYPE_PATCONFIG_DYNAMIC_2 structure.				*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_PATCONFIG_DYNAMIC_2 data structure to be initialized.    */
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_InitStruct_PATCONFIG_DYNAMIC_2(ptstData)
PT_GNM_PATCONFIG_DYNAMIC_2	ptstData;
{
    /* Initialize structure */
    ptstData->uiNbSites = 0;
    ptstData->uiNbTests = 0;
    ptstData->pstSitesDynamicPatConfig = NULL;

    return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : free data in the GNM_TYPE_PATCONFIG_DYNAMIC structure.				*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_PATCONFIG_DYNAMIC data structure to be freeed.			*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_FreeStruct_PATCONFIG_DYNAMIC(PT_GNM_PATCONFIG_DYNAMIC ptstData)
{
    /* If ptr is null, nothing to free */
    if(ptstData == NULL)
        return GNET_ERR_OKAY;

    /* Free structure */
	if(ptstData->pstTestDefinitions)
	{
		free(ptstData->pstTestDefinitions);
		ptstData->pstTestDefinitions = NULL;
	}
	
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : free data in the GNM_TYPE_PATCONFIG_DYNAMIC_2 structure.				*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_PATCONFIG_DYNAMIC_2 data structure to be freeed.			*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_FreeStruct_PATCONFIG_DYNAMIC_2(PT_GNM_PATCONFIG_DYNAMIC_2 ptstData)
{
    PT_GNM_SITE_DYNAMICPAT  lSite=NULL;
    unsigned int            lIndex=0;

    /* If ptr is null, nothing to free */
    if(ptstData == NULL)
        return GNET_ERR_OKAY;

    /* Free structures */
    for(lIndex=0; lIndex<ptstData->uiNbSites; ++lIndex)
    {
        lSite = ptstData->pstSitesDynamicPatConfig+lIndex;
        if(lSite)
        {
            if(lSite->pstTestDefinitions != NULL)
            {
                free(lSite->pstTestDefinitions);
                lSite->pstTestDefinitions=NULL;
            }
        }
    }

    if(ptstData->pstSitesDynamicPatConfig)
    {
        free(ptstData->pstSitesDynamicPatConfig);
        ptstData->pstSitesDynamicPatConfig=NULL;
    }

    return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : add data from GNM_TYPE_RESULTS structure to buffer.					*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that will receive the data.                                */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_RESULTS data structure used to fill the buffer.			*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_AddDataFrom_RESULTS(PT_GNB_HBUFFER pthBuffer, PT_GNM_RESULTS	 ptstData)
{
    unsigned int uiNbSites = ptstData->mNbSites;
    unsigned int uiNbTests = ptstData->mNbTests;
    unsigned int uiNbRuns = ptstData->mNbAllocatedRuns;
    unsigned int uiSiteIndex, uiTestIndex, uiRunIndex;
    unsigned long ulSiteIndex, ulTestIndex, ulIndex;

	/* Add Data to buffer */
    gnb_AddWord(pthBuffer, (GNB_WORD)ptstData->mPacketNb);
    gnb_AddWord(pthBuffer, (GNB_WORD)ptstData->mNbSites);
    gnb_AddWord(pthBuffer, (GNB_WORD)ptstData->mNbTests);
    gnb_AddWord(pthBuffer, (GNB_WORD)ptstData->mNbAllocatedRuns);
    gnb_AddWord(pthBuffer, (GNB_WORD)ptstData->mNbValidRuns);
    gnb_AddWord(pthBuffer, (GNB_WORD)ptstData->mSilentMode);

	/* Test results */
	for(uiSiteIndex=0; uiSiteIndex<uiNbSites; uiSiteIndex++)
	{
        ulSiteIndex = uiSiteIndex*uiNbTests*uiNbRuns;
        for(uiTestIndex=0; uiTestIndex<uiNbTests; uiTestIndex++)
        {
            ulTestIndex = ulSiteIndex + uiTestIndex*uiNbRuns;
            for(uiRunIndex=0; uiRunIndex<uiNbRuns; uiRunIndex++)
            {
                gnb_AddFloat(pthBuffer, (GNB_FLOAT)(ptstData->mTestResults[ulTestIndex + uiRunIndex].mValue));
                gnb_AddByte(pthBuffer, (GNB_BYTE)(ptstData->mTestResults[ulTestIndex + uiRunIndex].mFlags));
			}
		}
	}

	/* Run results */
	for(uiSiteIndex=0; uiSiteIndex<uiNbSites; uiSiteIndex++)
	{
		ulSiteIndex = uiSiteIndex*uiNbRuns;
		for(uiRunIndex=0; uiRunIndex<uiNbRuns; uiRunIndex++)
		{
            gnb_AddWord(pthBuffer, (GNB_WORD)(ptstData->mRunResults[ulSiteIndex + uiRunIndex].mOrgSoftBin));
            gnb_AddWord(pthBuffer, (GNB_WORD)(ptstData->mRunResults[ulSiteIndex + uiRunIndex].mOrgHardBin));
            gnb_AddWord(pthBuffer, (GNB_WORD)(ptstData->mRunResults[ulSiteIndex + uiRunIndex].mPatSoftBin));
            gnb_AddWord(pthBuffer, (GNB_WORD)(ptstData->mRunResults[ulSiteIndex + uiRunIndex].mPatHardBin));
            gnb_AddWord(pthBuffer, (GNB_WORD)(ptstData->mRunResults[ulSiteIndex + uiRunIndex].mNbTestsExecuted));
            gnb_AddString(pthBuffer, ptstData->mRunResults[ulSiteIndex + uiRunIndex].mPartID);
            gnb_AddByte(pthBuffer, (GNB_BYTE)(ptstData->mRunResults[ulSiteIndex + uiRunIndex].mFlags));
            gnb_AddWord(pthBuffer, (GNB_WORD)(ptstData->mRunResults[ulSiteIndex + uiRunIndex].mPartIndex));
        }
	}

	/* Test statistics */
	for(uiSiteIndex=0; uiSiteIndex<uiNbSites; uiSiteIndex++)
	{
		ulSiteIndex = uiSiteIndex*uiNbTests;
		for(uiTestIndex=0; uiTestIndex<uiNbTests; uiTestIndex++)
		{
			ulIndex = ulSiteIndex + uiTestIndex;
            gnb_AddWord(pthBuffer, (GNB_WORD)(ptstData->mTestStats[ulIndex].mSiteNb));
            gnb_AddDWord(pthBuffer, (GNB_DWORD)(ptstData->mTestStats[ulIndex].lTestNumber));
            gnb_AddString(pthBuffer, ptstData->mTestStats[ulIndex].mTestName);
            gnb_AddInt4(pthBuffer, (GNB_INT4)(ptstData->mTestStats[ulIndex].mPinIndex));
            gnb_AddWord(pthBuffer, (GNB_WORD)(ptstData->mTestStats[ulIndex].uiExecs));
		}
	}
	
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : read data from buffer to GNM_TYPE_RESULTS structure.					*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that contains the data to read.                            */
/*                                                                                      */
/*     void** ptptData                                                                  */
/*         Ptr to the GNM_TYPE_RESULTS data structure that will be filled by the		*/
/*         buffer content.                                                              */
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY if successfull.                                                    */
/*     GNET_ERR_MAF : memory allocation failure.                                        */
/*======================================================================================*/
static int gnm_ReadDataTo_RESULTS(PT_GNB_HBUFFER pthBuffer, void **ptptData)
{
    GNB_FLOAT fData;
    GNB_BYTE bData;
    GNB_DWORD dwData;
    GNB_WORD wData;
    GNB_INT4 intData;
    PT_GNM_RESULTS	ptstData;
    unsigned int uiNbSites, uiNbTests, uiNbRuns;
    unsigned int uiSiteIndex, uiTestIndex, uiRunIndex;
    unsigned long ulSiteIndex, ulTestIndex, ulIndex;
	
	/* Alloc memory for data structure */
	ptstData = (PT_GNM_RESULTS)malloc(sizeof(GNM_RESULTS));
	if(!ptstData)
		return GNET_ERR_MAF;
	
	/* Initialize structure */
	gnm_InitStruct_RESULTS(ptstData);

	/* Read Data from buffer */
	gnb_ReadWord(pthBuffer, &wData);
    ptstData->mPacketNb = (unsigned int)wData;
	gnb_ReadWord(pthBuffer, &wData);
    uiNbSites = ptstData->mNbSites = (unsigned int)wData;
	gnb_ReadWord(pthBuffer, &wData);
    uiNbTests = ptstData->mNbTests = (unsigned int)wData;
	gnb_ReadWord(pthBuffer, &wData);
    uiNbRuns = ptstData->mNbAllocatedRuns = (unsigned int)wData;
	gnb_ReadWord(pthBuffer, &wData);
    ptstData->mNbValidRuns = (unsigned int)wData;
	gnb_ReadWord(pthBuffer, &wData);
    ptstData->mSilentMode = (unsigned int)wData;

	ulIndex = uiNbSites * uiNbTests * uiNbRuns;
	if(ulIndex == 0)
	{
        ptstData->mTestResults = NULL;
        ptstData->mRunResults = NULL;
        ptstData->mTestStats = NULL;
	}
	else
	{
		/* Test results */
        ptstData->mTestResults = (PT_GNM_TESTRESULT)malloc(ulIndex*sizeof(GNM_TESTRESULT));
        if(!ptstData->mTestResults)
		{
			free(ptstData);
			return GNET_ERR_MAF;
		}
		for(uiSiteIndex=0; uiSiteIndex<uiNbSites; uiSiteIndex++)
		{
            ulSiteIndex = uiSiteIndex*uiNbTests*uiNbRuns;
            for(uiTestIndex=0; uiTestIndex<uiNbTests; uiTestIndex++)
            {
                ulTestIndex = ulSiteIndex + uiTestIndex*uiNbRuns;
                for(uiRunIndex=0; uiRunIndex<uiNbRuns; uiRunIndex++)
                {
                    gnb_ReadFloat(pthBuffer, &fData);
                    ptstData->mTestResults[ulTestIndex + uiRunIndex].mValue = (float)fData;
                    gnb_ReadByte(pthBuffer, &bData);
                    ptstData->mTestResults[ulTestIndex + uiRunIndex].mFlags = (char)bData;
                }
			}
		}

		/* Run results */
        ptstData->mRunResults = (PT_GNM_RUNRESULT)malloc(uiNbSites*uiNbRuns*sizeof(GNM_RUNRESULT));
        if(!ptstData->mRunResults)
		{
            free(ptstData->mTestResults);
			free(ptstData);
			return GNET_ERR_MAF;
		}
		for(uiSiteIndex=0; uiSiteIndex<uiNbSites; uiSiteIndex++)
		{
			ulSiteIndex = uiSiteIndex*uiNbRuns;
			for(uiRunIndex=0; uiRunIndex<uiNbRuns; uiRunIndex++)
			{
				gnb_ReadWord(pthBuffer, &wData);
                ptstData->mRunResults[ulSiteIndex + uiRunIndex].mOrgSoftBin = gnm_WordToInt(wData);
                gnb_ReadWord(pthBuffer, &wData);
                ptstData->mRunResults[ulSiteIndex + uiRunIndex].mOrgHardBin = gnm_WordToInt(wData);
                gnb_ReadWord(pthBuffer, &wData);
                ptstData->mRunResults[ulSiteIndex + uiRunIndex].mPatSoftBin = gnm_WordToInt(wData);
                gnb_ReadWord(pthBuffer, &wData);
                ptstData->mRunResults[ulSiteIndex + uiRunIndex].mPatHardBin = gnm_WordToInt(wData);
                gnb_ReadWord(pthBuffer, &wData);
                ptstData->mRunResults[ulSiteIndex + uiRunIndex].mNbTestsExecuted = (unsigned int)wData;
                gnb_ReadString(pthBuffer, ptstData->mRunResults[ulSiteIndex + uiRunIndex].mPartID);
                gnb_ReadByte(pthBuffer, &bData);
                ptstData->mRunResults[ulSiteIndex + uiRunIndex].mFlags = (char)bData;
                gnb_ReadWord(pthBuffer, &wData);
                ptstData->mRunResults[ulSiteIndex + uiRunIndex].mPartIndex = (unsigned int)wData;
            }
		}

		/* Test statistics */
        ptstData->mTestStats = (PT_GNM_TESTSTAT_RESULTS)malloc(uiNbSites*uiNbTests*sizeof(GNM_TESTSTAT_RESULTS));
        if(!ptstData->mTestStats)
		{
            free(ptstData->mRunResults);
            free(ptstData->mTestResults);
			free(ptstData);
			return GNET_ERR_MAF;
		}
		for(uiSiteIndex=0; uiSiteIndex<uiNbSites; uiSiteIndex++)
		{
			ulSiteIndex = uiSiteIndex*uiNbTests;
			for(uiTestIndex=0; uiTestIndex<uiNbTests; uiTestIndex++)
			{
				ulIndex = ulSiteIndex + uiTestIndex;
				gnb_ReadWord(pthBuffer, &wData);
                ptstData->mTestStats[ulIndex].mSiteNb = (unsigned int)wData;
				gnb_ReadDWord(pthBuffer, &dwData);
                ptstData->mTestStats[ulIndex].lTestNumber = (long)dwData;
                gnb_ReadString(pthBuffer, ptstData->mTestStats[ulIndex].mTestName);

                gnb_ReadInt4(pthBuffer, &intData);
                ptstData->mTestStats[ulIndex].mPinIndex = (int)intData;

                gnb_ReadWord(pthBuffer, &wData);
                ptstData->mTestStats[ulIndex].uiExecs = (unsigned int)wData;
			}
		}
	}
	
	*ptptData = (void*)ptstData;
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : init data in the GNM_TYPE_RESULTS structure.							*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_RESULTS data structure to be initialized.				*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_InitStruct_RESULTS(ptstData)
PT_GNM_RESULTS	ptstData;
{
	/* Initialize structure */
    ptstData->mPacketNb = 0;
    ptstData->mNbSites = 0;
    ptstData->mNbTests = 0;
    ptstData->mNbAllocatedRuns = 0;
    ptstData->mNbValidRuns = 0;
    ptstData->mSilentMode = 0;
    ptstData->mTestResults = NULL;
    ptstData->mRunResults = NULL;
    ptstData->mTestStats = NULL;
	
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : free data in the GNM_TYPE_RESULTS structure.							*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_RESULTS data structure to be freeed.						*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_FreeStruct_RESULTS(ptstData)
PT_GNM_RESULTS	ptstData;
{
    /* If ptr is null, nothing to free */
    if(ptstData == NULL)
        return GNET_ERR_OKAY;

    /* Free structure */
    if(ptstData->mTestResults)
	{
        free(ptstData->mTestResults);
        ptstData->mTestResults = NULL;
	}
    if(ptstData->mRunResults)
	{
        free(ptstData->mRunResults);
        ptstData->mRunResults = NULL;
	}
    if(ptstData->mTestStats)
	{
        free(ptstData->mTestStats);
        ptstData->mTestStats = NULL;
	}
	
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : add data from GNM_TYPE_NOTIFY structure to buffer.					*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that will receive the data.                                */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_NOTIFY data structure used to fill the buffer.			*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_AddDataFrom_NOTIFY(PT_GNB_HBUFFER pthBuffer, PT_GNM_NOTIFY ptstData)
{
	/* Add Data to buffer */
	gnb_AddDWord(pthBuffer, (GNB_DWORD)ptstData->nTimeout);
	gnb_AddString(pthBuffer, ptstData->szMessage);
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : read data from buffer to GNM_TYPE_NOTIFY structure.					*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that contains the data to read.                            */
/*                                                                                      */
/*     void** ptptData                                                                  */
/*         Ptr to the GNM_TYPE_NOTIFY data structure that will be filled by the			*/
/*         buffer content.                                                              */
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY if successfull.                                                    */
/*     GNET_ERR_MAF : memory allocation failure.                                        */
/*======================================================================================*/
static int gnm_ReadDataTo_NOTIFY(PT_GNB_HBUFFER pthBuffer, void **ptptData)
{
    GNB_DWORD dwData;
    PT_GNM_NOTIFY ptstData=0;

	/* Alloc memory for data structure */
	ptstData = (PT_GNM_NOTIFY)malloc(sizeof(GNM_NOTIFY));
	if(!ptstData)
		return GNET_ERR_MAF;

	/* Initialize structure */
	gnm_InitStruct_NOTIFY(ptstData);

	/* Read Data from buffer */
	gnb_ReadDWord(pthBuffer, &dwData);
	ptstData->nTimeout = (int)dwData;
	gnb_ReadString(pthBuffer, ptstData->szMessage);

	*ptptData = (void*)ptstData;
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : init data in the GNM_TYPE_NOTIFY structure.							*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_NOTIFY data structure to be initialized.					*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_InitStruct_NOTIFY(ptstData)
PT_GNM_NOTIFY	ptstData;
{
	/* Initialize structure */
	ptstData->nTimeout = 0;
	strcpy(ptstData->szMessage, "");
	
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : free data in the GNM_TYPE_NOTIFY structure.							*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_NOTIFY data structure to be freeed.						*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_FreeStruct_NOTIFY(ptstData)
PT_GNM_NOTIFY	ptstData;
{
    /* If ptr is null, nothing to free */
    if(ptstData == NULL)
        return GNET_ERR_OKAY;

    /* Free structure */
	
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : add data from GNM_TYPE_COMMAND structure to buffer.					*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that will receive the data.                                */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_COMMAND data structure used to fill the buffer.			*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_AddDataFrom_COMMAND(pthBuffer, ptstData)
PT_GNB_HBUFFER	pthBuffer;
PT_GNM_COMMAND	ptstData;
{
	/* Add Data to buffer */
	gnb_AddWord(pthBuffer, (GNB_WORD)ptstData->uiCommandID);
	gnb_AddString(pthBuffer, ptstData->szCommand);
	
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : read data from buffer to GNM_TYPE_COMMAND structure.					*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that contains the data to read.                            */
/*                                                                                      */
/*     void** ptptData                                                                  */
/*         Ptr to the GNM_TYPE_COMMAND data structure that will be filled by the		*/
/*         buffer content.                                                              */
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY if successfull.                                                    */
/*     GNET_ERR_MAF : memory allocation failure.                                        */
/*======================================================================================*/
static int gnm_ReadDataTo_COMMAND(pthBuffer, ptptData)
PT_GNB_HBUFFER  pthBuffer;
void            **ptptData;
{
	GNB_WORD		wData;
	PT_GNM_COMMAND	ptstData;
	
	/* Alloc memory for data structure */
	ptstData = (PT_GNM_COMMAND)malloc(sizeof(GNM_COMMAND));
	if(!ptstData)
		return GNET_ERR_MAF;
	
	/* Initialize structure */
	gnm_InitStruct_COMMAND(ptstData);

	/* Read Data from buffer */
	gnb_ReadWord(pthBuffer, &wData);
	ptstData->uiCommandID = (unsigned int)wData;
	gnb_ReadString(pthBuffer, ptstData->szCommand);
	
	*ptptData = (void*)ptstData;
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : init data in the GNM_TYPE_COMMAND structure.							*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_COMMAND data structure to be initialized.				*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_InitStruct_COMMAND(ptstData)
PT_GNM_COMMAND	ptstData;
{
	/* Initialize structure */
	ptstData->uiCommandID = GTC_GTLCOMMAND_NONE;
	strcpy(ptstData->szCommand, "");
	
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : free data in the GNM_TYPE_COMMAND structure.							*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_COMMAND data structure to be freeed.						*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_FreeStruct_COMMAND(ptstData)
PT_GNM_COMMAND	ptstData;
{
    /* If ptr is null, nothing to free */
    if(ptstData == NULL)
        return GNET_ERR_OKAY;

    /* Free structure */
	
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : add data from GNM_TYPE_WRITETOSTDF structure to buffer.				*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that will receive the data.                                */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_WRITETOSTDF data structure used to fill the buffer.		*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_AddDataFrom_WRITETOSTDF(pthBuffer, ptstData)
PT_GNB_HBUFFER		pthBuffer;
PT_GNM_WRITETOSTDF	ptstData;
{
	/* Add Data to buffer */
	gnb_AddWord(pthBuffer, (GNB_WORD)ptstData->nRecTyp);
	gnb_AddWord(pthBuffer, (GNB_WORD)ptstData->nRecSub);
	gnb_AddString(pthBuffer, ptstData->szString);
	
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : read data from buffer to GNM_TYPE_WRITETOSTDF structure.				*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that contains the data to read.                            */
/*                                                                                      */
/*     void** ptptData                                                                  */
/*         Ptr to the GNM_TYPE_WRITETOSTDF data structure that will be filled by the	*/
/*         buffer content.                                                              */
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY if successfull.                                                    */
/*     GNET_ERR_MAF : memory allocation failure.                                        */
/*======================================================================================*/
static int gnm_ReadDataTo_WRITETOSTDF(pthBuffer, ptptData)
PT_GNB_HBUFFER  pthBuffer;
void            **ptptData;
{
	GNB_WORD			wData;
	PT_GNM_WRITETOSTDF	ptstData;
	
	/* Alloc memory for data structure */
	ptstData = (PT_GNM_WRITETOSTDF)malloc(sizeof(GNM_WRITETOSTDF));
	if(!ptstData)
		return GNET_ERR_MAF;
	
	/* Initialize structure */
	gnm_InitStruct_WRITETOSTDF(ptstData);

	/* Read Data from buffer */
	gnb_ReadWord(pthBuffer, &wData);
	ptstData->nRecTyp = (int)wData;
	gnb_ReadWord(pthBuffer, &wData);
	ptstData->nRecSub = (int)wData;
	gnb_ReadString(pthBuffer, ptstData->szString);
	
	*ptptData = (void*)ptstData;
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : init data in the GNM_TYPE_WRITETOSTDF structure.						*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_WRITETOSTDF data structure to be initialized.			*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_InitStruct_WRITETOSTDF(ptstData)
PT_GNM_WRITETOSTDF	ptstData;
{
	/* Initialize structure */
	ptstData->nRecTyp = 0;
	ptstData->nRecSub = 0;
	strcpy(ptstData->szString, "");
	
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : free data in the GNM_TYPE_WRITETOSTDF structure.						*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_WRITETOSTDF data structure to be freeed.					*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_FreeStruct_WRITETOSTDF(ptstData)
PT_GNM_WRITETOSTDF	ptstData;
{
    /* If ptr is null, nothing to free */
    if(ptstData == NULL)
        return GNET_ERR_OKAY;

    /* Free structure */
	
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : add data from GNM_TYPE_Q_ENDLOT structure to buffer.					*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that will receive the data.                                */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_Q_ENDLOT data structure used to fill the buffer.			*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_AddDataFrom_Q_ENDLOT(PT_GNB_HBUFFER pthBuffer, PT_GNM_Q_ENDLOT ptstData)
{
	/* Add Data to buffer */
	gnb_AddDWord(pthBuffer, (GNB_DWORD)ptstData->uiTimeStamp);	
	return GNET_ERR_OKAY;
}

static int gnm_AddDataFrom_Q_END_SPLITLOT(PT_GNB_HBUFFER pthBuffer, PT_GNM_Q_END_OF_SPLITLOT ptstData)
{
    gnb_AddDWord(pthBuffer, (GNB_DWORD)ptstData->mTimeStamp);
    return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : read data from buffer to GNM_TYPE_Q_ENDLOT structure.					*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that contains the data to read.                            */
/*                                                                                      */
/*     void** ptptData                                                                  */
/*         Ptr to the GNM_TYPE_Q_ENDLOT data structure that will be filled by the			*/
/*         buffer content.                                                              */
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY if successfull.                                                    */
/*     GNET_ERR_MAF : memory allocation failure.                                        */
/*======================================================================================*/
static int gnm_ReadDataTo_Q_ENDLOT(PT_GNB_HBUFFER pthBuffer, void **ptptData)
{
    GNB_DWORD dwData;
    PT_GNM_Q_ENDLOT	ptstData=0;
	
	/* Alloc memory for data structure */
    ptstData = (PT_GNM_Q_ENDLOT)malloc(sizeof(GNM_Q_ENDLOT));
	if(!ptstData)
		return GNET_ERR_MAF;
	
	/* Initialize structure */
    gnm_InitStruct_Q_ENDLOT(ptstData);

	/* Read Data from buffer */
	gnb_ReadDWord(pthBuffer, &dwData);
	ptstData->uiTimeStamp = (unsigned int)dwData;
	
	*ptptData = (void*)ptstData;
	return GNET_ERR_OKAY;
}

static int gnm_ReadDataTo_Q_END_SPLITLOT(PT_GNB_HBUFFER pthBuffer, void **ptptData)
{
    GNB_DWORD dwData;
    PT_GNM_Q_END_OF_SPLITLOT ptstData=0;

    /* Alloc memory for data structure */
    ptstData = (PT_GNM_Q_END_OF_SPLITLOT)malloc(sizeof(GNM_Q_END_OF_SPLITLOT));
    if(!ptstData)
        return GNET_ERR_MAF;

    /* Initialize structure */
    gnm_InitStruct_Q_END_SPLITLOT(ptstData);

    /* Read Data from buffer */
    gnb_ReadDWord(pthBuffer, &dwData);
    ptstData->mTimeStamp = (unsigned int)dwData;

    *ptptData = (void*)ptstData;
    return GNET_ERR_OKAY;
}


/*======================================================================================*/
/* Description  : init data in the GNM_TYPE_Q_ENDLOT structure.							*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_Q_ENDLOT data structure to be initialized.					*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_InitStruct_Q_ENDLOT(PT_GNM_Q_ENDLOT ptstData)
{
	/* Initialize structure */
	ptstData->uiTimeStamp = 0;	
	return GNET_ERR_OKAY;
}

static int gnm_InitStruct_Q_END_SPLITLOT(PT_GNM_Q_END_OF_SPLITLOT ptstData)
{
    /* Initialize structure */
    ptstData->mTimeStamp = 0;
    return GNET_ERR_OKAY;
}


/*======================================================================================*/
/* Description  : free data in the GNM_TYPE_Q_ENDLOT structure.							*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_Q_ENDLOT data structure to be freeed.						*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_FreeStruct_Q_ENDLOT(PT_GNM_Q_ENDLOT ptstData)
{
    /* If ptr is null, nothing to free */
    if(ptstData == NULL)
        return GNET_ERR_OKAY;
    /* Free structure */
	return GNET_ERR_OKAY;
}

static int gnm_FreeStruct_Q_END_SPLITLOT(PT_GNM_Q_END_OF_SPLITLOT ptstData)
{
    /* If ptr is null, nothing to free */
    if(ptstData == NULL)
        return GNET_ERR_OKAY;
    /* Free structure */
    return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : add data from GNM_TYPE_R_ENDLOT structure to buffer.                  */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that will receive the data.                                */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_R_ENDLOT data structure used to fill the buffer.         */
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_AddDataFrom_R_ENDLOT(PT_GNB_HBUFFER pthBuffer, PT_GNM_R_ENDLOT ptstData)
{
    /* Add Data to buffer */
    gnb_AddDWord(pthBuffer, (GNB_DWORD)ptstData->mTimeStamp);
    gnb_AddDWord(pthBuffer, (GNB_DWORD)ptstData->mStatus);
    return GNET_ERR_OKAY;
}

static int gnm_AddDataFrom_R_END_SPLITLOT(PT_GNB_HBUFFER pthBuffer, PT_GNM_R_END_OF_SPLITLOT ptstData)
{
    /* Add Data to buffer */
    gnb_AddDWord(pthBuffer, (GNB_DWORD)ptstData->mTimeStamp);
    gnb_AddDWord(pthBuffer, (GNB_DWORD)ptstData->mStatus);
    return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : read data from buffer to GNM_TYPE_R_ENDLOT structure.                 */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that contains the data to read.                            */
/*                                                                                      */
/*     void** ptptData                                                                  */
/*         Ptr to the GNM_TYPE_R_ENDLOT data structure that will be filled by the buffer*/
/*         content.                                                                     */
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY if successfull.                                                    */
/*     GNET_ERR_MAF : memory allocation failure.                                        */
/*======================================================================================*/
static int gnm_ReadDataTo_R_ENDLOT(PT_GNB_HBUFFER pthBuffer, void **ptptData)
{
    GNB_DWORD dwData;
    PT_GNM_R_ENDLOT ptstData=0;

    /* Alloc memory for data structure */
    ptstData = (PT_GNM_R_ENDLOT)malloc(sizeof(GNM_R_ENDLOT));
    if(!ptstData)
        return GNET_ERR_MAF;

    /* Initialize structure */
    gnm_InitStruct_R_ENDLOT(ptstData);

    /* Read Data from buffer */
    gnb_ReadDWord(pthBuffer, &dwData);
    ptstData->mTimeStamp = (unsigned int)dwData;
    gnb_ReadDWord(pthBuffer, &dwData);
    ptstData->mStatus = (int)dwData;

    *ptptData = (void*)ptstData;
    return GNET_ERR_OKAY;
}

static int gnm_ReadDataTo_R_END_SPLITLOT(PT_GNB_HBUFFER pthBuffer, void **ptptData)
{
    GNB_DWORD dwData;
    PT_GNM_R_END_OF_SPLITLOT ptstData=0;

    /* Alloc memory for data structure */
    ptstData = (PT_GNM_R_END_OF_SPLITLOT)malloc(sizeof(GNM_R_END_OF_SPLITLOT));
    if(!ptstData)
        return GNET_ERR_MAF;

    /* Initialize structure */
    gnm_InitStruct_R_END_SPLITLOT(ptstData);

    /* Read Data from buffer */
    gnb_ReadDWord(pthBuffer, &dwData);
    ptstData->mTimeStamp = (unsigned int)dwData;
    gnb_ReadDWord(pthBuffer, &dwData);
    ptstData->mStatus = (int)dwData;

    *ptptData = (void*)ptstData;
    return GNET_ERR_OKAY;
}


/*======================================================================================*/
/* Description  : init data in the GNM_TYPE_R_ENDLOT structure.             			*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_R_ENDLOT data structure to be initialized.       		*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_InitStruct_R_ENDLOT(PT_GNM_R_ENDLOT ptstData)
{
    /* Initialize structure */
    ptstData->mTimeStamp = 0;
    ptstData->mStatus = 0;
    return GNET_ERR_OKAY;
}

static int gnm_InitStruct_R_END_SPLITLOT(PT_GNM_R_END_OF_SPLITLOT ptstData)
{
    ptstData->mTimeStamp = 0;
    ptstData->mStatus = 0;
    return GNET_ERR_OKAY;
}


/*======================================================================================*/
/* Description  : free data in the GNM_TYPE_R_ENDLOT structure.							*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_R_ENDLOT data structure to be freeed.            		*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_FreeStruct_R_ENDLOT(PT_GNM_R_ENDLOT ptstData)
{
    /* If pointer is null, nothing to free */
    if(ptstData == NULL)
        return GNET_ERR_OKAY;
    return GNET_ERR_OKAY;
}

static int gnm_FreeStruct_R_END_SPLITLOT(PT_GNM_R_END_OF_SPLITLOT ptstData)
{
    /* If ptr is null, nothing to free */
    if(ptstData == NULL)
        return GNET_ERR_OKAY;
    /* Free structure */
    return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : add data from GNM_TYPE_HEARTBEAT structure to buffer.					*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that will receive the data.                                */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_HEARTBEAT data structure used to fill the buffer.		*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_AddDataFrom_HEARTBEAT(pthBuffer, ptstData)
PT_GNB_HBUFFER      pthBuffer;
PT_GNM_HEARTBEAT	ptstData;
{
	/* Add Data to buffer */
    gnb_AddDWord(pthBuffer, (GNB_DWORD)ptstData->mTimeStamp);
	
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : read data from buffer to GNM_TYPE_HEARTBEAT structure.				*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GNB_HBUFFER*	phBuffer                                                            */
/*         Ptr to the buffer that contains the data to read.                            */
/*                                                                                      */
/*     void** ptptData                                                                  */
/*         Ptr to the GNM_TYPE_HEARTBEAT data structure that will be filled by the		*/
/*         buffer content.                                                              */
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY if successfull.                                                    */
/*     GNET_ERR_MAF : memory allocation failure.                                        */
/*======================================================================================*/
static int gnm_ReadDataTo_HEARTBEAT(PT_GNB_HBUFFER pthBuffer, void **ptptData)
{
    GNB_DWORD dwData;
    PT_GNM_HEARTBEAT ptstData=0;
	
	/* Alloc memory for data structure */
	ptstData = (PT_GNM_HEARTBEAT)malloc(sizeof(GNM_HEARTBEAT));
	if(!ptstData)
		return GNET_ERR_MAF;
	
	/* Initialize structure */
	gnm_InitStruct_HEARTBEAT(ptstData);

	/* Read Data from buffer */
	gnb_ReadDWord(pthBuffer, &dwData);
    ptstData->mTimeStamp = (unsigned int)dwData;
	
	*ptptData = (void*)ptstData;
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : init data in the GNM_TYPE_HEARTBEAT structure.						*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_HEARTBEAT data structure to be initialized.				*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_InitStruct_HEARTBEAT(ptstData)
PT_GNM_HEARTBEAT	ptstData;
{
	/* Initialize structure */
    ptstData->mTimeStamp = 0;
	
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : free data in the GNM_TYPE_HEARTBEAT structure.						*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptstData                                                                   */
/*         Ptr to the GNM_TYPE_HEARTBEAT data structure to be freeed.					*/
/*                                                                                      */
/* Return Codes :                                                                       */
/*     GNET_ERR_OKAY.                                                                   */
/*======================================================================================*/
static int gnm_FreeStruct_HEARTBEAT(ptstData)
PT_GNM_HEARTBEAT	ptstData;
{
    /* If ptr is null, nothing to free */
    if(ptstData == NULL)
        return GNET_ERR_OKAY;

    /* Free structure */
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : copy src string to dest buffer with control of buffer size.           */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*     char* dest                                                                       */
/*         Ptr. to destination buffer                                                   */
/*     const char* src                                                                  */
/*         Ptr. to source buffer                                                        */
/*     size_t size                                                                      */
/*         Max nb. of bytes to copy                                                     */
/*                                                                                      */
/* Return Codes :                                                                       */
/*======================================================================================*/
void gtcnm_CopyString(dest, src, size)
char* dest;
const char* src;
size_t size;
{
    /* Check pointers */
    if(!dest  || !src || size==0)
        return;

    /* copy string */
    strncpy(dest, src, size);

    /* make sure string is null-terminated */
    dest[size-1]='\0';
}

