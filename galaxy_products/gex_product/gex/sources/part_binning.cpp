#include "cpart_binning.h"
#include "cpart_info.h"
#include "report_build.h"
#include <gqtl_log.h>
#include "gex_group_of_files.h"

/////////////////////////////////////////////////////////////////////////////
// Class used to filter the Parts to process (eg: bin 1-7)
/////////////////////////////////////////////////////////////////////////////
CPartBinning::CPartBinning()
{
    m_nRunIndex				= 0;
    mGroupOfFile = 0;

    // Will be set during execution, if parts matching
    // the Filter criteria are found.
    bPartsFiltered = false;
        bIgnorePartsFiltered = false;

    // Reset to 0 all maping info: meaning any site remains aligned to the default partID
    resetPart();
}

/////////////////////////////////////////////////////////////////////////////
// Destructor: free memory buffer
/////////////////////////////////////////////////////////////////////////////
CPartBinning::~CPartBinning()
{
}

/////////////////////////////////////////////////////////////////////////////
// Reset the run index offset for the current group
/////////////////////////////////////////////////////////////////////////////
/*void CPartBinning::resetGroupRunOffset()
{
    m_nGroupRunOffset = 0;
}*/

/////////////////////////////////////////////////////////////////////////////
// Update the run index offset for the current group
/////////////////////////////////////////////////////////////////////////////
void CPartBinning::updateGroupRunOffset(int nGroupRunOffset)
{
    mGroupOfFile->m_nGroupRunOffset += nGroupRunOffset;
}

/////////////////////////////////////////////////////////////////////////////
// Return the current run offset for the group
/////////////////////////////////////////////////////////////////////////////
int CPartBinning::groupRunOffset()
{
    return  mGroupOfFile->m_nGroupRunOffset;
}

/////////////////////////////////////////////////////////////////////////////
// return the count of run for this file
/////////////////////////////////////////////////////////////////////////////
int CPartBinning::runCount() const
{
    return m_nRunIndex;
}

/////////////////////////////////////////////////////////////////////////////
// Return the part number according the given site ID
//
// Run index is calculated from the base run index plus the offset corresponding
// to the site ID.
/////////////////////////////////////////////////////////////////////////////
int CPartBinning::partNumber(int nSite) const
{
    if (nSite < 0)
        nSite = 0;

    if (nSite > 65535)
    {
        GSLOG(SYSLOG_SEV_ERROR, "CPartBinning::partNumber : nSite > 65535");
        nSite = 65535;
    }

    return m_lBasePartNumber + siteMapping(nSite);
}

/////////////////////////////////////////////////////////////////////////////
// return the run index according the given part number
//
// Run index is calculated from the run index in the file plus the offset
// corresponding to the run index in the group
/////////////////////////////////////////////////////////////////////////////
int CPartBinning::runIndex(int nPartNumber) const
{
    GEX_ASSERT(nPartNumber >= 0);

    if( m_BinCellMap.find( nPartNumber ) != m_BinCellMap.end() )
    {
        if( m_BinCellMap.at( nPartNumber ).nSampleIndex != -1 )
            return
                m_BinCellMap.at( nPartNumber ).nSampleIndex + mGroupOfFile->m_nGroupRunOffset;
        else
            return m_BinCellMap.at( nPartNumber ).nSampleIndex;
    }

    return -1;
}

/////////////////////////////////////////////////////////////////////////////
// return the run index according the given site ID
//
// Run index is calculated from the run index in the file plus the offset
// corresponding to the run index in the group
/////////////////////////////////////////////////////////////////////////////
int CPartBinning::runIndexFromSite(int nSite) const
{
    int nPartNumber = partNumber(nSite);

    if (nPartNumber >= 0)
    {
        if( m_BinCellMap.find( nPartNumber ) != m_BinCellMap.end() )
        {
            if (m_BinCellMap.at( nPartNumber ).nSampleIndex != -1)
                return
                    m_BinCellMap.at( nPartNumber ).nSampleIndex + mGroupOfFile->m_nGroupRunOffset;
            else
                return m_BinCellMap.at( nPartNumber ).nSampleIndex;
        }
    }

    GSLOG(SYSLOG_SEV_ERROR, QString("Part number %1 is invalid").arg( nPartNumber).toLatin1().constData());

    return -1;
}

///////////////////////////////////////////////////////////
// Increment the base part number and the base run index, called after each last PRR in a block PIR/PRR
///////////////////////////////////////////////////////////
void CPartBinning::nextPartNumber()
{
    // Decrement PIR nested level as we are exiting from a PIR/PRR block
    m_nPIRNestedLevel--;

    if (m_nPIRNestedLevel < 0)
        GSLOG(SYSLOG_SEV_WARNING, "PIR nested level lesser than 0...");

    // If we exit from the last PRR in the block, increment the base part number
    if (m_nPIRNestedLevel == 0)
    {
        m_lBasePartNumber	+= m_Sites.count();

        // Clear the site mapping table
        clearSiteMapping();
    }
}

///////////////////////////////////////////////////////////
// Reset information ecxept BinArrayCells
///////////////////////////////////////////////////////////
void CPartBinning::resetPart()
{
    m_lBasePartNumber	= 1;
    m_nPIRNestedLevel	= 0;

    clearSiteMapping();
}

///////////////////////////////////////////////////////////
//	return the PIR nested level
///////////////////////////////////////////////////////////
int CPartBinning::PIRNestedLevel() const
{
    return m_nPIRNestedLevel;
}

///////////////////////////////////////////////////////////
// Add a new site mapping
///////////////////////////////////////////////////////////
void CPartBinning::addSite(unsigned short site)
{
    // if this site is already mapped, then ignore it
    if (m_Sites.contains(site) == false)
    {
        // Keep track of PIR/PRR nested levels.
        if (m_nPIRNestedLevel > 0)
            m_nPIRNestedLevel++;
        else
            m_nPIRNestedLevel = 1;
    }
    else
        GSLOG(SYSLOG_SEV_NOTICE, QString("Site %1 already defined for part %2").arg(site).arg(partNumber(site)).toLatin1().data() );

    m_SiteMapping.insert(site, m_Sites.count());
    m_Sites.append(site);
}

///////////////////////////////////////////////////////////
// return the mapping offset for this site
///////////////////////////////////////////////////////////
int CPartBinning::siteMapping(unsigned short site) const
{
    if (m_SiteMapping.contains(site))
        return m_SiteMapping.value(site);

    return 0;
}

///////////////////////////////////////////////////////////
// Clear the mapping site table
///////////////////////////////////////////////////////////
void CPartBinning::clearSiteMapping()
{
    m_Sites.clear();
    m_SiteMapping.clear();
}

/////////////////////////////////////////////////////////////////////////////
// Saves binning result for current part#
/////////////////////////////////////////////////////////////////////////////
bool CPartBinning::saveBinning(CPartInfo *pPartInfo, int processBins)
{
    // Save Bin result for this part#.
    m_BinCellMap[pPartInfo->lPartNumber].iSoftBin      = pPartInfo->iSoftBin;
    m_BinCellMap[pPartInfo->lPartNumber].iHardBin      = pPartInfo->iHardBin;
    m_BinCellMap[pPartInfo->lPartNumber].bPass         = pPartInfo->bPass;
    m_BinCellMap[pPartInfo->lPartNumber].nTestExecuted	= pPartInfo->iTestsExecuted;

    if (IsFilteredPart(&ReportOptions, pPartInfo->m_site, false, processBins) == false)
    {
        m_BinCellMap[pPartInfo->lPartNumber].nSampleIndex = m_nRunIndex;

        // Increment the next sample index
        m_nRunIndex++;
    }
    else
        m_BinCellMap[pPartInfo->lPartNumber].nSampleIndex = -1;

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// Tells if current part should be ignored...
/////////////////////////////////////////////////////////////////////////////
bool CPartBinning::IsFilteredPart(CReportOptions * pReportOptions,
                                  int iSite/*=0*/,
                                  bool bSampleDataRecord /*= false*/,
                                  int processBins /*=-1*/)
{
    int		iSoftBin;
    int		iHardBin;
    bool	bPass           = false;
    int     lTestExecuted   = 0;
    long	lPartToCheck;

    lPartToCheck = partNumber(iSite);	// Check current part (standard call)

    iSoftBin        = m_BinCellMap[lPartToCheck].iSoftBin;
    iHardBin        = m_BinCellMap[lPartToCheck].iHardBin;
    bPass           = m_BinCellMap[lPartToCheck].bPass;
    lTestExecuted   = m_BinCellMap[lPartToCheck].nTestExecuted;

    if(iSoftBin == 65535)
        iSoftBin = iHardBin;

    // Check if valid binning
    if((iHardBin > 32767) && (iSoftBin > 32767))
    {
        if (pReportOptions->GetOption("dataprocessing", "stdf_compliancy").toString() == "flexible" &&
            lTestExecuted > 0)
        {
            GSLOG(SYSLOG_SEV_WARNING, "Invalid bin info in PRR");
        }
        else
        {
            // Ignore this part (sometimes, invalid binnings are used in PRR
            // to write all static test infos with no test results into a first
            // dummy part)
            // Flags the fact that some parts are ignored ...
            // will impact Test Statistics report
            bPartsFiltered = true;
            bIgnorePartsFiltered = true;
            return true;
        }
    }

    // Check if this binning is within the filter range...
    // Selection Index of bin type to process (0=all bins, 1=good bin, 2=fail bins, 3=custom list)

    switch(processBins)
    {
        case GEX_PROCESSPART_ALL:
        case GEX_PROCESSPART_PARTSINSIDE:		// Processed after loading
        case GEX_PROCESSPART_PARTSOUTSIDE:		// Processed after loading
        case GEX_PROCESSPART_FIRSTINSTANCE:		// Processed after loading
        case GEX_PROCESSPART_LASTINSTANCE:		// Processed after loading
            // Range= All bins
            return false;	// Bin is NOT out of range !
            break;
        case GEX_PROCESSPART_GOOD:
            // Range= Good bins.
            if(bPass == true)
                return false;	// Bin is NOT out of range !
            break;
        case GEX_PROCESSPART_FAIL:
            // Range = All bins EXCEPT Bin 1
            if(bPass == false)
                return false;	// Bin is NOT out of range !
            break;
        case GEX_PROCESSPART_SBINLIST:
            // Range of SoftBins is custom:
            if(pGexRangeList->Contains(iSoftBin) == true)
                return false;	// Bin is NOT out of range !
            break;
        case GEX_PROCESSPART_EXSBINLIST:
            // All bins except given list of SoftBins.
            if(pGexRangeList->Contains(iSoftBin) == false)
                return false;	// Bin is valid and doesn;t belong to the exclusion list!
            break;
        case GEX_PROCESSPART_HBINLIST:
            // Range of HardBins is custom:
            if(pGexRangeList->Contains(iHardBin) == true)
                return false;	// Bin is NOT out of range !
            break;
        case GEX_PROCESSPART_EXHBINLIST:
            // All bins except given list of HardBins.
            if(pGexRangeList->Contains(iHardBin) == false)
                return false;	// Bin is valid and doesn;t belong to the exclusion list!
            break;
        case GEX_PROCESSPART_PARTLIST:
            // Part or parts range
            if(pGexRangeList->Contains(lPartToCheck) == true)
                return false; // Part# is NOT out of range !
            break;
        case GEX_PROCESSPART_EXPARTLIST:
            // All Part except given list of parts
            if(pGexRangeList->Contains(lPartToCheck) == false)
                return false; // Part# is valid and doesn't belong to the exclusion list!
            break;
        case GEX_PROCESSPART_ODD: // Odd parts only (1,3,5,...)
            if(lPartToCheck % 2 == 1)
                return false; // This Part# is ODD, DO NOT filter it
            break;
        case GEX_PROCESSPART_EVEN: // Even parts only (2,4,6,...)
            if(lPartToCheck % 2 == 0)
                return false; // This Part# is EVEN, DO NOT filter it
            break;
        case GEX_PROCESSPART_NO_SAMPLES:
            return bSampleDataRecord;	// Ignore this record if it's Data Samples (eg: PTR or FTR)
            break;
    }

    // If we arrive here: Bin/part of this part is OUT of Range...so we must Ignore it.
    bPartsFiltered = true;	// Flags the fact that some parts are ignored...will impact Test Statistics report...
    return true;			// Binning outside the range: Ignore this part
}

///////////////////////////////////////////////////////////
// return true if there is at least one bin result count
///////////////////////////////////////////////////////////
bool CPartBinning::hasResultsCount() const
{
    return ! m_BinCellMap.empty();
}

