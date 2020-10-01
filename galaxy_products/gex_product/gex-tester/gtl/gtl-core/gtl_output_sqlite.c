#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <string.h> // for memcpy
//#include <limits> // only C++
#include <gtc_netmessage.h>
#include <gstdl_utils_c.h>
//#include <pthread.h> // test for cross platform availablility : not available on win64-tdm : to be added manually ?

#include "sqlite3.h" // local to GTL project as we cant use the one from Qt sources...
#include "gtl_core.h"
#include "gtl_message.h"
#include "gtl_constants.h"
#include "gtl_testlist.h"
#include "gtl_profile.h"
#include <tdr_gs_version.h>

extern GTL_GlobalInfo gtl_stGlobalInfo; /* GTL Global information */
extern GTL_ProdInfo gtl_stProdInfo;
extern GTL_Station gtl_stStation;
extern GTL_PatSettings gtl_stPatSettings; /* Pat settings */
extern SiteItem	gtl_tl_ptSiteArray[]; /* Array of sites */
extern char* gtl_GetGalaxySemiTempFolder(); // Get the temp folder to be used by GTL
extern TestItem* gtl_tl_FindTestBySiteNb(unsigned int uiSiteNb, long lTestNb, char* szTestName, int PinIndex,
                                    unsigned int uiFindTestKey, int nFindMode);


/*
#define include_file(x) include x

#define INCLUDE_TO_STRING(sn, filename)  char* sn="\
    #include_file(filename) \
    ";
    // INCLUDE_TO_STRING(create_script, "tdr_embedded.sql")
*/

/* Todo inorder to include the .sql in a C/C++ source :
    - remove all CRLF (do a join lines with NotePad) and start and finish the whole script with a "
    or
    - include each line in a "....."
    - remove all SET commands
    - remove all comments : -------
    - remove all 'db_name'.
    - move all UNSIGNED before the type : INT(...)/SMALLINT(...)/...   (not after : INT(...) UNSIGNED)
    - remove all DEFAULT CHARACTER SET...
    - AUTOINCREMENT is only allowed on an INTEGER PRIMARY KEY
    - modify any \0 by \\0
*/

char create_script[]= {
  #include "gs_tdr_b3_c.sql"
};

/* Return codes in SQLite are from 0 to ..., 0 is OK. */
// GTL_LOG(3, "SQLite ext res code: %d", sqlite3_extended_result_codes(sSQLiteDB) );
#define CHECK_SQLITE_RC_FATAL(rc)   if (rc!=SQLITE_OK) { \
    GTL_LOG(3, "SQLite error: %d : %s", rc, zErrMsg?zErrMsg:"?"); \
    GTL_LOG(3, sQuery, 0, 0, 0); \
    sqlite3_free(zErrMsg); \
    char lP[1024]=""; \
    sprintf(lP, "%s/%s", gtl_stGlobalInfo.mOutputFolder?gtl_stGlobalInfo.mOutputFolder:".", \
            gtl_stGlobalInfo.mOutputFileName?gtl_stGlobalInfo.mOutputFileName:"gtl.sqlite"); \
    gtl_backup_sqlite_db(sSQLiteDB, lP, 'Y'); \
    sqlite3_close( sSQLiteDB ); \
    sSQLiteDB=0; \
    return -rc; \
    }

#define CHECK_SQLITE_RC(rc)   if (rc!=SQLITE_OK) { \
    GTL_LOG(3, "SQLite error: %d : %s", rc, zErrMsg?zErrMsg:"?"); \
    GTL_LOG(3, sQuery?sQuery:"?", 0, 0, 0); \
    }

sqlite3 *sSQLiteDB=0;
char *zErrMsg = 0;
char sQuery[120000]="";

// lCloseTargetAfterTransfert = Y or N
int gtl_transfert_sqlite_db(sqlite3 *lSourceDB, sqlite3 *lTargetDB, char lCloseTargetAfterTransfert)
{
    GTL_LOG(6, "gtl_transfert_sqlite_db...",0);
    sqlite3_backup *dbbackup = sqlite3_backup_init(lTargetDB, "main", lSourceDB, "main");
    int rc=0;
    if (!dbbackup)
    {
        if (lTargetDB)
        {
            rc = sqlite3_errcode(lTargetDB);
            GTL_LOG(3, "sqlite3_backup_init failed : target DB sqlite3 errcode = %d", rc)
        }
        else
            rc=-1;
        //CHECK_SQLITE_RC(rc)

        return rc;
    }
    rc=sqlite3_backup_step(dbbackup, -1); // All pages ?
    if (rc!=SQLITE_DONE)
    {
        GTL_LOG(3, "sqlite3_backup_step failed: %d", rc);
        return rc; //CHECK_SQLITE_RC(rc)
    }
    rc=sqlite3_backup_finish(dbbackup);
    if (rc!=SQLITE_OK)
    {
        GTL_LOG(3, "sqlite3_backup_finish failed: %d", rc);
        return rc;
    }

    rc=0;
    if (lCloseTargetAfterTransfert=='Y')
        rc=sqlite3_close(lTargetDB); // in order to be sure the file can be moved/deleted
    if (rc!=SQLITE_OK)
    {
        GTL_LOG(3, "sqlite3_close on sqlite file failed: %d", rc);
        return rc;
    }
    return rc;
}

int gtl_backup_sqlite_db(sqlite3 *lSourceDB, char *lTargetDBname, char lCloseTargetAfterTransfert)
{
    if (!lTargetDBname)
        return GTL_CORE_ERR_INVALID_PARAM_POINTER;
    if (!lSourceDB)   //sSQLiteDB)
        return GTL_CORE_ERR_INVALID_OUTPUT; // nothing to do ?

    GTL_LOG(5, "gtl_backup_sqlite_db into %s", lTargetDBname?lTargetDBname:"?");
    sqlite3 *dest = 0;
    int rc = sqlite3_open(lTargetDBname, &dest);
    //CHECK_SQLITE_RC(rc)
    if (rc!=0)
        GTL_LOG(4, "sqlite3_open failed: %d", rc)
    if (rc!=SQLITE_OK)
      return rc;

    return gtl_transfert_sqlite_db(lSourceDB, dest, lCloseTargetAfterTransfert);
}

// Try to retrieve the max retest index of previous splitlots. If none, lMaxRetestIndex set to -1
// Returns 0 on success
int gtl_GetMaxRetestIndex(int *lMaxRetestIndex)
{
    if (!lMaxRetestIndex)
        return GTL_CORE_ERR_INVALID_PARAM_POINTER;

    *lMaxRetestIndex = -1;

    sprintf(sQuery, "select max(retest_index) from ft_splitlot");

    sqlite3_stmt *stmt = NULL;

    int rc = sqlite3_prepare_v2( sSQLiteDB, sQuery, -1, &stmt, NULL );
    CHECK_SQLITE_RC(rc)
    if (rc!=0)
    {
        return GTL_CORE_ERR_CANT_GET_MAX_RETEST_INDEX;
    }

    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        if (sqlite3_column_type(stmt, 0) != SQLITE_INTEGER)
            return GTL_CORE_ERR_CANT_GET_MAX_RETEST_INDEX;

        *lMaxRetestIndex = (const int)sqlite3_column_int( stmt, 0 );
    }

    sqlite3_finalize( stmt );

    return 0; // 0 = ok
}

// Get the last splitlot id for the desired RetestIndex (if '%', then is will be the last splitlot id)
int gtl_GetLastSplitlotId(int *lSplitlotId, char lDesiredRetestIndex)
{
    if (!lSplitlotId)
        return GTL_CORE_ERR_INVALID_PARAM_POINTER;

    sprintf(sQuery, "select max(splitlot_id) from ft_splitlot where retest_index like '%c' ",
            lDesiredRetestIndex=='\0'?'%':lDesiredRetestIndex);

    sqlite3_stmt *stmt = NULL;

    int rc = sqlite3_prepare_v2( sSQLiteDB, sQuery, -1, &stmt, NULL );
    CHECK_SQLITE_RC_FATAL(rc)

    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        *lSplitlotId = (const int)sqlite3_column_int( stmt, 0 );
    }

    sqlite3_finalize( stmt );

    return 0; // 0 = ok
}

// Update nb_parts, nb_parts_good, finish_t,...
int gtl_SQLiteCloseSplitlot()
{
    GTL_LOG(5, "Closing current splitlot (%d)...", gtl_stProdInfo.mSplitlotID);
    // Let's update nb_parts
    sprintf(sQuery, "update ft_splitlot set nb_parts=%d where splitlot_id=%d", gtl_stGlobalInfo.uiPartIndex,
            gtl_stProdInfo.mSplitlotID);
    int lR=sqlite3_exec(sSQLiteDB, sQuery, 0,0, &zErrMsg);
    CHECK_SQLITE_RC_FATAL(lR);
    // Let's update nb parts good
    sprintf(sQuery, "update ft_splitlot set nb_parts_good=%d where splitlot_id=%d", gtl_stGlobalInfo.mNbPartsGood,
            gtl_stProdInfo.mSplitlotID);
    lR=sqlite3_exec(sSQLiteDB, sQuery, 0,0, &zErrMsg);
    CHECK_SQLITE_RC_FATAL(lR);

    // Let's update finish_t of that splitlot for customer to know time window of splitlot
    unsigned finish_t=0;
    struct timeval tv;
    struct timezone tz;
    lR=gettimeofday(&tv, &tz); //  the number of seconds and microseconds since the Epoch
    if (lR==0)
    {
        finish_t=tv.tv_sec;
    }
    else
        GTL_LOG(3, "Failed to retrieve time for finish_t", 0);

    sprintf(sQuery, "update ft_splitlot set finish_t=%d where splitlot_id=%d", finish_t, gtl_stProdInfo.mSplitlotID);
    int rc=sqlite3_exec(sSQLiteDB, sQuery, 0,0, &zErrMsg);
    CHECK_SQLITE_RC_FATAL(rc);

    // mDataFileName
    if (gtl_stGlobalInfo.mDataFileName)
    {
        sprintf(sQuery, "update ft_splitlot set file_name='%s' where splitlot_id=%d",
                gtl_stGlobalInfo.mDataFileName, gtl_stProdInfo.mSplitlotID);
        rc=sqlite3_exec(sSQLiteDB, sQuery, 0,0, &zErrMsg);
        CHECK_SQLITE_RC_FATAL(rc);
    }

    // GCORE-9143 valid_splitlot='Y'
    sprintf(sQuery,
            "update ft_splitlot set valid_splitlot='Y' where splitlot_id=%d",
             gtl_stProdInfo.mSplitlotID);
    rc=sqlite3_exec(sSQLiteDB, sQuery, 0,0, &zErrMsg);
    CHECK_SQLITE_RC_FATAL(rc);

    GTL_LOG(4, "Reseting current splitlot id to -1 till new splitlot request", 0);
    gtl_stProdInfo.mSplitlotID=-1;

    return rc;
}

// Update a string field in the current splitlot
#define UPDATE_SPLITLOT_STRINGFIELD(field_name, value) { sprintf(sQuery, \
    "update ft_splitlot set %s='%s' where splitlot_id=%d", field_name, value, gtl_stProdInfo.mSplitlotID); \
    rc = sqlite3_exec(sSQLiteDB, sQuery, 0, 0, &zErrMsg); \
    CHECK_SQLITE_RC(rc) }


// gtl_SQLiteNewSplitlot: will SET the ProdInfo mSplitlotID to the splitlot id returned by SQLite
int gtl_SQLiteNewSplitlot()
{
    GTL_LOG(5, "Insert a new splitlot: RetestIndex will be %d...", gtl_stProdInfo.mRetestIndex);

    // splitlot
    char day[10]="?", week_nb_str[3]="?", month_nb_str[3]="?";
    int week_nb=0, month_nb=0, quarter_nb=-1, year_nb=-1;
    time_t current_time= time(NULL); /* Obtain current time as seconds elapsed since the Epoch. */
    if (current_time == ((time_t)-1))
    {
        GTL_LOG(3, "Failure to compute the current time", 0);
    }
    else
    {
        struct tm *tmp=localtime(&current_time);
        int lR=strftime(day, 10, "%a", tmp); // Abbreviated weekday name
        if (lR==0)
            GTL_LOG(3, "Cannot retrieve day", 0);

        // %V=ISO 8601 week number (00-53) : not implemented in MSVCRT i.e. mingw
        lR=strftime(week_nb_str, 3, "%U", tmp); // %U=...
        if (lR==0)
            GTL_LOG(3, "Cannot retrieve week nb: '%s'", week_nb_str)
        else
            if (sscanf(week_nb_str, "%d", &week_nb)==EOF)
                GTL_LOG(3, "Cannot convert week nb from '%s'", week_nb_str);

        lR=strftime(month_nb_str, 3, "%m", tmp); // 3 = 2 + 1(\0)
        if (lR==0)
            GTL_LOG(3, "Cannot retrieve month nb: '%s'", month_nb_str)
        else
        {
            if (sscanf(month_nb_str, "%d", &month_nb)==EOF)
                GTL_LOG(3, "Cannot convert month nb from '%s'", month_nb_str)
            else
                quarter_nb=(month_nb/4)+1;
        }
        char year_nb_str[5]="";
        lR=strftime(year_nb_str, 5, "%Y", tmp);
        if (lR!=0)
        {
            sscanf(year_nb_str, "%d", &year_nb);
        }
    }

    GTL_LOG(7, "codeme : retrieve the real file id of the recipe", 0);

    // Let s clean some global info to be SQL ready specially the ' char
    // any other char to replace in order to please SQL ?
    char lLotID[GTC_MAX_LOTID_LEN]="";
    if (gtl_stProdInfo.szLotID)
    {
        sprintf(lLotID, gtl_stProdInfo.szLotID);
        ut_ReplaceChar(lLotID, '\'', ' ');
    }
    char lSublotID[GTC_MAX_SLOTID_LEN]="";
    if (gtl_stProdInfo.szSublotID)
    {
        sprintf(lSublotID, gtl_stProdInfo.szSublotID);
        ut_ReplaceChar(lSublotID, '\'', ' ');
    }
    char lNodeName[GTC_MAX_NODENAME_LEN]="";
    if (gtl_stStation.szNodeName)
    {
        sprintf(lNodeName, gtl_stStation.szNodeName);
        ut_ReplaceChar(lNodeName, '\'', ' ');
    }
    char lTesterType[GTC_MAX_TESTERTYPE_LEN]="";
    if (gtl_stStation.szTesterType)
    {
        sprintf(lTesterType, gtl_stStation.szTesterType);
        ut_ReplaceChar(lTesterType, '\'', ' ');
    }
    char lProductID[GTC_MAX_PRODUCTID_LEN]="";
    if (gtl_stProdInfo.szProductID)
    {
        sprintf(lProductID, gtl_stProdInfo.szProductID);
        ut_ReplaceChar(lProductID, '\'', ' ');
    }
    char lTesterExecType[GTC_MAX_TESTEREXEC_LEN]="";
    if (gtl_stStation.szTesterExecType)
    {
        sprintf(lTesterExecType, gtl_stStation.szTesterExecType);
        ut_ReplaceChar(lTesterExecType, '\'', ' ');
    }
    char lTesterExecVersion[GTC_MAX_TESTEREXEC_LEN]="";
    if (gtl_stStation.szTesterExecVersion)
    {
        sprintf(lTesterExecVersion, gtl_stStation.szTesterExecVersion);
        ut_ReplaceChar(lTesterExecVersion, '\'', ' ');
    }

    unsigned start_t=0; // num of sec since 1 Jan 1970
    struct timeval tv;
    struct timezone tz;
    int lR=gettimeofday(&tv, &tz); //  the number of seconds and microseconds since the Epoch
    if (lR==0)
    {
        start_t=tv.tv_sec;
    }
    else
        GTL_LOG(3, "Failed to get time of day: start_t will be 0.", 0);

    sprintf(sQuery, "insert into ft_splitlot ("
      " 'lot_id', 'sublot_id', 'start_t', 'stat_num', 'tester_name', 'tester_type', 'retest_index', 'retest_hbins', "
      " 'exec_typ', 'exec_ver', 'part_typ', 'nb_sites', "
      " 'day','week_nb','month_nb','quarter_nb','year_nb','year_and_week','year_and_month','year_and_quarter', 'recipe_id') "
            " VALUES('%s', '%s', %d, %d, '%s', '%s', %d, '%s', '%s', '%s', '%s', " // ..., auxfile
      " %d, '%s', %d,%d,%d,%d, " // ..., year_nb
      "'XXXX-XX', 'XXXX-XX', 'XXXX-XX', 1 )",
      lLotID, lSublotID, start_t, gtl_stStation.uiStationNb, lNodeName, lTesterType, gtl_stProdInfo.mRetestIndex,
      gtl_stProdInfo.mRetestHbins?gtl_stProdInfo.mRetestHbins:"?",
      lTesterExecType, lTesterExecVersion, lProductID,
      gtl_stStation.uiNbSites,
      day, week_nb, month_nb, quarter_nb, year_nb );

    int rc = sqlite3_exec(sSQLiteDB, sQuery, 0, 0, &zErrMsg);
    CHECK_SQLITE_RC_FATAL(rc)

    // The insert splitlot query has generated a new splitlot id, probably starting from 1.
    // Let's retrieve it for the GTL to know which is the current splitlot.
    rc=gtl_GetLastSplitlotId(&gtl_stProdInfo.mSplitlotID, '%');
    //CHECK_SQLITE_RC_FATAL(rc)
    if (rc!=0)
    {
        GTL_LOG(3, "GetLastSplitlotId failed: %d", rc)
        return rc;
    }

    // Now we created a new splitlot, add information for this splitlot into ft_gtl_info
    rc = gtl_UpdateGtlInfoTable(sSQLiteDB);
    if (rc!=0)
    {
        GTL_LOG(3, "UpdateGtlInfoTable failed: %d", rc)
        return GTL_CORE_ERR_UPDATE_DB;
    }

    if (gtl_stGlobalInfo.mAuxFileName)
        UPDATE_SPLITLOT_STRINGFIELD("aux_file", gtl_stGlobalInfo.mAuxFileName);
    if (gtl_stProdInfo.mFacilityID)
        UPDATE_SPLITLOT_STRINGFIELD("facil_id", gtl_stProdInfo.mFacilityID);
    if (gtl_stProdInfo.mFamilyID)
        UPDATE_SPLITLOT_STRINGFIELD("famly_id", gtl_stProdInfo.mFamilyID);
    if (gtl_stProdInfo.mFlowID)
        UPDATE_SPLITLOT_STRINGFIELD("flow_id", gtl_stProdInfo.mFlowID);
    if (gtl_stProdInfo.mFloorID)
        UPDATE_SPLITLOT_STRINGFIELD("floor_id", gtl_stProdInfo.mFloorID);
    if (gtl_stStation.szTestJobName)
        UPDATE_SPLITLOT_STRINGFIELD("job_nam", gtl_stStation.szTestJobName);
    if (gtl_stProdInfo.szJobRevision)
        UPDATE_SPLITLOT_STRINGFIELD("job_rev", gtl_stProdInfo.szJobRevision);
    if (gtl_stProdInfo.szOperatorName)
        UPDATE_SPLITLOT_STRINGFIELD("oper_nam", gtl_stProdInfo.szOperatorName);
    if (gtl_stProdInfo.mRomCode)
        UPDATE_SPLITLOT_STRINGFIELD("rom_cod", gtl_stProdInfo.mRomCode);
    if (gtl_stProdInfo.mSpecName)
        UPDATE_SPLITLOT_STRINGFIELD("spec_nam", gtl_stProdInfo.mSpecName);
    if (gtl_stProdInfo.mSpecVersion)
        UPDATE_SPLITLOT_STRINGFIELD("spec_ver", gtl_stProdInfo.mSpecVersion);
    if (gtl_stProdInfo.mSetupID)
        UPDATE_SPLITLOT_STRINGFIELD("setup_id", gtl_stProdInfo.mSetupID);
    if (gtl_stProdInfo.mTestCode)
        UPDATE_SPLITLOT_STRINGFIELD("test_cod", gtl_stProdInfo.mTestCode);
    if (gtl_stProdInfo.mTemperature)
        UPDATE_SPLITLOT_STRINGFIELD("tst_temp", gtl_stProdInfo.mTemperature);
    if (gtl_stProdInfo.mUserText)
        UPDATE_SPLITLOT_STRINGFIELD("user_txt", gtl_stProdInfo.mUserText);

    //
    sprintf(sQuery, "update ft_splitlot set mode_cod='%c' where splitlot_id=%d",
            gtl_stProdInfo.mModeCode, gtl_stProdInfo.mSplitlotID);
    rc = sqlite3_exec(sSQLiteDB, sQuery, 0, 0, &zErrMsg);
    CHECK_SQLITE_RC_FATAL(rc)
    //
    sprintf(sQuery, "update ft_splitlot set rtst_cod='%c' where splitlot_id=%d",
            gtl_stProdInfo.mRetestCode, gtl_stProdInfo.mSplitlotID);
    rc = sqlite3_exec(sSQLiteDB, sQuery, 0, 0, &zErrMsg);
    CHECK_SQLITE_RC_FATAL(rc)

    return rc;
}

int gtl_CreateSchema(sqlite3 *lDB)
{
    GTL_LOG(6, "gtl_CreateSchema...", 0);
    sprintf(sQuery, "create table logs ('date_time' DATETIME, severity integer, message varchar, function varchar, "
            "file varchar, line_no integer)");
    int rc = sqlite3_exec(lDB, sQuery, 0, 0, &zErrMsg);
    CHECK_SQLITE_RC_FATAL(rc)

    sprintf(sQuery, create_script);
    rc = sqlite3_exec(lDB, create_script, 0, 0, &zErrMsg );
    CHECK_SQLITE_RC_FATAL(rc)

    // todo : add these 2 views for easy debug:
    // SELECT * FROM ft_ptest_rollinglimits where ptest_info_id=0;
    // select * from logs where function like '%limit%'

    return rc;
}

int gtl_UpdateGtlInfoTable(sqlite3 *lDB)
{
    GTL_LOG(6, "gtl_UpdateGtlInfoTable...", 0);
    sprintf(sQuery, "insert into ft_gtl_info VALUES('%d', 'sqlite_version', '%s')",
            gtl_stProdInfo.mSplitlotID, SQLITE_VERSION);
    int rc = sqlite3_exec(lDB, sQuery, 0, 0, &zErrMsg);
    CHECK_SQLITE_RC_FATAL(rc)

    sprintf(sQuery, "insert into ft_gtl_info VALUES('%d', 'gtl_version_major', '%d')",
            gtl_stProdInfo.mSplitlotID, GTL_VERSION_MAJOR );
    rc = sqlite3_exec(lDB, sQuery, 0, 0, &zErrMsg);
    CHECK_SQLITE_RC_FATAL(rc)

    sprintf(sQuery, "insert into ft_gtl_info VALUES('%d', 'gtl_version_minor', '%d')",
            gtl_stProdInfo.mSplitlotID, GTL_VERSION_MINOR );
    rc = sqlite3_exec(lDB, sQuery, 0, 0, &zErrMsg);
    CHECK_SQLITE_RC_FATAL(rc)

    sprintf(sQuery, "insert into ft_gtl_info VALUES('%d', 'db_version_nb', '%d')",
            gtl_stProdInfo.mSplitlotID, GS_TDR_DB_VERSION_NB );
    rc = sqlite3_exec(lDB, sQuery, 0, 0, &zErrMsg);
    CHECK_SQLITE_RC_FATAL(rc)

    sprintf(sQuery, "insert into ft_gtl_info VALUES('%d', 'db_version_name', '%s')",
            gtl_stProdInfo.mSplitlotID, GS_TDR_DB_VERSION_NAME );
    rc = sqlite3_exec(lDB, sQuery, 0, 0, &zErrMsg);
    CHECK_SQLITE_RC_FATAL(rc)

    return rc;
}


struct KeyFieldList* keyFieldListInit()
{
    struct KeyFieldList* this = malloc(sizeof(struct KeyFieldList));
    this->type = KEY_FIELD_EMPTY;
    this->next = NULL;
    return this;
}

void keyFieldListDestroy(struct KeyFieldList* this)
{
    if (this->next != NULL)
    {
        keyFieldListDestroy(this->next);
    }
    free(this);
}

void keyFieldListAddStr(struct KeyFieldList* this, const char* s)
{
    if (this->type == KEY_FIELD_EMPTY)
    {
        this->type = KEY_FIELD_STR;
        this->str = s;
    }
    else if (this->next == NULL)
    {
        this->next = keyFieldListInit();
        keyFieldListAddStr(this->next, s);
    }
    else
    {
        keyFieldListAddStr(this->next, s);
    }
}

void keyFieldListAddNum(struct KeyFieldList* this, int n)
{
    if (this->type == KEY_FIELD_EMPTY)
    {
        this->type = KEY_FIELD_NUM;
        this->num = n;
    }
    else if (this->next == NULL)
    {
        this->next = keyFieldListInit();
        keyFieldListAddNum(this->next, n);
    }
    else
    {
        keyFieldListAddNum(this->next, n);
    }
}

/******************************************************************************!
 * \fn gtl_is_same_context
 * \brief check if same product, lot, ... and return 'y' or 'n' or '?' on error
 ******************************************************************************/
char gtl_is_same_context(sqlite3* lDB)
{
    char lValue[1024];
    char lVersionQuery[200];

    sprintf(lValue, "gtl_is_same_context() using fields %s", gtl_stGlobalInfo.mFieldsToMatch);
    GTL_LOG(5, lValue, 0);

    if (! lDB)
    {
        GTL_LOG(3, "Invalid sqlite db pointer", 0);
        return '?';
    }

    // there is a previous DB

    // 1) Check which table to use to get DB version: ft_gtl_info or global_settings
    strcpy(lVersionQuery, "select gtl_value from ft_gtl_info where splitlot_id=1 and gtl_key='db_version_nb'");
    strcpy(sQuery, "select * from sqlite_master where type='table' and name='ft_gtl_info'");
    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2( lDB, sQuery, -1, &stmt, NULL );
    CHECK_SQLITE_RC(rc)
    if (rc != 0)
    {
        GTL_LOG(3, "Error checking name of the table containing the DB version. Query returned %d", rc);
        sqlite3_finalize(stmt);
        return '?';
    }
    if (sqlite3_step(stmt) != SQLITE_ROW)
    {
        strcpy(lVersionQuery, "select value from global_settings where key='db_version_nb'");
        strcpy(sQuery, "select * from sqlite_master where type='table' and name='global_settings'");
        stmt = NULL;
        rc = sqlite3_prepare_v2( lDB, sQuery, -1, &stmt, NULL );
        CHECK_SQLITE_RC(rc)
        if (rc != 0)
        {
            GTL_LOG(3, "Error checking name of the table containing the DB version. Query returned %d", rc);
            sqlite3_finalize(stmt);
            return '?';
        }
        if (sqlite3_step(stmt) != SQLITE_ROW)
        {
            GTL_LOG(3, "Error checking name of the table containing the DB version. No table found.", 0);
            sqlite3_finalize(stmt);
            return '?';
        }
    }
    sqlite3_finalize( stmt );

    // 2) Now check it is the same DB version
    strcpy(sQuery, lVersionQuery);
    stmt = NULL;
    rc = sqlite3_prepare_v2( lDB, sQuery, -1, &stmt, NULL );
    CHECK_SQLITE_RC(rc)
    if (rc != 0)
    {
        GTL_LOG(3, "Query extracting DB version returned %d", rc);
        sqlite3_finalize(stmt);
        return '?';
    }
    int lDbVersion=-1;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        lDbVersion = (const int)sqlite3_column_int( stmt, 0 );
        if(lDbVersion != GS_TDR_DB_VERSION_NB)
            break;
    }
    sqlite3_finalize( stmt );
    if (lDbVersion == -1)
    {
        GTL_LOG(3, "Query extracting DB version returned no row", 0);
        sqlite3_finalize(stmt);
        return '?';
    }
    if(lDbVersion != GS_TDR_DB_VERSION_NB)
    {
        GTL_LOG(3, "Same context query: previous DB has incompatible version %d", lDbVersion);
        return 'n';
    }

    // 3) Let's compare all fields from the list of fields to match
    strcpy(sQuery, "select ");
    char lFieldsToMatch[strlen(gtl_stGlobalInfo.mFieldsToMatch) + 1];
    strcpy(lFieldsToMatch, gtl_stGlobalInfo.mFieldsToMatch);
    char* lField = strtok(lFieldsToMatch, ", ");
    struct KeyFieldList* lKeyFieldList = keyFieldListInit();
    unsigned int lKeyFieldCount = 0;
    while (lField != NULL) {
        if (UT_stricmp(lField, GTL_KEY_PRODUCT_ID) == 0)
        {
            strcat(sQuery, "part_typ,");
            keyFieldListAddStr(lKeyFieldList, gtl_stProdInfo.szProductID);
        }
        else if (UT_stricmp(lField, GTL_KEY_LOT_ID) == 0)
        {
            strcat(sQuery, "lot_id,");
            keyFieldListAddStr(lKeyFieldList, gtl_stProdInfo.szLotID);
        }
        else if (UT_stricmp(lField, GTL_KEY_SUBLOT_ID) == 0)
        {
            strcat(sQuery, "sublot_id,");
            keyFieldListAddStr(lKeyFieldList, gtl_stProdInfo.szSublotID);
        }
        else if (UT_stricmp(lField, GTL_KEY_TESTER_NAME) == 0)
        {
            strcat(sQuery, "tester_name,");
            keyFieldListAddStr(lKeyFieldList, gtl_stStation.szNodeName);
        }
        else if (UT_stricmp(lField, GTL_KEY_TESTER_TYPE) == 0)
        {
            strcat(sQuery, "tester_type,");
            keyFieldListAddStr(lKeyFieldList, gtl_stStation.szTesterType);
        }
        else if (UT_stricmp(lField, GTL_KEY_STATION_NUMBER) == 0)
        {
            strcat(sQuery, "stat_num,");
            keyFieldListAddNum(lKeyFieldList, gtl_stStation.uiStationNb);
        }
        else if (UT_stricmp(lField, GTL_KEY_RETEST_CODE) == 0)
        {
            strcat(sQuery, "retest_index,");
            keyFieldListAddNum(lKeyFieldList, gtl_stProdInfo.mRetestIndex);
        }
        else if (UT_stricmp(lField, GTL_KEY_RETEST_HBINS) == 0)
        {
            strcat(sQuery, "retest_hbins,");
            keyFieldListAddStr(lKeyFieldList, gtl_stProdInfo.mRetestHbins);
        }
        else if (UT_stricmp(lField, GTL_KEY_JOB_NAME) == 0)
        {
            strcat(sQuery, "job_nam,");
            keyFieldListAddStr(lKeyFieldList, gtl_stStation.szTestJobName);
        }
        else if (UT_stricmp(lField, GTL_KEY_JOB_REVISION) == 0)
        {
            strcat(sQuery, "job_rev,");
            keyFieldListAddStr(lKeyFieldList, gtl_stProdInfo.szJobRevision);
        }
        else if (UT_stricmp(lField, GTL_KEY_OPERATOR_NAME) == 0)
        {
            strcat(sQuery, "oper_nam,");
            keyFieldListAddStr(lKeyFieldList, gtl_stProdInfo.szOperatorName);
        }
        else if (UT_stricmp(lField, GTL_KEY_TEST_CODE) == 0)
        {
            strcat(sQuery, "test_cod,");
            keyFieldListAddStr(lKeyFieldList, gtl_stProdInfo.mTestCode);
        }
        else if (UT_stricmp(lField, GTL_KEY_FACILITY_ID) == 0)
        {
            strcat(sQuery, "facil_id,");
            keyFieldListAddStr(lKeyFieldList, gtl_stProdInfo.mFacilityID);
        }
        else if (UT_stricmp(lField, GTL_KEY_TEST_TEMPERATURE) == 0)
        {
            strcat(sQuery, "tst_temp,");
            keyFieldListAddStr(lKeyFieldList, gtl_stProdInfo.mTemperature);
        }
        else if (UT_stricmp(lField, GTL_KEY_USER_TXT) == 0)
        {
            strcat(sQuery, "user_txt,");
            keyFieldListAddStr(lKeyFieldList, gtl_stProdInfo.mUserText);
        }
        else if (UT_stricmp(lField, GTL_KEY_FAMILY_ID) == 0)
        {
            strcat(sQuery, "famly_id,");
            keyFieldListAddStr(lKeyFieldList, gtl_stProdInfo.mFamilyID);
        }
        else if (UT_stricmp(lField, GTL_KEY_SPEC_NAME) == 0)
        {
            strcat(sQuery, "spec_nam,");
            keyFieldListAddStr(lKeyFieldList, gtl_stProdInfo.mSpecName);
        }
        else if (UT_stricmp(lField, GTL_KEY_DATA_FILENAME) == 0)
        {
            strcat(sQuery, "file_name,");
            keyFieldListAddStr(lKeyFieldList, gtl_stGlobalInfo.mDataFileName);
        }
        else
        {
            GTL_LOG(3, "unknown key field %s", lField);
            keyFieldListDestroy(lKeyFieldList);
            return '?';
        }
        ++lKeyFieldCount;
        lField = strtok(NULL, ", ");
    }
    if (sQuery[strlen(sQuery) - 1] != ',')
    {
        GTL_LOG(3, "key field list is empty, GTL_KEY_FIELDS_TO_MATCH=%s",
                lFieldsToMatch);
        keyFieldListDestroy(lKeyFieldList);
        return '?';
    }
    sQuery[strlen(sQuery) - 1] = '\0';
    strcat(sQuery, " from ft_splitlot order by splitlot_id desc limit 1");
    GTL_LOG(5, "query = %s", sQuery);

    stmt = NULL;
    rc = sqlite3_prepare_v2(lDB, sQuery, -1, &stmt, NULL);
    CHECK_SQLITE_RC(rc)
    if (rc != 0)
    {
        GTL_LOG(3, "Same context query returned %d", rc);
        sqlite3_finalize(stmt);
        keyFieldListDestroy(lKeyFieldList);
        return '?';
    }
    if (sqlite3_step(stmt) != SQLITE_ROW)
    {
        GTL_LOG(3, "Same context query returned no row", 0);
        sqlite3_finalize(stmt);
        keyFieldListDestroy(lKeyFieldList);
        return '?';
    }

    struct KeyFieldList* lKeyFieldCurr = lKeyFieldList;
    unsigned int lCount = 0;
    while (lKeyFieldCurr != NULL)
    {
        if (lKeyFieldCurr->type == KEY_FIELD_STR)
        {
            if (sqlite3_column_type(stmt, lCount) != SQLITE_TEXT)
            {
                GTL_LOG(3, "%d is not a SQLITE_TEXT", lCount);
                sqlite3_finalize(stmt);
                keyFieldListDestroy(lKeyFieldList);
                return '?';
            }
            if (UT_stricmp(sqlite3_column_text(stmt, lCount),
                           lKeyFieldCurr->str) != 0)
            {
                sqlite3_finalize(stmt);
                keyFieldListDestroy(lKeyFieldList);
                return 'n';
            }
        }
        else if (lKeyFieldCurr->type == KEY_FIELD_NUM)
        {
            if (sqlite3_column_type(stmt, lCount) != SQLITE_INTEGER)
            {
                GTL_LOG(3, "%d is not a SQLITE_INTEGER", lCount);
                sqlite3_finalize(stmt);
                keyFieldListDestroy(lKeyFieldList);
                return '?';
            }
            if (sqlite3_column_int(stmt, lCount) != lKeyFieldCurr->num)
            {
                sqlite3_finalize(stmt);
                keyFieldListDestroy(lKeyFieldList);
                return 'n';
            }
        }
        else
        {
            GTL_LOG(3, "%d is not used", lCount);
            sqlite3_finalize(stmt);
            keyFieldListDestroy(lKeyFieldList);
            return '?';
        }
        lKeyFieldCurr = lKeyFieldCurr->next;
        ++lCount;
    }
    sqlite3_finalize(stmt);
    keyFieldListDestroy(lKeyFieldList);
    return 'y';
}

/******************************************************************************!
 * \fn gtl_InitSQLiteOutput
 ******************************************************************************/
int gtl_InitSQLiteOutput()
{
    GTL_LOG(6, "Init SQLite output...", 0);

    // 7501 : check that we dont have to reload the previous one in the temp folder

    char* lTempFolder=gtl_stGlobalInfo.mTempFolder;
    if (lTempFolder[0]=='\0')
      lTempFolder=gtl_GetGalaxySemiTempFolder();
    if (!lTempFolder || lTempFolder[0]=='\0')
        return GTL_CORE_ERR_NO_TEMP_FOLDER;
    char lPreviousDBpath[1024]="";
    sprintf(lPreviousDBpath, "%s%sgtl.sqlite", lTempFolder, lTempFolder[strlen(lTempFolder)-1]=='/'?"":"/");
    GTL_LOG(5, "Searching for previous data in temp folder: '%s'", lPreviousDBpath);

    int rc=0;

    if (ut_CheckFile(lPreviousDBpath)==1) // file exists
    {
        sqlite3 *lPreviousDB=0;
        rc=sqlite3_open(lPreviousDBpath, &lPreviousDB);
        // we have opened either an old DB OR created a brand new DB : who knows ?
        if (rc!=SQLITE_OK)
        {
           GTL_LOG(3, "Corrupted or invalid DB: %s", lPreviousDBpath);
           return GTL_CORE_ERR_CANNOT_RELOAD_PREVIOUS_DB;
        }

        char lSameContext=gtl_is_same_context(lPreviousDB);
        GTL_LOG(5, "Same context: %c", lSameContext);
        if (lSameContext == 'y')
        {
            // Resume or retest: does not matter, lets transfert the hdd DB into mem
            rc = sqlite3_open(":memory:", &sSQLiteDB);
            if (rc != SQLITE_OK)
            {
                GTL_LOG(3, "Cannot open in memory SQLite DB: %d", rc);
                return GTL_CORE_ERR_CANNOT_OPEN_IN_MEM_DB;
            }

            rc=gtl_transfert_sqlite_db(lPreviousDB, sSQLiteDB, 'N');
            if (rc!=0)
            {
                sqlite3_close(lPreviousDB);
                GTL_LOG(3, "Cannot transfert previous on disk DB to in-mem DB: %d", rc);
                return GTL_CORE_ERR_CANNOT_RELOAD_PREVIOUS_DB;
            }
            // To force limits reload at the end of gtl init, let's set desired limits if not already done
            if (strcmp(gtl_stGlobalInfo.mDesiredLimits, "")==0)
                sprintf(gtl_stGlobalInfo.mDesiredLimits, GTL_LIMITS_LAST);
        }
        else
        // consider the context is different and start from scratch (new sqlite, trigger baseline...)
        // - 'n': mismatching fields (GTL_KEY_FIELDS_TO_MATCH)
        // - '?': error in checking if same context (query error, syntax error in GTL_KEY_FIELDS_TO_MATCH...)
        {
            if (lSameContext == '?')
            {
                // Log a warning, but consider the context is different (do not return an error)
                gtl_msg_Warning(GTL_MSGWARN_SQLITE_CANT_CHECK_CONTEXT, "");
            }            // if retest requested, then there is an issue as there is no previous DB
            //if (strcmp(gtl_stGlobalInfo.mDesiredLimits, (char*)"")!=0)
            if (gtl_stProdInfo.mRetestIndex!=0) // retest desired
            {
                GTL_LOG(4,
                  "Impossible to initiate retest as this is a different product/lot/sublot/tester than previous data",
                        0);
                return GTL_CORE_ERR_CANNOT_LOAD_DESIRED_LIMITS;
            }
            // if resume then we will simply overwrite the sqlite at gtl close time
            sqlite3_close(lPreviousDB);
            goto start_from_scratch;
        }
        sqlite3_close(lPreviousDB);
    }
    else // no previous sqlite file found in temp folder...
    {
        //if (strcmp(gtl_stGlobalInfo.mDesiredLimits, "")!=0)
        if (gtl_stProdInfo.mRetestIndex==1) // retest desired
        {
            GTL_LOG(4,
              "Impossible to initiate retest for this product/lot/sublot/tester: previous data not found",
                    0);
            return GTL_CORE_ERR_CANNOT_LOAD_DESIRED_LIMITS;
        }
start_from_scratch:
        // No previous DB or different Lot ? no problem, let's consider it is a fresh start.
        // to do : read the option in order to know which kind of sqlite db to create : in-mem, file... ?
        rc = sqlite3_open(":memory:", &sSQLiteDB);
        //CHECK_SQLITE_RC_FATAL(rc)
        if (rc != SQLITE_OK)
        {
            GTL_LOG(3, "sqlite3_open(':memory:') failed: %d", rc)
            return GTL_CORE_ERR_CANNOT_OPEN_IN_MEM_DB;
        }
        rc = gtl_CreateSchema(sSQLiteDB);
        //CHECK_SQLITE_RC_FATAL(rc)
        if (rc!=0)
        {
            GTL_LOG(3, "Create schema failed: %d", rc);
            return GTL_CORE_ERR_CREATE_DB;
        }
    }

    // Retest index should be 0 if resume, 1 if retest
    //GTL_LOG(4, "Better detect if multi-session retest", 0);
    //if (strcmp(gtl_stGlobalInfo.mDesiredLimits, "")!=0)
    if (gtl_stProdInfo.mRetestIndex==1) // retest desired
    {
        // multi session retest: let's find the previous retest index
        GTL_LOG(4, "Check me : find the previous retest index of the previous splitlot", 0);
        int lMaxRetestIndex=-1;
        int lRC=gtl_GetMaxRetestIndex(&lMaxRetestIndex);
        if (lRC!=0 || lMaxRetestIndex<0)
        {
            GTL_LOG(3, "Failed to retrieve the max retest index of previous splitlots", 0);
            return lRC;
        }
        gtl_stProdInfo.mRetestIndex=++lMaxRetestIndex;
        GTL_LOG(5, "RetestIndex will be %d", gtl_stProdInfo.mRetestIndex);
    }

    // RetestIndex should be 0 for initial test or resume, > 0 if retest
    rc=gtl_SQLiteNewSplitlot();
    if (rc!=0)
    {
        GTL_LOG(3, "Insert new splitlot failed: %d", rc);
        return GTL_CORE_ERR_INSERT_NEW_SPLITLOT;
    }

    // Let's retrieve the splitlot id that has been inserted as it should be incremented at each new splitlot
    rc=gtl_GetLastSplitlotId(&gtl_stProdInfo.mSplitlotID, '%');
    GTL_LOG(5, "Current splitlot ID = %d", gtl_stProdInfo.mSplitlotID);

    GTL_LOG(5, "SQLite mem used = %ld", sqlite3_memory_used()); // should return a sqlite_int64 : usually long long int

    return rc;
}

// Insert a float param in a SQL query string : if infinity, write null else write the float value
// How to detect an invalid float ? GTL_INFINITE_VALUE_FLOAT ? GEXD PAT FLOAT INF ? FLT_MIN/MAX ? INFINITY ?
// Uses isinf() macro from C99
// what about isnan() and isfinite() ?????
// INFINITY seems to be only on windows
#define INSERT_FLOAT_IN_SQL_QUERY(myfloat) if \
    ((myfloat)==GTL_INFINITE_VALUE_FLOAT || isinf(myfloat) || ((myfloat==GTL_INVALID_VALUE_FLOAT)) ) \
        sprintf(sQuery, "%s, null", sQuery); \
    else \
        sprintf(sQuery, "%s, %g", sQuery, (myfloat) );

int gtl_SQLiteOutputTestDefs(PT_GNM_TESTDEF lTestDef, unsigned  lNumTests)
{
    if (!sSQLiteDB)
    {
        GTL_LOG(3, "Cannot output test defs to SQLite as DB null", 0);
        return GTL_CORE_ERR_INVALID_OUTPUT;
    }

    GTL_LOG(6, "Output %d TestDefs in SQLite db...",lNumTests);
    unsigned long i=0;
    PT_GNM_TESTDEF ptTestDefinition=0; // or PT_GNM_TESTDEF_DYNAMICPAT ?

    if (!lTestDef || lNumTests<1)
        return -1;

    char lPinIndexStr[1024]="";
    char lCleanedTestName[GTC_MAX_TESTNAME_LEN]="";

    for (i=0; i<lNumTests; i++)
    {
        ptTestDefinition = lTestDef+i;
        if (!ptTestDefinition)
            return -1;

        sprintf(lPinIndexStr, ", %d", ptTestDefinition->mPinIndex);

        GTL_LOG(6, "Test %ld pin %d...", ptTestDefinition->lTestNumber, ptTestDefinition->mPinIndex);

        if (ptTestDefinition->szTestName)
        {
            strncpy(lCleanedTestName, ptTestDefinition->szTestName, GTC_MAX_TESTNAME_LEN);
            ut_ReplaceChar(lCleanedTestName, '\'', ' ');
        }
        else
            GTL_LOG(3, "null TestName for test %ld", ptTestDefinition->lTestNumber);

        if (ptTestDefinition->szTestUnits)
            ut_ReplaceChar(ptTestDefinition->szTestUnits, '\'', ' ');
        else
            GTL_LOG(3, "null TestUnits for test %ld", ptTestDefinition->lTestNumber);

        if((ptTestDefinition->uiTestFlags & GTL_TFLAG_HAS_LL) && (ptTestDefinition->uiTestFlags & GTL_TFLAG_HAS_HL))
        {
            sprintf(sQuery,
              "insert into ft_%sptest_info('splitlot_id', '%sptest_info_id', 'tnum', 'tname', %s"
              "'units', 'll', 'hl', 'flags') "
              " VALUES(%d, %d, %ld, '%s' %s , '%s'",
              ptTestDefinition->mPinIndex==-1?"":"m",
              ptTestDefinition->mPinIndex==-1?"":"m",
              ptTestDefinition->mPinIndex==-1?"":" 'tpin_arrayindex', ",
              // Values
              gtl_stProdInfo.mSplitlotID, //1,  // 7501
              (int)i,
              ptTestDefinition->lTestNumber, lCleanedTestName,
              ptTestDefinition->mPinIndex==-1?"":lPinIndexStr,
              ptTestDefinition->szTestUnits);

            INSERT_FLOAT_IN_SQL_QUERY(ptTestDefinition->fLowLimit);
            INSERT_FLOAT_IN_SQL_QUERY(ptTestDefinition->fHighLimit);
            sprintf(sQuery, "%s, %d)", sQuery, ptTestDefinition->uiTestFlags );
        }
        else if(ptTestDefinition->uiTestFlags & GTL_TFLAG_HAS_LL)
        {
            sprintf(sQuery, "insert into ft_%sptest_info('splitlot_id', '%sptest_info_id', 'tnum', 'tname', %s"
              "'units', 'll', 'hl', 'flags') "
              " VALUES(%d, %d, %ld, '%s' %s, '%s'",
              ptTestDefinition->mPinIndex==-1?"":"m",
              ptTestDefinition->mPinIndex==-1?"":"m",
              ptTestDefinition->mPinIndex==-1?"":" 'tpin_arrayindex', ",
              // Values
              gtl_stProdInfo.mSplitlotID,  //1,
              (int)i,
              ptTestDefinition->lTestNumber, lCleanedTestName,
              ptTestDefinition->mPinIndex==-1?"":lPinIndexStr,
              ptTestDefinition->szTestUnits);

            INSERT_FLOAT_IN_SQL_QUERY(ptTestDefinition->fLowLimit);
            sprintf(sQuery, "%s, NULL, %d)", sQuery, ptTestDefinition->uiTestFlags );
        }
        else if(ptTestDefinition->uiTestFlags & GTL_TFLAG_HAS_HL)
        {
            sprintf(sQuery, "insert into ft_%sptest_info('splitlot_id', '%sptest_info_id', 'tnum', 'tname', %s"
                   "'units', 'll', 'hl', 'flags') "
                   " VALUES(%d, %d, %ld, '%s' %s, '%s', NULL",
                   ptTestDefinition->mPinIndex==-1?"":"m",
                   ptTestDefinition->mPinIndex==-1?"":"m",
                   ptTestDefinition->mPinIndex==-1?"":" 'tpin_arrayindex', ",
                   // values
                   gtl_stProdInfo.mSplitlotID, //1,
                   (int)i,
                   ptTestDefinition->lTestNumber, lCleanedTestName,
                   ptTestDefinition->mPinIndex==-1?"":lPinIndexStr,
                   ptTestDefinition->szTestUnits);

            INSERT_FLOAT_IN_SQL_QUERY(ptTestDefinition->fHighLimit);
            sprintf(sQuery, "%s, %d)", sQuery, ptTestDefinition->uiTestFlags ); // bug fix !!!
        }
        else
        {
            sprintf(sQuery, "insert into ft_%sptest_info('splitlot_id', '%sptest_info_id', 'tnum', 'tname', %s"
                "'units', 'll', 'hl', 'flags') "
                " VALUES(%d, %d, %ld, '%s' %s, '%s', NULL, NULL, %d )",
                ptTestDefinition->mPinIndex==-1?"":"m",
                ptTestDefinition->mPinIndex==-1?"":"m",
                ptTestDefinition->mPinIndex==-1?"":" 'tpin_arrayindex', ",
                // values
                gtl_stProdInfo.mSplitlotID, //1,
                (int)i,
                ptTestDefinition->lTestNumber, lCleanedTestName,
                ptTestDefinition->mPinIndex==-1?"":lPinIndexStr,
                ptTestDefinition->szTestUnits,
                ptTestDefinition->uiTestFlags);
        }

        int rc = sqlite3_exec(sSQLiteDB, sQuery, 0, 0, &zErrMsg);
        CHECK_SQLITE_RC_FATAL(rc)
    }
    return 0;
}

int gtl_SQLiteOutputRecipe(char* lFileName, char* lRecipe)
{
    if (!sSQLiteDB)
        return GTL_CORE_ERR_INVALID_OUTPUT;

    if (!lRecipe || !sSQLiteDB)
        return -1;

    // The recipe could contain some ' chars which fail the query. Lets replace them by ' '
    size_t lRecipeLength=strlen(lRecipe);
    char* lRecipeCleaned=(char*)malloc(lRecipeLength+1);
    GTL_LOG(5, "Recipe length : %d chars", lRecipeLength);
    memset(lRecipeCleaned, '\0', lRecipeLength+1);
    memcpy(lRecipeCleaned, lRecipe, lRecipeLength);
    ut_ReplaceChar(lRecipeCleaned, '\'', ' ');

    char* lQuery=(char*)malloc(lRecipeLength+5000); // let s plan space for filename, and sql commands..
    if (!lQuery)
        return GTL_CORE_ERR_MALLOC_FAILED;

    sprintf(lQuery, "insert into global_files('file_name', 'file_type', 'file_format', 'file_content', "
        "'file_checksum', 'file_last_update') "
        " VALUES('%s', 'Recipe', 'csv/xml (GalaxySemi recipe)', '%s', 0, 0 )",
        lFileName?lFileName:"?", lRecipeCleaned );
    free(lRecipeCleaned);
    int rc = sqlite3_exec(sSQLiteDB, lQuery, 0, 0, &zErrMsg);
    if (rc!=SQLITE_OK)
    {
        free(lQuery);
        GTL_LOG(3, "Recipe insertion query failed: %d", rc);
        return GTL_CORE_ERR_CANNOT_INSERT_RECIPE;
        //CHECK_SQLITE_RC_FATAL(rc)
        /*
            GTL_LOG(3, "SQLite error: %d : %s", rc, zErrMsg?zErrMsg:"?");
            GTL_LOG(3, lQuery, 0);
            free(lQuery);
            sqlite3_free(zErrMsg);
            gtl_backup_sqlite_db(sSQLiteDB, "gtl.sqlite", 'Y');
            sqlite3_close( sSQLiteDB );
            sSQLiteDB=0;
            return GTL_CORE_ERR_INVALID_OUTPUT; //-rc;
        */
    }
    free(lQuery);
    // as this is probably the first file to be inserted, the file_id should be 1
    return 0;
}

int gtl_SQLiteOutputNewOutlieredTest(TestItem* item, float lResult, char LimitType)
{
    if (!sSQLiteDB)
        return GTL_CORE_ERR_INVALID_OUTPUT;

    //GTL_LOG(6, (char*)"gtl_SQLiteOutputNewOutlieredTest", 0);
    if (!item)
        return GTL_CORE_ERR_INVALID_PARAM_POINTER;

    if (!item->mSite)
        return GTL_CORE_ERR_INVALID_SITE_PTR;

    if (item->mPinIndex<-1)
        return GTL_CORE_ERR_INVALID_PIN;

    sprintf(sQuery,
      "insert into %s('splitlot_id', '%sptest_info_id', 'run_id', 'run_index', 'limits_run_id', 'limit_type', 'value')"
      " VALUES(%d, %d, %d, 0, %d, '%c' ",
      item->mPinIndex==-1?"ft_ptest_outliers":"ft_mptest_outliers",
      item->mPinIndex==-1?"":"m",
      gtl_stProdInfo.mSplitlotID,
      item->uiTestIndex,
      // 7304 BG: the run_index is an index incremented in a same run if a test has multiple results for the same
      // part. This is not supported for now at FT, so force 0. The run_id should be unique over all site,
      // so use uiPartIndex
      //gtl_stGlobalInfo.uiRunIndex_Lot,
      gtl_stGlobalInfo.uiPartIndex,
      ((SiteItem*)item->mSite)->mLastLimitsPartIndex,
      LimitType
    );

    INSERT_FLOAT_IN_SQL_QUERY(lResult)
    sprintf(sQuery, "%s )", sQuery);

    int rc = sqlite3_exec(sSQLiteDB, sQuery, 0, 0, &zErrMsg);
    CHECK_SQLITE_RC_FATAL(rc)
    return 0;
}

// return 0 on success
int gtl_SQLiteOutputNewEvent(char* lEventType, char* lEventSubtype, char* lEventMessage)
{
    if (!sSQLiteDB)
        return GTL_CORE_ERR_INVALID_OUTPUT;

    char lCleanedEventMessage[GTC_MAX_MESSAGE_LEN]="";
    if (lEventMessage)
    {
        strcpy(lCleanedEventMessage, lEventMessage);
        ut_ReplaceChar(lCleanedEventMessage, '\'', ' ');
    }

    sprintf(sQuery,
      "insert into ft_event('splitlot_id','event_id','run_id','event_type','event_subtype','event_time_local',"
      " 'event_time_utc','event_message') "
      " VALUES(%d, %d, %d, '%s', '%s', %s, %s, '%s')",
      gtl_stProdInfo.mSplitlotID, gtl_stGlobalInfo.mEventIndex++, gtl_stGlobalInfo.uiPartIndex,
      lEventType?lEventType:"?", lEventSubtype?lEventSubtype:"?",
      /*
        Event_time :
        " SELECT (julianday('now') - 2440587.5)*86400.0",
        The C function should be faster than SQL interface.
        (long)current_time // on win32, it is a long...
        "strftime('%%s','now') ", // should return something as "1372196830"
      */
      " datetime('now', 'localtime')", "datetime('now')",
      lCleanedEventMessage );
    int rc = sqlite3_exec(sSQLiteDB, sQuery, 0, 0, &zErrMsg);
    CHECK_SQLITE_RC_FATAL(rc)
    return 0;
}

/*
int gtl_SQLiteOutputDynStats(SiteItem* lSite)
{
    if (!sSQLiteDB)
        return GTL_CORE_ERR_INVALID_OUTPUT;

    if (!lSite)
        return GTL_CORE_ERR_INVALID_PARAM_POINTER;

    GTL_LOG(3, "Code me ? or deprecated ?", 0);

    return 0;
}
*/

int gtl_SQLiteOutputNewRun(SiteItem *lSite, const char* lPartID, int lHBin, int lSBin, char lPartStatus)
{
    if (!sSQLiteDB)
        return GTL_CORE_ERR_INVALID_OUTPUT;

    if (!lSite)
        return GTL_CORE_ERR_INVALID_SITE_PTR;

    if (!lPartID)
        return GTL_CORE_ERR_INVALID_PARTID;

    char* lPartIDCleaned=(char*)malloc(strlen(lPartID)+1);
    memcpy(lPartIDCleaned, lPartID, strlen(lPartID)+1);
    ut_ReplaceChar(lPartIDCleaned, '\'', ' ');

    //time_t current_time = time(NULL); /* Obtain current time as seconds elapsed since the Epoch. */

    sprintf(sQuery, "insert into ft_run('splitlot_id', 'run_id', 'site_no', 'part_id', 'part_status', 'hbin_no', 'sbin_no', 'ttime') "
      " VALUES(%d, %d, %d, '%s', '%c', %d, %d, %ld)",
      gtl_stProdInfo.mSplitlotID,
      // PartIndex is the new counter incremented at each gtl_binning() or gtl_set_binning() call
      gtl_stGlobalInfo.uiPartIndex,
      lSite->mSiteNb,
      lPartIDCleaned,
      lPartStatus,
      lHBin, lSBin,
      (long)0 // ttime is the test time, time needed for this run, not the time stamp
    );
    free(lPartIDCleaned);
    int rc = sqlite3_exec(sSQLiteDB, sQuery, 0, 0, &zErrMsg);
    CHECK_SQLITE_RC(rc)
    return 0;
}

/*
  Output a new bin in bin table is not already:
    bin_type : h or s
    bin_cat : P or F
*/
int gtl_SQLiteOutputBin(char bin_type, int bin_no, char bin_cat, char* bin_family, char* bin_subfamily)
{
    if (!sSQLiteDB)
        return GTL_CORE_ERR_INVALID_OUTPUT;

    sprintf(sQuery, "insert into ft_%cbin('splitlot_id', '%cbin_no', '%cbin_cat', 'bin_family', 'bin_subfamily') "
      " VALUES(%d, %d, '%c', '%s', '%s')",
      bin_type, bin_type, bin_type,
      gtl_stProdInfo.mSplitlotID, bin_no, bin_cat, bin_family?bin_family:"", bin_subfamily?bin_subfamily:"" );
    int rc = sqlite3_exec(sSQLiteDB, sQuery, 0, 0, &zErrMsg);
    // Allow the SQLITE_CONSTRAINT because this bin could be already inserted
    if (rc!=SQLITE_CONSTRAINT && rc!=SQLITE_OK)
    {
        CHECK_SQLITE_RC(rc)
    }

    return 0;
}

// TestType: "" for PTR, "m" for MPR
#define INSERT_ROLLINGLIMIT(lTestType, lTestIndex, lLimitIndex, lLimitType, lLimitMode, LL, HL) if \
    ( \
        ( (LL!=-GTL_INFINITE_VALUE_FLOAT) && (!isinf(LL)) && (LL!=GTL_INVALID_VALUE_FLOAT)) \
        || \
        ( (HL!=GTL_INFINITE_VALUE_FLOAT) && (!isinf(HL)) && (HL!=GTL_INVALID_VALUE_FLOAT)) \
    ) \
    { \
        sprintf(sQuery, \
        "insert into ft_%sptest_rollinglimits(" \
        "'splitlot_id', '%sptest_info_id', 'run_id', 'limit_index', 'limit_type', 'limit_mode', 'LL', 'HL')" \
        " VALUES(%d, %d, %d, %d, '%c', %d", \
        lTestType, lTestType, \
        gtl_stProdInfo.mSplitlotID, lTestIndex, gtl_stGlobalInfo.uiPartIndex, \
        lLimitIndex, lLimitType, lLimitMode \
        ); \
        if ( (LL==-GTL_INFINITE_VALUE_FLOAT) || (isinf(LL)) || (LL==GTL_INVALID_VALUE_FLOAT)) \
            sprintf(sQuery,"%s, null", sQuery); \
        else sprintf(sQuery,"%s, %g", sQuery, LL); \
        if ( (HL==GTL_INFINITE_VALUE_FLOAT) || (isinf(HL)) || (HL==GTL_INVALID_VALUE_FLOAT)) \
            sprintf(sQuery,"%s, null)", sQuery); \
        else sprintf(sQuery,"%s, %g)", sQuery, HL); \
        int rc=sqlite3_exec(sSQLiteDB, sQuery, 0,0, &zErrMsg); \
        CHECK_SQLITE_RC(rc) \
    }


int gtl_SQLiteOutputNewLimits(SiteItem *lSite, int lOriginalBin, int lPATBin, const char* lPartID)
{
    if (!sSQLiteDB)
        return GTL_CORE_ERR_INVALID_OUTPUT;

    if (!lSite)
        return GTL_CORE_ERR_INVALID_PARAM_POINTER;

    GTL_LOG(7, "gtl_SQLiteOutputNewLimits original bin %d bin to assign %d part %s", lOriginalBin, lPATBin, lPartID);
    // 7304 BG: use new event type & subtype
    if(!strcmp(lSite->mEventType,"GTL_DPAT_LIMITS") || !strcmp(lSite->mEventType,"GTL_SPAT_LIMITS"))
    {
        // 7304 BG: determine limit type
        char lLimitType='?';
        if(!strcmp(lSite->mEventType, "GTL_DPAT_LIMITS"))
            lLimitType='D';
        else if(!strcmp(lSite->mEventType, "GTL_SPAT_LIMITS"))
            lLimitType='S';
        // we have to insert asap the new limits in order to know from which run/part the limits are applied
        //char lET[255]="";
        //sprintf(lET, "GTL_DPAT_LIMITS"); // It should be always 'D' no ?
        char lM[255]="";
        if (lLimitType=='D')
            sprintf(lM, "New DPAT limits received for site %d at Part %s", lSite->mSiteNb, lPartID);
        else
            sprintf(lM, "New SPAT limits received");
        //char lEST[255]="BASELINE";
        //if (lSite->mCurrentSiteState==GTL_SITESTATE_DPAT )
        //    sprintf(lEST, "TUNING");
        int lR=gtl_SQLiteOutputNewEvent(lSite->mEventType, lSite->mEventSubtype, lM);
        if (lR!=0)
            return lR;

        // insert limits and stats...
        unsigned lTestIndex=0;
        for (lTestIndex=0; lTestIndex<lSite->mNbTests; lTestIndex++)
        {
            TestItem *lTestItem=lSite->mTestList+lTestIndex;
            if (!lTestItem)
            {
                GTL_LOG(4, "Found a null TestItem in test list", 0);
                break;
            }

            if (lLimitType=='D')
            {
                INSERT_ROLLINGLIMIT(lTestItem->mPinIndex==-1?"":"m", lTestIndex, 0, lLimitType, 1,
                                    lTestItem->fLowLimit1_Dynamic, lTestItem->fHighLimit1_Dynamic)
            }
            else
            {
                INSERT_ROLLINGLIMIT(lTestItem->mPinIndex==-1?"":"m", lTestIndex, 0, lLimitType, 1,
                                    lTestItem->fLowLimit_Static, lTestItem->fHighLimit_Static)
            }

            // Check me : insert LL2&HL2 ?
            //GTL_LOG(4, "CHECK ME: insert LL2&HL2 in rollinglimits when needed",0);
            if (lLimitType=='D')
                if (lTestItem->fLowLimit1_Dynamic!=lTestItem->fLowLimit2_Dynamic ||
                    lTestItem->fHighLimit1_Dynamic!=lTestItem->fHighLimit2_Dynamic )
                {
                    INSERT_ROLLINGLIMIT(lTestItem->mPinIndex==-1?"":"m", lTestIndex, 1, lLimitType, 2,
                                        lTestItem->fLowLimit2_Dynamic, lTestItem->fHighLimit2_Dynamic)
                }

            sprintf(sQuery,
              "insert into ft_%sptest_rollingstats("
              " 'splitlot_id', '%sptest_info_id', 'run_id', 'distribution_shape','n_factor', 't_factor', "
              " 'mean','sigma','min','q1','median','q3','max', 'exec_count', 'fail_count')"
              " VALUES(%d,%d,%d,   '%s',%g,%g",
              lTestItem->mPinIndex==-1?"":"m",
              lTestItem->mPinIndex==-1?"":"m",
              gtl_stProdInfo.mSplitlotID, lTestIndex, gtl_stGlobalInfo.uiPartIndex,
              lTestItem->mLastStatsFromGtm.mDistributionShape,
              lTestItem->mLastStatsFromGtm.mN_Factor, lTestItem->mLastStatsFromGtm.mT_Factor
            );

            INSERT_FLOAT_IN_SQL_QUERY(lTestItem->mLastStatsFromGtm.mMean);
            INSERT_FLOAT_IN_SQL_QUERY(lTestItem->mLastStatsFromGtm.mSigma);
            INSERT_FLOAT_IN_SQL_QUERY(lTestItem->mLastStatsFromGtm.mMin)
            INSERT_FLOAT_IN_SQL_QUERY(lTestItem->mLastStatsFromGtm.mQ1);
            INSERT_FLOAT_IN_SQL_QUERY(lTestItem->mLastStatsFromGtm.mMedian);
            INSERT_FLOAT_IN_SQL_QUERY(lTestItem->mLastStatsFromGtm.mQ3);
            INSERT_FLOAT_IN_SQL_QUERY(lTestItem->mLastStatsFromGtm.mMax);

            sprintf(sQuery, "%s, %d, %d)", sQuery, lTestItem->uiNbExecs, lTestItem->uiFailCount);
            int rc=sqlite3_exec(sSQLiteDB, sQuery, 0,0, &zErrMsg);
            CHECK_SQLITE_RC(rc)
        }
        // 7304 BG: use new event type & subtype
        lSite->mEventType[0] = '\0';
        lSite->mEventSubtype[0] = '\0';
    }

    return GTL_CORE_ERR_OKAY;
}

int gtl_SQLiteOutputNewMessage(GTL_Message* lM)
{
    if (!sSQLiteDB)
        return GTL_CORE_ERR_INVALID_OUTPUT;

    if (!lM)
        return GTL_CORE_ERR_INVALID_PARAM_POINTER;
    GTL_LOG(6, "New message: %s", lM->mMessage?lM->mMessage:"?");
    char lEventType[1024]="";
    char lEventSubType[1024]="";

    if (lM->mMessageID!=-1)
    {
        sprintf(lEventType, "GTL_NOTIFICATION" );
        sprintf(lEventSubType, "%d", lM->mMessageID);
    }
    else
    {
        sprintf(lEventType, "GTM_NOTIFICATION" );
        sprintf(lEventSubType, "%d", lM->mSeverity);
    }

    return gtl_SQLiteOutputNewEvent(lEventType, lEventSubType, lM->mMessage);
}

int gtl_CloseSQLiteOutput()
{
    GTL_LOG(6, "gtl_CloseSQLiteOutput...", 0);
    // todo : if flush on close :

    if (!sSQLiteDB)
    {
        GTL_LOG(3, "SQLiteDB null", 0);
        return 0;
    }

    // should be done in PerformEnfOfSplitlot
    // could fail but anyway nice to have
    //gtl_SQLiteUpdateFinishTime();

    //
    char* lTempFolder=gtl_GetGalaxySemiTempFolder();
    char lP[1024]="";
    if (lTempFolder)
    {
        sprintf(lP, "%s/%s", lTempFolder, "gtl.sqlite");
        int lR=gtl_backup_sqlite_db(sSQLiteDB, lP, 'Y');
        if (lR!=0)
          GTL_LOG(3, "Failed to save the SQLite in the temp folder", 0);
    }
    else
        GTL_LOG(3, "Failed to retrieve the temp folder. Cannot save the backup copy.", 0);

    /* 7304 BG: anti-crash: make sure buffers are not containing empty strings */
    sprintf(lP, "%s/%s", (gtl_stGlobalInfo.mOutputFolder && gtl_stGlobalInfo.mOutputFolder[0]!='\0')?
                gtl_stGlobalInfo.mOutputFolder:".",
                (gtl_stGlobalInfo.mOutputFileName && gtl_stGlobalInfo.mOutputFileName[0]!='\0')?
                gtl_stGlobalInfo.mOutputFileName:"gtl.sqlite");
    int lR=gtl_backup_sqlite_db(sSQLiteDB, lP, 'Y');
    if (lR!=0)
        return lR;

    lR=sqlite3_close(sSQLiteDB);
    if (lR!=SQLITE_OK)
        GTL_LOG(3, "SQLite close failed : %d", lR);
    sSQLiteDB=0;

    return 0;
}

int gtl_SQLiteExecQuery(char* lQuery, char* lOutputFileName)
{
    GTL_LOG(5, "SQLiteExecQuery into '%s'", lOutputFileName?lOutputFileName:"?")

    if (!lQuery || !lOutputFileName)
    {
        if (!lQuery)
            GTL_LOG(3, "Query string null", 0);
        if (!lOutputFileName)
            GTL_LOG(3, "Query output filename null", 0);
        return -1;
    }

    sqlite3_stmt *stmt = NULL;

    FILE *lOutputFile=fopen(lOutputFileName, "w");
    if (!lOutputFile)
    {
        GTL_LOG(3, "Failed to open in write mode '%s'", lOutputFileName)
        return GTL_CORE_ERR_INVALID_OUTPUT_FILE;
    }

    //int rc = sqlite3_prepare_v2( sSQLiteDB, gtl_stGlobalInfo.mQueryBuffer, -1, &stmt, NULL );
    int rc = sqlite3_prepare_v2( sSQLiteDB, lQuery, -1, &stmt, NULL );
    //strcpy(sQuery, lQuery);
    //CHECK_SQLITE_RC(rc)
    if (rc!=SQLITE_OK)
    {
        GTL_LOG(3, "SQLite error: %d : %s", rc, zErrMsg?zErrMsg:"?");
        GTL_LOG(3, lQuery, 0, 0, 0);
        return rc;
    }

    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        int i=0;
        for (i=0; i<sqlite3_column_count(stmt); i++)
            fprintf(lOutputFile, "%s, ", sqlite3_column_text(stmt, i));
        fprintf(lOutputFile, "\n");
        //*lSplitlotId = (const int)sqlite3_column_int( stmt, 0 );
        //printf( "%s\n", data ? data : "[NULL]" );
    }

    fclose(lOutputFile);

    sqlite3_finalize( stmt );

    return 0;
}

//! \brief Try to load desired limits from desired splitlot id: replaces the current limits into the TestItems
//! \param Type= "tightest", "latest", "widest"
//! \param Splitlot Id : integer
//! \brief Will set the DPAT state to all the Site returned by the query
//! \param IgnoreUnfindableTests : if 'y', it will continue even if a test returned by the query is not found in the internal test list
//! \return Returns 0 on success
int gtl_SQLiteLoadLimits(char* lType, const int lSplitlotID, char lIgnoreUnfindableTests)
{
    GTL_LOG(5, "LoadLimitsFromSQLiteDB: '%s' from Splitlot %d...", lType?lType:"?", lSplitlotID);

    // latest
    if (strcmp(lType, GTL_LIMITS_LAST)==0)
    {
        sprintf(sQuery,
        " select maxrun.site_no as site_no, pt.tnum as test_num, pt.tname as test_name, -1 as tpinindex, " \
        " rl.limit_mode as limit_mode, rl.ll as ll, rl.hl as hl " \
        " from ft_ptest_rollinglimits rl inner join " \
        " ( " \
        "   select r.site_no, max(rl.run_id) as run_id" \
        "   from ft_ptest_rollinglimits rl left outer join ft_run r " \
        "   on r.splitlot_id=rl.splitlot_id and r.run_id=rl.run_id " \
        "   where rl.splitlot_id=%d and rl.limit_type='D' " \
        "   group by r.site_no " \
        " ) maxrun " \
        " on rl.run_id=maxrun.run_id " \
        " left outer join ft_ptest_info pt " \
        " on pt.splitlot_id=rl.splitlot_id and pt.ptest_info_id=rl.ptest_info_id " \
        " UNION " \
        " select maxrun.site_no, pt.tnum, pt.tname, pt.tpin_arrayindex as tpinindex, " \
        " rl.limit_mode, rl.ll, rl.hl " \
        " from ft_mptest_rollinglimits rl inner join " \
        " ( " \
        "    select r.site_no, max(rl.run_id) as run_id " \
        "    from ft_ptest_rollinglimits rl left outer join ft_run r " \
        "    on r.splitlot_id=rl.splitlot_id and r.run_id=rl.run_id " \
        "    where rl.splitlot_id=%d and rl.limit_type='D' " \
        "    group by r.site_no " \
        " ) maxrun " \
        " on rl.run_id=maxrun.run_id " \
        " left outer join ft_mptest_info pt " \
        " on pt.splitlot_id=rl.splitlot_id and pt.mptest_info_id=rl.mptest_info_id ",
        lSplitlotID, lSplitlotID);
    }
    else if (strcmp(lType, GTL_LIMITS_TIGHTEST)==0)
    {
        sprintf(sQuery,
          "select t.ptest_info_id as tid, t.tnum as test_num, t.tname as test_name, -1 as tpinindex, "
          " l.limit_mode as limit_mode, r.site_no as site_no, max(l.ll) as ll, min(l.hl) as hl"
          " from ft_ptest_rollinglimits l  "
          " left outer join ft_run r on r.splitlot_id=l.splitlot_id and r.run_id=l.run_id"
          " left outer join ft_ptest_info t on t.splitlot_id=l.splitlot_id and t.ptest_info_id=l.ptest_info_id"
          " where l.splitlot_id=%d and l.limit_type='D'"
          " group by tid, limit_mode, site_no"
          " UNION"
          " select t.mptest_info_id as tid, t.tnum as test_num, t.tname as test_name, t.tpin_arrayindex as tpinindex, l.limit_mode as limit_mode, r.site_no as site_no, max(l.ll) as ll, min(l.hl) as hl"
          " from ft_mptest_rollinglimits l  "
          " left outer join ft_run r on r.splitlot_id=l.splitlot_id and r.run_id=l.run_id"
          " left outer join ft_mptest_info t on t.splitlot_id=l.splitlot_id and t.mptest_info_id=l.mptest_info_id"
          " where l.splitlot_id=%d and l.limit_type='D'"
          " group by tid, limit_mode, site_no"
          " order by tid, limit_mode, site_no",
        lSplitlotID, lSplitlotID);
    }
    else if (strcmp(lType, GTL_LIMITS_WIDEST)==0)
    {
        sprintf(sQuery,
          "select t.ptest_info_id as tid, t.tnum as test_num, t.tname as test_name, -1 as tpinindex, "
          " l.limit_mode as limit_mode, r.site_no as site_no, min(l.ll) as ll, max(l.hl) as hl"
          " from ft_ptest_rollinglimits l  "
          " left outer join ft_run r on r.splitlot_id=l.splitlot_id and r.run_id=l.run_id"
          " left outer join ft_ptest_info t on t.splitlot_id=l.splitlot_id and t.ptest_info_id=l.ptest_info_id"
          " where l.splitlot_id=%d and l.limit_type='D'"
          " group by tid, limit_mode, site_no"
          " UNION"
          " select t.mptest_info_id as tid, t.tnum, t.tname, t.tpin_arrayindex as tpinindex, l.limit_mode, "
          "  r.site_no, min(l.ll) as ll, max(l.hl) as hl"
          " from ft_mptest_rollinglimits l  "
          " left outer join ft_run r on r.splitlot_id=l.splitlot_id and r.run_id=l.run_id"
          " left outer join ft_mptest_info t on t.splitlot_id=l.splitlot_id and t.mptest_info_id=l.mptest_info_id"
          " where l.splitlot_id=%d and l.limit_type='D'"
          " group by tid, limit_mode, site_no"
          " order by tid, limit_mode, site_no",
        lSplitlotID, lSplitlotID);
    }
    else
    {
        GTL_LOG(4, "Unsupported limits type %s", lType);
        return GTL_CORE_ERR_INVALID_LIMITS_TYPE;
    }

    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2( sSQLiteDB, sQuery, -1, &stmt, NULL );
    CHECK_SQLITE_RC_FATAL(rc)

    int lSiteNum=-1, lTestNum=-1, lPinIndex=-2, lLimitMode=-1;
    char* lTestName=0;
    double lLL=0, lHL=0;
    unsigned lNumLimits=0;
    // Table to know if we have updated (and send an event) the site state or not
    char mSitesStatesUpdated[1024];
    memset(mSitesStatesUpdated, 'N', 1024);

    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        lNumLimits++;
        int lColCount=sqlite3_column_count(stmt);
        if (lColCount<5)
        {
            GTL_LOG(3, "This row has only %d cols", lColCount);
            return GTL_CORE_ERR_CANNOT_LOAD_DESIRED_LIMITS;
        }
        lSiteNum=-1, lTestNum=-1, lPinIndex=-2, lLimitMode=-1;
        lTestName=0; lLL=0, lHL=0;

        int i=0;
        for (i=0; i<lColCount; i++)
        {
            const char* lCN=sqlite3_column_name(stmt, i);
            if (!lCN)
            {
                GTL_LOG(3, "Failed to retrieve col name for col %d", i);
                return GTL_CORE_ERR_CANNOT_LOAD_DESIRED_LIMITS;
            }
            //GTL_LOG(5, "Col name is %s", lCN);
            if (strcmp(lCN, "site_no")==0)
                lSiteNum=sqlite3_column_int(stmt, i);
            else if (strcmp(lCN, "test_num")==0)
                lTestNum=sqlite3_column_int(stmt, i);
            else if (strcmp(lCN, "test_name")==0)
                lTestName=sqlite3_column_text(stmt, i);
            else if (strcmp(lCN, "tpinindex")==0)
                lPinIndex=sqlite3_column_int(stmt, i);
            else if (strcmp(lCN, "limit_mode")==0)
                lLimitMode=sqlite3_column_int(stmt, i);
            else if (strcmp(lCN, "ll")==0)
                lLL=sqlite3_column_double(stmt, i);
            else if (strcmp(lCN, "hl")==0)
                lHL=sqlite3_column_double(stmt, i);
        } // for all cols

        if (lTestName==0 || lTestNum==-1 || lPinIndex==-2 || lLimitMode==-1 || lSiteNum==-1)
        {
            GTL_LOG(3, "Bad test info returned by query: test_no=%d pin=%d name='%s' LimitMode=%d",
                    lTestNum, lPinIndex, lTestName?lTestName:"?", lLimitMode);
            return GTL_CORE_ERR_CANNOT_LOAD_DESIRED_LIMITS;
        }

        GTL_LOG(5, "Load limits for site %d test %d '%s' pin %d : LL:%f HL:%f",
                lSiteNum, lTestNum, lTestName, lPinIndex, lLL, lHL);

        // let s search for a TestItem
        TestItem* lTI=gtl_tl_FindTestBySiteNb(lSiteNum, lTestNum, lTestName, lPinIndex, gtl_stGlobalInfo.uiFindTestKey,
                                              GTL_TL_FIND_REWINDATEND);
        if (lTI==0 && lIgnoreUnfindableTests=='n')
        {
            GTL_LOG(3, "Unknown test %d '%s' pin %d on site %d", lTestNum, lTestName, lPinIndex, lSiteNum);
            return GTL_CORE_ERR_INVALID_TEST;
        }

        if (lTI)
        {
            if (lLimitMode==1)
            {
                lTI->fLowLimit1_Dynamic=lLL;
                lTI->fHighLimit1_Dynamic=lHL;
                // Also set limits for mode 2 if not already set
                if(lTI->fLowLimit2_Dynamic == -GTL_INFINITE_VALUE_FLOAT)
                    lTI->fLowLimit2_Dynamic=lLL;
                if(lTI->fHighLimit2_Dynamic == GTL_INFINITE_VALUE_FLOAT)
                    lTI->fHighLimit2_Dynamic=lHL;
            }
            else if (lLimitMode==2)
            {
                lTI->fLowLimit2_Dynamic=lLL;
                lTI->fHighLimit2_Dynamic=lHL;
            }
            else
                GTL_LOG(3, "Illegal limit mode %d", lLimitMode);
        }

        // GCORE-590 : update site state anyway
        if (lSiteNum>=0 && lSiteNum<255)
        {
            SiteItem *lSite=gtl_tl_ptSiteArray + lSiteNum;
            if(lSite)
            {
                strcpy(lSite->mEventType, "GTL_DPAT_LIMITS");
                strcpy(lSite->mEventSubtype, gtl_stProdInfo.mRetestIndex>0?"RETEST":"RESUME");
                lSite->mCurrentSiteState = GTL_SITESTATE_DPAT;
            }
        }
    }

    GTL_LOG(5, "%d limits retrieved from previous splitlot %d", lNumLimits, lSplitlotID);

    if (lNumLimits==0 && gtl_stProdInfo.mRetestIndex>0)
    {
        GTL_LOG(4, "No previous limits found", 0);
        return GTL_CORE_ERR_CANNOT_LOAD_DESIRED_LIMITS;
    }
    else if (lNumLimits==0 && gtl_stProdInfo.mRetestIndex==0)
    // GCORE-590
    {
        // just a warning
        gtl_push_back_new_message(0, "No previous limits found/reloaded from previous splitlots", -1);
    }

    return 0;
}
