#include <stdarg.h>
#include <stdio.h>
#include <gtl_core.h>

#include "gtlcoredynlib.h"


/*
    // Could be usefull to initialize some static stuffs inside a function by creating a static instance of a gtlcoredynlib
    gtlcoredynlib::gtlcoredynlib()
    {
        // do here what ever you'd like to be done at init
    }
*/

// Macro to get a string of a define
#define STR2(x) STR(x)
#define STR(x) #x

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

    // Chek me : to be compatible with MSVC : should functions be declared as __cdecl or __stdcall ?
    // The IGXL GTL lib has all functions declared __stdcall so VisualBasic can successfully use it....
    // It seems __cdecl is the best in order for Visual to use it.
    // http://www.mingw.org/wiki/MSVC_and_MinGW_DLLs
    // Default mangling is none : d_gtl_init symbol will appear as : 'd_gtl_init'
    // With __cdecl : d_gtl_init
    // With __stdcall : d_gtl_init@20
    // __stdcall : Prevent from linking the exec with at least gcc win32 : undefined symbols...

#ifdef WIN32
    #define CALLTYPE __cdecl
#else
    #define CALLTYPE
#endif

GTLCOREDYNLIBSHARED_EXPORT
int CALLTYPE d_gtl_init(
        char *lFullConfigFileName, char *szFullRecipeFileName,
        int MaxNumberOfActiveSites,
             int* SitesNumbers,
             const int MaxMessageStackSize)
{
  #ifdef WIN32
    GTL_LOG(5, (char*)"d_gtl_init: CALLTYPE='%s'", STR2(CALLTYPE) );
  #endif
  return gtl_init(lFullConfigFileName, szFullRecipeFileName, MaxNumberOfActiveSites, SitesNumbers, MaxMessageStackSize);
}

GTLCOREDYNLIBSHARED_EXPORT
int CALLTYPE d_gtl_set(char* k, char* v)
{
  return gtl_set(k, v);
}

GTLCOREDYNLIBSHARED_EXPORT
int CALLTYPE d_gtl_get(char* k, char* v)
{
  return gtl_get(k, v);
}

GTLCOREDYNLIBSHARED_EXPORT
int CALLTYPE d_gtl_get_lib_state()
{
  return gtl_get_lib_state();
}

GTLCOREDYNLIBSHARED_EXPORT
int CALLTYPE
d_gtl_get_site_state(const int site_nb)
{
    return gtl_get_site_state(site_nb);
}

GTLCOREDYNLIBSHARED_EXPORT
int CALLTYPE
d_gtl_get_lib_version(int *major, int *minor)
{
    return gtl_get_lib_version(major, minor);
}

GTLCOREDYNLIBSHARED_EXPORT
int CALLTYPE d_gtl_set_node_info(
    unsigned int StationNumber, char* HostID, char* NodeName, char* UserName, char* TesterType,
    char* TesterExec, char* TestJobName, char* TestJobFile, char* TestJobPath, char* TestSourceFilesPath)
{
  return gtl_set_node_info(
              StationNumber, HostID, NodeName, UserName, TesterType, TesterExec, TestJobName, TestJobFile,
                             TestJobPath, TestSourceFilesPath);
}

// todo : split me into several gtl_set(...) calls
GTLCOREDYNLIBSHARED_EXPORT
int CALLTYPE d_gtl_set_prod_info(char* OperatorName, char* JobRevision, char* LotID, char* SublotID, char* ProductID)
{
    return gtl_set_prod_info(OperatorName, JobRevision, LotID, SublotID, ProductID);
}

GTLCOREDYNLIBSHARED_EXPORT
int CALLTYPE d_gtl_command(char *command)
{
    return gtl_command(command);
}

GTLCOREDYNLIBSHARED_EXPORT
int CALLTYPE d_gtl_beginjob()
{
    return gtl_beginjob();
}

GTLCOREDYNLIBSHARED_EXPORT
int CALLTYPE d_gtl_test(unsigned int uiSiteNb, long lTestNb, char *szTestName, double lfResult)
{
    return gtl_test(uiSiteNb, lTestNb, szTestName, lfResult);
}

GTLCOREDYNLIBSHARED_EXPORT
int CALLTYPE d_gtl_mptest(unsigned int lSiteNb, long lTestNb, char *lTestName,
                          double* lResults, int* lPinIndexes, long lArraysSize)
{
    return gtl_mptest(lSiteNb, lTestNb, lTestName, lResults, lPinIndexes, lArraysSize);
}

GTLCOREDYNLIBSHARED_EXPORT
int CALLTYPE d_gtl_log(int severity, char* function, char* file, int line_no, char* message, ...)
{
    // __VA_ARGS__ can only appear in the expansion of a C99 variadic macro

    // probably dangerous
    /*
        va_list args;
        va_start(args, message);
            vprintf(message, args);
        va_end(args);
    */

    gtl_log(severity, function, file, line_no, message ); // dangerous as message could contains %d, %s, ...
    return 0;
}

GTLCOREDYNLIBSHARED_EXPORT
int CALLTYPE d_gtl_binning(unsigned int site_nb, int original_hbin, int original_sbin, int *new_hbin, int * new_sbin,
                           const char* part_id)
{
    return gtl_binning(site_nb, original_hbin, original_sbin, new_hbin, new_sbin, part_id);
}

GTLCOREDYNLIBSHARED_EXPORT
int CALLTYPE d_gtl_set_binning(unsigned int site_nb, int hbin, int sbin, const char* part_id)
{
    return gtl_set_binning(site_nb, hbin,  sbin, part_id);
}

GTLCOREDYNLIBSHARED_EXPORT
int CALLTYPE d_gtl_endjob()
{
    return gtl_endjob();
}

// remove me : use gtl_get instead
GTLCOREDYNLIBSHARED_EXPORT
int CALLTYPE d_gtl_get_number_messages_in_stack()
{
    return gtl_get_number_messages_in_stack();
}

GTLCOREDYNLIBSHARED_EXPORT
int CALLTYPE d_gtl_pop_last_message(int *severity, char* message, int* messageID)
{
    return gtl_pop_last_message(severity, message, messageID);
}

GTLCOREDYNLIBSHARED_EXPORT
int CALLTYPE d_gtl_pop_first_message(int *severity, char* message, int* messageID)
{
    return gtl_pop_first_message(severity, message, messageID);
}

// todo : remove me in GTL 6.0
// use instead gtl_command("ClearMessageStack")
GTLCOREDYNLIBSHARED_EXPORT
int CALLTYPE d_gtl_clear_messages_stack()
{
    return gtl_clear_messages_stack();
}

GTLCOREDYNLIBSHARED_EXPORT
int CALLTYPE d_gtl_close()
{
    return gtl_close();
}

GTLCOREDYNLIBSHARED_EXPORT
int CALLTYPE d_gtl_get_spat_limits(unsigned int SiteNb, long* TestNumbers, unsigned int* Flags, double* LowLimits, double* HighLimits, unsigned int* HardBins, unsigned int* SoftBins, unsigned int ArrayTotalSize, unsigned int* ArrayFilledSize)
{
    return gtl_get_spat_limits(SiteNb, TestNumbers, Flags, LowLimits, HighLimits, HardBins, SoftBins, ArrayTotalSize, ArrayFilledSize);
}

GTLCOREDYNLIBSHARED_EXPORT
int CALLTYPE d_gtl_get_dpat_limits(unsigned int SiteNb, long* TestNumbers, unsigned int* Flags, double* LowLimits, double* HighLimits, unsigned int* HardBins, unsigned int* SoftBins, unsigned int ArrayTotalSize, unsigned int* ArrayFilledSize)
{
    return gtl_get_dpat_limits(SiteNb, TestNumbers, Flags, LowLimits, HighLimits, HardBins, SoftBins, ArrayTotalSize, ArrayFilledSize);
}

} // extern C
