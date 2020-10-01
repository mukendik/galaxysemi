//==============================================================================
// LIST.CPP
//==============================================================================
#include "asl.h"
#include "function.h"

// There are five elements in each structure for each array position
// as shown below :-
/*
typedef struct	{
		// First, a name that is displayed on the list of functions screen
	char		*name;
		// Next the development number;
	unsigned long develop_no;
		// Next, the name of the actual function.
	void far	(*address)(test_function& func);
		// Next, the parameter setup section of the function
	void far	(*setup)(test_function& func,char **function_name,char *heading_line);
		// Next, the default initialization of the parameters (and various
		// memory requirements.
	void far	(*initialize)(short entry,test_function& func,char **function_name);
		// Lastly, the size of the block of parameters
	short			sizeof_param_block;
				} test_descriptions;
*/

//{ASL_CREATE BEGIN_TEST_LIST_INIT

test_descriptions g_test_list[] =
{
	{ "NEW_FUNC",0L,NEW_FUNC,setup_NEW_FUNC,initialize_NEW_FUNC,sizeof(NEW_FUNC_params) },
	{ "Fake_Test",0L,Fake_Test,setup_Fake_Test,initialize_Fake_Test,sizeof(Fake_Test_params) },
	{ "DynamicPAT",0L,DynamicPAT,setup_DynamicPAT,initialize_DynamicPAT,sizeof(DynamicPAT_params) },
	{ NULL,NULL }
};
// ASL_CREATE END_TEST_LIST_INIT}
