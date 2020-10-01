#include <QDir>
#include "report_options.h"
#include <gqtl_log.h>
#include "gex_constants.h"
#include "classes.h"
#include "interactive_charts.h"
#include "charac_line_chart_template.h"
#include "charac_box_whisker_template.h"

extern CReportOptions	ReportOptions;		// Holds options (report_build.h)

///////////////////////////////////////////////////////////
// Report Settings: reset to default or disabled.
///////////////////////////////////////////////////////////
void	CReportOptions::Reset(bool bDefault)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Reset %1").arg(bDefault?"default":"clear").toLatin1().data() );

    mOptionsHandler.Reset(bDefault);

    if(pGexRangeList!= NULL)
        delete 	pGexRangeList;						// List of Ranges to process (parts/bins,...)
    pGexRangeList = new GS::QtLib::Range(NULL);			// List of Ranges to process (parts/bins,...)

    iHtmlSectionsToSkip = 0;						// Default is to build ALL HTML sections.
    bSpeedUseSummaryDB	= false;
    iStatsType			= GEX_STATISTICS_ALL;		// default=ALL tests...

    if(pGexStatsRangeList != NULL)
    {
        delete pGexStatsRangeList;						// List of Ranges to process (parts/bins,...)
        pGexStatsRangeList = NULL;
    }

    if(pGexWafermapRangeList != NULL)
    {
        delete pGexWafermapRangeList;						// List of tests/bin to process
        pGexWafermapRangeList = NULL;
    }

    lfStatsLimit			= 0.0;		// Cp or Cpk limit if any defined
    iWafermapType			= GEX_WAFMAP_SOFTBIN;				// default=SOFTBIN
    if(pGexWafermapRangeList)
        delete pGexWafermapRangeList;
    pGexWafermapRangeList	= new CGexTestRange("1");	// Holds tests list or binning list (for wafermap report)

    // Standard Histogram section
    bForceAdvancedHisto		= false;							// Default: use standard histogram pre-built bars, not advanced histogram (unless needed: when comparing groups, etc...)
    iHistogramType			= GEX_HISTOGRAM_OVERDATA;
    iHistogramTests			= GEX_HISTOGRAM_ALL;
    if(pGexHistoRangeList != NULL)
    {
        delete pGexHistoRangeList;
        pGexHistoRangeList = NULL;
    }

    // For all Advanced+datalog charts:
    iAdvancedReport			= GEX_ADV_DISABLED;
    iAdvancedReportSettings = 0;								// options relative to the report (over limits,etc...all tests, single test,...)
    bAdvancedUsingSoftBin = true;
    if(pGexAdvancedRangeList!=NULL)
    {
        delete pGexAdvancedRangeList;
        pGexAdvancedRangeList = NULL;
    }

    uiYieldAxis			= GEX_ADV_PROD_YIELDAXIS_0_100;	// Yield axis viewport
    uiVolumeAxis		= GEX_ADV_PROD_VOLUMEAXIS_0_MAX;// Volume axis marker
    uiVolumeHL			= 0;							// Custom volume HL for volume axis viewport
    uiYieldMarker		= 0;							// Red yield marker

    // Holds list of Layers default styles (color, line style,...)
    while (!pLayersStyleList.isEmpty())
        delete pLayersStyleList.takeFirst();

    // Reset background charting color
    cBkgColor = QColor(235,235,235);

    // 'true' if overlay curve name on chart.
    bPlotLegend = false;

    // qqline
    mPlotQQLine = true;

    // Reset Binning colors to default...Need to manually delete because AutoDelet can't be used here (as list is sometimes shared)
    bUseCustomBinColors = false;	// Default: use internal Examinator colors, not user defined ones

    hardBinColorList.clear();
    softBinColorList.clear();
    siteColorList.clear();

    strTemplateFile.clear();
    strReportCenterTemplateFile.clear();

    if (mLineChartTemplate)
    {
        delete mLineChartTemplate;
        mLineChartTemplate = NULL;
    }

    if (mBoxWhiskerTemplate)
    {
        delete mBoxWhiskerTemplate;
        mBoxWhiskerTemplate = NULL;
    }

    // Clear report map
    mReportsMap.clear();
}
