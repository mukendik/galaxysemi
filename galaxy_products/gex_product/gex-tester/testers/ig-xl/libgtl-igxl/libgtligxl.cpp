#include "libgtligxl.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <gtl_core.h>
#include <wtypes.h> // needed for BSTR
#include <windows.h>
#include <iostream>
#include <tchar.h>
//#include <syslog.h>


extern "C"
{
    // to stay ABI compatible with gtl 4
    // remove me in GTL 6
    extern int gtl_clear_messages_stack();
    extern int gtl_close();
    extern int gtl_endjob();
    extern int gtl_get_lib_state();
    extern int gtl_get_lib_version(int*, int*);
    extern int gtl_get_number_messages_in_stack();
    extern int gtl_get_site_state(int);
    extern int gtl_init(char *FullConfigFileName, char *szFullRecipeFileName, int MaxNumberOfActiveSites,
                             int* SitesNumbers, const int MaxMessageStackSize);
    extern int gtl_set_node_info(unsigned int StationNumber, char* HostID, char* NodeName, char* UserName,
                                 char* TesterType, char* TesterExec, char* TestJobName, char* TestJobFile,
                                 char* TestJobPath, char* TestSourceFilesPath);
    extern int gtl_set_prod_info(char* OperatorName, char* JobRevision, char* LotID, char* SublotID, char* ProductID);
    extern int gtl_pop_first_message(int* severity, char* message, int* messageID);
    extern int gtl_pop_last_message(int* severity, char* message, int* messageID);
}

/*
  Just to comply to some Windows asking for DllInstall and/or DllRegisterServer...
*/

// Instructs an in-process server to create its registry entries for all classes supported in this server module.
extern "C"
LIBGTLIGXLSHARED_EXPORT
__stdcall HRESULT DllRegisterServer()
{
    FILE* f=fopen("gtl-igxl-register.txt", "a");
    if (f)
    {
        fprintf(f, "gtl-igxl DllRegisterServer() ok.\n");
        fclose(f);
    }
    // Calling syslog/GTL_LOG here makes crash Register.exe
    //syslog(5, (char*)"DllRegisterServer...");

    // This function can return the standard return values E_OUTOFMEMORY and E_UNEXPECTED, as well as the following values.
    // SELFREG_E_TYPELIB, SELFREG_E_CLASS, E_FAIL
    return S_OK;
}

extern "C"
LIBGTLIGXLSHARED_EXPORT
__stdcall HRESULT DllUnregisterServer()
{
    //GTL_LOG(5, (char*)"DllUnregisterServer");
    FILE* f=fopen("gtl-igxl-unregister.txt", "a");
    if (f)
    {
        fprintf(f, "gtl-igxl DllUnregisterServer() ok.\n");
        fclose(f);
    }

    return S_OK;
}

LIBGTLIGXLSHARED_EXPORT
__stdcall HRESULT DllCanUnloadNow()
{
    FILE* f=fopen("gtl-igxl-CanUnloadNow.txt", "a");
    if (f)
    {
        fprintf(f, "gtl-igxl DllCanUnloadNow() ok.\n");
        fclose(f);
    }

    //GTL_LOG(5, (char*)"DllCanUnloadNow");
    return S_OK;
}

extern "C" LIBGTLIGXLSHARED_EXPORT
__stdcall HRESULT
DllInstall( BOOL /*bInstall*/, PCWSTR /*pszCmdLine*/) // _In_opt_ ?
{
    //GTL_LOG(5, (char*)"DllInstall %d %p", bInstall, pszCmdLine);
    return S_OK;
}

/*
__stdcall HRESULT DllGetClassObject(const TGUID CLSID, const TGUID IID,  Obj)
{
    return S_OK;
}
*/

/*
    Argument Types in C/C++ and VBA
    You should note the following when you compare the declarations of argument types in C/C++ and VBA.
    A VBA String is passed as a pointer to a byte-string BSTR structure when passed ByVal,
        and as a pointer to a pointer when passed ByRef.
    A VBA Variant that contains a string is passed as a pointer to a Unicode wide-character string BSTR structure when passed ByVal,
        and as a pointer to a pointer when passed ByRef.
    The VBA Integer is a 16-bit type equivalent to a signed short in C/C++.
    The VBA Long is a 32-bit type equivalent to a signed int in C/C++.
    Both VBA and C/C++ allow the definition of user-defined data types, using the Type and struct statements respectively.
    Both VBA and C/C++ support the Variant data type, defined for C/C++ in the Windows OLE/COM header files as VARIANT.
    VBA arrays are OLE SafeArrays, defined for C/C++ in the Windows OLE/COM header files as SAFEARRAY.
    The VBA Currency data type is passed as a structure of type CY, defined in the Windows header file wtypes.h, when passed ByVal,
        and as a pointer to this when passed ByRef.
    In VBA, data elements in user-defined data types are packed to 4-byte boundaries,
        whereas in Visual Studio, by default, they are packed to 8-byte boundaries.
        Therefore you must enclose the C/C++ structure definition in a #pragma pack(4) ... #pragma pack() block to avoid elements being misaligned.

    The following table illustrates the equivalent conversion between several simple data types:

    VB Type     C++ Type
    Byte        unsigned char
    Integer     short
    Long        long
    Single      float
    Double      double
    Currency	__int64

*/

/* Could be usefull one day to initialize some static variables but could be responsible of some crashes...
Libgtligxl::Libgtligxl()
{
    GTL_LOG(5, "Libgtligxl\n");
}
*/

#define STRING_SIZE 1024

// Set some production information
// to be called by the test program before gtl_init
extern "C" LIBGTLIGXLSHARED_EXPORT
__stdcall int ole_gtl_set_prod_info(BSTR* lOperatorName, BSTR* lJobRevision, BSTR* lLotID,
                              BSTR* lSublotID, BSTR* lProductID)
{
    char OperatorName[STRING_SIZE];
    char JobRevision[STRING_SIZE];
    char LotID[STRING_SIZE];
    char SublotID[STRING_SIZE];
    char ProductID[STRING_SIZE];

    TCHAR cache[STRING_SIZE];
    if (lOperatorName)
    {
        _stprintf((char*)cache, _T("%s"), (char*)(LPCTSTR)lOperatorName);
        strcpy(OperatorName, (char*)cache);
    }

    if (lJobRevision)
    {
        _stprintf((char*)cache, _T("%s"), (char*)(LPCTSTR)lJobRevision);
        strcpy(JobRevision, (char*)cache);
    }

    if (lLotID)
    {
        _stprintf((char*)cache, _T("%s"), (char*)(LPCTSTR)lLotID);
        strcpy(LotID, (char*)cache);
    }

    if (lSublotID)
    {
        _stprintf((char*)cache, _T("%s"), (char*)(LPCTSTR)lSublotID);
        strcpy(SublotID, (char*)cache);
    }

    if (lProductID)
    {
        _stprintf((char*)cache, _T("%s"), (char*)(LPCTSTR)lProductID);
        strcpy(ProductID, (char*)cache);
    }

    return gtl_set_prod_info(OperatorName, JobRevision, LotID, SublotID, ProductID);
}

extern "C" LIBGTLIGXLSHARED_EXPORT
__stdcall int ole_gtl_set_node_info(
  long lStationNumber, BSTR* lHostID_bstr, BSTR* lNodeName_bstr, BSTR* lUserName_bstr,
  BSTR* lTesterType_bstr, BSTR* lTesterExec_bstr, BSTR* lTestJobName_bstr, BSTR* lTestJobFile_bstr,
  BSTR *lTestJobPath_bstr, BSTR *lTestSourceFilesPath_bstr)
{
    char HostID[STRING_SIZE]="";
    char NodeName[STRING_SIZE]="";
    char UserName[STRING_SIZE]="";
    char TesterType[STRING_SIZE]="";
    char TesterExec[STRING_SIZE]="";
    char TestJobName[STRING_SIZE]="";
    char TestJobFile[STRING_SIZE]="";
    char TestJobPath[STRING_SIZE]="";
    char TestSourceFilesPath[STRING_SIZE]="";

    //TCHAR buffer[255];
    if (lHostID_bstr)
        _stprintf((char*)HostID, _T("%s"), (char*)(LPCTSTR)lHostID_bstr);
    if (lNodeName_bstr)
        _stprintf((char*)NodeName, _T("%s"), (char*)(LPCTSTR)lNodeName_bstr);
    if (lUserName_bstr)
        _stprintf((char*)UserName, _T("%s"), (char*)(LPCTSTR)lUserName_bstr);
    if (lTesterType_bstr)
        _stprintf((char*)TesterType, _T("%s"), (char*)(LPCTSTR)lTesterType_bstr);
    if (lTesterExec_bstr)
        _stprintf((char*)TesterExec, _T("%s"), (char*)(LPCTSTR)lTesterExec_bstr);
    if (lTestJobName_bstr)
        _stprintf((char*)TestJobName, _T("%s"), (char*)(LPCTSTR)lTestJobName_bstr);
    if (lTestJobFile_bstr)
        _stprintf((char*)TestJobFile, _T("%s"), (char*)(LPCTSTR)lTestJobFile_bstr);
    if (lTestJobPath_bstr)
        _stprintf((char*)TestJobPath, _T("%s"), (char*)(LPCTSTR)lTestJobPath_bstr);
    if (lTestSourceFilesPath_bstr)
        _stprintf((char*)TestSourceFilesPath, _T("%s"), (char*)(LPCTSTR)lTestSourceFilesPath_bstr);

    return gtl_set_node_info((unsigned)lStationNumber, HostID, NodeName, UserName,
                      TesterType, TesterExec, TestJobName, TestJobFile, TestJobPath, TestSourceFilesPath);
}

extern "C" LIBGTLIGXLSHARED_EXPORT
__stdcall int ole_gtl_get(BSTR* Key, BSTR* Value)
{
    if (!Key || !Value)
        return GTL_CORE_ERR_INVALID_PARAM_POINTER;

    TCHAR lKey[1024]=L"";

    _stprintf((char*)lKey, _T("%s"), (char*)(LPCTSTR)Key);
    //_stprintf((char*)lValue, _T("%s"), (char*)(LPCTSTR)Value);

    return gtl_get((char*)lKey, (char*)Value);
}

extern "C" LIBGTLIGXLSHARED_EXPORT
__stdcall
int ole_gtl_set(BSTR* Key, BSTR* Value)
{
    if (!Key || !Value)
        return GTL_CORE_ERR_INVALID_PARAM_POINTER;

    //TCHAR lKey[1024];
    //TCHAR lValue[1024];

    //_stprintf((char*)lKey, _T("%s"), (char*)(LPCTSTR)Key);
    //_stprintf((char*)lValue, _T("%s"), (char*)(LPCTSTR)Value);

    return gtl_set((char*)Key, (char*)Value);
}

extern "C" LIBGTLIGXLSHARED_EXPORT
__stdcall int ole_gtl_get_lib_state()
{
    return gtl_get_lib_state();
}

extern "C" LIBGTLIGXLSHARED_EXPORT
__stdcall
int ole_gtl_get_site_state(unsigned int lSiteNb)
{
    return gtl_get_site_state(lSiteNb);
}

extern "C" LIBGTLIGXLSHARED_EXPORT
__stdcall int ole_gtl_command(BSTR* lCommand)
{
    if (!lCommand)
        return GTL_CORE_ERR_INVALID_PARAM_POINTER;
    TCHAR lC[255];
    _stprintf((char*)lC, _T("%s"), (char*)(LPCTSTR)lCommand);
    return gtl_command((char*)lC);
}

extern "C" LIBGTLIGXLSHARED_EXPORT
__stdcall int ole_gtl_init(BSTR* config_bstr, BSTR* recipe_bstr,
                           unsigned int lNbSites, long* lOLESiteNumbers, unsigned int lStackSize)
{
    if (!lOLESiteNumbers)
        return -1;

    TCHAR cfn[255]=L"";
    TCHAR rfn[255]=L"";

    if (config_bstr)
      _stprintf((char*)cfn, _T("%s"), (char*)(LPCTSTR)config_bstr);
    if (recipe_bstr)
      _stprintf((char*)rfn, _T("%s"), (char*)(LPCTSTR)recipe_bstr);

    for (int j=0; j<(int)lNbSites; j++)
        GTL_LOG(5, (char*)"Site index %d is %d", j, lOLESiteNumbers[j]);

    int lSiteNumbers[256];
    for (int i=0; i<256; i++)
        lSiteNumbers[i]=(int)lOLESiteNumbers[i];

    GTL_LOG(4, (char*)"gtl_init is deprecated and will be removed in next version. Use gtl_command(\"open\") instead.", 0);
    // in GTL4.0, 1 message = 1032o
    int r=gtl_init((char*)cfn, (char*)rfn, (int)lNbSites, lSiteNumbers, lStackSize);
    GTL_LOG(5, (char*)"ole_gtl_init returned %d %s %s", r, (char*)cfn, (char*)rfn);

    return r;
}

// todo : supports args : int *pnargc, char *argv[]
extern "C" LIBGTLIGXLSHARED_EXPORT
__stdcall int ole_gtl_beginjob()
{
    GTL_LOG(4, (char*)"gtl_beginjob is now deprecated and will be removed in next version. Use gtl_command(\"beginjob\") instead.", 0);
    return gtl_beginjob();
}

extern "C" LIBGTLIGXLSHARED_EXPORT
__stdcall int ole_gtl_test(unsigned int uiSiteNb, long lTestNb, BSTR* lTestName, double lfResult)
{
    TCHAR lTestNameCS[1024]=L"";
    if (lTestName)
        _stprintf((char*)lTestNameCS, _T("%s"), (char*)(LPCTSTR)lTestName);

    //GTL_LOG(6, (char*)"ole_gtl_test_lib site %d test %d result %f %s",
           //uiSiteNb, lTestNb, lfResult, (char*)lTestNameCS);

    return gtl_test(uiSiteNb, lTestNb, (char*)lTestNameCS, lfResult);
    //if (r!=0)
      //  GTL_LOG(3, (char*)"gtl_test(%d, %d...) failed: %d", uiSiteNb, lTestNb, r, 0);
    //return r;
}

extern "C" LIBGTLIGXLSHARED_EXPORT
__stdcall int ole_gtl_mptest(unsigned int lSiteNb, long lTestNb, BSTR* lTestName, double* lTestResults,
                             int* lPinIndexes, long lArraysSize)
{
    TCHAR lTestNameCS[1024]=L"";
    if (lTestName)
        _stprintf((char*)lTestNameCS, _T("%s"), (char*)(LPCTSTR)lTestName);

    //GTL_LOG(6, (char*)"ole_gtl_test_lib site %d test %d result %f %s",
           //uiSiteNb, lTestNb, lfResult, (char*)lTestNameCS);

    return gtl_mptest(lSiteNb, lTestNb, (char*)lTestNameCS, lTestResults, lPinIndexes, lArraysSize);
    //return r;
}

// Strangely there is no int in VisualBasic : it is either short or long...
extern "C" LIBGTLIGXLSHARED_EXPORT
__stdcall int ole_gtl_binning(unsigned int uiSiteNb,
                              int nHBinning, int nSBinning, long *nHBinToAssign, long *nSBinToAssign,
                              BSTR* lPartID_bstr)
{
    if (!nHBinToAssign || !nSBinToAssign)
        return GTL_CORE_ERR_INVALID_PARAM_POINTER;

    int newhbin=(int)*nHBinToAssign;
    int newsbin=(int)*nSBinToAssign;

    //GTL_LOG(7, (char*)"ole_gtl_binning: site %d original bin=%d...", uiSiteNb, nBinning)

    TCHAR lPartID[255]=L"";
    if (lPartID_bstr)
        _stprintf((char*)lPartID, _T("%s"), (char*)(LPCTSTR)lPartID_bstr);

    int r=gtl_binning(uiSiteNb, nHBinning, nSBinning, &newhbin, &newsbin, (char*)lPartID);
    if (r!=GTL_CORE_ERR_OKAY)
    {
        //GTL_LOG(3, (char*)"ole_gtl_binning: gtl_binning failed", 0);
        return r;
    }

    *nHBinToAssign=(long)newhbin;
    *nSBinToAssign=(long)newsbin;

    return r;
}

extern "C" LIBGTLIGXLSHARED_EXPORT
__stdcall int ole_gtl_endjob()
{
    GTL_LOG(4,
      (char*)"gtl_endjob is now deprecated and will be removed in next version. Use gtl_command(\"endjob\") instead.", 0);
    return gtl_endjob();
}

extern "C" LIBGTLIGXLSHARED_EXPORT
__stdcall int ole_gtl_close()
{
    GTL_LOG(4,
      (char*)"gtl_close is now deprecated and will be removed in next version. Use gtl_command(\"close\") instead.", 0);
    //GTL_LOG(5, (char*)"ole_gtl_close", 0);
    return gtl_close();
}

extern "C" LIBGTLIGXLSHARED_EXPORT
__stdcall int ole_gtl_get_number_messages_in_stack()
{
    GTL_LOG(4,
      (char*)"gtl_get_number_messages_in_stack() is now deprecated and will be removed in next version. Use gtl_get(...) instead.", 0);
    return gtl_get_number_messages_in_stack();
}

// To modify a string from VB, is it a LPSTR or BSTR ?
// http://msdn.microsoft.com/en-us/library/aa263531%28v=vs.60%29.aspx
extern "C" LIBGTLIGXLSHARED_EXPORT
__stdcall int ole_gtl_pop_last_message(long *severity, BSTR* message, long* messageID)
{
    if (!severity || !message || !messageID)
        return GTL_CORE_ERR_INVALID_PARAM_POINTER;

    int sev=0;
    int mid=0;

    // The string given by VisualBasic is static so has been probably already used by a previous call to this method.
    // Let s clean this string first to be sure not to have dirty characters at the end.
    int lML=strlen((char*)message);
    GTL_LOG(6, (char*)"ole_gtl_pop_last_message: message string len=%d ", lML
             // SysStringLen(*message) ); // SysStringByteLen(*message)
    );
    if (lML>0)
    {
        //sprintf((char*)message, (char*)" ");
        //strncat()
        for (int i=0; i<lML; i++)
          ((char*)message)[i]='\0';
    }

    //char m[GTL_MESSAGE_STRING_SIZE]="";
    int r=gtl_pop_last_message(&sev, (char*)message, &mid);

    //strcat((char*)message, "\0");
    //char* bstr_length=(char*)message; bstr_length--; //bstr_length--;
    //GTL_LOG(6, (char*)"ole_gtl_pop_last_message: %d %s (%d chars) %d", sev, message, strlen((char*)message), messageID);

    // The only way I have found to 'clear' the dirty string given by VB (thanks MicroSoft)
    strcat((char*)message, "                                                                    ");

    /*
    wchar_t wm[GTL_MESSAGE_STRING_SIZE]=L"?";
    size_t s=mbstowcs(wm, m, GTL_MESSAGE_STRING_SIZE);
    GTL_LOG(6, (char*)"mbstowcs returned %d SysStringLen=%d", s, SysStringLen(*message) );
    */

    // let s clear the rest of the VB string
    //for (int i=strlen((char*)message)+1; i<GTL_MESSAGE_STRING_SIZE-2; i++)
      //  message[i]='\0';

    //*message=SysAllocStringLenA(m, strlen(m));

    //BSTR new_bstr=SysAllocString((LPCWSTR) wm);
    //message=&new_bstr;

    //*bstr_length=strlen((char*)message); // does not work

    //*message = SysAllocStringByteLen((LPSTR)m, strlen(m) );

    /*
    if (SysReAllocString(message, wm))
        GTL_LOG(3, (char*)"SysReAllocString failed", 0);
    */

    if (severity)
        *severity=sev;
    if (messageID)
        *messageID=mid;
    return r;
}

extern "C" LIBGTLIGXLSHARED_EXPORT
__stdcall int ole_gtl_pop_first_message(long *severity, BSTR* message, long* messageID)
{
    if (!severity || !message || !messageID)
        return GTL_CORE_ERR_INVALID_PARAM_POINTER;

    int sev=0; // severity
    int mid=0;

    // The string given by VisualBasic is static so has been probably already used by a previous call to this method.
    // Let s clean this string first to be sure not to have dirty characters at the end.
    int lML=strlen((char*)message);
    //GTL_LOG(6, (char*)"ole_gtl_pop_last_message: message string len=%d ", lML
             // SysStringLen(*message) ); // SysStringByteLen(*message)
    //);
    if (lML>0)
    {
        for (int i=0; i<lML; i++)
          ((char*)message)[i]='\0';
    }

    int r=gtl_pop_first_message(&sev, (char*)message, &mid);

    // The only way I have found to 'clear' the dirty string given by VB (thanks MicroSoft)
    strcat((char*)message, "                                                                    ");

    if (severity)
        *severity=sev;
    if (messageID)
        *messageID=mid;
    return r;
}

extern "C" LIBGTLIGXLSHARED_EXPORT
__stdcall int ole_gtl_clear_messages_stack()
{
    GTL_LOG(4,
      (char*)"gtl_clear_messages_stack() is now deprecated and will be removed in next version. Use gtl_command(...) instead.", 0);
    return gtl_clear_messages_stack();
}
