#ifndef DBC_STEP_H
#define DBC_STEP_H

#include <QObject>

class DbcStep : public QObject
{
	Q_OBJECT
public:
	DbcStep(QObject *parent = 0);
	DbcStep(const DbcStep& step);
	DbcStep operator=(const DbcStep& step);
	
	virtual ~DbcStep();
	int id() {return m_nId;}
	QString sessionId() {return m_strSessionId;}
	QString fileId() {return m_strFileId;}
	void setId(int nId);	
	void setSessionId(const QString & strSessionId);
	void setFileId(const QString& strFileId);

private:
	int m_nId;
	QString m_strSessionId;
	QString m_strFileId;
};

#endif // DBC_STEP_H
