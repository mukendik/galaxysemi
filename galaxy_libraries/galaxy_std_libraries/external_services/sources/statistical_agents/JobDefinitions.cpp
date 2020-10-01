#include "JobDefinitions.h"
#include "case_insensitive_char_traits.h"

#include <sstream>

const std::string
    GS::Gex::StatisticalAgentJobData::s_testing_phase_names
    [ GS::Gex::StatisticalAgentJobData::MaxTestingStages ] =
    { "Wafer Sort", "Final Test", "Electric Test" };

const std::string
    GS::Gex::StatisticalAgentJob::s_statistic_type_names
    [ GS::Gex::StatisticalAgentJob::MaxStatisticTypes ] =
    { "AgentWorkflow" };

const std::string GS::Gex::StatisticalAgentJob::Dump() const
{
    std::stringstream ss;

    // create a stringified JSON representation of this instance
    ss << '{'
       << "\"type\" : " << '"' << GetType() << '"' << ','
       << "\"data\" : " << GetData().Dump()
       << '}';

    return ss.str();
}

GS::Gex::StatisticalAgentJob * GS::Gex::StatisticalAgentJob::Clone() const
{
    return new StatisticalAgentJob( *this );
}

const std::string GS::Gex::StatisticalAgentJobData::Dump() const
{
    std::stringstream ss;

    // create a stringified JSON representation of this instance
    ss << '{'
       << "\"title\" : " << '"' << GetTitle() << '"';

    ss << ','
       << "\"testingStage\" : " << '"' << GetTestingStage() << '"' ;

    ss << ','
       << "\"lot\" : " << '"' << GetLot() << '"' ;

    ss << ','
       << "\"sublot\" : " << '"' << GetSubLot() << '"' ;

    ss << ','
       << "\"wafer\" : " ;
    if( GetWafer().is_null() )
        ss << "null";
    else
        ss << '"' << GetWafer().get_data() << '"';

    ss << ','
       << "\"aggregationLevel\" : " << '"' << GetAggregationLevel() << '"';

    ss << ','
       << "\"testFlow\" : " << '"' << GetTestFlow() << '"';

    ss << ','
       << "\"testInsertion\" : " << '"' << GetTestInsertion() << '"';

    ss << ','
       << "\"adminHost\" : " << '"' << GetAdminHost() << '"';

    ss << ','
       << "\"adminPort\" : " << '"' << GetAdminPort() << '"';

    ss << ','
       << "\"adminSchema\" : " << '"' << GetAdminSchema() << '"';

    ss << ','
       << "\"adminUser\" : " << '"' << GetAdminUser() << '"';

    ss << ','
       << "\"adminPwd\" : " << '"' << GetAdminPwd() << '"';

    ss << ','
       << "\"rawSourceHost\" : " << '"' << GetRawSourceHost() << '"';

    ss << ','
       << "\"rawSourcePort\" : " << '"' << GetRawSourcePort() << '"';

    ss << ','
       << "\"rawSourceSchema\" : " << '"' << GetRawSourceSchema() << '"';

    ss << ','
       << "\"rawSourceUser\" : " << '"' << GetRawSourceUser() << '"';

    ss << ','
       << "\"rawSourcePwd\" : " << '"' << GetRawSourcePwd() << '"';

    ss << ','
       << "\"analyticsSourceHost\" : " << '"' << GetAnalyticsSourceHost() << '"';

    ss << ','
       << "\"analyticsSourcePort\" : " << '"' << GetAnalyticsSourcePort() << '"';

    ss << ','
       << "\"analyticsSourceSchema\" : " << '"' << GetAnalyticsSourceSchema() << '"';

    ss << ','
       << "\"analyticsSourceUser\" : " << '"' << GetAnalyticsSourceUser() << '"';

    ss << ','
       << "\"analyticsSourcePwd\" : " << '"' << GetAnalyticsSourcePwd() << '"';

    ss << ','
       << "\"targetHost\" : " << '"' << GetTargetHost() << '"';

    ss << ','
       << "\"targetPort\" : " << '"' << GetTargetPort() << '"';

    ss << ','
       << "\"targetSchema\" : " << '"' << GetTargetSchema() << '"';

    ss << ','
       << "\"targetUser\" : " << '"' << GetTargetUser() << '"';

    ss << ','
       << "\"targetPwd\" : " << '"' << GetTargetPwd() << '"'
       << '}';

    return ss.str();
}

GS::Gex::StatisticalAgentJobData * GS::Gex::StatisticalAgentJobData::Clone() const
{
    return new StatisticalAgentJobData( *this );
}

GS::Gex::StatisticalAgentJobData::TestingStages
    GS::Gex::StatisticalAgentJobData::EvaluateTestingStage
    ( const std::string &str )
{
    // a valid way to determine the testing stage is to split the input string
    // by each word and isolate wafer, sort, final, electric and test words,
    // case insensitive

    // alias on case insensitive string
    typedef std::basic_string< char, Qx::CaseInsensitiveCharTraits > ci_string;

    // keywords to find
    const ci_string final( "final" );
    const ci_string electric( "electric" );
    const ci_string e_( "e-" );
    const ci_string test ( "test" );
    const ci_string wafer( "wafer" );
    const ci_string sort( "sort" );

    // ci input string
    ci_string ci_str( str.c_str() );

    // wafer sort found
    bool wafer_sort_found =
        ( ci_str.find( wafer )    != ci_string::npos ) &&
        ( ci_str.find( sort )     != ci_string::npos ) &&
        ( ci_str.find( e_ )       == ci_string::npos ) &&
        ( ci_str.find( electric ) == ci_string::npos ) &&
        ( ci_str.find( final )    == ci_string::npos ) &&
        ( ci_str.find( test )     == ci_string::npos );
    if( wafer_sort_found ) return WaferSort;

    // electric test found ("electric test")
    bool electric_test_found =
        ( ci_str.find( electric ) != ci_string::npos ) &&
        ( ci_str.find( test )     != ci_string::npos ) &&
        ( ci_str.find( e_ )       == ci_string::npos ) &&
        ( ci_str.find( wafer )    == ci_string::npos ) &&
        ( ci_str.find( sort )     == ci_string::npos ) &&
        ( ci_str.find( final )    == ci_string::npos );

    if( electric_test_found ) return ElectricTest;

    // electric test found ("e-test")
    electric_test_found =
        ( ci_str.find( e_ )       != ci_string::npos ) &&
        ( ci_str.find( test )     != ci_string::npos ) &&
        ( ci_str.find( electric ) == ci_string::npos ) &&
        ( ci_str.find( wafer )    == ci_string::npos ) &&
        ( ci_str.find( sort )     == ci_string::npos ) &&
        ( ci_str.find( final )    == ci_string::npos );

    if( electric_test_found ) return ElectricTest;

    // final test found
    bool final_test_found =
        ( ci_str.find( final )    != ci_string::npos ) &&
        ( ci_str.find( test )     != ci_string::npos ) &&
        ( ci_str.find( e_ )       == ci_string::npos ) &&
        ( ci_str.find( wafer )    == ci_string::npos ) &&
        ( ci_str.find( sort )     == ci_string::npos ) &&
        ( ci_str.find( electric ) == ci_string::npos );

    if( final_test_found ) return FinalTest;

    // unhandled testing stage
    // choice is made to throw an exception here for one specific reason :
    // despite it exists locations where possible testing stage string definition are defined (gexdb_plugin_galaxy.h)
    // I encoutered string that are not specified in that file that define testing stage (ie. "electric test" for
    // electric test testing stage).
    // unless an uniformization is done regarding that facts, throwing an exception allow us to quickly track of bad
    // string definition for testing stage.
    throw unhandled_testing_stage_specified();
}
