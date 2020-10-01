#include <new> // for std::bad_alloc
#include <stdio.h>
#include <stdlib.h>
#include <gqtl_log.h>
#include "histogramdata.h"
#include "stddef.h"

namespace GS
{
namespace Gex
{

unsigned HistogramData::sNumOfInstances=0;

HistogramData::HistogramData()
    : mTotalBars(0), mMinRange(0), mMaxRange(0), mHistoData(NULL)
{
    sNumOfInstances++;
}

HistogramData::HistogramData(int lTotalBars, double lMinRange, double lMaxRange)
    : mTotalBars(lTotalBars), mMinRange(lMinRange),
      mMaxRange(lMaxRange), mHistoData(NULL)
{
    sNumOfInstances++;

    if (lTotalBars > 0)
    {
        try
        {
            mHistoData = new int[mTotalBars];
        }
        catch(const std::bad_alloc& e)
        {
            printf("\nHistogramData::HistogramData: bad_alloc : requesting %d bars\n", mTotalBars);
            GSLOG(SYSLOG_SEV_ERROR, "Cannot create new HistogramData: bad_alloc");
            mHistoData=0;
            //exit(EXIT_FAILURE); // 7881 : seems to create "terminate called recursively"
        }
        catch(...)
        {
            mHistoData=0;
        }

        if (mHistoData)
            for (int i = 0; i<mTotalBars; ++i)
                mHistoData[i] = 0;
    }
}

HistogramData::~HistogramData()
{
    sNumOfInstances--;

    if (mHistoData != NULL)
        delete [] mHistoData;
    mHistoData = NULL;
}

HistogramData& HistogramData::operator=(const HistogramData& other)
{
    if (this != &other)
    {
        mTotalBars = other.mTotalBars;
        mMinRange = other.mMinRange;
        mMaxRange = other.mMaxRange;
        if (mHistoData)
            delete [] mHistoData;
        mHistoData=0;
        if (mTotalBars > 0)
        {
            mHistoData = new int[mTotalBars];
            for (int i = 0; i < mTotalBars; ++i)
                mHistoData[i] = other.mHistoData[i];
        }
    }
    return *this;
}

int HistogramData::GetTotalBars() const
{
    return mTotalBars;
}

double HistogramData::GetMinRange() const
{
    return mMinRange;
}

double HistogramData::GetMaxRange() const
{
    return mMaxRange;
}

bool HistogramData::AddValue(double lValue, bool lForce /*= false */)
{
    if ( (lForce || (lValue >= mMinRange && lValue <= mMaxRange))  &&
         mHistoData != NULL)
    {
        int iCell = (int)( ((double)mTotalBars * (lValue-mMinRange))/(mMaxRange-mMinRange));
        if(iCell >= mTotalBars)
            iCell = mTotalBars-1;	// Security to avoid overflowing array.
        if(iCell < 0)
            iCell = 0;
        mHistoData[iCell] += 1;
        return true;
    }

    return false;
}

int HistogramData::GetClassCount(int lCell) const
{
    if (mHistoData != NULL)
        return mHistoData[lCell];
    return (0);
}

double HistogramData::GetClassRange(int lCell) const
{
    double lClassRange;
    double lClassStartingRange;

    lClassRange = (mMaxRange - mMinRange) / mTotalBars;
    lClassStartingRange = mMinRange + (lClassRange * lCell);
    return lClassStartingRange;
}
}
}
