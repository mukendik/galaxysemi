#include "pickname_dialog.h"
#include "browser_dialog.h"

///////////////////////////////////////////////////////////
// Constructor
PickNameDialog::PickNameDialog( QWidget* parent, bool modal, Qt::WindowFlags f ) 
	: QDialog( parent, f )
{
	setupUi(this);
	setModal(modal);

	// Apply Examinator palette
	GexMainwindow::applyPalette(this);

	QObject::connect(PushButtonOk,		SIGNAL(clicked()),						this, SLOT(accept()));
    QObject::connect(PushButtonCancel,	SIGNAL(clicked()),						this, SLOT(reject()));
	QObject::connect(treeWidgetSuggest,	SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(accept()));

	treeWidgetSuggest->sortByColumn(0, Qt::AscendingOrder);
}

///////////////////////////////////////////////////////////
// Empties the suggestion list
///////////////////////////////////////////////////////////
void PickNameDialog::clear(void)
{
	treeWidgetSuggest->clear();
}

///////////////////////////////////////////////////////////
// Add item
///////////////////////////////////////////////////////////
void PickNameDialog::insert(QString strLegend,QString strSuggestion)
{
	QTreeWidgetItem * pTreeItem = new QTreeWidgetItem(treeWidgetSuggest);
	pTreeItem->setText(0, strLegend);
	pTreeItem->setText(1, strSuggestion);

	// Focus on Test list.
	treeWidgetSuggest->setFocus();
}

///////////////////////////////////////////////////////////
// Tells if the list of test is empty or not...
///////////////////////////////////////////////////////////
bool PickNameDialog::isEmpty(void)
{
	//if(ListView->childCount() <= 0)
	if (treeWidgetSuggest->topLevelItemCount())
		return true;	// Empty list
	else
		return false;	// List initialized, tests in list.
}

///////////////////////////////////////////////////////////
// Tells if append or overwrite mode selected
///////////////////////////////////////////////////////////
bool PickNameDialog::isAppendMode(void)
{
	if(comboBox->currentIndex() == 0)
		return false;	// REPLACE mode
	else
		return true;	// APPEND mode
}


///////////////////////////////////////////////////////////
// Returns the name selected
///////////////////////////////////////////////////////////
QString PickNameDialog::nameList(void)
{
	QTreeWidgetItem * pTreeItem = treeWidgetSuggest->currentItem();

	if (pTreeItem)
		return pTreeItem->text(1);
	else
		return "";
}
