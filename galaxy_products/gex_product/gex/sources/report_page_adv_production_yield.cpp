/////////////////////////////////////////////////////////////////////////////
// Creates HTML 'Scatter' Correlation page.
/////////////////////////////////////////////////////////////////////////////

#include "gex_shared.h"
#include "browser.h"
#include "report_build.h"
#include "report_options.h"
#include "interactive_charts.h"		// Layer classes, etc
#include "chart_director.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "gex_report.h"
#include "product_info.h"
#include "engine.h"
// Galaxy QT libraries
#include "gqtl_sysutils.h"
#include "cbinning.h"

// main.cpp
extern QString			formatHtmlImageFilename(const QString& strImageFileName);

/////////////////////////////////////////////////////////////////////////////
// Prepares the report section to be written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::PrepareSection_AdvProductionYield(BOOL /*bValidSection*/)
{
    QString of=m_pReportOptions->GetOption("output", "format").toString();
    // Creates the 'scatter' page & header
    if(of=="CSV")
    {
        // Generating .CSV report file.
        fprintf(hReportFile,
          "\n---- Advanced report ----\nNo chart generated in .CSV format. Only Raw data table. For chart rendering: switch to HTML report mode!\n\n");
    }
    else
        if (m_pReportOptions->isReportOutputHtmlBased()) //(of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
    {
        // Generating HTML report file.

        // Open advanced.htm file
        if(OpenFile_Advanced() != GS::StdLib::Stdf::NoError) return GS::StdLib::Stdf::ReportFile;

        // Most of functions keep writing to the 'hReportFile' file.
        hReportFile = hAdvancedReport;

        // Keep count of total HTML pages to create (used to compute the ProgreeGauge size)
        m_pReportOptions->lAdvancedHtmlPages = mTotalAdvancedPages = 1;

    }

    return GS::StdLib::Stdf::NoError;
}

/////////////////////////////////////////////////////////////////////////////
// closes the report section just written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::CloseSection_AdvProductionYield(void)
{
    QString of=m_pReportOptions->GetOption("output", "format").toString();
    if(of=="CSV")
    {
        // Generating .CSV report file: no specific task to perform when section written !
    }
    else if (m_pReportOptions->isReportOutputHtmlBased())
    {
        if(hAdvancedReport == NULL)
            return GS::StdLib::Stdf::NoError;	// Just in case file was closed on memory allocation error while collecting scatter data!
        // close page...if this section was created.
        if((IfHtmlSectionCreated(GEX_HTMLSECTION_ADVANCED) == true) && (of=="HTML"))
        {
            // Close index page (unless creating one flat HTML file with all sections (eg: when creating a Word document)
            fprintf(hReportFile,C_HTML_FOOTER, GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );	// Writes HTML footer Application name/web site string
            fprintf(hReportFile,"</body>\n");
            fprintf(hReportFile,"</html>\n");
            CloseReportFile();	// Close report file
        }
        // Write page break (ignored if not writing a flat HTML document)
        WritePageBreak();
    }
    return GS::StdLib::Stdf::NoError;
}

class clProductionBar
{
public:
    clProductionBar();			// Constructor

    QString	strLabelName;		// Holds logical label name to display on X axis.
    long	lTotalParts;		// Total parts tested
    long	lTotalGoodParts;	// Total good parts
    QString mTestingDate;       // Holds testings date
    QString mProduct;           // holds product
    QString mLot;               // holds lot
    QString mSubLotWaferID;     // holds sublot / wafer id
    QString mBinYield;          // holds yield
    QString  mYield;             // holds yield
};

clProductionBar::clProductionBar()
{
    lTotalParts = 0;
    lTotalGoodParts = 0;
}

typedef QMap<QString, clProductionBar*> ProductionBarMap;

/////////////////////////////////////////////////////////////////////////////
// Write Production report Yield page
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::WriteAdvProductionYield()
{
    // Title + bookmark (unless MyReport which provides its own custom title)
    if(m_pReportOptions->strTemplateFile.isEmpty())
        WriteHtmlSectionTitle(hReportFile,"all_advanced","Production Reports...");

    bool bRefuseReport=false;
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring() ||
            GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPRO() ||
            GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT() ||
            GS::LPPlugin::ProductInfo::getInstance()->isExaminatorTerProPlus())
    {
        bRefuseReport = false;
    }
    else
        bRefuseReport = true;


    if(bRefuseReport)
    {
    QString m=ReportOptions.GetOption("messages", "upgrade").toString();
    fprintf(hReportFile,"*ERROR* %s", m.toLatin1().data() );
        return;
    }

    // If ChartDirector disabled (Computer OS to old), quietly return.
    if(m_pChartDirector->isEnabled() == false)
    {
    fprintf(hReportFile,"*ERROR* Your Computer OS is too old to build this report. You need to upgrade your OS.\nPlease contact %s",GEX_EMAIL_SALES);
        return;
    }

    // One chart per group (in case comparing queries)
    QString								strString;
    QString								strKey;			// Used to compute key to production cell (histogram bar) to fill
    QString								strXaxisLegend;	// USed to store X axis legend
    QString								strImage;		// USed when building image name
    QString								strImagePath;	// USed when building image name & path
    QDateTime							cSubLotDate;	// Holds Time/Date of each sub-lot
    double								lfYield;		// USed for computing yield level.
    double								lfLowestYield;	// Lowest yield found in chart
    double								lfHighestYield;	// Highest yield found in chart
    double								lfLowestVolume;	// Lowest volume found in chart
    double								lfHighestVolume;// Highest volume found in chart
    Axis								*pAxis;			// Pointer to axis: used when customizing scale, title, font,...
    LineLayer							*pYieldLine;	// Pointer to yield line object
    BarLayer							*pBarLayer;		// Pointer to Volume bar chart
    ProductionBarMap 					mProductionCell;
    ProductionBarMap::Iterator			it;
    QMap<QString, CGexFileInGroup *>	pTableRowMap;
    int									iGroup =0;
    QString								strLabelName;
    int									iCellIndex;
    clProductionBar	*					pCell	= NULL;		// Pointer used when creating/retrieving a histogram cell structure.
    CGexGroupOfFiles *					pGroup	= NULL;
    CGexFileInGroup *					pFile	= NULL;
    QListIterator<CGexGroupOfFiles*>	itGroupsList(getGroupsList());
    QString of=m_pReportOptions->GetOption("output", "format").toString();

    QString strDatasetSorting = (m_pReportOptions->GetOption("dataprocessing", "sorting")).toString();
    bool bIsDatasetSortedByDate = (strDatasetSorting == QString("date"));


    while(itGroupsList.hasNext())
    {
        // Clear variables.
        lfLowestYield	= 100;
        lfHighestYield	= 0;
        lfLowestVolume	= C_INFINITE;
        lfHighestVolume = 0;

        pGroup	= itGroupsList.next();
        QListIterator<CGexFileInGroup*> itFilesList(pGroup->pFilesList);

        while (itFilesList.hasNext())
        {
            pFile = itFilesList.next();

            // Get Time/Date when this sub-lot was tested (start-time)
            cSubLotDate.setTime_t(pFile->getMirDatas().lStartT);

            // Check type of bars to build: sublot, lot, day, month,...
            switch(m_pReportOptions->getAdvancedReportSettings())
            {
                case GEX_ADV_PRODYIELD_SBLOT:
                    // Key: <Lot>-<Sublot>[GEX]<start_t> or <sublot_date><lot>-<sublot>[GEX]<start_t>
                    // Label: <Lot-Sublot>
                    strKey          = QString(pFile->getMirDatas().szLot) + "-";
                    strLabelName    = QString(pFile->getMirDatas().szLot) + "-";

                    if(*pFile->getWaferMapData().szWaferID && qstricmp(pFile->getMirDatas().szLot,pFile->getWaferMapData().szWaferID))
                    {
                        // Wafersort data
                        bool lIsNumeric = false;
                        int  lWaferId   = QString(pFile->getWaferMapData().szWaferID).toInt(&lIsNumeric);

                        if (lIsNumeric)
                        {
                            QTextStream lStreamKey(&strKey);

                            lStreamKey.setFieldWidth(10);
                            lStreamKey.setFieldAlignment(QTextStream::AlignRight);
                            lStreamKey.setPadChar('0');
                            lStreamKey << lWaferId;
                        }
                        else
                            strKey += pFile->getWaferMapData().szWaferID;

                        strLabelName += pFile->getWaferMapData().szWaferID;

                    }
                    else if(qstricmp(pFile->getMirDatas().szLot,pFile->getWaferMapData().szWaferID))
                    {
                        // Final data
                        strKey          += pFile->getMirDatas().szSubLot;
                        strLabelName    += pFile->getMirDatas().szSubLot;
                    }

                    // Add start-time in case same wafer tested multiple times. String to be removed from label when displayed!
                    strKey += "[Gex]" + QString::number(pFile->getMirDatas().lStartT);

                    // if sorting by date, ensure key starts with date info!
                    if(bIsDatasetSortedByDate)
                        strKey = cSubLotDate.toString("yyyy/MM/dd_hhmmss ") + strKey;
                    break;

                case GEX_ADV_PRODYIELD_LOT:
                    // Key name: <Lot>
                    strLabelName = strKey = pFile->getMirDatas().szLot;

                    // if sorting by date, ensure key starts with date info!
                    if(bIsDatasetSortedByDate)
                        strKey = cSubLotDate.toString("yyyy/MM/dd_hhmmss ") + strKey;
                    break;

                case GEX_ADV_PRODYIELD_GROUP:
                    // Key name: <GroupName>
                    strLabelName = strKey = pGroup->strGroupName;

                    // if sorting by date, ensure key starts with date info!
                    if(bIsDatasetSortedByDate)
                        strKey = cSubLotDate.toString("yyyy/MM/dd_hhmmss ") + strKey;
                    break;

                case GEX_ADV_PRODYIELD_DAY:
                    // Key name: <YYYY/MM/DD>
                    strLabelName = strLabelName = strKey = cSubLotDate.toString("yyyy/MM/dd");
                    break;

                case GEX_ADV_PRODYIELD_WEEK:
                    // Key name: <YYYY/WEEK#>
                    strKey.sprintf("%d/%02d",cSubLotDate.date().year(),cSubLotDate.date().weekNumber());
                    strLabelName = strKey;
                    break;

                case GEX_ADV_PRODYIELD_MONTH:
                    // Key name: <YYYY/MM>
                    strLabelName = strKey = cSubLotDate.toString("yyyy/MM");
                    break;
            }

            // Get cell entry, or create it if it doesn't exist
            if(!mProductionCell.contains(strKey))
            {
                // Entry doesn't exist, create it & add entry to QMap!
                pCell = new clProductionBar;
                mProductionCell[strKey] = pCell;
            }
            else
                pCell = mProductionCell[strKey];

            // Add pFile to map used to create rows in the HTML table in the same order
            pTableRowMap[strKey] = pFile;

            // Update label name
            pCell->strLabelName = strLabelName;
            // Update counts & data
            if(m_pReportOptions->getAdvancedReportSettings() == GEX_ADV_PRODYIELD_GROUP)//GCORE-119
            {
                CBinning *ptBinCell = pGroup->cMergedData.ptMergedSoftBinList;
                int lCumulPassBins = 0;
                int lCumulFailBins = 0;
                while(ptBinCell != NULL)
                {
                    if(toupper(ptBinCell->cPassFail) == 'P')
                        lCumulPassBins+= ptBinCell->ldTotalCount;
                    if(toupper(ptBinCell->cPassFail) == 'F')
                        lCumulFailBins+= ptBinCell->ldTotalCount;
                    ptBinCell = ptBinCell->ptNextBin;
                }
                pCell->lTotalGoodParts = lCumulPassBins;
                pCell->lTotalParts = lCumulPassBins + lCumulFailBins;
            }
            else
            {
                pCell->lTotalGoodParts += pFile->lAdvBinningTrendTotalMatch;	// total matching bin in sublot
                pCell->lTotalParts += pFile->lAdvBinningTrendTotal;		// Total parts in sublot
            }
            pCell->mTestingDate = TimeStringUTC(pFile->getMirDatas().lStartT);
            pCell->mProduct = pFile->getMirDatas().szPartType;
            pCell->mLot = pFile->getMirDatas().szLot;
            if(*pFile->getWaferMapData().szWaferID)
                pCell->mSubLotWaferID = pFile->getWaferMapData().szWaferID;	// Wafersort data
            else
                pCell->mSubLotWaferID = pFile->getMirDatas().szSubLot;			// Final data
        };

        // Delete any previous charting object
        m_pChartDirector->clearXYChart();

        int	iTotalCells = mProductionCell.count();

        char **Labels = new char*[iTotalCells];			// To hold histogram bar legend
        double *DataYield= new double[iTotalCells];		// To hold bar Yield line
        double *DataVolume= new double[iTotalCells];	// To hold Volume bar

        // Fill histogram memory buffers
        iCellIndex=0;
        for(it = mProductionCell.begin(); it != mProductionCell.end(); ++it)
        {
            pCell = *it;
            // Get cell label (key string)
            // Get logical X-axis label.
            Labels[iCellIndex] = new char[pCell->strLabelName.size()+1];
            strcpy(Labels[iCellIndex], pCell->strLabelName.toLatin1().constData());

            // Get data volume (bar height)
            DataVolume[iCellIndex] = pCell->lTotalParts;

            // Keep track of lowest/highest volume on chart.
            lfLowestVolume = gex_min(lfLowestVolume, pCell->lTotalParts);
            lfHighestVolume = gex_max(lfHighestVolume, pCell->lTotalParts);

            // Get Yield level
            if(pCell->lTotalParts)
                lfYield = (double)100.0*pCell->lTotalGoodParts/(double)pCell->lTotalParts;
            else
                lfYield = 0;	// Total part=0, would make division by 0!
            DataYield[iCellIndex] = lfYield;

            // Keep track of lowest/highest yield on chart.
            lfLowestYield = gex_min(lfLowestYield, lfYield);
            lfHighestYield = gex_max(lfHighestYield, lfYield);
            pCell->mYield = QString::number(lfYield, 'f', 1);

            // Keep track of cell index
            iCellIndex++;
        }

        // Create charting object
        m_pChartDirector->allocateXYChart(900,450,(m_pReportOptions->cBkgColor.rgb() & 0xFFFFFF));

        //Set the plot area Enable both horizontal and vertical grids by setting their colors to grey (0xc0c0c0)
        m_pChartDirector->xyChart()->setPlotArea(100, 40, 700, 315)->setGridColor(0xc0c0c0, 0xc0c0c0);

        //Add a legend box(top of the chart) using horizontal layout
        //and 8 pts Arial font Set the background and border color to Transparent.
        m_pChartDirector->xyChart()->addLegend(100, 5, false, "", 8)->setBackground(Chart::Transparent);

        //Add a title to the chart
        {
            QString qstrVolumeMarkerOptions = (m_pReportOptions->GetOption("adv_production_yield", "marker")).toString();
            QStringList qstrlVolumeMarkerOptionList = qstrVolumeMarkerOptions.split(QString("|"));
            if(qstrlVolumeMarkerOptionList.contains(QString("title")))
                m_pChartDirector->xyChart()->addTitle(m_pReportOptions->strProdReportTitle.toLatin1().constData());
        }

        //Add a title to the X axis
        switch(m_pReportOptions->getAdvancedReportSettings())
        {
            case GEX_ADV_PRODYIELD_SBLOT:
                strXaxisLegend = "LotID & Sub-Lot";
                break;

            case GEX_ADV_PRODYIELD_LOT:
                strXaxisLegend = "LotID";
                break;

            case GEX_ADV_PRODYIELD_GROUP:
                strXaxisLegend = "Group/Query";
                break;

            case GEX_ADV_PRODYIELD_DAY:
                strXaxisLegend = "Daily yield";
                break;

            case GEX_ADV_PRODYIELD_WEEK:
                strXaxisLegend = "Weekly yield";
                break;

            case GEX_ADV_PRODYIELD_MONTH:
                strXaxisLegend = "Monthly yield";
                break;
        }

        // Chart title
        m_pChartDirector->xyChart()->xAxis()->setTitle(strXaxisLegend.toLatin1().constData());


        // Top layer: Yield trend + associate it to Right Y-axis + define datapoint marker
        QString strYieldChartTypeOption = (m_pReportOptions->GetOption("adv_production_yield","chart_type")).toString();
        if( (strYieldChartTypeOption == "yield") || (strYieldChartTypeOption == "yield_volume") )
        {
            pYieldLine = m_pChartDirector->xyChart()->addLineLayer();

            pYieldLine->addDataSet(DoubleArray(DataYield, iTotalCells),0x0000c0,"Yield")->setDataSymbol(Chart::DiamondSymbol, 9);
            pYieldLine->setUseYAxis2();

            // Check if display the yield value for each data point
            QString qstrVolumeMarkerOptions = (m_pReportOptions->GetOption("adv_production_yield", "marker")).toString();
            QStringList qstrlVolumeMarkerOptionList = qstrVolumeMarkerOptions.split(QString("|"));
            if(qstrlVolumeMarkerOptionList.contains(QString("yield")))
                pYieldLine->setDataLabelFormat("{value|1}%");

            //Y right-Axis: Title & font & scale & scale format
            pAxis = m_pChartDirector->xyChart()->yAxis2();
            pAxis->setLabelStyle("arial.ttf");
            pAxis->setTitle("Yield (%)");
            lfLowestYield = (int) (lfLowestYield-1);	// Go 1% below the lowest bar so we can always see it!
            if(lfLowestYield <=0) lfLowestYield = 0;
            lfHighestYield = (int) (lfHighestYield+1);	// Go 1% above the highest bar so we can always see it!
            if(lfHighestYield >= 100) lfHighestYield = 100;
            switch(m_pReportOptions->uiYieldAxis)
            {
                case GEX_ADV_PROD_YIELDAXIS_0_MAX:
                    pAxis->setLinearScale(0,lfHighestYield);
                    break;

                case GEX_ADV_PROD_YIELDAXIS_MIN_100:
                    pAxis->setLinearScale(lfLowestYield,100);
                    break;

                case GEX_ADV_PROD_YIELDAXIS_MIN_MAX:
                    pAxis->setLinearScale(lfLowestYield,lfHighestYield);
                    break;

                case GEX_ADV_PROD_YIELDAXIS_0_100:
                default:
                    pAxis->setLinearScale(0,100);
                    break;
            }
            pAxis->setLabelFormat("{value|1} %");		// Display ercentage in format: XX.Y%
        }

        // 2nd layer: Yield Control Limit
        if(m_pReportOptions->uiYieldMarker > 0)
        {
            lfYield = m_pReportOptions->uiYieldMarker;	// Yield Limit
            strString = QString::number(lfYield, 'f', 1);
            strString += "%";
            m_pChartDirector->xyChart()->yAxis2()->addMark(lfYield,0xff0000,strString.toLatin1().constData());
        }

        // 3rd layer: Volume bar chart
        if( (strYieldChartTypeOption == "volume") || (strYieldChartTypeOption == "yield_volume") )
        {
            pBarLayer = m_pChartDirector->xyChart()->addBarLayer(DoubleArray(DataVolume, iTotalCells), 0xff00,"Volume");
            pBarLayer->set3D();

            // Check if display the Volume count value for each bar
            QString qstrVolumeMarkerOptions = (m_pReportOptions->GetOption("adv_production_yield", "marker")).toString();
            QStringList qstrlVolumeMarkerOptionList = qstrVolumeMarkerOptions.split(QString("|"));
            if(qstrlVolumeMarkerOptionList.contains(QString("volume")))
            {
                pBarLayer->setDataLabelFormat("{value|0,}");
                pBarLayer->setDataLabelStyle("arial.ttf",8,Chart::TextColor, 45.0);
            }

            //Y left-Axis: Title & font & scale & scale format
            pAxis = m_pChartDirector->xyChart()->yAxis();
            pAxis->setLabelStyle("arial.ttf");
            pAxis->setTitle("Volume");
            pAxis->setLabelFormat("{value|0,}");		// Display volume (total parts tested)
            lfLowestVolume = (int)lfLowestVolume - ((int)lfHighestVolume*1)/100;	// Go 1% below the lowest bar so we can always see it!
            if(lfLowestVolume <=0) lfLowestVolume = 0;
            lfHighestVolume = (int)lfHighestVolume + ((int)lfHighestVolume*1)/100;	// Go 1% above the highest bar so we can always see it!
            switch(m_pReportOptions->uiVolumeAxis)
            {
                case GEX_ADV_PROD_VOLUMEAXIS_0_CUSTOM:
                    pAxis->setLinearScale(0, gex_max(lfHighestVolume, m_pReportOptions->uiVolumeHL));
                    break;

                case GEX_ADV_PROD_VOLUMEAXIS_MIN_MAX:
                    pAxis->setLinearScale(lfLowestVolume, lfHighestVolume);
                    break;

                case GEX_ADV_PROD_VOLUMEAXIS_MIN_CUSTOM:
                    pAxis->setLinearScale(lfLowestVolume, gex_max(lfHighestVolume, m_pReportOptions->uiVolumeHL));
                    break;

                case GEX_ADV_PROD_VOLUMEAXIS_0_MAX:
                default:
                    pAxis->setLinearScale(0, lfHighestVolume);
                    break;
            }
        }

        // Set the labels on the x axis.
        m_pChartDirector->xyChart()->xAxis()->setLabels(StringArray(Labels,iTotalCells));
        m_pChartDirector->xyChart()->xAxis()->setLabelStyle("arial.ttf",7,Chart::TextColor, 25.0);

        // Save chart to image (build unique name)...only when last group reached!
        strImage.sprintf("prod_%d_%d.png",iGroup,m_pReportOptions->getAdvancedReportSettings());
        strImagePath = m_pReportOptions->strReportDirectory + "/images/";
        strImagePath += strImage;

        switch(m_pReportOptions->getAdvancedReportSettings())
        {
            case GEX_ADV_PRODYIELD_GROUP:
                if(iGroup != (int)getGroupsList().count() - 1)
                    break;	// Exit if not last group yet...otherwise fall in next case!

            case GEX_ADV_PRODYIELD_SBLOT:
            case GEX_ADV_PRODYIELD_LOT:
            case GEX_ADV_PRODYIELD_DAY:
            case GEX_ADV_PRODYIELD_WEEK:
            case GEX_ADV_PRODYIELD_MONTH:
                m_pChartDirector->xyChart()->makeChart(strImagePath.toLatin1().constData());
                // Update report to include image URL
                if(of=="CSV")
                {
                    fprintf(hReportFile,"\n");
                }
                else
                {
                    // If multiple groups, display the current group name
                    if(getGroupsList().count() > 1 && (m_pReportOptions->getAdvancedReportSettings() != GEX_ADV_PRODYIELD_GROUP))
                        fprintf(hReportFile,"<br>Group name: %s<br>",pGroup->strGroupName.toLatin1().constData());
                    fprintf(hReportFile,"<bgcolor=%s> <img border=\"0\" src=\"../images/%s\" >", szDataColor, formatHtmlImageFilename(strImage).toLatin1().constData());
                    fprintf(hReportFile,"<br>\n");
                }
                break;
        }

        // Delete temporary buffers
        for (int lIdx = 0; lIdx < mProductionCell.count(); ++lIdx)
        {
            if (Labels[lIdx])
            {
                delete Labels[lIdx];
                Labels[lIdx] = NULL;
            }
        }

        delete []Labels;
        delete []DataYield;
        delete []DataVolume;

        // Delete any previous charting object
        m_pChartDirector->clearXYChart();

        // Create table using the same orgering as the bars on the chart
        int											iLineID=-1;
        for(it = mProductionCell.begin(); it != mProductionCell.end(); ++it)
        {
            //GCORE-119 when report is per group, this loop was not adapted
            if(m_pReportOptions->getAdvancedReportSettings() == GEX_ADV_PRODYIELD_GROUP
                    && iGroup != (int)getGroupsList().count() - 1)
                break;

            pCell = *it;

            // Keep track of total lilnes in page (only allow a defined number per page!)
            iLineID++;

            // Write header (and page break) ever 26 lines
            if((iLineID % 26) == 0)
            {
                // If previous table open, then close it first!
                if(iLineID && (m_pReportOptions->isReportOutputHtmlBased()) //(of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
                    )
                    fprintf(hReportFile,"</table>\n");

                // Table with Lot details
                WritePageBreak();
                // Update report to include image URL
                if(of=="CSV")
                {
                    // CSV output: write table header only once!
                    if(iLineID == 0)
                        fprintf(hReportFile,"Production report table\nTesting Date,Lot,SubLot,Matching Bin parts,Total parts,Bin Yield\n");
                }
                else
                {
                    fprintf(hReportFile,"<table border=\"0\" width=\"98%%\" cellspacing=\"1\">\n");
                    fprintf(hReportFile,"<tr>\n");
                    fprintf(hReportFile,"<td width=\"30%%\" bgcolor=%s align=\"left\"><b>Testing Date</b></td>\n",szFieldColor);
                    fprintf(hReportFile,"<td width=\"10%%\" bgcolor=%s align=\"left\"><b>Product</b></td>\n",szFieldColor);
                    fprintf(hReportFile,"<td width=\"10%%\" bgcolor=%s align=\"left\"><b>Lot</b></td>\n",szFieldColor);
                    fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"left\"><b>SubLot / Wafer ID</b></td>\n",szFieldColor);
                    fprintf(hReportFile,"<td width=\"10%%\" bgcolor=%s align=\"center\"><b>Matching Bin Parts</b></td>\n",szFieldColor);
                    fprintf(hReportFile,"<td width=\"10%%\" bgcolor=%s align=\"center\"><b>Total Parts</b></td>\n",szFieldColor);
                    fprintf(hReportFile,"<td width=\"10%%\" bgcolor=%s align=\"center\"><b>Bin Yield</b></td>\n",szFieldColor);
                    fprintf(hReportFile,"</tr>\n");
                }
            }

            if(of=="CSV")
            {
                // CSV output
                strString = pCell->mTestingDate;
                strString.replace("\n"," ");

                fprintf(hReportFile,"%s,%s,%s,%ld,%ld,%s %%\n",
                    strString.toLatin1().constData(),
                    pCell->mLot.toLatin1().constData(),
                    pCell->mSubLotWaferID.toLatin1().constData(),
                    pCell->lTotalGoodParts,
                    pCell->lTotalParts,
                    pCell->mYield.toLatin1().constData());
            }
            else
            {
                // HTML, flat HTML
                fprintf(hReportFile,"<tr>\n");
                fprintf(hReportFile,"<td width=\"30%%\" bgcolor=%s align=\"left\">%s</td>\n",szDataColor, pCell->mTestingDate.toLatin1().constData());
                fprintf(hReportFile,"<td width=\"10%%\" bgcolor=%s align=\"left\">%s&nbsp;</td>\n",szDataColor,pCell->mProduct.toLatin1().constData());
                fprintf(hReportFile,"<td width=\"10%%\" bgcolor=%s align=\"left\">%s&nbsp;</td>\n",szDataColor,pCell->mLot.toLatin1().constData());
                fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"left\">%s&nbsp;</td>\n",szDataColor,pCell->mSubLotWaferID.toLatin1().constData());
                fprintf(hReportFile,"<td width=\"10%%\" bgcolor=%s align=\"center\">%ld</td>\n",szDataColor,pCell->lTotalGoodParts);
                fprintf(hReportFile,"<td width=\"10%%\" bgcolor=%s align=\"center\">%ld</td>\n",szDataColor,pCell->lTotalParts);
                fprintf(hReportFile,"<td width=\"10%%\" bgcolor=%s align=\"center\">%s %%</td>\n",szDataColor,pCell->mYield.toLatin1().constData());
                fprintf(hReportFile,"</tr>\n");
            }
        }

        // Close table if not done already
        if(	m_pReportOptions->isReportOutputHtmlBased() //(of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
            && ((iLineID == 0) || (iLineID % 26)))
            fprintf(hReportFile,"</table>\n");

        // Write page break (ignored if not writing a flat HTML document)
        if(getGroupsList().count() > 1)
            WritePageBreak();

        // Delete objects....unless we need to keep it for all groups
        switch(m_pReportOptions->getAdvancedReportSettings())
        {
            case GEX_ADV_PRODYIELD_GROUP:
                if(iGroup != (int)getGroupsList().count() - 1)
                    break;	// Exit if not last group yet...otherwise fall in next case!

            case GEX_ADV_PRODYIELD_SBLOT:
            case GEX_ADV_PRODYIELD_LOT:
            case GEX_ADV_PRODYIELD_DAY:
            case GEX_ADV_PRODYIELD_WEEK:
            case GEX_ADV_PRODYIELD_MONTH:
                for ( it = mProductionCell.begin(); it != mProductionCell.end(); ++it )
                    delete it.value();
                // Empty QMap (list of pointers)
                mProductionCell.clear();
                break;
        }

        // Get next group.
        iGroup++;
    };
}

