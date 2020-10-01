#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <math.h>
#include "../../../gtl/include/gtl_core.h"
#include "ltx_std.h"
#include "return_codes.h"
#include "defines.h"

#ifdef R14_3
    #include "c_library.R14.3.h"
#else
    #include "c_library.h"
#endif

#include <string.h>
#include <syslog.h>

//If using cpp compiler, make sure these functions compiled as C 
// so their names do not get mangled and the linker can find them.

/* WARNING : do not declare faked or unimplemented function here 
	as CCC will try to resolve all the listed function here at lib init time. 
	So if any function is not found in the lib, the lib loading will failed in Cadence.
*/

#ifdef __cplusplus
extern "C" 
{
	int cad_gtl_get(LTX_CADENCE_C_STRUCT * p);
	int cad_gtl_set(LTX_CADENCE_C_STRUCT * p);

	int cad_gtl_get_lib_state(LTX_CADENCE_C_STRUCT * p);

    int cad_gtl_set_node_info(LTX_CADENCE_C_STRUCT * p);
    int cad_gtl_set_prod_info(LTX_CADENCE_C_STRUCT * p);

	int cad_gtl_command(LTX_CADENCE_C_STRUCT * p);

	int cad_gtl_init(LTX_CADENCE_C_STRUCT * p);

	int cad_gtl_get_number_messages_in_stack(LTX_CADENCE_C_STRUCT * p);
    int cad_gtl_pop_last_message(LTX_CADENCE_C_STRUCT * p);
    int cad_gtl_pop_first_message(LTX_CADENCE_C_STRUCT * p);

	int cad_gtl_close(LTX_CADENCE_C_STRUCT * p);

	int cad_gtl_beginjob(LTX_CADENCE_C_STRUCT * p);
	int cad_gtl_endjob(LTX_CADENCE_C_STRUCT * p);


	int cad_gtl_test(LTX_CADENCE_C_STRUCT * p);
	int cad_gtl_mptest(LTX_CADENCE_C_STRUCT * p);

	int cad_gtl_binning(LTX_CADENCE_C_STRUCT * p);

	int cad_gtl_get_site_state(LTX_CADENCE_C_STRUCT * p);

    int MyOpenLogNew(LTX_CADENCE_C_STRUCT * p);
    int MyOpenLogOld(LTX_CADENCE_C_STRUCT * p);
    int MyCloseLog(LTX_CADENCE_C_STRUCT * p);
	int gtl_test_arg(LTX_CADENCE_C_STRUCT * p);
}
#endif

extern "C" 
{
// deprecated hidden gtl functions:
extern int gtl_get_site_state(const int);
extern int gtl_set_node_info(unsigned int StationNumber, char* HostID, char* NodeName, char* UserName, char* TesterType, char* TesterExec, char* TestJobName, char* TestJobFile, char* TestJobPath, char* TestSourceFilesPath);
extern int gtl_set_prod_info(char* OperatorName, char* JobRevision, char* LotID, char* SublotID, char* ProductID);
int gtl_get_lib_state();
extern int gtl_init(char *FullConfigFileName, char *szFullRecipeFileName, int MaxNumberOfActiveSites,
               int* SitesNumbers, const int MaxMessageStackSize);
extern int gtl_pop_first_message(int* severity,char* message, int* messageID);
extern int gtl_pop_last_message(int* severity, char* message, int* messageID);
extern int gtl_get_number_messages_in_stack();
extern int gtl_close();
extern int gtl_endjob();
}

static FILE *MyLog=NULL;

int validate_fft_length(int length);
int ltx_itoa(int *int_values,  char *ascii_string, int str_len);
int ltx_atoi(char *ascii_string, int *ascii_values);
void writeToLog(char * buf);

char* set_string_param(LTX_CADENCE_C_STRUCT * p, const short param_number, const char* newstring)
{
	if (!p)
		return "LTX_CADENCE_C_STRUCT null";
	if (param_number<PAR1 || param_number>PAR20)
		return "param_number out of range";
    if((p->param[param_number].flag & LTX_IS_STRING)!=LTX_IS_STRING)
	{	
		GTL_LOG(4, "Param %d is not a string. flag=%d", param_number, p->param[param_number].flag);
		return "Param not a string";
	}
	STRING *in_cadence_string=p->param[param_number].p.strptr;
	if (in_cadence_string==0)
		return "p.strptr null";
	if (!in_cadence_string->txt)
		return "in_cadence_string->txt null";
	// cadence STRING has a max size of 256 chars	
	strncpy(in_cadence_string->txt, newstring, 253);
	int l=strlen(newstring);
	if (l>253)
		l=253;
	in_cadence_string->len=l;
	return "ok";
}

char* get_string_param_from_int_table(LTX_CADENCE_C_STRUCT * p, int param_number, char* dest)
{
	if (!p)
		return "LTX_CADENCE_C_STRUCT null";
	if (param_number<PAR1 || param_number>PAR20)
		return "param_number out of range";
    //if((p->param[param_number].flag & LTX_IS_ARRAY)!=LTX_IS_ARRAY)
    if((p->param[param_number].flag & IS_INTEGER_ARRAY)!=IS_INTEGER_ARRAY)
		return "This param is not an integer array"; // return PAR1E+NOTinteger_array;

	if (!p->param[param_number].p.iptr)
		return "p.iptr NULL";

	int *in_array=0,*out_array=0;
	unsigned long nelm=0;
	int max_len=MAXSTRLEN;     //Maximum string size in cadence
	//char in_string[MAXSTRLEN];
	int sts=STS_OK;
	//char log[2000]="";

    in_array=p->param[param_number].p.iptr;
    nelm=p->param[param_number].size/sizeof(int);
    if(nelm<MAXSTRLEN)
        max_len=nelm;

    // Convert integer array into char array
	sts=ltx_itoa(in_array, dest, max_len);
    if (sts!=STS_OK)
        return "ltx_itoa failed";

	return "ok";
}

char* set_int_table_from_string(LTX_CADENCE_C_STRUCT * p, int param_number, char* lString)
{
	if (!p)
		return "LTX_CADENCE_C_STRUCT null";
	if (param_number<PAR1 || param_number>PAR20)
		return "param_number out of range";
    //if((p->param[param_number].flag & LTX_IS_ARRAY)!=LTX_IS_ARRAY) ?
    if((p->param[param_number].flag & IS_INTEGER_ARRAY)!=IS_INTEGER_ARRAY)
		return "This param is not an integer array"; // return PAR1E+NOTinteger_array;
	if (!p->param[param_number].p.iptr)
		return "p.iptr NULL";

	unsigned lInputStringLength=strlen(lString);

	int sts=STS_OK;
	if (lInputStringLength>MAXSTRLEN)
	{
		GTL_LOG(4, "Input string too long to be contained in a Cadence string: %d vs %d", strlen(lString), MAXSTRLEN);
		return "input string too long";
	}
	
	GTL_LOG(7, "set_int_table_from_string: param array size: %d bytes, sizeof(int)=%d", 
		p->param[param_number].size, sizeof(int));
	
	unsigned lArraySize=p->param[param_number].size/sizeof(int);
	if (lArraySize<lInputStringLength)
	{
		GTL_LOG(4, "Int array too small to contain this string: %d < %d", lArraySize, lInputStringLength);
		return "input string too long for this array";
	} 
	/*
	char lTmp[1024]="";
	for (int i=0; i<25; i++)
		sprintf(lTmp, "%s%d,", lTmp, p->param[param_number].p.iptr[i]);
	GTL_LOG(5, "Before ltx_atoi: %s", lTmp);
	*/

	// ltx_atoi(char*, int*)
	sts=ltx_atoi(lString, p->param[param_number].p.iptr); // in cadence MAXSTRLEN seems to be 256...
	
	p->param[param_number].flag|=LTX_RETURN_DATA; // usefull ? yes.

	char* lGTLLL=getenv("GTL_LOGLEVEL");
	if (lGTLLL && strcmp(lGTLLL,"7")==0)
	{
		char lTmp[1024]="";
		lTmp[0]='\0';
		for (int i=0; i<lArraySize; i++)
			sprintf(lTmp, "%s%d,", lTmp, p->param[param_number].p.iptr[i] );
		GTL_LOG(7, "set_int_table_from_string: iptr=%s", lTmp);
	}

	if (sts==STS_OK)
		return "ok";
	return "ltx_atoi failed";
}

char* get_string_param(LTX_CADENCE_C_STRUCT * p, int param_number)
{
	if (!p)
		return 0;
	if (param_number<PAR1 || param_number>PAR20)
		return 0;
    if((p->param[param_number].flag & LTX_IS_STRING)!=LTX_IS_STRING)
	{	
		GTL_LOG(4, "Param %d is not a string. flag=%d", param_number, p->param[param_number].flag);
		return 0;
	}
	STRING *in_cadence_string=p->param[param_number].p.strptr;
	if (in_cadence_string==0)
		return 0;
	//GTL_LOG(7, "param %d = %s (%p) strptr=%p flag=%d", param_number, in_cadence_string->txt, in_cadence_string->txt, 
		//in_cadence_string, p->param[param_number].flag);
	return in_cadence_string->txt;
}

char* get_long_param(LTX_CADENCE_C_STRUCT * p, int param_number, long *param)
{
	if (!p)
		return "LTX_CADENCE_C_STRUCT null";
	if (param_number<PAR1 || param_number>PAR20)
		return "Param number out of range (0 to 20)";
    //if((p->param[param_number].flag & LTX_IS_UNSIGNED)!=LTX_IS_UNSIGNED) // LTX_IS_UNSIGNED ? LTX_IS_INTEGER ?
		//return "Parameter is not an unsigned";
	*param=(long)p->param[param_number].l_const;
	return "ok";
}

char* get_double_param(LTX_CADENCE_C_STRUCT * p, int param_number, double *param)
{
	if (!p)
		return "LTX_CADENCE_C_STRUCT null";
	if (param_number<PAR1 || param_number>PAR20)
		return "Param number out of range (0 to 20)";
	if (p->param[param_number].p.dptr)
		*param=*p->param[param_number].p.dptr;
	else
		*param=p->param[param_number].d_const;
	return "ok";
}

char* get_unsigned_int_param(LTX_CADENCE_C_STRUCT * p, int param_number, unsigned int *param)
{
	if (!p)
		return "LTX_CADENCE_C_STRUCT null";
	if (param_number<PAR1 || param_number>PAR20)
		return "Param number out of range (0 to 20)";
    //if((p->param[param_number].flag & LTX_IS_UNSIGNED)!=LTX_IS_UNSIGNED) // LTX_IS_UNSIGNED ? LTX_IS_INTEGER ?
		//return "Parameter is not an unsigned";
	if (p->param[param_number].p.iptr)
	{
		*param=*p->param[param_number].p.iptr;
		return "ok";
	}
	if (p->param[param_number].p.lptr)
	{
		*param=*p->param[param_number].p.lptr;
		return "ok";
	}
	if (p->param[param_number].p.wptr)
	{
		*param=*p->param[param_number].p.wptr;
		return "ok";
	}

	//GTL_LOG(5, "Param: w_const=%d l_const=%ld ", p->param[param_number].w_const, p->param[param_number].l_const);
	*param=p->param[param_number].l_const;

	return "ok";
}

char* set_int_param(LTX_CADENCE_C_STRUCT * p, int param_number, const int newvalue)
{
	if (!p)
		return "LTX_CADENCE_C_STRUCT null";
	if (param_number<PAR1 || param_number>PAR20)
		return "Param number out of range (0 to 20)";
	if (p->param[param_number].p.iptr==0)
		return "Param iptr null";
	*p->param[param_number].p.iptr=newvalue;
	return "ok";
}

char* get_int_table_param(LTX_CADENCE_C_STRUCT * p, int param_number, int** int_table)
{
	if (!p)
		return "LTX_CADENCE_C_STRUCT null";
	if (param_number<PAR1 || param_number>PAR20)
		return "Param number out of range (0 to 20)";

	GTL_LOG(6, "Int table size for param %d is %d", param_number, p->param[param_number].size);

	if (p->param[param_number].p.iptr==0)
		return "Param iptr null";

	*int_table=p->param[param_number].p.iptr;
	return "ok";
}

char* get_double_table_param(LTX_CADENCE_C_STRUCT * p, int param_number, double** table)
{
	if (!p)
		return "LTX_CADENCE_C_STRUCT null";
	if (param_number<PAR1 || param_number>PAR20)
		return "Param number out of range (0 to 20)";

	GTL_LOG(6, "Double table size for param %d is %d", param_number, p->param[param_number].size);

	if (p->param[param_number].p.dptr==0)
		return "Param iptr null";

	*table=p->param[param_number].p.dptr;
	return "ok";
}


char* get_int_param(LTX_CADENCE_C_STRUCT * p, int param_number, int *param)
{
	if (!p)
		return "LTX_CADENCE_C_STRUCT null";
	if (param_number<PAR1 || param_number>PAR20)
		return "Param number out of range (0 to 20)";
    if((p->param[param_number].flag & LTX_IS_INTEGER)!=LTX_IS_INTEGER)
	{
		GTL_LOG(4, "Param %d not an integer: flag=%d %d %ld %f %f", param_number, p->param[param_number].flag,
			p->param[param_number].w_const, p->param[param_number].l_const, 
			p->param[param_number].f_const, p->param[param_number].d_const);
		if (p->param[param_number].p.iptr)
			GTL_LOG(4, "Param %d seems to be an int: %d ", param_number, *p->param[param_number].p.iptr);
		if (p->param[param_number].p.lptr)
			GTL_LOG(4, "Param %d seems to be a long: %ld ", param_number, *p->param[param_number].p.lptr);
		if (p->param[param_number].p.sptr)
			GTL_LOG(4, "Param %d seems to be a short: %d ", param_number, *p->param[param_number].p.sptr);
		if (p->param[param_number].p.wptr)
			GTL_LOG(4, "Param %d seems to be a uns short: %d ", param_number, *p->param[param_number].p.wptr);
		if (p->param[param_number].p.fptr)
			GTL_LOG(4, "Param %d seems to be a float: %f ", param_number, *p->param[param_number].p.fptr);
		if (p->param[param_number].p.dptr)
			GTL_LOG(4, "Param %d seems to be a float: %f ", param_number, *p->param[param_number].p.dptr);
		if (p->param[param_number].p.bptr)
			GTL_LOG(4, "Param %d seems to be a bool: %d ", param_number, *p->param[param_number].p.bptr);
		if (p->param[param_number].p.ptr)
			GTL_LOG(4, "Param %d has at least a ptr on something: %p ", param_number, (p->param[param_number].p.ptr));
		if (p->param[param_number].p.strptr)
			GTL_LOG(4, "Param %d seems to be a string: %s ", param_number, (p->param[param_number].p.strptr->txt));

		return "Parameter is not an integer";
	}
	//p->param[param_number].p.iptr;
	if (p->param[param_number].p.iptr==0)
		*param=p->param[param_number].l_const; 	//return "Integer pointer null in cadence struct";
	else
		*param=*p->param[param_number].p.iptr;
	return "ok";
}



int cad_gtl_get(LTX_CADENCE_C_STRUCT * p)
{
	char lKey[1024]="";
	char* lRet=get_string_param_from_int_table(p, PAR1, lKey);

	if (strcmp(lRet, "ok")!=0)
	{
		GTL_LOG(4, "Cannot get first parameter (key): %s", lRet);
		return -1;
	}

	/* Because of CCC bug, let s play with in table strings for all params. Thanks LTXC.
	char* lKey=get_string_param(p, PAR1);
	if (!lKey)
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	*/

	char lValue[1024]="";
	int r=gtl_get(lKey, lValue);
	if (r!=0)
		return r;

	GTL_LOG(5, "get get '%s' : '%s'", lKey, lValue);

	/* // does not work perhaps because of the CCC bug
	char* lRet=set_string_param(p, PAR2, lValue);
	if (strcmp(lRet, "ok")==0)
		return 0;
	GTL_LOG(3, "get '%s' : Failed to set_string_param: '%s'", lKey, lRet);
	*/

	// let's use int array to store strings, this is the solution given by LTXC
	lRet=set_int_table_from_string(p, PAR2, lValue);
	GTL_LOG(5, "set_int_table_from_string: '%s'", lRet);
	
	if (strcmp(lRet, "ok")==0)
		return 0;

	return -1;
}

int cad_gtl_set(LTX_CADENCE_C_STRUCT * p)
{
	char* lKey=get_string_param(p, PAR1);
	if (!lKey)
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;

	char lValue[MAX_STR_LEN]="";
	char* r=get_string_param_from_int_table(p, PAR2, lValue);
	if (strcmp(r, "ok")!=0)
	{
		GTL_LOG(4, r, 0);
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}

	/* Bug CCC : only the first param is reachable as a string
	char* lValue=get_string_param(p, PAR2);
	if (!lValue)
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	*/

	return gtl_set(lKey, lValue);
}

int cad_gtl_command(LTX_CADENCE_C_STRUCT * p)
{
	char* lCommand=get_string_param(p, PAR1);
	if (!lCommand)
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	return gtl_command(lCommand);
}

int cad_gtl_get_site_state(LTX_CADENCE_C_STRUCT * p)
{
	int sitenb=-1;
	char* r=get_int_param(p, PAR1, &sitenb);
	if (r!="ok")
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	char site_nb_string[256]="?";
	sprintf(site_nb_string, "%d", sitenb);
	gtl_set(GTL_KEY_DESIRED_SITE_NB, site_nb_string);
	char site_state_string[1024]="?";
	gtl_get(GTL_KEY_SITE_STATE, site_state_string);
	int site_state=-1;
	sscanf(site_state_string, "%d", &site_state);
	return site_state;
	//return gtl_get_site_state(sitenb);
}

int cad_gtl_set_node_info(LTX_CADENCE_C_STRUCT * p)
{
	unsigned int StationNumber=0;
	char* r=get_unsigned_int_param(p, PAR1, &StationNumber);
	if (strcmp(r, "ok")!=0)
	{
		GTL_LOG(4, r, 0);
		return  GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}

	char hostid[MAX_STR_LEN]="";
	r=get_string_param_from_int_table(p, PAR2, hostid);
	if (strcmp(r, "ok")!=0)
	{
		GTL_LOG(4, r, 0);
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}

	char nodename[MAX_STR_LEN]="";
	r=get_string_param_from_int_table(p, PAR3, nodename);
	if (strcmp(r, "ok")!=0)
	{
		GTL_LOG(4, r, 0);
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}

	char username[MAX_STR_LEN]="";
	r=get_string_param_from_int_table(p, PAR4, username);
	if (strcmp(r, "ok")!=0)
	{
		GTL_LOG(4, r, 0);
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}

	char testertype[MAX_STR_LEN]="";
	r=get_string_param_from_int_table(p, PAR5, testertype);
	if (strcmp(r, "ok")!=0)
	{
		GTL_LOG(4, r, 0);
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}

	char testerexec[MAX_STR_LEN]="";
	r=get_string_param_from_int_table(p, PAR6, testerexec);
	if (strcmp(r, "ok")!=0)
	{
		GTL_LOG(4, r, 0);
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}

	char testjobname[MAX_STR_LEN]="";
	r=get_string_param_from_int_table(p, PAR7, testjobname);
	if (strcmp(r, "ok")!=0)
	{
		GTL_LOG(4, r, 0);
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}

	char testjobfile[MAX_STR_LEN]="";
	r=get_string_param_from_int_table(p, PAR8, testjobfile);
	if (strcmp(r, "ok")!=0)
	{
		//GTL_LOG(4, r, 0);
		//return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}

	char testjobpath[MAX_STR_LEN]="";
	r=get_string_param_from_int_table(p, PAR9, testjobpath);
	if (strcmp(r, "ok")!=0)
	{
		//GTL_LOG(4, r, 0);
		//return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}

	char TestSourceFilesPath[MAX_STR_LEN]="";
	r=get_string_param_from_int_table(p, PAR10, testjobpath);
	if (strcmp(r, "ok")!=0)
	{
		//GTL_LOG(4, r, 0);
		//return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}

	return gtl_set_node_info(StationNumber, hostid, nodename, username, testertype, 
		testerexec, testjobname, testjobfile, testjobpath, TestSourceFilesPath);
}

int cad_gtl_set_prod_info(LTX_CADENCE_C_STRUCT * p)
{
	char OperatorName[MAX_STR_LEN]="";
	char *r=get_string_param_from_int_table(p, PAR1, OperatorName);
	if (strcmp(r, "ok")!=0)
	{
		GTL_LOG(4, r, 0);
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}

	char JobRevision[MAX_STR_LEN]="";
	r=get_string_param_from_int_table(p, PAR2, JobRevision);
	if (strcmp(r, "ok")!=0)
	{
		GTL_LOG(4, r, 0);
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}

	char LotID[MAX_STR_LEN]="";
	r=get_string_param_from_int_table(p, PAR3, LotID);
	if (strcmp(r, "ok")!=0)
	{
		GTL_LOG(4, r, 0);
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}

	char SublotID[MAX_STR_LEN]="";
	r=get_string_param_from_int_table(p, PAR4, SublotID);
	if (strcmp(r, "ok")!=0)
	{
		GTL_LOG(4, r, 0);
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}

	char ProductID[MAX_STR_LEN]="";
	r=get_string_param_from_int_table(p, PAR5, ProductID);
	if (strcmp(r, "ok")!=0)
	{
		GTL_LOG(4, r, 0);
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}

	return gtl_set_prod_info(OperatorName, JobRevision, LotID, SublotID, ProductID);
}

int cad_gtl_get_lib_state(LTX_CADENCE_C_STRUCT * p)
{
	return gtl_get_lib_state();
}

int cad_gtl_init(LTX_CADENCE_C_STRUCT * p)
{
	int sts=FUNCTION_UNAVAILABLE; // sts=STS_OK;
	// #ifdef R14_3
	STRING *in_cadence_string=0;  /*See definition of STRING in c_library.R14.3.h */
	unsigned char maxlen,len, temp_len;
	char temp_string[MAXSTRLEN], temp[1000];

	if (!p)
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;

	// because of CCC/BIF LTXC bug, we cannot use STRING. Let s use old method (array of int)
	// char* conffile=get_string_param(p, PAR1);
	char conffile[MAX_STR_LEN]="";
	char* r=get_string_param_from_int_table(p, PAR1, conffile);
	if (strcmp(r, "ok")!=0)
	{
		GTL_LOG(4, r,0);
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}
	GTL_LOG(5, "cad_gtl_init: conf file: %s", conffile?conffile:"NULL");

	//char* recipe="unit_test_recipe.csv";	//get_string_param(p, PAR2);
	char recipe[MAX_STR_LEN]="";
	r=get_string_param_from_int_table(p, PAR2, recipe);
	if (strcmp(r, "ok")!=0)
	{
		GTL_LOG(4, r, 0);
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}
	GTL_LOG(5, "cad_gtl_init: recipe: %s",  recipe?recipe:"NULL");

	//GTL_LOG(5, "cad_gtl_init: third: %s",  third?third:"NULL");

	int nbsites=-1; //	MaxNumberOfActiveSites
	r=get_int_param(p, PAR3, &nbsites);
	if (strcmp(r, "ok")!=0)
	{
		GTL_LOG(3, r, 0);
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}
	GTL_LOG(5, "cad_gtl_init: nb sites: %d",  nbsites);
	if (nbsites<1 || nbsites >255)
		return GTL_CORE_ERR_INVALID_NUM_OF_SITES;
	
	// checkme : retrieve site_numbers table
	int* lSiteNumbers=0;
	int sn[8]={ 0, 1, 2, 3, 4, 5, 6, 7 };
	r=get_int_table_param(p, PAR4, &lSiteNumbers);
	if (strcmp(r, "ok")==0)
	{
		GTL_LOG(5, "SiteNumbers starts with %d", lSiteNumbers[0]);
	}
	else
	{
		GTL_LOG(3, "Failed to retrieve SiteNumbers param: %s", r);
		lSiteNumbers=sn;
		return GTL_CORE_ERR_INVALID_NUM_OF_SITES;
	}

	int stacksize=-1;
	r=get_int_param(p, PAR5, &stacksize);
	if (strcmp(r, "ok")!=0)
	{
		GTL_LOG(3, r, 0);
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}
	GTL_LOG(5, "cad_gtl_init: stacksize: %d",  stacksize);

	sts=gtl_init(conffile, recipe, nbsites, lSiteNumbers, stacksize);
	writeToLog("gtl_init called.");
	GTL_LOG(5, "gtl_init returned %d", sts);
	return sts;
}

int cad_gtl_pop_first_message(LTX_CADENCE_C_STRUCT * p)
{
	int severity=-999;
	char message[GTL_MESSAGE_STRING_SIZE]="";
	int messageID=0;

	int r=gtl_pop_first_message(&severity, message, &messageID);
	if (r!=0)
		return r;

	char* res=set_int_param(p, PAR1, severity);
	if (strcmp(res,"ok")!=0)
	{	
		GTL_LOG(4, res, 0);
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}

	res=set_string_param(p, PAR2, message);
	if (strcmp(res,"ok")!=0)
	{
		GTL_LOG(4, res, 0);
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}

	res=set_int_param(p, PAR3, messageID);
	if (strcmp(res,"ok")!=0)
	{
		GTL_LOG(4, res, 0);
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}
	return 0;
}

int cad_gtl_pop_last_message(LTX_CADENCE_C_STRUCT * p)
{
	int severity=-999;
	char message[GTL_MESSAGE_STRING_SIZE]="";
	int messageID=0;

	int r=gtl_pop_last_message(&severity, message, &messageID);
	if (r!=0)
		return r;

	char* res=set_int_param(p, PAR1, severity);
	if (strcmp(res,"ok")!=0)
	{	
		GTL_LOG(4, res, 0);
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}

	res=set_string_param(p, PAR2, message);
	if (strcmp(res,"ok")!=0)
	{
		GTL_LOG(4, res, 0);
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}

	res=set_int_param(p, PAR3, messageID);
	if (strcmp(res,"ok")!=0)
	{
		GTL_LOG(4, res, 0);
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}
	return 0;
}

int cad_gtl_get_number_messages_in_stack(LTX_CADENCE_C_STRUCT * p)
{
	return gtl_get_number_messages_in_stack();
}

int cad_gtl_close(LTX_CADENCE_C_STRUCT * p)
{
	return gtl_close();
}

int cad_gtl_beginjob(LTX_CADENCE_C_STRUCT * p)
{
	return gtl_beginjob();
}

int cad_gtl_test(LTX_CADENCE_C_STRUCT * p)
{
    //int  gtl_test(unsigned int uiSiteNb, long lTestNb, char *szTestName, double lfResult);
	unsigned int sitenb=0;
	char* r=get_unsigned_int_param(p, PAR1, &sitenb);
	if (strcmp(r,"ok")!=0)
	{
		GTL_LOG(4, r, 0);
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}
	long testnb=0;
	r=get_long_param(p, PAR2, &testnb);
	if (strcmp(r,"ok")!=0)
	{
		GTL_LOG(4, r, 0);
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}
	char* testname=get_string_param(p, PAR3);
	if (!testname)
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;

	double testvalue=-1.;
	r=get_double_param(p, PAR4, &testvalue);
	if (strcmp(r, "ok")!=0)
	{
		GTL_LOG(4, r, 0);
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}

	GTL_LOG(6, "cad_gtl_test %d %ld %s %f", sitenb, testnb, testname?testname:0, testvalue);

	return gtl_test(sitenb, testnb, testname, testvalue);
}

int cad_gtl_mptest(LTX_CADENCE_C_STRUCT * p)
{
    //int  gtl_mptest(unsigned int uiSiteNb, long lTestNb, char *szTestName, double* lfResults, int* pinindexes, int numofpins);
	unsigned int sitenb=0;
	char* r=get_unsigned_int_param(p, PAR1, &sitenb);
	if (strcmp(r,"ok")!=0)
	{
		GTL_LOG(4, r, 0);
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}
	long testnb=0;
	r=get_long_param(p, PAR2, &testnb);
	if (strcmp(r,"ok")!=0)
	{
		GTL_LOG(4, r, 0);
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}
	char* testname=get_string_param(p, PAR3);
	if (!testname)
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;

	double* testvalues=0;
	r=get_double_table_param(p, PAR4, &testvalues);
	if (strcmp(r, "ok")!=0)
	{
		GTL_LOG(4, r, 0);
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}

	int* pinindexes=0;
	r=get_int_table_param(p, PAR5, &pinindexes);
	if (strcmp(r, "ok")!=0)
	{
		GTL_LOG(4, r, 0);
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}

	int numofpins=0;
	r=get_int_param(p, PAR6, &numofpins);
	if (strcmp(r, "ok")!=0)
	{
		GTL_LOG(4, r, 0);
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}

	GTL_LOG(6, "cad_gtl_mptest %d %ld %s %d", sitenb, testnb, testname?testname:0, numofpins);

	return gtl_mptest(sitenb, testnb, testname, testvalues, pinindexes, numofpins);
}

int cad_gtl_binning(LTX_CADENCE_C_STRUCT * p)
{
	//int  gtl_binning(unsigned int uiSiteNb, 
	// int nHBinning, int nSBinning,
	// int* nNewHBinning, int* nNewSBinning, 
	// const char* lPartID);
	unsigned int uiSiteNb=0;
	char* r=get_unsigned_int_param(p, PAR1, &uiSiteNb);
	if (strcmp(r, "ok")!=0)
	{
		GTL_LOG(4, r, 0);
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}
	// H bin
	int nHBinning=0;
	r=get_int_param(p, PAR2, &nHBinning);
	if (strcmp(r,"ok")!=0)
	{
		GTL_LOG(4, r, 0);
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}
	int nNewHBinning=nHBinning;
	// S bin
	int nSBinning=0;
	r=get_int_param(p, PAR3, &nSBinning);
	if (strcmp(r,"ok")!=0)
	{
		GTL_LOG(4, r, 0);
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}
	int nNewSBinning=nSBinning;
	// Part ID
	char* partid=get_string_param(p, PAR6); // can be null

	int res=gtl_binning(uiSiteNb, nHBinning, nSBinning, 
		&nNewHBinning, &nNewSBinning, partid);

	GTL_LOG(6, "cad_gtl_binning on site %d HBin %d newHBin %d on part %s returned %d", 
		uiSiteNb, nHBinning, nNewHBinning, partid?partid:"?", res);

	// should be unusefull but anyway
	/*
	p->param[PAR4].l_const=nNewHBinning;
	p->param[PAR4].w_const=nNewHBinning;
	p->param[PAR4].data.i_data=nNewHBinning;
	p->param[PAR4].data.s_data=nNewHBinning;
	p->param[PAR4].data.w_data=nNewHBinning;
	p->param[PAR4].data.lw_data=nNewHBinning;
	*/
	if (p->param[PAR4].p.iptr)
	{
		p->param[PAR4].p.iptr[0]=nNewHBinning; // the good one !
	}
	else
	{
		GTL_LOG(3, "Cannot set the value of the CADENCE param PAR4: iptr null", 0);
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}
	p->param[PAR4].flag|=LTX_RETURN_DATA;

	if (p->param[PAR5].p.iptr)
	{
		p->param[PAR5].p.iptr[0]=nNewSBinning; // the good one !
	}
	else
	{
		GTL_LOG(3, "Cannot set the value of the CADENCE param PAR5: iptr null", 0);
		return GTL_CORE_ERR_INVALID_PARAM_POINTER;
	}
	p->param[PAR5].flag|=LTX_RETURN_DATA;

	return res;
}

int cad_gtl_endjob(LTX_CADENCE_C_STRUCT * p)
{
	return gtl_endjob();
}

// global variable to store valus from SetNodeInfo to GetNodeInfo calls
unsigned int NbSites=1;
/*
char HostID[STRING_SIZE];
char NodeName[STRING_SIZE];
char UserName[STRING_SIZE];
char TesterType[STRING_SIZE];
char TesterExec[STRING_SIZE];
char TestJobName[STRING_SIZE];
char TestJobFile[STRING_SIZE];
char TestJobPath[STRING_SIZE];
char TestSourceFilesPath[STRING_SIZE];
*/

/*
extern "C"
int tester_GetNodeInfo(unsigned int *puiStationNb,
                       unsigned int *puiNbSites,
                       int			*pnSiteNumbers,
                       char* szHostID,
                       char* szNodeName,
                       char* szUserName,
                       char* szTesterType,
                       char* szTesterExec,
                       char* szTestJobName,
                       char* szTestJobFile,
                       char* szTestJobPath,
                       char* szTestSourceFilesPath)
{
    //syslog(5, (char*)"tester_GetNodeInfo");

    int	nValidSites=0; // nFirstValidSite=0, nMaxSites=0, nSiteMask=0;
    unsigned int	uiSiteIndex=0; // uiSiteNb=0;

    if (szHostID)
        strcpy(szHostID, "?");
    strcpy(szNodeName, "?");
    strcpy(szUserName, "?");
    strcpy(szTesterType, "IG-XL");
    strcpy(szTesterExec, "?");
    strcpy(szTestJobName, "?");
    strcpy(szTestJobFile, "?");
    strcpy(szTestJobPath, "?");
    strcpy(szTestSourceFilesPath, "?");

    // Get station Nb
    if (puiStationNb)
        *puiStationNb = 1;

    // Get site information
    tester_GetSiteInfo(&nValidSites, &nFirstValidSite, &nMaxSites, &nSiteMask);
    *puiNbSites = (unsigned int)nValidSites;
    for(uiSiteNb=0; uiSiteNb<256; uiSiteNb++)
    {
        if(nSiteMask & ((0x01 << uiSiteNb) & 0x0ff))
            pnSiteNumbers[uiSiteIndex++] = uiSiteNb;
    }
    #ifdef _DEBUG
        printf("tester_GetNodeInfo: %d sites found\n", nValidSites);
    #endif
   
    *puiNbSites=NbSites;
    uiSiteIndex=0;
    nValidSites=1; // for the moment let s handle single site only
    for(unsigned uiSiteNb=0; uiSiteNb<256; uiSiteNb++)
    {
        //if(nSiteMask & ((0x01 << uiSiteNb) & 0x0ff))
            pnSiteNumbers[uiSiteIndex++] = uiSiteNb;
    }


    // Get host ID : a unique Id of the host : optional for the moment
    sprintf(szHostID, "?");

    // Get Node Name
    strcpy(szNodeName, "MX");

    // Get user name
    sprintf(szUserName, "?");

    // Get Tester type
    sprintf(szTesterType, "unknown");

    // Get Tester exec
    sprintf(szTesterExec, "?");

    // Get Job Name
    strcpy(szTestJobName, "unknown");

    // ProductID :

    // Get Job Path : path to the sources of the program test
    // Optional for the moment

    // path to gtl_tester.conf
    sprintf(szTestSourceFilesPath, "?");
    return 1;
}
*/

int gtl_test_arg(LTX_CADENCE_C_STRUCT * p)
{
	if (!p)
		return -1;
	
	printf("gtl_test_arg: flag=%d l_const=%d, w_const=%d, f_const=%d, d_const=%d, size=%d\n",
		p->param[PAR1].flag, 
		p->param[PAR1].l_const,
		p->param[PAR1].w_const,
		p->param[PAR1].f_const,
		p->param[PAR1].d_const,
		p->param[PAR1].size	);

	printf("gtl_test_arg: data f=%f d=%f i=%d s=%d w=%d lw=%ld\n",
		p->param[PAR1].data.f_data,
		p->param[PAR1].data.d_data,
		p->param[PAR1].data.i_data,
		p->param[PAR1].data.s_data,
		p->param[PAR1].data.w_data,
		p->param[PAR1].data.lw_data
	);

	if (p->param[PAR1].p.iptr)
	{	
		printf("gtl_test_arg: iptr=%d\n", p->param[PAR1].p.iptr[0]);
		p->param[PAR1].p.iptr[0]=69;
	}

	p->param[PAR1].l_const=69;
	p->param[PAR1].w_const=69;
	p->param[PAR1].f_const=69;
	p->param[PAR1].d_const=69;
	p->param[PAR1].data.i_data=69;
	p->param[PAR1].data.f_data=69;
	p->param[PAR1].data.d_data=69;
	p->param[PAR1].data.s_data=69;
	p->param[PAR1].data.w_data=69;
	p->param[PAR1].data.lw_data=69;

	p->param[PAR1].flag|=LTX_RETURN_DATA;

	return STS_OK;
}

/*
int MyMakeRamp(LTX_CADENCE_C_STRUCT * p)
{
float *out_array, offset,max,delta;
unsigned long nelm;
int i,sts=STS_OK;

    if((p->param[PAR1].flag & FLOAT_SCALAR_BITS)!=IS_FLOAT_SCALAR)
        return PAR1E+NOTfloat_scalar;

    //Following is becasue of different data structure between R14.3 and 
    //everything preceding it

    #ifdef R14_3
        offset=p->param[PAR1].data.f_data;
    #else
        offset=p->param[PAR1].f_const;
    #endif

    if((p->param[PAR2].flag & FLOAT_SCALAR_BITS)!=IS_FLOAT_SCALAR)
        return PAR2E+NOTfloat_scalar;

    #ifdef R14_3
        max=p->param[PAR2].data.f_data;
    #else
        max=p->param[PAR2].f_const;
    #endif

    if((p->param[PAR3].flag & IS_FLOAT_ARRAY)!=IS_FLOAT_ARRAY)
        return PAR3E+NOTfloat_array;
    nelm=p->param[PAR3].size/sizeof(float);
    out_array=p->param[PAR3].p.fptr;

    delta=(max-offset)/nelm;
 
    for(i=0; i<nelm; i++)
        out_array[i]=offset+(i*delta);
    
    p->param[PAR3].flag|=LTX_RETURN_DATA;
        
    return sts;
}
*/

/*
int MyGenSin(LTX_CADENCE_C_STRUCT * p)
{
float *ramp_array,*out_array, delta;
unsigned long nelm;
int i,sts=STS_OK;

    if((p->param[PAR1].flag & IS_FLOAT_ARRAY)!=IS_FLOAT_ARRAY)
        return PAR1E+NOTfloat_array;
    nelm=p->param[PAR1].size/sizeof(float);
    ramp_array=p->param[PAR1].p.fptr;

    if((p->param[PAR2].flag & IS_FLOAT_ARRAY)!=IS_FLOAT_ARRAY)
        return PAR2E+NOTfloat_array;
    if( (p->param[PAR2].size/sizeof(float)) !=nelm)
        return ARRAY_SIZE_MISMATCH;
    out_array=p->param[PAR2].p.fptr;

    for(i=0; i<nelm; i++)
        out_array[i]=sin(ramp_array[i]);
    
    p->param[PAR2].flag|=LTX_RETURN_DATA;
        
    return sts;
}
*/

/*
int MyInverseFFT(LTX_CADENCE_C_STRUCT * p)
{
float *in_array,*out_array, delta;
unsigned long nelm;
int i,sts=STS_OK, ret_val;

    if((p->param[PAR1].flag & IS_FLOAT_ARRAY)!=IS_FLOAT_ARRAY)
        return PAR1E+NOTfloat_array;
    nelm=p->param[PAR1].size/sizeof(float);
    ret_val=validate_fft_length(nelm);
    if(ret_val !=STS_OK)
        return BAD_FFT_LEN;
    in_array=p->param[PAR1].p.fptr;

    if((p->param[PAR2].flag & IS_FLOAT_ARRAY)!=IS_FLOAT_ARRAY)
        return PAR2E+NOTfloat_array;
    if( (p->param[PAR2].size/sizeof(float)) !=nelm)
        return ARRAY_SIZE_MISMATCH;
    out_array=p->param[PAR2].p.fptr;


    ret_val=spFFTW::RealFFT.inverseFFT(nelm,in_array,out_array);
    if(ret_val !=STS_OK)
        return FFT_ERR;

    p->param[PAR2].flag|=LTX_RETURN_DATA;

    return sts;
}
*/

/*
int MyForwardFFT(LTX_CADENCE_C_STRUCT * p)
{
float *in_array,*out_array, delta;
unsigned long nelm;
int i,sts=STS_OK, ret_val;

    if((p->param[PAR1].flag & IS_FLOAT_ARRAY)!=IS_FLOAT_ARRAY)
        return PAR1E+NOTfloat_array;
    nelm=p->param[PAR1].size/sizeof(float);
    ret_val=validate_fft_length(nelm);
    if(ret_val !=STS_OK)
        return BAD_FFT_LEN;
    in_array=p->param[PAR1].p.fptr;

    if((p->param[PAR2].flag & IS_FLOAT_ARRAY)!=IS_FLOAT_ARRAY)
        return PAR2E+NOTfloat_array;
    if( (p->param[PAR2].size/sizeof(float)) !=nelm)
        return ARRAY_SIZE_MISMATCH;
    out_array=p->param[PAR2].p.fptr;

    ret_val=spFFTW::RealFFT.forwardFFT(nelm,in_array,out_array);
    if(ret_val !=STS_OK)
        return FFT_ERR;

    p->param[PAR2].flag|=LTX_RETURN_DATA;
        
    return sts;
}
*/

/*
int MyQuickFoxOld(LTX_CADENCE_C_STRUCT * p)
{
int *in_array,*out_array;
unsigned long nelm;
int max_len=MAXSTRLEN;     //Maximum string size in cadence
char in_string[MAXSTRLEN];
int sts=STS_OK;
char log[2000];

    if((p->param[PAR1].flag & IS_INTEGER_ARRAY)!=IS_INTEGER_ARRAY)
        return PAR1E+NOTinteger_array;
    in_array=p->param[PAR1].p.iptr;
    nelm=p->param[PAR1].size/sizeof(int);
    if(nelm<MAXSTRLEN)
        max_len=nelm;

    // Convert integer array into char array
    if((sts=ltx_itoa(in_array,in_string,max_len))!=STS_OK)
        return sts;

    // Make sure input string equals "The quick brown fox " 
    if((strcmp(in_string,"The quick brown fox "))!=0)
        return INVALID_STRING;

    // Finish the sentence 
    strcat(in_string,"jumped over the lazy dog.");

    // Make sure it will all fit in integer array
    if(strlen(in_string)>nelm)
        return ARRAY_SIZE_NOT_VALID;

    sprintf(log,"Size of in_string=%d",strlen(in_string));
    writeToLog(log);

    // Convert char array back into integer array 
    ltx_atoi(in_string,in_array);

    // Tell Cadence we have added data to the integer array
    p->param[PAR1].flag|=LTX_RETURN_DATA;

    return sts;
}
*/

/*
int MyQuickFoxNew(LTX_CADENCE_C_STRUCT * p)
{
int sts=FUNCTION_UNAVAILABLE;
#ifdef R14_3
STRING *in_cadence_string;  //See definition of STRING in c_library.R14.3.h
unsigned char maxlen,len, temp_len;
char temp_string[MAXSTRLEN], temp[1000];
sts=STS_OK;

    if((p->param[PAR1].flag & LTX_IS_STRING)!=LTX_IS_STRING)
        return PAR1E+NOTstring;
    in_cadence_string=p->param[PAR1].p.strptr;

    // Get maxlen (size) of Cadence string 
    maxlen=in_cadence_string->maxlen;   

    // Get current len of Cadence string (number of valid characters in the string)  
    len=in_cadence_string->len;   

    // Make sure input Cadence string contains "The quick brown fox "
    if( (strncmp(in_cadence_string->txt,"The quick brown fox ",len)) !=0)
        return INVALID_STRING;

    // Make sure it will all fit in the Cadence string
    strcpy(temp_string,"jumped over the lazy dog.");
    temp_len=strlen(temp_string);
    if( (len+temp_len) >maxlen)
        return STRING_SIZE_ERROR;

    // Finish the sentence
    strcat(in_cadence_string->txt,temp_string);

    // Update the current string size
    in_cadence_string->len=len+temp_len;

    sprintf(temp,"From MyQuickFoxNew: maxlen=%d, original len=%d, new len=%d text=%s",maxlen,len,len+temp_len,in_cadence_string->txt);
    writeToLog(temp);

    // Tell Cadence we have added data to the Cadence string
    p->param[PAR1].flag|=LTX_RETURN_DATA;
#endif
    return sts;
}
*/

int MyOpenLogNew(LTX_CADENCE_C_STRUCT * p)
{
int sts=FUNCTION_UNAVAILABLE;
#ifdef R14_3
STRING *in_cadence_string;  /*See definition of STRING in c_library.R14.3.h */

    sts=STS_OK;
    if(MyLog)
        return LOG_FILE_ALREADY_OPEN;
    if((p->param[PAR1].flag & LTX_IS_STRING)!=LTX_IS_STRING)
        return PAR1E+NOTstring;
    in_cadence_string=p->param[PAR1].p.strptr;

    MyLog=fopen(in_cadence_string->txt,"w");
    if(!MyLog)
        return LOG_OPEN_ERR;
#endif 
    return sts;
} 
 
int MyOpenLogOld(LTX_CADENCE_C_STRUCT * p)
{
int sts=STS_OK;
int *in_array;
unsigned long nelm,len=MAXSTRLEN;
char in_string[MAXSTRLEN];

    if(MyLog)
        return LOG_FILE_ALREADY_OPEN;

    if((p->param[PAR1].flag & IS_INTEGER_ARRAY)!=IS_INTEGER_ARRAY)
        return PAR1E+NOTinteger_array;
    in_array=p->param[PAR1].p.iptr;
    nelm=p->param[PAR1].size/sizeof(int);
    if(nelm<MAXSTRLEN)
        len=nelm;

    /* Convert integer array into char array */
    if((sts=ltx_itoa(in_array,in_string,len))!=STS_OK)
        return sts;

    MyLog=fopen(in_string,"w");
    if(!MyLog)
        return LOG_OPEN_ERR;

    return sts;
} 
 
int MyCloseLog(LTX_CADENCE_C_STRUCT * p)
{
int sts;

    if(!MyLog)     //File was never opened
        return STS_OK;

    sts=fclose(MyLog);
    if(sts)
        return LOG_CLOSE_ERR;
  
    return STS_OK;
}  

/*********************************************************************
 * converts an array of ascii integer values into an ascii string 
 *******************************************************************/
int ltx_itoa(int *int_values,  char *ascii_string, int str_len)
{
    int i = 0;

    if (!str_len){
        return INVALID_STRING;
    }
    for(i=0; i<str_len; i++)
    {
        if(int_values[i]==0){
            ascii_string[i]='\0';
            return STS_OK;
        }
        else if(int_values[i]==10){
            if( (i+1)==str_len){
                return INVALID_STRING;  //No room for '\n' & '\0'
	    }
  	    else{
                ascii_string[i]='\n';
                ascii_string[i+1]='\0';
                return STS_OK;
	    }
	}
        else {
	    ascii_string[i] = (char)int_values[i];
	}
    }
    //If we get here, we went through int array and never found a
    // 0 or 10. No room left in string so error
    return INVALID_STRING;  
}

/* converts an ascii string to an array of ascii integers */
int ltx_atoi(char *ascii_string, int *ascii_values)
{
	int i = 0;
	
	do{
		ascii_values[i] = (int)ascii_string[i];
		i++;
	}while(ascii_string[i] != '\n' && ascii_string[i] != '\0');

	if(ascii_string[i] == '\n')
		ascii_values[i] = 10;
	else
		ascii_values[i] = 0;

	return 0;

}

int validate_fft_length(int length)
{
    int kk;

    for ( kk = 1; kk <= MAX_FFT_LEN; kk *= 2 )
    {
        if ( length == kk )
            return 0;
    }

    return BAD_FFT_LEN;
}

void writeToLog(char * buf)
{
char temp[2000];

    if(!MyLog)
        return;

    sprintf(temp,"%s\n",buf);  //Add CR to message
    
    /* Could add error checking if you wanted to */
    fwrite(temp,strlen(temp),1,MyLog);
    fflush(MyLog);        //Forces all messages to be written to the file immediately
}
