/*======================================================================*/
/* Galaxy                                                               */
/* Copyright 1996-98, Softlink                                          */
/* This computer program is protected by copyrigt law                   */
/* and international treaties. Unauthorized reproduction or             */
/* distribution of this program, or any portion of it,may               */
/* result in severe civil and criminal penalties, and will be           */
/* prosecuted to the maximum extent possible under the low.             */
/*======================================================================*/

#define _UtilsModule_

/*======================================================================*/
/*========================== UTILS.C  LIBRARY ==========================*/
/*======================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#if defined __unix__ || __APPLE__&__MACH__
#include <unistd.h>
#include <pwd.h>
#include <utime.h>
#include <termios.h>
#else
#include <process.h>
#include <io.h>
#include <conio.h>
#include <sys/utime.h>
#include <direct.h>
#endif

#include "gstdl_utils_c.h"

/*======================================================================*/
/* Defines                                                              */
/*======================================================================*/

/*======================================================================*/
/* PRIVATE Variables : declaration                                      */
/*======================================================================*/

/*======================================================================*/
/* PRIVATE Functions : declaration                                      */
/*======================================================================*/

/*======================================================================*/
/* PUBLIC Functions                                                     */
/*======================================================================*/
/*======================================================================*/
/* Check if directory exists and is writable                            */
/*                                                                      */
/* Return : 1 if directory exists, 0 else                               */
/*======================================================================*/
#if defined(ANSI)
int ut_CheckDir(const char *szDir)
#else
int ut_CheckDir(szDir)
const char  *szDir;
#endif
{
#if defined __unix__ || __APPLE__&__MACH__
	if(access(szDir, F_OK) == 0)
#else
	if(access(szDir, 0) == 0)
#endif
		return 1;

	return 0;
}

/*======================================================================*/
/* Create specified directory with specified access mode (unix)         */
/*                                                                      */
/* Return : 1 if directory created, 0 else                              */
/*======================================================================*/
#if defined(ANSI)
int ut_CreateDir(char *szDir, unsigned int uiAccessMode)
{
  (void) uiAccessMode;
#else
int ut_CreateDir(szDir, uiAccessMode)
char			*szDir;
unsigned int	uiAccessMode;
{
#endif
#if defined __unix__ || __APPLE__&__MACH__
	if(mkdir(szDir, uiAccessMode))
#else
	if(_mkdir(szDir))
#endif
		return 1;

	return 0;
}

/*======================================================================*/
/* Check if file exists                                                 */
/*                                                                      */
/* Return : 1 if file exists, 0 else                                    */
/*======================================================================*/
#if defined(ANSI)
int ut_CheckFile(const char *szFile)
#else
int ut_CheckFile(szFile)
const char  *szFile;
#endif
{
    FILE  *fpFile=0;
	fpFile = fopen(szFile, "r");
	if(fpFile)
	{
		fclose(fpFile);
		return 1;
	}

	return 0;
}

/*======================================================================*/
/* Compulse a compact timestamp                                         */
/*                                                                      */
/* hhmmssMmmddyyyy                                                      */
/*                                                                      */
/* Return : current time (used for timestamp)                           */
/*======================================================================*/
#if defined(ANSI)
time_t ut_GetCompactTimeStamp(char *szTimeStamp)
#else
time_t ut_GetCompactTimeStamp(szTimeStamp)
char  *szTimeStamp;
#endif
{
	time_t  tCurrentTime;
	char    szBuffer[UT_MAX_TIMESTAMP_LEN], szMonth[5], szDay[3], szTime[10], szYear[5];
	char    *szHH, *szMIN, *szSEC;
    unsigned int    i;

	/* Get Date and Time      */
	time( &tCurrentTime );
	strcpy(szBuffer, ctime( &tCurrentTime ));
	sscanf(szBuffer, "%*s %s %s %s %s", szMonth, szDay, szTime, szYear);
	strcat(szTime, ":");
	szHH = strtok(szTime, ":");
	szMIN = strtok(NULL, ":");
	szSEC = strtok(NULL, ":");

	/* Put Month in lowercase */
    for(i=0 ; i<(unsigned int)strlen(szMonth); i++)
		szMonth[i] = tolower(szMonth[i]);

	/* Compulse timestamp :  */
#ifndef __unix__
	/* to behave the same way as on unix, where the ctime function has no leading 0 on the day */
	if(szDay[0] == '0')
	{
		szDay[0] = szDay[1];
		szDay[1] = '\0';
	}
#endif
	if(szTimeStamp != NULL)
		sprintf(szTimeStamp, "%s%s%s%s%s%s",  szHH, szMIN, szSEC, szMonth, szDay, szYear);

	return tCurrentTime;
}

/*======================================================================*/
/* Compulse a compact timestamp for specified time_t value              */
/*                                                                      */
/* hhmmssMmmddyyyy                                                      */
/*                                                                      */
/* Return : current time (used for timestamp)                           */
/*======================================================================*/
#if defined(ANSI)
time_t ut_GetCompactTimeStamp2(char *szTimeStamp, time_t tTimestamp)
#else
time_t ut_GetCompactTimeStamp2(szTimeStamp, tTimestamp)
char	*szTimeStamp;
time_t	tTimestamp;
#endif
{
	char    szBuffer[UT_MAX_TIMESTAMP_LEN], szMonth[5], szDay[3], szTime[10], szYear[5];
	char    *szHH, *szMIN, *szSEC;
    unsigned int     i;

	strcpy(szBuffer, ctime( &tTimestamp ));
	sscanf(szBuffer, "%*s %s %s %s %s", szMonth, szDay, szTime, szYear);
	strcat(szTime, ":");
	szHH = strtok(szTime, ":");
	szMIN = strtok(NULL, ":");
	szSEC = strtok(NULL, ":");

	/* Put Month in lowercase */
    for(i=0 ; i<(unsigned int)strlen(szMonth); i++)
		szMonth[i] = tolower(szMonth[i]);

	/* Compulse timestamp :  */
#ifndef __unix__
	/* to behave the same way as on unix, where the ctime function has no leading 0 on the day */
	if(szDay[0] == '0')
	{
		szDay[0] = szDay[1];
		szDay[1] = '\0';
	}
#endif
	if(szTimeStamp != NULL)
		sprintf(szTimeStamp, "%s%s%s%s%s%s",  szHH, szMIN, szSEC, szMonth, szDay, szYear);

	return tTimestamp;
}

/*======================================================================*/
/* Compulse a complete timestamp                                        */
/*                                                                      */
/* ex: Tue Jun 01 1999 14:54:10                                         */
/*                                                                      */
/* Return : current time (used for timestamp)                           */
/*======================================================================*/
#if defined(ANSI)
time_t ut_GetFullTextTimeStamp(char *szTimeStamp)
#else
time_t ut_GetFullTextTimeStamp(szTimeStamp)
char  *szTimeStamp;
#endif
{
	time_t  tCurrentTime;
	char    szBuffer[UT_MAX_TIMESTAMP_LEN], szMonth[5], szDay[3], szTime[10], szYear[5];
	char    szDayName[5];

	/* Get Date and Time      */
	time( &tCurrentTime );
	strcpy(szBuffer, ctime( &tCurrentTime ));

	sscanf(szBuffer, "%*s %*s %*s %s", szTime);
	sscanf(szBuffer, "%s %s %s %*s %s", szDayName, szMonth, szDay, szYear);
	sprintf(szBuffer, "%s %s %s %s %s", szDayName, szMonth, szDay, szYear, szTime);

	strcpy(szTimeStamp, szBuffer);

	return tCurrentTime;
}

/*======================================================================*/
/* Compulse a complete timestamp for specified time_t value             */
/*                                                                      */
/* ex: Tue Jun 01 1999 14:54:10                                         */
/*                                                                      */
/* Return : current time (used for timestamp)                           */
/*======================================================================*/
#if defined(ANSI)
time_t ut_GetFullTextTimeStamp2(char *szTimeStamp, time_t tTimestamp)
#else
time_t ut_GetFullTextTimeStamp2(szTimeStamp, tTimestamp)
char  *szTimeStamp;
time_t	tTimestamp;
#endif
{
	char    szBuffer[UT_MAX_TIMESTAMP_LEN], szMonth[5], szDay[3], szTime[10], szYear[5];
	char    szDayName[5];

	/* Get Date and Time      */
	strcpy(szBuffer, ctime( &tTimestamp ));

	sscanf(szBuffer, "%*s %*s %*s %s", szTime);
	sscanf(szBuffer, "%s %s %s %*s %s", szDayName, szMonth, szDay, szYear);
	sprintf(szBuffer, "%s %s %s %s %s", szDayName, szMonth, szDay, szYear, szTime);

	strcpy(szTimeStamp, szBuffer);

	return tTimestamp;
}

/*======================================================================*/
/* Get PID of process and store it in a string                          */
/*                                                                      */
/* Return : Nothing                                                     */
/*======================================================================*/
#if defined(ANSI)
void ut_GetPID(char *szPID)
#else
void ut_GetPID(szPID)
char  *szPID;
#endif
{
#if defined __unix__ || __APPLE__&__MACH__
	sprintf(szPID, "%x", (int)getpid());
#else
	sprintf(szPID, "%x", _getpid());
#endif
}

/*======================================================================*/
/* Function   : ut_SplitPathName                                        */
/* Abstract   : splits a path in path file name and extension fields    */
/*              ("/home/file.txt" => "/home" + "file" + "txt")          */
/*                                                                      */
/* Parameters In  :                                                     */
/*   szFullName = ptr. on string with full name                         */
/*   szPath = ptr. on string to receive path (directory of file)        */
/*   szFileName = ptr. on string to receive file name                   */
/*   szFileExt = ptr. on string to receive file extension               */
/*                                                                      */
/* Parameters Out :                                                     */
/*                                                                      */
/* Return     : UT_ERR_OKAY                                             */
/*======================================================================*/
#if defined(ANSI)
int ut_SplitPathName(char *szFullName, char* szPath, char *szFileName, char *szFileExt)
#else
int ut_SplitPathName(szFullName, szPath, szFileName, szFileExt)
char *szFullName, *szPath, *szFileName, *szFileExt;
#endif
{
	int		iIndex;
	char  *ptChar;
#if defined __unix__ || __APPLE__&__MACH__
	char  cBadSeparator=0x5c;  /* Ascii code for '\' character */
	char  cGoodSeparator='/';
#else
	char  cBadSeparator='/';
	char  cGoodSeparator='\\';
#endif

	*szPath = '\0';
	*szFileName = '\0';
	*szFileExt = '\0';

    iIndex = (int)strlen(szFullName);
	if(iIndex < 1)
		return UT_ERR_OKAY;

	/* converts all '\\' or '/' into correct palteform char. for consistency...*/

	do
	{
		ptChar = strchr(szFullName,cBadSeparator);
		if(ptChar != NULL)
			*ptChar = cGoodSeparator;
	}
	while(ptChar != NULL);

	iIndex--;
	do
	{
        if(szFullName[(unsigned int)iIndex] == cGoodSeparator)
		{
			/* We have found the first separator character between file name & path. */
			strcpy(szPath,szFullName);
			/* Keep path string as '/dev/usr'    */
            szPath[(unsigned int)(iIndex++)] = cGoodSeparator;
            strcpy(szFileName,&szFullName[(unsigned int)iIndex]);
            szPath[(unsigned int)iIndex] = '\0';
			break;
		}
		iIndex--;
	}
	while(iIndex >= 0);

	/* Check if no path specified in string. */
	if(iIndex < 0)
		strcpy(szFileName,szFullName);

	/* Now split file name in file name and extension */
    iIndex = (int)strlen(szFileName);
	if(iIndex < 1)
		return UT_ERR_OKAY;
	do
	{
        if(szFileName[(unsigned int)iIndex] == '.')
		{
			/* We have found the file extension                   */
			/* Keep name string as 'file' and extension as 'c'    */
            strcpy(szFileExt,&szFileName[(unsigned int)iIndex]);
            szFileName[(unsigned int)iIndex] = '\0';
			return UT_ERR_OKAY;
		}
		iIndex--;
	}
	while(iIndex >= 0);

	/* Path okay, but file name may not have an extension. */
	return UT_ERR_OKAY;
}

/*======================================================================*/
/* Function   : ut_NormalizePath                                        */
/* Abstract   : Change \ to / if unix defined, and / to \ else          */
/*                                                                      */
/* Parameters In  :                                                     */
/*   szPath = string containing the path to normalize                   */
/*                                                                      */
/* Parameters Out :                                                     */
/*                                                                      */
/* Return     : UT_ERR_OKAY                                             */
/*======================================================================*/
#if defined(ANSI)
int ut_NormalizePath(char *szPath)
#else
int ut_NormalizePath(szPath)
char *szPath;
#endif
{
    unsigned int i=0;

    if(szPath == NULL)
        return UT_ERR_OKAY;

	while(szPath[i] != '\0')
	{
		/* 0x5c is ascii code for '\' */
#if defined __unix__ || __APPLE__&__MACH__
		if(szPath[i] == 0x5c) szPath[i] = '/';
#else
		if(szPath[i] == '/') szPath[i] = '\\';
#endif
		i++;
	}
	return UT_ERR_OKAY;
}

/*======================================================================*/
/* Function   : ut_RemoveDrive                                          */
/* Abstract   : Remove drive information from path (c:\temp -> \temp)   */
/*                                                                      */
/* Parameters In  :                                                     */
/*   szPath = string containing the path to treat		                    */
/*                                                                      */
/* Parameters Out :                                                     */
/*                                                                      */
/* Return     : UT_ERR_OKAY                                             */
/*======================================================================*/
#if defined(ANSI)
int ut_RemoveDrive(char *szPath)
#else
int ut_RemoveDrive(szPath)
char *szPath;
#endif
{
	/* Check if path contains drive information */
	if(szPath[1] == ':')
	{
		strcpy(szPath, szPath+2);
	}
	return UT_ERR_OKAY;
}

/*======================================================================*/
/* Function   : ut_NextSubDir		                                        */
/* Abstract   : Replace a path by its nearest sub-directory             */
/*                                                                      */
/* Parameters In  :                                                     */
/*   szPath = string containing the path to treat		                    */
/*                                                                      */
/* Parameters Out :                                                     */
/*	 szPath = nearest sub-directory, or "" if root is reached						*/
/*                                                                      */
/* Return     : UT_ERR_OKAY                                             */
/*======================================================================*/
#if defined(ANSI)
int ut_NextSubDir(char *szPath)
#else
int ut_NextSubDir(szPath)
char *szPath;
#endif
{
	int i;

    i = (int)strlen(szPath)-1;

    /* Make sure we have something */
    if(i <= 0)
        return UT_ERR_OKAY;

	/* Look for last '/' or '\' in path */
    while(i>=0 && szPath[(unsigned int)i] != '/' && szPath[(unsigned int)i] != '\\')		i--;

    if(i >= 0)
        szPath[(unsigned int)i] = '\0';
    else
        strcpy(szPath, "");

#ifndef __unix__
	if(szPath[1] == ':' && strlen(szPath) == 2)
		strcpy(szPath, "");
#endif

	return UT_ERR_OKAY;
}

/* ========================================================== */
/* Normalize 'fNumber' switch 'szUnit' value                  */
/* ========================================================== */
/* Line format : Unit to | Normalized | coefficient for       */
/*               normaliz| unit       | multiply              */
/*               -------------------------------------------- */
/*     exemple :    "mV" |    "V"     |  1e-03                */
/*                                                            */
/* ========================================================== */
#if defined(ANSI)
int ut_Normalize(float* fNumber, char* szUnit)
#else
int ut_Normalize(fNumber,szUnit)
float*  fNumber;
char*   szUnit;
#endif
{
	char	c = *szUnit;

	switch(c)
	{
	case 'f':
		*fNumber *= (float)1e-15;
		break;

	case 'p':
		*fNumber *= (float)1e-12;
		break;

	case 'n':
		*fNumber *= (float)1e-09;
		break;

	case 'u':
		*fNumber *= (float)1e-06;
		break;

	case 'm':
		*fNumber *= (float)1e-03;
		break;

	case '%':
		*fNumber *= (float)1e-02;
		break;

	case 'c':
		*fNumber *= (float)1e+02;
		break;

	case 'k':
	case 'K':
		*fNumber *= (float)1e+03;
		break;

	case 'M':
		*fNumber *= (float)1e+06;
		break;

	case 'G':
		*fNumber *= (float)1e+09;
		break;

	default:
		break;
	}
	return UT_ERR_OKAY;
}

/*======================================================================*/
/* Function   : ut_ReadFromXmlFile                                      */
/* Abstract   : Read en entry from a section in .ini style file         */
/*                                                                      */
/* Parameters In  :                                                     */
/*   hFile      = file handle of .ini style file                        */
/*   szSection  = string with section to look in                        */
/*   szEntry    = string with entry to read from section                */
/*   szValue    = buffer to receive string read                         */
/*	 nSectionsToSkip = nb of sections to be skipped before reading		*/
/*                                                                      */
/* Parameters Out :                                                     */
/*                                                                      */
/* Return     : UT_ERR_OKAY                                             */
/*              UT_ERR_NOSECTION : Couldn't find section                */
/*              UT_ERR_NOENTRTY  : Couldn't find entry                  */
/*======================================================================*/
#if defined(ANSI)
int ut_ReadFromXmlFile(FILE *hFile, const char *szSection, const char *szEntry, char *szValue, int buffer_len, int nSectionsToSkip)
#else
int ut_ReadFromXmlFile(hFile, szSection, szEntry, szValue, buffer_len,  nSectionsToSkip)
FILE		*hFile;
const char	*szSection, *szEntry;
char		*szValue;
int			buffer_len
int			nSectionsToSkip;
#endif
{
	char  szBuffer[UT_MAX_BUFFERLENGTH], szFullSectionName[UT_MAX_SECTIONLENGTH];
    int   nSectionFound = 0, nEndOfSection = 0;
    unsigned int nLength;
	char  *ptScan1, *ptScan2;

	/* Check file handle */
	if(!hFile)
		return UT_ERR_READ;

	/* Build full section name */
	sprintf(szFullSectionName, "<%s>", szSection);

	/* Rewind file */
	rewind(hFile);

	/* Read file */
	while((!nSectionFound || nSectionsToSkip) && fgets(szBuffer, UT_MAX_BUFFERLENGTH, hFile))
	{
		/* Skip a section ? */
		if(nSectionFound)
		{
			nSectionsToSkip--;
			nSectionFound = 0;
		}
		/* Remove \n */
		szBuffer[strlen(szBuffer)-1] = '\0';
		if(!UT_strnicmp(szBuffer, szFullSectionName, strlen(szFullSectionName)))
			nSectionFound = 1;
	}

	/* Check if section found */
	if(!nSectionFound)
		return UT_ERR_NOSECTION;

	/* Section found, look for entry */
	while(!nEndOfSection && fgets(szBuffer, UT_MAX_BUFFERLENGTH, hFile))
	{
		/* Remove \n if necessary */
		nLength = strlen(szBuffer);

		// buffer overflow
        if (nLength > (unsigned int)buffer_len)
			return UT_ERR_OVERFLOW;

        if((nLength > 0) && (szBuffer[nLength-1] == '\n'))
			szBuffer[nLength-1] = '\0';

		/* Check if beginning of a new section */
		if(szBuffer[0] == '[')
			nEndOfSection = 1;
		else
		{
			/* Check for '=' character */
			ptScan1 = szBuffer;
			while((*ptScan1 != '=') && (*ptScan1 != '\0'))    ptScan1++;
			if(*ptScan1 == '=')
			{
				ptScan2 = ptScan1+1;
				*ptScan1 = '\0';
				if(!UT_strnicmp(szBuffer, szEntry, strlen(szEntry)))
				{
					/* Entry found */
					strcpy(szValue, ptScan2);
					return UT_ERR_OKAY;
				}
			}
		}
	}

	/* Entry not found */

	return UT_ERR_NOENTRY;
}

/*======================================================================*/
/* Function   : ut_ReadFromIniFile                                      */
/* Abstract   : Read en entry from a section in .ini style file         */
/*                                                                      */
/* Parameters In  :                                                     */
/*   hFile      = file handle of .ini style file                        */
/*   szSection  = string with section to look in                        */
/*   szEntry    = string with entry to read from section                */
/*   szValue    = buffer to receive string read                         */
/*	 nSectionsToSkip = nb of sections to be skipped before reading		*/
/*                                                                      */
/* Parameters Out :                                                     */
/*                                                                      */
/* Return     : UT_ERR_OKAY                                             */
/*              UT_ERR_NOSECTION : Couldn't find section                */
/*              UT_ERR_NOENTRTY  : Couldn't find entry                  */
/*======================================================================*/
#if defined(ANSI)
int ut_ReadFromIniFile(FILE *hFile, const char *szSection, const char *szEntry, char *szValue, int buffer_len, int *nLastCheckedLine, int nSectionsToSkip)
#else
int ut_ReadFromIniFile(hFile, szSection, szEntry, szValue, buffer_len, nLastCheckedLine, nSectionsToSkip)
FILE *hFile;
const char *szSection;
const char *szEntry;
char *szValue;
int buffer_len;
int *nLastCheckedLine;
int nSectionsToSkip;
#endif
{
	char  szBuffer[UT_MAX_BUFFERLENGTH], szFullSectionName[UT_MAX_SECTIONLENGTH];
    int   nSectionFound = 0, nEndOfSection = 0;
    unsigned int nLength;
	char  *ptScan1, *ptScan2;
	*nLastCheckedLine = 0;

	/* Check file handle */
	if(!hFile)
		return UT_ERR_READ;

	/* Build full section name */
	sprintf(szFullSectionName, "[%s]", szSection);

	/* Rewind file */
	rewind(hFile);

	/* Read file */
	while((!nSectionFound || nSectionsToSkip) && fgets(szBuffer, UT_MAX_BUFFERLENGTH, hFile))
	{
		(*nLastCheckedLine)++;
		/* Skip a section ? */
		if(nSectionFound)
		{
			nSectionsToSkip--;
			nSectionFound = 0;
		}
		/* Remove \n */
		szBuffer[strlen(szBuffer)-1] = '\0';
		if(!UT_strnicmp(szBuffer, szFullSectionName, strlen(szFullSectionName)))
			nSectionFound = 1;
	}

	/* Check if section found */
	if(!nSectionFound)
		return UT_ERR_NOSECTION;

	/* Section found, look for entry */
	while(!nEndOfSection && fgets(szBuffer, UT_MAX_BUFFERLENGTH, hFile))
	{
		(*nLastCheckedLine)++;
		/* Remove \n if necessary */
		nLength = strlen(szBuffer);

		// buffer overflow
        if (nLength > (unsigned int)buffer_len)
			return UT_ERR_OVERFLOW;

        if((nLength > 0) && (szBuffer[nLength-1] == '\n'))
			szBuffer[nLength-1] = '\0';
		else
			return UT_ERR_LINE_TRUNCATED;

		/* Check if beginning of a new section */
		if(szBuffer[0] == '[')
			nEndOfSection = 1;
		else
		{
			/* Check for '=' character */
			ptScan1 = szBuffer;
			while((*ptScan1 != '=') && (*ptScan1 != '\0'))    ptScan1++;
			if(*ptScan1 == '=')
			{
				ptScan2 = ptScan1+1;
				*ptScan1 = '\0';
				if(!UT_strnicmp(szBuffer, szEntry, strlen(szEntry)))
				{
					/* Entry found */
					strcpy(szValue, ptScan2);
					return UT_ERR_OKAY;
				}
			}
		}
	}

	/* Entry not found */

	return UT_ERR_NOENTRY;
}

/*======================================================================*/
/* Function   : ut_ReadLineFromIniFile                                  */
/* Abstract   : Read a line from a section in .ini style file			*/
/*                                                                      */
/* Parameters In  :                                                     */
/*   hFile      = file handle of .ini style file                        */
/*   szSection  = string with section to look in                        */
/*   szValue    = buffer to receive string read                         */
/*	 nLineNb	= line nb to read in the specified section				*/
/*                                                                      */
/* Parameters Out :                                                     */
/*                                                                      */
/* Return     : UT_ERR_OKAY                                             */
/*              UT_ERR_NOSECTION : Couldn't find section                */
/*              UT_ERR_NOENTRTY  : Couldn't find specified line         */
/*======================================================================*/
#if defined(ANSI)
int ut_ReadLineFromIniFile(FILE *hFile, const char *szSection, char *szValue, int buffer_len, int nLineNb)
#else
int ut_ReadLineFromIniFile(hFile, szSection, szValue, buffer_len, nLineNb)
FILE		*hFile;
const char	*szSection;
char		*szValue;
int			buffer_len
int			nLineNb;
#endif
{
	char  szBuffer[UT_MAX_BUFFERLENGTH], szFullSectionName[UT_MAX_SECTIONLENGTH];
    int   nSectionFound = 0, nEndOfSection = 0;
    unsigned int nLength;

	/* Check file handle */
	if(!hFile || (nLineNb < 1))
		return UT_ERR_READ;

	/* Build full section name */
	sprintf(szFullSectionName, "[%s]", szSection);

	/* Rewind file */
	rewind(hFile);

	/* Read file */
	while(!nSectionFound && fgets(szBuffer, UT_MAX_BUFFERLENGTH, hFile))
	{
		/* Remove \n */
		szBuffer[strlen(szBuffer)-1] = '\0';
		if(!UT_strnicmp(szBuffer, szFullSectionName, strlen(szFullSectionName)))
			nSectionFound = 1;
	}

	/* Check if section found */
	if(!nSectionFound)
		return UT_ERR_NOSECTION;

	/* Section found, look for specified line */
	while(!nEndOfSection && fgets(szBuffer, UT_MAX_BUFFERLENGTH, hFile))
	{
		/* Remove \n if necessary */
		nLength = strlen(szBuffer);

		// buffer overflow
        if (nLength > (unsigned int)buffer_len)
			return UT_ERR_OVERFLOW;

		if((nLength > 0) && szBuffer[nLength-1] == '\n')
			szBuffer[nLength-1] = '\0';

		/* Check if end of section or beginning of a new section */
		if(!strcmp(szBuffer,"") || (szBuffer[0] == '['))
			nEndOfSection = 1;
		else if(nLineNb > 1)
			nLineNb--;
		else
		{
			strcpy(szValue, szBuffer);
			return UT_ERR_OKAY;
		}
	}

	/* Entry not found */
	return UT_ERR_NOENTRY;
}

/*======================================================================*/
/* Function   : ut_WriteSectionToIniFile                                */
/* Abstract   : Write a section declaration to an .ini style file		*/
/*                                                                      */
/* Parameters In  :                                                     */
/*   hFile		= An handle to a file									*/
/*   szSection  = string with section to write in                       */
/*                                                                      */
/* Parameters Out :                                                     */
/*                                                                      */
/* Return     : UT_ERR_OKAY                                             */
/*              UT_ERR_WRITE : Couldn't write the section               */
/*======================================================================*/
#if defined(ANSI)
int ut_WriteSectionToIniFile(FILE *hFile, const char *szSection)
#else
int ut_WriteSectionToIniFile(hFile, szSection)
FILE		*hFile;
const char	*szSection;
#endif
{
	if(fprintf(hFile,"[%s]\n",szSection) < 0)
		return UT_ERR_WRITE;

	return UT_ERR_OKAY;
}

/*======================================================================*/
/* Function   : ut_WriteEntryToIniFile									*/
/* Abstract   : Write an entry to an .ini style file					*/
/*                                                                      */
/* Parameters In  :                                                     */
/*   hFile		= An handle to a file									*/
/*	 szEntry	= string with the entry name							*/
/*   szValue	= string with entry value								*/
/*                                                                      */
/* Parameters Out :                                                     */
/*                                                                      */
/* Return     : UT_ERR_OKAY                                             */
/*              UT_ERR_WRITE : Couldn't write the section               */
/*======================================================================*/
#if defined(ANSI)
int ut_WriteEntryToIniFile(FILE *hFile, const char *szEntry, const char *szValue)
#else
int ut_WriteEntryToIniFile(hFile, szEntry,szValue)
FILE		*hFile;
const char	*szEntry,*szValue;
#endif
{
	if(fprintf(hFile,"%s=%s\n",szEntry,szValue) < 0)
		return UT_ERR_WRITE;

	return UT_ERR_OKAY;
}

/*======================================================================*/
/* Function   : ut_UpdateValueToIniFile									*/
/* Abstract   : Update the value of an entry to an .ini style file		*/
/*                                                                      */
/* Parameters In  :                                                     */
/*   hFile      = file handle of .ini style file                        */
/*   szSection  = string with section to look in                        */
/*   szEntry    = string with entry to read from section                */
/*   szValue    = string to write in the file							*/
/*	 nSectionsToSkip = nb of sections to be skipped before reading		*/
/*                                                                      */
/* Parameters Out :                                                     */
/*                                                                      */
/* Return     : UT_ERR_OKAY                                             */
/*              UT_ERR_NOSECTION : Couldn't find section                */
/*              UT_ERR_NOENTRTY  : Couldn't find entry                  */
/*              UT_ERR_READ : Couldn't read the section					*/
/*              UT_ERR_WRITE : Couldn't write the section               */
/*======================================================================*/
#if defined(ANSI)
int
ut_UpdateValueToIniFile(FILE* hFile,
                        const char* szSection,
                        const char* szEntry,
                        const char* szValue,
                        int nSectionsToSkip)
#else
int	ut_UpdateValueToIniFile(hFile,szSection,szEntry,szValue,nSectionsToSkip)
FILE		*hFile;
const char	*szSection, *szEntry, *szValue;
int			nSectionsToSkip;
#endif
{
  (void) hFile;
  (void) szSection;
  (void) szEntry;
  (void) szValue;
  (void) nSectionsToSkip;
  return UT_ERR_NOENTRY;
}



/*======================================================================*/
/* Find files in a directory										    */
/*                                                                      */
/* Parameters :                                                         */
/* ptHandle  = pointer on a handle for FIND_FIRST, FIND_NEXT operations */
/* szExt     = extension to search for                                  */
/* szDir     = directory to search in                                   */
/* szFilName = buffer to store name of file found                       */
/* nMode : UT_FINDFIRST                                                 */
/*         UT_FINDNEXT                                                  */
/*         UT_CLOSEFIND                                                 */
/* nType : UT_DIRONLY	(look only for directories)						*/
/*		   UT_FILEONLY	(look only for files)							*/
/*                                                                      */
/* Return Codes :                                                       */
/* Return     : 1 if a file was found , 0 else                          */
/*======================================================================*/
#if defined(ANSI)
int ut_FindFiles(PT_UT_FindFileHandle ptHandle, char *szExt, char *szDir, char *szFileName, int nMode, int nType)
#else
int ut_FindFiles(ptHandle, szExt, szDir, szFileName, nMode, nType)
PT_UT_FindFileHandle	ptHandle;
char					*szExt, *szDir, *szFileName;
int						nMode, nType;
#endif
{
  char szSearchDir[UT_MAX_PATH];
  char szFullName[UT_MAX_PATH];
#if defined __unix__ || __APPLE__&__MACH__
	char			szFPath[UT_MAX_PATH], szFExt[UT_MAX_EXT], szFName[UT_MAX_FNAME];
	struct dirent	*pDirEntry;
	struct stat		stFileStat;
#else
  char szFilter[UT_MAX_PATH];
	struct _finddata_t	hFileInfo;
	struct _stat		stFileStat;
	int					nStopLoop=0;
#endif

	/* Search dir must terminate with ending / */
	strcpy(szSearchDir, szDir);
	if(strcmp(szSearchDir, "") && (szSearchDir[strlen(szSearchDir)-1] != '/') && (szSearchDir[strlen(szSearchDir)-1] != '\\'))
		strcat(szSearchDir, "/");

	switch(nMode)
	{
    case UT_FINDFIRST: /* Find First */
		/* Get first .file in directory */
#if defined __unix__ || __APPLE__&__MACH__
		ptHandle->pDir = opendir(szSearchDir);
		if(!(ptHandle->pDir))
		{
			strcpy(szFileName, "");
			return 0;
		}
		pDirEntry = readdir(ptHandle->pDir);
		while(pDirEntry)
		{
			ut_SplitPathName(pDirEntry->d_name, szFPath, szFName, szFExt);
			if(!strcmp(szExt, "") || !strcmp(szFExt+1, szExt))
			{
				/* Work only on directories, or only on files ? */
				sprintf(szFullName, "%s%s", szSearchDir, pDirEntry->d_name);
				switch(nType)
				{
				case UT_DIRONLY:
					if(!stat(szFullName, &stFileStat) && (stFileStat.st_mode & S_IFDIR))
					{
						strcpy(szFileName, pDirEntry->d_name);
						return 1;
					}
					break;

				case UT_FILEONLY:
					if(!stat(szFullName, &stFileStat) && !(stFileStat.st_mode & S_IFDIR))
					{
						strcpy(szFileName, pDirEntry->d_name);
						return 1;
					}
					break;

				default:
					strcpy(szFileName, pDirEntry->d_name);
					return 1;
					break;
				}
			}
			pDirEntry = readdir(ptHandle->pDir);
		}
		strcpy(szFileName, "");
		return 0;
#else
		if(!strcmp(szExt, ""))
			sprintf(szFilter, "%s*", szSearchDir);
		else
			sprintf(szFilter, "%s*.%s", szSearchDir, szExt);
		while(!nStopLoop)
		{
			ptHandle->lHandle = _findfirst(szFilter, &hFileInfo);
			if((ptHandle->lHandle) == -1)
				nStopLoop = 1;
			else
			{
				sprintf(szFullName, "%s%s", szSearchDir, hFileInfo.name);
				/* Work only on directories, or only on files ? */
				switch(nType)
				{
				case UT_DIRONLY:
					if(!_stat(szFullName, &stFileStat) && (stFileStat.st_mode & S_IFDIR))
					{
						strcpy(szFileName, hFileInfo.name);
						return 1;
					}
					break;

				case UT_FILEONLY:
					if(!_stat(szFullName, &stFileStat) && !(stFileStat.st_mode & S_IFDIR))
					{
						strcpy(szFileName, hFileInfo.name);
						return 1;
					}
					break;

				default:
					strcpy(szFileName, hFileInfo.name);
					return 1;
					break;
				}
			}
		}
		strcpy(szFileName, "");
		return 0;
#endif
		break;

    case UT_FINDNEXT: /* Find Next */
		/* Get next file in directory */
#if defined __unix__ || __APPLE__&__MACH__
		pDirEntry = readdir(ptHandle->pDir);
		while(pDirEntry)
		{
			ut_SplitPathName(pDirEntry->d_name, szFPath, szFName, szFExt);
			if(!strcmp(szExt, "") || !strcmp(szFExt+1, szExt))
			{
				sprintf(szFullName, "%s%s", szSearchDir, pDirEntry->d_name);
				/* Work only on directories, or only on files ? */
				switch(nType)
				{
				case UT_DIRONLY:
					if(!stat(szFullName, &stFileStat) && (stFileStat.st_mode & S_IFDIR))
					{
						strcpy(szFileName, pDirEntry->d_name);
						return 1;
					}
					break;

				case UT_FILEONLY:
					if(!stat(szFullName, &stFileStat) && !(stFileStat.st_mode & S_IFDIR))
					{
						strcpy(szFileName, pDirEntry->d_name);
						return 1;
					}
					break;

				default:
					strcpy(szFileName, pDirEntry->d_name);
					return 1;
				}
			}
			pDirEntry = readdir(ptHandle->pDir);
		}
		strcpy(szFileName, "");
		return 0;
#else
		while(!nStopLoop)
		{
			if(_findnext(ptHandle->lHandle, &hFileInfo) == -1)
				nStopLoop = 1;
			else
			{
				sprintf(szFullName, "%s%s", szSearchDir, hFileInfo.name);
				/* Work only on directories, or only on files ? */
				switch(nType)
				{
				case UT_DIRONLY:
					if(!_stat(szFullName, &stFileStat) && (stFileStat.st_mode & S_IFDIR))
					{
						strcpy(szFileName, hFileInfo.name);
						return 1;
					}
					break;

				case UT_FILEONLY:
					if(!_stat(szFullName, &stFileStat) && !(stFileStat.st_mode & S_IFDIR))
					{
						strcpy(szFileName, hFileInfo.name);
						return 1;
					}
					break;

				default:
					strcpy(szFileName, hFileInfo.name);
					return 1;
				}
			}
		}
		strcpy(szFileName, "");
		return 0;
#endif
		break;

    case UT_CLOSEFIND: /* Close Find */
#if defined __unix__ || __APPLE__&__MACH__
		if(ptHandle->pDir)
			closedir(ptHandle->pDir);
#else
		if((ptHandle->lHandle) != -1)
			_findclose(ptHandle->lHandle);
#endif
		return 1;
		break;
	}
	return 0;
}

/*======================================================================*/
/* Return size of file                                                  */
/*                                                                      */
/* Parameters :                                                         */
/* szFile    = name of file                                             */
/*                                                                      */
/* Return     : size of the file in bytes (-1 if error)                 */
/*======================================================================*/
#if defined(ANSI)
int ut_GetFileSize(const char *szFileName)
#else
int ut_GetFileSize(szFileName)
const char  *szFileName;
#endif
{
	struct  stat attributes;

	/* Get File Size */
	if(stat(szFileName, &attributes) == -1)
	{
		return -1;
	}

	return attributes.st_size;
}

/*======================================================================*/
/* Return modification time of file                                     */
/*                                                                      */
/* Parameters :                                                         */
/* szFile    = name of file                                             */
/*                                                                      */
/* Return     : modification time (0 if error)							*/
/*======================================================================*/
#if defined(ANSI)
time_t ut_GetFileMtime(const char *szFileName)
#else
time_t ut_GetFileMtime(szFileName)
const char  *szFileName;
#endif
{
	struct  stat attributes;

	/* Get File modification time */
	if(stat(szFileName, &attributes) == -1)
	{
		return 0;
	}

	return attributes.st_mtime;
}

/*======================================================================*/
/* Return creation time of file											*/
/*                                                                      */
/* Parameters :                                                         */
/* szFile    = name of file                                             */
/*                                                                      */
/* Return     : modification time (0 if error)							*/
/*======================================================================*/
#if defined(ANSI)
time_t ut_GetFileCtime(const char *szFileName)
#else
time_t ut_GetFileCtime(szFileName)
const char  *szFileName;
#endif
{
	struct  stat attributes;

	/* Get File modification time */
	if(stat(szFileName, &attributes) == -1)
	{
		return 0;
	}

	return attributes.st_ctime;
}

/*======================================================================*/
/* Return last access time of file										*/
/*                                                                      */
/* Parameters :                                                         */
/* szFile    = name of file                                             */
/*                                                                      */
/* Return     : modification time (0 if error)							*/
/*======================================================================*/
#if defined(ANSI)
time_t ut_GetFileAtime(const char *szFileName)
#else
time_t ut_GetFileAtime(szFileName)
const char  *szFileName;
#endif
{
	struct  stat attributes;

	/* Get File modification time */
	if(stat(szFileName, &attributes) == -1)
	{
		return 0;
	}

	return attributes.st_atime;
}

/*======================================================================*/
/* Return permission mode of file										*/
/*                                                                      */
/* Parameters :                                                         */
/* szFile    = name of file                                             */
/*                                                                      */
/* Return     : permission mode (0 if error)							*/
/*======================================================================*/
#if defined(ANSI)
unsigned short ut_GetFileMode(const char *szFileName)
#else
unsigned short ut_GetFileMode(szFileName)
const char  *szFileName;
#endif
{
	struct  stat attributes;

	/* Get File modification time */
	if(stat(szFileName, &attributes) == -1)
	{
		return 0;
	}

	return attributes.st_mode;
}

/*======================================================================*/
/* Copy a file (using fopen() function)                                 */
/*                                                                      */
/* Parameters : 														*/
/*	szSourceFile : name of source file to copy							*/
/*	szDestFile	 : name of destination file								*/
/*	nOverWrite   : 0 if destination file should not be overwritten if	*/
/*				   it exists											*/
/*	nUseTemp	 : not 0 if the function should use a temporary file to */
/*				   copy to and then rename the destination file to		*/
/*				   szDestFile											*/
/*																		*/
/* Return : 1 if copy sucessfull, 0 else                                */
/*======================================================================*/
#if defined(ANSI)
int ut_CopyFile(const char *szSourceFile, const char *szDestFile, int nMode)
#else
int ut_CopyFile(szSourceFile, szDestFile, nMode)
const char	*szSourceFile, *szDestFile;
int			nMode;
#endif
{
	FILE	*fpSourceFile, *fpDestFile;
	char	szTempDestFile[UT_MAX_PATH], *ptBuffer;
	int		nFileSize, nBytes;
	struct utimbuf	time_buf;

	/* Check if files have different names */
	if(!strcmp(szSourceFile, szDestFile))
		return 0;

	/* Get size of the file to copy */
	if((nFileSize = ut_GetFileSize(szSourceFile)) == -1)
		return 0;

	/* Get modification time of file to copy */
	if((time_buf.modtime = ut_GetFileMtime(szSourceFile)) == 0)
		return 0;
	time_buf.actime = time_buf.modtime;

	/* Check if destination file exists */
	fpDestFile = fopen(szDestFile, "r");
	if(fpDestFile)
	{
		fclose(fpDestFile);
		if(!(nMode & UT_COPY_OVERWRITE))
			return 0;
		if(remove(szDestFile))
			return 0;
	}

	/* Allocate memory for transfer buffer */
	ptBuffer = (char *)malloc(nFileSize*sizeof(char));
	if(!ptBuffer)
		return 0;

	/* Change destination filename if nUseTemp */
	strcpy(szTempDestFile, szDestFile);
	if(nMode & UT_COPY_USETEMPNAME)
		szTempDestFile[strlen(szTempDestFile)-1] = '#';

	/* Open source file */
	fpSourceFile = fopen(szSourceFile, "rb");
	if(!fpSourceFile)
	{
		free(ptBuffer);
		return 0;
	}

	/* Open destination file */
	fpDestFile = fopen(szTempDestFile, "wb");
	if(!fpDestFile)
	{
		free(ptBuffer);
		fclose(fpSourceFile);
		return 0;
	}

	/* Copy the file */
	nBytes = fread(ptBuffer, sizeof(char), nFileSize, fpSourceFile);
	if(nBytes != nFileSize)
	{
		free(ptBuffer);
		fclose(fpSourceFile);
		fclose(fpDestFile);
		return 0;
	}
	nBytes = fwrite(ptBuffer, sizeof(char), nFileSize, fpDestFile);
	if(nBytes != nFileSize)
	{
		free(ptBuffer);
		fclose(fpSourceFile);
		fclose(fpDestFile);
		return 0;
	}

	/* Cleanup nUseTemp */
	free(ptBuffer);
	fclose(fpSourceFile);
	fclose(fpDestFile);

	/* Rename file if nUseTemp */
	if((nMode & UT_COPY_USETEMPNAME) && rename(szTempDestFile, szDestFile))
		return 0;

	/* Set modification time */
	if((nMode & UT_COPY_UPDATEMTIME) && utime(szDestFile, &time_buf))
		return 0;

	return 1;
}

#if 0
/*======================================================================*/
/* Copy a file (using open() function)                                  */
/*                                                                      */
/* Parameters : 														*/
/*	szSourceFile : name of source file to copy							*/
/*	szDestFile	 : name of destination file								*/
/*	nOverWrite   : 0 if destination file should not be overwritten if	*/
/*				   it exists											*/
/*	nUseTemp	 : not 0 if the function should use a temporary file to */
/*				   copy to and then rename the destination file to		*/
/*				   szDestFile											*/
/*																		*/
/* Return : 1 if copy sucessfull, 0 else                                */
/*======================================================================*/
int ut_CopyFile(szSourceFile, szDestFile, nMode)
const char	*szSourceFile, *szDestFile;
int		nMode;
{
	int				hSourceFile, hDestFile;
	char			szTempDestFile[UT_MAX_PATH], *ptBuffer;
	int				nFileSize, nBytes;
	struct utimbuf	time_buf;

	/* Check if files have different names */
	if(!strcmp(szSourceFile, szDestFile))
		return 0;

	/* Get size of the file to copy */
	if((nFileSize = ut_GetFileSize(szSourceFile)) == -1)
		return 0;

	/* Get modification time of file to copy */
	if((time_buf.modtime = ut_GetFileMtime(szSourceFile)) == 0)
		return 0;
	time_buf.actime = time_buf.modtime;

	/* Check if destination file exists */
	hDestFile = open(szDestFile, O_RDONLY);
	if(hDestFile != -1)
	{
		close(hDestFile);
		if(!(nMode & UT_COPY_OVERWRITE))
			return 0;
		if(remove(szDestFile))
			return 0;
	}

	/* Allocate memory for transfer buffer */
	ptBuffer = (char *)malloc(nFileSize*sizeof(char));
	if(!ptBuffer)
		return 0;

	/* Change destination filename if nUseTemp */
	strcpy(szTempDestFile, szDestFile);
	if(nMode & UT_COPY_USETEMPNAME)
		szTempDestFile[strlen(szTempDestFile)-1] = '#';

	/* Open source file */
	hSourceFile = open(szSourceFile, O_RDONLY);
	if(hSourceFile == -1)
	{
		free(ptBuffer);
		return 0;
	}

	/* Open destination file */
	hDestFile = open(szTempDestFile, O_WRONLY | O_CREAT, 0644);
	if(hDestFile == -1)
	{
		free(ptBuffer);
		close(hSourceFile);
		return 0;
	}


	/* Copy the file */
	nBytes = read(hSourceFile, ptBuffer, nFileSize);
	if(nBytes != nFileSize)
	{
		free(ptBuffer);
		close(hSourceFile);
		close(hDestFile);
		return 0;
	}
	nBytes = write(hDestFile, ptBuffer, nFileSize);
	if(nBytes != nFileSize)
	{
		free(ptBuffer);
		close(hSourceFile);
		close(hDestFile);
		return 0;
	}

	/* Cleanup nUseTemp */
	free(ptBuffer);
	close(hSourceFile);
	close(hDestFile);

	/* Rename file if nUseTemp */
	if((nMode & UT_COPY_USETEMPNAME) && rename(szTempDestFile, szDestFile))
		return 0;

	/* Set modification time */
	if((nMode & UT_COPY_UPDATEMTIME) && utime(szDestFile, &time_buf))
		return 0;

	return 1;
}
#endif

#if 0
/*======================================================================*/
/* Copy a file (using open() and lock functions)                        */
/*                                                                      */
/* Parameters : 														*/
/*	szSourceFile : name of source file to copy							*/
/*	szDestFile	 : name of destination file								*/
/*	nOverWrite   : 0 if destination file should not be overwritten if	*/
/*				   it exists											*/
/*	nUseTemp	 : not 0 if the function should use a temporary file to */
/*				   copy to and then rename the destination file to		*/
/*				   szDestFile											*/
/*																		*/
/* Return : 1 if copy sucessfull, 0 else                                */
/*======================================================================*/
int ut_CopyFile(szSourceFile, szDestFile, nMode)
const char	*szSourceFile, *szDestFile;
int		nMode;
{
	int				hSourceFile, hDestFile;
	char			szTempDestFile[UT_MAX_PATH], *ptBuffer;
	int				nFileSize, nBytes;
	struct utimbuf	time_buf;
#if defined __unix__ || __APPLE__&__MACH__
	int				nStatus;
	struct flock	stSourceLock, stDestLock;
#endif

	/* Check if files have different names */
	if(!strcmp(szSourceFile, szDestFile))
		return 0;

	/* Get size of the file to copy */
	if((nFileSize = ut_GetFileSize(szSourceFile)) == -1)
		return 0;

	/* Get modification time of file to copy */
	if((time_buf.modtime = ut_GetFileMtime(szSourceFile)) == 0)
		return 0;
	time_buf.actime = time_buf.modtime;

	/* Check if destination file exists */
	hDestFile = open(szDestFile, O_RDONLY);
	if(hDestFile != -1)
	{
		close(hDestFile);
		if(!(nMode & UT_COPY_OVERWRITE))
			return 0;
		if(remove(szDestFile))
			return 0;
	}

	/* Allocate memory for transfer buffer */
	ptBuffer = (char *)malloc(nFileSize*sizeof(char));
	if(!ptBuffer)
		return 0;

	/* Change destination filename if nUseTemp */
	strcpy(szTempDestFile, szDestFile);
	if(nMode & UT_COPY_USETEMPNAME)
		szTempDestFile[strlen(szTempDestFile)-1] = '#';

	/* Open source file */
	hSourceFile = open(szSourceFile, O_RDONLY);
	if(hSourceFile == -1)
	{
		free(ptBuffer);
		return 0;
	}

#if defined __unix__ || __APPLE__&__MACH__
	/* Lock source file for reading */
	stSourceLock.l_type = F_RDLCK;
	stSourceLock.l_whence = SEEK_SET;
	stSourceLock.l_start = 0;
	stSourceLock.l_len = 0;
	nStatus = fcntl(hSourceFile, F_SETLK, &stSourceLock);
	if(nStatus == -1)
	{
		free(ptBuffer);
		close(hSourceFile);
		return 0;
	}
	stSourceLock.l_type = F_UNLCK;
#endif

	/* Open destination file */
	hDestFile = open(szTempDestFile, O_WRONLY | O_CREAT, 0644);
	if(hDestFile == -1)
	{
		free(ptBuffer);
		close(hSourceFile);
		return 0;
	}

#if defined __unix__ || __APPLE__&__MACH__
	/* Lock dest file for writing */
	stDestLock.l_type = F_WRLCK;
	stDestLock.l_whence = SEEK_SET;
	stDestLock.l_start = 0;
	stDestLock.l_len = 0;

	nStatus = fcntl(hDestFile, F_SETLK, &stDestLock);
	if(nStatus == -1)
	{
		free(ptBuffer);
		fcntl(hSourceFile, F_SETLK, &stSourceLock);
		close(hSourceFile);
		close(hDestFile);
		return 0;
	}
	stDestLock.l_type = F_UNLCK;
#endif

	/* Copy the file */
	nBytes = read(hSourceFile, ptBuffer, nFileSize);
	if(nBytes != nFileSize)
	{
		free(ptBuffer);
#if defined __unix__ || __APPLE__&__MACH__
		fcntl(hSourceFile, F_SETLK, &stSourceLock);
		fcntl(hDestFile, F_SETLK, &stDestLock);
#endif
		close(hSourceFile);
		close(hDestFile);
		return 0;
	}
	nBytes = write(hDestFile, ptBuffer, nFileSize);
	if(nBytes != nFileSize)
	{
		free(ptBuffer);
#if defined __unix__ || __APPLE__&__MACH__
		fcntl(hSourceFile, F_SETLK, &stSourceLock);
		fcntl(hDestFile, F_SETLK, &stDestLock);
#endif
		close(hSourceFile);
		close(hDestFile);
		return 0;
	}

	/* Cleanup nUseTemp */
	free(ptBuffer);
#if defined __unix__ || __APPLE__&__MACH__
	fcntl(hSourceFile, F_SETLK, &stSourceLock);
	fcntl(hDestFile, F_SETLK, &stDestLock);
#endif
	close(hSourceFile);
	close(hDestFile);

	/* Rename file if nUseTemp */
	if((nMode & UT_COPY_USETEMPNAME) && rename(szTempDestFile, szDestFile))
		return 0;

	/* Set modification time */
	if((nMode & UT_COPY_UPDATEMTIME) && utime(szDestFile, &time_buf))
		return 0;

	return 1;
}
#endif

/*======================================================================*/
/* Convert IP address from xxx.xxx.xxx.xxx format to a long decimal		*/
/* value																*/
/*                                                                      */
/* Parameters :                                                         */
/* szIP		= IP address in xxx.xxx.xxx.xxx format                      */
/*                                                                      */
/* Return     : unsigned long decimal IP address						*/
/*======================================================================*/
#if defined(ANSI)
unsigned long ut_IPtoUL(char *szIP)
#else
unsigned long ut_IPtoUL(szIP)
char  *szIP;
#endif
{
	int				nIP_Byte;
	unsigned long	ul_IPAddress;

	/* If the ip addres is empty, just return 0 */
	if(strlen(szIP) == 0)
		return 0;

	/* Take the address from the server configuration file */
	ul_IPAddress = 0;
	sscanf(szIP, "%d.%*d.%*d.%*d", &nIP_Byte);
	ul_IPAddress = nIP_Byte;
	ul_IPAddress = ul_IPAddress << 8;
	sscanf(szIP, "%*d.%d.%*d.%*d", &nIP_Byte);
	ul_IPAddress += nIP_Byte;
	ul_IPAddress = ul_IPAddress << 8;
	sscanf(szIP, "%*d.%*d.%d.%*d", &nIP_Byte);
	ul_IPAddress += nIP_Byte;
	ul_IPAddress = ul_IPAddress << 8;
	sscanf(szIP, "%*d.%*d.%*d.%d", &nIP_Byte);
	ul_IPAddress += nIP_Byte;

	return ul_IPAddress;
}

/*======================================================================*/
/* Function   : ut_CheckFileDiff		                                */
/* Abstract   : Check if files rae different (modification time)		*/
/*                                                                      */
/* Parameters In  :                                                     */
/*                                                                      */
/* Parameters Out :                                                     */
/*                                                                      */
/* Return     : 1 if files are different, 0 else						*/
/*======================================================================*/
#if defined(ANSI)
int ut_CheckFileDiff(const char *szFile1, const char *szFile2)
#else
int ut_CheckFileDiff(szFile1, szFile2)
const char *szFile1, *szFile2;
#endif
{
	struct  stat attributes1, attributes2;

	/* Get attributes for File1 */
	if(stat(szFile1, &attributes1) == -1)
	{
		return 1;
	}

	/* Get attributes for File2 */
	if(stat(szFile2, &attributes2) == -1)
	{
		return 1;
	}

	if(attributes1.st_mtime != attributes2.st_mtime)
		return 1;
	else
		return 0;
}

/*======================================================================*/
/* Function   : ut_RemoveChar			                                */
/* Abstract   : Removed all occurences of specified char in string		*/
/*                                                                      */
/* Parameters In  :                                                     */
/*				szString : string to analyse							*/
/*				cChar : character to remove								*/
/*                                                                      */
/* Parameters Out :                                                     */
/*                                                                      */
/* Return     : none													*/
/*======================================================================*/
#if defined(ANSI)
void ut_RemoveChar(char *szString, char cChar)
#else
void ut_RemoveChar(szString, cChar)
char *szString, cChar;
#endif
{
	char *ptBuffer;
    unsigned int i=0, j=0;

	/* Check string pointer */
	if(!szString)	return;

	/* Malloc size for buffer */
	ptBuffer = (char *)malloc(sizeof(char)*strlen(szString));
	if(!ptBuffer)	return;

	/* parse string */
	while(szString[i] != '\0')
	{
		if(szString[i] != cChar)
			ptBuffer[j++] = szString[i];

		i++;
	}
	ptBuffer[j] = '\0';
	strcpy(szString, ptBuffer);

	free(ptBuffer);
	return;
}

/*======================================================================*/
/* Function   : ut_ReplaceChar			                                */
/* Abstract   : Replace all occurences of specified char in string		*/
/*                                                                      */
/* Parameters In  :                                                     */
/*				szString : string to analyse							*/
/*				cFromChar : character to replace						*/
/*				cToChar : character to replace with						*/
/*                                                                      */
/* Parameters Out :                                                     */
/*                                                                      */
/* Return     : none													*/
/*======================================================================*/
#if defined(ANSI)
void ut_ReplaceChar(char *szString, char cFromChar, char cToChar)
#else
void ut_ReplaceChar(szString, cFromChar, cToChar)
char *szString, cFromChar, cToChar;
#endif
{
    unsigned int i=0;

	/* Check string pointer */
    if(!szString)
        return;

	/* parse string */
	for(i=0 ; szString[i] != '\0' ; i++)
	{
		if(szString[i] == cFromChar)
			szString[i] = cToChar;
	}
	return;
}

/*======================================================================*/
/* Function   : ut_GetMonthNb			                                */
/* Abstract   : Get month number (1-12) of specified month (jan, feb ..)*/
/*                                                                      */
/* Parameters In  :                                                     */
/*				szMonth : month											*/
/*                                                                      */
/* Parameters Out :                                                     */
/*                                                                      */
/* Return     : month number or 0 if specified month not correct		*/
/*======================================================================*/
#if defined(ANSI)
int ut_GetMonthNb(char *szMonth)
#else
int ut_GetMonthNb(szMonth)
char *szMonth;
#endif
{
	if(!UT_stricmp(szMonth, "jan"))	return 1;
	if(!UT_stricmp(szMonth, "feb"))	return 2;
	if(!UT_stricmp(szMonth, "mar"))	return 3;
	if(!UT_stricmp(szMonth, "apr"))	return 4;
	if(!UT_stricmp(szMonth, "may"))	return 5;
	if(!UT_stricmp(szMonth, "jun"))	return 6;
	if(!UT_stricmp(szMonth, "jul"))	return 7;
	if(!UT_stricmp(szMonth, "aug"))	return 8;
	if(!UT_stricmp(szMonth, "sep"))	return 9;
	if(!UT_stricmp(szMonth, "oct"))	return 10;
	if(!UT_stricmp(szMonth, "nov"))	return 11;
	if(!UT_stricmp(szMonth, "dec"))	return 12;
	return 0;
}

/*======================================================================*/
/* Function   : ut_StringToUpper			                            */
/* Abstract   : Convert string to uppercase								*/
/*                                                                      */
/* Parameters In  :                                                     */
/*				szString : string to be converted						*/
/*                                                                      */
/* Parameters Out :                                                     */
/*                                                                      */
/* Return     : none													*/
/*======================================================================*/
#if defined(ANSI)
void ut_StringToUpper(char *szString)
#else
void ut_StringToUpper(szString)
char *szString;
#endif
{
    unsigned int i=0;

	/* Check string pointer */
	if(!szString)	return;

	/* parse string */
	for(i=0 ; szString[i] != '\0' ; i++)
    szString[i] = toupper(szString[i]);

	return;
}

/*======================================================================*/
/* Read a string from std input, without echoing the characters			*/
/*                                                                      */
/* Return : nb of characters read										*/
/*======================================================================*/
#if defined(ANSI)
int ut_ReadString_NoEcho(char *szString)
#else
int ut_ReadString_NoEcho(szString)
char  *szString;
#endif
{
#if defined __unix__ || __APPLE__&__MACH__
	struct termios	stTerm;
    int				c;
    unsigned int nCount=0;
	tcflag_t		tcOriginalFlag;

	/* Retrieve terminal attributes */
	if(tcgetattr(1, &stTerm) == -1)
		return 0;

	/* Save original flag value */
	tcOriginalFlag = stTerm.c_lflag;

	/* Disable echo */
	stTerm.c_lflag &= ~ECHO;
	if(tcsetattr(1, TCSANOW, &stTerm) == -1)
		return 0;

	/* Get string */
	strcpy(szString, "");
	while(((c = getchar()) != 10) && (c != 13))
	{
		if((c == 0x0) || (c == 0xE0))	/* Ignore 2 char keys (arrows...) */
			c = getchar();
		else
			szString[nCount++] = (char)c;
	}

	/* Terminate the string */
	szString[nCount] = '\0';

	/* Reset to original flag value */
	stTerm.c_lflag = tcOriginalFlag;
	tcsetattr(1, TCSANOW, &stTerm);

    return (int)nCount;
#else
    int	c;
    unsigned int nCount=0;

	/* Get string */
	strcpy(szString, "");
	while(((c = getch()) != 10) && (c != 13))
	{
		if((c == 0x0) || (c == 0xE0))	/* Ignore 2 char keys (arrows...) */
			c = getch();
		else
			szString[nCount++] = (char)c;
	}

	/* Terminate the string */
	szString[nCount] = '\0';

    return (int)nCount;
#endif
}

/*======================================================================*/
/* Function   : ut_TruncateFloat                                        */
/* Abstract   :	return value truncated to given precision               */
/*                                                                      */
/* Parameters In  :                                                     */
/*				Value : value to be rounded                             */
/*              Precision : precision to use                            */
/*                                                                      */
/* Parameters Out :                                                     */
/*                                                                      */
/* Return     : Rounded value											*/
/*======================================================================*/
#if defined(ANSI)
float ut_TruncateFloat(float Value, unsigned int Precision)
#else
void ut_TruncateFloat(Value, Precision)
float Value;
unsigned int Precision;
#endif
{
    double lMultiplier = powf(10.0F, (float)Precision);
    double lTruncatedValue = Value*lMultiplier;
    lTruncatedValue = ceil(lTruncatedValue);
    lTruncatedValue = lTruncatedValue/lMultiplier;

    return (float)lTruncatedValue;
}

