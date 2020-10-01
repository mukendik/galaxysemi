#include "spm_database_structure_inspector.h"

#include "gex_database_entry.h"
#include "db_external_database.h"
#include "message.h"
#include "engine.h"
#include "db_engine.h"

#include <QStringBuilder>
#include <QSqlError>

using GS::Gex::Message;

namespace Qx
{
namespace Gex
{

SpmDatabaseStructureInspector::SpmDatabaseStructureInspector(GexDatabaseEntry *aTDREntry) :
    m_TDREntry( aTDREntry ),
    m_ADREntry( NULL )
{
    setRequiredStoredProceduresInADR();
}

void SpmDatabaseStructureInspector::setRequiredStoredProceduresInADR()
{
    m_requiredStoredProceduresInADR
        << "test_stats_spm_et_get_datapoints"
        << "test_stats_spm_et_get_matched_items"
        << "test_stats_spm_et_get_metadatas"
        << "test_stats_spm_et_get_metadatas_count"
        << "test_stats_spm_et_get_num_datapoints"
        << "test_stats_spm_et_get_tests"
        << "test_stats_spm_ft_get_datapoints"
        << "test_stats_spm_ft_get_matched_items"
        << "test_stats_spm_ft_get_metadatas"
        << "test_stats_spm_ft_get_metadatas_count"
        << "test_stats_spm_ft_get_num_datapoints"
        << "test_stats_spm_ft_get_tests"
        << "test_stats_spm_wt_get_datapoints"
        << "test_stats_spm_wt_get_matched_items"
        << "test_stats_spm_wt_get_metadatas"
        << "test_stats_spm_wt_get_metadatas_count"
        << "test_stats_spm_wt_get_num_datapoints"
        << "test_stats_spm_wt_get_tests";
    m_requiredStoredProceduresInADR.sort();
}

bool SpmDatabaseStructureInspector::isCompatibleTDR()
{
    bool lIsCompatible =
        ( ( m_TDREntry != NULL )
          && m_TDREntry->IsCompatible()
          && m_TDREntry->IsUpToDate()
          && ( m_TDREntry->m_pExternalDatabase != NULL ) );

    if( ! lIsCompatible )
    {
        QString lError = "Error when verifying the TDR compatibility for SPM:\n";
        if(m_TDREntry)
        {
            if(m_TDREntry->StatusFlags() & STATUS_CONNECTED)
            {
                lError += "  - Your TDR is not up to date";
            }
            else
            {
                lError += "  - Your TDR is not connected";
            }
        }
        else
        {
            lError += "  - Your TDR is not loaded";
        }
        appendDBStructureErrorMessage( lError );
    }

    return lIsCompatible;
}

GexDbPlugin_Base * SpmDatabaseStructureInspector::getADRPlugin()
{
    return GetADREntry()->m_pExternalDatabase->GetPluginID()->m_pPlugin;
}

GexDbPlugin_Connector * SpmDatabaseStructureInspector::getADRConnector()
{
    return getADRPlugin()->m_pclDatabaseConnector;
}

GexDbPlugin_Query SpmDatabaseStructureInspector::getDBQueryToScanProceduresInADR()
{
    GexDbPlugin_Base *lPlugin = getADRPlugin();
    GexDbPlugin_Connector *lConnector = getADRConnector();
    QSqlDatabase lDatabase = QSqlDatabase::database(lConnector->m_strConnectionName);

    return GexDbPlugin_Query(lPlugin, lDatabase);
}

QString SpmDatabaseStructureInspector::buildQueryTextToScanProceduresInADR()
{
    QString lADRName = getADRConnector()->m_strDatabaseName;
    QString lQuery;
    lQuery =
        "select "
        "routine_name "
        "from "
        "information_schema.routines "
        "where "
        "routine_schema='" %
        lADRName % "'";

    return lQuery;
}

QxStringList SpmDatabaseStructureInspector::fetchAllRowsFromQueryThenSortResult(GexDbPlugin_Query &aDBQuery)
{
    QxStringList lProceduresNames;

    while( aDBQuery.next() )
    {
        lProceduresNames << aDBQuery.value( "routine_name" ).toString();
    }

    lProceduresNames.sort();

    return lProceduresNames;
}

QxStringList SpmDatabaseStructureInspector::emptyListWhenQueryingSPErrors(const GexDbPlugin_Query &aDBQuery)
{
    QSqlError lError = aDBQuery.lastError();
    appendDBStructureErrorMessage( "- Error while querying stored procedure names :" );
    appendDBStructureErrorMessage( "  - " % lError.text() );
    return QxStringList();
}

QxStringList SpmDatabaseStructureInspector::fetchProceduresNamesFromQuery()
{
    GexDbPlugin_Query lDBQuery = getDBQueryToScanProceduresInADR();
    QString lQueryText = buildQueryTextToScanProceduresInADR();

    bool lExecutionResult = lDBQuery.exec( lQueryText );

    return lExecutionResult
           ? fetchAllRowsFromQueryThenSortResult(lDBQuery)
           : emptyListWhenQueryingSPErrors(lDBQuery);
}

void SpmDatabaseStructureInspector::appendDBStructureErrorMessage(const QString &aMessage)
{
    m_databaseStructureErrorMessage = m_databaseStructureErrorMessage % aMessage % QChar( '\n' );
}

bool SpmDatabaseStructureInspector::hasAllTestStatsAgentStoredProcedureInstalledInADR()
{
    QxStringList lFetchedProceduresNames = fetchProceduresNamesFromQuery();

    bool lAllRequiredSPArePresent = ( m_requiredStoredProceduresInADR.hasCommonContentWith( lFetchedProceduresNames ) );

    if( ! lAllRequiredSPArePresent )
    {
        appendDBStructureErrorMessage( "- Missing one or more test stats agent stored procedures in the ADR" );
    }

    return lAllRequiredSPArePresent;
}

bool SpmDatabaseStructureInspector::hasAllRequiredStoredProcedureInstalledInADR()
{
    return hasAllTestStatsAgentStoredProcedureInstalledInADR();
}

bool SpmDatabaseStructureInspector::isValidADRDatabaseEntry()
{
    bool lResult = ( m_ADREntry != NULL );

    if( ! lResult )
    {
        appendDBStructureErrorMessage( "- No ADR associated with the currently selected TDR "
                                       "or the ADR is not connected" );
    }

    return lResult;
}

bool SpmDatabaseStructureInspector::isCompatibleADR()
{
    return ( isValidADRDatabaseEntry() && hasAllRequiredStoredProcedureInstalledInADR() );
}

void SpmDatabaseStructureInspector::resetErrorMessages()
{
    m_databaseStructureErrorMessage.clear();
}

void SpmDatabaseStructureInspector::initializeContextForDatabasesChecks()
{
    resetErrorMessages();
    initializeADREntry();
}

void SpmDatabaseStructureInspector::displayDatabaseStructureErrors()
{
    Message::critical( "Error : incompatible databases structures",
                       "SPM feature cannot work because of these errors :\n" %
                       m_databaseStructureErrorMessage );
}

GexDatabaseEntry * SpmDatabaseStructureInspector::GetADREntry()
{
    return m_ADREntry;
}

GexDatabaseEntry * SpmDatabaseStructureInspector::getConnectedADREntryOrNullFromTDREntry()
{
    GexDatabaseEntry * lADREntry = NULL;

    lADREntry = GS::Gex::Engine::GetInstance()
                .GetDatabaseEngine()
                .FindDatabaseEntry( m_TDREntry->m_pExternalDatabase->GetAdrLinkName(), true );

    if( lADREntry && ( lADREntry->m_pExternalDatabase != NULL ) )
        return lADREntry;

    return NULL;
}

void SpmDatabaseStructureInspector::initializeADREntry()
{
    if( m_TDREntry != NULL )
        m_ADREntry = getConnectedADREntryOrNullFromTDREntry();
}

bool SpmDatabaseStructureInspector::CheckTDRAndADRStructure()
{
    initializeContextForDatabasesChecks();

    if( isCompatibleTDR() && isCompatibleADR() ) return true;

    displayDatabaseStructureErrors();

    return false;
}

}
}
