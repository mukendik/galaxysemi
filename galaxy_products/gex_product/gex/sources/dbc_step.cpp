#include "dbc_step.h"


DbcStep::DbcStep(QObject *parent) :
		QObject(parent)
{
	m_nId = 0;
	m_strSessionId = "";
	m_strFileId = "";
}

/////////////////////////////////////////////////////////////////////
// 
/////////////////////////////////////////////////////////////////////
DbcStep::DbcStep(const DbcStep& step) : QObject(step.parent())
{
	m_nId = step.m_nId;
	m_strSessionId = step.m_strSessionId;
	m_strFileId = step.m_strFileId;
}

/////////////////////////////////////////////////////////////////////
// 
/////////////////////////////////////////////////////////////////////
DbcStep DbcStep::operator=(const DbcStep& step)
{
	m_nId = step.m_nId;
	m_strSessionId = step.m_strSessionId;
	m_strFileId = step.m_strFileId;

	return *this;
}


/////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////
DbcStep::~DbcStep()
{
	
}

/////////////////////////////////////////////////////////////////////
// 
/////////////////////////////////////////////////////////////////////
void DbcStep::setId(int nId)
{
	m_nId = nId;
}

/////////////////////////////////////////////////////////////////////
// 
/////////////////////////////////////////////////////////////////////
void DbcStep::setSessionId(const QString & strSessionId)
{
	m_strSessionId = strSessionId;
}

/////////////////////////////////////////////////////////////////////
// 
/////////////////////////////////////////////////////////////////////
void DbcStep::setFileId(const QString& strFileId)
{
	m_strFileId = strFileId;
}


