#pragma once

/******************************************************************************
 PORTABLE ROUTINES FOR WRITING PRIVATE PROFILE STRINGS --  
******************************************************************************/

#define MAX_LINE_LENGTH    2048
int get_private_profile_int(char *section, char *entry, int def,    char *file_name);
int get_private_profile_string(char *section, char *entry, char *deflt, char *buffer, int buffer_len, char *file_name);
int write_private_profile_string(char *section, char *entry, char *buffer, char *file_name);
int delete_private_profile_section(char *section, char *file_name);