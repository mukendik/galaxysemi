/******************************************************************************!
 * \file gs_data.c
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
# include "dlfcn_win32.h"
# define LIBEXT "dll"
#else
# include <dlfcn.h>
# if defined(__APPLE__) && defined(__MACH__)
#  define LIBEXT "dylib"
# else
#  define LIBEXT "so"
# endif
#endif
#include "gs_data.h"

// Macro to load a function pointer from the library
#define GS_DATA_LOAD_SYM(a, b, c, d) \
    gs_data_loadSym(d, (void**) &a, b); \
    if (d->error) \
    { \
        return c; \
    }

/******************************************************************************!
 * \fn gs_data_clear
 * \brief Clear all function pointers
 *        Used by gs_data_new and gs_data_quit only
 ******************************************************************************/
void gs_data_clear(struct GsData* d)
{
    d->fLoadData = NULL;
    d->fUnloadData = NULL;
    d->fGetParametricTestList = NULL;
    d->fGetSplitlotIdList = NULL;
    d->fGetLastSplitlotId = NULL;
    d->fGetMaxRetestIndex = NULL;
    d->fGetKeyInt = NULL;
    d->fGetKeyStr = NULL;
    d->fGetTableKeyValue = NULL;
    d->fGetVersion = NULL;
}

/******************************************************************************!
 * \fn gs_data_new
 * \brief Initialize a descriptor
 * \return GsData descriptor
 ******************************************************************************/
struct GsData* gs_data_new()
{
    struct GsData* d = malloc(sizeof(struct GsData));
    if (d == NULL)
    {
        return NULL;
    }

    d->lib = NULL;
    d->sqlite = NULL;
    d->buffer = NULL;
    d->error = 0;
    d->strerror = NULL;

    gs_data_clear(d);

    return d;
}

/******************************************************************************!
 * \fn gs_data_loadLibrary
 * \brief dlopen the library
 *        Used by gs_data_init only
 ******************************************************************************/
void gs_data_loadLibrary(struct GsData* d, const char* path)
{
    if (d == NULL)
    {
        return;
    }

    d->lib = dlopen(path, RTLD_LAZY);
    d->strerror = dlerror();
    if (d->strerror != NULL)
    {
        d->error = GS_DATA_ERROR_DLOPEN;
    }
}

/******************************************************************************!
 * \fn gs_data_init
 * \brief Initialize a descriptor
 * \return GsData descriptor
 ******************************************************************************/
struct GsData* gs_data_init(struct GsData* d, enum gs_data_type type)
{
#   if ! defined(_WIN32) && ! defined(_WIN64) && ! defined(__WIN32__)
    const char* lGexPath;
#   else
    char* c;
#   endif

    if (d == NULL)
    {
        return NULL;
    }

    if (d->lib != NULL)
    {
        d->strerror = "gs_data_init: plugin already loaded";
        d->error = GS_DATA_ERROR_LIBINIT;
        return d;
    }

    d->buffer = gs_buffer_new();
    if (d->buffer == NULL)
    {
        d->strerror = "gs_data_init: cannot allocate buffer";
        d->error = GS_DATA_ERROR_BUFFNEW;
        return d;
    }

    d->buffer = gs_buffer_init(d->buffer);

    // Lib path
#   if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
    if (GetModuleFileNameA(NULL, d->buffer->ptr, d->buffer->capacity) == 0)
    {
        d->strerror = "gs_data_init: GetModuleFileNameA error";
        d->error = GS_DATA_ERROR_LIBINIT;
        return d;
    }
    d->buffer->size = strlen(d->buffer->ptr);
    c = d->buffer->ptr + d->buffer->size - 1;
    while (c > d->buffer->ptr && *c != '\\')
    {
        --c;
    }
    *c = '\0';
    d->buffer->size = strlen(d->buffer->ptr);
    gs_buffer_add(d->buffer, "/plugins/db/libgs_data_");
#   else
    lGexPath = getenv("GEX_PATH");
    if (lGexPath == NULL)
    {
        d->strerror = "gs_data_init: error retrieving application's folder";
        d->error = GS_DATA_ERROR_LIBINIT;
        return d;
    }
    gs_buffer_add(d->buffer, "%s/plugins/db/libgs_data_", lGexPath);
#   endif

    // Lib filename
    switch (type)
    {
    case GS_DATA_STDF:
        gs_buffer_add(d->buffer, "stdf."LIBEXT);
        break;
    case GS_DATA_TDR:
        gs_buffer_add(d->buffer, "tdr."LIBEXT);
        break;
    case GS_DATA_SQLITE:
        gs_buffer_add(d->buffer, "sqlite."LIBEXT);
        break;
    default:
        d->strerror = "gs_data_init: unknown plugin type";
        d->error = GS_DATA_ERROR_LIBTYPE;
        break;
    }

    // Load
    gs_data_loadLibrary(d, gs_buffer_get(d->buffer));

    return d;
}

/******************************************************************************!
 * \fn gs_data_quit
 * \brief The oposite of init, do not forget to free the descriptor
 * \return void
 ******************************************************************************/
void gs_data_quit(struct GsData* d)
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
    if (d->lib != NULL)
    {
        gs_data_clear(d);
        if (dlclose(d->lib))
        {
            d->strerror = dlerror();
            d->error = GS_DATA_ERROR_DLCLOSE;
        }
        d->lib = NULL;
    }
}

/******************************************************************************!
 * \fn gs_data_loadSym
 * \brief Load a function pointer from the library
 *        Used by GS_DATA_LOAD_SYM only
 ******************************************************************************/
void gs_data_loadSym(struct GsData* d, void** func, const char* sym)
{
    if (*func == NULL)
    {
        *func = dlsym(d->lib, sym);
        if (*func == NULL)
        {
            d->strerror = dlerror();
            d->error = GS_DATA_ERROR_DLSYM;
        }
    }
}

/******************************************************************************!
 * \fn gs_data_getLastError
 * \brief Get the last error code
 * \return Error code
 ******************************************************************************/
int gs_data_getLastError(struct GsData* d)
{
    if (d == NULL)
    {
        return GS_DATA_ERROR_PTRNULL;
    }

    return d->error;
}

/******************************************************************************!
 * \fn gs_data_getLastStrError
 * \brief Get the last error decription
 * \return String
 ******************************************************************************/
const char* gs_data_getLastStrError(struct GsData* d)
{
    if (d == NULL)
    {
        return "gs_data null pointer";
    }

    if (d->strerror != NULL)
    {
        return d->strerror;
    }
    else
    {
        return "gs_data undefined error";
    }
}

/******************************************************************************!
 * \fn gs_data_loadData
 * \brief Load a file (SQLite, stdf, ...)
 *        This function must be specialized by libraries
 * \return void
 ******************************************************************************/
void gs_data_loadData(struct GsData* d, const char* path)
{
    struct stat buff;

    if (d == NULL)
    {
        return;
    }

    if (stat(path, &buff) == -1)
    {
        d->strerror = "gs_data_loadData: cannot stat the file";
        d->error = GS_DATA_ERROR_DATALOAD;
        return;
    }

    GS_DATA_LOAD_SYM(d->fLoadData,
                     "gs_data_loadData_", , d)
        (*d->fLoadData)(d, path);
}

/******************************************************************************!
 * \fn gs_data_unloadData
 * \brief Unload the file (SQLite, stdf, ...)
 *        This function must be specialized by libraries
 * \return void
 ******************************************************************************/
void gs_data_unloadData(struct GsData* d)
{
    if (d == NULL)
    {
        return;
    }

    GS_DATA_LOAD_SYM(d->fUnloadData,
                     "gs_data_unloadData_", , d)
        (*d->fUnloadData)(d);
}

/******************************************************************************!
 * \fn gs_data_getParametricTestList
 * \brief Get the parametric test list
 *        This function must be specialized by libraries
 * \return JSON string
 ******************************************************************************/
const char* gs_data_getParametricTestList(struct GsData* d)
{
    if (d == NULL)
    {
        return "";
    }

    GS_DATA_LOAD_SYM(d->fGetParametricTestList,
                     "gs_data_getParametricTestList_", NULL, d)
    return (*d->fGetParametricTestList)(d);
}

/******************************************************************************!
 * \fn gs_data_getSplitlotIdList
 * \brief Get the last splitlot id for the desired RetestIndex
 *        (if '%', then is will be the last splitlot id)
 *        This function must be specialized by libraries
 * \return -1 on error
 ******************************************************************************/
const char* gs_data_getSplitlotIdList(struct GsData* d)
{
    if (d == NULL)
    {
        return "";
    }

    GS_DATA_LOAD_SYM(d->fGetSplitlotIdList,
                     "gs_data_getSplitlotIdList_", NULL, d);
    return (*d->fGetSplitlotIdList)(d);
}

/******************************************************************************!
 * \fn gs_data_getLastSplitlotId
 * \brief Get the last splitlot id for the desired RetestIndex
 *        (if '%', then is will be the last splitlot id)
 *        This function must be specialized by libraries
 * \return splitlot id
 ******************************************************************************/
int gs_data_getLastSplitlotId(struct GsData* d, char lDesiredRetestIndex)
{
    if (d == NULL)
    {
        return -1;
    }

    GS_DATA_LOAD_SYM(d->fGetLastSplitlotId,
                     "gs_data_getLastSplitlotId_", -1, d);
    return (*d->fGetLastSplitlotId)(d, lDesiredRetestIndex);
}

/******************************************************************************!
 * \fn gs_data_getMaxRetestIndex
 * \brief Try to retrieve the max retest index of previous splitlots
 *        This function must be specialized by libraries
 * \return -1 on error
 ******************************************************************************/
int gs_data_getMaxRetestIndex(struct GsData* d)
{
    if (d == NULL)
    {
        return -1;
    }

    GS_DATA_LOAD_SYM(d->fGetMaxRetestIndex,
                     "gs_data_getMaxRetestIndex_", -1, d);
    return (*d->fGetMaxRetestIndex)(d);
}

/******************************************************************************!
 * \fn gs_data_getKeyInt
 * \brief Get integer value corresponding to the field name and the splitlot id
 *        This function must be specialized by libraries
 * \return -1 if the value is not unique
 ******************************************************************************/
int gs_data_getKeyInt(struct GsData* d, int splitlotId, const char* name)
{
    if (d == NULL)
    {
        return -1;
    }

    d->error = 0;
    d->strerror = NULL;

    GS_DATA_LOAD_SYM(d->fGetKeyInt,
                     "gs_data_getKeyInt_", -1, d)
    return (*d->fGetKeyInt)(d, splitlotId, name);
}

/******************************************************************************!
 * \fn gs_data_getKeyStr
 * \brief Get string value corresponding to the field name and the splitlot id
 *        This function must be specialized by libraries
 * \return String, empty string if the value is not unique
 ******************************************************************************/
const char*
gs_data_getKeyStr(struct GsData* d, int splitlotId, const char* name)
{
    if (d == NULL)
    {
        return "";
    }

    d->error = 0;
    d->strerror = NULL;

    GS_DATA_LOAD_SYM(d->fGetKeyStr,
                     "gs_data_getKeyStr_", NULL, d)
    return (*d->fGetKeyStr)(d, splitlotId, name);
}

/******************************************************************************!
 * \fn gs_data_getTableKeyValue
 * \brief Get string value corresponding to the field table.name
 *        This function must be specialized by libraries
 * \return String, empty string if the value is not unique
 ******************************************************************************/
const char*
gs_data_getTableKeyValue(struct GsData* d, const char* table, const char* name)
{
    if (d == NULL)
    {
        return "";
    }

    d->error = 0;
    d->strerror = NULL;

    GS_DATA_LOAD_SYM(d->fGetTableKeyValue,
                     "gs_data_getTableKeyValue_", NULL, d)
    return (*d->fGetTableKeyValue)(d, table, name);
}

/******************************************************************************!
 * \fn gs_data_getVersion
 * \brief Get major, minor, and patch number
 *        This function must be specialized by libraries
 * \return MMmmpp
 ******************************************************************************/
unsigned int gs_data_getVersion(struct GsData* d, int splitlotId)
{
    if (d == NULL)
    {
        return 0;
    }

    d->error = 0;
    d->strerror = NULL;

    GS_DATA_LOAD_SYM(d->fGetVersion,
                     "gs_data_getVersion_", 0, d)
    return (*d->fGetVersion)(d, splitlotId);
}
