#ifndef SPM_DATABASE_STRUCTURE_INSPECTOR_H
#define SPM_DATABASE_STRUCTURE_INSPECTOR_H

#include "qx_string_list.h"

#include <QStringList>
#include <QString>

class GexDatabaseEntry;
class GexDbPlugin_Connector;
class GexDbPlugin_Base;
class GexDbPlugin_Query;

namespace Qx
{
namespace Gex
{

/**
 * @brief The SpmDatabaseStructureInspector class ensures both TDR and ADR has the minimum prerequisites to be used in
 * the SPM feature of Yield man
 */
class SpmDatabaseStructureInspector
{
public :
    explicit SpmDatabaseStructureInspector( GexDatabaseEntry *aTDREntry );

    bool CheckTDRAndADRStructure();
    GexDatabaseEntry * GetADREntry();

private :
    QxStringList m_requiredStoredProceduresInADR;
    QString m_databaseStructureErrorMessage;
    GexDatabaseEntry *m_TDREntry;
    GexDatabaseEntry *m_ADREntry;

private :
    GexDatabaseEntry * getConnectedADREntryOrNullFromTDREntry();
    void initializeContextForDatabasesChecks();
    void displayDatabaseStructureErrors();
    void resetErrorMessages();
    void initializeADREntry();
    void appendDBStructureErrorMessage( const QString &aMessage );
    bool isCompatibleTDR();
    QxStringList emptyListWhenQueryingSPErrors( const GexDbPlugin_Query &aDBQuery );
    QxStringList fetchAllRowsFromQueryThenSortResult( GexDbPlugin_Query &aDBQuery );
    QxStringList fetchProceduresNamesFromQuery();
    QString buildQueryTextToScanProceduresInADR();
    GexDbPlugin_Connector * getADRConnector();
    GexDbPlugin_Base * getADRPlugin();
    GexDbPlugin_Query getDBQueryToScanProceduresInADR();
    void setRequiredStoredProceduresInADR();
    bool hasAllTestStatsAgentStoredProcedureInstalledInADR();
    bool hasAllRequiredStoredProcedureInstalledInADR();
    bool isValidADRDatabaseEntry();
    bool isCompatibleADR();
};

}
}

#endif // SPM_DATABASE_STRUCTURE_INSPECTOR_H
