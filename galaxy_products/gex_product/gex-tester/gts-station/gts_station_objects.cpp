/****************************************************************************************/
/* Copyright Galaxy                                                                     */
/* This computer program is protected by copyright law                                  */
/* and international treaties. Unauthorized reproduction or                             */
/* distribution of this program, or any portion of it,may                               */
/* result in severe civil and criminal penalties, and will be                           */
/* prosecuted to the maximum extent possible under the law.                             */
/****************************************************************************************/
/*                                                                                      */
/* Module: gts_station_objects.c                                                        */
/*                                                                                      */
/****************************************************************************************/
/*                                                                                      */
/* Notes:                                                                               */
/*                                                                                      */
/****************************************************************************************/
/*                                                                                      */
/* Revision history:                                                                    */
/*                                                                                      */
/* o [BG] 10 June 2005: Created.                                                        */
/*                                                                                      */
/****************************************************************************************/

#define _GTS_STATION_OBJECTS_MODULE_

#include <QList>

#include "gts_station_objects.h"


/*======================================================================================*/
/* Defines                                                                              */
/*======================================================================================*/

/*======================================================================================*/
/* Type definitions :					                                                */
/*======================================================================================*/

/*======================================================================================*/
/* EXTERN Variables                                                                     */
/*======================================================================================*/

/*======================================================================================*/
/* PUBLIC Variables : declaration                                                       */
/*======================================================================================*/

/*======================================================================================*/
/* PRIVATE Variables : declaration                                                      */
/*======================================================================================*/

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Compares 2 items of the site list (compare site nb, ascending order)
// Return type : true if item1 < item2
/////////////////////////////////////////////////////////////////////////////////////////
bool CompareSite(GtsStation_Site* Site1, GtsStation_Site* Site2)
{
    return Site1->m_uiSiteNb < Site2->m_uiSiteNb;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Compares 2 items of the binning list (compare bin nb, ascending order)
// Return type : true if item1 < item2
/////////////////////////////////////////////////////////////////////////////////////////
bool CompareBinNb_Asc(GtsStation_Binning* Binning1, GtsStation_Binning* Binning2)
{
    // If same Bin nb, sort by Bin count (asc)
    if(Binning1->m_uiBinNb == Binning2->m_uiBinNb)
        return Binning1->m_uiBinCount < Binning2->m_uiBinCount;
    return Binning1->m_uiBinNb < Binning2->m_uiBinNb;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Compares 2 items of the binning list (compare bin nb, descending order)
// Return type : true if item1 > item2
/////////////////////////////////////////////////////////////////////////////////////////
bool CompareBinNb_Desc(GtsStation_Binning* Binning1, GtsStation_Binning* Binning2)
{
    // If same Bin nb, sort by Bin count (asc)
    if(Binning1->m_uiBinNb == Binning2->m_uiBinNb)
        return Binning1->m_uiBinCount < Binning2->m_uiBinCount;
    return Binning1->m_uiBinNb > Binning2->m_uiBinNb;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Compares 2 items of the binning list (compare bin count, ascending order)
// Return type : true if item1 < item2
/////////////////////////////////////////////////////////////////////////////////////////
bool CompareBinCount_Asc(GtsStation_Binning* Binning1, GtsStation_Binning* Binning2)
{
    // If same Bin count, sort by Bin nb (asc)
    if(Binning1->m_uiBinCount == Binning2->m_uiBinCount)
        return Binning1->m_uiBinNb < Binning2->m_uiBinNb;
    return Binning1->m_uiBinCount < Binning2->m_uiBinCount;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Compares 2 items of the binning list (compare bin count, descending order)
// Return type : true if item1 > item2
/////////////////////////////////////////////////////////////////////////////////////////
bool CompareBinCount_Desc(GtsStation_Binning* Binning1, GtsStation_Binning* Binning2)
{
    // If same Bin count, sort by Bin nb (asc)
    if(Binning1->m_uiBinCount == Binning2->m_uiBinCount)
        return Binning1->m_uiBinNb < Binning2->m_uiBinNb;
    return Binning1->m_uiBinCount > Binning2->m_uiBinCount;
}

/////////////////////////////////////////////////////////////////////////////////////////
// BINNING OBJECT
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// Description : Constructor
//
// Argument(s) :
//
// Return type : 
/////////////////////////////////////////////////////////////////////////////////////////
GtsStation_Binning::GtsStation_Binning(unsigned int uiBinNb, BinStatus eBinStatus/* = eBinningUnknown*/)
{
	m_uiBinNb = uiBinNb;
	m_uiBinCount = 0;
	if(eBinStatus == eBinningUnknown)
	{
		if(uiBinNb == 1)
			m_eBinStatus = eBinningPass;
		else
			m_eBinStatus = eBinningFail;
	}
	else
		m_eBinStatus = eBinStatus;
}

/////////////////////////////////////////////////////////////////////////////////////////
// BINNINGLIIST OBJECT
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// Description : Constructor
//
// Argument(s) :
//
// Return type : 
/////////////////////////////////////////////////////////////////////////////////////////
GtsStation_BinningList::GtsStation_BinningList(): QList<GtsStation_Binning*>()
{
	// Set sorting parameters
    m_bSortAscending = true;
	m_eSortSelector = eSortOnBinCount;
}

GtsStation_BinningList::~GtsStation_BinningList()
{
    Clear();
}

void GtsStation_BinningList::Clear()
{
    while (!isEmpty())
         delete takeFirst();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Assignment operator
//
// Argument(s) :
//
//	const GtsStation_BinningList& source
//		reference to use for the assignement
//
// Return type : ptr to this object 
/////////////////////////////////////////////////////////////////////////////////////////
GtsStation_BinningList& GtsStation_BinningList::operator=(GtsStation_BinningList& source)
{
    // First clear list content
    Clear();

	// Go through source list, and fill current list
    GtsStation_Binning	*pBinning, *pBinning_Source;
    for(int lIndex=0; lIndex<source.size(); ++lIndex)
	{
        pBinning_Source = source.at(lIndex);
		pBinning = new GtsStation_Binning(pBinning_Source->m_uiBinNb, pBinning_Source->m_eBinStatus);
		append(pBinning);
	}

	return *this;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Sort list
//
// Argument(s) :
//
// Return type : void 
/////////////////////////////////////////////////////////////////////////////////////////
void GtsStation_BinningList::Sort(SortOn eSortSelector, bool bAscending)
{
	// Set sort parameters
	m_eSortSelector = eSortSelector;
	m_bSortAscending = bAscending;

    switch(eSortSelector)
    {
        case eSortOnBinCount:
            if(bAscending)
                qSort(begin(), end(), CompareBinCount_Asc);
            else
                qSort(begin(), end(), CompareBinCount_Desc);
            break;

        default:
        case eSortOnBinNb:
            if(bAscending)
                qSort(begin(), end(), CompareBinNb_Asc);
            else
                qSort(begin(), end(), CompareBinNb_Desc);
            break;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Find an element in the list
//
// Argument(s) :
//
// Return type : index of item in the list 
/////////////////////////////////////////////////////////////////////////////////////////
int GtsStation_BinningList::Find(const GtsStation_Binning * pBinning)
{
    for(int lIndex=0; lIndex<size(); ++lIndex)
    {
        if(at(lIndex)->m_uiBinNb == pBinning->m_uiBinNb)
            return lIndex;
    }
    return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Adds the binning to the list if it doesn't contain this bin nb.
//
// Argument(s) :
//
// Return type : true if binning added, false else (already in the list)
/////////////////////////////////////////////////////////////////////////////////////////
bool GtsStation_BinningList::AddBinning(unsigned int uiBinNb, GtsStation_Binning::BinStatus eBinStatus/* = GtsStation_Binning::eBinningUnknown*/)
{
    // Check if bin nb found in the list
    for(int lIndex=0; lIndex<size(); ++lIndex)
    {
        if(at(lIndex)->m_uiBinNb == uiBinNb)
            return false;
    }

	// Binning not found, create a new one
    GtsStation_Binning* lBinning = new GtsStation_Binning(uiBinNb, eBinStatus);
    if(lBinning)
    {
        append(lBinning);
        return true;
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Reset binning counters.
//
// Argument(s) :
//
// Return type : none 
/////////////////////////////////////////////////////////////////////////////////////////
void GtsStation_BinningList::ResetCounters()
{
    for(int lIndex=0; lIndex<size(); ++lIndex)
        at(lIndex)->m_uiBinCount = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Check if specified binning is a PASS binning.
//
// Argument(s) :
//
// Return type : TRUE if the specified binning is a PASS binning, FALSE else 
/////////////////////////////////////////////////////////////////////////////////////////
bool GtsStation_BinningList::IsPassBinning(unsigned int uiBinNb)
{
    for(int lIndex=0; lIndex<size(); ++lIndex)
    {
        if(at(lIndex)->m_uiBinNb == uiBinNb)
            return at(lIndex)->m_eBinStatus == GtsStation_Binning::eBinningPass;
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Increment counter for a specific binning.
//
// Argument(s) :
//
// Return type : none 
/////////////////////////////////////////////////////////////////////////////////////////
void GtsStation_BinningList::IncCounter(unsigned int uiBinNb)
{
    for(int lIndex=0; lIndex<size(); ++lIndex)
    {
        if(at(lIndex)->m_uiBinNb == uiBinNb)
        {
            (at(lIndex)->m_uiBinCount)++;
            return;
        }
    }

    // Binning not found, create new entry
    GtsStation_Binning* lBinning = new GtsStation_Binning(uiBinNb);
    if(lBinning)
    {
        (lBinning->m_uiBinCount)++;
        append(lBinning);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
// SITE OBJECT
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// Description : Constructor
//
// Argument(s) :
//
// Return type : 
/////////////////////////////////////////////////////////////////////////////////////////
GtsStation_Site::GtsStation_Site(unsigned int uiSiteNb)
{
	m_eSiteStatus = eSiteIdle;
	m_uiSiteNb = uiSiteNb;
	m_uiPartCount = 0;
	m_nBinning_Soft = -1;
	m_nBinning_Hard = -1;
	m_uiPassCount = 0;
	m_uiFailCount = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// SITELIIST OBJECT
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// Description : Constructor
//
// Argument(s) :
//
// Return type : 
/////////////////////////////////////////////////////////////////////////////////////////
GtsStation_SiteList::GtsStation_SiteList(): QList<GtsStation_Site*>()
{
}

GtsStation_SiteList::~GtsStation_SiteList()
{
    while (!isEmpty())
         delete takeFirst();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Adds the site to the list if it doesn't contain this site nb.
//
// Argument(s) :
//
// Return type : true if site added, false else (already in the list)
/////////////////////////////////////////////////////////////////////////////////////////
bool GtsStation_SiteList::AddSite(unsigned int uiSiteNb)
{
    // Check if site nb found in the list
    for(int lIndex=0; lIndex<size(); ++lIndex)
    {
        if(at(lIndex)->m_uiSiteNb == uiSiteNb)
            return false;
    }

    // Site not found, create a new one
    GtsStation_Site* lSite = new GtsStation_Site(uiSiteNb);
    if(lSite)
    {
        append(lSite);
        return true;
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Sets Software and hardware binning list for each site.
//
// Argument(s) :
//
// Return type : none 
/////////////////////////////////////////////////////////////////////////////////////////
void GtsStation_SiteList::SetBinningLists(GtsStation_BinningList & listSoftBinning, GtsStation_BinningList & listHardBinning)
{
    GtsStation_Site	*lSite = NULL;

	// Set global binning lists (all sites)
	m_listSoftBinnings = listSoftBinning;
	m_listHardBinnings = listHardBinning;

	// Go through all sites and set binning lists
    for(int lIndex=0; lIndex<size(); ++lIndex)
	{
        lSite = at(lIndex);
        lSite->m_listSoftBinnings = listSoftBinning;
        lSite->m_listHardBinnings = listHardBinning;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Reset run variables (for all sites).
//
// Argument(s) :
//
// Return type : none 
/////////////////////////////////////////////////////////////////////////////////////////
void GtsStation_SiteList::ResetRun()
{
	// Go through all sites and reset per run data
    for(int lIndex=0; lIndex<size(); ++lIndex)
        ResetRun(at(lIndex));
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Reset run variables ((for specific site).
//
// Argument(s) :
//
// Return type : none
/////////////////////////////////////////////////////////////////////////////////////////
void GtsStation_SiteList::ResetRun(const unsigned int SiteNb)
{
    GtsStation_Site	*lSite = NULL;

    // Go through all sites and reset per run data
    for(int lIndex=0; lIndex<size(); ++lIndex)
    {
        lSite = at(lIndex);
        if(lSite->m_uiSiteNb == SiteNb)
            ResetRun(lSite);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Reset run variables (for specific site).
//
// Argument(s) :
//
// Return type : none
/////////////////////////////////////////////////////////////////////////////////////////
void GtsStation_SiteList::ResetRun(GtsStation_Site *Site)
{
    if(!Site) return;
    Site->m_nBinning_Soft = -1;
    Site->m_nBinning_Hard = -1;
    Site->m_eSiteStatus = GtsStation_Site::eSiteIdle;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Reset alls sites.
//
// Argument(s) :
//
// Return type : none 
/////////////////////////////////////////////////////////////////////////////////////////
void GtsStation_SiteList::ResetAll()
{
    GtsStation_Site	*lSite = NULL;

	// Reset global binning lists (all sites)
	m_listSoftBinnings.ResetCounters();
	m_listHardBinnings.ResetCounters();

	// Go through all sites and reset data
    for(int lIndex=0; lIndex<size(); ++lIndex)
    {
        lSite = at(lIndex);
        lSite->m_nBinning_Soft = -1;
        lSite->m_nBinning_Hard = -1;
        lSite->m_eSiteStatus = GtsStation_Site::eSiteIdle;
        lSite->m_uiPartCount = 0;
        lSite->m_uiFailCount = 0;
        lSite->m_uiPassCount = 0;
        lSite->m_listSoftBinnings.ResetCounters();
        lSite->m_listHardBinnings.ResetCounters();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Get status of specified site.
//
// Argument(s) :
//
// Return type : TRUE if site has specified status, FALSE else 
/////////////////////////////////////////////////////////////////////////////////////////
bool GtsStation_SiteList::CheckSiteStatus(unsigned int uiSiteNb, GtsStation_Site::SiteStatus eSiteStatus)
{
    GtsStation_Site	*lSite = NULL;

	// Find specifed site, and check status
    for(int lIndex=0; lIndex<size(); ++lIndex)
    {
        lSite = at(lIndex);
        if(lSite->m_uiSiteNb == uiSiteNb)
            return lSite->m_eSiteStatus == eSiteStatus;
	}

    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Checks if all sites have specified status.
//
// Argument(s) :
//
// Return type : TRUE if all sites have specified status, FALSE else 
/////////////////////////////////////////////////////////////////////////////////////////
bool GtsStation_SiteList::CheckAllSitesStatus(GtsStation_Site::SiteStatus eSiteStatus)
{
	// Go through all sites and check their status
    for(int lIndex=0; lIndex<size(); ++lIndex)
    {
        if(at(lIndex)->m_eSiteStatus != eSiteStatus)
            return false;
	}

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Set status of a specific site.
//
// Argument(s) :
//
// Return type : none 
/////////////////////////////////////////////////////////////////////////////////////////
void GtsStation_SiteList::SetSiteStatus(unsigned int uiSiteNb, GtsStation_Site::SiteStatus eSiteStatus)
{
    GtsStation_Site	*lSite = NULL;

	// Find specified site
    for(int lIndex=0; lIndex<size(); ++lIndex)
    {
        lSite = at(lIndex);
        if(lSite->m_uiSiteNb == uiSiteNb)
		{
            lSite->m_eSiteStatus = eSiteStatus;
			return;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Set results of a specific site.
//
// Argument(s) :
//
// Return type : none 
/////////////////////////////////////////////////////////////////////////////////////////
void GtsStation_SiteList::SetSiteResults(unsigned int uiSiteNb, int nBinning_Soft, int nBinning_Hard)
{
    GtsStation_Site	*lSite = NULL;

	// Update global results (all sites)
	m_listSoftBinnings.IncCounter(nBinning_Soft);
	m_listHardBinnings.IncCounter(nBinning_Hard);

	// Find specified site
    for(int lIndex=0; lIndex<size(); ++lIndex)
    {
        lSite = at(lIndex);
        if(lSite->m_uiSiteNb == uiSiteNb)
		{
            // GCORE-590: Before updating Pass/Fail counts, first inc binning counters
            // If this is the first run, and file has no SBR/HBR, binning lists will be empty, and current binning
            // added to binnings lists during IncCounter(), with P/F status determined based on bin_no

			// SOFTWARE BINNING
            lSite->m_nBinning_Soft = nBinning_Soft;
            lSite->m_listSoftBinnings.IncCounter(nBinning_Soft);

			// HARDWARE BINNING
            lSite->m_nBinning_Hard = nBinning_Hard;
            lSite->m_listHardBinnings.IncCounter(nBinning_Hard);

            // Station found: update station results
            (lSite->m_uiPartCount)++;
            if(lSite->m_listSoftBinnings.IsPassBinning(nBinning_Soft))
                (lSite->m_uiPassCount)++;
            else
                (lSite->m_uiFailCount)++;

            // Results available for this site
            lSite->m_eSiteStatus = GtsStation_Site::eSiteResultsAvailable;

			return;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Return total part count.
//
// Argument(s) :
//
// Return type : total part count 
/////////////////////////////////////////////////////////////////////////////////////////
unsigned int GtsStation_SiteList::PartCount()
{
	unsigned int	uiPartCount = 0;

	// Go through all sites and cumul part count
    for(int lIndex=0; lIndex<size(); ++lIndex)
        uiPartCount += at(lIndex)->m_uiPartCount;

	return uiPartCount;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Return total PASS count.
//
// Argument(s) :
//
// Return type : total PASS count 
/////////////////////////////////////////////////////////////////////////////////////////
unsigned int GtsStation_SiteList::PassCount()
{
	unsigned int	uiPassCount = 0;

	// Go through all sites and cumul part count
    for(int lIndex=0; lIndex<size(); ++lIndex)
        uiPassCount += at(lIndex)->m_uiPassCount;

	return uiPassCount;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Return total FAIL count.
//
// Argument(s) :
//
// Return type : total FAIL count 
/////////////////////////////////////////////////////////////////////////////////////////
unsigned int GtsStation_SiteList::FailCount()
{
	unsigned int	uiFailCount = 0;

	// Go through all sites and cumul part count
    for(int lIndex=0; lIndex<size(); ++lIndex)
        uiFailCount += at(lIndex)->m_uiFailCount;

	return uiFailCount;
}

