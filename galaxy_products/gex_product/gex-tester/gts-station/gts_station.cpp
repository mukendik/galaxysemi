/****************************************************************************************/
/* Copyright Galaxy                                                                     */
/* This computer program is protected by copyright law                                  */
/* and international treaties. Unauthorized reproduction or                             */
/* distribution of this program, or any portion of it,may                               */
/* result in severe civil and criminal penalties, and will be                           */
/* prosecuted to the maximum extent possible under the law.                             */
/****************************************************************************************/
/*                                                                                      */
/* Module: gts_station.c                                                                */
/*                                                                                      */
/****************************************************************************************/
/*                                                                                      */
/* Revision history:                                                                    */
/*                                                                                      */
/* o [BG] 31 May 2005: Created.                                                         */
/*                                                                                      */
/****************************************************************************************/

#define _GTS_STATION_MODULE_

/*======================================================================================*/
/* Includes                                                                             */
/*======================================================================================*/
#include <gstdl_systeminfo.h>

#include "gts_station.h"
#include "gts_station_mainwindow.h"


/*======================================================================================*/
/* Defines                                                                              */
/*======================================================================================*/

/*======================================================================================*/
/* Type definitions :					                                                */
/*======================================================================================*/

/*======================================================================================*/
/* EXTERN Variables                                                                     */
/*======================================================================================*/
// From main.cpp
extern GtsStationMainwindow *pMainWindow;		// Ptr to main station window

/*======================================================================================*/
/* PUBLIC Variables : declaration                                                       */
/*======================================================================================*/

/*======================================================================================*/
/* PRIVATE Variables : declaration                                                      */
/*======================================================================================*/

/*======================================================================================*/
/* PRIVATE Functions : declaration                                                      */
/*======================================================================================*/

/*======================================================================================*/
/* PUBLIC Functions                                                                     */
/*======================================================================================*/
/*======================================================================================*/
/* Description  : returns station information (station nb, node name...)                */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes : 1 if successful, 0 else                                               */
/*======================================================================================*/
int gts_GetStationInfo(unsigned int *puiStationNb, unsigned int *puiNbSites, int *pnSiteNumbers, char *szHostID, char *szNodeName, char *szUserName, char *szTesterType, char *szTesterExec, char *szTestJobName, char *szTestJobFile, char *szTestJobPath, char *szTestSourceFilesPath)
{
    CGSystemInfo	clSysInfo;
	QString			strTesterExec, strProductID, strTestJobName, strTestJobFile, strTestJobPath, strTestSourceFilesPath;

	// Retrieve information from station window
	*puiStationNb = pMainWindow->StationNb();
	*puiNbSites = pMainWindow->NbSites();
	pMainWindow->FillSiteNumbersArray(pnSiteNumbers);
	pMainWindow->ApplicationName(strTesterExec);
	pMainWindow->TestJobName(strTestJobName);
	pMainWindow->TestJobFile(strTestJobFile);
	pMainWindow->TestJobPath(strTestJobPath);
	pMainWindow->TestSourceFilesPath(strTestSourceFilesPath);
	pMainWindow->ProductID(strProductID);

    // Resd system information
    if(!clSysInfo.ReadSystemInfo())
        return 0;

    strcpy(szHostID, clSysInfo.m_strHostID.c_str());
    strcpy(szNodeName, clSysInfo.m_strHostName.c_str());
    strcpy(szUserName, clSysInfo.m_strAccountName.c_str());
	strcpy(szTesterType, "GTS");
	strcpy(szTesterExec, strTesterExec.latin1());
	strcpy(szTestJobName, strTestJobName.latin1());
	strcpy(szTestJobFile, strTestJobFile.latin1());
	strcpy(szTestJobPath, strTestJobPath.latin1());
	strcpy(szTestSourceFilesPath, strTestSourceFilesPath.latin1());

    return 1;
}

/*======================================================================================*/
/* Description  : returns production information (operator name, lot ID...)				*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes : 1 if successful, 0 else                                               */
/*======================================================================================*/
int gts_GetProdInfo(char *szOperatorName, char *szJobRevision, char *szLotID, char *szSublotID, char *szProductID)
{
	QString			strOperatorName, strJobRevision, strLotID, strSublotID, strProductID;
	
	// Retrieve information from station window
	pMainWindow->OperatorName(strOperatorName);
	pMainWindow->JobRevision(strJobRevision);
	pMainWindow->LotID(strLotID);
	pMainWindow->SublotID(strSublotID);
	pMainWindow->ProductID(strProductID);

	strcpy(szOperatorName, strOperatorName.latin1());
	strcpy(szJobRevision, strJobRevision.latin1());
	strcpy(szLotID, strLotID.latin1());
	strcpy(szSublotID, strSublotID.latin1());
	strcpy(szProductID, strProductID.latin1());

	return 1;
}

/*======================================================================================*/
/* Description  : prints a message to tester output                                     */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes : none                                                                  */
/*======================================================================================*/
void gts_Printf(char *szMessage, int nTimeout)
{
	pMainWindow->Printf(szMessage, nTimeout);
}

/*======================================================================================*/
/* Description  : user defined function when baseline is re-started						*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes : none                                                                  */
/*======================================================================================*/
void gtl_baseline_restart()
{
    pMainWindow->Printf(QString("Function gtl_baseline_restart() called...\n").toLatin1().data(), 0);
}


