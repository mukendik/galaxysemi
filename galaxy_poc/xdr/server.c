// -------------------------------------------------------------------------- //
// server.c
// -------------------------------------------------------------------------- //
#include <signal.h>

#include "common.h"

/* -------------------------------------------------------------------------- */
/* serializeFloatBuffer */
/* -------------------------------------------------------------------------- */
char* serializeFloatBuffer(char* buffer)
{
    float* ptr = (float*) buffer;
    XDR xdr;
    unsigned int i;

    xdrmem_create(&xdr, buffer, FLOAT_NUMBER << 2, XDR_ENCODE);

    for (i = 0; i < FLOAT_NUMBER; ++i) {
        if (! xdr_float(&xdr, ptr)) {
            errx(EXIT_FAILURE, "error: xdr_float");
        }
        ++ptr;
    }

    xdr_destroy(&xdr);

    return buffer;
}

/* -------------------------------------------------------------------------- */
/* appendReadable */
/* -------------------------------------------------------------------------- */
char* appendReadable(char* buffer)
{
    float* ptr = (float*) buffer;
    char* str = buffer + (FLOAT_NUMBER << 2);
    unsigned int i;
    int n;
    char format[8];
#   if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
    char winFloat[FLOAT_STRSIZE + 1];
#   endif

    sprintf(format, "%%+.%ue", FLOAT_PRECISION);
    for (i = 0; i < FLOAT_NUMBER; ++i) {
#       if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
        if (snprintf(winFloat, FLOAT_STRSIZE + 1, format, (float) *ptr) >=
            FLOAT_STRSIZE + 1) {
            fprintf(stderr, "error: string overflow\n");
        }
        winFloat[FLOAT_STRSIZE - 3] = winFloat[FLOAT_STRSIZE - 2];
        winFloat[FLOAT_STRSIZE - 2] = winFloat[FLOAT_STRSIZE - 1];
        winFloat[FLOAT_STRSIZE - 1] = '\0';
        n = sprintf(str, "%s", winFloat) + 1;
#       else
        n = sprintf(str, format, *ptr) + 1;
#       endif
#       ifndef NDEBUG
        fprintf(stderr, "%s\n", str);
#       endif
        str += n;
        if (str > buffer + BUFFER_SIZE) {
            fprintf(stderr, "error: buffer overflow\n");
            return buffer;
        }
        ++ptr;
    }
    return buffer;
}

/* -------------------------------------------------------------------------- */
/* randFloatBuffer */
/* -------------------------------------------------------------------------- */
char* randFloatBuffer()
{
    uint16_t* ptr = (uint16_t*) gBuffer;
    unsigned int i;

    for (i = 0; i < (FLOAT_NUMBER << 1); ++i) {
        *ptr = rand() & 0xFFFFu;
        ++ptr;
    }
    return gBuffer;
}

/* -------------------------------------------------------------------------- */
/* sendBuffer */
/* -------------------------------------------------------------------------- */
void sendBuffer(int sock, const char* message)
{
    if (send(sock, message, BUFFER_SIZE, 0) != (ssize_t) BUFFER_SIZE) {
        err(EXIT_FAILURE, "send");
    }
}

/* -------------------------------------------------------------------------- */
/* sendMessage */
/* -------------------------------------------------------------------------- */
void sendMessage(int sock, const char* message)
{
    unsigned int len = strlen(message);

    if (send(sock, message, len, 0) != (ssize_t) len) {
        err(EXIT_FAILURE, "send");
    }
}

/* -------------------------------------------------------------------------- */
/* controlC */
/* -------------------------------------------------------------------------- */
void controlC(int sig)
{
    if (sig == SIGINT) {
        sockApiStop();
        fprintf(stderr, "ok\n");
        exit(EXIT_SUCCESS);
    }
}

// -------------------------------------------------------------------------- //
// loop
// -------------------------------------------------------------------------- //
void loop()
{
    static int sockServer;
    static int sockClient;
    static struct sockaddr_in sockaddrServer;
    static struct sockaddr_in sockaddrClient;
#   if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
    static int sizeofSockaddrClient = sizeof(sockaddrClient);
    unsigned long mode = 1;
#   else
    static unsigned int sizeofSockaddrClient = sizeof(sockaddrClient);
    int flags;
#   endif
    static enum {
        STATE0_INIT,
        STATE1_WAIT_CONN,
        STATE2_WAIT_DATA
    } serverState = STATE0_INIT;

    switch (serverState) {
    case STATE0_INIT:
        if ((sockServer = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
            err(EXIT_FAILURE, "socket");
        }

        memset(&sockaddrServer, 0, sizeof(sockaddrServer));
        sockaddrServer.sin_family = AF_INET;
        sockaddrServer.sin_addr.s_addr = htonl(INADDR_ANY);
        sockaddrServer.sin_port = htons(gPort);

        if (bind(sockServer, (struct sockaddr*) &sockaddrServer,
                 sizeof(sockaddrServer)) < 0) {
            err(EXIT_FAILURE, "bind");
        }

        if (listen(sockServer, 1) < 0) {
            err(EXIT_FAILURE, "listen");
        }

#       if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
        if (ioctlsocket(sockServer, FIONBIO, &mode) != 0) {
            closeSocket(sockServer);
            errx(EXIT_FAILURE, "error: ioctlsocket");
        }
#       else
        flags = fcntl(sockServer, F_GETFL);
        fcntl(sockServer, F_SETFL, flags | O_NONBLOCK);
#       endif

        serverState = STATE1_WAIT_CONN;

    case STATE1_WAIT_CONN:
        if ((sockClient =
                 accept(sockServer,
                        (struct sockaddr*) &sockaddrClient,
                        &sizeofSockaddrClient)) < 0) {
#           if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
            if (WSAGetLastError() == WSAEWOULDBLOCK) {
                return;
            } else {
                err(EXIT_FAILURE, "accept");
            }
#           else
            if (errno == EWOULDBLOCK) {
                return;
            } else {
                err(EXIT_FAILURE, "accept");
            }
#           endif
        }

        serverState = STATE2_WAIT_DATA;

    case STATE2_WAIT_DATA:
        *gBuffer = '\0';
        if (recv(sockClient, gBuffer, BUFFER_SIZE, 0) < 0) {
#           if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
            if (WSAGetLastError() == WSAEWOULDBLOCK) {
                return;
            } else if (WSAGetLastError() == WSAECONNRESET) {
                serverState = STATE1_WAIT_CONN;
                return;
            } else {
                err(EXIT_FAILURE, "recv");
            }
#           else
            if (errno == EWOULDBLOCK) {
                return;
            } else if (errno == ECONNRESET) {
                serverState = STATE1_WAIT_CONN;
                return;
            } else {
                err(EXIT_FAILURE, "recv");
            }
#           endif
        }

        if (strncmp(gBuffer, "GET /rand ", 10) == 0) {
            //sendBuffer(sockClient, appendReadable(randFloatBuffer()));
            sendBuffer(sockClient,
                       serializeFloatBuffer(
                           appendReadable(randFloatBuffer())));
            return;
        }

        if (strncmp(gBuffer, "GET /kill ", 10) == 0) {
            sendMessage(sockClient, "{\"content\"=\"ok\"}\n");
            closeSocket(sockClient);
            closeSocket(sockServer);
            sockApiStop();
            exit(EXIT_SUCCESS);
        }

        closeSocket(sockClient);
        serverState = STATE1_WAIT_CONN;
        return;

    default:
        errx(EXIT_FAILURE, "error: state");
    }
}

// -------------------------------------------------------------------------- //
// main
// -------------------------------------------------------------------------- //
int main(int argc, char** argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }
    gPort = atoi(argv[1]);

    if (sizeof(float) != 4) {
        errx(EXIT_FAILURE, "error: sizeof float");
    }

    sockApiStart();

    fprintf(stderr, "BUFFER_SIZE = %u\n", BUFFER_SIZE);

    if (signal(SIGINT, controlC) == SIG_ERR) {
        err(EXIT_FAILURE, "signal");
    }
    for (;;) {
        loop();
        usleep(100000);
    }

    return EXIT_SUCCESS;
}
