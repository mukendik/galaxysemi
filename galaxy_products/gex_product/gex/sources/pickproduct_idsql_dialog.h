#ifndef PICKPDTIDSQLDIALOG_H
#define PICKPDTIDSQLDIALOG_H

#include "ui_pickproduct_idsql_dialog.h"

#include <QTreeWidget>

/////////////////////////////////////////////////////////////////////////////
class PickProductIdSQLDialog : public QDialog, public Ui::PickProductIdSQLDialogBase
{
	Q_OBJECT
		
public:
	PickProductIdSQLDialog( QWidget* parent = 0, bool modal = false, Qt::WindowFlags f = 0 );
	void	fillList(QStringList & cList, bool bAllowMultiSelections, bool bMultiTestingStage);
	void	getProductList(QStringList & cList);


private:
	bool	m_bAllowMultiSelections;	// true if the multiple selections are allowed
	bool	m_bMultiTestingStage;		// true if the dialog should show products with data in more than 2 testing stages
};

class PickProductIdSQLDialogItem : public QTreeWidgetItem
{
public:

	PickProductIdSQLDialogItem(QTreeWidget *pTreeWidget);
	PickProductIdSQLDialogItem(QTreeWidget * pTreeWidget, QStringList strTreeListItem);
	PickProductIdSQLDialogItem(QStringList strTreeListItem);
	virtual ~PickProductIdSQLDialogItem();

	bool operator< ( const QTreeWidgetItem & other ) const;

};

#endif // PICKPDTIDSQLDIALOG_H
