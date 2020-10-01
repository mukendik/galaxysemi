///////////////////////////////////////////////////////////
// Examinator Monitoring: Scheduler page
///////////////////////////////////////////////////////////

#include <QFileDialog>
#include <QDate>
#include <QDateTime>
#include <QDesktopWidget>

// Galaxy modules includes
#include <gqtl_sysutils.h>
#include <gqtl_log.h>

#include "engine.h"
#include "yield/yield_task.h"
#include "yield/yield_taskdata.h"
#include "browser_dialog.h"
#include "cbinning.h"
#include "report_build.h"
#include "browser_dialog.h"
#include "report_options.h"
#include "pickproduct_id_dialog.h"
#include "scheduler_engine.h"
#include "status/status_task.h"
#include "status/status_taskdata.h"
#include "mo_email.h"
#include "autoadmin/autoadmin_taskdata.h"
#include "mo_task.h"
#include "gqtl_datakeys.h"
#include "db_transactions.h"
#include "gex_database_entry.h"
#include "tb_toolbox.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "gex_report.h"

#include "promis_interpreter_factory.h"
#include "promis_item_base.h"
#include "promis_interpreter_base.h"

#include <sstream>

extern CGexReport*		gexReport;			// Handle to report class
// All report generation options
extern CReportOptions	ReportOptions;

QString GS::Gex::SchedulerEngine::ExecuteYieldTask(GexDatabaseEntry *pDatabaseEntry,
        CGexMoTaskYieldMonitor *ptTask,
        QtLib::DatakeysContent &dbKeysContent)
{
    QString strString;
    strString = "Execute any Yield Task on product '";
    strString += ptTask->GetProperties()->strTitle;
    strString += QString("' for %1 groups...").arg(gexReport->getGroupsList().size());
    GSLOG(SYSLOG_SEV_NOTICE, strString.toLatin1().data());

    QString strErrorMessage;

    // Get pointer to first group & first file & parameter list
    CGexGroupOfFiles    *pGroup=0;
    CGexFileInGroup     *pFile=NULL;
    GS::QtLib::Range    *pProductYieldBinCheck=NULL;
    CBinning            *ptBinCell;     // Pointer to SoftBin cell
    long                lTotalSoftBins=0;
    long                lTotalBins=0;
    long                lTotalPartsTested=0;
    long                lGrossDieCount=-1;
    double              lfYieldRate=0;
    QString             strEmailBody;
    //QString           strYieldAlarmMessage;
    bool                bYieldAlarm=false;
    int                 iTotalAlarmCount=0;
    int                 lAlarmSeverity=0;
    bool                bSblMode=false;	// Set to 'true' if Yield limits are SBL (saved into a file)
    QString             strYieldAlarmDetails;
    bool                bMultiSiteFile = (gexReport->getGroupsList().count() > 1);
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Execute yield task : %1 group(s) will be considered")
          .arg(gexReport->getGroupsList().count()).toLatin1().constData());
    QList <int>         cSitesList;                 // Holds list of testing sites.
    cSitesList.clear();
    QMap<int, GexDbPlugin_BinList>  sitesbins;      // only used for BinsPerSite task type
    QMap<int, float>                sites2percent;  // only used for BinsPerSite task type

    // Compute total parts in data file
    pGroup = gexReport->getGroupsList().isEmpty()?NULL:gexReport->getGroupsList().first();

    // VISHAY - Check if 'GEX_PROMIS_DATA_PATH' environment variable is set
    QString lVishayEnv = QString(getenv("GEX_PROMIS_DATA_PATH"));

    // Process ALL tasks that refer to this Product...
    QString Filter;
    Filter = "Product="+dbKeysContent.Get("Product").toString();
    Filter+= "|Database="+dbKeysContent.Get("DatabaseName").toString();
    Filter+= "|TestingStage="+dbKeysContent.Get("TestingStage").toString();
    while(ptTask != NULL)
    {
        lTotalSoftBins=0;
        lTotalBins=0;
        lTotalPartsTested=0;
        lGrossDieCount=-1;
        lfYieldRate=0;
        bYieldAlarm=false;
        iTotalAlarmCount=0;
        lAlarmSeverity=0;
        sitesbins.clear();
        sites2percent.clear();
        cSitesList.clear();

        // Clear email message
        strEmailBody = "";

        // Build Parameter list selected in Task
        pProductYieldBinCheck = new GS::QtLib::Range(
                    ptTask->GetProperties()->strYieldBinsList.toLatin1().constData());

        QString checkType=ptTask->GetAttribute("CheckType").toString();

        // Check Yield for each testing site!
        QListIterator<CGexGroupOfFiles*> itGroupsList(gexReport->getGroupsList());
        while(itGroupsList.hasNext())
        {
            // Point to dataset
            pGroup	= itGroupsList.next();
            pFile	= (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
            if (!pFile)
            {
                GSLOG(SYSLOG_SEV_WARNING, "File NULL");
                continue;
            }

            // Compute total of Soft Bins matching our criteria...
            ptBinCell= pGroup->cMergedData.ptMergedSoftBinList;
            lTotalSoftBins = lTotalBins = lTotalPartsTested = 0;
            while(ptBinCell != NULL)
            {
                if(pProductYieldBinCheck->Contains(ptBinCell->iBinValue) == true)
                    lTotalSoftBins += ptBinCell->ldTotalCount;
                lTotalBins += ptBinCell->ldTotalCount;
                ptBinCell = ptBinCell->ptNextBin;
            };

            // Check if must use Gross Die count to compute yield...
            lTotalPartsTested = lTotalBins;
            if(!lVishayEnv.isEmpty()
                    && (dbKeysContent.Get("GrossDie").toInt() > 0))
                lGrossDieCount = lTotalBins = dbKeysContent.Get("GrossDie").toInt();	// Gross die count exists!

            // Compute Yield Percentage for given testing site.
            if(lTotalBins)
                lfYieldRate = 100.0 * ((double) lTotalSoftBins) / (double) lTotalBins;
            else
                lfYieldRate = 0;

            // If SBL/YBL data file defined
            // "Unknow" , "FixedYieldTreshold" , "SYASBL_File" , "BinsPerSite" , "SYASBL_RDB"
            //if(QFile::exists(ptTask->GetProperties()->strSblFile))
            // check if Yield level triggers alarm (only if we have enough samples: N samples for all file = N/sites per testing site)
            if((lTotalBins*(int)gexReport->getGroupsList().count()) >= ptTask->GetProperties()->lMinimumyieldParts)
            {
                if (checkType=="SYASBL_File")
                {
                    GSLOG(SYSLOG_SEV_WARNING, "SYASBL_File check type no more supported");
                    continue;
                }
                else if (checkType=="FixedYieldTreshold")
                {
                    if(ptTask->GetProperties()->iAlarmIfOverLimit)
                    {
                        // Check if OVER limit
                        if(lfYieldRate > ptTask->GetProperties()->iAlarmLevel)
                        {
                            GSLOG(SYSLOG_SEV_INFORMATIONAL, "FixedYieldTreshold = true");
                            GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("lfYieldRate = %1 and ptTask->GetProperties()->iAlarmLevel = %2")
                                  .arg(QString::number(lfYieldRate))
                                  .arg(QString::number(ptTask->GetProperties()->iAlarmLevel)).toLatin1().constData());
                            GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("ptTask->GetProperties()->iAlarmIfOverLimit is not null = %1")
                                  .arg(QString::number(ptTask->GetProperties()->iAlarmIfOverLimit)).toLatin1().constData());
                            bYieldAlarm = true;
                            // Keep track of total alarm conditions detected
                            iTotalAlarmCount++;
                            // Message about yield issue
                            if(bMultiSiteFile)
                            {
                                // Multi site yield report
                                strYieldAlarmDetails += QString::number(lfYieldRate,'f',2) + "% (" + pGroup->strGroupName + ") ; ";
                            }
                        }
                        else
                            bYieldAlarm= false;
                    }
                    else
                    {
                        // Check if UNDER limit
                        if(lfYieldRate < ptTask->GetProperties()->iAlarmLevel)
                        {
                            GSLOG(SYSLOG_SEV_INFORMATIONAL, "FixedYieldTreshold = true");
                            GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("lfYieldRate = %1 and ptTask->GetProperties()->iAlarmLevel = %2")
                                  .arg(QString::number(lfYieldRate))
                                  .arg(QString::number(ptTask->GetProperties()->iAlarmLevel)).toLatin1().constData());
                            GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("ptTask->GetProperties()->iAlarmIfOverLimit is null = %1")
                                  .arg(QString::number(ptTask->GetProperties()->iAlarmIfOverLimit)).toLatin1().constData());
                            bYieldAlarm = true;
                            // Keep track of total alarm conditions detected
                            iTotalAlarmCount++;
                            // Message about yield issue
                            if(bMultiSiteFile)
                            {
                                // Multi site yield report
                                strYieldAlarmDetails += QString::number(lfYieldRate,'f',2) + "% (" + pGroup->strGroupName + ") ; ";
                            }
                        }
                        else
                            bYieldAlarm= false;
                    }
                }
                else if (checkType=="BinsPerSite")
                {
                    GSLOG(SYSLOG_SEV_NOTICE, "Checking Bins percent delta per Site...");
                    QString r=pDatabaseEntry->m_pExternalDatabase->GetPluginID()
                            ->m_pPlugin->GetSplitlotBinCountsPerSite(
                                dbKeysContent.Get("TestingStage").toString(),  //"Final Test",
                                dbKeysContent.Get("SplitlotId").toLongLong(),
                                sitesbins);
                    //GSLOG(SYSLOG_SEV_NOTICE, QString("GetSplitlotBinCountsPerSite returned %1 : %2 sites found").arg( r).toLatin1().constData(), sitesbins.size() );.arg(                //GSLOG(SYSLOG_SEV_NOTICE, "GetSplitlotBinCountsPerSite returned %1 : %2 sites found").arg( r).toLatin1().constData().arg( sitesbins.size() );
                    if (r.startsWith("error"))
                    {
                        GSLOG(SYSLOG_SEV_WARNING, QString("GetSplitlotBinCountsPerSite failed : %1").arg( r).toLatin1().constData() );
                        continue;
                    }

                    foreach(const GexDbPlugin_BinList &bl, sitesbins.values())
                    {
                        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Site %1").arg(sitesbins.key(bl)).toLatin1().constData());
                        GexDbPlugin_BinInfo* bi=bl.first();
                        int total=0;
                        int total_selected_bins=0;
                        //                    while(bi)
                        foreach(bi, bl)
                        {
                            total+=bi->m_nBinCount;
                            if (!pProductYieldBinCheck->Contains(bi->m_nBinNo) )
                            {
                                //                            bi=bl.next();
                                continue;
                            }
                            //GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" bin %1 : %2").arg( bi->m_nBinNo, bi->m_nBinCount));.arg(                        //GSLOG(SYSLOG_SEV_INFORMATIONAL, " bin %1 : %2").arg( bi->m_nBinNo.arg( bi->m_nBinCount);
                            total_selected_bins += bi->m_nBinCount;
                            //                        bi=bl.next();
                        }
                        float p=100.f*(float)total_selected_bins/(float)total;
                        GSLOG(SYSLOG_SEV_NOTICE,
                              QString("Site %1 : %2 / %3 : %4 %5")
                              .arg(sitesbins.key(bl))
                              .arg(total_selected_bins)
                              .arg(total)
                              .arg(p).toLatin1().constData());
                        sites2percent.insert(sitesbins.key(bl), p);
                    }

                    float MaxDelta=ptTask->GetAttribute("MaxDeltaBinsPerSite").toFloat();
                    lfYieldRate=MaxDelta;
                    // let s find the max
                    int maxsite=-1; float maxpercent=-1.f;
                    foreach(int s, sites2percent.keys())
                        if (maxpercent<sites2percent.value(s))
                        {
                            maxsite=s; maxpercent=sites2percent.value(s);
                        }

                    GSLOG(SYSLOG_SEV_NOTICE, QString("The reference will be %1 with a percent of %2").arg( maxsite)
                          .arg(maxpercent).toLatin1().constData());
                    bool alert=false;
                    foreach(int s, sites2percent.keys())
                    {
                        if (s==maxsite)
                            continue;
                        if (fabs((double) (sites2percent.value(s)-maxpercent))>MaxDelta)
                        {
                            GSLOG(SYSLOG_SEV_WARNING, QString("Alarm : site %1 too far from reference site %2").arg( s)
                                  .arg(maxsite).toLatin1().constData());
                            alert=true;
                        }
                    }
                    if (!alert)
                        continue; // next group
                    bYieldAlarm=true;
                }
                else
                    GSLOG( SYSLOG_SEV_WARNING, QString("Not supported check type '%1'").arg( checkType).toLatin1().constData() );
            }
        };  // All testing sites analyzed. Really ?

        // Delete Bin list used for this task.
        delete pProductYieldBinCheck; pProductYieldBinCheck=0;

        // Alarm condition: Save Alarm (if RDB) + Create email message
        if(bYieldAlarm == true)
        {
            // If RDB database, insert alarm into DB
            if(pDatabaseEntry->IsExternal()
                    && pDatabaseEntry->m_pExternalDatabase
                    && pDatabaseEntry->m_pExternalDatabase->IsInsertionSupported())
            {
                if(!bSblMode)
                {
                    GexDbPlugin_Base::AlarmCategories	eAlarmCat;
                    GexDbPlugin_Base::AlarmLevels		eAlarmLevel;
                    unsigned int						uiFlags = 0;
                    float								fLCL=0.0, fUCL=0.0;
                    QString								strItemName;

                    // Set item Name
                    strItemName = "YIELD (Bins: ";
                    if (checkType=="BinsPerSite")
                        strItemName = "DELTA SITES (Bins: ";
                    strItemName += ptTask->GetProperties()->strYieldBinsList;
                    strItemName += ")";

                    if(ptTask->GetProperties()->iExceptionLevel == 0)
                        eAlarmLevel = GexDbPlugin_Base::eAlarmLevel_Standard;
                    else
                        eAlarmLevel = GexDbPlugin_Base::eAlarmLevel_Critical;

                    eAlarmCat = GexDbPlugin_Base::eAlarmCat_Yield;
                    if (checkType=="BinsPerSite")
                        eAlarmCat = GexDbPlugin_Base::eAlarmCat_BinsPercentPerSite;

                    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("before the compare, the value of ptTask->GetProperties()->iAlarmIfOverLimit = %1")
                          .arg(QString::number(ptTask->GetProperties()->iAlarmIfOverLimit)).toLatin1().constData());
                    if(ptTask->GetProperties()->iAlarmIfOverLimit)
                    {
                        uiFlags |= GEXDB_PLUGIN_PRODALARM_LCL_VALID;
                        fLCL = ptTask->GetProperties()->iAlarmLevel;
                    }
                    else
                    {
                        uiFlags |= GEXDB_PLUGIN_PRODALARM_UCL_VALID;
                        fUCL = ptTask->GetProperties()->iAlarmLevel;
                    }

                    bool b=pDatabaseEntry->m_pExternalDatabase->InsertAlarm(
                                eAlarmCat, eAlarmLevel, 0, strItemName, uiFlags, fLCL, fUCL, lfYieldRate, "%");
                    if (!b)
                        GSLOG(SYSLOG_SEV_WARNING, "Insert Alarm failed");
                } // not SblMode
            } // insert Alarm

            // Get Email spooling folder.
            CGexMoTaskStatus *ptStatusTask = GetStatusTask();
            if(ptStatusTask == NULL)
            {
                GSLOG(SYSLOG_SEV_ERROR, "failed to get Email spooling folder");
                strErrorMessage = "  > Reporting: failed to get Email spooling folder";
                onStopTask(ptTask,strErrorMessage);
                return strErrorMessage;
            }

            // Check if need to append report
            QString strReportType;
            QString strAttachment;
            switch(ptTask->GetProperties()->iEmailReportType)
            {
            case 0:	// Text: no need to create + append report
                break;
            case 1:	// WORD report to create & append
                strReportType = "word"; break;
            case 2:	// CSV report to create & append
                strReportType = "excel"; break;
            case 3:	// PowerPoint report to create & append
                strReportType = "ppt"; break;
            case 4:	// PDF report to create & append
                strReportType = "pdf"; break;
            }
            GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Appending report '%1'...").arg( strReportType.isEmpty()?"none":strReportType).toLatin1().constData());
            // If report to append, create it now
            QString	strReportName;
            QString strHttpReportName = "";
            if(ptTask->GetProperties()->iEmailReportType)
            {
                if(gexReport->buildReportArborescence(&ReportOptions,strReportType))
                {
                    bool bNeedPostProcessing = false;
                    gexReport->CreateTestReportPages(&bNeedPostProcessing);

                    // Does the report need post-processing?
                    if(bNeedPostProcessing == true)
                        gexReport->ReportPostProcessingConverter();
                }

                // If report to be sent as an attachment, then tell where the report is located
                if(ptTask->GetProperties()->iNotificationType == 0)
                    strAttachment = gexReport->reportAbsFilePath();
                else
                {
                    // Check if specify http path to report
                    if (!ptStatusTask->GetProperties()->reportHttpURL().isEmpty())
                        strHttpReportName = ptStatusTask->GetProperties()->reportHttpURL() + QDir::cleanPath("/" + gexReport->reportRelativeFilePath());
                }
            }

            // Build email + send it.
            GexMoBuildEmailString cEmailString;
            bool	bHtmlEmail = ptTask->GetProperties()->bHtmlEmail;	// 'true' if email to be sent in HTML format
            GexMoSendEmail Email;
            QString strFilePath;
            QString strFrom,strTo,strSubject;
            strFrom = ptTask->GetProperties()->strEmailFrom;
            strTo = ptTask->GetProperties()->strEmailNotify;
            strSubject = ptTask->GetProperties()->strTitle;

            QString strYield = QString::number(lfYieldRate,'f',2) + QString("% ");
            if(bSblMode == false)
            {
                GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("ptTask->GetProperties()->iAlarmIfOverLimit = %1 and first part = %2")
                      .arg(QString::number(ptTask->GetProperties()->iAlarmIfOverLimit))
                      .arg(strYield).toLatin1().constData());
                // Yield limits hard coded
                if(ptTask->GetProperties()->iAlarmIfOverLimit == 0)
                    strYield += "< ";
                else
                    strYield += "> ";
                strYield += QString::number(ptTask->GetProperties()->iAlarmLevel,'f',2) + "%";
                GSLOG(SYSLOG_SEV_INFORMATIONAL,QString("The Yield is %1").arg(strYield).toLatin1().constData());
            }

            // Build email!
            time_t lStartTime = dbKeysContent.Get("StartTime").toUInt();

            if(bHtmlEmail)
            {
                // HTML Header
                cEmailString.CreatePage(strSubject);
                // Table with Lot Info
                cEmailString.WriteHtmlOpenTable();
                // Write table
                cEmailString.WriteInfoLine("Testing date", TimeStringUTC_F(lStartTime, "d MMMM yyyy h:mm:ss"));
                cEmailString.WriteInfoLine("Product", dbKeysContent.Get("Product").toString());
                cEmailString.WriteInfoLine("Lot", dbKeysContent.Get("Lot").toString());
                cEmailString.WriteInfoLine("SubLot", dbKeysContent.Get("SubLot").toString());
                cEmailString.WriteInfoLine("Wafer ID", pFile->getWaferMapData().szWaferID);

                // Add needed logs to have all informations to find the sent mail.
                GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Testing date %1")
                      .arg( TimeStringUTC_F(lStartTime, "d MMMM yyyy h:mm:ss")).toLatin1().constData());
                GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Product %1")
                      .arg(dbKeysContent.Get("Product").toString()).toLatin1().constData());
                GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Lot %1")
                      .arg(dbKeysContent.Get("Lot").toString()).toLatin1().constData());
                GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("SubLot %1")
                      .arg(dbKeysContent.Get("strSubLot").toString()).toLatin1().constData());
                GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Wafer ID %1")
                      .arg(pFile->getWaferMapData().szWaferID).toLatin1().constData());

                QString checkType=ptTask->GetAttribute("CheckType").toString();
                cEmailString.WriteInfoLine("Check type", checkType);

                cEmailString.WriteInfoLine("Soft Bins list",ptTask->GetProperties()->strYieldBinsList);

                if (checkType!="BinsPerSite")
                {
                    if(bMultiSiteFile)
                        cEmailString.WriteInfoLine("Yield Level",strYieldAlarmDetails);
                    else
                        cEmailString.WriteInfoLine("Yield Level",strYield,true);	// Highlight in red as it is a Yield Alarm!
                }
                else
                {
                    float MaxDelta=ptTask->GetAttribute("MaxDeltaBinsPerSite").toFloat();
                    cEmailString.WriteInfoLine("MaxDelta", QString("%1%").arg(MaxDelta));
                    // let s find the max %
                    int maxsite=-1; float maxpercent=-1.f;
                    foreach(int s, sites2percent.keys())
                        if (maxpercent<sites2percent.value(s))
                        {
                            maxsite=s; maxpercent=sites2percent.value(s);
                        }

                    //GSLOG(SYSLOG_SEV_NOTICE, QString("The reference will be %1 with a percent of %2").arg( maxsite, maxpercent));.arg(                    //GSLOG(SYSLOG_SEV_NOTICE, "The reference will be %1 with a percent of %2").arg( maxsite.arg( maxpercent);
                    foreach(int s, sites2percent.keys())
                    {
                        if (s==maxsite)
                        {
                            cEmailString.WriteInfoLine(
                                        QString("Site %1").arg(s),
                                        QString("%1 (Reference)").arg(sites2percent.value(s) )
                                        );
                            continue;
                        }

                        float delta=fabs((double) (sites2percent.value(s)-maxpercent));
                        if (delta>MaxDelta)
                        {
                            GSLOG(SYSLOG_SEV_WARNING, QString("Alarm : site %1 too far from reference site %2").arg( s)
                                  .arg( maxsite).toLatin1().constData());
                            cEmailString.WriteInfoLine( QString("Site %1").arg(s),
                                                        QString("%1 : %2 > %3").arg(sites2percent.value(s) ).arg(delta)
                                                        .arg(MaxDelta), true
                                                        );
                        }
                        else
                            cEmailString.WriteInfoLine(
                                        QString("Site %1").arg(s),
                                        QString("%1 ").arg(sites2percent.value(s) )
                                        );
                    }
                    cEmailString.WriteHtmlCloseTable();

                    foreach(int s, sitesbins.keys())
                    {
                        GexDbPlugin_BinList bl=sitesbins.value(s);
                        GexDbPlugin_BinInfo* bi=bl.first();
                        cEmailString.WriteInfoLine(QString("Site %1").arg(s));
                        cEmailString.WriteHtmlOpenTable();
                        QStringList sl;  sl << "Binning" << "Name" << "Cat" << "Total count"; // << "Percentage";
                        cEmailString.WriteLabelLineList(sl);
                        foreach(bi, bl)
                        {
                            sl.clear();
                            sl << QString::number(bi->m_nBinNo) << bi->m_strBinName << QString(bi->m_cBinCat)
                               << QString::number( bi->m_nBinCount ); // << QString::number( bi->m_nBinCount /  ) ;
                            cEmailString.WriteInfoLineList(sl);
                        }
                        cEmailString.WriteHtmlCloseTable();
                    }

                    cEmailString.WriteHtmlOpenTable();

                }

                cEmailString.WriteInfoLine();
                if(dbKeysContent.Get("TesterName").toString().isEmpty() == false)
                    cEmailString.WriteInfoLine("Tester",dbKeysContent.Get("TesterName").toString());
                if(dbKeysContent.Get("Operator").toString().isEmpty() == false)
                    cEmailString.WriteInfoLine("Operator",dbKeysContent.Get("Operator").toString());
                if(dbKeysContent.Get("ProgramName").toString().isEmpty() == false)
                    cEmailString.WriteInfoLine("Program name",dbKeysContent.Get("ProgramName").toString());
                if(dbKeysContent.Get("ProgramRevision").toString().isEmpty() == false)
                    cEmailString.WriteInfoLine("Program revision",dbKeysContent.Get("ProgramRevision").toString());
                cEmailString.WriteInfoLine("Total parts tested",QString::number(lTotalPartsTested));
                if(lGrossDieCount > 0)
                    cEmailString.WriteInfoLine("Gross die count",QString::number(lGrossDieCount));
                cEmailString.WriteInfoLine("Total in bin list",QString::number(lTotalSoftBins));
                // Table with Lot Info
                cEmailString.WriteHtmlCloseTable();

                // If Full Report to be created
                if(ptTask->GetProperties()->iEmailReportType)
                {
                    // Check if report to be attached or remain on server and only the URL to send...
                    if(ptTask->GetProperties()->iNotificationType == 0)
                        cEmailString.AddHtmlString("<br><b>Full report attached to this message!</b><br>\n");
                    else
                    {
                        // Leave report on server, email URL only.
                        cEmailString.AddHtmlString("<br><b>Full Report available on server at:<br>\n");

                        // Create Hyperlilnk
                        if (!strHttpReportName.isEmpty())
                        {
                            // Http hyperlink
                            cEmailString.AddHtmlString("<a href=\"");
                            cEmailString.AddHtmlString(strHttpReportName);
                            cEmailString.AddHtmlString("\">");
                            // Display File name
                            cEmailString.AddHtmlString(strHttpReportName);
                        }
                        else
                        {
                            // File hyperlink
                            cEmailString.AddHtmlString("<a href=\"file:///");
                            cEmailString.AddHtmlString(strReportName);
                            cEmailString.AddHtmlString("\">");
                            // Display File name
                            cEmailString.AddHtmlString(strReportName);
                        }
                        // Close URL hyperlink
                        cEmailString.AddHtmlString("</a>");
                        // Close message
                        cEmailString.AddHtmlString("</b><br><br>\n");
                    }
                }
            }
            else // not html
            {
                // Plain Text email.
                strEmailBody = "\n###############################################################\n";
                strEmailBody += QString("Testing date       : ") + TimeStringUTC_F(lStartTime, "d MMMM yyyy h:mm:ss") + QString("\n");
                strEmailBody += QString("Product            : ") + dbKeysContent.Get("Product").toString() + QString("\n");
                strEmailBody += QString("Lot                : ") + dbKeysContent.Get("Lot").toString() + QString("\n");
                strEmailBody += QString("SubLot             : ") + dbKeysContent.Get("SubLot").toString() + QString("\n");
                strEmailBody += QString("Wafer ID           : ") + QString(pFile->getWaferMapData().szWaferID) + QString("\n");
                /*
                if(QFile::exists(ptTask->GetProperties()->strSblFile))
                {
                    strEmailBody += QString("SBL/YBL file       : ") + ptTask->GetProperties()->strSblFile + QString("\n");
                }
                else
                */
                {
                    strEmailBody += QString("Soft Bins list     : ") + ptTask->GetProperties()->strYieldBinsList + QString("\n");
                    //if(bMultiSiteFile)
                    //	strEmailBody += QString("Yield Level        : ") + strYieldAlarmDetails;
                    //else
                    if (checkType!="BinsPerSite")
                        strEmailBody += QString("Yield Level        : ") + strYield;
                }

                if (checkType=="BinsPerSite")
                {
                    float MaxDelta=ptTask->GetAttribute("MaxDeltaBinsPerSite").toFloat();
                    strEmailBody += "MaxDelta     :" + QString("%1%").arg(MaxDelta);
                    // let s find the max %
                    int maxsite=-1; float maxpercent=-1.f;
                    foreach(int s, sites2percent.keys())
                        if (maxpercent<sites2percent.value(s))
                        {
                            maxsite=s; maxpercent=sites2percent.value(s);
                        }

                    //GSLOG(SYSLOG_SEV_NOTICE, QString("The reference will be %1 with a percent of %2").arg( maxsite, maxpercent));.arg(                    //GSLOG(SYSLOG_SEV_NOTICE, "The reference will be %1 with a percent of %2").arg( maxsite.arg( maxpercent);
                    foreach(int s, sites2percent.keys())
                    {
                        if (s==maxsite)
                        {
                            strEmailBody += QString("Site %1 : ").arg(s)
                                    + QString("%1 (Reference)").arg(sites2percent.value(s)) ;
                            continue;
                        }
                        float delta=fabs( (double) (sites2percent.value(s)-maxpercent) );
                        if (delta>MaxDelta)
                        {
                            GSLOG(SYSLOG_SEV_WARNING, QString("Alarm : site %1 too far from reference site %2").arg( s)
                                  .arg( maxsite).toLatin1().constData());
                            strEmailBody += QString("Site %1 : ").arg(s) +
                                    QString("%1 : %2 > %3").arg(sites2percent.value(s)).arg(delta).arg(MaxDelta) ;
                        }
                        else
                            strEmailBody += QString("Site %1").arg(s) +
                                    QString("%1 ").arg(sites2percent.value(s)) ;
                    }


                } // BinsPerSite

                strEmailBody += "\n";
                strEmailBody += "###############################################################\n\n";

                // If Full Report to be created
                if(ptTask->GetProperties()->iEmailReportType)
                {
                    // Check if report to be attached or remain on server and only the URL to send...
                    if(ptTask->GetProperties()->iNotificationType == 0)
                        strEmailBody += "Full report attached to this message!\n";
                    else
                    {
                        // Leave report on server, email URL only.
                        strEmailBody += "Full Report available on server at:\n  ";
                        strEmailBody += strReportName;
                        strEmailBody += "\n";
                    }
                    strEmailBody += "\n";
                }

                if(dbKeysContent.Get("TesterName").toString().isEmpty() == false)
                    strEmailBody += QString("Tester             : ") + dbKeysContent.Get("TesterName").toString() + QString("\n");
                if(dbKeysContent.Get("Operator").toString().isEmpty() == false)
                    strEmailBody += QString("Operator           : ") + dbKeysContent.Get("Operator").toString() + QString("\n");
                if(dbKeysContent.Get("ProgramName").toString().isEmpty() == false)
                    strEmailBody += QString("Program name       : ") + dbKeysContent.Get("ProgramName").toString() + QString("\n");
                if(dbKeysContent.Get("ProgramRevision").toString().isEmpty() == false)
                    strEmailBody += QString("Program revision   : ") + dbKeysContent.Get("ProgramRevision").toString() + QString("\n");
                strEmailBody += QString("Total parts tested : ") + QString::number(lTotalPartsTested) + QString("\n");
                if(lGrossDieCount > 0)
                    strEmailBody += QString("Gross Die count    : ") + QString::number(lGrossDieCount) + QString("\n");
                strEmailBody += QString("Total in bin list  : ") + QString::number(lTotalSoftBins) + QString("\n");
            }

            // Add soft Bin summary
            QString strSummary;
            itGroupsList.toFront();
            while(itGroupsList.hasNext())
            {
                strSummary += GetBinSummaryString(itGroupsList.next(), lTotalBins,
                                                  ptTask->GetProperties()->bHtmlEmail, bMultiSiteFile);
            };

            if(bHtmlEmail)
                cEmailString.AddHtmlString(strSummary);
            else
                strEmailBody += strSummary;

            // Close HTML email string if need be
            if(bHtmlEmail)
                strEmailBody = cEmailString.ClosePage();

            // We have a spooling folder: create email file in it!
            GSLOG(SYSLOG_SEV_DEBUG, QString("Generating email in '%1'").arg( ptStatusTask->GetProperties()->intranetPath()).toLatin1().constData());
            strFilePath = ptStatusTask->GetProperties()->intranetPath() + "/";
            strFilePath += GEXMO_AUTOREPORT_FOLDER + QString("/");
            strFilePath += GEXMO_AUTOREPORT_EMAILS;

            // Send email with Yield Monitoring alarm message + report.
            Email.Send(strFilePath,strFrom,strTo,strSubject,strEmailBody,ptTask->GetProperties()->bHtmlEmail,strAttachment);
        }// Alarm condition

        // Keep track of highest severity level encountered.
        lAlarmSeverity = gex_max(ptTask->GetProperties()->iExceptionLevel,lAlarmSeverity);
        onStopTask(ptTask,(bYieldAlarm?"ok:Alarm created":"ok:No alarm"));
        // Find next Yield Monitoring task for this productID...
        ptTask = GetProductYieldInfo(dbKeysContent,false,ptTask);
    };

    // launch the shell, alarm or not
    // If no yield alarms detected for this ProductID set severity to -1
    if(iTotalAlarmCount == 0)
    {
        // For PAT and YIELD the SeverityLevel is -1:PASS, 0:STANDARD, 1:CRITICAL
        lAlarmSeverity = -1;
    }

    bool b=LaunchAlarmShell(ShellYield, lAlarmSeverity, iTotalAlarmCount,
                            dbKeysContent.Get("Product").toString(),
                            dbKeysContent.Get("Lot").toString(),
                            dbKeysContent.Get("SubLot").toString(),
                            dbKeysContent.Get("Wafer").toString(),
                            dbKeysContent.Get("TesterName").toString(),
                            dbKeysContent.Get("Operator").toString(),
                            dbKeysContent.Get("FileName").toString(), "?" );
    if (!b)
        GSLOG(SYSLOG_SEV_WARNING, "Launch Alarm Shell failed");

    return "ok";	// Return error message
}
