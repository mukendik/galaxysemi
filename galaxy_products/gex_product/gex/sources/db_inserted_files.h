#ifndef DB_INSERTED_FILES_H
#define DB_INSERTED_FILES_H

#include <QObject>
#include <QString>
#include <QVariant>

class GexDatabaseInsertedFiles : public QObject
{
    Q_OBJECT

public:
    // default constructor
    GexDatabaseInsertedFiles(QObject* parent=0): QObject(parent)
    {
    }
    // alternate constructor
    GexDatabaseInsertedFiles(const QString & strSourceFile, const QString & strDestFile)
        :QObject(0)
    {
        m_strSourceFile = strSourceFile;
        m_strDestFile = strDestFile;
    }
    virtual ~GexDatabaseInsertedFiles() {}
    // copy constructor
    GexDatabaseInsertedFiles(const GexDatabaseInsertedFiles& source)
        :QObject(source.parent())
    {
        m_strSourceFile = source.m_strSourceFile;
        m_strDestFile = source.m_strDestFile;
    }
    // assignment operator
    GexDatabaseInsertedFiles& operator=(const GexDatabaseInsertedFiles& source)
    {
        m_strSourceFile = source.m_strSourceFile;
        m_strDestFile = source.m_strDestFile;
        return *this;
    }
    QVariant Get(const QString key)
    {
        if (key=="source")
            return m_strSourceFile;
        if (key=="dest")
            return m_strDestFile;

        return QVariant();
    }

    QString		m_strSourceFile;
    QString		m_strDestFile;
};

#endif // DB_INSERTED_FILES_H
