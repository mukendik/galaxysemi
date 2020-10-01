#include "pat_gdbn_squeeze_algo.h"
#include "patman_lib.h"

///////////////////////////////////////////////////////////////////////////////////
// Class PatGdbnSqueezeAlgo - Class implementing GDBN algorithm with squeeze rule
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
PatGdbnSqueezeAlgo::PatGdbnSqueezeAlgo() : PatGdbnAbstractAlgorithm()
{
    m_nClusterSize      = 3;
    m_nMinimumFailCount = 2;
    m_bFailEdgeDies     = false;

}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
PatGdbnSqueezeAlgo::~PatGdbnSqueezeAlgo()
{

}

///////////////////////////////////////////////////////////
// METHODS
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool PatGdbnSqueezeAlgo::excludeDie(const CWaferMap * pWafermap, int dieIndex) const
//
// Description	:	Determine if a die should be failed as GDBN
//
///////////////////////////////////////////////////////////////////////////////////
bool PatGdbnSqueezeAlgo::excludeDie(const CWaferMap * pWafermap, int dieIndex) const
{
    bool    bExcludeDie = false;

    if (isValidDie(pWafermap, dieIndex))
    {
        bool    bEdgeDieDetected	= false;
        int     nCoordX             = GEX_WAFMAP_INVALID_COORD;
        int     nCoordY             = GEX_WAFMAP_INVALID_COORD;

        // Get die coordinates from index
        pWafermap->coordFromIndex(dieIndex, nCoordX, nCoordY);

        // Compute valid Low & High X offsets in wafermap
        int iLowX   = nCoordX - m_nClusterSize / 2;
        int iHighX  = nCoordX + m_nClusterSize / 2;

        // Compute valid Low & High Y offsets in wafermap
        int iLowY   = nCoordY - m_nClusterSize/2;
        int iHighY  = nCoordY + m_nClusterSize/2;

        // Die is on the edge of the wafer ?
        bEdgeDieDetected = pWafermap->isDieOnEdge(dieIndex, m_nEdgeDieType);

        // We only remove good dies if they are sqeezed between bad dies, as such we need to keep track of the bad die location
        // compared to the good die, we then devide the cluster zone in 8 areas, which are relative to the good bien (center):
        // Left, left-top,  letf-bottom, Right, right-top,  right-bottom, center-top,center-bottom
        int     icClusterSubZone[8];
        int     iSubZoneOffset;
        int     iBinSqueezed        = 0;
        int     iBin                = GEX_WAFMAP_EMPTY_CELL;
        int     iIndex              = 0;
        int     iFailSqueezeCount   = 0;
        bool    bValidBin           = false;

        memset(&icClusterSubZone[0],0,8*sizeof(int));

        // Check all dies in the cluster zone
        for(int iDieX = iLowX; iDieX <= iHighX; iDieX++)
        {
            for(int iDieY = iLowY; iDieY <= iHighY; iDieY++)
            {
                bValidBin	= false;
                iBin		= GEX_WAFMAP_EMPTY_CELL;

                // If die is within the wafermap array
                if (pWafermap->indexFromCoord(iIndex, iDieX, iDieY))
                {
                    iBin = ((CWaferMap *)pWafermap)->getWafMap()[iIndex].getBin();

                    // Binning is invalid, might be an out of wafer die
                    if (iBin == GEX_WAFMAP_EMPTY_CELL)
                    {
                        // Targeted die is on the edge of the wafer
                        // Option to consider out of wafer die as failed die activated
                        // Check if neighbor die is PCM or out of wafer
                        if (m_bFailEdgeDies && bEdgeDieDetected && pWafermap->isDieOutOfWafer(iIndex))
                        {
                            // Out of wafer die
                            bValidBin = true;
                        }
                    }
                    else
                    {
                        // Check if this bin belongs to our list of 'bad neighbors Bins'
                        bValidBin = isAroundBadDie(iBin, iIndex);
                    }
                }
                else if (m_bFailEdgeDies && bEdgeDieDetected)
                {
                    // Out of wafer die
                    bValidBin = true;
                }

                // If valid bin, proceed...
                if(bValidBin)
                {
                    // Update fail count: Used for Squeeze algorithm
                    iFailSqueezeCount++;

                    // Update the die count in the cluster sub-zone: Left, Right, Top, bottom, Letf-top, Top-right, etc...
                    if(iDieX < nCoordX)
                    {
                        // LEFT sub-zones
                        if(iDieY < nCoordY)
                            iSubZoneOffset = GEX_TPAT_NEIGHBOR_LEFT_TOP;
                        else if(iDieY == nCoordY)
                            iSubZoneOffset = GEX_TPAT_NEIGHBOR_LEFT;
                        else
                            iSubZoneOffset = GEX_TPAT_NEIGHBOR_LEFT_BOTTOM;
                    }
                    else if(iDieX > nCoordX)
                    {
                        // RIGHT sub-zones
                        if(iDieY < nCoordY)
                            iSubZoneOffset = GEX_TPAT_NEIGHBOR_RIGHT_TOP;
                        else if(iDieY == nCoordY)
                            iSubZoneOffset = GEX_TPAT_NEIGHBOR_RIGHT;
                        else
                            iSubZoneOffset = GEX_TPAT_NEIGHBOR_RIGHT_BOTTOM;
                    }
                    else
                    {
                        // CENTER sub-zones
                        if(iDieY < nCoordY)
                            iSubZoneOffset = GEX_TPAT_NEIGHBOR_CENTER_TOP;
                        else
                            iSubZoneOffset = GEX_TPAT_NEIGHBOR_CENTER_BOTTOM;
                    }

                    // Update fail count per sub-zone
                    icClusterSubZone[iSubZoneOffset]++;
                }
            }
        }


        // If not enough failures, return now
        if(iFailSqueezeCount >= m_nMinimumFailCount)
        {
            // We have enough failures...but check if they are located in such zones that the centered (good bin)
            // appears to be squeezed in!
            if(m_nMinimumFailCount == 1)
            {
                // Special situation: if asked to remove any bin touching a bad bin, then fail even if no sandwich situation!
                iBinSqueezed  = icClusterSubZone[GEX_TPAT_NEIGHBOR_LEFT] + icClusterSubZone[GEX_TPAT_NEIGHBOR_RIGHT];
                iBinSqueezed += icClusterSubZone[GEX_TPAT_NEIGHBOR_LEFT_TOP] + icClusterSubZone[GEX_TPAT_NEIGHBOR_RIGHT_BOTTOM];
                iBinSqueezed += icClusterSubZone[GEX_TPAT_NEIGHBOR_CENTER_TOP] + icClusterSubZone[GEX_TPAT_NEIGHBOR_CENTER_BOTTOM];
                iBinSqueezed += icClusterSubZone[GEX_TPAT_NEIGHBOR_LEFT_BOTTOM] + icClusterSubZone[GEX_TPAT_NEIGHBOR_RIGHT_TOP];
            }
            else
            {
                // Not stringent mode (requires real diagonal or sandwich situation)
                if(1)
                {
                    iBinSqueezed  = icClusterSubZone[GEX_TPAT_NEIGHBOR_LEFT]*icClusterSubZone[GEX_TPAT_NEIGHBOR_RIGHT];
                    iBinSqueezed += icClusterSubZone[GEX_TPAT_NEIGHBOR_LEFT_TOP]*icClusterSubZone[GEX_TPAT_NEIGHBOR_RIGHT_BOTTOM];
                    iBinSqueezed += icClusterSubZone[GEX_TPAT_NEIGHBOR_CENTER_TOP]*icClusterSubZone[GEX_TPAT_NEIGHBOR_CENTER_BOTTOM];
                    iBinSqueezed += icClusterSubZone[GEX_TPAT_NEIGHBOR_LEFT_BOTTOM]*icClusterSubZone[GEX_TPAT_NEIGHBOR_RIGHT_TOP];
                }
                else
                {
                    // Very stringent detection (diagonal, or close diagonal)
                    iBinSqueezed  = icClusterSubZone[GEX_TPAT_NEIGHBOR_LEFT]*icClusterSubZone[GEX_TPAT_NEIGHBOR_RIGHT];
                    iBinSqueezed += icClusterSubZone[GEX_TPAT_NEIGHBOR_LEFT]*icClusterSubZone[GEX_TPAT_NEIGHBOR_RIGHT_TOP];
                    iBinSqueezed += icClusterSubZone[GEX_TPAT_NEIGHBOR_LEFT]*icClusterSubZone[GEX_TPAT_NEIGHBOR_RIGHT_BOTTOM];
                    iBinSqueezed += icClusterSubZone[GEX_TPAT_NEIGHBOR_LEFT_TOP]*icClusterSubZone[GEX_TPAT_NEIGHBOR_RIGHT_BOTTOM];
                    iBinSqueezed += icClusterSubZone[GEX_TPAT_NEIGHBOR_LEFT_TOP]*icClusterSubZone[GEX_TPAT_NEIGHBOR_RIGHT];
                    iBinSqueezed += icClusterSubZone[GEX_TPAT_NEIGHBOR_LEFT_TOP]*icClusterSubZone[GEX_TPAT_NEIGHBOR_CENTER_BOTTOM];
                    iBinSqueezed += icClusterSubZone[GEX_TPAT_NEIGHBOR_CENTER_TOP]*icClusterSubZone[GEX_TPAT_NEIGHBOR_CENTER_BOTTOM];
                    iBinSqueezed += icClusterSubZone[GEX_TPAT_NEIGHBOR_CENTER_TOP]*icClusterSubZone[GEX_TPAT_NEIGHBOR_LEFT_BOTTOM];
                    iBinSqueezed += icClusterSubZone[GEX_TPAT_NEIGHBOR_CENTER_TOP]*icClusterSubZone[GEX_TPAT_NEIGHBOR_RIGHT_BOTTOM];
                    iBinSqueezed += icClusterSubZone[GEX_TPAT_NEIGHBOR_LEFT_BOTTOM]*icClusterSubZone[GEX_TPAT_NEIGHBOR_RIGHT_TOP];
                    iBinSqueezed += icClusterSubZone[GEX_TPAT_NEIGHBOR_LEFT_BOTTOM]*icClusterSubZone[GEX_TPAT_NEIGHBOR_CENTER_TOP];
                    iBinSqueezed += icClusterSubZone[GEX_TPAT_NEIGHBOR_LEFT_BOTTOM]*icClusterSubZone[GEX_TPAT_NEIGHBOR_RIGHT];
                }
            }

            // If result is 0, this means we never have two subzones appart from the good bin (in the center) that have failures
            if(iBinSqueezed > 0)
                bExcludeDie = true;
        }
    }

    return bExcludeDie;
}
