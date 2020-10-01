#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h> // timezone and timeval
#include <string.h> // memset
#include <string> // STL
#include <gstdl_mailer.h>
#include <gstdl_systeminfo.h> 
#include <stdio.h>

// 2800 bits of precision ?
int estimate_pi(int lPrecision)
{
    //int r[2800 + 1];
    int* r=(int*)malloc((lPrecision+1)*sizeof(int));
    int i, k;
    int b, d;
    int c = 0;

    for (i = 0; i < lPrecision; i++) {
        r[i] = 2000;
    }

    for (k = lPrecision; k > 0; k -= 14)
    {
        d = 0;

        i = k;
        for (;;) {
            d += r[i] * 10000;
            b = 2 * i - 1;

            r[i] = d % b;
            d /= b;
            i--;
            if (i == 0) break;
            d *= i;
        }
        printf("%.4d", c + d / 10000);
        c = d % 10000;
    }

    return 0;
}

int main()
{
    CGSystemInfo lSI;
    int mhz=lSI.EstimateCPUFreq();

    //jwsmtp::initNetworking(); // usefull only on windows but anyway...
    //CGSystemInfo lSI;     lSI.ReadSystemInfo();
    //char lSubjectString[1024]="";
    //sprintf(lSubjectString, "Unit test 9 on %s %d : %i MHz", lSI.m_strPlatform.c_str(), sizeof(void*)*8, mhz);

    // on our Win32 QA it usually returns ? MHz
    // on our Win64 QA it usually returns 2863 MHz
    // on our CentOS 32 QA it usually returns 2900 MHz
    // on our CentOS 64 QA it usually returns 2871 MHz
    // on our MacOX QA it usually returns 3192 MHz

    printf("%i MHz\n", mhz);

    /*
    printf("Estimating pi: ");
    estimate_pi(2800*8); // 2800
    printf("\n");
    printf("Pi according to math.h: %.24lf\n", M_PI);
    */

    if (mhz<500 || mhz>3500)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}
