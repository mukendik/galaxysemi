/******************************************************************************!
 * \file gs_data.h
 ******************************************************************************/
#ifndef GS_DATA_H
#define GS_DATA_H
#include "gs_buffer.h"

#define GS_SQLITE_ERROR_BASE 40000
#define GS_DATA_ERROR_BASE 47000
#define GS_DATA_ERROR_BUFFNEW (GS_DATA_ERROR_BASE + 1)
#define GS_DATA_ERROR_PTRNULL (GS_DATA_ERROR_BASE + 2)
#define GS_DATA_ERROR_DATALOAD (GS_DATA_ERROR_BASE + 3)
#define GS_DATA_ERROR_DLOPEN (GS_DATA_ERROR_BASE + 4)
#define GS_DATA_ERROR_LIBINIT (GS_DATA_ERROR_BASE + 5)
#define GS_DATA_ERROR_LIBTYPE (GS_DATA_ERROR_BASE + 6)
#define GS_DATA_ERROR_DLCLOSE (GS_DATA_ERROR_BASE + 7)
#define GS_DATA_ERROR_DLSYM (GS_DATA_ERROR_BASE + 8)
#define GS_DATA_ERROR_NOTUNIQ (GS_DATA_ERROR_BASE + 9)
#define GS_DATA_ERROR_NORESULT (GS_DATA_ERROR_BASE + 10)

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
# define EXPORT __declspec(dllexport)
#else
# define EXPORT
#endif

struct GsData
{
    /*!
     * \var lib
     * \brief Library loaded to open SQLite file, or stdf file, or...
     */
    void* lib;
    /*!
     * \var sqlite
     * \brief Not NULL if the datas are in a SQLite file
     */
    struct sqlite3* sqlite;
    /*!
     * \var buffer
     * \brief Generic buffer
     */
    struct GsBuffer* buffer;
    /*!
     * \var error
     * \brief Last error code
     */
    int error;
    /*!
     * \var strerror
     * \brief Last error description
     */
    const char* strerror;

    /*!
     * \fn fLoadData
     * \brief Pointer of function
     */
    int (* fLoadData)(struct GsData*, const char*);
    /*!
     * \fn fUnloadData
     * \brief Pointer of function
     */
    int (* fUnloadData)(struct GsData*);
    /*!
     * \fn fGetParametricTestList
     * \brief Pointer of function
     */
    const char* (* fGetParametricTestList)(struct GsData*);
    /*!
     * \fn fGetSplitlotIdList
     * \brief Pointer of function
     */
    const char* (* fGetSplitlotIdList)(struct GsData*);
    /*!
     * \fn fGetLastSplitlotId
     * \brief Pointer of function
     */
    int (* fGetLastSplitlotId)(struct GsData*, char);
    /*!
     * \fn fGetMaxRetestIndex
     * \brief Pointer of function
     */
    int (* fGetMaxRetestIndex)(struct GsData*);
    /*!
     * \fn fGetKeyInt
     * \brief Pointer of function
     */
    int (* fGetKeyInt)(struct GsData*, int, const char*);
    /*!
     * \fn fGetKeyStr
     * \brief Pointer of function
     */
    const char* (* fGetKeyStr)(struct GsData*, int, const char*);
    /*!
     * \fn fGetTableKeyValue
     * \brief Pointer of function
     */
    const char* (* fGetTableKeyValue)(struct GsData*, const char*, const char*);
    /*!
     * \fn fGetVersion
     * \brief Pointer of function
     */
    unsigned int (* fGetVersion)(struct GsData*, int);
};

enum gs_data_type
{
    GS_DATA_STDF,
    GS_DATA_TDR,
    GS_DATA_SQLITE
};

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \fn gs_data_new
 * \brief Create a descriptor
 * \return GsData descriptor
 */
struct GsData* gs_data_new();
/*!
 * \fn gs_data_init
 * \brief Initialize a descriptor
 * \return GsData descriptor
 */
struct GsData* gs_data_init(struct GsData* d, enum gs_data_type type);
/*!
 * \fn gs_data_quit
 * \brief The oposite of init, do not forget to free the descriptor
 * \return void
 */
void gs_data_quit(struct GsData* d);
/*!
 * \fn gs_data_getLastError
 * \brief Get the last error code
 * \return Error code
 */
int gs_data_getLastError(struct GsData* d);
/*!
 * \fn gs_data_getLastStrError
 * \brief Get the last error decription
 * \return String
 */
const char* gs_data_getLastStrError(struct GsData* d);
/*!
 * \fn gs_data_loadData
 * \brief Load a file (SQLite, stdf, ...)
 *        This function must be specialized by libraries
 * \return void
 */
void gs_data_loadData(struct GsData* d, const char* path);
/*!
 * \fn gs_data_unloadData
 * \brief Unload the file (SQLite, stdf, ...)
 *        This function must be specialized by libraries
 * \return void
 */
void gs_data_unloadData(struct GsData* d);
/*!
 * \fn gs_data_getParametricTestList
 * \brief Get the parametric test list
 *        This function must be specialized by libraries
 * \return JSON string
 */
const char* gs_data_getParametricTestList(struct GsData* d);
/*!
 * \fn gs_data_getSplitlotIdList
 * \brief Get the splitlot id list
 *        This function must be specialized by libraries
 * \return JSON string
 */
const char* gs_data_getSplitlotIdList(struct GsData* d);
/*!
 * \fn gs_data_getLastSplitlotId
 * \brief Get the last splitlot id for the desired RetestIndex
 *        (if '%', then is will be the last splitlot id)
 *        This function must be specialized by libraries
 * \return -1 on error
 */
int gs_data_getLastSplitlotId(struct GsData* d, char lDesiredRetestIndex);
/*!
 * \fn gs_data_getMaxRetestIndex
 * \brief Try to retrieve the max retest index of previous splitlots
 *        This function must be specialized by libraries
 * \return -1 on error
 */
int gs_data_getMaxRetestIndex(struct GsData* d);
/*!
 * \fn gs_data_getKeyInt
 * \brief Get integer value corresponding to the field name and the splitlot id
 *        This function must be specialized by libraries
 * \return -1 if the value is not unique
 */
int gs_data_getKeyInt(struct GsData* d, int splitlotId, const char* name);
/*!
 * \fn gs_data_getKeyStr
 * \brief Get string value corresponding to the field name and the splitlot id
 *        This function must be specialized by libraries
 * \return String, empty string if the value is not unique
 */
const char*
gs_data_getKeyStr(struct GsData* d, int splitlotId, const char* name);
/*!
 * \fn gs_data_getTableKeyValue
 * \brief Get string value corresponding to the field table.name
 *        This function must be specialized by libraries
 * \return String, empty string if the value is not unique
 */
const char*
gs_data_getTableKeyValue(struct GsData* d, const char* table, const char* name);
/*!
 * \fn gs_data_getVersion
 * \brief Get major, minor, and patch number
 *        This function must be specialized by libraries
 */
unsigned int gs_data_getVersion(struct GsData* d, int splitlotId);

#ifdef __cplusplus
}
#endif

#endif
