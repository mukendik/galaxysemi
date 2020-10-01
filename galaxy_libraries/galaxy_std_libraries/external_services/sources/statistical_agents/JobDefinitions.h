#ifndef _JOB_DEFINITION_H_
#define _JOB_DEFINITION_H_

#include "DumpableAs.h"
#include "NullableOf.h"
#include "Exceptions.h"
#include <string>

namespace GS
{
namespace Gex
{

/**
 * @brief This is a part of a statistical agent's job representation, this is
 * the data part. An instance of this class is dumpable as a const std::string.
 */
class StatisticalAgentJobData :
    public DumpableAs< const std::string >
{
public :
    /**
     * @brief enumerates all the valid testing stage that can be used to
     * initialize this part of the agent's job definition
     */
    enum TestingStages
    {
        /**
         * @brief the wafer sort testing stage
         */
        WaferSort,

        /**
         * @brief the final test testing stage
         */
        FinalTest,

        /**
         * @brief the electric test testing stage
         */
        ElectricTest,

        /**
         * @brief internal value used for check
         */
        MaxTestingStages
    };

    /**
     * @brief construction
     * @param title title of the job
     * @param testing_stage testing stage used
     * @param lot the lot that is concerned
     * @param sublot the sublot that is concerned
     * @param wafer the wafer that is concerned
     * @param aggregation_level the consolidation name we want to compute
     * @param aggregation_type the consolidation name we want to compute
     * @param test_insertion_index the consolidation name we want to compute
     * @param test_insertion the consolidation name we want to compute
     * @param test_flow the consolidation name we want to compute
     * @param source_tdr source database in which pick raw results
     * @param target_adr target database in which put statistical results
     */
    StatisticalAgentJobData( const std::string &title,
                             TestingStages testing_stage,
                             const std::string &lot,
                             const std::string &sublot,
                             const NullableOf< std::string > &wafer,
                             const std::string &aggregation_level,
                             const std::string &test_flow,
                             const std::string &test_insertion,
                             const std::string& admin_db_host,
                             const int& admin_db_port,
                             const std::string& admin_db_schema,
                             const std::string& admin_db_user,
                             const std::string& admin_db_pwd,
                             const std::string& raw_source_db_host,
                             const int& raw_source_db_port,
                             const std::string& raw_source_db_schema,
                             const std::string& raw_source_db_user,
                             const std::string& raw_source_db_pwd,
                             const std::string& analytics_source_db_host,
                             const int& analytics_source_db_port,
                             const std::string& analytics_source_db_schema,
                             const std::string& analytics_source_db_user,
                             const std::string& analytics_source_db_pwd,
                             const std::string& target_db_host,
                             const int& target_db_port,
                             const std::string& target_db_schema,
                             const std::string& target_db_user,
                             const std::string& target_db_pwd) :
        m_title( title ),
        m_testing_stage
            ( ( testing_stage < MaxTestingStages ) ?
              testing_stage : throw bad_testing_stage_used() ),
        m_lot( lot ),
        m_sublot( sublot ),
        m_wafer( wafer ),
        m_aggregation_level( aggregation_level ),
        m_test_flow( test_flow ),
        m_test_insertion( test_insertion ),
        m_admin_db_host(admin_db_host),
        m_admin_db_port(admin_db_port),
        m_admin_db_schema(admin_db_schema),
        m_admin_db_user(admin_db_user),
        m_admin_db_pwd(admin_db_pwd),
        m_raw_source_db_host(raw_source_db_host),
        m_raw_source_db_port(raw_source_db_port),
        m_raw_source_db_schema(raw_source_db_schema),
        m_raw_source_db_user(raw_source_db_user),
        m_raw_source_db_pwd(raw_source_db_pwd),
        m_analytics_source_db_host(analytics_source_db_host),
        m_analytics_source_db_port(analytics_source_db_port),
        m_analytics_source_db_schema(analytics_source_db_schema),
        m_analytics_source_db_user(analytics_source_db_user),
        m_analytics_source_db_pwd(analytics_source_db_pwd),
        m_target_db_host(target_db_host),
        m_target_db_port(target_db_port),
        m_target_db_schema(target_db_schema),
        m_target_db_user(target_db_user),
        m_target_db_pwd(target_db_pwd) {}

    /**
     * @brief Need this guy to mute a compiler warning
     */
    ~StatisticalAgentJobData() {}

    /**
     * @brief basic getter
     * @return the underlying state value
     */
    const std::string & GetTitle() const { return m_title; }

    /**
     * @brief basic setter
     * @param title value to use in order to change the related inner state
     */
    void SetTitle( const std::string &title ) { m_title = title; }

    /**
     * @brief basic getter
     * @return the underlying state value
     */
    const std::string & GetTestingStage() const
      { return s_testing_phase_names[ m_testing_stage ]; }

    /**
     * @brief basic setter
     * @param title value to use in order to change the related inner state.
     * Some supplementary checks are made here, exception may be thrown.
     */
    void SetTestingStage( TestingStages testing_stage )
    {
        m_testing_stage =
            ( testing_stage < MaxTestingStages ) ?
            testing_stage : throw bad_testing_stage_used();
    }

    /**
     * @brief basic getter
     * @return the underlying state value
     */
    const std::string & GetLot() const { return m_lot; }

    /**
     * @brief basic setter
     * @param title value to use in order to change the related inner state
     */
    void SetLot( const std::string &lot ) { m_lot = lot; }

    /**
     * @brief basic getter
     * @return the underlying state value
     */
    const std::string & GetSubLot() const { return m_sublot; }

    /**
     * @brief basic setter
     * @param title value to use in order to change the related inner state
     */
    void SetSubLot( const std::string &sublot ) { m_sublot = sublot; }

    /**
     * @brief basic getter
     * @return the underlying state value
     */
    const NullableOf< std::string > & GetWafer() const { return m_wafer; }

    /**
     * @brief basic setter
     * @param title value to use in order to change the related inner state
     */
    void SetWafer( const std::string & wafer ) { m_wafer.set_data( wafer ); }

    /**
     * @brief basic getter
     * @return the underlying state value
     */
    const std::string & GetAggregationLevel() const
    { return m_aggregation_level; }

    /**
     * @brief basic setter
     * @param title value to use in order to change the related inner state
     */
    void SetAggregationLevel( const std::string & aggregation_level )
    { m_aggregation_level = aggregation_level ; }

    /**
     * @brief basic getter
     * @return the underlying state value
     */
    const std::string & GetTestFlow() const
    { return m_test_flow; }

    /**
     * @brief basic setter
     * @param title value to use in order to change the related inner state
     */
    void SetTestFlow( const std::string & test_flow )
    { m_test_flow = test_flow ; }

    /**
     * @brief basic getter
     * @return the underlying state value
     */
    const std::string & GetTestInsertion() const
    { return m_test_insertion; }

    /**
     * @brief basic setter
     * @param title value to use in order to change the related inner state
     */
    void SetTestInsertion( const std::string & test_insertion )
    { m_test_insertion = test_insertion ; }

    /**
     * @brief basic getter
     * @return the underlying state value
     */
    const std::string & GetAdminHost() const { return m_admin_db_host; }
    const int & GetAdminPort() const { return m_admin_db_port; }
    const std::string & GetAdminSchema() const { return m_admin_db_schema; }
    const std::string & GetAdminUser() const { return m_admin_db_user; }
    const std::string & GetAdminPwd() const { return m_admin_db_pwd; }

    /**
     * @brief basic getter
     * @return the underlying state value
     */
    const std::string & GetRawSourceHost() const { return m_raw_source_db_host; }
    const int & GetRawSourcePort() const { return m_raw_source_db_port; }
    const std::string & GetRawSourceSchema() const { return m_raw_source_db_schema; }
    const std::string & GetRawSourceUser() const { return m_raw_source_db_user; }
    const std::string & GetRawSourcePwd() const { return m_raw_source_db_pwd; }

    /**
     * @brief basic getter
     * @return the underlying state value
     */
    const std::string & GetAnalyticsSourceHost() const { return m_analytics_source_db_host; }
    const int & GetAnalyticsSourcePort() const { return m_analytics_source_db_port; }
    const std::string & GetAnalyticsSourceSchema() const { return m_analytics_source_db_schema; }
    const std::string & GetAnalyticsSourceUser() const { return m_analytics_source_db_user; }
    const std::string & GetAnalyticsSourcePwd() const { return m_analytics_source_db_pwd; }

    /**
     * @brief basic getter
     * @return the underlying state value
     */
    const std::string & GetTargetHost() const { return m_target_db_host; }
    const int & GetTargetPort() const { return m_target_db_port; }
    const std::string & GetTargetSchema() const { return m_target_db_schema; }
    const std::string & GetTargetUser() const { return m_target_db_user; }
    const std::string & GetTargetPwd() const { return m_target_db_pwd; }

    /**
     * @brief Defines the dump as const string contract exposed in base class
     * @return a constant string designed to represent this instance
     */
    const std::string Dump() const;

    /**
     * @brief Clone this instance, implementing the base class Clone method
     * @return a fresh new instance
     */
    StatisticalAgentJobData * Clone() const;

    /**
     * @brief Utility to evaluate a valid testing stage from a raw string
     * @param str a string containing information allowing the evaluation of a
     * valid testing stage
     * @return a valid testing stage
     */
    static TestingStages EvaluateTestingStage( const std::string &str );

private :
    /**
     * @brief title of the job
     */
    std::string m_title;

    /**
     * @brief static member representing the string representation of each
     * testing stage enumerable value
     */
    static const std::string s_testing_phase_names[ MaxTestingStages ];

    /**
     * @brief the testing stage
     */
    TestingStages m_testing_stage;

    /**
     * @brief the concerned lot
     */
    std::string m_lot;

    /**
     * @brief the concerned sublot
     */
    std::string m_sublot;

    /**
     * @brief possible identifier of a wafer if wafer sort
     */
    NullableOf< std::string > m_wafer;

    /**
     * @brief the possible aggregation level
     */
    std::string m_aggregation_level;

    /**
     * @brief the possible test flow
     */
    std::string m_test_flow;

    /**
     * @brief the possible test_insertion
     */
    std::string m_test_insertion;

    /**
     * @brief a name designating a admin db in which read tasks and write events
     */
    std::string m_admin_db_host;
    int m_admin_db_port;
    std::string m_admin_db_schema;
    std::string m_admin_db_user;
    std::string m_admin_db_pwd;

    /**
     * @brief a name designating a source TDR in which pick raw results
     */
    std::string m_raw_source_db_host;
    int m_raw_source_db_port;
    std::string m_raw_source_db_schema;
    std::string m_raw_source_db_user;
    std::string m_raw_source_db_pwd;

    /**
     * @brief a name designating a source ADR in which pick analytics results
     */
    std::string m_analytics_source_db_host;
    int m_analytics_source_db_port;
    std::string m_analytics_source_db_schema;
    std::string m_analytics_source_db_user;
    std::string m_analytics_source_db_pwd;

    /**
     * @brief a name designating a target ADR in which put statistical results
     */
    std::string m_target_db_host;
    int m_target_db_port;
    std::string m_target_db_schema;
    std::string m_target_db_user;
    std::string m_target_db_pwd;
};

/**
 * @brief This is the statistical agent's job representation.
 * An instance of this class is dumpable as a const std::string.
 */
class StatisticalAgentJob :
    public DumpableAs< const std::string >
{
public :
    /**
     * @brief valid statistical agent's type
     */
    enum StatisticTypes
    {
        /**
         * @brief agent workflow
         */
        AgentWorkflow,
        /**
         * @brief internal value used to check
         */
        MaxStatisticTypes
    };

    /**
     * @brief parametric construction
     * @param type the type of the job
     * @param data job's data
     */
    StatisticalAgentJob( StatisticTypes type,
                         const StatisticalAgentJobData &data ) :
        m_type( ( type < MaxStatisticTypes ) ?
                type : throw bad_agent_type_used() ),
        m_data( data ) {}

    /**
     * @brief Need this guy to mute a compiler warning
     */
    ~StatisticalAgentJob() {}

    /**
     * @brief basic getter
     * @return the underlying state value
     */
    const std::string & GetType() const
      { return s_statistic_type_names[ m_type ]; }

    /**
     * @brief basic setter
     * @param title value to use in order to change the related inner state.
     * A check is made and if specified value is invalid an exception may be
     * thrown
     */
    void SetType( StatisticTypes type )
    {
        m_type =
            ( type < MaxStatisticTypes ) ?
            type : throw bad_agent_type_used();
    }

    /**
     * @brief basic getter
     * @return the underlying state value
     */
    const StatisticalAgentJobData & GetData() const { return m_data; }

    /**
     * @brief basic setter
     * @param title value to use in order to change the related inner state
     */
    void SetData( const StatisticalAgentJobData &data ) { m_data = data; }

    /**
     * @brief implement the dump as const string contract exposed by the base
     * class
     * @return the constant string representation of this instance
     */
    const std::string Dump() const;

    /**
     * @brief Clone this instance. Implements base class Clone method
     * @return a fresh instance as pointer
     */
    StatisticalAgentJob * Clone() const;

private :
    /**
     * @brief static member containing a textual representation of each valid
     * job type
     */
    static const std::string s_statistic_type_names[ MaxStatisticTypes ];

    /**
     * @brief type of the job
     */
    StatisticTypes m_type;

    /**
     * @brief data of the job
     */
    StatisticalAgentJobData m_data;
};

} // namespace Gex
} // namespace GS

#endif // _JOB_DEFINITION_H_
