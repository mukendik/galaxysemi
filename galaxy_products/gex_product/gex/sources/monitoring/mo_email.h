///////////////////////////////////////////////////////////
// Examinator Monitoring: Send Emails
///////////////////////////////////////////////////////////

#ifndef GEXMO_EMAIL_H
#define GEXMO_EMAIL_H

#include <QScriptable>
#include <qstring.h>
#include <qstringlist.h>
#include <qcolor.h>

class GexMoSendEmail : public QObject, public QScriptable
{
    Q_OBJECT

public:
  // 'Send' (generate files in given folder) email message.
  // returns false if failed
  bool	Send(QString strEmailSpoolFolder,
             QString strFrom,
             QString strTo,
             QString strSubject,
             QString strBody,
             bool bHtmlFormat=false,
             QString strAttachments="");
};

class GexMoBuildEmailString
{
public:
	void	CreatePage(QString strTitle);
	QString	ClosePage(void);
	void	WriteTitle(QString strTitle);
	void	WriteHtmlOpenTable(int iWidth=98,int iCellSpacing=1,int iFontSize=12);
	void	WriteHtmlCloseTable(void);
  // HTML code to write one line in the table
  // if Error then background will be red
    void	WriteInfoLine(QString strLabel="", QString strData="", bool bError=false, bool bWarning=false, bool bSplitLines=true);
  // HTML code to write one line of labels in the table
	void	WriteLabelLineList(QStringList strList);
  // HTML code to write one line in the table
	void	WriteInfoLineList(QStringList strList,QString strAlign="center");
  // HTML code to write one line in the table
	void	WriteInfoLineList(QStringList strList,QList<bool> & listError, QString strAlign="center");
	void	WriteInfoLineList(QStringList strList,QList<bool> & listError, const QColor& clErrorBkgColor, QString strAlign="center");
	void	AddHtmlString(QString strString);

private:
	QString strEmailMessage;
};

#endif
