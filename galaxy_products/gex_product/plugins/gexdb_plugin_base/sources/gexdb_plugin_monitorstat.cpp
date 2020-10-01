#include <math.h>

#include "gexdb_plugin_monitorstat.h"
#include "gqtl_log.h"

double c4Factors[26] = {
    -1,
    -1,
    0.797885,
    0.886227,
    0.921318,
    0.939986,
    0.951533,
    0.959369,
    0.965030,
    0.969311,
    0.972659,
    0.975350,
    0.977559,
    0.979406,
    0.980971,
    0.982316,
    0.983484,
    0.984506,
    0.985410,
    0.986214,
    0.986934,
    0.987583,
    0.988170,
    0.988705,
    0.989193,
    0.989640
};


GexDbPlugin_MonitorStat::GexDbPlugin_MonitorStat()
{
    ResetStats();
}

GexDbPlugin_MonitorStat::~GexDbPlugin_MonitorStat()
{
}

void GexDbPlugin_MonitorStat::Reset()
{
    dataPointList.clear();
    dataPointMapByValue.clear();
    dataPointMapNoOutliersByFilteredValue.clear();
    lotsWafersAlreadyInserted.clear();

    ResetStats();
}

void GexDbPlugin_MonitorStat::ResetStats(void)
{
    // Data cleaning flags on samples and
    // Reset the map without outliers
    dataPointMapNoOutliersByFilteredValue.clear();
    for(int i = 0; i < dataPointList.size(); ++i)
    {
        GexDbPlugin_MonitorDataPoint* dp = &dataPointList[i];
        dp->m_bIsOutlier = false;
        dataPointMapNoOutliersByFilteredValue.insertMulti(dp->m_filteredValue, dp);
    }

    mQuartile1_AllItems_Index = 0;
    mQuartile1_AllItems_Value = 0.0;
    mQuartile1_Filtered_Index = 0;
    mQuartile1_Filtered_Value = 0.0;

    mMedian_AllItems_Index = 0;
    mMedian_AllItems_Value = 0.0;
    mMedian_Filtered_Index = 0;
    mMedian_Filtered_Value = 0.0;

    mQuartile3_AllItems_Index = 0;
    mQuartile3_AllItems_Value = 0.0;
    mQuartile3_Filtered_Index = 0;
    mQuartile3_Filtered_Value = 0.0;

    mPercentileN_AllItems_Index = 0;
    mPercentileN_AllItems_Value = 0.0;
    mPercentileN_Filtered_Index = 0;
    mPercentileN_Filtered_Value = 0.0;

    mPercentile100lessN_AllItems_Index = 0;
    mPercentile100lessN_AllItems_Value = 0.0;
    mPercentile100lessN_Filtered_Index = 0;
    mPercentile100lessN_Filtered_Value = 0.0;

    mAvg_AllItems = 0.0;
    mStd_AllItems = 0.0;
    mMin_AllItems = 0.0;
    mMax_AllItems = 0.0;

    mAvg_Filtered = 0.0;
    mStd_Filtered = 0.0;
    mMin_Filtered = 0.0;
    mMax_Filtered = 0.0;

    baseStatsComputed = false;
}

void GexDbPlugin_MonitorStat::Append(GexDbPlugin_MonitorDataPoint& dataPoint)
{
    dataPointList.append(dataPoint);
    GexDbPlugin_MonitorDataPoint* listCopyRef = &dataPointList.last();
    dataPointMapByValue.insertMulti(
                dataPoint.m_value,
                listCopyRef);
    dataPointMapNoOutliersByFilteredValue.insertMulti(
                dataPoint.m_filteredValue,
                listCopyRef);
    lotsWafersAlreadyInserted.insert(
                QString(dataPoint.m_strLotID + "." + dataPoint.m_strWaferSublotID).toUpper());

    baseStatsComputed = false;
}

bool GexDbPlugin_MonitorStat::ProcessStat(bool removeOutliers, OutlierRule outlierRule, double factorN, double& lowLimit, double& highLimit, bool forceCompute)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, (QString(" using rule %1 and N=%2 for %3 datas...")
         .arg(outlierRule)
         .arg(factorN)
         .arg(dataPointMapByValue.size())).toLatin1().constData());

    if (dataPointList.size()==0)
    {
        GSLOG(SYSLOG_SEV_NOTICE, "no DataPoints provided, can't remove any outlier");
        return false;
    }

    if(factorN < 0.0)
    {
        return false;
    }

    // Compute stats if not already done
    if (!baseStatsComputed || forceCompute)
    {
        if (!ComputeStats(factorN))
        {
            return false;
        }
    }

    if(removeOutliers)
    {
        // Recursively Data cleaning one by one

        double                           farestExtreme=0.0;				// Holds farest from mean betweem min and max
        double                           nearestExtreme=0.0;				// Holds nearest from mean betweem min and max
        double                           outlierValue=0.0;					// Holds the outlier value to remove

        // Data cleaning

        // Compute new outlier limits
        if(!GetLimits(outlierRule, factorN, lowLimit, highLimit))
        {
            return false;
        }

        bool bOutliers = true;

        while(bOutliers
              && (dataPointMapNoOutliersByFilteredValue.size() != 0)
              && (mMax_Filtered != mMin_Filtered) )
        {
            bOutliers = false;

            // Get farest and nearest extremes
            if(fabs(mMin_Filtered-mAvg_Filtered) > fabs(mMax_Filtered-mAvg_Filtered))
            {
                farestExtreme = mMin_Filtered;			// Get 'Min' value from the yield samples
                nearestExtreme = mMax_Filtered;		// Get 'Max' value from the yield samples
            }
            else
            {
                farestExtreme = mMax_Filtered;			// Get 'Max' value from the yield samples
                nearestExtreme = mMin_Filtered;		// Get 'Min' value from the yield samples
            }

            // Check if farest extreme is an outlier
            if((farestExtreme < lowLimit) || (farestExtreme > highLimit))
            {
                bOutliers = true;
                outlierValue = farestExtreme;
            }
            // If not, check if nearest extreme is an outlier
            else if((nearestExtreme < lowLimit) || (nearestExtreme > highLimit))
            {
                bOutliers = true;
                outlierValue = nearestExtreme;
            }

            // Remove all datapoints matching the OutlierValue to remove.
            if(bOutliers)
            {
                QList<GexDbPlugin_MonitorDataPoint*> lOutliers = dataPointMapNoOutliersByFilteredValue.values(outlierValue);
                for(int i=0; i<lOutliers.size(); ++i)
                {
                    lOutliers.at(i)->m_bIsOutlier = true;
                }
                dataPointMapNoOutliersByFilteredValue.remove(outlierValue);

                // Recompute stats (sigma,...)
                if (!ComputeStats(factorN))
                {
                    return false;
                }
            }

            // Compute new outlier limits
            if(!GetLimits(outlierRule, factorN, lowLimit, highLimit))
            {
                return false;
            }
        };	// while : Keep looping until no more outliers!
    }
    else
    {
        if(!GetLimits(outlierRule, factorN, lowLimit, highLimit))
        {
            return false;
        }
    }

    return true;
}

bool GexDbPlugin_MonitorStat::ComputeStats(double percentileN)
{
    if (dataPointList.size()==0)
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, "Compute stats impossible : no DataPoints for computation !");
        return false;
    }

    bool                            minSet_AllItems=false, maxSet_AllItems=false;
    double                          sum_AllItems=0, squareSum_AllItems=0;

    bool                            minSet_Filtered=false, maxSet_Filtered=false;
    double                          sum_Filtered=0, squareSum_Filtered=0;

    unsigned int                    samples;
    GexDbPlugin_MonitorDataPoint*   dataPoint;

    if(!baseStatsComputed)
    {
        // Compute global stats

        for(int i = 0; i < dataPointList.size(); ++i)
        {
            dataPoint = &dataPointList[i];
            if(!minSet_AllItems)
            {
                mMin_AllItems = dataPoint->m_value;
                minSet_AllItems = true;
            }
            else if(dataPoint->m_value < mMin_AllItems)
            {
                mMin_AllItems = dataPoint->m_value;
            }

            if(!maxSet_AllItems)
            {
                mMax_AllItems = dataPoint->m_value;
                maxSet_AllItems = true;
            }
            else if(dataPoint->m_value > mMax_AllItems)
            {
                mMax_AllItems = dataPoint->m_value;
            }

            sum_AllItems += dataPoint->m_value;
            squareSum_AllItems += dataPoint->m_value*dataPoint->m_value;
        }

        samples = dataPointMapByValue.size();
        mAvg_AllItems = sum_AllItems/(samples);
        if(samples > 1)
        {
            // Std dev formula
            /*mStd_AllItems =
                (double)sqrt(
                    (squareSum_AllItems / (double)samples)
                    -
                    pow(sum_AllItems / (double)samples, 2.0)
                );*/
            // Sample std dev biaised estimator
            mStd_AllItems =
                (double)sqrt(
                    fabs(
                        (((double)samples*squareSum_AllItems) - pow(sum_AllItems,2.0))
                            /
                        ((double)samples*((double)(samples-1)))
                    )
                );
            // Sample std dev unbiaised estimator
            // mStd_AllItems = GetC4Factor(samples) * mStd_AllItems;
        }
        else
        {
            mStd_AllItems = 0.0F;
        }

        mQuartile1_AllItems_Index = GetPercentileIndex(dataPointMapByValue.size(), 25.0);
        mQuartile1_AllItems_Value = dataPointMapByValue.keys().at(mQuartile1_AllItems_Index);
        // and because there are no outliers yet:
        mQuartile1_Filtered_Index = mQuartile1_AllItems_Index;
        mQuartile1_Filtered_Value = dataPointMapNoOutliersByFilteredValue.keys().at(mQuartile1_AllItems_Index);

        mMedian_AllItems_Index = GetPercentileIndex(dataPointMapByValue.size(), 50.0);
        mMedian_AllItems_Value = dataPointMapByValue.keys().at(mMedian_AllItems_Index);
        // and because there are no outliers yet:
        mMedian_Filtered_Index = mMedian_AllItems_Index;
        mMedian_Filtered_Value = dataPointMapNoOutliersByFilteredValue.keys().at(mMedian_AllItems_Index);

        mQuartile3_AllItems_Index = GetPercentileIndex(dataPointMapByValue.size(), 75.0);
        mQuartile3_AllItems_Value = dataPointMapByValue.keys().at(mQuartile3_AllItems_Index);
        // and because there are no outliers yet:
        mQuartile3_Filtered_Index = mQuartile3_AllItems_Index;
        mQuartile3_Filtered_Value = dataPointMapNoOutliersByFilteredValue.keys().at(mQuartile3_AllItems_Index);

        mPercentileN_AllItems_Index = GetPercentileIndex(dataPointMapByValue.size(), percentileN);
        mPercentileN_AllItems_Value = dataPointMapByValue.keys().at(mPercentileN_AllItems_Index);
        // and because there are no outliers yet:
        mPercentileN_Filtered_Index = mPercentileN_AllItems_Index;
        mPercentileN_Filtered_Value = dataPointMapNoOutliersByFilteredValue.keys().at(mPercentileN_AllItems_Index);

        mPercentile100lessN_AllItems_Index = GetPercentileIndex(dataPointMapByValue.size(), 100.0 - percentileN);
        mPercentile100lessN_AllItems_Value = dataPointMapByValue.keys().at(mPercentile100lessN_AllItems_Index);
        // and because there are no outliers yet:
        mPercentile100lessN_Filtered_Index = mPercentile100lessN_AllItems_Index;
        mPercentile100lessN_Filtered_Value = dataPointMapNoOutliersByFilteredValue.keys().at(mPercentile100lessN_AllItems_Index);

        baseStatsComputed = true;
    }

    // Compute filtered stats

    QMap<double, GexDbPlugin_MonitorDataPoint*>::iterator it = dataPointMapNoOutliersByFilteredValue.begin();
    while(it != dataPointMapNoOutliersByFilteredValue.end())
    {
        dataPoint = (*it);

        // Because of the initial design of this class, dataPoint->m_bIsOutlier can be set to true
        // by the client code in order to manually exclude some datapoints.
        // it would have been too long to refacto this (removing the m_bIsOutlier member would have
        // had other impacts).
        if(dataPoint->m_bIsOutlier)
        {
            it= dataPointMapNoOutliersByFilteredValue.erase(it);
            continue;
        }

        if(!minSet_Filtered)
        {
            mMin_Filtered = dataPoint->m_filteredValue;
            minSet_Filtered = true;
        }
        else if(dataPoint->m_filteredValue < mMin_Filtered)
        {
            mMin_Filtered = dataPoint->m_filteredValue;
        }

        if(!maxSet_Filtered)
        {
            mMax_Filtered = dataPoint->m_filteredValue;
            maxSet_Filtered = true;
        }
        else if(dataPoint->m_filteredValue > mMax_Filtered)
        {
            mMax_Filtered = dataPoint->m_filteredValue;
        }

        sum_Filtered += dataPoint->m_filteredValue;
        squareSum_Filtered += dataPoint->m_filteredValue*dataPoint->m_filteredValue;

        it++;
    }

    if(dataPointMapNoOutliersByFilteredValue.size()>0)
    {
        samples = dataPointMapNoOutliersByFilteredValue.size();
        mAvg_Filtered = sum_Filtered/(samples);
        if(samples > 1)
        {
            // Std dev formula
            /*mStd_Filtered =
                (double)sqrt(
                    (squareSum_Filtered / (double)samples)
                    -
                    pow(sum_Filtered / (double)samples, 2.0)
                );*/
            // Sample std dev biaised estimator
            mStd_Filtered =
                (double)sqrt(
                    fabs(
                        (((double)samples*squareSum_Filtered) - pow(sum_Filtered,2.0))
                            /
                        ((double)samples*((double)(samples-1)))
                    )
                );
            // Sample std dev unbiaised estimator
            // mStd_Filtered = GetC4Factor(samples) * mStd_Filtered;
        }
        else
        {
            mStd_Filtered = 0.0;
        }

        mQuartile1_Filtered_Index = GetPercentileIndex(dataPointMapNoOutliersByFilteredValue.size(), 25.0);
        mQuartile1_Filtered_Value = dataPointMapNoOutliersByFilteredValue.keys().at(mQuartile1_Filtered_Index);

        mMedian_Filtered_Index = GetPercentileIndex(dataPointMapNoOutliersByFilteredValue.size(), 50.0);
        mMedian_Filtered_Value = dataPointMapNoOutliersByFilteredValue.keys().at(mMedian_Filtered_Index);

        mQuartile3_Filtered_Index = GetPercentileIndex(dataPointMapNoOutliersByFilteredValue.size(), 75.0);
        mQuartile3_Filtered_Value = dataPointMapNoOutliersByFilteredValue.keys().at(mQuartile3_Filtered_Index);

        mPercentileN_Filtered_Index = GetPercentileIndex(dataPointMapNoOutliersByFilteredValue.size(), percentileN);
        mPercentileN_Filtered_Value = dataPointMapNoOutliersByFilteredValue.keys().at(mPercentileN_Filtered_Index);

        mPercentile100lessN_Filtered_Index = GetPercentileIndex(dataPointMapNoOutliersByFilteredValue.size(), 100.0 - percentileN);
        mPercentile100lessN_Filtered_Value = dataPointMapNoOutliersByFilteredValue.keys().at(mPercentile100lessN_Filtered_Index);

        GSLOG(SYSLOG_SEV_DEBUG, (QString("Compute stats: total %1 datapoints (%2 not outliers)")
              .arg(dataPointMapByValue.size())
              .arg(dataPointMapNoOutliersByFilteredValue.size())).toLatin1().constData());

        return true;
    }

    GSLOG(SYSLOG_SEV_DEBUG, (QString("Compute stats error: not datapoint left after removing outliers")).toLatin1().constData());

    return false;
}

double GexDbPlugin_MonitorStat::GetC4Factor(int numberOfDataPoints)
{
    if(numberOfDataPoints < 26)
    {
        return c4Factors[numberOfDataPoints];
    }
    else if(numberOfDataPoints < 1000)
    {
        return (4*numberOfDataPoints-4)/(double)(4*numberOfDataPoints -3);
    }
    else
    {
        return 1.0;
    }
}

unsigned int GexDbPlugin_MonitorStat::GetPercentileIndex(unsigned int listSize, double percentile)
{
    if (percentile > 100.0) percentile=100.0;
    if (percentile < 0.0) percentile=0.0;
    if (percentile == 0.0) return 0;
    return (unsigned int)ceil(percentile/100.0*((double)listSize)) - 1;
}

bool GexDbPlugin_MonitorStat::GetLimits(OutlierRule outlierRule, double factorN, double& lowLimit, double& highLimit)
{
    double IQR=(mQuartile3_Filtered_Value-mQuartile1_Filtered_Value);
    double RobustSigma=IQR/1.5;
    switch(outlierRule)
    {
        case eNone:
            lowLimit = 0;
            highLimit = 0;
            GSLOG(SYSLOG_SEV_WARNING, "Limits computation: no outlier rule specified !");
        break;
        case eMeanNSigma:
            lowLimit = mAvg_Filtered - factorN*mStd_Filtered;
            highLimit = mAvg_Filtered + factorN*mStd_Filtered;
        break;
        case eMedianNRobustSigma:
            lowLimit = mMedian_Filtered_Value - factorN*RobustSigma;
            highLimit = mMedian_Filtered_Value + factorN*RobustSigma;
        break;
        case eMedianNIQR:
            lowLimit = mMedian_Filtered_Value - factorN*(IQR);
            highLimit = mMedian_Filtered_Value + factorN*(IQR);
        break;
        case ePercentil_0N100:
            lowLimit = mPercentileN_Filtered_Value;
            highLimit = mPercentile100lessN_Filtered_Value;
        break;
        case eQ1Q3NIQR:
            lowLimit = mQuartile1_Filtered_Value - factorN*(IQR);
            highLimit = mQuartile3_Filtered_Value + factorN*(IQR);
        break;
        default:
            GSLOG(SYSLOG_SEV_ERROR, QString("OutlierRule %1 not implemented or unknown !")
                  .arg( outlierRule).toLatin1().constData());
            return false;
        break;
    }
    return true;
}

unsigned int GexDbPlugin_MonitorStat::GetCurrentNumberOfOutliers()
{
    return dataPointMapByValue.size() - dataPointMapNoOutliersByFilteredValue.size();
}

bool GexDbPlugin_MonitorStat::GetLimitsAndParams(QList<double> datapoints,
                                                 bool removeOutliers,
                                                 OutlierRule outlierRule,
                                                 double factorN,
                                                 double &lowLimit,
                                                 double &highLimit,
                                                 double &mean,
                                                 double &sigma,
                                                 double &median,
                                                 double &Q1,
                                                 double &Q3,
                                                 double &percentN,
                                                 double &percent100MinusN,
                                                 int &numOutliers)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, (QString(" using rule %1 and N=%2 for %3 datas...")
         .arg(outlierRule)
         .arg(factorN)
         .arg(datapoints.size())).toLatin1().constData());

    if (datapoints.size()==0)
    {
        GSLOG(SYSLOG_SEV_NOTICE, "no DataPoints provided, can't remove any outlier");
        return false;
    }

    if(factorN < 0.0)
    {
        return false;
    }

    Reset();

    GexDbPlugin_MonitorDataPoint dataPoint;
    double value;
    QListIterator<double> iter(datapoints);
    while(iter.hasNext())
    {
        value = iter.next();
        dataPoint.m_value = value;
        dataPoint.m_filteredValue = value;
        Append(dataPoint);
    }

    // Compute stats if not already done
    if(!ProcessStat(removeOutliers, outlierRule, factorN, lowLimit, highLimit, true))
    {
        return false;
    }

    mean = mAvg_Filtered;
    sigma = mStd_Filtered;
    median = mMedian_Filtered_Value;
    Q1 = mQuartile1_Filtered_Value;
    Q3 = mQuartile3_Filtered_Value;
    percentN = mPercentileN_Filtered_Value;
    percent100MinusN = mPercentile100lessN_Filtered_Value;
    numOutliers = GetCurrentNumberOfOutliers();

    return true;
}
