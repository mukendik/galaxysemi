/******************************************************************************!
 * \file main.cpp
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "gs_data.h"
#include "gs_gtl_traceability.h"
#include "gs_json.h"

int gNbError = 0;

/******************************************************************************!
 * \fn gsDataError
 ******************************************************************************/
bool gsDataError(struct GsData* tr)
{
    if (gs_data_getLastError(tr))
    {
        fprintf(stderr, "error: %d: %s\n",
                gs_data_getLastError(tr),
                gs_data_getLastStrError(tr));
        ++gNbError;
        return true;
    }
    else
    {
        return false;
    }
}

/******************************************************************************!
 * \fn gsGtlTraceabilityError
 ******************************************************************************/
bool gsGtlTraceabilityError(struct GsGtlTraceability* tr)
{
    if (gs_gtl_traceability_getLastError(tr))
    {
        fprintf(stderr, "error: %d: %s\n",
                gs_gtl_traceability_getLastError(tr),
                gs_gtl_traceability_getLastStrError(tr));
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
    struct GsData* d;
    struct GsGtlTraceability* tr;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <sqlite>\n", argv[0]);
        return EXIT_FAILURE;
    }

    d = gs_data_new();
    d = gs_data_init(d, GS_DATA_SQLITE);
    if (gsDataError(d))
    {
        return EXIT_FAILURE;
    }
    gs_data_loadData(d, argv[1]);
    if (gsDataError(d))
    {
        gs_data_quit(d);
        free(d);
        return EXIT_FAILURE;
    }
    tr = gs_gtl_traceability_new();
    tr = gs_gtl_traceability_init_with_gsdata(tr, d);
    if (gsGtlTraceabilityError(tr))
    {
        return EXIT_FAILURE;
    }
    gs_gtl_traceability_loadData(tr, argv[1]);
    if (gsGtlTraceabilityError(tr))
    {
        gs_gtl_traceability_quit(tr);
        free(tr);
        return EXIT_FAILURE;
    }

    // splitlot_id list
    const char* json = gs_data_getSplitlotIdList(d);
    if (! gsGtlTraceabilityError(tr))
    {
        fprintf(stderr, "%s\n", json);
    }
    std::string splitlotIdList(json);

    GsJsonIterator iter;
    for (GsJsonIteratorBegin(&iter, splitlotIdList.c_str());
         GsJsonIteratorEnd(&iter) == 0;
         GsJsonIteratorNext(&iter))
    {
        // ft_hbin
        json = gs_gtl_traceability_getFtHBin(tr, GsJsonIteratorInt(&iter));
        if (! gsGtlTraceabilityError(tr))
        {
            fprintf(stderr, "%s\n", json);
        }
        GsJsonIterator hbinIter;
        const char* label;
        const char* value;
        std::string firstLabel;
        int pass = 0;
        for (GsJsonIteratorBegin(&hbinIter, json);
             GsJsonIteratorEnd(&hbinIter) == 0;
             GsJsonIteratorNext(&hbinIter))
        {
            label = GsJsonIteratorLabel(&hbinIter);
            value = GsJsonIteratorValue(&hbinIter);
            fprintf(stderr, "%s = %s\n", label, value);
        }
        for (GsJsonIteratorBegin(&hbinIter, json);
             GsJsonIteratorEnd(&hbinIter) == 0;
             GsJsonIteratorNext(&hbinIter))
        {
            label = GsJsonIteratorLabel(&hbinIter);
            if (! pass)
            {
                firstLabel = label;
            }
            else if (label == firstLabel)
            {
                break;
            }
            fprintf(stderr, "%s\n", label);
            ++pass;
        }
        for (GsJsonIteratorBegin(&hbinIter, json);
             GsJsonIteratorEnd(&hbinIter) == 0;
             GsJsonIteratorNext(&hbinIter))
        {
            value = GsJsonIteratorValue(&hbinIter);
            fprintf(stderr, "%s\n", value);
        }
    }

    gs_gtl_traceability_unloadData(tr);
    if (gsGtlTraceabilityError(tr))
    {
    }
    gs_gtl_traceability_quit(tr);
    if (gsGtlTraceabilityError(tr))
    {
        free(tr);
        return EXIT_FAILURE;
    }
    free(tr);

    if (gNbError)
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
