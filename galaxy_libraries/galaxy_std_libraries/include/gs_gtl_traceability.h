/******************************************************************************!
 * \file gs_gtl_traceability.h
 ******************************************************************************/
#ifndef GS_GTL_TRACEABILITY_H
#define GS_GTL_TRACEABILITY_H
#include <sqlite3.h>

#define GS_SQLITE_ERROR_BASE 40000
#define GS_GTL_TRACEABILITY_ERROR_BASE 47100
#define GS_GTL_TRACEABILITY_ERROR_BUFFNEW (GS_GTL_TRACEABILITY_ERROR_BASE + 1)
#define GS_GTL_TRACEABILITY_ERROR_PTRNULL (GS_GTL_TRACEABILITY_ERROR_BASE + 2)
#define GS_GTL_TRACEABILITY_ERROR_DATALOAD (GS_GTL_TRACEABILITY_ERROR_BASE + 3)

struct GsGtlTraceability
{
    /*!
     * \var sqlite
     * \brief Input from SQlite
     */
    sqlite3* sqlite;
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
};

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \fn gs_gtl_traceability_new
 * \brief Create a descriptor
 * \return GsGtlTraceability descriptor
 */
struct GsGtlTraceability* gs_gtl_traceability_new();
/*!
 * \fn gs_gtl_traceability_init
 * \brief Initialize a descriptor
 * \return GsGtlTraceability descriptor
 */
struct GsGtlTraceability* gs_gtl_traceability_init(struct GsGtlTraceability* d);
/*!
 * \fn gs_gtl_traceability_init_with_gsdata
 * \brief Initialize a descriptor with a GsData struct
 * \return GsGtlTraceability descriptor
 */
struct GsData;
struct GsGtlTraceability*
gs_gtl_traceability_init_with_gsdata(struct GsGtlTraceability* d,
                                     struct GsData* lGsData);
/*!
 * \fn gs_gtl_traceability_quit
 * \brief The oposite of init, do not forget to free the descriptor
 * \return void
 */
void gs_gtl_traceability_quit(struct GsGtlTraceability* d);
/*!
 * \fn gs_gtl_traceability_getLastError
 * \brief Get the last error code
 * \return Error code
 */
int gs_gtl_traceability_getLastError(struct GsGtlTraceability* d);
/*!
 * \fn gs_gtl_traceability_getLastStrError
 * \brief Get the last error decription
 * \return String
 */
const char* gs_gtl_traceability_getLastStrError(struct GsGtlTraceability* d);
/*!
 * \fn gs_gtl_traceability_loadData
 * \brief Load a SQlite file
 * \return void
 */
void
gs_gtl_traceability_loadData(struct GsGtlTraceability* d, const char* path);
/*!
 * \fn gs_gtl_traceability_unloadData
 * \brief Unload the SQlite file
 * \return void
 */
void
gs_gtl_traceability_unloadData(struct GsGtlTraceability* d);
/*!
 * \fn gs_gtl_traceability_getFtEvent
 * \return JSON string
 */
const char*
gs_gtl_traceability_getFtEvent(struct GsGtlTraceability* d,
                               int splitlotId);
/*!
 * \fn gs_gtl_traceability_getFtHBin
 * \return JSON string
 */
const char*
gs_gtl_traceability_getFtHBin(struct GsGtlTraceability* d,
                              int splitlotId);
/*!
 * \fn gs_gtl_traceability_getFtPtestInfo
 * \return JSON string
 */
const char*
gs_gtl_traceability_getFtPtestInfo(struct GsGtlTraceability* d,
                                   int splitlotId);
/*!
 * \fn gs_gtl_traceability_getFtPtestOutliers
 * \return JSON string
 */
const char*
gs_gtl_traceability_getFtPtestOutliers(struct GsGtlTraceability* d,
                                       int splitlotId);
/*!
 * \fn gs_gtl_traceability_getFtPtestRollingLimits
 * \return JSON string
 */
const char*
gs_gtl_traceability_getFtPtestRollingLimits(struct GsGtlTraceability* d,
                                            int splitlotId);
/*!
 * \fn gs_gtl_traceability_getFtPtestRollingStats
 * \return JSON string
 */
const char*
gs_gtl_traceability_getFtPtestRollingStats(struct GsGtlTraceability* d,
                                           int splitlotId);
/*!
 * \fn gs_gtl_traceability_getFtMPtestInfo
 * \return JSON string
 */
const char*
gs_gtl_traceability_getFtMPtestInfo(struct GsGtlTraceability* d,
                                    int splitlotId);
/*!
 * \fn gs_gtl_traceability_getFtMPtestOutliers
 * \return JSON string
 */
const char*
gs_gtl_traceability_getFtMPtestOutliers(struct GsGtlTraceability* d,
                                        int splitlotId);
/*!
 * \fn gs_gtl_traceability_getFtMPtestRollingLimits
 * \return JSON string
 */
const char*
gs_gtl_traceability_getFtMPtestRollingLimits(struct GsGtlTraceability* d,
                                             int splitlotId);
/*!
 * \fn gs_gtl_traceability_getFtMPtestRollingStats
 * \return JSON string
 */
const char*
gs_gtl_traceability_getFtMPtestRollingStats(struct GsGtlTraceability* d,
                                            int splitlotId);
/*!
 * \fn gs_gtl_traceability_getFtRun
 * \return JSON string
 */
const char*
gs_gtl_traceability_getFtRun(struct GsGtlTraceability* d,
                             int splitlotId);
/*!
 * \fn gs_gtl_traceability_getFtSBin
 * \return JSON string
 */
const char*
gs_gtl_traceability_getFtSBin(struct GsGtlTraceability* d,
                              int splitlotId);
/*!
 * \fn gs_gtl_traceability_getFtSplitlot
 * \return JSON string
 */
const char*
gs_gtl_traceability_getFtSplitlot(struct GsGtlTraceability* d,
                                  int splitlotId);
/*!
 * \fn gs_gtl_traceability_getGlobalFileInfos
 * \return JSON string
 */
const char*
gs_gtl_traceability_getGlobalFileInfos(struct GsGtlTraceability* d,
                                       int splitlotId);
/*!
 * \fn gs_gtl_traceability_getGlobalFileContent
 * \return JSON string
 */
const char*
gs_gtl_traceability_getGlobalFileContent(struct GsGtlTraceability* d,
                                         int splitlotId);
/*!
 * \fn gs_gtl_traceability_getFtGtlInfo
 * \return JSON string
 */
const char*
gs_gtl_traceability_getFtGtlInfo(struct GsGtlTraceability* d,
                                 int lSplitlotId);

#ifdef __cplusplus
}
#endif

#endif
