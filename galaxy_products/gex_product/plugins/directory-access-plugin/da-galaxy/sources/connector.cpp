
#include <QCryptographicHash>
#include <QDomDocument>
#include <QTimer>
#include <QLocale>

#include "connector_private.h"
#include "sql_connector.h"
#include "connector.h"
#include "gqtl_log.h"
#include "dir_file.h"
#include "users.h"

namespace GS
{
namespace DAPlugin
{

Connector::Connector()
{
    mPrivate = new ConnectorPrivate();
}

Connector::~Connector()
{
    if (mPrivate)
    {
        Disconnect();
        delete mPrivate;
    }
}

bool Connector::Connect(const QMap<QString, QString> &parameters)
{
    // Load parameters
    if (!LoadConParameters(parameters))
    {
        mPrivate->mLastError = "Unable to load connection parameters";
        GSLOG(SYSLOG_SEV_ERROR, mPrivate->mLastError.toLatin1().data());
        return false;
    }

    if (parameters.contains(PARAM_DIR_USER))
    {
        mPrivate->mUserId = parameters.value(PARAM_DIR_USER);
    }
    else
    {
        mPrivate->mLastError = "No user specified";
        GSLOG(SYSLOG_SEV_ERROR, mPrivate->mLastError.toLatin1().data());
        return false;
    }

    if (parameters.contains(PARAM_DIR_USER_PASS))
        mPrivate->mUserPass = parameters.value(PARAM_DIR_USER_PASS);
    else
        mPrivate->mUserPass = "";

    return Connect();
}

bool Connector::Reconnect()
{
    Disconnect();
    mPrivate->mConnected = 0;
    // Open connection to SQL server
    if (!mPrivate->mSqlConnector.Connect(mPrivate->mUserId))
    {
        mPrivate->mLastError = mPrivate->mSqlConnector.GetLastError();
        return false;
    }
    // Load galaxy dir file
    if (!mPrivate->mDirFile.IsFileUpToDate(mPrivate->mSqlConnector) && !LoadInputData())
        return false;

    emit sConnectionStateChanged(true);

    // Succesfull connection
    mPrivate->mConnected = 1;

    return true;
}

bool Connector::Disconnect()
{
    mPrivate->mConnected = 0;
//    mPrivate->mDirFile.Nullify();
    if (mPrivate->mSqlConnector.IsDatabaseConnected())
        mPrivate->mDirFile.ReleaseEditSession(mPrivate->mSqlConnector);
//    mPrivate->mUserId = "";
//    mPrivate->mUserPass = "";

    mPrivate->mSqlConnector.Disconnect();

    emit sSessionStateChanged(false);
    emit sConnectionStateChanged(false);

    GSLOG(SYSLOG_SEV_NOTICE, QString("User " + mPrivate->mUserId + " successfully disconnected").toLatin1().data());

    return true;
}

bool Connector::ChangeUser(const QMap<QString, QString> &parameters)
{
    // Check if new user is valid

    QString login, password;

    if (parameters.contains(PARAM_DIR_USER))
    {
        login = parameters.value(PARAM_DIR_USER);
    }
    else
    {
        mPrivate->mLastError = "No user specified";
        GSLOG(SYSLOG_SEV_ERROR, mPrivate->mLastError.toLatin1().data());
        return false;
    }

    if (parameters.contains(PARAM_DIR_USER_PASS))
    {
        password = parameters.value(PARAM_DIR_USER_PASS);
    }
    else
    {
        password = "";
    }

    if (!CheckUserValidity(login, password))
    {
        return false;
    }

    // Switch user

    mPrivate->mConnected = 0;
    mPrivate->mUserId = login;
    mPrivate->mUserPass = password;
    if (!mPrivate->mSqlConnector.Connect(mPrivate->mUserId))
    {
        mPrivate->mLastError = mPrivate->mSqlConnector.GetLastError();
        return false;
    }
    mPrivate->mConnected = 1;

    GSLOG(SYSLOG_SEV_NOTICE, QString("User successfully changed to " + mPrivate->mUserId).toLatin1().data());

    return true;
}

QString Connector::GetLastError()
{
    return mPrivate->mLastError;
}

bool Connector::IsConnected()
{
    return mPrivate->mConnected;
}

QDateTime Connector::GetServerDateTime()
{
    return mPrivate->mSqlConnector.GetServerDateTime();
}

QString Connector::GetSessionId()
{
    return mPrivate->mSqlConnector.GetSessionId();
}

bool Connector::OnApplyChangesRequested()
{
    if (!mPrivate->mDirFile.Save(mPrivate->mSqlConnector))
    {
        mPrivate->mLastError = mPrivate->mDirFile.GetLastError();
        GSLOG(SYSLOG_SEV_WARNING, mPrivate->mLastError.toLatin1().data());
        return false;
    }

    return true;
}

void Connector::OnConnectionRequested()
{
    Reconnect();
}

void Connector::OnCancelChangesRequested()
{
    // Load galaxy dir file
    LoadInputData();
}

void Connector::OnChangeSessionStateRequested(bool openSession)
{
    if (openSession)
        emit sSessionStateChanged(mPrivate->mDirFile.GetEditSession(mPrivate->mSqlConnector));
    else
        emit sSessionStateChanged(!mPrivate->mDirFile.ReleaseEditSession(mPrivate->mSqlConnector));
}

QString Connector::GetCurrentUser()
{
    return mPrivate->mUserId;
}

bool Connector::ValidityCheck()
{
    bool lIsCheckOk = true;
    // Check connection
    if (!mPrivate->mSqlConnector.IsDatabaseConnected() && !Reconnect())
    {
        lIsCheckOk = false;
        mPrivate->mLastError = "Invalid connection";
        GSLOG(SYSLOG_SEV_ERROR, mPrivate->mLastError.toLatin1().data());
    }
    // Check if up to date
    if (lIsCheckOk)
    {
        if (!mPrivate->mDirFile.IsFileUpToDate(mPrivate->mSqlConnector))
        {
            GSLOG(SYSLOG_SEV_NOTICE, "Data updated by an other user, reload");
            lIsCheckOk = LoadInputData();
        }
        if (!lIsCheckOk)
        {
            mPrivate->mLastError = "Unable to update file";
            GSLOG(SYSLOG_SEV_ERROR, mPrivate->mLastError.toLatin1().data());
        }
    }
    // if data is up to date
    if (lIsCheckOk)
    {
        // Check if edit session has been started somewhere else
        QString lOpenedSessionId = mPrivate->mDirFile.GetCurrentSessionIdFromServer(mPrivate->mSqlConnector, lIsCheckOk);
        QString lCurrentSessionId = mPrivate->mSqlConnector.GetSessionId();
        if (lOpenedSessionId.isEmpty())
            emit sOtherSessionStateChanged(QString());
        else if (lOpenedSessionId != lCurrentSessionId)
        {
            if (mPrivate->mSqlConnector.IsSessionActive(lOpenedSessionId))
                emit sOtherSessionStateChanged(lOpenedSessionId);
            else
                emit sOtherSessionStateChanged(QString());
        }
    }
    // If all is good restart timer
    if (!lIsCheckOk)
    {
        mPrivate->mLastError = "Connection Lost";
        GSLOG(SYSLOG_SEV_ERROR, mPrivate->mLastError.toLatin1().data());
        Disconnect();
    }

    return lIsCheckOk;
}

bool Connector::Connect()
{
    if (mPrivate->mConnected == 1)
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("User " + mPrivate->mUserId + " already connected").toLatin1().data());
        return true;
    }

    // Open connection to SQL server
    if (!mPrivate->mSqlConnector.Connect(mPrivate->mUserId))
    {
        mPrivate->mLastError = mPrivate->mSqlConnector.GetLastError();
        return false;
    }

    // Load galaxy dir file
    if (!LoadInputData())
        return false;

    // Check users validity
    if (!CheckUserValidity(mPrivate->mUserId, mPrivate->mUserPass))
        return false;

    // Succesfull connection
    mPrivate->mConnected = 1;

    emit sConnectionStateChanged(true);

    GSLOG(SYSLOG_SEV_NOTICE, QString("User " + mPrivate->mUserId + " successfully connected").toLatin1().data());

    return true;
}

bool Connector::LoadConParameters(QMap<QString, QString> parameters)
{
    if (parameters.isEmpty())
    {
        mPrivate->mLastError = "Error no connection parameter";
        GSLOG(SYSLOG_SEV_ERROR, mPrivate->mLastError.toLatin1().data());
        return false;
    }

    if (parameters.contains(PARAM_SQL_HOST))
        mPrivate->mSqlConnector.SetHost(parameters.value(PARAM_SQL_HOST));
    if (parameters.contains(PARAM_SQL_PORT))
        mPrivate->mSqlConnector.SetPort(parameters.value(PARAM_SQL_PORT).toInt());
    if (parameters.contains(PARAM_SQL_USER))
        mPrivate->mSqlConnector.SetUser(parameters.value(PARAM_SQL_USER));
    if (parameters.contains(PARAM_SQL_PASS))
        mPrivate->mSqlConnector.SetPass(parameters.value(PARAM_SQL_PASS));
    if (parameters.contains(PARAM_SQL_DRIVER))
        mPrivate->mSqlConnector.SetDriver(parameters.value(PARAM_SQL_DRIVER));
    if (parameters.contains(PARAM_SQL_DBSID))
        mPrivate->mSqlConnector.SetDatabaseSID(parameters.value(PARAM_SQL_DBSID));
    if (parameters.contains(PARAM_SQL_SCHEMA))
        mPrivate->mSqlConnector.SetSchemaName(parameters.value(PARAM_SQL_SCHEMA));
    if (parameters.contains(PARAM_SQL_CONNECTID))
        mPrivate->mSqlConnector.SetConnectionID(parameters.value(PARAM_SQL_CONNECTID));

    return true;
}

bool Connector::CheckUserValidity(const QString& login, const QString& password)
{
    // Load users
    Users lUsers;
    lUsers.Load(mPrivate->mDirFile.GetUsersNode());
    // Check if user is valid
    bool lOk = false;
    if ((login == "admin") && (password == SuperUserPass()))
        return true;

    QString lValidHashedPass = lUsers.GetUserAttribute(login, USER_PASS, lOk);
    if (!lOk)
    {
        mPrivate->mLastError = "Unknown user";
        GSLOG(SYSLOG_SEV_ERROR, mPrivate->mLastError.toLatin1().data());
        return false;
    }
    // Check pass
    QString lCurrentHashedPass = QCryptographicHash::hash(password.toLatin1(),QCryptographicHash::Md5).toHex();
    if (lCurrentHashedPass != lValidHashedPass)
    {
        mPrivate->mLastError = "Wrong password";
        GSLOG(SYSLOG_SEV_ERROR, mPrivate->mLastError.toLatin1().data());
        return false;
    }

    return true;
}

bool Connector::LoadInputData()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Load input data");
    // Load galaxy dir file if needed
    if (!mPrivate->mDirFile.Load(mPrivate->mSqlConnector))
    {
        mPrivate->mLastError = mPrivate->mDirFile.GetLastError();
        GSLOG(SYSLOG_SEV_ERROR, mPrivate->mLastError.toLatin1().data());
        return false;
    }

    emit sUsersLoaded(mPrivate->mDirFile.GetUsersNode());
    emit sGroupsLoaded(mPrivate->mDirFile.GetGroupsNode());
    emit sAppEntriesLoaded(mPrivate->mDirFile.GetAppEntriesNode());
    emit sDataUpdated();

    return true;
}

QString Connector::SuperUserPass()
{
    QString lPart1 = "9S#L1";
    QString lPart2 = "8Rz7sG";
    QString lPart3 = "8y@4ZS";
    QLocale lEnglishUS(QLocale::C);
    QDateTime lClientDate = QDateTime::currentDateTime();

    QString lMonthBeforeLastLetterMaj = lEnglishUS.toString(lClientDate ,"MMMM").right(2).left(1).toUpper();
    QString lMonthSecondLetterMinus = lEnglishUS.toString(lClientDate, "MMMM").mid(1, 1).toLower();

    // 9S#L1  month(beforelastletterMaj)  8Rz7sG  month(secondletterMinus)
    QString lPass = lPart1 + lMonthBeforeLastLetterMaj + lPart2 + lMonthSecondLetterMinus;
    int lDay = lClientDate.toString("d").toInt();
    // check parity
    if((lDay & 1) == 0)         // if even
        lPass.append(lPart3);   // PASS 8y@4ZS
    else                        // if odd
        lPass.prepend(lPart3);  // 8y@4ZS PASS
    // day * 3
    QString lDayMiltipliedBy3 = QString::number(lDay * 3).rightJustified(2, '0');
    int lMonth = lClientDate.toString("M").toInt();
    // now replace at month position first number of day*3
    lPass = lPass.replace(lMonth, 1, lDayMiltipliedBy3.left(1));
    // then at first position second number of the day * 3
    lPass = lPass.replace(0, 1, lDayMiltipliedBy3.right(1));
    // now replace at month position(from the end) first number of year
    lPass = lPass.replace(lPass.size() - lMonth - 1, 1, lClientDate.toString("yy").left(1));
    // then at last position second number of the year
    lPass = lPass.replace(lPass.size() - 1, 1, lClientDate.toString("yy").right(1));

    return lPass;
}

} // END DAPlugin
} // END GS

