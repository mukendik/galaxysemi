/****************************************************************************************/
/* Copyright Galaxy                                                                     */
/* This computer program is protected by copyright law                                  */
/* and international treaties. Unauthorized reproduction or                             */
/* distribution of this program, or any portion of it,may                               */
/* result in severe civil and criminal penalties, and will be                           */
/* prosecuted to the maximum extent possible under the law.                             */
/****************************************************************************************/
/*                                                                                      */
/* Module: gtl_message.h                                                                */
/*                                                                                      */
/* This module handles the different messages sent by the GTL to the screen or to the   */
/* log files.									                                        */
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

#ifndef _GTL_MESSAGE_H_
#define _GTL_MESSAGE_H_

/*======================================================================================*/
/* Includes                                                                             */
/*======================================================================================*/

/*======================================================================================*/
/* Defines                                                                              */
/*======================================================================================*/
/* Globals */
#define GTL_MSG_MAXSIZE							256		/* Maximum length of a message string */
#define GTL_MSG_CREATE							0		/* Open file in create mode */
#define GTL_MSG_APPEND							1		/* Open file in append mode */
#define GTL_MSG_TODISPLAY						0		/* Send message to display */
#define GTL_MSG_TOBUFFER						1		/* Send message to a buffer  */
#define GTL_MSG_TOLOGONLY						2		/* Send message only to log file */
	
/* Error Codes */
#define GTL_MSG_ERR_OK							0		/* No error */

/* Message headers */
#define GTL_ERR_LABEL							"[GTL-ERROR] : " 
#define GTL_INFO_LABEL							"[GTL-INFO]  : " 
#define GTL_WARN_LABEL							"[GTL-WARN]  : " 
#define GTL_DEBUG_LABEL							"[GTL-DEBUG] : " 



/*======================================================================================*/
/* Macros                                                                               */
/*======================================================================================*/
extern int gtl_nDebugMsg_ON;
#define gtl_msg_DebugMacro(x,y,z)	if(gtl_nDebugMsg_ON) gtl_msg_Debug(x,y,z)

/*======================================================================================*/
/* PUBLIC Functions                                                                     */
/*======================================================================================*/
#ifdef _GTL_MESSAGE_MODULE_

    void gtl_msg_InitLogfileNames();
    void gtl_msg_Info();
    int gtl_msg_Error();
    void gtl_msg_Warning();
    void gtl_msg_Debug();
    void gtl_msg_GetGTLStateString();
    void gtl_msg_DisplayHelp();

#else

extern void	gtl_msg_InitLogfileNames();
extern void	gtl_msg_Info();
extern int	gtl_msg_Error();
extern void	gtl_msg_Warning();
extern void	gtl_msg_Debug();
extern void	gtl_msg_GetGTLStateString();
extern void	gtl_msg_DisplayHelp();

#endif /* #ifdef _GTL_MESSAGE_MODULE_ */

#endif /* #ifndef _GTL_MESSAGE_H_ */

