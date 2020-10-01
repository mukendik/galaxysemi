#ifdef __unix__
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#endif


#define GS_ALLOW_CMD 0
// DONT ALLOW READ OPERATION IF YMADMINDB_WRITE is ONGOING
#define GS_YMADMINDB_WRITE_SETTING_CMD  1
// Define the YMADMINDB config file
#define GS_DB_SETTING_CONFIG_FILE_NAME  ".gexlm_config.xml"
// Directory used under linux system
#define GS_GEX_LOCAL_UNIXFOLDER         ".examinator"

// String used to check if the daemon is alive
#define GS_PING_CMD                     "PING:"
//YMADMINDB WRITE SETTING
#define GS_WRITE_YM_ADMINDB_CMD         "YMWR:"
//YMADMINDB READ SETTING
#define GS_READ_YM_ADMINDB_CMD          "YMRD:"
//Get Server Time
#define GS_SERVER_TIME_CMD              "TIME:"
//Get Server Timeout
#define GS_CLIENT_TIMEOUT_CMD           "TIMEOUT:"

//String used to start sending YMADMINDB content
#define GS_START_CMD                    "START:"
//String used to send next YMADMINDB content part
#define GS_NEXT_CMD                     "NEXT:"
//String used to continue sending YMADMINDB content part
#define GS_CONTINUE_CMD                 "CONTINUE:"
// String used to stop sending YMADMINDB content part
#define GS_STOP_CMD                     "STOP:"
// String used to stop end the YMADMINDB sending
#define GS_END_CMD                      "END:"

// Max length to send a buffer to the client application
#define GS_LC_VSEND_COMMAND_LENGTH      10
#define GS_LC_VSEND_TEXT_LENGTH         100
#define GS_LC_VSEND_MESSAGE_LENGTH      (GS_LC_VSEND_COMMAND_LENGTH + GS_LC_VSEND_TEXT_LENGTH)

// log file name for vendor daemon
#define GS_VENDOR_DAEMON_LOG            "daemon.txt"

static int  sGS_CurrentCMD = GS_ALLOW_CMD;//  GS_YMADMINDB_WRITE_SETTING_CMD;
static char sGS_vd_msg[GS_LC_VSEND_MESSAGE_LENGTH+1];
static char sGS_DatabaseSettingFile [MAX_PATH] = {'\0'};

//Function to retrieve the GS_LIC_CLIENT_TIMEOUT env var and return stored timeout values.
int GetClientTimeout()
{
    char *lClientTimeout= NULL;
    lClientTimeout = getenv("GS_LIC_CLIENT_TIMEOUT");
    if(lClientTimeout)
    {
        int lTimeoutValue = atoi(lClientTimeout);
        return lTimeoutValue;
    }
    else
    {
        return -1;
    }
}

int isGalaxylmDebugEnabled()
{
    char *  lGalaxylmDebug = NULL;
    int lLogDebug = 0;

    lGalaxylmDebug = getenv("GALAXYLM_DEBUG");

    if(lGalaxylmDebug == NULL)
    {
        lLogDebug = 0;
    }
    else
    {
        int lGalaxylmDebugVal = atoi(lGalaxylmDebug);
        if(lGalaxylmDebugVal == 0)
        {
            lLogDebug = 0;
        }
        else
        {
            lLogDebug = 1;
        }
    }

    return lLogDebug;
}

int GSLM_appendFile (const char *file_name, const char *data)
{
    FILE *lHandle = NULL;
    lHandle = fopen(file_name, "a");

    if(!lHandle)
        return 0;

    fprintf(lHandle,"%s", data);

    fclose(lHandle);
    return 1;
}

int GSLOG_appendFile (const char *file_name, const char *data)
{
    FILE *lHandle = NULL;
    if(isGalaxylmDebugEnabled() == 0)
    {
        return 1;
    }
    lHandle = fopen(file_name, "a");

    if(!lHandle)
        return 0;

    fprintf(lHandle,"%s", data);
    fclose(lHandle);
    return 1;
}

void GS_initDBSettingFile ()
{
    char *  lEnv = NULL;
    char    lLocalUnixFolder[MAX_PATH] = "";
    char    lLogString[MAX_PATH+1024] = "";

#ifdef __unix__
    DIR*    dir = NULL;
    int     dirCreationError = 0;
#endif

    if(sGS_DatabaseSettingFile[0] == '\0')
    {

        lEnv = getenv("GEX_SERVER_PROFILE");
        if(lEnv != NULL)
        {
#ifdef __unix__
            sprintf(lLocalUnixFolder,"%s%s%s",lEnv,DIRECTORY_SEPARATOR,GS_GEX_LOCAL_UNIXFOLDER);

            dir = opendir(lLocalUnixFolder);

            if(!dir)
            {
               /*try to create it if not created go ahead*/
               dirCreationError = mkdir(lLocalUnixFolder,0777);

            }

            sprintf(sGS_DatabaseSettingFile,"%s%s%s%s%s",lEnv,DIRECTORY_SEPARATOR,GS_GEX_LOCAL_UNIXFOLDER,
                    DIRECTORY_SEPARATOR,GS_DB_SETTING_CONFIG_FILE_NAME);
#else
            sprintf(sGS_DatabaseSettingFile,"%s%s%s",lEnv,DIRECTORY_SEPARATOR,GS_DB_SETTING_CONFIG_FILE_NAME);
#endif

            sprintf(lLogString, "Configuration file location: %s\n", sGS_DatabaseSettingFile);
            GSLOG_appendFile(GS_VENDOR_DAEMON_LOG, lLogString);
        }
        else
        {
            GSLOG_appendFile(GS_VENDOR_DAEMON_LOG, "GEX_SERVER_PROFILE environment variable not defined\n");
        }
    }
}

static char *GS_respond_to_lc_vsend(char *incoming)
{
    FILE *  lHandle = 0;
    long    lFileSize = 0;
    char    lText[GS_LC_VSEND_TEXT_LENGTH+1] = "";
    char    lCommand[GS_LC_VSEND_COMMAND_LENGTH+1] = "";
    char    lDumpData[1000];
    time_t  lServerTime;
    struct tm * lServerTimeTm = NULL;

    // Init globale string
    memset(sGS_vd_msg, 0, sizeof(sGS_vd_msg));

    sprintf(lDumpData, "incoming: %s\n", incoming);
    GSLOG_appendFile(GS_VENDOR_DAEMON_LOG, lDumpData);

    if(strncmp(incoming, GS_PING_CMD, strlen(GS_PING_CMD)) == 0)
    {
        strncpy(lCommand, GS_PING_CMD, GS_LC_VSEND_COMMAND_LENGTH);
        lCommand[GS_LC_VSEND_COMMAND_LENGTH] = '\0';

        strcpy(lText, "Galaxy Daemon is alive");
    }
    else if(strncmp(incoming, GS_SERVER_TIME_CMD, strlen(GS_SERVER_TIME_CMD)) == 0)
    {
        time ( &lServerTime );
        lServerTimeTm = localtime(&lServerTime);

        strncpy(lCommand, GS_SERVER_TIME_CMD, GS_LC_VSEND_COMMAND_LENGTH);
        lCommand[GS_LC_VSEND_COMMAND_LENGTH] = '\0';

        sprintf(lText, "%d:%d:%d:%d:%d:%d",
                lServerTimeTm->tm_year,lServerTimeTm->tm_mon,
                lServerTimeTm->tm_mday,lServerTimeTm->tm_hour,
                lServerTimeTm->tm_min,lServerTimeTm->tm_isdst);

    }
    else if(strncmp(incoming, GS_CLIENT_TIMEOUT_CMD, strlen(GS_CLIENT_TIMEOUT_CMD)) == 0)
    {
        int lClientTimeout = GetClientTimeout();

        strncpy(lCommand, GS_CLIENT_TIMEOUT_CMD, GS_LC_VSEND_COMMAND_LENGTH);
        lCommand[GS_LC_VSEND_COMMAND_LENGTH] = '\0';

        sprintf(lText, "%d", lClientTimeout);
    }
    //incoming = "YMWR:......."
    else if(strncmp(incoming, GS_WRITE_YM_ADMINDB_CMD, strlen(GS_WRITE_YM_ADMINDB_CMD)) == 0)
    {
        //incoming = "YMWR:START:......."
        if(strncmp(incoming+strlen(GS_WRITE_YM_ADMINDB_CMD), GS_START_CMD, strlen(GS_START_CMD)) == 0)
        {
            //INIT
            if(sGS_CurrentCMD == GS_YMADMINDB_WRITE_SETTING_CMD)
            {
                //Already started
                //Can not open the file  send "STOP:"
                strncpy(lCommand, GS_STOP_CMD, GS_LC_VSEND_COMMAND_LENGTH);
                lCommand[GS_LC_VSEND_COMMAND_LENGTH] = '\0';

                strcpy(lText, "Writting on .gexlm_config.xml file already started");
            }
            else
            {
                // Init db file setting
                GS_initDBSettingFile();

                if(sGS_DatabaseSettingFile[0] == '\0')
                {
                    strncpy(lCommand, GS_STOP_CMD, GS_LC_VSEND_COMMAND_LENGTH);
                    lCommand[GS_LC_VSEND_COMMAND_LENGTH] = '\0';

                    strcpy(lText, "Missing .gexlm_config.xml file location");
                }
                else
                {
                    sGS_CurrentCMD = GS_YMADMINDB_WRITE_SETTING_CMD;
                    lHandle = fopen(sGS_DatabaseSettingFile, "w");
                    if(!lHandle)
                    {
                        sGS_CurrentCMD = GS_ALLOW_CMD;
                        //Can not open the file  send "STOP:"
                        strncpy(lCommand, GS_STOP_CMD, GS_LC_VSEND_COMMAND_LENGTH);
                        lCommand[GS_LC_VSEND_COMMAND_LENGTH] = '\0';

                        strcpy(lText, "Can not create .gexlm_config.xml file");
                    }
                    else
                    {
                        strncpy(lCommand, GS_NEXT_CMD, GS_LC_VSEND_COMMAND_LENGTH);
                        lCommand[GS_LC_VSEND_COMMAND_LENGTH] = '\0';

                        lText[0] = '\0';
                        fclose(lHandle);
                    }
                }
            }
        }
        else if(strncmp(incoming+strlen(GS_WRITE_YM_ADMINDB_CMD), GS_CONTINUE_CMD, strlen(GS_CONTINUE_CMD)) == 0)
        {
            //incoming = "YMWR:CONTINUE:...filePart..."
            if(!GSLM_appendFile(sGS_DatabaseSettingFile,
                                incoming+strlen(GS_WRITE_YM_ADMINDB_CMD)+strlen(GS_CONTINUE_CMD)))
            {
                strncpy(lCommand, GS_STOP_CMD, GS_LC_VSEND_COMMAND_LENGTH);
                lCommand[GS_LC_VSEND_COMMAND_LENGTH] = '\0';

                strcpy(lText, "Can not update .gexlm_config.xml file");
            }
            else
            {
                strncpy(lCommand, GS_NEXT_CMD, GS_LC_VSEND_COMMAND_LENGTH);
                lCommand[GS_LC_VSEND_COMMAND_LENGTH] = '\0';

                lText[0] = '\0';
            }

        }
        else if(strncmp(incoming+strlen(GS_WRITE_YM_ADMINDB_CMD), GS_END_CMD, strlen(GS_END_CMD)) == 0)
        {   //incoming = "YMWR:END:......."
            sGS_CurrentCMD = GS_ALLOW_CMD;

            strncpy(lCommand, GS_END_CMD, GS_LC_VSEND_COMMAND_LENGTH);
            lCommand[GS_LC_VSEND_COMMAND_LENGTH] = '\0';

            lText[0] = '\0';
        }
    }
    else if(strncmp(incoming, GS_READ_YM_ADMINDB_CMD, strlen(GS_READ_YM_ADMINDB_CMD)) == 0)
    {
        //incoming = "YMRD:......."
        if(sGS_CurrentCMD == GS_YMADMINDB_WRITE_SETTING_CMD)
        {
            //Returning error vd_msg = "STOP: ....."
            strncpy(lCommand, GS_STOP_CMD, GS_LC_VSEND_COMMAND_LENGTH);
            lCommand[GS_LC_VSEND_COMMAND_LENGTH] = '\0';

            strcpy(lText, "Update of .gexlm_config.xml is on-going");
        }
        else
        {
            if(strncmp(incoming+strlen(GS_READ_YM_ADMINDB_CMD), GS_START_CMD, strlen(GS_START_CMD)) == 0)
            {
                // Init db file setting
                GS_initDBSettingFile();

                if(sGS_DatabaseSettingFile[0] == '\0')
                {
                    strncpy(lCommand, GS_STOP_CMD, GS_LC_VSEND_COMMAND_LENGTH);
                    lCommand[GS_LC_VSEND_COMMAND_LENGTH] = '\0';

                    strcpy(lText, "Missing DB setting file location");
                }
                else
                {
                    //incoming = "YMRD:START:......."
                    lHandle = fopen(sGS_DatabaseSettingFile, "r");
                    if(!lHandle)
                    {
                        //Returning error vd_msg = "STOP: ....."
                        strncpy(lCommand, GS_STOP_CMD, GS_LC_VSEND_COMMAND_LENGTH);
                        lCommand[GS_LC_VSEND_COMMAND_LENGTH] = '\0';

                        strcpy(lText, "Can not open .gexlm_config.xml file");
                    }
                    else
                    {
                        //Returning the file size
                        fseek (lHandle , 0 , SEEK_END);
                        lFileSize = ftell (lHandle);

                        //Returning the file size vd_msg = "NEXT:'..FILE_SIZE..'...."
                        strncpy(lCommand, GS_NEXT_CMD, GS_LC_VSEND_COMMAND_LENGTH);
                        lCommand[GS_LC_VSEND_COMMAND_LENGTH] = '\0';

                        sprintf(lText, "%ld", lFileSize);

                        fclose(lHandle);
                    }
                }
            }
            else if(strncmp(incoming+strlen(GS_READ_YM_ADMINDB_CMD), GS_NEXT_CMD, strlen(GS_NEXT_CMD)) == 0)
            {
                //incoming = "YMRD:NEXT:'..PART_NUMBER...'"
                int part =  atoi(incoming+strlen(GS_READ_YM_ADMINDB_CMD)+strlen(GS_NEXT_CMD));
                lHandle = fopen(sGS_DatabaseSettingFile, "r");
                if(!lHandle)
                {
                    strncpy(lCommand, GS_STOP_CMD, GS_LC_VSEND_COMMAND_LENGTH);
                    lCommand[GS_LC_VSEND_COMMAND_LENGTH] = '\0';

                    strcpy(lText, "Can not open .gexlm_config.xml file");
                }
                else
                {
                    int idx = 0;
                    int readedByte = 0;
                    while(!feof(lHandle) && (idx<=part))
                    {
                        readedByte = fread(lText, sizeof(char), GS_LC_VSEND_TEXT_LENGTH, lHandle);
                        idx++;
                    }

                    if(readedByte > 0 && readedByte <= GS_LC_VSEND_TEXT_LENGTH)
                    {
                        lText[readedByte] = '\0';

                        if (feof(lHandle))
                        {
                            //its the last string prpended with GS_END_CMD
                            strncpy(lCommand, GS_END_CMD, GS_LC_VSEND_COMMAND_LENGTH);
                            lCommand[GS_LC_VSEND_COMMAND_LENGTH] = '\0';
                        }
                    }
                    else
                    {
                        strncpy(lCommand, GS_STOP_CMD, GS_LC_VSEND_COMMAND_LENGTH);
                        lCommand[GS_LC_VSEND_COMMAND_LENGTH] = '\0';

                        strcpy(lText, "Can not read last part of .gexlm_config.xml file");
                    }

                    fclose(lHandle);
                }
            }
        }

    }

    if (lCommand[0] != '\0')
        strcat(sGS_vd_msg, lCommand);
    if (lText[0] != '\0')
        strcat(sGS_vd_msg, lText);

    sprintf(lDumpData, "vd_msg: %s\n", sGS_vd_msg);
    GSLOG_appendFile(GS_VENDOR_DAEMON_LOG, lDumpData);

    return sGS_vd_msg;
}
