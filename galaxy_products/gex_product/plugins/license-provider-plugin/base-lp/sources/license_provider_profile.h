
/******************************************************************************
 PORTABLE ROUTINES FOR WRITING PRIVATE PROFILE STRINGS --  
******************************************************************************/

#include "license_provider_global.h"

#ifndef _LP_PROFILE_H
#define _LP_PROFILE_H
#define MAX_LINE_LENGTH    2048

#ifdef __cplusplus
extern "C"
{
#endif

LICENSE_PROVIDERSHARED_EXPORT int get_private_profile_int(const char *section, const char *entry, int def, const char *file_name);
LICENSE_PROVIDERSHARED_EXPORT int get_private_profile_string(const char *section, const char *entry, const char *deflt, char *buffer, int buffer_len, const char *file_name);
LICENSE_PROVIDERSHARED_EXPORT int write_private_profile_string(const char *section, const char *entry, const char *buffer, const char *file_name);

#ifdef __cplusplus
}
#endif


#endif

