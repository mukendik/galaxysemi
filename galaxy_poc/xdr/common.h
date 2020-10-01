// -------------------------------------------------------------------------- //
// common.h
// -------------------------------------------------------------------------- //
#include <unistd.h>
#include <fcntl.h>
#include <rpc/types.h>
#include <rpc/xdr.h>
#include <errno.h>

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
#include <winsock2.h>
void err(int code, const char* str)
{
    fprintf(stderr, "%s: %s\n", str, strerror(errno));
    exit(code);
}
void errx(int code, const char* str)
{
    fprintf(stderr, "%s\n", str);
    exit(code);
}
int inet_aton(const char* cp, struct in_addr* addr)
{
    addr->s_addr = inet_addr(cp);
    return (addr->s_addr == INADDR_NONE) ? 0 : 1;
}
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <err.h>
#endif

#define FLOAT_NUMBER 536U  // Optimal on linux (TCP_MSS_DEFAULT)
#define FLOAT_PRECISION 16  // Max on windows
#define FLOAT_STRSIZE (FLOAT_PRECISION + 8)  // 8 = "+d.e+00" + '\0'
#define BUFFER_SIZE (FLOAT_NUMBER * (4 + FLOAT_STRSIZE))  // 4 = sizeof(float)
char gBuffer[BUFFER_SIZE];
unsigned int gPort;

// -------------------------------------------------------------------------- //
// sockApiStart
// -------------------------------------------------------------------------- //
void sockApiStart()
{
#   if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) == SOCKET_ERROR) {
        errx(EXIT_FAILURE, "error: WSAStartup");
    }
    if (LOBYTE(wsaData.wVersion) != 2 ||
        HIBYTE(wsaData.wVersion) != 2) {
        WSACleanup();
        sprintf(gBuffer, "error: winsock verion = %d.%d",
                LOBYTE(wsaData.wVersion),
                HIBYTE(wsaData.wVersion));
        errx(EXIT_FAILURE, gBuffer);
    }
#   endif
}

// -------------------------------------------------------------------------- //
// sockApiStop
// -------------------------------------------------------------------------- //
void sockApiStop()
{
#   if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
    WSACleanup();
#   endif
}

// -------------------------------------------------------------------------- //
// closeSocket
// -------------------------------------------------------------------------- //
void closeSocket(int fd)
{
#   if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
    if (closesocket(fd) == -1) {
        err(EXIT_FAILURE, "close");
    }
#   else
    if (close(fd) == -1) {
        err(EXIT_FAILURE, "close");
    }
#   endif
}
