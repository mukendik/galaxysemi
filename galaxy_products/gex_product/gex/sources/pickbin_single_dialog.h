#ifndef PICKBINSINGLEDIALOG_H
#define PICKBINSINGLEDIALOG_H

#include "ui_pickbin_single_dialog.h"

#include <QTreeWidget>

/////////////////////////////////////////////////////////////////////////////
class PickBinSingleDialog : public QDialog, public Ui::PickBinSingleDialogBase
{
	Q_OBJECT

public:
	PickBinSingleDialog( QWidget* parent = 0, bool modal = false, Qt::WindowFlags f = 0 );
	void	fillList(QStringList & cList);
	void	setMultipleSelection(bool bAllowMultiSelections);
	QString getBinsList(void);
    QString getBinsNameList(void);
private:
    bool	mGetSofBin;	// to hold type of bin to list (true=soft-bin, false=hard-bin)
};

class PickBinSingleDialogTreeItem : public QTreeWidgetItem
{
public:

	PickBinSingleDialogTreeItem(QTreeWidget * pTreeWidget, QStringList strTreeListItem);
	virtual ~PickBinSingleDialogTreeItem();

	bool operator< ( const QTreeWidgetItem & other ) const;

protected:

	bool	sortInteger(const QString& strLeft, const QString& strRight) const;
	bool	sortBinName(const QString& strLeft, const QString& strRight) const;
};

#endif //PICKBINSINGLEDIALOG_H
