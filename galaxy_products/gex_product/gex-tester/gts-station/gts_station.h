/****************************************************************************************/
/* Copyright Galaxy                                                                     */
/* This computer program is protected by copyright law                                  */
/* and international treaties. Unauthorized reproduction or                             */
/* distribution of this program, or any portion of it,may                               */
/* result in severe civil and criminal penalties, and will be                           */
/* prosecuted to the maximum extent possible under the law.                             */
/****************************************************************************************/
/*                                                                                      */
/* Module: gts_station.h                                                                */
/*                                                                                      */
/****************************************************************************************/
/*                                                                                      */
/* Notes:                                                                               */
/*                                                                                      */
/****************************************************************************************/
/*                                                                                      */
/* Revision history:                                                                    */
/*                                                                                      */
/* o [BG] 31 May 2005: Created.                                                         */
/*                                                                                      */
/****************************************************************************************/

#ifndef _GTS_STATION_H_
#define _GTS_STATION_H_

/*======================================================================================*/
/* Includes                                                                             */
/*======================================================================================*/

/*======================================================================================*/
/* Defines                                                                              */
/*======================================================================================*/

/*======================================================================================*/
/* PUBLIC Functions                                                                     */
/*======================================================================================*/
#ifdef _GTS_STATION_MODULE_

extern "C"
{
	int			gts_GetStationInfo(unsigned int *puiStationNb, unsigned int *puiNbSites, int *pnSiteNumbers, char *szHostID, char *szNodeName, char *szUserName, char *szTesterType, char *szTesterExec, char *szTestJobName, char *szTestJobFile, char *szTestJobPath, char *szTestSourceFilesPath);
	int			gts_GetProdInfo(char *szOperatorName, char *szJobRevision, char *szLotID, char *szSublotID, char *szProductID);
	void		gts_Printf(char *szMessage, int nBlocking);
	void		gtl_baseline_restart();
}

#else

#if defined(__cplusplus)

extern "C"
{
	int			gts_GetStationInfo(unsigned int *puiStationNb, unsigned int *puiNbSites, int *pnSiteNumbers, char *szHostID, char *szNodeName, char *szUserName, char *szTesterType, char *szTesterExec, char *szTestJobName, char *szTestJobFile, char *szTestJobPath, char *szTestSourceFilesPath);
	int			gts_GetProdInfo(char *szOperatorName, char *szJobRevision, char *szLotID, char *szSublotID, char *szProductID);
	void		gts_Printf(char *szMessage, int nBlocking);
	void		gtl_baseline_restart();
}

#else

extern int		gts_GetStationInfo();
extern int		gts_GetProdInfo();
extern void		gts_Printf();
extern void		gtl_baseline_restart();

#endif /* #if defined(__cplusplus) */

#endif /* #ifdef _GTS_STATION_MODULE_ */

#endif /* #ifndef _GTS_STATION_H_ */
