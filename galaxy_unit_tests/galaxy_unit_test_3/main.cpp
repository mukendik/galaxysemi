#include <time.h>
#include <iostream>
#include <stdio.h>
#include <gstdl_compat.h> // contains namespace : cannot be included in .c file
#include <gstdl_mailer.h>
#include <gstdl_systeminfo.h>

using namespace std;

int main()
{
    CGSystemInfo lSI;
    char* lTempFolder=lSI.GetGalaxySemiTempFolder();
    if (!lTempFolder)
    {
        printf("Temp folder null\n");
        return EXIT_FAILURE;
    }

    printf("Temp folder = %s\n", lTempFolder?lTempFolder:"?" );

    cout << "jwsmtp unit test:" << endl;

    jwsmtp::initNetworking(); // usefull only on windows but anyway...

    char m[256]="";

    jwsmtp::Syslog(0, (char*)"sev 0 : emerg");
    jwsmtp::Syslog(1, (char*)"sev 1 : alert");
    jwsmtp::Syslog(2, (char*)"sev 2 : critical");
    jwsmtp::Syslog(3, (char*)"sev 3 : error");
    jwsmtp::Syslog(4, (char*)"sev 4 : warning");
    jwsmtp::Syslog(5, (char*)"sev 5 : notice");
    jwsmtp::Syslog(6, (char*)"sev 6 : info");
    jwsmtp::Syslog(7, (char*)"sev 7 : debug");

    // It is known that UDP packet could be lost.
    // Let s try to send a few to check what is the probability.
    // On my Windows, I have send up to 30 without any lost.
    for (int i=0; i<30; i++)
    {
        sprintf(m, "Hello syslog world (%d)", i);
        int r=jwsmtp::Syslog(5, m);
        cout << "jwsmtp::Syslog "<< i <<" returned " << r << endl;
    }

    // todo : send few emails : in another ut ?

    return EXIT_SUCCESS;
}
