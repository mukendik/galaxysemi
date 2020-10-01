///////////////////////////////////////////////////////////
// Examinator Monitoring: Wafermap export task
///////////////////////////////////////////////////////////
#include <qfiledialog.h>
#include <qcheckbox.h>

#include "browser_dialog.h"
#include "report_build.h"
#include "browser_dialog.h"
#include "cbinning.h"
#include "scheduler_engine.h"
#include "status/status_taskdata.h"
#include "mo_task.h"
#include "trigger_pat_task.h"
#include "datapump/datapump_taskdata.h"
#include "gqtl_datakeys.h"
#include "patman_lib.h"
#include "gexmo_constants.h"
#include "db_engine.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "engine.h"
#include "wafer_export_process.h"

// Galaxy QT libraries
#include <gqtl_sysutils.h>

// main.cpp
extern void				WriteDebugMessageFile(const QString & strMessage);
extern CGexReport *		gexReport;				// report_build.cpp: Handle to report class

namespace GS
{
namespace Gex
{
QString SchedulerEngine::ExecuteWaferExport(CGexMoTaskTriggerPatPump* ptTask,
                                            GS::Gex::PATProcessing &cFields,
                                            QString &strErrorMessage)
{
    // Check if multiple input files. If so MERGE them first.
    QString strTestDataFile;

    if(cFields.strSources.count() > 1)
    {
        // Merge files
        if(MergeInputFiles(cFields,strTestDataFile,strErrorMessage) == false)
        {
            // Remove temporary merged STDF file created.
            GS::Gex::Engine::RemoveFileFromDisk(strTestDataFile);

            return strErrorMessage;	// Error occured
        }
    }
    else
        strTestDataFile = cFields.strSources.first();	// Only one STDF input file; no merge!

    // At this stage we no longer have multiple STDF files to process, all files are merged (if needed)
    cFields.strSources.clear();
    cFields.strSources << strTestDataFile;

    // Load the structure with the data file MIR info.
    GS::QtLib::DatakeysContent dbKeysContent(this);
    GS::Gex::Engine::GetInstance().GetDatabaseEngine().ExtractFileKeys(strTestDataFile,dbKeysContent);

    // Export WAFER
    strErrorMessage += ExecuteWaferExportTask(ptTask, dbKeysContent, cFields);

    return strErrorMessage;
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
QString
SchedulerEngine::ExecuteWaferExportTask(CGexMoTaskTriggerPatPump* ptTask,
                                        GS::QtLib::DatakeysContent& /*dbKeysContent*/,
                                        GS::Gex::PATProcessing& cTriggerFields)
{
    QDateTime	cCurrentDateTime = GS::Gex::Engine::GetInstance().GetClientDateTime();
    QString		strErrorMessage;

    // Debug message
    QString strString;
    strString = "---- SchedulerEngine::ExecuteWaferExportTask(";
    if(ptTask == NULL)
        strString += "No task (trigger file)";
    else if (ptTask)
    {
        if (ptTask->GetProperties())
            strString += ptTask->GetProperties()->strTitle;

        onStartTask(ptTask);
    }

    strString += ", ";
    strString += cCurrentDateTime.toString("hh:mm:ss")+"): ";
    WriteDebugMessageFile(strString);

    // Wafermap export Process class
    GS::Gex::WaferExportProcess lWaferExport(this);

    if (lWaferExport.Execute(cTriggerFields) == false)
    {
        strErrorMessage = lWaferExport.GetErrorMessage();

        onStopTask(ptTask, strErrorMessage);
        return strErrorMessage;
    }

    // Successful completion: copy or move input files
    QString strDestFile;
    if(!cTriggerFields.mOutputFolderSTDF.isEmpty())
    {
        QString					strSrcFile;
        QFileInfo				cFileInfo;
        QStringList::Iterator	it;
        QDir                    lDir;

        for(it = cTriggerFields.strSources.begin(); it != cTriggerFields.strSources.end(); ++it)
        {
            // Get source and dest file names
            strSrcFile = *it;
            cFileInfo.setFile(strSrcFile);
            strDestFile = cTriggerFields.mOutputFolderSTDF;
            if(strDestFile.endsWith("/") == false)
                strDestFile += "/";
            strDestFile += cFileInfo.fileName();

            // Copy or move file
            if(cTriggerFields.bDeleteDataSource)
                lDir.rename(strSrcFile, strDestFile);
            else
                CGexSystemUtils::CopyFile(strSrcFile, strDestFile);
        }
    }

    CreateWaferExportLogFile(cTriggerFields, strDestFile, strErrorMessage);

    onStopTask(ptTask, strErrorMessage);
    return strErrorMessage;
}

///////////////////////////////////////////////////////////
// Create WAFER_EXPORT Log file (if enabled)
///////////////////////////////////////////////////////////
void SchedulerEngine::CreateWaferExportLogFile(GS::Gex::PATProcessing& cFields,
                                              QString& strDestFile,
                                              QString& strErrorMessage)
{
    // Check if log file disabled
    if(cFields.strLogFilePath.isEmpty())
        return;

    // Check where to create the log file.
    QFileInfo cFileInfo(cFields.strTriggerFile);
    if(cFields.strLogFilePath == ".")
    {
        // Extract path from trigger file processed
        cFields.m_strLogFileName = cFileInfo.absolutePath();
    }
    else
        cFields.m_strLogFileName = cFields.strLogFilePath;

    // Add original trigger name + '.log' extension
    if(cFields.m_strLogFileName.endsWith("/") == false)
        cFields.m_strLogFileName += "/";
    cFields.m_strLogFileName += cFileInfo.completeBaseName();
    cFields.m_strLogFileName += ".log";

    cFields.m_strTemporaryLog = cFields.m_strLogFileName + ".tmp";

    // Open Log file
    cFields.m_LogFile.setFileName(cFields.m_strTemporaryLog);
    if(!cFields.m_LogFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return;	// Failed creating log file

    // Get curret date & time
    QDateTime dt = GS::Gex::Engine::GetInstance().GetClientDateTime();

    // Assign file I/O stream
    cFields.m_hLogFile.setDevice(&cFields.m_LogFile);

    // Write log file
    cFields.m_hLogFile << "# PAT-Man log file" << endl;
    cFields.m_hLogFile << "#" << endl;
    cFields.m_hLogFile << "# Global info" << endl;
    cFields.m_hLogFile << "Date," << dt.toString(Qt::ISODate) << endl;
    cFields.m_hLogFile << "PatmanRev,"
        // PAT-Man version used
      << GS::Gex::Engine::GetInstance().Get("AppFullName").toString() << endl;
    cFields.m_hLogFile << "Action,WAFER_EXPORT" << endl;							// WAFER_EXPORT log file
    cFields.m_hLogFile << "ErrorMessage," << strErrorMessage << endl;				// WAFER_EXPORT Error message (if any)

    // If file processed, report info
    CGexGroupOfFiles	*pGroup=NULL;
    CGexFileInGroup		*pFile=NULL;

    if(gexReport->getGroupsList().count())
    {
        // Get pointer to first group & first file (we always have them exist)
        pGroup = gexReport->getGroupsList().isEmpty()?NULL:gexReport->getGroupsList().first();
        pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

        // List Lot details
        if(!strDestFile.isEmpty())
            cFields.m_hLogFile << "DataOutput," << strDestFile << endl;						// STDF file processed.
        else
            cFields.m_hLogFile << "DataOutput," << pFile->strFileName << endl;				// STDF file processed.
        if(QFile::exists(cFields.strOptionalSource))
            cFields.m_hLogFile << "OptionalOutput," << cFields.strOptionalOutput<< endl;	// Map file
        cFields.m_hLogFile << "SetupTime," << ((pFile->getMirDatas().lSetupT > 0) ? TimeStringUTC(pFile->getMirDatas().lSetupT): "n/a.\n");
        cFields.m_hLogFile << "StartTime," << ((pFile->getMirDatas().lStartT > 0) ? TimeStringUTC(pFile->getMirDatas().lStartT): "n/a.\n");
        cFields.m_hLogFile << "EndTime," << ((pFile->getMirDatas().lEndT > 0) ? TimeStringUTC(pFile->getMirDatas().lEndT): "n/a.\n");
        if(!cFields.m_strProductName.isEmpty())
            cFields.m_hLogFile << "Product," << cFields.m_strProductName << endl;
        else
            cFields.m_hLogFile << "Product," << pFile->getMirDatas().szPartType << endl;
        cFields.m_hLogFile << "DesignRev," << pFile->getMirDatas().szDesignRev << endl;
        cFields.m_hLogFile << "Program," << pFile->getMirDatas().szJobName << endl;
        cFields.m_hLogFile << "ProgramRev," << pFile->getMirDatas().szJobRev << endl;
        cFields.m_hLogFile << "Lot," << pFile->getMirDatas().szLot << endl;
        cFields.m_hLogFile << "SubLot," << pFile->getMirDatas().szSubLot << endl;
        cFields.m_hLogFile << "WaferID," << pFile->getWaferMapData().szWaferID << endl;
    }

    // If file processed
    if(pFile!=NULL)
    {
        long		lTotalGood=0;
        CBinning	*ptBinCell;

        cFields.m_hLogFile << "#" << endl;
        cFields.m_hLogFile << "# STDF Wafer details" << endl;
        cFields.m_hLogFile << "Stdf_Rows," << pFile->getWaferMapData().SizeX << endl;
        cFields.m_hLogFile << "Stdf_Columns,"<< pFile->getWaferMapData().SizeY << endl;
        cFields.m_hLogFile << "Stdf_TotalDies," << pGroup->cMergedData.lTotalSoftBins << endl;
        if(cFields.iGrossDiePerWafer > 0)
            cFields.m_hLogFile << "Stdf_GrossDiesPerWafer," << cFields.iGrossDiePerWafer << endl;

        if(cFields.strWaferFileFullName.isEmpty() == false)
            cFields.m_hLogFile << "WafMapFile," << cFields.strWaferFileFullName << endl;	// Wafermap file created (if any)

        cFields.m_hLogFile << "#" << endl;
        cFields.m_hLogFile << "# Binning info" << endl;
        // Display Bin classes
        cFields.m_hLogFile << "BinName,";
        ptBinCell=pGroup->cMergedData.ptMergedSoftBinList;
        while(ptBinCell != NULL)
        {
            // Display Bin name
            cFields.m_hLogFile << ptBinCell->strBinName << ",";

            // Move to next Bin cell
            ptBinCell = ptBinCell->ptNextBin;
        };
        cFields.m_hLogFile << endl;

        // Display Bin classes
        cFields.m_hLogFile << "Bin#,";
        ptBinCell=pGroup->cMergedData.ptMergedSoftBinList;
        while(ptBinCell != NULL)
        {
            // Display Bin ID
            cFields.m_hLogFile << ptBinCell->iBinValue << ",";

            // Move to next Bin cell
            ptBinCell = ptBinCell->ptNextBin;
        };
        cFields.m_hLogFile << endl;

        // Display Bin count
        cFields.m_hLogFile << "BinCount,";
        ptBinCell=pGroup->cMergedData.ptMergedSoftBinList;
        while(ptBinCell != NULL)
        {
            // Display Bin count
            cFields.m_hLogFile << ptBinCell->ldTotalCount << ",";

            // Keep track of total Good dies & total fail dies.
            if((ptBinCell->cPassFail == 'P') || (ptBinCell->cPassFail == 'p'))
                lTotalGood += ptBinCell->ldTotalCount;

            // Move to next Bin cell
            ptBinCell = ptBinCell->ptNextBin;
        };
        cFields.m_hLogFile << endl;

        // Include few yield related info
        cFields.m_hLogFile << "#" << endl;
        cFields.m_hLogFile << "# Yield details" << endl;
        cFields.m_hLogFile << "GoodParts," << lTotalGood << endl;

    }// If file processed


    // Add contents of trigger file for traceability purpose
    QFile cTriggerFile(cFields.strTriggerFile);
    if(cTriggerFile.open(QIODevice::ReadOnly))
    {
        // Assign file I/O stream
        QTextStream hTriggerFile(&cTriggerFile);
        cFields.m_hLogFile << "#" << endl;
        cFields.m_hLogFile << "#" << endl;
        cFields.m_hLogFile << "# Trigger file processed: " << cFields.strTriggerFile << endl;
        cFields.m_hLogFile << "#" << endl;
        do
            cFields.m_hLogFile << "# " << hTriggerFile.readLine() << endl;
        while(hTriggerFile.atEnd() == false);
        hTriggerFile.setDevice(0);
        cTriggerFile.close();
    }

    // Close file
    cFields.m_hLogFile.setDevice(0);
    cFields.m_LogFile.close();

    // Rename log file to final name
    QDir cDir;
    cDir.remove(cFields.m_strLogFileName);
    cDir.rename(cFields.m_strTemporaryLog,cFields.m_strLogFileName);
}

}
}
