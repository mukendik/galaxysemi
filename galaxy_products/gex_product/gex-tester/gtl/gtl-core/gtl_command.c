#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gstdl_utils_c.h>
#include "gtl_core.h"
#include "gtl_message.h"
#include "gtl_constants.h"
#include "gtl_testlist.h"

extern int gtl_IsDisabled();
extern int gtl_IsInitialized();
extern void gtl_DisplayInfo();
extern int gtl_DisplayKeys();
extern void gtl_DisplayStatus();
extern GTL_GlobalInfo gtl_stGlobalInfo; /* GTL Global information */
extern GTL_ProdInfo gtl_stProdInfo;	// Tester Production information

#ifdef GTL_LOG_OPENCLOSE_TIME
struct timeval gOpenTv;
#endif

int gtl_command(const char *lCommand)
{
    /* Check for null */
    if (!lCommand)
        return GTL_CORE_ERR_INVALID_COMMAND;

    char* lPos=NULL;
    const int lCommandLenght=(int)strlen(lCommand);
    if (lCommandLenght<=0)
        return GTL_CORE_ERR_INVALID_COMMAND;

    //GTL_LOG(5, (char *)"Command: %s", (char*)lCommand);

    long lData=0;
    int  lRes=GTL_CORE_ERR_OKAY;

    if(strcmp(lCommand, GTL_COMMAND_OPEN)==0)
    {
        // No GTL_LOG() outside open/close, so let's open first, the log
        lRes = gtl_open();
#ifdef GTL_LOG_OPENCLOSE_TIME
        if(gettimeofday(&gOpenTv, NULL) == 0)
        {
            GTL_LOG(5, (char *)"GTL_COMMAND(OPEN) called at %lu seconds and %lu useconds from epoch.", (unsigned long)gOpenTv.tv_sec, (unsigned long)gOpenTv.tv_usec);
    }
        else
        {
            GTL_LOG(5, (char *)"Failed to get time for GTL_COMMAND(OPEN).", 0);
        }
#endif
        return lRes;
    }

    if(strcmp(lCommand, GTL_COMMAND_RETEST)==0)
    {
        return gtl_retest(gtl_stGlobalInfo.mDesiredLimits, gtl_stProdInfo.mRetestHbins);
    }

    if(strcmp(lCommand, GTL_COMMAND_CLEAR_MESSAGES_STACK)==0)
    {
        return gtl_clear_messages_stack();
    }

    if(strcmp(lCommand, GTL_COMAND_POP_FIRST_MESSAGE)==0)
    {
        return gtl_pop_first_message(&gtl_stGlobalInfo.mCurrentMessageSeverity,
                                     gtl_stGlobalInfo.mCurrentMessage, &gtl_stGlobalInfo.mCurrentMessageID );
    }
    if(strcmp(lCommand, GTL_COMAND_POP_LAST_MESSAGE)==0)
    {
        return gtl_pop_last_message(&gtl_stGlobalInfo.mCurrentMessageSeverity,
                                     gtl_stGlobalInfo.mCurrentMessage, &gtl_stGlobalInfo.mCurrentMessageID );
    }

    if(strcmp(lCommand, GTL_COMMAND_QUERY)==0)
    {
        // Skip white spaces
        char* lSelectPtr=gtl_stGlobalInfo.mQueryBuffer;
        int lSelectLength=strlen(gtl_stGlobalInfo.mQueryBuffer);
        while(((lSelectPtr-gtl_stGlobalInfo.mQueryBuffer)<lSelectLength) &&
              ((*lSelectPtr==' ') || (*lSelectPtr=='\t') || (*lSelectPtr=='\n')))
            lSelectPtr++;
        // Make sure SQL command is allowed (only "select" for now)
        if ( (UT_strnicmp(lSelectPtr, "select", 6)!=0)
             && (getenv("GS_GTL_ROOT")==0) )
        {
            GTL_LOG(3, (char *)"Illegal query", 0);
            return GTL_CORE_ERR_ILLEGAL_QUERY;
        }
        return gtl_SQLiteExecQuery(gtl_stGlobalInfo.mQueryBuffer, gtl_stGlobalInfo.mQueryOutputFileName);
    }

    // Let allow close even if gtl not init just for logs
    if(strcmp(lCommand, GTL_COMMAND_CLOSE)==0)
    {
        // No GTL_LOG() outside open/close, so let's log first, the close
#ifdef GTL_LOG_OPENCLOSE_TIME
        struct timeval tv;
        if(gettimeofday(&tv, NULL) == 0)
        {
            GTL_LOG(5, (char *)"GTL_COMMAND(CLOSE) called at %lu seconds and %lu useconds from epoch.", (unsigned long)tv.tv_sec, (unsigned long)tv.tv_usec);
            if(tv.tv_usec < gOpenTv.tv_usec)
            {
                tv.tv_usec += 1000000;
                tv.tv_sec -= 1;
            }
            GTL_LOG(5, (char *)"OPEN to CLOSE duration is %lu seconds and %lu useconds (excluding OPEN and CLOSE).", (unsigned long)(tv.tv_sec-gOpenTv.tv_sec), (unsigned long)(tv.tv_usec-gOpenTv.tv_usec));
        }
        else
        {
            GTL_LOG(5, (char *)"Failed to get time for GTL_COMMAND(CLOSE).",0);
        }
#endif
        return gtl_close();
    }

    /* Make sure GTL is initialized */
    if(!gtl_IsInitialized())
        return GTL_CORE_ERR_LIB_NOT_INITIALIZED;


    /* If GTL disabled, return error code */
    if(gtl_IsDisabled())
        return GTL_CORE_ERR_DISABLED;

    if(strcmp(lCommand, GTL_COMMAND_BEGINJOB)==0)
        return gtl_beginjob();
    if(strcmp(lCommand, GTL_COMMAND_ENDJOB)==0)
        return gtl_endjob();

    /* -gtl_debugon */
    if(strcmp(lCommand, "-gtl_debugon")==0)
    {
        gtl_nDebugMsg_ON = 1;
        return GTL_CORE_ERR_OKAY;
    }

    /* -gtl_debugoff */
    if(strcmp(lCommand, "-gtl_debugoff")==0)
    {
        gtl_nDebugMsg_ON = 0;
        return GTL_CORE_ERR_OKAY;
    }

#if 0 // Deprecated for now since GTL V3.6
    /* -gtl_disabled */
    if(!strcmp(lCommand,"-gtl_disabled"))
    {
        gtl_SetState_Disabled(GTL_DISABLED_ROOTCAUSE_USERCMD);
        return GTL_CORE_ERR_OKAY;
    }
#endif

    /* -gtl_quieton */
    if(strcmp(lCommand,"-gtl_quieton")==0)
    {
        gtl_stGlobalInfo.nUserOptions |= GTL_USEROPTION_QUIETON;
        return GTL_CORE_ERR_OKAY;
    }

    /* -gtl_quietoff */
    if(strcmp(lCommand,"-gtl_quietoff")==0)
    {
        gtl_stGlobalInfo.nUserOptions &= ~GTL_USEROPTION_QUIETON;
        return GTL_CORE_ERR_OKAY;
    }

    /* -gtl_info */
    if(strcmp(lCommand,"-gtl_info")==0)
    {
        gtl_DisplayInfo();
        return GTL_CORE_ERR_OKAY;
    }

#if 0 // Deprecated for now since GTL V3.6
    /* -gtl_reset */
    if(strcmp(lCommand,"-gtl_reset")==0)
    {
        int lR=gtl_reset();
        if (lR!=GTL_CORE_ERR_OKAY)
            GTL_LOG(3, (char *)"gtl_reset failed: %d", lR);
        return lR;
    }
#endif

    /* -gtl_help */
    if(strcmp(lCommand,"-gtl_help")==0)
    {
        gtl_msg_DisplayHelp();
        return GTL_CORE_ERR_OKAY;
    }

    /* -gtl_status */
    if(strcmp(lCommand,"-gtl_status")==0)
    {
        gtl_DisplayStatus();
        return GTL_CORE_ERR_OKAY;
    }

    /* -gtl_keys */
    if(strcmp(lCommand,"-gtl_keys")==0)
    {
        gtl_DisplayKeys();
        return GTL_CORE_ERR_OKAY;
    }

    /* -gtl_test <test number> */
    if(strncmp(lCommand,"-gtl_test ",10)==0)
    {
        if(lCommandLenght < 11)
            return GTL_CORE_ERR_INVALID_COMMAND;

        /* go to next word */
        lPos = (char*)lCommand+10;
        while((lPos<lCommand+lCommandLenght) && (*lPos==' ') && (*lPos!='\0')) ++lPos;
        if((lPos>=lCommand+lCommandLenght) || (*lPos=='\0'))
            return GTL_CORE_ERR_INVALID_COMMAND;

        /* extract test# */
        if(sscanf(lPos,"%ld",&lData)!=1)
            return GTL_CORE_ERR_INVALID_COMMAND;

        /* display test settings */
        gtl_tl_DisplayTest(lData);
        return GTL_CORE_ERR_OKAY;
    }

    /* -gtl_testlist */
    if(strcmp(lCommand,"-gtl_testlist")==0)
    {
        gtl_tl_DisplayTestlist();
        return GTL_CORE_ERR_OKAY;
    }

    /* Argument not a valid GTL command : display a warning */
    gtl_msg_Warning(GTL_MSGWARN_MAIN_INVCMD, lCommand);
    gtl_msg_DisplayHelp();
    return GTL_CORE_ERR_INVALID_COMMAND;
}

