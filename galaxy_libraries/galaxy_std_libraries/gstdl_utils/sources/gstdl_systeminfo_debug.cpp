#include <time.h>
#include <string.h>

#include "gstdl_systeminfo_debug.h"

FILE *GSystemInfo_Debug_hDebugFile;
#define GSYSTEMINFO_DEBUG_FILENAME "C:\\audit_debug.txt"

void GSystemInfo_Debug_StartSession()
{
    return;

	char *szTime;
	time_t lTime;

	time(&lTime);
	szTime = ctime(&lTime);
	szTime[strlen(szTime)-1] = 0;

	GSystemInfo_Debug_hDebugFile = fopen(GSYSTEMINFO_DEBUG_FILENAME, "a+");
	if(GSystemInfo_Debug_hDebugFile)
	{
		fprintf(GSystemInfo_Debug_hDebugFile, "**** START: %s ****************************************************\n", szTime);
		fclose(GSystemInfo_Debug_hDebugFile);
	}
}

void GSystemInfo_Debug_StopSession()
{
    return;

	char *szTime;
	time_t lTime;

	time(&lTime);
	szTime = ctime(&lTime);
	szTime[strlen(szTime)-1] = 0;

	GSystemInfo_Debug_hDebugFile = fopen(GSYSTEMINFO_DEBUG_FILENAME, "a+");
	if(GSystemInfo_Debug_hDebugFile)
	{
		fprintf(GSystemInfo_Debug_hDebugFile, "**** STOP: %s *****************************************************\n", szTime);
		fclose(GSystemInfo_Debug_hDebugFile);
	}
}

void GSystemInfo_Debug_FunctionEnter(const char* szFunctionName)
{
    return;

	GSystemInfo_Debug_hDebugFile = fopen(GSYSTEMINFO_DEBUG_FILENAME, "a+");
	if(GSystemInfo_Debug_hDebugFile)
	{
		fprintf(GSystemInfo_Debug_hDebugFile, "-------------------------------------------------------------------\n");
		fprintf(GSystemInfo_Debug_hDebugFile, "Entering function %s\n", szFunctionName);
		fclose(GSystemInfo_Debug_hDebugFile);
	}
}

void GSystemInfo_Debug_FunctionExit(const char* szFunctionName,
                                    long lReturnCode)
{
    return;

	GSystemInfo_Debug_hDebugFile = fopen(GSYSTEMINFO_DEBUG_FILENAME, "a+");
	if(GSystemInfo_Debug_hDebugFile)
	{
		if(lReturnCode == 9999999)
			fprintf(GSystemInfo_Debug_hDebugFile, "Exiting function %s\n", szFunctionName);
		else
			fprintf(GSystemInfo_Debug_hDebugFile, "Exiting function %s, return code = %d\n", szFunctionName, (int)lReturnCode);
		fprintf(GSystemInfo_Debug_hDebugFile, "-------------------------------------------------------------------\n");
		fclose(GSystemInfo_Debug_hDebugFile);
	}
}

void GSystemInfo_Debug_WriteString(const char *szString)
{
    return;

	GSystemInfo_Debug_hDebugFile = fopen(GSYSTEMINFO_DEBUG_FILENAME, "a+");
	if(GSystemInfo_Debug_hDebugFile)
	{
		fprintf(GSystemInfo_Debug_hDebugFile, "%s\n", szString);
		fclose(GSystemInfo_Debug_hDebugFile);
	}
}

void GSystemInfo_Debug_WriteLongVar(const char* szVarName, long lVarValue)
{
    return;

	GSystemInfo_Debug_hDebugFile = fopen(GSYSTEMINFO_DEBUG_FILENAME, "a+");
	if(GSystemInfo_Debug_hDebugFile)
	{
		fprintf(GSystemInfo_Debug_hDebugFile, "Variable %s = %ld\n", szVarName, lVarValue);
		fclose(GSystemInfo_Debug_hDebugFile);
	}
}

void GSystemInfo_Debug_WriteStringVar(const char* szVarName,
                                      const char* szVarValue)
{
    return;

	GSystemInfo_Debug_hDebugFile = fopen(GSYSTEMINFO_DEBUG_FILENAME, "a+");
	if(GSystemInfo_Debug_hDebugFile)
	{
		fprintf(GSystemInfo_Debug_hDebugFile, "Variable %s = %s\n", szVarName, szVarValue);
		fclose(GSystemInfo_Debug_hDebugFile);
	}
}

