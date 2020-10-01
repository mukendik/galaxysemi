#ifndef CPART_BINNING_H
#define CPART_BINNING_H

#include <QSet>
#include "stdf.h"	// for BYTE typedef
#include <map>
#include <gqtl_utils.h>

class CGexGroupOfFiles;
class CPartInfo;

struct BinCell
{
    int		iSoftBin;			// Bin result (= SoftBin: 0-65535) initialized with -1
    int		iHardBin;			// Bin result (= HardBin: 0-65535) initialized with -1
    bool	bPass;				// Pass info
    int		nSampleIndex;		// Sample index initialized with  -1
    int     nTestExecuted;      // Number of test executed for this part

    BinCell() : iSoftBin(-1),
                iHardBin(-1),
                bPass(),
                nSampleIndex(-1),
                nTestExecuted(){}
};

//////////////////////////////////////////////////////
// Part Binning class: keeps track of the binning of each part
// Used to filter parts to process (e.g: only bin 1-5)
//////////////////////////////////////////////////////
class	CPartBinning
{
public:
    CPartBinning();
    ~CPartBinning();

    bool						saveBinning(CPartInfo *pPartInfo, int processBins);			// Tells binning result for current part.
    bool						IsFilteredPart(class CReportOptions * Settings, int iSite = 0, bool bSampleDataRecord = false, int processBins = -1);	// Returns true if current part must be ignored (filtered)
    bool						bPartsFiltered;								// true if one or more parts have been filtered (removed) from the report.
    bool                        bIgnorePartsFiltered;

    void						resetPart();								// Reset information ecxept BinArrayCells

    void						addSite(unsigned short site);						// Add a new site to this part, called in each PIR
    void						nextPartNumber();							// Increment the base part number, called after each PRR

    int							runCount() const;							// return the count of run for this file
    int							runIndex(int nPartNumber) const;			// return the run index according the given part number
    int							runIndexFromSite(int nSite) const;			// return the run index according the given site ID
    int							partNumber(int nSite = 0) const;			// return the part number according the given site ID
    bool						hasResultsCount() const;					// return true if there is at least one result count
    int							PIRNestedLevel() const;						// return the PIR nested level
    const BinCell&		resultsAt(int nPartNumber) const;			// return the result for the given part number

    void					updateGroupRunOffset(int nGroupRunOffset);	// update the offset run index. Called by CGexGroupOfFiles::Scanfiles(...) to update the run index in the group
    int              groupRunOffset();                               // Return the current run offset for the group


   void setGroupOfFile(CGexGroupOfFiles* groupOfFiles) { mGroupOfFile = groupOfFiles;}
   void setRangeList(GS::QtLib::Range* gexRangeList) {pGexRangeList = gexRangeList;}

private :

    std::map< int, BinCell > m_BinCellMap;

    QList<unsigned short>					m_Sites;									// Hold a set of the site already mapped
    QHash<unsigned short, int>		    m_SiteMapping;                              // Used to map a logical site ID to an array index (1st logical site mapper to index 0, etc...)
    int							m_lBasePartNumber;							// Tells which part# in the STDF file we are processing.
    int							m_nPIRNestedLevel;							// Incremented when adding a new site mapping, decremented when going to next part. Used to know
                                                                            // we are processing an interlaced file.

    GS::QtLib::Range*               pGexRangeList ;
    int                             m_nRunIndex;								// Tells which run# in the file we are processing
   // static int					m_nGroupRunOffset;							// Tells the run index offset in the group

    void						clearSiteMapping();							// Clear the mapping site table
    // return the mapping offset for this site (-1 on error)
    int							siteMapping(unsigned short site) const;

    CGexGroupOfFiles* mGroupOfFile;


};

#endif // CPART_BINNING_H
