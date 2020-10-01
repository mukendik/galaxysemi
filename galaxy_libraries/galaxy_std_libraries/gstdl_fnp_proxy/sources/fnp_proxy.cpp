#ifdef __cplusplus
#  define LIB_GEX_LP_C_START           extern "C" {
#  define LIB_GEX_LP_C_END             }
#else
#  define LIB_GEX_LP_C_START
#  define LIB_GEX_LP_C_END
#endif

#ifdef LIB_GEX_LP_DLL_BUILD
    #ifndef LIB_GEX_LP_DLL
    #if defined(_MSC_VER) && !defined(LIB_GEX_LP_DISABLE_DLL)
    #  define LIB_GEX_LP_DLL     __declspec(dllexport)
    #  define LIB_GEX_LP_STDCALL __cdecl
    #else
    #  if defined(USE_GCC_VISIBILITY_FLAG)
    #    define LIB_GEX_LP_DLL     __attribute__ ((visibility("default")))
    #  define LIB_GEX_LP_STDCALL __cdecl
    #  else
    #    define LIB_GEX_LP_DLL
    #    define LIB_GEX_LP_STDCALL
    #  endif
    #endif
    #endif
#else
#ifdef LIB_GEX_LP_DLL_MAC
   #    define LIB_GEX_LP_DLL     __attribute__ ((visibility("default")))
   #    define LIB_GEX_LP_STDCALL
#  define LIB_GEX_LP_C_START           extern "C" {
#  define LIB_GEX_LP_C_END             }
#else
    #ifndef LIB_GEX_LP_DLL
    #if defined(_MSC_VER) && !defined(LIB_GEX_LP_DISABLE_DLL)
    #  define LIB_GEX_LP_DLL     __declspec(dllimport)
    #  define LIB_GEX_LP_STDCALL __cdecl
    #else
    #  if defined(USE_GCC_VISIBILITY_FLAG)
   #    define LIB_GEX_LP_DLL     __attribute__ ((visibility("default")))
    #  define LIB_GEX_LP_STDCALL __cdecl
    #  else
    #    define LIB_GEX_LP_DLL
    #    define LIB_GEX_LP_STDCALL
    #  endif
#  endif

    #endif
    #endif
#endif



#ifndef FLEXLM_DLL
#define FLEXLM_DLL
#endif

#include "lmclient.h"
#include "fnp_proxy.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#ifdef WINDOWS
    #include "wininstaller.h"
#endif

#include "lm_attr.h"
// 

static VENDORCODE sCode;
const char *	pszAppName = "gex";
const char *	pszPublisherName = "galaxy";

typedef void (*loger_call_back_function)(int sev, const char *messsage);


#define LOG_FILE "fnp_license_provider.log"
#define LOG_MESSAGE_BUFFER_SIZE 1000

class FNPProxy
{
    static LM_HANDLE *m_psLmJob;
    static flexinit_property_handle *m_poInitHandle;
    static char *m_sszLogMessage;

public:
    int getLastErrorCode();
    const char *getLastError();

protected:
    int init();
    int cleanup();

public:
    FNPProxy();
    ~FNPProxy();
    bool reserveLicense(const char *szFeature, const char *szVersion, int nlic, char *internalInfo[]);
    void releaseLicense(const char * szFeature);
    char *ClientDaemonQA(char *message);
    void idleStatus(int status);
    void setLicPath(const char *licPath);
    void setConnectionStatusCallBack(void *exitCallBack, void *reconnectCallBack, void *reconnectDoneCallBack);
    char *getHostId();
private :
    int m_iInitError;
public:
    int getInitError() {
        return m_iInitError;
    }
    static void loger(const char *szMessage);
    static void setLogFct(loger_call_back_function logerCallBackFct);
protected:
    static loger_call_back_function mlogerCallBackFct;
};


char *FNPProxy::m_sszLogMessage = NULL;
LM_HANDLE *FNPProxy::m_psLmJob = 0;
flexinit_property_handle *FNPProxy::m_poInitHandle = 0;
loger_call_back_function FNPProxy::mlogerCallBackFct = 0;

void FNPProxy::setLogFct(loger_call_back_function logerCallBackFct)
{
  mlogerCallBackFct = logerCallBackFct;

}
//static FILE* m_poHLogFile=0;
void FNPProxy::loger(const char *szMessage)
{
//    if(!m_poHLogFile){
//        m_poHLogFile = fopen(LOG_FILE, "a");
//        if(!m_poHLogFile)
//            m_poHLogFile = stdout;
//    }

//    char acDateTime[500];
//    time_t oTimestamp;
//    time(&oTimestamp);
//    strftime(acDateTime, sizeof(acDateTime), "%Y-%m-%d %H:%M:%S", localtime(&oTimestamp));

//    fprintf(m_poHLogFile,"%s : %s \n", acDateTime,szMessage);

//    fclose(m_poHLogFile);
//    m_poHLogFile = NULL;

    if(mlogerCallBackFct)
        mlogerCallBackFct(7, szMessage);
}

int FNPProxy::getLastErrorCode(){
    sprintf( m_sszLogMessage,  "getLastErrorCode m_psLmJob(0x%p)", m_psLmJob);
    loger(m_sszLogMessage);
    if(m_psLmJob)
        return lc_get_errno(m_psLmJob);
    else
        return 0;
}

const char *FNPProxy::getLastError(){

    sprintf( m_sszLogMessage,  "getLastError  m_psLmJob(0x%p)", m_psLmJob);
    loger(m_sszLogMessage);
    if(m_psLmJob)
        return lc_errstring(m_psLmJob);
    else
        return "NA";

}

int FNPProxy::init(){

    sprintf( m_sszLogMessage,  "FNPProxy::init() >> Start ");
    loger(m_sszLogMessage);
#ifdef WINDOWS
    sprintf( m_sszLogMessage,  "FNPProxy::init() >> fnpActSvcInstallWin");
    loger(m_sszLogMessage);
    unsigned int uiRet = fnpActSvcInstallWin("galaxy", "gex");
    unsigned int uiErrorCode = -1;
    if(uiRet != TRUE)
        uiErrorCode = fnpActSvcGetLastErrorWin();
    sprintf( m_sszLogMessage,  "E fnpActSvcInstallWin Return(%d) ErrorCode(%d)", uiRet, uiErrorCode);
    loger(m_sszLogMessage);
#endif /* WINDOWS */

    int iStat;
    sprintf( m_sszLogMessage,  "FNPProxy::init() >> lc_flexinit_property_handle_create ");
    loger(m_sszLogMessage);
    if (iStat = lc_flexinit_property_handle_create(&m_poInitHandle)){
        sprintf( m_sszLogMessage,  "FNPProxy::init() >> Fail lc_flexinit_property_handle_create with returned value(%d)",iStat);
        loger(m_sszLogMessage);
        return iStat;
    }
    sprintf( m_sszLogMessage,  "FNPProxy::init() >> succed lc_flexinit_property_handle_create with returned value(%d)",iStat);
    loger(m_sszLogMessage);

    sprintf( m_sszLogMessage,  "FNPProxy::init() >> lc_flexinit_property_handle_set");
    loger(m_sszLogMessage);
    if (iStat = lc_flexinit_property_handle_set(m_poInitHandle,
            (FLEXINIT_PROPERTY_TYPE)FLEXINIT_PROPERTY_USE_TRUSTED_STORAGE,
            (FLEXINIT_VALUE_TYPE)1)){
        sprintf( m_sszLogMessage,  "FNPProxy::init() >> Fail lc_flexinit_property_handle_set with returned value(%d)",iStat);
        loger(m_sszLogMessage);
        return iStat;
    }
    sprintf( m_sszLogMessage,  "FNPProxy::init() >> succed lc_flexinit_property_handle_set with returned value(%d)",iStat);
    loger(m_sszLogMessage);


    sprintf( m_sszLogMessage,  "FNPProxy::init() >> lc_flexinit");
    loger(m_sszLogMessage);
    if (iStat = lc_flexinit(m_poInitHandle)){
        sprintf( m_sszLogMessage,  "FNPProxy::init() >> warning lc_flexinit with returned value(%d)",iStat);
        loger(m_sszLogMessage);
        //return iStat;
    }
    sprintf( m_sszLogMessage,  "FNPProxy::init() >> succed lc_flexinit with returned value(%d)",iStat);
    loger(m_sszLogMessage);
    sprintf( m_sszLogMessage,  "FNPProxy::init() >> m_poInitHandle value(0x%p)",m_poInitHandle);
    loger(m_sszLogMessage);


    /* initialize the job handle for license checking */
    sprintf( m_sszLogMessage,  "FNPProxy::init() >> lc_new_job ");
    loger(m_sszLogMessage);
    int iRet;
    if(iRet = lc_new_job( 0, lc_new_job_arg2 /*NULL*//*lc_new_job_arg2*/, &sCode, &m_psLmJob ))//Set seconf argument to NULL
    {
        sprintf( m_sszLogMessage,  "FNPProxy::init() >> Fail lc_new_job with returned value(%d) m_psLmJob(0x%p)",iStat,m_psLmJob);
        loger(m_sszLogMessage);
        cleanup();
        return iRet;
    }
    sprintf( m_sszLogMessage,  "FNPProxy::init() >> succed lc_new_job with returned value(%d)",iRet);
    loger(m_sszLogMessage);
    sprintf( m_sszLogMessage,  "FNPProxy::init() >> m_psLmJob value(%d) m_psLmJob(0x%p)", iRet ,m_psLmJob);
    loger(m_sszLogMessage);

    (void)lc_set_attr(m_psLmJob, LM_A_CHECK_BADDATE, (LM_A_VAL_TYPE)1);
    (void)lc_set_attr(m_psLmJob, LM_A_TS_CHECK_BADDATE, (LM_A_VAL_TYPE)1);
    (void)lc_set_attr(m_psLmJob, LM_A_TCP_TIMEOUT, (LM_A_VAL_TYPE)4);
    (void)lc_set_attr(m_psLmJob, LM_A_CHECK_INTERVAL,(LM_A_VAL_TYPE)30);

    (void)lc_set_attr(m_psLmJob, LM_A_RETRY_COUNT,(LM_A_VAL_TYPE)1);
    (void)lc_set_attr(m_psLmJob, LM_A_RETRY_INTERVAL,(LM_A_VAL_TYPE)30);

    (void)lc_set_attr(m_psLmJob, LM_A_PROMPT_FOR_FILE,(LM_A_VAL_TYPE)0);
    (void)lc_set_attr(m_psLmJob, LM_A_DISABLE_ENV,(LM_A_VAL_TYPE)1);
    (void)lc_set_attr(m_psLmJob, LM_A_CKOUT_INSTALL_LIC,(LM_A_VAL_TYPE)0);
    (void)lc_set_attr(m_psLmJob, LM_A_SORT_TS_FIRST,(LM_A_VAL_TYPE)0);


    sprintf( m_sszLogMessage,  "FNPProxy::init() >> m_psLmJob->type value(%d)",m_psLmJob->type);
    loger(m_sszLogMessage);

    sprintf( m_sszLogMessage,  "FNPProxy::init() >> End");
    loger(m_sszLogMessage);
    return 0;
}

char *FNPProxy::getHostId()
{
    sprintf( m_sszLogMessage,  "FNPProxy::getHostId() >> lc_hostid ");
    loger(m_sszLogMessage);

    static char hostid[MAX_CONFIG_LINE];
    int ret = lc_hostid(m_psLmJob, HOSTID_ETHER, hostid);
    if(ret != 0)
    {
        sprintf( hostid, "#");
    }

    return hostid;
}

void FNPProxy::setLicPath(const char *licPath)
{
  sprintf( m_sszLogMessage,  "FNPProxy::setLicPath() >> lc_set_attr ");
  loger(m_sszLogMessage);
  (void)lc_set_attr(m_psLmJob, LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE)licPath);

}

void FNPProxy::setConnectionStatusCallBack(void *exitCallBack, void *reconnectCallBack, void *reconnectDoneCallBack)
{
  sprintf( m_sszLogMessage,  "FNPProxy::setConnectionStatusCallBack() >> lc_set_attr ");
  loger(m_sszLogMessage);
  sprintf( m_sszLogMessage,  "FNPProxy::setConnectionStatusCallBack() >> exitCallBack(0x%p) reconnectCallBack(0x%p) reconnectDoneCallBack(0x%p)",exitCallBack,reconnectCallBack,reconnectDoneCallBack);
  loger(m_sszLogMessage);

  if(exitCallBack)
      (void)lc_set_attr(m_psLmJob, LM_A_USER_EXITCALL, (LM_A_VAL_TYPE)exitCallBack);
  if(reconnectCallBack)
      (void)lc_set_attr(m_psLmJob, LM_A_USER_RECONNECT, (LM_A_VAL_TYPE)reconnectCallBack);
  if(reconnectDoneCallBack)
      (void)lc_set_attr(m_psLmJob, LM_A_USER_RECONNECT_DONE, (LM_A_VAL_TYPE)reconnectDoneCallBack);

}

int FNPProxy::cleanup(){
    int iStat;
    sprintf( m_sszLogMessage,  "FNPProxy::cleanup() >> Start");
    loger(m_sszLogMessage);

    sprintf( m_sszLogMessage,  "FNPProxy::cleanup() >> try to free m_psLmJob(0x%p)", m_psLmJob);
    loger(m_sszLogMessage);
    if(m_psLmJob)
        lc_free_job( m_psLmJob );
    sprintf( m_sszLogMessage,  "FNPProxy::cleanup() >> Free m_psLmJob(0x%p)", m_psLmJob);
    loger(m_sszLogMessage);


    sprintf( m_sszLogMessage,  "FNPProxy::cleanup() >> try to free m_poInitHandle(0x%p)", m_poInitHandle);
    loger(m_sszLogMessage);
    if(m_poInitHandle){
        sprintf( m_sszLogMessage,  "FNPProxy::cleanup() >> lc_flexinit_cleanup");
        loger(m_sszLogMessage);
        if (iStat = lc_flexinit_cleanup(m_poInitHandle)){
            sprintf( m_sszLogMessage,  "FNPProxy::cleanup() >> Fail lc_flexinit_cleanup with returned value(%d)",iStat);
            loger(m_sszLogMessage);
            return iStat;
        }
        sprintf( m_sszLogMessage,  "FNPProxy::cleanup() >> Succed lc_flexinit_cleanup with returned value(%d)",iStat);
        loger(m_sszLogMessage);

        sprintf( m_sszLogMessage,  "FNPProxy::cleanup() >> lc_flexinit_property_handle_free ");
        loger(m_sszLogMessage);
        if (iStat = lc_flexinit_property_handle_free(m_poInitHandle)){
            sprintf( m_sszLogMessage,  "FNPProxy::cleanup() >> Fail lc_flexinit_property_handle_free with returned value(%d)",iStat);
            loger(m_sszLogMessage);
            return iStat;
        }
        sprintf( m_sszLogMessage,  "FNPProxy::cleanup() >> Succed lc_flexinit_property_handle_free with returned value(%d)",iStat);
        loger(m_sszLogMessage);
    }
    sprintf( m_sszLogMessage,  "FNPProxy::cleanup() >> Free m_poInitHandle(0x%p) ", m_poInitHandle);
    loger(m_sszLogMessage);

#ifdef WINDOWS
    sprintf( m_sszLogMessage,  "FNPProxy::cleanup() >> call lc_cleanup");
    loger(m_sszLogMessage);
    lc_cleanup();
#endif /* WINDOWS */

    sprintf( m_sszLogMessage,  "FNPProxy::cleanup() >> End ");
    loger(m_sszLogMessage);


    return 0;

}

FNPProxy::FNPProxy(){
    m_iInitError = 0;
    m_sszLogMessage = 0;
    if(!m_sszLogMessage)
        m_sszLogMessage = new char[20000];
    sprintf( m_sszLogMessage,  "FNPProxy::FNPProxy() >> Start");
    loger(m_sszLogMessage);
    sprintf( m_sszLogMessage,  "FNPProxy::FNPProxy() >> init : m_psLmJob(0x%p) m_poInitHandle(0x%p)",m_psLmJob,m_poInitHandle);
    loger(m_sszLogMessage);

    m_iInitError = init();

    sprintf( m_sszLogMessage,  "FNPProxy::FNPProxy() >> Returned Init value(%d)", m_iInitError);
    loger(m_sszLogMessage);
    sprintf( m_sszLogMessage,  "FNPProxy::FNPProxy() >> End");
    loger(m_sszLogMessage);
}

FNPProxy::~FNPProxy(){
    setLogFct(NULL);
    cleanup();
    if(m_sszLogMessage)
        delete []m_sszLogMessage;
    m_sszLogMessage = NULL;
}

bool FNPProxy::reserveLicense(const char * szFeature, const char *  szVersion, int nlic, char *internalInfo[]){

    sprintf( m_sszLogMessage,  "FNPProxy::reserveLicense() >> Start ");
    loger(m_sszLogMessage);
    sprintf( m_sszLogMessage,  "FNPProxy::reserveLicense() >> Try to reserve Feature(%s) Version(%s) nlic(%d) with Job(0x%p)",szFeature,szVersion, nlic,m_psLmJob);
    loger(m_sszLogMessage);
    char acFeature[256];
    char acVersion[256];
    strcpy(acFeature, szFeature);
    strcpy(acVersion, szVersion);

    sprintf( m_sszLogMessage,  "FNPProxy::reserveLicense() >> lc_checkout ");
    loger(m_sszLogMessage);
    int lc_checkout_status = lc_checkout(m_psLmJob, acFeature, acVersion, nlic, LM_CO_NOWAIT, &sCode, LM_DUP_NONE);

    if(lc_checkout_status == 0)
    {

        if(internalInfo)
        {
            //0 expiration date
            //1 release date
            //2 maintenance date
            //3 license type
            //4 is an overdraft
            sprintf( m_sszLogMessage,  "FNPProxy::reserveLicense() >> Retriving internalInfo ");
            loger(m_sszLogMessage);
            CONFIG * configFeature = lc_auth_data(m_psLmJob,acFeature);
            if(configFeature)
            {
                //Job Type
                sprintf( m_sszLogMessage,  "lc_get_attr >> m_psLmJob->type value(%d)",m_psLmJob->type);
                loger(m_sszLogMessage);
                int getAttr = 0;

                sprintf( m_sszLogMessage,"<<date%s>>",configFeature->date);
                loger(m_sszLogMessage);
                sprintf( m_sszLogMessage,"<<startdate %s>>",configFeature->startdate);
                loger(m_sszLogMessage);

                sprintf( m_sszLogMessage,"<<init>>");
                loger(m_sszLogMessage);

                strcpy(internalInfo[0],configFeature->date);
                strcpy(internalInfo[1],configFeature->startdate);
                strcpy(internalInfo[2],"");


                //Dans le cas ou la feature viens d'un daemon conf->server va avoir l'adresse du daemon ou NULL
                //si c'est une license nodelocked. La valeur conf->idptr est NULL si c'est du trusted storage.
                sprintf( m_sszLogMessage,"-----------------CONFIG Content-----------------------");
                loger(m_sszLogMessage);
                sprintf( m_sszLogMessage,"<<configFeature->server(%p)>>",configFeature->server);
                loger(m_sszLogMessage);
                if(configFeature->server)
                {
                    sprintf( m_sszLogMessage,"<<\tconfigFeature->server.name(%s)>>",configFeature->server->name);
                    loger(m_sszLogMessage);
                    sprintf( m_sszLogMessage,"<<\tconfigFeature->server.idptr(%p)>>",configFeature->server->idptr);
                    loger(m_sszLogMessage);
                    if(configFeature->server->idptr)
                    {
                        sprintf( m_sszLogMessage,"<<configFeature->server->idptr->type (%d)>>",configFeature->server->idptr->type);
                        loger(m_sszLogMessage);
                        sprintf( m_sszLogMessage,"<<configFeature->server->idptr->override (%d)>>",configFeature->server->idptr->override);
                        loger(m_sszLogMessage);
                        sprintf( m_sszLogMessage,"<<configFeature->server->idptr->representation (%d)>>",configFeature->server->idptr->representation);
                        loger(m_sszLogMessage);


                    }

                    sprintf( m_sszLogMessage,"<<\tconfigFeature->server.commtype(%d)>>",configFeature->server->commtype);
                    loger(m_sszLogMessage);
                    sprintf( m_sszLogMessage,"<<\tconfigFeature->server.port(%d)>>",configFeature->server->port);
                    loger(m_sszLogMessage);
                    sprintf( m_sszLogMessage,"<<\tconfigFeature->server.filename(%p)>>",configFeature->server->filename);
                    loger(m_sszLogMessage);
                }

                sprintf( m_sszLogMessage,"<<configFeature->idptr(%p)>>",configFeature->idptr);
                loger(m_sszLogMessage);

                sprintf( m_sszLogMessage,"<<configFeature->lf(%d)>>",configFeature->lf);
                loger(m_sszLogMessage);

                sprintf( m_sszLogMessage,"-----------------CONFIG Content-----------------------");
                loger(m_sszLogMessage);

                if(configFeature->server && configFeature->server->port == -1)
                {
                    strcpy(internalInfo[3],"nodelocked_uncounted");
                    (void)lc_set_attr(m_psLmJob, LM_A_PHYSICAL_ETHERNETID,(LM_A_VAL_TYPE)1);
                }
                else if(configFeature->server && configFeature->server->port > -1)
                    strcpy(internalInfo[3],"floating");
//                else if(configFeature->server && configFeature->idptr)
//                    strcpy(internalInfo[3],"nodelocked_counted");

                LM_VD_FEATURE_INFO fi;
                fi.feat = configFeature;
                getAttr = lc_get_attr(m_psLmJob, LM_A_VD_FEATURE_INFO, (LM_SHORT_PTR)&fi);
                if(getAttr == 0)
                {
                    sprintf( m_sszLogMessage,"!!! lc_get_attr Overdraft succed %d",getAttr);
                    loger(m_sszLogMessage);
                    sprintf( m_sszLogMessage,"!!!! lc_get_attr Overdraft fi %p",&fi);
                    loger(m_sszLogMessage);
                    bool isOverdraft = fi.tot_lic_in_use > (fi.num_lic - fi.overdraft);
                    if(isOverdraft)
                    {
                        strcpy(internalInfo[4],"overdraft_license");
                    }
                    else
                    {
                        strcpy(internalInfo[4],"normal_license");
                    }
                    sprintf( m_sszLogMessage,"isOverdraft %s",internalInfo[4]);
                    loger(m_sszLogMessage);

                }
                else
                {
                    sprintf( m_sszLogMessage,"lc_get_attr Overdraft fail fi: %p => %d\n",&fi, getAttr);
                    loger(m_sszLogMessage);

                }
            }
            else
            {
                sprintf( m_sszLogMessage,"NO config found");
                loger(m_sszLogMessage);
            }
        }
        sprintf( m_sszLogMessage,  "FNPProxy::reserveLicense() >> End with Sucees ");
        loger(m_sszLogMessage);
        return true;
    }
    else
    {
        if((strcmp(acFeature, "Maintenance" ) == 0) && internalInfo)
        {
            sprintf( m_sszLogMessage,  "FNPProxy::reserveLicense() >> Retriving internalInfo ");
            loger(m_sszLogMessage);
            CONFIG * configFeature = lc_get_config(m_psLmJob,acFeature);
            if(configFeature)
            {
                sprintf( m_sszLogMessage,  "lc_get_attr >> m_psLmJob->type value(%d)",m_psLmJob->type);
                loger(m_sszLogMessage);
                int getAttr = 0;

                sprintf( m_sszLogMessage,"<<date%s>>",configFeature->date);
                loger(m_sszLogMessage);
                sprintf( m_sszLogMessage,"<<startdate %s>>",configFeature->startdate);
                loger(m_sszLogMessage);

                sprintf( m_sszLogMessage,"<<init>>");
                loger(m_sszLogMessage);

                strcpy(internalInfo[0],configFeature->date);
                strcpy(internalInfo[1],configFeature->startdate);
                strcpy(internalInfo[2],"");


                //Dans le cas ou la feature viens d'un daemon conf->server va avoir l'adresse du daemon ou NULL
                //si c'est une license nodelocked. La valeur conf->idptr est NULL si c'est du trusted storage.
                sprintf( m_sszLogMessage,"-----------------CONFIG Content-----------------------");
                loger(m_sszLogMessage);
                sprintf( m_sszLogMessage,"<<configFeature->server(%p)>>",configFeature->server);
                loger(m_sszLogMessage);
                if(configFeature->server)
                {
                    sprintf( m_sszLogMessage,"<<\tconfigFeature->server.name(%s)>>",configFeature->server->name);
                    loger(m_sszLogMessage);
                    sprintf( m_sszLogMessage,"<<\tconfigFeature->server.idptr(%p)>>",configFeature->server->idptr);
                    loger(m_sszLogMessage);
                    if(configFeature->server->idptr)
                    {
                        sprintf( m_sszLogMessage,"<<configFeature->server->idptr->type (%d)>>",configFeature->server->idptr->type);
                        loger(m_sszLogMessage);
                        sprintf( m_sszLogMessage,"<<configFeature->server->idptr->override (%d)>>",configFeature->server->idptr->override);
                        loger(m_sszLogMessage);
                        sprintf( m_sszLogMessage,"<<configFeature->server->idptr->representation (%d)>>",configFeature->server->idptr->representation);
                        loger(m_sszLogMessage);


                    }

                    sprintf( m_sszLogMessage,"<<\tconfigFeature->server.commtype(%d)>>",configFeature->server->commtype);
                    loger(m_sszLogMessage);
                    sprintf( m_sszLogMessage,"<<\tconfigFeature->server.port(%d)>>",configFeature->server->port);
                    loger(m_sszLogMessage);
                    sprintf( m_sszLogMessage,"<<\tconfigFeature->server.filename(%p)>>",configFeature->server->filename);
                    loger(m_sszLogMessage);
                }

                sprintf( m_sszLogMessage,"<<configFeature->idptr(%p)>>",configFeature->idptr);
                loger(m_sszLogMessage);

                sprintf( m_sszLogMessage,"<<configFeature->lf(%d)>>",configFeature->lf);
                loger(m_sszLogMessage);

                sprintf( m_sszLogMessage,"-----------------CONFIG Content-----------------------");
                loger(m_sszLogMessage);

                if(configFeature->server && configFeature->server->port == -1)
                {
                    strcpy(internalInfo[3],"nodelocked_uncounted");
                    (void)lc_set_attr(m_psLmJob, LM_A_PHYSICAL_ETHERNETID,(LM_A_VAL_TYPE)1);
                }
                else if(configFeature->server && configFeature->server->port > -1)
                    strcpy(internalInfo[3],"floating");
                //                else if(configFeature->server && configFeature->idptr)
                //                    strcpy(internalInfo[3],"nodelocked_counted");

                LM_VD_FEATURE_INFO fi;
                fi.feat = configFeature;
                getAttr = lc_get_attr(m_psLmJob, LM_A_VD_FEATURE_INFO, (LM_SHORT_PTR)&fi);
                if(getAttr == 0)
                {
                    sprintf( m_sszLogMessage,"!!! lc_get_attr Overdraft succed %d",getAttr);
                    loger(m_sszLogMessage);
                    sprintf( m_sszLogMessage,"!!!! lc_get_attr Overdraft fi %p",&fi);
                    loger(m_sszLogMessage);
                    bool isOverdraft = fi.tot_lic_in_use > (fi.num_lic - fi.overdraft);
                    if(isOverdraft)
                    {
                        strcpy(internalInfo[4],"overdraft_license");
                    }
                    else
                    {
                        strcpy(internalInfo[4],"normal_license");
                    }
                    sprintf( m_sszLogMessage,"isOverdraft %s",internalInfo[4]);
                    loger(m_sszLogMessage);

                }
                else
                {
                    sprintf( m_sszLogMessage,"lc_get_attr Overdraft fail fi: %p => %d\n",&fi, getAttr);
                    loger(m_sszLogMessage);

                }
            }

        }
        sprintf( m_sszLogMessage,  "FNPProxy::reserveLicense() >> End with fail");
        loger(m_sszLogMessage);
        return false;
    }
}

void FNPProxy::releaseLicense(const char *szFeature){


    sprintf( m_sszLogMessage,  "FNPProxy::releaseLicense() >> Start");
    loger(m_sszLogMessage);
    sprintf( m_sszLogMessage,  "FNPProxy::releaseLicense() >> Try to release Feature(%s) with Job(0x%p)",szFeature,m_psLmJob);
    loger(m_sszLogMessage);

    char acFeature[256];
    strcpy(acFeature, szFeature);

    sprintf( m_sszLogMessage,  "FNPProxy::releaseLicense() >> lc_checkin");
    loger(m_sszLogMessage);
    lc_checkin(m_psLmJob, acFeature, 5);
    sprintf( m_sszLogMessage,  "FNPProxy::releaseLicense() >> End");
    loger(m_sszLogMessage);
}

char * FNPProxy::ClientDaemonQA(char *message)
{
  sprintf( m_sszLogMessage,  "FNPProxy::sendMessageToDaemon() >> Start");
  loger(m_sszLogMessage);
  sprintf( m_sszLogMessage,  "FNPProxy::sendMessageToDaemon() >> Try to send message(%s) with Job(0x%p) to daemon",message,m_psLmJob);
  loger(m_sszLogMessage);

  char *responseString = lc_vsend(m_psLmJob, message);
  int lcVsendLastError = getLastErrorCode();

  sprintf( m_sszLogMessage,  "FNPProxy::sendMessageToDaemon() >>getLastErrorCode returns (%d) ",lcVsendLastError);
  loger(m_sszLogMessage);

  sprintf( m_sszLogMessage,  "FNPProxy::sendMessageToDaemon() >> Daemon response responseString@(0x%p) *responseString(#%s#)", responseString, (responseString ? responseString : "NULL"));
  loger(m_sszLogMessage);

  sprintf( m_sszLogMessage,  "FNPProxy::sendMessageToDaemon() >> Daemon response with (%s) with Job(0x%p) to daemon", (responseString ? responseString : "NULL"), m_psLmJob);
  loger(m_sszLogMessage);
  sprintf( m_sszLogMessage,  "FNPProxy::sendMessageToDaemon() >> End");
  loger(m_sszLogMessage);
  return responseString;
}

void FNPProxy::idleStatus(int status)
{
  sprintf( m_sszLogMessage,  "FNPProxy::idleStatus() >> Start");
  loger(m_sszLogMessage);
  sprintf( m_sszLogMessage,  "FNPProxy::idleStatus() >> Try idle (%d) with Job(0x%p) to daemon",status,m_psLmJob);
  loger(m_sszLogMessage);

  lc_idle(m_psLmJob, status);

  sprintf( m_sszLogMessage,  "FNPProxy::idleStatus() >> End");
  loger(m_sszLogMessage);
}


static FNPProxy *sGexProxyToFNPLPInstance = NULL;
/*LIB_GEX_LP_DLL*/ FNPProxy* LIB_GEX_LP_STDCALL getLPInstance(int &iErrorLib, char *szErrorStringLib){

    char szLogMessage[LOG_MESSAGE_BUFFER_SIZE];
    sprintf( szLogMessage,  "getLPInstance >> Start");
    FNPProxy::loger(szLogMessage);
    sprintf( szLogMessage,  "getLPInstance >> Initialize FNPProxy (0x%p)", sGexProxyToFNPLPInstance);
    FNPProxy::loger(szLogMessage);
    if(!sGexProxyToFNPLPInstance){
        sGexProxyToFNPLPInstance = new FNPProxy();
        sprintf( szLogMessage,  "getLPInstance >> returned FNPProxy (0x%p)", sGexProxyToFNPLPInstance);
        FNPProxy::loger(szLogMessage);
        if(!sGexProxyToFNPLPInstance)
            return NULL;

        int iError = sGexProxyToFNPLPInstance->getLastErrorCode();
        if( iError != 0 || sGexProxyToFNPLPInstance->getInitError() != 0 ){
            sprintf( szLogMessage,  "getLPInstance >> iError(%d) getInitError (%d)", iError, sGexProxyToFNPLPInstance->getInitError());
            FNPProxy::loger(szLogMessage);

            sprintf( szLogMessage,  "getLPInstance >> Process Error");
            FNPProxy::loger(szLogMessage);

            if(iError != 0)
            {
                sprintf( szLogMessage,  "getLPInstance >> iError != 0");
                FNPProxy::loger(szLogMessage);

                sprintf( szLogMessage,  "getLPInstance >> Processing iError(%d) ", iError);
                FNPProxy::loger(szLogMessage);

                iErrorLib = iError;
                const char *szInternalError = sGexProxyToFNPLPInstance->getLastError();
                sprintf(szErrorStringLib,"%s",szInternalError);
            }
            else
            {
                sprintf( szLogMessage,  "getLPInstance >>  Processing getInitError");
                FNPProxy::loger(szLogMessage);

                sprintf( szLogMessage,  "getLPInstance >> Processing getInitError(%d) ",  sGexProxyToFNPLPInstance->getInitError());
                 FNPProxy::loger(szLogMessage);

                iErrorLib = sGexProxyToFNPLPInstance->getInitError();
                const char *szInternalError = "Internal error when initializing the proxy object";
                sprintf(szErrorStringLib,"%s",szInternalError);
            }

            sprintf( szLogMessage,  "getLPInstance >> returned FNPProxy with error(0x%d)", iError);
            FNPProxy::loger(szLogMessage);
            delete sGexProxyToFNPLPInstance;
            sGexProxyToFNPLPInstance = NULL;
        }
    }
    sprintf( szLogMessage,  "getLPInstance >> returned FNPProxy (0x%p)", sGexProxyToFNPLPInstance);
    FNPProxy::loger(szLogMessage);
    sprintf( szLogMessage,  "getLPInstance >> End");
    FNPProxy::loger(szLogMessage);
    return sGexProxyToFNPLPInstance;
}

/*LIB_GEX_LP_DLL*/ void LIB_GEX_LP_STDCALL freeLPInstance(){

    char szLogMessage[LOG_MESSAGE_BUFFER_SIZE];
    sprintf( szLogMessage,  "freeLPInstance >> Start");
    FNPProxy::loger(szLogMessage);

    if(sGexProxyToFNPLPInstance){
        delete sGexProxyToFNPLPInstance;
        sGexProxyToFNPLPInstance = NULL;
    }

    sprintf( szLogMessage,  "freeLPInstance >> End \n");
    FNPProxy::loger(szLogMessage);
}

LIB_GEX_LP_C_START

bool LIB_GEX_LP_STDCALL initLibrary(int &iError, char *szErrorString, void *logerCallBackFunction)
{
  loger_call_back_function logerCallBackFct  = (loger_call_back_function)(logerCallBackFunction);
  FNPProxy::loger("initLibrary 1\n");
  if(!logerCallBackFct)
  {
      FNPProxy::loger("initLibrary 2\n");
      //(*szErrorString) = new char[20];
      iError = 1;
      sprintf(szErrorString,"%s", "No loger funtion found");
      FNPProxy::loger("initLibrary 3\n");
      return false;
  }
  FNPProxy::setLogFct(logerCallBackFct);
  FNPProxy::loger("initLibrary 4\n");

  FNPProxy::loger("initLibrary 5\n");
  if(!getLPInstance(iError, szErrorString))
  {
      FNPProxy::loger("initLibrary 6\n");
      return false;
  }

  FNPProxy::loger("initLibrary 7\n");
  return true;
}

void LIB_GEX_LP_STDCALL cleanupLibrary(){

    freeLPInstance();
}

bool LIB_GEX_LP_STDCALL reserveLicense (const char *szFeature, const char *szVersion, int nlic, char *internalInfo[]){

    char szLogMessage[LOG_MESSAGE_BUFFER_SIZE];
    sprintf( szLogMessage, "reserveLicense >> Start");
    FNPProxy::loger(szLogMessage);

    sprintf( szLogMessage, "reserveLicense >> Try to reserve Feature(%s) Version(%s) @FNP Instance (0x%p)",szFeature,szVersion,sGexProxyToFNPLPInstance);
    FNPProxy::loger(szLogMessage);

    bool bRet = sGexProxyToFNPLPInstance->reserveLicense(szFeature, szVersion,nlic,internalInfo);

    sprintf( szLogMessage, "reserveLicense >> End withe return value (%d)", bRet);
    FNPProxy::loger(szLogMessage);

    return bRet;
}

void LIB_GEX_LP_STDCALL releaseLicense (const char *szFeature){

    char szLogMessage[LOG_MESSAGE_BUFFER_SIZE];
    sprintf( szLogMessage, "ReleaseLicense >> Start \n");
    FNPProxy::loger(szLogMessage);
    sprintf( szLogMessage, "ReleaseLicense >> Try to release Feature(%s) \n",szFeature);
    FNPProxy::loger(szLogMessage);

    if (sGexProxyToFNPLPInstance != NULL)
    {
        sGexProxyToFNPLPInstance->releaseLicense(szFeature);
    }

    sprintf( szLogMessage, "ReleaseLicense >> End \n");
    FNPProxy::loger(szLogMessage);


}

char * LIB_GEX_LP_STDCALL ClientDaemonQA (char *message){

    char szLogMessage[LOG_MESSAGE_BUFFER_SIZE];
    sprintf( szLogMessage, "ClientDaemonQA >> returning \n");
    FNPProxy::loger(szLogMessage);
    return sGexProxyToFNPLPInstance->ClientDaemonQA(message);
}

void LIB_GEX_LP_STDCALL idleStatus (int status){

    char szLogMessage[LOG_MESSAGE_BUFFER_SIZE];
    sprintf( szLogMessage, "idleStatus >> returning \n");
    FNPProxy::loger(szLogMessage);
    sGexProxyToFNPLPInstance->idleStatus(status);
}

void LIB_GEX_LP_STDCALL setLicPath(const char *licPath)
{
  char szLogMessage[LOG_MESSAGE_BUFFER_SIZE];
  sprintf( szLogMessage, "setLicPath >> returning \n");
  FNPProxy::loger(szLogMessage);
  sGexProxyToFNPLPInstance->setLicPath(licPath);

}

void LIB_GEX_LP_STDCALL setConnectionStatusCallBack(void *exitCallBack, void *reconnectCallBack, void *reconnectDoneCallBack)
{
  char szLogMessage[LOG_MESSAGE_BUFFER_SIZE];
  sprintf( szLogMessage, "setConnectionStatusCallBack >> returning \n");
  FNPProxy::loger(szLogMessage);
  sGexProxyToFNPLPInstance->setConnectionStatusCallBack(exitCallBack, reconnectCallBack, reconnectDoneCallBack);
}


const char *LIB_GEX_LP_STDCALL getLastError (){

    char szLogMessage[LOG_MESSAGE_BUFFER_SIZE];
    sprintf( szLogMessage, "getLastError >> returning \n");
    FNPProxy::loger(szLogMessage);
    return sGexProxyToFNPLPInstance->getLastError();
}

int LIB_GEX_LP_STDCALL getLastErrorCode (){

    char szLogMessage[LOG_MESSAGE_BUFFER_SIZE];
    sprintf( szLogMessage, "getLastErrorCode >> returning \n");
    FNPProxy::loger(szLogMessage);

    return sGexProxyToFNPLPInstance->getLastErrorCode();
}

char * LIB_GEX_LP_STDCALL getHostID()
{
    char szLogMessage[LOG_MESSAGE_BUFFER_SIZE];
    sprintf( szLogMessage, "getHostId >> returning \n");
    FNPProxy::loger(szLogMessage);

    return sGexProxyToFNPLPInstance->getHostId();
}

char * LIB_GEX_LP_STDCALL getLibVersion()
{
    char szLogMessage[LOG_MESSAGE_BUFFER_SIZE];
    sprintf( szLogMessage, "getLibVersion >> returning \n");
    FNPProxy::loger(szLogMessage);

    static char lVersion[10];
    sprintf( lVersion, "%s",GS_FNP_PROXY_LIB_VERSION);
    return lVersion;
}


LIB_GEX_LP_DLL void* LIB_GEX_LP_STDCALL resSym(const char *routineName){

    //A zero value indicates that both strings are equal. strcmp ( const char * str1, const char * str2 );
    if(strcmp(routineName, "cleanupLibrary" ) == 0)	return 	(void*)cleanupLibrary;
    if(strcmp(routineName, "ClientDaemonQA" ) == 0)	return 	(void*)ClientDaemonQA;
    if(strcmp(routineName, "getLastError" ) == 0)	return 	(void*)getLastError  ;
    if(strcmp(routineName, "getLastErrorCode" ) == 0)	return 	(void*)getLastErrorCode;
    if(strcmp(routineName, "idleStatus" ) == 0)	return 	(void*)idleStatus;
    if(strcmp(routineName, "initLibrary" ) == 0)	return 	(void*)initLibrary;
    if(strcmp(routineName, "releaseLicense" ) == 0)	return 	(void*)releaseLicense;
    if(strcmp(routineName, "reserveLicense" ) == 0)	return 	(void*)reserveLicense;
    if(strcmp(routineName, "setConnectionStatusCallBack" ) ==0)	return 	(void*)setConnectionStatusCallBack;
    if(strcmp(routineName, "setLicPath" ) == 0)	return 	(void*)setLicPath;
    if(strcmp(routineName, "getHostID" ) == 0)	return 	(void*)getHostID;
    if(strcmp(routineName, "getLibVersion" ) == 0)	return 	(void*)getLibVersion;

    return NULL;
}

LIB_GEX_LP_C_END
