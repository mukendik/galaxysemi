#ifndef REPORTS_CENTER_ITEM_H
#define REPORTS_CENTER_ITEM_H

#include <QStringList>
#include <QtWidgets/QWidget>
#include <QList>
#include <QMap>
#include <QVariant>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QThread>
#include <QTreeView>
//#include <QLibrary>
#include <QMessageBox>

class ReportsCenterItem
{
public:
	ReportsCenterItem(const QList<QVariant> &data, ReportsCenterItem *parent, QString fullpath, QString type);

	~ReportsCenterItem()
	{
		//GSLOG(SYSLOG_SEV_DEBUG, " ");
		//qDeleteAll(childItems);
	}

	void appendChild(ReportsCenterItem *child) { childItems.append(child); }
	ReportsCenterItem *child(int row) { return childItems.value(row); }

	ReportsCenterItem* getChild(QString n);

	bool hasDirChild(QString dirname);

	// recursive !
	int removeAllChildsOfType(QString t);

	int childCount() const { return childItems.count(); }
	int columnCount() const { return itemData.count(); }
	QVariant data(int column) const { return itemData.value(column); }
	inline int row() const { if (parentItem) return parentItem->childItems.indexOf(const_cast<ReportsCenterItem*>(this)); return 0; }
	ReportsCenterItem *parent() { return parentItem; }
	QString m_fullpath; // "/WaferSort/W1"
	QString m_type;	// "report","dir",...
private:
	QList<ReportsCenterItem*> childItems;
	QList<QVariant> itemData;

	ReportsCenterItem *parentItem;
};

#endif // REPORTS_CENTER_ITEM_H
