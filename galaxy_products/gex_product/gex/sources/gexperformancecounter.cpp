///////////////////////////////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////////////////////////////
#include "gexperformancecounter.h"
#include <QFile>
#include <QDir>
#include <QDate>
#include <gqtl_log.h>

/*void timer::update()
{
     ++nbIteration;
    double lLast = double(clock() - _start_time) / CLOCKS_PER_SEC;
    total += lLast;
    mean = total/nbIteration;

    //GSLOG(SYSLOG_SEV_EMERGENCY, QString("%1 %2 (%3) [%4]").arg(msg.c_str()).arg(lLast).arg(mean).arg(total).toLatin1().constData());
}

void timer::flush(const QString& key)
{
    QString msg = key;
    msg += QString("- Mean: (%1)  Total: [%2]").arg(mean).arg(total);
    GSLOG(SYSLOG_SEV_EMERGENCY, msg.toLatin1().constData());
}*/

class GexMethodCallsManager
{
private:

    class CallHistory
    {
    public:

        CallHistory(const QString& strName)
        {
            m_strName		= strName;
            m_nCount		= 0;
            m_nDuration		= 0;
        }

        const QString&	name() const				{ return m_strName; }
        int				count() const				{ return m_nCount; }
        int				duration() const			{ return m_nDuration; }

        void			increaseCall()				{ m_nCount++; }
        void			addDuration(int nDuration)	{ m_nDuration += nDuration; }

        bool			operator<(const CallHistory& otherCall) const
        {
            return m_nCount > otherCall.m_nCount;
        }

    private:

        QString			m_strName;
        int				m_nCount;
        int				m_nDuration;
    };

    QMap<QString, QList<CallHistory> >				m_mapKeyCalls;

public:

    GexMethodCallsManager();
    ~GexMethodCallsManager();

    void		addMethodCall(const QString& strMethod, const QString& strParameter, int nDuration);
    void        dump() const;
};

GexMethodCallsManager	gMethodCallsManager;

GexMethodCallsManager::GexMethodCallsManager()
{
}

GexMethodCallsManager::~GexMethodCallsManager()
{
}

void GexMethodCallsManager::dump() const
{
    if (m_mapKeyCalls.count() == 0)
        return;

    QFile lOutputFile(QDir::homePath() + "/GalaxySemi/temp/MethodCalls.txt");
    if (!lOutputFile.open(QIODevice::WriteOnly))
    {
        #ifdef QT_DEBUG
            qDebug("MethodCallsManager dump : failed to write in '%s'", lOutputFile.fileName().toLatin1().data());
        #endif
        return;
    }
    QMap<QString, QList<CallHistory> >::const_iterator itBegin = m_mapKeyCalls.begin();

    #ifdef QT_DEBUG
        qDebug("MethodCallsManager: outputting report in '%s'\n", lOutputFile.fileName().toLatin1().data());
    #endif
    lOutputFile.write(QDateTime::currentDateTime().toString().toLatin1());
    lOutputFile.write("\n");

    for (; itBegin != m_mapKeyCalls.end(); ++itBegin)
    {
        QList<CallHistory> lstCallers = itBegin.value();

        // Sort callers by calls count
        qSort(lstCallers);

        lOutputFile.write(QString("* %1 is called:\n").arg(itBegin.key()).toLatin1());

        for (int nCaller = 0; nCaller < lstCallers.count(); nCaller++)
        {
            lOutputFile.write(QString("\t%1 times \t(%2 ms) \twith argument %3\n")
                   .arg(lstCallers.at(nCaller).count())
                   .arg(lstCallers.at(nCaller).duration()/1000)
                   .arg(lstCallers.at(nCaller).name())
                   .toLatin1());
        }
    }
    lOutputFile.close();
}

void GexMethodCallsManager::addMethodCall(const QString &strMethod, const QString &strParameter, int nDuration)
{
    if (m_mapKeyCalls.contains(strMethod))
    {
        bool bFindCaller = false;

        QList<CallHistory>& lstCallers = m_mapKeyCalls[strMethod];

        for (int nCaller = 0; nCaller < lstCallers.count() && bFindCaller == false; nCaller++)
        {
            if (lstCallers.at(nCaller).name() == strParameter)
            {
                lstCallers[nCaller].addDuration(nDuration);
                lstCallers[nCaller].increaseCall();

                bFindCaller = true;
            }
        }

        if (bFindCaller == false)
        {
            CallHistory caller(strParameter);
            caller.addDuration(nDuration);
            caller.increaseCall();

            lstCallers.append(caller);
        }
    }
    else
    {
        CallHistory caller(strParameter);
        caller.addDuration(nDuration);
        caller.increaseCall();

        QList<CallHistory> lstCallers;
        lstCallers.append(caller);

        m_mapKeyCalls.insert(strMethod, lstCallers);
    }
}

GexMethodCallsCollector::GexMethodCallsCollector(const QString &strMethod, const QString &strParameter)
    : m_strMethod(strMethod), m_strParameter(strParameter), m_perfCounter(true)
{
}

GexMethodCallsCollector::~GexMethodCallsCollector()
{
    int nElapsedTime = m_perfCounter.elapsedTime();
    gMethodCallsManager.addMethodCall(m_strMethod, m_strParameter, nElapsedTime);
}

void GexMethodCallsCollector::dump()
{
    gMethodCallsManager.dump();
}
