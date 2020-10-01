#include <string.h>
#include <stdlib.h>
#include <errno.h> // for remove failure
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <gstdl_utils_c.h>

#include "gtl_getset.h" // For private KEYS
#include "gtl_message.h"
#include "gtl_core.h"
#include "gtl_constants.h"
#include "gtc_netmessage.h"

extern GTL_GlobalInfo gtl_stGlobalInfo; // GTL Global information
extern GTL_ProdInfo gtl_stProdInfo;	// Tester Production information
extern GTL_Station gtl_stStation; // Tester station information
extern int gtl_init_http_server(unsigned lPort);
extern char* gtl_GetGalaxySemiTempFolder(); // Get the temp folder to be used by GTL

int gtl_set(const char* lKey, char* lValue)
{
    if (!lKey || !lValue)
        return GTL_CORE_ERR_INVALID_PARAM_POINTER;

    if (strcmp(GTL_KEY_AUX_FILE, lKey)==0)
    {
        gtcnm_CopyString(gtl_stGlobalInfo.mAuxFileName, lValue, GTC_MAX_PATH);
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_CONFIG_FILE, lKey)==0)
    {
        if(gtl_stGlobalInfo.nCurrentLibState != GTL_STATE_NOT_INITIALIZED)
            return GTL_CORE_ERR_LIB_ALREADY_INITIALIZED;
        gtcnm_CopyString(gtl_stGlobalInfo.mConfigFileName, lValue, GTC_MAX_PATH);
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp((char*)GTL_KEY_DESIRED_SITE_NB, lKey)==0)
    {
        int lSN=0;
        if(sscanf(lValue, "%d", &lSN) != 1)
        {
            return GTL_CORE_ERR_INVALID_PARAM_VALUE;
        }
        if (lSN<0 || lSN>256)
            return GTL_CORE_ERR_INVALID_PARAM_VALUE;

        gtl_stGlobalInfo.mDesiredSiteNb=lSN;
        return GTL_CORE_ERR_OKAY;
    }
    if (strcmp((char*)GTL_KEY_DESIRED_LIMITS, lKey)==0)
    {
        gtcnm_CopyString(gtl_stGlobalInfo.mDesiredLimits, lValue, GTC_MAX_STRING);
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp((char*)GTL_KEY_FACILITY_ID, lKey)==0)
    {
        gtcnm_CopyString(gtl_stProdInfo.mFacilityID, lValue, GTC_MAX_STRING);
        return GTL_CORE_ERR_OKAY;
    }
    if (strcmp(GTL_KEY_FAMILY_ID, lKey)==0)
    {
        gtcnm_CopyString(gtl_stProdInfo.mFamilyID, lValue, GTC_MAX_STRING);
        return GTL_CORE_ERR_OKAY;
    }
    if (strcmp(GTL_KEY_FLOW_ID, lKey)==0)
    {
        gtcnm_CopyString(gtl_stProdInfo.mFlowID, lValue, GTC_MAX_STRING);
        return GTL_CORE_ERR_OKAY;
    }
    if (strcmp(GTL_KEY_FLOOR_ID, lKey)==0)
    {
        gtcnm_CopyString(gtl_stProdInfo.mFloorID, lValue, GTC_MAX_STRING);
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_JOB_NAME, lKey)==0)
    {
        gtcnm_CopyString(gtl_stStation.szTestJobName, lValue, GTC_MAX_JOBNAME_LEN);
        return GTL_CORE_ERR_OKAY;
    }
    if (strcmp(GTL_KEY_JOB_REVISION, lKey)==0)
    {
        gtcnm_CopyString(gtl_stProdInfo.szJobRevision, lValue, GTC_MAX_JOBREV_LEN);
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_SPEC_NAME, lKey)==0)
    {
        gtcnm_CopyString(gtl_stProdInfo.mSpecName, lValue, GTC_MAX_STRING);
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_TESTER_NAME, lKey)==0)
    {
        if(gtl_stGlobalInfo.nCurrentLibState != GTL_STATE_NOT_INITIALIZED)
            return GTL_CORE_ERR_LIB_ALREADY_INITIALIZED;
        gtcnm_CopyString(gtl_stStation.szNodeName, lValue, GTC_MAX_NODENAME_LEN);
        return 0;
    }

    if (strcmp(GTL_KEY_TESTER_TYPE, lKey)==0)
    {
        if(gtl_stGlobalInfo.nCurrentLibState != GTL_STATE_NOT_INITIALIZED)
          return GTL_CORE_ERR_LIB_ALREADY_INITIALIZED;
        gtcnm_CopyString(gtl_stStation.szTesterType, lValue, GTC_MAX_TESTERTYPE_LEN);
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_TESTER_EXEC_TYPE, lKey)==0)
    {
        if(gtl_stGlobalInfo.nCurrentLibState != GTL_STATE_NOT_INITIALIZED)
          return GTL_CORE_ERR_LIB_ALREADY_INITIALIZED;
        gtcnm_CopyString(gtl_stStation.szTesterExecType, lValue, GTC_MAX_TESTEREXEC_LEN);
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_TESTER_EXEC_VERSION, lKey)==0)
    {
        if(gtl_stGlobalInfo.nCurrentLibState != GTL_STATE_NOT_INITIALIZED)
          return GTL_CORE_ERR_LIB_ALREADY_INITIALIZED;
        gtcnm_CopyString(gtl_stStation.szTesterExecVersion, lValue, GTC_MAX_TESTEREXEC_LEN);
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp((char*)GTL_KEY_QUERY_OUTPUT_FILE, lKey)==0)
    {
        gtcnm_CopyString(gtl_stGlobalInfo.mQueryOutputFileName, lValue, GTC_MAX_PATH);
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp((char*)GTL_KEY_QUERY, lKey)==0)
    {
        gtcnm_CopyString(gtl_stGlobalInfo.mQueryBuffer, lValue, GTC_MAX_QUERY_LEN);
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_RETEST_HBINS, lKey)==0)
    {
        gtcnm_CopyString(gtl_stProdInfo.mRetestHbins, lValue, GTC_MAX_STRING);
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_RECIPE_FILE, lKey)==0)
    {
        if(gtl_stGlobalInfo.nCurrentLibState != GTL_STATE_NOT_INITIALIZED)
            return GTL_CORE_ERR_LIB_ALREADY_INITIALIZED;
        gtcnm_CopyString(gtl_stGlobalInfo.mRecipeFileName, lValue, GTC_MAX_PATH);
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_STATION_NUMBER, lKey)==0)
    {
        unsigned int lSN=0;
        if(sscanf(lValue, "%d", &lSN) != 1)
        {
            return GTL_CORE_ERR_INVALID_PARAM_VALUE;
        }
        gtl_stStation.uiStationNb=lSN;
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_SPEC_VERSION, lKey)==0)
    {
        gtcnm_CopyString(gtl_stProdInfo.mSpecVersion, lValue, GTC_MAX_STRING);
        return GTL_CORE_ERR_OKAY; // ticket number 1167
    }

    if (strcmp(GTL_KEY_OUTPUT_FOLDER, lKey)==0)
    {
        /* return error code if GTL already initialized */
        if(gtl_stGlobalInfo.nCurrentLibState != GTL_STATE_NOT_INITIALIZED)
            return GTL_CORE_ERR_LIB_ALREADY_INITIALIZED;
        gtcnm_CopyString(gtl_stGlobalInfo.mOutputFolder, lValue, GTC_MAX_PATH);
        // test the folder
        GTL_LOG(7, (char *)"Trying to stat on '%s'", gtl_stGlobalInfo.mOutputFolder);
        struct stat fileStat;
        if(stat(gtl_stGlobalInfo.mOutputFolder, &fileStat) < 0)
        {
            GTL_LOG(3, (char *)"stat on '%s' failed", gtl_stGlobalInfo.mOutputFolder);
            return GTL_CORE_ERR_INVALID_OUTPUT_FOLDER;
        }
        GTL_LOG(7, (char *)"Trying to check if '%s' is a directory", gtl_stGlobalInfo.mOutputFolder);
        if (!S_ISDIR(fileStat.st_mode))
        {
            GTL_LOG(3, (char *)"Output folder '%s' does not seem to be a directory", gtl_stGlobalInfo.mOutputFolder);
            return GTL_CORE_ERR_INVALID_OUTPUT_FOLDER;
        }
        GTL_LOG(7, (char *)"Trying to check if '%s' is a writable", gtl_stGlobalInfo.mOutputFolder);
        if (!(fileStat.st_mode & S_IWUSR))
        {
            GTL_LOG(3, (char *)"This folder '%s' does not seem to be writable by this user", gtl_stGlobalInfo.mOutputFolder);
            return GTL_CORE_ERR_INVALID_OUTPUT_FOLDER;
        }
        GTL_LOG(7, (char *)"Trying to get current PID", 0);
        char lPID[100]=""; ut_GetPID(lPID);
        char lTestFile[100];
        sprintf(lTestFile, "%s/gtl_test_%s.txt", gtl_stGlobalInfo.mOutputFolder, lPID);
        GTL_LOG(7, (char *)"Trying to open test file '%s' in write mode", lTestFile);
        FILE *f=fopen(lTestFile, "w+");
        if (!f)
        {
            GTL_LOG(3, "Cannot write into the output folder '%s'", gtl_stGlobalInfo.mOutputFolder);
            return GTL_CORE_ERR_INVALID_OUTPUT_FOLDER;
        }
        //fwrite(f, "Test...");
        GTL_LOG(7, (char *)"Closing test file", 0);
        fclose(f);
        GTL_LOG(7, (char *)"Removing test file", 0);
        if (remove(lTestFile)!=0)
        {
            GTL_LOG(4, "Failed to delete test file %s : errno=%d", lTestFile, errno);
        }
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_TEMP_FOLDER, lKey)==0)
    {
        strncpy(gtl_stGlobalInfo.mTempFolder, lValue, GTC_MAX_PATH);
        return 0;
    }

    if (strcmp(GTL_KEY_TEST_CODE, lKey)==0)
    {
        gtcnm_CopyString(gtl_stProdInfo.mTestCode, lValue, GTC_MAX_STRING);
        return 0;
    }

    if (strcmp(GTL_KEY_OUTPUT_FILENAME, lKey)==0)
    {
        // 7501: let s allow the setting at anytime
        //if(gtl_stGlobalInfo.nCurrentLibState != GTL_STATE_NOT_INITIALIZED)
          //  return GTL_CORE_ERR_LIB_ALREADY_INITIALIZED;
        gtcnm_CopyString(gtl_stGlobalInfo.mOutputFileName, lValue, GTC_MAX_PATH);
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_DATA_FILENAME, lKey)==0)
    {
        // GCORE-1502
        //if(gtl_stGlobalInfo.nCurrentLibState != GTL_STATE_NOT_INITIALIZED)
          //  return GTL_CORE_ERR_LIB_ALREADY_INITIALIZED;
        gtcnm_CopyString(gtl_stGlobalInfo.mDataFileName, lValue, GTC_MAX_PATH);
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_LOT_ID, lKey)==0)
    {
        if(gtl_stGlobalInfo.nCurrentLibState != GTL_STATE_NOT_INITIALIZED)
            return GTL_CORE_ERR_LIB_ALREADY_INITIALIZED;
        gtcnm_CopyString(gtl_stProdInfo.szLotID, lValue, GTC_MAX_LOTID_LEN);
        return GTL_CORE_ERR_OKAY;
    }
    if (strcmp(GTL_KEY_MODE_CODE, lKey)==0)
    {
        gtl_stProdInfo.mModeCode=lValue[0];
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_OPERATOR_NAME, lKey)==0)
    {
        gtcnm_CopyString(gtl_stProdInfo.szOperatorName, lValue, GTC_MAX_OPERATORNAME_LEN);
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_PRODUCT_ID, lKey)==0)
    {
        if(gtl_stGlobalInfo.nCurrentLibState != GTL_STATE_NOT_INITIALIZED)
            return GTL_CORE_ERR_LIB_ALREADY_INITIALIZED;
        gtcnm_CopyString(gtl_stProdInfo.szProductID, lValue, GTC_MAX_PRODUCTID_LEN);
        return GTL_CORE_ERR_OKAY;
    }
    if (strcmp(GTL_KEY_RETEST_CODE, lKey)==0)
    {
        gtl_stProdInfo.mRetestCode=lValue[0];
        return GTL_CORE_ERR_OKAY;
    }
    if (strcmp(GTL_KEY_ROM_CODE, lKey)==0)
    {
        if (gtl_stProdInfo.mRomCode)
            gtcnm_CopyString(gtl_stProdInfo.mRomCode, lValue, GTC_MAX_STRING);
        return GTL_CORE_ERR_OKAY;
    }
    if (strcmp(GTL_KEY_GTM_COMMUNICATION_MODE, lKey)==0)
    {
        if(gtl_stGlobalInfo.nCurrentLibState != GTL_STATE_NOT_INITIALIZED)
            return GTL_CORE_ERR_LIB_ALREADY_INITIALIZED;

        if(strcmp(lValue,"synchronous")==0)
        {
            gtl_stGlobalInfo.mGtmCommunicationMode = GTL_GTMCOMM_SYNCHRONOUS;
            return GTL_CORE_ERR_OKAY;
        }
        if(strcmp(lValue,"asynchronous")==0)
        {
            gtl_stGlobalInfo.mGtmCommunicationMode = GTL_GTMCOMM_ASYNCHRONOUS;
            return GTL_CORE_ERR_OKAY;
        }
        return GTL_CORE_ERR_INVALID_PARAM_VALUE;
    }

    if ((strcmp(GTL_KEY_SUBLOT_ID, lKey) == 0) ||
        (strcmp(GTL_KEY_SUBLOT_NUMBER, lKey) == 0))
    {
        if(gtl_stGlobalInfo.nCurrentLibState != GTL_STATE_NOT_INITIALIZED)
            return GTL_CORE_ERR_LIB_ALREADY_INITIALIZED;
        gtcnm_CopyString(gtl_stProdInfo.szSublotID, lValue, GTC_MAX_SLOTID_LEN);
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_SETUP_ID, lKey)==0)
    {
        gtcnm_CopyString(gtl_stProdInfo.mSetupID, lValue, GTC_MAX_STRING);
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_SOCK_CONNECT_MODE, lKey)==0)
    {
        if((gtl_stGlobalInfo.nCurrentLibState != GTL_STATE_NOT_INITIALIZED) &&
                (gtl_stGlobalInfo.nCurrentLibState != GTL_STATE_OFFLINE))
            return GTL_CORE_ERR_LIB_ALREADY_INITIALIZED;

        if(strcmp(lValue,"blocking")==0)
        {
            gtl_stGlobalInfo.mSocketConnetionMode = GTL_SOCK_BLOCKING;
            return GTL_CORE_ERR_OKAY;
        }
        if(strcmp(lValue,"non-blocking")==0)
        {
            gtl_stGlobalInfo.mSocketConnetionMode = GTL_SOCK_NON_BLOCKING;
            return GTL_CORE_ERR_OKAY;
        }
        return GTL_CORE_ERR_INVALID_PARAM_VALUE;
    }

    if (strcmp(GTL_KEY_SOCK_COMM_MODE, lKey)==0)
    {
        if((gtl_stGlobalInfo.nCurrentLibState != GTL_STATE_NOT_INITIALIZED) &&
                (gtl_stGlobalInfo.nCurrentLibState != GTL_STATE_OFFLINE))
            return GTL_CORE_ERR_LIB_ALREADY_INITIALIZED;

        if(strcmp(lValue,"blocking")==0)
        {
            gtl_stGlobalInfo.mSocketCommunicationMode = GTL_SOCK_BLOCKING;
            return GTL_CORE_ERR_OKAY;
        }
        if(strcmp(lValue,"non-blocking")==0)
        {
            gtl_stGlobalInfo.mSocketCommunicationMode = GTL_SOCK_NON_BLOCKING;
            return GTL_CORE_ERR_OKAY;
        }
        return GTL_CORE_ERR_INVALID_PARAM_VALUE;
    }

    if (strcmp(GTL_KEY_SOCK_CONNECT_TIMEOUT, lKey)==0)
    {
        if((gtl_stGlobalInfo.nCurrentLibState != GTL_STATE_NOT_INITIALIZED) &&
                (gtl_stGlobalInfo.nCurrentLibState != GTL_STATE_OFFLINE))
            return GTL_CORE_ERR_LIB_ALREADY_INITIALIZED;

        unsigned int lTimeout=0;
        if(sscanf(lValue, "%u", &lTimeout) == 1)
        {
            gtl_stGlobalInfo.mSocketConnectionTimeout = lTimeout;
            return GTL_CORE_ERR_OKAY;
        }
        return GTL_CORE_ERR_INVALID_PARAM_VALUE;
    }

    if (strcmp(GTL_KEY_HTTP_SERVER, lKey)==0)
    {
        int lR=GTL_CORE_ERR_OKAY;
        if(strcmp(lValue,"on")==0)
        {
            lR=gtl_init_http_server(8080); // 808080 is illegal !
            GTL_LOG(5, "gtl_init_http_server returned %d", lR);
        }
        return lR;
    }

    if (strcmp(GTL_KEY_SOCK_RECEIVE_TIMEOUT, lKey)==0)
    {
        if((gtl_stGlobalInfo.nCurrentLibState != GTL_STATE_NOT_INITIALIZED) &&
                (gtl_stGlobalInfo.nCurrentLibState != GTL_STATE_OFFLINE))
            return GTL_CORE_ERR_LIB_ALREADY_INITIALIZED;

        unsigned int lTimeout=0;
        if(sscanf(lValue, "%u", &lTimeout) == 1)
        {
            gtl_stGlobalInfo.mSocketReceiveTimeout = lTimeout;
            return GTL_CORE_ERR_OKAY;
        }
        return GTL_CORE_ERR_INVALID_PARAM_VALUE;
    }

    if (strcmp(GTL_KEY_SOCK_SEND_TIMEOUT, lKey)==0)
    {
        if((gtl_stGlobalInfo.nCurrentLibState != GTL_STATE_NOT_INITIALIZED) &&
                (gtl_stGlobalInfo.nCurrentLibState != GTL_STATE_OFFLINE))
            return GTL_CORE_ERR_LIB_ALREADY_INITIALIZED;

        unsigned int lTimeout=0;
        if(sscanf(lValue, "%u", &lTimeout) == 1)
        {
            gtl_stGlobalInfo.mSocketSendTimeout = lTimeout;
            return GTL_CORE_ERR_OKAY;
        }
        return GTL_CORE_ERR_INVALID_PARAM_VALUE;
    }

    if (strcmp(GTL_KEY_MAX_MESSAGES_STACK_SIZE, lKey)==0)
    {
        int lSize=0;
        int lR=sscanf(lValue, "%d", &lSize);
        if (lR!=EOF && lR>0 && lSize>1)
        {
            int max_num_of_messages=lSize/((int)sizeof(GTL_Message));
            gtl_stGlobalInfo.mMessages.maxsize=max_num_of_messages;
            return GTL_CORE_ERR_OKAY;
        }
        return GTL_CORE_ERR_INVALID_PARAM_VALUE;
    }

    if (strcmp(GTL_KEY_SOCKET_TRACE, lKey)==0)
    {
        if(gtl_stGlobalInfo.nCurrentLibState != GTL_STATE_NOT_INITIALIZED)
            return GTL_CORE_ERR_LIB_ALREADY_INITIALIZED;

        if(strcmp(lValue,"on")==0)
        {
            gtl_stGlobalInfo.mSocketTrace = 1;
            return GTL_CORE_ERR_OKAY;
        }
        if(strcmp(lValue,"off")==0)
        {
            gtl_stGlobalInfo.mSocketTrace = 0;
            return GTL_CORE_ERR_OKAY;
        }
        return GTL_CORE_ERR_INVALID_PARAM_VALUE;
    }

    if (strcmp(GTL_KEY_TEST_TEMPERATURE, lKey)==0)
    {
        gtcnm_CopyString(gtl_stProdInfo.mTemperature, lValue, GTC_MAX_STRING);
        return 0;
    }

    if (strcmp(GTL_KEY_SITES_NUMBERS, lKey)==0)
    {
        int lSN[256]; memset(&lSN, -1, 256*sizeof(int) );
        char* lValuePointer=lValue;
        int lR=0, i=0;
        do
        {
            lR=sscanf(lValuePointer, "%d", &lSN[i]);
            if (lR==1)
            {
                i++;
                // let's jump any leading space
                while (*lValuePointer==' ')
                    lValuePointer++;
                // let's jump to next space is any
                while ( (*lValuePointer!='\0') && (*lValuePointer!=' ') )
                    lValuePointer++;
            }
            else break;
        }
        while ( (lR==1) && (lR!=EOF) && (*lValuePointer!='\n') && (i<255) );

        GTL_LOG(5, "%d sites number interpreted: %d %d %d %d %d %d %d %d %d...", i,
              lSN[0],lSN[1],lSN[2],lSN[3],lSN[4],lSN[5],lSN[6],lSN[7],lSN[8]);

        memcpy(gtl_stStation.pnSiteNumbers, lSN, 255);

        if (i>0)
            return GTL_CORE_ERR_OKAY;

        return GTL_CORE_ERR_INVALID_SITE_NUMBER;
    }

    if (strcmp(GTL_KEY_MAX_NUMBER_OF_ACTIVE_SITES, lKey)==0)
    {
        int lNS=-1;
        int lR=sscanf(lValue, "%d", &lNS);
        if (lR!=1 || lNS<1 || lNS>255)
            return GTL_CORE_ERR_INVALID_NUM_OF_SITES;
        gtl_stStation.uiNbSites=lNS;
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_RECONNECTION_MODE, lKey)==0)
    {
        if(strcmp(lValue,"on")==0)
        {
            gtl_stGlobalInfo.mReconnectionMode=GTL_RECONNECT_ON;
            return GTL_CORE_ERR_OKAY;
        }
        if(strcmp(lValue,"off")==0)
        {
            gtl_stGlobalInfo.mReconnectionMode=GTL_RECONNECT_OFF;
            return GTL_CORE_ERR_OKAY;
        }
        return GTL_CORE_ERR_INVALID_PARAM_VALUE;
    }

    if (strcmp(GTL_KEY_USER_TXT, lKey)==0)
    {
        gtcnm_CopyString(gtl_stProdInfo.mUserText, lValue, GTC_MAX_STRING);
        return 0;
    }

    if (strcmp(GTL_KEY_USER_NAME, lKey)==0)
    {
        gtcnm_CopyString(gtl_stStation.szUserName, lValue, GTC_MAX_USERNAME_LEN);
        return 0;
    }

    if (strcmp(GTL_KEY_FIELDS_TO_MATCH, lKey) == 0)
    {
        gtcnm_CopyString(gtl_stGlobalInfo.mFieldsToMatch,
                         lValue, GTC_MAX_STRING);
        return 0;
    }

    if (strcmp(GTL_KEY_RELOAD_LIMITS, lKey) == 0)
    {
        if(strcmp(lValue,"on")==0)
        {
            gtl_stGlobalInfo.mReloadLimits=GTL_RELOAD_LIMITS_ON;
            return GTL_CORE_ERR_OKAY;
        }
        if(strcmp(lValue,"off")==0)
        {
            gtl_stGlobalInfo.mReloadLimits=GTL_RELOAD_LIMITS_OFF;
            return GTL_CORE_ERR_OKAY;
        }
        return GTL_CORE_ERR_INVALID_PARAM_VALUE;
    }

    if (strcmp(GTL_KEY_LIMITS_GET_CONDITION, lKey) == 0)
    {
        gtcnm_CopyString(gtl_stGlobalInfo.mLimitsGetCondition,
                         lValue, GTC_MAX_STRING);
        return 0;
    }

    GTL_LOG(4, "gtl_set(): unknown key %s", lKey);

    return GTL_CORE_ERR_INVALID_PARAM_VALUE;
}

int gtl_get(const char* lKey, char* lValue)
{
    if (!lKey || !lValue)
        return GTL_CORE_ERR_INVALID_PARAM_POINTER;

    if (strcmp(GTL_KEY_AUX_FILE, lKey)==0)
    {
        if (gtl_stGlobalInfo.mAuxFileName)
            sprintf(lValue, "%s", gtl_stGlobalInfo.mAuxFileName);
        return GTL_CORE_ERR_OKAY;
    }
    if (strcmp(GTL_KEY_CONFIG_FILE, lKey)==0)
    {
        if (gtl_stGlobalInfo.mConfigFileName)
            sprintf(lValue, "%s", gtl_stGlobalInfo.mConfigFileName);
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp((char*)GTL_KEY_FACILITY_ID, lKey)==0)
    {
        if (gtl_stProdInfo.mFacilityID!=0)
            sprintf(lValue, "%s", gtl_stProdInfo.mFacilityID);
        return GTL_CORE_ERR_OKAY;
    }
    if (strcmp(GTL_KEY_FAMILY_ID, lKey)==0)
    {
        if (gtl_stProdInfo.mFamilyID!=0)
            sprintf(lValue, "%s", gtl_stProdInfo.mFamilyID);
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_FLOW_ID, lKey)==0)
    {
        if (gtl_stProdInfo.mFlowID!=0)
            sprintf(lValue, "%s", gtl_stProdInfo.mFlowID);
        return GTL_CORE_ERR_OKAY;
    }
    if (strcmp(GTL_KEY_FLOOR_ID, lKey)==0)
    {
        if (gtl_stProdInfo.mFloorID!=0)
            sprintf(lValue, "%s", gtl_stProdInfo.mFloorID);
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_JOB_NAME, lKey)==0)
    {
        if (gtl_stStation.szTestJobName)
            sprintf(lValue, "%s", gtl_stStation.szTestJobName);
        return GTL_CORE_ERR_OKAY;
    }
    if (strcmp(GTL_KEY_JOB_REVISION, lKey)==0)
    {
        if (gtl_stProdInfo.szJobRevision)
            sprintf(lValue, "%s", gtl_stProdInfo.szJobRevision);
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_CURRENT_MESSAGE, lKey)==0)
    {
        sprintf(lValue, "%s", gtl_stGlobalInfo.mCurrentMessage);
        return GTL_CORE_ERR_OKAY;
    }
    if (strcmp(GTL_KEY_CURRENT_MESSAGE_SEV, lKey)==0)
    {
        sprintf(lValue, "%d", gtl_stGlobalInfo.mCurrentMessageSeverity);
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_CURRENT_MESSAGE_ID, lKey)==0)
    {
        sprintf(lValue, "%d", gtl_stGlobalInfo.mCurrentMessageID);
        return GTL_CORE_ERR_OKAY;
    }
    if (strcmp(GTL_KEY_LIB_STATE, lKey)==0 )
    {
        sprintf(lValue, "%d", gtl_stGlobalInfo.nCurrentLibState );
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_LIB_VERSION, lKey)==0 )
    {
        sprintf(lValue, "%d.%d", GTL_VERSION_MAJOR, GTL_VERSION_MINOR);
        return GTL_CORE_ERR_OKAY;
    }
    if (strcmp(GTL_KEY_LIB_VERSION_MAJOR, lKey)==0 )
    {
        sprintf(lValue, "%d", GTL_VERSION_MAJOR);
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_LIB_VERSION_MINOR, lKey)==0 )
    {
        sprintf(lValue, "%d",GTL_VERSION_MINOR);
        return GTL_CORE_ERR_OKAY;
    }
    if (strcmp(GTL_KEY_DATA_FILENAME, lKey) == 0)
    {
        if (gtl_stGlobalInfo.mDataFileName)
        {
            sprintf(lValue, "%s", gtl_stGlobalInfo.mDataFileName);
        }
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_LOT_ID, lKey) == 0)
    {
        if (gtl_stProdInfo.szLotID)
        {
            sprintf(lValue, "%s", gtl_stProdInfo.szLotID);
        }
        return GTL_CORE_ERR_OKAY;
    }
    if (strcmp(GTL_KEY_MODE_CODE, lKey)==0)
    {
        sprintf(lValue, "%c", gtl_stProdInfo.mModeCode);
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_OPERATOR_NAME, lKey)==0)
    {
        if (gtl_stProdInfo.szOperatorName)
            sprintf(lValue, "%s", gtl_stProdInfo.szOperatorName);
        return GTL_CORE_ERR_OKAY;
    }
    if (strcmp(GTL_KEY_PRODUCT_ID, lKey) == 0)
    {
        if (gtl_stProdInfo.szProductID)
        {
            sprintf(lValue, "%s", gtl_stProdInfo.szProductID);
        }
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_RETEST_HBINS, lKey) == 0)
    {
        if (gtl_stProdInfo.mRetestHbins)
        {
            sprintf(lValue, "%s", gtl_stProdInfo.mRetestHbins);
        }
        return GTL_CORE_ERR_OKAY;
    }
    if (strcmp(GTL_KEY_RECIPE_FILE, lKey)==0)
    {
        if (gtl_stGlobalInfo.mRecipeFileName)
            sprintf(lValue, "%s", gtl_stGlobalInfo.mRecipeFileName);
        return GTL_CORE_ERR_OKAY;
    }

    if ( strcmp(GTL_KEY_NUM_OF_MESSAGES_IN_STACK, lKey)==0 )
    {
        sprintf(lValue, "%d", gtl_get_number_messages_in_stack());
        return GTL_CORE_ERR_OKAY;
    }
    if (strcmp(GTL_KEY_RETEST_CODE, lKey)==0)
    {
        sprintf(lValue, "%c", gtl_stProdInfo.mRetestCode);
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_ROM_CODE, lKey)==0)
    {
        if (gtl_stProdInfo.mRomCode)
            sprintf(lValue, "%s", gtl_stProdInfo.mRomCode);
        return GTL_CORE_ERR_OKAY;
    }
    if (strcmp(GTL_KEY_SUBLOT_ID, lKey) == 0)
    {
        if (gtl_stProdInfo.szSublotID)
        {
            sprintf(lValue, "%s", gtl_stProdInfo.szSublotID);
        }
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_SETUP_ID, lKey)==0)
    {
        if (gtl_stProdInfo.mSetupID)
            sprintf(lValue, "%s", gtl_stProdInfo.mSetupID);
        return GTL_CORE_ERR_OKAY;
    }
    if (strcmp(GTL_KEY_SPEC_NAME, lKey)==0)
    {
        if (gtl_stProdInfo.mSpecName)
            sprintf(lValue, "%s", gtl_stProdInfo.mSpecName);
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_SPEC_VERSION, lKey)==0)
    {
        if (gtl_stProdInfo.mSpecVersion)
            sprintf(lValue, "%s", gtl_stProdInfo.mSpecVersion);
        return GTL_CORE_ERR_OKAY;
    }
    if (strcmp(GTL_KEY_SPLITLOT_ID, lKey)==0 )
    {
        sprintf(lValue, "%d", gtl_stProdInfo.mSplitlotID);
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_STATION_NUMBER, lKey)==0)
    {
        sprintf(lValue, "%d", gtl_stStation.uiStationNb);
        return GTL_CORE_ERR_OKAY;
    }
    if (strcmp(GTL_KEY_TEST_CODE, lKey)==0)
    {
        if (gtl_stProdInfo.mTestCode)
          sprintf(lValue, "%s", gtl_stProdInfo.mTestCode);
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_TESTER_NAME, lKey)==0)
    {
        if (gtl_stStation.szNodeName)
          sprintf(lValue, "%s", gtl_stStation.szNodeName);
        return GTL_CORE_ERR_OKAY;
    }
    if (strcmp(GTL_KEY_TESTER_TYPE, lKey)==0)
    {
        if (gtl_stStation.szTesterType)
          sprintf(lValue, "%s", gtl_stStation.szTesterType);
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_TESTER_EXEC_TYPE, lKey)==0)
    {
        if (gtl_stStation.szTesterExecType)
          sprintf(lValue, "%s", gtl_stStation.szTesterExecType);
        return GTL_CORE_ERR_OKAY;
    }
    if (strcmp(GTL_KEY_TESTER_EXEC_VERSION, lKey)==0)
    {
        if (gtl_stStation.szTesterExecVersion)
          sprintf(lValue, "%s", gtl_stStation.szTesterExecVersion);
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_TEST_TEMPERATURE, lKey)==0)
    {
        sprintf(lValue, "%s", gtl_stProdInfo.mTemperature);
        return GTL_CORE_ERR_OKAY;
    }
    if ( strcmp(GTL_KEY_OUTPUT_FOLDER, lKey)==0 )
    {
        if (gtl_stGlobalInfo.mOutputFolder)
            sprintf(lValue, "%s", gtl_stGlobalInfo.mOutputFolder);
        return GTL_CORE_ERR_OKAY;
    }

    if ( strcmp(GTL_KEY_TEMP_FOLDER, lKey)==0 )
    {
        char* lTF=gtl_GetGalaxySemiTempFolder();
        if (!lTF)
            return GTL_CORE_ERR_NO_TEMP_FOLDER;
        strncpy(lValue, lTF, GTC_MAX_PATH);
        return GTL_CORE_ERR_OKAY;
    }
    if ( strcmp(GTL_KEY_OUTPUT_FILENAME, lKey)==0 )
    {
        gtcnm_CopyString(lValue, gtl_stGlobalInfo.mOutputFileName, GTC_MAX_PATH);
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_GTM_COMMUNICATION_MODE, lKey)==0)
    {
        sprintf(lValue, "%s", gtl_stGlobalInfo.mGtmCommunicationMode==GTL_GTMCOMM_SYNCHRONOUS?"synchronous":"asynchronous");
        return GTL_CORE_ERR_OKAY;
    }
    if (strcmp(GTL_KEY_SOCK_CONNECT_MODE, lKey)==0)
    {
        sprintf(lValue, "%s", gtl_stGlobalInfo.mSocketConnetionMode==GTL_SOCK_BLOCKING?"blocking":"non-blocking");
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_SOCK_COMM_MODE, lKey)==0)
    {
        sprintf(lValue, "%s", gtl_stGlobalInfo.mSocketCommunicationMode==GTL_SOCK_BLOCKING?"blocking":"non-blocking");
        return GTL_CORE_ERR_OKAY;
    }
    if (strcmp(GTL_KEY_SOCK_CONNECT_TIMEOUT, lKey)==0)
    {
        sprintf(lValue, "%u", gtl_stGlobalInfo.mSocketConnectionTimeout);
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_SOCK_RECEIVE_TIMEOUT, lKey)==0)
    {
        sprintf(lValue, "%u", gtl_stGlobalInfo.mSocketReceiveTimeout);
        return GTL_CORE_ERR_OKAY;
    }
    if (strcmp(GTL_KEY_SOCK_SEND_TIMEOUT, lKey)==0)
    {
        sprintf(lValue, "%u", gtl_stGlobalInfo.mSocketSendTimeout);
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_SOCKET_TRACE, lKey)==0)
    {
        if(gtl_stGlobalInfo.mSocketTrace == 1)
            strcpy(lValue, "on");
        else
            strcpy(lValue, "off");
        return GTL_CORE_ERR_OKAY;
    }
    if (strcmp(GTL_KEY_RECONNECTION_MODE, lKey)==0)
    {
        sprintf(lValue, "%s", gtl_stGlobalInfo.mReconnectionMode==GTL_RECONNECT_ON?"on":"off");
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_USER_TXT, lKey)==0)
    {
        sprintf(lValue, "%s", gtl_stProdInfo.mUserText);
        return GTL_CORE_ERR_OKAY;
    }
    if (strcmp(GTL_KEY_DESIRED_SITE_NB, lKey)==0)
    {
        sprintf(lValue, "%d", gtl_stGlobalInfo.mDesiredSiteNb);
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_DESIRED_LIMITS, lKey)==0)
    {
        if (gtl_stGlobalInfo.mDesiredLimits)
            sprintf(lValue, "%s", gtl_stGlobalInfo.mDesiredLimits);
        return GTL_CORE_ERR_OKAY;
    }
    if (strcmp(GTL_KEY_SITE_STATE, lKey)==0)
    {
        // No need to check validity of desires site nb., gtl_get_site_state() takes care of it.
        sprintf(lValue, "%d", gtl_get_site_state(gtl_stGlobalInfo.mDesiredSiteNb));
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_FIELDS_TO_MATCH, lKey) == 0)
    {
        sprintf(lValue, "%s", gtl_stGlobalInfo.mFieldsToMatch);
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_RELOAD_LIMITS, lKey)==0)
    {
        sprintf(lValue, "%s", gtl_stGlobalInfo.mReloadLimits==GTL_RELOAD_LIMITS_ON?"on":"off");
        return GTL_CORE_ERR_OKAY;
    }

    if (strcmp(GTL_KEY_LIMITS_GET_CONDITION, lKey) == 0)
    {
        sprintf(lValue, "%s", gtl_stGlobalInfo.mLimitsGetCondition);
        return GTL_CORE_ERR_OKAY;
    }

    GTL_LOG(4, "gtl_get(): unknown key %s", lKey);

    return GTL_CORE_ERR_INVALID_PARAM_VALUE;
}
