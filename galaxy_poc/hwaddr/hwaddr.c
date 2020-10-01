// -------------------------------------------------------------------------- //
// hwaddr.c
// -------------------------------------------------------------------------- //
#include <ifaddrs.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#if defined(__APPLE__) && defined(__MACH__)
# include <net/if_dl.h>
#else
# include <linux/if_arp.h>
#endif

int main()
{
    struct ifaddrs* ifap;
    struct ifaddrs* item;
    unsigned char*  ptr;
    char mac[18];

    if (getifaddrs(&ifap) != 0)
    {
        return EXIT_FAILURE;
    }

    for (item = ifap; item != NULL; item = item->ifa_next)
    {
#       if defined(__APPLE__) && defined(__MACH__)
        if (item->ifa_addr != NULL &&
            item->ifa_addr->sa_family == AF_LINK)
        {
            ptr = (unsigned char*)
                LLADDR((struct sockaddr_dl*) item->ifa_addr);
        }
        else
        {
            continue;
        }
#       else
        if (item->ifa_addr != NULL &&
            item->ifa_addr->sa_family == AF_PACKET)
        {
            ptr = (unsigned char*)
                ((struct sockaddr_ll*) item->ifa_addr)->sll_addr;
        }
        else
        {
            continue;
        }
#       endif

        if (ptr[0] == 0 && ptr[1] == 0 && ptr[2] == 0 &&
            ptr[3] == 0 && ptr[4] == 0 && ptr[5] == 0)
        {
            continue;
        }
        sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X",
                ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5]);
        printf("%s\n", mac);
    }

    freeifaddrs(ifap);
    return EXIT_SUCCESS;
}
