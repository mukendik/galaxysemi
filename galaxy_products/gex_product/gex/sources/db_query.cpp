#include <QProgressBar>
#include <QFileDialog>
#include <gqtl_sysutils.h>

#include "browser_dialog.h"
#include "gex_shared.h"
#include "db_engine.h"
#include "db_transactions.h"
#include "gex_database_entry.h"
#include "db_gexdatabasequery.h"
#include <gqtl_log.h>
#include "report_options.h"
#include "engine.h"
#include "message.h"

extern CReportOptions			ReportOptions;		// Holds options (report_build.h)

///////////////////////////////////////////////////////////
// Get list of files matching a query (for data analysis or HouseKeeping).
///////////////////////////////////////////////////////////
QStringList GS::Gex::DatabaseEngine::QuerySelectFiles(GexDatabaseQuery *pQuery, QString& aErrorMessage, unsigned uFileLimit/*=0*/)
{
    QStringList cMatchingFiles;
    aErrorMessage.clear();

    if (!pQuery)
        return cMatchingFiles;
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" on db '%1' TimePeriod:%2 TimeFactor:%3 TimeStep:%4")
          .arg(pQuery->strDatabaseLogicalName.toLatin1().data())
          .arg(pQuery->iTimePeriod)
          .arg(pQuery->iTimeNFactor)
          .arg(pQuery->m_eTimeStep )
          .toLatin1().constData());

    GexDatabaseEntry *pDatabaseEntry=NULL;
    QDate		cCurrentDate=QDate::currentDate(), cDate;
    int			iDays;

    pDatabaseEntry = FindDatabaseEntry(pQuery->strDatabaseLogicalName);
    if(pDatabaseEntry == NULL)
        return cMatchingFiles;	// Failed finding entry!

    // Save database type into the query structure
    pQuery->bBlackHole = pDatabaseEntry->IsBlackHole();
    pQuery->bCompressed = pDatabaseEntry->IsCompressed();
    pQuery->bExternal = pDatabaseEntry->IsExternal();
    pQuery->bHoldFileCopy = pDatabaseEntry->HoldsFileCopy();
    pQuery->bLocalDatabase = pDatabaseEntry->IsStoredInFolderLocal();
    pQuery->bSummaryOnly = pDatabaseEntry->IsSummaryOnly();

    // If query over External database (and not doing local database housekeeping), then trigger external query
    if(pDatabaseEntry->IsExternal() && (pQuery->bOfflineQuery == false))
    {
        QStringList						strlCorruptedFiles;
        GexDatabaseInsertedFilesList	listInsertedFiles;
        bool							bEditHeaderMode=false;
        QDir							cDir;
        QString							strFile, strLocalFtpPath;
        QStringList						strDataFilesToImport, strDataFiles;
        QStringList::Iterator			it;

        // Make sure the database /.temp_ftp/ folder exists!
        strLocalFtpPath = pDatabaseEntry->PhysicalPath() + GEX_DATABASE_TEMP_FTP_FOLDER;
        CGexSystemUtils::NormalizePath(strLocalFtpPath);
        cDir.mkdir(strLocalFtpPath);

        // Remove all files from the /.temp_ftp/ directory
        cDir.setPath(strLocalFtpPath);
        cDir.setFilter(QDir::Files | QDir::Hidden);
        strDataFiles = cDir.entryList(QStringList() << "*");
        for (it = strDataFiles.begin(); it != strDataFiles.end(); ++it )
        {
            // If folder is '.' or '..', ignore!
            if((*it != ".") && (*it != ".."))
            {
                strFile = strLocalFtpPath + "/" + *it;
                CGexSystemUtils::NormalizePath(strFile);
                cDir.remove(strFile);
            }
        }

        // Map regular Galaxy query to a Corporate Filter query.
        GexDbPlugin_Filter clPluginFilter(this);
        MapQueryToCorporateFilters(clPluginFilter, *pQuery);

        // Get list of files to retrieve
        tdGexDbPluginDataFileList		cDataFiles;
        bool							bFilesCreatedInFinalLocation=false;
        GexDbPlugin_Base::StatsSource	eStatsSource;

        {
            QString strOptionStorageDevice = (ReportOptions.GetOption("statistics","computation")).toString();
            if(strOptionStorageDevice == "samples_only")				// Compute statistics from samples only
            {
                eStatsSource = GexDbPlugin_Base::eStatsFromSamplesOnly;
            }
            else if(strOptionStorageDevice == "summary_only")			// Compute statistics from summary only
            {
                eStatsSource = GexDbPlugin_Base::eStatsFromSummaryOnly;
            }
            else if(strOptionStorageDevice == "samples_then_summary")	// Compute statistics from samples (if exists), then summary
            {
                eStatsSource = GexDbPlugin_Base::eStatsFromSamplesThenSummary;
            }
            else if(strOptionStorageDevice == "summary_then_samples")	// Compute statistics from summary (if exists), then samples
            {
                eStatsSource = GexDbPlugin_Base::eStatsFromSummaryThenSamples;
            }
            else
            {
                eStatsSource = GexDbPlugin_Base::eStatsFromSummaryThenSamples;
                GSLOG(SYSLOG_SEV_WARNING, QString(" error : unknown Option(statistics,computation) '%1' ")
                      .arg(strOptionStorageDevice.toLatin1().data() )
                      .toLatin1().constData());
                GEX_ASSERT(false);
            }
        }

        if(!pDatabaseEntry->m_pExternalDatabase->QueryDataFiles(
                    clPluginFilter, pQuery->strTestList, cDataFiles, pDatabaseEntry->PhysicalPath(),
                    strLocalFtpPath, &bFilesCreatedInFinalLocation, eStatsSource))
        {
            //GSLOG(SYSLOG_SEV_WARNING, "Query data files failed ");
            pDatabaseEntry->m_pExternalDatabase->GetLastError(aErrorMessage);
            if(aErrorMessage.contains("TDR"))
                aErrorMessage = aErrorMessage.section("TDR",1).section(":",1).trimmed();
            //std::string em;  pDatabaseEntry->m_pExternalDatabase->m_clLastErrorGexRemoteDatabase.GetErrorMessage(em);
            GSLOG(SYSLOG_SEV_ERROR, aErrorMessage.toLatin1().data() );
            QString m=QString("Error while extracting data from database.\n\n%1")
                    .arg(aErrorMessage);
            GS::Gex::Message::information("", m);
        }
        else
        {
            // Make sure some files were created
            if(cDataFiles.count() > 0)
            {
                if(!bFilesCreatedInFinalLocation && HasFtpFiles(cDataFiles))
                {
                    // Only retrieve files not already in the database!
                    SimplifyFtpList(pDatabaseEntry,cDataFiles,cMatchingFiles);

                    // FTP files to local "<database>/.temp_ftp" folder.
                    pDatabaseEntry->m_pExternalDatabase->TransferDataFiles(cDataFiles, strLocalFtpPath);

                    // Create list of absolute path to files just FTPed
                    cDir.setPath(strLocalFtpPath);
                    cDir.setFilter(QDir::Files | QDir::Hidden);
                    strDataFiles = cDir.entryList(QStringList() << "*");
                    for (it = strDataFiles.begin(); it != strDataFiles.end(); ++it )
                    {
                        // If folder is '.' or '..', ignore!
                        if((*it != ".") && (*it != ".."))
                        {
                            strFile = strLocalFtpPath + "/" + *it;
                            CGexSystemUtils::NormalizePath(strFile);
                            strDataFilesToImport += strFile;
                        }
                    }
                }
                else
                {
                    tdGexDbPluginDataFileListIterator	lstIteratorDataFile(cDataFiles);
                    GexDbPlugin_DataFile *				pFile = NULL;

                    while(lstIteratorDataFile.hasNext())
                    {
                        pFile = lstIteratorDataFile.next();

                        if (pFile)
                        {
                            strFile = pFile->m_strFilePath + "/" + pFile->m_strFileName;
                            CGexSystemUtils::NormalizePath(strFile);
                            strDataFilesToImport += strFile;
                        }
                    }
                }

                // Insert files into the database, delete after insertion is completed
                bool b=ImportFiles(pQuery->strDatabaseLogicalName,
                                   strDataFilesToImport,&strlCorruptedFiles,listInsertedFiles,
                                   aErrorMessage,bEditHeaderMode,true,NULL,true,bFilesCreatedInFinalLocation);
                if (!b)
                    GSLOG(SYSLOG_SEV_ERROR, "Query select files : ImportFiles into filebased db failed");
                GexDatabaseInsertedFilesList::iterator itInserted;
                for(itInserted = listInsertedFiles.begin(); itInserted != listInsertedFiles.end(); itInserted++)
                    cMatchingFiles += (*itInserted).m_strDestFile;

                // Update list of files FTPed over
                UpdateFtpList(pDatabaseEntry,cDataFiles,listInsertedFiles,strlCorruptedFiles);

                // Make sure status line is visible (so user can see Examinator ongoing actions)
                UpdateStatusMessage(" ");
            }
        }

        // Return list of inserted files
        return cMatchingFiles;
    }

    // Check the Calendar period to scan for
    switch(pQuery->iTimePeriod)
    {
    case GEX_QUERY_TIMEPERIOD_TODAY:
        cMatchingFiles = FindMatchingFiles(&cCurrentDate,pQuery,pDatabaseEntry);
        break;
    case GEX_QUERY_TIMEPERIOD_LAST2DAYS:
        for(iDays=0;iDays > -2 ;iDays--)
        {
            cDate = cCurrentDate.addDays(iDays);
            cMatchingFiles += FindMatchingFiles(&cDate,pQuery,pDatabaseEntry);

            // Check if we have set a lmit of the number of files to retrieve (used if the query is only to see the list of tests in files)
            if (uFileLimit &&
                    uFileLimit >= (unsigned int) cMatchingFiles.count())
                return cMatchingFiles;
        }
        break;
    case GEX_QUERY_TIMEPERIOD_LAST3DAYS:
        for(iDays=0;iDays > -3 ;iDays--)
        {
            cDate = cCurrentDate.addDays(iDays);
            cMatchingFiles += FindMatchingFiles(&cDate,pQuery,pDatabaseEntry);

            // Check if we have set a lmit of the number of files to retrieve (used if the query is only to see the list of tests in files)
            if (uFileLimit &&
                    uFileLimit >= (unsigned int) cMatchingFiles.count())
                return cMatchingFiles;
        }
        break;
    case GEX_QUERY_TIMEPERIOD_LAST7DAYS:
        for(iDays=0;iDays > -7 ;iDays--)
        {
            cDate = cCurrentDate.addDays(iDays);
            cMatchingFiles += FindMatchingFiles(&cDate,pQuery,pDatabaseEntry);

            // Check if we have set a lmit of the number of files to retrieve (used if the query is only to see the list of tests in files)
            if (uFileLimit &&
                    uFileLimit >= (unsigned int) cMatchingFiles.count())
                return cMatchingFiles;
        }
        break;
    case GEX_QUERY_TIMEPERIOD_LAST14DAYS:
        for(iDays=0;iDays > -14 ;iDays--)
        {
            cDate = cCurrentDate.addDays(iDays);
            cMatchingFiles += FindMatchingFiles(&cDate,pQuery,pDatabaseEntry);

            // Check if we have set a lmit of the number of files to retrieve (used if the query is only to see the list of tests in files)
            if (uFileLimit &&
                    uFileLimit >= (unsigned int) cMatchingFiles.count())
                return cMatchingFiles;
        }
        break;
    case GEX_QUERY_TIMEPERIOD_LAST31DAYS:
        for(iDays=0;iDays > -31 ;iDays--)
        {
            cDate = cCurrentDate.addDays(iDays);
            cMatchingFiles += FindMatchingFiles(&cDate,pQuery,pDatabaseEntry);

            // Check if we have set a lmit of the number of files to retrieve (used if the query is only to see the list of tests in files)
            if (uFileLimit &&
                    uFileLimit >= (unsigned int) cMatchingFiles.count())
                return cMatchingFiles;
        }
        break;
    case GEX_QUERY_TIMEPERIOD_LAST_N_X:
        // code me
        GSLOG(SYSLOG_SEV_ERROR, "code me");
        GEX_ASSERT(false);
        break;
    case GEX_QUERY_TIMEPERIOD_THISWEEK:
        // From 'Monday' to current date.
        cDate = cCurrentDate;
        cMatchingFiles = FindMatchingFiles(&cDate,pQuery,pDatabaseEntry);
        while(cDate.dayOfWeek() != 1)
        {
            cDate = cDate.addDays(-1);
            cMatchingFiles += FindMatchingFiles(&cDate,pQuery,pDatabaseEntry);

            // Check if we have set a lmit of the number of files to retrieve (used if the query is only to see the list of tests in files)
            if (uFileLimit &&
                    uFileLimit >= (unsigned int) cMatchingFiles.count())
                return cMatchingFiles;
        };
        break;
    case GEX_QUERY_TIMEPERIOD_THISMONTH:
        // From 1st day of the month to today...
        cDate = cCurrentDate;
        cMatchingFiles = FindMatchingFiles(&cDate,pQuery,pDatabaseEntry);
        while(cDate.day() != 1)
        {
            cDate = cDate.addDays(-1);
            cMatchingFiles += FindMatchingFiles(&cDate,pQuery,pDatabaseEntry);

            // Check if we have set a lmit of the number of files to retrieve (used if the query is only to see the list of tests in files)
            if (uFileLimit &&
                    uFileLimit >= (unsigned int) cMatchingFiles.count())
                return cMatchingFiles;
        };
        break;
    case GEX_QUERY_TIMEPERIOD_CALENDAR:
        // Find all valid years/months/days entries matching From-To dates...
        for(cDate = pQuery->calendarFrom; cDate <= pQuery->calendarTo; )
        {
            cMatchingFiles += FindMatchingFiles(&cDate,pQuery,pDatabaseEntry);
            cDate = cDate.addDays(1);

            // Check if we have set a lmit of the number of files to retrieve (used if the query is only to see the list of tests in files)
            if (uFileLimit &&
                    uFileLimit >= (unsigned int) cMatchingFiles.count())
                return cMatchingFiles;
        }
        break;
    case GEX_QUERY_TIMEPERIOD_ALLDATES:
    default:
        // Scan all valid folders!
        QDir d;
        QString		strFolder,strYearFolder,strMonthFolder;
        QStringList strValidYears,strValidMonths,strValidDays;
        QStringList::Iterator itYear,itMonth,itDay;
        int	iYear,iMonth,iDay;

        // Build sub-path from database home location.
        strFolder = pDatabaseEntry->PhysicalPath();

        // Find all valid YEAR, Month, Day folders!
        d.setPath(strFolder);
        d.setFilter(QDir::Dirs);
        strValidYears = d.entryList(QStringList() << "*");
        for(itYear = strValidYears.begin(); itYear != strValidYears.end(); ++itYear )
        {
            // Skip folders names that start with a '.'
            if((*itYear).startsWith("."))
                goto next_Year_Entry;

            // List of valid YEARS folders
            iYear = (*itYear).toInt();

            // Move to YEAR folder
            strYearFolder = strFolder + "/" + *itYear;
            d.setPath(strYearFolder);
            strValidMonths = d.entryList(QStringList() << "*");
            for(itMonth = strValidMonths.begin(); itMonth != strValidMonths.end(); ++itMonth )
            {
                // Skip '.' and '..' folders.
                if((*itMonth == ".") || (*itMonth == ".."))
                    goto next_Month_Entry;

                // List of valid MONTHS folders
                iMonth = (*itMonth).toInt();

                // Move to MONTH folder
                strMonthFolder = strYearFolder + "/" + *itMonth;
                d.setPath(strMonthFolder);
                strValidDays = d.entryList(QStringList() << "*");
                for(itDay = strValidDays.begin(); itDay != strValidDays.end(); ++itDay )
                {
                    // Skip '.' and '..' folders.
                    if((*itDay == ".") || (*itDay == ".."))
                        goto next_Day_Entry;
                    // List of valid Days folders
                    iDay = (*itDay).toInt();
                    if (iYear < 99) iYear += 1900;
                    cDate.setDate(iYear,iMonth,iDay);
                    cMatchingFiles += FindMatchingFiles(&cDate,pQuery,pDatabaseEntry);

                    // Check if we have set a limit of the number of files to retrieve (used if the query is only to see the list of tests in files)
                    if(uFileLimit && (cMatchingFiles.count() >= (int)uFileLimit))
                        return cMatchingFiles;
next_Day_Entry: ;
                }
next_Month_Entry: ;
            }
next_Year_Entry: ;
        }
        break;
    }

    return cMatchingFiles;
}

bool GS::Gex::DatabaseEngine::isYMDB(const QString &strDB )
{
    GexDatabaseEntry *pDatabaseEntry=NULL;
    pDatabaseEntry = FindDatabaseEntry(strDB);
    if(pDatabaseEntry == NULL)
        return false;
    return  pDatabaseEntry->IsYmProdTdr();
}

bool GS::Gex::DatabaseEngine::ExportCSVCondition(GexDatabaseQuery *pQuery, const QMap<QString, QString> &oConditions , QProgressDialog *poProgress){
    if (!pQuery)
        return false;

    GexDatabaseEntry *pDatabaseEntry=NULL;
    pDatabaseEntry = FindDatabaseEntry(pQuery->strDatabaseLogicalName);
    if(pDatabaseEntry == NULL)
        return false;	// Failed finding entry!

    // Save database type into the query strucutre
    pQuery->bBlackHole = pDatabaseEntry->IsBlackHole();
    pQuery->bCompressed = pDatabaseEntry->IsCompressed();
    pQuery->bExternal = pDatabaseEntry->IsExternal();
    pQuery->bHoldFileCopy = pDatabaseEntry->HoldsFileCopy();
    pQuery->bLocalDatabase = pDatabaseEntry->IsStoredInFolderLocal();
    pQuery->bSummaryOnly = pDatabaseEntry->IsSummaryOnly();
    if(pDatabaseEntry->IsExternal() && (pQuery->bOfflineQuery == false))
    {
        GexDbPlugin_Filter oPluginFilter(this);
        GexDbPlugin_SplitlotList oSplitLotList;
        MapQueryToCorporateFilters(oPluginFilter, *pQuery);

        bool bDataFound = true;
        if(!pDatabaseEntry->m_pExternalDatabase->QuerySplitlots(oPluginFilter, oSplitLotList, true))
            bDataFound = false;

        if(poProgress){
            poProgress->setLabelText("Retrieving date from database");
            poProgress->setValue(10);
        }

        if(oSplitLotList.isEmpty())
            bDataFound = false;

        if(!bDataFound){
            GS::Gex::Message::warning(
                "",
                "No data found to be exported to csv characterization file.");
            return false;
        }

        if(poProgress)
            poProgress->hide();
        QString strDefaultFileName = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString()+QDir::separator();

        strDefaultFileName += QString("condition_%1_%2.csv")
                .arg(pDatabaseEntry->PhysicalName()).arg(GS::Gex::Engine::GetInstance().GetClientDateTime().toString("yyyy-MM-dd_HH-mm-ss"));

        QString strFileName = QFileDialog::getSaveFileName(0,
                                                           "Export to test condition CSV file",
                                                           strDefaultFileName,
                                                           "CSV (*.csv)");
        if(strFileName.isEmpty())
            return false;
        if(poProgress)
            poProgress->show();
        oPluginFilter.strTestList = pQuery->strTestList;
        bool bRet = pDatabaseEntry->m_pExternalDatabase->exportCSVCondition(strFileName, oConditions,oPluginFilter, oSplitLotList, poProgress);
        return bRet;

    }
    return false;
}

bool GS::Gex::DatabaseEngine::PurgeDataBase(
        GexDatabaseQuery *pQuery,
        QProgressBar *poProgress )
{
    if (!pQuery)
        return false;

    GexDatabaseEntry *pDatabaseEntry
            = FindDatabaseEntry(pQuery->strDatabaseLogicalName);
    if(pDatabaseEntry == NULL)
        return false;	// Failed finding entry!

    // Save database type into the query strucutre
    pQuery->bBlackHole = pDatabaseEntry->IsBlackHole();
    pQuery->bCompressed = pDatabaseEntry->IsCompressed();
    pQuery->bExternal = pDatabaseEntry->IsExternal();
    pQuery->bHoldFileCopy = pDatabaseEntry->HoldsFileCopy();
    pQuery->bLocalDatabase = pDatabaseEntry->IsStoredInFolderLocal();
    pQuery->bSummaryOnly = pDatabaseEntry->IsSummaryOnly();
    if(pDatabaseEntry->IsExternal() && (pQuery->bOfflineQuery == false))
    {
        GexDbPlugin_Filter oPluginFilter(this);
        GexDbPlugin_SplitlotList oSplitLotList;
        MapQueryToCorporateFilters(oPluginFilter, *pQuery);

        if(!pDatabaseEntry->IsYmProdTdr())
        {
            if(!pDatabaseEntry->m_pExternalDatabase->QuerySplitlots(oPluginFilter, oSplitLotList, true))
                return false;
            if(oSplitLotList.isEmpty())
                return false;
        }

        if(poProgress) poProgress->setValue(1* (100/11));

        bool bRet = pDatabaseEntry->m_pExternalDatabase->purgeDataBase(
                    oPluginFilter, oSplitLotList);
        poProgress->setValue(100);
        return bRet;
    }

    return false;
}
