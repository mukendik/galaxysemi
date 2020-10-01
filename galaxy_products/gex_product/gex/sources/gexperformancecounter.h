#ifndef GEX_PERFORMANCE_COUNTER_H
#define GEX_PERFORMANCE_COUNTER_H

#ifdef _WIN32
  #include <windows.h>
#endif

#include <QString>
#include <QMap>

#ifdef QT_DEBUG
    #define GEX_BENCHMARK_METHOD(parameter_name)		GexMethodCallsCollector methodCallsCollector(QString(Q_FUNC_INFO), parameter_name);
    #define GEX_BENCHMARK_DUMP(parameter_name)          GexMethodCallsCollector::dump();
#else
    #define GEX_BENCHMARK_METHOD(parameter_name)
    #define GEX_BENCHMARK_DUMP(parameter_name)
#endif

/*class timer
{

    public:
    timer():_start_time(0),nbIteration(0), total(0.), mean(0.), started(false) { }


    bool    isStarted() { return started;}
    void    start() { _start_time = clock();}

    double elapsed() const
    {
        return double(clock() - _start_time) / CLOCKS_PER_SEC;
    }

    void update();

    void flush(const QString& key);

private:
    clock_t _start_time;
    int      nbIteration;
    double   total;
    double   mean;
    bool     started;
};*/


class GexPerformanceCounter
{

public:

    GexPerformanceCounter(bool bAutoStart = false)
    {
#ifdef _WIN32
        m_liStart.QuadPart		= 0;
        m_liFrequency.QuadPart	= 0;
#endif // _WIN32

        if (bAutoStart)
            start();
    }

    ~GexPerformanceCounter()
    {
    }

    void start()
    {
#ifdef _WIN32
        ::QueryPerformanceCounter(&m_liStart);
        ::QueryPerformanceFrequency(&m_liFrequency);
#endif // _WIN32
    }

    int elapsedTime() const
    {
        int nElapsed = -1;

#ifdef _WIN32
        if (m_liStart.QuadPart == 0)
            return nElapsed;

        LARGE_INTEGER liStop;

        ::QueryPerformanceCounter(&liStop);

        liStop.QuadPart -= m_liStart.QuadPart;
        liStop.QuadPart *= 1000000;
        liStop.QuadPart /= m_liFrequency.QuadPart;

        if (liStop.HighPart != 0)
            nElapsed = -1;
        else
            nElapsed = liStop.LowPart;
#endif // _WIN32

        return nElapsed;
    }


private:

#ifdef _WIN32
    LARGE_INTEGER		m_liStart;					// Used to hold the system's performance counter value when internal timer started
    LARGE_INTEGER		m_liFrequency;				// Used to hold the system's performance counter frequency (counts per sec);
#endif // _WIN32

};

class GexMethodCallsCollector
{
public:

    GexMethodCallsCollector(const QString& strMethod, const QString& strParameter);
    ~GexMethodCallsCollector();

    static void dump();

private:

    QString							m_strMethod;
    QString							m_strParameter;
    GexPerformanceCounter			m_perfCounter;
};

#endif // GEX_PERFORMANCE_COUNTER_H
