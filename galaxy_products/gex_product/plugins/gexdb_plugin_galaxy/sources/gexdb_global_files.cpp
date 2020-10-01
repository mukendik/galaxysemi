#include <QSqlError>
#include <QSqlRecord>

#include "gexdb_global_files.h"
#include "gexdb_plugin_galaxy.h"
#include "gexdb_plugin_base.h"

GexdbGlobalFiles::GexdbGlobalFiles(GexDbPlugin_Base *pPluginBase)
{
    m_pPluginBase = pPluginBase;
}

GexdbGlobalFiles::~GexdbGlobalFiles()
{

}

const QString& GexdbGlobalFiles::lastError() const
{
    return m_strLastErrorMsg;
}

///////////////////////////////////////////////////////////////////////////////
// Return true if the local file (update) date time is equal or more recent
// than the one of the database
///////////////////////////////////////////////////////////////////////////////
bool GexdbGlobalFiles::isLocalFileUpToDate(const QDateTime &dateLocal,
                                           const QString &strName,
                                           const QString &strType,
                                           const QString &strFormat)
{
    if (!m_pPluginBase || !m_pPluginBase->m_pclDatabaseConnector)
        return false;

    // Empty last error message
    m_strLastErrorMsg = "";

    if (!dateLocal.isValid())
    {
        m_strLastErrorMsg = "Invalid input File date";
        return false;
    }

    GexDbPlugin_Query   clQuery(m_pPluginBase, QSqlDatabase::database(m_pPluginBase->m_pclDatabaseConnector->m_strConnectionName));
    int                 iFieldNo;
    QString             strQuery;
    QDateTime           dateDatabaseFile;

    strQuery = "SELECT * FROM "+m_pPluginBase->m_pclDatabaseConnector->m_strSchemaName+".global_files "
               "WHERE "
               "file_name= '" + strName + "' "
               "AND file_type= '" + strType + "' "
               "AND file_format= '" + strFormat + "' ";

    // Exec query
    if(!clQuery.exec(strQuery))
    {
        m_strLastErrorMsg = "Error in query: " + strQuery + "\n"
                            " " + clQuery.lastError().text() + ".";
        return false;
    }

    // If no result
    if (!clQuery.first())
    {
        m_strLastErrorMsg = "No record found.";
        return false;
    }

    // Retrieve update date
    iFieldNo = clQuery.record().indexOf("file_last_update");
    dateDatabaseFile = clQuery.value(iFieldNo).toDateTime();

    // If the local file last update date is older than the one on the database
    if (dateLocal < dateDatabaseFile )
        return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Return true if file already exists in database
///////////////////////////////////////////////////////////////////////////////
bool GexdbGlobalFiles::exists(const QString &strName,
                              const QString &strType,
                              const QString &strFormat)
{
    if (!m_pPluginBase || !m_pPluginBase->m_pclDatabaseConnector)
        return false;

    // Empty last error message
    m_strLastErrorMsg = "";

    GexDbPlugin_Query	clQuery(m_pPluginBase, QSqlDatabase::database(m_pPluginBase->m_pclDatabaseConnector->m_strConnectionName));
    QString				strQuery;

    strQuery = "SELECT * FROM "+m_pPluginBase->m_pclDatabaseConnector->m_strSchemaName+".global_files "
               "WHERE "
               "file_name= '" + strName + "' "
               "AND file_type= '" + strType + "' "
               "AND file_format= '" + strFormat + "' ";

    // Exec query
    if(!clQuery.exec(strQuery))
    {
        m_strLastErrorMsg = "Error in query: " + strQuery + "\n"
                            " " + clQuery.lastError().text() + ".";
        return false;
    }

    // If no record found
    if (!clQuery.first())
        return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Save file to globel_files table, with a checksum on the file + date
// Return true if everything is OK
///////////////////////////////////////////////////////////////////////////////
bool GexdbGlobalFiles::saveFile(const QString &strName,
                                const QString &strType,
                                const QString &strFormat,
                                const QDateTime &dateLastUpdate,
                                const QString &strContent)
{
    if (!m_pPluginBase || !m_pPluginBase->m_pclDatabaseConnector)
        return false;

    GexDbPlugin_Query   clQuery(m_pPluginBase, QSqlDatabase::database(m_pPluginBase->m_pclDatabaseConnector->m_strConnectionName));
    QString             strQuery, strRowContent;
    int                 iChecksum = 0;

    // Empty last error message
    m_strLastErrorMsg = "";

    strRowContent = strName + strType + strFormat + dateLastUpdate.toString(Qt::ISODate) + strContent;
    strRowContent = strRowContent.simplified();

    // Compute the checksum on the row content
    QByteArray rowData = strRowContent.toLatin1();
    iChecksum = qChecksum(rowData.data(), rowData.length());

    // If already exist: update row
    if (this->exists(strName, strType, strFormat))
    {
        if(m_pPluginBase->m_pclDatabaseConnector->IsOracleDB())
        {
            strQuery = "UPDATE "+m_pPluginBase->m_pclDatabaseConnector->m_strSchemaName+".global_files\n"
                    "SET\n"
                    "file_content=" + m_pPluginBase->m_pclDatabaseConnector->TranslateStringToSqlLob(strContent) + ",\n"
                    "file_checksum=" + QString::number(iChecksum) + ",\n"
                    "file_last_update=TO_DATE('" + dateLastUpdate.toString("yyyy-MM-dd hh:mm:ss") + "','YYYY-MM-DD HH24:MI:SS')\n"
                    "WHERE\n"
                    "file_name='" + strName + "'\n"
                    "AND file_type='" + strType + "'\n"
                    "AND file_format='" + strFormat + "'\n";
        }
        else if (m_pPluginBase->m_pclDatabaseConnector->IsMySqlDB())
        {
            strQuery = "UPDATE "+m_pPluginBase->m_pclDatabaseConnector->m_strSchemaName+".global_files\n"
                    "SET\n"
                    "file_content=" + m_pPluginBase->m_pclDatabaseConnector->TranslateStringToSqlLob(strContent) + ",\n"
                    "file_checksum=" + QString::number(iChecksum) + ",\n"
                    "file_last_update='" + dateLastUpdate.toString("yyyy-MM-dd hh:mm:ss") + "'\n"
                    "WHERE\n"
                    "file_name='" + strName + "'\n"
                    "AND file_type='" + strType + "'\n"
                    "AND file_format='" + strFormat + "'\n";
        }
    }
    else // Insert new record
    {
        if(m_pPluginBase->m_pclDatabaseConnector->IsOracleDB())
        {
            unsigned int uiFileId = 0;
            QString strSequenceName = "global_file_id";
            // 1. Check if auto sequence exists
            strQuery = "select LAST_NUMBER from USER_SEQUENCES WHERE lower(SEQUENCE_NAME)='" + strSequenceName + "'";
            if(!clQuery.Execute(strQuery))
            {
                m_strLastErrorMsg = "Error in query: " + strQuery + ",\n can't select LAST_NUMBER from USER_SEQUENCE! ";
                m_strLastErrorMsg+= "\n"+clQuery.lastError().text();
                return false;
            }
            // 2. If not exist, create auto sequence
            if(!clQuery.First())
            {
                // Try to create auto sequence
                strQuery = "CREATE SEQUENCE "+m_pPluginBase->m_pclDatabaseConnector->m_strSchemaName+"." + strSequenceName + " START WITH 1 INCREMENT BY 1 NOCACHE";
                if(!clQuery.Execute(strQuery))
                {
                    m_strLastErrorMsg = "Error in query: " + strQuery + ",\n can't create SEQUENCE " + strSequenceName + "!";
                    m_strLastErrorMsg+= "\n"+clQuery.lastError().text();
                    return false;
                }
            }
            // 3. Get next value
            strQuery = "SELECT "+m_pPluginBase->m_pclDatabaseConnector->m_strSchemaName+"." + strSequenceName + ".nextval FROM dual";
            if(!clQuery.Execute(strQuery))
            {
                m_strLastErrorMsg = "Error in query: " + strQuery + ",\n can't select SEQUENCE.NEXTVAL!";
                m_strLastErrorMsg+= "\n"+clQuery.lastError().text();
                return false;
            }
            if(!clQuery.First())
            {
                m_strLastErrorMsg = "Error in query: " + strQuery + ",\n no record!";
                return false;
            }
            uiFileId = clQuery.value(0).toInt();

            strQuery = "INSERT INTO "+m_pPluginBase->m_pclDatabaseConnector->m_strSchemaName+".global_files\n"
                    "(file_id,\n"
                    "file_name,\n"
                    "file_type,\n"
                    "file_format,\n"
                    "file_content,\n"
                    "file_checksum,\n"
                    "file_last_update)\n"
                    "VALUES\n"
                    "(" + QString::number(uiFileId) + ",\n"
                    "'" + strName + "',\n"
                    "'" + strType + "',\n"
                    "'" + strFormat + "',\n"
                    "" + m_pPluginBase->m_pclDatabaseConnector->TranslateStringToSqlLob(strContent) + ",\n"
                    "" + QString::number(iChecksum) + ",\n"
                    "TO_DATE('" + dateLastUpdate.toString("yyyy-MM-dd hh:mm:ss") + "','YYYY-MM-DD HH24:MI:SS'))\n";
        }
        else if(m_pPluginBase->m_pclDatabaseConnector->IsMySqlDB())
        {
            strQuery = "INSERT INTO "+m_pPluginBase->m_pclDatabaseConnector->m_strSchemaName+".global_files\n"
                    "(file_name,\n"
                    "file_type,\n"
                    "file_format,\n"
                    "file_content,\n"
                    "file_checksum,\n"
                    "file_last_update)\n"
                    "VALUES\n"
                    "('" + strName + "',\n"
                    "'" + strType + "',\n"
                    "'" + strFormat + "',\n"
                    "" + m_pPluginBase->m_pclDatabaseConnector->TranslateStringToSqlLob(strContent) + ",\n"
                    "" + QString::number(iChecksum) + ",\n"
                    "'" + dateLastUpdate.toString("yyyy-MM-dd hh:mm:ss") + "')\n";
        }
    }

    // Exec query
    if(!clQuery.exec(strQuery))
    {
        m_strLastErrorMsg = "Error in query: " + strQuery + "\n"
                            " " + clQuery.lastError().text() + ".";
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Query the DB to get the requested file, check if the file hasn't been
// updated with another application by comparing the checksum
///////////////////////////////////////////////////////////////////////////////
bool GexdbGlobalFiles::loadFile(const QString &strName,
                                const QString &strType,
                                const QString &strFormat,
                                QDateTime &dateLastUpdate,
                                QString &strContent)
{
    if (!m_pPluginBase || !m_pPluginBase->m_pclDatabaseConnector)
        return false;

    GexDbPlugin_Query	clQuery(m_pPluginBase, QSqlDatabase::database(m_pPluginBase->m_pclDatabaseConnector->m_strConnectionName));
    QString				strQuery, strRowContent;
    int					iDatabaseChecksum, iComputedChecksum, iFieldNo;
    bool				bOk;

    // Empty last error message
    m_strLastErrorMsg = "";

    // Retrieve Database data
    strQuery = "SELECT "
               "file_content, "
               "file_checksum, "
               "file_last_update "
               "FROM "+m_pPluginBase->m_pclDatabaseConnector->m_strSchemaName+".global_files "
               "WHERE "
               "file_name='" + strName + "' "
               "AND file_type='" + strType + "' "
               "AND file_format='" + strFormat + "' ";

    // Exec query
    if(!clQuery.exec(strQuery))
    {
        m_strLastErrorMsg = "Error in query: " + strQuery + "\n"
                            " " + clQuery.lastError().text() + ".";
        return false;
    }

    // If no record found
    if (!clQuery.first())
    {
        m_strLastErrorMsg = "Error no record found.";
        return false;
    }

    iFieldNo = clQuery.record().indexOf("file_content");
    strContent = clQuery.value(iFieldNo).toString();
    iFieldNo = clQuery.record().indexOf("file_checksum");
    iDatabaseChecksum = clQuery.value(iFieldNo).toInt(&bOk);
    if (!bOk)
    {
        m_strLastErrorMsg = "Error while retrieving checksum.";
        return false;
    }
    iFieldNo = clQuery.record().indexOf("file_last_update");
    dateLastUpdate = clQuery.value(iFieldNo).toDateTime();

    // Recompute checksum
    strRowContent = strName + strType + strFormat + dateLastUpdate.toString(Qt::ISODate) + strContent;
    strRowContent = strRowContent.simplified();

    // Compute the checksum on the row content
    QByteArray rowData = strRowContent.toLatin1();
    iComputedChecksum = qChecksum(rowData.data(), rowData.length());

    // Check the checksum value
    if (iComputedChecksum != iDatabaseChecksum)
    {
        m_strLastErrorMsg = "Error checksum does not match.";
        return false;
    }

    return true;
}

QString GexdbGlobalFiles::escapeXMLChar(const QString& strXmlToEscape)
{
    QString strEscapedXml = "";
    int index;

    for (index = 0; index < strXmlToEscape.size(); index++)
    {
        QChar character(strXmlToEscape.at(index));

        switch (character.unicode())
        {
        case '&':
            strEscapedXml += "&amp;"; break;

        case '\'':
            strEscapedXml += "&apos;"; break;

        case '"':
            strEscapedXml += "&quot;"; break;

        case '<':
            strEscapedXml += "&lt;"; break;

        case '>':
            strEscapedXml += "&gt;"; break;

        default:
            strEscapedXml += character;
            break;
        }
    }

return strEscapedXml;
}

QString GexdbGlobalFiles::restoreXMLChar(const QString& strEscapedXml)
{
    QString strRestoredXml(strEscapedXml);

    strRestoredXml.replace("&amp;", "&");
    strRestoredXml.replace("&apos;", "'");
    strRestoredXml.replace("&quot;", "\"");
    strRestoredXml.replace("&lt;", "<");
    strRestoredXml.replace("&gt;", ">");

    return strRestoredXml;
}

