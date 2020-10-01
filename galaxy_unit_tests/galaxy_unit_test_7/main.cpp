#include <time.h>
#include <sstream>
#include <istream>
#include <fstream>
#include <iostream>
#include <iomanip> // usefull for setprecision
#include <ios>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h> // chmod
#include <gstdl_compat.h> // contains namespace : cannot be included in .c file
#include <gstdl_mailer.h>
#include <gstdl_systeminfo.h>

// To be run: ut7.exe "Subject" "Message content..."

using namespace std;

#define EMAIL_TO "dev.ops@galaxysemi.com"
#define EMAIL_FROM "gexprod@galaxysemi.com"
#define EMAIL_SMTP_SERVER "mail.galaxysemi.com"

int main(int argc, char* argv[])
{
    CGSystemInfo lSI;

    // Init smtp lib
    cout<<"main: init jwsmtp network..."<<endl;
    jwsmtp::initNetworking(); // usefull only on windows but anyway...

    lSI.ReadSystemInfo();

    std::stringstream lSS; lSS<<sizeof(void*)*8;
    std::string lSubjectString=std::string("Unit test 7 on ")+lSI.m_strPlatform+" "+lSS.str(); //lSI.m_strOS;
    const char* lSubject=lSubjectString.c_str();   //"Subject";
    if (argc>1)
        lSubject=argv[1];

    const char* lMessage="This is the message";
    if (argc>2)
        lMessage=argv[2];

    cout<<"Creating a Mailer instance: subject="<< lSubject <<endl;

    jwsmtp::mailer lMailer((const char*)EMAIL_TO,
                           (const char*)EMAIL_FROM,
                           lSubject,
                           lMessage,
                           EMAIL_SMTP_SERVER, jwsmtp::mailer::SMTP_PORT, false);

    // other args are attachements
    for (int i=3; i<100; i++)
        if (argc>i && argv[i])
            lMailer.attach(std::string(argv[3]));

    cout<<"Sending email..."<<endl;
    lMailer.send();

    std::cout<<lMailer.response()<<std::endl;

    if (lMailer.response().substr(0,3)==std::string("250"))
        return EXIT_SUCCESS;

    cout << "Failed to connect to smtp server on port " << jwsmtp::mailer::SMTP_PORT << endl;
    cout << "Trying on port 587" << endl;

    jwsmtp::mailer lMailer2((const char*)EMAIL_TO,
                           (const char*)EMAIL_FROM,
                           lSubject,
                           lMessage,
                           EMAIL_SMTP_SERVER, 587, false);

    // other args are attachements
    for (int i=3; i<100; i++)
        if (argc>i && argv[i])
            lMailer2.attach(std::string(argv[3]));

    cout<<"Sending email..."<<endl;
    lMailer2.send();

    std::cout<<lMailer2.response()<<std::endl;

    if (lMailer2.response().substr(0,3)==std::string("250"))
        return EXIT_SUCCESS;

    cout<<"Send failed: "<<lMailer2.response()<<endl;
    return EXIT_FAILURE;
}
