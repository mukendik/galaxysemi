#ifndef DBC_ADMIN_H
#define DBC_ADMIN_H

#include <QWidget>
#include <QTreeWidget>
#include <QFileInfo>

#define DBC_LOCAL_FOLDER						"local_dbc/"
#define DBC_TEMPLATE_PATH						"install/characterization/gex_dbc_v4.sqlite"
#define DBC_ADMIN_TREEWIDGET_FILEDB_COLUMN		3
#define DBC_ADMIN_TREEWIDGET_FILE_COLUMN		0


namespace Ui {
	class DbcAdmin;
}

class DbcTransaction;

class DbcAdmin : public QWidget {
	Q_OBJECT
public:
	DbcAdmin(QWidget *parent = 0);
	~DbcAdmin();
	void createLocalDbcDirectory();
	void createSessionDbcDirectory(const QString strDirPath);
	void initGui();
	bool removeDir(QString dirPath);

	// Overwrite virtual fcts from main class for Drag&Drop support.
	void dragEnterEvent(QDragEnterEvent *);
	void dragMoveEvent(QDragMoveEvent *);
	void dropEvent(QDropEvent *);

public slots:
	void onCreateSession();
	void onDeleteDatabase();
	void onRemoveFile();
	void onEditDatabase();
	void onRefreshView();
	void onRefreshSessionView();
	void onLoadView();
	void onContextMenuRequested(const QPoint &point);
	void updateProgressBar();

private:
	void importFiles(QStringList strLstFile, QString strSessionName, QString strDbName);
	void importFile(const QString & strFileName, DbcTransaction &dbcTransac, const QString &strOriginalFileName);
	void removeFile(const QString &strFile, const QString &strDbName, const QString &strSessionName);
	void loadDatabase(const QFileInfo &fileInfoDatabase);
	void loadSession(const QString &strDirPath);
	QTreeWidgetItem *sessionItem(const QString &strDirPath);
	QStringList sessionList();
	
	void refreshFiles(QTreeWidgetItem *);
	void refreshDatabase(QTreeWidgetItem *);
	
	Ui::DbcAdmin *m_ui;
	QString m_strGexDbcTemplatePath;
	QString m_strXmlFiltersTemplatePath;
};

#endif // DBC_ADMIN_H
