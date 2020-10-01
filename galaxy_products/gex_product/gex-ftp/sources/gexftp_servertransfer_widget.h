#ifndef CGexFtpServerTransfer_widget_H
#define CGexFtpServerTransfer_widget_H

///////////////////////////////////////////////////////////////////////////////////
// GEX-FTP Includes
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////////////////////////////
#include <qpixmap.h>
#include <qdatetime.h>
#include "qurlinfo.h"

///////////////////////////////////////////////////////////////////////////////////
// GUI Includes
///////////////////////////////////////////////////////////////////////////////////
#include "ui_gexftp_servertransfer_widget_base.h"

class QApplication;

class CGexFtpServerTransfer_widget : public QWidget, public Ui::CGexFtpServerTransfer_widget_base
{

	Q_OBJECT
		
public:

    CGexFtpServerTransfer_widget(QApplication* qApplication, const QString & strApplicationPath, const QString& strSettingsName, QWidget* parent = 0, Qt::WindowFlags fl = 0);
    ~CGexFtpServerTransfer_widget();

protected:
	
	QApplication *			m_qApplication;					// Ptr on QT application object
	QPixmap					m_pxInfo;						// Pixmap for information messages
	QPixmap					m_pxError;						// Pixmap for error messages
	QPixmap					m_pxTransferred;				// Pixmap for files that have been transferred
	QPixmap					m_pxNotTransferred;				// Pixmap for files that have not been transferred
	QPixmap					m_pxTransferError;				// Pixmap for files with transfer errors
	QPixmap					m_pxTransferring;				// Pixmap for the file that is currently being transferred

public slots:
	
	void					onLogMessage(int nMsgType, const QString& strMsg);

protected slots:

    void					OnButtonAbort();
	void					onChangeTimeout(int nTimeout);
    void					onDownloadProgress(int nTransferred, int nTotalToTransfer);
	void					onListFileInfo(int, int);
	void					onDownloadInfo(int, int, int);
	void					onTransferStatus(int, const QString&, bool);
	void					onStartFileTransfer(const QString& strFileName, int nNbFilesProcessed, int nNbFileToTransfer);
	void					onCleanup();
	void					onFileInfo(const QUrlInfo& urlInfo);
	void					onMaxFileToDownLoadInfo(int iMaxFileToTransfer);

signals:
	
	void					sUserAbort();
};

#endif // CGexFtpServerTransfer_widget_H
