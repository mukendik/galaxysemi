#include "browser_dialog.h"
#include "db_engine.h"
#include "db_transactions.h"
#include "scheduler_engine.h"
#include "outlierremoval/outlierremoval_taskdata.h"
#include "mo_task.h"
#include "trigger_pat_task.h"
#include "gexmo_constants.h"
#include "report_options.h"
#include "patman_lib.h"
#include <gqtl_log.h>
#include "product_info.h"
#include "engine.h"
#include "message.h"
//#include "pat_info.h"
#include "pat_engine.h"
#include "engine.h"

extern CGexReport *     gexReport;			// Handle to report class
extern CReportOptions			ReportOptions;		// Holds options (report_build.h)

bool GS::Gex::DatabaseEngine::IsTriggerFile(QString strTriggerFileName)
{
    // Build path to the 'Tasks' list.
    QFile file(strTriggerFileName); // Read the text from a file
    if (file.open(QIODevice::ReadOnly) == false)
        return false;	// Failed opening Trigger file.

    // Read Trigger File
    QString strString;
    QTextStream hTriggerFile(&file);

    // Check if valid header...or empty!
    do
    {
        strString = hTriggerFile.readLine();
        if (strString.startsWith("<?xml"))
        {
            file.close();
            return true;
        }
        if (strString.startsWith("<GalaxyTrigger>"))
        {
            file.close();
            return true;
        }
    }
    while(hTriggerFile.atEnd() == false);

    return false;
}

int	GS::Gex::DatabaseEngine::ProcessTriggerFile(
        CGexMoTaskTriggerPatPump *ptTask,
        QString strTriggerFileName,
        bool *pbDeleteTrigger,
        QString &strErrorMessage,
        QString &strShortErrorMsg)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Process Trigger File: %1...")
          .arg(strTriggerFileName)
          .toLatin1().constData());

    // Build path to the 'Tasks' list.
    QFile file(strTriggerFileName); // Read the text from a file
    if (file.open(QIODevice::ReadOnly) == false)
    {
        m_strInsertionShortErrorMessage = "Failed to open trigger file";
        strErrorMessage = strShortErrorMsg = m_strInsertionShortErrorMessage;
        return Failed;	// Failed opening Trigger file.
    }

    // Read Trigger File
    int iStatus = Failed;
    QString strString;
    QTextStream hTriggerFile(&file);

    // Check if valid header...or empty!
    do
    {
        strString = hTriggerFile.readLine();
        if (strString.startsWith("<?xml"))
        {
            file.close();
            iStatus = ProcessTriggerFile_XML(ptTask, strTriggerFileName,
              pbDeleteTrigger, strErrorMessage, strShortErrorMsg);
            goto stop_process;
        }
        if (strString.startsWith("<GalaxyTrigger>"))
            break;	// Found a vallid trigger file header.
    }
    while(hTriggerFile.atEnd() == false);

    // If end of file reached, then this is not a valid trigger file!
    if(hTriggerFile.atEnd())
    {
        GSLOG(SYSLOG_SEV_WARNING, "End of PAT trigger reached without close tag");
        m_strInsertionShortErrorMessage = "Not a valid trigger file";
        strErrorMessage = strShortErrorMsg = m_strInsertionShortErrorMessage;
        return Failed;
    }

    do
    {
        // Read one line from file
        strString = hTriggerFile.readLine();

        // Remove all spaces
        strString = strString.replace(" ","");
        strString = strString.replace("\t",""); // WT : oct 2012

        // Ignore comment lines
        if(strString.isEmpty() || strString.startsWith("#", Qt::CaseInsensitive))
            goto next_trigger_line;

        // Trigger type: PAT processing
        if(strString.toLower().startsWith("action=pat", Qt::CaseInsensitive) == true)
        {
#ifndef GCORE15334
            m_strInsertionShortErrorMessage = "Feature not available in this version: action=pat, \
                                                please contact support for more informations.";
            strErrorMessage = strShortErrorMsg = m_strInsertionShortErrorMessage;
            return Failed;

#else

            // Check if PAT monitoring
            if(ptTask && (ptTask->GetTaskType() != GEXMO_TASK_PATPUMP))
            {
                m_strInsertionShortErrorMessage = "Not a valid PAT Trigger task!";
                strErrorMessage = strShortErrorMsg = m_strInsertionShortErrorMessage;
                return Failed;
            }
            QString lRet=ProcessTrigger_PAT(strTriggerFileName, hTriggerFile, (CGexMoTaskPatPump*)ptTask);
            if(lRet.startsWith("ok"))
                iStatus = Passed;
            else
                m_strInsertionShortErrorMessage = lRet;  //"PAT error or no valid PAT license!";
            GSLOG(SYSLOG_SEV_WARNING, QString("ProcessTrigger_PAT:%1")
                  .arg( m_strInsertionShortErrorMessage).toLatin1().constData());
            goto stop_process;
#endif
        }

        // Trigger type: Composite PAT processing
        if(strString.toLower().startsWith("action=composite_pat", Qt::CaseInsensitive) == true)
        {
#ifndef GCORE15334
            m_strInsertionShortErrorMessage = "Feature not available in this version: action=composite_pat, \
                                                please contact support for more informations.";
            strErrorMessage = strShortErrorMsg = m_strInsertionShortErrorMessage;
            return Failed;

#else

            // Check if PAT monitoring
            if(ptTask && (ptTask->GetTaskType() != GEXMO_TASK_PATPUMP))
            {
                m_strInsertionShortErrorMessage = "Not a valid PAT Trigger task!";
                strErrorMessage = strShortErrorMsg = m_strInsertionShortErrorMessage;
                return Failed;
            }
            if(ProcessTrigger_CompositePAT(strTriggerFileName,hTriggerFile))
                iStatus = Passed;
            else
                m_strInsertionShortErrorMessage = "PAT error or no valid PAT license!";
            goto stop_process;
#endif
        }

        // Trigger type: Wafer Export
        if(strString.toLower().startsWith("action=wafer_export", Qt::CaseInsensitive) == true)
        {
 #ifdef GCORE15334

            // Check if YM/PM monitoring
            if(ptTask && (ptTask->GetTaskType() != GEXMO_TASK_TRIGGERPUMP) &&
               (ptTask->GetTaskType() != GEXMO_TASK_PATPUMP))
            {
                m_strInsertionShortErrorMessage = "Not a valid YM/PM Trigger task!";
                strErrorMessage = strShortErrorMsg = m_strInsertionShortErrorMessage;
                return Failed;
            }
#else
            if (ptTask->GetTaskType() == GEXMO_TASK_PATPUMP)
            {
                m_strInsertionShortErrorMessage = "Feature not available in this version: action=wafer_export, \
                        please contact support for more informations.";
                        strErrorMessage = strShortErrorMsg = m_strInsertionShortErrorMessage;
                return Failed;
            }
#endif
            if(ProcessTrigger_WAFER_EXPORT(ptTask, strTriggerFileName,hTriggerFile))
                iStatus = Passed;
            goto stop_process;
        }


        // Trigger type: Y123-Web insertion
        if(strString.toLower().startsWith("action=yieldmaninsert", Qt::CaseInsensitive) == true)
        {
            m_strInsertionShortErrorMessage = "Y123-Web insertion is disabled!";
            strErrorMessage = strShortErrorMsg = m_strInsertionShortErrorMessage;
            return Failed;
            //return ProcessTrigger_Y123(ptTask,pDatabaseEntry,strTriggerFileName,hTriggerFile, pbDeleteTrigger, strErrorMessage, strShortErrorMsg);
        }

        // Read next line in trigger file.
next_trigger_line:;
    }
    while(hTriggerFile.atEnd() == false);

    GSLOG(SYSLOG_SEV_WARNING, "No action found");

stop_process:

    // Update Error Message if any
    if(strErrorMessage.isEmpty())
        strErrorMessage = m_strInsertionShortErrorMessage;
    if(strShortErrorMsg.isEmpty())
        strShortErrorMsg = m_strInsertionShortErrorMessage;

    return iStatus;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
bool    GS::Gex::DatabaseEngine::ProcessTrigger_WAFER_EXPORT(CGexMoTaskTriggerPatPump * ptTask,
                                                             QString &strTriggerFileName,
                                                             QTextStream &hTriggerFile)
{
    QString	strErrorMessage;
    QString	strString;
    QString	strKeyword,strParameter;
    GS::Gex::PATProcessing cFields(this);
    // PAT-17 : do we have to register in the script engine this PATProcessing ?
    bool bStatus = true;

    // Refuse to process this WAFER_EXPORT trigger file if the Examinator package doesn't have the license for it!
    if(!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        return false;

    // Trigger file to process
    cFields.strTriggerFile = strTriggerFileName;

    int	iLineNumber=0;
    do
    {
        // Read one line from file
        strString    = hTriggerFile.readLine();
        strKeyword   = strString.section('=',0,0);
        strKeyword = strKeyword.trimmed();
        strParameter = strString.section('=',1);
        strParameter = strParameter.trimmed();

        // Keep track of line#
        iLineNumber++;

        // Ignore comment lines
        if(strString.isEmpty() || strKeyword.startsWith("#", Qt::CaseInsensitive))
            goto next_trigger_line;

        // Define custom recipe file to use.
        if(strKeyword.startsWith("LogFile", Qt::CaseInsensitive))
        {
            cFields.strLogFilePath= strParameter;
            goto next_trigger_line;
        }

        // Define input files.
        if(strKeyword.startsWith("DataSource", Qt::CaseInsensitive))
        {
            cFields.strSources += strParameter;
            goto next_trigger_line;
        }

        // Define input optional data file
        if(strKeyword.startsWith("OptionalSource", Qt::CaseInsensitive))
        {
            cFields.strOptionalSource = strParameter;
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

        // Post-Processing shell command to call
        if(strKeyword.startsWith("Shell", Qt::CaseInsensitive))
        {
            cFields.strPostProcessShell= strParameter;
            goto next_trigger_line;
        }

        //  Define inkless wafermap notch position
        if(strKeyword.startsWith("Notch", Qt::CaseInsensitive))
        {
            if(strParameter.startsWith("Up", Qt::CaseInsensitive))
                cFields.mOutputMapNotch = 12;	// Notch UP = '12 hours' direction
            else
                if(strParameter.startsWith("Right", Qt::CaseInsensitive))
                    cFields.mOutputMapNotch = 3;	// Notch RIGHT = '3 hours' direction
                else
                    if(strParameter.startsWith("Down", Qt::CaseInsensitive))
                        cFields.mOutputMapNotch = 6;	// Notch DOWN = '6 hours' direction
                    else
                        if(strParameter.startsWith("Left", Qt::CaseInsensitive))
                            cFields.mOutputMapNotch = 9;	// Notch LEFT = '9 hours' direction
                        else
                            cFields.mOutputMapNotch = -1;// Leave Notch to default direction
            goto next_trigger_line;
        }

        //  Define inkless wafermap positive X direction
        if(strKeyword.compare("OutputMapPosX",Qt::CaseInsensitive) == 0)
        {
            if(strParameter.compare("Right", Qt::CaseInsensitive) == 0)
                // Positive X towards the right, except if one specified in STDF
                cFields.mOutputMapXDirection = PATProcessing::ePositiveRight;
            else if(strParameter.compare("Left", Qt::CaseInsensitive) == 0)
                // Positive X towards the left, except if one specified in STDF
                cFields.mOutputMapXDirection = PATProcessing::ePositiveLeft;
            else
            {
                GSLOG(SYSLOG_SEV_ERROR,
                      QString("Invalid value %1 for GTF key %2").arg(strParameter).arg(strKeyword)
                      .toLatin1().constData());

                return false;
            }
            return true;
        }

        //  Define inkless wafermap positive Y direction
        if(strKeyword.compare("OutputMapPoxY",Qt::CaseInsensitive) == 0)
        {
            if(strParameter.compare("Up", Qt::CaseInsensitive) == 0)
                // Positive Y towards the up, except if one specified in STDF
                cFields.mOutputMapYDirection = PATProcessing::ePositiveUp;
            else if(strParameter.compare("Down", Qt::CaseInsensitive) == 0)
                // Positive Y towards the down, except if one specified in STDF
                cFields.mOutputMapYDirection = PATProcessing::ePositiveDown;
            else
            {
                GSLOG(SYSLOG_SEV_ERROR,
                      QString("Invalid value %1 for GTF key %2").arg(strParameter).arg(strKeyword)
                      .toLatin1().constData());

                return false;
            }

            return true;
        }

        //  Define inkless wafermap rotation flag
        if(strKeyword.startsWith("RotateWafer", Qt::CaseInsensitive))
        {
            if(strParameter.startsWith("Disabled", Qt::CaseInsensitive))
                cFields.bRotateWafer = false;
            goto next_trigger_line;
        }

        //  Define inkless wafermap positive X direction
        if(strKeyword.startsWith("PosX", Qt::CaseInsensitive))
        {
            if(strParameter.startsWith("Right", Qt::CaseInsensitive))
                cFields.mStdfXDirection = GS::Gex::PATProcessing::ePositiveRight;			// Positive X towards the right, except if one specified in STDF
            else
                if(strParameter.startsWith("Left", Qt::CaseInsensitive))
                    cFields.mStdfXDirection = GS::Gex::PATProcessing::ePositiveLeft;			// Positive X towards the left, except if one specified in STDF
                else
                    if(strParameter.startsWith("Force_Right", Qt::CaseInsensitive))
                        cFields.mStdfXDirection = GS::Gex::PATProcessing::ePositiveForceRight;	// Positive X towards the right, whatever in STDF
                    else
                        if(strParameter.startsWith("Force_Left", Qt::CaseInsensitive))
                            cFields.mStdfXDirection = GS::Gex::PATProcessing::ePositiveForceLeft;		// Positive X towards the left, whatever in STDF
            goto next_trigger_line;
        }

        //  Define inkless wafermap positive Y direction
        if(strKeyword.startsWith("PosY", Qt::CaseInsensitive))
        {
            if(strParameter.startsWith("Up", Qt::CaseInsensitive))
                cFields.mStdfYDirection = GS::Gex::PATProcessing::ePositiveUp;		// Positive Y towards the up, except if one specified in STDF
            else
                if(strParameter.startsWith("Down", Qt::CaseInsensitive))
                    cFields.mStdfYDirection = GS::Gex::PATProcessing::ePositiveDown;		// Positive Y towards the down, except if one specified in STDF
                else
                    if(strParameter.startsWith("Force_Up", Qt::CaseInsensitive))
                        cFields.mStdfYDirection = GS::Gex::PATProcessing::ePositiveForceUp;	// Positive Y towards the up, whatever in STDF
                    else
                        if(strParameter.startsWith("Force_Down", Qt::CaseInsensitive))
                            cFields.mStdfYDirection = GS::Gex::PATProcessing::ePositiveForceDown;	// Positive Y towards the down, whatever in STDF
            goto next_trigger_line;
        }

        // custom GrossDiePerWafer info.
        if(strKeyword.startsWith("GrossDie", Qt::CaseInsensitive))
        {
            cFields.iGrossDiePerWafer = strParameter.toInt();
            goto next_trigger_line;
        }

        // Gross Die Ratio that should be used to check if a wafer should be accepted
        if(strKeyword.startsWith("ValidWaferThreshold_GrossDieRatio", Qt::CaseInsensitive))
        {
            cFields.fValidWaferThreshold_GrossDieRatio = strParameter.toFloat();
            goto next_trigger_line;
        }

        // Flag specifying if incomplete wafers should be deleted
        if(strKeyword.startsWith("DeleteIncompleteWafers", Qt::CaseInsensitive))
        {
            if(strParameter == "1")
                cFields.bDeleteIncompleteWafers = true;
            goto next_trigger_line;
        }

        // Define output formats
        if(strKeyword.startsWith("Output", Qt::CaseInsensitive))
        {
            // Output format e.g.: 'STDF', 'TSMC' 'G85' ...
            QString strFormat = strParameter.section(' ',0,0);
            // Where to create the file. Remove extra space at start and end,
            // replaced sequence of '\t', '\n', '\v', '\f', '\r' and ' ' with a single space
            QString strPath         = strParameter.section(' ', 1, 1).simplified();
            QString lParameter      = strParameter.section(' ', 2, 2).trimmed();

            if(strFormat.indexOf("STDF", 0, Qt::CaseInsensitive) >= 0)
            {
                cFields.mOutputFolderSTDF     = strPath;	// Path where the STDF file has to be created
                cFields.mOutputDatalogFormat    = GEX_TPAT_DLG_OUTPUT_STDF;
            }
            else if(strFormat.indexOf("ATDF", 0, Qt::CaseInsensitive) >= 0)
            {
                cFields.mOutputFolderSTDF = strPath;	// Path where the ATDF file has to be created
                cFields.mOutputDatalogFormat = GEX_TPAT_DLG_OUTPUT_ATDF;
            }
            else
            {
                QPair<PATProcessing::MapBinType, QString> lMapInfo(PATProcessing::MapDefaultBin, strPath);

                if (lParameter.isEmpty() == false)
                {
                    GSLOG(SYSLOG_SEV_ERROR,
                          QString("Bin type parameter is not supported when exporting wafer (%1)")
                          .arg(lParameter).toLatin1().constData());

                    return false;
                }

                cFields.mOutputFormat.insert(strFormat, lMapInfo);
            }

            goto next_trigger_line;
        }

        // Delete source test data files after successful PAT processing.
        if(strKeyword.startsWith("DeleteDataSource", Qt::CaseInsensitive))
        {
            if(strParameter.startsWith("yes", Qt::CaseInsensitive))
                cFields.bDeleteDataSource = true;
            else
                cFields.bDeleteDataSource = false;
            goto next_trigger_line;
        }

        // Define retest policy: keep latest bin or promote highest bin
        if(strKeyword.startsWith("RetestPolicy", Qt::CaseInsensitive))
        {
            if(strParameter.startsWith("highest_die", Qt::CaseInsensitive))
                cFields.bRetest_HighestBin = true;
            else
                cFields.bRetest_HighestBin = false;
            goto next_trigger_line;
        }

        // Overload MIR product name.
        if(strKeyword.startsWith("Product", Qt::CaseInsensitive))
        {
            cFields.m_strProductName= strParameter;
            goto next_trigger_line;
        }

        // End of trigger file, execute action!
        if(strKeyword.startsWith("</GalaxyTrigger>", Qt::CaseInsensitive))
        {
            // Export wafer...
            GS::Gex::Engine::GetInstance().GetSchedulerEngine().ExecuteWaferExport(ptTask, cFields,strErrorMessage);

            // Check if processing error...and rpeort it.
            if(strErrorMessage.isEmpty() == false)
            {
                GS::Gex::Message::information("", strErrorMessage);
                bStatus = false;
            }
            else
                bStatus = true;

            // Job done!
            goto trigger_processed;
        }

        // Unexpected line. Report error!
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

    CPatInfo* lPatInfo = PATEngine::GetInstance().GetContext();
    if(lPatInfo)
    {
        if(bStatus)
            lPatInfo->CleanupGeneratedFileList(false);
        else
            lPatInfo->CleanupGeneratedFileList(true);
    }

    return bStatus;
}
