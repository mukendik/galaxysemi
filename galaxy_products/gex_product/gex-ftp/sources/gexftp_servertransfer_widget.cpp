/****************************************************************************
** Deriven from gexftp_servertransfer_widget_base.cpp
****************************************************************************/

#include "gexftp_servertransfer_widget.h"
#include "gexftp_constants.h"
#include "gexftp_settings.h"

#include <gqtl_sysutils.h>


/////////////////////////////////////////////////////////////////////////////////////
// Constructs a CGexFtpServerTransfer_widget as a child of 'parent', with the
// name 'name' and widget flags set to 'f'.
/////////////////////////////////////////////////////////////////////////////////////
CGexFtpServerTransfer_widget::CGexFtpServerTransfer_widget(QApplication* qApplication, const QString & strApplicationPath, const QString& strSettingsName, QWidget* parent, Qt::WindowFlags fl)
    : QWidget(parent, fl)
{
	// Setup UI
	setupUi(this);

	QString		strPixMapSource;
	QStringList	strLstHeader;

	// Init private members
	m_qApplication	= qApplication;
	
	strPixMapSource	= strApplicationPath + "/images/gexftp_info.png";
	CGexSystemUtils::NormalizePath(strPixMapSource);
	m_pxInfo = QPixmap(strPixMapSource);
	
	strPixMapSource	= strApplicationPath + "/images/gexftp_error.png";
	CGexSystemUtils::NormalizePath(strPixMapSource);
	m_pxError = QPixmap(strPixMapSource);
	
	strPixMapSource	= strApplicationPath + "/images/gexftp_transferred.png";
	CGexSystemUtils::NormalizePath(strPixMapSource);
	m_pxTransferred = QPixmap(strPixMapSource);
	
	strPixMapSource	= strApplicationPath + "/images/gexftp_nottransferred.png";
	CGexSystemUtils::NormalizePath(strPixMapSource);
	m_pxNotTransferred = QPixmap(strPixMapSource);
	
	strPixMapSource	= strApplicationPath + "/images/gexftp_transfererror.png";
	CGexSystemUtils::NormalizePath(strPixMapSource);
	m_pxTransferError = QPixmap(strPixMapSource);
	
	strPixMapSource	= strApplicationPath + "/images/gexftp_transferring.png";
	CGexSystemUtils::NormalizePath(strPixMapSource);
	m_pxTransferring = QPixmap(strPixMapSource);
	
	// Clear file counter label
	labelFileCounter->setText("");

	// Set profile name
	labelProfileNameValue->setText(strSettingsName);

	// Hide column titles of Log view, and disable	sorting
	strLstHeader << "I" << "Message";
	treeWidgetLog->setHeaderLabels(strLstHeader);
	treeWidgetLog->setHeaderHidden(true);
	treeWidgetLog->setColumnWidth(0, 20);

	// DIsable sorting on file Files view
	strLstHeader.clear();
	strLstHeader << "File" << "Size" << "Last modified" << "Transfer status";
	treeWidgetFiles->setHeaderLabels(strLstHeader);
	treeWidgetFiles->setSortingEnabled(false);

	// defaultly hide max number of file to download
	labelMaxFileToDownload->hide();

	// signals and slots connections
    connect( buttonAbort,	SIGNAL( clicked() ),					this, SLOT( OnButtonAbort() ) );
}

/////////////////////////////////////////////////////////////////////////////////////
// Destroys the object and frees any allocated resources
/////////////////////////////////////////////////////////////////////////////////////
CGexFtpServerTransfer_widget::~CGexFtpServerTransfer_widget()
{
}

/////////////////////////////////////////////////////////////////////////////////////
// Add to log listview
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpServerTransfer_widget::onLogMessage(int nMsgType, const QString& strMsg)
{
	// Add to list view
	QTreeWidgetItem * pItem = new QTreeWidgetItem(treeWidgetLog);
	pItem->setText(1, strMsg);
	
	switch(nMsgType)
	{
		case GEXFTP_MSG_INFO_SEQ:
			pItem->setIcon(0, m_pxInfo);
			break;
		case GEXFTP_MSG_ERROR:
			pItem->setIcon(0, m_pxError);
			break;
		default:
			break;
	}

	treeWidgetLog->setCurrentItem(pItem);
}

/////////////////////////////////////////////////////////////////////////////////////
// Abort button has been pressed
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpServerTransfer_widget::OnButtonAbort()
{
	// Abort
	emit sUserAbort();
}

/////////////////////////////////////////////////////////////////////////////////////
// Timeout remaining has changed
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpServerTransfer_widget::onChangeTimeout(int nTimeout)
{
	if (nTimeout >= 0)
		buttonAbort->setText("Abort\n(Timeout: " + QString::number(nTimeout) + "\")");
	else
		buttonAbort->setText("Abort\n!!");;
}

/////////////////////////////////////////////////////////////////////////////////////
// Data transferred has changed
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpServerTransfer_widget::onDownloadProgress(int nTransferred, int nTotalToTransfer)
{
    if (nTransferred == 0 && nTotalToTransfer == 0)
		progressBarTransfer->reset();
	else
	{
        if (progressBarTransfer->maximum() != nTotalToTransfer)
            progressBarTransfer->setMaximum(nTotalToTransfer);

        progressBarTransfer->setValue(nTransferred);
	}

	// To make sure we catch the signal if 'Abort' button is pressed
	m_qApplication->processEvents();	
}

/////////////////////////////////////////////////////////////////////////////////////
// Display information about the list of the files to transfer
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpServerTransfer_widget::onListFileInfo(int nNbTotalFileToTransfer, int nNbTotalFile)
{
	QString strFileCounter;
	
	strFileCounter = "Matching files: " + QString::number(nNbTotalFileToTransfer); 
	strFileCounter += "/" + QString::number(nNbTotalFile);
	labelFileCounter->setText(strFileCounter);
}

/////////////////////////////////////////////////////////////////////////////////////
// Display information about download process
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpServerTransfer_widget::onDownloadInfo(int nDownloaded, int nTotal, int nSpeed)
{
		QString			strTemp;
		strTemp = QString::number(std::max(0,nDownloaded)) + " / " + QString::number(std::max(0,nTotal)) + " bytes";
		labelDownloaded->setText(strTemp);
		
		strTemp = QString::number(std::max(0,nSpeed)) + " kb/sec";
		labelSpeed->setText(strTemp);
}

/////////////////////////////////////////////////////////////////////////////////////
// Update the transfer file status
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpServerTransfer_widget::onTransferStatus(int nStatus, const QString& strFile, bool bStart)
{
			// Change transfer status for file in listview
		QList<QTreeWidgetItem *> lstWidgetItem = treeWidgetFiles->findItems(strFile, 0);
		QList<QTreeWidgetItem *>::iterator itWidgetItem = lstWidgetItem.begin();

		while (itWidgetItem != lstWidgetItem.end())
		{
			QPixmap pxStatus;
			QString strStatus;

			switch (nStatus)
			{
				case GEXFTP_FILESTATUS_ERROR		:	strStatus	= "Transfer error";
														pxStatus	= m_pxTransferError;
														break;

				case GEXFTP_FILESTATUS_TRANSFERRED	:	strStatus	= "Transferred";
														pxStatus	= m_pxTransferred;
														break;

				case GEXFTP_FILESTATUS_TRANSFERRING	:	strStatus	= "Transferring...";
														pxStatus	= m_pxTransferring;
														break;
			}
			
			(*itWidgetItem)->setText(3, strStatus);
			(*itWidgetItem)->setIcon(3, pxStatus);
			
			if (bStart)
				treeWidgetFiles->setCurrentItem(*itWidgetItem);
			
			itWidgetItem++;
		}
}

/////////////////////////////////////////////////////////////////////////////////////
// Display the file which is going to transfer
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpServerTransfer_widget::onStartFileTransfer(const QString& strFileName, int nNbFilesProcessed, int nNbFileToTransfer)
{
	QString strTemp;

	// Update download status controls
	labelFileName->setText(strFileName);
	
	strTemp.sprintf("Processing file %d/%d", nNbFilesProcessed, nNbFileToTransfer);
	labelFileCounter->setText(strTemp);
}

/////////////////////////////////////////////////////////////////////////////////////
// Unable abort button when ftp transfer is stopped
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpServerTransfer_widget::onCleanup()
{
	buttonAbort->setEnabled(false);
}

/////////////////////////////////////////////////////////////////////////////////////
// Add new file and information about it in the file view
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpServerTransfer_widget::onFileInfo(const QUrlInfo& urlInfo)
{
		QTreeWidgetItem * 	pclItem;
		QString				strFileSize = QString::number(urlInfo.size()) + " bytes";
		
		pclItem = new QTreeWidgetItem(treeWidgetFiles);
		pclItem->setText(0, urlInfo.name().section('/',-1,-1));
		pclItem->setText(1, strFileSize);
		pclItem->setText(2, urlInfo.lastModified().toString());
		pclItem->setText(3, "Not transferred");
		pclItem->setIcon(3, m_pxNotTransferred);
}


/////////////////////////////////////////////////////////////////////////////////////
// Show/Hide Max. nb. of files to download per profile
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpServerTransfer_widget::onMaxFileToDownLoadInfo(int iMaxFileToTransfer)
{
	if (iMaxFileToTransfer <= 0)
	{
		labelMaxFileToDownload->hide();
	}
	else
	{
		labelMaxFileToDownload->setText(QString("Max. nb. of files to download per profile: %1").arg(QString::number(iMaxFileToTransfer)));
		labelMaxFileToDownload->show();
	}
}

