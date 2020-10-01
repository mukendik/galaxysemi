
#include <QDir>
#include <QUrl>
#include <QMenu>
#include <QString>
#include <QVariant>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QDirIterator>
#include <QInputDialog>
#include <QMessageBox>
#include <QDragEnterEvent>

#include "report_options.h"
#include "gex_constants.h"
#include "db_sqlite_man.h"
#include "dbc_transaction.h"
#include "dbc_dataset_editor.h"
#include "import_all.h"
#include "dbc_stdf_reader.h"
#include <gqtl_log.h>
#include "message.h"
#include "dbc_admin.h"
#include "ui_dbc_admin.h"

// report_build.cpp
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)

// External definitions for pointers to pixmaps.
#include "gex_pixmap_extern.h"




///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
DbcAdmin::DbcAdmin(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::DbcAdmin)
{
	m_ui->setupUi(this);
    m_strGexDbcTemplatePath = QApplication::applicationDirPath() + "/" + DBC_TEMPLATE_PATH;
    m_strXmlFiltersTemplatePath = QApplication::applicationDirPath() + "/plugins/filters.xml";
	initGui();
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
DbcAdmin::~DbcAdmin()
{
	delete m_ui;
}

///////////////////////////////////////////////////////////
// Initialize UI
///////////////////////////////////////////////////////////
void DbcAdmin::initGui()
{
	m_ui->progressBarDbc->setVisible(false);
	onRefreshSessionView();
	m_ui->treeWidgetDbChar->hideColumn(DBC_ADMIN_TREEWIDGET_FILEDB_COLUMN);
	m_ui->treeWidgetDbChar->setAcceptDrops(true);
	connect(m_ui->treeWidgetDbChar, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));
	connect(m_ui->treeWidgetDbChar, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onEditDatabase()));
}

///////////////////////////////////////////////////////////
// Show dialog to create new session and build it
///////////////////////////////////////////////////////////
void DbcAdmin::onCreateSession()
{
	bool ok;
	QString strSessionName = QInputDialog::getText(this, "Create session","Enter session name: ", QLineEdit::Normal, "My_Session", &ok);
	QString strCurrentTime_t = QString::number(QDateTime::currentDateTime().toTime_t());
	if (ok && !strSessionName.isEmpty())
	{
		createLocalDbcDirectory();
		QString strSessionDirPath = ReportOptions.GetLocalDatabaseFolder() + QDir::separator() + DBC_LOCAL_FOLDER + strCurrentTime_t + "_s_" + strSessionName + "/";
		createSessionDbcDirectory(strSessionDirPath);
		QFile::copy(m_strGexDbcTemplatePath, strSessionDirPath + strCurrentTime_t + ".sqlite");
		QFile::copy(m_strXmlFiltersTemplatePath, strSessionDirPath + "filters.xml");
		DbcTransaction transac(strSessionDirPath, strCurrentTime_t + ".sqlite");
		/// TODO to change
		transac.initDataBase("Raw_Database");
		QTreeWidgetItem* newSessionItem = sessionItem(strSessionDirPath);
		if (newSessionItem)
			m_ui->treeWidgetDbChar->insertTopLevelItem(m_ui->treeWidgetDbChar->topLevelItemCount(), newSessionItem);
	}
	onRefreshSessionView();
}

///////////////////////////////////////////////////////////
// Delete the selected database
///////////////////////////////////////////////////////////
void DbcAdmin::onDeleteDatabase()
{
	if (m_ui->treeWidgetDbChar->currentItem())
	{
		QString strSessionName = m_ui->treeWidgetDbChar->currentItem()->data(DBC_ADMIN_TREEWIDGET_FILEDB_COLUMN, Qt::DisplayRole).toString();
		removeDir(ReportOptions.GetLocalDatabaseFolder() + QDir::separator() + DBC_LOCAL_FOLDER + strSessionName);
		onRefreshSessionView();
	}
}

///////////////////////////////////////////////////////////
// Import file list linked a session in a database
///////////////////////////////////////////////////////////
void DbcAdmin::importFiles(QStringList strLstFile, QString strSessionName, QString strDbName)
{
	QString strWorkingFolder = ReportOptions.GetLocalDatabaseFolder() + QDir::separator() + DBC_LOCAL_FOLDER + strSessionName + "/";
	DbcTransaction dbcTransac(strWorkingFolder, strDbName);
	dbcTransac.loadSessionInfo();
	QStringList::iterator it;
	CGConvertToSTDF StdfConvert;
	for (it = strLstFile.begin(); it != strLstFile.end(); ++it)
	{
		QString strFileConverted, strFileToConvert = (*it);
		QListIterator<QStringList> itFileInfo(dbcTransac.fileList());
		bool bContainsFile = false;
		while (itFileInfo.hasNext() && !bContainsFile)
		{
			if (itFileInfo.next().contains(QFileInfo(strFileToConvert).fileName()))
				bContainsFile = true;
		}

		if (!bContainsFile)
		{
			bool bFileCreated = false;
			QString strMessage = "";
			int iStatus = StdfConvert.Convert(false, true, false, true, strFileToConvert, strFileConverted, QString(GEX_TEMPORARY_STDF), bFileCreated, strMessage, true, true);
			if(iStatus == CGConvertToSTDF::eConvertSuccess )
			{
				importFile(strFileConverted, dbcTransac, strFileToConvert);
				if(bFileCreated)
					QFile::remove(strFileConverted);
			}
			else
			{
				// Error
			}
		}
		else
		{
            GS::Gex::Message::critical("Import file",
                                       "ERROR: File already loaded in session");
		}
	}
	onRefreshSessionView();
	DbcDatasetEditor dbcParam(dbcTransac);
	dbcParam.exec();
}

///////////////////////////////////////////////////////////
// Show dataset editor to edit database
///////////////////////////////////////////////////////////
void DbcAdmin::onEditDatabase()
{
	if (m_ui->treeWidgetDbChar->currentItem())
	{
		QTreeWidgetItem *pSelectedItem = m_ui->treeWidgetDbChar->currentItem();
		QString strDbFileName = pSelectedItem->data(DBC_ADMIN_TREEWIDGET_FILEDB_COLUMN, Qt::DisplayRole).toString();
		if (!strDbFileName.contains(".sqlite"))
			return;
		QString strSessionName = pSelectedItem->parent()->data(DBC_ADMIN_TREEWIDGET_FILEDB_COLUMN, Qt::DisplayRole).toString();
		DbcTransaction dbTransac(ReportOptions.GetLocalDatabaseFolder() + QDir::separator() + DBC_LOCAL_FOLDER + strSessionName + "/", strDbFileName);
		dbTransac.loadSessionInfo();
		DbcDatasetEditor dbcParam(dbTransac);
		dbcParam.exec();
	}
}

///////////////////////////////////////////////////////////
// Create database directory
///////////////////////////////////////////////////////////
void DbcAdmin::createLocalDbcDirectory()
{
	QDir dir;
	// Go into Gex DB folder
	dir.cd(ReportOptions.GetLocalDatabaseFolder());
	QString strDirName = DBC_LOCAL_FOLDER;
	// Build the Characterization DATABASES folder
	if (!dir.exists(strDirName))
		dir.mkdir(strDirName);
}


///////////////////////////////////////////////////////////
// Create Session directory
///////////////////////////////////////////////////////////
void DbcAdmin::createSessionDbcDirectory(const QString strDirPath)
{
	QDir dir;
	// Build the Characterization DATABASES folder
	if (!dir.exists(strDirPath))
		dir.mkdir(strDirPath);
}

///////////////////////////////////////////////////////////
// Refresh list of characterization DB
///////////////////////////////////////////////////////////
void DbcAdmin::onRefreshView()
{
	// Is the tree widget is empty then load DBs
	if (m_ui->treeWidgetDbChar->topLevelItemCount() == 0)
		onLoadView();
	// Is the tree widget isn't empty then refresh it
	else
	{
		// Retrieve all DB declared in tree widget
		QStringList strLstExistingDb;
		QTreeWidgetItemIterator itemIter(m_ui->treeWidgetDbChar);
		while (*itemIter)
		{
			// If it's a top level item (=DB)
			if ((*itemIter)->text(DBC_ADMIN_TREEWIDGET_FILEDB_COLUMN).contains(".sqlite"))
				strLstExistingDb << (*itemIter)->text(DBC_ADMIN_TREEWIDGET_FILEDB_COLUMN);
			++itemIter;
		}

		// Searched file extension
		QStringList strLstFilter("*.sqlite");

		// Scan the folder to get all existing DB
		QDirIterator dirIterator(ReportOptions.GetLocalDatabaseFolder() + QDir::separator() + DBC_LOCAL_FOLDER, strLstFilter,  QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot | QDir::Writable);
		while (dirIterator.hasNext())
		{
			dirIterator.next();
			// If this DB is already in the tree widget remove it from the list and do nothing
			if (strLstExistingDb.contains(dirIterator.fileInfo().fileName()))
				strLstExistingDb.removeAll(dirIterator.fileInfo().fileName());

			// If this DB isn't in the tree widget then load it
			else
				loadDatabase(dirIterator.fileInfo());
		}

		// Remove from the widget all DBs which were in the widget but not in the folder
		QStringList::iterator itStr;
		for (itStr = strLstExistingDb.begin(); itStr != strLstExistingDb.end(); ++itStr)
			m_ui->treeWidgetDbChar->takeTopLevelItem(m_ui->treeWidgetDbChar->indexOfTopLevelItem(m_ui->treeWidgetDbChar->findItems((*itStr), Qt::MatchExactly, DBC_ADMIN_TREEWIDGET_FILEDB_COLUMN).first()));

		// Refresh file list of the DBs
		QTreeWidgetItemIterator it2(m_ui->treeWidgetDbChar);
		while (*it2)
		{
			// If it's a top level item
			if ((*it2)->text(DBC_ADMIN_TREEWIDGET_FILEDB_COLUMN).contains(".sqlite"))
			{
				(*it2)->takeChildren();
				refreshFiles((*it2));
			}
			else
			{
				refreshDatabase((*it2));
			}
			++it2;
		}
	}
}


///////////////////////////////////////////////////////////
// Refresh list of characterization DB
///////////////////////////////////////////////////////////
void DbcAdmin::onRefreshSessionView()
{
	// Clear tree widget
	m_ui->treeWidgetDbChar->clear();

	// Get session list
	QStringListIterator itSession(sessionList());

	// Load each session item
	while (itSession.hasNext())
	{
		QTreeWidgetItem* newSessionItem = sessionItem(ReportOptions.GetLocalDatabaseFolder() + QDir::separator() + QString(DBC_LOCAL_FOLDER) + itSession.next() + "/");
		if (newSessionItem)
			m_ui->treeWidgetDbChar->insertTopLevelItem(m_ui->treeWidgetDbChar->topLevelItemCount(), newSessionItem);
	}

	m_ui->treeWidgetDbChar->expandAll();
	m_ui->treeWidgetDbChar->resizeColumnToContents(0);
	m_ui->treeWidgetDbChar->resizeColumnToContents(1);
	m_ui->treeWidgetDbChar->resizeColumnToContents(2);
}

///////////////////////////////////////////////////////////
// Build session item
///////////////////////////////////////////////////////////
QTreeWidgetItem *DbcAdmin::sessionItem(const QString &strDirPath)
{
	QDir					dirSession(strDirPath);
	QStringList				lstSessionInfo;
	lstSessionInfo << dirSession.dirName().section("_", 2) << "" << "" << dirSession.dirName();
	QTreeWidgetItem *		newSessionItem = new QTreeWidgetItem(lstSessionInfo);

	QList<QTreeWidgetItem *> items;
	QStringList lstFileDBName = dirSession.entryList("*.sqlite");
	if (lstFileDBName.isEmpty())
		return NULL;

	// Open Db transaction
	DbcTransaction dbcTransac(strDirPath, lstFileDBName.first());
	dbcTransac.loadSessionInfo();

	QStringList strLstItems;
	strLstItems << dbcTransac.dbName();
	strLstItems << dbcTransac.creationDate().toString("HH:mm:ss MMM d, yyyy");
	strLstItems << QString::number(QFileInfo(strDirPath + lstFileDBName.first()).size()) + " bytes";
	strLstItems << lstFileDBName.first();
	QTreeWidgetItem *newItem = new QTreeWidgetItem(strLstItems);

	QListIterator<QStringList> itFileInfo(dbcTransac.fileList());
	while (itFileInfo.hasNext())
	{
		QTreeWidgetItem *newSubItem = new QTreeWidgetItem(QStringList(itFileInfo.next().at(1)));
		newItem->insertChild(0, newSubItem);
	}
	items.append(newItem);
	newSessionItem->addChildren(items);

	return newSessionItem;
}


///////////////////////////////////////////////////////////
// Receive drag enter events
///////////////////////////////////////////////////////////
void DbcAdmin::dragEnterEvent(QDragEnterEvent *e)
{
	// Accept Drag if files list dragged over.
	if(e->mimeData()->hasUrls())
		e->acceptProposedAction();
}

///////////////////////////////////////////////////////////
// Receive drag move events
///////////////////////////////////////////////////////////
void DbcAdmin::dragMoveEvent(QDragMoveEvent *e)
{
	// Accept Drag if files list dragged over.
	if(e->mimeData()->hasUrls())
	{
		m_ui->treeWidgetDbChar->activateWindow();
		QTreeWidgetItem *pSelectedItem = m_ui->treeWidgetDbChar->itemAt(m_ui->treeWidgetDbChar->mapFrom(this, e->pos()));
		if (pSelectedItem)
		{
			// To focus on Databases and not on files
			if (pSelectedItem->parent() && pSelectedItem->parent()->data(DBC_ADMIN_TREEWIDGET_FILEDB_COLUMN, Qt::DisplayRole).toString().contains(".sqlite"))
				pSelectedItem = pSelectedItem->parent();
			if (!pSelectedItem->data(DBC_ADMIN_TREEWIDGET_FILEDB_COLUMN, Qt::DisplayRole).toString().contains(".sqlite"))
				pSelectedItem = pSelectedItem->child(0);
			m_ui->treeWidgetDbChar->setCurrentItem(pSelectedItem);
			e->acceptProposedAction();
		}
	}
}

///////////////////////////////////////////////////////////
// Receive drop events
///////////////////////////////////////////////////////////
void DbcAdmin::dropEvent(QDropEvent *e)
{
	// Extract list of files dragged over...
	if(!e->mimeData()->hasUrls())
	{
		// No files dropped...ignore drag&drop.
		e->ignore();
		return;
	}

	QString		strFileName;
	QStringList strFileList;
	QList<QUrl> lstUrls = e->mimeData()->urls();

	for (int nUrl = 0; nUrl < lstUrls.count(); nUrl++)
	{
		strFileName = lstUrls.at(nUrl).toLocalFile();

		if (!strFileName.isEmpty())
			strFileList << strFileName;
	}

	if(strFileList.count() <= 0)
	{
		// Items dropped are not regular files...ignore.
		e->ignore();
		return;
	}

	// Accept drag & drop
	e->acceptProposedAction();
	QString strDbName = m_ui->treeWidgetDbChar->currentItem()->data(DBC_ADMIN_TREEWIDGET_FILEDB_COLUMN, Qt::DisplayRole).toString();
	QString strSessionName = m_ui->treeWidgetDbChar->currentItem()->parent()->data(DBC_ADMIN_TREEWIDGET_FILEDB_COLUMN, Qt::DisplayRole).toString();
	if (!strDbName.isEmpty() && !strFileList.isEmpty())
	{
        bool lOk;
        GS::Gex::Message::request("", QString("Insert file(s)", "Insert " + strFileList.join(",") +
                                              " to session " + strSessionName + "?"), lOk);
        if (lOk)
        {
			importFiles(strFileList, strSessionName, strDbName);
        }
	}
}

///////////////////////////////////////////////////////////
// Load session view
///////////////////////////////////////////////////////////
void DbcAdmin::onLoadView()
{
	QStringListIterator itSession(sessionList());
	while (itSession.hasNext())
		loadSession(itSession.next());
}

///////////////////////////////////////////////////////////
// Load database
///////////////////////////////////////////////////////////
void DbcAdmin::loadDatabase(const QFileInfo &fileInfoDatabase)
{
	QList<QTreeWidgetItem *> items;
	QString strFileName = fileInfoDatabase.fileName();
	QString strWorkingFolder = fileInfoDatabase.absolutePath();
	DbcTransaction dbcTransac(strWorkingFolder, strFileName);
	dbcTransac.loadSessionInfo();
	QStringList strLstItems;
	strLstItems << dbcTransac.dbName();
	strLstItems << dbcTransac.creationDate().toString("HH:mm:ss MMM d, yyyy");
	strLstItems << QString::number(fileInfoDatabase.size()) + " bytes";
	strLstItems << fileInfoDatabase.fileName();
	QTreeWidgetItem *newItem = new QTreeWidgetItem(strLstItems);

	QListIterator<QStringList> itFileInfo(dbcTransac.fileList());
	while (itFileInfo.hasNext())
	{
		QTreeWidgetItem *newSubItem = new QTreeWidgetItem(QStringList(itFileInfo.next().at(1)));
		newItem->insertChild(0, newSubItem);
	}
	items.append(newItem);

	m_ui->treeWidgetDbChar->insertTopLevelItems(m_ui->treeWidgetDbChar->topLevelItemCount(), items);
	m_ui->treeWidgetDbChar->resizeColumnToContents(0);
	m_ui->treeWidgetDbChar->resizeColumnToContents(1);
	m_ui->treeWidgetDbChar->resizeColumnToContents(2);
}

///////////////////////////////////////////////////////////
// Return session list
///////////////////////////////////////////////////////////
QStringList DbcAdmin::sessionList()
{
	QDir dirSessions(ReportOptions.GetLocalDatabaseFolder() + QDir::separator() + QString(DBC_LOCAL_FOLDER));
	return dirSessions.entryList("*_s_*", QDir::NoDotAndDotDot | QDir::Dirs);
}

///////////////////////////////////////////////////////////
// Load session
///////////////////////////////////////////////////////////
void DbcAdmin::loadSession(const QString &strDirPath)
{
	QList<QTreeWidgetItem *> items;
	QDir dirSession(strDirPath);
	QStringList lstFileDBName = dirSession.entryList("*.sqlite");
	if (lstFileDBName.isEmpty())
		return;
	DbcTransaction dbcTransac(strDirPath, lstFileDBName.first());
	dbcTransac.loadSessionInfo();

	QStringList strLstItems;
	strLstItems << dbcTransac.dbName();
	strLstItems << dbcTransac.creationDate().toString("HH:mm:ss MMM d, yyyy");
	strLstItems << QString::number(QFileInfo(strDirPath + lstFileDBName.first()).size()) + " bytes";
	strLstItems << lstFileDBName.first();
	QTreeWidgetItem *newItem = new QTreeWidgetItem(strLstItems);

	QListIterator<QStringList> itFileInfo(dbcTransac.fileList());
	while (itFileInfo.hasNext())
	{
		QTreeWidgetItem *newSubItem = new QTreeWidgetItem(QStringList(itFileInfo.next().at(1)));
		newItem->insertChild(0, newSubItem);
	}
	items.append(newItem);

	m_ui->treeWidgetDbChar->insertTopLevelItems(m_ui->treeWidgetDbChar->topLevelItemCount(), items);
}


///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
void DbcAdmin::refreshFiles(QTreeWidgetItem *itemDb)
{
	QString stFileDbName = itemDb->text(DBC_ADMIN_TREEWIDGET_FILEDB_COLUMN);
	DbcTransaction dbcTransac(ReportOptions.GetLocalDatabaseFolder() + QDir::separator() + DBC_LOCAL_FOLDER, stFileDbName);
	dbcTransac.loadSessionInfo();

	QListIterator<QStringList> itFileInfo(dbcTransac.fileList());
	while (itFileInfo.hasNext())
	{
		QTreeWidgetItem *newSubItem = new QTreeWidgetItem(QStringList(itFileInfo.next().at(1)));
		itemDb->insertChild(0, newSubItem);
	}
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
void DbcAdmin::refreshDatabase(QTreeWidgetItem* /*itemDb*/)
{
}

///////////////////////////////////////////////////////////
// Show context menu
///////////////////////////////////////////////////////////
void DbcAdmin::onContextMenuRequested(const QPoint &point)
{
	QTreeWidgetItem *itemSelected = m_ui->treeWidgetDbChar->itemAt(point);
	if(!itemSelected)
		return;
	QMenu menu(this);
	menu.addAction(*pixCreateFolder	, "Create a new database", this, SLOT(onCreateSession()));
	menu.addAction(*pixProperties	, "Edit database", this, SLOT(onEditDatabase()));
	menu.addAction(*pixOpen			, "Add files to database");
	menu.addSeparator();
	if (itemSelected->parent())
	{
		menu.addAction(*pixRemove	, "Remove file", this, SLOT(onRemoveFile()));
	}
	else
	{
		menu.addAction(*pixRemove	, "Remove database", this, SLOT(onDeleteDatabase()));
	}
	menu.exec(QCursor::pos());
}


///////////////////////////////////////////////////////////
// Import one file to database
///////////////////////////////////////////////////////////
void DbcAdmin::importFile(const QString & strFileName, DbcTransaction &dbcTransac, const QString & strOriginalFileName)
{
	m_ui->progressBarDbc->setValue(0);
	dbcTransac.beginTransaction();
	dbcTransac.addFile(strOriginalFileName);

	QString strMessage;
	DbcStdfReader stdfReader(this);
	connect(&stdfReader, SIGNAL(sCountedStep(int)), m_ui->progressBarDbc, SLOT(setMaximum(int)));
	connect(&stdfReader, SIGNAL(sReadParameterInfo(const QMap<TestKey, Gate_ParameterDef>&)), &dbcTransac, SLOT(insertParameterInfo(const QMap<TestKey, Gate_ParameterDef>&)));
	connect(&stdfReader, SIGNAL(sReadStep(const QMap<TestKey, Gate_DataResult>&)), this, SLOT(updateProgressBar()));
	connect(&stdfReader, SIGNAL(sReadStep(const QMap<TestKey, Gate_DataResult>&)), &dbcTransac, SLOT(insertParameterResults(const QMap<TestKey,Gate_DataResult>&)));

	if (!stdfReader.read(strFileName, strMessage))
		GSLOG(SYSLOG_SEV_ERROR, strMessage);
	else
		GSLOG(SYSLOG_SEV_DEBUG, QString(strFileName).append(" OK"));

	dbcTransac.commitTransaction();
}

///////////////////////////////////////////////////////////
// Remove selected file from database
///////////////////////////////////////////////////////////
void DbcAdmin::onRemoveFile()
{
	QString strFileName = m_ui->treeWidgetDbChar->currentItem()->data(DBC_ADMIN_TREEWIDGET_FILE_COLUMN, Qt::DisplayRole).toString();
	QString strDbName = m_ui->treeWidgetDbChar->currentItem()->parent()->data(DBC_ADMIN_TREEWIDGET_FILEDB_COLUMN, Qt::DisplayRole).toString();
	QString strSessionName = m_ui->treeWidgetDbChar->currentItem()->parent()->parent()->data(DBC_ADMIN_TREEWIDGET_FILEDB_COLUMN, Qt::DisplayRole).toString();
	removeFile(strFileName, strDbName, strSessionName);
}

///////////////////////////////////////////////////////////
// Remove strFile from database
///////////////////////////////////////////////////////////
void DbcAdmin::removeFile(const QString &strFile, const QString &strDbName, const QString &strSessionName)
{
	QString strWorkingFolder = ReportOptions.GetLocalDatabaseFolder() + QDir::separator() + DBC_LOCAL_FOLDER + strSessionName + "/";
	DbcTransaction dbcTransac(strWorkingFolder, strDbName);
	dbcTransac.beginTransaction();
	dbcTransac.removeFile(strFile);
	dbcTransac.commitTransaction();
	onRefreshSessionView();
}

///////////////////////////////////////////////////////////
// Update progress bar
///////////////////////////////////////////////////////////
void DbcAdmin::updateProgressBar()
{
	int iValue = m_ui->progressBarDbc->value() + 1;
	m_ui->progressBarDbc->setValue(iValue);

	if (iValue == m_ui->progressBarDbc->maximum())
		m_ui->progressBarDbc->setVisible(false);
	else
		m_ui->progressBarDbc->setVisible(true);
    QApplication::processEvents();
}

///////////////////////////////////////////////////////////
// Remove directory
///////////////////////////////////////////////////////////
bool DbcAdmin::removeDir(QString dirPath)
{
    QDir folder(dirPath);
    folder.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);
    foreach(QFileInfo fileInfo, folder.entryInfoList())
    {
        if(fileInfo.isDir())
        {
            if(!removeDir(fileInfo.filePath()))
                return false;
        }
        else if(fileInfo.isFile())
        {
            if(!QFile::remove(fileInfo.filePath()))
            {
				GSLOG(SYSLOG_SEV_ERROR, QString("unable to remove file : %1.")
				       .arg(fileInfo.filePath()));
                return false;
            }
        }
    }

    if(!folder.rmdir(dirPath))
    {
		GSLOG(SYSLOG_SEV_ERROR, QString("unable to remove folder : %1. Maybe this folder is not empty.")
		       .arg(dirPath));
        return false;
    }

    return true;
}


