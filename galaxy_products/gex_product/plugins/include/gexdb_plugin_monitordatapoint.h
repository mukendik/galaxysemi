#ifndef GEXDB_PLUGIN_MONITORDATAPOINT_H
#define GEXDB_PLUGIN_MONITORDATAPOINT_H

#include <QString>

//
///
/// \brief Holds the data of 1 data point
/// In the case of SYA, this can be the yield of 1 binning on 1 wafer/sublot
/// In the case of SPM, this can be the value of mean, sigma or cpk on 1 wafer/sublot
///
class GexDbPlugin_MonitorDataPoint
{
public:
    GexDbPlugin_MonitorDataPoint();
    ~GexDbPlugin_MonitorDataPoint();

    void Reset();

public:
    // SYA/SPM common fields
    QString         m_strLotID;             // Lot ID
    QString         m_strWaferSublotID;     // Wafer or sublot ID
    double          m_value;               // Value of the datapoint
    double          m_filteredValue;       // Filtered value of the datapoint (ex: yield computed excluding the outliers)
    bool            m_bIsOutlier;           // Set to true if this data point is an outlier

    // SYA specific fields
    unsigned int    m_ExcludedParts;        // Total parts for excluded bins
    unsigned int    m_uiGrossDie;           // Gross Die
    unsigned int    m_uiTotalParts;         // Total parts
    unsigned int    m_uiBinParts;           // Parts with this binning
};

#endif // GEXDB_PLUGIN_MONITORDATAPOINT_H
