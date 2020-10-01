#include <QThread>
#include <QSqlDatabase>

#include "gexdbthreadquery.h"
#include "gexdb_plugin_galaxy.h"
#include <QSqlError>
#include <QApplication>

int GexDbThreadQuery::mThreadCount = 0;

GexDbThreadQuery::GexDbThreadQuery(QObject* poObj, GexDbPlugin_Base *poPluginBase, const QString &strConnectionName, const QString &strQuery) :
    QThread(poObj)
{
    mConnectionName = QString("%1_%2").arg(strConnectionName).arg(mThreadCount);
    QSqlDatabase oDatabase = QSqlDatabase::cloneDatabase(QSqlDatabase::database(strConnectionName), mConnectionName);
    if(oDatabase.open())
        mGexDbQuery = new GexDbPlugin_Query(poPluginBase,oDatabase);
    else
        mGexDbQuery = 0;
    mQuery = strQuery;
    mQueryStatus = false;
    ++mThreadCount;
}

GexDbThreadQuery::~GexDbThreadQuery(){
    if(mGexDbQuery)
        delete mGexDbQuery;
    QSqlDatabase::removeDatabase(mConnectionName);
    --mThreadCount;
}

bool GexDbThreadQuery::getQueryStatus()
{
    return mQueryStatus;
}

void GexDbThreadQuery::setQuery(const QString &strQuery)
{
    mQuery = strQuery;
}

QString GexDbThreadQuery::getQuery()
{
    return mQuery;
}

GexDbPlugin_Query *GexDbThreadQuery::getQueryResult()
{
    return mGexDbQuery;
}

void GexDbThreadQuery::run(){
    if(mGexDbQuery)
        mQueryStatus = mGexDbQuery->Execute(mQuery);
    else
        mQueryStatus = false;
}

QList<GexDbThreadQuery *> GexDbThreadQuery::executeQueries(QObject *poObj,
                                                           GexDbPlugin_Base *poPluginBase,
                                                           const QString &strConnectionName,
                                                           const QStringList &croQueriesList,
                                                           int iAskedThreadNumber,
                                                           double dStep,
                                                           bool bDeleteWhenDone)
{
    if(croQueriesList.isEmpty() || !iAskedThreadNumber)
        return QList<GexDbThreadQuery *>();

    int lAllowedThreadNumber = qMin(iAskedThreadNumber, QThread::idealThreadCount());

    QCoreApplication::processEvents();

    QList<GexDbThreadQuery *> lThreadsDone;
    QList<GexDbThreadQuery *> lThreadsPool;
    for(int lQueryIdx=0; lQueryIdx<croQueriesList.count(); ++lQueryIdx)
    {
        if(lThreadsPool.count() < lAllowedThreadNumber)
        {
            GexDbThreadQuery *lThread = new GexDbThreadQuery(poObj, poPluginBase, strConnectionName, croQueriesList[lQueryIdx]);
            lThreadsPool.append(lThread);
            lThread->start();
        }
        bool lIsNoMoreQuery = (lQueryIdx == (croQueriesList.count()-1));

        if((lThreadsPool.count() == lAllowedThreadNumber) || lIsNoMoreQuery)
        {
            bool lIsFreePos = false;
            while(!lIsFreePos)
            {
                QList<GexDbThreadQuery *>::iterator lIterThread = lThreadsPool.begin();
                while(lIterThread != lThreadsPool.end())
                {
                    if((*lIterThread)->isFinished())
                    {
                        if(bDeleteWhenDone)
                        {
                            delete (*lIterThread);
                            lIterThread = lThreadsPool.erase(lIterThread);
                        }
                        else
                        {
                            lThreadsDone.append((*lIterThread));
                            lIterThread = lThreadsPool.erase(lIterThread);
                        }
                        lIsFreePos = true;
                        poPluginBase->IncrementProgress((int)dStep);

                    }
                    else
                        ++lIterThread;
                    QCoreApplication::processEvents();
                }
                if( lIsNoMoreQuery && lThreadsPool.count() )
                {
                    lIsFreePos = false;
                }
                QCoreApplication::processEvents();
            }
        }
    }

    QCoreApplication::processEvents();

    return lThreadsDone;
}
