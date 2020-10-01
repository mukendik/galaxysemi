// gexdb_plugin_galaxy_option.cpp: implementation of the administration functions of GexDbPlugin_Galaxy class.
// ----------------------------------------------------------------------------------------------------------
// Copyright Galaxy
// This computer program is protected by copyright law
// and international treaties. Unauthorized reproduction or
// distribution of this program, or any portion of it,may
// result in severe civil and criminal penalties, and will be
// prosecuted to the maximum extent possible under the low.
// ----------------------------------------------------------------------------------------------------------


// Local includes
#include "gexdb_plugin_galaxy.h"
//#include "import_constants.h"
//#include "consolidation_tree.h"
//#include "db_datakeys.h"
//#include "database_keys_engine.h"
#include "gexdb_plugin_option.h"

// Standard includes
//#include <math.h>

// Qt includes
#include <QSqlQuery>
//#include <QSqlDriver>
#include <QSqlResult>
#include <QSqlError>
//#include <QTextEdit>
#include <QRegExp>
//#include <QDir>

// Galaxy modules includes
//#include <gqtl_sysutils.h>
//#include <gstdl_utils_c.h>
#include <gqtl_log.h>
//#include "gexdbthreadquery.h"

// macro defined in windows.h
#undef max
#undef min


////////////////////////////////////////////////////////////////////////////////////
// Global Options definition
// All Options must be defined into the enum OptionDefinition
////////////////////////////////////////////////////////////////////////////////////






//////////////////////////////////////////////////////////////////////
// Get global options info
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::GetGlobalOptionName(int nOptionNb, QString &strValue)
{
    strValue = "";
    if(!mDbGlobalOptions.contains(nOptionNb))
        return false;

    strValue = mDbGlobalOptions[nOptionNb].mName;

    return true;
}

bool GexDbPlugin_Galaxy::GetGlobalOptionValue(int nOptionNb, QString &strValue)
{
    bool bIsDefined;
    strValue = "";
    return GetGlobalOptionValue(nOptionNb, strValue, bIsDefined);
}

bool GexDbPlugin_Galaxy::GetGlobalOptionValue(QString strOptionName, QString &strValue)
{
    strValue = "";
    if (!m_pclDatabaseConnector)
        return false;

    if(!mDbGlobalOptionsMapping.contains(strOptionName))
    {
        // Internal code uses the GlobalOption enum for the Defined Global options list
        // If it is some internal Debug option, those options are not defined into the list
        // Must query directly the TDR without control
        GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
        QString           lQuery;

        GexDbPlugin_Base::ConnectToCorporateDb();

        lQuery = "SELECT option_value FROM " + NormalizeTableName("global_options",false);
        lQuery += " WHERE UPPER(option_name)='" + strOptionName.toUpper();
        lQuery += "'";
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }
        if(!clGexDbQuery.First())
            return true;

        strValue = clGexDbQuery.value(0).toString().toUpper();
        return true;
    }

    return GetGlobalOptionValue(mDbGlobalOptionsMapping[strOptionName], strValue);

}

bool GexDbPlugin_Galaxy::SetGlobalOptionValue(int nOptionNb, QString &strValue)
{
    if((nOptionNb == eDatabaseRestrictionMode)
            && (m_uiDbVersionBuild < GEXDB_PLUGIN_GALAXY_BUILD_B24))
    {
        GSET_ERROR1(GexDbPlugin_Base, eDB_CheckDbVersion, NULL, "You must update your database version to enable this Security feature");
        return false;
    }

    if(!mDbGlobalOptions.contains(nOptionNb))
    {
        GSET_ERROR1(GexDbPlugin_Base, eDB_NoResult, NULL, "Global option undefined");
        return false;
    }

    if (!m_pclDatabaseConnector)
        return false;

    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    QString     lQuery;
    QString     lOptionName = mDbGlobalOptions[nOptionNb].mName;

    GexDbPlugin_Base::ConnectToCorporateDb();

    lQuery = "DELETE FROM " + NormalizeTableName("global_options",false);
    lQuery += " WHERE UPPER(option_name)=" + TranslateStringToSqlVarChar(lOptionName.toUpper());
    if(!clGexDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    if(!mDbGlobalOptions[nOptionNb].IsValidValue(strValue))
    {
        GSET_ERROR0(GexDbPlugin_Base, eWriteSettings, NULL);
        return false;
    }

    if(mDbGlobalOptions[nOptionNb].mDefaultValue == strValue)
    {
        return true;
    }

    lQuery = "INSERT INTO " + NormalizeTableName("global_options",false);
    lQuery+= "(option_name,option_value) VALUES(";
    lQuery+= TranslateStringToSqlVarChar(lOptionName)+","+TranslateStringToSqlVarChar(strValue)+")";

    if(!clGexDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    return true;
}

bool GexDbPlugin_Galaxy::SetGlobalOptionValue(QString strOptionName, QString &strValue)
{
    if (!m_pclDatabaseConnector)
        return false;

    if(!mDbGlobalOptionsMapping.contains(strOptionName))
    {
        GSET_ERROR1(GexDbPlugin_Base, eDB_NoResult, NULL, "Global option undefined");
        return false;
    }

    return SetGlobalOptionValue(mDbGlobalOptionsMapping[strOptionName], strValue);
}

bool GexDbPlugin_Galaxy::GetGlobalOptionValue(int nOptionNb, QString &strValue, bool &bIsDefined)
{
    bIsDefined = false;
    if(!mDbGlobalOptions.contains(nOptionNb))
        return false;

    QString strOptionName = mDbGlobalOptions[nOptionNb].mName;
    strValue = mDbGlobalOptions[nOptionNb].mDefaultValue;
    bIsDefined = false;

    if (!m_pclDatabaseConnector)
        return false;

    if(mDbGlobalOptions[nOptionNb].mReadOnly)
        return true;

    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    QString           strQuery;

    GexDbPlugin_Base::ConnectToCorporateDb();

    strQuery = "SELECT option_value FROM " + NormalizeTableName("global_options",false);
    strQuery += " WHERE UPPER(option_name)='" + strOptionName.toUpper();
    strQuery += "'";
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }
    if(!clGexDbQuery.First())
        return true;

    strValue = clGexDbQuery.value(0).toString();
    if(!mDbGlobalOptions[nOptionNb].IsValidValue(strValue))
    {
        // Value is incorrect
        strValue = mDbGlobalOptions[nOptionNb].mDefaultValue;
    }

    if((nOptionNb == eDatabaseRestrictionMode)
        && (m_uiDbVersionBuild < GEXDB_PLUGIN_GALAXY_BUILD_B24))
    {
        // Option not allowed
        strValue = mDbGlobalOptions[nOptionNb].mDefaultValue;
    }

    bIsDefined = (mDbGlobalOptions[nOptionNb].mDefaultValue != strValue);
    return true;
}

bool GexDbPlugin_Galaxy::GetGlobalOptionTypeValue(int nOptionNb, QString &strValue)
{
    if(!mDbGlobalOptions.contains(nOptionNb))
        return false;

    strValue = mDbGlobalOptions[nOptionNb].mType.join(", ");

    return true;
}

bool GexDbPlugin_Galaxy::GetGlobalOptionDefaultValue(int nOptionNb, QString &strValue)
{
    if(!mDbGlobalOptions.contains(nOptionNb))
        return false;

    strValue = mDbGlobalOptions[nOptionNb].mDefaultValue;

    return true;
}

bool GexDbPlugin_Galaxy::IsGlobalOptionValidValue(int nOptionNb, QString &strValue)
{
    if(!mDbGlobalOptions.contains(nOptionNb))
        return false;

    return mDbGlobalOptions[nOptionNb].IsValidValue(strValue);
}

bool GexDbPlugin_Galaxy::GetGlobalOptionDescription(int nOptionNb, QString &strValue)
{
    if(!mDbGlobalOptions.contains(nOptionNb))
        return false;

    strValue = mDbGlobalOptions[nOptionNb].mDescription;

    return true;
}

bool GexDbPlugin_Galaxy::GetGlobalOptionReadOnly(int nOptionNb, bool &bIsReadOnly)
{
    if(!mDbGlobalOptions.contains(nOptionNb))
        return false;

    bIsReadOnly = mDbGlobalOptions[nOptionNb].mReadOnly;

    return true;
}

bool GexDbPlugin_Galaxy::LoadGlobalOptions()
{
    mDbGlobalOptions.clear();

    CreateGlobalOptions(eTestAllowDuplicateTestNumbers, GexDbPlugin_Option(
                eTestAllowDuplicateTestNumbers,
                "TEST_ALLOW_DUPLICATE_TESTNUMBERS",
                "Allow duplicate test numbers.\n" \
                "Use both test number and test name for identification to not merge tests with identical test number",
                "BOOLEAN",
                "FALSE"));
    CreateGlobalOptions(eTestRemoveSequencerName, GexDbPlugin_Option(
                eTestRemoveSequencerName,
                "TEST_REMOVE_SEQUENCER_NAME",
                "Define if SequencerName should be removed or not\n"\
                " when the SequencerName appears into the TestName\n"\
                " and matchs the GEX syntax (<> sequencer name)",
                "BOOLEAN",
                "TRUE"));
    CreateGlobalOptions(eTestRemovePinName, GexDbPlugin_Option(
                eTestRemovePinName,
                "TEST_REMOVE_PIN_NAME",
                "Define if PinName info should be removed or not\n"\
                " when the PinName appears into the TestName\n"\
                " and matchs the GEX syntax (+/- 11 or 11.a1)",
                "BOOLEAN",
                "FALSE"));
    CreateGlobalOptions(eTestAllowDynamicLimits, GexDbPlugin_Option(
                eTestAllowDynamicLimits,
                "TEST_ALLOW_DYNAMIC_LIMITS",
                "Allow dynamic limits",
                "BOOLEAN",
                "FALSE"));
    CreateGlobalOptions(eTestIgnoreDtrTestcond, GexDbPlugin_Option(
                eTestIgnoreDtrTestcond,
                "TEST_IGNORE_DTR_TEST_CONDITIONS",
                "Specify if DTR have to be ignored or not\n"\
                " when they store test conditions",
                "BOOLEAN",
                "FALSE"));
    CreateGlobalOptions(eBinningConsolidationProcess, GexDbPlugin_Option(
                eBinningConsolidationProcess,
                "BINNING_CONSOLIDATION_PROCESS",
                "Indicates if the Consolidation Process should be done\n"\
                "* TRUE: the Consolidation Process will be scheduled at specific interval per Lot/SubLot\n"\
                "* FALSE: don't process any Consolidation",
                "BOOLEAN",
                "TRUE"));
    CreateGlobalOptions(eBinningFtYieldConsolidationMissingPartsBin, GexDbPlugin_Option(
                eBinningFtYieldConsolidationMissingPartsBin,
                "BINNING_FT_YIELDCONSOLIDATION_MISSINGPARTSBIN",
                "Add a MissingParts Binning for Final Test Yield Consolidation.\n"\
                "* -1 disables this option. (Max value: 65535)",
                "NUMBER",
                "-1", 65535));
    CreateGlobalOptions(eBinningFtConsolidateSBin, GexDbPlugin_Option(
                eBinningFtConsolidateSBin,
                "BINNING_FT_CONSOLIDATE_SBIN",
                "Consolidate Soft Binnings for Final Test",
                "BOOLEAN",
                "FALSE"));
    CreateGlobalOptions(eBinningFtMapInvalidHBinWithSBin, GexDbPlugin_Option(
                eBinningFtMapInvalidHBinWithSBin,
                "BINNING_FT_MAP_INVALID_HBIN_WITH_SBIN",
                "Allow invalid HardBin for Final Test (map HardBin with SoftBin).\n"\
                "* SoftBin must be valid",
                "BOOLEAN",
                "FALSE"));
    CreateGlobalOptions(eBinningWtMapInvalidHBinWithSBin, GexDbPlugin_Option(
                eBinningWtMapInvalidHBinWithSBin,
                "BINNING_WT_MAP_INVALID_HBIN_WITH_SBIN",
                "Allow invalid HardBin for Wafer Sort (map HardBin with SoftBin).\n"\
                "* SoftBin must be valid",
                "BOOLEAN",
                "TRUE"));
    CreateGlobalOptions(eBinningFtMapInvalidSBinWithHBin, GexDbPlugin_Option(
                eBinningFtMapInvalidSBinWithHBin,
                "BINNING_FT_MAP_INVALID_SBIN_WITH_HBIN",
                "Allow invalid SoftBin for Final Test (map SoftBin with HardBin).\n"\
                "* HardBin must be valid",
                "BOOLEAN",
                "FALSE"));
    CreateGlobalOptions(eBinningWtMapInvalidSBinWithHBin, GexDbPlugin_Option(
                eBinningWtMapInvalidSBinWithHBin,
                "BINNING_WT_MAP_INVALID_SBIN_WITH_HBIN",
                "Allow invalid SoftBin for Wafer Sort (map SoftBin with HardBin).\n"\
                "* HardBin must be valid",
                "BOOLEAN",
                "FALSE"));
    CreateGlobalOptions(eBinningFtForceInvalidPartBinWith, GexDbPlugin_Option(
                eBinningFtForceInvalidPartBinWith,
                "BINNING_FT_FORCE_INVALID_PART_BIN_WITH",
                "Use the value specified when no valid HardBin and SoftBin from a Part for Final Test (force HardBin and SoftBin).\n"\
                "* HardBin and SoftBin must be invalid for the Part.\n"\
                "* -1 disabled this option",
                "NUMBER",
                "-1"));
    CreateGlobalOptions(eBinningWtForceInvalidPartBinWith, GexDbPlugin_Option(
                eBinningWtForceInvalidPartBinWith,
                "BINNING_WT_FORCE_INVALID_PART_BIN_WITH",
                "Use the value specified when no valid HardBin and SoftBin from a Part for Wafer Sort (force HardBin and SoftBin).\n"\
                "* HardBin and SoftBin must be invalid for the Part.\n"\
                "* -1 disabled this option",
                "NUMBER",
                "-1"));
    CreateGlobalOptions(eBinningFtAllowHbinSbinMismatch, GexDbPlugin_Option(
                eBinningFtAllowHbinSbinMismatch,
                "BINNING_FT_ALLOW_HBIN_SBIN_MISMATCH",
                "Allow to insert FT files where SBIN(HBIN) is not a bijection: one SBIN associated with several HBINS.\n"\
                "* Example:\n  Part Ni: HBIN=4, SBIN=11\n  Part Nj: HBIN=9, SBIN=11",
                "BOOLEAN",
                "FALSE"));
    CreateGlobalOptions(eBinningWtAllowHbinSbinMismatch, GexDbPlugin_Option(
                eBinningWtAllowHbinSbinMismatch,
                "BINNING_WT_ALLOW_HBIN_SBIN_MISMATCH",
                "Allow to insert WT files where SBIN(HBIN) is not a bijection: one SBIN associated with several HBINS.\n"\
                "* Example:\n  Part Ni: HBIN=4, SBIN=11\n  Part Nj: HBIN=9, SBIN=11",
                "BOOLEAN",
                "FALSE"));
    CreateGlobalOptions(eInsertionFtAllowEmptySubLot, GexDbPlugin_Option(
                eInsertionFtAllowEmptySubLot,
                "INSERTION_FT_ALLOW_EMPTY_SUBLOT",
                "Allow inserting Final Test files with an empty Sublot ID.\n"\
                "* For a successful test & retest consolidation, the Sublot ID will be overloaded with the Lot ID",
                "BOOLEAN",
                "FALSE"));
    CreateGlobalOptions(eInsertionAllowMissingWaferNb, GexDbPlugin_Option(
                eInsertionAllowMissingWaferNb,
                "INSERTION_ALLOW_MISSING_WAFERNB",
                "Allow missing WaferNb for insertion",
                "BOOLEAN",
                "TRUE"));
    CreateGlobalOptions(eInsertionAllowCustomStdfRecords, GexDbPlugin_Option(
                eInsertionAllowCustomStdfRecords,
                "INSERTION_ALLOW_CUSTOM_STDF_RECORDS",
                "Allow STDF records with rec_typ >= 200 as 'CUSTOM' records, and ignore",
                "BOOLEAN",
                "FALSE"));
    CreateGlobalOptions(eInsertionTimerInterval, GexDbPlugin_Option(
                eInsertionTimerInterval,
                "INSERTION_TIMER_INTERVAL",
                "Timer (in msec) emiting a log message regarding the current insertion stage, progress, pass #, number of records read...\n" \
                "A low value (100 ms) will generate a lot of logs and a high value (as 3000ms) will generated only few messages.",
                "NUMBER",
                "2000"));
    CreateGlobalOptions(eInsertionFlushSqlBufferAfter, GexDbPlugin_Option(
                eInsertionFlushSqlBufferAfter,
                "INSERTION_FLUSH_SQLBUFFER_AFTER",
                "Maximum time in seconds for preparing big SQL insertion queries.\n" \
                "If the insertion query preparation exceeds this time, the query is flushed, and another query gets prepared.",
                "NUMBER",
                "10"));
    CreateGlobalOptions(eInsertionAllowMultiProducts, GexDbPlugin_Option(
                eInsertionAllowMultiProducts,
                "INSERTION_ALLOW_MULTI_PRODUCTS_PER_LOT",
                "Allow inserting several files for one lot under different product names",
                "BOOLEAN",
                "FALSE"));
    CreateGlobalOptions(eInsertionAllowMissingStdfRecords, GexDbPlugin_Option(
                eInsertionAllowMissingStdfRecords,
                "INSERTION_ALLOW_MISSING_STDF_RECORDS",
                "Allow the insertion of some files that are not compliant with the STDF specification such that some records are missing.\n" \
                "This option will still require a minimum set of summary records to ensure data quality.\n"\
                "* PRR: Ignores last PIR if no test results"\
                "* WRR: Generates a WRR according to the WIR info"\
                "* MRR: Generates a MRR according to the MIR info",
                // Create an option with
                // empty value
                // multi selection possibility
                QStringList() << "(PRR|WRR|MRR)*",
                ""));

    ///////////////////////////////////////////////
    // SPECIFIC OPTIONS FOR MYSQL
    if(m_pclDatabaseConnector->IsMySqlDB())
    {
        CreateGlobalOptions(eMysqlSplitPartitionBy, GexDbPlugin_Option(
                    eMysqlSplitPartitionBy,
                    "MYSQL_SPLIT_PARTITION_BY",
                    "Partition granularity on Mysql database\nPartitioning tables by Month, Week, or Day.\n"\
                    "Create a new partition each new month/week/day.\n"\
                    "The new granularity take effect at the end of the granularity currently active",
                    QStringList() << "MONTH" << "WEEK" << "DAY" << "DISABLED",
                    "MONTH"));
    }

    ///////////////////////////////////////////////
    // SECURITY ACCES
    CreateGlobalOptions(eDatabaseRestrictionMode, GexDbPlugin_Option(
                eDatabaseRestrictionMode,
                "GEXDB_RESTRICTION_MODE",
                "Database Security Access.\n"\
                "Manage your database security access.\n"\
                "Use 'users-groups' management tool to define privileges",
                QStringList() << "PUBLIC" << "SECURED",
                "PUBLIC"));

    ///////////////////////////////////////////////
    // EXTERNAL SERVICES
    CreateGlobalOptions(eExternalServicesAgentWorkflowEnabled, GexDbPlugin_Option(
                eExternalServicesAgentWorkflowEnabled,
                "EXT_SERV_AGENT_WORKFLOW_ENABLED",
                "Enable posting jobs to the agent workflow",
                "BOOLEAN",
                "FALSE"));
    CreateGlobalOptions(eExternalServicesJobQueueAddress, GexDbPlugin_Option(
                eExternalServicesJobQueueAddress,
                "EXT_SERV_JOB_QUEUE_ADDRESS",
                "Address of the job queue server",
                ".*",
                "localhost"));
    CreateGlobalOptions(eExternalServicesJobQueuePort, GexDbPlugin_Option(
                eExternalServicesJobQueuePort,
                "EXT_SERV_JOB_QUEUE_PORT",
                "Port to use on the job queue server",
                "NUMBER",
                "3200"));
    CreateGlobalOptions(eExternalServicesJobQueueRoute, GexDbPlugin_Option(
                eExternalServicesJobQueueRoute,
                "EXT_SERV_JOB_QUEUE_ROUTE",
                "Path of the REST API on the server",
                ".*",
                "/"));

    ///////////////////////////////////////////////
    // CHANGE PROPERTIES FOR CHARAC AND MANUAL DATABASES
    // If YM database no default global options
    // If Char or manual Prod database
    if(IsCharacTdr() || IsManualProdTdr())
    {
        foreach(const int &lKey, mDbGlobalOptions.keys())
        {
            // Move all options for the insertion management (test, binning, insertion) to ReadWrite
            // Move all options for the TDR managemen to ReadOnly
            if(mDbGlobalOptions[lKey].mName.startsWith("BINNING")
                    || mDbGlobalOptions[lKey].mName.startsWith("TEST")
                    || mDbGlobalOptions[lKey].mName.startsWith("INSERTION"))
                mDbGlobalOptions[lKey].mReadOnly = false;
            else
                mDbGlobalOptions[lKey].mReadOnly = true;

            // For Charac, disabled the consolidation
            if(mDbGlobalOptions[lKey].mName.contains("CONSOLIDATION")
                    || mDbGlobalOptions[lKey].mName.contains("CONSOLIDATE"))
                mDbGlobalOptions[lKey].mReadOnly = IsCharacTdr();

            // And turn off all options per default
            if(mDbGlobalOptions[lKey].mType.first() == "BOOLEAN")
                mDbGlobalOptions[lKey].mDefaultValue = "FALSE";
        }
    }

    // Check if have InnoDb option
    QString strEngine,strFormat;
    if(GetStorageEngineName(strEngine,strFormat) && (strEngine.toUpper() == "SPIDER"))
    {
        m_pclDatabaseConnector->m_bTransactionDb = true;
        m_pclDatabaseConnector->m_bPartitioningDb = true;

        // Special case for SPIDER
        // If the database is SPIDER, partition management must be done externally
        // Force partition_management to disable and read only
        mDbGlobalOptions[eMysqlSplitPartitionBy].mDefaultValue = "DISABLED";
        mDbGlobalOptions[eMysqlSplitPartitionBy].mReadOnly = true;
        mDbGlobalOptions[eMysqlSplitPartitionBy].mDescription +=
                ".\n This option is disable for Spider Monitoring";

    }

    return true;
}

bool GexDbPlugin_Galaxy::CreateGlobalOptions(int optionNumber, GexDbPlugin_Option optionDef)
{
    mDbGlobalOptions[optionNumber] = optionDef;
    mDbGlobalOptionsMapping[optionDef.mName] = optionNumber;

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_Option
/////////////////////////////////////////////////////////////////////////////////////////
// Check the validity of the value according to the option definition
// Then reformat the value if needed
//     - remove space
//     - upper case
//     - boolean value
/////////////////////////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Option::IsValidValue(QString &value)
{
    QString     lValue = value.toUpper().simplified().remove(" ");

    QStringList lTypes = mType;
    QString lType;
    QString lRegExp;
    QMap<QString, QString> lAutoCompletion;
    int lAutoCompletionNbChar = 1;
    if(lTypes.count()>1)
    {
        while(true)
        {
            // Check if autocompletion is possible
            if(lAutoCompletionNbChar > 4)
                break;
            // For each type, keep the first char
            foreach(const QString &type, lTypes)
                lAutoCompletion[type.left(lAutoCompletionNbChar)] = type;
            // If have the same count, OK
            if(lAutoCompletion.count() == lTypes.count())
                break;
            // Else try with more char
            lAutoCompletion.clear();
            lAutoCompletionNbChar++;
        }
    }


    if(lValue == "NULL")
    {
        lValue = "";
    }

    while(!lTypes.isEmpty())
    {
        lType = lTypes.takeFirst();
        // Translate the type to the correct RegExp
        if(lType.contains("|"))
        {
            // Multi Selection (PRR|WRR|MRR)
            // ends with * = empty value allowed
            // ends with + = no empty value
            // 'PRR' = valid
            // 'PRR|MRR' = valid
            // RegExp = (PRR|WRR|MRR)(\\|(PRR|WRR|MRR))*
            //          (1 from this) sep by '|' (another from this) ...

            // Check if the value can be empty
            if(lType.endsWith("*"))
            {
                if(lValue.isEmpty())
                {
                    // Reformat the value if needed
                    value = lValue;
                    return true;
                }
                // remove the *
                lType = lType.left(lType.length()-1);
            }

            if(!lType.startsWith("("))
                lType = "("+lType+")";
            lType = lType + "(\\|"+lType+")*";

            // Reformat the value if needed
            lValue = lValue.replace(",","|");
            QString     lTemp;
            QStringList lMultiValue = lValue.split("|");
            lValue = "";
            while(!lMultiValue.isEmpty())
            {
                lTemp = lMultiValue.takeFirst();
                if(lMultiValue.contains(lTemp))
                    continue;
                if(!lValue.isEmpty())
                    lValue += "|";
                lValue += lTemp;
            }
        }
        else if(lType == "NUMBER")
        {
            // as a number
            lType = "-1|\\d+";
        }
        else if(lType == "BOOLEAN")
        {
            // as a bool
            lType = "(TRUE|FALSE)";

            lAutoCompletion["1"] = "TRUE";
            lAutoCompletion["Y"] = "TRUE";
            lAutoCompletion["T"] = "TRUE";
            lAutoCompletion["0"] = "FALSE";
            lAutoCompletion["N"] = "FALSE";
            lAutoCompletion["F"] = "FALSE";
            lAutoCompletionNbChar = 1;
        }

        if(!lAutoCompletion.isEmpty())
        {
            // Reformat the value if needed
            if(lAutoCompletion.contains(lValue.left(lAutoCompletionNbChar)))
                lValue = lAutoCompletion[lValue.left(lAutoCompletionNbChar)];
        }

        // one of those possibilities (sep by '|')
        if(!lRegExp.isEmpty())
        {
            lRegExp += "|";
        }
        lRegExp += lType;
    }
    QRegExp clRegExp("("+lRegExp+")",Qt::CaseInsensitive);

    // Check each values
    if(!clRegExp.exactMatch(lValue) || (lValue.size() >= 255))
    {
        // Value is incorrect
        return false;
    }

    if(mMaxValue > 0)
    {
        bool convertion;
        int lInputValue = lValue.toInt(&convertion);
        // check if no overflow
        if(convertion)
        {
            if(lInputValue > 0)
            {
                lValue = QString::number(std::min(mMaxValue, lValue.toInt()));
            }
        }
        else
        {
             lValue = QString::number(mMaxValue);
        }
    }

    // Reformat the value if needed
    if((mType.size() != 1) || (mType[0] != ".*"))
    {
        value = lValue;
    }
    return true;
}
