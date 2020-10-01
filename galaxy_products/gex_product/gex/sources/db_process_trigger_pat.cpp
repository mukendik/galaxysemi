#ifdef GCORE15334


#include "browser_dialog.h"
#include "db_engine.h"
#include "db_transactions.h"
#include "scheduler_engine.h"
#include "outlierremoval/outlierremoval_taskdata.h"
#include "report_options.h"
#include "patman_lib.h"
#include "product_info.h"
#include <gqtl_log.h>
#include "engine.h"
#include "message.h"
#include "pat_process_ws.h"
//#include "pat_info.h"
#include "pat_engine.h"
#include "pat/pat_task.h"


extern CReportOptions	ReportOptions;		// Holds options (report_build.h)
extern CGexReport *     gexReport;			// Handle to report class

QString GS::Gex::DatabaseEngine::ProcessTrigger_PAT(
        QString &strTriggerFileName,
        QTextStream &hTriggerFile,
        CGexMoTaskPatPump *lPATPump)
{
    QString strErrorMessage;
    QString	strString;
    QString	strKeyword, strParameter;
    GS::Gex::PATProcessing cFields(this);
    bool bStatus = true;

    // Refuse to process this PAT trigger file if the Examinator package doesn't have the license for it!
    if(!GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
        return "error: product not PAT-Man";

    // Trigger file to process
    cFields.strTriggerFile = strTriggerFileName;

    int	iLineNumber=0;
    do
    {
        // Read one line from file
        strString    = hTriggerFile.readLine().trimmed();
        strKeyword   = strString.section('=',0,0);
        strKeyword = strKeyword.trimmed();
        strParameter = strString.section('=',1);
        strParameter = strParameter.trimmed();

        // Keep track of line#
        iLineNumber++;

        // Ignore comment lines
        if(strString.isEmpty() || strKeyword.startsWith("#", Qt::CaseInsensitive))
            goto next_trigger_line;

        // Define input files.
        if(strKeyword.compare("DataSource", Qt::CaseInsensitive) == 0)
        {
            cFields.Set("DataSource", strParameter);
            goto next_trigger_line;
        }

        // Define input optional data file
        if(strKeyword.compare("OptionalSource",Qt::CaseInsensitive) == 0)
        {
            cFields.Set("OptionalSource", strParameter);
            goto next_trigger_line;
        }

        // Customer name field to write in Wafermap file (Semi85)
        if(strKeyword.compare("CustomerName",Qt::CaseInsensitive) == 0)
        {
            cFields.Set("CustomerName", strParameter);
            goto next_trigger_line;
        }

        // Supplier name field to write in Wafermap file (Semi85)
        if(strKeyword.compare("SupplierName",Qt::CaseInsensitive) == 0)
        {
            cFields.Set("SupplierName", strParameter);
            goto next_trigger_line;
        }

        // Wafermap output is SoftBin or HardBin?
        if(strKeyword.compare("OutputMap",Qt::CaseInsensitive) == 0 ||
           strKeyword.compare("OutputMapBinType",Qt::CaseInsensitive) == 0)
        {
            cFields.Set("OutputMapBinType", strParameter);
            goto next_trigger_line;
        }

        // Log file: include DPAT test details?
        if(strKeyword.compare("LogDPAT_Details",Qt::CaseInsensitive) == 0)
        {
            cFields.Set("LogDPAT_Details", strParameter);
            goto next_trigger_line;
        }

        // Log file: include NNR test details?
        if(strKeyword.compare("LogNNR_Details",Qt::CaseInsensitive) == 0)
        {
            cFields.Set("LogNNR_Details", strParameter);
            goto next_trigger_line;
        }

        // Log file: include IDDQ-Delta test details?
        if(strKeyword.compare("LogIDDQ_Delta_Details",Qt::CaseInsensitive) == 0)
        {
            cFields.Set("LogIDDQ_Delta_Details", strParameter);
            goto next_trigger_line;
        }

        // Include Original Test limits in STDF file?
        if(strKeyword.compare("ReportStdfOriginalLimits",Qt::CaseInsensitive) == 0)
        {
            cFields.Set("ReportStdfOriginalLimits", strParameter);
            goto next_trigger_line;
        }

        // Embed PAT-Man HTML reportinto STDF file?
        if(strKeyword.compare("EmbedStdfReport",Qt::CaseInsensitive) == 0)
        {
            cFields.Set("EmbedStdfReport", strParameter);
            goto next_trigger_line;
        }

        // Specific bin to overload Good Map bin if STDF die is Missing
        if(strKeyword.compare("GoodMapOverload_MissingStdf",Qt::CaseInsensitive) == 0)
        {
            cFields.Set("GoodMapOverload_MissingStdf", strParameter);
            goto next_trigger_line;
        }

        // Specific bin to overload Good Map bin if STDF die is Failing
        if(strKeyword.compare("GoodMapOverload_FailStdf",Qt::CaseInsensitive) == 0)
        {
            cFields.Set("GoodMapOverload_FailStdf", strParameter);
            goto next_trigger_line;
        }

        // Overload STDF TimeStamp?
        if(strKeyword.compare("UpdateTimeFields",Qt::CaseInsensitive) == 0)
        {
            cFields.Set("UpdateTimeFields", strParameter);
            goto next_trigger_line;
        }

        // Post-Processing shell command to call
        if(strKeyword.compare("Shell",Qt::CaseInsensitive) == 0)
        {
            cFields.Set("Shell", strParameter);
            goto next_trigger_line;
        }

        //  Define inkless wafermap notch position
        if(strKeyword.compare("Notch",Qt::CaseInsensitive) == 0 ||
           strKeyword.compare("OutputMapNotch",Qt::CaseInsensitive) == 0)
        {
            cFields.Set("OutputMapNotch", strParameter);
            goto next_trigger_line;
        }

        //  Define inkless wafermap rotation flag
        if(strKeyword.compare("RotateWafer",Qt::CaseInsensitive) == 0)
        {
            cFields.Set("RotateWafer", strParameter);
            goto next_trigger_line;
        }

        //  Define inkless wafermap positive X direction
        if(strKeyword.compare("PosX",Qt::CaseInsensitive) == 0 ||
           strKeyword.compare("STDFPosX",Qt::CaseInsensitive) == 0)
        {
            cFields.Set("STDFPosX", strParameter);
            goto next_trigger_line;
        }

        //  Define inkless wafermap positive Y direction
        if(strKeyword.compare("PosY",Qt::CaseInsensitive) == 0 ||
           strKeyword.compare("STDFPosY",Qt::CaseInsensitive) == 0)
        {
            cFields.Set("STDFPosY", strParameter);
            goto next_trigger_line;
        }

        // custom GrossDiePerWafer info.
        if(strKeyword.compare("GrossDie",Qt::CaseInsensitive) == 0)
        {
            cFields.Set("GrossDie", strParameter);
            goto next_trigger_line;
        }

        // Custom Alarm level defined in trigger file (overloading PAT-Man GUI settings)?
        if(strKeyword.compare("PatAlarmLevel",Qt::CaseInsensitive) == 0)
        {
            cFields.Set("PatAlarmLevel", strParameter);
            goto next_trigger_line;
        }

        // Gross Die Ratio that should be used to check if a wafer should be accepted
        if(strKeyword.compare("ValidWaferThreshold_GrossDieRatio",Qt::CaseInsensitive) == 0)
        {
            cFields.Set("ValidWaferThreshold_GrossDieRatio", strParameter);
            goto next_trigger_line;
        }

        // Flag specifying if incomplete wafers should be deleted
        if(strKeyword.compare("DeleteIncompleteWafers",Qt::CaseInsensitive) == 0)
        {
            cFields.Set("DeleteIncompleteWafers", strParameter);
            goto next_trigger_line;
        }

        // Define output formats
        if(strKeyword.compare("Output",Qt::CaseInsensitive) == 0)
        {
            if (cFields.Set(strKeyword, strParameter) == true)
                goto next_trigger_line;
        }

        // Define Report output
        if(strKeyword.compare("ReportMode",Qt::CaseInsensitive) == 0)
        {
            cFields.Set("ReportMode", strParameter);
            goto next_trigger_line;
        }

        // Define Report output
        if(strKeyword.compare("ReportName",Qt::CaseInsensitive) == 0)
        {
            cFields.Set("ReportName", strParameter);
            goto next_trigger_line;
        }

        // Define custom recipe file to use.
        if(strKeyword.compare("Recipe",Qt::CaseInsensitive) == 0)
        {
            cFields.Set("Recipe", strParameter);
            goto next_trigger_line;
        }

        // Points to recipe overload file (typically Production options that engineering do not want to interfer with)
        if(strKeyword.compare("RecipeOverload",Qt::CaseInsensitive) == 0)
        {
            cFields.Set("RecipeOverload", strParameter);
            goto next_trigger_line;
        }

        // Select COMPOSITE file: holds exclusion zones, 3D neighborhood
        if(strKeyword.compare("CompositeFile",Qt::CaseInsensitive) == 0)
        {
            cFields.Set("CompositeFile", strParameter);
            goto next_trigger_line;
        }

        // Define custom recipe file to use.
        if(strKeyword.compare("LogFile",Qt::CaseInsensitive) == 0)
        {
            cFields.Set("LogFile", strParameter);
            goto next_trigger_line;
        }

        // Delete source test data files after successful PAT processing.
        if(strKeyword.compare("DeleteDataSource",Qt::CaseInsensitive) == 0)
        {
            cFields.Set("DeleteDataSource", strParameter);
            goto next_trigger_line;
        }

        // Define retest policy: keep latest bin or promote highest bin
        if(strKeyword.compare("RetestPolicy",Qt::CaseInsensitive) == 0)
        {
            cFields.Set("RetestPolicy", strParameter);
            goto next_trigger_line;
        }

        // Overload MIR product name.
        if(strKeyword.compare("Product",Qt::CaseInsensitive) == 0)
        {
            cFields.m_strProductName= strParameter;
            goto next_trigger_line;
        }
        // STDF Reference die Location
        if(strKeyword.compare("STDFRefLocation", Qt::CaseInsensitive) == 0)
        {
            if (cFields.Set(strKeyword, strParameter) == true)
                goto next_trigger_line;
        }

        // STDF Orientation
        if(strKeyword.compare("STDFNotch", Qt::CaseInsensitive) == 0)
        {
            if (cFields.Set(strKeyword, strParameter) == true)
                goto next_trigger_line;
        }

        // External map Reference die Location
        if(strKeyword.compare("ExternalRefLocation", Qt::CaseInsensitive) == 0)
        {
            if (cFields.Set(strKeyword,strParameter) == true)
                goto next_trigger_line;
        }

        // External map Orientation
        if(strKeyword.compare("ExternalNotch", Qt::CaseInsensitive) == 0)
        {
            if (cFields.Set(strKeyword,strParameter) == true)
                goto next_trigger_line;
        }

        // External map X Direction (positive X)
        if(strKeyword.compare("ExternalPosX", Qt::CaseInsensitive) == 0)
        {
            if (cFields.Set(strKeyword,strParameter) == true)
                goto next_trigger_line;
        }

        // External map Y Direction (positive Y)
        if(strKeyword.compare("ExternalPosY", Qt::CaseInsensitive) == 0)
        {
            if (cFields.Set(strKeyword,strParameter) == true)
                goto next_trigger_line;
        }

        // External map Fail bin overload
        if(strKeyword.compare("ExternalFailOverload", Qt::CaseInsensitive) == 0)
        {
            if (cFields.Set(strKeyword,strParameter) == true)
                goto next_trigger_line;
        }

        // Define reference coordinate system use for alignement
        if(strKeyword.compare("CoordSystemAlignment", Qt::CaseInsensitive) == 0)
        {
            if (cFields.Set(strKeyword, strParameter) == true)
                goto next_trigger_line;
        }

        // Define geometry check to execute after alignment
        if(strKeyword.compare("GeometryCheck", Qt::CaseInsensitive) == 0)
        {
            if (cFields.Set(strKeyword, strParameter) == true)
                goto next_trigger_line;
        }

        // Define if the maps have to be merged
        if(strKeyword.compare("MergeMaps", Qt::CaseInsensitive) == 0)
        {
            if (cFields.Set(strKeyword, strParameter) == true)
                goto next_trigger_line;
        }

        // Define bin to apply on die missing in stdf and good in the external map
        if(strKeyword.compare("MergeBin_GoodMap_MissingSTDF", Qt::CaseInsensitive) == 0)
        {
            if (cFields.Set(strKeyword, strParameter) == true)
                goto next_trigger_line;
        }

        if (strKeyword.compare("AutoAlignBoundaryBins", Qt::CaseInsensitive) == 0)
        {
            if (cFields.Set(strKeyword, strParameter) == true)
                goto next_trigger_line;
        }

        if (strKeyword.compare(GS::PAT::Trigger::sMergeBinRuleJSHook, Qt::CaseInsensitive) == 0)
        {
            if (cFields.Set(strKeyword, strParameter) == true)
                goto next_trigger_line;
        }

        if (strKeyword.compare(GS::PAT::Trigger::sProcessID, Qt::CaseInsensitive) == 0)
        {
            if (cFields.Set(strKeyword, strParameter) == true)
                goto next_trigger_line;
        }

        if (strKeyword.compare(GS::PAT::Trigger::sOnOppositeAxisDir, Qt::CaseInsensitive) == 0)
        {
            if (cFields.Set(strKeyword, strParameter) == true)
                goto next_trigger_line;
        }

        if (strKeyword.compare("ReticleStepInfo", Qt::CaseInsensitive) == 0)
        {
            if (cFields.Set(strKeyword, strParameter) == true)
                goto next_trigger_line;
        }

        // End of trigger file, execute action!
        if(strKeyword.compare("</GalaxyTrigger>",Qt::CaseInsensitive) == 0)
        {
            // Apply PAT to file...
            QString lRes=GS::Gex::Engine::GetInstance().GetSchedulerEngine()
                    .ExecutePatProcessing(cFields,strErrorMessage, lPATPump);
            GSLOG(SYSLOG_SEV_NOTICE, lRes.toLatin1().data());

            // Check if processing error...and report it.
            if(strErrorMessage.isEmpty() == false)
            {
                GS::Gex::Message::information("", strErrorMessage);
                // CASE1: Return ERROR if PatProcessing fail
                // CASE2: No syntax error, Return true but check ErrorMessage
                bStatus = false;
            }
            else
                bStatus = true;

            // PAT Job done! Check next lines (in case batch of PAT blocs)
            goto next_trigger_line;
        }

        // Starting a new PAT trigger section, reset all PAT variables
        if(strKeyword.compare("<GalaxyTrigger>",Qt::CaseInsensitive) == 0)
        {
            // Reset variables
            cFields.clear();
        }

        // Trigger type: Only PAT allowed here!
        if(strKeyword.compare("Action", Qt::CaseInsensitive) == true)
        {
            // Check if not 'PAT'; if so, throw error!
            if(strParameter.compare("PAT", Qt::CaseInsensitive) == true)
                goto next_trigger_line;
            else
                goto invalid_line;	// Invalid line found.
        }

        // Unexpected line. Report error!
invalid_line:
        strErrorMessage = "Invalid line in trigger file (Line# " +QString::number(iLineNumber);
        strErrorMessage += ") : " + strString;
        GS::Gex::Message::information("", strErrorMessage);
        bStatus = false;
        goto trigger_processed;

next_trigger_line:;
    }
    while(hTriggerFile.atEnd() == false);

trigger_processed:
    // Check if Post-Processing shell command line to launch
    if(cFields.strPostProcessShell.isEmpty() == false)
    {
        // Build argument
        QString strCommandLine;

        // Arg1 = status
        if(bStatus)
            strCommandLine = "1 ";
        else
            strCommandLine = "0 ";

        // Arg2 = log file name
        // If log file defined, add it to the argument list
        if(cFields.strLogFilePath.isEmpty() == false)
        {
            // If file path includes space, we have to specify it within quotes
            if(cFields.m_strLogFileName.indexOf(" ") >= 0)
            {
                strCommandLine += "\"";
                strCommandLine += cFields.m_strLogFileName;
                strCommandLine += "\"";
            }
            else
                strCommandLine += cFields.m_strLogFileName;
        }
        else
            strCommandLine += "no_log";

        if(gexReport && gexReport->getGroupsList().count())
        {
            CGexGroupOfFiles *  lGroup  = NULL;
            CGexFileInGroup *   lFile   = NULL;

            // Get pointer to first group & first file (we always have them exist)
            lGroup = (gexReport->getGroupsList().isEmpty())? NULL : gexReport->getGroupsList().first();
            lFile  = (lGroup->pFilesList.isEmpty()) ? NULL : lGroup->pFilesList.first();

            // Arg3 = ProductID
            if(!cFields.m_strProductName.isEmpty())
                strCommandLine += " " + cFields.m_strProductName;
            else if (lFile && *lFile->getMirDatas().szPartType)
                strCommandLine += " " + FormatArgumentScriptString(QString(lFile->getMirDatas().szPartType));
            else
                strCommandLine += " ?";

            // Arg4 = LotID
            if (lFile && *lFile->getMirDatas().szLot)
                strCommandLine += " " + FormatArgumentScriptString(QString(lFile->getMirDatas().szLot));
            else
                strCommandLine += " ?";

            // Arg5 = SublotID
            if (lFile && *lFile->getMirDatas().szSubLot)
                strCommandLine += " " + FormatArgumentScriptString(QString(lFile->getMirDatas().szSubLot));
            else
                strCommandLine += " ?";
        }
        else
        {
            // Arg3 = ProductID
            strCommandLine += " ?";     // Product ID
            // Arg4 = LotID
            strCommandLine += " ?";     // Lot ID
            // Arg5 = SublotID
            strCommandLine += " ?";     // SubLot ID
        }

        // Arg6 = GTF full file name
        if(cFields.strTriggerFile.isEmpty() == false)
        {
            // If file path includes space, we have to specify it within quotes
            if(cFields.strTriggerFile.indexOf(" ") >= 0)
            {
                strCommandLine += " \"";
                strCommandLine += cFields.strTriggerFile;
                strCommandLine += "\"";
            }
            else
                strCommandLine += QString(" %1").arg(cFields.strTriggerFile);
        }
        else
            strCommandLine += " no_gtf";

        // Launch Shell command (minimized)
        #ifdef _WIN32
                // Replace '/' to '\' to avoid MS-DOS compatibility issues
                strCommandLine = strCommandLine.replace('/','\\');

                GSLOG(SYSLOG_SEV_NOTICE, QString("Running: %1 %2").
                      arg(cFields.strPostProcessShell).
                      arg(strCommandLine).toLatin1().data());

                ShellExecuteA(NULL,
                              "open",
                              cFields.strPostProcessShell.toLatin1().constData(),
                              strCommandLine.toLatin1().constData(),
                              NULL,
                              SW_SHOWMINIMIZED);
        #else
                strCommandLine = cFields.strPostProcessShell + " " + strCommandLine;

                GSLOG(SYSLOG_SEV_NOTICE, QString("Running: %1").arg(strCommandLine).toLatin1().data());

                if (system(strCommandLine.toLatin1().constData()) == -1)
                {
                    // On Windows, we dont check if the shell exec failed or not so let s continue anyway
                    //return "error"; ? //return false; ?
                }
        #endif
    }

    if (bStatus)
        return "ok";

    if (strErrorMessage.startsWith("error"))
        return strErrorMessage;

    return "error:"+strErrorMessage;
}

bool    GS::Gex::DatabaseEngine::ProcessTrigger_CompositePAT(QString &strTriggerFileName,QTextStream &hTriggerFile)
{
    QString	strString;
    QString strErrorMessage;
    QString	strKeyword,strParameter;
    bool	bStatus=true;
    CGexCompositePatProcessing	cFields;
    CGexCompositePatWaferInfo	cWafer;

    // Refuse to process this PAT trigger file if the Examinator package doesn't have the license for it!
    if(!GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
        return false;

    // Trigger file to process
    cFields.strTriggerFile = strTriggerFileName;

    int	iLineNumber=0;
    do
    {
        // Read one line from file
        strString       = hTriggerFile.readLine().trimmed();
        strKeyword      = strString.section('=',0,0);
        strKeyword      = strKeyword.trimmed();
        strParameter    = strString.section('=',1);
        strParameter    = strParameter.trimmed();

        // Keep track of line#
        iLineNumber++;

        // Ignore comment lines
        if(strString.isEmpty() || strKeyword.startsWith("#", Qt::CaseInsensitive))
            goto next_trigger_line;

        /*
         * Let's use the Set() method.
         *
        // Define custom recipe file to use.
        if(strKeyword.startsWith("Recipe", Qt::CaseInsensitive))
        {
            cFields.strRecipeFile = strParameter;
            goto next_trigger_line;
        }

        // Define custom recipe file to use.
        if(strKeyword.startsWith("LogFile", Qt::CaseInsensitive))
        {
            cFields.strLogFilePath= strParameter;
            goto next_trigger_line;
        }

        // Define custom output composite file to use.
        if(strKeyword.startsWith("CompositeFile", Qt::CaseInsensitive))
        {
            cFields.strCompositeFile= strParameter;
            goto next_trigger_line;
        }

        // Overload MIR product name.
        if(strKeyword.startsWith("Product", Qt::CaseInsensitive))
        {
            cFields.strProductName= strParameter;
            goto next_trigger_line;
        }

        // Customer name field to write in Wafermap file (Semi85)
        if(strKeyword.startsWith("CustomerName", Qt::CaseInsensitive))
        {
            cFields.strCustomerName = strParameter;
            goto next_trigger_line;
        }

        // Supplier name field to write in Wafermap file (Semi85)
        if(strKeyword.startsWith("SupplierName", Qt::CaseInsensitive))
        {
            cFields.strSupplierName = strParameter;
            goto next_trigger_line;
        }

        // Force to Swap the dies over the X axis
        if(strKeyword.startsWith("StdfSwapDieX", Qt::CaseInsensitive))
        {
            if(strParameter.startsWith("yes", Qt::CaseInsensitive))
                cFields.bSwapDieX = true;
            else
                cFields.bSwapDieX = false;
            goto next_trigger_line;
        }

        // Force to Swap the dies over the Y axis
        if(strKeyword.startsWith(QString("StdfSwapDieY"), Qt::CaseInsensitive))
        {
            if(strParameter.startsWith(QString("yes"),Qt::CaseInsensitive))
                cFields.bSwapDieY = true;
            else
                cFields.bSwapDieY = false;
            goto next_trigger_line;
        }

        // Define retest policy: keep latest bin or promote highest bin
        if(strKeyword.startsWith(QString("RetestPolicy"), Qt::CaseInsensitive))
        {
            if(strParameter.startsWith(QString("highest_die"), Qt::CaseInsensitive))
                cFields.bRetest_HighestBin = true;
            else
                cFields.bRetest_HighestBin = false;
            goto next_trigger_line;
        }

        // Post-Processing shell command to call
        if(strKeyword.startsWith(QString("Shell"), Qt::CaseInsensitive))
        {
            cFields.strPostProcessShell= strParameter;
            goto next_trigger_line;
        }
        */

        // Wafer definition
        if(strKeyword.startsWith(QString("<wafer>"), Qt::CaseInsensitive))
        {
            // Clear structure
            cWafer.clear();

            do
            {
                // Read one line from file
                strString       = hTriggerFile.readLine().trimmed();
                strKeyword      = strString.section('=',0,0);
                strKeyword      = strKeyword.trimmed();
                strParameter    = strString.section('=',1);
                strParameter    = strParameter.trimmed();

                // Ignore comment lines
                if(strString.isEmpty() || strKeyword.startsWith("#", Qt::CaseInsensitive))
                    goto next_wafer_line;

                // Extract Wafer#
                if(strKeyword.startsWith("WaferID", Qt::CaseInsensitive))
                {
                    cWafer.iWaferID = strParameter.toInt();
                    goto next_wafer_line;
                }

                // Define input files.
                if(strKeyword.startsWith("DataSource", Qt::CaseInsensitive))
                {
                    cWafer.strSources += strParameter;
                    goto next_wafer_line;
                }

                // Define input optional data file
                if(strKeyword.startsWith("OptionalSource", Qt::CaseInsensitive))
                {
                    cWafer.strOptionalSource = strParameter;
                    goto next_wafer_line;
                }

                if(strString.startsWith("</wafer>", Qt::CaseInsensitive))
                {
                    cFields.cWaferMaps[cWafer.iWaferID] = cWafer;
                    goto next_trigger_line;
                }

                // Unexpected line. Report error!
                strErrorMessage = "Invalid line in trigger file (Line# " +QString::number(iLineNumber);
                strErrorMessage += ") : " + strString;
                GS::Gex::Message::information("", strErrorMessage);
                bStatus = false;
                goto composite_trigger_processed;

next_wafer_line:;
            }
            while(hTriggerFile.atEnd() == false);
        }

        if (GS::Gex::PATEngine::GetInstance().GetContext() != NULL)
            GS::Gex::PATEngine::GetInstance().DeleteContext();
        GS::Gex::PATEngine::GetInstance().CreateContext();

        // End of trigger file, execute action!
        if(strKeyword.startsWith(QString("</GalaxyTrigger>"), Qt::CaseInsensitive))
        {
            // Apply PAT to file...
            GS::Gex::Engine::GetInstance().GetSchedulerEngine().ExecuteCompositePatProcessing(cFields,strErrorMessage);

            // Check if processing error...and report it.
            if(strErrorMessage.isEmpty() == false)
            {
                GS::Gex::Message::information("", strErrorMessage);
                // CASE1: Return ERROR if PatProcessing fail
                // CASE2: No syntax error, Return true but check ErrorMessage
                bStatus = false;
            }
            else
                bStatus = true;
            // Task completed
            goto composite_trigger_processed;
        }

        // Set is supposed to return true if legal key
        if (!cFields.Set(strKeyword, strParameter))
        {
            // Unexpected line. Report error!
            strErrorMessage = "Invalid line in trigger file (Line# " +QString::number(iLineNumber);
            strErrorMessage += ") : " + strString;
            GS::Gex::Message::information("", strErrorMessage);
            bStatus = false;
            goto composite_trigger_processed;
        }

next_trigger_line:;
    }
    while(hTriggerFile.atEnd() == false);

composite_trigger_processed:

    // Check if Post-Processing shell command line to launch
    if(cFields.strPostProcessShell.isEmpty() == false)
    {
        // Build argument
        QString strCommandLine;

        // Arg1 = status
        if(bStatus)
            strCommandLine = "1 ";
        else
            strCommandLine = "0 ";

        // Arg2 = log file name
        // If log file defined, add it to the argument list
        if(cFields.strLogFilePath.isEmpty() == false)
        {
            // If file path includes space, we have to specify it within quotes
            if(cFields.m_strLogFileName.indexOf(" ") >= 0)
            {
                strCommandLine += "\"";
                strCommandLine += cFields.m_strLogFileName;
                strCommandLine += "\"";
            }
            else
                strCommandLine += cFields.m_strLogFileName;
        }
        else
            strCommandLine += "no_log";

        if(gexReport && gexReport->getGroupsList().count())
        {
            CGexGroupOfFiles *  lGroup  = NULL;
            CGexFileInGroup *   lFile   = NULL;

            // Get pointer to first group & first file (we always have them exist)
            lGroup = (gexReport->getGroupsList().isEmpty())? NULL : gexReport->getGroupsList().first();
            lFile  = (lGroup->pFilesList.isEmpty()) ? NULL : lGroup->pFilesList.first();

            // Arg3 = ProductID
            if(!cFields.strProductName.isEmpty())
                strCommandLine += " " + cFields.strProductName;
            else if (lFile && *lFile->getMirDatas().szPartType)
                strCommandLine += " " + FormatArgumentScriptString(QString(lFile->getMirDatas().szPartType));
            else
                strCommandLine += " ?";

            // Arg4 = LotID
            if (lFile && *lFile->getMirDatas().szLot)
                strCommandLine += " " + FormatArgumentScriptString(QString(lFile->getMirDatas().szLot));
            else
                strCommandLine += " ?";

            // Arg5 = SublotID
            if (lFile && *lFile->getMirDatas().szSubLot)
                strCommandLine += " " + FormatArgumentScriptString(QString(lFile->getMirDatas().szSubLot));
            else
                strCommandLine += " ?";
        }
        else
        {
            // Arg3 = ProductID
            strCommandLine += " ?";     // Product ID
            // Arg4 = LotID
            strCommandLine += " ?";     // Lot ID
            // Arg5 = SublotID
            strCommandLine += " ?";     // SubLot ID
        }

        // Arg6 = GTF full file name
        if(cFields.strTriggerFile.isEmpty() == false)
        {
            // If file path includes space, we have to specify it within quotes
            if(cFields.strTriggerFile.indexOf(" ") >= 0)
            {
                strCommandLine += " \"";
                strCommandLine += cFields.strTriggerFile;
                strCommandLine += "\"";
            }
            else
                strCommandLine += QString(" %1").arg(cFields.strTriggerFile);
        }
        else
            strCommandLine += " no_gtf";

        // Launch Shell command (minimized)
#ifdef _WIN32
        ShellExecuteA(NULL,
                      "open",
                      cFields.strPostProcessShell.toLatin1().constData(),
                      strCommandLine.toLatin1().constData(),
                      NULL,
                      SW_SHOWMINIMIZED);
#else
        strCommandLine = cFields.strPostProcessShell + " " + strCommandLine;
        if (system(strCommandLine.toLatin1().constData()) == -1) {
            return false;
        }
#endif
    }

    return bStatus;
}

#endif
