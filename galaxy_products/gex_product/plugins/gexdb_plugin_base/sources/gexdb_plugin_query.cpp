#include "gexdb_plugin_base.h"

///////////////////////////////////////////////////////////
// GexDbPlugin_Query class: query execution/iteration with
// performance management...
///////////////////////////////////////////////////////////
// Constructor
GexDbPlugin_Query::GexDbPlugin_Query(GexDbPlugin_Base *pPluginBase, QSqlDatabase db) : QSqlQuery(db), m_pPluginBase(pPluginBase)
{
	m_fTimer_DbQuery = 0.0F;
	m_fTimer_DbIteration = 0.0F;
	m_fTimer_DbQuery_Cumul = 0.0F;
	m_fTimer_DbIteration_Cumul = 0.0F;

	m_ulRetrievedRows = 0;
	m_ulRetrievedRows_Cumul = 0;

#ifdef _WIN32
		LARGE_INTEGER liPerformanceFrequency;

		m_ulPerformanceFrequency = 0;
		m_liPerformanceCounter_Ref.QuadPart = 0;
		m_liPerformanceCounter_Query.QuadPart = 0;

		m_bPerformanceCounterSupported = QueryPerformanceFrequency(&liPerformanceFrequency);
	if(m_bPerformanceCounterSupported)
		m_ulPerformanceFrequency = (unsigned long)liPerformanceFrequency.QuadPart;
#else
	m_bPerformanceCounterSupported = false;
#endif
};

// Copy constructor
GexDbPlugin_Query::GexDbPlugin_Query(const GexDbPlugin_Query& source) : QSqlQuery()
{
  *this = source;
}

// Destructor
GexDbPlugin_Query::~GexDbPlugin_Query()
{
}

// Operator =
GexDbPlugin_Query& GexDbPlugin_Query::operator=(const GexDbPlugin_Query& source)
{
#ifdef _WIN32
  m_ulPerformanceFrequency=source.m_ulPerformanceFrequency;
  m_liPerformanceCounter_Ref=source.m_liPerformanceCounter_Ref;
  m_liPerformanceCounter_Query=source.m_liPerformanceCounter_Query;
#endif
  m_bPerformanceCounterSupported=source.m_bPerformanceCounterSupported;
  m_strQuery=source.m_strQuery;
  m_clQueryTimer_Ref=source.m_clQueryTimer_Ref;
  m_clQueryTimer_Query=source.m_clQueryTimer_Query;
  m_fTimer_DbQuery=source.m_fTimer_DbQuery;
  m_fTimer_DbIteration=source.m_fTimer_DbIteration;
  m_fTimer_DbQuery_Cumul=source.m_fTimer_DbQuery_Cumul;
  m_fTimer_DbIteration_Cumul=source.m_fTimer_DbIteration_Cumul;
  m_ulRetrievedRows=source.m_ulRetrievedRows;
  m_ulRetrievedRows_Cumul=source.m_ulRetrievedRows_Cumul;
  m_pPluginBase = source.m_pPluginBase;

  return *this;
}

//////////////////////////////////////////////////////////////////////
// Start internal timer
//////////////////////////////////////////////////////////////////////
void GexDbPlugin_Query::StartTimer(bool bQueryMainTimer/*=false*/)
{
#ifdef _WIN32
	if(m_bPerformanceCounterSupported)
	{
		QueryPerformanceCounter(&m_liPerformanceCounter_Ref);
		if(bQueryMainTimer)
			m_liPerformanceCounter_Query.QuadPart = m_liPerformanceCounter_Ref.QuadPart;
	}
	else
	{
		m_clQueryTimer_Ref.start();
		if(bQueryMainTimer)
			m_clQueryTimer_Query.start();
	}
#else
	m_clQueryTimer_Ref.start();
	if(bQueryMainTimer)
		m_clQueryTimer_Query.start();
#endif
}

//////////////////////////////////////////////////////////////////////
// The value().toChar() doesn't seem to work correctly, so we implement
// a workaround
//////////////////////////////////////////////////////////////////////
char GexDbPlugin_Query::GetChar(int nIndex)
{
	char	cChar = ' ';
	QString strValue = value(nIndex).toString();

	if(!strValue.isEmpty())
		cChar = strValue.toLatin1().data()[0];

	return cChar;
}

QChar GexDbPlugin_Query::GetQChar(int nIndex)
{
	QChar	cChar = QChar(' ');
	QString strValue = value(nIndex).toString();

	if(!strValue.isEmpty())
		cChar = strValue.at(0);

	return cChar;
}
