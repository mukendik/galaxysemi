/////////////////////////////////////////////////////////////////////////////
// Creates HTML 'Wafermap' page.
/////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
  #include <windows.h>
#endif

#include "gex_shared.h"
#include "browser.h"
#include "browser_dialog.h"
#include "report_build.h"
#include "report_options.h"
#include "gex_constants.h"			// Constants shared in modules
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "cbinning.h"
#include "report_classes_sorting.h"	// Classes to sort lists
#include "pat_info.h"
#include "patman_lib.h"
#include "drillDataMining3D.h"
#include "chart_director.h"
#include "pick_export_wafermap_dialog.h"
#include "gex_algorithms.h"
#include "gexperformancecounter.h"
#include "gqtl_global.h"
#include <gqtl_log.h>
#include "product_info.h"
#include "waf_bin_mismatch.h"
#include "message.h"
#include <QPainter>
#include <QFileDialog>
#include "pat_engine.h"
#include "wafermap_export.h"

// Galaxy QT libraries
#include "gqtl_sysutils.h"

// For MemoryLeak detection: ALWAYS have to be LAST #include in file
//#include "DebugMemory.h"

// main.cpp
extern GexMainwindow *	pGexMainWindow;
extern CGexReport *		gexReport;				// Handle to report class

// cstats.cpp
extern double			ScalingPower(int iPower);

// csl/ZcGexLib.cpp
extern CGexTestRange *	createTestsRange(QString strParameterList,bool bAcceptRange, bool bIsAdvancedReport);

extern QString			formatHtmlImageFilename(const QString& strImageFileName);

extern CReportOptions	ReportOptions;		// Holds options (report_build.h)

// tp_pat_outlier_removal.cpp
//extern CPatInfo	*lPatInfo;			// Holds all global pat info required to create & build PAT reports for a given station (real-time final test), or given file (post-processing/wafer-sort)

// Initialize static members
QColor CBinningColor::m_colorBinOne		= QColor(0,0xc6,0);
QColor CBinningColor::m_colorDefault	= QColor(0xFF,0xFF,0xC0);

int	CGexReport::PrepareSection_Wafermap(BOOL /*bValidSection*/, int nFileIndex)
{
    char	szString[2048];

    // Compute total number of dies tested:
    CGexGroupOfFiles *	pGroup				= NULL;
    CGexFileInGroup *	pFile				= NULL;
    int					lTotalBins			= 0;
    int					lTotalPhysicalDies	= 0;

    // Iterator on Groups list
    QListIterator<CGexGroupOfFiles*> itGroupsList(getGroupsList());

    while(itGroupsList.hasNext())
    {
        pGroup	= itGroupsList.next();

        // update pGroup from pGroup->pFilesList; case 5601
        pGroup->BuildStackedWaferMap(m_pReportOptions);

        pFile	= (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

        // Compute total dies tested
        lTotalPhysicalDies += pGroup->cStackedWaferMapData.iTotalPhysicalDies;

        // Get pointer to wafermap data...if any!
        switch(m_pReportOptions->iWafermapType)
        {
            case GEX_WAFMAP_SOFTBIN:// Standard Wafer map: Software Binning
            case GEX_WAFMAP_STACK_SOFTBIN:
            case GEX_WAFMAP_ZONAL_SOFTBIN:
                lTotalBins += pGroup->cMergedData.lTotalSoftBins;
                break;
            case GEX_WAFMAP_HARDBIN:// Standard Wafer map: Hardware Binning
            case GEX_WAFMAP_STACK_HARDBIN:
            case GEX_WAFMAP_ZONAL_HARDBIN:
            default:
                lTotalBins += pGroup->cMergedData.lTotalHardBins;
                break;
        }
    };

    QString of = m_pReportOptions->GetOption("output", "format").toString();
    QStringList strLstWafermapToShow = m_pReportOptions->GetOption("wafer", "chart_show").toString().split("|");

    // Open Wafer map file section...
    if (of=="CSV")
    {
        // Header to display if some wafer info to report
        if (strLstWafermapToShow.contains(QString("stacked")) || strLstWafermapToShow.contains(QString("all_individual")) || strLstWafermapToShow.contains(QString("bin_mismatch")) || strLstWafermapToShow.contains(QString("bin_to_bin")) )
        {
            fprintf(hReportFile,"\n---- WAFER MAP ----\n\n");
            fprintf(hReportFile,"\nDies tested (including retest),%d\n",lTotalBins);
            fprintf(hReportFile,"\nTotal physical parts tested (all wafermaps),%d\n",lTotalPhysicalDies);
            if(pFile && pFile->grossDieCount() > 0 )
                fprintf(hReportFile,"\nGross die per wafer,%d\n", pFile->grossDieCount());
        }
    }
    else
    if(m_pReportOptions->isReportOutputHtmlBased()) //(of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
    {
        // Open <stdf-filename>/report/wafermap.htm
        if ( of=="HTML"
           && m_pReportOptions->strTemplateFile.isEmpty()
           && !(GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(
                       GS::LPPlugin::ProductInfo::waferMap)))
        {
                // Create Test index page
            if (m_pReportOptions->iWafermapType == GEX_WAFMAP_TESTOVERLIMITS || m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_TESTOVERLIMITS ||
                m_pReportOptions->iWafermapType == GEX_WAFMAP_TESTOVERDATA || m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_TESTOVERDATA ||
                m_pReportOptions->iWafermapType == GEX_WAFMAP_TEST_PASSFAIL || m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_TEST_PASSFAIL)
                sprintf(szString,"%s/pages/wafermap%d.htm",m_pReportOptions->strReportDirectory.toLatin1().constData(), nFileIndex);
            else
                sprintf(szString,"%s/pages/wafermap.htm",m_pReportOptions->strReportDirectory.toLatin1().constData());

            hReportFile = fopen(szString,"wt");
            if(hReportFile == NULL)
                return GS::StdLib::Stdf::ReportFile;
        }
        WriteHeaderHTML(hReportFile,"#000000");	// Default: Text is Black

        // Title + bookmark ""Wafer maps & Strip maps"
        if(m_pReportOptions->strTemplateFile.isEmpty())
            WriteHtmlSectionTitle(hReportFile,"all_wafers",m_pReportOptions->GetOption("wafer", "section_name").toString());

        // If creating a WORD file (out from a flat HTML), do not include HTML page navigation buttons...otherwise do it!
        if( of=="HTML"
           && iCurrentHtmlPage)
            WriteNavigationButtons(GEX_T("wafermap"), true);

        if (of=="HTML")
            fprintf(hReportFile,"<table border=\"0\" width=\"98%%\" height=\"74\">\n");
        else
            WriteHtmlOpenTable(98,0);	// HTML code to open table, 98%, cell spacing=0

        // Wafer info, time/date + ID
        WriteWaferMapTableLine("Map type", GetWaferMapReportType().toLatin1().constData());

        if(lTotalBins > 0)
        {
            sprintf(szString,"%d",lTotalBins);
            WriteWaferMapTableLine("Devices tested (with retests)",szString);
        }
        if(lTotalPhysicalDies > 0)
        {
            sprintf(szString,"%d ( only applies to wafermaps )",lTotalPhysicalDies);
            WriteWaferMapTableLine("Total physical parts tested",szString);
            if(pFile && pFile->grossDieCount() > 0)
            {
                sprintf(szString,"%d",pFile->grossDieCount());
                WriteWaferMapTableLine("Gross die per wafer",szString);
            }
        }
        fprintf(hReportFile,"</table><br>\n");

        // Comment.
        if (of=="HTML")
            fprintf(hReportFile,"<br>Note: Customize wafermap size from <a href=\"_gex_options.htm\">'Options'</a> page, section 'Wafer map, Strip map / Chart size'<br>\n");

        // Check how many wafermap images to build...
        // Get pointer to first group
        pGroup = getGroupsList().size()>0?getGroupsList().first():NULL;

        // Write page break (ignored if not writing a flat HTML document)...unless we only have ONE wafer and ONE group
        // (then write next page (bins & colors) with this one as both pages use only few lines.
        if(pGroup && ((m_pReportOptions->getAdvancedReport() != GEX_ADV_OUTLIER_REMOVAL) && (getGroupsList().count() != 1 || pGroup->pFilesList.count() != 1)))
        {
            // Write table of hyperlinks to several wafermap sections (unless PowerPoint
            //if (of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
            if(m_pReportOptions->isReportOutputHtmlBased())
            {
                fprintf(hReportFile,"<p align=\"left\">The wafermaps / Strip maps report includes:</p>\n");
                fprintf(hReportFile,"<blockquote>\n");
                fprintf(hReportFile,"<p align=\"left\">");

                // Wafermap gallery
                if((pGroup->pFilesList.count() > 1 || getGroupsList().count() > 1) && strLstWafermapToShow.contains("all_individual"))
                    fprintf(hReportFile,"<img border=\"0\" src=\"../images/action.png\" align=\"center\" width=\"17\" height=\"17\"> <a href=\"#wafmap_gallery\">Wafer maps gallery</a><br>\n");

                // wafermap compare
                if(getGroupsList().count() == 2 && ( strLstWafermapToShow.contains(QString("bin_mismatch"))||strLstWafermapToShow.contains(QString("bin_to_bin")) ))
                    fprintf(hReportFile,"<img border=\"0\" src=\"../images/action.png\" align=\"center\" width=\"17\" height=\"17\"> <a href=\"#wafmap_mismatch\">Compare Wafers</a><br>\n");

                // Stacked wafermap results
                if(pGroup->cStackedWaferMapData.iTotalWafermaps > 1 && strLstWafermapToShow.contains("stacked"))
                {
                    // Stacked wafer
                    fprintf(hReportFile,"<img border=\"0\" src=\"../images/action.png\" align=\"center\" width=\"17\" height=\"17\"> <a href=\"#all_stackedwafers\">Stacked wafermap</a><br>\n");

                    // Low-Yield patterns on stacked wafer-maps
                    fprintf(hReportFile,"<img border=\"0\" src=\"../images/action.png\" align=\"center\" width=\"17\" height=\"17\"> <a href=\"#all_stacked_patterns\">Low-Yield patterns on stacked wafermap</a><br>\n");
                }

                // Individual wafers
                if(strLstWafermapToShow.contains("all_individual"))
                    fprintf(hReportFile,"<img border=\"0\" src=\"../images/action.png\" align=\"center\" width=\"17\" height=\"17\"> <a href=\"#all_individualwafers\">Individual Wafer maps</a><br>\n");

                fprintf(hReportFile,"</p></blockquote>\n");
            }

            WritePageBreak();
        }

        int	iTotalWafermaps=0;

        // Loop at ALL groups
        itGroupsList.toFront();
        while(itGroupsList.hasNext())
        {
            // Get pointer to first file in group
            pGroup = itGroupsList.next();

            QListIterator<CGexFileInGroup*> itFilesList(pGroup->pFilesList);

            // Loop at Files in group groups
            while(itFilesList.hasNext())
            {
                pFile = itFilesList.next();

                if(pFile->getWaferMapData().getWafMap() != NULL)
                    iTotalWafermaps++;
            };
        };

        // PAT-Man report: we may have multiple groups but their are sites for the same data file!
        if(m_pReportOptions->getAdvancedReport() == GEX_ADV_OUTLIER_REMOVAL)
            iTotalWafermaps= 1;

        // Saves total number of charts to create...
        m_pReportOptions->lAdvancedHtmlPages = iTotalWafermaps;
    }
    return GS::StdLib::Stdf::NoError;
}


///////////////////////////////////////////////////////////
// Export wafermap to file:
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_ExportWafermap(void)
{
    Wizard_ExportWafermap(m_selectedURL.section('#',2));
}


void GexMainwindow::Wizard_ExportWafermap(QString strLink)
{
    // Enable/disable some features...
    if(GS::LPPlugin::ProductInfo::getInstance()->isExaminator() ||
       GS::LPPlugin::ProductInfo::getInstance()->isOEM())
    {
        QString m=ReportOptions.GetOption("messages", "upgrade").toString();
        GS::Gex::Message::information("", m);
        return;
    }

    // Get parameter of the URL (string after the '#'), extract GroupID & FileID
    QString strString = strLink.section("--",0,0);	//= "	<type>"
    QString strField = strString.section('=',0,0);	// = "drill_3d"
    QString strChartType = strString.section('=',1,1);	// "wafer", etc...

    // Extract Group#
    strString		=	strLink.section("--",1,1);	// = "g=<group#>
    strField		= strString.section('=',1,1);	// = group# (0= 1st group, etc...)
    int iGroupID	= strField.toLong();

    // Extract File#
    strString		= strLink.section("--",2,2);	// = "f=<file#>
    strField		= strString.section('=',1,1);	// = file# (0=1st file, etc...)
    int iFileID		= strField.toLong();

    // Extract stacked mode if exists (for stacked wafermap only, define how the wafermap has been stacked)
    QString strStackedMode;
    strString		= strLink.section("--",3,3);	// = "stacked=<mode>"
    if (!strString.isEmpty())
        strStackedMode = strString.section('=',1,1);

    // Define output path
    QString strWaferFileFullName;

    // Set the export wafer path to the default output path
    if (m_strExportWafermapPath.isEmpty())
        m_strExportWafermapPath = CGexReport::outputLocation(&ReportOptions);

    // WAFERMAP format selected...
    PickExportWafermapDialog::outputFormat	eFormatMask		= (iFileID == -1) ? PickExportWafermapDialog::outputPng : PickExportWafermapDialog::outputAll;
    CGexReport::wafermapMode				eWafermapMode	= CGexReport::individualWafermap;

    if (strStackedMode.compare("PassFailAll", Qt::CaseInsensitive) == 0)
    {
        eFormatMask = PickExportWafermapDialog::outputPng;
        eFormatMask |= PickExportWafermapDialog::outputLaurierDieSort1D;
        eFormatMask |= PickExportWafermapDialog::outputSemiG85inkAscii;

        eWafermapMode	= CGexReport::stackedWafermap;
    }
    else if (strStackedMode.compare("BinCount", Qt::CaseInsensitive) == 0)
        eWafermapMode	= CGexReport::stackedWafermap;
    else if (strStackedMode.compare("DieMismatchBin", Qt::CaseInsensitive) == 0)
        eWafermapMode	= CGexReport::compareBinMismatch;
    else if (strStackedMode.compare("DieMismatchBinToBin", Qt::CaseInsensitive) == 0)
        eWafermapMode	= CGexReport::compareBinToBinCorrelation;
    else if (strChartType.compare("wafer_param_range", Qt::CaseInsensitive) == 0 || strChartType.compare("wafer_param_limits", Qt::CaseInsensitive) == 0 ||
                strChartType.compare("wafer_test_passfail", Qt::CaseInsensitive) == 0 )
    {
        eFormatMask = PickExportWafermapDialog::outputPng;
    }


    PickExportWafermapDialog dlgExportWafermap(eFormatMask);

    if(dlgExportWafermap.exec() == QDialog::Rejected)
        return;

    QString				strErrorMessage;
    CTest *				pTestCell		= NULL;
    CGexGroupOfFiles *	pGroup			= NULL;
    bool				bPromptUser		= true;
    int					iNotch			= dlgExportWafermap.notchDirection();
    bool				bDumpAllFiles	= false;
    bool				bDumpAllGroups	= false;
    bool				bExportAll		= false;

    // Export all wafermaps selected
    if(dlgExportWafermap.checkBoxExportAllWafers->isChecked())
    {
        iGroupID = 0;	// Start from first group

        // Export One Wafer per File
        if (dlgExportWafermap.exportMode() == PickExportWafermapDialog::modeOneWaferPerFile)
        {
            // If iFileID < 0, it means we want to export stacked wafer map. We don't need to loop on all files
            if (iFileID >= 0)
            {
                iFileID			= 0;		// Start from first wafer in list.
                bDumpAllFiles	= true;
            }

            // Do not prompt user into export methods
            bPromptUser				= false;
            bDumpAllGroups			= true;
            bExportAll				= true;
        }
    }

    // Export all wafermap, no need to ask the files name, just ask for the folder
    if (bExportAll)
        m_strExportWafermapPath	= QFileDialog::getExistingDirectory(this, "Select the folder where you want to export all wafermaps...",
                                                                    m_strExportWafermapPath);

    int	iStatus = WafermapExport::NoError;

    // Export wafermaps
    while(iGroupID >= 0 && iGroupID < gexReport->getGroupsList().count())
    {
        pGroup = gexReport->getGroupsList().at(iGroupID);

        while (iFileID < (int) pGroup->pFilesList.count())
        {
            if (strChartType.compare("wafer_hbin", Qt::CaseInsensitive) == 0)
                gexReport->FillWaferMap(iGroupID, iFileID, pTestCell, GEX_WAFMAP_HARDBIN);
            else if (strChartType.compare("wafer_sbin", Qt::CaseInsensitive) == 0)
                gexReport->FillWaferMap(iGroupID, iFileID, pTestCell, GEX_WAFMAP_SOFTBIN);

            switch(dlgExportWafermap.format())
            {
                case PickExportWafermapDialog::outputTSMCink :	// TSMC format
                    iStatus = WafermapExport::CreateWafermapOutput_TSMC(m_strExportWafermapPath,strWaferFileFullName,iNotch,GS::Gex::PATProcessing::eDefault,GS::Gex::PATProcessing::eDefault,true,strErrorMessage,bPromptUser,iGroupID,iFileID);
                    break;

                case PickExportWafermapDialog::outputSemiG85inkAscii :	// G85 Semi85 format
                    iStatus = WafermapExport::CreateWafermapOutput_G85(m_strExportWafermapPath,strWaferFileFullName,iNotch,GS::Gex::PATProcessing::eDefault,GS::Gex::PATProcessing::eDefault,true,strErrorMessage,"","",false,bPromptUser,iGroupID,iFileID);
                    break;

                case PickExportWafermapDialog::outputSemiG85inkXml :	// G85 XML Semi85 format
                    iStatus = WafermapExport::CreateWafermapOutput_G85(m_strExportWafermapPath,strWaferFileFullName,iNotch,GS::Gex::PATProcessing::eDefault,GS::Gex::PATProcessing::eDefault,true,strErrorMessage,"","",true,bPromptUser,iGroupID,iFileID);
                    break;

                case PickExportWafermapDialog::outputSemiE142ink :	// SEMI E142 format
                    iStatus = WafermapExport::CreateWafermapOutput_SEMI142(false,m_strExportWafermapPath,strWaferFileFullName,iNotch,GS::Gex::PATProcessing::eDefault,GS::Gex::PATProcessing::eDefault,true,strErrorMessage,"","",bPromptUser,iGroupID,iFileID);
                    break;

                case PickExportWafermapDialog::outputSemiE142inkInteger2 :	// SEMI E142-Integer2 format
                    iStatus = WafermapExport::CreateWafermapOutput_SEMI142(true,m_strExportWafermapPath,strWaferFileFullName,iNotch,GS::Gex::PATProcessing::eDefault,GS::Gex::PATProcessing::eDefault,true,strErrorMessage,"","",bPromptUser,iGroupID,iFileID);
                    break;

                case PickExportWafermapDialog::outputKLA_INF :	// KLA/INF simplified
                {
                    GexTbPatSinf lSinfInfo;

                    iStatus = WafermapExport::CreateWafermapOutput_SINF(m_strExportWafermapPath,strWaferFileFullName,iNotch,GS::Gex::PATProcessing::eDefault,GS::Gex::PATProcessing::eDefault,true,strErrorMessage,lSinfInfo,bPromptUser,iGroupID,iFileID);
                    break;
                }

                case PickExportWafermapDialog::outputHtml :	// HTML

                    if (dlgExportWafermap.exportMode() == PickExportWafermapDialog::modeAllWaferInOneFile)
                    {
                        iFileID		= -1;
                        iGroupID	= -1;
                    }

                    iStatus = WafermapExport::CreateWafermapOutput_HTML(m_strExportWafermapPath,strWaferFileFullName,iNotch,GS::Gex::PATProcessing::eDefault,GS::Gex::PATProcessing::eDefault,true,strErrorMessage,bPromptUser,iGroupID,iFileID);
                    break;

                case PickExportWafermapDialog::outputPng :	// Image / PNG
                    iStatus = WafermapExport::CreateWafermapOutput_IMAGE_PNG(m_strExportWafermapPath,strWaferFileFullName,iNotch,GS::Gex::PATProcessing::eDefault,GS::Gex::PATProcessing::eDefault,true,strErrorMessage, eWafermapMode,bPromptUser,iGroupID,iFileID);
                    break;

                case PickExportWafermapDialog::outputLaurierDieSort1D :	// Laurier Die Sort 1D
                    iStatus = WafermapExport::CreateWafermapOutput_LaurierDieSort1D(m_strExportWafermapPath,strWaferFileFullName,iNotch,GS::Gex::PATProcessing::eDefault,GS::Gex::PATProcessing::eDefault,true,strErrorMessage,bPromptUser,iGroupID,iFileID);
                    break;

                case PickExportWafermapDialog::outputSTIF : // STIF
                    iStatus = WafermapExport::CreateWafermapOutput_STIF(m_strExportWafermapPath,strWaferFileFullName,iNotch,GS::Gex::PATProcessing::eDefault,GS::Gex::PATProcessing::eDefault,true,strErrorMessage,bPromptUser,iGroupID,iFileID);
                    break;
            }

            // Check if no Error
            if(!((iStatus == WafermapExport::NoError) || (iStatus == WafermapExport::ProcessDone)))
            {
                // Stop the process
                bDumpAllFiles = bDumpAllGroups = false;
                break;
            }

            // Move to next file/wafer in group.
            if(bDumpAllFiles)
                iFileID++;
            else
                break;
        }

        // Move to next Group
        if(bDumpAllGroups)
        {
            iGroupID++;
            // Reset to first file if dumping all files
            if(bDumpAllFiles)
                iFileID = 0;
        }
        else
            break;
    }

    if(!((iStatus == WafermapExport::NoError) || (iStatus == WafermapExport::ProcessDone)))
    {
        GS::Gex::Message::information("", strErrorMessage);
        return;
    }
    return;
}

/////////////////////////////////////////////////////////////////////////////
// Swap X and/or Y axis if needed
/////////////////////////////////////////////////////////////////////////////
bool	CGexFileInGroup::SwapWaferMapDies(bool bXaxis/*=true*/,bool bYaxis/*=true*/)
{
    QStringList lstVisualOptions = pReportOptions->GetOption("wafer", "visual_options").toString().split("|");

    // No swap required
    if(lstVisualOptions.contains("mirror_x") == false && lstVisualOptions.contains("mirror_y") == false)
        return false;

    // Swap X axis.
    int	iIndex;
    int iCol,iLine,iOffset,iDieFrom,iDieFromOrgRetest;

    if (bXaxis && lstVisualOptions.contains("mirror_x"))
    {
        // Clear die index
        iIndex=0;

        // Die location can be computed from:
        // iX = iIndex % pFile->getWaferMapData().SizeX;
        // iY = iIndex / pFile->getWaferMapData().SizeX;
        while(iIndex < getWaferMapData().SizeX*getWaferMapData().SizeY)
        {
            for(iCol=0; iCol < getWaferMapData().SizeX/2;iCol++)
            {
                // Save bin prior to overwrite it with symetrical die on X axis.
                iDieFrom = getWaferMapData().getWafMap()[iIndex+iCol].getBin();
                iDieFromOrgRetest = getWaferMapData().getWafMap()[iIndex+iCol].getOrgBin();
                // Offset between two dies to swap
                iOffset = getWaferMapData().SizeX-2*iCol-1;
                // Swap dies
                getWaferMapData().getWafMap()[iIndex+iCol].setBin(getWaferMapData().getWafMap()[iIndex+iCol+iOffset].getBin());
                getWaferMapData().getWafMap()[iIndex+iCol].setOrgBin(getWaferMapData().getWafMap()[iIndex+iCol+iOffset].getOrgBin());
                getWaferMapData().getWafMap()[iIndex+iCol+iOffset].setBin (iDieFrom);
                getWaferMapData().getWafMap()[iIndex+iCol+iOffset].setOrgBin (iDieFromOrgRetest);
            }
            // Go to next line
            iIndex += getWaferMapData().SizeX;
        };
    }

    // Swap Y axis.
    if (bYaxis && lstVisualOptions.contains("mirror_y"))
    {
        // Die location can be computed from:
        // iX = iIndex % pFile->getWaferMapData().SizeX;
        // iY = iIndex / pFile->getWaferMapData().SizeX;
        for(iIndex =0;iIndex < getWaferMapData().SizeX; iIndex++)
        {
            for(iLine=0; iLine < getWaferMapData().SizeY/2;iLine++)
            {
                // Save bin prior to overwrite it with symetrical die on X axis.
                iDieFrom = getWaferMapData().getWafMap()[iIndex+(iLine*getWaferMapData().SizeX)].getBin();
                iDieFromOrgRetest = getWaferMapData().getWafMap()[iIndex+(iLine*getWaferMapData().SizeX)].getOrgBin();
                // Offset between two dies to swap
                iOffset = getWaferMapData().SizeY-2*iLine-1;
                // Swap dies
                getWaferMapData().getWafMap()[iIndex+(iLine*getWaferMapData().SizeX)].setBin( getWaferMapData().getWafMap()[iIndex+((iLine+iOffset)*getWaferMapData().SizeX)].getBin());
                getWaferMapData().getWafMap()[iIndex+(iLine*getWaferMapData().SizeX)].setOrgBin(getWaferMapData().getWafMap()[iIndex+((iLine+iOffset)*getWaferMapData().SizeX)].getOrgBin());
                getWaferMapData().getWafMap()[iIndex+((iLine+iOffset)*getWaferMapData().SizeX)].setBin(iDieFrom);
                getWaferMapData().getWafMap()[iIndex+((iLine+iOffset)*getWaferMapData().SizeX)].setOrgBin(iDieFromOrgRetest);
            }
        }
    }

    // Swap performed
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// Merge all wafermaps in a single group, result in 'CStackedWaferMap' class
/////////////////////////////////////////////////////////////////////////////
void CGexGroupOfFiles::BuildStackedWaferMap(CReportOptions *pReportOptions,QString strBinList/*=""*/,int iWafermapType/*=-1*/)
{
    typedef					QVector<double>	td_lstDouble;
    double					lfLow=0.0,lfHigh=0.0,lfValue=0.0;
    int						i,iBin;
    int						iOffsetX;	// Offset in X to merge each wafermap correctly
    int						iOffsetY;	// Offset in Y to merge each wafermap correctly
    int						iIndex,iX,iY;
    bool					bPassFailMode = false;
    CTest *					ptTestCell=NULL;	// Pointer to test cell to receive STDF info.
    CGexFileInGroup *		pFile;
    QMap<int, BYTE>			mapGoodBinning;
    QVector<td_lstDouble>	vecListValue;		// Handle the list of value for each die

    QString					strWafermapBinStacked			= pReportOptions->GetOption("wafer","bin_stacked").toString();
    QString					strWafermapParametricStacked	= pReportOptions->GetOption("wafer","parametric_stacked").toString();

    // Type of wafermap to stack...
    if(iWafermapType < 0)
        iWafermapType = pReportOptions->iWafermapType;

    // Erase Stacked wafermap if it already exists!
    cStackedWaferMapData.clear();

    // First: swap X and/or Y dies if the wafermap is not in the right order.
    // Make Die locations shown in ascending orde: LowX...HighX
    QListIterator<CGexFileInGroup*> itFilesList(pFilesList);

    while(itFilesList.hasNext())
    {
        pFile = itFilesList.next();

        if(pFile->getWaferMapData().getWafMap() != NULL)
        {
            // The X die coordinates are not in ascending...swap X cells.
            if(pFile->getWaferMapData().iLowDieX > pFile->getWaferMapData().iHighDieX)
            {
                // Swap Low,High die values.
                iIndex = pFile->getWaferMapData().iLowDieX;
                pFile->getWaferMapData().iLowDieX = pFile->getWaferMapData().iHighDieX;
                pFile->getWaferMapData().iHighDieX = iIndex;
                pFile->getWaferMapData().SetPosXDirection(true);	// Positive X increments for drawing wafermap scale
            }
            // The Y die coordinates are not in ascending...swap Y cells.
            if(pFile->getWaferMapData().iLowDieY > pFile->getWaferMapData().iHighDieY)
            {
                // Swap Low,High die values.
                iIndex = pFile->getWaferMapData().iLowDieY;
                pFile->getWaferMapData().iLowDieY = pFile->getWaferMapData().iHighDieY;
                pFile->getWaferMapData().iHighDieY = iIndex;
                pFile->getWaferMapData().SetPosYDirection(true);	// Positive Y increments for drawing wafermap scale
            }
        }
    };

    pFile  = (pFilesList.isEmpty()) ? NULL : pFilesList.first();
    switch(iWafermapType)
    {
    case GEX_WAFMAP_SOFTBIN:
    case GEX_WAFMAP_HARDBIN:
    {
        // Create list of binnings to merge.
        if(cStackedWaferMapData.pGexBinList != NULL)
            delete cStackedWaferMapData.pGexBinList;

        cStackedWaferMapData.pGexBinList = new GS::QtLib::Range(pReportOptions->pGexWafermapRangeList->BuildTestListString(""));

        CBinning * ptBinCell = NULL;
        if(iWafermapType == GEX_WAFMAP_SOFTBIN)
            ptBinCell = cMergedData.ptMergedSoftBinList;
        else
            ptBinCell = cMergedData.ptMergedHardBinList;

        while(ptBinCell != NULL)
        {
            if (ptBinCell->cPassFail == 'P')
                mapGoodBinning[ptBinCell->iBinValue] = ptBinCell->cPassFail;
            // Move to next Bin cell
            ptBinCell = ptBinCell->ptNextBin;
        }
    }
    break;

    case GEX_WAFMAP_STACK_SOFTBIN:
    case GEX_WAFMAP_ZONAL_SOFTBIN:
    case GEX_WAFMAP_STACK_HARDBIN:
    case GEX_WAFMAP_ZONAL_HARDBIN:
    {
        if(GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(
                    GS::LPPlugin::ProductInfo::waferStack))
        {
            GSLOG(SYSLOG_SEV_ERROR, "error : Your licence doesn't allow this function" );
            break;
        }
        // Create list of binnings to merge.
        if(cStackedWaferMapData.pGexBinList != NULL)
            delete cStackedWaferMapData.pGexBinList;

        cStackedWaferMapData.pGexBinList = new GS::QtLib::Range(pReportOptions->pGexWafermapRangeList->BuildTestListString(""));

        CBinning * ptBinCell = NULL;
        if(iWafermapType == GEX_WAFMAP_SOFTBIN)
            ptBinCell = cMergedData.ptMergedSoftBinList;
        else
            ptBinCell = cMergedData.ptMergedHardBinList;

        while(ptBinCell != NULL)
        {
            if (ptBinCell->cPassFail == 'P')
                mapGoodBinning[ptBinCell->iBinValue] = ptBinCell->cPassFail;
            // Move to next Bin cell
            ptBinCell = ptBinCell->ptNextBin;
        }
    }
    break;

    case GEX_WAFMAP_TESTOVERLIMITS:  // zoning on test limits
    case GEX_WAFMAP_STACK_TESTOVERLIMITS:
    case GEX_WAFMAP_TESTOVERDATA:  // zoning on test values range
    case GEX_WAFMAP_STACK_TESTOVERDATA:
    case GEX_WAFMAP_TEST_PASSFAIL:
    case GEX_WAFMAP_STACK_TEST_PASSFAIL:
        long lZoningTest=-1,lZoningPinmapIndex=-1;
        if(pReportOptions->pGexWafermapRangeList && pReportOptions->pGexWafermapRangeList->pTestRangeList)
        {
            lZoningTest = pReportOptions->pGexWafermapRangeList->pTestRangeList->lFromTest;
            lZoningPinmapIndex = pReportOptions->pGexWafermapRangeList->pTestRangeList->lFromPinmap;
        }
        if(pFile->FindTestCell(lZoningTest,lZoningPinmapIndex,&ptTestCell,false,false) !=1)
            return;	// Zoning wafermap: No zoning wafermap data available for this test
        if(iWafermapType == GEX_WAFMAP_TESTOVERLIMITS ||
                iWafermapType == GEX_WAFMAP_STACK_TESTOVERLIMITS)
        {
            if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0) // Low limit exists
                lfLow = ptTestCell->GetCurrentLimitItem()->lfLowLimit;
            else
                lfLow = ptTestCell->lfSamplesMin;
            if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0) // High limit exists
                lfHigh = ptTestCell->GetCurrentLimitItem()->lfHighLimit;
            else
                lfHigh = ptTestCell->lfSamplesMax;
        }
        else
            if(iWafermapType == GEX_WAFMAP_TESTOVERDATA ||
                    iWafermapType == GEX_WAFMAP_STACK_TESTOVERDATA)
            {
                lfLow = ptTestCell->lfSamplesMin;
                lfHigh = ptTestCell->lfSamplesMax;
            }

        if ((ptTestCell->bTestType == 'P' || ptTestCell->bTestType == 'F') && (iWafermapType == GEX_WAFMAP_TEST_PASSFAIL || iWafermapType == GEX_WAFMAP_STACK_TEST_PASSFAIL))
            bPassFailMode = true;
        break;
    }

    // Step#1: compute stacked wafermap dimensions
    // Back to front of the file list
    itFilesList.toFront();

    while(itFilesList.hasNext())
    {
        pFile = itFilesList.next();
        if(pFile->getWaferMapData().getWafMap() != NULL)
        {
            // This file has a valid wafermap...check how it fits over the other wafers.
            cStackedWaferMapData.iLowDieX = gex_min(pFile->getWaferMapData().iLowDieX,cStackedWaferMapData.iLowDieX);
            cStackedWaferMapData.iHighDieX = gex_max(pFile->getWaferMapData().iHighDieX,cStackedWaferMapData.iHighDieX);
            cStackedWaferMapData.iLowDieY = gex_min(pFile->getWaferMapData().iLowDieY,cStackedWaferMapData.iLowDieY);
            cStackedWaferMapData.iHighDieY = gex_max(pFile->getWaferMapData().iHighDieY,cStackedWaferMapData.iHighDieY);
            // If parametric wafermap, keep track of test data range.
            if(ptTestCell != NULL)
            {
                cStackedWaferMapData.lfLowWindow = gex_min(lfLow,cStackedWaferMapData.lfLowWindow);
                cStackedWaferMapData.lfHighWindow = gex_max(lfHigh,cStackedWaferMapData.lfHighWindow);
            }
            // Update wafermap count.
            cStackedWaferMapData.iTotalWafermaps++;
        }
    };

    // Check if we have at least one valid wafermap!
    if(cStackedWaferMapData.iTotalWafermaps <= 0)
    {
        cStackedWaferMapData.SizeX = 0;
        cStackedWaferMapData.SizeY = 0;
        return;
    }

    // Check if custom Binlist specified
    if(strBinList.isEmpty() == false)
    {
        if(cStackedWaferMapData.pGexBinList != NULL)
            delete cStackedWaferMapData.pGexBinList;
        cStackedWaferMapData.pGexBinList = new GS::QtLib::Range(strBinList);
    }

    // Step#2: create + clear data array to hold merged data
    cStackedWaferMapData.SizeX = cStackedWaferMapData.iHighDieX-cStackedWaferMapData.iLowDieX+1;
    cStackedWaferMapData.SizeY = cStackedWaferMapData.iHighDieY-cStackedWaferMapData.iLowDieY+1;
    long iStackedSize = cStackedWaferMapData.SizeX*cStackedWaferMapData.SizeY;

    cStackedWaferMapData.bWaferMapExists = true;
    cStackedWaferMapData.cWafMap = new CStackedWafMapArray[iStackedSize];
    vecListValue.resize(iStackedSize);
    for(i=0;i< cStackedWaferMapData.SizeX*cStackedWaferMapData.SizeY;i++)
    {
        cStackedWaferMapData.cWafMap[i].ldCount				= GEX_WAFMAP_EMPTY_CELL;	// Number of tests or bins merged for this die.
        cStackedWaferMapData.cWafMap[i].lStatus				= GEX_WAFMAP_EMPTY_CELL;    // Die status
        cStackedWaferMapData.cWafMap[i].nBinToBinDie		= 0;						//
        cStackedWaferMapData.cWafMap[i].lfParamTotal		= 0.0;						// Sum of X
        cStackedWaferMapData.cWafMap[i].lfParamTotalSquare	= 0.0;						// Sum of X*X
    }

    // Step#3: merge (stack) wafermap data
    // Back to front of the file list
    itFilesList.toFront();

    while(itFilesList.hasNext())
    {
        pFile = itFilesList.next();
        // Load Wafermap array with relevant data
        gexReport->FillWaferMap(this,pFile,ptTestCell,iWafermapType);
        if(pFile->getWaferMapData().getWafMap() != NULL)
        {
            // Compute Die location offset for merging this wafermap
            iOffsetX = pFile->getWaferMapData().iLowDieX - cStackedWaferMapData.iLowDieX;
            iOffsetY = pFile->getWaferMapData().iLowDieY - cStackedWaferMapData.iLowDieY;

            // Reset counters
            pFile->getWaferMapData().iTotalPhysicalDies = 0;

            // Merge this wafermap to the stacked array.
            for(iIndex=0;iIndex < pFile->getWaferMapData().SizeX*pFile->getWaferMapData().SizeY;iIndex++)
            {
                // Compute die location in stacked wafer
                iX = iIndex % pFile->getWaferMapData().SizeX;
                iY = iIndex / pFile->getWaferMapData().SizeX;
                i = (iY+iOffsetY)*cStackedWaferMapData.SizeX + (iX+iOffsetX);

                // Get die value (Bin# or Parametric % value)
                iBin = pFile->getWaferMapData().getWafMap()[iIndex].getBin();

                // Keep track of the number of physical dies tested per wafer
                if(iBin != GEX_WAFMAP_EMPTY_CELL)
                {
                    // Total dies on this wafer
                    pFile->getWaferMapData().iTotalPhysicalDies++;
                    // Total dies all together.
                    cStackedWaferMapData.iTotalPhysicalDies++;
                }

                if(ptTestCell == NULL)
                {
                    if (strWafermapBinStacked == "bin_count")
                    {
                        // Binning wafermap
                        // check if we have to count this bin#
                        if((cStackedWaferMapData.pGexBinList != NULL) &&
                           (cStackedWaferMapData.pGexBinList->Contains(iBin)==true))
                        {
                            // Yes...then compute die location to update on stacked wafermap
                            if(cStackedWaferMapData.cWafMap[i].ldCount != GEX_WAFMAP_EMPTY_CELL)
                                cStackedWaferMapData.cWafMap[i].ldCount++;	// Not first time we increment.
                            else
                                cStackedWaferMapData.cWafMap[i].ldCount = 1;	// was -1 (means not set), now set it to 1 (as it is 1st count)
                        }
                        else if((iBin != GEX_WAFMAP_EMPTY_CELL) && (cStackedWaferMapData.cWafMap[i].ldCount == GEX_WAFMAP_EMPTY_CELL))
                            cStackedWaferMapData.cWafMap[i].ldCount = 0;	// Marks that this is a valid die...even if not a matching bin!
                    }
                    else if (strWafermapBinStacked == "pf_pass_if_all_pass")
                    {
                        if (iBin != GEX_WAFMAP_EMPTY_CELL)
                        {
                            if (mapGoodBinning.contains(iBin))
                                iBin = GEX_WAFMAP_PASS_CELL;
                            else
                                iBin = GEX_WAFMAP_FAIL_CELL;
                        }

                        if (iBin == GEX_WAFMAP_PASS_CELL)
                        {
                            // Yes...then compute die location to update on stacked wafermap
                            if(cStackedWaferMapData.cWafMap[i].ldCount != GEX_WAFMAP_EMPTY_CELL)
                                cStackedWaferMapData.cWafMap[i].ldCount++;	// Not first time we increment.
                            else
                                cStackedWaferMapData.cWafMap[i].ldCount = 1;	// was -1 (means not set), now set it to 1 (as it is 1st count)
                        }
                        else if((iBin != GEX_WAFMAP_EMPTY_CELL) && (cStackedWaferMapData.cWafMap[i].ldCount == GEX_WAFMAP_EMPTY_CELL))
                            cStackedWaferMapData.cWafMap[i].ldCount = 0;	// Marks that this is a valid die...even if not a matching bin!
                    }
#if 0 // Waiting for new options
                    else if (strWafermapBinStacked == "pf_pass_if_one_pass")
                    {
                        if (iBin != GEX_WAFMAP_EMPTY_CELL)
                        {
                            if (mapGoodBinning.contains(iBin))
                                iBin = GEX_WAFMAP_PASS_CELL;
                            else if (iBin )
                                iBin = GEX_WAFMAP_FAIL_CELL;
                        }

                        if (iBin == GEX_WAFMAP_PASS_CELL)
                        {
                            // Yes...then compute die location to update on stacked wafermap
                            if(cStackedWaferMapData.cWafMap[i].ldCount != GEX_WAFMAP_EMPTY_CELL)
                                cStackedWaferMapData.cWafMap[i].ldCount++;	// Not first time we increment.
                            else
                                cStackedWaferMapData.cWafMap[i].ldCount = 1;	// was -1 (means not set), now set it to 1 (as it is 1st count)
                        }
                        else if((iBin != GEX_WAFMAP_EMPTY_CELL) && (cStackedWaferMapData.cWafMap[i].ldCount == GEX_WAFMAP_EMPTY_CELL))
                            cStackedWaferMapData.cWafMap[i].ldCount = 0;	// Marks that this is a valid die...even if not a matching bin!

                    }
                    else if (strWafermapBinStacked == "highest_bin")
                    {
                        // Yes...then compute die location to update on stacked wafermap
                        if (iBin != GEX_WAFMAP_EMPTY_CELL)
                        {
                            if(cStackedWaferMapData.cWafMap[i].ldCount != GEX_WAFMAP_EMPTY_CELL)
                                cStackedWaferMapData.cWafMap[i].ldCount = qMax(cStackedWaferMapData.cWafMap[i].ldCount, iBin) ;	// Not first time we increment.
                            else
                                cStackedWaferMapData.cWafMap[i].ldCount = iBin;	// was -1 (means not set), now set it to bin value (as it is 1st count)
                        }
                    }
                    else if (strWafermapBinStacked == "highest_fail_bin")
                    {
                    }
#endif
                }
                else
                {
                    // Parametric test wafermap. Get die value (% in [0-100])

                    if(iBin != GEX_WAFMAP_EMPTY_CELL)
                    {
                        // Compute back real value from %
                        lfValue = pFile->getWaferMapData().getWafMap()[iIndex].getValue(); // lfLow + (iBin*(lfHigh-lfLow)/100.0);

                        cStackedWaferMapData.cWafMap[i].lfParamTotal += lfValue;		// Sum of X
                        cStackedWaferMapData.cWafMap[i].lfParamTotalSquare += (lfValue*lfValue);	// Sum of X*X

                        if (bPassFailMode)
                        {
                            if (cStackedWaferMapData.cWafMap[i].ldCount == GEX_WAFMAP_EMPTY_CELL)
                                cStackedWaferMapData.cWafMap[i].ldCount = 0;

                            if (iBin == GEX_WAFMAP_FAIL_CELL)
                                cStackedWaferMapData.cWafMap[i].ldCount++;			// Number of test results merged
                        }
                        else
                        {
                            if (strWafermapParametricStacked == "mean")
                            {
                                if(cStackedWaferMapData.cWafMap[i].ldCount != GEX_WAFMAP_EMPTY_CELL)
                                    cStackedWaferMapData.cWafMap[i].ldCount++;			// Number of test results merged
                                else
                                    cStackedWaferMapData.cWafMap[i].ldCount = 1;	// was -1 (means not set), now set it to 1 (as it is 1st count)
                            }
                            else
                                vecListValue[i].append(lfValue);
                        }
                    }
                }
            }
        }
    };

    // Step#4: compute highest Bin count on wafermap...
    // If Binning stacked wafermap: Compute the highest Bin count
    if(ptTestCell == NULL || bPassFailMode)
    {
        for(i=0;i< cStackedWaferMapData.SizeX*cStackedWaferMapData.SizeY;i++)
        {
            cStackedWaferMapData.iHighestDieCount = gex_max(cStackedWaferMapData.cWafMap[i].ldCount,cStackedWaferMapData.iHighestDieCount);

            if (strWafermapBinStacked == "pf_pass_if_all_pass")
                //.iWafmapBinStacked == GEX_OPTION_WAFMAP_BIN_STACKED_PASSFAILALL)
            {
                if (cStackedWaferMapData.cWafMap[i].ldCount == cStackedWaferMapData.iTotalWafermaps)
                    cStackedWaferMapData.cWafMap[i].lStatus = GEX_WAFMAP_PASS_CELL;
                else if (cStackedWaferMapData.cWafMap[i].ldCount != GEX_WAFMAP_EMPTY_CELL)
                    cStackedWaferMapData.cWafMap[i].lStatus = GEX_WAFMAP_FAIL_CELL;
            }
        }
    }

    // Back to front of the file list
    itFilesList.toFront();

    while(itFilesList.hasNext())
    {
        pFile = itFilesList.next();

        if(pFile->getWaferMapData().getWafMap() != NULL)
        {
            // Keep track of total testing time.
            int	iTestDuration = pFile->getWaferMapData().lWaferEndTime - pFile->getWaferMapData().lWaferStartTime;
            if(iTestDuration > 0)
                cStackedWaferMapData.iTotaltime += iTestDuration;

            if(pFile->getWaferMapData().bPartialWaferMap)
                cStackedWaferMapData.bPartialWaferMap = true;
        }
    };

    // Step#5: If Parametric wafer map, compute 'Mean' value for each stacked die.
    if(ptTestCell != NULL && !bPassFailMode)
    {
        double	lfWindowSize = cStackedWaferMapData.lfHighWindow-cStackedWaferMapData.lfLowWindow;

        if(lfWindowSize == 0)
            lfWindowSize = 1e-50;

        for(i=0; i< cStackedWaferMapData.SizeX*cStackedWaferMapData.SizeY; i++)
        {
            if (strWafermapParametricStacked == "mean")
            {
                double	lfMean;

                // If parametric wafermap, compute 'Mean' value per each stacked die.
                if(cStackedWaferMapData.cWafMap[i].ldCount > 0)
                {
                    lfMean = cStackedWaferMapData.cWafMap[i].lfParamTotal / cStackedWaferMapData.cWafMap[i].ldCount;
                    cStackedWaferMapData.cWafMap[i].dValue	= lfMean;

                    lfMean = 100.0*(lfMean - cStackedWaferMapData.lfLowWindow)/(lfWindowSize);
                    cStackedWaferMapData.cWafMap[i].ldCount = (int) lfMean;	// [0-100%]
                }
            }
            else
            {
                double	lfMedian;

                if (vecListValue[i].count() > 0)
                {
                    qSort(vecListValue[i].begin(), vecListValue[i].end());

                    lfMedian = algorithms::gexMedianValue(vecListValue[i]);

                    cStackedWaferMapData.cWafMap[i].dValue	= lfMedian;

                    lfMedian = 100.0*(lfMedian - cStackedWaferMapData.lfLowWindow) / (lfWindowSize);
                    cStackedWaferMapData.cWafMap[i].ldCount = (int) lfMedian;	// [0-100%]
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
int CGexReport::getDieTestingSite(int iGroup,int iFile,int iDieX,int iDieY)
{
    // Check if scan over one specific group or all groups!
    int	iFirstGroup,iLastGroup,iSite;
    if(iGroup >= 0)
    {
        iFirstGroup = iGroup;	// Only one group to focus on
        iLastGroup = iGroup + 1;
    }
    else
    {
        iFirstGroup=0;
        iLastGroup = getGroupsList().count();
    }

    CGexGroupOfFiles *pGroup;
    CGexFileInGroup *pFile;
    for(iGroup = iFirstGroup; iGroup < iLastGroup; iGroup++)
    {
        // Get pointer to relevant group & file
        pGroup = (iGroup >= getGroupsList().size()) ? NULL : getGroupsList().at(iGroup);
        if(!pGroup)
            return -1;

        if ((iFile < 0) || (iFile >= pGroup->pFilesList.size()))
            pFile = NULL;
        else
            pFile = pGroup->pFilesList.at(iFile);
        if(!pFile)
            return -1;

        iSite = getDieTestingSite(pGroup,pFile,iDieX,iDieY);
        if(iSite >= 0)
            return iSite;	// Site found, no more search to do!
    }

    return -1;
}

QString CGexReport::getDiePartId(int groupId, int coordX, int coordY)
{
    // Check if scan over one specific group or all groups!
    int     lFirstGroup;
    int     lLastGroup;
    QString lPartId = "?";

    if (groupId >= getGroupsList().count())
    {
        return "?";
    }
    else if (groupId >= 0)
    {
        lFirstGroup = groupId;	// Only one group to focus on
        lLastGroup  = groupId + 1;
    }
    else
    {
        lFirstGroup = 0;
        lLastGroup  = getGroupsList().count();
    }

    CGexGroupOfFiles * lGroup = NULL;

    for(groupId = lFirstGroup; groupId < lLastGroup; groupId++)
    {
        // Get pointer to relevant group & file
        lGroup = getGroupsList().at(groupId);

        if (lGroup->FindPartId(coordX, coordY, lPartId))
            return lPartId;
    }

    return lPartId;
}


///////////////////////////////////////////////////////////
// Returns testing site used to test a given die location
///////////////////////////////////////////////////////////
int CGexReport::getDieTestingSite(CGexGroupOfFiles *pGroup,CGexFileInGroup *pFile,int iDieX,int iDieY)
{
    // Validity checks.
    if(!pGroup)
        return -1;
    if(!pFile)
        return -1;

    CTest	*ptCellDieX;
    CTest	*ptCellDieY;
    CTest	*ptCellSiteID;
    if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEX,GEX_PTEST,&ptCellDieX,true,false) != 1)
        return -1;
    if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEY,GEX_PTEST,&ptCellDieY,true,false) != 1)
        return -1;
    if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_TESTING_SITE,GEX_PTEST,&ptCellSiteID,true,false) != 1)
        return -1;

    // Find (DieX,DieY) location
    int	iIndex;
    int nSamplesExecs;
    nSamplesExecs = gex_min(ptCellDieX->m_testResult.count(),
                            ptCellDieY->m_testResult.count());

    for(iIndex=0; iIndex < nSamplesExecs; iIndex++)
    {
        if((ptCellDieX->m_testResult.resultAt(iIndex) == (double) iDieX) &&	(ptCellDieY->m_testResult.resultAt(iIndex) == (double) iDieY))
            return (int) ptCellSiteID->m_testResult.resultAt(iIndex);
    }

    // Die location not found
    return -1;
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void CGexReport::FillWaferMap(int iGroup,
                              int iFile,
                              CTest *ptTestCell,
                              int iWaferType,
                              bool bSetOrgBin,
                              double dLowValueEx/*=GEX_C_DOUBLE_NAN*/,
                              double dHighValueEx/*=GEX_C_DOUBLE_NAN*/)
{
    // Get pointer to relevant group & file
    CGexGroupOfFiles *pGroup = (iGroup < 0 || iGroup >= getGroupsList().size()) ? NULL : getGroupsList().at(iGroup);
    CGexFileInGroup *pFile;
    if ((iFile < 0) || (iFile >= pGroup->pFilesList.size()))
        pFile = NULL;
    else
        pFile = pGroup->pFilesList.at(iFile);
    FillWaferMap(pGroup,pFile,ptTestCell,iWaferType,bSetOrgBin,dLowValueEx,dHighValueEx);
}

///////////////////////////////////////////////////////////
// Fill (group,file) wafermap with given parameter
///////////////////////////////////////////////////////////
void CGexReport::FillWaferMap(CGexGroupOfFiles *pGroup,CGexFileInGroup *pFile,
                              CTest *ptTestCell,int iWaferType, bool bSetOrgBin, double dLowValueEx, double dHighValueEx)
{
    // Check for valid pointers.
    if(pGroup == NULL)
        return;
    if(pFile == NULL)
        return;

    // IF not test defined, use the Soft-BIN parameter or Hard-BIN.
    if(ptTestCell == NULL)
    {
        switch(iWaferType)
        {
            default:
            case GEX_WAFMAP_STACK_SOFTBIN:
            case GEX_WAFMAP_ZONAL_SOFTBIN:
            case GEX_WAFMAP_SOFTBIN:
                // Loading SOFT-BIN
                if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_SBIN,GEX_PTEST,&ptTestCell,true,false) != 1)
                    return;
                break;
            case GEX_WAFMAP_STACK_HARDBIN:
            case GEX_WAFMAP_HARDBIN:
            case GEX_WAFMAP_ZONAL_HARDBIN:
                // Loading HARD-BIN
                if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_HBIN,GEX_PTEST,&ptTestCell,true,false) != 1)
                    return;
                break;
        }
    }

    // Get 1st run# offset in results array (in case multiple files merged)
    int iStartOffset	= 0;
    int iEndOffset		= 0;

    ptTestCell->findSublotOffset(iStartOffset, iEndOffset, pFile->lFileID);

    // Prefill wafer with empty cells
    if (pFile->getWaferMapData().SizeX == 0 || pFile->getWaferMapData().SizeY == 0)
        return;
    int	iIndex;
    for(iIndex=0;iIndex < pFile->getWaferMapData().SizeX*pFile->getWaferMapData().SizeY;iIndex++)
    {
        pFile->getWaferMapData().getWafMap()[iIndex].setBin( GEX_WAFMAP_EMPTY_CELL);
        pFile->getWaferMapData().getWafMap()[iIndex].setValue( GEX_C_DOUBLE_NAN);
    }

    // Save the testnumber used
    pFile->getWaferMapData().lTestNumber = ptTestCell->lTestNumber;

    if (ptTestCell->m_testResult.count() > 0)
    {
        // Get handle to DieX and DieY parameters
        CTest * ptCellDieX;
        CTest * ptCellDieY;
        CTest * ptSoftBin;
        CTest * ptHardBin;
        CTest * ptPartID;

        if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEX,GEX_PTEST,&ptCellDieX,true,false) != 1)
            return;
        if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEY,GEX_PTEST,&ptCellDieY,true,false) != 1)
            return;
        if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_SBIN,GEX_PTEST,&ptSoftBin,true,false) != 1)
            return;
        if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_HBIN,GEX_PTEST,&ptHardBin,true,false) != 1)
            return;

        if (pFile->getWaferMapData().bStripMap && pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_PARTID,GEX_PTEST,&ptPartID,true,false) != 1)
            return;

        // Check if special retest Bin processing 'Promote' mode enabled?
        // 'Promote' mode: means keep highest Bin# over retest instances.
        bool	bPromoteHighestBin=false;
        switch(iWaferType)
        {
            case GEX_WAFMAP_SOFTBIN:		// Packaged: SOFT BIN
            case GEX_WAFMAP_HARDBIN:		// Packaged: HARD BIN
                bPromoteHighestBin = (m_pReportOptions->GetOption("wafer","retest_policy").toString()=="highest_bin");
                break;
        }

        // Compute Values space
        double	lLowValue, lHighValue, lfValueSpace;
        lLowValue = 0; lHighValue = 0; lfValueSpace = 0;

        computeValuesSpaceWafermap(ptTestCell, dLowValueEx, dHighValueEx, iWaferType, lLowValue, lHighValue, lfValueSpace);

        // Fill tested dies with tested dies for the given parameter
        int		iSampleOffset,iDieX,iDieY,iCode;
        long	lPartID;
        double	lfValue;

        if( (ptTestCell->m_testResult.count() < iEndOffset)         ||
                (ptCellDieX->m_testResult.count() < iEndOffset)      ||
                (ptCellDieY->m_testResult.count() < iEndOffset)      )
        {
            GEX_ASSERT(false);
            iEndOffset = iStartOffset;  // has lists are not synchronized, be sure that we don't try to do something.
        }

        for(iSampleOffset = iStartOffset; iSampleOffset < iEndOffset; iSampleOffset++)
        {
            if(ptTestCell->m_testResult.isValidResultAt(iSampleOffset))
            {
                lfValue = ptTestCell->m_testResult.resultAt(iSampleOffset);

                if( (ptCellDieX->m_testResult.isValidResultAt(iSampleOffset))       &&
                        (ptCellDieY->m_testResult.isValidResultAt(iSampleOffset))   )
                {
                    // Compute associated DieX,Y coordinates
                    iDieX = (int) ptCellDieX->m_testResult.resultAt(iSampleOffset);
                    iDieY = (int) ptCellDieY->m_testResult.resultAt(iSampleOffset);
                }
                else
                {
                    if(pFile->getWaferMapData().bStripMap)
                    {
                      if (ptPartID->m_testResult.isValidIndex(iSampleOffset))
                        lPartID = static_cast<long>(
                          ptPartID->m_testResult.resultAt(iSampleOffset));
                      else
                        {
                            GEX_ASSERT(false);
                            lPartID = (long) GEX_C_DOUBLE_NAN;
                        }
                        iDieX	= lPartID % pFile->getWaferMapData().SizeX;
                        iDieY	= pFile->getWaferMapData().iHighDieY - (lPartID / pFile->getWaferMapData().SizeX);
                    }
                    else
                    {
                        GSLOG(SYSLOG_SEV_NOTICE,
                              QString("Invalid DieX and/or DieY").toLatin1().constData());
                        continue;
                    }
                }

                // evaluate and check index
                iIndex = (iDieX-pFile->getWaferMapData().iLowDieX) + (iDieY-pFile->getWaferMapData().iLowDieY)*pFile->getWaferMapData().SizeX;
                if(iIndex < 0 || iIndex >pFile->getWaferMapData().SizeX*pFile->getWaferMapData().SizeY)
                {
                    GSLOG(SYSLOG_SEV_NOTICE,
                          QString("Invalid Index").toLatin1().constData());
                    continue;
                }

                switch(iWaferType)
                {
                case GEX_WAFMAP_SOFTBIN:		// Packaged: SOFT BIN
                case GEX_WAFMAP_ZONAL_SOFTBIN:
                case GEX_WAFMAP_STACK_SOFTBIN:
                    if(ptSoftBin->m_testResult.isValidIndex(iSampleOffset))
                        iCode = (int) ptSoftBin->m_testResult.resultAt(iSampleOffset);
                    else
                    {
                        GEX_ASSERT(false);
                        iCode = (int) GEX_C_DOUBLE_NAN;
                    }
                    break;

                case GEX_WAFMAP_HARDBIN:		// Packaged: HARD BIN
                case GEX_WAFMAP_STACK_HARDBIN:
                case GEX_WAFMAP_ZONAL_HARDBIN:
                    if(ptHardBin->m_testResult.isValidIndex(iSampleOffset))
                        iCode = (int) ptHardBin->m_testResult.resultAt(iSampleOffset);
                    else
                    {
                        GEX_ASSERT(false);
                        iCode = (int) GEX_C_DOUBLE_NAN;
                    }
                    break;

                case GEX_WAFMAP_TESTOVERLIMITS:  // Zonnig on limits
                case GEX_WAFMAP_STACK_TESTOVERLIMITS:
                case GEX_WAFMAP_TESTOVERDATA:  // Zoning on test values range
                case GEX_WAFMAP_STACK_TESTOVERDATA:
                    lfValue = lfValue*ScalingPower(ptTestCell->res_scal);

                    if(lfValueSpace != 0)
                    {
                        // To avoid some issue concerning the rounding of some value
                        if ((lfValue >= lLowValue || lLowValue <= ptTestCell->lfSamplesMin) && (lfValue <= lHighValue || lHighValue >= ptTestCell->lfSamplesMax))
                            iCode = (int) (100.0*(lfValue-lLowValue)/lfValueSpace);
                        else
                            iCode = GEX_WAFMAP_EMPTY_CELL;
                    }
                    else
                        iCode = 0;

                    break;

                case GEX_WAFMAP_TEST_PASSFAIL:
                case GEX_WAFMAP_STACK_TEST_PASSFAIL:
                    lfValue = lfValue*ScalingPower(ptTestCell->res_scal);

                    if (ptTestCell->bTestType == 'P' || ptTestCell->bTestType == 'F')
                    {
                        if (ptTestCell->isFailingValue(lfValue, ptTestCell->m_testResult.passFailStatus(iSampleOffset)))
                            iCode = GEX_WAFMAP_FAIL_CELL;
                        else
                            iCode = GEX_WAFMAP_PASS_CELL;
                    }
                    else
                    {
                        if(lfValueSpace != 0)
                        {
                            // To avoid some issue concerning the rounding of some value
                            if ((lfValue >= lLowValue || lLowValue <= ptTestCell->lfSamplesMin) && (lfValue <= lHighValue || lHighValue >= ptTestCell->lfSamplesMax))
                                iCode = (int) (100.0*(lfValue-lLowValue)/lfValueSpace);
                            else
                                iCode = GEX_WAFMAP_EMPTY_CELL;
                        }
                        else
                            iCode = 0;
                    }
                    break;

                default:
                    iCode = GEX_WAFMAP_EMPTY_CELL;
                }
                // In all cases (retest or not), save bin result...but if retest, check the retest-policy!
                if(bPromoteHighestBin)
                {
                    // Promote the highest bin value.
                    if(pFile->getWaferMapData().getWafMap()[iIndex].getBin() != GEX_WAFMAP_EMPTY_CELL)
                    {
                        if (iCode > pFile->getWaferMapData().getWafMap()[iIndex].getBin())
                        {
                            pFile->getWaferMapData().getWafMap()[iIndex].setBin( iCode);
                            pFile->getWaferMapData().getWafMap()[iIndex].setValue( lfValue);

                            // Set Original bin value to?
                            if(bSetOrgBin)
                                pFile->getWaferMapData().getWafMap()[iIndex].setOrgBin( iCode);
                        }
                    }
                    else
                    {
                        pFile->getWaferMapData().getWafMap()[iIndex].setBin( iCode);
                        pFile->getWaferMapData().getWafMap()[iIndex].setValue(	lfValue);

                        // Set Original bin value to?
                        if(bSetOrgBin)
                            pFile->getWaferMapData().getWafMap()[iIndex].setOrgBin (iCode);
                    }
                }
                else
                {
                    // Standard retest policy: keep the latest Bin result
                    pFile->getWaferMapData().getWafMap()[iIndex].setBin( iCode);
                    pFile->getWaferMapData().getWafMap()[iIndex].setValue( lfValue);

                    // Set Original bin value to?
                    if(bSetOrgBin)
                        pFile->getWaferMapData().getWafMap()[iIndex].setOrgBin(iCode);
                }
            }
        }

        // Check for Swap-die action (set in 'Options')
        pFile->SwapWaferMapDies();
    }
    else
        GSLOG(SYSLOG_SEV_WARNING, "No test result in this test cell");
}

void CGexReport::computeValuesSpaceWafermap(
  CTest * ptTestCell, double dLowValueEx, double dHighValueEx, int iWaferType,
  double& dLowValue, double& dHighValue, double& dSpaceValue)
{
    switch (iWaferType)
    {
        case GEX_WAFMAP_TESTOVERLIMITS:			// Zonnig on limits
        case GEX_WAFMAP_STACK_TESTOVERLIMITS:
            {
                // Compute limits space
                if (dLowValueEx != GEX_C_DOUBLE_NAN)
                    dLowValue = dLowValueEx;
                else if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
                    dLowValue = ptTestCell->GetCurrentLimitItem()->lfLowLimit; // We have a LOW limit.
                else
                    dLowValue = ptTestCell->lfMin;		 // NO LOW limit...use Min value instead.

                if (dHighValueEx != GEX_C_DOUBLE_NAN)
                    dHighValue = dHighValueEx;
                else if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
                    dHighValue = ptTestCell->GetCurrentLimitItem()->lfHighLimit; // We have a HIGH limit.
                else
                    dHighValue = ptTestCell->lfMax;		// NO HIGH limit...use Max value instead.

                dSpaceValue = dHighValue - dLowValue;
            }
            break;

        case GEX_WAFMAP_TESTOVERDATA:			// Zoning on test values range
        case GEX_WAFMAP_STACK_TESTOVERDATA:
            {
                // Compute range space
                if (dLowValueEx != GEX_C_DOUBLE_NAN)
                    dLowValue = dLowValueEx;
                else
                    dLowValue = ptTestCell->lfSamplesMin;

                if (dHighValueEx != GEX_C_DOUBLE_NAN)
                    dHighValue = dHighValueEx;
                else
                    dHighValue = ptTestCell->lfSamplesMax;

                if (dHighValueEx != GEX_C_DOUBLE_NAN || dLowValueEx != GEX_C_DOUBLE_NAN)
                    dSpaceValue = dHighValue - dLowValue;
                else
                    dSpaceValue = ptTestCell->lfRange;
            }
            break;

        default:
            break;

    }
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
int CGexReport::SelectWaferMapDies(CGexGroupOfFiles *pGroup,
                                   CGexFileInGroup *pFile,
                                   CTest *ptTestCell,double lfLow,double lfHigh)
{
    // Check for valid pointers.
    if(pGroup == NULL)
        return -1;
    if(pFile == NULL)
        return -1;

    // Get 1st run# offset in results array (in case multiple files merged)
    int iStartOffset	= 0;
    int iEndOffset		= 0;

    ptTestCell->findSublotOffset(iStartOffset, iEndOffset, pFile->lFileID);

    // Get handle to DieX and DieY parameters
    CTest	*ptCellDieX;
    CTest	*ptCellDieY;
    if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEX,GEX_PTEST,&ptCellDieX,true,false) != 1)
        return -1;
    if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEY,GEX_PTEST,&ptCellDieY,true,false) != 1)
        return -1;

    // Fill tested dies with tested dies for the given parameter
    int		iSampleOffset,iDieX,iDieY;
    double	lfValue;
    int		iTotalMatch=0;
    if( (ptTestCell->m_testResult.count() < iEndOffset)
            || (ptCellDieX->m_testResult.count() < iEndOffset)
            || (ptCellDieY->m_testResult.count() < iEndOffset))
    {
        GEX_ASSERT(false);
        iEndOffset = iStartOffset;      // lists are not well synchronized
    }

    for(iSampleOffset = iStartOffset; iSampleOffset < iEndOffset; iSampleOffset++)
    {
        if(ptTestCell->m_testResult.isValidResultAt(iSampleOffset))
        {
            lfValue = ptTestCell->m_testResult.resultAt(iSampleOffset);

            if((lfValue >= lfLow) && (lfValue <= lfHigh))   // TODO ? &&(ptCellDieX->m_testResult.isValidResultAt(iSampleOffset)&&(ptCellDieY->m_testResult.isValidResultAt(iSampleOffset)
            {
                // Compute associated DieX,Y coordinates
                iDieX = (int) ptCellDieX->m_testResult.resultAt(iSampleOffset);
                iDieY = (int) ptCellDieY->m_testResult.resultAt(iSampleOffset);

                // Highlight relevant die
                pGexMainWindow->LastCreatedWizardWafer3D()->SelectDieLocation(pFile,iDieX,iDieY,(iTotalMatch == 0));

                // Keep track of total dies matching criteria
                iTotalMatch++;
            }
        }
    }

    // Return die count matching criteria
    return iTotalMatch;
}

/////////////////////////////////////////////////////////////////////////////
// Writes the Compare wafermap (2 groups)
/////////////////////////////////////////////////////////////////////////////
bool	CGexReport::WriteCompareWaferMap(BOOL bWriteReport)
{
    // Report must not be created yet...then quietly return
    if(bWriteReport == false)
        return false;

    // If PAT-Man report built, we may have multiple groups but it is testing sites for a same file!
    if(m_pReportOptions->getAdvancedReport() == GEX_ADV_OUTLIER_REMOVAL)
        return false;

    // We need two groups to compare!
    if((getGroupsList().count() != 2))
        return false;

    // Make sure Wafermap option is 'Software binning' or 'Hardware binning'
    if((m_pReportOptions->iWafermapType != GEX_WAFMAP_SOFTBIN) && (m_pReportOptions->iWafermapType != GEX_WAFMAP_HARDBIN))
        return true;

    CGexGroupOfFiles *	pGroup						= NULL;
    CGexFileInGroup *	pFile						= NULL;
    CWaferMap *			ptWaferMapData1				= NULL;				// Wafermap data resulting from STDF file analysis.
    CWaferMap *			ptWaferMapData2				= NULL;				// Wafermap data resulting from STDF file analysis.
    CBinning *			pBinning1					= NULL;
    CBinning *			pBinning2					= NULL;
    CStackedWaferMap *	pStackedWafermap			= NULL;
    CStackedWaferMap *	pDieMismatchWaferMapData	= NULL;

    // Rebuild the stacked wafermap
    pGroup	= (getGroupsList().isEmpty()) ? NULL : getGroupsList().at(0);
    pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

    pGroup->BuildStackedWaferMap(m_pReportOptions);

    pStackedWafermap			= &pGroup->cStackedWaferMapData;
    pDieMismatchWaferMapData	= &pGroup->cDieMismatchWaferMapData;
    ptWaferMapData1				= &pFile->getWaferMapData();

    if(ptWaferMapData1->bWaferMapExists != true)
        return false;

    switch(m_pReportOptions->iWafermapType)
    {
        case GEX_WAFMAP_SOFTBIN:
            pBinning1 = pGroup->cMergedData.ptMergedSoftBinList;
            break;

        case GEX_WAFMAP_HARDBIN:
            pBinning1 = pGroup->cMergedData.ptMergedHardBinList;
            break;
    }

    // check Group2...n: for any condition that may invalidate the need of creating the Mismatch wafermap.
    pGroup = (getGroupsList().size() < 2) ? NULL : getGroupsList().at(1);
    // Rebuild the stacked wafermap
    pGroup->BuildStackedWaferMap(m_pReportOptions);

    pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
    if(pGroup->pFilesList.count() != 1)
        return false;

    pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
    ptWaferMapData2 = &pFile->getWaferMapData();
    if(ptWaferMapData2->bWaferMapExists != true)
        return false;
    switch(m_pReportOptions->iWafermapType)
    {
        case GEX_WAFMAP_SOFTBIN:
            pBinning2 = pGroup->cMergedData.ptMergedSoftBinList;
            break;

        case GEX_WAFMAP_HARDBIN:
            pBinning2 = pGroup->cMergedData.ptMergedHardBinList;
            break;
    }

    QString			of						= m_pReportOptions->GetOption("output", "format").toString();
    QStringList		strLstWafermapCompare	= m_pReportOptions->GetOption("wafer", "compare").toString().split("|");
    bool			bDieMismatchTable		= strLstWafermapCompare.contains("diemismatch_table");
    bool			bDeltaYield_Section		= strLstWafermapCompare.contains("deltayield_section");

    // So we've got two groups with one wafermap each...then compare them if this is possible...
    //if (of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
    if(m_pReportOptions->isReportOutputHtmlBased())
        WriteHtmlSectionTitle(hReportFile,"wafmap_mismatch","Wafer map: Compare");
    else
    if (of=="CSV")
        fprintf(hReportFile,"\nWafer map: Compare\n\n");

    // Check if wafermap sizes are identical.
    if((strLstWafermapCompare.contains("any_size") == false) && ((ptWaferMapData1->SizeX != ptWaferMapData2->SizeX) || (ptWaferMapData1->SizeY != ptWaferMapData2->SizeY)))
    {
        // If size mismatch, tell it in the report...
        if(bWriteReport)
            fprintf(hReportFile,"Warning: Wafers do not have a matching size. To force comparing them, check 'Options' tab, section 'wafermap/Comparing wafers'\n");

        // Section created
        return true;
    }

    // We have Two wafermaps to compare and they have the same size...then do it!
    if(GS::LPPlugin::ProductInfo::getInstance()->isExaminator()
        || GS::LPPlugin::ProductInfo::getInstance()->isOEM()
        || GS::LPPlugin::ProductInfo::getInstance()->isMonitoring()
       )
    {
        fprintf(hReportFile,"Warning: Comparing Wafermaps is not available under this license.<br>\n");
        fprintf(hReportFile, "%s", ReportOptions.GetOption("messages", "upgrade").toString().toLatin1().data() );
            return true;
    }

    pDieMismatchWaferMapData->SizeX		= pStackedWafermap->SizeX;
    pDieMismatchWaferMapData->SizeY		= pStackedWafermap->SizeY;
    pDieMismatchWaferMapData->iLowDieX	= pStackedWafermap->iLowDieX;
    pDieMismatchWaferMapData->iLowDieY	= pStackedWafermap->iLowDieY;
    pDieMismatchWaferMapData->iHighDieX	= pStackedWafermap->iHighDieX;
    pDieMismatchWaferMapData->iHighDieY	= pStackedWafermap->iHighDieY;

    long iStackedSize = pDieMismatchWaferMapData->SizeX * pDieMismatchWaferMapData->SizeY;

    pDieMismatchWaferMapData->bWaferMapExists = true;
    pDieMismatchWaferMapData->cWafMap = new CStackedWafMapArray[iStackedSize];
    for( int nIndex = 0; nIndex < pDieMismatchWaferMapData->SizeX * pDieMismatchWaferMapData->SizeY; nIndex++)
    {
        pDieMismatchWaferMapData->cWafMap[nIndex].ldCount			= GEX_WAFMAP_EMPTY_CELL;	// Number of tests or bins merged for this die..
        pDieMismatchWaferMapData->cWafMap[nIndex].lStatus			= GEX_WAFMAP_EMPTY_CELL;    // Die status
        pDieMismatchWaferMapData->cWafMap[nIndex].nBinToBinDie		= 0;						//
        pDieMismatchWaferMapData->cWafMap[nIndex].lfParamTotal		= 0.0;						// Sum of X
        pDieMismatchWaferMapData->cWafMap[nIndex].lfParamTotalSquare	= 0.0;					// Sum of X*X
    }

    // Get wafermap to show
    QStringList strLstWafermapToShow	= m_pReportOptions->GetOption("wafer", "chart_show").toString().split("|");
    bool		bBinToBin				= strLstWafermapToShow.contains("bin_to_bin");

    // Build wafermap showing dies not matching on all wafermaps, and display it (HTML report type only)
    pDieMismatchWaferMapData->iTotalWafermaps = 1;

    int				iIndex;
    int				iDieX,iDieY;
    int				iDieIndexWafer1,iDieIndexWafer2;
    unsigned int	uiTotalGoodWafer1 = 0, uiTotalGoodWafer2 = 0, uiCommonDies = 0;
    CBinning		*ptBinCell1, *ptBinCell2;

    // Compute total good dies for both wafers
    for(iIndex = 0; iIndex < ptWaferMapData1->SizeX*ptWaferMapData1->SizeY; iIndex++)
    {
        if(ptWaferMapData1->getWafMap()[iIndex].getBin() != GEX_WAFMAP_EMPTY_CELL)
        {
            ptBinCell1 = findBinningCell(pBinning1,ptWaferMapData1->getWafMap()[iIndex].getBin());

            if(ptBinCell1 && (ptBinCell1->cPassFail == 'P'))
                uiTotalGoodWafer1++;
        }
    }
    for(iIndex = 0; iIndex < ptWaferMapData2->SizeX*ptWaferMapData2->SizeY; iIndex++)
    {
        if(ptWaferMapData2->getWafMap()[iIndex].getBin() != GEX_WAFMAP_EMPTY_CELL)
        {
            ptBinCell2 = findBinningCell(pBinning2,ptWaferMapData2->getWafMap()[iIndex].getBin());
            if(ptBinCell2 && (ptBinCell2->cPassFail == 'P'))
                uiTotalGoodWafer2++;
        }
    }

    // Clear the array that will keep track of total matching bins.
    for(iIndex = 0; iIndex < ptWaferMapData1->SizeX*ptWaferMapData1->SizeY; iIndex++)
        pDieMismatchWaferMapData->cWafMap[iIndex].ldCount = GEX_WAFMAP_EMPTY_CELL;

    // Compare wafer of group 0 with group 1
    pGroup = (getGroupsList().size() < 2) ? NULL : getGroupsList().at(1);
    pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
    ptWaferMapData2 = &pFile->getWaferMapData();
    if(ptWaferMapData2->bWaferMapExists != true)
        goto skip_compare;

    for(iIndex = 0; iIndex < ptWaferMapData1->SizeX*ptWaferMapData1->SizeY; iIndex++)
    {
        // Compute die location
        iDieX = ptWaferMapData1->iLowDieX + (iIndex % ptWaferMapData1->SizeX);
        iDieY = ptWaferMapData1->iLowDieY + (iIndex / ptWaferMapData1->SizeX);
        iDieIndexWafer1 = iIndex;

        // Check if this die location is within Wafer#2 area....
        if(iDieX >= ptWaferMapData2->iLowDieX && iDieX <= ptWaferMapData2->iHighDieX &&
            iDieY >= ptWaferMapData2->iLowDieY && iDieY <= ptWaferMapData2->iHighDieY)
        {
            // Compute die position in wafer#2 array...
            iDieIndexWafer2 = (iDieX-ptWaferMapData2->iLowDieX) + ((iDieY-ptWaferMapData2->iLowDieY)*ptWaferMapData2->SizeX);
        }
        else
            iDieIndexWafer2 = -1;	// This die in wafer#1 is outside of wafer~2 area covered.

        // Only check die location tested on all wafers.
        if((iDieIndexWafer2 >= 0) &&
            (ptWaferMapData1->getWafMap()[iDieIndexWafer1].getBin() != GEX_WAFMAP_EMPTY_CELL) &&
            (ptWaferMapData2->getWafMap()[iDieIndexWafer2].getBin() != GEX_WAFMAP_EMPTY_CELL))
        {
            uiCommonDies++;

            if(ptWaferMapData1->getWafMap()[iDieIndexWafer1].getBin() != ptWaferMapData2->getWafMap()[iDieIndexWafer2].getBin())
                pDieMismatchWaferMapData->cWafMap[iDieIndexWafer1].ldCount = 1;	// Different binning
            else
                pDieMismatchWaferMapData->cWafMap[iDieIndexWafer1].ldCount = 0;	// Same binning

            if (bBinToBin)
                pDieMismatchWaferMapData->cWafMap[iDieIndexWafer1].nBinToBinDie = (ptWaferMapData1->getWafMap()[iDieIndexWafer1].getBin() << 16) | (ptWaferMapData2->getWafMap()[iDieIndexWafer2].getBin() & 0x00FF);
        }
    }

skip_compare:;

    // Check if the wafers to compare have some common dies
    if(uiCommonDies == 0)
    {
        if (of=="CSV")
            fprintf(hReportFile,"WARNING: wafers have no common dies\n\n");
        else
        //if (of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
        if(m_pReportOptions->isReportOutputHtmlBased())
            fprintf(hReportFile,"<p><font color=\"#FF0000\"><b>WARNING: wafers have no common dies!!</b></font></p>\n");
        return true;
    }

    // Wafers have some common dies: generate compare sections
    QString		strImageName ;
    QString		strImageFile;

    pGroup = getGroupsList().isEmpty()?NULL:getGroupsList().first();
    pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

    //if (of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
    if(m_pReportOptions->isReportOutputHtmlBased())
    {
        if (strLstWafermapToShow.contains("bin_mismatch"))
        {
            // Create image from the stacked wafermap array just created and write it into HTML report.
            strImageName = "wafcmp.png";
            strImageFile = m_pReportOptions->strReportDirectory;
            strImageFile += "/images/";
            strImageFile += strImageName;
            fprintf(hReportFile,"<h2><font color=\"#006699\">Mismatch wafermap</font></h2>\n");
            fprintf(hReportFile,"<p>(Mismatching dies appear in <font color=\"#FF0000\"><b>Red</b></font>)</p>\n");

            // Show hyperlink to export compare-wafermap data
            QString	strDrillArgument;
            QString strExportWafer;
            strDrillArgument= "drill_3d=wafer";
            strDrillArgument += "--g=";
            strDrillArgument += QString::number(0);	// GroupID (0=1st group, etc...)
            strDrillArgument += "--f=";
            strDrillArgument += QString::number(-1);	// FileID = -1, means 'stacked wafer' structure to dump.
            strDrillArgument += "--stacked=DieMismatchBin";
            strExportWafer = "#_gex_export_wafmap.htm#" + strDrillArgument;
            WriteHtmlToolBar(GetWaferSizeRequested(GetWaferImageSize()),false,strDrillArgument,
                "Export to file","../images/save.png",strExportWafer);

            CreateWaferMapImage(CGexReport::compareBinMismatch, pGroup, pFile, bWriteReport,
                                strImageFile.toLatin1().constData(), strImageName.toLatin1().constData());
        }

        if (strLstWafermapToShow.contains("bin_to_bin"))
        {
            // Create image from the stacked wafermap array just created and write it into HTML report.
            strImageName = "wafcmpbintobin.png";

            strImageFile = m_pReportOptions->strReportDirectory;
            strImageFile += "/images/";
            strImageFile += strImageName;
            fprintf(hReportFile,"<h2><font color=\"#006699\">Bin to bin color correlation wafermap</font></h2>\n");

            WriteBinToBinColorCorrelationLegend();

            // Show hyperlink to export compare-wafermap data
            QString	strDrillArgument;
            QString strExportWafer;
            strDrillArgument= "drill_3d=wafer";
            strDrillArgument += "--g=";
            strDrillArgument += QString::number(0);	// GroupID (0=1st group, etc...)
            strDrillArgument += "--f=";
            strDrillArgument += QString::number(-1);	// FileID = -1, means 'stacked wafer' structure to dump.
            strDrillArgument += "--stacked=DieMismatchBinToBin";
            strExportWafer = "#_gex_export_wafmap.htm#" + strDrillArgument;

            WriteHtmlToolBar(GetWaferSizeRequested(GetWaferImageSize()),false,strDrillArgument,
                "Export to file","../images/save.png",strExportWafer);

            CreateWaferMapImage(CGexReport::compareBinToBinCorrelation, pGroup, pFile, bWriteReport,
                                strImageFile.toLatin1().constData(), strImageName.toLatin1().constData());
        }
    }
    else
    {
        // CSV Report
        // Bin to bin color correlation wfaermap are not included in the csv report

        // Export bin mismatch wafermap for csv report
        if (strLstWafermapToShow.contains("bin_mismatch"))
            CreateWaferMapImage(CGexReport::compareBinMismatch, pGroup, pFile, bWriteReport,
                                strImageFile.toLatin1().constData(), strImageName.toLatin1().constData());
    }

    //if (of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
    if(m_pReportOptions->isReportOutputHtmlBased())
        fprintf(hReportFile,"<br>\n");

    // Make sure at least one optional section is enabled
    if (bDieMismatchTable == false && bDeltaYield_Section == false)
        return true;

    // Check all common dies:
    // o fill mappings for binning pareto (mismatch, P->F, F->P) for delta yield pareto tabled
    // o if enabled, display die mismatch table
    QString					strLabel1,strLabel2;
    CWafBinMismatchPareto	clWafBinMismatchPareto_All;
    CWafBinMismatchPareto	clWafBinMismatchPareto_PassToFail;
    CWafBinMismatchPareto	clWafBinMismatchPareto_FailToPass;

    // Build wafer names
    strLabel1 = ptWaferMapData1->szWaferID;
    strLabel2 = ptWaferMapData2->szWaferID;

    pGroup = (getGroupsList().isEmpty()) ? NULL : getGroupsList().at(0);
    if(strLabel1.isEmpty())
    {
        pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
        strLabel1 = pGroup->strGroupName + QString(" - Wafer: ") + QString(pFile->getMirDatas().szSubLot);
    }
    else
        strLabel1 = pGroup->strGroupName + QString(" - Wafer: ") + QString(ptWaferMapData1->szWaferID);

    pGroup = (getGroupsList().size() < 2) ? NULL : getGroupsList().at(1);
    if(strLabel2.isEmpty())
    {
        pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
        strLabel2 = pGroup->strGroupName + QString(" - Wafer: ") + QString(pFile->getMirDatas().szSubLot);
    }
    else
        strLabel2 = pGroup->strGroupName + QString(" - Wafer: ") + QString(ptWaferMapData2->szWaferID);

    // If die mismatch table option enabled, write section header
    if (bDieMismatchTable)
    {
        if (of=="CSV")
            fprintf(hReportFile,"Die mismatch table\n");
        else
        //if (of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
        if(m_pReportOptions->isReportOutputHtmlBased())
            fprintf(hReportFile,"<h2><font color=\"#006699\">Die mismatch table</font></h2>\n");
    }

    // Go through all common dies
    for(iIndex = 0; iIndex < ptWaferMapData1->SizeX*ptWaferMapData1->SizeY; iIndex++)
    {
        // Compute die location for Wafer1

        // Check if binning mismatch at this location
        if(pDieMismatchWaferMapData->cWafMap[iIndex].ldCount == 1)
        {
            // Found a mismatch

            // Compute die locations
            iDieX = ptWaferMapData1->iLowDieX + (iIndex % ptWaferMapData1->SizeX);
            iDieY = ptWaferMapData1->iLowDieY + (iIndex / ptWaferMapData1->SizeX);
            iDieIndexWafer1 = iIndex;
            iDieIndexWafer2 = (iDieX-ptWaferMapData2->iLowDieX) + ((iDieY-ptWaferMapData2->iLowDieY)*ptWaferMapData2->SizeX);

            // Retrieve binning objects for both wafers
            ptBinCell1 = findBinningCell(pBinning1,ptWaferMapData1->getWafMap()[iDieIndexWafer1].getBin());
            ptBinCell2 = findBinningCell(pBinning2,ptWaferMapData2->getWafMap()[iDieIndexWafer2].getBin());

            // If enabled, write to die mismatch table
            if(bDieMismatchTable)
            {
                if(clWafBinMismatchPareto_All.totalCount() == 0)
                {
                    if (of=="CSV")
                    {
                        fprintf(hReportFile,"\n");
                        fprintf(hReportFile,"DieX,DieY,");
                        fprintf(hReportFile,"Binning for Wafer:%s,",strLabel1.toLatin1().constData());
                        fprintf(hReportFile,"Binning for Wafer:%s\n",strLabel2.toLatin1().constData());
                    }
                    else
                    //if (of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
                    if(m_pReportOptions->isReportOutputHtmlBased())
                    {
                        // First mismatch, write table header
                        if (of=="HTML")
                            fprintf(hReportFile,"<table border=\"0\" width=\"98%%\">\n");
                        else
                            WriteHtmlOpenTable(98,0);	// HTML code to open table, 98%, cell spacing=0

                        fprintf(hReportFile,"<tr>\n");
                        fprintf(hReportFile,"<td width=\"11%%\" bgcolor=%s align=\"center\"><b>DieX</b></td>\n",szFieldColor);
                        fprintf(hReportFile,"<td width=\"11%%\" bgcolor=%s align=\"center\"><b>DieY</b></td>\n",szFieldColor);
                        fprintf(hReportFile,"<td width=\"11%%\" bgcolor=%s align=\"center\"><b>Binning<br>%s</b></td>\n",szFieldColor,strLabel1.toLatin1().constData());
                        fprintf(hReportFile,"<td width=\"11%%\" bgcolor=%s align=\"center\"><b>Binning<br>%s</b></td>\n",szFieldColor,strLabel2.toLatin1().constData());
                        fprintf(hReportFile,"<td width=\"12%%\" bgcolor=\"#ffffff\" align=</td>\n"); // White column
                        fprintf(hReportFile,"<td width=\"11%%\" bgcolor=%s align=\"center\"><b>DieX</b></td>\n",szFieldColor);
                        fprintf(hReportFile,"<td width=\"11%%\" bgcolor=%s align=\"center\"><b>DieY</b></td>\n",szFieldColor);
                        fprintf(hReportFile,"<td width=\"11%%\" bgcolor=%s align=\"center\"><b>Binning<br>%s</b></td>\n",szFieldColor,strLabel1.toLatin1().constData());
                        fprintf(hReportFile,"<td width=\"11%%\" bgcolor=%s align=\"center\"><b>Binning<br>%s</b></td>\n",szFieldColor,strLabel2.toLatin1().constData());
                        fprintf(hReportFile,"</tr>\n");
                    }
                }

                if (of=="CSV")
                {
                    fprintf(hReportFile,"%d,", iDieX);
                    fprintf(hReportFile,"%d,", iDieY);
                    if(ptBinCell1 == NULL)
                        fprintf(hReportFile,"-,");
                    else
                    if(ptBinCell1->cPassFail == 'P')
                        fprintf(hReportFile,"%d (P),", ptBinCell1->iBinValue);
                    else
                        fprintf(hReportFile,"%d (F),", ptBinCell1->iBinValue);

                    if(ptBinCell2 == NULL)
                        fprintf(hReportFile,"-,");
                    else
                    if(ptBinCell2->cPassFail == 'P')
                        fprintf(hReportFile,"%d (P)\n", ptBinCell2->iBinValue);
                    else
                        fprintf(hReportFile,"%d (F)\n", ptBinCell2->iBinValue);
                }
                else
                //if (of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
                if(m_pReportOptions->isReportOutputHtmlBased())
                {
                    // If beginning of table line, insert marker
                    if(clWafBinMismatchPareto_All.totalCount() % 2 == 0)
                        fprintf(hReportFile,"<tr>\n");

                    fprintf(hReportFile,"<td width=\"11%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iDieX);
                    fprintf(hReportFile,"<td width=\"11%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iDieY);
                    if(ptBinCell1 == NULL)
                        fprintf(hReportFile,"<td width=\"11%%\" bgcolor=\"#00c600\" align=\"center\">-</td>\n");
                    else
                    if(ptBinCell1->cPassFail == 'P')
                        fprintf(hReportFile,"<td width=\"11%%\" bgcolor=\"#00c600\" align=\"center\">%d</td>\n",ptBinCell1->iBinValue);
                    else
                        fprintf(hReportFile,"<td width=\"11%%\" bgcolor=\"#ff0000\" align=\"center\">%d</td>\n",ptBinCell1->iBinValue);

                    if(ptBinCell2 == NULL)
                        fprintf(hReportFile,"<td width=\"11%%\" bgcolor=\"#00c600\" align=\"center\">-</td>\n");
                    else
                    if(ptBinCell2->cPassFail == 'P')
                        fprintf(hReportFile,"<td width=\"11%%\" bgcolor=\"#00c600\" align=\"center\">%d</td>\n",ptBinCell2->iBinValue);
                    else
                        fprintf(hReportFile,"<td width=\"11%%\" bgcolor=\"#ff0000\" align=\"center\">%d</td>\n",ptBinCell2->iBinValue);

                    // Every odd mismatch, insert empty white column, and every even mismatch, go to next line in table.
                    if(clWafBinMismatchPareto_All.totalCount() % 2)
                        fprintf(hReportFile,"</tr>\n");
                    else
                        fprintf(hReportFile,"<td width=\"12%%\" bgcolor=\"#ffffff\" align=</td>\n"); // White column
                }
            }

            // Insert into mismatch pareto objects
            clWafBinMismatchPareto_All.insert(ptBinCell1, ptBinCell2);

            if((ptBinCell1 && ptBinCell1->cPassFail == 'P') && (ptBinCell2 && ptBinCell2->cPassFail != 'P'))
                clWafBinMismatchPareto_PassToFail.insert(ptBinCell1, ptBinCell2);

            if((ptBinCell1 && ptBinCell1->cPassFail != 'P') && (ptBinCell2 && ptBinCell2->cPassFail == 'P'))
                clWafBinMismatchPareto_FailToPass.insert(ptBinCell1, ptBinCell2);
        }
    }

    // If no mismatch, tell it! otherwise, close the table.
    double lfPercent=0;
    if(bDieMismatchTable)
    {
        if (of=="CSV")
        {
            if(clWafBinMismatchPareto_All.totalCount())
            {
                fprintf(hReportFile,"\n");
                fprintf(hReportFile,"Total Dies not matching: %d\n",clWafBinMismatchPareto_All.totalCount());
            }
            else
                fprintf(hReportFile,"Perfect Wafer-Bin match!\n");
        }
        else
        //if ((of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE"))
        if(m_pReportOptions->isReportOutputHtmlBased())
        {
            if(clWafBinMismatchPareto_All.totalCount())
            {
                fprintf(hReportFile,"</table>\n");
                fprintf(hReportFile,"<p><b>Total Dies not matching: %d</b></p>",clWafBinMismatchPareto_All.totalCount());
            }
            else
                fprintf(hReportFile,"<p><b>Perfect Wafer-Bin match!</b></p>");
        }
    }

    // If enabled, display delta yield section
    if(bDeltaYield_Section && (clWafBinMismatchPareto_All.totalCount() > 0))
    {
        QString strSectionTitle;

        if (of=="CSV")
        {
            // Create a delta yield summary table
            fprintf(hReportFile,"\nDelta yield summary table\n\n");
            fprintf(hReportFile,"-,%s,%s,Common,%s (including P->F dies)\n",strLabel1.toLatin1().constData(),strLabel2.toLatin1().constData(),strLabel1.toLatin1().constData());

            // Total dies
            fprintf(hReportFile,"Total dies,%d,%d,%d,%d\n",ptWaferMapData1->iTotalPhysicalDies,ptWaferMapData2->iTotalPhysicalDies,uiCommonDies,ptWaferMapData1->iTotalPhysicalDies);

            // Total good
            fprintf(hReportFile,"Total PASS,%d,%d,-,%d\n",uiTotalGoodWafer1,uiTotalGoodWafer2,uiTotalGoodWafer1-clWafBinMismatchPareto_PassToFail.totalCount());

            // Yield
            fprintf(hReportFile,"Yield,");
            lfPercent = 100.0F*(double)uiTotalGoodWafer1 / (double)(ptWaferMapData1->iTotalPhysicalDies);
            fprintf(hReportFile,"%s%%,",algorithms::gexFormatDouble(lfPercent,2.0).toLatin1().constData());
            lfPercent = 100.0F*(double)uiTotalGoodWafer2 / (double)(ptWaferMapData2->iTotalPhysicalDies);
            fprintf(hReportFile,"%s%%,-,",algorithms::gexFormatDouble(lfPercent,2.0).toLatin1().constData());
            lfPercent = 100.0F*(double)(uiTotalGoodWafer1-clWafBinMismatchPareto_PassToFail.totalCount()) / (double)(ptWaferMapData1->iTotalPhysicalDies);
            fprintf(hReportFile,"%s%%\n",algorithms::gexFormatDouble(lfPercent,2.0).toLatin1().constData());

            // Mismatch % for all mismatches
            fprintf(hReportFile,"Mismatch (all): %d,",clWafBinMismatchPareto_All.totalCount());
            lfPercent = 100.0F*(double)(clWafBinMismatchPareto_All.totalCount()) / (double)(ptWaferMapData1->iTotalPhysicalDies);
            fprintf(hReportFile,"%s%%,",algorithms::gexFormatDouble(lfPercent,2.0).toLatin1().constData());
            lfPercent = 100.0F*(double)(clWafBinMismatchPareto_All.totalCount()) / (double)(ptWaferMapData2->iTotalPhysicalDies);
            fprintf(hReportFile,"%s%%,",algorithms::gexFormatDouble(lfPercent,2.0).toLatin1().constData());
            lfPercent = 100.0F*(double)(clWafBinMismatchPareto_All.totalCount()) / (double)uiCommonDies;
            fprintf(hReportFile,"%s%%,-\n",algorithms::gexFormatDouble(lfPercent,2.0).toLatin1().constData());

            // Mismatch % for P->F binnings
            fprintf(hReportFile,"Mismatch (P->F): %d,",clWafBinMismatchPareto_PassToFail.totalCount());
            lfPercent = 100.0F*(double)(clWafBinMismatchPareto_PassToFail.totalCount()) / (double)(ptWaferMapData1->iTotalPhysicalDies);
            fprintf(hReportFile,"%s%%,",algorithms::gexFormatDouble(lfPercent,2.0).toLatin1().constData());
            lfPercent = 100.0F*(double)(clWafBinMismatchPareto_PassToFail.totalCount()) / (double)(ptWaferMapData2->iTotalPhysicalDies);
            fprintf(hReportFile,"%s%%,",algorithms::gexFormatDouble(lfPercent,2.0).toLatin1().constData());
            lfPercent = 100.0F*(double)(clWafBinMismatchPareto_PassToFail.totalCount()) / (double)uiCommonDies;
            fprintf(hReportFile,"%s%%,-\n",algorithms::gexFormatDouble(lfPercent,2.0).toLatin1().constData());

            // Mismatch % for F->P binnings
            fprintf(hReportFile,"Mismatch (F->P): %d,",clWafBinMismatchPareto_FailToPass.totalCount());
            lfPercent = 100.0F*(double)(clWafBinMismatchPareto_FailToPass.totalCount()) / (double)(ptWaferMapData1->iTotalPhysicalDies);
            fprintf(hReportFile,"%s%%,",algorithms::gexFormatDouble(lfPercent,2.0).toLatin1().constData());
            lfPercent = 100.0F*(double)(clWafBinMismatchPareto_FailToPass.totalCount()) / (double)(ptWaferMapData2->iTotalPhysicalDies);
            fprintf(hReportFile,"%s%%,",algorithms::gexFormatDouble(lfPercent,2.0).toLatin1().constData());
            lfPercent = 100.0F*(double)(clWafBinMismatchPareto_FailToPass.totalCount()) / (double)uiCommonDies;
            fprintf(hReportFile,"%s%%,-\n\n",algorithms::gexFormatDouble(lfPercent,2.0).toLatin1().constData());

            // Write differential yield
            lfPercent = 100.0F*(1.0F-((double)(clWafBinMismatchPareto_PassToFail.totalCount())/(double)uiCommonDies));
            fprintf(hReportFile,"Differential yield [(1-(P->F dies)/Common dies)*100],%s%%\n",algorithms::gexFormatDouble(lfPercent,2.0).toLatin1().constData());
        }
        else
        {
            // Create a delta yield summary table
            fprintf(hReportFile,"<h2><font color=\"#006699\">Delta yield summary table</font></h2>\n");
            if (of=="HTML")
                fprintf(hReportFile,"<table border=\"0\" width=\"98%%\">\n");
            else
                WriteHtmlOpenTable(98,0);	// HTML code to open table, 98%, cell spacing=0
            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"center\">-</td>\n",szFieldColor);
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"center\"><b>%s</b></td>\n",szFieldColor,strLabel1.toLatin1().constData());
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"center\"><b>%s</b></td>\n",szFieldColor,strLabel2.toLatin1().constData());
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"center\"><b>Common</b></td>\n",szFieldColor);
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"center\"><b>%s<br>(including P->F dies)</b></td>\n",szFieldColor,strLabel1.toLatin1().constData());
            fprintf(hReportFile,"</tr>\n");

            // Total dies
            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"center\"><b>Total dies</b></td>\n",szFieldColor);
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,ptWaferMapData1->iTotalPhysicalDies);
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,ptWaferMapData2->iTotalPhysicalDies);
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,uiCommonDies);
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,ptWaferMapData1->iTotalPhysicalDies);
            fprintf(hReportFile,"</tr>\n");

            // Total good
            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"center\"><b>Total PASS</b></td>\n",szFieldColor);
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,uiTotalGoodWafer1);
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,uiTotalGoodWafer2);
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"center\">-</td>\n",szDataColor);
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,uiTotalGoodWafer1-clWafBinMismatchPareto_PassToFail.totalCount());
            fprintf(hReportFile,"</tr>\n");

            // Yield
            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"center\"><b>Yield</b></td>\n",szFieldColor);
            lfPercent = 100.0F*(double)uiTotalGoodWafer1 / (double)(ptWaferMapData1->iTotalPhysicalDies);
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"center\">%s%%</td>\n",szDataColor,algorithms::gexFormatDouble(lfPercent,2.0).toLatin1().constData());
            lfPercent = 100.0F*(double)uiTotalGoodWafer2 / (double)(ptWaferMapData2->iTotalPhysicalDies);
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"center\">%s%%</td>\n",szDataColor,algorithms::gexFormatDouble(lfPercent,2.0).toLatin1().constData());
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"center\">-</td>\n",szDataColor);
            lfPercent = 100.0F*(double)(uiTotalGoodWafer1-clWafBinMismatchPareto_PassToFail.totalCount()) / (double)(ptWaferMapData1->iTotalPhysicalDies);
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"center\">%s%%</td>\n",szDataColor,algorithms::gexFormatDouble(lfPercent,2.0).toLatin1().constData());
            fprintf(hReportFile,"</tr>\n");

            // Mismatch % for all mismatches
            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"center\"><b>Mismatch (all): %d</b></td>\n",szFieldColor,clWafBinMismatchPareto_All.totalCount());
            lfPercent = 100.0F*(double)(clWafBinMismatchPareto_All.totalCount()) / (double)(ptWaferMapData1->iTotalPhysicalDies);
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"center\">%s%%</td>\n",szDataColor,algorithms::gexFormatDouble(lfPercent,2.0).toLatin1().constData());
            lfPercent = 100.0F*(double)(clWafBinMismatchPareto_All.totalCount()) / (double)(ptWaferMapData2->iTotalPhysicalDies);
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"center\">%s%%</td>\n",szDataColor,algorithms::gexFormatDouble(lfPercent,2.0).toLatin1().constData());
            lfPercent = 100.0F*(double)(clWafBinMismatchPareto_All.totalCount()) / (double)uiCommonDies;
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"center\">%s%%</td>\n",szDataColor,algorithms::gexFormatDouble(lfPercent,2.0).toLatin1().constData());
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"center\">-</td>\n",szDataColor);
            fprintf(hReportFile,"</tr>\n");

            // Mismatch % for P->F binnings
            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"center\"><b>Mismatch (P->F): %d</b></td>\n",szFieldColor,clWafBinMismatchPareto_PassToFail.totalCount());
            lfPercent = 100.0F*(double)(clWafBinMismatchPareto_PassToFail.totalCount()) / (double)(ptWaferMapData1->iTotalPhysicalDies);
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"center\">%s%%</td>\n",szDataColor,algorithms::gexFormatDouble(lfPercent,2.0).toLatin1().constData());
            lfPercent = 100.0F*(double)(clWafBinMismatchPareto_PassToFail.totalCount()) / (double)(ptWaferMapData2->iTotalPhysicalDies);
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"center\">%s%%</td>\n",szDataColor,algorithms::gexFormatDouble(lfPercent,2.0).toLatin1().constData());
            lfPercent = 100.0F*(double)(clWafBinMismatchPareto_PassToFail.totalCount()) / (double)uiCommonDies;
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"center\">%s%%</td>\n",szDataColor,algorithms::gexFormatDouble(lfPercent,2.0).toLatin1().constData());
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"center\">-</td>\n",szDataColor);
            fprintf(hReportFile,"</tr>\n");

            // Mismatch % for F->P binnings
            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"center\"><b>Mismatch (F->P): %d</b></td>\n",szFieldColor,clWafBinMismatchPareto_FailToPass.totalCount());
            lfPercent = 100.0F*(double)(clWafBinMismatchPareto_FailToPass.totalCount()) / (double)(ptWaferMapData1->iTotalPhysicalDies);
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"center\">%s%%</td>\n",szDataColor,algorithms::gexFormatDouble(lfPercent,2.0).toLatin1().constData());
            lfPercent = 100.0F*(double)(clWafBinMismatchPareto_FailToPass.totalCount()) / (double)(ptWaferMapData2->iTotalPhysicalDies);
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"center\">%s%%</td>\n",szDataColor,algorithms::gexFormatDouble(lfPercent,2.0).toLatin1().constData());
            lfPercent = 100.0F*(double)(clWafBinMismatchPareto_FailToPass.totalCount()) / (double)uiCommonDies;
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"center\">%s%%</td>\n",szDataColor,algorithms::gexFormatDouble(lfPercent,2.0).toLatin1().constData());
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"center\">-</td>\n",szDataColor);
            fprintf(hReportFile,"</tr>\n");

            // Close summary table
            fprintf(hReportFile,"</table>\n");

            // Write differential yield
            lfPercent = 100.0F*(1.0F-((double)(clWafBinMismatchPareto_PassToFail.totalCount())/(double)uiCommonDies));
            fprintf(hReportFile,"<p><b>Differential yield [(1-(P->F dies)/Common dies)*100]: %s%%</b></p>",algorithms::gexFormatDouble(lfPercent,2.0).toLatin1().constData());
        }

        // Delta yield: all bins
        strSectionTitle = "Delta yield binning pareto: all mismatches";
        WriteDeltaYieldSection(&clWafBinMismatchPareto_All, strSectionTitle, strLabel1, strLabel2, uiCommonDies);

        // Delta yield: Pass->Fail bins
        strSectionTitle = "Delta yield binning pareto: Pass to Fail";
        WriteDeltaYieldSection(&clWafBinMismatchPareto_PassToFail, strSectionTitle, strLabel1, strLabel2, uiCommonDies);

        // Delta yield: Fail->Pass bins
        strSectionTitle = "Delta yield binning pareto: Fail to Pass";
        WriteDeltaYieldSection(&clWafBinMismatchPareto_FailToPass, strSectionTitle, strLabel1, strLabel2, uiCommonDies);
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// Writes Delta yield section
/////////////////////////////////////////////////////////////////////////////
void CGexReport::WriteDeltaYieldSection(CWafBinMismatchPareto *pWafBinMismatchPareto,
     const QString & strSectionTitle, const QString & strColumn1Label,
     const QString & strColumn2Label, unsigned int uiCommonDies)
{
    double lfPercent;
    QString of=m_pReportOptions->GetOption("output", "format").toString();

    if (of=="CSV")
    {
        // Write section title
        fprintf(hReportFile,"\n%s\n", strSectionTitle.toLatin1().constData());

        // Check if some parts for this section
        if(pWafBinMismatchPareto->totalCount() == 0)
        {
            fprintf(hReportFile,"\nNo dies for this section!!\n");
            return;
        }

        // Sort pareto and iterate through it
        pWafBinMismatchPareto->sort();
        const CWafBinMismatch	* pBinMismatch = NULL;

        // Table header
        fprintf(hReportFile,"\n%s,%s,Frequency (#),Frequency (%% of common dies)\n",strColumn1Label.toLatin1().constData(),strColumn2Label.toLatin1().constData());

        // Display mismatches
        for(int nItem = 0; nItem < pWafBinMismatchPareto->listWaferBinMismatch().count(); ++nItem)
        {
            pBinMismatch = pWafBinMismatchPareto->listWaferBinMismatch().at(nItem);

            lfPercent = 100.0*pBinMismatch->m_uiCount / uiCommonDies;

            if(pBinMismatch->m_pBinGroup1->cPassFail == 'P')
                fprintf(hReportFile,"%d (P),",pBinMismatch->m_pBinGroup1->iBinValue);
            else
                fprintf(hReportFile,"%d (F),",pBinMismatch->m_pBinGroup1->iBinValue);
            if(pBinMismatch->m_pBinGroup2->cPassFail == 'P')
                fprintf(hReportFile,"%d (P),",pBinMismatch->m_pBinGroup2->iBinValue);
            else
                fprintf(hReportFile,"%d (F),",pBinMismatch->m_pBinGroup2->iBinValue);
            fprintf(hReportFile,"%d,%s%%\n",pBinMismatch->m_uiCount,algorithms::gexFormatDouble(lfPercent, 2.0).toLatin1().constData());
        }

        // Total
        lfPercent = 100.0*pWafBinMismatchPareto->totalCount() / uiCommonDies;
        fprintf(hReportFile,"Total,-,%d,%s%%\n",pWafBinMismatchPareto->totalCount(),algorithms::gexFormatDouble(lfPercent, 2.0).toLatin1().constData());
    }
    else
    {
        // Write section title
        fprintf(hReportFile,"<h2><font color=\"#006699\">%s</font></h2>\n", strSectionTitle.toLatin1().constData());

        // Check if some parts for this section
        if(pWafBinMismatchPareto->totalCount() == 0)
        {
            fprintf(hReportFile,"<p><b>No dies for this section!!</b></p>\n");
            return;
        }

        // Sort pareto and iterate through it
        pWafBinMismatchPareto->sort();
        const CWafBinMismatch *	pBinMismatch = NULL;

        // Table header
        if (of=="HTML")
            fprintf(hReportFile,"<table border=\"0\" width=\"98%%\">\n");
        else
            WriteHtmlOpenTable(98,0);	// HTML code to open table, 98%, cell spacing=0
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td width=\"25%%\" bgcolor=%s align=\"center\"><b>%s</b></td>\n",szFieldColor,strColumn1Label.toLatin1().constData());
        fprintf(hReportFile,"<td width=\"25%%\" bgcolor=%s align=\"center\"><b>%s</b></td>\n",szFieldColor,strColumn2Label.toLatin1().constData());
        fprintf(hReportFile,"<td width=\"25%%\" bgcolor=%s align=\"center\"><b>Frequency<br>(#)</b></td>\n",szFieldColor);
        fprintf(hReportFile,"<td width=\"25%%\" bgcolor=%s align=\"center\"><b>Frequency<br>(%% of common dies)</b></td>\n",szFieldColor);
        fprintf(hReportFile,"</tr>\n");

        // Display mismatches
        for(int nItem = 0; nItem < pWafBinMismatchPareto->listWaferBinMismatch().count(); ++nItem)
        {
            pBinMismatch = pWafBinMismatchPareto->listWaferBinMismatch().at(nItem);

            lfPercent = 100.0*pBinMismatch->m_uiCount / uiCommonDies;
            fprintf(hReportFile,"<tr>\n");
            if(pBinMismatch->m_pBinGroup1 == NULL)
                fprintf(hReportFile,"<td width=\"25%%\" bgcolor=\"#00c600\" align=\"center\">-</td>\n");
            else
            if(pBinMismatch->m_pBinGroup1->cPassFail == 'P')
                fprintf(hReportFile,"<td width=\"25%%\" bgcolor=\"#00c600\" align=\"center\">%d</td>\n",pBinMismatch->m_pBinGroup1->iBinValue);
            else
                fprintf(hReportFile,"<td width=\"25%%\" bgcolor=\"#ff0000\" align=\"center\">%d</td>\n",pBinMismatch->m_pBinGroup1->iBinValue);

            if(pBinMismatch->m_pBinGroup2 == NULL)
                fprintf(hReportFile,"<td width=\"25%%\" bgcolor=\"#00c600\" align=\"center\">-</td>\n");
            else
            if(pBinMismatch->m_pBinGroup2->cPassFail == 'P')
                fprintf(hReportFile,"<td width=\"25%%\" bgcolor=\"#00c600\" align=\"center\">%d</td>\n",pBinMismatch->m_pBinGroup2->iBinValue);
            else
                fprintf(hReportFile,"<td width=\"25%%\" bgcolor=\"#ff0000\" align=\"center\">%d</td>\n",pBinMismatch->m_pBinGroup2->iBinValue);
            fprintf(hReportFile,"<td width=\"25%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,pBinMismatch->m_uiCount);
            fprintf(hReportFile,"<td width=\"25%%\" bgcolor=%s align=\"center\">%s%%</td>\n",szDataColor,algorithms::gexFormatDouble(lfPercent, 2.0).toLatin1().constData());
            fprintf(hReportFile,"</tr>\n");
        }

        // Total
        lfPercent = 100.0*pWafBinMismatchPareto->totalCount() / uiCommonDies;
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td width=\"25%%\" bgcolor=%s align=\"center\"><b>Total</b></td>\n",szFieldColor);
        fprintf(hReportFile,"<td width=\"25%%\" bgcolor=%s align=\"center\">-</td>\n",szFieldColor);
        fprintf(hReportFile,"<td width=\"25%%\" bgcolor=%s align=\"center\">%d</td>\n",szFieldColor,pWafBinMismatchPareto->totalCount());
        fprintf(hReportFile,"<td width=\"25%%\" bgcolor=%s align=\"center\">%s%%</td>\n",szFieldColor,algorithms::gexFormatDouble(lfPercent, 2.0).toLatin1().constData());
        fprintf(hReportFile,"</tr>\n");

        // Close summary table
        fprintf(hReportFile,"</table>\n<br>\n");
    }
}

/////////////////////////////////////////////////////////////////////////////
// Creates a spectrum of 50 colors for Zonal or Stacked or Parametric wafermaps.
/////////////////////////////////////////////////////////////////////////////
QColor *CGexReport::CreateColorSpectrum(void)
{
    QColor *sColor = new QColor[51];	// 50 colors for Zoning results, or stacked wafers.
    sColor[0] = QColor(0,8,255);		//
    sColor[1] = QColor(0,24,255);		//
    sColor[2] = QColor(0,49,255);		//
    sColor[3] = QColor(0,74,255);		//
    sColor[4] = QColor(0,99,255);		//
    sColor[5] = QColor(0,115,255);		//
    sColor[6] = QColor(0,132,255);		//
    sColor[7] = QColor(0,156,255);		//
    sColor[8] = QColor(0,173,255);		//
    sColor[9] = QColor(0,189,255);		//
    sColor[10] = QColor(0,214,255);		//
    sColor[11] = QColor(0,239,255);		//
    sColor[12] = QColor(0,255,239);		//
    sColor[13] = QColor(0,255,222);		//
    sColor[14] = QColor(0,255,198);		//
    sColor[15] = QColor(0,255,173);		//
    sColor[16] = QColor(0,255,148);		//
    sColor[17] = QColor(0,255,132);		//
    sColor[18] = QColor(0,255,115);		//
    sColor[19] = QColor(0,255,90);		//
    sColor[20] = QColor(0,255,74);		//
    sColor[21] = QColor(0,255,49);		//
    sColor[22] = QColor(0,255,33);		//
    sColor[23] = QColor(0,255,16);		//
    sColor[24] = QColor(0,255,0);		// Red
    sColor[25] = QColor(8,255,0);		//
    sColor[26] = QColor(24,255,0);		//
    sColor[27] = QColor(49,255,0);		//
    sColor[28] = QColor(74,255,0);		//
    sColor[29] = QColor(107,255,0);		//
    sColor[30] = QColor(123,255,0);		//
    sColor[31] = QColor(148,255,0);		//
    sColor[32] = QColor(165,255,0);		//
    sColor[33] = QColor(181,255,0);		//
    sColor[34] = QColor(214,255,0);		//
    sColor[35] = QColor(239,255,0);		//
    sColor[36] = QColor(255,255,0);		// Yellow
    sColor[37] = QColor(255,239,0);		//
    sColor[38] = QColor(255,214,0);		//
    sColor[39] = QColor(255,189,0);		//
    sColor[40] = QColor(255,173,0);		//
    sColor[41] = QColor(255,165,0);		//
    sColor[42] = QColor(255,140,0);		//
    sColor[43] = QColor(255,115,0);		//
    sColor[44] = QColor(255,99,0);		//
    sColor[45] = QColor(255,74,0);		//
    sColor[46] = QColor(255,57,0);		//
    sColor[47] = QColor(255,33,0);		//
    sColor[48] = QColor(255,8,0);		//
    sColor[49] = QColor(255,0,0);		// Red
    sColor[50] = QColor(255,0,0);		//
    return sColor;
}

/////////////////////////////////////////////////////////////////////////////
// Class used for computing yield per region
/////////////////////////////////////////////////////////////////////////////
// Constructor
QZonalRegion::QZonalRegion()
{
    iTotalParts = 0;	// total parts in zone
    iTotalMatch = 0;	// Total parts matching bin we focus on
}

/////////////////////////////////////////////////////////////////////////////////////////
int CGexReport::getZoneIndex(int nXLoc, int nYLoc, int m_nXMax,int m_nYMax)
{
    int				nA, nB;
    float			fC, fR;
    int				nRA, nRB;
    int				nZoneIndex = -1;

    nRA = (m_nXMax/2);
    nRB = (m_nYMax/2);
    // Computation of the zone
    if(nXLoc < nRA)
    {
        nA = nRA - nXLoc;
        if(nYLoc > nRB)
        {
            nZoneIndex = 4;
            nB = nYLoc - nRB ;
        }
        else
        {
            nZoneIndex = 1;
            nB = nRB - nYLoc;
        }
    }
    else
    {
        nA = nXLoc - nRA ;
        if(nYLoc > nRB)
        {
            nZoneIndex = 7;
            nB = nYLoc - nRB ;
        }
        else
        {
            nZoneIndex = 10;
            nB = nRB - nYLoc;
        }
    }
    // Add section in each zone
    // find the position in 3 circles with Pythagore

    // adaptive ray, depends of X and Y position
    // ellipse configurartion
    float	fAPercent = 0.0;
    float	fBPercent = 0.0;

    if((nA == 0) && (nB == 0))
    {
        fAPercent = 1.0;
        fBPercent = 1.0;
    }
    else
    {
        fAPercent = ((float) nA/(float)nRA);
        fBPercent = ((float) nB/(float)nRB);
    }

    fR = (fAPercent*(float)nRA) + (fBPercent*(float)nRB);
    fR /= (fAPercent + fBPercent);

    fC = sqrt(GS_POW((float)nA,2.0) + GS_POW((float)nB,2.0));
    if(fC > ((2.0/3.0)*fR))
    {
        //last circle
        nZoneIndex += 2;
    }
    else if(fC > ((1.0/3.0)*fR))
    {
        // second circle
        nZoneIndex += 1;
    }
    else
    {
        // first circle
        nZoneIndex += 0;
    }

    // Return value from 0 to 11
    return nZoneIndex-1;
}

/////////////////////////////////////////////////////////////////////////////
// Compute Zonal regions: 12 zones.
/////////////////////////////////////////////////////////////////////////////
void CGexReport::ComputeZonalRegions(
        CGexGroupOfFiles *pGroup,QZonalRegion	cZonalRegions[12])
{
    // Total wafers stacked
    int iTotalWafersStacked = pGroup->cStackedWaferMapData.iTotalWafermaps;

    // Wafermap (or Stacked Wafermap if multiple wafers in the group)
    int	iLine,iCol,iBinCode,iZone;
    for(iLine = 0; iLine < pGroup->cStackedWaferMapData.SizeY; iLine++)
    {
        // Processing a wafer line.
        for(iCol = 0; iCol < pGroup->cStackedWaferMapData.SizeX; iCol++)
        {
            // Compute zone: see in which zone belongs this die.
            iZone = getZoneIndex(iCol,iLine,pGroup->cStackedWaferMapData.SizeX,pGroup->cStackedWaferMapData.SizeY);

            // Get matching Bin count at location iRow,iCol.
            iBinCode = pGroup->cStackedWaferMapData.cWafMap[(iCol+(iLine*pGroup->cStackedWaferMapData.SizeX))].ldCount;
            switch(iBinCode)
            {
                case GEX_WAFMAP_EMPTY_CELL:	// Die not tested at all.
                    break;

                default:
                // Update Matching bin count
                cZonalRegions[iZone].iTotalMatch += iBinCode;

                // Update total count
                cZonalRegions[iZone].iTotalParts += iTotalWafersStacked;
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// Build a percentage string to display in Zonal wafermap.
static int getPercentageZone(int iTotalMatchingDies,int iTotalDies,
            float &fMinPercentage,float &fMaxPercentage,QString &strText)
{
    float fPercent=0;
    if(iTotalDies)
        fPercent = (float)iTotalMatchingDies*100.0/iTotalDies;
    strText.sprintf("%.1f%%",fPercent);

    // Returns 0 if = lowest percentage in zones, 50 if = highest percentage in zones
    if(fMaxPercentage-fMinPercentage)
        fPercent = 50.0*(fPercent-fMinPercentage)/(fMaxPercentage-fMinPercentage);
    else
        fPercent = 0;
    return (int) fPercent;	// Ranges in 0-50
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
bool
CGexReport::WriteZonalWaferMap(CGexGroupOfFiles* pGroup,
                               CGexFileInGroup* /*pFile*/,
                               BOOL bWriteReport,
                               int iGroupID,
                               QString /*strImageName = ""*/)
{
    // Report must not be created yet...then quietly return
    if(bWriteReport == false)
        return false;

    // Check if Zonal report selected
    switch(m_pReportOptions->iWafermapType)
    {
        case GEX_WAFMAP_ZONAL_SOFTBIN:
        case GEX_WAFMAP_ZONAL_HARDBIN:
            break;			// Yes: Zonal report selected

        default:
            return false;	// No: Zonal report is diseabled
    }

    // If Examinator Characterization STD edition....do not allow this report!
    bool bShow = true;
    if(GS::LPPlugin::ProductInfo::getInstance()->isOEM())
    {
        bShow = false;
    }

    QString of=m_pReportOptions->GetOption("output", "format").toString();

    if(bShow == false)
    {
        // Display message that Zonal wafer is not available under this module!
        if (of=="CSV")
        {
            fprintf(hReportFile,"Zonal wafers: Not available under this release.\n");
            QString m=ReportOptions.GetOption("messages", "upgrade").toString();
            fprintf(hReportFile, "%s", m.toLatin1().data() );
        }
        else
        //if (of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
        if(m_pReportOptions->isReportOutputHtmlBased())
        {
            // Title + bookmark
            WriteHtmlSectionTitle(hReportFile,"all_zonalwafers","Zonal Wafer maps");
            fprintf(hReportFile,"<br>Feature not available: <br>\n");
            fprintf(hReportFile,"For this report, you need to <b>upgrade</b> your license.<br>\n");
            fprintf(hReportFile,"Contact %s for more details.<br><br>\n",GEX_EMAIL_SALES);
        }
        return true;
    }

    // Creates the 'Wafermap' page & header for given file.
    QString		strString;
    QString		strBookmark;
    char		szString[2048];
    int	iDays=0;
    int	iHours=0;
    int	iMinutes=0;

    iDays = (pGroup->cStackedWaferMapData.iTotaltime) / (24*3600);
    iHours = ((pGroup->cStackedWaferMapData.iTotaltime) - iDays*(24*3600)) / 3600;
    iMinutes = ((pGroup->cStackedWaferMapData.iTotaltime) - iDays*(24*3600) - iHours*3600)/60;

    // Compute the 12 zones (yield, pass/fail count) of the wafermap
    // 2				11
    //    1			10
    //       0	9
    //	     3	 6
    //    4			7
    // 5				 8

    QZonalRegion	cZonalRegions[12];	// Allocates buffer for the 12 zones to compute
    ComputeZonalRegions(pGroup,cZonalRegions);

    // Compute minimum & maximum percentage values over the zone.
    int	iIndex;
    float	fMinPercentage=100;
    float	fMaxPercentage=0;
    float	fPercentage;
    for(iIndex=0;iIndex <12;iIndex++)
    {
        if(cZonalRegions[iIndex].iTotalParts)
        {
            fPercentage = 100.0*(float)cZonalRegions[iIndex].iTotalMatch/(float)cZonalRegions[iIndex].iTotalParts;
            fMinPercentage = gex_min(fMinPercentage,fPercentage);
            fMaxPercentage = gex_max(fMaxPercentage,fPercentage);
        }
    }


    int	iTotalDies=0,iTotalMatchingDies=0;
    if (of=="CSV")
    {
        fprintf(hReportFile,"Zonal wafermap\n");
        fprintf(hReportFile,"Total Wafers:,%d\n",pGroup->cStackedWaferMapData.iTotalWafermaps);
        switch(m_pReportOptions->iWafermapType)
        {
            case GEX_WAFMAP_ZONAL_SOFTBIN:
                fprintf(hReportFile,"Soft-Bin list selected:,%s\n",
                        m_pReportOptions->pGexWafermapRangeList->BuildTestListString("").toLatin1().constData());
                break;
            case GEX_WAFMAP_ZONAL_HARDBIN:
                fprintf(hReportFile,"Hard-Bin list selected:,%s\n",
                        m_pReportOptions->pGexWafermapRangeList->BuildTestListString("").toLatin1().constData());
                break;
        }

        if(pGroup->cStackedWaferMapData.iTotaltime)
        {
            fprintf(hReportFile,"Testing time:,");
            if(iDays)
                fprintf(hReportFile,"%d days ",iDays);
            fprintf(hReportFile,"%d Hours ",iHours);
            fprintf(hReportFile,"%d Min\n",iMinutes);
        }

        fprintf(hReportFile,"\n---- ZONAL MAP ----:\n");
        // Number of wafers.
        WriteInfoLine("Total Wafers",pGroup->cStackedWaferMapData.iTotalWafermaps);
        // Binning list.
        strString = m_pReportOptions->pGexWafermapRangeList->BuildTestListString("");
        strString += "    ( for a different Bin list, see the 'Settings' page, section 'wafermap' )";
        switch(m_pReportOptions->iWafermapType)
        {
            case GEX_WAFMAP_ZONAL_SOFTBIN:
                WriteInfoLine("Soft-Bin list", strString.toLatin1().constData());
                break;
            case GEX_WAFMAP_ZONAL_HARDBIN:
                WriteInfoLine("Hard-Bin list", strString.toLatin1().constData());
                break;
        }

        // Total testing time.
        if(pGroup->cStackedWaferMapData.iTotaltime)
        {
            if(iDays)
                sprintf(szString,"%d days %d Hours %d Min",iDays,iHours,iMinutes);
            else
                sprintf(szString,"%d Hours %d Min",iHours,iMinutes);
            WriteInfoLine("Testing time",szString);
        }

        // Display bin counts per zone
        fprintf(hReportFile,"Zone,Total dies, Matching dies in zone, Percentage\n");
        // Full wafer
        double lfPercentage;
        for(iIndex=0;iIndex <12;iIndex++)
        {
            iTotalMatchingDies += cZonalRegions[iIndex].iTotalMatch;
            iTotalDies += cZonalRegions[iIndex].iTotalParts;
        }
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"Full wafer,%d,%d,%.1lf %%\n",iTotalDies,iTotalMatchingDies,lfPercentage);

        // Upper wafer count
        iTotalMatchingDies = cZonalRegions[0].iTotalMatch+cZonalRegions[1].iTotalMatch+cZonalRegions[2].iTotalMatch;
        iTotalMatchingDies += cZonalRegions[9].iTotalMatch+cZonalRegions[10].iTotalMatch+cZonalRegions[11].iTotalMatch;
        iTotalDies = cZonalRegions[0].iTotalParts+cZonalRegions[1].iTotalParts+cZonalRegions[2].iTotalParts;
        iTotalDies += cZonalRegions[9].iTotalParts+cZonalRegions[10].iTotalParts+cZonalRegions[11].iTotalParts;
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"Upper half wafer,%d,%d,%.1lf %%\n",iTotalDies,iTotalMatchingDies,lfPercentage);

        // Lower wafer count
        iTotalMatchingDies = cZonalRegions[3].iTotalMatch+cZonalRegions[4].iTotalMatch+cZonalRegions[5].iTotalMatch;
        iTotalMatchingDies += cZonalRegions[6].iTotalMatch+cZonalRegions[7].iTotalMatch+cZonalRegions[8].iTotalMatch;
        iTotalDies = cZonalRegions[3].iTotalParts+cZonalRegions[4].iTotalParts+cZonalRegions[5].iTotalParts;
        iTotalDies += cZonalRegions[6].iTotalParts+cZonalRegions[7].iTotalParts+cZonalRegions[8].iTotalParts;
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"Lower half wafer,%d,%d,%.1lf %%\n",iTotalDies,iTotalMatchingDies,lfPercentage);

        // Left half wafer count
        iTotalMatchingDies = cZonalRegions[0].iTotalMatch+cZonalRegions[1].iTotalMatch+cZonalRegions[2].iTotalMatch;
        iTotalMatchingDies += cZonalRegions[3].iTotalMatch+cZonalRegions[4].iTotalMatch+cZonalRegions[5].iTotalMatch;
        iTotalDies = cZonalRegions[0].iTotalParts+cZonalRegions[1].iTotalParts+cZonalRegions[2].iTotalParts;
        iTotalDies += cZonalRegions[3].iTotalParts+cZonalRegions[4].iTotalParts+cZonalRegions[5].iTotalParts;
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"Left half wafer,%d,%d,%.1lf %%\n",iTotalDies,iTotalMatchingDies,lfPercentage);

        // Right half wafer count
        iTotalMatchingDies = cZonalRegions[6].iTotalMatch+cZonalRegions[7].iTotalMatch+cZonalRegions[8].iTotalMatch;
        iTotalMatchingDies += cZonalRegions[9].iTotalMatch+cZonalRegions[10].iTotalMatch+cZonalRegions[11].iTotalMatch;
        iTotalDies = cZonalRegions[6].iTotalParts+cZonalRegions[7].iTotalParts+cZonalRegions[8].iTotalParts;
        iTotalDies += cZonalRegions[9].iTotalParts+cZonalRegions[10].iTotalParts+cZonalRegions[11].iTotalParts;
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"Right half wafer,%d,%d,%.1lf %%\n",iTotalDies,iTotalMatchingDies,lfPercentage);

        // Upper left wafer count
        iTotalMatchingDies = cZonalRegions[0].iTotalMatch+cZonalRegions[1].iTotalMatch+cZonalRegions[2].iTotalMatch;
        iTotalDies = cZonalRegions[0].iTotalParts+cZonalRegions[1].iTotalParts+cZonalRegions[2].iTotalParts;
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"Upper Left slice,%d,%d,%.1lf %%\n",iTotalDies,iTotalMatchingDies,lfPercentage);

        // Upper Right wafer count
        iTotalMatchingDies = cZonalRegions[9].iTotalMatch+cZonalRegions[10].iTotalMatch+cZonalRegions[11].iTotalMatch;
        iTotalDies = cZonalRegions[9].iTotalParts+cZonalRegions[10].iTotalParts+cZonalRegions[11].iTotalParts;
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"Upper Right slice,%d,%d,%.1lf %%\n",iTotalDies,iTotalMatchingDies,lfPercentage);

        // Lower Leftwafer count
        iTotalMatchingDies = cZonalRegions[3].iTotalMatch+cZonalRegions[4].iTotalMatch+cZonalRegions[5].iTotalMatch;
        iTotalDies = cZonalRegions[3].iTotalParts+cZonalRegions[4].iTotalParts+cZonalRegions[5].iTotalParts;
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"Lower Left slice,%d,%d,%.1lf %%\n",iTotalDies,iTotalMatchingDies,lfPercentage);

        // Lower Right wafer count
        iTotalMatchingDies = cZonalRegions[6].iTotalMatch+cZonalRegions[7].iTotalMatch+cZonalRegions[8].iTotalMatch;
        iTotalDies = cZonalRegions[6].iTotalParts+cZonalRegions[7].iTotalParts+cZonalRegions[8].iTotalParts;
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"Lower Right slice,%d,%d,%.1lf %%\n",iTotalDies,iTotalMatchingDies,lfPercentage);

        // Outer ring
        iTotalMatchingDies = cZonalRegions[5].iTotalMatch+cZonalRegions[2].iTotalMatch+cZonalRegions[11].iTotalMatch+cZonalRegions[8].iTotalMatch;
        iTotalDies = cZonalRegions[5].iTotalParts+cZonalRegions[2].iTotalParts+cZonalRegions[11].iTotalParts+cZonalRegions[8].iTotalParts;
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"Outer Ring,%d,%d,%.1lf %%\n",iTotalDies,iTotalMatchingDies,lfPercentage);

        // Middle ring
        iTotalMatchingDies = cZonalRegions[4].iTotalMatch+cZonalRegions[1].iTotalMatch+cZonalRegions[10].iTotalMatch+cZonalRegions[7].iTotalMatch;
        iTotalDies = cZonalRegions[4].iTotalParts+cZonalRegions[1].iTotalParts+cZonalRegions[10].iTotalParts+cZonalRegions[7].iTotalParts;
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"Middle Ring,%d,%d,%.1lf %%\n",iTotalDies,iTotalMatchingDies,lfPercentage);

        // Inner ring
        iTotalMatchingDies = cZonalRegions[3].iTotalMatch+cZonalRegions[0].iTotalMatch+cZonalRegions[9].iTotalMatch+cZonalRegions[6].iTotalMatch;
        iTotalDies = cZonalRegions[3].iTotalParts+cZonalRegions[0].iTotalParts+cZonalRegions[9].iTotalParts+cZonalRegions[6].iTotalParts;
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"Inner Ring,%d,%d,%.1lf %%\n",iTotalDies,iTotalMatchingDies,lfPercentage);

        // Upper left - outer ring
        iTotalMatchingDies = cZonalRegions[2].iTotalMatch;
        iTotalDies = cZonalRegions[2].iTotalParts;
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"Upper Left - Outer Ring,%d,%d,%.1lf %%\n",iTotalDies,iTotalMatchingDies,lfPercentage);

        // Upper left - middle ring
        iTotalMatchingDies = cZonalRegions[1].iTotalMatch;
        iTotalDies = cZonalRegions[1].iTotalParts;
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"Upper Left - Middle Ring,%d,%d,%.1lf %%\n",iTotalDies,iTotalMatchingDies,lfPercentage);

        // Upper left - inner ring
        iTotalMatchingDies = cZonalRegions[0].iTotalMatch;
        iTotalDies = cZonalRegions[0].iTotalParts;
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"Upper Left - Inner Ring,%d,%d,%.1lf %%\n",iTotalDies,iTotalMatchingDies,lfPercentage);

        // Upper right - outer ring
        iTotalMatchingDies = cZonalRegions[11].iTotalMatch;
        iTotalDies = cZonalRegions[11].iTotalParts;
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"Upper Right - Outer Ring,%d,%d,%.1lf %%\n",iTotalDies,iTotalMatchingDies,lfPercentage);

        // Upper right - middle ring
        iTotalMatchingDies = cZonalRegions[10].iTotalMatch;
        iTotalDies = cZonalRegions[10].iTotalParts;
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"Upper Right - Middle Ring,%d,%d,%.1lf %%\n",iTotalDies,iTotalMatchingDies,lfPercentage);

        // Upper right - inner ring
        iTotalMatchingDies = cZonalRegions[9].iTotalMatch;
        iTotalDies = cZonalRegions[9].iTotalParts;
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"Upper Right - Inner Ring,%d,%d,%.1lf %%\n",iTotalDies,iTotalMatchingDies,lfPercentage);

        // Lower left - outer ring
        iTotalMatchingDies = cZonalRegions[5].iTotalMatch;
        iTotalDies = cZonalRegions[5].iTotalParts;
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"Lower Left - Outer Ring,%d,%d,%.1lf %%\n",iTotalDies,iTotalMatchingDies,lfPercentage);

        // Lower left - middle ring
        iTotalMatchingDies = cZonalRegions[4].iTotalMatch;
        iTotalDies = cZonalRegions[4].iTotalParts;
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"Lower Left - Middle Ring,%d,%d,%.1lf %%\n",iTotalDies,iTotalMatchingDies,lfPercentage);

        // Lower left - inner ring
        iTotalMatchingDies = cZonalRegions[3].iTotalMatch;
        iTotalDies = cZonalRegions[3].iTotalParts;
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"Lower Left - Inner Ring,%d,%d,%.1lf %%\n",iTotalDies,iTotalMatchingDies,lfPercentage);

        // Lower right - outer ring
        iTotalMatchingDies = cZonalRegions[8].iTotalMatch;
        iTotalDies = cZonalRegions[8].iTotalParts;
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"Lower Right - Outer Ring,%d,%d,%.1lf %%\n",iTotalDies,iTotalMatchingDies,lfPercentage);

        // Lower right - middle ring
        iTotalMatchingDies = cZonalRegions[7].iTotalMatch;
        iTotalDies = cZonalRegions[7].iTotalParts;
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"Lower Right - Middle Ring,%d,%d,%.1lf %%\n",iTotalDies,iTotalMatchingDies,lfPercentage);

        // Lower right - inner ring
        iTotalMatchingDies = cZonalRegions[6].iTotalMatch;
        iTotalDies = cZonalRegions[6].iTotalParts;
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"Lower Right - Inner Ring,%d,%d,%.1lf %%\n",iTotalDies,iTotalMatchingDies,lfPercentage);

        // Skip lines.
        fprintf(hReportFile,"\n\n");
    }
    else
    //if (of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
    if(m_pReportOptions->isReportOutputHtmlBased())
    {
        // Zonal Wafermap section...
        QString strBookmarkTitle, strBookmark;

        // Standard HTML: draw horizontal line (flat HTML use page breaks instead)
        if (of=="HTML")
            fprintf(hReportFile,"<hr><br>\n");

        if(m_pReportOptions->iGroups > 1)
        {
            strBookmarkTitle = "Zonal Wafer maps: " + pGroup->strGroupName;
            // Compute hyperlink to global info section (differs if flat HTML file, or multi-page HTML report)
            if (of=="HTML")
                strBookmark = "global.htm";
            else
                strBookmark = "all_globalinfo";
        }
        else
            strBookmarkTitle = "Zonal Wafer maps";

        // Title + bookmark
        WriteHtmlSectionTitle(hReportFile,strBookmark,strBookmarkTitle);

        if (of=="HTML")
            fprintf(hReportFile,"<table border=\"0\" width=\"98%%\">\n");
        else
            WriteHtmlOpenTable(98,0);	// HTML code to open table, Width 98%, cell spacing=0

        // Number of wafers.
        WriteInfoLine("Total Wafers",pGroup->cStackedWaferMapData.iTotalWafermaps);
        // Binning list.
        strString = m_pReportOptions->pGexWafermapRangeList->BuildTestListString("");
        strString += "    (for a different Bin list, see the <a href=\"_gex_file_settings.htm\">'Settings'</a> page, section 'wafermap')";
        switch(m_pReportOptions->iWafermapType)
        {
            case GEX_WAFMAP_ZONAL_SOFTBIN:
                WriteInfoLine("Soft-Bin list", strString.toLatin1().constData());
                break;
            case GEX_WAFMAP_ZONAL_HARDBIN:
                WriteInfoLine("Hard-Bin list", strString.toLatin1().constData());
                break;
        }

        // Total testing time.
        if(pGroup->cStackedWaferMapData.iTotaltime)
        {
            if(iDays)
                sprintf(szString,"%d days %d Hours %d Min",iDays,iHours,iMinutes);
            else
                sprintf(szString,"%d Hours %d Min",iHours,iMinutes);
            WriteInfoLine("Testing time",szString);
        }

        fprintf(hReportFile,"</tr>\n");
        fprintf(hReportFile,"</table>\n");

        fprintf(hReportFile,"<br>Wafer zones with Total die count, matching dies in zone, and yield level in zone:<br>\n");

        // Display bin counts per zone
        fprintf(hReportFile,"<br>\n");
        if (of=="HTML")
            fprintf(hReportFile,"<table border=\"0\" width=\"98%%\">\n");
        else
            WriteHtmlOpenTable(98,0);	// HTML code to open table, Width 98%, cell spacing=0
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td width=\"23%%\" bgcolor=%s align=\"left\"><b>Zone</b></td>\n",szFieldColor);
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\"><b>Total</b></td>\n",szFieldColor);
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\"><b>Match</b></td>\n",szFieldColor);
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\"><b>Percent</b></td>\n",szFieldColor);
        fprintf(hReportFile,"<td width=\"6%%\" align=\"center\">    </td>\n");
        fprintf(hReportFile,"<td width=\"23%%\" bgcolor=%s align=\"left\"><b>Zone</b></td>\n",szFieldColor);
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\"><b>Total</b></td>\n",szFieldColor);
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\"><b>Match</b></td>\n",szFieldColor);
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\"><b>Percent</b></td>\n",szFieldColor);
        fprintf(hReportFile,"</tr>\n");


        // Line1, Col1: Full wafer count
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td width=\"23%%\" bgcolor=%s align=\"left\">Full wafer</td>\n",szDataColor);
        double lfPercentage;
        for(iIndex=0;iIndex <12;iIndex++)
        {
            iTotalMatchingDies += cZonalRegions[iIndex].iTotalMatch;
            iTotalDies += cZonalRegions[iIndex].iTotalParts;
        }
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalDies);
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalMatchingDies);
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%.1lf %%</td>\n",szDataColor,lfPercentage);

        // mid-table column Separator
        fprintf(hReportFile,"<td width=\"6%%\" align=\"center\"></td>\n");

        // Line1, Col2: Upper left - outer ring
        fprintf(hReportFile,"<td width=\"23%%\" bgcolor=%s align=\"left\">Upper Left - Outer Ring</td>\n",szDataColor);
        iTotalMatchingDies = cZonalRegions[2].iTotalMatch;
        iTotalDies = cZonalRegions[2].iTotalParts;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalDies);
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalMatchingDies);
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%.1lf %%</td>\n",szDataColor,lfPercentage);
        fprintf(hReportFile,"</tr>\n");

        // Line2, Col1 : Upper wafer count
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td width=\"23%%\" bgcolor=%s align=\"left\">Upper Half wafer</td>\n",szDataColor);
        iTotalMatchingDies = cZonalRegions[0].iTotalMatch+cZonalRegions[1].iTotalMatch+cZonalRegions[2].iTotalMatch;
        iTotalMatchingDies += cZonalRegions[9].iTotalMatch+cZonalRegions[10].iTotalMatch+cZonalRegions[11].iTotalMatch;
        iTotalDies = cZonalRegions[0].iTotalParts+cZonalRegions[1].iTotalParts+cZonalRegions[2].iTotalParts;
        iTotalDies += cZonalRegions[9].iTotalParts+cZonalRegions[10].iTotalParts+cZonalRegions[11].iTotalParts;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalDies);
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalMatchingDies);
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%.1lf %%</td>\n",szDataColor,lfPercentage);

        // mid-table column Separator
        fprintf(hReportFile,"<td width=\"6%%\" align=\"center\"></td>\n");

        // Line2, Col2: Upper left - middle ring
        fprintf(hReportFile,"<td width=\"23%%\" bgcolor=%s align=\"left\">Upper Left - Middle Ring</td>\n",szDataColor);
        iTotalMatchingDies = cZonalRegions[1].iTotalMatch;
        iTotalDies = cZonalRegions[1].iTotalParts;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalDies);
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalMatchingDies);
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%.1lf %%</td>\n",szDataColor,lfPercentage);
        fprintf(hReportFile,"</tr>\n");

        // Line3, Col1 : Lower wafer count
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td width=\"23%%\" bgcolor=%s align=\"left\">Lower Half wafer</td>\n",szDataColor);
        iTotalMatchingDies = cZonalRegions[3].iTotalMatch+cZonalRegions[4].iTotalMatch+cZonalRegions[5].iTotalMatch;
        iTotalMatchingDies += cZonalRegions[6].iTotalMatch+cZonalRegions[7].iTotalMatch+cZonalRegions[8].iTotalMatch;
        iTotalDies = cZonalRegions[3].iTotalParts+cZonalRegions[4].iTotalParts+cZonalRegions[5].iTotalParts;
        iTotalDies += cZonalRegions[6].iTotalParts+cZonalRegions[7].iTotalParts+cZonalRegions[8].iTotalParts;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalDies);
        fprintf(hReportFile,"<td width=\"11%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalMatchingDies);
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%.1lf %%</td>\n",szDataColor,lfPercentage);

        // mid-table column Separator
        fprintf(hReportFile,"<td width=\"6%%\" align=\"center\"></td>\n");

        // Line3, Col2: Upper left - inner ring
        fprintf(hReportFile,"<td width=\"23%%\" bgcolor=%s align=\"left\">Upper Left - Inner Ring</td>\n",szDataColor);
        iTotalMatchingDies = cZonalRegions[0].iTotalMatch;
        iTotalDies = cZonalRegions[0].iTotalParts;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalDies);
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalMatchingDies);
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%.1lf %%</td>\n",szDataColor,lfPercentage);
        fprintf(hReportFile,"</tr>\n");

        // Line4, Col1 : Left half wafer count
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td width=\"23%%\" bgcolor=%s align=\"left\">Left Half wafer</td>\n",szDataColor);
        iTotalMatchingDies = cZonalRegions[0].iTotalMatch+cZonalRegions[1].iTotalMatch+cZonalRegions[2].iTotalMatch;
        iTotalMatchingDies += cZonalRegions[3].iTotalMatch+cZonalRegions[4].iTotalMatch+cZonalRegions[5].iTotalMatch;
        iTotalDies = cZonalRegions[0].iTotalParts+cZonalRegions[1].iTotalParts+cZonalRegions[2].iTotalParts;
        iTotalDies += cZonalRegions[3].iTotalParts+cZonalRegions[4].iTotalParts+cZonalRegions[5].iTotalParts;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalDies);
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalMatchingDies);
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%.1lf %%</td>\n",szDataColor,lfPercentage);

        // mid-table column Separator
        fprintf(hReportFile,"<td width=\"6%%\" align=\"center\"></td>\n");

        // Line4, Col2: Upper right - outer ring
        fprintf(hReportFile,"<td width=\"23%%\" bgcolor=%s align=\"left\">Upper Right - Outer Ring</td>\n",szDataColor);
        iTotalMatchingDies = cZonalRegions[11].iTotalMatch;
        iTotalDies = cZonalRegions[11].iTotalParts;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalDies);
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalMatchingDies);
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%.1lf %%</td>\n",szDataColor,lfPercentage);
        fprintf(hReportFile,"</tr>\n");

        // Line5, Col1 : Right half wafer count
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td width=\"23%%\" bgcolor=%s align=\"left\">Right Half wafer</td>\n",szDataColor);
        iTotalMatchingDies = cZonalRegions[6].iTotalMatch+cZonalRegions[7].iTotalMatch+cZonalRegions[8].iTotalMatch;
        iTotalMatchingDies += cZonalRegions[9].iTotalMatch+cZonalRegions[10].iTotalMatch+cZonalRegions[11].iTotalMatch;
        iTotalDies = cZonalRegions[6].iTotalParts+cZonalRegions[7].iTotalParts+cZonalRegions[8].iTotalParts;
        iTotalDies += cZonalRegions[9].iTotalParts+cZonalRegions[10].iTotalParts+cZonalRegions[11].iTotalParts;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalDies);
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalMatchingDies);
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%.1lf %%</td>\n",szDataColor,lfPercentage);

        // mid-table column Separator
        fprintf(hReportFile,"<td width=\"6%%\" align=\"center\"></td>\n");

        // Line5, Col2: Upper right - middle ring
        fprintf(hReportFile,"<td width=\"23%%\" bgcolor=%s align=\"left\">Upper Right - Middle Ring</td>\n",szDataColor);
        iTotalMatchingDies = cZonalRegions[10].iTotalMatch;
        iTotalDies = cZonalRegions[10].iTotalParts;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalDies);
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalMatchingDies);
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%.1lf %%</td>\n",szDataColor,lfPercentage);
        fprintf(hReportFile,"</tr>\n");

        // Line6, Col1 : Upper left wafer count
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td width=\"23%%\" bgcolor=%s align=\"left\">Upper Left slice</td>\n",szDataColor);
        iTotalMatchingDies = cZonalRegions[0].iTotalMatch+cZonalRegions[1].iTotalMatch+cZonalRegions[2].iTotalMatch;
        iTotalDies = cZonalRegions[0].iTotalParts+cZonalRegions[1].iTotalParts+cZonalRegions[2].iTotalParts;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalDies);
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalMatchingDies);
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%.1lf %%</td>\n",szDataColor,lfPercentage);

        // mid-table column Separator
        fprintf(hReportFile,"<td width=\"6%%\" align=\"center\"></td>\n");

        // Line6, Col2: Upper right - inner ring
        fprintf(hReportFile,"<td width=\"23%%\" bgcolor=%s align=\"left\">Upper Right - Inner Ring</td>\n",szDataColor);
        iTotalMatchingDies = cZonalRegions[9].iTotalMatch;
        iTotalDies = cZonalRegions[9].iTotalParts;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalDies);
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalMatchingDies);
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%.1lf %%</td>\n",szDataColor,lfPercentage);
        fprintf(hReportFile,"</tr>\n");

        // Line7, Col1 : Upper right wafer count
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td width=\"23%%\" bgcolor=%s align=\"left\">Upper Right slice</td>\n",szDataColor);
        iTotalMatchingDies = cZonalRegions[9].iTotalMatch+cZonalRegions[10].iTotalMatch+cZonalRegions[11].iTotalMatch;
        iTotalDies = cZonalRegions[9].iTotalParts+cZonalRegions[10].iTotalParts+cZonalRegions[11].iTotalParts;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalDies);
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalMatchingDies);
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%.1lf %%</td>\n",szDataColor,lfPercentage);

        // mid-table column Separator
        fprintf(hReportFile,"<td width=\"6%%\" align=\"center\"></td>\n");

        // Line7, Col2: Lower left - outerring
        fprintf(hReportFile,"<td width=\"23%%\" bgcolor=%s align=\"left\">Lower Left - Outer Ring</td>\n",szDataColor);
        iTotalMatchingDies = cZonalRegions[5].iTotalMatch;
        iTotalDies = cZonalRegions[5].iTotalParts;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalDies);
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalMatchingDies);
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%.1lf %%</td>\n",szDataColor,lfPercentage);
        fprintf(hReportFile,"</tr>\n");

        // Line8, Col1 : Lower left wafer count
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td width=\"23%%\" bgcolor=%s align=\"left\">Lower Left slice</td>\n",szDataColor);
        iTotalMatchingDies = cZonalRegions[3].iTotalMatch+cZonalRegions[4].iTotalMatch+cZonalRegions[5].iTotalMatch;
        iTotalDies = cZonalRegions[3].iTotalParts+cZonalRegions[4].iTotalParts+cZonalRegions[5].iTotalParts;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalDies);
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalMatchingDies);
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%.1lf %%</td>\n",szDataColor,lfPercentage);

        // mid-table column Separator
        fprintf(hReportFile,"<td width=\"6%%\" align=\"center\"></td>\n");

        // Line8, Col2: Lower left - middle  ring
        fprintf(hReportFile,"<td width=\"23%%\" bgcolor=%s align=\"left\">Lower Left - Middle Ring</td>\n",szDataColor);
        iTotalMatchingDies = cZonalRegions[4].iTotalMatch;
        iTotalDies = cZonalRegions[4].iTotalParts;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalDies);
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalMatchingDies);
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%.1lf %%</td>\n",szDataColor,lfPercentage);
        fprintf(hReportFile,"</tr>\n");

        // Line9, Col1 : Lower right wafer count
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td width=\"23%%\" bgcolor=%s align=\"left\">Lower Right slice</td>\n",szDataColor);
        iTotalMatchingDies = cZonalRegions[6].iTotalMatch+cZonalRegions[7].iTotalMatch+cZonalRegions[8].iTotalMatch;
        iTotalDies = cZonalRegions[6].iTotalParts+cZonalRegions[7].iTotalParts+cZonalRegions[8].iTotalParts;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalDies);
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalMatchingDies);
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%.1lf %%</td>\n",szDataColor,lfPercentage);

        // mid-table column Separator
        fprintf(hReportFile,"<td width=\"6%%\" align=\"center\"></td>\n");

        // Line9, Col2: Lower left - inner  ring
        fprintf(hReportFile,"<td width=\"23%%\" bgcolor=%s align=\"left\">Lower Left - Inner Ring</td>\n",szDataColor);
        iTotalMatchingDies = cZonalRegions[3].iTotalMatch;
        iTotalDies = cZonalRegions[3].iTotalParts;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalDies);
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalMatchingDies);
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%.1lf %%</td>\n",szDataColor,lfPercentage);
        fprintf(hReportFile,"</tr>\n");

        // Line10, Col1 : Outer Ring
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td width=\"23%%\" bgcolor=%s align=\"left\">Outer Ring</td>\n",szDataColor);
        iTotalMatchingDies = cZonalRegions[5].iTotalMatch+cZonalRegions[2].iTotalMatch+cZonalRegions[11].iTotalMatch+cZonalRegions[8].iTotalMatch;
        iTotalDies = cZonalRegions[5].iTotalParts+cZonalRegions[2].iTotalParts+cZonalRegions[11].iTotalParts+cZonalRegions[8].iTotalParts;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalDies);
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalMatchingDies);
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%.1lf %%</td>\n",szDataColor,lfPercentage);
        // mid-table column Separator
        fprintf(hReportFile,"<td width=\"6%%\" align=\"center\"></td>\n");

        // Line10, Col2: Lower right - outer  ring
        fprintf(hReportFile,"<td width=\"23%%\" bgcolor=%s align=\"left\">Lower Right - Outer Ring</td>\n",szDataColor);
        iTotalMatchingDies = cZonalRegions[8].iTotalMatch;
        iTotalDies = cZonalRegions[8].iTotalParts;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalDies);
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalMatchingDies);
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%.1lf %%</td>\n",szDataColor,lfPercentage);
        fprintf(hReportFile,"</tr>\n");

        // Line11, Col1 : Middle Ring
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td width=\"23%%\" bgcolor=%s align=\"left\">Middle Ring</td>\n",szDataColor);
        iTotalMatchingDies = cZonalRegions[4].iTotalMatch+cZonalRegions[1].iTotalMatch+cZonalRegions[10].iTotalMatch+cZonalRegions[7].iTotalMatch;
        iTotalDies = cZonalRegions[4].iTotalParts+cZonalRegions[1].iTotalParts+cZonalRegions[10].iTotalParts+cZonalRegions[7].iTotalParts;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalDies);
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalMatchingDies);
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%.1lf %%</td>\n",szDataColor,lfPercentage);
        // mid-table column Separator
        fprintf(hReportFile,"<td width=\"6%%\" align=\"center\"></td>\n");

        // Line11, Col2: Lower right - middle  ring
        fprintf(hReportFile,"<td width=\"23%%\" bgcolor=%s align=\"left\">Lower Right - Middle Ring</td>\n",szDataColor);
        iTotalMatchingDies = cZonalRegions[7].iTotalMatch;
        iTotalDies = cZonalRegions[7].iTotalParts;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalDies);
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalMatchingDies);
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%.1lf %%</td>\n",szDataColor,lfPercentage);
        fprintf(hReportFile,"</tr>\n");

        // Line12, Col1 : Inner Ring
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td width=\"23%%\" bgcolor=%s align=\"left\">Inner Ring</td>\n",szDataColor);
        iTotalMatchingDies = cZonalRegions[3].iTotalMatch+cZonalRegions[0].iTotalMatch+cZonalRegions[9].iTotalMatch+cZonalRegions[6].iTotalMatch;
        iTotalDies = cZonalRegions[3].iTotalParts+cZonalRegions[0].iTotalParts+cZonalRegions[9].iTotalParts+cZonalRegions[6].iTotalParts;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalDies);
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalMatchingDies);
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%.1lf %%</td>\n",szDataColor,lfPercentage);
        // mid-table column Separator
        fprintf(hReportFile,"<td width=\"6%%\" align=\"center\"></td>\n");

        // Line12, Col2: Lower right - inner  ring
        fprintf(hReportFile,"<td width=\"23%%\" bgcolor=%s align=\"left\">Lower Right - Inner Ring</td>\n",szDataColor);
        iTotalMatchingDies = cZonalRegions[6].iTotalMatch;
        iTotalDies = cZonalRegions[6].iTotalParts;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalDies);
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalMatchingDies);
        if(iTotalDies)
            lfPercentage = 100.0*iTotalMatchingDies/(double)iTotalDies;
        else
            lfPercentage=0;
        fprintf(hReportFile,"<td width=\"8%%\" bgcolor=%s align=\"center\">%.1lf %%</td>\n",szDataColor,lfPercentage);
        fprintf(hReportFile,"</tr>\n");

        // Close table
        fprintf(hReportFile,"</table>\n");

        // Standard HTML: skip 2 line)
        if (of=="HTML")
            fprintf(hReportFile,"<br><br>\n");
        else
            WritePageBreak();	// Flat HTML

        // Zonal Binning wafermap is using the same gadient color scale as Parametric test wafermap...
        // Show colored scale.
        fprintf(hReportFile,"Color scale: Yield level per zone<br>\n");
        fprintf(hReportFile,"<img border=\"0\" src=\"../images/zoning.png\"><br>\n");
        fprintf(hReportFile,"<table border=\"0\" width=\"300\" cellspacing=\"0\">\n");
        fprintf(hReportFile,"<tr>\n");
        // Few labels on scale.
        float fStep = (fMaxPercentage-fMinPercentage)/4;

        fprintf(hReportFile,"<td width=\"60\" align=\"left\">%.1f%%</td>\n",fMinPercentage);
        fprintf(hReportFile,"<td width=\"60\" align=\"left\">%.1f%%</td>\n",fMinPercentage+fStep);
        fprintf(hReportFile,"<td width=\"60\" align=\"center\">%.1f%%</td>\n",fMinPercentage+2*fStep);
        fprintf(hReportFile,"<td width=\"60\" align=\"center\">%.1f%%</td>\n",fMinPercentage+3*fStep);
        fprintf(hReportFile,"<td width=\"60\" align=\"right\">%.1f%%</td>\n",fMaxPercentage);
        fprintf(hReportFile,"</tr>\n");
        fprintf(hReportFile,"</table>\n");

        // Creates the Zonal wafermap PNG file: 'zonalwaf<group#>.png'
        QString		strImageName ;
        QString		strImageFile;
        strImageName = "zonalwaf";
        strImageName += QString::number(iGroupID);
        strImageName += ".png";

        strImageFile = m_pReportOptions->strReportDirectory;
        strImageFile += "/images/";
        strImageFile += strImageName;

        // Create Wafer zonal images.
        QPainter p;						// To draw into wafermap image
        QPixmap	*pm = new QPixmap( 800, 501);
        QColor *sColor = CreateColorSpectrum();

        // fills with White color
        pm->fill(QColor(255,255,255));

        // Begin painting task
        p.begin(pm);
        p.setPen(QColor(Qt::black));
        p.setFont(QFont("Helvetica", 8));

        // Draw Zonal Wafers:
        QRect rWafer1(10,10,240,240);
        QRect rWafer2(260,10,240,240);
        QRect rWafer3(510,10,240,240);
        QRect rWafer4(10,260,240,240);
        QRect rWafer5_LR(260,260,240,240);	// Large ring
        QRect rWafer5_MR(300,300,160,160);	// Medium ring
        QRect rWafer5_SR(340,340,80,80);	// Small ring
        QRect rWafer6_LR(510,260,240,240);	// Large ring
        QRect rWafer6_MR(550,300,160,160);	// Medium ring
        QRect rWafer6_SR(590,340,80,80);	// Small ring

        // Zonal Wafer1: overall yield
        iTotalDies=0,iTotalMatchingDies=0;
        QString strText;
        int	iColorIndex;
        for(iIndex=0;iIndex <12;iIndex++)
        {
            iTotalMatchingDies += cZonalRegions[iIndex].iTotalMatch;
            iTotalDies += cZonalRegions[iIndex].iTotalParts;
        }
        iColorIndex = getPercentageZone(iTotalMatchingDies,iTotalDies,fMinPercentage,fMaxPercentage,strText);
        p.setBrush(QBrush(sColor[iColorIndex]));
        p.drawEllipse(rWafer1);
        p.drawText(rWafer1,Qt::AlignCenter,strText);

        // Zonal Wafer2: Top half, Bottom half
        iTotalMatchingDies = cZonalRegions[0].iTotalMatch+cZonalRegions[1].iTotalMatch+cZonalRegions[2].iTotalMatch;
        iTotalMatchingDies += cZonalRegions[9].iTotalMatch+cZonalRegions[10].iTotalMatch+cZonalRegions[11].iTotalMatch;
        iTotalDies = cZonalRegions[0].iTotalParts+cZonalRegions[1].iTotalParts+cZonalRegions[2].iTotalParts;
        iTotalDies += cZonalRegions[9].iTotalParts+cZonalRegions[10].iTotalParts+cZonalRegions[11].iTotalParts;
        iColorIndex = getPercentageZone(iTotalMatchingDies,iTotalDies,fMinPercentage,fMaxPercentage,strText);
        p.setBrush(QBrush(sColor[iColorIndex]));
        p.drawPie(rWafer2,0,180*16);		// Top half
        p.drawText(rWafer2.center().x()-50,rWafer2.top()+rWafer2.height()/4,100,20,Qt::AlignCenter,strText);

        iTotalMatchingDies = cZonalRegions[3].iTotalMatch+cZonalRegions[4].iTotalMatch+cZonalRegions[5].iTotalMatch;
        iTotalMatchingDies += cZonalRegions[6].iTotalMatch+cZonalRegions[7].iTotalMatch+cZonalRegions[8].iTotalMatch;
        iTotalDies = cZonalRegions[3].iTotalParts+cZonalRegions[4].iTotalParts+cZonalRegions[5].iTotalParts;
        iTotalDies += cZonalRegions[6].iTotalParts+cZonalRegions[7].iTotalParts+cZonalRegions[8].iTotalParts;
        iColorIndex = getPercentageZone(iTotalMatchingDies,iTotalDies,fMinPercentage,fMaxPercentage,strText);
        p.setBrush(QBrush(sColor[iColorIndex]));
        p.drawPie(rWafer2,0,-180*16);		// Bottom half
        p.drawText(rWafer2.center().x()-50,rWafer2.bottom()-rWafer2.height()/4,100,20,Qt::AlignCenter,strText);

        // Zonal Wafer3: Left half, Right half
        iTotalMatchingDies = cZonalRegions[0].iTotalMatch+cZonalRegions[1].iTotalMatch+cZonalRegions[2].iTotalMatch;
        iTotalMatchingDies += cZonalRegions[3].iTotalMatch+cZonalRegions[4].iTotalMatch+cZonalRegions[5].iTotalMatch;
        iTotalDies = cZonalRegions[0].iTotalParts+cZonalRegions[1].iTotalParts+cZonalRegions[2].iTotalParts;
        iTotalDies += cZonalRegions[3].iTotalParts+cZonalRegions[4].iTotalParts+cZonalRegions[5].iTotalParts;
        iColorIndex = getPercentageZone(iTotalMatchingDies,iTotalDies,fMinPercentage,fMaxPercentage,strText);
        p.setBrush(QBrush(sColor[iColorIndex]));
        p.drawPie(rWafer3,90*16,180*16);	// Left half
        p.drawText(rWafer3.left()+(rWafer3.width()/4)-50,rWafer3.center().y(),100,20,Qt::AlignCenter,strText);

        iTotalMatchingDies = cZonalRegions[9].iTotalMatch+cZonalRegions[10].iTotalMatch+cZonalRegions[11].iTotalMatch;
        iTotalMatchingDies += cZonalRegions[6].iTotalMatch+cZonalRegions[7].iTotalMatch+cZonalRegions[8].iTotalMatch;
        iTotalDies = cZonalRegions[9].iTotalParts+cZonalRegions[10].iTotalParts+cZonalRegions[11].iTotalParts;
        iTotalDies += cZonalRegions[6].iTotalParts+cZonalRegions[7].iTotalParts+cZonalRegions[8].iTotalParts;
        iColorIndex = getPercentageZone(iTotalMatchingDies,iTotalDies,fMinPercentage,fMaxPercentage,strText);
        p.setBrush(QBrush(sColor[iColorIndex]));
        p.drawPie(rWafer3,90*16,-180*16);	// Right half
        p.drawText(rWafer3.right()-50-(rWafer3.width()/4),rWafer3.center().y(),100,20,Qt::AlignCenter,strText);

        // Zonal Wafer4: Left,Right,Top, Bottom areas
        iTotalMatchingDies = cZonalRegions[9].iTotalMatch+cZonalRegions[10].iTotalMatch+cZonalRegions[11].iTotalMatch;
        iTotalDies = cZonalRegions[9].iTotalParts+cZonalRegions[10].iTotalParts+cZonalRegions[11].iTotalParts;
        iColorIndex = getPercentageZone(iTotalMatchingDies,iTotalDies,fMinPercentage,fMaxPercentage,strText);
        p.setBrush(QBrush(sColor[iColorIndex]));
        p.drawPie(rWafer4,0,90*16);			// Top-Right
        p.drawText(rWafer4.right()-(rWafer4.width()/4)-50,rWafer4.top()+(rWafer4.height()/4),100,20,Qt::AlignCenter,strText);

        iTotalMatchingDies = cZonalRegions[0].iTotalMatch+cZonalRegions[1].iTotalMatch+cZonalRegions[2].iTotalMatch;
        iTotalDies = cZonalRegions[0].iTotalParts+cZonalRegions[1].iTotalParts+cZonalRegions[2].iTotalParts;
        iColorIndex = getPercentageZone(iTotalMatchingDies,iTotalDies,fMinPercentage,fMaxPercentage,strText);
        p.setBrush(QBrush(sColor[iColorIndex]));
        p.drawPie(rWafer4,90*16,90*16);		// Top-Left
        p.drawText(rWafer4.left()+(rWafer4.width()/4)-50,rWafer4.top()+(rWafer4.height()/4),100,20,Qt::AlignCenter,strText);

        iTotalMatchingDies = cZonalRegions[3].iTotalMatch+cZonalRegions[4].iTotalMatch+cZonalRegions[5].iTotalMatch;
        iTotalDies = cZonalRegions[3].iTotalParts+cZonalRegions[4].iTotalParts+cZonalRegions[5].iTotalParts;
        iColorIndex = getPercentageZone(iTotalMatchingDies,iTotalDies,fMinPercentage,fMaxPercentage,strText);
        p.setBrush(QBrush(sColor[iColorIndex]));
        p.drawPie(rWafer4,180*16,90*16);	// Bottom-Left
        p.drawText(rWafer4.left()+(rWafer4.width()/4)-50,rWafer4.bottom()-(rWafer4.height()/4),100,20,Qt::AlignCenter,strText);

        iTotalMatchingDies = cZonalRegions[6].iTotalMatch+cZonalRegions[7].iTotalMatch+cZonalRegions[8].iTotalMatch;
        iTotalDies = cZonalRegions[6].iTotalParts+cZonalRegions[7].iTotalParts+cZonalRegions[8].iTotalParts;
        iColorIndex = getPercentageZone(iTotalMatchingDies,iTotalDies,fMinPercentage,fMaxPercentage,strText);
        p.setBrush(QBrush(sColor[iColorIndex]));
        p.drawPie(rWafer4,270*16,90*16);	// Bottom-Right
        p.drawText(rWafer4.right()-(rWafer4.width()/4)-50,rWafer4.bottom()-(rWafer4.height()/4),100,20,Qt::AlignCenter,strText);

        // Zonal Wafer5: 3 rings
        iTotalMatchingDies = cZonalRegions[2].iTotalMatch+cZonalRegions[11].iTotalMatch+cZonalRegions[5].iTotalMatch+cZonalRegions[8].iTotalMatch;
        iTotalDies = cZonalRegions[2].iTotalParts+cZonalRegions[11].iTotalParts+cZonalRegions[5].iTotalParts+cZonalRegions[8].iTotalParts;
        iColorIndex = getPercentageZone(iTotalMatchingDies,iTotalDies,fMinPercentage,fMaxPercentage,strText);
        p.setBrush(QBrush(sColor[iColorIndex]));
        p.drawEllipse(rWafer5_LR);			// Outside/Large ring
        p.drawText(rWafer5_LR.center().x()-45,rWafer5_LR.top()+20,100,20,Qt::AlignCenter,strText);

        iTotalMatchingDies = cZonalRegions[1].iTotalMatch+cZonalRegions[4].iTotalMatch+cZonalRegions[7].iTotalMatch+cZonalRegions[10].iTotalMatch;
        iTotalDies = cZonalRegions[1].iTotalParts+cZonalRegions[4].iTotalParts+cZonalRegions[7].iTotalParts+cZonalRegions[10].iTotalParts;
        iColorIndex = getPercentageZone(iTotalMatchingDies,iTotalDies,fMinPercentage,fMaxPercentage,strText);
        p.setBrush(QBrush(sColor[iColorIndex]));
        p.drawEllipse(rWafer5_MR);			// Medium ring
        p.drawText(rWafer5_MR.center().x()-45,rWafer5_MR.top()+20,100,20,Qt::AlignCenter,strText);

        iTotalMatchingDies = cZonalRegions[0].iTotalMatch+cZonalRegions[3].iTotalMatch+cZonalRegions[6].iTotalMatch+cZonalRegions[9].iTotalMatch;
        iTotalDies = cZonalRegions[0].iTotalParts+cZonalRegions[3].iTotalParts+cZonalRegions[6].iTotalParts+cZonalRegions[9].iTotalParts;
        iColorIndex = getPercentageZone(iTotalMatchingDies,iTotalDies,fMinPercentage,fMaxPercentage,strText);
        p.setBrush(QBrush(sColor[iColorIndex]));
        p.drawEllipse(rWafer5_SR);			// Internal/Small ring
        p.drawText(rWafer5_SR.center().x()-45,rWafer5_SR.center().y()-5,100,20,Qt::AlignCenter,strText);

        // Zonal Wafer6: 3 rings, and Left,Right,Top, Bottom areas
        iTotalMatchingDies = cZonalRegions[11].iTotalMatch,
        iTotalDies = cZonalRegions[11].iTotalParts;
        iColorIndex = getPercentageZone(iTotalMatchingDies,iTotalDies,fMinPercentage,fMaxPercentage,strText);
        p.setBrush(QBrush(sColor[iColorIndex]));
        p.drawPie(rWafer6_LR,0,90*16);		// Large ring: Top-Right
        p.drawText(rWafer6_LR.left()+150,rWafer6_LR.top()+50,100,20,Qt::AlignCenter,strText);

        iTotalMatchingDies = cZonalRegions[2].iTotalMatch,
        iTotalDies = cZonalRegions[2].iTotalParts;
        iColorIndex = getPercentageZone(iTotalMatchingDies,iTotalDies,fMinPercentage,fMaxPercentage,strText);
        p.setBrush(QBrush(sColor[iColorIndex]));
        p.drawPie(rWafer6_LR,90*16,90*16);	// Large ring: Top-Left
        p.drawText(rWafer6_LR.left()-10,rWafer6_LR.top()+50,100,20,Qt::AlignCenter,strText);

        iTotalMatchingDies = cZonalRegions[5].iTotalMatch,
        iTotalDies = cZonalRegions[5].iTotalParts;
        iColorIndex = getPercentageZone(iTotalMatchingDies,iTotalDies,fMinPercentage,fMaxPercentage,strText);
        p.setBrush(QBrush(sColor[iColorIndex]));
        p.drawPie(rWafer6_LR,180*16,90*16);	// Large ring: Bottom-Left
        p.drawText(rWafer6_LR.left()-10,rWafer6_LR.top()+175,100,20,Qt::AlignCenter,strText);

        iTotalMatchingDies = cZonalRegions[8].iTotalMatch,
        iTotalDies = cZonalRegions[8].iTotalParts;
        iColorIndex = getPercentageZone(iTotalMatchingDies,iTotalDies,fMinPercentage,fMaxPercentage,strText);
        p.setBrush(QBrush(sColor[iColorIndex]));
        p.drawPie(rWafer6_LR,270*16,90*16);	// Large ring: Bottom-Right
        p.drawText(rWafer6_LR.left()+150,rWafer6_LR.top()+175,100,20,Qt::AlignCenter,strText);

        iTotalMatchingDies = cZonalRegions[10].iTotalMatch,
        iTotalDies = cZonalRegions[10].iTotalParts;
        iColorIndex = getPercentageZone(iTotalMatchingDies,iTotalDies,fMinPercentage,fMaxPercentage,strText);
        p.setBrush(QBrush(sColor[iColorIndex]));
        p.drawPie(rWafer6_MR,0,90*16);		// Medium ring: Top-Right
        p.drawText(rWafer6_LR.left()+120,rWafer6_LR.top()+80,100,20,Qt::AlignCenter,strText);

        iTotalMatchingDies = cZonalRegions[1].iTotalMatch,
        iTotalDies = cZonalRegions[1].iTotalParts;
        iColorIndex = getPercentageZone(iTotalMatchingDies,iTotalDies,fMinPercentage,fMaxPercentage,strText);
        p.setBrush(QBrush(sColor[iColorIndex]));
        p.drawPie(rWafer6_MR,90*16,90*16);	// Medium ring: Top-Left
        p.drawText(rWafer6_LR.left()+20,rWafer6_LR.top()+80,100,20,Qt::AlignCenter,strText);

        iTotalMatchingDies = cZonalRegions[4].iTotalMatch,
        iTotalDies = cZonalRegions[4].iTotalParts;
        iColorIndex = getPercentageZone(iTotalMatchingDies,iTotalDies,fMinPercentage,fMaxPercentage,strText);
        p.setBrush(QBrush(sColor[iColorIndex]));
        p.drawPie(rWafer6_MR,180*16,90*16);	// Medium ring: Bottom-Left
        p.drawText(rWafer6_LR.left()+20,rWafer6_LR.top()+145,100,20,Qt::AlignCenter,strText);

        iTotalMatchingDies = cZonalRegions[7].iTotalMatch,
        iTotalDies = cZonalRegions[7].iTotalParts;
        iColorIndex = getPercentageZone(iTotalMatchingDies,iTotalDies,fMinPercentage,fMaxPercentage,strText);
        p.setBrush(QBrush(sColor[iColorIndex]));
        p.drawPie(rWafer6_MR,270*16,90*16);	// Medium ring: Bottom-Right
        p.drawText(rWafer6_LR.left()+120,rWafer6_LR.top()+145,100,20,Qt::AlignCenter,strText);

        iTotalMatchingDies = cZonalRegions[9].iTotalMatch,
        iTotalDies = cZonalRegions[9].iTotalParts;
        iColorIndex = getPercentageZone(iTotalMatchingDies,iTotalDies,fMinPercentage,fMaxPercentage,strText);
        p.setBrush(QBrush(sColor[iColorIndex]));
        p.drawPie(rWafer6_SR,0,90*16);		// Small ring: Top-Right
        p.drawText(rWafer6_LR.left()+90,rWafer6_LR.top()+100,100,20,Qt::AlignCenter,strText);

        iTotalMatchingDies = cZonalRegions[0].iTotalMatch,
        iTotalDies = cZonalRegions[0].iTotalParts;
        iColorIndex = getPercentageZone(iTotalMatchingDies,iTotalDies,fMinPercentage,fMaxPercentage,strText);
        p.setBrush(QBrush(sColor[iColorIndex]));
        p.drawPie(rWafer6_SR,90*16,90*16);	// Small ring: Top-Left
        p.drawText(rWafer6_LR.left()+50,rWafer6_LR.top()+100,100,20,Qt::AlignCenter,strText);

        iTotalMatchingDies = cZonalRegions[3].iTotalMatch,
        iTotalDies = cZonalRegions[3].iTotalParts;
        iColorIndex = getPercentageZone(iTotalMatchingDies,iTotalDies,fMinPercentage,fMaxPercentage,strText);
        p.setBrush(QBrush(sColor[iColorIndex]));
        p.drawPie(rWafer6_SR,180*16,90*16);	// Small ring: Bottom-Left
        p.drawText(rWafer6_LR.left()+50,rWafer6_LR.top()+125,100,20,Qt::AlignCenter,strText);

        iTotalMatchingDies = cZonalRegions[6].iTotalMatch,
        iTotalDies = cZonalRegions[6].iTotalParts;
        iColorIndex = getPercentageZone(iTotalMatchingDies,iTotalDies,fMinPercentage,fMaxPercentage,strText);
        p.setBrush(QBrush(sColor[iColorIndex]));
        p.drawPie(rWafer6_SR,270*16,90*16);	// Small ring: Bottom-Right
        p.drawText(rWafer6_LR.left()+90,rWafer6_LR.top()+125,100,20,Qt::AlignCenter,strText);

        // Save to disk.
        p.end();
        pm->save(strImageFile,"PNG");
        fprintf(hReportFile,"<img border=\"0\" src=\"../images/%s\" ><br><br>\n", formatHtmlImageFilename(strImageName).toLatin1().constData());

        // Free memory
        delete pm; pm=0;
        delete []sColor;
    }

    //if (of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
    if(m_pReportOptions->isReportOutputHtmlBased())
    {
        bool bIndividualWafermap = m_pReportOptions->GetOption("wafer", "chart_show").toString().split("|").contains("all_individual", Qt::CaseSensitive);

        // If ZONAL wafer exists and is enabled and INDIVIDUAL wafermap disabled, show Info message on HOW to also display individual wafermaps.
        if((bIndividualWafermap == false)
                && (pGroup->cStackedWaferMapData.iTotalWafermaps > 1))
            fprintf(hReportFile,"<br>Note: To see ALL individual wafermaps, check the <a href=\"_gex_options.htm\">'Options'</a> page, section 'wafermap / Wafermaps to include in report'<br>\n");
    }

    // Standard HTML: draw horizontal line (flat HTML use page breaks instead)
    if (of=="HTML")
        fprintf(hReportFile,"<br><hr><br>\n");

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// STACK wafermaps of each group in 1st group: PAT-Man only
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::StackedGroupsWaferMap(void)
{
    int	iBin;
    int	iIndex;

    CGexGroupOfFiles *	pGroup						= NULL;
    CGexFileInGroup *	pFile						= NULL;
    pGroup = getGroupsList().isEmpty()?NULL:getGroupsList().first();
    pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

    // Get copy of combined wafermap (includes PAT binning).
    CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();
    if (!lPatInfo)
    {
         GSLOG(3, "PatInfo NULL");
         return;
    }
    WaferMapAllData = lPatInfo->m_AllSitesMap_Sbin;

    // Merged soft(or hard) bins of all groups into group#1 + merge parts in all goups (used for 3D wafermap!)
    // Iterator on Groups list
    QListIterator<CGexGroupOfFiles*> itGroupsList(getGroupsList());
    itGroupsList.toFront();

    CGexGroupOfFiles *	pGroup1				= itGroupsList.next();
    CGexFileInGroup  *	pFile1				= (pGroup1->pFilesList.isEmpty()) ? NULL : pGroup1->pFilesList.first();
    CBinning *			ptNewBinItem		= NULL;
    CBinning *			ptMasterBinList		= NULL;
    CBinning *			ptMasterBinListTail	= NULL;
    CBinning *			ptBinList			= NULL;
    bool				bBinAlreadyExists;

    while(itGroupsList.hasNext())
    {
        // Handle to file in group
        pGroup	= itGroupsList.next();
        pFile	= (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

        // Keep track of total bins
        if(lPatInfo->GetRecipeOptions().iReport_WafermapType)
        {
            pGroup1->cMergedData.lTotalHardBins += pGroup->cMergedData.lTotalHardBins;
            ptBinList= pGroup->cMergedData.ptMergedHardBinList; // Points first HARD binning structure
        }
        else
        {
            pGroup1->cMergedData.lTotalSoftBins += pGroup->cMergedData.lTotalSoftBins;
            ptBinList= pGroup->cMergedData.ptMergedSoftBinList; // Points first SOFT binning structure
        }

        while(ptBinList)
        {
            // Check if each bin list entry exists in global list (group#1)
            bBinAlreadyExists = false;

            if(lPatInfo->GetRecipeOptions().iReport_WafermapType)
            {
                ptMasterBinList= pGroup1->cMergedData.ptMergedSoftBinList; // Points first SOFT binning structure in group#1 (master bin list to update)
            }
            else
            {
                ptMasterBinList= pGroup1->cMergedData.ptMergedHardBinList; // Points first SOFT binning structure in group#1 (master bin list to update)
            }

            while(ptMasterBinList)
            {
                // Check if this bin is already in the master list
                if(ptMasterBinList->iBinValue == ptBinList->iBinValue)
                {
                    bBinAlreadyExists = true;
                    break;
                }

                // Move to next Bin# in master list
                ptMasterBinListTail = ptMasterBinList;
                ptMasterBinList = ptMasterBinList->ptNextBin;
            };

            // If Bin# was not in master list, add it.
            if(bBinAlreadyExists == false)
            {
                // Add Bin cell to the list (append to the end)
                ptNewBinItem = new CBinning;
                ptNewBinItem->iBinValue = ptBinList->iBinValue;
                ptNewBinItem->cPassFail = ptBinList->cPassFail;
                ptNewBinItem->strBinName= ptBinList->strBinName;
                ptNewBinItem->ptNextBin = NULL;
                if(ptMasterBinListTail == NULL)
                {
                    if(lPatInfo->GetRecipeOptions().iReport_WafermapType)
                        pGroup1->cMergedData.ptMergedHardBinList = ptNewBinItem;
                    else
                        pGroup1->cMergedData.ptMergedSoftBinList = ptNewBinItem;
                }
                else
                    ptMasterBinListTail->ptNextBin = ptNewBinItem;
            }

            // Move to next Bin# in current group
            ptBinList = ptBinList->ptNextBin;
        };

        // Merge total part count into Group#1 (collapse)
        pFile1->getPcrDatas().lPartCount += pFile->getPcrDatas().lPartCount;
    };

    // Reset die count
    if(lPatInfo->GetRecipeOptions().iReport_WafermapType)
        ptMasterBinList= pGroup1->cMergedData.ptMergedHardBinList; // Points first HARD binning structure in group#1 (master bin list to update)
    else
        ptMasterBinList= pGroup1->cMergedData.ptMergedSoftBinList; // Points first SOFT binning structure in group#1 (master bin list to update)
    while(ptMasterBinList)
    {
        // Clear all die counts
        ptMasterBinList->ldTotalCount = 0;
        ptMasterBinList = ptMasterBinList->ptNextBin;
    };

    // handle to first group!
    pGroup	= getGroupsList().isEmpty()?NULL:getGroupsList().first();
    pFile	= (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

    // count all bins in merged wafermap produced...
    WaferMapAllData.iTotalPhysicalDies = 0;
    if(lPatInfo->GetRecipeOptions().iReport_WafermapType)
        pGroup1->cMergedData.lTotalHardBins = 0;
    else
        pGroup1->cMergedData.lTotalSoftBins = 0;

    for(iIndex=0;iIndex < WaferMapAllData.SizeX*WaferMapAllData.SizeY;iIndex++)
    {
        // Update Die count (in combined group1 wafermap) for given binning
        iBin = WaferMapAllData.getWafMap()[iIndex].getBin();
        if(iBin != GEX_WAFMAP_EMPTY_CELL)
        {
            // Keep track of die count
            WaferMapAllData.iTotalPhysicalDies++;

            // Update relevant bin count.
            if(lPatInfo->GetRecipeOptions().iReport_WafermapType)
                ptMasterBinList= pGroup1->cMergedData.ptMergedHardBinList; // Points first HARD binning structure in group#1 (master bin list to update)
            else
                ptMasterBinList= pGroup1->cMergedData.ptMergedSoftBinList; // Points first SOFT binning structure in group#1 (master bin list to update)
            while(ptMasterBinList)
            {
                if(ptMasterBinList->iBinValue == iBin)
                {
                    ptMasterBinList->ldTotalCount++;
                    if(lPatInfo->GetRecipeOptions().iReport_WafermapType)
                        pGroup1->cMergedData.lTotalHardBins++;
                    else
                        pGroup1->cMergedData.lTotalSoftBins++;
                    break;
                }
                // Next Bin structure
                ptMasterBinList = ptMasterBinList->ptNextBin;
            };
        }
    }

    // Duplicate Wafermap array
    int	iWaferArraysize = WaferMapAllData.SizeX*WaferMapAllData.SizeY;
    delete [] pFile->getWaferMapData().getWafMap();

    pFile->getWaferMapData().setWaferMap(CWafMapArray::allocate(iWaferArraysize, WaferMapAllData.getWafMap()));
    pFile->getWaferMapData().allocCellTestCounter(iWaferArraysize, WaferMapAllData.getCellTestCounter());

    /* HTH - case 4156 - Catch memory allocation exception
    pFile->getWaferMapData().getWafMap() = new CWafMapArray[iWaferArraysize];			// Pointer to wafermap BIN results array
    memcpy(pFile->getWaferMapData().getWafMap(),WaferMapAllData.getWafMap(),iWaferArraysize*sizeof(CWafMapArray));
    */

    pFile->getWaferMapData().iLowDieX = WaferMapAllData.iLowDieX;
    pFile->getWaferMapData().iHighDieX = WaferMapAllData.iHighDieX;
    pFile->getWaferMapData().iLowDieY = WaferMapAllData.iLowDieY;
    pFile->getWaferMapData().iHighDieY = WaferMapAllData.iHighDieY;
    pFile->getWaferMapData().SizeX = WaferMapAllData.SizeX;
    pFile->getWaferMapData().SizeY = WaferMapAllData.SizeY;		// Wafermap size (cells in X, and Y)
    pFile->getWaferMapData().iTotalPhysicalDies = WaferMapAllData.iTotalPhysicalDies;	// Total number of physical dies tested on the wafer (dosn't counts retests)

    // Update total parts tested
    if(lPatInfo->GetRecipeOptions().iReport_WafermapType)
        pGroup->cMergedData.lTotalHardBins = pGroup1->cMergedData.lTotalHardBins;
    else
        pGroup->cMergedData.lTotalSoftBins = pGroup1->cMergedData.lTotalSoftBins;
}

/////////////////////////////////////////////////////////////////////////////
// Writes the STACKED wafermap for all groups (one file per group): PAT-Man only, or comparing sites within same file!
/////////////////////////////////////////////////////////////////////////////
bool	CGexReport::WriteStackedGroupsWaferMap(int iTotalWafers,BOOL bWriteReport)
{
    CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();
    if((lPatInfo == NULL) || (ReportOptions.getAdvancedReport() != GEX_ADV_OUTLIER_REMOVAL))
        return false;

    // PAT-Man enabled?
    if(!GS::LPPlugin::ProductInfo::getInstance()->isPATMan() &&
       !GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT())
        return false;

    // Report must not be created yet...then quietly return
    if(bWriteReport == false)
        return false;

    // We need multiple groups
    if(getGroupsList().count() <= 1)
        return false;

    bool	bDisplayStackedWafer;
    if(iTotalWafers)
    {
        // Some outliers exist, so wafermap already shows...
        bDisplayStackedWafer = (!m_pReportOptions->GetOption("wafer", "chart_show").toString().isEmpty()) ? true : false;
    }
    else
    {
        // No wafer reported so far, so ensure we display one, even if no outlier detected!
        bDisplayStackedWafer = true;
    }


    // If no STACKED wafermap...quietly return
    int					iTotalStackedGroups			= 0;
    CGexGroupOfFiles *	pGroup						= NULL;
    CGexFileInGroup *	pFile						= NULL;
    bool				bMultipleDataFilesCompared	= false;
    QString				strDataFile;

    // Iterator on Groups list
    QListIterator<CGexGroupOfFiles*> itGroupsList(getGroupsList());

    while(itGroupsList.hasNext())
    {
        pGroup	= itGroupsList.next();
        pFile	= (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

        if (strDataFile.isEmpty())
            strDataFile = pFile->strFileName;

        // Check i group holds a wafermap.
        if((pGroup->cStackedWaferMapData.iTotalWafermaps == 1) && (pGroup->cStackedWaferMapData.cWafMap != NULL))
            iTotalStackedGroups++;

        // Flag if one of the dataset is not the same file.
        if(strDataFile != pFile->strFileName || pGroup->pFilesList.count() > 1)
            bMultipleDataFilesCompared = true;
    }

    // IF multiple datasets compared (and not same file), exit!
    if(bMultipleDataFilesCompared == true)
        return false;

    // Check if at least one stacked wafermap exists
    if(iTotalStackedGroups == 0)
        return false;

    // Combine stacked wafer from each group...do it over group#1
    StackedGroupsWaferMap();

    // Display of stacked wafer enabled?
    if(bDisplayStackedWafer == false)
        return false;

    // Ensure to force wafermap visible
    QString strOriginalWafermapMode = m_pReportOptions->GetOption("wafer", "chart_show").toString();
    m_pReportOptions->SetOption("wafer", "chart_show", "all_individual");

    // Display bin count in wafer
    itGroupsList.toFront();
    CGexGroupOfFiles *	pGroup1 = itGroupsList.next();
    pFile = pGroup1->pFilesList.first();
    WriteBinningLegend(pGroup1,bWriteReport);

    // Display resulting wafermap: disable reloading wafermap from array (as we've just merged all sites in group 0!)
    WriteIndividualWafer(pGroup1,pFile,bWriteReport,1,1,"",false);

    // Restore golbal option
    m_pReportOptions->SetOption("wafer", "chart_show", strOriginalWafermapMode);

    // Success
    return true;
}
