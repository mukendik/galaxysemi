#include "engine.h"
#include "report_options.h"
#include "gex_constants.h"
#include "gex_version.h"
#include "classes.h"
#include "interactive_charts.h"	// for CGexSingleChart
#include "libgexoc.h"
#include <gqtl_log.h>
#include "script_wizard.h"
#include "QDir"
#include "gexperformancecounter.h"
#include "browser_dialog.h"
#include "charac_line_chart_template.h"
#include "charac_box_whisker_template.h"
#include "product_info.h"

#ifdef _WIN32
    #include <windows.h>
    // #define used to readback computer paper size
    #ifndef LOCALE_IPAPERSIZE
        #define LOCALE_IPAPERSIZE             0x0000100A
    #endif
#endif

extern CReportOptions	ReportOptions;		// Holds options (report_build.h)

int CReportOptions::s_nInstances=0;

// in script_wizard.h
extern void ConvertToScriptString(QString &strFile);

///////////////////////////////////////////////////////////
// Constructor: Report Settings
///////////////////////////////////////////////////////////
CReportOptions::CReportOptions()
    : QObject(),
      mPlotQQLine(true),
      mLineChartTemplate(NULL),
      mBoxWhiskerTemplate(NULL)
{
    s_nInstances++;

    // Data type allowed to process. 0=Any Data file (Examinator), 1= Crdence only, etc...4=ExaminatorDB
    pGexRangeList			= NULL;		// List of part/bins to process
    pGexStatsRangeList		= NULL;		// List of Test statistics
    pGexWafermapRangeList	= NULL;		// List of Test/Bin wafermap
    pGexHistoRangeList		= NULL;		// List of Test Histograms
    pGexAdvancedRangeList	= NULL;		// List of tests in Advanced reports (Adv Histo+ Datalog)

    bSpeedCollectSamples	= true;
    mComputeAdvancedTest    = true;


    /*
     // impossible because the AppDir is unknown at the very beginning of Gex
    if (!CreateDefaultRnRRulesFile())
        GSLOG(SYSLOG_SEV_WARNING, " error : cant copy default RnR file !");
    */

    // Do not call the Reset() member from constructor, as:
    // o We have a global CReportOptions object
    // o Reset() might require stuff that is not initialized when the global object is created
    // Reset() member is now called from GexMainwindow::OnStart()
    //Reset(true);	// Reset to default
}

CReportOptions::CReportOptions(const CReportOptions& other)
    : QObject(), mLineChartTemplate(NULL), mBoxWhiskerTemplate(NULL)
{
    s_nInstances++;

    pGexRangeList			= NULL;		// List of part/bins to process
    pGexStatsRangeList		= NULL;		// List of Test statistics
    pGexWafermapRangeList	= NULL;		// List of Test/Bin wafermap
    pGexHistoRangeList		= NULL;		// List of Test Histograms
    pGexAdvancedRangeList	= NULL;		// List of tests in Advanced reports (Adv Histo+ Datalog)

    *this = other;
}

CReportOptions::~CReportOptions()
{
    // An instance of this object will be deleted after quiting main. Lets remove the GSLOG for the moment.
    //GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("still %1 instances of CReportOptions").arg( s_nInstances));
    clearMemory();
    s_nInstances--;

}

CReportOptions& CReportOptions::operator=(const CReportOptions& other)
                            {
    if (this != &other)
    {
        clearMemory();

        // Copy options map
        mOptionsHandler.SetOptionsMap(other.mOptionsHandler.GetOptionsMap());

        //
        iGroups						= other.iGroups;
        iFiles						= other.iFiles;

        if (other.pGexRangeList)
            pGexRangeList = new GS::QtLib::Range(*(other.pGexRangeList));

        //
        iHtmlSectionsToSkip			= other.iHtmlSectionsToSkip;
        strReportDirectory			= other.strReportDirectory;
        strReportTitle				= other.strReportTitle;
        strReportNormalizedTitle	= other.strReportNormalizedTitle;

        // Dataset type: Query or files.
        bQueryDataset				= other.bQueryDataset;

        // Speed Optimization
        mComputeAdvancedTest		= other.mComputeAdvancedTest;
        bSpeedCollectSamples		= other.bSpeedCollectSamples;
        bSpeedUseSummaryDB			= other.bSpeedUseSummaryDB;

        // Test statistics section
        iStatsType					= other.iStatsType;
        lfStatsLimit				= other.lfStatsLimit;
        bStatsTableSuggestLim		= other.bStatsTableSuggestLim;

        if (other.pGexStatsRangeList)
            pGexStatsRangeList		= new CGexTestRange(*(other.pGexStatsRangeList));

        iWafermapType				= other.iWafermapType;
        iWafermapTests				= other.iWafermapTests;
        iWafermapNumberOfTests		= other.iWafermapNumberOfTests;

        if (other.pGexWafermapRangeList)
            pGexWafermapRangeList	= new CGexTestRange(*(other.pGexWafermapRangeList));

        // Standard Histogram section
        bForceAdvancedHisto			= other.bForceAdvancedHisto;
        iHistogramType				= other.iHistogramType;
        iHistogramTests				= other.iHistogramTests;
        iHistogramNumberOfTests		= other.iHistogramNumberOfTests;

        if (other.pGexHistoRangeList)
            pGexHistoRangeList		= new CGexTestRange(*(other.pGexHistoRangeList));

        // For all Advanced+datalog charts:
        iAdvancedReport				= other.iAdvancedReport;
        iAdvancedReportSettings		= other.iAdvancedReportSettings;
        bAdvancedUsingSoftBin		= other.bAdvancedUsingSoftBin;
        strAdvancedTestList			= other.strAdvancedTestList;
        lAdvancedHtmlPages			= other.lAdvancedHtmlPages;
        lAdvancedHtmlLinesInPage	= other.lAdvancedHtmlLinesInPage;

        if (other.pGexAdvancedRangeList)
            pGexAdvancedRangeList	= new CGexTestRange(*(other.pGexAdvancedRangeList));
//		else
//			pGexAdvancedRangeList	= NULL;

        // Advanced Histogram
        bHistoMarkerMin				= other.bHistoMarkerMin;
        bHistoMarkerMax				= other.bHistoMarkerMax;

        // Advanced Trend
        bTrendMarkerMin				= other.bTrendMarkerMin;
        bTrendMarkerMax				= other.bTrendMarkerMax;

        // Advanced Correlation
        iScatterChartType			= other.iScatterChartType;
        bScatterMarkerMin			= other.bScatterMarkerMin;
        bScatterMarkerMax			= other.bScatterMarkerMax;

        // Advanced Probability plot
        iProbPlotChartType			= other.iProbPlotChartType;
        bProbPlotMarkerMin			= other.bProbPlotMarkerMin;
        bProbPlotMarkerMax			= other.bProbPlotMarkerMax;

        // Advanced Box plot
        iBoxPlotExChartType			= other.iBoxPlotExChartType;
        bBoxPlotExMarkerMin			= other.bBoxPlotExMarkerMin;
        bBoxPlotExMarkerMax			= other.bBoxPlotExMarkerMax;

        // Production Yield reports
        strProdReportTitle			= other.strProdReportTitle;
        uiYieldAxis					= other.uiYieldAxis;
        uiVolumeAxis				= other.uiVolumeAxis;
        uiVolumeHL					= other.uiVolumeHL;
        uiYieldMarker				= other.uiYieldMarker;

        // MyReport Template file
        strTemplateFile				= other.strTemplateFile;

        // Report Center Template
        strReportCenterTemplateFile	= other.strReportCenterTemplateFile;

        // For all Interactive Drill charts:
        iDrillReport				= other.iDrillReport;

        // Background color for all charts
        cBkgColor					= other.cBkgColor;
        bPlotLegend					= other.bPlotLegend;
        mPlotQQLine                 = other.mPlotQQLine;
        mTextRotation               = other.mTextRotation;

        // List of Layers style definitions (colors, line width, etc...)
        QList<CGexSingleChart*>::const_iterator itBegin	= other.pLayersStyleList.begin();
        QList<CGexSingleChart*>::const_iterator itEnd	= other.pLayersStyleList.end();

        while (itBegin != itEnd)
        {
            pLayersStyleList.append(new CGexSingleChart(*(*itBegin)));  // Comment : clearMemory(); at the beginning of the method assume that pLayersStyleList is empty.
            ++itBegin;
        }

        // List of Soft & Hard bins, and color assigned to them.
        softBinColorList			= other.softBinColorList;
        hardBinColorList			= other.hardBinColorList;
        siteColorList             = other.siteColorList;
        bUseCustomBinColors			= other.bUseCustomBinColors;

        if (other.mLineChartTemplate)
            mLineChartTemplate = new GS::Gex::CharacLineChartTemplate(*other.mLineChartTemplate);

        if (other.mBoxWhiskerTemplate)
            mBoxWhiskerTemplate = new GS::Gex::CharacBoxWhiskerTemplate(*other.mBoxWhiskerTemplate);


        mTotalBars =  GetOption("adv_histogram","total_bars").toInt();
        mYScale = 0;
        if( GetOption("adv_histogram","y_axis").toString().compare("hits", Qt::CaseInsensitive) == 0)
           mYScale = 1;

       // mTotalBars                 = other.mTotalBars;
        //mYScale                    = other.mYScale;
        mCustom                    = other.mCustom;
        mTotalBarsCustom           = mTotalBars;


    }

    return *this;
}

GS::Gex::OptionsHandler &CReportOptions::GetOptionsHandler()
{
    return mOptionsHandler;
}

const QString &CReportOptions::GetPATTraceabilityFile() const
{
    return mPATTraceabilityFile;
}

bool CReportOptions::addReportUnit(QString key, QVector<QString> params)
{
    if (mReportsMap.contains(key) || key=="")
        return false;
    mReportsMap.insert(key, params);
    return true;
}

bool CReportOptions::CreateDefaultRnRRulesFile()
{
    QDir dir;
    if (!dir.mkpath(GS::Gex::Engine::GetInstance().Get("UserFolder").toString()
                    +GEX_DEFAULT_DIR))
        return false;

    QString strFilePath = GS::Gex::Engine::GetInstance().Get("UserFolder").toString()
            +QDir::separator() + GEX_DEFAULT_DIR + QDir::separator()+".galaxy_rr.txt";

    if(QFile::exists(strFilePath))
        return true;

    QString src=GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString()
            +QDir::separator()+"samples"+QDir::separator()+".galaxy_rr.txt";
    src=QDir::cleanPath(src);
    if (!QFile::exists(src))
    {
        GSLOG(SYSLOG_SEV_WARNING,QString("default RnR rules file model '%1' does not exist !").arg( src).toLatin1().constData());
        return false;
    }

    QString d=GS::Gex::Engine::GetInstance().Get("UserFolder").toString()+QDir::separator()+GEX_DEFAULT_DIR+"/.galaxy_rr.txt";
    d=QDir::cleanPath(d);

    if (!QFile::copy(src,d))
    {
        GSLOG(SYSLOG_SEV_WARNING,"default RnR rules file copy failed !");
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////
// Exports Options related to interactive pages (markers, style, etc...)
///////////////////////////////////////////////////////////
void	CReportOptions::keepInteractive(CReportOptions *ptTo)
{

    bHistoMarkerMin			= ptTo->bHistoMarkerMin;
    bHistoMarkerMax			= ptTo->bHistoMarkerMax;
    SetOption(QString("adv_histogram"), QString ("marker"), ptTo->GetOption(QString("adv_histogram"),QString ("marker")).toString());


    bProbPlotMarkerMin		= ptTo->bProbPlotMarkerMin;
    bProbPlotMarkerMax		= ptTo->bProbPlotMarkerMax;
    SetOption(QString("adv_probabilityplot"), QString ("marker"), ptTo->GetOption(QString("adv_probabilityplot"),QString ("marker")).toString());

    bScatterMarkerMin		= ptTo->bScatterMarkerMin;
    bScatterMarkerMax		= ptTo->bScatterMarkerMax;
    SetOption(QString("adv_correlation"), QString ("marker"), ptTo->GetOption(QString("adv_correlation"),QString ("marker")).toString());


    bTrendMarkerMin			= ptTo->bTrendMarkerMin;
    bTrendMarkerMax			= ptTo->bTrendMarkerMax;
    SetOption(QString("adv_trend"), QString ("marker"), ptTo->GetOption(QString("adv_trend"),QString ("marker")).toString());

    bBoxPlotExMarkerMax		= ptTo->bBoxPlotExMarkerMax;
    bBoxPlotExMarkerMin		= ptTo->bBoxPlotExMarkerMin;
    SetOption(QString("adv_boxplot_ex"), QString ("marker"), ptTo->GetOption(QString("adv_boxplot_ex"),QString ("marker")).toString());

    iAdvancedReportSettings		= ptTo->iAdvancedReportSettings;
    bAdvancedUsingSoftBin		= ptTo->bAdvancedUsingSoftBin;
    bPlotLegend					= ptTo->bPlotLegend;
    mPlotQQLine = ptTo->mPlotQQLine;

    SetOption("adv_histogram","y_axis", ptTo->GetOption("adv_histogram","y_axis").toString() );

    SetOption("adv_histogram","total_bars", ptTo->GetOption("adv_histogram","total_bars").toString());

    SetOption("adv_probabilityplot","y_axis", ptTo->GetOption("adv_probabilityplot","y_axis").toString());
    SetOption("adv_boxplot_ex","orientation", ptTo->GetOption("adv_boxplot_ex","orientation").toString() );
}

QVariant CReportOptions::GetOption(const QString section,const QString option_name)
{
    GEX_BENCHMARK_METHOD(QString("%1::%2").arg(section).arg(option_name))

    return mOptionsHandler.GetOptionsMap().GetOption(section, option_name);
}

////////////////////////////////////////////////////////////////////////////////////////
// Return the Local Databases Folder
// If 'default', use the <GalaxySemi>/databases/local folder
//		and display a debug message
////////////////////////////////////////////////////////////////////////////////////////
QString CReportOptions::GetLocalDatabaseFolder()
{
    QString strDatabaseFolder = GetOption("databases","local_path").toString();
    if (strDatabaseFolder == "default" || strDatabaseFolder == "")
    {
        // Use the default <GalaxySemi>/databases one !
        strDatabaseFolder = QDir::cleanPath(QDir::homePath() + QString(GEX_DEFAULT_DIR) +
                                            QString(GEX_DATABASE_FOLDER) + "/local/");
        GSLOG(SYSLOG_SEV_INFORMATIONAL,
              QString("Get Local Database Folder ('databases','local_path') = 'default': will use %1 ")
              .arg(strDatabaseFolder)
              .toLatin1().constData());
    }

    return strDatabaseFolder;
}

////////////////////////////////////////////////////////////////////////////////////////
// Return the Server Databases Folder
// If 'default', use the <GalaxySemi>/databases/server folder
//		and display a debug message
////////////////////////////////////////////////////////////////////////////////////////
QString CReportOptions::GetServerDatabaseFolder(bool bUseLocalDatabaseFolderIfNotDefined)
{
    QString strDatabaseFolder = GetOption("databases","server_path").toString();
    if(strDatabaseFolder == "default" || strDatabaseFolder == "")
    {
        // Use the default <GalaxySemi>/databases one !
        if(bUseLocalDatabaseFolderIfNotDefined)
            strDatabaseFolder = GetLocalDatabaseFolder();
        else
        {
            strDatabaseFolder = QDir::cleanPath(QDir::homePath() + QString(GEX_DEFAULT_DIR)
                                                + QString(GEX_DATABASE_FOLDER) + "/server/");

            GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" 'databases' 'server_path' = 'default': using %1 ").arg( strDatabaseFolder).toLatin1().constData());
        }
    }

    return strDatabaseFolder;
}

bool CReportOptions::SetOption(const QString section, const QString option, const QString value)
{
    return mOptionsHandler.SetOption(section, option, value);
}

bool CReportOptions::SetSpecificFlagOption(const QString strSection, const QString strField, const QString strValue, const bool bToSetOn)
{
    return mOptionsHandler.SetSpecificFlagOption(strSection, strField, strValue, bToSetOn);
}

void CReportOptions::SetPATTraceabilityFile(const QString &lPatTraceability)
{
    mPATTraceabilityFile = lPatTraceability;
}

bool CReportOptions::isReportOutputHtmlBased()
{
    QString of=GetOption("output", "format").toString().toUpper();
    if (of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE"||of=="ODT")
        return true;

    return false;
}

// Remove me when OptionsCenter ready.
int CReportOptions::GetTimeBeforeAutoClose(QString value)
{
    if (GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        return 0;

    int lServerGlobalTimeout = GS::Gex::Engine::GetInstance().GetServerGlobalTimeout();
    if(lServerGlobalTimeout > 0 )
    {
        return lServerGlobalTimeout*60;
    }

    QString ac(value);
    if(value.isEmpty())
        ac=GetOption("preferences", "auto_close").toString();

    if ( (ac.compare("15min", Qt::CaseInsensitive) ==0) )
        return 15*60;	// save information in seconds!
    else if ( (ac.compare("30min", Qt::CaseInsensitive) ==0) )
        return 30*60;
    else  if ( (ac.compare("1hour", Qt::CaseInsensitive) ==0) )
            return 60*60;
    else if ( (ac.compare("2hours", Qt::CaseInsensitive) ==0) )
        return 2*60*60;
    else if ( (ac.compare("4hours", Qt::CaseInsensitive) ==0) )
        return 4*60*60;
    else if ( (ac.compare("never", Qt::CaseInsensitive) ==0) )
        return 0;

    if (ac.endsWith("min"))
    {
        bool ok=false;
        int nm=ac.section("min",0,0).toInt(&ok);
        if (ok)
            return nm*60;
    }

    if (ac.endsWith("secs"))
    {
        bool ok=false;
        int ns=ac.section("secs",0,0).toInt(&ok);
        if (ok)
            return ns;
    }

    return 0;
}

// Write bin colors definition
bool CReportOptions::WriteBinColorsToFile(FILE* hFile)
{
    if (hFile)
    {
        int	iValue	= (bUseCustomBinColors) ? 1 : 0;
        int	iR,iG,iB;

        /* case 4242 by HTH
        fprintf(hFile,"  gexOptions('bin_style','custom_colors','%d');\n",iValue);
        fprintf(hFile,"  gexOptions('bin_style','clear','');\n");	// Empty current list.
        */
        fprintf(hFile,"  gexBinStyle('custom_colors','%d');\n",iValue);
        fprintf(hFile,"  gexBinStyle('clear','');\n");	// Empty current list.

        // SOFTWARE BINNING
        QString strString;

        QList<CBinColor>::iterator itBegin = softBinColorList.begin();
        QList<CBinColor>::iterator itEnd	= softBinColorList.end();

        while(itBegin != itEnd)
        {
            strString = (*itBegin).cBinRange->GetRangeList();
            /* case 4242 by HTH
            fprintf(hFile,"  gexOptions('bin_style','bin_list','%s');\n",strString.toLatin1().constData());
            */
            fprintf(hFile,"  gexBinStyle('bin_list','%s');\n",strString.toLatin1().constData());

            iR = (*itBegin).cBinColor.red();
            iG = (*itBegin).cBinColor.green();
            iB = (*itBegin).cBinColor.blue();
            /* case 4242 by HTH
            fprintf(hFile,"  gexOptions('bin_style','bin_color','%d %d %d');\n",iR,iG,iB);
            fprintf(hFile,"  gexOptions('bin_style','add_bin','soft_bin');\n");
            */
            fprintf(hFile,"  gexBinStyle('bin_color','%d %d %d');\n",iR,iG,iB);
            fprintf(hFile,"  gexBinStyle('add_bin','soft_bin');\n");

            // Move to next BinColor object
            itBegin++;
        };

        // HARDWARE BINNING
        itBegin = hardBinColorList.begin();
        itEnd	= hardBinColorList.end();

        while(itBegin != itEnd)
        {
            strString = (*itBegin).cBinRange->GetRangeList();
            /* case 4242 by HTH
            fprintf(hFile,"  gexOptions('bin_style','bin_list','%s');\n",strString.toLatin1().constData());
            */
            fprintf(hFile,"  gexBinStyle('bin_list','%s');\n",strString.toLatin1().constData());

            iR = (*itBegin).cBinColor.red();
            iG = (*itBegin).cBinColor.green();
            iB = (*itBegin).cBinColor.blue();
            /* case 4242 by HTH
            fprintf(hFile,"  gexOptions('bin_style','bin_color','%d %d %d');\n",iR,iG,iB);
            fprintf(hFile,"  gexOptions('bin_style','add_bin','hard_bin');\n");
            */
            fprintf(hFile,"  gexBinStyle('bin_color','%d %d %d');\n",iR,iG,iB);
            fprintf(hFile,"  gexBinStyle('add_bin','hard_bin');\n");

            // Move to next BinColor object
            itBegin++;
        };

        // SITE STYLE
        itBegin = siteColorList.begin();
        itEnd	= siteColorList.end();

        while(itBegin != itEnd)
        {
            strString = (*itBegin).cBinRange->GetRangeList();
            fprintf(hFile,"  gexSiteStyle('site_list','%s');\n",strString.toLatin1().constData());

            iR = (*itBegin).cBinColor.red();
            iG = (*itBegin).cBinColor.green();
            iB = (*itBegin).cBinColor.blue();

            fprintf(hFile,"  gexSiteStyle('site_color','%d %d %d');\n",iR,iG,iB);
            fprintf(hFile,"  gexSiteStyle('add_color','hard_bin');\n");

            // Move to next BinColor object
            itBegin++;
        };

        return true;
    }

    return false;
}

// Write chart style definition
bool CReportOptions::WriteChartStyleToFile(FILE* hFile)
{
    if (hFile)
    {
        /////////////////////////////////////////////
        // Interactive style.
        /////////////////////////////////////////////
        unsigned	iLayerID=0;
        int	iValue;
        int	iR,iG,iB;

        // Chart background color
        iR = cBkgColor.red();
        iG = cBkgColor.green();
        iB = cBkgColor.blue();
        /* case 4241 by HTH
        fprintf(hFile,"  gexOptions('chart_style','bkg_color','%d %d %d');\n",iR,iG,iB);
        */
        fprintf(hFile,"  gexChartStyle('bkg_color','%d %d %d');\n",iR,iG,iB);

        // Plot curve's name on chart.?
        iValue = (ReportOptions.bPlotLegend) ? 1 : 0;
        /* case 4241 by HTH
        fprintf(hFile,"  gexOptions('chart_style','show_legend','%d');\n",iValue);
        */
        fprintf(hFile,"  gexChartStyle('show_legend','%d');\n",iValue);

        fprintf(hFile,"  gexChartStyle('text_rotation','%d');\n",ReportOptions.mTextRotation);

        // qqline
        iValue = (ReportOptions.mPlotQQLine) ? 1 : 0;
        fprintf(hFile,"  gexChartStyle('show_qqline','%d');\n", iValue);

        fprintf(hFile,"  gexChartStyle('total_bars','%d');\n",ReportOptions.mTotalBars);
        fprintf(hFile,"  gexChartStyle('indexY_scale','%d');\n",ReportOptions.mYScale);
        fprintf(hFile,"  gexChartStyle('custom','%d');\n",ReportOptions.mCustom);
        fprintf(hFile,"  gexChartStyle('total_bars_custom','%d');\n",ReportOptions.mTotalBarsCustom);


        // Drawing style for each layer
        QListIterator<CGexSingleChart*> lstIteratorChart(pLayersStyleList);
        CGexSingleChart	*				pLayerStyle  = NULL;

        lstIteratorChart.toFront();
        bool lIsMultilayer = (pLayersStyleList.count() > 1);

        // Limit export to 100 layers.
        while(lstIteratorChart.hasNext() && iLayerID < 100)
        {
            pLayerStyle = lstIteratorChart.next();

            // Save style info.
            iValue = (pLayerStyle->bBoxBars) ? 1 : 0;
            /* case 4241 by HTH
            fprintf(hFile,"  gexOptions('chart_style','box_bars','%d');\n",iValue);
            */
            fprintf(hFile,"  gexChartStyle('box_bars','%d');\n",iValue);

            iValue = (pLayerStyle->bBox3DBars) ? 1 : 0;
            /* case 4241 by HTH
            fprintf(hFile,"  gexOptions('chart_style','box_3d_bars','%d');\n",iValue);
            */
            fprintf(hFile,"  gexChartStyle('box_3d_bars','%d');\n",iValue);

            iValue = (pLayerStyle->mIsStacked) ? 1 : 0;
            /* case 4241 by HTH
            fprintf(hFile,"  gexOptions('chart_style','stack','%d');\n",iValue);
            */
            fprintf(hFile,"  gexChartStyle('stack','%d');\n",iValue);

            iValue = (pLayerStyle->bFittingCurve) ? 1 : 0;
            /* case 4241 by HTH
            fprintf(hFile,"  gexOptions('chart_style','fitting_curve','%d');\n",iValue);
            */
            fprintf(hFile,"  gexChartStyle('fitting_curve','%d');\n",iValue);

            iValue = (pLayerStyle->bBellCurve) ? 1 : 0;
            /* case 4241 by HTH
            fprintf(hFile,"  gexOptions('chart_style','bell_curve','%d');\n",iValue);
            */
            fprintf(hFile,"  gexChartStyle('bell_curve','%d');\n",iValue);

            iValue = (pLayerStyle->bLines) ? 1 : 0;
            /* case 4241 by HTH
            fprintf(hFile,"  gexOptions('chart_style','lines','%d');\n",iValue);
            */
            fprintf(hFile,"  gexChartStyle('lines','%d');\n",iValue);

            iValue = (pLayerStyle->bSpots) ? 1 : 0;
            /* case 4241 by HTH
            fprintf(hFile,"  gexOptions('chart_style','spots','%d');\n",iValue);
            */
            fprintf(hFile,"  gexChartStyle('spots','%d');\n",iValue);

            // Box-Plot Whisker type.
            switch(pLayerStyle->iWhiskerMode)
            {
            case GEX_WHISKER_RANGE:
                /* case 4241 by HTH
                fprintf(hFile,"  gexOptions('chart_style','box_whisker','range');\n");
                */
                fprintf(hFile,"  gexChartStyle('box_whisker','range');\n");
                break;

            case GEX_WHISKER_Q1Q3:
                /* case 4241 by HTH
                fprintf(hFile,"  gexOptions('chart_style','box_whisker','q1q3');\n");
                */
                fprintf(hFile,"  gexChartStyle('box_whisker','q1q3');\n");
                break;

            case GEX_WHISKER_IQR:
                /* case 4241 by HTH
                fprintf(hFile,"  gexOptions('chart_style','box_whisker','iqr');\n");
                */
                fprintf(hFile,"  gexChartStyle('box_whisker','iqr');\n");
                break;
            }

            /* case 4241 by HTH
            fprintf(hFile,"  gexOptions('chart_style','line_width','%d');\n",pLayerStyle->iLineWidth);
            fprintf(hFile,"  gexOptions('chart_style','line_style','%d');\n",pLayerStyle->iLineStyle);
            fprintf(hFile,"  gexOptions('chart_style','spot_style','%d');\n",pLayerStyle->iSpotStyle);
            */
            fprintf(hFile,"  gexChartStyle('line_width','%d');\n",pLayerStyle->iLineWidth);
            fprintf(hFile,"  gexChartStyle('line_style','%d');\n",pLayerStyle->iLineStyle);
            fprintf(hFile,"  gexChartStyle('spot_style','%d');\n",pLayerStyle->iSpotStyle);

            iR = pLayerStyle->cColor.red();
            iG = pLayerStyle->cColor.green();
            iB = pLayerStyle->cColor.blue();
            /* case 4241 by HTH
            fprintf(hFile,"  gexOptions('chart_style','rgb_color','%d %d %d');\n",iR,iG,iB);
            */
            fprintf(hFile,"  gexChartStyle('rgb_color','%d %d %d');\n",iR,iG,iB);

            // Markers line with & color
            iR = pLayerStyle->meanColor(lIsMultilayer).red();
            iG = pLayerStyle->meanColor(lIsMultilayer).green();
            iB = pLayerStyle->meanColor(lIsMultilayer).blue();
            /* case 4241 by HTH
            fprintf(hFile,"  gexOptions('chart_style','marker_mean','%d %d %d %d');\n",pLayerStyle->meanLineWidth(),iR,iG,iB);
            */
            fprintf(hFile,"  gexChartStyle('marker_mean','%d %d %d %d');\n",pLayerStyle->meanLineWidth(),iR,iG,iB);

            iR = pLayerStyle->medianColor(lIsMultilayer).red();
            iG = pLayerStyle->medianColor(lIsMultilayer).green();
            iB = pLayerStyle->medianColor(lIsMultilayer).blue();
            /* case 4241 by HTH
            fprintf(hFile,"  gexOptions('chart_style','marker_median','%d %d %d %d');\n",pLayerStyle->medianLineWidth(),iR,iG,iB);
            */
            fprintf(hFile,"  gexChartStyle('marker_median','%d %d %d %d');\n",pLayerStyle->medianLineWidth(),iR,iG,iB);

            iR = pLayerStyle->minColor(lIsMultilayer).red();
            iG = pLayerStyle->minColor(lIsMultilayer).green();
            iB = pLayerStyle->minColor(lIsMultilayer).blue();
            /* case 4241 by HTH
            fprintf(hFile,"  gexOptions('chart_style','marker_min','%d %d %d %d');\n",pLayerStyle->minLineWidth(),iR,iG,iB);
            */
            fprintf(hFile,"  gexChartStyle('marker_min','%d %d %d %d');\n",pLayerStyle->minLineWidth(),iR,iG,iB);

            iR = pLayerStyle->maxColor(lIsMultilayer).red();
            iG = pLayerStyle->maxColor(lIsMultilayer).green();
            iB = pLayerStyle->maxColor(lIsMultilayer).blue();
            /* case 4241 by HTH
            fprintf(hFile,"  gexOptions('chart_style','marker_max','%d %d %d %d');\n",pLayerStyle->maxLineWidth(),iR,iG,iB);
            */
            fprintf(hFile,"  gexChartStyle('marker_max','%d %d %d %d');\n",pLayerStyle->maxLineWidth(),iR,iG,iB);

            iR = pLayerStyle->limitsColor(lIsMultilayer).red();
            iG = pLayerStyle->limitsColor(lIsMultilayer).green();
            iB = pLayerStyle->limitsColor(lIsMultilayer).blue();
            /* case 4241 by HTH
            fprintf(hFile,"  gexOptions('chart_style','marker_limits','%d %d %d %d');\n",pLayerStyle->limitsLineWidth(),iR,iG,iB);
            */
            fprintf(hFile,"  gexChartStyle('marker_limits','%d %d %d %d');\n",pLayerStyle->limitsLineWidth(),iR,iG,iB);

            iR = pLayerStyle->specLimitsColor(lIsMultilayer).red();
            iG = pLayerStyle->specLimitsColor(lIsMultilayer).green();
            iB = pLayerStyle->specLimitsColor(lIsMultilayer).blue();
            fprintf(hFile,"  gexChartStyle('marker_speclimits','%d %d %d %d');\n",pLayerStyle->specLimitsLineWidth(),iR,iG,iB);


            iR = pLayerStyle->limitsColor(lIsMultilayer).red();
            iG = pLayerStyle->limitsColor(lIsMultilayer).green();
            iB = pLayerStyle->limitsColor(lIsMultilayer).blue();
            /* case 4241 by HTH
            fprintf(hFile,"  gexOptions('chart_style','marker_limits','%d %d %d %d');\n",pLayerStyle->limitsLineWidth(),iR,iG,iB);
            */
            fprintf(hFile,"  gexChartStyle('marker_rolling_limits','%d %d %d %d');\n",pLayerStyle->GetRollingLimitsLineWidth(),iR,iG,iB);

            iR = pLayerStyle->sigma2Color(lIsMultilayer).red();
            iG = pLayerStyle->sigma2Color(lIsMultilayer).green();
            iB = pLayerStyle->sigma2Color(lIsMultilayer).blue();
            /* case 4241 by HTH
            fprintf(hFile,"  gexOptions('chart_style','marker_2sigma','%d %d %d %d');\n",pLayerStyle->sigma2LineWidth(),iR,iG,iB);
            */
            fprintf(hFile,"  gexChartStyle('marker_2sigma','%d %d %d %d');\n",pLayerStyle->sigma2LineWidth(),iR,iG,iB);

            iR = pLayerStyle->sigma3Color(lIsMultilayer).red();
            iG = pLayerStyle->sigma3Color(lIsMultilayer).green();
            iB = pLayerStyle->sigma3Color(lIsMultilayer).blue();
            /* case 4241 by HTH
            fprintf(hFile,"  gexOptions('chart_style','marker_3sigma','%d %d %d %d');\n",pLayerStyle->sigma3LineWidth(),iR,iG,iB);
            */
            fprintf(hFile,"  gexChartStyle('marker_3sigma','%d %d %d %d');\n",pLayerStyle->sigma3LineWidth(),iR,iG,iB);

            iR = pLayerStyle->sigma6Color(lIsMultilayer).red();
            iG = pLayerStyle->sigma6Color(lIsMultilayer).green();
            iB = pLayerStyle->sigma6Color(lIsMultilayer).blue();
            /* case 4241 by HTH
            fprintf(hFile,"  gexOptions('chart_style','marker_6sigma','%d %d %d %d');\n",pLayerStyle->sigma6LineWidth(),iR,iG,iB);
            */
            fprintf(hFile,"  gexChartStyle('marker_6sigma','%d %d %d %d');\n",pLayerStyle->sigma6LineWidth(),iR,iG,iB);

            iR = pLayerStyle->sigma12Color(lIsMultilayer).red();
            iG = pLayerStyle->sigma12Color(lIsMultilayer).green();
            iB = pLayerStyle->sigma12Color(lIsMultilayer).blue();
            /* case 4241 by HTH
            fprintf(hFile,"  gexOptions('chart_style','marker_12sigma','%d %d %d %d');\n",pLayerStyle->sigma12LineWidth(),iR,iG,iB);
            */
            fprintf(hFile,"  gexChartStyle('marker_12sigma','%d %d %d %d');\n",pLayerStyle->sigma12LineWidth(),iR,iG,iB);

            iR = pLayerStyle->quartileQ1Color(lIsMultilayer).red();
            iG = pLayerStyle->quartileQ1Color(lIsMultilayer).green();
            iB = pLayerStyle->quartileQ1Color(lIsMultilayer).blue();
            fprintf(hFile,"  gexChartStyle('marker_QuartileQ1','%d %d %d %d');\n",pLayerStyle->quartileQ1LineWidth(),iR,iG,iB);

            iR = pLayerStyle->quartileQ3Color(lIsMultilayer).red();
            iG = pLayerStyle->quartileQ3Color(lIsMultilayer).green();
            iB = pLayerStyle->quartileQ3Color(lIsMultilayer).blue();
            fprintf(hFile,"  gexChartStyle('marker_QuartileQ3','%d %d %d %d');\n",pLayerStyle->quartileQ3LineWidth(),iR,iG,iB);


            // Trigger layer style insertion
            /* case 4241 by HTH
            fprintf(hFile,"  gexOptions('chart_style','chart_layer','%d');\n",iLayerID);
            */
            fprintf(hFile,"  gexChartStyle('chart_layer','%d');\n",iLayerID);

            // Keep track of layer ID
            iLayerID++;
        };

        return true;
    }

    return false;
}

bool CReportOptions::WriteOptionSectionToFile(FILE *hFile)
{
    if(!hFile)
        return false;

    bool bWriteRslt=true;

    // Writes 'Favorite Scripts' section
    fprintf(hFile,"//////////////////////////////////////////\n");
    fprintf(hFile,"// Setup the GEX 'Options' section\n");
    fprintf(hFile,"//////////////////////////////////////////\n");
    fprintf(hFile,"SetOptions()\n");
    fprintf(hFile,"{\n");
    fprintf(hFile,"\n");
    fprintf(hFile,"  // Sets all data analysis global options\n");
    fprintf(hFile,"  gexResetOptions('clear');\n\n");	// csl v2

    fprintf(hFile,"  gexCslVersion('%1.2f');\n", GEX_MAX_CSL_VERSION);	// ReportOptions.WriteOptionsToFile options are latest version of the csl format.
    bWriteRslt = GS::Gex::Engine::GetInstance().GetOptionsHandler().WriteOptionsToFile(hFile);
    if(!bWriteRslt)
    {
        GSLOG(SYSLOG_SEV_NOTICE, "Can't write options !! ");
        GEX_ASSERT(false);
    }

    ////////////////////////////////
    // Options to up date

    /////////////////////////////////////////////
    // Application preferences.
    /////////////////////////////////////////////

    /////////////////////////////////////////////
    // Interactive style.
    /////////////////////////////////////////////
    if (WriteChartStyleToFile(hFile) == false)
    {
        GSLOG(SYSLOG_SEV_NOTICE, "Can't write interactive chart style !! ");
        GEX_ASSERT(false);
    }

    /////////////////////////////////////////////
    // Custom Binning colors.
    /////////////////////////////////////////////
    if (WriteBinColorsToFile(hFile) == false)
    {
        GSLOG(SYSLOG_SEV_NOTICE, "Can't write bin colors!! ");
        GEX_ASSERT(false);
    }

    /////////////////////////////////////////////
    // Global Info Settings
    /////////////////////////////////////////////

    // Message to console saying 'success loading options'
    fprintf(hFile,"\n\n  sysLog('* Quantix Examinator Options loaded! *');\n");


    // Close function
    fprintf(hFile,"}\n");
    fprintf(hFile,"\n");

    return bWriteRslt;
}

/////////////////////////////////////////////////////////
//      Internal methods
/////////////////////////////////////////////////////////


void CReportOptions::clearMemory()
{
    ///////////////////////////////////////////////
    // delete pointers
    ///////////////////////////////////////////////
    CGexSingleChart* pCgscSingleChartPointer = NULL;
    while(pLayersStyleList.count()>0)
    {
        pCgscSingleChartPointer = pLayersStyleList.takeFirst();     // pLayersStyleList has to contain valid pointers
        delete pCgscSingleChartPointer;
    }

    if(pGexAdvancedRangeList)
        delete pGexAdvancedRangeList;
    pGexAdvancedRangeList = NULL;

    if(pGexHistoRangeList)
        delete pGexHistoRangeList;
    pGexHistoRangeList = NULL;

    if(pGexWafermapRangeList)
        delete pGexWafermapRangeList;
    pGexWafermapRangeList = NULL;

    if(pGexStatsRangeList)
        delete pGexStatsRangeList;
    pGexStatsRangeList = NULL;

    if(pGexRangeList)
        delete pGexRangeList;
    pGexRangeList = NULL;

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
}


QString CReportOptions::GetOptionType(const QString section,const QString option_name)
{
    return mOptionsHandler.GetOptionsTypeDefinition().getOptionType(section, option_name);
}

void CReportOptions::setAdvancedReport(int iReport){
    iAdvancedReport = iReport;
}
int CReportOptions::getAdvancedReport() const {
    return iAdvancedReport;
}

void CReportOptions::setAdvancedReportSettings(int iReportSettings ){
    iAdvancedReportSettings = iReportSettings;
}
int CReportOptions::getAdvancedReportSettings() const {
    return iAdvancedReportSettings;
}
