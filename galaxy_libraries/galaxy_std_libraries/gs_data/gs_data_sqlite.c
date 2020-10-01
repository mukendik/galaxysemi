/******************************************************************************!
 * \file gs_data_sqlite.c
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include "gs_data.h"
#include "gs_buffer.h"

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
        d->strerror = sqlite3_errmsg(d->sqlite); \
        d->error = sqlite3_errcode(d->sqlite) + GS_SQLITE_ERROR_BASE; \
        return r; \
    }

unsigned int gs_data_getVersion_(struct GsData* d, int splitlotId);

/******************************************************************************!
 * \fn gs_data_loadData_
 * \brief Load a file (SQLite, stdf, ...)
 * \return void
 ******************************************************************************/
EXPORT void gs_data_loadData_(struct GsData* d, const char* path)
{
    int r;

    if (d == NULL)
    {
        return;
    }

    r = sqlite3_initialize();
    SQLITE_CHECK_RC_OR_RETURN(r, )
    r = sqlite3_open(path, &d->sqlite);
    SQLITE_CHECK_RC_OR_RETURN(r, )
}

/******************************************************************************!
 * \fn gs_data_unloadData_
 * \brief Unload the file (SQLite, stdf, ...)
 * \return void
 ******************************************************************************/
EXPORT void gs_data_unloadData_(struct GsData* d)
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
 * \fn gs_data_getParametricTestList_
 * \brief Get the parametric test list
 * \return JSON string
 ******************************************************************************/
EXPORT const char* gs_data_getParametricTestList_(struct GsData* d)
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
                  "select tnum,tname,units,ll,hl,ll_scal,hl_scal "
                  "from ft_ptest_info");
    SQLITE_PREPARE_OR_RETURN(NULL)

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff, "{\"ptest\":[");
    pass = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (pass)
        {
            gs_buffer_add(buff, ",");
        }
        gs_buffer_add(buff, "{");
        gs_buffer_add(buff, "\"tnum\":%d,", sqlite3_column_int(stmt, 0));
        gs_buffer_add(buff, "\"tname\":\"%s\",", sqlite3_column_text(stmt, 1));
        gs_buffer_add(buff, "\"units\":\"%s\",", sqlite3_column_text(stmt, 2));
        gs_buffer_add(buff, "\"ll\":%.15e,", sqlite3_column_double(stmt, 3));
        gs_buffer_add(buff, "\"hl\":%.15e,", sqlite3_column_double(stmt, 4));
        gs_buffer_add(buff, "\"llscal\":%d,", sqlite3_column_int(stmt, 5));
        gs_buffer_add(buff, "\"hl_scal\":%d", sqlite3_column_int(stmt, 6));
        gs_buffer_add(buff, "}");
        ++pass;
    }
    gs_buffer_add(buff, "]}");

    sqlite3_finalize(stmt);

    return gs_buffer_get(d->buffer);
}

/******************************************************************************!
 * \fn gs_data_getSplitlotIdList_
 * \brief Get the splitlot id list
 * \return JSON string
 ******************************************************************************/
EXPORT const char* gs_data_getSplitlotIdList_(struct GsData* d)
{
    sqlite3_stmt* stmt;
    int pass;
    struct GsBuffer* buff;
    unsigned int version;

    if (d == NULL)
    {
        return "";
    }

    version = gs_data_getVersion_(d, -1);

    buff = gs_buffer_init(d->buffer);

    if (version > 60100)
    {
        gs_buffer_add(buff,
                      "select splitlot_id "
                      "from ft_splitlot where valid_splitlot='Y'");
    }
    else
    {
        gs_buffer_add(buff,
                      "select splitlot_id "
                      "from ft_splitlot");
    }
    SQLITE_PREPARE_OR_RETURN(NULL)

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff, "{\"splitlot_id\":[");
    pass = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        if (pass)
        {
            gs_buffer_add(buff, ",");
        }
        gs_buffer_add(buff, "%d", sqlite3_column_int(stmt, 0));
        ++pass;
    }
    gs_buffer_add(buff, "]}");

    sqlite3_finalize(stmt);

    return gs_buffer_get(d->buffer);
}

/******************************************************************************!
 * \fn gs_data_getLastSplitlotId_
 * \brief Get the last splitlot id for the desired RetestIndex
          (if '%', then is will be the last splitlot id)
 * \return -1 on error
 ******************************************************************************/
EXPORT int
gs_data_getLastSplitlotId_(struct GsData* d, char lDesiredRetestIndex)
{
    sqlite3_stmt* stmt;
    int id = -1;
    struct GsBuffer* buff;

    if (d == NULL)
    {
        return -1;
    }

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff,
                  "select max(splitlot_id) "
                  "from ft_splitlot "
                  "where retest_index like '%c'",
                  lDesiredRetestIndex == '\0' ? '%' : lDesiredRetestIndex);
    SQLITE_PREPARE_OR_RETURN(-1)

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        id = sqlite3_column_int(stmt, 0);
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            sqlite3_finalize(stmt);
            d->error = GS_DATA_ERROR_NOTUNIQ;
            d->strerror =
                "gs_data_getLastSplitlotId: only one sqlite row expected";
            return -1;
        }
    }
    else
    {
        sqlite3_finalize(stmt);
        d->error = GS_DATA_ERROR_NORESULT;
        d->strerror = "gs_data_getLastSplitlotId: no result for sqlite query";
        return -1;
    }

    sqlite3_finalize(stmt);

    return id;
}

/******************************************************************************!
 * \fn gs_data_getMaxRetestIndex_
 * \brief Try to retrieve the max retest index of previous splitlots
 * \return -1 on error
 ******************************************************************************/
EXPORT int gs_data_getMaxRetestIndex_(struct GsData* d)
{
    sqlite3_stmt* stmt;
    int index = -1;
    struct GsBuffer* buff;

    if (d == NULL)
    {
        return -1;
    }

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff, "select max(retest_index) from ft_splitlot");
    SQLITE_PREPARE_OR_RETURN(-1)

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        index = sqlite3_column_int(stmt, 0);
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            sqlite3_finalize(stmt);
            d->error = GS_DATA_ERROR_NOTUNIQ;
            d->strerror =
                "gs_data_getMaxRetestIndex: only one sqlite row expected";
            return -1;
        }
    }
    else
    {
        sqlite3_finalize(stmt);
        d->error = GS_DATA_ERROR_NORESULT;
        d->strerror = "gs_data_getMaxRetestIndex: no result for sqlite query";
        return -1;
    }

    sqlite3_finalize(stmt);

    return index;
}

/******************************************************************************!
 * \fn gs_data_getKeyInt_
 * \brief Get integer value corresponding to the field name and the splitlot id
 * \return -1 if the value is not unique
 ******************************************************************************/
EXPORT int
gs_data_getKeyInt_(struct GsData* d, int splitlotId, const char* name)
{
    sqlite3_stmt* stmt;
    int key = -1;
    struct GsBuffer* buff;

    if (d == NULL)
    {
        return -1;
    }

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff,
                  "select %s "
                  "from ft_splitlot "
                  "where splitlot_id = %d", name, splitlotId);
    SQLITE_PREPARE_OR_RETURN(-1)

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        key = sqlite3_column_int(stmt, 0);
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            sqlite3_finalize(stmt);
            d->error = GS_DATA_ERROR_NOTUNIQ;
            d->strerror = "gs_data_getKeyInt: only one sqlite row expected";
            return -1;
        }
    }
    else
    {
        sqlite3_finalize(stmt);
        d->error = GS_DATA_ERROR_NORESULT;
        d->strerror = "gs_data_getKeyInt: no result for sqlite query";
        return -1;
    }

    sqlite3_finalize(stmt);

    //fprintf(stderr, "%s=%d\n", name, key);
    return key;
}

/******************************************************************************!
 * \fn gs_data_getKeyStr_
 * \brief Get string value corresponding to the field name and the splitlot id
 * \return -1 if the value is not unique
 ******************************************************************************/
EXPORT const char*
gs_data_getKeyStr_(struct GsData* d, int splitlotId, const char* name)
{
    sqlite3_stmt* stmt;
    struct GsBuffer* buff;

    if (d == NULL)
    {
        return "";
    }

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff,
                  "select %s "
                  "from ft_splitlot "
                  "where splitlot_id = %d", name, splitlotId);
    SQLITE_PREPARE_OR_RETURN(NULL)

    buff = gs_buffer_init(d->buffer);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        gs_buffer_add(buff, "%s", sqlite3_column_text(stmt, 0));
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            sqlite3_finalize(stmt);
            d->error = GS_DATA_ERROR_NOTUNIQ;
            d->strerror = "gs_data_getKeyStr: only one sqlite row expected";
            return "";
        }
    }
    else
    {
        sqlite3_finalize(stmt);
        d->error = GS_DATA_ERROR_NORESULT;
        d->strerror = "gs_data_getKeyStr: no result for sqlite query";
        return "";
    }

    sqlite3_finalize(stmt);

    //fprintf(stderr, "%s=%s\n", name, gs_buffer_get(d->buffer));
    return gs_buffer_get(d->buffer);
}

/******************************************************************************!
 * \fn gs_data_getTableKeyValue_
 * \brief Get string value corresponding to the field table.name
 * \return String, empty string if the value is not unique
 ******************************************************************************/
EXPORT const char*
gs_data_getTableKeyValue_(struct GsData* d,
                          const char* table,
                          const char* where)
{
    sqlite3_stmt* stmt;
    struct GsBuffer* buff;

    if (d == NULL)
    {
        return "";
    }

    buff = gs_buffer_init(d->buffer);

    gs_buffer_add(buff,
                  "select value from %s where %s", table, where);
    SQLITE_PREPARE_OR_RETURN(NULL)

    buff = gs_buffer_init(d->buffer);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        gs_buffer_add(buff, "%s", sqlite3_column_text(stmt, 0));
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            sqlite3_finalize(stmt);
            d->error = GS_DATA_ERROR_NOTUNIQ;
            d->strerror =
                "gs_data_getTableKeyValue: only one sqlite row expected";
            return "";
        }
    }
    else
    {
        sqlite3_finalize(stmt);
        d->error = GS_DATA_ERROR_NORESULT;
        d->strerror = "gs_data_getTableKeyValue: no result for sqlite query";
        return "";
    }

    sqlite3_finalize(stmt);

    //fprintf(stderr, "where %s:%s\n", where, gs_buffer_get(d->buffer));
    return gs_buffer_get(d->buffer);
}

/******************************************************************************!
 * \fn gs_data_getVersion_
 * \brief Get major, minor, and patch number
 * \return MMmmpp
 ******************************************************************************/
unsigned int gs_data_getVersion_(struct GsData* d, int splitlotId)
{
    sqlite3_stmt* stmt;
    struct GsBuffer* buff;
    unsigned int version = 0;

    if (d == NULL)
    {
        return version;
    }

    // Major
    buff = gs_buffer_init(d->buffer);
    if (splitlotId != -1)
    {
        gs_buffer_add(buff,
                      "select value from ft_gtl_info "
                      "where gtl_key='gtl_version_major' "
                      "and splitlot_id=%d", splitlotId);
    }
    else
    {
        gs_buffer_add(buff,
                      "select value from ft_gtl_info "
                      "where gtl_key='gtl_version_major'");
    }
    if (sqlite3_prepare_v2(d->sqlite, gs_buffer_get(d->buffer),
                           -1, &stmt, NULL) == SQLITE_OK &&
        sqlite3_step(stmt) == SQLITE_ROW)
    {
        gs_buffer_add(buff, "%s", sqlite3_column_text(stmt, 0));
        version += atoi(gs_buffer_get(d->buffer)) * 10000;
        sqlite3_finalize(stmt);

        // Minor
        buff = gs_buffer_init(d->buffer);
        gs_buffer_add(buff,
                      "select value from ft_gtl_info "
                      "where gtl_key='gtl_version_minor' "
                      "and splitlot_id=%d", splitlotId);
        SQLITE_PREPARE_OR_RETURN(version)
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            gs_buffer_add(buff, "%s", sqlite3_column_text(stmt, 0));
            version += atoi(gs_buffer_get(d->buffer)) * 100;
        }
        sqlite3_finalize(stmt);
    }
    else
    {
        sqlite3_finalize(stmt);

        // Major
        buff = gs_buffer_init(d->buffer);
        gs_buffer_add(buff,
                      "select value from global_settings "
                      "where key='gtl_version_major'");
        SQLITE_PREPARE_OR_RETURN(version)
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            gs_buffer_add(buff, "%s", sqlite3_column_text(stmt, 0));
            version += atoi(gs_buffer_get(d->buffer)) * 10000;
        }
        sqlite3_finalize(stmt);

        // Minor
        buff = gs_buffer_init(d->buffer);
        gs_buffer_add(buff,
                      "select value from global_settings "
                      "where key='gtl_version_minor'");
        SQLITE_PREPARE_OR_RETURN(version)
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            gs_buffer_add(buff, "%s", sqlite3_column_text(stmt, 0));
            version += atoi(gs_buffer_get(d->buffer)) * 100;
        }
        sqlite3_finalize(stmt);
    }

    return version;
}
