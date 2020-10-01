#include "treewidgetitemcmp.h"

///////////////////////////////////////////////////////////////////////////////
// Class QTreeWidgetItemCmp - Class which represents an item in a treewidget
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////
QTreeWidgetItemCmp::QTreeWidgetItemCmp(QTreeWidget * pParent) : QTreeWidgetItem(pParent)
{
}

QTreeWidgetItemCmp::QTreeWidgetItemCmp(QTreeWidget * pParent, QTreeWidgetItem * after) : QTreeWidgetItem(pParent, after)
{
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
QTreeWidgetItemCmp::~QTreeWidgetItemCmp()
{
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void setText( int column, const QString& text )
//
// Description	:	Override the standard setText function
//					Add a text extension to the group name if this one is the Reference Group
//					Extension is " (Ref)"
//
// Param		:	int				- 	Column number to update
//					const QString&	-	Text to display
//
///////////////////////////////////////////////////////////////////////////////////
void QTreeWidgetItemCmp::setText( int column, const QString& text )
{
	// Override writting of group name
	if (column == 0)
	{
		QString strGroupTmp = m_strGroupBase = text;

		// First item in the listview, reference DataSet
        if (treeWidget()->indexOfTopLevelItem(this) == 0 && strGroupTmp.endsWith(" (Ref)") == false)
			strGroupTmp += " (Ref)";

		QTreeWidgetItem::setText(column, strGroupTmp);
	}
	else
		QTreeWidgetItem::setText(column, text);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void updateGroupName()
//
// Description	:	Refresh the column containing group name.
//
///////////////////////////////////////////////////////////////////////////////////
void QTreeWidgetItemCmp::updateGroupName()
{
	setText(0, m_strGroupBase);
}
