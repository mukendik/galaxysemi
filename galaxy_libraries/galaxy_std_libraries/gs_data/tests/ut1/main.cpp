/******************************************************************************!
 * \file main.cpp
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "gs_data.h"

int gNbError = 0;

/******************************************************************************!
 * \fn gsDataError
 ******************************************************************************/
bool gsDataError(struct GsData* d)
{
    if (gs_data_getLastError(d))
    {
        fprintf(stderr, "error: %d: %s\n",
                gs_data_getLastError(d),
                gs_data_getLastStrError(d));
        ++gNbError;
        return true;
    }
    else
    {
        return false;
    }
}

/******************************************************************************!
 * \fn main
 ******************************************************************************/
int main(int argc, char* argv[])
{
    const char* ptests;
    int splitlotId;
    int intval;
    const char* strval;
    struct GsData* d;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <sqlite>\n", argv[0]);
        return EXIT_FAILURE;
    }

    d = gs_data_new();
    d = gs_data_init(d, GS_DATA_SQLITE);
    if (gsDataError(d))
    {
        if (d != NULL)
        {
            free(d);
        }
        return EXIT_FAILURE;
    }
    gs_data_loadData(d, argv[1]);
    if (gsDataError(d))
    {
        gs_data_quit(d);
        free(d);
        return EXIT_FAILURE;
    }

    // Parametric test list
    ptests = gs_data_getParametricTestList(d);
    if (! gsDataError(d))
    {
        fprintf(stderr, "%s\n", ptests);
    }

    // Last splitlot id
    splitlotId = gs_data_getLastSplitlotId(d, 0);
    if (! gsDataError(d))
    {
        fprintf(stderr, "splitlot_id = %d\n", splitlotId);
    }

    // Max retest index
    intval = gs_data_getMaxRetestIndex(d);
    if (! gsDataError(d))
    {
        fprintf(stderr, "max retest index = %d\n", intval);
    }

    // key start_t
    intval = gs_data_getKeyInt(d, splitlotId, "start_t");
    if (! gsDataError(d))
    {
        fprintf(stderr, "start_t = %d\n", intval);
    }

    // key finish_t
    intval = gs_data_getKeyInt(d, splitlotId, "finish_t");
    if (! gsDataError(d))
    {
        fprintf(stderr, "finish_t = %d\n", intval);
    }

    // key lot_id
    strval = gs_data_getKeyStr(d, splitlotId, "lot_id");
    if (! gsDataError(d))
    {
        fprintf(stderr, "lot_id = %s\n", strval);
    }

    gs_data_unloadData(d);
    if (gsDataError(d))
    {
    }
    gs_data_quit(d);
    if (gsDataError(d))
    {
        free(d);
        return EXIT_FAILURE;
    }
    free(d);

    if (gNbError)
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
