//==============================================================================
// DynamicPAT.cpp (User function)
// 
//    void DynamicPAT_user_init(test_function& func)
//    void DynamicPAT(test_function& func)
//
//==============================================================================

#include "asl.h"
// Disable warning C4244 "conversion from 'const double' to 'float', possible loss of data"
#pragma warning (disable : 4244)

#include "DynamicPAT.h"

// !!!! User #includes and externs can be placed between the comments

// !!!!

// *************************************************************************

void DynamicPAT_user_init(test_function& func)
{
	DynamicPAT_params *ours;
    ours = (DynamicPAT_params *)func.params;

// !!!! User initialization code below this comment (do not remove comment)

// !!!! User initialization code above this comment (do not remove comment)
}

// *************************************************************************

void DynamicPAT(test_function& func)
{
    // The two lines below must be the first two in the function.
    DynamicPAT_params *ours;
    ours = (DynamicPAT_params *)func.params;
	// std::cout << _MSC_VER << std::endl;
#ifdef MAXIM_DPAT 
	DPAT(func);
#endif
#ifdef GALAXY_DPAT
	galaxy_bin(func);
#endif
}

