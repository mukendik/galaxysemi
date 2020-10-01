/****************************************************************************
** Deriven from ftp_transfer_dialogbase.h
****************************************************************************/

#ifndef GEX_FTP_TRANSFER_DIALOG_H
#define GEX_FTP_TRANSFER_DIALOG_H

// QT includes
#include <qpixmap.h>
#include "qurlinfo.h"
#include <qdatetime.h>
#include <qfile.h>
#include <qftp.h>

// Galaxy modules includes
#include <gstdl_errormgr.h>
#include <gqtl_skin.h>
#include "gexdb_plugin_datafile.h"
#include <QNetworkAccessManager>

// Local includes
#include "ui_gexdb_plugin_filetransfer_dialog.h"

//class QtFtp ;
class QTimer;
class QWidget;

class GexDbFtpTransferDialog : public QDialog, public Ui::GexDbFtpTransferDialogBase
{
	Q_OBJECT
		
public:
	GDECLARE_ERROR_MAP(GexDbFtpTransferDialog)
	{
		eFtpError,				// Error from Ftp command
		eCreateLocalFile,		// Couldn't create local file
		eRemoveLocalFile,		// Couldn't remove local file
		eOperationAborted,		// Abort
		eFtpConnectionBroken	// Connection lost
	}
	GDECLARE_END_ERROR_MAP(GexDbFtpTransferDialog)

    GexDbFtpTransferDialog(const QString & strApplicationPath,
                           const QString & strLocalFtpDirectory,
                           tdGexDbPluginDataFileList *pFilesToTransfer,
                           CGexSkin * pGexSkin,
                           QWidget* parent = 0,
                           Qt::WindowFlags fl = 0);
    ~GexDbFtpTransferDialog();

	bool	Start();							// Returns 'true' if ok for the '.exec()'
	void	GetLastError(QString &strError);	// Returns details about last error

protected:
	void	UpdateTransferStatus(int nInfoCode);
	void	ResolveHost();
	void	Cleanup();
	void	CreateFtpObject();
	void	DeleteFtpObject();

protected:
	enum InfoCode
	{
		iStateUnconnected,
		iStateHostLookup,
		iStateConnecting,
		iStateConnected,
		iStateLoggedIn,
		iStateClosing,
		iSequenceConnect,
		iSequenceLogin,
		iSequenceChdir,
		iSequenceGet,
		iSequenceNext,
		iSequenceCleanup,
		iCommandStarted,
		iCommandFinishedOK,
		iCommandFinishedNOK
	};

	QFtp						*m_pclFtp;						// Ptr on QT FTP object
	QString						m_strLocalFtpDirectory;			// Local directory for file transfer
	QString						m_strFileInTransferLocalName;	// Name of the file (on local system) currently being transferred (put/get)
	QString						m_strFileInTransferRemoteName;	// Name of the file (on remote system) currently being transferred (put/get)
	QString						m_strFinalLocalName;			// The file in transfer should be renamed to this name after transfer OK
	tdGexDbPluginDataFileList*	m_plistFilesToTransfer;		// List of files to be transferred
	GexDbPlugin_DataFile		*m_pCurrentFile;				// File currently under transfer
	QTreeWidgetItem				*m_pCurrentItemInTreeWidget;		// Item in tree widget for current file
	QString						m_strFtpSiteName;				// Domain name of current Ftp server
	QString						m_strFtpSiteIP;					// IP of Ftp server
	QString						m_strFtpServer;					// String to use to connect to Ftp server (IP if available, domain name else)
	int							m_nCurrentSequence;				// Current task in Ftp sequence
	QPixmap						m_pxTransferred;				// Pixmap for files that have been transferred
	QPixmap						m_pxNotTransferred;				// Pixmap for files that have not been transferred
	QPixmap						m_pxTransferError;				// Pixmap for files with transfer errors
	QPixmap						m_pxTransferring;				// Pixmap for files in transfer progress
	QTimer						*m_pTimer;
    bool						m_bTransferDone;				// Set to true when transfer is completed, and dialog should exit during next timer
    bool						m_bDetailsVisible;				// Set to true if Details are visible in dialog box
	QFile						m_clCurrentFileHandle;			// Handle on file being transferred

protected slots:
	void	ftp_sequencer();
	void	ftp_sequence_connect();
	void	ftp_sequence_chdir();
	void	ftp_sequence_get();
    void	ftp_sequence_close();
	void	ftp_sequence_nextfile();
    void	ftp_commandStarted(int id);
	void	ftp_stateChanged(int state);
	void	ftp_commandFinished(int id, bool error);
    void	ftp_done(bool error);
    void	OnButtonAbort();
    void	OnButtonDetails();
	void	OnTransferProgress(qint64, qint64);
	void	OnTimer();

signals:

    void sNextSequence();
};

#endif // GEX_FTP_TRANSFER_DIALOG_H
