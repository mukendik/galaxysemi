#include <QStringList>
#include "gexlogthread.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif

QStringList s_LevelsStrings = (QStringList()
							   << "EMERG"<<"ALERT"
							   << "CRITIC" << "ERROR"
							   << "WARN" << "NOTICE"
							   << "INFO" << "DEBUG" );

QString _log_to_console(int sev, const char* file, const char* m, const char* func, QString module)
{
	if (sev<0)
		sev=0;

#ifdef _WIN32
	/*
	 //GetConsoleScreenBufferInfo();
	SetConsoleTextAttribute(
		__in  HANDLE hConsoleOutput,
		__in  WORD wAttributes
	);
	*/
	HANDLE hConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	int c=0;
	switch (sev)
	{
		case 0: case 1: c=FOREGROUND_RED | FOREGROUND_INTENSITY;	break;
		case 2: case 3:	c=FOREGROUND_RED;	break;
		case 4: c=FOREGROUND_GREEN | FOREGROUND_INTENSITY; break;	// WARNING
		case 5: c=FOREGROUND_GREEN; break;			//
		case 6: c=FOREGROUND_BLUE | FOREGROUND_INTENSITY;	break;	// Info
		case 7: c=FOREGROUND_BLUE; break;			// Debug
	}
	/*
	#define FOREGROUND_BLUE	1
	#define FOREGROUND_GREEN	2
	#define FOREGROUND_RED	4
	#define FOREGROUND_INTENSITY	8

	#define BACKGROUND_BLUE	16
	#define BACKGROUND_GREEN	32
	#define BACKGROUND_RED	64
	#define BACKGROUND_INTENSITY	128
	*/
	BOOL b=SetConsoleTextAttribute(hConsoleHandle, c);
	if (!b)
		SetConsoleTextAttribute(hConsoleHandle, 0);

  //qDebug or printf ?
  printf("%s %s %s: %s, in %s\n",
		sev>=s_LevelsStrings.size()?"????\t":QString(s_LevelsStrings.at(sev)+"\t").toLatin1().data(),
		module.isEmpty()?"?":module.toLatin1().data(),
		func?QString(func).section('(',0,0).toLatin1().data():"?",
		QString(m).replace('\n',' ').toLatin1().data(),
		file?file:"?"
		);

	SetConsoleTextAttribute(hConsoleHandle, 7);
#else
//#ifdef __linux__
	//int c
	//const char *const green = "\033[0;40;32m";
	//const char *const normal = "\033[0m";
	//printf("%sHello World%s\n", green, normal);

    printf("\033[3%dm %s %s %s: %s, in %s \n", sev,
           sev>s_LevelsStrings.size()?"????\t":QString(s_LevelsStrings.at(sev)+"\t").toLatin1().data(),
           module.isEmpty()?"?":module.toLatin1().data(),
           func?QString(func).section('(',0,0).toLatin1().data():"?",
           QString(m).replace('\n',' ').toLatin1().data(),
           file?file:"?"
                 );
	// fflush(stdout); // Confirm me !
#endif

	return "ok";
}

bool CConsoleOutput::PopFront()
{
	if (m_buffer.size()==0)
		return "ok";
	SMessage m=m_buffer.takeLast();	// takeFirst ?
	_log_to_console(m.m_sev,
					m.m_atts["file"].toLatin1().constData(),
					m.m_atts["msg"].toLatin1().constData(),
					m.m_atts["func"].toLatin1().constData(),
					m.m_atts["module"].toLatin1().constData()
					);
    return true;
}
