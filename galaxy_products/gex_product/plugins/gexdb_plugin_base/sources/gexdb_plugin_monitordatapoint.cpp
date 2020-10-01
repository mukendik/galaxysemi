#include "gexdb_plugin_monitordatapoint.h"

GexDbPlugin_MonitorDataPoint::GexDbPlugin_MonitorDataPoint()
{
    Reset();
}

GexDbPlugin_MonitorDataPoint::~GexDbPlugin_MonitorDataPoint()
{
}

void GexDbPlugin_MonitorDataPoint::Reset()
{
    m_strLotID = "";
    m_strWaferSublotID = "";
    m_value = 0.0;
    m_filteredValue = 0.0;
    m_bIsOutlier = false;

    m_uiTotalParts = 0;
    m_ExcludedParts = 0;
    m_uiGrossDie = 0;
    m_uiBinParts = 0;
}

