#ifndef GEXDB_PLUGIN_OPTION_H
#define GEXDB_PLUGIN_OPTION_H

#include <QString>


////////////////////////////////////////////////////////////////////////////////////
// Global Options definition
// All Options must be defined into the enum OptionDefinition
////////////////////////////////////////////////////////////////////////////////////

// Options Definitions
enum OptionDefinition
{
    eBinningFtYieldConsolidationMissingPartsBin = 0,
    eBinningFtConsolidateSBin,
    eBinningFtMapInvalidHBinWithSBin,
    eBinningWtMapInvalidHBinWithSBin,
    eBinningFtMapInvalidSBinWithHBin,
    eBinningWtMapInvalidSBinWithHBin,
    eBinningFtForceInvalidPartBinWith,
    eBinningWtForceInvalidPartBinWith,
    eBinningFtAllowHbinSbinMismatch,
    eBinningWtAllowHbinSbinMismatch,
    eBinningConsolidationProcess,
    eInsertionFtAllowEmptySubLot,
    eInsertionAllowMultiProducts,
    eInsertionAllowMissingWaferNb,
    eInsertionAllowMissingStdfRecords,
    eInsertionAllowCustomStdfRecords,
    eInsertionTimerInterval,
    eInsertionFlushSqlBufferAfter,
    eTestAllowDynamicLimits,
    eTestRemoveSequencerName,
    eTestRemovePinName,
    eTestAllowDuplicateTestNumbers,
    eTestIgnoreDtrTestcond,
    eOracleIndexInsertMode,
    eOracleFtInitialExtentSize,
    eOracleFtNextExtentSize,
    eOracleFtMaxDatafileSize,
    eOracleWtInitialExtentSize,
    eOracleWtNextExtentSize,
    eOracleWtMaxDatafileSize,
    eOracleEtInitialExtentSize,
    eOracleEtNextExtentSize,
    eOracleEtMaxDatafileSize,
    eOracleSplitPartitionBy,
    eMysqlSplitPartitionBy,
    eDatabaseRestrictionMode,
    eExternalServicesAgentWorkflowEnabled,
    eExternalServicesJobQueueAddress,
    eExternalServicesJobQueuePort,
    eExternalServicesJobQueueRoute,
    eMaxOptions     // For Options loop
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_Option
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_Option
{
public:
    GexDbPlugin_Option()
    {
        mNumber=-1;
        mMaxValue=0;
        mReadOnly = false;
    }
    GexDbPlugin_Option(int Number,QString Name, QString Description, QString Type, QString DefaultValue, int maxValue = 0)
    {
        mNumber=Number;
        mName = Name;
        mDescription = Description;
        mType = QStringList(Type);
        mDefaultValue = DefaultValue;
        mReadOnly = false;
        mMaxValue = maxValue;
    }
    GexDbPlugin_Option(int Number,QString Name, QString Description, QStringList Type, QString DefaultValue, int maxValue = 0)
    {
        mNumber=Number;
        mName = Name;
        mDescription = Description;
        mType = Type;
        mDefaultValue = DefaultValue;
        mReadOnly = false;
        mMaxValue = maxValue;
    }
    ~GexDbPlugin_Option(){}

    int                     mMaxValue;

    // Option number
    int                     mNumber;
    // option_name stored into gexdb.global_options
    QString                 mName;
    // Option description
    QString                 mDescription;
    // Option type indicates the allowed values
    // can be:
    // BOOLEAN
    // NUMBER
    // SET OF SINGLE VALUES
    // -> one elt (as a KEY) per ListElement
    // ie: 'MONTH' << 'WEEK' << 'DAY'
    // SET OF MULTI VALUES
    // -> multi elt (as a RegExp) per ListElement
    // ie: 'DISABLE' << '(PRR|WRR|MRR)'
    QStringList             mType;

    // Other Properties
    // can be updated for specific Tdr

    // Default value
    QString                 mDefaultValue;
    // ReadOnly
    bool                    mReadOnly;

    // Check the validity of the value according to the option definition
    // BOOLEAN = TRUE or FALSE
    // NUMBER
    // eltA , eltB = eltA or eltB
    // eltA|eltB as RegExp for MultiSelect = eltA or eltB or eltA|eltB or eltA|eltA
    bool                    IsValidValue(QString &value);
};


#endif // GEXDB_PLUGIN_OPTION_H
