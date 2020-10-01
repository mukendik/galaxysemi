#ifndef GEXDBTHREADQUERY_H
#define GEXDBTHREADQUERY_H

#include <QString>
#include <QThread>

class GexDbPlugin_Base;
class GexDbPlugin_Query;

class GexDbThreadQuery : public QThread
{
public:
    GexDbThreadQuery(QObject* poObj, GexDbPlugin_Base *poPluginBase, const QString &strConnectionName, const QString &strQuery);
    virtual ~GexDbThreadQuery();

    bool getQueryStatus();

    void setQuery(const QString &strQuery);

    QString getQuery();

    GexDbPlugin_Query *getQueryResult();

    static QList<GexDbThreadQuery *> executeQueries(QObject* poObj,
                                                    GexDbPlugin_Base *poPluginBase,
                                                    const QString &strConnectionName,
                                                    const QStringList &croQueriesList,
                                                    int iAskedThreadNumber=10,
                                                    double dStep=0,
                                                    bool bDeleteWhenDone = false);

private:
    void run();

private:
    bool                 mQueryStatus;
    QString              mQuery;
    QString              mConnectionName;
    GexDbPlugin_Query    *mGexDbQuery;
    static int           mThreadCount;
};

#endif // GEXDBTHREADQUERY_H
