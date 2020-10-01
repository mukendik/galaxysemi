#include "dbkeysunittest.h"
#include "db_datakeys.h"

#include <QFile>
#include <QFileInfo>


DbKeysUnitTest::DbKeysUnitTest(const QString &unitTestFile)
    : mDbKeysEngine(NULL) ,mUnitTestFile(unitTestFile)
{
}

bool DbKeysUnitTest::execute()
{
    QDomDocument	xmlDoc;
    QFile           unitTest(mUnitTestFile);

	if (unitTest.open(QIODevice::ReadOnly))
	{
		if (xmlDoc.setContent(&unitTest))
		{
            if (xmlDoc.documentElement().tagName() == "dbkeys_engine_ut")
			{
                if (executeStaticUnitTest(xmlDoc.documentElement()) == false)
                {
                    qDebug("Failed to execute Static Unit Test");
                    return false;
                }
                if (executeDynamicUnitTest(xmlDoc.documentElement()) == false)
                {
                    qDebug("Failed to execute Dynamic Unit Test");
                    return false;
                }
            }
            else
                qDebug("not unit test file");
		}
        else
            qDebug("not xml compliant");

		unitTest.close();

        return true;
	}
    else
        qDebug(QString("unable to open unit test file: %1").arg(mUnitTestFile).toLatin1().constData());

    return false;
}

bool DbKeysUnitTest::executeStaticUnitTest(const QDomElement &domElement)
{
    QDomElement		staticElement	= domElement.firstChildElement("static");
	QDomElement		dataElement     = staticElement.firstChildElement("data");
    QDomElement     resultElement   = staticElement.firstChildElement("result");
    QDomElement		keyElement;

    QString                 dbKeyName;
    QString                 dbKeyValue;
    QString                 configFileName;

    bool                    status          = false;
    bool                    validationFail  = false;
    int                     lineError       = -1;
    QString                 error;

    keyElement = dataElement.firstChildElement("key");

    while (keyElement.isNull() == false)
    {
        dbKeyName   = keyElement.attribute("name");
        dbKeyValue  = keyElement.text();

        if (mDbKeysEngine.dbKeysContent().SetDbKeyContent(dbKeyName, dbKeyValue) == false)
        {
            qDebug(QString("Error: %1").arg(mDbKeysEngine.dbKeysContent().Get("Error").toString())
                   .toLatin1().constData());
            return false;
        }

        keyElement = keyElement.nextSiblingElement("key");
    }

    mDbKeysEngine.dbKeysContent().Set("SourceArchive", mUnitTestFile);

    if (mDbKeysEngine.findConfigDbKeysFile(configFileName, mDbKeysEngine.dbKeysContent(), QFileInfo(mUnitTestFile).absolutePath()))
    {
        qDebug(QString("Config file found: %1").arg(configFileName).toLatin1().constData());

        status = mDbKeysEngine.loadConfigDbKeysFile(configFileName, lineError, error);

        if (status)
            status = mDbKeysEngine.evaluateStaticDbKeys(validationFail, lineError, error);
    }
    else
        qDebug("Failed to load config file");

    // Check result
    bool    expectedStatus     = false;
    bool    expectedValidation = false;
    int     expectedLineError  = -1;
    QString expectedDbKeyValue;

    if (resultElement.hasAttribute("status"))
    {
        expectedStatus = (resultElement.attribute("status").toInt() == 1);

        if (expectedStatus != status)
        {
            qDebug(QString("Unexpected status: %1 - %2 - %3").arg(status).arg(lineError).arg(error).toLatin1().constData());

            return false;
        }
    }

    if (resultElement.hasAttribute("validation"))
    {
        expectedValidation = (resultElement.attribute("validation").toInt() == 1);

        if (expectedValidation != validationFail)
        {
            qDebug(QString("Unexpected validation: %1 - %2 - %3").arg(status).arg(lineError).arg(error).toLatin1().constData());

            return false;
        }
    }

    if (resultElement.hasAttribute("lineError"))
    {
        expectedLineError = resultElement.attribute("lineError").toInt();

        if (expectedLineError != lineError)
        {
            qDebug(QString("Unexpected line error: %1 - %2 - %3").arg(status).arg(lineError).arg(error).toLatin1().constData());

            return false;
        }
    }

    keyElement = resultElement.firstChildElement("key");

    while (keyElement.isNull() == false)
    {
        dbKeyName           = keyElement.attribute("name");
        expectedDbKeyValue  = keyElement.text();

        if (mDbKeysEngine.dbKeysContent().GetDbKeyContent(dbKeyName, dbKeyValue) == false)
            return false;

        qDebug(QString("DbKey %1: expected[%2] evaluated[%3]").arg(dbKeyName).arg(expectedDbKeyValue).arg(dbKeyValue).toLatin1().constData());

        if (expectedDbKeyValue != dbKeyValue)
        {
            qDebug("Evaluation failed");
            return false;
        }

        keyElement = keyElement.nextSiblingElement("key");
    }

    return true;
}

bool DbKeysUnitTest::executeDynamicUnitTest(const QDomElement &domElement)
{
    QDomElement		staticElement	= domElement.firstChildElement("dynamic");
    QDomElement     iterElement     = staticElement.firstChildElement("iteration");
	QDomElement		dataElement     = iterElement.firstChildElement("data");
    QDomElement     resultElement   = staticElement.firstChildElement("result");
    QDomElement		keyElement;

    QString                 dbKeyName;
    QString                 dbKeyValue;

    bool                    status          = false;
    int                     lineError       = -1;
    QString                 error;

    while (iterElement.isNull() == false)
    {
        qDebug("New iteration");

        dataElement     = iterElement.firstChildElement("data");
        resultElement   = iterElement.firstChildElement("result");

        keyElement      = dataElement.firstChildElement("key");

        while (keyElement.isNull() == false)
        {
            dbKeyName   = keyElement.attribute("name");
            dbKeyValue  = keyElement.text();

            if (mDbKeysEngine.dbKeysContent().dbKeysType(dbKeyName) != GexDatabaseKeysContent::DbKeyDynamic)
                return false;

            if (mDbKeysEngine.dbKeysContent().SetDbKeyContent(dbKeyName, dbKeyValue) == false)
            {
                qDebug(QString("Error: %1").arg(mDbKeysEngine.dbKeysContent().Get("Error").toString())
                       .toLatin1().constData());
                return false;
            }

            keyElement = keyElement.nextSiblingElement("key");
        }

        status = mDbKeysEngine.evaluateDynamicDbKeys(lineError, error);

        // Check result
        bool    expectedStatus     = false;
        QString expectedDbKeyValue;

        if (resultElement.hasAttribute("status"))
        {
            expectedStatus = (resultElement.attribute("status").toInt() == 1);

            if (!status)
            {
                qDebug(QString("Error at line %1").arg(lineError).toLatin1().constData());
                return false;
            }
        }

        keyElement = resultElement.firstChildElement("key");

        while (keyElement.isNull() == false)
        {
            dbKeyName           = keyElement.attribute("name");
            expectedDbKeyValue  = keyElement.text();

            if (mDbKeysEngine.dbKeysContent().GetDbKeyContent(dbKeyName, dbKeyValue) == false)
                return false;

            qDebug(QString("DbKey %1: expected[%2] evaluated[%3]").arg(dbKeyName).arg(expectedDbKeyValue).arg(dbKeyValue).toLatin1().constData());

            if (expectedDbKeyValue != dbKeyValue)
            {
                qDebug("Evaluation failed");
                return false;
            }

            keyElement = keyElement.nextSiblingElement("key");
        }

        iterElement = iterElement.nextSiblingElement("iteration");
    }

    return true;
}


