#include "consolidation_tree.h"
#include "consolidation_tree_replies.h"
#include "consolidation_tree_updater.h"
#include "gexdb_plugin_galaxy.h"
#include "gexdb_global_files.h"
#include <gqtl_log.h>

#include <QIODevice>
#include <QFile>
#include <QTextEdit>

ConsolidationTree::ConsolidationTree(GexDbPlugin_Galaxy * pPlugin)
    : m_pPlugin(pPlugin), m_data(pPlugin)
{
}

const QDateTime &ConsolidationTree::lastModification() const
{
    return m_lastModification;
}

const QString& ConsolidationTree::xmlContent() const
{
    return m_data.content();
}

const ConsolidationTreeData &ConsolidationTree::data() const
{
    return m_data;
}

const CTReplies &ConsolidationTree::replies() const
{
    return m_replies;
}

bool ConsolidationTree::loadFromDB()
{
    bool success = false;

    // Reset internal variable
    m_replies.clear();
    m_data.clear();
    m_valid = false;
    m_lastModification = QDateTime();

    if (m_pPlugin && m_pPlugin->m_pclDatabaseConnector)
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Consolidation Tree [load from DB: %1]: starting").arg(
                    m_pPlugin->m_pclDatabaseConnector->m_strSchemaName.toLatin1().data()).toLatin1().constData());

        GexdbGlobalFiles    globalFiles(m_pPlugin);
        QDateTime           lastUpdate;
        QString             fileContent;

        if (globalFiles.loadFile("consolidation_tree", "consolidation", "xml", lastUpdate, fileContent))
        {
            success             = true;
            m_lastModification  = lastUpdate;
            m_valid             = m_data.setContent(fileContent, m_replies);

            GSLOG(SYSLOG_SEV_INFORMATIONAL, (QString("Consolidation Tree [load from DB: %1]: success [%2]")
                     .arg(m_pPlugin->m_pclDatabaseConnector->m_strSchemaName.toLatin1().data())
                     .arg((m_data.isValid()) ? "valid" : "invalid")).toLatin1().constData());
        }
        else
        {
            QString message = QString("Consolidation Tree [load from DB: %1]: failed [%2]")
                                .arg(m_pPlugin->m_pclDatabaseConnector->m_strSchemaName)
                                .arg(globalFiles.lastError());

            GSLOG(SYSLOG_SEV_ERROR, message.toLatin1().data());
        }
    }
    else
        GSLOG(SYSLOG_SEV_ERROR, "Consolidation Tree [load from DB]: No database connector instantiated");

    return success;
}

//bool ConsolidationTree::load(const QByteArray &data, const QDateTime &lastModification, CTReplies& replies)
//{
//    m_xmlContent        = data;
//    m_lastModification  = lastModification;

//    m_valid = validate(data, replies);

//    return m_valid;
//}

bool ConsolidationTree::importFromFile(const QString &fileLocation)
{
    // Reset internal variable
    m_replies.clear();
    m_data.clear();
    m_valid = false;
    m_lastModification = QDateTime();

    GSLOG(SYSLOG_SEV_INFORMATIONAL, (QString("Consolidation Tree [Import from file %1]: importing").arg(
                fileLocation.toLatin1().data())).toLatin1().constData());

    if (fileLocation.isEmpty() == false)
    {
        QFile xmlFile;
        xmlFile.setFileName(fileLocation);

        if (xmlFile.open(QIODevice::ReadOnly))
        {
            QDateTime   lastUpdate  = QDateTime::currentDateTime();
            QByteArray  xmlData     = xmlFile.readAll();

            if (m_data.setContent(xmlData, m_replies))
            {
                if (saveIntoDB(xmlData, lastUpdate))
                {
                    m_valid             = true;
                    m_lastModification  = lastUpdate;

                    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Consolidation Tree [Import from file %1]: success").arg(
                                fileLocation.toLatin1().data()).toLatin1().constData());
                }
                else
                    GSLOG(SYSLOG_SEV_ERROR, QString("Consolidation Tree [Import from file %1]: DB import failed").arg(
                             fileLocation.toLatin1().data()).toLatin1().constData());
            }
            else
            {
                QString message = QString("Consolidation Tree [Import from file %1]: validation failed").arg(fileLocation);

                GSLOG(SYSLOG_SEV_ERROR, message.toLatin1().data());
            }

            xmlFile.close();
        }
        else
        {
            GSLOG(SYSLOG_SEV_ERROR, QString("Consolidation Tree [Import from file %1]: Unable to open file").arg(
                     fileLocation.toLatin1().data()).toLatin1().constData());
        }
    }
    else
        GSLOG(SYSLOG_SEV_ERROR, "Consolidation Tree [Import from file]: File name is empty");

    return m_valid;
}

bool ConsolidationTree::isUpToDate() const
{
    bool upToDate = false;

    GSLOG(SYSLOG_SEV_DEBUG, "Consolidation Tree [isUpToDate]: starting");

    if (m_lastModification.isValid())
    {
        GexdbGlobalFiles    globalFiles(m_pPlugin);

        if (globalFiles.isLocalFileUpToDate(m_lastModification, "consolidation_tree", "consolidation", "xml"))
            upToDate = true;
        else if (globalFiles.lastError().isEmpty() == false)
            GSLOG(SYSLOG_SEV_ERROR, QString("Consolidation Tree [isUpToDate]: failed - %1")
                  .arg( globalFiles.lastError()).toLatin1().constData());
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("Consolidation Tree [isUpToDate]: %1")
          .arg( (upToDate) ? "true" : "false").toLatin1().constData());

    return upToDate;
}

bool ConsolidationTree::update(const QString& xmlContent)
{
    GSLOG(SYSLOG_SEV_DEBUG, "Consolidation Tree [update]: starting");

    // Clear replies
    m_replies.clear();

    ConsolidationTreeData data(m_pPlugin);

    if (data.setContent(xmlContent, m_replies))
    {
        // Check each wafer/sublot to flag as 'Need to Reconsolidate'.
        ConsolidationTreeUpdater CTUpdater;

        if (CTUpdater.prepare(m_data, data))
        {
            // Create a backup of the current Consolidation Tree data
            // If there is an error during the DB update or the save into DB, we rollback
            // the consolidation tree to this backup.
            ConsolidationTreeData backupData = m_data;

            // Update current Consolidation Tree with new data
            m_data = data;

            // Execute all commands generated during the prepare step.
            if (CTUpdater.execute(m_pPlugin))
            {
                // Write new consolidation tree into the database.
                QDateTime lastUpdate = QDateTime::currentDateTime();

                if (saveIntoDB(data.content(), lastUpdate))
                {
                    m_valid             = true;
                    m_lastModification  = lastUpdate;

                    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Consolidation Tree [update]: success");

                    return true;
                }
                else
                    GSLOG(SYSLOG_SEV_ERROR, "Consolidation Tree [update]: DB save failed");
            }
            else
            {
                GSLOG(SYSLOG_SEV_ERROR, "Consolidation Tree [update]: DB update failed");

                m_replies.appendReply(CTReply(CTReply::ReplyError, "Consolidation Tree [update]: DB update failed"));
            }

            // Something wrong happened during DB update.
            // Rollback to the previous version
            m_data = backupData;
        }
        else
        {
            // Write new consolidation tree into the database.
            QDateTime lastUpdate = QDateTime::currentDateTime();

            if (saveIntoDB(data.content(), lastUpdate))
            {
                m_data = data;
                m_valid             = true;
                m_lastModification  = lastUpdate;

                GSLOG(SYSLOG_SEV_INFORMATIONAL, "Consolidation Tree [update]: success");

                return true;
            }
        }
    }

    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Consolidation Tree [update]: failed");

    return false;
}

bool ConsolidationTree::isValid() const
{
    return m_valid;
}

//bool ConsolidationTree::validate(const QByteArray &data, CTReplies &replies) const
//{
//    QString xsdFilePath(":/resources/consolidation_tree.xsd");

//    ConsolidationTreeValidator validator(m_pPlugin, xsdFilePath);

//    return validator.validate(data, replies);
//}

bool ConsolidationTree::saveIntoDB(const QString &xmlContent, const QDateTime& lastUpdate)
{
    bool                success = false;

    if (m_pPlugin && m_pPlugin->m_pclDatabaseConnector)
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Consolidation Tree [save into DB %1]: starting").arg(
                    m_pPlugin->m_pclDatabaseConnector->m_strSchemaName.toLatin1().constData()).toLatin1().constData());

        GexdbGlobalFiles    globalFiles(m_pPlugin);

        if (globalFiles.saveFile("consolidation_tree", "consolidation", "xml", lastUpdate, xmlContent))
        {
            success = true;
            GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Consolidation Tree [save into DB %1]: success").arg(
                        m_pPlugin->m_pclDatabaseConnector->m_strSchemaName.toLatin1().data()).toLatin1().constData());
        }
        else
        {
            QString message = QString("Consolidation Tree [save into DB %1]: failed [%2]")
                                    .arg(m_pPlugin->m_pclDatabaseConnector->m_strSchemaName)
                                    .arg(globalFiles.lastError());

            GSLOG(SYSLOG_SEV_ERROR, message.toLatin1().constData());
            m_replies.appendReply(CTReply(CTReply::ReplyError, message));
        }
    }
    else
        GSLOG(SYSLOG_SEV_ERROR, "Consolidation Tree [save into DB]: No database connector allocated");

    return success;
}






