#include <QIODevice>
#include <QTextStream>
#include <QFile>

#include <gqtl_log.h>

#include "gqtl_datakeys_content.h"
#include "gqtl_datakeys_file.h"

namespace GS
{
namespace QtLib
{

struct DatakeysFilePrivate
{
    inline DatakeysFilePrivate()      {}

    inline DatakeysFilePrivate(const QString &fileName)
        : mConfigFileName(fileName)  {}

    QList<DatakeysInfo>           mStaticFlow;
    QList<DatakeysInfo>           mDynamicFlow;
    QString                     mConfigFileName;
};

DatakeysFile::DatakeysFile(QObject * parent /*= NULL*/)
    : QObject(parent), mPrivate(new DatakeysFilePrivate())
{

}

DatakeysFile::DatakeysFile(const QString &fileName):
    QObject(NULL), mPrivate(new DatakeysFilePrivate(fileName))
{

}

DatakeysFile::DatakeysFile(const DatakeysFile &other)
    : QObject(other.parent())
{
    *this = other;
}

DatakeysFile::~DatakeysFile()
{
    if (mPrivate)
        delete mPrivate;
}

DatakeysFile &DatakeysFile::operator =(const DatakeysFile &other)
{
    if (this != &other)
    {
        if (!mPrivate)
            mPrivate = new DatakeysFilePrivate();

        mPrivate->mConfigFileName   = other.mPrivate->mConfigFileName;
        mPrivate->mDynamicFlow      = other.mPrivate->mDynamicFlow;
        mPrivate->mStaticFlow       = other.mPrivate->mStaticFlow;
    }

    return *this;
}

bool DatakeysFile::Read(QList<DatakeysError> &lErrors)
{
    if (mPrivate)
    {
        int currentLine = -1;
        DatakeysError lDbKeyError;
        QFile lConfigFile(mPrivate->mConfigFileName);

        // Check file name
        if (!lConfigFile.fileName().endsWith(".gexdbkeys"))
        {
            lDbKeyError.mLine       = -1;
            lDbKeyError.mMessage    = QString("%1 is not a config keys file. Unable to read it.").arg(mPrivate->mConfigFileName);

            lErrors.append(lDbKeyError);

            GSLOG(SYSLOG_SEV_ERROR, lDbKeyError.mMessage.toLatin1().constData());

            return false;
        }

        // Configuration file exists: read it!
        if (!lConfigFile.open(QIODevice::ReadOnly))
        {
            lDbKeyError.mLine       = -1;
            lDbKeyError.mMessage    = QString("Unable to open config key file %1").arg(mPrivate->mConfigFileName);

            lErrors.append(lDbKeyError);

            GSLOG(SYSLOG_SEV_ERROR, lDbKeyError.mMessage.toLatin1().constData());

            return false;
        }

        // Assign file I/O stream
        QTextStream streamFile(&lConfigFile);

        // Read config file, and load keys found.
        QString dbKeyLine;
        QString dbKeyName;
        QString dbKeyExpression;

        do
        {
            dbKeyLine       = streamFile.readLine();
            dbKeyName       = dbKeyLine.section(',',0,0);             // First field is 'Parameter'
            dbKeyName       = dbKeyName.trimmed().toLower();          // remove leading spaces and convert to lower case
            dbKeyExpression = dbKeyLine.section(',',1);               // Rest of the line is the expression
            dbKeyExpression = dbKeyExpression.trimmed();              // remove leading spaces.

            // Increase line number
            ++currentLine;

            // Check if comment line.
            if(dbKeyName.isEmpty() || dbKeyLine.startsWith("#"))
                continue;

            if (DatakeysContent::dbKeysType(dbKeyName) == DatakeysContent::DbKeyDynamic)
            {
                // Init Test condition in the DB keys content in order to know the list
                // of test condition
                mPrivate->mDynamicFlow.append(DatakeysInfo(currentLine,
                                                         dbKeyName,
                                                         dbKeyExpression));

                QString message;
                if (DatakeysContent::isValidDynamicKeys(dbKeyName, message) == false)
                {
                    lDbKeyError.mLine       = currentLine;
                    lDbKeyError.mMessage    = message;

                    lErrors.append(lDbKeyError);

                    GSLOG(SYSLOG_SEV_ERROR, lDbKeyError.mMessage.toLatin1().constData());
                }
            }
            else if (DatakeysContent::dbKeysType(dbKeyName) == DatakeysContent::DbKeyStatic)
            {
                if (mPrivate->mDynamicFlow.isEmpty() == false)
                {
                    lDbKeyError.mLine       = currentLine;
                    lDbKeyError.mMessage    = QString("It is not allowed to declare static db key (%1) after dynamic db keys").arg(dbKeyName);

                    lErrors.append(lDbKeyError);

                    GSLOG(SYSLOG_SEV_ERROR, lDbKeyError.mMessage.toLatin1().constData());
                }

                mPrivate->mStaticFlow.append(DatakeysInfo(currentLine,
                                                          dbKeyName,
                                                          dbKeyExpression));
            }
            // Unknown db keys
            else
            {
                lDbKeyError.mLine       = currentLine;
                lDbKeyError.mMessage    = QString("Unknown db key %1 (%2)").arg(dbKeyName).arg(dbKeyExpression);

                lErrors.append(lDbKeyError);

                GSLOG(SYSLOG_SEV_ERROR, lDbKeyError.mMessage.toLatin1().constData());
            }

        }
        while((streamFile.atEnd() == false));

        // Close file
        lConfigFile.close();
    }
    else
    {
        DatakeysError lDbKeyError(-1, "Internal error.");

        lErrors.append(lDbKeyError);

        GSLOG(SYSLOG_SEV_CRITICAL, lDbKeyError.mMessage.toLatin1().constData());
    }

    return lErrors.isEmpty();
}

bool DatakeysFile::Write(DatakeysError& lError)
{
    if (mPrivate)
    {
        QFile configFile(mPrivate->mConfigFileName);

        // Configuration file exists: read it!
        if(!configFile.open(QIODevice::WriteOnly))
        {
            lError.mLine    = -1;
            lError.mMessage = QString("Unable to open file: %1").arg(mPrivate->mConfigFileName);
            GSLOG(SYSLOG_SEV_ERROR, lError.mMessage.toLatin1().data());

            return false;
        }

        // Assign file I/O stream
        QTextStream streamFile(&configFile);

        // Read config file, and load keys found.
        QString dbKeyLine;
        QString dbKeyName;
        QString dbKeyExpression;

        // Write Static keys first
        streamFile << "# STATIC KEYS" << endl;
        for (int i = 0 ; i < mPrivate->mStaticFlow.count(); ++i)
        {
            dbKeyName = mPrivate->mStaticFlow.at(i).mKeyName;
            dbKeyExpression = mPrivate->mStaticFlow.at(i).mKeyExpression;
            dbKeyLine = dbKeyName + ", " + dbKeyExpression;
            streamFile << dbKeyLine << endl;
        }
        streamFile << endl;

        // Write Dynamic keys keys secondly
        streamFile << "# DYNAMIC KEYS" << endl;
        for (int i = 0 ; i < mPrivate->mDynamicFlow.count(); ++i)
        {
            dbKeyName = mPrivate->mDynamicFlow.at(i).mKeyName;
            dbKeyExpression = mPrivate->mDynamicFlow.at(i).mKeyExpression;
            dbKeyLine = dbKeyName + ", " + dbKeyExpression;
            streamFile << dbKeyLine << endl;
        }
        streamFile << endl;

        // Close file
        configFile.close();

        return true;
    }

    lError.mLine    = -1;
    lError.mMessage = "Internal Error.";

    GSLOG(SYSLOG_SEV_CRITICAL, lError.mMessage.toLatin1().data());

    return false;
}

int DatakeysFile::CountStaticKeys() const
{
    if (mPrivate)
        return mPrivate->mStaticFlow.count();
    else
    {
        GSLOG(SYSLOG_SEV_CRITICAL, "Internal Error.");
        return -1;
    }
}

int DatakeysFile::CountDynamicKeys() const
{
    if (mPrivate)
        return mPrivate->mDynamicFlow.count();
    else
    {
        GSLOG(SYSLOG_SEV_CRITICAL, "Internal Error.");
        return -1;
    }
}

bool DatakeysFile::GetStaticKeysAt(DatakeysInfo &key, int flowId) const
{
    if (mPrivate)
    {
        if (flowId >= 0 && flowId < mPrivate->mStaticFlow.count())
        {
            key = mPrivate->mStaticFlow.at(flowId);
            return true;
        }
    }
    else
        GSLOG(SYSLOG_SEV_CRITICAL, "Internal Error.");

    return false;
}

bool DatakeysFile::GetDynamicKeysAt(DatakeysInfo &key, int flowId) const
{
    if (mPrivate)
    {
        if (flowId >= 0 && flowId < mPrivate->mDynamicFlow.count())
        {
            key = mPrivate->mDynamicFlow.at(flowId);
            return true;
        }
    }
    else
        GSLOG(SYSLOG_SEV_CRITICAL, "Internal Error.");

    return false;
}

QString DatakeysFile::GetFileName() const
{
    if (mPrivate)
        return mPrivate->mConfigFileName;
    else
        GSLOG(SYSLOG_SEV_CRITICAL, "Internal Error.");

    return QString();
}

void DatakeysFile::InsertStaticKey(const DatakeysInfo &key, int flowId/*=-1*/)
{
    if (mPrivate)
    {
        if (flowId < 0 || flowId >= mPrivate->mStaticFlow.count())
            mPrivate->mStaticFlow.insert(mPrivate->mStaticFlow.count(), key);
        else
            mPrivate->mStaticFlow.insert(flowId, key);
    }
    else
        GSLOG(SYSLOG_SEV_CRITICAL, "Internal Error.");
}

void DatakeysFile::InsertStaticKey(const QString &name,
                                 const QString &expression,
                                 int flowId/* = -1*/)
{
    if (mPrivate)
    {
        if (flowId < 0 || flowId > mPrivate->mStaticFlow.count())
            InsertStaticKey(DatakeysInfo(mPrivate->mStaticFlow.count(), name, expression),
                            mPrivate->mStaticFlow.count());
        else
            InsertStaticKey(DatakeysInfo(flowId, name, expression),flowId);
    }
    else
        GSLOG(SYSLOG_SEV_CRITICAL, "Internal Error.");
}

void DatakeysFile::InsertDynamicKey(const DatakeysInfo &key, int flowId/*=-1*/)
{
    if (mPrivate)
    {
        if (flowId < 0 || flowId >= mPrivate->mDynamicFlow.count())
            mPrivate->mDynamicFlow.insert(mPrivate->mDynamicFlow.count(), key);
        else
            mPrivate->mDynamicFlow.insert(flowId, key);
    }
    else
        GSLOG(SYSLOG_SEV_CRITICAL, "Internal Error.");
}

void DatakeysFile::InsertDynamicKey(const QString &name,
                                  const QString &expression,
                                  int flowId/* = -1*/)
{
    if (mPrivate)
    {
        if (flowId < 0 || flowId > mPrivate->mDynamicFlow.count())
            InsertDynamicKey(DatakeysInfo(mPrivate->mDynamicFlow.count(), name, expression),
                            mPrivate->mDynamicFlow.count());
        else
            InsertDynamicKey(DatakeysInfo(flowId, name, expression),flowId);
    }
    else
        GSLOG(SYSLOG_SEV_CRITICAL, "Internal Error.");
}

bool DatakeysFile::RemoveStaticKeyAt(int flowId)
{
    if (mPrivate)
    {
        if (flowId >= 0 && flowId < mPrivate->mStaticFlow.count())
        {
            mPrivate->mStaticFlow.removeAt(flowId);
            return true;
        }
    }
    else
        GSLOG(SYSLOG_SEV_CRITICAL, "Internal Error.");

    return false;
}

bool DatakeysFile::RemoveDynamicKeyAt(int flowId)
{
    if (mPrivate)
    {
        if (flowId >= 0 && flowId < mPrivate->mDynamicFlow.count())
        {
            mPrivate->mDynamicFlow.removeAt(flowId);
            return true;
        }
    }
    else
        GSLOG(SYSLOG_SEV_CRITICAL, "Internal Error.");

    return false;
}

} //END namespace QtLib
} //END namespace GS
