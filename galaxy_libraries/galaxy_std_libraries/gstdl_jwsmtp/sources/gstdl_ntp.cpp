#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "gstdl_compat.h"
#include <gstdl_systeminfo.h>
#include <gstdl_utils_c.h>

//! \brief draft of function doing a ntp query on given server and returning localtime according to the NTP server
//! \brief Will be useful one day for GTL and co...
int ntp_get()
{
    // ntp test
    int	maxlen=1024; //check our buffers
    char msg[48]={010,0,0,0,0,0,0,0,0};	// the packet we send : unsigned ?
    char buf[1024]="";  //unsigned long  buf[maxlen];	// the buffer we get back

    struct protoent *proto=getprotobyname("udp");
    SOCKET s;
    bool r=jwsmtp::Socket(s, PF_INET, SOCK_DGRAM, proto->p_proto);
    if (!r)
    {
        printf("Cannot create ntp Socket\n");
        return EXIT_FAILURE;
    }

    //unsigned short port=123;
    jwsmtp::SOCKADDR_IN server_addr( "localhost", htons(123), AF_INET ); // pool.ntp.org ? 59.124.196.85 ?
    // impossible because jwsmtp::SOCKADDR_IN is asking for a IPv4 addr
    //jwsmtp::SOCKADDR_IN server_addr( std::string("pool.ntp.org"), htons(123), AF_INET ); // pool.ntp.org ?
    printf("server_addr = %s\n", (bool)server_addr?"ok":"nok");
    if (!(bool)server_addr)
    {
        printf("SOCKADDR_IN creation failed\n");
        return EXIT_FAILURE;
    }

    CGSystemInfo lSI;
    char* lTempFolder=lSI.GetGalaxySemiTempFolder();
    if (!lTempFolder)
        return EXIT_FAILURE;
    char lTraceFile[1024]="";
    sprintf(lTraceFile, "%s/ntp.txt", lTempFolder);
    printf("Connecting to ntp server...\n");
    r=jwsmtp::Connect(s, server_addr, true, lTraceFile);
    if (!r)
    {
        printf("ntp Send failed\n");
        return EXIT_FAILURE;
    }

    int CharsSent=0;
    r=jwsmtp::Send(CharsSent, s, msg, sizeof(msg), 0, true, "ntp.txt");
    if (!r)
    {
        printf("ntp Send failed\n");
        return EXIT_FAILURE;
    }

    int CharsRecv=0;
    r=jwsmtp::Recv(CharsRecv, s, buf, maxlen, 0, true, "ntp.txt");
    if (!r)
    {
        printf("ntp Recv failed: %d\n", CharsRecv);
        return EXIT_FAILURE;
    }

    printf("ntp received %d chars...\n", CharsRecv);

    // Fix Linux compilation warning: warning: dereferencing type-punned pointer will break strict-aliasing rules
    // Use time_t instead of int, and appropriate casting in below lines [BG]
    time_t	tmit=0;	// the time -- This is a time_t sort of
    /*
     * The high word of transmit time is the 10th word we get back.
     * tmit is the time in seconds, not accounting for network delays which
     * should be way less than a second if this is a local NTP server
     */
    //tmit=ntohl((time_t)(((unsigned long*)buf)[10]));	//# get transmit time
    tmit=(time_t)ntohl(((unsigned long*)buf)[10]);	//# get transmit time
    //printf("raw tmit=%ld\n",tmit);
    //printf("raw tmit in nb of years=%d\n", tmit/60/60/24/365);


    /*
     * Convert time to unix standard time: NTP is number of seconds since 0000
     * UT on 1 January 1900. unix time is seconds since 0000 UT on 1 January
     * 1970. There has been a trend to add a 2 leap seconds every 3 years.
     * Leap seconds are only an issue the last second of the month in June and
     * December if you don't try to set the clock then it can be ignored but
     * this is importaint to people who coordinate times with GPS clock sources.
     */
    tmit -= 2208988800U;
    //printf("tmit=%d\n",tmit);

    /* use unix library function to show me the local time (it takes care
     * of timezone issues for both north and south of the equator and places
     * that do Summer time/ Daylight savings time.
     */
    char* timestr=ctime(&tmit);
    printf("ntp Time: %s\n", timestr);
    return EXIT_SUCCESS;
}
