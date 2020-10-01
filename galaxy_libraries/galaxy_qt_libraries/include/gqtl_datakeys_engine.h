#ifndef GQTL_DATAKEYS_ENGINE_H
#define GQTL_DATAKEYS_ENGINE_H

#include <QObject>
#include <QStringList>

#include "gqtl_datakeys_global.h"

namespace GS
{
namespace QtLib
{

class  DatakeysContent;

struct DatakeysEnginePrivate;

class GQTL_DATAKEYSSHARED_EXPORT DatakeysEngine : public QObject
{
    Q_OBJECT

public:

    /*!
        @brief  Constructs a default dbKeys engine.

        @param  parent pointer to the parent object
    */
    DatakeysEngine(QObject* parent);

    /*!
        \brief Constructs a dbKeys engine initialized with a given set of
        dbKeys
    */
    DatakeysEngine(const DatakeysContent& dbKeysContent);

    /*!
      \brief Destroys the dbKeys engine.
    */
    ~DatakeysEngine();

    /*!
        @brief      Load a db keys config file

        @param[in]  configFileName  name of the config file
        @param[out] lineError       Line number where the error occured
        @param[out] error           Message describing the error

        @return     true if the config file is correctly loaded, otherwise false
    */
    bool    loadConfigDbKeysFile(const QString& configFileName, int &lineError,
                                 QString &error);
    void    deleteConfigDbKeysFile();
    void    renameConfigDbKeysFile(const QString& newExtension);
    void    moveConfigDbKeysFile(const QString& destDir);

    bool    evaluateStaticDbKeys(bool &validationFail,
                                 int &lineError, QString &error) const;
    bool    evaluateDynamicDbKeys(int& lineError, QString &error) const;

    /*!
        @brief      Give access to the db key content

        @return     Returns a reference on the contents of the dbKeys
    */
    DatakeysContent& dbKeysContent();

    /*!
        @brief      Get the name of the config file being used

        @return     Returns a string containing the config filename
    */
    QString configFileName() const;

    int GetCountDynamicKeys() const;

    enum TestCondOrigin {NONE, CONFIG_FILE, DTR};
    /*!
        @brief      check  test conditions origin
        @return     Returns NONE, CONFIG_FILE or DTR
    */
    TestCondOrigin GetTestConditionsOrigin() const;
    /*!
        @brief      Set test conditions origin
        @return     nothing
    */
    void SetTestConditionsOrigin(TestCondOrigin origin);

protected:

    bool    evaluateDbKeys(const QString& dbKeyName,
                           const QString& dbKeyExpression,
                           bool& validationFail, QString& error) const;

    bool    parseValue(const QString &expression, QString& result,
                       QString& keyValue, bool& isMatching, bool& isValidResult,
                       QString& error) const;

    bool    parseValueFieldSection(const QString &strExpression, QString &strResult,
                                   QString &strKeyValue,
                                   QString &strError) const;

    bool    parseValueFieldRegExp(const QString &strExpression, QString &strResult,
                                  QString &strKeyValue, bool &bMatch,
                                  bool &bValidResult, QString &strError,
                                  QRegExp::PatternSyntax PatternSyntax) const;

    bool    parseValueDate(const QString &strExpression, QString &strResult,
                           QString &strKeyValue, QString &strError) const;

private:

    Q_DISABLE_COPY(DatakeysEngine);

    DatakeysEnginePrivate *   mPrivate;

    // STATIC METHODS
public:

    static bool     evaluateDbKeys(DatakeysContent& dbKeysContent,
                                   const QString& dbKeyName,
                                   const QString& dbKeyExpression,
                                   QString& error);

    static bool     evaluateExpression(const QString &strExpression,
                                       QString& keyValue,
                                       QString& strResult, bool& bMatch,
                                       bool& bValidResult, QString& strError);

    static bool     findConfigDbKeysFile(QString& configFileName,
                                         DatakeysContent& keyContent,
                                         const QString &configFolder = "");
};

} //END namespace QtLib
} //END namespace GS

#endif // GQTL_DATAKEYS_ENGINE_H
