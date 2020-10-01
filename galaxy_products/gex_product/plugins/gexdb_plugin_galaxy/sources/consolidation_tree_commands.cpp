#include "consolidation_tree_commands.h"
#include "gexdb_plugin_galaxy.h"
#include <gqtl_log.h>

#include <QSqlError>

CTUpdateSplitlotCommand::CTUpdateSplitlotCommand()
    : CTAbstractCommand()
{
}

CTUpdateSplitlotCommand::~CTUpdateSplitlotCommand()
{

}

bool CTUpdateSplitlotCommand::execute(GexDbPlugin_Galaxy *pPlugin)
{
    // TD-78: use the Query system to build the 'select' statement.
    bool success = false;

    if (pPlugin && pPlugin->m_pclDatabaseConnector)
    {
        QString query;

        query = "update wt_splitlot TSL \n";
        query += "set incremental_update = \'BINNING_CONSOLIDATION\' \n";

        if (m_includedProducts.count() > 0)
        {
            query += "where \n";
            query += "TSL.lot_id IN \n";
            query += "(SELECT TL.lot_id FROM  "+pPlugin->m_pclDatabaseConnector->m_strSchemaName+".wt_lot TL  \n";
            query += "  WHERE \n";

            for(int idx = 0; idx < m_includedProducts.count(); idx++)
            {
                // GCORE-1200: Checked [SC]
                if (idx == 0)
                    query += "TL.product_name = \'" + m_includedProducts.at(idx) + "\' \n";
                else
                    query += "or TL.product_name = \'" + m_includedProducts.at(idx) + "\' \n";
            }

            query += ") \n";
        }
        else if (m_excludedProducts.count() > 0)
        {
            query += "where \n";
          query += "TSL.lot_id IN \n";
          query += "(SELECT TL.lot_id FROM  "+pPlugin->m_pclDatabaseConnector->m_strSchemaName+".wt_lot TL  \n";
          query += "  WHERE \n";
            query += "( \n";

            for(int idx = 0; idx < m_excludedProducts.count(); idx++)
            {
                // GCORE-1200: Checked [SC]
                if (idx == 0)
                    query += "TL.product_name != \'" + m_excludedProducts.at(idx) + "\' \n";
                else
                    query += "and TL.product_name != \'" + m_excludedProducts.at(idx) + "\' \n";
            }

            query += ") \n";
        }

        if (!m_beginDate.isNull())
            query += "and TSL.start_t >= " + QString::number(m_beginDate.toTime_t()) + " \n";

        if (!m_endDate.isNull())
            query += "and TSL.start_t <= " + QString::number(m_endDate.toTime_t()) + " \n";

        GexDbPlugin_Query	clQuery(pPlugin,
                                    QSqlDatabase::database(pPlugin->m_pclDatabaseConnector->m_strConnectionName));

        if (clQuery.Execute(query))
            success = true;
        else
        {
            m_errorMessage = "Update splitlot commands: failed - " + clQuery.lastError().text();
            GSLOG(SYSLOG_SEV_ERROR, m_errorMessage.toLatin1().constData());
        }
    }

    return success;
}

void CTUpdateSplitlotCommand::setTestingStage(const QString &testingStage)
{
    m_testingStage = testingStage;
}

void CTUpdateSplitlotCommand::setIncludedProducts(const QStringList &products)
{
    m_includedProducts = products;
}

void CTUpdateSplitlotCommand::setExcludedProducts(const QStringList &products)
{
    m_excludedProducts = products;
}

void CTUpdateSplitlotCommand::setBeginDate(const QDateTime &begin)
{
    m_beginDate = begin;
}

void CTUpdateSplitlotCommand::setBeginDate(const QDate &begin)
{
    m_beginDate.setDate(begin);

    if (m_beginDate.isNull() == false)
        m_beginDate.setTime(QTime());
}

void CTUpdateSplitlotCommand::setEndDate(const QDateTime &end)
{
    m_endDate = end;
}

void CTUpdateSplitlotCommand::setEndDate(const QDate &end)
{
    m_endDate.setDate(end);

    if (m_endDate.isNull() == false)
        m_endDate.setTime(QTime(23, 59, 59, 999));
}

CTUpdateStartTimeCommand::CTUpdateStartTimeCommand()
    : CTAbstractCommand()
{
}

CTUpdateStartTimeCommand::~CTUpdateStartTimeCommand()
{
}

bool CTUpdateStartTimeCommand::execute(GexDbPlugin_Galaxy * pPlugin)
{
    bool success = false;

    if (pPlugin)
    {
        int     testingType = pPlugin->GetTestingStageEnum(m_testingStage);
        QString function    = "min";

        if (m_startTimeFunction.toLower() == "max_start_time")
            function = "max";

        if (pPlugin->UpdateAllConsolidatedTableStartTimeFields(testingType, function))
            success = true;
        else
        {
            m_errorMessage = "Update Start Time commands: failed - ";
            m_errorMessage += GGET_LASTERRORMSG(GexDbPlugin_Base, pPlugin);

            GSLOG(SYSLOG_SEV_ERROR, m_errorMessage.toLatin1().constData());
        }
    }

    return success;
}

bool CTUpdateStartTimeCommand::isHeavyUpdate() const
{
    return true;
}

void CTUpdateStartTimeCommand::setTestingStage(const QString &testingStage)
{
    m_testingStage = testingStage;
}

void CTUpdateStartTimeCommand::setStartTimeFunction(const QString &function)
{
    m_startTimeFunction = function;
}

bool CTAbstractCommand::isHeavyUpdate() const
{
    return false;
}
