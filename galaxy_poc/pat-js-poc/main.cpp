#include <QCoreApplication>
#include "JSClasses.h"

#include <QScriptEngine>
#include <QScriptValue>
#include <QtScriptTools>
#include <QTextStream>

void test(int a)
{
    for (a = 0; a < 1000; ++a)
        qDebug("coucou");
}


Q_DECLARE_METATYPE(JSBinDescription)
Q_DECLARE_METATYPE(JSBinDescription*)

Q_DECLARE_METATYPE(JSWaferMap)
Q_DECLARE_METATYPE(JSWaferMap*)

template <typename T>
QScriptValue qScriptValueFromQObject(QScriptEngine *engine, T const &qobject)
{
    return engine->newQObject(qobject);
}

template <typename T>
void qScriptValueToQObject(const QScriptValue &value, T &qobject)
{
    qobject = qobject_cast<T>(value.toQObject());
}

template <typename T>
int qScriptRegisterQObjectMetaType(
        QScriptEngine *engine,
        const QScriptValue &prototype = QScriptValue(),
        T* /* dummy */ = 0
        )
{
    return qScriptRegisterMetaType<T>(
                engine, qScriptValueFromQObject, qScriptValueToQObject, prototype);
}

Q_SCRIPT_DECLARE_QMETAOBJECT(JSBinDescription, QObject*)
Q_SCRIPT_DECLARE_QMETAOBJECT(JSWaferMap, QObject*)

JSBinDescription MakeBinPrecedenceDecision(const JSBinDescription& lFromMAP,
                                           const JSBinDescription& lFromSTDF)
{
    JSBinDescription lMerged;

    if (lFromMAP.GetNumber() == -1)
    {
        if (lFromSTDF.GetNumber() == -1)
        {
            lMerged.SetNumber(-1);
        }
    }
    else if (lFromMAP.GetCategory() == "P")
    {
        if (lFromSTDF.GetNumber() != -1)
            lMerged = lFromSTDF;
        else
        {
            // Untested die in STDF map
            // Keep the prober one
            lMerged = lFromMAP;
        }
    }
    else
    {
        // Pass die in STDF map
        // Keep the prober one
        lMerged = lFromMAP;
    }

    return lMerged;
}

void ExecuteBinPrecedenceThroughJS(QScriptEngine& lScriptEngine, int lDieCount, int nTimes)
{
    JSBinDescription        lBinMap;
    JSBinDescription        lBinSTDF;
    QElapsedTimer           eTimer;

    eTimer.start();

    QScriptValue    lFromMapBin = lScriptEngine.newQObject((QObject*)&lBinMap);
    if (lFromMapBin.isNull() )
        qDebug("Failed to register FromMap");

    QScriptValue    lFromSTDFBin = lScriptEngine.newQObject((QObject*)&lBinSTDF);
    if (lFromSTDFBin.isNull() )
        qDebug("Failed to register FromSTDF");

    QScriptValue        binPrecedence = lScriptEngine.globalObject().property("BinPrecedence");
    QScriptValueList    svl;

    svl.append(lFromMapBin);
    svl.append(lFromSTDFBin);

    for (int iter = 0; iter < nTimes; ++iter)
    {
        for (int a = 0; a < lDieCount; ++a)
        {
            if (a % 100 == 0)
            {
                lBinMap.SetNumber(-1);
                lBinMap.SetCategory("P");
            }
            else if (a %10 == 0)
            {
                lBinMap.SetCategory("F");
                lBinMap.SetNumber(99);
            }
            else
            {
                lBinMap.SetCategory("P");
                lBinMap.SetNumber(1);
            }

            if (a % 200 == 0)
            {
                lBinSTDF.SetNumber(-1);
                lBinSTDF.SetCategory("F");
            }
            else if (a % 25 == 0)
            {
                lBinSTDF.SetNumber(25);
                lBinSTDF.SetCategory("F");
            }
            else
            {
                lBinSTDF.SetNumber(1);
                lBinSTDF.SetCategory("P");
            }

//            qDebug(QString("stdf bin : %1").arg(lBinSTDF.GetNumber()).toLatin1().constData());
            JSBinDescription * lBinResult = qobject_cast<JSBinDescription *>(binPrecedence.call(QScriptValue(), svl).toQObject());
//            qDebug(QString("stdf bin : %1").arg(lBinSTDF.GetNumber()).toLatin1().constData());

    //        qDebug(QString("%1").arg(lResult->GetNumber()).toLatin1().constData());
    //        qDebug(lResult->GetCategory().toLatin1().constData());
    //        qDebug(lBinMap.GetName().toLatin1().constData());
        }
    }

    qDebug(QString("Execute bin precedence through JS %1 times on %2 dies: %3 ms")
           .arg(nTimes).arg(lDieCount).arg(eTimer.elapsed()).toLatin1().constData());


}

void ExecuteBinPrecedenceThroughNC(int lDieCount, int nTimes)
{
    QElapsedTimer           eTimer;
    JSBinDescription        lBinMap;
    JSBinDescription        lBinSTDF;
    JSBinDescription        lBinResult;

    eTimer.start();

    for (int iter = 0; iter < nTimes; ++iter)
    {
        for (int a = 0; a < lDieCount; ++a)
        {
            if (a % 100 == 0)
            {
                lBinMap.SetNumber(-1);
                lBinMap.SetCategory("P");
            }
            else if (a %10 == 0)
            {
                lBinMap.SetCategory("F");
                lBinMap.SetNumber(99);
            }
            else
            {
                lBinMap.SetCategory("P");
                lBinMap.SetNumber(1);
            }

            if (a % 200 == 0)
            {
                lBinSTDF.SetNumber(-1);
                lBinSTDF.SetCategory("F");
            }
            else if (a % 25 == 0)
            {
                lBinSTDF.SetNumber(25);
                lBinSTDF.SetCategory("F");
            }
            else
            {
                lBinSTDF.SetNumber(1);
                lBinSTDF.SetCategory("P");
            }

            if (lBinMap.GetNumber() == -1)
            {
                if (lBinSTDF.GetNumber() == -1)
                {
                    lBinResult.SetNumber(-1);
                }
            }
            else if (lBinMap.GetCategory() == "P")
            {
                if (lBinSTDF.GetNumber() != -1)
                    lBinResult = lBinSTDF;
                else
                {
                    // Untested die in STDF map
                    // Keep the prober one
                    lBinResult = lBinMap;
                }
            }
            else
            {
                // Pass die in STDF map
                // Keep the prober one
                lBinResult = lBinMap;
            }

           // lBinResult = MakeBinPrecedenceDecision(lBinMap, lBinSTDF);
        }
    }

    qDebug(QString("Execute bin precedence through NC %1 times on %2 dies: %3 ms")
           .arg(nTimes).arg(lDieCount).arg(eTimer.elapsed()).toLatin1().constData());
}

void ExecuteMergeMapThroughJS(QScriptEngine& lScriptEngine, int nTimes)
{
    QElapsedTimer   eTimer;

    eTimer.start();

    for (int iter = 0; iter < nTimes; ++iter)
    {
        JSWaferMap      lExternalMap;
        JSWaferMap      lSTDFMap;

        lExternalMap.Create(0,0,99,99);
        lSTDFMap.Create(0,0,99,99);

        QScriptValue    lJSExternalMap = lScriptEngine.newQObject((QObject*)&lExternalMap);
        if (lJSExternalMap.isNull() )
            qDebug("Failed to register JSExternalMap");

        QScriptValue    lJSSTDFMap = lScriptEngine.newQObject((QObject*)&lSTDFMap);
        if (lJSSTDFMap.isNull() )
            qDebug("Failed to register JSSTDFMap");

        QScriptValue        MergeMap = lScriptEngine.globalObject().property("MergeMap");
        QScriptValueList    svl;

        svl.append(lJSExternalMap);
        svl.append(lJSSTDFMap);

        JSWaferMap * lWafer = qobject_cast<JSWaferMap *>(MergeMap.call(QScriptValue(), svl).toQObject());
    }

    qDebug(QString("Execute Merge maps through JS %1 times on %2 dies: %3 ms")
           .arg(nTimes).arg(10000).arg(eTimer.elapsed()).toLatin1().constData());
}

void ExecuteMergeMapThroughNC(int nTimes)
{
    QElapsedTimer   eTimer;

    eTimer.start();

    for (int iter = 0; iter < nTimes; ++iter)
    {
        JSWaferMap      lMergedMap;
        JSWaferMap      lExternalMap;
        JSWaferMap      lSTDFMap;

        lExternalMap.Create(0,0,99,99);
        lSTDFMap.Create(0,0,99,99);

        int MinX = (lExternalMap.GetLowX() < lSTDFMap.GetLowX()) ? lExternalMap.GetLowX() : lSTDFMap.GetLowX();
        int MinY = (lExternalMap.GetLowY() < lSTDFMap.GetLowY()) ? lExternalMap.GetLowY() : lSTDFMap.GetLowY();
        int MaxX = (lExternalMap.GetHighX() > lSTDFMap.GetHighX()) ? lExternalMap.GetHighX() : lSTDFMap.GetHighX();
        int MaxY = (lExternalMap.GetHighY() > lSTDFMap.GetHighY()) ? lExternalMap.GetHighY() : lSTDFMap.GetHighY();
        int binMerged;

        lMergedMap.Create(MinX, MinY, MaxX, MaxY);

        for (int indexX = lMergedMap.GetLowX(); indexX < lMergedMap.GetHighX(); indexX++)
        {
            for (int indexY = lMergedMap.GetLowY(); indexY < lMergedMap.GetHighY(); indexY++)
            {
                int binExt 	= lExternalMap.GetBinAt(indexX, indexY);
                int binSTDF	= lSTDFMap.GetBinAt(indexX, indexY);

                if (binExt == -1)
                {
                    if (binSTDF == -1)
                        binMerged = -1;
                    else
                        binMerged = binSTDF;
                }
                else if (binExt == 1)
                {
                    if (binSTDF != -1)
                        binMerged = binSTDF;
                    else
                        binMerged = binExt;
                }
                else
                    binMerged = binExt;

                lSTDFMap.SetBinAt(binMerged, indexX, indexY);
            }
        }
    }

    qDebug(QString("Execute Merge maps through NC %1 times on %2 dies: %3 ms")
           .arg(nTimes).arg(10000).arg(eTimer.elapsed()).toLatin1().constData());
}

int main(int argc, char *argv[])
{
    QCoreApplication    lApplication(argc, argv);
    int                 lDieCount = 50000;
    int                 lRepeat   = 1;

    for (int lIdx = 1; lIdx < lApplication.arguments().count(); ++lIdx)
    {
        QString lArg = lApplication.arguments().at(lIdx);
        if (lArg.startsWith("--dieCount="))
        {
            bool    bNumeric;

            lDieCount   = lArg.remove("--dieCount=").toInt(&bNumeric);

            if (!bNumeric)
            {
                qDebug(QString("Error: argument dieCount must be a numeric value").toLatin1().constData());
                return 3;
            }
        }
        else
        {
            qDebug(QString("Error: invalid argument %1").arg(lArg).toLatin1().constData());
            return 4;
        }

    }

    QScriptEngine   lScriptEngine;
    QScriptValue    lJSBinDescription   = lScriptEngine.scriptValueFromQMetaObject<JSBinDescription>();
    QScriptValue    lJSWaferMap         = lScriptEngine.scriptValueFromQMetaObject<JSWaferMap>();

    qScriptRegisterQObjectMetaType<JSBinDescription*>(&lScriptEngine, lJSBinDescription);
    lScriptEngine.globalObject().setProperty("JSBinDescription", lJSBinDescription);

    qScriptRegisterQObjectMetaType<JSWaferMap*>(&lScriptEngine, lJSWaferMap);
    lScriptEngine.globalObject().setProperty("JSWaferMap", lJSWaferMap);

    QString fileName = "/Users/Herve/Development/Repositories/galaxy_dev_master/galaxy_poc/pat-js-poc/poc-pat-js.js";
    QFile scriptFile(fileName);
    if (!scriptFile.open(QIODevice::ReadOnly))
        return 1 ;// handle error
    QTextStream    lStream(&scriptFile);
    QString        contents = lStream.readAll();
    scriptFile.close();

    QScriptValue lError = lScriptEngine.evaluate(contents, fileName);
    if(lError.isError() || lScriptEngine.hasUncaughtException())
    {
        qDebug(QString("Error when loading script: %1").arg(lError.toString()).toLatin1().constData());
        return 2;
    }
    else
        qDebug(QString("Successfully loaded script: %1").arg(fileName).toLatin1().constData());

    ExecuteBinPrecedenceThroughJS(lScriptEngine, lDieCount, lRepeat);

    ExecuteBinPrecedenceThroughNC(lDieCount, lRepeat);

    ExecuteMergeMapThroughJS(lScriptEngine, lRepeat);

    ExecuteMergeMapThroughNC(lRepeat);

    return 0;
}
