//==============================================================================
// Fake_Test.cpp (User function)
// 
//    void Fake_Test_user_init(test_function& func)
//    void Fake_Test(test_function& func)
//
//==============================================================================

#include "asl.h"
// Disable warning C4244 "conversion from 'const double' to 'float', possible loss of data"
#pragma warning (disable : 4244)

#include "Fake_Test.h"
#include <math.h>
//#define CSC_MAX_SITES 8
// !!!! User #includes and externs can be placed between the comments

// !!!!

// *************************************************************************

void Fake_Test_user_init(test_function& func)
{
	Fake_Test_params *ours;
    ours = (Fake_Test_params *)func.params;

// !!!! User initialization code below this comment (do not remove comment)

// !!!! User initialization code above this comment (do not remove comment)
}

// *************************************************************************

void Fake_Test(test_function& func)
{
	int isBaseLine=0;

    // The two lines below must be the first two in the function.
    Fake_Test_params *ours;
    ours = (Fake_Test_params *)func.params;
	int test_no = 1;
	Sfloat value;
	//float value;
	int bin_num = 20;
	int site_num=0;
	int pat_bin=1;
	srand(time(NULL));
	value = ( 45 + 10 * rand()/RAND_MAX );

	// Test 1
	START_SEQ_LOOP(site_num)
	if(isBaseLine)
		// Baseline
		value = ( 98 + 4 * rand()/RAND_MAX );
	else
		// DPAT
		value = ( 90 + 20 * rand()/RAND_MAX );
	END_SEQ_LOOP
    Su_test(func, value,POWER_UNIT , bin_num, test_no++);

	// Test 2
	START_SEQ_LOOP(site_num)
	if(isBaseLine)
		// Baseline
		value = ( 0.9 + 0.2 * rand()/RAND_MAX );
	else
		// DPAT
		value = ( 0.6 + 0.8 * rand()/RAND_MAX );
	END_SEQ_LOOP
    Su_test(func, value,POWER_UNIT , bin_num, test_no++);

	// Test 3
	START_SEQ_LOOP(site_num)
	if(isBaseLine)
		// Baseline
		value = ( 0.95 + 0.1 * rand()/RAND_MAX );
	else
		// DPAT
		value = ( 0.9 + 0.2 * rand()/RAND_MAX );
	END_SEQ_LOOP
    Su_test(func, value,POWER_UNIT , bin_num, test_no++);
	
}
