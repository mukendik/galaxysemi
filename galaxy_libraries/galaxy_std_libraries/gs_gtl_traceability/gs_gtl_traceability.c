/******************************************************************************!
 * \file gs_gtl_traceability.c
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sqlite3.h>
#include "gs_gtl_traceability.h"
#include "gs_buffer.h"
#include "gs_data.h"

#define SQLITE_CHECK_RC_OR_RETURN(c, r) \
    if (c != SQLITE_OK) \
    { \
        d->strerror = sqlite3_errmsg(d->sqlite); \
        d->error = sqlite3_errcode(d->sqlite) + GS_SQLITE_ERROR_BASE; \
        return r; \
    }
#define SQLITE_PREPARE_OR_RETURN(r) \
    if (sqlite3_prepare_v2(d->sqlite, gs_buffer_get(d->buffer), \
                           -1, &stmt, NULL) != SQLITE_OK) \
    { \
        d->strerror = sqlite3_errmsg(d->sqlite);  \
        d->error = sqlite3_errcode(d->sqlite) + GS_SQLITE_ERROR_BASE; \
        return r; \
    }

#define SQLITE_TABLE_EXISTS_OR_RETURN(t) \
    buff = gs_buffer_init(d->buffer); \
    gs_buffer_add(buff, \
                  "select count(*) from sqlite_master " \
                  "where type='table' AND name='%s'", t); \
    SQLITE_PREPARE_OR_RETURN(NULL) \
    if (sqlite3_step(stmt) == SQLITE_ROW && \
        sqlite3_column_int(stmt, 0) == 0) \
    { \
        buff = gs_buffer_init(d->buffer); \
        gs_buffer_add(buff, "{\"%s\":[]}", t); \
        sqlite3_finalize(stmt); \
        return gs_buffer_get(d->buffer); \
    }

/******************************************************************************!
 * \fn gs_gtl_traceability_new
 * \brief Create a descriptor
 * \return GsGtlTraceability descriptor
 ******************************************************************************/
struct GsGtlTraceability* gs_gtl_traceability_new()
{
    struct GsGtlTraceability* d = malloc(sizeof(struct GsGtlTraceability));
    if (d == NULL)
    {
        return NULL;
    }

    d->sqlite = NULL;
    d->buffer = NULL;
    d->error = 0;
    d->strerror = NULL;

    return d;
}

/******************************************************************************!
 * \fn gs_gtl_traceability_init
 * \brief Initialize a descriptor
 * \return GsGtlTraceability descriptor
 ******************************************************************************/
struct GsGtlTraceability* gs_gtl_traceability_init(struct GsGtlTraceability* d)
{
    if (d == NULL)
    {
        return NULL;
    }

    d->buffer = gs_buffer_new();
    if (d->buffer == NULL)
    {
        d->strerror = "gs_gtl_traceability_init: cannot allocate buffer";
        d->error = GS_GTL_TRACEABILITY_ERROR_BUFFNEW;
    }

    return d;
}

/******************************************************************************!
 * \fn gs_gtl_traceability_init_with_gsdata
 * \brief Initialize a descriptor with a GsData struct
 * \return GsGtlTraceability descriptor
 ******************************************************************************/
struct GsGtlTraceability*
gs_gtl_traceability_init_with_gsdata(struct GsGtlTraceability* d,
                                     struct GsData* lGsData)
{
    int r;

    if (d == NULL)
    {
        return NULL;
    }
    d->buffer = lGsData->buffer;
    d->sqlite = lGsData->sqlite;

    r = sqlite3_initialize();
    SQLITE_CHECK_RC_OR_RETURN(r, d)

    return d;
}

/******************************************************************************!
 * \fn gs_gtl_traceability_quit
 * \brief The oposite of init, do not forget to free the descriptor
 * \return void
 ******************************************************************************/
void gs_gtl_traceability_quit(struct GsGtlTraceability* d)
{
    if (d == NULL)
    {
        return;
    }

    if (d->buffer != NULL)
    {
        gs_buffer_quit(d->buffer);
        free(d->buffer);
        d->buffer = NULL;
    }
}

/******************************************************************************!
 * \fn gs_gtl_traceability_getLastError
 * \brief Get the last error code
 * \return Error code
 ******************************************************************************/
int gs_gtl_traceability_getLastError(struct GsGtlTraceability* d)
{
    if (d == NULL)
    {
        return GS_GTL_TRACEABILITY_ERROR_PTRNULL;
    }

    return d->error;
}

/******************************************************************************!
 * \fn gs_gtl_traceability_getLastStrError
 * \brief Get the last error decription
 * \return String
 ******************************************************************************/
const char* gs_gtl_traceability_getLastStrError(struct GsGtlTraceability* d)
{
    if (d == NULL)
    {
        return "gs_gtl_traceability null pointer";
    }

    if (d->strerror != NULL)
    {
        return d->strerror;
    }
    else
    {
        return "gs_gtl_traceability undefined error";
    }
}

/******************************************************************************!
 * \fn gs_gtl_traceability_loadData
 * \brief Load a SQlite file
 * \return void
 ******************************************************************************/
void gs_gtl_traceability_loadData(struct GsGtlTraceability* d, const char* path)
{
    struct stat buff;
    int r;

    if (d == NULL)
    {
        return;
    }

    if (stat(path, &buff) == -1)
    {
        d->strerror = "gs_gtl_traceability_loadData: cannot stat the file";
        d->error = GS_GTL_TRACEABILITY_ERROR_DATALOAD;
        return;
    }

    r = sqlite3_initialize();
    SQLITE_CHECK_RC_OR_RETURN(r, )
    r = sqlite3_open(path, &d->sqlite);
    SQLITE_CHECK_RC_OR_RETURN(r, )
}

/******************************************************************************!
 * \fn gs_gtl_traceability_unloadData
 * \brief Unload the SQlite file
 * \return void
 ******************************************************************************/
void gs_gtl_traceability_unloadData(struct GsGtlTraceability* d)
{
    int r;

    if (d == NULL)
    {
        return;
    }

    if (d->sqlite == NULL)
    {
        return;
    }

    r = sqlite3_close(d->sqlite);
    d->sqlite = NULL;
    SQLITE_CHECK_RC_OR_RETURN(r, )
    r = sqlite3_shutdown();
    SQLITE_CHECK_RC_OR_RETURN(r, )
}

/******************************************************************************!
 * \fn gs_gtl_traceability_getFtEvent
 * \return JSON string
 ******************************************************************************/
const char*
gs_gtl_traceability_getFtEvent(struct GsGtlTraceability* d, int splitlotId)
{
    sqlite3_stmt* stmt;
    int pass;
    struct GsBuffer* buff;

    if (d == NULL)
    {
        return "";
    }

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff,
                  "select event_id,run_id,event_type,event_subtype,"
                  "event_time_local,event_time_utc,event_message "
                  "from ft_event "
                  "where splitlot_id=%d", splitlotId);
    SQLITE_PREPARE_OR_RETURN(NULL)

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff, "{\"splitlot_id\":%d,", splitlotId);
    gs_buffer_add(buff, "\"ft_event\":[");
    pass = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        if (pass)
        {
            gs_buffer_add(buff, ",");
        }
        gs_buffer_add(buff, "{");
        gs_buffer_add(buff, "\"event_id\":%d,",
                      sqlite3_column_int(stmt, 0));
        gs_buffer_add(buff, "\"run_id\":%d,",
                      sqlite3_column_int(stmt, 1));
        gs_buffer_add(buff, "\"event_type\":\"%s\",",
                      sqlite3_column_text(stmt, 2));
        gs_buffer_add(buff, "\"event_subtype\":\"%s\",",
                      sqlite3_column_text(stmt, 3));
        gs_buffer_add(buff, "\"event_time_local\":\"%s\",",
                      sqlite3_column_text(stmt, 4));
        gs_buffer_add(buff, "\"event_time_utc\":\"%s\",",
                      sqlite3_column_text(stmt, 5));
        gs_buffer_add(buff, "\"event_message\":\"%s\"",
                      sqlite3_column_text(stmt, 6));
        gs_buffer_add(buff, "}");
        ++pass;
    }
    gs_buffer_add(buff, "]}");

    sqlite3_finalize(stmt);

    return gs_buffer_get(d->buffer);
}

/******************************************************************************!
 * \fn gs_gtl_traceability_getFtHBin
 * \return JSON string
 ******************************************************************************/
const char*
gs_gtl_traceability_getFtHBin(struct GsGtlTraceability* d, int splitlotId)
{
    sqlite3_stmt* stmt;
    int pass;
    struct GsBuffer* buff;

    if (d == NULL)
    {
        return "";
    }

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff,
                  "select hbin_no,hbin_name,hbin_cat,bin_family,bin_subfamily "
                  "from ft_hbin "
                  "where splitlot_id=%d", splitlotId);
    SQLITE_PREPARE_OR_RETURN(NULL)

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff, "{\"splitlot_id\":%d,", splitlotId);
    gs_buffer_add(buff, "\"ft_hbin\":[");
    pass = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        if (pass)
        {
            gs_buffer_add(buff, ",");
        }
        gs_buffer_add(buff, "{");
        gs_buffer_add(buff, "\"hbin_no\":%d,",
                      sqlite3_column_int(stmt, 0));
        gs_buffer_add(buff, "\"hbin_name\":\"%s\",",
                      sqlite3_column_text(stmt, 1));
        gs_buffer_add(buff, "\"hbin_cat\":\"%s\",",
                      sqlite3_column_text(stmt, 2));
        gs_buffer_add(buff, "\"bin_family\":\"%s\",",
                      sqlite3_column_text(stmt, 3));
        gs_buffer_add(buff, "\"bin_subfamily\":\"%s\"",
                      sqlite3_column_text(stmt, 4));
        gs_buffer_add(buff, "}");
        ++pass;
    }
    gs_buffer_add(buff, "]}");

    sqlite3_finalize(stmt);

    return gs_buffer_get(d->buffer);
}

/******************************************************************************!
 * \fn gs_gtl_traceability_getFtPtestInfo
 * \return JSON string
 ******************************************************************************/
const char*
gs_gtl_traceability_getFtPtestInfo(struct GsGtlTraceability* d,int splitlotId)
{
    sqlite3_stmt* stmt;
    int pass;
    struct GsBuffer* buff;

    if (d == NULL)
    {
        return "";
    }

    SQLITE_TABLE_EXISTS_OR_RETURN("ft_ptest_info")

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff,
                  "select ptest_info_id,tnum,tname,units,flags,ll,hl,testseq,"
                  "spec_ll,spec_hl,spec_target,res_scal,ll_scal,hl_scal "
                  "from ft_ptest_info "
                  "where splitlot_id=%d", splitlotId);
    SQLITE_PREPARE_OR_RETURN(NULL)

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff, "{\"splitlot_id\":%d,", splitlotId);
    gs_buffer_add(buff, "\"ft_ptest_info\":[");
    pass = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        if (pass)
        {
            gs_buffer_add(buff, ",");
        }
        gs_buffer_add(buff, "{");
        gs_buffer_add(buff, "\"ptest_info_id\":%d,",
                      sqlite3_column_int(stmt, 0));
        gs_buffer_add(buff, "\"tnum\":%d,",
                      sqlite3_column_int(stmt, 1));
        gs_buffer_add(buff, "\"tname\":\"%s\",",
                      sqlite3_column_text(stmt, 2));
        gs_buffer_add(buff, "\"units\":\"%s\",",
                      sqlite3_column_text(stmt, 3));
        gs_buffer_add(buff, "\"flags\":%d,",
                      sqlite3_column_int(stmt, 4));
        gs_buffer_add(buff, "\"ll\":%.15e,",
                      sqlite3_column_double(stmt, 5));
        gs_buffer_add(buff, "\"hl\":%.15e,",
                      sqlite3_column_double(stmt, 6));
        gs_buffer_add(buff, "\"testseq\":%d,",
                      sqlite3_column_int(stmt, 7));
        gs_buffer_add(buff, "\"spec_ll\":%.15e,",
                      sqlite3_column_double(stmt, 8));
        gs_buffer_add(buff, "\"spec_hl\":%.15e,",
                      sqlite3_column_double(stmt, 9));
        gs_buffer_add(buff, "\"spec_target\":%.15e,",
                      sqlite3_column_double(stmt, 10));
        gs_buffer_add(buff, "\"res_scal\":%d,",
                      sqlite3_column_int(stmt, 11));
        gs_buffer_add(buff, "\"ll_scal\":%d,",
                      sqlite3_column_int(stmt, 12));
        gs_buffer_add(buff, "\"hl_scal\":%d",
                      sqlite3_column_int(stmt, 13));
        gs_buffer_add(buff, "}");
        ++pass;
    }
    gs_buffer_add(buff, "]}");

    sqlite3_finalize(stmt);

    return gs_buffer_get(d->buffer);
}

/******************************************************************************!
 * \fn gs_gtl_traceability_getFtPtestOutliers
 * \return JSON string
 ******************************************************************************/
const char*
gs_gtl_traceability_getFtPtestOutliers(struct GsGtlTraceability* d,
                                       int splitlotId)
{
    sqlite3_stmt* stmt;
    int pass;
    struct GsBuffer* buff;

    if (d == NULL)
    {
        return "";
    }

    SQLITE_TABLE_EXISTS_OR_RETURN("ft_ptest_outliers")

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff,
                  "select ptest_info_id,run_id,run_index,"
                  "limits_run_id,limit_type,value "
                  "from ft_ptest_outliers "
                  "where splitlot_id=%d", splitlotId);
    SQLITE_PREPARE_OR_RETURN(NULL)

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff, "{\"splitlot_id\":%d,", splitlotId);
    gs_buffer_add(buff, "\"ft_ptest_outliers\":[");
    pass = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        if (pass)
        {
            gs_buffer_add(buff, ",");
        }
        gs_buffer_add(buff, "{");
        gs_buffer_add(buff, "\"ptest_info_id\":%d,",
                      sqlite3_column_int(stmt, 0));
        gs_buffer_add(buff, "\"run_id\":%d,",
                      sqlite3_column_int(stmt, 1));
        gs_buffer_add(buff, "\"run_index\":%d,",
                      sqlite3_column_int(stmt, 2));
        gs_buffer_add(buff, "\"limits_run_id\":%d,",
                      sqlite3_column_int(stmt, 3));
        gs_buffer_add(buff, "\"limit_type\":\"%s\",",
                      sqlite3_column_text(stmt, 4));
        gs_buffer_add(buff, "\"value\":\"%.15e\"",
                      sqlite3_column_double(stmt, 5));
        gs_buffer_add(buff, "}");
        ++pass;
    }
    gs_buffer_add(buff, "]}");

    sqlite3_finalize(stmt);

    return gs_buffer_get(d->buffer);
}

/******************************************************************************!
 * \fn gs_gtl_traceability_getFtPtestRollingLimits
 * \return JSON string
 ******************************************************************************/
const char*
gs_gtl_traceability_getFtPtestRollingLimits(struct GsGtlTraceability* d,
                                            int splitlotId)
{
    sqlite3_stmt* stmt;
    int pass;
    struct GsBuffer* buff;

    if (d == NULL)
    {
        return "";
    }

    SQLITE_TABLE_EXISTS_OR_RETURN("ft_ptest_rollinglimits")

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff,
                  "select ptest_info_id,run_id,"
                  "limit_index,limit_type,limit_mode,LL,HL "
                  "from ft_ptest_rollinglimits "
                  "where splitlot_id=%d", splitlotId);
    SQLITE_PREPARE_OR_RETURN(NULL)

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff, "{\"splitlot_id\":%d,", splitlotId);
    gs_buffer_add(buff, "\"ft_ptest_rollinglimits\":[");
    pass = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        if (pass)
        {
            gs_buffer_add(buff, ",");
        }
        gs_buffer_add(buff, "{");
        gs_buffer_add(buff, "\"ptest_info_id\":%d,",
                      sqlite3_column_int(stmt, 0));
        gs_buffer_add(buff, "\"run_id\":%d,",
                      sqlite3_column_int(stmt, 1));
        gs_buffer_add(buff, "\"limit_index\":%d,",
                      sqlite3_column_int(stmt, 2));
        gs_buffer_add(buff, "\"limit_type\":\"%s\",",
                      sqlite3_column_text(stmt, 3));
        gs_buffer_add(buff, "\"limit_mode\":%d,",
                      sqlite3_column_int(stmt, 4));
        gs_buffer_add(buff, "\"LL\":\"%.15e\",",
                      sqlite3_column_double(stmt, 5));
        gs_buffer_add(buff, "\"HL\":\"%.15e\"",
                      sqlite3_column_double(stmt, 6));
        gs_buffer_add(buff, "}");
        ++pass;
    }
    gs_buffer_add(buff, "]}");

    sqlite3_finalize(stmt);

    return gs_buffer_get(d->buffer);
}

/******************************************************************************!
 * \fn gs_gtl_traceability_getFtPtestRollingStats
 * \return JSON string
 ******************************************************************************/
const char*
gs_gtl_traceability_getFtPtestRollingStats(struct GsGtlTraceability* d,
                                           int splitlotId)
{
    sqlite3_stmt* stmt;
    int pass;
    struct GsBuffer* buff;

    if (d == NULL)
    {
        return "";
    }

    SQLITE_TABLE_EXISTS_OR_RETURN("ft_ptest_rollingstats")

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff,
                  "select ptest_info_id,run_id,"
                  "distribution_shape,n_factor,t_factor,"
                  "mean,sigma,min,q1,median,q3,max,exec_count,fail_count "
                  "from ft_ptest_rollingstats "
                  "where splitlot_id=%d", splitlotId);
    SQLITE_PREPARE_OR_RETURN(NULL)

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff, "{\"splitlot_id\":%d,", splitlotId);
    gs_buffer_add(buff, "\"ft_ptest_rollinglimits\":[");
    pass = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        if (pass)
        {
            gs_buffer_add(buff, ",");
        }
        gs_buffer_add(buff, "{");
        gs_buffer_add(buff, "\"ptest_info_id\":%d,",
                      sqlite3_column_int(stmt, 0));
        gs_buffer_add(buff, "\"run_id\":%d,",
                      sqlite3_column_int(stmt, 1));
        gs_buffer_add(buff, "\"distribution_shape\":\"%s\",",
                      sqlite3_column_text(stmt, 2));
        gs_buffer_add(buff, "\"n_factor\":\"%.15e\",",
                      sqlite3_column_double(stmt, 3));
        gs_buffer_add(buff, "\"t_factor\":\"%.15e\",",
                      sqlite3_column_double(stmt, 4));
        gs_buffer_add(buff, "\"mean\":\"%.15e\",",
                      sqlite3_column_double(stmt, 5));
        gs_buffer_add(buff, "\"sigma\":\"%.15e\",",
                      sqlite3_column_double(stmt, 6));
        gs_buffer_add(buff, "\"min\":\"%.15e\",",
                      sqlite3_column_double(stmt, 7));
        gs_buffer_add(buff, "\"q1\":\"%.15e\",",
                      sqlite3_column_double(stmt, 8));
        gs_buffer_add(buff, "\"median\":\"%.15e\",",
                      sqlite3_column_double(stmt, 9));
        gs_buffer_add(buff, "\"q3\":\"%.15e\",",
                      sqlite3_column_double(stmt, 10));
        gs_buffer_add(buff, "\"max\":\"%.15e\",",
                      sqlite3_column_double(stmt, 11));
        gs_buffer_add(buff, "\"exec_count\":%d,",
                      sqlite3_column_int(stmt, 12));
        gs_buffer_add(buff, "\"fail_count\":%d",
                      sqlite3_column_int(stmt, 13));
        gs_buffer_add(buff, "}");
        ++pass;
    }
    gs_buffer_add(buff, "]}");

    sqlite3_finalize(stmt);

    return gs_buffer_get(d->buffer);
}

/******************************************************************************!
 * \fn gs_gtl_traceability_getFtMPtestInfo
 * \return JSON string
 ******************************************************************************/
const char*
gs_gtl_traceability_getFtMPtestInfo(struct GsGtlTraceability* d,int splitlotId)
{
    sqlite3_stmt* stmt;
    int pass;
    struct GsBuffer* buff;

    if (d == NULL)
    {
        return "";
    }

    SQLITE_TABLE_EXISTS_OR_RETURN("ft_mptest_info")

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff,
                  "select mptest_info_id,tnum,tname,units,flags,ll,hl,testseq,"
                  "spec_ll,spec_hl,spec_target,res_scal,ll_scal,hl_scal "
                  "from ft_mptest_info "
                  "where splitlot_id=%d", splitlotId);
    SQLITE_PREPARE_OR_RETURN(NULL)

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff, "{\"splitlot_id\":%d,", splitlotId);
    gs_buffer_add(buff, "\"ft_mptest_info\":[");
    pass = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        if (pass)
        {
            gs_buffer_add(buff, ",");
        }
        gs_buffer_add(buff, "{");
        gs_buffer_add(buff, "\"mptest_info_id\":%d,",
                      sqlite3_column_int(stmt, 0));
        gs_buffer_add(buff, "\"tnum\":%d,",
                      sqlite3_column_int(stmt, 1));
        gs_buffer_add(buff, "\"tname\":\"%s\",",
                      sqlite3_column_text(stmt, 2));
        gs_buffer_add(buff, "\"units\":\"%s\",",
                      sqlite3_column_text(stmt, 3));
        gs_buffer_add(buff, "\"flags\":%d,",
                      sqlite3_column_int(stmt, 4));
        gs_buffer_add(buff, "\"ll\":%.15e,",
                      sqlite3_column_double(stmt, 5));
        gs_buffer_add(buff, "\"hl\":%.15e,",
                      sqlite3_column_double(stmt, 6));
        gs_buffer_add(buff, "\"testseq\":%d,",
                      sqlite3_column_int(stmt, 7));
        gs_buffer_add(buff, "\"spec_ll\":%.15e,",
                      sqlite3_column_double(stmt, 8));
        gs_buffer_add(buff, "\"spec_hl\":%.15e,",
                      sqlite3_column_double(stmt, 9));
        gs_buffer_add(buff, "\"spec_target\":%.15e,",
                      sqlite3_column_double(stmt, 10));
        gs_buffer_add(buff, "\"res_scal\":%d,",
                      sqlite3_column_int(stmt, 11));
        gs_buffer_add(buff, "\"ll_scal\":%d,",
                      sqlite3_column_int(stmt, 12));
        gs_buffer_add(buff, "\"hl_scal\":%d",
                      sqlite3_column_int(stmt, 13));
        gs_buffer_add(buff, "}");
        ++pass;
    }
    gs_buffer_add(buff, "]}");

    sqlite3_finalize(stmt);

    return gs_buffer_get(d->buffer);
}

/******************************************************************************!
 * \fn gs_gtl_traceability_getFtMPtestOutliers
 * \return JSON string
 ******************************************************************************/
const char*
gs_gtl_traceability_getFtMPtestOutliers(struct GsGtlTraceability* d,
                                        int splitlotId)
{
    sqlite3_stmt* stmt;
    int pass;
    struct GsBuffer* buff;

    if (d == NULL)
    {
        return "";
    }

    SQLITE_TABLE_EXISTS_OR_RETURN("ft_mptest_outliers")

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff,
                  "select mptest_info_id,run_id,run_index,"
                  "limits_run_id,limit_type,value "
                  "from ft_mptest_outliers "
                  "where splitlot_id=%d", splitlotId);
    SQLITE_PREPARE_OR_RETURN(NULL)

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff, "{\"splitlot_id\":%d,", splitlotId);
    gs_buffer_add(buff, "\"ft_mptest_outliers\":[");
    pass = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        if (pass)
        {
            gs_buffer_add(buff, ",");
        }
        gs_buffer_add(buff, "{");
        gs_buffer_add(buff, "\"mptest_info_id\":%d,",
                      sqlite3_column_int(stmt, 0));
        gs_buffer_add(buff, "\"run_id\":%d,",
                      sqlite3_column_int(stmt, 1));
        gs_buffer_add(buff, "\"run_index\":%d,",
                      sqlite3_column_int(stmt, 2));
        gs_buffer_add(buff, "\"limits_run_id\":%d,",
                      sqlite3_column_int(stmt, 3));
        gs_buffer_add(buff, "\"limit_type\":\"%s\",",
                      sqlite3_column_text(stmt, 4));
        gs_buffer_add(buff, "\"value\":\"%.15e\"",
                      sqlite3_column_double(stmt, 5));
        gs_buffer_add(buff, "}");
        ++pass;
    }
    gs_buffer_add(buff, "]}");

    sqlite3_finalize(stmt);

    return gs_buffer_get(d->buffer);
}

/******************************************************************************!
 * \fn gs_gtl_traceability_getFtMPtestRollingLimits
 * \return JSON string
 ******************************************************************************/
const char*
gs_gtl_traceability_getFtMPtestRollingLimits(struct GsGtlTraceability* d,
                                             int splitlotId)
{
    sqlite3_stmt* stmt;
    int pass;
    struct GsBuffer* buff;

    if (d == NULL)
    {
        return "";
    }

    SQLITE_TABLE_EXISTS_OR_RETURN("ft_mptest_rollinglimits")

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff,
                  "select mptest_info_id,run_id,"
                  "limit_index,limit_type,limit_mode,LL,HL "
                  "from ft_mptest_rollinglimits "
                  "where splitlot_id=%d", splitlotId);
    SQLITE_PREPARE_OR_RETURN(NULL)

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff, "{\"splitlot_id\":%d,", splitlotId);
    gs_buffer_add(buff, "\"ft_mptest_rollinglimits\":[");
    pass = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        if (pass)
        {
            gs_buffer_add(buff, ",");
        }
        gs_buffer_add(buff, "{");
        gs_buffer_add(buff, "\"mptest_info_id\":%d,",
                      sqlite3_column_int(stmt, 0));
        gs_buffer_add(buff, "\"run_id\":%d,",
                      sqlite3_column_int(stmt, 1));
        gs_buffer_add(buff, "\"limit_index\":%d,",
                      sqlite3_column_int(stmt, 2));
        gs_buffer_add(buff, "\"limit_type\":\"%s\",",
                      sqlite3_column_text(stmt, 3));
        gs_buffer_add(buff, "\"limit_mode\":%d,",
                      sqlite3_column_int(stmt, 4));
        gs_buffer_add(buff, "\"LL\":\"%.15e\",",
                      sqlite3_column_double(stmt, 5));
        gs_buffer_add(buff, "\"HL\":\"%.15e\"",
                      sqlite3_column_double(stmt, 6));
        gs_buffer_add(buff, "}");
        ++pass;
    }
    gs_buffer_add(buff, "]}");

    sqlite3_finalize(stmt);

    return gs_buffer_get(d->buffer);
}

/******************************************************************************!
 * \fn gs_gtl_traceability_getFtMPtestRollingStats
 * \return JSON string
 ******************************************************************************/
const char*
gs_gtl_traceability_getFtMPtestRollingStats(struct GsGtlTraceability* d,
                                            int splitlotId)
{
    sqlite3_stmt* stmt;
    int pass;
    struct GsBuffer* buff;

    if (d == NULL)
    {
        return "";
    }

    SQLITE_TABLE_EXISTS_OR_RETURN("ft_mptest_rollingstats")

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff,
                  "select mptest_info_id,run_id,"
                  "distribution_shape,n_factor,t_factor,"
                  "mean,sigma,min,q1,median,q3,max,exec_count,fail_count "
                  "from ft_mptest_rollingstats "
                  "where splitlot_id=%d", splitlotId);
    SQLITE_PREPARE_OR_RETURN(NULL)

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff, "{\"splitlot_id\":%d,", splitlotId);
    gs_buffer_add(buff, "\"ft_mptest_rollinglimits\":[");
    pass = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        if (pass)
        {
            gs_buffer_add(buff, ",");
        }
        gs_buffer_add(buff, "{");
        gs_buffer_add(buff, "\"mptest_info_id\":%d,",
                      sqlite3_column_int(stmt, 0));
        gs_buffer_add(buff, "\"run_id\":%d,",
                      sqlite3_column_int(stmt, 1));
        gs_buffer_add(buff, "\"distribution_shape\":\"%s\",",
                      sqlite3_column_text(stmt, 2));
        gs_buffer_add(buff, "\"n_factor\":\"%.15e\",",
                      sqlite3_column_double(stmt, 3));
        gs_buffer_add(buff, "\"t_factor\":\"%.15e\",",
                      sqlite3_column_double(stmt, 4));
        gs_buffer_add(buff, "\"mean\":\"%.15e\",",
                      sqlite3_column_double(stmt, 5));
        gs_buffer_add(buff, "\"sigma\":\"%.15e\",",
                      sqlite3_column_double(stmt, 6));
        gs_buffer_add(buff, "\"min\":\"%.15e\",",
                      sqlite3_column_double(stmt, 7));
        gs_buffer_add(buff, "\"q1\":\"%.15e\",",
                      sqlite3_column_double(stmt, 8));
        gs_buffer_add(buff, "\"median\":\"%.15e\",",
                      sqlite3_column_double(stmt, 9));
        gs_buffer_add(buff, "\"q3\":\"%.15e\",",
                      sqlite3_column_double(stmt, 10));
        gs_buffer_add(buff, "\"max\":\"%.15e\",",
                      sqlite3_column_double(stmt, 11));
        gs_buffer_add(buff, "\"exec_count\":%d,",
                      sqlite3_column_int(stmt, 12));
        gs_buffer_add(buff, "\"fail_count\":%d",
                      sqlite3_column_int(stmt, 13));
        gs_buffer_add(buff, "}");
        ++pass;
    }
    gs_buffer_add(buff, "]}");

    sqlite3_finalize(stmt);

    return gs_buffer_get(d->buffer);
}

/******************************************************************************!
 * \fn gs_gtl_traceability_getFtRun
 * \return JSON string
 ******************************************************************************/
const char*
gs_gtl_traceability_getFtRun(struct GsGtlTraceability* d, int splitlotId)
{
    sqlite3_stmt* stmt;
    int pass;
    struct GsBuffer* buff;

    if (d == NULL)
    {
        return "";
    }

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff,
                  "select run_id,site_no,part_id,part_x,part_y,hbin_no,"
                  "sbin_no,ttime,tests_executed,tests_failed,firstfail_tnum,"
                  "firstfail_tname,retest_index,wafer_id,part_txt "
                  "from ft_run "
                  "where splitlot_id=%d", splitlotId);
    SQLITE_PREPARE_OR_RETURN(NULL)

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff, "{\"splitlot_id\":%d,", splitlotId);
    gs_buffer_add(buff, "\"ft_run\":[");
    pass = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        if (pass)
        {
            gs_buffer_add(buff, ",");
        }
        gs_buffer_add(buff, "{");
        gs_buffer_add(buff, "\"run_id\":%d,",
                      sqlite3_column_int(stmt, 0));
        gs_buffer_add(buff, "\"site_no\":%d,",
                      sqlite3_column_int(stmt, 1));
        gs_buffer_add(buff, "\"part_id\":\"%s\",",
                      sqlite3_column_text(stmt, 2));
        gs_buffer_add(buff, "\"part_x\":%d,",
                      sqlite3_column_int(stmt, 3));
        gs_buffer_add(buff, "\"part_y\":%d,",
                      sqlite3_column_int(stmt, 4));
        gs_buffer_add(buff, "\"hbin_no\":%d,",
                      sqlite3_column_int(stmt, 5));
        gs_buffer_add(buff, "\"sbin_no\":%d,",
                      sqlite3_column_int(stmt, 6));
        gs_buffer_add(buff, "\"ttime\":%d,",
                      sqlite3_column_int(stmt, 7));
        gs_buffer_add(buff, "\"tests_executed\":%d,",
                      sqlite3_column_int(stmt, 8));
        gs_buffer_add(buff, "\"tests_failed\":%d,",
                      sqlite3_column_int(stmt, 9));
        gs_buffer_add(buff, "\"firstfail_tnum\":%d,",
                      sqlite3_column_int(stmt, 10));
        gs_buffer_add(buff, "\"firstfail_tname\":\"%s\",",
                      sqlite3_column_text(stmt, 11));
        gs_buffer_add(buff, "\"retest_index\":%d,",
                      sqlite3_column_int(stmt, 12));
        gs_buffer_add(buff, "\"wafer_id\":\"%s\",",
                      sqlite3_column_text(stmt, 13));
        gs_buffer_add(buff, "\"part_txt\":\"%s\"",
                      sqlite3_column_text(stmt, 14));
        gs_buffer_add(buff, "}");
        ++pass;
    }
    gs_buffer_add(buff, "]}");

    sqlite3_finalize(stmt);

    return gs_buffer_get(d->buffer);
}

/******************************************************************************!
 * \fn gs_gtl_traceability_getFtSBin
 * \return JSON string
 ******************************************************************************/
const char*
gs_gtl_traceability_getFtSBin(struct GsGtlTraceability* d, int splitlotId)
{
    sqlite3_stmt* stmt;
    int pass;
    struct GsBuffer* buff;

    if (d == NULL)
    {
        return "";
    }

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff,
                  "select sbin_no,sbin_name,sbin_cat,bin_family,bin_subfamily "
                  "from ft_sbin "
                  "where splitlot_id=%d", splitlotId);
    SQLITE_PREPARE_OR_RETURN(NULL)

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff, "{\"splitlot_id\":%d,", splitlotId);
    gs_buffer_add(buff, "\"ft_sbin\":[");
    pass = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        if (pass)
        {
            gs_buffer_add(buff, ",");
        }
        gs_buffer_add(buff, "{");
        gs_buffer_add(buff, "\"sbin_no\":%d,",
                      sqlite3_column_int(stmt, 0));
        gs_buffer_add(buff, "\"sbin_name\":\"%s\",",
                      sqlite3_column_text(stmt, 1));
        gs_buffer_add(buff, "\"sbin_cat\":\"%s\",",
                      sqlite3_column_text(stmt, 2));
        gs_buffer_add(buff, "\"bin_family\":\"%s\",",
                      sqlite3_column_text(stmt, 3));
        gs_buffer_add(buff, "\"bin_subfamily\":\"%s\"",
                      sqlite3_column_text(stmt, 4));
        gs_buffer_add(buff, "}");
        ++pass;
    }
    gs_buffer_add(buff, "]}");

    sqlite3_finalize(stmt);

    return gs_buffer_get(d->buffer);
}

/******************************************************************************!
 * \fn gs_gtl_traceability_getFtSplitlot
 * \return JSON string
 ******************************************************************************/
const char*
gs_gtl_traceability_getFtSplitlot(struct GsGtlTraceability* d, int splitlotId)
{
    sqlite3_stmt* stmt;
    int pass;
    struct GsBuffer* buff;

    if (d == NULL)
    {
        return "";
    }

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff,
                  "select lot_id,sublot_id,setup_t,start_t,finish_t,stat_num,"
                  "tester_name,tester_type,flags,nb_parts,nb_parts_good,"
                  "nb_parts_samples,nb_parts_samples_good,nb_parts_summary,"
                  "nb_parts_summary_good,data_provider,data_type,prod_data,"
                  "retest_index,retest_hbins,rework_code,job_nam,job_rev,"
                  "oper_nam,exec_typ,exec_ver,test_cod,facil_id,tst_temp,"
                  "mode_cod,rtst_cod,prot_cod,burn_tim,cmod_cod,part_typ,"
                  "user_txt,aux_file,pkg_typ,famly_id,date_cod,floor_id,"
                  "proc_id,oper_frq,spec_nam,spec_ver,flow_id,setup_id,"
                  "dsgn_rev,eng_id,rom_cod,serl_num,supr_nam,nb_sites,"
                  "head_num,handler_typ,handler_id,card_typ,card_id,"
                  "loadboard_typ,loadboard_id,dib_typ,dib_id,cable_typ,"
                  "cable_id,contactor_typ,contactor_id,laser_typ,laser_id,"
                  "extra_typ,extra_id,file_host_id,file_path,file_name,"
                  "valid_splitlot,insertion_time,subcon_lot_id,"
                  "incremental_update,sya_id,day,week_nb,month_nb,quarter_nb,"
                  "year_nb,year_and_week,year_and_month,year_and_quarter,"
                  "recipe_id "
                  "from ft_splitlot "
                  "where splitlot_id=%d", splitlotId);
    SQLITE_PREPARE_OR_RETURN(NULL)

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff, "{\"splitlot_id\":%d,", splitlotId);
    gs_buffer_add(buff, "\"ft_splitlot\":[");
    pass = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        if (pass)
        {
            d->strerror =
                "gs_gtl_traceability_getFtSplitlot: only one row expected";
            d->error = GS_GTL_TRACEABILITY_ERROR_DATALOAD;
            sqlite3_finalize(stmt);
            return "";
        }
        gs_buffer_add(buff, "{");
        gs_buffer_add(buff, "\"lot_id\":\"%s\",",
                      sqlite3_column_text(stmt, 0));
        gs_buffer_add(buff, "\"sublot_id\":\"%s\",",
                      sqlite3_column_text(stmt, 1));
        gs_buffer_add(buff, "\"setup_t\":%d,",
                      sqlite3_column_int(stmt, 2));
        gs_buffer_add(buff, "\"start_t\":%d,",
                      sqlite3_column_int(stmt, 3));
        gs_buffer_add(buff, "\"finish_t\":%d,",
                      sqlite3_column_int(stmt, 4));
        gs_buffer_add(buff, "\"stat_num\":%d,",
                      sqlite3_column_int(stmt, 5));
        gs_buffer_add(buff, "\"tester_name\":\"%s\",",
                      sqlite3_column_text(stmt, 6));
        gs_buffer_add(buff, "\"tester_type\":\"%s\",",
                      sqlite3_column_text(stmt, 7));
        gs_buffer_add(buff, "\"flags\":%d,",
                      sqlite3_column_int(stmt, 8));
        gs_buffer_add(buff, "\"nb_parts\":%d,",
                      sqlite3_column_int(stmt, 9));
        gs_buffer_add(buff, "\"nb_parts_good\":%d,",
                      sqlite3_column_int(stmt, 10));
        gs_buffer_add(buff, "\"nb_parts_samples\":%d,",
                      sqlite3_column_int(stmt, 11));
        gs_buffer_add(buff, "\"nb_parts_samples_good\":%d,",
                      sqlite3_column_int(stmt, 12));
        gs_buffer_add(buff, "\"nb_parts_summary\":%d,",
                      sqlite3_column_int(stmt, 13));
        gs_buffer_add(buff, "\"nb_parts_summary_good\":%d,",
                      sqlite3_column_int(stmt, 14));
        gs_buffer_add(buff, "\"data_provider\":\"%s\",",
                      sqlite3_column_text(stmt, 15));
        gs_buffer_add(buff, "\"data_type\":\"%s\",",
                      sqlite3_column_text(stmt, 16));
        gs_buffer_add(buff, "\"prod_data\":\"%s\",",
                      sqlite3_column_text(stmt, 17));
        gs_buffer_add(buff, "\"retest_index\":%d,",
                      sqlite3_column_int(stmt, 18));
        gs_buffer_add(buff, "\"retest_hbins\":\"%s\",",
                      sqlite3_column_text(stmt, 19));
        gs_buffer_add(buff, "\"rework_code\":%d,",
                      sqlite3_column_int(stmt, 20));
        gs_buffer_add(buff, "\"job_nam\":\"%s\",",
                      sqlite3_column_text(stmt, 21));
        gs_buffer_add(buff, "\"job_rev\":\"%s\",",
                      sqlite3_column_text(stmt, 22));
        gs_buffer_add(buff, "\"oper_nam\":\"%s\",",
                      sqlite3_column_text(stmt, 23));
        gs_buffer_add(buff, "\"exec_typ\":\"%s\",",
                      sqlite3_column_text(stmt, 24));
        gs_buffer_add(buff, "\"exec_ver\":\"%s\",",
                      sqlite3_column_text(stmt, 25));
        gs_buffer_add(buff, "\"test_cod\":\"%s\",",
                      sqlite3_column_text(stmt, 26));
        gs_buffer_add(buff, "\"facil_id\":\"%s\",",
                      sqlite3_column_text(stmt, 27));
        gs_buffer_add(buff, "\"tst_temp\":\"%s\",",
                      sqlite3_column_text(stmt, 28));
        gs_buffer_add(buff, "\"mode_cod\":\"%s\",",
                      sqlite3_column_text(stmt, 29));
        gs_buffer_add(buff, "\"rtst_cod\":\"%s\",",
                      sqlite3_column_text(stmt, 30));
        gs_buffer_add(buff, "\"prot_cod\":\"%s\",",
                      sqlite3_column_text(stmt, 31));
        gs_buffer_add(buff, "\"burn_tim\":%d,",
                      sqlite3_column_int(stmt, 32));
        gs_buffer_add(buff, "\"cmod_cod\":\"%s\",",
                      sqlite3_column_text(stmt, 33));
        gs_buffer_add(buff, "\"part_typ\":\"%s\",",
                      sqlite3_column_text(stmt, 34));
        gs_buffer_add(buff, "\"user_txt\":\"%s\",",
                      sqlite3_column_text(stmt, 35));
        gs_buffer_add(buff, "\"aux_file\":\"%s\",",
                      sqlite3_column_text(stmt, 36));
        gs_buffer_add(buff, "\"pkg_typ\":\"%s\",",
                      sqlite3_column_text(stmt, 37));
        gs_buffer_add(buff, "\"famly_id\":\"%s\",",
                      sqlite3_column_text(stmt, 38));
        gs_buffer_add(buff, "\"date_cod\":\"%s\",",
                      sqlite3_column_text(stmt, 39));
        gs_buffer_add(buff, "\"floor_id\":\"%s\",",
                      sqlite3_column_text(stmt, 40));
        gs_buffer_add(buff, "\"proc_id\":\"%s\",",
                      sqlite3_column_text(stmt, 41));
        gs_buffer_add(buff, "\"oper_frq\":\"%s\",",
                      sqlite3_column_text(stmt, 42));
        gs_buffer_add(buff, "\"spec_nam\":\"%s\",",
                      sqlite3_column_text(stmt, 43));
        gs_buffer_add(buff, "\"spec_ver\":\"%s\",",
                      sqlite3_column_text(stmt, 44));
        gs_buffer_add(buff, "\"flow_id\":\"%s\",",
                      sqlite3_column_text(stmt, 45));
        gs_buffer_add(buff, "\"setup_id\":\"%s\",",
                      sqlite3_column_text(stmt, 46));
        gs_buffer_add(buff, "\"dsgn_rev\":\"%s\",",
                      sqlite3_column_text(stmt, 47));
        gs_buffer_add(buff, "\"eng_id\":\"%s\",",
                      sqlite3_column_text(stmt, 48));
        gs_buffer_add(buff, "\"rom_cod\":\"%s\",",
                      sqlite3_column_text(stmt, 49));
        gs_buffer_add(buff, "\"serl_num\":\"%s\",",
                      sqlite3_column_text(stmt, 50));
        gs_buffer_add(buff, "\"supr_nam\":\"%s\",",
                      sqlite3_column_text(stmt, 51));
        gs_buffer_add(buff, "\"nb_sites\":%d,",
                      sqlite3_column_int(stmt, 52));
        gs_buffer_add(buff, "\"head_num\":%d,",
                      sqlite3_column_int(stmt, 53));
        gs_buffer_add(buff, "\"handler_typ\":\"%s\",",
                      sqlite3_column_text(stmt, 54));
        gs_buffer_add(buff, "\"handler_id\":\"%s\",",
                      sqlite3_column_text(stmt, 55));
        gs_buffer_add(buff, "\"card_typ\":\"%s\",",
                      sqlite3_column_text(stmt, 56));
        gs_buffer_add(buff, "\"card_id\":\"%s\",",
                      sqlite3_column_text(stmt, 57));
        gs_buffer_add(buff, "\"loadboard_typ\":\"%s\",",
                      sqlite3_column_text(stmt, 58));
        gs_buffer_add(buff, "\"loadboard_id\":\"%s\",",
                      sqlite3_column_text(stmt, 59));
        gs_buffer_add(buff, "\"dib_typ\":\"%s\",",
                      sqlite3_column_text(stmt, 60));
        gs_buffer_add(buff, "\"dib_id\":\"%s\",",
                      sqlite3_column_text(stmt, 61));
        gs_buffer_add(buff, "\"cable_typ\":\"%s\",",
                      sqlite3_column_text(stmt, 62));
        gs_buffer_add(buff, "\"cable_id\":\"%s\",",
                      sqlite3_column_text(stmt, 63));
        gs_buffer_add(buff, "\"contactor_typ\":\"%s\",",
                      sqlite3_column_text(stmt, 64));
        gs_buffer_add(buff, "\"contactor_id\":\"%s\",",
                      sqlite3_column_text(stmt, 65));
        gs_buffer_add(buff, "\"laser_typ\":\"%s\",",
                      sqlite3_column_text(stmt, 66));
        gs_buffer_add(buff, "\"laser_id\":\"%s\",",
                      sqlite3_column_text(stmt, 67));
        gs_buffer_add(buff, "\"extra_typ\":\"%s\",",
                      sqlite3_column_text(stmt, 68));
        gs_buffer_add(buff, "\"extra_id\":\"%s\",",
                      sqlite3_column_text(stmt, 69));
        gs_buffer_add(buff, "\"file_host_id\":%d,",
                      sqlite3_column_int(stmt, 70));
        gs_buffer_add(buff, "\"file_path\":\"%s\",",
                      sqlite3_column_text(stmt, 71));
        gs_buffer_add(buff, "\"file_name\":\"%s\",",
                      sqlite3_column_text(stmt, 72));
        gs_buffer_add(buff, "\"valid_splitlot\":\"%s\",",
                      sqlite3_column_text(stmt, 73));
        gs_buffer_add(buff, "\"insertion_time\":%d,",
                      sqlite3_column_int(stmt, 74));
        gs_buffer_add(buff, "\"subcon_lot_id\":\"%s\",",
                      sqlite3_column_text(stmt, 75));
        gs_buffer_add(buff, "\"incremental_update\":\"%s\",",
                      sqlite3_column_text(stmt, 76));
        gs_buffer_add(buff, "\"sya_id\":%d,",
                      sqlite3_column_int(stmt, 77));
        gs_buffer_add(buff, "\"day\":\"%s\",",
                      sqlite3_column_text(stmt, 78));
        gs_buffer_add(buff, "\"week_nb\":%d,",
                      sqlite3_column_int(stmt, 79));
        gs_buffer_add(buff, "\"month_nb\":%d,",
                      sqlite3_column_int(stmt, 80));
        gs_buffer_add(buff, "\"quarter_nb\":%d,",
                      sqlite3_column_int(stmt, 81));
        gs_buffer_add(buff, "\"year_nb\":%d,",
                      sqlite3_column_int(stmt, 82));
        gs_buffer_add(buff, "\"year_and_week\":\"%s\",",
                      sqlite3_column_text(stmt, 83));
        gs_buffer_add(buff, "\"year_and_month\":\"%s\",",
                      sqlite3_column_text(stmt, 84));
        gs_buffer_add(buff, "\"year_and_quarter\":\"%s\",",
                      sqlite3_column_text(stmt, 85));
        gs_buffer_add(buff, "\"recipe_id\":%d",
                      sqlite3_column_int(stmt, 86));
        gs_buffer_add(buff, "}");
        ++pass;
    }
    gs_buffer_add(buff, "]}");

    sqlite3_finalize(stmt);

    return gs_buffer_get(d->buffer);
}

/******************************************************************************!
 * \fn gs_gtl_traceability_getGlobalFileInfo
 * \return JSON string
 ******************************************************************************/
const char*
gs_gtl_traceability_getGlobalFileInfos(struct GsGtlTraceability* d,
                                      int splitlotId)
{
    sqlite3_stmt* stmt;
    struct GsBuffer* buff;
    int recipe;

    if (d == NULL)
    {
        return "";
    }

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff,
                  "select recipe_id from ft_splitlot "
                  "where splitlot_id=%d", splitlotId);
    SQLITE_PREPARE_OR_RETURN(NULL)
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        recipe = sqlite3_column_int(stmt, 0);
    }
    else
    {
        sqlite3_finalize(stmt);
        return "";
    }

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff,
                  "select file_id,file_name,file_format,file_last_update "
                  "from global_files "
                  "where file_id=%d", recipe);
    SQLITE_PREPARE_OR_RETURN(NULL)

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff, "{\"global_file\":[{");
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        gs_buffer_add(buff, "\"file_id\":%d,",
                      sqlite3_column_int(stmt, 0));
        gs_buffer_add(buff, "\"file_name\":\"%s\",",
                      sqlite3_column_text(stmt, 1));
        gs_buffer_add(buff, "\"file_format\":\"%s\",",
                      sqlite3_column_text(stmt, 2));
        gs_buffer_add(buff, "\"file_last_update\":\"%s\"",
                      sqlite3_column_text(stmt, 3));
    }
    gs_buffer_add(buff, "}]}");

    sqlite3_finalize(stmt);

    return gs_buffer_get(d->buffer);
}

/******************************************************************************!
 * \fn gs_gtl_traceability_getFtGtlInfo
 * \return JSON string
 ******************************************************************************/
const char*
gs_gtl_traceability_getFtGtlInfo(struct GsGtlTraceability* d,
                                 int lSplitlotId)
{
    sqlite3_stmt* stmt;
    int pass;
    struct GsBuffer* buff;

    if (d == NULL)
    {
        return "";
    }

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff,
                  "select count(*) from sqlite_master " \
                  "where type='table' AND name='ft_gtl_info'");
    SQLITE_PREPARE_OR_RETURN(NULL)
    buff = gs_buffer_init(d->buffer);
    if (sqlite3_step(stmt) == SQLITE_ROW &&
        sqlite3_column_int(stmt, 0) == 0)
    {
        gs_buffer_add(buff, "select key,value from global_settings");
    }
    else
    {
        gs_buffer_add(buff,
                      "select gtl_key,gtl_value from ft_gtl_info "
                      "where splitlot_id=%d", lSplitlotId);
    }
    SQLITE_PREPARE_OR_RETURN(NULL)

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff, "{\"splitlot_id\":%d,", lSplitlotId);
    gs_buffer_add(buff, "\"ft_gtl_info\":[");
    pass = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        if (pass)
        {
            gs_buffer_add(buff, ",");
        }
        gs_buffer_add(buff, "{");
        gs_buffer_add(buff, "\"gtl_key\":\"%s\",",
                      sqlite3_column_text(stmt, 0));
        gs_buffer_add(buff, "\"gtl_value\":\"%s\"",
                      sqlite3_column_text(stmt, 1));
        gs_buffer_add(buff, "}");
        ++pass;
    }
    gs_buffer_add(buff, "]}");

    sqlite3_finalize(stmt);

    return gs_buffer_get(d->buffer);
}

/******************************************************************************!
 * \fn gs_gtl_traceability_getGlobalFileContent
 * \return JSON string
 ******************************************************************************/
const char*
gs_gtl_traceability_getGlobalFileContent(struct GsGtlTraceability* d,
                                         int fileId)
{
    sqlite3_stmt* stmt;
    struct GsBuffer* buff;

    if (d == NULL)
    {
        return "";
    }

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff,
                  "select file_content from global_files "
                  "where file_id=%d", fileId);
    SQLITE_PREPARE_OR_RETURN(NULL)

    buff = gs_buffer_init(d->buffer);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        gs_buffer_add(buff, "%s", sqlite3_column_blob(stmt, 0));
    }
    else
    {
        sqlite3_finalize(stmt);
        return "";
    }

    sqlite3_finalize(stmt);

    return gs_buffer_get(d->buffer);
}
