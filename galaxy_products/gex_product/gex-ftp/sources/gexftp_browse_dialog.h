#ifndef GEXFTP_BROWSE_DIALOG_H
#define GEXFTP_BROWSE_DIALOG_H

#include <QDialog>
#include <QHash>
#include <QTimer>
#include <QEventLoop>
#include "qurlinfo.h"

namespace Ui {
    class GexFtpBrowseDialog;
}

struct GexFtpBrowseDialogArgs
{
    GexFtpBrowseDialogArgs() : parent(0){}

	QWidget *parent;
	QString strCaption;
	QString strStartDirectory;
	QString strFtpSiteURL;
	int		iFtpPortNb;
	QString strLogin;
	QString	strPassword;
	int		iTimerInterval;
	bool	bAllowCustomContextMenu;
};

class GexFtpQFtp;
class  QNetworkAccessManager;
class QTreeWidgetItem;

class GexFtpBrowseDialog : public QDialog {
    Q_OBJECT
public:
	GexFtpBrowseDialog(QWidget *parent = 0);
	GexFtpBrowseDialog(QWidget *parent = 0,
						const QString	&strCaption = QString(),
						const QString	&strDir = QString(),
						const QString	&strFtpSiteURL = QString(),
						const int		&iFtpPortNb = 0,
						const QString	&strLogin = QString(),
						const QString	&strPassword = QString(),
						const int		&iTimerInterval = 5000,
						bool			bAllowCustomContextMenu = 0,
					    bool bDisplay = 1);
	GexFtpBrowseDialog(const GexFtpBrowseDialogArgs &args, bool bDisplay = 1);
	~GexFtpBrowseDialog();
	
	QString			lastError() const;
	QString			selectedDirectory() const;
	QString			loginDirectory() const;
	bool			hasTimeout();
	void			createDirectory(const QString &strDir);
	void			deleteDirectory(const QString &strDir);
	void			renameDirectory(const QString &strOldDir, const QString &strNewDir);
	
	static QString	getExistingFtpDirectory(const GexFtpBrowseDialogArgs &args);
	static bool		isValidFtpDirectory(QString &strDir, GexFtpBrowseDialogArgs &args, QString &strError);

	
public slots:
	void			onCreateDirectoryRequested();
	void			onDeleteDirectoryRequested();
	void			onRenameDirectoryRequested();


private slots:
	void			cdToParent();
	void			cdToHome();
	void			connectToServer();
	void			disconnectFromServer();
	void			ftpCommandStarted(int id );
	void			ftpCommandFinished(int, bool error);
	void			ftpRawCommandReply(int nReplyCode, const QString & strDetail);
	void			ftpCommandDone(bool error);
	void			ftpList();
	void			addToList(const QUrlInfo &urlInfo);
	void			processItem(QTreeWidgetItem *item, int /*column*/);
	void			onCustomContextMenuRequested(const QPoint &point);
	void			ftpTimeout();
	void			quitEventLoop();


private:
	void			init();
	void			initGui();
	void			execEventLoop();
	
	
    Ui::GexFtpBrowseDialog	*m_ui;
	QString					m_strCaption;				// Stores the Gui Title
	QString					m_strStartDir;				// Define if the first connection has to start in a custom dir (or in the login dir if empty)
	QString					m_strFtpSiteURL;			// Ftp url
	int						m_iFtpPortNb;				// Ftp port
	QString					m_strLogin;					// Ftp login
	QString					m_strPassword;				// Ftp password
	QString					m_strLoginDir;				// Stores the login directory (define in the Ftp user account)
	GexFtpQFtp				*m_pFtp;
	
	QTimer					*m_pTimer;
	QEventLoop				*m_pEventLoop;
	QHash<QString, bool>	m_hashIsDirectory;			// Map to link the list reply (file and dir) to their property of being a dir or not
	QString					m_strCurrentPath;			// Stores the current Ftp path
	QString					m_strErrorMsg;				// Stores the last error message
	int						m_iCurrentSequence;			// Define the current Ftp sequence 
	bool					m_bDisplay;					// Define if the Gui has to be shown or not
	bool					m_bTimeout;					// Define if the last Ftp connection has timeout(ed) or not
	bool					m_bAllowCustomContextMenu;	// Define is custom context menu(create, delete, and rename directory) is allowed or not
	int						m_iTimerInterval;			// Define in ms the time interval before timeout
};

#endif // GEXFTP_BROWSE_DIALOG_H
