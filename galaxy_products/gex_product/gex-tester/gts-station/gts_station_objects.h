/****************************************************************************************/
/* Copyright Galaxy                                                                     */
/* This computer program is protected by copyright law                                  */
/* and international treaties. Unauthorized reproduction or                             */
/* distribution of this program, or any portion of it,may                               */
/* result in severe civil and criminal penalties, and will be                           */
/* prosecuted to the maximum extent possible under the law.                             */
/****************************************************************************************/
/*                                                                                      */
/* Module: gts_station_objects.h                                                        */
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

#ifndef _GTS_STATION_OBJECTS_H_
#define _GTS_STATION_OBJECTS_H_

/*======================================================================================*/
/* Includes                                                                             */
/*======================================================================================*/
#include <QList>
#include <QMap>
#include <QPair>

#include <gqtl_datakeys.h>

typedef QMap< QPair<int, time_t>, QPair<QString, GS::QtLib::DatakeysContent> > GtsStdfFiles;

/*======================================================================================*/
/* Defines                                                                              */
/*======================================================================================*/

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GtsStation_Binning
// Holds information for a specific binning
/////////////////////////////////////////////////////////////////////////////////////////
class GtsStation_Binning
{
public:
	enum BinStatus
	{
		eBinningPass,
		eBinningFail,
		eBinningUnknown
	};

	GtsStation_Binning(unsigned int uiBinNb, BinStatus eBinStatus = eBinningUnknown);

	BinStatus		m_eBinStatus;		// Binning status (Pass/Fail)
	unsigned int	m_uiBinNb;			// Binning nb
	unsigned int	m_uiBinCount;		// Binning count
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GtsStation_BinningList
// Holds list of binnings
/////////////////////////////////////////////////////////////////////////////////////////
class GtsStation_BinningList : public QList<GtsStation_Binning*>
{
// CONSTRUCTOR/DESTRUCTOR
public:
	GtsStation_BinningList();
    ~GtsStation_BinningList();

	enum SortOn 
	{
		eSortOnBinNb,			// Sorting should be done on Bin nb
		eSortOnBinCount			// Sorting should be done on Bin count
	};

// OPERATORS
public:
    GtsStation_BinningList& operator=(GtsStation_BinningList& source);		// assignment operator

// PUBLIC METHODS
public:
	bool	AddBinning(unsigned int uiBinNb, GtsStation_Binning::BinStatus eBinStatus = GtsStation_Binning::eBinningUnknown);
	void	ResetCounters();
	void	IncCounter(unsigned int uiBinNb);
	bool	IsPassBinning(unsigned int uiBinNb);
    void	Sort(SortOn eSortSelector = eSortOnBinCount, bool bAscending = false);
	int		Find(const GtsStation_Binning * pBinning);

// PUBLIC DATA
public:

// PRIVATE METHODS
private:

// PRIVATE DATA
private:

// PROTECTED METHODS
protected:
    void Clear();

// PROTECTED DATA
protected:
	// For sorting
	bool			m_bSortAscending;	// TRUE if sorting should be ascending
	int				m_eSortSelector;	// Tells on which var to sort
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GtsStation_Site
// Holds information for a specific site
/////////////////////////////////////////////////////////////////////////////////////////
class GtsStation_Site
{
public:
	GtsStation_Site(unsigned int uiSiteNb);

	enum SiteStatus
	{
		eSiteIdle,
        eSiteStarted,
        eSiteResultsAvailable
	};

	SiteStatus				m_eSiteStatus;			// Current status of site
	unsigned int			m_uiSiteNb;				// Site Nb
	unsigned int			m_uiPartCount;			// Total nb of parts tested on this site
	int						m_nBinning_Soft;		// Last software binning for this site
	int						m_nBinning_Hard;		// Last hardware binning for this site
	unsigned int			m_uiPassCount;			// Total nb of PASS parts on this site
	unsigned int			m_uiFailCount;			// Total nb of FAIL parts on this site

	GtsStation_BinningList	m_listSoftBinnings;		// List of software binnings
	GtsStation_BinningList	m_listHardBinnings;		// List of hardware binnings
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GtsStation_SiteList
// Holds list of sites
/////////////////////////////////////////////////////////////////////////////////////////
class GtsStation_SiteList : public QList<GtsStation_Site*>
{
// CONSTRUCTOR/DESTRUCTOR
public:
	GtsStation_SiteList();
    ~GtsStation_SiteList();

// PUBLIC METHODS
public:
	bool			AddSite(unsigned int uiSiteNb);
    void			ResetRun();
    void			ResetRun(const unsigned int SiteNb);
    void			ResetRun(GtsStation_Site *Site);
    void			ResetAll();
	bool			CheckAllSitesStatus(GtsStation_Site::SiteStatus eSiteStatus);
	bool			CheckSiteStatus(unsigned int uiSiteNb, GtsStation_Site::SiteStatus eSiteStatus);
	void			SetSiteStatus(unsigned int uiSiteNb, GtsStation_Site::SiteStatus eSiteStatus);
	void			SetSiteResults(unsigned int uiSiteNb, int nBinning_Soft, int nBinning_Hard);
	void			SetBinningLists(GtsStation_BinningList & listSoftBinning, GtsStation_BinningList & listHardBinning);
	unsigned int	PartCount();
	unsigned int	PassCount();
	unsigned int	FailCount();

// PUBLIC DATA
public:
	GtsStation_BinningList		m_listSoftBinnings;			// List of software binnings (all sites)
	GtsStation_BinningList		m_listHardBinnings;			// List of hardware binnings (all sites)

// PRIVATE METHODS
private:

// PRIVATE DATA
private:

// PROTECTED METHODS
protected:

// PROTECTED DATA
protected:
};

#endif /* #ifndef _GTS_STATION_OBJECTS_H_ */
