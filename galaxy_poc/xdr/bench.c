// -------------------------------------------------------------------------- //
// bench.c
// -------------------------------------------------------------------------- //
#include <sys/time.h>

#include "common.h"

// -------------------------------------------------------------------------- //
// main
// -------------------------------------------------------------------------- //
int main(int argc, char** argv)
{
    unsigned int nb;
    unsigned int i;
    char* buffer = NULL;
    float* ptrFloat;
    uint16_t* ptrInt;
    struct timeval tv0;
    struct timeval tv1;
#   ifndef NXDR
    XDR xdr;
#   endif

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <nb>\n", argv[0]);
        return EXIT_FAILURE;
    }
    nb = atoi(argv[1]);

    if (sizeof(float) != 4) {
        errx(EXIT_FAILURE, "error: sizeof float");
    }

    if ((buffer = malloc(nb << 2)) == NULL) {
        err(EXIT_FAILURE, "malloc");
    }

    ptrInt = (uint16_t*) buffer;
    for (i = 0; i < (nb << 1); ++i) {
        *ptrInt = rand() & 0xFFFFu;
        ++ptrInt;
    }

    if (gettimeofday(&tv0, NULL)) {
        err(EXIT_FAILURE, "gettimeofday");
    }

#   ifndef NXDR
    xdrmem_create(&xdr, buffer, nb << 2, XDR_ENCODE);
#   endif

    ptrFloat = (float*) buffer;
    for (i = 0; i < nb; ++i) {
#       ifndef NXDR
        if (! xdr_float(&xdr, ptrFloat)) {
            errx(EXIT_FAILURE, "error: xdr_float");
        }
#       else
        *ptrFloat = *ptrFloat * 2.0f;
#       endif
        ++ptrFloat;
    }

#   ifndef NXDR
    xdr_destroy(&xdr);
#   endif

#   ifndef NXDR
    xdrmem_create(&xdr, buffer, nb << 2, XDR_DECODE);
#   endif

    ptrFloat = (float*) buffer;
    for (i = 0; i < nb; ++i) {
#       ifndef NXDR
        if (! xdr_float(&xdr, ptrFloat)) {
            errx(EXIT_FAILURE, "error: xdr_float");
        }
#       else
        *ptrFloat = *ptrFloat * 2.0f;
#       endif
        ++ptrFloat;
    }

#   ifndef NXDR
    xdr_destroy(&xdr);
#   endif

    if (gettimeofday(&tv1, NULL)) {
        err(EXIT_FAILURE, "gettimeofday");
    }

    if (tv1.tv_usec < tv0.tv_usec) {
        tv1.tv_usec += 1000000;
        tv1.tv_sec -= 1;
    }
    fprintf(stderr, "%-11u %ld.%06ld\n", nb,
            tv1.tv_sec - tv0.tv_sec, (long) tv1.tv_usec - tv0.tv_usec);

    return EXIT_SUCCESS;
}
