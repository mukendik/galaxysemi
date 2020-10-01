
#include <QDomDocument>
#include <QByteArray>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QDateTime>
#include <QVariant>
#include <QFile>

#include "dir_file.h"

namespace GS
{
namespace DAPlugin
{
DirFile::DirFile()
{
}

DirFile::~DirFile()
{
}

bool DirFile::Load(SqlConnector &conn)
{
    bool lOk;
    // Get file from sql server
    QString lDirFile = GetFileFromSqlServer(conn, lOk);
    if (!lOk)
        return false;

    // If empty
    if (lDirFile.isEmpty() && lOk)
    {
        // Load default
        lDirFile = GetDefaultFile();
        if (lDirFile.isEmpty())
        {
            mLastError = "Error: unable to load file";
            return false;
        }
    }

    // Decrypt
    lDirFile = Decrypt(lDirFile);

    QString lErrorMsg;
    int lErrorLine, lErrorColumn;

    // Store decrypted
    if (!mGalaxyDirDoc.setContent(lDirFile, &lErrorMsg, &lErrorLine, &lErrorColumn))
    {
        mLastError = QString("%1 at line %2, column %3.").
                arg(lErrorMsg).
                arg(QString::number(lErrorLine)).
                arg(QString::number(lErrorColumn));
        return false;
    }

    // Check file validity
    if (!IsValidFile(mGalaxyDirDoc))
    {
        mLastError = "Error invalid dir file";
        return false;
    }


    return true;
}

bool DirFile::Save(SqlConnector &conn)
{
    // If empty
    if (mGalaxyDirDoc.isNull())
    {
        mLastError = "Error: empty galaxy dir file, unable to save";
        return false;
    }

    return UpdateFileToSqlServer(conn);
}

QString DirFile::GetFileFromSqlServer(SqlConnector &conn, bool &ok)
{
    QString lDirectory = "";
    // Query to get galaxy dir file
    QString lQuery;
    if(conn.IsMySqlServer())
        lQuery = "SELECT AES_DECRYPT(" + QString(SQL_DIR_FIELD_DIRECTORY) + ", '" + ENCRYPTION_KEY + "') AS " + SQL_DIR_FIELD_DIRECTORY + ",\n"
                + SQL_DIR_FIELD_LAST_UPDATE + ",\n"
                + SQL_DIR_FIELD_CHECKSUM + "\n" +
                " FROM\n" +
                conn.Schema() +
                ".da_galaxy";
    else
        lQuery = "SELECT " + QString(SQL_DIR_FIELD_DIRECTORY) + ",\n"
                + SQL_DIR_FIELD_LAST_UPDATE + ",\n"
                + SQL_DIR_FIELD_CHECKSUM + "\n" +
                " FROM\n" +
                conn.Schema() +
                ".da_galaxy";


    QSqlQuery lResultQuery = conn.ExecQuery(lQuery, ok);
    if (!ok)
    {
        mLastError = conn.GetLastError();
        // Hide encryption key in log messages
        mLastError = mLastError.replace(ENCRYPTION_KEY, "##DAG_EK##");
        return lDirectory;
    }

    if (lResultQuery.next())
    {
        int lFieldNo = lResultQuery.record().indexOf(SQL_DIR_FIELD_CHECKSUM);
        int lSavedChecksum = lResultQuery.value(lFieldNo).toInt();
        lFieldNo = lResultQuery.record().indexOf(SQL_DIR_FIELD_DIRECTORY);
        lDirectory = lResultQuery.value(lFieldNo).toString();

        QByteArray lRowData = lDirectory.toLatin1();
        int lComputedChecksum = qChecksum(lRowData.data(), lRowData.length());
        if (lSavedChecksum != lComputedChecksum)
        {
            ok = false;
            lDirectory = "";
            mLastError = "Invalid checksum";
        }
        else
        {
            lFieldNo = lResultQuery.record().indexOf(SQL_DIR_FIELD_LAST_UPDATE);
            mLastUpdate = lResultQuery.value(lFieldNo).toDateTime();
        }
    }

    return lDirectory;
}

bool DirFile::IsFileUpToDate(SqlConnector &conn)
{
    bool lOk = true;
    QDateTime lServerLastUpdate = GetServerFileLastUpdate(conn, lOk);

    if (!lOk)
    {
        mLastError = "Unable to check last update";
        return false;
    }

    if (lServerLastUpdate == mLastUpdate)
        return true;

    return false;
}

bool DirFile::UpdateFileToSqlServer(SqlConnector &conn)
{
    bool lOk, lRecordExists = false;
    QString lQuery;

    lRecordExists = ExistsOnSqlServer(conn);

    QString lDocStringContent = mGalaxyDirDoc.toString();
    QDateTime lUpdateTime = conn.GetServerDateTime();

    // Insert if not exists
    if (!lRecordExists)
    {
        if(conn.IsOracleServer())
        {
            lQuery = "INSERT INTO " + conn.Schema() + "." SQL_DIR_TABLE "\n"
                    "(" SQL_DIR_FIELD_ID ",\n"
                    "" SQL_DIR_FIELD_SESSION ",\n"
                    "" SQL_DIR_FIELD_DIRECTORY ",\n"
                    "" SQL_DIR_FIELD_LAST_UPDATE ")\n"
                    "VALUES\n"
                    "(1,\n"
                    "'',\n"
                    "" + conn.TranslateStringToSqlLob(lDocStringContent) + ",\n"
                    "TO_DATE('" + lUpdateTime.toString("yyyy-MM-dd hh:mm:ss") +
                    "','YYYY-MM-DD HH24:MI:SS'))";
        }
        else if (conn.IsMySqlServer())
        {
            lQuery = "INSERT INTO " + conn.Schema() + "." SQL_DIR_TABLE "\n"
                    "(" SQL_DIR_FIELD_ID ",\n"
                    "" SQL_DIR_FIELD_SESSION ",\n"
                    "" SQL_DIR_FIELD_DIRECTORY ",\n"
                    "" SQL_DIR_FIELD_LAST_UPDATE ")\n"
                    "VALUES\n"
                    "(1,\n"
                    "'',\n"
                    "AES_ENCRYPT('" + conn.TranslateStringToSqlVarChar(lDocStringContent).toLatin1() + "','" + ENCRYPTION_KEY + "'),\n"
                    "'" + lUpdateTime.toString("yyyy-MM-dd hh:mm:ss") +"')";
        }
    }
    else // Update current
    {
        if(conn.IsOracleServer())
        {
            lQuery = "UPDATE " + conn.Schema() + "." SQL_DIR_TABLE "\n"
                    "SET " SQL_DIR_FIELD_DIRECTORY "=" + conn.TranslateStringToSqlLob(lDocStringContent) + ",\n"
                    SQL_DIR_FIELD_LAST_UPDATE "=TO_DATE('" + lUpdateTime.toString("yyyy-MM-dd hh:mm:ss") +
                    "','YYYY-MM-DD HH24:MI:SS')";
        }
        else if (conn.IsMySqlServer())
        {
            lQuery = "UPDATE " + conn.Schema() + "." SQL_DIR_TABLE "\n"
                    "SET " SQL_DIR_FIELD_DIRECTORY "=AES_ENCRYPT('" + conn.TranslateStringToSqlVarChar(lDocStringContent).toLatin1() + "','" + ENCRYPTION_KEY + "'),\n"
                    SQL_DIR_FIELD_LAST_UPDATE "='" + lUpdateTime.toString("yyyy-MM-dd hh:mm:ss") +"'";
        }
    }

    mLastUpdate = lUpdateTime;
    conn.ExecQuery(lQuery, lOk);
    if (!lOk)
    {
        mLastError = "QUERY="+lQuery + " ERROR=" + conn.GetLastError();
        // Hide encryption key in log messages
        mLastError = mLastError.replace(ENCRYPTION_KEY, "##DAG_EK##");
        return false;
    }

    // Get SQL file content
    if(conn.IsMySqlServer())
        lQuery = "SELECT AES_DECRYPT(" + QString(SQL_DIR_FIELD_DIRECTORY) + ", '" + ENCRYPTION_KEY + "') AS " + SQL_DIR_FIELD_DIRECTORY + "\n"
                " FROM\n" +
                conn.Schema() +
                ".da_galaxy";
    else
        lQuery = "SELECT " + QString(SQL_DIR_FIELD_DIRECTORY) + "\n"
                " FROM\n" +
                conn.Schema() +
                ".da_galaxy";


    QSqlQuery lResultQuery = conn.ExecQuery(lQuery, lOk);
    if (!lOk)
    {
        mLastError = conn.GetLastError();
        // Hide encryption key in log messages
        mLastError = mLastError.replace(ENCRYPTION_KEY, "##DAG_EK##");
        return false;
    }

    if (!lResultQuery.next())
    {
        mLastError = "No Directory access recorded";
        return false;
    }

    int lFieldNo = lResultQuery.record().indexOf(SQL_DIR_FIELD_DIRECTORY);
    lDocStringContent = lResultQuery.value(lFieldNo).toString();

    // Update checksum into DB
    QByteArray lRowData = lDocStringContent.toLatin1();
    int lComputedChecksum = qChecksum(lRowData.data(), lRowData.length());

    lQuery = "UPDATE " + conn.Schema() + "." SQL_DIR_TABLE "\n"
            "SET " SQL_DIR_FIELD_CHECKSUM "=" + QString::number(lComputedChecksum) + "\n";

    conn.ExecQuery(lQuery, lOk);
    if (!lOk)
    {
        mLastError = lQuery + "  " + conn.GetLastError();
        return false;
    }

    // Commit query update
    conn.ExecQuery("COMMIT", lOk);
    return true;
}

QString DirFile::Decrypt(const QString& cryptedDirFile)
{
    /// TODO
    return cryptedDirFile;
}


bool DirFile::IsValidFile(const QDomDocument & doc)
{
    if (doc.isNull())
        return false;

    return true;
}

QDomNode DirFile::GetUsersNode()
{
    QDomElement lNodeRoot = mGalaxyDirDoc.
            firstChildElement("galaxy_dir_file");

    QDomNode lNodeUsers = lNodeRoot.firstChildElement("users");

    return lNodeUsers;
}

QDomNode DirFile::GetGroupsNode()
{
    QDomElement lNodeRoot = mGalaxyDirDoc.
            firstChildElement("galaxy_dir_file");

    QDomNode lNodeGroups = lNodeRoot.firstChildElement("groups");

    return lNodeGroups;
}

QDomNode DirFile::GetAppEntriesNode()
{
    QDomElement lNodeRoot = mGalaxyDirDoc.
            firstChildElement("galaxy_dir_file");

    QDomNode lNodeGroups = lNodeRoot.firstChildElement("application_entries");

    return lNodeGroups;
}

void DirFile::Nullify()
{
    mGalaxyDirDoc = QDomDocument();
}

QString DirFile::GetLastError()
{
    return mLastError;
}

QString DirFile::GetDefaultFile()
{
    QFile lFile(":/default_dir_file.xml");

    if (!lFile.open(QIODevice::ReadOnly))
        return QString();

    return QString(lFile.readAll());
}

bool DirFile::GetEditSession(SqlConnector &conn)
{
    bool lOk;
    QString lSessionId = GetCurrentSessionIdFromServer(conn, lOk);

    if (!lOk)
        return false;

    // If the session Id is empty
    if (lSessionId.isEmpty())
        return OpenEditSession(conn); // try to open new
    else
    {
        // check if it's current instance session
        if (lSessionId == conn.GetSessionId())
            return true;
        else
        {
            // Check if this session is active
            if (conn.IsSessionActive(lSessionId))
                return false;
            else
                return OpenEditSession(conn); // try to open new
        }
    }

    return false;
}

bool DirFile::ReleaseEditSession(SqlConnector &conn)
{
    QString lQuery;
    bool lOk;

    QString lServerSessionId = GetCurrentSessionIdFromServer(conn, lOk);

    if (!lOk)
        return false; // unable to release session

    // session released somewhere else
    if (conn.GetSessionId() != lServerSessionId)
        return true;

    lQuery = "UPDATE " + conn.Schema() + "." + QString(SQL_DIR_TABLE) + "\n"
            "SET " +
            QString(SQL_DIR_FIELD_SESSION) + "=NULL\n";

    conn.ExecQuery(lQuery, lOk);
    if (!lOk)
    {
        mLastError = lQuery + "  " + conn.GetLastError();
        return false;  // unable to release session
    }

    // Commit query update
    conn.ExecQuery("COMMIT", lOk);

    return true;
}

QString DirFile::GetCurrentSessionIdFromServer(SqlConnector &conn, bool &ok)
{
    ok = true;

    if (!ExistsOnSqlServer(conn))
        return QString();

    // Query to check if  session is running
    QString lQuery = "SELECT " + QString(SQL_DIR_FIELD_SESSION) +
            " FROM " +
            conn.Schema() +
            ".da_galaxy";

    QSqlQuery lResultQuery = conn.ExecQuery(lQuery, ok);
    if (!ok)
    {
        mLastError = conn.GetLastError();
        ok = false;
        return QString();
    }

    QString lSessionId = "";
    if (lResultQuery.next())
    {
        int lFieldNo = lResultQuery.record().indexOf(SQL_DIR_FIELD_SESSION);
        lSessionId = lResultQuery.value(lFieldNo).toString();
    }
    else
        ok = false;

    return lSessionId;
}

bool DirFile::OpenEditSession(SqlConnector &conn)
{
    QString lQuery;
    bool lOk, lRecordExists;

    QString lCurrentSessionId = conn.GetSessionId();
    if (lCurrentSessionId.isEmpty())
        return false;

    // check if exits in SQL server
    lRecordExists = ExistsOnSqlServer(conn);

    if (lRecordExists)
    {
        lQuery = "UPDATE " + conn.Schema() + "." + QString(SQL_DIR_TABLE) + "\n"
                "SET " +
                QString(SQL_DIR_FIELD_SESSION) + "='" + lCurrentSessionId + "'\n";
    }
    else
    {
        QDateTime lUpdateTime = conn.GetServerDateTime();
        if(conn.IsOracleServer())
        {
            lQuery = "INSERT INTO " + conn.Schema() + "." + QString(SQL_DIR_TABLE) + "\n"
                    "(" + QString(SQL_DIR_FIELD_ID) + ",\n"
                    "" + QString(SQL_DIR_FIELD_SESSION) + ",\n"
                    "" + QString(SQL_DIR_FIELD_LAST_UPDATE) + ")\n"
                    "VALUES\n"
                    "(1,\n"
                    "'" + lCurrentSessionId+ "',\n"
                    "TO_DATE('" + lUpdateTime.toString("yyyy-MM-dd hh:mm:ss") +
                    "','YYYY-MM-DD HH24:MI:SS'))";
        }
        else if (conn.IsMySqlServer())
        {
            lQuery = "INSERT INTO " + conn.Schema() + "." + QString(SQL_DIR_TABLE) + "\n"
                    "(" + QString(SQL_DIR_FIELD_ID) + ",\n"
                    "" + QString(SQL_DIR_FIELD_SESSION) + ",\n"
                    "" + QString(SQL_DIR_FIELD_LAST_UPDATE) + ")\n"
                    "VALUES\n"
                    "(1,\n"
                    "'" + lCurrentSessionId+ "',\n"
                    "'" + lUpdateTime.toString("yyyy-MM-dd hh:mm:ss") +"')";
        }
    }

    conn.ExecQuery(lQuery, lOk);
    if (!lOk)
    {
        mLastError = lQuery + "  " + conn.GetLastError();
        return false;
    }

    // Commit query update
    conn.ExecQuery("COMMIT", lOk);

    // Security check to ensure another instance has not taken the session
    if (lCurrentSessionId == GetCurrentSessionIdFromServer(conn, lOk))
        return true;

    return false;
}

bool DirFile::ExistsOnSqlServer(SqlConnector &conn)
{
    bool lOk;
    QString lQuery;

    // Query to get galaxy dir file
    lQuery = "SELECT " + QString(SQL_DIR_FIELD_DIRECTORY) + "\n" +
            " FROM\n" +
            conn.Schema() +
            ".da_galaxy";

    QSqlQuery lResultQuery = conn.ExecQuery(lQuery, lOk);
    if (!lOk)
    {
        mLastError = conn.GetLastError();
        return false;
    }

    if (lResultQuery.next())
        return true;

    return false;
}

QDateTime DirFile::GetServerFileLastUpdate(SqlConnector &conn, bool &ok)
{
    // Query to get last update
    QString lQuery = "SELECT " + QString(SQL_DIR_FIELD_LAST_UPDATE) +
            " FROM " +
            conn.Schema() +
            ".da_galaxy";

    QSqlQuery lResultQuery = conn.ExecQuery(lQuery, ok);
    if (!ok)
    {
        mLastError = conn.GetLastError();
        return QDateTime();
    }

    if (lResultQuery.next())
    {
        int lFieldNo = lResultQuery.record().indexOf(SQL_DIR_FIELD_LAST_UPDATE);
        return lResultQuery.value(lFieldNo).toDateTime();
    }
    else
        ok =false;

    return QDateTime();
}
} // END DAPlugin
} // END GS

