#ifndef GEXDB_PLUGIN_MONITORSTAT_H
#define GEXDB_PLUGIN_MONITORSTAT_H

#include <QList>
#include <QHash>
#include "gexdb_plugin_monitordatapoint.h"
#include "gexdb_plugin_base.h"

///
/// \brief Computes and holds the stats of a set of values on 1 wafer/sublot
/// In the case of SYA, the set of values are bin yields
/// In the case of SPM, the set of values are mean, sigma or cpk values
///
class GexDbPlugin_MonitorStat
{
public:
    GexDbPlugin_MonitorStat();
    virtual ~GexDbPlugin_MonitorStat();

    ///
    /// \brief Recursively compute limits, identify and 'remove'(mark as) outliers using the given rule
    /// and sets the corresponding limits.
    /// The outliers will have the m_bIsOutlier set to true.
    /// \param the list of datapoints to process
    /// \param set to true to activate outlier removal during statistics computation
    /// \param specifies the rule to use while computing the limits and removing the outliers
    /// \param specifies the N factor to use while computing the limits and removing the outliers
    /// \param if the statistics computation succeeds, this parameter stores the stat low limit
    /// \param if the statistics computation succeeds, this parameter stores the stat high limit
    /// \param if the statistics computation succeeds, this parameter stores the stat mean
    /// \param if the statistics computation succeeds, this parameter stores the stat sigma
    /// \param if the statistics computation succeeds, this parameter stores the stat median
    /// \param if the statistics computation succeeds, this parameter stores the stat Q1
    /// \param if the statistics computation succeeds, this parameter stores the stat Q3
    /// \param if the statistics computation succeeds, this parameter stores the stat N percentile
    /// \param if the statistics computation succeeds, this parameter stores the stat 100-N percentile
    /// \param if the statistics computation succeeds, this parameter stores number of detected outliers
    /// \return true if the statistics computation succeeds, false otherwise
    ///
    bool GetLimitsAndParams(QList<double> datapoints,
                            bool removeOutliers,
                            OutlierRule outlierRule,
                            double factorN,
                            double& lowLimit,
                            double& highLimit,
                            double& mean,
                            double& sigma,
                            double& median,
                            double& Q1,
                            double& Q3,
                            double& percentN,
                            double& i00MinusNPercent,
                            int& numOutliers);

private:
    ///
    /// \brief Resets the stat to its initial post-construction state
    ///
    virtual void Reset();

    ///
    /// \brief Appends a datapoint to the stat's datapoints list
    /// \param the dataPoint to the stat
    ///
    void Append(GexDbPlugin_MonitorDataPoint& dataPoint);

    ///
    /// \brief Reset only the statistics of the stat, not its datapoints
    ///
    void ResetStats();

    ///
    /// \brief Recursively compute limits, identify and 'remove'(mark as) outliers using the given rule
    /// and sets the corresponding limits.
    /// The outliers will have the m_bIsOutlier set to true.
    /// \param set to true to activate outlier removal during statistics computation
    /// \param specifies the rule to use while computing the limits and removing the outliers
    /// \param specifies the N factore to use while computing the limits and removing the outliers
    /// \param if the statistics computation succeeds, this parameter stores the stat low limit
    /// \param if the statistics computation succeeds, this parameter stores the stat high limit
    /// \param set to true if the statistics must be recomputed whatever the reason
    /// \return true if the statistics computation succeeds, false otherwise
    ///
    bool ProcessStat(
            bool removeOutliers,
            OutlierRule outlierRule,
            double factorN,
            double& lowLimit,
            double& highLimit,
            bool forceRecompute = false);

    ///
    /// \brief Return the current number of datapoints marked as outlier
    /// \return number of outliers
    ///
    unsigned int GetCurrentNumberOfOutliers();

    ///
    /// \brief Holds the datapoints used for the statiticss and limits computation
    ///
    QList<GexDbPlugin_MonitorDataPoint>         dataPointList;

    ///
    /// \brief Holds references to the datapoints, ordered by their value (for quartile computation)
    ///
    QMap<double, GexDbPlugin_MonitorDataPoint*> dataPointMapByValue;

    ///
    /// \brief Holds references to the datapoints which are not outliers, ordered by their outlier-filtered value (for quartile computation)
    ///
    QMap<double, GexDbPlugin_MonitorDataPoint*> dataPointMapNoOutliersByFilteredValue;

    ///
    /// \brief lists the lots and wafers (or sublots) of which datapoints have already been inserted
    ///
    QSet<QString>                               lotsWafersAlreadyInserted;

    ////////////////
    // Statistics //
    ////////////////

    // Each statistic is represented twice:
    // - once computed for all the datapoints provided
    // - once computed for all the datapoints that are not outliers

    // Quartiles
    unsigned int					mQuartile1_AllItems_Index;
    double							mQuartile1_AllItems_Value;
    unsigned int					mQuartile1_Filtered_Index;
    double							mQuartile1_Filtered_Value;

    unsigned int					mMedian_AllItems_Index;
    double							mMedian_AllItems_Value;
    unsigned int					mMedian_Filtered_Index;
    double							mMedian_Filtered_Value;

    unsigned int					mQuartile3_AllItems_Index;
    double							mQuartile3_AllItems_Value;
    unsigned int					mQuartile3_Filtered_Index;
    double							mQuartile3_Filtered_Value;

    // Percentiles
    unsigned int					mPercentileN_AllItems_Index;
    double                          mPercentileN_AllItems_Value;
    unsigned int					mPercentileN_Filtered_Index;
    double                          mPercentileN_Filtered_Value;

    unsigned int					mPercentile100lessN_AllItems_Index;
    double                          mPercentile100lessN_AllItems_Value;
    unsigned int					mPercentile100lessN_Filtered_Index;
    double                          mPercentile100lessN_Filtered_Value;

    // Mean
    double							mAvg_AllItems;
    double							mAvg_Filtered;

    // Sigma
    double							mStd_AllItems;
    double							mStd_Filtered;

    // Min
    double							mMin_AllItems;
    double							mMin_Filtered;

    // Max
    double							mMax_AllItems;
    double							mMax_Filtered;

    ///
    /// \brief Computes the statistics with regard of the provided percentile
    /// \param the percentile to use
    /// \return true is suceeded, false otherwise
    ///
    bool ComputeStats(double percentileN);

    double GetC4Factor(int numberOfDataPoints);

    ///
    /// \brief Gets the limits of the current stat, according to the provided rule and factor
    /// \param specifies the rule to use while computing the limits
    /// \param specifies the N factore to use while computing the limits
    /// \param if the limits computation succeeds, this parameter stores the low limit
    /// \param if the limits computation succeeds, this parameter stores the high limit
    /// \return true if succeeded, false otherwise
    ///
    bool GetLimits(OutlierRule outlierRule, double factorN, double& lowLimit, double& highLimit);

    ///
    /// \brief Gets the index of the provided percentile in a list of the specified size
    /// \param the size of the list
    /// \param the percentile index to find
    /// \return percentile's index
    ///
    unsigned int GetPercentileIndex(unsigned int listSize, double percentile);

    ///
    /// \brief Stores whether the statistics have been computed or not since the last statistics reset
    ///
    bool baseStatsComputed;
};

#endif // GEXDB_PLUGIN_MONITORSTAT_H
