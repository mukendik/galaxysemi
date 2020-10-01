#include <qmessagebox.h>
#include <qfiledialog.h>
#include <QDate>

// Galaxy modules includes
#include <gqtl_sysutils.h>
#include <gqtl_log.h>

#include "yield_taskdata.h"
#include "browser_dialog.h"
#include "report_build.h"
#include "browser_dialog.h"
#include "cbinning.h"
#include "engine.h"
#include "pickproduct_id_dialog.h"
#include "scheduler_engine.h"
#include "status/status_taskdata.h"
#include "mo_task.h"
#include "mo_email.h"
#include "autoadmin/autoadmin_taskdata.h"
#include "gqtl_datakeys.h"
#include "db_transactions.h"
#include "gex_database_entry.h"
#include "tb_toolbox.h"
#include "report_options.h"
#include "db_engine.h"
#include "gex_report.h"
#include "gex_shared.h"
#include "gex_database_filter.h"
#include "product_info.h"
#include "message.h"

// main.cpp
extern GexMainwindow *	pGexMainWindow;
// report_build.cpp
extern CGexReport*		gexReport;			// Handle to report class
extern CGexSkin*		pGexSkin;			// holds the skin settings
extern CReportOptions	ReportOptions;		// Holds options

const QStringList GexMoYieldMonitoringTaskData::sCheckTypes=QStringList()
        << "Unknow" << "FixedYieldTreshold" << "BinsPerSite" << "SYASBL_RDB";

// Yield-Monitoring structure constructor
GexMoYieldMonitoringTaskData::GexMoYieldMonitoringTaskData(QObject* parent): TaskProperties(parent)
{
    clear();
}

GexMoYieldMonitoringTaskData &GexMoYieldMonitoringTaskData::operator=(const GexMoYieldMonitoringTaskData &copy)
{
    if(this != &copy)
    {
        strTitle                = copy.strTitle;
        strDatabase             = copy.strDatabase;
        strTestingStage         = copy.strTestingStage;
        strProductID            = copy.strProductID;
        strYieldBinsList        = copy.strYieldBinsList;
        iBiningType             = copy.iBiningType;
        iAlarmLevel             = copy.iAlarmLevel;
        iAlarmIfOverLimit       = copy.iAlarmIfOverLimit;
        lMinimumyieldParts      = copy.lMinimumyieldParts;
        strSblFile              = copy.strSblFile;
        strEmailFrom            = copy.strEmailFrom;
        strEmailNotify          = copy.strEmailNotify;
        bHtmlEmail              = copy.bHtmlEmail;
        iEmailReportType        = copy.iEmailReportType;
        iNotificationType       = copy.iNotificationType;
        iExceptionLevel         = copy.iExceptionLevel;

        bSYA_active_on_datafile_insertion   = copy.bSYA_active_on_datafile_insertion;
        bSYA_active_on_trigger_file         = copy.bSYA_active_on_trigger_file;
        eSYA_Rule               = copy.eSYA_Rule;
        strSYA_Rule             = copy.strSYA_Rule;
        fSYA_N1_value           = copy.fSYA_N1_value;
        fSYA_N2_value           = copy.fSYA_N2_value;
        iSYA_LotsRequired       = copy.iSYA_LotsRequired;
        iSYA_Period             = copy.iSYA_Period;
        strSYA_SBL1_LL_Disabled = copy.strSYA_SBL1_LL_Disabled;
        strSYA_SBL1_HL_Disabled = copy.strSYA_SBL1_HL_Disabled;
        strSYA_SBL2_LL_Disabled = copy.strSYA_SBL2_LL_Disabled;
        strSYA_SBL2_HL_Disabled = copy.strSYA_SBL2_HL_Disabled;
        bSYA_SYL1_LL_Disabled   = copy.bSYA_SYL1_LL_Disabled;
        bSYA_SYL1_HL_Disabled   = copy.bSYA_SYL1_HL_Disabled;
        bSYA_SYL2_LL_Disabled   = copy.bSYA_SYL2_LL_Disabled;
        bSYA_SYL2_HL_Disabled   = copy.bSYA_SYL2_HL_Disabled;
        bSYA_IgnoreDataPointsWithNullSigma = copy.bSYA_IgnoreDataPointsWithNullSigma;
        bSYA_IgnoreOutliers     = copy.bSYA_IgnoreOutliers;
        bSYA_UseGrossDie        = copy.bSYA_UseGrossDie;
        iSYA_MinDataPoints      = copy.iSYA_MinDataPoints;
        mapBins_rules           = copy.mapBins_rules;

        TaskProperties::operator =(copy);
        UpdatePrivateAttributes();
    }
    return *this;
}

// Yield-Monitoring Rule: reset variables
void GexMoYieldMonitoringTaskData::clear(void)
{
    ResetAllAttributes();

    bSYA_active_on_trigger_file         =true;
    bSYA_active_on_datafile_insertion   =true;

    // Default 'From' email-address
    if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
        strEmailFrom = GEX_EMAIL_PAT_MAN;
    else
        strEmailFrom = GEX_EMAIL_YIELD_MAN;

    strTitle                = "";                   // Task title.
    strDatabase             ="";                    // Database to focus on
    strTestingStage         ="";                    // Testing stage tables to focus on
    strProductID            = "*";                  // ProductID to look for

    strYieldBinsList        = "1";                  // List of bins to monitor
    iBiningType             =0;                     // Binning type: 0=Good bins, 1=Failing bins
    iAlarmLevel             = 100;                  // Ranges in 0-100
    iAlarmIfOverLimit       =0;                     // 0= alarm if Yield Under Limit, 1= alarm if Yield Over limit
    lMinimumyieldParts      = 50;                   // Minimum parts required to check the yield.
    strSblFile              ="";                    // Full path to SBL/YBL data file (for Statistical Bin Limits monitoring)
    strEmailNotify          = "";                   // Default 'From' email-address
    bHtmlEmail              = true;                 // 'true' if email to be sent in HTML format
    iEmailReportType        = GEXMO_YIELD_EMAILREPORT_TEXT; // GEXMO_YIELD_EMAILREPORT_xxx  :ASCII text in BODY, CSV, Word,  PPT, PDF
    iNotificationType       = 0;                            // 0= Report attached to email, 1= keep on server, only email its URL
    iExceptionLevel         = 0;                    // 0=Standard, 1=Critical...


    eSYA_Rule               = eNone;                // Rule: None means SYL/SBL disabled, only Fixed limit defined
    strSYA_Rule             = "";                   // Name of the rule (for recording in GEXDB)
    fSYA_N1_value           = 6;                    // N1 parameter
    fSYA_N2_value           = 6;                    // N2 parameter
    iSYA_LotsRequired       = 6;                    // Minimum Total lots required for computing new SYL-SBL
    iSYA_Period             = GEXMO_YIELD_SYA_PERIOD_6M;    // Period for reprocessing SYL/SBL: 0=1week,1=1Month,2...
    strSYA_SBL1_LL_Disabled = "n/a";                // List of Binnings for which the SBL1 LL should be disabled
    strSYA_SBL1_HL_Disabled = "n/a";                // List of Binnings for which the SBL1 HL should be disabled
    strSYA_SBL2_LL_Disabled = "n/a";                // List of Binnings for which the SBL2 LL should be disabled
    strSYA_SBL2_HL_Disabled = "n/a";                // List of Binnings for which the SBL2 HL should be disabled
    bSYA_SYL1_LL_Disabled   = false;                // True if SYL1 LL should be disabled
    bSYA_SYL1_HL_Disabled   = true;                 // True if SYL1 HL should be disabled
    bSYA_SYL2_LL_Disabled   = false;                // True if SYL2 LL should be disabled
    bSYA_SYL2_HL_Disabled   = true;                 // True if SYL2 HL should be disabled
    bSYA_IgnoreDataPointsWithNullSigma = true;      // Set to true if datapoints with null sigma should be ignored
    bSYA_IgnoreOutliers     = true;                 // Set to true if outliers shoud be ignored
    bSYA_UseGrossDie        = true;                 // Set to true if Gross Die should be used
    iSYA_MinDataPoints      = 30;                   // Minimum datapoints (wafers, lots if FT) to compute SYL/SBL
    mapBins_rules.clear();                          //

    SetAttribute("CheckType", QVariant("Unknown") );
    // SYL-SBL (SQL RDB only)
    SetAttribute("ExpirationDate",QDate::currentDate());
    SetAttribute("ExpirationWarningReprieve", "7"); // Respite=rpit(pause) reprieve=sursit delay period duration

    UpdatePrivateAttributes();
}

void GexMoYieldMonitoringTaskData::UpdatePrivateAttributes()
{
    ResetPrivateAttributes();

    // Insert new values
    SetPrivateAttribute("Title",strTitle);
    SetPrivateAttribute("ProductID",strProductID);
    SetPrivateAttribute("YieldBins",strYieldBinsList);
    SetPrivateAttribute("BiningType",iBiningType);
    SetPrivateAttribute("AlarmLevel",iAlarmLevel);
    SetPrivateAttribute("AlarmDirection",iAlarmIfOverLimit);
    SetPrivateAttribute("MinimumParts",qlonglong(lMinimumyieldParts));
    SetPrivateAttribute("EmailFrom",strEmailFrom);
    SetPrivateAttribute("Emails",strEmailNotify);
    SetPrivateAttribute("EmailFormat",bHtmlEmail ? "HTML" : "TEXT");
    SetPrivateAttribute("EmailReportType",iEmailReportType);
    SetPrivateAttribute("NotificationType",iNotificationType);
    SetPrivateAttribute("ExceptionLevel",iExceptionLevel);
    SetPrivateAttribute("ActiveOnDatafileInsertion",bSYA_active_on_datafile_insertion ? "1" : "0");
    SetPrivateAttribute("ActiveOnTriggerFile",bSYA_active_on_trigger_file ? "1" : "0");

    QMapIterator<int, QMap<QString, QVariant> > itBinRule(mapBins_rules);
    QString strValues;
    QStringList lRules;
    while (itBinRule.hasNext())
    {
        itBinRule.next();
        int nBinNo = itBinRule.key();
        int nRuleType = itBinRule.value()["RuleType"].toInt();
        if(nRuleType == eManual)
        {
            // Save the rule limit
            strValues= itBinRule.value()["LL1"].toString()+"|";
            strValues+=itBinRule.value()["HL1"].toString()+"|";
            strValues+=itBinRule.value()["LL2"].toString()+"|";
            strValues+=itBinRule.value()["HL2"].toString();
        }
        else
        {
            strValues = itBinRule.value()["N1"].toString()+"|";
            strValues+= itBinRule.value()["N2"].toString();
        }
        lRules << QString("%1|%2|%3").arg(QString::number(nBinNo),QString::number(nRuleType),strValues);
    }
    SetPrivateAttribute("BinRule",lRules.join(";"));
    SetPrivateAttribute("Database",strDatabase);
    SetPrivateAttribute("TestingStage",strTestingStage);
    SetPrivateAttribute("RuleType",eSYA_Rule);
    SetPrivateAttribute("RuleTypeString",strSYA_Rule);
    SetPrivateAttribute("N1_Parameter",fSYA_N1_value);
    SetPrivateAttribute("N2_Parameter",fSYA_N2_value);
    SetPrivateAttribute("MinimumLotsRequired",iSYA_LotsRequired);
    SetPrivateAttribute("ValidityPeriod",iSYA_Period);
    SetPrivateAttribute("SBL1_LL_Disabled",strSYA_SBL1_LL_Disabled);
    SetPrivateAttribute("SBL1_HL_Disabled",strSYA_SBL1_HL_Disabled);
    SetPrivateAttribute("SBL2_LL_Disabled",strSYA_SBL2_LL_Disabled);
    SetPrivateAttribute("SBL2_HL_Disabled",strSYA_SBL2_HL_Disabled);
    SetPrivateAttribute("SYL1_LL_Disabled",bSYA_SYL1_LL_Disabled ? "1" : "0");
    SetPrivateAttribute("SYL1_HL_Disabled",bSYA_SYL1_HL_Disabled ? "1" : "0");
    SetPrivateAttribute("SYL2_LL_Disabled",bSYA_SYL2_LL_Disabled ? "1" : "0");
    SetPrivateAttribute("SYL2_HL_Disabled",bSYA_SYL2_HL_Disabled ? "1" : "0");
    SetPrivateAttribute("IgnoreDataPointsWithNullSigma",bSYA_IgnoreDataPointsWithNullSigma ? "1" : "0");
    SetPrivateAttribute("IgnoreOutliers",bSYA_IgnoreOutliers ? "1" : "0");
    SetPrivateAttribute("UseGrossDie",bSYA_UseGrossDie ? "1" : "0");
    SetPrivateAttribute("MinDataPoints",iSYA_MinDataPoints);
}

bool GS::Gex::SchedulerEngine::CheckSblYield(QString &strSblFile,
                                             GS::QtLib::Range *pBinCheck,
                                             CBinning *ptBinList,
                                             QString &strYieldAlarmMessage,
                                             QString &strErrorMessage)
{
    QString strString;
    QString strParameter;

    // Read SBL data file
    QFile file(strSblFile);
    if(file.open(QIODevice::ReadOnly) == false)
        return true;	// Error: Failed opening SBL file.

    // Read Tasks definition File
    QTextStream hSlbFile(&file);	// Assign file handle to data stream

    // SBL file format:
    // # Comment line
    // <SoftBin>,<Low_SBL>,<High_SBL>
    bool	bSeekingHeader=true;	// Set to false as soon as header <sbl> line is found
    bool	bFlag;
    int		iSoftBin;
    double	lfLowLimit,lfHighLimit;
    int		iLine=1;
    do
    {
        // Read one line from file
        strString = hSlbFile.readLine();

        if(strString.startsWith("#") || strString.isEmpty())
            goto next_line;


        // Check if valid header...or empty!
        if(bSeekingHeader)
        {
            if(strString != "<sbl>")
            {
                strErrorMessage = "  > *ERROR* SBL file not starting with '<sbl>' header line\n  > " + strSblFile;
                return true;	// Error
            }

            // Header found, we can now parse the rest of the file
            bSeekingHeader=false;
            goto next_line;
        }

        // End of file marker
        if(strString.startsWith("</sbl>") == true)
            break;

        // Extract SoftBin# and SBL limits
        strParameter = strString.section(',',0,0).trimmed();	// SoftBin
        iSoftBin = strParameter.toInt(&bFlag);
        if(bFlag == false)
        {
            strErrorMessage = "  > *ERROR* SBL file error reading SoftBin: Line #" +  QString::number(iLine) + "\n  > " + strSblFile;
            return true;	// Error
        }
        strParameter = strString.section(',',1,1).trimmed();	// SBL LowLimit
        if(strParameter.isEmpty())
            lfLowLimit = 0;	// No low limit define!
        else
        {
            lfLowLimit = strParameter.toDouble(&bFlag);
            if(bFlag == false)
            {
                strErrorMessage = "  > *ERROR* SBL file error reading Low Limit: Line #" +  QString::number(iLine) + "\n  > " + strSblFile;
                return true;	// Error
            }
        }
        strParameter = strString.section(',',2,2).trimmed();	// SBL HighLimit
        if(strParameter.isEmpty())
            lfHighLimit = 100;	// No High limit define!
        else
        {
            lfHighLimit = strParameter.toDouble(&bFlag);
            if(bFlag == false)
            {
                strErrorMessage = "  > *ERROR* SBL file error reading High Limit: Line #" +  QString::number(iLine) + "\n  > " + strSblFile;
                return true;	// Error
            }
        }

        // Check if this SBL bin belongs to the list of binning to monitor!
        if(pBinCheck->Contains(iSoftBin) == true)
        {
            // Compute total of Soft Bins matching our criteria...
            CBinning *ptBinCell = ptBinList;
            long	lTotalSoftBins = 0;
            long	lTotalBins = 0;
            double	lfYieldRate;
            while(ptBinCell != NULL)
            {
                // Get the total parts falling in the bin.
                if(iSoftBin == ptBinCell->iBinValue)
                    lTotalSoftBins = ptBinCell->ldTotalCount;
                lTotalBins += ptBinCell->ldTotalCount;

                ptBinCell = ptBinCell->ptNextBin;
            };

            // Compute Yield level for this SBL bin
            if(lTotalBins)
                lfYieldRate = 100.0 * ((double) lTotalSoftBins) / (double) lTotalBins;
            else
                lfYieldRate = 0;


            // Check if the yield is within the limits defined!
            if((lfYieldRate < lfLowLimit) || (lfYieldRate > lfHighLimit))
            {
                // Build SBL Yield alarm error
                strYieldAlarmMessage = "Bin " + QString::number(iSoftBin) + ": ";
                strYieldAlarmMessage += QString::number(lfYieldRate,'f',2) + QString("% ");
                strYieldAlarmMessage += " (SBL limits: ";
                strYieldAlarmMessage += "LL=" + QString::number(lfLowLimit,'f',2) + QString("% ");
                strYieldAlarmMessage += ", HL=" + QString::number(lfHighLimit,'f',2) + QString("% )");

                return true;	// ALARM: Yield level outside of SBL limits!
            }
        }

next_line:
        iLine++;	// Keep track of line count
    }
    while(hSlbFile.atEnd() == false);
    file.close();

    // None of the bins under monitoring failed the SBL limits!
    return false;	// No alarm.
}
