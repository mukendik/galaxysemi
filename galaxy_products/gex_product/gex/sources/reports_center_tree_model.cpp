#include "reports_center_widget.h"
#include "reports_center_item.h"
#include "jasper_threads.h"
#include "jasper_params_widget.h"
//#include "libjasper.h"
#include <gqtl_log.h>
#include "browser_dialog.h"

#include "ui_reports_center_widget.h"

//#include <QLibrary>
#include <QTreeView>
#include <QMutex>
#include <QDir>

extern void	WriteDebugMessageFile(const QString& strMessage);

ReportsCenterTreeModel::ReportsCenterTreeModel(QObject *parent) : QAbstractItemModel(parent)
{
	QList<QVariant> rootData;
	rootData << "name" << "description";
	m_rootItem = new ReportsCenterItem(rootData, NULL, "/", "?");

	// Always ?
	/*
	QList<QVariant> wsdata; wsdata << "Wafer Sort" << "Wafer sort stuff..." << "/WaferSort";
	ReportsCenterItem* wsitem=new ReportsCenterItem( wsdata, m_rootItem, "/WaferSort", "folder" );
	m_rootItem->appendChild(wsitem);
	*/
}

ReportsCenterTreeModel::~ReportsCenterTreeModel()
{
//    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" m_rootItem=%1, must delete %2 childs")
//          .arg( m_rootItem?m_rootItem:0)
//          .arg( m_rootItem->childCount()));

	// Check me : do we have to delete by our own ?
	if (m_rootItem)
		delete m_rootItem;
	m_rootItem=NULL;
}

int ReportsCenterTreeModel::columnCount(const QModelIndex &parent) const
{
		if (parent.isValid())
		 return static_cast<ReportsCenterItem*>(parent.internalPointer())->columnCount();
		else
		 return m_rootItem->columnCount();
}

ReportsCenterItem* ReportsCenterTreeModel::GetItem(const QModelIndex &index)
{
	if (!index.isValid())
				return NULL;
	return static_cast<ReportsCenterItem*>(index.internalPointer());
}

QVariant ReportsCenterTreeModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	ReportsCenterItem *item = static_cast<ReportsCenterItem*>(index.internalPointer());
	if (!item)
		return QVariant();

	if (role==Qt::DecorationRole)
	{
		if (index.column()==0)
			return	item->data(2); //QVariant(QPixmap(QString::fromUtf8(":/gex/icons/script_new.png")));
		else
			return QVariant();
	}

	if (role != Qt::DisplayRole)
		return QVariant();

	return item->data(index.column());
}

void ReportsCenterTreeModel::revert()
{
	GSLOG(SYSLOG_SEV_DEBUG, " ");
}

bool ReportsCenterTreeModel::submit()
{
    GSLOG( SYSLOG_SEV_DEBUG, QString("x:%1 y:%2").
           arg(m_pTreeView->x()).
           arg(m_pTreeView->y()).toLatin1().constData());

	//ReportsCenterItem *item=GetItem(m_pTreeView->currentIndex());

	/*
	WriteDebugMessageFile(QString("ReportsCenterTreeModel::submit: c%1,r:%2 %3 %4")
		.arg(m_pTreeView->currentIndex().column())
		.arg(m_pTreeView->currentIndex().row())
		.arg(m_pTreeView->currentIndex().data(0).toString())
		.arg(item?QString(item->m_fullpath+" "+item->m_type):QString("?"))
		);
	*/
	return true;
}

int ReportsCenterTreeModel::removeAllChildOfType(QString t)
{
	int n=0;
	n=m_rootItem->removeAllChildsOfType(t); //n=n+RecursiveRemoveChildOfType(m_rootItem, t);
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString(" %1 items removed !").arg(n).toLatin1().constData());
	return n;
}

/*
int ReportsCenterTreeModel::RecursiveRemoveChildOfType(ReportsCenterItem *i, QString t)
{
	int n=0;
	return n;
}
*/

Qt::ItemFlags ReportsCenterTreeModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsEnabled;
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant ReportsCenterTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return m_rootItem->data(section);
	return QVariant();
}

QModelIndex ReportsCenterTreeModel::index(int row, int column, const QModelIndex &parent) const
{
	ReportsCenterItem *parentItem;

	if (!parent.isValid())
		parentItem = m_rootItem;
	else
		parentItem = static_cast<ReportsCenterItem*>(parent.internalPointer());

	ReportsCenterItem *childItem = parentItem->child(row);
	if (childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}

QModelIndex ReportsCenterTreeModel::parent(const QModelIndex &index) const
{
	if (!index.isValid())
			return QModelIndex();

	ReportsCenterItem *childItem = static_cast<ReportsCenterItem*>(index.internalPointer());
	ReportsCenterItem *parentItem = childItem->parent();

	if (parentItem == m_rootItem)
			return QModelIndex();

	return createIndex(parentItem->row(), 0, parentItem);
}

int ReportsCenterTreeModel::rowCount(const QModelIndex &parent) const
{
	ReportsCenterItem *parentItem;

	if (!parent.isValid())
			parentItem = m_rootItem;
	else
			parentItem = static_cast<ReportsCenterItem*>(parent.internalPointer());

	return parentItem->childCount();
}
