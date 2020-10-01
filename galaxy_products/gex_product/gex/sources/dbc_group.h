#ifndef DBC_GROUP_H
#define DBC_GROUP_H

#include <QObject>

class DbcGroup : public QObject
{
    Q_OBJECT
public:
    DbcGroup(QObject *parent = 0);
	int			groupId();
	QString		groupName();
	QList<int>	parameterList();
	
	void		setGroupId(int iValue);
	void		setGroupName(QString strValue);
	void		addParameter(int iValue);
	void		removeParameter(int iValue);

signals:

public slots:
	
private:
	int			m_iGroupId;
	QString		m_strGroupName;
	QList<int>	m_listParameters;

};

#endif // DBC_GROUP_H
