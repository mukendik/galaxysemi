#ifndef DBKEYSUNITTEST_H
#define DBKEYSUNITTEST_H

#include "database_keys_engine.h"
#include <QDomDocument>

class DbKeysUnitTest
{
public:

    DbKeysUnitTest(const QString& unitTestFile);

    bool                        execute();

protected:

    bool                        executeStaticUnitTest(const QDomElement &domElement);
    bool                        executeDynamicUnitTest(const QDomElement& domElement);

private:

    GS::DbPluginBase::DatabaseKeysEngine        mDbKeysEngine;
    QString                                     mUnitTestFile;

};



#endif // DBKEYSUNITTEST_H
