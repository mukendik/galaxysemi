#include "wafer_export_process_private.h"
#include "gqtl_log.h"
#include "report_options.h"
#include "import_all.h"
#include "engine.h"
#include "csl/csl_engine.h"
#include "temporary_files_manager.h"
#include "gex_constants.h"
#include "gex_report.h"
#include "wafer_export.h"
#include "cbinning.h"
#include "gqtl_sysutils.h"

extern CGexReport *     gexReport;
extern CReportOptions	ReportOptions;

extern QString          ConvertToScriptString(const QString &strFile);

namespace GS
{
namespace Gex
{

WaferExportProcessPrivate::WaferExportProcessPrivate()
{

}

WaferExportProcessPrivate::~WaferExportProcessPrivate()
{

}

bool WaferExportProcessPrivate::LoadSTDFData()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Loading Data from input file(s)...");

    if (mSettings.strSources.isEmpty() || mSettings.strSources.first().isEmpty())
    {
        mErrorMessage = "No input data file defined.";
        return false;
    }

    if (mSettings.strSources.count() > 1)
    {
        mErrorMessage = QString("Too many input data files (%1) defined.").arg(mSettings.strSources.count());
        return false;
    }

    // convert to STDF if needed
    // Build output name and extension
    ConvertToSTDF StdfConvert;
    QString         lInputFile      = mSettings.strSources.first();
    QString         lSTDFFileName   = Engine::GetInstance().Get("TempFolder").toString() + QDir::separator() +
                                      QFileInfo(lInputFile).fileName() + GEX_TEMPORARY_STDF;
    bool            bFileCreated    = false;

    int nConvertStatus = StdfConvert.Convert(false, true, false, true, mSettings.strSources.first(),
                                             lSTDFFileName, "", bFileCreated, mErrorMessage);

    if(nConvertStatus == ConvertToSTDF::eConvertError)
    {
        // Ensure to delete intermediate STDF file created.
        GS::Gex::Engine::GetInstance().GetTempFilesManager().removeAll(TemporaryFile::BasicCheck, true);

        return false;
    }

    // Check if Input file was already in STDF format!
    if(bFileCreated == true)
        lInputFile = lSTDFFileName;

    // Have the STDF file analysed so we can create a summary version!
    FILE *  hFile       = NULL;
    QString lScriptFile = Engine::GetInstance().GetAssistantScript();

    // Create script that will read data file + compute all statistics (but NO report created)
    hFile = fopen(lScriptFile.toLatin1().constData(),"w");

    if(hFile == NULL)
    {
        mErrorMessage = "  > Failed to create script file: " + lScriptFile;
        return false;
    }

    // Creates 'SetOptions' section
    if(!ReportOptions.WriteOptionSectionToFile(hFile))
    {
        mErrorMessage = "Error : can't write option section";
        return false;
    }

    // Creates 'SetProcessData' section
    fprintf(hFile, "SetProcessData()\n");
    fprintf(hFile, "{\n");
    fprintf(hFile, "  var group_id;\n");
    fprintf(hFile, "  gexGroup('reset','all');\n");

    // WCreate group
    // Make sure any '\' in string is doubled.
    QString lSTDFScriptFile = ConvertToScriptString(lInputFile);

    fprintf(hFile, "  group_id = gexGroup('insert','DataSet_1');\n");
    fprintf(hFile,
            "  gexFile(group_id,'insert','%s','All','last_instance',' ','','');\n\n",
            lSTDFScriptFile.toLatin1().constData());

    fprintf(hFile, "}\n\n");

    // Creates 'main' section
    fprintf(hFile, "main()\n");
    fprintf(hFile, "{\n");
    fprintf(hFile, "  SetOptions();\n");
    // Avoids editor to be launched if output options is set to Word or spreadsheet editor!
    fprintf(hFile, "  gexOptions('output','format','html');\n");
    // Merge tests with same test number (even if test number is different)
    fprintf(hFile, "  gexOptions('dataprocessing', 'duplicate_test', 'merge');\n");
    // Always compute Quartiles & Skew & Kurtosis.
    fprintf(hFile, "  gexOptions('speed','adv_stats','always');\n");
    // Force to compute statistics from samples, ignore summary.
    fprintf(hFile, "  gexOptions('statistics','computation','samples_only');\n");
    // Disable outlier removal so to make sure all values are taken into account.
        fprintf(hFile, "  gexOptions('dataprocessing', 'data_cleaning_mode','none');\n");
    // Ensure FULL wafermap is created in any case, non matter the filtering of parts (ensures good processing over Bad Neigbhoors)
    fprintf(hFile, "  gexOptions('wafer','visual_options','all_parts');\n");
    fprintf(hFile, "  gexOptions('binning','computation','wafer_map');\n");
    fprintf(hFile, "  gexReportType('wafer','soft_bin');\n");

    fprintf(hFile, "  SetProcessData();\n");
    // Only data analysis, no report created!
    fprintf(hFile, "  gexOptions('report','build','false');\n");
    // Show 'Database Admin' page after processing file.
    fprintf(hFile, "  gexBuildReport('admin','0');\n");
    fprintf(hFile, "}\n\n");

    fclose(hFile);

    // Execute script.
    if (GS::Gex::CSLEngine::GetInstance().RunScript(lScriptFile).IsFailed())
    {
        mErrorMessage = "  > Failed to analyze bins from stdf file: " + lInputFile;
        return false;
    }

    return true;
}

bool WaferExportProcessPrivate::HasRequiredParts()
{
    // Check if the file has enough data
    CGexGroupOfFiles *  lGroup      = NULL;
    CGexFileInGroup	 *  lFile       = NULL;
    int					lPartsLimit = (int)(mSettings.fValidWaferThreshold_GrossDieRatio * (float) mSettings.iGrossDiePerWafer);

    lGroup = gexReport->getGroupsList().isEmpty() ? NULL : gexReport->getGroupsList().first();

    if(lGroup == NULL)
    {
        mErrorMessage = "No data loaded from files";
        return false;
    }

    lFile = (lGroup->pFilesList.isEmpty()) ? NULL : lGroup->pFilesList.first();

    if(lGroup == NULL)
    {
        mErrorMessage = "No data loaded from files";
        return false;
    }

    if(lFile->getWaferMapData().iTotalPhysicalDies < lPartsLimit)
    {
        if(mSettings.bDeleteIncompleteWafers)
        {
            QDir lDir;
            lDir.remove(mSettings.strSources.first());
        }

        mErrorMessage = "Not enough parts in wafer: ";
        mErrorMessage += QString::number(lFile->getWaferMapData().iTotalPhysicalDies);
        mErrorMessage += " < " + QString::number(lPartsLimit);

        return false;
    }

    return true;
}

bool WaferExportProcessPrivate::GenerateOutput()
{
    // Export Wafermap...
    // Compute output folder
    QString         lWaferFullFileName;
    QString         lOutputWafermapPath;
    PAT::SINFInfo   lSINFInfo;
    int             lFormat;

    QMap<QString, QPair<PATProcessing::MapBinType, QString> >::const_iterator   itBegin;
    QMap<QString, QPair<PATProcessing::MapBinType, QString> >::const_iterator   itEnd;

    itBegin = mSettings.mOutputFormat.constBegin();
    itEnd   = mSettings.mOutputFormat.constEnd();

    // Overload Product name (enables to overload the Product name in the exported Wafermap INF file with the one specified in the trigger file)
    if(mSettings.m_strProductName.isEmpty() == false)
        lSINFInfo.mDeviceName = mSettings.m_strProductName;

    while (itBegin != itEnd )
    {
        if (WaferExport::IsSupportedOutputFormat(itBegin.key(), lFormat))
        {
            if(itBegin.value().second.isEmpty() == false)
            {
                lOutputWafermapPath = itBegin.value().second;	// Custom wafermap output folder.
            }
            else if(mSettings.mOutputFolderSTDF.isEmpty() == false)
            {
                // Use STDF output folder
                QFileInfo lInfo(mSettings.mOutputFolderSTDF);
                lOutputWafermapPath = lInfo.absolutePath(); // Get path to STDF Output folder
            }
            else
            {
                QFileInfo lSTDFFileInfo(mSettings.strSources.first());
                lOutputWafermapPath = lSTDFFileInfo.absolutePath(); // Get path to STDF file created
            }

            WaferExport lWaferExport;

            lWaferExport.SetCustomer(mSettings.strCustomerName);
            lWaferExport.SetSupplier(mSettings.strSupplierName);
            lWaferExport.SetRotateWafer(mSettings.bRotateWafer);
            lWaferExport.SetXAxisDirection(mSettings.mOutputMapXDirection);
            lWaferExport.SetYAxisDirection(mSettings.mOutputMapYDirection);
            lWaferExport.SetOrientation(mSettings.mOutputMapNotch);
            lWaferExport.SetSINFInfo(lSINFInfo);

            // Full file path to create
            QFileInfo lFileInfo(lOutputWafermapPath);
            if(lFileInfo.isDir() == false)
                lWaferFullFileName = lOutputWafermapPath;
            else
                lWaferFullFileName = lOutputWafermapPath + "/" +
                                     lWaferExport.GenerateOutputName((WaferExport::Output) lFormat);

            if (lWaferExport.CreateOutputMap((WaferExport::Output)lFormat, lWaferFullFileName) == false)
            {
                QString lRootCause = QString("Failed to create output map %1: %2")
                                     .arg(itBegin.key()).arg(lWaferExport.GetErrorMessage());

                mErrorMessage = "*Error* " + lRootCause;
                GSLOG(SYSLOG_SEV_WARNING, lRootCause.toLatin1().constData());

                return false;
            }

            mExportedFiles.append(lWaferFullFileName);
        }
        else
        {
            mErrorMessage = "*Error* Unsupported wafer export format " + itBegin.key();
            return false;
        }

        ++itBegin;
    }

    // Successful completion: copy or move input files
    if(mSettings.mOutputFolderSTDF.isEmpty() == false)
    {
        QString                 lDestFile;
        QFileInfo				lFileInfo;
        QStringList::Iterator	it;
        for(it = mSettings.strSources.begin(); it != mSettings.strSources.end(); ++it)
        {
            // Get source and dest file names
            lFileInfo.setFile(*it);
            lDestFile = mSettings.mOutputFolderSTDF;
            if(lDestFile.endsWith("/") == false)
                lDestFile += "/";
            lDestFile += lFileInfo.fileName();

            // Copy or move file
            if(mSettings.bDeleteDataSource)
            {
                QDir lDir;
                lDir.rename(*it, lDestFile);
            }
            else
                CGexSystemUtils::CopyFile(*it, lDestFile);
        }
    }

    return true;
}
/*
void WaferExportProcessPrivate::CreateLogFile(const QString& lDestFile)
{
    // Check if log file disabled
    if(mSettings.strLogFilePath.isEmpty() == false)
    {
        // Check where to create the log file.
        QFileInfo lFileInfo(mSettings.strTriggerFile);
        if(mSettings.strLogFilePath == ".")
        {
            // Extract path from trigger file processed
            mSettings.m_strLogFileName = lFileInfo.absolutePath();
        }
        else
            mSettings.m_strLogFileName = mSettings.strLogFilePath;

        // Add original trigger name + '.log' extension
        if(mSettings.m_strLogFileName.endsWith("/") == false)
            mSettings.m_strLogFileName += "/";

        mSettings.m_strLogFileName += lFileInfo.completeBaseName();
        mSettings.m_strLogFileName += ".log";

        mSettings.m_strTemporaryLog = mSettings.m_strLogFileName + ".tmp";

        // Open Log file
        mSettings.m_LogFile.setFileName(mSettings.m_strTemporaryLog);
        if(!mSettings.m_LogFile.open(QIODevice::WriteOnly | QIODevice::Text))
            return;	// Failed creating log file

        // Get curret date & time
        QDateTime dt = QDateTime::currentDateTime();

        // Assign file I/O stream
        mSettings.m_hLogFile.setDevice(&mSettings.m_LogFile);

        // Write log file
        mSettings.m_hLogFile << "# PAT-Man log file" << endl;
        mSettings.m_hLogFile << "#" << endl;
        mSettings.m_hLogFile << "# Global info" << endl;
        mSettings.m_hLogFile << "Date," << dt.toString("dd MMMM yyyy hh:mm:ss") << endl;	// date: '21 January 2006 21:23:05'
        mSettings.m_hLogFile << "PatmanRev," << GS::Gex::Engine::GetInstance().Get("AppFullName").toString() << endl;
        mSettings.m_hLogFile << "Action,WAFER_EXPORT" << endl;				// WAFER_EXPORT log file
        mSettings.m_hLogFile << "ErrorMessage," << mErrorMessage << endl;	// WAFER_EXPORT Error message (if any)

        // If file processed, report info
        CGexGroupOfFiles *  lGroup  = NULL;
        CGexFileInGroup *   lFile   = NULL;

        // Get pointer to first group & first file (we always have them exist)
        lGroup = gexReport->getGroupsList().isEmpty() ? NULL : gexReport->getGroupsList().first();
        lFile  = (lGroup != NULL && lGroup->pFilesList.isEmpty()) ? NULL : lGroup->pFilesList.first();

        // If file processed
        if(lFile != NULL)
        {
            long		lTotalGood=0;
            CBinning	*ptBinCell;

            // List Lot details
            if(!lDestFile.isEmpty())
                mSettings.m_hLogFile << "DataOutput," << lDestFile << endl;						// STDF file processed.
            else
                mSettings.m_hLogFile << "DataOutput," << lFile->strFileName << endl;				// STDF file processed.

            if(QFile::exists(mSettings.strOptionalSource))
                mSettings.m_hLogFile << "OptionalOutput," << mSettings.strOptionalOutput<< endl;	// Map file
            mSettings.m_hLogFile << "SetupTime," << ((lFile->MirData.lSetupT > 0) ? TimeStringUTC(lFile->MirData.lSetupT): "n/a.\n");
            mSettings.m_hLogFile << "StartTime," << ((lFile->MirData.lStartT > 0) ? TimeStringUTC(lFile->MirData.lStartT): "n/a.\n");
            mSettings.m_hLogFile << "EndTime," << ((lFile->MirData.lEndT > 0) ? TimeStringUTC(lFile->MirData.lEndT): "n/a.\n");
            if(!mSettings.m_strProductName.isEmpty())
                mSettings.m_hLogFile << "Product," << mSettings.m_strProductName << endl;
            else
                mSettings.m_hLogFile << "Product," << lFile->MirData.szPartType << endl;
            mSettings.m_hLogFile << "DesignRev," << lFile->MirData.szDesignRev << endl;
            mSettings.m_hLogFile << "Program," << lFile->MirData.szJobName << endl;
            mSettings.m_hLogFile << "ProgramRev," << lFile->MirData.szJobRev << endl;
            mSettings.m_hLogFile << "Lot," << lFile->MirData.szLot << endl;
            mSettings.m_hLogFile << "SubLot," << lFile->MirData.szSubLot << endl;
            mSettings.m_hLogFile << "WaferID," << lFile->getWaferMapData().szWaferID << endl;

            mSettings.m_hLogFile << "#" << endl;
            mSettings.m_hLogFile << "# STDF Wafer details" << endl;
            mSettings.m_hLogFile << "Stdf_Rows," << lFile->getWaferMapData().SizeX << endl;
            mSettings.m_hLogFile << "Stdf_Columns,"<< lFile->getWaferMapData().SizeY << endl;
            mSettings.m_hLogFile << "Stdf_TotalDies," << lGroup->cMergedData.lTotalSoftBins << endl;
            if(mSettings.iGrossDiePerWafer > 0)
                mSettings.m_hLogFile << "Stdf_GrossDiesPerWafer," << mSettings.iGrossDiePerWafer << endl;

            if(mSettings.strWaferFileFullName.isEmpty() == false)
                mSettings.m_hLogFile << "WafMapFile," << mSettings.strWaferFileFullName << endl;	// Wafermap file created (if any)

            mSettings.m_hLogFile << "#" << endl;
            mSettings.m_hLogFile << "# Binning info" << endl;

            // Display Bin classes
            mSettings.m_hLogFile << "BinName,";
            ptBinCell = lGroup->cMergedData.ptMergedSoftBinList;
            while(ptBinCell != NULL)
            {
                // Display Bin name
                mSettings.m_hLogFile << ptBinCell->strBinName << ",";

                // Move to next Bin cell
                ptBinCell = ptBinCell->ptNextBin;
            };
            mSettings.m_hLogFile << endl;

            // Display Bin classes
            mSettings.m_hLogFile << "Bin#,";
            ptBinCell = lGroup->cMergedData.ptMergedSoftBinList;
            while(ptBinCell != NULL)
            {
                // Display Bin ID
                mSettings.m_hLogFile << ptBinCell->iBinValue << ",";

                // Move to next Bin cell
                ptBinCell = ptBinCell->ptNextBin;
            };
            mSettings.m_hLogFile << endl;

            // Display Bin count
            mSettings.m_hLogFile << "BinCount,";
            ptBinCell = lGroup->cMergedData.ptMergedSoftBinList;
            while(ptBinCell != NULL)
            {
                // Display Bin count
                mSettings.m_hLogFile << ptBinCell->ldTotalCount << ",";

                // Keep track of total Good dies & total fail dies.
                if((ptBinCell->cPassFail == 'P') || (ptBinCell->cPassFail == 'p'))
                    lTotalGood += ptBinCell->ldTotalCount;

                // Move to next Bin cell
                ptBinCell = ptBinCell->ptNextBin;
            };
            mSettings.m_hLogFile << endl;

            // Include few yield related info
            mSettings.m_hLogFile << "#" << endl;
            mSettings.m_hLogFile << "# Yield details" << endl;
            mSettings.m_hLogFile << "GoodParts," << lTotalGood << endl;

        }// If file processed


        // Add contents of trigger file for traceability purpose
        QFile lTriggerFile(mSettings.strTriggerFile);
        if(lTriggerFile.open(QIODevice::ReadOnly))
        {
            // Assign file I/O stream
            QTextStream hTriggerFile(&lTriggerFile);
            mSettings.m_hLogFile << "#" << endl;
            mSettings.m_hLogFile << "#" << endl;
            mSettings.m_hLogFile << "# Trigger file processed: " << mSettings.strTriggerFile << endl;
            mSettings.m_hLogFile << "#" << endl;
            do
                mSettings.m_hLogFile << "# " << hTriggerFile.readLine() << endl;
            while(hTriggerFile.atEnd() == false);

            hTriggerFile.setDevice(0);
            lTriggerFile.close();
        }

        // Close file
        mSettings.m_hLogFile.setDevice(0);
        mSettings.m_LogFile.close();

        // Rename log file to final name
        QDir lDir;
        lDir.remove(mSettings.m_strLogFileName);
        lDir.rename(mSettings.m_strTemporaryLog, mSettings.m_strLogFileName);
    }
}*/

}
}
