#include <QUrl>
#include <QDir>
#include <QMenu>
#include <QMessageBox>
#include <QInputDialog>
#include <QTreeWidgetItem>

#include "gexftp_qftp.h"
#include "gexftp_server.h"

#include "gexftp_browse_dialog.h"
#include "ui_gexftp_browse_dialog.h"


#define GEXFTP_SEQUENCE_INITIAL					0
#define GEXFTP_SEQUENCE_CONNECT					1
#define GEXFTP_SEQUENCE_GET_USERDIR				2
#define GEXFTP_SEQUENCE_REACH_STARTINGDIR		3
#define GEXFTP_SEQUENCE_SELECT_DIR				4
#define GEXFTP_SEQUENCE_TEST_DIR				5


/////////////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////////////
GexFtpBrowseDialog::GexFtpBrowseDialog(QWidget *parent /*= 0*/,   const QString	&strCaption /*= QString()*/,
										   const QString	&strDir /*= QString()*/, const QString	&strFtpSiteURL /*= QString()*/,
										   const int		&iFtpPortNb /*= 0*/, const QString	&strLogin /*= QString()*/,
										   const QString	&strPassword /*= QString()*/,const int		&iTimerInterval/* = 5000*/,
										   bool bAllowCustomContextMenu/* = 0*/,bool bDisplay/* = 1*/) :
	QDialog(parent), 
    m_ui(new Ui::GexFtpBrowseDialog)
{
	m_ui->setupUi(this);
	m_strCaption = strCaption;
	m_strStartDir = strDir;
	m_strFtpSiteURL = strFtpSiteURL;
	m_iFtpPortNb = iFtpPortNb;
	m_strLogin = strLogin;
	m_strPassword = strPassword;
	m_bDisplay = bDisplay;
	m_iTimerInterval = iTimerInterval;
	m_bAllowCustomContextMenu = bAllowCustomContextMenu;
	init();
}


/////////////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////////////
GexFtpBrowseDialog::GexFtpBrowseDialog(const GexFtpBrowseDialogArgs &args, bool bDisplay/* = 1*/) :
	QDialog(args.parent), 
    m_ui(new Ui::GexFtpBrowseDialog)
{
	m_ui->setupUi(this);
	m_strCaption = args.strCaption;
	m_strStartDir = args.strStartDirectory;
	m_strFtpSiteURL = args.strFtpSiteURL;
	m_iFtpPortNb = args.iFtpPortNb;
	m_strLogin = args.strLogin;
	m_strPassword = args.strPassword;
	m_iTimerInterval = args.iTimerInterval;
	m_bAllowCustomContextMenu = args.bAllowCustomContextMenu;
	m_bDisplay = bDisplay;
	init();
}


/////////////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////////////
GexFtpBrowseDialog::~GexFtpBrowseDialog()
{
	if (m_pFtp)
	{
		disconnectFromServer();
		delete m_pFtp;
	}
	
	delete m_ui;
}


/////////////////////////////////////////////////////////////////////////////////////
// Initialize the dialog
/////////////////////////////////////////////////////////////////////////////////////
void GexFtpBrowseDialog::init()
{
	// Define current sequence
	m_iCurrentSequence = GEXFTP_SEQUENCE_INITIAL;
	
	m_strLoginDir = "";
	m_strErrorMsg = "";
	
	if (!m_bDisplay)
	{
		m_pTimer = new QTimer(this);
		m_pEventLoop = new QEventLoop(this);
		m_bTimeout = false;
		connect(m_pTimer, SIGNAL(timeout()), this, SLOT(ftpTimeout()));
	}
	
	initGui();
	
	// Connect to Ftp server
	connectToServer();
	
	m_ui->treeWidgetDirList->setEnabled(true);
	// Set status
	m_ui->labelStatus->setText(tr("Connect to %1...").arg(m_strFtpSiteURL));
}

/////////////////////////////////////////////////////////////////////////////////////
// Exec event loop
/////////////////////////////////////////////////////////////////////////////////////
void GexFtpBrowseDialog::execEventLoop()
{
	m_bTimeout = false;
	m_pTimer->start(m_iTimerInterval);
	m_pEventLoop->exec(QEventLoop::AllEvents|QEventLoop::WaitForMoreEvents);
}

/////////////////////////////////////////////////////////////////////////////////////
// If we have a timeout quit event loop
/////////////////////////////////////////////////////////////////////////////////////
void GexFtpBrowseDialog::ftpTimeout()
{
	m_pEventLoop->quit();
	m_bTimeout = true;
	m_pTimer->stop();
}

/////////////////////////////////////////////////////////////////////////////////////
// Quit event loop before timeout
/////////////////////////////////////////////////////////////////////////////////////
void GexFtpBrowseDialog::quitEventLoop()
{
	m_pEventLoop->quit();
	m_pTimer->stop();
	m_bTimeout = false;
}

/////////////////////////////////////////////////////////////////////////////////////
// Initialize UI
/////////////////////////////////////////////////////////////////////////////////////
void GexFtpBrowseDialog::initGui()
{
	// Set title
	setWindowTitle(m_strCaption);
	// Clear Treewidget
	m_ui->treeWidgetDirList->clear();
	m_ui->treeWidgetDirList->sortByColumn(2,Qt::AscendingOrder);
	
	m_ui->lineEditFtpServer->setText(m_strFtpSiteURL);
	
	connect(m_ui->pushButtonCdToParent, SIGNAL(clicked()),								this, SLOT(cdToParent()));
	connect(m_ui->pushButtonHome,		SIGNAL(clicked()),								this, SLOT(cdToHome()));
	connect(m_ui->treeWidgetDirList,	SIGNAL(itemActivated(QTreeWidgetItem*,int)),	this, SLOT(processItem(QTreeWidgetItem*,int)));
	
	// If custom context menu allowed (create, rename, delete directory)
	if (m_bAllowCustomContextMenu)
		connect(m_ui->treeWidgetDirList,	SIGNAL(customContextMenuRequested(QPoint)),		this, SLOT(onCustomContextMenuRequested(QPoint)));
}


/////////////////////////////////////////////////////////////////////////////////////
// Create connection to Ftp server
/////////////////////////////////////////////////////////////////////////////////////
void GexFtpBrowseDialog::connectToServer()
{
	// Define current sequence
	m_iCurrentSequence = GEXFTP_SEQUENCE_CONNECT;
	
	m_pFtp = new GexFtpQFtp(this);
	connect(m_pFtp, SIGNAL(commandStarted(int)),			this, SLOT(ftpCommandStarted(int)));
	connect(m_pFtp, SIGNAL(commandFinished(int,bool)),		this, SLOT(ftpCommandFinished(int,bool)));
    connect(m_pFtp, SIGNAL(listInfo(QUrlInfo)),				this, SLOT(addToList(QUrlInfo)));
	connect(m_pFtp, SIGNAL(rawCommandReply(int,QString)),	this, SLOT(ftpRawCommandReply(int,QString)));
	connect(m_pFtp, SIGNAL(done(bool)),						this, SLOT(ftpCommandDone(bool)));

	// Connection
	m_pFtp->connectToHost(m_strFtpSiteURL, m_iFtpPortNb);
	// Login
	m_pFtp->login(m_strLogin, m_strPassword);
}

/////////////////////////////////////////////////////////////////////////////////////
// Disconnect to Ftp server
/////////////////////////////////////////////////////////////////////////////////////
void GexFtpBrowseDialog::disconnectFromServer()
{
	// If connection exists
	if (m_pFtp)
	{
		m_pFtp->close();
	}
}


/////////////////////////////////////////////////////////////////////////////////////
// Slot call when the command identified by id has started
/////////////////////////////////////////////////////////////////////////////////////
void GexFtpBrowseDialog::ftpCommandStarted(int /*id*/)
{
	// If the command was to list all items of the dir
	if (m_pFtp->currentCommand() == GexFtpQFtp::List)
	{
		m_ui->treeWidgetDirList->clear();
		m_hashIsDirectory.clear();
		
		return;
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Slot call when the command identified by id has finished
/////////////////////////////////////////////////////////////////////////////////////
void GexFtpBrowseDialog::ftpCommandFinished(int /*id*/, bool error)
{
	// If the command was to connect
	if (m_pFtp->currentCommand() == GexFtpQFtp::ConnectToHost) 
	{
		if (error)
		{
			m_strErrorMsg = m_pFtp->errorString();
			m_ui->labelStatus->setText(lastError());
			setEnabled(false);
			return;
		}
		// Connected
		m_ui->labelStatus->setText(tr("Connected to %1.").arg(m_strFtpSiteURL));
		m_ui->treeWidgetDirList->setFocus();
		return;
	}

	// If the command was to log
	if (m_pFtp->currentCommand() == GexFtpQFtp::Login)
	{
		if (error)
		{
			m_strErrorMsg = m_pFtp->errorString();
			m_ui->labelStatus->setText(lastError());
			setEnabled(false);
			return;
		}
		
		m_ui->labelStatus->setText(tr("Logged to %1.").arg(m_strFtpSiteURL));
		
		// Define current sequence
		m_iCurrentSequence = GEXFTP_SEQUENCE_GET_USERDIR;
		// Locate user dir
		m_pFtp->rawCommand("pwd");
		
		return;
	}

	// If the command was to move (CD)
	if (m_pFtp->currentCommand() == GexFtpQFtp::Cd)
	{
		if (error)
		{
			m_strErrorMsg = m_pFtp->errorString();
			m_ui->labelStatus->setText(lastError());
			if (m_iCurrentSequence == GEXFTP_SEQUENCE_REACH_STARTINGDIR)
			{
				m_strStartDir.clear();
				m_pFtp->rawCommand("pwd");
			}
		}
		
		m_ui->lineEditDirPath->setText(m_strCurrentPath);
		if (m_strCurrentPath != "/")
			m_ui->pushButtonCdToParent->setEnabled(true);
		m_ui->treeWidgetDirList->setFocus();
			
		return;
	}

	// If the command was to list all items of the dir
	if (m_pFtp->currentCommand() == GexFtpQFtp::List)
	{
		// If we are at root of the folder
		if (m_strCurrentPath == "/")
			m_ui->pushButtonCdToParent->setEnabled(false);
		return;
	}
	
	// If the command was to create a dir
	if (m_pFtp->currentCommand() == GexFtpQFtp::Mkdir)
	{
		if (error)
		{
			m_strErrorMsg = m_pFtp->errorString();
			m_ui->labelStatus->setText(lastError());
		}
		
		ftpList();
		
		return;
	}
	
	// If the command was to rename a dir
	if (m_pFtp->currentCommand() == GexFtpQFtp::Rename)
	{
		if (error)
		{
			m_strErrorMsg = m_pFtp->errorString();
			m_ui->labelStatus->setText(lastError());
		}
		
		ftpList();
		
		return;
	}
	
	// If the command was to delete a dir
	if (m_pFtp->currentCommand() == GexFtpQFtp::Rmdir)
	{
		if (error)
		{
			m_strErrorMsg = m_pFtp->errorString();
			m_ui->labelStatus->setText(lastError());
		}
		
		ftpList();
		
		return;
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Slot called when a list entry (FTP list command) is retrieved
/////////////////////////////////////////////////////////////////////////////////////
void GexFtpBrowseDialog::ftpRawCommandReply(int nReplyCode, const QString & strDetail)
{
	switch(nReplyCode)
	{
		case 257:
			// 257 <pwd>
			m_strCurrentPath = strDetail.section('"', 1, 1);
			if (m_iCurrentSequence == GEXFTP_SEQUENCE_GET_USERDIR)
			{
				// Define current sequence
				m_iCurrentSequence = GEXFTP_SEQUENCE_REACH_STARTINGDIR;
				m_strLoginDir = m_strCurrentPath;
				// Update user dir info
				m_ui->lineEditUserDirectory->setText(m_strLoginDir);
			}
			
			if (m_iCurrentSequence == GEXFTP_SEQUENCE_REACH_STARTINGDIR)
			{
				// If a dir was previously selected
				if (!m_strStartDir.isEmpty())
				{
					if (m_strStartDir.left(1) != "/")
						m_strStartDir = "/" + m_strStartDir;
					m_strCurrentPath = m_strStartDir;
					// Go to this dir
					m_pFtp->cd(m_strCurrentPath);
				}
				else
					// Define current sequence
					m_iCurrentSequence = GEXFTP_SEQUENCE_SELECT_DIR;

				m_ui->lineEditDirPath->setText(m_strCurrentPath);
				if (m_strCurrentPath != "/")
					m_ui->pushButtonCdToParent->setEnabled(true);
				ftpList();
			}
			break;
		default:
			break;
	}
}


/////////////////////////////////////////////////////////////////////////////////////
// Slot called when the last pending command has finished
/////////////////////////////////////////////////////////////////////////////////////
void GexFtpBrowseDialog::ftpCommandDone(bool error)
{	
	// If OK
	if (!error)
		m_ui->labelStatus->setText("Done.");
}

/////////////////////////////////////////////////////////////////////////////////////
// Launch a list only if display needed
/////////////////////////////////////////////////////////////////////////////////////
void GexFtpBrowseDialog::ftpList()
{
	if (m_bDisplay)
		m_pFtp->list();
}

/////////////////////////////////////////////////////////////////////////////////////
// Slot called for each entry found in the dir
/////////////////////////////////////////////////////////////////////////////////////
void GexFtpBrowseDialog::addToList(const QUrlInfo &urlInfo)
 {
	// Don't show
	if (urlInfo.name() == "." || !urlInfo.isDir())
		return;

	QTreeWidgetItem *item = new QTreeWidgetItem(m_ui->treeWidgetDirList);
	item->setText(0, urlInfo.name());
	// No property for parent folder
	if (urlInfo.name() != "..")
	{
		item->setText(1, QString::number(urlInfo.size()));
		item->setText(2, urlInfo.isDir() ? "dir" : "file");
		item->setText(3, urlInfo.lastModified().toString("MMM dd yyyy"));
		item->setText(4, urlInfo.owner());
		item->setText(5, urlInfo.group());
	}

	QPixmap pixmap(urlInfo.isDir() ? ":/gex-ftp/images/resources/dir.png" : ":/gex-ftp/images/resources/file.png");
	item->setIcon(0, pixmap);
	
	// Add item to found items
	m_hashIsDirectory[urlInfo.name()] = urlInfo.isDir();
	m_ui->treeWidgetDirList->addTopLevelItem(item);
	if (!m_ui->treeWidgetDirList->currentItem())
	{
		m_ui->treeWidgetDirList->setCurrentItem(m_ui->treeWidgetDirList->topLevelItem(0));
		m_ui->treeWidgetDirList->setEnabled(true);
	}
 }


/////////////////////////////////////////////////////////////////////////////////////
// Slot called when user double click a dir, to move into this dir
/////////////////////////////////////////////////////////////////////////////////////
void GexFtpBrowseDialog::processItem(QTreeWidgetItem *item, int /*column*/)
 {
	
	QString name = item->text(0);
	// If dir is ".." go to parent dir
	if (name == "..")
	{
		cdToParent();
		return;
	}
	
	// If item links a dir (not a file)
	if (m_hashIsDirectory.value(name))
	{
		m_ui->labelStatus->setText("Browse...");

		if (m_strCurrentPath != "/")
			m_strCurrentPath += "/";
		m_strCurrentPath += name;
		// Go to this dir
		m_pFtp->cd(name);
		
		ftpList();

		// Allow cd to parent dir
		m_ui->pushButtonCdToParent->setEnabled(true);

		return;
	}
 }

/////////////////////////////////////////////////////////////////////////////////////
// Slot called when user click go to parent dir button
/////////////////////////////////////////////////////////////////////////////////////
void GexFtpBrowseDialog::cdToParent()
{
	m_ui->labelStatus->setText("Browse...");
	m_strCurrentPath = m_strCurrentPath.left(m_strCurrentPath.lastIndexOf('/'));
	if (m_strCurrentPath.isEmpty())
	{
		m_ui->pushButtonCdToParent->setEnabled(false);
		m_strCurrentPath = "/";
		m_pFtp->cd(m_strCurrentPath);
	}
	else
	{
		m_pFtp->cd(m_strCurrentPath);
	}

	ftpList();
}

/////////////////////////////////////////////////////////////////////////////////////
// Slot called when user click go to home dir button
/////////////////////////////////////////////////////////////////////////////////////
void GexFtpBrowseDialog::cdToHome()
 {
	m_ui->labelStatus->setText("Browse...");
	m_strCurrentPath = m_strLoginDir;
	if (m_strCurrentPath.isEmpty())
	{
		m_strCurrentPath = "/";
		m_pFtp->cd(m_strCurrentPath);
	}
	else
	{
		m_pFtp->cd(m_strCurrentPath);
	}
	
	ftpList();
}


/////////////////////////////////////////////////////////////////////////////////////
// Load context menu
/////////////////////////////////////////////////////////////////////////////////////
void GexFtpBrowseDialog::onCustomContextMenuRequested(const QPoint &point)
{
	if (m_pFtp->hasPendingCommands())
		return;
	
	QMenu menu(this);
	
	menu.addAction("Create directory", this, SLOT(onCreateDirectoryRequested()));
	
	QTreeWidgetItem *itemSelected = m_ui->treeWidgetDirList->itemAt(point);
	
	if (itemSelected)
	{
		QString strDirName = itemSelected->text(0);
		if ((m_hashIsDirectory.value(strDirName)) && strDirName != "..")
		{
			menu.addAction("Rename", this, SLOT(onRenameDirectoryRequested()));
			menu.addAction("Delete", this, SLOT(onDeleteDirectoryRequested()));
		}
	}
	menu.exec(QCursor::pos());
}


/////////////////////////////////////////////////////////////////////////////////////
// Create Dir requested
/////////////////////////////////////////////////////////////////////////////////////
void GexFtpBrowseDialog::onCreateDirectoryRequested()
{
	bool ok;
	QString strDirName = QInputDialog::getText(this, "Create directory","Please enter the name of the directory which should be created : ", 
											   QLineEdit::Normal, "New_directory", &ok);
	if (ok && !strDirName.isEmpty())
		createDirectory(selectedDirectory() + "/" + strDirName);
}

/////////////////////////////////////////////////////////////////////////////////////
// Delete Dir requested
/////////////////////////////////////////////////////////////////////////////////////
void GexFtpBrowseDialog::onDeleteDirectoryRequested()
{
	QTreeWidgetItem *itemSelected = m_ui->treeWidgetDirList->selectedItems().at(0);
	QString strDirName = itemSelected->text(0);
	
	QMessageBox msgBox(QMessageBox::Warning, "Delete directory", "Really delete " + strDirName + " directory with its contents?", QMessageBox::Ok|QMessageBox::Cancel, this);
	if (msgBox.exec() == QMessageBox::Ok)
		deleteDirectory(selectedDirectory() + "/" + strDirName);
}

/////////////////////////////////////////////////////////////////////////////////////
// Rename Dir requested
/////////////////////////////////////////////////////////////////////////////////////
void GexFtpBrowseDialog::onRenameDirectoryRequested()
{
	bool ok;
	QTreeWidgetItem *itemSelected = m_ui->treeWidgetDirList->selectedItems().at(0);
	QString strOldDirName = itemSelected->text(0);
	
	QString strNewDirName = QInputDialog::getText(this, "Rename directory","Please enter the new name of the directory: ", 
											   QLineEdit::Normal, strOldDirName , &ok);
	if (ok && !strNewDirName.isEmpty())
		renameDirectory(selectedDirectory() + "/" + strOldDirName, selectedDirectory() + "/" + strNewDirName);
}

/////////////////////////////////////////////////////////////////////////////////////
// Try to create strDir
/////////////////////////////////////////////////////////////////////////////////////
void GexFtpBrowseDialog::createDirectory(const QString &strDir)
{
	m_pFtp->mkdir(strDir);
	m_ui->labelStatus->setText("Create " + strDir + "...");
	ftpList();
}

/////////////////////////////////////////////////////////////////////////////////////
// Try to delete strDir
/////////////////////////////////////////////////////////////////////////////////////
void GexFtpBrowseDialog::deleteDirectory(const QString &strDir)
{
	m_pFtp->rmdir(strDir);
	m_ui->labelStatus->setText("Delete " + strDir + "...");
	ftpList();
}

/////////////////////////////////////////////////////////////////////////////////////
// Try to rename strOldDir by strNewDir
/////////////////////////////////////////////////////////////////////////////////////
void GexFtpBrowseDialog::renameDirectory(const QString &strOldDir, const QString &strNewDir)
{
	m_pFtp->rename(strOldDir, strNewDir);
	m_ui->labelStatus->setText("Rename " + strOldDir + ": " + strNewDir + "...");
	ftpList();
}

/////////////////////////////////////////////////////////////////////////////////////
// Return last error message
/////////////////////////////////////////////////////////////////////////////////////
QString GexFtpBrowseDialog::lastError() const
{
	return m_strErrorMsg;
}

/////////////////////////////////////////////////////////////////////////////////////
// Return current path
/////////////////////////////////////////////////////////////////////////////////////
QString GexFtpBrowseDialog::selectedDirectory() const
{
	return m_strCurrentPath;
}

/////////////////////////////////////////////////////////////////////////////////////
// Called to select dir on Ftp server
/////////////////////////////////////////////////////////////////////////////////////
QString GexFtpBrowseDialog::getExistingFtpDirectory(const GexFtpBrowseDialogArgs &args)
{
	GexFtpBrowseDialog dialog(args);
	
	if (dialog.exec() == QDialog::Accepted)
	{
		return dialog.selectedDirectory();
	}
	
	return QString();
}

/////////////////////////////////////////////////////////////////////////////////////
// Return login directory
/////////////////////////////////////////////////////////////////////////////////////
QString GexFtpBrowseDialog::loginDirectory() const
{
	return m_strLoginDir;
}

/////////////////////////////////////////////////////////////////////////////////////
// Return true if object has timeoutted
/////////////////////////////////////////////////////////////////////////////////////
bool GexFtpBrowseDialog::hasTimeout()
{
	return m_bTimeout;
}


/////////////////////////////////////////////////////////////////////////////////////
// Return true if the dir exists. If not, try to prepend login dir, if good update
// strDir with the absolute path
/////////////////////////////////////////////////////////////////////////////////////
bool GexFtpBrowseDialog::isValidFtpDirectory(QString &strDir, GexFtpBrowseDialogArgs &args, QString &strError)
{
	// Get login dir
	args.strStartDirectory = "";
	GexFtpBrowseDialog dialog(args, false);

	connect(dialog.m_pFtp,	SIGNAL(done(bool)), &dialog, SLOT(quitEventLoop()));
	dialog.execEventLoop();
	
	// If error before search directory
	if (!dialog.lastError().isEmpty())
	{
		strError = dialog.lastError();
		return false;
	}

	if (dialog.hasTimeout())
	{
		strError = "Timeout!";
		return false;
	}

	// Move path to absolute path if needed
	if (!strDir.contains(dialog.loginDirectory()))
		strDir = QDir::cleanPath(dialog.loginDirectory() + "/" + strDir);

	dialog.m_pFtp->cd(strDir);
	dialog.execEventLoop();

	if (dialog.hasTimeout())
	{
		strError = "Timeout!";
		return false;
	}

	// If error while searching directory, 
	if (!dialog.lastError().isEmpty())
	{
		// Try absolute path without loginDir
		dialog.m_strErrorMsg = "";
		strDir = QDir::cleanPath(strDir.remove(0, dialog.loginDirectory().size()));
		dialog.m_pFtp->cd(strDir);
		dialog.execEventLoop();

		if (dialog.hasTimeout())
		{
			strError = "Timeout!";
			return false;
		}

		// If error before search directory
		if (!dialog.lastError().isEmpty())
		{
			strError = dialog.lastError();
			return false;
		}
	}

	return true;
}

