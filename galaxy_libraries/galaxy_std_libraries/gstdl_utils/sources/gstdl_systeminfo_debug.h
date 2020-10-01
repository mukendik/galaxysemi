#include <stdio.h>

void GSystemInfo_Debug_StartSession();
void GSystemInfo_Debug_StopSession();
void GSystemInfo_Debug_FunctionEnter(const char* szFunctionName);
void GSystemInfo_Debug_FunctionExit(const char* szFunctionName,
                                    long lReturnCode = 9999999);
void GSystemInfo_Debug_WriteString(const char* szString);
void GSystemInfo_Debug_WriteLongVar(const char* szVarName, long lVarValue);
void GSystemInfo_Debug_WriteStringVar(const char* szVarName,
                                      const char *szVarValue);
