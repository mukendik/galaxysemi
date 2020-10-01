#include "dbc_group.h"


DbcGroup::DbcGroup(QObject *parent) :
    QObject(parent)
{
}

int DbcGroup::groupId()
{
	return m_iGroupId;
}

QString DbcGroup::groupName()
{
	return m_strGroupName;
}

QList<int> DbcGroup::parameterList()
{
	return m_listParameters;
}

void DbcGroup::setGroupId(int iValue)
{
	m_iGroupId = iValue;
}

void DbcGroup::setGroupName(QString strValue)
{
	m_strGroupName = strValue;
}

void DbcGroup::addParameter(int iValue)
{
	if(!m_listParameters.contains(iValue))
		m_listParameters.append(iValue);
}

void DbcGroup::removeParameter(int iValue)
{
	if(m_listParameters.contains(iValue))
		m_listParameters.remove(iValue);
}

