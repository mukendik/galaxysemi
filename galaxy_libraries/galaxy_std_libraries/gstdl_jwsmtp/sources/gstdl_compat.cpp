// This file is part of the jwSMTP library.
//
//  jwSMTP library is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  jwSMTP library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with jwSMTP library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//   http://johnwiggins.net
//   smtplib@johnwiggins.net
//

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <gstdl_utils_c.h>
#include "gstdl_compat.h"

#if defined(__unix__)
    #include <stdlib.h>
    #include <string.h>
#endif

namespace jwsmtp
{

SOCKADDR_IN::SOCKADDR_IN(const std::string& address, unsigned short port, short family /*= AF_INET*/ )
{
  ADDR.sin_port = port;
  ADDR.sin_family = family;
  //    ADDR.sin_addr.S_un.S_addr = inet_addr(address.c_str());
  //    ok = (ADDR.sin_addr.S_un.S_addr != INADDR_NONE);
  ADDR.sin_addr.s_addr = inet_addr(address.c_str());
  ok = (ADDR.sin_addr.s_addr != INADDR_NONE);
}

void SOCKADDR_IN::set_ip(const std::string& newip)
{
  //    ADDR.sin_addr.S_un.S_addr = inet_addr(newip.c_str());
  //    ok = (ADDR.sin_addr.S_un.S_addr != INADDR_NONE);
  ADDR.sin_addr.s_addr = inet_addr(newip.c_str());
  ok = (ADDR.sin_addr.s_addr != INADDR_NONE);
}




// Just a small function sending a syslog packet using jwsmtp network functions
#define SYSLOG_DGRAM_SIZE 1024
static char sDatagram[ SYSLOG_DGRAM_SIZE ]="";
int Syslog(unsigned sev, char* m)
{
    static SOCKET sSyslogSocket=0; // on windows, SOCKET is just a u_int, on linux a int
    if(sSyslogSocket==0) //
    {
        #ifdef WIN32
            //if (sSyslogSocket==(unsigned)4294967295)
              //  return -1;
        #endif

        initNetworking(); // Usefull on windows

        if (!Socket(sSyslogSocket, AF_INET, SOCK_DGRAM, 0))
        {
            //returnstring = "451 Requested action aborted: socket function error";
            //sSyslogSocket=-1; // on Windows, it is a u int so...
            sSyslogSocket=0; // Just to retry at next Syslog call ?
            return -1;
        }
        // On my Windows, sSyslogSocket is always 200...
        // On my CentOS, sSyslogSocket is always 3...
        //printf("Syslog: sSyslogSocket %d\n", (int)sSyslogSocket);
        char* target=getenv("GSTDL_SYSLOG_TARGET");
        //printf("Syslog: GSTDL_SYSLOG_TARGET = %s\n", target?target:"?");
        //char* portstr=getenv("GSTDL_SYSLOG_PORT");
        //u_short port=scanf();
        SOCKADDR_IN addr(target?target:"127.0.0.1", htons(514), AF_INET);
        if(!Connect(sSyslogSocket, addr, false, "")) // Is it really necessary for UDP ?
        {
           return -2; // connection unavailable
        }

    }

    static char *month[] = {(char*)"Jan", (char*)"Feb", (char*)"Mar", (char*)"Apr", (char*)"May", (char*)"Jun",
                            (char*)"Jul", (char*)"Aug", (char*)"Sep", (char*)"Oct", (char*)"Nov", (char*)"Dec" };

    //char lPID[256]; strcpy(lPID, ""); ut_GetPID(lPID);

    if (strlen(m)>SYSLOG_DGRAM_SIZE-25) // The header of syslog message is at least 25 chars
        return -4; // code me : truncate/resize m even if loosing the end

    time_t current_time= time(NULL); /* Obtain current time as seconds elapsed since the Epoch. */
    struct tm *tmp=localtime(&current_time);
    //int lPID=getpid();

    //short lMonthNb=tmp?tmp->tm_mon:0;
    int len = sprintf( sDatagram, "<%d>%s %2d %02d:%02d:%02d %s %s[%d]: %s",
                   sev,
                   month[tmp?tmp->tm_mon:0], // month
                   tmp?tmp->tm_mday:1, // day of the month from 1 to 31
                   tmp?tmp->tm_hour:0, // h
                   tmp?tmp->tm_min:0, // mn
                   tmp?tmp->tm_sec:4, // seconds
                   "?", // hostname
                   "GSTDL", //__argv?(__argv[0]?__argv[0]:""):"?", // identity __argv[] gives the full path of the exec...
                   getpid(), // PID
                   m // message
                   );

    if (len<0)
        return -3;

    int ret=0;
    // bool Send(int &CharsSent, SOCKET s, const char *msg, size_t len, int flags, bool bTraceEnabled, const char *szTraceFile)
    if(!Send(ret, sSyslogSocket, sDatagram, strlen(sDatagram), 0, false, ""))
    {
       //returnstring = "451 Requested action aborted: server seems to have disconnected.";
       return -5; // On Linux, the Send() failed half time even if the packet is actully really send
    }

    //printf("%d: %s\n", sev, m);

    return 0;
}

bool Connect(SOCKET sockfd, const SOCKADDR_IN& addr, bool bTraceEnabled, const char *szTraceFile) {
	bool bStatus;
	
	if(bTraceEnabled && szTraceFile)
	{
		FILE *pFile = fopen(szTraceFile, "a+");
		if(pFile)
		{
			char	szTimestamp[UT_MAX_TIMESTAMP_LEN];
			ut_GetFullTextTimeStamp(szTimestamp);
			fprintf(pFile, "%s ** CONNECT: IP=%s, Port=%hu\n", szTimestamp, inet_ntoa(addr.ADDR.sin_addr), ntohs(addr.ADDR.sin_port));
			fclose(pFile);
		}
	}
   
#ifdef WIN32
	bStatus = bool(connect(sockfd, (sockaddr*)&addr, addr.get_size()) != SOCKET_ERROR);
#else
	bStatus = bool(connect(sockfd, (sockaddr*)&addr, addr.get_size()) == 0);
#endif

	if(bTraceEnabled && szTraceFile)
	{
		FILE *pFile = fopen(szTraceFile, "a+");
		if(pFile)
		{
			char	szTimestamp[UT_MAX_TIMESTAMP_LEN];
			ut_GetFullTextTimeStamp(szTimestamp);
			fprintf(pFile, "%s ** CONNECTION status: %d\n", szTimestamp, (int)bStatus);
			fclose(pFile);
		}
	}

	return bStatus;
}

bool Socket(SOCKET& s, int /*domain*/, int type, int protocol)
{
   s = socket(AF_INET, type, protocol); // AF_INET = IPv4 Internet protocols
#ifdef WIN32
   return bool(s != INVALID_SOCKET);
#else
   return bool(s != -1);
#endif   
}

bool Send(int &CharsSent, SOCKET s, const char *msg, size_t len, int flags, bool bTraceEnabled, const char *szTraceFile)
{
	if(bTraceEnabled && szTraceFile)
	{
		FILE *pFile = fopen(szTraceFile, "a+");
		if(pFile)
		{
			char szTimestamp[UT_MAX_TIMESTAMP_LEN];
			ut_GetFullTextTimeStamp(szTimestamp);
			fprintf(pFile, "%s ** SEND: %s\n", szTimestamp, msg);
			fclose(pFile);
		}
	}
   
	CharsSent = send(s, msg, len, flags);

#ifdef WIN32
	return bool((CharsSent != SOCKET_ERROR));
#else
	return bool((CharsSent != -1));
#endif   
}

bool Recv(int &CharsRecv, SOCKET s, char *buf, size_t len, int flags, bool bTraceEnabled, const char *szTraceFile) {
	if(bTraceEnabled && szTraceFile)
	{
		FILE *pFile = fopen(szTraceFile, "a+");
		if(pFile)
		{
			char szTimestamp[UT_MAX_TIMESTAMP_LEN];
			ut_GetFullTextTimeStamp(szTimestamp);
			fprintf(pFile, "%s ** RECEIVE DATA\n", szTimestamp);
			fclose(pFile);
		}
	}

	CharsRecv = recv(s, buf, len, flags);

    if(bTraceEnabled && szTraceFile && CharsRecv!=-1) // CharsRecv!=-1 : anti crash
	{
		char *pBuffer = (char *)malloc(CharsRecv+1*sizeof(char));
		if(pBuffer)
		{
			strncpy(pBuffer, buf, CharsRecv);
			pBuffer[CharsRecv] = '\0';
			FILE *pFile = fopen(szTraceFile, "a+");
			if(pFile)
			{
				char szTimestamp[UT_MAX_TIMESTAMP_LEN];
				ut_GetFullTextTimeStamp(szTimestamp);
				fprintf(pFile, "%s ** RECEIVED: %s\n", szTimestamp, pBuffer);
				fclose(pFile);
			}
			free(pBuffer);
		}
	}

#ifdef WIN32
	return bool((CharsRecv != SOCKET_ERROR));
#else
	return bool((CharsRecv != -1));
#endif  
}

// just wrap the call to shutdown the connection on a socket
// this way I don't have to call it this way everywhere.
void Closesocket(const SOCKET& s, bool bTraceEnabled, const char *szTraceFile) {
	if(bTraceEnabled && szTraceFile)
	{
		FILE *pFile = fopen(szTraceFile, "a+");
		if(pFile)
		{
			char szTimestamp[UT_MAX_TIMESTAMP_LEN];
			ut_GetFullTextTimeStamp(szTimestamp);
			fprintf(pFile, "%s ** DISCONNECT\n", szTimestamp);
			fclose(pFile);
		}
	}

#ifdef WIN32
	closesocket(s);
#else
	close(s);
#endif
}

// This does nothing on unix.
// for windoze only, to initialise networking, snore
void initNetworking()
{
#ifdef WIN32
	class socks
	{
	public:
		bool init() {

			WORD wVersionRequested;
			WSADATA wsaData;

			wVersionRequested = MAKEWORD( 2, 0 );
			int ret = WSAStartup( wVersionRequested, &wsaData);
            if(ret)
				return false;
			initialised = true;
			return true;
		}
		bool IsInitialised() const {return initialised;}
		socks():initialised(false){init();}
		~socks()
		{
			if(initialised)
				shutdown();
		}
	private:
		void shutdown(){WSACleanup();}
		bool initialised;
	};
	static socks s;
#endif
}

} // end namespace jwsmtp

