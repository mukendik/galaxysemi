#ifndef GQTL_DATAKEYS_CONTENT_H
#define GQTL_DATAKEYS_CONTENT_H

///////////////////////////////////////////////////////////
// QT includes
///////////////////////////////////////////////////////////
#include <QObject>
#include <QMap>
#include <QVariant>
#include <QString>
#include <stdf.h>
#include <stdfparse.h>

#include "gqtl_datakeys_global.h"

namespace GS
{
namespace QtLib
{

class DataKeysDefinitionLoader;
///////////////////////////////////////////////////////////
// Database Keys: Dialog box to let user modify keys when
// importing files into the database.
///////////////////////////////////////////////////////////

class GQTL_DATAKEYSSHARED_EXPORT DatakeysContent : public QObject
{
    Q_OBJECT

    QMap<QString, QVariant> mAttributes;
    QStringList             mAttributesOverloaded;
    static unsigned sNumOfInstances;

public slots:
    // Accessors
    bool    SetDbKeyContent(const QString& key, const QVariant &value, bool fromInputFile=false);

    bool    GetDbKeyContent(const QString& key, QVariant &value) const;
    bool    GetDbKeyContent(const QString &key, QString &value) const;


    // set an attribute from the key name
    bool     Set(const QString &key, QVariant, bool fromInputFile=false);
    // get
    QVariant Get(const QString &) const;

    static QString      defaultConfigFileName();
    static bool         isDefaultConfigFile(const QString& fileName);
    static unsigned     GetNumberOfInstances();

    /*!
      *	\brief Clear all Dynamic DbKeys values
      *
      */
    void            ClearDynamicDbKeys();

    /*!
      *	\brief Clear all test condition values
      *
      */
    void            ClearConditions();

    /*!
      *	\brief Clear 'private' attributes : FileName, DatabaseName,...
      */
    void            ClearAttributes(bool fromInputFile =false);

    /*!
      *	\brief Clear all DbKeys
      */
    void            Clear(bool fromInputFile = false);

public:

    DatakeysContent(QObject* parent=0, bool fromInputFile = false);
    DatakeysContent(const DatakeysContent& other);
    ~DatakeysContent();

    DatakeysContent& operator =(const DatakeysContent&);

    /*!
      *	\brief Returns a QStringList containing the list of the allowed static DbKeys
      *
      */
    QStringList allowedStaticDbKeys() const;
    DataKeysDefinitionLoader &DataKeysDefinition();

    // set internal values.
    bool SetInternal(const QString &, QVariant);

    // Check if the Attibute was overloaded by the user
    bool IsOverloaded(const QString &key);

    //Clean data from extra char
    void CleanKeyData(const QString &key, const QStringList &chars);

    // Not modifiable in config file
    // Record count
    int             StdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_COUNT];

    // ToDo : move me in a Map
    // Not modifiable in config file !
    // DataPumpName: Name of the current datapump
    // ConfigFileName: Name of gexdbkeys config file
    // StdfFileName: STDF file resulting from 'strFileName' converted to STDF.
    // SourceArchive: Original data file name before any conversion (eg: .wat, or .tar.gz file, etc...)
    // FileName: Data file name (source. eg: file.csv, etc...)
    // FullDestinationName: Full filename of the inserted file once it has been moved or renamed
    //
    // SplilotID: (must have for case 2761). -1 if unknown/invalid.
    // DatabaseName: Database use for this insertion
    // This TestingStage string is set by the plugin when inserting.
    // This way, you can know AFTER insertion which TestingStage the file has been inserted into.
    // Default value is consequently '?'
    // TestingStage: TS as generated/set by plugin. The Quantix plugin use "E-Test", "Wafer Sort", "Final Test", "A to Z".
    // Status:
    //    PassedInsertion = 0,        // File inserted with success
    //    FailedInsertion,            // File failed insertion (eg: file corrupted)
    //    FailedValidationStep,       // Failed validation step, file not corrupted but doesn't match with the validation step
    //    DelayInsertion              // File failed insertion but not corrupted (eg: copy problem, etc), so delay insertion to try again later
    // Error: can contain Error message or Warning
    // ToDo : move me in a Map

    /*!
      * \brief Give a summary of the keycontent: FileName, Lot, ...
      * toMap => map["Status"]="Error: x"
      * toList => list.first="Status=Error:x"
      */
    QMap<QString, QVariant> toMap(bool bKeepEmptyParts=false);
    QStringList             toList();

    /*!
      *	\brief Returns a QMap containing the test conditions name and value
      *
      */
    const QMap<QString, QString>&   testConditions() const;

    /*!
      *	\brief Returns a QMap containing the test attributes name and value
      *
      */
    const QMap<QString, QString>&   testAttributes() const;


    enum DbKeysType
    {
        DbKeyUnknown = 0,
        DbKeyStatic,
        DbKeyDynamic
    };

    /*!
      @brief    Get the type of db keys.

      @param    dbKeyName   Name of the db key

      @return   Return the db key type

      @sa       GexDatabaseKeysContent::DbKeysType
      */
    static DbKeysType   dbKeysType(const QString& dbKeyName);

    /*!
      @brief    Check the validity of db dynamic key and its attribute.

      @param    lDynamicKey Name of the db dynamic key including its attribute
                (e.g.: test[name])

      @return   true if the dynamic db keys is correct and its attribute valid,
                otherwise false
      */
    static bool         isValidDynamicKeys(const QString& lDynamicKey, QString &lMessage);

private:

    QMap<QString, QString>                      mTestConditions;
    QMap<QString, QString>                      mTestAttributes;

    // GCORE-1567 - Allow to initialize Keys through JS if not already initialized by GEX
    // true when the KeysContent is initialized for Data insertion
    bool mReadOnly;

};

} //END namespace QtLib
} //END namespace GS

Q_DECLARE_METATYPE(GS::QtLib::DatakeysContent)
Q_DECLARE_METATYPE(GS::QtLib::DatakeysContent*)

#endif // GQTL_DATAKEYS_CONTENT_H
