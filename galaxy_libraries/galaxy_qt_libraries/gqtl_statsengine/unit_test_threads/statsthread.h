#ifndef STATSTHREAD_H
#define STATSTHREAD_H

#include <QtCore>
#include "stats_engine.h"

class StatsThread : public QThread
{
public:
    StatsThread(const QString& appDir, int threadId);
    double **readCSVFile(QStringList &testsHeader,
                         QStringList &partsHeader,
                         const QString& inputFile);
    bool    runAnalysis(GS::SE::StatsEngine *engine,
                                 double **dataFrame,
                                 const QStringList& testsHeader,
                                 const QStringList& partsHeader);
    bool    isThreadOk() const;
private:
    void    run();
    QString mAppDir;
    int     mThreadId;
    bool    mIsThreadOK;
};



#endif // STATSTHREAD_H
