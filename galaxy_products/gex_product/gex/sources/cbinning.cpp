#include <gqtl_log.h>
#include "cbinning.h"

#ifdef unix
#include <stdio.h>
#endif

///////////////////////////////////////////////////////////
// For each binning (SBR) found, creates one of this structure.
///////////////////////////////////////////////////////////
CBinning::CBinning()
{
    iBinValue       = 0;		// Software Binning Value
    cPassFail       = ' ';		// Pass/Fail info. Default is unkown
    ldTotalCount    = 0;		// Total parts in this bin.
    ptNextBin       = NULL;		// Ensures pointer is initialized
    strBinName      = "";
}

void CBinning::Append(int lBin, BYTE lStatus, int lCount, const QString& lName)
{
    CBinning * pLast = this;

    // Find out the last binning object in the list
    while (pLast->ptNextBin)
        pLast = pLast->ptNextBin;

    // Create new binning object
    CBinning * pBinning =0;
    try
    {
        pBinning = new CBinning;
    }
    catch(const std::bad_alloc &e)
    {
        printf("\nCBinning::Append %s\n", e.what());
        GSLOG(SYSLOG_SEV_ERROR, e.what() );
        return; // todo : return a int, string, enum,... but return an error !
    }

    pBinning->iBinValue     = lBin;
    pBinning->cPassFail     = lStatus;
    pBinning->ldTotalCount  = lCount;
    pBinning->strBinName    = lName;

    pLast->ptNextBin = pBinning;
}

void CBinning::Insert(int lBin, BYTE lStatus, int lCount, const QString &lName)
{
    CBinning * pPrevious    = NULL;
    CBinning * pNext        = this;

    // Find out the last binning object in the list
    while (pNext && pNext->iBinValue < lBin)
    {
        pPrevious   = pNext;
        pNext       = pNext->ptNextBin;
    }

    // Create new binning object
    CBinning * pBinning = 0;
    try
    {
        pBinning = new CBinning;
    }
    catch(const std::bad_alloc &e)
    {
        //GSLOG(SYSLOG_SEV_ERROR, e.what() );
        printf("\nCBinning::Insert: %s\n", e.what());
        return; // todo : return a int, string, enum,... but return a status error with root cause.
    }

    pBinning->iBinValue     = lBin;
    pBinning->cPassFail     = lStatus;
    pBinning->ldTotalCount  = lCount;
    pBinning->strBinName    = lName;
    pBinning->ptNextBin     = pNext;

    if (pPrevious)
        pPrevious->ptNextBin    = pBinning;
}

CBinning *CBinning::Clone() const
{
    CBinning * pBinning = new CBinning;

    pBinning->iBinValue     = iBinValue;
    pBinning->cPassFail     = cPassFail;
    pBinning->ldTotalCount  = ldTotalCount;
    pBinning->strBinName    = strBinName;

    if (ptNextBin)
        pBinning->ptNextBin = ptNextBin->Clone();

    return pBinning;
}

CBinning *CBinning::Find(int binNumber)
{
    CBinning * pFind = this;

    while (pFind && binNumber != pFind->iBinValue)
        pFind = pFind->ptNextBin;

    return pFind;
}

unsigned CBinning::CountBins()
{
    CBinning* lCurBin=this;
    unsigned lCount=1;
    while (lCurBin->ptNextBin)
    {
        lCount++;
        lCurBin=lCurBin->ptNextBin;
    }
    return lCount;
}
