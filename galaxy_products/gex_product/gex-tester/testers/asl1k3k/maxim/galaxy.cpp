// Routines to implement dynamic PAT on TMT ASL testers with Galaxy
// Written by Patrick Hayes 
// $Rev:  $ $Date:  $
// $LastChangedBy: phayes $ 
// $URL: $

#include <stdio.h>
#include <stdlib.h>

#include <gtl_core_asl.h>


#pragma comment (lib, "c:/max_nt/galaxy/lib/libgtl_core.a")
// The GTL (Galaxy Tester Lib) ASL implementation is now provided in gtl_tester_asl.cpp
// No need library anymore
//#pragma comment (lib, "c:/max_nt/galaxy/lib/libgtl_tester_none.a")
#pragma comment (lib, "c:/max_nt/galaxy/lib/libgstdl.a")
#pragma comment (lib, "ws2_32.lib")
#ifdef Multisite_lib_h
#define MULTI_SITE
#endif


#ifndef CSC_MAX_SITES
#define CSC_MAX_SITES 8
#endif

#ifndef MAX_SITES
#define MAX_SITES 256
#endif

#ifdef MULTI_SITE
extern BOOL device_passed[MAX_SITES];
#endif

void galaxy_bin(test_function& func)
{
	int test_no = 1;
	int current_bin = 1;
	int new_bin = 1;
	int test_bin = 1;
	Sfloat value;

	// Test 1
	START_SEQ_LOOP(site_num)
	test_bin = 1;
	current_bin = asl_get_bin(site_num);
	// Get new bin (can be overloaded with PAT bin if PAT failures)
#ifdef _DEBUG 
	printf("**** DPAT: calling gtl_binning_lib(%d, %d)\n", site_num, current_bin);
#endif
	new_bin = (float)gtl_binning_lib(site_num, current_bin);
#ifdef _DEBUG 
	printf("**** DONE\n");
#endif
	if(new_bin != current_bin)
	{
		// PAT fail: force a fail for this test
		value = 1;
		test_bin = new_bin;
	}
	else
		// no PAT fail: force a pass for this test
		value = 0.1;
	END_SEQ_LOOP
    Su_test(func, value, POWER_UNIT , test_bin, test_no++);
}

void galaxy_data(test_function& func, unsigned int uiSiteNb, long lTestNb, char *szTestName, double lfResult)
{
#ifdef _DEBUG 
	printf("**** DPAT: calling gtl_test_lib(%d, %ld, %s, %g)\n", uiSiteNb, lTestNb, szTestName, lfResult);
#endif
	gtl_test_lib(uiSiteNb, lTestNb, szTestName, lfResult);
#ifdef _DEBUG 
	printf("**** DONE\n");
#endif
}
