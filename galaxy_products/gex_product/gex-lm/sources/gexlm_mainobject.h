///////////////////////////////////////////////////////////
// GEX-LM main object header file
///////////////////////////////////////////////////////////
#ifndef GEXLM_MAINOBJECT_H
#define GEXLM_MAINOBJECT_H

#include <QObject>
#include <QDateTime>

class GexLicenseManager : public QObject
{
    Q_OBJECT

public:
	GexLicenseManager(int nPortNumber);
	~GexLicenseManager();

	static	void		checkTimeStamp();
	static	void		writeTimeStamp(const QDateTime& dateTimeStamp);
	static  QDateTime	readTimeStamp();

private:
	QString	ReadLineFromSocket(void);

public slots:
	void	timerDone();
};

#endif // ifdef GEXLM_MAINOBJECT_H
