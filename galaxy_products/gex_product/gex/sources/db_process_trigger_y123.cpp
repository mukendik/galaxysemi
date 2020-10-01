#include "db_engine.h"
#include "db_transactions.h"
#include "gex_database_entry.h"
#include "report_options.h"
#include "mo_y123.h"
#include "datapump/datapump_taskdata.h"
#include "mo_task.h"
#include "product_info.h"

extern CReportOptions	ReportOptions;		// Holds options (report_build.h)
// Interaction with Y123-Web loop
CGexMoY123		clGexMoY123;

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
int GS::Gex::DatabaseEngine::ProcessTrigger_Y123(
        CGexMoTaskItem *ptTask,
        GexDatabaseEntry *pDatabaseEntry,
        QString &strTriggerFileName,
        QTextStream &hTriggerFile,
        bool *pbDeleteTrigger,
        QString &strErrorMessage,
        QString &strShortErrorMsg)
{
    // Check if Y123 mode activated
//    if(!GS::LPPlugin::ProductInfo::getInstance()->isY123WebMode())
//        return Passed;

    // Read trigger file
    QString		strString, strKeyword, strParameter;
    QString		strTrackingKey, strLocalErrorMessage;
    QStringList	strlDataFiles;

    strString = hTriggerFile.readLine();
    while(hTriggerFile.atEnd() == false)
    {
        // Extract parameters
        strKeyword   = strString.section('=',0,0).trimmed();
        strParameter = strString.section('=',1).trimmed();

        // Ignore comment lines
        if(strString.isEmpty() || strKeyword.startsWith("#", Qt::CaseInsensitive))
            goto next_trigger_line;

        // Tracking Key.
        if(strKeyword.toLower().startsWith("trackingkey", Qt::CaseInsensitive))
        {
            strTrackingKey = strParameter;
            goto next_trigger_line;
        }

        // Data file.
        if(strKeyword.toLower().startsWith("datafile", Qt::CaseInsensitive))
        {
            strlDataFiles.append(strParameter);
            goto next_trigger_line;
        }

next_trigger_line:
        // Read next line
        strString = hTriggerFile.readLine();
    }

    // Check if we could extract required parameters
    if(strTrackingKey.isEmpty() || strlDataFiles.isEmpty())
    {
        strShortErrorMsg = "Invalid Y123 trigger file";
        strLocalErrorMessage = strShortErrorMsg + ": ";
        strLocalErrorMessage += strTriggerFileName;
        clGexMoY123.InitError(strLocalErrorMessage);
        m_strInsertionShortErrorMessage = strShortErrorMsg;
        UpdateLogError(strErrorMessage,strLocalErrorMessage);
        return Failed;
    }

    // Notify Y123-Web that Yield-Man insertion step started
    switch(clGexMoY123.Start(strTrackingKey,strlDataFiles))
    {
        case CGexMoY123::eWait:
            *pbDeleteTrigger = false;
            strShortErrorMsg = "Waiting for connection to Y123-Client";
            strLocalErrorMessage = strShortErrorMsg + " [";
            strLocalErrorMessage += clGexMoY123.GetSocketStatus();
            strLocalErrorMessage +="]";
            m_strInsertionShortErrorMessage = strShortErrorMsg;
            UpdateLogError(strErrorMessage,strLocalErrorMessage);
            return Delay;

        case CGexMoY123::eError:
            strShortErrorMsg = "No connection to Y123-Client";
            strLocalErrorMessage = GGET_LASTERRORMSG(CGexMoY123, &clGexMoY123);
            m_strInsertionShortErrorMessage = strShortErrorMsg;
            UpdateLogError(strErrorMessage,strLocalErrorMessage);
            return Failed;
    }

    // Insert file
    bool							bStatus, bEditHeaderMode = false;
    QStringList						strlCorruptedFiles;
    GexDatabaseInsertedFilesList	listInsertedFiles;

    // Get logical database name (skip the [Local]/[Server] info)
    QString	 strDatabaseLogicalName = ptTask->m_strDatabaseName;
    if(strDatabaseLogicalName.startsWith("[Local]") || strDatabaseLogicalName.startsWith("[Server]"))
        strDatabaseLogicalName = strDatabaseLogicalName.section("]",1).trimmed();

    QStringList		strlWarnings;
    // Import files into GEXDB

    // Have to reset Plugin Error Message (no reset after the last insertion)
    pDatabaseEntry->m_pExternalDatabase->GetPluginID()->m_pPlugin->m_clLastErrorGexDbPlugin_Base.Reset();

    // Update flags
    // Check all options but fail only if "Already inserted"
    pDatabaseEntry->m_pExternalDatabase->SetInsertionValidationOptionFlag(GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_ALL);// &~GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_FIELDPARTLOCATION);
    pDatabaseEntry->m_pExternalDatabase->SetInsertionValidationFailOnFlag(GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_ALREADYINSERTED);

    // Try to import file
    bStatus = ImportFiles(strDatabaseLogicalName, strlDataFiles, &strlCorruptedFiles, listInsertedFiles,
                          strErrorMessage,bEditHeaderMode);
    // Get all errors and warning messages
    if(!bStatus)
    {
        if(GexDbPlugin_Base::eValidation_AlreadyInserted == GGET_LASTERRORCODE(GexDbPlugin_Base,pDatabaseEntry->m_pExternalDatabase->GetPluginID()->m_pPlugin))
        {
            // LOT_ID already inserted with the same TrackingKey
            // No error
            // Notify Y123-Web that Yield-Man insertion step finished successfully
            clGexMoY123.Stop(CGexMoY123::eSuccess);
            m_strInsertionShortErrorMessage = strShortErrorMsg = strErrorMessage = "";

            return Passed;
        }
        // Update the server with the good Error Message
        // If error from insertion => get the plugin error message
        strString = GGET_LASTERRORMSG(GexDbPlugin_Base,pDatabaseEntry->m_pExternalDatabase->GetPluginID()->m_pPlugin);

        // If error from convert => strInsertionShortErrorMessage
        if(strString.isEmpty())
            strString = m_strInsertionShortErrorMessage;

        // Only retreive the first level error (before \n)
        strShortErrorMsg = strString.section("\n",0,0);
        strLocalErrorMessage = strString;
        m_strInsertionShortErrorMessage = strShortErrorMsg;

        //UpdateLogError(strErrorMessage,strShortErrorMsg);

        // Notify Y123-Web that Yield-Man insertion step finished unsuccessfully
        clGexMoY123.Stop(CGexMoY123::eUserError, strShortErrorMsg);
        return Failed;
    }

    // Notify Y123-Web that Yield-Man insertion step finished successfully
    pDatabaseEntry->m_pExternalDatabase->GetWarnings(strlWarnings);
    clGexMoY123.Stop(CGexMoY123::eSuccess, "",strlWarnings);

    return Passed;
}

