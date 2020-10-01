#ifndef REPORTS_CENTER_TREE_MODEL_H
#define REPORTS_CENTER_TREE_MODEL_H

#include <QAbstractItemModel>
#include "reports_center_item.h"


class ReportsCenterTreeModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	ReportsCenterTreeModel(QObject *parent = 0);
	~ReportsCenterTreeModel();

	ReportsCenterItem* GetItem(const QModelIndex &index);
	QVariant data(const QModelIndex &index, int role) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	int removeAllChildOfType(QString t);	// remove all child of the given type and return number of removed
	// the root (should contain "WaferSort","FinalTest",...)
	ReportsCenterItem *m_rootItem;
	// The tree view.
	QTreeView* m_pTreeView;
	// virtual slot
	bool submit();
	void revert();

public slots:
	void UpdateGUI()	{	reset(); }
private:
};

#endif // REPORTS_CENTER_TREE_MODEL_H
