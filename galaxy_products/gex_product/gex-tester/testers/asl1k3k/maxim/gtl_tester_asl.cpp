/****************************************************************************************/
/* Copyright Galaxy                                                                     */
/* This computer program is protected by copyright law                                  */
/* and international treaties. Unauthorized reproduction or                             */
/* distribution of this program, or any portion of it,may                               */
/* result in severe civil and criminal penalties, and will be                           */
/* prosecuted to the maximum extent possible under the law.                             */
/****************************************************************************************/
/*                                                                                      */
/* Revision history:                                                                    */
/*                                                                                      */
/* o [WT] 20 Nov 2012: Created.                                                         */
/* o [BG] 22 Nov 2012: Implemented GetSiteInfo                                          */
/* o [WT] 26 Nov 2012: Implemented GetProdInfo                                          */
/* o [WT] 27 Nov 2012: Implemented GetNodeInfo						                    */
/* o [WT] 29 Nov 2012: Misc											                    */
/*                                                                                      */
/****************************************************************************************/

#define _GTL_TESTER_MODULE_

/*======================================================================================*/
/* Includes                                                                             */
/*======================================================================================*/

// ASL includes in ASL_NT/System...
#include <asl.h>
#include <UserInfo.h>
// The Datalog class is the function with set_bin(int), get_test_result(), set_max_limit_value(...),... 
#include <Datalog.h>

// Standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// User include files
#include "USER.H"

// GalaxySemi includes
//#include <c:/Max_nt/galaxy/include/gstdl_utils_c.h>
//#include <c:/Max_nt/galaxy/include/gtc_constants.h>
#include <c:/Max_nt/galaxy/include/gtl_tester.h>


/*======================================================================*/
/* DEFINES																*/
/*======================================================================*/
/* To temporarly store env variables */
#define ENV_TMP_FILE			".galaxy_env_tmp.txt"
/* For test limit specification */
#define TEST_LIMIT_VALID		0
#define TEST_LIMIT_INVALID		1

/*======================================================================*/
/* EXTERN Variables 																										*/
/*======================================================================*/

//#define MAXPATHLEN 256

// Patrick : how to know the number of sites : MAX_SITES, MaxSites, CSC_MAX_SITES,... ?
// In any cases, let's put in this define the max sites number
#if 0
#define GS_MAX_SITES 4
#endif

// These global variables are sometimes created in USER.CPP or Multisite.cpp or ...
// Patrick : is n't there an ASL function to retrieve the sites info ?
#if 0
extern BOOL active_site[GS_MAX_SITES];
extern BOOL store_active_site[GS_MAX_SITES];	// active site status at START OF TEST
extern BOOL current_active_site[GS_MAX_SITES];  // active site status inside the application ("shared resources")
#endif

extern BOOL active_site[];
extern BOOL store_active_site[];	// active site status at START OF TEST
extern BOOL current_active_site[];  // active site status inside the application ("shared resources")

/*======================================================================*/
/* PUBLIC Functions 																										*/
/*======================================================================*/

// Just to prevent some link issues on VisualC 6 if these functions are used inside a C library
extern "C"
{
	int snprintf(char *s1, size_t size, const char * fmt, ...)
	{
		va_list ap;
		va_start( ap, fmt );
		int r=_snprintf(s1, size, fmt, ap);
		va_end( ap );
		return r;
	}
	int vsnprintf(char *s1, size_t size, const char *fmt, va_list ap)
	{
		int r=_vsnprintf(s1, size, fmt, ap);
		return r;
	}
}


/*======================================================================*/
/* Retrieves informations for current test (test nb, LL, HL)			*/
/*	Set test number and test limits of the current test																	*/
/* Return Codes : 1 if successfull, 0 else								*/
/*======================================================================*/
int tester_GetTestInfo(
	int *nTestNb,  // test number
	double *dTestLL, // Low Limit
	int *nTestLL_Status, // Low Limit status (valid or invalid)
	double *dTestHL,	// High Limit 
	int *nTestHL_Status) // High Limit status (valid or invalid)
{
	int nStatus=0;

	// probably need a 'test_func' but optional for the moment

	/* Retrieve test number */
	// *nTestNb = ...

	/* Retrieve test LL */
	// *nTestLL_Status = TEST_LIMIT_VALID;
	// *nTestLL_Status = TEST_LIMIT_INVALID;
	
	/* Retrieve test HL */
	// *nTestHL_Status = TEST_LIMIT_VALID;
	// *nTestHL_Status = TEST_LIMIT_INVALID;

	return 0;
}

/*
 ======================================================================
 Get some node information: 
 most important are:
 - puiNbSites: number of valid sites for the sublot/lot
 - pnSiteNumbers: table of 256 int (default to -1) that must give the sites numbers: 
 	example: 0 1 2 3 ...
 Return Codes : 1 if successfull, 0 else.
*/
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
#ifdef _DEBUG
	printf("tester_GetNodeInfo\n");
	printf("tester_GetNodeInfo: active sites: ");
	for (int i=0; i<CSC_MAX_SITES; i++)
		printf("%d", active_site[i]);
	printf("\n");

	printf("tester_GetNodeInfo: store active sites: ");
	for (i=0; i<CSC_MAX_SITES; i++)
		printf("%d", store_active_site[i]);
	printf("\n");

	printf("tester_GetNodeInfo: current active sites: ");
	for (i=0; i<CSC_MAX_SITES; i++)
		printf("%d", current_active_site[i]);
	printf("\n");

	/*
	// Seen in the pdf doc but not in header...
	STRING VATEVersion;
	if (GetVATEVersion(VATEVersion))
		printf("VisualATE version: %s\n", (const char*)VATEVersion);
	*/
	STRING wip;
	if (Get_wafer_in_progress(wip))
		printf("tester_GetNodeInfo: Wafer in progress=%s\n", (const char*)wip);
	STRING wid;
	if (Get_wafer_id(wid))
		printf("tester_GetNodeInfo: Wafer ID=%s\n", (const char*)wid);
	STRING pmc;
	if (GetProgramModeCode(pmc))
		printf("tester_GetNodeInfo: ProgramModeCode=%s\n", (const char*)pmc);

	if (!puiStationNb || !puiNbSites || !pnSiteNumbers || !szHostID || !szNodeName)
		OpenErrorMessage(STRING("GalaxySemi GetNodeInfo warning: some pointers null!"));

#endif

	char szTesterHFile[] = "gtl_core_image.h";
	int	nValidSites, nFirstValidSite, nMaxSites, nSiteMask;
	unsigned int	uiSiteIndex=0, uiSiteNb;

	strcpy(szHostID, "");
	strcpy(szNodeName, "");
	strcpy(szUserName, "");
	strcpy(szTesterType, "");
	strcpy(szTesterExec, "");
	strcpy(szTestJobName, "");
	strcpy(szTestJobFile, "");
	strcpy(szTestJobPath, "");
	strcpy(szTestSourceFilesPath, "");

	/* Get station Nb */
	// Todo : WT : I did not find anything in the ASL doc
	// Patrick : how to retrieve the station number ?
	*puiStationNb = 1; 

	/* Get site information */
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


	// Get host ID : a unique Id of the host : optional for the moment
	sprintf(szHostID, "?"); 
	/*
	//DWORD nid=GetNetworkID();
	// http://msdn.microsoft.com/en-us/library/ms738520%28v=VS.85%29.aspx
	struct addrinfo hints;
	struct addrinfo *result = NULL;
	DWORD dwRetval = getaddrinfo("", "", &hints, &result);
    if ( dwRetval != 0 ) {
        printf("getaddrinfo failed with error: %d\n", dwRetval);
        WSACleanup();
        return 1;
    }
	//sprintf(szHostID, "0x%08lx", gethostid());
	*/



	/* Get Node Name */
	// No node name found in ASL API doc.
	// But Maxim seems to use ComputerName...
	char comp_name[20]; 
	DWORD comp_name_size= 20;
	// in Patrick's VM, returns : VM-TMT-PHH
	if (GetComputerName(comp_name, &comp_name_size)) 
		strcpy(szNodeName, comp_name);
	else
		strcpy(szNodeName, "?");
	#ifdef _DEBUG
		printf("tester_GetNodeInfo: Node Name=%s\n", szNodeName);
	#endif




	// Get user name
	UserInfo dat;
	if (GetUserInfoObject(&dat))
	{	
		#ifdef _DEBUG
			if (dat.GetLoggedOnStatusString())
				printf("tester_GetNodeInfo: User status: %s\n", dat.GetLoggedOnStatusString() );			
		#endif
		if (dat.GetUserName())
			sprintf(szUserName, dat.GetUserName() );
		else
			sprintf(szUserName, "?");
	}
	else
	{
		printf("Failed to retrieve UserInfo from ASL\n");
		SendErrorMessage(STRING("GalaxySemi GetNodeInfo: Failed to retrieve UserInfo from ASL")); // in ASL UI.h
		sprintf(szUserName, "unknown" );
	}

	#ifdef _DEBUG 
		printf("tester_GetNodeInfo: username=%s\n", szUserName);
	#endif




	/* Get Tester type */
	// Patrick : how to know the tester type : ASL1000, ASL3000,...
	#ifdef _DEBUG
		// seen in MVXML_TOOLS.C
		//printf("tester_GetNodeInfo: Handler name=%s\n", g_mvxml_handler_name);
		//printf("tester_GetNodeInfo: Tester ID=%s\n", g_mvxml_tester_id);

		user_strings User_String;
		if (GetUserStringsStruct(&User_String))
			printf("tester_GetNodeInfo: handler name=%s\n", User_String.handler_name);
		STRING dn;
		if (GetDeviceName(dn))
			printf("tester_GetNodeInfo: DeviceName=%s\n", (const char*) dn);
	#endif
	// Seen in Board.h
	//DetermineWhichAsl(); // but not used anywhere
	sprintf(szTesterType, "unknown");
	// Seen some defines : ASL_CFG_1000, ASL_CFG_2000, ASL_CFG_3000 in a ASL header
	// Seen in mvxml.h : MVXML_ASL_1000_STR
	///sprintf(szTesterType, MVXML_ASL_1000_STR); // should be "ASL1000" on Patrick's VM ...
	#ifdef _DEBUG
		printf("tester_GetNodeInfo: TesterType=%s\n", szTesterType); 
	#endif


	
	
	
	/* Get Tester exec */
	sprintf(szTesterExec, ""); // empty on Patrick's VM	
	#ifdef _DEBUG
		STRING pr;
		if (GetProgramRevision(pr))
			printf("tester_GetNodeInfo: ProgramRev=%s\n", (const char*)pr);	
	#endif





	// Get Job Name
	// No function found in ASL API doc except ProgramName
	STRING pn;
	if(GetProgramName(pn))
		sprintf(szTestJobName, (const char*)pn);	
	else
		strcpy(szTestJobName, "unknown");
	#ifdef _DEBUG
		printf("tester_GetNodeInfo: ProgramName=%s\n", (const char*)pn);	
	#endif
	
		
		
		
		
	// ProductID : Patrick : how to get the product ?
	/* 
		// Seen in mvxml.h : would need to link with mvxml lib...
		if (g_mvxml_product_id)
			printf("tester_GetNodeInfo: productID=%s\n", g_mvxml_product_id);
	*/



	// Get Job Path : path to the sources of the program test 
	// Optional for the moment
	/*
		strcpy(szTestSourceFilesPath, szFilePath);
		pChar = (char *) ?
		if(pChar != NULL)
		{
			strcpy(szTmpProgDir, pChar);
			if(strcmp(szTmpProgDir, szTesterHFile))
			{
				szTmpProgDir[strlen(szTmpProgDir) - strlen(szTesterHFile)] = '\0';
				strcpy(szTestSourceFilesPath, szTmpProgDir);
			}
		}
	*/

	// path to gtl_tester.conf
	sprintf(szTestSourceFilesPath, "c:/ASL_NT/System/Bin");
	return 1;
}



/*======================================================================*/
/* Get some production information	 									*/
/*																		*/
/* Return Codes : 1 if successfull, 0 else								*/
/*======================================================================*/
extern "C"
int tester_GetProdInfo(char* szOperatorName, 
					   char* szJobRevision, 
					   char*  szLotID, 
					   char*  szSublotID, 
					   char*  szProductID)
{
	strcpy(szOperatorName, "");
	strcpy(szJobRevision, "");
	strcpy(szLotID, "");
	strcpy(szSublotID, "");
	strcpy(szProductID, "");

	/* Get operator name */
	UserInfo dat;
	if (GetUserInfoObject(&dat))
	{	
		#ifdef _DEBUG
			if (!dat.LoggedOnAsOperator())
				printf("tester_GetProdInfo: user not logged as operator !\n");
			if (!dat.HasOperatorPriv())
				printf("tester_GetProdInfo: user has no operator priviledge !\n");
		#endif
		sprintf(szOperatorName, dat.GetUserName() );
	}
	else
		sprintf(szOperatorName, "?" );


	// Get job revision
	sprintf(szJobRevision, "unknown");
	STRING pr;
	if (GetProgramRevision(pr))
		sprintf(szJobRevision, (const char*)pr);
	#ifdef _DEBUG
		printf("tester_GetProdInfo: JobRev= %s\n", szJobRevision);
	#endif


	// Get lot ID
	STRING lotid;
	if (GetLotIDName(lotid))
		sprintf(szLotID, (const char*)lotid);
	else
		sprintf(szLotID, "?");

	/* Get sublot ID */
	STRING sublotname;
	if (Get_sublot_name(sublotname))
		sprintf(szSublotID, (const char*)sublotname);
	else
		sprintf(szSublotID, "?");

	/* Get product ID : TODO
		Seen in mvxml.h : 
		- testOccurrence_t : char *productID;
		- lotHistoryEvent_t : char *productID;
		Seen in MVXML_TOOLS.C:
		- char g_mvxml_product_id[MVXML_STRNLEN] = "";
	*/
	sprintf(szProductID, "unknown");

	return 1;
}


/*======================================================================*/
/* Print message to tester screen										*/
//	if timeout = 0, no wait, just printf
//	if timeout < 0, ask for an action from user
//	if timeout > 0,
// TODO : search for ASL console output ? 
/* Return Codes : none													*/
/*======================================================================*/
extern "C"
void tester_Printf(char* szMessage, int nTimeout)
{
	int nRead;
	int	nStartTime=0, nSecondsLeft=0;

	// Seen in UI.h: aslPauseProgram(), aslResumeProgram(), StopProgram(), ...

	printf("%s", szMessage);
	fflush(stdout);
	
	if(nTimeout < 0)
	{
		printf("\nPlease hit the 'c' key (+ Enter) to continue...");
		fflush(stdout);
		fflush(stdin);
		nRead = getchar();
		while((nRead != 'c') && (nRead != 'C'))
		{
			printf("Please hit the 'c' key (+ Enter) to continue...");
			fflush(stdout);
			fflush(stdin);
			nRead = getchar();
		}
	}
	else if(nTimeout > 0)
	{
		printf("Timeout > 0\n");
		// todo ?
		/*
		nStartTime = tester_ReadTimer(0);
		printf("\nPlease hit the 'c' key (+ Enter) to continue. Testing will resume automatically in %d seconds...", nTimeout);
		fflush(stdout);
		fflush(stdin);
		nRead = getchar();
		nSecondsLeft = nTimeout - tester_ReadTimer(nStartTime)/1000000;
		while((nSecondsLeft > 0) && ((nRead != 'c') && (nRead != 'C')))
		{
			printf("\nPlease hit the 'c' key (+ Enter) to continue. Testing will resume automatically in %d seconds...", nSecondsLeft);
			fflush(stdout);
			fflush(stdin);
			nRead = getchar();
			nSecondsLeft = nTimeout - tester_ReadTimer(nStartTime)/1000000;
		}
		*/
	}
}


/*======================================================================*/
/* Returns the difference between the value passed as parameter and the */
/* tester timer (measured in microseconds). 							*/
/* To get a time interval, this function must be called twice : 		*/
/*																		*/
/* t1 = tester_ReadTimer(0);											*/
/* dt = tester_ReadTimer(t1); 											*/
/*																		*/
/* Return Codes : 														*/
/* Difference between the timer (in microsec) and the parameter value 	*/
/*======================================================================*/
int tester_ReadTimer(int iRef)
{
	// todo : implement me with win32 timer ?
	return 0;
}


/*======================================================================
 Get infos on sites (single OR multi-site)
 - site mask is a 1D int table with site info : 0 = off 1 = on
 - Valid Sites is the number of valid sites
 - FirstValidSite is the index/pos of the first valid site
 - MaxSites is the index of the last valid site
 Example: 
	SitesMask = 00110010
	valid sites = 3
	first = 2
	max = 6
  ======================================================================*/
void tester_GetSiteInfo(
	int* pnValidSites, 
	int* pnFirstValidSite, 
	int* pnMaxSites, 
	int* pnSiteMask)
{	
	int nSiteMask=0, i=0;

	*pnValidSites = 0;
	*pnFirstValidSite = -1;
	*pnMaxSites = 0;
	*pnSiteMask = 0;

	/* Seen in mvxml:
		int mvxml_get_active_sites(unsigned char *mvxml_active_sites);
		bool g_mvxml_active_site[MVXML_NUM_SITES];
		mvxml_number_of_sites(MVXML_NUM_SITES);
	*/

	int site_num=0;
	for (site_num=0; site_num<CSC_MAX_SITES; site_num++)
	{
		if (active_site[site_num]==TRUE)
		{
			*pnSiteMask|=(0x01<<site_num);
			(*pnValidSites)++;
			if (*pnFirstValidSite==-1) 
				*pnFirstValidSite=site_num;
			*pnMaxSites=site_num;
		}
	}
	
	if (*pnValidSites==0)
	{
		*pnSiteMask=0x01;
		*pnValidSites=1;
		*pnFirstValidSite=0;
		*pnMaxSites=0;
	}

	#ifdef _DEBUG
		printf("tester_GetSiteInfo: %d sites found\n", *pnValidSites);
	#endif

}

/*======================================================================*/
/* Returns the site mask of enabled sites (multi-site support)			*/
// Exple : 001110000000000
/*======================================================================*/
int tester_GetEnabledSitemask()
{
	// Not used anymore ?
	return 0;
}

/*======================================================================*/
/* Returns the current site. Must be called from within a serial		*/
/* (multi-site support) 												*/
// For the current run or test or part : a number between 0 and 32 ?
/*======================================================================*/
int tester_GetCurrentSite()
{
	// Deprecated ?
	return 0;
}


/*======================================================================*/
/* Gets current test condition (name) 									*/
/*																		*/
/* Return Codes : none													*/
/*======================================================================*/
void tester_GetTestCondName(char* szTestCondition)
{
	// ToDo : BG

	char *szString="";
	//szString = read_test_condition_name();
	sprintf(szString, "");
	if(szString == NULL)
		strcpy(szTestCondition, "");
	else
		strcpy(szTestCondition, szString);
}


/*======================================================================*/
/* Gets current test condition (index)									*/
/*																		*/
/* Return Codes : index of test condition (0 if no test condition)		*/
/*======================================================================*/
int tester_GetTestCondIndex()
{
	// TODO : BG
	return 0;	//read_test_condition();
}

/*======================================================================*/
/* Read current lot (LotID, a string)													*/
/*																		*/
/* Return Codes : none													*/
/*======================================================================*/
void tester_ReadLot(char* szLot)
{
	STRING lotid;
	if (GetLotIDName(lotid))
		sprintf(szLot, (const char*)lotid);
	else
		sprintf(szLot, "?");
}

/*======================================================================*/
/* Read current sublot													*/
/*																		*/
/* Return Codes : none													*/
/*======================================================================*/
void tester_ReadSublot(char* szSublot)
{
	STRING sublotname;
	if (Get_sublot_name(sublotname))
		sprintf(szSublot, (const char*)sublotname);
	else
		sprintf(szSublot, "?");
}


/*
//real serial number, from Multisite_lib.cpp
void GetSerialNum(STRING &sernumstr, int this_site)
{
	GetSerialNum(sernumstr);
	int sernum = atoi(sernumstr) - 1;
	for (int site=0; site < CSC_MAX_SITES; site++)
		if (store_active_site[site]) {
			sernum++;
			if (site == this_site)
				site = CSC_MAX_SITES;
		}
	char sernumstr_[10];
	sprintf(sernumstr_,"%d",sernum);
	sernumstr = sernumstr_;
}
*/
extern void GetSerialNum(STRING &sernumstr, int this_site);

/*======================================================================*/
/* Read current device number for specified site						*/
/*																		*/
/* Return Codes : device nb or -1										*/
/*======================================================================*/
int tester_ReadPartNb(int nSite)
{
	// todo : Validate me
	int nDeviceNb = -1;
	STRING sn;
	GetSerialNum(sn, nSite);
	int r=atoi((const char*)sn);
	if (r!=0)
		nDeviceNb=r;
	return nDeviceNb;
}


/*======================================================================*/
/* Read max device number under test									*/
/*																	*/
/* Return Codes : device nb												*/
/*======================================================================*/
int tester_ReadMaxPartNb()
{
	int nMaxDeviceNb = 0, nDeviceNb=0;
	// todo : BG

	return nMaxDeviceNb;
}


/*======================================================================*/
/* Read current x,y and device number 									*/
/*																		*/
/* Return Codes : none													*/
/*======================================================================*/
void tester_ReadPartInfos(int *pnX, int *pnY, int *pnPartNb)
{
	// Todo : BG
}


/*======================================================================*/
/* Get environment variable							                    */
/* The function getvar() doesn't work when called from the test program */
/*                                                                      */
/* Return Codes : 1 if variable defined, 0 else                         */
/*======================================================================*/
int tester_GetEnv(char* szVarName, char* szEnvValue)
{
	char 	szBuffer[256]="";
	FILE 	*fpEnvFile=0;
	int		nReturn = 0;

	/* Init with empty string */
	strcpy(szEnvValue, "");

	szEnvValue=getenv(szVarName);
	if (szEnvValue)
		nReturn=1;

	return nReturn;
}

/*======================================================================*/
/* Baseline re-started. Perform cleanup if applicable.					*/
/*                                                                      */
/* Return Codes : none													*/
/*======================================================================*/
void tester_Baseline_Restart()
{
	// Todo : BG
	printf("tester_Baseline_Restart\n");
	//gtl_baseline_restart();
}
