// -------------------------------------------------------------------------- //
// client.c
// -------------------------------------------------------------------------- //
#include "common.h"

struct in_addr gInAddr;

// -------------------------------------------------------------------------- //
// deserializeFloatBuffer
// -------------------------------------------------------------------------- //
char* deserializeFloatBuffer(char* buffer)
{
    float* ptr = (float*) buffer;
    XDR xdr;
    unsigned int i;

    xdrmem_create(&xdr, buffer, FLOAT_NUMBER << 2, XDR_DECODE);

    for (i = 0; i < FLOAT_NUMBER; ++i) {
        if (! xdr_float(&xdr, ptr)) {
            errx(EXIT_FAILURE, "error: xdr_float");
        }
        ++ptr;
    }

    xdr_destroy(&xdr);

    return buffer;
}

// -------------------------------------------------------------------------- //
// printFloatBuffer
// -------------------------------------------------------------------------- //
unsigned int printFloatBuffer(char* buffer)
{
    float* ptr = (float*) buffer;
    char* str = buffer + (FLOAT_NUMBER << 2);
    unsigned int i;
    int n;
    char ref[FLOAT_STRSIZE];
    char cur[FLOAT_STRSIZE];
    char format[8];
    int fails = 0;
#   if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
    char winFloat[FLOAT_STRSIZE + 1];
#   endif

    sprintf(format, "%%+.%ue", FLOAT_PRECISION);
    for (i = 0; i < FLOAT_NUMBER; ++i) {
#       if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
        sprintf(winFloat, format, *ptr);
        winFloat[FLOAT_STRSIZE - 3] = winFloat[FLOAT_STRSIZE - 2];
        winFloat[FLOAT_STRSIZE - 2] = winFloat[FLOAT_STRSIZE - 1];
        winFloat[FLOAT_STRSIZE - 1] = '\0';
        sprintf(cur, "%s", winFloat);
#       else
        sprintf(cur, format, *ptr);
#       endif
        n = sprintf(ref, "%s", str) + 1;
        str += n;
        ++ptr;
        if (strcmp(cur, ref) != 0) {
            fprintf(stderr, "%s %s !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", cur, ref);
            ++fails;
        } else {
#           ifndef NDEBUG
            fprintf(stderr, "%s %s\n", cur, ref);
#           endif
        }
    }

    return fails;
}

// -------------------------------------------------------------------------- //
// getResource
// -------------------------------------------------------------------------- //
void getResource(const char* url)
{
    int sock;
    struct sockaddr_in sockaddrServer;
    ssize_t size;
    ssize_t totalSize;

    sprintf(gBuffer, "GET %s ", url);

    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        err(EXIT_FAILURE, "socket");
    }

    memset(&sockaddrServer, 0, sizeof(sockaddrServer));
    sockaddrServer.sin_family = AF_INET;
    sockaddrServer.sin_addr.s_addr = gInAddr.s_addr;
    sockaddrServer.sin_port = htons(gPort);

    if (connect(sock, (struct sockaddr*) &sockaddrServer,
                sizeof(sockaddrServer)) == -1) {
        closeSocket(sock);
        err(EXIT_FAILURE, "connect");
    }

    if (send(sock, gBuffer, strlen(gBuffer), 0) != (ssize_t) strlen(gBuffer)) {
        err(EXIT_FAILURE, "send");
    }

#   if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
    if (shutdown(sock, SD_SEND) == -1) {
        closeSocket(sock);
        err(EXIT_FAILURE, "shutdown");
    }
#   else
    if (shutdown(sock, SHUT_WR) == -1) {
        closeSocket(sock);
        err(EXIT_FAILURE, "shutdown");
    }
#   endif

    totalSize = 0;
    do {
        if ((size = recv(sock, gBuffer + totalSize,
                         BUFFER_SIZE - totalSize, 0)) == -1) {
            closeSocket(sock);
            err(EXIT_FAILURE, "recv");
        }
        fprintf(stderr, "recv size = %d\n", (int) size);
        totalSize += size;
    } while (totalSize != BUFFER_SIZE);

    closeSocket(sock);
}

// -------------------------------------------------------------------------- //
// main
// -------------------------------------------------------------------------- //
int main(int argc, char** argv)
{
    unsigned int fails;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <host> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (inet_aton(argv[1], &gInAddr) == 0) {
        errx(EXIT_FAILURE, "inet_aton");
    }
    gPort = atoi(argv[2]);

    if (sizeof(float) != 4) {
        errx(EXIT_FAILURE, "error: sizeof float");
    }

    sockApiStart();

    fprintf(stderr, "BUFFER_SIZE = %u\n", BUFFER_SIZE);

    getResource("/rand");

    sockApiStop();

    //fails = printFloatBuffer(gBuffer);
    fails = printFloatBuffer(deserializeFloatBuffer(gBuffer));
    if (fails > 0) {
        fprintf(stderr, "%u FAILURE(S) / %u\n", fails, FLOAT_NUMBER);
        return EXIT_FAILURE;
    } else {
        fprintf(stderr, "SUCCESS\n");
        return EXIT_SUCCESS;
    }
}
