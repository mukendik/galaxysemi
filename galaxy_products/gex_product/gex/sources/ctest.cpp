#include "ctest.h"
#include "gex_constants.h"
#include "gex_xbar_r_data.h"
#include <gqtl_global.h>

#include <QVariant>
#if defined __APPLE__&__MACH__
    #include <malloc/malloc.h>
#else
    #include <malloc.h>
#endif
#include <qmath.h>
#include <QString>
#include "pat_defines.h"

#if !defined(GEX_CTEST_CPP__INCLUDED_)
#define GEX_CTEST_CPP__INCLUDED_

CGageR_R::CGageR_R()
{
    lfEV=-1;	// Equipment Variation (Negative value means not computed)
    lfAV=-1;	// Appraiser Variation (Negative value means not computed)
    lfRR=-1;	// Gage R&R (Negative value means not computed)
    lfRR_percent = 0;
    lfGB=-1;	// GuardBanding: %RNR*Tolerance
    lfPV=-1;	// Part Variation (Negative value means not computed)
    lfTV=-1;	// Total Variation (Negative value means not computed)
    lfP_T=-1;	// Ratio P/T
}

CFunctionalTest::CFunctionalTest()
{
    strVectorName = '-';
    lFails=0;			// Total failures in vector
    lExecs=0;			// Total execution of this vector
}

int CTest::s_nInstances=0;
// HTH: This doesnt look useful anymore since GCORE-10297
// GS::Core::MultiLimitItem* CTest::mInvalidLimitItem = 0;

CTest::CTest() :
    QObject(),
    m_testResult(this),
    mUserTestName(""),
    mUserTestNumber(""),
    mUserPinName("")
{
    //GSLOG(SYSLOG_SEV_EMERGENCY, QString("sizeof(CTest)=%1").arg(sizeof(CTest)));

    m_bUseCustomBarNumber = false;
    s_nInstances++;

// HTH: This doesnt look useful anymore since GCORE-10297
//    if(mInvalidLimitItem == 0)
//    {
//        mInvalidLimitItem = new GS::Core::MultiLimitItem();
//        mInvalidLimitItem->Clear();
//        mInvalidLimitItem->SetUnValidLimit();
//    }

    setObjectName(QString("Test%1").arg(s_nInstances));

    lTestFlowID = 0;			// Testing order in data flow (1,2,...)
    lTestNumber=0;				// TestNumber
    lPinmapIndex = -1;
    ldExecs=0;					// Number of executions
    lfTotal=-C_INFINITE;		// Sum of X
    lfTotalSquare=-C_INFINITE;	// Sum of X*X
    lfSamplesTotal=0;			// Sum of X (sampled data)
    lfSamplesTotalSquare=0;		// Sum of X*X (sampled data)
    ldSamplesExecs=0;			// Number of Test results sampled (including NaN values)
    ldSamplesValidExecs=0;		// Total valid samples to consider in result array buffer
    ldCurrentSample=0;			// In pass2, keeps track of sample# (first sample= 0,...)
    lPartIDsArraySize=0;		// memory size allocated to store PartIDs
    ptResultArrayIndexes = NULL;// Pinmap index list, used when processing MPR
    ptFirstResultArrayIndexes = NULL;// Pinmap index list, store first occurence
    mPinCount = 0;
    lfTmpSamplesTotal=0;		// Sum of X (including outliers)
    lfTmpSamplesTotalSquare=0;	// Sum of X*X (including outliers)
    ldTmpSamplesExecs=0;
    lfTmpHistogramMin=C_INFINITE;// Min. of values
    lfTmpSamplesMin=C_INFINITE;	// Min. of values
    lfTmpHistogramMax=-C_INFINITE;// Max. of values
    lfTmpSamplesMax=-C_INFINITE;// Max. of values
    res_scal = 0;				// No scaling
    llm_scal = 0;
    hlm_scal = 0;
    iAlarm =0;					// Includes bit flags for drifting alarm detected when doing correlation check (multiple groups)
    iFailBin=-1;				// Soft Bin associated with this test when failing.
    iHtmlHistoPage = 0;			// Tells to which Histogram HTML page index test belongs
    iHtmlStatsPage = 0;			// Tells to which Statistic HTML page index test belongs
    iHtmlAdvancedPage = 0;		// Tells to which HTML page index the test Advanced chart belongs (Trend, etc...)
    iHtmlWafermapPage = 0;		// Tells to which Wafer Map HTML page index test belongs
    bTsrIncorrectScale=false;
    bTestExecuted=false;
    bTestInList=false;
    bTestType = ' ';			// TestType: Parametric,Functional,...
    bStatsFromSamples = true;	// Statistics mode:
    lFirstPartResult=0;		// First part# collected for Advanced charting
    lLastPartResult=0;		// Last part# collected for Advanced charting
    lfLowSpecLimit		= -C_INFINITE;			// Test Low Spec Limit
    lfHighSpecLimit		= C_INFINITE;			// Test High Spec Limit
    bOutlierLimitsSet=false;// true once outlier limits are computer.
    lfStartIn=0;			// Multiparamteric: starting input value condition (for shmoo)
    lfIncrementIn=0;		// Multiparametric: increment of input condition (for shmoo)
    lfMean=0;				// Mean Built after pass2 completed...
    mTestShifts.clear();
    lfT_Test=0;				// Student's T-Test after pass2 completed...
    lfP_Value=0;
    lfSigma=0;				// Sigma Built after pass2 completed...
    lfRange=0;
    lfMaxRange=0;
    lfMin=C_INFINITE;		// Min. of values
    lfMax=-C_INFINITE;		// Max. of values
    lfSamplesMin=C_INFINITE;// Min. of values
    lfSamplesMax=-C_INFINITE;// Max. of values
    // No limits defined
    //bLimitFlag= CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL | CTEST_LIMITFLG_NOLSL | CTEST_LIMITFLG_NOHSL;
    // bit0=1 (no low limit forced), bit1=1 (no high limit forced)
    //bLimitWhatIfFlag=CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL;
    strTestName="";			// Test Name.
    *szTestLabel=0;			// TestNumber.Pinmap# ...string used in HTML, CSV reports
    *szTestUnits= 0;		// Test Units.
    *szTestUnitsIn= 0;		// Input condition Test Units (Multiparametric test)

    *szLowSpecL= 0;			// Test low Spec limit.
    *szHighSpecL= 0;		// Test low Spec limit.
    ptNextTest = NULL;
    ptChartOptions = NULL;	// Custom charting options.
    mDistribution = PATMAN_LIB_SHAPE_UNSET;
    mBimodalSplitValue = -C_INFINITE;
    for(int iIndex=0;iIndex < TEST_HISTOSIZE;iIndex++)
        lHistogram[iIndex] = 0;
    bTestNameMapped = false;

    // Advanced Statistics
    lfSamplesSkew	= -C_INFINITE;		// Skew (degree of asymmetry of a distribution around the mean)
    // Kurtosis or relative peakedness or flatness of a distribution compared with the normal distribution
    lfSamplesKurt	= -C_INFINITE;
    lfSamplesP0_5 = -C_INFINITE;		// Data at end of 0.5% of sorted data.
    lfSamplesP2_5 = -C_INFINITE;		// Data at end of 2.5% of sorted data.
    lfSamplesP10 = -C_INFINITE;		// Data at end of 10% of sorted data.
    lfSamplesQuartile1 = -C_INFINITE;	// Data at end of 25% of sorted data.
    lfSamplesQuartile2 = -C_INFINITE;	// Median: Data at end of 50%
    lfSamplesQuartile3 = -C_INFINITE;	// Data at end of 75% of sorted data.
    lfSamplesP90 = -C_INFINITE;		// Data at end of 90% of sorted data.
    lfSamplesP97_5 = -C_INFINITE;		// Data at end of 97.5% of sorted data.
    lfSamplesP99_5 = -C_INFINITE;		// Data at end of 99.5% of sorted data.
    lfSamplesStartAfterNoise= -C_INFINITE;	// Quartile1, without first few percents
    lfSamplesEndBeforeNoise= -C_INFINITE;		// Last quartile without last few percents.
    lfSamplesSigmaInterQuartiles = -C_INFINITE;// Sigma of values from quartile 1 to Quartile 3 values.
    lfSamplesSkewWithoutNoise=0;	// Skew computed on data without the first and last X% (the noise)
    lSamplesMaxCell = 0;
    lSamplesMinCell = 0;
    // Holds the number of different values available in the dataset
    // (used to detect distributions with only few distinct values)
    lDifferentValues = 0;
    bIntegerValues=true; // cleared to 'false' if samples are NOT integer numbers but floatig point values.
    m_eOrigin = CTest::OriginStdf;

    lTestID = -1;
    // Reset custom markers list
    ptCustomMarkers.clear();

    // Functional test: Empty failing vector lists
    mVectors.clear();

    // Gage R&R
    pGage = NULL;

    // XBar-R
    m_pXbarRDataset = NULL;

    // Multi-result count
    m_cMaxCount = 0;
    clearMultiResultCount();

    m_iHistoClasses = 40;
    ptPreviousTest = 0;
    bStatsFromSamples = true;	// Statistics mode:
    bTestExecuted = false;
    *szTestUnits= 0;		// Test Units.
    *szTestUnitsIn= 0;		// Input condition Test Units (Multiparametric test)

    mFirstMultiLimits = false;
    mCurrentLimitItem = new GS::Core::MultiLimitItem();
    AddMultiLimitItem(mCurrentLimitItem);
}


CTest::~CTest()
{
    // Delete array of indexes if we had MPR tests
    if(ptResultArrayIndexes!= NULL)
        delete [] ptResultArrayIndexes;
    // Delete array of first occurence of RTN_INDX
    if(ptFirstResultArrayIndexes != NULL)
        delete [] ptFirstResultArrayIndexes;

    // If custom charting options, destroy structure
    if(ptChartOptions != NULL)
        delete ptChartOptions;
    ptChartOptions=0;

    // If gage R&R computed, delete it
    if(pGage != NULL)
        delete pGage;
    pGage = NULL;

    if (m_pXbarRDataset)
    {
        delete m_pXbarRDataset;
        m_pXbarRDataset = NULL;
    }

    // Ensure pointers are reset too.
    ptResultArrayIndexes=NULL;
    ptFirstResultArrayIndexes = NULL;
    ptChartOptions=NULL;

    s_nInstances--;
    for (int i=0; i<mMultiLimits.size(); ++i)
    {
        delete mMultiLimits[i];

// HTH: This doesnt look useful anymore since GCORE-10297
//        //-- the unvalid limt can be shared,
//        if(mMultiLimits[i]->IsUnvalidLimit() == false)
//            delete mMultiLimits[i];

    }
    mMultiLimits.clear();
    mCurrentLimitItem = 0;
}

void CTest::addMultiResultCount(unsigned short site)
{
    m_cSiteCount[site]++;
    m_cMaxCount = qMax(m_cMaxCount, m_cSiteCount[site]);
}

void CTest::clearMultiResultCount()
{
    m_cSiteCount.clear();
}

bool CTest::IsMprMasterTest() const
{
    return (lResultArraySize > 0);
}

bool CTest::isFailingValue(double lfValue, CTestResult::PassFailStatus ePassFailStatus,
                           bool *ptLowFail/*=NULL*/,bool *ptHighFail/*=NULL*/)
{
    // No pass fail status for this run index
    if (ePassFailStatus == CTestResult::statusUndefined ||
            // If limit WhatIf was defined (PassStatus = Undefined)
            ((mCurrentLimitItem->bLimitWhatIfFlag & (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL))
             != (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL)))
    {
         // Check with the new limits
        if((mCurrentLimitItem->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
        {
            // Low limit exists
            if(lfValue < mCurrentLimitItem->lfLowLimit
               || ((lfValue == mCurrentLimitItem->lfLowLimit) && ((mCurrentLimitItem->bLimitFlag & CTEST_LIMITFLG_LTLNOSTRICT) == 0)))
            {
                if(ptLowFail)
                    *ptLowFail = true;
                return true;
            }
        }

        if((mCurrentLimitItem->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
        {
            // High limit exists
            if(lfValue > mCurrentLimitItem->lfHighLimit
               || ((lfValue == mCurrentLimitItem->lfHighLimit) && ((mCurrentLimitItem->bLimitFlag & CTEST_LIMITFLG_HTLNOSTRICT) == 0)))
            {
                if(ptHighFail)
                    *ptHighFail = true;
                return true;
            }
        }

        if (bTestType == 'F')
            return (lfValue == 0.0);
    }
    // Else just return the standard flag
    else if (ePassFailStatus == CTestResult::statusFail)
        return true;

    // PASS value.
    return false;
}

long CTest::FindValidResultOffset(long lValidValueOccurante)
{
    // Check if not enough data to find requested value
    if(lValidValueOccurante  < 0 || lValidValueOccurante >= ldSamplesValidExecs)
        return -1;

    long lRunOffset=0;
    do
    {
        if(m_testResult.isValidResultAt(lRunOffset))
        {
            // Decrement total valid samples found
            lValidValueOccurante--;

            // If reached specified sample, return!
            if(lValidValueOccurante < 0)
                return lRunOffset;
        }

        // Check next run#
        lRunOffset++;
    }
    while(1);
}

int CTest::FindValidResult(long lIndex,double &lfValue)
{
    // Check if not enough data to find requested value
    if(lIndex  < 0 || lIndex >= ldSamplesValidExecs)
        return -1;

    long lRunOffset=0;
    do
    {
        if(m_testResult.isValidResultAt(lRunOffset)
                || lTestNumber == GEX_TESTNBR_OFFSET_EXT_DIEX
                || lTestNumber == GEX_TESTNBR_OFFSET_EXT_DIEY)
        {
            // Get the result value
            lfValue = m_testResult.resultAt(lRunOffset);

            // Decrement total valid samples found
            lIndex--;

            // If reached specified sample, return!
            if(lIndex < 0)
                return 1;
        }

        // Check next run#
        lRunOffset++;
    }
    while(1);
  return -1;
}

void CTest::findSublotOffset(int& nBegin, int& nEnd, int nSublot) const
{
    // Get 1st run# offset in results array (in case multiple files merged)
    int	 nIndex	= 0;

    // Initialize begin and end offset
    nBegin	= 0;
    nEnd	= 0;

    while (nIndex < nSublot && nIndex < pSamplesInSublots.count()-1)
    {
        // Compute sample 'ending' offset
        nBegin += pSamplesInSublots[nIndex];

        // Next sub-lot
        nIndex++;
    };

    // Get last run# in results array
    if(nIndex == (int) pSamplesInSublots.count()-1 || pSamplesInSublots.count() == 0)
        nEnd = ldSamplesExecs;
    else
        nEnd = nBegin + pSamplesInSublots[nIndex];
}

void CTest::executeSmartScaling()
{
    // Apply only to parametric and multi-parametric test
    if ((bTestType == 'P' || bTestType == 'M') && (m_testResult.count() > 0)
            && szTestUnits[0] != '%' && szTestUnits[0] != 0)
    {
        // Find out what is the most appropriate scaling factor for this test
        int nSmartScalingFactor = findoutSmartScalingFactor();

        // Smart scaling factor differs from result scaling, then rescale all results
        if (nSmartScalingFactor != res_scal)
        {
            m_testResult.rescaleResults(res_scal, nSmartScalingFactor);

            // set the new result scaling factor
            res_scal = nSmartScalingFactor;
        }
    }
}

bool CTest::UpdateProperties()
{
    mProperties.insert("testnumber", QVariant(lTestNumber));
    mProperties.insert("testname", QVariant(strTestName));

    mProperties.insert("pinmapindex",  QVariant(lPinmapIndex) );
    mProperties.insert("testID", QVariant(lTestID) );
    mProperties.insert("testFlowID", QVariant(lTestFlowID) );
    mProperties.insert("samplesValidExecs", QVariant(ldSamplesValidExecs) );
    mProperties.insert("samplesExecs", QVariant( ldSamplesExecs ) );
    mProperties.insert("execs", QVariant(ldExecs) );
    mProperties.insert("sampleFails", QVariant(mCurrentLimitItem->ldSampleFails) );
    mProperties.insert("failCount", QVariant(mCurrentLimitItem->ldFailCount) );
    mProperties.insert("removed_count", QVariant(mCurrentLimitItem->ldOutliers) );

    mProperties.insert("median", QVariant(lfSamplesQuartile2));
    mProperties.insert("mean", QVariant(lfMean));
    mProperties.insert("min", QVariant(lfMin));
    mProperties.insert("max", QVariant(lfMax));
    mProperties.insert("sigma", QVariant(lfSigma));
    mProperties.insert("cp", QVariant(mCurrentLimitItem->lfCp) );

    TestShift lShift;
    GS::Core::MLShift lMLShift;
    if (mTestShifts.size() > 0)
    {
        lShift = mTestShifts.first();
        lMLShift = lShift.GetMlShift(mCurrentLimitItem);
    }
    mProperties.insert("cpShift", QVariant(lMLShift.mCpShift) );
    mProperties.insert("crShift", QVariant(lMLShift.mCrShift) );
    mProperties.insert("cpk", QVariant(mCurrentLimitItem->lfCpk) );
    mProperties.insert("cpl", QVariant(mCurrentLimitItem->lfCpkLow) );
    mProperties.insert("cph", QVariant(mCurrentLimitItem->lfCpkHigh) );
    mProperties.insert("cpkShift", QVariant(lMLShift.mCpkShift) );

    mProperties.insert("maxRange", QVariant(lfMaxRange) );
    mProperties.insert("samplesMin", QVariant(lfSamplesMin) );
    mProperties.insert("samplesMax", QVariant(lfSamplesMax) );

    mProperties.insert("samplesSkew", QVariant(lfSamplesSkew) );

    mProperties.insert("6sigma", QVariant(6*lfSigma) );
    mProperties.insert("3sigma", QVariant(3*lfSigma) );
    mProperties.insert("ul", QVariant(mCurrentLimitItem->lfHighLimit) );
    mProperties.insert("ll", QVariant(mCurrentLimitItem->lfLowLimit) );
    mProperties.insert("lowSpecsLimit", QVariant(lfLowSpecLimit));
    mProperties.insert("highSpecsLimit", QVariant(lfHighSpecLimit));

    mProperties.insert("lowLimitOutlier", QVariant(mCurrentLimitItem->lfLowLimitOutlier));
    mProperties.insert("highLimitOutlier", QVariant(mCurrentLimitItem->lfHighLimitOutlier));

    return true;
}

int CTest::findoutSmartScalingFactor()
{
    if (res_scal == 2)
        return 2;

    // Check the limits
    double	dMinValue	= 0;
    double	dMaxValue	= 0;
    double	dMeanValue	= 0;
    int		nScale		= 0;

    if (lfMin != C_INFINITE)
        dMinValue = fabs(lfMin);

    if (lfMax != -C_INFINITE)
        dMaxValue = fabs(lfMax);

    dMeanValue = (dMaxValue + dMinValue) / 2;

    if (dMeanValue)
    {
        // Get power of 10 for the middle point of the limits in absolute value
        int	nExponent = (int) log10(dMeanValue);
        nExponent -= (dMeanValue >= 1) ? 0 : 1;

        if(nExponent <= -13)
            nScale =  15;	// Fento
        else if(nExponent >= 12)
            nScale =  -12;	// Tera
        else
        {
            switch(nExponent)
            {
                case -12:
                case -11:
                case -10:
                    nScale =  12;	// 'n'
                    break;

                case -9:
                case -8:
                case -7:
                    nScale = 9;	// 'p'
                    break;

                case -6:
                case -5:
                case -4:
                    nScale = 6;	// 'u'
                    break;

                case -1:
                case -2:
                case -3:
                    nScale = 3;	// 'm'
                    break;

                default:
                case 0:
                case 1:
                case 2:
                    nScale = 0;
                    break;

                case 3:
                case 4:
                case 5:
                    nScale = -3;	// K
                    break;

                case 6:
                case 7:
                case 8:
                    nScale = -6;	// M
                    break;

                case 9:
                case 10:
                case 11:
                    nScale = -9;	// G
                    break;
            }
        }
    }

    return nScale;
}

void CTest::uniformizePercentUnits()
{
    if (szTestUnits[0] == '%' && res_scal == 2)
        szTestUnits[0] = '\0';
}


QMap<QString, QMap<uint,int> >  CTest::buildFunctionalHistoData(int iHistoType, QString strPattern="")
{
    QStringList oStrPatternList;
    if(strPattern.isEmpty())
        oStrPatternList = mVectors.keys();
    else
        oStrPatternList.append(strPattern);
    QMap<QString, QMap<uint,int> > oMapHistoList;
    foreach(const QString &strPattern, oStrPatternList){
        CFunctionalTest &oFuncTest = mVectors[strPattern];//Functional Test Data
        QMap<uint,int> oHisto;
        QList<GexVectorFailureInfo>	&lstVectorInfo = oFuncTest.m_lstVectorInfo;
        for(int iVectorInfo=0; iVectorInfo <lstVectorInfo.count(); ++iVectorInfo){

            if(iHistoType == GEX_ADV_FUNCTIONAL_CYCL_CNT){
                uint uiVectorCycleCount = lstVectorInfo[iVectorInfo].vectorCycleCount();
                if(!oHisto.contains(uiVectorCycleCount)){
                    oHisto.insert(uiVectorCycleCount, 1);
                } else {
                    oHisto[uiVectorCycleCount]++;
                }
            }else if(iHistoType == GEX_ADV_FUNCTIONAL_REL_VAD){
                uint uiRelativeVectorAddress = lstVectorInfo[iVectorInfo].relativeVectorAddress();
                if(!oHisto.contains(uiRelativeVectorAddress)){
                    oHisto.insert(uiRelativeVectorAddress, 1);
                } else {
                    oHisto[uiRelativeVectorAddress]++;
                }
            }

        }
        oMapHistoList.insert(strPattern, oHisto);
    }

    return oMapHistoList;
}

QString CTest::GetScaledUnits(double * pCustomScaleFactor, const QString & scalingOption)
{
    QString strUnits;
    *pCustomScaleFactor = 1.0;

    // If we have to keep values in normalized format, do not rescale!
    if (scalingOption != "normalized")
    {
      int nScale = res_scal;

      switch (nScale)
      {
      case 0:
        strUnits = " ";
        break;

      default:
        *pCustomScaleFactor = 1 / GS_POW(10.0, res_scal);
        strUnits               = " e" + QString::number(-res_scal);
        break;

      case 253:  // for unsigned -3
      case -3:
        strUnits = " K";
        break;
      case 250:  // for unsigned -6
      case -6:
        strUnits = " M";
        break;
      case 247:  // for unsigned -9
      case -9:
        strUnits = " G";
        break;
      case 244:  // for unsigned -13
      case -12:
        strUnits = " T";
        break;
      case 2:
        if (szTestUnits[0] != '%')
        {
          strUnits = " %";
        }
        break;
      case 3:
        strUnits = " m";
        break;
      case 6:
        strUnits = " u";
        break;
      case 9:
        strUnits = " n";
        break;
      case 12:
        strUnits = " p";
        break;
      case 15:
        strUnits = " f";
        break;
      }
    }

    if (szTestUnits[0])
    {
      strUnits += szTestUnits;
    }

    return strUnits;
}

QString CTest::generateTestKey(int lTestMergeRule)
{
    if (lTestMergeRule == TEST_MERGE_NUMBER)
    {
        //Ignore test name: always merge tests with identical test number|merge
        //Merge by test name: pinmap + test name

        return QString("%1_%2").arg(lPinmapIndex).arg(lTestNumber);
    }
    else if (lTestMergeRule == TEST_MERGE_NAME)
    {
        //Ignore test#: always merge tests with identical name|merge_name
        //Merge by test number: pinmap + test number
        return QString("%1_%2").arg(lPinmapIndex).arg(strTestName);
    }
    else
    {
        //Never merge tests with identical test number if test name not matching|no_merge"
        //Never merge: pinmap + test number + test name
        return QString("%1_%2_%3").arg(lPinmapIndex).arg(lTestNumber).arg(strTestName);
    }
}

int CTest::GetDistribution() const
{
    return mDistribution;
}

void CTest::SetDistribution(int distrib)
{
    mDistribution = distrib;
}

double CTest::GetBimodalSplitValue() const
{
    return mBimodalSplitValue;
}

void CTest::SetBimodalSplitValue(double distrib)
{
    mBimodalSplitValue = distrib;
}

double CTest::getP_Value()
{
    return lfP_Value;
}
void   CTest::setP_Value(double dVal)
{
    lfP_Value = dVal;
}

void CTest::AddLimitItem(GS::Core::MultiLimitItem* limitItem)
{
    if (limitItem)
    {
        mMultiLimits.append(limitItem);
        GS::Core::MultiLimitItem::UpdateNbMaxLimits(mMultiLimits.size());
    }
}

bool CTest::AddMultiLimitItem(GS::Core::MultiLimitItem* limitItem)
{
    if (limitItem)
    {
        // Add the first element (comes from PTR)
        if (mMultiLimits.size() == 0)
        {
            AddLimitItem(limitItem);
            return true;
        }
        else
        {
            // If false, means, this is the first multi limit, and it is equal to the standard limits
            if(mFirstMultiLimits == false)
            {
                mFirstMultiLimits = true;
                return false;
            }
            // Compare only between Multi Limits and the first item is from PTR (standard limits) and has HBIN = -1
            for (int i=0; i<mMultiLimits.size(); ++i)
            {
                if (mMultiLimits[i]->IsEqual(limitItem))
                {
                    return false;
                }
            }
            // GCORE-6093: Allow to keep duplicate Multi-limit
            AddLimitItem(limitItem);
            return true;
        }
    }
    return false;
}

int CTest::MultiLimitItemCount() const
{
    return mMultiLimits.size();
}

void CTest::CompleteWithInvalideLimit(int nbMultiLimits)
{
    if(nbMultiLimits > mMultiLimits.size() )
    {
        for(int i = 0; i <= (nbMultiLimits - mMultiLimits.size()); ++i)
        {
            GS::Core::MultiLimitItem* lInvalidLimit = new GS::Core::MultiLimitItem();
            lInvalidLimit->SetValid(false) ;
            AddLimitItem(lInvalidLimit);
        }
    }
}

GS::Core::MultiLimitItem* CTest::GetCurrentLimitItem() const
{
    return mCurrentLimitItem;
}

GS::Core::MultiLimitItem* CTest::GetMultiLimitItem(int index)
{
    //-- complete the missing indexes with the invalid Limit if needed
    CompleteWithInvalideLimit(index + 1);

    if ((index >= 0) && (index < mMultiLimits.size()))
    {
        return mMultiLimits.at(index);
    }
    else

    return 0;
}

double CTest::getT_Test()
{
    return lfT_Test;
}
void   CTest::setT_Test(double dVal)
{
    lfT_Test = dVal;
}


void CTest::UpdateSampleFails(int site)
{

    if(mTotalFailPerSite.contains(site))
    {
        ++mTotalFailPerSite[site];
    }
    else
    {
          mTotalFailPerSite[site] = 1;
    }
}

BOOL CTest::getBStatsFromSamples() const
{
    return bStatsFromSamples;
}

void CTest::setBStatsFromSamples(const BOOL &value)
{
    bStatsFromSamples = value;
}

void CTest::setCurrentLimitItem(GS::Core::MultiLimitItem *currentLimitItem)
{
    mCurrentLimitItem = currentLimitItem;
}

bool CTest::setCurrentLimitItem(int index)
{
    //-- complete the missing indexes with the invalid Limit if needed
    CompleteWithInvalideLimit(index + 1);

    if ((index >= 0) && (index < mMultiLimits.size()))
    {
        mCurrentLimitItem = mMultiLimits.at(index);
        return true;
    }

    return false;
}

//double CTest::GetRelevantCpk(int lOutliersToKeep)
//{
//    switch(lOutliersToKeep)
//    {
//        case GEX_TPAT_KEEPTYPE_NONEID:
//        default:
//            return lfCpk;
//        case GEX_TPAT_KEEPTYPE_LOWID:	// If keep low outliers, then ignore Low limit for Cpk computation, take CpkHigh
//            return lfCpkHigh;
//        case GEX_TPAT_KEEPTYPE_HIGHID:	// If keep high outliers, then ignore High limit for Cpk computation, take CpkLow
//            return lfCpkLow;
//    }
//    return 0.;
//}
#endif	// #ifdef GEX_CTEST_CPP__INCLUDED_

