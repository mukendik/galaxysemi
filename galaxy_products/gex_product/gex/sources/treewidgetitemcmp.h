#ifndef TREEWIDGETITEMCMP_H
#define TREEWIDGETITEMCMP_H

///////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////

#include <QTreeWidgetItem>

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	QTreeWidgetItemCmp
//
// Description	:	Represents an item in the treewidget for compare and group
//					compare wizard pages.
//					Override the standard QTreeWidgetItem in order to add to the
//					Reference Group a text extension "( Ref)"
//
///////////////////////////////////////////////////////////////////////////////////
class QTreeWidgetItemCmp : public QTreeWidgetItem
{
public:

	QTreeWidgetItemCmp(QTreeWidget * pParent);
	QTreeWidgetItemCmp(QTreeWidget * pParent, QTreeWidgetItem * after);
	virtual ~QTreeWidgetItemCmp();

///////////////////////////////////////////////////////////
// Properties
///////////////////////////////////////////////////////////
	const QString&	groupBase() const		{ return m_strGroupBase; }

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////
	virtual void	setText( int column, const QString& text );			// Override the setText function to add " (Ref)" to the reference item

	void			updateGroupName();									// Refresh the gui with the group name

private:

    QString			m_strGroupBase;										// Base of the group name
};


#endif // TREEWIDGETITEMCMP_H
