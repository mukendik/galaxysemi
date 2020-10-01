///////////////////////////////////////////////////////////
// GS Includes
///////////////////////////////////////////////////////////
#include "wafermap_parametric.h"
#include "test_result_item.h"
#include "report_options.h"
#include "ctest.h"
#include "classes.h"
#include "gex_report.h"
#include "gqtl_log.h"
#include "pat_part_filter.h"
#include <gqtl_global.h>
#include "gex_algorithms.h"
#include "pat_defines.h"
//#include "report_build.h"

///////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////
#include <QStringList>
#include <QFile>

///////////////////////////////////////////////////////////////////////////////////
// External object
///////////////////////////////////////////////////////////////////////////////////
extern CGexReport *		gexReport;
extern CReportOptions	ReportOptions;

extern double	ScalingPower(int iPower);

#define WAFERMAP_NNR_MAX_RADIUS         7
#define WAFERMAP_NNR_MATRIX_DEFAULT     5

///////////////////////////////////////////////////////////
// This class is used to manipula parametric wafermap
///////////////////////////////////////////////////////////
CParametricWaferMap::CParametricWaferMap()
{
    cParametricWafMap = NULL;
    mLowDieY    = mLowDieX = 65535;
    mHighDieY   = mHighDieX = -65535;
    mSizeX      = 0;
    mSizeY      = 0;
    mLowLimit   = -C_INFINITE;
    mHighLimit  = C_INFINITE;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CParametricWaferMap::~CParametricWaferMap()
{
    if(cParametricWafMap)
        delete [] cParametricWafMap;
    cParametricWafMap = NULL;
}

bool CParametricWaferMap::Init(int lLowDieX, int lLowDieY, int lHighDieX, int lHighDieY)
{
    mLowDieX    = lLowDieX;
    mLowDieY    = lLowDieY;
    mHighDieX   = lHighDieX;
    mHighDieY   = lHighDieY;
    mSizeX      = lHighDieX - lLowDieX + 1;
    mSizeY      = lHighDieY - lLowDieY + 1;

    if (cParametricWafMap)
    {
        delete [] cParametricWafMap;
    }

    int lSize = mSizeX * mSizeY;

    cParametricWafMap = new CParametricWafMapArray[lSize];

    for (int lIdx = 0; lIdx < lSize; ++lIdx)
        cParametricWafMap[lIdx].lfValue = GEX_C_DOUBLE_NAN;

    return true;
}

CParametricWaferMap &CParametricWaferMap::operator=(const CParametricWaferMap &lOther)
{
    if (this != &lOther)
    {
        mLowDieX    = lOther.mLowDieX;
        mLowDieY    = lOther.mLowDieY;
        mHighDieX   = lOther.mHighDieX;
        mHighDieY   = lOther.mHighDieY;
        mSizeX      = mHighDieX - mLowDieX + 1;
        mSizeY      = mHighDieY - mLowDieY + 1;
        mLowLimit   = lOther.mLowLimit;
        mHighLimit  = lOther.mHighLimit;

        if (cParametricWafMap)
        {
            delete [] cParametricWafMap;
        }

        int lSize = mSizeX * mSizeY;

        cParametricWafMap = new CParametricWafMapArray[lSize];

        if (lOther.cParametricWafMap)
            memcpy(cParametricWafMap, lOther.cParametricWafMap, lSize * sizeof(CParametricWafMapArray));
        else
        {
            for (int lIdx = 0; lIdx < lSize; ++lIdx)
                cParametricWafMap[lIdx].lfValue = GEX_C_DOUBLE_NAN;
        }
    }

    return *this;
}

int CParametricWaferMap::GetLowDieX() const
{
    return mLowDieX;
}

int CParametricWaferMap::GetLowDieY() const
{
    return mLowDieY;
}

int CParametricWaferMap::GetHighDieX() const
{
    return mHighDieX;
}

int CParametricWaferMap::GetHighDieY() const
{
    return mHighDieY;
}

int CParametricWaferMap::GetSizeX() const
{
    return mSizeX;
}

int CParametricWaferMap::GetSizeY() const
{
    return mSizeY;
}

///////////////////////////////////////////////////////////
// Fill parametric wafermap (merge all sites)
///////////////////////////////////////////////////////////
bool CParametricWaferMap::FillWaferMapFromMultipleDatasets(CTest *ptTestCell, GS::QtLib::Range *pGoodBinsList/*=NULL*/)
{
    // Check for valid pointers.
    if(gexReport == NULL || ptTestCell == NULL)
        return false;

    CGexGroupOfFiles *	pGroup	= NULL;
    CGexFileInGroup *	pFile	= NULL;
    int                 uGroupID;

    // Compute stacked-paramteric wafersize, and also mean of sites medians.
    CTest *	pTestCellInSite		= NULL;
    double	lfMedianMean		= 0;
    int     lLowDieX            = 65535;
    int     lLowDieY            = 65535;
    int     lHighDieX           = -65535;
    int     lHighDieY           = -65535;

    for(uGroupID=0;uGroupID < gexReport->getGroupsList().count();uGroupID++)
    {
        // Get group & file handle
        pGroup = gexReport->getGroupsList().at(uGroupID);
        if (!pGroup->pFilesList.isEmpty())
        {
            pFile = pGroup->pFilesList.at(0);

            if(pFile->getWaferMapData().getWafMap() != NULL)
            {
                // This file has a valid wafermap...check how it fits over the other wafers.
                lLowDieX    = gex_min(pFile->getWaferMapData().iLowDieX, lLowDieX);
                lHighDieX   = gex_max(pFile->getWaferMapData().iHighDieX, lHighDieX);
                lLowDieY    = gex_min(pFile->getWaferMapData().iLowDieY, lLowDieY);
                lHighDieY   = gex_max(pFile->getWaferMapData().iHighDieY, lHighDieY);
            }

            // Get handle to test cell (so to get its median value)
            if(pFile->FindTestCell(ptTestCell->lTestNumber,ptTestCell->lPinmapIndex,&pTestCellInSite) != 1)
                return false;

            // Add all medians...
            lfMedianMean += pTestCellInSite->lfSamplesQuartile2;
        }
    }

    // Compute Mean of sites median.
    lfMedianMean /= gexReport->getGroupsList().count();

    // This file has a valid wafermap...check how it fits over the other wafers.
    if (Init(lLowDieX, lLowDieY, lHighDieX, lHighDieY) == false)
        return false;

    // Loop over each group to merge parametric maps
    double	lfSiteOffset;
    CTest	*ptCellDieX;
    CTest	*ptCellDieY;
    CTest	*ptSoftBin;
    int		iSampleOffset,iDieX,iDieY,iSoftBin;
    double	lfValue;
    int		iStartOffset	= 0;	// Get 1st run# offset in results array (in case multiple files merged)
    int		iEndOffset		= 0;
    int     lIndex          = -1;
    for(uGroupID=0;uGroupID < gexReport->getGroupsList().count();uGroupID++)
    {
        // Get handle to testing site
        pGroup = gexReport->getGroupsList().at(uGroupID);
        if (!pGroup->pFilesList.isEmpty())
        {
            pFile = pGroup->pFilesList.at(0);

            // Get handle to test cell in site
            if(pFile->FindTestCell(ptTestCell->lTestNumber,ptTestCell->lPinmapIndex,&pTestCellInSite) != 1)
                return false;

            // Get handle to DieX and DieY parameters
            if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEX,GEX_PTEST,&ptCellDieX,true,false) != 1)
                return false;
            if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEY,GEX_PTEST,&ptCellDieY,true,false) != 1)
                return false;
            if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_SBIN,GEX_PTEST,&ptSoftBin,true,false) != 1)
                return false;

            // Get samples start-end offsets
            pTestCellInSite->findSublotOffset(iStartOffset, iEndOffset, pFile->lFileID);
            if( (pTestCellInSite->m_testResult.count() < iEndOffset)
                    || (ptCellDieX->m_testResult.count() < iEndOffset)
                    || (ptCellDieY->m_testResult.count() < iEndOffset))
            {
                GEX_ASSERT(false);
                iEndOffset = iStartOffset;  // has lists are not synchronized, be sure that we don't try to do something.
            }

            // Compute site offset: Median-Mean of all sites median
            lfSiteOffset = pTestCellInSite->lfSamplesQuartile2-lfMedianMean;

            // Fill tested dies with tested dies for the given parameter
            for(iSampleOffset = iStartOffset; iSampleOffset < iEndOffset; iSampleOffset++)
            {
                if(pTestCellInSite->m_testResult.isValidResultAt(iSampleOffset))
                {
                    lfValue = pTestCellInSite->m_testResult.resultAt(iSampleOffset);

                    // Compute associated DieX,Y coordinates
                    iDieX = (int) ptCellDieX->m_testResult.resultAt(iSampleOffset);
                    iDieY = (int) ptCellDieY->m_testResult.resultAt(iSampleOffset);

                    // Get SoftBin for this die location
                    iSoftBin = (int) ptSoftBin->m_testResult.resultAt(iSampleOffset);

                    // Check if we must only load parametric values for dies that are tested GOOD (Good Bin Softbin), or load all values
                    if((pGoodBinsList == NULL) ||
                       (pGoodBinsList && pGoodBinsList->Contains(iSoftBin)))
                    {
                        // Normalize value then Offset it by site offset
                        lfValue = (lfValue*ScalingPower(pTestCellInSite->res_scal)) - lfSiteOffset;

                        // This die binning matches our criteria
                        // Check if valid Die coordinates
                        lIndex = (iDieX-mLowDieX) + (iDieY-mLowDieY)*mSizeX;
                        if(iDieX != GEX_WAFMAP_INVALID_COORD && iDieY != GEX_WAFMAP_INVALID_COORD && lIndex >= 0 && lIndex < mSizeX*mSizeY)
                            cParametricWafMap[lIndex].lfValue = lfValue;	// Valid value
                    }
                }
            }
        }
    }

    // Parametric wafermap structure sucessfuly filled
    return true;
}

bool CParametricWaferMap::FillWaferMapFromSingleDataset(CTest *pTestCell, QList<int> lSiteList,
                                                        GS::QtLib::Range * pGoodBinsList)
{
    // Check for valid pointers.
    if(gexReport == NULL || pTestCell == NULL)
        return false;

    // Get group & file handle
    CGexGroupOfFiles *	lGroup	= (gexReport->getGroupsList().isEmpty()) ? NULL : gexReport->getGroupsList().first();
    CGexFileInGroup *	lFile	= (lGroup == NULL || lGroup->pFilesList.isEmpty()) ? NULL : lGroup->pFilesList.first();

    if (lFile == NULL || lFile->getWaferMapData().getWafMap() == NULL)
        return false;

    // This file has a valid wafermap...check how it fits over the other wafers.
    if (Init(lFile->getWaferMapData().iLowDieX, lFile->getWaferMapData().iLowDieY,
             lFile->getWaferMapData().iHighDieX,lFile->getWaferMapData().iHighDieY) == false)
        return false;

    // Compute mean of sites medians.
    CTest *	pTestSite		= NULL;
    CTest * pTestDieX       = NULL;
    CTest * pTestDieY       = NULL;
    CTest * pTestSoftBin    = NULL;
    double	lMedianMean     = 0;

    // Get handle to DieX and DieY parameters
    if(lFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEX, GEX_PTEST, &pTestDieX, true, false) != 1)
        return false;
    if(lFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEY, GEX_PTEST, &pTestDieY, true, false) != 1)
        return false;
    if(lFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_SBIN, GEX_PTEST, &pTestSoftBin, true, false) != 1)
        return false;
    if(lFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_TESTING_SITE, GEX_PTEST, &pTestSite, true, false) != 1)
        return false;

    QHash<int, double>  lSitesMedian;
    CGexStats           lStatsEngine;
    QString             lCpkOption          = ReportOptions.GetOption("statistics", "cp_cpk_computation").toString();
    bool                lUsePercentileCPK   = false;

    if (lCpkOption == "percentile")
        lUsePercentileCPK = true;

    GS::Gex::PATPartFilter lPartFilter;
    for(int lIdx = 0; lIdx < lSiteList.count(); ++lIdx)
    {
        lPartFilter.RemoveAllFilters();
        lPartFilter.AddFilterElement(GS::Gex::PATPartFilterElement(pTestSoftBin,
                                     GS::Gex::PATPartFilterElement::InFilter,
                                     GS::QtLib::Range(*pGoodBinsList)));
        if (lSiteList.at(lIdx) >= 0)
        {
            lPartFilter.AddFilterElement(GS::Gex::PATPartFilterElement(pTestSite,
                                         GS::Gex::PATPartFilterElement::InFilter,
                                         GS::QtLib::Range(QString::number(lSiteList.at(lIdx)))));
        }

        lStatsEngine.ComputeLowLevelTestStatistics(pTestCell, ScalingPower(pTestCell->res_scal), &lPartFilter);
        lStatsEngine.ComputeAdvancedDataStatistics_Quartiles(pTestCell, true, lUsePercentileCPK, false, &lPartFilter);

        // Add all medians...
        lMedianMean += pTestCell->lfSamplesQuartile2;
        GSLOG(SYSLOG_SEV_DEBUG, QString("Site %1 : median = %2 for %3 valid execs")
              .arg(lSiteList.at(lIdx)).arg(pTestCell->lfSamplesQuartile2).arg(pTestCell->ldSamplesValidExecs).toLatin1().constData());

        lSitesMedian.insert(lSiteList.at(lIdx), pTestCell->lfSamplesQuartile2);
    }

    // Compute Mean of sites median.
    lMedianMean /= lSiteList.count();

    // Loop over each group to merge parametric maps
    double	lfSiteOffset;
    int     iDieX;
    int     iDieY;
    int     iSite;
    int     iSoftBin;
    double	lfValue;
    int     iIndex;

    for(int lIdx = 0; lIdx < pTestCell->m_testResult.count(); ++lIdx)
    {
        iSite       = (int) pTestSite->m_testResult.resultAt(lIdx);
        iSoftBin    = (int) pTestSoftBin->m_testResult.resultAt(lIdx);

        if(lSitesMedian.contains(iSite) &&
           (pGoodBinsList == NULL || pGoodBinsList->Contains(iSoftBin)) &&
           pTestCell->m_testResult.isValidResultAt(lIdx))
        {
            lfValue = pTestCell->m_testResult.resultAt(lIdx);

            // Compute associated DieX,Y coordinates
            iDieX = (int) pTestDieX->m_testResult.resultAt(lIdx);
            iDieY = (int) pTestDieY->m_testResult.resultAt(lIdx);

            // Compute site offset: Median-Mean of all sites median
            lfSiteOffset = lSitesMedian.value(iSite) - lMedianMean;

            // Check if we must only load parametric values for dies that are tested GOOD (Good Bin Softbin), or load all values
            // Normalize value then Offset it by site offset
            lfValue = (lfValue * ScalingPower(pTestCell->res_scal)) - lfSiteOffset;

            // This die binning matches our criteria
            // Check if valid Die coordinates
            iIndex = (iDieX-mLowDieX) + (iDieY-mLowDieY)*mSizeX;
            if(iDieX != GEX_WAFMAP_INVALID_COORD && iDieY != GEX_WAFMAP_INVALID_COORD && iIndex >= 0 && iIndex < mSizeX*mSizeY)
                cParametricWafMap[iIndex].lfValue = lfValue;	// Valid value

        }
    }

    // Parametric wafermap structure sucessfuly filled
    return true;
}

/*
bool CParametricWaferMap::FillIDDQWaferMap(CTest *testCellPre, CTest *testCellPost, GS::QtLib::Range *pGoodBinsList)
{
    // Check for valid pointers.
    if(gexReport == NULL || testCellPre == NULL || testCellPost == NULL)
        return false;

    // Get group & file handle
    CGexGroupOfFiles *	lGroup	= (gexReport->getGroupsList().isEmpty()) ? NULL : gexReport->getGroupsList().first();
    CGexFileInGroup *	lFile	= (lGroup == NULL || lGroup->pFilesList.isEmpty()) ? NULL : lGroup->pFilesList.first();

    if (lFile == NULL || lFile->getWaferMapData().getWafMap() == NULL)
        return false;

    // This file has a valid wafermap...check how it fits over the other wafers.
    if (Init(lFile->getWaferMapData().iLowDieX, lFile->getWaferMapData().iLowDieY,
             lFile->getWaferMapData().iHighDieX,lFile->getWaferMapData().iHighDieY) == false)
        return false;

    // Compute mean of sites medians.
    CTest * lTestDieX       = NULL;
    CTest * lTestDieY       = NULL;
    CTest * lTestSoftBin    = NULL;

    // Get handle to DieX and DieY parameters
    if(lFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEX, GEX_PTEST, &lTestDieX, true, false, true) != 1)
        return false;
    if(lFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEY, GEX_PTEST, &lTestDieY, true, false, true) != 1)
        return false;
    if(lFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_SBIN, GEX_PTEST, &lTestSoftBin, true, false, true) != 1)
        return false;

    // Merge parametric maps
    int     lDieX;
    int     lDieY;
    int     lSoftBin;
    double	lPreValue;
    double	lPostValue;
    int     lIndex;

    for(int lIdx = 0; lIdx < testCellPre->m_testResult.count(); ++lIdx)
    {
        lSoftBin    = (int) lTestSoftBin->m_testResult.resultAt(lIdx);

        if((pGoodBinsList == NULL || pGoodBinsList->Contains(lSoftBin)) &&
           testCellPre->m_testResult.isValidResultAt(lIdx) &&
           testCellPost->m_testResult.isValidResultAt(lIdx))
        {
            // Check if we must only load parametric values for dies that are tested GOOD (Good Bin Softbin), or load all values
            // Normalize value
            lPreValue   = testCellPre->m_testResult.scaledResultAt(lIdx, testCellPre->res_scal);
            lPostValue  = testCellPost->m_testResult.scaledResultAt(lIdx, testCellPost->res_scal);

            // Compute associated DieX,Y coordinates
            lDieX = (int) lTestDieX->m_testResult.resultAt(lIdx);
            lDieY = (int) lTestDieY->m_testResult.resultAt(lIdx);

            // This die binning matches our criteria
            // Check if valid Die coordinates
            lIndex = (lDieX-mLowDieX) + (lDieY-mLowDieY)*mSizeX;
            if(lDieX != GEX_WAFMAP_INVALID_COORD && lDieY != GEX_WAFMAP_INVALID_COORD && lIndex >= 0 && lIndex < mSizeX*mSizeY)
                cParametricWafMap[lIndex].lfValue = lPreValue - lPostValue;	// Valid value
        }
    }

    // Parametric wafermap structure sucessfuly filled
    return true;
}
*/

///////////////////////////////////////////////////////////
// NNRBuild "Location Averaging" for a given die position
// Value = Median (Top Y closest values of value(x,y) in cluster)
///////////////////////////////////////////////////////////
bool CParametricWaferMap::NNR_GetLA(CParametricWaferMap& cParametricMap,CParametricWaferMapNNR &cNNR, double OriginalValue)
{
    // Local variables
    long                totalValidValues    = 0;
    long                totalNeededValues   = 0;
    QVector<double>     validDataPoints;

    // Get data points for the distribution to use
#ifdef BOSH_NNR
    if (cNNR.iMatrixSize == -1)
        getNNRWholeWaferDistribution(validDataPoints, totalValidValues,
                                     totalNeededValues, cParametricMap, cNNR);
    else
        getNNRClusterDistribution(validDataPoints, totalValidValues,
                                  totalNeededValues, cParametricMap, cNNR);
#else
    getNNRClusterDistribution(validDataPoints, totalValidValues,
                              totalNeededValues, cParametricMap, cNNR);
#endif

    if (totalValidValues == 0)
    {
        GSLOG(SYSLOG_SEV_NOTICE,
              QString("No valid value found for Location Averaging at position (%1,%2).")
              .arg(cNNR.iDieX).arg(cNNR.iDieY).toLatin1().constData());
        return true;
    }

    if (totalNeededValues == 0)
    {
        GSLOG(SYSLOG_SEV_NOTICE,
              QString("No value needed for Location Averaging at position (%1,%2).")
              .arg(cNNR.iDieX).arg(cNNR.iDieY).toLatin1().constData());
        return true;
    }


    long firstIndex = 0;
    long lastIndex  = validDataPoints.count() - 1;

    while (totalNeededValues < totalValidValues)
    {
        if (fabs(OriginalValue - validDataPoints[firstIndex]) > fabs(OriginalValue - validDataPoints[lastIndex]))
            ++firstIndex;
        else
            --lastIndex;

        --totalValidValues;
    }

    // Return median of delta values computed
    double  lMedian = algorithms::gexMedianValue(validDataPoints, firstIndex, lastIndex);

    if (lMedian != GEX_C_DOUBLE_NAN)
        cNNR.lfValue = OriginalValue - lMedian;
    else
    {
        GSLOG(SYSLOG_SEV_NOTICE,
              QString("No median value found for Location Average algorithm at position (%1,%2).")
              .arg(cNNR.iDieX).arg(cNNR.iDieY).toLatin1().constData());
    }

    return true;
}

///////////////////////////////////////////////////////////
// NNR: Build NNR map
// - For each die location (x,y), compute:
// - New_Value at (x,y) = Value_at(x,y) - Median (Top Y closest values of value(x,y) in cluster)
///////////////////////////////////////////////////////////
bool CParametricWaferMap::NNR_BuildMap(CParametricWaferMap& cParametricMap,CParametricWaferMapNNR &cNNR)
{
    int	iIndex;
    double	lfValue;

    // Check if parametric wafermap structure filled
    if(cParametricMap.cParametricWafMap == NULL)
        return false;

    if (Init(cParametricMap.GetLowDieX(), cParametricMap.GetLowDieY(), cParametricMap.GetHighDieX(),
             cParametricMap.GetHighDieY()) == false)
        return false;

    // Fill NNR wafermap
    QVector<double> vectDataPoints(mSizeX*mSizeY);
    int iVectorID=0;

#ifdef BOSCH
    mNNRWholeWafer.clear();
#endif

    //
#ifdef NNR_DUMP
    QFile dumpFile(QString("Test-NNR-dump.csv"));

    dumpFile.open(QIODevice::WriteOnly);
    QTextStream txtStreamNNR(&dumpFile);

    txtStreamNNR << "NNR Dump for test " << endl;
    if (cNNR.iMatrixSize==-1)
        txtStreamNNR << "Matrix size;Whole wafer" << endl;
    else
        txtStreamNNR << "Matrix size;" << cNNR.iMatrixSize <<  endl;
    txtStreamNNR << "Location Averaging;" << cNNR.lf_LA << endl;
    txtStreamNNR << endl;

#endif

    for(iIndex=0;iIndex < mSizeX*mSizeY;iIndex++)
    {
        // Clear previous NNR value
        cNNR.lfValue = GEX_C_DOUBLE_NAN;

        // Compute die location
        cNNR.iDieX = (iIndex % mSizeX) + mLowDieX;
        cNNR.iDieY = (iIndex / mSizeX) + mLowDieY;

        // If valid value, compute DeltaValue for this die location
        if(cParametricMap.cParametricWafMap[iIndex].lfValue != GEX_C_DOUBLE_NAN)
        {
            // "L.A." algorithm: "Location Averaging"
            // Delta value = original value - median (top X closest values in cluster)
            if(NNR_GetLA(cParametricMap, cNNR, cParametricMap.cParametricWafMap[iIndex].lfValue) == false)
                return false; // error, not enough data to compute NNR!

#ifdef NNR_DUMP
            txtStreamNNR << cNNR.iDieX << ";" << cNNR.iDieY << ";";
            txtStreamNNR << cParametricMap.cParametricWafMap[iIndex].lfValue << ";";

            if (cNNR.lfValue == GEX_C_DOUBLE_NAN)
                txtStreamNNR << "n/a" << endl;
            else
                txtStreamNNR << cNNR.lfValue << endl;

#endif
        }


        // Save Delta (NNR) value
        cParametricWafMap[iIndex].lfValue = cNNR.lfValue;

        // Fill array to later sort Delta values.
        if(cNNR.lfValue != GEX_C_DOUBLE_NAN)
        {
            vectDataPoints.insert(iVectorID, cNNR.lfValue);
            iVectorID++;
        }
    }

    ////////////////////////////////////////////////
    // Compute NNR limits.
    ////////////////////////////////////////////////

    // If too few samples, quietly return, no NRR check.
    if(iVectorID < 5)
        return false;

    // Remove extra memory space
    vectDataPoints.resize(iVectorID);

    // Sort list
    qSort(vectDataPoints);

    // Compute stats.
    double lfSumValues=0.0,lfSumSquaredValues=0.0;
    for(iIndex = 0; iIndex < iVectorID; iIndex++)
    {
        // Compute sum of valid results (with Min & Max excluded)
        lfValue = vectDataPoints[iIndex];
        lfSumValues += lfValue;

        // Compute sum of squared values (so to compute sigma)
        lfSumSquaredValues += lfValue*lfValue;
    }

    // Compute mean of NRR values
    double	lfMean = lfSumValues/(iVectorID);

    // Compute sigma of NRR values
    double	lfSigma = sqrt(fabs((((double)iVectorID*lfSumSquaredValues) - GS_POW(lfSumValues,2.0))/
                        ((double)iVectorID*((double)iVectorID-1))));

    // Compute Median and IQR
    double	lfMedian = vectDataPoints[iVectorID/2];
    double lfQ1 = vectDataPoints[iVectorID * 25 / 100];
    double lfQ3 = vectDataPoints[iVectorID * 75 / 100];
    double	lfIQR = lfQ3-lfQ1;

    // Compute NNR limits...
    mLowLimit = -C_INFINITE;
    mHighLimit = C_INFINITE;
    switch(cNNR.iAlgorithm)
    {
        case GEX_TPAT_NNR_ALGO_LOCAL_SIGMA:
            // Mean +/-N*Cluster_Sigma
            if(lfSigma == 0)
                return false;	// All values identical, so no NNR!
            mLowLimit = lfMean - (cNNR.lfN_Factor*lfSigma);
            mHighLimit = lfMean + (cNNR.lfN_Factor*lfSigma);
            break;

        case GEX_TPAT_NNR_ALGO_LOCAL_MEDIAN:
            // Median +/-N*RobustSigma
            if(lfIQR == 0)
                return false;	// All values identical, so no NNR!
            mLowLimit = lfMedian - (cNNR.lfN_Factor*lfIQR/1.35);
            mHighLimit = lfMedian + (cNNR.lfN_Factor*lfIQR/1.35);
            break;

        case GEX_TPAT_NNR_ALGO_LOCAL_Q1Q3IQR:
            // Q1-N*IQR, Q3+N*IQR
            if(lfIQR == 0)
                return false;	// All values identical, so no NNR!
            mLowLimit = lfQ1 - (cNNR.lfN_Factor*lfIQR);
            mHighLimit = lfQ3 + (cNNR.lfN_Factor*lfIQR);
            break;
    }

#ifdef NNR_DUMP
        txtStreamNNR << "Computed mean;" << lfMean << endl;
        txtStreamNNR << "Computed StdDev;" << lfSigma << endl;
        txtStreamNNR << "Computed Low Limit;" << mLowLimit << endl;
        txtStreamNNR << "Computed High Limit;" << mHighLimit << endl;
#endif

    // Return status
    return true;
}

///////////////////////////////////////////////////////////
// Parametric wafer map: checks if given die location
// Is a NNR (value is outlier compared to neighbhoors: 5*5 matrix)
///////////////////////////////////////////////////////////
bool CParametricWaferMap::isNNR(CParametricWaferMapNNR &cNNR)
{
    // Check if given die hold valid Delta value, and if failing NNR limits
    if((cNNR.lfValue != GEX_C_DOUBLE_NAN) && (cNNR.lfValue < mLowLimit || cNNR.lfValue > mHighLimit))
        return true;	// NNR outlier
    else
        return false;	// Not a NNR outlier
}

void CParametricWaferMap::getNNRClusterDistribution(
        QVector<double> &validDataPoints, long &totalValidValues,
        long &totalNeededValues, const CParametricWaferMap &cParametric,
        const CParametricWaferMapNNR &cNNR)
{
    // Initializing
    totalValidValues    = 0;
    totalNeededValues   = 0;
    validDataPoints.clear();

    // Find closest N values in cluster.
    int	iMatrixRadius;
    switch(cNNR.iMatrixSize)
    {
        default:
            iMatrixRadius = WAFERMAP_NNR_MATRIX_DEFAULT;	// Fall into case 5.
        case 3:	// 3*3
        case 5:	// 5*5
        case 7:	// 7*7
        case 9:	// 9x9
        case 11:// 11x11
        case 13:// 13x13
        case 15:// 15*15
            iMatrixRadius = ((cNNR.iMatrixSize-1)/2);	// Matrix radius (maximum distance analyzed around center die)
            break;

        case -1: // Whole wafer
            return;
    }

    // "LA": Top X best dies to identify (closest values to reference value)
    totalNeededValues = static_cast<long>(
                (cNNR.iMatrixSize * cNNR.iMatrixSize * cNNR.lf_LA) / 100.0);

    // Buffer to hold all values of the N*N parametric wafermap centered on die to analyze (N= 13 maximum)
    double          lfTestValue;
    int             iX,iY,iOffset;
    int             iMatrixIndex;

#ifdef NNR_DUMP_DETAILS
    QFile dumpFile(QString("NNR-LA-dump-Test-Coord-%2-%3.csv").arg(cNNR.iDieX).arg(cNNR.iDieY));

    dumpFile.open(QIODevice::WriteOnly);
    QTextStream txtStreamNNR(&dumpFile);

    txtStreamNNR << "NNR LA Dump for test at Coord (" << cNNR.iDieX << "," << cNNR.iDieY << ")" << endl;
    txtStreamNNR << endl;
#endif

    // If not enough good dies found, enlarge radius...but do NOT exceed maximum limit allowed!
    while(iMatrixRadius <= WAFERMAP_NNR_MAX_RADIUS)
    {
        // Reset variables
        iMatrixIndex        = 0;
        totalValidValues    = 0;
        validDataPoints.clear();

        // reserve space to store delta (better performance)
        validDataPoints.reserve(static_cast<int>(GS_POW(iMatrixRadius * 2, 2.0)));

#ifdef NNR_DUMP_DETAILS
        txtStreamNNR << "Matrix of " << iMatrixRadius*2 << endl;
#endif
        // Scan NxN square area
        for(iX = -iMatrixRadius; iX <= iMatrixRadius; iX++)
        {
            for(iY = -iMatrixRadius; iY <= iMatrixRadius; iY++)
            {
                // Offset to point to each die of the cluster to analyze (center die is checked for NNR)
                iOffset = ((cNNR.iDieX+iX)-mLowDieX) + ((cNNR.iDieY+iY)-mLowDieY)*mSizeX;

                // Check if valid offset in stacked wafermap and not reading outside of it!
                if((iX == 0 && iY == 0))
                {
                    // Pointing on center die: ignore it as it is neighbhoors we want to check!
                }
                else
                if((iOffset >= 0) && (iOffset < mSizeX*mSizeY) && ((cNNR.iDieX+iX)-mLowDieX < mSizeX) && ((cNNR.iDieY+iY)-mLowDieY < mSizeY))
                {
                    // Valid die coordinate, retrieve parametric value
                    lfTestValue = cParametric.cParametricWafMap[iOffset].lfValue;
                    if(lfTestValue != GEX_C_DOUBLE_NAN)
                    {
#ifdef NNR_DUMP_DETAILS
                        txtStreamNNR << cNNR.iDieX + iX << ";" << cNNR.iDieY + iY << ";" << lfTestValue << endl;
#endif
                        // Save delta : reference die value - test result
                        validDataPoints.append(lfTestValue);

                        // Keep track of total valid tests stored in matrix
                        totalValidValues++;
                    }
                }

                // Move to next matrix cell to fill
                iMatrixIndex++;
            }
        }
        // If not enough valid result in the matrix; enlarge matrix
        if(totalValidValues < totalNeededValues)
        {
            iMatrixRadius++;
#ifdef NNR_DUMP_DETAILS
            txtStreamNNR << "Not enough die (" << totalValidValues << "/" << totalNeededValues << "): increasing matrix" << endl;
#endif
        }
        else
            break;	// Enough valid samples found, no need to enlarge working cluster
    }

    // Sort NNR list
    qSort(validDataPoints);
}

#ifdef BOSCH_NNR
void CParametricWaferMap::getNNRWholeWaferDistribution(
        QVector<double> &validDataPoints, long &totalValidValues,
        long &totalNeededValues, const CParametricWaferMap &cParametric,
        const CParametricWaferMapNNR &cNNR)
{
    //
    double testValue = GEX_C_DOUBLE_NAN;

    // Initializing
    totalValidValues    = 0;
    totalNeededValues   = 0;
    validDataPoints.clear();

    if (cNNR.iMatrixSize != -1)
        return;

    // Fill all wafer data points
    if (mNNRWholeWafer.isEmpty())
    {
        mNNRWholeWafer.reserve(SizeX * SizeY);

        for(int index = 0; index < SizeX*SizeY; ++index)
        {
            // Valid die coordinate, retrieve parametric value
            testValue = cParametric.cParametricWafMap[index].lfValue;
            if(testValue != GEX_C_DOUBLE_NAN)
                mNNRWholeWafer.append(testValue);
        }

        qSort(mNNRWholeWafer);
    }

    validDataPoints << mNNRWholeWafer;

    totalValidValues    = validDataPoints.count();
    totalNeededValues   = totalValidValues * cNNR.lf_LA / 100;
}
#endif
